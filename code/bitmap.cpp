#ifndef BITMAP_C
#define BITMAP_C

static Bitmap
stb_load_image(String8 dir, String8 file){
    Bitmap result = {0};
    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_path_append(scratch.arena, dir, file);

    int x,y,n;
    int ok = stbi_info((char const*)full_path.str, &x, &y, &n);
    result.base = (u8*)stbi_load((char const*)full_path.str, &result.width, &result.width, &result.stride, 0);
//    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
    result.stride = result.width * 4;

    end_scratch(scratch);
    return(result);
}

static BitScanResult
find_first_set_bit(u32 value){
    BitScanResult result = {0};

    for(u32 i=0; i < 32; ++i){
        if(value & (1 << i)){
            result.index = i;
            result.found = true;
            break;
        }
    }
    return(result);
}

// NOTE: This is not a complete BMP loader. Use stb_image.h to load bmps instead if you want something complete
static Bitmap
load_bitmap(Arena* arena, String8 filename){
    Bitmap result = {0};

    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_concatenate(scratch.arena, build_path, filename);
    File file = os_file_open(full_path, GENERIC_READ, OPEN_EXISTING);
    assert_h(file.handle);

    String8 data = os_file_read(scratch.arena, file);
    BitmapHeader *header = (BitmapHeader *)data.str;
    u8* bytes_read = (u8 *)data.str + header->bitmap_offset;

    result.width = header->width;
    result.height = header->height;
    result.stride = header->width * 4;
    result.base = push_array(arena, u8, (u32)(result.stride * result.height));

    //// NOTE: As bmps can have ARGB or RGBA or ..., we need to find where our color
    //// shifts are and position the each 8 bit into a ARGB format.
    BitScanResult red_shift = {0};
    BitScanResult green_shift = {0};
    BitScanResult blue_shift = {0};
    BitScanResult alpha_shift = {0};
    if(header->Compression == 3){
        u32 alpha_mask = ~(header->RedMask | header->GreenMask | header->BlueMask);
        red_shift = find_first_set_bit(header->RedMask);
        green_shift = find_first_set_bit(header->GreenMask);
        blue_shift = find_first_set_bit(header->BlueMask);
        alpha_shift = find_first_set_bit(alpha_mask);

        assert(red_shift.found);
        assert(green_shift.found);
        assert(blue_shift.found);
        assert(alpha_shift.found);
    }

    u8* row = bytes_read;
    s32 row_inc = 1;
    if(result.height > 0){ // if height is positive, swap to top bottom order
        row = bytes_read + ((result.height - 1) * result.stride);
        row_inc = -1;
    }

    u32* dest = (u32*)result.base;
    for(s32 y=0; y < result.height; ++y){
        u32* pixel = (u32*)row;
        for(s32 x=0; x < result.width; ++x){
            u32 color_u32 = *pixel++;
            if(header->Compression == 3){
                color_u32 = ((((*pixel >> alpha_shift.index) & 0xFF) << 24) |
                             (((*pixel >> red_shift.index) & 0xFF)   << 16) |
                             (((*pixel >> green_shift.index) & 0xFF)  << 8) |
                             (((*pixel >> blue_shift.index) & 0xFF)   << 0));
            }

            // u32 to RGBA
            RGBA color = {
                .r = ((f32)((color_u32 >> 16) & 0xFF) / 255.0f),
                .g = ((f32)((color_u32 >> 8) & 0xFF) / 255.0f),
                .b = ((f32)((color_u32 >> 0) & 0xFF) / 255.0f),
                .a = ((f32)((color_u32 >> 24) & 0xFF) / 255.0f),
            };

            // sRGB to linear
            color.r = square_f32(color.r),
            color.g = square_f32(color.g),
            color.b = square_f32(color.b),

            // gamma correction
            color.rgb *= color.a;

            // linear to sRGB
            color.r = sqrt_f32(color.r),
            color.g = sqrt_f32(color.g),
            color.b = sqrt_f32(color.b),

            // write pixel
            *dest++ = (u32)(round_f32_u32(color.a * 255.0f) << 24 |
                            round_f32_u32(color.r * 255.0f) << 16 |
                            round_f32_u32(color.g * 255.0f) << 8  |
                            round_f32_u32(color.b * 255.0f) << 0);
        }
        row = row + (row_inc * result.stride);
    }

    end_scratch(scratch);
    os_file_close(file);
    return(result);
}

#endif
