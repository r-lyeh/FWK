#ifndef MAX_BONES
#define MAX_BONES 110
#endif
uniform mat3x4 vsBoneMatrix[MAX_BONES];
uniform bool SKINNED = false;
uniform mat4 M; // RIM
uniform mat4 VP;
uniform mat4 P;
uniform int u_billboard;

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
#ifndef MAX_BLENDSHAPES
#define MAX_BLENDSHAPES 16
#endif
uniform vec4 blend_weights[MAX_BLENDSHAPES]; // @todo: implement me
uniform float f_num_blend_shapes; // @todo: implement me
uniform sampler2DArray blend_shapes; // @todo: implement me


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




// shadow
uniform mat4 model, view;
uniform mat4 cameraToShadowProjector;
out vec4 vneye;
out vec4 vpeye;
out vec4 sc;
void do_shadow() {
    vneye = view * model * vec4(att_normal,   0.0f);
    vpeye = view * model * vec4(att_position, 1.0);
    sc = cameraToShadowProjector * model * vec4(att_position, 1.0f);
}


// blendshapes
vec3 evaluate_blend_shape(int vertex_index) {
    ivec2 coord = ivec2(vertex_index & (2048 - 1), vertex_index >> 11);
    int num_blend_shapes = int(f_num_blend_shapes);
    vec3 offset = vec3(0.0);
    for (int i = 0; i < num_blend_shapes; i++) {
        vec4 packedw = blend_weights[i >> 2];
        float weight = packedw[i & 3];
        offset += weight * texelFetch(blend_shapes, ivec3(coord, i), 0).xyz;
    }
    return offset;
}


void main() {
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
    
    //   vec3 tangent = att_tangent.xyz;
    //   vec3 bitangent = cross(att_normal, att_tangent.xyz) * att_tangent.w;
    v_normal_ws = normalize(vec3(model * vec4(v_normal, 0.))); // normal to world/model space
    v_normal = normalize(v_normal);
    v_position = att_position;
    v_texcoord = att_texcoord;
    v_texcoord2 = att_texcoord2;
    v_color = att_color;
    mat4 modelView = view * att_instanced_matrix;
    mat4 l_model = att_instanced_matrix;
    v_position_ws = (l_model * vec4( objPos, 1.0 )).xyz;

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
    v_position_ws = (l_model * vec4( objPos, 1.0 )).xyz;
    gl_Position = P * modelView * vec4( objPos, 1.0 );
    do_shadow();
}