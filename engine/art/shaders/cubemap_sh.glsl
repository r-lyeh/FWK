layout(local_size_x = 16, local_size_y = 16) in;

uniform sampler2D inputTexture;
layout(std430, binding = 1) buffer SHCoefficients {
    vec3 sh[9];
};
layout(std430, binding = 2) buffer WorkGroupResults {
    vec3 wg_results[];
};

uniform float skyIntensity;
uniform ivec2 textureSize;
uniform int faceIndex;
uniform int pass;

shared vec3 sh_local[9];

const float PI = 3.14159265359;

const vec3 skyDir[6] = vec3[6](
    vec3( 1, 0, 0), vec3(-1, 0, 0),
    vec3( 0, 1, 0), vec3( 0,-1, 0),
    vec3( 0, 0, 1), vec3( 0, 0,-1)
);
const vec3 skyX[6] = vec3[6](
    vec3( 0, 0,-1), vec3( 0, 0, 1),
    vec3( 1, 0, 0), vec3( 1, 0, 0),
    vec3( 1, 0, 0), vec3(-1, 0, 0)
);
const vec3 skyY[6] = vec3[6](
    vec3( 0, 1, 0), vec3( 0, 1, 0),
    vec3( 0, 0,-1), vec3( 0, 0, 1),
    vec3( 0, 1, 0), vec3( 0, 1, 0)
);

void main() {
    if (pass == 0) {
        // First pass: process texels and accumulate in work groups
        if (gl_LocalInvocationIndex == 0) {
            for (int i = 0; i < 9; ++i) {
                sh_local[i] = vec3(0.0);
            }
        }
        barrier();

        ivec2 texel = ivec2(gl_GlobalInvocationID.xy);
        if (any(greaterThanEqual(texel, textureSize))) return;
        
        vec2 uv = (vec2(texel) + 0.5) / vec2(textureSize);
        vec3 n = normalize(skyDir[faceIndex] +
                           skyX[faceIndex] * (2.0 * uv.x - 1.0) +
                           skyY[faceIndex] * (1.0 - 2.0 * uv.y));

        vec3 color = textureLod(inputTexture, uv, 0).rgb;
        float l = length(n);
        vec3 light = color * (1.0 / (l * l * l)) * skyIntensity;

        // Accumulate SH coefficients
        vec3 sh_[9];
        sh_[0] = light * 0.282095f;
        sh_[1] = light * (-0.488603f * n.y * 2.0 / 3.0);
        sh_[2] = light * (0.488603f * n.z * 2.0 / 3.0);
        sh_[3] = light * (-0.488603f * n.x * 2.0 / 3.0);
        sh_[4] = light * (1.092548f * n.x * n.y / 4.0);
        sh_[5] = light * (-1.092548f * n.y * n.z / 4.0);
        sh_[6] = light * (0.315392f * (3.0f * n.z * n.z - 1.0f) / 4.0);
        sh_[7] = light * (-1.092548f * n.x * n.z / 4.0);
        sh_[8] = light * (0.546274f * (n.x * n.x - n.y * n.y) / 4.0);
        
        // Add to local SH coefficients
        for (int i = 0; i < 9; ++i) {
            sh_local[i] += sh_[i];
        }

        barrier();

        // Store work group results
        if (gl_LocalInvocationIndex == 0) {
            uint wg_index = gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x;
            for (int i = 0; i < 9; ++i) {
                wg_results[wg_index * 9 + i] = sh_local[i];
            }
        }
    } else {
        // Second pass: combine work group results
        if (gl_WorkGroupID.x == 0 && gl_WorkGroupID.y == 0) {
            vec3 total_sh[9] = vec3[9](vec3(0), vec3(0), vec3(0), vec3(0), vec3(0), vec3(0), vec3(0), vec3(0), vec3(0));
            uint num_work_groups = gl_NumWorkGroups.x * gl_NumWorkGroups.y;
            
            for (uint i = gl_LocalInvocationIndex; i < num_work_groups; i += gl_WorkGroupSize.x * gl_WorkGroupSize.y) {
                for (int j = 0; j < 9; ++j) {
                    total_sh[j] += wg_results[i * 9 + j];
                }
            }
            
            barrier();
            
            if (gl_LocalInvocationIndex < 9) {
                sh[gl_LocalInvocationIndex] += total_sh[gl_LocalInvocationIndex];
            }
        }
    }
}