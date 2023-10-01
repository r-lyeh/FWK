//  uniform mat4 view = mat4(1.0);
uniform vec3 lightPos = vec3(1.0);
uniform float doTexture = 1.;
#ifdef VSMCUBE
uniform samplerCube shadowMap; // VSMCUBE
#else
uniform sampler2D shadowMap; // !VSMCUBE
#endif


struct light {
    vec3 position; // world-space
    vec4 diffuse;
    vec4 specular;
    float constantAttenuation, linearAttenuation, quadraticAttenuation;
    };
    
    light light0 = light(
    lightPos,
    vec4(1,1,1,1), // diffuse
    vec4(1,1,1,1), // specular
    1.0, 0.0, 0.0  // attenuation (const, linear, quad)
    );
    
    // From http://fabiensanglard.net/shadowmappingVSM/index.php
    #ifdef VSMCUBE
    float chebyshevUpperBound(float distance, vec3 dir) {
        distance = distance/20 ;
        vec2 moments = texture(shadowMap, dir).rg;
        #else
        float chebyshevUpperBound(float distance, vec4 scPostW) {
            vec2 moments = texture(shadowMap,scPostW.xy).rg;
            #endif
            // Surface is fully lit. as the current fragment is before the light occluder
            if (distance <= moments.x)
            return 1.0;
            
            // The fragment is either in shadow or penumbra. We now use chebyshev's upperBound to check
            // How likely this pixel is to be lit (p_max)
            float variance = moments.y - (moments.x*moments.x);
            //variance = max(variance, 0.000002);
            variance = max(variance, 0.00002);
            
            float d = distance - moments.x;
            float p_max = variance / (variance + d*d);
            
            return p_max;
        }
        
        
        vec4 shadowmap(in vec4 vpeye, in vec4 vneye, in vec2 uv, in vec4 sc) {
            #ifndef VSMCUBE
            return vec4(1.);
            #else
            vec3 fragment = vec3(vpeye);
            vec3 normal   = vec3(normalize(vneye));
            vec3 viewDir  = normalize(-fragment);
            
            // Lighting
            // Convert to eye-space
            vec3 light = vec3(view * vec4(light0.position, 1.0));
            
            #ifdef VSMCUBE
            // Vectors
            vec3 fragmentToLight     = light - fragment;
            vec3 fragmentToLightDir  = normalize(fragmentToLight);
            
            // Shadows
            vec4 fragmentToLight_world = inverse(view) * vec4(fragmentToLightDir, 0.0);
            float shadowFactor = chebyshevUpperBound(length(fragmentToLight), -fragmentToLight_world.xyz);
            #else
            // Shadows
            vec4 scPostW = sc / sc.w;
            scPostW = scPostW * 0.5 + 0.5;
            
            float shadowFactor = 1.0; // Not in shadow
            
            bool outsideShadowMap = sc.w <= 0.0f || (scPostW.x < 0 || scPostW.y < 0) || (scPostW.x >= 1 || scPostW.y >= 1);
            if (!outsideShadowMap) {
                shadowFactor = chebyshevUpperBound(scPostW.z, scPostW);
            }
            #endif
            
            vec4 diffColor = vec4(1,1,1,1);
            #ifdef VSMCUBE
            if(doTexture != 0) diffColor = vec4(vec3(texture(shadowMap, -fragmentToLight_world.xyz).r), 1.0);
            #else
            if(doTexture != 0) diffColor = vec4(vec3(texture(shadowMap, vec2(uv.x, 1.0 - uv.y)).r), 1.0);
            #endif
            
            #if 1
            vec3 positionToLight = light - fragment;
            vec3 lightDir  = normalize(positionToLight);
            
            // Angle between fragment-normal and incoming light
            float cosAngIncidence = dot(lightDir, normal);
            cosAngIncidence = clamp(cosAngIncidence, 0, 1);
            
            float attenuation = 1.0f;
            attenuation = 1.0 / (light0.constantAttenuation + light0.linearAttenuation * length(positionToLight) + light0.quadraticAttenuation * pow(length(positionToLight),2));
            
            vec4 diffuse  = diffColor * light0.diffuse  * cosAngIncidence * attenuation;
            
            vec4 total_lighting;
            total_lighting += vec4(0.1, 0.1, 0.1, 1.0) * diffColor; // Ambient
            total_lighting += diffuse * shadowFactor; // Diffuse
            #else
            vec4 total_lighting = diffColor;
            #endif
            return vec4(clamp(vec3(total_lighting), 0., 1.), 1.0);
            #endif
        }