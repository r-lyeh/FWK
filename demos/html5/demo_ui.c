#include "fwk.h"

void render(void *arg) {
    static int integer = 42;
    static bool toggle = true;
    static bool boolean = true;
    static float floating = 3.14159;
    static float float2[2] = {1,2};
    static float float3[3] = {1,2,3};
    static float float4[4] = {1,2,3,4};
    static float rgb[3] = {0.84,0.67,0.17};
    static float rgba[4] = {0.67,0.90,0.12,1};
    static float slider = 0.5f;
    static float slider2 = 0.5f;
    static char string[64] = "hello world 123";
    static int item = 0; const char *list[] = {"one","two","three"};
    static bool show_dialog = false;
    static uint8_t bitmask = 0x55;

    if( ui_panel("UI", 0)) {
        if( ui_label("my label")) {}
        if( ui_label("my label with tooltip@built on " __DATE__ " " __TIME__)) {}
        if( ui_separator() ) {}
        if( ui_bool("my bool", &boolean) ) puts("bool changed");
        if( ui_int("my int", &integer) ) puts("int changed");
        if( ui_float("my float", &floating) ) puts("float changed");
        if( ui_buffer("my string", string, 64) ) puts("string changed");
        if( ui_separator() ) {}
        if( ui_slider("my slider", &slider)) puts("slider changed");
        if( ui_slider2("my slider 2", &slider2, va("%.2f", slider2))) puts("slider2 changed");
        if( ui_separator() ) {}
        if( ui_list("my list", list, 3, &item ) ) puts("list changed");
        if( ui_separator() ) {}
        if( ui_color3f("my color3", rgb) ) puts("color3 changed");
        if( ui_color4f("my color4@this is a tooltip", rgba) ) puts("color4 changed");
        if( ui_separator() ) {}
        if( ui_float2("my float2", float2) ) puts("float2 changed");
        if( ui_float3("my float3", float3) ) puts("float3 changed");
        if( ui_float4("my float4", float4) ) puts("float4 changed");
        if( ui_bits8("my bitmask", &bitmask) ) printf("bitmask changed %x\n", bitmask);
        if( ui_separator() ) {}
        if( ui_toggle("my toggle", &toggle) ) printf("toggle %s\n", toggle ? "on":"off");
        if( ui_separator() ) {}
        if( ui_image("my image", texture_checker().id, 0, 0) ) { puts("image clicked"); }
        if( ui_button("my button") ) { puts("button clicked"); show_dialog = true; }
        if( ui_dialog("my dialog", __FILE__ "\n" __DATE__ "\n" "Public Domain.", 2/*two buttons*/, &show_dialog) ) {}

        ui_panel_end();
    }

    input_demo();
}

int main() {
    window_create(0.75f, 0);
    window_loop(render, NULL);
}
