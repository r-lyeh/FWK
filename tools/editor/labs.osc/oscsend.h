#pragma once
#define osc_open    udp_open
#define osc_send    udp_send
#define osc_close   udp_close

// rgb_formats:
// - 888 (RGB 3bytes/pixel),
// - 332 (RGB 1byte/pixel),
// - 242 (RGB 1byte/pixel),
// - 7755 (YCoCg 3bytes/2pixels)

API void osc_send_fb( void *pixels, int w, int h, int comp, int rgb_format );


// usage:
// void* rgb = render_capture(3);
// int rgbmode = 332; // 7755,242,332,888 safest+slowest
// osc_send_fb(rgb, window_width(), window_height(), 3, rgbmode); // 242 crash); 7755 green);

//viewport_clear(false, true);
//ddraw_frame(NULL);
//ddraw_axis(10);


#pragma once
#include "oscpack.h"

void osc_send_fb( void *pixels, int w, int h, int comp, int rgb_format ) {
    static int s, *init = 0;
    if( !init ) { init = &s; s = osc_open("127.0.0.1", "9000"); }
    if( s <= 0 ) return;

    if( !pixels ) return;
    if( (w * h * comp) <= 0 ) return;

    unsigned char* line_a = (unsigned char *)pixels;
    unsigned char* line_b = (unsigned char *)pixels + (0 + (h - 1) * w) * comp;
    unsigned char* line;

    static unsigned char activity = 0;
    static unsigned num_sent_packets = 0;

    static char *oscbuf = 0;
    if( !oscbuf ) oscbuf = (char*)MALLOC( 8192 * 4 + 64 ); // 8K RGBA32 max + some room for osc headers

    bool sending = 0;
    int stride = w * comp;

    line = line_b + stride;
    if( rgb_format == 888 ) for( int y = 0; y < h; ++y ) { line -= stride;
        int osclen = osc_pack( oscbuf, "/render/", "iiiib", w,h,rgb_format, y, w*3,line);
        bool sent = osc_send( s, oscbuf + 4, osclen - 4 );
        num_sent_packets += sent; sending |= sent;
    }
    if( rgb_format == 332 ) for( int y = 0; y < h; ++y ) { line -= stride;
        for( int x = 0; x < w; ++x ) {
            unsigned char r = line[x*3+0], g = line[x*3+1], b = line[x*3+2];
            line[x] = (( r >> 5 ) << 5) | (( g >> 5 ) << 2) | (( b >> 6 ) << 0);
        }
        int osclen = osc_pack( oscbuf, "/render/", "iiiib", w,h,rgb_format, y, w,line);
        bool sent = osc_send( s, oscbuf + 4, osclen - 4 );
        num_sent_packets += sent; sending |= sent;
    }
    if( rgb_format == 242 ) for( int y = 0; y < h; ++y ) { line -= stride;
        for( int x = 0; x < w; ++x ) {
            unsigned char r = line[x*3+0], g = line[x*3+1], b = line[x*3+2];
            line[x] = (( r >> 6 ) << 6) | (( g >> 4 ) << 2) | (( b >> 6 ) << 0);
        }
        int osclen = osc_pack( oscbuf, "/render/", "iiiib", w,h,rgb_format, y, w,line);
        bool sent = osc_send( s, oscbuf + 4, osclen - 4 );
        num_sent_packets += sent; sending |= sent;
    }
    if( rgb_format == 7755 ) for( int y = 0; y < h; ++y ) { line -= stride;
        /**/ if( (w % 2) == 0 ) {}
        else if( (w % 2) == 1 ) w -= 1;
        assert( (w % 2) == 0 );
        uint8_t *out = line;
        for( int x = 0, i = 0; i < w/2; ++i, ++x ) {

            // (/2>>1 /4>>2 /8>>3 /16>>4 /32>>5 /64>>6 /128>>7)

            // Y Y CoCg (7 7 5 5) : 3bytes == 2px !!wow!!
            int32_t r,g,b,y0,y1,co0,co1,cg0,cg1;
            r = line[x*6+0], g = line[x*6+1], b = line[x*6+2];
            y0 = r/4 + g/2 + b/4 + 1; // +1 to avoid some ycocg2rgb overflows
            co0 = r/2 - b/2;
            cg0 = g/2 - r/4 - b/4;
            r = line[x*6+3], g = line[x*6+4], b = line[x*6+5];
            y1 = r/4 + g/2 + b/4 + 1; // +1 to avoid some ycocg2rgb overflows
            co1 = r/2 - b/2;
            cg1 = g/2 - r/4 - b/4;
            int32_t co = (co0+co1)/2 + 128, cg = (cg0+cg1)/2 + 128;
            // 7 7 5 5
            uint8_t y0_7 = y0 / 2, y1_7 = y1 / 2, co_5 = co / 8, cg_5 = cg / 8;
            uint32_t pack24 = (y0_7<<17)|(y1_7<<10)|(co_5<<5)|(cg_5<<0);
            *out++ = ( pack24 >> 16 ) & 255;
            *out++ = ( pack24 >>  8 ) & 255;
            *out++ = ( pack24 >>  0 ) & 255;

            // other packed options
            // YCoCg (4 4 4) : 12bits == 1px
            // YCoCg (2 3 3) : 1byte == 1px
            // Y CoCg (8 4 4) : 2bytes == 1px
            // Y Y CoCg (5 5 3 3) : 2bytes == 2px (meh)
            // Y Y CoCg (8 8 4 4) : 3bytes == 2px
            // Y Y CoCg (7 7 5 5) : 3bytes == 2px (wow!)
            // Y Y Y CoCg (8 8 8 4 4) : 4bytes == 3px
            // Y Y Y CoCg (7 7 7 5 6) : 4bytes == 3px
            // Y Y Y CoCg (6 6 6 7 7) : 4bytes == 3px
            // Y Y Y Y CoCg (7 7 7 7 6 6) : 5bytes == 4px
            // Y Y Y Y CoCg (6 6 6 6 8 8) : 5bytes == 4px
        }
        int osclen = osc_pack( oscbuf, "/render/", "iiiib", w,h,rgb_format, y, w*2,line);
        bool sent = osc_send( s, oscbuf + 4, osclen - 4 );
        num_sent_packets += sent; sending |= sent;
    }

    activity += sending;
    // printf("\r%c netsend %d", "\\|/-"[ activity % 4 ], num_sent_packets);
    // stbi_write_png("out.png", w, h, 3, pixels, w * 3);
}
