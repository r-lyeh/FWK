#include "model_vs.glsl"

// lights
#include "light.glsl"

void main() {
    vec3 objPos = get_object_pos();

    mat4 model_mat = att_instanced_matrix;
    if (!u_instanced) {
        model_mat = M;
    }
    
    // Set attributes up
    v_normal_ws = normalize(vec3(model_mat * vec4(v_normal, 0.))); // normal to world/model space
    v_normal = normalize(v_normal);
    v_position = att_position;
    v_texcoord = att_texcoord;
    v_texcoord2 = att_texcoord2;
    v_color = att_color;
    mat4 modelView = view * model_mat;
    mat4 l_model = model_mat;
    v_position_ws = (l_model * vec4( objPos, 1.0 )).xyz;
    vec3 binormal = cross(att_normal, att_tangent.xyz) * att_tangent.w;
    v_binormal = normalize(mat3(model_mat) * binormal);

    // Optional: Billboarding
    {
        billboard_t bb = setup_billboard(modelView, l_model);
        modelView = bb.modelView;
        l_model = bb.l_model;
    }

    v_position_ws = (l_model * vec4( objPos, 1.0 )).xyz;


    vec3 T = normalize(mat3(l_model) * att_tangent.xyz);
    vec3 B = normalize(mat3(l_model) * binormal);
    vec3 N = normalize(mat3(l_model) * att_normal);
    mat3 TBN = transpose(mat3(T, B, N));

    v_tangent = normalize(mat3(l_model) * att_tangent.xyz);
    v_tangent_ws = normalize(mat3(modelView) * att_tangent.xyz);
    v_tangent_view = TBN * u_cam_pos;
    v_tangent_view.y = -v_tangent_view.y;
    v_tangent_world = TBN * v_position_ws;
    v_tangent_world.y = -v_tangent_world.y;
    v_tbn = TBN;

    // Compute lighting (vertex-lit models)
    material_t dummy_mat;
    v_vertcolor = lighting(dummy_mat);

    // Compute final position and camera vector
    vec4 finalPos = modelView * vec4( objPos, 1.0 );
    vec3 to_camera = normalize( -finalPos.xyz );
    v_to_camera = mat3( inv_view ) * to_camera;

    // Set final position
    v_depth = -finalPos.z;
    gl_Position = P * finalPos;

    // Prepare shadow data for shadow mapping
    do_shadow(model_mat, objPos, v_normal);
}