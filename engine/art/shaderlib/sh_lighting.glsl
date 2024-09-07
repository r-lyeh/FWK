#ifndef SH_LIGHTING_GLSL
#define SH_LIGHTING_GLSL

vec3 sh_lighting(vec3 n) {
    vec3 SHLightResult[9];
    SHLightResult[0] =  0.282095f * u_coefficients_sh[0];
    SHLightResult[1] = -0.488603f * u_coefficients_sh[1] * n.y;
    SHLightResult[2] =  0.488603f * u_coefficients_sh[2] * n.z;
    SHLightResult[3] = -0.488603f * u_coefficients_sh[3] * n.x;
    SHLightResult[4] =  1.092548f * u_coefficients_sh[4] * n.x * n.y;
    SHLightResult[5] = -1.092548f * u_coefficients_sh[5] * n.y * n.z;
    SHLightResult[6] =  0.315392f * u_coefficients_sh[6] * (3.0f * n.z * n.z - 1.0f);
    SHLightResult[7] = -1.092548f * u_coefficients_sh[7] * n.x * n.z;
    SHLightResult[8] =  0.546274f * u_coefficients_sh[8] * (n.x * n.x - n.y * n.y);
    vec3 result = vec3(0.0);
    for (int i = 0; i < 9; ++i)
        result += SHLightResult[i];
    return result;
}

#endif