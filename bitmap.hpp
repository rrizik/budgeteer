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

// IMPORTANT
// UNTESTED We changed our Rect type to screenspace, and changed all functions. Some tested, some not.
// CLEANUP
#pragma pack(push, 1)
typedef struct BitmapHeader {
    u16 file_type;
	u32 file_size;
	u16 reserved1;
	u16 reserved2;
	u32 bitmap_offset;
	u32 header_size;
	s32 width;
	s32 height;
	u16 planes;
	u16 bits_per_pixel;
    u32 Compression;
	u32 SizeOfBitmap;
	s32 HorzResolution;
	s32 VertResolution;
	u32 ColorsUsed;
	u32 ColorsImportant;
    u32 RedMask;
    u32 GreenMask;
    u32 BlueMask;
} BitmapHeader;
#pragma pack(pop)

typedef struct Bitmap{
    u8*  base;
	s32  width;
	s32  height;
	s32  stride;
} Bitmap;

static Bitmap stb_load_image(String8 dir, String8 file);
typedef struct BitScanResult{
    bool found;
    u32 index;
} BitScanResult;

static BitScanResult find_first_set_bit(u32 value);
static Bitmap load_bitmap(Arena* arena, String8 filename);

#endif
