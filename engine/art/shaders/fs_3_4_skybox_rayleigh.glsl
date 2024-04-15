uniform vec3 uSunPos;
uniform vec3 uRayOrigin;
uniform float uSunIntensity;
uniform float uPlanetRadius;
uniform float uAtmosphereRadius;
uniform vec3 uRayleighScattering;
uniform float uMieScattering;
uniform float uRayleighScaleHeight;
uniform float uMieScaleHeight;
uniform float uMiePreferredDirection;

in vec3 v_direction;
out vec4 fragcolor;


vec3 atmosphere(vec3 r, vec3 r0, vec3 pSun, float iSun, float rPlanet, float rAtmos, vec3 kRlh, float kMie, float shRlh, float shMie, float g);


void main() {
    vec3 color = atmosphere(
        normalize(v_direction),      // normalized ray direction
        uRayOrigin,                  // ray origin
        uSunPos,                     // position of the sun
        uSunIntensity,               // intensity of the sun
        uPlanetRadius,               // radius of the planet in meters
        uAtmosphereRadius,           // radius of the atmosphere in meters
        uRayleighScattering,         // Rayleigh scattering coefficient
        uMieScattering,              // Mie scattering coefficient
        uRayleighScaleHeight,        // Rayleigh scale height
        uMieScaleHeight,             // Mie scale height
        uMiePreferredDirection       // Mie preferred scattering direction
    );
    
    // Apply exposure.
    color = 1.0 - exp(-1.0 * color);
    
    fragcolor = vec4(color, 1);
}


// [src] https://github.com/wwwtyro/glsl-atmosphere by wwwtyro (Unlicensed)
// For more information, please refer to <http://unlicense.org>


#define PI 3.141592
#define iSteps 16
#define jSteps 8


vec2 rsi(vec3 r0, vec3 rd, float sr) {
    // ray-sphere intersection that assumes
    // the sphere is centered at the origin.
    // No intersection when result.x > result.y
    float a = dot(rd, rd);
    float b = 2.0 * dot(rd, r0);
    float c = dot(r0, r0) - (sr * sr);
    float d = (b*b) - 4.0*a*c;
    if (d < 0.0) return vec2(1e5,-1e5);
    return vec2(
    (-b - sqrt(d))/(2.0*a),
    (-b + sqrt(d))/(2.0*a)
    );
}


vec3 atmosphere(vec3 r, vec3 r0, vec3 pSun, float iSun, float rPlanet, float rAtmos, vec3 kRlh, float kMie, float shRlh, float shMie, float g) {
    // Normalize the sun and view directions.
    pSun = normalize(pSun);
    r = normalize(r);
    
    // Calculate the step size of the primary ray.
    vec2 p = rsi(r0, r, rAtmos);
    if (p.x > p.y) return vec3(0,0,0);
    p.y = min(p.y, rsi(r0, r, rPlanet).x);
    float iStepSize = (p.y - p.x) / float(iSteps);
    
    // Initialize the primary ray time.
    float iTime = 0.0;
    
    // Initialize accumulators for Rayleigh and Mie scattering.
    vec3 totalRlh = vec3(0,0,0);
    vec3 totalMie = vec3(0,0,0);
    
    // Initialize optical depth accumulators for the primary ray.
    float iOdRlh = 0.0;
    float iOdMie = 0.0;
    
    // Calculate the Rayleigh and Mie phases.
    float mu = dot(r, pSun);
    float mumu = mu * mu;
    float gg = g * g;
    float pRlh = 3.0 / (16.0 * PI) * (1.0 + mumu);
    float pMie = 3.0 / (8.0 * PI) * ((1.0 - gg) * (mumu + 1.0)) / (pow(1.0 + gg - 2.0 * mu * g, 1.5) * (2.0 + gg));
    
    // Sample the primary ray.
    for (int i = 0; i < iSteps; i++) {
        
        // Calculate the primary ray sample position.
        vec3 iPos = r0 + r * (iTime + iStepSize * 0.5);
        
        // Calculate the height of the sample.
        float iHeight = length(iPos) - rPlanet;
        
        // Calculate the optical depth of the Rayleigh and Mie scattering for this step.
        float odStepRlh = exp(-iHeight / shRlh) * iStepSize;
        float odStepMie = exp(-iHeight / shMie) * iStepSize;
        
        // Accumulate optical depth.
        iOdRlh += odStepRlh;
        iOdMie += odStepMie;
        
        // Calculate the step size of the secondary ray.
        float jStepSize = rsi(iPos, pSun, rAtmos).y / float(jSteps);
        
        // Initialize the secondary ray time.
        float jTime = 0.0;
        
        // Initialize optical depth accumulators for the secondary ray.
        float jOdRlh = 0.0;
        float jOdMie = 0.0;
        
        // Sample the secondary ray.
        for (int j = 0; j < jSteps; j++) {
            
            // Calculate the secondary ray sample position.
            vec3 jPos = iPos + pSun * (jTime + jStepSize * 0.5);
            
            // Calculate the height of the sample.
            float jHeight = length(jPos) - rPlanet;
            
            // Accumulate the optical depth.
            jOdRlh += exp(-jHeight / shRlh) * jStepSize;
            jOdMie += exp(-jHeight / shMie) * jStepSize;
            
            // Increment the secondary ray time.
            jTime += jStepSize;
        }
        
        // Calculate attenuation.
        vec3 attn = exp(-(kMie * (iOdMie + jOdMie) + kRlh * (iOdRlh + jOdRlh)));
        
        // Accumulate scattering.
        totalRlh += odStepRlh * attn;
        totalMie += odStepMie * attn;
        
        // Increment the primary ray time.
        iTime += iStepSize;
        
    }
    
    // Calculate and return the final color.
    return iSun * (pRlh * kRlh * totalRlh + pMie * kMie * totalMie);
}