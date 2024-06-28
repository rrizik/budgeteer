#ifndef INPUT_H
#define INPUT_H

// CLEANUP: everything that is commented out, I dont think needs to exist.
typedef enum KeyCode{
    KeyCode_None = 0,

    // INCOMPLETE: these need to be looked at again when we do mouse
    MOUSE_BUTTON_LEFT   = 1,
    MOUSE_BUTTON_RIGHT  = 2,
    MOUSE_BUTTON_MIDDLE = 3,

    //SL_MOUSE_BUTTON  = 1,
    //SR_MOUSE_BUTTON  = 2,
    //SLR_MOUSE_BUTTON = 3,
    //SM_MOUSE_BUTTON  = 16,

    KeyCode_BACKSPACE = 8,
    KeyCode_TAB       = 9,
    KeyCode_ENTER     = 13,
    KeyCode_ESCAPE    = 27,
    KeyCode_SPACEBAR  = 32,
    KeyCode_PAGE_UP   = 33,
    KeyCode_PAGE_DOWN = 34,
    KeyCode_END       = 35,
    KeyCode_HOME      = 36,
    //KeyCode_DELETE    = 46,

	//KeyCode_EXCLAMATION = 33,
	//KeyCode_QUOTATION =   34,
	//KeyCode_OCTOTHORP =   35,
	//KeyCode_DOLLAR =      36,
	KeyCode_LEFT =     37,
	KeyCode_UP =   38,
	KeyCode_RIGHT =  39,
	KeyCode_DOWN  = 40,
	//KeyCode_CLOSE_PARENTHESIS = 41,
	//KeyCode_ASTERISK =  42,
	//KeyCode_PLUS_SIGN = 43,
    KeyCode_INSERT    = 45,
	//KeyCode_COMMA =  44,
	//KeyCode_HYPHEN = 45,
	//KeyCode_PERIOD = 46,
	//KeyCode_SLASH =  47,
	KeyCode_ZERO  = 48,
	KeyCode_ONE   = 49,
	KeyCode_TWO   = 50,
	KeyCode_THREE = 51,
	KeyCode_FOUR  = 52,
	KeyCode_FIVE  = 53,
	KeyCode_SIX   = 54,
	KeyCode_SEVEN = 55,
	KeyCode_EIGHT = 56,
	KeyCode_NINE  = 57,
	//KeyCode_COLON = 58,
	//KeyCode_SEMICOLON = 59,
	KeyCode_LESS_THAN = 60,
	//KeyCode_EQUALS_TO = 61,
	KeyCode_GREATER_THAN = 62,
	KeyCode_QUESTION_MARK = 63,
	//KeyCode_AT_SIGN = 64,
	KeyCode_A = 65,
	KeyCode_B = 66,
	KeyCode_C = 67,
	KeyCode_D = 68,
	KeyCode_E = 69,
	KeyCode_F = 70,
	KeyCode_G = 71,
	KeyCode_H = 72,
	KeyCode_I = 73,
	KeyCode_J = 74,
	KeyCode_K = 75,
	KeyCode_L = 76,
	KeyCode_M = 77,
	KeyCode_N = 78,
	KeyCode_O = 79,
	KeyCode_P = 80,
	KeyCode_Q = 81,
	KeyCode_R = 82,
	KeyCode_S = 83,
	KeyCode_T = 84,
	KeyCode_U = 85,
	KeyCode_V = 86,
	KeyCode_W = 87,
	KeyCode_X = 88,
	KeyCode_Y = 89,
	KeyCode_Z = 90,
	//KeyCode_LEFT_SQUARE_BRACKET = 91,
	//KeyCode_BACKSLASH = 92,
	//KeyCode_RIGHT_SQUARE_BRACKET = 93,
	//KeyCode_CARET = 94,
	//KeyCode_UNDERSCORE = 95,
	//KeyCode_GRAVE_ACCENT = 96,
    KeyCode_F1 = 0x70,
    KeyCode_F2 = 113,
    KeyCode_F3 = 115,
    KeyCode_F4 = 116,
    KeyCode_F5 = 117,
    KeyCode_F6 = 118,
    KeyCode_F7 = 119,
    KeyCode_F8 = 120,
    KeyCode_F9 = 121,
    KeyCode_F10 = 122,
 	//KeyCode_LEFT_CURLY_BRACE =  123,
 	//KeyCode_VERTICAL_BAR =      124,
 	//KeyCode_RIGHT_CURLY_BRACE = 125,

    KeyCode_COMMA = 188,
    KeyCode_PERIOD = 190,
 	KeyCode_TILDE = 192,

    KeyCode_Count,
} KeyCode;


// NOTE: Controller
typedef struct Button{
    bool pressed;
    bool held;
} Button;

typedef struct Mouse{
    v2s32 pos;
    f32 dx;
    f32 dy;
    f32 wheel_direction;
    bool tracking_leave;
} Mouse;

typedef struct Controller{
    Mouse mouse;
    Button button[KeyCode_Count];
} Controller;
global Controller controller;
static void clear_controller_pressed();

//
// NOTE: EVENTS:
//

// TODO: I don't think I need this? I cant just assign it directly to the event.
global bool alt_pressed;
global bool shift_pressed;
global bool ctrl_pressed;

typedef enum EventType{
    QUIT,
    KEYBOARD,
    MOUSE,
    TEXT_INPUT,
} EventType;

typedef struct Event{
    EventType type;
    u64 keycode;

    bool key_pressed;
    bool shift_pressed;
    bool ctrl_pressed;
    bool alt_pressed;
    bool repeat;

    s32 mouse_wheel_dir;
    v2s32 mouse_pos;
    f32 mouse_dx;
    f32 mouse_dy;
} Event;

// NOTE: Must be a size that is a power of 2
typedef struct Events{
    Event e[256];
    u32 write;
    u32 read;
    u32 size;
} Events;
global Events events;

static void  events_init(Events* events);
static u32   events_count(Events* events);
static bool  events_full(Events* events);
static bool  events_empty(Events* events);
static u32   mask(Events* events, u32 idx);
static void  events_add(Events* events, Event event);
static Event events_next(Events* events);

#endif
