// MIT License

// A simple debug tool for inspecting buffers pixel values

// How to use:
// 1. Replace the image pass with the code below, move the image pass to a different buffer.
// 2. Set the Channel0 binding to the previous image pass buffer (or any other intermediate buffer or texture).
// 3. Select one of the following modes:

//#define MODE 1 // Click to set origin, drag to scale.
//#define MODE 2 // Click to scale and set origin, drag to pan.
//#define MODE 3 // Click to scale, drag to set origin.

#define MODE 1


// Enable thumbnail preview, this can be removed
//#define ENABLE_PREVIEW


const float ClickScale = 100.0;

const float GridVisibleScale = 6.0;
const float ValueVisibleScale = 8.0;
const float FontScale = 2.0;

const float MaskContrastThreshold = 0.5;

// 4x6 bit masks
const int GlyphWidth = 4;
const int GlyphHeight = 6;
const int[] DigitGlyphs = int[] (6920598, 2499111, 6885967, 6889878, 1268209, 16310678, 6875542, 15803460, 6908310, 6919958);
const int PointGlyph = 2;
const int MinusGlyph = 3840;

const int MaxCharacters = 9;
const int CharacterSpacing = 1;
const int LineWidth = MaxCharacters * (GlyphWidth + CharacterSpacing) - CharacterSpacing;

const vec2 CellPadding = vec2(2.0, 4.0);
const int RowSpacing = 5;
const int RowsCount = 4;
const int RowsHeight = RowsCount * (GlyphHeight + RowSpacing) - RowSpacing;
const float MinValueScale = 1.0 / max(float(LineWidth) + 2.0 * CellPadding.x, float(RowsHeight) + 2.0 * CellPadding.y);

const float[] Pow10 = float[] ( 0.0000000001, 0.000000001, 0.00000001, 0.0000001, 0.000001, 0.00001, 0.0001, 0.001, 0.01, 0.1, 1.0, 10.0, 100.0, 10.0 * ClickScale, 10000.0, 100000.0, 1000000.0, 10000000.0, 100000000.0, 1000000000.0, 10000000000.0 );

float grayscale(vec3 color)
{
    return dot(color, vec3(0.299, 0.587, 0.114));
}

vec3 mixColorMask(vec3 color, float mask)
{
    return mask > 0.01 ? mix(color, vec3(step(grayscale(color), MaskContrastThreshold)), mask) : color;
}

int getGlyphMask(ivec2 pos, int glyph)
{
    if (pos.x >= 0 && pos.x < GlyphWidth && pos.y >= 0 && pos.y < GlyphHeight)
    {
        glyph >>= GlyphWidth * (GlyphHeight - 1 - pos.y) + GlyphWidth - 1 - pos.x;
        return glyph & 1;
    }

    return 0;
}

int getGlyph(int index, float n)
{
    if (n < 0.0)
    {
        if (index == 0)
        {
            return MinusGlyph;
        }

        n = -n;
        index--; // skip minus position
    }

    if (n < 1.0)
    {
        index++; // skip leading zero
    }

    if (n < Pow10[0])
    {
        return index == 1 ? PointGlyph : DigitGlyphs[0];
    }

    //int scale = max(0, int(floor(log(n) / log(10.0))));
    int scale = 0;
    while (scale < 10 && n >= Pow10[scale + 11])
    {
        scale++;
    }

    if (index == scale + 1)
    {
        return PointGlyph;
    }

    if (index > scale)
    {
        index--; // skip decimal point position
        n = fract(n);
    }

    //int digit = int(n * pow(10.0, float(index - scale))) % 10;
    int digit = int(n * Pow10[clamp(index - scale + 10, 0, 20)]) % 10;

    return DigitGlyphs[digit];
}

int getNumberMask(ivec2 pos, float n)
{
    if (pos.x < 0 || pos.x >= LineWidth || pos.y < 0 || pos.y >= GlyphHeight)
    {
        return 0;
    }

    int index = pos.x / (GlyphWidth + CharacterSpacing);
    pos.x -= index * (GlyphWidth + CharacterSpacing);

    int glyph = getGlyph(index, n);

    return getGlyphMask(pos, glyph);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
#ifdef ENABLE_PREVIEW
    bool preview = dot(iMouse, iMouse) < 0.1;
#else
    bool preview = false;
#endif

    if (iMouse.z <= 0.0 && !preview)
    {
        fragColor = texelFetch(iChannel0, ivec2(fragCoord), 0);
        return;
    }

    vec2 mouse = iMouse.xy + 0.5;
    vec2 origin = abs(iMouse.zw) + 0.5;

#ifdef ENABLE_PREVIEW
    if (preview)
    {
        origin = floor(iResolution.xy * 0.5) + 0.5;
        mouse = origin;
        mouse.y += (iResolution.y * (iResolution.y / 1.5 - 1.0) / (10.0 * ClickScale)) * (0.5 - 0.5 * cos(iTime * 0.3));
    }
#endif

#if MODE == 1
    float scale = 10.0 * ClickScale * length(origin - mouse) / min(iResolution.x, iResolution.y) + 1.0;
    vec2 offset = -origin * (scale - 1.0);

    vec2 bounds = min(vec2(0.0), origin - scale / 2.0) + max(vec2(0.0), origin + scale / 2.0 - iResolution.xy);
    offset -= bounds;
    origin -= bounds;

    float originMask = step(abs(max(abs(fragCoord.x - origin.x + 0.5), abs(fragCoord.y - origin.y + 0.5)) - 0.5 * scale), 1.5);
#endif

#if MODE == 2
    float scale = ClickScale;
    vec2 offset = mouse - origin * scale;
    float originMask = 0.0f;
    offset = clamp(offset, -(iChannelResolution[0].xy + 1.0) * scale + iResolution.xy, vec2(scale));
#endif

#if MODE == 3
    float scale = ClickScale;
    vec2 offset = -(mouse - 0.5) * scale;
    origin = vec2(0.0);
    float originMask = 0.0f;
    offset = clamp(offset, origin - (iChannelResolution[0].xy + 1.0) * scale + iResolution.xy, origin + 1.0 * scale);
#endif

    vec2 position = (fragCoord.xy - offset) / scale;
    if (position.x < 0.0 || position.x > iChannelResolution[0].x || position.y < 0.0 || position.y > iChannelResolution[0].y)
    {
        float grid = mod(floor(fragCoord.x / 15.0) + floor(fragCoord.y / 15.0), 2.0);
        fragColor = vec4(vec3(clamp(0.2 * (1.0 + grid * 0.1), 0.0, 1.0)), 1.0);
        return;
    }

    vec4 value = texelFetch(iChannel0, ivec2(floor(position)), 0);

    float gridMask = 0.0;
    float valueMask = 0.0;

    if (scale > GridVisibleScale)
    {
        vec2 diff = floor(position + vec2(1.0 / scale)) - floor(position);

        gridMask = min(1.0, diff.x + diff.y);
        gridMask *= smoothstep(GridVisibleScale, 10.0 * GridVisibleScale, scale);
    }

    if (scale > ValueVisibleScale)
    {
        vec2 cellPosition = fract(position);
        cellPosition.y = 1.0 - cellPosition.y;

        float valueScale = max(FontScale / scale, MinValueScale);

        if (cellPosition.x < 1.0 - CellPadding.x * valueScale &&
            cellPosition.y < 1.0 - CellPadding.y * valueScale)
        {
            vec2 valuePosition = vec2(cellPosition / valueScale);

            valuePosition -= vec2(CellPadding);
            int valueIndex = clamp(int(floor(valuePosition.y)) / (GlyphHeight + RowSpacing), 0, 3);
            valuePosition.y -= float(valueIndex * (GlyphHeight + RowSpacing));

            valueMask = float(getNumberMask(ivec2(floor(valuePosition)), value[valueIndex]));
            valueMask *= smoothstep(ValueVisibleScale, 10.0 * ValueVisibleScale, scale);
        }
    }

    fragColor = vec4(mixColorMask(clamp(value.rgb, 0.0, 1.0), min(originMask + gridMask + valueMask, 1.0)), 1.0);
}