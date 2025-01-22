/**
 * @file lv_conf.h
 * v9.2.0 的配置文件
 */

/*
 * 将此文件复制为 `lv_conf.h`
 * 1. 直接放在 `lvgl` 文件夹旁边
 * 2. 或放在其他位置并且
 *    - 定义 `LV_CONF_INCLUDE_SIMPLE`
 *    - 添加路径作为包含路径
 */

/* clang-format off */
#if 1 /*启用内容*/
#ifndef LV_CONF_H
#define LV_CONF_H

/*如果需要在此处包含任何内容,请在 `__ASSEMBLY__` 保护内放置*/
#if  0 && defined(__ASSEMBLY__)
#include "my_include.h"
#endif

/*====================
   颜色设置
 *====================*/

/*颜色深度: 1 (I1), 8 (L8), 16 (RGB565), 24 (RGB888), 32 (XRGB8888)*/
#define LV_COLOR_DEPTH 32

/*=========================
   标准库包装器设置
 *=========================*/

/* 可能的值
 * - LV_STDLIB_BUILTIN:     LVGL 的内置实现
 * - LV_STDLIB_CLIB:        标准 C 函数,如 malloc, strlen 等
 * - LV_STDLIB_MICROPYTHON: MicroPython 实现
 * - LV_STDLIB_RTTHREAD:    RT-Thread 实现
 * - LV_STDLIB_CUSTOM:      外部实现这些函数
 */
#define LV_USE_STDLIB_MALLOC    LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_STRING    LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_SPRINTF   LV_STDLIB_BUILTIN

#define LV_STDINT_INCLUDE       <stdint.h>
#define LV_STDDEF_INCLUDE       <stddef.h>
#define LV_STDBOOL_INCLUDE      <stdbool.h>
#define LV_INTTYPES_INCLUDE     <inttypes.h>
#define LV_LIMITS_INCLUDE       <limits.h>
#define LV_STDARG_INCLUDE       <stdarg.h>

#if LV_USE_STDLIB_MALLOC == LV_STDLIB_BUILTIN
    /*`lv_malloc()` 可用的内存大小(字节)(>= 2kB)*/
    #define LV_MEM_SIZE (1024 * 1024)

    /*`lv_malloc()` 的内存扩展大小(字节)*/
    #define LV_MEM_POOL_EXPAND_SIZE 0

    /*为内存池设置一个地址,而不是将其作为普通数组分配。也可以在外部 SRAM 中。*/
    #define LV_MEM_ADR 0     /*0: 未使用*/
    /*不使用地址而是提供一个内存分配器,它将被调用以获取 LVGL 的内存池。例如 my_malloc*/
    #if LV_MEM_ADR == 0
        #undef LV_MEM_POOL_INCLUDE
        #undef LV_MEM_POOL_ALLOC
    #endif
#endif  /*LV_USE_STDLIB_MALLOC == LV_STDLIB_BUILTIN*/

/*====================
   HAL 设置
 *====================*/

/*默认显示刷新、输入设备读取和动画步骤周期。*/
#define LV_DEF_REFR_PERIOD  33      /*[ms]*/

/*默认每英寸点数。用于初始化默认大小,如小部件大小、样式填充。
 *(不太重要,您可以调整它来修改默认大小和空间)*/
#define LV_DPI_DEF 130     /*[px/inch]*/

/*=================
 * 操作系统
 *=================*/
/*选择要使用的操作系统。可能的选项:
 * - LV_OS_NONE
 * - LV_OS_PTHREAD
 * - LV_OS_FREERTOS
 * - LV_OS_CMSIS_RTOS2
 * - LV_OS_RTTHREAD
 * - LV_OS_WINDOWS
 * - LV_OS_MQX
 * - LV_OS_CUSTOM */
#define LV_USE_OS   LV_OS_NONE

#if LV_USE_OS == LV_OS_CUSTOM
    #define LV_OS_CUSTOM_INCLUDE <stdint.h>
#endif

/*========================
 * 渲染配置
 *========================*/

/*将所有图层和图像的步长对齐到这些字节*/
#define LV_DRAW_BUF_STRIDE_ALIGN                1

/*将绘制缓冲区地址的起始地址对齐到这些字节*/
#define LV_DRAW_BUF_ALIGN                       4

/*使用矩阵进行变换。
 *要求:
    `LV_USE_MATRIX = 1`。
    渲染引擎需要支持 3x3 矩阵变换。*/
#define LV_DRAW_TRANSFORM_USE_MATRIX            0

/* 如果小部件具有 `style_opa < 255` (不是 `bg_opa`、`text_opa` 等)或非 NORMAL 混合模式
 * 它会在渲染前被缓冲到一个"简单"图层中。小部件可以分块缓冲。
 * "变换图层"(如果设置了 `transform_angle/zoom`)使用更大的缓冲区
 * 且不能分块绘制。 */

/*简单图层块的目标缓冲区大小*/
#define LV_DRAW_LAYER_SIMPLE_BUF_SIZE    (3 * 1024 * 1024)   /*[bytes]*/

/* 绘制线程的堆栈大小。
 * 注意:如果启用了 FreeType 或 ThorVG,建议将其设置为 32KB 或更大。
 */
#define LV_DRAW_THREAD_STACK_SIZE    (5 * 1024 * 1024)   /*[bytes]*/

#define LV_USE_DRAW_SW 1
#if LV_USE_DRAW_SW == 1

	/*
	 * 选择性地禁用颜色格式支持以减小代码大小。
	 * 注意:某些功能在内部使用特定的颜色格式,例如
	 * - 渐变使用 RGB888
	 * - 带透明度的位图可能使用 ARGB8888
	 */

	#define LV_DRAW_SW_SUPPORT_RGB565		0
	#define LV_DRAW_SW_SUPPORT_RGB565A8		0
	#define LV_DRAW_SW_SUPPORT_RGB888		1
	#define LV_DRAW_SW_SUPPORT_XRGB8888		1
	#define LV_DRAW_SW_SUPPORT_ARGB8888		1
	#define LV_DRAW_SW_SUPPORT_L8			0
	#define LV_DRAW_SW_SUPPORT_AL88			0
	#define LV_DRAW_SW_SUPPORT_A8			0
	#define LV_DRAW_SW_SUPPORT_I1			0

	/* 设置绘制单元的数量。
     * > 1 需要在 `LV_USE_OS` 中启用操作系统
     * > 1 意味着多个线程将并行渲染屏幕 */
    #define LV_DRAW_SW_DRAW_UNIT_CNT    1

    /* 使用 Arm-2D 加速软件渲染 */
    #define LV_USE_DRAW_ARM2D_SYNC      0

    /* 启用原生 helium 汇编编译 */
    #define LV_USE_NATIVE_HELIUM_ASM    0

    /* 0: 使用简单渲染器,只能绘制带渐变的简单矩形、图像、文本和直线
     * 1: 使用复杂渲染器,能够绘制圆角、阴影、倾斜线和圆弧 */
    #define LV_DRAW_SW_COMPLEX          1

    #if LV_DRAW_SW_COMPLEX == 1
        /*允许缓存一些阴影计算。
        *LV_DRAW_SW_SHADOW_CACHE_SIZE 是要缓存的最大阴影大小,其中阴影大小是 `shadow_width + radius`
        *缓存具有 LV_DRAW_SW_SHADOW_CACHE_SIZE^2 RAM 成本*/
        #define LV_DRAW_SW_SHADOW_CACHE_SIZE 0

        /* 设置最大缓存圆形数据的数量。
        * 保存 1/4 圆的周长用于抗锯齿
        * 每个圆使用半径 * 4 字节(保存最常用的半径)
        * 0: 禁用缓存 */
        #define LV_DRAW_SW_CIRCLE_CACHE_SIZE 4
    #endif

    #define  LV_USE_DRAW_SW_ASM     LV_DRAW_SW_ASM_NONE

    #if LV_USE_DRAW_SW_ASM == LV_DRAW_SW_ASM_CUSTOM
        #define  LV_DRAW_SW_ASM_CUSTOM_INCLUDE ""
    #endif

    /* 启用在软件中绘制复杂渐变:角度线性、径向或锥形 */
    #define LV_USE_DRAW_SW_COMPLEX_GRADIENTS    1
#endif

/* 在 iMX RTxxx 平台上使用 NXP 的 VG-Lite GPU。 */
#define LV_USE_DRAW_VGLITE 0

#if LV_USE_DRAW_VGLITE
    /* 启用针对屏幕尺寸 > 352 像素推荐的位块传输质量降级解决方案。 */
    #define LV_USE_VGLITE_BLIT_SPLIT 0

    #if LV_USE_OS
        /* 使用额外的绘制线程进行 VG-Lite 处理。*/
        #define LV_USE_VGLITE_DRAW_THREAD 1

        #if LV_USE_VGLITE_DRAW_THREAD
            /* 启用 VGLite 异步绘制。将多个任务排队并一次性刷新到 GPU。 */
            #define LV_USE_VGLITE_DRAW_ASYNC 1
        #endif
    #endif

    /* 启用 VGLite 断言。 */
    #define LV_USE_VGLITE_ASSERT 0
#endif

/* 在 iMX RTxxx 平台上使用 NXP 的 PXP。 */
#define LV_USE_DRAW_PXP 0

#if LV_USE_DRAW_PXP
    #if LV_USE_OS
        /* 使用额外的绘制线程进行 PXP 处理。*/
        #define LV_USE_PXP_DRAW_THREAD 1
    #endif

    /* 启用 PXP 断言。 */
    #define LV_USE_PXP_ASSERT 0
#endif

/* 在 RA 平台上使用 Renesas Dave2D。 */
#define LV_USE_DRAW_DAVE2D 0

/* 使用缓存的 SDL 纹理绘制*/
#define LV_USE_DRAW_SDL 0

/* 使用 VG-Lite GPU。 */
#define LV_USE_DRAW_VG_LITE 0

#if LV_USE_DRAW_VG_LITE
    /* 启用 VG-Lite 自定义外部 'gpu_init()' 函数 */
    #define LV_VG_LITE_USE_GPU_INIT 0

    /* 启用 VG-Lite 断言。 */
    #define LV_VG_LITE_USE_ASSERT 0

    /* VG-Lite 刷新提交触发阈值。GPU 将尝试批处理这么多绘制任务。 */
    #define LV_VG_LITE_FLUSH_MAX_COUNT 8

    /* 启用边框来模拟阴影
     * 注意:这通常会提高性能,
     * 但不能保证与软件相同的渲染质量。 */
    #define LV_VG_LITE_USE_BOX_SHADOW 0

    /* VG-Lite 渐变最大缓存数量。
     * 注意:单个渐变图像的内存使用量为 4K 字节。
     */
    #define LV_VG_LITE_GRAD_CACHE_CNT 32

    /* VG-Lite 描边最大缓存数量。
     */
    #define LV_VG_LITE_STROKE_CACHE_CNT 32

#endif

/*=======================
 * 功能配置
 *=======================*/

/*-------------
 * 日志
 *-----------*/

/*启用日志模块*/
#define LV_USE_LOG 1
#if LV_USE_LOG

    /*应添加多重要的日志:
    *LV_LOG_LEVEL_TRACE       大量日志以提供详细信息
    *LV_LOG_LEVEL_INFO        记录重要事件
    *LV_LOG_LEVEL_WARN        记录发生意外但未造成问题的情况
    *LV_LOG_LEVEL_ERROR       仅记录关键问题,系统可能会失败
    *LV_LOG_LEVEL_USER        仅记录用户添加的日志
    *LV_LOG_LEVEL_NONE        不记录任何内容*/
    #define LV_LOG_LEVEL LV_LOG_LEVEL_WARN

    /*1: 使用 'printf' 打印日志;
    *0: 用户需要使用 `lv_log_register_print_cb()` 注册回调*/
    #define LV_LOG_PRINTF 1

    /*设置打印日志的回调。
     *例如 `my_print`。原型应为 `void my_print(lv_log_level_t level, const char * buf)`
     *可以通过 `lv_log_register_print_cb` 覆盖*/
    //#define LV_LOG_PRINT_CB

    /*1: 启用打印时间戳;
     *0: 禁用打印时间戳*/
    #define LV_LOG_USE_TIMESTAMP 1

    /*1: 打印日志的文件和行号;
     *0: 不打印日志的文件和行号*/
    #define LV_LOG_USE_FILE_LINE 1


    /*启用/禁用在产生大量日志的模块中的 LV_LOG_TRACE*/
    #define LV_LOG_TRACE_MEM        1
    #define LV_LOG_TRACE_TIMER      1
    #define LV_LOG_TRACE_INDEV      1
    #define LV_LOG_TRACE_DISP_REFR  1
    #define LV_LOG_TRACE_EVENT      1
    #define LV_LOG_TRACE_OBJ_CREATE 1
    #define LV_LOG_TRACE_LAYOUT     1
    #define LV_LOG_TRACE_ANIM       1
    #define LV_LOG_TRACE_CACHE      1

#endif  /*LV_USE_LOG*/

/*-------------
 * 断言
 *-----------*/

/*当操作失败或发现无效数据时启用断言。
 *如果启用了 LV_USE_LOG,将在失败时打印错误消息*/
#define LV_USE_ASSERT_NULL          1   /*检查参数是否为 NULL。(非常快,推荐)*/
#define LV_USE_ASSERT_MALLOC        1   /*检查内存是否成功分配。(非常快,推荐)*/
#define LV_USE_ASSERT_STYLE         0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ           0

/*添加断言发生时的自定义处理程序,例如重启 MCU*/
#define LV_ASSERT_HANDLER_INCLUDE <stdint.h>
#define LV_ASSERT_HANDLER while(1);   /*默认暂停*/

/*-------------
 * 调试
 *-----------*/

/*1: 在重绘区域上绘制随机颜色的矩形*/
#define LV_USE_REFR_DEBUG 0

/*1: 为 ARGB 图层绘制红色叠加层,为 RGB 图层绘制绿色叠加层*/
#define LV_USE_LAYER_DEBUG 0

/*1: 为每个绘制单元的任务绘制不同颜色的叠加层。
 *还在白色背景上添加绘制单元的索引号。
 *对于图层,在黑色背景上添加绘制单元的索引号。*/
#define LV_USE_PARALLEL_DRAW_DEBUG 0

/*-------------
 * 其他
 *-----------*/

#define LV_ENABLE_GLOBAL_CUSTOM 0
#if LV_ENABLE_GLOBAL_CUSTOM
    /*自定义 'lv_global' 函数的包含头文件"*/
    #define LV_GLOBAL_CUSTOM_INCLUDE <stdint.h>
#endif

/*默认缓存大小(字节)。
 *用于图像解码器(如 `lv_lodepng`)在内存中保存解码后的图像。
 *如果大小不设为 0,当缓存已满时解码器将无法解码。
 *如果大小为 0,则不启用缓存功能,解码的内存将在使用后立即释放。*/
#define LV_CACHE_DEF_SIZE       0

/*默认图像头缓存条目数。缓存用于存储图像头
 *主要逻辑与 `LV_CACHE_DEF_SIZE` 相同,但用于图像头。*/
#define LV_IMAGE_HEADER_CACHE_DEF_CNT 0

/*每个渐变允许的停止点数量。增加此值以允许更多停止点。
 *每增加一个停止点会增加(sizeof(lv_color_t) + 1)字节*/
#define LV_GRADIENT_MAX_STOPS   2

/* 调整颜色混合函数的舍入。GPU 可能会以不同方式计算颜色混合(混合)。
 * 0: 向下舍入, 64: 从 x.75 向上舍入, 128: 从一半向上舍入, 192: 从 x.25 向上舍入, 254: 向上舍入 */
#define LV_COLOR_MIX_ROUND_OFS  0

/* 为每个 lv_obj_t 添加 2 个 32 位变量以加快获取样式属性的速度 */
#define LV_OBJ_STYLE_CACHE      1

/* 为 `lv_obj_t` 添加 `id` 字段 */
#define LV_USE_OBJ_ID           0

/* 创建对象时自动分配 ID */
#define LV_OBJ_ID_AUTO_ASSIGN   LV_USE_OBJ_ID

/*使用内置的对象 ID 处理函数:
* - lv_obj_assign_id:       创建小部件时调用。为每个小部件类使用单独的计数器作为 ID。
* - lv_obj_id_compare:      比较 ID 以决定是否与请求的值匹配。
* - lv_obj_stringify_id:    返回例如 "button3"
* - lv_obj_free_id:         不执行任何操作,因为 ID 没有内存分配。
* 禁用时需要由用户实现这些函数。*/
#define LV_USE_OBJ_ID_BUILTIN   1

/*使用对象属性设置/获取 API*/
#define LV_USE_OBJ_PROPERTY 0

/*启用属性名称支持*/
#define LV_USE_OBJ_PROPERTY_NAME 1

/* VG-Lite 模拟器 */
/*需要: LV_USE_THORVG_INTERNAL 或 LV_USE_THORVG_EXTERNAL */
#define LV_USE_VG_LITE_THORVG  0

#if LV_USE_VG_LITE_THORVG

    /*启用 LVGL 的混合模式支持*/
    #define LV_VG_LITE_THORVG_LVGL_BLEND_SUPPORT 0

    /*启用 YUV 颜色格式支持*/
    #define LV_VG_LITE_THORVG_YUV_SUPPORT 0

    /*启用线性渐变扩展支持*/
    #define LV_VG_LITE_THORVG_LINEAR_GRADIENT_EXT_SUPPORT 0

    /*启用 16 像素对齐*/
    #define LV_VG_LITE_THORVG_16PIXELS_ALIGN 1

    /*缓冲区地址对齐*/
    #define LV_VG_LITE_THORVG_BUF_ADDR_ALIGN 64

    /*启用多线程渲染*/
    #define LV_VG_LITE_THORVG_THREAD_RENDER 0

#endif

/*=====================
 *  编译器设置
 *====================*/

/*对于大端系统设置为 1*/
#define LV_BIG_ENDIAN_SYSTEM 0

/*为 `lv_tick_inc` 函数定义自定义属性*/
#define LV_ATTRIBUTE_TICK_INC

/*为 `lv_timer_handler` 函数定义自定义属性*/
#define LV_ATTRIBUTE_TIMER_HANDLER

/*为 `lv_display_flush_ready` 函数定义自定义属性*/
#define LV_ATTRIBUTE_FLUSH_READY

/*缓冲区所需的对齐大小*/
#define LV_ATTRIBUTE_MEM_ALIGN_SIZE 1

/*将在需要对齐内存的地方添加(使用 -Os 时数据可能默认不会对齐到边界)。
 * 例如 __attribute__((aligned(4)))*/
#define LV_ATTRIBUTE_MEM_ALIGN

/*用于标记大型常量数组的属性,例如字体位图*/
#define LV_ATTRIBUTE_LARGE_CONST

/*在 RAM 中声明大数组的编译器前缀*/
#define LV_ATTRIBUTE_LARGE_RAM_ARRAY

/*将性能关键函数放入更快的内存(例如 RAM)*/
#define LV_ATTRIBUTE_FAST_MEM

/*导出整数常量到绑定。此宏用于 LV_<CONST> 形式的常量,
 *这些常量也应出现在 LVGL 绑定 API(如 MicroPython)中。*/
#define LV_EXPORT_CONST_INT(int_value) struct _silence_gcc_warning /*默认值仅防止 GCC 警告*/

/*为所有全局外部数据添加此前缀*/
#define LV_ATTRIBUTE_EXTERN_DATA

/* 使用 `float` 作为 `lv_value_precise_t` */
#define LV_USE_FLOAT            1

/*启用矩阵支持
 *需要 `LV_USE_FLOAT = 1`*/
#define LV_USE_MATRIX           1

/*默认在 `lvgl.h` 中包含 `lvgl_private.h` 以访问内部数据和函数*/
#define LV_USE_PRIVATE_API		0

/*==================
 *   字体使用
 *===================*/

/*带有 ASCII 范围和一些符号的 Montserrat 字体,使用 bpp = 4
 *https://fonts.google.com/specimen/Montserrat*/
#define LV_FONT_MONTSERRAT_8  0
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_26 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_30 1
#define LV_FONT_MONTSERRAT_32 1
#define LV_FONT_MONTSERRAT_34 1
#define LV_FONT_MONTSERRAT_36 1
#define LV_FONT_MONTSERRAT_38 1
#define LV_FONT_MONTSERRAT_40 1
#define LV_FONT_MONTSERRAT_42 1
#define LV_FONT_MONTSERRAT_44 1
#define LV_FONT_MONTSERRAT_46 1
#define LV_FONT_MONTSERRAT_48 1

/*演示特殊功能*/
#define LV_FONT_MONTSERRAT_28_COMPRESSED 1
#define LV_FONT_DEJAVU_16_PERSIAN_HEBREW 1
#define LV_FONT_SIMSUN_14_CJK            0  /*1000 个最常用的 CJK 部首*/
#define LV_FONT_SIMSUN_16_CJK            1

/*像素完美的等宽字体*/
#define LV_FONT_UNSCII_8  1
#define LV_FONT_UNSCII_16 0

/*在此处可选声明自定义字体。
 *您可以将这些字体作为默认字体使用,它们将全局可用。
 *例如 #define LV_FONT_CUSTOM_DECLARE   LV_FONT_DECLARE(my_font_1) LV_FONT_DECLARE(my_font_2)*/
#define LV_FONT_CUSTOM_DECLARE

/*始终设置默认字体*/
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/*启用处理大字体和/或具有大量字符的字体。
 *限制取决于字体大小、字体面和 bpp。
 *如果字体需要它,将触发编译器错误。*/
#define LV_FONT_FMT_TXT_LARGE 0

/*启用/禁用压缩字体支持。*/
#define LV_USE_FONT_COMPRESSED 0

/*启用在找不到字形描述符时绘制占位符*/
#define LV_USE_FONT_PLACEHOLDER 1

/*=================
 *  文本设置
 *=================*/

/**
 * 为字符串选择字符编码。
 * 您的 IDE 或编辑器应该使用相同的字符编码
 * - LV_TXT_ENC_UTF8
 * - LV_TXT_ENC_ASCII
 */
#define LV_TXT_ENC LV_TXT_ENC_UTF8

/*可以在这些字符处断行(换行)*/
#define LV_TXT_BREAK_CHARS " ,.;:-_)]}"

/*如果一个单词至少这么长,将在"最美"的地方断行
 *要禁用,设置为 <= 0 的值*/
#define LV_TXT_LINE_BREAK_LONG_LEN 0

/*断行前在长单词中放置的最小字符数。
 *取决于 LV_TXT_LINE_BREAK_LONG_LEN。*/
#define LV_TXT_LINE_BREAK_LONG_PRE_MIN_LEN 3

/*断行后在长单词中放置的最小字符数。
 *取决于 LV_TXT_LINE_BREAK_LONG_LEN。*/
#define LV_TXT_LINE_BREAK_LONG_POST_MIN_LEN 3

/*支持双向文本。允许混合从左到右和从右到左的文本。
 *方向将根据 Unicode 双向算法进行处理:
 *https://www.w3.org/International/articles/inline-bidi-markup/uba-basics*/
#define LV_USE_BIDI 0
#if LV_USE_BIDI
    /*Set the default direction. Supported values:
    *`LV_BASE_DIR_LTR` Left-to-Right
    *`LV_BASE_DIR_RTL` Right-to-Left
    *`LV_BASE_DIR_AUTO` detect texts base direction*/
    #define LV_BIDI_BASE_DIR_DEF LV_BASE_DIR_AUTO
#endif

/*启用阿拉伯语/波斯语处理
 *在这些语言中,字符应根据其在文本中的位置替换为另一种形式*/
#define LV_USE_ARABIC_PERSIAN_CHARS 0

/*==================
 * 组件
 *================*/

/*组件文档: https://docs.lvgl.io/latest/en/html/widgets/index.html*/

#define LV_WIDGETS_HAS_DEFAULT_VALUE  1

#define LV_USE_ANIMIMG    1

#define LV_USE_ARC        1

#define LV_USE_BAR        1

#define LV_USE_BUTTON        1

#define LV_USE_BUTTONMATRIX  1

#define LV_USE_CALENDAR   1
#if LV_USE_CALENDAR
    #define LV_CALENDAR_WEEK_STARTS_MONDAY 0
    #if LV_CALENDAR_WEEK_STARTS_MONDAY
        #define LV_CALENDAR_DEFAULT_DAY_NAMES {"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"}
    #else
        #define LV_CALENDAR_DEFAULT_DAY_NAMES {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"}
    #endif

    #define LV_CALENDAR_DEFAULT_MONTH_NAMES {"January", "February", "March",  "April", "May",  "June", "July", "August", "September", "October", "November", "December"}
    #define LV_USE_CALENDAR_HEADER_ARROW 1
    #define LV_USE_CALENDAR_HEADER_DROPDOWN 1
    #define LV_USE_CALENDAR_CHINESE 0
#endif  /*LV_USE_CALENDAR*/

#define LV_USE_CANVAS     0

#define LV_USE_CHART      0

#define LV_USE_CHECKBOX   0

#define LV_USE_DROPDOWN   1   /*需要: lv_label*/

#define LV_USE_IMAGE      1   /*需要: lv_label*/

#define LV_USE_IMAGEBUTTON     1

#define LV_USE_KEYBOARD   1

#define LV_USE_LABEL      1
#if LV_USE_LABEL
    #define LV_LABEL_TEXT_SELECTION 1 /*启用标签文本选择*/
    #define LV_LABEL_LONG_TXT_HINT 1  /*在标签中存储一些额外信息以加速绘制非常长的文本*/
    #define LV_LABEL_WAIT_CHAR_COUNT 3  /*等待字符的数量*/
#endif

#define LV_USE_LED        0

#define LV_USE_LINE       1

#define LV_USE_LIST       1

#define LV_USE_LOTTIE     0

#define LV_USE_MENU       1

#define LV_USE_MSGBOX     1

#define LV_USE_ROLLER     1   /*需要: lv_label*/

#define LV_USE_SCALE      1

#define LV_USE_SLIDER     1   /*需要: lv_bar*/

#define LV_USE_SPAN       1
#if LV_USE_SPAN
    /*一行文本可以包含的最大span描述符数量*/
    #define LV_SPAN_SNIPPET_STACK_SIZE 64
#endif

#define LV_USE_SPINBOX    1

#define LV_USE_SPINNER    1

#define LV_USE_SWITCH     1

#define LV_USE_TEXTAREA   1   /*需要: lv_label*/
#if LV_USE_TEXTAREA != 0
    #define LV_TEXTAREA_DEF_PWD_SHOW_TIME 1500    /*毫秒*/
#endif

#define LV_USE_TABLE      1

#define LV_USE_TABVIEW    1

#define LV_USE_TILEVIEW   1

#define LV_USE_WIN        0

/*==================
 * 主题
 *==================*/

/*一个简单、令人印象深刻且非常完整的主题*/
#define LV_USE_THEME_DEFAULT 1
#if LV_USE_THEME_DEFAULT

    /*0: 浅色模式; 1: 深色模式*/
    #define LV_THEME_DEFAULT_DARK 0

    /*1: 启用按下时增大效果*/
    #define LV_THEME_DEFAULT_GROW 1

    /*默认过渡时间[毫秒]*/
    #define LV_THEME_DEFAULT_TRANSITION_TIME 80
#endif /*LV_USE_THEME_DEFAULT*/

/*一个非常简单的主题,是自定义主题的良好起点*/
#define LV_USE_THEME_SIMPLE 1

/*为单色显示器设计的主题*/
#define LV_USE_THEME_MONO 1

/*==================
 * 布局
 *==================*/

/*类似CSS中Flexbox的布局*/
#define LV_USE_FLEX 1

/*类似CSS中Grid的布局*/
#define LV_USE_GRID 1

/*====================
 * 第三方库
 *====================*/

/*常用API的文件系统接口*/

/*设置默认驱动器字母允许在文件路径中跳过驱动器前缀*/
#define LV_FS_DEFAULT_DRIVE_LETTER '\0'

/*fopen、fread等的API*/
#define LV_USE_FS_STDIO 1
#if LV_USE_FS_STDIO
    #define LV_FS_STDIO_LETTER 'A'
    #define LV_FS_STDIO_PATH "/root"         /*设置工作目录。文件/目录路径将附加到其后*/
    #define LV_FS_STDIO_CACHE_SIZE 0    /*>0 在lv_fs_read()中缓存这么多字节*/
#endif

/*open、read等的API*/
#define LV_USE_FS_POSIX 0
#if LV_USE_FS_POSIX
    #define LV_FS_POSIX_LETTER '\0'     /*设置驱动器可访问的大写字母(如'A')*/
    #define LV_FS_POSIX_PATH ""         /*设置工作目录。文件/目录路径将附加到其后*/
    #define LV_FS_POSIX_CACHE_SIZE 0    /*>0 在lv_fs_read()中缓存这么多字节*/
#endif

/*CreateFile、ReadFile等的API*/
#define LV_USE_FS_WIN32 0
#if LV_USE_FS_WIN32
    #define LV_FS_WIN32_LETTER '\0'     /*设置驱动器可访问的大写字母(如'A')*/
    #define LV_FS_WIN32_PATH ""         /*设置工作目录。文件/目录路径将附加到其后*/
    #define LV_FS_WIN32_CACHE_SIZE 0    /*>0 在lv_fs_read()中缓存这么多字节*/
#endif

/*FATFS的API(需要单独添加)。使用f_open、f_read等*/
#define LV_USE_FS_FATFS 0
#if LV_USE_FS_FATFS
    #define LV_FS_FATFS_LETTER '\0'     /*设置驱动器可访问的大写字母(如'A')*/
    #define LV_FS_FATFS_CACHE_SIZE 0    /*>0 在lv_fs_read()中缓存这么多字节*/
#endif

/*内存映射文件访问的API*/
#define LV_USE_FS_MEMFS 0
#if LV_USE_FS_MEMFS
    #define LV_FS_MEMFS_LETTER '\0'     /*设置驱动器可访问的大写字母(如'A')*/
#endif

/*LittleFs的API*/
#define LV_USE_FS_LITTLEFS 0
#if LV_USE_FS_LITTLEFS
    #define LV_FS_LITTLEFS_LETTER '\0'     /*设置驱动器可访问的大写字母(如'A')*/
#endif

/*Arduino LittleFs的API*/
#define LV_USE_FS_ARDUINO_ESP_LITTLEFS 0
#if LV_USE_FS_ARDUINO_ESP_LITTLEFS
    #define LV_FS_ARDUINO_ESP_LITTLEFS_LETTER '\0'     /*设置驱动器可访问的大写字母(如'A')*/
#endif

/*Arduino Sd的API*/
#define LV_USE_FS_ARDUINO_SD 0
#if LV_USE_FS_ARDUINO_SD
    #define LV_FS_ARDUINO_SD_LETTER '\0'     /*设置驱动器可访问的大写字母(如'A')*/
#endif

/*LODEPNG解码器库*/
#define LV_USE_LODEPNG 1

/*PNG解码器(libpng)库*/
#define LV_USE_LIBPNG 0

/*BMP解码器库*/
#define LV_USE_BMP 1

/*JPG + 分割JPG解码器库
 *分割JPG是为嵌入式系统优化的自定义格式*/
#define LV_USE_TJPGD 1

/*libjpeg-turbo解码器库
 *支持完整的JPEG规范和高性能JPEG解码*/
#define LV_USE_LIBJPEG_TURBO 0

/*GIF解码器库*/
#define LV_USE_GIF 0
#if LV_USE_GIF
    /*GIF解码器加速*/
    #define LV_GIF_CACHE_DECODE_DATA 0
#endif

/*将二进制图像解码到RAM*/
#define LV_BIN_DECODER_RAM_LOAD 1

/*RLE解压缩库*/
#define LV_USE_RLE 0

/*二维码库*/
#define LV_USE_QRCODE 0

/*条形码库*/
#define LV_USE_BARCODE 0

/*FreeType库*/
#define LV_USE_FREETYPE 1
#if LV_USE_FREETYPE
    /*让FreeType使用LVGL内存和文件移植*/
    #define LV_FREETYPE_USE_LVGL_PORT 1

    /*FreeType中字形的缓存数量。表示可以缓存的字形数量。
     *值越高,使用的内存就越多*/
    #define LV_FREETYPE_CACHE_FT_GLYPH_CNT 512
#endif

/*内置TTF解码器*/
#define LV_USE_TINY_TTF 1
#if LV_USE_TINY_TTF
    /*启用从文件加载TTF数据*/
    #define LV_TINY_TTF_FILE_SUPPORT 0
    #define LV_TINY_TTF_CACHE_GLYPH_CNT 256
#endif

/*Rlottie库*/
#define LV_USE_RLOTTIE 0

/*启用矢量图形API
 *需要`LV_USE_MATRIX = 1`*/
#define LV_USE_VECTOR_GRAPHIC  0

/*从src/libs文件夹启用ThorVG(矢量图形库)*/
#define LV_USE_THORVG_INTERNAL 0

/*假设已安装并链接到项目来启用ThorVG*/
#define LV_USE_THORVG_EXTERNAL 0

/*使用lvgl内置的LZ4库*/
#define LV_USE_LZ4_INTERNAL  0

/*使用外部LZ4库*/
#define LV_USE_LZ4_EXTERNAL  0

/*FFmpeg库用于图像解码和播放视频
 *支持所有主要图像格式,因此不要与其他图像解码器一起启用*/
#define LV_USE_FFMPEG 1
#if LV_USE_FFMPEG
    /*将输入信息转储到stderr*/
    #define LV_FFMPEG_DUMP_FORMAT 0
#endif

/*==================
 * 其他
 *==================*/

/*1: 启用API以获取对象快照*/
#define LV_USE_SNAPSHOT 0

/*1: 启用系统监视器组件*/
#define LV_USE_SYSMON   0
#if LV_USE_SYSMON
    /*获取空闲百分比。例如 uint32_t my_get_idle(void);*/
    #define LV_SYSMON_GET_IDLE lv_timer_get_idle

    /*1: 显示CPU使用率和FPS计数
     * 需要`LV_USE_SYSMON = 1`*/
    #define LV_USE_PERF_MONITOR 1
    #if LV_USE_PERF_MONITOR
        #define LV_USE_PERF_MONITOR_POS LV_ALIGN_BOTTOM_RIGHT

        /*0: 在屏幕上显示性能数据, 1: 使用日志打印性能数据*/
        #define LV_USE_PERF_MONITOR_LOG_MODE 0
    #endif

    /*1: 显示已使用的内存和内存碎片
     * 需要`LV_USE_STDLIB_MALLOC = LV_STDLIB_BUILTIN`
     * 需要`LV_USE_SYSMON = 1`*/
    #define LV_USE_MEM_MONITOR 0
    #if LV_USE_MEM_MONITOR
        #define LV_USE_MEM_MONITOR_POS LV_ALIGN_BOTTOM_LEFT
    #endif

#endif /*LV_USE_SYSMON*/

/*1: 启用运行时性能分析器*/
#define LV_USE_PROFILER 0
#if LV_USE_PROFILER
    /*1: 启用内置分析器*/
    #define LV_USE_PROFILER_BUILTIN 1
    #if LV_USE_PROFILER_BUILTIN
        /*默认分析器跟踪缓冲区大小*/
        #define LV_PROFILER_BUILTIN_BUF_SIZE (16 * 1024)     /*[字节]*/
    #endif

    /*要包含的分析器头文件*/
    #define LV_PROFILER_INCLUDE "lvgl/src/misc/lv_profiler_builtin.h"

    /*分析器起点函数*/
    #define LV_PROFILER_BEGIN    LV_PROFILER_BUILTIN_BEGIN

    /*分析器终点函数*/
    #define LV_PROFILER_END      LV_PROFILER_BUILTIN_END

    /*带自定义标签的分析器起点函数*/
    #define LV_PROFILER_BEGIN_TAG LV_PROFILER_BUILTIN_BEGIN_TAG

    /*带自定义标签的分析器终点函数*/
    #define LV_PROFILER_END_TAG   LV_PROFILER_BUILTIN_END_TAG
#endif

/*1: 启用Monkey测试*/
#define LV_USE_MONKEY 0

/*1: 启用网格导航*/
#define LV_USE_GRIDNAV 0

/*1: 启用lv_obj片段*/
#define LV_USE_FRAGMENT 0

/*1: 支持在标签或span组件中使用图像作为字体*/
#define LV_USE_IMGFONT 1

/*1: 启用观察者模式实现*/
#define LV_USE_OBSERVER 0

/*1: 启用拼音输入法*/
/*需要: lv_keyboard*/
#define LV_USE_IME_PINYIN 0
#if LV_USE_IME_PINYIN
    /*1: 使用默认词库*/
    /*如果不使用默认词库,请确保在设置词库后使用`lv_ime_pinyin`*/
    #define LV_IME_PINYIN_USE_DEFAULT_DICT 1
    /*设置可以显示的候选面板的最大数量*/
    /*这需要根据屏幕大小进行调整*/
    #define LV_IME_PINYIN_CAND_TEXT_NUM 6

    /*使用9键输入(k9)*/
    #define LV_IME_PINYIN_USE_K9_MODE      1
    #if LV_IME_PINYIN_USE_K9_MODE == 1
        #define LV_IME_PINYIN_K9_CAND_TEXT_NUM 3
    #endif /*LV_IME_PINYIN_USE_K9_MODE*/
#endif

/*1: 启用文件浏览器*/
/*需要: lv_table*/
#define LV_USE_FILE_EXPLORER                     1
#if LV_USE_FILE_EXPLORER
    /*路径的最大长度*/
    #define LV_FILE_EXPLORER_PATH_MAX_LEN        (128)
    /*快速访问栏, 1:使用, 0:不使用*/
    /*需要: lv_list*/
    #define LV_FILE_EXPLORER_QUICK_ACCESS        1
#endif

/*==================
 * 设备
 *==================*/

/*使用SDL在PC上打开窗口并处理鼠标和键盘*/
#define LV_USE_SDL              0
#if LV_USE_SDL
    #define LV_SDL_INCLUDE_PATH     <SDL2/SDL.h>
    #define LV_SDL_RENDER_MODE      LV_DISPLAY_RENDER_MODE_DIRECT   /*推荐使用LV_DISPLAY_RENDER_MODE_DIRECT以获得最佳性能*/
    #define LV_SDL_BUF_COUNT        1    /*1或2*/
    #define LV_SDL_ACCELERATED      1    /*1: 使用硬件加速*/
    #define LV_SDL_FULLSCREEN       0    /*1: 默认使窗口全屏*/
    #define LV_SDL_DIRECT_EXIT      1    /*1: 当所有SDL窗口关闭时退出应用程序*/
    #define LV_SDL_MOUSEWHEEL_MODE  LV_SDL_MOUSEWHEEL_MODE_ENCODER  /*LV_SDL_MOUSEWHEEL_MODE_ENCODER/CROWN*/
#endif

/*使用X11在Linux桌面上打开窗口并处理鼠标和键盘*/
#define LV_USE_X11              0
#if LV_USE_X11
    #define LV_X11_DIRECT_EXIT         1  /*当所有X11窗口关闭时退出应用程序*/
    #define LV_X11_DOUBLE_BUFFER       1  /*使用双缓冲进行渲染*/
    /*仅选择以下渲染模式之一(推荐LV_X11_RENDER_MODE_PARTIAL!)*/
    #define LV_X11_RENDER_MODE_PARTIAL 1  /*部分渲染模式(推荐)*/
    #define LV_X11_RENDER_MODE_DIRECT  0  /*直接渲染模式*/
    #define LV_X11_RENDER_MODE_FULL    0  /*完整渲染模式*/
#endif

/*使用Wayland在Linux或BSD桌面上打开窗口并处理输入*/
#define LV_USE_WAYLAND          0
#if LV_USE_WAYLAND
    #define LV_WAYLAND_WINDOW_DECORATIONS   0    /*仅在Mutter/GNOME上需要绘制客户端窗口装饰*/
    #define LV_WAYLAND_WL_SHELL             0    /*使用传统的wl_shell协议而不是默认的XDG shell*/
#endif

/*用于/dev/fb的驱动程序*/
#define LV_USE_LINUX_FBDEV      0
#if LV_USE_LINUX_FBDEV
    #define LV_LINUX_FBDEV_BSD           0
    #define LV_LINUX_FBDEV_RENDER_MODE   LV_DISPLAY_RENDER_MODE_PARTIAL
    #define LV_LINUX_FBDEV_BUFFER_COUNT  0
    #define LV_LINUX_FBDEV_BUFFER_SIZE   60
#endif

/*使用Nuttx打开窗口并处理触摸屏*/
#define LV_USE_NUTTX    0

#if LV_USE_NUTTX
    #define LV_USE_NUTTX_LIBUV    0

    /*使用Nuttx自定义初始化API打开窗口并处理触摸屏*/
    #define LV_USE_NUTTX_CUSTOM_INIT    0

    /*用于/dev/lcd的驱动程序*/
    #define LV_USE_NUTTX_LCD      0
    #if LV_USE_NUTTX_LCD
        #define LV_NUTTX_LCD_BUFFER_COUNT    0
        #define LV_NUTTX_LCD_BUFFER_SIZE     60
    #endif

    /*用于/dev/input的驱动程序*/
    #define LV_USE_NUTTX_TOUCHSCREEN    0

#endif

/*用于/dev/dri/card的驱动程序*/
#define LV_USE_LINUX_DRM        1

/*TFT_eSPI的接口*/
#define LV_USE_TFT_ESPI         0

/*用于evdev输入设备的驱动程序*/
#define LV_USE_EVDEV    1

/*用于libinput输入设备的驱动程序*/
#define LV_USE_LIBINPUT    0

#if LV_USE_LIBINPUT
    #define LV_LIBINPUT_BSD    0

    /*完整键盘支持*/
    #define LV_LIBINPUT_XKB             0
    #if LV_LIBINPUT_XKB
        /*"setxkbmap -query"可以帮助找到键盘的正确值*/
        #define LV_LIBINPUT_XKB_KEY_MAP { .rules = NULL, .model = "pc101", .layout = "us", .variant = NULL, .options = NULL }
    #endif
#endif

/*通过SPI/并口连接的LCD设备驱动程序*/
#define LV_USE_ST7735        0
#define LV_USE_ST7789        0
#define LV_USE_ST7796        0
#define LV_USE_ILI9341       0

#define LV_USE_GENERIC_MIPI (LV_USE_ST7735 | LV_USE_ST7789 | LV_USE_ST7796 | LV_USE_ILI9341)

/*瑞萨GLCD驱动程序*/
#define LV_USE_RENESAS_GLCDC    0

/*LVGL Windows后端*/
#define LV_USE_WINDOWS    0

/*使用OpenGL在PC上打开窗口并处理鼠标和键盘*/
#define LV_USE_OPENGLES   0
#if LV_USE_OPENGLES
    #define LV_USE_OPENGLES_DEBUG        1    /*启用或禁用opengles调试*/
#endif

/*QNX Screen显示和输入驱动程序*/
#define LV_USE_QNX              0
#if LV_USE_QNX
    #define LV_QNX_BUF_COUNT        1    /*1或2*/
#endif

/*==================
* 示例
*==================*/

/*启用示例与库一起构建*/
#define LV_BUILD_EXAMPLES 0

/*===================
 * 演示使用
 ====================*/

/*显示一些组件。可能需要增加`LV_MEM_SIZE`*/
#define LV_USE_DEMO_WIDGETS 0

/*演示编码器和键盘的使用*/
#define LV_USE_DEMO_KEYPAD_AND_ENCODER 0

/*对系统进行基准测试*/
#define LV_USE_DEMO_BENCHMARK 0

/*为每个基本图形进行渲染测试。需要至少480x272显示器*/
#define LV_USE_DEMO_RENDER 0

/*LVGL压力测试*/
#define LV_USE_DEMO_STRESS 0

/*音乐播放器演示*/
#define LV_USE_DEMO_MUSIC 0
#if LV_USE_DEMO_MUSIC
    #define LV_DEMO_MUSIC_SQUARE    0
    #define LV_DEMO_MUSIC_LANDSCAPE 0
    #define LV_DEMO_MUSIC_ROUND     0
    #define LV_DEMO_MUSIC_LARGE     0
    #define LV_DEMO_MUSIC_AUTO_PLAY 0
#endif

/*Flex布局演示*/
#define LV_USE_DEMO_FLEX_LAYOUT     0

/*智能手机类多语言演示*/
#define LV_USE_DEMO_MULTILANG       0

/*组件变换演示*/
#define LV_USE_DEMO_TRANSFORM       0

/*演示滚动设置*/
#define LV_USE_DEMO_SCROLL          0

/*矢量图形演示*/
#define LV_USE_DEMO_VECTOR_GRAPHIC  0

/*--LV_CONF_H结束--*/

#endif /*LV_CONF_H*/

#endif /*"Content enable"结束*/
