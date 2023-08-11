// -----------------------------------------------------------------------------
// input framework
// - rlyeh, public domain
//
// @todo: window
// @todo: for extra savings (168->72 bytes), promote bits to real bits (/8 %8) & normalized floats [-1,+1] to shorts or chars
// @todo: GAMEPAD_A|2, MOUSE_L|1, KEY_C|3
// @todo: load/save
// @todo: send virtual presses & outputs (rumble, light, led, text, etc)
// @todo: fix if logger !60 hz
// @tofo: fix click2/repeat edge cases

API int         input_use( int controller_id ); // [0..3]

// -- basic polling api (read input at current frame)

API float       input( int vk );
API vec2        input2( int vk );
API float       input_diff( int vk ); // @todo: rename diff->delta
API vec2        input_diff2( int vk ); // @todo: rename diff2->delta2

// -- extended polling api (read input at Nth frame ago)

API float       input_frame( int vk, int frame );
API vec2        input_frame2( int vk, int frame );
API const char* input_frames( int vk, int frame );

// -- events api

API int         input_up( int vk ); // ON -> OFF (release)
API int         input_down( int vk ); // OFF -> ON (trigger)
API int         input_held( int vk ); // ON -> ON (pressed)
API int         input_idle( int vk ); // OFF -> OFF

API int         input_click( int vk, int ms ); // OFF -> ON -> OFF
API int         input_click2( int vk, int ms ); // OFF -> ON -> OFF -> ON -> OFF
API int         input_repeat( int vk, int ms ); // [...] ON -> ON -> ON

API int         input_chord2( int vk1, int vk2 ); // all vk1 && vk2 are ON
API int         input_chord3( int vk1, int vk2, int vk3 ); // all vk1 && vk2 && vk3 are ON
API int         input_chord4( int vk1, int vk2, int vk3, int vk4 ); // all vk1 && vk2 && vk3 && vk4 are ON

// -- 1d/2d filters

API float       input_filter_positive( float v ); // [-1..1] -> [0..1]
API vec2        input_filter_positive2( vec2 v ); // [-1..1] -> [0..1]
API vec2        input_filter_deadzone( vec2 v, float deadzone_treshold );
API vec2        input_filter_deadzone_4way( vec2 v, float deadzone_treshold );

// -- multi-touch

enum TOUCH_BUTTONS {
    TOUCH_0,    // defaults to left screen area. input_touch_area() to override
    TOUCH_1,    // defaults to right screen area. input_touch_area() to override
};

API void        input_touch_area(unsigned button, vec2 begin_coord_ndc, vec2 end_coord_ndc);
API vec2        input_touch(unsigned button, float sensitivity);                   // absolute position in 2d coords
API vec2        input_touch_delta(unsigned button, float sensitivity);             // delta from previous position
API vec2        input_touch_delta_from_origin(unsigned button, float sensitivity); // relative position from initial touch
API bool        input_touch_active();

// -- utils

API void        input_demo();
API void        input_mappings(const char *filename); // update gamepad mappings (usually "gamecontrollerdb.txt" file)
API void        input_send( int vk ); // @todo
API void*       input_save_state( int id, int *size); // @todo
API bool        input_load_state( int id, void *ptr, int size); // @todo
API char        input_keychar(unsigned code); // Converts keyboard code to its latin char (if any)
API int         input_anykey();

// --

enum INPUT_ENUMS {
    // -- bits: x104 keyboard, x3 mouse, x15 gamepad, x7 window
    // keyboard gaming keys (53-bit): first-class keys for gaming
    KEY_ESC,
    KEY_TICK, KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,KEY_0,  KEY_BS,
    KEY_TAB,   KEY_Q,KEY_W,KEY_E,KEY_R,KEY_T,KEY_Y,KEY_U,KEY_I,KEY_O,KEY_P,
    KEY_CAPS,     KEY_A,KEY_S,KEY_D,KEY_F,KEY_G,KEY_H,KEY_J,KEY_K,KEY_L, KEY_ENTER,
    KEY_LSHIFT,       KEY_Z,KEY_X,KEY_C,KEY_V,KEY_B,KEY_N,KEY_M,        KEY_RSHIFT,            KEY_UP,
    KEY_LCTRL,KEY_LALT,               KEY_SPACE,                KEY_RALT,KEY_RCTRL,  KEY_LEFT,KEY_DOWN,KEY_RIGHT,

    // for completeness, secondary keys below (52-bit). beware!
    KEY_INS,KEY_HOME,KEY_PGUP,KEY_DEL,KEY_END,KEY_PGDN, // beware: different behavior win/osx (also, osx: no home/end).
    KEY_LMETA,KEY_RMETA,KEY_MENU,KEY_PRINT,KEY_PAUSE,KEY_SCROLL,KEY_NUMLOCK, // beware: may trigger unexpected OS behavior. (@todo: add RSHIFT here for win?)
    KEY_MINUS,KEY_EQUAL,KEY_LSQUARE,KEY_RSQUARE,KEY_SEMICOLON,KEY_QUOTE,KEY_HASH,KEY_BAR,KEY_COMMA,KEY_DOT,KEY_SLASH, // beware: non-us keyboard layouts
    KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F6,KEY_F7,KEY_F8,KEY_F9,KEY_F10,KEY_F11,KEY_F12, // beware: complicated on laptops/osx
    KEY_PAD1,KEY_PAD2,KEY_PAD3,KEY_PAD4,KEY_PAD5,KEY_PAD6,KEY_PAD7,KEY_PAD8,KEY_PAD9,KEY_PAD0, // beware: complicated on laptops
    KEY_PADADD,KEY_PADSUB,KEY_PADMUL,KEY_PADDIV,KEY_PADDOT,KEY_PADENTER, // beware: complicated on laptops

    MOUSE_L, MOUSE_M, MOUSE_R,
    GAMEPAD_CONNECTED, GAMEPAD_A, GAMEPAD_B, GAMEPAD_X, GAMEPAD_Y,
    GAMEPAD_UP, GAMEPAD_DOWN, GAMEPAD_LEFT, GAMEPAD_RIGHT, GAMEPAD_MENU, GAMEPAD_START,
    GAMEPAD_LB, GAMEPAD_RB, GAMEPAD_LTHUMB, GAMEPAD_RTHUMB,
    WINDOW_BLUR, WINDOW_FOCUS, WINDOW_CLOSE, WINDOW_MINIMIZE, WINDOW_MAXIMIZE, WINDOW_FULLSCREEN, WINDOW_WINDOWED, // MINI/MAXI/RESTORED, SHOWN/HIDDEN

    // -- floats: x7 gamepad, x3 mouse, x4 touch, x4 window
    GAMEPAD_LPAD, GAMEPAD_LPADX = GAMEPAD_LPAD, GAMEPAD_LPADY,
    GAMEPAD_RPAD, GAMEPAD_RPADX = GAMEPAD_RPAD, GAMEPAD_RPADY,
    GAMEPAD_LTRIGGER, GAMEPAD_LT = GAMEPAD_LTRIGGER, GAMEPAD_RTRIGGER, GAMEPAD_RT = GAMEPAD_RTRIGGER, GAMEPAD_BATTERY,
    MOUSE, MOUSE_X = MOUSE, MOUSE_Y, MOUSE_W,
    TOUCH_X1, TOUCH_Y1, TOUCH_X2, TOUCH_Y2,
    WINDOW_RESIZE, WINDOW_RESIZEX = WINDOW_RESIZE, WINDOW_RESIZEY, WINDOW_ORIENTATION, WINDOW_BATTERY,

    // -- strings: x2 gamepad
    GAMEPAD_GUID, GAMEPAD_NAME,
};
