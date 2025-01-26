#include "Model.hpp"
#include "PostProcess.hpp"
#include <cmath>
#include <cstring>
#include <iostream>

static int get_core_num()
{
    static int core_num = 0;
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    int temp = core_num % NPU_CORE_NUM;
    core_num++;
    return temp;
}

static int read_data_from_file(const char * path, char ** out_data)
{
    FILE * fp = fopen(path, "rb");
    if(fp == NULL) {
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    int file_size   = ftell(fp);
    char * data     = (char *)malloc(file_size + 1);
    data[file_size] = 0;
    fseek(fp, 0, SEEK_SET);
    if(file_size != fread(data, 1, file_size, fp)) {
        free(data);
        fclose(fp);
        return -1;
    }
    if(fp) {
        fclose(fp);
    }
    *out_data = data;
    return file_size;
}

// 将量化后的数据转换为浮点数
static float deqnt_affine_to_f32(int8_t qnt, int32_t zp, float scale)
{
    return ((float)qnt - (float)zp) * scale;
}

// 输出归一化
static void output_normalization(rknn_app_context_t * app_ctx, uint8_t * output, std::vector<float> & out_fp32)
{
    int32_t zp  = app_ctx->output_attrs->zp;
    float scale = app_ctx->output_attrs->scale;

    for(int i = 0; i < 128; i++) out_fp32[i] = deqnt_affine_to_f32(output[i], zp, scale);
    float sum = 0;
    for(int i = 0; i < 128; i++) sum += out_fp32[i] * out_fp32[i];
    float norm = std::sqrt(sum);
    for(int i = 0; i < 128; i++) out_fp32[i] /= norm;
}

// 打印tensor属性
static void dump_tensor_attr(rknn_tensor_attr * attr)
{
    printf("  index=%d, name=%s, n_dims=%d, dims=[%d, %d, %d, %d], n_elems=%d, "
           "size=%d, fmt=%s, type=%s, qnt_type=%s, "
           "zp=%d, scale=%f\n",
           attr->index, attr->name, attr->n_dims, attr->dims[0], attr->dims[1], attr->dims[2], attr->dims[3],
           attr->n_elems, attr->size, get_format_string(attr->fmt), get_type_string(attr->type),
           get_qnt_type_string(attr->qnt_type), attr->zp, attr->scale);
}

// ===================================================================================================================

Facenet::Facenet()
{}

int Facenet::init(rknn_context * ctx_in, bool is_copy)
{
    int model_len = 0;
    char * model;
    int ret   = 0;
    model_len = read_data_from_file(FACENET_MODEL_PATH, &model);

    if(model == nullptr) {
        std::cout << "Load model failed" << std::endl;
        return -1;
    }

    if(is_copy) {
        ret = rknn_dup_context(ctx_in, &ctx_);
        if(ret != RKNN_SUCC) {
            std::cout << "rknn_dup_context failed! error code = " << ret << std::endl;
            return -1;
        }
    } else {
        std::cout << "rknn_init() is called" << std::endl;
        ret = rknn_init(&ctx_, model, model_len, 0, NULL);
        free(model);
        if(ret != RKNN_SUCC) {
            std::cout << "rknn_init failed! error code = " << ret << std::endl;
            return -1;
        }
    }

    rknn_core_mask core_mask;

    switch(get_core_num()) {
        case 0: core_mask = RKNN_NPU_CORE_0; break;
        case 1: core_mask = RKNN_NPU_CORE_1; break;
        case 2: core_mask = RKNN_NPU_CORE_2; break;
    }

    ret = rknn_set_core_mask(ctx_, core_mask);

    if(ret < 0) {
        std::cout << "rknn_set_core_mask failed! error code = " << ret << std::endl;
        return -1;
    }

    rknn_sdk_version version;

    ret = rknn_query(ctx_, RKNN_QUERY_SDK_VERSION, &version, sizeof(rknn_sdk_version));
    if(ret < 0) {
        return -1;
    }

    // Get Model Input Output Number
    rknn_input_output_num io_num;

    ret = rknn_query(ctx_, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));

    if(ret != RKNN_SUCC) {
        std::cout << "rknn_query failed! error code = " << ret << std::endl;
        return -1;
    }

    // Get Model Input Info

    rknn_tensor_attr input_attrs[io_num.n_input]; // 这里使用的是变长数组，不建议这么使用

    memset(input_attrs, 0, sizeof(input_attrs));

    for(int i = 0; i < io_num.n_input; i++) {
        input_attrs[i].index = i;
        ret                  = rknn_query(ctx_, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
        if(ret != RKNN_SUCC) {
            std::cout << "input rknn_query failed! error code = " << ret << std::endl;
            return -1;
        }
        dump_tensor_attr(&(input_attrs[i]));
    }

    // Get Model Output Info

    rknn_tensor_attr output_attrs[io_num.n_output];

    memset(output_attrs, 0, sizeof(output_attrs));

    for(int i = 0; i < io_num.n_output; i++) {
        output_attrs[i].index = i;
        ret                   = rknn_query(ctx_, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));

        if(ret != RKNN_SUCC) {
            std::cout << "output rknn_query fail! error code = " << ret << std::endl;
            return -1;
        }

        dump_tensor_attr(&(output_attrs[i]));
    }

    // Set to context
    app_ctx_.rknn_ctx = ctx_;

    if(output_attrs[0].qnt_type == RKNN_TENSOR_QNT_AFFINE_ASYMMETRIC && output_attrs[0].type == RKNN_TENSOR_INT8) {
        // std::cout << "this is a quant model" << std::endl;
        app_ctx_.is_quant = true;
    } else {
        // std::cout << "this is a float model" << std::endl;
        app_ctx_.is_quant = false;
    }

    app_ctx_.io_num      = io_num;
    app_ctx_.input_attrs = (rknn_tensor_attr *)malloc(io_num.n_input * sizeof(rknn_tensor_attr));
    memcpy(app_ctx_.input_attrs, input_attrs, io_num.n_input * sizeof(rknn_tensor_attr));

    app_ctx_.output_attrs = (rknn_tensor_attr *)malloc(io_num.n_output * sizeof(rknn_tensor_attr));
    memcpy(app_ctx_.output_attrs, output_attrs, io_num.n_output * sizeof(rknn_tensor_attr));

    // 获取模型输入的宽高和通道数
    if(input_attrs[0].fmt == RKNN_TENSOR_NCHW) {
        // std::cout << "model is NCHW input fmt" << std::endl;
        app_ctx_.model_channel = input_attrs[0].dims[1];
        app_ctx_.model_height  = input_attrs[0].dims[2];
        app_ctx_.model_width   = input_attrs[0].dims[3];
    } else {
        // std::cout << "model is NHWC input fmt" << std::endl;
        app_ctx_.model_height  = input_attrs[0].dims[1];
        app_ctx_.model_width   = input_attrs[0].dims[2];
        app_ctx_.model_channel = input_attrs[0].dims[3];
    }

    if(!is_copy) {
        std::cout << "sdk version: " << version.api_version << " driver version: " << version.drv_version << std::endl;
        std::cout << "model input num: " << io_num.n_input << ", and output num: " << io_num.n_output << std::endl;
        std::cout << "input tensors:" << std::endl;
        std::cout << "output tensors:" << std::endl;
        std::cout << "model input height=" << app_ctx_.model_height << ", width=" << app_ctx_.model_width
                  << ", channel=" << app_ctx_.model_channel << std::endl;
    }

    // 初始化输入输出参数
    inputs_  = std::make_unique<rknn_input[]>(app_ctx_.io_num.n_input);
    outputs_ = std::make_unique<rknn_output[]>(app_ctx_.io_num.n_output);

    inputs_[0].index = 0;
    inputs_[0].type  = RKNN_TENSOR_UINT8;
    inputs_[0].fmt   = RKNN_TENSOR_NHWC;
    inputs_[0].size  = app_ctx_.model_width * app_ctx_.model_height * app_ctx_.model_channel;
    inputs_[0].buf   = nullptr;

    return 0;
}

Facenet::~Facenet()
{
    deinit();
}

int Facenet::deinit()
{
    if(app_ctx_.rknn_ctx != 0) {
        std::cout << "rknn_destroy" << std::endl;
        rknn_destroy(app_ctx_.rknn_ctx);
        app_ctx_.rknn_ctx = 0;
    }
    if(app_ctx_.input_attrs != nullptr) {
        std::cout << "free input_attrs" << std::endl;
        free(app_ctx_.input_attrs);
    }
    if(app_ctx_.output_attrs != nullptr) {
        std::cout << "free output_attrs" << std::endl;
        free(app_ctx_.output_attrs);
    }

    return 0;
}

rknn_context * Facenet::get_rknn_context()
{
    return &(this->ctx_);
}

int Facenet::inference(void * image_buf, std::vector<float> & out_fp32, letterbox_t letter_box)
{
    // image_buf 是 RGB888 格式，width 和 height 是模型输入的宽高
    inputs_[0].buf = image_buf;

    int ret = rknn_inputs_set(app_ctx_.rknn_ctx, app_ctx_.io_num.n_input, inputs_.get());
    if(ret < 0) {
        std::cout << "rknn_input_set failed! error code = " << ret << std::endl;
        return -1;
    }

    ret = rknn_run(app_ctx_.rknn_ctx, nullptr);
    if(ret != RKNN_SUCC) {
        std::cout << "rknn_run failed, error code = " << ret << std::endl;
        return -1;
    }

    for(int i = 0; i < app_ctx_.io_num.n_output; i++) {
        outputs_[i].index      = i;
        outputs_[i].want_float = 0;
    }

    outputs_lock_.lock();

    ret = rknn_outputs_get(app_ctx_.rknn_ctx, app_ctx_.io_num.n_output, outputs_.get(), nullptr);

    if(ret != RKNN_SUCC) {
        std::cout << "rknn_outputs_get failed, error code = " << ret << std::endl;
        return -1;
    }

    uint8_t * output_data = (uint8_t *)outputs_[0].buf;

    output_normalization(&app_ctx_, output_data, out_fp32);

    // TODO
    // float distance = get_duclidean_distance(old_out_fp32, out_fp32.data());

    // Remeber to release rknn outputs_
    rknn_outputs_release(app_ctx_.rknn_ctx, app_ctx_.io_num.n_output, outputs_.get());

    outputs_lock_.unlock();

    return 0;
}

int Facenet::get_model_width()
{
    return app_ctx_.model_width;
}

int Facenet::get_model_height()
{
    return app_ctx_.model_height;
}

// ===================================================================================================================

Retinaface::Retinaface()
{}

int Retinaface::init(rknn_context * ctx_in, bool is_copy)
{
    int model_len = 0;
    char * model;
    int ret   = 0;
    model_len = read_data_from_file(RETINA_FACE_MODEL_PATH, &model);

    if(model == nullptr) {
        std::cout << "Load model failed" << std::endl;
        return -1;
    }

    if(is_copy) {
        ret = rknn_dup_context(ctx_in, &ctx_);
        if(ret != RKNN_SUCC) {
            std::cout << "rknn_dup_context failed! error code = " << ret << std::endl;
            return -1;
        }
    } else {
        std::cout << "rknn_init() is called" << std::endl;
        ret = rknn_init(&ctx_, model, model_len, 0, NULL);
        free(model);
        if(ret != RKNN_SUCC) {
            std::cout << "rknn_init failed! error code = " << ret << std::endl;
            return -1;
        }
    }

    rknn_core_mask core_mask;

    switch(get_core_num()) {
        case 0: core_mask = RKNN_NPU_CORE_0; break;
        case 1: core_mask = RKNN_NPU_CORE_1; break;
        case 2: core_mask = RKNN_NPU_CORE_2; break;
    }

    ret = rknn_set_core_mask(ctx_, core_mask);

    if(ret < 0) {
        std::cout << "rknn_set_core_mask failed! error code = " << ret << std::endl;
        return -1;
    }

    rknn_sdk_version version;

    ret = rknn_query(ctx_, RKNN_QUERY_SDK_VERSION, &version, sizeof(rknn_sdk_version));
    if(ret < 0) {
        return -1;
    }

    // Get Model Input Output Number
    rknn_input_output_num io_num;

    ret = rknn_query(ctx_, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));

    if(ret != RKNN_SUCC) {
        std::cout << "rknn_query failed! error code = " << ret << std::endl;
        return -1;
    }

    // Get Model Input Info

    rknn_tensor_attr input_attrs[io_num.n_input]; // 这里使用的是变长数组，不建议这么使用

    memset(input_attrs, 0, sizeof(input_attrs));

    for(int i = 0; i < io_num.n_input; i++) {
        input_attrs[i].index = i;
        ret                  = rknn_query(ctx_, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
        if(ret != RKNN_SUCC) {
            std::cout << "input rknn_query failed! error code = " << ret << std::endl;
            return -1;
        }
        dump_tensor_attr(&(input_attrs[i]));
    }

    // Get Model Output Info

    rknn_tensor_attr output_attrs[io_num.n_output];

    memset(output_attrs, 0, sizeof(output_attrs));

    for(int i = 0; i < io_num.n_output; i++) {
        output_attrs[i].index = i;
        ret                   = rknn_query(ctx_, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));

        if(ret != RKNN_SUCC) {
            std::cout << "output rknn_query fail! error code = " << ret << std::endl;
            return -1;
        }

        dump_tensor_attr(&(output_attrs[i]));
    }

    // Set to context
    app_ctx_.rknn_ctx = ctx_;

    if(output_attrs[0].qnt_type == RKNN_TENSOR_QNT_AFFINE_ASYMMETRIC && output_attrs[0].type == RKNN_TENSOR_INT8) {
        // std::cout << "this is a quant model" << std::endl;
        app_ctx_.is_quant = true;
    } else {
        // std::cout << "this is a float model" << std::endl;
        app_ctx_.is_quant = false;
    }

    app_ctx_.io_num      = io_num;
    app_ctx_.input_attrs = (rknn_tensor_attr *)malloc(io_num.n_input * sizeof(rknn_tensor_attr));
    memcpy(app_ctx_.input_attrs, input_attrs, io_num.n_input * sizeof(rknn_tensor_attr));

    app_ctx_.output_attrs = (rknn_tensor_attr *)malloc(io_num.n_output * sizeof(rknn_tensor_attr));
    memcpy(app_ctx_.output_attrs, output_attrs, io_num.n_output * sizeof(rknn_tensor_attr));

    // 获取模型输入的宽高和通道数
    if(input_attrs[0].fmt == RKNN_TENSOR_NCHW) {
        // std::cout << "model is NCHW input fmt" << std::endl;
        app_ctx_.model_channel = input_attrs[0].dims[1];
        app_ctx_.model_height  = input_attrs[0].dims[2];
        app_ctx_.model_width   = input_attrs[0].dims[3];
    } else {
        // std::cout << "model is NHWC input fmt" << std::endl;
        app_ctx_.model_height  = input_attrs[0].dims[1];
        app_ctx_.model_width   = input_attrs[0].dims[2];
        app_ctx_.model_channel = input_attrs[0].dims[3];
    }

    if(!is_copy) {
        std::cout << "sdk version: " << version.api_version << " driver version: " << version.drv_version << std::endl;
        std::cout << "model input num: " << io_num.n_input << ", and output num: " << io_num.n_output << std::endl;
        std::cout << "input tensors:" << std::endl;
        std::cout << "output tensors:" << std::endl;
        std::cout << "model input height=" << app_ctx_.model_height << ", width=" << app_ctx_.model_width
                  << ", channel=" << app_ctx_.model_channel << std::endl;
    }

    // 初始化输入输出参数
    inputs_  = std::make_unique<rknn_input[]>(app_ctx_.io_num.n_input);
    outputs_ = std::make_unique<rknn_output[]>(app_ctx_.io_num.n_output);

    inputs_[0].index = 0;
    inputs_[0].type  = RKNN_TENSOR_UINT8;
    inputs_[0].fmt   = RKNN_TENSOR_NHWC;
    inputs_[0].size  = app_ctx_.model_width * app_ctx_.model_height * app_ctx_.model_channel;
    inputs_[0].buf   = nullptr;

    return 0;
}

Retinaface::~Retinaface()
{
    deinit();
}

int Retinaface::deinit()
{
    if(app_ctx_.rknn_ctx != 0) {
        std::cout << "rknn_destroy" << std::endl;
        rknn_destroy(app_ctx_.rknn_ctx);
        app_ctx_.rknn_ctx = 0;
    }
    if(app_ctx_.input_attrs != nullptr) {
        std::cout << "free input_attrs" << std::endl;
        free(app_ctx_.input_attrs);
    }
    if(app_ctx_.output_attrs != nullptr) {
        std::cout << "free output_attrs" << std::endl;
        free(app_ctx_.output_attrs);
    }

    return 0;
}

rknn_context * Retinaface::get_rknn_context()
{
    return &(this->ctx_);
}

int Retinaface::inference(void * image_buf, retinaface_result * results, letterbox_t letter_box)
{
    // image_buf 是 RGB888 格式，width 和 height 是模型输入的宽高
    inputs_[0].buf = image_buf;

    int ret = rknn_inputs_set(app_ctx_.rknn_ctx, app_ctx_.io_num.n_input, inputs_.get());
    if(ret < 0) {
        std::cout << "rknn_input_set failed! error code = " << ret << std::endl;
        return -1;
    }

    ret = rknn_run(app_ctx_.rknn_ctx, nullptr);
    if(ret != RKNN_SUCC) {
        std::cout << "rknn_run failed, error code = " << ret << std::endl;
        return -1;
    }

    for(int i = 0; i < app_ctx_.io_num.n_output; i++) {
        outputs_[i].index      = i;
        outputs_[i].want_float = 1;
        // outputs_[i].want_float = (!app_ctx_.is_quant);
    }

    outputs_lock_.lock();

    ret = rknn_outputs_get(app_ctx_.rknn_ctx, app_ctx_.io_num.n_output, outputs_.get(), nullptr);

    if(ret != RKNN_SUCC) {
        std::cout << "rknn_outputs_get failed, error code = " << ret << std::endl;
        return -1;
    }

    // Post Process
    retinaface_post_process(&app_ctx_, outputs_.get(), &letter_box, results);

    // Remeber to release rknn outputs_
    rknn_outputs_release(app_ctx_.rknn_ctx, app_ctx_.io_num.n_output, outputs_.get());

    outputs_lock_.unlock();

    return 0;
}

int Retinaface::get_model_width()
{
    return app_ctx_.model_width;
}

int Retinaface::get_model_height()
{
    return app_ctx_.model_height;
}
