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

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
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

    u32 transactions_count;
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
    //std::string row_number;
    char input[128];
    char planned[128];
    char actual[128];
    std::string diff;
} Row;

typedef struct Category{
    //std::string row_number;
    char input[128];
    s32 planned;
    s32 actual;
    s32 diff;
    //std::string diff;

    bool draw_rows;
    u32 row_count;

    std::vector<Row> rows;
} Category;

static std::vector<Category> categories;
static char budget[128];
static s32 total_planned;
static s32 total_actual;
static s32 total_diff;
static s32 total_saved;

static f32 input_padding = 4.0f;
static f32 totals_number_start = 75.0f;

static f32 collapse_column_start = 8.0f;
static f32 collapse_column_width = 25.0f;

static f32 row_count_column_start = collapse_column_start + collapse_column_width;
static f32 row_count_column_width = 25.0f;

static f32 category_column_start = row_count_column_start + row_count_column_width;
static f32 category_column_width = 100.0f;

static f32 planned_column_start = category_column_start + category_column_width + 5.0f;
static f32 planned_column_width = 50.0f;

static f32 actual_column_start = planned_column_start + planned_column_width + 5.0f;
static f32 actual_column_width = 50.0f;

static f32 diff_column_start = actual_column_start + actual_column_width + 5.0f;
static f32 diff_column_width = 50.0f;

static f32 plus_column_start = diff_column_start + diff_column_width + 5.0f;
static f32 plus_column_width = 15.0f;

static f32 x_column_start = plus_column_start + plus_column_width + 5.0f;

static void custom_separator(f32 thickness = 1.0f) {
    float columnWidth = ImGui::GetColumnWidth();
    float padding = ImGui::GetStyle().WindowPadding.x;
    float lineWidth = columnWidth - 2 * padding;

    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddLine(
        ImVec2(cursorPos.x, cursorPos.y),
        ImVec2(cursorPos.x + lineWidth, cursorPos.y),
        ImGui::GetColorU32(ImGuiCol_Separator),
        thickness
    );

    ImGui::Dummy(ImVec2(0.0f, thickness));
}

static f32 date_column_start = 100;
static f32 amount_column_start = 175;
static f32 description_column_start = 250;
static f32 category_select_column_start = 350;
static f32 plus_expense_column_start = 450;

static void update_column2_pos(f32 pos, f32 dynamic_value){
    date_column_start = date_column_start + dynamic_value;
    amount_column_start = amount_column_start + dynamic_value;
    description_column_start = description_column_start + dynamic_value;
    category_select_column_start = category_select_column_start + dynamic_value;
    plus_expense_column_start = plus_expense_column_start + dynamic_value;
}

typedef struct Transaction{
    u32 date;
    u32 amount;
    char description[128];
    char category[128];

} Transation;

static std::vector<Transaction> transactions;

static f32 get_ui_right_x(){
    ImVec2 window_pos = ImGui::GetWindowPos();

// Get the size of the ImGui window
    ImVec2 window_size = ImGui::GetWindowSize();

// Calculate the X position of the right side of the window
    f32 right_side = window_pos.x + window_size.x;
    return(right_side);
}

#endif
