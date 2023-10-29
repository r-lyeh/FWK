// base64 de/encoder. Based on code by Jon Mayo - November 13, 2003 (PUBLIC DOMAIN).
// - rlyeh, public domain

#ifndef BASE64_H
#define BASE64_H

unsigned base64_bounds(unsigned size);
char*    base64_encode(const void *inp, unsigned inlen); // free() after use
char*    base64_decode(const char *inp, unsigned inlen); // array_free() after use

#endif

#ifdef BASE64_C
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>

#define BASE64_ENCODE_OUT_SIZE(s) ((unsigned int)((((s) + 2) / 3) * 4 + 1))
#define BASE64_DECODE_OUT_SIZE(s) ((unsigned int)(((s) / 4) * 3))

unsigned base64_bounds(unsigned size) {
    return BASE64_ENCODE_OUT_SIZE(size);
}

char* base64_encode(const void *inp, unsigned inlen) { // free() after use
    unsigned outlen = base64_bounds(inlen);
    char *out_ = MALLOC(outlen);
    out_[outlen] = 0;

    uint_least32_t v;
    unsigned ii, io, rem;
    char *out = (char *)out_;
    const unsigned char *in = (const unsigned char *)inp;
    const uint8_t base64enc_tab[]= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    for(io = 0, ii = 0, v = 0, rem = 0; ii < inlen; ii ++) {
        unsigned char ch;
        ch = in[ii];
        v = (v << 8) | ch;
        rem += 8;
        while (rem >= 6) {
            rem -= 6;
            if (io >= outlen)
                return (FREE(out_), NULL); /* truncation is failure */
            out[io ++] = base64enc_tab[(v >> rem) & 63];
        }
    }
    if (rem) {
        v <<= (6 - rem);
        if (io >= outlen)
            return (FREE(out_), NULL); /* truncation is failure */
        out[io ++] = base64enc_tab[v & 63];
    }
    while(io&3) {
        if(io>=outlen) return (FREE(out_), NULL); /* truncation is failure */
        out[io++]='=';
    }
    if(io>=outlen) return (FREE(out_), NULL); /* no room for null terminator */
    out[io]=0;
    return out_;
}

#ifdef array_resize
array(char) base64_decode(const char *inp, unsigned inlen) { // array_free() after use
#if 0
    unsigned long outlen = BASE64_DECODE_OUT_SIZE(inlen);
    array(char) out_ = 0; array_resize(out_, outlen+1);

    if( base64_decodex((const unsigned char *)inp, (unsigned long)inlen, (unsigned char *)out_, &outlen) != CRYPT_OK ) {
        array_free(out_);
        return 0;
    }

    array_resize(out_, outlen);
    out_[outlen] = 0;
    return out_;
#else
    unsigned outlen = BASE64_DECODE_OUT_SIZE(inlen);
    array(char) out_ = 0; array_resize(out_, outlen);

    // based on code by Jon Mayo - November 13, 2003 (PUBLIC DOMAIN)
    uint_least32_t v;
    unsigned ii, io, rem;
    char *out = (char *)out_;
    const unsigned char *in = (const unsigned char *)inp;
    const uint8_t base64dec_tab[256]= {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
     52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,
    255, 254, 255, 255, 255,   0,   1,   2,   3,   4,   5,   6,
      7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,
     19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
    255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,
     37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
     49,  50,  51, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255 };

    for (io = 0, ii = 0,v = 0, rem = 0; ii < inlen; ii ++) {
        unsigned char ch;
        if (isspace(in[ii]))
            continue;
        if ((in[ii]=='=') || (!in[ii]))
            break; /* stop at = or null character*/
        ch = base64dec_tab[(unsigned char)in[ii]];
        if (ch == 255)
            break; /* stop at a parse error */
        v = (v<<6) | ch;
        rem += 6;
        if (rem >= 8) {
            rem -= 8;
            if (io >= outlen)
                return (array_free(out_), NULL); /* truncation is failure */
            out[io ++] = (v >> rem) & 255;
        }
    }
    if (rem >= 8) {
        rem -= 8;
        if (io >= outlen)
            return (array_free(out_), NULL); /* truncation is failure */
        out[io ++] = (v >> rem) & 255;
    }
    return (array_resize(out_, io), out_);
#endif
}
#endif // array_resize
#endif // BASE64_C
