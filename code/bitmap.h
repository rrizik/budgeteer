#ifndef BITMAP_H
#define BITMAP_H

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
    #pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
    #pragma clang diagnostic ignored "-Wextra-semi-stmt"
    #define STB_IMAGE_IMPLEMENTATION
    #include "stb_image.h"
    #pragma clang diagnostic pop
#endif

#if COMPILER_CL
    #pragma warning(push, 0)
    #define STB_IMAGE_IMPLEMENTATION
    #include "stb_image.h"
    #pragma warning(pop)
#endif

typedef struct Bitmap{
    u8*  base;
	s32  width;
	s32  height;
	s32  stride;
	s32  channels;
} Bitmap;

static Bitmap stb_load_image(Arena* arena, String8 dir, String8 file, bool vertical_flip=false);

#endif
