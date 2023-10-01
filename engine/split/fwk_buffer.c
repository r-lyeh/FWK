// ----------------------------------------------------------------------------
// compression api

static struct zcompressor {
    // id of compressor
    unsigned enumerator;
    // name of compressor
    const char name1, *name4, *name;
    // returns worst case compression estimation for selected flags
    unsigned (*bounds)(unsigned bytes, unsigned flags);
    // returns number of bytes written. 0 if error.
    unsigned (*encode)(const void *in, unsigned inlen, void *out, unsigned outcap, unsigned flags);
    // returns number of excess bytes that will be overwritten when decoding.
    unsigned (*excess)(unsigned flags);
    // returns number of bytes written. 0 if error.
    unsigned (*decode)(const void *in, unsigned inlen, void *out, unsigned outcap);
} zlist[] = {
    { COMPRESS_RAW,     '0', "raw",  "raw",     raw_bounds, raw_encode, raw_excess, raw_decode },
    { COMPRESS_PPP,     'p', "ppp",  "ppp",     ppp_bounds, ppp_encode, ppp_excess, ppp_decode },
    { COMPRESS_ULZ,     'u', "ulz",  "ulz",     ulz_bounds, ulz_encode, ulz_excess, ulz_decode },
    { COMPRESS_LZ4,     '4', "lz4x", "lz4x",    lz4x_bounds, lz4x_encode, lz4x_excess, lz4x_decode },
    { COMPRESS_CRUSH,   'c', "crsh", "crush",   crush_bounds, crush_encode, crush_excess, crush_decode },
    { COMPRESS_DEFLATE, 'd', "defl", "deflate", deflate_bounds, deflate_encode, deflate_excess, deflate_decode },
    { COMPRESS_LZP1,    '1', "lzp1", "lzp1",    lzp1_bounds, lzp1_encode, lzp1_excess, lzp1_decode },
    { COMPRESS_LZMA,    'm', "lzma", "lzma",    lzma_bounds, lzma_encode, lzma_excess, lzma_decode },
    { COMPRESS_BALZ,    'b', "balz", "balz",    balz_bounds, balz_encode, balz_excess, balz_decode },
    { COMPRESS_LZW3,    'w', "lzw3", "lzrw3a",  lzrw3a_bounds, lzrw3a_encode, lzrw3a_excess, lzrw3a_decode },
    { COMPRESS_LZSS,    's', "lzss", "lzss",    lzss_bounds, lzss_encode, lzss_excess, lzss_decode },
    { COMPRESS_BCM,     'B', "bcm",  "bcm",     bcm_bounds, bcm_encode, bcm_excess, bcm_decode },
    { COMPRESS_ZLIB,    'z', "zlib", "zlib",    deflate_bounds, deflatez_encode, deflate_excess, deflatez_decode },
};

enum { COMPRESS_NUM = 14 };

static char *znameof(unsigned flags) {
    static __thread char buf[16];
    snprintf(buf, 16, "%4s.%c", zlist[(flags>>4)&0x0F].name4, "0123456789ABCDEF"[flags&0xF]);
    return buf;
}
unsigned zencode(void *out, unsigned outlen, const void *in, unsigned inlen, unsigned flags) {
    return zlist[(flags >> 4) % COMPRESS_NUM].encode(in, inlen, (uint8_t*)out, outlen, flags & 0x0F);
}
unsigned zdecode(void *out, unsigned outlen, const void *in, unsigned inlen, unsigned flags) {
    return zlist[(flags >> 4) % COMPRESS_NUM].decode((uint8_t*)in, inlen, out, outlen);
}
unsigned zbounds(unsigned inlen, unsigned flags) {
    return zlist[(flags >> 4) % COMPRESS_NUM].bounds(inlen, flags & 0x0F);
}
unsigned zexcess(unsigned flags) {
    return zlist[(flags >> 4) % COMPRESS_NUM].excess(flags & 0x0F);
}

// ----------------------------------------------------------------------------
// BASE92 en/decoder
// THE BEERWARE LICENSE (Revision 42):
// <thenoviceoof> wrote this file. As long as you retain this notice you
// can do whatever you want with this stuff. If we meet some day, and you
// think this stuff is worth it, you can buy me a beer in return
// - Nathan Hwang (thenoviceoof)

unsigned base92_bounds(unsigned inlen) {
    unsigned size = (inlen * 8) % 13, extra_null = 1;
    if(size == 0) return 2 * ((inlen * 8) / 13) + extra_null;
    if(size  < 7) return 2 * ((inlen * 8) / 13) + extra_null + 1;
                  return 2 * ((inlen * 8) / 13) + extra_null + 2;
}

unsigned base92_encode(const void* in, unsigned inlen, void *out, unsigned size) {
    char *res = (char *)out;
    const unsigned char *str = (const unsigned char *)in;
    unsigned int j = 0;          // j for encoded
    unsigned long workspace = 0; // bits holding bin
    unsigned short wssize = 0;   // number of good bits in workspace
    unsigned char c;
    const unsigned char ENCODE_MAPPING[256] = {
        33, 35, 36, 37, 38, 39, 40, 41, 42, 43,
        44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
        54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
        64, 65, 66, 67, 68, 69, 70, 71, 72, 73,
        74, 75, 76, 77, 78, 79, 80, 81, 82, 83,
        84, 85, 86, 87, 88, 89, 90, 91, 92, 93,
        94, 95, 97, 98, 99, 100, 101, 102, 103, 104,
        105, 106, 107, 108, 109, 110, 111, 112, 113, 114,
        115, 116, 117, 118, 119, 120, 121, 122, 123, 124,
        125, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0
    };
    if (inlen) {
        for (unsigned i = 0; i < inlen; i++) {
            workspace = workspace << 8 | str[i];
            wssize += 8;
            if (wssize >= 13) {
                int tmp = (workspace >> (wssize - 13)) & 8191;
                c = ENCODE_MAPPING[(tmp / 91)];
                if (c == 0) return 0; // illegal char
                res[j++] = c;
                c = ENCODE_MAPPING[(tmp % 91)];
                if (c == 0) return 0; // illegal char
                res[j++] = c;
                wssize -= 13;
            }
        }
        // encode a last byte
        if (0 < wssize && wssize < 7) {
            int tmp = (workspace << (6 - wssize)) & 63;  // pad the right side
            c = ENCODE_MAPPING[(tmp)];
            if (c == 0) return 0; // illegal char
            res[j++] = c;
        } else if (7 <= wssize) {
            int tmp = (workspace << (13 - wssize)) & 8191; // pad the right side
            c = ENCODE_MAPPING[(tmp / 91)];
            if (c == 0) return 0; // illegal char
            res[j++] = c;
            c = ENCODE_MAPPING[(tmp % 91)];
            if (c == 0) return 0; // illegal char
            res[j++] = c;
        }
    } else {
        res[j++] = '~';
    }
    // add null byte
    res[j] = 0;
    return j;
}

// this guy expects a null-terminated string
// gives back a non-null terminated string, and properly filled len
unsigned base92_decode(const void* in, unsigned size, void *out, unsigned outlen_unused) {
    const char* str = (const char*)in;
    unsigned char *res = (unsigned char *)out;
    int i, j = 0, b1, b2;
    unsigned long workspace = 0;
    unsigned short wssize = 0;
    const unsigned char DECODE_MAPPING[256] = {
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 0, 255, 1, 2, 3, 4, 5,
        6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
        26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
        36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
        46, 47, 48, 49, 50, 51, 52, 53, 54, 55,
        56, 57, 58, 59, 60, 61, 255, 62, 63, 64,
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        75, 76, 77, 78, 79, 80, 81, 82, 83, 84,
        85, 86, 87, 88, 89, 90, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255
    };

    // handle small cases first
    if (size == 0 || (str[0] == '~' && str[1] == '\0')) {
        res[0] = 0;
        return 1;
    }
    // calculate size
    int len = ((size/2 * 13) + (size%2 * 6)) / 8;
    // handle pairs of chars
    for (i = 0; i + 1 < size; i += 2) {
        b1 = DECODE_MAPPING[(str[i])];
        b2 = DECODE_MAPPING[(str[i+1])];
        workspace = (workspace << 13) | (b1 * 91 + b2);
        wssize += 13;
        while (wssize >= 8) {
            res[j++] = (workspace >> (wssize - 8)) & 255;
            wssize -= 8;
        }
    }
    // handle single char
    if (size % 2 == 1) {
        workspace = (workspace << 6) | DECODE_MAPPING[(str[size - 1])];
        wssize += 6;
        while (wssize >= 8) {
            res[j++] = (workspace >> (wssize - 8)) & 255;
            wssize -= 8;
        }
    }
    //assert(j == len);
    return j;
}

// ----------------------------------------------------------------------------
// COBS en/decoder
// Based on code by Jacques Fortier.
// "Redistribution and use in source and binary forms are permitted, with or without modification."
//
// Consistent Overhead Byte Stuffing is an encoding that removes all 0 bytes from arbitrary binary data.
// The encoded data consists only of bytes with values from 0x01 to 0xFF. This is useful for preparing data for
// transmission over a serial link (RS-232 or RS-485 for example), as the 0 byte can be used to unambiguously indicate
// packet boundaries. COBS also has the advantage of adding very little overhead (at least 1 byte, plus up to an
// additional byte per 254 bytes of data). For messages smaller than 254 bytes, the overhead is constant.
//
// This implementation is designed to be both efficient and robust.
// The decoder is designed to detect malformed input data and report an error upon detection.
//

unsigned cobs_bounds( unsigned len ) {
    return len + ceil(len / 254.0) + 1;
}
unsigned cobs_encode(const void *in, unsigned inlen, void *out, unsigned outlen) {
    const uint8_t *src = (const uint8_t *)in;
    uint8_t *dst = (uint8_t*)out;
    size_t srclen = inlen;

    uint8_t code = 1;
    size_t read_index = 0, write_index = 1, code_index = 0;

    while( read_index < srclen ) {
        if( src[ read_index ] == 0) {
            dst[ code_index ] = code;
            code = 1;
            code_index = write_index++;
            read_index++;
        } else {
            dst[ write_index++ ] = src[ read_index++ ];
            code++;
            if( code == 0xFF ) {
                dst[ code_index ] = code;
                code = 1;
                code_index = write_index++;
            }
        }
    }

    dst[ code_index ] = code;
    return write_index;
}
unsigned cobs_decode(const void *in, unsigned inlen, void *out, unsigned outlen) {
    const uint8_t *src = (const uint8_t *)in;
    uint8_t *dst = (uint8_t*)out;
    size_t srclen = inlen;

    uint8_t code, i;
    size_t read_index = 0, write_index = 0;

    while( read_index < srclen ) {
        code = src[ read_index ];

        if( ((read_index + code) > srclen) && (code != 1) ) {
            return 0;
        }

        read_index++;

        for( i = 1; i < code; i++ ) {
            dst[ write_index++ ] = src[ read_index++ ];
        }
        if( (code != 0xFF) && (read_index != srclen) ) {
            dst[ write_index++ ] = '\0';
        }
    }

    return write_index;
}

#if 0
static
void cobs_test( const char *buffer, int buflen ) {
    char enc[4096];
    int enclen = cobs_encode( buffer, buflen, enc, 4096 );

    char dec[4096];
    int declen = cobs_decode( enc, enclen, dec, 4096 );

    test( enclen >= buflen );
    test( declen == buflen );
    test( memcmp(dec, buffer, buflen) == 0 );

    printf("%d->%d->%d (+%d extra bytes)\n", declen, enclen, declen, enclen - declen);
}
AUTORUN {
    const char *null = 0;
    cobs_test( null, 0 );

    const char empty[] = "";
    cobs_test( empty, sizeof(empty) );

    const char text[] = "hello world\n";
    cobs_test( text, sizeof(text) );

    const char bintext[] = "hello\0\0\0world\n";
    cobs_test( bintext, sizeof(bintext) );

    const char blank[512] = {0};
    cobs_test( blank, sizeof(blank) );

    char longbintext[1024];
    for( int i = 0; i < 1024; ++i ) longbintext[i] = (unsigned char)i;
    cobs_test( longbintext, sizeof(longbintext) );

    assert(~puts("Ok"));
}
#endif

// ----------------------------------------------------------------------------
// netstring en/decoder
// - rlyeh, public domain.

unsigned netstring_bounds(unsigned inlen) {
    return 5 + inlen + 3; // 3 for ;,\0 + 5 if inlen < 100k ; else (unsigned)ceil(log10(inlen + 1))
}
unsigned netstring_encode(const char *in, unsigned inlen, char *out, unsigned outlen) {
//  if(outlen < netstring_bounds(inlen)) return 0;
    sprintf(out, "%u:%.*s,", inlen, inlen, in);
    return strlen(out);
}
unsigned netstring_decode(const char *in, unsigned inlen, char *out, unsigned outlen) {
//  if(outlen < inlen) return 0;
    const char *bak = in;
    sscanf(in, "%u", &outlen);
    while( *++in != ':' );
    memcpy(out, in+1, outlen), out[outlen-1] = 0;
    // return outlen; // number of written bytes
    return (outlen + (in+2 - bak)); // number of read bytes
}

#if 0
AUTORUN {
    // encode
    const char text1[] = "hello world!", text2[] = "abc123";
    unsigned buflen = netstring_bounds(strlen(text1) + strlen(text2));
    char *buf = malloc(buflen), *ptr = buf;
    ptr += netstring_encode(text1, strlen(text1), ptr, buflen -= (ptr - buf));
    ptr += netstring_encode(text2, strlen(text2), ptr, buflen -= (ptr - buf));
    printf("%s -> ", buf);

    // decode
    char out[12];
    unsigned plen = strlen(ptr = buf);
    while(plen > 0) {
        int written = netstring_decode(ptr, plen, out, 12);
        ptr += written;
        plen -= written;
        printf("'%s'(%s)(%d), ", out, ptr, plen );
    }
    puts("");
}
#endif

// ----------------------------------------------------------------------------
// array de/interleaving
// - rlyeh, public domain.
//
// results:
// R0G0B0   R1G1B1   R2G2B2...   -> R0R1R2... B0B1B2... G0G1G2...
// R0G0B0A0 R1G1B1A1 R2G2B2A2... -> R0R1R2... A0A1A2... B0B1B2... G0G1G2...

void *interleave( void *out, const void *list, int list_count, int sizeof_item, unsigned columns ) {
    void *bak = out;
    assert( columns < list_count ); // required
    int row_count = list_count / columns;
    for( int offset = 0; offset < columns; offset++ ) {
        for( int row = 0; row < row_count; row++ ) {
            memcpy( out, &((char*)list)[ (offset + row * columns) * sizeof_item ], sizeof_item );
            out = ((char*)out) + sizeof_item;
        }
    }
    return bak;
}

#if 0
static
void interleave_test( const char *name, int interleaving, int deinterleaving, const char *original ) {
    char interleaved[128] = {0};
    interleave( interleaved, original, strlen(original)/2, 2, interleaving );
    char deinterleaved[128] = {0};
    interleave( deinterleaved, interleaved, strlen(original)/2, 2, deinterleaving );

    printf( "\n%s\n", name );
    printf( "original:\t%s\n", original );
    printf( "interleaved:\t%s\n", interleaved );
    printf( "deinterleaved:\t%s\n", deinterleaved );

    assert( 0 == strcmp(original, deinterleaved) );
}

AUTORUN {
    interleave_test(
        "audio 2ch", 2, 3,
        "L0R0"
        "L1R1"
        "L2R2"
    );
    interleave_test(
        "image 3ch", 3, 3,
        "R0G0B0"
        "R1G1B1"
        "R2G2B2"
    );
    interleave_test(
        "image 4ch", 4, 3,
        "R0G0B0A0"
        "R1G1B1A1"
        "R2G2B2A2"
    );
    interleave_test(
        "audio 5ch", 5, 3,
        "A0B0C0L0R0"
        "A1B1C1L1R1"
        "A2B2C2L2R2"
    );
    interleave_test(
        "audio 5.1ch", 6, 3,
        "A0B0C0L0R0S0"
        "A1B1C1L1R1S1"
        "A2B2C2L2R2S2"
    );
    interleave_test(
        "opengl material 9ch", 9, 3,
        "X0Y0Z0q0w0e0r0u0v0"
        "X1Y1Z1q1w1e1r1u1v1"
        "X2Y2Z2q2w2e2r2u2v2"
    );
    interleave_test(
        "opengl material 10ch", 10, 3,
        "X0Y0Z0q0w0e0r0s0u0v0"
        "X1Y1Z1q1w1e1r1s1u1v1"
        "X2Y2Z2q2w2e2r2s2u2v2"
    );
    assert(~puts("Ok"));
}
#endif

// ----------------------------------------------------------------------------
// delta encoder

#define delta_expand_template(N) \
void delta##N##_encode(void *buffer_, unsigned count) { \
    uint##N##_t current, last = 0, *buffer = (uint##N##_t*)buffer_; \
    for( unsigned i = 0; i < count; i++ ) { \
        current = buffer[i]; \
        buffer[i] = current - last; \
        last = current; \
    } \
} \
void delta##N##_decode(void *buffer_, unsigned count) { \
    uint##N##_t delta, last = 0, *buffer = (uint##N##_t*)buffer_; \
    for( unsigned i = 0; i < count; i++ ) { \
        delta = buffer[i]; \
        buffer[i] = delta + last; \
        last = buffer[i]; \
    } \
}
delta_expand_template(8);
delta_expand_template(16);
delta_expand_template(32);
delta_expand_template(64);

#if 0
AUTORUN {
    char buf[] = "1231112223345555";
    int buflen = strlen(buf);

    char *dt = strdup(buf);
    printf("  delta8: ", dt);
    for( int i = 0; i < buflen; ++i ) printf("%c", dt[i] );
    printf("->");
    delta8_encode(dt, buflen);
    for( int i = 0; i < buflen; ++i ) printf("%02d,", dt[i] );
    printf("->");
    delta8_decode(dt, buflen);
    for( int i = 0; i < buflen; ++i ) printf("%c", dt[i] );
    printf("\r%c\n", 0 == strcmp(buf,dt) ? 'Y':'N');
}
#endif

// ----------------------------------------------------------------------------
// zigzag en/decoder
// - rlyeh, public domain

uint64_t zig64( int64_t value ) { // convert sign|magnitude to magnitude|sign
    return (value >> 63) ^ (value << 1);
}
int64_t zag64( uint64_t value ) { // convert magnitude|sign to sign|magnitude
    return (value >> 1) ^ -(value & 1);
}

// branchless zigzag encoding 32/64
// sign|magnitude to magnitude|sign and back
// [ref] https://developers.google.com/protocol-buffers/docs/encoding
uint32_t enczig32u( int32_t n) { return ((n << 1) ^ (n >> 31)); }
uint64_t enczig64u( int64_t n) { return ((n << 1) ^ (n >> 63)); }
 int32_t deczig32i(uint32_t n) { return ((n >> 1) ^  -(n & 1)); }
 int64_t deczig64i(uint64_t n) { return ((n >> 1) ^  -(n & 1)); }

#if 0
AUTORUN {
    int16_t x = -1000;
    printf("%d -> %llu %llx -> %lld\n", x, zig64(x), zig64(x), zag64(zig64(x)));
}
AUTORUN {
    #define CMP32(signedN) do { \
        int32_t reconverted = deczig32i( enczig32u(signedN) ); \
        int equal = signedN == reconverted; \
        printf("[%s] %d vs %d\n", equal ? " OK " : "FAIL", signedN, reconverted ); \
    } while(0)

    #define CMP64(signedN) do { \
        int64_t reconverted = deczig64i( enczig64u(signedN) ); \
        int equal = signedN == reconverted; \
        printf("[%s] %lld vs %lld\n", equal ? " OK " : "FAIL", signedN, reconverted ); \
    } while(0)

    CMP32( 0);
    CMP32(-1);
    CMP32(+1);
    CMP32(-2);
    CMP32(+2);
    CMP32(INT32_MAX - 1);
    CMP32(INT32_MIN + 1);
    CMP32(INT32_MAX);
    CMP32(INT32_MIN);

    CMP64( 0ll);
    CMP64(-1ll);
    CMP64(+1ll);
    CMP64(-2ll);
    CMP64(+2ll);
    CMP64(INT64_MAX - 1);
    CMP64(INT64_MIN + 1);
    CMP64(INT64_MAX);
    CMP64(INT64_MIN);
}
void TESTU( uint64_t N ) {
    uint8_t buf[9] = {0};
    enczig64i(buf, (N));
    uint64_t reconstructed = deczig64i(buf, 0);
    if( reconstructed != (N) ) printf("[FAIL] %llu vs %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", (N), buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8] );
    else if( 0xffffff == ((N) & 0xffffff) ) printf("[ OK ] %llx\n", (N));
}
void TESTI( int64_t N ) {
    TESTU( enczig64u(N) );
}
AUTORUN {
    TESTU(0LLU);
    TESTU(1LLU);
    TESTU(2LLU);
    TESTU(UINT64_MAX/8);
    TESTU(UINT64_MAX/4);
    TESTU(UINT64_MAX/2);
    TESTU(UINT64_MAX-2);
    TESTU(UINT64_MAX-1);
    TESTU(UINT64_MAX);

   #pragma omp parallel for  // compile with /openmp
   for( int64_t N = INT64_MIN; N < INT64_MAX; ++N ) {
        TESTU(N);
        TESTI((int64_t)N);
   }
}
#endif

// ----------------------------------------------------------------------------
// ARC4 en/decryptor. Based on code by Mike Shaffer.
// - rlyeh, public domain.

void *arc4( void *buf_, unsigned buflen, const void *pass_, unsigned passlen ) {
    // [ref] http://www.4guysfromrolla.com/webtech/code/rc4.inc.html
    assert(passlen);
    int sbox[256], key[256];
    char *buf = (char*)buf_;
    const char *pass = (const char*)pass_;
    for( unsigned a = 0; a < 256; a++ ) {
        key[a] = pass[a % passlen];
        sbox[a] = a;
    }
    for( unsigned a = 0, b = 0; a < 256; a++ ) {
        b = (b + sbox[a] + key[a]) % 256;
        int swap = sbox[a]; sbox[a] = sbox[b]; sbox[b] = swap;
    }
    for( unsigned a = 0, b = 0, i = 0; i < buflen; ++i ) {
        a = (a + 1) % 256;
        b = (b + sbox[a]) % 256;
        int swap = sbox[a]; sbox[a] = sbox[b]; sbox[b] = swap;
        buf[i] ^= sbox[(sbox[a] + sbox[b]) % 256];
    }
    return buf_;
}

#if 0
AUTORUN {
    char buffer[] = "Hello world."; int buflen = strlen(buffer);
    char *password = "abc123"; int passlen = strlen(password);

    printf("Original: %s\n", buffer);
    printf("Password: %s\n", password);

    char *encrypted = arc4( buffer, buflen, password, passlen );
    printf("ARC4 Encrypted text: '%s'\n", encrypted);

    char *decrypted = arc4( buffer, buflen, password, passlen );
    printf("ARC4 Decrypted text: '%s'\n", decrypted);
}
#endif

// ----------------------------------------------------------------------------
// crc64
// - rlyeh, public domain

uint64_t crc64(uint64_t h, const void *ptr, uint64_t len) {
    // based on public domain code by Lasse Collin
    // also, use poly64 0xC96C5795D7870F42 for crc64-ecma
    static uint64_t crc64_table[256];
    static uint64_t poly64 = UINT64_C(0x95AC9329AC4BC9B5);
    if( poly64 ) {
        for( int b = 0; b < 256; ++b ) {
            uint64_t r = b;
            for( int i = 0; i < 8; ++i ) {
                r = r & 1 ? (r >> 1) ^ poly64 : r >> 1;
            }
            crc64_table[ b ] = r;
            //printf("%016llx\n", crc64_table[b]);
        }
        poly64 = 0;
    }
    const uint8_t *buf = (const uint8_t *)ptr;
    uint64_t crc = ~h; // ~crc;
    while( len != 0 ) {
        crc = crc64_table[(uint8_t)crc ^ *buf++] ^ (crc >> 8);
        --len;
    }
    return ~crc;
}

#if 0
unsigned crc32(unsigned h, const void *ptr_, unsigned len) {
    // based on public domain code by Karl Malbrain
    const uint8_t *ptr = (const uint8_t *)ptr_;
    if (!ptr) return 0;
    const unsigned tbl[16] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c, 0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c };
    for(h = ~h; len--; ) { uint8_t b = *ptr++; h = (h >> 4) ^ tbl[(h & 15) ^ (b & 15)]; h = (h >> 4) ^ tbl[(h & 15) ^ (b >> 4)]; }
    return ~h;
}
#endif

// ----------------------------------------------------------------------------
// entropy encoder

#if is(win32)
#include <winsock2.h>
#include <wincrypt.h>
#pragma comment(lib, "advapi32")

void entropy( void *buf, unsigned n ) {
    HCRYPTPROV provider;
    if( CryptAcquireContext( &provider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) == 0 ) {
        assert(!"CryptAcquireContext failed");
    }

    int rc = CryptGenRandom( provider, n, (BYTE *)buf );
    assert( rc != 0 );
    CryptReleaseContext( provider, 0 );
}

#elif is(linux) || is(osx)

void entropy( void *buf, unsigned n ) {
    FILE *fp = fopen( "/dev/urandom", "r" );
    if( !fp ) assert(!"/dev/urandom open failed");

    size_t read = fread( buf, 1, n, fp );
    assert( read == n && "/dev/urandom read failed" );
    fclose( fp );
}

#else // unused for now. likely emscripten will hit this

// pseudo random number generator with 128 bit internal state... probably not suited for cryptographical usage.
// [src] http://github.com/kokke (UNLICENSE)
// [ref] http://burtleburtle.net/bob/rand/smallprng.html

#include <time.h>

#if is(win32)
#include <process.h>
#else
#include <unistd.h>
#endif

static uint32_t prng_next(void) {
    #define prng_rotate(x,k) (x << k) | (x >> (32 - k))
    #define prng_shuffle() do { \
    uint32_t e = ctx[0] - prng_rotate(ctx[1], 27); \
    ctx[0] = ctx[1] ^ prng_rotate(ctx[2], 17); \
    ctx[1] = ctx[2] + ctx[3]; \
    ctx[2] = ctx[3] + e; \
    ctx[3] = e + ctx[0]; } while(0)
    static __thread uint32_t ctx[4], *once = 0; if( !once ) {
        uint32_t seed = (uint32_t)( ifdef(win32,_getpid,getpid)() + time(0) + ((uintptr_t)once) );
        ctx[0] = 0xf1ea5eed;
        ctx[1] = ctx[2] = ctx[3] = seed;
        for (int i = 0; i < 31; ++i) {
            prng_shuffle();
        }
        once = ctx;
    }
    prng_shuffle();
    return ctx[3];
}

void entropy( void *buf, unsigned n ) {
    for( ; n >= 4 ; n -= 4 ) {
        uint32_t a = prng_next();
        memcpy(buf, &a, 4);
        buf = ((char*)buf) + 4;
    }
    if( n > 0 ) {
        uint32_t a = prng_next();
        memcpy(buf, &a, n);
    }
}

#endif

#if 0
AUTORUN {
    unsigned char buf[128];
    entropy(buf, 128);
    for( int i = 0; i < 128; ++i ) {
        printf("%02x", buf[i]);
    }
    puts("");
}
#endif
