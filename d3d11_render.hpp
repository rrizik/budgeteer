#ifndef D3D11_RENDER_H
#define D3D11_RENDER_H

static RGBA srgb_to_linear_approx(RGBA value);
static RGBA linear_to_srgb_approx(RGBA value);
static RGBA srgb_to_linear(RGBA value);

typedef enum RenderCommandType{
    RenderCommandType_ClearColor,
    RenderCommandType_Quad,
    RenderCommandType_Line,
    RenderCommandType_Texture,
    RenderCommandType_Text,
} RenderCommandType;

typedef struct RenderCommand{
    RenderCommandType type;
    v2 p0;
    v2 p1;
    v2 p2;
    v2 p3;

    s32 width;

    RGBA color;
	//ID3D11ShaderResourceView** texture;
	u32 texture;

    u32 font_id;
    String8 text;
} RenderCommand;

static void draw_clear_color(Arena* arena, RGBA color);
static void draw_quad(Arena* arena, v2 p0, v2 p1, v2 p2, v2 p3, RGBA color);
static void draw_text(Arena* arena, u32 font_id, String8 text, v2 pos, RGBA color);
static void draw_texture(Arena* arena, v2 p0, v2 p1, v2 p2, v2 p3, RGBA color, ID3D11ShaderResourceView** texture);
static void draw_commands(Arena* commands);

#endif
