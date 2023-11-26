// -----------------------------------------------------------------------------
// font framework
// - rlyeh, public domain

// font size tags
#define FONT_H1 "\1" // largest
#define FONT_H2 "\2"
#define FONT_H3 "\3"
#define FONT_H4 "\4"
#define FONT_H5 "\5"
#define FONT_H6 "\6" // smallest

// font color tags
#define FONT_COLOR1   "\x1a"
#define FONT_COLOR2   "\x1b"
#define FONT_COLOR3   "\x1c"
#define FONT_COLOR4   "\x1d"
#define FONT_COLOR5   "\x1e"
#define FONT_COLOR6   "\x1f"

// font face tags
#define FONT_FACE1    "\x10"
#define FONT_FACE2    "\x11"
#define FONT_FACE3    "\x12"
#define FONT_FACE4    "\x13"
#define FONT_FACE5    "\x14"
#define FONT_FACE6    "\x15"
#define FONT_FACE7    "\x16"
#define FONT_FACE8    "\x17" // editor may override this one
#define FONT_FACE9    "\x18" // editor may override this one
#define FONT_FACE10   "\x19" // editor may override this one

// font align tags
#define FONT_LEFT     "\\<"
#define FONT_CENTER   "\\|"
#define FONT_RIGHT    "\\>"
#define FONT_TOP      "\\^"
#define FONT_MIDDLE   "\\-"
#define FONT_BASELINE "\\_"
#define FONT_BOTTOM   "\\v"

// font flags
enum FONT_FLAGS {
    // font atlas size
    FONT_512 = 0x0,
    FONT_1024 = 0x1,
    FONT_2048 = 0x2,
    FONT_4096 = 0x4,

    // font oversampling
    FONT_NO_OVERSAMPLE = 0x0,
    FONT_OVERSAMPLE_X = 0x08,
    FONT_OVERSAMPLE_Y = 0x10,

    // unicode ranges
    FONT_ASCII = 0x800, // Compatible charset
    FONT_AR = 0x001000, // Arabic and Arabic-Indic digits
    FONT_ZH = 0x002000, // Chinese Simplified (@todo: add ZH_FULL)
    FONT_EL = 0x004000, // Greek, Coptic, modern Georgian, Svan, Mingrelian, Ancient Greek
    FONT_EM = 0x008000, // Emoji
    FONT_EU = 0x010000, // Eastern/western Europe, IPA, Latin ext A/B
    FONT_HE = 0x020000, // Hebrew, Yiddish, Ladino, and other diaspora languages
    FONT_JP = 0x040000, // Hiragana, Katakana, Punctuations, Half-width chars
    FONT_KR = 0x080000, // Korean, Hangul
    FONT_RU = 0x100000, // Cyrillic + ext A/B
    FONT_TH = 0x200000, // Thai
    FONT_VI = 0x400000, // Vietnamese
    FONT_CJK = FONT_ZH|FONT_JP|FONT_KR,

    // FONT_DEFAULTS = FONT_512 | FONT_NO_OVERSAMPLE | FONT_ASCII,
};

typedef struct font_metrics_t {
    float ascent;   // max distance above baseline for all glyphs
    float descent;  // max distance below baseline for all glyphs
    float linegap;  // distance betwen ascent of next line and descent of current line
    float linedist; // distance between the baseline of two lines (ascent - descent + linegap)
} font_metrics_t;

// configures
API void  font_face(const char *face_tag, const char *filename_ttf, float font_size, unsigned flags);
API void  font_face_from_mem(const char *tag, const void *ttf_buffer, unsigned ttf_len, float font_size, unsigned flags);
API void  font_scales(const char *face_tag, float h1, float h2, float h3, float h4, float h5, float h6);
API void  font_color(const char *color_tag, uint32_t color);

// commands
API vec2           font_xy();
API void           font_goto(float x, float y);
API vec2           font_print(const char *text);
API vec2           font_rect(const char *text);
API font_metrics_t font_metrics(const char *text);
//  void  font_clip(vec2 topleft, vec2 bottomright);
//  void  font_wrap(vec2 topleft, vec2 bottomright);

// syntax highlighting
API void* font_colorize(const char *text, const char *comma_types, const char *comma_keywords); // comma separated tokens. expensive, please cache result.
API vec2  font_highlight(const char *text, const void *colors);

// ui
API void  ui_font();
