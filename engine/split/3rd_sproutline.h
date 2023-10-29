/* sproutline - v0.10 - public domain sprite outline detector - http://github.org/ands/sproutline
                        no warranty implied; use at your own risk

   Do this:
      #define S2O_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.

   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define S2O_IMPLEMENTATION
   #include "sproutline.h"

   You can #define S2O_MALLOC to avoid using malloc


   QUICK NOTES:
      Primarily of interest to game developers.
      - Recommended to be used with stb_image.
      - Detects outlines in sprite images with alpha channels.
      - Extracts outlines as clockwise paths.
      - Simplifies outlines based on a distance metric.

   Full documentation under "DOCUMENTATION" below.


   Revision 0.10 release notes:

      - Initial release of sproutline.h.

      - Added S2O_MALLOC macro for replacing the memory allocator.
        Unlike most STB libraries, this macro doesn't support a context parameter,
        so if you need to pass a context in to the allocator, you'll have to
        store it in a global or a thread-local variable.


   Revision history:
      0.10  (2015-10-22) initial version

 ============================    Contributors    =========================

 Andreas Mantler (ands)

License:
   This software is in the public domain. Where that dedication is not
   recognized, you are granted a perpetual, irrevocable license to copy
   and modify this file however you want.

*/

#ifndef S2O_INCLUDE_SPROUTLINE_H
#define S2O_INCLUDE_SPROUTLINE_H

// DOCUMENTATION
//
// Limitations:
//    - currently only works with images that have alpha channels
//
// Basic usage (with stb_image):
//    int w, h, n, l;
//    unsigned char *rgba = stbi_load(filename, &w, &h, &n, 4);
//    unsigned char *alpha = s2o_rgba_to_alpha(rgba, w, h);
//    unsigned char *thresholded = s2o_alpha_to_thresholded(alpha, w, h, ALPHA_THRESHOLD);
//    unsigned char *outlined = s2o_thresholded_to_outlined(thresholded, w, h);
//    s2o_point *outline = s2o_extract_outline_path(outlined, w, h, &l, 0);
//    while(l)
//    {
//        s2o_distance_based_path_simplification(outline, &l, DISTANCE_THRESHOLD);
//        // ... process outline here ...
//        // ... l = number of points in outline
//        // ... ALPHA_THRESHOLD = 1..255 (the min value to be considered solid)
//        // ... DISTANCE_THRESHOLD = 0.0f..Inf (~0.5f is a suitable value)
//        // ... a greater value results in fewer points in the output
//        
//        outline = s2o_extract_outline_path(outlined, w, h, &l, outline);
//    };
//    free(outline);
//    free(outlined);
//    free(thresholded);
//    free(alpha);
//    free(rgba);
//
// s2o_rgba_to_alpha:
//    Expects an 'unsigned char *' to memory of w * h 4-byte pixels in 'RGBA' order.
//    The return value is an 'unsigned char *' to memory of w * h 1-byte pixel alpha components.
//
// s2o_alpha_to_thresholded:
//    Expects an 'unsigned char *' to memory of w * h 1-byte pixel alpha components.
//    The return value is an 'unsigned char *' to memory of w * h 1-byte values
//    that are 255 if the corresponding input is >= the specified threshold, otherwise 0.
//
// s2o_thresholded_to_outlined:
//    Expects an 'unsigned char *' to memory of w * h 1-byte pixels indicating their solidity {0, nonzero}.
//    The return value is an 'unsigned char *' to memory of w * h 1-byte pixels that indicate if the
//    corresponding input value is part of an outline (= is solid and has a non-solid neighbour).
//
// s2o_extract_outline_path:
//    Expects an 'unsigned char *' to memory of w * h 1-byte pixels indicating their outline membership.
//    The return value is an 's2o_point *' to memory of l s2o_point values consisting of a short x and y value.
//    The procedure scans the input data from top to bottom and starts extracting the first outline it finds.
//    The pixels corresponding to the extracted outline are set to 0 in the input, so that a subsequent call to
//    s2o_extract_outline_path extracts a different outline.
//    The length is set to 0 if no outline was found.
//
// s2o_distance_based_path_simplification:
//    Expects an 's2o_point *' to memory of l outline points.
//    The procedure throws out points in place that lie on or close to linear sections of the outline.
//    The distanceThreshold parameter specifies the min distance value for points to remain in the outline.
//
// ===========================================================================
//
// Philosophy
//
// This library is designed with the stb philosophy in mind.
// stb libraries are designed with the following priorities:
//
//    1. easy to use
//    2. easy to maintain
//    3. good performance
//
// Some secondary priorities arise directly from the first two, some of which
// make more explicit reasons why performance can't be emphasized.
//
//    - Portable ("ease of use")
//    - Small footprint ("easy to maintain")
//    - No dependencies ("ease of use")
//

typedef unsigned char s2o_uc;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef S2O_STATIC
#define S2ODEF static
#else
#define S2ODEF extern
#endif

//////////////////////////////////////////////////////////////////////////////
//
// PRIMARY API
//

S2ODEF s2o_uc * s2o_rgba_to_alpha          (const s2o_uc *data, int w, int h);
S2ODEF s2o_uc * s2o_alpha_to_thresholded   (const s2o_uc *data, int w, int h, s2o_uc threshold);
S2ODEF s2o_uc * s2o_thresholded_to_outlined(const s2o_uc *data, int w, int h);

typedef struct { short x, y; } s2o_point;
S2ODEF s2o_point * s2o_extract_outline_path(s2o_uc *data, int w, int h, int *point_count, s2o_point *reusable_outline);
S2ODEF void        s2o_distance_based_path_simplification(s2o_point *outline, int *outline_length, float distance_threshold);

#ifdef __cplusplus
}
#endif

//
//
////   end header file   /////////////////////////////////////////////////////
#endif // S2O_INCLUDE_SPROUTLINE_H

#ifdef S2O_IMPLEMENTATION

#include <math.h> // sqrtf, abs

#ifndef S2O_MALLOC
#include <stdlib.h> // malloc
#define S2O_MALLOC(sz)    malloc(sz)
#endif

///////////////////////////////////////////////
//
//  locally used types

typedef int s2o_bool;

// 2d point type helpers
#define S2O_POINT_ADD(result, a, b) { (result).x = (a).x + (b).x; (result).y = (a).y + (b).y; }
#define S2O_POINT_SUB(result, a, b) { (result).x = (a).x - (b).x; (result).y = (a).y - (b).y; }
#define S2O_POINT_IS_INSIDE(a, w, h) ((a).x >= 0 && (a).y >= 0 && (a).x < (w) && (a).y < (h))
#define S2O_POINT_IS_NEXT_TO(a, b) ((a).x - (b).x <= 1 && (a).x - (b).x >= -1 && (a).y - (b).y <= 1 && (a).y - (b).y >= -1)

// direction type
typedef int s2o_direction; // 8 cw directions: >, _|, v, |_, <, |", ^, "|
#define S2O_DIRECTION_OPPOSITE(dir) ((dir + 4) & 7)
static const s2o_point s2o_direction_to_pixel_offset[] = { {1,0}, {1,-1}, {0,-1}, {-1,-1}, {-1,0}, {-1,1}, {0,1}, {1,1} };

// image manipulation functions
S2ODEF s2o_uc * s2o_rgba_to_alpha(const s2o_uc *data, int w, int h)
{
	s2o_uc *result = (s2o_uc*)S2O_MALLOC(w * h);
	int x, y;
	for (y = 0; y < h; y++)
		for (x = 0; x < w; x++)
			result[y * w + x] = data[(y * w + x) * 4 + 3];
	return result;
}

S2ODEF s2o_uc * s2o_alpha_to_thresholded(const s2o_uc *data, int w, int h, s2o_uc threshold)
{
	s2o_uc *result = (s2o_uc*)S2O_MALLOC(w * h);
	int x, y;
	for (y = 0; y < h; y++)
		for (x = 0; x < w; x++)
			result[y * w + x] = data[y * w + x] >= threshold ? 255 : 0;
	return result;
}

S2ODEF s2o_uc * s2o_dilate_thresholded(const s2o_uc *data, int w, int h)
{
	int x, y, dx, dy, cx, cy;
	s2o_uc *result = (s2o_uc*)S2O_MALLOC(w * h);
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			result[y * w + x] = 0;
			for (dy = -1; dy <= 1; dy++)
			{
				for (dx = -1; dx <= 1; dx++)
				{
					cx = x + dx;
					cy = y + dy;
					if (cx >= 0 && cx < w && cy >= 0 && cy < h)
					{
						if (data[cy * w + cx])
						{
							result[y * w + x] = 255;
							dy = 1;
							break;
						}
					}
				}
			}
		}
	}
	return result;
}

S2ODEF s2o_uc * s2o_thresholded_to_outlined(const s2o_uc *data, int w, int h)
{
	s2o_uc *result = (s2o_uc*)S2O_MALLOC(w * h);
	int x, y;
	for (x = 0; x < w; x++)
	{
		result[x] = data[x];
		result[(h - 1) * w + x] = data[(h - 1) * w + x];
	}
	for (y = 1; y < h - 1; y++)
	{
		result[y * w] = data[y * w];
		for (x = 1; x < w - 1; x++)
		{
			if (data[y * w + x] &&
				(
					!data[y * w + x - 1] ||
					!data[y * w + x + 1] ||
					!data[y * w + x - w] ||
					!data[y * w + x + w]
				))
			{
				result[y * w + x] = 255;
			}
			else
			{
				result[y * w + x] = 0;
			}
		}
		result[y * w + w - 1] = data[y * w + w - 1];
	}
	return result;
}

// outline path procedures
static s2o_bool s2o_find_first_filled_pixel(const s2o_uc *data, int w, int h, s2o_point *first)
{
	int x, y;
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			if (data[y * w + x])
			{
				first->x = (short)x;
				first->y = (short)y;
				return 1;
			}
		}
	}
	return 0;
}

static s2o_bool s2o_find_next_filled_pixel(const s2o_uc *data, int w, int h, s2o_point current, s2o_direction *dir, s2o_point *next)
{
	// turn around 180°, then make a clockwise scan for a filled pixel
	*dir = S2O_DIRECTION_OPPOSITE(*dir);
	int i;
	for (i = 0; i < 8; i++)
	{
		S2O_POINT_ADD(*next, current, s2o_direction_to_pixel_offset[*dir]);

		if (S2O_POINT_IS_INSIDE(*next, w, h) && data[next->y * w + next->x])
			return 1;

		// move to next angle (clockwise)
		*dir = *dir - 1;
		if (*dir < 0)
			*dir = 7;
	}
	return 0;
}

S2ODEF s2o_point * s2o_extract_outline_path(s2o_uc *data, int w, int h, int *point_count, s2o_point *reusable_outline)
{
	s2o_point *outline = reusable_outline;
	if (!outline)
		outline = (s2o_point*)S2O_MALLOC(w * h * sizeof(s2o_point));

	s2o_point current, next;

restart:
	if (!s2o_find_first_filled_pixel(data, w, h, &current))
	{
		*point_count = 0;
		return outline;
	}

	int count = 0;
	s2o_direction dir = 0;

	while(S2O_POINT_IS_INSIDE(current, w, h) && count < (w*h)) //< @r-lyeh: buffer overflow: add count<w*h
	{
		data[current.y * w + current.x] = 0; // clear the visited path
		outline[count++] = current; // add our current point to the outline
		if (!s2o_find_next_filled_pixel(data, w, h, current, &dir, &next))
		{
			// find loop connection
			s2o_bool found = 0;
			int i;
			for (i = 0; i < count / 2; i++) // only allow big loops
			{
				if (S2O_POINT_IS_NEXT_TO(current, outline[i]))
				{
					found = 1;
					break;
				}
			}

			if (found)
			{
				break;
			}
			else
			{
				// go backwards until we see outline pixels again
				dir = S2O_DIRECTION_OPPOSITE(dir);
				count--; // back up
				int prev;
				for(prev = count; prev >= 0 && count < (w * h); prev--) //< @r-lyeh: buffer overflow: add count<w*h 
				{
					current = outline[prev];
					outline[count++] = current; // add our current point to the outline again
					if (s2o_find_next_filled_pixel(data, w, h, current, &dir, &next))
						break;
				}
			}
		}
		current = next;
	}

	if (count <= 2) // too small, discard and try again!
		goto restart;
	*point_count = count;
	return outline;
}

S2ODEF void s2o_distance_based_path_simplification(s2o_point *outline, int *outline_length, float distance_threshold)
{
	int length = *outline_length;
	int l;
	for (l = length / 2 /*length - 1*/; l > 1; l--)
	{
		int a, b = l;
		for (a = 0; a < length; a++)
		{
			s2o_point ab;
			S2O_POINT_SUB(ab, outline[b], outline[a]);
			float lab = sqrtf((float)(ab.x * ab.x + ab.y * ab.y));
			float ilab = 1.0f / lab;
			float abnx = ab.x * ilab, abny = ab.y * ilab;

			if (lab != 0.0f)
			{
				s2o_bool found = 1;
				int i = (a + 1) % length;
				while (i != b)
				{
					s2o_point ai;
					S2O_POINT_SUB(ai, outline[i], outline[a]);
					float t = (abnx * ai.x + abny * ai.y) * ilab;
					float distance = -abny * ai.x + abnx * ai.y;
					if (t < 0.0f || t > 1.0f || distance > distance_threshold || -distance > distance_threshold)
					{
						found = 0;
						break;
					}

					if (++i == length)
						i = 0;
				}

				if (found)
				{
					int i;
					if (a < b)
					{
						for (i = 0; i < length - b; i++)
							outline[a + i + 1] = outline[b + i];
						length -= b - a - 1;
					}
					else
					{
						length = a - b + 1;
						for (i = 0; i < length; i++)
							outline[i] = outline[b + i];
					}
					if (l >= length)
						l = length - 1;
				}
			}

			if (++b >= length)
				b = 0;
		}
	}
	*outline_length = length;
}

#endif // S2O_IMPLEMENTATION
