/* game framework.
*  - rlyeh, public domain
*
* ## FWK License
*
* This software is available under 3 licenses. Choose whichever you prefer.
* ------------------------------------------------------------------------------
* ALTERNATIVE A - Public Domain (https://unlicense.org/)
* ------------------------------------------------------------------------------
* This is free and unencumbered software released into the public domain.
*
* Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
* software, either in source code form or as a compiled binary, for any purpose,
* commercial or non-commercial, and by any means.
*
* In jurisdictions that recognize copyright laws, the author or authors of this
* software dedicate any and all copyright interest in the software to the public
* domain. We make this dedication for the benefit of the public at large and to
* the detriment of our heirs and successors. We intend this dedication to be an
* overt act of relinquishment in perpetuity of all present and future rights to
* this software under copyright law.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
* ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
* ------------------------------------------------------------------------------
* ALTERNATIVE B - MIT-0 (No Attribution clause)
* ------------------------------------------------------------------------------
* Permission is hereby granted, free of charge, to any person obtaining a copy of this
* software and associated documentation files (the "Software"), to deal in the Software
* without restriction, including without limitation the rights to use, copy, modify,
* merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
* ------------------------------------------------------------------------------
* ALTERNATIVE C - Zero BSD License (https://opensource.org/licenses/FPL-1.0.0)
* ------------------------------------------------------------------------------
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
* REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
* FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
* INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
* LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
* OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
* PERFORMANCE OF THIS SOFTWARE.
*
* ## License: Contributed Code ------------------------------------------------
*
* Dear Contributor,
*
* In order to ensure this project remains completely free and unencumbered by
* anyone's copyright monopoly, it is advisable that you dedicate your code-base
* contributions to the three licensing terms above. This removes any possible
* ambiguity as to what terms somebody might have thought they were contributing
* under, in case of a future dispute. These concerns are not unique to public
* domain software. Most large, established open-source projects have a
* Contributor License Agreement (CLA) process, of varying degrees of formality.
*
* Please add yourself to the list below before contributing.
* Thanks.
*
* --
*
* "I dedicate any and all copyright interest in this software to the three
* license terms listed above. I make this dedication for the benefit of the
* public at large and to the detriment of my heirs and successors. I intend
* this dedication to be an overt act of relinquishment in perpetuity of all
* present and future rights to this software under copyright law."
*
* Author (name)              I agree (YES/NO)    Files/Features (optional)
* ------------------------------------------------------------------------------
* @r-lyeh                    YES                 Initial codebase
* ------------------------------------------------------------------------------
*/

#define GLAD_GL_IMPLEMENTATION              // glad

#include "fwk.h"

//-----------------------------------------------------------------------------
// 3rd party libs

#define ARCHIVE_C                           // archive.c
#define COMPRESS_C                          // compress.c
#define ENET_IMPLEMENTATION                 // enet
#define GJK_C                               // gjk
#define LSH_GLFW_IMPLEMENTATION             // glfw
#define GLFW_INCLUDE_NONE                   // glfw
#define HTTPS_IMPLEMENTATION                // https
#define JAR_MOD_IMPLEMENTATION              // jar_mod
#define JAR_XM_IMPLEMENTATION               // jar_xm
#define JO_MPEG_COMPONENTS 3                // jo_mpeg
#define JSON5_C                             // json5
#define LUA_IMPL                            // lua542
#define MINIAUDIO_IMPLEMENTATION            // miniaudio
#define NK_GLFW_GL3_IMPLEMENTATION          // nuklear
#define NK_IMPLEMENTATION                   // nuklear
#define NK_INCLUDE_DEFAULT_ALLOCATOR        // nuklear
#define NK_INCLUDE_DEFAULT_FONT             // nuklear
#define NK_INCLUDE_FIXED_TYPES              // nuklear
#define NK_INCLUDE_FONT_BAKING              // nuklear
#define NK_INCLUDE_STANDARD_IO              // nuklear
#define NK_INCLUDE_STANDARD_VARARGS         // nuklear
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT     // nuklear
#define NK_KEYSTATE_BASED_INPUT             // nuklear
#define PL_MPEG_IMPLEMENTATION              // pl_mpeg
#define STB_IMAGE_IMPLEMENTATION            // stbi
#define STB_IMAGE_WRITE_IMPLEMENTATION      // stbi_write
#define STS_MIXER_IMPLEMENTATION            // sts_mixer
#define SIMPLEX_C                           // simplex
#define SWRAP_IMPLEMENTATION                // swrap
#define SWRAP_STATIC                        // swrap
#define THREAD_IMPLEMENTATION               // thread
#define BQ_PLATFORM_IMPLEMENTATION          // websocket
#define BQ_WEBSOCKET_IMPLEMENTATION         // websocket
#ifdef __APPLE__
#define _GLFW_COCOA                         // glfw osx
#elif defined _WIN32
#define _GLFW_WIN                           // glfw win32
#else
#define _GLFW_X11                           // glfw linux, also _GLFW_MESA + LSH_GLFW_OSMESA
#define LSH_GLFW_X11                        // or _GLFW_WAYLAND + LSH_GLFW_WAYLAND as well
#endif

{{FILE:fwk_compat.h}}

//---
{{FILE:3rd_glfw3.h}}
#undef timeGetTime
//---
{{FILE:3rd_swrap.h}}
//---
{{FILE:3rd_jo_mp1.h}}
{{FILE:3rd_sfxr.h}}
#define get_bits stb_vorbis_get_bits
#define error stb_vorbis_error
{{FILE:3rd_stb_vorbis.h}}
#undef error
#define channel jar_channel
{{FILE:3rd_jar_mod.h}}
#undef channel
#undef DEBUG
{{FILE:3rd_jar_xm.h}}
{{FILE:3rd_sts_mixer.h}}
{{FILE:3rd_miniaudio.h}}
//---
#undef L
#undef C
#undef R
#define error l_error
#define panic l_panic
{{FILE:3rd_lua543.h}}
#undef cast
#undef G
//---
{{FILE:3rd_stb_image.h}}
{{FILE:3rd_stb_image_write.h}}
//---
#undef freelist
{{FILE:3rd_nuklear.h}}
{{FILE:3rd_nuklear_glfw_gl3.h}}
//---
#ifdef WITH_ASSIMP
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

//-----------------------------------------------------------------------------
// C files

{{FILE:fwk_ds.c}}
{{FILE:fwk_string.c}}
{{FILE:fwk_compat.c}}
{{FILE:fwk_profile.c}}

#define  atexit(...) // hack to boost exit time. there are no critical systems that need to shutdown properly
{{FILE:fwk_audio.c}}
{{FILE:fwk_collide.c}}
{{FILE:fwk_cooker.c}}
{{FILE:fwk_data.c}}
{{FILE:fwk_file.c}}
{{FILE:fwk_input.c}}
{{FILE:fwk_math.c}}
{{FILE:fwk_memory.c}}
{{FILE:fwk_network.c}}
{{FILE:fwk_render.c}}
{{FILE:fwk_renderdd.c}}
{{FILE:fwk_scene.c}}
{{FILE:fwk_script.c}}
{{FILE:fwk_system.c}}
{{FILE:fwk_ui.c}}
{{FILE:fwk_video.c}}
{{FILE:fwk_window.c}}
{{FILE:fwk_editor.c}}
// editor is last in place, so it can use all internals from above headers

{{FILE:fwk_main.c}}
