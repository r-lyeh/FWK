// -----------------------------------------------------------------------------

// opengl

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3

#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_NOTIFICATION    0x826B
#define GL_DEBUG_SOURCE_API               0x8246
#define GL_DEBUG_TYPE_ERROR               0x824C
//
void glDebugCallback(uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int32_t length, const char * message, void * userdata) {
    // whitelisted codes (also: 131169, 131204).
    if( id == 131154 ) return; // Pixel-path performance warning: Pixel transfer is synchronized with 3D rendering.
    if( id == 131185 ) return; // Buffer object 2 (bound to GL_ELEMENT_ARRAY_BUFFER_ARB, usage hint is GL_STATIC_DRAW) will use VIDEO memory as the source for buffer object operations
    if( id == 131218 ) return; // Program/shader state performance warning: Vertex shader in program 9 is being recompiled based on GL state.
    if( id == 2 ) return; // INFO: API_ID_RECOMPILE_FRAGMENT_SHADER performance warning has been generated. Fragment shader recompiled due to state change. [ID: 2]

    const char * GL_ERROR_TYPE[] = { "ERROR", "DEPRECATED BEHAVIOR", "UNDEFINED DEHAVIOUR", "PORTABILITY", "PERFORMANCE", "OTHER" };
    const char * GL_ERROR_SOURCE[] = { "API", "WINDOW SYSTEM", "SHADER COMPILER", "THIRD PARTY", "APPLICATION", "OTHER" };
    const char * GL_ERROR_SEVERITY[] = { "HIGH", "MEDIUM", "LOW", "NOTIFICATION" };

    type = type - GL_DEBUG_TYPE_ERROR;
    source = source - GL_DEBUG_SOURCE_API;
    severity = severity == GL_DEBUG_SEVERITY_NOTIFICATION ? 3 : severity - GL_DEBUG_SEVERITY_HIGH;
    if(severity >= 2) return; // do not log low_severity or notifications

    PRINTF( "!%s:%s [ID: %u]\n", type == 0 ? "ERROR":"WARNING", message, id );
//  PANIC( "!%s:%s [ID: %u]\n", type == 0 ? "ERROR":"WARNING", message, id );
}
void glDebugEnable() {
    do_once {
    typedef void (*GLDEBUGPROC)(uint32_t, uint32_t, uint32_t, uint32_t, int32_t, const char *, const void *);
    typedef void (*GLDEBUGMESSAGECALLBACKPROC)(GLDEBUGPROC, const void *);
        void *func = glfwGetProcAddress("glDebugMessageCallback");
    void (*glDebugMessageCallback)(GLDEBUGPROC, const void *) = (GLDEBUGMESSAGECALLBACKPROC)func;
        if( func ) {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        glDebugMessageCallback((GLDEBUGPROC)glDebugCallback, NULL);
        }
    }
}

static
void glCopyBackbufferToTexture( texture_t *tex ) { // unused
    glActiveTexture( GL_TEXTURE0 + texture_unit() );
    glBindTexture( GL_TEXTURE_2D, tex->id );
    glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 0, 0, window_width(), window_height(), 0 );
}

// ----------------------------------------------------------------------------
// renderstate

renderstate_t renderstate() {
    renderstate_t state = {0};

    // Set default clear color to black
    state.clear_color[0] = 0.0f; // Red
    state.clear_color[1] = 0.0f; // Green
    state.clear_color[2] = 0.0f; // Blue
    state.clear_color[3] = 1.0f; // Alpha

    // Set default color mask to GL_TRUE
    state.color_mask[0] = GL_TRUE;
    state.color_mask[1] = GL_TRUE;
    state.color_mask[2] = GL_TRUE;
    state.color_mask[3] = GL_TRUE;

    // Set default clear depth to maximum distance
    state.clear_depth = 1.0;

    // Enable depth test by default with less or equal function
    state.depth_test_enabled = GL_TRUE;
    state.depth_write_enabled = GL_TRUE;
    state.depth_func = GL_LEQUAL;
    
    // Disable polygon offset by default
    state.polygon_offset_enabled = GL_FALSE;
    state.polygon_offset_factor = 0.0f;
    state.polygon_offset = 0.0f;

    // Disable blending by default
    state.blend_enabled = GL_FALSE;
    state.blend_func = GL_FUNC_ADD;
    state.blend_src = GL_ONE;
    state.blend_dst = GL_ZERO;

    // Disable culling by default but cull back faces
    state.cull_face_enabled = GL_FALSE;
    state.cull_face_mode = GL_BACK;

    // Disable stencil test by default
    state.stencil_test_enabled = GL_FALSE;
    state.stencil_func = GL_ALWAYS;
    state.stencil_op_fail = GL_KEEP;
    state.stencil_op_zfail = GL_KEEP;
    state.stencil_op_zpass = GL_KEEP;
    state.stencil_ref = 0;
    state.stencil_read_mask = 0xFFFFFFFF;
    state.stencil_write_mask = 0xFFFFFFFF;

    // Set default front face to counter-clockwise
    state.front_face = GL_CCW;

    // Set default line width
    state.line_smooth_enabled = GL_FALSE;
    state.line_width = 1.0f;

    // Set default point size
    state.point_size_enabled = GL_FALSE;
    state.point_size = 1.0f;

    // Set default polygon mode to fill
    state.polygon_mode_face = GL_FRONT_AND_BACK;
    state.polygon_mode_draw = GL_FILL;

    // Disable scissor test by default
    state.scissor_test_enabled = GL_FALSE;

    // Enable seamless cubemap by default
    state.seamless_cubemap = GL_TRUE;

    // Disable depth clamp by default
    state.depth_clamp_enabled = GL_FALSE;

    return state;
}

bool renderstate_compare(const renderstate_t *stateA, const renderstate_t *stateB) {
    return memcmp(stateA, stateB, sizeof(renderstate_t)) == 0;
}

uint32_t renderstate_checksum(const renderstate_t *state) {
    return 0;//hash_bin(state, sizeof(renderstate_t));
}

static renderstate_t last_rs;

void renderstate_apply(const renderstate_t *state) {
    if (state != NULL) {
        // Compare renderstates and bail if they are the same
        if (renderstate_compare(state, &last_rs)) {
            return;
        }

        // Store renderstate
        last_rs = *state;

        // Apply clear color
        glClearColor(state->clear_color[0], state->clear_color[1], state->clear_color[2], state->clear_color[3]);

        // Apply color mask
        glColorMask(state->color_mask[0], state->color_mask[1], state->color_mask[2], state->color_mask[3]);

        // Apply clear depth
        glClearDepth(state->clear_depth);

        // Apply depth test
        if (state->depth_test_enabled) {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(state->depth_func);
        } else {
            glDisable(GL_DEPTH_TEST);
        }

        // Apply polygon offset
        if (state->polygon_offset_enabled) {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(state->polygon_offset_factor, state->polygon_offset);
        } else {
            glDisable(GL_POLYGON_OFFSET_FILL);
        }        

        // Apply depth write
        glDepthMask(state->depth_write_enabled);

        // Apply blending
        if (state->blend_enabled) {
            glEnable(GL_BLEND);
            glBlendEquation(state->blend_func);
            glBlendFunc(state->blend_src, state->blend_dst);
        } else {
            glDisable(GL_BLEND);
        }

        // Apply culling @fixme
        if (state->cull_face_enabled) {
            glEnable(GL_CULL_FACE);
            glCullFace(state->cull_face_mode);
        } else {
            glDisable(GL_CULL_FACE);
        }

        // Apply stencil test
        if (state->stencil_test_enabled) {
            glEnable(GL_STENCIL_TEST);
            glStencilMask(state->stencil_write_mask);
            glStencilFunc(state->stencil_func, state->stencil_ref, state->stencil_read_mask);
            glStencilOp(state->stencil_op_fail, state->stencil_op_zfail, state->stencil_op_zpass);
        } else {
            glDisable(GL_STENCIL_TEST);
        }

        // Apply front face direction @fixme
        glFrontFace(state->front_face);

        // Apply line width
        glLineWidth(state->line_width);

        // Apply smooth lines
        if (state->line_smooth_enabled) {
            glEnable(GL_LINE_SMOOTH);
        } else {
            glDisable(GL_LINE_SMOOTH);
        }

#if !is(ems)
        // Apply point size
        if (state->point_size_enabled) {
            glEnable(GL_PROGRAM_POINT_SIZE);
            glPointSize(state->point_size);
        } else {
            glDisable(GL_PROGRAM_POINT_SIZE);
        }

        // Apply polygon mode
        glPolygonMode(state->polygon_mode_face, state->polygon_mode_draw);
#endif

        // Apply scissor test
        if (state->scissor_test_enabled) {
            glEnable(GL_SCISSOR_TEST);
        } else {
            glDisable(GL_SCISSOR_TEST);
        }

        // Apply seamless cubemap
        if (state->seamless_cubemap) {
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        } else {
            glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        }

        // Apply depth clamp
        if (state->depth_clamp_enabled) {
            glEnable(GL_DEPTH_CLAMP);
        } else {
            glDisable(GL_DEPTH_CLAMP);
        }
    }
}

// ----------------------------------------------------------------------------
// shaders

void shader_print(const char *source) {
    for(int line = 0, i = 0; source[i] > 0; ) {
        printf("\t%03d: ", line+1);
        while( source[i] >= 32 || source[i] == '\t' ) fputc(source[i++], stdout);
        while( source[i] > 0 && source[i] < 32 ) line += source[i++] == '\n';
        puts("");
    }
}

// sorted by shader handle. an array of properties per shader. properties are plain strings.
static __thread map(unsigned, array(char*)) shader_reflect;

static
GLuint shader_compile( GLenum type, const char *source ) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char **)&source, NULL);
    glCompileShader(shader);

    GLint status = GL_FALSE, length;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if( status == GL_FALSE ) {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
//      ASSERT(length < 2048); char buf[2048] = { 0 };
        char *buf = stack(length+1);
        glGetShaderInfoLog(shader, length, NULL, buf);

        // dump log with line numbers
        shader_print( source );
        PANIC("!ERROR: shader_compile():\nDevice: %s\nDriver: %s\nShader/program link: %s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION), buf);
        return 0;
    }

    return shader;
}

unsigned shader(const char *vs, const char *fs, const char *attribs, const char *fragcolor, const char *defines){
    return shader_geom(NULL, vs, fs, attribs, fragcolor, defines);
}

static inline
char *shader_preprocess(const char *src, const char *defines) {
    if (!src) return NULL;

    const char *gles = "#version 300 es\n"
                       "#define textureQueryLod(t,uv) vec2(0.,0.)\n" // "#extension GL_EXT_texture_query_lod : enable\n"
                       "#define MEDIUMP mediump\n"
                       "precision MEDIUMP float;\n";
    
    char *processed_src = file_preprocess(src, NULL, vfs_read, "shader()");
    const char *desktop = strstr(processed_src, "textureQueryLod") ? "#version 400\n#define MEDIUMP\n" : "#version 330\n#define MEDIUMP\n";
    const char *glsl_version = ifdef(ems, gles, desktop);

    // detect GLSL version if set
    if (processed_src[0] == '#' && processed_src[1] == 'v') {
        #if 0
        const char *end = strstri(src, "\n");
        glsl_version = va("%.*s", (int)(end-src), src);
        src = end+1;
        #else
        PANIC("!ERROR: shader with #version specified on it. we do not support this anymore.");
        #endif
    }

    // char *preamble = vfs_read("shaderlib/compat.glsl");
    const char *preamble = ""
        "#ifndef HAS_TEXTURE_QUERY_LOD\n"
        "#define textureQueryLod(t,c) vec2(0.0,0.0)\n"
        "#else\n"
        "#extension GL_EXT_texture_query_lod : enable\n"
        "#endif\n";

    ASSERT(preamble);
    char *extensions = "";
    {
        // Check if GL_ARB_texture_query_lod extension is available
        if (ifdef(ems,0,GLAD_GL_ARB_texture_query_lod)) {
            extensions = va("%s", "#define HAS_TEXTURE_QUERY_LOD 1\n");
        }
    }

    char *dst = va("%s\n%s\n%s\n%s\n%s", glsl_version, extensions, preamble, defines ? defines : "", processed_src);
    return FREE(processed_src), dst;
}

unsigned shader_geom(const char *gs, const char *vs, const char *fs, const char *attribs, const char *fragcolor, const char *defines) {
    PRINTF(/*"!"*/"Compiling shader\n");

    char *glsl_defines = "";
    if( defines ) {
        for each_substring(defines, ",", def) {
            glsl_defines = va("%s#define %s\n", glsl_defines, def);
        }
    }

    if(gs)
    gs = shader_preprocess(gs, glsl_defines);
    vs = shader_preprocess(vs, glsl_defines);
    fs = shader_preprocess(fs, glsl_defines);

    GLuint vert = shader_compile(GL_VERTEX_SHADER, vs);
    GLuint frag = shader_compile(GL_FRAGMENT_SHADER, fs);
    GLuint geom = 0; if (gs) geom = shader_compile(GL_GEOMETRY_SHADER, gs);
    GLuint program = 0;

    if( vert && frag ) {
        program = glCreateProgram();

        glAttachShader(program, vert);
        glAttachShader(program, frag);
        if (geom) glAttachShader(program, geom);

        for( int i = 0; attribs && attribs[0]; ++i ) {
            char attrib[128] = {0};
            sscanf(attribs, "%127[^,]", attrib);
            while( attribs[0] && attribs[0] != ',' ) { attribs++; }
            while( attribs[0] && attribs[0] == ',' ) { attribs++; break; }
            if(!attrib[0]) continue;
            glBindAttribLocation(program, i, attrib);
            // PRINTF("Shader.attribute[%d]=%s\n", i, attrib);
        }

#if !is(ems) // @fixme
        if(fragcolor)
        glBindFragDataLocation(program, 0, fragcolor);
#endif

        glLinkProgram(program);

        GLint status = GL_FALSE, length;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
#ifdef DEBUG_SHADER
        if (status != GL_FALSE && program == DEBUG_SHADER) {
#else
        if (status == GL_FALSE) {
#endif
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            // ASSERT(length < 2048); char buf[2048] = { 0 };
            char *buf = stack(length+1);
            glGetProgramInfoLog(program, length, NULL, buf);
            puts("--- vs:");
            shader_print(vs);
            puts("--- fs:");
            shader_print(fs);
            if (geom) {
                puts("--- gs:");
                shader_print(gs);
            }

            PANIC("ERROR: shader():\nDevice: %s\nDriver: %s\nShader/program link: %s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION), buf);
        }

        glDeleteShader(vert);
        glDeleteShader(frag);
        if (geom) glDeleteShader(geom);

//#ifdef DEBUG_ANY_SHADER
//        PRINTF("Shader #%d:\n", program);
//        shader_print(vs);
//        shader_print(fs);
//#endif
    }

    glFinish();

/*
    if( s->program ) {
        strcatf(&s->name, "// vs (%s)\n%s\n\n\n", file_vs, vs);
        strcatf(&s->name, "// fs (%s)\n%s\n\n\n", file_fs, fs);
    }
*/

    // shader compiled fine, before returning, let's parse the source and reflect the uniforms
    array(char*) props = 0;
    do_once map_init_int( shader_reflect );
    if(vs) for each_substring(vs, "\r\n", line) {
        const char *found = strstr(line, "/""//");
        if( found > line && line[0] == '/' && line[1] == '/' ) continue;
        if( found ) array_push(props, STRDUP(line));
    }
    if(fs) for each_substring(fs, "\r\n", line) {
        const char *found = strstr(line, "/""//");
        if( found > line && line[0] == '/' && line[1] == '/' ) continue;
        if( found ) array_push(props, STRDUP(line));
    }
    if(gs) for each_substring(gs, "\r\n", line) {
        const char *found = strstr(line, "/""//");
        if( found > line && line[0] == '/' && line[1] == '/' ) continue;
        if( found ) array_push(props, STRDUP(line));
    }
    if( props ) {
        map_insert(shader_reflect, program, props);
        for( int i = 0; i < array_count(props); ++i ) shader_apply_param(program, i);
    }

    return program;
}

unsigned shader_properties(unsigned shader) {
    array(char*) *found = map_find(shader_reflect, shader);
    return found ? array_count(*found) : 0;
}

char** shader_property(unsigned shader, unsigned property) {
    array(char*) *found = map_find(shader_reflect, shader);
    return found && property < array_count(*found) ? &(*found)[property] : NULL;
}

void shader_apply_param(unsigned shader, unsigned param_no) {
    unsigned num_properties = shader_properties(shader);
    if( param_no < num_properties ) {
        char *buf = *shader_property(shader, param_no);

        char type[32], name[32], line[128]; snprintf(line, 127, "%s", buf);
        if( sscanf(line, "%*s %s %[^ =;/]", type, name) != 2 ) return;

        char *mins = strstr(line, "min:");
        char *sets = strstr(line, "set:");
        char *maxs = strstr(line, "max:");
        char *tips = strstr(line, "tip:");
        if( mins ) *mins = 0, mins += 4;
        if( sets ) *sets = 0, sets += 4;
        if( maxs ) *maxs = 0, maxs += 4;
        if( tips ) *tips = 0, tips += 4;

        int is_color = !!strstri(name, "color"), top = is_color ? 1 : 10;
        vec4 minv = mins ? atof4(mins) : vec4(0,0,0,0);
        vec4 setv = sets ? atof4(sets) : vec4(0,0,0,0);
        vec4 maxv = maxs ? atof4(maxs) : vec4(top,top,top,top);

        if(minv.x > maxv.x) swapf(&minv.x, &maxv.x);
        if(minv.y > maxv.y) swapf(&minv.y, &maxv.y);
        if(minv.z > maxv.z) swapf(&minv.z, &maxv.z);
        if(minv.w > maxv.w) swapf(&minv.w, &maxv.w);

        if( !maxs ) {
        if(setv.x > maxv.x) maxv.x = setv.x;
        if(setv.y > maxv.y) maxv.y = setv.y;
        if(setv.z > maxv.z) maxv.z = setv.z;
        if(setv.w > maxv.w) maxv.w = setv.w;
        }

        setv = clamp4(setv, minv, maxv);

        if( strchr("ibfv", type[0]) ) {
            GLint shader_bak; glGetIntegerv(GL_CURRENT_PROGRAM, &shader_bak);
            glUseProgram(shader);
            /**/ if(type[0] == 'i') glUniform1i(glGetUniformLocation(shader, name), setv.x);
            else if(type[0] == 'b') glUniform1i(glGetUniformLocation(shader, name), !!setv.x);
            else if(type[0] == 'f') glUniform1f(glGetUniformLocation(shader, name), setv.x);
            else if(type[3] == '2') glUniform2fv(glGetUniformLocation(shader, name), 1, &setv.x);
            else if(type[3] == '3') glUniform3fv(glGetUniformLocation(shader, name), 1, &setv.x);
            else if(type[3] == '4') glUniform4fv(glGetUniformLocation(shader, name), 1, &setv.x);
            glUseProgram(shader_bak);
        }
    }
}

void shader_apply_params(unsigned shader, const char *parameter_mask) {
    unsigned num_properties = shader_properties(shader);
    for( unsigned i = 0; i < num_properties; ++i ) {
        char *line = *shader_property(shader,i);

        char name[32];
        if( sscanf(line, "%*s %*s %s", name) != 1 ) continue;
        if( !strmatch(name, parameter_mask) ) continue;
        shader_apply_param(shader, i);
    }
}

int ui_shader(unsigned shader) {
    int changed = 0;

    unsigned num_properties = shader_properties(shader);
    for( unsigned i = 0; i < num_properties; ++i ) {
        char **ptr = shader_property(shader,i);

        char line[128]; snprintf(line, 127, "%s", *ptr); // debug: ui_label(line);

        char uniform[32], type[32], name[32], early_exit = '\0';
        if( sscanf(line, "%s %s %[^ =;/]", uniform, type, name) != 3 ) continue; // @todo optimize: move to shader()

        char *mins = strstr(line, "min:");
        char *sets = strstr(line, "set:");
        char *maxs = strstr(line, "max:");
        char *tips = strstr(line, "tip:");
        if( mins ) *mins = 0, mins += 4;
        if( sets ) *sets = 0, sets += 4;
        if( maxs ) *maxs = 0, maxs += 4;
        if( tips ) *tips = 0, tips += 4;

        if( strcmp(uniform, "uniform") && strcmp(uniform, "}uniform") ) { if(tips) ui_label(va(ICON_MD_INFO "%s", tips)); continue; } // @todo optimize: move to shader()

        int is_color = !!strstri(name, "color"), top = is_color ? 1 : 10;
        vec4 minv = mins ? atof4(mins) : vec4(0,0,0,0);
        vec4 setv = sets ? atof4(sets) : vec4(0,0,0,0);
        vec4 maxv = maxs ? atof4(maxs) : vec4(top,top,top,top);
        char *label = !tips ? va("%c%s", name[0] - 32 * !!(name[0] >= 'a'), name+1) :
            va("%c%s  " ICON_MD_INFO  "@%s", name[0] - 32 * !!(name[0] >= 'a'), name+1, tips);

        if(minv.x > maxv.x) swapf(&minv.x, &maxv.x); // @optimize: move to shader()
        if(minv.y > maxv.y) swapf(&minv.y, &maxv.y); // @optimize: move to shader()
        if(minv.z > maxv.z) swapf(&minv.z, &maxv.z); // @optimize: move to shader()
        if(minv.w > maxv.w) swapf(&minv.w, &maxv.w); // @optimize: move to shader()

        if( !maxs ) {
        if(setv.x > maxv.x) maxv.x = setv.x;
        if(setv.y > maxv.y) maxv.y = setv.y;
        if(setv.z > maxv.z) maxv.z = setv.z;
        if(setv.w > maxv.w) maxv.w = setv.w;
        }

        setv = clamp4(setv, minv, maxv);

        // supports int,float,vec2/3/4,color3/4
        int touched = 0;
        if( type[0] == 'b' ) {
            bool v = !!setv.x;

            if( (touched = ui_bool(label, &v)) != 0 ) {
                setv.x = v;
            }
        }
        else if( type[0] == 'i' ) {
            int v = setv.x;

            if( (touched = ui_int(label, &v)) != 0 ) {
                setv.x = clampi(v, minv.x, maxv.x); // min..max range
            }
        }
        else if( type[0] == 'f' ) {
            setv.x = clampf(setv.x, minv.x, maxv.x);
            char *caption = va("%5.3f", setv.x);
            setv.x = (setv.x - minv.x) / (maxv.x - minv.x);

            if( (touched = ui_slider2(label, &setv.x, caption)) != 0 ) {
                setv.x = clampf(minv.x + setv.x * (maxv.x-minv.x), minv.x, maxv.x); // min..max range
            }
        }
        else if( type[0] == 'v' && type[3] == '2' ) {
            setv.xy = clamp2(setv.xy,minv.xy,maxv.xy);

            if( (touched = ui_float2(label, &setv.x)) != 0 ) {
                setv.xy = clamp2(setv.xy,minv.xy,maxv.xy);
            }
        }
        else if( type[0] == 'v' && type[3] == '3' ) {
            setv.xyz = clamp3(setv.xyz,minv.xyz,maxv.xyz);

            if( (touched = (is_color ? ui_color3f : ui_float3)(label, &setv.x)) != 0 ) {
                setv.xyz = clamp3(setv.xyz,minv.xyz,maxv.xyz);
            }
        }
        else if( type[0] == 'v' && type[3] == '4' ) {
            setv = clamp4(setv,minv,maxv);

            if( (touched = (is_color ? ui_color4f : ui_float4)(label, &setv.x)) != 0 ) {
                setv = clamp4(setv,minv,maxv);
            }
        }
        else if( tips ) ui_label( tips );

        if( touched ) {
            // upgrade value
            *ptr = FREE(*ptr);
            *ptr = stringf("%s %s %s ///set:%s min:%s max:%s tip:%s", uniform,type,name,ftoa4(setv),ftoa4(minv),ftoa4(maxv),tips?tips:"");

            // apply
            shader_apply_param(shader, i);

            changed = 1;
        }
    }

    return changed;
}

int ui_shaders() {
    if( !map_count(shader_reflect) ) return ui_label(ICON_MD_WARNING " No shaders with annotations loaded."), 0;

    int changed = 0;
    for each_map_ptr(shader_reflect, unsigned, k, array(char*), v) {
        int open = 0, clicked_or_toggled = 0;
        char *id = va("##SHD%d", *k);
        char *title = va("Shader %d", *k);
        for( int p = (open = ui_collapse(title, id)), dummy = (clicked_or_toggled = ui_collapse_clicked()); p; ui_collapse_end(), p = 0) {
            ui_label(va("Shader %d",*k));
            changed |= ui_shader(*k);
        }
    }
    return changed;
}

unsigned compute(const char *cs){
    #if is(ems)
    return 0;
    #else
    PRINTF(/*"!"*/"Compiling compute shader\n");

    cs = cs[0] == '#' && cs[1] == 'c' ? cs : va("#version 450 core\n%s", cs ? cs : "");

    GLuint comp = shader_compile(GL_COMPUTE_SHADER, cs);
    GLuint program = 0;

    if( comp ) {
        program = glCreateProgram();

        glAttachShader(program, comp);

        glLinkProgram(program);

        GLint status = GL_FALSE, length;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
#ifdef DEBUG_SHADER
        if (status != GL_FALSE && program == DEBUG_SHADER) {
#else
        if (status == GL_FALSE) {
#endif
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            char *buf = stack(length+1);
            glGetProgramInfoLog(program, length, NULL, buf);
            puts("--- cs:");
            shader_print(cs);
        }
        if (status == GL_FALSE) {
            PANIC("ERROR: shader(): Shader/program link: %s\n", buf);
            return 0;
        }

        glDeleteShader(comp);
    }
    return program;
    #endif
}

void compute_dispatch(unsigned wx, unsigned wy, unsigned wz){
    glDispatchCompute(wx, wy, wz);
}

void write_barrier(){
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void write_barrier_image(){
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void shader_destroy(unsigned program){
    if( program == ~0u ) return;
    glDeleteProgram(program);
// if(s->name) FREE(s->name), s->name = NULL;
}

unsigned ssbo_create(const void *data, int len, unsigned usage){
    static GLuint gl_usage[] = { GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY, GL_DYNAMIC_DRAW, GL_DYNAMIC_READ, GL_DYNAMIC_COPY, GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY };
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, len, data, gl_usage[usage]);
    return ssbo;
}

void ssbo_destroy(unsigned ssbo){
    glDeleteBuffers(1, &ssbo);
}

void ssbo_update(int offset, int len, const void *data){
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, len, data);
}

void *ssbo_map(unsigned access){
    static GLenum gl_access[] = {GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE};
    return glMapBuffer(GL_SHADER_STORAGE_BUFFER, gl_access[access]);
}
void ssbo_unmap(){
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void ssbo_bind(unsigned ssbo, unsigned unit){
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, unit, ssbo);
}

void ssbo_unbind(){
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

// ubo

unsigned ubo_create(void *data, int size, unsigned usage) {
    static GLuint gl_usage[] = { GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY, GL_DYNAMIC_DRAW, GL_DYNAMIC_READ, GL_DYNAMIC_COPY, GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY };
    GLuint ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, size, data, gl_usage[usage]);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    return ubo;
}

void ubo_update(unsigned ubo, int offset, void *data, int size) {
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void ubo_bind(unsigned ubo, unsigned unit) {
    glBindBufferBase(GL_UNIFORM_BUFFER, unit, ubo);
}

void ubo_unbind(unsigned unit) {
    glBindBufferBase(GL_UNIFORM_BUFFER, unit, 0);
}

void ubo_destroy(unsigned ubo) {
    glDeleteBuffers(1, &ubo);
}

// shaders

static __thread unsigned last_shader = -1;

int shader_uniform(const char *name) {
    return glGetUniformLocation(last_shader, name);
}
unsigned shader_get_active() { return last_shader; }
unsigned shader_bind(unsigned program) { unsigned ret = last_shader; return glUseProgram(last_shader = program), ret; }
static inline void shader_int_(int uniform, int i)     { glUniform1i(uniform, i); }
static inline void shader_float_(int uniform, float f) { glUniform1f(uniform, f); }
static inline void shader_vec2_(int uniform, vec2 v)   { glUniform2fv(uniform, 1, &v.x); }
static inline void shader_vec3_(int uniform, vec3 v)   { glUniform3fv(uniform, 1, &v.x); }
static inline void shader_vec3v_(int uniform, int count, vec3 *v) { glUniform3fv(uniform, count, &v[0].x); }
static inline void shader_vec4_(int uniform, vec4 v)   { glUniform4fv(uniform, 1, &v.x); }
static inline void shader_mat44_(int uniform, mat44 m) { glUniformMatrix4fv(uniform, 1, GL_FALSE/*GL_TRUE*/, m); }
static inline void shader_cubemap_(int sampler, unsigned texture) { 
    int id = texture_unit();
    glUniform1i(sampler, id); 
    glActiveTexture(GL_TEXTURE0 + id); 
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
}
static inline void shader_bool_(int uniform, bool x) { glUniform1i(uniform, x); }
static inline void shader_uint_(int uniform, unsigned x ) { glUniform1ui(uniform, x); }
static inline void shader_texture_unit_kind_(int kind, int sampler, unsigned id, unsigned unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(kind, id);
    glUniform1i(sampler, unit);
}
static inline void shader_texture_unit_(int sampler, unsigned id, unsigned unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id);
    glUniform1i(sampler, unit);
}
static inline void shader_texture_(int sampler, texture_t t) { shader_texture_unit_(sampler, t.id, texture_unit()); }

// public api
void shader_int(const char *uniform, int i)     { glUniform1i(shader_uniform(uniform), i); }
void shader_float(const char *uniform, float f) { glUniform1f(shader_uniform(uniform), f); }
void shader_vec2(const char *uniform, vec2 v)   { glUniform2fv(shader_uniform(uniform), 1, &v.x); }
void shader_vec3(const char *uniform, vec3 v)   { glUniform3fv(shader_uniform(uniform), 1, &v.x); }
void shader_vec3v(const char *uniform, int count, vec3 *v) { glUniform3fv(shader_uniform(uniform), count, &v[0].x); }
void shader_vec4(const char *uniform, vec4 v)   { glUniform4fv(shader_uniform(uniform), 1, &v.x); }
void shader_mat44(const char *uniform, mat44 m) { glUniformMatrix4fv(shader_uniform(uniform), 1, GL_FALSE/*GL_TRUE*/, m); }
void shader_cubemap(const char *sampler, unsigned texture) { 
    int id = texture_unit();
    glUniform1i(shader_uniform(sampler), id); 
    glActiveTexture(GL_TEXTURE0 + id); 
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
}
void shader_bool(const char *uniform, bool x) { glUniform1i(shader_uniform(uniform), x); }
void shader_uint(const char *uniform, unsigned x ) { glUniform1ui(shader_uniform(uniform), x); }
void shader_texture(const char *sampler, texture_t t) { shader_texture_unit(sampler, t.id, texture_unit()); }
void shader_texture_unit(const char *sampler, unsigned id, unsigned unit) {
    glUniform1i(shader_uniform(sampler), unit);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id);
}
void shader_image(texture_t t, unsigned unit, unsigned level, int layer /* -1 to disable layered access */, unsigned access){
    shader_image_unit(t.id, unit, level, layer, t.texel_type, access);
}
void shader_image_unit(unsigned texture, unsigned unit, unsigned level, int layer, unsigned texel_type, unsigned access){
    static GLenum gl_access[] = {GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE};
    glBindImageTexture(unit, texture, level, layer!=-1, layer!=-1?layer:0, gl_access[access], texel_type);
}

void shader_colormap(const char *name, colormap_t c ) {
    // assumes shader uses `struct { vec4 color; bool has_tex } name + sampler2D name_tex;`
    shader_vec4( va("%s.color", name), c.color );
    shader_bool( va("%s.has_tex", name), c.texture != NULL );
    if( c.texture ) shader_texture( va("%s_tex", name), *c.texture );
}

// -----------------------------------------------------------------------------
// colors

unsigned rgba( uint8_t r, uint8_t g, uint8_t b, uint8_t a ) {
    return (unsigned)a << 24 | b << 16 | g << 8 | r;
}
unsigned bgra( uint8_t b, uint8_t g, uint8_t r, uint8_t a ) {
    return rgba(r,g,b,a);
}
unsigned alpha( unsigned rgba ) {
    return rgba >> 24;
}

unsigned rgbaf(float r, float g, float b, float a) {
    return rgba(r * 255, g * 255, b * 255, a * 255);
}
unsigned bgraf(float b, float g, float r, float a) {
    return rgba(r * 255, g * 255, b * 255, a * 255);
}

unsigned atorgba(const char *s) {
    if( s[0] != '#' ) return 0;
    unsigned r = 0, g = 0, b = 0, a = 255;
    int slen = strspn(s+1, "0123456789abcdefABCDEF");
    if( slen > 8 ) slen = 8;
    /**/ if( slen == 6 ) sscanf(s+1, "%2x%2x%2x",    &r,&g,&b);
    else if( slen == 8 ) sscanf(s+1, "%2x%2x%2x%2x", &r,&g,&b,&a);
    else if( slen == 3 ) sscanf(s+1, "%1x%1x%1x",    &r,&g,&b   ), r=r<<4|r,g=g<<4|g,b=b<<4|b;
    else if( slen == 4 ) sscanf(s+1, "%1x%1x%1x%1x", &r,&g,&b,&a), r=r<<4|r,g=g<<4|g,b=b<<4|b,a=a<<4|a;
    return rgba(r,g,b,a);
}
char *rgbatoa(unsigned rgba) {
    unsigned a = rgba >> 24;
    unsigned b =(rgba >> 16) & 255;
    unsigned g =(rgba >>  8) & 255;
    unsigned r = rgba        & 255;
    char *s = va("#        ");
    sprintf(s+1, "%02x%02x%02x%02x", r,g,b,a);
    return s;
}

// -----------------------------------------------------------------------------
// images

image_t image_create(int x, int y, int flags) {
    int n = 3; // defaults to RGB
    if(flags & IMAGE_R) n = 1;
    if(flags & IMAGE_RG) n = 2;
    if(flags & IMAGE_RGB) n = 3;
    if(flags & IMAGE_RGBA) n = 4;
    image_t img; img.x = x; img.y = y; img.n = n;
    img.pixels = REALLOC(0, x * y * n ); // @fixme: image_destroy() requires stbi allocator to match REALLOC
    return img;
}

image_t image_from_mem(const void *data, int size, int flags) {
    image_t img = {0};
    if( data && size ) {
        stbi_set_flip_vertically_on_load(flags & IMAGE_FLIP ? 1 : 0);

        int n = 0;
        if(flags & IMAGE_R) n = 1;
        if(flags & IMAGE_RG) n = 2;
        if(flags & IMAGE_RGB) n = 3;
        if(flags & IMAGE_RGBA) n = 4;
        if(flags & IMAGE_FLOAT)
        img.pixels = stbi_loadf_from_memory((const stbi_uc*)data, size, (int*)&img.x,(int*)&img.y,(int*)&img.n, n);
        else
        img.pixels = stbi_load_from_memory((const stbi_uc*)data, size, (int*)&img.x,(int*)&img.y,(int*)&img.n, n);
        if( img.pixels ) {
            PRINTF("Loaded image (%dx%d %.*s->%.*s)\n",img.w,img.h,img.n,"RGBA",n?n:img.n,"RGBA");
        } else {
            // PANIC("Error loading image (%s)\n", pathfile);
        }
        img.n = n ? n : img.n;
    }
    return img;
}

image_t image(const char *pathfile, int flags) {
    //const char *fname = vfs_remap(pathfile);
    // if( !fname[0] ) fname = vfs_remap(va("%s.png",pathfile)); // needed?
    // if( !fname[0] ) fname = vfs_remap(va("%s.jpg",pathfile)); // needed?
    // if( !fname[0] ) fname = vfs_remap(va("%s.tga",pathfile)); // needed?
    // if( !fname[0] ) fname = vfs_remap(va("%s.jpg.png",pathfile)); // needed?
    // if( !fname[0] ) fname = vfs_remap(va("%s.tga.png",pathfile)); // needed?
    // if( !fname[0] ) fname = vfs_remap(va("%s.png.jpg",pathfile)); // needed?
    // if( !fname[0] ) fname = vfs_remap(va("%s.tga.jpg",pathfile)); // needed?

    int size = 0;
    char *data = vfs_load(pathfile, &size);
    return image_from_mem(data, size, flags);
}

void image_destroy(image_t *img) {
    if(img->pixels) stbi_image_free(img->pixels);
    img->pixels = 0; // *img = (image_t){0}; // do not clear fields yet. might be useful in the future.
}

// bilinear interpolation (uv must be in image coords, range [0..w-1,0..h-1])
static
vec3 bilinear(image_t in, vec2 uv) { // image_bilinear_pixel() ?
    float w = in.x, h = in.y, u = uv.x, v = uv.y;
    float u1 = (int)u, v1 = (int)v, u2 = minf(u1+1, w-1), v2 = minf(v1+1, h-1);
    float c1 = u - u1, c2 = v - v1;
    uint8_t *p1 = &in.pixels8[ in.n * (int)(u1 + v1 * in.w) ];
    uint8_t *p2 = &in.pixels8[ in.n * (int)(u2 + v1 * in.w) ];
    uint8_t *p3 = &in.pixels8[ in.n * (int)(u1 + v2 * in.w) ];
    uint8_t *p4 = &in.pixels8[ in.n * (int)(u2 + v2 * in.w) ];
    vec3 A = vec3( p1[0], p1[1], p1[2] );
    vec3 B = vec3( p2[0], p2[1], p2[2] );
    vec3 C = vec3( p3[0], p3[1], p3[2] );
    vec3 D = vec3( p4[0], p4[1], p4[2] );
    return mix3(mix3(A, B, c1), mix3(C, D, c1), c2);
}

// -----------------------------------------------------------------------------
// textures

static int textureUnit = 0, totalTextureUnits = 0;
int texture_unit() {
    do_once glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &totalTextureUnits);
    // ASSERT(textureUnit < totalTextureUnits, "%d texture units exceeded", totalTextureUnits);
    return textureUnit++ % totalTextureUnits;
}

unsigned texture_update(texture_t *t, unsigned w, unsigned h, unsigned n, const void *pixels, int flags) {
    if( t && !t->id ) {
        glGenTextures( 1, &t->id );
        return texture_update(t, w, h, n, pixels, flags);
    }
    ASSERT( t && t->id );
    ASSERT( n <= 4 );
    GLuint pixel_types[] = { GL_RED, GL_RED, GL_RG, GL_RGB, GL_RGBA, GL_R32F, GL_R32F, GL_RG32F, GL_RGB32F, GL_RGBA32F };
    GLenum pixel_storage = flags & TEXTURE_FLOAT ? GL_FLOAT : GL_UNSIGNED_BYTE;
    GLuint pixel_type = pixel_types[ n ];
    GLuint texel_type = t->texel_type = pixel_types[ n + 5 * !!(flags & TEXTURE_FLOAT) ];
    GLenum wrap = GL_CLAMP_TO_EDGE;
    GLenum min_filter = GL_NEAREST, mag_filter = GL_NEAREST;
//    GLfloat color = (flags&7)/7.f, border_color[4] = { color, color, color, 1.f };

    if( flags & TEXTURE_BGR )  if( pixel_type == GL_RGB )  pixel_type = GL_BGR;
    if( flags & TEXTURE_BGR )  if( pixel_type == GL_RGBA ) pixel_type = GL_BGRA;
    if( flags & TEXTURE_SRGB ) if( texel_type == GL_RGB )  texel_type = GL_SRGB;
    if( flags & TEXTURE_SRGB ) if( texel_type == GL_RGBA ) texel_type = GL_SRGB_ALPHA; // GL_SRGB8_ALPHA8 ?

    if( flags & TEXTURE_BC1 ) texel_type = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    if( flags & TEXTURE_BC2 ) texel_type = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    if( flags & TEXTURE_BC3 ) texel_type = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    if( flags & TEXTURE_DEPTH ) texel_type = pixel_type = GL_DEPTH_COMPONENT; // GL_DEPTH_COMPONENT32

    if( flags & TEXTURE_REPEAT ) wrap = GL_REPEAT;
    if( flags & TEXTURE_BORDER ) wrap = GL_CLAMP_TO_BORDER;
    if( flags & TEXTURE_LINEAR ) min_filter = GL_LINEAR, mag_filter = GL_LINEAR;
    if( flags & TEXTURE_MIPMAPS  ) min_filter = flags & TEXTURE_LINEAR ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR; // : GL_LINEAR_MIPMAP_NEAREST; maybe?
    if( flags & TEXTURE_MIPMAPS  ) mag_filter = flags & TEXTURE_LINEAR ? GL_LINEAR : GL_NEAREST;

#if 0
    if( 0 ) { // flags & TEXTURE_PREMULTIPLY_ALPHA )
        uint8_t *p = pixels;
        if(n == 2) for( unsigned i = 0; i < 2*w*h; i += 2 ) {
            p[i] = (p[i] * p[i+1] + 128) >> 8;
        }
        if(n == 4) for( unsigned i = 0; i < 4*w*h; i += 4 ) {
            p[i+0] = (p[i+0] * p[i+3] + 128) >> 8;
            p[i+1] = (p[i+1] * p[i+3] + 128) >> 8;
            p[i+2] = (p[i+2] * p[i+3] + 128) >> 8;
        }
    }
#endif

GLenum texture_type = t->flags & TEXTURE_ARRAY ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D; // @fixme: test GL_TEXTURE_2D_ARRAY

//glPixelStorei( GL_UNPACK_ALIGNMENT, n < 4 ? 1 : 4 ); // for framebuffer reading
//glActiveTexture(GL_TEXTURE0 + (flags&7));
    glBindTexture(texture_type, t->id);
    glTexImage2D(texture_type, 0, texel_type, w, h, 0, pixel_type, pixel_storage, pixels);
    glTexParameteri(texture_type, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(texture_type, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(texture_type, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(texture_type, GL_TEXTURE_MAG_FILTER, mag_filter);

    if (flags & TEXTURE_ANISOTROPY) {
        GLfloat value, max_anisotropy = 16.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &value);

        value = (value > max_anisotropy) ? max_anisotropy : value;
        glTexParameterf(texture_type, GL_TEXTURE_MAX_ANISOTROPY, value);
    }

#if 0 // only for sampler2DShadow
    if( flags & TEXTURE_DEPTH )   glTexParameteri(texture_type, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    if( flags & TEXTURE_DEPTH )   glTexParameteri(texture_type, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
#endif
//  if( flags & TEXTURE_BORDER )  glTexParameterfv(texture_type, GL_TEXTURE_BORDER_COLOR, border_color);
    if( flags & TEXTURE_MIPMAPS ) glGenerateMipmap(texture_type);

if( flags & TEXTURE_MIPMAPS ) {
        GLfloat max_aniso = 0;
//        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &max_aniso);
max_aniso = 4;
       glTexParameterf(texture_type, GL_TEXTURE_MAX_ANISOTROPY, max_aniso);
}

    // glBindTexture(texture_type, 0); // do not unbind. current code expects texture to be bound at function exit

    t->w = w;
    t->h = h;
    t->n = n;
    t->flags = flags;
    t->filename = t->filename ? t->filename : "";
    t->transparent = 0;

    if (t->n == 4 && pixels) {
        for (int i = 0; i < w * h; i++) {
            if (((uint8_t *)pixels)[i * 4 + 3] < 255) {
                t->transparent = 1;
                break;
            }
        }
    }

    return t->id;
}

texture_t texture_create(unsigned w, unsigned h, unsigned n, const void *pixels, int flags) {
    texture_t texture = {0};
    glGenTextures( 1, &texture.id );
    texture_update( &texture, w, h, n, pixels, flags );
    return texture;
}

texture_t texture_checker() {
    static texture_t texture = {0};
    if( !texture.id ) {
#if 0
        float pixels[] = { 1,0.5,0.5,1 };
        texture = texture_create(2,2,1, pixels, TEXTURE_FLOAT|TEXTURE_MIPMAPS|TEXTURE_REPEAT|TEXTURE_BORDER);
#else
        uint32_t *pixels = REALLOC(0, 256*256*4);
        for (int y = 0, i = 0; y < 256; y++) {
            for (int x = 0; x < 256; x++) {
#if 0
                extern const uint32_t secret_palette[32];
                uint32_t rgb = secret_palette[ y / 8 ] * !!((x ^ y) & 0x8);
                pixels[i++] = (rgb>>16) & 255;
                pixels[i++] = (rgb>>8) & 255;
                pixels[i++] = (rgb>>0) & 255;
                pixels[i++] = 255;
#elif 0
                extern const uint32_t secret_palette[32];
                uint32_t rgb = ((x ^ y) & 0x8) ? secret_palette[6] : secret_palette[ 8 + ((x^y) / (256/6)) ];
                pixels[i++] = (rgb>>16) & 255;
                pixels[i++] = (rgb>>8) & 255;
                pixels[i++] = (rgb>>0) & 255;
                pixels[i++] = 255;
#elif 0
                extern const uint32_t secret_palette[32];
                uint32_t lum = (x^y) & 8 ? 128 : (x^y) & 128 ? 192 : 255;
                uint32_t rgb = rgba(lum,lum,lum,255);
                pixels[i++] = rgb;
#else
                int j = y, i = x;
                unsigned char *p = (unsigned char *)&pixels[x + y * 256];
                p[0] = (i / 16) % 2 == (j / 16) % 2 ? 255 : 0; // r
                p[1] = ((i - j) / 16) % 2 == 0 ? 255 : 0; // g
                p[2] = ((i + j) / 16) % 2 == 0 ? 255 : 0; // b
                p[3] = 255; // a
#endif
            }
        }
        texture = texture_create(256,256,4, pixels, TEXTURE_RGBA|TEXTURE_MIPMAPS|TEXTURE_REPEAT|TEXTURE_BORDER);
        FREE(pixels);
#endif
    }
    return texture;
}

texture_t texture_from_mem(const void *ptr, int len, int flags) {
    image_t img = image_from_mem(ptr, len, flags);
    if( img.pixels ) {
        texture_t t = texture_create(img.x, img.y, img.n, img.pixels, flags);
        image_destroy(&img);
        return t;
    }
    return texture_checker();
}

texture_t texture(const char *pathfile, int flags) {
    // PRINTF("Loading file %s\n", pathfile);
    image_t img = image(pathfile, flags);
    if( img.pixels ) {
        texture_t t = texture_create(img.x, img.y, img.n, img.pixels, flags);
        t.filename = STRDUP(file_name(pathfile));
        image_destroy(&img);
        return t;
    }
    return texture_checker();
}

void texture_destroy( texture_t *t ) {
    if(t->filename && t->filename[0]) FREE(t->filename), t->filename = 0;
    if(t->fbo) fbo_destroy(t->fbo), t->fbo = 0;
    if(t->id) glDeleteTextures(1, &t->id), t->id = 0;
    *t = (texture_t){0};
}

bool texture_rec_begin(texture_t *t, unsigned tw, unsigned th) {
    for( unsigned w = tw ? tw : window_width(), h = th ? th : window_height(); w*h ; ) {
        // resize if needed
        if( t->w != w || t->h != h ) {
            // re-create texture, set texture parameters and content
            texture_update(t, w, h, 4, NULL, TEXTURE_RGBA);
            if(!t->fbo) t->fbo = fbo(t->id, 0, 0);
        }
        // bind fbo to texture
        fbo_bind(t->fbo);
        return true;
    }
    return false;

}
void texture_rec_end(texture_t *t) {
    fbo_unbind();
}

// ktx texture loader
// - rlyeh, public domain
//
// [ref] https://developer.nvidia.com/astc-texture-compression-for-game-assets
//
// # Compatibility and modes. What to choose.
// - iOS: PVRTC1_4_RGB or PVRTC1_4 (RGBA) with q:pvrtcnormal.
// - Desktop (OSX/Linux/Windows): BC1, BC1a or BC3 with q:normal.
// - Android: ETC2_RGB or ETC2_RGBA with q:etcfast. ASTC_4x4 or ASTC_8x8 with q:astcmedium, as a fallback.

#if 0
enum {
    // for glFormat
    GLFORMAT_RED                 = 0x1903,
    GLFORMAT_RG                  = 0x8227,
    GLFORMAT_RGB                 = 0x1907,
    GLFORMAT_RGBA                = 0x1908,
    //GLFORMAT_ALPHA             = 0x1906, // 8
    //GLFORMAT_LUMINANCE         = 0x1909, // 8
    //GLFORMAT_LUMINANCE_ALPHA   = 0x190A, // 88

    // for glType
    GLTYPE_UNSIGNED_BYTE         = 0x1401,

    // for glInternalFormat: RAW           // @todo: SRGB, SRGBA, SBGR, SBGRA
    UNCOMPRESSED_RGB             = 0x8051, // 888, GL_RGB8_EXT
    UNCOMPRESSED_RGB_565         = 0x8363,
    UNCOMPRESSED_RGBA            = 0x8058, // 8888, GL_RGBA8_EXT
    UNCOMPRESSED_RGBA_4444       = 0x8033,
    UNCOMPRESSED_RGBA_5551       = 0x8034,
    UNCOMPRESSED_BGR             = 0x80E0, // 888
    UNCOMPRESSED_BGRA            = 0x80E1, // 8888

    // for glInternalFormat: S3TC/DXTn/BCn // @todo: BC4,5,6,7*
    COMPRESSED_RGB_BC1           = 0x83F0, // DXT1
    COMPRESSED_RGBA_BC1          = 0x83F1, // DXT1a, BC1a
    COMPRESSED_RGBA_BC2          = 0x83F2, // DXT3
    COMPRESSED_RGBA_BC3          = 0x83F3, // DXT5
    COMPRESSED_RGBA_BC7          = 0x8E8C, // COMPRESSED_RGBA_BPTC_UNORM_ARB
    COMPRESSED_SRGB_BC1          = 0x8C4C,
    COMPRESSED_SRGBA_BC1         = 0x8C4D,
    COMPRESSED_SRGBA_BC2         = 0x8C4E,
    COMPRESSED_SRGBA_BC3         = 0x8C4F,
    // RGB_BC7f  COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB
    // RGB_BC7uf COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB
    // RGBA_BC7  COMPRESSED_RGBA_BPTC_UNORM_ARB
    // SRGBA_BC7 COMPRESSED_SRGBA_BPTC_UNORM_ARB

    // for glInternalFormat: ETC2+EAC
    COMPRESSED_R_EAC             = 0x9270, // 4bpp
    COMPRESSED_R_EAC_SIGNED      = 0x9271, // 4bpp. can preserve 0
    COMPRESSED_RG_EAC            = 0x9272, // 8bpp
    COMPRESSED_RG_EAC_SIGNED     = 0x9273, // 8bbp. can preserve 0
    COMPRESSED_RGB_ETC2          = 0x9274, // 4bpp
    COMPRESSED_RGBA_ETC2         = 0x9276, // 4bpp A1
    COMPRESSED_RGBA_ETC2_EAC     = 0x9278, // 8bpp
    COMPRESSED_SRGB_ETC2         = 0x9275, // 4bpp
    COMPRESSED_SRGBA_ETC2        = 0x9277, // 4bpp A1
    COMPRESSED_SRGBA_ETC2_EAC    = 0x9279, // 8bpp

    // for glInternalFormat: PVR
    COMPRESSED_RGB_PVR1_2        = 0x8C01,
    COMPRESSED_RGB_PVR1_4        = 0x8C00,
    COMPRESSED_RGBA_PVR1_2       = 0x8C03,
    COMPRESSED_RGBA_PVR1_4       = 0x8C02,

    COMPRESSED_SRGB_PVR1_2       = 0x8A54, // _EXT
    COMPRESSED_SRGB_PVR1_4       = 0x8A55, // _EXT
    COMPRESSED_SRGBA_PVR1_2      = 0x8A56, // _EXT
    COMPRESSED_SRGBA_PVR1_4      = 0x8A57, // _EXT

    COMPRESSED_RGBA_PVR2_2       = 0x9137,
    COMPRESSED_RGBA_PVR2_4       = 0x9138,
    COMPRESSED_SRGBA_PVR2_2      = 0x93F0,
    COMPRESSED_SRGBA_PVR2_4      = 0x93F1,

    // for glInternalFormat: ASTC
    COMPRESSED_RGBA_ASTC4x4      = 0x93B0, // 8.00bpp
    COMPRESSED_RGBA_ASTC5x4      = 0x93B1, // 6.40bpp
    COMPRESSED_RGBA_ASTC5x5      = 0x93B2, // 5.12bpp
    COMPRESSED_RGBA_ASTC6x5      = 0x93B3, // 4.27bpp
    COMPRESSED_RGBA_ASTC6x6      = 0x93B4, // 3.56bpp
    COMPRESSED_RGBA_ASTC8x5      = 0x93B5, // 3.20bpp
    COMPRESSED_RGBA_ASTC8x6      = 0x93B6, // 2.67bpp
    COMPRESSED_RGBA_ASTC8x8      = 0x93B7, // 2.56bpp
    COMPRESSED_RGBA_ASTC10x5     = 0x93B8, // 2.13bpp
    COMPRESSED_RGBA_ASTC10x6     = 0x93B9, // 2.00bpp
    COMPRESSED_RGBA_ASTC10x8     = 0x93BA, // 1.60bpp
    COMPRESSED_RGBA_ASTC10x10    = 0x93BB, // 1.28bpp
    COMPRESSED_RGBA_ASTC12x10    = 0x93BC, // 1.07bpp
    COMPRESSED_RGBA_ASTC12x12    = 0x93BD, // 0.89bpp

    COMPRESSED_SRGBA_ASTC4x4     = 0x93D0, // 8.00bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC5x4     = 0x93D1, // 6.40bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC5x5     = 0x93D2, // 5.12bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC6x5     = 0x93D3, // 4.27bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC6x6     = 0x93D4, // 3.56bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC8x5     = 0x93D5, // 3.20bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC8x6     = 0x93D6, // 2.67bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC8x8     = 0x93D7, // 2.56bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC10x5    = 0x93D8, // 2.13bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC10x6    = 0x93D9, // 2.00bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC10x8    = 0x93DA, // 1.60bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC10x10   = 0x93DB, // 1.28bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC12x10   = 0x93DC, // 1.07bpp SRGB8 A8
    COMPRESSED_SRGBA_ASTC12x12   = 0x93DD, // 0.89bpp SRGB8 A8

    // others:
    // COMPRESSED_RED_RGTC1
    // COMPRESSED_SIGNED_RED_RGTC1
    // COMPRESSED_RG_RGTC2
    // COMPRESSED_SIGNED_RG_RGTC2
};
#endif

#pragma pack(push, 1) // not really needed. the struct is exactly 64 bytes, and all members are 32-bit unsigned
typedef struct ktx_header {
    unsigned identifier[3];         // "«KTX 11»\r\n\x1A\n"
    unsigned endianness;            // 0x04030201 if match

    unsigned glType;                // 0 if compressed; otherwise: UNSIGNED_BYTE, UNSIGNED_SHORT_5_6_5, etc.
    unsigned glTypeSize;            // 1 if compressed; otherwise, size in bytes of glType for endianness conversion. not needed.
    unsigned glFormat;              // STENCIL_INDEX, DEPTH_COMPONENT, DEPTH_STENCIL, RED, GREEN, BLUE, RG, RGB, RGBA, BGR, BGRA, RED_INTEGER, GREEN_INTEGER, BLUE_INTEGER, RG_INTEGER, RGB_INTEGER, RGBA_INTEGER, BGR_INTEGER, BGRA_INTEGER,
    unsigned glInternalFormat;      // COMPRESSED_RED, COMPRESSED_RG, COMPRESSED_RGB, COMPRESSED_RGBA, COMPRESSED_SRGB, COMPRESSED_SRGB_ALPHA, COMPRESSED_RED_RGTC1, COMPRESSED_SIGNED_RED_RGTC1, COMPRESSED_RG_RGTC2, COMPRESSED_SIGNED_RG_RGTC2, COMPRESSED_RGBA_BPTC_UNORM, COMPRESSED_SRGB_ALPHA_BPTC_UNORM, COMPRESSED_RGB_BPTC_SIGNED_FLOAT, COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT, COMPRESSED_RGB8_ETC2, COMPRESSED_SRGB8_ETC2, COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, COMPRESSED_RGBA8_ETC2_EAC, COMPRESSED_SRGB8_ALPHA8_ETC2_EAC, COMPRESSED_R11_EAC, COMPRESSED_SIGNED_R11_EAC, COMPRESSED_RG11_EAC, COMPRESSED_SIGNED_RG11_EAC,
    unsigned glBaseInternalFormat;  // DEPTH_COMPONENT, DEPTH_STENCIL, RED, RG, RGB, RGBA, STENCIL_INDEX,

    unsigned width;
    unsigned height;
    unsigned depth;

    unsigned num_surfaces; // >1 for material
    unsigned num_faces;    // =6 for cubemaps (+X,-X,+Y,-Y,+Z,-Z order), 1 otherwise
    unsigned num_mipmaps;  // >1 for mipmaps

    unsigned metadata_size; // length of following header
    // struct ktx_metadata {
    //     unsigned key_and_value_size;
    //     char     key_and_value[key_and_value_size];
    //     char     value_padding[3 - ((key_and_value_size + 3) % 4)];
    // };
    // struct ktx_texture_data {
    //     unsigned size;
    //     char     data[0];
    // } tx;
} ktx_header;
#pragma pack(pop)

typedef struct ktx_texture {
    unsigned width;
    unsigned height;
    unsigned depth;
    unsigned size;
    const char* data;
} ktx_texture;

typedef struct ktx {
    ktx_header hdr;
    const char *error;
} ktx;

static __thread array(ktx_texture) ktx_textures;

static
ktx ktx_load(const void *data, unsigned int len) {
    ktx ctx = {0};

    // check ktx signature
    bool is_ktx = (len > sizeof(ktx_header)) && !memcmp(data, "\xABKTX 11\xBB\r\n\x1A\n", 12);
    if( !is_ktx ) {
        return ctx.error = "ERROR_BAD_KTX_FILE", ctx;
    }

    // copy texture header
    ktx_header *hdr = &ctx.hdr;
    *hdr = *((const ktx_header *)data);

    // sanity checks
    STATIC_ASSERT(sizeof(ktx_header) == (16*4));
    for( int i = 0; i < sizeof(ktx_header)/4; ++i) {
        i[(unsigned*)hdr] = lil32(i[(unsigned*)hdr]);
    }
    if( hdr->endianness != 0x04030201 ) {
        return ctx.error = "ERROR_BAD_ENDIANNESS", ctx;
    }
    if( (hdr->num_faces != 1) && (hdr->num_faces != 6) ) {
        return ctx.error = "ERROR_BAD_NUMBER_OF_FACES", ctx;
    }

    // normalize glInternalFormat if uncompressed.
    if( hdr->glType != 0 ) {
        hdr->glInternalFormat = hdr->glBaseInternalFormat;
    }

    // normalize [1..N] range
    hdr->num_mipmaps += !hdr->num_mipmaps;
    hdr->num_surfaces += !hdr->num_surfaces;
    hdr->num_faces += !hdr->num_faces;

    // basically,
    // for each level in num_mipmaps { UInt32 imageSize;
    //     for each surface in num_surfaces {
    //         for each face in num_faces {
    //             for each slice in depth {
    //                 for each row in height {
    //                     for each pixel in width {
    //                         byte data[size_based_on_pixelformat]
    //         byte facePadding[0-3] }}}
    //     }
    // Byte mipPadding[0-3] }

    array_resize(ktx_textures, hdr->num_mipmaps * hdr->num_surfaces * hdr->num_faces);

    const char *bitmap = ((const char*)data) + sizeof(ktx_header) + hdr->metadata_size;

    for( unsigned m = 0; m < hdr->num_mipmaps; ++m ) {
        for( unsigned s = 0; s < hdr->num_surfaces; ++s ) {
            for( unsigned f = 0; f < hdr->num_faces; ++f ) {
                ktx_texture *t = &ktx_textures[f+s*hdr->num_faces+m*hdr->num_faces*hdr->num_surfaces];

                // set dimensions [1..N]
                t->width  = (hdr->width  >> m) + !(hdr->width  >> m);
                t->height = (hdr->height >> m) + !(hdr->height >> m);
                t->depth  = (hdr->depth  >> m) + !(hdr->depth  >> m);

                // seek to mip
                const char *ptr = bitmap;
                for( int i = 0; i <= m; i++ ) {
                    // if cubemap, *ptr holds unpadded size of single face,
                    // else, *ptr holds size of all surfaces+faces+slices for whole mipmap.
                    unsigned size = lil32(*(unsigned*)ptr);
                    unsigned padding = 3 - ((size + 3) % 4);

                    // seek to data
                    t->data = ptr + 4 + (size * f);

                    // seek to next mipmap
                    ptr = ptr + 4 + (size * hdr->num_faces) + padding;

                    // adjust size
                    t->size = (uintptr_t)(ptr - t->data); // -padding; needed?
                }

                // ensure we're in bounds
                ASSERT(t->data < ((char*)data + len), "%p < %p", t->data, ((char*)data + len));
                ASSERT(((char*)t->data+t->size) <= ((char*)data + len), "%p < %p", (char*)t->data + t->size, ((char*)data + len));
            }
        }
    }

    return ctx;
}

// ---

texture_t texture_compressed_from_mem(const void *data, int len, unsigned flags) {
    ktx ctx = ktx_load(data, len);
    if( ctx.error ) {
        // puts(ctx.error);
        // return texture_checker();
        return texture_from_mem(data, len, flags);
    }

    ktx_header hdr = ctx.hdr;

    // flags
    int target = hdr.num_faces == 6 ? GL_TEXTURE_CUBE_MAP : hdr.depth > 0 ? GL_TEXTURE_3D : GL_TEXTURE_2D;
    int dimensions = target == GL_TEXTURE_3D ? 3 : target == GL_TEXTURE_2D || target == GL_TEXTURE_CUBE_MAP ? 2 : 1;

    // create texture
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(target, id);

    // filtering
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, hdr.num_mipmaps > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

    // wrapping
    if( dimensions > 0 ) glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    if( dimensions > 1 ) glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if( dimensions > 2 ) glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_REPEAT);
    if( flags&TEXTURE_CLAMP && dimensions > 0 ) glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    if( flags&TEXTURE_CLAMP && dimensions > 1 ) glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if( flags&TEXTURE_CLAMP && dimensions > 2 ) glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    if( target == GL_TEXTURE_CUBE_MAP ) target = GL_TEXTURE_CUBE_MAP_POSITIVE_X;

    // GLenum internalFormat = flags & TEXTURE_SRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8; // @fixme

    int bytes = 0;
    enum { border = 0 };
    for( int m = 0; m < hdr.num_mipmaps; ++m ) {
        for( int s = 0; s < hdr.num_surfaces; ++s ) {
            for( int f = 0; f < hdr.num_faces; ++f ) {
                int d3 = target == GL_TEXTURE_3D, compr = hdr.glType == 0, mode = d3+compr*2;
                ktx_texture *t = &ktx_textures[f+s*hdr.num_faces+m*hdr.num_faces*hdr.num_surfaces];
                /**/ if(mode==0) glTexImage2D(target+f,m,hdr.glInternalFormat,t->width,t->height,                   border,hdr.glFormat,hdr.glType,t->data);
                else if(mode==1) glTexImage3D(target  ,m,hdr.glInternalFormat,t->width,t->height,t->depth,          border,hdr.glFormat,hdr.glType,t->data);
                else if(mode==2) glCompressedTexImage2D(target+f,m,hdr.glInternalFormat,t->width,t->height,         border,t->size,t->data);
                else if(mode==3) glCompressedTexImage3D(target  ,m,hdr.glInternalFormat,t->width,t->height,t->depth,border,t->size,t->data);
                bytes += t->size;
            }
        }
    }

//    if( !hdr.num_mipmaps )
//    if( flags & TEXTURE_MIPMAPS ) glGenerateMipmap(target);

    texture_t t = {0};
    t.id = id;
    t.w = ktx_textures[0].width;
    t.h = ktx_textures[0].height;
    t.d = ktx_textures[0].depth;
    // @todo: reconstruct flags

    PRINTF("dims:%dx%dx%d,size:%.2fMiB,mips:%d,layers:%d,faces:%d\n", t.w, t.h, t.d, bytes / 1024.0 / 1024.0, hdr.num_mipmaps, hdr.num_surfaces, hdr.num_faces);
    return t;
}

texture_t texture_compressed(const char *pathfile, unsigned flags) {
    //const char *fname = vfs_remap(pathfile);

    int size = 0;
    char *data = vfs_load(pathfile, &size);
    return texture_compressed_from_mem(data, size, flags);
}


// -----------------------------------------------------------------------------

light_t light() {
    light_t l = {0};
    l.diffuse = vec3(1,1,1);
    l.dir = vec3(1,-1,-1);
    l.falloff.constant = 1.0f;
    l.falloff.linear = 0.09f;
    l.falloff.quadratic = 0.0032f;
    l.specularPower = 32.f;
    l.innerCone = 0.85f;// 31 deg
    l.outerCone = 0.9f; // 25 deg
    l.cast_shadows = true;
    l.processed_shadows = false;
    l.shadow_distance = 400.0f;
    l.shadow_near_clip = 0.01f;
    l.shadow_bias = 0.003f;
    l.normal_bias = 0.0025f;
    l.shadow_softness = 7.0f;
    l.penumbra_size = 2.0f;
    l.min_variance = 0.00002f;
    l.variance_transition = 0.2f;
    return l;
}

void light_type(light_t* l, char type) {
    l->cached = 0;
    l->type = type;
}

void light_diffuse(light_t* l, vec3 color) {
    l->cached = 0;
    l->diffuse = color;
}

void light_specular(light_t* l, vec3 color) {
    l->cached = 0;
    l->specular = color;
}

void light_ambient(light_t* l, vec3 color) {
    l->cached = 0;
    l->ambient = color;
}

void light_teleport(light_t* l, vec3 pos) {
    l->cached = 0;
    l->pos = pos;
}

void light_pos(light_t* l, vec3 pos) {
    l->cached = 0;
    l->pos = pos;
}

void light_dir(light_t* l, vec3 dir) {
    l->cached = 0;
    l->dir = dir;
}

void light_power(light_t* l, float power) {
    l->cached = 0;
    l->specularPower = power;
}

void light_falloff(light_t* l, float constant, float linear, float quadratic) {
    l->cached = 0;
    l->falloff.constant = constant;
    l->falloff.linear = linear;
    l->falloff.quadratic = quadratic;
}

void light_radius(light_t* l, float radius) {
    l->cached = 0;
    l->radius = radius;
}

void light_cone(light_t* l, float innerCone, float outerCone) {
    l->cached = 0;
    l->innerCone = acos(innerCone);
    l->outerCone = acos(outerCone);
}

array(light_t) lightlist(const char *pathfile) {
    light_t *lightlist = 0;
    char *light_file = vfs_read(strendi(pathfile,".txt") ? pathfile : va("%s@lightlist.txt", pathfile));
    if( light_file ) {
        // deserialize light
        for each_substring(light_file, "\r\n", light_line) {
            light_t l = light();
            int type;
            vec3 position, direction, color;
            float power;
            char name[128] = {0};

            if (sscanf(light_line, "light: %d %f %f %f %f %f %f %f %f %f %f %127[^\n]",
                       &type, &position.x, &position.y, &position.z,
                       &direction.x, &direction.y, &direction.z,
                       &color.x, &color.y, &color.z,
                       &power, name) != 12) continue;

            l.type = type;
            l.pos = position;
            l.dir = direction;
            // l.diffuse = scale3(color, power);
            l.diffuse = color;
            l.radius = power;
            float falloff = power*0.005f;
            // recalculate attenuation based on radius
            l.falloff.constant = 1.0f/power;
            l.falloff.linear = 0.0f;
            l.falloff.quadratic = 1.0f/falloff/power;
            l.name = STRDUP(name); //@leak

            array_push(lightlist, l);
        }
    }
    return lightlist;
}

static inline
char *light_fieldname(const char *fmt, ...) {
    static char buf[32];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return buf;
}

typedef struct light_object_t {
    vec4 diffuse;
    vec4 specular;
    vec4 ambient;
    vec4 pos;
    vec4 dir;
    float power;
    float radius;
    float innerCone;
    float outerCone;
    float constant;
    float linear;
    float quadratic;
    float shadow_bias;
    float normal_bias;
    float min_variance;
    float variance_transition;
    float shadow_softness;
    float penumbra_size;
    int type;
    int processed_shadows;
    int _padding;
} light_object_t;

static inline
void light_update(unsigned* ubo, unsigned num_lights, light_t *lv) {
    if (num_lights > MAX_LIGHTS) {
        num_lights = MAX_LIGHTS;
    }

    light_object_t lights[MAX_LIGHTS] = {0};
    for (unsigned i = 0; i < num_lights; i++) {
        light_object_t *light = &lights[i];
        {
            light->type = lv[i].type;
            light->diffuse = vec34(lv[i].diffuse, 0.0f);
            light->specular = vec34(lv[i].specular, 0.0f);
            light->ambient = vec34(lv[i].ambient, 0.0f);
            light->pos = vec34(lv[i].pos, 0.0f);
            light->dir = vec34(lv[i].dir, 0.0f);
            light->power = lv[i].specularPower;
            light->radius = lv[i].radius;
            light->innerCone = lv[i].innerCone;
            light->outerCone = lv[i].outerCone;
            light->processed_shadows = lv[i].processed_shadows;
            light->constant = lv[i].falloff.constant;
            light->linear = lv[i].falloff.linear;
            light->quadratic = lv[i].falloff.quadratic;
            light->shadow_bias = lv[i].shadow_bias;
            light->normal_bias = lv[i].normal_bias;
            light->min_variance = lv[i].min_variance;
            light->variance_transition = lv[i].variance_transition;
            light->shadow_softness = lv[i].shadow_softness;
            light->penumbra_size = lv[i].penumbra_size;
        }
    }

    shader_int("u_num_lights", num_lights);
    ASSERT(ubo);

    if (*ubo == 0 /* buffer not created */) {
        *ubo = ubo_create(&lights[0], sizeof(light_object_t) * MAX_LIGHTS, STREAM_DRAW);
    } else {
        ubo_update(*ubo, 0, &lights[0], sizeof(light_object_t) * num_lights);
    }

    ubo_bind(*ubo, 0);

    for (unsigned i=0; i < num_lights; ++i) {
        lv[i].cached = 1;
        bool processed = false;
        if (lv[i].processed_shadows && lv[i].shadow_technique == SHADOW_CSM) {
            processed = true;
            for (int j = 0; j < NUM_SHADOW_CASCADES; j++) {
                shader_mat44(light_fieldname("light_shadow_matrix_csm[%d]", j), lv[i].shadow_matrix[j]);
            }
        }
        if (processed) break;
    }
}

void ui_light(light_t *l) {
    ui_int("Type", &l->type);
    ui_float3("Position", &l->pos.x);
    ui_float3("Direction", &l->dir.x);
    ui_color3f("Diffuse", &l->diffuse.x);
    ui_color3f("Specular", &l->specular.x);
    ui_color3f("Ambient", &l->ambient.x);
    ui_float("Specular Power", &l->specularPower);
    ui_clampf("Radius", &l->radius, 0.0f, FLT_MAX);
    ui_clampf_("Constant Falloff", &l->falloff.constant, 0.0, FLT_MAX, 0.005);
    ui_clampf_("Linear Falloff", &l->falloff.linear, 0.0, FLT_MAX, 0.005);
    ui_clampf_("Quadratic Falloff", &l->falloff.quadratic, 0.0, FLT_MAX, 0.005);
    ui_float("Inner Cone", &l->innerCone);
    ui_float("Outer Cone", &l->outerCone);
    ui_float_("Shadow Bias", &l->shadow_bias, 0.00005);
    ui_float_("Normal Bias", &l->normal_bias, 0.00005);
    ui_float_("Shadow Softness", &l->shadow_softness, 0.5);
    ui_float_("Penumbra Size", &l->penumbra_size, 0.05f);
    ui_float_("Min Variance", &l->min_variance, 0.00005);
    ui_float_("Variance Transition", &l->variance_transition, 0.0005);
}

void ui_lights(unsigned num_lights, light_t *lights) {
    for (unsigned i = 0; i < num_lights; ++i) {
        if (ui_collapse(va("Light %d", i), va("light_%d", i))) {
            ui_light(&lights[i]);
            ui_collapse_end();
        }
    }
}

// -----------------------------------------------------------------------------
// shadowmaps

static inline
void shadowmap_init_common_resources(shadowmap_t *s, int vsm_texture_width, int csm_texture_width) {
    // Create a cubemap depth texture for Variance Shadow Mapping (VSM)
    glGenTextures(1, &s->depth_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, s->depth_texture);
    for (int i = 0; i < 6; i++) {
        // Create a 16-bit depth component texture for each face of the cubemap
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT16, vsm_texture_width, vsm_texture_width, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, 0);
    }

    // Unbind the cubemap texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    // Create a 2D depth texture for Cascaded Shadow Mapping (CSM)
    glGenTextures(1, &s->depth_texture_2d);
    glBindTexture(GL_TEXTURE_2D, s->depth_texture_2d);
    // Create a single 16-bit depth component texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, csm_texture_width, csm_texture_width, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, 0);

    // Unbind the 2D texture
    glBindTexture(GL_TEXTURE_2D, 0);
}


#if !is(ems)
static inline void
shadowmap_init_caster_vsm(shadowmap_t *s, int light_index, int texture_width) {
    float borderColor[] = {1.0, 1.0, 1.0, 1.0};

    if (s->maps[light_index].texture) {
        return;
    }

    // Create a cubemap moments texture
    glGenTextures(1, &s->maps[light_index].texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, s->maps[light_index].texture);
    for (int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RG32F, texture_width, texture_width, 0, GL_RG, GL_FLOAT, 0);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

static inline void
shadowmap_init_caster_csm(shadowmap_t *s, int light_index, int texture_width) {
    float borderColor[] = {1.0, 1.0, 1.0, 1.0};

    if (s->maps[light_index].texture_2d[0]) {
        return;
    }

    // Initialise shadow map 2D
    for (int i = 0; i < NUM_SHADOW_CASCADES; i++) {
        glGenTextures(1, &s->maps[light_index].texture_2d[i]);
        glBindTexture(GL_TEXTURE_2D, s->maps[light_index].texture_2d[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, texture_width, texture_width, 0, GL_RED, GL_HALF_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

shadowmap_t shadowmap(int vsm_texture_width, int csm_texture_width) { // = 512, 4096
    shadowmap_t s = {0};
    s.vsm_texture_width = vsm_texture_width;
    s.csm_texture_width = csm_texture_width;
    s.saved_fb = 0;
    s.filter_size = 6;
    s.window_size = 10;

    s.cascade_splits[0] = 0.1f;
    s.cascade_splits[1] = 0.25f;
    s.cascade_splits[2] = 0.75f;
    s.cascade_splits[3] = 1.0f;  /* sticks to camera far plane */

    glGenFramebuffers(1, &s.fbo);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &s.saved_fb);

    for (int i = 0; i < MAX_SHADOW_LIGHTS; i++) {
        s.maps[i].shadow_technique = 0xFFFF;
        for (int j = 0; j < NUM_SHADOW_CASCADES; j++) {
            s.maps[i].cascade_distances[j] = 0.0f;
        }
    }

    shadowmap_init_common_resources(&s, vsm_texture_width, csm_texture_width);

    glBindFramebuffer(GL_FRAMEBUFFER, s.saved_fb);
    return s;
}

static inline
float shadowmap_offsets_build_jitter() {
    return (randf() - 0.5f);    
}

static inline
float *shadowmap_offsets_build_data(int filter_size, int window_size) {
    int bufsize = filter_size * filter_size * window_size * window_size * 2;
    float *data = MALLOC(bufsize * sizeof(float));

    int index = 0;

    for (int y = 0; y < window_size; y++) {
        for (int x = 0; x < window_size; x++) {
            for (int v = filter_size-1; v >= 0; v--) {
                for (int u = 0; u < filter_size; u++) {
                    float x = ((float)(u + 0.5f + shadowmap_offsets_build_jitter()) / (float)filter_size);
                    float y = ((float)(v + 0.5f + shadowmap_offsets_build_jitter()) / (float)filter_size);
                    ASSERT(index + 1 < bufsize);
                    data[index+0] = sqrtf(y) * cosf(2 * M_PI * x);
                    data[index+1] = sqrtf(y) * sinf(2 * M_PI * x);
                    index += 2;
                }
            }
        }
    }
    return data;
}

void shadowmap_offsets_build(shadowmap_t *s, int filter_size, int window_size) {
    if (s->offsets_texture) {
        glDeleteTextures(1, &s->offsets_texture);
        s->offsets_texture = 0;
    }

    s->filter_size = filter_size;
    s->window_size = window_size;
    int num_samples = filter_size * filter_size;

    float *data = shadowmap_offsets_build_data(filter_size, window_size);

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &s->offsets_texture);
    glBindTexture(GL_TEXTURE_3D, s->offsets_texture);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA32F, num_samples / 2, window_size, window_size);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, num_samples / 2, window_size, window_size, GL_RGBA, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_3D, 0);

    FREE(data);
}

static inline
void shadowmap_destroy_light(shadowmap_t *s, int light_index) {
    s->maps[light_index].gen = 0;
    s->maps[light_index].shadow_technique = 0xFFFF;

    if (s->maps[light_index].texture) {
        glDeleteTextures(1, &s->maps[light_index].texture);
        s->maps[light_index].texture = 0;
    }
    
    for (int i = 0; i < NUM_SHADOW_CASCADES; i++) {
        if (s->maps[light_index].texture_2d[i]) {
            glDeleteTextures(1, &s->maps[light_index].texture_2d[i]);
            s->maps[light_index].texture_2d[i] = 0;
        }
    }
}

void shadowmap_destroy(shadowmap_t *s) {
    for (int i = 0; i < MAX_SHADOW_LIGHTS; i++) {
        shadowmap_destroy_light(s, i);
    }

    if (s->depth_texture) {
        glDeleteTextures(1, &s->depth_texture);
        s->depth_texture = 0;
    }

    if (s->depth_texture_2d) {
        glDeleteTextures(1, &s->depth_texture_2d);
        s->depth_texture_2d = 0;
    }

    shadowmap_t z = {0};
    *s = z;
}

static shadowmap_t *active_shadowmap = NULL;

void shadowmap_begin(shadowmap_t *s) {
    glGetIntegerv(GL_VIEWPORT, s->saved_vp);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &s->saved_fb);

    for (int i = 0; i < MAX_SHADOW_LIGHTS; i++) {
        if (s->maps[i].gen != s->gen) {
            shadowmap_destroy_light(s, i);
        }
    }

    if (s->filter_size != s->old_filter_size || s->window_size != s->old_window_size) {
        shadowmap_offsets_build(s, s->filter_size, s->window_size);
        s->old_filter_size = s->filter_size;
        s->old_window_size = s->window_size;
    }

    s->step = 0;
    s->light_step = 0;
    s->cascade_index = 0;
    s->gen++;
    active_shadowmap = s;
    s->saved_pass = model_getpass();
}

static void shadowmap_light_point(shadowmap_t *s, light_t *l, int dir) {
    if(dir<0) return;
    mat44 P, V, PV;
    perspective44(P, 90.0f, 1.0f, l->shadow_near_clip, l->shadow_distance);
    vec3 lightPos = l->pos;

    /**/ if(dir == 0) lookat44(V, lightPos, add3(lightPos, vec3(+1,  0,  0)), vec3(0, -1,  0)); // +X
    else if(dir == 1) lookat44(V, lightPos, add3(lightPos, vec3(-1,  0,  0)), vec3(0, -1,  0)); // -X
    else if(dir == 2) lookat44(V, lightPos, add3(lightPos, vec3( 0, +1,  0)), vec3(0,  0, +1)); // +Y
    else if(dir == 3) lookat44(V, lightPos, add3(lightPos, vec3( 0, -1,  0)), vec3(0,  0, -1)); // -Y
    else if(dir == 4) lookat44(V, lightPos, add3(lightPos, vec3( 0,  0, +1)), vec3(0, -1,  0)); // +Z
    else /*dir == 5*/ lookat44(V, lightPos, add3(lightPos, vec3( 0,  0, -1)), vec3(0, -1,  0)); // -Z
    multiply44x2(PV, P, V); // -Z

    copy44(s->V, V);
    copy44(s->PV, PV);
    
    l->processed_shadows = true;
    s->shadow_technique = l->shadow_technique = SHADOW_VSM;
    model_setpass(RENDER_PASS_SHADOW_VSM);
}

static array(vec3) frustum_corners = 0;

static inline
void shadowmap_light_directional_calc_frustum_corners(mat44 cam_proj, mat44 cam_view) {
    mat44 PV; multiply44x2(PV, cam_proj, cam_view);
    mat44 inverse_view_proj; invert44(inverse_view_proj, PV);
    array_resize(frustum_corners, 0);
    for (unsigned x = 0; x < 2; x++) {
        for (unsigned y = 0; y < 2; y++) {
            for (unsigned z = 0; z < 2; z++) {
                vec4 corner = {
                    x * 2.0f - 1.0f,
                    y * 2.0f - 1.0f,
                    z * 2.0f - 1.0f,
                    1.0f
                };
                vec4 world_corner = transform444(inverse_view_proj, corner);
                world_corner = scale4(world_corner, 1.0f / world_corner.w);
                array_push(frustum_corners, vec3(world_corner.x, world_corner.y, world_corner.z));
            }
        }
    }
}

static void shadowmap_light_directional(shadowmap_t *s, light_t *l, int dir, float cam_fov, float cam_far, mat44 cam_view) {
    if (dir != 0) {
        s->skip_render = true;
        return;
    }

    float far_plane = 0.0f;
    float near_plane = 0.0f;

    if (s->cascade_index == 0) {
        near_plane = l->shadow_near_clip;
        far_plane = l->shadow_distance * s->cascade_splits[0];
    } else if (s->cascade_index < NUM_SHADOW_CASCADES - 1) {
        near_plane = l->shadow_distance * s->cascade_splits[s->cascade_index-1];
        far_plane = l->shadow_distance * s->cascade_splits[s->cascade_index];
    } else {
        near_plane = l->shadow_distance * s->cascade_splits[NUM_SHADOW_CASCADES-1];
        far_plane = cam_far;
    }

    mat44 proj; perspective44(proj, 75, 1.0f, near_plane, far_plane);
    shadowmap_light_directional_calc_frustum_corners(proj, cam_view);
    vec3 center = {0,0,0};
    float sphere_radius = 0.0f;
    for (unsigned i = 0; i < array_count(frustum_corners); i++) {
        center = add3(center, frustum_corners[i]);
        float dist = len3(frustum_corners[i]);
        sphere_radius = max(sphere_radius, dist);
    }
    center = scale3(center, 1.0f / array_count(frustum_corners));


    s->maps[s->light_step].cascade_distances[s->cascade_index] = far_plane;

    float minX = FLT_MAX, maxX = FLT_MIN;
    float minY = FLT_MAX, maxY = FLT_MIN;
    float minZ = FLT_MAX, maxZ = FLT_MIN;

    mat44 V;
    vec3 lightDir = norm3(l->dir);
    vec3 up = vec3(0, 1, 0);

    lookat44(V, sub3(center, scale3(lightDir, sphere_radius)), add3(center, scale3(lightDir, sphere_radius)), up);

    for (unsigned i = 0; i < array_count(frustum_corners); i++) {
        vec3 corner = frustum_corners[i];

        corner = transform344(V, corner);
        minX = min(minX, corner.x);
        maxX = max(maxX, corner.x);
        minY = min(minY, corner.y);
        maxY = max(maxY, corner.y);
        minZ = min(minZ, corner.z);
        maxZ = max(maxZ, corner.z);
    }

#if 0
    float tmpZ = -minZ;
    minZ = -maxZ;
    maxZ = tmpZ;

    float mid = (maxZ + minZ) * 0.5f;
    minZ -= mid * 5.0f;
    maxZ += mid * 5.0f;
#endif

    mat44 P, PV;
    ortho44(P, 
        minX, maxX, 
        minY, maxY, 
        // minZ, maxZ);
        -maxZ, -minZ);

    multiply44x2(PV, P, V);

    copy44(s->V, V);
    copy44(s->PV, PV);
    copy44(l->shadow_matrix[s->cascade_index], PV);

    l->processed_shadows = true;
    l->cached = 0;
    s->shadow_technique = l->shadow_technique = SHADOW_CSM;
    model_setpass(RENDER_PASS_SHADOW_CSM);
}

static inline
bool shadowmap_step_finish(shadowmap_t *s) {
    if (s->shadow_technique == SHADOW_CSM) {
        if (s->cascade_index < NUM_SHADOW_CASCADES-1) {
            s->cascade_index++;
            s->step = 0;
            return false;
        }
    }

    s->step = 0;
    s->light_step++;
    s->cascade_index = 0;
    model_setpass(s->saved_pass);
    return true;
}

bool shadowmap_step(shadowmap_t *s) {
    int max_steps = s->shadow_technique == 0xffff ? 1 : s->shadow_technique == SHADOW_CSM ? 1 : 6;
    if (s->step >= max_steps) {
        return !shadowmap_step_finish(s);
    }

    if (s->light_step >= MAX_SHADOW_LIGHTS) {
        return false;
    }

    s->step++;
    s->skip_render = false;
    s->lights_pushed = 0;
    return true;
}

static inline
void shadowmap_clear_fbo() {
    glClearColor(1, 1, 1, 1);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void shadowmap_light(shadowmap_t *s, light_t *l, mat44 cam_proj, mat44 cam_view) {
    l->processed_shadows = false;
    if (l->cast_shadows) {
        int step = s->step - 1;

        float y_scale = cam_proj[5];
        float cam_fov = (2.0f * atan(1.0f / y_scale)) * TO_DEG;
        float cam_far = 0.0f; {
            float m22 = cam_proj[10];
            float m32 = cam_proj[14];
            float near_plane = -m32 / (m22 + 1.0f);
            cam_far = (2.0f * near_plane) / (m22 - 1.0f);
            cam_far *= 0.5f;
        }

        if (l->type == LIGHT_POINT || l->type == LIGHT_SPOT) {
            shadowmap_light_point(s, l, step);
        } else if (l->type == LIGHT_DIRECTIONAL) {
            shadowmap_light_directional(s, l, step, cam_fov, cam_far, cam_view);
        }

        if (s->skip_render) {
            return;
        }

        if (s->maps[s->light_step].shadow_technique != l->shadow_technique) {
            shadowmap_destroy_light(s, s->light_step);
            if (l->shadow_technique == SHADOW_VSM) {
                shadowmap_init_caster_vsm(s, s->light_step, s->vsm_texture_width);
            } else if (l->shadow_technique == SHADOW_CSM) {
                shadowmap_init_caster_csm(s, s->light_step, s->csm_texture_width);
            }
        }

        s->maps[s->light_step].gen = s->gen;
        s->maps[s->light_step].shadow_technique = l->shadow_technique;

        ASSERT(s->lights_pushed == 0);
        s->lights_pushed++;

        if (l->type == LIGHT_DIRECTIONAL) {
            glBindFramebuffer(GL_FRAMEBUFFER, s->fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s->maps[s->light_step].texture_2d[s->cascade_index], 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, s->depth_texture_2d, 0);
            shadowmap_clear_fbo();
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, s->fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + step, s->maps[s->light_step].texture, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + step, s->depth_texture, 0);
            shadowmap_clear_fbo();
        }
    
        unsigned texture_width = s->shadow_technique == SHADOW_VSM ? s->vsm_texture_width : s->csm_texture_width;
        glViewport(0, 0, texture_width, texture_width);

        s->shadow_frustum = frustum_build(s->PV);
    }
}

void shadowmap_end(shadowmap_t *s) {
    glViewport(s->saved_vp[0], s->saved_vp[1], s->saved_vp[2], s->saved_vp[3]);
    glBindFramebuffer(GL_FRAMEBUFFER, s->saved_fb);
    active_shadowmap = NULL;

    // calculate vram usage
    s->vram_usage = 0;
    s->vram_usage_total = 0;
    s->vram_usage_vsm = 0;
    s->vram_usage_csm = 0;
    {
        // Common resources
        s->vram_usage += 6 * s->vsm_texture_width * s->vsm_texture_width * 2; // VSM depth texture (GL_DEPTH_COMPONENT16)
        s->vram_usage += s->csm_texture_width * s->csm_texture_width * 2; // CSM depth texture (GL_DEPTH_COMPONENT16)

        // Per-light resources
        for (int i = 0; i < MAX_SHADOW_LIGHTS; i++) {
            if (s->maps[i].shadow_technique == SHADOW_VSM) {
                // VSM cubemap texture (GL_RGB32F)
                s->vram_usage_vsm += 6 * s->vsm_texture_width * s->vsm_texture_width * 8;
            } else if (s->maps[i].shadow_technique == SHADOW_CSM) {
                // CSM textures (GL_RG16F)
                s->vram_usage_csm += NUM_SHADOW_CASCADES * s->csm_texture_width * s->csm_texture_width * 2;
            }
        }
        s->vram_usage_total = s->vram_usage + s->vram_usage_vsm + s->vram_usage_csm;
    }
}

void ui_shadowmap(shadowmap_t *s) {
    if (!s) return;

    int vsm_width = s->vsm_texture_width;
    int csm_width = s->csm_texture_width;
    ui_int("Texture Width (VSM)", &vsm_width);
    ui_int("Texture Width (CSM)", &csm_width);

    if (ui_collapse("Shadowmap Offsets", "shadowmap_offsets")) {
        ui_int("Filter Size", &s->filter_size);
        ui_int("Window Size", &s->window_size);
        ui_collapse_end();
    }

    if (ui_collapse("VRAM Usage", "vram_usage")) {
        ui_label2("Total VRAM", va("%lld KB", s->vram_usage_total / 1024));
        ui_label2("VSM VRAM", va("%lld KB", s->vram_usage_vsm / 1024));
        ui_label2("CSM VRAM", va("%lld KB", s->vram_usage_csm / 1024));
        ui_label2("Depth Texture VRAM", va("%lld KB", s->vram_usage / 1024));
        ui_collapse_end();
    }
}
#else // @todo ems support
shadowmap_t shadowmap(int vsm_texture_width, int csm_texture_width) { // = 512, 4096
    shadowmap_t s = {0};
    s.vsm_texture_width = vsm_texture_width;
    s.csm_texture_width = csm_texture_width;
    s.saved_fb = 0;
    s.filter_size = 8;
    s.window_size = 10;
    s.cascade_splits[0] = 0.1f;
    s.cascade_splits[1] = 0.5f;
    s.cascade_splits[2] = 1.0f;
    s.cascade_splits[3] = 1.0f;  /* sticks to camera far plane */
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &s.saved_fb);
    glBindFramebuffer(GL_FRAMEBUFFER, s.saved_fb);
    return s;
}

static inline
void shadowmap_destroy_light(shadowmap_t *s, int light_index) {
    s->maps[light_index].gen = 0;
    s->maps[light_index].shadow_technique = 0xFFFF;
}

void shadowmap_destroy(shadowmap_t *s) {
    for (int i = 0; i < MAX_LIGHTS; i++) {
        shadowmap_destroy_light(s, i);
    }
    shadowmap_t z = {0};
    *s = z;
}

static shadowmap_t *active_shadowmap = NULL;

void shadowmap_begin(shadowmap_t *s) {
    glGetIntegerv(GL_VIEWPORT, s->saved_vp);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &s->saved_fb);

    for (int i = 0; i < MAX_SHADOW_LIGHTS; i++) {
        if (s->maps[i].gen != s->gen) {
            shadowmap_destroy_light(s, i);
        }
    }

    s->step = 0;
    s->light_step = 0;
    s->cascade_index = 0;
    s->gen++;
    active_shadowmap = s;
    s->saved_pass = model_getpass();
}

bool shadowmap_step(shadowmap_t *s) {
    s->skip_render = true;
    return false;
}

void shadowmap_light(shadowmap_t *s, light_t *l, mat44 cam_proj, mat44 cam_view) {
    l->processed_shadows = false;
}

void shadowmap_end(shadowmap_t *s) {
}

void ui_shadowmap(shadowmap_t *s) {
}
#endif
// -----------------------------------------------------------------------------
// Occlusion queries

static renderstate_t query_test_rs;

static inline
unsigned query_adjust_samples_msaa(unsigned samples) {
    if (window_msaa() > 1) {
        return samples / window_msaa();
    }
    return samples;
}

static inline
void query_test_rs_init() {
    do_once {
        query_test_rs = renderstate();
        query_test_rs.depth_test_enabled = true;
        query_test_rs.depth_write_enabled = false;
        query_test_rs.depth_func = GL_LESS;
        query_test_rs.point_size_enabled = 1;
        query_test_rs.point_size = 1.0f;
        memset(query_test_rs.color_mask, 0, sizeof(query_test_rs.color_mask));
    }
}

unsigned query_test_point(mat44 proj, mat44 view, vec3 pos, float size) {
    static int program = -1, vao = -1, u_mvp = -1, query = -1;
    if( program < 0 ) {
        const char* vs = vfs_read("shaders/query_point_vs.glsl");
        const char* fs = vfs_read("shaders/query_point_fs.glsl");

        program = shader(vs, fs, "", "fragcolor" , NULL);
        u_mvp = glGetUniformLocation(program, "u_mvp");
        glGenVertexArrays( 1, (GLuint*)&vao );
        glGenQueries(1, (GLuint*)&query);
        query_test_rs_init();
    }

    query_test_rs.point_size = size;
    renderstate_apply(&query_test_rs);

    int oldprog = last_shader;
    glUseProgram( program );
    
    mat44 M; translation44(M, pos.x, pos.y, pos.z);
    mat44 MVP; multiply44x3(MVP, proj, view, M);
    glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MVP);

    glBindVertexArray( vao );

    glBeginQuery(GL_SAMPLES_PASSED, query);
    glDrawArrays( GL_POINTS, 0, 1 );
    glEndQuery(GL_SAMPLES_PASSED);

    GLuint samples_passed = 0;
    glGetQueryObjectuiv(query, GL_QUERY_RESULT, &samples_passed);

    glBindVertexArray( 0 );
    glUseProgram( oldprog );
    return query_adjust_samples_msaa(samples_passed);
}

// -----------------------------------------------------------------------------
// fullscreen quads

// usage: bind empty vao & commit call for 6 (quad) or 3 vertices (tri).
// ie, glBindVertexArray(empty_vao); glDrawArrays(GL_TRIANGLES, 0, 3);

static renderstate_t fullscreen_quad_rs;

static inline
void fullscreen_quad_rs_init() {
    do_once {
        fullscreen_quad_rs = renderstate();
        fullscreen_quad_rs.depth_test_enabled = false;
        fullscreen_quad_rs.blend_enabled = true;
        fullscreen_quad_rs.blend_src = GL_SRC_ALPHA;
        fullscreen_quad_rs.blend_dst = GL_ONE_MINUS_SRC_ALPHA;
        fullscreen_quad_rs.front_face = GL_CW;
    }
}

void fullscreen_quad_rgb_gamma( texture_t texture, float gamma ) {
    fullscreen_quad_rs_init();
    static int program = -1, vao = -1, u_inv_gamma = -1;
    if( program < 0 ) {
        const char* vs = vfs_read("shaders/vs_0_2_fullscreen_quad_B_flipped.glsl");
        const char* fs = vfs_read("shaders/fs_2_4_texel_inv_gamma.glsl");

        program = shader(vs, fs, "", "fragcolor" , NULL);
        u_inv_gamma = glGetUniformLocation(program, "u_inv_gamma");
        glGenVertexArrays( 1, (GLuint*)&vao );
    }

    GLenum texture_type = texture.flags & TEXTURE_ARRAY ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
    renderstate_apply(&fullscreen_quad_rs);
    glUseProgram( program );
    float inv_gamma = 1.0f / gamma;
    glUniform1f( u_inv_gamma, inv_gamma );

    glBindVertexArray( vao );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( texture_type, texture.id );

    glDrawArrays( GL_TRIANGLES, 0, 6 );
    profile_incstat("Render.num_drawcalls", +1);
    profile_incstat("Render.num_triangles", +2);

    glBindTexture( texture_type, 0 );
    glBindVertexArray( 0 );
    glUseProgram( 0 );
}

void fullscreen_quad_rgb_flipped_gamma( texture_t texture, float gamma ) {
    fullscreen_quad_rs_init();
    static int program = -1, vao = -1, u_inv_gamma = -1;
    if( program < 0 ) {
        const char* vs = vfs_read("shaders/vs_0_2_fullscreen_quad_B.glsl");
        const char* fs = vfs_read("shaders/fs_2_4_texel_inv_gamma.glsl");

        program = shader(vs, fs, "", "fragcolor" , NULL);
        u_inv_gamma = glGetUniformLocation(program, "u_inv_gamma");
        glGenVertexArrays( 1, (GLuint*)&vao );
    }

    GLenum texture_type = texture.flags & TEXTURE_ARRAY ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
    renderstate_apply(&fullscreen_quad_rs);
    glUseProgram( program );
    float inv_gamma = 1.0f / gamma;
    glUniform1f( u_inv_gamma, inv_gamma );

    glBindVertexArray( vao );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( texture_type, texture.id );

    glDrawArrays( GL_TRIANGLES, 0, 6 );
    profile_incstat("Render.num_drawcalls", +1);
    profile_incstat("Render.num_triangles", +2);

    glBindTexture( texture_type, 0 );
    glBindVertexArray( 0 );
    glUseProgram( 0 );
}

void fullscreen_quad_rgb( texture_t texture_rgb ) {
    fullscreen_quad_rgb_gamma(texture_rgb, 1.0f);
}

void fullscreen_quad_rgb_flipped( texture_t texture ) {
    fullscreen_quad_rgb_flipped_gamma(texture, 1.0f);
}

void fullscreen_quad_ycbcr( texture_t textureYCbCr[3] ) {
    fullscreen_quad_rs_init();
    static int program = -1, vao = -1, u_gamma = -1, uy = -1, ucb = -1, ucr = -1;
    if( program < 0 ) {
        const char* vs = vfs_read("shaders/vs_0_2_fullscreen_quad_B_flipped.glsl");
        const char* fs = vfs_read("shaders/fs_2_4_texel_ycbr_gamma_saturation.glsl");

        program = shader(vs, fs, "", "fragcolor" , NULL);
        u_gamma = glGetUniformLocation(program, "u_gamma");

        uy = glGetUniformLocation(program, "u_texture_y");
        ucb = glGetUniformLocation(program, "u_texture_cb");
        ucr = glGetUniformLocation(program, "u_texture_cr");

        glGenVertexArrays( 1, (GLuint*)&vao );
    }

    renderstate_apply(&fullscreen_quad_rs);
    glUseProgram( program );
    // glUniform1f( u_gamma, gamma );

    glBindVertexArray( vao );

    glUniform1i(uy, 0);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, textureYCbCr[0].id );

    glUniform1i(ucb, 1);
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, textureYCbCr[1].id );

    glUniform1i(ucr, 2);
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, textureYCbCr[2].id );

    glDrawArrays( GL_TRIANGLES, 0, 6 );
    profile_incstat("Render.num_drawcalls", +1);
    profile_incstat("Render.num_triangles", +2);

    glBindTexture( GL_TEXTURE_2D, 0 );
    glBindVertexArray( 0 );
    glUseProgram( 0 );
//    glDisable( GL_BLEND );
}

void fullscreen_quad_ycbcr_flipped( texture_t textureYCbCr[3] ) {
    fullscreen_quad_rs_init();
    static int program = -1, vao = -1, u_gamma = -1, uy = -1, ucb = -1, ucr = -1;
    if( program < 0 ) {
        const char* vs = vfs_read("shaders/vs_0_2_fullscreen_quad_B.glsl");
        const char* fs = vfs_read("shaders/fs_2_4_texel_ycbr_gamma_saturation.glsl");

        program = shader(vs, fs, "", "fragcolor" , NULL);
        u_gamma = glGetUniformLocation(program, "u_gamma");

        uy = glGetUniformLocation(program, "u_texture_y");
        ucb = glGetUniformLocation(program, "u_texture_cb");
        ucr = glGetUniformLocation(program, "u_texture_cr");

        glGenVertexArrays( 1, (GLuint*)&vao );
    }

    renderstate_apply(&fullscreen_quad_rs);
    glUseProgram( program );
    // glUniform1f( u_gamma, gamma );

    glBindVertexArray( vao );

    glUniform1i(uy, 0);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, textureYCbCr[0].id );

    glUniform1i(ucb, 1);
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, textureYCbCr[1].id );

    glUniform1i(ucr, 2);
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, textureYCbCr[2].id );

    glDrawArrays( GL_TRIANGLES, 0, 6 );
    profile_incstat("Render.num_drawcalls", +1);
    profile_incstat("Render.num_triangles", +2);

    glBindTexture( GL_TEXTURE_2D, 0 );
    glBindVertexArray( 0 );
    glUseProgram( 0 );
//    glDisable( GL_BLEND );
}

// -----------------------------------------------------------------------------
// 2D quad drawing

void quad_render_id( int texture_type, int texture_id, vec2 dims, vec2 tex_start, vec2 tex_end, int rgba, vec2 start, vec2 end ) {
    static renderstate_t rect_rs;
    do_once {
        rect_rs = renderstate();
        rect_rs.depth_test_enabled = false;
        rect_rs.blend_enabled = true;
        rect_rs.blend_src = GL_SRC_ALPHA;
        rect_rs.blend_dst = GL_ONE_MINUS_SRC_ALPHA;
        rect_rs.front_face = GL_CW;
    }
    static int program = -1, vbo = -1, vao = -1, u_tint = -1, u_has_tex = -1, u_window_width = -1, u_window_height = -1;
    float gamma = 1;
    vec2 dpi = ifdef(osx, window_dpi(), vec2(1,1));
    if( program < 0 ) {
        const char* vs = vfs_read("shaders/rect_2d.vs");
        const char* fs = vfs_read("shaders/rect_2d.fs");

        program = shader(vs, fs, "", "fragcolor" , NULL);
        ASSERT(program > 0);
        u_tint = glGetUniformLocation(program, "u_tint");
        u_has_tex = glGetUniformLocation(program, "u_has_tex");
        u_window_width = glGetUniformLocation(program, "u_window_width");
        u_window_height = glGetUniformLocation(program, "u_window_height");
        glGenVertexArrays( 1, (GLuint*)&vao );
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
    }

    start = mul2(start, dpi);
    end = mul2(end, dpi);

    renderstate_apply(&rect_rs);

    glUseProgram( program );

    glBindVertexArray( vao );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( texture_type, texture_id );

    glUniform1i(u_has_tex, (texture_id != 0));
    glUniform1f(u_window_width, (float)window_width());
    glUniform1f(u_window_height, (float)window_height());

    vec4 rgbaf = {((rgba>>24)&255)/255.f, ((rgba>>16)&255)/255.f,((rgba>>8)&255)/255.f,((rgba>>0)&255)/255.f};
    glUniform4fv(u_tint, GL_TRUE, &rgbaf.x);

    // normalize texture regions
    if (texture_id != 0) {
        tex_start.x /= dims.x;
        tex_start.y /= dims.y;
        tex_end.x /= dims.x;
        tex_end.y /= dims.y;
    }

    GLfloat vertices[] = {
        // Positions      // UVs
        start.x, start.y, tex_start.x, tex_start.y,
        end.x, start.y,   tex_end.x, tex_start.y,
        end.x, end.y,     tex_end.x, tex_end.y,
        start.x, start.y, tex_start.x, tex_start.y,
        end.x, end.y,     tex_end.x, tex_end.y,
        start.x, end.y,   tex_start.x, tex_end.y
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

    glDrawArrays( GL_TRIANGLES, 0, 6 );
    profile_incstat("Render.num_drawcalls", +1);
    profile_incstat("Render.num_triangles", +2);

    glBindTexture( texture_type, 0 );
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindVertexArray( 0 );
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram( 0 );
}

void quad_render( texture_t texture, vec2 tex_start, vec2 tex_end, int rgba, vec2 start, vec2 end ) {
    quad_render_id(texture.flags & TEXTURE_ARRAY ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D, texture.id, vec2(texture.w, texture.h), tex_start, tex_end, rgba, start, end);
}

// -----------------------------------------------------------------------------
// cubemaps

// project cubemap coords into sphere normals
static
vec3 cubemap2polar(int face, int x, int y, int texture_width) {
    float u = (x / (texture_width - 1.f)) * 2 - 1;
    float v = (y / (texture_width - 1.f)) * 2 - 1;
    /**/ if( face == 0 ) return vec3( u, -1, -v);
    else if( face == 1 ) return vec3(-v, -u,  1);
    else if( face == 2 ) return vec3(-1, -u, -v);
    else if( face == 3 ) return vec3(-u,  1, -v);
    else if( face == 4 ) return vec3( v, -u, -1);
    else                 return vec3( 1,  u, -v);
}
// project normal in a sphere as 2d texcoord
static
vec2 polar2uv(vec3 n) {
    n = norm3(n);
    float theta = atan2(n.y, n.x);
    float phi = atan2(n.z, hypot(n.x, n.y));
    float u = (theta + C_PI) / C_PI;
    float v = (C_PI/2 - phi) / C_PI;
    return vec2(u, v);
}

// equirectangular panorama (2:1) to cubemap - in RGB, out RGB
static
void panorama2cubemap_(image_t out[6], const image_t in, int width){
    int face;
    #pragma omp parallel for
    for( face = 0; face < 6; ++face ) {
        out[face] = image_create(width, width, IMAGE_RGB);
        for (int j=0; j < width; ++j) {
            uint32_t *line = &out[ face ].pixels32[ 0 + j * width ];
            for (int i=0; i < width; ++i) {
                vec3 polar = cubemap2polar(face, i, j, width);
                vec2 uv = polar2uv(polar);
                uv = scale2(uv, in.h-1); // source coords (assumes 2:1, 2*h == w)
                vec3 rgb = bilinear(in, uv);
                union color {
                    struct { uint8_t r,g,b,a; };
                    uint32_t rgba;
                } c = { rgb.x, rgb.y, rgb.z, 255 };
                line[i] = c.rgba;
            }
        }
    }
}
// equirectangular panorama (2:1) to cubemap - in RGB, out RGBA
void panorama2cubemap(image_t out[6], const image_t in, int width) {
    int face;
    #pragma omp parallel for
    for( face = 0; face < 6; ++face ) {
        out[face] = image_create(width, width, IMAGE_RGBA);
        for (int j=0; j < width; ++j) {
            uint32_t *line = &out[ face ].pixels32[ 0 + j * width ];
            for (int i=0; i < width; ++i) {
                vec3 polar = cubemap2polar(face, i, j, width);
                vec2 uv = polar2uv(polar);
                uv = scale2(uv, in.h-1); // source coords (assumes 2:1, 2*h == w)
                vec3 rgb = bilinear(in, uv);
                union color {
                    struct { uint8_t r,g,b,a; };
                    uint32_t rgba;
                } c = { rgb.x, rgb.y, rgb.z, 255 };
                line[i] = c.rgba;
            }
        }
    }
}


cubemap_t cubemap6( const image_t images[6], int flags ) {
    cubemap_t c = {0}, z = {0};

    glGenTextures(1, &c.id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, c.id);

    int samples = 0;
    for (int i = 0; i < 6; i++) {
        image_t img = images[i]; //image(textures[i], IMAGE_RGB);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, img.w, img.h, 0, img.n == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);

        // calculate SH coefficients (@ands)
        const vec3 skyDir[] = {{ 1, 0, 0},{-1, 0, 0},{ 0, 1, 0},{ 0,-1, 0},{ 0, 0, 1},{ 0, 0,-1}};
        const vec3 skyX[]   = {{ 0, 0,-1},{ 0, 0, 1},{ 1, 0, 0},{ 1, 0, 0},{ 1, 0, 0},{-1, 0, 0}};
        const vec3 skyY[]   = {{ 0, 1, 0},{ 0, 1, 0},{ 0, 0,-1},{ 0, 0, 1},{ 0, 1, 0},{ 0, 1, 0}};
        int step = 16;
        for (int y = 0; y < img.h; y += step) {
            unsigned char *p = (unsigned char*)img.pixels + y * img.w * img.n;
            for (int x = 0; x < img.w; x += step) {
                vec3 n = add3(
                    add3(
                        scale3(skyX[i],  2.0f * (x / (img.w - 1.0f)) - 1.0f),
                        scale3(skyY[i], -2.0f * (y / (img.h - 1.0f)) + 1.0f)),
                    skyDir[i]); // texelDirection;
                float l = len3(n);
                vec3 light = scale3(vec3(p[0], p[1], p[2]), 1 / (255.0f * l * l * l)); // texelSolidAngle * texel_radiance;
                n = norm3(n);
                c.sh[0] = add3(c.sh[0], scale3(light,  0.282095f));
                c.sh[1] = add3(c.sh[1], scale3(light, -0.488603f * n.y * 2.0 / 3.0));
                c.sh[2] = add3(c.sh[2], scale3(light,  0.488603f * n.z * 2.0 / 3.0));
                c.sh[3] = add3(c.sh[3], scale3(light, -0.488603f * n.x * 2.0 / 3.0));
                c.sh[4] = add3(c.sh[4], scale3(light,  1.092548f * n.x * n.y / 4.0));
                c.sh[5] = add3(c.sh[5], scale3(light, -1.092548f * n.y * n.z / 4.0));
                c.sh[6] = add3(c.sh[6], scale3(light,  0.315392f * (3.0f * n.z * n.z - 1.0f) / 4.0));
                c.sh[7] = add3(c.sh[7], scale3(light, -1.092548f * n.x * n.z / 4.0));
                c.sh[8] = add3(c.sh[8], scale3(light,  0.546274f * (n.x * n.x - n.y * n.y) / 4.0));
                p += img.n * step;
                samples++;
            }
        }
    }


    for (int s = 0; s < 9; s++) {
        c.sh[s] = scale3(c.sh[s], 32.f / samples);
    }

    // if( glGenerateMipmap )
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, /* glGenerateMipmap ?*/ GL_LINEAR_MIPMAP_LINEAR /*: GL_LINEAR*/);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return c;
}

cubemap_t cubemap( const image_t in, int flags ) {
    ASSERT( in.n == 4 );
    image_t out[6];
    panorama2cubemap(out, in, in.h);
    image_t swap[6] = { out[0],out[3],out[1],out[4],out[2],out[5] };
    cubemap_t c = cubemap6(swap, flags);
    int i;
    #pragma omp parallel for
    for( i = 0; i < 6; ++i) image_destroy(&out[i]);
    return c;
}

void cubemap_destroy(cubemap_t *c) {
    glDeleteTextures(1, &c->id);
    c->id = 0; // do not destroy SH coefficients still. they might be useful in the future.

    if (c->pixels) {
        FREE(c->pixels);
        glDeleteFramebuffers(6, c->framebuffers);
        glDeleteTextures(6, c->textures);
        glDeleteRenderbuffers(6, c->depth_buffers);
    }
}

static cubemap_t *last_cubemap;

cubemap_t* cubemap_get_active() {
    return last_cubemap;
}

// cubemap baker

static int sky_last_fb;
static int sky_last_vp[4];
void cubemap_beginbake(cubemap_t *c, vec3 pos, unsigned width, unsigned height) {
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &sky_last_fb);
    glGetIntegerv(GL_VIEWPORT, sky_last_vp);
    c->step = 0;
    c->pos = pos;

    if (!c->pixels || (c->width != width || c->height != height)) {
        c->pixels = REALLOC(c->pixels, width*height*12);
        c->width = width;
        c->height = height;

        if (c->framebuffers[0]) {
            glDeleteFramebuffers(6, c->framebuffers);
            glDeleteTextures(6, c->textures);
            glDeleteRenderbuffers(6, c->depth_buffers);
            for(int i = 0; i < 6; ++i) {
                c->framebuffers[i] = 0;
            }
        }
    }

    if (!c->framebuffers[0]) {
        for(int i = 0; i < 6; ++i) {
            glGenFramebuffers(1, &c->framebuffers[i]);
            glBindFramebuffer(GL_FRAMEBUFFER, c->framebuffers[i]);
            
            glGenTextures(1, &c->textures[i]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, c->textures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, c->textures[i], 0);

            // attach depth buffer
            glGenRenderbuffers(1, &c->depth_buffers[i]);
            glBindRenderbuffer(GL_RENDERBUFFER, c->depth_buffers[i]);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, c->depth_buffers[i]);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }
    }
}

bool cubemap_stepbake(cubemap_t *c, mat44 proj /* out */, mat44 view /* out */) {
    if (c->step >= 6) return false;

    static vec3 directions[6] = {{ 1, 0, 0},{-1, 0, 0},{ 0, 1, 0},{ 0,-1, 0},{ 0, 0, 1},{ 0, 0,-1}};
    static vec3 up_vectors[6] = {{ 0,-1, 0},{ 0,-1, 0},{ 0, 0, 1},{ 0, 0,-1},{ 0,-1, 0},{ 0,-1, 0}};

    glBindFramebuffer(GL_FRAMEBUFFER, c->framebuffers[c->step]);
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, c->width, c->height);

    perspective44(proj, 90.0f, c->width / (float)c->height, 0.1f, 1000.f);
    lookat44(view, c->pos, add3(c->pos, directions[c->step]), up_vectors[c->step]);
    ++c->step;

    return true;
}

void cubemap_endbake(cubemap_t *c, int step, float sky_intensity) {
    if (!sky_intensity) {
        sky_intensity = 1.0f;
    }
    if (!step) {
        step = 16;
    }

    if (c->id) {
        glDeleteTextures(1, &c->id);
        c->id = 0;
    }
  
    #if 0
    static unsigned sh_shader = -1, sh_buffer = -1, wg_buffer = -1, u_intensity = -1, u_size = -1, u_face_index = -1, u_texture = -1, u_step = -1, u_pass = -1;
    do_once {
        sh_shader = compute(vfs_read("shaders/cubemap_sh.glsl"));
        glGenBuffers(1, &sh_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, sh_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, 9 * sizeof(vec3), NULL, GL_DYNAMIC_COPY);
        
        u_texture = glGetUniformLocation(sh_shader, "cubeFace");
        u_intensity = glGetUniformLocation(sh_shader, "skyIntensity");
        u_size = glGetUniformLocation(sh_shader, "textureSize");
        u_face_index = glGetUniformLocation(sh_shader, "faceIndex");
        u_step = glGetUniformLocation(sh_shader, "step");
        u_pass = glGetUniformLocation(sh_shader, "pass");
    }

    // Prepare work group buffer
    glGenBuffers(1, &wg_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, wg_buffer);
    int num_work_groups = ((c->width + 15) / 16) * ((c->height + 15) / 16);
    glBufferData(GL_SHADER_STORAGE_BUFFER, num_work_groups * 9 * sizeof(vec3), NULL, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, wg_buffer);
    
    // Clear SH buffer
    vec3 zero = vec3(0,0,0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sh_buffer);
    glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_RGB32F, GL_RGB, GL_FLOAT, &zero);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sh_buffer);

    // Set up render parameters
    int step = 16;
    shader_bind(sh_shader);
    glUniform1f(u_intensity, sky_intensity);
    glUniform2i(u_size, c->width, c->height);

    for (int i = 0; i < 6; i++) {
        // Bind texture to texture unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, c->textures[i]);
        glUniform1i(u_texture, 0);

        // Set up face index
        glUniform1i(u_face_index, i);

        // Dispatch compute shader
        glUniform1i(u_pass, 0);
        glDispatchCompute((c->width+step-1)/step, (c->height+step-1)/step, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glUniform1i(u_pass, 1);
        glDispatchCompute((c->width+step-1)/step, (c->height+step-1)/step, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    // Copy SH coefficients from buffer to array
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 9 * sizeof(vec3), c->sh);

    // Normalize SH coefficients
    int total_samples = 16 * 2 * 6;
    for (int s = 0; s < 9; s++) {
        c->sh[s] = scale3(c->sh[s], 32.f / total_samples);
        // c->sh[s] = scale3(c->sh[s], 4.f * M_PI / total_samples);
    }

    glDeleteBuffers(1, &wg_buffer);

    // Generate cubemap texture
    glGenTextures(1, &c->id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, c->id);

    // Copy each face of the cubemap to the cubemap texture
    for (int i = 0; i < 6; ++i) {
        glCopyImageSubData(c->textures[i], GL_TEXTURE_2D, 0, 0, 0, 0,
                           c->id, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, 0,
                           c->width, c->height, 1);
    }
    
    // Generate mipmaps
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    #else

    glGenTextures(1, &c->id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, c->id);

    int samples = 0;
    for (int i = 0; i < 6; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, c->framebuffers[i]);
        glReadPixels(0, 0, c->width, c->height, GL_RGB, GL_FLOAT, c->pixels);
  
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, c->width, c->height, 0, GL_RGB, GL_FLOAT, c->pixels);

        // calculate SH coefficients (@ands)
        // copied from cubemap6 method
        const vec3 skyDir[] = {{ 1, 0, 0},{-1, 0, 0},{ 0, 1, 0},{ 0,-1, 0},{ 0, 0, 1},{ 0, 0,-1}};
        const vec3 skyX[]   = {{ 0, 0,-1},{ 0, 0, 1},{ 1, 0, 0},{ 1, 0, 0},{ 1, 0, 0},{-1, 0, 0}};
        // const vec3 skyY[]   = {{ 0, 1, 0},{ 0, 1, 0},{ 0, 0,-1},{ 0, 0, 1},{ 0, 1, 0},{ 0, 1, 0}};
        static vec3 skyY[6] = {{ 0,-1, 0},{ 0,-1, 0},{ 0, 0, 1},{ 0, 0,-1},{ 0,-1, 0},{ 0,-1, 0}};

        for (int y = 0; y < c->height; y += step) {
            float *p = (float*)(c->pixels + y * c->width * 3);
            for (int x = 0; x < c->width; x += step) {
                vec3 n = add3(
                    add3(
                        scale3(skyX[i],  2.0f * (x / (c->width - 1.0f)) - 1.0f),
                        scale3(skyY[i], -2.0f * (y / (c->height - 1.0f)) + 1.0f)),
                    skyDir[i]); // texelDirection;
                float l = len3(n);
                vec3 light = scale3(vec3(p[0], p[1], p[2]), (1 / (l * l * l)) * sky_intensity); // texelSolidAngle * texel_radiance;
                n = norm3(n);
                c->sh[0] = add3(c->sh[0], scale3(light,  0.282095f));
                c->sh[1] = add3(c->sh[1], scale3(light, -0.488603f * n.y * 2.0 / 3.0));
                c->sh[2] = add3(c->sh[2], scale3(light,  0.488603f * n.z * 2.0 / 3.0));
                c->sh[3] = add3(c->sh[3], scale3(light, -0.488603f * n.x * 2.0 / 3.0));
                c->sh[4] = add3(c->sh[4], scale3(light,  1.092548f * n.x * n.y / 4.0));
                c->sh[5] = add3(c->sh[5], scale3(light, -1.092548f * n.y * n.z / 4.0));
                c->sh[6] = add3(c->sh[6], scale3(light,  0.315392f * (3.0f * n.z * n.z - 1.0f) / 4.0));
                c->sh[7] = add3(c->sh[7], scale3(light, -1.092548f * n.x * n.z / 4.0));
                c->sh[8] = add3(c->sh[8], scale3(light,  0.546274f * (n.x * n.x - n.y * n.y) / 4.0));
                p += 3 * step;
                samples++;
            }
        }
    }

    for (int s = 0; s < 9; s++) {
        c->sh[s] = scale3(c->sh[s], 32.f / samples);
    }
    
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    #endif

    glBindFramebuffer(GL_FRAMEBUFFER, sky_last_fb);
    glViewport(sky_last_vp[0], sky_last_vp[1], sky_last_vp[2], sky_last_vp[3]);
}

void cubemap_sh_reset(cubemap_t *c) {
    for (int s = 0; s < 9; s++) {
        c->sh[s] = vec3(0,0,0);
    }
}

void cubemap_sh_addlight(cubemap_t *c, vec3 light, vec3 dir, float strength) {
    // Normalize the direction
    vec3 norm_dir = norm3(dir);

    // Scale the light color and intensity
    vec3 scaled_light = scale3(light, strength);

    // Add light to the SH coefficients
    c->sh[0] = add3(c->sh[0], scale3(scaled_light,  0.282095f));
    c->sh[1] = add3(c->sh[1], scale3(scaled_light, -0.488603f * norm_dir.y));
    c->sh[2] = add3(c->sh[2], scale3(scaled_light,  0.488603f * norm_dir.z));
    c->sh[3] = add3(c->sh[3], scale3(scaled_light, -0.488603f * norm_dir.x));
}

void cubemap_sh_blend(vec3 pos, float max_dist, unsigned count, cubemap_t *probes, vec3 *out_sh) {
    if (count == 0) {
        memset(out_sh, 0, 9 * sizeof(vec3));
        return;
    }

    float total_weight = 0.0f;
    vec3 final_sh[9] = {0};

    // Iterate through each probe
    for (unsigned i = 0; i < count; i++) {
        float distance = len3(sub3(pos, probes[i].pos));
        float weight = 1.0f - (distance / max_dist);
        weight = weight * weight;

        for (int s = 0; s < 9; s++) {
            final_sh[s] = add3(final_sh[s], scale3(probes[i].sh[s], weight));
        }

        total_weight += weight;
    }

    // Normalize the final SH coefficients
    for (int s = 0; s < 9; s++) {
        final_sh[s] = scale3(final_sh[s], 1.0f / total_weight);
    }

    // Apply SH coefficients to the shader
    memcpy(out_sh, final_sh, 9 * sizeof(vec3));
}

// -----------------------------------------------------------------------------
// skyboxes

skybox_t skybox(const char *asset, int flags) {
    skybox_t sky = {0};

    // sky mesh
    vec3 vertices[] = {{+1,-1,+1},{+1,+1,+1},{+1,+1,-1},{-1,+1,-1},{+1,-1,-1},{-1,-1,-1},{-1,-1,+1},{-1,+1,+1}};
    unsigned indices[] = { 0, 1, 2, 3, 4, 5, 6, 3, 7, 1, 6, 0, 4, 2 };
    mesh_update(&sky.geometry, "p3", 0,countof(vertices),vertices, countof(indices),indices, MESH_TRIANGLE_STRIP);

    // sky program
    sky.flags = flags && flags != SKYBOX_PBR ? flags : !!asset ? SKYBOX_CUBEMAP : SKYBOX_RAYLEIGH; // either cubemap or rayleigh
    sky.program = shader(vfs_read("shaders/vs_3_3_skybox.glsl"),
        vfs_read("fs_3_4_skybox.glsl"),
        "att_position", "fragcolor", NULL);
    sky.rayleigh_program = shader(vfs_read("shaders/vs_3_3_skybox.glsl"),
        vfs_read("shaders/fs_3_4_skybox_rayleigh.glsl"),
        "att_position", "fragcolor", NULL);

    // sky cubemap & SH
    if( asset ) {
        int is_panorama = vfs_size( asset );
        if( is_panorama ) { // is file
            stbi_hdr_to_ldr_gamma(2.2f);
            image_t panorama = image( asset, IMAGE_RGBA );
            sky.cubemap = cubemap( panorama, 0 ); // RGBA required
            image_destroy(&panorama);
        } else { // is folder
            image_t images[6] = {0};
            images[0] = image( va("%s/posx", asset), IMAGE_RGB ); // cubepx
            images[1] = image( va("%s/negx", asset), IMAGE_RGB ); // cubenx
            images[2] = image( va("%s/posy", asset), IMAGE_RGB ); // cubepy
            images[3] = image( va("%s/negy", asset), IMAGE_RGB ); // cubeny
            images[4] = image( va("%s/posz", asset), IMAGE_RGB ); // cubepz
            images[5] = image( va("%s/negz", asset), IMAGE_RGB ); // cubenz
            sky.cubemap = cubemap6( images, 0 );
            for( int i = 0; i < countof(images); ++i ) image_destroy(&images[i]);
        }
    } else {
        // set up mie defaults // @fixme: use shader params instead
        shader_bind(sky.rayleigh_program);
        shader_vec3("uSunPos", vec3( 0, 0.1, -1 ));
        shader_vec3("uRayOrigin", vec3(0.0, 6372000.0, 0.0));
        shader_float("uSunIntensity", 22.0);
        shader_float("uPlanetRadius", 6371000.0);
        shader_float("uAtmosphereRadius", 6471000.0);
        shader_vec3("uRayleighScattering", vec3(5.5e-6, 13.0e-6, 22.4e-6));
        shader_float("uMieScattering", 21e-6);
        shader_float("uRayleighScaleHeight", 8000.0);
        shader_float("uMieScaleHeight", 1200.0);
        shader_float("uMiePreferredDirection", 0.758);
        skybox_mie_calc_sh(&sky, 1.2);
    }

    return sky;
}

static inline
texture_t load_env_tex( const char *pathfile, unsigned flags ) {
    stbi_hdr_to_ldr_gamma(2.2f);
    int flags_hdr = strendi(pathfile, ".hdr") ? TEXTURE_FLOAT | TEXTURE_RGBA : 0;
    texture_t t = texture(pathfile, flags | TEXTURE_LINEAR | TEXTURE_MIPMAPS | TEXTURE_REPEAT | flags_hdr);
    glBindTexture( GL_TEXTURE_2D, t.id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    return t;
}

skybox_t skybox_pbr(const char *sky_map, const char *refl_map, const char *env_map) {
    skybox_t sky = {0};

    // sky mesh
    vec3 vertices[] = {{+1,-1,+1},{+1,+1,+1},{+1,+1,-1},{-1,+1,-1},{+1,-1,-1},{-1,-1,-1},{-1,-1,+1},{-1,+1,+1}};
    unsigned indices[] = { 0, 1, 2, 3, 4, 5, 6, 3, 7, 1, 6, 0, 4, 2 };
    mesh_update(&sky.geometry, "p3", 0,countof(vertices),vertices, countof(indices),indices, MESH_TRIANGLE_STRIP);

    // sky program
    sky.flags = SKYBOX_PBR;
    sky.program = shader(vfs_read("shaders/vs_3_3_skybox.glsl"),
        vfs_read("fs_3_4_skybox.glsl"),
        "att_position", "fragcolor", NULL);

    // sky cubemap & SH
    if( sky_map ) {
        int is_panorama = vfs_size( sky_map );
        if( is_panorama ) { // is file
            stbi_hdr_to_ldr_gamma(2.2f);
            image_t panorama = image( sky_map, IMAGE_RGBA );
            sky.cubemap = cubemap( panorama, 0 ); // RGBA required
            image_destroy(&panorama);
        } else { // is folder
            image_t images[6] = {0};
            images[0] = image( va("%s/posx", sky_map), IMAGE_RGB ); // cubepx
            images[1] = image( va("%s/negx", sky_map), IMAGE_RGB ); // cubenx
            images[2] = image( va("%s/posy", sky_map), IMAGE_RGB ); // cubepy
            images[3] = image( va("%s/negy", sky_map), IMAGE_RGB ); // cubeny
            images[4] = image( va("%s/posz", sky_map), IMAGE_RGB ); // cubepz
            images[5] = image( va("%s/negz", sky_map), IMAGE_RGB ); // cubenz
            sky.cubemap = cubemap6( images, 0 );
            for( int i = 0; i < countof(images); ++i ) image_destroy(&images[i]);
        }
    }
    if( refl_map ) {
        sky.refl = load_env_tex(refl_map, 0);
    }
    if( env_map ) {
        sky.env = load_env_tex(env_map, 0);
    }

    return sky;
}

static renderstate_t skybox_rs;
API vec4 window_getcolor_(); // internal use, not public

static inline
void skybox_render_rayleigh(skybox_t *sky, mat44 proj, mat44 view) {
    last_cubemap = &sky->cubemap;

    do_once {
        skybox_rs = renderstate();
        skybox_rs.depth_test_enabled = 1;
        skybox_rs.cull_face_enabled = 0;
        skybox_rs.front_face = GL_CCW;
    }

    // we have to reset clear color here, because of wrong alpha compositing issues on native transparent windows otherwise
    // vec4 bgcolor = window_getcolor_(); 
    // skybox_rs.clear_color[0] = bgcolor.r;
    // skybox_rs.clear_color[1] = bgcolor.g;
    // skybox_rs.clear_color[2] = bgcolor.b;
    // skybox_rs.clear_color[3] = 1; // @transparent

    mat44 mvp; multiply44x2(mvp, proj, view);

    //glDepthMask(GL_FALSE);
    shader_bind(sky->rayleigh_program);
    shader_mat44("u_mvp", mvp);

    renderstate_apply(&skybox_rs);
    mesh_render(&sky->geometry);
}

void skybox_mie_calc_sh(skybox_t *sky, float sky_intensity) {
    cubemap_beginbake(&sky->cubemap, vec3(0, 0, 0), 1024, 1024);
    mat44 proj, view;
    while (cubemap_stepbake(&sky->cubemap, proj, view)) {
        skybox_render_rayleigh(sky, proj, view);
    }
    cubemap_endbake(&sky->cubemap, 0, sky_intensity);
}

void skybox_sh_add_light(skybox_t *sky, vec3 light, vec3 dir, float strength) {
    cubemap_sh_addlight(&sky->cubemap, light, dir, strength);
}

int skybox_push_state(skybox_t *sky, mat44 proj, mat44 view) {
    last_cubemap = &sky->cubemap;

    do_once {
        skybox_rs = renderstate();
        skybox_rs.depth_test_enabled = 1;
        skybox_rs.cull_face_enabled = 0;
        skybox_rs.front_face = GL_CCW;
    }

    // we have to reset clear color here, because of wrong alpha compositing issues on native transparent windows otherwise
    // vec4 bgcolor = window_getcolor_(); 
    // skybox_rs.clear_color[0] = bgcolor.r;
    // skybox_rs.clear_color[1] = bgcolor.g;
    // skybox_rs.clear_color[2] = bgcolor.b;
    // skybox_rs.clear_color[3] = 1; // @transparent



    mat44 mvp;
    multiply44x2(mvp, proj, view);

    //glDepthMask(GL_FALSE);
    shader_bind(sky->program);
    shader_mat44("u_mvp", mvp);
    shader_cubemap("u_cubemap", sky->cubemap.id);

    renderstate_apply(&skybox_rs);
    return 0; // @fixme: return sortable hash here?
}
int skybox_pop_state() {
    //vec4 bgcolor = window_getcolor_(); glClearColor(bgcolor.r, bgcolor.g, bgcolor.b, window_has_transparent() ? 0 : bgcolor.a); // @transparent
    //glDepthMask(GL_TRUE);
    //glClear(GL_DEPTH_BUFFER_BIT);
    return 0;
}
int skybox_render(skybox_t *sky, mat44 proj, mat44 view) {
    if (sky->rayleigh_immediate && !sky->flags) {
        skybox_render_rayleigh(sky, proj, view);
        return 0;
    }
    skybox_push_state(sky, proj, view);
    mesh_render(&sky->geometry);
    skybox_pop_state();
    return 0;
}
void skybox_destroy(skybox_t *sky) {
    glDeleteProgram(sky->program);
    glDeleteProgram(sky->rayleigh_program);
    cubemap_destroy(&sky->cubemap);
    mesh_destroy(&sky->geometry);
}

// -----------------------------------------------------------------------------
// meshes

mesh_t mesh() {
    mesh_t z = {0};
    return z;
}

aabb mesh_bounds(mesh_t *m) {
    aabb b = {{1e9,1e9,1e9},{-1e9,-1e9,-1e9}};
    for( int i = 0; i < array_count(m->in_vertex3); ++i ) {
        if( m->in_vertex3[i].x < b.min.x ) b.min.x = m->in_vertex3[i].x;
        if( m->in_vertex3[i].x > b.max.x ) b.max.x = m->in_vertex3[i].x;

        if( m->in_vertex3[i].y < b.min.y ) b.min.y = m->in_vertex3[i].y;
        if( m->in_vertex3[i].y > b.max.y ) b.max.y = m->in_vertex3[i].y;

        if( m->in_vertex3[i].z < b.min.z ) b.min.z = m->in_vertex3[i].z;
        if( m->in_vertex3[i].z > b.max.z ) b.max.z = m->in_vertex3[i].z;
    }
    return b;
}

void mesh_update(mesh_t *m, const char *format, int vertex_stride,int vertex_count,const void *vertex_data, int index_count,const void *index_data, int flags) {
    m->flags = flags;

    // setup
    unsigned sizeof_index = sizeof(GLuint);
    unsigned sizeof_vertex = 0;
    m->index_count = index_count;
    m->vertex_count = vertex_count;

    // iterate vertex attributes { position, normal + uv + tangent + bitangent + ... }
    struct vertex_descriptor {
        int vertex_type, num_attribute, num_components, alt_normalized;
        int stride, offset;
    } descriptor[16] = {0}, *dc = &descriptor[0];

    do switch( *format ) {
        break; case '*': dc->alt_normalized = 1;
        break; case '0': dc->num_components = 0;
        break; case '1': dc->num_components = 1;
        break; case '2': dc->num_components = 2;
        break; case '3': dc->num_components = 3;
        break; case '4': dc->num_components = 4;
        break; case 'F': dc->vertex_type = GL_FLOAT;
        break; case 'U': case 'I': dc->vertex_type = GL_UNSIGNED_INT;
        break; case 'B': if(format[-1] >= '0' && format[-1] <= '9') dc->vertex_type = GL_UNSIGNED_BYTE; //else bitangent.
        break; case ' ': while (format[1] == ' ') format++; case '\0':
            if (!dc->vertex_type) dc->vertex_type = GL_FLOAT;
            dc->offset = sizeof_vertex;
            sizeof_vertex += (dc->stride = dc->num_components * (dc->vertex_type == GL_UNSIGNED_BYTE ? 1 : 4));
            ++dc;
        break; default: if( !strchr("pntbcwai", *format) ) PANIC("unsupported vertex type '%c'", *format);
    } while (*format++);

    if(vertex_stride > 0) sizeof_vertex = vertex_stride;

    // layout
    if(!m->vao) glGenVertexArrays(1, &m->vao);
    glBindVertexArray(m->vao);

    // index data
    if( index_data && index_count ) {
        m->index_count = index_count;

        if(!m->ibo) glGenBuffers(1, &m->ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->index_count * sizeof_index, index_data, flags & MESH_STREAM ? GL_STREAM_DRAW : GL_STATIC_DRAW);
    }

    // vertex data
    if( vertex_data && vertex_count ) {
        m->vertex_count = vertex_count;

        if(!m->vbo) glGenBuffers(1, &m->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
        glBufferData(GL_ARRAY_BUFFER, m->vertex_count * sizeof_vertex, vertex_data, flags & MESH_STREAM ? GL_STREAM_DRAW : GL_STATIC_DRAW);
    }

    for( int i = 0; i < 8; ++i ) {
//        glDisableVertexAttribArray(i);
    }

    // vertex setup: iterate descriptors
    for( int i = 0; i < countof(descriptor); ++i ) {
        if( descriptor[i].num_components ) {
            glDisableVertexAttribArray(i);
            glVertexAttribPointer(i,
                descriptor[i].num_components, descriptor[i].vertex_type, (descriptor[i].vertex_type == GL_UNSIGNED_BYTE ? GL_TRUE : GL_FALSE) ^ (descriptor[i].alt_normalized ? GL_TRUE : GL_FALSE),
                sizeof_vertex, (GLchar*)NULL + descriptor[i].offset);
            glEnableVertexAttribArray(i);
        } else {
            glDisableVertexAttribArray(i);
        }
    }

    glBindVertexArray(0);
}

void mesh_render(mesh_t *sm) {
    if( sm->vao ) {
        glBindVertexArray(sm->vao);
        if( sm->ibo ) { // with indices
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sm->ibo); // <-- why intel?
            glDrawElements(sm->flags & MESH_TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, sm->index_count, GL_UNSIGNED_INT, (char*)0);
            profile_incstat("Render.num_drawcalls", +1);
            profile_incstat("Render.num_triangles", sm->index_count/3);
        } else { // with vertices only
            glDrawArrays(sm->flags & MESH_TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, 0, sm->vertex_count /* / 3 */);
            profile_incstat("Render.num_drawcalls", +1);
            profile_incstat("Render.num_triangles", sm->vertex_count/3);
        }
    }
}

void mesh_render_prim(mesh_t *sm, unsigned prim) {
    if( sm->vao ) {
        glBindVertexArray(sm->vao);
        if( sm->ibo ) { // with indices
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sm->ibo); // <-- why intel?
            glDrawElements(prim, sm->index_count, GL_UNSIGNED_INT, (char*)0);
            profile_incstat("Render.num_drawcalls", +1);
            profile_incstat("Render.num_triangles", sm->index_count/3);
        } else { // with vertices only
            glDrawArrays(prim, 0, sm->vertex_count /* / 3 */);
            profile_incstat("Render.num_drawcalls", +1);
            profile_incstat("Render.num_triangles", sm->vertex_count/3);
        }
    }
}

void mesh_destroy(mesh_t *m) {
    // @todo
    glDeleteBuffers(1, &m->vbo);
    glDeleteBuffers(1, &m->ibo);
    glDeleteVertexArrays(1, &m->vao);
}

// -----------------------------------------------------------------------------
// screenshots

void* screenshot( int n ) { // 3 RGB, 4 RGBA, -3 BGR, -4 BGRA
    // sync, 10 ms -- pixel perfect

    int w = window_width(), h = window_height();
    int mode = n == 3 ? GL_RGB : n == -3 ? GL_BGR : n == 4 ? GL_RGBA : GL_BGRA;
    static __thread uint8_t *pixels = 0;
    pixels = (uint8_t*)REALLOC(pixels, w * h * 4 ); // @leak per thread

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0); // disable any pbo, in case somebody did for us
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, w, h, mode, GL_UNSIGNED_BYTE, pixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    return pixels;
}

void* screenshot_async( int n ) { // 3 RGB, 4 RGBA, -3 BGR, -4 BGRA
#if is(ems)
    return screenshot(n); // no glMapBuffer() on emscripten
#else
    // async, 0 ms -- @fixme: MSAA can cause some artifacts with PBOs: either use glDisable(GL_MULTISAMPLE) when recording or do not create window with WINDOW_MSAAx options at all.

    int w = window_width(), h = window_height();
    int mode = n == 3 ? GL_RGB : n == -3 ? GL_BGR : n == 4 ? GL_RGBA : GL_BGRA;
    static __thread uint8_t *pixels = 0;
    pixels = (uint8_t*)REALLOC(pixels, w * h * 4 ); // @leak per thread

    enum { NUM_PBOS = 16 };
    static __thread GLuint pbo[NUM_PBOS] = {0}, lastw = 0, lasth = 0, bound = 0;

    if( lastw != w || lasth != h ) {
        lastw = w, lasth = h;
        bound = 0;

        for( int i = 0; i < NUM_PBOS; ++i ) {
        if(!pbo[i]) glGenBuffers(1, &pbo[i]);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[i]);
        glBufferData(GL_PIXEL_PACK_BUFFER, w * h * 4, NULL, GL_STREAM_READ); // GL_STATIC_READ);
        //glReadPixels(0, 0, w, h, mode, GL_UNSIGNED_BYTE, (GLvoid*)((GLchar*)NULL+0));
        }
    }

    // read from oldest bound pbo
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[bound]);
    void *ptr = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    memcpy(pixels, ptr, w * h * abs(n));
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

    // trigger next read
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, w, h, mode, GL_UNSIGNED_BYTE, (GLvoid*)((GLchar*)NULL+0));
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    bound = (bound + 1) % NUM_PBOS;
    return pixels;
#endif
}

// -----------------------------------------------------------------------------
// viewports

void viewport_color(unsigned color) {
    unsigned r = (color >>  0) & 255;
    unsigned g = (color >>  8) & 255;
    unsigned b = (color >> 16) & 255;
    unsigned a = (color >> 24) & 255;
    glClearColor(r, g, b, a);
}

void viewport_clear(bool color, bool depth) {
    glClearDepthf(1);
    glClearStencil(0);
    glClear((color ? GL_COLOR_BUFFER_BIT : 0) | (depth ? GL_DEPTH_BUFFER_BIT : 0));
}

void viewport_clip(vec2 from, vec2 to) {
    float x = from.x, y = from.y, w = to.x-from.x, h = to.y-from.y;

    y = window_height()-y-h;
    glViewport(x, y, w, h);
    glScissor(x, y, w, h);
}

// -----------------------------------------------------------------------------
// fbos

unsigned fbo(unsigned color_texture_id, unsigned depth_texture_id, int flags) {
    int last_fb;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &last_fb);

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    if( color_texture_id ) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture_id, 0);
    if( depth_texture_id ) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture_id, 0);
#if 0 // this is working; it's just not enabled for now
    else {
        // create a non-sampleable renderbuffer object for depth and stencil attachments
        unsigned int rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, color.width, color.height); // use a single renderbuffer object for both a depth AND stencil buffer.
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    }
#endif

#if is(ems)
    GLenum nones[] = { GL_NONE };
    if(flags) glDrawBuffers(1, nones);
    if(flags) glReadBuffer(GL_NONE);
#else
    if(flags) glDrawBuffer(GL_NONE);
    if(flags) glReadBuffer(GL_NONE);
#endif

#if 1
    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if( GL_FRAMEBUFFER_COMPLETE != result ) {
        PANIC("ERROR: Framebuffer not complete.");
    }
#else
    switch (glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
        case GL_FRAMEBUFFER_COMPLETE: break;
        case GL_FRAMEBUFFER_UNDEFINED: PANIC("GL_FRAMEBUFFER_UNDEFINED");
        case GL_FRAMEBUFFER_UNSUPPORTED: PANIC("GL_FRAMEBUFFER_UNSUPPORTED");
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: PANIC("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: PANIC("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: PANIC("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: PANIC("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
//      case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT: PANIC("GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT");
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: PANIC("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS");
//      case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT: PANIC("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT");
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: PANIC("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
        default: PANIC("ERROR: Framebuffer not complete. glCheckFramebufferStatus returned %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }
#endif

    glBindFramebuffer (GL_FRAMEBUFFER, last_fb);
    return fbo;
}
static __thread array(handle) fbos;
void fbo_bind(unsigned id) {
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    array_push(fbos, id);
}
void fbo_unbind() {
    handle id = 0;
    if (array_count(fbos)) {
        array_pop(fbos);
        id = *array_back(fbos);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, id);
}
void fbo_destroy(unsigned id) {
    // glDeleteRenderbuffers(1, &renderbuffer);
    glDeleteFramebuffers(1, &id);
}

// -----------------------------------------------------------------------------
// post-fxs swapchain

typedef struct passfx passfx;
typedef struct postfx postfx;

void postfx_create(postfx *fx, int flags);
void postfx_destroy(postfx *fx);

bool postfx_load(postfx *fx, const char *name, const char *fragment);
bool postfx_begin(postfx *fx, int width, int height);
bool postfx_end(postfx *fx);

bool postfx_enabled(postfx *fx, int pass_number);
bool postfx_enable(postfx *fx, int pass_number, bool enabled);
// bool postfx_toggle(postfx *fx, int pass_number);
void postfx_clear(postfx *fx);
void postfx_order(postfx *fx, int pass, unsigned priority);

char* postfx_name(postfx *fx, int slot);

int   ui_postfx(postfx *fx, int slot);

struct passfx {
    mesh_t m;
    char *name;
    unsigned program;
    int uniforms[16];
    unsigned priority;
    bool enabled;
};

struct postfx {
    // renderbuffers: color & depth textures
    unsigned fb[2];
    texture_t diffuse[2], depth[2];
    // shader passes
    array(passfx) pass;
    // global enable flag
    bool enabled;
};

enum {
    u_color,
    u_depth,
    u_time,
    u_frame,
    u_width, u_height,
    u_mousex, u_mousey,
    u_channelres0x, u_channelres0y,
    u_channelres1x, u_channelres1y,
};

void postfx_create(postfx *fx, int flags) {
    postfx z = {0};
    *fx = z;
    fx->enabled = 1;
    (void)flags;
}

void postfx_destroy( postfx *fx ) {
    for( int i = 0; i < array_count(fx->pass); ++i ) {
        FREE(fx->pass[i].name);
    }
    array_free(fx->pass);
    texture_destroy(&fx->diffuse[0]);
    texture_destroy(&fx->diffuse[1]);
    texture_destroy(&fx->depth[0]);
    texture_destroy(&fx->depth[1]);
    fbo_destroy(fx->fb[0]);
    fbo_destroy(fx->fb[1]);
    postfx z = {0};
    *fx = z;
}

char* postfx_name(postfx *fx, int slot) {
    return slot < 0 || slot >= array_count(fx->pass) ? "" : fx->pass[ slot ].name;
}
int postfx_find(postfx *fx, const char *name) {
    name = file_name(name);
    for( int i = 0; i < array_count(fx->pass); ++i) if(!strcmpi(fx->pass[i].name, name)) return i;
    return -1;
}

static
int postfx_sort_fn(const void *a, const void *b) {
    unsigned p1 = ((passfx*)a)->priority;
    unsigned p2 = ((passfx*)b)->priority;
    return (p1 > p2) - (p1 < p2);
}
void postfx_order(postfx *fx, int pass, unsigned priority) {
    if (pass < 0 || pass >= array_count(fx->pass)) return;
    if (priority >= array_count(fx->pass)) return;
    fx->pass[priority].priority = pass;
    fx->pass[pass].priority = priority;
    array_sort(fx->pass, postfx_sort_fn);
}

int postfx_load_from_mem( postfx *fx, const char *name, const char *fs ) {
    PRINTF("%s\n", name);
    if(!fs || !fs[0]) return -1; // PANIC("!invalid fragment shader");

    passfx pass={0};
    array_push(fx->pass, pass);
    passfx *p = array_back(fx->pass);
    p->name = STRDUP(name);
    p->priority = array_count(fx->pass)-1;

    // preload stuff
    static const char *vs = 0;
    static const char *preamble = 0;
    static const char *shadertoy = 0;
    static char *fs2 = 0;
    do_once {
        vs = STRDUP(vfs_read("shaders/vs_0_2_fullscreen_quad_B.glsl"));
        preamble = STRDUP(vfs_read("shaders/fs_2_4_preamble.glsl"));
        shadertoy = STRDUP(vfs_read("shaders/fs_main_shadertoy.glsl"));
        fs2 = (char*)CALLOC(1, 128*1024);
    }
    // patch fragment
    snprintf(fs2, 128*1024, "%s%s%s", preamble, strstr(fs, "mainImage") ? shadertoy : "", fs );

    p->program = shader(vs, fs2, "vtexcoord", "fragColor" , NULL);

    glUseProgram(p->program); // needed?

    for( int i = 0; i < countof(p->uniforms); ++i ) p->uniforms[i] = -1;

    if( p->uniforms[u_time] == -1 )   p->uniforms[u_time] = glGetUniformLocation(p->program, "iTime");

    if( p->uniforms[u_frame] == -1 )   p->uniforms[u_frame] = glGetUniformLocation(p->program, "iFrame");

    if( p->uniforms[u_width] == -1 )  p->uniforms[u_width] = glGetUniformLocation(p->program, "iWidth");
    if( p->uniforms[u_height] == -1 ) p->uniforms[u_height] = glGetUniformLocation(p->program, "iHeight");

    if( p->uniforms[u_mousex] == -1 ) p->uniforms[u_mousex] = glGetUniformLocation(p->program, "iMousex");
    if( p->uniforms[u_mousey] == -1 ) p->uniforms[u_mousey] = glGetUniformLocation(p->program, "iMousey");

    if( p->uniforms[u_color] == -1 ) p->uniforms[u_color] = glGetUniformLocation(p->program, "tex");
    if( p->uniforms[u_color] == -1 ) p->uniforms[u_color] = glGetUniformLocation(p->program, "tex0");
    if( p->uniforms[u_color] == -1 ) p->uniforms[u_color] = glGetUniformLocation(p->program, "tColor");
    if( p->uniforms[u_color] == -1 ) p->uniforms[u_color] = glGetUniformLocation(p->program, "tDiffuse");
    if( p->uniforms[u_color] == -1 ) p->uniforms[u_color] = glGetUniformLocation(p->program, "iChannel0");

    if( p->uniforms[u_depth] == -1 ) p->uniforms[u_depth] = glGetUniformLocation(p->program, "tex1");
    if( p->uniforms[u_depth] == -1 ) p->uniforms[u_depth] = glGetUniformLocation(p->program, "tDepth");
    if( p->uniforms[u_depth] == -1 ) p->uniforms[u_depth] = glGetUniformLocation(p->program, "iChannel1");

    if( p->uniforms[u_channelres0x] == -1 ) p->uniforms[u_channelres0x] = glGetUniformLocation(p->program, "iChannelRes0x");
    if( p->uniforms[u_channelres0y] == -1 ) p->uniforms[u_channelres0y] = glGetUniformLocation(p->program, "iChannelRes0y");

    if( p->uniforms[u_channelres1x] == -1 ) p->uniforms[u_channelres1x] = glGetUniformLocation(p->program, "iChannelRes1x");
    if( p->uniforms[u_channelres1y] == -1 ) p->uniforms[u_channelres1y] = glGetUniformLocation(p->program, "iChannelRes1y");
    
    // set quad
    glGenVertexArrays(1, &p->m.vao);
    return array_count(fx->pass)-1;
}

bool postfx_enable(postfx *fx, int pass, bool enabled) {
    if( pass < 0 || pass >= array_count(fx->pass) ) return false;
    fx->pass[pass].enabled = enabled;
    fx->enabled = !!array_count(fx->pass);
    return fx->enabled;
}

bool postfx_enabled(postfx *fx, int pass) {
    if( pass < 0 || pass >= array_count(fx->pass) ) return false;
    return fx->pass[pass].enabled;
}

bool postfx_toggle(postfx *fx, int pass) {
    if( pass < 0 || pass >= array_count(fx->pass) ) return false;
    return postfx_enable(fx, pass, 1 ^ postfx_enabled(fx, pass));
}

void postfx_clear(postfx *fx) {
    for (int i = 0; i < array_count(fx->pass); i++) {
        fx->pass[i].enabled = 0;
    }
    fx->enabled = 0;
}
unsigned postfx_program(postfx *fx, int pass) {
    if( pass < 0 || pass >= array_count(fx->pass) ) return 0;
    return fx->pass[pass].program;
}

int ui_postfx(postfx *fx, int pass) {
    if (pass < 0 || pass >= array_count(fx->pass)) return 0;
    int on = ui_enabled();
    ( postfx_enabled(fx,pass) ? ui_enable : ui_disable )();
    int rc = ui_shader(fx->pass[pass].program);
    ui_separator();
    int btn = ui_buttons(2, "Move up", "Move down");
    if (btn == 1) {
        postfx_order(fx, pass, fx->pass[pass].priority-1);
    } 
    else if (btn == 2) {
        postfx_order(fx, pass, fx->pass[pass].priority+1);
    }
    ( on ? ui_enable : ui_disable )();
    return rc;
}

static
int postfx_active_passes(postfx *fx) {
    int num_passes = 0;
    for (int i = 0; i < array_count(fx->pass); i++)
        if (fx->pass[i].enabled)
            ++num_passes;
    return num_passes;
}

bool postfx_begin(postfx *fx, int width, int height) {
    // reset clear color: needed in case transparent window is being used (alpha != 0)
    glClearColor(0,0,0,0); // @transparent

    width += !width;
    height += !height;

    // resize if needed
    if( fx->diffuse[0].w != width || fx->diffuse[0].h != height ) {
        texture_destroy(&fx->diffuse[0]);
        texture_destroy(&fx->diffuse[1]);
        texture_destroy(&fx->depth[0]);
        texture_destroy(&fx->depth[1]);
        fbo_destroy(fx->fb[0]);
        fbo_destroy(fx->fb[1]);

        // create texture, set texture parameters and content
        fx->diffuse[0] = texture_create(width, height, 4, NULL, TEXTURE_RGBA|TEXTURE_FLOAT);
        fx->depth[0] = texture_create(width, height, 1,  NULL, TEXTURE_DEPTH|TEXTURE_FLOAT);
        fx->fb[0] = fbo(fx->diffuse[0].id, fx->depth[0].id, 0);

        // create texture, set texture parameters and content
        fx->diffuse[1] = texture_create(width, height, 4, NULL, TEXTURE_RGBA|TEXTURE_FLOAT);
        fx->depth[1] = texture_create(width, height, 1, NULL, TEXTURE_DEPTH|TEXTURE_FLOAT);
        fx->fb[1] = fbo(fx->diffuse[1].id, fx->depth[1].id, 0);
    }

    uint64_t num_active_passes = postfx_active_passes(fx);
    bool active = fx->enabled && num_active_passes;
    if( !active ) {
        return false;
    }

    fbo_bind(fx->fb[1]);
    viewport_clear(true, true);
    viewport_clip(vec2(0,0), vec2(width, height));
    fbo_unbind();

    fbo_bind(fx->fb[0]);
    viewport_clear(true, true);
    viewport_clip(vec2(0,0), vec2(width, height));
    // we keep fbo_0 bound so that user can render into it.

    return true;
}

static renderstate_t postfx_rs;

bool postfx_end(postfx *fx) {
    uint64_t num_active_passes = postfx_active_passes(fx);
    bool active = fx->enabled && num_active_passes;
    if( !active ) {
        return false;
    }

    do_once {
        postfx_rs = renderstate();
        // disable depth test in 2d rendering
        postfx_rs.depth_test_enabled = 0;
        postfx_rs.cull_face_enabled = 0;
        postfx_rs.blend_enabled = 1;
        postfx_rs.blend_src = GL_ONE;
        postfx_rs.blend_dst = GL_ONE_MINUS_SRC_ALPHA;
    }

    // unbind postfx fbo
    fbo_unbind();

    renderstate_apply(&postfx_rs);

    int frame = 0;
    float t = time_ms() / 1000.f;
    float w = fx->diffuse[0].w;
    float h = fx->diffuse[0].h;
    float mx = input(MOUSE_X);
    float my = input(MOUSE_Y);

    for(int i = 0, e = array_count(fx->pass); i < e; ++i) {
        passfx *pass = &fx->pass[i];
        if( pass->enabled ) {
            if( !pass->program ) { --num_active_passes; continue; }
            glUseProgram(pass->program);

            // bind texture to texture unit 0
            // shader_texture_unit(fx->diffuse[frame], 0);
 glActiveTexture(GL_TEXTURE0 + 0);            glBindTexture(GL_TEXTURE_2D, fx->diffuse[frame].id);
            glUniform1i(pass->uniforms[u_color], 0);

            glUniform1f(pass->uniforms[u_channelres0x], fx->diffuse[frame].w);
            glUniform1f(pass->uniforms[u_channelres0y], fx->diffuse[frame].h);
            
            // bind depth to texture unit 1
            // shader_texture_unit(fx->depth[frame], 1);
 glActiveTexture(GL_TEXTURE0 + 1);            glBindTexture(GL_TEXTURE_2D, fx->depth[frame].id);
            glUniform1i(pass->uniforms[u_depth], 1);

            // bind uniforms
            static unsigned f = 0; ++f;
            glUniform1f(pass->uniforms[u_time], t);
            glUniform1f(pass->uniforms[u_frame], f-1);
            glUniform1f(pass->uniforms[u_width], w);
            glUniform1f(pass->uniforms[u_height], h);

            glUniform1f(pass->uniforms[u_mousex], mx);
            glUniform1f(pass->uniforms[u_mousey], my);

            // bind the vao
            int bound = --num_active_passes;
            if (bound) fbo_bind(fx->fb[frame ^= 1]);

                // fullscreen quad
                glBindVertexArray(pass->m.vao);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                profile_incstat("Render.num_drawcalls", +1);
                profile_incstat("Render.num_triangles", +2);
                glBindVertexArray(0);

            if (bound) fbo_unbind();
        }
    }
    glUseProgram(0);

    // restore clear color: needed in case transparent window is being used (alpha != 0)
    glClearColor(0,0,0,1); // @transparent

    return true;
}

static postfx fx;
int fx_load_from_mem(const char *nameid, const char *content) {
    do_once postfx_create(&fx, 0);
    return postfx_load_from_mem(&fx, nameid, content);
}
int fx_load(const char *filemask) {
    static set(char*) added = 0; do_once set_init_str(added);
    for each_array( vfs_list(filemask), char*, list ) {
        if( set_find(added, list) ) continue;
        char *name = STRDUP(list); // @leak
        set_insert(added, name);
        (void)postfx_load_from_mem(&fx, file_name(name), vfs_read(name));
    }
    if( 1 )
    for each_array( file_list(filemask), char*, list ) {
        if( set_find(added, list) ) continue;
        char *name = STRDUP(list); // @leak
        set_insert(added, name);
        (void)postfx_load_from_mem(&fx, file_name(name), file_read(name));
    }
    return 1;
}
void fx_begin() {
    postfx_begin(&fx, window_width(), window_height());
}
void fx_begin_res(int w, int h) {
    postfx_begin(&fx, w, h);
}
void fx_end() {
    postfx_end(&fx);
}
int fx_enabled(int pass) {
    return postfx_enabled(&fx, pass);
}
void fx_enable(int pass, int enabled) {
    postfx_enable(&fx, pass, enabled);
}
void fx_enable_all(int enabled) {
    for( int i = 0; i < array_count(fx.pass); ++i ) fx_enable(i, enabled);
}
char *fx_name(int pass) {
    return postfx_name(&fx, pass);
}
int fx_find(const char *name) {
    return postfx_find(&fx, name);
}
void fx_order(int pass, unsigned priority) {
    postfx_order(&fx, pass, priority);
}
unsigned fx_program(int pass) {
    return postfx_program(&fx, pass);
}
void fx_setparam(int pass, const char *name, float value) {
    unsigned program = fx_program(pass);
    if( !program ) return;
    unsigned oldprogram = shader_bind(program);
    shader_float(name, value);
    shader_bind(oldprogram);
}
int ui_fx(int pass) {
    return ui_postfx(&fx, pass);
}
int ui_fxs() {
    if(!array_count(fx.pass)) return ui_label(ICON_MD_WARNING " No Post FXs with annotations loaded."), 0;

    int changed = 0;
    for( int i = 0; i < array_count(fx.pass); ++i ) {
        char *name = fx_name(i); if( !name ) break;
        bool b = fx_enabled(i);
        if( ui_bool(name, &b) ) fx_enable(i, fx_enabled(i) ^ 1);
        ui_fx(i);
        ui_separator();
    }
    return changed;
}

// -----------------------------------------------------------------------------
// brdf

static texture_t brdf = {0};

static void brdf_load() {
    // generate texture
    unsigned tex;
    glGenTextures(1, &tex);

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

    brdf.id = tex;
    brdf.w = 512;
    brdf.h = 512;

    // create program and generate BRDF LUT
    unsigned lut_fbo = fbo(tex, 0, 0), rbo=0;
    fbo_bind(lut_fbo);

    static int program = -1, vao = -1;
    if( program < 0 ) {
        const char* vs = vfs_read("shaders/vs_0_2_fullscreen_quad_B_flipped.glsl");
        const char* fs = vfs_read("shaders/brdf_lut.glsl");

        program = shader(vs, fs, "", "fragcolor", NULL);
        glGenVertexArrays( 1, (GLuint*)&vao );
    }

    glDisable(GL_BLEND);

    handle old_shader = last_shader;
    glUseProgram( program );

    glViewport(0, 0, 512, 512);

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glBindVertexArray( vao );

    glDrawArrays( GL_TRIANGLES, 0, 6 );
    profile_incstat("Render.num_drawcalls", +1);
    profile_incstat("Render.num_triangles", +2);

    glBindVertexArray( 0 );

    glUseProgram( last_shader );

    fbo_unbind();
    fbo_destroy(lut_fbo);
}

texture_t brdf_lut() {
    do_once brdf_load();
    return brdf;
}

// -----------------------------------------------------------------------------
// materials

bool colormap( colormap_t *cm, const char *texture_name, bool load_as_srgb ) {
    if( !texture_name ) return false;

    if( cm->texture ) {
        texture_destroy(cm->texture);
        FREE(cm->texture), cm->texture = NULL;
    }

    int srgb = load_as_srgb ? TEXTURE_SRGB : 0;
    // int srgb = 0;
    int hdr = strendi(texture_name, ".hdr") ? TEXTURE_FLOAT|TEXTURE_RGBA : 0;
    texture_t t = texture_compressed(texture_name, TEXTURE_LINEAR | TEXTURE_ANISOTROPY | TEXTURE_MIPMAPS | TEXTURE_REPEAT | hdr | srgb);

    if( t.id == texture_checker().id ) {
        cm->texture = NULL;
        return false;
    }
    cm->texture = CALLOC(1, sizeof(texture_t));
    *cm->texture = t;
    return true;
}

// ----------------------------------------------------------------------------
// shadertoys
//
// @todo: multipass
// - https://www.shadertoy.com/view/Mst3Wr - la calanque
// - https://www.shadertoy.com/view/XsyGWV - sirenian dawn
// - https://www.shadertoy.com/view/Xst3zX - wordtoy
// - https://www.shadertoy.com/view/MddGzf - bricks game
// - https://www.shadertoy.com/view/Ms33WB - post process - ssao
// - https://www.shadertoy.com/view/Xds3zN

enum shadertoy_uniforms {
    iFrame,
    iTime,
    iDate,
    iGlobalTime,
    iGlobalFrame,
    iGlobalDelta,
    iChannel0,
    iChannel1,
    iChannel2,
    iChannel3,
    iResolution,
    iMouse,
    iOffset,
    iSampleRate,
    iChannelResolution,
    iChannelTime,
    // iCameraScreen
    // iCameraPosition
    // iCameraActive
};

shadertoy_t shadertoy( const char *shaderfile, unsigned flags ) {
    shadertoy_t s = {0};
    s.flags = flags;

    char *file = vfs_read(shaderfile);
    if( !file ) return s;

    glGenVertexArrays(1, &s.vao);

    char *fs = stringf("%s%s", vfs_read("header_shadertoy.glsl"), file);
    s.program = shader((flags&SHADERTOY_FLIP_Y) ? vfs_read("shaders/vs_shadertoy_flip.glsl") : vfs_read("shaders/vs_shadertoy.glsl"), fs, "", "fragColor", NULL);
    FREE(fs);

    if( strstr(file, "noise3.jpg"))
    s.texture_channels[0] = texture("shadertoys/tex12.png", 0).id;
    else
    s.texture_channels[0] = texture("shadertoys/tex04.jpg", 0).id;

    s.uniforms[iFrame] = glGetUniformLocation(s.program, "iFrame");
    s.uniforms[iTime] = glGetUniformLocation(s.program, "iTime");
    s.uniforms[iDate] = glGetUniformLocation(s.program, "iDate");
    s.uniforms[iGlobalTime] = glGetUniformLocation(s.program, "iGlobalTime");
    s.uniforms[iGlobalDelta] = glGetUniformLocation(s.program, "iGlobalDelta");
    s.uniforms[iGlobalFrame] = glGetUniformLocation(s.program, "iGlobalFrame");
    s.uniforms[iResolution] = glGetUniformLocation(s.program, "iResolution");

    s.uniforms[iChannel0] = glGetUniformLocation(s.program, "iChannel0");
    s.uniforms[iChannel1] = glGetUniformLocation(s.program, "iChannel1");
    s.uniforms[iChannel2] = glGetUniformLocation(s.program, "iChannel2");
    s.uniforms[iChannel3] = glGetUniformLocation(s.program, "iChannel3");

    s.uniforms[iMouse] = glGetUniformLocation(s.program, "iMouse");
    s.uniforms[iOffset] = glGetUniformLocation(s.program, "iOffset");
    s.uniforms[iSampleRate] = glGetUniformLocation(s.program, "iSampleRate");
    s.uniforms[iChannelResolution] = glGetUniformLocation(s.program, "iChannelResolution");
    s.uniforms[iChannelTime] = glGetUniformLocation(s.program, "iChannelTime");

    return s;
}

shadertoy_t* shadertoy_render(shadertoy_t *s, float delta) {
    int saved_vp[4];
    glGetIntegerv(GL_VIEWPORT, saved_vp);
    if( s->program && s->vao ) {
        if( s->dims.x && !(s->flags&SHADERTOY_IGNORE_FBO) && !texture_rec_begin(&s->tx, s->dims.x, s->dims.y) ) {
            return s;
        }

        if(input_down(MOUSE_L) || input_down(MOUSE_R) ) s->mouse.z = input(MOUSE_X), s->mouse.w = -(window_height() - input(MOUSE_Y));
        if(input(MOUSE_L) || input(MOUSE_R)) s->mouse.x = input(MOUSE_X), s->mouse.y = (window_height() - input(MOUSE_Y));
        vec4 m = mul4(s->mouse, vec4(1,1,1-2*(!input(MOUSE_L) && !input(MOUSE_R)),1-2*(input_down(MOUSE_L) || input_down(MOUSE_R))));

        time_t tmsec = time(0);
        struct tm *tm = localtime(&tmsec);
        s->t += delta * 1000;

        glUseProgram(s->program);
        glUniform1f(s->uniforms[iGlobalTime], s->t / 1000.f );
        glUniform1f(s->uniforms[iGlobalFrame], s->frame++);
        glUniform1f(s->uniforms[iGlobalDelta], delta / 1000.f );
        glUniform2f(s->uniforms[iResolution], s->dims.x ? s->dims.x : window_width(), s->dims.y ? s->dims.y : window_height());
        if (!(s->flags&SHADERTOY_IGNORE_MOUSE)) glUniform4f(s->uniforms[iMouse], m.x,m.y,m.z,m.w );

        glUniform1i(s->uniforms[iFrame], (int)window_frame());
        glUniform1f(s->uniforms[iTime], time_ss());
        glUniform4f(s->uniforms[iDate], tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_sec + tm->tm_min * 60 + tm->tm_hour * 3600);

        int unit = 0;
        for( int i = 0; i < 4; i++ ) {
            if( s->texture_channels[i] ) {
                glActiveTexture(GL_TEXTURE0 + unit);
                glBindTexture(GL_TEXTURE_2D, s->texture_channels[i]);
                glUniform1i(s->uniforms[iChannel0+i], unit);
                unit++;
            }
        }

        glViewport(0, 0, s->dims.x ? s->dims.x : window_width(), s->dims.y ? s->dims.y : window_height());
        glBindVertexArray(s->vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        if(s->dims.x && !(s->flags&SHADERTOY_IGNORE_FBO)) texture_rec_end(&s->tx); // texture_rec
    }
    glViewport(saved_vp[0], saved_vp[1], saved_vp[2], saved_vp[3]);
    return s;
}

// -----------------------------------------------------------------------------
// skeletal meshes (iqm)

#define IQM_MAGIC "INTERQUAKEMODEL"
#define IQM_VERSION 2

struct iqmheader {
    char magic[16];
    unsigned version;
    unsigned filesize;
    unsigned flags;
    unsigned num_text, ofs_text;
    unsigned num_meshes, ofs_meshes;
    unsigned num_vertexarrays, num_vertexes, ofs_vertexarrays;
    unsigned num_triangles, ofs_triangles, ofs_adjacency;
    unsigned num_joints, ofs_joints;
    unsigned num_poses, ofs_poses;
    unsigned num_anims, ofs_anims;
    unsigned num_frames, num_framechannels, ofs_frames, ofs_bounds;
    unsigned num_comment, ofs_comment;
    unsigned num_extensions, ofs_extensions;
};

struct iqmmesh {
    unsigned name;
    unsigned material;
    unsigned first_vertex, num_vertexes;
    unsigned first_triangle, num_triangles;
};

enum {
    IQM_POSITION,
    IQM_TEXCOORD,
    IQM_NORMAL,
    IQM_TANGENT,
    IQM_BLENDINDEXES,
    IQM_BLENDWEIGHTS,
    IQM_COLOR,
    IQM_CUSTOM = 0x10
};

enum {
    IQM_BYTE,
    IQM_UBYTE,
    IQM_SHORT,
    IQM_USHORT,
    IQM_INT,
    IQM_UINT,
    IQM_HALF,
    IQM_FLOAT,
    IQM_DOUBLE,
};

struct iqmtriangle {
    unsigned vertex[3];
};

struct iqmadjacency {
    unsigned triangle[3];
};

struct iqmjoint {
    unsigned name;
    int parent;
    float translate[3], rotate[4], scale[3];
};

struct iqmpose {
    int parent;
    unsigned mask;
    float channeloffset[10];
    float channelscale[10];
};

struct iqmanim {
    unsigned name;
    unsigned first_frame, num_frames;
    float framerate;
    unsigned flags;
};

enum {
    IQM_LOOP = 1<<0
};

struct iqmvertexarray {
    unsigned type;
    unsigned flags;
    unsigned format;
    unsigned size;
    unsigned offset;
};

struct iqmbounds {
    union {
        struct { float bbmin[3], bbmax[3]; };
        struct { vec3 min3, max3; };
        aabb box;
    };
    float xyradius, radius;
};

// -----------------------------------------------------------------------------

typedef struct iqm_vertex {
    GLfloat position[3];
    GLfloat texcoord[2];
    GLfloat normal[3];
    GLfloat tangent[4];
    GLubyte blendindexes[4];
    GLubyte blendweights[4];
    GLfloat blendvertexindex;
    GLfloat color[4];
    GLfloat texcoord2[2];
} iqm_vertex;

STATIC_ASSERT((sizeof(iqm_vertex) == sizeof(model_vertex_t)));

typedef struct iqm_t {
    int nummeshes, numtris, numverts, numjoints, numframes, numanims;
    GLuint vao, ibo, vbo;
    GLuint *textures;
    uint8_t *buf, *meshdata, *animdata;
    struct iqmmesh *meshes;
    struct iqmjoint *joints;
    struct iqmpose *poses;
    struct iqmanim *anims;
    struct iqmbounds *bounds;
    mat34 *baseframe, *inversebaseframe, *outframe, *frames;
    GLint bonematsoffset;
    vec4 *colormaps;
    uint32_t instancing_checksum;
    int uniforms[2][NUM_MODEL_UNIFORMS];
    handle program;
    handle shadow_program;
    unsigned light_ubo;
    int texture_unit;
} iqm_t;

//
// model binds
//

bool model_compareuniform(const model_uniform_t *a, const model_uniform_t *b) {
    if (a->kind != b->kind) return false;
    if (strcmp(a->name, b->name) != 0) return false;

    switch (a->kind) {
        case UNIFORM_FLOAT:
            if (a->f != b->f) return false;
            break;
        case UNIFORM_INT:
        case UNIFORM_UINT:
        case UNIFORM_BOOL:
        case UNIFORM_SAMPLER2D:
        case UNIFORM_SAMPLER3D:
        case UNIFORM_SAMPLERCUBE:
            if (a->i != b->i) return false;
            break;
        case UNIFORM_VEC2:
            if (memcmp(&a->v2.x, &b->v2.x, sizeof(vec2)) != 0) return false;
            break;
        case UNIFORM_VEC3:
            if (memcmp(&a->v3.x, &b->v3.x, sizeof(vec3)) != 0) return false;
            break;
        case UNIFORM_VEC4:
            if (memcmp(&a->v4.x, &b->v4.x, sizeof(vec4)) != 0) return false;
            break;
        case UNIFORM_MAT3:
            if (memcmp(a->m33, b->m33, sizeof(mat33)) != 0) return false;
            break;
        case UNIFORM_MAT4:
            if (memcmp(a->m44, b->m44, sizeof(mat44)) != 0) return false;
            break;
        default:
            return false;
    }
    return true;
}

bool model_compareuniforms(unsigned s1, const model_uniform_t *a, unsigned s2, const model_uniform_t *b) {
    if (s1 != s2) return false;
    
    for (unsigned i = 0; i < s1; ++i) {
        if (!model_compareuniform(&a[i], &b[i])) return false;
    }
    
    return true;
}

uint32_t model_uniforms_checksum(unsigned count, model_uniform_t *uniforms) {
    uint32_t checksum = 0;
    for (unsigned i = 0; i < count; ++i) {
        checksum ^= hh_str(uniforms[i].name);
        checksum ^= uniforms[i].kind;
        
        switch (uniforms[i].kind) {
            case UNIFORM_FLOAT:
                checksum ^= hh_float(uniforms[i].f);
                break;
            case UNIFORM_INT:
            case UNIFORM_UINT:
            case UNIFORM_BOOL:
            case UNIFORM_SAMPLER2D:
            case UNIFORM_SAMPLER3D:
            case UNIFORM_SAMPLERCUBE:
                checksum ^= hh_int(uniforms[i].i);
                break;
            case UNIFORM_VEC2:
                checksum ^= hh_vec2(uniforms[i].v2);
                break;
            case UNIFORM_VEC3:
                checksum ^= hh_vec3(uniforms[i].v3);
                break;
            case UNIFORM_VEC4:
                checksum ^= hh_vec4(uniforms[i].v4);
                break;
            case UNIFORM_MAT3:
                checksum ^= hh_mat33(uniforms[i].m33);
                break;
            case UNIFORM_MAT4:
                checksum ^= hh_mat44(uniforms[i].m44);
                break;
        }
    }
    return checksum;
}

void model_set_texture(model_t *m, texture_t t) {
    if(!m->iqm) return;
    iqm_t *q = m->iqm;

    for( int i = 0; i < q->nummeshes; ++i) { // assume 1 texture per mesh
        q->textures[i] = t.id;
        if (!m->materials[i].layer[MATERIAL_CHANNEL_DIFFUSE].map.texture) {
            m->materials[i].layer[MATERIAL_CHANNEL_DIFFUSE].map.texture = CALLOC(1, sizeof(texture_t));
        }
        *m->materials[i].layer[MATERIAL_CHANNEL_DIFFUSE].map.texture = t;
    }
}

static inline
void model_init_uniforms(iqm_t *q, int slot, int program) {
    if(!q) return;

    for (int i = 0; i < NUM_MODEL_UNIFORMS; ++i) q->uniforms[slot][i] = -1;
    unsigned shader = program;
    int loc = -1;
    glUseProgram(shader);

    // MV Matrix
    if ((loc = glGetUniformLocation(shader, "u_mv")) >= 0 || (loc = glGetUniformLocation(shader, "MV")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_MV] = loc;

    // MVP Matrix
    if ((loc = glGetUniformLocation(shader, "u_mvp")) >= 0 || (loc = glGetUniformLocation(shader, "MVP")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_MVP] = loc;


    // VP Matrix
    if ((loc = glGetUniformLocation(shader, "u_vp")) >= 0 || (loc = glGetUniformLocation(shader, "VP")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_VP] = loc;

    // Camera Position
    if ((loc = glGetUniformLocation(shader, "u_cam_pos")) >= 0 || (loc = glGetUniformLocation(shader, "cam_pos")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_CAM_POS] = loc;

    // Camera Direction
    if ((loc = glGetUniformLocation(shader, "u_cam_dir")) >= 0 || (loc = glGetUniformLocation(shader, "cam_dir")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_CAM_DIR] = loc;

    // Billboard
    if ((loc = glGetUniformLocation(shader, "u_billboard")) >= 0 || (loc = glGetUniformLocation(shader, "billboard")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_BILLBOARD] = loc;

    // Model Matrix
    if ((loc = glGetUniformLocation(shader, "M")) >= 0 || (loc = glGetUniformLocation(shader, "model")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_MODEL] = loc;

    // View Matrix
    if ((loc = glGetUniformLocation(shader, "V")) >= 0 || (loc = glGetUniformLocation(shader, "view")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_VIEW] = loc;

    // Instanced
    if ((loc = glGetUniformLocation(shader, "u_instanced")) >= 0 || (loc = glGetUniformLocation(shader, "instanced")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_INSTANCED] = loc;

    // Inverse View Matrix
    if ((loc = glGetUniformLocation(shader, "inv_view")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_INV_VIEW] = loc;

    // Projection Matrix
    if ((loc = glGetUniformLocation(shader, "P")) >= 0 || (loc = glGetUniformLocation(shader, "proj")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_PROJ] = loc;

    // Skinned
    if ((loc = glGetUniformLocation(shader, "SKINNED")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_SKINNED] = loc;

    // Bone Matrix
    if ((loc = glGetUniformLocation(shader, "vsBoneMatrix")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_VS_BONE_MATRIX] = loc;

    // Matcaps
    if ((loc = glGetUniformLocation(shader, "u_matcaps")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_U_MATCAPS] = loc;

    // Frame Count
    if ((loc = glGetUniformLocation(shader, "frame_count")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_FRAME_COUNT] = loc;

    // Frame Time
    if ((loc = glGetUniformLocation(shader, "frame_time")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_FRAME_TIME] = loc;

    // Shadow Uniforms
    if ((loc = glGetUniformLocation(shader, "cameraToShadowView")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_SHADOW_CAMERA_TO_SHADOW_VIEW] = loc;

    if ((loc = glGetUniformLocation(shader, "cameraToShadowProjector")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_SHADOW_CAMERA_TO_SHADOW_PROJECTOR] = loc;

    if ((loc = glGetUniformLocation(shader, "shadow_technique")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_SHADOW_TECHNIQUE] = loc;

    if ((loc = glGetUniformLocation(shader, "u_shadow_receiver")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_U_SHADOW_RECEIVER] = loc;

    if ((loc = glGetUniformLocation(shader, "shadow_offsets")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_SHADOW_OFFSETS] = loc;

    if ((loc = glGetUniformLocation(shader, "shadow_filter_size")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_SHADOW_FILTER_SIZE] = loc;

    if ((loc = glGetUniformLocation(shader, "shadow_window_size")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_SHADOW_WINDOW_SIZE] = loc;

    // PBR Uniforms
    if ((loc = glGetUniformLocation(shader, "resolution")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_RESOLUTION] = loc;

    if ((loc = glGetUniformLocation(shader, "has_tex_skycube")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_HAS_TEX_ENV_CUBEMAP] = loc;

    if ((loc = glGetUniformLocation(shader, "tex_skycube")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_TEX_ENV_CUBEMAP] = loc;

    if ((loc = glGetUniformLocation(shader, "has_tex_skysphere")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_HAS_TEX_SKYSPHERE] = loc;

    if ((loc = glGetUniformLocation(shader, "has_tex_skyenv")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_HAS_TEX_SKYENV] = loc;

    if ((loc = glGetUniformLocation(shader, "tex_skysphere")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_TEX_SKYSPHERE] = loc;

    if ((loc = glGetUniformLocation(shader, "skysphere_mip_count")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_SKYSPHERE_MIP_COUNT] = loc;

    if ((loc = glGetUniformLocation(shader, "tex_skyenv")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_TEX_SKYENV] = loc;

    if ((loc = glGetUniformLocation(shader, "tex_brdf_lut")) >= 0)
        q->uniforms[slot][MODEL_UNIFORM_TEX_BRDF_LUT] = loc;

    for (int j = 0; j < NUM_SHADOW_CASCADES; j++) {
        if ((loc = glGetUniformLocation(shader, va("u_cascade_distances[%d]", j))) >= 0)
            q->uniforms[slot][MODEL_UNIFORM_SHADOW_CASCADE_DISTANCES + j] = loc;
        if ((loc = glGetUniformLocation(shader, va("shadowMap2D[%d]", j))) >= 0)
            q->uniforms[slot][MODEL_UNIFORM_SHADOW_MAP_2D + j] = loc;
    }
    for (int i = 0; i < MAX_LIGHTS; i++) {
        if ((loc = glGetUniformLocation(shader, va("shadowMap[%d]", i))) >= 0)
            q->uniforms[slot][MODEL_UNIFORM_SHADOW_MAP_CUBEMAP+i] = loc;
    }
}


static int model_totalTextureUnits = 0;
int model_texture_unit(model_t *m) {
    do_once glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &model_totalTextureUnits);
    // ASSERT(textureUnit < totalTextureUnits, "%d texture units exceeded", totalTextureUnits);
    return MODEL_TEXTURE_USER_DEFINED + (m->iqm->texture_unit++ % (model_totalTextureUnits - MODEL_TEXTURE_USER_DEFINED));
}

void model_applyuniform(model_t m, const model_uniform_t *t);

static
void model_set_uniforms(model_t m, int shader, mat44 mv, mat44 proj, mat44 view, mat44 model) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    int slot = shader == q->shadow_program ? 1 : 0;
    if (q->uniforms[slot][MODEL_UNIFORM_MODEL] == -1) {
        model_init_uniforms(q, slot, shader);
    }

    shader_bind(shader);
    int loc;

    if ((loc = q->uniforms[slot][MODEL_UNIFORM_MV]) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, mv);
    }
    if ((loc = q->uniforms[slot][MODEL_UNIFORM_MVP]) >= 0) {
        mat44 mvp; multiply44x2(mvp, proj, mv);
        glUniformMatrix4fv(loc, 1, GL_FALSE, mvp);
    }
    if ((loc = q->uniforms[slot][MODEL_UNIFORM_VP]) >= 0) {
        mat44 vp; multiply44x2(vp, proj, view);
        glUniformMatrix4fv(loc, 1, GL_FALSE, vp);
    }
    if ((loc = q->uniforms[slot][MODEL_UNIFORM_CAM_POS]) >= 0) {
        vec3 pos = pos44(view);
        glUniform3fv(loc, 1, &pos.x);
    }
    if ((loc = q->uniforms[slot][MODEL_UNIFORM_CAM_DIR]) >= 0) {
        vec3 dir = norm3(vec3(view[2], view[6], view[10]));
        glUniform3fv(loc, 1, &dir.x);
    }
    if ((loc = q->uniforms[slot][MODEL_UNIFORM_BILLBOARD]) >= 0) {
        glUniform1i(loc, m.billboard);
    }
    if ((loc = q->uniforms[slot][MODEL_UNIFORM_MODEL]) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, model);
    }
    if ((loc = q->uniforms[slot][MODEL_UNIFORM_VIEW]) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, view);
    }
    if ((loc = q->uniforms[slot][MODEL_UNIFORM_INV_VIEW]) >= 0) {
        mat44 inv_view;
        invert44(inv_view, view);
        glUniformMatrix4fv(loc, 1, GL_FALSE, inv_view);
    }
    if ((loc = q->uniforms[slot][MODEL_UNIFORM_PROJ]) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, proj);
    }
    if ((loc = q->uniforms[slot][MODEL_UNIFORM_SKINNED]) >= 0) {
        glUniform1i(loc, q->numanims ? GL_TRUE : GL_FALSE);
    }
    if ((loc = q->uniforms[slot][MODEL_UNIFORM_INSTANCED]) >= 0) {
        glUniform1i(loc, m.num_instances > 1 ? GL_TRUE : GL_FALSE);
    }
    if (q->numanims) {
        if ((loc = q->uniforms[slot][MODEL_UNIFORM_VS_BONE_MATRIX]) >= 0) {
            glUniformMatrix3x4fv(loc, q->numjoints, GL_FALSE, q->outframe[0]);
        }
    }
    if ((loc = q->uniforms[slot][MODEL_UNIFORM_U_MATCAPS]) >= 0) {
        glUniform1i(loc, m.flags & MODEL_MATCAPS ? GL_TRUE : GL_FALSE);
    }
    
    if ((loc = q->uniforms[slot][MODEL_UNIFORM_FRAME_COUNT]) >= 0) {
        glUniform1i(loc, (unsigned)window_frame());
    }
    if ((loc = q->uniforms[slot][MODEL_UNIFORM_FRAME_TIME]) >= 0) {
        glUniform1f(loc, (float)window_time());
    }

    // Shadow uniforms
    if (shader == q->shadow_program) {
        shadowmap_t *sm = active_shadowmap;
        ASSERT(sm);
        if ((loc = q->uniforms[slot][MODEL_UNIFORM_SHADOW_CAMERA_TO_SHADOW_VIEW]) >= 0) {
            glUniformMatrix4fv(loc, 1, GL_FALSE, sm->V);
        }
        if ((loc = q->uniforms[slot][MODEL_UNIFORM_SHADOW_CAMERA_TO_SHADOW_PROJECTOR]) >= 0) {
            glUniformMatrix4fv(loc, 1, GL_FALSE, sm->PV);
        }
        if ((loc = q->uniforms[slot][MODEL_UNIFORM_SHADOW_TECHNIQUE]) >= 0) {
            glUniform1i(loc, sm->shadow_technique);
        }
    } else {
        // Shadow receiving
        if (m.shadow_map && m.shadow_receiver) {
            if ((loc = q->uniforms[slot][MODEL_UNIFORM_U_SHADOW_RECEIVER]) >= 0) {
                glUniform1i(loc, GL_TRUE);
            }

            bool was_csm_pushed = 0;
            int shadow_lights_pushed = 0;
            for (int i = 0; i < MAX_LIGHTS; i++) {
                if (shadow_lights_pushed >= MAX_SHADOW_LIGHTS) break;
                shadow_lights_pushed++;
                if (i < m.lights.count) {
                    light_t *light = &m.lights.base[i];

                    if (light->shadow_technique == SHADOW_CSM) {
                        for (int j = 0; j < NUM_SHADOW_CASCADES; j++) {
                            shader_texture_unit_kind_(GL_TEXTURE_2D, q->uniforms[slot][MODEL_UNIFORM_SHADOW_MAP_2D + j], m.shadow_map->maps[i].texture_2d[j], MODEL_TEXTURE_SHADOW_MAP_2D + j);
                            glUniform1f(q->uniforms[slot][MODEL_UNIFORM_SHADOW_CASCADE_DISTANCES + j], m.shadow_map->maps[i].cascade_distances[j]);
                        }
                        was_csm_pushed = 1;
                        shader_texture_unit_kind_(GL_TEXTURE_CUBE_MAP, q->uniforms[slot][MODEL_UNIFORM_SHADOW_MAP_CUBEMAP+i], 0, MODEL_TEXTURE_SHADOW_MAP_CUBEMAP+i);
                    }
                    else if (light->shadow_technique == SHADOW_VSM) {
                        shader_texture_unit_kind_(GL_TEXTURE_CUBE_MAP, q->uniforms[slot][MODEL_UNIFORM_SHADOW_MAP_CUBEMAP+i], m.shadow_map->maps[i].texture, MODEL_TEXTURE_SHADOW_MAP_CUBEMAP+i);
                    }
                } else {
                    if (!was_csm_pushed) {
                        was_csm_pushed = 1;
                        for (int j = 0; j < NUM_SHADOW_CASCADES; j++) {
                            glUniform1i(q->uniforms[slot][MODEL_UNIFORM_SHADOW_MAP_2D + j], MODEL_TEXTURE_SHADOW_MAP_2D + j);
                        }
                    }
                    glUniform1i(q->uniforms[slot][MODEL_UNIFORM_SHADOW_MAP_CUBEMAP+i], MODEL_TEXTURE_SHADOW_MAP_CUBEMAP+i);
                }
            }
            if ((loc = q->uniforms[slot][MODEL_UNIFORM_SHADOW_OFFSETS]) >= 0) {
                shader_texture_unit_kind_(GL_TEXTURE_3D, q->uniforms[slot][MODEL_UNIFORM_SHADOW_OFFSETS], m.shadow_map->offsets_texture, MODEL_TEXTURE_SHADOW_OFFSETS);
            }
            if ((loc = q->uniforms[slot][MODEL_UNIFORM_SHADOW_FILTER_SIZE]) >= 0) {
                glUniform1i(loc, m.shadow_map->filter_size);
            }
            if ((loc = q->uniforms[slot][MODEL_UNIFORM_SHADOW_WINDOW_SIZE]) >= 0) {
                glUniform1i(loc, m.shadow_map->window_size);
            }
        }
        else if (m.shadow_map == NULL || !m.shadow_receiver) {
            if ((loc = q->uniforms[slot][MODEL_UNIFORM_U_SHADOW_RECEIVER]) >= 0) {
                glUniform1i(loc, GL_FALSE);
            }
            for (int j = 0; j < NUM_SHADOW_CASCADES; j++) {
                if ((loc = q->uniforms[slot][MODEL_UNIFORM_SHADOW_MAP_2D + j]) >= 0) {
                    glUniform1i(loc, MODEL_TEXTURE_SHADOW_MAP_2D + j);
                }
            }
            for (int i = 0; i < MAX_SHADOW_LIGHTS; i++) {
                if ((loc = q->uniforms[slot][MODEL_UNIFORM_SHADOW_MAP_CUBEMAP + i]) >= 0) {
                    glUniform1i(loc, MODEL_TEXTURE_SHADOW_MAP_CUBEMAP + i);
                }
            }
            if ((loc = q->uniforms[slot][MODEL_UNIFORM_SHADOW_OFFSETS]) >= 0) {
                glUniform1i(q->uniforms[slot][MODEL_UNIFORM_SHADOW_OFFSETS], MODEL_TEXTURE_SHADOW_OFFSETS);
            }
        }
    }

    if (m.shading == SHADING_PBR) {
        if ((loc = q->uniforms[slot][MODEL_UNIFORM_RESOLUTION]) >= 0) {
            glUniform2f(loc, (float)window_width(), (float)window_height());
        }
        
        bool has_tex_skysphere = m.sky_refl.id != 0;
        bool has_tex_skyenv = m.sky_env.id != 0;
        bool has_tex_env_cubemap = m.sky_cubemap.id != 0;
        glUniform1i(q->uniforms[slot][MODEL_UNIFORM_HAS_TEX_ENV_CUBEMAP], has_tex_env_cubemap);
        glUniform1i(q->uniforms[slot][MODEL_UNIFORM_HAS_TEX_SKYSPHERE], has_tex_skysphere);
        glUniform1i(q->uniforms[slot][MODEL_UNIFORM_HAS_TEX_SKYENV], has_tex_skyenv);
        if (has_tex_env_cubemap) {
            shader_texture_unit_kind_(GL_TEXTURE_CUBE_MAP, q->uniforms[slot][MODEL_UNIFORM_TEX_ENV_CUBEMAP], m.sky_cubemap.id, MODEL_TEXTURE_ENV_CUBEMAP);
        } else {
            glUniform1i(q->uniforms[slot][MODEL_UNIFORM_TEX_ENV_CUBEMAP], MODEL_TEXTURE_ENV_CUBEMAP);
        }
        if( has_tex_skysphere ) {
            float mipCount = floor( log2( max(m.sky_refl.w, m.sky_refl.h) ) );
            shader_texture_unit_kind_(GL_TEXTURE_2D, q->uniforms[slot][MODEL_UNIFORM_TEX_SKYSPHERE], m.sky_refl.id, MODEL_TEXTURE_SKYSPHERE);
            glUniform1f(q->uniforms[slot][MODEL_UNIFORM_SKYSPHERE_MIP_COUNT], mipCount);
        } else {
            glUniform1i(q->uniforms[slot][MODEL_UNIFORM_TEX_SKYSPHERE], MODEL_TEXTURE_SKYSPHERE);
        }
        if( has_tex_skyenv ) {
            shader_texture_unit_kind_(GL_TEXTURE_2D, q->uniforms[slot][MODEL_UNIFORM_TEX_SKYENV], m.sky_env.id, MODEL_TEXTURE_SKYENV);
        } else {
            glUniform1i(q->uniforms[slot][MODEL_UNIFORM_TEX_SKYENV], MODEL_TEXTURE_SKYENV);
        }
        shader_texture_unit_kind_(GL_TEXTURE_2D, q->uniforms[slot][MODEL_UNIFORM_TEX_BRDF_LUT], brdf_lut().id, MODEL_TEXTURE_BRDF_LUT);
    }

    /* apply custom model uniforms */
    for (int i = 0; i < array_count(m.uniforms); i++) {
        model_applyuniform(m, &m.uniforms[i]);
    }
}

static inline
uint32_t model_instancing_checksum(float *matrices, unsigned count) {
    uint32_t checksum = 0;
    float *data = matrices;
    int total_floats = count * 16;

    for (int i = 0; i < total_floats; i++) {
        uint32_t int_value = *(uint32_t*)&data[i];
        checksum = ((checksum << 5) + checksum) + int_value;
    }

    return checksum;
}

static
void model_set_state(model_t m) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    glBindVertexArray( q->vao );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, q->ibo);
    glBindBuffer(GL_ARRAY_BUFFER, q->vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, position) );
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, texcoord) );
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, normal) );
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, tangent) );

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    // vertex color
    glVertexAttribPointer(11, 4, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex,color) );
    glEnableVertexAttribArray(11);

    // lmap data
    glVertexAttribPointer(12, 2, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, texcoord2) );
    glEnableVertexAttribArray(12);

    // animation
    if(q->numframes > 0) {
        glVertexAttribPointer( 8, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex,blendindexes) );
        glVertexAttribPointer( 9, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex,blendweights) );
        glVertexAttribPointer(10, 1, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, blendvertexindex) );
        glEnableVertexAttribArray(8);
        glEnableVertexAttribArray(9);
        glEnableVertexAttribArray(10);
    }

    // mat4 attribute; for instanced rendering
    if(m.num_instances > 1) {
        unsigned vec4_size = sizeof(vec4);
        unsigned mat4_size = sizeof(vec4) * 4;

        // vertex buffer object
        glBindBuffer(GL_ARRAY_BUFFER, m.vao_instanced);

        uint32_t checksum = model_instancing_checksum(m.instanced_matrices, m.num_instances);
        if (checksum != q->instancing_checksum) {
            q->instancing_checksum = checksum;
            glBufferData(GL_ARRAY_BUFFER, m.num_instances * mat4_size, m.instanced_matrices, GL_STREAM_DRAW);
        }

        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (GLvoid*)(((char*)NULL)+(0 * vec4_size)));
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (GLvoid*)(((char*)NULL)+(1 * vec4_size)));
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (GLvoid*)(((char*)NULL)+(2 * vec4_size)));
        glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (GLvoid*)(((char*)NULL)+(3 * vec4_size)));

        glEnableVertexAttribArray(4);
        glEnableVertexAttribArray(5);
        glEnableVertexAttribArray(6);
        glEnableVertexAttribArray(7);

        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glVertexAttribDivisor(7, 1);
    }

    // 7 bitangent? into texcoord.z?

    glBindVertexArray( 0 );
}

void model_sync(model_t m, int num_vertices, model_vertex_t *vertices, int num_indices, uint32_t *indices) {
    if (!m.iqm) return;
    iqm_t *q = m.iqm;

    if (!(m.flags & MODEL_PROCEDURAL)) {
        PANIC("model_sync() cannot be used with non-procedural models");
    }

    if (!q->vao) glGenVertexArrays(1, &q->vao);
    glBindVertexArray(q->vao);
   
    if (num_vertices > 0) { 
        if(!q->vbo) glGenBuffers(1, &q->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, q->vbo);
        glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(model_vertex_t), vertices, m.flags & MODEL_STREAM ? GL_STREAM_DRAW : GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if (num_indices > 0) {
        if (!q->ibo) glGenBuffers(1, &q->ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, q->ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(uint32_t), indices, m.flags & MODEL_STREAM ? GL_STREAM_DRAW : GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    q->nummeshes = 1;
    q->numtris = num_indices/3;
    q->numverts = num_vertices;

    if (!q->meshes) q->meshes = CALLOC(q->nummeshes, sizeof(struct iqmmesh));
    struct iqmmesh *mesh = &q->meshes[0];
    mesh->first_vertex = 0;
    mesh->num_vertexes = num_vertices;
    mesh->first_triangle = 0;
    mesh->num_triangles = num_indices/3;

    model_set_state(m);
    glBindVertexArray(0);
}

static
bool model_load_meshes(iqm_t *q, const struct iqmheader *hdr, model_t *m) {
    if(q->meshdata) return false;

    lil32p(&q->buf[hdr->ofs_vertexarrays], hdr->num_vertexarrays*sizeof(struct iqmvertexarray)/sizeof(uint32_t));
    lil32p(&q->buf[hdr->ofs_triangles], hdr->num_triangles*sizeof(struct iqmtriangle)/sizeof(uint32_t));
    lil32p(&q->buf[hdr->ofs_meshes], hdr->num_meshes*sizeof(struct iqmmesh)/sizeof(uint32_t));
    lil32p(&q->buf[hdr->ofs_joints], hdr->num_joints*sizeof(struct iqmjoint)/sizeof(uint32_t));

    q->meshdata = q->buf;
    q->nummeshes = hdr->num_meshes;
    q->numtris = hdr->num_triangles;
    q->numverts = hdr->num_vertexes;
    q->numjoints = hdr->num_joints;
    q->outframe = CALLOC(hdr->num_joints, sizeof(mat34));

    float *inposition = NULL, *innormal = NULL, *intangent = NULL, *intexcoord = NULL, *invertexindex = NULL;
    uint8_t *inblendindex8 = NULL, *inblendweight8 = NULL;
    int *inblendindexi = NULL; float *inblendweightf = NULL;
    float *invertexcolor = NULL;
    struct iqmvertexarray *vas = (struct iqmvertexarray *)&q->buf[hdr->ofs_vertexarrays];
    for(int i = 0; i < (int)hdr->num_vertexarrays; i++) {
        struct iqmvertexarray *va = &vas[i];
        switch(va->type) {
        default: continue; // return PANIC("unknown iqm vertex type (%d)", va->type), false;
        break; case IQM_POSITION: ASSERT(va->format == IQM_FLOAT && va->size == 3); inposition = (float *)&q->buf[va->offset]; lil32pf(inposition, 3*hdr->num_vertexes);
        break; case IQM_NORMAL: ASSERT(va->format == IQM_FLOAT && va->size == 3); innormal = (float *)&q->buf[va->offset]; lil32pf(innormal, 3*hdr->num_vertexes);
        break; case IQM_TANGENT: ASSERT(va->format == IQM_FLOAT && va->size == 4); intangent = (float *)&q->buf[va->offset]; lil32pf(intangent, 4*hdr->num_vertexes);
        break; case IQM_TEXCOORD: ASSERT(va->format == IQM_FLOAT && va->size == 2); intexcoord = (float *)&q->buf[va->offset]; lil32pf(intexcoord, 2*hdr->num_vertexes);
        break; case IQM_COLOR: ASSERT(va->size == 4); ASSERT(va->format == IQM_FLOAT); invertexcolor = (float *)&q->buf[va->offset];
        break; case IQM_BLENDINDEXES: ASSERT(va->size == 4); ASSERT(va->format == IQM_UBYTE || va->format == IQM_INT);
        if(va->format == IQM_UBYTE) inblendindex8 = (uint8_t *)&q->buf[va->offset];
        else inblendindexi = (int *)&q->buf[va->offset];
        break; case IQM_BLENDWEIGHTS: ASSERT(va->size == 4); ASSERT(va->format == IQM_UBYTE || va->format == IQM_FLOAT);
        if(va->format == IQM_UBYTE) inblendweight8 = (uint8_t *)&q->buf[va->offset];
        else inblendweightf = (float *)&q->buf[va->offset];
        invertexindex = (inblendweight8 ? (float*)(inblendweight8 + 4) : inblendweightf + 4 );
        }
    }

    if (hdr->ofs_bounds) lil32p(q->buf + hdr->ofs_bounds, hdr->num_frames * sizeof(struct iqmbounds));
    if (hdr->ofs_bounds) q->bounds = (struct iqmbounds *) &q->buf[hdr->ofs_bounds];

    q->meshes = (struct iqmmesh *)&q->buf[hdr->ofs_meshes];
    q->joints = (struct iqmjoint *)&q->buf[hdr->ofs_joints];

    q->baseframe = CALLOC(hdr->num_joints, sizeof(mat34));
    q->inversebaseframe = CALLOC(hdr->num_joints, sizeof(mat34));
    for(int i = 0; i < (int)hdr->num_joints; i++) {
        struct iqmjoint *j = &q->joints[i];
        compose34(q->baseframe[i], ptr3(j->translate), normq(ptrq(j->rotate)), ptr3(j->scale));
        invert34(q->inversebaseframe[i], q->baseframe[i]);
        if(j->parent >= 0) {
            multiply34x2(q->baseframe[i], q->baseframe[j->parent], q->baseframe[i]);
            multiply34(q->inversebaseframe[i], q->inversebaseframe[j->parent]);
        }
    }

    struct iqmtriangle *tris = (struct iqmtriangle *)&q->buf[hdr->ofs_triangles];
    m->num_tris = hdr->num_triangles;
    m->tris = (void*)tris;

    glGenVertexArrays(1, &q->vao);
    glBindVertexArray(q->vao);

    if(!q->ibo) glGenBuffers(1, &q->ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, q->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, hdr->num_triangles*sizeof(struct iqmtriangle), tris, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    iqm_vertex *verts = CALLOC(hdr->num_vertexes, sizeof(iqm_vertex));
    for(int i = 0; i < (int)hdr->num_vertexes; i++) {
        iqm_vertex *v = &verts[i];
        if(inposition) memcpy(v->position, &inposition[i*3], sizeof(v->position));
        if(innormal) memcpy(v->normal, &innormal[i*3], sizeof(v->normal));
        if(intangent) memcpy(v->tangent, &intangent[i*4], sizeof(v->tangent));
        if(intexcoord) {
            memcpy(v->texcoord, &intexcoord[i*2], sizeof(v->texcoord));
            memcpy(v->texcoord2, &intexcoord[i*2], sizeof(v->texcoord2)); // populate UV1 with the same value, used by lightmapper
        }
        if(inblendindex8) memcpy(v->blendindexes, &inblendindex8[i*4], sizeof(v->blendindexes));
        if(inblendweight8) memcpy(v->blendweights, &inblendweight8[i*4], sizeof(v->blendweights));
        if(inblendindexi) {
            uint8_t conv[4] = { inblendindexi[i*4], inblendindexi[i*4+1], inblendindexi[i*4+2], inblendindexi[i*4+3] };
            memcpy(v->blendindexes, conv, sizeof(v->blendindexes));
        }
        if(inblendweightf) {
            uint8_t conv[4] = { inblendweightf[i*4] * 255, inblendweightf[i*4+1] * 255, inblendweightf[i*4+2] * 255, inblendweightf[i*4+3] * 255 };
            memcpy(v->blendweights, conv, sizeof(v->blendweights));
        }
        if(invertexindex) {
            float conv = i;
            memcpy(&v->blendvertexindex, &conv, 4);
        }
        if(invertexcolor) {
            v->color[0] = invertexcolor[i*4+0];
            v->color[1] = invertexcolor[i*4+1];
            v->color[2] = invertexcolor[i*4+2];
            v->color[3] = invertexcolor[i*4+3];
        }
        else {
            v->color[0] = 1.0f;
            v->color[1] = 1.0f;
            v->color[2] = 1.0f;
            v->color[3] = 1.0f;
        }

        /* handle vertex colors for parts of mesh that don't utilise it. */
        if (v->color[0] + v->color[1] + v->color[2] + v->color[3] < 0.001f) {
            v->color[0] = 1.0f;
            v->color[1] = 1.0f;
            v->color[2] = 1.0f;
            v->color[3] = 1.0f;
        }
    }

    if(!q->vbo) glGenBuffers(1, &q->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, q->vbo);
    glBufferData(GL_ARRAY_BUFFER, hdr->num_vertexes*sizeof(iqm_vertex), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m->stride = sizeof(iqm_vertex);
    #if 0
    m->stride = 0;
    if(inposition) m->stride += sizeof(verts[0].position);
    if(innormal) m->stride += sizeof(verts[0].normal);
    if(intangent) m->stride += sizeof(verts[0].tangent);
    if(intexcoord) m->stride += sizeof(verts[0].texcoord);
    if(inblendindex8) m->stride += sizeof(verts[0].blendindexes); // no index8? bug?
    if(inblendweight8) m->stride += sizeof(verts[0].blendweights); // no weight8? bug?
    if(inblendindexi) m->stride += sizeof(verts[0].blendindexes);
    if(inblendweightf) m->stride += sizeof(verts[0].blendweights);
    if(invertexcolor8) m->stride += sizeof(verts[0].color);
    #endif
    //for( int i = 0; i < 16; ++i ) printf("%.9g%s", ((float*)verts)[i], (i % 3) == 2 ? "\n" : ",");
    m->verts = verts;
    /*m->verts = 0; FREE(verts);*/

    q->textures = CALLOC(hdr->num_meshes * 8, sizeof(GLuint));
    q->colormaps = CALLOC(hdr->num_meshes * 8, sizeof(vec4));

    const char *str = hdr->ofs_text ? (char *)&q->buf[hdr->ofs_text] : "";
    for(int i = 0; i < (int)hdr->num_meshes; i++) {
        struct iqmmesh *m = &q->meshes[i];
        PRINTF("loaded mesh: %s\n", &str[m->name]);
    }

    return true;
}

static
bool model_load_anims(iqm_t *q, const struct iqmheader *hdr) {
    if((int)hdr->num_poses != q->numjoints) return false;

    if(q->animdata) {
        if(q->animdata != q->meshdata) FREE(q->animdata);
        FREE(q->frames);
        q->animdata = NULL;
        q->anims = NULL;
        q->frames = 0;
        q->numframes = 0;
        q->numanims = 0;
    }

    lil32p(&q->buf[hdr->ofs_poses], hdr->num_poses*sizeof(struct iqmpose)/sizeof(uint32_t));
    lil32p(&q->buf[hdr->ofs_anims], hdr->num_anims*sizeof(struct iqmanim)/sizeof(uint32_t));
    lil16p((uint16_t *)&q->buf[hdr->ofs_frames], hdr->num_frames*hdr->num_framechannels);

    q->animdata = q->buf;
    q->numanims = hdr->num_anims;
    q->numframes = hdr->num_frames;

    q->anims = (struct iqmanim *)&q->buf[hdr->ofs_anims];
    q->poses = (struct iqmpose *)&q->buf[hdr->ofs_poses];
    q->frames = CALLOC(hdr->num_frames * hdr->num_poses, sizeof(mat34));
    uint16_t *framedata = (uint16_t *)&q->buf[hdr->ofs_frames];

    for(int i = 0; i < (int)hdr->num_frames; i++) {
        for(int j = 0; j < (int)hdr->num_poses; j++) {
            struct iqmpose *p = &q->poses[j];
            quat rotate;
            vec3 translate, scale;
            translate.x = p->channeloffset[0]; if(p->mask&0x01) translate.x += *framedata++ * p->channelscale[0];
            translate.y = p->channeloffset[1]; if(p->mask&0x02) translate.y += *framedata++ * p->channelscale[1];
            translate.z = p->channeloffset[2]; if(p->mask&0x04) translate.z += *framedata++ * p->channelscale[2];

            rotate.x = p->channeloffset[3]; if(p->mask&0x08) rotate.x += *framedata++ * p->channelscale[3];
            rotate.y = p->channeloffset[4]; if(p->mask&0x10) rotate.y += *framedata++ * p->channelscale[4];
            rotate.z = p->channeloffset[5]; if(p->mask&0x20) rotate.z += *framedata++ * p->channelscale[5];
            rotate.w = p->channeloffset[6]; if(p->mask&0x40) rotate.w += *framedata++ * p->channelscale[6];

            scale.x = p->channeloffset[7]; if(p->mask&0x80)  scale.x += *framedata++ * p->channelscale[7];
            scale.y = p->channeloffset[8]; if(p->mask&0x100) scale.y += *framedata++ * p->channelscale[8];
            scale.z = p->channeloffset[9]; if(p->mask&0x200) scale.z += *framedata++ * p->channelscale[9];

            // Concatenate each pose with the inverse base pose to avoid doing this at animation time.
            // If the joint has a parent, then it needs to be pre-concatenated with its parent's base pose.
            // Thus it all negates at animation time like so:
            //   (parentPose * parentInverseBasePose) * (parentBasePose * childPose * childInverseBasePose) =>
            //   parentPose * (parentInverseBasePose * parentBasePose) * childPose * childInverseBasePose =>
            //   parentPose * childPose * childInverseBasePose

            mat34 m; compose34(m, translate, normq(rotate), scale);
            if(p->parent >= 0) multiply34x3(q->frames[i*hdr->num_poses + j], q->baseframe[p->parent], m, q->inversebaseframe[j]);
            else multiply34x2(q->frames[i*hdr->num_poses + j], m, q->inversebaseframe[j]);
        }
    }

    // const char *str = hdr->ofs_text ? (char *)&q->buf[hdr->ofs_text] : "";
    // for(int i = 0; i < (int)hdr->num_anims; i++) {
    //     struct iqmanim *a = &anims[i];
    //     PRINTF("loaded anim[%d]: %s\n", i, &str[a->name]);
    // }

    return true;
}

void ui_material(material_t *m) {
    static char* channel_names[] = {
        "Diffuse", "Normals", "Specular", "Albedo", "Roughness", "Metallic", "AO", "Ambient", "Emissive", "Parallax"
    };
    for (int i = 0; i < MAX_CHANNELS_PER_MATERIAL; i++) {
        
        if (ui_collapse(channel_names[i], va("%s_%d", m->name, i))) {
            ui_color4f(va("%s Color", channel_names[i]), &m->layer[i].map.color.x);
            
            if (m->layer[i].map.texture) {
                ui_texture(va("%s Texture", channel_names[i]), *m->layer[i].map.texture);
            }
            
            if (i == MATERIAL_CHANNEL_SPECULAR) {
                ui_float("Specular Shininess", &m->layer[i].value);
            }

            if (i == MATERIAL_CHANNEL_PARALLAX) {
                ui_float("Parallax Scale", &m->layer[i].value);
                ui_float("Parallax Shadow Power", &m->layer[i].value2);
            }
            
            ui_collapse_end();
        }
    }
}

void ui_materials(model_t *m) {
    for (int i = 0; i < array_count(m->materials); i++) {
        if (ui_collapse(m->materials[i].name, va("material_%d", i))) {
            ui_material(&m->materials[i]);
            ui_collapse_end();
        }
    }
}

// prevents crash on osx when strcpy'ing non __restrict arguments
static char* strcpy_safe(char *d, const char *s) {
    sprintf(d, "%s", s);
    return d;
}

static
void model_load_pbr_layer(material_layer_t *layer, const char *texname, bool load_as_srgb) {
    strcpy_safe(layer->texname, texname);
    colormap(&layer->map, texname, load_as_srgb);
}

static
void model_load_pbr(model_t *m, material_t *mt, int mesh) {
    if (!m->iqm) return;
    struct iqm_t *q = m->iqm;

    // initialise default colors
    // mt->layer[MATERIAL_CHANNEL_DIFFUSE].map.color = vec4(0.5,0.5,0.5,1.0);
    mt->layer[MATERIAL_CHANNEL_NORMALS].map.color = vec4(0,0,0,0);
    mt->layer[MATERIAL_CHANNEL_SPECULAR].map.color = vec4(0,0,0,0);
    mt->layer[MATERIAL_CHANNEL_SPECULAR].value = 1.0f; // specular_shininess
    mt->layer[MATERIAL_CHANNEL_ALBEDO].map.color = vec4(0.5,0.5,0.5,1.0);
    mt->layer[MATERIAL_CHANNEL_ROUGHNESS].map.color = vec4(1,1,1,1);
    mt->layer[MATERIAL_CHANNEL_METALLIC].map.color = vec4(0,0,0,0);
    mt->layer[MATERIAL_CHANNEL_AO].map.color = vec4(1,1,1,0);
    mt->layer[MATERIAL_CHANNEL_AMBIENT].map.color = vec4(0,0,0,1);
    mt->layer[MATERIAL_CHANNEL_EMISSIVE].map.color = vec4(0,0,0,0);
    mt->layer[MATERIAL_CHANNEL_PARALLAX].map.color = vec4(0,0,0,0);
    mt->layer[MATERIAL_CHANNEL_PARALLAX].value = 0.1f;
    mt->layer[MATERIAL_CHANNEL_PARALLAX].value2 = 4.0f;
    
    // load colormaps
    array(char*) tokens = strsplit(mt->name, "+");
    for( int j = 0, end = array_count(tokens); j < end; ++j ) {
        char *t = tokens[j];
        if( strstri(t, "_D.") || strstri(t, "Diffuse") || strstri(t, "BaseColor") || strstri(t, "Base_Color") )    { model_load_pbr_layer(&mt->layer[MATERIAL_CHANNEL_DIFFUSE], t, 1); continue; }
        if( strstri(t, "_N.") || strstri(t, "Normal") )     { model_load_pbr_layer(&mt->layer[MATERIAL_CHANNEL_NORMALS], t, 0); continue; }
        if( strstri(t, "_S.") || strstri(t, "Specular") )   { model_load_pbr_layer(&mt->layer[MATERIAL_CHANNEL_SPECULAR], t, 0); continue; }
        if( strstri(t, "_A.") || strstri(t, "Albedo") )     { model_load_pbr_layer(&mt->layer[MATERIAL_CHANNEL_ALBEDO], t, 1); continue; }
        if( strstri(t, "Roughness") )  { model_load_pbr_layer(&mt->layer[MATERIAL_CHANNEL_ROUGHNESS], t, 0); continue; }
        if( strstri(t, "_MR.")|| strstri(t, "MetallicRoughness") || strstri(t, "OcclusionRoughnessMetallic") )  { model_load_pbr_layer(&mt->layer[MATERIAL_CHANNEL_ROUGHNESS], t, 0); continue; }
        else
        if( strstri(t, "_M.") || strstri(t, "Metallic") )   { model_load_pbr_layer(&mt->layer[MATERIAL_CHANNEL_METALLIC], t, 0); continue; }
        if( strstri(t, "_E.") || strstri(t, "Emissive") )   { model_load_pbr_layer(&mt->layer[MATERIAL_CHANNEL_EMISSIVE], t, 1); continue; }
        if( strstri(t, "_AO.") || strstri(t, "AO") || strstri(t, "Occlusion") ) { model_load_pbr_layer(&mt->layer[MATERIAL_CHANNEL_AO], t, 0); continue; }
        if( strstri(t, "_P.") || strstri(t, "Parallax") || strstri(t, "disp.") ) { model_load_pbr_layer(&mt->layer[MATERIAL_CHANNEL_PARALLAX], t, 0); continue; }
    }
}

static
bool model_load_textures(iqm_t *q, const struct iqmheader *hdr, model_t *model, int _flags) {
    q->textures = q->textures ? q->textures : CALLOC(hdr->num_meshes * 8, sizeof(GLuint)); // up to 8 textures per mesh
    q->colormaps = q->colormaps ? q->colormaps : CALLOC(hdr->num_meshes * 8, sizeof(vec4)); // up to 8 colormaps per mesh

    texture_t tex = {0};
    GLuint *out = q->textures;

    const char *str = hdr->ofs_text ? (char *)&q->buf[hdr->ofs_text] : "";
    for(int i = 0; i < (int)hdr->num_meshes; i++) {
        struct iqmmesh *m = &q->meshes[i];

        // reuse texture+material if already decoded
        bool reused = 0;
        for( int j = 0; !reused && j < model->num_textures; ++j ) {
            if( !strcmpi(model->texture_names[j], &str[m->material])) {

                *out++ = model->materials[j].layer[0].map.texture->id;

                {
                    model->num_textures++;
                    array_push(model->texture_names, STRDUP(&str[m->material]));

                    array_push(model->materials, model->materials[j]);
                    array_back(model->materials)->name = STRDUP(&str[m->material]);
                }

                reused = true;
            }
        }
        if( reused ) continue;

        // decode texture+material
        int load_as_srgb = _flags & MODEL_NO_PBR ? 0 : TEXTURE_SRGB;
        int flags = TEXTURE_MIPMAPS|TEXTURE_REPEAT|TEXTURE_ANISOTROPY|load_as_srgb; // LINEAR, NEAREST
        if (!(_flags & MODEL_NO_FILTERING))
            flags |= TEXTURE_LINEAR;
        int invalid = texture_checker().id;

#if 1
        char *material_embedded_texture = strstr(&str[m->material], "+b64:");
        if( material_embedded_texture ) {
            *material_embedded_texture = '\0';
            material_embedded_texture += 5;
            array(char) embedded_texture = base64_decode(material_embedded_texture, strlen(material_embedded_texture));
            //printf("%s %d\n", material_embedded_texture, array_count(embedded_texture));
            //hexdump(embedded_texture, array_count(embedded_texture));
            tex = texture_compressed_from_mem( embedded_texture, array_count(embedded_texture), flags );
            *out = tex.id;
            array_free(embedded_texture);
        }

        char* material_color_hex = strstr(&str[m->material], "+$");
        vec4 material_color = vec4(1,1,1,1);
        if( material_color_hex ) {
            *material_color_hex = '\0';
            material_color_hex += 2;
            material_color.r = ((material_color_hex[0] >= 'a') ? material_color_hex[0] - 'a' + 10 : material_color_hex[0] - '0') / 15.f;
            material_color.g = ((material_color_hex[1] >= 'a') ? material_color_hex[1] - 'a' + 10 : material_color_hex[1] - '0') / 15.f;
            material_color.b = ((material_color_hex[2] >= 'a') ? material_color_hex[2] - 'a' + 10 : material_color_hex[2] - '0') / 15.f;
            material_color.a = ((material_color_hex[3] >= 'a') ? material_color_hex[3] - 'a' + 10 : material_color_hex[3] - '0') / 15.f;
            #if 0 // not enabled because of some .obj files like suzanne, with color_hex=9990 found
            if(material_color_hex[3])
            material_color.a = ((material_color_hex[3] >= 'a') ? material_color_hex[3] - 'a' + 10 : material_color_hex[3] - '0') / 15.f;
            else
            #endif
        }

        if( !material_embedded_texture ) {
            char* material_name;
            // remove any material+name from materials (.fbx)
            // try left token first
            if( 1 ) {
                material_name = va("%s", &str[m->material]);
                char* plus = strrchr(material_name, '+');
                if (plus) { strcpy_safe(plus, file_ext(material_name)); }
                tex = texture_compressed(material_name, flags);
                *out = tex.id;
            }
            // else try right token
            if (*out == invalid) {
                material_name = file_normalize( va("%s", &str[m->material]) );
                char* plus = strrchr(material_name, '+'), *slash = strrchr(material_name, '/');
                if (plus) {
                    strcpy_safe(slash ? slash + 1 : material_name, plus + 1);
                    tex = texture_compressed(material_name, flags);
                    *out = tex.id;
                }
            }
            // else last resort
            if (*out == invalid) {
                tex = texture_compressed(material_name, flags);
                *out = tex.id; // needed?
            }
        }

        if( *out != invalid) {
            PRINTF("loaded material[%d]: %s\n", i, &str[m->material]);
        } else {
            PRINTF("warn: material[%d] not found: %s\n", i, &str[m->material]);
            PRINTF("warn: using placeholder material[%d]=texture_checker\n", i);
            tex = texture_checker();
            *out = tex.id; // placeholder
        }

        inscribe_tex:;
        {
            model->num_textures++;
            array_push(model->texture_names, STRDUP(&str[m->material]));

            material_t mt = {0};
            mt.name = STRDUP(&str[m->material]);

            // initialise basic texture layer
            mt.layer[MATERIAL_CHANNEL_DIFFUSE].map.color = material_color_hex ? material_color : vec4(1,1,1,1);
            mt.layer[MATERIAL_CHANNEL_DIFFUSE].map.texture = CALLOC(1, sizeof(texture_t));
            *mt.layer[MATERIAL_CHANNEL_DIFFUSE].map.texture = tex;
            printf("texture_id: %d\n", mt.layer[MATERIAL_CHANNEL_DIFFUSE].map.texture->id);
            out++;

            array_push(model->materials, mt);
        }

#else
        material_t mt = {0};
        mt.name = STRDUP(&str[m->material]);

        array(char*) tokens = strsplit(&str[m->material], "+");
        for each_array(tokens, char*, it) {
            *out = texture(it, flags).id;

            if( *out == invalid ) {
                PRINTF("warn: material[%d] not found: %s\n", i, it);
            } else {
                PRINTF("loaded material[%d]: %s\n", i, it);

                mt.layer[mt.count++].texture = *out;

                ++out;
            }
        }

        // if no materials were loaded, try to signal a checkered placeholder
        if( out == textures ) {
            PRINTF("warn: using placeholder material[%d]=texture_checker\n", i);
            *out++ = invalid;
        }

        int count = (int)(intptr_t)(out - textures);
        model->num_textures += count;
        array_push(model->texture_names, STRDUP(&str[m->material]));

        array_push(model->materials, mt);
#endif

    }

    if( array_count(model->materials) == 0 ) {
        material_t mt = {0};
        mt.name = "placeholder";
        mt.layer[MATERIAL_CHANNEL_DIFFUSE].map.color = vec4(1,1,1,1);
        mt.layer[MATERIAL_CHANNEL_DIFFUSE].map.texture = CALLOC(1, sizeof(texture_t));
        mt.layer[MATERIAL_CHANNEL_DIFFUSE].map.texture->id = texture_checker().id;

        array_push(model->materials, mt);
    }

    return true;
}

static
void model_set_renderstates(model_t *m) {
    for (int i = 0; i<NUM_RENDER_PASSES; ++i) {
        m->rs[i] = renderstate();
    }

    // Opaque pass
    renderstate_t *opaque_rs = &m->rs[RENDER_PASS_OPAQUE];
    {
#if 1 // @todo: we should keep blend_enabled=0, however our transparency detection still needs work
        opaque_rs->blend_enabled = 0;
#else
        opaque_rs->blend_enabled = 1;
        opaque_rs->blend_src = GL_SRC_ALPHA;
        opaque_rs->blend_dst = GL_ONE_MINUS_SRC_ALPHA;
#endif
        opaque_rs->cull_face_mode = GL_BACK;
        opaque_rs->front_face = GL_CW;
    }

    // Transparent pass
    renderstate_t *transparent_rs = &m->rs[RENDER_PASS_TRANSPARENT];
    {
        transparent_rs->blend_enabled = 1;
        transparent_rs->blend_src = GL_SRC_ALPHA;
        transparent_rs->blend_dst = GL_ONE_MINUS_SRC_ALPHA;
        transparent_rs->cull_face_mode = GL_BACK;
        transparent_rs->front_face = GL_CW;
    }

    // Shadow pass
    renderstate_t *csm_shadow_rs = &m->rs[RENDER_PASS_SHADOW_CSM];
    {
        csm_shadow_rs->blend_enabled = 0;
        csm_shadow_rs->depth_test_enabled = true;
        csm_shadow_rs->depth_write_enabled = true;
        csm_shadow_rs->cull_face_enabled = 0;
        csm_shadow_rs->cull_face_mode = GL_BACK;
        csm_shadow_rs->front_face = GL_CW;
        csm_shadow_rs->depth_clamp_enabled = 1;
    }

    renderstate_t *vsm_shadow_rs = &m->rs[RENDER_PASS_SHADOW_VSM];
    {
        vsm_shadow_rs->blend_enabled = 0;
        vsm_shadow_rs->depth_test_enabled = true;
        vsm_shadow_rs->depth_write_enabled = true;
        vsm_shadow_rs->cull_face_enabled = 0;
        vsm_shadow_rs->cull_face_mode = GL_BACK;
        vsm_shadow_rs->front_face = GL_CW;
        vsm_shadow_rs->depth_clamp_enabled = 1;
    }
}

model_t model_from_mem(const void *mem, int len, int flags) {
    model_t m = {0};

    m.stored_flags = flags;
    m.shading = !(flags & MODEL_NO_PBR) ? SHADING_PBR : SHADING_PHONG;
    model_set_renderstates(&m);

    const char *ptr = (const char *)mem;
    iqm_t *q = CALLOC(1, sizeof(iqm_t));

    int error = 1;
    if( ptr && len ) {
        struct iqmheader hdr; memcpy(&hdr, ptr, sizeof(hdr)); ptr += sizeof(hdr);
        if( !memcmp(hdr.magic, IQM_MAGIC, sizeof(hdr.magic))) {
            lil32p(&hdr.version, (sizeof(hdr) - sizeof(hdr.magic))/sizeof(uint32_t));
            if(hdr.version == IQM_VERSION) {
                q->buf = CALLOC(hdr.filesize, sizeof(uint8_t));
                memcpy(q->buf + sizeof(hdr), ptr, hdr.filesize - sizeof(hdr));
                error = 0;
                if( hdr.num_meshes > 0 && !(flags & MODEL_NO_MESHES) )     error |= !model_load_meshes(q, &hdr, &m);
                if( hdr.num_meshes > 0 && !(flags & MODEL_NO_TEXTURES) )   error |= !model_load_textures(q, &hdr, &m, flags);
                else {
                    // setup fallback
                    material_t mt = {0};
                    mt.name = "placeholder";
                    mt.layer[0].map.color = vec4(1,1,1,1);
                    mt.layer[0].map.texture = CALLOC(1, sizeof(texture_t));
                    mt.layer[0].map.texture->id = texture_checker().id;

                    array_push(m.materials, mt);
                }
                if( hdr.num_anims  > 0 && !(flags & MODEL_NO_ANIMATIONS) ) error |= !model_load_anims(q, &hdr);
                if( q->buf != q->meshdata && q->buf != q->animdata ) FREE(q->buf);
            }
        }
    }

    if (flags & MODEL_PROCEDURAL) {
        error = 0;

        material_t mt = {0};
        mt.name = "base";
        mt.layer[0].map.color = vec4(1,1,1,1);
        mt.layer[0].map.texture = CALLOC(1, sizeof(texture_t));
        mt.layer[0].map.texture->id = texture_checker().id;

        q->nummeshes = 1;
        if (!q->textures) q->textures = CALLOC(1 * 8, sizeof(GLuint));
        if (!q->colormaps) q->colormaps = CALLOC(1 * 8, sizeof(vec4));

        q->textures[0] = mt.layer[0].map.texture->id;

        array_push(m.materials, mt);
    }

    if(error) {
        PRINTF("Error: cannot load %s", "model");
        FREE(q), q = 0;
    } else {
        m.vao = q->vao;
        m.ibo = q->ibo;
        m.vbo = q->vbo;
        m.num_verts = q->numverts;

        // m.boxes = bounds; // <@todo
        m.num_meshes = q->nummeshes;
        m.num_triangles = q->numtris;
        m.num_joints = q->numjoints;
        //m.num_poses = numposes;
        m.num_anims = q->numanims;
        m.num_frames = q->numframes;
        m.iqm = q;
        m.curframe = model_animate(m, 0);

        //m.num_textures = q->nummeshes; // assume 1 texture only per mesh
        m.textures = (q->textures);
        m.sky_env.id = 0;
        m.sky_refl.id = 0;

        m.flags = flags;

        id44(m.pivot);

        m.num_instances = 0;
        m.instanced_matrices = m.pivot;

        glGenBuffers(1, &m.vao_instanced);
        model_set_state(m);
        model_setstyle(&m, !(flags & MODEL_NO_PBR) ? SHADING_PBR : SHADING_PHONG);
    }
    return m;
}
model_t model(const char *filename, int flags) {
    if (!filename) {
        return model_from_mem( NULL, 0, flags|MODEL_PROCEDURAL );
    }
    int len;  // vfs_pushd(filedir(filename))
    char *ptr = vfs_load(filename, &len); // + vfs_popd
    return model_from_mem( ptr, len, flags );
}

uint32_t material_checksum(material_t *m) {
    uint32_t checksum = 0;
    for (int i = 0; i < MAX_CHANNELS_PER_MATERIAL; i++) {
        checksum ^= hh_float(m->layer[i].value);
        checksum ^= (m->layer[i].map.texture) ? m->layer[i].map.texture->id : 0;
        checksum ^= hh_vec4(m->layer[i].map.color);
    }
    return checksum;
}

uint32_t model_checksum(model_t *m) {
    uint32_t checksum = 0;
    
    // Basic properties
    checksum ^= m->num_meshes ^ m->num_triangles ^ m->num_joints ^ m->num_anims ^ m->num_frames;
    checksum ^= m->flags ^ (m->shadow_receiver << 1) ^ (m->billboard << 2);
    
    // VAO, IBO, VBO
    checksum ^= m->vao ^ m->ibo ^ m->vbo;
    
    // Textures
    for (int i = 0; i < m->num_textures; i++) {
        checksum ^= (uint32_t)(uintptr_t)m->textures[i];
    }
    
    // LOD
    checksum ^= (uint32_t)(uintptr_t)m->lod_collapse_map ^ m->lod_num_verts ^ m->lod_num_tris;
    
    // Renderstates
    checksum ^= renderstate_checksum(&m->rs[RENDER_PASS_OPAQUE]);
    checksum ^= renderstate_checksum(&m->rs[RENDER_PASS_TRANSPARENT]);
    checksum ^= renderstate_checksum(&m->rs[RENDER_PASS_CUSTOM]);
    
    // Materials
    for (int i = 0; i < array_count(m->materials); i++) {
        checksum ^= material_checksum(&m->materials[i]);
    }
    
    // Shader uniforms
    checksum ^= model_uniforms_checksum(array_count(m->uniforms), m->uniforms);
    
    return checksum;
}

bool model_get_bone_pose(model_t m, unsigned joint, mat34 *out) {
    if(!m.iqm) return false;
    iqm_t *q = m.iqm;

    if(joint >= q->numjoints) return false;

    multiply34x2(*out, q->outframe[joint], q->baseframe[joint]);
    return true;
}

bool model_get_bone_position(model_t m, unsigned joint, mat44 M, vec3 *out) {
    if(!m.iqm) return false;
    iqm_t *q = m.iqm;
    
    mat34 f;
    if (!model_get_bone_pose(m, joint, &f)) return false;
    vec3 pos = vec3(f[3],f[7],f[11]);

    pos = transform344(M, pos);
    *out = pos;

    return true;
}

anim_t clip(float minframe, float maxframe, float blendtime, unsigned flags) {
    return ((anim_t){minframe, maxframe, blendtime, flags, 1e6});
}
anim_t loop(float minframe, float maxframe, float blendtime, unsigned flags) {
    return clip(minframe, maxframe, blendtime, flags | ANIM_LOOP);
}

array(anim_t) animlist(const char *pathfile) {
    anim_t *animlist = 0;
    char *anim_file = vfs_read(strendi(pathfile,".txt") ? pathfile : va("%s@animlist.txt", pathfile));
    if( anim_file ) {
        // deserialize anim
        for each_substring(anim_file, "\r\n", anim) {
            int from, to;
            char anim_name[128] = {0};
            if( sscanf(anim, "%*s %d-%d %127[^\r\n]", &from, &to, anim_name) != 3) continue;
            array_push(animlist, !!strstri(anim_name, "loop") || !strcmpi(anim_name, "idle") ? loop(from, to, 0, 0) : clip(from, to, 0, 0)); // [from,to,flags]
            array_back(animlist)->name = strswap(strswap(strswap(STRDUP(anim_name), "Loop", ""), "loop", ""), "()", ""); // @leak
        }
    } else {
        // placeholder
        array_push(animlist, clip(0,1,0,0));
        array_back(animlist)->name = STRDUP("Error"); // @leak
    }
    return animlist;
}

static
void anim_tick(anim_t *p, bool is_primary, float delta) { // delta can be negative (reverses anim)
    if( !is_primary ) p->active = 0;
    if( is_primary && !p->active ) {
        p->active = 1;
        p->timer = 0;
        p->alpha = 0;
        if( p->flags & ANIM_DONT_RESET_AFTER_USE ) {} else p->curframe = 1e6;
    }

    p->alpha = 1 - ease(p->timer / p->blendtime, p->easing);
    p->timer += window_delta();

    p->curframe += delta;
    if(p->curframe < p->from || p->curframe > p->to ) p->curframe = delta >= 0 ? p->from : p->to;
    p->pose = pose(delta >= 0, p->curframe, p->from, p->to, p->flags & ANIM_LOOP, NULL);
}

float model_animate_blends(model_t m, anim_t *primary, anim_t *secondary, float delta) {
    if(!m.iqm) return -1;
    iqm_t *q = m.iqm;

    anim_tick(primary, 1, delta);
    anim_tick(secondary, 0, delta);

    float alpha = primary->alpha;
//  if( alpha <= 0 ) return model_animate(m, primary.pose.x);
//  if( alpha >= 1 ) return model_animate(m, secondary.pose.x);

    unsigned frame1 = primary->pose.x;
    unsigned frame2 = primary->pose.y;
    float    alphaA = primary->pose.z;

    unsigned frame3 = secondary->pose.x;
    unsigned frame4 = secondary->pose.y;
    float    alphaB = secondary->pose.z;

    mat34 *mat1 = &q->frames[frame1 * q->numjoints];
    mat34 *mat2 = &q->frames[frame2 * q->numjoints];
    mat34 *mat3 = &q->frames[frame3 * q->numjoints];
    mat34 *mat4 = &q->frames[frame4 * q->numjoints];

    for(int i = 0; i < q->numjoints; i++) {
        mat34 matA, matB, matF;
        lerp34(matA, mat1[i], mat2[i], alphaA);
        lerp34(matB, mat3[i], mat4[i], alphaB);
        lerp34(matF, matA, matB, alpha );
        if(q->joints[i].parent >= 0) multiply34x2(q->outframe[i], q->outframe[q->joints[i].parent], matF);
        else copy34(q->outframe[i], matF);
    }

    return frame1 + alpha;
}

vec3 pose(bool forward_time, float curframe, int minframe, int maxframe, bool loop, float *retframe) {
    float offset = curframe - (int)curframe;
#if 1
    int frame1 = (int)curframe;
    int frame2 = frame1 + (forward_time ? 1 : -1);
#else
    float frame1 = curframe;
    float frame2 = curframe + (forward_time ? 1 : -1);
#endif

    if( loop ) {
        int distance = maxframe - minframe;
        frame1 = fmod(frame1 - minframe, distance) + minframe; // frame1 >= maxframe ? minframe : frame1 < minframe ? maxframe - clampf(minframe - frame1, 0, distance) : frame1;
        frame2 = fmod(frame2 - minframe, distance) + minframe; // frame2 >= maxframe ? minframe : frame2 < minframe ? maxframe - clampf(minframe - frame2, 0, distance) : frame2;
        if(retframe) *retframe = fmod(frame1 + offset - minframe, distance) + minframe;
    } else {
        frame1 = clampf(frame1, minframe, maxframe);
        frame2 = clampf(frame2, minframe, maxframe);
        if(retframe) *retframe = clampf(frame1 + offset, minframe, maxframe);
    }

    return vec3(frame1 + (offset > 0 && offset < 1 ? offset : 0),frame2,offset);
}

float model_animate_clip(model_t m, float curframe, int minframe, int maxframe, bool loop) {
    if(!m.iqm) return -1;
    iqm_t *q = m.iqm;

    float retframe = -1;
    if( q->numframes > 0 ) {
        vec3 p = pose(curframe >= m.curframe, curframe, minframe, maxframe, loop, &retframe);
        int frame1 = p.x;
        int frame2 = p.y;
        float offset = p.z;

        mat34 *mat1 = &q->frames[frame1 * q->numjoints];
        mat34 *mat2 = &q->frames[frame2 * q->numjoints];

        // @todo: add animation blending and inter-frame blending here
        for(int i = 0; i < q->numjoints; i++) {
            mat34 mat; lerp34(mat, mat1[i], mat2[i], offset);
            if(q->joints[i].parent >= 0) multiply34x2(q->outframe[i], q->outframe[q->joints[i].parent], mat);
            else copy34(q->outframe[i], mat);
        }
    }

    return retframe;
}

void model_render_skeleton(model_t m, mat44 M) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    if(!q->numjoints) return;

    ddraw_ontop_push(true);
    ddraw_color_push(RED);

    for( int joint = 0; joint < q->numjoints; joint++ ) {
        if( q->joints[joint].parent < 0) continue;

        // bone space...
        mat34 f;
        model_get_bone_pose(m, joint, &f);
        vec3 pos = vec3(f[3],f[7],f[11]);

        model_get_bone_pose(m, q->joints[joint].parent, &f);
        vec3 src = vec3(f[3],f[7],f[11]);

        // ...to model space
        src = transform344(M, src);
        pos = transform344(M, pos);

        // red line
        ddraw_color(RED);
//      ddraw_line(src, pos);
        ddraw_bone(src, pos);

        // green dot
        ddraw_color(GREEN);
        ddraw_point(pos);

        // yellow text
        ddraw_color(YELLOW);
        ddraw_text(pos, 0.005, va("%d", joint));
    }

    ddraw_color_pop();
    ddraw_ontop_pop();
}

float model_animate(model_t m, float curframe) {
    if(!m.iqm) return -1;
    iqm_t *q = m.iqm;
    return model_animate_clip(m, curframe, 0, q->numframes-1, true);
}

static inline
void shader_colormap_model_internal(model_t *m,const char *col_name, const char *bool_name, const char *tex_name, colormap_t c, int slot ) {
    // assumes shader uses `struct { vec4 color; bool has_tex } name + sampler2D name_tex;`
    shader_vec4( col_name, c.color );
    shader_bool( bool_name, c.texture != NULL && (c.texture && c.texture->id != texture_checker().id) );
    if( c.texture ) shader_texture_unit_kind_(GL_TEXTURE_2D, shader_uniform(tex_name), c.texture->id, slot);
    else {
        glUniform1i(shader_uniform(tex_name), slot);
    }
}


typedef struct drawcall_t {
    int mesh;
    union {
        uint64_t order;
        struct {
            uint32_t tex;
            float distance;
        };
    };
} drawcall_t;

static
int drawcall_compare(const void *a, const void *b) {
    const drawcall_t *da = a, *db = b;
    return da->order < db->order ? 1 : da->order > db->order ? -1 : 0;
}

bool model_has_transparency_mesh(model_t m, int mesh) {
    if(!m.iqm) return false;
    iqm_t *q = m.iqm;

    if (m.flags & MODEL_TRANSPARENT) {
        return true;
    }
    if (m.materials[mesh].layer[MATERIAL_CHANNEL_DIFFUSE].map.color.a < 1 || (m.materials[mesh].layer[MATERIAL_CHANNEL_DIFFUSE].map.texture && m.materials[mesh].layer[MATERIAL_CHANNEL_DIFFUSE].map.texture->transparent)) {
        return true;
    }
    if (m.shading == SHADING_PBR && (m.materials[mesh].layer[MATERIAL_CHANNEL_ALBEDO].map.color.a < 1 || (m.materials[mesh].layer[MATERIAL_CHANNEL_ALBEDO].map.texture && m.materials[mesh].layer[MATERIAL_CHANNEL_ALBEDO].map.texture->transparent))){
        return true;
    }

    return false;
}

bool model_has_transparency(model_t m) {
    if(!m.iqm) return false;
    iqm_t *q = m.iqm;

    for (int i = 0; i < q->nummeshes; i++) {
        if (model_has_transparency_mesh(m, i)) {
            return true;
        }
    }

    return false;
}


void model_set_frustum(model_t *m, frustum f) {
    m->frustum_enabled = 1;
    m->frustum_state = f;
}

void model_clear_frustum(model_t *m) {
    m->frustum_enabled = 0;
}

static frustum global_frustum;
static mat44 global_frustum_stored_mat_proj;
static mat44 global_frustum_stored_mat_view;

static inline
bool model_is_visible(model_t m, mat44 model_mat, mat44 proj, mat44 view) {
    if(!m.iqm) return false;

    bool is_enabled = m.frustum_enabled;
    frustum fr = m.frustum_state;

#if GLOBAL_FRUSTUM_ENABLED
    if (active_shadowmap) {
        is_enabled = true;
        fr = active_shadowmap->shadow_frustum;
    }

    if (!is_enabled) { /* custom frustum not provided, let's compute one from input call */
        if (memcmp(global_frustum_stored_mat_proj, proj, sizeof(mat44)) != 0 ||
            memcmp(global_frustum_stored_mat_view, view, sizeof(mat44)) != 0) {
            copy44(global_frustum_stored_mat_proj, proj);
            copy44(global_frustum_stored_mat_view, view);
            mat44 proj_modified;
            copy44(proj_modified, proj);
            
            // Increase FOV by GLOBAL_FRUSTUM_FOV_MULTIPLIER
            float fov_scale = 1.0f / GLOBAL_FRUSTUM_FOV_MULTIPLIER;
            proj_modified[0] *= fov_scale;
            proj_modified[5] *= fov_scale;
            mat44 projview; multiply44x2(projview, proj_modified, view);
            global_frustum = frustum_build(projview);
        }
        fr = global_frustum;
        is_enabled = true;
    }
#endif

    if(!is_enabled) return true;

    sphere s = model_bsphere(m, model_mat);

    if (!frustum_test_sphere(fr, s)) {
        return false;
    }

    aabb box = model_aabb(m, model_mat);

    if (!frustum_test_aabb(fr, box)) {
        return false;
    }

    return true;
}

static inline
void model_set_mesh_material(model_t m, int mesh, int shader, int rs_idx) {
    iqm_t *q = m.iqm;
    int loc = -1;
    int i = mesh;

    if (m.shading != SHADING_PBR) {
        int loc = glGetUniformLocation(shader, "u_texture2d");
        shader_texture_unit_kind_(GL_TEXTURE_2D, loc, q->textures[i], MODEL_TEXTURE_DIFFUSE);

        if ((loc = glGetUniformLocation(shader, "u_textured")) >= 0) {
            bool textured = !!q->textures[i] && q->textures[i] != texture_checker().id; // m.materials[i].layer[0].texture != texture_checker().id;
            glUniform1i(loc, textured ? GL_TRUE : GL_FALSE);
            if ((loc = glGetUniformLocation(shader, "u_diffuse")) >= 0) {
                glUniform4f(loc, m.materials[i].layer[0].map.color.r, m.materials[i].layer[0].map.color.g, m.materials[i].layer[0].map.color.b, m.materials[i].layer[0].map.color.a);
            }
        }

    } else {
        const material_t *material = &m.materials[i];
        shader_colormap_model_internal(&m, "map_diffuse.color", "map_diffuse.has_tex", "map_diffuse_tex", material->layer[MATERIAL_CHANNEL_DIFFUSE].map, MODEL_TEXTURE_DIFFUSE);
        shader_colormap_model_internal(&m, "map_albedo.color", "map_albedo.has_tex", "map_albedo_tex", material->layer[MATERIAL_CHANNEL_ALBEDO].map, MODEL_TEXTURE_ALBEDO);
        if (rs_idx < RENDER_PASS_SHADOW_BEGIN || rs_idx > RENDER_PASS_SHADOW_END) {
            shader_colormap_model_internal(&m, "map_normals.color", "map_normals.has_tex", "map_normals_tex", material->layer[MATERIAL_CHANNEL_NORMALS].map, MODEL_TEXTURE_NORMALS);
            shader_colormap_model_internal(&m, "map_specular.color", "map_specular.has_tex", "map_specular_tex", material->layer[MATERIAL_CHANNEL_SPECULAR].map, MODEL_TEXTURE_SPECULAR);
            shader_colormap_model_internal(&m, "map_roughness.color", "map_roughness.has_tex", "map_roughness_tex", material->layer[MATERIAL_CHANNEL_ROUGHNESS].map, MODEL_TEXTURE_ROUGHNESS);
            shader_colormap_model_internal(&m, "map_metallic.color", "map_metallic.has_tex", "map_metallic_tex", material->layer[MATERIAL_CHANNEL_METALLIC].map, MODEL_TEXTURE_METALLIC);
            shader_colormap_model_internal(&m, "map_ao.color", "map_ao.has_tex", "map_ao_tex", material->layer[MATERIAL_CHANNEL_AO].map, MODEL_TEXTURE_AO);
            shader_colormap_model_internal(&m, "map_ambient.color", "map_ambient.has_tex", "map_ambient_tex", material->layer[MATERIAL_CHANNEL_AMBIENT].map, MODEL_TEXTURE_AMBIENT);
            shader_colormap_model_internal(&m, "map_emissive.color", "map_emissive.has_tex", "map_emissive_tex", material->layer[MATERIAL_CHANNEL_EMISSIVE].map, MODEL_TEXTURE_EMISSIVE);
            shader_colormap_model_internal(&m, "map_parallax.color", "map_parallax.has_tex", "map_parallax_tex", material->layer[MATERIAL_CHANNEL_PARALLAX].map, MODEL_TEXTURE_PARALLAX);
            shader_float("parallax_scale", material->layer[MATERIAL_CHANNEL_PARALLAX].value);
            shader_float("parallax_shadow_power", material->layer[MATERIAL_CHANNEL_PARALLAX].value2);
        }
    }
}

static
void model_draw_call(model_t m, int shader, int pass, vec3 cam_pos, mat44 model_mat, mat44 proj, mat44 view) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    handle old_shader = last_shader;
    shader_bind(shader);

    int rs_idx = model_getpass();
    renderstate_t *rs = &m.rs[rs_idx];
    renderstate_apply(rs);

    glBindVertexArray( q->vao );

    static array(int) required_rs = 0;
    array_resize(required_rs, q->nummeshes);

    for(int i = 0; i < q->nummeshes; i++) {
        struct iqmmesh *im = &q->meshes[i];
        required_rs[i] = rs_idx;

        if (required_rs[i] < RENDER_PASS_OVERRIDES_BEGIN) {
            if (model_has_transparency_mesh(m, i)) {
                required_rs[i] = RENDER_PASS_TRANSPARENT;
            }
        }
    }

    static array(drawcall_t) drawcalls = 0;
    array_resize(drawcalls, 0);

    if (rs_idx > RENDER_PASS_OVERRIDES_BEGIN) {
        for(int i = 0; i < q->nummeshes; i++) {
            array_push(drawcalls, (drawcall_t){i, -1});
        }
    } else {
        if(pass == -1 || pass == RENDER_PASS_OPAQUE) {
            for(int i = 0; i < q->nummeshes; i++) {
                // collect opaque drawcalls
                if (required_rs[i] == RENDER_PASS_OPAQUE) {
                    drawcall_t call;
                    call.mesh = i;
                    call.tex = m.textures[i];
                    call.distance = -1;
                    if (m.shading == SHADING_PBR)
                        call.tex = m.materials[i].layer[MATERIAL_CHANNEL_ALBEDO].map.texture ? m.materials[i].layer[MATERIAL_CHANNEL_ALBEDO].map.texture->id : m.materials[i].layer[MATERIAL_CHANNEL_DIFFUSE].map.texture ? m.materials[i].layer[MATERIAL_CHANNEL_DIFFUSE].map.texture->id : texture_checker().id;
                    array_push(drawcalls, call);
                }
            }
        }
        
        if(pass == -1 || pass == RENDER_PASS_TRANSPARENT) {
            for(int i = 0; i < q->nummeshes; i++) {
                // collect transparent drawcalls
                if (required_rs[i] == RENDER_PASS_TRANSPARENT) {
                    drawcall_t call;
                    call.mesh = i;
                    call.tex = m.textures[i];
                    struct iqmmesh *im = &q->meshes[i];
                    
                    // calculate distance from camera
                    {
                        vec3 pos = ptr3(((struct iqm_vertex*)m.verts)[im->first_vertex].position);
                        call.distance = len3sq(sub3(cam_pos, transform344(model_mat, pos)));
                    }

                    if (m.shading == SHADING_PBR)
                        call.tex = m.materials[i].layer[MATERIAL_CHANNEL_ALBEDO].map.texture ? m.materials[i].layer[MATERIAL_CHANNEL_ALBEDO].map.texture->id : m.materials[i].layer[MATERIAL_CHANNEL_DIFFUSE].map.texture ? m.materials[i].layer[MATERIAL_CHANNEL_DIFFUSE].map.texture->id : texture_checker().id;
                    array_push(drawcalls, call);
                }
            }
        }
    }

    // sort drawcalls by order
    array_sort(drawcalls, drawcall_compare);

    struct iqmtriangle *tris = NULL;
    for(int di = 0; di < array_count(drawcalls); di++) {
        int i = drawcalls[di].mesh;
        struct iqmmesh *im = &q->meshes[i];

        if (pass != -1 && pass != required_rs[i]) continue;
        
        if (rs_idx != required_rs[i]) {
            rs_idx = required_rs[i];
            rs = &m.rs[rs_idx];
            renderstate_apply(rs);
        }
        
        model_set_mesh_material(m, i, shader, rs_idx);

        glDrawElementsInstanced(GL_TRIANGLES, 3*im->num_triangles, GL_UNSIGNED_INT, &tris[im->first_triangle], m.num_instances);
        profile_incstat("Render.num_drawcalls", +1);
        profile_incstat("Render.num_triangles", +im->num_triangles*m.num_instances);
    }

    glBindVertexArray( 0 );

    shader_bind(old_shader);
}

static mat44 *pass_model_matrices = NULL;

void model_render_instanced_pass(model_t mdl, mat44 proj, mat44 view, mat44* models, unsigned count, int pass) {
    if(!mdl.iqm) return;
    iqm_t *q = mdl.iqm;

    if (q->vbo == 0 || q->ibo == 0) {
        return;
    }

    if (active_shadowmap && active_shadowmap->skip_render) {
        return;
    }

    pass_model_matrices = array_resize(pass_model_matrices, count); //@leak
    memcpy(pass_model_matrices, models, count * sizeof(mat44));

    for (int i = 0; i < count; i++) {
        bool visible = model_is_visible(mdl, pass_model_matrices[i], proj, view);
        if (!visible) {
            array_erase_fast(pass_model_matrices, i);
            i--;
            count--;
        }
    }

    if (count == 0) {
        return;
    }

    mdl.iqm->texture_unit = 0;

    mat44 mv; multiply44x2(mv, view, pass_model_matrices[0]);

    if( count != mdl.num_instances ) {
        mdl.num_instances = count;
        mdl.instanced_matrices = (float*)pass_model_matrices;
        model_set_state(mdl);
    }

    int shader = mdl.iqm->program;
    if (model_getpass() > RENDER_PASS_SHADOW_BEGIN && model_getpass() < RENDER_PASS_SHADOW_END) {
        shader = mdl.iqm->shadow_program;
        model_set_uniforms(mdl, shader, mv, proj, view, pass_model_matrices[0]);
        model_draw_call(mdl, shader, pass, pos44(view), pass_model_matrices[0], proj, view);
        return;
    }

    shader_bind(shader);

    if (mdl.lights.count > 0) {
        light_update(&q->light_ubo, mdl.lights.count, mdl.lights.base);
    }

    model_set_uniforms(mdl, shader, mv, proj, view, pass_model_matrices[0]);
    model_draw_call(mdl, shader, pass, pos44(view), pass_model_matrices[0], proj, view);
    
    if (q->light_ubo) {
        ubo_unbind(q->light_ubo);
    }
}

void model_render_instanced(model_t m, mat44 proj, mat44 view, mat44* models, unsigned count) {
    model_render_instanced_pass(m, proj, view, models, count, -1);
}

void model_render_pass(model_t m, mat44 proj, mat44 view, mat44 model, int pass) {
    model_render_instanced_pass(m, proj, view, (mat44*)model, 1, pass);
}

void model_render(model_t m, mat44 proj, mat44 view, mat44 model) {
    model_render_pass(m, proj, view, model, -1);
}

void model_setshader(model_t *m, int shading, const char *vs, const char *fs, const char *defines) {
    if (!m->iqm) return;
    iqm_t *q = m->iqm;

    m->shading = shading;
    int flags = m->stored_flags;

    // load pbr material if SHADING_PBR was selected
    if (shading == SHADING_PBR) {
        for (int i = 0; i < array_count(m->materials); ++i) {
            model_load_pbr(m, &m->materials[i], i);
        }
    }

    if (!vs) {
        vs = vfs_read("shaders/vs_323444143_16_3322_model.glsl");
    }

    if (!fs) {
        fs = vfs_read("shaders/fs_32_4_model.glsl");
    }

    /* needs to match SHADING_MODE */
    const char *shading_defines[] = {
        "SHADING_NONE",
        "SHADING_PHONG",
        "SHADING_VERTEXLIT",
        "SHADING_PBR",
    };

    ASSERT(shading < countof(shading_defines));
    const char *shading_define = shading_defines[shading];

    // rebind shader
    glUseProgram(0);
    if (q->program) {
        glDeleteProgram(q->program);
    }
    
    {
        int shaderprog = shader(vs, fs,
            "att_position,att_texcoord,att_normal,att_tangent,att_instanced_matrix,,,,att_indexes,att_weights,att_vertexindex,att_color,att_bitangent,att_texcoord2","fragColor",
            va("%s,%s,%s", (flags & MODEL_RIMLIGHT) ? "RIM" : "NORIM", defines ? defines : "NO_CUSTOM_DEFINES", shading_define));
        q->program = shaderprog;
    }

    {
        int shaderprog = shader(vs, vfs_read("shaders/fs_shadow_pass.glsl"),
            "att_position,att_texcoord,att_normal,att_tangent,att_instanced_matrix,,,,att_indexes,att_weights,att_vertexindex,att_color,att_bitangent,att_texcoord2","fragcolor",
            va("SHADOW_CAST,%s", defines ? defines : "NO_CUSTOM_DEFINES"));
        q->shadow_program = shaderprog;
    }
    model_init_uniforms(q, 0, q->program);
    model_init_uniforms(q, 1, q->shadow_program);

    unsigned block_index = glGetUniformBlockIndex(q->program, "LightBuffer");
    glUniformBlockBinding(q->program, block_index, 0);
}

void model_setstyle(model_t *m, int shading) {
    model_setshader(m, shading, NULL, NULL, NULL);
}

void model_skybox(model_t *mdl, skybox_t sky) {
    model_cubemap(mdl, &sky.cubemap);
    mdl->sky_cubemap.id = sky.cubemap.id;
    mdl->sky_refl = sky.refl;
    mdl->sky_env = sky.env;
}

void model_cubemap(model_t *mdl, cubemap_t *c) {
    static char coef_names[9][128] = {0};
    do_once {
        for (int i = 0; i < 9; i++) {
            sprintf(coef_names[i], "u_coefficients_sh[%d]", i);
        }
    }
    for (int i = 0; i < 9; i++) {
        model_adduniform(mdl, model_uniform(coef_names[i], UNIFORM_VEC3, .v3 = c ? c->sh[i] : vec3(0,0,0)));
    }
}

void model_probe(model_t *mdl, vec3 center, float radius, unsigned count, cubemap_t *c) {
    vec3 sh[9] = {0};
    cubemap_sh_blend(center, radius, count, c, sh);
    cubemap_t sh_cubemap = {0};
    memcpy(sh_cubemap.sh, sh, sizeof(sh));
    model_cubemap(mdl, &sh_cubemap);
}

void model_shadow(model_t *mdl, shadowmap_t *sm) {
    if (sm) {
        mdl->shadow_receiver = true;
        mdl->shadow_map = sm;
    } else {
        mdl->shadow_receiver = false;
        mdl->shadow_map = NULL;
    }
}

void model_light(model_t *mdl, unsigned count, light_t *lights) {
    mdl->lights.base = lights;
    mdl->lights.count = count;
}

void model_rimlight(model_t *mdl, vec3 rim_range, vec3 rim_color, vec3 rim_pivot, bool rim_ambient) {
    model_adduniform(mdl, model_uniform("u_rimrange", UNIFORM_VEC3, .v3 = rim_range));
    model_adduniform(mdl, model_uniform("u_rimcolor", UNIFORM_VEC3, .v3 = rim_color));
    model_adduniform(mdl, model_uniform("u_rimpivot", UNIFORM_VEC3, .v3 = rim_pivot));
    model_adduniform(mdl, model_uniform("u_rimambient", UNIFORM_BOOL, .i = rim_ambient));
}

void model_fog(model_t *mdl, unsigned mode, vec3 color, float start, float end, float density) {
    model_adduniform(mdl, model_uniform("u_fog_color", UNIFORM_VEC3, .v3 = color));
    model_adduniform(mdl, model_uniform("u_fog_density", UNIFORM_FLOAT, .f = density));
    model_adduniform(mdl, model_uniform("u_fog_start", UNIFORM_FLOAT, .f = start));
    model_adduniform(mdl, model_uniform("u_fog_end", UNIFORM_FLOAT, .f = end));
    model_adduniform(mdl, model_uniform("u_fog_type", UNIFORM_INT, .i = mode));
}

void model_applyuniform(model_t m, const model_uniform_t *t) {
    if (!m.iqm) return;
    iqm_t *q = m.iqm;

    int oldprog = last_shader;
    shader_bind(q->program);

    switch (t->kind) {
    case UNIFORM_FLOAT:
        glUniform1f(shader_uniform(t->name), t->f);
        break;
    case UNIFORM_INT:
    case UNIFORM_BOOL:
        glUniform1i(shader_uniform(t->name), t->i);
    case UNIFORM_UINT:
        glUniform1ui(shader_uniform(t->name), t->u);
        break;
    case UNIFORM_VEC2:
        glUniform2fv(shader_uniform(t->name), 1, &t->v2.x);
        break;
    case UNIFORM_VEC3:
        glUniform3fv(shader_uniform(t->name), 1, &t->v3.x);
        break;
    case UNIFORM_VEC4:
        glUniform4fv(shader_uniform(t->name), 1, &t->v4.x);
        break;
    case UNIFORM_MAT3:
        glUniformMatrix3fv(shader_uniform(t->name), 1, GL_FALSE, t->m33);
        break;
    case UNIFORM_MAT4:
        glUniformMatrix4fv(shader_uniform(t->name), 1, GL_FALSE, t->m44);
        break;
    case UNIFORM_SAMPLER2D:
        shader_texture_unit_kind_(GL_TEXTURE_2D, shader_uniform(t->name), t->u, model_texture_unit(&m));
        break;
    case UNIFORM_SAMPLER3D:
        shader_texture_unit_kind_(GL_TEXTURE_3D, shader_uniform(t->name), t->u, model_texture_unit(&m));
        break;
    case UNIFORM_SAMPLERCUBE:
        shader_texture_unit_kind_(GL_TEXTURE_CUBE_MAP, shader_uniform(t->name), t->u, model_texture_unit(&m));
        break;
    }

    shader_bind(oldprog);
}

void model_adduniform(model_t* m, model_uniform_t uniform) {
    for (unsigned i = 0; i < array_count(m->uniforms); i++) {
        if (strcmp(m->uniforms[i].name, uniform.name) == 0) {
            m->uniforms[i] = uniform;
            return;
        }
    }
    array_push(m->uniforms, uniform);
}

void model_adduniforms(model_t* m, unsigned count, model_uniform_t* uniforms) {
    for (unsigned i = 0; i < count; i++) {
        model_adduniform(m, uniforms[i]);
    }
}

// static
aabb aabb_transform( aabb A, mat44 M ) {
    // Based on "Transforming Axis-Aligned Bounding Boxes" by Jim Arvo, 1990
    aabb B = { {M[12],M[13],M[14]}, {M[12],M[13],M[14]} }; // extract translation from mat44
    for( int i = 0; i < 3; i++ )
    for( int j = 0; j < 3; j++ ) {
        float a = M[i*4+j] * j[&A.min.x]; // use mat33 from mat44
        float b = M[i*4+j] * j[&A.max.x]; // use mat33 from mat44
        if( a < b ) {
            i[&B.min.x] += a;
            i[&B.max.x] += b;
        } else {
            i[&B.min.x] += b;
            i[&B.max.x] += a;
        }
    }
    return B;
}

aabb model_aabb(model_t m, mat44 transform) {
    iqm_t *q = m.iqm;
    if( q && q->bounds ) {
    int f = ( (int)m.curframe ) % (q->numframes + !q->numframes);
    vec3 bbmin = ptr3(q->bounds[f].bbmin);
    vec3 bbmax = ptr3(q->bounds[f].bbmax);
    return aabb_transform(aabb(bbmin,bbmax), transform);
    }
    return aabb(vec3(0,0,0),vec3(0,0,0));
}

sphere model_bsphere(model_t m, mat44 transform) {
    iqm_t *q = m.iqm;
    if( q && q->bounds ) {
    int f = ( (int)m.curframe ) % (q->numframes + !q->numframes);
    vec3 bbmin = ptr3(q->bounds[f].bbmin);
    vec3 bbmax = ptr3(q->bounds[f].bbmax);
    aabb box = aabb_transform(aabb(bbmin,bbmax), transform);
    return sphere(scale3(add3(box.min, box.max), 0.5f), 0.5f * (len3(sub3(box.min, box.max))));
    }
    return sphere(vec3(0,0,0), 0);
}

static inline int MapReduce(array(int) collapse_map, int n, int mx) {
    while( n >= mx ) n = collapse_map[n];
    return n;
}

API void ProgressiveMesh(int vert_n, int vert_stride, const float *v, int tri_n, const int *tri, int *map, int *permutation);

static inline
void MorphVertex(struct iqm_vertex *v, struct iqm_vertex *v0, struct iqm_vertex *v1, float t) {
    v->position[0] = mixf(v0->position[0], v1->position[0], t);
    v->position[1] = mixf(v0->position[1], v1->position[1], t);
    v->position[2] = mixf(v0->position[2], v1->position[2], t);

    v->normal[0] = mixf(v0->normal[0], v1->normal[0], t);
    v->normal[1] = mixf(v0->normal[1], v1->normal[1], t);
    v->normal[2] = mixf(v0->normal[2], v1->normal[2], t);

    v->tangent[0] = mixf(v0->tangent[0], v1->tangent[0], t);
    v->tangent[1] = mixf(v0->tangent[1], v1->tangent[1], t);
    v->tangent[2] = mixf(v0->tangent[2], v1->tangent[2], t);

    v->texcoord[0] = mixf(v0->texcoord[0], v1->texcoord[0], t);
    v->texcoord[1] = mixf(v0->texcoord[1], v1->texcoord[1], t);
}

void model_lod(model_t *mdl, float lo_detail, float hi_detail, float morph) {
    assert(mdl->num_meshes == 1);
    if (array_count(mdl->lod_collapse_map) == 0) {
        array(int) permutation = 0;
            array(float) positions = 0;
            array_resize(mdl->lod_collapse_map, mdl->num_verts);
            array_resize(permutation, mdl->num_verts);
            array_resize(positions, mdl->num_verts*3);
            for (int i = 0; i < mdl->num_verts; i++) {
                struct iqm_vertex *v = (struct iqm_vertex *)((char *)mdl->verts + i*mdl->stride);
                positions[i*3 + 0] = v->position[0];
                positions[i*3 + 1] = v->position[1];
                positions[i*3 + 2] = v->position[2];
            }
            ProgressiveMesh(mdl->num_verts, sizeof(float)*3, (const float *)positions, mdl->num_tris, (const int *)mdl->tris, mdl->lod_collapse_map, permutation);
            array_free(positions);
            // PermuteVertices {
                ASSERT(array_count(permutation) == mdl->num_verts);
                // rearrange the vertex Array
                char *tmp = REALLOC(0, mdl->stride*mdl->num_verts);
                char *verts = (char *)mdl->verts;
                memcpy(tmp, verts, mdl->stride*mdl->num_verts);

                for(int i = 0; i < mdl->num_verts; i++) {
                    int index = permutation[i];
                    int src_offset = i * mdl->stride;
                    int offset = index * mdl->stride;

                    memcpy(verts + offset, tmp + src_offset, mdl->stride);
                }
                int *tris = (int *)mdl->tris;
                // update the changes in the entries in the triangle Array
                for (int i = 0; i < mdl->num_tris; i++) {
                    tris[i*3 + 0] = permutation[tris[i*3 + 0]];
                    tris[i*3 + 1] = permutation[tris[i*3 + 1]];
                    tris[i*3 + 2] = permutation[tris[i*3 + 2]];
                }

                // upload modified data
                glBindVertexArray(mdl->vao);
                glBindBuffer(GL_ARRAY_BUFFER, mdl->vbo);
                glBufferData(GL_ARRAY_BUFFER, mdl->num_verts*mdl->stride, mdl->verts, GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mdl->ibo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, mdl->num_tris*3*sizeof(int), mdl->tris, GL_STATIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                glBindVertexArray(0);

                FREE(tmp);
            // } PermuteVertices
        array_free(permutation);
    }
    
    ASSERT(array_count(mdl->lod_collapse_map));

    int max_verts_to_render = hi_detail * mdl->num_verts;
    int min_verts_to_render = lo_detail * mdl->num_verts;

    if( max_verts_to_render <= 0 || min_verts_to_render <= 0 )
        return;

    FREE(mdl->lod_verts);
    FREE(mdl->lod_tris);

    char *verts = (char *)mdl->verts;
    int *tris = (int *)mdl->tris;
    int max_lod_tris = 0;

    //@fixme: optimise
    for( unsigned int i = 0; i < mdl->num_tris; i++ ) {
        int p0 = MapReduce(mdl->lod_collapse_map, tris[i*3 + 0], max_verts_to_render);
        int p1 = MapReduce(mdl->lod_collapse_map, tris[i*3 + 1], max_verts_to_render);
        int p2 = MapReduce(mdl->lod_collapse_map, tris[i*3 + 2], max_verts_to_render);
        if(p0==p1 || p0==p2 || p1==p2) continue;
        ++max_lod_tris;
    }

    mdl->lod_verts = REALLOC(0, max_lod_tris*3*mdl->stride);
    mdl->lod_tris = REALLOC(0, max_lod_tris*3*sizeof(int));
    mdl->lod_num_verts = 0;
    mdl->lod_num_tris = 0;

    struct iqm_vertex *lod_verts = (struct iqm_vertex *)mdl->lod_verts;
    int *lod_tris = (int *)mdl->lod_tris;

    for( int i = 0; i < mdl->num_tris; i++ ) {
        int p0 = MapReduce(mdl->lod_collapse_map, tris[i*3 + 0], max_verts_to_render);
        int p1 = MapReduce(mdl->lod_collapse_map, tris[i*3 + 1], max_verts_to_render);
        int p2 = MapReduce(mdl->lod_collapse_map, tris[i*3 + 2], max_verts_to_render);
        if(p0==p1 || p0==p2 || p1==p2) continue;

        int q0 = MapReduce(mdl->lod_collapse_map, p0, min_verts_to_render);
        int q1 = MapReduce(mdl->lod_collapse_map, p1, min_verts_to_render);
        int q2 = MapReduce(mdl->lod_collapse_map, p2, min_verts_to_render);
        // if(q0==q1 || q0==q2 || q1==q2) continue;

        struct iqm_vertex v0 = *(struct iqm_vertex *)(verts + (p0*mdl->stride));
        struct iqm_vertex v1 = *(struct iqm_vertex *)(verts + (p1*mdl->stride));
        struct iqm_vertex v2 = *(struct iqm_vertex *)(verts + (p2*mdl->stride));
        
        struct iqm_vertex u0 = *(struct iqm_vertex *)(verts + (q0*mdl->stride));
        struct iqm_vertex u1 = *(struct iqm_vertex *)(verts + (q1*mdl->stride));
        struct iqm_vertex u2 = *(struct iqm_vertex *)(verts + (q2*mdl->stride));
            
        struct iqm_vertex f0=v0,f1=v1,f2=v2;

        if (morph == 0.0f) {
            f0=u0,f1=u1,f2=u2;
        }
        else if (morph < 1.0f) {
            MorphVertex(&f0, &v0, &u0, 1.0f - morph);
            MorphVertex(&f1, &v1, &u1, 1.0f - morph);
            MorphVertex(&f2, &v2, &u2, 1.0f - morph);
        }

        lod_verts[mdl->lod_num_verts + 0] = f0;
        lod_verts[mdl->lod_num_verts + 1] = f1;
        lod_verts[mdl->lod_num_verts + 2] = f2;

        int idx = mdl->lod_num_verts;
        lod_tris[mdl->lod_num_tris*3 + 0] = idx+0;
        lod_tris[mdl->lod_num_tris*3 + 1] = idx+1;
        lod_tris[mdl->lod_num_tris*3 + 2] = idx+2;
        mdl->lod_num_verts += 3;
        ++mdl->lod_num_tris;
    }

    // upload modified data
    glBindVertexArray(mdl->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mdl->vbo);
    glBufferData(GL_ARRAY_BUFFER, mdl->lod_num_verts*mdl->stride, mdl->lod_verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mdl->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mdl->lod_num_tris*3*sizeof(int), mdl->lod_tris, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void model_destroy(model_t m) {
    FREE(m.verts);
    for( int i = 0, end = array_count(m.texture_names); i < end; ++i ) {
        FREE(m.texture_names[i]);
    }
    array_free(m.texture_names);

    if (m.iqm->light_ubo) {
        ubo_destroy(m.iqm->light_ubo);
    }

    iqm_t *q = m.iqm;
//    if(m.mesh) mesh_destroy(m.mesh);
    FREE(q->outframe);
    FREE(q->colormaps);
    FREE(q->textures);
    FREE(q->baseframe);
    FREE(q->inversebaseframe);
    if(q->animdata != q->meshdata) FREE(q->animdata);
    //FREE(q->meshdata);
    FREE(q->frames);
    FREE(q->buf);
    FREE(q);
    array_free(m.uniforms);
}

static unsigned model_renderpass = RENDER_PASS_OPAQUE;

unsigned model_getpass() {
    return model_renderpass;
}

unsigned model_setpass(unsigned pass) {
    ASSERT(pass < NUM_RENDER_PASSES);
    ASSERT(pass != RENDER_PASS_OVERRIDES_BEGIN && pass != RENDER_PASS_OVERRIDES_END);
    unsigned old_pass = model_renderpass;
    model_renderpass = pass;
    return old_pass;
}
