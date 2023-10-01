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
// - rlyeh, public domain.
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
