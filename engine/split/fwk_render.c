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
    glActiveTexture( GL_TEXTURE0 + tex->unit );
    glBindTexture( GL_TEXTURE_2D, tex->id );
    glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 0, 0, window_width(), window_height(), 0 );
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
        PRINTF("!ERROR: shader_compile(): %s\n%s\n", type == GL_VERTEX_SHADER ? "Vertex" : "Fragment", buf);
        return 0;
    }

    return shader;
}

unsigned shader(const char *vs, const char *fs, const char *attribs, const char *fragcolor, const char *defines){
    return shader_geom(NULL, vs, fs, attribs, fragcolor, defines);
}

unsigned shader_geom(const char *gs, const char *vs, const char *fs, const char *attribs, const char *fragcolor, const char *defines) {
    PRINTF(/*"!"*/"Compiling shader\n");

    char *glsl_defines = "";
    if( defines ) {
        for each_substring(defines, ",", def) {
            glsl_defines = va("%s#define %s\n", glsl_defines, def);
        }
    }

    const char *glsl_version = ifdef(ems, "300 es", "150");

    if(gs)
    gs = gs && gs[0] == '#' && gs[1] == 'v' ? gs : va("#version %s\n%s\n%s", glsl_version, glsl_defines, gs ? gs : "");
    vs = vs && vs[0] == '#' && vs[1] == 'v' ? vs : va("#version %s\n%s\n%s", glsl_version, glsl_defines, vs ? vs : "");
    fs = fs && fs[0] == '#' && fs[1] == 'v' ? fs : va("#version %s\n%s\n%s", glsl_version, glsl_defines, fs ? fs : "");

#if is(ems)
    {
        char *vs_ = REALLOC( 0, strlen(vs) + 512 ); strcpy(vs_, vs);
        char *fs_ = REALLOC( 0, strlen(fs) + 512 ); strcpy(fs_, fs);
        char *gs_ = 0; if (gs) REALLOC( 0, strlen(gs) + 512 ); strcpy(gs_, gs);
        strrepl(&fs_, "#version 300 es\n", "#version 300 es\nprecision mediump float;\n");
        vs = vs_; fs = fs_; gs = gs_;
    }
#endif

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
            PRINTF("Shader.attribute[%d]=%s\n", i, attrib);
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
        }
        if (status == GL_FALSE) {
            PANIC("ERROR: shader(): Shader/program link: %s\n", buf);
            return 0;
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

static __thread unsigned last_shader = -1;
static
int shader_uniform(const char *name) {
    int ret = glGetUniformLocation(last_shader, name);
    // if( ret < 0 ) PRINTF("!cannot find uniform '%s' in shader program %d\n", name, (int)last_shader );
    return ret;
}
unsigned shader_get_active() { return last_shader; }
unsigned shader_bind(unsigned program) { unsigned ret = last_shader; return glUseProgram(last_shader = program), ret; }
void shader_int(const char *uniform, int i)     { glUniform1i(shader_uniform(uniform), i); }
void shader_float(const char *uniform, float f) { glUniform1f(shader_uniform(uniform), f); }
void shader_vec2(const char *uniform, vec2 v)   { glUniform2fv(shader_uniform(uniform), 1, &v.x); }
void shader_vec3(const char *uniform, vec3 v)   { glUniform3fv(shader_uniform(uniform), 1, &v.x); }
void shader_vec3v(const char *uniform, int count, vec3 *v) { glUniform3fv(shader_uniform(uniform), count, &v[0].x); }
void shader_vec4(const char *uniform, vec4 v)   { glUniform4fv(shader_uniform(uniform), 1, &v.x); }
void shader_mat44(const char *uniform, mat44 m) { glUniformMatrix4fv(shader_uniform(uniform), 1, GL_FALSE/*GL_TRUE*/, m); }
void shader_cubemap(const char *sampler, unsigned texture) { glUniform1i(shader_uniform(sampler), 0); glBindTexture(GL_TEXTURE_CUBE_MAP, texture); }
void shader_bool(const char *uniform, bool x) { glUniform1i(shader_uniform(uniform), x); }
void shader_uint(const char *uniform, unsigned x ) { glUniform1ui(shader_uniform(uniform), x); }
void shader_texture(const char *sampler, texture_t t) { shader_texture_unit(sampler, t.id, t.unit); }
void shader_texture_unit(const char *sampler, unsigned id, unsigned unit) {
    // @todo. if tex.h == 1 ? GL_TEXTURE_1D : GL_TEXTURE_2D
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

static
int allocate_texture_unit() {
    static int textureUnit = 0, totalTextureUnits = 0;
    do_once glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &totalTextureUnits);

    ASSERT(textureUnit < totalTextureUnits, "%d texture units exceeded", totalTextureUnits);
    return textureUnit++;
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
//        glTexParameterf(texture_type, GL_TEXTURE_MAX_ANISOTROPY, max_aniso);
}

    // glBindTexture(texture_type, 0); // do not unbind. current code expects texture to be bound at function exit

    t->w = w;
    t->h = h;
    t->n = n;
    t->flags = flags;
    t->filename = t->filename ? t->filename : "";

    return t->id;
}

texture_t texture_create(unsigned w, unsigned h, unsigned n, const void *pixels, int flags) {
    texture_t texture = {0};
    glGenTextures( 1, &texture.id );
    texture_update( &texture, w, h, n, pixels, flags );
    texture.unit = allocate_texture_unit();
    texture.transparent = texture.n > 3; // @fixme: should be true only if any pixel.a == 0
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
// shadowmaps

shadowmap_t shadowmap(int texture_width) { // = 1024
    shadowmap_t s = {0};
    s.texture_width = texture_width;

    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &s.saved_fb);

    glGenFramebuffers(1, &s.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, s.fbo);

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &s.texture);
    glBindTexture(GL_TEXTURE_2D, s.texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, texture_width, texture_width, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, s.texture, 0);

#if is(ems)
    GLenum nones[] = { GL_NONE };
    glDrawBuffers(1, nones);
    glReadBuffer(GL_NONE);
#else
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
#endif

    glBindFramebuffer(GL_FRAMEBUFFER, s.saved_fb);
    return s;
}

void shadowmap_destroy(shadowmap_t *s) {
    if (s->texture) {
        glDeleteTextures(1, &s->texture);
    }
    if (s->fbo) {
        glDeleteFramebuffers(1, &s->fbo);
    }
    shadowmap_t z = {0};
    *s = z;
}

void shadowmap_set_shadowmatrix(shadowmap_t *s, vec3 aLightPos, vec3 aLightAt, vec3 aLightUp, const mat44 projection) {
    copy44(s->proj, projection);
    s->light_position = vec4(aLightPos.x, aLightPos.y, aLightPos.z, 1);
    lookat44(s->mv, aLightPos, aLightAt, aLightUp);

    mat44 bias = {
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.5, 0.5, 0.5, 1.0 };

    // s->shadowmatrix = bias;
    // s->shadowmatrix *= s->proj;
    // s->shadowmatrix *= s->mv;
//  multiply44x3(s->shadowmatrix, s->mv, s->proj, bias);
    multiply44x3(s->shadowmatrix, bias, s->proj, s->mv);

    // mvp = projection * s->mv;
//  multiply44x2(s->mvp, s->mv, projection);
    multiply44x2(s->mvp, projection, s->mv);
}

void shadowmap_begin(shadowmap_t *s) {
    glGetIntegerv(GL_VIEWPORT, &s->saved_viewport[0]);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &s->saved_fb);

    glBindFramebuffer(GL_FRAMEBUFFER, s->fbo);
    glViewport(0, 0, s->texture_width, s->texture_width);

    glClearDepth(1);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void shadowmap_end(shadowmap_t *s) {
    glViewport(s->saved_viewport[0], s->saved_viewport[1], s->saved_viewport[2], s->saved_viewport[3]);
    glBindFramebuffer(GL_FRAMEBUFFER, s->saved_fb);
}

// shadowmap utils

void shadowmatrix_proj(mat44 shm_proj, float aLightFov, float znear, float zfar) {
    perspective44(shm_proj, aLightFov, 1.0f, znear, zfar);
}

void shadowmatrix_ortho(mat44 shm_proj, float left, float right, float bottom, float top, float znear, float zfar) {
    ortho44(shm_proj, left, right, bottom, top, znear, zfar);
}

// -----------------------------------------------------------------------------
// fullscreen quads

// usage: bind empty vao & commit call for 6 (quad) or 3 vertices (tri).
// ie, glBindVertexArray(empty_vao); glDrawArrays(GL_TRIANGLES, 0, 3);

void fullscreen_quad_rgb( texture_t texture, float gamma ) {
    static int program = -1, vao = -1, u_inv_gamma = -1;
    if( program < 0 ) {
        const char* vs = vfs_read("shaders/vs_0_2_fullscreen_quad_B_flipped.glsl");
        const char* fs = vfs_read("shaders/fs_2_4_texel_inv_gamma.glsl");

        program = shader(vs, fs, "", "fragcolor" , NULL);
        u_inv_gamma = glGetUniformLocation(program, "u_inv_gamma");
        glGenVertexArrays( 1, (GLuint*)&vao );
    }

    GLenum texture_type = texture.flags & TEXTURE_ARRAY ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
//    glEnable( GL_BLEND );
    glUseProgram( program );
    glUniform1f( u_inv_gamma, 1.0f / (gamma + !gamma) );

    glBindVertexArray( vao );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( texture_type, texture.id );

    glDrawArrays( GL_TRIANGLES, 0, 6 );
    profile_incstat("Render.num_drawcalls", +1);
    profile_incstat("Render.num_triangles", +2);

    glBindTexture( texture_type, 0 );
    glBindVertexArray( 0 );
    glUseProgram( 0 );
//    glDisable( GL_BLEND );
}

void fullscreen_quad_rgb_flipped( texture_t texture, float gamma ) {
    static int program = -1, vao = -1, u_inv_gamma = -1;
    if( program < 0 ) {
        const char* vs = vfs_read("shaders/vs_0_2_fullscreen_quad_B.glsl");
        const char* fs = vfs_read("shaders/fs_2_4_texel_inv_gamma.glsl");

        program = shader(vs, fs, "", "fragcolor" , NULL);
        u_inv_gamma = glGetUniformLocation(program, "u_inv_gamma");
        glGenVertexArrays( 1, (GLuint*)&vao );
    }

    GLenum texture_type = texture.flags & TEXTURE_ARRAY ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
//    glEnable( GL_BLEND );
    glUseProgram( program );
    glUniform1f( u_inv_gamma, 1.0f / (gamma + !gamma) );

    glBindVertexArray( vao );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( texture_type, texture.id );

    glDrawArrays( GL_TRIANGLES, 0, 6 );
    profile_incstat("Render.num_drawcalls", +1);
    profile_incstat("Render.num_triangles", +2);

    glBindTexture( texture_type, 0 );
    glBindVertexArray( 0 );
    glUseProgram( 0 );
//    glDisable( GL_BLEND );
}

void fullscreen_quad_ycbcr( texture_t textureYCbCr[3], float gamma ) {
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

//    glEnable( GL_BLEND );
    glUseProgram( program );
    glUniform1f( u_gamma, gamma );

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

void fullscreen_quad_ycbcr_flipped( texture_t textureYCbCr[3], float gamma ) {
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

//    glEnable( GL_BLEND );
    glUseProgram( program );
    glUniform1f( u_gamma, gamma );

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
}

static cubemap_t *last_cubemap;

cubemap_t* cubemap_get_active() {
    return last_cubemap;
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
    sky.flags = flags ? flags : !!asset; // either cubemap or rayleigh
    sky.program = shader(vfs_read("shaders/vs_3_3_skybox.glsl"),
        sky.flags ? vfs_read("fs_3_4_skybox.glsl") : vfs_read("shaders/fs_3_4_skybox_rayleigh.glsl"),
        "att_position", "fragcolor", NULL);

    // sky cubemap & SH
    if( asset ) {
        int is_panorama = vfs_size( asset );
        if( is_panorama ) { // is file
            stbi_hdr_to_ldr_gamma(1.2f);
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
        shader_bind(sky.program);
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

void skybox_mie_calc_sh(skybox_t *sky, float sky_intensity) {
    unsigned WIDTH = 1024, HEIGHT = 1024;
    int last_fb;
    int vp[4];
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &last_fb);
    glGetIntegerv(GL_VIEWPORT, vp);

    if (!sky_intensity) {
        sky_intensity = 1.0f;
    }

    if (!sky->pixels)
        sky->pixels = MALLOC(WIDTH*HEIGHT*12);

    if (!sky->framebuffers[0]) {
        for(int i = 0; i < 6; ++i) {
            glGenFramebuffers(1, &sky->framebuffers[i]);
            glBindFramebuffer(GL_FRAMEBUFFER, sky->framebuffers[i]);

            glGenTextures(1, &sky->textures[i]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, sky->textures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sky->textures[i], 0);
        }
    }

    static vec3 directions[6] = {{ 1, 0, 0},{-1, 0, 0},{ 0, 1, 0},{ 0,-1, 0},{ 0, 0, 1},{ 0, 0,-1}};

    int samples = 0;
    for(int i = 0; i < 6; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, sky->framebuffers[i]);
        glViewport(0, 0, WIDTH, HEIGHT);
        glUseProgram(sky->program);

        mat44 proj; perspective44(proj, 90.0f, WIDTH / (float)HEIGHT, 0.1f, 500.f);
        mat44 view; lookat44(view, vec3(0,0,0), directions[i], vec3(0,-1,0));

        skybox_render(sky, proj, view);

        glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGB, GL_FLOAT, sky->pixels);

        // calculate SH coefficients (@ands)
        // copied from cubemap6 method
        const vec3 skyDir[] = {{ 1, 0, 0},{-1, 0, 0},{ 0, 1, 0},{ 0,-1, 0},{ 0, 0, 1},{ 0, 0,-1}};
        const vec3 skyX[]   = {{ 0, 0,-1},{ 0, 0, 1},{ 1, 0, 0},{ 1, 0, 0},{ 1, 0, 0},{-1, 0, 0}};
        const vec3 skyY[]   = {{ 0, 1, 0},{ 0, 1, 0},{ 0, 0,-1},{ 0, 0, 1},{ 0, 1, 0},{ 0, 1, 0}};
        int step = 16;
        for (int y = 0; y < HEIGHT; y += step) {
            float *p = (float*)(sky->pixels + y * WIDTH * 3);
            for (int x = 0; x < WIDTH; x += step) {
                vec3 n = add3(
                    add3(
                        scale3(skyX[i],  2.0f * (x / (WIDTH - 1.0f)) - 1.0f),
                        scale3(skyY[i], -2.0f * (y / (HEIGHT - 1.0f)) + 1.0f)),
                    skyDir[i]); // texelDirection;
                float l = len3(n);
                vec3 light = scale3(vec3(p[0], p[1], p[2]), (1 / (l * l * l)) * sky_intensity); // texelSolidAngle * texel_radiance;
                n = norm3(n);
                sky->cubemap.sh[0] = add3(sky->cubemap.sh[0], scale3(light,  0.282095f));
                sky->cubemap.sh[1] = add3(sky->cubemap.sh[1], scale3(light, -0.488603f * n.y * 2.0 / 3.0));
                sky->cubemap.sh[2] = add3(sky->cubemap.sh[2], scale3(light,  0.488603f * n.z * 2.0 / 3.0));
                sky->cubemap.sh[3] = add3(sky->cubemap.sh[3], scale3(light, -0.488603f * n.x * 2.0 / 3.0));
                sky->cubemap.sh[4] = add3(sky->cubemap.sh[4], scale3(light,  1.092548f * n.x * n.y / 4.0));
                sky->cubemap.sh[5] = add3(sky->cubemap.sh[5], scale3(light, -1.092548f * n.y * n.z / 4.0));
                sky->cubemap.sh[6] = add3(sky->cubemap.sh[6], scale3(light,  0.315392f * (3.0f * n.z * n.z - 1.0f) / 4.0));
                sky->cubemap.sh[7] = add3(sky->cubemap.sh[7], scale3(light, -1.092548f * n.x * n.z / 4.0));
                sky->cubemap.sh[8] = add3(sky->cubemap.sh[8], scale3(light,  0.546274f * (n.x * n.x - n.y * n.y) / 4.0));
                p += 3 * step;
                samples++;
            }
        }
    }

    for (int s = 0; s < 9; s++) {
        sky->cubemap.sh[s] = scale3(sky->cubemap.sh[s], 32.f / samples);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, last_fb);
    glViewport(vp[0], vp[1], vp[2], vp[3]);
}

void skybox_sh_reset(skybox_t *sky) {
    for (int s = 0; s < 9; s++) {
        sky->cubemap.sh[s] = vec3(0,0,0);
    }
}

void skybox_sh_add_light(skybox_t *sky, vec3 light, vec3 dir, float strength) {
    // Normalize the direction
    vec3 norm_dir = norm3(dir);

    // Scale the light color and intensity
    vec3 scaled_light = scale3(light, strength);

    // Add light to the SH coefficients
    sky->cubemap.sh[0] = add3(sky->cubemap.sh[0], scale3(scaled_light,  0.282095f));
    sky->cubemap.sh[1] = add3(sky->cubemap.sh[1], scale3(scaled_light, -0.488603f * norm_dir.y));
    sky->cubemap.sh[2] = add3(sky->cubemap.sh[2], scale3(scaled_light,  0.488603f * norm_dir.z));
    sky->cubemap.sh[3] = add3(sky->cubemap.sh[3], scale3(scaled_light, -0.488603f * norm_dir.x));
}

API vec4 window_getcolor_(); // internal use, not public

int skybox_push_state(skybox_t *sky, mat44 proj, mat44 view) {
    last_cubemap = &sky->cubemap;

    //glClear(GL_DEPTH_BUFFER_BIT);
    //glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    //glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    // we have to reset clear color here, because of wrong alpha compositing issues on native transparent windows otherwise
    vec4 bgcolor = window_getcolor_(); glClearColor(bgcolor.r, bgcolor.g, bgcolor.b, 1); // @transparent

    mat44 mvp; multiply44x2(mvp, proj, view);

    //glDepthMask(GL_FALSE);
    shader_bind(sky->program);
    shader_mat44("u_mvp", mvp);
    if( sky->flags ) {
        shader_cubemap("u_cubemap", sky->cubemap.id);
    }
    return 0; // @fixme: return sortable hash here?
}
int skybox_pop_state() {
    //vec4 bgcolor = window_getcolor_(); glClearColor(bgcolor.r, bgcolor.g, bgcolor.b, window_has_transparent() ? 0 : bgcolor.a); // @transparent
    //glDepthMask(GL_TRUE);
    //glClear(GL_DEPTH_BUFFER_BIT);
    return 0;
}
int skybox_render(skybox_t *sky, mat44 proj, mat44 view) {
    skybox_push_state(sky, proj, view);
    glEnable(GL_DEPTH_TEST);
    mesh_render(&sky->geometry);
    skybox_pop_state();
    return 0;
}
void skybox_destroy(skybox_t *sky) {
    glDeleteProgram(sky->program);
    cubemap_destroy(&sky->cubemap);
    mesh_destroy(&sky->geometry);

    if (sky->pixels) {
        FREE(sky->pixels);
        glDeleteFramebuffers(6, sky->framebuffers);
        glDeleteTextures(6, sky->textures);
    }
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
    (void)m;
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

    glBindFramebuffer (GL_FRAMEBUFFER, 0);
    return fbo;
}
void fbo_bind(unsigned id) {
    glBindFramebuffer(GL_FRAMEBUFFER, id);
}
void fbo_unbind() {
    fbo_bind(0);
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

char* postfx_name(postfx *fx, int slot);

int   ui_postfx(postfx *fx, int slot);

struct passfx {
    mesh_t m;
    char *name;
    unsigned program;
    int uniforms[16];
};

struct postfx {
    // renderbuffers: color & depth textures
    unsigned fb[2];
    texture_t diffuse[2], depth[2];
    // shader passes
    passfx pass[64];
    uint64_t mask;
    // global enable flag
    bool enabled;
    //
    int num_loaded;
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
    for( int i = 0; i < 64; ++i ) {
        FREE(fx->pass[i].name);
    }
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
    return slot < 0 ? "" : fx->pass[ slot & 63 ].name;
}
int postfx_find(postfx *fx, const char *name) {
    name = file_name(name);
    for( int i = 0; i < 64; ++i) if(!strcmpi(fx->pass[i].name, name)) return i;
    return -1;
}

int postfx_load_from_mem( postfx *fx, const char *name, const char *fs ) {
    PRINTF("%s\n", name);
    if(!fs || !fs[0]) return -1; // PANIC("!invalid fragment shader");

    int slot = fx->num_loaded++;

    passfx *p = &fx->pass[ slot & 63 ];
    p->name = STRDUP(name);

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
    return slot;
}

bool postfx_enable(postfx *fx, int pass, bool enabled) {
    if( pass < 0 ) return false;
    fx->mask = enabled ? fx->mask | (1ull << pass) : fx->mask & ~(1ull << pass);
    fx->enabled = !!popcnt64(fx->mask);
    return fx->enabled;
}

bool postfx_enabled(postfx *fx, int pass) {
    if( pass < 0 ) return false;
    return (!!(fx->mask & (1ull << pass)));
}

bool postfx_toggle(postfx *fx, int pass) {
    if( pass < 0 ) return false;
    return postfx_enable(fx, pass, 1 ^ postfx_enabled(fx, pass));
}

void postfx_clear(postfx *fx) {
    fx->mask = fx->enabled = 0;
}

int ui_postfx(postfx *fx, int pass) {
    int on = ui_enabled();
    ( postfx_enabled(fx,pass) ? ui_enable : ui_disable )();
    int rc = ui_shader(fx->pass[pass].program);
    ( on ? ui_enable : ui_disable )();
    return rc;
}

static __thread array(handle) last_fb;

bool postfx_begin(postfx *fx, int width, int height) {
    // reset clear color: needed in case transparent window is being used (alpha != 0)
    glClearColor(0,0,0,0); // @transparent

    width += !width;
    height += !height;

    int fb;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fb);
    array_push(last_fb, fb);

    // resize if needed
    if( fx->diffuse[0].w != width || fx->diffuse[0].h != height ) {
        texture_destroy(&fx->diffuse[0]);
        texture_destroy(&fx->diffuse[1]);
        texture_destroy(&fx->depth[0]);
        texture_destroy(&fx->depth[1]);
        fbo_destroy(fx->fb[0]);
        fbo_destroy(fx->fb[1]);

        // create texture, set texture parameters and content
        fx->diffuse[0] = texture_create(width, height, 4, NULL, TEXTURE_RGBA);
        fx->depth[0] = texture_create(width, height, 1,  NULL, TEXTURE_DEPTH|TEXTURE_FLOAT);
        fx->fb[0] = fbo(fx->diffuse[0].id, fx->depth[0].id, 0);

        // create texture, set texture parameters and content
        fx->diffuse[1] = texture_create(width, height, 4, NULL, TEXTURE_RGBA);
        fx->depth[1] = texture_create(width, height, 1, NULL, TEXTURE_DEPTH|TEXTURE_FLOAT);
        fx->fb[1] = fbo(fx->diffuse[1].id, fx->depth[1].id, 0);
    }

    uint64_t num_active_passes = popcnt64(fx->mask);
    bool active = fx->enabled && num_active_passes;
    if( !active ) {
        array_pop(last_fb);
        fbo_bind(fb);
        return false;
    }

    fbo_bind(fx->fb[1]);

    viewport_clear(true, true);
    viewport_clip(vec2(0,0), vec2(width, height));

    fbo_bind(fx->fb[0]);

    viewport_clear(true, true);
    viewport_clip(vec2(0,0), vec2(width, height));

    return true;
}

bool postfx_end(postfx *fx) {
    uint64_t num_active_passes = popcnt64(fx->mask);
    bool active = fx->enabled && num_active_passes;
    if( !active ) {
        return false;
    }

    handle fb = *array_back(last_fb);
    array_pop(last_fb);
    fbo_bind(fb);

    // disable depth test in 2d rendering
    bool is_depth_test_enabled = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);

    int frame = 0;
    float t = time_ms() / 1000.f;
    float w = fx->diffuse[0].w;
    float h = fx->diffuse[0].h;
    float mx = input(MOUSE_X);
    float my = input(MOUSE_Y);

    for(int i = 0, e = countof(fx->pass); i < e; ++i) {
        if( fx->mask & (1ull << i) ) {
            passfx *pass = &fx->pass[i];

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
            if( bound ) fbo_bind(fx->fb[frame ^= 1]);

                // fullscreen quad
                glBindVertexArray(pass->m.vao);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                profile_incstat("Render.num_drawcalls", +1);
                profile_incstat("Render.num_triangles", +2);
                glBindVertexArray(0);

            if( bound ) fbo_bind(fb);
            else glUseProgram(0);
        }
    }

    if(is_depth_test_enabled);
    glEnable(GL_DEPTH_TEST);

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
    for( int i = 0; i < fx.num_loaded; ++i ) fx_enable(i, enabled);
}
char *fx_name(int pass) {
    return postfx_name(&fx, pass);
}
int fx_find(const char *name) {
    return postfx_find(&fx, name);
}
int ui_fx(int pass) {
    return ui_postfx(&fx, pass);
}
int ui_fxs() {
    if(!fx.num_loaded) return ui_label(ICON_MD_WARNING " No Post FXs with annotations loaded."), 0;

    int changed = 0;
    for( int i = 0; i < 64; ++i ) {
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
    const char *filename;
    filename = "Skyboxes/brdf_lut1k_256x256_32F.ktx";
    filename = "Skyboxes/brdf_lut2k_512x512_32F.ktx";

    brdf = texture_compressed( filename,
        TEXTURE_CLAMP | TEXTURE_NEAREST | TEXTURE_RG | TEXTURE_FLOAT | TEXTURE_SRGB
    );
    unsigned texchecker = texture_checker().id;
    ASSERT(brdf.id != texchecker, "!Couldn't load BRDF lookup table '%s'!", filename );
}

texture_t brdf_lut() {
    do_once brdf_load();
    return brdf;
}

// -----------------------------------------------------------------------------
// materials

bool colormap( colormap_t *cm, const char *material_file, bool load_as_srgb ) {
    if( !material_file ) return false;

    if( cm->texture ) {
        texture_destroy(cm->texture);
        FREE(cm->texture), cm->texture = NULL;
    }

    int srgb = load_as_srgb ? TEXTURE_SRGB : 0;
    int hdr = strendi(material_file, ".hdr") ? TEXTURE_FLOAT | TEXTURE_RGBA : 0;
    texture_t t = texture_compressed(material_file, TEXTURE_LINEAR | TEXTURE_MIPMAPS | TEXTURE_REPEAT | hdr | srgb);

    if( t.id == texture_checker().id ) {
        cm->texture = NULL;
        return false;
    }
    cm->texture = CALLOC(1, sizeof(texture_t));
    *cm->texture = t;
    return true;
}

bool pbr_material(pbr_material_t *pbr, const char *material) {
    if( !material || !pbr ) return false;

    //pbr_material_destroy(pbr);
    *pbr = (pbr_material_t){0};

    pbr->name = STRDUP(material);

    pbr->specular_shininess = 1.0f;
    /*
    if( const float *f = aiGetMaterialFloat(scn_material[i], aiMaterialTypeString(MATERIAL_SHININESS)) ) {
        pbr->specular_shininess = *f;
    }
    */

    pbr->diffuse.color = vec4(0.5,0.5,0.5,0.5);
    pbr->normals.color = vec4(0,0,0,0);
    pbr->specular.color = vec4(0,0,0,0);
    pbr->albedo.color = vec4(0.5,0.5,0.5,1.0);
    pbr->roughness.color = vec4(1,1,1,1);
    pbr->metallic.color = vec4(0,0,0,0);
    pbr->ao.color = vec4(1,1,1,1);
    pbr->ambient.color = vec4(0,0,0,1);
    pbr->emissive.color = vec4(0,0,0,0);

    array(char*) tokens = strsplit(material, "+");
    for( int j = 0, end = array_count(tokens); j < end; ++j ) {
        char *t = tokens[j];
        if( strstri(t, "_D.") || strstri(t, "Diffuse") || strstri(t, "BaseColor") )    colormap(&pbr->diffuse, t, 1);
        if( strstri(t, "_N.") || strstri(t, "Normal") )     colormap(&pbr->normals, t, 0);
        if( strstri(t, "_S.") || strstri(t, "Specular") )   colormap(&pbr->specular, t, 0);
        if( strstri(t, "_A.") || strstri(t, "Albedo") )     colormap(&pbr->albedo, t, 1); // 0?
        if( strstri(t, "_MR.")|| strstri(t, "Roughness") )  colormap(&pbr->roughness, t, 0);
        else
        if( strstri(t, "_M.") || strstri(t, "Metallic") )   colormap(&pbr->metallic, t, 0);
      //if( strstri(t, "_S.") || strstri(t, "Shininess") )  colormap(&pbr->roughness, t, 0);
      //if( strstri(t, "_A.") || strstri(t, "Ambient") )    colormap(&pbr->ambient, t, 0);
        if( strstri(t, "_E.") || strstri(t, "Emissive") )   colormap(&pbr->emissive, t, 1);
        if( strstri(t, "_AO.") || strstri(t, "AO") || strstri(t, "Occlusion") ) colormap(&pbr->ao, t, 0);
    }

    return true;
}

void pbr_material_destroy(pbr_material_t *m) {
    if( m->name )              FREE(m->name), m->name = NULL;
    if( m->diffuse.texture)    texture_destroy( m->diffuse.texture );
    if( m->normals.texture)    texture_destroy( m->normals.texture );
    if( m->specular.texture)   texture_destroy( m->specular.texture );
    if( m->albedo.texture)     texture_destroy( m->albedo.texture );
    if( m->roughness.texture)  texture_destroy( m->roughness.texture );
    if( m->metallic.texture)   texture_destroy( m->metallic.texture );
    if( m->ao.texture )        texture_destroy( m->ao.texture );
    if( m->ambient.texture )   texture_destroy( m->ambient.texture );
    *m = (pbr_material_t){0};
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

        glBindVertexArray(s->vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        if(s->dims.x && !(s->flags&SHADERTOY_IGNORE_FBO)) texture_rec_end(&s->tx); // texture_rec
    }
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
    GLubyte color[4];
    GLfloat texcoord2[2];
} iqm_vertex;

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
} iqm_t;

#define meshdata (q->meshdata)
#define animdata (q->animdata)
#define nummeshes (q->nummeshes)
#define numtris (q->numtris)
#define numverts (q->numverts)
#define numjoints (q->numjoints)
#define numframes (q->numframes)
#define numanims (q->numanims)
#define meshes (q->meshes)
#define textures (q->textures)
#define joints (q->joints)
#define poses (q->poses)
#define anims (q->anims)
#define baseframe (q->baseframe)
#define inversebaseframe (q->inversebaseframe)
#define outframe (q->outframe)
#define frames (q->frames)
#define vao (q->vao)
#define ibo (q->ibo)
#define vbo (q->vbo)
#define bonematsoffset (q->bonematsoffset)
#define buf (q->buf)
#define bounds (q->bounds)
#define colormaps (q->colormaps)

void model_set_texture(model_t m, texture_t t) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    for( int i = 0; i < nummeshes; ++i) { // assume 1 texture per mesh
        textures[i] = t.id;
    }
}

static
void model_set_uniforms(model_t m, int shader, mat44 mv, mat44 proj, mat44 view, mat44 model) {  // @todo: cache uniform locs
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    glUseProgram(shader);
    int loc;
    //if( (loc = glGetUniformLocation(shader, "M")) >= 0 ) glUniformMatrix4fv( loc, 1, GL_FALSE/*GL_TRUE*/, m); // RIM
    if( (loc = glGetUniformLocation(shader, "MV")) >= 0 ) {
        glUniformMatrix4fv( loc, 1, GL_FALSE, mv);
    }
    else
    if( (loc = glGetUniformLocation(shader, "u_mv")) >= 0 ) {
        glUniformMatrix4fv( loc, 1, GL_FALSE, mv);
    }
    if( (loc = glGetUniformLocation(shader, "MVP")) >= 0 ) {
        mat44 mvp; multiply44x2(mvp, proj, mv); // multiply44x3(mvp, proj, view, model);
        glUniformMatrix4fv( loc, 1, GL_FALSE, mvp);
    }
    else
    if( (loc = glGetUniformLocation(shader, "u_mvp")) >= 0 ) {
        mat44 mvp; multiply44x2(mvp, proj, mv); // multiply44x3(mvp, proj, view, model);
        glUniformMatrix4fv( loc, 1, GL_FALSE, mvp);
    }
    if( (loc = glGetUniformLocation(shader, "VP")) >= 0 ) {
        mat44 vp; multiply44x2(vp, proj, view);
        glUniformMatrix4fv( loc, 1, GL_FALSE, vp);
    }
    else
    if( (loc = glGetUniformLocation(shader, "u_vp")) >= 0 ) {
        mat44 vp; multiply44x2(vp, proj, view);
        glUniformMatrix4fv( loc, 1, GL_FALSE, vp);
    }
    if( (loc = glGetUniformLocation(shader, "u_cam_pos")) >= 0 ) {
        vec3 pos = vec3(view[12], view[13], view[14]);
        glUniform3fv( loc, 1, &pos.x );
    }
    else
    if( (loc = glGetUniformLocation(shader, "cam_pos")) >= 0 ) {
        vec3 pos = vec3(view[12], view[13], view[14]);
        glUniform3fv( loc, 1, &pos.x );
    }
    if( (loc = glGetUniformLocation(shader, "u_cam_dir")) >= 0 ) {
        vec3 dir = norm3(vec3(view[2], view[6], view[10]));
        glUniform3fv( loc, 1, &dir.x );
    }
    else
    if( (loc = glGetUniformLocation(shader, "cam_dir")) >= 0 ) {
        vec3 dir = norm3(vec3(view[2], view[6], view[10]));
        glUniform3fv( loc, 1, &dir.x );
    }
    if( (loc = glGetUniformLocation(shader, "billboard")) >= 0 ) {
        glUniform1i( loc, m.billboard );
    }
    else
    if( (loc = glGetUniformLocation(shader, "u_billboard")) >= 0 ) {
        glUniform1i( loc, m.billboard );
    }
    if( (loc = glGetUniformLocation(shader, "texlit")) >= 0 ) {
        glUniform1i( loc, (m.lightmap.w != 0) );
    }
    else
    if( (loc = glGetUniformLocation(shader, "u_texlit")) >= 0 ) {
        glUniform1i( loc, (m.lightmap.w != 0) );
    }
#if 0
    // @todo: mat44 projview (useful?)
#endif
    if ((loc = glGetUniformLocation(shader, "M")) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, model);
    }
    else
    if ((loc = glGetUniformLocation(shader, "model")) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, model);
    }
    if ((loc = glGetUniformLocation(shader, "V")) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, view);
    }
    else
    if ((loc = glGetUniformLocation(shader, "view")) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, view);
    }
    if ((loc = glGetUniformLocation(shader, "P")) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, proj);
    }
    else
    if ((loc = glGetUniformLocation(shader, "proj")) >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, proj);
    }
    if( (loc = glGetUniformLocation(shader, "SKINNED")) >= 0 ) glUniform1i( loc, numanims ? GL_TRUE : GL_FALSE);
    if( numanims )
    if( (loc = glGetUniformLocation(shader, "vsBoneMatrix")) >= 0 ) glUniformMatrix3x4fv( loc, numjoints, GL_FALSE, outframe[0]);

    if ((loc = glGetUniformLocation(shader, "u_matcaps")) >= 0) {
        glUniform1i(loc, m.flags & MODEL_MATCAPS ? GL_TRUE:GL_FALSE);
    }
}
static
void model_set_state(model_t m) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    glBindVertexArray( vao );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, position) );
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, texcoord) );
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, normal) );
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, tangent) );

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    // vertex color
    glVertexAttribPointer(11, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex,color) );
    glEnableVertexAttribArray(11);

    // lmap data
    glVertexAttribPointer(12, 2, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, texcoord2) );
    glEnableVertexAttribArray(12);

    // animation
    if(numframes > 0) {
        glVertexAttribPointer( 8, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex,blendindexes) );
        glVertexAttribPointer( 9, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex,blendweights) );
        glVertexAttribPointer(10, 1, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, blendvertexindex) );
        glEnableVertexAttribArray(8);
        glEnableVertexAttribArray(9);
        glEnableVertexAttribArray(10);
    }

    // mat4 attribute; for instanced rendering
    if( 1 ) {
        unsigned vec4_size = sizeof(vec4);
        unsigned mat4_size = sizeof(vec4) * 4;

        // vertex buffer object
        glBindBuffer(GL_ARRAY_BUFFER, m.vao_instanced);
        glBufferData(GL_ARRAY_BUFFER, m.num_instances * mat4_size, m.instanced_matrices, GL_STATIC_DRAW);

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

static
bool model_load_meshes(iqm_t *q, const struct iqmheader *hdr, model_t *m) {
    if(meshdata) return false;

    lil32p(&buf[hdr->ofs_vertexarrays], hdr->num_vertexarrays*sizeof(struct iqmvertexarray)/sizeof(uint32_t));
    lil32p(&buf[hdr->ofs_triangles], hdr->num_triangles*sizeof(struct iqmtriangle)/sizeof(uint32_t));
    lil32p(&buf[hdr->ofs_meshes], hdr->num_meshes*sizeof(struct iqmmesh)/sizeof(uint32_t));
    lil32p(&buf[hdr->ofs_joints], hdr->num_joints*sizeof(struct iqmjoint)/sizeof(uint32_t));

    meshdata = buf;
    nummeshes = hdr->num_meshes;
    numtris = hdr->num_triangles;
    numverts = hdr->num_vertexes;
    numjoints = hdr->num_joints;
    outframe = CALLOC(hdr->num_joints, sizeof(mat34));

    float *inposition = NULL, *innormal = NULL, *intangent = NULL, *intexcoord = NULL, *invertexindex = NULL;
    uint8_t *inblendindex8 = NULL, *inblendweight8 = NULL;
    int *inblendindexi = NULL; float *inblendweightf = NULL;
    uint8_t *invertexcolor8 = NULL;
    struct iqmvertexarray *vas = (struct iqmvertexarray *)&buf[hdr->ofs_vertexarrays];
    for(int i = 0; i < (int)hdr->num_vertexarrays; i++) {
        struct iqmvertexarray *va = &vas[i];
        switch(va->type) {
        default: continue; // return PANIC("unknown iqm vertex type (%d)", va->type), false;
        break; case IQM_POSITION: ASSERT(va->format == IQM_FLOAT && va->size == 3); inposition = (float *)&buf[va->offset]; lil32pf(inposition, 3*hdr->num_vertexes);
        break; case IQM_NORMAL: ASSERT(va->format == IQM_FLOAT && va->size == 3); innormal = (float *)&buf[va->offset]; lil32pf(innormal, 3*hdr->num_vertexes);
        break; case IQM_TANGENT: ASSERT(va->format == IQM_FLOAT && va->size == 4); intangent = (float *)&buf[va->offset]; lil32pf(intangent, 4*hdr->num_vertexes);
        break; case IQM_TEXCOORD: ASSERT(va->format == IQM_FLOAT && va->size == 2); intexcoord = (float *)&buf[va->offset]; lil32pf(intexcoord, 2*hdr->num_vertexes);
        break; case IQM_COLOR: ASSERT(va->size == 4); ASSERT(va->format == IQM_UBYTE); invertexcolor8 = (uint8_t *)&buf[va->offset];
        break; case IQM_BLENDINDEXES: ASSERT(va->size == 4); ASSERT(va->format == IQM_UBYTE || va->format == IQM_INT);
        if(va->format == IQM_UBYTE) inblendindex8 = (uint8_t *)&buf[va->offset];
        else inblendindexi = (int *)&buf[va->offset];
        break; case IQM_BLENDWEIGHTS: ASSERT(va->size == 4); ASSERT(va->format == IQM_UBYTE || va->format == IQM_FLOAT);
        if(va->format == IQM_UBYTE) inblendweight8 = (uint8_t *)&buf[va->offset];
        else inblendweightf = (float *)&buf[va->offset];
        invertexindex = (inblendweight8 ? (float*)(inblendweight8 + 4) : inblendweightf + 4 );
        }
    }

    if (hdr->ofs_bounds) lil32p(buf + hdr->ofs_bounds, hdr->num_frames * sizeof(struct iqmbounds));
    if (hdr->ofs_bounds) bounds = (struct iqmbounds *) &buf[hdr->ofs_bounds];

    meshes = (struct iqmmesh *)&buf[hdr->ofs_meshes];
    joints = (struct iqmjoint *)&buf[hdr->ofs_joints];

    baseframe = CALLOC(hdr->num_joints, sizeof(mat34));
    inversebaseframe = CALLOC(hdr->num_joints, sizeof(mat34));
    for(int i = 0; i < (int)hdr->num_joints; i++) {
        struct iqmjoint *j = &joints[i];
        compose34(baseframe[i], ptr3(j->translate), normq(ptrq(j->rotate)), ptr3(j->scale));
        invert34(inversebaseframe[i], baseframe[i]);
        if(j->parent >= 0) {
            multiply34x2(baseframe[i], baseframe[j->parent], baseframe[i]);
            multiply34(inversebaseframe[i], inversebaseframe[j->parent]);
        }
    }

    struct iqmtriangle *tris = (struct iqmtriangle *)&buf[hdr->ofs_triangles];
    m->num_tris = hdr->num_triangles;
    m->tris = (void*)tris;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    if(!ibo) glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
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
        if(invertexcolor8) memcpy(v->color, &invertexcolor8[i*4], sizeof(v->color));
    }

    if(!vbo) glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
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

    textures = CALLOC(hdr->num_meshes * 8, sizeof(GLuint));
    colormaps = CALLOC(hdr->num_meshes * 8, sizeof(vec4));
    for(int i = 0; i < (int)hdr->num_meshes; i++) {
        int invalid = texture_checker().id;
        textures[i] = invalid;
    }

    const char *str = hdr->ofs_text ? (char *)&buf[hdr->ofs_text] : "";
    for(int i = 0; i < (int)hdr->num_meshes; i++) {
        struct iqmmesh *m = &meshes[i];
        PRINTF("loaded mesh: %s\n", &str[m->name]);
    }

    return true;
}

static
bool model_load_anims(iqm_t *q, const struct iqmheader *hdr) {
    if((int)hdr->num_poses != numjoints) return false;

    if(animdata) {
        if(animdata != meshdata) FREE(animdata);
        FREE(frames);
        animdata = NULL;
        anims = NULL;
        frames = 0;
        numframes = 0;
        numanims = 0;
    }

    lil32p(&buf[hdr->ofs_poses], hdr->num_poses*sizeof(struct iqmpose)/sizeof(uint32_t));
    lil32p(&buf[hdr->ofs_anims], hdr->num_anims*sizeof(struct iqmanim)/sizeof(uint32_t));
    lil16p((uint16_t *)&buf[hdr->ofs_frames], hdr->num_frames*hdr->num_framechannels);

    animdata = buf;
    numanims = hdr->num_anims;
    numframes = hdr->num_frames;

    anims = (struct iqmanim *)&buf[hdr->ofs_anims];
    poses = (struct iqmpose *)&buf[hdr->ofs_poses];
    frames = CALLOC(hdr->num_frames * hdr->num_poses, sizeof(mat34));
    uint16_t *framedata = (uint16_t *)&buf[hdr->ofs_frames];

    for(int i = 0; i < (int)hdr->num_frames; i++) {
        for(int j = 0; j < (int)hdr->num_poses; j++) {
            struct iqmpose *p = &poses[j];
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
            if(p->parent >= 0) multiply34x3(frames[i*hdr->num_poses + j], baseframe[p->parent], m, inversebaseframe[j]);
            else multiply34x2(frames[i*hdr->num_poses + j], m, inversebaseframe[j]);
        }
    }

    const char *str = hdr->ofs_text ? (char *)&buf[hdr->ofs_text] : "";
    for(int i = 0; i < (int)hdr->num_anims; i++) {
        struct iqmanim *a = &anims[i];
        PRINTF("loaded anim[%d]: %s\n", i, &str[a->name]);
    }

    return true;
}

// prevents crash on osx when strcpy'ing non __restrict arguments
static char* strcpy_safe(char *d, const char *s) {
    sprintf(d, "%s", s);
    return d;
}

static
bool model_load_textures(iqm_t *q, const struct iqmheader *hdr, model_t *model) {
    textures = textures ? textures : CALLOC(hdr->num_meshes * 8, sizeof(GLuint)); // up to 8 textures per mesh
    colormaps = colormaps ? colormaps : CALLOC(hdr->num_meshes * 8, sizeof(vec4)); // up to 8 colormaps per mesh

    GLuint *out = textures;

    const char *str = hdr->ofs_text ? (char *)&buf[hdr->ofs_text] : "";
    for(int i = 0; i < (int)hdr->num_meshes; i++) {
        struct iqmmesh *m = &meshes[i];

        // reuse texture+material if already decoded
        bool reused = 0;
        for( int j = 0; !reused && j < model->num_textures; ++j ) {
            if( !strcmpi(model->texture_names[j], &str[m->material])) {

                *out++ = model->materials[j].layer[0].texture;

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
        int flags = TEXTURE_MIPMAPS|TEXTURE_REPEAT; // LINEAR, NEAREST
        int invalid = texture_checker().id;

#if 1
        char *material_embedded_texture = strstr(&str[m->material], "+b64:");
        if( material_embedded_texture ) {
            *material_embedded_texture = '\0';
            material_embedded_texture += 5;
            array(char) embedded_texture = base64_decode(material_embedded_texture, strlen(material_embedded_texture));
            //printf("%s %d\n", material_embedded_texture, array_count(embedded_texture));
            //hexdump(embedded_texture, array_count(embedded_texture));
            *out = texture_compressed_from_mem( embedded_texture, array_count(embedded_texture), flags ).id;
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
            #if 0 // not enabled because of some .obj files like suzanne, with color_hex=9990 found
            if(material_color_hex[3])
            material_color.a = ((material_color_hex[3] >= 'a') ? material_color_hex[3] - 'a' + 10 : material_color_hex[3] - '0') / 15.f;
            else
            #endif
            material_color.a = 1;
        }

        if( !material_embedded_texture ) {
            char* material_name;
            // remove any material+name from materials (.fbx)
            // try left token first
            if( 1 ) {
                material_name = va("%s", &str[m->material]);
                char* plus = strrchr(material_name, '+');
                if (plus) { strcpy_safe(plus, file_ext(material_name)); }
                *out = texture_compressed(material_name, flags).id;
            }
            // else try right token
            if (*out == invalid) {
                material_name = file_normalize( va("%s", &str[m->material]) );
                char* plus = strrchr(material_name, '+'), *slash = strrchr(material_name, '/');
                if (plus) {
                    strcpy_safe(slash ? slash + 1 : material_name, plus + 1);
                    *out = texture_compressed(material_name, flags).id;
                }
            }
            // else last resort
            if (*out == invalid) {
                *out = texture_compressed(material_name, flags).id; // needed?
            }
        }

        if( *out != invalid) {
            PRINTF("loaded material[%d]: %s\n", i, &str[m->material]);
        } else {
            PRINTF("warn: material[%d] not found: %s\n", i, &str[m->material]);
            PRINTF("warn: using placeholder material[%d]=texture_checker\n", i);
            *out = texture_checker().id; // placeholder
        }

        inscribe_tex:;
        {
            model->num_textures++;
            array_push(model->texture_names, STRDUP(&str[m->material]));

            material_t mt = {0};
            mt.name = STRDUP(&str[m->material]);
            mt.layer[mt.count].color = material_color_hex ? material_color : vec4(1,1,1,1);
            mt.layer[mt.count++].texture = *out++;
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
        mt.count = 1;
        mt.layer[0].color = vec4(1,1,1,1);
        mt.layer[0].texture = texture_checker().id;

        array_push(model->materials, mt);
    }

    return true;
}

model_t model_from_mem(const void *mem, int len, int flags) {
    model_t m = {0};

    const char *ptr = (const char *)mem;
    // can't cache shader programs since we enable features via flags here
    // static int shaderprog = -1;
    // if( shaderprog < 0 ) {
        const char *symbols[] = { "{{include-shadowmap}}", vfs_read("shaders/fs_0_0_shadowmap_lit.glsl") }; // #define RIM
        int shaderprog = shader(strlerp(1,symbols,vfs_read("shaders/vs_323444143_16_3322_model.glsl")), strlerp(1,symbols,vfs_read("shaders/fs_32_4_model.glsl")), //fs,
            "att_position,att_texcoord,att_normal,att_tangent,att_instanced_matrix,,,,att_indexes,att_weights,att_vertexindex,att_color,att_bitangent,att_texcoord2","fragColor",
            va("SHADING_PHONG,%s", (flags&MODEL_RIMLIGHT)?"RIM":""));
    // }
    // ASSERT(shaderprog > 0);

    iqm_t *q = CALLOC(1, sizeof(iqm_t));
    m.program = shaderprog;

    int error = 1;
    if( ptr && len ) {
        struct iqmheader hdr; memcpy(&hdr, ptr, sizeof(hdr)); ptr += sizeof(hdr);
        if( !memcmp(hdr.magic, IQM_MAGIC, sizeof(hdr.magic))) {
            lil32p(&hdr.version, (sizeof(hdr) - sizeof(hdr.magic))/sizeof(uint32_t));
            if(hdr.version == IQM_VERSION) {
                buf = CALLOC(hdr.filesize, sizeof(uint8_t));
                memcpy(buf + sizeof(hdr), ptr, hdr.filesize - sizeof(hdr));
                error = 0;
                if( hdr.num_meshes > 0 && !(flags & MODEL_NO_MESHES) )     error |= !model_load_meshes(q, &hdr, &m);
                if( hdr.num_meshes > 0 && !(flags & MODEL_NO_TEXTURES) )   error |= !model_load_textures(q, &hdr, &m);
                if( hdr.num_anims  > 0 && !(flags & MODEL_NO_ANIMATIONS) ) error |= !model_load_anims(q, &hdr);
                if( buf != meshdata && buf != animdata ) FREE(buf);
            }
        }
    }

    if( error ) {
        PRINTF("Error: cannot load %s", "model");
        FREE(q), q = 0;
    } else {
        #undef vao
        #undef ibo
        #undef vbo
        m.vao = q->vao;
        m.ibo = q->ibo;
        m.vbo = q->vbo;
        m.num_verts = numverts;
        #define vao (q->vao)
        #define ibo (q->ibo)
        #define vbo (q->vbo)

        // m.boxes = bounds; // <@todo
        m.num_meshes = nummeshes;
        m.num_triangles = numtris;
        m.num_joints = numjoints;
        //m.num_poses = numposes;
        m.num_anims = numanims;
        m.num_frames = numframes;
        m.iqm = q;
        m.curframe = model_animate(m, 0);

        //m.num_textures = nummeshes; // assume 1 texture only per mesh
        #undef textures
        m.textures = (q->textures);
        #define textures (q->textures)

        m.flags = flags;

        id44(m.pivot);

        m.num_instances = 0;
        m.instanced_matrices = m.pivot;

        glGenBuffers(1, &m.vao_instanced);
        model_set_state(m);
    }
    return m;
}
model_t model(const char *filename, int flags) {
    int len;  // vfs_pushd(filedir(filename))
    char *ptr = vfs_load(filename, &len); // + vfs_popd
    return model_from_mem( ptr, len, flags );
}

bool model_get_bone_pose(model_t m, unsigned joint, mat34 *out) {
    if(!m.iqm) return false;
    iqm_t *q = m.iqm;

    if(joint >= numjoints) return false;

    multiply34x2(*out, outframe[joint], baseframe[joint]);
    return true;
}

anim_t clip(float minframe, float maxframe, float blendtime, unsigned flags) {
    return ((anim_t){minframe, maxframe, blendtime, flags, 1e6});
}
anim_t loop(float minframe, float maxframe, float blendtime, unsigned flags) {
    return clip(minframe, maxframe, blendtime, flags | ANIM_LOOP);
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

    mat34 *mat1 = &frames[frame1 * numjoints];
    mat34 *mat2 = &frames[frame2 * numjoints];
    mat34 *mat3 = &frames[frame3 * numjoints];
    mat34 *mat4 = &frames[frame4 * numjoints];

    for(int i = 0; i < numjoints; i++) {
        mat34 matA, matB, matF;
        lerp34(matA, mat1[i], mat2[i], alphaA);
        lerp34(matB, mat3[i], mat4[i], alphaB);
        lerp34(matF, matA, matB, alpha );
        if(joints[i].parent >= 0) multiply34x2(outframe[i], outframe[joints[i].parent], matF);
        else copy34(outframe[i], matF);
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
    if( numframes > 0 ) {
        vec3 p = pose(curframe >= m.curframe, curframe, minframe, maxframe, loop, &retframe);
        int frame1 = p.x;
        int frame2 = p.y;
        float offset = p.z;

        mat34 *mat1 = &frames[frame1 * numjoints];
        mat34 *mat2 = &frames[frame2 * numjoints];

        // @todo: add animation blending and inter-frame blending here
        for(int i = 0; i < numjoints; i++) {
            mat34 mat; lerp34(mat, mat1[i], mat2[i], offset);
            if(joints[i].parent >= 0) multiply34x2(outframe[i], outframe[joints[i].parent], mat);
            else copy34(outframe[i], mat);
        }
    }

    return retframe;
}

void model_render_skeleton(model_t m, mat44 M) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    if(!numjoints) return;

    ddraw_ontop_push(true);
    ddraw_color_push(RED);

    for( int joint = 0; joint < numjoints; joint++ ) {
        if( joints[joint].parent < 0) continue;

        // bone space...
        mat34 f;
        model_get_bone_pose(m, joint, &f);
        vec3 pos = vec3(f[3],f[7],f[11]);

        model_get_bone_pose(m, joints[joint].parent, &f);
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
    return model_animate_clip(m, curframe, 0, numframes-1, true);
}

static
void model_draw_call(model_t m, int shader) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    glBindVertexArray( vao );

    struct iqmtriangle *tris = NULL;
    for(int i = 0; i < nummeshes; i++) {
        struct iqmmesh *im = &meshes[i];

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[i] );
        glUniform1i(glGetUniformLocation(shader, "u_texture2d"), 0 );

        int loc;
        if ((loc = glGetUniformLocation(shader, "u_textured")) >= 0) {
            bool textured = !!textures[i] && textures[i] != texture_checker().id; // m.materials[i].layer[0].texture != texture_checker().id;
            glUniform1i(loc, textured ? GL_TRUE : GL_FALSE);
            if ((loc = glGetUniformLocation(shader, "u_diffuse")) >= 0) {
                glUniform4f(loc, m.materials[i].layer[0].color.r, m.materials[i].layer[0].color.g, m.materials[i].layer[0].color.b, m.materials[i].layer[0].color.a);
            }
        }

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m.lightmap.id);
        glUniform1i(glGetUniformLocation(shader, "u_lightmap"), 1 );

        glDrawElementsInstanced(GL_TRIANGLES, 3*im->num_triangles, GL_UNSIGNED_INT, &tris[im->first_triangle], m.num_instances);
        profile_incstat("Render.num_drawcalls", +1);
        profile_incstat("Render.num_triangles", +im->num_triangles);
    }

    glBindVertexArray( 0 );
}

void model_render_instanced(model_t m, mat44 proj, mat44 view, mat44* models, int shader, unsigned count) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    mat44 mv; multiply44x2(mv, view, models[0]);

    if( count != m.num_instances ) {
        m.num_instances = count;
        m.instanced_matrices = (float*)models;
        model_set_state(m);
    }

    model_set_uniforms(m, shader > 0 ? shader : m.program, mv, proj, view, models[0]);
    model_draw_call(m, shader > 0 ? shader : m.program);
}

void model_render(model_t m, mat44 proj, mat44 view, mat44 model, int shader) {
    model_render_instanced(m, proj, view, (mat44*)model, shader, 1);
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
    if( q && bounds ) {
    int f = ( (int)m.curframe ) % (numframes + !numframes);
    vec3 bbmin = ptr3(bounds[f].bbmin);
    vec3 bbmax = ptr3(bounds[f].bbmax);
    return aabb_transform(aabb(bbmin,bbmax), transform);
    }
    return aabb(vec3(0,0,0),vec3(0,0,0));
}

void model_destroy(model_t m) {
    FREE(m.verts);
    for( int i = 0, end = array_count(m.texture_names); i < end; ++i ) {
        FREE(m.texture_names[i]);
    }
    array_free(m.texture_names);

    iqm_t *q = m.iqm;
//    if(m.mesh) mesh_destroy(m.mesh);
    FREE(outframe);
    FREE(colormaps);
    FREE(textures);
    FREE(baseframe);
    FREE(inversebaseframe);
    if(animdata != meshdata) FREE(animdata);
    //FREE(meshdata);
    FREE(frames);
    FREE(buf);
    FREE(q);
}

#undef program
#undef meshdata
#undef animdata
#undef nummeshes
#undef numtris
#undef numverts
#undef numjoints
#undef numframes
#undef numanims
#undef meshes
#undef textures
#undef joints
#undef poses
#undef anims
#undef baseframe
#undef inversebaseframe
#undef outframe
#undef frames
#undef vao
#undef ibo
#undef vbo
#undef bonematsoffset
#undef buf
#undef bounds
#undef colormaps

anims_t animations(const char *pathfile, int flags) {
    anims_t a = {0};
    char *anim_file = vfs_read(strendi(pathfile,".txt") ? pathfile : va("%s@animlist.txt", pathfile));
    if( anim_file ) {
        // deserialize anim
        a.speed = 1.0;
        for each_substring(anim_file, "\r\n", anim) {
            int from, to;
            char anim_name[128] = {0};
            if( sscanf(anim, "%*s %d-%d %127[^\r\n]", &from, &to, anim_name) != 3) continue;
            array_push(a.anims, !!strstri(anim_name, "loop") || !strcmpi(anim_name, "idle") ? loop(from, to, 0, 0) : clip(from, to, 0, 0)); // [from,to,flags]
            array_back(a.anims)->name = strswap(strswap(strswap(STRDUP(anim_name), "Loop", ""), "loop", ""), "()", ""); // @leak
        }
    } else {
        // placeholder
        array_push(a.anims, clip(0,1,0,0));
        array_back(a.anims)->name = STRDUP("Error"); // @leak
    }
    return a;
}

// -----------------------------------------------------------------------------
// lightmapping utils
// @fixme: support xatlas uv packing, add UV1 coords to vertex model specs
lightmap_t lightmap(int hmsize, float cnear, float cfar, vec3 color, int passes, float threshold, float distmod) {
    lightmap_t lm = {0};
    lm.ctx = lmCreate(hmsize, cnear, cfar, color.x, color.y, color.z, passes, threshold, distmod);

    if (!lm.ctx) {
        PANIC("Error: Could not initialize lightmapper.\n");
        return lm;
    }

    const char *symbols[] = { "{{include-shadowmap}}", vfs_read("shaders/fs_0_0_shadowmap_lit.glsl") }; // #define RIM
    lm.shader = shader(strlerp(1,symbols,vfs_read("shaders/vs_323444143_16_3322_model.glsl")), strlerp(1,symbols,vfs_read("shaders/fs_32_4_model.glsl")), //fs,
        "att_position,att_texcoord,att_normal,att_tangent,att_instanced_matrix,,,,att_indexes,att_weights,att_vertexindex,att_color,att_bitangent,att_texcoord2","fragColor",
        va("%s", "LIGHTMAP_BAKING"));

    return lm;
}

void lightmap_destroy(lightmap_t *lm) {
    lmDestroy(lm->ctx);
    shader_destroy(lm->shader);
    //
}

void lightmap_setup(lightmap_t *lm, int w, int h) {
    lm->ready=1;
    //@fixme: prep atlas for lightmaps
    lm->w = w;
    lm->h = h;
}

void lightmap_bake(lightmap_t *lm, int bounces, void (*drawscene)(lightmap_t *lm, model_t *m, float *view, float *proj, void *userdata), void (*progressupdate)(float progress), void *userdata) {
    ASSERT(lm->ready);
    // @fixme: use xatlas to UV pack all models, update their UV1 and upload them to GPU.

    GLint cullface=0;
    glGetIntegerv(GL_CULL_FACE, &cullface);
    glDisable(GL_CULL_FACE);

    int w = lm->w, h = lm->h;
    for (int i = 0; i < array_count(lm->models); i++) {
        model_t *m = lm->models[i];
        if (m->lightmap.w != 0) {
            texture_destroy(&m->lightmap);
        }
        m->lightmap = texture_create(w, h, 4, 0, TEXTURE_LINEAR|TEXTURE_FLOAT);
        glBindTexture(GL_TEXTURE_2D, m->lightmap.id);
        unsigned char emissive[] = { 0, 0, 0, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, emissive);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    for (int b = 0; b < bounces; b++) {
        for (int i = 0; i < array_count(lm->models); i++) {
            model_t *m = lm->models[i];
            if (!m->lmdata) {
                m->lmdata = CALLOC(w*h*4, sizeof(float));
            }
            memset(m->lmdata, 0, w*h*4);
            lmSetTargetLightmap(lm->ctx, m->lmdata, w, h, 4);
            lmSetGeometry(lm->ctx, m->pivot,
                LM_FLOAT, (uint8_t*)m->verts + offsetof(iqm_vertex, position), sizeof(iqm_vertex),
                LM_FLOAT, (uint8_t*)m->verts + offsetof(iqm_vertex, normal), sizeof(iqm_vertex),
                LM_FLOAT, (uint8_t*)m->verts + offsetof(iqm_vertex, texcoord), sizeof(iqm_vertex),
                m->num_tris*3, LM_UNSIGNED_INT, m->tris);

            glDisable(GL_BLEND);
            int vp[4];
            float view[16], projection[16];
            while (lmBegin(lm->ctx, vp, view, projection))
            {
                // render to lightmapper framebuffer
                glViewport(vp[0], vp[1], vp[2], vp[3]);
                drawscene(lm, m, view, projection, userdata);
                if (progressupdate) progressupdate(lmProgress(lm->ctx));
                lmEnd(lm->ctx);
            }
        }

        // postprocess texture
        for (int i = 0; i < array_count(lm->models); i++) {
            model_t *m = lm->models[i];
            float *temp = CALLOC(w * h * 4, sizeof(float));
            for (int i = 0; i < 16; i++)
            {
                lmImageDilate(m->lmdata, temp, w, h, 4);
                lmImageDilate(temp, m->lmdata, w, h, 4);
            }
            lmImageSmooth(m->lmdata, temp, w, h, 4);
            lmImageDilate(temp, m->lmdata, w, h, 4);
            lmImagePower(m->lmdata, w, h, 4, 1.0f / 2.2f, 0x7); // gamma correct color channels
            FREE(temp);

            // save result to a file
            // if (lmImageSaveTGAf("result.tga", m->lmdata, w, h, 4, 1.0f))
            //     printf("Saved result.tga\n");
            // upload result
            glBindTexture(GL_TEXTURE_2D, m->lightmap.id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_FLOAT, m->lmdata);
            FREE(m->lmdata); m->lmdata = NULL;
        }
    }

    if (cullface) glEnable(GL_CULL_FACE);
}

