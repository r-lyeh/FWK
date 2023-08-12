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
// embedded shaders (@fixme: promote to files?)

static const char *const fs_0_0_shadowmap_lit =
    // "#version 140 // inverse() requires v140\n"
    "//" FILELINE "\n"

//  "uniform mat4 view = mat4(1.0);\n"
    "uniform vec3 lightPos = vec3(1.0);\n"
    "uniform float doTexture = 1.;\n"
#if VSMCUBE
    "uniform samplerCube shadowMap;\n" // VSMCUBE
#else
    "uniform sampler2D shadowMap;\n" // !VSMCUBE
#endif

    "struct light {\n"
    "    vec3 position; // world-space\n"
    "    vec4 diffuse;\n"
    "    vec4 specular;\n"
    "    float constantAttenuation, linearAttenuation, quadraticAttenuation;\n"
    "};\n"
    "light light0 = light(\n"
    "    lightPos,\n"
    "    vec4(1,1,1,1), // diffuse\n"
    "    vec4(1,1,1,1), // specular\n"
    "    1.0, 0.0, 0.0  // attenuation (const, linear, quad)\n"
    ");\n"

    "// From http://fabiensanglard.net/shadowmappingVSM/index.php\n"
#if VSMCUBE
    "float chebyshevUpperBound(float distance, vec3 dir) {\n"
    "    distance = distance/20 ;\n"
    "    vec2 moments = texture(shadowMap, dir).rg;\n"
#else
    "float chebyshevUpperBound(float distance, vec4 scPostW) {\n"
    "    vec2 moments = texture(shadowMap,scPostW.xy).rg;\n"
#endif
    "    // Surface is fully lit. as the current fragment is before the light occluder\n"
    "    if (distance <= moments.x)\n"
    "        return 1.0;\n"

    "    // The fragment is either in shadow or penumbra. We now use chebyshev's upperBound to check\n"
    "    // How likely this pixel is to be lit (p_max)\n"
    "    float variance = moments.y - (moments.x*moments.x);\n"
    "    //variance = max(variance, 0.000002);\n"
    "    variance = max(variance, 0.00002);\n"

    "    float d = distance - moments.x;\n"
    "    float p_max = variance / (variance + d*d);\n"

    "    return p_max;\n"
    "}\n"

    "vec4 shadowmap(in vec4 vpeye, in vec4 vneye, in vec2 uv, in vec4 sc) {\n"
#ifndef VSMCUBE
    "    return vec4(1.);\n"
#else

    "    vec3 fragment = vec3(vpeye);\n"
    "    vec3 normal   = vec3(normalize(vneye));\n"
    "    vec3 viewDir  = normalize(-fragment);\n"

    "    // Lighting\n"
    "    // Convert to eye-space\n"
    "    vec3 light = vec3(view * vec4(light0.position, 1.0));\n"

#if VSMCUBE
    "    // Vectors\n"
    "    vec3 fragmentToLight     = light - fragment;\n"
    "    vec3 fragmentToLightDir  = normalize(fragmentToLight);\n"

    "    // Shadows\n"
    "    vec4 fragmentToLight_world = inverse(view) * vec4(fragmentToLightDir, 0.0);\n"
    "    float shadowFactor = chebyshevUpperBound(length(fragmentToLight), -fragmentToLight_world.xyz);\n"
#else
    "    // Shadows\n"
    "    vec4 scPostW = sc / sc.w;\n"
    "    scPostW = scPostW * 0.5 + 0.5;\n"

    "    float shadowFactor = 1.0; // Not in shadow\n"

    "    bool outsideShadowMap = sc.w <= 0.0f || (scPostW.x < 0 || scPostW.y < 0) || (scPostW.x >= 1 || scPostW.y >= 1);\n"
    "    if (!outsideShadowMap) {\n"
    "        shadowFactor = chebyshevUpperBound(scPostW.z, scPostW);\n"
    "    }\n"
#endif

    "    vec4 diffColor = vec4(1,1,1,1);\n"
#if VSMCUBE
    "    if(doTexture != 0) diffColor = vec4(vec3(texture(shadowMap, -fragmentToLight_world.xyz).r), 1.0);\n"
#else
    "    if(doTexture != 0) diffColor = vec4(vec3(texture(shadowMap, vec2(uv.x, 1.0 - uv.y)).r), 1.0);\n"
#endif

#if 1
    "    vec3 positionToLight = light - fragment;\n"
    "    vec3 lightDir  = normalize(positionToLight);\n"

    "    // Angle between fragment-normal and incoming light\n"
    "    float cosAngIncidence = dot(lightDir, normal);\n"
    "    cosAngIncidence = clamp(cosAngIncidence, 0, 1);\n"

    "    float attenuation = 1.0f;\n"
    "    attenuation = 1.0 / (light0.constantAttenuation + light0.linearAttenuation * length(positionToLight) + light0.quadraticAttenuation * pow(length(positionToLight),2));\n"

    "    vec4 diffuse  = diffColor * light0.diffuse  * cosAngIncidence * attenuation;\n"

    "    vec4 total_lighting;\n"
    "    total_lighting += vec4(0.1, 0.1, 0.1, 1.0) * diffColor; // Ambient\n"
    "    total_lighting += diffuse * shadowFactor; // Diffuse\n"
#else
    "    vec4 total_lighting = diffColor;\n"
#endif
    "    return vec4(clamp(vec3(total_lighting), 0., 1.), 1.0);\n"
#endif
    "}\n";


static const char *const fs_0_0_shadowmap_unlit = "//" FILELINE "\n"
//  "uniform mat4 view = mat4(1.0);\n"
    "uniform vec3 lightPos = vec3(1.0);\n"
    "uniform float doTexture = 0.;\n"
    "uniform sampler2D shadowMap;\n"

    "vec4 shadowmap(in vec4 vpeye, in vec4 vneye, in vec2 Texcoord, in vec4 sc) {\n"
    "    return vec4(1.);\n"
    "};\n";


static const char *const vs_3_3_skybox = "//" FILELINE "\n"
    "uniform mat4 u_mvp;\n"

    "in vec3 att_position;\n"
    "out vec3 v_direction;\n"

    "void main() {\n"
    "    vec4 position = u_mvp * vec4(att_position, 0.0);\n"
    "    gl_Position = position.xyww;\n"
    "    v_direction = att_position;\n"
    "}\n";

static const char *const fs_3_4_skybox = "//" FILELINE "\n"
    "uniform samplerCube u_cubemap;\n"

    "in vec3 v_direction;\n"
    "out vec4 fragcolor;\n"

    "void main() {\n"
    "    fragcolor = vec4(texture(u_cubemap, v_direction).rgb, 1.0);\n"
    "}\n";

static const char *const fs_3_4_skybox_rayleigh = "//" FILELINE "\n"
    "uniform vec3 uSunPos = vec3( 0, 0.1, -1 ); // = [0, Math.cos(theta) * 0.3 + 0.2, -1];\n"

    "in vec3 v_direction;\n"
    "out vec4 fragcolor;\n"

    "vec3 atmosphere(vec3 r, vec3 r0, vec3 pSun, float iSun, float rPlanet, float rAtmos, vec3 kRlh, float kMie, float shRlh, float shMie, float g);\n"

    "void main() {\n"
    "    vec3 color = atmosphere(\n"
    "        normalize(v_direction),         // normalized ray direction\n"
    "        vec3(0,6372e3,0),               // ray origin\n"
    "        uSunPos,                        // position of the sun\n"
    "        22.0,                           // intensity of the sun\n"
    "        6371e3,                         // radius of the planet in meters\n"
    "        6471e3,                         // radius of the atmosphere in meters\n"
    "        vec3(5.5e-6, 13.0e-6, 22.4e-6), // Rayleigh scattering coefficient\n"
    "        21e-6,                          // Mie scattering coefficient\n"
    "        8e3,                            // Rayleigh scale height\n"
    "        1.2e3,                          // Mie scale height\n"
    "        0.758                           // Mie preferred scattering direction\n"
    "    );\n"

    "    // Apply exposure.\n"
    "    color = 1.0 - exp(-1.0 * color);\n"

    "    fragcolor = vec4(color, 1);\n"
    "}\n"

    "// [src] https://github.com/wwwtyro/glsl-atmosphere by wwwtyro (Unlicensed)\n"
    "// For more information, please refer to <http://unlicense.org>\n"

    "#define PI 3.141592\n"
    "#define iSteps 16\n"
    "#define jSteps 8\n"

    "vec2 rsi(vec3 r0, vec3 rd, float sr) {\n"
    "    // ray-sphere intersection that assumes\n"
    "    // the sphere is centered at the origin.\n"
    "    // No intersection when result.x > result.y\n"
    "    float a = dot(rd, rd);\n"
    "    float b = 2.0 * dot(rd, r0);\n"
    "    float c = dot(r0, r0) - (sr * sr);\n"
    "    float d = (b*b) - 4.0*a*c;\n"
    "    if (d < 0.0) return vec2(1e5,-1e5);\n"
    "    return vec2(\n"
    "        (-b - sqrt(d))/(2.0*a),\n"
    "        (-b + sqrt(d))/(2.0*a)\n"
    "    );\n"
    "}\n"

    "vec3 atmosphere(vec3 r, vec3 r0, vec3 pSun, float iSun, float rPlanet, float rAtmos, vec3 kRlh, float kMie, float shRlh, float shMie, float g) {\n"
    "    // Normalize the sun and view directions.\n"
    "    pSun = normalize(pSun);\n"
    "    r = normalize(r);\n"

    "    // Calculate the step size of the primary ray.\n"
    "    vec2 p = rsi(r0, r, rAtmos);\n"
    "    if (p.x > p.y) return vec3(0,0,0);\n"
    "    p.y = min(p.y, rsi(r0, r, rPlanet).x);\n"
    "    float iStepSize = (p.y - p.x) / float(iSteps);\n"

    "    // Initialize the primary ray time.\n"
    "    float iTime = 0.0;\n"

    "    // Initialize accumulators for Rayleigh and Mie scattering.\n"
    "    vec3 totalRlh = vec3(0,0,0);\n"
    "    vec3 totalMie = vec3(0,0,0);\n"

    "    // Initialize optical depth accumulators for the primary ray.\n"
    "    float iOdRlh = 0.0;\n"
    "    float iOdMie = 0.0;\n"

    "    // Calculate the Rayleigh and Mie phases.\n"
    "    float mu = dot(r, pSun);\n"
    "    float mumu = mu * mu;\n"
    "    float gg = g * g;\n"
    "    float pRlh = 3.0 / (16.0 * PI) * (1.0 + mumu);\n"
    "    float pMie = 3.0 / (8.0 * PI) * ((1.0 - gg) * (mumu + 1.0)) / (pow(1.0 + gg - 2.0 * mu * g, 1.5) * (2.0 + gg));\n"

    "    // Sample the primary ray.\n"
    "    for (int i = 0; i < iSteps; i++) {\n"

    "        // Calculate the primary ray sample position.\n"
    "        vec3 iPos = r0 + r * (iTime + iStepSize * 0.5);\n"

    "        // Calculate the height of the sample.\n"
    "        float iHeight = length(iPos) - rPlanet;\n"

    "        // Calculate the optical depth of the Rayleigh and Mie scattering for this step.\n"
    "        float odStepRlh = exp(-iHeight / shRlh) * iStepSize;\n"
    "        float odStepMie = exp(-iHeight / shMie) * iStepSize;\n"

    "        // Accumulate optical depth.\n"
    "        iOdRlh += odStepRlh;\n"
    "        iOdMie += odStepMie;\n"

    "        // Calculate the step size of the secondary ray.\n"
    "        float jStepSize = rsi(iPos, pSun, rAtmos).y / float(jSteps);\n"

    "        // Initialize the secondary ray time.\n"
    "        float jTime = 0.0;\n"

    "        // Initialize optical depth accumulators for the secondary ray.\n"
    "        float jOdRlh = 0.0;\n"
    "        float jOdMie = 0.0;\n"

    "        // Sample the secondary ray.\n"
    "        for (int j = 0; j < jSteps; j++) {\n"

    "            // Calculate the secondary ray sample position.\n"
    "            vec3 jPos = iPos + pSun * (jTime + jStepSize * 0.5);\n"

    "            // Calculate the height of the sample.\n"
    "            float jHeight = length(jPos) - rPlanet;\n"

    "            // Accumulate the optical depth.\n"
    "            jOdRlh += exp(-jHeight / shRlh) * jStepSize;\n"
    "            jOdMie += exp(-jHeight / shMie) * jStepSize;\n"

    "            // Increment the secondary ray time.\n"
    "            jTime += jStepSize;\n"
    "        }\n"

    "        // Calculate attenuation.\n"
    "        vec3 attn = exp(-(kMie * (iOdMie + jOdMie) + kRlh * (iOdRlh + jOdRlh)));\n"

    "        // Accumulate scattering.\n"
    "        totalRlh += odStepRlh * attn;\n"
    "        totalMie += odStepMie * attn;\n"

    "        // Increment the primary ray time.\n"
    "        iTime += iStepSize;\n"

    "    }\n"

    "    // Calculate and return the final color.\n"
    "    return iSun * (pRlh * kRlh * totalRlh + pMie * kMie * totalMie);\n"
    "}\n";


static const char *const vs_332_32 = "//" FILELINE "\n"
    //"uniform mat4 u_model, u_view, u_proj;\n"
    "uniform mat4 u_mvp;\n"

    "in vec3 att_position;\n"
    "in vec3 att_normal;\n"
    "in vec2 att_texcoord;\n"
    "in vec4 att_color;\n"
    "out vec4 v_color;\n"
    "out vec3 v_normal;\n"
    "out vec3 v_normal_ws;\n"
    "out vec2 v_texcoord;\n"

        // shadow
        "uniform mat4 model, view, proj;\n"
        "uniform mat4 cameraToShadowProjector;\n" // !VSMCUBE
        "out vec4 vneye;\n"
        "out vec4 vpeye;\n"
        "out vec4 sc;\n" // !VSMCUBE
        "void do_shadow() {\n"
        "    vneye = view * model * vec4(att_normal,   0.0f);\n"
        "    vpeye = view * model * vec4(att_position, 1.0);\n"
        "    sc = cameraToShadowProjector * model * vec4(att_position, 1.0f);\n"
        "}\n"

    "void main() {\n"
    //"    gl_Position = proj * view * model * vec4(att_position, 1.0);\n"
    "    gl_Position = u_mvp * vec4(att_position, 1.0);\n"
    "    v_normal = normalize(att_normal);\n"
    "    v_normal_ws = normalize(vec3(model * vec4(att_normal, 0.)));\n" // normal world/model space
    "    v_texcoord = att_texcoord;\n"
    "    v_color = att_color;\n"
    "    do_shadow();\n"
    "}";

static const char *const vs_0_2_fullscreen_quad_A = "//" FILELINE "\n"
    "out vec2 texcoord;\n"

    "void main() {\n"
    "   texcoord = vec2( (gl_VertexID << 1) & 2, gl_VertexID & 2 );\n"
    "   gl_Position = vec4( texCoord * 2.0 - 1.0, 0.0, 1.0 );\n"
    "}\n";

static const char *const vs_0_2_fullscreen_quad_B = "//" FILELINE "\n"
    "out vec2 uv;\n"

    "void main() {\n"
    "    float x = float(((uint(gl_VertexID) + 2u) / 3u)%2u); \n"
    "    float y = float(((uint(gl_VertexID) + 1u) / 3u)%2u); \n"
    "    gl_Position = vec4(-1.0 + x*2.0, 0.0+(-1.0+y*2.0), 0.0, 1.0);\n" // normal(0+),flipped(0-)
    "    uv = vec2(x, y);\n" // normal(y),flipped(1.0-y)
    "}\n";

static const char *const vs_0_2_fullscreen_quad_B_flipped = "//" FILELINE "\n"
    "out vec2 uv;\n"

    "void main() {\n"
    "    float x = float(((uint(gl_VertexID) + 2u) / 3u)%2u); \n"
    "    float y = float(((uint(gl_VertexID) + 1u) / 3u)%2u); \n"
    "    gl_Position = vec4(-1.0 + x*2.0, 0.0-(-1.0+y*2.0), 0.0, 1.0);\n" // normal(0+),flipped(0-)
    "    uv = vec2(x, y);\n" // normal(y),flipped(1.0-y)
    "}\n";

/*
    "out vec2 uv;\n"
    "void main() {\n"
    "   float x = gl_VertexID / 2;\n"
    "   float y = gl_VertexID % 2;\n"
    "   uv = vec2(x, y);\n"
    "   gl_Position = vec4(2.0*uv - 1.0, 0.0, 1.0);\n"
    "}\n";
*/
static const char *const fs_2_4_texel_inv_gamma = "//" FILELINE "\n"
    "uniform sampler2D texture0; /*unit0*/\n"
    "uniform float u_inv_gamma;\n"

    "in vec2 uv;\n"
    "out vec4 fragcolor;\n"

    "void main() {\n"
    "    vec4 texel = texture( texture0, uv );\n"
    "    fragcolor = texel;\n"
    "    fragcolor.rgb = pow( fragcolor.rgb, vec3( u_inv_gamma ) );\n" // defaults: 1.0/2.2 gamma
    "}\n";

// vertex stride = 4*(3+2+3+4+4+4+1+4+3) = 112 bytes + 16 bytes/instance
static const char *const vs_323444143_16_332_model = "//" FILELINE "\n"
    "#ifndef MAX_BONES\n"
    "#define MAX_BONES 110\n"
    "#endif\n"
    "uniform mat3x4 vsBoneMatrix[MAX_BONES];\n"
    "uniform bool SKINNED = false;\n"
    // "uniform mat4 M;\n" // RIM
    "uniform mat4 VP;\n"

#if 0
    // Fetch blend channels from all attached blend deformers.
    for (size_t di = 0; di < mesh->blend_deformers.count; di++) {
        ufbx_blend_deformer *deformer = mesh->blend_deformers.data[di];
        for (size_t ci = 0; ci < deformer->channels.count; ci++) {
            ufbx_blend_channel *chan = deformer->channels.data[ci];
            if (chan->keyframes.count == 0) continue;
            if (num_blend_shapes < MAX_BLEND_SHAPES) {
                blend_channels[num_blend_shapes] = chan;
                vmesh->blend_channel_indices[num_blend_shapes] = (int32_t)chan->typed_id;
                num_blend_shapes++;
            }
        }
    }
    if (num_blend_shapes > 0) {
        vmesh->blend_shape_image = pack_blend_channels_to_image(mesh, blend_channels, num_blend_shapes);
        vmesh->num_blend_shapes = num_blend_shapes;
    }

    ubo.f_num_blend_shapes = (float)mesh->num_blend_shapes;
    for (size_t i = 0; i < mesh->num_blend_shapes; i++) {
        ubo.blend_weights[i] = view->scene.blend_channels[mesh->blend_channel_indices[i]].weight;
    }

    sg_image blend_shapes = mesh->num_blend_shapes > 0 ? mesh->blend_shape_image : view->empty_blend_shape_image;
#endif

    // for blendshapes
    "#ifndef MAX_BLENDSHAPES\n"
    "#define MAX_BLENDSHAPES 16\n"
    "#endif\n"
    "uniform vec4 blend_weights[MAX_BLENDSHAPES];\n" // @todo: implement me
    "uniform float f_num_blend_shapes;\n" // @todo: implement me
    "uniform sampler2DArray blend_shapes;\n" // @todo: implement me

    "in vec3 att_position;\n" // @todo: reorder ass2iqe to emit p3 n3 u2 t3 b3 c4B i4 w4 instead
    "in vec2 att_texcoord;\n"
    "in vec3 att_normal;\n"
    "in vec4 att_tangent;\n" // vec3 + bi sign
    "in mat4 att_instanced_matrix;\n" // for instanced rendering
    "in vec4 att_indexes;\n" // @fixme: gles might use ivec4 instead?
    "in vec4 att_weights;\n" // @todo: downgrade from float to byte
    "in float att_vertexindex;\n" // for blendshapes
    "in vec4 att_color;\n"
    "in vec3 att_bitangent;\n" // @todo: remove? also, ass2iqe might output this
    "out vec4 v_color;\n"
    "out vec3 v_position;\n"
    "out vec3 v_normal, v_normal_ws;\n"
    "out vec2 v_texcoord;\n"


        // shadow
        "uniform mat4 model, view;\n"
        "uniform mat4 cameraToShadowProjector;\n"
        "out vec4 vneye;\n"
        "out vec4 vpeye;\n"
        "out vec4 sc;\n"
        "void do_shadow() {\n"
        "    vneye = view * model * vec4(att_normal,   0.0f);\n"
        "    vpeye = view * model * vec4(att_position, 1.0);\n"
        "    sc = cameraToShadowProjector * model * vec4(att_position, 1.0f);\n"
        "}\n"

// blendshapes
"vec3 evaluate_blend_shape(int vertex_index) {\n"
"    ivec2 coord = ivec2(vertex_index & (2048 - 1), vertex_index >> 11);\n"
"    int num_blend_shapes = int(f_num_blend_shapes);\n"
"    vec3 offset = vec3(0.0);\n"
"    for (int i = 0; i < num_blend_shapes; i++) {\n"
"        vec4 packedw = blend_weights[i >> 2];\n"
"        float weight = packedw[i & 3];\n"
"        offset += weight * texelFetch(blend_shapes, ivec3(coord, i), 0).xyz;\n"
"    }\n"
"    return offset;\n"
"}\n"

    "void main() {\n"
    "   vec3 objPos;\n"
    "   if(!SKINNED) {\n"
    "       objPos = att_position;\n"
    "       v_normal = att_normal;\n"
    "   } else {\n"
    "       mat3x4 m = vsBoneMatrix[int(att_indexes.x)] * att_weights.x;\n"
    "       m += vsBoneMatrix[int(att_indexes.y)] * att_weights.y;\n"
    "       m += vsBoneMatrix[int(att_indexes.z)] * att_weights.z;\n"
    "       m += vsBoneMatrix[int(att_indexes.w)] * att_weights.w;\n"
    "       objPos = vec4(att_position, 1.0) * m;\n"

// blendshapes
// "objPos += evaluate_blend_shape(int(att_vertexindex));\n"

    "       v_normal = vec4(att_normal, 0.0) * m;\n"
    "       //@todo: tangents\n"
    "   }\n"

    //"   vec3 tangent = att_tangent.xyz;\n"
    //"   vec3 bitangent = cross(att_normal, att_tangent.xyz) * att_tangent.w;

    "   v_normal_ws = normalize(vec3(model * vec4(v_normal, 0.)));\n" // normal to world/model space
    "   v_normal = normalize(v_normal);\n"
    "   v_position = att_position;\n"
    "   v_texcoord = att_texcoord;\n"
    "   v_color = att_color;\n"
    "   gl_Position = VP * att_instanced_matrix * vec4( objPos, 1.0 );\n"
    "   do_shadow();\n"
    "}\n";

#if 0
static const char *const fs_32_4_model_basic = "//" FILELINE "\n"
    "uniform sampler2D fsDiffTex;\n"
    "uniform sampler2D fsNormalTex;\n"
    "uniform sampler2D fsPositionTex;\n"
    "uniform mat4 MVP;\n"

    "in vec3 v_normal;\n"
    "in vec2 v_texcoord;\n"
    "out vec4 fragColor;\n"

    "void main() {\n"
    "    vec4 diff = texture(fsDiffTex, v_texcoord).rgba;\n"
    "    vec3 n = normalize(mat3(MVP) * v_normal); // transform normal to eye space\n"
    "    fragColor = diff;// * vec4(v_normal.xyz, 1);\n"
    "}\n";
#endif

static const char *const fs_32_4_model = "//" FILELINE "\n"
    "uniform mat4 model, view;\n"
    "uniform sampler2D u_texture2d;\n"
    "uniform vec3 u_coefficients_sh[9];\n"
    "uniform bool u_textured = true;\n"
    "uniform bool u_lit = false;\n"
    "uniform bool u_matcaps = false;\n"
    "uniform vec4 u_diffuse = vec4(1.0,1.0,1.0,1.0);\n"

    "#ifdef RIM\n"
    "in vec3 v_position;\n"
    "#endif\n"
    "in vec3 v_normal, v_normal_ws;\n"
    "in vec2 v_texcoord;\n"
    "in vec4 v_color;\n"
    "out vec4 fragcolor;\n"

        "{{include-shadowmap}}\n"
        "in vec4 vpeye;\n"
        "in vec4 vneye;\n"
        "in vec4 sc;\n"
        "vec4 shadowing() {\n"
        "    return shadowmap(vpeye, vneye, v_texcoord, sc);\n"
        "}\n"

    "void main() {\n"
    "    vec3 n = /*normalize*/(v_normal);\n"

    // SH lighting
    "    vec4 lit = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "        vec3 SHLightResult[9];\n"
    "        SHLightResult[0] =  0.282095f * u_coefficients_sh[0];\n"
    "        SHLightResult[1] = -0.488603f * u_coefficients_sh[1] * n.y;\n"
    "        SHLightResult[2] =  0.488603f * u_coefficients_sh[2] * n.z;\n"
    "        SHLightResult[3] = -0.488603f * u_coefficients_sh[3] * n.x;\n"
    "        SHLightResult[4] =  1.092548f * u_coefficients_sh[4] * n.x * n.y;\n"
    "        SHLightResult[5] = -1.092548f * u_coefficients_sh[5] * n.y * n.z;\n"
    "        SHLightResult[6] =  0.315392f * u_coefficients_sh[6] * (3.0f * n.z * n.z - 1.0f);\n"
    "        SHLightResult[7] = -1.092548f * u_coefficients_sh[7] * n.x * n.z;\n"
    "        SHLightResult[8] =  0.546274f * u_coefficients_sh[8] * (n.x * n.x - n.y * n.y);\n"
    "        vec3 result = vec3(0.0);\n"
    "        for (int i = 0; i < 9; ++i)\n"
    "            result += SHLightResult[i];\n"
    "    if( (result.x*result.x+result.y*result.y+result.z*result.z) > 0.0 ) lit = vec4(result, 1.0);\n"
    "\n"

    // base
    "    vec4 diffuse;\n"
    "    if(u_matcaps) {\n"
    "        vec2 muv = vec2(view * vec4(v_normal_ws, 0))*0.5+vec2(0.5,0.5);\n" // normal (model space) to view space
    "        diffuse = texture(u_texture2d, vec2(muv.x, 1.0-muv.y));\n"
    "    } else if(u_textured) {\n"
    "        diffuse = texture(u_texture2d, v_texcoord);\n"
    "    } else {\n"
    "        diffuse = u_diffuse; // * v_color;\n"
    "    }\n"

    // lighting mix
    "    fragcolor = diffuse * lit * shadowing();\n"

    // rimlight
    "#ifdef RIM\n"
    "   {vec3 n = normalize(mat3(M) * v_normal);  // convert normal to view space\n"
    "    vec3 p = (M * vec4(v_position,1.0)).xyz; // convert position to view space\n"
    "    vec3 v = normalize(-p);                  // eye vector\n"
    "    float rim = 1.0 - max(dot(v, n), 0.0);   // rimlight\n"
    "    rim = smoothstep(1.0-0.01, 1.0, rim);    // intensity (0.01)\n"
    "    fragcolor += vec4(0.0, 0.0, rim, 1.0);}  // blue\n"
    "#endif\n"

    "}\n";


static const char *const fs_2_4_texel_ycbr_gamma_saturation = "//" FILELINE "\n"
    "uniform sampler2D u_texture_y;  /*unit0*/\n"
    "uniform sampler2D u_texture_cb; /*unit1*/\n"
    "uniform sampler2D u_texture_cr; /*unit2*/\n"
    "uniform float u_gamma;\n"

    "in vec2 uv;\n"
    "out vec4 fragcolor;\n"

    "void main() {\n"
    "    float y = texture(u_texture_y, uv).r;\n"
    "    float cb = texture(u_texture_cb, uv).r;\n"
    "    float cr = texture(u_texture_cr, uv).r;\n"

    "    const mat4 to_rgb = mat4(\n"
    "       1.0000, 1.0000, 1.0000, 0.0000,\n"
    "       0.0000, -0.3441, 1.7720, 0.0000,\n"
    "       1.4020, -0.7141, 0.0000, 0.0000,\n"
    "       -0.7010, 0.5291, -0.8860, 1.0000\n"
    "    );\n"
    "    vec4 texel = to_rgb * vec4(y, cb, cr, 1.0);\n"
    /* same as:
    "    vec3 yCbCr = vec3(y,cb-0.5,cr-0.5);\n"
    "    vec4 texel = vec4(  dot( vec3( 1.0,  0.0,      1.402 ),    yCbCr ),\n"
    "                        dot( vec3( 1.0 , -0.34414 , -0.71414 ), yCbCr ),\n"
    "                        dot( vec3( 1.0,  1.772,    0.0 ),      yCbCr ), 1.0);\n"
    */
    "    // gamma correction\n"
    "    texel.rgb = pow(texel.rgb, vec3(1.0 / u_gamma));\n"

    "   // saturation (algorithm from Chapter 16 of OpenGL Shading Language)\n"
    "   if(false) { float saturation = 2.0; const vec3 W = vec3(0.2125, 0.7154, 0.0721);\n"
    "   vec3 intensity = vec3(dot(texel.rgb, W));\n"
    "   texel.rgb = mix(intensity, texel.rgb, saturation); }\n"

    "    fragcolor = vec4(texel.rgb, 1.0);\n"
    "}\n";

static const char *const vs_324_24_sprite = "//" FILELINE "\n"
    "uniform mat4 u_mvp;\n"

    "in vec3 att_Position;\n"
    "in vec2 att_TexCoord;\n"
    "in vec4 att_Color;\n"
    "out vec2 vTexCoord;\n"
    "out vec4 vColor;\n"

    "void main() {\n"
    "    vColor = att_Color;\n"
    "    vTexCoord = att_TexCoord;\n"
    "    gl_Position = u_mvp * vec4(att_Position, 1.0);\n"
    "}\n";

static const char *const fs_24_4_sprite = "//" FILELINE "\n"
    "uniform sampler2D u_texture;\n"

    "in vec2 vTexCoord;\n"
    "in vec4 vColor;\n"
    "out vec4 fragColor;\n"

    "// [src] https://www.shadertoy.com/view/MllBWf CC1.0\n"
    "vec4 texture_AA(sampler2D tx, vec2 uv) {\n"
    "    vec2 res = vec2(textureSize(tx, 0));\n"
    "    uv = uv*res + 0.5;\n"
    "    // tweak fractionnal value of the texture coordinate\n"
    "    vec2 fl = floor(uv);\n"
    "    vec2 fr = fract(uv);\n"
    "    vec2 aa = fwidth(uv)*0.75;\n"
    "    fr = smoothstep( vec2(0.5)-aa, vec2(0.5)+aa, fr);\n"
    "    // return value\n"
    "    uv = (fl+fr-0.5) / res;\n"
    "    return texture(tx, uv);\n"
    "}\n"

    "// [src] https://www.shadertoy.com/view/MllBWf CC1.0\n"
    "vec4 texture_AA2( sampler2D tex, vec2 uv) {\n"
    "    vec2 res = vec2(textureSize(tex,0));\n"
    "    uv = uv*res;\n"
    "    vec2 seam = floor(uv+0.5);\n"
    "    uv = seam + clamp( (uv-seam)/fwidth(uv), -0.5, 0.5);\n"
    "    return texture(tex, uv/res);\n"
    "}\n"

    "// [src] https://www.shadertoy.com/view/ltBfRD\n"
    "vec4 texture_AA3(sampler2D tex, vec2 uv) {\n"
    "    vec2 res = vec2(textureSize(tex,0));\n"
    "    float width = 2.0;\n"
    "    uv = uv * res;\n"
    "    // ---\n"
    "    vec2 uv_floor = floor(uv + 0.5);\n"
    "    vec2 uv_fract = fract(uv + 0.5);\n"
    "    vec2 uv_aa = fwidth(uv) * width * 0.5;\n"
    "    uv_fract = smoothstep(\n"
    "        vec2(0.5) - uv_aa,\n"
    "        vec2(0.5) + uv_aa,\n"
    "        uv_fract\n"
    "        );\n"
    "    uv = (uv_floor + uv_fract - 0.5) / res;\n"
    "    return texture(tex, uv);\n"
    "}\n"

    "void main() {\n"
    "    vec4 texColor = texture_AA2(u_texture, vTexCoord);\n"
    "if(texColor.a < 0.9) discard;"
    "    fragColor = vColor * texColor;\n"
    "}\n";

static const char *const fs_2_4_preamble = "//" FILELINE "\n"
    "#define texture2D texture\n"
    "#define texture2DLod textureLod\n"
    "#define FRAGCOLOR fragColor\n"
    "#define texcoord uv\n"
    "#define TEXCOORD uv\n"
    "uniform sampler2D iChannel0;\n"
    "uniform sampler2D iChannel1;\n"
    "uniform float iWidth, iHeight, iTime, iFrame, iMousex, iMousey;\n"
    "uniform float iChannelRes0x, iChannelRes0y;\n"
    "uniform float iChannelRes1x, iChannelRes1y;\n"
    "vec2 iResolution = vec2(iWidth, iHeight);\n"
    "vec2 iMouse = vec2(iMousex, iMousey);\n"
    "vec2 iChannelResolution[2] = vec2[2]( vec2(iChannelRes0x, iChannelRes0y),vec2(iChannelRes1x, iChannelRes1y) );\n"
    "float iGlobalTime = iTime;\n"
    "in vec2 texcoord;\n"
    "out vec4 fragColor;\n";

static const char *const fs_main_shadertoy = "//" FILELINE "\n"
    "void mainImage( out vec4 fragColor, in vec2 fragCoord );\n"
    "void main() {\n"
    "   mainImage(fragColor, texcoord.xy * iResolution);\n"
    "}\n";

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
        PANIC("ERROR: shader_compile(): %s\n%s\n", type == GL_VERTEX_SHADER ? "Vertex" : "Fragment", buf);
        return 0;
    }

    return shader;
}



unsigned shader(const char *vs, const char *fs, const char *attribs, const char *fragcolor) {
    PRINTF(/*"!"*/"Compiling shader\n");

    //char *vs = vfs_read(file_vs); if(!vs) vs = (char*)file_vs;
    //char *fs = vfs_read(file_fs); if(!fs) fs = (char*)file_fs;

    const char *glsl_version = ifdef(ems, "300 es", "150");

    vs = vs[0] == '#' && vs[1] == 'v' ? vs : va("#version %s\n%s", glsl_version, vs ? vs : "");
    fs = fs[0] == '#' && fs[1] == 'v' ? fs : va("#version %s\n%s", glsl_version, fs ? fs : "");

#if is(ems)
    {
        char *vs_ = REALLOC( 0, strlen(vs) + 512 ); strcpy(vs_, vs);
        char *fs_ = REALLOC( 0, strlen(fs) + 512 ); strcpy(fs_, fs);
        //strrepl(&vs_, "\nin ", "\nattribute ");
        //strrepl(&vs_, "\nout ", "\nvarying ");
        strrepl(&fs_, "#version 300 es\n", "#version 300 es\nprecision mediump float;\n");
        //strrepl(&fs_, "\nin ", "\nattribute ");
        //strrepl(&fs_, "\nout ", "\nvarying ");
        //strrepl(&fs_, "FRAGCOLOR", "gl_FragColor");
        //strrepl(&fs_, "fragcolor", "gl_FragColor" );
        //strrepl(&fs_, "fragColor", "gl_FragColor" );
        #if 0
        //strrepl(&fs_, "outcolor", "gl_FragColor" );
        //strrepl(&fs_, "outColor", "gl_FragColor" );
        #endif
        //strrepl(&fs_, "out vec4 gl_FragColor", "//out vec4 outcolor");
        vs = vs_; fs = fs_;
    }
#endif

    GLuint vert = shader_compile(GL_VERTEX_SHADER, vs);
    GLuint frag = shader_compile(GL_FRAGMENT_SHADER, fs);
  //GLuint geom = shader_compile(GL_GEOMETRY_SHADER, gs);
    GLuint program = 0;

    if( vert && frag ) {
        program = glCreateProgram();

        glAttachShader(program, vert);
        glAttachShader(program, frag);
        // glAttachShader(program, geom);

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
        }
        if (status == GL_FALSE) {
            PANIC("ERROR: shader(): Shader/program link: %s\n", buf);
            return 0;
        }

        // glDetachShader(program, vert);
        // glDetachShader(program, frag);
        // glDetachShader(program, geom);

        glDeleteShader(vert);
        glDeleteShader(frag);
        // glDeleteShader(geom);

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

    return program;
}

void shader_destroy(unsigned program){
    if( program == ~0u ) return;
    glDeleteProgram(program);
// if(s->name) FREE(s->name), s->name = NULL;
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

void shader_colormap(const char *name, colormap_t c ) {
    // assumes shader uses `struct { vec4 color; bool has_tex } name + sampler2D name_tex;`
    shader_vec4( va("%s.color", name), c.color );
    shader_bool( va("%s.has_tex", name), c.texture != NULL );
    if( c.texture ) shader_texture( va("%s_tex", name), *c.texture );
}

// -----------------------------------------------------------------------------
// colors

unsigned rgba( uint8_t r, uint8_t g, uint8_t b, uint8_t a ) {
    return (unsigned)a << 24 | r << 16 | g << 8 | b;
}
unsigned bgra( uint8_t b, uint8_t g, uint8_t r, uint8_t a ) {
    return rgba(r,g,b,a);
}
float alpha( unsigned rgba ) {
    return ( rgba >> 24 ) / 255.f;
}

unsigned rgbaf(float r, float g, float b, float a) {
    return rgba(r * 255, g * 255, b * 255, a * 255);
}
unsigned bgraf(float b, float g, float r, float a) {
    return rgba(r * 255, g * 255, b * 255, a * 255);
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
    GLuint texel_type = pixel_types[ n + 5 * !!(flags & TEXTURE_FLOAT) ];
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
#else
                extern const uint32_t secret_palette[32];
                uint32_t lum = (x^y) & 8 ? 128 : (x^y) & 128 ? 192 : 255;
                uint32_t rgb = rgba(lum,lum,lum,255);
                pixels[i++] = rgb;
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
        const char* vs = vs_0_2_fullscreen_quad_B_flipped;
        const char* fs = fs_2_4_texel_inv_gamma;

        program = shader(vs, fs, "", "fragcolor" );
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
        const char* vs = vs_0_2_fullscreen_quad_B_flipped;
        const char* fs = fs_2_4_texel_ycbr_gamma_saturation;

        program = shader(vs, fs, "", "fragcolor" );
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

// ----------------------------------------------------------------------------
// sprites

typedef struct sprite_t {
    float px, py, pz;         // origin x, y, depth
    float ox, oy, cos, sin;   // offset x, offset y, cos/sin of rotation degree
    float sx, sy;             // scale x,y
    uint32_t rgba;            // vertex color
    float cellw, cellh;       // dimensions of any cell in spritesheet

    union {
    struct {
        int frame, ncx, ncy;      // frame in a (num cellx, num celly) spritesheet
    };
    struct {
        float x, y, w, h;         // normalized[0..1] within texture bounds
    };
    };
} sprite_t;

// sprite batching
typedef struct batch_t { array(sprite_t) sprites; mesh_t mesh; int dirty; } batch_t;
typedef map(int, batch_t) batch_group_t; // mapkey is anything that forces a flush. texture_id for now, might be texture_id+program_id soon

// sprite stream
typedef struct sprite_vertex { vec3 pos; vec2 uv; uint32_t rgba; } sprite_vertex;
typedef struct sprite_index  { GLuint triangle[3]; } sprite_index;

#define sprite_vertex(...) C_CAST(sprite_vertex, __VA_ARGS__)
#define sprite_index(...)  C_CAST(sprite_index, __VA_ARGS__)

// sprite impl
static int sprite_count = 0;
static int sprite_program = -1;
static array(sprite_index)  sprite_indices = 0;
static array(sprite_vertex) sprite_vertices = 0;
static batch_group_t sprite_additive_group = {0}; // (w/2,h/2) centered
static batch_group_t sprite_translucent_group = {0}; // (w/2,h/2) centered
static batch_group_t sprite_00_translucent_group = {0}; // (0,0) centered

void sprite( texture_t texture, float position[3], float rotation, uint32_t color ) {
    float offset[2] = {0,0}, scale[2] = {1,1}, spritesheet[3] = {0,0,0};
    sprite_sheet( texture, spritesheet, position, rotation, offset, scale, 0, color, false );
}

// rect(x,y,w,h) is [0..1] normalized, z-index, pos(x,y,scale), rotation (degrees), color (rgba)
void sprite_rect( texture_t t, vec4 rect, float zindex, vec3 pos, float tilt_deg, unsigned tint_rgba) {
    // @todo: no need to queue if alpha or scale are zero
    sprite_t s = {0};

    s.x = rect.x, s.y = rect.y, s.w = rect.z, s.h = rect.w;
    s.cellw = s.w * t.w, s.cellh = s.h * t.h;

    s.px = pos.x, s.py = pos.y, s.pz = zindex;
    s.sx = s.sy = pos.z;
    s.rgba = tint_rgba;
    s.ox = 0/*ox*/ * s.sx;
    s.oy = 0/*oy*/ * s.sy;
    if( tilt_deg ) {
        tilt_deg = (tilt_deg + 0) * ((float)C_PI / 180);
        s.cos = cosf(tilt_deg);
        s.sin = sinf(tilt_deg);
    } else {
        s.cos = 1;
        s.sin = 0;
    }

    batch_group_t *batches = &sprite_00_translucent_group;
    batch_t *found = map_find_or_add(*batches, t.id, (batch_t){0});

    array_push(found->sprites, s);
}

void sprite_sheet( texture_t texture, float spritesheet[3], float position[3], float rotation, float offset[2], float scale[2], int is_additive, uint32_t rgba, int resolution_independant) {
    const float px = position[0], py = position[1], pz = position[2];
    const float ox = offset[0], oy = offset[1], sx = scale[0], sy = scale[1];
    const float frame = spritesheet[0], xcells = spritesheet[1], ycells = spritesheet[2];

    if (frame < 0) return;
    if (frame > 0 && frame >= (xcells * ycells)) return;

    // no need to queue if alpha or scale are zero
    if( sx && sy && alpha(rgba) ) {
        vec3 bak = camera_get_active()->position;
        if( resolution_independant ) { // @todo: optimize me
        sprite_flush();
        camera_get_active()->position = vec3(window_width()/2,window_height()/2,1);
        }

        sprite_t s;
        s.px = px;
        s.py = py;
        s.pz = pz;
        s.frame = frame;
        s.ncx = xcells ? xcells : 1;
        s.ncy = ycells ? ycells : 1;
        s.sx = sx;
        s.sy = sy;
        s.ox = ox * sx;
        s.oy = oy * sy;
        s.cellw = (texture.x * sx / s.ncx);
        s.cellh = (texture.y * sy / s.ncy);
        s.rgba = rgba;
        s.cos = 1;
        s.sin = 0;
        if(rotation) {
            rotation = (rotation + 0) * ((float)C_PI / 180);
            s.cos = cosf(rotation);
            s.sin = sinf(rotation);
        }

        batch_group_t *batches = is_additive ? &sprite_additive_group : &sprite_translucent_group;
#if 0
        batch_t *found = map_find(*batches, texture.id);
        if( !found ) found = map_insert(*batches, texture.id, (batch_t){0});
#else
        batch_t *found = map_find_or_add(*batches, texture.id, (batch_t){0});
#endif

        array_push(found->sprites, s);

        if( resolution_independant ) { // @todo: optimize me
        sprite_flush();
        camera_get_active()->position = bak;
        }
    }
}

static void sprite_rebuild_meshes() {
    sprite_count = 0;

    batch_group_t* list[] = { &sprite_additive_group, &sprite_translucent_group };
    for( int l = 0; l < countof(list); ++l) {
        for each_map_ptr(*list[l], int,_, batch_t,bt) {

            bt->dirty = array_count(bt->sprites) ? 1 : 0;
            if( !bt->dirty ) continue;

            int index = 0;
            array_clear(sprite_indices);
            array_clear(sprite_vertices);

            array_foreach_ptr(bt->sprites, sprite_t,it ) {
                float x0 = it->ox - it->cellw/2, x3 = x0 + it->cellw;
                float y0 = it->oy - it->cellh/2, y3 = y0;
                float x1 = x0,                   x2 = x3;
                float y1 = y0 + it->cellh,       y2 = y1;

                // @todo: move this affine transform into glsl shader
                vec3 v0 = { it->px + ( x0 * it->cos - y0 * it->sin ), it->py + ( x0 * it->sin + y0 * it->cos ), it->pz };
                vec3 v1 = { it->px + ( x1 * it->cos - y1 * it->sin ), it->py + ( x1 * it->sin + y1 * it->cos ), it->pz };
                vec3 v2 = { it->px + ( x2 * it->cos - y2 * it->sin ), it->py + ( x2 * it->sin + y2 * it->cos ), it->pz };
                vec3 v3 = { it->px + ( x3 * it->cos - y3 * it->sin ), it->py + ( x3 * it->sin + y3 * it->cos ), it->pz };

                float cx = (1.0f / it->ncx) - 1e-9f;
                float cy = (1.0f / it->ncy) - 1e-9f;
                int idx = (int)it->frame;
                int px = idx % it->ncx;
                int py = idx / it->ncx;

                float ux = px * cx, uy = py * cy;
                float vx = ux + cx, vy = uy + cy;

                vec2 uv0 = vec2(ux, uy);
                vec2 uv1 = vec2(ux, vy);
                vec2 uv2 = vec2(vx, vy);
                vec2 uv3 = vec2(vx, uy);

                array_push( sprite_vertices, sprite_vertex(v0, uv0, it->rgba) ); // Vertex 0 (A)
                array_push( sprite_vertices, sprite_vertex(v1, uv1, it->rgba) ); // Vertex 1 (B)
                array_push( sprite_vertices, sprite_vertex(v2, uv2, it->rgba) ); // Vertex 2 (C)
                array_push( sprite_vertices, sprite_vertex(v3, uv3, it->rgba) ); // Vertex 3 (D)

                //      A--B                  A               A-B
                // quad |  | becomes triangle |\  and triangle \|
                //      D--C                  D-C               C
                GLuint A = (index+0), B = (index+1), C = (index+2), D = (index+3); index += 4;

                array_push( sprite_indices, sprite_index(C, D, A) ); // Triangle 1
                array_push( sprite_indices, sprite_index(C, A, B) ); // Triangle 2
            }

            mesh_update(&bt->mesh, "p3 t2 c4B", 0,array_count(sprite_vertices),sprite_vertices, 3*array_count(sprite_indices),sprite_indices, MESH_STATIC);

            // clear elements from queue
            sprite_count += array_count(bt->sprites);
            array_clear(bt->sprites);
        }
    }

    batch_group_t* list2[] = { &sprite_00_translucent_group };
    for( int l = 0; l < countof(list2); ++l) {
        for each_map_ptr(*list2[l], int,_, batch_t,bt) {

            bt->dirty = array_count(bt->sprites) ? 1 : 0;
            if( !bt->dirty ) continue;

            int index = 0;
            array_clear(sprite_indices);
            array_clear(sprite_vertices);

            array_foreach_ptr(bt->sprites, sprite_t,it ) {
                float x0 = it->ox - it->cellw/2, x3 = x0 + it->cellw;
                float y0 = it->oy - it->cellh/2, y3 = y0;
                float x1 = x0,                   x2 = x3;
                float y1 = y0 + it->cellh,       y2 = y1;

                // @todo: move this affine transform into glsl shader
                vec3 v0 = { it->px + ( x0 * it->cos - y0 * it->sin ), it->py + ( x0 * it->sin + y0 * it->cos ), it->pz };
                vec3 v1 = { it->px + ( x1 * it->cos - y1 * it->sin ), it->py + ( x1 * it->sin + y1 * it->cos ), it->pz };
                vec3 v2 = { it->px + ( x2 * it->cos - y2 * it->sin ), it->py + ( x2 * it->sin + y2 * it->cos ), it->pz };
                vec3 v3 = { it->px + ( x3 * it->cos - y3 * it->sin ), it->py + ( x3 * it->sin + y3 * it->cos ), it->pz };

                float ux = it->x, vx = ux + it->w;
                float uy = it->y, vy = uy + it->h;

                vec2 uv0 = vec2(ux, uy);
                vec2 uv1 = vec2(ux, vy);
                vec2 uv2 = vec2(vx, vy);
                vec2 uv3 = vec2(vx, uy);

                array_push( sprite_vertices, sprite_vertex(v0, uv0, it->rgba) ); // Vertex 0 (A)
                array_push( sprite_vertices, sprite_vertex(v1, uv1, it->rgba) ); // Vertex 1 (B)
                array_push( sprite_vertices, sprite_vertex(v2, uv2, it->rgba) ); // Vertex 2 (C)
                array_push( sprite_vertices, sprite_vertex(v3, uv3, it->rgba) ); // Vertex 3 (D)

                //      A--B                  A               A-B
                // quad |  | becomes triangle |\  and triangle \|
                //      D--C                  D-C               C
                GLuint A = (index+0), B = (index+1), C = (index+2), D = (index+3); index += 4;

                array_push( sprite_indices, sprite_index(C, D, A) ); // Triangle 1
                array_push( sprite_indices, sprite_index(C, A, B) ); // Triangle 2
            }

            mesh_update(&bt->mesh, "p3 t2 c4B", 0,array_count(sprite_vertices),sprite_vertices, 3*array_count(sprite_indices),sprite_indices, MESH_STATIC);

            // clear elements from queue
            sprite_count += array_count(bt->sprites);
            array_clear(bt->sprites);
        }
    }
}

static void sprite_render_meshes() {
    if( map_count(sprite_additive_group) <= 0 )
    if( map_count(sprite_translucent_group) <= 0 )
    if( map_count(sprite_00_translucent_group) <= 0 )
        return;

    if( sprite_program < 0 ) {
        sprite_program = shader( vs_324_24_sprite, fs_24_4_sprite,
            "att_Position,att_TexCoord,att_Color",
            "fragColor"
        );
    }

    // use the shader and  bind the texture @ unit 0
    shader_bind(sprite_program);
    glActiveTexture(GL_TEXTURE0);

    // setup rendering state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glDepthFunc(GL_LEQUAL); // try to help with zfighting

    // update camera
    // camera_fps(camera_get_active(), 0,0);
    vec3 pos = camera_get_active()->position;
    float zoom = absf(pos.z); if(zoom < 0.1f) zoom = 0.1f; zoom = 1.f / (zoom + !zoom);
    float width  = window_width();
    float height = window_height();

    // set mvp in the uniform. (0,0) is center of screen.
    mat44 mvp2d;
    float zdepth_max = window_height(); // 1;
    float l = pos.x - width  * zoom / 2;
    float r = pos.x + width  * zoom / 2;
    float b = pos.y + height * zoom / 2;
    float t = pos.y - height * zoom / 2;
    ortho44(mvp2d, l,r,b,t, -zdepth_max, +zdepth_max);

    shader_mat44("u_mvp", mvp2d);

    // set (unit 0) in the uniform texture sampler, and render batch
    // for all additive then translucent groups

    if( map_count(sprite_additive_group) > 0 ) {
        glBlendFunc( GL_SRC_ALPHA, GL_ONE );
        for each_map_ptr(sprite_additive_group, int,texture_id, batch_t,bt) {
            if( bt->dirty ) {
                shader_texture_unit("u_texture", *texture_id, 0);
                mesh_render(&bt->mesh);
            }
        }
//        map_clear(sprite_additive_group);
    }

    if( map_count(sprite_translucent_group) > 0 ) {
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        for each_map_ptr(sprite_translucent_group, int,texture_id, batch_t,bt) {
            if( bt->dirty ) {
                shader_texture_unit("u_texture", *texture_id, 0);
                mesh_render(&bt->mesh);
            }
        }
//        map_clear(sprite_translucent_group);
    }

    if( map_count(sprite_00_translucent_group) > 0 ) {
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        for each_map_ptr(sprite_00_translucent_group, int,texture_id, batch_t,bt) {
            if( bt->dirty ) {
                shader_texture_unit("u_texture", *texture_id, 0);
                mesh_render(&bt->mesh);
            }
        }
//        map_clear(sprite_00_translucent_group);
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDepthFunc(GL_LESS);
    glUseProgram(0);
}

static void sprite_init() {
    do_once {
    map_init(sprite_00_translucent_group, less_int, hash_int);
    map_init(sprite_translucent_group, less_int, hash_int);
    map_init(sprite_additive_group, less_int, hash_int);
    }
}

void sprite_flush() {
    profile("Sprite.rebuild_time") {
    sprite_rebuild_meshes();
    }
    profile("Sprite.render_time") {
    sprite_render_meshes();
    }
}

// -----------------------------------------------------------------------------
// tilemaps

tilemap_t tilemap(const char *map, int blank_chr, int linefeed_chr) {
    tilemap_t t = {0};
    t.tint = ~0u; // WHITE
    t.blank_chr = blank_chr;
    for( ; *map ; ++map ) {
        if( map[0] == linefeed_chr ) ++t.rows;
        else {
            array_push(t.map, map[0]);
            ++t.cols;
        }
    }
    return t;
}

void tilemap_render_ext( tilemap_t m, tileset_t t, float zindex, float xy_zoom[3], float tilt, unsigned tint, bool is_additive ) {
    vec3 old_pos = camera_get_active()->position;
    sprite_flush();
    camera_get_active()->position = vec3(window_width()/2,window_height()/2,1);

    float scale[2] = {xy_zoom[2], xy_zoom[2]};
    xy_zoom[2] = zindex;

    float offset[2] = {0,0};
    float spritesheet[3] = {0,t.cols,t.rows}; // selected tile index and spritesheet dimensions (cols,rows)

    for( unsigned y = 0, c = 0; y < m.rows; ++y ) {
        for( unsigned x = 0; x < m.cols; ++x, ++c ) {
            if( m.map[c] != m.blank_chr ) {
                spritesheet[0] = m.map[c];
                sprite_sheet(t.tex, spritesheet, xy_zoom, tilt, offset, scale, is_additive, tint, false);
            }
            offset[0] += t.tile_w;
        }
        offset[0] = 0, offset[1] += t.tile_h;
    }

    sprite_flush();
    camera_get_active()->position = old_pos;
}

void tilemap_render( tilemap_t map, tileset_t set ) {
    map.position.x += set.tile_w;
    map.position.y += set.tile_h;
    tilemap_render_ext( map, set, map.zindex, &map.position.x, map.tilt, map.tint, map.is_additive );
}

tileset_t tileset(texture_t tex, unsigned tile_w, unsigned tile_h, unsigned cols, unsigned rows) {
    tileset_t t = {0};
    t.tex = tex;
    t.cols = cols, t.rows = rows;
    t.tile_w = tile_w, t.tile_h = tile_h;
    return t;
}

int tileset_ui( tileset_t t ) {
    ui_subimage(va("Selection #%d (%d,%d)", t.selected, t.selected % t.cols, t.selected / t.cols), t.tex.id, t.tex.w, t.tex.h, (t.selected % t.cols) * t.tile_w, (t.selected / t.cols) * t.tile_h, t.tile_w, t.tile_h);
    int choice;
    if( (choice = ui_image(0, t.tex.id, t.tex.w,t.tex.h)) ) { 
        int px = ((choice / 100) / 100.f) * t.tex.w / t.tile_w;
        int py = ((choice % 100) / 100.f) * t.tex.h / t.tile_h;
        t.selected = px + py * t.cols;
    }
    // if( (choice = ui_buttons(3, "load", "save", "clear")) ) {}
    return t.selected;
}

// -----------------------------------------------------------------------------
// tiled

tiled_t tiled(const char *file_tmx) {
    tiled_t zero = {0}, ti = zero;

    // read file and parse json
    if( !xml_push(file_tmx) ) return zero;

    // sanity checks
    bool supported = !strcmp(xml_string("/map/@orientation"), "orthogonal") && !strcmp(xml_string("/map/@renderorder"), "right-down");
    if( !supported ) return xml_pop(), zero;

    // tileset
    const char *file_tsx = xml_string("/map/tileset/@source");
    if( !xml_push(vfs_read(file_tsx)) ) return zero;
        const char *set_src = xml_string("/tileset/image/@source");
        int set_w = xml_int("/tileset/@tilewidth");
        int set_h = xml_int("/tileset/@tileheight");
        int set_c = xml_int("/tileset/@columns");
        int set_r = xml_int("/tileset/@tilecount") / set_c;
        tileset_t set = tileset(texture(set_src,0), set_w, set_h, set_c, set_r );
    xml_pop();

    // actual parsing
    ti.w = xml_int("/map/@width");
    ti.h = xml_int("/map/@height");
    ti.tilew = xml_int("/map/@tilewidth");
    ti.tileh = xml_int("/map/@tileheight");
    ti.first_gid = xml_int("/map/tileset/@firstgid");
    ti.map_name = STRDUP( xml_string("/map/tileset/@source") ); // @leak

    for(int l = 0, layers = xml_count("/map/layer"); l < layers; ++l ) {
        if( strcmp(xml_string("/map/layer[%d]/data/@encoding",l), "base64") || strcmp(xml_string("/map/layer[%d]/data/@compression",l), "zlib") ) {
            PRINTF("Warning: layer encoding not supported: '%s' -> layer '%s'\n", file_tmx, *array_back(ti.names));
            continue;
        }

        int cols = xml_int("/map/layer[%d]/@width",l);
        int rows = xml_int("/map/layer[%d]/@height",l);

        tilemap_t tm = tilemap("", ' ', '\n');
        tm.blank_chr = ~0u; //ti.first_gid - 1;
        tm.cols = cols;
        tm.rows = rows;
        array_resize(tm.map, tm.cols * tm.rows);
        memset(tm.map, 0xFF, tm.cols * tm.rows * sizeof(int));

        for( int c = 0, chunks = xml_count("/map/layer[%d]/data/chunk", l); c <= chunks; ++c ) {
            int cw, ch;
            int cx, cy;
            array(char) b64 = 0;

            if( !chunks ) { // non-infinite mode
                b64 = xml_blob("/map/layer[%d]/data/$",l);
                cw = tm.cols, ch = tm.rows;
                cx = 0, cy = 0;
            } else { // infinite mode
                b64 = xml_blob("/map/layer[%d]/data/chunk[%d]/$",l,c);
                cw = xml_int("/map/layer[%d]/data/chunk[%d]/@width",l,c), ch = xml_int("/map/layer[%d]/data/chunk[%d]/@height",l,c); // 20x20
                cx = xml_int("/map/layer[%d]/data/chunk[%d]/@x",l,c), cy = xml_int("/map/layer[%d]/data/chunk[%d]/@y",l,c); // (-16,-32)
                cx = abs(cx), cy = abs(cy);
            }

            int outlen = cw * ch * 4;
            static __thread int *out = 0; out = (int *)REALLOC( 0, outlen + zexcess(COMPRESS_ZLIB) ); // @leak
            if( zdecode( out, outlen, b64, array_count(b64), COMPRESS_ZLIB ) > 0 ) {
                for( int y = 0, p = 0; y < ch; ++y ) {
                    for( int x = 0; x < cw; ++x, ++p ) {
                        if( out[p] >= ti.first_gid ) {
                            int offset = (x + cx) + (y + cy) * tm.cols;
                            if( offset >= 0 && offset < (cw * ch) )
                            tm.map[ offset ] = out[ p ] - ti.first_gid;
                        }
                    }
                }
            }
            else {
                PRINTF("Warning: bad zlib stream: '%s' -> layer #%d -> chunk #%d\n", file_tmx, l, c);
            }

            array_free(b64);
        }

        array_push(ti.layers, tm);
        array_push(ti.names, STRDUP(xml_string("/map/layer[%d]/@name",l)));
        array_push(ti.visible, true);
        array_push(ti.sets, set);
    }

    xml_pop();
    return ti;
}

void tiled_render(tiled_t tmx, vec3 pos) {
    for( unsigned i = 0, end = array_count(tmx.layers); i < end; ++i ) {
        tmx.layers[i].position = pos; // add3(camera_get_active()->position, pos);
        if( tmx.parallax ) tmx.layers[i].position.x /= (3+i), tmx.layers[i].position.y /= (5+i);
        if( tmx.visible[i] ) tilemap_render(tmx.layers[i], tmx.sets[i]);
    }
}

void tiled_ui(tiled_t *t) {
    ui_label2("Loaded map", t->map_name ? t->map_name : "(none)");
    ui_label2("Map dimensions", va("%dx%d", t->w, t->h));
    ui_label2("Tile dimensions", va("%dx%d", t->tilew, t->tileh));
    ui_separator();
    ui_bool("Parallax", &t->parallax);
    ui_separator();
    ui_label2("Layers", va("%d", array_count(t->layers)));
    for( int i = 0; i < array_count(t->layers); ++i ) {
        if( ui_label2_toolbar(va("- %s (%dx%d)", t->names[i], t->layers[i].cols, t->layers[i].rows ), t->visible[i] ? "\xee\xa3\xb4" : "\xee\xa3\xb5") > 0 ) { // ICON_MD_VISIBILITY / ICON_MD_VISIBILITY_OFF
            t->visible[i] ^= true;
        }
    }
    ui_separator();
    if( ui_collapse(va("Sets: %d", array_count(t->layers)), va("%p",t))) {
        for( int i = 0; i < array_count(t->layers); ++i ) {
            if( ui_collapse(va("%d", i+1), va("%p%d",t,i)) ) {
                t->sets[i].selected = tileset_ui( t->sets[i] );
                ui_collapse_end();
            }
        }
        ui_collapse_end();
    }
}

// -----------------------------------------------------------------------------
// spine json loader (wip)
// - rlyeh, public domain
//
// [ref] http://es.esotericsoftware.com/spine-json-format
//
// notable misses:
// - mesh deforms
// - cubic beziers
// - shears
// - bounding boxes

enum { SPINE_MAX_BONES = 64 }; // max bones

typedef struct spine_bone_t {
    char *name, *parent;
    struct spine_bone_t *parent_bone;

    float z; // draw order usually matches bone-id. ie, zindex == bone_id .. root(0) < chest (mid) < finger(top)

    float len;
    float x, y, deg;        // base
    float x2, y2, deg2;     // accum / temporaries during bone transform time
    float x3, y3, deg3;     // values from timeline

    unsigned rect_id;
    unsigned atlas_id;
} spine_bone_t;

typedef struct spine_slot_t {
    char *name, *bone, *attach;
} spine_slot_t;

typedef struct spine_rect_t {
    char *name;
    float x,y,w,h,sx,sy,deg;
} spine_rect_t;

typedef struct spine_skin_t {
    char *name;
    array(spine_rect_t) rects;
} spine_skin_t;

typedef struct spine_animkey_t { // offline; only during loading
    float time, curve[4];        // time is mandatory, curve is optional
    union {
        char *name;              // type: attachment (mode-1)
        struct { float deg; };   // type: rotate (mode-2)
        struct { float x,y; };   // type: translate (mode-3)
    };
} spine_animkey_t;

#if 0
typedef struct spine_pose_t { // runtime; only during playing
    unsigned frame;
    array(vec4) xform; // entry per bone. translation(x,y),rotation(z),attachment-id(w)
} spine_pose_t;
#endif

typedef struct spine_anim_t {
    char *name;
    union {
#if 0
        struct {
            unsigned frames;
            array(spine_pose_t) poses;
        };
#endif
        struct {
            array(spine_animkey_t) attach_keys[SPINE_MAX_BONES];
            array(spine_animkey_t) rotate_keys[SPINE_MAX_BONES];
            array(spine_animkey_t) translate_keys[SPINE_MAX_BONES];
        };
    };
} spine_anim_t;

typedef struct spine_atlas_t {
    char *name;
    float x,y,w,h,deg;
} spine_atlas_t;

typedef struct spine_t {
    char *name;
    texture_t texture;
    unsigned skin;
    array(spine_bone_t) bones;
    array(spine_slot_t) slots;
    array(spine_skin_t) skins;
    array(spine_anim_t) anims;
    array(spine_atlas_t) atlas;
    // anim controller
    unsigned inuse;
    float time, maxtime;
    unsigned debug_atlas_id;
} spine_t;

// ---

static
void spine_convert_animkeys_to_animpose(spine_anim_t *input) {
    spine_anim_t copy = *input; // @todo
    // @leak: attach/rot/tra keys
}

static
int find_bone_id(spine_t *s, const char *bone_name) {
    for( unsigned i = 0, end = array_count(s->bones); i < end; ++i )
        if( !strcmp(s->bones[i].name, bone_name)) return i;
    return -1;
}
static
spine_bone_t *find_bone(spine_t *s, const char *bone_name) {
    int bone_id = find_bone_id(s, bone_name);
    return bone_id >= 0 ? &s->bones[bone_id] : NULL;
}

void spine_skin(spine_t *p, unsigned skin) {
    if( !p->texture.id ) return;
    if( skin >= array_count(p->skins) ) return;

    p->skin = skin;

    char *skin_name = va("%s/", p->skins[skin].name);
    int header = strlen(skin_name);

    for( int i = 0; i < array_count(p->atlas); ++i) {
        if(!strbeg(p->atlas[i].name, skin_name)) continue;

        int bone_id = find_bone_id(p, p->atlas[i].name+header );
        if( bone_id < 0 ) continue;

        p->bones[bone_id].atlas_id = i;
    }

    for( int i = 0; i < array_count(p->skins[p->skin].rects); ++i) {
        int bone_id = find_bone_id(p, p->skins[p->skin].rects[i].name );
        if( bone_id < 0 ) continue;

        p->bones[bone_id].rect_id = i;
    }
}

static
bool spine_(spine_t *t, const char *file_json, const char *file_atlas, unsigned flags) {
    char *atlas = vfs_read(file_atlas);
    if(!atlas || !atlas[0]) return false;

    memset(t, 0, sizeof(spine_t));

    // goblins.png
    //   size: 1024, 128
    //   filter: Linear, Linear
    //   pma: true
    // dagger
    //   bounds: 2, 18, 26, 108
    // goblin/eyes-closed
    //   bounds: 2, 4, 34, 12
    spine_atlas_t *sa = 0;
    const char *last_id = 0;
    const char *texture_name = 0;
    const char *texture_filter = 0;
    const char *texture_format = 0;
    const char *texture_repeat = 0;
    float texture_width = 0, texture_height = 0, temp;
    for each_substring(atlas, "\r\n", it) {
        it += strspn(it, " \t\f\v");
        /**/ if( strbeg(it, "pma:" ) || strbeg(it, "index:") ) {} // ignored
        else if( strbeg(it, "size:" ) ) sscanf(it+5, "%f,%f", &texture_width, &texture_height);
        else if( strbeg(it, "rotate:" ) ) { float tmp; tmp=sa->w,sa->w=sa->h,sa->h=tmp; sa->deg = 90; } // assert(val==90)
        else if( strbeg(it, "repeat:" ) ) texture_repeat = it+7; // temp string
        else if( strbeg(it, "filter:" ) ) texture_filter = it+7; // temp string
        else if( strbeg(it, "format:" ) ) texture_format = it+7; // temp string
        else if( strbeg(it, "bounds:" ) ) {
            sscanf(it+7, "%f,%f,%f,%f", &sa->x, &sa->y, &sa->w, &sa->h);
        }
        else if( !texture_name ) texture_name = va("%s", it);
        else {
            array_push(t->atlas, ((spine_atlas_t){0}) );
            sa = &t->atlas[array_count(t->atlas) - 1];
            sa->name = STRDUP(it);
        }
    }
    for( int i = 0; i < array_count(t->atlas); ++i ) {
        sa = &t->atlas[i];
        sa->x /= texture_width, sa->y /= texture_height;
        sa->w /= texture_width, sa->h /= texture_height;
    }

    if(!texture_name) return false;

    t->texture = texture(texture_name, TEXTURE_LINEAR);

    json_push(vfs_read(file_json)); // @fixme: json_push_from_file() ?

    array_resize(t->bones, json_count("/bones"));
    array_reserve(t->slots, json_count("/slots"));
    array_resize(t->skins, json_count("/skins"));
    array_resize(t->anims, json_count("/animations"));

    for( int i = 0, end = json_count("/bones"); i < end; ++i ) {
        spine_bone_t v = {0};
        v.name = STRDUP(json_string("/bones[%d]/name", i));
        v.parent = STRDUP(json_string("/bones[%d]/parent", i));
        v.x = json_float("/bones[%d]/x", i);
        v.y = json_float("/bones[%d]/y", i);
        v.z = i;
        v.len = json_float("/bones[%d]/length", i);
        v.deg = json_float("/bones[%d]/rotation", i);
        t->bones[i] = v;

        for( int j = i-1; j > 0; --j ) {
            if( strcmp(t->bones[j].name,v.parent) ) continue;
            t->bones[i].parent_bone = &t->bones[j];
            break;
        }
    }

    for( int i = 0, end = json_count("/slots"); i < end; ++i ) {
        spine_slot_t v = {0};
        v.name = STRDUP(json_string("/slots[%d]/name", i));
        v.bone = STRDUP(json_string("/slots[%d]/bone", i));
        v.attach = STRDUP(json_string("/slots[%d]/attachment", i));

        array_push(t->slots, v);

        // slots define draw-order. so, update draw-order/zindex in bone
        spine_bone_t *b = find_bone(t, v.name);
        if( b ) b->z = i;
    }

    for( int i = 0, end = json_count("/skins"); i < end; ++i ) {
        spine_skin_t v = {0};
        v.name = STRDUP(json_string("/skins[%d]/name", i));

        for( int j = 0, jend = json_count("/skins[%d]/attachments",i); j < jend; ++j ) // /skins/default/
        for( int k = 0, kend = json_count("/skins[%d]/attachments[%d]",i,j); k < kend; ++k ) { // /skins/default/left hand item/
            spine_rect_t r = {0};
            r.name = STRDUP(json_key("/skins[%d]/attachments[%d][%d]",i,j,k)); // stringf("%s-%s-%s", json_key("/skins[%d]",i), json_key("/skins[%d][%d]",i,j), json_key("/skins[%d][%d][%d]",i,j,k));
            r.x = json_float("/skins[%d]/attachments[%d][%d]/x",i,j,k);
            r.y = json_float("/skins[%d]/attachments[%d][%d]/y",i,j,k);
            r.sx= json_float("/skins[%d]/attachments[%d][%d]/scaleX",i,j,k); r.sx += !r.sx;
            r.sy= json_float("/skins[%d]/attachments[%d][%d]/scaleY",i,j,k); r.sy += !r.sy;
            r.w = json_float("/skins[%d]/attachments[%d][%d]/width",i,j,k);
            r.h = json_float("/skins[%d]/attachments[%d][%d]/height",i,j,k);
            r.deg = json_float("/skins[%d]/attachments[%d][%d]/rotation",i,j,k);
            array_push(v.rects, r);
        }

        t->skins[i] = v;
    }

#if 1
    // simplify:
    // merge /skins/default into existing /skins/*, then delete /skins/default
    if( array_count(t->skins) > 1 ) {
        for( int i = 1; i < array_count(t->skins); ++i ) {
            for( int j = 0; j < array_count(t->skins[0].rects); ++j ) {
                array_push(t->skins[i].rects, t->skins[0].rects[j]);
            }
        }
        // @leak @fixme: free(t->skins[0])
        for( int i = 0; i < array_count(t->skins)-1; ++i ) {
            t->skins[i] = t->skins[i+1];
        }
        array_pop(t->skins);
    }
#endif

    for( int i = 0, end = json_count("/animations"); i < end; ++i ) {
        int id;
        const char *name;

        spine_anim_t v = {0};
        v.name = STRDUP(json_key("/animations[%d]", i));

        // slots / attachments

        for( int j = 0, jend = json_count("/animations[%d]/slots",i); j < jend; ++j )
        for( int k = 0, kend = json_count("/animations[%d]/slots[%d]",i,j); k < kend; ++k ) // ids
        {
            int bone_id = find_bone_id(t, json_key("/animations[%d]/bones[%d]",i,j));
            if( bone_id < 0 ) continue;

            for( int l = 0, lend = json_count("/animations[%d]/slots[%d][%d]",i,j,k); l < lend; ++l ) { // channels (rot,tra,attach)
                spine_animkey_t key = {0};

                key.name = STRDUP(json_string("/animations[%d]/slots[%d][%d][%d]/name",i,j,k,l));
                key.time = json_float("/animations[%d]/slots[%d][%d][%d]/time",i,j,k,l);
                if( json_count("/animations[%d]/slots[%d][%d][%d]/curve",i,j,k,l) == 4 ) {
                key.curve[0] = json_float("/animations[%d]/slots[%d][%d][%d]/curve[0]",i,j,k,l);
                key.curve[1] = json_float("/animations[%d]/slots[%d][%d][%d]/curve[1]",i,j,k,l);
                key.curve[2] = json_float("/animations[%d]/slots[%d][%d][%d]/curve[2]",i,j,k,l);
                key.curve[3] = json_float("/animations[%d]/slots[%d][%d][%d]/curve[3]",i,j,k,l);
                }

                // @todo: convert name to id
                // for(id = 0; t->bones[id].name && strcmp(t->bones[id].name,key.name); ++id)
                // printf("%s vs %s\n", key.name, t->bones[id].name);

                array_push(v.attach_keys[bone_id], key);
            }
        }

        // bones

        for( int j = 0, jend = json_count("/animations[%d]/bones",i); j < jend; ++j ) // slots or bones
        for( int k = 0, kend = json_count("/animations[%d]/bones[%d]",i,j); k < kend; ++k ) { // bone ids
            int bone_id = find_bone_id(t, json_key("/animations[%d]/bones[%d]",i,j));
            if( bone_id < 0 ) continue;

            // parse bones
            for( int l = 0, lend = json_count("/animations[%d]/bones[%d][%d]",i,j,k); l < lend; ++l ) { // channels (rot,tra,attach)
                const char *channel = json_key("/animations[%d]/bones[%d][%d]",i,j,k);
                int track = !strcmp(channel, "rotate") ? 1 : !strcmp(channel, "translate") ? 2 : 0;
                if( !track ) continue;

                spine_animkey_t key = {0};

                key.time = json_float("/animations[%d]/bones[%d][%d][%d]/time",i,j,k,l);
                if( json_count("/animations[%d]/bones[%d][%d][%d]/curve",i,j,k,l) == 4 ) {
                key.curve[0] = json_float("/animations[%d]/bones[%d][%d][%d]/curve[0]",i,j,k,l);
                key.curve[1] = json_float("/animations[%d]/bones[%d][%d][%d]/curve[1]",i,j,k,l);
                key.curve[2] = json_float("/animations[%d]/bones[%d][%d][%d]/curve[2]",i,j,k,l);
                key.curve[3] = json_float("/animations[%d]/bones[%d][%d][%d]/curve[3]",i,j,k,l);
                }

                if( track == 1 )
                key.deg = json_float("/animations[%d]/bones[%d][%d][%d]/value",i,j,k,l), // "/angle"
                array_push(v.rotate_keys[bone_id], key);
                else
                key.x = json_float("/animations[%d]/bones[%d][%d][%d]/x",i,j,k,l),
                key.y = json_float("/animations[%d]/bones[%d][%d][%d]/y",i,j,k,l),
                array_push(v.translate_keys[bone_id], key);
            }
        }

        t->anims[i] = v;
    }

    json_pop();

    spine_skin(t, 0);

    return true;
}

spine_t* spine(const char *file_json, const char *file_atlas, unsigned flags) {
    spine_t *t = MALLOC(sizeof(spine_t));
    if( !spine_(t, file_json, file_atlas, flags) ) return FREE(t), NULL;
    return t;
}

void spine_render(spine_t *p, vec3 offset, unsigned flags) {
    if( !p->texture.id ) return;
    if( !flags ) return;

    ddraw_push_2d();
        // if( flags & 2 ) ddraw_line(vec3(0,0,0), vec3(window_width(),window_height(),0));
        // if( flags & 2 ) ddraw_line(vec3(window_width(),0,0), vec3(0,window_height(),0));

        // int already_computed[SPINE_MAX_BONES] = {0}; // @fixme: optimize: update longest chains first, then remnant branches

        for( int i = 1; i < array_count(p->bones); ++i ) {
            spine_bone_t *self = &p->bones[i];
            if( !self->rect_id ) continue;

            int num_bones = 0;
            static array(spine_bone_t*) chain = 0; array_resize(chain, 0);
            for( spine_bone_t *next = self; next ; next = next->parent_bone, ++num_bones ) {
                array_push(chain, next);
            }

            vec3 target = {0}, prev = {0};
            for( int j = 0, end = array_count(chain); j < end; ++j ) { // traverse from root(skipped) -> `i` bone direction
                int j_opposite = end - 1 - j;

                spine_bone_t *b = chain[j_opposite]; // bone
                spine_bone_t *pb = b->parent_bone; // parent bone

                float pb_x2 = 0, pb_y2 = 0, pb_deg2 = 0;
                if( pb ) pb_x2 = pb->x2, pb_y2 = pb->y2, pb_deg2 = pb->deg2;

                const float deg2rad = C_PI / 180;
                b->x2 =      b->x3 + pb_x2   + b->x * cos( -pb_deg2 * deg2rad ) - b->y * sin( -pb_deg2 * deg2rad );
                b->y2 =     -b->y3 + pb_y2   - b->y * cos(  pb_deg2 * deg2rad ) + b->x * sin(  pb_deg2 * deg2rad );
                b->deg2 = -b->deg3 + pb_deg2 - b->deg;

                prev = target;
                target = vec3(b->x2,b->y2,b->deg2);
            }

            target.z = 0;
            target = add3(target, offset);
            prev.z = 0;
            prev = add3(prev, offset);

            if( flags & 2 ) {
                ddraw_point( target );
                ddraw_text( target, -0.25f, self->name );
                ddraw_bone( prev, target ); // from parent to bone
            }
            if( flags & 1 ) {
                spine_atlas_t *a = &p->atlas[self->atlas_id];
                spine_rect_t *r = &p->skins[p->skin].rects[self->rect_id];

                vec4 rect = ptr4(&a->x);
                float zindex = self->z;
                float offsx = 0;
                float offsy = 0;
                float tilt = self->deg2 + (a->deg - r->deg);
                unsigned tint = self->atlas_id == p->debug_atlas_id ? 0xFF<<24 | 0xFF : ~0u;

                if( 1 ) {
                    vec3 dir = vec3(r->x,r->y,0);
                    dir = rotatez3(dir, self->deg2);
                    offsx = dir.x * r->sx;
                    offsy = dir.y * r->sy;
                }

                sprite_rect(p->texture, rect, zindex, add3(vec3(target.x,target.y,1),vec3(offsx,offsy,0)), tilt, tint);
            }
         }

    ddraw_pop_2d();
    ddraw_flush();
}

static
void spine_animate_(spine_t *p, float *time, float *maxtime, float delta) {
    if( !p->texture.id ) return;

    if( delta > 1/120.f ) delta = 1/120.f;
    if( *time >= *maxtime ) *time = 0; else *time += delta;

    // reset root // needed?
    p->bones[0].x2 = 0;
    p->bones[0].y2 = 0;
    p->bones[0].deg2 = 0;
    p->bones[0].x3 = 0;
    p->bones[0].y3 = 0;
    p->bones[0].deg3 = 0;

    for( int i = 0, end = array_count(p->bones); i < end; ++i) {
        // @todo: attach channel
        // @todo: per channel: if curve == linear || curve == stepped || array_count(curve) == 4 {...}
        for each_array_ptr(p->anims[p->inuse].rotate_keys[i], spine_animkey_t, r) {
            double r0 = r->time;
            *maxtime = maxf( *maxtime, r0 );
            if( absf(*time - r0) < delta ) {
                p->bones[i].deg3 = r->deg;
            }
        }
        for each_array_ptr(p->anims[p->inuse].translate_keys[i], spine_animkey_t, r) {
            double r0 = r->time;
            *maxtime = maxf( *maxtime, r0 );
            if( absf(*time - r0) < delta ) {
                p->bones[i].x3 = r->x;
                p->bones[i].y3 = r->y;
            }
        }
    }
}

void spine_animate(spine_t *p, float delta) {
    spine_animate_(p, &p->time, &p->maxtime, delta);
}

void spine_ui(spine_t *p) {
    if( ui_collapse(va("Anims: %d", array_count(p->anims)), va("%p-a", p))) {
        for each_array_ptr(p->anims, spine_anim_t, q) {
            if(ui_slider2("", &p->time, va("%.2f/%.0f %.2f%%", p->time, p->maxtime, p->time * 100.f))) {
                spine_animate(p, 0);
            }

            int choice = ui_label2_toolbar(q->name, ICON_MD_PAUSE_CIRCLE " " ICON_MD_PLAY_CIRCLE);
            if( choice == 1 ) window_pause( 0 ); // play
            if( choice == 2 ) window_pause( 1 ); // pause

            for( int i = 0; i < SPINE_MAX_BONES; ++i ) {
                ui_separator();
                ui_label(va("Bone %d: Attachment keys", i));
                for each_array_ptr(q->attach_keys[i], spine_animkey_t, r) {
                    ui_label(va("%.2f [%.2f %.2f %.2f %.2f] %s", r->time, r->curve[0], r->curve[1], r->curve[2], r->curve[3], r->name));
                }
                ui_label(va("Bone %d: Rotate keys", i));
                for each_array_ptr(q->rotate_keys[i], spine_animkey_t, r) {
                    ui_label(va("%.2f [%.2f %.2f %.2f %.2f] %.2f deg", r->time, r->curve[0], r->curve[1], r->curve[2], r->curve[3], r->deg));
                }
                ui_label(va("Bone %d: Translate keys", i));
                for each_array_ptr(q->translate_keys[i], spine_animkey_t, r) {
                    ui_label(va("%.2f [%.2f %.2f %.2f %.2f] (%.2f,%.2f)", r->time, r->curve[0], r->curve[1], r->curve[2], r->curve[3], r->x, r->y));
                }
            }
        }
        ui_collapse_end();
    }
    if( ui_collapse(va("Bones: %d", array_count(p->bones)), va("%p-b", p))) {
        for each_array_ptr(p->bones, spine_bone_t, q)
        if( ui_collapse(q->name, va("%p-b2", q)) ) {
            ui_label2("Parent:", q->parent);
            ui_label2("X:", va("%.2f", q->x));
            ui_label2("Y:", va("%.2f", q->y));
            ui_label2("Length:", va("%.2f", q->len));
            ui_label2("Rotation:", va("%.2f", q->deg));
            ui_collapse_end();
        }
        ui_collapse_end();
    }
    if( ui_collapse(va("Slots: %d", array_count(p->slots)), va("%p-s", p))) {
        for each_array_ptr(p->slots, spine_slot_t, q)
        if( ui_collapse(q->name, va("%p-s2", q)) ) {
            ui_label2("Bone:", q->bone);
            ui_label2("Attachment:", q->attach);
            ui_collapse_end();
        }
        ui_collapse_end();
    }
    if( ui_collapse(va("Skins: %d", array_count(p->skins)), va("%p-k", p))) {
        for each_array_ptr(p->skins, spine_skin_t, q)
        if( ui_collapse(q->name, va("%p-k2", q)) ) {
            for each_array_ptr(q->rects, spine_rect_t, r)
            if( ui_collapse(r->name, va("%p-k3", r)) ) {
                ui_label2("X:", va("%.2f", r->x));
                ui_label2("Y:", va("%.2f", r->y));
                ui_label2("Scale X:", va("%.2f", r->sx));
                ui_label2("Scale Y:", va("%.2f", r->sy));
                ui_label2("Width:", va("%.2f", r->w));
                ui_label2("Height:", va("%.2f", r->h));
                ui_label2("Rotation:", va("%.2f", r->deg));
                ui_collapse_end();

                spine_bone_t *b = find_bone(p, r->name);
                if( b ) {
                    p->debug_atlas_id = b->atlas_id;

                    static float tilt = 0;
                    if( input(KEY_LCTRL) ) tilt += 60*1/60.f; else tilt = 0;
                    spine_atlas_t *r = p->atlas + b->atlas_id;
                    sprite_flush();
                    camera_get_active()->position = vec3(0,0,2);
                            vec4 rect = ptr4(&r->x); float zindex = 0; vec3 xy_zoom = vec3(0,0,0); unsigned tint = ~0u;
                            sprite_rect(p->texture,
                                // rect: vec4(r->x*1.0/p->texture.w,r->y*1.0/p->texture.h,(r->x+r->w)*1.0/p->texture.w,(r->y+r->h)*1.0/p->texture.h),
                                ptr4(&r->x), // atlas
                                0, vec3(0,0,0), r->deg + tilt, tint);
                            sprite_flush();
                    camera_get_active()->position = vec3(+window_width()/3,window_height()/2.25,2);
                }
            }
            ui_collapse_end();
        }
        ui_collapse_end();
    }

    if( ui_int("Use skin", &p->skin) ) {
    p->skin = clampf(p->skin, 0, array_count(p->skins) - 1);
    spine_skin(p, p->skin);
    }

    if( p->texture.id ) ui_texture(0, p->texture);
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
    sky.program = shader(vs_3_3_skybox,
        sky.flags ? fs_3_4_skybox : fs_3_4_skybox_rayleigh,
        "att_position", "fragcolor");

    // sky cubemap & SH
    if( asset ) {
        int is_panorama = vfs_size( asset );
        if( is_panorama ) {
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
    }

    return sky;
}
int skybox_push_state(skybox_t *sky, mat44 proj, mat44 view) {
    last_cubemap = &sky->cubemap;

//glClear(GL_DEPTH_BUFFER_BIT);
//glEnable(GL_DEPTH_TEST);
glDepthFunc(GL_LEQUAL);
//glDisable(GL_CULL_FACE);
glDisable(GL_DEPTH_TEST);

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

void viewport_color3(vec3 color3) {
    glClearColor(color3.x, color3.y, color3.z, 1);
}
void viewport_color(uint32_t rgba) {
    float b = ((rgba >>  0) & 255) / 255.f;
    float g = ((rgba >>  8) & 255) / 255.f;
    float r = ((rgba >> 16) & 255) / 255.f;
    glClearColor(r, g, b, 1);
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

    const char *vs = vs_0_2_fullscreen_quad_B;

    // patch fragment
    char *fs2 = (char*)CALLOC(1, 128*1024);
    strcat(fs2, fs_2_4_preamble);

    if( strstr(fs, "mainImage") ) {
        strcat(fs2, fs_main_shadertoy );
    }

    strcat(fs2, fs);

    p->program = shader(vs, fs2, "vtexcoord", "fragColor" );

    FREE(fs2);

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

bool postfx_begin(postfx *fx, int width, int height) {
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
        fbo_unbind();
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

    fbo_unbind();

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

            if( bound ) fbo_unbind();
            else glUseProgram(0);
        }
    }

    if(is_depth_test_enabled);
    glEnable(GL_DEPTH_TEST);

    return true;
}

static postfx fx;
int fx_load_from_mem(const char *nameid, const char *content) {
    do_once postfx_create(&fx, 0);
    return postfx_load_from_mem(&fx, nameid, content);
}
int fx_load(const char *filemask) {
    static set(char*) added = 0; do_once set_init_str(added);
    for(const char **list = vfs_list(filemask); *list; list++) {
        if( set_find(added, (char*)*list) ) continue;
        char *name = STRDUP(*list); // @leak
        set_insert(added, name);
        (void)postfx_load_from_mem(&fx, file_name(name), vfs_read(name));
    }
    return 1;
}
void fx_begin() {
    postfx_begin(&fx, window_width(), window_height());
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
    ASSERT(brdf.id != texture_checker().id, "!Couldn't load BRDF lookup table '%s'!", filename );
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
    s.dims = flags;

    char *file = vfs_read(shaderfile);
    if( !file ) return s;

    glGenVertexArrays(1, &s.vao);

    // Uses gl_VertexID to draw a fullscreen quad without vbo
    const char *vs = "#version 130\n"
        "uniform vec2 iResolution;           // viewport resolution (in pixels)\n"
        "out vec2 texCoord;\n"
        "void main() {\n"
            "   texCoord = vec2( (gl_VertexID << 1) & 2, gl_VertexID & 2 );\n"
            "   gl_Position = vec4( texCoord * 2.0 - 1.0, 0.0, 1.0 );\n"
            "   texCoord = texCoord * iResolution;\n"
        "}\n";
    const char *vs_flip = "#version 130\n"
        "uniform vec2 iResolution;           // viewport resolution (in pixels)\n"
        "out vec2 texCoord;\n"
        "void main() {\n"
            "   texCoord = vec2( (gl_VertexID << 1) & 2, gl_VertexID & 2 );\n"
            "   gl_Position = vec4( texCoord * 2.0 - 1.0, 0.0, 1.0 );\n"
            "   texCoord = texCoord * iResolution;\n"
            "   texCoord.y = iResolution.y - texCoord.y; // flip Y\n"
        "}\n";

    const char *header = "#version 130\n"
        "#define texture2D texture\n"
        "uniform float      iGlobalTime;           // shader playback time (in seconds)\n"
        "uniform float      iGlobalDelta;          // ??\n"
        "uniform float      iGlobalFrame;          // ??\n"
        "uniform float      iSampleRate;           // ??\n"
        "uniform float      iTime;                 // ??\n"
        "uniform int        iFrame;                // ??\n"
        "uniform float      iChannelTime[4];       // channel playback time (in seconds)\n"
        "uniform vec2       iResolution;           // viewport resolution (in pixels)\n"
        "uniform vec3       iChannelResolution[4]; // channel resolution (in pixels)\n"
        "uniform vec3       iOffset;               // ?? (0,0,0)\n"
        "uniform vec4       iMouse;                // mouse pixel coords. xy: hover, zw: LMB click)\n"
        "uniform vec4       iDate;                 // (year, month, day, time in seconds)\n"
        "uniform sampler2D  iChannel0;             // input channel 0\n" /*sampler%s*/
        "uniform sampler2D  iChannel1;             // input channel 1\n"
        "uniform sampler2D  iChannel2;             // input channel 2\n"
        "uniform sampler2D  iChannel3;             // input channel 3\n"
        "in  vec2 texCoord;\n"
        "out vec4 fragColor;\n"
        "void mainImage( out vec4 fragColor, in vec2 fragCoord );\n"
        "void main() {\n"
        "   mainImage(fragColor, texCoord.xy);\n"
        "}\n";

    char *fs = stringf("%s%s", header, file);
    s.program = shader(flags ? vs_flip : vs, fs, "", "fragColor");
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
        if( s->dims && !texture_rec_begin(&s->tx, s->dims, s->dims / 2) ) {
            return s;
        }

        float mx = input(MOUSE_X), my = input(MOUSE_Y);
        if(input(MOUSE_L)) s->clickx = mx, s->clicky = my;

        time_t tmsec = time(0);
        struct tm *tm = localtime(&tmsec);
        s->t += delta * 1000;

        glUseProgram(s->program);
        glUniform1f(s->uniforms[iGlobalTime], s->t / 1000.f );
        glUniform1f(s->uniforms[iGlobalFrame], s->frame++);
        glUniform1f(s->uniforms[iGlobalDelta], delta / 1000.f );
        glUniform2f(s->uniforms[iResolution], window_width(), window_height());
        glUniform4f(s->uniforms[iMouse], mx, my, s->clickx, s->clicky );

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

        if(s->dims) texture_rec_end(&s->tx); // texture_rec
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
} iqm_vertex;

typedef struct iqm_t {
    int nummeshes, numtris, numverts, numjoints, numframes, numanims;
    GLuint program;
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

#define program (q->program)
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
        if(intexcoord) memcpy(v->texcoord, &intexcoord[i*2], sizeof(v->texcoord));
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
//m->verts = verts; //FREE(verts);
m->verts = 0; FREE(verts);

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

        int flags = TEXTURE_MIPMAPS|TEXTURE_REPEAT; // LINEAR, NEAREST
        int invalid = texture_checker().id;

#if 1
        char *material_embedded_texture = strstr(&str[m->material], "+b64:");
        if( material_embedded_texture ) {
            *material_embedded_texture = '\0';
            material_embedded_texture += 5;
            array(char) embedded_texture = base64__decode(material_embedded_texture, strlen(material_embedded_texture));
            //printf("%s %d\n", material_embedded_texture, array_count(embedded_texture));
            //hexdump(embedded_texture, array_count(embedded_texture));
            *out = texture_compressed_from_mem( embedded_texture, array_count(embedded_texture), 0 ).id;
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
    static int shaderprog = -1;
    if( shaderprog < 0 ) {
        const char *symbols[] = { "{{include-shadowmap}}", fs_0_0_shadowmap_lit }; // #define RIM
        shaderprog = shader(strlerp(1,symbols,vs_323444143_16_332_model), strlerp(1,symbols,fs_32_4_model), //fs,
            "att_position,att_texcoord,att_normal,att_tangent,att_instanced_matrix,,,,att_indexes,att_weights,att_vertexindex,att_color,att_bitangent","fragColor");
    }

    iqm_t *q = CALLOC(1, sizeof(iqm_t));
    program = shaderprog;

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
        #undef program
        m.program = (q->program);
        #define program (q->program)

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
void model_draw_call(model_t m) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    glBindVertexArray( vao );

    struct iqmtriangle *tris = NULL;
    for(int i = 0; i < nummeshes; i++) {
        struct iqmmesh *im = &meshes[i];

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[i] );
        glUniform1i(glGetUniformLocation(program, "fsDiffTex"), 0 /*<-- unit!*/ );

        int loc;
        if ((loc = glGetUniformLocation(program, "u_textured")) >= 0) {
            bool textured = !!textures[i] && textures[i] != texture_checker().id; // m.materials[i].layer[0].texture != texture_checker().id;
            glUniform1i(loc, textured ? GL_TRUE : GL_FALSE);
            if ((loc = glGetUniformLocation(program, "u_diffuse")) >= 0) {
                glUniform4f(loc, m.materials[i].layer[0].color.r, m.materials[i].layer[0].color.g, m.materials[i].layer[0].color.b, m.materials[i].layer[0].color.a);
            }
        }

        glDrawElementsInstanced(GL_TRIANGLES, 3*im->num_triangles, GL_UNSIGNED_INT, &tris[im->first_triangle], m.num_instances);
        profile_incstat("Render.num_drawcalls", +1);
        profile_incstat("Render.num_triangles", +im->num_triangles);
    }

    glBindVertexArray( 0 );
}

void model_render_instanced(model_t m, mat44 proj, mat44 view, mat44* models, int shader, unsigned count) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    // @fixme: instanced billboards
    mat44 mv; multiply44x2(mv, view, models[0]);
    if( m.billboard ) {
        float d = sqrt(mv[4*0+0] * mv[4*0+0] + mv[4*1+1] * mv[4*1+1] + mv[4*2+2] * mv[4*2+2]);
        if(m.billboard & 4) mv[4*0+0] = d, mv[4*0+1] =  0, mv[4*0+2] = 0;
        if(m.billboard & 2) mv[4*1+0] = 0, mv[4*1+1] = -d, mv[4*1+2] = 0;
        if(m.billboard & 1) mv[4*2+0] = 0, mv[4*2+1] =  0, mv[4*2+2] = d;
    }

    if( count != m.num_instances ) {
        m.num_instances = count;
        m.instanced_matrices = (float*)models;
        model_set_state(m);
    }

    model_set_uniforms(m, shader > 0 ? shader : program, mv, proj, view, models[0]);
    model_draw_call(m);
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
