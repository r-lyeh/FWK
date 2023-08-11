#version 330 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_texcoord;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in vec4 in_tangent; // vec3 + bi sign
//in vec3 in_binormal;

out vec3 out_normal;
out vec3 out_tangent;
out vec3 out_binormal;
out vec2 out_texcoord;
out vec3 out_worldpos;
out vec3 out_viewpos;
out vec3 out_to_camera;

uniform mat4x4 mat_projection;
uniform mat4x4 mat_view;
uniform mat4x4 mat_view_inverse;
uniform mat4x4 mat_world;

void main()
{
  vec4 o = vec4( in_pos.x, in_pos.y, in_pos.z, 1.0 );
  o = mat_world * o;
  out_worldpos = o.xyz;
  o = mat_view * o;
  out_viewpos = o.xyz;

  vec3 to_camera = normalize( -o.xyz );
  out_to_camera = mat3( mat_view_inverse ) * to_camera;

  o = mat_projection * o;
  gl_Position = o;

#if 0
      // compute tangent T and bitangent B
      vec3 Q1 = dFdx(in_pos);
      vec3 Q2 = dFdy(in_pos);
      vec2 st1 = dFdx(in_texcoord);
      vec2 st2 = dFdy(in_texcoord);

      vec3 T = normalize(Q1*st2.t - Q2*st1.t);
      vec3 B = normalize(-Q1*st2.s + Q2*st1.s);
      vec3 in_binormal = B;
#else
      vec3 in_binormal = cross(in_normal, in_tangent.xyz) * in_tangent.w;
#endif

  out_normal = normalize( mat3( mat_world ) * in_normal );
  out_tangent = normalize( mat3( mat_world ) * in_tangent.xyz );
  out_binormal = normalize( mat3( mat_world ) * in_binormal );
  out_texcoord = in_texcoord;
}
