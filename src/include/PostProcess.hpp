#pragma once
#include <stdint.h>

#include "Common.hpp"
#include "rknn_api.h"

int retinaface_post_process(rknn_app_context_t *app_ctx, rknn_output *outputs,
                 letterbox_t *letter_box, retinaface_result *results);
