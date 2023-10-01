// ARK: lightweight, append-only, header-less journaling file format specification.
// - rlyeh, public domain.

// Features:
// - [x] Journaling support: data can be rolled back to an earlier state to retrieve older versions of files.
// - [x] Append-only format: create or update new entries just by appending stuff to the journal file.
// - [x] Compaction support: compact archives by keeping, for each duplicated file, its latest revision only.
// - [x] Concat friendly: journals can be glued together, and the result will still be a valid journey file.
// - [x] Foreign support: append random data to a foreign file and result will still be a valid journey file.
// - [x] Always aligned: file data is always 512-byte aligned for safe/fast memory access.
// - [x] Simple, tiny, portable, cross-platform, header-only.
// - [x] Public domain, CC0, 0-BSD, unlicensed (pick one).

// Extension:
// .ark

// File format:
// [foreign data] 
// [...]
// [archive-entry #1]
// [archive-entry #2]
// [...]
// [archive-entry #N]
// [EOF]
//
// Where, each archive-entry is {
//      [zero] 472-byte aligned zero padding
//      [mark] 64-bit magic id 'ArK1' (if 1KrA is found, swap endianness)
//      [time] 64-bit time stamp in seconds (unix epoch)
//      [dlen] 64-bit data length
//      [hash] 64-bit data hash
//      [nlen] 64-bit name length+1
//      [data] file data (512-byte aligned)
//      [name] file name+NUL
// }

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ARK1         'ArK1'
#define ARK1_PADDING (512 - 40) // 472

#ifndef ARK_PRINTF
#define ARK_PRINTF printf
#endif

#ifndef ARK_SWAP32
#define ARK_SWAP32(x) x
#define ARK_SWAP64(x) x
#endif

#ifndef ARK_REALLOC
#define ARK_REALLOC realloc
#endif

static int        ark_fput64( FILE *out, uint64_t v ) { return fwrite( (v = ARK_SWAP64(v), &v), 1, 8, out); }
static uint64_t   ark_fget64( FILE *in ) { uint64_t v; fread( &v, 1, 8, in ); return ARK_SWAP64(v); }
static const char ark1_zero[ARK1_PADDING] = {0};

// `outfile` must be fopen("file.ark", "a+b") beforehand
int ark_append_mem( FILE *out, const char *name, const void *data, int datalen, uint64_t stamp ) {
    fseek( out, 0L, SEEK_END );
    while( 0 != (ftell(out) % ARK1_PADDING) ) fwrite(ark1_zero, 1, ARK1_PADDING - (ftell(out) % ARK1_PADDING), out);
    ARK_PRINTF("Writing %s at #%d\n", name, (int)ftell(out));
    int namelen = strlen(name) + 1;
    uint64_t mark = ARK1;
    ark_fput64(out, mark);
    ark_fput64(out, stamp);
    ark_fput64(out, datalen);
    ark_fput64(out, 0ULL/*hash*/);
    ark_fput64(out, namelen);
    fwrite(data, 1, datalen, out);
    fwrite(name, 1, namelen, out);
    return 1;
}

// `outfile` must be fopen("file.ark", "a+b") beforehand
int ark_append_file( FILE *out, const char *name ) {
    FILE *in = fopen(name, "rb");
    if( in ) {
        fseek(in, 0L,  SEEK_END);
        size_t sz = ftell(in);
        fseek(in, 0L, SEEK_SET);
        char *buffer = ARK_REALLOC(0, sz);
        if( !buffer ) return fclose(in), 0;
        fread(buffer, 1, sz, in);
        fclose(in);
        int rc = ark_append_mem(out, name, buffer, sz, 0ULL);
        ARK_REALLOC(buffer, 0);
        return rc;
    }
    return 0;
}

void ark_list( FILE *in, void *yield_fn ) {
    int (*ark_yield_fn)() = yield_fn;
    while( !feof(in) ) {
        while( 0 != (ftell(in) % ARK1_PADDING) && !feof(in) ) fseek(in, ARK1_PADDING - (ftell(in) % ARK1_PADDING), SEEK_CUR);
        ARK_PRINTF("Reading at #%d\n", (int)ftell(in));
        uint64_t mark = ark_fget64(in);
        if( mark != ARK1 ) continue;
        uint64_t stamp = ark_fget64(in);
        uint64_t datalen = ark_fget64(in);
        uint64_t datahash = ark_fget64(in);
        uint64_t namelen = ark_fget64(in);
        char *data = ARK_REALLOC(0, datalen);
        fread(data, 1, datalen, in);
        char *name = ARK_REALLOC(0, namelen);
        fread(name, 1, namelen, in);

        if( yield_fn == printf ) {
            printf("Found %s (%d bytes)\n", name, (int)datalen );
            ARK_REALLOC(name, 0);
            ARK_REALLOC(data, 0);
        }
        else {
            if( !ark_yield_fn(name, data, datalen, datahash, stamp) )
                return;
        }
    }
}

int main(int argc, char **argv) {
    if( argc > 2 ) {
        FILE *ark = fopen(argv[1], "a+b");
        if( ark ) for( int i = 2; i < argc; ++i) ark_append_file( ark, argv[i] );
        if( ark ) fclose(ark);
    }
    else if( argc == 2 ) {
        FILE *ark = fopen(argv[1], "rb");
        if( ark ) ark_list(ark, printf), fclose(ark);
    }
    else printf("%s infile.ark\n%s outfile.ark infile1 [infile2...]\n", argv[0], argv[0]);
}

// cl ark.c /MT /O2 /DNDEBUG /link setargv.obj
