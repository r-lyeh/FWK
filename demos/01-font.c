#include "fwk.h"

int main() {
    window_create(75.0, WINDOW_MSAA8);

    // style: our aliases
    #define FONT_REGULAR   FONT_FACE1
    #define FONT_ITALIC    FONT_FACE2
    #define FONT_BOLD      FONT_FACE3
    #define FONT_JAPANESE  FONT_FACE4
    #define FONT_MONOSPACE FONT_FACE5

    #define FONT_WHITE     FONT_COLOR1
    #define FONT_GRAY      FONT_COLOR2
    #define FONT_ORANGE    FONT_COLOR3
    #define FONT_LIME      FONT_COLOR4
    #define FONT_GREEN     FONT_COLOR5
    #define FONT_CYAN      FONT_COLOR6

    #define FONT_LARGEST   FONT_H1
    #define FONT_LARGE     FONT_H2
    #define FONT_MEDIUM    FONT_H3
    #define FONT_NORMAL    FONT_H4
    #define FONT_SMALL     FONT_H5
    #define FONT_TINY      FONT_H6

    // style: atlas size, unicode ranges and font faces (up to 10 faces)
    font_face(FONT_REGULAR,   "B612""-Regular.ttf", 48.f, FONT_EU|FONT_AR|FONT_RU|FONT_2048);
    font_face(FONT_ITALIC,    "B612""-Italic.ttf", 48.f, FONT_EU|FONT_AR|FONT_RU|FONT_2048);
    font_face(FONT_BOLD,      "B612""-Bold.ttf", 48.f, FONT_EU|FONT_AR|FONT_RU|FONT_2048);
    font_face(FONT_JAPANESE,  "mplus-1p-medium.ttf", 48.f, FONT_JP|FONT_2048); // CJK|FONT_2048|FONT_OVERSAMPLE_Y);
    font_face(FONT_MONOSPACE, "B612Mono""-Regular.ttf", 24.f, FONT_EU|FONT_512);

    // style: colors (up to 6 colors)
    font_color(FONT_GRAY,   RGB4(100,100,100,255));
    font_color(FONT_ORANGE, RGB4(255,192,0,255));
    font_color(FONT_LIME,   RGB4(192,255,0,255));
    font_color(FONT_GREEN,  RGB4(0,255,192,255));
    font_color(FONT_CYAN,   RGB4(0,192,255,255));

    // prepare color highlighting for following code snippet
    const char *source =
        FONT_MONOSPACE FONT_LARGEST
        "int main(int argc, char **argv) {\n"
        "   for( int i = 0; i < 10; ++i)\n"
        "   puts(\"hello world\");\n"
        "   return 0;\n"
        "}\n";
    const void *colors = font_colorize(source, "void,int,char", "if,else,for,do,while,return,switch,case,break,default,");

    // read input file with strings to display
    array(char*) lines = 0;
    for each_substring( vfs_read("pangrams.txt"), "\r\n", it ) array_push(lines, STRDUP(it));

    // panning controls
    vec2 cursor = {0};

    // demo loop
    while( window_swap() && !input(KEY_ESC) ) {
        ddraw_grid(0);

        // pan the viewport
        if (input(MOUSE_L) && !ui_active()) {
            cursor.x += input_diff(MOUSE_X);
            cursor.y += input_diff(MOUSE_Y);
        }

        // initial spacing
        font_goto(cursor.x, cursor.y + 50);

        // print a code snippet with syntax highlighting
        font_highlight(source, colors);

        // print a few strings with markup codes
        font_print(
            FONT_REGULAR
            FONT_LARGEST  FONT_GRAY   "The quick "
            FONT_LARGE    FONT_LIME   "brown "
            FONT_MEDIUM   FONT_GRAY   "fox "
            FONT_NORMAL   "jumps over "
            FONT_SMALL    "the lazy "
            FONT_TINY     "dog.\n");

        font_print(
            FONT_REGULAR FONT_LARGE FONT_CYAN
            "Now is the time for all " FONT_ITALIC "good men " FONT_REGULAR "to come to the aid of " FONT_BOLD "the party.\n");

        font_print(
            FONT_ITALIC FONT_LARGE FONT_GREEN
            "Ég get etið gler án þess að meiða mig!\n");

        font_print(
            FONT_BOLD FONT_LARGE FONT_ORANGE
            "Эх, чужак! Общий съём цен шляп (юфть)—вдрызг!.\n");

        font_print(
            FONT_JAPANESE
            "私はガラスを食べられます。それは私を傷つけません。\n");

        font_print( "This text ");
        font_print( "should display concatenated, ");
        font_print( "as there are no linefeeds.\n" );

        // i18n: pangrams.txt file, line browser
        static int counter = 0;
        counter += input_down(KEY_RIGHT)-input_down(KEY_LEFT);
        counter += counter < 0 ? array_count(lines) : 0;
        font_print( va("<< %s >>\n", lines[counter % array_count(lines)]) );

        // this does not work yet. you cant chain alignments yet...
        //font_print(FONT_TOP "Top" FONT_MIDDLE "Middle" FONT_BASELINE "Baseline" FONT_BOTTOM "Bottom\n");
        //font_print(FONT_LEFT "Left" FONT_CENTER "Center" FONT_RIGHT "Right\n");

        // ... alignment must be the first tag in a string for now. this is a temporary hack.
        font_goto(0, window_height()*3/4.);
        font_print(FONT_LEFT     "left");
        font_print(FONT_CENTER   "center");
        font_print(FONT_RIGHT    "right");

        font_print(FONT_TOP      FONT_CENTER "top");
        font_print(FONT_MIDDLE   FONT_RIGHT  "middle");
        font_print(FONT_BASELINE FONT_RIGHT  "baseline");
        font_print(FONT_BOTTOM   FONT_CENTER "bottom");

        {
            vec2 pos = vec2(cursor.x + 1290, cursor.y + 120);
            ddraw_push_2d();
            char *txt = "This is the first line.\nAnd now the second line.\nYou can do a third great line, too!\n";
            font_goto(pos.x, pos.y);
            vec2 size=font_rect(txt);
            font_metrics_t m=font_metrics(txt);
            ddraw_aabb(vec3(pos.x,pos.y,0), vec3(pos.x+size.x,pos.y+size.y-m.descent+m.linegap,0));
            font_print(txt);
            ddraw_pop_2d();
        }

        {
            vec2 pos = vec2(cursor.x + 1490, cursor.y + 820);
            ddraw_push_2d();
            char *txt = FONT_JUSTIFY FONT_MIDDLE "This is the first line.\nAnd now the second line.\nYou can do a third great line, too!\nNow this is a very long line aaaaaaaaaa!\n";
            font_goto(pos.x, pos.y);
            vec2 size=font_rect(txt);
            font_metrics_t m=font_metrics(txt);
            ddraw_aabb(vec3(pos.x,pos.y,0), vec3(pos.x+size.x,pos.y+size.y-m.descent+m.linegap,0));
            font_clip(txt, vec4(pos.x, pos.y, size.x, size.y));
            ddraw_pop_2d();
        }

        static float max_width = 300.0f;
        {
            vec2 pos = vec2(cursor.x + 2000, cursor.y + 240);
            ddraw_push_2d();
            char *txt = "This is the first line.\n \nAnd now the second line.\n \nYou can do a third great line, too!\n \nNow this is a very long line aaaaaaaaaa!\n";
            font_goto(pos.x, pos.y);
            const char *wrapped_text = font_wrap(txt, max_width);
            vec2 size=font_rect(wrapped_text);
            font_metrics_t m=font_metrics(wrapped_text);
            ddraw_aabb(vec3(pos.x,pos.y,0), vec3(pos.x+size.x,pos.y+size.y-m.descent+m.linegap,0));
            font_clip(wrapped_text, vec4(pos.x, pos.y, size.x, size.y));
            ddraw_pop_2d();
        }

        {
            vec2 pos = vec2(cursor.x + 1490, cursor.y + 240);
            ddraw_push_2d();
            char *txt = "This is the first line.\nAnd now the second line.\nYou can do a third great line, too!\nNow this is a very long line aaaaaaaaaa!\n";
            font_goto(pos.x, pos.y);
            vec2 size=font_rect(txt);
            size.y -= 20; // artifically shrink textbox to test clipping
            font_metrics_t m=font_metrics(txt);
            ddraw_aabb(vec3(pos.x,pos.y,0), vec3(pos.x+size.x,pos.y+size.y-m.descent+m.linegap,0));
            font_clip(txt, vec4(pos.x, pos.y, size.x, size.y));
            ddraw_pop_2d();
        }

        {
            vec2 pos = vec2(cursor.x + 1990, cursor.y + 820);
            ddraw_push_2d();
            char *txt = FONT_RIGHT FONT_BOTTOM 
                        FONT_WHITE "This is the first line.\n" 
                        FONT_LIME "And now the second line.\n" 
                        FONT_WHITE "You can do a third" 
                        FONT_ORANGE " great" 
                        FONT_WHITE " line, too!\n";
            font_goto(pos.x, pos.y);
            vec2 size=font_rect(txt);
            font_metrics_t m=font_metrics(txt);
            ddraw_aabb(vec3(pos.x,pos.y,0), vec3(pos.x+size.x,pos.y+size.y-m.descent+m.linegap+100,0));
            font_clip(txt, vec4(pos.x, pos.y, size.x, size.y+100));
            ddraw_pop_2d();
        }

        {
            vec2 pos = vec2(cursor.x + 830, cursor.y + 80);
            ddraw_push_2d();
            char *txt = "Very iffy global text.";
            font_goto(pos.x, pos.y);
            vec2 size=font_rect(txt);

            ddraw_aabb(vec3(pos.x,pos.y,0), vec3(pos.x+size.x,pos.y+size.y,0));
            font_print(txt);
            ddraw_pop_2d();
        }

        {
            vec2 pos = vec2(cursor.x + 830, cursor.y + 160);
            ddraw_push_2d();
            char *txt = FONT_H1 "Very iffy global text.";
            font_goto(pos.x, pos.y);
            vec2 size=font_rect(txt);

            ddraw_aabb(vec3(pos.x,pos.y,0), vec3(pos.x+size.x,pos.y+size.y,0));
            font_print(txt);
            ddraw_pop_2d();
        }

        for (int i = 1; i <= 6; i++) {
            vec2 pos = vec2(cursor.x + 830, cursor.y + 550 + 55*(i+1));
            ddraw_push_2d();
            char *txt = va("%cTest line %d", i, i);
            font_goto(pos.x, pos.y);
            vec2 size=font_rect(txt);

            ddraw_aabb(vec3(pos.x,pos.y,0), vec3(pos.x+size.x,pos.y+size.y,0));
            font_print(txt);
            ddraw_pop_2d();
        }

        if (ui_panel("Fonts", 0)) {
            ui_font();
            ui_panel_end();
        }

        if (ui_panel("Textbox", 0)) {
            ui_float("Max width", &max_width);
            ui_panel_end();
        }
    }
}
