#include "Font.hpp"
#include "Lvgl.hpp"
#include "PageManager.hpp"
#include "UI.hpp"
#include <iostream>
#include <src/display/lv_display.h>
#include <src/layouts/flex/lv_flex.h>
#include <src/misc/lv_color.h>

extern "C" {
LV_IMAGE_DECLARE(bg);
LV_IMAGE_DECLARE(face_icon);
LV_IMAGE_DECLARE(security_camera_icon);
}

MainPage::MainPage()
{
    main_screen = new LvObject(nullptr);
    main_screen->set_style_bg_image_src(&bg, 0)
        .set_style_text_font(Font24::get_font(), 0)
        .set_flex_flow(LV_FLEX_FLOW_ROW)
        .set_flex_align(LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    LvObject face_container{main_screen->raw()};
    face_container.set_size(LV_SIZE_CONTENT, LV_SIZE_CONTENT)
        .set_style_bg_opa(LV_OPA_TRANSP, 0)
        .set_style_border_width(0, 0)
        .set_flex_flow(LV_FLEX_FLOW_COLUMN)
        .set_flex_align(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER)
        .add_event_cb(
            [&](lv_event_t * e, void * obj) {
                PageManager::getInstance().switchToPage(PageManager::PageType::ACCESS_CONTROL_PAGE);
            },
            LV_EVENT_CLICKED, nullptr);

    LvImage face_icon_image{face_container.raw()};
    face_icon_image.set_src(&face_icon);

    LvLabel face_icon_label{face_container.raw(), "门禁系统", lv_color_white()};


    LvObject security_camera_container{main_screen->raw()};
    security_camera_container.set_size(LV_SIZE_CONTENT, LV_SIZE_CONTENT)
        .set_style_bg_opa(LV_OPA_TRANSP, 0)
        .set_style_border_width(0, 0)
        .set_flex_flow(LV_FLEX_FLOW_COLUMN)
        .set_flex_align(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER)
        .add_event_cb(
            [&](lv_event_t * e, void * obj) {
                PageManager::getInstance().switchToPage(PageManager::PageType::SECURITY_CAMERA_PAGE);
            },
            LV_EVENT_CLICKED, nullptr);

    LvImage security_camera_icon_image{security_camera_container.raw()};
    security_camera_icon_image.set_src(&security_camera_icon);

    LvLabel security_camera_icon_label{security_camera_container.raw(), "安防摄像头", lv_color_white()};
}

void MainPage::show()
{
    lv_screen_load(main_screen->raw());
}

void MainPage::hide()
{}