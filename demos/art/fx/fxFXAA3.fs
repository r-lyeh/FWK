/*****************************************
 * FXAA 3.11 Implementation - effendiian
 * -------------------------------------
 * FXAA implementation based off of the 
 * work by Timothy Lottes in the Nvidia white paper:
 * https://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf
 *
 * Also used these resources:
 * - https://catlikecoding.com/unity/tutorials/advanced-rendering/fxaa/
 * - https://blog.codinghorror.com/fast-approximate-anti-aliasing-fxaa/
 *****************************************/

// Turn off FXAA.
// #define FXAA 0

// Turn on FXAA.
#define FXAA 1

// Turn on split screen between no-FXAA and FXAA.
// #define FXAA 2

/*
/   FXAA setting, defined via preprocessor variables
*/
#ifndef FXAA_PRESET
    #define FXAA_PRESET 5
    #define FXAA_DEBUG_SKIPPED 0
    #define FXAA_DEBUG_PASSTHROUGH 0
    #define FXAA_DEBUG_HORZVERT 0
    #define FXAA_DEBUG_PAIR 0
    #define FXAA_DEBUG_NEGPOS 0
    #define FXAA_DEBUG_OFFSET 0
    #define FXAA_DEBUG_HIGHLIGHT 0
    #define FXAA_LUMINANCE 1
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_PRESET == 0)
    #define FXAA_EDGE_THRESHOLD      (1.0/4.0)
    #define FXAA_EDGE_THRESHOLD_MIN  (1.0/12.0)
    #define FXAA_SEARCH_STEPS        2
    #define FXAA_SEARCH_ACCELERATION 4
    #define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
    #define FXAA_SUBPIX              1
    #define FXAA_SUBPIX_FASTER       1
    #define FXAA_SUBPIX_CAP          (2.0/3.0)
    #define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_PRESET == 1)
    #define FXAA_EDGE_THRESHOLD      (1.0/8.0)
    #define FXAA_EDGE_THRESHOLD_MIN  (1.0/16.0)
    #define FXAA_SEARCH_STEPS        4
    #define FXAA_SEARCH_ACCELERATION 3
    #define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
    #define FXAA_SUBPIX              1
    #define FXAA_SUBPIX_FASTER       0
    #define FXAA_SUBPIX_CAP          (3.0/4.0)
    #define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_PRESET == 2)
    #define FXAA_EDGE_THRESHOLD      (1.0/8.0)
    #define FXAA_EDGE_THRESHOLD_MIN  (1.0/24.0)
    #define FXAA_SEARCH_STEPS        8
    #define FXAA_SEARCH_ACCELERATION 2
    #define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
    #define FXAA_SUBPIX              1
    #define FXAA_SUBPIX_FASTER       0
    #define FXAA_SUBPIX_CAP          (3.0/4.0)
    #define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_PRESET == 3)
    #define FXAA_EDGE_THRESHOLD      (1.0/8.0)
    #define FXAA_EDGE_THRESHOLD_MIN  (1.0/24.0)
    #define FXAA_SEARCH_STEPS        16
    #define FXAA_SEARCH_ACCELERATION 1
    #define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
    #define FXAA_SUBPIX              1
    #define FXAA_SUBPIX_FASTER       0
    #define FXAA_SUBPIX_CAP          (3.0/4.0)
    #define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_PRESET == 4)
    #define FXAA_EDGE_THRESHOLD      (1.0/8.0)
    #define FXAA_EDGE_THRESHOLD_MIN  (1.0/24.0)
    #define FXAA_SEARCH_STEPS        24
    #define FXAA_SEARCH_ACCELERATION 1
    #define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
    #define FXAA_SUBPIX              1
    #define FXAA_SUBPIX_FASTER       0
    #define FXAA_SUBPIX_CAP          (3.0/4.0)
    #define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_PRESET == 5)
    #define FXAA_EDGE_THRESHOLD      (1.0/8.0)
    #define FXAA_EDGE_THRESHOLD_MIN  (1.0/24.0)
    #define FXAA_SEARCH_STEPS        32
    #define FXAA_SEARCH_ACCELERATION 1
    #define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
    #define FXAA_SUBPIX              1
    #define FXAA_SUBPIX_FASTER       0
    #define FXAA_SUBPIX_CAP          (3.0/4.0)
    #define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif
/*--------------------------------------------------------------------------*/
#define FXAA_SUBPIX_TRIM_SCALE (1.0/(1.0 - FXAA_SUBPIX_TRIM))

// --------------------------------------
// Helper functions.
// --------------------------------------

// ---------------------
// Conversion functions.

// ToVec2
vec2 ToVec2( float value ) { return vec2(value, value); }

// ToVec3
vec3 ToVec3( float value ) { return vec3(value, value, value); }
vec3 ToVec3( vec2 vector, float z ) { return vec3(vector.x, vector.y, z); }
vec3 ToVec3( vec2 vector ) { return ToVec3(vector, 0.0); }

// ToVec4
vec4 ToVec4( vec2 vector, float z, float w ) { return vec4(vector.x, vector.y, z, w); }
vec4 ToVec4( vec2 vector, float z ) { return ToVec4(vector, z, 0.0); }
vec4 ToVec4( vec2 vector ) { return ToVec4(vector, 0.0); }
vec4 ToVec4( vec3 vector, float w ) { return vec4(vector.x, vector.y, vector.z, w); }
vec4 ToVec4( vec3 vector ) { return ToVec4(vector, 0.0); }
vec4 ToVec4( float value, float w ) { return vec4(value, value, value, w); }
vec4 ToVec4( float value ) { return ToVec4(value, 0.0); }

// ---------------------
// Texture sampler functions.

// Return sampled image from a point + offset texel space.
vec4 TextureOffset( sampler2D tex, 
                    vec2 uv, 
                    vec2 offset ) {
    
    // Return color from the specified location.
    return texture(tex, uv + offset); 
        
}

// ---------------------
// Grayscale functions.

// Return grayscaled image based off of the selected color channel.
vec3 Grayscale( vec3 color, int index ) {
    int selectedChannel = clamp(index, 0, 2); // [0]r, [1]g, [2]b.
    return ToVec3(color[selectedChannel]);
}

// Return grayscaled image based off of the selected color channel.
vec4 Grayscale( vec4 color, int index ) {
    int selectedChannel = clamp(index, 0, 3); // [0]r, [1]g, [2]b, [3]a.
    return ToVec4(color[selectedChannel]);
}

// Default to green color channel when no index is supplied.
vec3 Grayscale( vec3 color ) { return Grayscale(color, 1); }
vec4 Grayscale( vec4 color ) { return Grayscale(color, 1); }

// ---------------------
// Luminance functions.

// Map RGB to Luminance linearly.
float LinearRGBLuminance( vec3 color ) {
    
    // Weights for relative luma from here: https://en.wikipedia.org/wiki/Luma_(video)
    vec3 weight = vec3(0.2126729, 0.7151522, 0.0721750);
    
    // Get the dot product:
    // - color.r * weight.r + color.g * weight.g + color.b * weight*b.
    return dot(color, weight);
}

// Luminance based off of the original specification.
float FXAALuminance( vec3 color ) {
    
    #if FXAA_LUMINANCE == 0
    
    return LinearRGBLuminance( color );
    
    #else
    
    return color.g * (0.587/0.299) + color.r;
    
    #endif
}

// ---------------------
// Vertical/Horizontal Edge Test functions.

float FXAAVerticalEdge( float lumaO,
                       float lumaN, 
                       float lumaE, 
                       float lumaS, 
                       float lumaW,
                       float lumaNW,
                       float lumaNE,
                       float lumaSW,
                       float lumaSE ) {
    
    // Slices to calculate.
    float top = (0.25 * lumaNW) + (-0.5 * lumaN) + (0.25 * lumaNE);
    float middle = (0.50 * lumaW ) + (-1.0 * lumaO) + (0.50 * lumaE );
    float bottom = (0.25 * lumaSW) + (-0.5 * lumaS) + (0.25 * lumaSE);
    
    // Return value.
    return abs(top) + abs(middle) + abs(bottom);
}

float FXAAHorizontalEdge( float lumaO,
                       float lumaN, 
                       float lumaE, 
                       float lumaS, 
                       float lumaW,
                       float lumaNW,
                       float lumaNE,
                       float lumaSW,
                       float lumaSE ) {
    
    // Slices to calculate.
    float top = (0.25 * lumaNW) + (-0.5 * lumaW) + (0.25 * lumaSW);
    float middle = (0.50 * lumaN ) + (-1.0 * lumaO) + (0.50 * lumaS );
    float bottom = (0.25 * lumaNE) + (-0.5 * lumaE) + (0.25 * lumaSE);
    
    // Return value.
    return abs(top) + abs(middle) + abs(bottom);
}

// ------------------------
// FXAA specific functions.
// ------------------------

// Entry point for the FXAA process.
vec3 applyFXAA(sampler2D textureSource, 
               vec2 textureDimensions, 
               vec2 pixelPosition,
               vec2 screenResolution) {
    
    // Normalized pixel coordinates (from 0 to 1).
    vec2 uv = pixelPosition / screenResolution;
    
    // Calculate distance between pixels in texture space.
    vec2 texel = vec2(1.0, 1.0) / textureDimensions;
    
    // Caculate the luminance.
    // float luma = FXAALuminance(rgbO.xyz);
    // float luma = LinearRGBLuminance(clamp(rgbO.xyz, 0.0, 1.0));
    
    //-------------------------
    // 1. LOCAL CONTRAST CHECK
    
    // Sample textures from cardinal directions.
    vec3 rgbN = TextureOffset(textureSource, uv, vec2(0, -texel.y)).rgb; // NORTH
    vec3 rgbW = TextureOffset(textureSource, uv, vec2(-texel.x, 0)).rgb; // WEST
    vec3 rgbO = TextureOffset(textureSource, uv, vec2(0, 0)).rgb; // ORIGIN
    vec3 rgbE = TextureOffset(textureSource, uv, vec2(texel.x, 0)).rgb; // EAST
    vec3 rgbS = TextureOffset(textureSource, uv, vec2(0, texel.y)).rgb; // SOUTH
    
    #if FXAA == 0
    return rgbO; // Skip FXAA if it is off.
    #endif    
    
    // Calculate the luminance for each sampled value.
    float lumaN = FXAALuminance(rgbN);
    float lumaW = FXAALuminance(rgbW);
    float lumaO = FXAALuminance(rgbO);
    float lumaE = FXAALuminance(rgbE);
    float lumaS = FXAALuminance(rgbS);
    
    // Calculate the minimum luma range.
    float minLuma = min( lumaO, min( min( lumaN, lumaW ), min( lumaS, lumaE ) ) );
    float maxLuma = max( lumaO, max( max( lumaN, lumaW ), max( lumaS, lumaE ) ) );
    float localContrast = maxLuma - minLuma;    
    
    // Check for early exit.
    if(localContrast < max( FXAA_EDGE_THRESHOLD_MIN, maxLuma * FXAA_EDGE_THRESHOLD )) {
        
        #if FXAA_DEBUG_SKIPPED
                
        return vec3(0);
        
        #else
        
        return rgbO;
        
        #endif
    }
        
    //-------------------------
    // 2. SUB-PIXEL ALIASING TEST
    
    // Calculate the pixel contrast ratio.
    // - Sub-pixel aliasing is detected by taking the ratio of the 
    // pixel contrast over the local contrast. This ratio nears 1.0
    // in the presence of single pixel dots and otherwise falls off
    // towards 0.0 as more pixels contribute to an edge. This ratio
    // is transformed into the amount of lowpass filter to blend in
    // at the end of the algorithm.
    
    #if FXAA_SUBPIX > 0
    
    // Calculate sum of local samples for the lowpass.
    vec3 rgbL = (rgbN + rgbW + rgbO + rgbE + rgbS);
    
        #if FXAA_SUBPIX_FASTER

        // Average the lowpass now since this skips the addition of the diagonal neighbors (NW, NE, SW, SE).
        rgbL *= (1.0/5.0);

        #endif    

    // Calculate the lowpass luma.
    // - Lowpass luma is calculated as the average between the luma of neigboring pixels.
    float lumaL = (lumaN + lumaW + lumaS + lumaE) * 0.25;

    // Calculate the pixel contrast.
    // - Pixel contrast is the abs() difference between origin pixel luma and lowpass luma of neighbors.
    float pixelContrast = abs(lumaL - lumaO);
    
    // Remember: 
    // - pixel contrast is the origin - lowpass(neighbors).
    // - local contrast is the min(origin + neighbors) - max(origin + neighbors) < threshold.
   
    // Calculate the ratio between the pixelContrast and localContrast.
    float contrastRatio = pixelContrast / localContrast;
    float lowpassBlend = 0.0; // Default is zero. Will be changed depending on subpixel level.
    
        #if FXAA_SUBPIX == 1
    
        // Normal subpixel aliasing. Set based on FXAA algorithm for subpixel aliasing.
        lowpassBlend = max( 0.0, contrastRatio - FXAA_SUBPIX_TRIM ) * FXAA_SUBPIX_TRIM_SCALE;
        lowpassBlend = min( FXAA_SUBPIX_CAP, lowpassBlend );
    
        #elif FXAA_SUBPIX == 2
    
        // Full force subpixel aliasing. Set blend to ratio.
        lowpassBlend = contrastRatio;
    
        #endif
    
    #endif
    
    // Show selected pixels if debug mode is active.
    #if FXAA_DEBUG_PASSTHROUGH
    
        #if FXAA_SUBPIX > 0    
    
        return vec3(localContrast, lowpassBlend, 0.0);
    
        #else 
        
        return vec3(localContrast, 0.0, 0.0);   
    
        #endif
    
    #endif
    
    //-------------------------
    // 3. VERTICAL & HORIZONTAL EDGE TEST
    
    // Sample the additional diagonal neighbors.
    vec3 rgbNW = TextureOffset(textureSource, uv, vec2(-texel.x, -texel.y)).rgb; // NORTH-WEST
    vec3 rgbNE = TextureOffset(textureSource, uv, vec2(texel.x, -texel.y)).rgb; // NORTH-EAST
    vec3 rgbSW = TextureOffset(textureSource, uv, vec2(-texel.x, texel.y)).rgb; // SOUTH-WEST
    vec3 rgbSE = TextureOffset(textureSource, uv, vec2(texel.x, texel.y)).rgb; // SOUTH-EAST
    
    // Average additional neighbors when sub-pix aliasing is on and it isn't in 'fast' mode.
    #if FXAA_SUBPIX > 0
        #if FXAA_SUBPIX_FASTER == 0
            // Add missing neighbors and average them.
            rgbL += (rgbNW + rgbNE + rgbSW + rgbSE);  
            rgbL *= (1.0/9.0);
        #endif
    #endif
    
    // Calculate luma for additional neighbors.
    float lumaNW = FXAALuminance(rgbNW);
    float lumaNE = FXAALuminance(rgbNE);
    float lumaSW = FXAALuminance(rgbSW);
    float lumaSE = FXAALuminance(rgbSE);
    
    // Calculate the vertical and horizontal edges. (Uses algorithm from FXAA white paper).
    float edgeVert = FXAAVerticalEdge(lumaO, lumaN, lumaE, lumaS, lumaW, lumaNW, lumaNE, lumaSW, lumaSE);
    float edgeHori = FXAAHorizontalEdge(lumaO, lumaN, lumaE, lumaS, lumaW, lumaNW, lumaNE, lumaSW, lumaSE);
    
    // Check if edge is horizontal.
    bool isHorizontal = edgeHori >= edgeVert;
    
    #if FXAA_DEBUG_HORZVERT
    if(isHorizontal) 
    {
        return vec3(1.0, 0.75, 0.0);
    } 
    else 
    {
        return vec3(0.10, 0.10, 1.0);
    }
    #endif
    
    //-------------------------
    // 4. FIND HIGHEST CONTRAST PAIR 90deg TO EDGE
    
    // Contain the appropriate sign for the top left.
    float edgeSign = isHorizontal ? -texel.y : -texel.x; // Note, if isHorizontal == true, -texel.y is applied (not -texel.x).
    
    // Calculate the gradients. The luma used changes based on the horizontal edge status.
    float gradientNeg = isHorizontal ? abs(lumaN - lumaO) : abs(lumaW - lumaO);
    float gradientPos = isHorizontal ? abs(lumaS - lumaO) : abs(lumaE - lumaO); 
    
    // Calculate the luma based on its direction.
    // It is an average of the origin and the luma in the respective direction.
    float lumaNeg = isHorizontal ? ((lumaN + lumaO) * 0.5) : ((lumaW + lumaO) * 0.5);    
    float lumaPos = isHorizontal ? ((lumaS + lumaO) * 0.5) : ((lumaE + lumaO) * 0.5);
    
    // Select the highest gradient pair.
    bool isNegative = (gradientNeg >= gradientPos);
    float gradientHighest = isNegative ? gradientNeg : gradientPos; // Assign higher pair.
    float lumaHighest = isNegative ? lumaNeg : lumaPos;
    
    // If gradient pair in the negative direction is higher, flip the edge sign.
    if(isNegative) { edgeSign *= -1.0; }
    
    #if FXAA_DEBUG_PAIR
    return isHorizontal ? vec3(0.0, gradientHighest, lumaHighest) : vec3(0.0, lumaHighest, gradientHighest); 
    #endif
    
    //-------------------------
    // 5. END-OF-EDGE SEARCH
    
    // Select starting point.
    vec2 pointN = vec2(0.0, 0.0);
    pointN.x = uv.x + (isHorizontal ? 0.0 : edgeSign * 0.5);
    pointN.y = uv.y + (isHorizontal ? edgeSign * 0.5 : 0.0);
    
    // Assign search limiting values.
    gradientHighest *= FXAA_SEARCH_THRESHOLD;
    
    // Prepare variables for search.
    vec2 pointP = pointN; // Start at the same point.
    vec2 pointOffset = isHorizontal ? vec2(texel.x, 0.0) : vec2(0.0, texel.y);
    float lumaNegEnd = lumaNeg;
    float lumaPosEnd = lumaPos;
    bool searchNeg = false;
    bool searchPos = false;
    
    // Apply values based on FXAA flags.
    if(FXAA_SEARCH_ACCELERATION == 1) {
        
        pointN += pointOffset * vec2(-1.0);
        pointP += pointOffset * vec2(1.0);
        // pointOffset *= vec2(1.0);
        
    } else if(FXAA_SEARCH_ACCELERATION == 2) {    
        
        pointN += pointOffset * vec2(-1.5);
        pointP += pointOffset * vec2(1.5);
        pointOffset *= vec2(2.0);
        
    } else if(FXAA_SEARCH_ACCELERATION == 3) {  
        
        pointN += pointOffset * vec2(-2.0);
        pointP += pointOffset * vec2(2.0);
        pointOffset *= vec2(3.0);
        
    } else if(FXAA_SEARCH_ACCELERATION == 4) { 
        
        pointN += pointOffset * vec2(-2.5);
        pointP += pointOffset * vec2(2.5);
        pointOffset *= vec2(4.0);
        
    }
    
    // Perform the end-of-edge search.
    for(int i = 0; i < FXAA_SEARCH_STEPS; i++) 
    {
        if(FXAA_SEARCH_ACCELERATION == 1) {            
            if(!searchNeg) { lumaNegEnd = FXAALuminance(texture(textureSource, pointN).rgb); }
            if(!searchPos) { lumaPosEnd = FXAALuminance(texture(textureSource, pointP).rgb); } 
        } 
        else
        {
            if(!searchNeg) { lumaNegEnd = FXAALuminance(textureGrad(textureSource, pointN, pointOffset, pointOffset).rgb); }
            if(!searchPos) { lumaPosEnd = FXAALuminance(textureGrad(textureSource, pointP, pointOffset, pointOffset).rgb); } 
        }
        
        // Search for significant change in luma compared to current highest pair.
#if 0 // original
        searchNeg = searchNeg || (abs(lumaNegEnd - lumaNeg) >= gradientNeg);
        searchPos = searchPos || (abs(lumaPosEnd - lumaPos) >= gradientPos);
#else // iradicator's fix
        searchNeg = searchNeg || (abs(lumaNegEnd - lumaHighest) >= gradientHighest);
        searchPos = searchPos || (abs(lumaPosEnd - lumaHighest) >= gradientHighest);
#endif        
        
        // Display debug information regarding edges.
        #if FXAA_DEBUG_NEGPOS
        
        if(searchNeg) { 
            return vec3(abs(lumaNegEnd - gradientNeg), 0.0, 0.0);  
        } else if(searchPos) { 
            return vec3(0.0, 0.0, abs(lumaPosEnd - gradientPos));  
        }

        #endif
        
        // Determine if search is over early.
        if(searchNeg && searchPos) { break; }
        
        // If still searching, increment offset.
        if(!searchNeg) { pointN -= pointOffset; }
        if(!searchPos) { pointP += pointOffset; }
    }
    
    //-------------------------
    // 6. SUB-PIXEL SHIFT
    
    // Determine if sub-pixel center falls on positive or negative side.
    float distanceNeg = isHorizontal ? uv.x - pointN.x : uv.y - pointN.y;
    float distancePos = isHorizontal ? pointP.x - uv.x : pointP.y - uv.y;
    bool isCloserToNegative = distanceNeg < distancePos;
    
    // Assign respective luma.
    float lumaEnd = isCloserToNegative ? lumaNegEnd : lumaPosEnd;
    
    // Check if pixel is in area that receives no filtering.
    if( ((lumaO - lumaNeg) < 0.0) == ((lumaEnd - lumaNeg) < 0.0) ) {
        edgeSign = 0.0;
    }
    
    // Compute sub-pixel offset and filter span.
    float filterSpanLength = (distancePos + distanceNeg);
    float filterDistance = isCloserToNegative ? distanceNeg : distancePos;
    float subpixelOffset = ( 0.5 + ( filterDistance * (-1.0 / filterSpanLength) ) ) * edgeSign;
    
    #if FXAA_DEBUG_OFFSET  
    
    if(subpixelOffset < 0.0) {
        return isHorizontal ? vec3(1.0, 0.0, 0.0) : vec3(1.0, 0.7, 0.1); // neg-horizontal (red) : neg-vertical (gold)
    } 
    
    if(subpixelOffset > 0.0) {
        return isHorizontal ? vec3(0.0, 0.0, 1.0) : vec3(0.1, 0.3, 1.0); // pos-horizontal (blue) : pos-vertical (skyblue)
    }
        
    #endif
    
    // Resample using the subpixel offset.
    vec3 rgbOffset = textureLod(textureSource, vec2( uv.x + (isHorizontal ? 0.0 : subpixelOffset), uv.y + (isHorizontal ? subpixelOffset : 0.0)), 0.0).rgb;
    
    // return vec3((lumaN + lumaS + lumaE + lumaW + lumaNW + lumaNE + lumaSW + lumaSE) * (1.0/9.0));
    
    #if FXAA_DEBUG_HIGHLIGHT
    
    return isHorizontal ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 1.0, 0.0);
    
    #endif
    
    // Return the FXAA effect.
    #if FXAA_SUBPIX == 0
    
    return vec3(rgbOffset);
    
    #else
    
    return mix(rgbOffset, rgbL, lowpassBlend);
    
    #endif
}

// ------------------------
// Main function.
// ------------------------

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{ 
    
    #if (FXAA == 2)
    
    vec2 uv = fragCoord/iResolution.xy; // Normalized pixel coordinates (from 0 to 1)
    vec3 resultFXAA = vec3(1.0);
    
    float speed = 0.45;
    vec2 extents = vec2(0.1, 0.8);
    float divisor = ( ((sin(iTime * speed) * 0.5) + 0.5) * extents.y ) + extents.x;
    float increment = 0.005;
    
    float divNeg = divisor - increment;
    float divPos = divisor + increment;
    
    if(uv.x >= divNeg && uv.x <= divPos) { resultFXAA = vec3(0.1); }
    if(uv.x < divNeg) { resultFXAA = mix(texture(iChannel0, vec2(uv.x, uv.y)).xyz, vec3(0.9, 0.9, 0.9), 0.1); }
    if(uv.x > divPos) { resultFXAA = applyFXAA(iChannel0, iChannelResolution[0].xy, fragCoord, iResolution.xy); }   
    
    #else
        
    // Calculuate the FXAA value for the whole screen.
    vec3 resultFXAA = applyFXAA(iChannel0, iChannelResolution[0].xy, fragCoord, iResolution.xy);
    
    #endif
    
    // Return the sampled pixel.
    fragColor = ToVec4(resultFXAA, 1.0);    
}