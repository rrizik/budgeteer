#ifndef MAIN_H
#define MAIN_H

#if DEBUG
#define ENABLE_ASSERT 1
#endif

#pragma comment(lib, "user32")
// note: libs for tinyfiledialogs
#pragma comment(lib, "ole32")
#pragma comment(lib, "comdlg32")
#pragma comment(lib, "Shell32")

#include "base_inc.h"
#include "win32_base_inc.h"

#define PROFILER 1
#include "profiler.h"

#include "input.hpp"
#include "clock.hpp"
#include "d3d11_init.hpp"

#include "input.cpp"
#include "clock.cpp"
#include "d3d11_init.cpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#include "tinyfiledialogs/tinyfiledialogs.h"

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

typedef enum WindowType{
    WindowType_Fullscreen,
    WindowType_Windowed,
} WindowType;

typedef struct Window{
    union{
        struct{
            f32 width;
            f32 height;
        };
        v2 dim;
    };
    f32 aspect_ratio;

    HWND handle;
    WindowType type;
} Window;
global Window window;
static Window win32_window_create(const wchar* window_name, s32 width, s32 height);

global bool should_quit;
global Arena* global_arena = os_make_arena(MB(100));

static void show_cursor(bool show);
static void init_paths(Arena* arena);

s32 WinMain(HINSTANCE instance, HINSTANCE pinstance, LPSTR command_line, s32 window_type);
static LRESULT win_message_handler_callback(HWND hwnd, u32 message, u64 w_param, s64 l_param);


typedef struct Row{
    Row* next;
    Row* prev;

    char name[128];
    char planned[128];
    f32 actual;
    f32 diff;

    bool muted;
} Row;

typedef struct Category{
    Category* next;
    Category* prev;
    Row* rows;

    char name[128];
    f32 planned;
    f32 actual;
    f32 diff;

    u32 row_count;
    bool draw_rows;
    bool muted;
} Category;

typedef struct Transaction{
    Transaction* next;
    Transaction* prev;

    char date[128];
    char amount[128];
    char description[128];
    char selection[128];

    bool muted;
} Transation;

typedef struct Totals{
    f32 planned;
    f32 actual;
    f32 diff;
    f32 saved;
    f32 goal;
} Totals;

typedef struct MonthInfo{
    Month type;
    Transaction* transactions;
    u32 transactions_count;

    Totals totals;
    bool muted;
} MonthInfo;

typedef struct PermanentMemory{
    // memory
    Arena arena;
    PoolArena* category_pool;
    PoolArena* row_pool;
    PoolArena* transaction_pool;
    Arena* data_arena;

    // category/rows/months/transactions
    Category* categories;
    MonthInfo months[12];
    MonthInfo* month;
    u32 month_tab_idx; // selected month tab idx

    u32 total_rows_count;
    u32 categories_count;
    u32 transactions_count;

    // for creating the transaction selection list
	String8* selection_list;
    u32 selection_count;

    // for config loading
    String8 date_names[32];
    u32 date_names_count;
    String8 amount_names[32];
    u32 amount_names_count;
    String8 desc_names[32];
    u32 desc_names_count;

    // for setting tab flags
    u32 tab_flags[12];

    String8 default_path;

    // budget totals
    String8 budget;
    //char budget[128];

    //Totals month_totals[12];
    s32 quarter_idx;
    s32 biannual_idx;
    Totals quarter_totals[4];
    Totals biannual_totals[2];
    Totals annual_totals;

    bool draw_month_plan;
    f32 hover_time;
    f32 epsilon;


    ImVec4 default_button_color;
    ImVec4 default_button_hovered_color;

} PermanentMemory, State;
global PermanentMemory* pm;

typedef struct TransientMemory{
    Arena arena;
    Arena *frame_arena;
    Arena *options_arena;

} TransientMemory;
global TransientMemory* tm;

static f32
round_to_hundredth(f32 value){
    value = value * 100;
    value = round_f32(value);
    value = value / 100;
    return(value);
}

static f32 input_padding = 4.0f;
static f32 totals_number_start = 75.0f;

static f32 collapse_column_start = 8.0f;
static f32 collapse_column_width = 25.0f;

static f32 row_count_column_start = collapse_column_start + collapse_column_width;
static f32 row_count_column_width = 25.0f;

static f32 category_column_start = row_count_column_start + row_count_column_width;
static f32 category_column_width = 100.0f;

static f32 planned_column_start = category_column_start + category_column_width + 10.0f;
static f32 planned_column_width = 75.0f;

static f32 actual_column_start = planned_column_start + planned_column_width + 10.0f;
static f32 actual_column_width = 50.0f;

static f32 diff_column_start = actual_column_start + actual_column_width + 10.0f;
static f32 diff_column_width = 75.0f;

static f32 plus_column_start = diff_column_start + diff_column_width + 10.0f;
static f32 plus_column_width = 15.0f;

static f32 x_column_start = plus_column_start + plus_column_width + 5.0f;
static f32 x_column_width = 15.0f;

static f32 m_column_start = x_column_start + x_column_width + 5.0f;

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

static f32 hash_column_start = 20;
static f32 hash_column_width = 20;
static f32 date_column_start = hash_column_start + hash_column_width + 10;
static f32 date_column_width = 80;
static f32 amount_column_start = date_column_start + date_column_width + 20;
static f32 amount_column_width = 75;
static f32 description_column_start = amount_column_start + amount_column_width + 20;
static f32 description_column_width = 160;
static f32 category_select_column_start = description_column_start + description_column_width + 20;
static f32 category_select_column_width = 100;
static f32 plus_expense_column_start = category_select_column_start + category_select_column_width;
static f32 plus_expense_column_width = 23;
static f32 x_expense_column_start = plus_expense_column_start + plus_expense_column_width;
//static f32 plus_expense_column_width = 75;


static bool
char_compare(char* left, char* right){
    u32 count = 0;

    while(*left){
        if(*left++ != *right++){
            return(false);
        }
        count++;
    }
    left = left - count;
    right = right - count;
    return(true);
}

// todo: remove this once you change to str8 for everything
static bool
char_only_spaces(char* src){
    u32 count = 0;
    while(*src){
        if(!(*src++ == ' ')){
            return(false);
        }
    }
    return(true);
}

// todo: remove this once you change to str8 for everything
static void
copy_word_to_char(char* c, String8 string){
    for(s32 i=0; i < string.size; ++i){
        c[i] = string.str[i];
    }
    c[string.size] = '\0';
    if(c[string.size - 1] == '\n' || c[string.size - 1] == '\x1B'){
        c[string.size - 1] = '\0';
    }
}

// note: this is dangeours if dst doesn't have enough memory
static void
str8_copy(String8* dst, String8* src){
    s32 count = 0;
    while(src->count != count){
        dst->data[count] = src->data[count];
        ++count;
    }
    dst->count = src->count;
}

static u32
str8_extend_to_char(String8* string, char c){
    u8* opl = string->str + string->size;
    u32 count = 0;
    bool found = false;
    while(*opl != '\n' && *opl != '\0' && !found){
        if(*opl == c){
            found = true;
        }
        count++;
        opl++;
    }
    if(*opl == c){
        ++opl; // consume newline char
        ++count; // consume newline char
    }

    if(found){
        string->size += count;
    }
    else{
        count = 0;
    }
    return(count);
}

// todo: verify that I need to do this
static bool
str8_strip_newline(String8* string){
    bool result = false;

    if(string->data[string->count - 1] == '\n'){
        string->count--;
        result = true;
    }

    return(result);
}

// todo: verify that I need to do this
static bool
str8_strip_quotes(String8* string){
    bool result = false;
    if(string->data[0] == '"'){
        string->data = string->data + 1;
        --string->count;
        result = true;
    }
    if(string->data[string->count - 1] == '"'){
        --string->count;
        result = true;
    }

    return(result);
}

static String8
str8_eat_word_csv(String8* string){
    String8 result = {0};
    str8_eat_spaces(string);

    u64 count = 0;
    while(string->count){
        if((*string->data == ',') || (*string->data == '\n')){
            break;
        }

        str8_advance(string, 1);
        ++count;
    }

    result = {string->data - count, count};
    str8_advance(string, 1);
    return(result);
}


//static String8
//str8_next_csv_word(String8* string){
//    String8 result = {0};
//    str8_eat_spaces(string);
//
//    u8* ptr = string->str;
//    u32 count = 0;
//    if(string->size){
//        while(*ptr != ',' && *ptr != '\0'){
//            count++;
//            ptr++;
//            if(count >= string->size){
//                result = {string->str, count};
//                str8_advance(string, count);
//                return(result);
//            }
//        }
//
//        result = {string->str, count};
//        // note: +1 to account for comma in csv format
//        str8_advance(string, count + 1);
//    }
//
//    return(result);
//}

typedef enum ParsingState{
    ParsingState_None,
    ParsingState_Budget,
    ParsingState_Category,
    ParsingState_Row,
    ParsingState_Month,
    ParsingState_Transaction,
    ParsingState_Config,

    ParsingState_Date,
    ParsingState_Amount,
    ParsingState_Description,

    ParsingState_Count,
} ParsingState;

ParsingState state = ParsingState_None;

static void
load_csv(String8 full_path){

    File file = os_file_open(full_path, GENERIC_READ, OPEN_EXISTING);
    if(!file.size){
        //todo: log error
        print("Error: failed to open file <%s>\n", full_path.str);
        os_file_close(file);
        return;
    }
    ScratchArena scratch = begin_scratch();

    String8 data = os_file_read(scratch.arena, file);
    String8* ptr = &data;

    state = ParsingState_None;
    s32 date_idx = -1;
    s32 amount_idx = -1;
    s32 desc_idx = -1;
    bool header = true;
    String8 line = {0};
    while(ptr->size){
        line = str8_eat_line(ptr);

        if(header){
            String8 word;
            u32 word_count = 0;
            while(line.size){
                word = str8_eat_word_csv(&line);
                for(u32 i=0; i < 32; ++i){
                    if(str8_compare(pm->date_names[i], word)){
                        date_idx = word_count;
                    }
                    else if(str8_compare(pm->amount_names[i], word)){
                        amount_idx = word_count;
                    }
                    else if(str8_compare(pm->desc_names[i], word)){
                        desc_idx = word_count;
                    }
                }
                ++word_count;
            }
            header = false;
        }

        else if(line.size){
            Transaction* trans = (Transaction*)pool_next(pm->transaction_pool);
            dll_push_back(pm->month->transactions, trans);
            ++pm->month->transactions_count;

            u32 count = 0;
            String8 word;
            while(line.size){
                word = str8_eat_word_csv(&line);
                str8_strip_newline(&word);
                if(count == date_idx){
                    if(word.size == 0){
                        copy_word_to_char(trans->date, str8_literal("\0"));
                    }
                    else{
                        copy_word_to_char(trans->date, word);
                    }
                }
                else if(count == amount_idx){
                    if(word.size == 0){
                        copy_word_to_char(trans->amount, str8_literal("\0"));
                    }
                    else{
                        if(str8_starts_with(word, str8_literal("-"))){
                            str8_advance(&word, 1);
                        }
                        copy_word_to_char(trans->amount, word);
                    }
                }
                else if(count == desc_idx){
                    if(word.size == 0){
                        copy_word_to_char(trans->description, str8_literal("\0"));
                    }
                    else{
                        String8* view = &word;
                        str8_strip_quotes(view);
                        copy_word_to_char(trans->description, *view);
                    }
                }

                ++count;
            }
        }
    }

    state = ParsingState_None;
    os_file_close(file);
    end_scratch(scratch);
}

static void
load_config(void){
    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_path_append(scratch.arena, build_path, str8_literal("config.conf"));

    File file = os_file_open(full_path, GENERIC_READ, OPEN_EXISTING);
    if(!file.size){
        //todo: log error
        print("Error: failed to open file <%s>\n", full_path.str);
        os_file_close(file);
        end_scratch(scratch);
        return;
    }

    String8 data = os_file_read(scratch.arena, file);
    String8* ptr = &data;

    state = ParsingState_None;
    String8 line = {0};
    while(ptr->size){
        line = str8_eat_line(ptr);
        if(str8_starts_with(line, str8_literal("#"))){
            if(str8_compare(line, str8_literal("#date\n"))){
                state = ParsingState_Date;
            }
            else if(str8_compare(line, str8_literal("#amount\n"))){
                state = ParsingState_Amount;
            }
            else if(str8_compare(line, str8_literal("#description\n"))){
                state = ParsingState_Description;
            }
        }
        else if(state == ParsingState_Date){
            String8 word;
            while(line.size){
                word = str8_eat_word_csv(&line);
                String8* str = pm->date_names + pm->date_names_count;
                ++pm->date_names_count;

                memcpy(str->data, word.data, word.count);
                str->count = word.count;
                str8_strip_newline(str);
            }
        }
        else if(state == ParsingState_Amount){
            String8 word;
            while(line.size){
                word = str8_eat_word_csv(&line);
                String8* str = pm->amount_names + pm->amount_names_count;
                ++pm->amount_names_count;

                memcpy(str->data, word.data, word.count);
                str->count = word.count;
                str8_strip_newline(str);
            }
        }
        else if(state == ParsingState_Description){
            String8 word;
            while(line.size){
                word = str8_eat_word_csv(&line);
                String8* str = pm->desc_names + pm->desc_names_count;
                ++pm->desc_names_count;

                memcpy(str->data, word.data, word.count);
                str->count = word.count;
                str8_strip_newline(str);
            }
        }
    }
    state = ParsingState_None;
}

static void
deserialize_data(void){
    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_path_append(scratch.arena, saves_path, str8_literal("budget.b"));

    File file = os_file_open(full_path, GENERIC_READ, OPEN_EXISTING);
    if(!file.size){
        //todo: log error
        print("Error: file size 0, no data to load. <%s>\n", (char*)full_path.str);
        os_file_close(file);
        end_scratch(scratch);
        return;
    }

    String8 data = os_file_read(scratch.arena, file);
    String8* ptr = &data;

    s32 month_idx = 0;
    state = ParsingState_None;
    String8 line = {0};
    while(ptr->size){
        line = str8_eat_line(ptr);

        if(str8_starts_with(line, str8_literal("#"))){
            if(str8_compare(line, str8_literal("#budget\n"))){
                state = ParsingState_Budget;
            }
            else if(str8_compare(line, str8_literal("#category\n"))){
                state = ParsingState_Category;
            }
            else if(str8_contains(line, str8_literal("#month"))){
                state = ParsingState_Month;
                pm->month = pm->months + month_idx;
                ++month_idx;
            }
            else if(str8_compare(line, str8_literal("#config\n"))){
                state = ParsingState_Config;
            }
        }
        else if(state == ParsingState_Budget){
            String8 word = str8_eat_word(&line);

            String8Node str8_node = {0};
            str8_node = str8_split(scratch.arena, word, '=');
            str8_copy(&pm->budget, &str8_node.prev->str);
            //copy_word_to_char(pm->budget, str8_node.prev->str);
        }
        else if(state == ParsingState_Category){
            Category* category = (Category*)pool_next(pm->category_pool);
            dll_push_back(pm->categories, category);
            category->rows = (Row*)pool_next(pm->row_pool);
            dll_clear(category->rows);
            ++pm->categories_count;

            while(line.size){
                String8 word = str8_eat_word(&line);

                String8Node str8_node = {0};
                if(str8_contains(word, str8_literal("name"))){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_to_char(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    str8_node = str8_split(scratch.arena, word, '=');
                    if(str8_compare(str8_node.prev->str, str8_node.next->str)){
                        copy_word_to_char(category->name, str8_literal("\0"));
                    }
                    else{
                        copy_word_to_char(category->name, str8_node.prev->str);
                    }
                }
                else if(str8_contains(word, str8_literal("draw_rows"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    category->draw_rows = atoi((char*)str8_node.prev->str.str);
                }
                else if(str8_contains(word, str8_literal("muted"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    category->muted = atoi((char*)str8_node.prev->str.str);
                }
            }
            state = ParsingState_Row;
        }
        else if(state == ParsingState_Row){
            Category* category = pm->categories->prev;
            ++category->row_count;

            Row* row = (Row*)pool_next(pm->row_pool);
            dll_push_back(category->rows, row);
            ++pm->total_rows_count;

            while(line.size){
                String8 word = str8_eat_word(&line);

                String8Node str8_node = {0};
                if(str8_contains(word, str8_literal("name"))){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_to_char(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    str8_node = str8_split(scratch.arena, word, '=');
                    if(str8_compare(str8_node.prev->str, str8_node.next->str)){
                        copy_word_to_char(row->name, str8_literal("\0"));
                    }
                    else{
                        copy_word_to_char(row->name, str8_node.prev->str);
                    }
                }
                else if(str8_contains(word, str8_literal("planned"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    copy_word_to_char(row->planned, str8_node.prev->str);
                }
                else if(str8_contains(word, str8_literal("muted"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    row->muted = atoi((char*)str8_node.prev->str.str);
                }
            }
        }
        else if(state == ParsingState_Month){
            while(line.size){
                String8 word = str8_eat_word(&line);

                String8Node str8_node = {0};
                if(str8_contains(word, str8_literal("muted"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    pm->month->muted = atoi((char*)str8_node.prev->str.str);
                }
            }
            state = ParsingState_Transaction;
        }
        else if(state == ParsingState_Transaction){

            Transaction* trans;
            if(line.size){
                trans = (Transaction*)pool_next(pm->transaction_pool);
                dll_push_back(pm->month->transactions, trans);
                ++pm->month->transactions_count;
            }

            while(line.size){
                String8 word = str8_eat_word(&line);

                String8Node str8_node = {0};
                if(str8_contains(word, str8_literal("date"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    if(str8_compare(str8_node.prev->str, str8_node.next->str)){
                        copy_word_to_char(trans->date, str8_literal("\0"));
                    }
                    else{
                        copy_word_to_char(trans->date, str8_node.prev->str);
                    }
                }
                else if(str8_contains(word, str8_literal("amount"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    if(str8_compare(str8_node.prev->str, str8_node.next->str)){
                        copy_word_to_char(trans->amount, str8_literal("\0"));
                    }
                    else{
                        copy_word_to_char(trans->amount, str8_node.prev->str);
                    }
                }
                else if(str8_contains(word, str8_literal("description"))){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_to_char(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    str8_node = str8_split(scratch.arena, word, '=');
                    if(str8_compare(str8_node.prev->str, str8_node.next->str)){
                        copy_word_to_char(trans->description, str8_literal("\0"));
                    }
                    else{
                        copy_word_to_char(trans->description, str8_node.prev->str);
                    }
                }
                else if(str8_contains(word, str8_literal("selection"))){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_to_char(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    str8_node = str8_split(scratch.arena, word, '=');
                    if(str8_compare(str8_node.prev->str, str8_node.next->str)){
                        copy_word_to_char(trans->selection, str8_literal("\0"));
                    }
                    else{
                        copy_word_to_char(trans->selection, str8_node.prev->str);
                    }
                }
                else if(str8_contains(word, str8_literal("muted"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    trans->muted = atoi((char*)str8_node.prev->str.str);
                }
            }
        }
        else if(state == ParsingState_Config){
            while(line.size){
                String8 word = str8_eat_word(&line);

                String8Node str8_node = {0};
                if(str8_contains(word, str8_literal("month_tab_idx"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    pm->month_tab_idx = atoi((char*)str8_node.prev->str.str);
                }
            }
        }
    }

    state = ParsingState_None;
    os_file_close(file);
    end_scratch(scratch);
}

static void
serialize_data(void){
    Arena* arena = pm->data_arena;
    Category* c = pm->categories;

    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "#budget\n");
    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "budget=%s\n", pm->budget.str);
    //arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "budget=%s\n", pm->budget);

    for(s32 c_idx = 0; c_idx < pm->categories_count; ++c_idx){
        c = c->next;

        arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "#category\n");
        arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at,
                              "name=%s\x1B draw_rows=%i muted=%i\n", c->name, c->draw_rows, c->muted);

        Row* r = c->rows;
        for(s32 r_idx = 0; r_idx < c->row_count; ++r_idx){
            r = r->next;
            arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at,
                                  "\tname=%s\x1B planned=%s muted=%i\n", r->name, r->planned, r->muted);
        }
    }

    for(s32 m_idx=0; m_idx < Month_Count; ++m_idx){

        pm->month = pm->months + m_idx;
        arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "#month_m%i\n", m_idx);
        arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "muted=%i\n", pm->month->muted);

        Transaction* t = pm->month->transactions;
        for(s32 t_idx = 0; t_idx < pm->month->transactions_count; ++t_idx){
            t = t->next;
            arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at,
                                  "date=%s amount=%s description=%s\x1B selection=%s\x1B muted=%i\n",
                                  t->date, t->amount, t->description, t->selection, t->muted);
        }
    }
    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "#config\n");
    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "month_tab_idx=%i\n", pm->month_tab_idx);

    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "\0");

    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_path_append(scratch.arena, saves_path, str8_literal("budget.b"));

    File file = os_file_open(full_path, GENERIC_WRITE, CREATE_ALWAYS);
    if(file.handle != INVALID_HANDLE_VALUE){
        os_file_write(file, arena->base, arena->at);
    }

    os_file_close(file);
    end_scratch(scratch);
}


#endif
