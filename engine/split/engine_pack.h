// ----------------------------------------------------------------------------
// compression api

enum COMPRESS_FLAGS {
    COMPRESS_RAW     = 0,
    COMPRESS_PPP     = (1<<4),
    COMPRESS_ULZ     = (2<<4),
    COMPRESS_LZ4     = (3<<4),
    COMPRESS_CRUSH   = (4<<4),
    COMPRESS_DEFLATE = (5<<4),
    COMPRESS_LZP1    = (6<<4),
    COMPRESS_LZMA    = (7<<4),
    COMPRESS_BALZ    = (8<<4),
    COMPRESS_LZW3    = (9<<4),
    COMPRESS_LZSS    = (10<<4),
    COMPRESS_BCM     = (11<<4),
    COMPRESS_ZLIB    = (12<<4), // same as deflate with header
};

API unsigned zbounds(unsigned inlen, unsigned flags);
API unsigned zencode(void *out, unsigned outlen, const void *in, unsigned inlen, unsigned flags);
API unsigned zexcess(unsigned flags);
API unsigned zdecode(void *out, unsigned outlen, const void *in, unsigned inlen, unsigned flags);

// ----------------------------------------------------------------------------
// array de/interleaving
//
// results:
// R0G0B0   R1G1B1   R2G2B2...   -> R0R1R2... B0B1B2... G0G1G2...
// R0G0B0A0 R1G1B1A1 R2G2B2A2... -> R0R1R2... A0A1A2... B0B1B2... G0G1G2...

API void *interleave( void *out, const void *list, int list_count, int sizeof_item, unsigned columns );

// ----------------------------------------------------------------------------
// cobs en/decoder

API unsigned cobs_bounds(unsigned len);
API unsigned cobs_encode(const void *in, unsigned inlen, void *out, unsigned outlen);
API unsigned cobs_decode(const void *in, unsigned inlen, void *out, unsigned outlen);

// ----------------------------------------------------------------------------
// base92 en/decoder

API unsigned base92_encode(const void *in, unsigned inlen, void* out, unsigned outlen);
API unsigned base92_decode(const void *in, unsigned inlen, void* out, unsigned outlen);
API unsigned base92_bounds(unsigned inlen);

// ----------------------------------------------------------------------------
// netstring en/decoder

API unsigned netstring_bounds(unsigned inlen);
API unsigned netstring_encode(const char *in, unsigned inlen, char *out, unsigned outlen);
API unsigned netstring_decode(const char *in, unsigned inlen, char *out, unsigned outlen);

// ----------------------------------------------------------------------------
// delta en/decoder

API void delta8_encode(void *buffer, unsigned count);
API void delta8_decode(void *buffer, unsigned count);

API void delta16_encode(void *buffer, unsigned count);
API void delta16_decode(void *buffer, unsigned count);

API void delta32_encode(void *buffer, unsigned count);
API void delta32_decode(void *buffer, unsigned count);

API void delta64_encode(void *buffer, unsigned count);
API void delta64_decode(void *buffer, unsigned count);

// ----------------------------------------------------------------------------
// zigzag en/decoder

API uint64_t zig64( int64_t value ); // convert sign|magnitude to magnitude|sign
API int64_t zag64( uint64_t value ); // convert magnitude|sign to sign|magnitude

API uint32_t enczig32u( int32_t n);
API uint64_t enczig64u( int64_t n);
API  int32_t deczig32i(uint32_t n);
API  int64_t deczig64i(uint64_t n);

// ----------------------------------------------------------------------------
// arc4 en/decryptor

API void *arc4( void *buffer, unsigned buflen, const void *pass, unsigned passlen );

// ----------------------------------------------------------------------------
// crc64

API uint64_t crc64(uint64_t h, const void *ptr, uint64_t len);

// ----------------------------------------------------------------------------
// entropy encoder

API void entropy( void *buf, unsigned n );

// -----------------------------------------------------------------------------
// semantic versioning in a single byte (octal)

API int semver( int major, int minor, int patch );
API int semvercmp( int v1, int v2 );

#define SEMVER(major,minor,patch) (0100 * (major) + 010 * (minor) + (patch))
#define SEMVERCMP(v1,v2) (((v1) & 0110) - ((v2) & 0110))
#define SEMVERFMT "%03o"

// -----------------------------------------------------------------------------
// storage types. refer to vec2i/3i, vec2/3/4 if you plan to do math operations

typedef struct byte2 { uint8_t x,y; } byte2;
typedef struct byte3 { uint8_t x,y,z; } byte3;
typedef struct byte4 { uint8_t x,y,z,w; } byte4;

typedef struct int2 { int x,y; } int2;
typedef struct int3 { int x,y,z; } int3;
typedef struct int4 { int x,y,z,w; } int4;

typedef struct uint2 { unsigned int x,y; } uint2;
typedef struct uint3 { unsigned int x,y,z; } uint3;
typedef struct uint4 { unsigned int x,y,z,w; } uint4;

typedef struct float2 { float x,y; } float2;
typedef struct float3 { float x,y,z; } float3;
typedef struct float4 { float x,y,z,w; } float4;

typedef struct double2 { double x,y; } double2;
typedef struct double3 { double x,y,z; } double3;
typedef struct double4 { double x,y,z,w; } double4;

#define byte2(x,y)       C_CAST(byte2, (uint8_t)(x), (uint8_t)(y) )
#define byte3(x,y,z)     C_CAST(byte3, (uint8_t)(x), (uint8_t)(y), (uint8_t)(z) )
#define byte4(x,y,z,w)   C_CAST(byte4, (uint8_t)(x), (uint8_t)(y), (uint8_t)(z), (uint8_t)(w) )

#define int2(x,y)        C_CAST(int2, (int)(x), (int)(y) )
#define int3(x,y,z)      C_CAST(int3, (int)(x), (int)(y), (int)(z) )
#define int4(x,y,z,w)    C_CAST(int4, (int)(x), (int)(y), (int)(z), (int)(w) )

#define uint2(x,y)       C_CAST(uint2, (unsigned)(x), (unsigned)(y) )
#define uint3(x,y,z)     C_CAST(uint3, (unsigned)(x), (unsigned)(y), (unsigned)(z) )
#define uint4(x,y,z,w)   C_CAST(uint4, (unsigned)(x), (unsigned)(y), (unsigned)(z), (unsigned)(w) )

#define float2(x,y)      C_CAST(float2, (float)(x), (float)(y) )
#define float3(x,y,z)    C_CAST(float3, (float)(x), (float)(y), (float)(z) )
#define float4(x,y,z,w)  C_CAST(float4, (float)(x), (float)(y), (float)(z), (float)(w) )

#define double2(x,y)     C_CAST(double2, (double)(x), (double)(y) )
#define double3(x,y,z)   C_CAST(double3, (double)(x), (double)(y), (double)(z) )
#define double4(x,y,z,w) C_CAST(double4, (double)(x), (double)(y), (double)(z), (double)(w) )

// -----------------------------------------------------------------------------
// compile-time fourcc, eightcc

API char *cc4str(unsigned cc);
API char *cc8str(uint64_t cc);

enum {
#   define _(a,b,c,d,e) cc__##a, cc__##b, cc__##c, cc__##d, cc__##e
    cc__1 = '1', _(2,3,4,5,6),_(7,8,9,0,_), cc__ = ' ',
    cc__A = 'A', _(B,C,D,E,F),_(G,H,I,J,K),_(L,M,N,O,P),_(Q,R,S,T,U),_(V,W,X,Y,Z),
    cc__a = 'a', _(b,c,d,e,f),_(g,h,i,j,k),_(l,m,n,o,p),_(q,r,s,t,u),_(v,w,x,y,z),
#   undef _
};

#ifdef BIG
#define cc4(a,b,c,d) ((uint32_t)(cc__##a<<24) | (cc__##b<<16) | (cc__##c<<8) | (cc__##d<<0))
#define cc8(a,b,c,d,e,f,g,h) (((uint64_t)cc4(a,b,c,d) << 32ULL) | cc4(e,f,g,h))
#else
#define cc4(a,b,c,d) ((uint32_t)(cc__##d<<24) | (cc__##c<<16) | (cc__##b<<8) | (cc__##a<<0))
#define cc8(a,b,c,d,e,f,g,h) (((uint64_t)cc4(e,f,g,h) << 32ULL) | cc4(a,b,c,d))
#endif

#define cc3(a,b,c) cc4(,a,b,c)
#define cc5(a,b,c,d,e) cc6(,a,b,c,d,e)
#define cc6(a,b,c,d,e,f) cc7(,a,b,c,d,e,f)
#define cc7(a,b,c,d,e,f,g) cc8(,a,b,c,d,e,f,g)

// ----------------------------------------------------------------------------
// text conversions

API char* ftoa1(float v);
API char* ftoa2(vec2  v);
API char* ftoa3(vec3  v);
API char* ftoa4(vec4  v);

API float atof1(const char *s);
API vec2  atof2(const char *s);
API vec3  atof3(const char *s);
API vec4  atof4(const char *s);

API char* itoa1(int   v);
API char* itoa2(vec2i v);
API char* itoa3(vec3i v);

API int   atoi1(const char *s);
API vec2i atoi2(const char *s);
API vec3i atoi3(const char *s);

// ----------------------------------------------------------------------------
// endianness

API int         is_big();
API int         is_little();

API uint16_t    swap16( uint16_t x );
API uint32_t    swap32( uint32_t x );
API uint64_t    swap64( uint64_t x );
API float       swap32f(float n);
API double      swap64f(double n);
API void        swapf(float *a, float *b);
API void        swapf2(vec2 *a, vec2 *b);
API void        swapf3(vec3 *a, vec3 *b);
API void        swapf4(vec4 *a, vec4 *b);

API uint16_t    lil16(uint16_t n); // swap16 as lil
API uint32_t    lil32(uint32_t n); // swap32 as lil
API uint64_t    lil64(uint64_t n); // swap64 as lil
API float       lil32f(float n);   // swap32 as lil
API double      lil64f(double n);  // swap64 as lil

API uint16_t    big16(uint16_t n); // swap16 as big
API uint32_t    big32(uint32_t n); // swap32 as big
API uint64_t    big64(uint64_t n); // swap64 as big
API float       big32f(float n);   // swap32 as big
API double      big64f(double n);  // swap64 as big

API uint16_t*   lil16p(void *p, int sz);
API uint32_t*   lil32p(void *p, int sz);
API uint64_t*   lil64p(void *p, int sz);
API float   *   lil32pf(void *p, int sz);
API double  *   lil64pf(void *p, int sz);

API uint16_t*   big16p(void *p, int sz);
API uint32_t*   big32p(void *p, int sz);
API uint64_t*   big64p(void *p, int sz);
API float   *   big32pf(void *p, int sz);
API double  *   big64pf(void *p, int sz);

#if is(cl)
#define swap16 _byteswap_ushort
#define swap32 _byteswap_ulong
#define swap64 _byteswap_uint64
#elif is(gcc)
#define swap16 __builtin_bswap16
#define swap32 __builtin_bswap32
#define swap64 __builtin_bswap64
#endif

#define hton16 big16
#define ntoh16 big16
#define hton32 big32
#define ntoh32 big32
#define hton64 big64
#define ntoh64 big64

#define IS_BIG    ((*(uint16_t *)"\0\1") == 1)
#define IS_LITTLE ((*(uint16_t *)"\0\1") != 1)

// ----------------------------------------------------------------------------
// half packing

typedef uint16_t half;
API float half_to_float(half value);
API half  float_to_half(float value);

// ----------------------------------------------------------------------------
// int packing

// pack16i() -- store a 16-bit int into a char buffer (like htons())
// pack32i() -- store a 32-bit int into a char buffer (like htonl())
// pack64i() -- store a 64-bit int into a char buffer (like htonl())

API void pack16i(uint8_t *buf, uint16_t i, int swap);
API void pack32i(uint8_t *buf, uint32_t i, int swap);
API void pack64i(uint8_t *buf, uint64_t i, int swap);

// unpack16i() -- unpack a 16-bit int from a char buffer (like ntohs())
// unpack32i() -- unpack a 32-bit int from a char buffer (like ntohl())
// unpack64i() -- unpack a 64-bit int from a char buffer (like ntohl())
// changes unsigned numbers to signed if needed.

API int16_t unpack16i(const uint8_t *buf, int swap);
API int32_t unpack32i(const uint8_t *buf, int swap);
API int64_t unpack64i(const uint8_t *buf, int swap);

// ----------------------------------------------------------------------------
// float un/packing: 8 (micro), 16 (half), 32 (float), 64 (double) types

#define pack754_8(f)    (  pack754((f),  8,  4))
#define pack754_16(f)   (  pack754((f), 16,  5))
#define pack754_32(f)   (  pack754((f), 32,  8))
#define pack754_64(f)   (  pack754((f), 64, 11))
#define unpack754_8(u)  (unpack754((u),  8,  4))
#define unpack754_16(u) (unpack754((u), 16,  5))
#define unpack754_32(u) (unpack754((u), 32,  8))
#define unpack754_64(u) (unpack754((u), 64, 11))

API    uint64_t pack754(long double f, unsigned bits, unsigned expbits);
API long double unpack754(uint64_t i, unsigned bits, unsigned expbits);

// ----------------------------------------------------------------------------
// variable-length integer packing

API uint64_t pack64uv( uint8_t *buffer, uint64_t value );
API uint64_t unpack64uv( const uint8_t *buffer, uint64_t *value );
API uint64_t pack64iv( uint8_t *buffer, int64_t value_ );
API uint64_t unpack64iv( const uint8_t *buffer, int64_t *value );

// ----------------------------------------------------------------------------
// msgpack v5, schema based struct/buffer bitpacking

// api v2

API int  msgpack(const char *fmt, ... );                // va arg pack "n,b,u,d/i,s,p,f/g,e,[,{". returns number of written bytes
API int  msgunpack(const char *fmt, ... );              // va arg pack "n,b,u,d/i,s,p,f/g,e,[,{". returns number of parsed args

// api v1

API int msgpack_new(uint8_t *w, size_t l);
API int msgpack_nil();                                  // write null
API int msgpack_chr(bool n);                            // write boolean
API int msgpack_uns(uint64_t n);                        // write unsigned integer
API int msgpack_int(int64_t n);                         // write integer
API int msgpack_str(const char *s);                     // write string
API int msgpack_bin(const char *s, size_t n);           // write binary pointer
API int msgpack_flt(double g);                          // write real
API int msgpack_ext(uint8_t key, void *val, size_t n);  // write extension type
API int msgpack_arr(uint32_t n);                        // write array mark for next N items
API int msgpack_map(uint32_t n);                        // write map mark for next N pairs (N keys + N values)
API int msgpack_eof();                                  // write full?
API int msgpack_err();                                  // write error?

API bool msgunpack_new( const void *opaque_or_FILE, size_t bytes );
API bool msgunpack_nil();
API bool msgunpack_chr(bool *chr);
API bool msgunpack_uns(uint64_t *uns);
API bool msgunpack_int(int64_t *sig);
API bool msgunpack_str(char **str);
API bool msgunpack_bin(void **bin, uint64_t *len);
API bool msgunpack_flt(float *flt);
API bool msgunpack_dbl(double *dbl);
API bool msgunpack_ext(uint8_t *key, void **val, uint64_t *len);
API bool msgunpack_arr(uint64_t *len);
API bool msgunpack_map(uint64_t *len);
API bool msgunpack_eof();
API bool msgunpack_err();

// ----------------------------------------------------------------------------
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

// - save data dictated by the format string from the buffer. return: number of bytes written, or 0 if error.
//   if first argument is zero, returns number of bytes required for packing.

API int savef(FILE *file, const char *format, ...);
API int saveb(unsigned char *buf, const char *format, ...);

// - load data dictated by the format string into the buffer. return: number of bytes read, or 0 if error.
//   if first argument is zero, returns number of bytes required for unpacking.

API int loadf(FILE *file, const char *format, ...);
API int loadb(const unsigned char *buf, const char *format, ...);

