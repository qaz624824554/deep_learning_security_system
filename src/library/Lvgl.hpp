#pragma once

#include <src/core/lv_obj_tree.h>
#include <src/draw/lv_image_dsc.h>
#include <src/libs/ffmpeg/lv_ffmpeg.h>
#include <src/libs/freetype/lv_freetype.h>
#include <src/misc/lv_anim.h>
#include <src/misc/lv_async.h>
#include <src/misc/lv_event.h>
#include <src/misc/lv_timer.h>
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
#include <src/misc/lv_area.h>
#include <src/misc/lv_color.h>
#include <src/misc/lv_style.h>
#include <src/misc/lv_types.h>
#include <src/widgets/label/lv_label.h>

struct CallbackData
{
    std::function<void(void *)> cb;
    void * user_data;
};

struct EventCallbackData
{
    std::function<void(lv_event_t *, void *)> cb;
    void * user_data;
};

struct TimerCallbackData
{
    std::function<void(lv_timer_t *, void *)> cb;
    void * user_data;
};

class LvWidget {
  protected:
    lv_obj_t * obj_;
    virtual ~LvWidget() = 0;

  public:
    lv_obj_t * raw()
    {
        return obj_;
    };

    // delete

    LvWidget & delete_delayed(uint32_t delay_ms)
    {
        lv_obj_delete_delayed(obj_, delay_ms);
        return *this;
    };

    void del_async()
    {
        lv_obj_delete_async(obj_);
    };

    LvWidget & delete_obj()
    {
        lv_obj_delete(obj_);
        return *this;
    };

    LvWidget & clean()
    {
        lv_obj_clean(obj_);
        return *this;
    };

    // size
    LvWidget & set_size(int32_t width, int32_t height)
    {
        lv_obj_set_size(obj_, width, height);
        return *this;
    };

    LvWidget & set_width(int32_t width)
    {
        lv_obj_set_width(obj_, width);
        return *this;
    };

    LvWidget & set_height(int32_t height)
    {
        lv_obj_set_height(obj_, height);
        return *this;
    };

    LvWidget & set_content_width(int32_t width)
    {
        lv_obj_set_content_width(obj_, width);
        return *this;
    };

    LvWidget & set_content_height(int32_t height)
    {
        lv_obj_set_content_height(obj_, height);
        return *this;
    };

    // layout
    LvWidget & set_layout(uint32_t layout)
    {
        lv_obj_set_layout(obj_, layout);
        return *this;
    };

    // position
    LvWidget & set_pos(int32_t x, int32_t y)
    {
        lv_obj_set_pos(obj_, x, y);
        return *this;
    };

    LvWidget & set_x(int32_t x)
    {
        lv_obj_set_x(obj_, x);
        return *this;
    };

    LvWidget & set_y(int32_t y)
    {
        lv_obj_set_y(obj_, y);
        return *this;
    };

    LvWidget & set_align(lv_align_t align)
    {
        lv_obj_set_align(obj_, align);
        return *this;
    };

    LvWidget & center()
    {
        lv_obj_center(obj_);
        return *this;
    };

    LvWidget & align(lv_align_t align, int32_t x_ofs, int32_t y_ofs)
    {
        lv_obj_align(obj_, align, x_ofs, y_ofs);
        return *this;
    };

    LvWidget & align_to(const lv_obj_t * base, lv_align_t align, int32_t x_ofs, int32_t y_ofs)
    {
        lv_obj_align_to(obj_, base, align, x_ofs, y_ofs);
        return *this;
    };

    // flag
    LvWidget & add_flag(lv_obj_flag_t flag)
    {
        lv_obj_add_flag(obj_, flag);
        return *this;
    };
    LvWidget & remove_flag(lv_obj_flag_t flag)
    {
        lv_obj_remove_flag(obj_, flag);
        return *this;
    };

    bool has_flag(lv_obj_flag_t flag)
    {
        return lv_obj_has_flag(obj_, flag);
    }

    // state
    LvWidget & set_state(lv_state_t state, bool v)
    {
        lv_obj_set_state(obj_, state, v);
        return *this;
    };
    LvWidget & remove_state(lv_state_t state)
    {
        lv_obj_remove_state(obj_, state);
        return *this;
    };

    // state
    LvWidget & add_state(lv_state_t state)
    {
        lv_obj_add_state(obj_, state);
        return *this;
    };

    // flex
    LvWidget & set_flex_flow(lv_flex_flow_t flow)
    {
        lv_obj_set_flex_flow(obj_, flow);
        return *this;
    };

    LvWidget & set_flex_align(lv_flex_align_t main_place, lv_flex_align_t cross_place,
                              lv_flex_align_t track_cross_place)
    {
        lv_obj_set_flex_align(obj_, main_place, cross_place, track_cross_place);
        return *this;
    };

    LvWidget & set_flex_grow(uint8_t grow)
    {
        lv_obj_set_flex_grow(obj_, grow);
        return *this;
    };

    // style
    LvWidget & add_style(const lv_style_t * t, lv_style_selector_t selector)
    {
        lv_obj_add_style(obj_, t, selector);
        return *this;
    };

    LvWidget & remove_style_all()
    {
        lv_obj_remove_style_all(obj_);
        return *this;
    };

    LvWidget & set_style_bg_color(lv_color_t color, lv_style_selector_t selector)
    {
        lv_obj_set_style_bg_color(obj_, color, selector);
        return *this;
    };

    LvWidget & set_style_bg_opa(lv_opa_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_bg_opa(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_bg_image_src(const void * value, lv_style_selector_t selector)
    {
        lv_obj_set_style_bg_image_src(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_bg_image_opa(lv_opa_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_bg_image_opa(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_bg_grad_color(lv_color_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_bg_grad_color(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_bg_grad_dir(lv_grad_dir_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_bg_grad_dir(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_bg_grad_stop(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_bg_grad_stop(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_bg_grad(const lv_grad_dsc_t * value, lv_style_selector_t selector)
    {
        lv_obj_set_style_bg_grad(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_pad_all(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_pad_all(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_pad_hor(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_pad_hor(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_pad_ver(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_pad_ver(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_pad_top(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_pad_top(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_pad_bottom(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_pad_bottom(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_pad_left(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_pad_left(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_pad_right(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_pad_right(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_pad_row(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_pad_row(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_pad_column(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_pad_column(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_margin_all(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_margin_all(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_margin_hor(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_margin_hor(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_margin_ver(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_margin_ver(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_margin_top(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_margin_top(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_margin_bottom(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_margin_bottom(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_margin_left(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_margin_left(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_margin_right(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_margin_right(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_radius(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_radius(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_border_color(lv_color_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_border_color(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_border_opa(lv_opa_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_border_opa(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_border_width(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_border_width(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_border_side(lv_border_side_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_border_side(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_border_post(bool value, lv_style_selector_t selector)
    {
        lv_obj_set_style_border_post(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_transform_scale(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_transform_scale(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_transition(const lv_style_transition_dsc_t * value, lv_style_selector_t selector)
    {
        lv_obj_set_style_transition(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_outline_width(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_outline_width(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_outline_color(lv_color_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_outline_color(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_outline_opa(lv_opa_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_outline_opa(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_outline_pad(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_outline_pad(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_text_color(lv_color_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_text_color(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_text_opa(lv_opa_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_text_opa(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_text_font(const lv_font_t * value, lv_style_selector_t selector)
    {
        lv_obj_set_style_text_font(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_text_letter_space(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_text_letter_space(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_text_line_space(int32_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_text_line_space(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_text_decor(lv_text_decor_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_text_decor(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_text_align(lv_text_align_t value, lv_style_selector_t selector)
    {
        lv_obj_set_style_text_align(obj_, value, selector);
        return *this;
    };

    LvWidget & set_style_clip_corner(bool value, lv_style_selector_t selector)
    {
        lv_obj_set_style_clip_corner(obj_, value, selector);
        return *this;
    };

    /* event */
    lv_event_dsc_t * add_event_cb(std::function<void(lv_event_t *, void *)> event_cb, lv_event_code_t filter,
                                  void * user_data)
    {

        auto cb_data = new EventCallbackData{event_cb, user_data};

        return lv_obj_add_event_cb(
            obj_,
            [](lv_event_t * e) {
                auto data = (EventCallbackData *)lv_event_get_user_data(e);
                data->cb(e, data->user_data);
            },
            filter, cb_data);
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
    LvWidget & set_parent(lv_obj_t * parent)
    {
        lv_obj_set_parent(obj_, parent);
        return *this;
    };

    /* layer */
    // void move_foreground()
    // {
    //     lv_obj_mo(obj_);
    // };
    // void move_background()
    // {
    //     lv_obj_move_background(obj_);
    // };

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

inline LvWidget::~LvWidget()
{}

class LvFont {
  protected:
    lv_font_t * font_;

  public:
    lv_font_t * raw()
    {
        return font_;
    };
};

class LvObject : public LvWidget {
  public:
    LvObject(lv_obj_t * parent)
    {
        obj_ = lv_obj_create(parent);
    };
};

class LvImage : public LvWidget {
  public:
    LvImage(lv_obj_t * parent)
    {
        this->obj_ = lv_image_create(parent);
    };
    LvImage(lv_obj_t * parent, const void * src)
    {
        this->obj_ = lv_image_create(parent);
        this->set_src(src);
    };
    LvImage & set_src(const void * src)
    {
        lv_image_set_src(this->obj_, src);
        return *this;
    };
    LvImage & set_pivot(int32_t x, int32_t y)
    {
        lv_image_set_pivot(this->obj_, x, y);
        return *this;
    };
    LvImage & set_scale(uint32_t zoom)
    {
        lv_image_set_scale(this->obj_, zoom);
        return *this;
    };
    LvImage & set_scale_x(uint32_t zoom)
    {
        lv_image_set_scale_x(this->obj_, zoom);
        return *this;
    };
    LvImage & set_scale_y(uint32_t zoom)
    {
        lv_image_set_scale_y(this->obj_, zoom);
        return *this;
    };
};

class LvLabel : public LvWidget {
  public:
    LvLabel(lv_obj_t * parent)
    {
        this->obj_ = lv_label_create(parent);
    };
    LvLabel(lv_obj_t * parent, const char * text)
    {
        this->obj_ = lv_label_create(parent);
        lv_label_set_text(this->obj_, text);
    };
    LvLabel(lv_obj_t * parent, const char * text, lv_color_t color)
    {
        this->obj_ = lv_label_create(parent);
        lv_label_set_text(this->obj_, text);
        lv_obj_set_style_text_color(this->obj_, color, 0);
    };
    LvLabel & set_text(const char * text)
    {
        lv_label_set_text(this->obj_, text);
        return *this;
    };
    LvLabel & set_text_fmt(const char * fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        lv_label_set_text_fmt(this->obj_, fmt, args);
        va_end(args);
        return *this;
    };
};

class LvButton : public LvWidget {
  public:
    LvButton(lv_obj_t * parent)
    {
        this->obj_ = lv_button_create(parent);
    };
    LvButton(lv_obj_t * parent, const char * str)
    {
        this->obj_       = lv_button_create(parent);
        lv_obj_t * label = lv_label_create(this->obj_);
        lv_label_set_text(label, str);
        lv_obj_center(label);
    };
};

class LvSlider : public LvWidget {
  public:
    LvSlider(lv_obj_t * parent)
    {
        this->obj_ = lv_slider_create(parent);
    };
    LvSlider & set_range(int32_t min, int32_t max)
    {
        lv_slider_set_range(this->obj_, min, max);
        return *this;
    };
    LvSlider & set_value(int32_t value, lv_anim_enable_t anim)
    {
        lv_slider_set_value(this->obj_, value, anim);
        return *this;
    };
    LvSlider & set_mode(lv_slider_mode_t mode)
    {
        lv_slider_set_mode(this->obj_, mode);
        return *this;
    };
    LvSlider & set_left_value(int32_t value, lv_anim_enable_t anim)
    {
        lv_slider_set_left_value(this->obj_, value, anim);
        return *this;
    };
};

class LvSwitch : public LvWidget {
  public:
    LvSwitch(lv_obj_t * parent)
    {
        this->obj_ = lv_switch_create(parent);
    };
};

class LvSpinner : public LvWidget {
  public:
    LvSpinner(lv_obj_t * parent)
    {
        this->obj_ = lv_spinner_create(parent);
    };
    LvSpinner & set_anim_params(uint32_t t, uint32_t angle)
    {
        lv_spinner_set_anim_params(this->obj_, t, angle);
        return *this;
    };
};

class LvFileExplorer : public LvWidget {
  public:
    LvFileExplorer(lv_obj_t * parent)
    {
        this->obj_ = lv_file_explorer_create(parent);
    };
    LvFileExplorer & open_dir(const char * path)
    {
        lv_file_explorer_open_dir(this->obj_, path);
        return *this;
    };
    LvFileExplorer & set_quick_access_path(lv_file_explorer_dir_t dir, const char * path)
    {
        lv_file_explorer_set_quick_access_path(this->obj_, dir, path);
        return *this;
    };
    LvFileExplorer & set_sort(lv_file_explorer_sort_t sort)
    {
        lv_file_explorer_set_sort(this->obj_, sort);
        return *this;
    };
};

class LvBar : public LvWidget {
  public:
    LvBar(lv_obj_t * parent)
    {
        this->obj_ = lv_bar_create(parent);
    };
    LvBar & set_value(int32_t value, lv_anim_enable_t anim)
    {
        lv_bar_set_value(this->obj_, value, anim);
        return *this;
    };
    LvBar & set_start_value(int32_t start_value, lv_anim_enable_t anim)
    {
        lv_bar_set_start_value(this->obj_, start_value, anim);
        return *this;
    };
    LvBar & set_range(int32_t min, int32_t max)
    {
        lv_bar_set_range(this->obj_, min, max);
        return *this;
    };
    LvBar & set_mode(lv_bar_mode_t mode)
    {
        lv_bar_set_mode(this->obj_, mode);
        return *this;
    };
};

class LvFFmpegPlayer : public LvWidget {
  public:
    LvFFmpegPlayer(lv_obj_t * parent)
    {
        this->obj_ = lv_ffmpeg_player_create(parent);
    };
    lv_result_t set_src(const char * src)
    {
        return lv_ffmpeg_player_set_src(this->obj_, src);
    };
    LvFFmpegPlayer & set_auto_restart(bool en)
    {
        lv_ffmpeg_player_set_auto_restart(this->obj_, en);
        return *this;
    };
    LvFFmpegPlayer & set_cmd(lv_ffmpeg_player_cmd_t cmd)
    {
        lv_ffmpeg_player_set_cmd(this->obj_, cmd);
        return *this;
    };
};

class LvScreenActive : public LvWidget {
  public:
    LvScreenActive()
    {
        this->obj_ = lv_screen_active();
    };
};

class LvFreetypeFont : public LvFont {
  public:
    LvFreetypeFont(const char * pathname, lv_freetype_font_render_mode_t render_mode, uint32_t size,
                   lv_freetype_font_style_t style)
    {
        this->font_ = lv_freetype_font_create(pathname, render_mode, size, style);
    };
};

class LvImageDsc {
  private:
    lv_image_dsc_t dsc;

  public:
    lv_image_dsc_t * raw()
    {
        return &dsc;
    };
};

class LvGradDsc {
  private:
    lv_grad_dsc_t dsc;

  public:
    lv_grad_dsc_t * raw()
    {
        return &dsc;
    };
};

class LvStyleTransitionDsc {
  private:
    lv_style_transition_dsc_t dsc;

  public:
    lv_style_transition_dsc_t * raw()
    {
        return &dsc;
    };
};

class LvTimer {
  private:
    lv_timer_t * timer_;

  public:
    LvTimer(std::function<void(lv_timer_t *, void *)> timer_xcb, uint32_t period, void * user_data)
    {

        auto cb_data = new TimerCallbackData{timer_xcb, user_data};
        timer_       = lv_timer_create(
            [](lv_timer_t * timer) {
                auto data = (TimerCallbackData *)lv_timer_get_user_data(timer);
                data->cb(timer, data->user_data);
            },
            period, cb_data);
    };

    lv_timer_t * raw()
    {
        return timer_;
    }

    LvTimer & set_repeat_count(uint32_t count)
    {
        lv_timer_set_repeat_count(timer_, count);
        return *this;
    };

    LvTimer & del()
    {
        lv_timer_delete(timer_);
        return *this;
    };
    LvTimer & pause()
    {
        lv_timer_pause(timer_);
        return *this;
    };

    LvTimer & resume()
    {
        lv_timer_resume(timer_);
        return *this;
    }
};

class LvAsync {
  private:
    LvAsync() = delete;

  public:
    static void call(std::function<void()> cb)
    {
        lv_async_call(
            [](void * data) {
                auto cb = static_cast<std::function<void()> *>(data);
                (*cb)();
            },
            &cb);
    }
};

class LvAnimation {
  private:
    lv_anim_t anim;
    uint32_t repeat_cnt;
    bool is_set_repeat_cnt = false;

  public:
    LvAnimation() {
        lv_anim_init(&this->anim);
    };
    LvAnimation & set_var(lv_obj_t * obj)
    {
        lv_anim_set_var(&anim, obj);
        return *this;
    };
    LvAnimation & set_exec_cb(lv_anim_exec_xcb_t exec_cb)
    {
        lv_anim_set_exec_cb(&anim, exec_cb);
        return *this;
    };
    LvAnimation & set_duration(uint32_t duration)
    {
        lv_anim_set_duration(&anim, duration);
        return *this;
    };
    LvAnimation & set_time(uint32_t duration)
    {
        lv_anim_set_time(&anim, duration);
        return *this;
    };
    LvAnimation & set_delay(uint32_t delay)
    {
        lv_anim_set_delay(&anim, delay);
        return *this;
    };
    LvAnimation & set_values(int32_t start, int32_t end)
    {
        lv_anim_set_values(&anim, start, end);
        return *this;
    };
    LvAnimation & set_custom_exec_cb(lv_anim_custom_exec_cb_t exec_cb)
    {
        lv_anim_set_custom_exec_cb(&anim, exec_cb);
        return *this;
    };
    LvAnimation & set_path_cb(lv_anim_path_cb_t path_cb)
    {
        lv_anim_set_path_cb(&anim, path_cb);
        return *this;
    };
    LvAnimation & set_start_cb(lv_anim_start_cb_t start_cb)
    {
        lv_anim_set_start_cb(&anim, start_cb);
        return *this;
    };
    LvAnimation & set_get_value_cb(lv_anim_get_value_cb_t get_value_cb)
    {
        lv_anim_set_get_value_cb(&anim, get_value_cb);
        return *this;
    };
    LvAnimation & set_completed_cb(lv_anim_completed_cb_t completed_cb)
    {
        lv_anim_set_completed_cb(&anim, completed_cb);
        return *this;
    };
    LvAnimation & set_deleted_cb(lv_anim_deleted_cb_t deleted_cb)
    {
        lv_anim_set_deleted_cb(&anim, deleted_cb);
        return *this;
    };
    LvAnimation & set_playback_duration(uint32_t duration)
    {
        lv_anim_set_playback_duration(&anim, duration);
        return *this;
    };
    LvAnimation & set_playback_time(uint32_t duration)
    {
        lv_anim_set_playback_time(&anim, duration);
        return *this;
    };
    LvAnimation & set_playback_delay(uint32_t delay)
    {
        lv_anim_set_playback_delay(&anim, delay);
        return *this;
    };
    LvAnimation & set_repeat_count(uint32_t cnt)
    {
        if(!this->is_set_repeat_cnt) {
            this->is_set_repeat_cnt = true;
            this->repeat_cnt        = cnt;
        }
        lv_anim_set_repeat_count(&this->anim, cnt);
        return *this;
    };
    LvAnimation & set_repeat_delay(uint32_t delay)
    {
        lv_anim_set_repeat_delay(&anim, delay);
        return *this;
    };
    LvAnimation & set_early_apply(bool en)
    {
        lv_anim_set_early_apply(&anim, en);
        return *this;
    };
    LvAnimation & set_user_data(void * user_data)
    {
        lv_anim_set_user_data(&anim, user_data);
        return *this;
    };
    LvAnimation & set_bezier3_param(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
    {
        lv_anim_set_bezier3_param(&anim, x1, y1, x2, y2);
        return *this;
    };
    LvAnimation & start()
    {
        if(this->is_set_repeat_cnt) {
            lv_anim_set_repeat_count(&this->anim, this->repeat_cnt);
        }
        lv_anim_start(&this->anim);

        return *this;
    };
    LvAnimation & stop()
    {
        lv_anim_set_repeat_count(&this->anim, 0);
        return *this;
    };
};
