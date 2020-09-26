<h1 align="center">F·W·K</h1>
<p align="center">Minimalist framework in plain C for quick 3D game development.</p>

<!--## Gallery-->
<p align="center">
<img src="https://i.imgur.com/sInbRoA.gif"/><br/>
</p>

## Goals
- [x] ~C++~. C.
- [x] ~Fast~. Naive.
- [x] ~Modern~. Simple.
- [x] ~Full featured~. Small.
- [x] ~Royaltie fee~. Free and unlicensed.

## Features ᕦ(ᐛ)ᕤ
- [x] OS: Windows, Linux and OSX.
- [x] Compiler: MSVC, MINGW64, TCC, GCC and clang.
- [x] Window: fullscreen, msaa, cursor handling.
- [x] Input: keyboard, mouse and gamepads.
- [x] Script: Lua, bindings ~~and FFI~~.
- [x] Network: downloads (HTTPS), sockets (TCP/UDP) ~~and messages (enet/websocket)~~.
- [x] UI: color3/4, button, list, slider, toggle, checkbox, editbox, dialog, menus.
- [x] Math: rand, noise, ease, vec2/3/4, mat33/34/44, quat ~~and quat2, bezier, catmull~~.
- [x] Geometry: ray, line, plane, aabb, sphere, capsule, triangle, poly and frustum.
- [x] Image: JPG, PNG, TGA, BMP, PSD, HDR, PIC, PNM ~~and GIF~~.
- [x] Texture: BCn 1/2/3/~~4/5/6/7, DDS, KTX, PVR, PKM, ETC~~.
- [x] Audio: WAV, MOD, XM, FLAC, OGG, MP1, MP3, MID ~~and SFXR~~.
- [x] Video: MP4, MPG, OGV, MKV, WMV and AVI.
- [x] Model: IQM, GLTF/2, FBX, OBJ, DAE, BLEND, MD3, MD5, MS3D, SMD, X, 3DS, BVH, DXF, LWO.
- [x] Render: 2D Sprites, spritesheets, batching, ~~tiled and 2D Spines~~.
- [x] Render: 3D Anims, skeletal anims and hardware skinning.
- [x] Render: 3D Debugdraw, batching and vectorial font.
- [x] Render: Cubemaps, panoramas and spherical harmonics. Rayleigh/Mie scattering.
- [x] Render: Post-effects (SSAO,FXAA1/3,CRT,Contrast,Grain,Outline,Vignette...) ~~and shadertoys~~.
- [x] Compression: DEFLATE, LZMA, LZ4, ULZ, BALZ, BCM, CRUSH, LZW3, LZSS and PPP.
- [x] Virtual filesystem: ZIP, PAK, TAR and DIR.
- [x] Disk cache.
- [x] Scene handling.
- [x] Integrated asset pipeline.
- [x] Profiler, stats and leaks finder.
- [x] Level data: JSON5 ~~and XML~~.

## Roadmap ᕕ(ᐛ)ᕗ
- [ ] Level objects: ~~volumes, triggers, platforms, streaming~~.
- [ ] Level editor: gizmos, ~~scene tree, property editor, load/save~~.
- [ ] Render: Shadow mapping and baked lightmaps.
- [ ] Render: Materials (textures, matcaps, videos, shadertoys).
- [ ] Network: message api and pub/sub wrappers.
- [ ] Pipeline: Extend asset pipeline (shaders, bindings, xml, textures). Asset options.
- [ ] Script: Teal and typescript.
- [ ] Script: Refl/meta tool (during cook stage).
- [ ] API: Review API and lock it.
- [ ] API: Docs.
- [ ] Repo: first release.

<!--
//
// almost done:
// [x] shadertoy textures
// [ ] xml
// [ ] billboards
// [ ] soft shadows (vsm, vsmcube)
// [ ] enet, websocket
//
// roadmap:
// 1) lighting: vsm,vsmcube (per @procedural's request)
//    2d: billboards
// 2) fixing leaks
//    extending asset pipeline
//      cook: img2png (spot, stbi), tex2pvr (spot, soil2)
//      cook: hlsl2glsl (XShaderCompiler), hlsl2spirv (dxc), spirv2many (spirv-cross), glsl2many (sokol-shdc)
//      cook: ogg2wav, mp32wav, mp22wav, flac2wav
//      cook: xml2json, mp2json
//      cook: tiled2json, spine2json, tlfx22json
// 3) lock API and release first stable version.
//    modules: script or dll + ram load/save/diff/patch + play/stop/init/ + attach/detach
//    logic tree/ << [] |> || >>
//    - scene |>
//       - enemies
// 4) (json) editor: load/save jsons, property editor for anything (remote osc server/client)
//    cam: friction, projections (dimetric, isometric, ...)
//    gizmo: proportional, arcball XY (+shift for Z/tilt)
//    scene: scenegraph, obj naming, ~~obj picking, obj bounds,~~ obj collisions, obj/scene streaming
//    ecs: sys are modules, ecs: messaging, ecs: filesystem (e/dir,c/files,s/dll)
//    world: streaming, migration
//    gfx: tessellation
//    placeholders google
//    vcs
// X) level
//    scripts: states, signals, h/fsm, coroutines, load/save context
//    ai: h/fsm for level objects and small enemies, planning GOAL, BTrees,
//    nav: navpaths, waypoints, navigation, pathfinding (jps.hh), a*, swarm/flocks,
//    physics: emitters: particles, lights, lightmaps, sound sources, triggers, etc
//    level: box triggers, start/end, spawn, streaming, checkpoints,
//    level: jump, shoots, platforms, collisions
//    level: 60s, 70s, 80s, 90s
// 6) anims, I (playlist: forward/backwards/loop/rewind), II (blend/shapes), III (ik/bone), IV (graph/controller)
//    blend anims, animtracks+animevents, draw skeleton, additive anims,
//    fwk_data: quantization: ~~half, quant, microfloat~~.
//    anim; keyframes[] { frame+delay,frame+delay,... }, anim duration, anim flip
// 7) network replication & messaging
//    network: replication, dead reckoning, interpolation, extrapolation, bandwidth
//    network: messaging: un/reliable, fragmentation, priority, etc
//    network: topologies: bus, star, p2p, pubsub, etc
//    network: filesystem
//    int send_game_state(void *ptr, int len, int flags); PROTOCOL_V1|QUANTIZE|COMPRESS|RLE
//    int recv_game_state(); compensate, extrapolate, intrapolate(); lerp();
// 8) vm/ecs core + engines: custom frontends & backends
//    vm: ram, workqueues, threading, priorities
//    service protocols: websocket bqqbarbhg/bq_websocket, https, handshake
//    databases, services, quotas, black/whitelists, etc
// 9) render+
//    PBR/IBL/materials (from Foxotron+sgorsten) + shading models
//    lightmapping/xatlas (demos), reflection probes
//    instancing, frustum culling, impostors, mesh lods,
//    renderbuckets
//    decals
//    2d: spines, particles (tlfx2)
//    reverse-z {
//    fbo attach format D16_UNORM -> D32_SFLOAT
//    pipeline depth compare LEQUAL -> GEQUAL
//    pipeline depth clear 1.0 -> 0.0
//    proj matrix: float a = zfar / (zfar - znear); -> float a = -znear / (zfar - znear);
//    proj matrix: float b = (-znear * zfar) / (zfar - znear); -> float b = (znear * zfar) / (zfar - znear);
//    }
//10) fwk_app: app state manager, options, app icon, cpu usage, battery, orientation
//    fwk_input: cursor, mouse clip, mouse wrap,
//    animated textures (shadertoys)
//    sprites: pixel perfect zooming (demo+wheel)
//    zip0 seek-vfs optimization
// 11) fwk_cook (*.c, *.h) as .proto/.pbc maybe, free reflection+automatic bindings

-->

## Build
Type `MAKE.bat` (Win) or `sh MAKE.bat` (Linux/OSX) to build everything. Alternatively,

```bash
cl  demo.c       fwk.c                                                         && echo Windows (vc+tcc)
gcc demo.c       fwk.c -o demo -w -lws2_32 -lgdi32 -lwinmm -ldbghelp -std=c99  && echo Windows (mingw64)
cc  demo.c       fwk.c -o demo -w -lm -ldl -lpthread                           && echo Linux (gcc+clang+tcc)
cc  demo.c -ObjC fwk.c -o demo -w -framework cocoa -framework iokit            && echo OSX (gcc+clang)
```
<!-- - Note: Windows: Assimp.dll may need [this package installed](https://www.microsoft.com/en-us/download/confirmation.aspx?id=30679).-->
- Note: TCC is partially supported on Windows+Linux. Beware, no threading.

## Amalgamation
- Split `fwk.h` into separate files by running `art/tools/split.bat` (or `sh art/tools/split.bat` in Linux/OSX).
- Merge those files back into `fwk.h` by running `art/tools/join.bat` (or `sh art/tools/join.bat` in Linux/OSX).
- Optionally, generate a single-header distribution by executing following script:
```
type fwk.h            > fwk-single-header.h
echo #ifdef FWK_C    >> fwk-single-header.h
echo #pragma once    >> fwk-single-header.h
type fwk.c           >> fwk-single-header.h
echo #endif // FWK_C >> fwk-single-header.h
```

## Extra tips
- Dropped files into game window will be saved in [`art/import`](art/import) folder.
- Linux/OSX users can optionally install wine to use the Windows pipeline as an alternate asset pipeline.

## License
This software is released into the [public domain](https://unlicense.org/).<br/>
Also dual-licensed as [0-BSD](https://opensource.org/licenses/FPL-1.0.0) or [MIT (No Attribution)](https://github.com/aws/mit-0) for those countries where public domain is a concern (sigh).<br/>
Any contribution to this repository is implicitly subjected to the same release conditions aforementioned.

## Credits (Artwork + demos)
- [FMS_Cat](https://gist.github.com/FMS-Cat/a1ccea3ce866c34706084e3526204f4f), for nicest VHS/VCR shader around (MIT).
- [Goblin165cm](https://sketchfab.com/3d-models/halloween-little-witch-ccc023590bfb4789af9322864e42d1ab), for witch 3D model (CC BY 4.0).
- [Nanofactory](https://sketchfab.com/3d-models/kgirls01-d2f946f58a8040ae993cda70c97b302c), for kgirls01 3D model (CC BY-NC-ND 4.0).
- [Quaternius](https://www.patreon.com/quaternius), for the lovely 3D robots (CC0).
- [Rxi](https://github.com/rxi/autobatch), for lovely sprites & cats demo (MIT).
- [wwwtyro](https://github.com/wwwtyro/glsl-atmosphere), for nicest rayleigh/mie scattering shader around (CC0).

## Credits (Tools)
- [Assimp authors](https://github.com/assimp/assimp), for assimp (BSD3).
- [Bernhard Schelling](https://github.com/schellingb/TinySoundFont), for tml.h (Zlib) and tsf.h (MIT).
- [Krzysztof Gabis](https://github.com/kgabis/ape), for split.py/join.py (MIT).
- [Mattias Gustavsson](https://github.com/mattiasgustavsson/libs), for mid.h (PD).
- [Tor Andersson](https://github.com/ccxvii/asstools), for assiqe.c (BSD).
- [Lee Salzman](https://github.com/lsalzman/iqm/tree/5882b8c32fa622eba3861a621bb715d693573420/demo), for iqm.cpp (PD).

## Credits (Runtime)
- [Barerose](https://github.com/barerose), for swrap (CC0).
- [Camilla Löwy](https://github.com/elmindreda), for glfw3 (Zlib).
- [Dave Rand](https://tools.ietf.org/html/rfc1978) for ppp (PD).
- [David Herberth](https://github.com/dav1dde/), for glad generated code (PD).
- [David Reid](https://github.com/mackron), for miniaudio (PD).
- [Dominic Szablewski](https://github.com/phoboslab/pl_mpeg), for pl_mpeg (MIT).
- [Dominik Madarász](https://github.com/zaklaus), for json5 parser (PD).
- [Eduard Suica](https://github.com/eduardsui/tlse), for tlse (PD).
- [Haruhiko Okumura](https://oku.edu.mie-u.ac.jp/~okumura/compression/) for lzss (PD).
- [Igor Pavlov](https://www.7-zip.org/) for LZMA (PD).
- [Ilya Muravyov](https://github.com/encode84) for bcm, balz, crush, ulz, lz4x (PD).
- [Jon Olick](https://www.jonolick.com/), for jo_mp1 and jo_mpeg (PD).
- [Joonas Pihlajamaa](https://github.com/jokkebk/JUnzip), for JUnzip library (PD).
- [Joshua Reisenauer](https://github.com/kd7tck), for jar_mod and jar_xm (PD).
- [Lee Salzman](https://github.com/lsalzman/iqm/tree/5882b8c32fa622eba3861a621bb715d693573420/demo), for IQM spec & player (PD).
- [Lee Salzman, V.Hrytsenko and D.Madarász](https://github.com/zpl-c/enet/), for enet (MIT).
- [Libtomcrypt](https://github.com/libtom/libtomcrypt), for libtomcrypt (Unlicense).
- [Lua authors](https://www.lua.org/), for Lua language (MIT).
- [Mattias Gustavsson](https://github.com/mattiasgustavsson/libs), for thread.h and https.h (PD).
- [Micha Mettke](https://github.com/vurtun/nuklear), for nuklear (PD).
- [Rich Geldreich](https://github.com/richgel999/miniz), for miniz (PD).
- [Ross Williams](http://ross.net/compression/lzrw3a.html) for lzrw3a (PD).
- [Samuli Raivio](https://github.com/bqqbarbhg/bq_websocket), for bq_websocket (PD).
- [Sean Barrett](https://github.com/nothings), for stb_image, stb_image_write and stb_vorbis (PD).
- [Sebastian Steinhauer](https://github.com/kieselsteini), for sts_mixer (PD).
- [Stefan Gustavson](https://github.com/stegu/perlin-noise), for simplex noise (PD).
- [Tomas Pettersson](http://www.drpetter.se/), for sfxr (PD).
- Special thanks to [@ands](https://github.com/ands) (PD), [@barerose](https://github.com/barerose) (CC0), [@datenwolf](https://github.com/datenwolf) (WTFPL2), [@evanw](https://github.com/evanw) (CC0), [@glampert](https://github.com/glampert) (PD), [@krig](https://github.com/krig) (CC0), [@sgorsten](https://github.com/sgorsten) (Unlicense) and [@vurtun](https://github.com/vurtun) (PD) for their math libraries.

## Links
<a href="https://github.com/r-lyeh/FWK/issues"><img alt="Issues" src="https://img.shields.io/github/issues-raw/r-lyeh/FWK.svg"/></a>
<a href="https://discord.gg/vu6Vt9d"><img alt="Discord" src="https://img.shields.io/badge/chat-FWK%20lounge-738bd7.svg?logo=discord"/></a>

Still looking for alternatives?
[amulet](https://github.com/ianmaclarty/amulet),
[aroma](https://github.com/leafo/aroma/),
[astera](https://github.com/tek256/astera),
[blendelf](https://github.com/jesterKing/BlendELF),
[bullordengine](https://github.com/MarilynDafa/Bulllord-Engine),
[candle](https://github.com/EvilPudding/candle),
[cave](https://github.com/kieselsteini/cave),
[chickpea](https://github.com/ivansafrin/chickpea),
[corange](https://github.com/orangeduck/Corange),
[cute](https://github.com/RandyGaul/cute_framework),
[ejoy2d](https://github.com/ejoy/ejoy2d),
[exengine](https://github.com/exezin/exengine),
[gunslinger](https://github.com/MrFrenik/gunslinger),
[hate](https://github.com/excessive/hate),
[island](https://github.com/island-org/island),
[juno](https://github.com/rxi/juno),
[l](https://github.com/Lyatus/L),
[lgf](https://github.com/Planimeter/lgf),
[limbus](https://github.com/redien/limbus),
[love](https://github.com/love2d/love/),
[lovr](https://github.com/bjornbytes/lovr),
[mini3d](https://github.com/mini3d/mini3d),
[mintaro](https://github.com/mackron/mintaro),
[mio](https://github.com/ccxvii/mio),
[opensource](https://github.com/w23/OpenSource),
[pez](https://github.com/prideout/pez),
[pixie](https://github.com/mattiasgustavsson/pixie),
[punity](https://github.com/martincohen/Punity),
[ricotech](https://github.com/dbechrd/RicoTech),
[rizz](https://github.com/septag/rizz),
[tigr](https://github.com/erkkah/tigr),
