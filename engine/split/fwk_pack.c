// -----------------------------------------------------------------------------
// semantic versioning in a single byte (octal)
// - rlyeh, public domain.
//
// - single octal byte that represents semantic versioning (major.minor.patch).
// - allowed range [0000..0377] ( <-> [0..255] decimal )
// - comparison checks only major.minor tuple as per convention.

int semver( int major, int minor, int patch ) {
    return SEMVER(major, minor, patch);
}
int semvercmp( int v1, int v2 ) {
    return SEMVERCMP(v1, v2);
}

#if 0
AUTORUN {
    for( int i= 0; i <= 255; ++i) printf(SEMVERFMT ",", i);
    puts("");

    printf(SEMVERFMT "\n", semver(3,7,7));
    printf(SEMVERFMT "\n", semver(2,7,7));
    printf(SEMVERFMT "\n", semver(1,7,7));
    printf(SEMVERFMT "\n", semver(0,7,7));

    printf(SEMVERFMT "\n", semver(3,7,1));
    printf(SEMVERFMT "\n", semver(2,5,3));
    printf(SEMVERFMT "\n", semver(1,3,5));
    printf(SEMVERFMT "\n", semver(0,1,7));

    assert( semvercmp( 0357, 0300 )  > 0 );
    assert( semvercmp( 0277, 0300 )  < 0 );
    assert( semvercmp( 0277, 0200 )  > 0 );
    assert( semvercmp( 0277, 0100 )  < 0 );
    assert( semvercmp( 0076, 0070 ) == 0 );
    assert( semvercmp( 0076, 0077 ) == 0 );
    assert( semvercmp( 0176, 0170 ) == 0 );
    assert( semvercmp( 0176, 0177 ) == 0 );
    assert( semvercmp( 0276, 0270 ) == 0 );
    assert( semvercmp( 0276, 0277 ) == 0 );
    assert( semvercmp( 0376, 0370 ) == 0 );
    assert( semvercmp( 0376, 0377 ) == 0 );
}
#endif

// -----------------------------------------------------------------------------
// compile-time fourcc, eightcc

char *cc4str(unsigned x) {
    static __thread char type[4+1] = {0};
    type[3] = (x >> 24ULL) & 255;
    type[2] = (x >> 16ULL) & 255;
    type[1] = (x >>  8ULL) & 255;
    type[0] = (x >>  0ULL) & 255;
    return type;
}
char *cc8str(uint64_t x) {
    static __thread char type[8+1] = {0};
    type[7] = (x >> 56ULL) & 255;
    type[6] = (x >> 48ULL) & 255;
    type[5] = (x >> 40ULL) & 255;
    type[4] = (x >> 32ULL) & 255;
    type[3] = (x >> 24ULL) & 255;
    type[2] = (x >> 16ULL) & 255;
    type[1] = (x >>  8ULL) & 255;
    type[0] = (x >>  0ULL) & 255;
    return type;
}

// ----------------------------------------------------------------------------
// float conversion (text)

char* itoa1(int v) {
    return va("%d", v);
}
char* itoa2(vec2i v) {
    return va("%d,%d", v.x,v.y);
}
char* itoa3(vec3i v) {
    return va("%d,%d,%d", v.x,v.y,v.z);
}

char* ftoa1(float v) {
    return va("%f", v);
}
char* ftoa2(vec2 v) {
    return va("%f,%f", v.x, v.y);
}
char* ftoa3(vec3 v) {
    return va("%f,%f,%f", v.x, v.y, v.z);
}
char* ftoa4(vec4 v) {
    return va("%f,%f,%f,%f", v.x, v.y, v.z, v.w);
}

float atof1(const char *s) {
    char buf[64];
    return sscanf(s, "%63[^]\r\n,}]", buf) == 1 ? (float)eval(buf) : (float)NAN;
}
vec2 atof2(const char *s) {
    vec2 v = { 0 };
    char buf1[64],buf2[64];
    int num = sscanf(s, "%63[^]\r\n,}],%63[^]\r\n,}]", buf1, buf2);
    if( num > 0 ) v.x = eval(buf1);
    if( num > 1 ) v.y = eval(buf2);
    return v;
}
vec3 atof3(const char *s) {
    vec3 v = {0};
    char buf1[64],buf2[64],buf3[64];
    int num = sscanf(s, "%63[^]\r\n,}],%63[^]\r\n,}],%63[^]\r\n,}]", buf1, buf2, buf3);
    if( num > 0 ) v.x = eval(buf1);
    if( num > 1 ) v.y = eval(buf2);
    if( num > 2 ) v.z = eval(buf3);
    return v;
}
vec4 atof4(const char *s) {
    vec4 v = {0};
    char buf1[64],buf2[64],buf3[64],buf4[64];
    int num = sscanf(s, "%63[^]\r\n,}],%63[^]\r\n,}],%63[^]\r\n,}],%63[^]\r\n,}]", buf1, buf2, buf3, buf4);
    if( num > 0 ) v.x = eval(buf1);
    if( num > 1 ) v.y = eval(buf2);
    if( num > 2 ) v.z = eval(buf3);
    if( num > 3 ) v.w = eval(buf4);
    return v;
}

// @todo: expand this to proper int parsers
int atoi1(const char *s) {
    return (int)atof1(s);
}
vec2i atoi2(const char *s) {
    vec2 v = atof2(s);
    return vec2i( v.x, v.y );
}
vec3i atoi3(const char *s) {
    vec3 v = atof3(s);
    return vec3i( v.x, v.y, v.z );
}

// endianness -----------------------------------------------------------------
// - rlyeh, public domain

int is_big() { return IS_BIG; }
int is_little() { return IS_LITTLE; }

uint16_t  lil16(uint16_t n) { return IS_BIG     ? swap16(n) : n; }
uint32_t  lil32(uint32_t n) { return IS_BIG     ? swap32(n) : n; }
uint64_t  lil64(uint64_t n) { return IS_BIG     ? swap64(n) : n; }
uint16_t  big16(uint16_t n) { return IS_LITTLE  ? swap16(n) : n; }
uint32_t  big32(uint32_t n) { return IS_LITTLE  ? swap32(n) : n; }
uint64_t  big64(uint64_t n) { return IS_LITTLE  ? swap64(n) : n; }

float     lil32f(float n)  { return IS_BIG     ? swap32f(n) : n; }
double    lil64f(double n) { return IS_BIG     ? swap64f(n) : n; }
float     big32f(float n)  { return IS_LITTLE  ? swap32f(n) : n; }
double    big64f(double n) { return IS_LITTLE  ? swap64f(n) : n; }

uint16_t* lil16p(void *p, int sz)  { if(IS_BIG    ) { uint16_t *n = (uint16_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap16(n[i]); } return p; }
uint16_t* big16p(void *p, int sz)  { if(IS_LITTLE ) { uint16_t *n = (uint16_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap16(n[i]); } return p; }
uint32_t* lil32p(void *p, int sz)  { if(IS_BIG    ) { uint32_t *n = (uint32_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap32(n[i]); } return p; }
uint32_t* big32p(void *p, int sz)  { if(IS_LITTLE ) { uint32_t *n = (uint32_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap32(n[i]); } return p; }
uint64_t* lil64p(void *p, int sz)  { if(IS_BIG    ) { uint64_t *n = (uint64_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap64(n[i]); } return p; }
uint64_t* big64p(void *p, int sz)  { if(IS_LITTLE ) { uint64_t *n = (uint64_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap64(n[i]); } return p; }

float   * lil32pf(void *p, int sz) { if(IS_BIG    ) { float    *n = (float    *)p; for(int i = 0; i < sz; ++i) n[i] = swap32f(n[i]); } return p; }
float   * big32pf(void *p, int sz) { if(IS_LITTLE ) { float    *n = (float    *)p; for(int i = 0; i < sz; ++i) n[i] = swap32f(n[i]); } return p; }
double  * lil64pf(void *p, int sz) { if(IS_BIG    ) { double   *n = (double   *)p; for(int i = 0; i < sz; ++i) n[i] = swap64f(n[i]); } return p; }
double  * big64pf(void *p, int sz) { if(IS_LITTLE ) { double   *n = (double   *)p; for(int i = 0; i < sz; ++i) n[i] = swap64f(n[i]); } return p; }

#if !is(cl) && !is(gcc)
uint16_t (swap16)( uint16_t x ) { return (x << 8) | (x >> 8); }
uint32_t (swap32)( uint32_t x ) { x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff); return (x << 16) | (x >> 16); }
uint64_t (swap64)( uint64_t x ) { x = ((x <<  8) & 0xff00ff00ff00ff00ULL) | ((x >>  8) & 0x00ff00ff00ff00ffULL); x = ((x << 16) & 0xffff0000ffff0000ULL) | ((x >> 16) & 0x0000ffff0000ffffULL); return (x << 32) | (x >> 32); }
#endif

float    swap32f(float n)  { union { float  t; uint32_t i; } conv; conv.t = n; conv.i = swap32(conv.i); return conv.t; }
double   swap64f(double n) { union { double t; uint64_t i; } conv; conv.t = n; conv.i = swap64(conv.i); return conv.t; }

void swapf(float *a, float *b) {
    float t = *a; *a = *b; *b = *a;
}
void swapf2(vec2 *a, vec2 *b) {
    float x = a->x; a->x = b->x; b->x = a->x;
    float y = a->y; a->y = b->y; b->y = a->y;
}
void swapf3(vec3 *a, vec3 *b) {
    float x = a->x; a->x = b->x; b->x = a->x;
    float y = a->y; a->y = b->y; b->y = a->y;
    float z = a->z; a->z = b->z; b->z = a->z;
}
void swapf4(vec4 *a, vec4 *b) {
    float x = a->x; a->x = b->x; b->x = a->x;
    float y = a->y; a->y = b->y; b->y = a->y;
    float z = a->z; a->z = b->z; b->z = a->z;
    float w = a->w; a->w = b->w; b->w = a->w;
}

// half packing -----------------------------------------------------------------
// from GingerBill's gbmath.h (public domain)

float half_to_float(half value) {
    union { unsigned int i; float f; } result;
    int s = (value >> 15) & 0x001;
    int e = (value >> 10) & 0x01f;
    int m =  value        & 0x3ff;

    if (e == 0) {
        if (m == 0) {
            /* Plus or minus zero */
            result.i = (unsigned int)(s << 31);
            return result.f;
        } else {
            /* Denormalized number */
            while (!(m & 0x00000400)) {
                m <<= 1;
                e -=  1;
            }
            e += 1;
            m &= ~0x00000400;
        }
    } else if (e == 31) {
        if (m == 0) {
            /* Positive or negative infinity */
            result.i = (unsigned int)((s << 31) | 0x7f800000);
            return result.f;
        } else {
            /* Nan */
            result.i = (unsigned int)((s << 31) | 0x7f800000 | (m << 13));
            return result.f;
        }
    }

    e = e + (127 - 15);
    m = m << 13;

    result.i = (unsigned int)((s << 31) | (e << 23) | m);
    return result.f;
}

half float_to_half(float value) {
    union { unsigned int i; float f; } v;
    int i, s, e, m;

    v.f = value;
    i = (int)v.i;

    s =  (i >> 16) & 0x00008000;
    e = ((i >> 23) & 0x000000ff) - (127 - 15);
    m =   i        & 0x007fffff;

    if (e <= 0) {
        if (e < -10) return (half)s;
        m = (m | 0x00800000) >> (1 - e);

        if (m & 0x00001000)
            m += 0x00002000;

        return (half)(s | (m >> 13));
    } else if (e == 0xff - (127 - 15)) {
        if (m == 0) {
            return (half)(s | 0x7c00); /* NOTE(bill): infinity */
        } else {
            /* NOTE(bill): NAN */
            m >>= 13;
            return (half)(s | 0x7c00 | m | (m == 0));
        }
    } else {
        if (m & 0x00001000) {
            m += 0x00002000;
            if (m & 0x00800000) {
                m = 0;
                e += 1;
            }
        }
        if (e > 30) {
            float volatile f = 1e12f;
            int j;
            for (j = 0; j < 10; j++)
                f *= f; /* NOTE(bill): Cause overflow */
            return (half)(s | 0x7c00);
        }
        return (half)(s | (e << 10) | (m >> 13));
    }
}

// int packing -----------------------------------------------------------------
// - rlyeh, public domain

// pack16i() -- store a 16-bit int into a char buffer (like htons())
// pack32i() -- store a 32-bit int into a char buffer (like htonl())
// pack64i() -- store a 64-bit int into a char buffer (like htonl())

void pack16i(uint8_t *buf, uint16_t i, int swap) {
    if( swap ) i = swap16(i);
    memcpy( buf, &i, sizeof(i) );
}

void pack32i(uint8_t *buf, uint32_t i, int swap) {
    if( swap ) i = swap32(i);
    memcpy( buf, &i, sizeof(i) );
}

void pack64i(uint8_t *buf, uint64_t i, int swap) {
    if( swap ) i = swap64(i);
    memcpy( buf, &i, sizeof(i) );
}

// unpack16i() -- unpack a 16-bit int from a char buffer (like ntohs())
// unpack32i() -- unpack a 32-bit int from a char buffer (like ntohl())
// unpack64i() -- unpack a 64-bit int from a char buffer (like ntohl())
// changes unsigned numbers to signed if needed.

int16_t unpack16i(const uint8_t *buf, int swap) {
    uint16_t i;
    memcpy(&i, buf, sizeof(i));
    if( swap ) i = swap16(i);
    return i <= 0x7fffu ? (int16_t)i : -1 -(uint16_t)(0xffffu - i);
}

int32_t unpack32i(const uint8_t *buf, int swap) {
    uint32_t i;
    memcpy(&i, buf, sizeof(i));
    if( swap ) i = swap32(i);
    return i <= 0x7fffffffu ? (int32_t)i : -1 -(int32_t)(0xffffffffu - i);
}

int64_t unpack64i(const uint8_t *buf, int swap) {
    uint64_t i;
    memcpy(&i, buf, sizeof(i));
    if( swap ) i = swap64(i);
    return i <= 0x7fffffffffffffffull ? (int64_t)i : -1 -(int64_t)(0xffffffffffffffffull - i);
}

// ----------------------------------------------------------------------------
// float un/packing: 8 (micro), 16 (half), 32 (float), 64 (double) types
// - rlyeh, public domain. original code by Beej.us (PD).
//
// [src] http://beej.us/guide/bgnet/output/html/multipage/advanced.html#serialization
// Modified to encode NaN and Infinity as well.
//
// [1] http://www.mrob.com/pub/math/floatformats.html#minifloat
// [2] microfloat: [0.002 to 240] range.
// [3] half float: can approximate any 16-bit unsigned integer or its reciprocal to 3 decimal places.

uint64_t pack754(long double f, unsigned bits, unsigned expbits) {
    long double fnorm;
    int shift;
    long long sign, exp, significand;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit

    if (f == 0.0) return 0; // get this special case out of the way
//< @r-lyeh beware! works for 32/64 only
    else if (f ==  INFINITY) return 0x7f800000ULL << (bits - 32); // 0111 1111 1000
    else if (f == -INFINITY) return 0xff800000ULL << (bits - 32);
    else if (f != f)         return 0x7fc00000ULL << (bits - 32); // 0111 1111 1100 NaN
//< @r-lyeh

    // check sign and begin normalization
    if (f < 0) { sign = 1; fnorm = -f; }
    else { sign = 0; fnorm = f; }

    // get the normalized form of f and track the exponent
    shift = 0;
    while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
    while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
    fnorm = fnorm - 1.0;

    // calculate the binary form (non-float) of the significand data
    significand = fnorm * ((1LL<<significandbits) + 0.5f);

    // get the biased exponent
    exp = shift + ((1<<(expbits-1)) - 1); // shift + bias

    // return the final answer
    return (sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand;
}

long double unpack754(uint64_t i, unsigned bits, unsigned expbits) {
    long double result;
    long long shift;
    unsigned bias;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit

    if (i == 0) return 0.0;
//< @r-lyeh beware! works for 32 only
    else if (i == 0x7fc00000ULL) return  NAN;      //  NaN
    else if (i == 0x7f800000ULL) return  INFINITY; // +Inf
    else if (i == 0xff800000ULL) return -INFINITY; // -Inf
//< @r-lyeh

    // pull the significand
    result = (i&((1LL<<significandbits)-1)); // mask
    result /= (1LL<<significandbits); // convert back to float
    result += 1.0f; // add the one back on

    // deal with the exponent
    bias = (1<<(expbits-1)) - 1;
    shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
    while(shift > 0) { result *= 2.0; shift--; }
    while(shift < 0) { result /= 2.0; shift++; }

    // sign it
    result *= (i>>(bits-1))&1? -1.0: 1.0;

    return result;
}

typedef int static_assert_flt[ sizeof(float) == 4 ];
typedef int static_assert_dbl[ sizeof(double) == 8 ];

// ----------------------------------------------------------------------------
// variable-length integer packing
// - rlyeh, public domain.
//
// 7 [0 xxxx xxx]                        7-bit value in  1 byte  (0-                127)
// 7 [10 xxx xxx] [yyyyyyyy]            14-bit value in  2 bytes (0-             16,383)
// 6 [110 xx xxx] [yyyyyyyy] [zzzzzzzz] 21-bit value in  3 bytes (0-          2,097,151)
// 8 [111 00 xxx] [ 3 bytes]            27-bit value in  4 bytes (0-        134,217,727)
// 8 [111 01 xxx] [ 4 bytes]            35-bit value in  5 bytes (0-     34,359,738,367)
// 5 [111 10 xxx] [ 5 bytes]            43-bit value in  6 bytes (0-  8,796,093,022,207)
// 8 [111 11 000] [ 6 bytes]            48-bit value in  7 bytes (0-281,474,976,710,655)
// 8 [111 11 001] [ 7 bytes]            56-bit value in  8 bytes (...)
// 8 [111 11 010] [ 8 bytes]            64-bit value in  9 bytes
// 8 [111 11 011] [ 9 bytes]            72-bit value in 10 bytes
// 8 [111 11 100] [10 bytes]            80-bit value in 11 bytes
// A [111 11 101] [12 bytes]            96-bit value in 13 bytes
// A [111 11 110] [14 bytes]           112-bit value in 15 bytes
// A [111 11 111] [16 bytes]           128-bit value in 17 bytes
// 1 1 2 3 = 7

uint64_t pack64uv( uint8_t *buffer, uint64_t value ) {
#if 1 // LEB128
    /* encode unsigned : 7-bit pack. MSB terminates stream */
    const uint8_t *buffer0 = buffer;
    while( value > 127 ) {
        *buffer++ = value | 0x80; // (uint8_t)(( value & 0xFF ) | 0x80 );
        value >>= 7;
    }
    *buffer++ = value;
    return buffer - buffer0;
#else
    #define ADD(bits) *buffer++ = (value >>= bits)
    if( value < (1ull<< 7) ) return *buffer = value, 1;
    if( value < (1ull<<14) ) return *buffer++ = 0x80|(value&0x3f), ADD(6), 2;
    if( value < (1ull<<21) ) return *buffer++ = 0xc0|(value&0x1f), ADD(5), ADD(8), 3;
    if( value < (1ull<<27) ) return *buffer++ = 0xe0|(value&0x07), ADD(3), ADD(8), ADD(8), 4;
    if( value < (1ull<<35) ) return *buffer++ = 0xe8|(value&0x07), ADD(3), ADD(8), ADD(8), ADD(8), 5;
    if( value < (1ull<<43) ) return *buffer++ = 0xf0|(value&0x07), ADD(3), ADD(8), ADD(8), ADD(8), ADD(8), 6;
    if( value < (1ull<<48) ) return *buffer++ = 0xf8|(value&0x00), ADD(0), ADD(8), ADD(8), ADD(8), ADD(8), ADD(8), 7;
    if( value < (1ull<<56) ) return *buffer++ = 0xf9|(value&0x00), ADD(0), ADD(8), ADD(8), ADD(8), ADD(8), ADD(8), ADD(8), 8;
  /*if( value < (1ull<<64))*/return *buffer++ = 0xfa|(value&0x00), ADD(0), ADD(8), ADD(8), ADD(8), ADD(8), ADD(8), ADD(8), ADD(8), 9;
  /*...*/
    #undef ADD
#endif
}
uint64_t unpack64uv( const uint8_t *buffer, uint64_t *value ) {
#if 1 // LEB128
    /* decode unsigned : 7-bit unpack. MSB terminates stream */
    const uint8_t *buffer0 = buffer;
    uint64_t out = 0, j = -7;
    do {
        out |= ( ((uint64_t)*buffer) & 0x7f) << (j += 7);
    } while( (*buffer++) & 0x80 );
    return buffer - buffer0;
#else
    uint64_t bytes, out = 0, shift = 0;
    const int table[] = { 6,7,8,9,10,12,14,16 };
    /**/ if( *buffer >= 0xf8 ) bytes = table[*buffer - 0xf8];
    else if( *buffer >= 0xe0 ) bytes = 3 + ((*buffer>>3) & 3);
    else if( *buffer >= 0xc0 ) bytes = 2;
    else                       bytes = *buffer >= 0x80;

    #define POP(bits) out = out | (uint64_t)*buffer++ << (shift += bits);
    switch( bytes ) {
        default:
        break; case 0: out = *buffer++;
        break; case 1: out = *buffer++ & 0x3f; POP(6);
        break; case 2: out = *buffer++ & 0x1f; POP(5); POP(8);
        break; case 3: out = *buffer++ & 0x07; POP(3); POP(8); POP(8);
        break; case 4: out = *buffer++ & 0x07; POP(3); POP(8); POP(8); POP(8);
        break; case 5: out = *buffer++ & 0x07; POP(3); POP(8); POP(8); POP(8); POP(8);
        break; case 6: ++buffer; shift = -8;   POP(8); POP(8); POP(8); POP(8); POP(8); POP(8);
        break; case 7: ++buffer; shift = -8;   POP(8); POP(8); POP(8); POP(8); POP(8); POP(8); POP(8);
        break; case 8: ++buffer; shift = -8;   POP(8); POP(8); POP(8); POP(8); POP(8); POP(8); POP(8); POP(8);
    }
    #undef POP

    return *value = out, bytes+1;
#endif
}

// vbyte, varint (signed)

uint64_t pack64iv( uint8_t *buffer, int64_t value_ ) {
    uint64_t value = (uint64_t)((value_ >> 63) ^ (value_ << 1));
    return pack64uv(buffer, value); /* convert sign|magnitude to magnitude|sign */
}

uint64_t unpack64iv( const uint8_t *buffer, int64_t *value ) {
    uint64_t out = 0, ret = unpack64uv( buffer, &out );
    *value = ((out >> 1) ^ -(out & 1)); /* convert magnitude|sign to sign|magnitude */
    return ret;
}


#if 0
AUTORUN {
    int tests = 0, passes = 0;

    #define testi(v) do { \
        int64_t val = v; \
        char out[16]; \
        int len = pack64iv(out, val); \
        int64_t in = ~val; \
        unpack64iv(out, &in); \
        int ok = val == in; ++tests; passes += ok; \
        printf("%c %02d/%02d (-) %#llx (%llu) <-> %d bytes <-> %#llx (%llu)\n", "NY"[ok], passes, tests, val, val, len, in, in); \
    } while(0)

    #define testu(v) do { \
        uint64_t val = (v); \
        char out[16]; \
        int len = pack64uv(out, val); \
        uint64_t in = ~val; \
        unpack64uv(out, &in); \
        int ok = val == in; ++tests; passes += ok; \
        printf("%c %02d/%02d (+) %#llx (%llu) <-> %d bytes <-> %#llx (%llu)\n", "NY"[ok], passes, tests, val, val, len, in, in); \
    } while(0)

    #define TEST(v) do { testi(v); testu(v); } while(0)

    TEST(0);
    TEST((1ull<<7)-1);
    TEST( 1ull<<7);
    TEST((1ull<<14)-1);
    TEST( 1ull<<14);
    TEST((1ull<<21)-1);
    TEST( 1ull<<21);
    TEST((1ull<<27)-1);
    TEST( 1ull<<27);
    TEST((1ull<<35)-1);
    TEST( 1ull<<35);
    TEST((1ull<<48)-1);
    TEST( 1ull<<48);
    TEST(~0ull-1);
    TEST(~0ull);

    #undef TEST

    printf("%d tests, %d errors\n", tests, tests - passes);
}
#endif

// ----------------------------------------------------------------------------
// msgpack v5, schema based struct/buffer bitpacking
// - rlyeh, public domain.
//
// [ref] https://github.com/msgpack/msgpack/blob/master/spec.md
//
// @todo: finish msgunpack()
// @todo: alt api v3
// int msgpack( uint8_t *buf, const char *fmt, ... );
//   if !buf, bulk size; else pack.
//   returns number of bytes written; 0 if not space enough.
// int msgunpack( const uint8_t *buf, const char *fmt, ... );
//   if !buf, test message; else unpack.
//   returns number of processed bytes; 0 if parse error.


// private alt unpack api v1 {
enum {
    ERR,NIL,BOL,UNS,SIG,STR,BIN,FLT,EXT,ARR,MAP
};
typedef struct variant {
    union {
    uint8_t     chr;
    uint64_t    uns;
    int64_t     sig;
    char       *str;
    void       *bin;
    double      flt;
    uint32_t    u32;
    };
    uint64_t sz;
    uint16_t ext;
    uint16_t type; //[0..10]={err,nil,bol,uns,sig,str,bin,flt,ext,arr,map}
} variant;
bool msgunpack_var(struct variant *var);
// } private alt unpack api v1

struct writer {
    uint8_t *w; // Write pointer into buffer
    size_t len; // Written bytes up to date
    size_t cap; // Buffer capacity
};

struct reader {
    FILE *fp;
    const void *membuf;
    size_t memsize, offset;
    struct variant v; // tmp
};

static __thread struct writer out;
static __thread struct reader in;

static void wrbe(uint64_t n, uint8_t *b) {
#ifndef BIG
    n = ntoh64(n);
#endif
    memcpy(b, &n, sizeof(uint64_t));
}
static int wr(int len, uint8_t opcode, uint64_t value) {
    uint8_t b[8];
    assert((out.len + (len+1) < out.cap) && "buffer overflow!");
    *out.w++ = (opcode);
    /**/ if(len == 1) *out.w++ = (uint8_t)(value);
    else if(len == 2) wrbe(value, b), memcpy(out.w, &b[6], 2), out.w += 2;
    else if(len == 4) wrbe(value, b), memcpy(out.w, &b[4], 4), out.w += 4;
    else if(len == 8) wrbe(value, b), memcpy(out.w, &b[0], 8), out.w += 8;
    out.len += len+1;
    return len+1;
}
static bool rd(void *buf, size_t len, size_t swap) { // return false any error and/or eof
    bool ret;
    if( in.fp ) {
        assert( !ferror(in.fp) && "invalid file handle (reader)" );
        ret = len == fread((char*)buf, 1, len, in.fp);
    } else {
        assert( in.membuf && "invalid memory buffer (reader)");
        assert( (in.offset + len <= in.memsize) && "memory overflow! (reader)");
        ret = !!memcpy(buf, (char*)in.membuf + in.offset, len);
    }
#ifndef BIG
    /**/ if( swap && len == 2 ) *((uint16_t*)buf) = ntoh16(*((uint16_t*)buf));
    else if( swap && len == 4 ) *((uint32_t*)buf) = ntoh32(*((uint32_t*)buf));
    else if( swap && len == 8 ) *((uint64_t*)buf) = ntoh64(*((uint64_t*)buf));
#endif
    return in.offset += len, ret;
}
static bool rdbuf(char **buf, size_t len) { // return false on error or out of memory
    char *ptr = REALLOC(*buf, len+1);
    if( ptr && rd(ptr, len, 0) ) {
        (*buf = ptr)[len] = 0;
    } else {
        FREE(ptr), ptr = 0;
    }
    return !!ptr;
}

int msgpack_new(uint8_t *w, size_t l) {
    out.w = w;
    out.len = 0;
    out.cap = l;
    return w != 0 && l != 0;
}
int msgpack_nil() {
    return wr(0, 0xC0, 0);
}
int msgpack_chr(bool c) {
    return wr(0, c ? 0xC3 : 0xC2, 0);
}
int msgpack_uns(uint64_t n) {
    /**/ if (n < 0x80)           return wr(0, n, 0);
    else if (n < 0x100)          return wr(1, 0xCC, n);
    else if (n < 0x10000)        return wr(2, 0xCD, n);
    else if (n < 0x100000000)    return wr(4, 0xCE, n);
    else                         return wr(8, 0xCF, n);
}
int msgpack_int(int64_t n) {
    /**/ if (n >= 0)             return msgpack_uns(n);
    else if (n >= -32)           return wr(0, n, 0); //wr(0, 0xE0 | n, 0);
    else if (n >= -128)          return wr(1, 0xD0, n + 0xff + 1);
    else if (n >= -32768)        return wr(2, 0xD1, n + 0xffff + 1);
    else if (n >= -2147483648LL) return wr(4, 0xD2, n + 0xffffffffull + 1);
    else                         return wr(8, 0xD3, n + 0xffffffffffffffffull + 1);
}
int msgpack_flt(double g) {
    float f = (float)g;
    double h = f;
    /**/ if(g == h) return wr(4, 0xCA, pack754_32(f));
    else            return wr(8, 0xCB, pack754_64(g));
}
int msgpack_str(const char *s) {
    size_t n = strlen(s), c = n;

    /**/ if (n < 0x20)     c += wr(0, 0xA0 | n, 0);
    else if (n < 0x100)    c += wr(1, 0xD9, n);
    else if (n < 0x10000)  c += wr(2, 0xDA, n);
    else                   c += wr(4, 0xDB, n);

    memcpy(out.w, s, n);
    out.w += n;
    out.len += n;
    return c;
}
int msgpack_bin(const char *s, size_t n) {
    size_t c = n;
    /**/ if (n < 0x100)    c += wr(1, 0xC4, n);
    else if (n < 0x10000)  c += wr(2, 0xC5, n);
    else                   c += wr(4, 0xC6, n);

    memcpy(out.w, s, n);
    out.w += n;
    out.len += n;
    return c;
}
int msgpack_arr(uint32_t numitems) {
    uint32_t n = numitems;
    /**/ if (n < 0x10)    return wr(0, 0x90 | n, 0);
    else if (n < 0x10000) return wr(2, 0xDC, n);
    else                  return wr(4, 0xDD, n);
}
int msgpack_map(uint32_t numpairs) {
    uint32_t n = numpairs;
    /**/ if (n < 0x10)    return wr(0, 0x80 | n, 0);
    else if (n < 0x10000) return wr(2, 0xDE, n);
    else                  return wr(4, 0xDF, n);
}
int msgpack_ext(uint8_t key, void *val, size_t n) {
    uint32_t c = n;
    /**/ if (n == 1)        c += wr(1, 0xD4, key);
    else if (n == 2)        c += wr(1, 0xD5, key);
    else if (n == 4)        c += wr(1, 0xD6, key);
    else if (n == 8)        c += wr(1, 0xD7, key);
    else if (n == 16)       c += wr(1, 0xD8, key);
    else if (n < 0x100)     c += wr(1, 0xC7, n), c += wr(0, key, 0);
    else if (n < 0x10000)   c += wr(2, 0xC8, n), c += wr(0, key, 0);
    else                    c += wr(4, 0xC9, n), c += wr(0, key, 0);

    memcpy(out.w, val, n);
    out.w += n;
    out.len += n;
    return c;
}

bool msgunpack_new( const void *opaque_or_FILE, size_t bytes ) {
    return !!((memset(&in, 0, sizeof(in)), in.memsize = bytes) ? (in.membuf = opaque_or_FILE) : (in.fp = (FILE*)opaque_or_FILE));
}
bool msgunpack_eof() {
    return in.fp ? !!feof(in.fp) : (in.offset > in.memsize);
}
bool msgunpack_err() {
    return in.fp ? !!ferror(in.fp) : !in.memsize;
}
bool msgunpack_var(struct variant *w) {
    uint8_t tag;
    struct variant v = {0};
    if( rd(&tag, 1, 0) )
    switch(tag) {
        default:
        /**/ if((tag & 0x80) == 0x00) { v.type = UNS; v.sz = 1; v.uns =         tag; }
        else if((tag & 0xe0) == 0xe0) { v.type = SIG; v.sz = 1; v.sig = (int8_t)tag; }
        else if((tag & 0xe0) == 0xa0) { v.type = rdbuf(&v.str, v.sz = tag & 0x1f) ? STR : ERR; }
        else if((tag & 0xf0) == 0x90) { v.type = ARR; v.sz = tag & 0x0f; }
        else if((tag & 0xf0) == 0x80) { v.type = MAP; v.sz = tag & 0x0f; }

        break; case 0xc0: v.type = NIL; v.sz = 0;
        break; case 0xc2: v.type = BOL; v.sz = 1; v.chr = 0;
        break; case 0xc3: v.type = BOL; v.sz = 1; v.chr = 1;

        break; case 0xcc: v.type = rd(&v.uns, v.sz = 1, 0) ? UNS : ERR;
        break; case 0xcd: v.type = rd(&v.uns, v.sz = 2, 1) ? UNS : ERR;
        break; case 0xce: v.type = rd(&v.uns, v.sz = 4, 1) ? UNS : ERR;
        break; case 0xcf: v.type = rd(&v.uns, v.sz = 8, 1) ? UNS : ERR;

        break; case 0xd0: v.type = rd(&v.uns, v.sz = 1, 0) ? (v.sig -= 0xff + 1, SIG) : ERR;
        break; case 0xd1: v.type = rd(&v.uns, v.sz = 2, 1) ? (v.sig -= 0xffff + 1, SIG) : ERR;
        break; case 0xd2: v.type = rd(&v.uns, v.sz = 4, 1) ? (v.sig -= 0xffffffffull + 1, SIG) : ERR;
        break; case 0xd3: v.type = rd(&v.uns, v.sz = 8, 1) ? (v.sig -= 0xffffffffffffffffull + 1, SIG) : ERR;

        break; case 0xca: v.type = rd(&v.u32, v.sz = 4, 1) ? (v.flt = unpack754_32(v.u32), FLT) : ERR;
        break; case 0xcb: v.type = rd(&v.uns, v.sz = 8, 1) ? (v.flt = unpack754_64(v.uns), FLT) : ERR;

        break; case 0xd9: v.type = rd(&v.sz, 1, 0) && rdbuf(&v.str, v.sz) ? STR : ERR;
        break; case 0xda: v.type = rd(&v.sz, 2, 1) && rdbuf(&v.str, v.sz) ? STR : ERR;
        break; case 0xdb: v.type = rd(&v.sz, 4, 1) && rdbuf(&v.str, v.sz) ? STR : ERR;

        break; case 0xc4: v.type = rd(&v.sz, 1, 0) && rdbuf(&v.str, v.sz) ? BIN : ERR;
        break; case 0xc5: v.type = rd(&v.sz, 2, 1) && rdbuf(&v.str, v.sz) ? BIN : ERR;
        break; case 0xc6: v.type = rd(&v.sz, 4, 1) && rdbuf(&v.str, v.sz) ? BIN : ERR;

        break; case 0xdc: v.type = rd(&v.sz, 2, 1) ? ARR : ERR;
        break; case 0xdd: v.type = rd(&v.sz, 4, 1) ? ARR : ERR;

        break; case 0xde: v.type = rd(&v.sz, 2, 1) ? MAP : ERR;
        break; case 0xdf: v.type = rd(&v.sz, 4, 1) ? MAP : ERR;

        break; case 0xd4: v.type = rd(&v.ext, 1, 0) && rd(&v.uns, 1, 0) && rdbuf(&v.str, v.sz = v.uns) ? EXT : ERR;
        break; case 0xd5: v.type = rd(&v.ext, 1, 0) && rd(&v.uns, 2, 1) && rdbuf(&v.str, v.sz = v.uns) ? EXT : ERR;
        break; case 0xd6: v.type = rd(&v.ext, 1, 0) && rd(&v.uns, 4, 1) && rdbuf(&v.str, v.sz = v.uns) ? EXT : ERR;
        break; case 0xd7: v.type = rd(&v.ext, 1, 0) && rd(&v.uns, 8, 1) && rdbuf(&v.str, v.sz = v.uns) ? EXT : ERR;
        break; case 0xd8: v.type = rd(&v.ext, 1, 0) && rd(&v.uns,16, 1) && rdbuf(&v.str, v.sz = v.uns) ? EXT : ERR;

        break; case 0xc7: v.type = rd(&v.sz, 1, 0) && rd(&v.ext, 1, 0) && rdbuf(&v.str,v.sz) ? EXT : ERR;
        break; case 0xc8: v.type = rd(&v.sz, 2, 1) && rd(&v.ext, 1, 1) && rdbuf(&v.str,v.sz) ? EXT : ERR;
        break; case 0xc9: v.type = rd(&v.sz, 4, 1) && rd(&v.ext, 1, 1) && rdbuf(&v.str,v.sz) ? EXT : ERR;
    }
    return *w = v, v.type != ERR;
}
bool msgunpack_nil() {
    return msgunpack_var(&in.v) && (in.v.type == NIL);
}
bool msgunpack_chr(bool *chr) {
    return msgunpack_var(&in.v) && (*chr = in.v.chr, in.v.type == BOL);
}
bool msgunpack_uns(uint64_t *uns) {
    return msgunpack_var(&in.v) && (*uns = in.v.uns, in.v.type == UNS);
}
bool msgunpack_int(int64_t *sig) {
    return msgunpack_var(&in.v) && (*sig = in.v.sig, in.v.type == SIG);
}
bool msgunpack_flt(float *flt) {
    return msgunpack_var(&in.v) && (*flt = in.v.flt, in.v.type == FLT);
}
bool msgunpack_dbl(double *dbl) {
    return msgunpack_var(&in.v) && (*dbl = in.v.flt, in.v.type == FLT);
}
bool msgunpack_bin(void **bin, uint64_t *len) {
    return msgunpack_var(&in.v) && (*bin = in.v.bin, *len = in.v.sz, in.v.type == BIN);
}
bool msgunpack_str(char **str) {
    return msgunpack_var(&in.v) && (str ? *str = in.v.str, in.v.type == STR : in.v.type == STR);
}
bool msgunpack_ext(uint8_t *key, void **val, uint64_t *len) {
    return msgunpack_var(&in.v) && (*key = in.v.ext, *val = in.v.bin, *len = in.v.sz, in.v.type == EXT);
}
bool msgunpack_arr(uint64_t *len) {
    return msgunpack_var(&in.v) && (*len = in.v.sz, in.v.type == ARR);
}
bool msgunpack_map(uint64_t *len) {
    return msgunpack_var(&in.v) && (*len = in.v.sz, in.v.type == MAP);
}


int msgpack(const char *fmt, ... ) {
    int count = 0;
    va_list vl;
    va_start(vl, fmt);
    while( *fmt ) {
        char f = *fmt++;
        switch( f ) {
            break; case '{': { int i = va_arg(vl, int64_t); count += msgpack_map( i ); }
            break; case '[': { int i = va_arg(vl, int64_t); count += msgpack_arr( i ); }
            break; case 'b': { bool v = !!va_arg(vl, int64_t); count += msgpack_chr(v); }
            break; case 'e': { uint8_t k = va_arg(vl, uint64_t); void *v = va_arg(vl, void*); size_t l = va_arg(vl, uint64_t); count += msgpack_ext( k, v, l ); }
            break; case 'n': { count += msgpack_nil(); }
            break; case 'p': { void *p = va_arg(vl, void*); size_t l = va_arg(vl, uint64_t); count += msgpack_bin( p, l ); }
            break; case 's': { const char *v = va_arg(vl, const char *); count += msgpack_str(v); }
            break; case 'u': { uint64_t v = va_arg(vl, uint64_t); count += msgpack_uns(v); }
            break; case 'd': case 'i': { int64_t v = va_arg(vl, int64_t); count += msgpack_int(v); }
            break; case 'f': case 'g': { double v = va_arg(vl, double); count += msgpack_flt(v); }
            default: /*count = 0;*/ break;
        }
    }
    va_end(vl);
    return count;
}
int msgunpack(const char *fmt, ... ) {
    int count = 0;
    va_list vl;
    va_start(vl, fmt);
    while( *fmt ) {
        char f = *fmt++;
        switch( f ) {
            break; case '{': { int64_t *i = va_arg(vl, int64_t*); count += msgunpack_map( i ); }
            break; case '[': { int64_t *i = va_arg(vl, int64_t*); count += msgunpack_arr( i ); }
            break; case 'f': { float *v = va_arg(vl, float*); count += msgunpack_flt(v); }
            break; case 'g': { double *v = va_arg(vl, double*); count += msgunpack_dbl(v); }
            break; case 's': { char **v = va_arg(vl, char **); count += msgunpack_str(v); }
//          break; case 'b': { bool *v = !!va_arg(vl, bool*); count += msgunpack_chr(v); }
//          break; case 'e': { uint8_t k = va_arg(vl, uint64_t); void *v = va_arg(vl, void*); size_t l = va_arg(vl, uint64_t); count += msgunpack_ext( k, v, l ); }
//          break; case 'n': { count += msgunpack_nil(); }
            break; case 'p': { void *p = va_arg(vl, void*); uint64_t l = va_arg(vl, uint64_t); count += msgunpack_bin( p, &l ); }
//          break; case 'u': { uint64_t v = va_arg(vl, uint64_t); count += msgunpack_uns(v); }
//          break; case 'd': case 'i': { int64_t v = va_arg(vl, int64_t); count += msgunpack_int(v); }
            default: /*count = 0;*/ break;
        }
    }
    va_end(vl);
    return count;
}

#if 0
AUTORUN {
#   define unit(title)
#   define data(data) msgunpack_new(data, sizeof(data) -1 )
#   define TEST(expr) test(msgunpack_var(&obj) && !!(expr))

    int test_len;
    const char *test_data = 0;
    struct variant obj = {0};

    /*
     * Test vectors are derived from
     * `https://github.com/ludocode/mpack/blob/v0.8.2/test/test-write.c`.
     */

    unit("(minposfixint)");
    data("\x00");
    TEST(obj.type == UNS && obj.sz == 1 && obj.uns == 0);

    unit("(maxposfixint)");
    data("\x7f");
    TEST(obj.type == UNS && obj.sz == 1 && obj.uns == 127);

    unit("(maxnegfixint)");
    data("\xe0");
    TEST(obj.type == SIG && obj.sz == 1 && obj.uns == -32);

    unit("(minnegfixint)");
    data("\xff");
    TEST(obj.type == SIG && obj.sz == 1 && obj.uns == -1);

    unit("(uint8)");
    data("\xcc\0");
    TEST(obj.type == UNS && obj.sz == 1 && obj.uns == 0);

    unit("(uint16)");
    data("\xcd\0\0");
    TEST(obj.type == UNS && obj.sz == 2 && obj.uns == 0);

    unit("(uint32)");
    data("\xce\0\0\0\0");
    TEST(obj.type == UNS && obj.sz == 4 && obj.uns == 0);

    unit("(uint64)");
    data("\xcf\0\0\0\0\0\0\0\0");
    TEST(obj.type == UNS && obj.sz == 8 && obj.uns == 0);

    unit("(float32)");
    data("\xca\0\0\0\0");
    TEST(obj.type == FLT && obj.sz == 4 && obj.uns == 0);

    unit("(float64)");
    data("\xcb\0\0\0\0\0\0\0\0");
    TEST(obj.type == FLT && obj.sz == 8 && obj.uns == 0);

    unit("(string)");
    data("\xa5Hello");
    TEST(obj.type == STR && obj.sz == 5 && !strcmp(obj.str, "Hello"));

    unit("(str8)");
    data("\xd9\x05Hello");
    TEST(obj.type == STR && obj.sz == 5 && !strcmp(obj.str, "Hello"));

    unit("(str16)");
    data("\xda\0\x05Hello");
    TEST(obj.type == STR && obj.sz == 5 && !strcmp(obj.str, "Hello"));

    unit("(str32)");
    data("\xdb\0\0\0\x05Hello");
    TEST(obj.type == STR && obj.sz == 5 && !strcmp(obj.str, "Hello"));

    unit("(array)");
    data("\x91\x01");
    TEST(obj.type == ARR && obj.sz == 1);

    unit("(array8)");
    data("\x91\x01");
    TEST(obj.type == ARR && obj.sz == 1);

    unit("(array16)");
    data("\xdc\0\x01\x01");
    TEST(obj.type == ARR && obj.sz == 1);

    unit("(map8)");
    data("\x81\x01\x01");
    TEST(obj.type == MAP && obj.sz == 1);
    TEST(obj.type == UNS && obj.sz == 1 && obj.uns == 1);
    TEST(obj.type == UNS && obj.sz == 1 && obj.uns == 1);

    unit("(map32)");
    data("\xdf\0\0\0\x01\xa5Hello\x01");
    TEST(obj.type == MAP && obj.sz == 1);
    TEST(obj.type == STR && obj.sz == 5 && !strcmp(obj.str, "Hello"));
    TEST(obj.type == UNS && obj.sz == 1 && obj.uns == 1);

    unit("(+fixnum)");
    data("\x00"); TEST(obj.type == UNS && obj.uns == 0);
    data("\x01"); TEST(obj.type == UNS && obj.uns == 1);
    data("\x02"); TEST(obj.type == UNS && obj.uns == 2);
    data("\x0f"); TEST(obj.type == UNS && obj.uns == 0x0f);
    data("\x10"); TEST(obj.type == UNS && obj.uns == 0x10);
    data("\x7f"); TEST(obj.type == UNS && obj.uns == 0x7f);

    unit("(-fixnum)");
    data("\xff"); TEST(obj.type == SIG && obj.sig == -1);
    data("\xfe"); TEST(obj.type == SIG && obj.sig == -2);
    data("\xf0"); TEST(obj.type == SIG && obj.sig == -16);
    data("\xe0"); TEST(obj.type == SIG && obj.sig == -32);

    unit("(+int)");
    data("\xcc\x80"); TEST(obj.type == UNS && obj.uns == 0x80);
    data("\xcc\xff"); TEST(obj.type == UNS && obj.uns == 0xff);
    data("\xcd\x01\x00"); TEST(obj.type == UNS && obj.uns == 0x100);
    data("\xcd\xff\xff"); TEST(obj.type == UNS && obj.uns == 0xffff);
    data("\xce\x00\x01\x00\x00"); TEST(obj.type == UNS && obj.uns == 0x10000);
    data("\xce\xff\xff\xff\xff"); TEST(obj.type == UNS && obj.uns == 0xffffffffull);
    data("\xcf\x00\x00\x00\x01\x00\x00\x00\x00"); TEST(obj.type == UNS && obj.uns == 0x100000000ull);
    data("\xcf\xff\xff\xff\xff\xff\xff\xff\xff"); TEST(obj.type == UNS && obj.uns == 0xffffffffffffffffull);

    unit("(-int)");
    data("\xd0\xdf"); TEST(obj.type == SIG && obj.sig == -33);
    data("\xd0\x80"); TEST(obj.type == SIG && obj.sig == -128);
    data("\xd1\xff\x7f"); TEST(obj.type == SIG && obj.sig == -129);
    data("\xd1\x80\x00"); TEST(obj.type == SIG && obj.sig == -32768);
    data("\xd2\xff\xff\x7f\xff"); TEST(obj.type == SIG && obj.sig == -32769);
    data("\xd2\x80\x00\x00\x00"); TEST(obj.type == SIG && obj.sig == -2147483648ll);
    data("\xd3\xff\xff\xff\xff\x7f\xff\xff\xff"); TEST(obj.type == SIG && obj.sig == -2147483649ll);
    data("\xd3\x80\x00\x00\x00\x00\x00\x00\x00"); TEST(obj.type == SIG && obj.sig == INT64_MIN);

    unit("(misc)");
    data("\xc0"); TEST(obj.type == NIL && obj.chr == 0);
    data("\xc2"); TEST(obj.type == BOL && obj.chr == 0);
    data("\xc3"); TEST(obj.type == BOL && obj.chr == 1);

    data("\x90"); TEST(obj.type == ARR && obj.sz == 0);

    data("\x91\xc0");
    TEST(obj.type==ARR && obj.sz==1);
    TEST(obj.type==NIL);

    data("\x9f\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e");
    TEST(obj.type==ARR && obj.sz==15);
    for(int i = 0; i < 15; ++i) {
        TEST(obj.type==UNS && obj.sig==i);
    }

    data("\xdc\x00\x10\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c"
         "\x0d\x0e\x0f");
    TEST(obj.type==ARR && obj.sz==16);
    for(unsigned i = 0; i < 16; ++i) {
        TEST(obj.type == UNS && obj.uns == i);
    }

    data("\x80");
    TEST(obj.type == MAP && obj.sz == 0);

    data("\x81\xc0\xc0");
    TEST(obj.type == MAP && obj.sz == 1);
    TEST(obj.type == NIL);
    TEST(obj.type == NIL);

    data("\x82\x00\x00\x01\x01");
    TEST(obj.type == MAP && obj.sz == 2);
    TEST(obj.type == UNS && obj.sig == 0);
    TEST(obj.type == UNS && obj.sig == 0);
    TEST(obj.type == UNS && obj.sig == 1);
    TEST(obj.type == UNS && obj.sig == 1);

    data("\x8f\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e"
         "\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d");
    TEST(obj.type == MAP && obj.sz == 15);
    for(unsigned i = 0; i < 15; ++i) {
        TEST(obj.type == UNS && obj.uns == i*2+0);
        TEST(obj.type == UNS && obj.uns == i*2+1);
    }

    data("\xde\x00\x10\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c"
         "\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c"
         "\x1d\x1e\x1f");
    TEST(obj.type == MAP && obj.sz == 16);
    for(unsigned i = 0; i < 16; ++i) {
        TEST(obj.type == UNS && obj.uns == i*2+0);
        TEST(obj.type == UNS && obj.uns == i*2+1);
    }

    data("\x91\x90");
    test( obj.type == ARR && obj.sz == 1 );
    test( obj.type == ARR && obj.sz == 0 );

    data("\x93\x90\x91\x00\x92\x01\x02");
    test( obj.type == ARR && obj.sz == 3 );
        test( obj.type == ARR && obj.sz == 0 );
        test( obj.type == ARR && obj.sz == 1 );
            test( obj.type == UNS && obj.uns == 0 );
        test( obj.type == ARR && obj.sz == 2 );
            test( obj.type == UNS && obj.uns == 1 );
            test( obj.type == UNS && obj.uns == 2 );

    data("\x95\x90\x91\xc0\x92\x90\x91\xc0\x9f\x00\x01\x02\x03\x04\x05\x06"
         "\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\xdc\x00\x10\x00\x01\x02\x03\x04"
         "\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f");
    test( obj.type == ARR && obj.sz == 5 );
        test( obj.type == ARR && obj.sz == 0 );
        test( obj.type == ARR && obj.sz == 1 );
            test( obj.type == NIL );
        test( obj.type == ARR && obj.sz == 2 );
            test( obj.type == ARR && obj.sz == 0 );
            test( obj.type == ARR && obj.sz == 1 );
                test( obj.type == NIL );
        test( obj.type == ARR && obj.sz == 15 );
            for( unsigned i = 0; i < 15; ++i ) {
                test( obj.type == UNS && obj.uns == i );
            }
        test( obj.type == ARR && obj.sz == 16 );
            for( unsigned i = 0; i < 15; ++i ) {
                test( obj.type == UNS && obj.uns == i );
            }

    data("\x85\x00\x80\x01\x81\x00\xc0\x02\x82\x00\x80\x01\x81\xc0\xc0\x03"
         "\x8f\x00\x00\x01\x01\x02\x02\x03\x03\x04\x04\x05\x05\x06\x06\x07"
         "\x07\x08\x08\x09\x09\x0a\x0a\x0b\x0b\x0c\x0c\x0d\x0d\x0e\x0e\x04"
         "\xde\x00\x10\x00\x00\x01\x01\x02\x02\x03\x03\x04\x04\x05\x05\x06"
         "\x06\x07\x07\x08\x08\x09\x09\x0a\x0a\x0b\x0b\x0c\x0c\x0d\x0d\x0e"
         "\x0e\x0f\x0f");
    TEST(obj.type == MAP && obj.sz == 5);
        TEST(obj.type == UNS && obj.uns == 0);
        TEST(obj.type == MAP && obj.sz == 0);
        TEST(obj.type == UNS && obj.uns == 1);
        TEST(obj.type == MAP && obj.sz == 1);
            TEST(obj.type == UNS && obj.uns == 0);
            TEST(obj.type == NIL);
        TEST(obj.type == UNS && obj.uns == 2);
        TEST(obj.type == MAP && obj.sz == 2);
            TEST(obj.type == UNS && obj.uns == 0);
            TEST(obj.type == MAP && obj.sz == 0);
            TEST(obj.type == UNS && obj.uns == 1);
            TEST(obj.type == MAP && obj.sz == 1);
                TEST(obj.type == NIL);
                TEST(obj.type == NIL);
        TEST(obj.type == UNS && obj.uns == 3);
        TEST(obj.type == MAP && obj.sz == 15);
            for( unsigned i = 0; i < 15; ++i ) {
                TEST(obj.type == UNS && obj.uns == i);
                TEST(obj.type == UNS && obj.uns == i);
            }
        TEST(obj.type == UNS && obj.uns == 4);
        TEST(obj.type == MAP && obj.sz == 16);
            for( unsigned i = 0; i < 16; ++i ) {
                TEST(obj.type == UNS && obj.uns == i);
                TEST(obj.type == UNS && obj.uns == i);
            }

    data("\x85\xd0\xd1\x91\xc0\x90\x81\xc0\x00\xc0\x82\xc0\x90\x04\x05\xa5"
         "\x68\x65\x6c\x6c\x6f\x93\xa7\x62\x6f\x6e\x6a\x6f\x75\x72\xc0\xff"
         "\x91\x5c\xcd\x01\x5e");
    TEST(obj.type == MAP && obj.sz == 5);
        TEST(obj.type == SIG && obj.sig == -47);
        TEST(obj.type == ARR && obj.sz == 1);
            TEST(obj.type == NIL);
        TEST(obj.type == ARR && obj.sz == 0);
        TEST(obj.type == MAP && obj.sz == 1);
            TEST(obj.type == NIL);
            TEST(obj.type == UNS && obj.uns == 0);
        TEST(obj.type == NIL);
        TEST(obj.type == MAP && obj.sz == 2);
            TEST(obj.type == NIL);
            TEST(obj.type == ARR && obj.sz == 0);
            TEST(obj.type == UNS && obj.uns == 4);
            TEST(obj.type == UNS && obj.uns == 5);
        TEST(obj.type == STR && !strcmp(obj.str, "hello"));
        TEST(obj.type == ARR && obj.sz == 3);
            TEST(obj.type == STR && !strcmp(obj.str, "bonjour"));
            TEST(obj.type == NIL);
            TEST(obj.type == SIG && obj.sig == -1);
        TEST(obj.type == ARR && obj.sz == 1);
            TEST(obj.type == UNS && obj.uns == 92);
        TEST(obj.type == UNS && obj.uns == 350);

    data("\x82\xa7" "compact" "\xc3\xa6" "schema" "\x00");
    TEST(obj.type == MAP && obj.sz == 2);
    TEST(obj.type == STR && obj.sz == 7 && !strcmp(obj.str, "compact"));
    TEST(obj.type == BOL && obj.chr == 1);
    TEST(obj.type == STR && obj.sz == 6 && !strcmp(obj.str, "schema"));
    TEST(obj.type == UNS && obj.sz == 1 && obj.uns == 0);

#   undef TEST
#   undef data
#   undef unit
}

bool vardump( struct variant *w ) {
    static int tabs = 0;
    struct variant v = *w;
    printf("%.*s", tabs, "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t");
    switch( v.type ) {
    default: case ERR:
         if( !msgunpack_eof() ) printf("ERROR: unknown tag type (%02X)\n", (int)v.type);
         return false;
    break; case NIL: printf("(%s)\n", "null");
    break; case BOL: printf("bool: %d\n", v.chr);
    break; case SIG: printf("int: %lld\n", v.sig);
    break; case UNS: printf("uint: %llu\n", v.uns);
    break; case FLT: printf("float: %g\n", v.flt);
    break; case STR: printf("string: '%s'\n", v.str);
    break; case BIN: { for( size_t n = 0; n < v.sz; n++ ) printf("%s%02x(%c)", n > 0 ? " ":"binary: ", v.str[n], v.str[n] >= 32 ? v.str[n] : '.'); puts(""); }
    break; case EXT: { printf("ext: [%02X (%d)] ", v.ext, v.ext); for( size_t n = 0; n < v.sz; n++ ) printf("%s%02x(%c)", n > 0 ? " ":"", v.str[n], v.str[n] >= 32 ? v.str[n] : '.'); puts(""); }
    break; case ARR: {
        ++tabs; puts("[");
        for( size_t n = v.sz; n-- > 0; ) {
            if( !msgunpack_var(&v) || !vardump(&v) ) return false;
        }
        --tabs; puts("]");
    }
    break; case MAP: {
        ++tabs; puts("{");
        for( size_t n = v.sz; n-- > 0; ) {
            if( !msgunpack_var(&v) || !vardump(&v) ) return false;
            if( !msgunpack_var(&v) || !vardump(&v) ) return false;
        }
        --tabs; puts("}");
    }}
    return true;
}

void testdump( const char *fname ) {
    FILE *fp = fopen(fname, "rb");
    if( !fp ) {
        fputs("Cannot read input stream", stderr);
    } else {
        if( msgunpack_new(fp, 0) ) {
            struct variant v;
            while( msgunpack_var(&v) ) {
                vardump(&v);
            }
            if( msgunpack_err() ) {
                fputs("Error while unpacking", stderr);
            }
        }
        fclose(fp);
    }
}

void testwrite(const char *outfile) {
    char buf[256];
    msgpack_new(buf, 256);
    int len = msgpack("ddufs [dddddddd-dddddddd {sisi bne"/*bp0*/,
        -123LL, 123LL, 123456ULL, 3.14159f, "hello world",
        16ULL,
         -31LL, -32LL, -127LL, -128LL, -255LL, -256LL, -511LL, -512LL,  // ,121, 3, "hi",
         +31LL, +32LL, +127LL, +128LL, +255LL, +256LL, +511LL, +512LL,  // ,121, 3, "hi",
        2ULL,
          "hello", -123LL,
          "world", -456LL,
        1ULL,
        0xeeULL, "this is an EXT type", sizeof("this is an EXT type")-1
    );
    hexdump(buf, len);

    FILE *fp = fopen(outfile, "wb");
    if( fp ) {
        fwrite( buf, len, 1, fp );
        fclose(fp);
    }
}

AUTORUN {
    testwrite("out.mp");
    testdump("out.mp");
}
#endif

// ----------------------------------------------------------------------------
// STRUCT PACKING
// Based on code by Brian "Beej Jorgensen" Hall (public domain) [1].
// Based on code by Ginger Bill's half<->float (public domain) [2].
// - rlyeh, public domain.
//
// pack.c  -- perl/python-ish pack/unpack functions
// like printf and scanf, but for binary data.
//
// format flags:
//  (<) little endian       (>) big endian (! also)     (=) native endian
//  (c) 8-bit  char         (b) 8-bit  byte
//  (h) 16-bit half         (w) 16-bit word
//  (i) 32-bit integer      (u) 32-bit unsigned         (f) 32-bit float
//  (l) 64-bit long         (q) 64-bit quad             (d) 64-bit double
//  (v) varint
//  (s) string   (64-bit varint length prepended)
//  (S) string   (32-bit fixed  length prepended)
//  (m) memblock (64-bit varint length prepended)
//  (M) memblock (32-bit fixed  length prepended)
//  (z) memblock (zeroed)
//  (#) number of arguments processed (only when unpacking)
//
// @todo:
// - (x) document & test flag
// @totest:
// - (s) string   (64-bit variable length automatically prepended)
// - (S) string   (32-bit fixed    length automatically prepended)
// - (m) memblock (64-bit variable length automatically prepended)
// - (M) memblock (32-bit fixed    length automatically prepended)
// - (z) memblock (zeroed)
// - (#) number of arguments processed (only when unpacking)
//
// @refs:
// [1] http://beej.us/guide/bgnet/output/html/multipage/advanced.html#serialization (Modified to encode NaN and Infinity as well.)
// [2] https://github.com/gingerBill/gb
// [3] http://www.mrob.com/pub/math/floatformats.html#minifloat
// [4] microfloat: [0.002 to 240] range.
// [5] half float: can approximate any 16-bit unsigned integer or its reciprocal to 3 decimal places.

// b/f packing -----------------------------------------------------------------

int loadb_(const uint8_t *buf, const char *fmt, va_list ap) {
    uint64_t args = 0;
    const uint8_t *buf0 = buf;
    char tmp[16+1];
    //uint64_t size = 0, len;
    int32_t len, count, maxstrlen=0;
    int le = 0;

    if(!buf) // buffer estimation
    for(; *fmt != '\0'; fmt++) {
        switch(*fmt) {
        default: if (!isdigit(*fmt)) return 0;
        break; case '!': case '>': case '<': case '=': case ' ':                                                      //  0-bit endianness
        break; case 'c': case 'b': { int8_t c = (int8_t)va_arg(ap, int); buf += 1; }                                  //  8-bit promoted
        break; case 'h': case 'w': { int16_t h = (int16_t)va_arg(ap, int); buf += 2; }                                // 16-bit promoted
        break; case 'i': case 'u': { int32_t l = va_arg(ap, int32_t); buf += 4; }                                     // 32-bit
        break; case 'l': case 'q': { int64_t L = va_arg(ap, int64_t); buf += 8; }                                     // 64-bit
        break; case 'f': { float f = (float)va_arg(ap, double); buf += 4; }                                           // 32-bit float promoted
        break; case 'd': { double F = (double)va_arg(ap, double); buf += 8; }                                         // 64-bit float (double)
        break; case 'v': { int64_t L = va_arg(ap, int64_t); buf += pack64iv(tmp, L); }                                // varint (8,16,32,64 ...)
        break; case 's': { char* s = va_arg(ap, char*); len = strlen(s); buf += pack64iv(tmp, len) + len; }           // string, 64-bit variable length prepended
        break; case 'S': { char* s = va_arg(ap, char*); len = strlen(s); buf += 4 + len; }                            // string, 32-bit fixed    length prepended
        break; case 'm': { int len = va_arg(ap, int); char *s = va_arg(ap, char*); buf += pack64iv(tmp, len) + len; } // memblock, 64-bit variable length prepended
        break; case 'M': { int len = va_arg(ap, int); char *s = va_arg(ap, char*); buf += 4 + len; }                  // memblock, 32-bit fixed    length prepended
        break; case 'z': { int len = va_arg(ap, int); buf += len; }                                                   // memblock (zeroed)
        }
    }

    if(buf) // buffer unpacking
    for(; *fmt != '\0'; fmt++) {
        switch(*fmt) {
        default:
            if (isdigit(*fmt)) { // track max str len
                maxstrlen = maxstrlen * 10 + (*fmt-'0');
            } else {
                return 0;
            }
        break; case ' ':
        break; case '!': le = 0;
        break; case '>': le = 0;
        break; case '<': le = 1;
        break; case '=': le = is_little() ? 1 : 0;
        break; case 'c': case 'b': ++args; { // 8-bit
            int8_t *v = va_arg(ap, int8_t*);
            *v = *buf <= 0x7f ? (int8_t)*buf : -1 -(uint8_t)(0xffu - *buf);
            buf += 1;
        }
        break; case 'h': case 'w': ++args; { // 16-bit
            int16_t *v = va_arg(ap, int16_t*);
            *v = unpack16i(buf, le);
            buf += 2;
        }
        break; case 'i': case 'u': ++args; { // 32-bit
            int32_t *v = va_arg(ap, int32_t*);
            *v = unpack32i(buf, le);
            buf += 4;
        }
        break; case 'l': case 'q': ++args; { // 64-bit
            int64_t *v = va_arg(ap, int64_t*);
            *v = unpack64i(buf, le);
            buf += 8;
        }
        break; case 'v': ++args; { // varint (8,16,32,64 ...)
            int64_t *L = va_arg(ap, int64_t*);
            buf += unpack64iv(buf, L);
        }
        break; case 'f': ++args; { // 32-bit float
            float *v = va_arg(ap, float*);
            int32_t i = unpack32i(buf, le);
            *v = unpack754_32(i);
            buf += 4;
        }
        break; case 'd': ++args; { // 64-bit float (double)
            double *v = va_arg(ap, double*);
            int64_t i = unpack64i(buf, le);
            *v = unpack754_64(i);
            buf += 8;
        }
        break; case 'S': ++args; { // string, 32-bit fixed length prepended
            char *s = va_arg(ap, char*);
            int64_t vlen = unpack32i(buf, le), read = 4;
            count = (maxstrlen > 0 && vlen >= maxstrlen ? maxstrlen - 1 : vlen);
            memcpy(s, buf + read, count);
            s[count] = '\0';
            buf += read + vlen;
        }
        break; case 's': ++args; { // string, 64-bit variable length prepended
            char *s = va_arg(ap, char*);
            int64_t vlen, read = unpack64iv(buf, &vlen);
            count = (maxstrlen > 0 && vlen >= maxstrlen ? maxstrlen - 1 : vlen);
            memcpy(s, buf + read, count);
            s[count] = '\0';
            buf += read + vlen;
        }
        break; case 'M': ++args; { // memblock, 32-bit fixed length prepended
            char *s = va_arg(ap, char*);
            int64_t vlen = unpack64iv(buf, &vlen), read = 4;
            count = vlen; //(maxstrlen > 0 && vlen >= maxstrlen ? maxstrlen - 1 : vlen);
            memcpy(s, buf + read, count);
            //s[count] = '\0';
            buf += read + vlen;
        }
        break; case 'm': ++args; { // memblock, 64-bit variable length prepended
            char *s = va_arg(ap, char*);
            int64_t vlen, read = unpack64iv(buf, &vlen);
            count = vlen; //(maxstrlen > 0 && vlen >= maxstrlen ? maxstrlen - 1 : vlen);
            memcpy(s, buf + read, count);
            //s[count] = '\0';
            buf += read + vlen;
        }
        break; case 'z': ++args; { // zero-init mem block
            int *l = va_arg(ap, int*);
            const uint8_t *prev = buf;
            while( *buf == 0 ) ++buf;
            *l = buf - prev;
        }
        break; case '#': {
            int *l = va_arg(ap, int*);
            *l = args;
        }
        }

        if (!isdigit(*fmt)) {
            maxstrlen = 0;
        }
    }

    return (int)( buf - buf0 );
}

int saveb_(uint8_t *buf, const char *fmt, va_list ap) {
    uint64_t size = 0, len;
    int le = 0;

    // buffer estimation
    if( !buf ) {
        return loadb_(buf, fmt, ap); // + strlen(buf) * 17; // worse (v)arint estimation for 128-bit ints (17 bytes each)
    }

    // buffer packing
    for(; *fmt != '\0'; fmt++) {
        switch(*fmt) {
        default: size = 0; // error
        break; case '!': le = 0;
        break; case '>': le = 0;
        break; case '<': le = 1;
        break; case ' ': le = le;
        break; case '=': le = is_little() ? 1 : 0;
        break; case 'c': case 'b': { // 8-bit
            int v = (int8_t)va_arg(ap, int /*promoted*/ );
            *buf++ = (v>>0)&0xff;
            size += 1;
        }
        break; case 'h': case 'w': { // 16-bit
            int v = (int16_t)va_arg(ap, int /*promoted*/ );
            pack16i(buf, v, le);
            buf += 2;
            size += 2;
        }
        break; case 'i': case 'u': { // 32-bit
            int32_t v = va_arg(ap, int32_t);
            pack32i(buf, v, le);
            buf += 4;
            size += 4;
        }
        break; case 'l': case 'q': { // 64-bit
            int64_t v = va_arg(ap, int64_t);
            pack64i(buf, v, le);
            buf += 8;
            size += 8;
        }
        break; case 'v': { // varint (8,16,32,64 ...)
            int64_t v = va_arg(ap, int64_t);
            int64_t L = pack64iv(buf, v);
            buf += L;
            size += L;
        }
        break; case 'f': { // 32-bit float
            double v = (float)va_arg(ap, double /*promoted*/ );
            int32_t i = pack754_32(v); // convert to IEEE 754
            pack32i(buf, i, le);
            buf += 4;
            size += 4;
        }
        break; case 'd': { // 64-bit float (double)
            double v = (double)va_arg(ap, double);
            int64_t i = pack754_64(v); // convert to IEEE 754
            pack64i(buf, i, le);
            buf += 8;
            size += 8;
        }
        break; case 'S': { // string, 32-bit fixed length prepended
            char* s = va_arg(ap, char*);
            int len = strlen(s);
            pack32i(buf, len, le);
            memcpy(buf + 4, s, len);
            buf += 4 + len;
            size += 4 + len;
        }
        break; case 's': { // string, 64-bit variable length prepended
            char* s = va_arg(ap, char*);
            int len = strlen(s);
            int64_t L = pack64iv(buf, len);
            memcpy(buf + L, s, len);
            buf += L + len;
            size += L + len;
        }
        break; case 'M': { // memblock, 32-bit fixed length prepended
            int len = va_arg(ap, int);
            char* s = va_arg(ap, char*);
            pack32i(buf, len, le);
            memcpy(buf + 4, s, len);
            buf += 4 + len;
            size += 4 + len;
        }
        break; case 'm': { // memblock, 64-bit variable length prepended
            int len = va_arg(ap, int);
            char* s = va_arg(ap, char*);
            int64_t L = pack64iv(buf, len);
            memcpy(buf + L, s, len);
            buf += L + len;
            size += L + len;
        }
        break; case 'z': { // memblock (zeroed)
            int len = va_arg(ap, int);
            memset(buf, 0, len);
            buf += len;
            size += len;
        }
        }
    }

    return (int)size;
}


int saveb(uint8_t *buf, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int rc = saveb_( buf, fmt, ap);
    va_end(ap);
    return rc;
}
int loadb(const uint8_t *buf, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int rc = loadb_( buf, fmt, ap);
    va_end(ap);
    return rc;
}

int savef(FILE *fp, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    // estimate bytes
    int req = saveb_( 0, fmt, ap);

    char stack[4096];
    char *buf = req < 4096 ? stack : (char*)calloc(1, req + 1 );
        int rc = saveb_(buf, fmt, ap);
        fwrite(buf, req,1, fp);
    if( !(req < 4096) ) free(buf);

    va_end(ap);
    return rc;
}
int loadf(FILE *fp, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    // estimate bytes
    int req = loadb_( 0, fmt, ap) * 2; // *2 in case it is underestimated

    char stack[4096];
    char *buf = req < 4096 ? stack : (char*)calloc(1, req + 1 );
        fread(buf, req,1, fp);
        int rc = loadb_(buf, fmt, ap);
    if( !(req < 4096) ) free(buf);

    va_end(ap);
    return rc;
}

#if 0
AUTORUN {
    const char *dna = "3b8bhbhbbhhbhhhuu"; // "1c1h212122122233"; "i3c8chchchhchhhdd"

    struct bootsector {
        uint8_t jump_instruction[3];
        uint8_t oem_name[8];
        uint16_t bytes_per_sector;
        uint8_t sectors_per_cluster;
        uint16_t reserved_sectors;
        uint8_t fat_copies;
        uint16_t max_dirs;
        uint16_t sector_count;
        uint8_t media_descriptor;
        uint16_t sectors_per_fat;
        uint16_t sectors_per_head;
        uint16_t heads;
        uint32_t hidden_sectors;
        uint32_t sector_countz;
    } fat = { {0,1,2},{3,4,5,6,7,8,9,10},11,12,13,14,15,16,17,18,19,20,21,22 };
    hexdump(&fat, sizeof(struct bootsector));

    FILE *fp = fopen("test.mbr", "wb");
    savef(fp, dna, &fat); //
    fclose(fp);

    memset(&fat, 0, sizeof(struct bootsector));

    fp = fopen("test.mbr", "rb");
    loadf(fp, dna, &fat);
    fclose(fp);

    hexdump(&fat, sizeof(struct bootsector));
}
#endif
