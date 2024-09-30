#ifndef MODEL_VS_GLSL
#define MODEL_VS_GLSL
#define VS_PASS

#ifndef MAX_BONES
#define MAX_BONES 110
#endif
uniform mat3x4 vsBoneMatrix[MAX_BONES];
uniform bool SKINNED; /// set:0
uniform bool u_instanced; /// set:0
uniform mat4 M; // RIM
uniform mat4 VP;
uniform mat4 P;
uniform vec3 u_cam_pos;
uniform vec3 u_cam_dir;
uniform float frame_time;
uniform int u_billboard;

// for blendshapes
#ifndef MAX_BLENDSHAPES
#define MAX_BLENDSHAPES 16
#endif
uniform vec4 blend_weights[MAX_BLENDSHAPES]; // @todo: implement me
uniform float f_num_blend_shapes; // @todo: implement me
uniform MEDIUMP sampler2DArray blend_shapes; // @todo: implement me

in vec3 att_position; // @todo: reorder ass2iqe to emit p3 n3 u2 t3 b3 c4B i4 w4 instead
in vec2 att_texcoord;
in vec3 att_normal;
in vec4 att_tangent; // vec3 + bi sign
in mat4 att_instanced_matrix; // for instanced rendering
in vec4 att_indexes; // @fixme: gles might use ivec4 instead?
in vec4 att_weights; // @todo: downgrade from float to byte
in float att_vertexindex; // for blendshapes
in vec4 att_color;
in vec3 att_bitangent; // @todo: remove? also, ass2iqe might output this
in vec2 att_texcoord2;
out vec4 v_color;
out vec3 v_position, v_position_ws;
out vec3 v_normal, v_normal_ws;
out vec2 v_texcoord, v_texcoord2;
out vec3 v_tangent, v_tangent_ws;
out vec3 v_tangent_view, v_tangent_world;
out vec3 v_binormal;
out vec3 v_viewpos;
out vec3 v_to_camera;
out vec3 v_vertcolor;
out float v_depth;
out mat3 v_tbn;
// shadow
uniform mat4 model, view, inv_view;
uniform mat4 cameraToShadowProjector;
uniform mat4 cameraToShadowView;
out vec4 vpeye;
out vec4 vneye;
void do_shadow(mat4 modelMat, vec3 objPos, vec3 objNormal) {
#ifdef SHADOW_CAST
    gl_Position = cameraToShadowProjector * modelMat * vec4(objPos, 1.0f);
    v_position = (cameraToShadowView * modelMat * vec4(objPos, 1.0f)).xyz;
#else
    vpeye = view * modelMat * vec4(objPos, 1.0f);
    vneye = view * modelMat * vec4(objNormal, 0.0f);
#endif
}

vec3 get_object_pos() {
    vec3 objPos;
    if(!SKINNED) {
        objPos = att_position;
        v_normal = att_normal;
    } else {
        mat3x4 m = vsBoneMatrix[int(att_indexes.x)] * att_weights.x;
        m += vsBoneMatrix[int(att_indexes.y)] * att_weights.y;
        m += vsBoneMatrix[int(att_indexes.z)] * att_weights.z;
        m += vsBoneMatrix[int(att_indexes.w)] * att_weights.w;
        objPos = vec4(att_position, 1.0) * m;
        
        // blendshapes
        // objPos += evaluate_blend_shape(int(att_vertexindex));
        
        v_normal = vec4(att_normal, 0.0) * m;
        //@todo: tangents
    }

    return objPos;
}

struct billboard_t {
    mat4 modelView;
    mat4 l_model;
};

billboard_t setup_billboard(mat4 modelView, mat4 l_model) {
    if(u_billboard > 0) {
        vec3 cameraPosition = -transpose(mat3(view)) * view[3].xyz;
        vec3 lookDir = normalize(cameraPosition - v_position_ws);

        vec3 up = vec3(modelView[0][1], modelView[1][1], modelView[2][1]);
        vec3 right = normalize(cross(up, lookDir));
        up = cross(lookDir, right);

        vec3 scale;
        scale.x = length(vec3(l_model[0]));
        scale.y = length(vec3(l_model[1]));
        scale.z = length(vec3(l_model[2]));
        // scale.x *= sign(l_model[0][0]);
        // scale.y *= sign(l_model[1][1]);
        // scale.z *= sign(l_model[2][2]);

        mat4 billboardRotation = mat4(
            vec4(right * scale.x, 0.0),
            vec4(-up * scale.y, 0.0),
            vec4(-lookDir * scale.z, 0.0),
            vec4(0.0, 0.0, 0.0, 1.0)
        );

        if((u_billboard & 0x4) != 0) l_model[0] = billboardRotation[0];
        if((u_billboard & 0x2) != 0) l_model[1] = billboardRotation[1];
        if((u_billboard & 0x1) != 0) l_model[2] = billboardRotation[2];
        modelView = view * l_model;
    }
    billboard_t bb;
    bb.modelView = modelView;
    bb.l_model = l_model;
    return bb;
}

#endif