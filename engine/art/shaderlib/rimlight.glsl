#ifndef RIMLIGHT_GLSL
#define RIMLIGHT_GLSL

vec3 get_rimlight() {
#ifdef RIM
    vec3 n = normalize(mat3(M) * v_normal);  // convert normal to view space
    vec3 p = (M * vec4(v_position,1.0)).xyz; // convert position to view space
    vec3 v = vec3(0,-1,0);
    if (!u_rimambient) {
        v = normalize(u_rimpivot-p);
    }
    float rim = 1.0 - max(dot(v,n), 0.0);
    vec3 col = u_rimcolor*(pow(smoothstep(1.0-u_rimrange.x,u_rimrange.y,rim), u_rimrange.z));
    return col;
#else
    return vec3(0);
#endif
}

#endif