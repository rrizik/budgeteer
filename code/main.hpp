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

#include <string>
#include <vector>

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

typedef struct Row{
    char name[128];
    char planned[128];
    s32 actual;
    s32 diff;
} Row;

typedef struct Category{
    char name[128];
    s32 planned;
    s32 actual;
    s32 diff;

    bool draw_rows;
    u32 row_count;

    std::vector<Row> rows;
} Category;

s32 counter = 0;
s32 r_counter = 0;
typedef struct RRow{
    RRow* next;
    RRow* prev;

    char name[128];
    char planned[128];
    s32 actual;
    s32 diff;
} RRow;

typedef struct CCategory{
    CCategory* next;
    CCategory* prev;
    RRow* rows;

    char name[128];
    s32 planned;
    s32 actual;
    s32 diff;

    u32 row_count;
    bool draw_rows;
} CCategory;

typedef struct Transaction{
    Transaction* next;
    Transaction* prev;

    char date[128];
    char amount[128];
    char description[128];
    s32 category_option;
} Transation;

static std::vector<Transaction> transactions;

#define MAX_LEVELS 3
#define MAX_LIVES 3
#define WIN_SCORE 3000
#define ENTITIES_MAX 4096
typedef struct PermanentMemory{
    Arena arena;
    PoolArena* category_pool;
    PoolArena* row_pool;
    PoolArena* transaction_pool;

    u32 total_rows_count;
    u32 categories_count;
    u32 transactions_count;

    CCategory* categories;
    Transaction* transactions;



    Font* font;
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
    f64 spawn_t;
    u32 current_font;
} PermanentMemory, State;
global PermanentMemory* pm;

typedef struct TransientMemory{
    Arena arena;
    Arena *frame_arena;
    Arena *render_command_arena;
    Arena *options_arena;
    Assets assets;

} TransientMemory;
global TransientMemory* tm;

f32 text_padding = 20;

static char budget[128];
static s32 total_planned;
static s32 total_actual;
static s32 total_diff;
static s32 total_saved;

String8* category_options;

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

static f32 date_column_start = 50;
static f32 date_column_width = 80;
static f32 amount_column_start = 150;
static f32 amount_column_width = 75;
static f32 description_column_start = 245;
static f32 description_column_width = 160;
static f32 category_select_column_start = 425;
static f32 category_select_column_width = 100;
static f32 plus_expense_column_start = 520;
//static f32 plus_expense_column_width = 75;


//static f32 get_ui_right_x(){
//    ImVec2 window_pos = ImGui::GetWindowPos();
//
//    ImVec2 window_size = ImGui::GetWindowSize();
//
//    f32 right_side = window_pos.x + window_size.x;
//    return(right_side);
//}

static void
char_copy(char* left, char* right){
    u32 count = 0;
    while(*left){
        *right = *left;
        right++;
        left++;
        count++;
    }
    left = left - count;
    right = right - count;
}

#endif
