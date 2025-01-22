#include <src/libs/freetype/lv_freetype.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "lvgl/lvgl.h"
#include "lvgl/src/display/lv_display.h"

extern "C" {
LV_IMAGE_DECLARE(bg);
}

uint16_t window_width;
uint16_t window_height;
bool fullscreen;
bool maximize;

static void configure_simulator(int argc, char ** argv);

static const char * getenv_default(const char * name, const char * dflt)
{
    return getenv(name) ?: dflt;
}

#if LV_USE_EVDEV
static void lv_linux_init_input_pointer(lv_display_t * disp)
{
    /* Enables a pointer (touchscreen/mouse) input device
     * Use 'evtest' to find the correct input device. /dev/input/by-id/ is recommended if possible
     * Use /dev/input/by-id/my-mouse-or-touchscreen or /dev/input/eventX
     */
    const char * input_device = getenv("LV_LINUX_EVDEV_POINTER_DEVICE");

    if(input_device == NULL) {
        fprintf(stderr, "please set the LV_LINUX_EVDEV_POINTER_DEVICE environment variable\n");
        exit(1);
    }

    lv_indev_t * touch = lv_evdev_create(LV_INDEV_TYPE_POINTER, input_device);
    lv_indev_set_display(touch, disp);

    /* Set the cursor icon */
    // LV_IMAGE_DECLARE(mouse_cursor_icon);
    // lv_obj_t * cursor_obj = lv_image_create(lv_screen_active());
    // lv_image_set_src(cursor_obj, &mouse_cursor_icon);
    // lv_indev_set_cursor(touch, cursor_obj);
}
#endif

#if LV_USE_LINUX_FBDEV
static void lv_linux_disp_init(void)
{
    const char * device = getenv_default("LV_LINUX_FBDEV_DEVICE", "/dev/fb0");
    lv_display_t * disp = lv_linux_fbdev_create();

#if LV_USE_EVDEV
    lv_linux_init_input_pointer(disp);
#endif

    lv_linux_fbdev_set_file(disp, device);
}
#elif LV_USE_LINUX_DRM
static void lv_linux_disp_init(void)
{
    const char * device = getenv_default("LV_LINUX_DRM_CARD", "/dev/dri/card0");
    lv_display_t * disp = lv_linux_drm_create();

#if LV_USE_EVDEV
    lv_linux_init_input_pointer(disp);
#endif

    lv_linux_drm_set_file(disp, device, -1);
}
#endif

#if LV_USE_WAYLAND == 0
void lv_linux_run_loop(void)
{
    uint32_t idle_time;

    /*Handle LVGL tasks*/
    while(1) {

        idle_time = lv_timer_handler(); /*Returns the time to the next timer execution*/
        usleep(idle_time * 1000);
    }
}
#endif

/*
 * Process command line arguments and environment
 * variables to configure the simulator
 */
static void configure_simulator(int argc, char ** argv)
{

    int opt  = 0;
    bool err = false;

    /* Default values */
    fullscreen = maximize = false;
    window_width          = atoi(getenv("LV_SIM_WINDOW_WIDTH") ?: "800");
    window_height         = atoi(getenv("LV_SIM_WINDOW_HEIGHT") ?: "480");

    /* Parse the command-line options. */
    while((opt = getopt(argc, argv, "fmw:h:")) != -1) {
        switch(opt) {
            case 'f':
                fullscreen = true;
                if(LV_USE_WAYLAND == 0) {
                    fprintf(stderr, "The SDL driver doesn't support fullscreen mode on start\n");
                    exit(1);
                }
                break;
            case 'm':
                maximize = true;
                if(LV_USE_WAYLAND == 0) {
                    fprintf(stderr, "The SDL driver doesn't support maximized mode on start\n");
                    exit(1);
                }
                break;
            case 'w': window_width = atoi(optarg); break;
            case 'h': window_height = atoi(optarg); break;
            case ':': fprintf(stderr, "Option -%c requires an argument.\n", optopt); exit(1);
            case '?': fprintf(stderr, "Unknown option -%c.\n", optopt); exit(1);
        }
    }
}

int main(int argc, char ** argv)
{
    configure_simulator(argc, argv);

    lv_init();

    lv_linux_disp_init();

    lv_obj_t * screen = lv_screen_active();
    // lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_img_src(screen, &bg, 0);

    lv_font_t * font = lv_freetype_font_create("/root/deep_learning_security_system/src/assets/font/font.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
                                               18, LV_FREETYPE_FONT_STYLE_NORMAL);

    if(!font) {
        LV_LOG_ERROR("freetype font create failed.");
        return 1;
    }

    /*Create style with the new font*/
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, font);
    lv_style_set_text_align(&style, LV_TEXT_ALIGN_CENTER);

    /*Create a label with the new style*/
    lv_obj_t * label = lv_label_create(screen);
    lv_obj_add_style(label, &style, 0);
    lv_label_set_text(label, "Hello world\nI'm a font created with FreeType");
    lv_obj_center(label);

    lv_linux_run_loop();

    return 0;
}
