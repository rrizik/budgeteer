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
#define PROFILER_TIMER READ_TIMESTAMP_COUNTER
#include "profiler.h"
#define CLOCK_TIMER READ_TIMESTAMP_COUNTER
#include "clock.hpp"

#include "input.hpp"
#include "window.hpp"
#include "bitmap.hpp"
#include "d3d11_init.hpp"
#include <time.h>

#include "clock.cpp"
#include "input.cpp"
#include "bitmap.cpp"
#include "d3d11_init.cpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#include "tinyfiledialogs/tinyfiledialogs.h"
#include "meta.h"

static String8 build_path;
static String8 fonts_path;
static String8 shaders_path;
static String8 saves_path;
static String8 sprites_path;
static String8 sounds_path;

static String8 fmt;
static RGBA RED;
static RGBA GREEN;

static ImFont* my_font12;
static ImFont* my_font20;
static char icon_lookup[] = {' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v'};
typedef enum Icon{
    Icon_None,

    Icon_UpArrow,
    Icon_DownArrow,
    Icon_LeftArrow,
    Icon_RightArrow,

    Icon_DoubleLeftArrow,
    Icon_DoubleRightArrow,

    Icon_UpTriangle,
    Icon_DownTriangle,
    Icon_LeftTriangle,
    Icon_RightTriangle,

    Icon_Collapse,
    Icon_Expand,

    Icon_CheckSquare,
    Icon_Check,
    Icon_XSquare,
    Icon_X,
    Icon_PlusSquare,
    Icon_Plus,
    Icon_Square,
    Icon_SquareRounded,

    Icon_MagA,
    Icon_MagB,

    Icon_Count,
} Icon;

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

#define DEFAULT_SCREEN_WIDTH 1600
#define DEFAULT_SCREEN_HEIGHT 720

static Window win32_window_create(const wchar* window_name, s32 x, s32 y, s32 width, s32 height, bool maximized=false);

global bool should_quit;
global Arena* global_arena = os_make_arena(MB(100));

static void show_cursor(bool show);
static void init_paths(Arena* arena);

s32 WinMain(HINSTANCE instance, HINSTANCE pinstance, LPSTR command_line, s32 window_type);
static LRESULT win_message_handler_callback(HWND hwnd, u32 message, u64 w_param, s64 l_param);

static s32
wrap_index(s32 idx, s32 size){
    return(((idx % size) + size) % size);
}

#define ROW_NAME_SIZE 128
#define ROW_PLANNED_SIZE 128
typedef struct Row{
    Row* next;
    Row* prev;

    char name[ROW_NAME_SIZE];
    char planned[ROW_PLANNED_SIZE];
    f32 spent;
    f32 diff;

    bool muted;
} Row;

#define CAT_NAME_SIZE 128
typedef struct Category{
    Category* next;
    Category* prev;
    Row* rows;

    char name[CAT_NAME_SIZE];
    f32 planned; // todo(rr): this is considered a kind of cache, don't do this unless you actually need to
    f32 spent;
    f32 diff;

    u32 row_count;
    bool draw_rows;
    bool muted;
} Category;

#define TRANS_DATE_SIZE 128
#define TRANS_AMOUNT_SIZE 128
#define TRANS_DESCRIPTION_SIZE 1024
#define TRANS_SELECTION_SIZE 128
typedef struct Transaction{
    Transaction* next;
    Transaction* prev;

    char date[TRANS_DATE_SIZE];
    char amount[TRANS_AMOUNT_SIZE];
    char description[TRANS_DESCRIPTION_SIZE];
    char selection[TRANS_SELECTION_SIZE];

    bool muted;
} Transation;

// todo(rr): don't need this, don't store this info. Just calculate it once a free
typedef struct Totals{
    f32 planned;
    f32 spent;
    f32 diff;
    f32 saved;
    f32 goal;
} Totals;

typedef struct MonthInfo{
    Transaction* transactions;
    u32 transaction_count;

    Totals totals;
    bool muted;
} MonthInfo;

#define MAX_YEAR_COUNT 128
typedef struct Year{
    s32 number;
    s32 transaction_count;
    //MonthInfo* month; // todo(rr): decide if this is something you want. Its only for convenience
    MonthInfo months[Month_Count];
} Year;

#define CSV_COLUMN_NAME_SIZE 128
typedef struct CSVColumnNode{
    CSVColumnNode* next;
    CSVColumnNode* prev;

    char name[CSV_COLUMN_NAME_SIZE];
} CSVColumnNode;

typedef enum Format_Kind{
    Format_Kind_NONE,
    Format_Kind_MMDDYYYY_Forward_Slash,
    Format_Kind_DDMMYYYY_Forward_Slash,
    Format_Kind_YYYYMMDD_Forward_Slash,
    Format_Kind_MMDDYYYY_Dash,
    Format_Kind_DDMMYYYY_Dash,
    Format_Kind_YYYYMMDD_Dash,
    Format_Kind_COUNT,
} Format_Kind;

static char date_delimiters[7] = {
    ' ',
    '/',
    '/',
    '/',
    '-',
    '-',
    '-',
};

static String8 date_formats[7] = {
    str8_literal(" "),
    str8_literal("mm/dd/yyyy"),
    str8_literal("dd/mm/yyyy"),
    str8_literal("yyyy/mm/dd"),
    str8_literal("mm-dd-yyyy"),
    str8_literal("dd-mm-yyyy"),
    str8_literal("yyyy-mm-dd"),
};

#define PROFILE_FILE_PATH_SIZE 1024
#define PROFILE_NAME_SIZE 128
#define PROFILE_DATE_SIZE 128
#define PROFILE_AMOUNT_SIZE 128
#define PROFILE_DESCRIPTION_SIZE 1024
#define PROFILE_DATE_FORMAT_SIZE 128
typedef struct CSV_Profile{
    CSV_Profile* next;
    CSV_Profile* prev;
    char file_path[PROFILE_FILE_PATH_SIZE];
    char name[PROFILE_NAME_SIZE];
    char date[PROFILE_DATE_SIZE];
    char amount[PROFILE_AMOUNT_SIZE];
    char description[PROFILE_DESCRIPTION_SIZE];
    char date_format[PROFILE_DATE_FORMAT_SIZE];
    s32  date_format_idx;
} CSV_Profile;

#define SELECTION_LIST_SIZE 128
#define MAX_CATEGORY_COUNT 256
#define MAX_ROW_COUNT 2048
#define MAX_TRANSACTION_COUNT 32768
#define MAX_PROFILE_COUNT 32
#define MAX_SELECTION_LIST_COUNT 1024
typedef struct PermanentMemory{
    // memory
    Arena arena;
    PoolArena* category_pool;
    PoolArena* row_pool;
    PoolArena* transaction_pool;
    PoolArena* csv_profile_pool;
    Arena* data_arena;

    // category/rows/months/transactions
    Category* annual_categories;
    Category* biannual_categories;
    Category* quarter_categories;
    Category* month_categories;

    // Years are setup where 0 == current year, -1 == current year - 1, 1 == current year + 1
    Year years[MAX_YEAR_COUNT];
    Year* year;
    s32 year_idx;
    s32 transaction_year;
    s32 current_year;

    // todo(rr): review total_ for all
    u32 total_rows_count;
    u32 categories_count;
    u32 transaction_count;
    u32 quarter_categories_count;
    u32 biannual_categories_count;
    u32 annual_categories_count;

    // for creating the transaction selection list
    // todo(rr): just turn this into a list
	String8* selection_list;
    u32 selection_count;

    // Profiles
    CSV_Profile* csv_profiles;
    CSV_Profile* csv_profile;
    u32 csv_profile_count;
    u32 csv_profile_idx;
    bool date_header_found;
    bool amount_header_found;
    bool description_header_found;
    bool date_format_found;
    s32 dd_s32;
    s32 mm_s32;
    s32 yyyy_s32;
    String8 dd;
    String8 mm;
    String8 yyyy;
    bool date_input_was_active;
    bool new_file_or_format;

    // for setting tab flags
    u32 month_tab_flags[Month_Count];
    u32 quarter_tab_flags[4];
    u32 biannual_tab_flags[2];
    u32 month_tab_idx;
    s32 quarter_tab_idx;
    s32 biannual_tab_idx;

    // todo(rr) "tinyfiledialogs/tinyfiledialogs.h" somehow caches the last used path even between instances. Maybe I don't need this.
    String8 default_path;
    char csv_path[4096];

    // budget totals
    char budget[128];

    Totals quarter_totals[4];
    Totals biannual_totals[2];
    Totals annual_totals;

    // todo: serialize these
    bool draw_month_plan;
    bool draw_quarter_plan;
    bool draw_biannual_plan;
    bool draw_annual_plan;

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

static ImVec4  combo_popup_background_color;
static ImColor combo_popup_alternating_colors[2];
static String8 last_combo_name;
static const char* month_names[Month_Count] = {"January", "Febuary", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
static ImVec4 default_active_color;
static ImVec4 active_color;

static ImVec4 default_hover_color;
static ImVec4 hover_color;

static f32 input_padding = 4.0f;
static f32 totals_number_start = 75.0f;

static f32 collapse_column_start = 30.0f;
static f32 collapse_column_width = 25.0f;

static f32 row_count_column_start = collapse_column_start + collapse_column_width;
static f32 row_count_column_width = 25.0f;

static f32 category_column_start = row_count_column_start + row_count_column_width;
static f32 category_column_width = 100.0f;

static f32 planned_column_start = category_column_start + category_column_width + 10.0f;
static f32 planned_column_width = 75.0f;

static f32 spent_column_start = planned_column_start + planned_column_width + 10.0f;
static f32 spent_column_width = 50.0f;

static f32 diff_column_start = spent_column_start + spent_column_width + 20.0f;
static f32 diff_column_width = 75.0f;

static f32 plus_column_start = diff_column_start + diff_column_width + 10.0f;
static f32 plus_column_width = 15.0f;

static f32 x_column_start = plus_column_start + plus_column_width + 5.0f;
static f32 x_column_width = 15.0f;

static f32 m_column_start = x_column_start + x_column_width + 5.0f;

static f32 hash_column_start = 25;
static f32 hash_column_width = 20;
static f32 date_column_start = hash_column_start + hash_column_width + 10;
static f32 date_column_width = 80;
static f32 amount_column_start = date_column_start + date_column_width + 20;
static f32 amount_column_width = 90;
static f32 description_column_start = amount_column_start + amount_column_width + 20;
static f32 description_column_width = 160;
static f32 category_select_column_start = description_column_start + description_column_width + 10;
static f32 category_select_column_width = 100;
static f32 plus_expense_column_start = category_select_column_start + category_select_column_width + 10;
static f32 plus_expense_column_width = 23;
static f32 x_expense_column_start = plus_expense_column_start + plus_expense_column_width;

static f32 csv_date_plus_button_start = 160.0f;
static f32 csv_date_plus_button_width = 15.0f;
static f32 csv_date_x_button_start = csv_date_plus_button_start + csv_date_plus_button_width + 5.0f;

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

static void
dll_bubble_sort_amount(Transaction* sentinel, bool ascending=false){
    if(sentinel->next == sentinel || sentinel->prev == sentinel){
        return;
    }

    bool no_swaps = true;
    while(no_swaps){
        no_swaps = false;
        Transaction* current = sentinel->next;
        Transaction* next = sentinel->next->next;
        while(next != sentinel){
            s32 result = strcmp(current->amount, next->amount);

            if(ascending && result > 0){
                dll_swap(current, next, Transaction);
                Transaction* tmp = current;
                current = next;
                next = tmp;
                no_swaps = true;
            }
            else if(!ascending && result < 0){
                dll_swap(current, next, Transaction);
                Transaction* tmp = current;
                current = next;
                next = tmp;
                no_swaps = true;
            }

            current = current->next;
            next = next->next;
        }
    }
}

static void
dll_bubble_sort_date(Transaction* sentinel, bool ascending=false){
    if(sentinel->next == sentinel || sentinel->prev == sentinel){
        return;
    }

    bool no_swaps = true;
    while(no_swaps){
        no_swaps = false;
        Transaction* current = sentinel->next;
        Transaction* next = sentinel->next->next;
        while(next != sentinel){
            s32 result = strcmp(current->date, next->date);

            if(ascending && result > 0){
                dll_swap(current, next, Transaction);
                Transaction* tmp = current;
                current = next;
                next = tmp;
                no_swaps = true;
            }
            else if(!ascending && result < 0){
                dll_swap(current, next, Transaction);
                Transaction* tmp = current;
                current = next;
                next = tmp;
                no_swaps = true;
            }

            current = current->next;
            next = next->next;
        }
    }
}

static void
dll_insertion_sort(Transaction* sentinel){
    // return if sentinel is empty
    if(sentinel->next == sentinel || sentinel->prev == sentinel){
        return;
    }

    // grab first and second node
    Transaction* current = sentinel->next->next;
    while(current != sentinel){
        Transaction* next_unsorted = current->next;

        // find the insertion point
        Transaction* insertion = sentinel->next;
        s32 r = strcmp(current->date, insertion->date);
        while(insertion != sentinel && r > 0){
            insertion = insertion->next;
        }

        dll_remove(current);
        current->next = insertion;
        current->prev = insertion->prev;
        insertion->prev->next = current;
        insertion->prev = current;

        current = next_unsorted;
    }
}

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
copy_str8_to_char(char* c, String8 string, s32 max_size){
    s32 smallest_size = max_size <= string.size ? max_size : string.size;
    for(s32 i=0; i < smallest_size; ++i){
        c[i] = string.str[i];
    }
    c[string.size] = '\0';

    // todo(rr): do I need this?
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
str8_extend_word_to_byte(String8* string, char c){
    u32 count = 0;
    //if(!str8_contains_byte(*string, c)){
        u8* opl = string->str + string->size;
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
    //}
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

static bool
str8_strip_quotes(String8* string){
    bool result = false;

    if(string->data[0] == '"' && string->data[string->count - 1] == '"'){
        string->data = string->data + 1;
        string->count -= 2;
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

typedef enum BudgetParsingState{
    BudgetParsingState_None,
    BudgetParsingState_Budget,
    BudgetParsingState_Category,
    BudgetParsingState_Row,

    BudgetParsingState_Count,
} BudgetParsingState;
BudgetParsingState bps = BudgetParsingState_None;

typedef enum TransactionParsingState{
    TransactionParsingState_None,

    TransactionParsingState_Month,
    TransactionParsingState_Transaction,

    TransactionParsingState_Count,
} TransactionParingState;
TransactionParsingState tps = TransactionParsingState_None;

typedef enum ConfigParsingState{
    ConfigParsingState_None,
    ConfigParsingState_CSV_Profile,
    ConfigParsingState_CSV_Profile_Settings,

    ConfigParsingState_TabsSelected,
    ConfigParsingState_Collapsables,
    ConfigParsingState_Window,
    ConfigParsingState_Restored,
    ConfigParsingState_Tooltips,

    ConfigParsingState_Year,

    ConfigParsingState_Count,
} ConfigParsingState;
ConfigParsingState cps = ConfigParsingState_None;

global s32 window_width;
global s32 window_height;
global s32 window_maximized;
global s32 window_x;
global s32 window_y;
global Rect window_restored_rect;
global bool show_tooltips = true;
global bool year_config_deserialized = false;

static void
parse_day_month_year(Transaction* trans){
    ScratchArena scratch = begin_scratch();

    CSV_Profile* profile = pm->csv_profile;
    char delimiter = date_delimiters[profile->date_format_idx];
    String8 transaction_date = str8_cstring(trans->date);
    String8Node* date_parts = str8_split(scratch.arena, transaction_date, delimiter);

    //String8 dd, mm, yyyy;
    switch(profile->date_format_idx){
        case Format_Kind_MMDDYYYY_Dash:
        case Format_Kind_MMDDYYYY_Forward_Slash:{
            str8_copy(&pm->mm,   &date_parts->next->str);
            str8_copy(&pm->dd,   &date_parts->next->next->str);
            str8_copy(&pm->yyyy, &date_parts->next->next->next->str);
        } break;
        case Format_Kind_DDMMYYYY_Dash:
        case Format_Kind_DDMMYYYY_Forward_Slash:{
            str8_copy(&pm->dd,   &date_parts->next->str);
            str8_copy(&pm->mm,   &date_parts->next->next->str);
            str8_copy(&pm->yyyy, &date_parts->next->next->next->str);
        } break;
        case Format_Kind_YYYYMMDD_Dash:
        case Format_Kind_YYYYMMDD_Forward_Slash:{
            str8_copy(&pm->yyyy, &date_parts->next->str);
            str8_copy(&pm->mm,   &date_parts->next->next->str);
            str8_copy(&pm->dd,   &date_parts->next->next->next->str);
        } break;
    }

    if(pm->dd.count == 1){
        pm->dd.str[1] = pm->dd.str[0];
        pm->dd.str[0] = '0';
        pm->dd.count = 2;
    }

    if(pm->mm.count == 1){
        pm->mm.str[1] = pm->mm.str[0];
        pm->mm.str[0] = '0';
        pm->mm.count = 2;
    }

    pm->dd_s32   = atoi((char*)pm->dd.str);
    pm->mm_s32   = atoi((char*)pm->mm.str);
    pm->yyyy_s32 = atoi((char*)pm->yyyy.str);

    end_scratch(scratch);
}

static Year*
set_year_based_on_date(){
    Year* year;
    bool found = false;
    for(s32 i=0; i < MAX_YEAR_COUNT; ++i){
        year = pm->years + i;
        if(pm->yyyy_s32 == year->number){
            pm->year = pm->years + i;
            //pm->year_idx = i;
            found = true;
            break;
        }
    }
    if(!found){
        year = 0;
    }
    return(year);
}

static MonthInfo*
set_month_based_on_date(){
    MonthInfo* month;
    for(s32 i=1; i < Month_Count + 1; ++i){
        if(pm->mm_s32 == i){
            month = pm->year->months + i - 1;
            break;
        }
    }
    return(month);
}

static bool
test_date_against_format(String8 date){
    ScratchArena scratch = begin_scratch();
    defer(end_scratch(scratch));

    CSV_Profile* profile = pm->csv_profile;
    char delimiter = date_delimiters[profile->date_format_idx];

    if(!str8_contains_byte(date, delimiter)){
        return(false);
    }

    String8Node* date_parts = str8_split(scratch.arena, date, delimiter);

    String8 dd, mm, yyyy;
    switch(profile->date_format_idx){
        case Format_Kind_MMDDYYYY_Dash:
        case Format_Kind_MMDDYYYY_Forward_Slash:{
            mm   = date_parts->next->str;
            dd   = date_parts->next->next->str;
            yyyy = date_parts->next->next->next->str;
        } break;
        case Format_Kind_DDMMYYYY_Dash:
        case Format_Kind_DDMMYYYY_Forward_Slash:{
            dd   = date_parts->next->str;
            mm   = date_parts->next->next->str;
            yyyy = date_parts->next->next->next->str;
        } break;
        case Format_Kind_YYYYMMDD_Dash:
        case Format_Kind_YYYYMMDD_Forward_Slash:{
            yyyy = date_parts->next->str;
            mm   = date_parts->next->next->str;
            dd   = date_parts->next->next->next->str;
        } break;
    }

    if(dd.count > 2 || mm.count > 2 || yyyy.count != 4){
        return(false);
    }

    s32 date_day_value   = atoi((char*)dd.str);
    s32 date_month_value = atoi((char*)mm.str);
    s32 date_year_value  = atoi((char*)yyyy.str);
    if(date_day_value > 31 || date_month_value > 12){
        return(false);
    }

    return(true);
}

static void
test_csv_against_profile(String8 path){
    pm->date_header_found = false;
    pm->amount_header_found = false;
    pm->description_header_found = false;
    pm->new_file_or_format = false;
    pm->date_format_found = false;

    if(!os_path_exists(path)){
        return;
    }

    File file = os_file_open(path, GENERIC_READ, OPEN_EXISTING);
    if(!file.size){
        //todo: log error
        return;
    }

    CSV_Profile* profile = pm->csv_profiles->next;
    for(s32 i=0; i < pm->csv_profile_idx; ++i){
        profile = profile->next;
    }

    ScratchArena scratch = begin_scratch();
    String8 data = os_file_read(scratch.arena, file);
    String8* header_view = &data;

    s32 date_idx = -1;
    String8 line;
    String8 word;
    bool header = true;
    while(header_view->size && header){
        line = str8_eat_line(header_view);

        if(header){
            s32 word_count = 0;
            while(line.size){
                word = str8_eat_word_csv(&line);
                str8_strip_quotes(&word);

                String8 str8_name;
                str8_name = str8_cstring(profile->date);
                if(str8_compare(str8_name, word)){
                    pm->date_header_found = true;
                    date_idx = word_count;
                }
                str8_name = str8_cstring(profile->amount);
                if(str8_compare(str8_name, word)){
                    pm->amount_header_found = true;
                }
                str8_name = str8_cstring(profile->description);
                if(str8_compare(str8_name, word)){
                    pm->description_header_found = true;
                }
                ++word_count;
            }
            header = false;
        }
    }

    pm->date_format_found = false;
    if(pm->date_header_found){
        String8* date_view = &data;
        bool format_good = true;
        while(date_view->size && format_good){
        //while(date_view->size && format_good && pm->new_file_or_format){
            line = str8_eat_line(date_view);

            s32 count = 0;
            while(line.size){
                word = str8_eat_word_csv(&line);
                str8_strip_quotes(&word);

                if(count == date_idx){ // only test against date column
                    s32 idx = str8_index_from_left(word, ',');
                    String8 date = str8_split_left(word, idx);
                    format_good = test_date_against_format(date);
                    break;
                }

                ++count;
            }
        }
        pm->date_format_found = format_good;
        //pm->new_file_or_format = false;
    }

    os_file_close(file);
    end_scratch(scratch);
}

static void
deserialize_csv(String8 full_path){

    File file = os_file_open(full_path, GENERIC_READ, OPEN_EXISTING);
    if(!file.size){
        //todo: log error
        return;
    }

    CSV_Profile* profile = pm->csv_profiles->next;
    for(s32 i=0; i < pm->csv_profile_idx; ++i){
        profile = profile->next;
    }

    ScratchArena scratch = begin_scratch();

    String8 data = os_file_read(scratch.arena, file);
    String8* ptr = &data;

    s32 date_idx = -1;
    s32 amount_idx = -1;
    s32 desc_idx = -1;
    bool header = true;
    String8 line = {0};

    Year* year = pm->year;
    MonthInfo* month = pm->year->months + pm->month_tab_idx;
    while(ptr->size){
        line = str8_eat_line(ptr);

        if(header){
            String8 word;
            u32 word_count = 0;
            while(line.size){
                word = str8_eat_word_csv(&line);
                str8_strip_quotes(&word);

                u64 len;
                String8 str8_name;

                str8_name = str8_cstring(profile->date);
                if(str8_compare(str8_name, word)){
                    date_idx = word_count;
                }
                str8_name = str8_cstring(profile->amount);
                if(str8_compare(str8_name, word)){
                    amount_idx = word_count;
                }
                str8_name = str8_cstring(profile->description);
                if(str8_compare(str8_name, word)){
                    desc_idx = word_count;
                }

                ++word_count;
            }
            header = false;
        }

        else if(line.size){

            Transaction* trans = (Transaction*)pool_next(pm->transaction_pool);

            bool date_parsed = false;
            u32 count = 0;
            String8 word;
            while(line.size){
                word = str8_eat_word_csv(&line);
                str8_strip_newline(&word);
                str8_strip_quotes(&word);

                if(count == date_idx){
                    copy_str8_to_char(trans->date, word, TRANS_DESCRIPTION_SIZE);
                    date_parsed = true;
                }
                else if(count == amount_idx){
                    if(str8_starts_with(word, str8_literal("-"))){
                        str8_advance(&word, 1);
                    }
                    copy_str8_to_char(trans->amount, word, TRANS_DESCRIPTION_SIZE);
                }
                else if(count == desc_idx){
                    String8 view = word;
                    str8_strip_quotes(&view);
                    copy_str8_to_char(trans->description, view, TRANS_DESCRIPTION_SIZE);
                }

                ++count;
            }

            // after we pull all transaction info, use the date to find the correct year and month
            if(pm->date_format_found && date_parsed){
                parse_day_month_year(trans);
                year = set_year_based_on_date();
                month = set_month_based_on_date();

                ScratchArena scratch = begin_scratch();
                String8 result = str8_concat(scratch.arena, pm->yyyy, str8_literal("-"));
                result = str8_concat(scratch.arena, result, pm->mm);
                result = str8_concat(scratch.arena, result, str8_literal("-"));
                result = str8_concat(scratch.arena, result, pm->dd);
                copy_str8_to_char(trans->date, result, TRANS_DESCRIPTION_SIZE);
                end_scratch(scratch);
            }

            // we still want to add the transaction even if the date wasnt found, just for the year selected
            if(year){
                dll_push_back(month->transactions, trans);
                ++month->transaction_count;
                ++year->transaction_count;
            }
            else{
                pool_free(pm->transaction_pool, trans);
            }
            //todo(rr): STATUSBAR ERROR
        }
    }

    pm->transaction_count += year->transaction_count;
    pm->year = pm->years + wrap_index(pm->year_idx, MAX_YEAR_COUNT);
    os_file_close(file);
    end_scratch(scratch);
}

static void
deserialize_config(void){
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

    cps = ConfigParsingState_None;
    String8 line = {0};
    while(ptr->size){
        line = str8_eat_line(ptr);
        if(str8_starts_with(line, str8_literal("#"))){
            if(str8_starts_with(line, str8_literal("#csv_profile"))){
                cps = ConfigParsingState_CSV_Profile;
            }
            else if(str8_compare(line, str8_literal("#profile_settings\n"))){
                cps = ConfigParsingState_CSV_Profile_Settings;
            }
            else if(str8_compare(line, str8_literal("#tabs_selected\n"))){
                cps = ConfigParsingState_TabsSelected;
            }
            else if(str8_compare(line, str8_literal("#collapsables\n"))){
                cps = ConfigParsingState_Collapsables;
            }
            else if(str8_compare(line, str8_literal("#window\n"))){
                cps = ConfigParsingState_Window;
            }
            else if(str8_compare(line, str8_literal("#restored\n"))){
                cps = ConfigParsingState_Restored;
            }
            else if(str8_compare(line, str8_literal("#show_tooltips\n"))){
                cps = ConfigParsingState_Tooltips;
            }
            else if(str8_compare(line, str8_literal("#year\n"))){
                cps = ConfigParsingState_Year;
            }
        }
        else if(cps == ConfigParsingState_CSV_Profile_Settings){
            while(line.size){
                String8 word = str8_eat_word(&line);
                if(word.count){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_word_to_byte(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    String8Node* str8_node = str8_split(scratch.arena, word, '=');
                    String8 key = str8_node->next->str;
                    String8 value = str8_node->prev->str;

                    if(str8_contains(key, str8_literal("csv_profile_idx"))){
                        pm->csv_profile_idx = atoi((char*)value.str);
                    }
                }
            }
        }
        else if(cps == ConfigParsingState_CSV_Profile){

            CSV_Profile* profile = (CSV_Profile*)pool_next(pm->csv_profile_pool);
            dll_push_back(pm->csv_profiles, profile);
            pm->csv_profile_count++;

            while(line.size){
                String8 word = str8_eat_word(&line);
                if(word.count){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_word_to_byte(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    String8Node* str8_node = str8_split(scratch.arena, word, '=');
                    String8 key = str8_node->next->str;
                    String8 value = str8_node->prev->str;

                    if(str8_compare(key, str8_literal("name"))){
                        copy_str8_to_char(profile->name, value, PROFILE_NAME_SIZE);
                    }
                    else if(str8_compare(key, str8_literal("date"))){
                        copy_str8_to_char(profile->date, value, PROFILE_DATE_SIZE);
                    }
                    else if(str8_compare(key, str8_literal("amount"))){
                        copy_str8_to_char(profile->amount, value, PROFILE_AMOUNT_SIZE);
                    }
                    else if(str8_compare(key, str8_literal("description"))){
                        copy_str8_to_char(profile->description, value, PROFILE_DESCRIPTION_SIZE);
                    }
                    else if(str8_compare(key, str8_literal("format"))){
                        copy_str8_to_char(profile->date_format, value, PROFILE_DATE_FORMAT_SIZE);
                    }
                    else if(str8_compare(key, str8_literal("format_idx"))){
                        profile->date_format_idx = atoi((char*)value.str);
                    }
                }
            }
            cps = ConfigParsingState_None;
        }
        else if(cps == ConfigParsingState_TabsSelected){
            while(line.size){
                String8 word = str8_eat_word(&line);
                if(word.count){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_word_to_byte(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    String8Node* str8_node = str8_split(scratch.arena, word, '=');
                    String8 key = str8_node->next->str;
                    String8 value = str8_node->prev->str;

                    if(str8_compare(key, str8_literal("quarter_tab_idx"))){
                        pm->quarter_tab_idx = atoi((char*)value.str);
                    }
                    else if(str8_compare(key, str8_literal("biannual_tab_idx"))){
                        pm->biannual_tab_idx = atoi((char*)value.str);
                    }
                }
            }
        }
        else if(cps == ConfigParsingState_Collapsables){
            while(line.size){
                String8 word = str8_eat_word(&line);
                if(word.count){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_word_to_byte(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    String8Node* str8_node = str8_split(scratch.arena, word, '=');
                    String8 key = str8_node->next->str;
                    String8 value = str8_node->prev->str;

                    if(str8_compare(key, str8_literal("draw_month_plan"))){
                        pm->draw_month_plan = atoi((char*)value.str);
                    }
                    else if(str8_compare(key, str8_literal("draw_quarter_plan"))){
                        pm->draw_quarter_plan = atoi((char*)value.str);
                    }
                    else if(str8_compare(key, str8_literal("draw_biannual_plan"))){
                        pm->draw_biannual_plan = atoi((char*)value.str);
                    }
                    else if(str8_compare(key, str8_literal("draw_annual_plan"))){
                        pm->draw_annual_plan = atoi((char*)value.str);
                    }
                }
            }
        }
        else if(cps == ConfigParsingState_Window){
            while(line.size){
                String8 word = str8_eat_word(&line);
                if(word.count){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_word_to_byte(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    String8Node* str8_node = str8_split(scratch.arena, word, '=');
                    String8 key = str8_node->next->str;
                    String8 value = str8_node->prev->str;

                    if(str8_compare(key, str8_literal("width"))){
                        window_width = atoi((char*)value.str);
                    }
                    else if(str8_compare(key, str8_literal("height"))){
                        window_height = atoi((char*)value.str);
                    }
                    else if(str8_compare(key, str8_literal("maximized"))){
                        window_maximized = atoi((char*)value.str);
                    }
                    else if(str8_compare(key, str8_literal("xpos"))){
                        window_x = atoi((char*)value.str);
                    }
                    else if(str8_compare(key, str8_literal("ypos"))){
                        window_y = atoi((char*)value.str);
                    }
                }
            }
        }
        else if(cps == ConfigParsingState_Restored){
            while(line.size){
                String8 word = str8_eat_word(&line);
                if(word.count){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_word_to_byte(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    String8Node* str8_node = str8_split(scratch.arena, word, '=');
                    String8 key = str8_node->next->str;
                    String8 value = str8_node->prev->str;

                    if(str8_compare(key, str8_literal("left"))){
                        window_restored_rect.left = atoi((char*)value.str);
                    }
                    else if(str8_compare(key, str8_literal("top"))){
                        window_restored_rect.top = atoi((char*)value.str);
                    }
                    else if(str8_compare(key, str8_literal("right"))){
                        window_restored_rect.right = atoi((char*)value.str);
                    }
                    else if(str8_compare(key, str8_literal("bottom"))){
                        window_restored_rect.bottom = atoi((char*)value.str);
                    }
                }
            }
        }
        else if(cps == ConfigParsingState_Tooltips){
            while(line.size){
                String8 word = str8_eat_word(&line);
                if(word.count){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_word_to_byte(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    String8Node* str8_node = str8_split(scratch.arena, word, '=');
                    String8 key = str8_node->next->str;
                    String8 value = str8_node->prev->str;

                    if(str8_compare(key, str8_literal("tooltips"))){
                        show_tooltips = atoi((char*)value.str);
                    }
                }
            }
        }
        else if(cps == ConfigParsingState_Year){
            while(line.size){
                String8 word = str8_eat_word(&line);
                if(word.count){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_word_to_byte(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    String8Node* str8_node = str8_split(scratch.arena, word, '=');
                    String8 key = str8_node->next->str;
                    String8 value = str8_node->prev->str;

                    if(str8_compare(key, str8_literal("year_idx"))){
                        pm->year_idx = atoi((char*)value.str);
                        year_config_deserialized = true;
                    }
                    else if(str8_compare(key, str8_literal("month_tab_idx"))){
                        pm->month_tab_idx = atoi((char*)value.str);
                    }
                }
            }
        }
    }
    cps = ConfigParsingState_None;

    os_file_close(file);
    end_scratch(scratch);
}

static void
serialize_config(void){
    Arena* arena = pm->data_arena;

    CSV_Profile* profile = pm->csv_profiles;
    for(s32 i=0; i < pm->csv_profile_count; ++i){
        profile = profile->next;
        arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "#csv_profile%i\n", i);
        arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at,
                "name=%s\x1B date=%s\x1B amount=%s\x1B description=%s\x1B format=%s\x1B format_idx=%i\n",
                profile->name, profile->date, profile->amount, profile->description, profile->date_format, profile->date_format_idx);
    }
    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "#profile_settings\n");
    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "csv_profile_idx=%i\n", pm->csv_profile_idx);

    // tabs selected
    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "#tabs_selected\n");
    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at,
                          "quarter_tab_idx=%i biannual_tab_idx=%i\n", pm->quarter_tab_idx, pm->biannual_tab_idx);

    // collapsables
    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "#collapsables\n");
    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at,
                          "draw_month_plan=%i draw_quarter_plan=%i draw_biannual_plan=%i draw_annual_plan=%i\n",
                          pm->draw_month_plan, pm->draw_quarter_plan, pm->draw_biannual_plan, pm->draw_annual_plan);

    // window width/height/x/y/restored state
    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "#window\n");
    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at,
                          "xpos=%i ypos=%i width=%i height=%i maximized=%i\n",
                          (s32)window.rect.x0, (s32)window.rect.y0,
                          (s32)window.width, (s32)window.height,
                          window.maximized);
    if(window.maximized){
        arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "#restored\n");
        arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at,
                              "left=%i top=%i right=%i bottom=%i\n",
                              (s32)window_restored_rect.left, (s32)window_restored_rect.top,
                              (s32)window_restored_rect.right, (s32)window_restored_rect.bottom);
    }

    // tooltip
    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "#show_tooltips\n");
    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "show_tooltips=%i\n", show_tooltips);

    // transaction_year
    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "#year\n");
    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at,
            "year_idx=%i month_tab_idx=%i\n", pm->year_idx, pm->month_tab_idx);

    arena->at += snprintf((char*)arena->base + arena->at, arena->size - (s32)arena->at, "\0");
    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_path_append(scratch.arena, build_path, str8_literal("config.conf"));

    File file = os_file_open(full_path, GENERIC_WRITE, CREATE_ALWAYS);
    if(file.handle != INVALID_HANDLE_VALUE){
        os_file_write(file, arena->base, arena->at);
    }

    os_file_close(file);
    end_scratch(scratch);
    arena_free(pm->data_arena);
}

static void
deserialize_year(Year* year){
    ScratchArena scratch = begin_scratch();
    String8 filename = str8_fmt(scratch.arena, "%04d.b", year->number);
    String8 full_path = str8_path_append(scratch.arena, saves_path, filename);

    // todo(rr): I think I prefer this check to encapsulate the function on the outside, but I didn't want to create the file names twice. Maybe think of how you might want to change this, to maybe pass the filename in again
    if(!os_file_exists(saves_path, filename)){
        return;
    }

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
    tps = TransactionParsingState_None;
    String8 line = {0};
    MonthInfo* month;
    while(ptr->size){
        line = str8_eat_line(ptr);

        if(str8_starts_with(line, str8_literal("#"))){
            if(tps == TransactionParsingState_Transaction){
                year->transaction_count += month->transaction_count;
                // todo(rr): maybe serialize transactions_count and check to see if it matches here?
            }
            if(month_idx == 11){
                tps = TransactionParsingState_None;
            }

            if(str8_contains(line, str8_literal("#month"))){
                tps = TransactionParsingState_Month;
                month = year->months + month_idx;
                ++month_idx;
            }
        }
        else if(tps == TransactionParsingState_Month){
            while(line.size){
                String8 word = str8_eat_word(&line);
                if(word.count){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_word_to_byte(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    String8Node* str8_node = str8_split(scratch.arena, word, '=');
                    String8 key = str8_node->next->str;
                    String8 value = str8_node->prev->str;

                    if(str8_compare(key, str8_literal("muted"))){
                        month->muted = atoi((char*)value.str);
                    }
                }
            }
            tps = TransactionParsingState_Transaction;
        }
        else if(tps == TransactionParsingState_Transaction){

            Transaction* trans;
            if(line.size){
                trans = (Transaction*)pool_next(pm->transaction_pool);
                dll_push_back(month->transactions, trans);
                ++month->transaction_count;
            }

            while(line.size){
                String8 word = str8_eat_word(&line);
                if(word.count){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_word_to_byte(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    String8Node* str8_node = str8_split(scratch.arena, word, '=');
                    String8 key = str8_node->next->str;
                    String8 value = str8_node->prev->str;

                    if(str8_compare(key, str8_literal("date"))){
                        copy_str8_to_char(trans->date, value, TRANS_DESCRIPTION_SIZE);
                    }
                    else if(str8_compare(key, str8_literal("amount"))){
                        copy_str8_to_char(trans->amount, value, TRANS_DESCRIPTION_SIZE);
                    }
                    else if(str8_compare(key, str8_literal("description"))){
                        copy_str8_to_char(trans->description, value, TRANS_DESCRIPTION_SIZE);
                    }
                    else if(str8_compare(key, str8_literal("selection"))){
                        copy_str8_to_char(trans->selection, value, TRANS_DESCRIPTION_SIZE);
                    }
                    else if(str8_compare(key, str8_literal("muted"))){
                        trans->muted = atoi((char*)value.str);
                    }
                }
            }
        }
    }

    pm->transaction_count += year->transaction_count;
    tps = TransactionParsingState_None;
    os_file_close(file);
    end_scratch(scratch);
}

static void
serialize_year(Year* year){
    ScratchArena scratch = begin_scratch();
    Arena* arena = pm->data_arena;

    String8 filename = str8_fmt(scratch.arena, "%04d.b", year->number);
    if(year->transaction_count){

        MonthInfo* month;
        for(s32 m_idx=0; m_idx < Month_Count; ++m_idx){
            month = year->months + m_idx;
            arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "#month_m%i\n", m_idx);
            arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "muted=%i\n", month->muted);

            Transaction* t = month->transactions;
            for(s32 t_idx = 0; t_idx < month->transaction_count; ++t_idx){
                t = t->next;
                arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at,
                                      "date=%s\x1B amount=%s\x1B description=%s\x1B selection=%s\x1B muted=%i\n",
                                      t->date, t->amount, t->description, t->selection, t->muted);
            }
        }

        arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "\0");

        String8 full_path = str8_path_append(scratch.arena, saves_path, filename);

        File file = os_file_open(full_path, GENERIC_WRITE, CREATE_ALWAYS);
        if(file.handle != INVALID_HANDLE_VALUE){
            os_file_write(file, arena->base, arena->at);
        }

        os_file_close(file);
        arena_free(pm->data_arena);
    }
    else{
        if(os_file_exists(saves_path, filename)){
            if(!os_file_delete(saves_path, filename)){
                print_last_error(GetLastError());
                assert(1==0); // todo(rr): temporary assert to make sure things are working correctly
            }
        }
    }
    end_scratch(scratch);
}

static void
deserialize_budget(void){
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
    bps = BudgetParsingState_None;
    String8 line = {0};
    while(ptr->size){
        line = str8_eat_line(ptr);

        if(str8_starts_with(line, str8_literal("#"))){
            if(str8_compare(line, str8_literal("#budget\n"))){
                bps = BudgetParsingState_Budget;
            }
            else if(str8_compare(line, str8_literal("#category\n"))){
                bps = BudgetParsingState_Category;
            }
        }
        else if(bps == BudgetParsingState_Budget){
            String8 word = str8_eat_word(&line);
            if(word.count){
                if(!str8_contains_byte(word, '\x1B')){
                    u32 count = str8_extend_word_to_byte(&word, '\x1B');
                    str8_advance(&line, count);
                }
                String8Node* str8_node = str8_split(scratch.arena, word, '=');
                String8 key = str8_node->next->str;
                String8 value = str8_node->prev->str;

                if(str8_compare(key, str8_literal("budget"))){
                    copy_str8_to_char(pm->budget, value, TRANS_DESCRIPTION_SIZE);
                }
            }
        }
        else if(bps == BudgetParsingState_Category){
            Category* category = (Category*)pool_next(pm->category_pool);
            dll_push_back(pm->month_categories, category);
            category->rows = (Row*)pool_next(pm->row_pool);
            dll_clear(category->rows);
            ++pm->categories_count;

            while(line.size){
                String8 word = str8_eat_word(&line);
                if(word.count){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_word_to_byte(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    String8Node* str8_node = str8_split(scratch.arena, word, '=');
                    String8 key = str8_node->next->str;
                    String8 value = str8_node->prev->str;

                    if(str8_compare(key, str8_literal("name"))){
                        copy_str8_to_char(category->name, value, TRANS_DESCRIPTION_SIZE);
                    }
                    else if(str8_compare(key, str8_literal("draw_rows"))){
                        category->draw_rows = atoi((char*)value.str);
                    }
                    else if(str8_compare(key, str8_literal("muted"))){
                        category->muted = atoi((char*)value.str);
                    }
                }
            }
            bps = BudgetParsingState_Row;
        }
        else if(bps == BudgetParsingState_Row){
            Category* category = pm->month_categories->prev;
            ++category->row_count;

            Row* row = (Row*)pool_next(pm->row_pool);
            dll_push_back(category->rows, row);
            ++pm->total_rows_count;

            while(line.size){
                String8 word = str8_eat_word(&line);
                if(word.count){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_word_to_byte(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    String8Node* str8_node = str8_split(scratch.arena, word, '=');
                    String8 key = str8_node->next->str;
                    String8 value = str8_node->prev->str;

                    if(str8_compare(key, str8_literal("name"))){
                        copy_str8_to_char(row->name, value, TRANS_DESCRIPTION_SIZE);
                    }
                    else if(str8_compare(key, str8_literal("planned"))){
                        copy_str8_to_char(row->planned, value, TRANS_DESCRIPTION_SIZE);
                    }
                    else if(str8_compare(key, str8_literal("muted"))){
                        row->muted = atoi((char*)value.str);
                    }
                }
            }
        }
    }

    bps = BudgetParsingState_None;
    os_file_close(file);
    end_scratch(scratch);
}

static void
serialize_budget(void){
    Arena* arena = pm->data_arena;
    Category* c = pm->month_categories;

    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "#budget\n");
    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "budget=%i\n", atoi(pm->budget));

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

    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "\0");

    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_path_append(scratch.arena, saves_path, str8_literal("budget.b"));

    File file = os_file_open(full_path, GENERIC_WRITE, CREATE_ALWAYS);
    if(file.handle != INVALID_HANDLE_VALUE){
        os_file_write(file, arena->base, arena->at);
    }
    os_file_close(file);

    end_scratch(scratch);
    arena_free(pm->data_arena);
}

static void change_resolution(Window* window, f32 width, f32 height);
static bool handle_controller_events(Event event);
static void draw_entire_ui(void);
static void collect_totals_for_months(void);
static void do_one_frame(void);
static void tooltip(String8 str);

static f64 FPS;
static f64 MSPF;
static u64 total_frames;
static u64 frame_count;
//static u32 simulations;
static f64 time_elapsed;
static f64 accumulator;

static u64 last_ticks;
static u64 frame_tick_start;

static RGBA
linear_from_srgb(RGBA color){
    RGBA result = {0};
    result.a = color.a;

    if(color.r < 0.04045f){
        result.r = color.r / 12.92f;
    }
    else{
        result.r = powf(((color.r + 0.055f) / (1.055f)), 2.4f);
    }

    if(color.g < 0.04045f){
        result.g = color.g / 12.92f;
    }
    else{
        result.g = powf(((color.g + 0.055f) / (1.055f)), 2.4f);
    }

    if(color.b < 0.04045f){
        result.b = color.b / 12.92f;
    }
    else{
        result.b = powf(((color.b + 0.055f) / (1.055f)), 2.4f);
    }
    return(result);
}

static RGBA
RGBA_1_to_255(RGBA color){
    RGBA result = {
        .r = color.r * 255,
        .g = color.g * 255,
        .b = color.b * 255,
        .a = color.a * 255,
    };
    return(result);
}
#endif
