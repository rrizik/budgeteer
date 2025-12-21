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
#include "clock.h"

#include "input.h"
#include "window.h"
#include "bitmap.h"
#include "d3d11_init.h"
#include <time.h>

#include "clock.cpp"
#include "input.cpp"
#include "bitmap.cpp"
#include "d3d11_init.cpp"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#include "tinyfiledialogs/tinyfiledialogs.h"
#include "meta.h"

bool do_once = false;
static String8 build_path;
static String8 saves_path;

static String8 fmt;
static RGBA RED;
static RGBA GREEN;

static ImFont* my_font12;
static ImFont* my_font20;
static char icon_lookup[] = {' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y'};
typedef enum Icon{
    Icon_None,

    Icon_UpArrow,
    Icon_DownArrow,
    Icon_LeftArrow,
    Icon_RightArrow,

    Icon_DoubleLeftArrow,
    Icon_DoubleRightArrow,

    Icon_Collapse,
    Icon_Expand,

    Icon_CheckSquare,
    Icon_Check,
    Icon_XSquare,
    Icon_X,
    Icon_Square,
    Icon_SquareRounded,

    Icon_MagA,
    Icon_MagB,

    Icon_Refresh,

    Icon_Show,
    Icon_Hide,

    Icon_Unlocked,
    Icon_Locked,

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

#define CATEGORY_NAME_SIZE 128
#define CATEGORY_PLANNED_SIZE 128
typedef struct Category{
    Category* next;
    Category* prev;

    char name[CATEGORY_NAME_SIZE];
    char planned[CATEGORY_PLANNED_SIZE];
    f32 spent;
    f32 diff;

    bool muted;
} Category;

typedef struct CategoryGroup{
    CategoryGroup* next;
    CategoryGroup* prev;
    Category* categories;

    char name[CATEGORY_NAME_SIZE];
    f32 planned; // todo(rr): this is considered a kind of cache, don't do this unless you actually need to
    f32 spent;
    f32 diff;

    u32 category_count;
    bool draw_categories;
    bool muted;
} CategoryGroup;

global u64 merchant_id = 0;
#define MERCH_DESCRIPTION_SIZE 1024
#define MERCH_CATEGORY_SIZE 128
typedef struct Merchant{
    Merchant* next;
    u64 id;

    char description[MERCH_DESCRIPTION_SIZE];
    char category[MERCH_CATEGORY_SIZE];

    bool hidden;
} Merchant;

#define TRANS_DATE_SIZE 128
#define TRANS_AMOUNT_SIZE 128
#define TRANS_DESCRIPTION_SIZE 1024
#define TRANS_CATEGORY_SIZE 128
typedef struct Transaction{
    Transaction* next;
    Transaction* prev;

    char date[TRANS_DATE_SIZE];
    char amount[TRANS_AMOUNT_SIZE];
    char description[TRANS_DESCRIPTION_SIZE];
    char category[TRANS_CATEGORY_SIZE];

    u64 merchant_id;

    bool muted;
    bool hidden;
    bool locked;
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
    u32 transaction_visible_count;

    Totals totals;
    bool muted;
    bool locked;
    bool hidden;
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
    s32  date_format_kind;
} CSV_Profile;

#define CATEGORY_SIZE 128
#define MAX_CATEGORY_GROUP_COUNT 256
#define MAX_CATEGORY_COUNT 2048
#define MAX_TRANSACTION_COUNT 32768
#define MAX_PROFILE_COUNT 32
#define MAX_CATEGORY_LIST_COUNT 1024
typedef struct PermanentMemory{
    // memory
    Arena arena;
    PoolArena* category_group_pool;
    PoolArena* category_pool;
    PoolArena* transaction_pool;
    PoolArena* csv_profile_pool;
    Arena* data_arena;

    // category_group/categories/months/transactions
    CategoryGroup* annual_category_groups;
    CategoryGroup* biannual_category_groups;
    CategoryGroup* quarter_category_groups;
    CategoryGroup* month_category_groups;

    // Years are setup where 0 == current year, -1 == current year - 1, 1 == current year + 1
    Year years[MAX_YEAR_COUNT];
    Year* year;
    s32 year_idx;
    s32 transaction_year;
    s32 current_year;
    Merchant* merchants;

    // todo(rr): Review totals for all.
    u32 total_categories_count;
    u32 category_groups_count;
    u32 total_transaction_count;
    u32 quarter_category_groups_count;
    u32 biannual_category_groups_count;
    u32 annual_category_groups_count;

    // for creating the transaction selection list
    // todo(rr): just turn this into a list
	String8* category_list;
    u32 category_list_count;

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

static bool show_all_merchants = true;
static bool apply_hidden_transactions = true;
static bool apply_new_category = false;
static String8 empty_category = str8_literal(" \0");
static String8 empty_deserialized_category = str8_literal(" \x1B");
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

static f32 category_count_column_start = collapse_column_start + collapse_column_width;
static f32 category_count_column_width = 25.0f;

static f32 category_group_column_start = category_count_column_start + category_count_column_width;
static f32 category_group_column_width = 100.0f;

static f32 planned_column_start = category_group_column_start + category_group_column_width + 10.0f;
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
static f32 category_group_select_column_start = description_column_start + description_column_width + 10;
static f32 category_group_select_column_width = 160;
static f32 plus_expense_column_start = category_group_select_column_start + category_group_select_column_width + 10;
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
            f64 left = strtod((char*)current->amount, 0);
            f64 right = strtod((char*)next->amount, 0);

            if(ascending && left > right){
                dll_swap(current, next, Transaction);
                Transaction* tmp = current;
                current = next;
                next = tmp;
                no_swaps = true;
            }
            else if(!ascending && right > left){
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
dll_bubble_sort_description_merch(Merchant** head, bool ascending=false){
    if(!head) return;

    bool swapped;
    do {
        swapped = false;
        Merchant** node = head;
        while(*node && (*node)->next){
            Merchant* a = *node;
            Merchant* b = a->next;

            s32 result = strcmp(a->description, b->description);
            bool out_of_order = ascending ? (result > 0) : (result < 0);
            if(out_of_order){
                a->next = b->next;
                b->next = a;
                *node = b;

                swapped = true;
            }
            node = &(*node)->next;
        }
    } while(swapped);
}

static void
dll_bubble_sort_category_merch(Merchant** head, bool ascending=false){
    if(!head) return;

    bool swapped;
    do {
        swapped = false;
        Merchant** node = head;
        while(*node && (*node)->next){
            Merchant* a = *node;
            Merchant* b = a->next;

            s32 result = strcmp(a->category, b->category);
            bool out_of_order = ascending ? (result > 0) : (result < 0);
            if(out_of_order){
                a->next = b->next;
                b->next = a;
                *node = b;

                swapped = true;
            }
            node = &(*node)->next;
        }
    } while(swapped);
}

static void
dll_bubble_sort_category(Transaction* sentinel, bool ascending=false){
    if(sentinel->next == sentinel || sentinel->prev == sentinel){
        return;
    }

    bool no_swaps = true;
    while(no_swaps){
        no_swaps = false;
        Transaction* current = sentinel->next;
        Transaction* next = sentinel->next->next;
        while(next != sentinel){
            s32 result = strcmp(current->category, next->category);

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
dll_bubble_sort_description(Transaction* sentinel, bool ascending=false){
    if(sentinel->next == sentinel || sentinel->prev == sentinel){
        return;
    }

    bool no_swaps = true;
    while(no_swaps){
        no_swaps = false;
        Transaction* current = sentinel->next;
        Transaction* next = sentinel->next->next;
        while(next != sentinel){
            s32 result = strcmp(current->description, next->description);

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

static u32
str8_copy_to_char(char* dst, String8 src, u32 max_size){
    if(!dst || src.count == 0) return(0);

    s32 wrote_count = MIN(max_size, src.size);
    for(s32 i=0; i < wrote_count; ++i){
        dst[i] = src.str[i];
    }
    dst[wrote_count] = 0;

    return(wrote_count);
    //for(s32 i=0; i < wrote_count; ++i){
    //    c[i] = src.str[i];
    //}
    //c[src.size] = '\0';

    // todo(rr): do I need this?
    //if(c[src.size - 1] == '\n' || c[src.size - 1] == '\x1B'){
    //    c[src.size - 1] = '\0';
    //}
}

// todo: Get rid of this.
// note important: this is dangeours if dst doesn't have enough memory
static void
str8_copy(String8* dst, String8* src){
    s32 count = 0;
    while(count != src->count){
        dst->data[count] = src->data[count];
        count++;
    }
    if(count > 0){
        dst->data[count] = '\0';
        dst->count = count;
    }
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
    BudgetParsingState_CategoryGroup,
    BudgetParsingState_Category,

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
    ConfigParsingState_Merchants,

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
global bool config_year_idx_deserialized = false;

global bool debug_show_window = false;
global bool debug_size_window = true;
global bool debug_show_scratch = false;
global bool debug_show_pm_memory = false;
global bool debug_show_tm_memory = false;

global s32 lock_button_width = 0.0f;

//global String8 domain_names[] = {
//    .COM
//        .NET
//        .ORG
//        .CO
//        .BILL
//        .BIZ
//        .IO
//
//}

global String8 white_list[] = {
    str8_lit("F45"),
};

global String8 processor_prefix[] = {
   str8_lit("SQ"),
   str8_lit("SQ*"),
   str8_lit("VC*"),
   str8_lit("PY*"),
   str8_lit("TST*"),
   str8_lit("WL*"),
   str8_lit("WP*"),
   str8_lit("WT*"),
   str8_lit("AA*"),
   str8_lit("VC"),
};

global String8 noise_tokens[] = {
    str8_lit("ROUND"),
    str8_lit("UP"),
    str8_lit("TRANSFER"),
    str8_lit("DIR"),
    str8_lit("DEP"),
    str8_lit("WEB"),
    str8_lit("PMTS"),
    str8_lit("FUNDS"),
    str8_lit("TRAN"),
    str8_lit("WITHDRAWAL"),
    str8_lit("DIVIDEND"),
    str8_lit("FEE"),
    str8_lit("SERVICE"),
    str8_lit("TO"),
    str8_lit("CTR"),
    str8_lit("<###>"),

    str8_lit("PAYMENT"),
    str8_lit("ACCTVERIFY"),
    str8_lit("INT"),
    str8_lit("TST"),
    str8_lit("NNT"),
    str8_lit("NTTA"),
    str8_lit("HCTRA"),
    str8_lit("ATGPAY"),

    str8_lit("LLC"),
    str8_lit("INC"),
    str8_lit("CO"),
    str8_lit("COMPANY"),
    str8_lit("SERVICES"),
    str8_lit("SERVICE"),
    str8_lit("STORE"),
    str8_lit("OFFIC"),
    str8_lit("ONLINE"),

    str8_lit("ST"),
    str8_lit("RD"),
    str8_lit("DR"),
    str8_lit("HWY"),
    str8_lit("AVE"),
    str8_lit("AVENUE"),
    str8_lit("PKWY"),
    str8_lit("PLAZA"),
    str8_lit("SUITE"),
    str8_lit("STE"),
    str8_lit("UNIT"),
    str8_lit("FLOOR"),
    str8_lit("N"),
    str8_lit("S"),
    str8_lit("E"),
    str8_lit("W"),

    str8_lit("STATE"),
    str8_lit("HIGHWAY"),
    str8_lit("LOS"),
    str8_lit("BLVD"),
    str8_lit("BUDA"),
    str8_lit("AUSTIN"),
    str8_lit("PLANO"),
    str8_lit("RIOS"),
    str8_lit("STPLANO"),
    str8_lit("PARK"),
    str8_lit("DALLAS"),
    str8_lit("MCKINNEY"),
    str8_lit("RICHARDSON"),
    str8_lit("MURPHY"),
    str8_lit("ALLEN"),
    str8_lit("FRISCO"),
    str8_lit("TEMPLE"),
    str8_lit("WYLIE"),
    str8_lit("CARROLLTON"),
    str8_lit("BELLEVUE"),
    str8_lit("SEATTLE"),
    str8_lit("HOLLYWOOD"),
    str8_lit("MIAMI"),
    str8_lit("BOSTON"),
    str8_lit("HOUSTON"),
    str8_lit("BOZEMAN"),
    str8_lit("CHICAGO"),
    str8_lit("NASHVILLE"),
    str8_lit("PHOENIX"),
};

global String8 country_states[] = {
    str8_lit("ABCAN"),
    str8_lit("AUS"),
    str8_lit("AZUS"),
    str8_lit("BCCAN"),
    str8_lit("CAUS"),
    str8_lit("COUS"),
    str8_lit("CYCY"),
    str8_lit("DEU"),
    str8_lit("DEUS"),
    str8_lit("FLUS"),
    str8_lit("FRA"),
    str8_lit("GAUS"),
    str8_lit("GBR"),
    str8_lit("HKHK"),
    str8_lit("ILUS"),
    str8_lit("MAUS"),
    str8_lit("MTUS"),
    str8_lit("MXMX"),
    str8_lit("NCUS"),
    str8_lit("NHUS"),
    str8_lit("NMUS"),
    str8_lit("NJUS"),
    str8_lit("NSCAN"),
    str8_lit("NYUS"),
    str8_lit("NZL"),
    str8_lit("OKUS"),
    str8_lit("OHUS"),
    str8_lit("ONCAN"),
    str8_lit("PAUS"),
    str8_lit("TNUS"),
    str8_lit("TXUS"),
    str8_lit("TXUSA"),
    str8_lit("UTUS"),
    str8_lit("USA"),
    str8_lit("WAUS"),
};

static bool is_white_list(String8 string){
    bool result = false;
    s32 count = array_count(white_list);
    for(s32 idx = 0; idx < count; ++idx){
        if(str8_compare(string, white_list[idx])){
            result = true;
            break;
        }
    }
    return(result);
}

static void
generate_merchants(){
    ScratchArena scratch = begin_scratch(0);
    Arena* arena = pm->data_arena;

    for(s32 year_idx=0; year_idx < MAX_YEAR_COUNT; ++year_idx){
        Year* year = pm->years + year_idx;
        for(s32 month_idx=0; month_idx < Month_Count; ++month_idx){
            MonthInfo* month = year->months + month_idx;
            for(Transaction* trans = month->transactions->next; trans != month->transactions; trans = trans->next){

                String8 string = {0};
                string.str = push_array(scratch.arena, u8, TRANS_DESCRIPTION_SIZE);
                u64 length = char_length(trans->description);

                memcpy(string.str, trans->description, length);
                string.count = length;

                str8_to_upper(&string);

                // Filter out disallowed characters.
                for(s32 i=0; i < string.count; ++i){
                    char byte = string.str[i];
                    if(!byte_is_alnum(byte) &&
                       !byte_is_space(byte) &&
                        byte != '/' &&
                        byte != '.' &&
                        byte != '-' &&
                        byte != '\''){

                        //print("%c\n", (char)byte);
                        string.str[i] = ' ';
                    }
                }

                // Trim all white space from start and end.
                while(str8_ends_with_byte(string, ' ')){
                    str8_trim_right(&string, 1);
                }
                while(str8_starts_with_byte(string, ' ')){
                    str8_trim_left(&string, 1);
                }

                // Trim multiple spaces down to 1.
                s32 space_count = 0;
                s32 write_idx = 0;
                for(s32 read_idx=0; read_idx < string.count; ++read_idx){
                    u8 byte = string.str[read_idx];

                    if(byte_is_space(byte)){
                        space_count++;
                    }
                    else{
                        space_count = 0;
                    }

                    if(space_count <= 1){
                        string.str[write_idx++] = byte;
                    }
                }
                string.count = write_idx;

                // alternative to the above code
                //String8List parts = str8_split(scratch.arena, trans_description, ' ', 0);
                //String8Join join = {0};
                //join.mid = str8_lit(" ");
                //String8 result = str8_join(scratch.arena, parts, join);

                // replace numbers with <###>
                char tmp[1024] = {0};
                s32 digit_count = 0;
                write_idx = 0;
                for(s32 read_idx = 0; read_idx < string.count; ++read_idx){
                    u8 byte = string.str[read_idx];

                    if(byte_is_digit(byte)){
                        digit_count++;
                    }
                    else{
                        if(digit_count >= 3){
                            tmp[write_idx++] = '<';
                            tmp[write_idx++] = '#';
                            tmp[write_idx++] = '#';
                            tmp[write_idx++] = '#';
                            tmp[write_idx++] = '>';
                            tmp[write_idx++] = (char)byte;
                        }
                        else if(digit_count != 0){
                            while(digit_count != 0){
                                tmp[write_idx++] = (char)string.str[read_idx - digit_count];
                                digit_count--;
                            }
                            tmp[write_idx++] = (char)byte;
                        }
                        else{
                            tmp[write_idx++] = (char)byte;
                        }
                        digit_count = 0;
                    }
                }
                if(digit_count >= 3){
                    tmp[write_idx++] = '<';
                    tmp[write_idx++] = 'N';
                    tmp[write_idx++] = 'U';
                    tmp[write_idx++] = 'M';
                    tmp[write_idx++] = '>';
                }
                string = str8(tmp, write_idx);

                // tokenize the string
                String8List parts = str8_split(scratch.arena, string, ' ', 0);

                // truncate on country state code
                s32 count = array_count(country_states);
                for(String8Node* node = parts.first; node != 0; ){
                    String8Node* next = node->next;
                    for(s32 idx = 0; idx < count; idx++){
                        String8 country_state = country_states[idx];
                        if(str8_compare(node->string, country_state)){
                            dll_remove(&parts, node);
                            break;
                        }
                    }
                    node = next;
                }

                // remove noise
                count = array_count(noise_tokens);
                for(String8Node* node = parts.first; node != 0; ){
                    String8Node* next = node->next;

                    for(s32 idx = 0; idx < count; idx++){
                        String8 token = noise_tokens[idx];
                        if(str8_compare(node->string, token)){
                            dll_remove(&parts, node);
                            break;
                        }
                    }

                    node = next;
                }

                // trim domains
                for(String8Node* node = parts.first; node != 0; ){
                    String8Node* next = node->next;

                    String8 test1 = str8_lit("WWW.");
                    if(str8_starts_with(node->string, test1)){
                        dll_remove(&parts, node);
                    }

                    String8 test2 = str8_lit(".COM");
                    if(str8_contains(node->string, test2)){
                        s32 idx = (s32)str8_index_from_left(node->string, test2);
                        str8_trim_right(&node->string, node->string.count - idx);
                    }

                    node = next;
                }


                for(String8Node* node = parts.first; node != 0; node = node->next){
                    if(is_white_list(node->string)){
                        continue;
                    }

                    s32 digit_count = 0;
                    if(node->string.count == 2){
                        if(byte_is_alpha(node->string.str[0]) &&
                           byte_is_digit(node->string.str[node->string.count - 1])){
                            digit_count = 1;
                        }
                    }
                    else if(node->string.count > 2){
                        if(byte_is_alpha(node->string.str[0]) &&
                           byte_is_digit(node->string.str[node->string.count - 1]) &&
                           byte_is_digit(node->string.str[node->string.count - 2])){
                            digit_count = 2;
                        }
                    }

                    if(digit_count != 0){
                       str8_trim_right(&node->string, digit_count);
                    }
                }

                // Remove digis only or <###>.
                for(String8Node* node = parts.first; node != 0; ){
                    String8Node* next = node->next;

                    if(str8_is_digit(node->string)){
                        dll_remove(&parts, node);
                    }
                    else if(str8_compare(node->string, str8_lit("<###>")) ||
                            str8_compare(node->string, str8_lit(""))){
                        dll_remove(&parts, node);
                    }

                    node = next;
                }

                // Remove < 2 count.
                for(String8Node* node = parts.first; node != 0; ){
                    String8Node* next = node->next;
                    if(!is_white_list(node->string)){
                        if(node->string.count < 2){
                            dll_remove(&parts, node);
                        }
                    }
                    node = next;
                }

                // Remove preprocessor prefix from first node only.
                if(parts.node_count){
                    count = array_count(processor_prefix);
                    for(s32 idx=0; idx < count; ++idx){
                        if(str8_compare(parts.first->string, processor_prefix[idx])){
                            dll_pop_front(&parts);
                        }
                    }
                }

                for(String8Node* node = parts.first; node != 0; ){
                    String8Node* next = node->next;
                   if( str8_contains(node->string, str8_lit("<###>")) &&
                       !str8_contains_alpha(node->string)){
                        dll_remove(&parts, node);
                    }
                    node = next;
                }

                //// trim <###>
                //for(String8Node* node = parts.first; node != 0; node = node->next){
                //    String8 test = str8_lit("<###>");
                //    if(str8_ends_with(node->string, test)){
                //        str8_trim_right(&node->string, 5);
                //    }

                //    if(str8_starts_with(node->string, test)){
                //        str8_trim_left(&node->string, 5);
                //    }

                //    if(node->string.count <= 2){
                //        String8Node* remove_node = node;
                //        node = node->prev;
                //        dll_remove(&parts, remove_node);
                //    }
                //}

                String8Join join = {0};
                join.mid = str8_lit(" ");
                string = str8_join(scratch.arena, &parts, &join);
                if(string.count == 0){
                    string = str8_lit("UNKNOWN");
                }
                arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "%.*s\n", (int)string.count, string.str);












                //String8Join join = {0};
                //join.mid = str8_lit(" ");
                //string = str8_join(scratch.arena, parts, join);

                //s32 a = 1;

                //bool found = false;
                //for(Merchant* m = pm->merchants; m != 0; m = m->next){
                //    String8 m_description = str8_cstring(m->description);
                //    if(str8_compare(m_description, trans_description)){
                //        trans->hidden = m->hidden;
                //        if(!m->hidden){
                //            month->transaction_visible_count++;
                //        }
                //        break;
                //    }
                //}
                //if(!found){
                //    Merchant* merch = push_struct(&pm->arena, Merchant);
                //    sll_push_front(pm->merchants, merch);

                //    str8_copy_to_char(merch->description, description_str, MERCH_DESCRIPTION_SIZE);
                //    str8_copy_to_char(merch->category, empty_category, empty_category.count);
                //    merch->id = merchant_id++;

                //    trans->merchant_id = merch->id;
                //}
            }
        }
    }

    String8 full_path = str8_path_append(scratch.arena, saves_path, str8_literal("test.b"));
    File file = os_file_open(full_path, GENERIC_WRITE, CREATE_ALWAYS);
    if(file.handle != INVALID_HANDLE_VALUE){
        os_file_write(file, arena->base, arena->at);
    }
    os_file_close(file);

    end_scratch(scratch);
    arena_free(pm->data_arena);

}

static void
parse_day_month_year(Transaction* trans){
    ScratchArena scratch = begin_scratch();

    CSV_Profile* profile = pm->csv_profile;
    char delimiter = date_delimiters[profile->date_format_kind];
    String8 transaction_date = str8_cstring(trans->date);
    String8List date_parts = str8_split(scratch.arena, transaction_date, delimiter, 0);

    //String8 dd, mm, yyyy;
    switch(profile->date_format_kind){
        case Format_Kind_MMDDYYYY_Dash:
        case Format_Kind_MMDDYYYY_Forward_Slash:{
            str8_copy(&pm->mm,   &date_parts.first->string);
            str8_copy(&pm->dd,   &date_parts.first->next->string);
            str8_copy(&pm->yyyy, &date_parts.first->next->next->string);
        } break;
        case Format_Kind_DDMMYYYY_Dash:
        case Format_Kind_DDMMYYYY_Forward_Slash:{
            str8_copy(&pm->dd,   &date_parts.first->string);
            str8_copy(&pm->mm,   &date_parts.first->next->string);
            str8_copy(&pm->yyyy, &date_parts.first->next->next->string);
        } break;
        case Format_Kind_YYYYMMDD_Dash:
        case Format_Kind_YYYYMMDD_Forward_Slash:{
            str8_copy(&pm->yyyy, &date_parts.first->string);
            str8_copy(&pm->mm,   &date_parts.first->next->string);
            str8_copy(&pm->dd,   &date_parts.first->next->next->string);
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

    // Set current year based on the transaction that is deserialized
    s32 year_idx = (MAX_YEAR_COUNT/2) + (pm->yyyy_s32 - pm->current_year);
    year_idx = clamp_s32(year_idx, 0, MAX_YEAR_COUNT);
    pm->year = pm->years + year_idx;

    end_scratch(scratch);
}

static bool
test_date_against_format(String8 date){

    CSV_Profile* profile = pm->csv_profile;
    char delimiter = date_delimiters[profile->date_format_kind];

    if(!str8_contains_byte(date, delimiter)){
        return(false);
    }

    ScratchArena scratch = begin_scratch();
    String8List date_parts = str8_split(scratch.arena, date, delimiter, 0);

    String8 dd, mm, yyyy;
    switch(profile->date_format_kind){
        case Format_Kind_MMDDYYYY_Dash:
        case Format_Kind_MMDDYYYY_Forward_Slash:{
            mm   = date_parts.first->string;
            dd   = date_parts.first->next->string;
            yyyy = date_parts.first->next->next->string;
        } break;
        case Format_Kind_DDMMYYYY_Dash:
        case Format_Kind_DDMMYYYY_Forward_Slash:{
            dd   = date_parts.first->string;
            mm   = date_parts.first->next->string;
            yyyy = date_parts.first->next->next->string;
        } break;
        case Format_Kind_YYYYMMDD_Dash:
        case Format_Kind_YYYYMMDD_Forward_Slash:{
            yyyy = date_parts.first->string;
            mm   = date_parts.first->next->string;
            dd   = date_parts.first->next->next->string;
        } break;
    }

    if(dd.count > 2 || mm.count > 2 || yyyy.count != 4){
        end_scratch(scratch);
        return(false);
    }

    s32 date_day_value   = atoi((char*)dd.str);
    s32 date_month_value = atoi((char*)mm.str);
    s32 date_year_value  = atoi((char*)yyyy.str);
    if(date_day_value > 31 || date_month_value > 12){
        end_scratch(scratch);
        return(false);
    }

    end_scratch(scratch);
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

    // todo: what is this for?
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

            String8 description_str = {0};
            bool date_parsed = false;
            u32 count = 0;
            String8 word;
            while(line.size){
                word = str8_eat_word_csv(&line);
                str8_strip_newline(&word);
                str8_strip_quotes(&word);

                if(count == date_idx){
                    str8_copy_to_char(trans->date, word, TRANS_DATE_SIZE);
                    date_parsed = true;
                }
                else if(count == amount_idx){
                    if(str8_starts_with(word, str8_literal("-"))){
                        str8_advance(&word, 1);
                    }
                    str8_copy_to_char(trans->amount, word, TRANS_AMOUNT_SIZE);
                }
                else if(count == desc_idx){
                    String8 view = word;
                    str8_strip_quotes(&view);
                    str8_copy_to_char(trans->description, view, TRANS_DESCRIPTION_SIZE);
                    description_str = str8(view.str, view.count);
                    str8_to_upper(&description_str);
                }

                ++count;
            }
            str8_copy_to_char(trans->category, empty_category, empty_category.count);

            // speed: this can be hashed later if its an issue.
            bool found = false;
            for(Merchant* merch = pm->merchants; merch != 0; merch = merch->next){
                String8 merch_description = str8_cstring(merch->description);
                if(str8_compare(merch_description, description_str)){
                    trans->merchant_id = merch->id;
                    found = true;
                    break;
                }
            }
            if(!found){
                Merchant* merch = push_struct(&pm->arena, Merchant);
                sll_push_front(pm->merchants, merch);

                str8_copy_to_char(merch->description, description_str, MERCH_DESCRIPTION_SIZE);
                str8_copy_to_char(merch->category, empty_category, empty_category.count);
                //merch->description.str = push_array(&pm->arena, u8, TRANS_DESCRIPTION_SIZE);
                //merch->category.str = push_array(&pm->arena, u8, TRANS_CATEGORY_SIZE);
                merch->id = merchant_id++;

                //memcpy(merch->description.str, description_str.str, description_str.count);
                //merch->description.count = description_str.count;
                //merch->description.str[merch->description.count] = '\0';

                //memcpy(merch->category.str, empty_category.str, empty_category.count);
                //merch->category.count = empty_category.count;
                //merch->category.str[merch->category.count] = '\0';

                trans->merchant_id = merch->id;
            }

            // after we pull all transaction info, use the date to find the correct year and month
            if(pm->date_format_found && date_parsed){
                parse_day_month_year(trans);

                year = pm->year;
                month = pm->year->months + pm->mm_s32 - 1;

                String8 result = str8_concat(scratch.arena, pm->yyyy, str8_literal("-"));
                result = str8_concat(scratch.arena, result, pm->mm);
                result = str8_concat(scratch.arena, result, str8_literal("-"));
                result = str8_concat(scratch.arena, result, pm->dd);
                str8_copy_to_char(trans->date, result, TRANS_DESCRIPTION_SIZE);
            }

            // we still want to add the transaction even if the date wasnt found, just for the year selected
            if(year){
                dll_push_back_old(month->transactions, trans);
                ++month->transaction_count;
                ++year->transaction_count;
            }
            else{
                pool_free(pm->transaction_pool, trans);
            }
            //todo(rr): STATUSBAR ERROR
        }
    }

    apply_new_category = true;
    apply_hidden_transactions = true;
    pm->total_transaction_count += year->transaction_count;
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
            else if(str8_compare(line, str8_literal("#merchants\n"))){
                cps = ConfigParsingState_Merchants;
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
                    String8List str8_node = str8_split(scratch.arena, word, '=', 0);
                    String8 key = str8_node.first->string;
                    String8 value = str8_node.last->string;
                    if(str8_ends_with_byte(value, '\x1B')){
                        str8_trim_right(&value, 1);
                    }

                    if(str8_contains(key, str8_literal("csv_profile_idx"))){
                        pm->csv_profile_idx = atoi((char*)value.str);
                    }
                }
            }
        }
        else if(cps == ConfigParsingState_CSV_Profile){

            CSV_Profile* profile = (CSV_Profile*)pool_next(pm->csv_profile_pool);
            dll_push_back_old(pm->csv_profiles, profile);
            pm->csv_profile_count++;

            while(line.size){
                String8 word = str8_eat_word(&line);
                if(word.count){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_word_to_byte(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    String8List str8_node = str8_split(scratch.arena, word, '=', 0);
                    String8 key = str8_node.first->string;
                    String8 value = str8_node.last->string;
                    if(str8_ends_with_byte(value, '\x1B')){
                        str8_trim_right(&value, 1);
                    }

                    if(str8_compare(key, str8_literal("name"))){
                        str8_copy_to_char(profile->name, value, PROFILE_NAME_SIZE);
                    }
                    else if(str8_compare(key, str8_literal("date"))){
                        str8_copy_to_char(profile->date, value, PROFILE_DATE_SIZE);
                    }
                    else if(str8_compare(key, str8_literal("amount"))){
                        str8_copy_to_char(profile->amount, value, PROFILE_AMOUNT_SIZE);
                    }
                    else if(str8_compare(key, str8_literal("description"))){
                        str8_copy_to_char(profile->description, value, PROFILE_DESCRIPTION_SIZE);
                    }
                    else if(str8_compare(key, str8_literal("format"))){
                        str8_copy_to_char(profile->date_format, value, PROFILE_DATE_FORMAT_SIZE);
                    }
                    else if(str8_compare(key, str8_literal("format_kind"))){
                        profile->date_format_kind = atoi((char*)value.str);
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
                    String8List str8_node = str8_split(scratch.arena, word, '=', 0);
                    String8 key = str8_node.first->string;
                    String8 value = str8_node.last->string;
                    if(str8_ends_with_byte(value, '\x1B')){
                        str8_trim_right(&value, 1);
                    }

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
                    String8List str8_node = str8_split(scratch.arena, word, '=', 0);
                    String8 key = str8_node.first->string;
                    String8 value = str8_node.last->string;
                    if(str8_ends_with_byte(value, '\x1B')){
                        str8_trim_right(&value, 1);
                    }

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
                    String8List str8_node = str8_split(scratch.arena, word, '=', 0);
                    String8 key = str8_node.first->string;
                    String8 value = str8_node.last->string;
                    if(str8_ends_with_byte(value, '\x1B')){
                        str8_trim_right(&value, 1);
                    }

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
                    String8List str8_node = str8_split(scratch.arena, word, '=', 0);
                    String8 key = str8_node.first->string;
                    String8 value = str8_node.last->string;
                    if(str8_ends_with_byte(value, '\x1B')){
                        str8_trim_right(&value, 1);
                    }

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
                    String8List str8_node = str8_split(scratch.arena, word, '=', 0);
                    String8 key = str8_node.first->string;
                    String8 value = str8_node.last->string;
                    if(str8_ends_with_byte(value, '\x1B')){
                        str8_trim_right(&value, 1);
                    }

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
                    String8List str8_node = str8_split(scratch.arena, word, '=', 0);
                    String8 key = str8_node.first->string;
                    String8 value = str8_node.last->string;
                    if(str8_ends_with_byte(value, '\x1B')){
                        str8_trim_right(&value, 1);
                    }

                    if(str8_compare(key, str8_literal("year_idx"))){
                        pm->year_idx = atoi((char*)value.str);
                        config_year_idx_deserialized = true;
                    }
                    else if(str8_compare(key, str8_literal("month_tab_idx"))){
                        pm->month_tab_idx = atoi((char*)value.str);
                        pm->month_tab_flags[pm->month_tab_idx] = ImGuiTabItemFlags_SetSelected;
                    }
                }
            }
        }
        else if(cps == ConfigParsingState_Merchants){
            Merchant* merch = push_struct(&pm->arena, Merchant);
            sll_push_front(pm->merchants, merch);
            str8_copy_to_char(merch->category, empty_category, empty_category.count);
            //merch->description.str = push_array(&pm->arena, u8, TRANS_DESCRIPTION_SIZE);
            //merch->category.str = push_array(&pm->arena, u8, TRANS_CATEGORY_SIZE);
            merch->id = merchant_id++;

            while(line.size){
                String8 word = str8_eat_word(&line);
                if(word.count){
                    if(!str8_contains_byte(word, '\x1B')){ // Todo: Check if I even need this if statement
                        u32 count = str8_extend_word_to_byte(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    String8List str8_node = str8_split(scratch.arena, word, '=', 0);
                    String8 key = str8_node.first->string;
                    String8 value = str8_node.last->string;
                    if(str8_ends_with_byte(value, '\x1B')){
                        str8_trim_right(&value, 1);
                    }

                    if(str8_compare(key, str8_literal("description"))){
                        str8_to_upper(&value); // todo: temporary until merchant stuff is done
                        str8_copy_to_char(merch->description, value, value.count);
                        //memcpy(merch->description.str, value.str, value.count);
                        //merch->description.count = value.count - 1;
                        //merch->description.str[merch->description.count] = '\0';
                    }
                    else if(str8_compare(key, str8_literal("category"))){
                        str8_copy_to_char(merch->category, value, value.count);
                        //memcpy(merch->category.str, value.str, value.count);
                        //if(str8_compare(value, empty_deserialized_category)){
                        //    merch->category.count = value.count;
                        //    merch->category.str[merch->category.count - 1] = '\0';
                        //}
                        //else{
                        //    merch->category.count = value.count - 1;
                        //    merch->category.str[merch->category.count] = '\0';
                        //}
                    }
                    else if(str8_compare(key, str8_literal("hidden"))){
                        merch->hidden = atoi((char*)value.str);
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
                "name=%s\x1B date=%s\x1B amount=%s\x1B description=%s\x1B format=%s\x1B format_kind=%i\n",
                profile->name, profile->date, profile->amount, profile->description, profile->date_format, profile->date_format_kind);
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

    // merchants
    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "#merchants\n");
    for(Merchant* merch = pm->merchants; merch != 0; merch = merch->next){
        arena->at += snprintf(
            (char*)arena->base + arena->at, arena->size - arena->at,
            "description=%s\x1B category=%s\x1B hidden=%i\n",
            merch->description, merch->category, merch->hidden
        );
    }

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

// Note: Deserializes year files based on year number. If the year file doesn't exist, return and go next.
static void
deserialize_year(Year* year){
    ScratchArena scratch = begin_scratch();
    String8 filename = str8_fmt(scratch.arena, "%04d.b", year->number);
    String8 full_path = str8_path_append(scratch.arena, saves_path, filename);

    // todo(rr): I think I prefer this check to encapsulate the function on the outside, just create the path and check it out side the function. This should be called if we know we can serialize, and then it doesn't hide the check.
    if(!os_path_exists(full_path)){
        end_scratch(scratch);
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
                //year->transaction_count += month->transaction_count;
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
                    String8List str8_node = str8_split(scratch.arena, word, '=', 0);
                    String8 key = str8_node.first->string;
                    String8 value = str8_node.last->string;
                    if(str8_ends_with_byte(value, '\x1B')){
                        str8_trim_right(&value, 1);
                    }

                    if(str8_compare(key, str8_literal("muted"))){
                        month->muted = atoi((char*)value.str);
                    }
                    else if(str8_compare(key, str8_literal("locked"))){
                        month->locked = atoi((char*)value.str);
                    }
                    else if(str8_compare(key, str8_literal("hidden"))){
                        month->hidden = atoi((char*)value.str);
                    }
                }
            }
            tps = TransactionParsingState_Transaction;
        }
        else if(tps == TransactionParsingState_Transaction){

            Transaction* trans;
            if(line.size){
                trans = (Transaction*)pool_next(pm->transaction_pool);
                dll_push_back_old(month->transactions, trans);
                month->transaction_count++;
                year->transaction_count++;
            }

            String8 description_str = {0};
            String8 category_str = {0};
            while(line.size){
                String8 word = str8_eat_word(&line);
                if(word.count){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_word_to_byte(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    String8List str8_node = str8_split(scratch.arena, word, '=', 0);
                    String8 key = str8_node.first->string;
                    String8 value = str8_node.last->string;
                    if(str8_ends_with_byte(value, '\x1B')){
                        str8_trim_right(&value, 1);
                    }

                    if(str8_compare(key, str8_literal("date"))){
                        str8_copy_to_char(trans->date, value, value.count);
                    }
                    else if(str8_compare(key, str8_literal("amount"))){
                        str8_copy_to_char(trans->amount, value, value.count);
                    }
                    else if(str8_compare(key, str8_literal("description"))){
                        str8_copy_to_char(trans->description, value, value.count);
                        description_str = str8(value.str, value.count);
                        str8_to_upper(&description_str);
                    }
                    else if(str8_compare(key, str8_literal("category"))){
                        str8_copy_to_char(trans->category, value, value.count);
                        category_str = str8(value.str, value.count);
                    }
                    else if(str8_compare(key, str8_literal("muted"))){
                        trans->muted = atoi((char*)value.str);
                    }
                    else if(str8_compare(key, str8_literal("hidden"))){
                        trans->hidden = atoi((char*)value.str);
                    }
                    else if(str8_compare(key, str8_literal("locked"))){
                        trans->locked = atoi((char*)value.str);
                    }
                    else if(str8_compare(key, str8_literal("merchant_id"))){
                        trans->merchant_id = strtoull((char*)value.str, 0, 10);
                    }
                }
            }

            // deserialize_year
            // todo: temporary, remove this once merchant stuff is done.
            //bool found = false;
            //for(Merchant* merch = pm->merchants; merch != 0; merch = merch->next){
            //    if(str8_compare(merch->description, description_str)){
            //        trans->merchant_id = merch->id;
            //        found = true;
            //        break;
            //    }
            //}
            //if(!found){
            //    Merchant* merch = push_struct(&pm->arena, Merchant);
            //    sll_push_front(pm->merchants, merch);

            //    merch->description.str = push_array(&pm->arena, u8, TRANS_DESCRIPTION_SIZE);
            //    merch->category.str = push_array(&pm->arena, u8, TRANS_CATEGORY_SIZE);
            //    merch->id = merchant_id++;

            //    memcpy(merch->description.str, description_str.str, description_str.count);
            //    merch->description.count = description_str.count;
            //    merch->description.str[merch->description.count] = '\0';

            //    memcpy(merch->category.str, empty_category.str, empty_category.count);
            //    merch->category.count = empty_category.count;
            //    merch->category.str[merch->category.count] = '\0';

            //    trans->merchant_id = merch->id;
            //}
        }
    }

    pm->total_transaction_count += year->transaction_count;
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
            arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "muted=%i\x1B locked=%i\x1B hidden=%i\n", month->muted, month->hidden, month->locked);

            Transaction* t = month->transactions;
            for(s32 t_idx = 0; t_idx < month->transaction_count; ++t_idx){
                t = t->next;
                arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at,
                                      "date=%s\x1B amount=%s\x1B description=%s\x1B category=%s\x1B muted=%i\x1B hidden=%i\x1B locked=%i\x1B merchant_id=%llu\n",
                                      t->date, t->amount, t->description, t->category, t->muted, t->hidden, t->locked, t->merchant_id);
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
        // always try and delete
        os_file_delete(saves_path, filename);
    }
    end_scratch(scratch);
}

static f32
best_fit(String8 a, String8 b){
    f32 result = 0.0f;

    s32 count = 0;
    for(s32 i=0; i < a.length; ++i){
        if(a.str[i] == b.str[i]){
            count++;
        }
    }
    result = 100 * ((f32)count / (f32)a.length);
    return(result);
}

typedef struct Fit{
    String8 str;
    f32 amount;
} Fit;

static void
test_merchants(void){
    Fit fit[1024];

    s32 count = 0;
    for(Merchant* first = pm->merchants; first != 0; first = first->next){
        print("%s\n", first->description);
        for(Merchant* second = pm->merchants; second != 0; second = second->next){
            String8 a = str8_cstring(first->description);
            String8 b = str8_cstring(second->description);
            f32 result = best_fit(a, b);
            if(result > 50.0f){
                Fit f = {
                    .str = b.str,
                    .amount = result,
                };
                fit[count++] = f;
            }
        }
        //if(count > 1){
        //    print("%i-------------------------\n", count);
        //    print("%s\n", first->description.str);
        //    print("-------------------------\n");
        //    for(s32 i=0; i < count; ++i){
        //        print("%s [%.2f]\n", fit[i].str.str, fit[i].amount);
        //    }
        //}
        count = 0;
    }
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
            else if(str8_compare(line, str8_literal("#category_group\n"))){
                bps = BudgetParsingState_CategoryGroup;
            }
        }
        else if(bps == BudgetParsingState_Budget){
            String8 word = str8_eat_word(&line);
            if(word.count){
                if(!str8_contains_byte(word, '\x1B')){
                    u32 count = str8_extend_word_to_byte(&word, '\x1B');
                    str8_advance(&line, count);
                }
                String8List str8_node = str8_split(scratch.arena, word, '=', 0);
                String8 key = str8_node.first->string;
                String8 value = str8_node.last->string;
                if(str8_ends_with_byte(value, '\x1B')){
                    str8_trim_right(&value, 1);
                }

                if(str8_compare(key, str8_literal("budget"))){
                    str8_copy_to_char(pm->budget, value, TRANS_DESCRIPTION_SIZE);
                }
            }
        }
        else if(bps == BudgetParsingState_CategoryGroup){
            CategoryGroup* category_group = (CategoryGroup*)pool_next(pm->category_group_pool);
            dll_push_back_old(pm->month_category_groups, category_group);
            category_group->categories = (Category*)pool_next(pm->category_pool);
            dll_clear(category_group->categories);
            ++pm->category_groups_count;

            while(line.size){
                String8 word = str8_eat_word(&line);
                if(word.count){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_word_to_byte(&word, '\x1B');
                        str8_advance(&line, count);
                    }

                    String8List str8_node = str8_split(scratch.arena, word, '=', 0);
                    String8 key = str8_node.first->string;
                    String8 value = str8_node.last->string;
                    if(str8_ends_with_byte(value, '\x1B')){
                        str8_trim_right(&value, 1);
                    }

                    if(str8_compare(key, str8_literal("name"))){
                        str8_copy_to_char(category_group->name, value, TRANS_DESCRIPTION_SIZE);
                    }
                    else if(str8_compare(key, str8_literal("draw_categories"))){
                        category_group->draw_categories = atoi((char*)value.str);
                    }
                    else if(str8_compare(key, str8_literal("muted"))){
                        category_group->muted = atoi((char*)value.str);
                    }
                }
            }
            bps = BudgetParsingState_Category;
        }
        else if(bps == BudgetParsingState_Category){
            CategoryGroup* category_group = pm->month_category_groups->prev;
            ++category_group->category_count;

            Category* category = (Category*)pool_next(pm->category_pool);
            dll_push_back_old(category_group->categories, category);
            ++pm->total_categories_count;

            while(line.size){
                String8 word = str8_eat_word(&line);
                if(word.count){
                    if(!str8_contains_byte(word, '\x1B')){
                        u32 count = str8_extend_word_to_byte(&word, '\x1B');
                        str8_advance(&line, count);
                    }
                    String8List str8_node = str8_split(scratch.arena, word, '=', 0);
                    String8 key = str8_node.first->string;
                    String8 value = str8_node.last->string;
                    if(str8_ends_with_byte(value, '\x1B')){
                        str8_trim_right(&value, 1);
                    }

                    if(str8_compare(key, str8_literal("name"))){
                        str8_copy_to_char(category->name, value, TRANS_DESCRIPTION_SIZE);
                    }
                    else if(str8_compare(key, str8_literal("planned"))){
                        str8_copy_to_char(category->planned, value, TRANS_DESCRIPTION_SIZE);
                    }
                    else if(str8_compare(key, str8_literal("muted"))){
                        category->muted = atoi((char*)value.str);
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
    CategoryGroup* cg = pm->month_category_groups;

    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "#budget\n");
    arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "budget=%i\n", atoi(pm->budget));

    for(s32 c_idx = 0; c_idx < pm->category_groups_count; ++c_idx){
        cg = cg->next;

        arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at, "#category_group\n");
        arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at,
                              "name=%s\x1B draw_categories=%i muted=%i\n", cg->name, cg->draw_categories, cg->muted);

        Category* c = cg->categories;
        for(s32 r_idx = 0; r_idx < cg->category_count; ++r_idx){
            c = c->next;
            arena->at += snprintf((char*)arena->base + arena->at, arena->size - arena->at,
                                  "\tname=%s\x1B planned=%s muted=%i\n", c->name, c->planned, c->muted);
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

f32 minus_padding = 8;
#endif
