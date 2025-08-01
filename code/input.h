#ifndef INPUT_H
#define INPUT_H

typedef enum KeyCode{
    KeyCode_None = 0,

    // change to MouseButton_Right or KeyCode_MBR, MBL, MBM, KeyCode_MWU,MWD
    MOUSE_BUTTON_LEFT     = 0x01,
    MOUSE_BUTTON_RIGHT    = 0x02,
    MOUSE_BUTTON_MIDDLE   = 0x04,
    MOUSE_WHEEL_UP        = 0x05,
    MOUSE_WHEEL_DOWN      = 0x06,

    KeyCode_BACKSPACE     = 0x08,
    KeyCode_TAB 	      = 0x09,
    KeyCode_ENTER         = 0x0D,
    KeyCode_SHIFT         = 0x10,
    KeyCode_CTRL          = 0x11,
    KeyCode_ALT           = 0x12,
    KeyCode_CAPSLOCK      = 0x14,
    KeyCode_ESCAPE 	      = 0x1B,
    KeyCode_SPACEBAR      = 0x20,
    KeyCode_PAGE_UP       = 0x21,
    KeyCode_PAGE_DOWN     = 0x22,
    KeyCode_END 	      = 0x23,
    KeyCode_HOME          = 0x24,
    KeyCode_LEFT 	      = 0x25,
    KeyCode_UP 	          = 0x26,
    KeyCode_RIGHT 	      = 0x27,
    KeyCode_DOWN 	      = 0x28,
    KeyCode_INSERT 	      = 0x2D,
    KeyCode_DELETE 	      = 0x2E,

    KeyCode_ZERO          = 0x30,
    KeyCode_ONE           = 0x31,
    KeyCode_TWO           = 0x32,
    KeyCode_THREE         = 0x33,
    KeyCode_FOUR          = 0x34,
    KeyCode_FIVE          = 0x35,
    KeyCode_SIX           = 0x36,
    KeyCode_SEVEN         = 0x37,
    KeyCode_EIGHT         = 0x38,
    KeyCode_NINE          = 0x39,
    KeyCode_A             = 0x41,
    KeyCode_B             = 0x42,
    KeyCode_C             = 0x43,
    KeyCode_D             = 0x44,
    KeyCode_E             = 0x45,
    KeyCode_F             = 0x46,
    KeyCode_G             = 0x47,
    KeyCode_H             = 0x48,
    KeyCode_I             = 0x49,
    KeyCode_J             = 0x4A,
    KeyCode_K             = 0x4B,
    KeyCode_L             = 0x4C,
    KeyCode_M             = 0x4D,
    KeyCode_N             = 0x4E,
    KeyCode_O             = 0x4F,
    KeyCode_P             = 0x50,
    KeyCode_Q             = 0x51,
    KeyCode_R             = 0x52,
    KeyCode_S             = 0x53,
    KeyCode_T             = 0x54,
    KeyCode_U             = 0x55,
    KeyCode_V             = 0x56,
    KeyCode_W             = 0x57,
    KeyCode_X             = 0x58,
    KeyCode_Y             = 0x59,
    KeyCode_Z             = 0x5A,

    KeyCode_WINDOW_LEFT   = 0x5B,
    KeyCode_WINDOW_RIGHT  = 0x5C,
    KeyCode_NUMPAD0 	  = 0x60,
    KeyCode_NUMPAD1 	  = 0x61,
    KeyCode_NUMPAD2 	  = 0x62,
    KeyCode_NUMPAD3 	  = 0x63,
    KeyCode_NUMPAD4 	  = 0x64,
    KeyCode_NUMPAD5 	  = 0x65,
    KeyCode_NUMPAD6 	  = 0x66,
    KeyCode_NUMPAD7 	  = 0x67,
    KeyCode_NUMPAD8 	  = 0x68,
    KeyCode_NUMPAD9 	  = 0x69,

    KeyCode_F1 	          = 0x70,
    KeyCode_F2 	          = 0x71,
    KeyCode_F3 	          = 0x72,
    KeyCode_F4 	          = 0x73,
    KeyCode_F5 	          = 0x74,
    KeyCode_F6 	          = 0x75,
    KeyCode_F7 	          = 0x76,
    KeyCode_F8 	          = 0x77,
    KeyCode_F9 	          = 0x78,
    KeyCode_F10 	      = 0x79,
    KeyCode_F11 	      = 0x7A,
    KeyCode_F12 	      = 0x7B,

    KeyCode_SEMI_COLON 	  = 0xBA,
    KeyCode_EQUAL 	      = 0xBB,
    KeyCode_COMMA 	      = 0xBC,
    KeyCode_MINUS 	      = 0xBD,
    KeyCode_PERIOD 	      = 0xBE,

    KeyCode_QUESTION_MARK = 0xBF,
    KeyCode_TILDE         = 0xC0,
    KeyCode_LEFT_BRACKET  = 0xDB,
    KeyCode_PIPE          = 0xDC,
    KeyCode_RIGHT_BRACKET = 0xDD,
    KeyCode_QUOTE         = 0xDE,

    //MOUSE_BUTTON_X1       = 0x05,
    //MOUSE_BUTTON_X2       = 0x06,

    //KeyCode_ADD 	      = 0x6B, 	Add key
    //KeyCode_SEPARATOR   = 0x6C, 	Separator key
    //KeyCode_SUBTRACT 	  = 0x6D, 	Subtract key
    //KeyCode_DIVIDE 	  = 0x6F, 	Divide key

    //KeyCode_SHIFT_LEFT  = 0xA0, 	Left SHIFT key
    //KeyCode_SHIFT_RIGHT = 0xA1, 	Right SHIFT key
    //KeyCode_CTRL_LEFT   = 0xA2, 	Left CONTROL key
    //KeyCode_CTRL_RIGHT  = 0xA3, 	Right CONTROL key
    //KeyCode_ALT_LEFT    = 0xA4, 	Left ALT key
    //KeyCode_ALT_RIGHT   = 0xA5, 	Right ALT key

    KeyCode_Count,
} KeyCode;

// NOTE: Controller
typedef struct Button{
    bool released;
    bool pressed;
    bool held;
} Button;

typedef struct Mouse{
    union{
        struct{
            f32 x;
            f32 y;
        };
        v2 pos;
    };
    f32 dx;
    f32 dy;
    s32 wheel_dir;

    bool edge_left;
    bool edge_right;
    bool edge_top;
    bool edge_bottom;
} Mouse;

typedef struct Controller{
    Mouse mouse;
    Button button[KeyCode_Count];

    bool shift_pressed;
    bool ctrl_pressed;
    bool alt_pressed;
} Controller;
global Controller controller;

static void clear_controller_pressed(void);
static void clear_controller_held(void);
static bool controller_button_pressed(KeyCode key, bool consume = false);
static bool controller_button_released(KeyCode key, bool consume = false);
static bool controller_button_held(KeyCode key);

// NOTE: EVENTS:
global bool shift_pressed;
global bool ctrl_pressed;
global bool alt_pressed;

typedef enum EventType{
    EventType_NONE,
    EventType_QUIT,
    EventType_KEYBOARD,
    EventType_MOUSE,
    EventType_TEXT_INPUT,
    EventType_NO_CLIENT,
    EventType_COUNT,
} EventType;

typedef struct Event{
    EventType type;
    u64 keycode;

    bool repeat;
    bool key_pressed;
    bool key_released;
    bool shift_pressed;
    bool ctrl_pressed;
    bool alt_pressed;

    s32 mouse_wheel_dir;
    f32 mouse_x;
    f32 mouse_y;
    f32 mouse_dx;
    f32 mouse_dy;

    bool mouse_edge_left;
    bool mouse_edge_right;
    bool mouse_edge_top;
    bool mouse_edge_bottom;
} Event;

// NOTE: Must be a size that is a power of 2
typedef struct Events{
    Event e[256];
    u32 write;
    u32 read;
    u32 size;
} Events;
global Events events;

static void  init_events(Events* events);
static u32   events_count(Events* events);
static bool  events_full(Events* events);
static bool  events_empty(Events* events);
static u32   mask(Events* events, u32 idx);
static void  events_add(Events* events, Event event);
static Event events_next(Events* events);
static void  events_quit(Events* events);

#endif
