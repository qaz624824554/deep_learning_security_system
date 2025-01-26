#include "Font.hpp"
#include <src/libs/freetype/lv_freetype.h>



Font16::Font16()
{
    font_ = std::make_unique<LvFreetypeFont>(FONT_FILE_PATH, LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 16,
                                             LV_FREETYPE_FONT_STYLE_NORMAL);
}

Font24::Font24()
{
    font_ = std::make_unique<LvFreetypeFont>(FONT_FILE_PATH, LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 24,
                                             LV_FREETYPE_FONT_STYLE_NORMAL);
}
