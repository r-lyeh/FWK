// OSC buffer packing,
// - rlyeh, public domain.
//
// pack format: (i)nt, (h)int64, (t)ime, (f)loat, (s)tring, (S)ymbol, (c)har, (r)gba, (m)idi,
// (T)true, (N|F)nil+false, (I)nfinity, (b)lob, (d)ouble,                    @todo: ([)array.
//
// warning: osc_pack() generates OSC compliant messages, however,
// every osc_pack_va() call generates a 32-bit length prefix (in machine dependant order)

API int   osc_bundle( char *buf, uint64_t ts );
API int   osc_pack( char *buf, const char *addr, const char *fmt, ... );
API char* osc_pack_va( const char *addr, const char *fmt, ... );


#pragma once
/*
#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32")
#else
#include <arpa/inet.h>
#endif
*/

int osc__buffer_vl( char *buf, const char *fmt, va_list vl ) {
    // if `buf` is NULL, just calc needed space
    if( !buf ) {
        int bytes = 0;
        while( *fmt++ ) {
            switch( fmt[-1] ) {
                default: // bypass
                break; case 'T': case 'F': case 'N': case 'I': bytes += 4;
                break; case 'i': case 'c': case 'r': case 'm': bytes += 4; (void)va_arg(vl, uint32_t);
                break; case 't': case 'h':                     bytes += 8; (void)va_arg(vl, uint64_t);
                break; case 'd':                               bytes += 8; (void)va_arg(vl, double);
                break; case 'f':                               bytes += 4; (void)va_arg(vl, double);
                break; case 'b':                               bytes += va_arg(vl, uint32_t); (void)va_arg(vl, const char *);
                break; case 's': case 'S':                     bytes += strlen( va_arg(vl, const char *) ) + 1;
            }
        }
        return bytes;
    }

    char *src = buf;
    while( *fmt++ ) {
        switch( fmt[-1] ) {
            default: *buf++ = fmt[-1]; // bypass
            break; case 'T': case 'F': case 'N': case 'I':
            {}
            break; case 'i': case 'c': case 'r': case 'm':
            { uint32_t i = va_arg(vl, uint32_t); i = ntohl(i); memcpy(buf, &i, 4); buf += 4; }
            break; case 't': case 'h':
            { uint64_t l = va_arg(vl, uint64_t); l = ntohll(l); memcpy(buf, &l, 8); buf += 8; }
            break; case 'd':
            { union { double f; uint64_t i; } u; u.f = va_arg(vl, double); u.i = ntohll(u.i); memcpy(buf, &u.i, 8); buf += 8; }
            break; case 'f':
            { union { float f; uint32_t i; } u; u.f = (float)va_arg(vl, double); u.i = ntohl(u.i); memcpy(buf, &u.i, 4); buf += 4; }
            break; case 'b':
            { uint32_t l = va_arg(vl, uint32_t), ll = ntohl(l); memcpy(buf, &ll, 4); buf += 4; /*}*/
            /*{*/ const char *s = va_arg(vl, const char *); int32_t i = 0;
              memcpy(buf, s, l); memcpy(buf + l, &i, 4); buf += l; buf += 3 - (((intptr_t)buf+3) & 3); }
            break; case 's': case 'S':
            { const char *s = va_arg(vl, const char *); int32_t i = 0, l = (int32_t)strlen(s) + 1;
              memcpy(buf, s, l); memcpy(buf + l, &i, 4); buf += l; buf += 3 - (((intptr_t)buf+3) & 3); }
        }
    }
    return buf - src;
}

int osc__buffer( char *buf, const char *fmt, ... ) {
    va_list vl;
    va_start(vl, fmt);
    int l = osc__buffer_vl(buf, fmt, vl);
    va_end(vl);
    return l;
}

int osc_pack( char *buf, const char *addr, const char *fmt, ... ) {
    char tmp[8192];
    va_list vl;
    va_start(vl, fmt);
    int l2 = osc__buffer_vl(tmp, fmt, vl);
    va_end(vl);
    int l1 = osc__buffer(buf, "s,s", addr, fmt);
    memcpy(buf+l1, tmp, l2);
    return l1+l2;
}

char* osc_pack_va( const char *addr, const char *fmt, ... ) { // @todo: optimize me
    char buf[1024];

    char tmp[8192];
    va_list vl;
    va_start(vl, fmt);
    int l2 = osc__buffer_vl(tmp, fmt, vl);
    va_end(vl);
    int l1 = osc__buffer(buf, "s,s", addr, fmt);
    memcpy(buf+l1, tmp, l2);
    int total = l1+l2;

    char *out = va("%*.s", 4+total, "");
    memcpy(out, &total, 4);
    memcpy(out+4, buf, total);
    return out;
}

int osc_bundle( char *buf, uint64_t ts ) {
    return osc__buffer( buf, "sh", "#bundle", ts );
}

#ifdef OSCPACK_DEMO

int main() {
    // OSC message
    {
        char buf[4096];
        int l = osc_pack(buf, "/foo", "iisff", 1000, -1, "hello", 1.234f, 5.678f);
        //use as: udp_send(socket, buf+4, l-4);
        hexdump(buf, l);

        assert( 0 == memcmp( buf, 
        "\x2f\x66\x6f\x6f\x00\x00\x00\x00\x2c\x69\x69\x73\x66\x66\x00\x00"
        "\x00\x00\x03\xe8\xff\xff\xff\xff\x68\x65\x6c\x6c\x6f\x00\x00\x00"
        "\x3f\x9d\xf3\xb6\x40\xb5\xb2\x2d", l-4));
    }

    // OSC message (w/ initial 4-bytes payload)
    {
        char *buf = osc_pack_va("/foo", "iisff", 1000, -1, "hello", 1.234f, 5.678f);
        int l = *(int*)buf;
        printf("---------------%x\n", l);
        hexdump(buf, l);

        assert( 0 == memcmp( buf+4, //"\x28\x00\x00\x00"
        "\x2f\x66\x6f\x6f\x00\x00\x00\x00\x2c\x69\x69\x73\x66\x66\x00\x00"
        "\x00\x00\x03\xe8\xff\xff\xff\xff\x68\x65\x6c\x6c\x6f\x00\x00\x00"
        "\x3f\x9d\xf3\xb6\x40\xb5\xb2\x2d", l-4));
    }

    // OSC bundle
    {
        // OSC bundle test taken from Julien Pommier's oscpkt.hh
        // wr.startBundle();
        // wr.addMessage("/foo").pushInt32(1000,-1).pushStr("hello").pushFloat(1.234f,5.678f);
        // wr.endBundle();

        char buf[4096];
        int h = osc_bundle(buf, 1ULL);
        int m = osc_pack(buf+h, "/foo", "iisff", 1000, -1, "hello", 1.234f, 5.678f);

        int l = h+m;
        hexdump(buf, h+m);

        assert( l == 0x3c-4 );
        assert( 0 == memcmp( buf,
        "\x23\x62\x75\x6e\x64\x6c\x65\x00\x00\x00\x00\x00\x00\x00\x00\x01"
        "\x2f\x66\x6f\x6f\x00\x00\x00\x00\x2c\x69\x69\x73"
        "\x66\x66\x00\x00\x00\x00\x03\xe8\xff\xff\xff\xff\x68\x65\x6c\x6c"
        "\x6f\x00\x00\x00\x3f\x9d\xf3\xb6\x40\xb5\xb2\x2d", l) );
    }
}
#endif
