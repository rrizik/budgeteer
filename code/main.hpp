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

#define DEFAULT_SCREEN_WIDTH 1600
#define DEFAULT_SCREEN_HEIGHT 720

static Window win32_window_create(const wchar* window_name, s32 x, s32 y, s32 width, s32 height, bool maximized=false);

global bool should_quit;
global Arena* global_arena = os_make_arena(MB(100));

static void show_cursor(bool show);
static void init_paths(Arena* arena);

s32 WinMain(HINSTANCE instance, HINSTANCE pinstance, LPSTR command_line, s32 window_type);
static LRESULT win_message_handler_callback(HWND hwnd, u32 message, u64 w_param, s64 l_param);


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
    Month type;

    Transaction* transactions;
    u32 transactions_count;

    Totals totals;
    bool muted;
} MonthInfo;

typedef struct Year{
    s32 number;
    s32 total_transactions;
    //MonthInfo* month; // todo(rr): decide if this is something you want. Its only for convenience
    MonthInfo months[Month_Count];
} Year;
#define MAX_YEAR_COUNT 128

#define CSV_COLUMN_NAME_SIZE 128
typedef struct CSVColumnNode{
    CSVColumnNode* next;
    CSVColumnNode* prev;

    char name[CSV_COLUMN_NAME_SIZE];
} CSVColumnNode;

#define DATE_FORMAT_SIZE 128
typedef struct Date_Format{
    Date_Format* next;
    Date_Format* prev;
    char name [DATE_FORMAT_SIZE];
} Date_Format;

static String8 date_formats[4] = {
    str8_literal(""),
    str8_literal("mm/dd/yyyy"),
    str8_literal("dd/mm/yyyy"),
    str8_literal("yyyy/mm/dd")
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
    //Date_Format* date_format;
} CSV_Profile;

#define CONFIG_NAMES_COUNT 32
#define SELECTION_LIST_SIZE 128
typedef struct PermanentMemory{
    // memory
    Arena arena;
    PoolArena* category_pool;
    PoolArena* row_pool;
    PoolArena* transaction_pool;
    PoolArena* csv_profile_pool;
    Arena* data_arena;
    //Arena* date_formats_arena;

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
    u32 transactions_count;
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
    s32 csv_profile_count;
    s32 csv_profile_idx;
    bool date_header_found;
    bool amount_header_found;
    bool description_header_found;
    bool date_format_found;

    // Date Formats
    s32 date_formats_count;
    Date_Format* date_formats;
    Date_Format* selected_date_format;

    // for setting tab flags
    u32 month_tab_flags[12];
    u32 quarter_tab_flags[4];
    u32 biannual_tab_flags[2];
    u32 month_tab_idx;
    s32 quarter_tab_idx;
    s32 biannual_tab_idx;

    // todo(rr) "tinyfiledialogs/tinyfiledialogs.h" somehow caches the last used path even between instances. Maybe I don't need this.
    String8 default_path;
    //String8 csv_path;
    char csv_path[4096];

    // budget totals
    // TODO WRONG FIXME(rr): budget needs to change to a char array
    String8 budget;
    //char budget[128];

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
static const char* month_names[12] = {"January", "Febuary", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
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
static f32 amount_column_width = 75;
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

static void test_date_format(String8 format){
    print("%s\n", format.data);
}

static void
test_csv_against_profile(String8 path){
    pm->date_header_found = false;
    pm->amount_header_found = false;
    pm->description_header_found = false;

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
    String8* data_view = &data;

    s32 date_idx = -1;
    String8 line;
    String8 word;
    bool header = true;
    while(data_view->size){
        line = str8_eat_line(data_view);

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
        //else{

        //    s32 count = 0;
        //    while(line.size){
        //        word = str8_eat_word_csv(&line);
        //        str8_strip_quotes(&word);

        //        if(count == date_idx){
        //            s32 idx = str8_index_from_left(word, ',');
        //            String8 format = str8_split_left(word, idx);
        //            //test_date_format(format);
        //            continue;
        //        }

        //        ++count;
        //    }
        //}
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
            dll_push_back(month->transactions, trans);
            ++month->transactions_count;

            u32 count = 0;
            String8 word;
            while(line.size){
                word = str8_eat_word_csv(&line);
                str8_strip_newline(&word);
                str8_strip_quotes(&word);

                if(count == date_idx){
                    if(word.size == 0){
                        copy_str8_to_char(trans->date, str8_literal("\0"), TRANS_DESCRIPTION_SIZE);
                    }
                    else{
                        copy_str8_to_char(trans->date, word, TRANS_DESCRIPTION_SIZE);
                    }
                }
                else if(count == amount_idx){
                    if(word.size == 0){
                        copy_str8_to_char(trans->amount, str8_literal("\0"), TRANS_DESCRIPTION_SIZE);
                    }
                    else{
                        if(str8_starts_with(word, str8_literal("-"))){
                            str8_advance(&word, 1);
                        }
                        copy_str8_to_char(trans->amount, word, TRANS_DESCRIPTION_SIZE);
                    }
                }
                else if(count == desc_idx){
                    if(word.size == 0){
                        copy_str8_to_char(trans->description, str8_literal("\0"), TRANS_DESCRIPTION_SIZE);
                    }
                    else{
                        String8 view = word;
                        str8_strip_quotes(&view);
                        copy_str8_to_char(trans->description, view, TRANS_DESCRIPTION_SIZE);
                    }
                }

                ++count;
            }
        }
    }

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

                String8Node* str8_node;
                if(str8_contains(word, str8_literal("csv_profile_idx"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    pm->csv_profile_idx = atoi((char*)str8_node->prev->str.str);
                }
            }
        }
        else if(cps == ConfigParsingState_CSV_Profile){

            CSV_Profile* profile = (CSV_Profile*)pool_next(pm->csv_profile_pool);
            dll_push_back(pm->csv_profiles, profile);
            pm->csv_profile_count++;

            while(line.size){
                String8 word = str8_eat_word(&line);

                String8Node* str8_node;
                if(str8_contains(word, str8_literal("name"))){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_to_char(&word, '\x1B');
                        str8_advance(&line, count);
                    }

                    str8_node = str8_split(scratch.arena, word, '=');
                    if(str8_compare(str8_node->prev->str, str8_node->next->str)){
                        copy_str8_to_char(profile->name, str8_literal("\0"), PROFILE_NAME_SIZE);
                    }
                    else{
                        copy_str8_to_char(profile->name, str8_node->prev->str, PROFILE_NAME_SIZE);
                    }
                }
                else if(str8_contains(word, str8_literal("date"))){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_to_char(&word, '\x1B');
                        str8_advance(&line, count);
                    }

                    str8_node = str8_split(scratch.arena, word, '=');
                    if(str8_compare(str8_node->prev->str, str8_node->next->str)){
                        copy_str8_to_char(profile->date, str8_literal("\0"), PROFILE_DATE_SIZE);
                    }
                    else{
                        copy_str8_to_char(profile->date, str8_node->prev->str, PROFILE_DATE_SIZE);
                    }
                }
                else if(str8_contains(word, str8_literal("amount"))){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_to_char(&word, '\x1B');
                        str8_advance(&line, count);
                    }

                    str8_node = str8_split(scratch.arena, word, '=');
                    if(str8_compare(str8_node->prev->str, str8_node->next->str)){
                        copy_str8_to_char(profile->amount, str8_literal("\0"), PROFILE_AMOUNT_SIZE);
                    }
                    else{
                        copy_str8_to_char(profile->amount, str8_node->prev->str, PROFILE_AMOUNT_SIZE);
                    }
                }
                else if(str8_contains(word, str8_literal("description"))){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_to_char(&word, '\x1B');
                        str8_advance(&line, count);
                    }

                    str8_node = str8_split(scratch.arena, word, '=');
                    if(str8_compare(str8_node->prev->str, str8_node->next->str)){
                        copy_str8_to_char(profile->description, str8_literal("\0"), PROFILE_DESCRIPTION_SIZE);
                    }
                    else{
                        copy_str8_to_char(profile->description, str8_node->prev->str, PROFILE_DESCRIPTION_SIZE);
                    }
                }
                else if(str8_contains(word, str8_literal("format"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    if(str8_compare(str8_node->prev->str, str8_node->next->str)){
                        copy_str8_to_char(profile->date_format, str8_literal("\0"), PROFILE_DATE_FORMAT_SIZE);
                    }
                    else{
                        copy_str8_to_char(profile->date_format, str8_node->prev->str, PROFILE_DATE_FORMAT_SIZE);
                    }
                }
            }
            cps = ConfigParsingState_None;
        }
        else if(cps == ConfigParsingState_TabsSelected){
            while(line.size){
                String8 word = str8_eat_word(&line);

                String8Node* str8_node;
                if(str8_contains(word, str8_literal("quarter_tab_idx"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    pm->quarter_tab_idx = atoi((char*)str8_node->prev->str.str);
                }
                else if(str8_contains(word, str8_literal("biannual_tab_idx"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    pm->biannual_tab_idx = atoi((char*)str8_node->prev->str.str);
                }
            }
        }
        else if(cps == ConfigParsingState_Collapsables){
            while(line.size){
                String8 word = str8_eat_word(&line);

                String8Node* str8_node = {0};
                if(str8_contains(word, str8_literal("draw_month_plan"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    pm->draw_month_plan = atoi((char*)str8_node->prev->str.str);
                }
                else if(str8_contains(word, str8_literal("draw_quarter_plan"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    pm->draw_quarter_plan = atoi((char*)str8_node->prev->str.str);
                }
                else if(str8_contains(word, str8_literal("draw_biannual_plan"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    pm->draw_biannual_plan = atoi((char*)str8_node->prev->str.str);
                }
                else if(str8_contains(word, str8_literal("draw_annual_plan"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    pm->draw_annual_plan = atoi((char*)str8_node->prev->str.str);
                }
            }
        }
        else if(cps == ConfigParsingState_Window){
            while(line.size){
                String8 word = str8_eat_word(&line);

                String8Node* str8_node = {0};
                if(str8_contains(word, str8_literal("width"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    window_width = atoi((char*)str8_node->prev->str.str);
                }
                else if(str8_contains(word, str8_literal("height"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    window_height = atoi((char*)str8_node->prev->str.str);
                }
                else if(str8_contains(word, str8_literal("maximized"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    window_maximized = atoi((char*)str8_node->prev->str.str);
                }
                else if(str8_contains(word, str8_literal("xpos"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    window_x = atoi((char*)str8_node->prev->str.str);
                }
                else if(str8_contains(word, str8_literal("ypos"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    window_y = atoi((char*)str8_node->prev->str.str);
                }
            }
        }
        else if(cps == ConfigParsingState_Restored){
            while(line.size){
                String8 word = str8_eat_word(&line);

                String8Node* str8_node = {0};
                if(str8_contains(word, str8_literal("left"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    window_restored_rect.left = atoi((char*)str8_node->prev->str.str);
                }
                else if(str8_contains(word, str8_literal("top"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    window_restored_rect.top = atoi((char*)str8_node->prev->str.str);
                }
                else if(str8_contains(word, str8_literal("right"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    window_restored_rect.right = atoi((char*)str8_node->prev->str.str);
                }
                else if(str8_contains(word, str8_literal("bottom"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    window_restored_rect.bottom = atoi((char*)str8_node->prev->str.str);
                }
            }
        }
        else if(cps == ConfigParsingState_Tooltips){
            while(line.size){
                String8 word = str8_eat_word(&line);

                String8Node* str8_node = {0};
                if(str8_contains(word, str8_literal("tooltips"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    show_tooltips = atoi((char*)str8_node->prev->str.str);
                }
            }
        }
        else if(cps == ConfigParsingState_Year){
            while(line.size){
                String8 word = str8_eat_word(&line);

                String8Node* str8_node = {0};
                if(str8_contains(word, str8_literal("year_idx"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    pm->year_idx = atoi((char*)str8_node->prev->str.str);
                    year_config_deserialized = true;
                }
                else if(str8_contains(word, str8_literal("month_tab_idx"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    pm->month_tab_idx = atoi((char*)str8_node->prev->str.str);
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
                "name=%s\x1B date=%s\x1B amount=%s\x1B description=%s\x1B format=%s\n",
                profile->name, profile->date, profile->amount, profile->description, profile->date_format);
    }
    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "#profile_settings\n");
    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "csv_profile_idx=%i\n", pm->csv_profile_idx);

    // date formats
    //arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "#date_formats\n");

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
                year->total_transactions += month->transactions_count;
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

                String8Node* str8_node = {0};
                if(str8_contains(word, str8_literal("muted"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    month->muted = atoi((char*)str8_node->prev->str.str);
                }
            }
            tps = TransactionParsingState_Transaction;
        }
        else if(tps == TransactionParsingState_Transaction){

            Transaction* trans;
            if(line.size){
                trans = (Transaction*)pool_next(pm->transaction_pool);
                dll_push_back(month->transactions, trans);
                ++month->transactions_count;
            }

            while(line.size){
                String8 word = str8_eat_word(&line);

                String8Node* str8_node = {0};
                if(str8_contains(word, str8_literal("date"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    if(str8_compare(str8_node->prev->str, str8_node->next->str)){
                        copy_str8_to_char(trans->date, str8_literal("\0"), TRANS_DESCRIPTION_SIZE);
                    }
                    else{
                        copy_str8_to_char(trans->date, str8_node->prev->str, TRANS_DESCRIPTION_SIZE);
                    }
                }
                else if(str8_contains(word, str8_literal("amount"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    if(str8_compare(str8_node->prev->str, str8_node->next->str)){
                        copy_str8_to_char(trans->amount, str8_literal("\0"), TRANS_DESCRIPTION_SIZE);
                    }
                    else{
                        copy_str8_to_char(trans->amount, str8_node->prev->str, TRANS_DESCRIPTION_SIZE);
                    }
                }
                else if(str8_contains(word, str8_literal("description"))){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_to_char(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    str8_node = str8_split(scratch.arena, word, '=');
                    if(str8_compare(str8_node->prev->str, str8_node->next->str)){
                        copy_str8_to_char(trans->description, str8_literal("\0"), TRANS_DESCRIPTION_SIZE);
                    }
                    else{
                        copy_str8_to_char(trans->description, str8_node->prev->str, TRANS_DESCRIPTION_SIZE);
                    }
                }
                else if(str8_contains(word, str8_literal("selection"))){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_to_char(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    str8_node = str8_split(scratch.arena, word, '=');
                    if(str8_compare(str8_node->prev->str, str8_node->next->str)){
                        copy_str8_to_char(trans->selection, str8_literal("\0"), TRANS_DESCRIPTION_SIZE);
                    }
                    else{
                        copy_str8_to_char(trans->selection, str8_node->prev->str, TRANS_DESCRIPTION_SIZE);
                    }
                }
                else if(str8_contains(word, str8_literal("muted"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    trans->muted = atoi((char*)str8_node->prev->str.str);
                }
            }
        }
    }

    tps = TransactionParsingState_None;
    os_file_close(file);
    end_scratch(scratch);
}

static void
serialize_year(Year* year){
    ScratchArena scratch = begin_scratch();
    Arena* arena = pm->data_arena;

    String8 filename = str8_fmt(scratch.arena, "%04d.b", year->number);
    if(year->total_transactions){

        MonthInfo* month;
        for(s32 m_idx=0; m_idx < Month_Count; ++m_idx){
            month = year->months + m_idx;
            arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "#month_m%i\n", m_idx);
            arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "muted=%i\n", month->muted);

            Transaction* t = month->transactions;
            for(s32 t_idx = 0; t_idx < month->transactions_count; ++t_idx){
                t = t->next;
                arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at,
                                      "date=%s amount=%s description=%s\x1B selection=%s\x1B muted=%i\n",
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

            String8Node* str8_node = {0};
            str8_node = str8_split(scratch.arena, word, '=');
            // TODO WRONG FIXME(rr): budget needs to change to a char array
            str8_copy(&pm->budget, &str8_node->prev->str);
            //copy_str8_to_char(pm->budget, str8_node->prev->str, TRANS_DESCRIPTION_SIZE);
        }
        else if(bps == BudgetParsingState_Category){
            Category* category = (Category*)pool_next(pm->category_pool);
            dll_push_back(pm->month_categories, category);
            category->rows = (Row*)pool_next(pm->row_pool);
            dll_clear(category->rows);
            ++pm->categories_count;

            while(line.size){
                String8 word = str8_eat_word(&line);

                String8Node* str8_node = {0};
                if(str8_contains(word, str8_literal("name"))){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_to_char(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    str8_node = str8_split(scratch.arena, word, '=');
                    if(str8_compare(str8_node->prev->str, str8_node->next->str)){
                        copy_str8_to_char(category->name, str8_literal("\0"), TRANS_DESCRIPTION_SIZE);
                    }
                    else{
                        copy_str8_to_char(category->name, str8_node->prev->str, TRANS_DESCRIPTION_SIZE);
                    }
                }
                else if(str8_contains(word, str8_literal("draw_rows"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    category->draw_rows = atoi((char*)str8_node->prev->str.str);
                }
                else if(str8_contains(word, str8_literal("muted"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    category->muted = atoi((char*)str8_node->prev->str.str);
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

                String8Node* str8_node = {0};
                if(str8_contains(word, str8_literal("name"))){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_to_char(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    str8_node = str8_split(scratch.arena, word, '=');
                    if(str8_compare(str8_node->prev->str, str8_node->next->str)){
                        copy_str8_to_char(row->name, str8_literal("\0"), TRANS_DESCRIPTION_SIZE);
                    }
                    else{
                        copy_str8_to_char(row->name, str8_node->prev->str, TRANS_DESCRIPTION_SIZE);
                    }
                }
                else if(str8_contains(word, str8_literal("planned"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    copy_str8_to_char(row->planned, str8_node->prev->str, TRANS_DESCRIPTION_SIZE);
                }
                else if(str8_contains(word, str8_literal("muted"))){
                    str8_node = str8_split(scratch.arena, word, '=');
                    row->muted = atoi((char*)str8_node->prev->str.str);
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
    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "budget=%s\n", pm->budget.str);

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

Texture gear_texture;
ImTextureID gear_texture_id;
Texture green_box_texture;
ImTextureID green_box_texture_id;
Texture red_box_texture;
ImTextureID red_box_texture_id;
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

#endif
