#ifndef FONT_H
#define FONT_H

// NOTE: Don't use conversion errors on stb_truetype.h
#if COMPILER_CLANG
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wconversion"
    #pragma clang diagnostic ignored "-Wall"
    #pragma clang diagnostic ignored "-Wextra"
    #pragma clang diagnostic ignored "-Wreserved-id-macro"
    #pragma clang diagnostic ignored "-Wcast-qual"
    #pragma clang diagnostic ignored "-Wshadow"
    #pragma clang diagnostic ignored "-Wdouble-promotion"
    #pragma clang diagnostic ignored "-Wreserved-id-macro"
    #pragma clang diagnostic ignored "-Wcast-qual"
    #pragma clang diagnostic ignored "-Wshadow"
    #pragma clang diagnostic ignored "-Wimplicit-fallthrough"
    #pragma clang diagnostic ignored "-Wcomma"

    #define STB_RECT_PACK_IMPLEMENTATION
    #include "stb_rect_pack.h"
    #define STB_TRUETYPE_IMPLEMENTATION
    #include "stb_truetype.h"
    #pragma clang diagnostic pop
#endif

#if COMPILER_CL
    #pragma warning(push, 0)
    #define STB_RECT_PACK_IMPLEMENTATION
    #include "stb_rect_pack.h"
    #define STB_TRUETYPE_IMPLEMENTATION
    #include "stb_truetype.h"
    #pragma warning(pop)
#endif

typedef struct Font{
    stbtt_fontinfo info;
    s32 vertical_offset;
    s32 ascent, descent, line_gap;
    f32 scale;

    s32 first_char;
    s32 texture_w;
    s32 texture_h;
    f32 size;
    stbtt_packedchar packed_chars[256];

    Texture atlas;
    u32 texture;
    bool succeed;
} Font;

static Font load_font_ttf(Arena* arena, String8 path, f32 size);
static f32 font_char_width(u32 font_id, u8 c);
static f32 font_string_width(u32 font_id, String8 str);

#endif
