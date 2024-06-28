#ifndef CAMERA_H
#define CAMERA_H

struct Camera{
    f32 fov;

    f32 yaw;
    f32 pitch;

    v3 pos;
    v3 forward;
    v3 up;

    f32 move_speed;
    f32 rotation_speed;
};
static Camera camera;

static void init_camera();
static void update_camera(f32 dx, f32 dy, f32 dt);

#endif
