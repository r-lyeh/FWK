// ase2spr, based on atlas code by Sepehr Taghdisian (BSD2 licensed)
// - rlyeh, public domain.

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef _MSC_VER
#include <strings.h>
#define strcmpi strcasecmp
#else
#define strcmpi stricmp
#endif

#define ATLAS_REALLOC vrealloc
#define ATLAS_MSIZE   vlen

void* vrealloc( void* p, size_t sz ) {
    if( !sz ) {
        if( p ) {
            size_t *ret = (size_t*)p - 2;
            ret[0] = 0;
            ret[1] = 0;
            realloc( ret, 0 );
        }
        return 0;
    } else {
        size_t *ret;
        if( !p ) {
            ret = (size_t*)realloc( 0, sizeof(size_t) * 2 + sz );
            ret[0] = sz;
            ret[1] = 0;
        } else {
            ret = (size_t*)p - 2;
            size_t osz = ret[0];
            size_t ocp = ret[1];
            if( sz <= (osz + ocp) ) {
                ret[0] = sz;
                ret[1] = ocp - (sz - osz);
            } else {
                ret = (size_t*)realloc( ret, sizeof(size_t) * 2 + sz * 1.75 );
                ret[0] = sz;
                ret[1] = (size_t)(sz * 1.75) - sz;
            }
        }
        return &ret[2];
    }
}
size_t vlen( void* p ) {
    return p ? 0[ (size_t*)p - 2 ] : 0;
}

char *vstrdup(const char *s) { size_t n = strlen(s)+1; return ((char*)memcpy(ATLAS_REALLOC(0,n), s, n)); } ///-

#define STRDUP vstrdup

static unsigned array_c;
#define array(t) t*
#define array_push(arr, v) ( array_c = array_count(arr), array_c[(arr) = vrealloc((arr), (array_c + 1) * sizeof(0[arr]))] = (v) )
#define array_pop(arr) ( (arr) ? (arr) = vrealloc((arr), (array_count(arr)-1) * sizeof(0[arr])) : (0) )
#define array_count(arr) (int)( (arr) ? vlen(arr) / sizeof(0[arr]) : sizeof(0[arr]) - sizeof(0[arr]) )
#define array_free(arr) ( (arr) ? (vrealloc(arr, 0), 1) : 0 )

const uint64_t hash_init = 14695981039346656037ULL;
uint64_t hash_bin(const void* ptr, unsigned len, uint64_t hash ) {
    for( unsigned char *str = (unsigned char *)ptr; len--; )
    hash = ( (unsigned char)*str++ ^ hash ) * 0x100000001b3ULL;
    return hash;
}

char *trimspace(char *str) {
    for( char *s = str; *s; ++s )
        if(*s <= 32) memmove(s, s+1, strlen(s));
    return str;
}

char* strcatf(char **src_, const char *fmt, ...) {
    static char buf[1024] = {0};
    va_list va;
    va_start(va, fmt);
    int buflen = vsnprintf( buf, sizeof(buf), fmt, va );
    va_end(va);

    char *src = src_ ? *src_ : 0;
        int srclen = (src ? strlen(src) : 0);
        src = (char*)vrealloc(src, srclen + buflen + 1 );
        memcpy(src + srclen, buf, buflen + 1 );
    if(src_) *src_ = src;
    return src;
}


#ifdef __TINYC__
#define STBI_NO_SIMD
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "3rd_stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "3rd_stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "3rd_stb_image_resize.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "3rd_stb_rect_pack.h"

#define S2O_IMPLEMENTATION
#include "3rd_sproutline.h"

#define DELAUNAY_C
#include "3rd_delaunay.h"

#define CUTE_ASEPRITE_IMPLEMENTATION
#include "3rd_aseprite.h"

#define DIR_C
#include "3rd_archive.h"

#define MALLOC(sz) ATLAS_REALLOC(0,(sz))
#define FREE(p)    ATLAS_REALLOC((p),0)
#define BASE64_C
#include "3rd_base64.h"

#define ATLASC_IMPLEMENTATION
#include "3rd_atlasc.h"


#if   defined _WIN32 && defined _MSC_VER
__declspec(dllexport)
#elif defined _WIN32 && defined __GNUC__
__attribute__ ((dllexport))
#endif

int main(int argc, char* argv[]) {
    const char *help =
        " [flags] -o outfile [folder or file...]\n\n"
        "-h    Show help\n"
        "-V    Show version\n"
        "-o    Output file (defaults: stdout)\n"
        "-W    Maximum output image width (default:1024)\n"
        "-H    Maximum output image height (default:1024)\n"
        "-B    Border size for each sprite (default:2)\n"
        "-2    Make output image size power-of-two\n"
        "-P    Set padding for each sprite (default:1)\n"
        "-m    Make sprite meshes\n"
        "-M    Set maximum vertices for each generated sprite mesh (default:25)\n"
        "-A    Alpha threshold for cropping [1..255] (default:20)\n"
        "-s    Set scale for individual images (default:1.0)\n\n"
    ;

    atlas_flags flags = {
       .alpha_threshold = 20,
       .max_width = 2048,
       .max_height = 2048,
       .border = 2,
       .padding = 1,
       .max_verts_per_mesh = 25,
       .scale = 1.0
    };
    array(char*) files = 0;
    char *outfile = 0;

    for( int i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        if( arg[0] != '-' ) {
            if( path_isfile(arg) )
            array_push(files, STRDUP(arg));
            else
            for( dir *d = dir_open(arg, "r"); d; dir_close(d), d = 0) {
                for( int i = 0, end = dir_count(d); i < end; ++i ) {
                    if( path_isfile(dir_name(d, i)))
                        array_push(files, STRDUP(dir_name(d, i)));
                }
            }
        }
        else
        switch( arg[1] ) {
        case 'V': exit(-printf("%s\nUsing atlasc v" ATLASC_VERSION "\n", argv[0]));
        case '2': flags.pot = 1; break;
        case 'm': flags.mesh = 1; break;
        case '?':
        case 'h': exit(-printf("%s%s", argv[0], help));
        case 'o': outfile = argv[++i]; break;
        case 'A': flags.alpha_threshold = atoi(argv[++i]); break;
        case 'W': flags.max_width = atoi(argv[++i]); break;
        case 'H': flags.max_height = atoi(argv[++i]); break;
        case 'B': flags.border = atoi(argv[++i]); break;
        case 'P': flags.padding = atoi(argv[++i]); break;
        case 'M': flags.max_verts_per_mesh = atoi(argv[++i]); break;
        case 's': flags.scale = atof(argv[++i]); break;
        default:  exit(-printf("Unknown argument: %s\n", arg));
        }
    }

    if( flags.scale < 0.0001f ) {
        printf("%s%s", argv[0], help);
        return(-puts("invalid `scale` parameter"));
    }

    if( !files ) {
        printf("%s%s", argv[0], help);
        return(-puts("no input file(s)"));
    }

    if( !outfile ) outfile = "stdout";

    char *error = g_error_str;

    if( !files ) {
        error = "No input file(s)";
    } else {
        atlas_t* atlas = atlas_loadfiles(files, flags);
        if ( atlas ) {
            bool r = atlas_save(outfile, atlas, flags);
            if( r ) {
                // fprintf(stderr, "Written: %d->%d\n", flags.num_files, !!r);
                error = 0;
            }
            atlas_free(atlas);
        }

        for( int i = 0; i < array_count(files); ++i)
            ATLAS_REALLOC(files[i], 0);
        array_free(files);
    }

    return error ? fprintf(stderr, "%s\n", error), -1 : 0;
}

// tcc ase2ini.c -I ..\engine\split -DNDEBUG
// cl  ase2ini.c -I ..\engine\split /DNDEBUG /O2 /Ox /MT /LD
// cl  ase2ini.c -I ..\engine\split /DNDEBUG /O2 /Ox /MT
// cc  ase2ini.c -I ../engine/split -lm -O3 -oase2ini.linux 
// cc  ase2ini.c -I ../engine/split -lm -O3 -oase2ini.osx
