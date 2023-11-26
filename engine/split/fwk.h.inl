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
* ALTERNATIVE B - 0-BSD License (https://opensource.org/licenses/FPL-1.0.0)
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
* ------------------------------------------------------------------------------
* ALTERNATIVE C - MIT-0 (No Attribution clause)
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
* licensing terms listed above. I make this dedication for the benefit of the
* public at large and to the detriment of my heirs and successors. I intend
* this dedication to be an overt act of relinquishment in perpetuity of all
* present and future rights to this software under copyright law."
*
* Author (name)              I agree (YES/NO)    Files/Features (optional)
* ------------------------------------------------------------------------------
* @r-lyeh                    YES                 Initial codebase
* @zak@v4.games              YES                 N/A
* ------------------------------------------------------------------------------
*/

#ifndef FWK_H
#define FWK_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// Headers

{{FILE:fwk_config.h}}

{{FILE:fwk_ds.h}}

{{FILE:fwk_math.h}}

{{FILE:fwk_obj.h}}


{{FILE:fwk_ai.h}}

{{FILE:fwk_audio.h}}

{{FILE:fwk_collide.h}}

{{FILE:fwk_cook.h}}

{{FILE:fwk_data.h}}

{{FILE:fwk_extend.h}}

{{FILE:fwk_file.h}}

{{FILE:fwk_font.h}}

{{FILE:fwk_input.h}}

{{FILE:fwk_memory.h}}

{{FILE:fwk_network.h}}

{{FILE:fwk_netsync.h}}

{{FILE:fwk_pack.h}}

{{FILE:fwk_profile.h}}

{{FILE:fwk_reflect.h}}

{{FILE:fwk_render.h}}

{{FILE:fwk_renderdd.h}}

{{FILE:fwk_scene.h}}

{{FILE:fwk_string.h}}

{{FILE:fwk_sprite.h}}
{{FILE:fwk_gui.h}}

{{FILE:fwk_steam.h}}

{{FILE:fwk_system.h}}

{{FILE:fwk_time.h}}

{{FILE:fwk_ui.h}}

{{FILE:fwk_video.h}}

{{FILE:fwk_window.h}}

// ----

{{FILE:fwk_editor.h}}

// ----

#if is(cpp)
} // extern "C"
#endif

// expose glfw/glad apis
#if is(ems)
    #include <GL/glew.h>
    #include <GLFW/glfw3.h>
    #include <emscripten.h>
    #include <emscripten/html5.h>
    #define gladLoadGL(func) (glewExperimental = true, glewInit() == GLEW_OK) ///-
#else
    #if is(win32) /*&& is(tcc)*/ // && ENABLE_DLL
    #ifdef GLAD_API_CALL
    #undef GLAD_API_CALL
    #endif
    #define GLAD_API_CALL extern API ///-
    #endif
    #ifndef GLAD_GL_H_
    #include "fwk"
    #endif
#endif

#endif // FWK_H
