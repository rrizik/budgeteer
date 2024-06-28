#ifndef FONT_C
#define FONT_C

static Font
load_font_ttf(Arena* arena, String8 filename, f32 size){
    // open file

    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_concatenate(scratch.arena, build_path, filename);
    File file = os_file_open(full_path, GENERIC_READ, OPEN_EXISTING);
    end_scratch(scratch);
    assert_h(file.handle);

    // init font
    Font result = {0};
    String8 file_data =  os_file_read(arena, file); // stb ttf fonts need to remain loaded in memory. todo: ask HMH about how to get around this. Afterall, I've already created the texture.
    if(!stbtt_InitFont(&result.info, (u8*)file_data.str, 0)){
        result.succeed = false;
        return(result);
    }
    result.scale = stbtt_ScaleForPixelHeight(&result.info, size);
    stbtt_GetFontVMetrics(&result.info, &result.ascent, &result.descent, &result.line_gap);
    result.vertical_offset = round_f32_s32((f32)(result.ascent - result.descent + result.line_gap) * result.scale); // note: I don't see value in keeping this in unscaled.

    result.texture_w = 1024;
    result.texture_h = 1024;
    result.first_char = 32;
    result.size = size;

    // u8 data single channel as alpha
    String8 bitmap_a;
    bitmap_a.size = (u64)(result.texture_w * result.texture_h);
    bitmap_a.str = push_array(scratch.arena, u8, bitmap_a.size);

    stbtt_pack_context context;
    if (!stbtt_PackBegin(&context, bitmap_a.str, result.texture_w, result.texture_h, 0, 1, 0)) {
        result.succeed = false;
        return(result);
    }

    stbtt_pack_range range;
    range.chardata_for_range = result.packed_chars;
    range.array_of_unicode_codepoints = NULL; // Indicates that we are using the range
    range.first_unicode_codepoint_in_range = 32;
    range.num_chars = 95;
    range.font_size = size;

    if (!stbtt_PackFontRanges(&context, (u8*)file_data.str, 0, &range, 1)) {
        result.succeed = false;
        return(result);
    }
    stbtt_PackEnd(&context);

    // u32 data 4 channel as rgba
    String8 bitmap_rgba;
    bitmap_rgba.size = (u64)(result.texture_w * result.texture_h * 4);
    bitmap_rgba.str = push_array(scratch.arena, u8, bitmap_rgba.size);
    u32_buffer_from_u8_buffer(&bitmap_a, &bitmap_rgba); // convert from Alpha to RBGA in order to use 1 shader

    D3D11_TEXTURE2D_DESC desc = {
        .Width = (u32)result.texture_w,
        .Height = (u32)result.texture_h,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
        .SampleDesc = {1, 0},
        .Usage = D3D11_USAGE_IMMUTABLE,
        .BindFlags = D3D11_BIND_SHADER_RESOURCE,
    };

    D3D11_SUBRESOURCE_DATA shader_data = {
        .pSysMem = bitmap_rgba.str,
        .SysMemPitch = (u32)(result.texture_w * 4),
    };
    ID3D11Texture2D* texture;
    hr = d3d_device->CreateTexture2D(&desc, &shader_data, &texture);
    assert_hr(hr);
    hr = d3d_device->CreateShaderResourceView(texture, 0, &result.atlas.view);
    assert_hr(hr);

    texture->Release();
    os_file_close(file);
    end_scratch(scratch);
    return(result);
}

static f32
font_char_width(u32 font_id, u8 c){
    Font* font = &tm->assets.fonts[font_id];
    f32 result = 0;
    s32 advance_width, lsb;
    stbtt_GetCodepointHMetrics(&font->info, c, &advance_width, &lsb);
    result = (f32)advance_width * font->scale;
    return(result);
}

static f32
font_string_width(u32 font_id, String8 str){
    Font* font = &tm->assets.fonts[font_id];
    f32 result = 0;
    s32 advance_width, lsb;
    for(s32 i=0; i < str.size; ++i){
        u8 c = str.str[i];
        stbtt_GetCodepointHMetrics(&font->info, c, &advance_width, &lsb);
        result += (f32)advance_width * font->scale;
    }
    return(result);
}

#endif
