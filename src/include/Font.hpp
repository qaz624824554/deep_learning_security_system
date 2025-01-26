#pragma once

#include "Lvgl.hpp"
#include <memory>

#define FONT_FILE_PATH "src/assets/font/font.ttf"

class Font16 {
  private:
    std::unique_ptr<LvFreetypeFont> font_;

  public:
    Font16();
    static lv_font_t * get_font()
    {
        static Font16 font;
        return font.font_->raw();
    }
};

class Font24 {
  private:
    std::unique_ptr<LvFreetypeFont> font_;

  public:
    Font24();
    static lv_font_t * get_font()
    {
        static Font24 font;
        return font.font_->raw();
    }
};
