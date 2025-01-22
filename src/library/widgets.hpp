#ifndef _WIDGETS_H
#define _WIDGETS_H

#include <src/libs/ffmpeg/lv_ffmpeg.h>
#include <src/others/file_explorer/lv_file_explorer.h>
#include <src/widgets/button/lv_button.h>
#include <src/widgets/image/lv_image.h>
#include <src/widgets/slider/lv_slider.h>
#include <src/widgets/spinner/lv_spinner.h>
#include <src/widgets/switch/lv_switch.h>
#include <stdlib.h>
#include <cstdint>
#include <functional>
#include <src/core/lv_obj_style.h>
#include <src/core/lv_obj_style_gen.h>
#include <src/lv_api_map_v8.h>
#include <src/misc/lv_area.h>
#include <src/misc/lv_color.h>
#include <src/misc/lv_style.h>
#include <src/misc/lv_types.h>
#include <src/widgets/label/lv_label.h>

class Widget {
  protected:
    lv_obj_t * obj_;

  public:
    lv_obj_t * get_obj()
    {
        return obj_;
    };

    // size
    Widget & set_size(int32_t width, int32_t height)
    {
        lv_obj_set_size(obj_, width, height);
        return *this;
    };

    Widget & set_width(int32_t width)
    {
        lv_obj_set_width(obj_, width);
        return *this;
    };

    Widget & set_height(int32_t height)
    {
        lv_obj_set_height(obj_, height);
        return *this;
    };

    Widget & set_content_width(int32_t width)
    {
        lv_obj_set_content_width(obj_, width);
        return *this;
    };

    Widget & set_content_height(int32_t height)
    {
        lv_obj_set_content_height(obj_, height);
        return *this;
    };

    // layout
    Widget & set_layout(uint32_t layout)
    {
        lv_obj_set_layout(obj_, layout);
        return *this;
    };

    // position
    Widget & set_pos(int32_t x, int32_t y)
    {
        lv_obj_set_pos(obj_, x, y);
        return *this;
    };

    Widget & set_x(int32_t x)
    {
        lv_obj_set_x(obj_, x);
        return *this;
    };

    Widget & set_y(int32_t y)
    {
        lv_obj_set_y(obj_, y);
        return *this;
    };

    Widget & set_align(lv_align_t align)
    {
        lv_obj_set_align(obj_, align);
        return *this;
    };

    Widget & center()
    {
        lv_obj_center(obj_);
        return *this;
    };

    Widget & align(lv_align_t align, int32_t x_ofs, int32_t y_ofs)
    {
        lv_obj_align(obj_, align, x_ofs, y_ofs);
        return *this;
    };

    Widget & align_to(const lv_obj_t * base, lv_align_t align, int32_t x_ofs, int32_t y_ofs)
    {
        lv_obj_align_to(obj_, base, align, x_ofs, y_ofs);
        return *this;
    };

    // flag
    Widget & add_flag(lv_obj_flag_t flag)
    {
        lv_obj_add_flag(obj_, flag);
        return *this;
    };
    Widget & remove_flag(lv_obj_flag_t flag)
    {
        lv_obj_remove_flag(obj_, flag);
        return *this;
    };

    // state
    Widget & set_state(lv_state_t state, bool v)
    {
        lv_obj_set_state(obj_, state, v);
        return *this;
    };
    Widget & remove_state(lv_state_t state)
    {
        lv_obj_remove_state(obj_, state);
        return *this;
    };

    // state
    Widget & add_state(lv_state_t state)
    {
        lv_obj_add_state(obj_, state);
        return *this;
    };

    // flex
    Widget & set_flex_flow(lv_flex_flow_t flow)
    {
        lv_obj_set_flex_flow(obj_, flow);
        return *this;
    };

    Widget & set_flex_align(lv_flex_align_t main_place, lv_flex_align_t cross_place, lv_flex_align_t track_cross_place)
    {
        lv_obj_set_flex_align(obj_, main_place, cross_place, track_cross_place);
        return *this;
    };

    Widget & set_flex_grow(uint8_t grow)
    {
        lv_obj_set_flex_grow(obj_, grow);
        return *this;
    };

    // style
    Widget & add_style(const lv_style_t * t, lv_style_selector_t selector)
    {
        lv_obj_add_style(obj_, t, selector);
        return *this;
    };

    Widget & remove_style_all()
    {
        lv_obj_remove_style_all(obj_);
        return *this;
    };

    Widget & set_style_bg_color(lv_color_t color, lv_style_selector_t selector)
    {
        lv_obj_set_style_bg_color(obj_, color, selector);
        return *this;
    };

    Widget & set_style_bg_opa(lv_opa_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_bg_opa(obj_, value, selector);
        return *this;
    };

    Widget & set_style_bg_image_src(const void * value, lv_style_selector_t selector)
    {
        lv_obj_set_style_bg_image_src(obj_, value, selector);
        return *this;
    };

    Widget & set_style_bg_image_opa(lv_opa_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_bg_image_opa(obj_, value, selector);
        return *this;
    };

    Widget & set_style_bg_grad_color(lv_color_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_bg_grad_color(obj_, value, selector);
        return *this;
    };

    Widget & set_style_bg_grad_dir(lv_grad_dir_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_bg_grad_dir(obj_, value, selector);
        return *this;
    };

    Widget & set_style_bg_grad_stop(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_bg_grad_stop(obj_, value, selector);
        return *this;
    };

    Widget & set_style_bg_grad(const lv_grad_dsc_t * value, lv_style_selector_t selector)
    {
        lv_obj_set_style_bg_grad(obj_, value, selector);
        return *this;
    };

    Widget & set_style_pad_all(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_pad_all(obj_, value, selector);
        return *this;
    };

    Widget & set_style_pad_hor(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_pad_hor(obj_, value, selector);
        return *this;
    };

    Widget & set_style_pad_ver(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_pad_ver(obj_, value, selector);
        return *this;
    };

    Widget & set_style_pad_top(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_pad_top(obj_, value, selector);
        return *this;
    };

    Widget & set_style_pad_bottom(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_pad_bottom(obj_, value, selector);
        return *this;
    };

    Widget & set_style_pad_left(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_pad_left(obj_, value, selector);
        return *this;
    };

    Widget & set_style_pad_right(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_pad_right(obj_, value, selector);
        return *this;
    };

    Widget & set_style_pad_row(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_pad_row(obj_, value, selector);
        return *this;
    };

    Widget & set_style_pad_column(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_pad_column(obj_, value, selector);
        return *this;
    };

    Widget & set_style_margin_all(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_margin_all(obj_, value, selector);
        return *this;
    };

    Widget & set_style_margin_hor(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_margin_hor(obj_, value, selector);
        return *this;
    };

    Widget & set_style_margin_ver(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_margin_ver(obj_, value, selector);
        return *this;
    };

    Widget & set_style_margin_top(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_margin_top(obj_, value, selector);
        return *this;
    };

    Widget & set_style_margin_bottom(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_margin_bottom(obj_, value, selector);
        return *this;
    };

    Widget & set_style_margin_left(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_margin_left(obj_, value, selector);
        return *this;
    };

    Widget & set_style_margin_right(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_margin_right(obj_, value, selector);
        return *this;
    };

    Widget & set_style_radius(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_radius(obj_, value, selector);
        return *this;
    };

    Widget & set_style_border_color(lv_color_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_border_color(obj_, value, selector);
        return *this;
    };

    Widget & set_style_border_opa(lv_opa_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_border_opa(obj_, value, selector);
        return *this;
    };

    Widget & set_style_border_width(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_border_width(obj_, value, selector);
        return *this;
    };

    Widget & set_style_border_side(lv_border_side_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_border_side(obj_, value, selector);
        return *this;
    };

    Widget & set_style_border_post(bool value, lv_style_selector_t selector)
    {
        lv_obj_set_style_border_post(obj_, value, selector);
        return *this;
    };

    Widget & set_style_transform_scale(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_transform_scale(obj_, value, selector);
        return *this;
    };

    Widget & set_style_transition(const lv_style_transition_dsc_t * value, lv_style_selector_t selector)
    {
        lv_obj_set_style_transition(obj_, value, selector);
        return *this;
    };

    Widget & set_style_outline_width(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_outline_width(obj_, value, selector);
        return *this;
    };

    Widget & set_style_outline_color(lv_color_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_outline_color(obj_, value, selector);
        return *this;
    };

    Widget & set_style_outline_opa(lv_opa_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_outline_opa(obj_, value, selector);
        return *this;
    };

    Widget & set_style_outline_pad(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_outline_pad(obj_, value, selector);
        return *this;
    };

    Widget & set_style_text_color(lv_color_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_text_color(obj_, value, selector);
        return *this;
    };

    Widget & set_style_text_opa(lv_opa_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_text_opa(obj_, value, selector);
        return *this;
    };

    Widget & set_style_text_font(const lv_font_t * value, lv_style_selector_t selector)
    {
        lv_obj_set_style_text_font(obj_, value, selector);
        return *this;
    };

    Widget & set_style_text_letter_space(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_text_letter_space(obj_, value, selector);
        return *this;
    };

    Widget & set_style_text_line_space(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_text_line_space(obj_, value, selector);
        return *this;
    };

    Widget & set_style_text_decor(lv_text_decor_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_text_decor(obj_, value, selector);
        return *this;
    };

    Widget & set_style_text_align(lv_text_align_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_text_align(obj_, value, selector);
        return *this;
    };

    Widget & set_style_clip_corner(bool value, lv_style_selector_t selector)
    {
        lv_obj_set_style_clip_corner(obj_, value, selector);
        return *this;
    };

    /* event */
    lv_event_dsc_t * add_event_cb(lv_event_cb_t event_cb, lv_event_code_t filter, void * user_data)
    {
        return lv_obj_add_event_cb(obj_, event_cb, filter, user_data);
    };
    lv_result_t send_event(lv_event_code_t event_code, void * param)
    {
        return lv_obj_send_event(obj_, event_code, param);
    };
    bool remove_event_cb(lv_event_cb_t event_cb)
    {
        return lv_obj_remove_event_cb(obj_, event_cb);
    };

    /* parent */
    Widget & set_parent(lv_obj_t * parent)
    {
        lv_obj_set_parent(obj_, parent);
        return *this;
    };

    /* delete */
    void del()
    {
        lv_obj_del(obj_);
    };
    void del_async()
    {
        lv_obj_del_async(obj_);
    };

    /* layer */
    void move_foreground()
    {
        lv_obj_move_foreground(obj_);
    };
    void move_background()
    {
        lv_obj_move_background(obj_);
    };

    /* show/hidden */
    void show()
    {
        lv_obj_remove_flag(obj_, LV_OBJ_FLAG_HIDDEN);
    };
    void hide()
    {
        lv_obj_add_flag(obj_, LV_OBJ_FLAG_HIDDEN);
    };

    /* timer */
    void set_timer(uint32_t period, uint32_t repeat_count, std::function<void(lv_timer_t *)> cb)
    {
        lv_timer_t * timer = lv_timer_create(
            [](lv_timer_t * timer) {
                auto cb = (std::function<void(lv_timer_t *)> *)lv_timer_get_user_data(timer);
                (*cb)(timer);
            },
            period, nullptr);
        lv_timer_set_repeat_count(timer, repeat_count);
        lv_timer_set_user_data(timer, new std::function<void(lv_timer_t *)>(cb));
    };
};

class Object : public Widget {
  public:
    Object(lv_obj_t * parent)
    {
        obj_ = lv_obj_create(parent);
    };
};

class Image : public Widget {
  public:
    Image(lv_obj_t * parent)
    {
        this->obj_ = lv_image_create(parent);
    };
    Image(lv_obj_t * parent, const void * src)
    {
        this->obj_ = lv_image_create(parent);
        this->set_src(src);
    };
    Image & set_src(const void * src)
    {
        lv_image_set_src(this->obj_, src);
        return *this;
    };
    Image & set_pivot(int32_t x, int32_t y)
    {
        lv_image_set_pivot(this->obj_, x, y);
        return *this;
    };
    Image & set_scale(uint32_t zoom)
    {
        lv_image_set_scale(this->obj_, zoom);
        return *this;
    };
    Image & set_scale_x(uint32_t zoom)
    {
        lv_image_set_scale_x(this->obj_, zoom);
        return *this;
    };
    Image & set_scale_y(uint32_t zoom)
    {
        lv_image_set_scale_y(this->obj_, zoom);
        return *this;
    };
};

class Label : public Widget {
  public:
    Label(lv_obj_t * parent)
    {
        this->obj_ = lv_label_create(parent);
    };
    Label(lv_obj_t * parent, const char * text)
    {
        this->obj_ = lv_label_create(parent);
        lv_label_set_text(this->obj_, text);
    };
    Label(lv_obj_t * parent, const char * text, lv_color_t color)
    {
        this->obj_ = lv_label_create(parent);
        lv_label_set_text(this->obj_, text);
        lv_obj_set_style_text_color(this->obj_, color, 0);
    };
    Label & set_text(const char * text)
    {
        lv_label_set_text(this->obj_, text);
        return *this;
    };
    Label & set_text_fmt(const char * fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        lv_label_set_text_fmt(this->obj_, fmt, args);
        va_end(args);
        return *this;
    };
};

class Button : public Widget {
  public:
    Button(lv_obj_t * parent)
    {
        this->obj_ = lv_button_create(parent);
    };
    Button(lv_obj_t * parent, const char * str)
    {
        this->obj_       = lv_button_create(parent);
        lv_obj_t * label = lv_label_create(this->obj_);
        lv_label_set_text(label, str);
        lv_obj_center(label);
    };
};

class Slider : public Widget {
  public:
    Slider(lv_obj_t * parent)
    {
        this->obj_ = lv_slider_create(parent);
    };
    Slider & set_range(int32_t min, int32_t max)
    {
        lv_slider_set_range(this->obj_, min, max);
        return *this;
    };
    Slider & set_value(int32_t value, lv_anim_enable_t anim)
    {
        lv_slider_set_value(this->obj_, value, anim);
        return *this;
    };
    Slider & set_mode(lv_slider_mode_t mode)
    {
        lv_slider_set_mode(this->obj_, mode);
        return *this;
    };
    Slider & set_left_value(int32_t value, lv_anim_enable_t anim)
    {
        lv_slider_set_left_value(this->obj_, value, anim);
        return *this;
    };
};

class Switch : public Widget {
  public:
    Switch(lv_obj_t * parent)
    {
        this->obj_ = lv_switch_create(parent);
    };
};

class Spinner : public Widget {
  public:
    Spinner(lv_obj_t * parent)
    {
        this->obj_ = lv_spinner_create(parent);
    };
    Spinner & set_anim_params(uint32_t t, uint32_t angle)
    {
        lv_spinner_set_anim_params(this->obj_, t, angle);
        return *this;
    };
};

class FileExplorer : public Widget {
  public:
    FileExplorer(lv_obj_t * parent)
    {
        this->obj_ = lv_file_explorer_create(parent);
    };
    FileExplorer & open_dir(const char * path)
    {
        lv_file_explorer_open_dir(this->obj_, path);
        return *this;
    };
    FileExplorer & set_quick_access_path(lv_file_explorer_dir_t dir, const char * path)
    {
        lv_file_explorer_set_quick_access_path(this->obj_, dir, path);
        return *this;
    };
    FileExplorer & set_sort(lv_file_explorer_sort_t sort)
    {
        lv_file_explorer_set_sort(this->obj_, sort);
        return *this;
    };
};

class Bar : public Widget {
  public:
    Bar(lv_obj_t * parent)
    {
        this->obj_ = lv_bar_create(parent);
    };
    Bar & set_value(int32_t value, lv_anim_enable_t anim)
    {
        lv_bar_set_value(this->obj_, value, anim);
        return *this;
    };
    Bar & set_start_value(int32_t start_value, lv_anim_enable_t anim)
    {
        lv_bar_set_start_value(this->obj_, start_value, anim);
        return *this;
    };
    Bar & set_range(int32_t min, int32_t max)
    {
        lv_bar_set_range(this->obj_, min, max);
        return *this;
    };
    Bar & set_mode(lv_bar_mode_t mode)
    {
        lv_bar_set_mode(this->obj_, mode);
        return *this;
    };
};

class FFmpegPlayer : public Widget {
  public:
    FFmpegPlayer(lv_obj_t * parent)
    {
        this->obj_ = lv_ffmpeg_player_create(parent);
    };
    lv_result_t set_src(const char * src)
    {
        return lv_ffmpeg_player_set_src(this->obj_, src);
    };
    FFmpegPlayer & set_auto_restart(bool en)
    {
        lv_ffmpeg_player_set_auto_restart(this->obj_, en);
        return *this;
    };
    FFmpegPlayer & set_cmd(lv_ffmpeg_player_cmd_t cmd)
    {
        lv_ffmpeg_player_set_cmd(this->obj_, cmd);
        return *this;
    };
};

#endif