#ifndef MAIN_H
#define MAIN_H


#if DEBUG
#define ENABLE_ASSERT 1
#endif

#pragma comment(lib, "user32")
#include "base_inc.h"
#include "win32_base_inc.h"

#define PROFILER 1
#include "profiler.h"

#include "input.hpp"
#include "clock.hpp"
#include "wave.h"
#include "wasapi.h"
#include "camera.hpp"
#include "rect.hpp"
#include "bitmap.hpp"
#include "d3d11_init.hpp"
#include "font.hpp"
#include "d3d11_render.hpp"
#include "entity.hpp"
#include "console.hpp"
#include "command.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"


#include "input.cpp"
#include "clock.cpp"
#include "camera.cpp"
#include "rect.cpp"
#include "entity.cpp"

static String8 build_path;
static String8 fonts_path;
static String8 shaders_path;
static String8 saves_path;
static String8 sprites_path;
static String8 sounds_path;

typedef struct Memory{
    void* base;
    size_t size;

    void* permanent_base;
    size_t permanent_size;
    void* transient_base;
    size_t transient_size;

    bool initialized;
} Memory;
global Memory memory;
static void memory_init();

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
struct Window{
    s32 width;
    s32 height;
    HWND handle;
};
global Window window;
static Window win32_window_create(const wchar* window_name, s32 width, s32 height);
#include "game.hpp"

global bool should_quit;
global Arena* global_arena = os_make_arena(MB(100));

static void show_cursor(bool show);
static void init_paths(Arena* arena);

s32 WinMain(HINSTANCE instance, HINSTANCE pinstance, LPSTR command_line, s32 window_type);
static LRESULT win_message_handler_callback(HWND hwnd, u32 message, u64 w_param, s64 l_param);

#define MAX_LEVELS 3
#define MAX_LIVES 3
#define WIN_SCORE 3000
#define ENTITIES_MAX 4096
typedef struct PermanentMemory{
    Arena arena;
    u32 game_mode; // GameMode

    Entity entities[ENTITIES_MAX];
    u32 entities_count;

    u32 generation[ENTITIES_MAX];
    u32 free_entities[ENTITIES_MAX];
    u32 free_entities_at;

    Entity* ship;
    bool ship_loaded;
    s32 score;
    s32 lives;

    Level levels[MAX_LEVELS];
    s32 level_index;
    Level* current_level;
    Font* font;

    u32 current_font;
    f64 spawn_t;

#define SUB_CATEGORIES_MAX 1024
    u32 sub_categories_count;
    u32 sub_categories_index;
    String8 sub_categories[SUB_CATEGORIES_MAX];
    bool selected_sub_categories[1024];

#define CATEGORIES_MAX 1024
    u32 categories_count;
    u32 categories_index;
    String8 categories[CATEGORIES_MAX];
} PermanentMemory, State;
global PermanentMemory* pm;

typedef struct TransientMemory{
    Arena arena;
    Arena *frame_arena;
    Arena *render_command_arena;
    Arena *asset_arena;

    Assets assets;
} TransientMemory;
global TransientMemory* tm;

f32 text_padding = 20;

#include <string>
#include <vector>

typedef struct Row {
    std::string row_number;
    char input[128];
    char planned[128];
    char actual[128];
    std::string diff;
} Row;

typedef struct Category{
    std::string row_number;
    char input[128];
    s32 planned;
    s32 actual;
    std::string diff;

    u32 row_count;

    std::vector<Row> rows;
} Category;

std::vector<Category> categories;

f32 column1_start = 8.0f;
f32 column1_width = 45.0f;

f32 column2_start = column1_start + column1_width;
f32 column2_width = 100.0f;

f32 column3_start = column2_start + column2_width + 5.0f;
f32 column3_width = 50.0f;

f32 column4_start = column3_start + column3_width + 5.0f;
f32 column4_width = 50.0f;

f32 column5_start = column4_start + column4_width + 5.0f;
f32 column5_width = 50.0f;

f32 column6_start = column5_start + column5_width + 5.0f;
f32 column6_width = 15.0f;

f32 column7_start = column6_start + column6_width + 5.0f;

f32 row_column1_start = 30.0f;
f32 row_column1_width = 23.0f;

f32 row_column2_start = row_column1_start + row_column1_width;
f32 row_column2_width = 100.0f;

f32 row_column3_start = row_column2_start + row_column2_width + 5.0f;
f32 row_column3_width = 50.0f;

f32 row_column4_start = row_column3_start + row_column3_width + 5.0f;
f32 row_column4_width = 50.0f;

f32 row_column5_start = row_column4_start + row_column4_width + 5.0f;
f32 row_column5_width = 50.0f;

f32 row_column6_start = row_column5_start + row_column5_width + 5.0f;
f32 row_column6_width = 15.0f;

f32 row_column7_start = row_column6_start + row_column6_width + 5.0f;
static int InputTextCallback(ImGuiInputTextCallbackData* data){
    if (data->EventChar < '0' || data->EventChar > '9')
        print("no\n");
        return 0;

    return 1;
}

#endif
