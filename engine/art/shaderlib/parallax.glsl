#ifndef PARALLAX_GLSL
#define PARALLAX_GLSL

uniform float parallax_scale; /// set:0.025
uniform float parallax_min_layers; /// set:15.0
uniform float parallax_max_layers; /// set:32.0
uniform float parallax_shadow_power; /// set:4.0

vec2 parallax(vec2 texCoords, vec3 viewDir)
{
#ifdef SHADING_PBR
    float numLayers = mix(parallax_max_layers, parallax_min_layers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * parallax_scale; 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = sample_colormap(map_parallax, currentTexCoords).r;
      
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = sample_colormap(map_parallax, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = sample_colormap(map_parallax, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
#else
    return texCoords;
#endif
}

float parallax_shadowing(in vec3 normal, in vec3 lightDir, in vec2 initialTexCoords)
{
#ifdef SHADING_PBR
 float initialHeight = sample_colormap(map_parallax, initialTexCoords).r;
 // shadowMultiplier = 0.0, no shadow -> in light
 // shadowMultiplier = 1.0, shadow -> outside light
 float shadowMultiplier = 0.0;

 // calculate lighting only for surface oriented to the light source
 if(dot(vec3(0.0, 0.0, 1.0), lightDir) > 0)
 {
  // calculate initial parameters
  float numSamplesUnderSurface = 0;
  shadowMultiplier = 0;
  float numLayers = mix(parallax_max_layers, parallax_min_layers, abs(dot(vec3(0.0, 0.0, 1.0), lightDir)));
  float layerHeight = initialHeight / numLayers;
  vec2 texStep = parallax_scale * lightDir.xy / lightDir.z / numLayers;

  // current parameters
  float currentLayerHeight = initialHeight - layerHeight;
  vec2 currentTexCoords = initialTexCoords + texStep;
  float depthFromTexture = sample_colormap(map_parallax, currentTexCoords).r;
  int stepIndex = 1;
  
  // while point is below depth 0.0
  while(currentLayerHeight > 0.0)
  { 
   // if point is under the surface
   if(depthFromTexture < currentLayerHeight)
   {
    // calculate partial shadowing factor
    numSamplesUnderSurface += 1;
    float newShadowMultiplier = (currentLayerHeight - depthFromTexture) * (1.0 - stepIndex / numLayers);
    shadowMultiplier = max(shadowMultiplier, newShadowMultiplier);
   }

   // offset to the next layer
   stepIndex += 1;
   currentLayerHeight -= layerHeight;
   currentTexCoords += texStep;
   depthFromTexture = sample_colormap(map_parallax, currentTexCoords).r;
  }

  // Shadowing factor should be 0 if there were no points under the surface
  if(numSamplesUnderSurface < 1)
   shadowMultiplier = 0; 
 }

 return shadowMultiplier;
#else
 return 1.0;
#endif
}

#endif
