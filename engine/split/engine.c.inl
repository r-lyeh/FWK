/* game framework.
*  - rlyeh, public domain
*
* ## License
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

#ifndef ENGINE_H
#include "engine.h"
#endif

#if is(win32) // hack to boost exit time. there are no critical systems that need to shutdown properly on Windows,
#define atexit(...) // however Linux needs proper atexit() to deinitialize 3rd_miniaudio.h; likely OSX as well.
#endif

#ifndef ENGINE_3RD
#define ENGINE_3RD
#include "engine"
#endif

//-----------------------------------------------------------------------------
// C files

{{FILE:engine_begin.c}}

{{FILE:engine_ds.c}}

{{FILE:engine_string.c}}

{{FILE:engine_compat.c}}

{{FILE:engine_ui.c}}


{{FILE:engine_audio.c}}

{{FILE:engine_collide.c}}

{{FILE:engine_cook.c}}

{{FILE:engine_data.c}}

{{FILE:engine_extend.c}}

{{FILE:engine_file.c}}

{{FILE:engine_font.c}}

{{FILE:engine_gui.c}}

{{FILE:engine_hash.c}}

{{FILE:engine_input.c}}

{{FILE:engine_math.c}}

{{FILE:engine_memory.c}}

{{FILE:engine_network.c}}

{{FILE:engine_netsync.c}}

{{FILE:engine_pack.c}}

{{FILE:engine_reflect.c}}

{{FILE:engine_render.c}}

{{FILE:engine_renderdd.c}}

{{FILE:engine_scene.c}}

{{FILE:engine_sprite.c}}

{{FILE:engine_steam.c}}

{{FILE:engine_system.c}}

{{FILE:engine_time.c}}

{{FILE:engine_profile.c}}

{{FILE:engine_video.c}}

{{FILE:engine_window.c}}

{{FILE:engine_obj.c}}

{{FILE:engine_ai.c}}

{{FILE:engine_editor0.c}}

{{FILE:engine_main.c}}

// editor is last in place, so it can use all internals from above headers
{{FILE:engine_editor.c}}
{{FILE:engine_editor_scene.h}}
{{FILE:engine_editor_browser.h}}
{{FILE:engine_editor_timeline.h}}
{{FILE:engine_editor_console.h}}
{{FILE:engine_editor_nodes.h}}
{{FILE:engine_editor_script.h}}

{{FILE:engine_end.c}}
