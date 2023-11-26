uniform mat4 model, view;
uniform sampler2D u_texture2d;
uniform vec3 u_coefficients_sh[9];
uniform bool u_textured = true;
uniform bool u_lit = false;
uniform bool u_matcaps = false;
uniform vec4 u_diffuse = vec4(1.0,1.0,1.0,1.0);

// lightmapping
uniform sampler2D u_lightmap;
uniform bool u_texlit;
uniform bool u_texmod = true;
uniform float u_litboost = 1.0;

in vec3 v_position;
in vec3 v_position_ws;
#ifdef RIM
uniform mat4 M; // RIM
uniform vec3 u_rimcolor = vec3(0.2,0.2,0.2);
uniform vec3 u_rimrange = vec3(0.11,0.98,0.5);
uniform vec3 u_rimpivot = vec3(0,0,0);
uniform bool u_rimambient = true;
#endif
in vec3 v_normal, v_normal_ws;
in vec2 v_texcoord, v_texcoord2;
in vec4 v_color;
out vec4 fragcolor;


{{include-shadowmap}}
in vec4 vpeye;
in vec4 vneye;
in vec4 sc;
vec4 shadowing() {
return shadowmap(vpeye, vneye, v_texcoord, sc);
}

uniform vec3 u_cam_pos;
uniform vec3 u_cam_dir;

uniform int u_num_lights;

struct light_t {
    int type;
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    vec3 pos;
    vec3 dir;
    float power;
    float innerCone;
    float outerCone;

    // falloff
    float constant;
    float linear;
    float quadratic;
};

#define MAX_LIGHTS 16
const int LIGHT_DIRECTIONAL = 0;
const int LIGHT_POINT = 1;
const int LIGHT_SPOT = 2;

uniform light_t u_lights[MAX_LIGHTS];

#ifdef SHADING_PHONG
vec3 shading_phong(light_t l) {
    vec3 lightDir;
    float attenuation = 1.0;

    if (l.type == LIGHT_DIRECTIONAL) {
        lightDir = normalize(-l.dir);
    } else if (l.type == LIGHT_POINT || l.type == LIGHT_SPOT) {
        vec3 toLight = l.pos - v_position_ws;
        lightDir = normalize(toLight);
        float distance = length(toLight);
        attenuation = 1.0 / (l.constant + l.linear * distance + l.quadratic * (distance * distance));

        if (l.type == LIGHT_SPOT) {
            float angle = dot(l.dir, -lightDir);
            if (angle > l.outerCone) {
                float intensity = (angle-l.outerCone)/(l.innerCone-l.outerCone);
                attenuation *= clamp(intensity, 0.0, 1.0);
            } else {
                attenuation = 0.0;
            }
        }
    }

    // fast-rejection for faraway vertices
    if (attenuation <= 0.01) {
        return vec3(0,0,0);
    }

    vec3 n = normalize(v_normal_ws);

    float diffuse = max(dot(n, lightDir), 0.0);

    vec3 halfVec = normalize(lightDir + u_cam_dir);
    float specular = pow(max(dot(n, halfVec), 0.0), l.power);

    return (attenuation*l.ambient + diffuse*attenuation*l.diffuse + specular*attenuation*l.specular);
}
#endif

vec3 lighting() {
    vec3 lit = vec3(0,0,0);
#ifndef SHADING_NONE
    for (int i=0; i<u_num_lights; i++) {
    #ifdef SHADING_PHONG
        lit += shading_phong(u_lights[i]);
    #endif
    }
#endif
    return lit;
}

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

#ifdef LIGHTMAP_BAKING
void main() {
    vec3 n = normalize(v_normal_ws);
    vec4 diffuse;

    if(u_textured) {
        diffuse = texture(u_texture2d, v_texcoord);
    } else {
        diffuse = u_diffuse; // * v_color;
    }

    if (u_texlit) {
        vec4 litsample = texture(u_lightmap, v_texcoord);
        diffuse *= litsample;
    }

    fragcolor = vec4(diffuse.rgb*u_litboost, 1.0);
}
#else
void main() {
    vec3 n = normalize(v_normal_ws);

    vec4 lit = vec4(1.0, 1.0, 1.0, 1.0);
    // SH lighting
    if (!u_texlit) {
        vec3 result = sh_lighting(n);
        if( (result.x*result.x+result.y*result.y+result.z*result.z) > 0.0 ) lit = vec4(result, 1.0);
    }

    // analytical lights (phong shading)
    lit += vec4(lighting(), 0.0);

    // base
    vec4 diffuse;

    if(u_matcaps) {
        vec2 muv = vec2(view * vec4(v_normal_ws, 0))*0.5+vec2(0.5,0.5); // normal (model space) to view space
        diffuse = texture(u_texture2d, vec2(muv.x, 1.0-muv.y));
    } else if(u_textured) {
        diffuse = texture(u_texture2d, v_texcoord);
    } else {
        diffuse = u_diffuse; // * v_color;
    }

    if (u_texlit) {
        vec4 litsample = texture(u_lightmap, v_texcoord);

        if (u_texmod) {
            diffuse *= litsample;
        } else {
            diffuse += litsample;
        }

        diffuse.rgb += sh_lighting(n);
    }
    
    // lighting mix
    fragcolor = diffuse * lit * shadowing();
    
    // rimlight
    #ifdef RIM
    {
        vec3 n = normalize(mat3(M) * v_normal);  // convert normal to view space
        vec3 p = (M * vec4(v_position,1.0)).xyz; // convert position to view space
        vec3 v = vec3(0,-1,0);
        if (!u_rimambient) {
            v = normalize(u_rimpivot-p);
        }
        float rim = 1.0 - max(dot(v,n), 0.0);
        vec3 col = u_rimcolor*(pow(smoothstep(1.0-u_rimrange.x,u_rimrange.y,rim), u_rimrange.z));
        fragcolor += vec4(col, 1.0);}
    #endif
}
#endif
