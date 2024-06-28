#ifndef RENDERER_H
#define RENDERER_H

// NOTE: SIMD
#include <emmintrin.h>

static RGBA
u32_to_rgba_normal(u32 value){
    RGBA result = {0};
	result.a = ((f32)((value >> 24) & 0xFF) / 255.0f);
	result.r = ((f32)((value >> 16) & 0xFF) / 255.0f);
	result.g = ((f32)((value >> 8) & 0xFF) / 255.0f);
	result.b = ((f32)((value >> 0) & 0xFF) / 255.0f);
    return result;
}

static RGBA
u32_to_rgba(u32 value){
    RGBA result = {0};
	result.a = (f32)((value >> 24) & 0xFF);
	result.r = (f32)((value >> 16) & 0xFF);
	result.g = (f32)((value >> 8) & 0xFF);
	result.b = (f32)((value >> 0) & 0xFF);
    return result;
}

static RGBA
srgb_to_linear(RGBA value){
    RGBA result = {
        .r = square_f32(value.r),
        .g = square_f32(value.g),
        .b = square_f32(value.b),
        .a = value.a,
    };
    return(result);
}

static RGBA
linear_to_srgb(RGBA value){
    RGBA result = {
        .r = sqrt_f32(value.r),
        .g = sqrt_f32(value.g),
        .b = sqrt_f32(value.b),
        .a = value.a,
    };
    return(result);
}

typedef enum RenderCommandType{
    RenderCommand_ClearColor,
    RenderCommand_Pixel,
    RenderCommand_Segment,
    RenderCommand_Line,
    RenderCommand_Ray,
    RenderCommand_Rect,
    RenderCommand_Basis,
    RenderCommand_Box,
    RenderCommand_Quad,
    RenderCommand_Triangle,
    RenderCommand_Circle,
    RenderCommand_Bitmap,
} RenderCommandType;

typedef struct CommandHeader{
    RenderCommandType type;
    Rect rect;
    v2  direction;
    v2  origin;
    v2  x_axis;
    v2  y_axis;
    s32 border_size;

    f32 rad;

    RGBA color;
    RGBA border_color;

    v4   clip_region;
    bool fill;

    size_t arena_used;
} CommandHeader;

typedef struct ClearColorCommand{
    CommandHeader ch;
} ClearColorCommand;

typedef struct PixelCommand{
    CommandHeader ch;
} PixelCommand;

typedef struct SegmentCommand{
    CommandHeader ch;
    v2 p0;
    v2 p1;
} SegmentCommand;

typedef struct RayCommand{
    CommandHeader ch;
} RayCommand;

typedef struct LineCommand{
    CommandHeader ch;
} LineCommand;

typedef struct RectCommand{
    CommandHeader ch;
} RectCommand;

typedef struct BasisCommand{
    CommandHeader ch;
    Bitmap texture;
} BasisCommand;

typedef struct BoxCommand{
    CommandHeader ch;
} BoxCommand;

typedef struct QuadCommand{
    CommandHeader ch;
    v2 p0;
    v2 p1;
    v2 p2;
    v2 p3;
} QuadCommand;

typedef struct TriangleCommand{
    CommandHeader ch;
    v2 p0;
    v2 p1;
    v2 p2;
} TriangleCommand;

typedef struct CircleCommand{
    CommandHeader ch;
} CircleCommand;

typedef struct BitmapCommand{
    CommandHeader ch;
    Bitmap texture;
} BitmapCommand;

static void
push_clear_color(Arena *arena, RGBA color){
    ClearColorCommand* command = push_struct(arena, ClearColorCommand);
    command->ch.type = RenderCommand_ClearColor,
    command->ch.arena_used = arena->used;
    command->ch.color = color;
}

static void
push_clear_color_region(Arena *arena, Rect rect, RGBA color){
    ClearColorCommand* command = push_struct(arena, ClearColorCommand);
    command->ch.type = RenderCommand_ClearColor,
    command->ch.color = color;
    command->ch.rect = rect;
    command->ch.arena_used = arena->used;
}

static void
push_pixel(Arena *arena, Rect rect, RGBA color){
    PixelCommand* command = push_struct(arena, PixelCommand);
    command->ch.type = RenderCommand_Pixel;
    command->ch.arena_used = arena->used;
    command->ch.rect = rect;
    command->ch.color = color;
}

static void
push_segment(Arena *arena, v2 p0, v2 p1, RGBA color){
    SegmentCommand* command = push_struct(arena, SegmentCommand);
    command->ch.type = RenderCommand_Segment;
    command->ch.arena_used = arena->used;
    command->ch.color = color;
    command->p0 = p0;
    command->p1 = p1;
}

static void
push_ray(Arena *arena, Rect rect, v2 direction, RGBA color){
    RayCommand* command = push_struct(arena, RayCommand);
    command->ch.type = RenderCommand_Ray;
    command->ch.arena_used = arena->used;
    command->ch.rect = rect;
    command->ch.direction = direction;
    command->ch.color = color;
}

static void
push_line(Arena *arena, Rect rect, v2 direction, RGBA color){
    LineCommand* command = push_struct(arena, LineCommand);
    command->ch.type = RenderCommand_Line;
    command->ch.arena_used = arena->used;
    command->ch.rect = rect;
    command->ch.color = color;
    command->ch.direction = direction;
}

static void
push_rect(Arena *arena, Rect rect, RGBA color, s32 border_size = 0, RGBA border_color = {0, 0, 0, 0}){
    RectCommand* command = push_struct(arena, RectCommand);
    command->ch.type = RenderCommand_Rect;
    command->ch.arena_used = arena->used;
    command->ch.rect = rect;
    command->ch.color = color;
    command->ch.border_size = border_size;
    command->ch.border_color = border_color;
}

static void
push_basis(Arena *arena, v2 origin, v2 x_axis, v2 y_axis, Bitmap texture, RGBA color = {0, 0, 0, 1}){
    BasisCommand* command = push_struct(arena, BasisCommand);
    command->ch.type = RenderCommand_Basis;
    command->ch.arena_used = arena->used;
    command->ch.color = color;
    command->ch.origin = origin;
    command->ch.x_axis = x_axis;
    command->ch.y_axis = y_axis;
    command->texture = texture;
}

static void
push_box(Arena *arena, Rect rect, RGBA color){
    BoxCommand* command = push_struct(arena, BoxCommand);
    command->ch.type = RenderCommand_Box;
    command->ch.arena_used = arena->used;
    command->ch.color = color;
    command->ch.rect = rect;
}

static void
push_quad(Arena *arena, v2 p0, v2 p1, v2 p2, v2 p3, RGBA color, bool fill){
    QuadCommand* command = push_struct(arena, QuadCommand);
    command->ch.type = RenderCommand_Quad;
    command->ch.arena_used = arena->used;
    command->ch.color = color;
    command->ch.fill = fill;
    command->p0 = p0;
    command->p1 = p1;
    command->p2 = p2;
    command->p3 = p3;
}

static void
push_triangle(Arena *arena, v2 p0, v2 p1, v2 p2, RGBA color, bool fill){
    TriangleCommand* command = push_struct(arena, TriangleCommand);
    command->ch.type = RenderCommand_Triangle;
    command->ch.arena_used = arena->used;
    command->ch.color = color;
    command->ch.fill = fill;
    command->p0 = p0;
    command->p1 = p1;
    command->p2 = p2;
}

static void
push_circle(Arena *arena, Rect rect, f32 rad, RGBA color, bool fill){
    CircleCommand* command = push_struct(arena, CircleCommand);
    command->ch.type = RenderCommand_Circle;
    command->ch.arena_used = arena->used;
    command->ch.rect = rect;
    command->ch.color = color;
    command->ch.fill = fill;
    command->ch.rad = rad;
}

static void
push_bitmap(Arena *arena, Rect rect, Bitmap texture){
    BitmapCommand* command = push_struct(arena, BitmapCommand);
    command->ch.type = RenderCommand_Bitmap;
    command->ch.arena_used = arena->used;
    command->ch.rect = rect;
    command->texture = texture;
}

static void
push_text(Arena* command_arena, v2 pos, Font* font, String8 string){
    u8* c;
    s32 kern;
    v2s32 unscaled_offset = {0, 0};

    for(u32 i=0; i < string.size; ++i){
        c = string.str + i;
        if(*c != '\n'){
            Glyph glyph = font->glyphs[*c];

            // setup rect to be pushed to command_arena
            Rect rect = {
                pos.x + (s32)round_f32_s32((unscaled_offset.x + glyph.lsb) * font->scale),
                pos.y + (s32)(round_f32_s32(unscaled_offset.y * font->scale) - glyph.y1),
                0, 0 // no x1, y1 needed for bitmap
            };

            // advance x + kern
            unscaled_offset.x += glyph.advance_width;
            if(string.str[i + 1]){
                kern = stbtt_GetCodepointKernAdvance(&font->info, *c, string.str[i+1]);
                unscaled_offset.x += kern;
            }

            push_bitmap(command_arena, rect, glyph.bitmap);
        }
        else{
            // advance to next line
            unscaled_offset.y -= font->vertical_offset;
            unscaled_offset.x = 0;
        }
    }
}

static void push_text_array(Arena* command_arena, v2 pos, Font* font, String8 strings[], u32 count, bool newline_down = true){
    u8* c;
    v2s32 unscaled_offset = {0, 0};
    for(u32 i=0; i < count; ++i){
       String8* string = strings + i;

        for(u32 i=0; i < string->size; ++i){
            c = string->str + i;
            if(*c != '\n'){
                Glyph glyph = font->glyphs[*c];

                // setup rect
                Rect rect = {
                    pos.x + round_f32_s32((unscaled_offset.x + glyph.lsb) * font->scale),
                    pos.y + (s32)(round_f32_s32(unscaled_offset.y * font->scale) - glyph.y1),
                    0,
                    0
                };

                // advance on x
                unscaled_offset.x += glyph.advance_width;
                if(string->str[i + 1]){
                    s32 kern = stbtt_GetCodepointKernAdvance(&font->info, *c, string->str[i+1]);
                    unscaled_offset.x += kern;
                }

                push_bitmap(command_arena, rect, glyph.bitmap);
            }
            else{
				// advance to next line
                unscaled_offset.y += font->vertical_offset;
                unscaled_offset.x = 0;
            }
        }
        // CONSIDER: maybe I do want this? Idk
        unscaled_offset.y -= font->vertical_offset;
        unscaled_offset.x = 0;
    }
}

static void
draw_pixel(RenderBuffer *render_buffer, v2 position, RGBA color){
    v2s32 pos = round_v2_v2s32(position);

    if(pos.x >= 0 && pos.x < render_buffer->width && pos.y >= 0 && pos.y < render_buffer->height){
        u8 *row = (u8 *)render_buffer->base +
                   (pos.y * render_buffer->stride) +
                   (pos.x * render_buffer->bytes_per_pixel);
        u32 *pixel = (u32 *)row;

        if(color.a == 1){
            u32 new_color = (u32)(round_f32_s32(color.a * 255.0f) << 24 | round_f32_s32(color.r * 255.0f) << 16 | round_f32_s32(color.g * 255.0f) << 8 | round_f32_s32(color.b * 255.0f) << 0);
            *pixel = new_color;
        }
        else if(color.a > 0){
            f32 current_a = ((f32)((*pixel >> 24) & 0xFF) / 255.0f);
            f32 current_r = (f32)((*pixel >> 16) & 0xFF);
            f32 current_g = (f32)((*pixel >> 8) & 0xFF);
            f32 current_b = (f32)((*pixel >> 0) & 0xFF);

            color.r *= 255.0f;
            color.g *= 255.0f;
            color.b *= 255.0f;

            f32 new_r = (1 - color.a) * current_r + (color.a * color.r);
            f32 new_g = (1 - color.a) * current_g + (color.a * color.g);
            f32 new_b = (1 - color.a) * current_b + (color.a * color.b);

            u32 new_color = (u32)(round_f32_s32(color.a * 255.0f) << 24 | round_f32_s32(new_r) << 16 | round_f32_s32(new_g) << 8 | round_f32_s32(new_b) << 0);
            *pixel = new_color;
        }
    }
}

// UNTESTED: untested with rect screenspace change
static void
draw_line(RenderBuffer *render_buffer, Rect rect, v2 direction, RGBA color){
    //RectPixelSpace rect_ps = screen_to_pixel(rect, resolution);

    v2 point1 = round_v2(rect.min);
    v2 point2 = point1;
    v2 non_normalized_direction = make_v2(point1.x + direction.x, point1.y + direction.y);
    direction = round_v2(non_normalized_direction);

    f32 distance_x =  abs_f32(direction.x - point1.x);
    f32 distance_y = -abs_f32(direction.y - point1.y);
    f32 d1_step_x = point1.x < direction.x ? 1.0f : -1.0f;
    f32 d1_step_y = point1.y < direction.y ? 1.0f : -1.0f;
    f32 d2_step_x = -(d1_step_x);
    f32 d2_step_y = -(d1_step_y);

    f32 error = distance_x + distance_y;

    for(;;){
        draw_pixel(render_buffer, point1, color); // NOTE: before break, so you can draw a single point draw_segment (a pixel)
        if(point1.x < 0 || point1.x > (f32)render_buffer->width || point1.y < 0 || point1.y > (f32)render_buffer->height)break;

        f32 error2 = 2 * error;
        if (error2 >= distance_y){
            error += distance_y;
            point1.x += d1_step_x;
        }
        if (error2 <= distance_x){
            error += distance_x;
            point1.y += d1_step_y;
        }
    }
    for(;;){
        draw_pixel(render_buffer, point2, color);
        if(point2.x < 0 || point2.x > (f32)render_buffer->width || point2.y < 0 || point2.y > (f32)render_buffer->height)break;

        f32 error2 = 2 * error;
        if (error2 >= distance_y){
            error += distance_y;
            point2.x += d2_step_x;
        }
        if (error2 <= distance_x){
            error += distance_x;
            point2.y += d2_step_y;
        }
    }
}

// UNTESTED: untested with rect screenspace change
static void
draw_ray(RenderBuffer *render_buffer, Rect rect, v2 direction, RGBA color){
    //RectPixelSpace rect_ps = screen_to_pixel(rect, resolution);
    v2 pos = round_v2(rect.min);

    v2 non_normalized_direction = round_v2(make_v2((direction.x * 100000), (direction.y * 100000)));
    v2 new_direction = make_v2(pos.x + non_normalized_direction.x, pos.y + non_normalized_direction.y);

    f32 distance_x =  abs_f32(new_direction.x - pos.x);
    f32 distance_y = -abs_f32(new_direction.y - pos.y);
    f32 step_x = pos.x < new_direction.x ? 1.0f : -1.0f;
    f32 step_y = pos.y < new_direction.y ? 1.0f : -1.0f;

    f32 error = distance_x + distance_y;

    for(;;){
        draw_pixel(render_buffer, make_v2(pos.x, pos.y), color); // NOTE: before break, so you can draw a single position draw_segment (a pixel)
        if(pos.x < 0 || pos.x > (f32)render_buffer->width || pos.y < 0 || pos.y > (f32)render_buffer->height)break;

        f32 error2 = 2 * error;
        if (error2 >= distance_y){
            error += distance_y;
            pos.x += step_x;
        }
        if (error2 <= distance_x){
            error += distance_x;
            pos.y += step_y;
        }
    }
}

static void
draw_segment(RenderBuffer *render_buffer, v2 p0, v2 p1, RGBA color){
    p0 = round_v2(p0);
    p1 = round_v2(p1);

    f32 distance_x =  abs_f32(p1.x - p0.x);
    f32 distance_y = -abs_f32(p1.y - p0.y);
    f32 step_x = p0.x < p1.x ? 1.0f : -1.0f;
    f32 step_y = p0.y < p1.y ? 1.0f : -1.0f;

    f32 error = distance_x + distance_y;

    for(;;){
        if(p0.x == p1.x && p0.y == p1.y) break;
        draw_pixel(render_buffer, p0, color); // NOTE: before break, so you can draw a single point draw_segment (a pixel)

        f32 error2 = 2 * error;
        if (error2 >= distance_y){
            error += distance_y;
            p0.x += step_x;
        }
        if (error2 <= distance_x){
            error += distance_x;
            p0.y += step_y;
        }
    }
}

static void
draw_flattop_triangle(RenderBuffer *render_buffer, v2 p0, v2 p1, v2 p2, RGBA color){
    f32 left_slope = (p0.x - p2.x) / (p0.y - p2.y);
    f32 right_slope = (p1.x - p2.x) / (p1.y - p2.y);

    //s32 start_y = (int)round(p2.y);
    //s32 end_y = (int)round(p0.y);
    s32 start_y = (s32)round_f32_s32(p2.y);
    s32 end_y = (s32)round_f32_s32(p0.y);

    for(s32 y=start_y; y < end_y; ++y){
        f32 x0 = left_slope * ((f32)y + 0.5f - p0.y) + p0.x;
        f32 x1 = right_slope * ((f32)y + 0.5f - p1.y) + p1.x;
        s32 start_x = (s32)ceil_f32(x0 - 0.5f);
        s32 end_x = (s32)ceil_f32(x1 - 0.5f);
        for(s32 x=start_x; x < end_x; ++x){
            draw_pixel(render_buffer, make_v2((f32)x, (f32)y), color);
        }
    }
}

static void
draw_flatbottom_triangle(RenderBuffer *render_buffer, v2 p0, v2 p1, v2 p2, RGBA color){
    f32 left_slope = (p1.x - p0.x) / (p1.y - p0.y);
    f32 right_slope = (p2.x - p0.x) / (p2.y - p0.y);

    //s32 start_y = (int)round(p0.y);
    //s32 end_y = (int)round(p1.y);
    s32 start_y = (s32)round_f32_s32(p0.y);
    s32 end_y = (s32)round_f32_s32(p1.y);

    for(s32 y=start_y; y >= end_y; --y){
        f32 x0 = left_slope * ((f32)y + 0.5f - p0.y) + p0.x;
        f32 x1 = right_slope * ((f32)y + 0.5f - p0.y) + p0.x;
        s32 start_x = (s32)ceil_f32(x0 - 0.5f);
        s32 end_x = (s32)ceil_f32(x1 - 0.5f);
        for(s32 x=start_x; x < end_x; ++x){
            draw_pixel(render_buffer, make_v2((f32)x, (f32)y), color);
        }
    }
}

static void
draw_triangle_outlined(RenderBuffer *render_buffer, v2 p0, v2 p1, v2 p2, RGBA color, RGBA c_outlined, bool fill){
    if(p0.y < p1.y){ swap_v2(&p0, &p1); }
    if(p0.y < p2.y){ swap_v2(&p0, &p2); }
    if(p1.y < p2.y){ swap_v2(&p1, &p2); }

    if(fill){
        if(p0.y == p1.y){
            if(p0.x > p1.x){ swap_v2(&p0, &p1); }
            draw_flattop_triangle(render_buffer, p0, p1, p2, color);
        }
        else if(p1.y == p2.y){
            if(p1.x > p2.x){ swap_v2(&p1, &p2); }
            draw_flatbottom_triangle(render_buffer, p0, p1, p2, color);
        }
        else{
            f32 x = p0.x + ((p1.y - p0.y) / (p2.y - p0.y)) * (p2.x - p0.x);
            v2 p3 = {x, p1.y};
            if(p1.x > p3.x){ swap_v2(&p1, &p3); }
            draw_flattop_triangle(render_buffer, p1, p3, p2, color);
            draw_flatbottom_triangle(render_buffer, p0, p1, p3, color);
        }
    }
    draw_segment(render_buffer, p0, p1, c_outlined);
    draw_segment(render_buffer, p1, p2, c_outlined);
    draw_segment(render_buffer, p2, p0, c_outlined);
}

static void
draw_triangle(RenderBuffer *render_buffer, v2 p0, v2 p1, v2 p2, RGBA color, bool fill){
    if(p0.y < p1.y){ swap_v2(&p0, &p1); }
    if(p0.y < p2.y){ swap_v2(&p0, &p2); }
    if(p1.y < p2.y){ swap_v2(&p1, &p2); }

    if(fill){
        if(p0.y == p1.y){
            if(p0.x > p1.x){ swap_v2(&p0, &p1); }
            draw_flattop_triangle(render_buffer, p0, p1, p2, color);
        }
        else if(p1.y == p2.y){
            if(p1.x > p2.x){ swap_v2(&p1, &p2); }
            draw_flatbottom_triangle(render_buffer, p0, p1, p2, color);
        }
        else{
            f32 x = p0.x + ((p1.y - p0.y) / (p2.y - p0.y)) * (p2.x - p0.x);
            v2 p3 = {x, p1.y};
            if(p1.x > p3.x){ swap_v2(&p1, &p3); }
            draw_flattop_triangle(render_buffer, p1, p3, p2, color);
            draw_flatbottom_triangle(render_buffer, p0, p1, p3, color);
        }
    }
    else{
        draw_segment(render_buffer, p0, p1, color);
        draw_segment(render_buffer, p1, p2, color);
        draw_segment(render_buffer, p2, p0, color);
    }
}

static void
clear_color(RenderBuffer *render_buffer, RGBA color={0, 0, 0, 1}){
    u32 *pixel = (u32 *)((u8 *)(render_buffer->base));
    for(s32 i=0; i < (render_buffer->width * render_buffer->height); ++i){
        u32 new_color = (u32)(round_f32_s32(color.a * 255.0f) << 24 | round_f32_s32(color.r*255.0f) << 16 | round_f32_s32(color.g*255.0f) << 8 | round_f32_s32(color.b*255.0f) << 0);
        *pixel++ = new_color;
    }
}

// IMPORTANT: Martins example of how to clear screen in a way that us user friendly
static void
clear_image(void* pix, int width, int height, int pitch, uint32_t color){
    for(int y=0; y<height; y++){
        u32* row = (u32*)( (u8*)pix + y * pitch );

        for(int x=0; x<width; x++) {
            row[x] = color;
        }
    }
}

// TODO: clear only a region
// INCOMPLETE: Rect type has changed
//static void
//clear_color_region(RenderBuffer *render_buffer, Rect rect, RGBA color={0, 0, 0, 1}){
//    v2s32 pos = round_v2_v2s32(rect.pos);
//    u8 *row = (u8 *)render_buffer->base +
//                 ((render_buffer->height - pos.y - 1) * render_buffer->stride) +
//                 (pos.x * render_buffer->bytes_per_pixel);
//
//    for(s32 y=rect.y0; y < (rect.y0 + rect.y1); ++y){
//        u32* pixel = (u32*)row;
//        for(s32 x=rect.x0; x < (rect.x0 + rect.x1); ++x){
//            u32 new_color = (round_f32_s32(color.a * 255.0f) << 24 | round_f32_s32(color.r*255.0f) << 16 | round_f32_s32(color.g*255.0f) << 8 | round_f32_s32(color.b*255.0f) << 0);
//            *pixel++ = new_color;
//        }
//        row += render_buffer->stride;
//    }
//}


static void
draw_bitmap(RenderBuffer *render_buffer, v2 pos, Bitmap* texture){
    Rect rect = make_rect(pos.x, pos.y, pos.x + (f32)texture->width, pos.y + (f32)texture->height);
    v2s32 pixel_min = round_v2_v2s32(rect.min);
    v2s32 pixel_max = round_v2_v2s32(rect.max);

    u32* at = (u32*)texture->base;
    for(s32 y=pixel_min.y; y < pixel_max.y; ++y){
        for(s32 x=pixel_min.x; x < pixel_max.x; ++x){
            RGBA color = u32_to_rgba_normal(*at++);
            draw_pixel(render_buffer, make_v2((f32)x, (f32)y), color);
        }
    }
}


static void
draw_bitmap_slow(RenderBuffer *render_buffer, v2 origin, v2 x_axis, v2 y_axis, Bitmap* texture, RGBA tint_color = {0, 0, 0, 1}){

    // pre-multiply alpha for color
    tint_color.rgb *= tint_color.a;

    f32 inv_xaxis_mag_sqrt = 1.0f / magnitude_sqrt_v2(x_axis);
    f32 inv_yaxis_mag_sqrt = 1.0f / magnitude_sqrt_v2(y_axis);

    s32 max_width = render_buffer->width - 1;
    s32 max_height = render_buffer->height - 1;

    s32 xmax = 0;
    s32 xmin = max_width;
    s32 ymax = 0;
    s32 ymin = max_height;

    v2 points[4] = {origin, origin + x_axis, origin + y_axis, origin + x_axis + y_axis};
    for(u32 i=0; i < array_count(points); ++i){
        v2 test_p = points[i];
        s32 x = round_f32_s32(test_p.x);
        s32 y = round_f32_s32(test_p.y);

        if(x < xmin){ xmin = x; }
        if(y < ymin){ ymin = y; }
        if(x > xmax){ xmax = x; }
        if(y > ymax){ ymax = y; }
    }

    if(xmin < 0){ xmin = 0; }
    if(ymin < 0){ ymin = 0; }
    if(xmax > max_width){ xmax = max_width; }
    if(ymax > max_height){ ymax = max_height; }

    u8 *row = (u8*)render_buffer->base +
                  (ymin * render_buffer->stride) +
                  (xmin * render_buffer->bytes_per_pixel);
    for(s32 y=ymin; y < ymax; ++y){
        u32* pixel = (u32*)row;
        for(s32 x=xmin; x <= xmax; ++x){

            v2 pixel_pos = {(f32)x, (f32)y};
            v2 dist = pixel_pos - origin;
            f32 edge0 = dot_v2(dist,                  -perp(x_axis));
            f32 edge1 = dot_v2(dist - x_axis,         -perp(y_axis));
            f32 edge2 = dot_v2(dist - x_axis - y_axis, perp(x_axis));
            f32 edge3 = dot_v2(dist - y_axis,          perp(y_axis));

            if((edge0 < 0) && (edge1 < 0) && (edge2 < 0) && (edge3 < 0)){
                // get UV coordinates
                f32 u = inv_xaxis_mag_sqrt * dot_v2(dist, x_axis);
                f32 v = inv_yaxis_mag_sqrt * dot_v2(dist, y_axis);

                //assert(u >= 0.0f && u <= 1.0f);
                //assert(v >= 0.0f && v <= 1.0f);

                f32 tx = ((u * (f32)(texture->width  - 2)));
                f32 ty = ((v * (f32)(texture->height - 2)));
                //f32 tx = 1.0f + ((u * (f32)(texture->width  - 3)) + 0.5f);
                //f32 ty = 1.0f + ((v * (f32)(texture->height - 3)) + 0.5f);
                s32 x = (s32)tx;
                s32 y = (s32)ty;
                f32 fx = tx - (f32)x;
                f32 fy = ty - (f32)y;

                //assert(x >= 0 && x < texture->width);
                //assert(y >= 0 && y < texture->height);

                // select 4 pixels from source
                u8*  texel_ptr = ((u8*)texture->base + (y * texture->stride) + (x * 4));
                u32* texel_ptr_a = (u32*)(texel_ptr);
                u32* texel_ptr_b = (u32*)(texel_ptr + sizeof(u32));
                u32* texel_ptr_c = (u32*)(texel_ptr + texture->stride);
                u32* texel_ptr_d = (u32*)(texel_ptr + texture->stride + sizeof(u32));

                // convert source to RGBA normalized
                RGBA texel_a = u32_to_rgba_normal(*texel_ptr_a);
                RGBA texel_b = u32_to_rgba_normal(*texel_ptr_b);
                RGBA texel_c = u32_to_rgba_normal(*texel_ptr_c);
                RGBA texel_d = u32_to_rgba_normal(*texel_ptr_d);

                // convert source to linear space
                texel_a = srgb_to_linear(texel_a);
                texel_b = srgb_to_linear(texel_b);
                texel_c = srgb_to_linear(texel_c);
                texel_d = srgb_to_linear(texel_d);

                // bilinear filtering on source
                RGBA texel = lerp(lerp(texel_a, texel_b, fx), lerp(texel_c, texel_d, fx), fy);

                // color tinting
                //texel.r *= tint_color.r;
                //texel.g *= tint_color.g;
                //texel.b *= tint_color.b;
                //texel.a *= tint_color.a;

                // convert dest to linear space
                RGBA pixel_color = u32_to_rgba_normal(*pixel);
                pixel_color = srgb_to_linear(pixel_color);

                // linear blend
                RGBA write_color = {
                    .r = ((1.0f - texel.a) * pixel_color.r) + texel.r,
                    .g = ((1.0f - texel.a) * pixel_color.g) + texel.g,
                    .b = ((1.0f - texel.a) * pixel_color.b) + texel.b,
                    .a = ((1.0f - texel.a) * pixel_color.a) + texel.a,
                };

                // convert to sRGB space, normal
                write_color = linear_to_srgb(write_color);

                // write color
                *pixel = (u32)(round_f32_u32(write_color.a * 255.0f) << 24 |
                               round_f32_u32(write_color.r * 255.0f) << 16 |
                               round_f32_u32(write_color.g * 255.0f) << 8 |
                               round_f32_u32(write_color.b * 255.0f) << 0);
            }
            pixel++;
        }
        row += render_buffer->stride;
    }
}

static void
draw_rect(RenderBuffer *render_buffer, Rect rect, RGBA color){
    // round min/max
    v2s32 pixel_min = round_v2_v2s32(rect.min);
    v2s32 pixel_max = round_v2_v2s32(rect.max);

    // clamp min/max of rect to render_buffer
    if(pixel_min.x < 0) { pixel_min.x = 0; }
    if(pixel_min.y < 0) { pixel_min.y = 0; }
    if(pixel_max.x > render_buffer->width) { pixel_max.x = render_buffer->width; }
    if(pixel_max.y > render_buffer->height) { pixel_max.y = render_buffer->height; }

    // helper 4x variables
    __m128i max_x_4x = _mm_set1_epi32(pixel_max.x);
    __m128 color_255_4x = _mm_set_ps1(255.0f);
    __m128 color_one_4x = _mm_set_ps1(1.0f);
    __m128 color_zero_4x = _mm_set_ps1(0.0f);
    __m128 mask_FF = _mm_set1_epi32(0xFF);

    // 4x of input color
    __m128 color_a_4x = _mm_set_ps1(color.a);
    __m128 color_r_4x = _mm_set_ps1(color.r);
    __m128 color_g_4x = _mm_set_ps1(color.g);
    __m128 color_b_4x = _mm_set_ps1(color.b);

    // pre multiply alphas for current
    __m128 current_a_percent_4x = _mm_sub_ps(color_one_4x, color_a_4x);
    __m128 scaled_color_r_4x = _mm_mul_ps(color_r_4x, color_255_4x);
    __m128 scaled_color_g_4x = _mm_mul_ps(color_g_4x, color_255_4x);
    __m128 scaled_color_b_4x = _mm_mul_ps(color_b_4x, color_255_4x);

    // pre multiply alphas for input color
    __m128 new_color_r_4x = (color_a_4x * scaled_color_r_4x);
    __m128 new_color_g_4x = (color_a_4x * scaled_color_g_4x);
    __m128 new_color_b_4x = (color_a_4x * scaled_color_b_4x);

    // compute initial increment
    s32 width = pixel_max.x - pixel_min.x;
    s32 remainder = width % 4;
    s32 increment = remainder ? remainder : 4;

    // compute mask
    __m128i mask = _mm_setr_epi32(-(increment > 0), -(increment > 1), -(increment > 2), -(increment > 3));
    __m128i ones_mask = _mm_cmpeq_epi32(mask, mask);

    // get row based of clamped pixel_min.x/pixel_min.y
    u8 *row = (u8 *)render_buffer->base +
              (pixel_min.y * render_buffer->stride) +
              (pixel_min.x * render_buffer->bytes_per_pixel);

    // iterate over clamped pixel_min.x/pixel_min.y
    //BEGIN_CYCLE_COUNTER(draw_rect_fast)
    for(s32 y=pixel_min.y; y < pixel_max.y; ++y){
        u32* pixel = (u32*)row;

        for(s32 x=pixel_min.x; x < pixel_max.x; ){
            // if x in dead zone, load last 4 pixels and set mask appropriately
            if(x > render_buffer->width - 4){
                row = (u8 *)render_buffer->base +
                      ((render_buffer->height - 1 - pixel_min.y) * render_buffer->stride) +
                      ((render_buffer->width - 4) * render_buffer->bytes_per_pixel);
                pixel = (u32*)row;
                mask = ~mask;
            }

            __m128i loaded_pixel_4x = _mm_loadu_si128((__m128i*)pixel);

            // from: argb-argb-argb-argb
            // to:   aaaa-rrrr-gggg-bbbb
            __m128 loaded_r_4x = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(loaded_pixel_4x, 16), mask_FF));
            __m128 loaded_g_4x = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(loaded_pixel_4x, 8),  mask_FF));
            __m128 loaded_b_4x = _mm_cvtepi32_ps(_mm_and_si128(loaded_pixel_4x, mask_FF));

            // linear blend between loaded color and input color
            __m128 new_loaded_r_4x = _mm_mul_ps(current_a_percent_4x, loaded_r_4x);
            __m128 new_loaded_g_4x = _mm_mul_ps(current_a_percent_4x, loaded_g_4x);
            __m128 new_loaded_b_4x = _mm_mul_ps(current_a_percent_4x, loaded_b_4x);

            __m128 new_r_4x = _mm_add_ps(new_loaded_r_4x, new_color_r_4x);
            __m128 new_g_4x = _mm_add_ps(new_loaded_g_4x, new_color_g_4x);
            __m128 new_b_4x = _mm_add_ps(new_loaded_b_4x, new_color_b_4x);
            __m128 new_a_4x = _mm_mul_ps(color_a_4x, color_255_4x);

            // convert to int, cvtps rounds to nearest. no need for +0.5f
            __m128i int_a_4x = _mm_cvtps_epi32(new_a_4x);
            __m128i int_r_4x = _mm_cvtps_epi32(new_r_4x);
            __m128i int_g_4x = _mm_cvtps_epi32(new_g_4x);
            __m128i int_b_4x = _mm_cvtps_epi32(new_b_4x);

            // shift argb order
            __m128i shifted_int_a_4x = _mm_slli_epi32(int_a_4x, 24);
            __m128i shifted_int_r_4x = _mm_slli_epi32(int_r_4x, 16);
            __m128i shifted_int_g_4x = _mm_slli_epi32(int_g_4x, 8);
            __m128i shifted_int_b_4x = int_b_4x;

            // or everything together
            __m128i output_pixel_4x = _mm_or_si128(shifted_int_a_4x,
                                      _mm_or_si128(shifted_int_r_4x,
                                      _mm_or_si128(shifted_int_g_4x,
                                                   shifted_int_b_4x)));

            __m128i output_masked = _mm_and_si128(mask, output_pixel_4x);

            _mm_storeu_si128((__m128i * )pixel, output_masked);

            pixel += increment;
            x+=increment;
            increment = 4;
            mask = ones_mask;
        }
        increment = remainder ? remainder : 4;
        mask = _mm_setr_epi32(-(increment > 0), -(increment > 1), -(increment > 2), -(increment > 3));
        row += render_buffer->stride;
    }
    //END_CYCLE_COUNTER(draw_rect_fast)
}

// UNTESTED: untested with rect screenspace change
static void
draw_box(RenderBuffer *render_buffer, Rect rect, RGBA color){
    //RectPixelSpace rect_ps = screen_to_pixel(rect, resolution);

    v2 p0 = rect.min;
    v2 p1 = {rect.max.x, rect.min.y};
    v2 p2 = rect.max;
    v2 p3 = {rect.min.x, rect.max.y};

    draw_segment(render_buffer, p0, p1, color);
    draw_segment(render_buffer, p1, p2, color);
    draw_segment(render_buffer, p2, p3, color);
    draw_segment(render_buffer, p3, p0, color);
}

static void
draw_quad(RenderBuffer *render_buffer, v2 p0_, v2 p1_, v2 p2_, v2 p3_, RGBA color, bool fill){
    v2 p0 = p0_;
    v2 p1 = p1_;
    v2 p2 = p2_;
    v2 p3 = p3_;

    if(p0.x > p1.x){ swap_v2(&p0, &p1); }
    if(p0.x > p2.x){ swap_v2(&p0, &p2); }
    if(p0.y > p2.y){ swap_v2(&p0, &p2); }
    if(p0.x > p3.x){ swap_v2(&p0, &p3); }
    if(p0.y > p3.y){ swap_v2(&p0, &p3); }

    if(p1.x < p0.x){ swap_v2(&p1, &p0); }
    if(p1.x < p3.x){ swap_v2(&p1, &p3); }
    if(p1.y > p2.y){ swap_v2(&p1, &p2); }
    if(p1.y > p3.y){ swap_v2(&p1, &p3); }

    if(p2.x < p0.x){ swap_v2(&p2, &p0); }
    if(p2.x < p3.x){ swap_v2(&p2, &p3); }
    if(p2.y < p0.y){ swap_v2(&p2, &p0); }
    if(p2.y < p1.y){ swap_v2(&p2, &p1); }

    if(p3.x > p1.x){ swap_v2(&p3, &p1); }
    if(p3.x > p2.x){ swap_v2(&p3, &p2); }
    if(p3.y < p0.y){ swap_v2(&p3, &p0); }
    if(p3.y < p1.y){ swap_v2(&p3, &p1); }

    draw_triangle(render_buffer, p0, p1, p2, color, fill);
    draw_triangle(render_buffer, p0, p2, p3, color, fill);
}

static void
draw_polygon(RenderBuffer *render_buffer, v2 *points, u32 count, RGBA color){
    v2 first = *points++;
    v2 prev = first;
    for(u32 i=1; i<count; ++i){
        draw_segment(render_buffer, prev, *points, color);
        prev = *points++;
    }
    draw_segment(render_buffer, first, prev, color);
}

static void
draw_circle(RenderBuffer *render_buffer, f32 xm, f32 ym, f32 r, RGBA color, bool fill) {
    f32 x = -r;
    f32 y = 0;
    f32 err = 2-2*r;
    do {
        draw_pixel(render_buffer, make_v2((xm - x), (ym + y)), color);
        draw_pixel(render_buffer, make_v2((xm - y), (ym - x)), color);
        draw_pixel(render_buffer, make_v2((xm + x), (ym - y)), color);
        draw_pixel(render_buffer, make_v2((xm + y), (ym + x)), color);
        r = err;
        if (r <= y){
            if(fill){
                if(ym + y == ym - y){
                    draw_segment(render_buffer, make_v2((xm - x - 1), (ym + y)), make_v2((xm + x), (ym + y)), color);
                }
                else{
                    draw_segment(render_buffer, make_v2((xm - x - 1), (ym + y)), make_v2((xm + x), (ym + y)), color);
                    draw_segment(render_buffer, make_v2((xm - x - 1), (ym - y)), make_v2((xm + x), (ym - y)), color);
                }
            }
            y++;
            err += y * 2 + 1;
        }
        if (r > x || err > y){
            x++;
            err += x * 2 + 1;
        }
    } while (x < 0);
}

static void
draw_commands(RenderBuffer *render_buffer, Arena *commands){
    void* at = commands->base;
    void* end = (u8*)commands->base + commands->used;
    while(at != end){
        CommandHeader* base_command = (CommandHeader*)at;

        switch(base_command->type){
            case RenderCommand_ClearColor:{
                ClearColorCommand *command = (ClearColorCommand*)base_command;
                clear_color(render_buffer, command->ch.color);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Pixel:{
                PixelCommand *command = (PixelCommand*)base_command;
                draw_pixel(render_buffer, make_v2(command->ch.rect.x0, command->ch.rect.y0), command->ch.color);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Segment:{
                SegmentCommand *command = (SegmentCommand*)base_command;
                draw_segment(render_buffer, command->p0, command->p1, command->ch.color);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Ray:{
                RayCommand *command = (RayCommand*)base_command;
                draw_ray(render_buffer, command->ch.rect, command->ch.direction, command->ch.color);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Line:{
                LineCommand *command = (LineCommand*)base_command;
                draw_line(render_buffer, command->ch.rect, command->ch.direction, command->ch.color);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Rect:{
                RectCommand *command = (RectCommand*)base_command;
                draw_rect(render_buffer, command->ch.rect, command->ch.color);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Basis:{
                BasisCommand *command = (BasisCommand*)base_command;
#if 1
                draw_bitmap_slow(render_buffer, command->ch.origin, command->ch.x_axis, command->ch.y_axis, &command->texture);
#else
                RGBA color = {
                    .r = 0.5f + 0.5f * sin_f32(angle*2.0f),
                    .g = 0.5f + 0.5f * cos_f32(angle),
                    .b = 0.5f + 0.5f * sin_f32(angle),
                    //.a = 1.0f,
                    .a = 0.5f + 0.5f * cos_f32(angle*2.0f),
                };
                draw_bitmap_slow(render_buffer, command->ch.origin, command->ch.x_axis, command->ch.y_axis, &command->texture, color);
#endif
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Box:{
                BoxCommand *command = (BoxCommand*)base_command;
                draw_box(render_buffer, command->ch.rect, command->ch.color);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Quad:{
                QuadCommand *command = (QuadCommand*)base_command;
                draw_quad(render_buffer, command->p0, command->p1, command->p2, command->p3, command->ch.color, command->ch.fill);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Triangle:{
                TriangleCommand *command = (TriangleCommand*)base_command;
                draw_triangle(render_buffer, command->p0, command->p1, command->p2, base_command->color, base_command->fill);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Circle:{
                CircleCommand *command = (CircleCommand*)base_command;
                draw_circle(render_buffer, command->ch.rect.x0, command->ch.rect.y0, command->ch.rad, command->ch.color, command->ch.fill);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Bitmap:{
                BitmapCommand *command = (BitmapCommand*)base_command;
                draw_bitmap(render_buffer, command->ch.rect.min, &command->texture);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
        }
    }
}







//typedef struct Rect2{
//    u32 l, r, t, b;
//} Rect2;
//
//
//
//static Rect
//rect2_intersection(Rect a, Rect b){
//    if(a.l < b.l) a.l = b.l;
//    if(a.r < b.r) a.r = b.r;
//    if(a.t < b.t) a.t = b.t;
//    if(a.b < b.b) a.b = b.b;
//    return(a);
//}
//
//static bool
//rect2_contains_v2(Rect a, v2 b){
//    bool result = (a.l <= b.x && a.r > b.x && a.t > b.y && a.b <= b.y);
//    return(result);
//}
//
//static bool
//rect2_is_valid(Rect a){
//    bool result = (a.r < a.l && a.t > a.b);
//    return(result);
//}
//
//static bool
//rect2_cmp(Rect a, Rect b){
//    bool result = (a.l == b.l && a.r == b.r && a.t == b.t && a.b == b.b);
//    return(result);
//}
//
//// Compute the smallest rectangle containing both of the input rectangles.
//static Rect
//rect2_bounding(Rect a, Rect b){
//    Rect2 result = ZERO_INIT;
//    if(a.l > b.l) a.l = b.l;
//    if(a.r < b.r) a.r = b.r;
//    if(a.t < b.t) a.t = b.t;
//    if(a.b > b.b) a.b = b.b;
//    return(result);
//}

#endif
