#ifndef RECT_C
#define RECT_C

static Rect
make_rect(f32 x0, f32 y0, f32 x1, f32 y1){
    Rect result;
    result.x0 = x0;
    result.y0 = y0;
    result.x1 = x1;
    result.y1 = y1;
    return(result);
}

static Rect
make_rect(v2 min, v2 max){
    Rect result;
    result.min = min;
    result.max = max;
    return(result);
}

static Rect
rect_pixel_from_clip(Rect r, v2s32 res){
    Rect result = {
        ((r.x0  + 1.0f) / 2.0f) * (f32)res.w,
        ((r.y0  + 1.0f) / 2.0f) * (f32)res.h,
        ((r.x1  + 1.0f) / 2.0f) * (f32)res.w,
        ((r.y1  + 1.0f) / 2.0f) * (f32)res.h,
    };
    return(result);
}

static Rect
rect_clip_from_pixel(Rect r, v2s32 res){
    Rect result = {
        ((r.x0 / (f32)res.w) * 2.0f) - 1.0f,
        ((r.y0 / (f32)res.h) * 2.0f) - 1.0f,
        ((r.x1 / (f32)res.w) * 2.0f) - 1.0f,
        ((r.y1 / (f32)res.h) * 2.0f) - 1.0f,
    };
    return(result);
}

static Quad
make_quad(v2 p0, v2 p1, v2 p2, v2 p3){
    Quad result;
    result.p0 = p0;
    result.p1 = p1;
    result.p2 = p2;
    result.p3 = p3;
    return(result);
}

static Quad
quad_clip_from_pixel(Quad q, v2s32 res){
    v2 p0 = {((q.p0.x / (f32)res.w) * 2.0f) - 1.0f, ((q.p0.y / (f32)res.h) * 2.0f) - 1.0f};
    v2 p1 = {((q.p1.x / (f32)res.w) * 2.0f) - 1.0f, ((q.p1.y / (f32)res.h) * 2.0f) - 1.0f};
    v2 p2 = {((q.p2.x / (f32)res.w) * 2.0f) - 1.0f, ((q.p2.y / (f32)res.h) * 2.0f) - 1.0f};
    v2 p3 = {((q.p3.x / (f32)res.w) * 2.0f) - 1.0f, ((q.p3.y / (f32)res.h) * 2.0f) - 1.0f};

    Quad result = make_quad(p0, p1, p2, p3);
    return(result);
}

static Rect
rect_clip_from_pixel_inverted(Rect r, v2s32 res){
    Rect result = {
       (((r.x0 / (f32)res.w) * 2.0f) - 1.0f),
      -(((r.y0 / (f32)res.h) * 2.0f) - 1.0f),
       (((r.x1 / (f32)res.w) * 2.0f) - 1.0f),
      -(((r.y1 / (f32)res.h) * 2.0f) - 1.0f)
    };
    return(result);
}

static v2
rect_width_height(Rect rect){
    //Rect ps_rect = screen_to_pixel(rect);

    v2 result = rect.max - rect.min;
    return(result);
}

static bool
rect_collides_rect(Rect rect1, Rect rect2) {
    // Check for no overlap
    if (rect1.x1 < rect2.x0 || rect1.x0 > rect2.x1 || rect1.y1 < rect2.y0 || rect1.y0 > rect2.y1) {
        return false;
    }
    // Rectangles overlap
    return true;
}

static bool
rect_collides_point(Rect r1, v2 p){
    if((p.x < r1.x0 + r1.x1) &&
       (p.x > r1.x0) &&
       (p.y < r1.y0 + r1.y1) &&
       (p.y > r1.y0)){
        return true;
    }
    return false;
}

static bool
rect_contains_rect(Rect r1, Rect r2){
    if((r2.x0 > r1.x0) &&
       (r2.x0 + r2.x1 < r1.x0 + r1.x1) &&
       (r2.y0 > r1.y0) &&
       (r2.y0 + r2.y1 < r1.y0 + r1.y1)){
        return true;
    }
    return false;
}

static Rect
rect_calc_border(Rect rect, s32 border_size){
    //RectPixelSpace rect_ps = screen_to_pixel(rect);
    Rect result = {
        rect.x0 - (f32)border_size,
        rect.y0 - (f32)border_size,
        rect.x1 + (f32)border_size,
        rect.y1 + (f32)border_size,
    };

    //RectScreenSpace result = pixel_to_screen(rect_ps);
    return(result);
}

//static Rect
//rect_get_border_intruding(Rect rect, s32 border_size){
//    RectPixelSpace rect_ps = screen_to_pixel(rect);
//
//    RectScreenSpace result = pixel_to_screen(rect_ps);
//    return(result);
//}

static Quad quad_from_pos_center(v2 pos, v2 dim){
    Quad result = {0};
    result.p0 = make_v2(pos.x - dim.w/2, pos.y - dim.h/2);
    result.p1 = make_v2(pos.x + dim.w/2, pos.y - dim.h/2);
    result.p2 = make_v2(pos.x + dim.w/2, pos.y + dim.h/2);
    result.p3 = make_v2(pos.x - dim.w/2, pos.y + dim.h/2);

    return(result);
}

static Quad quad_from_pos(v2 pos, v2 dim){
    Quad result = {0};
    result.p0 = make_v2(pos.x, pos.y);
    result.p1 = make_v2(pos.x + dim.w, pos.y);
    result.p2 = make_v2(pos.x + dim.w, pos.y + dim.h);
    result.p3 = make_v2(pos.x, pos.y + dim.h);

    return(result);
}

#endif
