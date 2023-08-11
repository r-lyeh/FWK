/* game framework (3rd libs)
*  - rlyeh, public domain
*/

#ifdef FWK_3RD
#define GLAD_GL_IMPLEMENTATION              // glad
#endif

{{FILE:3rd_glad.h}}
{{FILE:3rd_font_md.h}}

#ifdef FWK_3RD

//-----------------------------------------------------------------------------
// 3rd party libs

#define ARCHIVE_C                             // archive.c
#define COMPRESS_C                            // compress.c
#define ENET_IMPLEMENTATION                   // enet
#define GJK_C                                 // gjk
#define _GLFW_IMPLEMENTATION                  // glfw337
#define GLFW_INCLUDE_NONE                     // glfw337
#define HTTPS_IMPLEMENTATION                  // https
#define JO_MPEG_COMPONENTS 3                  // jo_mpeg
#define JSON5_C                               // json5
#define LUA_IMPL                              // lua544
#define MINIAUDIO_IMPLEMENTATION              // miniaudio
#define MA_NO_FLAC                            // miniaudio
#define NK_GLFW_GL3_IMPLEMENTATION            // nuklear
#define NK_IMPLEMENTATION                     // nuklear
#define NK_INCLUDE_DEFAULT_ALLOCATOR          // nuklear
#define NK_INCLUDE_DEFAULT_FONT               // nuklear
#define NK_INCLUDE_FIXED_TYPES                // nuklear
#define NK_INCLUDE_FONT_BAKING                // nuklear
#define NK_INCLUDE_STANDARD_IO                // nuklear
#define NK_INCLUDE_STANDARD_VARARGS           // nuklear
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT       // nuklear
#define NK_MAX_LAYOUT_ROW_TEMPLATE_COLUMNS 64 // nuklear
#define NK_KEYSTATE_BASED_INPUT               // nuklear
#define PL_MPEG_IMPLEMENTATION                // pl_mpeg
#define STB_IMAGE_IMPLEMENTATION              // stbi
#define STB_IMAGE_WRITE_IMPLEMENTATION        // stbi_write
#define STB_SPRINTF_IMPLEMENTATION            // stb_sprintf
#define STB_SPRINTF_NOUNALIGNED               // stb_sprintf
#define STS_MIXER_IMPLEMENTATION              // sts_mixer
#define SIMPLEX_C                             // simplex
#define SWRAP_IMPLEMENTATION                  // swrap
#define SWRAP_STATIC                          // swrap
#define THREAD_IMPLEMENTATION                 // thread
#define TFD_IMPLEMENTATION                    // tinyfiledialogs
#define BQ_PLATFORM_IMPLEMENTATION            // websocket
#define BQ_WEBSOCKET_IMPLEMENTATION           // websocket
#define XML_C                                 // xml
#ifdef __APPLE__
#define _GLFW_COCOA                           // glfw osx
#elif defined _WIN32
#define _GLFW_WIN32                           // glfw win32
#else
#define _GLFW_X11                             // glfw linux, also _GLFW_OSMESA or _GLFW_WAYLAND
#endif

#if defined __TINYC__ && defined _WIN32
#define MAPVK_VSC_TO_VK 1
#define MAPVK_VK_TO_VSC 0
#define IPV6_V6ONLY 0
#define _WIN32_WINNT_VISTA 0
#define _WIN32_WINNT_WINXP 0
#define _WIN32_WINNT_WIN7 0
#endif
#ifdef __TINYC__
#define STBI_NO_SIMD
// no uint128_t (3rd_https.h)
char* strtok_s(
   char* str,
   const char* delimiters,
   char** context
);
#endif

#if defined __clang__ && defined _WIN32
int execv(const char *path, char *const argv[]);
#elif (is(tcc) /*|| defined __clang__*/) && defined _WIN32
int execv(const char *path, char *const argv[]);
errno_t strerror_s(
   char *buffer,
   size_t sizeInBytes,
   int errnum
);
typedef int socklen_t;
#if is(tcc)
#define restrict
const char *inet_ntop(int af, const void *restrict src,
                     char *restrict dst, socklen_t size);
int inet_pton(int af, const char *restrict src, void *restrict dst);
#endif
errno_t fopen_s(
   FILE** pFile,
   const char *filename,
   const char *mode
);
#endif

//---
{{FILE:3rd_glfw3.h}}
#undef timeGetTime
//---
{{FILE:3rd_swrap.h}}
//---
{{FILE:3rd_jo_mp1.h}}
#define get_bits stb_vorbis_get_bits
#define error stb_vorbis_error
{{FILE:3rd_stb_vorbis.h}}
#undef error
#undef DEBUG
{{FILE:3rd_sts_mixer.h}}
{{FILE:3rd_miniaudio.h}}
//---
#undef L
#undef C
#undef R
#define error l_error
#define panic l_panic
{{FILE:3rd_lua.h}}
#undef cast
#undef G
//---
{{FILE:3rd_stb_image.h}}
{{FILE:3rd_stb_image_write.h}}
//---
#undef freelist
#define STBTT_malloc(x,u)  ((void)(u),MALLOC(x))
#define STBTT_free(x,u)    ((void)(u),FREE(x))
#define NK_ASSERT ASSERT
#define NK_DTOA(s,n) strcpy(s, va("%f", n)) // override cos built-in nk_dtoa() will freeze while parsing UINT_MAX otherwise
{{FILE:3rd_nuklear.h}}
{{FILE:3rd_nuklear_glfw_gl3.h}}
{{FILE:3rd_nuklear_filebrowser.h}}
//---
#ifdef ENABLE_ASSIMP
//{{FILE/*:*/3rd_assimp.h}}
//#include "3rd_assimp/cimport.h"
//#include "3rd_assimp/scene.h"
//#include "3rd_assimp/postprocess.h"
//#pragma comment(lib, "3rd/3rd_assimp/x64/assimp")
#endif
{{FILE:3rd_json5.h}}
{{FILE:3rd_gjk.h}}
{{FILE:3rd_compress.h}}
{{FILE:3rd_archive.h}}
{{FILE:3rd_thread.h}}
{{FILE:3rd_plmpeg.h}}
{{FILE:3rd_jo_mpeg.h}}
//#define _RTL_RUN_ONCE _RTL_RUN_ONCE2 // __MINGW64__
{{FILE:3rd_https.h}}
#undef F2
#undef F3
{{FILE:3rd_enet.h}}
#define tls_init tls_init2
{{FILE:3rd_bq_websocket.h}}
{{FILE:3rd_simplex.h}}
{{FILE:3rd_tfd.h}}
{{FILE:3rd_stb_sprintf.h}}
#define g g2
{{FILE:3rd_xml.h}}
#undef g
{{FILE:3rd_polychop.h}}
#endif // FWK_3RD
