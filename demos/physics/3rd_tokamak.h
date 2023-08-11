/*************************************************************************
 *                                                                       *
 * Tokamak Physics Engine, Copyright (C) 2002-2007 David Lam.            *
 * All rights reserved.  Email: david@tokamakphysics.com                 *
 *                       Web: www.tokamakphysics.com                     *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *                                                                       *
 *************************************************************************

 Tokamak Physics Engine
 Copyright (c) 2002-2007, David Lam.
 All rights reserved.

 Tokamak Physics Engine can be redistributed using either the BSD3 license or the Zlib license.

 BSD License
 ===========

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 Neither the names of Tokamak Physics Engine's copyright owner nor the names of its 
 contributors may be used to endorse or promote products derived from this software 
 without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ZLib License
 ============

 THIS SOFTWARE IS PROVIDED 'AS-IS', WITHOUT ANY EXPRESS OR IMPLIED WARRANTY.
 IN NO EVENT WILL THE AUTHORS BE HELD LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE.

 Permission is granted to anyone to use this software for any purpose, including commercial 
 applications, and to alter it and redistribute it freely, subject to the following restrictions:

 1. The origin of this software must not be misrepresented; you must not claim that you 
    wrote the original software. If you use this software in a product, an acknowledgment in 
    the product documentation would be appreciated but is not required.

 2. Altered source versions must be plainly marked as such, and must not be misrepresented 
    as being the original software.

 3. This notice may not be removed or altered from any source distribution.

 CHANGELOG
 =========

 2022-03-26  1.0.6   64-bit fixes + OSX fixes + single-header version (@r-lyeh).
 2004-04-10  1.0.5a  added version number define TOKAMAK_VERSION.
                     added Zlib license to Tokamak Physics Engine.
 2004-08-07  1.0.4a  bug fixed: crash when constraint join two rigid bodies to one animated body.
 2004-08-06  1.0.4   bug fixed: crash when constraint join two rigid bodies to one animated body.
 2007-07-20  1.0.3   linux friendly syntax changes.
 2007-07-05  1.0.2   release with source to adjacency.cpp.
 
 INNER WORKINGS
 ==============

 - The dynamic solver hasn't change since back in 2003. It is what you can call sequential impulse. But the naive implementation of this doesn't work very well. Simply calculating impulse for zero-relative velocity alone will allow the joints position to drift. So there is an extra term in the impulse calculation for drift correction. The correction impulse should be such that it produce a relative velocity that reduce the displacement "in a frame step time". All the impulses for one body are accumulated before applying.
 - There is a hack for reducing the number of iteration require for maintaining stable stacking. Tokamak builds a directed graph of contacts pointing away from the gravity vector. And contact constraints are solve in that order.
 - The other things that was important was putting bodies to sleep. That's mostly done by considering the history of the body's state. I don't remember the exact procedure but I am happy to take a look at it again if anyone is interested.
 - For collision detection, everything is pretty standard except for narrow-phase convex meshes. I used the algorithm describe in this paper http://www.cs.hku.hk/research/techreps/ ... 005-01.pdf (Collision Detection of Convex Polyhedra Based on Duality Transformation). The algorithm seems sound, but for some reason I found very few people know or discuss about it. I think there's some numerical issue with my implementation so sometime it can fail.
 - That's off the top of my head. I am happy to talk about anything which I haven't cover. My view on physics engine is pretty much represented what Tokamak does: the important of speed, memory and stability far outweigh any accuracy concerns. I do intend on going back to work on Tokamak, for my own personal use if not for anything else. */


#ifndef TOKAMAK_H
#define TOKAMAK_H

#define TOKAMAK_VERSION_MAJOR 1  
#define TOKAMAK_VERSION_MINOR 0  
#define TOKAMAK_VERSION_BUGFIX 6
#define TOKAMAK_VERSION  (( TOKAMAK_VERSION_MAJOR <<24)+(TOKAMAK_VERSION_MINOR <<16)+(TOKAMAK_VERSION_BUGFIX <<8) + 0)

#ifndef __APPLE__
#include <malloc.h>
#endif

#ifdef TOKAMAK_USE_DLL
#ifdef TOKAMAK_DLL_EXPORTS
#define TOKAMAK_API __declspec(dllexport)
#else
#define TOKAMAK_API __declspec(dllimport)
#endif
#else
#define TOKAMAK_API 
#endif


#ifndef NE_MATH_H
#define NE_MATH_H

#ifdef USE_OPCODE
        
#include "Opcode.h"

#endif //USE_OPCODE

#include <math.h>
#include <float.h>

/**ne_type.h**/

#ifndef NE_TYPE_H
#define NE_TYPE_H

#include <stdarg.h>
//#include <tchar.h>
//#include <strsafe.h>
///////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////

#ifdef NULL
#undef NULL
#endif

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

///////////////////////////////////////////////////////////////////////////

#define FALSE       0                   // make sure that we know what false is
#define TRUE        1                   // Make sure that we know what true is
#define NULL        0                   // Make sure that null does have a type

///////////////////////////////////////////////////////////////////////////
// BASIC TYPES
///////////////////////////////////////////////////////////////////////////

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef signed   char       s8;
typedef signed   short      s16;
typedef signed   int        s32;
typedef float               f32;
typedef double              f64;
typedef u8                  neByte;
typedef s32                 neErr;
typedef s32                 neBool;

#if _MSC_VER
    typedef signed   __int64    s64;
    typedef unsigned __int64    u64;
    #define neFinite _finite
    #define inline   __forceinline       // Make sure that the compiler inlines when we tell him
    #define NEINLINE __forceinline
    const char PATH_SEP = '\\';
#elif defined __GNUC__
    typedef signed long long    s64;
    typedef unsigned long long  u64;
    #define neFinite isfinite
    #define NEINLINE inline
    const char PATH_SEP = '/';
#endif

#endif //NE_TYPE_H


/**ne_debug.h**/

#ifndef NE_DEBUG_H
#define NE_DEBUG_H

#ifdef WIN32
#include <windows.h>
#include <stdio.h>
#endif

#include <assert.h>

#ifdef VERIFY
#undef VERIFY
#endif

#ifdef VERIFYS
#undef VERIFYS
#endif

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef ASSERTS
#undef ASSERTS
#endif

#ifdef BREAK
#undef BREAK
#endif


#if _DEBUG
    #define ASSERT(E)    assert(E)
#else
    #define ASSERT(E)    
#endif

#if _DEBUG

    #ifdef WIN32
        
        #define TOKAMAK_OUTPUT(str) {OutputDebugString(str);}

        #define TOKAMAK_OUTPUT_1(frmt,d) \
        {   char tmpBuffer[256];\
            sprintf(tmpBuffer, frmt, d);\
            OutputDebugString(tmpBuffer);\
        }
        #define TOKAMAK_OUTPUT_2(frmt,d1,d2) \
        {   char tmpBuffer[256];\
            sprintf(tmpBuffer, frmt, d1, d2);\
            OutputDebugString(tmpBuffer);\
        }
        #define TOKAMAK_OUTPUT_3(frmt,d1,d2,d3) \
        {   char tmpBuffer[256];\
            sprintf(tmpBuffer, frmt, d1, d2,d3);\
            OutputDebugString(tmpBuffer);\
        }
        #define TOKAMAK_OUTPUT_4(frmt,d1,d2,d3,d4) \
        {   char tmpBuffer[256];\
            sprintf(tmpBuffer, frmt, d1,d2,d3,d4);\
            OutputDebugString(tmpBuffer);\
        }

    #else

        #define TOKAMAK_OUTPUT(str)
        #define TOKAMAK_OUTPUT_1(frmt,d1)
        #define TOKAMAK_OUTPUT_2(frmt,d1,d2)
        #define TOKAMAK_OUTPUT_3(frmt,d1,d2,d3)
        #define TOKAMAK_OUTPUT_4(frmt,d1,d2,d3,d4)

    #endif

#else

        #define TOKAMAK_OUTPUT(str)
        #define TOKAMAK_OUTPUT_1(frmt,d1)
        #define TOKAMAK_OUTPUT_2(frmt,d1,d2)
        #define TOKAMAK_OUTPUT_3(frmt,d1,d2,d3)
        #define TOKAMAK_OUTPUT_4(frmt,d1,d2,d3,d4)

#endif

#endif//NE_DEBUG_H


/**ne_smath.h**/

#ifndef NE_SMATH_H
#define NE_SMATH_H

#include <math.h>
//#include "ne_type.h"

#define NE_PI   (3.141592653589793238462643f)

#define NE_RAD_TO_DEG(A) ((f32)(((A) * (180.0f / NE_PI))))
#define NE_DEG_TO_RAD(A) ((f32)(((A) * (NE_PI / 180.0f))))
#define NE_RI             NE_DEG_TO_RAD(1)  
#define NE_ZERO (1.0e-6f)

typedef f32 neRadian;

///////////////////////////////////////////////////////////////////////////
//
// GENERAL
//
///////////////////////////////////////////////////////////////////////////

f32         neFRand      ( f32 Min, f32 Max );
f32         neSin        ( neRadian S );
neRadian    neASin       ( f32 S );
f32         neCos        ( neRadian C );
neRadian    neACos       ( f32 C );
f32         neTan        ( neRadian T );
neRadian    neATan       ( f32 T );
neRadian    neATan2      ( f32 y, f32 x );
neBool      neRealsEqual    ( f32 s1, f32 s2);
neBool      neIsConsiderZero(f32 f);
neBool      neIsFinite  (f32);

//template< class ta >                     NEINLINE ta      neAbs     ( const ta&  A )                               { return ( A < 0 ) ? -A : A;   }

NEINLINE f32 neAbs(f32 f)
{
    return (f32)fabs(f);
}

template< class ta, class tb, class tc > NEINLINE neBool  neInRange ( const ta&  X, const tb& Min, const tc& Max ) { return (Min <= X) && (X <= Max);}
template< class ta, class tb, class tc > NEINLINE ta      neRange   ( const ta&  X, const tb& Min, const tc& Max ) { if( X < Min ) return Min; return(X > Max) ? Max : X; }
template< class ta>                      NEINLINE void    neSwap    ( ta &  X, ta & Y) { ta tmp = X; X = Y; Y = tmp; }
template< class ta >                     NEINLINE ta      neSqr     ( const ta&  A )                               { return A * A; }
template< class ta >                     NEINLINE ta      neMin     ( const ta&  A, const ta& B )                  { return ( A < B ) ?  A : B;   }
template< class ta >                     NEINLINE ta      neMax     ( const ta&  A, const ta& B )                  { return ( A > B ) ?  A : B;   }
NEINLINE f32     neMin     ( const s32& A, const f32& B )                 { return ( A < B ) ?  A : B;   }
NEINLINE f32     neMax     ( const s32& A, const f32& B )                 { return ( A > B ) ?  A : B;   }
NEINLINE f32     neMin     ( const f32& A, const s32& B )                 { return ( A < B ) ?  A : B;   }
NEINLINE f32     neMax     ( const f32& A, const s32& B )                 { return ( A > B ) ?  A : B;   }
NEINLINE neBool    neIsFinite   (f32 n) {return neFinite((double)n);} 

#endif


//#include <xmmintrin.h>
/****************************************************************************
*
*   neV3
*
****************************************************************************/ 

static s32 neNextDim1[] = {1, 2, 0};
static s32 neNextDim2[] = {2, 0, 1};

typedef struct neQ neQ;

typedef struct neM3 neM3;

//struct __declspec(align(16)) neV3
struct neV3
{
public:

    f32 v[4];
/*
    union
    {
        f32 v[4];
        
        struct nTag
        {
            f32 X,Y,Z,W;
        }n;

        //__m128 m;
    };
*/
    NEINLINE neV3 & SetZero (void );
    NEINLINE neV3 & SetOne(void);
    NEINLINE neV3 & SetHalf(void);
    NEINLINE neV3 & Set(f32 value);
    NEINLINE neV3 & Set  (f32 x, f32 y, f32 z );
    NEINLINE neV3 & Set  (const neV3& V);
    NEINLINE neV3 & Set  (const neQ& Q);
    NEINLINE void   Set  (f32 val[3]);
    NEINLINE void   Get  (f32 val[3]);

    NEINLINE f32&   operator   []   (s32 I);
    NEINLINE f32    operator   []   (s32 I) const;

    NEINLINE f32 X() const;
    NEINLINE f32 Y() const;
    NEINLINE f32 Z() const;
    NEINLINE f32 W() const;

    NEINLINE void   Normalize   (void);
    NEINLINE f32    Length      (void) const;
    NEINLINE f32    Dot         (const neV3& V) const;
    NEINLINE neV3   Cross       (const neV3& V) const;

    NEINLINE void   RotateX      (neRadian Rx);
    NEINLINE void   RotateY      (neRadian Ry);
    NEINLINE void   RotateZ      (neRadian Rz);
    NEINLINE neRadian GetPitch      (void) const;
    NEINLINE neRadian GetYaw        (void) const;

    NEINLINE void   SetBoxTensor    (f32 width, f32 height, f32 depth, f32 mass);
    NEINLINE void   SetAbs          (const neV3 & v);
    NEINLINE f32    GetDistanceFromLine(const neV3 & point1, const neV3 & point2);
    NEINLINE f32    GetDistanceFromLine2(neV3 & project, const neV3 & pointA, const neV3 & pointB);
    NEINLINE f32    GetDistanceFromLineAndProject(neV3 & result, const neV3 & startPoint, const neV3 & dir);
    NEINLINE neBool GetIntersectPlane(neV3 & normal, neV3 & pointOnPlane, neV3 & point1, neV3 & point2);

    NEINLINE void   SetMin          (const neV3& V1, const neV3& V2);
    NEINLINE void   SetMax          (const neV3& V1, const neV3& V2);

    NEINLINE void   RemoveComponent (const neV3& V);

    NEINLINE bool   IsConsiderZero  () const; 
    NEINLINE bool   IsFinite        () const;
    NEINLINE neV3   Project         (const neV3 & v) const;

//  NEINLINE neV3 & operator = (const neV3& V);
    NEINLINE neV3& operator /= (f32 S);
    NEINLINE neV3& operator *= (f32 S);
    NEINLINE neV3& operator += (const neV3& V);
    NEINLINE neV3& operator -= (const neV3& V);

    NEINLINE neV3 friend operator + (const neV3& V1, const neV3& V2);
    NEINLINE neV3 friend operator - (const neV3& V1, const neV3& V2);
    NEINLINE neV3 friend operator / (const neV3& V,  f32 S);
    NEINLINE neV3 friend operator * (const neV3& V,  f32 S);
    NEINLINE neV3 friend operator * (const neV3& V1, const neV3& V2);
    NEINLINE neV3 friend operator * (const neV3& V,  const neM3& M);
    NEINLINE neM3 friend operator ^ (const neV3 & V, const neM3 & M); //cross product operator
    NEINLINE friend neV3 operator - (const neV3& V );
    NEINLINE friend neV3 operator * (f32 S,  const neV3& V  );

#ifdef USE_OPCODE

    NEINLINE neV3 & operator = (const IceMaths::Point & pt);
    NEINLINE IceMaths::Point& AssignIcePoint(IceMaths::Point & pt) const;

#endif //USE_OPCODE
};

/****************************************************************************
*
*   neV4
*
****************************************************************************/ 

struct neV4
{
    f32 X, Y, Z, W;

    // functions
    NEINLINE neV4 ( void );
    NEINLINE neV4 ( f32 x, f32 y, f32 z, f32 w );
    NEINLINE neV4 ( const neV3& V, f32 w );
    NEINLINE neV4 ( const neV4& V );
    NEINLINE void SetZero ( void );
    NEINLINE void Set ( f32 x, f32 y, f32 z, f32 w );

    NEINLINE f32&    operator   []   ( s32 I );
    NEINLINE neV4&   operator   /=   ( f32 S );
    NEINLINE neV4&   operator   *=   ( f32 S );
    NEINLINE neV4&   operator   +=   ( const neV4& V );
    NEINLINE neV4&   operator   -=   ( const neV4& V );
    NEINLINE neV4&   Normalize       ( void );
    NEINLINE f32     Length          ( void ) const;
    NEINLINE f32     Dot             ( const neV4& V ) const;

    NEINLINE friend neV4 operator -  ( const neV4& V );
    NEINLINE friend neV4 operator *  ( f32             S,  const neV4& V  );
    NEINLINE friend neV4 operator /  ( const neV4& V,  f32             S  );
    NEINLINE friend neV4 operator *  ( const neV4& V,  f32             S  );
    NEINLINE friend neV4 operator +  ( const neV4& V1, const neV4& V2 );
    NEINLINE friend neV4 operator -  ( const neV4& V1, const neV4& V2 );
};

/****************************************************************************
*
*   neM3
*
****************************************************************************/ 

struct neM3
{
    neV3 M[3];

    NEINLINE neV3&  operator   [] ( s32 I );
    NEINLINE neV3   operator   [] ( s32 I ) const;

    NEINLINE void SetZero ( void );
    NEINLINE void SetIdentity ( void );
    NEINLINE neBool SetInvert(const neM3 & rhs);
    NEINLINE neM3 & SetTranspose ( neM3 & M );
    NEINLINE void GetColumns( neV3& V1, neV3& V2, neV3& V3 ) const;
    NEINLINE void SetColumns( const neV3& V1, const neV3& V2, const neV3& V3 );
    NEINLINE neV3 GetDiagonal();
    NEINLINE neV3 TransposeMulV3(const neV3 & V);
    NEINLINE void RotateXYZ(const neV3 & rotate);
    NEINLINE neM3 & SkewSymmetric(const neV3 & V);

    NEINLINE neBool IsIdentity() const;
    NEINLINE neBool IsOrthogonalNormal() const;

    NEINLINE neBool IsFinite() const;
    
    NEINLINE neM3 &operator += ( const neM3& add);
    NEINLINE neM3 operator ^ (const neV3 & vec) const; //cross product
    NEINLINE neM3 operator * (f32 scalar) const;

    NEINLINE friend neM3 operator +     ( const neM3& add1, const neM3& add2);
    NEINLINE friend neM3 operator -     ( const neM3& sub1, const neM3& sub2);
    NEINLINE friend neM3 operator *     ( const neM3& M1, const neM3& M2 );
    NEINLINE friend neV3 operator *     ( const neM3& M1, const neV3& V  );
    NEINLINE friend neM3 operator *     ( f32 scalar, const neM3& M );
    NEINLINE friend neM3& operator *=   ( neM3& M1, const f32 f  );
    NEINLINE friend neM3& operator /=   ( neM3& M1, const f32 f  );
};

/****************************************************************************
*
*   neM4
*
****************************************************************************/ 

struct neM4
{
    f32 M[4][4];

    // functions
    NEINLINE void Set(float row00, float row01, float row02, float row03
                        , float row10, float row11, float row12, float row13
                        , float row20, float row21, float row22, float row23
                        , float row30, float row31, float row32, float row33);

    NEINLINE void Set(int row, int col, f32 val){M[col][row] = val;}
    NEINLINE void SetZero        ( void );
    NEINLINE void SetIdentity    ( void );
                                            
    NEINLINE neV3  GetScale       ( void ) const;
    NEINLINE neV3  GetTranslation ( void ) const;

    NEINLINE void  SetTranslation ( const neV3& V ); 
    NEINLINE void  SetScale       ( const neV3& V ); 

    NEINLINE f32&  operator []    ( s32 I );
    NEINLINE neM4& operator *=    ( const neM4& M );
    NEINLINE neM4& operator =     ( const neM3& M );
    NEINLINE neV3  TransformAs3x3 ( const neV3& V ) const;

    NEINLINE void  GetRows        ( neV3& V1, neV3& V2, neV3& V3 ) const;
    NEINLINE void  SetRows        ( const neV3& V1, const neV3& V2, const neV3& V3 );
    NEINLINE void  GetColumns     ( neV3& V1, neV3& V2, neV3& V3 ) const;
    NEINLINE void  SetColumns     ( const neV3& V1, const neV3& V2, const neV3& V3 );
    NEINLINE void  GetColumn      ( neV3& V1, u32 col) const;

    NEINLINE void SetTranspose(const neM4 & M);
    NEINLINE void SetFastInvert  ( const neM4& Src );
    
    NEINLINE friend neM4 operator *             ( const neM4& M1, const neM4& M2 );
    NEINLINE friend neV3 operator *             ( const neM4& M1, const neV3& V  );
};

/****************************************************************************
*
*   neQ
*
****************************************************************************/ 

struct neQ
{
    f32 X, Y, Z, W;

    // functions
    NEINLINE neQ         ( void );
    NEINLINE neQ         ( f32 X, f32 Y, f32 Z, f32 W );
    NEINLINE neQ         ( const neM4& M );

    NEINLINE void       Zero                ( void );
    NEINLINE void       Identity            ( void );
    NEINLINE void       SetupFromMatrix     ( const neM4& Matrix );
    NEINLINE void       SetupFromMatrix3    ( const neM3& Matrix );
    NEINLINE void       GetAxisAngle        ( neV3& Axis, neRadian& Angle ) const;

    NEINLINE neM4       BuildMatrix         ( void ) const;
    NEINLINE neM3       BuildMatrix3         ( void ) const;
    NEINLINE neQ&       Normalize           ( void );
    NEINLINE f32        Dot                 ( const neQ& Q ) const;
    NEINLINE neQ&       Invert              ( void );
    NEINLINE neBool     IsFinite            ();

    NEINLINE neQ&       operator *=         ( const neQ& Q );
    NEINLINE neQ&       operator *=         ( f32 S );
    NEINLINE neQ&       operator +=         ( const neQ& Q );
    NEINLINE neQ&       operator -=         ( const neQ& Q );

    NEINLINE neQ&       Set                 (f32 X, f32 Y, f32 Z, f32 W);
    NEINLINE neQ&       Set                 (const neV3 & V, f32 W);
    NEINLINE neQ&       Set                 (f32 angle, const neV3 & axis);

    NEINLINE friend neQ  operator -          ( const neQ& V );
    NEINLINE friend neQ  operator *          ( const neQ& Qa, const neQ& Qb );
    NEINLINE friend neV3 operator *          ( const neQ& Q,  const neV3& V );
    NEINLINE friend neQ  operator *          ( const neQ& Q, f32 S );
    NEINLINE friend neQ  operator *          ( f32 S, const neQ& Q );
    NEINLINE friend neQ  operator +          ( const neQ& Qa, const neQ& Qb );
    NEINLINE friend neQ  operator -          ( const neQ& Qa, const neQ& Qb );
};

/****************************************************************************
*
*   neT3
*
****************************************************************************/ 

struct neT3
{
public:
    neM3 rot;
    neV3 pos;

    NEINLINE neT3 FastInverse();
    NEINLINE neT3 operator * (const neT3 & t);
    NEINLINE neV3 operator * (const neV3 & v);
    NEINLINE neBool IsFinite();

public:
    NEINLINE void MakeD3DCompatibleMatrix()
    {
        rot[0].v[3] = 0.0f;
        rot[1].v[3] = 0.0f;
        rot[2].v[3] = 0.0f;
        pos.v[3] = 1.0f;
    }

    NEINLINE void SetIdentity()
    {
        rot.SetIdentity();
        pos.SetZero();
/*
 *      additional code to make this binary compatible with rendering matrix
 */
        MakeD3DCompatibleMatrix();
    }

#ifdef USE_OPCODE
    
    NEINLINE neT3 & operator = (const IceMaths::Matrix4x4 & mat);
    NEINLINE IceMaths::Matrix4x4 & AssignIceMatrix(IceMaths::Matrix4x4 & mat) const;

#endif //USE_OPCODE
};

///////////////////////////////////////////////////////////////////////////
// INCLUDE INLINE HEADERS
///////////////////////////////////////////////////////////////////////////

/**ne_math_misc_inline.h**/

#include <stdlib.h>

//=========================================================================

NEINLINE f32 neFRand( f32 Min, f32 Max )
{
    ASSERT( Max >= Min );
    return( (((f32)rand() / (f32)RAND_MAX) * (Max-Min)) + Min );
}

//=========================================================================

NEINLINE f32 neSin( neRadian S )
{
    return (f32)sin( S );
}

//=========================================================================

NEINLINE f32 neCos( neRadian C )
{
    return (f32)cos( C );
}

//=========================================================================

NEINLINE neRadian neASin( f32 S )
{
    return (f32)asin( S );
}

//=========================================================================

NEINLINE neRadian neACos( f32 C )
{
    return (f32)acos( C );
}

//=========================================================================

NEINLINE f32 neTan( neRadian T )
{
    return (f32)tan( T );
}

//=========================================================================

NEINLINE neRadian neATan( f32 T )
{
    return (f32)atan( T );
}

//=========================================================================

NEINLINE neRadian neATan2( f32 y, f32 x )
{
    return (f32)atan2( y, x );
}

//=========================================================================

NEINLINE neBool neRealsEqual(f32 s1, f32 s2)
{
    if ( (2.0f * neAbs( s1 - s2 ) / ( s1 + s2 )) < NE_ZERO )
    {
        return true;
    }

    return false;
}

//=========================================================================

NEINLINE neBool neIsConsiderZero(f32 f)
{
    return (neAbs(f) < NE_ZERO);
}

/**ne_math_v3_inline.h**/

//=========================================================================

NEINLINE f32& neV3::operator[]( s32 I )
{
    ASSERT( I >= 0 );
    ASSERT( I <= 2 );
   // return ((f32*)this)[I];
    return v[I];
}

//=========================================================================

NEINLINE f32 neV3::operator [] ( s32 I ) const
{
    ASSERT( I >= 0 );
    ASSERT( I <= 2 );
    return v[I];
}

//=========================================================================

NEINLINE neV3 & neV3::Set( f32 x, f32 y, f32 z )
{
    this->v[0] = x; this->v[1] = y; this->v[2] = z;
    //m = _mm_set_ps(x, y, z, 0);
    return (*this);
}
/*
NEINLINE neV3 & neV3::operator =(const neV3 & V)
{ 
    _mm_store_ps(&v[0], V.m); 
    return (*this);
};
*/
NEINLINE neV3 & neV3::Set (const neV3& V)
{
    (*this) = V;
    return (*this);
}

NEINLINE neV3 & neV3::Set(const neQ& Q)
{
    v[0] = Q.X;
    v[1] = Q.Y;
    v[2] = Q.Z;

    return (*this);
}

NEINLINE void neV3::SetAbs(const neV3 & a)
{
    v[0] = neAbs(a[0]);
    v[1] = neAbs(a[1]);
    v[2] = neAbs(a[2]);
}

//=========================================================================

NEINLINE void neV3::Set( f32 val[3])
{
    this->v[0] = val[0]; this->v[1] = val[1]; this->v[2] = val[2];
}

//=========================================================================

NEINLINE void neV3::Get( f32 val[3])
{
    val[0] = X(); val[1] = Y(); val[2] = Z();
}

//=========================================================================

NEINLINE f32 neV3::Length( void ) const
{
    f32 dot = this->Dot( *this );

    if (neIsConsiderZero(dot))
        return 0.0f;

    return (f32)sqrtf( dot );
}

//=========================================================================

NEINLINE void neV3::Normalize( void )
{
    f32 l = Length();

    if (l == 0.0f)
    {
        SetZero();
    }
    else
    {
        *this *= 1.0f / Length();
    }
}

//=========================================================================

NEINLINE void neV3::RotateX( neRadian Rx )
{
    f32 s = (f32)sinf( Rx );
    f32 c = (f32)cosf( Rx );
    f32 y = v[1];
    f32 z = v[2];

    v[1]  = (c * y) - (s * z);
    v[2]  = (c * z) + (s * y);
}

//=========================================================================

NEINLINE void neV3::RotateY( neRadian Ry )
{
    f32 s = (f32)sinf( Ry );
    f32 c = (f32)cosf( Ry );
    f32 x = X();
    f32 z = Z();

    this->v[0] = (c * x) + (s * z);
    this->v[2] = (c * z) - (s * x);
}

//=========================================================================

NEINLINE void neV3::RotateZ( neRadian Rz )
{
    f32 s = (f32)sinf( Rz );
    f32 c = (f32)cosf( Rz );
    f32 x = X();
    f32 y = Y();

    this->v[0]  = (c * x) - (s * y);
    this->v[1]  = (c * y) + (s * x);
}

//=========================================================================

NEINLINE neV3 & neV3::SetZero( void )
{
    this->v[0] = this->v[1] = this->v[2] = 0.0f;

    return (*this);
}

NEINLINE neV3 & neV3::SetOne(void)
{
    this->v[0] = this->v[1] = this->v[2] = 1.0f;
    return (*this);
}

NEINLINE neV3 & neV3::SetHalf(void)
{
    this->v[0] = this->v[1] = this->v[2] = 0.5f;
    return (*this);
}

NEINLINE neV3 & neV3::Set(f32 value)
{
    this->v[0] = this->v[1] = this->v[2] = value;
    return (*this);
}

//=========================================================================

NEINLINE f32 neV3::Dot( const neV3& V ) const
{
    return  X() * V.X() + Y() * V.Y() + Z() * V.Z();
}

//=========================================================================

NEINLINE neV3 neV3::Cross( const neV3& V ) const
{
    neV3 tmp;

    tmp.v[0] = Y() * V.Z() - Z() * V.Y();
    tmp.v[1] = Z() * V.X() - X() * V.Z();
    tmp.v[2] = X() * V.Y() - Y() * V.X();

    return tmp;
}

//=========================================================================

NEINLINE neV3& neV3::operator += ( const neV3& V )
{
    v[0] += V.X(); v[1] += V.Y(); v[2] += V.Z();
    return *this;
}

//=========================================================================

NEINLINE neV3& neV3::operator -= ( const neV3& V )
{
    v[0] -= V.X(); 
    v[1] -= V.Y(); 
    v[2] -= V.Z();
    return *this;
}

//=========================================================================

NEINLINE neV3& neV3::operator /= ( f32 S )
{
    *this = *this / S;
    return *this;
}

//=========================================================================

NEINLINE neV3& neV3::operator *= ( f32 S )
{
    *this = *this * S;
    return *this;
}

//=========================================================================

NEINLINE neRadian neV3::GetPitch( void ) const
{
    return(f32) -atan2( Y(), (f32)sqrt( X()*X() + Z()*Z() ) );
}

//=========================================================================

NEINLINE neRadian neV3::GetYaw( void ) const
{
    return (f32)atan2( X(), Z() );
}

///////////////////////////////////////////////////////////////////////////
// VECTOR3 FRIEND FUNCTIONS
///////////////////////////////////////////////////////////////////////////

//=========================================================================

NEINLINE neV3 operator - ( const neV3& V )
{
    neV3 tmp;
    return tmp.Set( -V.X(), -V.Y(), -V.Z() );
}

//=========================================================================

NEINLINE neV3 operator + ( const neV3& V1, const neV3& V2 )
{
    neV3 tmp;

    //tmp.m = _mm_add_ps(V1.m, V2.m);

    //return tmp;
    
    return tmp.Set( V1.X() + V2.X(), V1.Y() + V2.Y(), V1.Z() + V2.Z() );
}

//=========================================================================

NEINLINE neV3 operator - ( const neV3& V1, const neV3& V2 )
{
    neV3 tmp;

    //tmp.m = _mm_sub_ps(V1.m, V2.m);

    //return tmp;
    return tmp.Set( V1.X() - V2.X(), V1.Y() - V2.Y(), V1.Z() - V2.Z() );
}

//=========================================================================

NEINLINE neV3 operator / ( const neV3& V, f32 S )
{
    return V * (1.0f/S);
}

//=========================================================================

NEINLINE neV3 operator * ( const neV3& V, const f32 S )
{
    neV3 tmp;
    return tmp.Set( V.X() * S, V.Y() * S, V.Z() * S );
}

//=========================================================================

NEINLINE neV3 operator * ( f32 S,  const neV3& V )
{
    return V * S;
}

//=========================================================================
NEINLINE void neV3::SetBoxTensor(f32 width, f32 height, f32 length, f32 mass)
{
    v[0] = mass * (length * length + height * height) / 12.0f;
    v[1] = mass * (width * width + height * height) / 12.0f;
    v[2] = mass * (length * length + width * width) / 12.0f;
}

//=========================================================================
NEINLINE void neV3::SetMin(const neV3& V1, const neV3& V2)
{
    (*this)[0] = (V1.X() < V2.X()) ? V1.X() : V2.X();
    (*this)[1] = (V1.Y() < V2.Y()) ? V1.Y() : V2.Y();
    (*this)[2] = (V1.Z() < V2.Z()) ? V1.Z() : V2.Z();
}
//=========================================================================
NEINLINE void neV3::SetMax(const neV3& V1, const neV3& V2)
{
    (*this)[0] = (V1.X() > V2.X()) ? V1.X() : V2.X();
    (*this)[1] = (V1.Y() > V2.Y()) ? V1.Y() : V2.Y();
    (*this)[2] = (V1.Z() > V2.Z()) ? V1.Z() : V2.Z();
}
//=========================================================================
NEINLINE neV3 operator *      ( const neV3& V,  const neM3&     M  )
{
    neV3 ret;

    ret[0] = V.Dot(M[0]);
    ret[1] = V.Dot(M[1]);
    ret[2] = V.Dot(M[2]);
    return ret;
}
//=========================================================================
NEINLINE neV3 operator * ( const neV3& V1, const neV3& V2 )
{
    neV3 ret;

    //ret.m = _mm_mul_ps(V1.m, V2.m);

    //return ret;

    ret[0] = V1[0] * V2[0];
    ret[1] = V1[1] * V2[1];
    ret[2] = V1[2] * V2[2];

    return ret;
}

NEINLINE bool neV3::IsConsiderZero() const
{
    return (neIsConsiderZero(v[0]) &&
            neIsConsiderZero(v[1]) &&
            neIsConsiderZero(v[2]));
}

NEINLINE bool neV3::IsFinite() const
{
    if (neFinite((double)v[0]) &&
        neFinite((double)v[1]) &&
        neFinite((double)v[2]))
        return true;
    return false;
}

NEINLINE f32 neV3::GetDistanceFromLine(const neV3 & pointA, const neV3 & pointB)
{
    neV3 ba = pointB - pointA;

    f32 len = ba.Length();

    if (neIsConsiderZero(len))
        ba.SetZero();
    else
        ba *= 1.0f / len;

    neV3 pa = (*this) - pointA;

    f32 k = pa.Dot(ba);

    neV3 q = pointA + k * ba;

    neV3 diff = (*this) - q;

    return diff.Length();
}

NEINLINE f32 neV3::GetDistanceFromLine2(neV3 & project, const neV3 & pointA, const neV3 & pointB)
{
    neV3 ba = pointB - pointA;

    f32 len = ba.Length();

    if (neIsConsiderZero(len))
        ba.SetZero();
    else
        ba *= 1.0f / len;

    neV3 pa = (*this) - pointA;

    f32 k = pa.Dot(ba);

    project = pointA + k * ba;

    neV3 diff = (*this) - project;

    return diff.Length();
}

NEINLINE f32 neV3::GetDistanceFromLineAndProject(neV3 & result, const neV3 & startPoint, const neV3 & dir)
{
    neV3 pa = (*this) - startPoint;

    f32 k = pa.Dot(dir);
    
    result = startPoint + k * dir;

    neV3 diff = (*this) - result;

    return diff.Length();
}

NEINLINE void neV3::RemoveComponent (const neV3& V)
{
    f32 dot = (*this).Dot(V);

    (*this) = (*this) - V * dot;
}

NEINLINE neV3 neV3::Project(const neV3 & v) const
{
    f32 dot = (*this).Dot(v);

    return (v * dot);
}

NEINLINE neBool neV3::GetIntersectPlane(neV3 & normal, neV3 & pointOnPlane, neV3 & point1, neV3 & point2)
{
    neV3 diff = point2 - point1;

    f32 d2 = normal.Dot(diff);

    if (neIsConsiderZero(d2))
        return false;

    f32 d1 = normal.Dot(pointOnPlane - point1);

    f32 u = d1 / d2;

    *this = point1 + diff * u;

    return true;
}

NEINLINE f32 neV3::X() const
{
    return v[0];
}

NEINLINE f32 neV3::Y() const
{
    return v[1];
}

NEINLINE f32 neV3::Z() const
{
    return v[2];
}

NEINLINE f32 neV3::W() const
{
    return v[3];
}

///////////////////////////////////////////////////////////////////////////
// END
///////////////////////////////////////////////////////////////////////////

/**ne_math_v4_inline.h**/

///////////////////////////////////////////////////////////////////////////
// VECTOR4 MEMBER FUNCTIONS
///////////////////////////////////////////////////////////////////////////

//=========================================================================

NEINLINE neV4::neV4( void ){}

//=========================================================================

NEINLINE neV4::neV4( f32 x, f32 y, f32 z, f32 w )
{
    X = x; Y = y; Z = z; W = w;
}

//=========================================================================

NEINLINE neV4::neV4( const neV4& V )
{
    (*this) = V;
}

//=========================================================================

NEINLINE neV4::neV4( const neV3& V, f32 w )
{
    X = V.X(); Y = V.Y(); Z = V.Z(); W = w;    
}

//=========================================================================

NEINLINE f32 neV4::Length( void ) const
{
    return (f32)sqrt( this->Dot( *this ) );
}

//=========================================================================

NEINLINE neV4& neV4::Normalize( void )
{
    *this *= 1 / Length();
    return *this;
}

//=========================================================================

NEINLINE void neV4::SetZero( void )
{
    X = Y = Z = W = 0;
}

//=========================================================================

NEINLINE void neV4::Set( f32 x, f32 y, f32 z, f32 w )
{
    X = x; Y = y; Z = z; W = w;
}

//=========================================================================

NEINLINE f32 neV4::Dot( const neV4& V ) const
{
    return  X * V.X + Y * V.Y + Z * V.Z + W * V.W;
}

//=========================================================================

NEINLINE f32& neV4::operator[]( s32 I )
{
    ASSERT( I >= 0 );
    ASSERT( I <= 3 );
    return ((f32*)this)[I];
}

//=========================================================================

NEINLINE neV4& neV4::operator += ( const neV4& V )
{
    X += V.X; Y += V.Y; Z += V.Z; W += V.W;
    return *this;
}

//=========================================================================

NEINLINE neV4& neV4::operator -= ( const neV4& V )
{
    X -= V.X; Y -= V.Y; Z -= V.Z;  W -= V.W;
    return *this;
}

//=========================================================================

NEINLINE neV4& neV4::operator /= ( f32 S )
{
    *this = *this / S;
    return *this;
}

//=========================================================================

NEINLINE neV4& neV4::operator *= ( f32 S )
{
    *this = *this * S;
    return *this;
}


///////////////////////////////////////////////////////////////////////////
// VECTOR4 FRIEND FUNCTIONS
///////////////////////////////////////////////////////////////////////////

//=========================================================================

NEINLINE neV4 operator - ( const neV4& V )
{
    return neV4( -V.X, -V.Y, -V.Z, -V.W );
}

//=========================================================================

NEINLINE neV4 operator + ( const neV4& V1, const neV4& V2 )
{
    return neV4( V1.X + V2.X, V1.Y + V2.Y, V1.Z + V2.Z, V1.W + V2.W );
}

//=========================================================================

NEINLINE neV4 operator - ( const neV4& V1, const neV4& V2 )
{
    return neV4( V1.X - V2.X, V1.Y - V2.Y, V1.Z - V2.Z, V1.W - V2.W );
}

//=========================================================================

NEINLINE neV4 operator / ( const neV4& V, f32 S )
{
    return V * (1/S);
}

//=========================================================================

NEINLINE neV4 operator * ( const neV4& V, const f32 S )
{
    return neV4( V.X * S, V.Y * S, V.Z * S, V.W * S );
}

//=========================================================================

NEINLINE neV4 operator * ( f32 S,  const neV4& V )
{
    return V * S;
}

///////////////////////////////////////////////////////////////////////////
// END
///////////////////////////////////////////////////////////////////////////

/**ne_math_m4_inline.h**/

/****************************************************************************
*
*   neM4
*
****************************************************************************/ 

NEINLINE void   neM4::Set(float row00, float row01, float row02, float row03
                    , float row10, float row11, float row12, float row13
                    , float row20, float row21, float row22, float row23
                    , float row30, float row31, float row32, float row33)
{
    M[0][0] = row00; M[1][0] = row01; M[2][0] = row02; M[3][0] = row03;
    M[0][1] = row10; M[1][1] = row11; M[2][1] = row12; M[3][1] = row13;
    M[0][2] = row20; M[1][2] = row21; M[2][2] = row22; M[3][2] = row23;
    M[0][3] = row30; M[1][3] = row31; M[2][3] = row32; M[3][3] = row33;
}

//=========================================================================

NEINLINE void neM4::SetZero( void )
{
    M[0][0] = M[1][0] = M[2][0] = M[3][0] = 0;
    M[0][1] = M[1][2] = M[2][1] = M[3][1] = 0;
    M[0][2] = M[1][3] = M[2][3] = M[3][2] = 0;
    M[0][3] = M[1][1] = M[2][2] = M[3][3] = 0;
}

//=========================================================================

NEINLINE f32& neM4::operator[]( s32 I )
{
    ASSERT( I >= 0  );
    ASSERT( I < 4*4 );
    return ((f32*)this)[I];
}

//=========================================================================

NEINLINE void neM4::SetIdentity( void )
{
    M[0][1] = M[1][0] = M[2][0] = M[3][0] = 0;
    M[0][2] = M[1][2] = M[2][1] = M[3][1] = 0;
    M[0][3] = M[1][3] = M[2][3] = M[3][2] = 0;
    M[0][0] = M[1][1] = M[2][2] = M[3][3] = 1;
}

//=========================================================================

NEINLINE neV3 neM4::GetScale( void ) const
{
    neV3 tmp;
    return tmp.Set( M[0][0], M[1][1], M[2][2] );
}

//=========================================================================

NEINLINE void neM4::SetTranslation( const neV3& V )
{
    M[3][0] = V.X(); M[3][1] = V.Y(); M[3][2] = V.Z();
}

//=========================================================================

NEINLINE void neM4::SetScale( const neV3& V )
{
    M[0][0] = V.X(); M[1][1] = V.Y(); M[2][2] = V.Z();
}

//=========================================================================

NEINLINE neM4& neM4::operator *= ( const neM4& M )
{
    return (*this) = (*this) * M;
}

//=========================================================================

NEINLINE neV3 neM4::GetTranslation( void ) const
{
    neV3 tmp;
    return tmp.Set( M[3][0], M[3][1], M[3][2] );
}

//==============================================================================

NEINLINE void neM4::GetRows( neV3& V1, neV3& V2, neV3& V3 ) const
{
    V1.Set( M[0][0], M[1][0], M[2][0] );
    V2.Set( M[0][1], M[1][1], M[2][1] );
    V3.Set( M[0][2], M[1][2], M[2][2] ); 
}

//==============================================================================

NEINLINE void neM4::GetColumn(neV3& V1, u32 col) const
{
    V1.Set( M[col][0], M[col][1], M[col][2] );
    //V1.Set( M[0][row], M[1][row], M[2][row] );
}

//==============================================================================

NEINLINE void neM4::SetRows( const neV3& V1, const neV3& V2, const neV3& V3 )
{
    M[0][0] = V1.X();  M[1][0] = V1.Y();  M[2][0] = V1.Z();
    M[0][1] = V2.X();  M[1][1] = V2.Y();  M[2][1] = V2.Z();
    M[0][2] = V3.X();  M[1][2] = V3.Y();  M[2][2] = V3.Z();
}

//==============================================================================

NEINLINE void neM4::GetColumns( neV3& V1, neV3& V2, neV3& V3 ) const
{
    V1.Set( M[0][0], M[0][1], M[0][2] );
    V2.Set( M[1][0], M[1][1], M[1][2] );
    V3.Set( M[2][0], M[2][1], M[2][2] ); 
}

//==============================================================================

NEINLINE void neM4::SetColumns( const neV3& V1, const neV3& V2, const neV3& V3 )
{
    M[0][0] = V1.X();  M[0][1] = V1.Y();  M[0][2] = V1.Z();
    M[1][0] = V2.X();  M[1][1] = V2.Y();  M[1][2] = V2.Z();
    M[2][0] = V3.X();  M[2][1] = V3.Y();  M[2][2] = V3.Z();
}

//=========================================================================

NEINLINE neV3 neM4::TransformAs3x3( const neV3& V ) const
{
    neV3 tmp;
    return tmp.Set( M[0][0] * V.X() + M[1][0] * V.Y() + M[2][0] * V.Z(),
                     M[0][1] * V.X() + M[1][1] * V.Y() + M[2][1] * V.Z(),
                     M[0][2] * V.X() + M[1][2] * V.Y() + M[2][2] * V.Z() );
}

/****************************************************************************
*
*   friend functions
*
****************************************************************************/ 

//=========================================================================

NEINLINE neM4 operator * ( const neM4& M1, const neM4& M2 )
{
    neM4 Temp;
    f32*  L;
    f32*  R;

    L = (f32*)M1.M;
    R = (f32*)M2.M;

    for( f32* D = (f32*)&Temp.M[0]; D < (f32*)&Temp.M[4]; D+=4, R+=4 )
    {
        D[0] = L[0] * R[0] + L[4] * R[1] + L[8]  * R[2] + L[12] * R[3];
        D[1] = L[1] * R[0] + L[5] * R[1] + L[9]  * R[2] + L[13] * R[3];
        D[2] = L[2] * R[0] + L[6] * R[1] + L[10] * R[2] + L[14] * R[3];
        D[3] = L[3] * R[0] + L[7] * R[1] + L[11] * R[2] + L[15] * R[3];
    }

    return Temp;
}

//=========================================================================

NEINLINE neV3 operator * ( const neM4& M, const neV3& V )
{
    neV3 tmp;
    return tmp.Set( M.M[0][0] * V.X() + M.M[1][0] * V.Y() + M.M[2][0] * V.Z() + M.M[3][0],
                     M.M[0][1] * V.X() + M.M[1][1] * V.Y() + M.M[2][1] * V.Z() + M.M[3][1],
                     M.M[0][2] * V.X() + M.M[1][2] * V.Y() + M.M[2][2] * V.Z() + M.M[3][2] );
}

//=========================================================================

NEINLINE void neM4::SetFastInvert( const neM4& Src )
{
    f32     Determinant;
    neM4    & Matrix = (*this);

    //
    // Calculate the determinant.
    //
    Determinant = ( Src.M[0][0] * ( Src.M[1][1] * Src.M[2][2] - Src.M[1][2] * Src.M[2][1] ) -
                    Src.M[0][1] * ( Src.M[1][0] * Src.M[2][2] - Src.M[1][2] * Src.M[2][0] ) +
                    Src.M[0][2] * ( Src.M[1][0] * Src.M[2][1] - Src.M[1][1] * Src.M[2][0] ) );

    if( fabs( Determinant ) < 0.0001f ) 
    {
        Matrix.SetIdentity();
        //return Matrix; 
    }

    Determinant = 1.0f / Determinant;

    //
    // Find the inverse of the matrix.
    //
    Matrix.M[0][0] =  Determinant * ( Src.M[1][1] * Src.M[2][2] - Src.M[1][2] * Src.M[2][1] );
    Matrix.M[0][1] = -Determinant * ( Src.M[0][1] * Src.M[2][2] - Src.M[0][2] * Src.M[2][1] );
    Matrix.M[0][2] =  Determinant * ( Src.M[0][1] * Src.M[1][2] - Src.M[0][2] * Src.M[1][1] );
    Matrix.M[0][3] = 0.0f;

    Matrix.M[1][0] = -Determinant * ( Src.M[1][0] * Src.M[2][2] - Src.M[1][2] * Src.M[2][0] );
    Matrix.M[1][1] =  Determinant * ( Src.M[0][0] * Src.M[2][2] - Src.M[0][2] * Src.M[2][0] );
    Matrix.M[1][2] = -Determinant * ( Src.M[0][0] * Src.M[1][2] - Src.M[0][2] * Src.M[1][0] );
    Matrix.M[1][3] = 0.0f;

    Matrix.M[2][0] =  Determinant * ( Src.M[1][0] * Src.M[2][1] - Src.M[1][1] * Src.M[2][0] );
    Matrix.M[2][1] = -Determinant * ( Src.M[0][0] * Src.M[2][1] - Src.M[0][1] * Src.M[2][0] );
    Matrix.M[2][2] =  Determinant * ( Src.M[0][0] * Src.M[1][1] - Src.M[0][1] * Src.M[1][0] );
    Matrix.M[2][3] = 0.0f;

    Matrix.M[3][0] = -( Src.M[3][0] * Matrix.M[0][0] + Src.M[3][1] * Matrix.M[1][0] + Src.M[3][2] * Matrix.M[2][0] );
    Matrix.M[3][1] = -( Src.M[3][0] * Matrix.M[0][1] + Src.M[3][1] * Matrix.M[1][1] + Src.M[3][2] * Matrix.M[2][1] );
    Matrix.M[3][2] = -( Src.M[3][0] * Matrix.M[0][2] + Src.M[3][1] * Matrix.M[1][2] + Src.M[3][2] * Matrix.M[2][2] );
    Matrix.M[3][3] = 1.0f;

    //return Matrix;
}

//=========================================================================

NEINLINE neM4 neM4_BuildRotX( neRadian Rx )
{
    f32 s = (f32)sin( Rx );
    f32 c = (f32)cos( Rx );
    neM4 Temp;

    Temp.SetIdentity();

    Temp.M[1][1] = Temp.M[2][2] = c;
    Temp.M[2][1] = -s;
    Temp.M[1][2] =  s;

    return Temp;
}
    
//=========================================================================

NEINLINE neM4 neM4_BuildRotY( neRadian Ry )
{
    f32 s = (f32)sin( Ry );
    f32 c = (f32)cos( Ry );
    neM4 Temp;

    Temp.SetIdentity();
    Temp.M[0][0] = Temp.M[2][2] = c;
    Temp.M[0][2] = s;
    Temp.M[2][0] = - s;

    return Temp;
}

//=========================================================================

NEINLINE neM4 neM4_BuildRotZ( neRadian Rz )
{
    f32 s = (f32)sin( Rz );
    f32 c = (f32)cos( Rz );
    neM4 Temp;

    Temp.SetIdentity();
    Temp.M[0][0] = Temp.M[1][1] = c;
    Temp.M[0][1] =  s;
    Temp.M[1][0] = -s;

    return Temp;
}

//=========================================================================

NEINLINE neM4 neM4_BuildScale( const neV3 & Scale )
{
    neM4 Temp;

    Temp.SetIdentity();
    Temp.M[0][0] = Scale[0]; 
    Temp.M[1][1] = Scale[1]; 
    Temp.M[2][2] = Scale[2];

    return Temp;
}

//=========================================================================

NEINLINE neM4 neM4_BuildTranslate( const neV3 & Trans )
{
    neM4 Temp;

    Temp.SetIdentity();
    Temp.SetTranslation( Trans );

    return Temp;
}

//=========================================================================

NEINLINE neM4 neM4_BuildSkewSymmetric( const neV3& Vector )
{
    neM4 m;

    m.M[0][0] =  0;        m.M[0][1] =  Vector.Z(); m.M[0][2] = -Vector.Y();
    m.M[1][0] = -Vector.Z(); m.M[1][1] = 0;         m.M[1][2] =  Vector.X();
    m.M[2][0] =  Vector.Y(); m.M[2][1] = -Vector.X(); m.M[2][2] = 0;

    m.M[3][0] = m.M[3][1] = m.M[3][2] = 
    m.M[0][3] = m.M[1][3] = m.M[2][3] = 0;
    m.M[3][3] = 1;

    return m;
}

NEINLINE neM4& neM4::operator =( const neM3& m3 )
{
    int i, j;
    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
            M[i][j] = m3.M[i][j];
    return *this;
}

NEINLINE void neM4::SetTranspose(const neM4 & M)
{
    neM4 m;

    m.M[0][0] = M.M[0][0];  
    m.M[1][0] = M.M[0][1];  
    m.M[2][0] = M.M[0][2]; 
    m.M[3][0] = M.M[0][3];

    m.M[0][1] = M.M[1][0];  
    m.M[1][1] = M.M[1][1];  
    m.M[2][1] = M.M[1][2]; 
    m.M[3][1] = M.M[1][3];

    m.M[0][2] = M.M[2][0];  
    m.M[1][2] = M.M[2][1];  
    m.M[2][2] = M.M[2][2];
    m.M[3][2] = M.M[2][3];

    m.M[0][3] = M.M[3][0];  
    m.M[1][3] = M.M[3][1];  
    m.M[2][3] = M.M[3][2];  
    m.M[3][3] = M.M[3][3];

    (*this) = m;
}

/**ne_math_m3_inline.h**/

NEINLINE neV3&   neM3::operator   []       ( s32 I )
{
    ASSERT(I >= 0);
    ASSERT(I < 3);

    return M[I];
}

NEINLINE neV3   neM3::operator   []     ( s32 I ) const
{
    ASSERT(I >= 0);
    ASSERT(I < 3);

    return M[I];
}

NEINLINE void neM3::SetZero( void )
{
    M[0][0] = 0.0f; 
    M[1][0] = 0.0f; 
    M[2][0] = 0.0f;
    M[0][1] = 0.0f;
    M[1][1] = 0.0f;
    M[2][1] = 0.0f;
    M[0][2] = 0.0f;
    M[1][2] = 0.0f;
    M[2][2] = 0.0f;
}

NEINLINE void neM3::SetIdentity( void )
{
    M[0][1] = M[1][0] = M[2][0] = 0.0f;
    M[0][2] = M[1][2] = M[2][1] = 0.0f;    
    M[0][0] = M[1][1] = M[2][2] = 1.0f;
}

NEINLINE neM3 & neM3::SetTranspose( neM3 & m )
{
    if (this == &m)
    {
        neSwap(M[1].v[0], M[0].v[1]);  
        neSwap(M[2].v[0], M[0].v[2]); 

        //neSwap(M[0].Y, M[1].X);  
        neSwap(M[2].v[1], M[1].v[2]); 

        //neSwap(M[0].Z, M[2].X);  
        //neSwap(M[1].Z, M[2].Y);  
    }
    else
    {
        M[0].v[0] = m.M[0].v[0];  
        M[1].v[0] = m.M[0].v[1];  
        M[2].v[0] = m.M[0].v[2]; 

        M[0].v[1] = m.M[1].v[0];  
        M[1].v[1] = m.M[1].v[1];  
        M[2].v[1] = m.M[1].v[2]; 

        M[0].v[2] = m.M[2].v[0];  
        M[1].v[2] = m.M[2].v[1];  
        M[2].v[2] = m.M[2].v[2];
    }
    return (*this);
}

NEINLINE neV3 neM3::TransposeMulV3(const neV3 & V)
{
    neV3 tmp;
    
    tmp[0] = M[0].v[0] * V.X() + M[0].v[1] * V.Y() + M[0].v[2] * V.Z();
    tmp[1] = M[1].v[0] * V.X() + M[1].v[1] * V.Y() + M[1].v[2] * V.Z();
    tmp[2] = M[2].v[0] * V.X() + M[2].v[1] * V.Y() + M[2].v[2] * V.Z();
    
    return tmp;
}

NEINLINE neM3 operator * ( const neM3& m1, const neM3& m2 )
{
    neM3 tmp;

    tmp.M[0].v[0] = m2.M[0].v[0] * m1.M[0].v[0] + m2.M[0].v[1] * m1.M[1].v[0] + m2.M[0].v[2] * m1.M[2].v[0];
    tmp.M[0].v[1] = m2.M[0].v[0] * m1.M[0].v[1] + m2.M[0].v[1] * m1.M[1].v[1] + m2.M[0].v[2] * m1.M[2].v[1];
    tmp.M[0].v[2] = m2.M[0].v[0] * m1.M[0].v[2] + m2.M[0].v[1] * m1.M[1].v[2] + m2.M[0].v[2] * m1.M[2].v[2];
    tmp.M[1].v[0] = m2.M[1].v[0] * m1.M[0].v[0] + m2.M[1].v[1] * m1.M[1].v[0] + m2.M[1].v[2] * m1.M[2].v[0];
    tmp.M[1].v[1] = m2.M[1].v[0] * m1.M[0].v[1] + m2.M[1].v[1] * m1.M[1].v[1] + m2.M[1].v[2] * m1.M[2].v[1];
    tmp.M[1].v[2] = m2.M[1].v[0] * m1.M[0].v[2] + m2.M[1].v[1] * m1.M[1].v[2] + m2.M[1].v[2] * m1.M[2].v[2];
    tmp.M[2].v[0] = m2.M[2].v[0] * m1.M[0].v[0] + m2.M[2].v[1] * m1.M[1].v[0] + m2.M[2].v[2] * m1.M[2].v[0];
    tmp.M[2].v[1] = m2.M[2].v[0] * m1.M[0].v[1] + m2.M[2].v[1] * m1.M[1].v[1] + m2.M[2].v[2] * m1.M[2].v[1];
    tmp.M[2].v[2] = m2.M[2].v[0] * m1.M[0].v[2] + m2.M[2].v[1] * m1.M[1].v[2] + m2.M[2].v[2] * m1.M[2].v[2];
/*  
    tmp.M[0][0] = m1[0][0] * m2[0][0] + m1[1][0] * m2[0][1] + m1[2][0] * m2[0][2];
    tmp.M[0][1] = m1[0][1] * m2[0][0] + m1[1][1] * m2[0][1] + m1[2][1] * m2[0][2];
    tmp.M[0][2] = m1[0][2] * m2[0][0] + m1[1][2] * m2[0][1] + m1[2][2] * m2[0][2];

    tmp.M[1][0] = m1[0][0] * m2[1][0] + m1[1][0] * m2[1][1] + m1[2][0] * m2[1][2];
    tmp.M[1][1] = m1[0][1] * m2[1][0] + m1[1][1] * m2[1][1] + m1[2][1] * m2[1][2];
    tmp.M[1][2] = m1[0][2] * m2[1][0] + m1[1][2] * m2[1][1] + m1[2][2] * m2[1][2];

    tmp.M[2][0] = m1[0][0] * m2[2][0] + m1[1][0] * m2[2][1] + m1[2][0] * m2[2][2];
    tmp.M[2][1] = m1[0][1] * m2[2][0] + m1[1][1] * m2[2][1] + m1[2][1] * m2[2][2];
    tmp.M[2][2] = m1[0][2] * m2[2][0] + m1[1][2] * m2[2][1] + m1[2][2] * m2[2][2];
*/
    return tmp;
}

NEINLINE neV3 operator * ( const neM3& m1, const neV3& v)
{
    neV3 tmp;
    
    tmp[0] = m1.M[0].v[0] * v.X() + m1.M[1].v[0] * v.Y() + m1.M[2].v[0] * v.Z();
    tmp[1] = m1.M[0].v[1] * v.X() + m1.M[1].v[1] * v.Y() + m1.M[2].v[1] * v.Z();
    tmp[2] = m1.M[0].v[2] * v.X() + m1.M[1].v[2] * v.Y() + m1.M[2].v[2] * v.Z();
    
    return tmp;
}

NEINLINE neM3& operator *= ( neM3& M1, const f32 f  )
{
    M1.M[0].v[0] *= f;
    M1.M[0].v[1] *= f;
    M1.M[0].v[2] *= f;
    M1.M[1].v[0] *= f;
    M1.M[1].v[1] *= f;
    M1.M[1].v[2] *= f;
    M1.M[2].v[0] *= f;
    M1.M[2].v[1] *= f;
    M1.M[2].v[2] *= f;
    
    return M1;
}

NEINLINE neM3& operator /= ( neM3& M1, const f32 f  )
{
    M1.M[0].v[0] /= f;
    M1.M[0].v[1] /= f;
    M1.M[0].v[2] /= f;
    M1.M[1].v[0] /= f;
    M1.M[1].v[1] /= f;
    M1.M[1].v[2] /= f;
    M1.M[2].v[0] /= f;
    M1.M[2].v[1] /= f;
    M1.M[2].v[2] /= f;
    
    return M1;
}

NEINLINE neM3& operator -= ( neM3& M1, const neM3& M2)
{
    M1.M[0].v[0] -= M2.M[0].v[0];
    M1.M[0].v[1] -= M2.M[0].v[1];
    M1.M[0].v[2] -= M2.M[0].v[2];
    M1.M[1].v[0] -= M2.M[1].v[0];
    M1.M[1].v[1] -= M2.M[1].v[1];
    M1.M[1].v[2] -= M2.M[1].v[2];
    M1.M[2].v[0] -= M2.M[2].v[0];
    M1.M[2].v[1] -= M2.M[2].v[1];
    M1.M[2].v[2] -= M2.M[2].v[2];
    
    return M1;
}

NEINLINE void neM3::GetColumns( neV3& V1, neV3& V2, neV3& V3 ) const
{
    V1.Set( M[0][0], M[0][1], M[0][2] );
    V2.Set( M[1][0], M[1][1], M[1][2] );
    V3.Set( M[2][0], M[2][1], M[2][2] ); 
}

NEINLINE void neM3::SetColumns( const neV3& V1, const neV3& V2, const neV3& V3 )
{
    M[0][0] = V1.X();  M[0][1] = V1.Y();  M[0][2] = V1.Z();
    M[1][0] = V2.X();  M[1][1] = V2.Y();  M[1][2] = V2.Z();
    M[2][0] = V3.X();  M[2][1] = V3.Y();  M[2][2] = V3.Z();
}

NEINLINE neBool neM3::IsIdentity() const
{
    if (!(neIsConsiderZero(M[0][0] - 1.0f) && 
        neIsConsiderZero(M[1][1] - 1.0f) &&
        neIsConsiderZero(M[2][2] - 1.0f)))
        return false;

    return (neIsConsiderZero(M[0][1]) && 
            neIsConsiderZero(M[0][2]) &&
            neIsConsiderZero(M[1][0]) &&
            neIsConsiderZero(M[1][2]) &&
            neIsConsiderZero(M[2][0]) &&
            neIsConsiderZero(M[2][1]));
}

NEINLINE neBool neM3::SetInvert(const neM3 & rhs)
{
    f32     det, invDet;

    M[2] = rhs.M[0].Cross ( rhs.M[1] );

    det = rhs.M[2].Dot(M[2]);

    if ( neAbs(det) < 1.0e-17f )
    {
        return false;
    }

    M[0] = rhs.M[1].Cross( rhs.M[2] );
    
    M[1] = rhs.M[2].Cross( rhs.M[0] );
    
    invDet = 1.0f / det;
    
    M[0] = M[0] * invDet;
    
    M[1] = M[1] * invDet;
    
    M[2] = M[2] * invDet;
    
    SetTranspose(*this);

    return true;
}

NEINLINE neM3 operator +    ( const neM3& add1, const neM3& add2)
{
    neM3 sum;

    sum[0] = add1[0] + add2[0];
    sum[1] = add1[1] + add2[1];
    sum[2] = add1[2] + add2[2];

    return sum;
}

NEINLINE neM3 operator -    ( const neM3& add1, const neM3& add2)
{
    neM3 sum;

    sum[0] = add1[0] - add2[0];
    sum[1] = add1[1] - add2[1];
    sum[2] = add1[2] - add2[2];

    return sum;
}

NEINLINE neM3 & neM3::operator +=   ( const neM3& add)
{
    (*this)[0] += add[0];
    (*this)[1] += add[1];
    (*this)[2] += add[2];

    return (*this);
}

NEINLINE neV3 neM3::GetDiagonal()
{
    neV3 ret;

    ret[0] = M[0][0];
    ret[1] = M[1][1];
    ret[2] = M[2][2];

    return ret;
}

NEINLINE neM3 neM3::operator ^ (const neV3 & v) const
{
    neM3 ret;

    ret[0][0] = M[1][0] * v.Z() - M[2][0] * v.Y();
    ret[0][1] = M[1][1] * v.Z() - M[2][1] * v.Y();
    ret[0][2] = M[1][2] * v.Z() - M[2][2] * v.Y();

    ret[1][0] = M[0][0] * -v.Z() + M[2][0] * v.X();
    ret[1][1] = M[0][1] * -v.Z() + M[2][1] * v.X();
    ret[1][2] = M[0][2] * -v.Z() + M[2][2] * v.X(); 
    
    ret[2][0] = M[0][0] * v.Y() - M[1][0] * v.X();
    ret[2][1] = M[0][1] * v.Y() - M[1][1] * v.X();
    ret[2][2] = M[0][2] * v.Y() - M[1][2] * v.X();

    return ret;
}

NEINLINE neM3 operator ^ (const neV3 & v, const neM3 & matrix)
{
    neM3 ret;

    ret[0][0] = -v.Z() * matrix[0][1] + v.Y() * matrix[0][2];
    ret[0][1] =  v.Z() * matrix[0][0] - v.X() * matrix[0][2];
    ret[0][2] = -v.Y() * matrix[0][0] + v.X() * matrix[0][1];

    ret[1][0] = -v.Z() * matrix[1][1] + v.Y() * matrix[1][2]; 
    ret[1][1] =  v.Z() * matrix[1][0] - v.X() * matrix[1][2]; 
    ret[1][2] = -v.Y() * matrix[1][0] + v.X() * matrix[1][1]; 
    
    ret[2][0] = -v.Z() * matrix[2][1] + v.Y() * matrix[2][2]; 
    ret[2][1] =  v.Z() * matrix[2][0] - v.X() * matrix[2][2]; 
    ret[2][2] = -v.Y() * matrix[2][0] + v.X() * matrix[2][1]; 

    return ret;
}

NEINLINE neBool neM3::IsOrthogonalNormal() const
{
    neV3 cross;

    const neM3 & me = (*this);

    f32 m = me[0].Length();

    if (!neIsConsiderZero(m - 1.0f))
        return false;

    m = me[1].Length();

    if (!neIsConsiderZero(m - 1.0f))
        return false;

    m = me[2].Length();

    if (!neIsConsiderZero(m - 1.0f))
        return false;

    cross = me[0].Cross(me[1]);

    f32 dot = cross.Dot(me[2]);

    if (!neIsConsiderZero(dot - 1.0f))
        return false;

    return true;
}

NEINLINE void neM3::RotateXYZ(const neV3 & rotate)
{
    neM3 rx, ry, rz;

    f32 sintheta, costheta;

    sintheta = sinf(rotate[0]);
    costheta = cosf(rotate[0]);
    
    rx[0].Set(1.0f, 0.0f, 0.0f);
    rx[1].Set(0.0f, costheta, sintheta);
    rx[2].Set(0.0f,-sintheta, costheta);

    sintheta = sinf(rotate[1]);
    costheta = cosf(rotate[1]);

    ry[0].Set(costheta, 0.0f, -sintheta);
    ry[1].Set(0.0f, 1.0f, 0.0f);
    ry[2].Set(sintheta, 0.0f, costheta);

    sintheta = sinf(rotate[2]);
    costheta = cosf(rotate[2]);

    rz[0].Set(costheta, sintheta, 0.0f);
    rz[1].Set(-sintheta, costheta, 0.0f);
    rz[2].Set(0.0f, 0.0f, 1.0f);

    (*this) = rz * ry * rx;
}

NEINLINE neM3 neM3::operator * (f32 scalar) const
{
    neM3 ret;

    ret[0] = (*this)[0] * scalar;
    ret[1] = (*this)[1] * scalar;
    ret[2] = (*this)[2] * scalar;
    return ret;
}

NEINLINE neM3 operator * ( f32 scalar, const neM3& M )
{
    neM3 ret;

    ret[0] = M[0] * scalar;
    ret[1] = M[1] * scalar;
    ret[2] = M[2] * scalar;
    return ret;
}

NEINLINE neM3 & neM3::SkewSymmetric(const neV3 & V)
{
    (*this)[0].Set(0.0f, V[2], -V[1]);
    
    (*this)[1].Set(-V[2], 0.0f, V[0]);
    
    (*this)[2].Set(V[1], -V[0], 0.0f);

    return (*this);
}

neBool neM3::IsFinite() const
{
    return ((*this)[0].IsFinite() &&
            (*this)[1].IsFinite() &&
            (*this)[2].IsFinite() );
}

#ifdef USE_OPCODE

NEINLINE neV3 & neV3::operator = (const IceMaths::Point & pt)
{
    (*this)[0] = pt[0];
    (*this)[1] = pt[1];
    (*this)[2] = pt[2];

    return (*this);
}

NEINLINE IceMaths::Point& neV3::AssignIcePoint(IceMaths::Point & pt) const
{
    pt[0] = (*this)[0];
    pt[1] = (*this)[1];
    pt[2] = (*this)[2];
    return pt;
}

#endif //USE_OPCODE

/**ne_math_q_inline.h**/

///////////////////////////////////////////////////////////////////////////
// neQ inlines
///////////////////////////////////////////////////////////////////////////

//=========================================================================

NEINLINE neQ::neQ( void ) 
{
    Identity();
}

//=========================================================================

NEINLINE neQ::neQ( f32 x, f32 y, f32 z, f32 w )
{
    X = x; Y = y; Z = z; W = w;
}

//==========================================================================

NEINLINE neQ::neQ( const neM4& M )
{
    SetupFromMatrix( M );
}

//==========================================================================

NEINLINE void neQ::Zero( void )
{
    X = Y = Z = W = 0;
}

//==========================================================================

NEINLINE void neQ::Identity( void )
{
    X = Y = Z = 0; W = 1;
}

//==========================================================================

NEINLINE neQ& neQ::Invert( void )
{
    // This can be done also be negating (X,Y,Z) stead of W
    W = -W; 
    return *this;
}

//=========================================================================

NEINLINE neM4 neQ::BuildMatrix( void ) const
{
    neM4 M;

    const f32 tx  = 2.0f*X;    
    const f32 ty  = 2.0f*Y;    
    const f32 tz  = 2.0f*Z;
    const f32 twx = tx*W;    
    const f32 twy = ty*W;    
    const f32 twz = tz*W;
    const f32 txx = tx*X;    
    const f32 txy = ty*X;    
    const f32 txz = tz*X;
    const f32 tyy = ty*Y;   
    const f32 tyz = tz*Y;   
    const f32 tzz = tz*Z;

    M.M[0][0] = 1.0f-(tyy+tzz);   
    M.M[1][0] = txy-twz;          
    M.M[2][0] = txz+twy;
    M.M[0][1] = txy+twz;          
    M.M[1][1] = 1.0f-(txx+tzz);   
    M.M[2][1] = tyz-twx;
    M.M[0][2] = txz-twy;          
    M.M[1][2] = tyz+twx;          
    M.M[2][2] = 1.0f-(txx+tyy);

    M.M[3][0] = M.M[3][1] = M.M[3][2] = 
    M.M[0][3] = M.M[1][3] = M.M[2][3] = 0.0f;
    M.M[3][3] = 1.0f;

    return M;
}

NEINLINE neM3 neQ::BuildMatrix3( void ) const
{
    neM3 M;

    const f32 tx  = 2.0f*X;    
    const f32 ty  = 2.0f*Y;    
    const f32 tz  = 2.0f*Z;
    const f32 twx = tx*W;    
    const f32 twy = ty*W;    
    const f32 twz = tz*W;
    const f32 txx = tx*X;    
    const f32 txy = ty*X;    
    const f32 txz = tz*X;
    const f32 tyy = ty*Y;   
    const f32 tyz = tz*Y;   
    const f32 tzz = tz*Z;

    M.M[0][0] = 1.0f-(tyy+tzz);   
    M.M[1][0] = txy-twz;          
    M.M[2][0] = txz+twy;
    M.M[0][1] = txy+twz;          
    M.M[1][1] = 1.0f-(txx+tzz);   
    M.M[2][1] = tyz-twx;
    M.M[0][2] = txz-twy;          
    M.M[1][2] = tyz+twx;          
    M.M[2][2] = 1.0f-(txx+tyy);

    return M;
}

//=========================================================================

NEINLINE void neQ::SetupFromMatrix( const neM4& Matrix )
{
    // squared magniudes of quaternion components
    // first compute squared magnitudes of quaternion components - at least one
    // will be greater than 0 since quaternion is unit magnitude
    const f32 qs2 = 0.25f * (Matrix.M[0][0] + Matrix.M[1][1] + Matrix.M[2][2] + 1.0f );
    const f32 qx2 = qs2 - 0.5f * (Matrix.M[1][1] + Matrix.M[2][2]);
    const f32 qy2 = qs2 - 0.5f * (Matrix.M[2][2] + Matrix.M[0][0]);
    const f32 qz2 = qs2 - 0.5f * (Matrix.M[0][0] + Matrix.M[1][1]);


    // find maximum magnitude component
    const s32 i = (qs2 > qx2 ) ?
    ((qs2 > qy2) ? ((qs2 > qz2) ? 0 : 3) : ((qy2 > qz2) ? 2 : 3)) :
    ((qx2 > qy2) ? ((qx2 > qz2) ? 1 : 3) : ((qy2 > qz2) ? 2 : 3));

    // compute signed quaternion components using numerically stable method
    switch(i) 
    {
        case 0:
            {
                W = (f32)sqrt(qs2);
                const f32 tmp = 0.25f / W;
                X = (Matrix.M[1][2] - Matrix.M[2][1]) * tmp;
                Y = (Matrix.M[2][0] - Matrix.M[0][2]) * tmp;
                Z = (Matrix.M[0][1] - Matrix.M[1][0]) * tmp;
                break;
            }
        case 1:
            {
                X = (f32)sqrt(qx2);
                const f32 tmp = 0.25f / X;
                W = (Matrix.M[1][2] - Matrix.M[2][1]) * tmp;
                Y = (Matrix.M[1][0] + Matrix.M[0][1]) * tmp;
                Z = (Matrix.M[2][0] + Matrix.M[0][2]) * tmp;
                break;
            }
        case 2:
            {
                Y = (f32)sqrt(qy2);
                const f32 tmp = 0.25f / Y;
                W = (Matrix.M[2][0] - Matrix.M[0][2]) * tmp;
                Z = (Matrix.M[2][1] + Matrix.M[1][2]) * tmp;
                X = (Matrix.M[0][1] + Matrix.M[1][0]) * tmp;
                break;
            }
        case 3:
            {
                Z = (f32)sqrt(qz2);
                const f32 tmp = 0.25f / Z;
                W = (Matrix.M[0][1] - Matrix.M[1][0]) * tmp;
                X = (Matrix.M[0][2] + Matrix.M[2][0]) * tmp;
                Y = (Matrix.M[1][2] + Matrix.M[2][1]) * tmp;
                break;
            }
    }

    // for consistency, force positive scalar component [ (s; v) = (-s; -v) ]
    if( W < 0) *this = -*this;

    // normalize, just to be safe
    Normalize();
}

NEINLINE void neQ::SetupFromMatrix3( const neM3& Matrix )
{
    neM4 m;
    
    m.SetIdentity();

    u32 i,j;

    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
            m.M[i][j] = Matrix.M[i][j]; 
    
    SetupFromMatrix(m);
}

//=========================================================================

NEINLINE f32 neQ::Dot( const neQ& Q ) const
{
    return Q.X*X + Q.Y*Y + Q.Z*Z + Q.W*W;
}

//=========================================================================

NEINLINE neQ& neQ::Normalize( void )
{
    f32 t;
    f32 norm = Dot(*this);

//  if (neRealsEqual(norm, 1.0f))
//      return *this;

    ASSERT(norm >= 0.0f);

    t = 1.0f / (float)sqrt(norm);

    X *= t;        
    Y *= t;        
    Z *= t;        
    W *= t;        

    return *this;
}

//=========================================================================

NEINLINE neQ& neQ::operator *= ( const neQ& Q )
{
    *this = *this * Q;
    return *this;
}

//=========================================================================

NEINLINE neQ& neQ::operator *= ( f32 S )
{
    *this = *this * S;
    return *this;
}

//==========================================================================

NEINLINE neQ& neQ::operator += ( const neQ& Q )
{
    *this = *this + Q;
    return *this;
}

//==========================================================================

NEINLINE neQ& neQ::operator -= ( const neQ& Q )
{
    *this = *this - Q;
    return *this;
}

//=========================================================================

NEINLINE neQ operator - ( const neQ& Q )
{
    return neQ( -Q.X, -Q.Y, -Q.Z, -Q.W );
}

//==========================================================================

NEINLINE neQ operator * ( const neQ& q1,  const neQ& q2 )
{
    f32     tmp_0,  //temporary variables
            tmp_1,
            tmp_2,
            tmp_3,
            tmp_4,
            tmp_5,
            tmp_6,
            tmp_7,
            tmp_8,
            tmp_9;

    neQ zq;

    tmp_0 = (q1.Z - q1.Y) * (q2.Y - q2.Z);
    tmp_1 = (q1.W + q1.X) * (q2.W + q2.X);
    tmp_2 = (q1.W - q1.X) * (q2.Y + q2.Z);
    tmp_3 = (q1.Y + q1.Z) * (q2.W - q2.X);
    tmp_4 = (q1.Z - q1.X) * (q2.X - q2.Y);
    tmp_5 = (q1.Z + q1.X) * (q2.X + q2.Y);
    tmp_6 = (q1.W + q1.Y) * (q2.W - q2.Z);
    tmp_7 = (q1.W - q1.Y) * (q2.W + q2.Z);
    tmp_8 = tmp_5 + tmp_6 + tmp_7;
    tmp_9 = 0.5f * (tmp_4 + tmp_8);


    zq.X =tmp_1 + tmp_9 - tmp_8;
    zq.Y =tmp_2 + tmp_9 - tmp_7;
    zq.Z =tmp_3 + tmp_9 - tmp_6;
    zq.W =tmp_0 + tmp_9 - tmp_5;

    return zq;

}

//==========================================================================

NEINLINE void neQ::GetAxisAngle( neV3& Axis, neRadian& Angle ) const
{
    f32 sum = X*X + Y*Y + Z*Z;

    if (neIsConsiderZero(sum))
    {
        Angle = 0.0f;

        Axis.Set(1.0f, 0.0f, 0.0f);

        return;
    }
    f32 OneOver = 1.0f/(f32)sqrtf( sum );

    Axis.Set( OneOver * X, OneOver * Y, OneOver * Z );

    f32 w = W;

    if (neIsConsiderZero(W - 1.0f))
    {
        Angle = 0.0f;
    }
    else
    {
        Angle = 2.0f * (f32)acosf( w );
    }
}

NEINLINE neV3 operator * ( const neQ& Q, const neV3& V )
{   
    neV3 Axis; Axis.Set(Q.X, Q.Y, Q.Z);
    neV3 uv; uv.Set( Axis.Cross( V ) );
    neV3 uuv; uuv.Set( Axis.Cross( uv ) * 2.0f );         

    return V + uuv + ( uv * Q.W * 2.0f );
}

//==========================================================================

NEINLINE neQ operator * ( const neQ& Q, f32 S )
{
    return neQ( Q.X*S, Q.Y*S, Q.Z*S, Q.W*S );
}

//==========================================================================

NEINLINE neQ operator * ( f32 S, const neQ& Q )
{
    return Q * S;
}

//==========================================================================

NEINLINE neQ operator + ( const neQ& Qa, const neQ& Qb )
{
    return neQ( Qa.X + Qb.X, Qa.Y + Qb.Y, Qa.Z + Qb.Z, Qa.W + Qb.W );
}

//==========================================================================

NEINLINE neQ operator - ( const neQ& Qa, const neQ& Qb )
{
    return neQ( Qa.X - Qb.X, Qa.Y - Qb.Y, Qa.Z - Qb.Z, Qa.W - Qb.W );
}

NEINLINE neQ&   neQ::Set(f32 _X, f32 _Y, f32 _Z, f32 _W)
{
    X = _X;
    Y = _Y;
    Z = _Z;
    W = _W;
    return (*this);
}

NEINLINE neQ&   neQ::Set(const neV3 & V, f32 W)
{
    return Set(V[0], V[1], V[2], W);
}

NEINLINE neQ& neQ::Set(f32 angle, const neV3 & axis)
{
    f32 halfAngle = angle * 0.5f;

    f32 sinHalfAngle = sinf(halfAngle);

    f32 cosHalfAngle = cosf(halfAngle);

    neV3 tmp = axis * sinHalfAngle;

    Set(tmp, cosHalfAngle);

    return (*this);
}

NEINLINE neBool neQ::IsFinite()
{
    return (neFinite(X) && 
            neFinite(Y) &&
            neFinite(Z) &&
            neFinite(W) );
}

/**ne_math_t3_inline.h**/

///////////////////////////////////////////////////////////////////////////
// neT3 
///////////////////////////////////////////////////////////////////////////

//=========================================================================

NEINLINE neT3 neT3::operator * (const neT3 & t)
{
    neT3 ret;

    ret.rot.M[0][0] = rot.M[0][0] * t.rot.M[0][0] + rot.M[1][0] * t.rot.M[0][1] + rot.M[2][0] * t.rot.M[0][2];
    ret.rot.M[0][1] = rot.M[0][1] * t.rot.M[0][0] + rot.M[1][1] * t.rot.M[0][1] + rot.M[2][1] * t.rot.M[0][2];
    ret.rot.M[0][2] = rot.M[0][2] * t.rot.M[0][0] + rot.M[1][2] * t.rot.M[0][1] + rot.M[2][2] * t.rot.M[0][2];

    ret.rot.M[1][0] = rot.M[0][0] * t.rot.M[1][0] + rot.M[1][0] * t.rot.M[1][1] + rot.M[2][0] * t.rot.M[1][2];
    ret.rot.M[1][1] = rot.M[0][1] * t.rot.M[1][0] + rot.M[1][1] * t.rot.M[1][1] + rot.M[2][1] * t.rot.M[1][2];
    ret.rot.M[1][2] = rot.M[0][2] * t.rot.M[1][0] + rot.M[1][2] * t.rot.M[1][1] + rot.M[2][2] * t.rot.M[1][2];

    ret.rot.M[2][0] = rot.M[0][0] * t.rot.M[2][0] + rot.M[1][0] * t.rot.M[2][1] + rot.M[2][0] * t.rot.M[2][2];
    ret.rot.M[2][1] = rot.M[0][1] * t.rot.M[2][0] + rot.M[1][1] * t.rot.M[2][1] + rot.M[2][1] * t.rot.M[2][2];
    ret.rot.M[2][2] = rot.M[0][2] * t.rot.M[2][0] + rot.M[1][2] * t.rot.M[2][1] + rot.M[2][2] * t.rot.M[2][2];

    ret.pos.v[0] = rot.M[0][0] * t.pos.v[0] + rot.M[1][0] * t.pos.v[1] + rot.M[2][0] * t.pos.v[2] + pos.v[0];
    ret.pos.v[1] = rot.M[0][1] * t.pos.v[0] + rot.M[1][1] * t.pos.v[1] + rot.M[2][1] * t.pos.v[2] + pos.v[1];
    ret.pos.v[2] = rot.M[0][2] * t.pos.v[0] + rot.M[1][2] * t.pos.v[1] + rot.M[2][2] * t.pos.v[2] + pos.v[2];

/*
    ret.rot[0] = rot[0] * t.rot.M[0][0] + rot[1] * t.rot.M[0][1] + rot[2] * t.rot.M[0][2];
    ret.rot[1] = rot[0] * t.rot.M[1][0] + rot[1] * t.rot.M[1][1] + rot[2] * t.rot.M[1][2];
    ret.rot[2] = rot[0] * t.rot.M[2][0] + rot[1] * t.rot.M[2][1] + rot[2] * t.rot.M[2][2];
    
    ret.pos = rot[0] * t.pos[0] + rot[1] * t.pos[1] + rot[2] * t.pos[2] + pos;
*/
    return ret;
}

NEINLINE neV3 neT3::operator * (const neV3 & v)
{
    return rot * v + pos;
}

NEINLINE neT3 neT3::FastInverse()
{
    neT3 ret;

    ret.rot.SetTranspose(rot);

    neV3 tpos = ret.rot * pos;

    ret.pos = -tpos;

    return ret;
}
    
#ifdef USE_OPCODE

NEINLINE neT3 & neT3::operator = (const IceMaths::Matrix4x4 & mat)
{
    (*this).rot[0][0] = mat.m[0][0];
    (*this).rot[0][1] = mat.m[0][1];
    (*this).rot[0][2] = mat.m[0][2];

    (*this).rot[1][0] = mat.m[1][0];
    (*this).rot[1][1] = mat.m[1][1];
    (*this).rot[1][2] = mat.m[1][2];

    (*this).rot[2][0] = mat.m[2][0];
    (*this).rot[2][1] = mat.m[2][1];
    (*this).rot[2][2] = mat.m[2][2];

    (*this).pos[0] = mat.m[3][0];
    (*this).pos[1] = mat.m[3][1];
    (*this).pos[2] = mat.m[3][2];


    return (*this);
}

NEINLINE IceMaths::Matrix4x4 & neT3::AssignIceMatrix(IceMaths::Matrix4x4 & mat) const
{
    mat.m[0][0] = (*this).rot[0][0];
    mat.m[0][1] = (*this).rot[0][1];
    mat.m[0][2] = (*this).rot[0][2];
    mat.m[0][3] = 0.0f;

    mat.m[1][0] = (*this).rot[1][0];
    mat.m[1][1] = (*this).rot[1][1];
    mat.m[1][2] = (*this).rot[1][2];
    mat.m[1][3] = 0.0f;

    mat.m[2][0] = (*this).rot[2][0];
    mat.m[2][1] = (*this).rot[2][1];
    mat.m[2][2] = (*this).rot[2][2];
    mat.m[2][3] = 0.0f;

    mat.m[3][0] = (*this).pos[0];
    mat.m[3][1] = (*this).pos[1];
    mat.m[3][2] = (*this).pos[2];
    mat.m[3][3] = 1.0f;

    return mat;
}

neBool neT3::IsFinite()
{
    return ((*this).rot.IsFinite() && (*this).pos.IsFinite());
}

#endif //USE_OPCODE

///////////////////////////////////////////////////////////////////////////
// END
///////////////////////////////////////////////////////////////////////////
#endif //NE_MATH_H


#define NE_INTERFACE(n) protected: n(){}; n& operator = (const n & e){return (*this);}

class TOKAMAK_API neRigidBody;

typedef enum
{
    NE_TERRAIN = 0, 
    NE_RIGID_BODY,
    NE_ANIMATED_BODY,
} neBodyType;

/****************************************************************************
*
*   Tokamak Game Physics SDK 
*
*   Class: neAllocatorAbstract
*
*   Desc:
*
****************************************************************************/ 

class neAllocatorAbstract
{
public:
    virtual neByte * Alloc(s32 size, s32 alignment = 0) = 0;

    virtual void   Free(neByte *) = 0;
};


/****************************************************************************
*
*   Tokamak Game Physics SDK 
*
*   Class: neAllocatorDefault
*
*   Desc:
*
****************************************************************************/ 

class neAllocatorDefault: public neAllocatorAbstract
{
public:
    neAllocatorDefault()
    {
        usedMem = 0;
    }
    neByte * Alloc(s32 size, s32 alignment = 0) {
        
        usedMem += size;

        return (neByte *)malloc(size);
    }
    void Free(neByte * ptr) {

        free(ptr);
    }
public:
    s32 usedMem;
};


/****************************************************************************
*
*   Tokamak Game Physics SDK 
*
*   Class: nePerformanceReport
*
*   Desc:
*
****************************************************************************/ 

class nePerformanceReport
{
public:
    enum{
        NE_PERF_TOTAL_TIME = 0,
        NE_PERF_DYNAMIC,
        NE_PERF_POSITION,
        NE_PERF_CONTRAIN_SOLVING_1,
        NE_PERF_CONTRAIN_SOLVING_2,
        NE_PERF_COLLISION_DETECTION,
        NE_PERF_COLLISION_CULLING,
        NE_PERF_TERRAIN_CULLING,
        NE_PERF_TERRAIN,
        NE_PERF_CONTROLLER_CALLBACK,
        NE_PERF_LAST,
    };
    enum
    {
        NE_PERF_RUNNING_AVERAGE = 0,
        NE_PERF_SAMPLE,
    };
    f32 time[NE_PERF_LAST];
    f32 accTime[NE_PERF_LAST];

    void Reset()
    {
        for (s32 i = 0; i < NE_PERF_LAST; i++)
        {
            time[i] = 0.0f;
            accTime[i] = 0.0f;
        }
        numSample = 0;
    }
    void SetReportType(s32 type)
    {
        reportType = type;
    }
    s32 reportType;
    s32 numSample;
};

/****************************************************************************
*
*   Tokamak Game Physics SDK 
*
*   Class: neGeometry
*
*   Desc:
*
****************************************************************************/ 

class TOKAMAK_API neGeometry;

typedef void (neBreakageCallback)(neByte * originalBody, neBodyType bodyType, neGeometry * brokenGeometry, neRigidBody * newBody);

class TOKAMAK_API neGeometry
{
NE_INTERFACE(neGeometry)

public:

typedef enum
{
    NE_BREAK_DISABLE,
    NE_BREAK_NORMAL,
    NE_BREAK_ALL,
    NE_BREAK_NEIGHBOUR,
    
    /*  the following are the same as above, 
        except it create a rigid particle instead of a rigid body 
    */

    NE_BREAK_NORMAL_PARTICLE, 
    NE_BREAK_ALL_PARTICLE,
    NE_BREAK_NEIGHBOUR_PARTICLE,
} neBreakFlag;

public:
    void    SetTransform(neT3 & t);
    
    void    SetMaterialIndex(s32 index);

    s32     GetMaterialIndex();

    neT3    GetTransform();

    void    SetUserData(uintptr_t userData);

    uintptr_t       GetUserData();

    /*
        Box
    */
    void    SetBoxSize(f32 width, f32 height, f32 depth);

    void    SetBoxSize(const neV3 & boxSize);

    neBool  GetBoxSize(neV3 & boxSize); // return false if geometry is not a box

    /*
        Sphere
    */
    void    SetSphereDiameter(f32 diameter);

    neBool  GetSphereDiameter(f32 & diameter); // return false if geometry is not a sphere

    /*
        Cylinder
    */
    void    SetCylinder(f32 diameter, f32 height);

    neBool  GetCylinder(f32 & diameter, f32 & height); // return false if geometry is not a cylinder

    /*
        Convex
    */
    void    SetConvexMesh(neByte * convexData);

    neBool  GetConvexMesh(neByte *& convexData);

    /*
        Breakage functions
    */
    void    SetBreakageFlag(neBreakFlag flag);

    neBreakFlag GetBreakageFlag();

    void    SetBreakageMass(f32 mass);

    f32     GetBreakageMass();
    
    void    SetBreakageInertiaTensor(const neV3 & tensor);

    neV3    GetBreakageInertiaTensor();

    void    SetBreakageMagnitude(f32 mag);

    f32     GetBreakageMagnitude();

    void    SetBreakageAbsorption(f32 absorb);

    f32     GetBreakageAbsorption();

    void    SetBreakagePlane(const neV3 & planeNormal);

    neV3    GetBreakagePlane();

    void    SetBreakageNeighbourRadius(f32 radius);

    f32     GetBreakageNeighbourRadius();
};

/****************************************************************************
*
*   Tokamak Game Physics SDK 
*
*   Class: neTriangleMesh
*
*   Desc:
*
****************************************************************************/ 
class neTriangle
{
public:
    neTriangle()
    {
        flag = NE_TRI_TRIANGLE;
        materialID = 0;
        indices[0] = -1;
        indices[1] = -1;
        indices[2] = -1;
        userData = 0;
    }

    enum
    {
        NE_TRI_TRIANGLE = 0,
        NE_TRI_HEIGHT_MAP,
    };
    s32 indices[3];
    s32 materialID;
    u32 flag;
    uintptr_t userData;
};

class neTriangleMesh
{
public:
    neV3 * vertices;

    s32 vertexCount;

    neTriangle * triangles;

    s32 triangleCount;
};

/****************************************************************************
*
*   Tokamak Game Physics SDK 
*
*   Class: neSensor
*
*   Desc:
*
****************************************************************************/ 

class TOKAMAK_API neRigidBody;

class TOKAMAK_API neAnimatedBody;

class TOKAMAK_API neSensor
{
NE_INTERFACE(neSensor)

public:
    void    SetLineSensor(const neV3 & pos, const neV3 & lineVector);

    void    SetUserData(uintptr_t userData);

    uintptr_t       GetUserData();

    neV3    GetLineVector();

    neV3    GetLineUnitVector();

    neV3    GetLinePos();

    f32     GetDetectDepth();

    neV3    GetDetectNormal();

    neV3    GetDetectContactPoint();

    neRigidBody * GetDetectRigidBody();

    neAnimatedBody * GetDetectAnimatedBody();

    s32     GetDetectMaterial();
};

/****************************************************************************
*
*   Tokamak Game Physics SDK 
*
*   Class: neAnimatedBody
*
*   Desc:
*
****************************************************************************/ 

class TOKAMAK_API neAnimatedBody
{
NE_INTERFACE(neAnimatedBody)

public:

//spatial states
    neV3    GetPos();
    
    void    SetPos(const neV3 & p);
    
    neM3    GetRotationM3();
    
    neQ     GetRotationQ();
    
    void    SetRotation(const neM3 & m);
    
    void    SetRotation(const neQ & q);
    
    neT3    GetTransform();

//collision related
    void    SetCollisionID(s32 cid);
    
    s32     GetCollisionID();
    
    void    SetUserData(uintptr_t userData);
    
    uintptr_t       GetUserData();
    
    s32     GetGeometryCount();
    
//collision geometries and sensors

    neGeometry *    AddGeometry();

    neBool          RemoveGeometry(neGeometry * g);

    void            BeginIterateGeometry();

    neGeometry *    GetNextGeometry();

    neRigidBody *   BreakGeometry(neGeometry * g);

    neSensor *      AddSensor();

    neBool          RemoveSensor(neSensor * s);

    void            BeginIterateSensor();

    neSensor *      GetNextSensor();

    void            UseCustomCollisionDetection(neBool yes, const neT3 * obb, f32 boundingRadius);

    neBool          UseCustomCollisionDetection();

//functions
    void    UpdateBoundingInfo();

    // collide with any body which connected to this body indirectly

    void    CollideConnected(neBool yes);

    neBool  CollideConnected();

    // collide with any body which connected to this body directly

    void    CollideDirectlyConnected(neBool yes);

    neBool  CollideDirectlyConnected();

    void    Active(neBool yes, neRigidBody * hint = NULL);

    void    Active(neBool yes, neAnimatedBody * hint = NULL);

    neBool  Active();
};

/****************************************************************************
*
*   Tokamak Game Physics SDK 
*
*   Class: neRigidBody
*
*   Desc:
*
****************************************************************************/ 

class TOKAMAK_API neRigidBodyController;

class TOKAMAK_API neJointController;

class neRigidBodyControllerCallback
{
public:
    virtual void RigidBodyControllerCallback(neRigidBodyController * controller, float timeStep) = 0;
};

class neJointControllerCallback
{
public:
    virtual void ConstraintControllerCallback(neJointController * controller, float timeStep) = 0;
};

class TOKAMAK_API neRigidBody
{
NE_INTERFACE(neRigidBody)

public:

//physical properties
    f32     GetMass();
    
    void    SetMass(f32 mass);
    
    void    SetInertiaTensor(const neM3 & tensor);

    void    SetInertiaTensor(const neV3 & tensor);

//other properties
    void    SetCollisionID(s32 cid);
    
    s32     GetCollisionID();
    
    void    SetUserData(uintptr_t userData);
    
    uintptr_t       GetUserData();
    
    s32     GetGeometryCount();

    void    SetLinearDamping(f32 damp); 

    f32     GetLinearDamping();

    void    SetAngularDamping(f32 damp);    

    f32     GetAngularDamping();

    void    SetSleepingParameter(f32 sleepingParam);

    f32     GetSleepingParameter();

//collision geometries, sensors and controllers
    
    neGeometry *    AddGeometry();

    neBool          RemoveGeometry(neGeometry * g);

    void            BeginIterateGeometry();

    neGeometry *    GetNextGeometry();

    neRigidBody *   BreakGeometry(neGeometry * g);

    void            UseCustomCollisionDetection(neBool yes,  const neT3 * obb, f32 boundingRadius);
    
    neBool          UseCustomCollisionDetection();

    neSensor *      AddSensor();

    neBool          RemoveSensor(neSensor * s);
    
    void            BeginIterateSensor();

    neSensor *      GetNextSensor();

    neRigidBodyController * AddController(neRigidBodyControllerCallback * controller, s32 period);

    neBool          RemoveController(neRigidBodyController * rbController);

    void            BeginIterateController();

    neRigidBodyController * GetNextController();

//spatial states
    neV3    GetPos();
    
    void    SetPos(const neV3 & p);
    
    neM3    GetRotationM3();
    
    neQ     GetRotationQ();
    
    void    SetRotation(const neM3 & m);
    
    void    SetRotation(const neQ & q);
    
    neT3    GetTransform();

//dynamic states
    neV3    GetVelocity();
    
    void    SetVelocity(const neV3 & v);
    
    neV3    GetAngularVelocity();
    
    neV3    GetAngularMomentum();
    
    void    SetAngularMomentum(const neV3& am);

    neV3    GetVelocityAtPoint(const neV3 & pt);

//functions
    void    UpdateBoundingInfo();
    
    void    UpdateInertiaTensor();
    
    void    SetForce(const neV3 & force);

    void    SetTorque(const neV3 & torque);
    
    void    SetForce(const neV3 & force, const neV3 & pos);

    neV3    GetForce();

    neV3    GetTorque();

    void    ApplyImpulse(const neV3 & impulse);

    void    ApplyImpulse(const neV3 & impulse, const neV3 & pos);

    void    ApplyTwist(const neV3 & twist);

    void    GravityEnable(neBool yes);

    neBool  GravityEnable();

    // collide with any body which connected to this body indirectly
    
    void    CollideConnected(neBool yes); 

    neBool  CollideConnected();

    // collide with any body which connected to this body directly

    void    CollideDirectlyConnected(neBool yes);

    neBool  CollideDirectlyConnected();

    void    Active(neBool yes, neRigidBody * hint = NULL);

    void    Active(neBool yes, neAnimatedBody * hint = NULL);

    neBool  Active();

    neBool  IsIdle();
};

/****************************************************************************
*
*   Tokamak Game Physics SDK 
*
*   Class: neJoint
*
*   Desc:
*
****************************************************************************/ 

class TOKAMAK_API neJoint
{
NE_INTERFACE(neJoint)

public:
    typedef enum
    {
        NE_JOINT_BALLSOCKET,
        NE_JOINT_BALLSOCKET2,
        NE_JOINT_HINGE,
        NE_JOINT_SLIDE,

    }ConstraintType;

    void SetType(ConstraintType t);

    ConstraintType GetType();

    void SetJointFrameA(const neT3 & frameA);

    void SetJointFrameB(const neT3 & frameB);

    void SetJointFrameWorld(const neT3 & frame);

    neT3 GetJointFrameA();

    neT3 GetJointFrameB();

    void SetJointLength(f32 length);

    f32 GetJointLength();

    neRigidBody * GetRigidBodyA();

    neRigidBody * GetRigidBodyB();

    neAnimatedBody * GetAnimatedBodyB();

    void Enable(neBool yes);

    neBool Enable();

    void SetDampingFactor(f32 damp);

    f32 GetDampingFactor();

    /*
        Query Joint position
    */

    f32 GetPosition();

    f32 GetPosition2();

    /*
        Constraint primary limit functions
    */
    neBool EnableLimit();

    void EnableLimit(neBool yes);

    f32 GetUpperLimit();

    void SetUpperLimit(f32 upperLimit);

    f32 GetLowerLimit();

    void SetLowerLimit(f32 lowerLimit);

    /*
        Constraint secondary limit functions (only apply to some Constraint types)
    */

    neBool EnableLimit2();

    void EnableLimit2(neBool yes);

    f32 GetUpperLimit2();

    void SetUpperLimit2(f32 upperLimit);

    f32 GetLowerLimit2();

    void SetLowerLimit2(f32 lowerLimit);

    /*
        relates to accuracy and speed of the joint solver
    */
    void SetEpsilon(f32 e);

    f32 GetEpsilon();

    void SetIteration(s32 i);

    s32 GetIteration();

    /*
        Constraint controller functions
    */

    neJointController * AddController(neJointControllerCallback * controller, s32 period);

    neBool  RemoveController(neJointController * rbController);

    void    BeginIterateController();

    neJointController * GetNextController();
    
    /*
        Constraint primary motor function, currently only implemented for hinge Constraint
     */
    enum MotorType
    {
        NE_MOTOR_SPEED,
        NE_MOTOR_POSITION, //not implemented
    };
    neBool EnableMotor();

    void EnableMotor(neBool yes);

    void SetMotor(MotorType motorType, f32 desireValue, f32 maxForce);

    void GetMotor(MotorType & motorType, f32 & desireValue, f32 & maxForce);

    neBool EnableMotor2();

    void EnableMotor2(neBool yes);

    void SetMotor2(MotorType motorType, f32 desireValue, f32 maxForce);

    void GetMotor2(MotorType & motorType, f32 & desireValue, f32 & maxForce);
};

/****************************************************************************
*
*   Tokamak Game Physics SDK 
*
*   Class: 
*
*   Desc:
*
****************************************************************************/ 

class TOKAMAK_API neRigidBodyController
{
NE_INTERFACE(neRigidBodyController);

public:
    neRigidBody * GetRigidBody();

    neV3 GetControllerForce();

    neV3 GetControllerTorque();
    
    void SetControllerForce(const neV3 & force);

    void SetControllerTorque(const neV3 & torque);

    void SetControllerForceWithTorque(const neV3 & force, const neV3 & pos);
};

class TOKAMAK_API neJointController
{
NE_INTERFACE(neJointController);

public:
    neJoint * GetJoint();

    neV3 GetControllerForceBodyA();

    neV3 GetControllerForceBodyB();

    neV3 GetControllerTorqueBodyA();

    neV3 GetControllerTorqueBodyB();

    void SetControllerForceBodyA(const neV3 & force);

    void SetControllerForceBodyB(const neV3 & force);

    void SetControllerForceWithTorqueBodyA(const neV3 & force, const neV3 & pos);

    void SetControllerForceWithTorqueBodyB(const neV3 & force, const neV3 & pos);

    void SetControllerTorqueBodyA(const neV3 & torque);

    void SetControllerTorqueBodyB(const neV3 & torque);
};
/****************************************************************************
*
*   Tokamak Game Physics SDK 
*
*   Class: neCollisionTable
*
*   Desc:
*
****************************************************************************/ 

class TOKAMAK_API neCollisionTable
{
NE_INTERFACE(neCollisionTable)

public:
    enum neReponseBitFlag
    {
        RESPONSE_IGNORE = 0,
        RESPONSE_IMPULSE = 1,
        RESPONSE_CALLBACK = 2,
        RESPONSE_IMPULSE_CALLBACK = 3,
    };

    enum
    {
        NE_COLLISION_TABLE_MAX = 64,
    };
    void Set(s32 collisionID1, s32 collisionID2, neReponseBitFlag response = RESPONSE_IMPULSE);

    neReponseBitFlag Get(s32 collisionID1, s32 collisionID2);

    s32 GetMaxCollisionID();
};

/****************************************************************************
*
*   Tokamak Game Physics SDK 
*
*   Class: neSimulatorSizeInfo
*
*   Desc:
*
****************************************************************************/ 


class neSimulatorSizeInfo
{
public:
    enum
    {
        DEFAULT_RIGIDBODIES_COUNT = 50,
        DEFAULT_ANIMATEDBODIES_COUNT = 50, 
        DEFAULT_RIGIDPARTICLES_COUNT = 50,

        DEFAULT_CONTROLLERS_COUNT = 50,
        DEFAULT_OVERLAPPED_PAIRS_COUNT = 1225,

        DEFAULT_GEOMETRIES_COUNT = 50,
        
        DEFAULT_CONSTRAINTS_COUNT = 100,
        DEFAULT_CONTRAINT_SETS_COUNT = 100,
        DEFAULT_SOLVER_BUFFER_SIZE = 2000,
        DEFAULT_SENSORS_COUNT = 100,

        DEFAULT_TERRAIN_NODES_START_COUNT = 200,
        DEFAULT_TERRAIN_NODES_GROWBY_COUNT = -1,
    };

public:
    
    s32 rigidBodiesCount;       /* Number of rigid bodies in the simulation */
    s32 animatedBodiesCount;    /* Number of animated bodies in the simulation */
    s32 rigidParticleCount;     /* Number of rigid particles in the simulation */

    s32 controllersCount;       /* Number of controller instances in the simulation */
    
    s32 overlappedPairsCount;   /* Number of possible overlapping pairs.
                                   This has the maximum value of (n x (n - 1)) / 2,
                                   where n = rigidBodyCount + animatedBodyCount.
                                   But in practice it rarely reach that high.
                                   You can try to specify a smaller number to save memory.
                                */
    s32 geometriesCount;        /* Number of collision geometries in the simulator*/


    s32 constraintsCount;       /* Number of joints in the simulation */
    s32 constraintSetsCount;    /* Number of joint Sets in the simulation */
    s32 constraintBufferSize;   /* Size of the buffer use to solve joints */
    s32 sensorsCount;

    s32 terrainNodesStartCount; /* Number of nodes use to store terrain triangles */
    s32 terrainNodesGrowByCount;/* Grow by this size if run out of nodes */

public:
    
    neSimulatorSizeInfo()       /* Fill with default size values */
    {
        rigidBodiesCount = DEFAULT_RIGIDBODIES_COUNT;
        animatedBodiesCount = DEFAULT_ANIMATEDBODIES_COUNT;
        rigidParticleCount = DEFAULT_RIGIDPARTICLES_COUNT;

        controllersCount = DEFAULT_CONTROLLERS_COUNT;

        overlappedPairsCount = DEFAULT_OVERLAPPED_PAIRS_COUNT;

        geometriesCount = DEFAULT_GEOMETRIES_COUNT;

        constraintsCount = DEFAULT_CONSTRAINTS_COUNT;
        constraintSetsCount = DEFAULT_CONTRAINT_SETS_COUNT;
        constraintBufferSize = DEFAULT_SOLVER_BUFFER_SIZE;
        sensorsCount = DEFAULT_SENSORS_COUNT;

        terrainNodesStartCount = DEFAULT_TERRAIN_NODES_START_COUNT;
        terrainNodesGrowByCount = DEFAULT_TERRAIN_NODES_GROWBY_COUNT;   
                                        /* -1 signify double the number of terrainNode, whenever the 
                                           it reach full capacity.
                                        */
    }
};

/****************************************************************************
*
*   Tokamak Game Physics SDK 
*
*   Class: neSimulator
*
*   Desc:
*
****************************************************************************/ 
typedef struct neCollisionInfo neCollisionInfo;

struct neCollisionInfo
{
    neByte * bodyA;
    neByte * bodyB;
    neBodyType typeA;
    neBodyType typeB;
    neGeometry * geometryA;
    neGeometry * geometryB;
    s32 materialIdA;
    s32 materialIdB;
    neV3 bodyContactPointA;     // contact point A in body space of A
    neV3 bodyContactPointB;     // contact point B in body space of B
    neV3 worldContactPointA;    // contact point A in world space
    neV3 worldContactPointB;    // contact point B in world space
    neV3 relativeVelocity;
    neV3 collisionNormal;
};

typedef void (neLogOutputCallback)(char * logString);

typedef void (neCollisionCallback)(neCollisionInfo & collisionInfo);

typedef void (neTerrainTriangleQueryCallback)(const neV3 & minBound, const neV3 & maxBound, 
                                              s32 ** candidateTriangles,
                                                neTriangle ** triangles,
                                                neV3 ** vertices,
                                                s32 * candidateCount,
                                                s32 * triangleCount,
                                                neRigidBody * body);

typedef struct neCustomCDInfo neCustomCDInfo;

struct neCustomCDInfo
{
    neV3 collisionNormal;
    neV3 worldContactPointA;
    neV3 worldContactPointB;
    f32 penetrationDepth;
    s32 materialIdA;
    s32 materialIdB;
};

typedef neBool (neCustomCDRB2RBCallback)(neRigidBody * bodyA, neRigidBody * bodyB, neCustomCDInfo & cdInfo);

typedef neBool (neCustomCDRB2ABCallback)(neRigidBody * bodyA, neAnimatedBody * bodyB, neCustomCDInfo & cdInfo);

class TOKAMAK_API neSimulator
{
NE_INTERFACE(neSimulator)

public:
    typedef enum
    {
        LOG_OUTPUT_LEVEL_NONE = 0,
        LOG_OUTPUT_LEVEL_ONE,
        LOG_OUTPUT_LEVEL_FULL,
    } LOG_OUTPUT_LEVEL;
    
public:
    /* 
        Static factory functions  
    */

    static neSimulator * CreateSimulator(const neSimulatorSizeInfo & sizeInfo, 
                                        neAllocatorAbstract * alloc = NULL, 
                                        const neV3 * gravity = NULL);
    
    static void DestroySimulator(neSimulator * sim);
    
    /* 
        Rigid body managment functions 
    */

    neRigidBody * CreateRigidBody();

    neRigidBody * CreateRigidParticle();
    
    neAnimatedBody * CreateAnimatedBody();
    
    void FreeRigidBody(neRigidBody * body);

    void FreeAnimatedBody(neAnimatedBody * body);

    neCollisionTable * GetCollisionTable();

    /*
        Material managment functions
    */

    bool SetMaterial(s32 index, f32 friction, f32 restitution);

    bool GetMaterial(s32 index, f32& friction, f32& restitution);

    /*
        Advancing the simulation
    */

    void Advance(f32 sec, s32 nSteps = 1, nePerformanceReport * perfReport = NULL);

    void Advance(f32 sec, f32 minTimeStep, f32 maxTimeStep, nePerformanceReport * perfReport = NULL);

    /*
        Terrain setup function
    */

    void SetTerrainMesh(neTriangleMesh * tris);

    void FreeTerrainMesh();

    /*
        Constraint related
    */

    neJoint * CreateJoint(neRigidBody * bodyA);

    neJoint * CreateJoint(neRigidBody * bodyA, neRigidBody * bodyB);

    neJoint * CreateJoint(neRigidBody * bodyA, neAnimatedBody * bodyB);

    void FreeJoint(neJoint * joint);

    /*
        Others
    */
    neV3 Gravity();

    void Gravity(const neV3 & gravity);

    void SetBreakageCallback(neBreakageCallback * cb);

    neBreakageCallback * GetBreakageCallback();

    void SetCollisionCallback(neCollisionCallback * cb);

    neCollisionCallback * GetCollisionCallback();

    void SetTerrainTriangleQueryCallback(neTerrainTriangleQueryCallback * cb);
        
    neTerrainTriangleQueryCallback * GetTerrainTriangleQueryCallback();

    void SetCustomCDRB2RBCallback(neCustomCDRB2RBCallback * cb);

    neCustomCDRB2RBCallback * GetCustomCDRB2RBCallback();

    void SetCustomCDRB2ABCallback(neCustomCDRB2ABCallback * cb);

    neCustomCDRB2ABCallback * GetCustomCDRB2ABCallback();

    void SetLogOutputCallback(neLogOutputCallback * cb);

    neLogOutputCallback * GetLogOutputCallback();

    void SetLogOutputLevel(LOG_OUTPUT_LEVEL lvl = LOG_OUTPUT_LEVEL_FULL);

    neSimulatorSizeInfo GetCurrentSizeInfo();

    neSimulatorSizeInfo GetStartSizeInfo();

    void GetMemoryAllocated(s32 & memoryAllocated);
};


/****************************************************************************
*
*   Tokamak Game Physics SDK 
*
*   Misc. helper functions
*
*   
*
****************************************************************************/ 

neV3 TOKAMAK_API neBoxInertiaTensor(f32 width, f32 height, f32 depth, f32 mass);

neV3 TOKAMAK_API neBoxInertiaTensor(const neV3 & boxSize, f32 mass);

neV3 TOKAMAK_API neSphereInertiaTensor(f32 diameter, f32 mass);

neV3 TOKAMAK_API neCylinderInertiaTensor(f32 diameter, f32 height, f32 mass);

#endif//TOKAMAK_H


#ifdef TOKAMAK_IMPLEMENTATION
#pragma once

#include <stdint.h>

/**containers.h**/

#ifndef CONTAINERS_H
#define CONTAINERS_H

#define _CRT_SECURE_DEPRECATE_MEMORY
#include <memory.h>

#define PLACEMENT_MAGIC \
    public:\
    NEINLINE void * operator new(size_t s,void * addr)\
    {\
        return addr;\
    }\
    NEINLINE void * operator new[] (size_t s,void * addr)\
    {\
        return addr;\
    }\
    NEINLINE void operator delete(void *)\
    {\
    }\
    NEINLINE void operator delete[](void *)\
    {\
    }\
    NEINLINE void operator delete(void *, void *)\
    {\
    }\
    NEINLINE void operator delete[](void *, void *)\
    {\
    }

template <class T, int initFixedSize = 1> class neSimpleArray
{
public:
    PLACEMENT_MAGIC

    NEINLINE bool IsFixedSize()
    {
        return (initFixedSize != 1);
    }

    NEINLINE neSimpleArray()
    {
        if (IsFixedSize())
        {
            data = initArray;
            nextFree = data;
            size = initFixedSize;
            alloc = NULL;
            growBy = 0;
            usedSize = 0;
        }
        else
        {
            data = NULL;
            nextFree = NULL;
            size = 0;
            alloc = &allocDef;
            growBy = 0;
            usedSize = 0;
        }
    }
    NEINLINE ~neSimpleArray()
    {
        Free();
    }
    NEINLINE bool Reserve(s32 n, neAllocatorAbstract * al = NULL, s32 _growBy = 0) 
    {
        if (IsFixedSize())
        {
            ASSERT(0);
            return false;
        }
        Free();

        if (al)
            alloc = al;
        else
            alloc = & allocDef;
        
        growBy = _growBy;

        neByte * mem = alloc->Alloc(sizeof(T) * n);

        data = (T*) mem;

        if (data)
        {
            nextFree = data;
            size = n;
            return true;
        }
        return false;
    };

    NEINLINE T * Alloc(s32 dummy = 0) 
    {
        if (nextFree >= (data + size))
        {
            if (growBy == 0)
                return NULL;
            
            T * oldData = data;

            if (growBy == -1)
                data = (T*)alloc->Alloc((size * 2) * sizeof(T));
            else
                data = (T*)alloc->Alloc((size + growBy) * sizeof(T));

            if (!data)
            {
                data = oldData;

                return NULL;
            }

            memcpy(data, oldData, size * sizeof(T));

            if (oldData)
                alloc->Free((neByte*)oldData);

            nextFree = data + size;

            if (growBy == -1)
                size *= 2;
            else
                size += growBy;

            usedSize++;
            return nextFree++;
        }
        else
        {
            usedSize++;
            return nextFree++;
        }
    }
    NEINLINE s32 GetIndex(T * c)
    {
        ASSERT(c >= data);
        ASSERT(c < nextFree);

        return (s32)(c - data);
    }
    NEINLINE s32 GetUsedCount(){
        return (nextFree - data);
    }
    NEINLINE s32 GetTotalSize(){
        return size;
    }
    NEINLINE void Free()
    {
        if (IsFixedSize())
        {
            return;
        }
        if (data)
        {
            alloc->Free((neByte*)data);
        }
        data = NULL;
        nextFree = NULL;
        size = 0;
        usedSize = 0;
    }
    NEINLINE void Clear()
    {
        nextFree = data;
        usedSize = 0;
    }
    NEINLINE T & operator [] (s32 index) {
        ASSERT(index >= 0);
        ASSERT(index < size);
        return data[index];
    }
    NEINLINE void MakeFromPointer(T * pdata, s32 makeSize)
    {
        data = pdata;
        size = makeSize;
        usedSize = size;
        nextFree = pdata + makeSize;
        alloc = NULL;
        growBy = 0;
    }
protected:
    T * data;
    T * nextFree;
    s32 size;
    s32 usedSize;
    neAllocatorAbstract * alloc;
    neAllocatorDefault allocDef;
    s32 growBy;
    T initArray[initFixedSize];
};

template <class T, int initFixedSize = 1> class neArray
{
PLACEMENT_MAGIC
public:
    NEINLINE bool IsFixedSize()
    {
        return (initFixedSize != 1);
    }
    NEINLINE neArray(){
        if (IsFixedSize())
        {
            data = initArray;
            nextFree = data;
            size = initFixedSize;
            alloc = NULL;
            growBy = 0;
        }
        else
        {
            data = NULL;
            nextFree = NULL;
            size = 0;
            alloc = &allocDef;
            growBy = 0;
        }
    }
    NEINLINE ~neArray()
    {
        Free();
    }
    NEINLINE bool Reserve(s32 n, neAllocatorAbstract * al = NULL, s32 _growBy = 0) 
    {
        if (IsFixedSize())
        {
            ASSERT(0);
            return false;
        }
        Free();

        if (al)
            alloc = al;
        else
            alloc = & allocDef;
        
        growBy = _growBy;

        neByte * mem = alloc->Alloc(sizeof(T) * n);

        data = (T*) mem;

        if (data)
        {
            nextFree = data;
            size = n;
            return true;
        }
        return false;
    };

    NEINLINE T * Alloc() 
    {
        if (nextFree >= (data + size))
        {
            if (growBy == 0)
                return NULL;
            
            T * oldData = data;

            if (growBy == -1)
                data = (T*)alloc->Alloc(sizeof(T) * (size * 2));
            else
                data = (T*)alloc->Alloc(sizeof(T) * (size + growBy));

            if (!data)
            {
                data = oldData;

                return NULL;
            }

            memcpy(data, oldData, size * sizeof(T));

            if (oldData)
                alloc->Free((neByte*)oldData);

            nextFree = data + size;

            if (growBy == -1)
                size *= 2;
            else
                size += growBy;

        }
        T * ret = new ((void*)(nextFree++)) T;
        
        return ret;
    }
    NEINLINE s32 GetIndex(T * c)
    {
        ASSERT(c >= data);
        ASSERT(c < nextFree);

        return (s32)(c - data);
    }
    NEINLINE s32 GetUsedCount(){
        return (nextFree - data);
    }
    NEINLINE s32 GetTotalSize(){
        return size;
    }
    NEINLINE void Free()
    {
        if (IsFixedSize())
        {
            return;
        }
        if (data)
        {
            //delete [] (data, (void*)data);

            alloc->Free((neByte*)data);
        }
        data = NULL;
        nextFree = NULL;
        size = 0;
    }
    NEINLINE void Clear()
    {
        nextFree = data;
    }
    NEINLINE T & operator [] (s32 index) {
        ASSERT(index >= 0);
        ASSERT(index < size);
        return data[index];
    }
    NEINLINE void MakeFromPointer(T * pdata, s32 makeSize)
    {
        data = pdata;
        size = makeSize;
        nextFree = pdata + makeSize;
        alloc = NULL;
        growBy = 0;
    }
protected:
    T * data;
    T * nextFree;
    s32 size;
    neAllocatorAbstract * alloc;
    neAllocatorDefault allocDef;
    s32 growBy;

    T initArray[initFixedSize];
};

/////////////////////////////////////////////////////////////

template <class T> class neFreeListItem
{
PLACEMENT_MAGIC
public:
/*
    NEINLINE void * operator new (size_t s,void * addr)
    {
        return addr;
    }
    NEINLINE void * operator new[] (size_t s,void * addr)
    {
        return addr;
    }
    NEINLINE void operator delete(void *, void *)
    {
    }
    NEINLINE void operator delete[](void *, void *)
    {
    }
*/  T thing;
    neFreeListItem * next;
    neFreeListItem * prev;
    bool state;

    NEINLINE neFreeListItem()
    {
        prev = NULL;
        next = NULL;
        state = false;
    }

    NEINLINE void Remove()
    {
        if (next != NULL)
        {
            next->prev = prev;
        }
        if (prev != NULL)
        {
            prev->next = next;
        }
        Solo();
    }
    NEINLINE void Insert(neFreeListItem * newItem)
    {
        newItem->next = this;
        newItem->prev = prev;
        if (prev)
        {
            prev->next = newItem;
        }
        prev = newItem;
    }
    NEINLINE void Append(neFreeListItem * newItem)
    {
        newItem->next = next;
        newItem->prev = this;
        if (next)
        {
            next->prev = newItem;
        }
        next = newItem;
    }
    NEINLINE void Concat(neFreeListItem * newItem)
    {
        ASSERT(next == NULL);

        next = newItem;

        newItem->prev = this;
    }
    NEINLINE void Solo()
    {
        prev = NULL;
        next = NULL;
    }
};

/////////////////////////////////////////////////////////////

template <class T, int initFixedSize = 1> class neDLinkList
{
public:
    typedef neFreeListItem<T> listItem;

    NEINLINE bool IsFixedSize()
    {
        return (initFixedSize != 1);
    }
    NEINLINE neBool CheckBelongAndInUse(T * t)
    {
        listItem * item = (listItem *)t;

        if (item < data)
            return false;

        if (item >= (data + size))
            return false;

        return item->state; //1 = in use
    }
    NEINLINE void Init()
    {
        if (!IsFixedSize())
        {
            data = NULL;
            unused = NULL;
            used = NULL;
            unusedTail = NULL;
            usedTail = NULL;
            size = 0;
            usedCount = 0;
            unusedCount = 0;
        }
        else
        {
            data = initArray;
            size = initFixedSize;
            for (int i = 0; i < size; i++)
            {
                data[i].next = &(data[i+1]);
                data[i].prev = &(data[i-1]);
                data[i].state = false;
            }
            data[0].prev = NULL;
            data[size-1].next = NULL;

            unused = data;
            unusedTail = data + size;
            unusedCount = size;

            used = NULL;
            usedTail = NULL;
            usedCount = 0;
        }
    }
    NEINLINE neDLinkList()
    {
        alloc = &allocDef;

        Init();
    }
    NEINLINE void Free()
    {
        if (IsFixedSize())
            return;

        //delete [] (data, (void*) data);

        if (data)
            alloc->Free((neByte*)data-mallocNewDiff);

        Init();
    }
    NEINLINE s32 Size()
    {
        return size;
    }
    NEINLINE ~neDLinkList()
    {
        Free();
    }
    NEINLINE T * Alloc(s32 flag = 0)
    {
        if (!unused)
            return NULL;
        
        T * ret = &(unused->thing);

        ASSERT(unused->state == false);

        unused->state = true;

        listItem * newUnusedHead;

        newUnusedHead = unused->next;
        
        unused->Remove();

        if (flag == 0)
        {
            if (usedTail)
            {
                usedTail->Append(unused);
                usedTail = unused;
            }
            else
            {
                used = unused;
                used->Solo();
                usedTail = used;
            }
        }
        else
        {
            unused->Solo();
        }

        if (unused == unusedTail)
        {
            unusedTail = NULL;
            unused = NULL;;
            ASSERT(newUnusedHead == NULL);
        }
        else
            unused = newUnusedHead;

        unusedCount--;
        usedCount++;
        return ret;
    }
    NEINLINE bool Reserve(s32 n, neAllocatorAbstract * al = NULL)
    {
        if (IsFixedSize())
        {
            ASSERT(0);
            return false;
        }
        Free();

        if (n == 0)
            return true;

        if (al)
            alloc = al;

        neByte * mem = alloc->Alloc(sizeof(listItem) * n + 4);

        data = new (mem) listItem[n];

        mallocNewDiff = (neByte*)data - mem;
        
        size = n;

        for (int i = 0; i < n; i++)
        {
            data[i].next = &(data[i+1]);
            data[i].prev = &(data[i-1]);
            data[i].state = false;
        }
        data[0].prev = NULL;
        data[n-1].next = NULL;

        unused = data;
        unusedTail = data + size;
        unusedCount = n;

        used = NULL;
        usedTail = NULL;
        usedCount = 0;

        return true;
    }
    NEINLINE void Dealloc(T * thing, s32 flag = 0)
    {
        if (!flag && !used)
        {
            ASSERT(0);
            return;
        }

        s32 n = GetID(thing);

        ASSERT(n >= 0);
        ASSERT(n < size);

        listItem * newUnused = &(data[n]);
        
        ASSERT(newUnused->state == true);

        newUnused->state = false;

        if (flag == 0)
        {
            if (newUnused == used) // dealloc head of used
            {
                used = newUnused->next;
            }
            if (newUnused == usedTail) // dealloc tail of used
            {
                usedTail = newUnused->prev;
            }
        }

        newUnused->Remove();

        if (unused)
        {
            unused->Insert(newUnused);
        }
        else
        {
            newUnused->Solo();
            unusedTail = newUnused;
        }
        unused = newUnused;
        
        unusedCount++;
        usedCount--;
    }
    NEINLINE s32 GetID(T * t)
    {
        return ((listItem*)t - data);
    }
    NEINLINE s32 GetUsedCount()
    {
        return usedCount;
    }
    class iterator;

    NEINLINE void Clear()
    {
        iterator iter;

        for (iter = BeginUsed(); iter.Valid();)
        {
            iterator next = BeginNext(iter);

            Dealloc(*iter);

            iter = next;
        }
    }
    NEINLINE iterator BeginUsed()
    {
        iterator iter;
        
        iter.cur = used;

        return iter;
    }
    NEINLINE iterator BeginUnused()
    {
        iterator iter;
        
        iter.cur = unused;

        return iter;
    }
    NEINLINE iterator BeginNext(const iterator & it)
    {
        iterator next;

        next.cur = it.cur->next;

        return next;
    }
    class iterator
    {
    public:
        NEINLINE T * operator * () const
        {
            if (cur)
                return &(cur->thing);
            return NULL;
        }
        NEINLINE bool operator ++ (int) 
        {
            if (cur)
            {
                cur = cur->next;
                return true;
            }
            return false;
        }
        NEINLINE bool operator -- () 
        {
            if (cur)
            {
                cur = cur->prev;
                return true;
            }
            return false;
        }
        NEINLINE bool Valid()
        {
            return (cur != NULL);
        }
    public:
        listItem * cur;
    };

public:
    listItem * data;
    listItem * unused;
    listItem * used;
    listItem * unusedTail;
    listItem * usedTail;
    s32 size;
    s32 unusedCount;
    s32 usedCount;
    neAllocatorAbstract * alloc;
    neAllocatorDefault allocDef;
    s32 mallocNewDiff;
    listItem initArray[initFixedSize];
};
/*
template <class T, int initFixedSize = 1> class neHeap
{
public:
    typedef neDLinkList<T*, initFixedSize> FreeList;
    
    NEINLINE IsFixedSize()
    {
        return (initFixedSize != 1);
    }
    NEINLINE neHeap()
    {
        Init();
    }
    NEINLINE void Init()
    {
        freeList.Init();

        if (IsFixedSize())
        {
            buffer = initArray;
            alloc = NULL;
        }
        else
        {
            buffer = NULL;
            alloc = &allocDef;
        }
    }
    NEINLINE ~neHeap()
    {
        Free();
    }
    NEINLINE bool Reserve(s32 n, neAllocatorAbstract * al = NULL)
    {
        if (IsFixedSize())
        {
            ASSERT(0);
            return false;
        }
        Free();

        if (!freeList.Reserve(n, al))
            return false;
        
        if (al)
            alloc = al;

        neByte * mem = alloc->Alloc(sizeof(T) * n + 4);

        buffer = new(mem) T[n];
    
        mallocNewDiff = (neByte*)buffer - mem;
        
        if (!buffer)
        {
            Free();
            return false;
        }
        FreeList::iterator it;
        
        int i = 0;
        for (it = freeList.BeginUnused(); it.Valid(); it++)
        {
            (**it) = &(buffer[i]);
            i++;
        }
        return true;
    }
    NEINLINE T * Alloc(s32 flag = 0)
    {
        T ** pt =  freeList.Alloc(flag);

        new (*pt) T;
        
        if (!pt)
            return NULL;
        else
            return *pt;
    }
    NEINLINE void Dealloc(T * t, s32 flag = 0)
    {
        s32 offset = GetID(t);

        FreeList::listItem * li = freeList.data + offset;

        freeList.Dealloc((T**)li, flag);
    }
    NEINLINE s32 GetID(T * t)
    {
        return (t - buffer);
    }
    NEINLINE neBool IsInUse(T * t)
    {
        s32 i = GetID(t);

        ASSERT(i >= 0 && i <freeList.size);

        return freeList.data[i].state;
    }
    NEINLINE void Free()
    {
        if (IsFixedSize())
            return;

        //delete [] (buffer, (void*)buffer);

        if (buffer)
            alloc->Free((neByte*)buffer-mallocNewDiff);

        freeList.Free();

        Init();
    }
    NEINLINE s32 GetUsedCount()
    {
        return freeList.usedCount;
    }
    NEINLINE s32 GetUnusedCount()
    {
        return freeList.unusedCount;
    }
    NEINLINE s32 Size()
    {
        return freeList.size;
    }
    class iterator;

    NEINLINE iterator BeginUsed()
    {
        iterator cur;

        cur.iter = freeList.BeginUsed();

        return cur;
    }
    NEINLINE iterator BeginUnused()
    {
        iterator cur;

        cur.iter = freeList.BeginUnused();

        return cur;
    }
    NEINLINE iterator BeginNext(const iterator & it)
    {
        iterator next;

        next.iter = freeList.BeginNext(it.iter);

        return next;
    }
    class iterator
    {
    public:
        FreeList::iterator iter;

        NEINLINE T * operator * () const
        {
            return (**iter);
        }
        NEINLINE bool operator ++ (int) 
        {
            return (iter++);
        }
        NEINLINE bool operator -- () 
        {
            return (iter--)
        }
        NEINLINE bool Valid()
        {
            return (iter.Valid());
        }
    };

protected:
    T * buffer;
    FreeList freeList;
    neAllocatorAbstract * alloc;
    neAllocatorDefault allocDef;
    s32 mallocNewDiff;

    T initArray[initFixedSize];
};
*/
template <class T> class neCollection
{
public:
    typedef neFreeListItem<T*> itemType;

    neCollection()
    {
        Reset();
    }
    void Reset()
    {
        headItem = NULL;
        tailItem = NULL;
        count = 0;
    }
    void Add(itemType * add)
    {
        ASSERT(add);
        
        if (headItem)
        {
            tailItem->Append(add);

            tailItem = add;
        }
        else
        {
            headItem = add;
            tailItem = add;
        }
        count++;
    }
    void Remove(itemType * rem)
    {
        ASSERT(count > 0);

        ASSERT(rem);

        if (rem == headItem)
        {
            headItem = rem->next;
        }
        if (rem == tailItem)
        {
            tailItem = rem->prev;
        }
        rem->Remove();

        count --;
    }
    itemType * GetHead()
    {
        return headItem;
    }
    itemType * GetNext(itemType * cur)
    {
        return cur->next;
    }
    itemType * GetPrev(itemType * cur)
    {
        return cur->prev;
    }

public:
    neFreeListItem<T*> * headItem;
    neFreeListItem<T*> * tailItem;
    s32 count;
};

template <class T> class neList
{
public:
    typedef neFreeListItem<T> itemType;

    neList()
    {
        Reset();
    }
    void Reset()
    {
        headItem = NULL;
        tailItem = NULL;
        count = 0;
    }
    void Add(T * add)
    {
        ASSERT(add);

        itemType * addItem = (itemType *)add;
        
        if (headItem)
        {
            tailItem->Append(addItem);

            tailItem = addItem;
        }
        else
        {
            headItem = addItem;
            tailItem = addItem;
        }
        count++;
    }
    void AddOrder(T * add)
    {
        ASSERT(add);

        itemType * addItem = (itemType *)add;

        if (!headItem)
        {
            headItem = addItem;

            tailItem = addItem;

            count = 1;

            return;
        }

        itemType * curItem = tailItem;

        neBool done = false;

        while (curItem)
        {
            T * curT = (T *)curItem;

            if (add->Value() <= curT->Value())
            {
                done = true;

                curItem->Append(addItem);

                if (curItem == tailItem)
                {
                    tailItem = addItem;
                }
                break;
            }
            curItem = curItem->prev;
        }
        if (!done)
        {
            headItem->Insert(addItem);

            headItem = addItem;
        }
        count++;
    }
    void UpdateOrder(T * u)
    {
        if (count == 1)
            return;

        itemType * uItem = (itemType *) u;

        itemType * cItem;

        neBool done = false;

        if (uItem == tailItem) // move up
        {
            cItem = uItem->prev;

            Remove(u);

            while (cItem)
            {
                if (((T*)cItem)->Value() >= u->Value())
                {
                    cItem->Append(uItem);

                    if (cItem == tailItem)
                    {
                        tailItem = uItem;
                    }
                    done = true;

                    break;
                }
                cItem = cItem->prev;
            }
            if (!done)
            {
                headItem->Insert(uItem);

                headItem = uItem;
            }
            count++; // because Remove dec count;
        }
        else if (uItem == headItem) // move down
        {
            cItem = uItem->next;

            Remove(u);

            while (cItem)
            {
                if (((T*)cItem)->Value() <= u->Value())
                {
                    cItem->Insert(uItem);

                    if (cItem == headItem)
                    {
                        headItem = uItem;
                    }
                    done = true;

                    break;
                }
                cItem = cItem->next;
            }
            if (!done)
            {
                tailItem->Append(uItem);

                tailItem = uItem;
            }
            count ++;
        }
        else
        {
            itemType * nextItem = uItem->next;

            T * nextT = (T*) nextItem;

            if (u->Value() < nextT->Value())
            {
                //move down
                cItem = nextItem;

                Remove(u);

                while (cItem)
                {
                    if (((T*)cItem)->Value() <= u->Value())
                    {
                        cItem->Insert(uItem);

                        done = true;

                        break;
                    }
                    cItem = cItem->next;
                }
                if (!done)
                {
                    tailItem->Append(uItem);

                    tailItem = uItem;
                }
                count ++;
            }
            else
            {
                //move up
                cItem = uItem->prev;

                Remove(u);

                while (cItem)
                {
                    if (((T*)cItem)->Value() >= u->Value())
                    {
                        cItem->Append(uItem);

                        if (cItem == tailItem)
                        {
                            tailItem = uItem;
                        }
                        done = true;

                        break;
                    }
                    cItem = cItem->prev;
                }
                if (!done)
                {
                    headItem->Insert(uItem);

                    headItem = uItem;
                }
                count++; // because Remove dec count;
            }
        }
    }
    void Remove(T * rem)
    {
        ASSERT(count > 0);

        ASSERT(rem);

        itemType * remItem = (itemType *)rem;

        if (remItem == headItem)
        {
            headItem = remItem->next;
        }
        if (remItem == tailItem)
        {
            tailItem = remItem->prev;
        }
        remItem->Remove();

        count --;
    }
    T * GetHead()
    {
        return(T*)headItem;
    }
    T * GetNext(T * cur)
    {
        return (T*)((itemType*)cur)->next;
    }
    T * GetPrev(T * cur)
    {
        return (T*)((itemType*)cur)->prev;
    }

public:
    itemType * headItem;
    itemType * tailItem;
    s32 count;
};

#endif //CONTAINERS_H

/**scenery.h**/

#ifndef NE_SCENERY_H
#define NE_SCENERY_H

class neTriangleTree;

class neFixedTimeStepSimulator;

/****************************************************************************
*
*   NE Physics Engine 
*
*   Class: neTriangle_
*
*   Desc:
*
****************************************************************************/ 

class neTriangle_: public neTriangle
{
PLACEMENT_MAGIC
public:
//  void * operator new (size_t s, void * addr) {
//      return addr;
//  }
//  void operator delete (void *) {}

};

/****************************************************************************
*
*   NE Physics Engine 
*
*   Class: neQuadTreeNode
*
*   Desc:
*
****************************************************************************/ 

//#define NE_TREE_DIM 2

//#define NE_TREE_SECTOR_COUNT  (2 * 2)

class neTreeNode
{
public:
    void Initialise(neTriangleTree * _tree, s32 _parent, const neV3 & minBound, const neV3 & maxBound);

    void Build(neSimpleArray<s32> & triIndex, s32 level);

    void CountTriangleInSector(neSimpleArray<s32> &tris, neSimpleArray<s32> &sectorTris, const neV3 & com, s32 i);

    s32 CountTriangleInSector2(neSimpleArray<s32> &tris, const neV3 & com, s32 sector);

    void MakeLeaf(neSimpleArray<s32> &tris);

    bool IsOverlapped(const neV3 & minBound, const neV3 & maxBound);

    void GetCandidateNodes(neSimpleArray<neTreeNode*> & nodes, const neV3 & minBound, const neV3 & maxBound, s32 level);

    void SelectBound(const neV3 & com, neV3 & minBound, neV3 & maxBound, s32 sector);

    void DrawTriangles();

    void DrawBounds();

public:
    static s32 numOfChildren;

    void * operator new (size_t t, void * addr){
        return addr;
    }
    void operator delete [] (void *, void *){}

    void operator delete (void *, void *){}

    neTreeNode();

    neTriangleTree * tree;

    s32 parent;

    s32 children[4];
    
    neV3 bounds[3];//min/max x,y,z

    neSimpleArray<s32> triangleIndices; //leaf only
};

/****************************************************************************
*
*   NE Physics Engine 
*
*   Class: neTree
*
*   Desc:
*
****************************************************************************/ 

class neTriangleTree
{
public: 
    neTriangleTree();

    ~neTriangleTree();

    neBool BuildTree(neV3 * vertices, s32 vertexCount, neTriangle * tris, s32 triCount, neAllocatorAbstract * _alloc);

    void FreeTree();

    neTreeNode & GetRoot(){ return root;}

    bool HasTerrain() {return nodes.GetUsedCount() > 0;};

    neTreeNode & GetNode(s32 nodeIndex);

public:
    neV3 * vertices;

    s32 vertexCount;

    neArray<neTriangle_> triangles;

    neArray<neTreeNode> nodes;

    neAllocatorAbstract * alloc;

    neAllocatorDefault allocDef;

    neTreeNode root;

    neFixedTimeStepSimulator * sim;
};

#endif //NE_SCENERY_H

/**collision.h**/

#ifndef COLLISION_H
#define COLLISION_H

typedef enum
{
    IMPULSE_IGNORE,
    IMPULSE_NORMAL,
    IMPULSE_CONTACT,
    IMPULSE_CONSTRAINT,
    IMPULSE_SLIDER,
    IMPULSE_SLIDER_LIMIT_PRIMARY,
//  IMPULSE_LIMIT,
    IMPULSE_ANGULAR_LIMIT_PRIMARY,
    IMPULSE_ANGULAR_LIMIT_SECONDARY,
    IMPULSE_ANGULAR_MOTOR_PRIMARY,
    IMPULSE_ANGULAR_MOTOR_SECONDARY,
    IMPULSE_RELATIVE_LINEAR_VELOCITY,
}neImpulseType;

class neRigidBodyBase;

class neRigidBody_;

///////////////////////////////////////////////////////////////////
//
//  Collision Model
//
//
///////////////////////////////////////////////////////////////////

typedef struct
{
    neV3    point[2];       // closest point in world space, but relative to object centre
    neV3    normal;         // toward feature on body A

    f32     distance;
    bool    penetrated;     
    
    int     matIndex[2];
}TCollisionResult;

///////////////////////////////////////////////////////////////////

//typedef struct neBox neBox; 

typedef struct _neBox
{
    neV3    boxSize; //half of dimensions
    //f32 boxSize[4];
}neBox;

struct neTri
{
    s32 indices[3];
};

struct neTriangleTerrain
{
    neSimpleArray<s32> * triIndex;
    neArray<neTriangle_> * triangles;
};

struct neSphere
{
    f32 radius;
    f32 radiusSq;
};

struct neCylinder
{
    f32 radius;
    f32 radiusSq;
    f32 halfHeight;
};

struct neConvexMesh
{
    neV3 * vertices;
    s32 * neighbours;
    s32 vertexCount;
};

struct neConvexDCD
{
    neByte * convexData;
    neV3 * vertices;
    s32 numVerts;
};

#ifdef USE_OPCODE

struct neOPCMesh
{
    Opcode::OPCODE_Model * opmodel;
    IceMaths::Point * vertices;
    u32 vertCount;
    IndexedTriangle * triIndices;
    u32 triCount;
};

#endif

typedef struct neBreakInfo neBreakInfo;

struct neBreakInfo
{
    neV3 inertiaTensor;
    neV3 breakPlane;
    f32 mass;
    f32 breakMagnitude;
    f32 breakAbsorb;
    f32 neighbourRadius;
    neGeometry::neBreakFlag flag; //break all,
};

typedef struct TConvex TConvex;

struct TConvex
{
    enum
    {
        POINT,
        LINE,
        TRIANGLE,
        BOX,
        SPHERE,
        CYLINDER,
        TERRAIN,
        CONVEXITY,
        CONVEXDCD,
        OPCODE_MESH,
    };

    union
    {
        neBox box;
        neTri tri;
        neTriangleTerrain terrain;
        neSphere sphere;
        neCylinder cylinder;
        neConvexMesh convexMesh;
        neConvexDCD convexDCD;
#ifdef USE_OPCODE
        neOPCMesh opcodeMesh;
#endif
    }as;

    neT3    c2p;    // convex to physics object
    f32     boundingRadius;
    f32     envelope;
    u32     type;
    s32     matIndex;
    uintptr_t       userData;
    neBreakInfo breakInfo;
    neV3 *  vertices;

    void    SetBoxSize(f32 width, f32 height, f32 depth);
    void    SetSphere(f32 radius);
    void    SetTriangle(s32 a, s32 b, s32 c, neV3 * vertices);
    void    SetTerrain(neSimpleArray<s32> & triangleIndex, neArray<neTriangle_> & triangles, neV3 * vertices);
    void    SetConvexMesh(neByte * convexData);

#ifdef USE_OPCODE

    void    SetOpcodeMesh(IndexedTriangle * triIndex, u32 triCount, IceMaths::Point * vertArray, u32 vertCount);

#endif

    void    SetTransform(neT3 & t3);
    neT3    GetTransform();
    void    SetUserData(uintptr_t ud)
    {
            userData = ud;
    }
    uintptr_t       GetUserData()
    {
            return userData;
    }
    void    SetMaterialId(s32 id);
    s32     GetMaterialId();
    f32     GetBoundRadius();
    u32     GetType();
    void    Initialise();
    neM3    CalcInertiaTensor(f32 density, f32 & mass);
    void    GetExtend(neV3 & minExt, neV3 & maxEnt);

    //quick access functions
    NEINLINE f32 BoxSize(s32 dir)
    {
        ASSERT(type == BOX);
        
        return as.box.boxSize[dir];
    }
    NEINLINE f32 Radius()
    {
        ASSERT(type == SPHERE);
        
        return as.sphere.radius;
    }
    NEINLINE f32 RadiusSq()
    {
        ASSERT(type == SPHERE);
        
        return as.sphere.radiusSq;
    }
    NEINLINE f32 CylinderRadius()
    {
        ASSERT(type == CYLINDER);
        
        return as.cylinder.radius;
    }
    NEINLINE f32 CylinderRadiusSq()
    {
        ASSERT(type == CYLINDER);
        
        return as.cylinder.radiusSq;
    }
    NEINLINE f32 CylinderHalfHeight()
    {
        ASSERT(type == CYLINDER);
        
        return as.cylinder.halfHeight;
    }
};

class neSensor_
{
public:
    neV3 pos;

    neV3 dir;

    neV3 dirNormal;

    f32 length;

    uintptr_t cookies;

    //results
    
    neV3 normal;

    neV3 contactPoint;

    f32 depth;

    s32 materialID;

    neRigidBodyBase * body;

public:

    neSensor_()
    {
        pos.SetZero();
        dir.SetZero();
        cookies = 0;
        normal.SetZero();
        depth = 0;
        materialID = 0;
        body = NULL;
    }
};
/****************************************************************************
*
*   NE Physics Engine 
*
*   Class: neCollision
*
*   Desc:
*
****************************************************************************/ 

typedef neFreeListItem<TConvex> TConvexItem;

class neCollision
{
public:
    neCollision()
    {
        convex = NULL;
        convexCount = 0;
        boundingRadius = 0.0f;
        obb.SetBoxSize(1.0f, 1.0f, 1.0f);
    }

//  void        SeTConvex(TConvex * con, s32 count)
//  {
//      convex = con;
//      convexCount = count+1;
//  }

    void        CalcBB();

public:
    TConvex     obb;
    TConvex *   convex;
    s32         convexCount;
    f32         boundingRadius;
};

class neCollisionResult;

typedef neCollection<neCollisionResult>::itemType neCollisionResultHandle;

class neCollisionResult
{
PLACEMENT_MAGIC
public:
    //neCollisionResultHandle bodyAHandle;
    //neCollisionResultHandle bodyBHandle;

    neV3 contactA;
    neV3 contactB;

    neV3 contactABody; // or relative tangential velocity
    neV3 contactBBody; // or angular limit axis

    neV3 contactAWorld;
    neV3 contactBWorld;

    neM3 collisionFrame;

    neM3 w2c;

    neV3 initRelVelWorld;
    neV3 initRelVel;
    neV3 finaltRelVel;
    
    s32  materialIdA;
    s32  materialIdB;
    f32  depth; //+ve
    neBool penetrate;

    neRigidBodyBase * bodyA;
    neRigidBodyBase * bodyB;

    f32 relativeSpeed;
    f32 finalRelativeSpeed;

    TConvex * convexA;
    TConvex * convexB;

    neM3 k;

    neM3 kInv;

    f32 impulseScale;

    neImpulseType impulseType;

    neBool flag;

    void UpdateConstraintRelativeSpeed();
    void StartStage2();
    void PrepareForSolver(neBool aIdle = false, neBool bIdle = false);
    void CalcCollisionMatrix(neRigidBody_* ba, neRigidBody_ * bb, neBool isWorld);
    void CalcCollisionMatrix2(neRigidBody_* ba, neRigidBody_ * bb);
    void CalcCollisionMatrix3(neRigidBody_* ba, neRigidBody_ * bb);
    
    f32 SolveContact(neFixedTimeStepSimulator * sim);
    f32 SolveConstraint(neFixedTimeStepSimulator * sim);
    f32 SolveSlider(neFixedTimeStepSimulator * sim);
    f32 SolveSliderLimit(neFixedTimeStepSimulator * sim);
    f32 SolveAngularPrimary(neFixedTimeStepSimulator * sim);
    f32 SolveAngularSecondary(neFixedTimeStepSimulator * sim);
    f32 SolveAngularMotorPrimary(neFixedTimeStepSimulator * sim);
    f32 SolveRelativeLinear(neFixedTimeStepSimulator * sim);
    
    f32 SolveAngular(f32 depth, const neV3 & axis, f32 relAV, neFixedTimeStepSimulator * sim);
    f32 SolveAngular2(const neV3 & axisA, const neV3 & axisB, f32 relAV, f32 desireAV, f32 depth, neFixedTimeStepSimulator * sim);
    f32 SolveAngular3(f32 depth, const neV3 & axis, f32 relAV, neFixedTimeStepSimulator * sim);
    
    void CalcError(neFixedTimeStepSimulator * sim);
//  void AddToBodies();
    
    f32 Value(){
        return relativeSpeed;};

    neBool CheckIdle();

    void Swap()
    {
        collisionFrame[2] *=  -1.0f;

        neSwap(contactA, contactB);

        neSwap(convexA, convexB);
    }
};

void CollisionTest(neCollisionResult & result, neCollision & colA, neT3 & transA, neCollision & colB, neT3 & transB, const neV3 & backupVector);

void CollisionTestSensor(TConvex * obbA, neSensor_ * sensorsA, neT3 & transA, neCollision & colB, neT3 & transB, neRigidBodyBase * body);

void ConvexCollisionTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB, const neV3 & backupVector);

void Box2BoxTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB, const neV3 & backupVector);

void Box2TriangleTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB);

void Box2TerrainTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB);

void Box2SphereTest(neCollisionResult & result, TConvex & boxA, neT3 & transA, TConvex & sphereB, neT3 & transB);

void Box2CylinderTest(neCollisionResult & result, TConvex & boxA, neT3 & transA, TConvex & sphereB, neT3 & transB);

void Sphere2TerrainTest(neCollisionResult & result, TConvex & sphereA, neT3 & transA, TConvex & terrainB);

void Sphere2SphereTest(neCollisionResult & result, TConvex & sphereA, neT3 & transA, TConvex & sphereB, neT3 & transB);

void Cylinder2CylinderTest(neCollisionResult & result, TConvex & cA, neT3 & transA, TConvex & cB, neT3 & transB);

void Cylinder2TerrainTest(neCollisionResult & result, TConvex & cylinderA, neT3 & transA, TConvex & terrainB);

void Cylinder2SphereTest(neCollisionResult & result, TConvex & cylinderA, neT3 & transA, TConvex & sphereB, neT3 & transB);

void Box2ConvexTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB, const neV3 & backupVector);

void Convex2ConvexTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB, const neV3 & backupVector);

void TranslateCOM(neM3 & I, neV3 &translate, f32 mass, f32 factor);

void DiagonalizeMassTensor(neM3 & I, neV3 & diagonal, neM3 & eigenVectors);

void SensorTest(neSensor_ & sensorA, TConvex & convexB, neT3 & transB);

#ifdef USE_OPCODE

void Box2OpcodeTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB);

void Sphere2OpcodeTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB);

void Cylinder2OpcodeTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB);

void Opcode2TerrainTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB);

void Opcode2OpcodeTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB);

#endif //USE_OPCODE

#endif

/**collision2.h**/

class TriangleParam
{
public:
    neV3 vert[3];
    neV3 edges[3];
    neV3 normal;
    f32 d;

    //extra info for cylinder

//  neV3 edgeUnits[3];
    neV3 edgeNormals[3]; // the normal for the edges
    neV3 vertNormals[3];

    void ConputeExtraInfo();

    void Transform(const TriangleParam & from, neT3 & trans);

    NEINLINE bool PointInYProjection(neV3 & point);

    s32 IsPointInside(const neV3 & point);
};

typedef struct ConvexTestResult ConvexTestResult;

struct ConvexTestResult
{
    neV3 contactA;
    neV3 contactB;
    neV3 contactNormal;
    neV3 contactX;
    neV3 contactY;
    neV3 edgeA[2];
    neV3 edgeB[2];
    f32 depth;
    bool valid;
    bool isEdgeEdge;
    bool needTransform;
    neBool ComputerEdgeContactPoint(ConvexTestResult & res);
    neBool ComputerEdgeContactPoint2(f32 & au, f32 & bu);
    void Reverse()
    {
        neSwap(contactA, contactB);
        contactNormal *= -1.0f;
    }
};

class BoxTestParam
{
public:
    BoxTestParam()
    {
        isVertCalc = false;
    }
    TConvex * convex;
    neT3 * trans;
    neM3 radii;
    
    bool isVertCalc;
    neV3 verts[8];

    void CalcVertInWorld();
    bool BoxTest(ConvexTestResult & result, BoxTestParam & otherBox);
    bool MeasureVertexFacePeneration(ConvexTestResult & result, BoxTestParam & otherBox, s32 whichFace);
    neBool MeasureEdgePeneration(ConvexTestResult & result, BoxTestParam & otherBox, s32 dim1, s32 dim2);
    bool TriTest(ConvexTestResult & result, TriangleParam & tri);
    bool TriHeightTest(ConvexTestResult & result, TriangleParam & tri);
    NEINLINE bool MeasurePlanePenetration(ConvexTestResult & result, const neV3 & normal, f32 d);
    bool MeasureBoxFaceTrianglePenetration(ConvexTestResult & result, TriangleParam & tri, s32 whichFace);
    bool MeasureBoxEdgeTriangleEdgePenetration(ConvexTestResult & result, TriangleParam & tri, s32 dim1, s32 dim2);

    //cylinder functions

    //neBool CylinderEndVertexTest(ConvexTestResult & res, TConvex & cylinder);

    //neBool CylinderRimFaceTest(ConvexTestResult & res, TConvex & cylinder, s32 whichFace);

    neBool CylinderFaceTest(ConvexTestResult & res, TConvex & cylinderB, neT3 & transB, s32 whichFace);

    //neBool CylinderEdgeTest(ConvexTestResult & res, TConvex & cylinder, s32 whichEdge);

    neBool CylinderEdgeTest(ConvexTestResult & res, TConvex & cylinder, neT3 & transB, s32 whichEdge);

    neBool LineTest(ConvexTestResult & res, neV3 & point1, neV3 & point2);
};

void ChooseAxis(neV3 & x, neV3 & y, const neV3 & normal);

neBool SphereTriTest(const neV3 & center, f32 radius, ConvexTestResult & result, TriangleParam & tri);

neBool CylinderTriTest(TConvex & sphere, neT3 & trans, ConvexTestResult & result, TriangleParam & tri);

neBool CylinderTriTest_Line(TConvex & cylinder, neT3 & trans, ConvexTestResult & result, neV3 & point1, neV3 & point2);

/**constraint.h**/

class _neConstraint;
class neRigidBodyBase;
class neRigidBody_;
class neJointSolver;
class neFixedTimeStepSimulator;

/****************************************************************************
*
*   NE Physics Engine 
*
*   Class: neJointPoint
*
*   Desc:
*
****************************************************************************/ 

class neJointPoint
{
public:
    neV3 points[2];

    neV3 & PtBody() {return points[0];}
    neV3 & PtWorld() {return points[1];}
};


/****************************************************************************
*
*   NE Physics Engine 
*
*   Class: _neConstraint
*
*   Desc:
*
****************************************************************************/ 
class neController;

typedef neFreeListItem<neController> neControllerItem;

typedef neCollection<_neConstraint>::itemType neConstraintHandle;

typedef struct neLimitState neLimitState;

struct neLimitState
{
    s32 limitType;

    neBool applyLimitImpulse;

    neV3 limitAxis;

    neV3 limitNormalA;

    neV3 limitNormalB;

    neBool lowerLimitOn; // false means upper limit

    neBool enableLimit;

    f32 limitAngularPenetration;

    f32 limitAngularPenetration2;

    f32 upperLimit;

    f32 lowerLimit;

    _neConstraint * constr;

    void Reset(_neConstraint * c, s32 ltype = 0)
    {
        constr = c;
        
        applyLimitImpulse = false;
        
        limitType = ltype;

        enableLimit = false;

        upperLimit = 0.0f;

        lowerLimit = 0.0f;
    }

    void CheckLimitPrimary();

    void CheckLimitSecondary();

    void CheckLimitPrimarySlider();
};

struct neMotor
{
    neBool enable;

    neJoint::MotorType motorType;

    f32 desireVelocity;

    f32 maxForce;

    neV3 axis;//for ball joint

    void Reset()
    {
        enable = false;

        motorType = neJoint::NE_MOTOR_SPEED;
        
        desireVelocity = 0.0f;

        maxForce = 0.0f;

        axis.Set(1.0f, 0.0f, 0.0f);
    }

    void PrepareForSolver(_neConstraint * constraint);
};

class _neConstraint
{
public:
    enum
    {
        NE_CPOINT_TYPE_BODY,
        NE_CPOINT_TYPE_WORLD,
        NE_CPOINT_TYPE_RESULT,
    };
    neT3 frameA;
    neT3 frameB;

    neT3 frameAWorld;
    neT3 frameBWorld;

    neFixedTimeStepSimulator * sim;

    neController * controllers;

    neControllerItem * controllerCursor;

    neRigidBody_ * bodyA;

    neRigidBodyBase * bodyB;

    neConstraintHandle bodyAHandle;
    neConstraintHandle bodyBHandle;

    s32 pointCount;
    
    neJointPoint cpointsA[2];
    neJointPoint cpointsB[2];
/*  
    neV3 cpointResults[2][2];

    f32 clength[2];
    f32 clengthSq[2];
*/
    neBool enable;

    neBool infiniteMassB;

    f32 accuracy;

    s32 iteration;

    f32 jointLength;

    f32 pos;

    f32 pos2;

    f32 jointDampingFactor;

    neBool alreadySetup;
    /*
    
      apply limit

    */
    neLimitState limitStates[2];

    neMotor motors[2];

    neJoint::ConstraintType type;

    void Enable(neBool yes);

    void GeneratePointsFromFrame();
    
    void Reset();

    void SetType(neJoint::ConstraintType t);

//  neV3 * GetPoint(neRigidBodyBase * body, s32 index, s32 ptType);

//  void ChooseRigidConstraints();

    void UpdateConstraintPoint();

//  f32 ApplyConstraintImpulse(neFixedTimeStepSimulator & sim);

    void InfiniteMassB(neBool yes);

    void AddToRigidBody();

    neController * AddController(neJointControllerCallback * jc, s32 period);

    void BeginIterateController();

    neController * GetNextController();

    void UpdateController();

    void FindGreatest();

    neRigidBodyBase * GetNotThisBody(neRigidBody_ * notThis)
    {
        if (bodyA == notThis)
        {
            return bodyB;
        }
        else
        {
            return (neRigidBodyBase *)bodyA;
        }
    }

    void CheckLimit();

    void DrawCPointLine();

    neT3 GetFrameBWorld();

    neT3 GetBodyB2W();

    void SetupLimitCollision();

    void UpdateCurrentPosition();

    void ApplyDamping();

//  void SolveAngularConstraint();
};

typedef neFreeListItem<neRigidBodyBase *> neCBodyItem;

class neConstraintHeader
{
public:
    enum
    {
        FLAG_NONE = 0,
        FLAG_NEED_SETUP = 1,
        FLAG_NEED_REORG = 2,
    };

    _neConstraint * head;
    _neConstraint * tail;
    
    neBool solved;

    s32 flag;

    neCollection<neRigidBodyBase> bodies;

    neConstraintHeader() { Reset();}

    void AddToSolver(f32 & epsilon, s32 & iteration);

    void Reset() 
    {
        head = NULL; 
        
        tail = NULL;

        flag = FLAG_NEED_SETUP;

        solved = false;

        bodies.Reset();
    }
    void Add(_neConstraint * c)
    {
        if (tail)
        {
            ((neFreeListItem<_neConstraint> *)tail)->Append((neFreeListItem<_neConstraint>*)c);

            tail = c;
        }
        else
        {
            head = tail = c;
            
            ((neFreeListItem<_neConstraint> *)c)->Solo();
        }
    }

    void Remove(_neConstraint * c)
    {
        neFreeListItem<_neConstraint> * item = (neFreeListItem<_neConstraint> *) c;

        if (c == head)
        {
            head = (_neConstraint *)item->next;
        }
        if (c == tail)
        {
            tail = (_neConstraint *)item->prev;
        }
        item->Remove();
    }
    //void Purge(neFixedTimeStepSimulator * sim);

    void TraverseApplyConstraint(neBool autoSleep);

    neBool StationaryCheck();

    void BecomeIdle(neBool checkResting = false);

    void WakeUp();

    void RemoveAll();
};

/**dcd.h**/

#ifndef DCD_H
#define DCD_H

struct DCDFace
{
    neByte *neighbourFaces;
    neByte *neighbourVerts;
    neByte *neighbourEdges;
};

struct DCDVert
{
    neByte * neighbourEdges;
};

struct DCDEdge
{
    neByte f1;
    neByte f2;
    neByte v1;
    neByte v2;
};

struct DCDMesh
{
    s32 numFaces;
    s32 numVerts;
    s32 numEdges;
    neByte pad0;

    s32 numNeighbour;

    neV3 * normals;
    neV3 * vertices;

    DCDFace * faces;
    DCDVert * verts;
    DCDEdge * edges;

    void SetConvex(const TConvex & convex, neV3 * vertArray);
    neV3 GetVertOnFace(s32 faceIndex, s32 vertIndex);
    neV3 GetVert(s32 vertIndex);
    neV3  GetNormal(s32 faceIndex);
//  neByte FaceGetNumFaceNeighbour(s32 faceIndex);
    neByte FaceGetFaceNeighbour(s32 faceIndex, s32 neighbourIndex);
    neByte FaceGetEdgeNeighbour(s32 faceIndex, s32 neighbourIndex);
//  neByte VertGetNumEdgeNeighbour(s32 vertIndex);
    neByte VertGetEdgeNeighbour(s32 vertIndex, s32 neighbourIndex);
    neByte EdgeGetVert1(s32 edgeIndex);
    neByte EdgeGetVert2(s32 edgeIndex);
};

bool TestDCD(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB, const neV3 & backupVector);

void Convex2TerrainTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB);

#endif

/**message.h**/

#ifndef NE_MESSAGE_H
#define NE_MESSAGE_H

#define MSG_MEMORY_ALLOC_FAILED     "Failed to allocate memory.\n"
#define MSG_RUN_OUT_GEOMETRY        "Run out of Geometries. Increase 'geometriesCount'.\n"
#define MSG_RUN_OUT_SENSOR          "Run out of Sensors. Increase 'sensorsCount'.\n"
#define MSG_RUN_OUT_RIDIGBODY       "Run out of RigidBodies. Increase 'rigidBodiesCount'.\n"
#define MSG_RUN_OUT_RIDIGPARTICLE   "Run out of RigidBodies. Increase 'rigidParticleCount'.\n"
#define MSG_RUN_OUT_ANIMATEDBODY    "Run out of AnimatedBodies. Increase 'animatedBodiesCount'.\n"

#define MSG_CONTROLLER_FULL         "Run out of Controllers. Increase 'constraintSetsCount'.\n"
#define MSG_CONSTRAINT_FULL         "Run out of Constraints. Increase 'constraintsCount'.\n"
#define MSG_CONSTRAINT_HEADER_FULL  "Run out of Constraint Sets. Increase 'constraintSetsCount'.\n"
#define MSG_CONSTRAINT_BUFFER_FULL  "Run out of Constraint Buffer. Cannot solve all constraints. Increase constraintBufferSize.\n"
#define MSG_TOO_MANY_CONSTRAINT     "Rigid Body contain too many constraints.\n"
#define MSG_STACK_BUFFER_FULL       "Stacking Buffer full Error. Please contact techincal support\n"

#define MSG_TRYING_TO_FREE_INVALID_RB   "Trying to Free invalid RigidBody.\n"
#define MSG_TRYING_TO_FREE_INVALID_RP   "Trying to Free invalid RigidParticle.\n"
#define MSG_TRYING_TO_FREE_INVALID_CB   "Trying to Free invalid AnimatedBody.\n"

#endif

/**perf.h**/
class nePerformanceData
{
public:
    static nePerformanceData * Create();

    nePerformanceData()
    {
        Reset();
    }
    void Reset()
    {
        dynamic = 0.0f;
        position = 0.0f;
        constrain_1 = 0.0f;
        constrain_2 = 0.0f;
        cd = 0.0f;
        cdCulling = 0.0f;
        terrain = 0.0f;
        terrainCulling = 0.0f;
        controllerCallback = 0.0f;
    }
    void Start();
    void Init();
    f32 GetCount();
    f32 GetTotalTime()
    {
        return dynamic + 
                position +
                constrain_1 +
                constrain_2 +
                cd +
                cdCulling +
                terrain +
                terrainCulling +
                controllerCallback;
    };

    void UpdateDynamic();
    void UpdatePosition();
    void UpdateConstrain1();
    void UpdateConstrain2();
    void UpdateCD();
    void UpdateCDCulling();
    void UpdateTerrain();
    void UpdateTerrainCulling();
    void UpdateControllerCallback();

    f32 dynamic;

    f32 position;

    f32 controllerCallback;

    f32 constrain_1;

    f32 constrain_2;

    f32 cdCulling;

    f32 cd;

    f32 terrain;

    f32 terrainCulling;

    s32 perfFreqAdjust; // in case Freq is too big

    s32 overheadTicks;   // overhead  in calling timer
};

#ifdef _WIN32

/**perfwin32.h**/

#include <winsock2.h>
class nePerformanceDataWin32: public nePerformanceData
{
public:
    void Start();
    
    f32 GetCount();

    LARGE_INTEGER perfFreq;

    LARGE_INTEGER counter;
};

#else

/**perflinux.h**/

class nePerformanceDataLinux: public nePerformanceData
{
public:
    void Start();

    f32 GetCount();

    s64 perfFreq;

    s64 counter;
};

#endif

/**stack.h**/

#ifndef NE_STACK_H
#define NE_STACK_H

#define NE_MAX_REST_ON 3

class neStackHeader;

class neStackInfo
{
public:
    void Init()
    {
        stackHeader = NULL;
        body = NULL;
        isTerminator = true;
        isBroken = false;
    }
    void Resolve();

    void AddToSolver(neBool addCheader);

    neStackHeader * CheckAcceptNewHeader(neStackHeader * newHeader);

    void ForceAcceptNewHeader(neStackHeader * newHeader);

    //void Break();

    void CheckHeader(neStackHeader * sh);

    neBool isResolved;
    neBool isTerminator;
    neBool isBroken;
    neStackHeader * stackHeader;
    neRigidBody_ * body;
    s32 startTime;
    s32 endTime;

    //neRestRecord restRecords[neRigidBody_::NE_RB_MAX_RESTON_RECORDS];
};

typedef neDLinkList<neStackInfo> neStackInfoHeap;

typedef neFreeListItem<neStackInfo> neStackInfoItem;

class neStackHeader
{
public:
    neFixedTimeStepSimulator * sim;

    neStackInfo * head;
    neStackInfo * tail;
    s32 infoCount;
    neBool isHeaderX;
    neBool isAllIdle;
    static s32 golbalTime;
    neBool dynamicSolved;
    
    void Null()
    {
        head = NULL;

        tail = NULL;

        infoCount = 0;

        isHeaderX = false;

        isAllIdle = false;

        dynamicSolved = false;
    }

    //void Purge();

    void Resolve();

    void CheckLength()
    {
        s32 c = 0;

        neStackInfoItem * item = (neStackInfoItem *) head;

        while (item)
        {
            ASSERT(c < infoCount);

            c++;

            item = item->next;
        }
    }
    void CheckHeader()
    {
        ASSERT(infoCount != 0);
        
        s32 c = 0;

        neStackInfoItem * item = (neStackInfoItem *) head;

        while (item)
        {
            ASSERT(c < infoCount);

            c++;

            neStackInfo * sinfo = (neStackInfo*) item;

            ASSERT(sinfo->stackHeader == this);

            if (!sinfo->isTerminator)
                sinfo->CheckHeader(this);

            item = item->next;
        }
        ASSERT(c == infoCount);
    }
    void Add(neStackInfo * add)
    {
        if (!head)
        {
            head = tail = add;

            ASSERT(((neStackInfoItem*)add)->next == NULL);
        }
        else
        {
            ASSERT(add != tail);
            
            ((neStackInfoItem*)tail)->Append((neStackInfoItem*)add);

            tail = add;
        }
        infoCount++;

        add->stackHeader = this;
    }
    void Remove(neStackInfo * add, s32 flag = 0)
    {
/*      if (infoCount == 1 && !isHeaderX && flag == 0)
            ASSERT(0);
*/
        neStackInfoItem * item = (neStackInfoItem *)add;

        if (head == add)
            head = (neStackInfo*)item->next;

        if (tail == add)
            tail = (neStackInfo*)item->prev;

        item->Remove();

        infoCount--;

        add->stackHeader = NULL;
    }
    neBool Check(neStackInfo * st)
    {
        s32 c = 0;

        neStackInfoItem * item = (neStackInfoItem *) head;

        while (item)
        {
            ASSERT(c < infoCount);

            c++;

            neStackInfo * sinfo = (neStackInfo*) item;

            ASSERT(sinfo->stackHeader == this);

            if (st == sinfo)
            {
                return true;
            }
            item = item->next;
        }
        return false;
    }
    neBool CheckStackDisconnected();

    neRigidBody_ * GetBottomStackBody()
    {
        return NULL;
/*      if (!head)
            return NULL;

        neStackInfoItem * item = (neStackInfoItem *) head;

        while (item)
        {
            neStackInfo * sinfo = (neStackInfo *) item;

            neRigidBody_ * body = sinfo->body;

            neStackInfo * nextSinfo = NULL;

            for (s32 i = 0; i < sinfo->restOnCount; i++)
            {
                ASSERT (sinfo->restOn[i].body);

                if (sinfo->restOn[i].body->stackInfo)
                {
                    if (sinfo->restOn[i].body->stackHeader == NULL)
                    {

                    }
                    else
                    {
                        nextSinfo = sinfo->restOn[i].body->stackInfo;
                        break;
                    }
                }
                else
                {
                    //return sinfo->restOn[i].body;
                }
            }
            if (nextSinfo != NULL)
            {
                item = (neStackInfoItem *)nextSinfo;
            }
            else
            {
                return sinfo->body;
            }
        }
        ASSERT(0);
        return NULL;
*/  }
    void ChangeHeader(neStackHeader * newHeader);

    void AddToSolver(/*neBool withConstraint*/);

    void AddToSolverNoConstraintHeader();

    void ResetRigidBodyFlag();
};

typedef neDLinkList<neStackHeader> neStackHeaderHeap;

typedef neFreeListItem<neStackHeader> neStackHeaderItem;

#endif //NE_STACK_H

/**rigidbody.h**/

#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#pragma inline_depth( 8 )

#define MAX_RB_STATES 1
#define MAX_RB_IMPULSE_RECORDS 4
#define MAX_RB_CONTROLLERS 5

class neRigidBodyBase;

class neRigidBody_;

class neFixedTimeStepSimulator;

class neConstraintSolver;

class neRestRecord;

///////////////////////////////////////////////////////////////////
//
//  neControllerCallback
//
///////////////////////////////////////////////////////////////////
class neController
{
PLACEMENT_MAGIC
public:
    s32 period;
    s32 count;
    neRigidBodyControllerCallback * rbc;
    neJointControllerCallback * jc;
    _neConstraint * constraint;
    neRigidBody_ * rb;
    neV3 forceA;
    neV3 torqueA;
    neV3 forceB;
    neV3 torqueB;

public:
    neController()
    {
        period = 0;
        count = 0;
        rbc = NULL;
        jc = NULL;
        constraint = NULL;
        rb = NULL;
    }
};

//typedef neFreeListItem<neController> neControllerItem;

//coordinate list

class CCoordListEntry
{
PLACEMENT_MAGIC
public:
    enum
    {
        LowEnd = 0,
        HighEnd = 1,
    };
    neByte flag;
    f32 value;
    neRigidBodyBase * bb;
};

typedef CCoordListEntry* PCCoordListEntry;

typedef neDLinkList<CCoordListEntry>::listItem CCoordListEntryItem;

///////////////////////////////////////////////////////////////////
//
//  neRigidBodyBase
//
///////////////////////////////////////////////////////////////////
class _neConstraint;

typedef neFreeListItem<neSensor_> neSensorItem;

class neCollisionBody_;

typedef neCollection<neRigidBodyBase>::itemType neBodyHandle;

//neCollection<_neConstraint>::itemType neConstraintItem;

typedef enum
{
    NE_OBJECT_COLISION,
    NE_OBJECT_RIGID,
}neObjectType;

typedef enum
{
    NE_RIGID_NORMAL,
    NE_RIGID_PARTICLE,
}neRigidType;

class neRigidBodyBase
{
public:
    neRigidBodyBase(){ 
        //isAnimated = true; 
        btype = NE_OBJECT_COLISION;
        col.convex = NULL;
        col.convexCount = 0;
        col.obb.Initialise();
        sim = NULL;
        cid = 0;
        isCollideConnected = false;
        isCollideDirectlyConnected = false;
        sensors = NULL;

        geometryCursor = NULL;
        sensorCursor = NULL;

        constraintHeaderItem.thing = this;
        isActive = true;
        regionHandle = NULL;

        _constraintHeader = NULL;

        isCustomCD = false;

        for (s32 i = 0; i < 3; i++)
        {
            maxCoord[i] = NULL;
            minCoord[i] = NULL;
        }

        backupVector.SetZero();

        pendingAddToRegion = 0;
    }
    ~neRigidBodyBase()
    {
        //if (col.convex)
        //  delete [] col.convex;
    };
    void RecalcBB();
    
    //TConvex * GetConvex(s32 index);

    void Free();

    NEINLINE neByte IsAABOverlapped(neRigidBodyBase * b)
    {
        neByte ret = 0;

        for (s32 i = 0; i < 3; i++)
        {
            if (minCoord[i])
            {
                if (!(minCoord[i]->value >= b->maxCoord[i]->value || 
                    maxCoord[i]->value <= b->minCoord[i]->value))
                    ret |= (1 << i);
            }
        }

        return ret;
    }

    neBool IsValid();

    neV3 VelocityAtPoint(const neV3 & pt);

    NEINLINE neV3 GetLinearVelocity();

    NEINLINE neV3 GetAngularVelocity();

    void CollideConnected(neBool yes);

    neBool CollideConnected();

    neSensor_ * AddSensor();

    void BeginIterateSensor();

    neSensor_ * GetNextSensor();

    void ClearSensor();

    TConvex * AddGeometry();

    void BeginIterateGeometry();

    TConvex * GetNextGeometry();

    void Active(neBool yes, neRigidBodyBase * hint);

    NEINLINE neCollisionBody_ * AsCollisionBody() 
    {
        if (btype != NE_OBJECT_COLISION)
            return NULL;

        return (neCollisionBody_ *)this;
    }

    NEINLINE neRigidBody_ * AsRigidBody()
    {
        if (btype != NE_OBJECT_RIGID)
            return NULL;
            
        return (neRigidBody_ *)this;
    }
    
    neT3 & GetB2W();

    NEINLINE void SetConstraintHeader(neConstraintHeader * cheader)
    {
        _constraintHeader = cheader;
    }
    NEINLINE neConstraintHeader * GetConstraintHeader()
    {
        return _constraintHeader;
    }
    void RemoveConstraintHeader();

    NEINLINE neBool IsInRegion()
    {
        return (regionHandle != NULL);
    }

    neBodyHandle constraintHeaderItem;

    //neBodyHandle activeHandle;

    neCollection<_neConstraint> constraintCollection;

    TConvexItem * geometryCursor;

    neSensorItem * sensorCursor;

    uintptr_t cookies;
    
    neCollision col;
    
    u32 id;

    u32 cid;

    neConstraintHeader * _constraintHeader;

    //bool isAnimated;
    neObjectType btype;

    neBool isActive;

    neBool isCollideConnected;

    neBool isCollideDirectlyConnected;

    neSensor_ * sensors;

    neFreeListItem<neRigidBodyBase *> * regionHandle;

    neFixedTimeStepSimulator * sim;

    neT3 obb;

    neBool isCustomCD;

    neV3 minBound;
    neV3 maxBound;
    CCoordListEntry * maxCoord[3];
    CCoordListEntry * minCoord[3];  

    neV3 backupVector;

    neCollection<neRestRecord> rbRestingOnMe;

    s32 pendingAddToRegion;
    
//  neV3 debugMinBound;
//  neM3 dobb;
};

///////////////////////////////////////////////////////////////////
//
//  neRigidBody_
//
///////////////////////////////////////////////////////////////////

class neRigidBodyState
{
public:
    neRigidBodyState();
    neQ q;
    neV3 linearMom; // P
    neV3 angularMom; // L

//  NEINLINE neV3 &pos() const{ 
//      return b2w.pos;}

//  NEINLINE void SetPos(const neV3 & pos){ 
//      b2w.pos = pos;}


    NEINLINE neM3 & rot() { 
        return b2w.rot;}

//private:
    neT3 b2w;
};

class neRigidBodyDerive
{
public:
    neRigidBodyDerive()
    {
        linearVel.SetZero();
        angularVel.SetZero();
        qDot.Zero();
        Iinv.SetZero();
    }
    neV3 linearVel; // v
    neV3 angularVel; // w
    neQ qDot; 
    neM3 Iinv; // R * IbodyInv * Rtrans
    f32 speed;
};

typedef struct neImpulseRecord neImpulseRecord;

struct neImpulseRecord
{
    neV3 point;
    u32 stepCount;
};

///////////////////////////////////////////////////////////////////
//
//  neCollisionBody_
//
///////////////////////////////////////////////////////////////////

class neCollisionBody_: public neRigidBodyBase
{
public:
PLACEMENT_MAGIC

    neCollisionBody_()
    {
        moved = false;
    }
    void UpdateAABB();

    void Free();

public:
    neT3 b2w;
    
    neBool moved;
};

class neStackInfo;
class neStackHeader;

/////////////////////////////////////////////////////////////////

typedef neCollection<neRestRecord>::itemType neRestRecordHandle;

class neRestRecord
{
public:
    typedef enum
    {
        REST_ON_NOT_VALID,
        REST_ON_RIGID_BODY,
        REST_ON_COLLISION_BODY,
        REST_ON_WORLD,

    } RestOnType;

    neV3 bodyPoint;
    neV3 otherBodyPoint;
    neV3 worldThisBody;
    neV3 worldOtherBody;
    neV3 worldDiff;
    neV3 normalBody; //normal define in the body space of otherBody
    neV3 normalWorld; //normal define in the world space
    f32 depth;
    f32 normalDiff;
    f32 tangentialDiffSq;
    s32 material;
    s32 otherMaterial;

private:
    RestOnType rtype;
    neRigidBodyBase * otherBody;
    neRestRecordHandle restOnHandle;
    neRigidBody_ * body;
    s32 counter;

public: 
    neRestRecord()
    {
        restOnHandle.thing = this;
    }
    void Init()
    {
        rtype = REST_ON_NOT_VALID;
        counter = 0;
        otherBody = NULL;
        body = NULL;
        restOnHandle.Remove();
    }
    neBool IsValid()
    {
        return rtype != REST_ON_NOT_VALID;
    }
    void Update();

    neRigidBodyBase * GetOtherBody() const
    {
        return otherBody;
    }
    neRigidBody_ * GetOtherRigidBody() const
    {
        if (!otherBody)
            return NULL;

        return otherBody->AsRigidBody();
    }
    neCollisionBody_ * GetOtherCollisionBody() const
    {
        if (!otherBody)
            return NULL;

        return otherBody->AsCollisionBody();
    }
    neBool CanConsiderOtherBodyIdle();

    neBool CheckOtherBody(neFixedTimeStepSimulator * sim);

    void SetInvalid();

    neV3 GetOtherBodyPoint();

    void Set(neRigidBody_* thisBody, const neRestRecord & rc);

    void SetTmp(neRigidBodyBase * otherb, const neV3 & contactA, const neV3 & contactB, const neV3 & normalBody, s32 matA, s32 matB);
};

class neRestHull
{
public:
    typedef enum
    {
        NONE,
        POINT,
        LINE,
        TRIANGLE,
        QUAD,
    }neRestHullType;

    s32 htype;
    s32 indices[4];
    neV3 normal;
};

enum
{
    NE_RB_MAX_PAST_RECORDS = 10,
    NE_RB_MAX_RESTON_RECORDS = 3,
};  

class neRBExtra
{
public:
    neRestRecord restOnRecord[NE_RB_MAX_RESTON_RECORDS];
    
    neV3 velRecords[NE_RB_MAX_PAST_RECORDS];

    neV3 angVelRecords[NE_RB_MAX_PAST_RECORDS];

    neRestHull restHull;
};

class neMotionCorrectionInfo
{
public:
    neQ  lastQuat;
    neV3 lastPos;
    neV3 lastAM;
    neV3 lastW;
    neM3 lastIinv;
    neV3 lastVel;
    neV3 lastAVel;
};

class neRigidBody_: public neRigidBodyBase
{
PLACEMENT_MAGIC

friend class neFixedTimeStepSimulator;
public:

    enum
    {
        NE_RBSTATUS_NORMAL = 0,
        NE_RBSTATUS_IDLE,
        NE_RBSTATUS_ANIMATED,
    };
    f32     mass;
    f32     oneOnMass;
    neM3    IbodyInv;
    neM3    Ibody;
    neV3    force;
    neV3    torque;
    s32     status;
    neBool  gravityOn;
    neV3    gforce;
    neV3    cforce;
    neV3    ctorque;
    neV3    totalTorque;
    neV3    totalForce;
    neV3    acc;

    f32     linearDamp;
    f32     angularDamp;

    u32 curState;
    
    neRigidBodyState stateBuffer[MAX_RB_STATES];
    
    neRigidBodyDerive derive;   
    
    s32 lowEnergyCounter;

    // constraints
    neStackInfo * stackInfo;
    
    neBool isShifted;

    neBool isShifted2;

    neController * controllers;

    neControllerItem * controllerCursor;

    neBool needRecalc;

    neBool isAddedToSolver;

    neRBExtra * rbExtra;

    neRBExtra eggs;

    neRigidType subType;

    neQ totalRot;

    neV3 totalTrans;

    s32 rotCount;

    s32 transCount;

    neQ totalLastRot;

    s32 lastRotCount;

    neV3 totalLastTrans;

    s32 lastTransCount;

    neBool needSolveContactDynamic;

    neV3 totalDV;

    neV3 totalDA;

    s32 impulseCount;

    s32 twistCount;

    neV3 dvRecord[NE_RB_MAX_PAST_RECORDS];
    neV3 davRecord[NE_RB_MAX_PAST_RECORDS];

    neCollisionResult * maxErrCResult;

    f32 sleepingParam;

    neV3 oldPosition;

    neQ oldRotation;

    neV3 oldVelocity;

    neV3 oldAngularVelocity;

    s32 oldCounter;

    //////////////////////////////////////////////////

    NEINLINE neBool IsParticle()
    {
        return (subType == NE_RIGID_PARTICLE);
    }

    NEINLINE neRestRecord & GetRestRecord(s32 index)
    {
        ASSERT(rbExtra);
        return rbExtra->restOnRecord[index];
    }

    NEINLINE neRestHull & GetRestHull()
    {
        ASSERT(rbExtra);
        return rbExtra->restHull;
    }

    NEINLINE neV3 & GetVelRecord(s32 index)
    {
        ASSERT(rbExtra);
        return rbExtra->velRecords[index];
    }

    NEINLINE neV3 & GetAngVelRecord(s32 index)
    {
        ASSERT(rbExtra);
        return rbExtra->angVelRecords[index];
    }

public:
    neRigidBody_();

    ~neRigidBody_();
    
    NEINLINE neRigidBodyState & State() 
    {
        return stateBuffer[curState];
    };
    NEINLINE void SetPos(const neV3 & newPos)
    {
        State().b2w.pos = newPos;
    }
    NEINLINE neV3 GetPos()
    {
        return State().b2w.pos;
    }
    NEINLINE neRigidBodyDerive & Derive(){
        return derive;
    }
    void RecalcInertiaTensor();

    void SetAngMom(const neV3 & am);

    void GravityEnable(neBool yes);

    void CheckForIdle();

    void CheckForIdleNonJoint();

    void CheckForIdleJoint();

    void BecomeIdle();

    void ZeroMotion();

    void WakeUp();

    neBool AddStackInfo(neRestRecord & rc);

    void  FreeStackInfo();

    neBool NewStackInfo(neRestRecord & rc);

    void MigrateNewHeader(neStackHeader * newHeader, neStackHeader * curHeader);

    void ResetRestOnRecords();

    void RemoveStackInfo();

    neBool NewStackInfoTerminator(neStackHeader * header);

    s32 AddContactImpulseRecord(neBool withConstraint);

    neBool IsRestPointStillValid();

    void AddRestContact(neRestRecord & rc);

    neBool CheckContactValidity();

    void ResolveRestingPenetration();

    neBool IsConstraintNeighbour(neRigidBodyBase * otherBody);

    neController * AddController(neRigidBodyControllerCallback * rbc, s32 period);

    void BeginIterateController();

    neController * GetNextController();

    void Free();

    void DrawCPointLine();

    void SetAngMomComponent(const neV3 & angMom, const neV3 & dir);

    void ShiftPosition(const neV3 & delta);

    void UpdateAABB();

    s32 CheckRestHull();

    neBool ApplyCollisionImpulse(const neV3 & impulse, const neV3 & contactPoint, neImpulseType itype);

    neV3 GetCorrectRotation(neRigidBody_ * otherBody, f32 massOther, neV3 & pointThis, neV3 & pointOther);

    void CorrectPosition(neV3 & pointThis, neV3 & pointDest, s32 flag, s32 changeLast);

    void CorrectRotation(f32 massOther, neV3 & pointThis, neV3 & pointDest, neV3 & pointDest2, s32 flag, s32 changeLast);

    void CorrectPenetrationDrift();

    void CorrectPenetrationDrift2(s32 index, neBool slide, s32 flag);

    f32 TestImpulse(neV3 & dir, neV3 & pt, f32 & linear, f32 & angular);

    void UpdateDerive();

    void AddContactConstraint();

    void CorrectPenetrationRotation();

    void CorrectPenetrationTranslation();

    void CorrectPenetrationRotation2(s32 index, neBool slide);

    void CorrectPenetrationTranslation2(s32 index, neBool slide);

    neBool CheckStillIdle();

    neBool CheckHighEnergy();

    neBool TestWakeUpImpulse(const neV3 & impulse);

    void MidPointIntegration(const neV3 & totalTorque, f32 tStep);

    void ImprovedEulerIntegration(const neV3 & totalTorque, f32 tStep);

    void RungeKutta4Integration(const neV3 & totalTorque, f32 tStep);

    void WakeUpAllJoint();

    void ApplyLinearConstraint();

    void ApplyAngularConstraint();

    void ConstraintDoSleepCheck();

    neBool CheckStationary();

    void SyncOldState();

    neBool AllRestRecordInvalid()   ;

protected:
    
    void AdvanceDynamic(f32 tStep);
    void AdvancePosition(f32 tStep);
    void IsCollideWith(neRigidBody_& rb);
    void UpdateController();
};

NEINLINE neV3 neRigidBodyBase::GetLinearVelocity()
{
    if (AsCollisionBody())
    {
        neV3 v;

        v.SetZero();

        return v;
    }
    else
    {
        return AsRigidBody()->Derive().linearVel;
    }
}

NEINLINE neV3 neRigidBodyBase::GetAngularVelocity()
{
    if (AsCollisionBody())
    {
        neV3 v;

        v.SetZero();

        return v;
    }
    else
    {
        return ((neRigidBody_*)this)->Derive().angularVel;
    }
}
NEINLINE neV3 neRigidBodyBase::VelocityAtPoint(const neV3 & pt)
{
    neV3 ret;

    if (AsCollisionBody())
    {
        ret.SetZero();

        return ret;
    }
    else
    {
        ret = ((neRigidBody_*)this)->Derive().linearVel;

        ret += ((neRigidBody_*)this)->Derive().angularVel.Cross(pt);

        return ret;
    }
}

/*
NEINLINE neV3 neRigidBodyBase::VelocityAtPoint(const neV3 & pt)
{
    neV3 ret;

    if (AsCollisionBody())
    {
        ret.SetZero();

        return ret;
    }
    else
    {
        //ret = ((neRigidBody_*)this)->Derive().linearVel;

        ret = (((neRigidBody_*)this)->State().pos() - ((neRigidBody_*)this)->correctionInfo.lastPos);

        ret *= (1.0f / sim->currentTimeStep);

        ret += ((neRigidBody_*)this)->Derive().angularVel.Cross(pt);

        return ret;
    }
}
*/
#define TILT_TOLERANCE 0.9f

#endif

/**simulator.h**/

#ifndef SIMULATOR_H
#define SIMULATOR_H

#define DEFAULT_CONSTRAINT_EPSILON 0.001f

#define DEFAULT_CONSTRAINT_ITERATION 4

//#define _DEBUG_REGION


///////////////////////////////////////////////////////////////////
//
//  Simulator
//
//
///////////////////////////////////////////////////////////////////
/*
class neGravityController: public neController
{
public:
    neV3 gravity;

    void ControllerUpdate(neRigidBody & rb)
    {
        neV3 gforce = gravity;

        gforce *= rb.GetMass();

        rb.ApplyForceCOG(gforce);
    }
};
*/

class neFastImpulse
{
public:
    neRigidBody_ * bodyA;
    neRigidBody_ * bodyB;
    neV3 contactA;
    neV3 contactB;
    neM3 k;
    neM3 kInv;
    neV3 initRelVel;
    neM3 collisionFrame;
    neM3 w2c;
    f32 relativeSpeedSq;
    
public:
    void Init();
    void Update();
    void Apply(f32 scale);
};


struct nePhysicsMaterial
{
    f32 friction;
    f32 resititution;
    f32 density;
};


/****************************************************************************
*
*   NE Physics Engine 
*
*   Class: neRegion
*
*   Desc:
*
****************************************************************************/ 
class neRegion;

class neCoordList
{
public:
    neDLinkList<CCoordListEntry> coordList;

    void Add(neRigidBodyBase * bb, neRigidBodyBase * hint, s32 hintCoord);

    bool Reserve(s32 size, neAllocatorAbstract * all = NULL)
    {
        return coordList.Reserve(size, all);
    }

    void Sort(bool sortOnly);

#ifdef _DEBUG

    void OuputDebug();

#endif

    s32 dim;

    neByte dimPower2;

    neRegion * region;
};

typedef struct neOverlappedPair neOverlappedPair;

struct neOverlappedPair
{
PLACEMENT_MAGIC
    neRigidBodyBase * bodyA;
    neRigidBodyBase * bodyB;
};

typedef struct neOverlapped neOverlapped;

struct neOverlapped
{
    neByte status;
    neOverlappedPair * pairItem;
};

struct neAddBodyInfo
{
    neRigidBodyBase * body;

    neRigidBodyBase * hint;
};

class neRegion 
{
public:
    neRegion() {}

//  enum {
//      MAX_OVERLAPPED = 100000,
//  };
    enum {
        SORT_DIMENSION_X = 1,
        SORT_DIMENSION_Y = 2,
        SORT_DIMENSION_Z = 4,
    };
    void Initialise(neFixedTimeStepSimulator * s, neByte sortD = (SORT_DIMENSION_X | SORT_DIMENSION_Y));

    bool AddBody(neRigidBodyBase * bb, neRigidBodyBase * hint);

    void InsertCoordList(neRigidBodyBase * bb, neRigidBodyBase * hint);

    void RemoveBody(neRigidBodyBase * bb);

    void Update();

    void Rebuild();

    neOverlapped * GetOverlappedStatus(neRigidBodyBase * a, neRigidBodyBase * b);

    void ToggleOverlapStatus(neRigidBodyBase * a, neRigidBodyBase * b, neByte dimp2);

    void ResetOverlapStatus(neRigidBodyBase * a, neRigidBodyBase * b);

    void MakeTerrain(neTriangleMesh * tris);

    void FreeTerrain();

    neTriangleTree & GetTriangleTree() {return terrainTree;}

    ~neRegion();

public:
    neByte sortDimension;
        
    neFixedTimeStepSimulator * sim;

    s32 maxRigidBodies;

    s32 maxAnimBodies;

    s32 totalBodies;

    s32 maxParticle;

//  neArray<neOverlapped> rb2rb;

//  neArray<neOverlapped> rb2ab;

    neArray<neOverlapped> b2b;

    neArray<neOverlapped> b2p;

    neSimpleArray<neAddBodyInfo> newBodies;

    neDLinkList<neRigidBodyBase *> bodies;

    neDLinkList<neOverlappedPair> overlappedPairs;

    neCoordList coordLists[3];

    neTriangleTree terrainTree;

#ifdef _DEBUG_REGION
    bool debugOn;
#endif
};


/****************************************************************************
*
*   NE Physics Engine 
*
*   Class: neCollisionTable_
*
*   Desc:
*
****************************************************************************/ 

class neCollisionTable_
{
public:
    enum
    {
        NE_COLLISION_TABLE_MAX = neCollisionTable::NE_COLLISION_TABLE_MAX,
    };

    neCollisionTable_();

    ~neCollisionTable_();

    void Set(s32 collisionID1, s32 collisionID2, neCollisionTable::neReponseBitFlag flag);

    neCollisionTable::neReponseBitFlag Get(s32 collisionID1, s32 collisionID2);

    s32 GetMaxCollisionID() {
        return NE_COLLISION_TABLE_MAX;
    };

public:
    neCollisionTable::neReponseBitFlag table[NE_COLLISION_TABLE_MAX][NE_COLLISION_TABLE_MAX];

    neCollisionTable::neReponseBitFlag terrainTable[NE_COLLISION_TABLE_MAX];
};


class neFixedTimeStepSimulator
{
public:
    friend class neRegion;

    enum {MAX_MATERIAL = 256,};

    neFixedTimeStepSimulator(const neSimulatorSizeInfo & _sizeInfo, neAllocatorAbstract * alloc = NULL, const neV3 * grav = NULL);
    
    ~neFixedTimeStepSimulator();

    void Initialise(const neV3& gravity);
    
    neRigidBody_* CreateRigidBody(neBool isParticle = false);

    neRigidBody_ * CreateRigidBodyFromConvex(TConvex * convex, neRigidBodyBase * originalBody);
    
    neCollisionBody_* CreateCollisionBody();

    void Free(neRigidBodyBase * bb);
    
    void Advance(f32 time, u32 nStep, nePerformanceReport * _perfReport = NULL);

    void Advance(f32 time, f32 minTimeStep, f32 maxTimeStep, nePerformanceReport * _perfReport = NULL);

    void Advance(nePerformanceReport * _perfReport = NULL);

    bool SetMaterial(s32 index, f32 friction, f32 restitution, f32 density);

    bool GetMaterial(s32 index, f32& friction, f32& restitution, f32& density);

    f32 HandleCollision(neRigidBodyBase * bodyA, neRigidBodyBase * bodyB, neCollisionResult & cresult, neImpulseType impulseType, f32 scale = 0.0f);

    void CollisionRigidParticle(neRigidBody_ * ba, neRigidBody_ * bb, neCollisionResult & cresult);

    void SimpleShift(const neCollisionResult & cresult);

    void RegisterPenetration(neRigidBodyBase * bodyA, neRigidBodyBase * bodyB, neCollisionResult & cresult);

    void SetTerrainMesh(neTriangleMesh * tris);

    void FreeTerrainMesh();

    void CreatePoint2PointConstraint(neRigidBodyBase * bodyA, const neV3 & pointA, neRigidBodyBase * bodyB, const neV3 & pointB);

    neStackHeader * NewStackHeader(neStackInfo *);

    neConstraintHeader * NewConstraintHeader();

    void CheckStackHeader();

    neLogOutputCallback * SetLogOutputCallback(neLogOutputCallback * fn);

    neCollisionCallback * SetCollisionCallback(neCollisionCallback * fn);

    void LogOutput(neSimulator::LOG_OUTPUT_LEVEL);

    void SetLogOutputLevel(neSimulator::LOG_OUTPUT_LEVEL lvl);

    void UpdateConstraintControllers();

    void FreeAllBodies();

    void GetMemoryAllocated(s32 & memoryAllocated);

    neBool CheckBreakage(neRigidBodyBase * originalBody, TConvex * convex, const neV3 & contactPoint, neV3 & impulse);

    void ResetTotalForce();
    
    void AdvanceDynamicRigidBodies();
        
    void AdvanceDynamicParticles();

    void AdvancePositionRigidBodies();
        
    void AdvancePositionParticles();

    void ApplyJointDamping();

    void ClearCollisionBodySensors();

    void UpdateAABB();

    //f32 SolveDynamicLocal(neCollisionResult * cr);

    f32 SolveLocal(neCollisionResult * cr);

    void AddContactConstraint(f32 & epsilon, s32 & iteration);

    void SetGravity(const neV3 & g);

    neV3 CalcNormalImpulse(neCollisionResult & cresult, neBool isContact);

    void ResetStackHeaderFlag();

    void AddCollisionResult(neCollisionResult & cresult);

    neCollisionBody_ * GetTerrainBody()
    {
        return &fakeCollisionBody;
    }
    
public:
    neSimulatorSizeInfo sizeInfo;

    nePerformanceReport * perfReport;

    nePerformanceData * perf;

    neV3 gravity;

    neV3 gravityVector;

    f32 gravityMag;

    f32 restingSpeed;

    s32 stepSoFar;

    f32 _currentTimeStep;

    f32 oneOnCurrentTimeStep;

    f32 highEnergy;

//  neConstraintSolver solver;

    neDLinkList<neConstraintHeader> constraintHeaders;

    neDLinkList<_neConstraint> constraintHeap;

//  neDLinkList<neMiniConstraint> miniConstraintHeap;

    neDLinkList<neController> controllerHeap;

    neStackInfoHeap stackInfoHeap;

    neStackHeaderHeap stackHeaderHeap;

    neStackHeader stackHeaderX;

    neDLinkList<neSensor_> sensorHeap;

    neDLinkList<TConvex> geometryHeap;

    neSimpleArray<neByte *> pointerBuffer1;

    neSimpleArray<neByte *> pointerBuffer2;

    neSimulator::LOG_OUTPUT_LEVEL logLevel;

    s32 solverStage;

    bool solverLastIteration;

    static char logBuffer[256];

    neSimpleArray<neCollisionResult> cresultHeap;

    neSimpleArray<neCollisionResult> cresultHeap2;

    neConstraintHeader contactConstraintHeader;

    f32 magicNumber;

    s32 currentRecord;

    f32 timeFromLastFrame;

    f32 lastTimeStep;

protected:
    void CheckCollision();

    void CheckTerrainCollision();

    void SolveAllConstrain();

    void SolveOneConstrainChain(f32 epsilon, s32 iteration);

    void ResolvePenetration();

    void SolveContactConstrain();

    void CheckIfStationary();

    nePhysicsMaterial materials[MAX_MATERIAL];

public:

    neAllocatorAbstract * allocator;

    neAllocatorDefault allocDef;

//data

    u32 maxRigidBodies;

    u32 maxAnimBodies;

    u32 maxParticles;

    neDLinkList<neRigidBody_> rigidBodyHeap;

    neDLinkList<neCollisionBody_> collisionBodyHeap;

    neDLinkList<neRigidBody_> rigidParticleHeap;

    neList<neRigidBody_> activeRB;

    neList<neRigidBody_> inactiveRB;

    neList<neCollisionBody_> activeCB;

    neList<neCollisionBody_> inactiveCB;

    neList<neRigidBody_> activeRP;

    neList<neRigidBody_> inactiveRP;

    neList<neCollisionResult> colResults;

    neRegion region;

    neCollisionTable_ colTable;

    neSimpleArray<neTreeNode*> treeNodes;

    neSimpleArray<s32> triangleIndex;

    neCollisionBody_ fakeCollisionBody;

//state
    bool buildCoordList;

//others
    neCollisionCallback * collisionCallback;
    
    neLogOutputCallback * logCallback;  

    neBreakageCallback * breakageCallback;

    neTerrainTriangleQueryCallback * terrainQueryCallback;

    neCustomCDRB2RBCallback * customCDRB2RBCallback;

    neCustomCDRB2ABCallback * customCDRB2ABCallback;

    s32 idleBodyCount;
};

#endif


/**boxcylinder.cpp**/

//#include "tokamak.h"
//#include "containers.h"
//#include "scenery.h"
//#include "collision.h"
//#include "collision2.h"
//#include "constraint.h"
//#include "rigidbody.h"

#include <assert.h>
#include <stdio.h>

neBool BoxTestParam::CylinderFaceTest(ConvexTestResult & res, TConvex & cylinderB, neT3 & transB, s32 whichFace)
{
    neV3 diff = trans->pos - transB.pos;

    neV3 dir = trans->rot[whichFace];

    f32 dot = dir.Dot(diff);

    if (dot > 0.0f)
    {
        dot *= -1.0f;
    }
    else
    {
        dir *= -1.0f;
    }

    f32 depth = dot + convex->BoxSize(whichFace);

    neV3 contactPoint = transB.pos;

    neV3 tmp = transB.rot[1] * cylinderB.CylinderHalfHeight();

    dot = tmp.Dot(dir);

    if (dot > 0.0f)
    {
        depth += dot;

        contactPoint += tmp;
    }
    else
    {
        depth -= dot;
        
        contactPoint -= tmp;
    }
    depth += cylinderB.CylinderRadius();

    if (depth <= 0.0f)
        return false;

    if (depth >= res.depth)
        return true;

    contactPoint += dir * cylinderB.CylinderRadius();

    neV3 project = contactPoint - dir * depth;

    s32 otherAxis1 = neNextDim1[whichFace];
    
    s32 otherAxis2 = neNextDim2[whichFace];
    
    neV3 sub = project - trans->pos;

    dot = neAbs(sub.Dot(trans->rot[otherAxis1]));

    if (dot > (convex->BoxSize(otherAxis1) * 1.001f))
        return true;// not false ???? no it is true!!! 

    dot = neAbs(sub.Dot(trans->rot[otherAxis2]));

    if (dot > (convex->BoxSize(otherAxis2) * 1.001f))
        return true;// not false ???? no it is true!!! 
    
    res.contactA = project;

    res.contactB = contactPoint;

    res.contactNormal = dir;

    res.depth = depth;

    res.valid = true;
    
    return true;
}

neBool BoxTestParam::CylinderEdgeTest(ConvexTestResult & res, TConvex & cylinderB, neT3 & transB, s32 whichEdge)
{
    neV3 diff = trans->pos - transB.pos;

    neV3 dir = trans->rot[whichEdge].Cross(transB.rot[1]);

    f32 len = dir.Length();

    if (neIsConsiderZero(len))
        return true;

    dir *= (1.0f / len);

    f32 dot = dir.Dot(diff);

    if (dot > 0.0f)
    {
        dot *= -1.0f;
    }
    else
    {
        dir *= -1.0f;
    }

    f32 depth = dot + cylinderB.CylinderRadius();

    neV3 contactPoint = trans->pos;

    s32 i;

    for (i = 0; i < 3; i++)
    {
        if (i == whichEdge)
            continue;

        dot = dir.Dot(radii[i]);

        if (dot > 0.0f)
        {
            depth += dot;

            contactPoint -= radii[i];
        }
        else
        {
            depth -= dot;

            contactPoint += radii[i];
        }
    }
    if (depth <= 0.0f)
        return false;

    ConvexTestResult cr;

    cr.edgeA[0] = contactPoint + radii[whichEdge];
    cr.edgeA[1] = contactPoint - radii[whichEdge];
    cr.edgeB[0] = transB.pos + transB.rot[1] * cylinderB.CylinderHalfHeight();
    cr.edgeB[1] = transB.pos - transB.rot[1] * cylinderB.CylinderHalfHeight();

    f32 au, bu;

    // A is the box, B is the cylinder

    cr.ComputerEdgeContactPoint2(au, bu);

    if (cr.depth >= res.depth)
        return true;

    if (cr.valid)
    {
        depth = cylinderB.CylinderRadius() - cr.depth;

        if (depth <= 0.0f)
            return false;

        if (depth >= res.depth)
            return true;;

        res.valid = true;

        res.contactNormal = dir;

        res.contactA = cr.contactA;

        res.contactB = cr.contactB + res.contactNormal * cylinderB.CylinderRadius();

        res.depth = depth;
    }
    else
    {
        // A is the box, B is the cylinder

        if (au > 0.0 && au < 1.0f)
        {
            // box edge and cylinder end

            neV3 cylinderVert;

            if (bu <= 0.0f)
            {
                cylinderVert = cr.edgeB[0];
            }
            else
            {
                cylinderVert = cr.edgeB[1];
            }
            neV3 project;

            f32 dist = cylinderVert.GetDistanceFromLine2(project, cr.edgeA[0], cr.edgeA[1]);

            f32 depth = cylinderB.CylinderRadius() - dist;

            if (depth <= 0.0f)
                return true;

            if (depth >= res.depth)
                return true;

            res.depth = depth;
            res.valid = true;
            res.contactNormal = project - cylinderVert;
            res.contactNormal.Normalize();
            res.contactA = project;
            res.contactB = cylinderVert + res.contactNormal * cylinderB.CylinderRadius();
        }
        else
        {
            neV3 boxVert;

            if (au <= 0.0f)
            {
                boxVert = cr.edgeA[0];
            }
            else // au >= 1.0f
            {
                boxVert = cr.edgeA[1];
            }
            if (bu > 0.0f && bu < 1.0f)
            {
                // boxVert and cylinder edge

                neV3 project;

                f32 depth = boxVert.GetDistanceFromLine2(project, cr.edgeB[0], cr.edgeB[1]);

                depth = cylinderB.CylinderRadius() - depth;

                if (depth <= 0.0f)
                    return true;

                if (depth >= res.depth)
                    return true;

                res.depth = depth;
                res.valid = true;
                res.contactA = boxVert;
                res.contactNormal = boxVert - project;
                res.contactNormal.Normalize();
                res.contactB = project + res.contactNormal * cylinderB.CylinderRadius();
            }
            else
            {
                // box vert and cylinder end

                neV3 cylinderVert;

                if (bu <= 0.0f)
                {
                    cylinderVert = cr.edgeB[0];
                }
                else
                {
                    cylinderVert = cr.edgeB[1];
                }
                neV3 diff = boxVert - cylinderVert;

                f32 depth = diff.Dot(diff);

                if (depth >= cylinderB.CylinderRadiusSq())
                    return true;

                depth = sqrtf(depth);

                depth = cylinderB.CylinderRadius() - depth;

                if (depth >= res.depth)
                    return true;

                res.depth = depth;
                res.valid = true;
                res.contactNormal = diff;
                res.contactNormal.Normalize();
                res.contactA = boxVert;
                res.contactB = cylinderVert + res.contactNormal * cylinderB.CylinderRadius();
            }
        }
    }   

    return true;
}


/**collisionbody.cpp**/

#pragma inline_recursion( on )
#pragma inline_depth( 250 )

//#include <stdio.h>

/*
#ifdef _WIN32
#include <windows.h>
#endif
*/

//#include "tokamak.h"
//#include "containers.h"
//#include "scenery.h"
//#include "collision.h"
//#include "constraint.h"
//#include "rigidbody.h"
//#include "scenery.h"
//#include "stack.h"
//#include "simulator.h"
//#include "message.h"


/****************************************************************************
*
*   neCollisionBody_::UpdateAABB
*
****************************************************************************/ 

void neCollisionBody_::UpdateAABB()
{
    if (col.convexCount == 0 && !isCustomCD)
        return;
/*
    neM3 c;
        
    c[0] = col.obb.as.box.boxSize[0] * col.obb.c2p.rot[0];
    c[1] = col.obb.as.box.boxSize[1] * col.obb.c2p.rot[1];
    c[2] = col.obb.as.box.boxSize[2] * col.obb.c2p.rot[2];
*/
    neT3 c2w = b2w * obb;   

    neV3 &pos = c2w.pos;

    int i;

    for (i = 0; i < 3; i++)
    {
        f32 a = neAbs(c2w.rot[0][i]) + neAbs(c2w.rot[1][i]) + neAbs(c2w.rot[2][i]);

        minBound[i] = pos[i] - a;
        maxBound[i] = pos[i] + a;

        if (minCoord[i])
            minCoord[i]->value = pos[i] - a;// - col.boundingRadius;

        if (maxCoord[i])
            maxCoord[i]->value = pos[i] + a;// + col.boundingRadius;
    }
};

void neCollisionBody_::Free()
{
    neRigidBodyBase::Free();

    RemoveConstraintHeader();
}


/**collision.cpp**/

//#include <stdio.h>
/*
#ifdef _WIN32
#include <windows.h>
#endif
*/
//#include "tokamak.h"
//#include "containers.h"
//#include "scenery.h"
//#include "collision.h"
//#include "collision2.h"
//#include "constraint.h"
//#include "rigidbody.h"
//#include "scenery.h"
//#include "stack.h"
//#include "simulator.h"
//#include "message.h"
//#include "dcd.h"

s32 currentMicroStep = 0;

extern f32 CONSTRAINT_THESHOLD_JOINT;

extern f32 CONSTRAINT_THESHOLD_CONTACT;

extern f32 CONSTRAINT_THESHOLD_LIMIT;

extern f32 CONSTRAINT_CONVERGE_FACTOR_JOINT;

extern f32 CONSTRAINT_CONVERGE_FACTOR_CONTACT;

extern f32 CONSTRAINT_CONVERGE_FACTOR_LIMIT;

s32 magicN;

//extern void DrawLine(const neV3 & colour, neV3 * startpoint, s32 count);

//#pragma inline_recursion( on )
//#pragma inline_depth( 50 )

neBool neCollisionResult::CheckIdle()
{
    f32 theshold = 1.0f;

    if (relativeSpeed > theshold)
        return false;

    neRigidBody_* ba = bodyA->AsRigidBody();

    neRigidBody_* bb = NULL;
    
    if (bodyB)
    {
        bb = bodyB->AsRigidBody();
    }

    if (ba && ba->status == neRigidBody_::NE_RBSTATUS_IDLE)
    {
        if (bb)
        {
            if (bb->status == neRigidBody_::NE_RBSTATUS_IDLE)
            {
                return true;
            }
            else
            {
                bodyA = NULL;

                return false;
            }
        }
        else
        {
            return true;
        }
    }
    else if (bb && bb->status == neRigidBody_::NE_RBSTATUS_IDLE)
    {
        if (ba)
        {
            bodyB = NULL;

            return false;
        }
    }
    return false;
}

void neCollisionResult::StartStage2()
{
    if (impulseType != IMPULSE_CONTACT)
        return;

    //f32 timeStep;

    neV3 relVel;

    if (bodyA)
    {
        relVel = bodyA->VelocityAtPoint(contactA) * 1.0f;

        //timeStep = bodyA->sim->currentTimeStep;

        if (bodyA->AsRigidBody())
            bodyA->AsRigidBody()->needRecalc = true;
    }

    if (bodyB)
    {
        relVel -= bodyB->VelocityAtPoint(contactB);

        //timeStep = bodyB->sim->currentTimeStep;

        if (bodyB->AsRigidBody())
            bodyB->AsRigidBody()->needRecalc = true;
    }
    contactBWorld = w2c * relVel;

    contactBWorld[2] = 0.0f;
    
    //contactBWorld.RemoveComponent(collisionFrame[2]);

    //contactBWorld.SetZero();
}

void neCollisionResult::UpdateConstraintRelativeSpeed()
{
    ASSERT(impulseType == IMPULSE_CONTACT);

    neV3 relVel;

    relVel.SetZero();

    s32 solverStage;

    if (bodyA)
    {
        relVel = bodyA->VelocityAtPoint(contactA) * -1.0f;

        solverStage = bodyA->sim->solverStage;
    }

    if (bodyB)
    {
        relVel += bodyB->VelocityAtPoint(contactB);

        solverStage = bodyB->sim->solverStage;
    }
    if (solverStage != 2)
    {
        ASSERT(FALSE);

        return;
    }
    relativeSpeed = relVel.Dot(collisionFrame[2]);

    if (relativeSpeed < 0.0f)
    {
        relativeSpeed = 0.0f;
    }
}

void neCollisionResult::CalcCollisionMatrix(neRigidBody_* ba, neRigidBody_ * bb, neBool isWorld)
{
    neM3 zero;

    zero.SetZero();

    neM3 * IinvAW; 
    
    f32 oneOnMassA;

    if (!ba)
    {
        oneOnMassA = 0.0f;

        IinvAW = &zero;//.SetZero();
    }
    else
    {
        IinvAW = &ba->Derive().Iinv;

        oneOnMassA = ba->oneOnMass;
    }
    neM3 * IinvBW;

    f32 oneOnMassB;

    if (!bb)
    {
        oneOnMassB = 0.0f;

        IinvBW = &zero;//.SetZero();
    }
    else
    {
        IinvBW = &bb->Derive().Iinv;

        oneOnMassB = bb->oneOnMass;
    }

    k.SetIdentity();

    //k *= (oneOnMassA + oneOnMassB);
    f32 oom = oneOnMassA + oneOnMassB;

    k[0][0] = oom;
    k[1][1] = oom;
    k[2][2] = oom;

    if (isWorld)
    {
        neM3 tmp = contactA ^ (*IinvAW) ^ contactA;

        k = k - tmp;

        tmp = contactB ^ (*IinvBW) ^ contactB;

        k = k - tmp;
    }
    else
    {
        //neM3 w2c; 

        //w2c.SetTranspose(collisionFrame);

        neV3 pointA; pointA = w2c * contactA;

        neV3 pointB; pointB = w2c * contactB;

        neM3 IinvAC; IinvAC = w2c * (*IinvAW) * collisionFrame;

        neM3 IinvBC; IinvBC = w2c * (*IinvBW) * collisionFrame;

        neM3 tmp = pointA ^ IinvAC ^ pointA;

        k = k - tmp;

        tmp = pointB ^ IinvBC ^ pointB;

        k = k - tmp;
    }
    kInv.SetInvert(k);

    ASSERT(kInv.IsFinite());
}

void neCollisionResult::CalcCollisionMatrix2(neRigidBody_* ba, neRigidBody_ * bb)
{
    k.SetZero();

    if (ba)
        k = ba->Derive().Iinv;

    if (bb)
        k += bb->Derive().Iinv;

    kInv.SetInvert(k);
}

void neCollisionResult::CalcCollisionMatrix3(neRigidBody_* ba, neRigidBody_ * bb)
{
    neM3 kk;

    kk.SetZero();

    neM3 ii;

    ii = bb->GetB2W().rot * k;

    neM3 kTrans; kTrans.SetTranspose(ii);

    if (ba)
        kk = ii * ba->Derive().Iinv * kTrans;

    if (bb)
        kk += bb->Derive().Iinv;

    kInv.SetInvert(kk);
}

void neCollisionResult::PrepareForSolver(neBool aIdle, neBool bIdle)
{
    neRigidBody_ * ba = NULL;

    neRigidBody_ * bb = NULL;

    if (bodyA && bodyA->AsRigidBody() && !aIdle)
    {
        ba = bodyA->AsRigidBody();
    }
    if (bodyB && bodyB->AsRigidBody() && !bIdle)
    {
        bb = bodyB->AsRigidBody();
    }

    switch (impulseType)
    {
    case IMPULSE_NORMAL:
    case IMPULSE_CONTACT:
        {
            ChooseAxis(collisionFrame[0], collisionFrame[1], collisionFrame[2]);

            ASSERT(collisionFrame.IsFinite());

            w2c.SetTranspose(collisionFrame);

            CalcCollisionMatrix(ba, bb, false);
        }
        break;

    case IMPULSE_CONSTRAINT:
    case IMPULSE_SLIDER:
    case IMPULSE_SLIDER_LIMIT_PRIMARY:
        {
            CalcCollisionMatrix(ba, bb, true);
        }
        
        break;
    case IMPULSE_ANGULAR_LIMIT_PRIMARY:
    case IMPULSE_ANGULAR_MOTOR_PRIMARY:
        {
            CalcCollisionMatrix2(ba, bb);
        }
        break;
    
    case IMPULSE_ANGULAR_LIMIT_SECONDARY:
        {
            CalcCollisionMatrix3(ba, bb);
        }
        break;

    case IMPULSE_RELATIVE_LINEAR_VELOCITY:
        {
            f32 oneOnMassA, oneOnMassB;

            if (!ba)
            {
                oneOnMassA = 0.0f;
            }
            else
            {
                oneOnMassA = ba->oneOnMass;
            }
            if (!bb)
            {
                oneOnMassB = 0.0f;
            }
            else
            {
                oneOnMassB = bb->oneOnMass;
            }
            kInv[0][0] = 1.0f / (oneOnMassA + oneOnMassB);
        }   
        break;
    }
}

void neCollision::CalcBB()
{
    s32 i;
    
    boundingRadius = 0.0f;  

    if (convexCount == 0)
        return;

    TConvexItem * gi = (TConvexItem *) convex;

    while (gi)
    {
        TConvex * g = (TConvex *)gi;

        gi = gi->next;

        f32 r = g->GetBoundRadius();

        if (r > boundingRadius)
            boundingRadius = r;
    }

    if (convexCount == 1 && (convex->type == TConvex::BOX))
    {
        obb = *convex;
        //obb.as.box.boxSize *= 1.5f;
    }
    else
    {
        neV3 maxExt, minExt;

        maxExt.Set(-1.0e6f, -1.0e6f, -1.0e6f);
        minExt.Set(1.0e6f, 1.0e6f, 1.0e6f);

        neV3 _maxExt, _minExt;

        gi = (TConvexItem *) convex;
        
        while (gi)
        {
            TConvex * g = (TConvex *)gi;

            gi = gi->next;

            g->GetExtend(_minExt, _maxExt);

            for (s32 j = 0; j < 3; j++)
            {
                maxExt[j] = neMax(maxExt[j], _maxExt[j]);
                minExt[j] = neMin(minExt[j], _minExt[j]);
            }
        }

        obb.c2p.rot.SetIdentity();

        for (i = 0; i < 3; i++)
        {
            obb.as.box.boxSize[i] = ( maxExt[i] - minExt[i] ) * 0.5f;
            obb.c2p.pos[i] = minExt[i] + obb.as.box.boxSize[i];
        }
    }
}

void TConvex::GetExtend(neV3 & minExt, neV3 & maxExt)
{
    s32 i;

    switch (GetType())
    {
    case TConvex::BOX:
        for (i = 0; i < 3; i++)
        {
            maxExt[i] = neAbs(c2p.rot[0][i]) * as.box.boxSize[0] +
                            neAbs(c2p.rot[1][i]) * as.box.boxSize[1] +
                            neAbs(c2p.rot[2][i]) * as.box.boxSize[2] +
                            c2p.pos[i];
            minExt[i] = -neAbs(c2p.rot[0][i]) * as.box.boxSize[0] +
                            -neAbs(c2p.rot[1][i]) * as.box.boxSize[1] +
                            -neAbs(c2p.rot[2][i]) * as.box.boxSize[2] +
                            c2p.pos[i];
        }
        break;
    
    case TConvex::SPHERE:
        {
            neV3 rad;

            rad.Set(Radius());

            maxExt = c2p.pos + rad;

            minExt = c2p.pos - rad;
        }
        break;
    
    case TConvex::CYLINDER:
        for (i = 0; i < 3; i++)
        {
            maxExt[i] = neAbs(c2p.rot[0][i]) * CylinderRadius() +
                            neAbs(c2p.rot[1][i]) * (CylinderHalfHeight() + CylinderRadius()) +
                            neAbs(c2p.rot[2][i]) * CylinderRadius() +
                            c2p.pos[i];
            minExt[i] = -neAbs(c2p.rot[0][i]) * CylinderRadius() +
                            -neAbs(c2p.rot[1][i]) * (CylinderHalfHeight() + CylinderRadius()) +
                            -neAbs(c2p.rot[2][i]) * CylinderRadius() +
                            c2p.pos[i];
        }
        break;

#ifdef USE_OPCODE
    
    case TConvex::OPCODE_MESH:
    {
        IceMaths::Point minex, maxex;

        minex.Set(1.0e6f);

        maxex.Set(-1.0e6f);

        for (u32 kk = 0; kk < as.opcodeMesh.vertCount; kk++)
        {
            minex = minex.Min(as.opcodeMesh.vertices[kk]);

            maxex = maxex.Max(as.opcodeMesh.vertices[kk]);
        }
        minExt = minex;

        maxExt = maxex;
    }
    break;

#endif //USE_OPCODE

    case TConvex::CONVEXITY:
    {
        neV3 minex; minex.Set(1.0e6f);

        neV3 maxex; maxex.Set(-1.0e6f);

        for (s32 kk = 0; kk < as.convexMesh.vertexCount; kk++)
        {
            minex.SetMin(minex, as.convexMesh.vertices[kk]);

            maxex.SetMax(maxex, as.convexMesh.vertices[kk]);
        }
        minExt = minex * (2.0f + envelope);

        maxExt = maxex * (2.0f + envelope);
    }
    break;

    case TConvex::CONVEXDCD:
    {
        neV3 minex; minex.Set(1.0e6f);

        neV3 maxex; maxex.Set(-1.0e6f);

        for (s32 kk = 0; kk < as.convexDCD.numVerts; kk++)
        {
            minex.SetMin(minex, as.convexDCD.vertices[kk]);

            maxex.SetMax(maxex, as.convexDCD.vertices[kk]);
        }
        minExt = minex * (2.0f);

        maxExt = maxex * (2.0f);
    }
    break;

    default:
        ASSERT(0);
    }
}

f32 TConvex::GetBoundRadius()
{
    f32 extend = 0.0f;

    switch (GetType())
    {
    case TConvex::BOX:
        {
            neV3 v3;
            
            v3.Set(as.box.boxSize[0], as.box.boxSize[1], as.box.boxSize[2]);
            
            extend = v3.Length();

            extend += c2p.pos.Length();
        }
        break;
    case TConvex::SPHERE:
        extend = c2p.pos.Length() + as.sphere.radius;
        break;

    case TConvex::CYLINDER:
        //extend = c2p.pos.Length() + sqrtf(CylinderRadiusSq() + CylinderHalfHeight() * CylinderHalfHeight());
        {
            f32 r = CylinderRadiusSq() + CylinderHalfHeight();
            
            extend = c2p.pos.Length() + r;
        }
        break;

    case TConvex::CONVEXITY:
        {
            for (s32 i = 0; i < as.convexMesh.vertexCount; i++)
            {
                f32 l = as.convexMesh.vertices[i].Length();

                if (l > extend)
                {
                    extend = l;
                }
            }
            extend *= (1.0f + envelope);
        }
        break;

    case TConvex::CONVEXDCD:
        {
            for (s32 i = 0; i < as.convexMesh.vertexCount; i++)
            {
                f32 l = as.convexMesh.vertices[i].Length();

                if (l > extend)
                {
                    extend = l;
                }
            }
        }
        break;

#ifdef USE_OPCODE
        
    case TConvex::OPCODE_MESH:
    {       
        for (u32 kk = 0; kk < as.opcodeMesh.vertCount; kk++)
        {
            f32 tmp = as.opcodeMesh.vertices[kk].Magnitude();

            if (tmp > extend)
                extend = tmp;
        }
    }
    break;

#endif //USE_OPCODE

    default:
        //fprintf(stderr, "TConvex::GetExtend - error: unrecongised primitive type\n");
        ASSERT(0);
        break;
    }
    return extend;
}

void TConvex::SetBoxSize(f32 width, f32 height, f32 depth)
{
    type = TConvex::BOX;
    as.box.boxSize[0] = width / 2.0f;
    as.box.boxSize[1] = height / 2.0f;
    as.box.boxSize[2] = depth / 2.0f;

    boundingRadius = as.box.boxSize.Length();

    envelope = 0.0f;
}

void TConvex::SetSphere(f32 radius)
{
    type = TConvex::SPHERE;

    as.sphere.radius = radius;

    as.sphere.radiusSq = radius * radius;

    boundingRadius = radius;

    envelope = 0.0f;
}

void TConvex::SetConvexMesh(neByte * convexData)
{
    type = TConvex::CONVEXDCD;
    
    as.convexDCD.convexData = convexData;

    s32 numFace = *((s32*)convexData);

    as.convexDCD.numVerts =  *((s32*)convexData + 1);

    as.convexDCD.vertices = (neV3 *)(convexData + numFace * sizeof(f32) * 4);
}

void TConvex::SetTriangle(s32 a, s32 b, s32 c, neV3 * _vertices)
{
    type = TConvex::TRIANGLE;

    as.tri.indices[0] = a;
    as.tri.indices[1] = b;
    as.tri.indices[2] = c;
    vertices = _vertices;
}

void TConvex::SetTerrain(neSimpleArray<s32> & triangleIndex, neArray<neTriangle_> & triangles, neV3 * _vertices)
{
    type = TConvex::TERRAIN;

    as.terrain.triangles = &triangles;
    as.terrain.triIndex = &triangleIndex;
    vertices = _vertices;
}


#ifdef USE_OPCODE

void TConvex::SetOpcodeMesh(IndexedTriangle * triIndex, u32 triCount, IceMaths::Point * vertArray, u32 vertCount)
{
    type = TConvex::OPCODE_MESH;
    
    as.opcodeMesh.triIndices = triIndex;
    as.opcodeMesh.triCount = triCount;
    as.opcodeMesh.vertices = vertArray;
    as.opcodeMesh.vertCount = vertCount;
}

#endif //USE_OPCODE

void TConvex::SetMaterialId(s32 index)
{
    matIndex = index;
}
/*
void TConvex::SetId(s32 _id)
{
    id = _id;
}

s32  TConvex::GetId()
{
    return id;
}
*/
s32  TConvex::GetMaterialId()
{
    return matIndex;
}

u32 TConvex::GetType()
{
    return type;
}

void TConvex::SetTransform(neT3 & t3)
{
    c2p = t3;
}

neT3 TConvex::GetTransform()
{
    switch (GetType())
    {
    case TConvex::BOX: 
    case TConvex::CYLINDER: 
    case TConvex::SPHERE: 

#ifdef USE_OPCODE
        
    case TConvex::OPCODE_MESH:

#endif //USE_OPCODE

        return c2p;
        break;
    default:
        ASSERT(1);
    }

    neT3 ret;

    ret.SetIdentity();
    
    return ret;
}

void TConvex::Initialise()
{
    SetBoxSize(1.0f, 1.0f, 1.0f);
    neT3 t;
    t.SetIdentity();
    SetTransform(t);
    matIndex = 0;
    //id = 0;
    userData = 0;

    breakInfo.mass = 1.0f;
    breakInfo.inertiaTensor = neBoxInertiaTensor(1.0f, 1.0f, 1.0f, 1.0f);
    breakInfo.breakMagnitude = 0.0f;
    breakInfo.breakAbsorb = 0.5f;
    breakInfo.neighbourRadius = 0.0f;
    breakInfo.flag = neGeometry::NE_BREAK_DISABLE; //break all,
    breakInfo.breakPlane.SetZero();
}

/****************************************************************************
*
*   TConvex::CalcInertiaTensor
*
****************************************************************************/ 
void TranslateCOM(neM3 & I, neV3 &translate, f32 mass, f32 factor)
{
    s32 i,j,k;
    f32 change;

    for(i=0;i<3;i++)
    {
        for(j=i;j<3;j++)
        {
            if(i==j)
            {
                change = 0.0f;
                for(k=0;k<3;k++)

                {
                    if(k!=i)
                    {
                        change += (translate[k] * translate[k]);
                    }
                }
            }
            else
            {
                change += (translate[i] * translate[j]);
            }
            change *= mass;

            change *= factor;

            I[j][i] += change;
            
            if (i != j)
                I[i][j] += change;
        }
    }
    return;
}

neM3 TConvex::CalcInertiaTensor(f32 density, f32 & mass)
{
    neM3 ret;

    ret.SetZero();

    switch (GetType())
    {
    case TConvex::BOX:
    {
        f32 xsq = as.box.boxSize[0];
        f32 ysq = as.box.boxSize[1];
        f32 zsq = as.box.boxSize[2];

        xsq *= xsq;
        ysq *= ysq;
        zsq *= zsq;

        mass = as.box.boxSize[0] * as.box.boxSize[1] * as.box.boxSize[2] * 8.0f * density;

        ret[0][0] = (ysq + zsq) * mass / 3.0f;
        ret[1][1] = (xsq + zsq) * mass / 3.0f;
        ret[2][2] = (xsq + ysq) * mass / 3.0f;

        break;
    }
    default:
        ASSERT(1);
    }

    neM3 invrotation;
    
    //invrotation.SetInvert(c2p.rot);

    invrotation.SetTranspose(c2p.rot);

    ret = ret * invrotation;

    ret = c2p.rot * ret;

    TranslateCOM(ret, c2p.pos, mass, 1.0f);
    
    return ret;
}

/****************************************************************************
*
*   CollisionModelTest
*
****************************************************************************/ 

void CollisionTest(neCollisionResult & result, neCollision & colA, neT3 & transA, neCollision & colB, neT3 & transB, const neV3 & backupVector)
{
    result.penetrate = false;

    if (colA.convexCount == 0 || colB.convexCount == 0)
    {
        return;
    }

    neCollisionResult candidate[2];

    s32 cur = 0;
    s32 res = 1;
    s32 tmp;

    candidate[res].depth = 0.0f;

    neT3 convex2WorldA;
    neT3 convex2WorldB;

    if (colA.convexCount == 1 && colB.convexCount == 1)
    {
        convex2WorldA = transA * colA.convex->c2p;

        if (colB.convex->type != TConvex::TERRAIN)
            convex2WorldB = transB * colB.convex->c2p;

        ConvexCollisionTest(candidate[res], *colA.convex, convex2WorldA, *colB.convex, convex2WorldB, backupVector);

        if (candidate[res].penetrate)
        {
            candidate[res].convexA = colA.convex;

            candidate[res].convexB = colB.convex;
            
            goto CollisionTest_Exit;
        }
        else
        {
            return;
        }
    }

    convex2WorldA = transA * colA.obb.c2p;

    if (colB.obb.type != TConvex::TERRAIN)
        convex2WorldB = transB * colB.obb.c2p;

    ConvexCollisionTest(candidate[res], colA.obb, convex2WorldA, colB.obb, convex2WorldB, backupVector);

    if (candidate[res].penetrate == false)
    {
        return; //no more to do
    }

    candidate[res].depth = 0.0f;

    candidate[res].penetrate = false;

    if (colA.convexCount == 1 && colB.convexCount > 1)
    {
        TConvexItem * gi = (TConvexItem *)colB.convex;

        while (gi)
        {
            TConvex * g = (TConvex *) gi;

            gi = gi->next;

            convex2WorldB = transB * g->c2p;

            ConvexCollisionTest(candidate[cur], *colA.convex, convex2WorldA, *g, convex2WorldB, backupVector);

            if (candidate[cur].penetrate && (candidate[cur].depth > candidate[res].depth))
            {
                candidate[cur].convexA = colA.convex;
                candidate[cur].convexB = g;
                tmp = res;
                res = cur;
                cur = tmp;
            }
        }
    }
    else if (colA.convexCount > 1 && colB.convexCount == 1)
    {
        TConvexItem * gi = (TConvexItem *)colA.convex;

        while (gi)
        {
            TConvex * g = (TConvex *) gi;

            gi = gi->next;

            convex2WorldA = transA * g->c2p;

            ConvexCollisionTest(candidate[cur], *g, convex2WorldA, *colB.convex, convex2WorldB, backupVector);

            if (candidate[cur].penetrate && (candidate[cur].depth > candidate[res].depth))
            {
                candidate[cur].convexA = g;
                candidate[cur].convexB = colB.convex;
                tmp = res;
                res = cur;
                cur = tmp;
            }
        }
    }
    else //colA.convexCount > 1 && colB.convexCount > 1
    {
        const s32 totalPotentials = 100;

        static TConvex * potentialsA[totalPotentials];
        static TConvex * potentialsB[totalPotentials];

        s32 potentialsACount = 0;
        s32 potentialsBCount = 0;

        TConvexItem * giA = (TConvexItem *)colA.convex;

        convex2WorldB = transB * colB.obb.c2p;

        while (giA)
        {
            TConvex * gA = (TConvex *) giA;

            giA = giA->next;

            convex2WorldA = transA * gA->c2p;

            ConvexCollisionTest(candidate[0], *gA, convex2WorldA, colB.obb, convex2WorldB, backupVector);

            if (!candidate[0].penetrate)
                continue;
            
            potentialsA[potentialsACount++] = gA;
        }

        TConvexItem * giB = (TConvexItem *)colB.convex;

        convex2WorldA = transA * colA.obb.c2p;

        while (giB)
        {
            TConvex * gB = (TConvex *) giB;

            giB = giB->next;

            convex2WorldB = transB * gB->c2p;

            ConvexCollisionTest(candidate[0], colA.obb, convex2WorldA, *gB, convex2WorldB, backupVector);

            if (!candidate[0].penetrate)
                continue;
            
            potentialsB[potentialsBCount++] = gB;
        }

        
        cur = 0;

        res = 1;

        candidate[res].depth = 0.0f;

        candidate[res].penetrate = false;

        for (s32 i = 0; i < potentialsACount; i++)
        {
            convex2WorldA = transA * potentialsA[i]->c2p;

            for (s32 j = 0; j < potentialsBCount; j++)
            {
                convex2WorldB = transB * potentialsB[j]->c2p;

                ConvexCollisionTest(candidate[cur], *potentialsA[i], convex2WorldA, 
                                                    *potentialsB[j], convex2WorldB, backupVector);  

                if (candidate[cur].penetrate && (candidate[cur].depth > candidate[res].depth))
                {
                    candidate[cur].convexA = potentialsA[i];
                    candidate[cur].convexB = potentialsB[j];
                    tmp = res;
                    res = cur;
                    cur = tmp;
                }
            }
        }
    }

    if (!candidate[res].penetrate)
        return;

CollisionTest_Exit:

    result = candidate[res];

    result.contactAWorld = result.contactA;

    result.contactBWorld = result.contactB;

    result.contactA = result.contactA - transA.pos;

    result.contactB = result.contactB - transB.pos;

    result.contactABody = transA.rot.TransposeMulV3(result.contactA);

    result.contactBBody = transB.rot.TransposeMulV3(result.contactB);
    
    result.materialIdA = result.convexA->GetMaterialId(); 

    if (colB.obb.GetType() != TConvex::TERRAIN && colB.obb.GetType() != TConvex::TRIANGLE)
    {
        result.materialIdB = result.convexB->GetMaterialId();
    }
}

/****************************************************************************
*
*   ConvexCollisionTest
*
****************************************************************************/ 

void ConvexCollisionTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB, const neV3 & backupVector)
{
    switch (convexA.type)
    {
    case TConvex::BOX:
        switch (convexB.type)
        {
        case TConvex::BOX:
            Box2BoxTest(result, convexA, transA, convexB, transB, backupVector);
            break;
        
        case TConvex::SPHERE:
            Box2SphereTest(result, convexA, transA, convexB, transB);
            break;

        case TConvex::CYLINDER:
            Box2CylinderTest(result, convexA, transA, convexB, transB);
            break;

        case TConvex::TRIANGLE:
            Box2TriangleTest(result, convexA, transA, convexB, transB);
            break;
        case TConvex::TERRAIN:
            Box2TerrainTest(result, convexA, transA, convexB);
            break;
        case TConvex::CONVEXDCD:
            Box2ConvexTest(result, convexA, transA, convexB, transB, backupVector);
            break;

#ifdef USE_OPCODE

        case TConvex::OPCODE_MESH:
            Box2OpcodeTest(result, convexA, transA, convexB, transB);
            break;
                
#endif //USE_OPCODE

        default:
            ASSERT(0);
            break;
        }
        break;
    
    case TConvex::SPHERE:
        switch (convexB.type)
        {
            case TConvex::BOX:
                Box2SphereTest(result, convexB, transB, convexA, transA);
                
                result.Swap();
                
                break;
            
            case TConvex::SPHERE:
                Sphere2SphereTest(result, convexA, transA, convexB, transB);

                break;

            case TConvex::CYLINDER:
                Cylinder2SphereTest(result, convexB, transB, convexA, transA);

                result.Swap();

                break;

            case TConvex::TRIANGLE:
                //Sphere2TriangleTest(result, convexA, transA, convexB, transB);
                break;
            case TConvex::TERRAIN:
                Sphere2TerrainTest(result, convexA, transA, convexB);
                break;

#ifdef USE_OPCODE

            case TConvex::OPCODE_MESH:
                Sphere2OpcodeTest(result, convexA, transA, convexB, transB);
                break;

#endif //USE_OPCODE

            default:
                ASSERT(0);
                break;
        }
        break;
    case TConvex::CYLINDER:
        switch (convexB.type)
        {
            case TConvex::BOX:
                Box2CylinderTest(result, convexB, transB, convexA, transA);
                
                result.Swap();
                
                break;
            
            case TConvex::CYLINDER:
                Cylinder2CylinderTest(result, convexA, transA, convexB, transB);

                break;

            case TConvex::SPHERE:
                Cylinder2SphereTest(result, convexA, transA, convexB, transB);

                break;

            case TConvex::TRIANGLE:
                //Sphere2TriangleTest(result, convexA, transA, convexB, transB);
                break;
            case TConvex::TERRAIN:
                Cylinder2TerrainTest(result, convexA, transA, convexB);
                break;

#ifdef USE_OPCODE

            case TConvex::OPCODE_MESH:
                Cylinder2OpcodeTest(result, convexA, transA, convexB, transB);
                break;
                
#endif //USE_OPCODE

            default:
                ASSERT(0);
                break;
        }
        break;

#ifdef USE_OPCODE

    case TConvex::OPCODE_MESH:

        switch(convexB.type)
        {
        case TConvex::BOX:
            Box2OpcodeTest(result, convexB, transB, convexA, transA);
            result.Swap();

            break;
        case TConvex::SPHERE:
            Sphere2OpcodeTest(result, convexB, transB, convexA, transA);
            result.Swap();

            break;
        case TConvex::CYLINDER:
            Cylinder2OpcodeTest(result, convexB, transB, convexA, transA);
            result.Swap();

            break;
        case TConvex::TERRAIN:
            Opcode2TerrainTest(result, convexA, transA, convexB);
            break;

        case TConvex::OPCODE_MESH:
            Opcode2OpcodeTest(result, convexA, transA, convexB, transB);

            break;
        default:
            ASSERT(0);
            break;
        };
        break;
        
#endif //USE_OPCODE

    case TConvex::CONVEXDCD:
    {
        switch (convexB.type)
        {
        case TConvex::CONVEXDCD:

            Convex2ConvexTest(result, convexA, transA, convexB, transB, backupVector);
            
            break;

        case TConvex::BOX:
            
            Box2ConvexTest(result, convexB, transB, convexA, transA, -backupVector);

            result.Swap();

            break;
        
        case TConvex::TERRAIN:

            Convex2TerrainTest(result, convexA, transA, convexB);
            
            break;
        }
        break;

    }
    default:
        ASSERT(0);
        break;
    }
}

/****************************************************************************
*
*   Box2BoxTest
*
****************************************************************************/ 
#if 1
void Box2BoxTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB, const neV3 & backupVector)
{
/*  neCollisionResult dcdresult;

    TestDCD(dcdresult, convexA, transA, convexB, transB, backupVector);


    if (dcdresult.penetrate)
    {
        result.penetrate = true;

        result.depth = dcdresult.depth;

        result.collisionFrame[2] = dcdresult.collisionFrame[2];

        result.contactA = dcdresult.contactA;

        result.contactB = dcdresult.contactB;
    }
    else
    {
        result.penetrate = false;
    }
    
    return;
*/
    ConvexTestResult res;

    BoxTestParam boxParamA;
    BoxTestParam boxParamB;

    boxParamA.convex = &convexA;
    boxParamA.trans = &transA;
    boxParamA.radii[0] = transA.rot[0] * convexA.as.box.boxSize[0];
    boxParamA.radii[1] = transA.rot[1] * convexA.as.box.boxSize[1];
    boxParamA.radii[2] = transA.rot[2] * convexA.as.box.boxSize[2];

    boxParamB.convex = &convexB;
    boxParamB.trans = &transB;
    boxParamB.radii[0] = transB.rot[0] * convexB.as.box.boxSize[0];
    boxParamB.radii[1] = transB.rot[1] * convexB.as.box.boxSize[1];
    boxParamB.radii[2] = transB.rot[2] * convexB.as.box.boxSize[2];

    if (boxParamA.BoxTest(res, boxParamB))
    {
        //return;
        result.penetrate = true;
        
        result.depth = res.depth;

//      result.collisionFrame[0] = res.contactX;
//      result.collisionFrame[1] = res.contactY;
        result.collisionFrame[2] = res.contactNormal;

//      if (res.isEdgeEdge)
        {
            result.contactA = res.contactA;

            result.contactB = res.contactB;
        }
//      else
//      {
//          result.contactA = res.contactA;
//          
//          result.contactB = res.contactB;
//      }
    }
    else
    {
        result.penetrate = false;
    }
//  ASSERT(result.penetrate == dcdresult.penetrate);
}

#else

void Box2BoxTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB, neV3 & backupVector)
{
    Simplex simplex;

    GJKObj gjkObjA, gjkObjB;

    simplex.cache_valid = false;

    simplex.epsilon = 1.0e-6f;

    gjkObjA.half_box.Set(convexA.as.box.boxSize);

    gjkObjB.half_box.Set(convexB.as.box.boxSize);

    gjkObjA.scaleFactor = 1.0f;

    gjkObjB.scaleFactor = 1.0f;

    gjkObjA.type = GJK_BOX;

    gjkObjB.type = GJK_BOX;

    gjkObjA.lw = &transA;

    gjkObjB.lw = &transB;

    gjkObjA.lwrot = &transA.rot;

    gjkObjB.lwrot = &transB.rot;

    f32 envelopeA = convexA.envelope = 0.1f;

    f32 envelopeB = convexB.envelope = 0.1f;

    f32 envelope = envelopeA + envelopeB;

    f32 dist = calc_dist(&simplex, &gjkObjA, &gjkObjB, 1);

    if (dist > 0)
    {
        dist = sqrtf(dist);

        ASSERT(dist > 0.0f);

        if (dist < envelope)
        {
            neV3 pa, pb;

            pa = transA * simplex.closest_pointA;

            pb = transB * simplex.closest_pointB;

            neV3 diff = pa - pb;

            diff.Normalize();

            result.collisionFrame[2] = diff;

            result.depth = envelope - dist;

            result.contactA = pa - diff * envelopeA;

            result.contactB = pb + diff * envelopeB;

            result.penetrate = true;
        }
        else
            result.penetrate = false;
    }
    else
    {
        neV3 posA, posB;

        posA = transA.pos;

        posB = transB.pos;

        f32 dist = 0.0f;

        //simplex.cache_valid = false;

        neV3 bv = backupVector * 10.0f;

        transA.pos += bv;
        
        dist = calc_dist(&simplex, &gjkObjA, &gjkObjB, 1);

        if (dist > 0.0f)
        {
            neV3 pa, pb;

            pa = transA * simplex.closest_pointA;

            pb = transB * simplex.closest_pointB;

            neV3 diff = pa - pb;

            f32 d = diff.Length();

            result.collisionFrame[2] = diff * (1.0f / d);

            transA.pos = posA;

            pa = transA * simplex.closest_pointA;

            diff = pb - pa;

            d = diff.Length();

            result.depth = d + envelope;

            result.penetrate = true;
            
            result.contactA = pa;

            result.contactB = pb;
        }
        else
        {
            //result.penetrate = false;

            //return;
            
            f32 shrink = 0.8f;

            transA.pos = posA;

            simplex.cache_valid = false;

            for (s32 i = 0; i < 5; i++)
            {
                simplex.cache_valid = false;

                gjkObjA.scaleFactor = shrink;

                gjkObjB.scaleFactor = shrink;

                dist = calc_dist(&simplex, &gjkObjA, &gjkObjB, 1);

                if (dist > 0.0f)
                    break;
                
                shrink *= 0.8f;
            }
            if (dist == 0.0f)
            {
                result.penetrate = false;

                return;
            }
            if (!simplex.closest_pointA.IsFinite() ||
                !simplex.closest_pointB.IsFinite() )
            {
                result.penetrate = false;

                return;
            }
            neV3 pa, pb;

            pa = transA * simplex.closest_pointA;

            pb = transB * simplex.closest_pointB;

            neV3 diff = pa - pb;

            diff.Normalize();

            f32 factor;

            if (convexA.boundingRadius > convexB.boundingRadius)
                factor = convexA.boundingRadius;
            else
                factor = convexB.boundingRadius;

            transA.pos += (diff * factor);

            simplex.cache_valid = false;

            ASSERT(transA.pos.IsFinite());

            gjkObjA.scaleFactor = 1.0f;

            gjkObjB.scaleFactor = 1.0f;

            dist = calc_dist(&simplex, &gjkObjA, &gjkObjB, 1);

            ASSERT(dist > 0.0f);

            transA.pos = posA;

            result.contactA = transA * simplex.closest_pointA;

            result.contactB = transB * simplex.closest_pointB;

            result.penetrate = true;

            result.collisionFrame[2] = result.contactB - result.contactA;

            result.depth = result.collisionFrame[2].Length();

            result.collisionFrame[2] *= (1.0f / result.depth);

            result.depth += envelope;
        }
    }
}

#endif
/*
void Convex2ConvexTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB, neV3 & backupVector)
{
    Simplex simplex;

    GJKObj gjkObjA, gjkObjB;

    simplex.cache_valid = false;

    simplex.epsilon = 1.0e-6f;

    //gjkObjA.half_box.Set(convexA.as.box.boxSize);

    //gjkObjB.half_box.Set(convexB.as.box.boxSize);

    gjkObjA.vp = convexA.as.convexMesh.vertices;

    gjkObjA.neighbors = convexA.as.convexMesh.neighbours;

    gjkObjB.vp = convexB.as.convexMesh.vertices;

    gjkObjB.neighbors = convexB.as.convexMesh.neighbours;

    gjkObjA.scaleFactor = 1.0f;

    gjkObjB.scaleFactor = 1.0f;

    gjkObjA.type = GJK_MESH;

    gjkObjB.type = GJK_MESH;

    simplex.hintA = 0;

    simplex.hintB = 0;

    gjkObjA.lw = &transA;

    gjkObjB.lw = &transB;

    gjkObjA.lwrot = &transA.rot;

    gjkObjB.lwrot = &transB.rot;

    f32 envelopeA = convexA.envelope;

    f32 envelopeB = convexB.envelope;

    f32 envelope = envelopeA + envelopeB;

    f32 dist = calc_dist(&simplex, &gjkObjA, &gjkObjB, 1);

    if (dist > 0)
    {
        dist = sqrtf(dist);

        ASSERT(dist > 0.0f);

        if (dist < envelope)
        {
            neV3 pa, pb;

            pa = transA * simplex.closest_pointA;

            pb = transB * simplex.closest_pointB;

            neV3 diff = pa - pb;

            diff.Normalize();

            result.collisionFrame[2] = diff;

            result.depth = envelope - dist;

            result.contactA = pa - diff * envelopeA;

            result.contactB = pb + diff * envelopeB;

            result.penetrate = true;
        }
        else
            result.penetrate = false;
    }
    else
    {
        neV3 posA, posB;

        posA = transA.pos;

        posB = transB.pos;

        f32 dist = 0.0f;

        simplex.cache_valid = false;

        //neV3 bv = posA - posB;
        
        neV3 bv = backupVector;

        bv.Normalize();

        bv *= (convexA.boundingRadius * 100.0f);

        transA.pos += bv;
        
        dist = calc_dist(&simplex, &gjkObjA, &gjkObjB, 1);

        if (0)//dist > 0.0f)
        {
            neV3 pa, pb;

            pa = transA * simplex.closest_pointA;

            pb = transB * simplex.closest_pointB;

            neV3 diff = pa - pb;

            f32 d = diff.Length();

            result.collisionFrame[2] = diff * (1.0f / d);

            transA.pos = posA;

            pa = transA * simplex.closest_pointA;

            diff = pb - pa;

            d = diff.Length();

            result.depth = d + envelope;

            result.penetrate = true;
            
            result.contactA = pa;

            result.contactB = pb;
        }
        else
        {
            //result.penetrate = false;

            //return;
            
            f32 shrink = 0.8f;

            transA.pos = posA;

            simplex.cache_valid = false;

            for (s32 i = 0; i < 5; i++)
            {
                simplex.cache_valid = false;

                gjkObjA.scaleFactor = shrink;

                gjkObjB.scaleFactor = shrink;

                dist = calc_dist(&simplex, &gjkObjA, &gjkObjB, 1);

                if (dist > 0.0f)
                    break;
                
                shrink *= 0.8f;
            }
            if (dist == 0.0f)
            {
                result.penetrate = false;

                return;
            }
            if (!simplex.closest_pointA.IsFinite() ||
                !simplex.closest_pointB.IsFinite() )
            {
                result.penetrate = false;

                return;
            }
            neV3 pa, pb;

            pa = transA * simplex.closest_pointA;

            pb = transB * simplex.closest_pointB;

            neV3 diff = pa - pb;

            diff.Normalize();

            f32 factor;

            if (convexA.boundingRadius > convexB.boundingRadius)
                factor = convexA.boundingRadius;
            else
                factor = convexB.boundingRadius;

            transA.pos = posA + (diff * factor * 10.0f);

            simplex.cache_valid = false;

            ASSERT(transA.pos.IsFinite());

            gjkObjA.scaleFactor = 1.0f;

            gjkObjB.scaleFactor = 1.0f;

            dist = calc_dist(&simplex, &gjkObjA, &gjkObjB, 1);

            ASSERT(dist > 0.0f);

            pa = transA * simplex.closest_pointA;

            pb = transB * simplex.closest_pointB;

            diff = pa - pb;

            diff.Normalize();

            result.collisionFrame[2] = diff;

            transA.pos = posA;

            result.contactA = transA * simplex.closest_pointA;

            result.contactB = pb;//transB * simplex.closest_pointB;

            result.penetrate = true;

            diff = result.contactB - result.contactA;

            result.depth = diff.Length();

            result.depth += envelope;
        }
    }
}

*/

void Convex2ConvexTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB, const neV3 & backupVector)
{
    neCollisionResult dcdresult;

    TestDCD(dcdresult, convexA, transA, convexB, transB, backupVector);


    if (dcdresult.penetrate)
    {
        result.penetrate = true;

        result.depth = dcdresult.depth;

        result.collisionFrame[2] = dcdresult.collisionFrame[2];

        result.contactA = dcdresult.contactA;

        result.contactB = dcdresult.contactB;
    }
    else
    {
        result.penetrate = false;
    }
    
    return;
}
void Box2ConvexTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB, const neV3 & backupVector)
{
    neCollisionResult dcdresult;

    TestDCD(dcdresult, convexA, transA, convexB, transB, backupVector);


    if (dcdresult.penetrate)
    {
        result.penetrate = true;

        result.depth = dcdresult.depth;

        result.collisionFrame[2] = dcdresult.collisionFrame[2];

        result.contactA = dcdresult.contactA;

        result.contactB = dcdresult.contactB;
    }
    else
    {
        result.penetrate = false;
    }
    
    return;
}

/*
void Box2ConvexTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB, neV3 & backupVector)
{
    Simplex simplex;

    GJKObj gjkObjA, gjkObjB;

    simplex.cache_valid = false;

    simplex.epsilon = 1.0e-6f;

    gjkObjA.half_box.Set(convexA.as.box.boxSize);

    //gjkObjB.half_box.Set(convexB.as.box.boxSize);

    gjkObjB.vp = convexB.as.convexMesh.vertices;

    gjkObjB.neighbors = convexB.as.convexMesh.neighbours;

    gjkObjA.scaleFactor = 1.0f;

    gjkObjB.scaleFactor = 1.0f;

    gjkObjA.type = GJK_BOX;

    gjkObjB.type = GJK_MESH;

    simplex.hintB = 0;

    gjkObjA.lw = &transA;

    gjkObjB.lw = &transB;

    gjkObjA.lwrot = &transA.rot;

    gjkObjB.lwrot = &transB.rot;

    f32 envelopeA = 0.0f;//convexA.envelope = 0.05f;

    f32 envelopeB = convexB.envelope;

    f32 envelope = envelopeA + envelopeB;

    f32 dist = calc_dist(&simplex, &gjkObjA, &gjkObjB, 1);

    if (dist > 0)
    {
        dist = sqrtf(dist);

        ASSERT(dist > 0.0f);

        if (dist < envelope)
        {
            neV3 pa, pb;

            pa = transA * simplex.closest_pointA;

            pb = transB * simplex.closest_pointB;

            neV3 diff = pa - pb;

            diff.Normalize();

            result.collisionFrame[2] = diff;

            result.depth = envelope - dist;

            result.contactA = pa - diff * envelopeA;

            result.contactB = pb + diff * envelopeB;

            result.penetrate = true;
        }
        else
            result.penetrate = false;
    }
    else
    {
        neV3 posA, posB;

        posA = transA.pos;

        posB = transB.pos;

        f32 dist = 0.0f;

        //simplex.cache_valid = false;

        neV3 bv = backupVector * 10.0f;

        transA.pos += bv;
        
        dist = calc_dist(&simplex, &gjkObjA, &gjkObjB, 1);

        if (dist > 0.0f)
        {
            neV3 pa, pb;

            pa = transA * simplex.closest_pointA;

            pb = transB * simplex.closest_pointB;

            neV3 diff = pa - pb;

            f32 d = diff.Length();

            result.collisionFrame[2] = diff * (1.0f / d);

            transA.pos = posA;

            pa = transA * simplex.closest_pointA;

            diff = pb - pa;

            d = diff.Length();

            result.depth = d + envelope;

            result.penetrate = true;
            
            result.contactA = pa;

            result.contactB = pb;
        }
        else
        {
            //result.penetrate = false;

            //return;
            
            f32 shrink = 0.8f;

            //transA.pos = posA;

            simplex.cache_valid = false;

            for (s32 i = 0; i < 5; i++)
            {
                simplex.cache_valid = false;

                gjkObjA.scaleFactor = shrink;

                gjkObjB.scaleFactor = shrink;

                dist = calc_dist(&simplex, &gjkObjA, &gjkObjB, 1);

                if (dist > 0.0f)
                    break;
                
                shrink *= 0.8f;
            }
            if (dist == 0.0f)
            {
                result.penetrate = false;

                return;
            }
            if (!simplex.closest_pointA.IsFinite() ||
                !simplex.closest_pointB.IsFinite() )
            {
                result.penetrate = false;

                return;
            }
            neV3 pa, pb;

            pa = transA * simplex.closest_pointA;

            pb = transB * simplex.closest_pointB;

            neV3 diff = pa - pb;

            diff.Normalize();

            f32 factor;

            if (convexA.boundingRadius > convexB.boundingRadius)
                factor = convexA.boundingRadius;
            else
                factor = convexB.boundingRadius;

            transA.pos += (diff * factor);

            simplex.cache_valid = false;

            ASSERT(transA.pos.IsFinite());

            gjkObjA.scaleFactor = 1.0f;

            gjkObjB.scaleFactor = 1.0f;

            dist = calc_dist(&simplex, &gjkObjA, &gjkObjB, 1);

            ASSERT(dist > 0.0f);

            transA.pos = posA;

            result.contactA = transA * simplex.closest_pointA;

            result.contactB = transB * simplex.closest_pointB;

            result.penetrate = true;

            result.collisionFrame[2] = result.contactB - result.contactA;

            result.depth = result.collisionFrame[2].Length();

            result.collisionFrame[2] *= (1.0f / result.depth);

            result.depth += envelope;
        }
    }
}
*/
void BoxTestParam::CalcVertInWorld()
{
    isVertCalc = true;
    
    verts[0] = trans->pos + radii[0] + radii[1] + radii[2];
    verts[1] = trans->pos + radii[0] + radii[1] - radii[2];
    verts[2] = trans->pos + radii[0] - radii[1] + radii[2];
    verts[3] = trans->pos + radii[0] - radii[1] - radii[2];
    verts[4] = trans->pos - radii[0] + radii[1] + radii[2];
    verts[5] = trans->pos - radii[0] + radii[1] - radii[2];
    verts[6] = trans->pos - radii[0] - radii[1] + radii[2];
    verts[7] = trans->pos - radii[0] - radii[1] - radii[2];
}

bool BoxTestParam::BoxTest(ConvexTestResult & result, BoxTestParam & otherBox)
{
    result.depth = 1.e5f;
    result.isEdgeEdge = false;
    result.valid = false;

    if (MeasureVertexFacePeneration(result, otherBox, 0) && //vertex of B with face of A
        MeasureVertexFacePeneration(result, otherBox, 1) &&
        MeasureVertexFacePeneration(result, otherBox, 2))
    {
        result.contactNormal *= -1.0f; // normal points toward A (this)
        neV3 tmp = result.contactA;
        result.contactA = result.contactB;
        result.contactB = tmp;
    }
    else
    {
        return false;
    }

    if (otherBox.MeasureVertexFacePeneration(result, *this, 0) && //vertex of A with face of B
        otherBox.MeasureVertexFacePeneration(result, *this, 1) &&
        otherBox.MeasureVertexFacePeneration(result, *this, 2))
    {
        
    }
    else
    {
        return false;
    }
    
    ConvexTestResult result2;

    result2.valid = false;

    result2.depth = result.depth;

    bool edgeCollided = false;

    if (MeasureEdgePeneration(result2, otherBox, 0, 0) &&
        MeasureEdgePeneration(result2, otherBox, 0, 1) &&
        MeasureEdgePeneration(result2, otherBox, 0, 2) &&
        MeasureEdgePeneration(result2, otherBox, 1, 0) &&
        MeasureEdgePeneration(result2, otherBox, 1, 1) &&
        MeasureEdgePeneration(result2, otherBox, 1, 2) &&
        MeasureEdgePeneration(result2, otherBox, 2, 0) &&
        MeasureEdgePeneration(result2, otherBox, 2, 1) &&
        MeasureEdgePeneration(result2, otherBox, 2, 2) )
    {
        if (result2.valid)
            edgeCollided = true;
    }
    else
    {
        return false;
    }

    if (edgeCollided)
    {
        result2.ComputerEdgeContactPoint(result);
        result.isEdgeEdge = true;
        result.contactNormal = result2.contactNormal * -1.0f;
    }
    else
    {
        return result.valid;
    }
    return result.valid;
}

bool BoxTestParam::MeasureVertexFacePeneration(ConvexTestResult & result, BoxTestParam & otherBox, s32 whichFace)
{
    neV3 me2otherBox;

    me2otherBox = otherBox.trans->pos - trans->pos;

    neV3 direction;

    direction = trans->rot[whichFace];

    neV3 contactPoint;

    contactPoint = otherBox.trans->pos;

    f32 penetrated;

    bool reverse = false;
    
    if ((penetrated = me2otherBox.Dot(direction)) < 0.0f)
    {
        direction = direction * -1.0f;

        reverse = true;
    }
    else
    {
        penetrated = penetrated * -1.0f;
    }

    penetrated += convex->as.box.boxSize[whichFace];

    neV3 progression;

    progression = direction * otherBox.radii;

    neV3 sign;

    sign[0] = progression[0] > 0.0f ? 1.0f: -1.0f;
    sign[1] = progression[1] > 0.0f ? 1.0f: -1.0f;
    sign[2] = progression[2] > 0.0f ? 1.0f: -1.0f;

    penetrated += (progression[0] * sign[0]);
    penetrated += (progression[1] * sign[1]);
    penetrated += (progression[2] * sign[2]);

    contactPoint -= (otherBox.radii[0] * sign[0]);
    contactPoint -= (otherBox.radii[1] * sign[1]);
    contactPoint -= (otherBox.radii[2] * sign[2]);

    if (penetrated <= 0.0f)
        return false;

    if ((penetrated + 0.0001f) < result.depth)
    {
        result.depth = penetrated;
        result.contactA = contactPoint; // contactPoint is vertex of otherBox
        result.contactB = contactPoint + direction * penetrated;
        result.valid = true;
        result.contactNormal = direction;
    }
    else if (neIsConsiderZero(penetrated - result.depth))
    {
        s32 otherAxis1 = neNextDim1[whichFace];
        
        s32 otherAxis2 = neNextDim2[whichFace];

        //check to see if this one fall into the faces
        neV3 sub = contactPoint - trans->pos;

        f32 dot = neAbs(sub.Dot(trans->rot[otherAxis1]));

        if (dot > (convex->as.box.boxSize[otherAxis1] * 1.001f))
            return true;// not false ???? no it is true!!! 

        dot = neAbs(sub.Dot(trans->rot[otherAxis2]));

        if (dot > (convex->as.box.boxSize[otherAxis2] * 1.001f))
            return true;// not false ???? no it is true!!! 

        result.depth = penetrated;
        result.contactA = contactPoint;
        result.contactB = contactPoint + direction * penetrated;
        result.valid = true;
        result.contactNormal = direction;
    }
    return true;
}

neBool BoxTestParam::MeasureEdgePeneration(ConvexTestResult & result, BoxTestParam & otherBox, s32 dim1, s32 dim2)
{
    neV3 contactA = trans->pos;

    neV3 contactB = otherBox.trans->pos;
    
    neV3 contactNormal = trans->rot[dim1].Cross(otherBox.trans->rot[dim2]);

    f32 len = contactNormal.Length();

    if (neIsConsiderZero(len))
        return true;

    contactNormal *= (1.0f / len);

    neV3 me2OtherBox = otherBox.trans->pos - trans->pos;

    f32 penetrated = me2OtherBox.Dot(contactNormal);

    bool reverse = false;

    if (penetrated < 0.0f)
    {
        contactNormal = contactNormal * -1.0f;

        reverse = true;
    }
    else
        penetrated = penetrated * -1.0f;

    f32 progression[4];

    s32 otherAxisA1 = (dim1 + 1) % 3;
    s32 otherAxisA2 = (dim1 + 2) % 3;
    s32 otherAxisB1 = (dim2 + 1) % 3;
    s32 otherAxisB2 = (dim2 + 2) % 3;

    progression[0] = radii[otherAxisA1].Dot(contactNormal);
    progression[1] = radii[otherAxisA2].Dot(contactNormal);
    progression[2] = otherBox.radii[otherAxisB1].Dot(contactNormal);
    progression[3] = otherBox.radii[otherAxisB2].Dot(contactNormal);

    f32 sign[4];

    sign[0] = progression[0] > 0.0f ? 1.0f: -1.0f;
    sign[1] = progression[1] > 0.0f ? 1.0f: -1.0f;
    sign[2] = progression[2] > 0.0f ? 1.0f: -1.0f;
    sign[3] = progression[3] > 0.0f ? 1.0f: -1.0f;

    penetrated += (progression[0] * sign[0]);
    penetrated += (progression[1] * sign[1]);
    penetrated += (progression[2] * sign[2]);
    penetrated += (progression[3] * sign[3]);

    contactA += (radii[otherAxisA1] * sign[0]);
    contactA += (radii[otherAxisA2] * sign[1]);
    contactB -= (otherBox.radii[otherAxisB1] * sign[2]);
    contactB -= (otherBox.radii[otherAxisB2] * sign[3]);

    if(penetrated <= 0.0f)
        return false;

    if (penetrated < result.depth)
    {
        result.depth = penetrated;

        result.valid = true;

        result.edgeA[0] = contactA + (radii[dim1]);
        result.edgeA[1] = contactA - (radii[dim1]);

        result.edgeB[0] = contactB + (otherBox.radii[dim2]);
        result.edgeB[1] = contactB - (otherBox.radii[dim2]);

        result.contactA = contactA;
        result.contactB = contactB;
/*      
        if (reverse)
            result.contactX = trans->rot[dim1];
        else
            result.contactX = trans->rot[dim1];

        result.contactY = contactNormal.Cross(result.contactX);
*/      result.contactNormal = contactNormal;
    }
    return true;
}

neBool ConvexTestResult::ComputerEdgeContactPoint(ConvexTestResult & res)
{
    f32 d1343, d4321, d1321, d4343, d2121;
    f32 numer, denom, au, bu;
    
    neV3 p13;
    neV3 p43;
    neV3 p21;
//  neV3 diff;

    p13 = (edgeA[0]) - (edgeB[0]);
    p43 = (edgeB[1]) - (edgeB[0]);

    if ( p43.IsConsiderZero() )
    {
        goto ComputerEdgeContactPoint_Exit;
    }
    
    p21 = (edgeA[1]) - (edgeA[0]);

    if ( p21.IsConsiderZero() )
    {
        goto ComputerEdgeContactPoint_Exit;
    }
    
    d1343 = p13.Dot(p43);
    d4321 = p43.Dot(p21);
    d1321 = p13.Dot(p21);
    d4343 = p43.Dot(p43);
    d2121 = p21.Dot(p21);

    denom = d2121 * d4343 - d4321 * d4321;   

    if (neAbs(denom) < NE_ZERO) 
        goto ComputerEdgeContactPoint_Exit;

    numer = d1343 * d4321 - d1321 * d4343;
    au = numer / denom;   
    bu = (d1343 + d4321 * (au)) / d4343;

    if (au < 0.0f || au >= 1.0f)
        goto ComputerEdgeContactPoint_Exit;
    
    if (bu < 0.0f || bu >= 1.0f)
        goto ComputerEdgeContactPoint_Exit;

    {
        neV3 tmpv;

        tmpv = p21 * au;
        res.contactA = (edgeA[0]) + tmpv;

        tmpv = p43 * bu;
        res.contactB = (edgeB[0]) + tmpv;
    }

//  diff = (res.contactA) - (res.contactB);
    res.depth = depth;//sqrtf(diff.Dot(diff));

    return true;

ComputerEdgeContactPoint_Exit:
    //res.contactA = contactA;
    //res.contactB = contactB;
    //diff = (res.contactA) - (res.contactB);
    res.depth = depth;//sqrtf(diff.Dot(diff));
    return false;
}

neBool ConvexTestResult::ComputerEdgeContactPoint2(f32 & au, f32 & bu)
{
    f32 d1343, d4321, d1321, d4343, d2121;
    f32 numer, denom;
    
    neV3 p13;
    neV3 p43;
    neV3 p21;
    neV3 diff;

    p13 = (edgeA[0]) - (edgeB[0]);
    p43 = (edgeB[1]) - (edgeB[0]);

    if ( p43.IsConsiderZero() )
    {
        valid = false;
        goto ComputerEdgeContactPoint2_Exit;
    }
    
    p21 = (edgeA[1]) - (edgeA[0]);

    if ( p21.IsConsiderZero() )
    {
        valid = false;
        goto ComputerEdgeContactPoint2_Exit;
    }
    
    d1343 = p13.Dot(p43);
    d4321 = p43.Dot(p21);
    d1321 = p13.Dot(p21);
    d4343 = p43.Dot(p43);
    d2121 = p21.Dot(p21);

    denom = d2121 * d4343 - d4321 * d4321;   

    if (neAbs(denom) < NE_ZERO) 
    {
        valid = false;

        goto ComputerEdgeContactPoint2_Exit;
    }

    numer = d1343 * d4321 - d1321 * d4343;
    au = numer / denom;   
    bu = (d1343 + d4321 * (au)) / d4343;

    if (au < 0.0f || au >= 1.0f)
    {
        valid = false;
    }
    else if (bu < 0.0f || bu >= 1.0f)
    {
        valid = false;
    }
    else
    {
        valid = true;
    }
    {
        neV3 tmpv;

        tmpv = p21 * au;
        contactA = (edgeA[0]) + tmpv;

        tmpv = p43 * bu;
        contactB = (edgeB[0]) + tmpv;
    }

    diff = contactA - contactB;
    
    depth = sqrtf(diff.Dot(diff));

    return true;

ComputerEdgeContactPoint2_Exit:

    return false;
}

neBool BoxTestParam::LineTest(ConvexTestResult & res, neV3 & point1, neV3 & point2)
{
    return false;   
}


/**constraint.cpp**/

//#include <stdio.h>

//#include "tokamak.h"
//#include "containers.h"
//#include "scenery.h"
//#include "collision.h"
//#include "constraint.h"
//#include "rigidbody.h"
//#include "scenery.h"
//#include "stack.h"
//#include "simulator.h"
//#include "message.h"

//extern void DrawLine(const neV3 & colour, neV3 * startpoint, s32 count);

#pragma inline_recursion( on )
#pragma inline_depth( 50 )

f32 AngleBetweenVector(const neV3 & va, const neV3 & vb, const neV3 & normal)
{
    // rotation from va to vb

    neV3 ra = va;
    
    ra.RemoveComponent(normal);

    ra.Normalize();

    neV3 rb = vb;
    
    rb.RemoveComponent(normal);

    rb.Normalize();

    f32 dot = ra.Dot(rb);

    if (neIsConsiderZero(dot - 1.0f))
    {
        return 0.0f;
    }
    if (neIsConsiderZero(dot + 1.0f))
    {
        return NE_PI;
    }
    neV3 cross = ra.Cross(rb);

    f32 dot2 = cross.Dot(normal);

    if (dot2 > 0.0f)
    {
        return acosf(dot);
    }
    else
    {
        return 2.0f * NE_PI - acosf(dot);
    }
}

neQ GetQuatRotateTo(const neV3 & fromV, const neV3 & toV, const neV3 & axis)
{
    neQ ret;

    f32 dot = fromV.Dot(toV);

    if (neIsConsiderZero(dot - 1.0f))
    {
        ret.Identity();
    }
    else if (neIsConsiderZero(dot + 1.0f))
    {
        ret.Set(NE_PI, axis);
    }
    else
    {
        neV3 cross = fromV.Cross(toV);

        cross.Normalize();

        f32 angle;

        angle = acosf(dot);

        ret.Set(angle, cross);
    }

    return ret;
}

f32 DistanceFromPlane(const neV3 & point, const neV3 & normal, const neV3 & pointOnPlane, neV3 & projected)
{
    neV3 diff = point - pointOnPlane;

    f32 dot = diff.Dot(normal);

    projected = point - normal * dot;

    return dot;
}

void _neConstraint::GeneratePointsFromFrame()
{
    f32 hingeHalfLength = jointLength * 0.5f;

    switch (type)
    {
    case neJoint::NE_JOINT_BALLSOCKET:
        
        cpointsA[0].PtBody() = frameA.pos;

        cpointsB[0].PtBody() = frameB.pos;

        break;

    case neJoint::NE_JOINT_HINGE:
        
        cpointsA[0].PtBody() = frameA.pos + frameA.rot[1] * hingeHalfLength;

        cpointsA[1].PtBody() = frameA.pos - frameA.rot[1] * hingeHalfLength;

        cpointsB[0].PtBody() = frameB.pos + frameB.rot[1] * hingeHalfLength;

        cpointsB[1].PtBody() = frameB.pos - frameB.rot[1] * hingeHalfLength;

        break;

    case neJoint::NE_JOINT_SLIDE:
        
        cpointsA[0].PtBody() = frameA.pos + frameA.rot[1] * hingeHalfLength;

        cpointsA[1].PtBody() = frameA.pos - frameA.rot[1] * hingeHalfLength;

        break;

    default:
        ASSERT(0);
        break;
    }
}

void _neConstraint::Enable(neBool yes)
{
    if (!bodyA)
        return;

    if (enable && yes)
        return;

    if (!enable && !yes)
        return;

    if (alreadySetup)
    {
        enable = yes;

        if (bodyA->status == neRigidBody_::NE_RBSTATUS_IDLE)
        {
            bodyA->WakeUp();
        }
        if (bodyB && bodyB->AsRigidBody())
        {
            if (bodyB->AsRigidBody()->status == neRigidBody_::NE_RBSTATUS_IDLE)
            {
                bodyB->AsRigidBody()->WakeUp();
            }
        }
    }
    else
    {
        if (yes)
        {
            alreadySetup = true;
            
            GeneratePointsFromFrame();

            AddToRigidBody();

            enable = true;

            if (bodyA->status == neRigidBody_::NE_RBSTATUS_IDLE)
            {
                bodyA->WakeUp();
            }
            if (bodyB && bodyB->AsRigidBody())
            {
                if (bodyB->AsRigidBody()->status == neRigidBody_::NE_RBSTATUS_IDLE)
                {
                    bodyB->AsRigidBody()->WakeUp();
                }
            }
        }
        else
        {
            enable = false;
        }
    }
}

void _neConstraint::AddToRigidBody()
{
    neConstraintHeader * header = NULL;

    neRigidBody_ * rb = bodyB ? bodyB->AsRigidBody() : NULL;

    neCollisionBody_ * cb =bodyB ? bodyB->AsCollisionBody() : NULL;

    if (bodyA->GetConstraintHeader())
    {
        header = bodyA->GetConstraintHeader();
    }

    if (!bodyB)
    {
        if (header == NULL)
        {
            header = sim->NewConstraintHeader();

            if (!header)
            {
                if (bodyA->sim->logLevel >= neSimulator::LOG_OUTPUT_LEVEL_ONE)
                {

                    sprintf(bodyA->sim->logBuffer, MSG_CONSTRAINT_HEADER_FULL);
                    bodyA->sim->LogOutput(neSimulator::LOG_OUTPUT_LEVEL_ONE);
                }
                return;
            }

            header->Add(this);

            bodyA->SetConstraintHeader(header);

            header->bodies.Add(&bodyA->constraintHeaderItem);
        }
        else
        {
            header->Add(this);
        }
    }
    else
    {   
        neConstraintHeader * bHeader = NULL;

        if (cb && cb->GetConstraintHeader())
        {
            bHeader = cb->GetConstraintHeader();
        }
        else if (rb && rb->GetConstraintHeader())
        {
            bHeader = rb->GetConstraintHeader();
        }
        if (bHeader)
        {
            if (header == NULL)
            {
                bodyA->SetConstraintHeader(bHeader);

                //header = rb->GetConstraintHeader();

                bHeader->Add(this);

                bHeader->bodies.Add(&bodyA->constraintHeaderItem);
            }
            else
            {
                //if (rb->GetConstraintHeader() == 
                //  bodyA->GetConstraintHeader())
                if (bHeader == header)
                {
                    header->Add(this);
                }
                else
                {
                    //merge
                    //ASSERT(0);
                    header->Add(this);

                    while (bHeader->head != NULL)
                    {
                        _neConstraint * c = bHeader->head;

                        bHeader->Remove(bHeader->head);

                        header->Add(c);
                    }
                    neBodyHandle * bodyHandle = bHeader->bodies.GetHead();

                    while (bodyHandle)
                    {
                        neBodyHandle * bBodyHandle = bodyHandle;
                        
                        bodyHandle = bodyHandle->next;

                        bBodyHandle->Remove();

                        header->bodies.Add(bBodyHandle);

                        bBodyHandle->thing->SetConstraintHeader(header);
                    }
                    sim->constraintHeaders.Dealloc(bHeader);
                }
            }
        }
        else
        {
            if (header == NULL)
            {
                //create new header
                header = sim->NewConstraintHeader();

                if (!header)
                {
                    if (bodyA->sim->logLevel >= neSimulator::LOG_OUTPUT_LEVEL_ONE)
                    {
                        sprintf(bodyA->sim->logBuffer, MSG_CONSTRAINT_HEADER_FULL);
                        bodyA->sim->LogOutput(neSimulator::LOG_OUTPUT_LEVEL_ONE);
                    }
                    return;
                }
                
                bodyA->SetConstraintHeader(header);

                header->bodies.Add(&bodyA->constraintHeaderItem);

                header->Add(this);

                bodyB->SetConstraintHeader(header);

                header->bodies.Add(&bodyB->constraintHeaderItem);
            }
            else
            {
                header->Add(this);

                bodyB->SetConstraintHeader(header);

                header->bodies.Add(&bodyB->constraintHeaderItem);
            }
        }
    }
    bodyA->GetConstraintHeader()->flag |= neConstraintHeader::FLAG_NEED_SETUP;
}

void _neConstraint::InfiniteMassB(neBool yes)
{
    infiniteMassB = yes;
}

void _neConstraint::Reset()
{
    bodyA = NULL;
    bodyB = NULL;
    enable = false;
    infiniteMassB = false;
    type = neJoint::NE_JOINT_BALLSOCKET;
    accuracy = -1.0f;//0.5f;
    sim = NULL;
    controllers = NULL;
    controllerCursor = NULL;
    frameA.rot.SetIdentity();
    frameA.pos.SetZero();
    frameB.rot.SetIdentity();
    frameB.pos.SetZero();
    jointLength = 1.0f;
    iteration = -1;
    jointDampingFactor = 0.0f;
    
    limitStates[0].Reset(this);
    limitStates[1].Reset(this, 1);

    motors[0].Reset();
    motors[1].Reset();

    alreadySetup = false;

/*  limitConstraintCount = 0;
    for (s32 i = 0; i < 2; i++)
        for (s32 j = 0; j < 2; j++)
            limitRigidConstraint[i][j] = NULL;

    enableLimitMiniConstraint = false;

    applyLimitImpulse = false;
*/
    bodyAHandle.thing = this;

    bodyBHandle.thing = this;

//  cres[0] = cres[1] = NULL;
}

void _neConstraint::SetType(neJoint::ConstraintType t)
{
    switch (t)  
    {
    case neJoint::NE_JOINT_BALLSOCKET:
        type = t;
        pointCount = 1;
        break;
    case neJoint::NE_JOINT_HINGE:
        type = t;
        pointCount = 2;
        break;
    case neJoint::NE_JOINT_SLIDE:
        type = t;
        pointCount = 2;
        break;
    }
}

neController * _neConstraint::AddController(neJointControllerCallback * jc, s32 period)
{
    if (!jc)
        return NULL;

    neController * c = sim->controllerHeap.Alloc(1);

    if (!c)
    {
        sprintf(sim->logBuffer, MSG_CONTROLLER_FULL);
        
        sim->LogOutput(neSimulator::LOG_OUTPUT_LEVEL_ONE);

        return NULL;
    }
    if (!controllers)
    {
        controllers = c;
    }
    else
    {
        ((neControllerItem *)controllers)->Append((neControllerItem *)c);       
    }
    c->rb = NULL;

    c->constraint = this;

    c->rbc = NULL;

    c->jc = jc;

    c->period = period;

    c->count = period;

    c->forceA.SetZero();

    c->torqueA.SetZero();

    c->forceB.SetZero();

    c->torqueB.SetZero();

    return c;
}

void _neConstraint::BeginIterateController()
{
    controllerCursor = (neControllerItem *)controllers;
}

neController * _neConstraint::GetNextController()
{
    if (!controllerCursor)
        return NULL;

    neController * ret = (neController *)controllerCursor;

    controllerCursor = controllerCursor->next;

    return ret;
}

void _neConstraint::UpdateController()
{
    neControllerItem * ci = (neControllerItem *)controllers;

    while (ci)
    {
        neController * con = (neController *) ci;

        ci = ci->next;

        if (con->count == 0)
        {
            ASSERT(con->jc);
            
            con->jc->ConstraintControllerCallback((neJointController*)con, sim->_currentTimeStep);
            
            con->count = con->period;
        }
        else
        {
            con->count--;
        }
        con->constraint->bodyA->cforce += con->forceA;

        con->constraint->bodyA->ctorque += con->torqueA;

        if (con->constraint->bodyB)
        {
            neRigidBody_* bb = (neRigidBody_*)con->constraint->bodyB->AsRigidBody();

            if (bb)
            {
                bb->cforce += con->forceB;

                bb->ctorque += con->torqueB;
            }
        }
    }
}

void _neConstraint::UpdateConstraintPoint()
{
    ASSERT(type == neJoint::NE_JOINT_BALLSOCKET ||
            type == neJoint::NE_JOINT_HINGE ||
            type == neJoint::NE_JOINT_SLIDE);

    ASSERT(bodyA);

    neRigidBody_* rbodyB = NULL;

    neCollisionBody_* cbodyB = NULL;

    if (bodyB)
    {
        rbodyB = bodyB->AsRigidBody();

        cbodyB = bodyB->AsCollisionBody();
    }

    for (s32 i = 0; i < pointCount; i++)
    {
        if (type == neJoint::NE_JOINT_BALLSOCKET || 
            type == neJoint::NE_JOINT_HINGE)
        {
            cpointsA[i].PtWorld() = bodyA->State().b2w * cpointsA[i].PtBody();

            if (rbodyB)
            {
                cpointsB[i].PtWorld() = rbodyB->State().b2w * cpointsB[i].PtBody();
            }
            else if (cbodyB)
            {
                cpointsB[i].PtWorld() = cbodyB->b2w * cpointsB[i].PtBody();
            }
            else
            {
                cpointsB[i].PtWorld() = cpointsB[i].PtBody();
            }
        }
        else if (type == neJoint::NE_JOINT_SLIDE)
        {
            cpointsA[i].PtWorld() = bodyA->State().b2w * cpointsA[i].PtBody();
        }

        ASSERT(cpointsA[i].PtWorld().IsFinite());

        ASSERT(cpointsB[i].PtWorld().IsFinite());
    }
}

/****************************************************************************
*
*   neConstrain::FindGreatest
*
****************************************************************************/ 

void _neConstraint::FindGreatest()
{
    neRigidBody_ * rbodyB = NULL;
    
    neCollisionBody_ * cbodyB = NULL;

    if (bodyB)
    {
        rbodyB = bodyB->AsRigidBody();

        cbodyB = bodyB->AsCollisionBody();
    }
    switch (type)
    {
    case neJoint::NE_JOINT_BALLSOCKET:
    case neJoint::NE_JOINT_HINGE:
        {

            for (s32 i = 0; i < pointCount; i++)
            {
                neCollisionResult * cresult = bodyA->sim->cresultHeap.Alloc(0);

                cresult->contactA = cpointsA[i].PtWorld() - bodyA->GetPos();

                if (rbodyB)
                {
                    cresult->contactB = cpointsB[i].PtWorld() - rbodyB->GetPos();
                }
                else
                {
                    cresult->contactB = cpointsB[i].PtWorld();

                    cresult->contactBWorld = cpointsB[i].PtWorld();
                }

                cresult->contactAWorld = cpointsA[i].PtWorld() - cpointsB[i].PtWorld();
                cresult->depth = cresult->contactAWorld.Length();
                cresult->bodyA = bodyA;
                cresult->bodyB = bodyB;
                //cresult->relativeSpeed = relSpeed;
                cresult->impulseType = IMPULSE_CONSTRAINT;
                cresult->PrepareForSolver();
            }
        }   
        if (limitStates[0].enableLimit || 
            limitStates[1].enableLimit)
        {
            CheckLimit();
        }
        break;

    case neJoint::NE_JOINT_SLIDE:
        {
            for (s32 i = 0; i < pointCount; i++)
            {
                neCollisionResult * cresult = bodyA->sim->cresultHeap.Alloc(0);

                cresult->contactA = cpointsA[i].PtWorld() - bodyA->GetPos();

                neV3 diff = cpointsA[i].PtWorld() - frameBWorld.pos;

                f32 dot = diff.Dot(frameBWorld.rot[1]);

                cpointsB[i].PtWorld() = dot * frameBWorld.rot[1] + frameBWorld.pos;

                if (rbodyB)
                {
                    cresult->contactB = cpointsB[i].PtWorld() - rbodyB->GetPos();
                }
                else
                {
                    cresult->contactB = cpointsB[i].PtWorld();

                    cresult->contactBWorld = cpointsB[i].PtWorld();
                }
                cresult->contactAWorld = cpointsA[i].PtWorld() - cpointsB[i].PtWorld();
                cresult->finalRelativeSpeed = cresult->contactAWorld.Length();
                cresult->contactBWorld = frameBWorld.rot[1];
                cresult->bodyA = bodyA;
                cresult->bodyB = bodyB;
                cresult->impulseType = IMPULSE_SLIDER;
                cresult->PrepareForSolver();
            }
            if (limitStates[0].enableLimit || 
                limitStates[1].enableLimit)
            {
                CheckLimit();
            }
        }
        break;
    
    default:
        break;
    }
    if (bodyA->stackInfo)
    {
        bodyA->AddContactConstraint();
    }
    if (rbodyB && rbodyB->stackInfo)
    {
        rbodyB->AddContactConstraint();
    }
    if (motors[0].enable)
        motors[0].PrepareForSolver(this);

    if (motors[1].enable)
        motors[1].PrepareForSolver(this);
}
/*
void _neConstraint::SetupLimitCollision()
{
    neRigidBody_ * rbodyB = NULL;
    
    neCollisionBody_ * cbodyB = NULL;

    if (bodyB)
    {
        rbodyB = bodyB->AsRigidBody();

        cbodyB = bodyB->AsCollisionBody();
    }

    frameAWorld = bodyA->State().b2w * frameA;

    frameBWorld = GetFrameBWorld();
}
*/
void _neConstraint::DrawCPointLine()
{
#if 0
    neV3 points[2];
    neV3 color;

//  if (limitConstraintCount == 1)
//      return;

    s32 i;
    for (i = 0; i < pointCount; i++)
    {
        points[0] = limitPt[i][0].PtWorld();

        points[1] = limitPt[i][1].PtWorld();

        DrawLine(color, points, 2); 
    }

    for (i = 0; i < pointCount; i++)
    {
        for (s32 j = 0; j < limitConstraintCount; j++)
        {
            neMiniConstraintItem * item = (neMiniConstraintItem *)limitRigidConstraint[i][j];

            while (item)
            {
                neMiniConstraint * mc = (neMiniConstraint *) item;

                item = item->next;

                points[0] = *mc->pointA;

                points[1] = *mc->pointB;

                DrawLine(color, points, 2); 
            }
        }
    }
#endif
}

neT3 _neConstraint::GetFrameBWorld()
{
    neT3 ret;

    neRigidBody_ * rb = NULL;

    if (bodyB)
    {
        rb = bodyB->AsRigidBody();

        if (rb)
        {
            ret = rb->State().b2w * frameB;
        }
        else
        {
            ret = bodyB->AsCollisionBody()->b2w * frameB;
        }
    }
    else
    {
        ret = frameB;
    }
    return ret;
}

neT3 _neConstraint::GetBodyB2W()
{
    neT3 ret;

    neRigidBody_ * rb = NULL;

    if (bodyB)
    {
        rb = bodyB->AsRigidBody();

        if (rb)
        {
            ret = rb->State().b2w;
        }
        else
        {
            ret = bodyB->AsCollisionBody()->b2w;
        }
    }
    else
    {
        ret.SetIdentity();
    }
    return ret;
}

void _neConstraint::CheckLimit()
{
    if (type == neJoint::NE_JOINT_BALLSOCKET || type == neJoint::NE_JOINT_HINGE)
    {
        if (limitStates[0].enableLimit)
            limitStates[0].CheckLimitPrimary();

        if (type == neJoint::NE_JOINT_BALLSOCKET && 
            limitStates[1].enableLimit)
        {
            limitStates[1].CheckLimitSecondary();
        }
    }
    else if (type == neJoint::NE_JOINT_SLIDE)
    {
        if (limitStates[0].enableLimit)
            limitStates[0].CheckLimitPrimarySlider();
//      if (limitStates[1].enableLimit)
//          limitStates[1].CheckLimitSecondarySlider();
    }
}

void _neConstraint::UpdateCurrentPosition()
{
    neRigidBody_ * rb = NULL;

    if (bodyB && bodyB->AsRigidBody())
    {
        rb = bodyB->AsRigidBody();
    }
    frameAWorld = bodyA->State().b2w * frameA;

    frameBWorld = GetFrameBWorld();

    f32 dot;

    if (type == neJoint::NE_JOINT_HINGE)
    {
        limitStates[0].limitAxis = frameAWorld.rot[1] + frameBWorld.rot[1];

        limitStates[0].limitAxis.Normalize();

        f32 dot = frameAWorld.rot[0].Dot(frameBWorld.rot[0]);

        if (neIsConsiderZero(dot - 1.0f)) // dot = 1
        {
            pos = 0.0f;

            pos2 = 0.0f;
        }
        else if (neIsConsiderZero(dot + 1.0f)) // dot = -1
        {
            pos = NE_PI;

            pos2 = -NE_PI;
        }
        else
        {
            neV3 cross = frameBWorld.rot[0].Cross(frameAWorld.rot[0]);

            f32 len = cross.Length();

            cross *= (1.0f / len);

            f32 dot2 = limitStates[0].limitAxis.Dot(cross);

            if (dot2 > 0.0f)
            {
                pos = acosf(dot);

                pos2 = pos;
            }
            else
            {
                f32 t = acosf(dot);

                pos = 2.0f * NE_PI - t;

                pos2 = -t; 
            }
        }
    }
    else if (type == neJoint::NE_JOINT_BALLSOCKET)
    {
        dot = frameAWorld.rot[0].Dot(frameBWorld.rot[0]);

        if (neIsConsiderZero(dot - 1.0f)) // dot = 1
        {
            pos = 0.0f;
            
            limitStates[0].limitAxis = frameAWorld.rot[1];
        }
        else if (neIsConsiderZero(dot + 1.0f)) // = -1
        {
            pos = NE_PI;

            limitStates[0].limitAxis = frameAWorld.rot[1];
        }
        else
        {
            limitStates[0].limitAxis = frameBWorld.rot[0].Cross(frameAWorld.rot[0]);

            f32 len = limitStates[0].limitAxis.Length();

            ASSERT(!neIsConsiderZero(len));
    
            limitStates[0].limitAxis *= (1.0f / len);

            pos = acosf(dot);
        }
    }
    else if (type == neJoint::NE_JOINT_SLIDE)
    {
        limitStates[0].limitAxis = frameAWorld.rot[1] + frameBWorld.rot[1];

        limitStates[0].limitAxis.Normalize();

        neV3 diff = frameAWorld.pos - frameBWorld.pos;

        pos = diff.Dot(frameBWorld.rot[1]);
    }
    else
    {
        return;
    }
}

#if 1

void neLimitState::CheckLimitSecondary()
{
    ASSERT(limitType == 1);

    applyLimitImpulse = false;

    f32 dot;

    f32 ang = -constr->pos;

    neQ quat;

    quat.Set(ang, constr->limitStates[0].limitAxis);

    neV3 zaAdjust;

    zaAdjust = quat * constr->frameAWorld.rot[2];

    f32 angle;

    dot = constr->frameBWorld.rot[2].Dot(zaAdjust);

    if (neIsConsiderZero(dot - 1.0f)) // dot == 1
    {
        angle = 0.0f;
    }
    else if (neIsConsiderZero(dot + 1.0f))
    {
        angle = NE_PI;
    }
    else
    {
        neV3 cross = constr->frameBWorld.rot[2].Cross(zaAdjust);

        f32 len = cross.Length();

        cross *= (1.0f / len);

        f32 dot2 = constr->frameBWorld.rot[0].Dot(cross);

        if (dot2 >= 0.0f)
        {
            angle = acosf(dot);
        }
        else
        {
            f32 t = acosf(dot);

            angle = - t;
        }
    }

    if (angle > lowerLimit)
    {
        limitAngularPenetration = (angle - lowerLimit);

        limitAngularPenetration2 = limitAngularPenetration;

        applyLimitImpulse = true;
    }
    else if (angle < -lowerLimit)
    {
        limitAngularPenetration = (angle + lowerLimit);

        limitAngularPenetration2 = limitAngularPenetration;

        applyLimitImpulse = true;
    }
    else
    {
        return;
    }
    neCollisionResult * cresult = constr->bodyA->sim->cresultHeap.Alloc(0);

    cresult->bodyA = constr->bodyA;

    cresult->bodyB = constr->bodyB;

    cresult->impulseType = IMPULSE_ANGULAR_LIMIT_SECONDARY;

    cresult->contactBBody = constr->frameBWorld.rot[0];

    cresult->contactABody = constr->frameAWorld.rot[0];

    cresult->depth = limitAngularPenetration;

    cresult->impulseScale = limitAngularPenetration2;

    cresult->k = quat.BuildMatrix3();

    //cresult->k = constr->bodyB->GetB2W().rot * cresult->k;

    cresult->PrepareForSolver();
}

#else

void neLimitState::CheckLimitSecondary()
{
    ASSERT(limitType == 1);

    applyLimitImpulse = false;

    f32 dot = constr->frameAWorld.rot[1].Dot(constr->frameBWorld.rot[1]);

    neV3 target;

    f32 ang;

    if (dot >= 0.0f)
    {
        target = constr->frameAWorld.rot[0];

        ang = -constr->pos;
    }
    else
    {
        applyLimitImpulse = false;

        return;
    }

    neQ quat;

    quat.Set(ang, constr->limitStates[0].limitAxis);

    neV3 zaAdjust;

    zaAdjust = quat * constr->frameAWorld.rot[2];

    f32 angle;

    dot = constr->frameBWorld.rot[2].Dot(zaAdjust);

    if (neIsConsiderZero(dot - 1.0f)) // dot == 1
    {
        angle = 0.0f;
    }
    else if (neIsConsiderZero(dot + 1.0f))
    {
        angle = NE_PI;
    }
    else
    {
        neV3 cross = constr->frameBWorld.rot[2].Cross(zaAdjust);

        f32 len = cross.Length();

        cross *= (1.0f / len);

        f32 dot2 = constr->frameAWorld.rot[0].Dot(cross);

        if (dot2 >= 0.0f)
        {
            angle = acosf(dot);
        }
        else
        {
            f32 t = acosf(dot);

            angle = - t;
        }
    }

    if (angle > lowerLimit)
    {
        limitAngularPenetration = (angle - lowerLimit);

        limitAngularPenetration2 = limitAngularPenetration;

        applyLimitImpulse = true;
    }
    else if (angle < -lowerLimit)
    {
        limitAngularPenetration = (angle + lowerLimit);

        limitAngularPenetration2 = limitAngularPenetration;

        applyLimitImpulse = true;
    }
    else
    {
        return;
    }
    neCollisionResult * cresult = constr->bodyA->sim->cresultHeap.Alloc(0);

    cresult->bodyA = constr->bodyA;

    cresult->bodyB = constr->bodyB;

    cresult->impulseType = IMPULSE_ANGULAR_LIMIT_SECONDARY;

    cresult->contactBBody = constr->frameBWorld.rot[0];

    cresult->contactABody = constr->frameAWorld.rot[0];

    cresult->depth = limitAngularPenetration;

    cresult->impulseScale = limitAngularPenetration2;

    cresult->PrepareForSolver();
}
#endif

void neLimitState::CheckLimitPrimary()
{
    applyLimitImpulse = false;

//  neRigidBody_ * rb = NULL;

//  if (constr->bodyB && constr->bodyB->AsRigidBody())
//  {
//      rb = constr->bodyB->AsRigidBody();
//  }

    ASSERT(neIsFinite(constr->pos));

    ASSERT(neIsFinite(constr->pos2));

    f32 rotation;

    if (lowerLimit > 0.0f || constr->type == neJoint::NE_JOINT_BALLSOCKET)
    {
        //if (upperLimit >= 0.0f) //upperlimit must also be positive
        {
            if (constr->pos < lowerLimit)
            {
                lowerLimitOn = true;

                rotation = -(lowerLimit - constr->pos);
            }
            else if (constr->pos > upperLimit)
            {
                lowerLimitOn = false;

                rotation = constr->pos - upperLimit;
            }
            else
            {
                return;
            }
        }
    }
    else
    {
        if (upperLimit >= 0.0f)
        {
            if (constr->pos2 < lowerLimit)
            {
                lowerLimitOn = true;

                rotation = -(lowerLimit - constr->pos2);
            }
            else if (constr->pos2 > upperLimit)
            {
                lowerLimitOn = false;

                rotation = constr->pos2 - upperLimit;
            }
            else
            {
                return;
            }
        }
        else
        {
            if (constr->pos2 < lowerLimit)
            {
                lowerLimitOn = true;

                rotation = -(lowerLimit - constr->pos2);
            }
            else if (constr->pos2 > upperLimit)
            {
                lowerLimitOn = false;

                rotation = constr->pos2 - upperLimit;
            }
            else
            {
                return;
            }
        }
    }
    limitAngularPenetration = rotation;

    applyLimitImpulse = true;

    neCollisionResult * cresult = constr->bodyA->sim->cresultHeap.Alloc(0);

    cresult->bodyA = constr->bodyA;

    cresult->bodyB = constr->bodyB;

    cresult->impulseType = IMPULSE_ANGULAR_LIMIT_PRIMARY;

    cresult->contactBBody = limitAxis;

    cresult->depth = limitAngularPenetration;

    cresult->convexA = (TConvex *)constr;

    cresult->PrepareForSolver();
}

void neLimitState::CheckLimitPrimarySlider()
{
    neV3 diff = constr->frameAWorld.pos - constr->frameBWorld.pos;

    f32 dot = diff.Dot(constr->frameBWorld.rot[1]);

    f32 depth, sign;

    if (dot > upperLimit)
    {
        depth = dot - upperLimit;

        sign = 1.0f;

        lowerLimitOn = false;

        applyLimitImpulse = true;
    }
    else if (dot < lowerLimit)
    {
        depth = lowerLimit - dot;

        sign = -1.0f;

        lowerLimitOn = true;

        applyLimitImpulse = true;
    }
    else
    {
        applyLimitImpulse = false;

        return;
    }

    ASSERT(depth >= 0.0f);

    neCollisionResult * cresult = constr->bodyA->sim->cresultHeap.Alloc(0);

    if (depth > 0.05f)
        depth = 0.05f;

    cresult->depth = depth;

    cresult->bodyA = constr->bodyA;

    cresult->bodyB = constr->bodyB;

    cresult->contactA = constr->frameAWorld.pos - constr->bodyA->State().b2w.pos;

    if (constr->bodyB)
    {
        cresult->contactB = constr->frameBWorld.pos - constr->bodyB->GetB2W().pos;
    }
    else
    {
        cresult->contactB = constr->frameBWorld.pos;
    }
    cresult->impulseType = IMPULSE_SLIDER_LIMIT_PRIMARY;

    cresult->contactBWorld = constr->frameBWorld.rot[1] * sign;

    cresult->convexA = (TConvex *)constr;

    cresult->PrepareForSolver();
}

void neConstraintHeader::AddToSolver(f32 & epsilon, s32 & iteration)
{
    _neConstraint * constraint = head;

    s32 i = 0;

    while (constraint)
    {
        if (constraint->enable)
        {
            constraint->UpdateCurrentPosition();

            constraint->UpdateConstraintPoint();

            constraint->FindGreatest();

            //constraint->DrawCPointLine();

            if (epsilon == -1.0f || (constraint->accuracy > 0.0f && constraint->accuracy < epsilon))
                epsilon = constraint->accuracy;

            if (constraint->iteration > iteration)
                iteration = constraint->iteration;
        }
        neFreeListItem<_neConstraint> * item = (neFreeListItem<_neConstraint> *)constraint;

        constraint = (_neConstraint*)(item->next);

        i++;
    }

    solved = true;
}

void _neConstraint::ApplyDamping()
{
    // rel vel between a and b, as if b was hold still

    neRigidBody_ * rb = NULL;

    if (bodyB && bodyB->AsRigidBody())
    {
        rb = bodyB->AsRigidBody();
    }

    neV3 relVel;
    bool isLinear = false;

    switch (type)
    {
    case neJoint::NE_JOINT_BALLSOCKET:
        //damp rotation

        relVel = bodyA->Derive().angularVel;

        if (rb)
        {
            relVel -= rb->Derive().angularVel;
        }
        
        break;

    case neJoint::NE_JOINT_HINGE:
        //damp rotation only along the axis of the hinge joint
        {
            frameAWorld = bodyA->State().b2w * frameA;

            frameBWorld = GetFrameBWorld();

            neV3 jointAxis = frameAWorld.rot[1] + frameBWorld.rot[1];

            jointAxis.Normalize();

            relVel = bodyA->Derive().angularVel.Dot(jointAxis) * jointAxis;

            if (rb)
            {
                relVel -= (rb->Derive().angularVel.Dot(jointAxis) * jointAxis);
            }
        }
        break;

    case neJoint::NE_JOINT_SLIDE:
        break;
    default:
        ASSERT(0);
        break;
    }
    if (isLinear)
    {
        bodyA->totalForce -= (relVel * jointDampingFactor);
        
        if (rb)
            rb->totalForce += (relVel * jointDampingFactor);
    }
    else
    {
        bodyA->totalTorque -= (relVel * jointDampingFactor);

        if (rb)
            rb->totalTorque += (relVel * jointDampingFactor);
    }
}

void neConstraintHeader::TraverseApplyConstraint(neBool doCheckSleep)
{
    neBodyHandle * bodyHandle = bodies.GetHead();

    while (bodyHandle)
    {
        if (!bodyHandle->thing->AsRigidBody())
        {
            bodyHandle = bodyHandle->next;

            continue;
        }

        bodyHandle->thing->AsRigidBody()->needRecalc = false;

        neRigidBody_* rb = bodyHandle->thing->AsRigidBody();
/*
        if (!doCheckSleep && rb->status == neRigidBody_::NE_RBSTATUS_IDLE)
        {
            bodyHandle = bodyHandle->next;

            continue;
        }
*/      if (rb->impulseCount > 0)
        {
            rb->ApplyLinearConstraint();

        }
        if (rb->twistCount > 0)
        {
            rb->ApplyAngularConstraint();
        }
/*      if (doCheckSleep && rb->status == neRigidBody_::NE_RBSTATUS_IDLE)
        {
            rb->ConstraintDoSleepCheck();
        }
*/      bodyHandle = bodyHandle->next;
    }// while (bodyHanle)   next body in the constraint
}

neBool neConstraintHeader::StationaryCheck()
{
    neBool allStationary = true;

    neBodyHandle * bodyHandle = bodies.GetHead();

    while (bodyHandle)
    {
        neRigidBody_* rb = bodyHandle->thing->AsRigidBody();

        if (!rb)
        {
            bodyHandle = bodyHandle->next;

            continue;
        }

        if (!rb->CheckStationary())
        {
            allStationary = false;
        }
        bodyHandle = bodyHandle->next;
    }
    return allStationary;
}

void neConstraintHeader::BecomeIdle(neBool checkResting)
{
    neBodyHandle * bodyHandle = bodies.GetHead();

    while (bodyHandle)
    {
        neRigidBody_* rb = bodyHandle->thing->AsRigidBody();

        if (!rb)
        {
            bodyHandle = bodyHandle->next;

            continue;
        }
        if (checkResting)
        {
            if (rb->IsRestPointStillValid())
            {
                //if (rb->CheckRestHull())
                {
                    rb->BecomeIdle();
                }
            }
        }
        else
        {
            rb->BecomeIdle();
        }
        bodyHandle = bodyHandle->next;
    }
}

void neConstraintHeader::WakeUp()
{
    neBodyHandle * bodyHandle = bodies.GetHead();

    while (bodyHandle)
    {
        neRigidBody_* rb = bodyHandle->thing->AsRigidBody();

        if (!rb)
        {
            bodyHandle = bodyHandle->next;

            continue;
        }

        rb->status = neRigidBody_::NE_RBSTATUS_NORMAL;

        bodyHandle = bodyHandle->next;
    }
}

void neConstraintHeader::RemoveAll()
{
    neBodyHandle * bodyHandle = bodies.GetHead();

    while (bodyHandle)
    {
        neRigidBody_ * rb = bodyHandle->thing->AsRigidBody();

        ASSERT(rb);

        bodyHandle = bodyHandle->next;

        rb->SetConstraintHeader(NULL);

        bodies.Remove(&rb->constraintHeaderItem);
    }
    Reset();
}

void neMotor::PrepareForSolver(_neConstraint * constr)
{
    if (this == &constr->motors[0])
    {
        // primary motor

        // check if at limit already
        if (constr->limitStates[0].enableLimit && constr->limitStates[0].applyLimitImpulse)
        {
            if (constr->limitStates[0].lowerLimitOn)
            {
                if (desireVelocity < 0.0f)
                    return;
            }
            else
            {
                if (desireVelocity > 0.0f)
                    return;
            }
        }

        switch (constr->type)
        {
        case neJoint::NE_JOINT_HINGE:
            {
                neCollisionResult * cresult = constr->bodyA->sim->cresultHeap.Alloc(0);

                cresult->bodyA = constr->bodyA;

                cresult->bodyB = constr->bodyB;

                cresult->impulseType = IMPULSE_ANGULAR_MOTOR_PRIMARY;

                cresult->finalRelativeSpeed = desireVelocity;

                cresult->depth = maxForce;

                cresult->contactBBody = constr->frameBWorld.rot[1];

                cresult->contactABody = constr->frameAWorld.rot[1];//limitStates[0].limitAxis;

                cresult->PrepareForSolver();
            }
            break;
        
        case neJoint::NE_JOINT_SLIDE:
            {
                // up and down the shaft of the slider
                neCollisionResult * cresult = constr->bodyA->sim->cresultHeap.Alloc(0);

                cresult->bodyA = constr->bodyA;

                cresult->bodyB = constr->bodyB;

                cresult->impulseType = IMPULSE_RELATIVE_LINEAR_VELOCITY;

                cresult->finalRelativeSpeed = desireVelocity;

                cresult->depth = maxForce;

                cresult->contactBBody = constr->frameBWorld.rot[1];

                cresult->contactABody = constr->frameAWorld.rot[1];

                cresult->PrepareForSolver();
            }
            break;

        default:
            break;
        }
    }
    else
    {
        // secondary motor
        switch (constr->type)
        {
        case neJoint::NE_JOINT_SLIDE:
            {
                // rotation around the slider
            }
            break;

        default:
            break;
        }
    }
}


/**cylinder.cpp**/

//#include "tokamak.h"
//#include "containers.h"
//#include "scenery.h"
//#include "collision.h"
//#include "collision2.h"
//#include "constraint.h"
//#include "rigidbody.h"

#include <assert.h>
#include <stdio.h>

//extern void DrawLine(const neV3 & colour, neV3 * startpoint, s32 count);

void Cylinder2TerrainTest(neCollisionResult & result, TConvex & cylinderA, neT3 & transA, TConvex & terrainB)
{
    neSimpleArray<s32> & _triIndex = *terrainB.as.terrain.triIndex;

    s32 triangleCount = _triIndex.GetUsedCount();

    neArray<neTriangle_> & triangleArray = *terrainB.as.terrain.triangles;

    ConvexTestResult res[2];

    s32 finalTriIndex = -1;
    s32 currentRes = 1;
    s32 testRes = 0;

    res[currentRes].depth = -1.0e6f;
    res[currentRes].valid = false;
    res[testRes].depth = 1.0e6f;
    
    s32 terrainMatID = 0;

    for (s32 i = 0; i < triangleCount; i++)
    {
        s32 test = _triIndex[i];

        neTriangle_ * t = &triangleArray[_triIndex[i]];

        TriangleParam triParam;

        triParam.vert[0] = terrainB.vertices[t->indices[0]];
        triParam.vert[1] = terrainB.vertices[t->indices[1]];
        triParam.vert[2] = terrainB.vertices[t->indices[2]];

        triParam.edges[0] = triParam.vert[1] - triParam.vert[0];
        triParam.edges[1] = triParam.vert[2] - triParam.vert[1];
        triParam.edges[2] = triParam.vert[0] - triParam.vert[2];
        triParam.normal = triParam.edges[0].Cross(triParam.edges[1]);
        triParam.normal.Normalize();
        triParam.d = triParam.normal.Dot(triParam.vert[0]);

        if (t->flag == neTriangle::NE_TRI_TRIANGLE)
        {
            if (CylinderTriTest(cylinderA, transA, res[testRes], triParam))
            {
                if (res[testRes].depth > res[currentRes].depth)
                {
                    s32 tmp = testRes;  

                    testRes = currentRes;

                    currentRes = tmp;

                    terrainMatID = t->materialID;

                    finalTriIndex = _triIndex[i];
                }
            }
        }
        else if (t->flag == neTriangle::NE_TRI_HEIGHT_MAP)
        {
        }
        else
        {
            ASSERT(0);
        }
    }
    if (res[currentRes].valid)
    {
/*      {
            neV3 points[4];
            neV3 red;

            neTriangle_ * t =  &triangleArray[finalTriIndex];

            points[0] = terrainB.vertices[t->indices[0]];
            points[1] = terrainB.vertices[t->indices[1]];
            points[2] = terrainB.vertices[t->indices[2]];
            points[3] = terrainB.vertices[t->indices[0]];

            DrawLine(red, points, 4);
        }
*/      result.penetrate = true;

        result.depth = res[currentRes].depth;

        result.collisionFrame[2] = res[currentRes].contactNormal;

        result.materialIdB = terrainMatID;

        result.contactA = res[currentRes].contactA;

        result.contactB = res[currentRes].contactB;
    }
    else
    {
        result.penetrate = false;
    }
}

neBool CylinderTriTest_PlaneEnd(TConvex & cylinder, neT3 & trans, ConvexTestResult & result, TriangleParam & tri)
{
    f32 dist = trans.pos.Dot(tri.normal) - tri.d;

    neV3 dir;

    if (dist >= 0.0f)
    {
        dir = tri.normal * -1.0f;

        dist *= -1.0f;
    }
    else
    {
        dir = tri.normal;
    }

    if (neAbs(dist) >= ( cylinder.CylinderHalfHeight() + cylinder.CylinderRadius()))
        return false;

    neV3 contactPoint = trans.pos;

    neV3 l = trans.rot[1] * cylinder.CylinderHalfHeight();

    f32 dot = l.Dot(dir);

    if (dot > 0.0f)
    {
        contactPoint += l;

        dist += dot;
    }
    else
    {
        contactPoint -= l;

        dist -= dot;
    }
    contactPoint += dir * cylinder.CylinderRadius();

    dist += cylinder.CylinderRadius();

    if (dist <= 0.0f)
        return false;

    if (dist >= result.depth)
        return true;

    neV3 project = contactPoint - dir * dist;

    if (tri.IsPointInside(project) != -1)
        return true;

    neV3 project2; 
    
    project2.GetIntersectPlane(dir, tri.vert[0], trans.pos, contactPoint);

    s32 region = tri.IsPointInside(project2);
    
    if (region != -1)
        return true;

    result.valid = true;
    result.depth = dist;
    result.contactA = contactPoint; // on the cylinder
    result.contactB = project; // on the triangle
    result.contactNormal = dir * -1.0f;

    return true;
}

neBool CylinderTriTest_Line(TConvex & cylinder, neT3 & trans, ConvexTestResult & result, neV3 & point1, neV3 & point2)
{
    ConvexTestResult cr;

    cr.edgeA[0] = trans.pos + trans.rot[1] * cylinder.CylinderHalfHeight();
    cr.edgeA[1] = trans.pos - trans.rot[1] * cylinder.CylinderHalfHeight();
    cr.edgeB[0] = point1;
    cr.edgeB[1] = point2;

    f32 au, bu;

    if (!cr.ComputerEdgeContactPoint2(au, bu))
        return true;

    f32 depth = cylinder.CylinderRadius() - cr.depth;

    if (depth <= 0.0f)
        return false;

    if (depth >= result.depth)
        return true;

    if (cr.valid)
    {
        result.depth = depth;
        result.valid = true;
        result.contactA = cr.contactA;
        result.contactB = cr.contactB;
        result.contactNormal = trans.rot[1].Cross(point1 - point2);
        result.contactNormal.Normalize();

        neV3 diff = cr.contactA - cr.contactB;

        if (diff.Dot(result.contactNormal) < 0.0f)
        {
            result.contactNormal *= -1.0f;
        }
        result.contactA -= result.contactNormal * cylinder.CylinderRadius();
    }
    else
    {
        if (au > 0.0f && au < 1.0f)
        {
            // vertex of line and trunk of cylinder

            neV3 vert;

            if (bu <= 0.0f)
            {
                //point1

                vert = point1;
            }
            else
            {
                //point2

                vert = point2;
            }
            neV3 project;

            f32 depth = vert.GetDistanceFromLine2(project, cr.edgeA[0], cr.edgeA[1]);

            depth = cylinder.CylinderRadius() - depth;

            if (depth <= 0.0f)
                return false;

            if (depth >= result.depth)
                return true;

            result.depth = depth;
            result.valid = true;
            result.contactB = vert;
            result.contactNormal = project - vert;
            result.contactNormal.Normalize();
            result.contactA = project - result.contactNormal * cylinder.CylinderRadius();
        }
        else
        {
            neV3 cylinderVert;

            if (au <= 0.0f)
            {
                cylinderVert = cr.edgeA[0];
            }
            else // au >= 1.0f
            {
                cylinderVert = cr.edgeA[1];
            }
            if (bu > 0.0f && bu < 1.0f)
            {
                // cylinderVert and edge

                neV3 project;

                f32 depth = cylinderVert.GetDistanceFromLine2(project, cr.edgeB[0], cr.edgeB[1]);

                depth = cylinder.CylinderRadius() - depth;

                if (depth <= 0.0f)
                    return false;

                if (depth >= result.depth)
                    return true;

                result.depth = depth;
                result.valid = true;
                result.contactB = project;
                result.contactNormal = cylinderVert - project;
                result.contactNormal.Normalize();
                result.contactA = cylinderVert - result.contactNormal * cylinder.CylinderRadius();
            }
            else
            {
                neV3 lineVert;

                if (bu <= 0.0f)
                {
                    //point1
                    lineVert = point1;
                }
                else
                {
                    //point2
                    lineVert = point2;
                }
                neV3 diff = cylinderVert - lineVert;

                f32 depth = diff.Dot(diff);

                if (depth >= cylinder.CylinderRadiusSq())
                    return false;

                depth = sqrtf(depth);

                depth = cylinder.CylinderRadius() - depth;

                if (depth >= result.depth)
                    return true;

                result.depth = depth;
                result.valid = true;
                result.contactB = lineVert;
                result.contactNormal = diff;
                result.contactNormal.Normalize();
                result.contactA = cylinderVert - result.contactNormal * cylinder.CylinderRadius();
            }
        }
    }
    return true;
}

neBool CylinderTriTest(TConvex & cylinder, neT3 & trans, ConvexTestResult & result, TriangleParam & tri)
{
    // test plane of triangle and rim of cylinder
    result.valid = false;
    result.depth = 1.0e6f;
    result.needTransform = false;

    if (!CylinderTriTest_PlaneEnd(cylinder, trans, result, tri))
        return false;
    
    if (!result.valid)
    {
        result.valid = false;
        result.depth = 1.0e6f;
        result.needTransform = false;
        for (s32 i = 0; i < 3; i++)
        {
            CylinderTriTest_Line(cylinder, trans, result, tri.vert[i], tri.vert[neNextDim1[i]]);
        }
    }
    return result.valid;
}

void TestCylinderVertEdge(neCollisionResult & result, neV3 & edgeA1, neV3 & edgeA2, neV3 & vertB, 
                          TConvex & cA, TConvex & cB, neT3 & transA, neT3 & transB, neBool flip)
{
    neV3 project;

    f32 dist = vertB.GetDistanceFromLine2(project, edgeA1, edgeA2);

    f32 depth = cA.CylinderRadius() + cB.CylinderRadius() - dist;

    if (depth <= 0.0f)
        return;

    if (depth <= result.depth)
        return;

    result.penetrate = true;

    result.depth = depth;

    if (!flip)
    {
        result.collisionFrame[2] = project - vertB;

        result.collisionFrame[2].Normalize();

        result.contactA = project - result.collisionFrame[2] * cA.CylinderRadius();

        result.contactB = vertB + result.collisionFrame[2] * cB.CylinderRadius();
    }
    else
    {
        result.collisionFrame[2] = vertB - project;

        result.collisionFrame[2].Normalize();

        result.contactA = vertB - result.collisionFrame[2] * cB.CylinderRadius();

        result.contactB = project + result.collisionFrame[2] * cA.CylinderRadius();
    }
}

void TestCylinderVertVert(neCollisionResult & result, neV3 & vertA, neV3 & vertB, 
                          TConvex & cA, TConvex & cB, neT3 & transA, neT3 & transB)
{
    neV3 diff = vertA - vertB;

    f32 dist = diff.Length();

    f32 depth = cA.CylinderRadius() + cB.CylinderRadius() - dist;

    if (depth <= 0.0f)
        return;

    if (depth <= result.depth)
        return;

    result.penetrate = true;

    result.depth = depth;

    result.collisionFrame[2] = diff * (1.0f / dist);

    result.contactA = vertA - result.collisionFrame[2] * cA.CylinderRadius();

    result.contactB = vertB + result.collisionFrame[2] * cB.CylinderRadius();
}

void Cylinder2CylinderTest(neCollisionResult & result, TConvex & cA, neT3 & transA, TConvex & cB, neT3 & transB)
{
    result.penetrate = false;

    neV3 dir = transA.rot[1].Cross(transB.rot[1]);

    f32 len = dir.Length();

    neBool isParallel = neIsConsiderZero(len);

    s32 doVertCheck = 0;

    ConvexTestResult cr;

    cr.edgeA[0] = transA.pos + transA.rot[1] * cA.CylinderHalfHeight();
    cr.edgeA[1] = transA.pos - transA.rot[1] * cA.CylinderHalfHeight();
    cr.edgeB[0] = transB.pos + transB.rot[1] * cB.CylinderHalfHeight();
    cr.edgeB[1] = transB.pos - transB.rot[1] * cB.CylinderHalfHeight();

    f32 dot = transA.rot[1].Dot(transB.rot[1]);

    if (!neIsConsiderZero(len))
    {
        f32 au, bu;

        cr.ComputerEdgeContactPoint2(au, bu);

        if (cr.valid)
        {
            f32 depth = cA.CylinderRadius() + cB.CylinderRadius() - cr.depth;

            if (depth <= 0.0f)
                return;

            result.depth = depth;

            result.penetrate = true;

            result.collisionFrame[2] = cr.contactA - cr.contactB;
            
            result.collisionFrame[2] *= (1.0f / cr.depth);

            result.contactA = cr.contactA - result.collisionFrame[2] * cA.CylinderRadius();

            result.contactB = cr.contactB + result.collisionFrame[2] * cB.CylinderRadius();

            return;
        }
    }
    result.depth = -1.0e6f;

    s32 i;

    for (i = 0; i < 2; i++)
    {
        //project onto edge b

        neV3 diff = cr.edgeA[i] - cr.edgeB[1];

        f32 dot = diff.Dot(transB.rot[1]);

        if (dot < 0.0f)
        {
            TestCylinderVertVert(result, cr.edgeA[i], cr.edgeB[1], cA, cB, transA, transB);
        }
        else if (dot > (2.0f * cB.CylinderHalfHeight()))
        {
            TestCylinderVertVert(result, cr.edgeA[i], cr.edgeB[0], cA, cB, transA, transB);
        }
        else
        {
            TestCylinderVertEdge(result, cr.edgeB[0], cr.edgeB[1], cr.edgeA[i], cB, cA, transB, transA, true);
        }
    }
    for (i = 0; i < 2; i++)
    {
        //project onto edge b

        neV3 diff = cr.edgeB[i] - cr.edgeA[1];

        f32 dot = diff.Dot(transA.rot[1]);

        if (dot < 0.0f)
        {
            TestCylinderVertVert(result, cr.edgeB[i], cr.edgeA[1], cA, cB, transA, transB);
        }
        else if (dot > (2.0f * cB.CylinderHalfHeight()))
        {
            TestCylinderVertVert(result, cr.edgeB[i], cr.edgeA[0], cA, cB, transA, transB);
        }
        else
        {
            TestCylinderVertEdge(result, cr.edgeA[0], cr.edgeA[1], cr.edgeB[i], cA, cB, transA, transB, false);
        }
    }
}

void Cylinder2SphereTest(neCollisionResult & result, TConvex & cylinderA, neT3 & transA, TConvex & sphereB, neT3 & transB)
{
    result.penetrate = false;

    neV3 cylinderTop = transA.pos + transA.rot[1] * cylinderA.CylinderHalfHeight();

    neV3 cylinderBottom = transA.pos - transA.rot[1] * cylinderA.CylinderHalfHeight();

    neV3 diff0 = transB.pos - cylinderBottom;

    f32 k = diff0.Dot(transA.rot[1]);

    if (k >= (2.0f * cylinderA.CylinderHalfHeight() + cylinderA.CylinderRadius() + sphereB.Radius()))
        return;

    if (k <= -(cylinderA.CylinderRadius() + sphereB.Radius()))
        return;

    neV3 project;

    if (k > 2.0f * cylinderA.CylinderHalfHeight()) //cylinderTop
    {
        project = cylinderTop;
    }
    else if (k < 0.0f)
    {
        project = cylinderBottom;
    }
    else
    {
        project = cylinderBottom + k * transA.rot[1];
    }

    neV3 diff1 = project - transB.pos;

    f32 dist = diff1.Dot(diff1);
    
    dist = sqrtf(dist);

    if (dist >= (cylinderA.CylinderRadius() + sphereB.Radius()))
        return;

    f32 depth = cylinderA.CylinderRadius() + sphereB.Radius() - dist;

    ASSERT(depth > 0.0f);

    result.penetrate = true;

    result.depth = depth;

    if (!neIsConsiderZero(dist))
        result.collisionFrame[2] = diff1 * (1.0f / dist);
    else
        result.collisionFrame[2] = transA.rot[0];

    result.contactA = project - result.collisionFrame[2] * cylinderA.CylinderRadius();

    result.contactB = transB.pos + result.collisionFrame[2] * cylinderA.CylinderRadius();
}
/*
void Box2CylinderTest(neCollisionResult & result, TConvex & boxA, neT3 & transA, TConvex & cylinderB, neT3 & transB)
{
    result.penetrate = false;

    ConvexTestResult res;

    res.valid = false;

    res.depth = 1.0e6f;

    BoxTestParam boxParamA;

    boxParamA.convex = &boxA;
    boxParamA.trans = &transA;
    boxParamA.radii[0] = transA.rot[0] * boxA.BoxSize(0);
    boxParamA.radii[1] = transA.rot[1] * boxA.BoxSize(1);
    boxParamA.radii[2] = transA.rot[2] * boxA.BoxSize(2);

    for (s32 i = 0; i < 3; i++)
    {
        if (!boxParamA.CylinderFaceTest(res, cylinderB, transB, i))
            return;
    }

    if (!res.valid)
    {
        for (s32 i = 0; i < 3; i++)
        {
            if (!boxParamA.CylinderEdgeTest(res, cylinderB, transB, i))
                return;
        }
    }
    if (res.valid)
    {
        result.penetrate = true;

        result.depth = res.depth;

        result.collisionFrame[2] = res.contactNormal;

        result.contactA = res.contactA;

        result.contactB = res.contactB;
    }
}
*/
void ClosestLine2Box(const neV3 & p1, const neV3 & p2, const neV3 & boxSize, neV3 & lret, neV3 & bret)
{
    neV3 sign, s, v;

    s = p1;
    v = p2 - p1;

    s32 i;
    for (i = 0; i < 3; i++)
    {
        if (v[i] < 0.0f)
        {
            s[i] = -s[i];
            v[i] = -v[i];
            sign[i] = -1.0f;
        }
        else
        {
            sign[i] = 1.0f;
        }
    }

    neV3 v2; v2 = v * v;

    neV3 h; h = boxSize;

    int region[3];
    neV3 tanchor;

    for (i = 0; i < 3; i++)
    {
        if (v[i] > 0.0f)
        {
            if (s[i] < -h[i])
            {
                region[i] = -1;
                tanchor[i] = (-h[i] - s[i]) / v[i];
            }
            else
            {
                region[i] = (s[i] > h[i]);
                tanchor[i] = (h[i] - s[i]) / v[i];
            }
        }
        else
        {
            region[i] = 0;
            tanchor[i] = 2;
        }
    }

    f32 t = 0;
    f32 dd2dt = 0;

    for (i = 0; i < 3; i++)
    {
        dd2dt -= (region[i] ? v2[i] : 0) * tanchor[i];
    }
    if (dd2dt >= 0.0f)
        goto got_answer;

    do {
        f32 next_t = 1;

        for (i = 0; i < 3; i++)
        {
            if (tanchor[i] > t && tanchor[i] < 1 && tanchor[i] < next_t)
                next_t = tanchor[i];
        }
        f32 next_dd2dt = 0;

        for (i=0; i<3; i++) 
        {
            next_dd2dt += (region[i] ? v2[i] : 0) * (next_t - tanchor[i]);
        }
        if (next_dd2dt >= 0) 
        {
            f32 m = (next_dd2dt-dd2dt)/(next_t - t);
            t -= dd2dt/m;
            goto got_answer;
        }
        for (i=0; i<3; i++) {
            if (tanchor[i] == next_t) 
            {
                tanchor[i] = (h[i]-s[i])/v[i];
                region[i]++;
            }
        }
        t = next_t;
        dd2dt = next_dd2dt;
    } while(t < 1);
    
    t = 1;

got_answer:

    neV3 diff = p2 - p1;

    lret = p1 + diff * t;

    for (i=0; i<3; i++) 
    {
        bret[i] = sign[i] * (s[i] + t*v[i]);
        if (bret[i] < -h[i]) 
            bret[i] = -h[i];
        else if (bret[i] > h[i]) 
            bret[i] = h[i];
    }
}

void Box2CylinderTest(neCollisionResult & result, TConvex & boxA, neT3 & transA, TConvex & cylinderB, neT3 & transB)
{
    result.penetrate = false;

    ConvexTestResult res;

    res.valid = false;

    res.depth = 1.0e6f;

    neT3 cylinder2box;

    cylinder2box = transA.FastInverse();

    cylinder2box = cylinder2box * transB;

    neV3 c1, c2;

    c1 = cylinder2box.pos - cylinder2box.rot[1] * cylinderB.as.cylinder.halfHeight;

    c2 = cylinder2box.pos + cylinder2box.rot[1] * cylinderB.as.cylinder.halfHeight;

    neV3 lret, bret;

    ClosestLine2Box(c1, c2, boxA.as.box.boxSize, lret, bret);

    bret = transA * bret;

    lret = transA * lret;

    neV3 diff; 
    
    diff = bret - lret;

    f32 dist = diff.Length();

    if (dist > cylinderB.as.cylinder.radius)
    {
        return;
    }
    result.depth = cylinderB.as.cylinder.radius - dist;

    result.penetrate = true;

    f32 d1 = 1.0f / dist;

    result.contactA = bret;

    result.collisionFrame[2] = diff * d1;

    result.contactB = (lret + result.collisionFrame[2] * cylinderB.as.cylinder.radius) ;
}


/**dcd.cpp**/

//#include "tokamak.h"
//#include "containers.h"
//#include "scenery.h"
//#include "collision.h"
//#include "collision2.h"
//#include "constraint.h"
//#include "rigidbody.h"
//#include "scenery.h"
//#include "stack.h"
//#include "simulator.h"
//#include "message.h"
//#include "dcd.h"

const s32 BOX_NUM_FACES = 6;

const s32 BOX_NUM_VERTS = 8;

const s32 BOX_NUM_EDGES = 12;

const s32 TRI_NUM_FACES = 2;

const s32 TRI_NUM_VERTS = 3;

const s32 TRI_NUM_EDGES = 3;


s32 _num_edge_test;

s32 _num_face_test;

static neByte _boxNeighbourFaces[][4] = {{2,3,4,5},{2,3,4,5},{0,1,4,5},{0,1,4,5},{0,1,2,3},{0,1,2,3}};
static neByte _boxNeighbourVerts[][4] = {{2,3,6,7},{0,1,4,5},{4,5,6,7},{0,1,2,3},{1,3,5,7},{0,2,4,6}};
static neByte _boxNeighbourEdges[][4] = {{0,1,2,3},{4,5,6,7},{0,4,8,9},{1,5,10,11},{2,8,6,10},{3,7,9,11}};
static neByte _boxVertNeighbourEdges[][4] = {{5,7,11,0xff},{5,6,10,0xff},{1,3,11,0xff},{1,2,10,0xff},{4,7,9,0xff},{4,6,8,0xff},{0,3,9,0xff},{0,2,8,0xff}};
static neV3 _boxNormals[BOX_NUM_FACES] = {{0,1,0,0},{0,-1,0,0},{1,0,0,0},{-1,0,0,0},{0,0,1,0},{0,0,-1,0}};
static neV3 _boxVertexPos0[BOX_NUM_VERTS] = {{-1,-1,-1,0},{-1,-1,1,0},{-1,1,-1,0},{-1,1,1,0},{1,-1,-1,0},{1,-1,1,0},{1,1,-1,0},{1,1,1,0}};
static neV3 _boxVertexPosP[BOX_NUM_VERTS];
static neV3 _boxVertexPosQ[BOX_NUM_VERTS];
static neBool _visited[100];

DCDFace BoxFaces[BOX_NUM_FACES] =
{
    {_boxNeighbourFaces[0],_boxNeighbourVerts[0],_boxNeighbourEdges[0]}, //0
    {_boxNeighbourFaces[1],_boxNeighbourVerts[1],_boxNeighbourEdges[1]}, //1
    {_boxNeighbourFaces[2],_boxNeighbourVerts[2],_boxNeighbourEdges[2]}, //2
    {_boxNeighbourFaces[3],_boxNeighbourVerts[3],_boxNeighbourEdges[3]}, //3
    {_boxNeighbourFaces[4],_boxNeighbourVerts[4],_boxNeighbourEdges[4]}, //4
    {_boxNeighbourFaces[5],_boxNeighbourVerts[5],_boxNeighbourEdges[5]}, //5
};

DCDVert BoxVertices[BOX_NUM_VERTS] =
{
    {_boxVertNeighbourEdges[0],}, //0
    {_boxVertNeighbourEdges[1],}, //1
    {_boxVertNeighbourEdges[2],}, //2
    {_boxVertNeighbourEdges[3],}, //3
    {_boxVertNeighbourEdges[4],}, //4
    {_boxVertNeighbourEdges[5],}, //5
    {_boxVertNeighbourEdges[6],}, //6
    {_boxVertNeighbourEdges[7],}, //7
};

DCDEdge BoxEdges[BOX_NUM_EDGES] = 
{
    {0,2,6,7}, //0
    {0,3,2,3}, //1
    {0,4,3,7}, //2
    {0,5,2,6}, //3
    {1,2,4,5}, //4
    {1,3,0,1}, //5
    {1,4,1,5}, //6
    {1,5,0,4}, //7
    {2,4,5,7}, //8
    {2,5,4,6}, //9
    {3,4,1,3}, //10
    {3,5,0,2}, //11
};

static neByte _triNeigbhourFaces[TRI_NUM_FACES][1] = {{0}, {1}};
static neByte _triNeighbourVerts[TRI_NUM_FACES][3] = {{0,1,2},{0,1,2}};
static neByte _triNeighbourEdges[TRI_NUM_FACES][3] = {{0,1,2},{0,1,2}};
static neByte _triVertNeighbourEdges[TRI_NUM_VERTS][3] = {{0,2, 0xff},{0, 1, 0xff},{1, 2, 0xff}};

static neV3 _triNormals[TRI_NUM_FACES];
static neV3 _triVertexPos[3];

DCDFace TriFaces[TRI_NUM_FACES] = 
{
    {_triNeigbhourFaces[0],_triNeighbourVerts[0],_triNeighbourEdges[0]},
    {_triNeigbhourFaces[1],_triNeighbourVerts[1],_triNeighbourEdges[1]},
};

DCDVert TriVertices[TRI_NUM_VERTS] = 
{
    {_triVertNeighbourEdges[0]},
    {_triVertNeighbourEdges[1]},
    {_triVertNeighbourEdges[2]},
};

DCDEdge TriEdges[TRI_NUM_EDGES] =
{
    {0,1,0,1},
    {0,1,1,2},
    {0,1,2,0},
};

neV3 TriEdgeDir[TRI_NUM_EDGES];

void DCDMesh::SetConvex(const TConvex & convex, neV3 * vertArray)
{
    if (convex.type == TConvex::BOX)
    {
        numFaces = BOX_NUM_FACES;
        numVerts = BOX_NUM_VERTS;
        //numEdges = BOX_NUM_EDGES;
        normals = _boxNormals;
        faces = BoxFaces;
        verts = BoxVertices;
        edges = BoxEdges;
        if (vertArray)
            vertices = vertArray;

        numNeighbour = 4;
    }
    else if (convex.type == TConvex::CONVEXDCD)
    {
        numFaces = *(int*)convex.as.convexDCD.convexData;
        numVerts = *((int*)convex.as.convexDCD.convexData+1);
        //numEdges = *((int*)convex.as.convexDCD.convexData+2);

        f32 * np = (f32 *)(convex.as.convexDCD.convexData + 4 * sizeof(int));
        normals = (neV3*)np;

        vertices = (neV3*)(np + 4 * numFaces);
        faces = (DCDFace*)(vertices + numVerts);
        verts = (DCDVert*)((neByte*)faces + sizeof(DCDFace) * numFaces);
        edges = (DCDEdge*)((neByte*)verts + sizeof(DCDVert) * numVerts);

        numNeighbour = 3;
    }
    else if (convex.type == TConvex::TRIANGLE)
    {
        numFaces = TRI_NUM_FACES;
        numVerts = TRI_NUM_VERTS;
        normals = _triNormals;
        vertices = _triVertexPos;
        faces = TriFaces;
        verts = TriVertices;
        edges = TriEdges;
        numNeighbour = 3;
    }
}
neV3 DCDMesh::GetVertOnFace(s32 faceIndex, s32 vertIndex)
{
    return vertices[faces[faceIndex].neighbourVerts[vertIndex]];
}
neV3 DCDMesh::GetVert(s32 vertIndex)
{
    return vertices[vertIndex];
}
neV3  DCDMesh::GetNormal(s32 faceIndex)
{
    return normals[faceIndex];
}
/*neByte DCDMesh::FaceGetNumFaceNeighbour(s32 faceIndex)
{
    return faces[faceIndex].numberFaceNeighbour;
}
*/neByte DCDMesh::FaceGetFaceNeighbour(s32 faceIndex, s32 neighbourIndex)
{
    return faces[faceIndex].neighbourFaces[neighbourIndex];
}
neByte DCDMesh::FaceGetEdgeNeighbour(s32 faceIndex, s32 neighbourIndex)
{
    return faces[faceIndex].neighbourEdges[neighbourIndex];
}
/*neByte DCDMesh::VertGetNumEdgeNeighbour(s32 vertIndex)
{
    return verts[vertIndex].numberEdgeNeighbour;
}
*/neByte DCDMesh::VertGetEdgeNeighbour(s32 vertIndex, s32 neighbourIndex)
{
    return verts[vertIndex].neighbourEdges[neighbourIndex];
}
neByte DCDMesh::EdgeGetVert1(s32 edgeIndex)
{
    return edges[edgeIndex].v1;
}
neByte DCDMesh::EdgeGetVert2(s32 edgeIndex)
{
    return edges[edgeIndex].v2;
}

const s32 NUM_STACK_SIZE = 200;

bool CalcContactEE(const neV3 & edgeA0, 
                    const neV3 & edgeA1, 
                    const neV3 & edgeB0, 
                    const neV3 & edgeB1, neV3 & contactA, neV3 & contactB);

struct EdgeStackRecord
{
    s32 edgeP;
    s32 edgeQ;
};

class EdgeStack
{
public:
    void Init()
    {
        tos = 0;
    }
    void Push(s32 edgeP, s32 edgeQ)
    {
        ASSERT(tos < NUM_STACK_SIZE);

        for (s32 i = 0; i < tos; i++)
        {
            if ((eStack[i].edgeP == edgeP && eStack[i].edgeQ == edgeQ) ||
                (eStack[i].edgeP == edgeQ && eStack[i].edgeQ == edgeP))
            return;
        }
        eStack[tos].edgeP = edgeP;
        eStack[tos].edgeQ = edgeQ;
        tos++;
    }

    bool Pop(s32 & edgeP, s32 & edgeQ)
    {
        ASSERT(tos > 0);

        tos--;
        edgeP = eStack[tos].edgeP;
        edgeQ = eStack[tos].edgeQ;
        return true;
    }
    neBool IsEmpty()
    {
        return tos == 0;
    }
private:
    s32 tos;
    EdgeStackRecord eStack[NUM_STACK_SIZE];
};

EdgeStack gEdgeStack;

neV3 BigC;

f32 BigCLength;

class Face
{
public:
    // face is defined as normal.Dot(p) = k
    neV3 normal;
    f32 k;
};

class DCDObj
{
public:
    //TConvex * convex;
    neBool isBox;

    DCDMesh mesh;

    neT3 * trans;

    Face GetFace(s32 faceIndex)
    {
        Face face0;

        face0.normal = trans->rot * mesh.normals[faceIndex];

        neV3 tmp1 = (*trans) * mesh.GetVertOnFace(faceIndex, 0);

        //face0.k = face0.normal.Dot(trans->pos) + mesh.normals[faceIndex].v[3] * -1.0f;
        
        face0.k = face0.normal.Dot(tmp1);

        return face0;
    }
    neV3 GetVertWorld(s32 vertIndex)
    {
        neV3 vert = (*trans) * mesh.vertices[vertIndex];

        return vert;
    }
    neV3 GetNegVertWorld(s32 vertIndex)
    {
        neV3 vert = GetVertWorld(vertIndex) * -1.0f;

        return vert;
    }
    NEINLINE neV3 GetWorldNormalByEdge1(s32 edgeIndex)
    {
        neV3 ret; ret = trans->rot * mesh.normals[mesh.edges[edgeIndex].f1];

        return ret;
    }
    NEINLINE neV3 GetWorldNormalByEdge2(s32 edgeIndex)
    {
        neV3 ret; ret = trans->rot * mesh.normals[mesh.edges[edgeIndex].f2];

        return ret;
    }
    NEINLINE s32 GetSupportPoint(const neV3 & norm)
    {
        if (isBox)
            return GetSupportPointBox(norm);

        else
            return GetSupportPointMesh(norm);

        return 0;
    }
    NEINLINE void GetWorldEdgeVerts(s32 edgeIndex, neV3 & av, neV3 & bv)
    {
        neV3 tmp;
        
        tmp = mesh.vertices[mesh.edges[edgeIndex].v1];

        av = (*trans) * tmp;

        tmp = mesh.vertices[mesh.edges[edgeIndex].v2];

        bv = (*trans) * tmp;
    }
    neV3 FaceGetWorldNormal(s32 faceIndex)
    {
        return trans->rot * mesh.GetNormal(faceIndex);
    }
private:
    s32 GetSupportPointBox(const neV3 & norm)
    {
        neV3 localNorm = trans->rot.TransposeMulV3(norm);

        localNorm *= -1.0f;

        s32 ret = 0;

        if (localNorm[0] >= 0.0f)
        {
            if (localNorm[1] >= 0.0f)
            {
                if (localNorm[2] >= 0.0f)
                    ret = 7;
                else
                    ret = 6;
            }
            else
            {
                if (localNorm[2] >= 0.0f)
                    ret = 5;
                else
                    ret = 4;
            }
        }
        else
        {
            if (localNorm[1] >= 0.0f)
            {
                if (localNorm[2] >= 0.0f)
                    ret = 3;
                else
                    ret = 2;
            }
            else
            {
                if (localNorm[2] >= 0.0f)
                    ret = 1;
                else
                    ret = 0;
            }
        }
        return ret;
    }
    s32 GetSupportPointMesh(const neV3 & norm)
    {
        neV3 localNorm = trans->rot.TransposeMulV3(norm);

        localNorm *= -1.0f;

        s32 ret = 0;

        f32 maxd = -1.0e6f;

        neByte neighbourEdge;

        neBool moving;

        do {
            moving = false;
            
            s32 i = 0;

            //while (i < mesh.verts[ret].numberEdgeNeighbour)
            do 
            {
                s32 currentVert;

                neighbourEdge = mesh.verts[ret].neighbourEdges[i];

                if (neighbourEdge == 0xff)
                    break;

                if (mesh.edges[neighbourEdge].v1 == ret)
                    currentVert = mesh.edges[neighbourEdge].v2;
                else
                    currentVert = mesh.edges[neighbourEdge].v1;

                //if (currentVert > 10)
                //  ASSERT(0);
                
                f32 dot = mesh.vertices[currentVert].Dot(localNorm);

                if (dot > maxd)
                {
                    maxd = dot;

                    ret = currentVert;

                    moving = 1;
                    
                    break;
                }

                i++;
            } while (true);
        } while(moving);

        return ret;
/*
        for (s32 i = 0; i < mesh.numVerts; i++)
        {
            f32 dot = mesh.vertices[i].Dot(localNorm);

            if (dot > maxd)
            {
                maxd = dot;

                ret = i;
            }
        }
        return ret;
*/
    }

};

f32 funcD(const Face & face)
{
    f32 k = face.k;
    
    neV3 N; 
    
    N = face.normal;

    if (face.k < 0.0f)
    {
        k = -face.k;
        N *= -1.0f;
    }

    f32 den = k - N.Dot(BigC);

    den = BigCLength * den;

    ASSERT(!neIsConsiderZero(den));

    f32 ret = -k / den;

    return ret;
}

f32 SignedDistance(const Face & faceP, const neV3 & vertQ , Face & faceM)
{
    faceM = faceP;

    f32 dot = faceP.normal.Dot(vertQ);

    faceM.k += (dot);

    return funcD(faceM);
}

class SearchResult
{
public:
    enum Type
    {
        FACE,
        VERTEX,
        EDGE,
    } ;
    
    SearchResult(const TConvex & convexA, neT3 * transA, const TConvex & convexB, neT3 * transB, neV3 * vertArrayA, neV3 * vertArrayB)
    {
        objA.mesh.SetConvex(convexA, vertArrayA);

        objA.isBox = (convexA.type == TConvex::BOX);

        objA.trans = transA;

        objB.mesh.SetConvex(convexB, vertArrayB);

        objB.isBox = (convexB.type == TConvex::BOX);

        objB.trans = transB;

        dMax = -1.0e6f;
    }
    neBool TestFace(s32 face0Index, neBool & assigned)
    {
        assigned = false;

        _visited[face0Index] = true;

        Face face0 = objA.GetFace(face0Index);

        neByte _indexB = objB.GetSupportPoint(face0.normal);

        neV3 vertB;

        vertB = objB.GetNegVertWorld(_indexB);

        Face newFace;

        f32 d = SignedDistance(face0, vertB, newFace);

        if (d >= 0.0f)
            return false;

        if (d <= dMax)
            return true;

        dMax = d;

        typeA = SearchResult::FACE;

        typeB = SearchResult::VERTEX;

        indexA = face0Index;

        indexB = _indexB;

        face = newFace;

        assigned = true;

        _num_face_test++;

        return true;
    }
    neBool SearchFV(s32 initialFace, neBool & assigned);

    neBool SearchEE(s32 flag /*0 or 1*/, s32 aIndex, s32 bIndex, neBool & assigned);

    neBool SearchEETri(s32 flag /*0 or 1*/, s32 aIndex, s32 bIndex, neBool & assigned);

    DCDObj objA;

    Type typeA;

    s32 indexA;

    DCDObj objB;

    Type typeB;

    s32 indexB;

    Face face;

    f32 dMax;
};

neBool SearchResult::SearchFV(s32 initialFace, neBool & assigned)
{
    for (s32 i = 0; i < objA.mesh.numFaces; i++)
    {
        _visited[i] = false;

        if (objA.isBox)
        {
            objA.mesh.normals[i].v[3] = objA.mesh.vertices[objA.mesh.faces[i].neighbourVerts[0]].Dot(objA.mesh.normals[i]) * -1.0f;
        }
    }
    
    if (!TestFace(initialFace, assigned))
        return false;

    //ASSERT(assigned);

    neBool found = true;

    s32 currentFace = initialFace;

    while (found)
    {
        found = false;

        for (s32 ii = 0; ii < objA.mesh.numNeighbour; ii++)
        {
            s32 i = objA.mesh.FaceGetFaceNeighbour(currentFace, ii);

            if (_visited[i])
                continue;

            neBool _assigned;

            if (!TestFace(i, _assigned))
                return false;

            if (_assigned)
                found = true;
        }
        if (found)
        {
            currentFace = indexA;
        }
    }
    return true;
}

f32 Determinant(const neV3 & a, const neV3 & b, const neV3 & c)
{
    f32 t1 = a[0] * b[1] * c[2];

    f32 t2 = a[1] * b[0] * c[2];

    f32 t3 = a[0] * b[2] * c[1];

    f32 t4 = a[2] * b[1] * c[0];

    f32 t5 = a[1] * b[2] * c[0];

    f32 t6 = a[2] * b[0] * c[1];

    f32 ret = t1 - t2 - t3 - t4 + t5 + t6;
    
    return ret;
}

neBool SearchResult::SearchEE(s32 flag, s32 aIndex, s32 bIndex, neBool & assigned)
{
    assigned = false;

    gEdgeStack.Init();

    neByte edgeIndex;

    if (flag == 0) //fv
    {
        for (s32 i = 0; i < objA.mesh.numNeighbour; i++)
        {
            int j = 0;

            while ((edgeIndex = objB.mesh.VertGetEdgeNeighbour(bIndex, j)) != 0xff)
            {
                gEdgeStack.Push(objA.mesh.FaceGetEdgeNeighbour(aIndex, i),
                                objB.mesh.VertGetEdgeNeighbour(bIndex, j));

                j++;
            }
        }
    }
    else //vf
    {
        s32 i = 0;

        while ((edgeIndex = objA.mesh.VertGetEdgeNeighbour(aIndex, i)) != 0xff)
        {
            for (s32 j = 0; j < objB.mesh.numNeighbour; j++)
            {
                gEdgeStack.Push(objA.mesh.VertGetEdgeNeighbour(aIndex, i),
                                objB.mesh.FaceGetEdgeNeighbour(bIndex, j));
            }           
            i++;
        }
    }
    while (!gEdgeStack.IsEmpty())
    {
        _num_edge_test++;

        s32 edgeP, edgeQ;

        gEdgeStack.Pop(edgeP, edgeQ);

        // does the edge form a face
        neV3 a = objA.GetWorldNormalByEdge1(edgeP);

        neV3 b = objA.GetWorldNormalByEdge2(edgeP);

        neV3 c = objB.GetWorldNormalByEdge1(edgeQ) * -1.0f;

        neV3 d = objB.GetWorldNormalByEdge2(edgeQ) * -1.0f;

        f32 cba = Determinant(c,b,a);

        f32 dba = Determinant(d,b,a);

        f32 prod0 = cba * dba;

        if (prod0 >= 0.0f/*-1.0e-6f*/)
        {
            continue;
        }

        f32 adc = Determinant(a,d,c);

        f32 bdc = Determinant(b,d,c);

        f32 prod1 = adc * bdc;

        if (prod1 >= 0.0f/*-1.0e-6f*/)
        {
            continue;
        }
        f32 prod2 = cba * bdc;

        if (prod2 <= 0.0f/*1.0e-6f*/)
        {
            continue;
        }
        neV3 ai, bi;
        neV3 naj, nbj;

        objA.GetWorldEdgeVerts(edgeP, ai, bi);

        objB.GetWorldEdgeVerts(edgeQ, naj, nbj);

        naj *= -1.0f; nbj *= -1.0f;

        neV3 ainaj = ai + naj;
        neV3 ainbj = ai + nbj;
        neV3 binaj = bi + naj;
        //neV3 binbj = bi + nbj;

        neV3 diff1 = ainaj - ainbj;
        neV3 diff2 = ainaj - binaj ;

        Face testFace;

        testFace.normal = diff1.Cross(diff2);

        f32 len = testFace.normal.Length();

        if (neIsConsiderZero(len))
        {
            continue;
        }
        testFace.normal *= (1.0f / len);

        testFace.k = testFace.normal.Dot(ainaj);

        f32 testD = funcD(testFace);

        if (testD >= 0)
            return false;


        if (testD <= dMax)
            continue;

        assigned = true;
        dMax = testD;
        face = testFace;
        indexA = edgeP;
        indexB = edgeQ;
        typeA = SearchResult::EDGE; 
        typeB = SearchResult::EDGE;

        // push
        s32 i, j;

        s32 vindex;

        vindex = objB.mesh.EdgeGetVert1(edgeQ);

        i = 0;

        while ((j = objB.mesh.VertGetEdgeNeighbour(vindex, i)) != 0xff)
        {
            if (j != edgeQ)
                gEdgeStack.Push(edgeP, j);

            i++;
        }

        vindex = objB.mesh.EdgeGetVert2(edgeQ);

        i = 0;

        while ((j = objB.mesh.VertGetEdgeNeighbour(vindex, i)) != 0xff)
        {
            if (j != edgeQ)
                gEdgeStack.Push(edgeP, j);

            i++;
        }

        vindex = objA.mesh.EdgeGetVert1(edgeP);

        i = 0;

        while((j = objA.mesh.VertGetEdgeNeighbour(vindex, i)) != 0xff)
        {
            if (j != edgeP)
                gEdgeStack.Push(j, edgeQ);

            i++;
        }

        vindex = objA.mesh.EdgeGetVert2(edgeP);

        //for (i = 0; i < objA.mesh.VertGetNumEdgeNeighbour(vindex); i++)
        i = 0;

        while ((j = objA.mesh.VertGetEdgeNeighbour(vindex, i)) != 0xff)
        {
            if (j != edgeP)
                gEdgeStack.Push(j, edgeQ);

            i++;
        }
/*
        if (testD <= dMax)
            continue;

        assigned = true;
        dMax = testD;
        face = testFace;
        indexA = edgeP;
        indexB = edgeQ;
        typeA = SearchResult::EDGE; 
        typeB = SearchResult::EDGE;
*/  }
    return true;
}

neBool SearchResult::SearchEETri(s32 flag, s32 aIndex, s32 bIndex, neBool & assigned)
{
    assigned = false;

    gEdgeStack.Init();

    neByte edgeIndex;

    if (flag == 0) //fv
    {
        // face of convex A
        // vertex of triangle B
        for (s32 i = 0; i < objA.mesh.numNeighbour; i++) // for each edge neighbour of Face aIndex
        {
            int j = 0;

            while ((edgeIndex = objB.mesh.VertGetEdgeNeighbour(bIndex, j)) != 0xff)
            {
                gEdgeStack.Push(objA.mesh.FaceGetEdgeNeighbour(aIndex, i),
                                objB.mesh.VertGetEdgeNeighbour(bIndex, j));
                
                j++;
            }
        }
    }
    else //vf
    {
        //vertex of convex A
        //face of triangle B
        s32 i = 0;

        //for each edge neighbour incident to Vertex aIndex
        
        while ((edgeIndex = objA.mesh.VertGetEdgeNeighbour(aIndex, i)) != 0xff)
        {
            for (s32 j = 0; j < objB.mesh.numNeighbour; j++)
            {
                gEdgeStack.Push(objA.mesh.VertGetEdgeNeighbour(aIndex, i),
                                objB.mesh.FaceGetEdgeNeighbour(bIndex, j));
            }           
            i++;
        }
    }
    while (!gEdgeStack.IsEmpty())
    {
        _num_edge_test++;

        s32 edgeP, edgeQ;

        gEdgeStack.Pop(edgeP, edgeQ);

        // does the edge form a face
        neV3 a = objA.GetWorldNormalByEdge1(edgeP);

        neV3 b = objA.GetWorldNormalByEdge2(edgeP);

        neV3 c = objB.GetWorldNormalByEdge1(edgeQ) * -1.0f;

        neV3 d = objB.GetWorldNormalByEdge2(edgeQ) * -1.0f;

        c += (TriEdgeDir[edgeQ] * 0.01f);

        d += (TriEdgeDir[edgeQ] * 0.01f);

        c.Normalize();

        d.Normalize();

        f32 cba = Determinant(c,b,a);

        f32 dba = Determinant(d,b,a);

        f32 prod0 = cba * dba;

        if (prod0 >= -1.0e-6f)
        {
            continue;
        }

        f32 adc = Determinant(a,d,c);

        f32 bdc = Determinant(b,d,c);

        f32 prod1 = adc * bdc;

        if (prod1 >= -1.0e-6f)
        {
            continue;
        }
        f32 prod2 = cba * bdc;

        if (prod2 <= 1.0e-6f)
        {
            continue;
        }


        neV3 ai, bi;
        neV3 naj, nbj;

        objA.GetWorldEdgeVerts(edgeP, ai, bi);

        objB.GetWorldEdgeVerts(edgeQ, naj, nbj);
        
        naj *= -1.0f; nbj *= -1.0f;

        neV3 ainaj = ai + naj;
        neV3 ainbj = ai + nbj;
        neV3 binaj = bi + naj;
        //neV3 binbj = bi + nbj;

        neV3 diff1 = ainaj - ainbj;
        neV3 diff2 = ainaj - binaj ;

        Face testFace;

        testFace.normal = diff1.Cross(diff2);

        f32 len = testFace.normal.Length();

        if (neIsConsiderZero(len))
        {
            continue;
        }
        testFace.normal *= (1.0f / len);

        testFace.k = testFace.normal.Dot(ainaj);

        f32 testD = funcD(testFace);

        if (testD >= 0)
            return false;

        if (testD <= dMax)
            continue;

        assigned = true;
        dMax = testD;
        face = testFace;
        indexA = edgeP;
        indexB = edgeQ;
        typeA = SearchResult::EDGE; 
        typeB = SearchResult::EDGE;

        // push
        s32 i, j;

        s32 vindex;

        vindex = objB.mesh.EdgeGetVert1(edgeQ);

        i = 0;

        while ((j = objB.mesh.VertGetEdgeNeighbour(vindex, i)) != 0xff)
        {
            if (j != edgeQ)
                gEdgeStack.Push(edgeP, j);

            i++;
        }

        vindex = objB.mesh.EdgeGetVert2(edgeQ);

        i = 0;

        while ((j = objB.mesh.VertGetEdgeNeighbour(vindex, i)) != 0xff)
        {
            if (j != edgeQ)
                gEdgeStack.Push(edgeP, j);

            i++;
        }

        vindex = objA.mesh.EdgeGetVert1(edgeP);

        i = 0;
        
        while((j = objA.mesh.VertGetEdgeNeighbour(vindex, i)) != 0xff)
        {
            if (j != edgeP)
                gEdgeStack.Push(j, edgeQ);

            i++;
        }

        vindex = objA.mesh.EdgeGetVert2(edgeP);

        //for (i = 0; i < objA.mesh.VertGetNumEdgeNeighbour(vindex); i++)
        i = 0;

        while ((j = objA.mesh.VertGetEdgeNeighbour(vindex, i)) != 0xff)
        {
            if (j != edgeP)
                gEdgeStack.Push(j, edgeQ);

            i++;
        }
    }
    return true;
}

bool TestDCD(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB, const neV3 & backupVector)
{
    _num_edge_test = 0;

    _num_face_test = 0;

    result.penetrate = false;
    
    neV3 aPoint = transA.pos;

    neV3 av; av.Set(0.1f);

    aPoint += av;

    neV3 bPoint = transB.pos;

    av.Set(0.2f);

    bPoint += av;

    BigC = aPoint - bPoint;

    BigCLength = BigC.Length();

    neV3 * aVertArray, * bVertArray;

    if (convexA.type == TConvex::BOX)
    {
        for (s32 i = 0; i < BOX_NUM_VERTS; i++)
        {
            _boxVertexPosP[i] = _boxVertexPos0[i] * convexA.as.box.boxSize;
        }
        aVertArray = _boxVertexPosP;
    }
    if (convexB.type == TConvex::BOX)
    {
        for (s32 i = 0; i < BOX_NUM_VERTS; i++)
        {
            _boxVertexPosQ[i] = _boxVertexPos0[i] * convexB.as.box.boxSize;
        }
        bVertArray = _boxVertexPosQ;
    }

    SearchResult srFV(convexA, &transA, convexB, &transB, aVertArray, bVertArray);

    neBool showDebug = 0;
    
    neBool showDebug2  = (srFV.objA.mesh.numVerts > 8 && srFV.objB.mesh.numVerts > 8);
    
    neBool assigned;

    neBool res = srFV.SearchFV(0, assigned);

    if (!res)
    {
        if (showDebug)
        {TOKAMAK_OUTPUT_2("%d, %d \n", _num_face_test, _num_edge_test);}

        return false;
    }
    SearchResult srVF(convexB, &transB, convexA, &transA, bVertArray, aVertArray);

    srVF.dMax = srFV.dMax;

    BigC *= -1.0f;

    s32 whichF = srFV.objB.mesh.edges[srFV.objB.mesh.verts[srFV.indexB].neighbourEdges[0]].f1;

    res = srVF.SearchFV(whichF, assigned);
    
    if (!res)
    {
        if (showDebug)
            {TOKAMAK_OUTPUT_2("%d, %d \n", _num_face_test, _num_edge_test);}

        return false;
    }

    bool need2Swap = false;

    SearchResult srEE(convexA, &transA, convexB, &transB, aVertArray, bVertArray);

    s32 eeflag = 0;
    
    s32 pindex, qindex;

    if (srVF.dMax > srFV.dMax)
    {
        need2Swap = true;

        srEE.dMax = srVF.dMax;

        eeflag = 1;

        pindex = srVF.indexB;

        qindex = srVF.indexA;
    }
    else
    {
        srEE.dMax = srFV.dMax;

        pindex = srFV.indexA;

        qindex = srFV.indexB;
    }
    BigC *= -1.0f;

    if (!srEE.SearchEE(eeflag, pindex, qindex, assigned))
    {
        if (showDebug)
            {TOKAMAK_OUTPUT_2("%d, %d \n", _num_face_test, _num_edge_test);}

        return false;
    }
    if (showDebug2)
    {
        TOKAMAK_OUTPUT_2("%d, %d \n", _num_face_test, _num_edge_test);
    }
    if (!assigned)
    {
        if (!need2Swap)
        {
            ASSERT(srFV.typeA == SearchResult::FACE && srFV.typeB == SearchResult::VERTEX);

            result.penetrate = true;

            result.collisionFrame[2] = srFV.face.normal * -1.0f;

            result.depth = srFV.face.k;

            result.contactB = srFV.objB.GetVertWorld(srFV.indexB);

            result.contactA = result.contactB + srFV.face.normal * srFV.face.k;
        }
        else
        {
            ASSERT(srVF.typeA == SearchResult::FACE && srVF.typeB == SearchResult::VERTEX);

            result.penetrate = true;

            result.collisionFrame[2] = srVF.face.normal;

            result.depth = srVF.face.k;

            result.contactA = srVF.objB.GetVertWorld(srVF.indexB);

            result.contactB = result.contactA + srVF.face.normal * srVF.face.k;
        }
    }
    else
    {
        ASSERT(srEE.typeA == SearchResult::EDGE &&
                srEE.typeB == SearchResult::EDGE);

        neV3 edgeA[2];
        neV3 edgeB[2];

        srEE.objA.GetWorldEdgeVerts(srEE.indexA, edgeA[0], edgeA[1]);

        srEE.objB.GetWorldEdgeVerts(srEE.indexB, edgeB[0], edgeB[1]);

        bool r = CalcContactEE(edgeA[0], edgeA[1], edgeB[0], edgeB[1], result.contactA, result.contactB);

        if (r)
        {
            if (srEE.face.k > 0.0f)
            {
                result.collisionFrame[2] = srEE.face.normal * -1.0f;

                result.depth = srEE.face.k;
            }
            else
            {
                result.collisionFrame[2] = srEE.face.normal;

                result.depth = srEE.face.k * -1.0f;
            }

            result.penetrate = true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool TestDCDTri(ConvexTestResult & res, TConvex & convexA, neT3 & transA, const neV3 & insidePoint)
{
    res.valid = false;

    neV3 aPoint = transA.pos;

    neV3 av; av.Set(0.1f);

    aPoint += av;

    neV3 bPoint =  insidePoint;

    BigC = aPoint - bPoint;

    BigCLength = BigC.Length();

    neV3 * aVertArray = NULL, * bVertArray = NULL;

    TConvex dummyB; dummyB.type = TConvex::TRIANGLE;

    if (convexA.type == TConvex::BOX)
    {
        for (s32 i = 0; i < BOX_NUM_VERTS; i++)
        {
            _boxVertexPosP[i] = _boxVertexPos0[i] * convexA.as.box.boxSize;
        }
        aVertArray = _boxVertexPosP;
    }
    neT3 transB; transB.SetIdentity();

    SearchResult srBoxFaceTriVert(convexA, &transA, dummyB, &transB, aVertArray, bVertArray);

    neBool assigned;

    neBool r = srBoxFaceTriVert.SearchFV(0, assigned);

    if (!r)
        {return false;}

/*  bPoint = insidePoint;// + _triNormals[1];
    BigC = bPoint - aPoint;
    BigCLength = BigC.Length();
*/
    BigC *= -1.0f;

    SearchResult srBoxVertTriFace(dummyB, &transB, convexA, &transA, bVertArray, aVertArray);

    if (!(r = srBoxVertTriFace.TestFace(0, assigned)))
        return false;

    //BigC *= -1.0f;
/*
    bPoint = insidePoint + _triNormals[0];
    BigC = bPoint - aPoint;
    BigCLength = BigC.Length();
*/
    neBool assigned2;

    if (!(r = srBoxVertTriFace.TestFace(1, assigned2)))
        return false;

    assigned |= assigned2;

    BigC *= -1.0f;
/*
    bPoint = insidePoint;
    BigC = aPoint - bPoint;
    BigCLength = BigC.Length();
*/

    neBool need2Swap = false;

    SearchResult srBoxTriEE(convexA, &transA, dummyB, &transB, aVertArray, bVertArray);

    s32 eeflag = 0;

    s32 pindex, qindex;

    if (srBoxVertTriFace.dMax > srBoxFaceTriVert.dMax)
    {
        need2Swap = true;

        srBoxTriEE.dMax = srBoxVertTriFace.dMax;

        eeflag = 1;

        pindex = srBoxVertTriFace.indexB; // vertex of Convex

        qindex = srBoxVertTriFace.indexA; // face of Triangle
    }
    else
    {
        srBoxTriEE.dMax = srBoxFaceTriVert.dMax;

        pindex = srBoxFaceTriVert.indexA; // face of Convex

        qindex = srBoxFaceTriVert.indexB; // vertex of Triangle
    }
    //BigC *= -1.0f;

    if (!srBoxTriEE.SearchEETri(eeflag, pindex, qindex, assigned))
    {
        return false;
    }
    if (!assigned)
    {
FV_Backup:
        if (!need2Swap)
        {
            ASSERT(srBoxFaceTriVert.typeA == SearchResult::FACE && srBoxFaceTriVert.typeB == SearchResult::VERTEX);

            res.valid = true;

            res.contactNormal = srBoxFaceTriVert.face.normal * -1.0f;

            res.depth = srBoxFaceTriVert.face.k;

            res.contactB = srBoxFaceTriVert.objB.GetVertWorld(srBoxFaceTriVert.indexB);

            res.contactA = res.contactB + srBoxFaceTriVert.face.normal * srBoxFaceTriVert.face.k;
        }
        else
        {
            ASSERT(srBoxVertTriFace.typeA == SearchResult::FACE && srBoxVertTriFace.typeB == SearchResult::VERTEX);

            res.valid = true;

            res.contactNormal = srBoxVertTriFace.face.normal;

            res.depth = srBoxVertTriFace.face.k;

            res.contactA = srBoxVertTriFace.objB.GetVertWorld(srBoxVertTriFace.indexB);

            res.contactB = res.contactA + srBoxVertTriFace.face.normal * srBoxVertTriFace.face.k;
        }
    }
    else
    {
        ASSERT(srBoxTriEE.typeA == SearchResult::EDGE &&
                srBoxTriEE.typeB == SearchResult::EDGE);

        neV3 edgeA[2];
        neV3 edgeB[2];

        srBoxTriEE.objA.GetWorldEdgeVerts(srBoxTriEE.indexA, edgeA[0], edgeA[1]);

        srBoxTriEE.objB.GetWorldEdgeVerts(srBoxTriEE.indexB, edgeB[0], edgeB[1]);

        bool r = CalcContactEE(edgeA[0], edgeA[1], edgeB[0], edgeB[1], res.contactA, res.contactB);

        if (r)
        {
            if (srBoxTriEE.face.k > 0.0f)
            {
                res.contactNormal = srBoxTriEE.face.normal * -1.0f;

                res.depth = srBoxTriEE.face.k;
            }
            else
            {
                res.contactNormal = srBoxTriEE.face.normal;

                res.depth = srBoxTriEE.face.k * -1.0f;
            }

            res.valid = true;
        }
        else
        {
            //return false;
            goto FV_Backup;
        }
    }
    return true;
}

void Convex2TerrainTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB)
{
    neSimpleArray<s32> & _triIndex = *convexB.as.terrain.triIndex;

    s32 triangleCount = _triIndex.GetUsedCount();

    neArray<neTriangle_> & triangleArray = *convexB.as.terrain.triangles;

    ConvexTestResult res[2];

    s32 finalTriIndex = -1;
    s32 currentRes = 1;
    s32 testRes = 0;

    res[currentRes].depth = -1.0e6f;
    res[currentRes].valid = false;
    res[testRes].depth = 1.0e6f;
    
    s32 terrainMatID = 0;

    neBool found = false;
#if 0
    for (s32 j = 0/*triangleCount-1*/; j < triangleCount; j++)
    //int j = 12;
    {
        neV3 points[4];
        neV3 red;red.Set(1.0f);

        neTriangle_ * t =  &triangleArray[_triIndex[j]];

        points[0] = convexB.vertices[t->indices[0]];
        points[1] = convexB.vertices[t->indices[1]];
        points[2] = convexB.vertices[t->indices[2]];
        points[3] = convexB.vertices[t->indices[0]];
        extern void DrawLine(const neV3 & colour, neV3 * startpoint, s32 count);
        DrawLine(red, points, 4);
    }
#endif
    for (s32 i = 0; i < triangleCount; i++)
    {
        s32 test = _triIndex[i];

        neTriangle_ * t = &triangleArray[_triIndex[i]];

        _triVertexPos[0] = convexB.vertices[t->indices[0]];
        _triVertexPos[1] = convexB.vertices[t->indices[1]];
        _triVertexPos[2] = convexB.vertices[t->indices[2]];

        neV3 diff1 = _triVertexPos[1] - _triVertexPos[0];
        neV3 diff2 = _triVertexPos[2] - _triVertexPos[1];
        neV3 diff3 = _triVertexPos[0] - _triVertexPos[2];

        _triNormals[0] = diff1.Cross(diff2);

        _triNormals[0].Normalize();

        _triNormals[1] = -_triNormals[0];

        _triNormals[0].v[3] = _triNormals[0].Dot(_triVertexPos[0]);

        _triNormals[1].v[3] = -_triNormals[0].v[3];

        TriEdgeDir[0] = _triNormals[0].Cross(diff1);
        TriEdgeDir[1] = _triNormals[0].Cross(diff2);
        TriEdgeDir[2] = _triNormals[0].Cross(diff3);
        TriEdgeDir[0].Normalize();
        TriEdgeDir[1].Normalize();
        TriEdgeDir[2].Normalize();

        neV3 insidePoint = _triVertexPos[0] + _triVertexPos[1] + _triVertexPos[2];

        insidePoint *= (1.0f / 3.0f);

        //insidePoint += (_triNormals[1] * 0.1f);

        if (TestDCDTri(res[testRes], convexA, transA, insidePoint))
        {
            if (res[testRes].depth > res[currentRes].depth)
            {
                s32 tmp = testRes;  

                testRes = currentRes;

                currentRes = tmp;

                terrainMatID = t->materialID;

                finalTriIndex = _triIndex[i];

                found = true;
            }
        }
    }
    if (found)
    {
        result.penetrate = true;

        result.depth = res[currentRes].depth;

        result.collisionFrame[2] = res[currentRes].contactNormal;

        result.materialIdB = terrainMatID;

        result.contactA = res[currentRes].contactA;

        result.contactB = res[currentRes].contactB;
    }
    else
    {
        result.penetrate = false;
    }
}

bool CalcContactEE(const neV3 & edgeA0, 
                    const neV3 & edgeA1, 
                    const neV3 & edgeB0, 
                    const neV3 & edgeB1, neV3 & contactA, neV3 & contactB)
{
    f32 d1343, d4321, d1321, d4343, d2121;
    f32 numer, denom, au, bu;
    
    neV3 p13;
    neV3 p43;
    neV3 p21;

    p13 = (edgeA0) - (edgeB0);
    p43 = (edgeB1) - (edgeB0);

    if ( p43.IsConsiderZero() )
    {
        goto CalcContactEE_Exit;
    }
    
    p21 = (edgeA1) - (edgeA0);

    if ( p21.IsConsiderZero() )
    {
        goto CalcContactEE_Exit;
    }
    
    d1343 = p13.Dot(p43);
    d4321 = p43.Dot(p21);
    d1321 = p13.Dot(p21);
    d4343 = p43.Dot(p43);
    d2121 = p21.Dot(p21);

    denom = d2121 * d4343 - d4321 * d4321;   

    if (neAbs(denom) < NE_ZERO) 
        goto CalcContactEE_Exit;

    numer = d1343 * d4321 - d1321 * d4343;
    au = numer / denom;   
    bu = (d1343 + d4321 * (au)) / d4343;

    if (au < 0.0f || au >= 1.0f)
        goto CalcContactEE_Exit;
    
    if (bu < 0.0f || bu >= 1.0f)
        goto CalcContactEE_Exit;

    {
        neV3 tmpv;

        tmpv = p21 * au;
        contactA = (edgeA0) + tmpv;

        tmpv = p43 * bu;
        contactB = (edgeB0) + tmpv;
    }

    return true;

CalcContactEE_Exit:

    return false;
}


/**lines.cpp**/

//#include <stdio.h>
/*
#ifdef _WIN32
#include <windows.h>
#endif
*/
//#include "tokamak.h"
//#include "containers.h"
//#include "scenery.h"
//#include "collision.h"
//#include "constraint.h"
//#include "rigidbody.h"
//#include "scenery.h"
//#include "stack.h"
//#include "simulator.h"
//#include "message.h"

void CollisionTestSensor(TConvex * obbA, neSensor_ * sensorsA, neT3 & transA, neCollision & colB, neT3 & transB, neRigidBodyBase * body)
{
    neT3 convex2WorldB;

    convex2WorldB = transB * colB.obb.c2p;
    
    neT3 world2convexB;

    world2convexB = convex2WorldB.FastInverse();

    neT3 AtoB;

    AtoB = world2convexB * transA;

    if (colB.convexCount == 1)
    {
        neSensorItem * si = (neSensorItem *)sensorsA;

        while (si)
        {
            neSensor_ * s = (neSensor_ *) si;

            si = si->next;

            neSensor_ tmp = *s;

            tmp.depth = 0.0f;
            
            tmp.pos = AtoB * s->pos;

            tmp.dir = AtoB.rot * s->dir;

            tmp.length = s->length;

            SensorTest(tmp, colB.obb, convex2WorldB);

            if (tmp.depth > 0.0f && tmp.depth > s->depth)
            {
                s->depth = tmp.depth;
                s->body = tmp.body;
                s->materialID = tmp.materialID;
                s->normal = tmp.normal;
                s->contactPoint = convex2WorldB * tmp.contactPoint;
                s->body = body;
/*
                char ss[256];

                sprintf(ss, "normal = %f, %f, %f \n", s->normal[0], s->normal[1], s->normal[2]);
                OutputDebugString(ss);
*/          }
        }
    }
    else
    {
        neSensorItem * si = (neSensorItem *)sensorsA;

        while (si)
        {
            neSensor_ * s = (neSensor_ *) si;

            si = si->next;

            neSensor_ tmp = *s;

            tmp.depth = 0.0f;

            TConvexItem * ti = (TConvexItem *)colB.convex;

            while (ti)
            {
                TConvex * t = (TConvex *)ti;

                ti = ti->next;

                convex2WorldB = transB * t->c2p;

                world2convexB = convex2WorldB.FastInverse();

                AtoB = world2convexB * transA;

                tmp.pos = AtoB * s->pos;

                tmp.dir = AtoB.rot * s->dir;

                tmp.length = s->length;

                //SensorTest(tmp, colB.obb, convex2WorldB);
                SensorTest(tmp, *t, convex2WorldB);

                if (tmp.depth > 0.0f && tmp.depth > s->depth)
                {
                    s->depth = tmp.depth;
                    s->body = tmp.body;
                    s->materialID = tmp.materialID;
                    s->normal = tmp.normal;
                    s->contactPoint = convex2WorldB * tmp.contactPoint;
                    s->body = body;
                }
            }
        }
    }
}

NEINLINE neBool SameSide(const neV3 & p1, const neV3 & p2, const neV3 & a, const neV3 & edge)
{
    neV3 cp1 = edge.Cross(p1 - a);
    
    neV3 cp2 = edge.Cross(p2 - a);

    f32 dot = cp1.Dot(cp2);

    return (dot >= 0.0f);
}

/*
 *  
        neBool found = false;

        f32 dist, ratio, factor, depth;

        neV3 contact;

        s32 i;

        for (i = 0; i < 3; i++)
        {
            if (!neIsConsiderZero(sensorA.dir[i]))
            {
                if (sensorA.dir[i] > 0.0f)
                {
                    if (sensorA.pos[i] > -convexB.as.box.boxSize[i])
                        continue;
        
                    factor = 1.0f;
                }
                else
                {
                    if (sensorA.pos[i] < convexB.as.box.boxSize[i])
                        continue;

                    factor = -1.0f;
                }
                dist = factor * (convexB.as.box.boxSize[i] - sensorA.pos[i]);

                ASSERT(dist > 0.0f);

                if (dist > neAbs(sensorA.dir[i]))
                    return;

                ratio = dist / neAbs(sensorA.dir[i]);

                contact = sensorA.pos + sensorA.dir * ratio;

                s32 other1, other2;

                other1 = (i + 1)%3;
                
                other2 = (i + 2)%3;

                if (contact[other1] >= convexB.as.box.boxSize[other1] || contact[other1] <= -convexB.as.box.boxSize[other1])
                    continue;

                if (contact[other2] >= convexB.as.box.boxSize[other2] || contact[other2] <= -convexB.as.box.boxSize[other2])
                    continue;

                found = true;

                depth = (1.0f - ratio) * sensorA.length;

                break;
            }
            else if (sensorA.pos[i] >= convexB.as.box.boxSize[i] || sensorA.pos[i] <= -convexB.as.box.boxSize[i])
            {
                return;
            }
        }
        if (found)
        {
            sensorA.depth = depth;

            sensorA.normal = transB.rot[i] * factor * -1.0f;

            sensorA.contactPoint = contact;

            sensorA.materialID = convexB.matIndex;
        }

 */
void SensorTest(neSensor_ & sensorA, TConvex & convexB, neT3 & transB)
{
    if (convexB.type == TConvex::BOX)
    {
        int nearDim = -1;
        int farDim = -1;

//  set Tnear = - infinity, Tfar = infinity
//  For each pair of planes P associated with X, Y, and Z do:
//  (example using X planes)
//  if direction Xd = 0 then the ray is parallel to the X planes, so
//  if origin Xo is not between the slabs ( Xo < Xl or Xo > Xh) then return false
//  else, if the ray is not parallel to the plane then
//  begin
//  compute the intersection distance of the planes
//  T1 = (Xl - Xo) / Xd
//  T2 = (Xh - Xo) / Xd
//  If T1 > T2 swap (T1, T2) /* since T1 intersection with near plane */
//  If T1 > Tnear set Tnear =T1 /* want largest Tnear */
//  If T2 < Tfar set Tfar="T2" /* want smallest Tfar */
//  If Tnear > Tfar box is missed so return false
//  If Tfar < 0 box is behind ray return false end


        float tNear = -1.0e6;
        float tFar = 1.0e6;

        for (int i = 0; i < 3; i++)
        {
            if (neIsConsiderZero(sensorA.dir[i]))
            {
                if (sensorA.pos[i] < -convexB.as.box.boxSize[i] ||
                    sensorA.pos[i] > convexB.as.box.boxSize[i])
                {
                    return;
                }
            }
            float t1 = (-convexB.as.box.boxSize[i] - sensorA.pos[i]) / sensorA.dir[i];
            
            float t2 = (convexB.as.box.boxSize[i] - sensorA.pos[i]) / sensorA.dir[i];

            float tt;

            if (t1 > t2)
            {
                tt = t1;
                t1 = t2;
                t2 = tt;
            }

            if (t1 > tNear)
            {
                tNear = t1;
                nearDim = i;
            }

            if (t2 < tFar)
            {
                tFar = t2;
                farDim = i;
            }

            if (tNear > tFar)
                return;

            if (tFar < 0)
                return;

        }
        //assert(nearDim != -1);
        //assert(farDim != -1);

        if (tNear > 1.0f)
            return;

        neV3 contact = sensorA.pos + tNear * sensorA.dir;

        neV3 sensorEnd = sensorA.pos + sensorA.dir;

        f32 depth = (sensorEnd - contact).Length();

        sensorA.depth = depth;

        f32 factor = (sensorA.dir[nearDim] >= 0) ? -1.0f : 1.0f;
        sensorA.normal = transB.rot[nearDim] * factor;

        sensorA.contactPoint = contact;

        sensorA.materialID = convexB.matIndex;
    }
    else if (convexB.type == TConvex::TERRAIN)
    {
        neSimpleArray<s32> & _triIndex = *convexB.as.terrain.triIndex;

        s32 triangleCount = _triIndex.GetUsedCount();

        neArray<neTriangle_> & triangleArray = *convexB.as.terrain.triangles;
        
        for (s32 i = 0; i < triangleCount; i++)
        {
            s32 test = _triIndex[i];

            neTriangle_ * t = &triangleArray[_triIndex[i]];

            neV3 * vert[3];

            neV3 edges[3];

            neV3 normal;

            f32 d;

            vert[0] = &convexB.vertices[t->indices[0]];
            vert[1] = &convexB.vertices[t->indices[1]];
            vert[2] = &convexB.vertices[t->indices[2]];

            edges[0] = *vert[1] - *vert[0];
            edges[1] = *vert[2] - *vert[1];
            edges[2] = *vert[0] - *vert[2];
            normal = edges[0].Cross(edges[1]);

            normal.Normalize();

            d = normal.Dot(*vert[0]);

            f32 nd = normal.Dot(sensorA.dir);

            f32 np = normal.Dot(sensorA.pos);

            f32 t1;

            t1 = (d - np) / nd;

            if (t1 > 1.0f || t1 < 0.0f)
                continue;

            neV3 contactPoint = sensorA.pos + sensorA.dir * t1;

            if (!SameSide(contactPoint, *vert[2], *vert[0], edges[0]))
                continue;

            if (!SameSide(contactPoint, *vert[0], *vert[1], edges[1]))
                continue;

            if (!SameSide(contactPoint, *vert[1], *vert[2], edges[2]))
                continue;

            sensorA.depth = (1.0f - t1) * sensorA.length;

            if (nd > 0.0)
                sensorA.normal = normal * -1.0f;
            else
                sensorA.normal = normal;

            sensorA.contactPoint = contactPoint;

            sensorA.materialID = t->materialID;
        }
    }
    else
    {
        // other primitives to do
    }
}


/**ne_interface.cpp**/

//#include "math/ne_type.h"
//#include "math/ne_debug.h"
//#include "tokamak.h"
//#include "containers.h"
//#include "scenery.h"
//#include "collision.h"
//#include "constraint.h"
//#include "rigidbody.h"

#ifdef _WIN32
#include <windows.h>
#endif

//#include "stack.h"
//#include "simulator.h"
//#include "message.h"

//#include <stdio.h>

#define CAST_THIS(a, b) a& b = reinterpret_cast<a&>(*this);

#ifdef TOKAMAK_COMPILE_DLL

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

#endif

/****************************************************************************
*
*   neGeometry::SetBoxSize
*
****************************************************************************/ 

void neGeometry::SetBoxSize(f32 width, f32 height, f32 depth)
{
    CAST_THIS(TConvex, con);

    con.SetBoxSize(width, height, depth);
}

/****************************************************************************
*
*   neGeometry::SetBoxSize
*
****************************************************************************/ 

void neGeometry::SetBoxSize(const neV3 & boxSize)
{
    CAST_THIS(TConvex, con);

    con.SetBoxSize(boxSize[0], boxSize[1], boxSize[2]);
}

/****************************************************************************
*
*   neGeometry::SetCylinder
*
****************************************************************************/ 

void neGeometry::SetCylinder(f32 diameter, f32 height)
{
    CAST_THIS(TConvex, con);

    con.type = TConvex::CYLINDER;

    con.as.cylinder.radius = diameter * 0.5f;

    con.as.cylinder.radiusSq = con.as.cylinder.radius * con.as.cylinder.radius;

    con.as.cylinder.halfHeight = height * 0.5f;
}

/****************************************************************************
*
*   neGeometry::GetCylinder
*
****************************************************************************/ 

neBool neGeometry::GetCylinder(f32 & diameter, f32 & height) // return false if geometry is not a cylinder
{
    CAST_THIS(TConvex, con);

    if (con.type != TConvex::CYLINDER)
        return false;

    diameter = con.CylinderRadius() * 2.0f;

    height = con.CylinderHalfHeight() * 2.0f;

    return true;
}

/****************************************************************************
*
*   neGeometry::SetConvexMesh
*
****************************************************************************/ 

void neGeometry::SetConvexMesh(neByte * convexData)
{
    CAST_THIS(TConvex, con);

    con.SetConvexMesh(convexData);
}

/****************************************************************************
*
*   neGeometry::GetConvexMesh
*
****************************************************************************/ 

neBool neGeometry::GetConvexMesh(neByte *& convexData)
{
    CAST_THIS(TConvex, con);

    if (con.type != TConvex::CONVEXDCD)
        return false;

    convexData = con.as.convexDCD.convexData;

    return true;
}

/****************************************************************************
*
*   neGeometry::SetTransform
*
****************************************************************************/ 

void neGeometry::SetTransform(neT3 & t)
{
    CAST_THIS(TConvex, con);

    con.SetTransform(t);
}

/****************************************************************************
*
*   neGeometry::SetMaterialIndex
*
****************************************************************************/ 

void neGeometry::SetMaterialIndex(s32 index)
{
    CAST_THIS(TConvex, con);

    con.SetMaterialId(index);
}

/****************************************************************************
*
*   neGeometry::GetMaterialIndex
*
****************************************************************************/ 

s32 neGeometry::GetMaterialIndex()
{
    CAST_THIS(TConvex, con);

    return con.matIndex;
}

/****************************************************************************
*
*   neGeometry::GetTransform
*
****************************************************************************/ 

neT3 neGeometry::GetTransform()
{
    CAST_THIS(TConvex, con);

    return con.c2p;
}

/****************************************************************************
*
*   neGeometry::SetUserData
*
****************************************************************************/ 

void neGeometry::SetUserData(uintptr_t userData)
{
    CAST_THIS(TConvex, con);

    con.userData = userData;
}

/****************************************************************************
*
*   neGeometry::GetUserData
*
****************************************************************************/ 

uintptr_t neGeometry::GetUserData()
{
    CAST_THIS(TConvex, con);

    return con.userData;
}

/****************************************************************************
*
*   neGeometry::GetBoxSize
*
****************************************************************************/ 

neBool neGeometry::GetBoxSize(neV3 & boxSize) // return false if geometry is not a box
{
    CAST_THIS(TConvex, con);

    if (con.type != TConvex::BOX)
        return false;

    boxSize = con.as.box.boxSize * 2.0f;

    return true;
}

/****************************************************************************
*
*   neGeometry::SetSphereDiameter
*
****************************************************************************/ 

void neGeometry::SetSphereDiameter(f32 diameter)
{
    CAST_THIS(TConvex, con);

    con.type = TConvex::SPHERE;

    con.as.sphere.radius = diameter * 0.5f;

    con.as.sphere.radiusSq = con.as.sphere.radius * con.as.sphere.radius;
}

/****************************************************************************
*
*   neGeometry::GetSphereDiameter
*
****************************************************************************/ 

neBool neGeometry::GetSphereDiameter(f32 & diameter) // return false if geometry is not a sphere
{
    CAST_THIS(TConvex, con);

    if (con.type != TConvex::SPHERE)
        return false;

    diameter = con.Radius() * 2.0f;

    return true;
}

/****************************************************************************
*
*   neGeometry::SetBreakFlag
*
****************************************************************************/ 

void neGeometry::SetBreakageFlag(neBreakFlag flag)
{
    CAST_THIS(TConvex, con);

    con.breakInfo.flag = flag;
}

/****************************************************************************
*
*   neGeometry::GetBreakFlag
*
****************************************************************************/ 

neGeometry::neBreakFlag neGeometry::GetBreakageFlag()
{
    CAST_THIS(TConvex, con);

    return con.breakInfo.flag;
}

/****************************************************************************
*
*   neGeometry::SetBreakageMass
*
****************************************************************************/ 

void neGeometry::SetBreakageMass(f32 mass)
{
    CAST_THIS(TConvex, con);

    con.breakInfo.mass = mass;
}

/****************************************************************************
*
*   neGeometry::GetBreakageMass
*
****************************************************************************/ 

f32 neGeometry::GetBreakageMass()
{
    CAST_THIS(TConvex, con);

    return con.breakInfo.mass;
}

/****************************************************************************
*
*   neGeometry::SetBreakageInertiaTensor
*
****************************************************************************/ 

void neGeometry::SetBreakageInertiaTensor(const neV3 & tensor)
{
    CAST_THIS(TConvex, con);

    con.breakInfo.inertiaTensor = tensor;
}

/****************************************************************************
*
*   neGeometry::GetBreakageInertiaTensor
*
****************************************************************************/ 

neV3 neGeometry::GetBreakageInertiaTensor()
{
    CAST_THIS(TConvex, con);

    return con.breakInfo.inertiaTensor;
}

/****************************************************************************
*
*   neGeometry::SetBreakageMagnitude
*
****************************************************************************/ 

void neGeometry::SetBreakageMagnitude(f32 mag)
{
    CAST_THIS(TConvex, con);

    con.breakInfo.breakMagnitude = mag;
}

/****************************************************************************
*
*   neGeometry::GetBreakageMagnitude
*
****************************************************************************/ 

f32 neGeometry::GetBreakageMagnitude()
{
    CAST_THIS(TConvex, con);

    return con.breakInfo.breakMagnitude;
}

/****************************************************************************
*
*   neGeometry::SetBreakageAbsorption
*
****************************************************************************/ 

void neGeometry::SetBreakageAbsorption(f32 absorb)
{
    CAST_THIS(TConvex, con);

    con.breakInfo.breakAbsorb = absorb;
}

void neGeometry::SetBreakagePlane(const neV3 & planeNormal)
{
    CAST_THIS(TConvex, con);

    con.breakInfo.breakPlane = planeNormal;
}

neV3 neGeometry::GetBreakagePlane()
{
    CAST_THIS(TConvex, con);

    return con.breakInfo.breakPlane;
}

/****************************************************************************
*
*   neGeometry::GetBreakageAbsorption
*
****************************************************************************/ 

f32 neGeometry::GetBreakageAbsorption()
{
    CAST_THIS(TConvex, con);

    return con.breakInfo.breakAbsorb;
}

/****************************************************************************
*
*   neGeometry::SetBreakNeighbourRadius
*
****************************************************************************/ 

void neGeometry::SetBreakageNeighbourRadius(f32 radius)
{
    CAST_THIS(TConvex, con);

    con.breakInfo.neighbourRadius = radius;
}

/****************************************************************************
*
*   neGeometry::GetBreakNeighbourRadius
*
****************************************************************************/ 

f32 neGeometry::GetBreakageNeighbourRadius()
{
    CAST_THIS(TConvex, con);

    return con.breakInfo.neighbourRadius;
}

/****************************************************************************
*
*   neAnimatedBody::GetPos
*
****************************************************************************/ 

neV3 neAnimatedBody::GetPos()
{
    CAST_THIS(neCollisionBody_, cb);

    return cb.b2w.pos;
}

/****************************************************************************
*
*   neAnimatedBody::SetPos
*
****************************************************************************/ 

void neAnimatedBody::SetPos(const neV3 & p)
{
    CAST_THIS(neCollisionBody_, cb);

    cb.b2w.pos = p;

    cb.UpdateAABB();

    cb.moved = true;
}

/****************************************************************************
*
*   neAnimatedBody::GetRotationM3
*
****************************************************************************/ 

neM3 neAnimatedBody::GetRotationM3()
{
    CAST_THIS(neCollisionBody_, cb);

    return cb.b2w.rot;
}

/****************************************************************************
*
*   neAnimatedBody::GetRotationQ
*
****************************************************************************/ 

neQ neAnimatedBody::GetRotationQ()
{
    CAST_THIS(neCollisionBody_, cb);

    neQ q;

    q.SetupFromMatrix3(cb.b2w.rot);

    return q;
}

/****************************************************************************
*
*   neAnimatedBody::SetRotation
*
****************************************************************************/ 

void neAnimatedBody::SetRotation(const neM3 & m)
{
    CAST_THIS(neCollisionBody_, cb);

    cb.b2w.rot = m;

    cb.moved = true;
}

/****************************************************************************
*
*   neAnimatedBody::SetRotation
*
****************************************************************************/ 

void neAnimatedBody::SetRotation(const neQ & q)
{
    CAST_THIS(neCollisionBody_, cb);

    cb.b2w.rot = q.BuildMatrix3();

    cb.moved = true;
}

/****************************************************************************
*
*   neAnimatedBody::GetTransform
*
****************************************************************************/ 

neT3 neAnimatedBody::GetTransform()
{
    CAST_THIS(neCollisionBody_, cb);

    return cb.b2w;
}

/****************************************************************************
*
*   neAnimatedBody::SetCollisionID
*
****************************************************************************/ 

void neAnimatedBody::SetCollisionID(s32 cid)
{
    CAST_THIS(neCollisionBody_, cb);

    cb.cid = cid;
}

/****************************************************************************
*
*   neAnimatedBody::GetCollisionID
*
****************************************************************************/ 

s32 neAnimatedBody::GetCollisionID()
{
    CAST_THIS(neCollisionBody_, cb);

    return cb.cid;
}

/****************************************************************************
*
*   neAnimatedBody::SetUserData
*
****************************************************************************/ 

void neAnimatedBody::SetUserData(uintptr_t cookies)
{
    CAST_THIS(neCollisionBody_, cb);

    cb.cookies = cookies;
}

/****************************************************************************
*
*   neAnimatedBody::GetUserData
*
****************************************************************************/ 

uintptr_t   neAnimatedBody::GetUserData()
{
    CAST_THIS(neCollisionBody_, cb);

    return cb.cookies;
}

/****************************************************************************
*
*   neAnimatedBody::GetGeometryCount
*
****************************************************************************/ 

s32 neAnimatedBody::GetGeometryCount()
{
    CAST_THIS(neCollisionBody_, cb);

    return cb.col.convexCount;
}

/****************************************************************************
*
*   neAnimatedBody::GetGeometry
*
****************************************************************************/ 
/*
neGeometry * neAnimatedBody::GetGeometry(s32 index)
{
    CAST_THIS(neCollisionBody_, cb);

    return reinterpret_cast<neGeometry*>(cb.GetConvex(index));
}
*/
/****************************************************************************
*
*   neAnimatedBody::SetGeometry
*
****************************************************************************/ 
/*
void neAnimatedBody::SetGeometry(s32 geometryCount, neGeometry * geometryArray)
{
    CAST_THIS(neCollisionBody_, cb);

    //todo
}
*/
/****************************************************************************
*
*   neAnimatedBody::UpdateBoundingInfo
*
****************************************************************************/ 

void neAnimatedBody::UpdateBoundingInfo()
{
    CAST_THIS(neRigidBodyBase, rb);

    rb.RecalcBB();
}

/****************************************************************************
*
*   neAnimatedBody::CollideConnected
*
****************************************************************************/ 

void neAnimatedBody::CollideConnected(neBool yes)
{
    CAST_THIS(neRigidBodyBase, rb);

    rb.CollideConnected(yes);
}

/****************************************************************************
*
*   neAnimatedBody::IsCollideConnected
*
****************************************************************************/ 

neBool neAnimatedBody::CollideConnected()
{
    CAST_THIS(neRigidBodyBase, rb);

    return rb.CollideConnected();
}

/****************************************************************************
*
*   neAnimatedBody::CollideDirectlyConnected
*
****************************************************************************/ 

void neAnimatedBody::CollideDirectlyConnected(neBool yes)
{
    CAST_THIS(neRigidBodyBase, rb);

    rb.isCollideDirectlyConnected = yes;
}

/****************************************************************************
*
*   neAnimatedBody::CollideDirectlyConnected
*
****************************************************************************/ 

neBool neAnimatedBody::CollideDirectlyConnected()
{
    CAST_THIS(neRigidBodyBase, rb);

    return rb.isCollideDirectlyConnected;
}

/****************************************************************************
*
*   neAnimatedBody::AddGeometry
*
****************************************************************************/ 

neGeometry * neAnimatedBody::AddGeometry()
{
    CAST_THIS(neCollisionBody_, ab);

    TConvex * g = ab.AddGeometry();

    return reinterpret_cast<neGeometry *>(g);
}

/****************************************************************************
*
*   neAnimatedBody::RemoveGeometry
*
****************************************************************************/ 

neBool neAnimatedBody::RemoveGeometry(neGeometry * g)
{
    CAST_THIS(neCollisionBody_, ab);

    if (!ab.col.convex)
        return false;

    TConvexItem * gi = (TConvexItem *)ab.col.convex;

    while (gi)
    {
        TConvex * convex = reinterpret_cast<TConvex *>(gi);

        gi = gi->next;

        if (convex == reinterpret_cast<TConvex *>(g))
        {
            if (ab.col.convex == convex)
            {
                ab.col.convex = (TConvex*)gi;
            }

            ab.sim->geometryHeap.Dealloc(convex, 1);

            ab.col.convexCount--;

            if (ab.col.convexCount == 0)
            {
                ab.col.convex = NULL;

                if (ab.IsInRegion() && !ab.isCustomCD)
                    ab.sim->region.RemoveBody(&ab);
            }

            return true;
        }
    }
    return false;
}

/****************************************************************************
*
*   neAnimatedBody::BeginIterateGeometry
*
****************************************************************************/ 

void neAnimatedBody::BeginIterateGeometry()
{
    CAST_THIS(neCollisionBody_, ab);

    ab.BeginIterateGeometry();
}

/****************************************************************************
*
*   neAnimatedBody::GetNextGeometry
*
****************************************************************************/ 

neGeometry * neAnimatedBody::GetNextGeometry()
{
    CAST_THIS(neCollisionBody_, ab);

    return reinterpret_cast<neGeometry *>(ab.GetNextGeometry());
}

/****************************************************************************
*
*   neAnimatedBody::BreakGeometry
*
****************************************************************************/ 

neRigidBody * neAnimatedBody::BreakGeometry(neGeometry * g)
{
    CAST_THIS(neCollisionBody_, ab);

    neRigidBody_ * newBody = ab.sim->CreateRigidBodyFromConvex((TConvex*)g, &ab);

    return (neRigidBody *)newBody;
}

/****************************************************************************
*
*   neAnimatedBody::UseCustomCollisionDetection
*
****************************************************************************/ 

void neAnimatedBody::UseCustomCollisionDetection(neBool yes,  const neT3 * obb, f32 boundingRadius)
{
    CAST_THIS(neCollisionBody_, ab);

    if (yes)
    {
        ab.obb = *obb;

        ab.col.boundingRadius = boundingRadius;

        ab.isCustomCD = yes;

        if (ab.isActive && !ab.IsInRegion())
        {
            ab.sim->region.AddBody(&ab, NULL);
        }
    }
    else
    {
        ab.isCustomCD = yes;

        this->UpdateBoundingInfo();

        if (ab.IsInRegion() && GetGeometryCount() == 0)
        {
            ab.sim->region.RemoveBody(&ab);
        }
    }
}

/****************************************************************************
*
*   neAnimatedBody::UseCustomCollisionDetection
*
****************************************************************************/ 

neBool neAnimatedBody::UseCustomCollisionDetection()
{
    CAST_THIS(neCollisionBody_, ab);

    return ab.isCustomCD;
}

/****************************************************************************
*
*   neAnimatedBody::AddSensor
*
****************************************************************************/ 

neSensor * neAnimatedBody::AddSensor()
{
    CAST_THIS(neRigidBodyBase, ab);

    neSensor_ * s = ab.AddSensor();

    return reinterpret_cast<neSensor *>(s);
}

/****************************************************************************
*
*   neAnimatedBody::RemoveSensor
*
****************************************************************************/ 

neBool neAnimatedBody::RemoveSensor(neSensor * s)
{
    CAST_THIS(neRigidBodyBase, ab);

    if (!ab.sensors)
        return false;

    neSensorItem * si = (neSensorItem *)ab.sensors;

    while (si)
    {
        neSensor_ * sensor = (neSensor_ *) si;

        si = si->next;

        if (sensor == reinterpret_cast<neSensor_*>(s))
        {
            //reinterpret_cast<neSensorItem *>(s)->Remove();

            ab.sim->sensorHeap.Dealloc(sensor, 1);

            return true;
        }
    }
    return false;
}

/****************************************************************************
*
*   neAnimatedBody::BeginIterateSensor
*
****************************************************************************/ 

void neAnimatedBody::BeginIterateSensor()
{
    CAST_THIS(neRigidBodyBase, ab);

    ab.BeginIterateSensor();
}

/****************************************************************************
*
*   neAnimatedBody::GetNextSensor
*
****************************************************************************/ 

neSensor * neAnimatedBody::GetNextSensor()
{
    CAST_THIS(neRigidBodyBase, ab);

    return reinterpret_cast<neSensor *>(ab.GetNextSensor());
}

/****************************************************************************
*
*   neAnimatedBody::Active
*
****************************************************************************/ 

void neAnimatedBody::Active(neBool yes, neRigidBody * hint)
{
    CAST_THIS(neRigidBodyBase, ab);

    ab.Active(yes, (neRigidBodyBase *)hint);
}

/****************************************************************************
*
*   neAnimatedBody::Active
*
****************************************************************************/ 

void neAnimatedBody::Active(neBool yes, neAnimatedBody * hint)
{
    CAST_THIS(neRigidBodyBase, ab);

    ab.Active(yes, (neRigidBodyBase *)hint);
}

/****************************************************************************
*
*   neAnimatedBody::IsActive
*
****************************************************************************/ 

neBool neAnimatedBody::Active()
{
    CAST_THIS(neRigidBodyBase, ab);

    return ab.isActive;
}

/****************************************************************************
*
*   neRigidBody::GetMass
*
****************************************************************************/ 

f32 neRigidBody::GetMass()
{
    CAST_THIS(neRigidBody_, rb);

    return rb.mass;
}

/****************************************************************************
*
*   neRigidBody::SetMass
*
****************************************************************************/ 

void neRigidBody::SetMass(f32 mass)
{
    CAST_THIS(neRigidBody_, rb);

    ASSERT(neIsFinite(mass));

    rb.mass = mass;

    rb.oneOnMass = 1.0f / mass;
}

/****************************************************************************
*
*   neRigidBody::SetInertiaTensor
*
****************************************************************************/ 

void neRigidBody::SetInertiaTensor(const neM3 & tensor)
{
    CAST_THIS(neRigidBody_, rb);

    rb.Ibody = tensor;

    rb.IbodyInv.SetInvert(tensor);

    //ASSERT(tensor.Invert(rb.IbodyInv));
}

/****************************************************************************
*
*   neRigidBody::SetInertiaTensor
*
****************************************************************************/ 

void neRigidBody::SetInertiaTensor(const neV3 & tensor)
{
    CAST_THIS(neRigidBody_, rb);

    neM3 i;

    i.SetIdentity();

    i[0][0] = tensor[0];
    i[1][1] = tensor[1];
    i[2][2] = tensor[2];

    rb.Ibody = i;

    rb.IbodyInv.SetInvert(rb.Ibody);
}

/****************************************************************************
*
*   neRigidBody::SetCollisionID
*
****************************************************************************/ 

void neRigidBody::SetCollisionID(s32 cid)
{
    CAST_THIS(neRigidBodyBase, rb);

    rb.cid = cid;
}

/****************************************************************************
*
*   neRigidBody::GetCollisionID
*
****************************************************************************/ 

s32 neRigidBody::GetCollisionID()
{
    CAST_THIS(neRigidBodyBase, rb);

    return rb.cid;
}

/****************************************************************************
*
*   neRigidBody::SetUserData
*
****************************************************************************/ 

void neRigidBody::SetUserData(uintptr_t cookies)
{
    CAST_THIS(neRigidBodyBase, rb);
    
    rb.cookies = cookies;
}

/****************************************************************************
*
*   neRigidBody::GetUserData
*
****************************************************************************/ 

uintptr_t   neRigidBody::GetUserData()
{
    CAST_THIS(neRigidBodyBase, rb);

    return rb.cookies;
}

/****************************************************************************
*
*   neRigidBody::GetGeometryCount
*
****************************************************************************/ 

s32 neRigidBody::GetGeometryCount()
{
    CAST_THIS(neRigidBodyBase, rb);

    return rb.col.convexCount;
}

void neRigidBody::SetLinearDamping(f32 damp)
{
    CAST_THIS(neRigidBody_, rb);

    rb.linearDamp = neAbs(damp);
}

f32 neRigidBody::GetLinearDamping()
{
    CAST_THIS(neRigidBody_, rb);

    return rb.linearDamp;
}

void neRigidBody::SetAngularDamping(f32 damp)
{
    CAST_THIS(neRigidBody_, rb);

    rb.angularDamp = neAbs(damp);
}

f32 neRigidBody::GetAngularDamping()
{
    CAST_THIS(neRigidBody_, rb);

    return rb.angularDamp;
}

void neRigidBody::SetSleepingParameter(f32 sleepingParam)
{
    CAST_THIS(neRigidBody_, rb);

    rb.sleepingParam = sleepingParam;
}

f32 neRigidBody::GetSleepingParameter()
{
    CAST_THIS(neRigidBody_, rb);

    return rb.sleepingParam;
}


/****************************************************************************
*
*   neRigidBody::GetGeometry
*
****************************************************************************/ 
/*
neGeometry * neRigidBody::GetGeometry(s32 index)
{
    CAST_THIS(neRigidBodyBase, rb);

    return reinterpret_cast<neGeometry*>(rb.GetConvex(index));
}
*/
/****************************************************************************
*
*   neRigidBody::SetGeometry
*
****************************************************************************/ 
/*
void neRigidBody::SetGeometry(s32 geometryCount, neGeometry * geometryArray)
{
    //todo
}
*/
/****************************************************************************
*
*   neRigidBody::GetPos
*
****************************************************************************/ 

neV3 neRigidBody::GetPos()
{
    CAST_THIS(neRigidBody_, rb);

    return rb.GetPos();
}

/****************************************************************************
*
*   neRigidBody::SetPos
*
****************************************************************************/ 

void neRigidBody::SetPos(const neV3 & p)
{
    CAST_THIS(neRigidBody_, rb);

    rb.SetPos(p);

    rb.WakeUp();
}

/****************************************************************************
*
*   neRigidBody::GetRotationM3
*
****************************************************************************/ 

neM3 neRigidBody::GetRotationM3()
{
    CAST_THIS(neRigidBody_, rb);

    return rb.State().rot();
}

/****************************************************************************
*
*   neRigidBody::GetRotationQ
*
****************************************************************************/ 

neQ neRigidBody::GetRotationQ()
{
    CAST_THIS(neRigidBody_, rb);

    return rb.State().q;
}

/****************************************************************************
*
*   neRigidBody::SetRotation
*
****************************************************************************/ 

void neRigidBody::SetRotation(const neM3 & m)
{
    ASSERT(m.IsOrthogonalNormal());

    CAST_THIS(neRigidBody_, rb);

    rb.State().rot() = m;

    rb.State().q.SetupFromMatrix3(m);

    rb.WakeUp();
}

/****************************************************************************
*
*   neRigidBody::SetRotation
*
****************************************************************************/ 

void neRigidBody::SetRotation(const neQ & q)
{
    CAST_THIS(neRigidBody_, rb);

    rb.State().q = q;

    rb.State().rot() = q.BuildMatrix3();

    rb.WakeUp();
}

/****************************************************************************
*
*   neRigidBody::GetTransform
*
****************************************************************************/ 

neT3 neRigidBody::GetTransform()
{
    CAST_THIS(neRigidBody_, rb);

    rb.State().b2w.rot[0].v[3] = 0.0f;
    rb.State().b2w.rot[1].v[3] = 0.0f;
    rb.State().b2w.rot[2].v[3] = 0.0f;
    rb.State().b2w.pos.v[3] = 1.0f;
    return rb.State().b2w;
}

/****************************************************************************
*
*   neRigidBody::GetVelocity
*
****************************************************************************/ 

neV3 neRigidBody::GetVelocity()
{
    CAST_THIS(neRigidBody_, rb);

    return rb.Derive().linearVel;
}

/****************************************************************************
*
*   neRigidBody::SetVelocity
*
****************************************************************************/ 

void neRigidBody::SetVelocity(const neV3 & v)
{
    CAST_THIS(neRigidBody_, rb);

    rb.Derive().linearVel = v;

    rb.WakeUpAllJoint();
}

/****************************************************************************
*
*   neRigidBody::GetAngularVelocity
*
****************************************************************************/ 

neV3 neRigidBody::GetAngularVelocity()
{
    CAST_THIS(neRigidBody_, rb);

    return rb.Derive().angularVel;
}

/****************************************************************************
*
*   neRigidBody::GetAngularMomentum
*
****************************************************************************/ 


neV3 neRigidBody::GetAngularMomentum()
{
    CAST_THIS(neRigidBody_, rb);

    return rb.State().angularMom;
}

/****************************************************************************
*
*   neRigidBody::SetAngularMomemtum
*
****************************************************************************/ 

void neRigidBody::SetAngularMomentum(const neV3& am)
{
    CAST_THIS(neRigidBody_, rb);

    rb.SetAngMom(am);

    rb.WakeUpAllJoint();
}

/****************************************************************************
*
*   neRigidBody::GetVelocityAtPoint
*
****************************************************************************/ 

neV3 neRigidBody::GetVelocityAtPoint(const neV3 & pt)
{
    CAST_THIS(neRigidBody_, rb);

    return rb.VelocityAtPoint(pt);
}

/****************************************************************************
*
*   neRigidBody::UpdateBoundingInfo
*
****************************************************************************/ 

void neRigidBody::UpdateBoundingInfo()
{
    CAST_THIS(neRigidBodyBase, rb);

    rb.RecalcBB();
}

/****************************************************************************
*
*   neRigidBody::UpdateInertiaTensor
*
****************************************************************************/ 

void neRigidBody::UpdateInertiaTensor()
{
    CAST_THIS(neRigidBody_, rb);

    rb.RecalcInertiaTensor();
}

/****************************************************************************
*
*   neRigidBody::SetForce
*
****************************************************************************/ 

void neRigidBody::SetForce(const neV3 & force, const neV3 & pos)
{
    CAST_THIS(neRigidBody_, rb);

    if (force.IsConsiderZero())
    {
        rb.force = force;

        rb.torque = ((pos - rb.GetPos()).Cross(force));

        return;
    }

    rb.force = force;

    rb.torque = ((pos - rb.GetPos()).Cross(force));

    rb.WakeUp();
}

/****************************************************************************
*
*   neRigidBody::SetForce
*
****************************************************************************/ 

void neRigidBody::SetTorque(const neV3 & torque)
{
    CAST_THIS(neRigidBody_, rb);

    if (torque.IsConsiderZero())
    {
        rb.torque = torque;

        return;
    }
    rb.torque = torque;

    rb.WakeUp();
}

/****************************************************************************
*
*   neRigidBody::ApplyForceCOG
*
****************************************************************************/ 

void neRigidBody::SetForce(const neV3 & force)
{
    CAST_THIS(neRigidBody_, rb);

    if (force.IsConsiderZero())
    {
        rb.force = force;

        return;
    }
    rb.force = force;

    rb.WakeUp();
}

neV3 neRigidBody::GetForce()
{
    CAST_THIS(neRigidBody_, rb);

    return rb.force;
}

neV3 neRigidBody::GetTorque()
{
    CAST_THIS(neRigidBody_, rb);

    return rb.torque;
}

/****************************************************************************
*
*   neRigidBody::AddImpulse
*
****************************************************************************/ 

void neRigidBody::ApplyImpulse(const neV3 & impulse)
{
    CAST_THIS(neRigidBody_, rb);

    neV3 dv = impulse * rb.oneOnMass;

    rb.Derive().linearVel += dv;

    //rb.WakeUp();
    rb.WakeUpAllJoint();
}

/****************************************************************************
*
*   neRigidBody::AddImpulseWithTwist
*
****************************************************************************/ 

void neRigidBody::ApplyImpulse(const neV3 & impulse, const neV3 & pos)
{
    CAST_THIS(neRigidBody_, rb);

    neV3 dv = impulse * rb.oneOnMass;

    neV3 da = (pos - rb.GetPos()).Cross(impulse);

    rb.Derive().linearVel += dv;

    neV3 newAM = rb.State().angularMom + da;

    rb.SetAngMom(newAM);

    rb.WakeUp();
}

/****************************************************************************
*
*   neRigidBody::ApplyTwist
*
****************************************************************************/ 

void neRigidBody::ApplyTwist(const neV3 & twist)
{
    CAST_THIS(neRigidBody_, rb);
    
    neV3 newAM = twist;

    rb.SetAngMom(newAM);

    rb.WakeUp();
}

/****************************************************************************
*
*   neRigidBody::AddController
*
****************************************************************************/ 

neRigidBodyController * neRigidBody::AddController(neRigidBodyControllerCallback * controller, s32 period)
{
    CAST_THIS(neRigidBody_, rb);

    return (neRigidBodyController *)rb.AddController(controller, period);
}

/****************************************************************************
*
*   neRigidBody::RemoveController
*
****************************************************************************/ 

neBool neRigidBody::RemoveController(neRigidBodyController * rbController)
{
    CAST_THIS(neRigidBody_, rb);

    if (!rb.controllers)
        return false;

    neControllerItem * ci = (neControllerItem *)rb.controllers;

    while (ci)
    {
        neController * con = reinterpret_cast<neController *>(ci);

        ci = ci->next;

        if (con == reinterpret_cast<neController *>(rbController))
        {
            //reinterpret_cast<neControllerItem *>(con)->Remove();

            rb.sim->controllerHeap.Dealloc(con, 1);

            return true;
        }
    }
    return false;
}

/****************************************************************************
*
*   neRigidBody::BeginIterateController
*
****************************************************************************/ 

void neRigidBody::BeginIterateController()
{
    CAST_THIS(neRigidBody_, rb);

    rb.BeginIterateController();
}

/****************************************************************************
*
*   neRigidBody::GetNextController
*
****************************************************************************/ 

neRigidBodyController * neRigidBody::GetNextController()
{
    CAST_THIS(neRigidBody_, rb);

    return (neRigidBodyController *)rb.GetNextController();
}

/****************************************************************************
*
*   neRigidBody::GravityEnable
*
****************************************************************************/ 

void neRigidBody::GravityEnable(neBool yes)
{
    CAST_THIS(neRigidBody_, rb);

    rb.GravityEnable(yes);
}

/****************************************************************************
*
*   neRigidBody::GravityEnable
*
****************************************************************************/ 

neBool neRigidBody::GravityEnable()
{
    CAST_THIS(neRigidBody_, rb);

    return rb.gravityOn;
}

/****************************************************************************
*
*   neRigidBody::CollideConnected
*
****************************************************************************/ 

void neRigidBody::CollideConnected(neBool yes)
{
    CAST_THIS(neRigidBody_, rb);

    rb.CollideConnected(yes);
}

/****************************************************************************
*
*   neRigidBody::CollideConnected
*
****************************************************************************/ 

neBool neRigidBody::CollideConnected()
{
    CAST_THIS(neRigidBody_, rb);

    return rb.CollideConnected();
}

/****************************************************************************
*
*   neRigidBody::CollideDirectlyConnected
*
****************************************************************************/ 

void neRigidBody::CollideDirectlyConnected(neBool yes)
{
    CAST_THIS(neRigidBody_, rb);

    rb.isCollideDirectlyConnected = yes;
}

/****************************************************************************
*
*   neRigidBody::CollideConnected
*
****************************************************************************/ 

neBool neRigidBody::CollideDirectlyConnected()
{
    CAST_THIS(neRigidBody_, rb);

    return rb.isCollideDirectlyConnected;
}

/****************************************************************************
*
*   neRigidBody::AddGeometry
*
****************************************************************************/ 

neGeometry * neRigidBody::AddGeometry()
{
    CAST_THIS(neRigidBody_, rb);

    TConvex * g = rb.AddGeometry();

    return reinterpret_cast<neGeometry *>(g);
}

/****************************************************************************
*
*   neRigidBody::RemoveGeometry
*
****************************************************************************/ 

neBool neRigidBody::RemoveGeometry(neGeometry * g)
{
    CAST_THIS(neRigidBody_, rb);

    if (!rb.col.convex)
        return false;

    TConvexItem * gi = (TConvexItem *)rb.col.convex;

    while (gi)
    {
        TConvex * convex = reinterpret_cast<TConvex *>(gi);

        gi = gi->next;

        if (convex == reinterpret_cast<TConvex *>(g))
        {
            if (rb.col.convex == convex)
            {
                rb.col.convex = (TConvex*)gi;
            }
            rb.sim->geometryHeap.Dealloc(convex, 1);

            rb.col.convexCount--;

            if (rb.col.convexCount == 0)
            {
                rb.col.convex = NULL;

                if (rb.IsInRegion() && !rb.isCustomCD)
                    rb.sim->region.RemoveBody(&rb);
            }
            return true;
        }
    }
    return false;
}

/****************************************************************************
*
*   neRigidBody::BeginIterateGeometry
*
****************************************************************************/ 

void neRigidBody::BeginIterateGeometry()
{
    CAST_THIS(neRigidBody_, rb);

    rb.BeginIterateGeometry();
}

/****************************************************************************
*
*   neRigidBody::GetNextGeometry
*
****************************************************************************/ 

neGeometry * neRigidBody::GetNextGeometry()
{
    CAST_THIS(neRigidBody_, rb);

    return reinterpret_cast<neGeometry *>(rb.GetNextGeometry());
}

/****************************************************************************
*
*   neRigidBody::BreakGeometry
*
****************************************************************************/ 

neRigidBody * neRigidBody::BreakGeometry(neGeometry * g)
{
    CAST_THIS(neRigidBody_, rb);

    neRigidBody_ * newBody = rb.sim->CreateRigidBodyFromConvex((TConvex*)g, &rb);

    return (neRigidBody *)newBody;
}

/****************************************************************************
*
*   neRigidBody::UseCustomCollisionDetection
*
****************************************************************************/ 

void neRigidBody::UseCustomCollisionDetection(neBool yes,  const neT3 * obb, f32 boundingRadius)
{
    CAST_THIS(neRigidBody_, rb);

    if (yes)
    {
        rb.obb = *obb;

        rb.col.boundingRadius = boundingRadius;

        rb.isCustomCD = yes;

        if (rb.isActive && !rb.IsInRegion())
        {
            rb.sim->region.AddBody(&rb, NULL);
        }
    }
    else
    {
        rb.isCustomCD = yes;

        this->UpdateBoundingInfo();

        if (rb.IsInRegion() && GetGeometryCount() == 0)
        {
            rb.sim->region.RemoveBody(&rb);
        }
    }
}

/****************************************************************************
*
*   neRigidBody::UseCustomCollisionDetection
*
****************************************************************************/ 

neBool neRigidBody::UseCustomCollisionDetection()
{
    CAST_THIS(neRigidBody_, rb);

    return rb.isCustomCD;
}

/****************************************************************************
*
*   neRigidBody::AddSensor
*
****************************************************************************/ 

neSensor * neRigidBody::AddSensor()
{
    CAST_THIS(neRigidBody_, rb);

    neSensor_ * s = rb.AddSensor();

    return reinterpret_cast<neSensor *>(s);
}

/****************************************************************************
*
*   neRigidBody::RemoveSensor
*
****************************************************************************/ 

neBool neRigidBody::RemoveSensor(neSensor * s)
{
    CAST_THIS(neRigidBody_, rb);

    if (!rb.sensors)
        return false;

    neSensorItem * si = (neSensorItem *)rb.sensors;

    while (si)
    {
        neSensor_ * sensor = reinterpret_cast<neSensor_ *>(si);

        si = si->next;

        if (sensor == reinterpret_cast<neSensor_ *>(s))
        {
            //reinterpret_cast<neSensorItem *>(s)->Remove();

            rb.sim->sensorHeap.Dealloc(sensor, 1);

            return true;
        }
    }
    return false;
}

/****************************************************************************
*
*   neRigidBody::BeginIterateSensor
*
****************************************************************************/ 

void neRigidBody::BeginIterateSensor()
{
    CAST_THIS(neRigidBody_, rb);

    rb.BeginIterateSensor();
}

/****************************************************************************
*
*   neRigidBody::GetNextSensor
*
****************************************************************************/ 

neSensor * neRigidBody::GetNextSensor()
{
    CAST_THIS(neRigidBody_, rb);

    return reinterpret_cast<neSensor *>(rb.GetNextSensor());
}

/****************************************************************************
*
*   neRigidBody::Active
*
****************************************************************************/ 

void neRigidBody::Active(neBool yes, neRigidBody * hint)
{
    CAST_THIS(neRigidBodyBase, ab);

    ab.Active(yes, (neRigidBodyBase *)hint);
}

/****************************************************************************
*
*   neRigidBody::Active
*
****************************************************************************/ 

void neRigidBody::Active(neBool yes, neAnimatedBody * hint)
{
    CAST_THIS(neRigidBodyBase, ab);

    ab.Active(yes, (neRigidBodyBase *)hint);
}

/****************************************************************************
*
*   neAnimatedBody::IsActive
*
****************************************************************************/ 

neBool neRigidBody::Active()
{
    CAST_THIS(neRigidBodyBase, ab);

    return ab.isActive;
}

neBool neRigidBody::IsIdle()
{
    CAST_THIS(neRigidBody_, rb);

    return (rb.status == neRigidBody_::NE_RBSTATUS_IDLE);
}

/****************************************************************************
*
*   neSimulator::CreateSimulator
*
****************************************************************************/ 

neSimulator * neSimulator::CreateSimulator(const neSimulatorSizeInfo & sizeInfo, neAllocatorAbstract * alloc, const neV3 * grav)
{
    neFixedTimeStepSimulator * s = new neFixedTimeStepSimulator(sizeInfo, alloc, grav);

    return reinterpret_cast<neSimulator*>(s);
}

/****************************************************************************
*
*   neSimulator::DestroySimulator(neSimulator * sim);
*
****************************************************************************/ 

void neSimulator::DestroySimulator(neSimulator * sim)
{
    neFixedTimeStepSimulator * s = reinterpret_cast<neFixedTimeStepSimulator *>(sim);

    delete s;
}

/****************************************************************************
*
*   neSimulator::Gravity
*
****************************************************************************/ 

neV3 neSimulator::Gravity()
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    return sim.gravity;
}

/****************************************************************************
*
*   neSimulator::Gravity
*
****************************************************************************/ 

void neSimulator::Gravity(const neV3 & g)
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    sim.SetGravity(g);
/*
    sim.gravity = g;

    sim.gravityVector = g;

    sim.gravityVector.Normalize();
*/
}

/****************************************************************************
*
*   neSimulator::CreateRigidBody
*
****************************************************************************/ 

neRigidBody * neSimulator::CreateRigidBody()
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    neRigidBody_ * ret = sim.CreateRigidBody();

    return reinterpret_cast<neRigidBody *>(ret);
}

/****************************************************************************
*
*   neSimulator::CreateRigidParticle
*
****************************************************************************/ 

neRigidBody * neSimulator::CreateRigidParticle()
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    neRigidBody_ * ret = sim.CreateRigidBody(true);

    return reinterpret_cast<neRigidBody *>(ret);
}

/****************************************************************************
*
*   neSimulator::CreateCollisionBody()
*
****************************************************************************/ 

neAnimatedBody * neSimulator::CreateAnimatedBody()
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    neCollisionBody_ * ret = sim.CreateCollisionBody();

    return reinterpret_cast<neAnimatedBody *>(ret);
}

/****************************************************************************
*
*   neSimulator::FreeBody
*
****************************************************************************/ 

void neSimulator::FreeRigidBody(neRigidBody * body)
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    sim.Free(reinterpret_cast<neRigidBody_*>(body));
}

/****************************************************************************
*
*   neSimulator::FreeCollisionBody
*
****************************************************************************/ 

void neSimulator::FreeAnimatedBody(neAnimatedBody * body)
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    sim.Free(reinterpret_cast<neRigidBody_*>(body));
}

/****************************************************************************
*
*   neSimulator::GetCollisionTable
*
****************************************************************************/ 

neCollisionTable * neSimulator::GetCollisionTable()
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    return (neCollisionTable *)(&sim.colTable);
}

/****************************************************************************
*
*   neSimulator::GetMaterial
*
****************************************************************************/ 

bool neSimulator::SetMaterial(s32 index, f32 friction, f32 restitution)
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    return sim.SetMaterial(index, friction, restitution, 0.0f);
}

/****************************************************************************
*
*   neSimulator::GetMaterial
*
****************************************************************************/ 

bool neSimulator::GetMaterial(s32 index, f32& friction, f32& restitution)
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    f32 density;
    
    return sim.GetMaterial(index, friction, restitution, density);
}

/****************************************************************************
*
*   neSimulator::Advance
*
****************************************************************************/ 

void neSimulator::Advance(f32 sec, s32 step, nePerformanceReport * perfReport)
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    sim.Advance(sec, step, perfReport);
}

void neSimulator::Advance(f32 sec, f32 minTimeStep, f32 maxTimeStep, nePerformanceReport * perfReport)
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    sim.Advance(sec, minTimeStep, maxTimeStep, perfReport);
}

/****************************************************************************
*
*   neSimulator::SetTerrainMesh
*
****************************************************************************/ 

void neSimulator::SetTerrainMesh(neTriangleMesh * tris)
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    sim.SetTerrainMesh(tris);
}

void neSimulator::FreeTerrainMesh()
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    sim.FreeTerrainMesh();

}

/****************************************************************************
*
*    neSimulator::CreateJoint
*
****************************************************************************/ 

neJoint * neSimulator::CreateJoint(neRigidBody * bodyA)
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    if (!bodyA)
        return NULL;

    _neConstraint * constr = sim.constraintHeap.Alloc(1); // 1 means make it solo

    if (!constr)
    {
        sprintf(sim.logBuffer,  MSG_CONSTRAINT_FULL);

        sim.LogOutput(neSimulator::LOG_OUTPUT_LEVEL_ONE);

        return NULL;
    }

    constr->Reset();

    constr->sim = &sim;

    constr->bodyA = (neRigidBody_*)bodyA;

    neRigidBody_ * ba = (neRigidBody_*)bodyA;
    
    ba->constraintCollection.Add(&constr->bodyAHandle);

    return reinterpret_cast<neJoint*>(constr);
}

/****************************************************************************
*
*    neSimulator::CreateJoint
*
****************************************************************************/ 

neJoint * neSimulator::CreateJoint(neRigidBody * bodyA, neRigidBody * bodyB)
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    if (!bodyA)
        return NULL;

    if (!bodyB)
        return NULL;

    _neConstraint * constr = sim.constraintHeap.Alloc(1); // 1 means make it solo

    if (!constr)
    {
        sprintf(sim.logBuffer,  MSG_CONSTRAINT_FULL);

        sim.LogOutput(neSimulator::LOG_OUTPUT_LEVEL_ONE);

        return NULL;
    }

    constr->Reset();

    constr->sim = &sim;

    constr->bodyA = (neRigidBody_*)bodyA;

    neRigidBody_ * ba = (neRigidBody_*)bodyA;
    
    ba->constraintCollection.Add(&constr->bodyAHandle);

    constr->bodyB = (neRigidBodyBase*)bodyB;

    neRigidBody_ * bb = (neRigidBody_*)bodyB;
    
    bb->constraintCollection.Add(&constr->bodyBHandle);

    return reinterpret_cast<neJoint*>(constr);
}

/****************************************************************************
*
*    neSimulator::CreateJoint
*
****************************************************************************/ 

neJoint * neSimulator::CreateJoint(neRigidBody * bodyA, neAnimatedBody * bodyB)
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    if (!bodyA)
        return NULL;

    if (!bodyB)
        return NULL;

    _neConstraint * constr = sim.constraintHeap.Alloc(1); // 1 means make it solo

    if (!constr)
    {
        sprintf(sim.logBuffer,  MSG_CONSTRAINT_FULL);

        sim.LogOutput(neSimulator::LOG_OUTPUT_LEVEL_ONE);

        return NULL;
    }

    constr->Reset();

    constr->sim = &sim;

    constr->bodyA = (neRigidBody_*)bodyA;

    neRigidBody_ * ba = (neRigidBody_*)bodyA;
    
    ba->constraintCollection.Add(&constr->bodyAHandle);

    constr->bodyB = (neRigidBodyBase*)bodyB;

    neRigidBodyBase * bb = (neRigidBodyBase*)bodyB;
    
    bb->constraintCollection.Add(&constr->bodyBHandle);

    return reinterpret_cast<neJoint*>(constr);
}

/****************************************************************************
*
*   neSimulator::FreeJoint
*
****************************************************************************/ 

void neSimulator::FreeJoint(neJoint * constraint)
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    _neConstraint * c = (_neConstraint *)constraint;

    ASSERT(sim.constraintHeap.CheckBelongAndInUse(c));

    if (c->bodyA)
    {
        c->bodyA->constraintCollection.Remove(&c->bodyAHandle);

        if (c->bodyB)
            c->bodyB->constraintCollection.Remove(&c->bodyBHandle);

        neConstraintHeader * h = c->bodyA->GetConstraintHeader();

        if (h)
        {
            h->Remove(c);

            h->flag = neConstraintHeader::FLAG_NEED_REORG;
        }
        sim.constraintHeap.Dealloc(c, 1);

        if (c->bodyA->constraintCollection.count == 0)
            c->bodyA->RemoveConstraintHeader();

        if (c->bodyB &&
            c->bodyB->constraintCollection.count == 0)
            c->bodyB->RemoveConstraintHeader();
    }
    else
    {
        sim.constraintHeap.Dealloc(c, 1);
    }
}

/****************************************************************************
*
*   neSimulator::SetCollisionCallback
*
****************************************************************************/ 

void neSimulator::SetCollisionCallback(neCollisionCallback * fn)
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    sim.SetCollisionCallback(fn);
}

/****************************************************************************
*
*   neSimulator::GetCollisionCallback
*
****************************************************************************/ 

neCollisionCallback * neSimulator::GetCollisionCallback()
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    return sim.collisionCallback;
}


/****************************************************************************
*
*   neSimulator::SetBreakageCallback
*
****************************************************************************/ 

void neSimulator::SetBreakageCallback(neBreakageCallback * cb)
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    sim.breakageCallback = cb;
}

/****************************************************************************
*
*   neSimulator::GetBreakageCallback
*
****************************************************************************/ 

neBreakageCallback * neSimulator::GetBreakageCallback()
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    return sim.breakageCallback;
}

/****************************************************************************
*
*   neSimulator::SetTerrainTriangleQueryCallback
*
****************************************************************************/ 

void neSimulator::SetTerrainTriangleQueryCallback(neTerrainTriangleQueryCallback * cb)
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    sim.terrainQueryCallback = cb;  
}

/****************************************************************************
*
*   neSimulator::GetTerrainTriangleQueryCallback
*
****************************************************************************/ 

neTerrainTriangleQueryCallback * neSimulator::GetTerrainTriangleQueryCallback()
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    return sim.terrainQueryCallback;
}

/****************************************************************************
*
*   neSimulator::SetCustomCDRB2RBCallback
*
****************************************************************************/ 

void neSimulator::SetCustomCDRB2RBCallback(neCustomCDRB2RBCallback * cb)
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    sim.customCDRB2RBCallback = cb;
}

/****************************************************************************
*
*   neSimulator::GetCustomCDRB2RBCallback
*
****************************************************************************/ 

neCustomCDRB2RBCallback * neSimulator::GetCustomCDRB2RBCallback()
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    return sim.customCDRB2RBCallback;
}

/****************************************************************************
*
*   neSimulator::SetCustomCDRB2ABCallback
*
****************************************************************************/ 

void neSimulator::SetCustomCDRB2ABCallback(neCustomCDRB2ABCallback * cb)
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    sim.customCDRB2ABCallback = cb;
}

/****************************************************************************
*
*   neSimulator::GetCustomCDRB2ABCallback
*
****************************************************************************/ 

neCustomCDRB2ABCallback * neSimulator::GetCustomCDRB2ABCallback()
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    return sim.customCDRB2ABCallback;
}

/****************************************************************************
*
*   neSimulator::SetLogOutputCallback
*
****************************************************************************/ 

void neSimulator::SetLogOutputCallback(neLogOutputCallback * fn)
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    sim.SetLogOutputCallback(fn);
}
/*
f32 neSimulator::GetMagicNumber()
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    return sim.magicNumber;
}
*/
/****************************************************************************
*
*   neSimulator::GetLogOutputCallback
*
****************************************************************************/ 

neLogOutputCallback * neSimulator::GetLogOutputCallback()
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    return sim.logCallback;
}

/****************************************************************************
*
*   neSimulator::SetLogOutputLevel
*
****************************************************************************/ 

void neSimulator::SetLogOutputLevel(LOG_OUTPUT_LEVEL lvl)
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    sim.SetLogOutputLevel(lvl);
}

/****************************************************************************
*
*   neSimulator::GetCurrentSizeInfo
*
****************************************************************************/ 

neSimulatorSizeInfo neSimulator::GetCurrentSizeInfo()
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    neSimulatorSizeInfo ret;

    ret.rigidBodiesCount = sim.rigidBodyHeap.GetUsedCount();
    ret.animatedBodiesCount = sim.collisionBodyHeap.GetUsedCount();
    ret.rigidParticleCount = sim.rigidParticleHeap.GetUsedCount();
    ret.controllersCount = sim.controllerHeap.GetUsedCount();
    ret.overlappedPairsCount = sim.region.overlappedPairs.GetUsedCount();
    ret.geometriesCount = sim.geometryHeap.GetUsedCount();

    ret.constraintsCount = sim.constraintHeap.GetUsedCount();
    ret.constraintSetsCount = sim.constraintHeaders.GetUsedCount();
//  ret.constraintBufferSize = sim.miniConstraintHeap.GetUsedCount();
    ret.sensorsCount = sim.sensorHeap.GetUsedCount();

    ret.terrainNodesStartCount = sim.region.terrainTree.nodes.GetUsedCount();
    ret.terrainNodesGrowByCount = sim.sizeInfo.terrainNodesGrowByCount;

    return ret;
}

/****************************************************************************
*
*   neSimulator::GetStartSizeInfo
*
****************************************************************************/ 

neSimulatorSizeInfo neSimulator::GetStartSizeInfo()
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    return sim.sizeInfo;
}

/****************************************************************************
*
*   neSimulator::GetMemoryUsage
*
****************************************************************************/ 

void neSimulator::GetMemoryAllocated(s32 & memoryAllocated)
{
    CAST_THIS(neFixedTimeStepSimulator, sim);

    sim.GetMemoryAllocated(memoryAllocated);
}

/****************************************************************************
*
*   neJoint::SetType
*
****************************************************************************/ 

void neJoint::SetType(ConstraintType t)
{
    CAST_THIS(_neConstraint, c);

    c.SetType(t);
}

/****************************************************************************
*
*   neJoint::GetType
*
****************************************************************************/ 

neJoint::ConstraintType neJoint::GetType()
{
    CAST_THIS(_neConstraint, c);

    return c.type;
}

/****************************************************************************
*
*   neJoint::GetRigidBodyA
*
****************************************************************************/ 

neRigidBody * neJoint::GetRigidBodyA()
{
    CAST_THIS(_neConstraint, c);

    return reinterpret_cast<neRigidBody *>(c.bodyA);
}

/****************************************************************************
*
*   neJoint::GetRigidBodyB
*
****************************************************************************/ 

neRigidBody * neJoint::GetRigidBodyB()
{
    CAST_THIS(_neConstraint, c);

    if (!c.bodyB)
        return NULL;

    if (c.bodyB->AsCollisionBody())
        return NULL;

    return reinterpret_cast<neRigidBody *>(c.bodyB);
}

/****************************************************************************
*
*   neJoint::GetAnimatedBodyB
*
****************************************************************************/ 

neAnimatedBody * neJoint::GetAnimatedBodyB()
{
    CAST_THIS(_neConstraint, c);

    if (!c.bodyB)
        return NULL;

    if (c.bodyB->AsRigidBody())
        return NULL;

    return reinterpret_cast<neAnimatedBody *>(c.bodyB);
}

/****************************************************************************
*
*   neJoint::SetJointFrameA
*
****************************************************************************/ 

void neJoint::SetJointFrameA(const neT3 & frameA)
{
    CAST_THIS(_neConstraint, c);

    c.frameA = frameA;
}

/****************************************************************************
*
*   neJoint::SetJointFrameB
*
****************************************************************************/ 

void neJoint::SetJointFrameB(const neT3 & frameB)
{
    CAST_THIS(_neConstraint, c);

    c.frameB = frameB;
}

void neJoint::SetJointFrameWorld(const neT3 & frame)
{
    CAST_THIS(_neConstraint, c);

    neT3 w2b;

    w2b = c.bodyA->GetB2W().FastInverse();

    c.frameA = w2b * frame;

    if (!c.bodyB)
    {
        c.frameB = frame;

        return;
    }
    w2b = c.bodyB->GetB2W().FastInverse();

    c.frameB = w2b * frame;
}

/****************************************************************************
*
*   neJoint::GetJointFrameA
*
****************************************************************************/ 

neT3 neJoint::GetJointFrameA()
{
    CAST_THIS(_neConstraint, c);

    if (!c.bodyA)
    {
        return c.frameA;
    }
    neT3 ret;

    ret = c.bodyA->State().b2w * c.frameA;

    return ret;
}

/****************************************************************************
*
*   neJoint::GetJointFrameB
*
****************************************************************************/ 

neT3 neJoint::GetJointFrameB()
{
    CAST_THIS(_neConstraint, c);

    if (!c.bodyB)
    {
        return c.frameB;
    }
    neT3 ret;

    neCollisionBody_ * cb = c.bodyB->AsCollisionBody();

    if (cb)
    {
        ret = cb->b2w * c.frameB;
    }
    else
    {
        neRigidBody_ * rb = c.bodyB->AsRigidBody();

        ret = rb->State().b2w * c.frameB;
    }
    return ret;
}

/****************************************************************************
*
*   neJoint::SetJointLength
*
****************************************************************************/ 

void neJoint::SetJointLength(f32 length)
{
    CAST_THIS(_neConstraint, c);

    c.jointLength = length;
}

/****************************************************************************
*
*   neJoint::GetJointLength
*
****************************************************************************/ 

f32 neJoint::GetJointLength()
{
    CAST_THIS(_neConstraint, c);

    return c.jointLength;
}

/****************************************************************************
*
*   neJoint::Enable
*
****************************************************************************/ 

void neJoint::Enable(neBool yes)
{
    CAST_THIS(_neConstraint, c);

    c.Enable(yes);
}

/****************************************************************************
*
*   neJoint::IsEnable
*
****************************************************************************/ 

neBool neJoint::Enable()
{
    CAST_THIS(_neConstraint, c);

    return c.enable;
}

/****************************************************************************
*
*   neJoint::InfiniteMassB
*
****************************************************************************/ 
/*
void neJoint::InfiniteMassB(neBool yes)
{
    CAST_THIS(_neConstraint, c);

    c.InfiniteMassB(yes);
}
*/
/****************************************************************************
*
*   neJoint::SetDampingFactor
*
****************************************************************************/ 

void neJoint::SetDampingFactor(f32 damp)
{
    CAST_THIS(_neConstraint, c);

    c.jointDampingFactor = damp;
}

/****************************************************************************
*
*   neJoint::GetDampingFactor
*
****************************************************************************/ 

f32 neJoint::GetDampingFactor()
{
    CAST_THIS(_neConstraint, c);

    return c.jointDampingFactor;
}

/****************************************************************************
*
*   neJoint::SetEpsilon
*
****************************************************************************/ 

void neJoint::SetEpsilon(f32 t)
{
    CAST_THIS(_neConstraint, c);

    c.accuracy = t;
}

/****************************************************************************
*
*   neJoint::GetEpsilon
*
****************************************************************************/ 

f32 neJoint::GetEpsilon()
{
    CAST_THIS(_neConstraint, c);

    if (c.accuracy <= 0.0f)
        return DEFAULT_CONSTRAINT_EPSILON;
    
    return c.accuracy;
}

/****************************************************************************
*
*   neJoint::SetIteration2
*
****************************************************************************/ 

void neJoint::SetIteration(s32 i)
{
    CAST_THIS(_neConstraint, c);

    c.iteration = i;
}

/****************************************************************************
*
*   neJoint::GetIteration2
*
****************************************************************************/ 

s32 neJoint::GetIteration()
{
    CAST_THIS(_neConstraint, c);

    return c.iteration;
}

/****************************************************************************
*
*   neJoint::GetUpperLimit
*
****************************************************************************/ 

f32 neJoint::GetUpperLimit()
{
    CAST_THIS(_neConstraint, c);

    return c.limitStates[0].upperLimit;
}

/****************************************************************************
*
*   neJoint::SetUpperLimit
*
****************************************************************************/ 

void neJoint::SetUpperLimit(f32 upperLimit)
{
    CAST_THIS(_neConstraint, c);

    c.limitStates[0].upperLimit = upperLimit;
}

/****************************************************************************
*
*   neJoint::GetLowerLimit
*
****************************************************************************/ 

f32 neJoint::GetLowerLimit()
{
    CAST_THIS(_neConstraint, c);

    return c.limitStates[0].lowerLimit;
}

/****************************************************************************
*
*   neJoint::SetLowerLimit
*
****************************************************************************/ 

void neJoint::SetLowerLimit(f32 lowerLimit)
{
    CAST_THIS(_neConstraint, c);

    c.limitStates[0].lowerLimit = lowerLimit;
}

/****************************************************************************
*
*   neJoint::IsEnableLimit
*
****************************************************************************/ 

neBool neJoint::EnableLimit()
{
    CAST_THIS(_neConstraint, c);

    return c.limitStates[0].enableLimit;
}

/****************************************************************************
*
*   neJoint::EnableLimite
*
****************************************************************************/ 

void neJoint::EnableLimit(neBool yes)
{
    CAST_THIS(_neConstraint, c);

    c.limitStates[0].enableLimit = yes;
}

/****************************************************************************
*
*   neJoint::GetUpperLimit2
*
****************************************************************************/ 

f32 neJoint::GetUpperLimit2()
{
    CAST_THIS(_neConstraint, c);

    return c.limitStates[1].upperLimit;
}

/****************************************************************************
*
*   neJoint::SetUpperLimit2
*
****************************************************************************/ 

void neJoint::SetUpperLimit2(f32 upperLimit)
{
    CAST_THIS(_neConstraint, c);

    c.limitStates[1].upperLimit = upperLimit;
}

/****************************************************************************
*
*   neJoint::GetLowerLimit2
*
****************************************************************************/ 

f32 neJoint::GetLowerLimit2()
{
    CAST_THIS(_neConstraint, c);

    return c.limitStates[1].lowerLimit;
}

/****************************************************************************
*
*   neJoint::SetLowerLimit2
*
****************************************************************************/ 

void neJoint::SetLowerLimit2(f32 lowerLimit)
{
    CAST_THIS(_neConstraint, c);

    c.limitStates[1].lowerLimit = lowerLimit;
}

/****************************************************************************
*
*   neJoint::IsEnableLimit2
*
****************************************************************************/ 

neBool neJoint::EnableLimit2()
{
    CAST_THIS(_neConstraint, c);

    return c.limitStates[1].enableLimit;
}

/****************************************************************************
*
*   neJoint::EnableMotor
*
****************************************************************************/ 

neBool neJoint::EnableMotor()
{
    CAST_THIS(_neConstraint, c);

    return c.motors[0].enable;
}

/****************************************************************************
*
*   neJoint::EnableMotor
*
****************************************************************************/ 

void neJoint::EnableMotor(neBool yes)
{
    CAST_THIS(_neConstraint, c);

    c.motors[0].enable = yes;
}

/****************************************************************************
*
*   neJoint::SetMotor
*
****************************************************************************/ 

void neJoint::SetMotor(MotorType motorType, f32 desireValue, f32 maxForce)
{
    CAST_THIS(_neConstraint, c);

    c.motors[0].motorType = motorType;

    c.motors[0].desireVelocity = desireValue;

    c.motors[0].maxForce = neAbs(maxForce);
}

/****************************************************************************
*
*   neJoint::GetMotor
*
****************************************************************************/ 

void neJoint::GetMotor(MotorType & motorType, f32 & desireValue, f32 & maxForce)
{
    CAST_THIS(_neConstraint, c);

    motorType = c.motors[0].motorType;

    desireValue = c.motors[0].desireVelocity;

    maxForce = c.motors[0].maxForce;
}

/****************************************************************************
*
*   neJoint::EnableMotor2
*
****************************************************************************/ 

neBool neJoint::EnableMotor2()
{
    CAST_THIS(_neConstraint, c);

    return c.motors[1].enable;
}

/****************************************************************************
*
*   neJoint::EnableMotor2
*
****************************************************************************/ 

void neJoint::EnableMotor2(neBool yes)
{
    CAST_THIS(_neConstraint, c);

    c.motors[1].enable = yes;
}

/****************************************************************************
*
*   neJoint::SetMotor2
*
****************************************************************************/ 

void neJoint::SetMotor2(MotorType motorType, f32 desireValue, f32 maxForce)
{
    CAST_THIS(_neConstraint, c);

    c.motors[1].motorType = motorType;

    c.motors[1].desireVelocity = desireValue;

    c.motors[1].maxForce = neAbs(maxForce);
}

/****************************************************************************
*
*   neJoint::GetMotor2
*
****************************************************************************/ 

void neJoint::GetMotor2(MotorType & motorType, f32 & desireValue, f32 & maxForce)
{
    CAST_THIS(_neConstraint, c);

    motorType = c.motors[1].motorType;

    desireValue = c.motors[1].desireVelocity;

    maxForce = c.motors[1].maxForce;
}

/****************************************************************************
*
*   neJoint::EnableLimite
*
****************************************************************************/ 

void neJoint::EnableLimit2(neBool yes)
{
    CAST_THIS(_neConstraint, c);

    c.limitStates[1].enableLimit = yes;
}


/****************************************************************************
*
*   neJoint::AddController
*
****************************************************************************/ 

neJointController * neJoint::AddController(neJointControllerCallback * controller, s32 period)
{
    CAST_THIS(_neConstraint, c);

    return (neJointController *)c.AddController(controller, period);
}

/****************************************************************************
*
*   neJoint::RemoveController
*
****************************************************************************/ 

neBool neJoint::RemoveController(neJointController * jController)
{
    CAST_THIS(_neConstraint, c);

    if (!c.controllers)
        return false;

    neControllerItem * ci = (neControllerItem *)c.controllers;

    while (ci)
    {
        neController * con = reinterpret_cast<neController *>(ci);

        ci = ci->next;

        if (con == reinterpret_cast<neController *>(jController))
        {
            //reinterpret_cast<neControllerItem *>(con)->Remove();

            c.sim->controllerHeap.Dealloc(con, 1);

            return true;
        }
    }
    return false;
}

/****************************************************************************
*
*   neJoint::BeginIterateController
*
****************************************************************************/ 

void neJoint::BeginIterateController()
{
    CAST_THIS(_neConstraint, c);

    c.BeginIterateController();
}

/****************************************************************************
*
*   neJoint::GetNextController
*
****************************************************************************/ 

neJointController * neJoint::GetNextController()
{
    CAST_THIS(_neConstraint, c);

    return (neJointController *)c.GetNextController();
}

/****************************************************************************
*
*   neJoint::SetBSPoints
*
****************************************************************************/ 
/*
neBool neJoint::SetBSPoints(const neV3 & pointA, const neV3 & pointB)
{
    CAST_THIS(_neConstraint, c);

    if (c.type != NE_Constraint_BALLSOCKET)
        return false;

    c.cpointsA[0].PtBody() = pointA;

    c.cpointsB[0].PtBody() = pointB;

    return true;
}
*/
/****************************************************************************
*
*   neJoint::SetHingePoints
*
****************************************************************************/ 
/*
neBool neJoint::SetHingePoints(const neV3 & pointA1, const neV3 & pointA2,
                        const neV3 & pointB1, const neV3 & pointB2)
{
    CAST_THIS(_neConstraint, c);

    if (c.type != NE_Constraint_HINGE)
        return false;

    c.cpointsA[0].PtBody() = pointA1;

    c.cpointsA[1].PtBody() = pointA2;

    c.cpointsB[0].PtBody() = pointB1;

    c.cpointsB[1].PtBody() = pointB2;

    return true;
}
*/
/****************************************************************************
*
*   neSensor::SetLineSensor
*
****************************************************************************/ 

void neSensor::SetLineSensor(const neV3 & pos, const neV3 & lineVector)
{
    CAST_THIS(neSensor_, sensor);

    sensor.pos = pos;

    sensor.dir = lineVector;

    sensor.dirNormal = lineVector;

    sensor.dirNormal.Normalize();

    sensor.length = lineVector.Length();
}

/****************************************************************************
*
*   neSensor::SetUserData
*
****************************************************************************/ 

void neSensor::SetUserData(uintptr_t cookies)
{
    CAST_THIS(neSensor_, sensor);

    sensor.cookies = cookies;
}

/****************************************************************************
*
*   neSensor::GetUserData
*
****************************************************************************/ 

uintptr_t neSensor::GetUserData()
{
    CAST_THIS(neSensor_, sensor);

    return sensor.cookies;
}

/****************************************************************************
*
*   neSensor::GetLineNormal
*
****************************************************************************/ 

neV3 neSensor::GetLineVector()
{
    CAST_THIS(neSensor_, sensor);

    return sensor.dir;
}

/****************************************************************************
*
*   neSensor::GetLineNormal
*
****************************************************************************/ 

neV3 neSensor::GetLineUnitVector()
{
    CAST_THIS(neSensor_, sensor);

    return sensor.dirNormal ;
}

/****************************************************************************
*
*   neSensor::GetLinePos
*
****************************************************************************/ 

neV3 neSensor::GetLinePos()
{
    CAST_THIS(neSensor_, sensor);

    return sensor.pos;
}

/****************************************************************************
*
*   neSensor::GetDetectDepth
*
****************************************************************************/ 

f32 neSensor::GetDetectDepth()
{
    CAST_THIS(neSensor_, sensor);

    return sensor.depth;
}

/****************************************************************************
*
*   neSensor::GetDetectNormal
*
****************************************************************************/ 

neV3 neSensor::GetDetectNormal()
{
    CAST_THIS(neSensor_, sensor);

    return sensor.normal;
}

/****************************************************************************
*
*   neSensor::GetDetectContactPoint
*
****************************************************************************/ 

neV3 neSensor::GetDetectContactPoint()
{
    CAST_THIS(neSensor_, sensor);

    return sensor.contactPoint;
}

/****************************************************************************
*
*   neSensor::GetDetectRigidBody
*
****************************************************************************/ 

neRigidBody * neSensor::GetDetectRigidBody()
{
    CAST_THIS(neSensor_, sensor);

    if (!sensor.body)
        return NULL;

    if (sensor.body->AsCollisionBody())
        return NULL;

    return (neRigidBody *)sensor.body;
}

/****************************************************************************
*
*   neSensor::GetDetectAnimatedBody
*
****************************************************************************/ 

neAnimatedBody * neSensor::GetDetectAnimatedBody()
{
    CAST_THIS(neSensor_, sensor);

    if (!sensor.body)
        return NULL;

    if (sensor.body->AsRigidBody())
        return NULL;

    return (neAnimatedBody *)sensor.body;
}

/****************************************************************************
*
*   neSensor::
*
****************************************************************************/ 

s32 neSensor::GetDetectMaterial()
{
    CAST_THIS(neSensor_, sensor);

    return sensor.materialID;
}

/****************************************************************************
*
*   neRigidBodyController::
*
****************************************************************************/ 

neRigidBody * neRigidBodyController::GetRigidBody()
{
    CAST_THIS(neController, c);

    return (neRigidBody *)c.rb;
}

/****************************************************************************
*
*   neRigidBodyController::
*
****************************************************************************/ 

neV3 neRigidBodyController::GetControllerForce()
{
    CAST_THIS(neController, c);

    return c.forceA;
}

/****************************************************************************
*
*   neRigidBodyController::
*
****************************************************************************/ 

neV3 neRigidBodyController::GetControllerTorque()
{
    CAST_THIS(neController, c);

    return c.torqueA;
}
    
/****************************************************************************
*
*   neRigidBodyController::
*
****************************************************************************/ 

void neRigidBodyController::SetControllerForce(const neV3 & force)
{
    CAST_THIS(neController, c);

    c.forceA = force;
}

/****************************************************************************
*
*   neRigidBodyController::
*
****************************************************************************/ 

void neRigidBodyController::SetControllerForceWithTorque(const neV3 & force, const neV3 & pos)
{
    CAST_THIS(neController, c);

    c.forceA = force;

    c.torqueA = ((pos - c.rb->GetPos()).Cross(force));
}

/****************************************************************************
*
*   neRigidBodyController::
*
****************************************************************************/ 

void neRigidBodyController::SetControllerTorque(const neV3 & torque)
{
    CAST_THIS(neController, c);

    c.torqueA = torque;
}

/****************************************************************************
*
*   neJointController::
*
****************************************************************************/ 

neJoint * neJointController::GetJoint()
{
    CAST_THIS(neController, c);

    return (neJoint *)c.constraint;
}

/****************************************************************************
*
*   neJointController::
*
****************************************************************************/ 

neV3 neJointController::GetControllerForceBodyA()
{
    CAST_THIS(neController, c);

    return c.forceA;
}


/****************************************************************************
*
*   neJointController::
*
****************************************************************************/ 

neV3 neJointController::GetControllerForceBodyB()
{
    CAST_THIS(neController, c);

    return c.forceB;
}

/****************************************************************************
*
*   neJointController::
*
****************************************************************************/ 

neV3 neJointController::GetControllerTorqueBodyA()
{
    CAST_THIS(neController, c);

    return c.torqueA;
}

/****************************************************************************
*
*   neJointController::
*
****************************************************************************/ 

neV3 neJointController::GetControllerTorqueBodyB()
{
    CAST_THIS(neController, c);

    return c.torqueB;
}

/****************************************************************************
*
*   neJointController::
*
****************************************************************************/ 

void neJointController::SetControllerForceBodyA(const neV3 & force)
{
    CAST_THIS(neController, c);

    c.forceA = force;
}

/****************************************************************************
*
*   neJointController::
*
****************************************************************************/ 

void neJointController::SetControllerForceWithTorqueBodyA(const neV3 & force, const neV3 & pos)
{
    CAST_THIS(neController, c);

    c.forceA = force;

    c.torqueA = ((pos - c.constraint->bodyA->GetPos()).Cross(force));
}

/****************************************************************************
*
*   neJointController::
*
****************************************************************************/ 

void neJointController::SetControllerForceWithTorqueBodyB(const neV3 & force, const neV3 & pos)
{
    CAST_THIS(neController, c);

    c.forceB = force;

    if (c.constraint->bodyB &&
        !c.constraint->bodyB->AsCollisionBody())
    {
        neRigidBody_ * rb = (neRigidBody_*)c.constraint->bodyB; 

        c.torqueB = ((pos - rb->GetPos()).Cross(force));
    }
}

/****************************************************************************
*
*   neJointController::
*
****************************************************************************/ 

void neJointController::SetControllerForceBodyB(const neV3 & force)
{
    CAST_THIS(neController, c);

    c.forceB = force;
}

/****************************************************************************
*
*   neJointController::
*
****************************************************************************/ 

void neJointController::SetControllerTorqueBodyA(const neV3 & torque)
{
    CAST_THIS(neController, c);

    c.torqueA = torque;
}

/****************************************************************************
*
*   neJointController::
*
****************************************************************************/ 

void neJointController::SetControllerTorqueBodyB(const neV3 & torque)
{
    CAST_THIS(neController, c);

    c.torqueB = torque;
}

/****************************************************************************
*
*   neCollisionTable::Set
*
****************************************************************************/ 

void neCollisionTable::Set(s32 collisionID1, s32 collisionID2, neReponseBitFlag response)
{
    CAST_THIS(neCollisionTable_, ct);

    ct.Set(collisionID1, collisionID2, response);
}

/****************************************************************************
*
*   neCollisionTable::Get
*
****************************************************************************/ 

neCollisionTable::neReponseBitFlag neCollisionTable::Get(s32 collisionID1, s32 collisionID2)
{
    CAST_THIS(neCollisionTable_, ct);

    ASSERT(collisionID1 < NE_COLLISION_TABLE_MAX);

    ASSERT(collisionID2 < NE_COLLISION_TABLE_MAX);

    if (collisionID1 < NE_COLLISION_TABLE_MAX && collisionID2 < NE_COLLISION_TABLE_MAX)
    {
        return ct.table[collisionID1][collisionID2];
    }
    else
    {
        return RESPONSE_IGNORE;
    }
    
}

/****************************************************************************
*
*   neCollisionTable::GetMaxCollisionID
*
****************************************************************************/ 

s32 neCollisionTable::GetMaxCollisionID()
{
    return NE_COLLISION_TABLE_MAX;
}

/****************************************************************************
*
*   Helper functions
*
****************************************************************************/ 

/****************************************************************************
*
*   BoxInertiaTensor
*
****************************************************************************/ 

neV3 neBoxInertiaTensor(const neV3 & boxSize, f32 mass)
{
    return neBoxInertiaTensor(boxSize[0], boxSize[1], boxSize[2], mass);
}

neV3 neBoxInertiaTensor(f32 width, f32 height, f32 depth, f32 mass)
{
    neV3 ret;

    f32 maxdim = width;

    if (height > maxdim)
        maxdim = height;

    if (depth > maxdim)
        maxdim = depth;
#if 0
    f32 xsq = width;
    f32 ysq = height;
    f32 zsq = depth;
#else
    f32 xsq = maxdim;
    f32 ysq = maxdim;
    f32 zsq = maxdim;
#endif

    xsq *= xsq;
    ysq *= ysq;
    zsq *= zsq;

    ret[0] = (ysq + zsq) * mass / 3.0f;
    ret[1] = (xsq + zsq) * mass / 3.0f;
    ret[2] = (xsq + ysq) * mass / 3.0f;

    return ret;
}

neV3 neSphereInertiaTensor(f32 diameter, f32 mass)
{
    f32 radius = diameter * 0.5f;

    f32 value = 2.0f / 5.0f * mass * radius * radius;

    neV3 ret;

    ret.Set(value);

    return ret;
}

neV3 neCylinderInertiaTensor(f32 diameter, f32 height, f32 mass)
{
//  if (height > diameter)
//  {
//      diameter = height;
//  }
    f32 radius = diameter * 0.5f;

    f32 radiusSq = radius * radius;

    f32 Ixz = 1.0f / 12.0f * mass * height * height + 0.25f * mass * radiusSq;

    f32 Iyy = 0.5f * mass * radiusSq;

    neV3 ret;
    
    ret.Set(Ixz, Iyy, Ixz);

    return ret;
}

/*
    neBool IsEnableLimit();

    void EnableLimit(neBool yes);

    f32 GetUpperLimit();

    void SetUpperLimit(f32 upperLimit);

    f32 GetLowerLimit();

    void SetLowerLimit(f32 lowerLimit);
*/

#ifdef _WIN32

/**perfwin32.cpp**/

//#include "math/ne_type.h"
//#include "math/ne_debug.h"
//#include "tokamak.h"
//#include "containers.h"
//#include "scenery.h"
//#include "collision.h"
//#include "constraint.h"
//#include "rigidbody.h"
//#include "scenery.h"

//#ifdef _WIN32
//#include <windows.h>
//#endif

//#include "stack.h"
//#include "simulator.h"
//#include "perfwin32.h"
/*
typedef unsigned __int64 u64;
typedef __int64 s64;
u64 tickPerSec;
u64 m_ticks_at_start;

float divide64(u64 a, u64 b)
{
    return 0;//float( double(s64(a)) / double(s64(b)) );
}

u64 getTickCounter()
{
    u64 ticks;
    // note: using cpuid as a serializing makes timings more accurate, 
    // at the expense of more overhead. (1.5% without versus 5% with cpuid)
    __asm {
        push ebx
        //cpuid 
        pop ebx
        rdtsc
        mov dword ptr[ticks  ], eax
        mov dword ptr[ticks+4], edx
    }

    return ticks;
}

u64 getTicksPerSecond()
{
    static u64 freq = 0;
    if(freq==0)
    {
        u64 ticks;
        u64 qticks;
        u64 ticks2;
        u64 qticks2;
        double minFactor = 1e6f;

        // Iterate several times
        // We take the minimum value beacuse Sleep() sleeps for at least the specified time
        for (int iter = 0; iter <10; iter++)
        {
            ticks = getTickCounter();
            QueryPerformanceCounter( (LARGE_INTEGER*) &qticks);

            ///
            /// Sleep for a little while
            ///
            volatile x=1;
            for (int j=0; j< 5000; j++)
            {
                x += x*x;
            }


            ticks2 = getTickCounter();
            QueryPerformanceCounter( (LARGE_INTEGER*) &qticks2);

            // We assume that this is fixed & regular 
            QueryPerformanceFrequency( (LARGE_INTEGER*) &freq);

            // Work our calibration factor
            u64 diff = ticks2 - ticks;
            u64 qdiff = qticks2 - qticks;

            double factor = double(diff)/ double(qdiff);
            
            // Is this smaller?
            if (factor < minFactor)
            {
                minFactor = factor;
            }
        }
        freq = u64(minFactor * freq);
    }
    return freq;
}
*/
nePerformanceData * nePerformanceData::Create()
{
    return new nePerformanceData;
}

LARGE_INTEGER perfFreq;

LARGE_INTEGER counter;

/****************************************************************************
*
*   nePerformanceData::Start
*
****************************************************************************/ 

void DunselFunction() { return; }

void nePerformanceData::Init()
{
   Reset();

//   tickPerSec = getTicksPerSecond();

//   return;
   
   void (*pFunc)() = DunselFunction;

   // Assume the worst
   if ( QueryPerformanceFrequency(&perfFreq) )
      {
      // We can use hires timer, determine overhead

      overheadTicks = 200;
      for ( int i=0; i < 20; i++ )
         {
         LARGE_INTEGER b,e;
         int Ticks;
         QueryPerformanceCounter(&b);
         (*pFunc)();
         QueryPerformanceCounter(&e);
         Ticks = e.LowPart - b.LowPart;
         if ( Ticks >= 0 && Ticks < overheadTicks )
            overheadTicks = Ticks;
         }
      // See if Freq fits in 32 bits; if not lose some precision
      perfFreqAdjust = 0;

      int High32 = perfFreq.HighPart;
      
      while ( High32 )
         {
         High32 >>= 1;
         perfFreqAdjust++;
         }
      }
   
   //QueryPerformanceCounter(&counter);
}

void nePerformanceData::Start()
{
    Reset();
    
    QueryPerformanceCounter(&counter);
    //m_ticks_at_start = getTickCounter();
}

f32 nePerformanceData::GetCount()
{
    //u64 ticks_now = getTickCounter();

    //u64 m_ticks_total = ticks_now - m_ticks_at_start;

    //return divide64(m_ticks_total, tickPerSec);

    LARGE_INTEGER tStart, tStop;
    LARGE_INTEGER Freq = perfFreq;
    int Oht = overheadTicks;
    int ReduceMag = 0;
/*
    SetThreadPriority(GetCurrentThread(), 
     THREAD_PRIORITY_TIME_CRITICAL);

    QueryPerformanceCounter(&tStart);
    (*funcp)();   //call the actual function being timed
*/  
    tStart = counter;
    
    QueryPerformanceCounter(&tStop);

    counter = tStop;
//  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
    // Results are 64 bits but we only do 32
    unsigned int High32 = tStop.HighPart - counter.HighPart;
    while ( High32 )
    {
        High32 >>= 1;
        ReduceMag++;
    }
    if ( perfFreqAdjust || ReduceMag )
    {
        if ( perfFreqAdjust > ReduceMag )
            ReduceMag = perfFreqAdjust;

         tStart.QuadPart = Int64ShrlMod32(tStart.QuadPart, ReduceMag);
         tStop.QuadPart = Int64ShrlMod32(tStop.QuadPart, ReduceMag);
         Freq.QuadPart = Int64ShrlMod32(Freq.QuadPart, ReduceMag);
         Oht >>= ReduceMag;
    }
    double time;
    
    // Reduced numbers to 32 bits, now can do the math
    if ( Freq.LowPart == 0 )
        time = 0.0;
    else
        time = ((double)(tStop.LowPart - tStart.LowPart - Oht))/Freq.LowPart;

    return (f32)time;
}

void nePerformanceData::UpdateDynamic()
{
    dynamic += GetCount();
}
void nePerformanceData::UpdatePosition()
{
    position += GetCount();
}
void nePerformanceData::UpdateConstrain1()
{
    constrain_1 += GetCount();
}
void nePerformanceData::UpdateConstrain2()
{
    constrain_2 += GetCount();
}
void nePerformanceData::UpdateCD()
{
    cd += GetCount();
}
void nePerformanceData::UpdateCDCulling()
{
    cdCulling += GetCount();
}
void nePerformanceData::UpdateTerrain()
{
    terrain += GetCount();
}
void nePerformanceData::UpdateControllerCallback()
{
    controllerCallback += GetCount();
}
void nePerformanceData::UpdateTerrainCulling()
{
    terrainCulling += GetCount();
}

#else

/**perflinux.cpp**/

//#include "math/ne_type.h"
//#include "math/ne_debug.h"
//#include "tokamak.h"
//#include "containers.h"
//#include "scenery.h"
//#include "collision.h"
//#include "constraint.h"
//#include "rigidbody.h"
//#include "scenery.h"

//#include "stack.h"
//#include "simulator.h"
//#include "perflinux.h"

#include <sys/time.h>

nePerformanceData * nePerformanceData::Create()
{
    return new nePerformanceData;
}

s64 perfFreq;

timeval counter;

/****************************************************************************
*
*   nePerformanceData::Start
*
****************************************************************************/

void DunselFunction() { return; }

void nePerformanceData::Init()
{
    Reset();

    void (*pFunc)() = DunselFunction;

    gettimeofday(&counter, NULL);
}

void nePerformanceData::Start()
{
    Reset();

    gettimeofday(&counter, NULL);
}

f32 nePerformanceData::GetCount()
{
    timeval tStart, tStop;
    f32 start, end;

    tStart = counter;

    gettimeofday(&tStop, NULL);

    start = (tStart.tv_sec * 1000000.0) + tStart.tv_usec;
    end = (tStop.tv_sec * 1000000.0) + tStop.tv_usec;

    return (end - start) * 0.000001;
}

void nePerformanceData::UpdateDynamic()
{
    dynamic += GetCount();
}
void nePerformanceData::UpdatePosition()
{
    position += GetCount();
}
void nePerformanceData::UpdateConstrain1()
{
    constrain_1 += GetCount();
}
void nePerformanceData::UpdateConstrain2()
{
    constrain_2 += GetCount();
}
void nePerformanceData::UpdateCD()
{
    cd += GetCount();
}
void nePerformanceData::UpdateCDCulling()
{
    cdCulling += GetCount();
}
void nePerformanceData::UpdateTerrain()
{
    terrain += GetCount();
}
void nePerformanceData::UpdateControllerCallback()
{
    controllerCallback += GetCount();
}
void nePerformanceData::UpdateTerrainCulling()
{
    terrainCulling += GetCount();
}

#endif


/**region.cpp**/

//#include "math/ne_type.h"
//#include "math/ne_debug.h"
//#include "tokamak.h"
//#include "containers.h"
//#include "scenery.h"
//#include "collision.h"
//#include "constraint.h"
//#include "rigidbody.h"
//#include "scenery.h"
/*
#ifdef _WIN32
#include <windows.h>
#endif
*/
//#include "stack.h"
//#include "simulator.h"

#include <stdio.h>
#include <assert.h>

/****************************************************************************
*
*   neRegion::~neRegion
*
****************************************************************************/ 

neRegion::~neRegion()
{
}

/****************************************************************************
*
*   neRegion::Initialise
*
****************************************************************************/ 

void neRegion::Initialise(neFixedTimeStepSimulator * s, neByte sortD)
{
    sim = s;

    maxRigidBodies = s->maxRigidBodies;

    maxAnimBodies = s->maxAnimBodies;

    totalBodies = maxRigidBodies + maxAnimBodies;

    maxParticle = s->maxParticles;

    b2b.Reserve(totalBodies * (totalBodies - 1) / 2, sim->allocator);

    b2p.Reserve(totalBodies * maxParticle, sim->allocator);

    newBodies.Reserve(totalBodies + maxParticle, sim->allocator);

    bodies.Reserve(totalBodies + maxParticle, sim->allocator);

    overlappedPairs.Reserve(sim->sizeInfo.overlappedPairsCount, sim->allocator);

    sortDimension = sortD;

    for (s32 i = 0; i < 3; i++)
    {
        if (sortD & (1 << i ) )
        {
            bool b = coordLists[i].Reserve((totalBodies + maxParticle) * 2, sim->allocator);
            
            coordLists[i].dim = i;

            coordLists[i].dimPower2 = 1 << i;

            coordLists[i].region = this;
            
            ASSERT(b);
        }
    }

//  needRebuild = true;

    terrainTree.sim = sim;
    
#ifdef _DEBUG_REGION
    debugOn = false;
#endif
}

/****************************************************************************
*
*   neRegion::AddBody
*
****************************************************************************/ 

bool neRegion::AddBody(neRigidBodyBase * bb, neRigidBodyBase * hint)
{
    if (bb->IsInRegion())
        return true;
    
    neAddBodyInfo * bi = newBodies.Alloc();
    
    bi->body = bb;

    bi->hint = hint;

    neRigidBodyBase ** bp = bodies.Alloc();
        
    *bp = bi->body;

    bi->body->regionHandle = (neFreeListItem<neRigidBodyBase *>*)bp;

    bb->pendingAddToRegion = 1;

    return true;
}

/****************************************************************************
*
*   neRegion::RemoveBody
*
****************************************************************************/ 

void neRegion::RemoveBody(neRigidBodyBase * bb)
{
    if (!bb->IsInRegion())
        return;

    if (bb->pendingAddToRegion == 1)
    {
        bb->pendingAddToRegion = 2; // to signify adding to region is aborted
    }

    for (s32 i = 0; i < 3; i++)
    {
        if (sortDimension & (1 << i) )
        {
            if (bb->maxCoord[i])
            {
                coordLists[i].coordList.Dealloc(bb->maxCoord[i]);

                bb->maxCoord[i] = NULL;
            }
            if (bb->minCoord[i])
            {
                coordLists[i].coordList.Dealloc(bb->minCoord[i]);

                bb->minCoord[i] = NULL;
            }
        }
    }
    if (bb->regionHandle)
        bodies.Dealloc((neRigidBodyBase**)bb->regionHandle);

    bb->regionHandle = NULL;
    
    neFreeListItem<neOverlappedPair> * oitem = (neFreeListItem<neOverlappedPair> *)(*overlappedPairs.BeginUsed());

    while (oitem)
    {
        neOverlappedPair * op = (neOverlappedPair *) oitem;

        oitem = oitem->next;

        if (op->bodyA == bb || op->bodyB == bb)
        {
            overlappedPairs.Dealloc(op);
        }
    }
}

/****************************************************************************
*
*   neRegion::UpdateOverlapPairs
*
****************************************************************************/ 

void neRegion::Rebuild()
{
    // sort coordinate list
    for (s32 i = 0; i < 3; i++)
    {
        if (sortDimension & (1 << i) )
        {
            coordLists[i].Sort(true);
        }
    }

    overlappedPairs.Clear();
    
    neRigidBody_ * rb = sim->activeRB.GetHead();

    while (rb)
    {
        neRigidBody_ * rbNext = sim->activeRB.GetNext(rb);

        neRigidBody_ * rbNext_ = rbNext;

        while (rbNext)
        {
            ResetOverlapStatus(rb, rbNext);

            rbNext = sim->activeRB.GetNext(rbNext);
        }
        rb = rbNext_;
    }

    rb = sim->activeRB.GetHead();

    while (rb)
    {
        neCollisionBody_ * cb = sim->activeCB.GetHead();

        while (cb)
        {
            ResetOverlapStatus(rb, cb);

            cb = sim->activeCB.GetNext(cb);
        }
        rb = sim->activeRB.GetNext(rb);
    }
}

/****************************************************************************
*
*   neRegion::Update
*
****************************************************************************/ 

void neRegion::Update()
{
    for (s32 i = 0; i < 3; i++)
    {
        if (sortDimension & (1 << i) )
        {
            coordLists[i].Sort(false);
        }
    }

    for (s32 k = 0; k < newBodies.GetUsedCount(); k++)
    {
        neAddBodyInfo * bi = &newBodies[k];

        if (bi->body->pendingAddToRegion == 2 || !bi->body->IsValid())
        {
            continue;
        }

        ASSERT(bi->body->pendingAddToRegion == 1);

        bi->body->pendingAddToRegion = 0;

        InsertCoordList(bi->body, bi->hint);

        neDLinkList<neRigidBodyBase *>::iterator iter;

        for (iter = bodies.BeginUsed(); iter.Valid(); iter++)
        {
            neRigidBodyBase * b = *(*iter);

            if (b->minCoord[0] == NULL && b->minCoord[1] == NULL && b->minCoord[2] == NULL)
                continue;

            if (b == bi->body)
                continue;
            
            neRigidBody_ * b1 = bi->body->AsRigidBody();

            neRigidBody_ * b2 = b->AsRigidBody();

            if ((b1 && b2))
            {
                if (! (b1->IsParticle() && b2->IsParticle()) )
                    ResetOverlapStatus(bi->body, b);
            }
            else
            {
                ResetOverlapStatus(bi->body, b);
            }
        }
/*
        neRigidBodyBase ** bp = bodies.Alloc();
        
        *bp = bi->body;

        bi->body->regionHandle = (neFreeListItem<neRigidBodyBase *>*)bp;
*/  }

    newBodies.Clear();

#ifdef _DEBUG_REGION
    
    if (debugOn)
    {
        for (s32 j = 0; j < 3; j++)
        {
            if (sortDimension & 1 << j)
                coordLists[j].OuputDebug();
        }
    }

#endif
}

/****************************************************************************
*
*   neRegion::GetOverlappedPair
*
****************************************************************************/ 

neOverlapped * neRegion::GetOverlappedStatus(neRigidBodyBase * a, neRigidBodyBase * b)
{
    s32 smallIndex, largeIndex;

    if (a->id > b->id)
    {
        smallIndex = b->id;
        largeIndex = a->id;
    }
    else
    {
        smallIndex = a->id;
        largeIndex = b->id;
    }
    
    neOverlapped * o;

    if (largeIndex >= totalBodies)
    {
        //b overlapping p
        //if (smallIndex >= maxRigidBodies) // ab 2 ab, return
        //  return NULL;

        ASSERT(smallIndex < totalBodies); //ab 2 ab

        ASSERT(((largeIndex - totalBodies) + smallIndex * maxParticle) < b2p.GetTotalSize());
        
        o = &b2p[(largeIndex - totalBodies) + smallIndex * maxParticle];
    }
    else
    {
        //b overlapping b
        ASSERT(((largeIndex * (largeIndex - 1)) / 2) + smallIndex < b2b.GetTotalSize());
        
        o = &b2b[((largeIndex * (largeIndex - 1)) / 2) + smallIndex];
    }
    return o;
}
/****************************************************************************
*
*   neRegion::ToggleOverlapStatus
*
****************************************************************************/ 

void neRegion::ToggleOverlapStatus(neRigidBodyBase * a, neRigidBodyBase * b, neByte dimp2)
{
    neOverlapped * o = GetOverlappedStatus(a,b);

    ASSERT(o);
    
    if (o->status == sortDimension)
    {
        o->status ^= dimp2;

        //if (o->status != sortDimension && (sim->colTable.Get(a->cid, b->cid) != neCollisionTable::NE_COLLISION_IGNORE))
        if (o->status != sortDimension && o->pairItem)
        {
            //remove
            overlappedPairs.Dealloc(o->pairItem);

            o->pairItem = NULL;
        }
    }
    else
    {
        o->status ^= dimp2;

        if (o->status == sortDimension && (sim->colTable.Get(a->cid, b->cid) != neCollisionTable::RESPONSE_IGNORE))
        {
            if (overlappedPairs.usedCount >= sim->sizeInfo.overlappedPairsCount)
            {
                sprintf(sim->logBuffer, "Overlap Pair buffer full. Increase buffer size.\n");
                sim->LogOutput(neSimulator::LOG_OUTPUT_LEVEL_ONE);
                return;
            }
            //insert
            o->pairItem = overlappedPairs.Alloc();

            o->pairItem->bodyA = a;

            o->pairItem->bodyB = b;
        }
    }
    //zDebugMessage("toggle in %d, between objects %d and %d\n", dimp2 >> 1, a->id, b->id);
    //ASSERT(overlappedPairs.unusedCount <= 1);
}

/****************************************************************************
*
*   neRegion::ResetOverlapStatus
*
****************************************************************************/ 

void neRegion::ResetOverlapStatus(neRigidBodyBase * a, neRigidBodyBase * b)
{
    neOverlapped * o = GetOverlappedStatus(a,b);

    o->status = a->IsAABOverlapped(b);

    if (o->status == sortDimension)
    {
        o->pairItem = overlappedPairs.Alloc();

        neRigidBody_ * ra = a->AsRigidBody();

        neRigidBody_ * rb = b->AsRigidBody();
        
        if (ra)
        {
            if (ra->IsParticle())
            {
                if (rb)
                {
                    ASSERT(!rb->IsParticle());

                    o->pairItem->bodyA = b;

                    o->pairItem->bodyB = a;
                }
                else
                {
                    o->pairItem->bodyA = a;

                    o->pairItem->bodyB = b;
                }
            }
            else
            {
                o->pairItem->bodyA = a;

                o->pairItem->bodyB = b;
            }
        }
        else
        {
            o->pairItem->bodyA = b;

            o->pairItem->bodyB = a;
        }
    }
    else
    {
        o->pairItem = NULL;
    }
}

void neRegion::MakeTerrain(neTriangleMesh * tris)
{
    terrainTree.BuildTree(tris->vertices, tris->vertexCount, tris->triangles, tris->triangleCount, sim->allocator);
}

void neRegion::FreeTerrain()
{
    terrainTree.FreeTree();
}

void neRegion::InsertCoordList(neRigidBodyBase * bb, neRigidBodyBase * hint)
{
    for (s32 i = 0; i < 3; i++)
    {
        if (sortDimension & (1 << i) )
        {
            coordLists[i].Add(bb, hint, i);
        }
        else
        {
            bb->maxCoord[i] = NULL;
            bb->minCoord[i] = NULL;
        }
    }
}

/****************************************************************************
*
*   neCoordList::Add
*
****************************************************************************/ 

void neCoordList::Add(neRigidBodyBase * bb, neRigidBodyBase * hint, s32 hintCoord)
{
    CCoordListEntryItem * startSearch = coordList.usedTail;

    CCoordListEntry * lentry = coordList.Alloc();
    
    lentry->bb = bb;
    
    lentry->flag = CCoordListEntry::LowEnd;
    
    bb->minCoord[dim] = lentry;

    CCoordListEntry * hentry = coordList.Alloc();

    hentry->bb = bb;

    hentry->flag = CCoordListEntry::HighEnd;

    bb->maxCoord[dim] = hentry;

    if (bb->AsCollisionBody())
    {
        bb->AsCollisionBody()->UpdateAABB();
    }
    else
    {
        bb->AsRigidBody()->UpdateAABB();
    }

    if (!startSearch)
    {
        return;
    }

    CCoordListEntryItem * lentryItem = (CCoordListEntryItem*)lentry;

    CCoordListEntryItem * hentryItem = (CCoordListEntryItem*)hentry;

    if (lentryItem->thing.value >= lentryItem->prev->thing.value)
    {
        //already in place
        return;
    }

    lentryItem->Remove();

    hentryItem->Remove();

    coordList.usedTail = startSearch;
    
    if (hint)
    {
        if (hint->minBound[hintCoord] == 0)
        {
            hint = NULL;
        }
        else
        {
            ASSERT(hint->minBound[hintCoord] && hint->maxBound[hintCoord]);
        }
    }

    if (!hint)
    {
        //search from the end of the list

        CCoordListEntryItem* cur = startSearch;
        
        neBool done = false;

        do 
        {
            if (lentryItem->thing.value < cur->thing.value)
            {
                if (cur->prev)
                {
                    cur = cur->prev;
                }
                else
                {
                    cur->Insert(lentryItem);

                    coordList.used = lentryItem;

                    done = true;
                }
            }
            else
            {
                if (cur == coordList.usedTail)
                    coordList.usedTail = lentryItem;
                
                cur->Append(lentryItem);

                done = true;
            }
        } while (!done);

        cur = startSearch;

        done = false;

        do 
        {
            if (hentryItem->thing.value <= cur->thing.value)
            {
                ASSERT(hentryItem->thing.bb != cur->thing.bb); //should go pass the same body's low end

                if (cur->prev)
                {
                    cur = cur->prev;
                }
                else
                {
                    cur->Insert(hentryItem);

                    coordList.used = hentryItem;

                    done = true;
                }
            }
            else
            {
                if (cur == coordList.usedTail)
                    coordList.usedTail = hentryItem;

                cur->Append(hentryItem);

                done = true;
            }
        } while (!done);
    }
    else
    {
        //search from where the hint is

        for (s32 i = 0; i < 2; i++)
        {
            CCoordListEntryItem * entryItem;

            CCoordListEntryItem* cur;

            neBool searchUp;
            
            if (i == 0)
            {
                entryItem = lentryItem;
                
                cur = (CCoordListEntryItem*)hint->minCoord[hintCoord];

                if (entryItem->thing.value < cur->thing.value)
                    searchUp = true;
                else
                    searchUp = false;
            }
            else
            {
                cur = lentryItem;

                entryItem = hentryItem;

                searchUp = false;
            }
            
            if (searchUp)
            {
                neBool done = false;

                do 
                {
                    if (entryItem->thing.value < cur->thing.value)
                    {
                        if (cur->prev)
                        {
                            cur = cur->prev;
                        }
                        else
                        {
                            cur->Insert(entryItem);

                            coordList.used = entryItem;

                            done = true;
                        }
                    }
                    else
                    {
                        if (cur == coordList.usedTail)
                            coordList.usedTail = entryItem;
                        
                        cur->Append(lentryItem);

                        done = true;
                    }
                } while (!done);
            }
            else
            {
                neBool done = false;

                do 
                {
                    if (entryItem->thing.value >= cur->thing.value)
                    {
                        if (cur->next)
                        {
                            cur = cur->next;
                        }
                        else
                        {
                            cur->Append(entryItem);

                            coordList.usedTail = entryItem;

                            done = true;
                        }
                    }
                    else
                    {
                        if (cur == coordList.used)
                            coordList.used = entryItem;

                        cur->Insert(entryItem);

                        done = true;
                    }
                } while (!done);
            }
        }
    }
}

/****************************************************************************
*
*   neCoordList::Sort
*
****************************************************************************/ 

void neCoordList::Sort(bool sortOnly)
{
    neDLinkList<CCoordListEntry>::listItem * sortStart = coordList.used;

    if (! sortStart)
        return;

    coordList.used = sortStart->next;
    
    sortStart->Remove();

    neDLinkList<CCoordListEntry>::listItem * usedStart = sortStart;

    neDLinkList<CCoordListEntry>::listItem * usedTail = NULL;

    while (coordList.used)
    {
        neDLinkList<CCoordListEntry>::listItem * nextUsed;

        nextUsed = coordList.used->next;
        
        coordList.used->Remove();

        neDLinkList<CCoordListEntry>::listItem * insert = coordList.used;

        // insert sort start here
        bool done = false;

        if (insert->thing.flag == CCoordListEntry::LowEnd)
        {
            neDLinkList<CCoordListEntry>::listItem * cur = sortStart;

            while (!done && cur)
            {
                //compare
                if (cur->thing.bb == insert->thing.bb)
                {
                    if (cur->prev)
                    {
                        cur = cur->prev;
                    }
                    else
                    {
                        cur->Insert(insert);

                        usedStart = insert;
                        
                        done = true;
                    }
                }
                else
                {
                    if (insert->thing.value < cur->thing.value)
                    {
                        // update overlap status
                        if (cur->thing.flag == CCoordListEntry::HighEnd && !sortOnly)
                        {
                            neRigidBody_ * b1 = cur->thing.bb->AsRigidBody();

                            neRigidBody_ * b2 = insert->thing.bb->AsRigidBody();

                            if (b1 && b2)
                            {
                                if (! (b1->IsParticle() && b2->IsParticle()))
                                    region->ToggleOverlapStatus(cur->thing.bb, insert->thing.bb, dimPower2);
                            }
                            else
                            {
                                region->ToggleOverlapStatus(cur->thing.bb, insert->thing.bb, dimPower2);
                            }
                        }
                        if (cur->prev)
                        {
                            cur = cur->prev;
                        }
                        else
                        {
                            cur->Insert(insert);

                            usedStart = insert;

                            done = true;
                        }
                    }
                    else
                    {
                        cur->Append(insert);
                        
                        done = true;

                        if (cur == sortStart)
                        {
                            sortStart = insert;
                        }
                    }
                }
            }
        }
        else //HighEnd
        {
            neDLinkList<CCoordListEntry>::listItem * cur = sortStart;

            while (!done && cur)
            {
                //compare
                if (cur->thing.bb == insert->thing.bb)
                {
                    cur->Append(insert);

                    done = true;

                    if (cur == sortStart)
                        sortStart = insert;
                }
                else
                {
                    if (insert->thing.value < cur->thing.value)
                    {
                        // update overlap status
                        if (cur->thing.flag == CCoordListEntry::LowEnd && !sortOnly)
                        {
                            neRigidBody_ * b1 = cur->thing.bb->AsRigidBody();

                            neRigidBody_ * b2 = insert->thing.bb->AsRigidBody();

                            if (b1 && b2)
                            {
                                if (! (b1->IsParticle() && b2->IsParticle()))
                                    region->ToggleOverlapStatus(cur->thing.bb, insert->thing.bb, dimPower2);
                            }
                            else
                            {
                                region->ToggleOverlapStatus(cur->thing.bb, insert->thing.bb, dimPower2);
                            }
                        }

                        if (cur->prev)
                        {
                            cur = cur->prev;
                        }
                        else
                        {
                            cur->Insert(insert);

                            usedStart = insert;

                            done = true;
                        }
                    }
                    else
                    {
                        cur->Append(insert);
                        
                        done = true;

                        if (cur == sortStart)
                            sortStart = insert;
                    }
                }
            }
        }

        // insert sort end

        coordList.used = nextUsed;
    }

    coordList.used = usedStart;

    coordList.usedTail = sortStart;
}

#ifdef _DEBUG_REGION
void neCoordList::OuputDebug()
{
    neDLinkList<CCoordListEntry>::iterator iter;

    zDebugMessage("Coord %d\n", this->dim);

    for (iter = coordList.BeginUsed(); iter.Valid(); iter++)
    {
        CCoordListEntry * c = (*iter);

        char * high = "High";
        char * low = "Low";

        char * fs;

        if (c->flag == CCoordListEntry::LowEnd)
            fs = low;
        else
            fs = high;
        
        zDebugMessage("object id = %d   %s    %f \n", c->bb->id, fs, c->value);
    }
}
#endif //_DEBUG


/**restcontact.cpp**/

#pragma inline_recursion( on )
#pragma inline_depth( 250 )

//#include <stdio.h>

/*
#ifdef _WIN32
#include <windows.h>
#endif
*/

//#include "tokamak.h"
//#include "containers.h"
//#include "scenery.h"
//#include "collision.h"
//#include "constraint.h"
//#include "rigidbody.h"
//#include "scenery.h"
//#include "stack.h"
//#include "simulator.h"
//#include "message.h"

#define ne_Default_Mass 1.0f

#define CONTACT_VALIDITY_NORMAL_DISTANCE (0.01f)
#define CONTACT_VALIDITY_TANGENT_DISTANCE_SQ (0.001f)//(0.01f * 0.01f)

/****************************************************************************
*
*   neRigidBody_::CheckForIdle
*
****************************************************************************/ 
neBool TestZeroInTriangle(const neV3 & _p1, const neV3 & _p2, const neV3 & _p3)
{
    ASSERT(_p1.IsFinite());
    ASSERT(_p2.IsFinite());
    ASSERT(_p3.IsFinite());

    neV3 p1, p2, p3, average;

    average = _p1 + _p2 + _p3;

    average *= (1.0f / 3.0f);

    //increase the triangle by 10%
    p1 = average + (_p1 - average) * 1.1f;
    p2 = average + (_p2 - average) * 1.1f;
    p3 = average + (_p3 - average) * 1.1f;

    //p1 cross p2
    const s32 x = 0;
    const s32 z = 2;

    f32 cross12 = p1[z] * p2[x] - p1[x] * p2[z];
    f32 cross23 = p2[z] * p3[x] - p2[x] * p3[z];
    f32 cross31 = p3[z] * p1[x] - p3[x] * p1[z];

    if (cross12 > 0.0f && cross23 > 0.0f && cross31 > 0.0f)
    {
        return true;
    }
    else if (cross12 < 0.0f && cross23 < 0.0f && cross31 < 0.0f)
    {
        return true;
    }
    return false;
}

void neRigidBody_::CheckForIdle()
{
    CheckForIdleNonJoint();
/*
    if (CheckStationary())
    {
        if ((stackInfo && IsRestPointStillValid()))
        {
            s32 hull = CheckRestHull();

            if (hull >= 2)
            {
                //if (hull == 2 && lowEnergyCounter > 100)
                //  BecomeIdle();
                //else if (hull > 2)
                    BecomeIdle();
            }
        }
    }
*/
}
#pragma optimize( "", off )     
void neRigidBody_::CheckForIdleNonJoint()
{
    //return;
    
    f32 e = Derive().linearVel.Dot(Derive().linearVel);

    f32 f = (Derive().angularVel.Dot(Derive().angularVel)) * 1.0f;

    if (e < sim->highEnergy && f < sim->highEnergy)
    {
        lowEnergyCounter++;
    }
    else
    {
        s32 i;

        neV3 total; total.SetZero();

        for (i = 0; i < NE_RB_MAX_PAST_RECORDS; i++)
        {
            total += GetVelRecord(i);
        }

        neV3 t;
        t = total *(1.0f / NE_RB_MAX_PAST_RECORDS);
        total = t;

        ASSERT(total.IsFinite());

        f32 g; g = total.Dot(sim->gravityVector);

        if (g > 0.0f)
        {
            total.RemoveComponent(sim->gravityVector);
        }

        f32 v = total.Dot(total);

        if (v > 2.0f)//5
        {
            lowEnergyCounter = 0;
            return;
        }
        total.SetZero();

        for (i = 0; i < NE_RB_MAX_PAST_RECORDS; i++)
        {
            total += GetAngVelRecord(i);
        }
        total *= (1.0f / NE_RB_MAX_PAST_RECORDS);

        v = total.Dot(total);

        if (v > 1.0f)//4
        {
            lowEnergyCounter = 0;
            return;
        }
        lowEnergyCounter+=1;
    }
    if (lowEnergyCounter > 10)
    {
        if ((stackInfo && IsRestPointStillValid()))
        {
            s32 hull = CheckRestHull();

            if (hull)
            {
                if (hull == 2 && lowEnergyCounter > 100)
                    BecomeIdle();
                else if (hull > 2)
                    BecomeIdle();
            }
        }
    }
}
#pragma optimize( "", on )
void neRigidBody_::CheckForIdleJoint()
{
    f32 e = Derive().linearVel.Length();

    f32 f = col.boundingRadius * Derive().angularVel.Length();

    e += f;

    if (e < sleepingParam)
    {
        lowEnergyCounter++;
    }
    else
    {
        lowEnergyCounter = 0;
    }
    if (lowEnergyCounter > 50)
    {
        // calculate net force
        //NE_RB_MAX_PAST_RECORDS

        neV3 sum;

        sum.SetZero();

        for (s32 i = 0; i < NE_RB_MAX_PAST_RECORDS; i++)
        {
            sum += dvRecord[i];
        }

        ASSERT(sum.IsFinite());

        neV3 average = sum;// / (f32)NE_RB_MAX_PAST_RECORDS;

        f32 len1 = average.Length();

        if (len1 < sleepingParam/*0.3f*/)
        {
            BecomeIdle();
        }
    }
}

s32 neRigidBody_::CheckRestHull()
{
    if (totalForce.IsConsiderZero())
        return 3;
    
    neM3 forceFrame;

    forceFrame[1] = totalForce;

    forceFrame[1].Normalize();

    void ChooseAxis(neV3 & x, neV3 & y, const neV3 & normal);

    ChooseAxis(forceFrame[0], forceFrame[2], forceFrame[1]);

    forceFrame.SetTranspose(forceFrame);

    neV3 p[3];
    
    if (GetRestHull().htype == neRestHull::TRIANGLE)
    {
        neBool allIdle = true;

        for (s32 i = 0; i < 3; i++)
        {
            if (!GetRestRecord(i).IsValid())
            {
                ASSERT(0);
            }
            if (!GetRestRecord(i).CanConsiderOtherBodyIdle())
            {
                allIdle = false;

                break;
            }
            p[i] = GetRestRecord(i).worldThisBody - GetPos();

            p[i] = forceFrame.TransposeMulV3(p[i]);
        }
        if (!allIdle)
            return 0;
        
        neBool ret = TestZeroInTriangle(p[0], p[1], p[2]);

        if (ret)
            return 3;
    }
    if (GetRestHull().htype == neRestHull::LINE)
    {
        neBool allIdle = true;

        for (s32 i = 0; i < 2; i++)
        {
            s32 j = GetRestHull().indices[i];

            if (!GetRestRecord(j).IsValid())
            {
                allIdle = false;

                break;
            }
            if (!GetRestRecord(j).CanConsiderOtherBodyIdle())
            {
                allIdle = false;

                break;
            }
            p[i] = GetRestRecord(j).worldThisBody - GetPos();

            p[i] = forceFrame.TransposeMulV3(p[i]);

            p[i][1] = 0.0f;
        }
        if (!allIdle)
            return 0;
        
        p[2].SetZero();
        
        f32 d = p[2].GetDistanceFromLine(p[0], p[1]);

        if (d < 0.005f)
            return 2;

        return false;
    }
    else if (GetRestHull().htype == neRestHull::POINT)
    {
        neBool allIdle = true;

        s32 i = GetRestHull().indices[0];

        if (!GetRestRecord(i).IsValid())
        {
            return 0;
        }
        if (!GetRestRecord(i).CanConsiderOtherBodyIdle())
        {
            return 0;
        }
        p[0] = GetRestRecord(GetRestHull().indices[0]).worldThisBody - GetPos();

        p[0] = forceFrame.TransposeMulV3(p[0]);

        p[0][1] = 0.0f;
        
        f32 d = sqrtf(p[0][0] * p[0][0] + p[0][2] * p[0][2]);

        if (d < 0.005f)
            return 1;

        return false;
    }
    else
    {
        return false;
    }
}

neBool neRigidBody_::CheckStillIdle()
{
    if (!CheckHighEnergy())
    {
        if (subType == NE_RIGID_PARTICLE || CheckRestHull())
        {
            ZeroMotion();

            UpdateController();
            
            sim->idleBodyCount++;
            return true;
        }
        else
        {
            return false;
        }
        
    }
    else
    {
        WakeUp();

        return false;
    }
}

s32 neRigidBody_::CheckContactValidity()
{
    if (!stackInfo)
        return 0;
    
    s32 validCount = 0;

    s32 i;

    neBool allIdle = false;

    if (status == neRigidBody_::NE_RBSTATUS_IDLE && !isShifted)
    {
        allIdle = true;

        for (i = 0; i < NE_RB_MAX_RESTON_RECORDS; i++)
        {
            if (!GetRestRecord(i).IsValid())
            {
                continue;
            }
                
            if (!GetRestRecord(i).CanConsiderOtherBodyIdle())
            {
                allIdle = false;

                break;
            }
            validCount++;
        }
    }

    if (allIdle)
    {
        return validCount;
    }

    validCount = 0;

    for (i = 0; i < NE_RB_MAX_RESTON_RECORDS; i++)
    {
        if (!GetRestRecord(i).IsValid())
            continue;

        GetRestRecord(i).Update();

        f32 d = GetRestRecord(i).normalWorld.Dot(sim->gravityVector);

        if (d > 0.0f/*-TILT_TOLERANCE*/)
        {
            GetRestRecord(i).SetInvalid();
            GetRestHull().htype = neRestHull::NONE;
            continue;
        }
        if (GetRestRecord(i).normalDiff > CONTACT_VALIDITY_NORMAL_DISTANCE)
        {
            GetRestRecord(i).SetInvalid();
            GetRestHull().htype = neRestHull::NONE;
            continue;
        }
        if (GetRestRecord(i).tangentialDiffSq > CONTACT_VALIDITY_TANGENT_DISTANCE_SQ)
        {
            GetRestRecord(i).SetInvalid();
            GetRestHull().htype = neRestHull::NONE;
            continue;
        }
        validCount++;
    }
    if (validCount == 0)
        stackInfo->isBroken = true;
    else
        stackInfo->isBroken = false;
    
    return validCount;
}

s32 neRigidBody_::AddContactImpulseRecord(neBool withConstraint)
{
    s32 i = 0;
    static neV3 world1[NE_RB_MAX_RESTON_RECORDS];
    static neV3 world2[NE_RB_MAX_RESTON_RECORDS];
    static neV3 diff[NE_RB_MAX_RESTON_RECORDS];
    static f32 height[NE_RB_MAX_RESTON_RECORDS];
    s32 validCount = 0;
    static s32 validIndices[NE_RB_MAX_RESTON_RECORDS];
    s32 deepestIndex = -1;
    f32 deepest = -1.0e6f;

    for (i = 0; i < NE_RB_MAX_RESTON_RECORDS; i++)
    {
        if (!GetRestRecord(i).IsValid())
            continue;

        if (!GetRestRecord(i).CheckOtherBody(sim))
        {
            GetRestRecord(i).SetInvalid();

            continue;
        }
        GetRestRecord(i).Update();

        world1[i] = State().b2w * GetRestRecord(i).bodyPoint;

        world2[i] = GetRestRecord(i).GetOtherBodyPoint();

        diff[i] = world1[i] - world2[i];

        neV3 d; d = diff[i];

        d.RemoveComponent(GetRestRecord(i).normalWorld);

        if (d.Dot(d) > 0.025f)
        {
            GetRestRecord(i).SetInvalid();

            GetRestHull().htype = neRestHull::NONE;

            continue;
        }

        height[i] = diff[i].Dot(GetRestRecord(i).normalWorld);

        validIndices[validCount] = i;

        validCount++;

        if (height[i] > deepest)
        {
            deepest = height[i];

            deepestIndex = i;
        }
    }
    if (validCount == 0)
    {       
        GetRestHull().htype = neRestHull::NONE;
        return 0;
    }

    if (0)//subType == NE_RIGID_PARTICLE)
    {
        ASSERT(deepestIndex != -1);

        i = deepestIndex;

        neCollisionResult tmpcr;
        neCollisionResult * cresult = &tmpcr;

        cresult->bodyA = (neRigidBodyBase*)this;
        cresult->bodyB = GetRestRecord(i).GetOtherBody();
        cresult->collisionFrame[2] = GetRestRecord(i).normalWorld;
        cresult->contactA = world1[i] - GetPos();
        cresult->contactB = world2[i] - GetRestRecord(i).GetOtherBody()->GetB2W().pos;
        cresult->materialIdA = GetRestRecord(i).material;
        cresult->materialIdB = GetRestRecord(i).otherMaterial;
        cresult->depth = -GetRestRecord(i).normalDiff;//GetRestRecord(i).depth; 
        cresult->impulseType = IMPULSE_CONTACT;
        cresult->impulseScale = 1.0f;
        cresult->PrepareForSolver();
        
        if (withConstraint || !cresult->CheckIdle())
        {
            sim->AddCollisionResult(tmpcr);         
        }
    }

    s32 j;
    
    f32 heightScale = 1.0f;

    if (validCount == 1 && height[validIndices[0]] < 0.0f)
    {
        //if (subType == NE_RIGID_NORMAL)
        {
            i = validIndices[0];

            neCollisionResult tmpcr;
            neCollisionResult * cresult = &tmpcr;

            cresult->bodyA = (neRigidBodyBase*)this;
            cresult->bodyB = GetRestRecord(i).GetOtherBody();
            cresult->collisionFrame[2] = GetRestRecord(i).normalWorld;
            ASSERT(cresult->collisionFrame[2].IsFinite());
            cresult->contactA = world1[i] - GetPos();
            cresult->contactB = world2[i] - GetRestRecord(i).GetOtherBody()->GetB2W().pos;
            cresult->materialIdA = GetRestRecord(i).material;
            cresult->materialIdB = GetRestRecord(i).otherMaterial;
            cresult->depth = -GetRestRecord(i).normalDiff;//GetRestRecord(i).depth; 
            cresult->impulseType = IMPULSE_CONTACT;
            //cresult->UpdateConstraintRelativeSpeed();
            cresult->PrepareForSolver();
            cresult->impulseScale = 1.0f;

            if (withConstraint || !cresult->CheckIdle())
            {
                //*sim->cresultHeap.Alloc(0) = tmpcr;
                sim->AddCollisionResult(tmpcr);
            }
        }           
        GetRestHull().htype = neRestHull::NONE;
        return 1;
    }
    else if (validCount == 2)
    {
        s32 v1  = validIndices[0];
        s32 v2  = validIndices[1];

        neV3 d1 = world1[v1] - world1[v2];
        neV3 d2 = world2[v1] - world2[v2];

        f32 len1 = d1.Length();

        if (neIsConsiderZero(len1))
        {
            heightScale = 1.0f;
        }
        else
        {
            f32 len2 = d2.Length();

            if (neIsConsiderZero(len2))
            {
                heightScale = 1.0f;
            }
            else
            {
                d1 *= (1.0f / len1);

                d2 *= (1.0f / len2);

                heightScale = neAbs(d1.Dot(d2));
            }
        }

        ASSERT(neIsFinite(heightScale));
        //if (!neIsFinite(heightScale))
        //  heightScale = 1.0f;
    }
    else if (validCount == 3)
    {
        neV3 tri1[3];
        neV3 tri2[3];

        tri1[0] = world1[1] - world1[0];
        tri1[1] = world1[2] - world1[1];
        tri1[2] = world1[0] - world1[2];

        tri2[0] = world2[1] - world2[0];
        tri2[1] = world2[2] - world2[1];
        tri2[2] = world2[0] - world2[2];
        
        neV3 normal1 = tri1[1].Cross(tri1[0]);
        neV3 normal2 = tri2[1].Cross(tri2[0]);

        f32 len1 = normal1.Length();
        
        if (neIsConsiderZero(len1))
        {
            heightScale = 1.0f;
        }
        else
        {
            f32 len2 = normal2.Length();

            if (neIsConsiderZero(len2))
            {
                heightScale = 1.0f;
            }
            else
            {
                normal1 *= (1.0f / len1);

                normal2 *= (1.0f / len2);

                heightScale = neAbs(normal1.Dot(normal2));
            }
        }
        ASSERT(neIsFinite(heightScale));
        //if (!neIsFinite(heightScale))
        //  heightScale = 1.0f;
    }
    
    f32 e = 0.0005f;
    f32 f = 1.0f - e;

    heightScale = (heightScale - f) / e;

    if (heightScale < 0.0f)
    {
        heightScale = e;
    }
    s32 actualValidCount = 0;
    //f32 limit = 0.05f;
    f32 limit = 0.01f;

    for (i = 0; i < validCount; i++)
    {
        f32 scale = 1.0f;

        j = validIndices[i];

        f32 scaleLimit = 0.01f;//limit * heightScale;

        if (height[j] > 0)
        {
            if (height[j] > scaleLimit)
            {
                //GetRestRecord(j).rtype = neRestRecord::REST_ON_NOT_VALID;
                //GetRestHull().htype = neRestHull::NONE;
                continue;
            }
            scale = (scaleLimit - height[j]) / scaleLimit;

            scale = scale * scale * scale;
        }
        //if (subType == NE_RIGID_NORMAL)
        {
            neCollisionResult tmpcr;
            neCollisionResult * cresult = &tmpcr;//sim->cresultHeap.Alloc(0);

            cresult->bodyA = (neRigidBodyBase*)this;
            cresult->bodyB = GetRestRecord(j).GetOtherBody();
            cresult->collisionFrame[2] = GetRestRecord(j).normalWorld;
            cresult->contactA = world1[j] - GetPos();
            cresult->contactB = world2[j] - GetRestRecord(j).GetOtherBody()->GetB2W().pos;
            cresult->materialIdA = GetRestRecord(j).material;
            cresult->materialIdB = GetRestRecord(j).otherMaterial;
            cresult->depth = -GetRestRecord(j).normalDiff;//GetRestRecord(j).depth;
            cresult->impulseType = IMPULSE_CONTACT;
            //cresult->UpdateConstraintRelativeSpeed();
            cresult->PrepareForSolver();
            cresult->impulseScale = scale;          
            if (withConstraint || !cresult->CheckIdle())
            {
                //*sim->cresultHeap.Alloc(0) = tmpcr;
                sim->AddCollisionResult(tmpcr);
            }
        }
        //sim->HandleCollision(this, GetRestRecord(j).otherBody, cresult, IMPULSE_NORMAL, scale);

        GetRestHull().indices[actualValidCount++] = j;
    }
    if (actualValidCount >= 2)
    {
        if (actualValidCount == 2)
        {
            GetRestHull().htype = neRestHull::LINE;
            
            ASSERT(GetRestRecord(GetRestHull().indices[0]).IsValid());
            ASSERT(GetRestRecord(GetRestHull().indices[1]).IsValid());

            GetRestHull().normal = GetRestRecord(GetRestHull().indices[0]).normalWorld +
                                    GetRestRecord(GetRestHull().indices[1]).normalWorld;
/*
            neV3 diff = world2[GetRestHull().indices[0]] - world2[GetRestHull().indices[1]];

            neV3 cross = diff.Cross(sim->gravityVector);

            GetRestHull().normal= cross.Cross(diff);
*/
            GetRestHull().normal.Normalize();

            if (GetRestHull().normal.Dot(sim->gravityVector) < 0.0f)
                GetRestHull().normal *= -1.0f;
        }
        else
        {
            GetRestHull().htype = neRestHull::TRIANGLE;

            ASSERT(GetRestRecord(GetRestHull().indices[0]).IsValid());
            ASSERT(GetRestRecord(GetRestHull().indices[1]).IsValid());
            ASSERT(GetRestRecord(GetRestHull().indices[2]).IsValid());

            GetRestHull().normal = GetRestRecord(GetRestHull().indices[0]).normalWorld +
                                    GetRestRecord(GetRestHull().indices[1]).normalWorld +
                                    GetRestRecord(GetRestHull().indices[2]).normalWorld;

/*
            neV3 diff1 = world2[GetRestHull().indices[0]] - world2[GetRestHull().indices[1]];

            neV3 diff2 = world2[GetRestHull().indices[2]] - world2[GetRestHull().indices[0]];

            GetRestHull().normal = diff1.Cross(diff2);
*/
            GetRestHull().normal.Normalize();

            if (GetRestHull().normal.Dot(sim->gravityVector) < 0.0f)
                GetRestHull().normal *= -1.0f;
        }
    }
    else
    {
        if (actualValidCount == 1)
        {
            GetRestHull().htype = neRestHull::POINT;

            ASSERT(GetRestRecord(GetRestHull().indices[0]).IsValid());

            GetRestHull().normal = GetRestRecord(GetRestHull().indices[0]).normalWorld;

            if (GetRestHull().normal.Dot(sim->gravityVector) < 0.0f)
                GetRestHull().normal *= -1.0f;
        }
        else
        {
            GetRestHull().htype = neRestHull::NONE;
        }
    }
    return actualValidCount;
}

void neRigidBody_::AddContactConstraint()
{
    if (stackInfo->stackHeader == &sim->stackHeaderX)
    {
        if (needSolveContactDynamic)
        {
            needSolveContactDynamic = false;
        
            AddContactImpulseRecord(true);
        }
    }
    else
    {
        if (stackInfo->stackHeader->dynamicSolved)
            return;

        neByte ** p = sim->pointerBuffer2.Alloc();

        ASSERT(p);

        *p = (neByte*)(stackInfo->stackHeader);

        stackInfo->stackHeader->dynamicSolved = true;
    }
}

neBool neRigidBody_::CheckHighEnergy()
{
    f32 e;
    
    f32 m;

    if (0)//_constraintHeader)
    {
        e = Derive().linearVel.Length();

        e += col.boundingRadius * Derive().angularVel.Length();

        m = 0.5f;
    }
    else
    {
        e = Derive().linearVel.Dot(Derive().linearVel);

        e += Derive().angularVel.Dot(Derive().angularVel);

        m = sim->highEnergy;
    }

    if (e < m)
        return false;
    
    return true;
}

neBool neRigidBody_::CheckStationary()
{
//  return false;

    const s32 oldCounterMax = 60;

    const f32 StationarySpeed = sleepingParam;//0.2f;
    const f32 StationaryAcc = 5.0f;
    const f32 StationaryW = 10.f;
    const f32 StationaryAngAcc = 10.5f;

    if (oldCounter < oldCounterMax)
        return FALSE;

    neV3 deltaPos = State().b2w.pos - oldPosition;

    neV3 vel = deltaPos / (sim->_currentTimeStep * oldCounterMax);

    f32 speed = vel.Length();

    if (speed > StationarySpeed)
    {
        SyncOldState();

        return FALSE;
    }

    neV3 deltaVel = Derive().linearVel - oldVelocity;

    neV3 acc = deltaVel / (sim->_currentTimeStep * oldCounterMax);

    f32 accMag = acc.Length();

    if (accMag > StationaryAcc)
    {
        SyncOldState();

        return FALSE;
    }

    neQ oldQInvert = oldRotation;

    oldQInvert.Invert();

    neQ deltaQ = State().q * oldQInvert;

    neV3 axis; f32 angle;

    deltaQ.GetAxisAngle(axis, angle);

    f32 angularVel = angle / (sim->_currentTimeStep * oldCounterMax);

    if (angularVel > StationaryW)
    {
        SyncOldState();

        return FALSE;
    }

    neV3 deltaW = Derive().angularVel - oldAngularVelocity;

    f32 angularAcc = deltaW.Length();
    
    angularAcc /= (sim->_currentTimeStep * oldCounterMax);

    if (angularAcc > StationaryAngAcc)
    {
        SyncOldState();

        return FALSE;
    }
/*  Derive().linearVel *= 0.9f;

    neV3 am = State().angularMom * 0.9f;

    SetAngMom(am);
*/
    return true;
}

/****************************************************************************
*
*   neRigidBody_::AddStackInfo
*
****************************************************************************/ 

neBool neRigidBody_::AddStackInfo(neRestRecord & rc)
{
    if (!stackInfo)
        return NewStackInfo(rc);

    neRigidBody_ * rb = (neRigidBody_ *) rc.GetOtherBody();

    ASSERT(stackInfo->stackHeader);

    if (stackInfo->isTerminator)
    {
        stackInfo->isTerminator = false;

        //ResetRestOnRecords();
    }

    AddRestContact(rc);
    
    if (rc.GetOtherCollisionBody())
    {
        return true;
    }

    if (!rb->stackInfo)
    {
        if (stackInfo->stackHeader->isHeaderX)
        {
            sim->stackHeaderX.Remove(stackInfo);

            sim->NewStackHeader(stackInfo);

            return rb->NewStackInfoTerminator(stackInfo->stackHeader);
        }
        else
        {
            return rb->NewStackInfoTerminator(stackInfo->stackHeader);
        }
    }
    
    neStackHeader * otherStackHeader = rb->stackInfo->stackHeader;

    ASSERT(otherStackHeader);

    if (otherStackHeader->isHeaderX)
    {
        if (stackInfo->stackHeader->isHeaderX)
        {
            sim->stackHeaderX.Remove(stackInfo);
            
            sim->stackHeaderX.Remove(rb->stackInfo);

            sim->NewStackHeader(stackInfo);

            stackInfo->stackHeader->Add(rb->stackInfo);

            //stackInfo->stackHeader->CheckHeader();
        }
        else
        {
            otherStackHeader->Remove(rb->stackInfo);

            stackInfo->stackHeader->Add(rb->stackInfo);

            //stackInfo->stackHeader->CheckHeader();
        }
    }
    else
    {
        if (stackInfo->stackHeader->isHeaderX)
        {
            stackInfo->stackHeader->Remove(stackInfo);

            otherStackHeader->Add(stackInfo);

            //otherStackHeader->CheckHeader();
        }
        else 
        {
            if (stackInfo->stackHeader != otherStackHeader)
            {
                // merge
                otherStackHeader->ChangeHeader(stackInfo->stackHeader);

                //stackInfo->stackHeader->CheckHeader();

                sim->stackHeaderHeap.Dealloc(otherStackHeader);
            }
        }
    }
    return true;
}

void neRigidBody_::ResetRestOnRecords()
{
/*  for (s32 i = 0; i < NE_MAX_REST_ON; i++)
    {
        GetRestRecord(i).Init();
    }
*/
}

void  neRigidBody_::FreeStackInfo()
{
    ASSERT(stackInfo);

    sim->stackInfoHeap.Dealloc(stackInfo, 1);

    stackInfo = NULL;
}

neBool neRigidBody_::NewStackInfo(neRestRecord & rc)
{
    ASSERT(stackInfo == NULL);

    stackInfo = sim->stackInfoHeap.Alloc(1);

    if (!stackInfo)
    {
        if (sim->logLevel >= neSimulator::LOG_OUTPUT_LEVEL_ONE)
        {
            sprintf(sim->logBuffer, MSG_STACK_BUFFER_FULL);
            sim->LogOutput(neSimulator::LOG_OUTPUT_LEVEL_ONE);
        }
        return false;
    }

    stackInfo->Init();

    {
        ASSERT(AllRestRecordInvalid());
    }
    ResetRestOnRecords();
    
    stackInfo->body = this;

    stackInfo->isTerminator = false;

    AddRestContact(rc);

    if (rc.GetOtherCollisionBody())
    {
        sim->stackHeaderX.Add(stackInfo);

        return true;
    }

    neRigidBody_ * rb = (neRigidBody_ *) rc.GetOtherBody();
    
    if (!rb->stackInfo)
    {
        sim->NewStackHeader(stackInfo);
        
        ASSERT(stackInfo->stackHeader);
        
        return rb->NewStackInfoTerminator(stackInfo->stackHeader);
    }

    neStackHeader * otherStackHeader = rb->stackInfo->stackHeader;

    ASSERT(otherStackHeader);

    if (otherStackHeader->isHeaderX)
    {
        sim->stackHeaderX.Remove(rb->stackInfo);

        sim->NewStackHeader(stackInfo);

        stackInfo->stackHeader->Add(rb->stackInfo);

        //stackInfo->stackHeader->CheckHeader();
    }
    else
    {
        otherStackHeader->Add(stackInfo);

        //otherStackHeader->CheckHeader();
    }
    if (!rb->_constraintHeader)
        rb->WakeUp();
    
    rb = rc.GetOtherRigidBody();
    
    if (rb && !rb->_constraintHeader)
    {
        rb->WakeUp();
    }
    return true;
}

neBool neRigidBody_::NewStackInfoTerminator(neStackHeader * header)
{
    ASSERT(stackInfo == NULL);

    stackInfo = sim->stackInfoHeap.Alloc(1);

    ASSERT(stackInfo);

    if (!stackInfo)
    {
        if (sim->logLevel >= neSimulator::LOG_OUTPUT_LEVEL_ONE)
        {
            sprintf(sim->logBuffer, MSG_STACK_BUFFER_FULL);
            sim->LogOutput(neSimulator::LOG_OUTPUT_LEVEL_ONE);
        }
        return false;
    }
    stackInfo->Init();

    {
        ASSERT(AllRestRecordInvalid());
    }

    ResetRestOnRecords();

    stackInfo->body = this;

    stackInfo->isTerminator = true;

    header->Add(stackInfo);

    //header->CheckHeader();

    return true;
}

void neRigidBody_::MigrateNewHeader(neStackHeader * newHeader, neStackHeader * curHeader)
{
    ASSERT(stackInfo);
    ASSERT(stackInfo->stackHeader != newHeader);

    ASSERT(curHeader == stackInfo->stackHeader);

    neStackHeader * oldHeader = stackInfo->stackHeader;

    oldHeader->Remove(stackInfo);

    newHeader->Add(stackInfo);

//  oldHeader->CheckHeader();

    for (s32 i = 0; i < NE_RB_MAX_RESTON_RECORDS; i++)
    {
        

        if (GetRestRecord(i).GetOtherBody())
        {
            neRigidBody_* otherBody = GetRestRecord(i).GetOtherRigidBody();

            if (otherBody)
            {
                if (!otherBody->stackInfo)
                    continue;

                if (otherBody->stackInfo->stackHeader == newHeader)
                    continue;

                if (!otherBody->stackInfo->isTerminator)
                    otherBody->MigrateNewHeader(newHeader, curHeader);          
            }
        }
    }
}

neBool neRigidBody_::IsRestPointStillValid()
{
    if (!stackInfo || stackInfo->isTerminator)
        return false;
    
    s32 count = 0;

    switch (GetRestHull().htype)
    {
    case neRestHull::LINE:
        count = 2;
        //return false;
        break;

    case neRestHull::TRIANGLE:
        count = 3;
        break;
    default:
        return false;
    }

    for (s32 i = 0; i < count; i++)
    {
        neV3 world1, world2;

        s32 j = GetRestHull().indices[i];

        ASSERT(j < 3);
        ASSERT(j >= 0);

        ASSERT(GetRestRecord(j).IsValid());
        //if (GetRestRecord(j).rtype != neRestRecord::REST_ON_NOT_VALID)
        //  continue;

        world1 = State().b2w * GetRestRecord(j).bodyPoint;

        world2 = GetRestRecord(j).GetOtherBodyPoint();
        
        neV3 diff; diff = world1 - world2;

        //diff.RemoveComponent(sim->gravityVector); //remove the vertical component

        f32 d = diff.Dot(diff);

        //if (d > 0.02f) // 0.05 M or 5 cm
        if (d > 0.002f) // 0.05 M or 5 cm
        {
            GetRestRecord(j).SetInvalid();
            
            GetRestHull().htype = neRestHull::NONE;

            return false;
        }
    }
    return true;
}
/*
void neRigidBody_::ResolveRestingPenetration()
{
    s32 i;

    neBool s = false;

    for (i = 0; i < NE_RB_MAX_RESTON_RECORDS; i++)
    {
        if (!GetRestRecord(i).IsValid())
            continue;

        if ((GetRestRecord(i).otherBody != sim->GetTerrainBody()) && 
            (!GetRestRecord(i).otherBody->IsValid() || !GetRestRecord(i).otherBody->isActive))
        {
            GetRestRecord(i).rtype = neRestRecord::REST_ON_NOT_VALID;

            GetRestRecord(i).otherBody = NULL;

            continue;
        }
        GetRestRecord(i).Update();

        if (GetRestRecord(i).normalDiff >= 0.0f)
            continue;

        if (neAbs(GetRestRecord(i).normalDiff) < -0.005f)
            continue;

//      if (neAbs(GetRestRecord(i).normalDiff) > 0.9f)
//          continue;

        s = true;

        CorrectPenetrationDrift2(i, true, 1);
    }
}
*/
#if 1

void neRigidBody_::CorrectRotation(f32 massOther, neV3 & pointThis, neV3 & pointDest, neV3 & pointDest2, s32 flag, s32 changeLast)
{
    neV3 dir1 = pointThis - GetPos();
    
    neV3 dir2 = pointDest - GetPos();

    f32 len1 = dir1.Length();

    if (neIsConsiderZero(len1) || !neIsFinite(len1))
        return;

    f32 len2 = dir2.Length();

    if (neIsConsiderZero(len2) || !neIsFinite(len2))
        return;

    dir1 *= (1.0f / len1);

    dir2 *= (1.0f / len2);

    f32 dot = dir1.Dot(dir2);

    if (neIsConsiderZero(neAbs(dot) - 1.0f))
        return;
    
    neV3 axis = dir1.Cross(dir2);

    axis.Normalize();

    f32 angle = acosf(dot);

    neQ quat; quat.Set(angle, axis);

    quat.Normalize();

    if (flag == 1)
    //if (1)
    {
        State().q = quat * State().q;

        UpdateDerive();
    }
    else
    {
        totalRot = quat * totalRot;

        rotCount++;

        if (changeLast)
        {
            totalLastRot = quat * totalLastRot;

            lastRotCount++;
        }
    }
}

#else

void neRigidBody_::CorrectRotation(f32 massOther, neV3 & pointThis, neV3 & pointDest, neV3 & pointDest2, s32 flag, s32 changeLast)
{
    neV3 p1 = pointThis - GetPos();
    
    neV3 p2 = pointDest2 - pointThis;

    f32 dot = p1.Dot(p2);

    neV3 cross = p1.Cross(p2);

    f32 len = cross.Length();

    if (!neIsFinite(len) || neIsConsiderZero(len))
    {
        return;
    }

    neV3 magic; magic = Derive().Iinv * cross;

    neV3 deltaR = (mass * massOther) / (mass + massOther) * magic;

    f32 angle = deltaR.Length();

    deltaR *= (1.0f / angle);

    //if (angle > 1.0f)
    //  angle = 1.0f;

    neQ quat; quat.Set(angle, deltaR);

    if (flag == 1)
    {
        State().q = quat * State().q;

        UpdateDerive();
    }
    else
    {
        totalRot = quat * totalRot;

        rotCount++;

        if (changeLast)
        {
            totalLastRot = quat * totalLastRot;

            lastRotCount++;
        }
    }
}

#endif
    

void neRigidBody_::CorrectPosition(neV3 & pointThis, neV3 & pointDest, s32 flag, s32 changeLast)
{
    neV3 shift = pointDest - pointThis;

    if (flag == 1)
    //if (1)
    {
        State().b2w.pos = GetPos() + shift;
        //SetPos(GetPos() + shift);
    }
    else
    {
        totalTrans += shift;

        transCount++;

        if (changeLast)
        {
            totalLastTrans += shift;

            lastTransCount++;
        }
    }
}
/*
void neRigidBody_::CorrectPenetrationRotation()
{
    if (!stackInfo)
        return;
    
    s32 i;

    s32 deepestIndex = -1;

    f32 deepest = 0.0f;

    for (i = 0; i < NE_RB_MAX_RESTON_RECORDS; i++)
    {
        if (GetRestRecord(i).rtype == neRestRecord::REST_ON_NOT_VALID)
            continue;

        if ((GetRestRecord(i).otherBody != sim->GetTerrainBody()) && 
            (!GetRestRecord(i).otherBody->IsValid() || !GetRestRecord(i).otherBody->isActive))
        {
            GetRestRecord(i).rtype = neRestRecord::REST_ON_NOT_VALID;

            GetRestRecord(i).otherBody = NULL;

            continue;
        }
        GetRestRecord(i).Update();

        if (GetRestRecord(i).normalDiff >= 0.0f)
            continue;
        
        if (GetRestRecord(i).normalDiff >= -0.005f) // never move things out completely
            continue;

        if (neAbs(GetRestRecord(i).normalDiff) > 1.0f)
            continue;

        CorrectPenetrationRotation2(i, false);
    }
}

void neRigidBody_::CorrectPenetrationTranslation()
{
    if (!stackInfo)
        return;
    
    s32 i;

    s32 deepestIndex = -1;

    f32 deepest = 0.0f;

    for (i = 0; i < NE_RB_MAX_RESTON_RECORDS; i++)
    {
        if (GetRestRecord(i).rtype == neRestRecord::REST_ON_NOT_VALID)
            continue;

        if ((GetRestRecord(i).otherBody != sim->GetTerrainBody()) && 
            (!GetRestRecord(i).otherBody->IsValid() || !GetRestRecord(i).otherBody->isActive))
        {
            GetRestRecord(i).rtype = neRestRecord::REST_ON_NOT_VALID;

            GetRestRecord(i).otherBody = NULL;

            continue;
        }
        GetRestRecord(i).Update();

        if (GetRestRecord(i).normalDiff >= 0.0f)
            continue;
        
        if (GetRestRecord(i).normalDiff >= -0.005f) // never move things out completely
            continue;

        if (neAbs(GetRestRecord(i).normalDiff) > 1.0f)
            continue;

        CorrectPenetrationTranslation2(i, false);
    }
}
*/
void neRigidBody_::CorrectPenetrationRotation2(s32 index, neBool slide)
{
/*  neRigidBodyBase * rb = GetRestRecord(index).otherBody;

    f32 effectiveMassA, effectiveMassB, mass2;

    neV3 dir = GetRestRecord(index).normalWorld;

    neV3 pointA = GetRestRecord(index).worldThisBody;

    neV3 pointB = GetRestRecord(index).worldOtherBody;

    f32 alinear, arotate;

    f32 blinear, brotate;

    effectiveMassA = TestImpulse(dir, pointA, alinear, arotate);

    if (rb->AsRigidBody())
    {
        effectiveMassB = rb->AsRigidBody()->TestImpulse(dir * -1.0f, pointB, blinear, brotate);

        mass2 = rb->AsRigidBody()->mass;
    }
    else
    {
        effectiveMassB = 0.0f;

        mass2 = 1.0e6f;
    }

    neV3 diff = pointA - pointB;

    f32 dot;

    slide = 0;

    if (slide)
    {
        dot = diff.Dot(GetRestRecord(index).normalWorld);

        diff = dot * GetRestRecord(index).normalWorld;
    }

    f32 scale = 0.5f;

    neV3 midA = pointA - (effectiveMassA) / (effectiveMassA + effectiveMassB) * diff * scale * arotate;

    neV3 midB = pointB + (effectiveMassB) / (effectiveMassA + effectiveMassB) * diff * scale * brotate;

    CorrectRotation(mass2, pointA, midA, pointB, 0, true);

    if (rb->AsRigidBody())
    {
        rb->AsRigidBody()->CorrectRotation(mass, pointB, midB, pointA, 0, true);
    }
*/
}

void neRigidBody_::CorrectPenetrationTranslation2(s32 index, neBool slide)
{
/*
    neRigidBodyBase * rb = GetRestRecord(index).otherBody;

    f32 effectiveMassA, effectiveMassB, mass2;

    neV3 pointA = State().b2w * GetRestRecord(index).bodyPoint;

    neV3 pointB = rb->GetB2W() * GetRestRecord(index).otherBodyPoint;

    neV3 dir = GetRestRecord(index).normalWorld;

    f32 alinear, arotate;

    f32 blinear, brotate;

    effectiveMassA = TestImpulse(dir, pointA, alinear, arotate);

    if (rb->AsRigidBody())
    {
        effectiveMassB = rb->AsRigidBody()->TestImpulse(dir * -1.0f, pointB, blinear, brotate);

        mass2 = rb->AsRigidBody()->mass;
    }
    else
    {
        effectiveMassB = 0.0f;

        mass2 = 1.0e6f;
    }

    neV3 diff = pointA - pointB;

    slide = 0;
    
    if (slide)
    {
        f32 dot = diff.Dot(GetRestRecord(index).normalWorld);

        diff = dot * GetRestRecord(index).normalWorld;
    }

    neV3 midA, midB;

    f32 scale = 0.5f;

    if (!slide)
        midA = pointA - (effectiveMassA) / (effectiveMassA + effectiveMassB) * diff * scale * alinear;
    else
        midA = pointA - (effectiveMassA) / (effectiveMassA + effectiveMassB) * diff * scale * alinear;

    midB = pointB + (effectiveMassB) / (effectiveMassA + effectiveMassB) * diff * scale * blinear;
    
    CorrectPosition(pointA, midA, 0, true);

    if (rb->AsRigidBody())
        rb->AsRigidBody()->CorrectPosition(pointB, midB, 0, true);
*/
}
#if 0
void neRigidBody_::CorrectPenetrationDrift()
{
    if (!stackInfo)
        return;
    
    s32 i;

    s32 deepestIndex = -1;

    f32 deepest = 0.0f;

    for (i = 0; i < NE_RB_MAX_RESTON_RECORDS; i++)
    {
        if (GetRestRecord(i).rtype == neRestRecord::REST_ON_NOT_VALID)
            continue;

        if ((GetRestRecord(i).otherBody != sim->GetTerrainBody()) && 
            (!GetRestRecord(i).otherBody->IsValid() || !GetRestRecord(i).otherBody->isActive))
        {
            GetRestRecord(i).rtype = neRestRecord::REST_ON_NOT_VALID;

            GetRestRecord(i).otherBody = NULL;

            continue;
        }
        GetRestRecord(i).Update();

        if (GetRestRecord(i).normalDiff >= 0.0f)
            continue;
        
        if (GetRestRecord(i).normalDiff >= -0.005f) // never move things out completely
            continue;

        if (neAbs(GetRestRecord(i).normalDiff) > 2.0f)
            continue;

        CorrectPenetrationDrift2(i, false, 1);
/*
        if (GetRestRecord(i).normalDiff < deepest)
        {
            deepestIndex = i;

            deepest = GetRestRecord(i).normalDiff;
        }
*/  }
//  if (deepestIndex != -1)
//      CorrectPenetrationDrift2(deepestIndex, false);
}

void neRigidBody_::CorrectPenetrationDrift2(s32 index, neBool slide, s32 flag)
{
    // remember current position

//  neV3 posA = GetPos();

    neQ currentQ = State().q;

    currentQ.Invert();

    //neQ deltaQA = correctionInfo.lastQuat * currentQ;

//  neV3 posB;

    //neQ deltaQB;

/*  if (GetRestRecord(index).otherBody->AsRigidBody())
    {
        posB = GetRestRecord(index).otherBody->AsRigidBody()->GetPos();

        currentQ = GetRestRecord(index).otherBody->AsRigidBody()->State().q;

        currentQ.Invert();

        deltaQB =  GetRestRecord(index).otherBody->AsRigidBody()->correctionInfo.lastQuat * 
                                currentQ;
    }
*/
    f32 err = 0.0f;
    
    neRigidBodyBase * rb = GetRestRecord(index).otherBody;

    f32 effectiveMassA, effectiveMassB, mass2;

    neV3 dir = GetRestRecord(index).normalWorld;

    neV3 pointA = GetRestRecord(index).worldThisBody;

    neV3 pointB = GetRestRecord(index).worldOtherBody;

    f32 alinear, arotate;

    f32 blinear, brotate;

    effectiveMassA = TestImpulse(dir, pointA, alinear, arotate);

    if (rb->AsRigidBody())
    {
        neV3 tmp = dir * -1.0f;

        effectiveMassB = rb->AsRigidBody()->TestImpulse(tmp, pointB, blinear, brotate);

        mass2 = rb->AsRigidBody()->mass;
    }
    else
    {
        effectiveMassB = 0.0f;

        mass2 = 1.0e6f;

        brotate = 0.0f;

        blinear = 0.0f;
    }
    //effectiveMassB = 0.0f;

    neV3 diff = pointA - pointB;

    f32 dot;

    if (slide)
    {
        dot = diff.Dot(GetRestRecord(index).normalWorld);

        diff = dot * GetRestRecord(index).normalWorld;
    }

    f32 scaleA = 0.8f;

    f32 scaleB = 0.1f;

    neV3 midA = pointA - (effectiveMassA) / (effectiveMassA + effectiveMassB) * diff * arotate * scaleA;

    neV3 midB = pointB + (effectiveMassB) / (effectiveMassA + effectiveMassB) * diff * brotate * scaleB;

    CorrectRotation(mass2, pointA, midA, pointB, flag, false);

    if (rb->AsRigidBody())
    {
        rb->AsRigidBody()->CorrectRotation(mass, pointB, midB, pointA, flag, false);
    }
    pointA = State().b2w * GetRestRecord(index).bodyPoint;

    pointB = rb->GetB2W() * GetRestRecord(index).otherBodyPoint;

    diff = pointA - pointB;

    if (slide)
    {
        dot = diff.Dot(GetRestRecord(index).normalWorld);

        diff = dot * GetRestRecord(index).normalWorld;
    }

    if (!slide)
        midA = pointA - (effectiveMassA) / (effectiveMassA + effectiveMassB) * diff * alinear * scaleA;
    else
        midA = pointA - (effectiveMassA) / (effectiveMassA + effectiveMassB) * diff * alinear * scaleA;

    midB = pointB + (effectiveMassB) / (effectiveMassA + effectiveMassB) * diff * blinear * scaleB;
    
    CorrectPosition(pointA, midA, flag, false);

    if (rb->AsRigidBody())
        rb->AsRigidBody()->CorrectPosition(pointB, midB, flag, false);
/*
    neV3 shifted = GetPos() - posA;

    correctionInfo.lastPos += shifted;

    correctionInfo.lastQuat = deltaQA * State().q;

    if (GetRestRecord(index).otherBody->AsRigidBody())
    {
        shifted = GetRestRecord(index).otherBody->AsRigidBody()->GetPos() - posB;

        GetRestRecord(index).otherBody->AsRigidBody()->correctionInfo.lastPos += shifted;

        GetRestRecord(index).otherBody->AsRigidBody()->correctionInfo.lastQuat = deltaQB * 
            GetRestRecord(index).otherBody->AsRigidBody()->State().q;
    }
*/
}
#endif

f32 neRigidBody_::TestImpulse(neV3 & dir, neV3 & pt, f32 & linear, f32 & angular)
{
    neV3 point = pt - GetPos();

    neV3 dv = dir * oneOnMass;

    neV3 da = point.Cross(dir);

    neV3 angVel = Derive().Iinv * da;

    neV3 vel;

    neV3 dav = angVel.Cross(point);

    vel = dv + dav;

    //neV3 dist = vel;// / sim->currentTimeStep;

    f32 linearSpeed = dv.Length();

    f32 angularSpeed = dav.Length();

    f32 totalSpeed = linearSpeed + angularSpeed;

    linear = linearSpeed / totalSpeed;

    angular = angularSpeed / totalSpeed;

    f32 ret = linearSpeed + angularSpeed;//dist.Length();

    if (neIsFinite(ret))
        return ret;
    
    return 0.0f;
}

void neRigidBody_::ShiftPosition(const neV3 & delta)
{
    neConstraintHeader * header = GetConstraintHeader();

    SetPos(GetPos() + delta);
}

neBool neRigidBody_::AllRestRecordInvalid() 
{
    for (s32 i = 0; i < 3; i++)
    {
        if (GetRestRecord(i).IsValid())
            return false;
    }
    return true;
}


/**rigidbodybase.cpp**/

#pragma inline_recursion( on )
#pragma inline_depth( 250 )

//#include <stdio.h>

/*
#ifdef _WIN32
#include <windows.h>
#endif
*/

//#include "tokamak.h"
//#include "containers.h"
//#include "scenery.h"
//#include "collision.h"
//#include "constraint.h"
//#include "rigidbody.h"
//#include "scenery.h"
//#include "stack.h"
//#include "simulator.h"
//#include "message.h"

/****************************************************************************
*
*   neRigidBodyBase::GetConvex
*
****************************************************************************/ 
/*
TConvex * neRigidBodyBase::GetConvex(s32 index)
{
    ASSERT(index < col.convexCount);

    if (col.convexCount == 0)
        return NULL;

    if (col.convexCount == 1 && index == 0)
        return &col.obb;

    if (index == -1)
    {
        return &col.obb;
    }
    
    return &col.convex[index];
}
*/
void neRigidBodyBase::CollideConnected(neBool yes)
{
    isCollideConnected = yes;
}

neBool neRigidBodyBase::CollideConnected()
{
    return isCollideConnected;
}

void neRigidBodyBase::RecalcBB()
{
    col.CalcBB();

    neV3 maxExt, minExt;

    col.obb.GetExtend(minExt, maxExt);

    neSensorItem * si = (neSensorItem *)sensors;

    while (si)
    {
        neSensor_ * sensor = (neSensor_ *) si;

        si = si->next;

        neV3 sstart;
        neV3 send;

        sstart = sensor->pos;
        send = sensor->pos + sensor->dir;

        for (s32 j = 0; j < 3; j++)
        {
            maxExt[j] = neMax(maxExt[j], sstart[j]);
            maxExt[j] = neMax(maxExt[j], send[j]);
            minExt[j] = neMin(minExt[j], sstart[j]);
            minExt[j] = neMin(minExt[j], send[j]);
        }
    }

    for (s32 i = 0; i < 3; i++)
    {
        col.obb.as.box.boxSize[i] = ( maxExt[i] - minExt[i] ) * 0.5f;
        col.obb.c2p.pos[i] = minExt[i] + col.obb.as.box.boxSize[i];
    }

    obb.rot[0] = col.obb.as.box.boxSize[0] * col.obb.c2p.rot[0];
    obb.rot[1] = col.obb.as.box.boxSize[1] * col.obb.c2p.rot[1];
    obb.rot[2] = col.obb.as.box.boxSize[2] * col.obb.c2p.rot[2];
    obb.pos = col.obb.c2p.pos;

};

/*
neV3 neRigidBodyBase::VelocityAtPoint(const neV3 & pt)
{
    neV3 ret;

    if (AsCollisionBody())
    {
        ret.SetZero();

        return ret;
    }
    else
    {
        ret = ((neRigidBody_*)this)->Derive().linearVel;

        ret += ((neRigidBody_*)this)->Derive().angularVel.Cross(pt);

        return ret;
    }
}
*/
neSensor_ * neRigidBodyBase::AddSensor()
{
    neSensor_ * newSensor = sim->sensorHeap.Alloc(1);
    
    if (!newSensor)
    {
        sprintf(sim->logBuffer, MSG_RUN_OUT_SENSOR);

        sim->LogOutput(neSimulator::LOG_OUTPUT_LEVEL_ONE);

        return NULL;
    }
    if (sensors)
    {
        //((neSensorItem *)sensors)->Append((neSensorItem *)newSensor);

        neSensorItem * sitem = (neSensorItem *)sensors;

        while (sitem->next)
        {
            sitem = sitem->next;
        }
        sitem->Append((neSensorItem *)newSensor);
    }
    else
    {
        sensors = newSensor;
    }
    return newSensor;
}

void neRigidBodyBase::BeginIterateSensor()
{
    sensorCursor = (neSensorItem *)sensors;
}

neSensor_ * neRigidBodyBase::GetNextSensor()
{
    if (!sensorCursor)
        return NULL;

    neSensor_ * ret = (neSensor_ *)sensorCursor;

    sensorCursor = sensorCursor->next;

    return ret;
}

void neRigidBodyBase::ClearSensor()
{
    neSensorItem * si = (neSensorItem *)sensors;

    while (si)
    {
        neSensor_ * s = (neSensor_ *) si;

        si = si->next;

        s->depth = 0.0f;

        s->body = NULL;
    }
}

TConvex * neRigidBodyBase::AddGeometry()
{
    TConvex * newConvex = sim->geometryHeap.Alloc(1);

    if (!newConvex)
    {
        sprintf(sim->logBuffer, MSG_RUN_OUT_GEOMETRY);

        sim->LogOutput(neSimulator::LOG_OUTPUT_LEVEL_ONE);

        return NULL;
    }
    newConvex->Initialise();
    
    if (col.convex)
    {
        TConvexItem * citem = (TConvexItem *)col.convex;

        while (citem)
        {
            if (!citem->next)
            {
                citem->Append((TConvexItem *)newConvex);        
                
                break;
            }
            else
            {
                citem = citem->next;
            }
        }
    }
    else
    {
        col.convex = newConvex;
    }
    col.convexCount++;

    if (isActive && !IsInRegion())
        sim->region.AddBody(this, NULL);

    return newConvex;
}

void neRigidBodyBase::BeginIterateGeometry()
{
    geometryCursor = (TConvexItem *)col.convex;
}

TConvex * neRigidBodyBase::GetNextGeometry()
{
    if (!geometryCursor)
        return NULL;

    TConvex * ret = (TConvex *)geometryCursor;

    geometryCursor = geometryCursor->next;

    return ret;
}

void neRigidBodyBase::RemoveConstraintHeader()
{
    neConstraintHeader * h = GetConstraintHeader();

    if (h)
    {
        h->bodies.Remove(&constraintHeaderItem);

        h->flag = neConstraintHeader::FLAG_NEED_REORG;

        SetConstraintHeader(NULL);

        if (h->bodies.count == 0)
        {
            sim->constraintHeaders.Dealloc(h);
        }
    }
}

void neRigidBodyBase::Free()
{
    //free sensor
    
    neFreeListItem<neSensor_> * si = (neFreeListItem<neSensor_> *) sensors;

    while (si)
    {
        neFreeListItem<neSensor_> * next = si->next;

        //si->Remove();

        sim->sensorHeap.Dealloc((neSensor_*)si, 1);

        si = next;
    }
    sensors = NULL;

    //remove from region
    if (IsInRegion())
        sim->region.RemoveBody(this);

    //free geometry

    neFreeListItem<TConvex> * gi = (neFreeListItem<TConvex> *) col.convex;

    while (gi)
    {
        neFreeListItem<TConvex> * next = gi->next;

        //gi->Remove();

        sim->geometryHeap.Dealloc((TConvex*)gi, 1);

        gi = next;
    }
    col.convex = NULL;

    col.convexCount = 0;

    //free constraint
    neConstraintHandle * chandle = constraintCollection.GetHead();

    while (chandle)
    {
        _neConstraint * c = chandle->thing;

        chandle = constraintCollection.GetNext(chandle);

        c->bodyA->constraintCollection.Remove(&c->bodyAHandle);

        if (c->bodyB)
            c->bodyB->constraintCollection.Remove(&c->bodyBHandle);

        if (GetConstraintHeader())
            GetConstraintHeader()->Remove(c);

        neFreeListItem<neController> * ci = (neFreeListItem<neController> *) c->controllers;

        while (ci)
        {
            neFreeListItem<neController> * next = ci->next;

            ci->Remove();

            sim->controllerHeap.Dealloc((neController *)ci, 1);

            ci = next;
        }
        c->controllers = NULL;

        sim->constraintHeap.Dealloc(c, 1);
    }
    neRestRecordHandle * rhandle = rbRestingOnMe.GetHead();

    while (rhandle)
    {
        neRestRecord * r = rhandle->thing;

        rhandle = rbRestingOnMe.GetNext(rhandle);

        r->SetInvalid();
    };
}

neBool neRigidBodyBase::IsValid()
{
    if (btype == NE_OBJECT_COLISION)
    {
        return ((neList<neCollisionBody_>::itemType *)this)->state;// sim->abHeap.IsInUse((neCollisionBody_*)this);
    }
    else
    {
        return ((neList<neRigidBody_>::itemType *)this)->state;//sim->rbHeap.IsInUse((neRigidBody_*)this);
    }
}

neT3 & neRigidBodyBase::GetB2W()
{
    if (btype == NE_OBJECT_COLISION)
    {
        return AsCollisionBody()->b2w;
    }
    else
    {
        return AsRigidBody()->State().b2w;
    }
}

void neRigidBody_::DrawCPointLine()
{
    return;

#if 0
    neConstraintPointArray & pointArray = GetRBCData().GetCPointArray();

    for (s32 i = 0; i < pointArray.GetUsedCount(); i++)
    {
        for (s32 j = i + 1; j < pointArray.GetUsedCount(); j++)
        {
//          if (pointArray[i].constraint == pointArray[j].constraint)
//              continue;
            
            neV3 points[2];
            neV3 color;

            points[0] = *pointArray[i].GetPtResult(this);
            points[1] = *pointArray[j].GetPtResult(this);
            f32 test = (points[0] - points[1]).Length();
            DrawLine(color, points, 2);
        }
    }
    if (calignMethod == ALIGN_POINT_ORIGIN ||
        calignMethod == ALIGN_LINE_ORIGIN ||
        calignMethod == ALIGN_TRI_AUX)
    {
        for (i = 0; i < pointArray.GetUsedCount(); i++)
        {
            neV3 points[2];
            neV3 color;

            points[0] = *pointArray[i].GetPtResult(this);
            points[1] = auxCPoints[1];

            f32 test = (points[0] - points[1]).Length();
            DrawLine(color, points, 2);
        }

    }
#endif
}

void neRigidBodyBase::Active(neBool yes, neRigidBodyBase * hint)
{
    if (isActive && yes)
        return;

    if (isActive) //make inactive
    {
        if (AsCollisionBody())
        {
            sim->activeCB.Remove((neCollisionBody_*)this);

            sim->inactiveCB.Add((neCollisionBody_*)this);
        }
        else
        {
            if (AsRigidBody()->IsParticle())
            {
                sim->activeRP.Remove((neRigidBody_*)this);

                sim->inactiveRP.Add((neRigidBody_*)this);
            }
            else
            {
                sim->activeRB.Remove((neRigidBody_*)this);

                sim->inactiveRB.Add((neRigidBody_*)this);
            }
        }
        //remove from region

        if (IsInRegion())
            sim->region.RemoveBody(this);

        isActive = false;
    }
    else //make active
    {
        if (AsCollisionBody())
        {
            sim->inactiveCB.Remove((neCollisionBody_*)this);

            sim->activeCB.Add((neCollisionBody_*)this);
        }
        else
        {
            if (AsRigidBody()->IsParticle())
            {
                sim->inactiveRP.Remove((neRigidBody_*)this);

                sim->activeRP.Add((neRigidBody_*)this);
            }
            else
            {
                sim->inactiveRB.Remove((neRigidBody_*)this);

                sim->activeRB.Add((neRigidBody_*)this);
            }
        }
        //insert into the region

        if (col.convexCount > 0 || isCustomCD)
            sim->region.AddBody(this, hint);

        isActive = true;
    }
}


/**rigidbody.cpp**/

#pragma inline_recursion( on )
#pragma inline_depth( 250 )

//#include <stdio.h>

/*
#ifdef _WIN32
#include <windows.h>
#endif
*/

//#include "tokamak.h"
//#include "containers.h"
//#include "scenery.h"
//#include "collision.h"
//#include "constraint.h"
//#include "rigidbody.h"
//#include "scenery.h"
//#include "stack.h"
//#include "simulator.h"
//#include "message.h"

#define ne_Default_Mass 1.0f

//extern void DrawLine(const neV3 & colour, neV3 * startpoint, s32 count);


/****************************************************************************
*
*   neRigidBodyState::neRigidBodyState
*
****************************************************************************/ 

neRigidBodyState::neRigidBodyState()
{
    q.Identity();

    b2w.SetIdentity();
    
//  b2w.pos.SetZero(); // x
    
    angularMom.SetZero(); // L
    
//  rot().SetIdentity();
    
    linearMom.SetZero();
}

/****************************************************************************
*
*   neRigidBody_::~neRigidBody_
*
****************************************************************************/ 

neRigidBody_::~neRigidBody_()
{

}

/****************************************************************************
*
*   neRigidBody_::neRigidBody_
*
****************************************************************************/ 

neRigidBody_::neRigidBody_()
{
    btype = NE_OBJECT_RIGID;

    rbExtra = &eggs;

    curState = 0;
    
    mass = ne_Default_Mass;

    oneOnMass = 1.0f / ne_Default_Mass;
    
    IbodyInv.SetZero();
    
    IbodyInv.M[0][0] = oneOnMass;
    IbodyInv.M[1][1] = oneOnMass;
    IbodyInv.M[2][2] = oneOnMass;

    Ibody.SetZero();

    IbodyInv.M[0][0] = ne_Default_Mass;
    IbodyInv.M[1][1] = ne_Default_Mass;
    IbodyInv.M[2][2] = ne_Default_Mass;

    cookies = 0;

    force.SetZero();
    torque.SetZero();

    gravityOn = true;

    status = NE_RBSTATUS_NORMAL;
    
    SetConstraintHeader(NULL);

    s32 i;

    for (i = 0; i < NE_RB_MAX_RESTON_RECORDS; i++)
    {
        GetRestRecord(i).Init();
    }
    for (i = 0; i < NE_RB_MAX_PAST_RECORDS; i++)
    {
        GetVelRecord(i).SetZero();
        GetAngVelRecord(i).SetZero();
        dvRecord[i].SetZero();
        davRecord[i].SetZero();
    }

    stackInfo = NULL;

    lowEnergyCounter = 0;

    GetRestHull().htype = neRestHull::NONE;

    isShifted = isShifted2 = false;

    controllers = NULL;

    controllerCursor = NULL;

    gforce.SetZero();

    cforce.SetZero();

    ctorque.SetZero();

    subType = NE_RIGID_NORMAL;

    acc.SetZero();

    angularDamp = 0.0f;

    linearDamp = 0.0f;

    sleepingParam = 0.2f;

    SyncOldState();
}

/****************************************************************************
*
*   neRigidBody_::Free
*
****************************************************************************/ 

void neRigidBody_::Free()
{
    neRigidBodyBase::Free();

    //free controller

    neFreeListItem<neController> * ci = (neFreeListItem<neController> *) controllers;

    while (ci)
    {
        neFreeListItem<neController> * next = ci->next;

        ci->Remove();

        sim->controllerHeap.Dealloc((neController *)ci, 1);

        ci = next;
    }
    controllers = NULL;

    //free constraint header

    RemoveConstraintHeader();

    //free stack
    
    if (stackInfo)
    {
        ASSERT(stackInfo->stackHeader);

        stackInfo->stackHeader->Remove(stackInfo);

        //sim->stackInfoHeap.Dealloc(stackInfo, 1);
        FreeStackInfo();
    }
    for (s32 i = 0; i < NE_MAX_REST_ON; i++)
    {
        GetRestRecord(i).SetInvalid();
    }
}

/****************************************************************************
*
*   neRigidBody_::RecalcInertiaTensor
*
****************************************************************************/ 

void neRigidBody_::RecalcInertiaTensor()
{
    if (col.convexCount == 0)   
        return;

    neM3 tensor;
    neV3 cogShift;
    f32 _mass = 0.0f;

    f32 density = 0.0f;
    f32 friction = 0.0f;
    f32 restitution = 0.0f;

    cogShift.SetZero();

    tensor.SetZero();

    if (col.convexCount == 1)
    {
        sim->GetMaterial(col.obb.GetMaterialId(), friction, restitution, density);

        tensor = col.obb.CalcInertiaTensor(density, _mass);
        
        IbodyInv.SetInvert(tensor);

        mass = _mass;

        cogShift = col.obb.c2p.pos * _mass;
    }
    else
    {
        s32 i;
        f32 m;
        neM3 _tensor;

        for (i = 0; i < col.convexCount; i++)
        {
            sim->GetMaterial(col.convex[i].GetMaterialId() , friction, restitution, density);

            _tensor = col.convex[i].CalcInertiaTensor(density, m);
            
            tensor += _tensor;

            _mass += m;

            cogShift += col.convex[i].c2p.pos * _mass;
        }

        mass = _mass;       
    }
    oneOnMass = 1.0f/mass;

    cogShift =  cogShift * (1.0f / mass);

    TranslateCOM(tensor, cogShift, mass, -1.0f);

    Ibody = tensor;

    IbodyInv.SetInvert(tensor);
}

/****************************************************************************
*
*   neRigidBody_::AddController
*
****************************************************************************/ 

neController * neRigidBody_::AddController(neRigidBodyControllerCallback * rbc, s32 period)
{
    neController * c = sim->controllerHeap.Alloc(1);

    if (!c)
    {
        sprintf(sim->logBuffer, MSG_CONTROLLER_FULL);
        
        sim->LogOutput(neSimulator::LOG_OUTPUT_LEVEL_ONE);

        return NULL;
    }
    if (!controllers)
    {
        controllers = c;
    }
    else
    {
        //((neControllerItem *)controllers)->Append((neControllerItem *)c);     

        neControllerItem * citem = (neControllerItem *)controllers;

        while (citem->next)
        {
            citem = citem->next;
        }
        citem->Append((neControllerItem *)c);
    }
    c->rb = this;

    c->constraint = NULL;

    c->rbc = rbc;

    c->jc = NULL;

    c->period = period;

    c->count = period;

    c->forceA.SetZero();

    c->torqueA.SetZero();

    c->forceB.SetZero();

    c->torqueB.SetZero();

    return c;
}

/****************************************************************************
*
*   neRigidBody_::BeginIterateController
*
****************************************************************************/ 

void neRigidBody_::BeginIterateController()
{
    controllerCursor = (neControllerItem *)controllers;
}

/****************************************************************************
*
*   neRigidBody_::GetNextController
*
****************************************************************************/ 

neController * neRigidBody_::GetNextController()
{
    if (!controllerCursor)
        return NULL;

    neController * ret = (neController *)controllerCursor;

    controllerCursor = controllerCursor->next;

    return ret;
}

/****************************************************************************
*
*   neRigidBody_::GravityEnable
*
****************************************************************************/ 

void neRigidBody_::GravityEnable(neBool yes)
{
    gravityOn = yes;
}

/****************************************************************************
*
*   neRigidBody_::UpdateDerive
*
****************************************************************************/ 
void neRigidBody_::SetAngMom(const neV3 & am)
{
    neQ     w;
    
    State().angularMom = am;

    Derive().angularVel = Derive().Iinv * State().angularMom;

    w.Set(Derive().angularVel, 0.0f);

    Derive().qDot = w * State().q;

    Derive().qDot *= 0.5f;
}

void neRigidBody_::UpdateDerive()
{
    neRigidBodyState & state = State();
    
    state.q = state.q.Normalize();

    state.rot() = state.q.BuildMatrix3();

    // Iinv = R * IbodyInv Rtrans
    //derive.Iinv = state.rot() * IbodyInv * neM3_BuildTranspose(state.rot());
    derive.Iinv = state.rot() * IbodyInv * neM3().SetTranspose(state.rot());

    // w = Iinv * L
    derive.angularVel = derive.Iinv * state.angularMom;

    neQ w;
    w.W = 0.0f;
    w.X = derive.angularVel[0];
    w.Y = derive.angularVel[1];
    w.Z = derive.angularVel[2];

    derive.qDot = w * state.q;

    derive.qDot = derive.qDot * 0.5f;

    derive.speed = derive.linearVel.Length();
}


/****************************************************************************
*
*   neRigidBody_::Advance
*
****************************************************************************/ 

void neRigidBody_::AdvanceDynamic(f32 tStep)
{
    oldCounter++;

    totalDV.SetZero();

    totalDA.SetZero();

    impulseCount = 0;

    twistCount = 0;
    
    isAddedToSolver = false;

    if (status == neRigidBody_::NE_RBSTATUS_IDLE)
    {
        if (!cforce.IsConsiderZero())
            WakeUp();
        
        if (!ctorque.IsConsiderZero())
            WakeUp();
    }

    if (status == neRigidBody_::NE_RBSTATUS_IDLE)
    {
        if (CheckStillIdle())
        {
            return;
        }
    }

    if (status == neRigidBody_::NE_RBSTATUS_ANIMATED)
    {
        return;
    }

    dvRecord[sim->stepSoFar % NE_RB_MAX_PAST_RECORDS].SetZero();
    davRecord[sim->stepSoFar % NE_RB_MAX_PAST_RECORDS].SetZero();

    neRigidBodyState & state = State();

    int newStateId = (curState + 1) % MAX_RB_STATES;

    neRigidBodyState & newState = stateBuffer[newStateId];

    totalForce += (force + gforce + cforce);

    dvRecord[sim->stepSoFar % NE_RB_MAX_PAST_RECORDS] = totalForce * oneOnMass * tStep;

    acc = totalForce * oneOnMass;

    derive.linearVel += acc * tStep;

    derive.linearVel *= (1.0f - linearDamp);

    totalTorque += (torque + ctorque);

    //L = L0 + torque * t

    MidPointIntegration(totalTorque, tStep);
}

void neRigidBody_::MidPointIntegration(const neV3 & totalTorque, f32 tStep)
{
    State().angularMom *= (1.0f - angularDamp);
    
    neV3 newAngularMom = State().angularMom + totalTorque * tStep;

    neQ tmpQ;

    tmpQ = State().q;

    f32 midStep = tStep * 0.5f;

    tmpQ = State().q + Derive().qDot * midStep;

    neV3 tmpL = State().angularMom + totalTorque  * midStep;

    tmpQ = tmpQ.Normalize();

    neM3 rot = tmpQ.BuildMatrix3();

    neM3 tmpIinv = rot * IbodyInv * neM3().SetTranspose(rot);

    neV3 tmpAngVel = tmpIinv * tmpL;

    neQ tmpW; 
    
    tmpW.Set(tmpAngVel, 0.0f);

    Derive().qDot = tmpW * tmpQ * 0.5f;

    curState = (curState + 1) % MAX_RB_STATES;

    State().angularMom = newAngularMom;

    Derive().angularVel = tmpAngVel;

    //SetAngMom(am);
}

void neRigidBody_::ImprovedEulerIntegration(const neV3 & totalTorque, f32 tStep)
{
/*  neV3 newAngularMom = State().angularMom + totalTorque * tStep;

    neQ tmpQ = State().q + Derive().qDot * tStep;

    tmpQ.Normalize();

    neM3 rot = tmpQ.BuildMatrix3();

    neV3 Iinv = rot * IbodyInv * neM3().SetTranspose(rot);

    neV3 angVel = Iinv * newAngularMom;



    curState = (curState + 1) % MAX_RB_STATES;

    State().angularMom = newAngularMom;
*/
}

void neRigidBody_::RungeKutta4Integration(const neV3 & totalTorque, f32 tStep)
{
/*  neV3 newAngularMom = State().angularMom + totalTorque * tStep;

    neQ q1, q2, q3, q4;

    neV3 L1, L2, L3, L4;

    f32 midStep = tStep * 0.5f;

    q1 = State().q + Derive().qDot * tStep;

    L1 = 

    curState = (curState + 1) % MAX_RB_STATES;

    State().angularMom = newAngularMom;
*/
}

void neRigidBody_::AdvancePosition(f32 tStep)
{
//  needSolveContactDynamic = true;

//  totalForce.SetZero();

//  totalTorque.SetZero();

//  if (status == neRigidBody_::NE_RBSTATUS_IDLE)
//      return;

    //derive.linearVel *= 0.99;

    neV3 newPos;

    newPos = GetPos() + derive.linearVel * tStep + 0.5f * acc * tStep * tStep;

    neV3 bv = newPos - GetPos();

    backupVector = 0.7f * backupVector + 0.3f * bv;

    SetPos(newPos);

//  derive.linearVel += acc * tStep;
/*  
    neQ tmpQ;

    tmpQ = State().q;

    f32 midStep = tStep * 0.5f;

    tmpQ = State().q + derive.qDot * midStep;

    neV3 tmpL = State().angularMom + totalTorque  * midStep;

    tmpQ = tmpQ.Normalize();

    neM3 rot = tmpQ.BuildMatrix3();

    neM3 tmpIinv = rot * IbodyInv * neM3().SetTranspose(rot);

    neV3 tmpAngVel = tmpIinv * tmpL;

    neQ tmpW; 
    
    tmpW.Set(tmpAngVel, 0.0f);

    neQ tmpQDot = tmpW * tmpQ * 0.5f;
*/
    //q = q0 + qdot * t

    neV3 am = State().angularMom;// * 0.95f;

    SetAngMom(am);

    State().q = State().q + Derive().qDot * tStep;

    UpdateDerive();

    UpdateController();

    if (sensors)
        ClearSensor();

    GetVelRecord(sim->stepSoFar % NE_RB_MAX_PAST_RECORDS) = Derive().linearVel;

    GetAngVelRecord(sim->stepSoFar % NE_RB_MAX_PAST_RECORDS) = Derive().angularVel;

    //CheckForIdle();
}

/****************************************************************************
*
*   neRigidBody_::UpdateController
*
****************************************************************************/ 

void neRigidBody_::UpdateController()
{   
    if (gravityOn && status != neRigidBody_::NE_RBSTATUS_IDLE)
    {
        gforce = sim->gravity * mass;
    }
    else
    {
        gforce.SetZero();
    }

    cforce.SetZero();

    ctorque.SetZero();

    neControllerItem * ci = (neControllerItem *)controllers;

    while (ci)
    {
        neController * con = (neController *) ci;

        ci = ci->next;

        if (con->count == 0)
        {
            ASSERT(con->rbc);
            
            con->rbc->RigidBodyControllerCallback((neRigidBodyController*)con, sim->_currentTimeStep);

            con->count = con->period;
        }
        else
        {
            con->count--;
        }
        con->rb->cforce += con->forceA;

        con->rb->ctorque += con->torqueA;
    }
}

/****************************************************************************
*
*   neRigidBody_::UpdateAABB
*
****************************************************************************/ 

void neRigidBody_::UpdateAABB()
{
    if (col.convexCount == 0 && !isCustomCD)
        return;

//  for (s32 i = 0; i < 3; i++)
    {
#if 0
        if (minCoord[0])
            minCoord[0]->value = p[0] /*- neAbs(c2w[0][0]) - neAbs(c2w[0][1]) - neAbs(c2w[0][2])*/ - col.boundingRadius;
        if (maxCoord[0])
            maxCoord[0]->value = p[0] /*+ neAbs(c2w[0][0]) + neAbs(c2w[0][1]) + neAbs(c2w[0][2]);//*/ + col.boundingRadius;

        if (minCoord[1])
            minCoord[1]->value = p[1] /*- neAbs(c2w[1][0]) - neAbs(c2w[1][1]) - neAbs(c2w[1][2])*/ - col.boundingRadius;
        if (maxCoord[1])
            maxCoord[1]->value = p[1] /*+ neAbs(c2w[1][0]) + neAbs(c2w[1][1]) + neAbs(c2w[1][2]);*/ + col.boundingRadius;
        
        if (minCoord[2])
            minCoord[2]->value = p[2] /*- neAbs(c2w[2][0]) - neAbs(c2w[2][1]) - neAbs(c2w[2][2])*/ - col.boundingRadius;
        if (maxCoord[2])
            maxCoord[2]->value = p[2] /*+ neAbs(c2w[2][0]) + neAbs(c2w[2][1]) + neAbs(c2w[2][2]);*/ + col.boundingRadius;
#else
        //neM3 & rot = State().rot();

        neT3 c2w = State().b2w * obb; 
/*
        neM3 c2w2 = rot * dobb;

        for (s32 i = 0; i < 3; i++)
        {
            f32 a = neAbs(c2w2[0][i]) + neAbs(c2w2[1][i]) + neAbs(c2w2[2][i]);

            debugMinBound[i] = p[i] - a;
        }
*/
/*
    c2w.M[0][0] = obb.M[0][0] * rot.M[0][0] + obb.M[0][1] * rot.M[1][0] + obb.M[0][2] * rot.M[2][0];
    c2w.M[0][1] = obb.M[0][0] * rot.M[0][1] + obb.M[0][1] * rot.M[1][1] + obb.M[0][2] * rot.M[2][1];
    c2w.M[0][2] = obb.M[0][0] * rot.M[0][2] + obb.M[0][1] * rot.M[1][2] + obb.M[0][2] * rot.M[2][2];
    c2w.M[1][0] = obb.M[1][0] * rot.M[0][0] + obb.M[1][1] * rot.M[1][0] + obb.M[1][2] * rot.M[2][0];
    c2w.M[1][1] = obb.M[1][0] * rot.M[0][1] + obb.M[1][1] * rot.M[1][1] + obb.M[1][2] * rot.M[2][1];
    c2w.M[1][2] = obb.M[1][0] * rot.M[0][2] + obb.M[1][1] * rot.M[1][2] + obb.M[1][2] * rot.M[2][2];
    c2w.M[2][0] = obb.M[2][0] * rot.M[0][0] + obb.M[2][1] * rot.M[1][0] + obb.M[2][2] * rot.M[2][0];
    c2w.M[2][1] = obb.M[2][0] * rot.M[0][1] + obb.M[2][1] * rot.M[1][1] + obb.M[2][2] * rot.M[2][1];
    c2w.M[2][2] = obb.M[2][0] * rot.M[0][2] + obb.M[2][1] * rot.M[1][2] + obb.M[2][2] * rot.M[2][2];
*/

        neV3 &p = c2w.pos;

        f32 a = neAbs(c2w.rot[0][0]) + neAbs(c2w.rot[1][0]) + neAbs(c2w.rot[2][0]);

        minBound[0] = p[0] - a;
        maxBound[0] = p[0] + a;

        if (minCoord[0])
            minCoord[0]->value = p[0] - a ;
        if (maxCoord[0])
            maxCoord[0]->value = p[0] + a;

        a = neAbs(c2w.rot[0][1]) + neAbs(c2w.rot[1][1]) + neAbs(c2w.rot[2][1]);

        minBound[1] = p[1] - a;
        maxBound[1] = p[1] + a;

        if (minCoord[1])
            minCoord[1]->value = p[1] - a;
        if (maxCoord[1])
            maxCoord[1]->value = p[1] + a;
        
        a = neAbs(c2w.rot[0][2]) + neAbs(c2w.rot[1][2]) + neAbs(c2w.rot[2][2]);

        minBound[2] = p[2] - a;
        maxBound[2] = p[2] + a;

        if (minCoord[2])
            minCoord[2]->value = p[2] - a;
        if (maxCoord[2])
            maxCoord[2]->value = p[2] + a;
#endif
    }
}

void neRigidBody_::WakeUp()
{
    status = neRigidBody_::NE_RBSTATUS_NORMAL;

    lowEnergyCounter = 0;

    SyncOldState();
}

void neRigidBody_::SyncOldState()
{
    oldPosition = State().b2w.pos;

    oldRotation = State().q;

    oldVelocity = Derive().linearVel;

    oldAngularVelocity = Derive().angularVel;

    oldCounter = 0;
}

void neRigidBody_::BecomeIdle()
{
    //return;

    status = neRigidBody_::NE_RBSTATUS_IDLE;
    
    ZeroMotion();
}

void neRigidBody_::ZeroMotion()
{
    Derive().angularVel.SetZero();
    Derive().linearVel.SetZero();
    Derive().qDot.Zero();
    Derive().speed = 0.0f;
    State().angularMom.SetZero();

    for (s32 i = 0; i < NE_RB_MAX_PAST_RECORDS; i++)
    {
        GetVelRecord(i).SetZero();
        GetAngVelRecord(i).SetZero();
        dvRecord[i].SetZero();
    }
}

/****************************************************************************
*
*   neRigidBody_::ApplyCollisionImpulse
*
****************************************************************************/ 

neBool neRigidBody_::ApplyCollisionImpulse(const neV3 & impulse, const neV3 & contactPoint, neImpulseType itype)
{
    neV3 dv, da, newAM;

    dv = impulse * oneOnMass;

    da = contactPoint.Cross(impulse);

    neBool immediate = true;

    if (itype == IMPULSE_CONTACT)// && sim->solverStage == 1) // only if contact and resolve penetration stage
    {
        immediate = false;
    }
    
/*  if (itype != IMPULSE_NORMAL && sim->solverStage != 2)
    {
        immediate = false;
    }
*/  

    immediate = true;

    if (immediate)
    {
        Derive().linearVel += dv;

        dvRecord[sim->stepSoFar % NE_RB_MAX_PAST_RECORDS] += dv;

        neV3 dav = Derive().Iinv * da;

        davRecord[sim->stepSoFar % NE_RB_MAX_PAST_RECORDS] += dav;

        newAM = State().angularMom + da;

        SetAngMom(newAM);

        f32 l = dv.Length();
/*
        if (id == 1 && l > sim->magicNumber)
        {
            sim->magicNumber = l;
        }
*/
        ASSERT(newAM.IsFinite());
    }
    else
    {
        totalDV += dv;
        totalDA += da;
        impulseCount++;
        twistCount++;
    }
    return true;
}

void neRigidBody_::AddRestContact(neRestRecord & rc)
{
    //search existing matching records

    s32 oldest = -1;

    s32 freeOne = -1;

    f32 shallowest = 0.0f;

    s32 i;

    for (i = 0; i < NE_RB_MAX_RESTON_RECORDS; i++)
    {
        if (!GetRestRecord(i).IsValid())
        {
            freeOne = i;

            continue;
        }
        if (shallowest == 0.0f)
        {
            shallowest = GetRestRecord(i).depth;
            oldest = i;
        }
        else if (GetRestRecord(i).depth < shallowest)
        {
            shallowest = GetRestRecord(i).depth;
            oldest = i;
        }
        neV3 diff = rc.bodyPoint - GetRestRecord(i).bodyPoint;

        f32 dot = diff.Dot(diff);

        if (dot < 0.0025f) //difference of 0.05 meters, or 5 cm
        {
            //found
            break;
        }
    }
/*  if (i < NE_RB_MAX_RESTON_RECORDS)
    {
        GetRestRecord(i).SetInvalid();
    }
    else
    {
        if (freeOne != -1)
        {
            i = freeOne;
        }
        else
        {
            //use the olderest one
            ASSERT(oldest != -1);
            
            i = oldest;
        }
    }
*/
    if (i == NE_RB_MAX_RESTON_RECORDS)
    {   //not found

        //find a free one
        if (freeOne != -1)
        {
            i = freeOne;
        }
        else
        {
            //use the olderest one
            ASSERT(oldest != -1);
            
            i = oldest;

            GetRestRecord(i).SetInvalid();
        }
    }
    else
    {
        GetRestRecord(i).SetInvalid();
    }
    if (i >= NE_RB_MAX_RESTON_RECORDS || i < 0)
        return;

    GetRestRecord(i).Set(this, rc);
}

neBool neRigidBody_::IsConstraintNeighbour(neRigidBodyBase * otherBody)
{
    neConstraintHandle * chandle = constraintCollection.GetHead();

    while (chandle)
    {
        _neConstraint * c = chandle->thing;

        chandle = constraintCollection.GetNext(chandle);

        neConstraintHandle * chandleB = otherBody->constraintCollection.GetHead();

        while (chandleB)
        {
            _neConstraint * cB = chandleB->thing;

            if (c == cB)
                return true;

            chandleB = otherBody->constraintCollection.GetNext(chandleB);
        }
    };
    return false;
}

void neRigidBody_::SetAngMomComponent(const neV3 & angMom, const neV3 & dir)
{
    neV3 newMom = State().angularMom;

    newMom.RemoveComponent(dir);

    newMom += angMom;

    SetAngMom(newMom);

    needRecalc = true;
}

void neRigidBody_::WakeUpAllJoint()
{
    if (!GetConstraintHeader())
    {
        WakeUp();
        return;
    }
    neBodyHandle * bodyHandle = GetConstraintHeader()->bodies.GetHead();
    
    while (bodyHandle)
    {
        if (!bodyHandle->thing->AsRigidBody())
        {
            bodyHandle = bodyHandle->next;

            continue;
        }

        bodyHandle->thing->AsRigidBody()->WakeUp();

        bodyHandle = bodyHandle->next;
    }
}

void neRigidBody_::ApplyLinearConstraint()
{
    neV3 dv = totalDV / (f32)impulseCount;

//  cacheImpulse = totalDV;

    Derive().linearVel += dv;

    totalDV.SetZero();
    
    //dvRecord[sim->stepSoFar % NE_RB_MAX_PAST_RECORDS] += dv;

    impulseCount = 0;
}

void neRigidBody_::ApplyAngularConstraint()
{
    neV3 da = totalDA / (f32)twistCount;

//  cacheTwist += da;

    neV3 newAM = State().angularMom + da;

    SetAngMom(newAM);

    totalDA.SetZero();

    twistCount = 0;
}
/*
void neRigidBody_::ConstraintDoSleepCheck()
{
    f32 len = dvRecord[sim->stepSoFar % NE_RB_MAX_PAST_RECORDS].Length();


    if (len > 0.3f)
    {
        //WakeUpAllJoint();
        //WakeUp();
    }
    else 
    {
        len = davRecord[sim->stepSoFar % NE_RB_MAX_PAST_RECORDS].Length();

        if (len > 0.5f)
        {
            //WakeUp();//AllJoint();
        }
        else
        {
            BecomeIdle();
        }
    }
}
*/

neBool neRestRecord::CanConsiderOtherBodyIdle()
{
    if (rtype == REST_ON_WORLD)
        return true;

    ASSERT(otherBody != NULL);

    neRigidBody_ * rb = otherBody->AsRigidBody();

    if (rb != NULL)
    {
        return (rb->status == neRigidBody_::NE_RBSTATUS_IDLE);
    }
    neCollisionBody_ * cb = otherBody->AsCollisionBody();

    ASSERT(cb);

    return (!cb->moved);
}

neBool neRestRecord::CheckOtherBody(neFixedTimeStepSimulator * sim)
{
    ASSERT(otherBody);

    if (otherBody != sim->GetTerrainBody() && 
        (!otherBody->IsValid() || !otherBody->isActive))
    {
        return false;
    }
    return true;
}

neV3 neRestRecord::GetOtherBodyPoint()
{
    ASSERT(otherBody);

    neCollisionBody_ * cb = otherBody->AsCollisionBody();

    neV3 ret;

    if (cb)
    {
        normalWorld = cb->b2w.rot * normalBody;

        ret = cb->b2w * otherBodyPoint;
    }
    else
    {
        normalWorld = ((neRigidBody_*)otherBody)->State().b2w.rot * normalBody;

        ret = ((neRigidBody_*)otherBody)->State().b2w * otherBodyPoint;
    }
    return ret;
}

void neRestRecord::Set(neRigidBody_* thisBody, const neRestRecord & rc)
{
    bodyPoint = rc.bodyPoint;
    otherBodyPoint = rc.otherBodyPoint;
    otherBody = rc.otherBody;
    counter = thisBody->sim->stepSoFar;
    normalBody = rc.normalBody;
    depth = rc.depth;
    body = thisBody;
    material = rc.material;
    otherMaterial = rc.otherMaterial;

    if (rc.otherBody->AsCollisionBody())
        rtype = neRestRecord::REST_ON_COLLISION_BODY;
    else
        rtype = neRestRecord::REST_ON_RIGID_BODY;

    otherBody->rbRestingOnMe.Add(&restOnHandle);
}

void neRestRecord::SetTmp(neRigidBodyBase * _otherBody, const neV3 & contactA, const neV3 & contactB, const neV3 & _normalBody, s32 matA, s32 matB)
{
    otherBody = _otherBody;
    bodyPoint = contactA;
    otherBodyPoint = contactB;
    material = matA;
    otherMaterial = matB;
    normalBody = _normalBody;
}

void neRestRecord::SetInvalid()
{
    //ASSERT(rtype != REST_ON_NOT_VALID);

    rtype = REST_ON_NOT_VALID;

    counter = 0;

//  body = NULL;

    if (otherBody)
    {
        otherBody->rbRestingOnMe.Remove(&restOnHandle);
    }
    otherBody = NULL;
}


/**scenery.cpp**/

//#include "math/ne_type.h"
//#include "math/ne_debug.h"
//#include "tokamak.h"
//#include "containers.h"
//#include "scenery.h"
//#include "collision.h"
//#include "constraint.h"
//#include "rigidbody.h"
//#include "scenery.h"
/*
#ifdef _WIN32
#include <windows.h>
#endif
*/
//#include "stack.h"
//#include "simulator.h"

/********************************************************/
/* AABB-triangle overlap test code                      */
/* by Tomas Akenine-Möller                              */
/* Function: int triBoxOverlap(float boxcenter[3],      */
/*          float boxhalfsize[3],float triverts[3][3]); */
/* History:                                             */
/*   2001-03-05: released the code in its first version */
/*   2001-06-18: changed the order of the tests, faster */
/*                                                      */
/* Acknowledgement: Many thanks to Pierre Terdiman for  */
/* suggestions and discussions on how to optimize code. */
/* Thanks to David Hunt for finding a ">="-bug!         */
/********************************************************/
#include <math.h>
#include <stdio.h>

enum { X = 0 };
enum { Y = 1 };
enum { Z = 2 };

#define CROSS(dest,v1,v2) \
          dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
          dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
          dest[2]=v1[0]*v2[1]-v1[1]*v2[0];

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define SUB(dest,v1,v2) \
          dest[0]=v1[0]-v2[0]; \
          dest[1]=v1[1]-v2[1]; \
          dest[2]=v1[2]-v2[2];

#define FINDMINMAX(x0,x1,x2,min,max) \
  min = max = x0;   \
  if(x1<min) min=x1;\
  if(x1>max) max=x1;\
  if(x2<min) min=x2;\
  if(x2>max) max=x2;

int planeBoxOverlap(float normal[3],float d, float maxbox[3])
{
  int q;
  float vmin[3],vmax[3];
  for(q=X;q<=Z;q++)
  {
    if(normal[q]>0.0f)
    {
      vmin[q]=-maxbox[q];
      vmax[q]=maxbox[q];
    }
    else
    {
      vmin[q]=maxbox[q];
      vmax[q]=-maxbox[q];
    }
  }
  if(DOT(normal,vmin)+d>0.0f) return 0;
  if(DOT(normal,vmax)+d>=0.0f) return 1;

  return 0;
}


/*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb)             \
    p0 = a*v0[Y] - b*v0[Z];                    \
    p2 = a*v2[Y] - b*v2[Z];                    \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
    rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
    if(min>rad || max<-rad) return 0;

#define AXISTEST_X2(a, b, fa, fb)              \
    p0 = a*v0[Y] - b*v0[Z];                    \
    p1 = a*v1[Y] - b*v1[Z];                    \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
    if(min>rad || max<-rad) return 0;

/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)             \
    p0 = -a*v0[X] + b*v0[Z];                   \
    p2 = -a*v2[X] + b*v2[Z];                       \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
    if(min>rad || max<-rad) return 0;

#define AXISTEST_Y1(a, b, fa, fb)              \
    p0 = -a*v0[X] + b*v0[Z];                   \
    p1 = -a*v1[X] + b*v1[Z];                       \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
    if(min>rad || max<-rad) return 0;

/*======================== Z-tests ========================*/

#define AXISTEST_Z12(a, b, fa, fb)             \
    p1 = a*v1[X] - b*v1[Y];                    \
    p2 = a*v2[X] - b*v2[Y];                    \
        if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
    if(min>rad || max<-rad) return 0;

#define AXISTEST_Z0(a, b, fa, fb)              \
    p0 = a*v0[X] - b*v0[Y];                \
    p1 = a*v1[X] - b*v1[Y];                    \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
    if(min>rad || max<-rad) return 0;

int _triBoxOverlap_(float boxcenter[3],float boxhalfsize[3],float triverts[3][3])
{

  /*    use separating axis theorem to test overlap between triangle and box */
  /*    need to test for overlap in these directions: */
  /*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
  /*       we do not even need to test these) */
  /*    2) normal of the triangle */
  /*    3) crossproduct(edge from tri, {x,y,z}-directin) */
  /*       this gives 3x3=9 more tests */
   float v0[3],v1[3],v2[3];
   float min,max,d,p0,p1,p2,rad,fex,fey,fez;
   float normal[3],e0[3],e1[3],e2[3];

   /* This is the fastest branch on Sun */
   /* move everything so that the boxcenter is in (0,0,0) */
   SUB(v0,triverts[0],boxcenter);
   SUB(v1,triverts[1],boxcenter);
   SUB(v2,triverts[2],boxcenter);

   /* compute triangle edges */
   SUB(e0,v1,v0);      /* tri edge 0 */
   SUB(e1,v2,v1);      /* tri edge 1 */
   SUB(e2,v0,v2);      /* tri edge 2 */

   /* Bullet 3:  */
   /*  test the 9 tests first (this was faster) */
   fex = fabs(e0[X]);
   fey = fabs(e0[Y]);
   fez = fabs(e0[Z]);
   AXISTEST_X01(e0[Z], e0[Y], fez, fey);
   AXISTEST_Y02(e0[Z], e0[X], fez, fex);
   AXISTEST_Z12(e0[Y], e0[X], fey, fex);

   fex = fabs(e1[X]);
   fey = fabs(e1[Y]);
   fez = fabs(e1[Z]);
   AXISTEST_X01(e1[Z], e1[Y], fez, fey);
   AXISTEST_Y02(e1[Z], e1[X], fez, fex);
   AXISTEST_Z0(e1[Y], e1[X], fey, fex);

   fex = fabs(e2[X]);
   fey = fabs(e2[Y]);
   fez = fabs(e2[Z]);
   AXISTEST_X2(e2[Z], e2[Y], fez, fey);
   AXISTEST_Y1(e2[Z], e2[X], fez, fex);
   AXISTEST_Z12(e2[Y], e2[X], fey, fex);

   /* Bullet 1: */
   /*  first test overlap in the {x,y,z}-directions */
   /*  find min, max of the triangle each direction, and test for overlap in */
   /*  that direction -- this is equivalent to testing a minimal AABB around */
   /*  the triangle against the AABB */

   /* test in X-direction */
   FINDMINMAX(v0[X],v1[X],v2[X],min,max);
   if(min>boxhalfsize[X] || max<-boxhalfsize[X]) return 0;

   /* test in Y-direction */
   FINDMINMAX(v0[Y],v1[Y],v2[Y],min,max);
   if(min>boxhalfsize[Y] || max<-boxhalfsize[Y]) return 0;

   /* test in Z-direction */
   FINDMINMAX(v0[Z],v1[Z],v2[Z],min,max);
   if(min>boxhalfsize[Z] || max<-boxhalfsize[Z]) return 0;

   /* Bullet 2: */
   /*  test if the box intersects the plane of the triangle */
   /*  compute plane equation of triangle: normal*x+d=0 */
   CROSS(normal,e0,e1);
   d=-DOT(normal,v0);  /* plane eq: normal.x+d=0 */
   if(!planeBoxOverlap(normal,d,boxhalfsize)) return 0;

   return 1;   /* box and triangle overlaps */
}


//extern void DrawLine(const neV3 & colour, neV3 * startpoint, s32 count);

s32 neTreeNode::numOfChildren = 4;//NE_TREE_DIM * NE_TREE_DIM;

void FindCenterOfMass(neTriangleTree * tree, neSimpleArray<s32>& triIndex, neV3 * com)
{
    s32 i;
    
    neV3 ret;

    ret.SetZero();
    
    for(i = 0; i < triIndex.GetUsedCount(); i++)
    {
        neTriangle_ & t = tree->triangles[triIndex[i]];

        for (s32 j = 0; j < 3; j++)
        {
            ret += tree->vertices[t.indices[j]];
        }
    }

    f32 div = (f32)triIndex.GetUsedCount() * 3;

    ret /= div;

    *com = ret;
    //_mm_store_ps(&com->v[0], ret.m);
}

void FindMinMaxBound(neTriangleTree * tree, neSimpleArray<s32>& triIndex, neV3 & minBound, neV3 & maxBound)
{
    s32 i;
    
    minBound.Set(1.0e6f, 1.0e6f, 1.0e6f);

    maxBound.Set(-1.0e6f, -1.0e6f, -1.0e6f);

    for(i = 0; i < triIndex.GetUsedCount(); i++)
    {
        neTriangle_ & t = tree->triangles[triIndex[i]];

        for (s32 j = 0; j < 3; j++)
        {
            minBound.SetMin(minBound, tree->vertices[t.indices[j]]);

            maxBound.SetMax(maxBound, tree->vertices[t.indices[j]]);
        }
    }
}

neBool IntersectAABBTriangle(neTriangleTree * tree, const neV3 & minBound, const neV3 & maxBound, const neTriangle & triangle)
{
    neCollision box;
    neCollision tri;
//  neT3 boxt3;
    neT3 trit3;

    trit3.SetIdentity();
    tri.obb.SetTriangle(triangle.indices[0], triangle.indices[1], triangle.indices[2], tree->vertices);
    tri.obb.SetTransform(trit3);
    tri.convexCount = 1;
    tri.convex = &tri.obb;
/*
    if (triangle.indices[0] == 1312 &&
        triangle.indices[1] == 1363 &&
        triangle.indices[2] == 1364)
    {
        ASSERT(0);
    }
*/

    neV3 edge0 = tree->vertices[triangle.indices[1]] - tree->vertices[triangle.indices[0]];
    neV3 edge1 = tree->vertices[triangle.indices[2]] - tree->vertices[triangle.indices[1]];

    neV3 normal = edge0.Cross(edge1);

    normal.Normalize();

    f32 xfactor = 0.0f;
    f32 yfactor = 0.0f;
    f32 zfactor = 0.0f;

    if (neIsConsiderZero(neAbs(normal[0]) - 1.0f))
    {
        xfactor = 0.01f;
    }
    else if (neIsConsiderZero(neAbs(normal[1]) - 1.0f))
    {
        yfactor = 0.01f;
    }
    else if (neIsConsiderZero(neAbs(normal[2]) - 1.0f))
    {
        zfactor = 0.01f;
    }

    float boxpos[3];
    float boxhalfsize[3];
    float triverts[3][3];
    int i;

    boxhalfsize[0] = ((maxBound[0] - minBound[0]) * 0.5f) + xfactor;
    boxhalfsize[1] = ((maxBound[1] - minBound[1]) * 0.5f) + yfactor;
    boxhalfsize[2] = ((maxBound[2] - minBound[2]) * 0.5f) + zfactor;

    for (i = 0; i < 3; i++)
        boxpos[i] = (maxBound[i] + minBound[i]) * 0.5f;

    for (i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            triverts[i][j] = tree->vertices[triangle.indices[i]][j];

    int ret = _triBoxOverlap_(boxpos, boxhalfsize, triverts);

    return ret;

/*
    boxt3.SetIdentity();
    box.obb.SetBoxSize((maxBound[0] - minBound[0]) + xfactor, 
                        (maxBound[1] - minBound[1]) + yfactor, 
                        (maxBound[2] - minBound[2]) + zfactor);
    box.obb.SetTransform(boxt3);
    box.convexCount = 1;
    box.convex = &box.obb;

    neCollisionResult res;

    boxt3.pos = ( maxBound + minBound ) * 0.5f;

    CollisionTest(res, box, boxt3, tri, trit3);

    return res.penetrate;
*/
}

/****************************************************************************
*
*   neTreeNode::neTreeNode
*
****************************************************************************/ 

neTreeNode::neTreeNode()
{
    //triangleIndices = NULL; //leaf only
    neV3 minBound, maxBound;

    minBound.SetZero();

    maxBound.SetZero();

    Initialise(NULL, 0, minBound, maxBound);
}

void neTreeNode::Initialise(neTriangleTree * _tree, s32 _parent, const neV3 & minBound, const neV3 & maxBound)
{
    s32 i;

    for (i = 0; i < 3; i++)
    {
        bounds[i][0] = minBound[i];
        bounds[i][1] = maxBound[i];
    }

    tree = _tree;

    parent = _parent;

    for (i = 0; i < numOfChildren; i++)
        children[i] = -1;
}

/****************************************************************************
*
*   neTreeNode::Build
*
****************************************************************************/ 

void neTreeNode::SelectBound(const neV3 & com, neV3 & minBound, neV3 & maxBound, s32 sector)
{
    switch (sector)
    {
    case 0:
        minBound.Set(bounds[0][0], bounds[1][0], bounds[2][0]);
        maxBound.Set(com[0], bounds[1][1], com[2]);
        break;
    case 1:
        minBound.Set(com[0], bounds[1][0], bounds[2][0]);
        maxBound.Set(bounds[0][1], bounds[1][1], com[2]);
        break;
    case 2:
        minBound.Set(com[0], bounds[1][0], com[2]);
        maxBound.Set(bounds[0][1], bounds[1][1], bounds[2][1]);
        break;
    case 3:
        minBound.Set(bounds[0][0], bounds[1][0], com[2]);
        maxBound.Set(com[0], bounds[1][1], bounds[2][1]);
        break;
    }
}

void neTreeNode::CountTriangleInSector(neSimpleArray<s32> &tris, neSimpleArray<s32> &sectorTris, const neV3 & com, s32 sector)
{
    neV3 maxBound;

    neV3 minBound;

    SelectBound(com, minBound, maxBound, sector);

    neSimpleArray<s32> triSet;

    triSet.Reserve(2000 , tree->alloc, 2000);

    s32 i;

    for (i = 0; i < tris.GetUsedCount(); i++)
    {
        neTriangle * triangle = &tree->triangles[tris[i]];

        ASSERT(triangle);

        // check triangle collide with this AABB
        if (IntersectAABBTriangle(tree, minBound, maxBound, *triangle))
        {
            s32 * n = triSet.Alloc();

            ASSERT(n);

            *n = tris[i];
        }
    }
    if (triSet.GetUsedCount() > 0)
    {
        sectorTris.Reserve(triSet.GetUsedCount(), tree->alloc);
        
        for (i = 0; i < triSet.GetUsedCount(); i++)
        {
            s32 * j = sectorTris.Alloc();
            ASSERT(j);
            *j = triSet[i];
        }
    }
}

s32 neTreeNode::CountTriangleInSector2(neSimpleArray<s32> &tris, const neV3 & com, s32 sector)
{
    s32 ret = 0;

    neV3 maxBound;

    neV3 minBound;

    SelectBound(com, minBound, maxBound, sector);

    neSimpleArray<s32> triSet;

    s32 i;

    for (i = 0; i < tris.GetUsedCount(); i++)
    {
        neTriangle * triangle = &tree->triangles[tris[i]];

        ASSERT(triangle);

        // check triangle collide with this AABB
        if (IntersectAABBTriangle(tree, minBound, maxBound, *triangle))
        {
            ret++;
        }
    }
    return ret;
}

#pragma optimize( "", off )

void neTreeNode::Build(neSimpleArray<s32> & triIndex, s32 level)
{
    neV3 maxBound, minBound, com;

    FindCenterOfMass(tree, triIndex, &com);

    s32 i;

    if (level > 10)
    {
        MakeLeaf(triIndex);

        return;
    }
    if (triIndex.GetUsedCount() < 4)
    {
        MakeLeaf(triIndex);

        return;
    }

    for (i = 0; i < 4; i++)
    {
        neSimpleArray<s32> sectorTris;

        CountTriangleInSector(triIndex, sectorTris, com, i);

        if (sectorTris.GetUsedCount())
        {
            if (sectorTris.GetUsedCount() == triIndex.GetUsedCount())
            {
                MakeLeaf(triIndex); 

                return;
            }
            else
            {
                FindMinMaxBound(tree, sectorTris, minBound, maxBound);

                f32 yMin = minBound[1];

                f32 yMax = maxBound[1];

                neTreeNode * node = tree->nodes.Alloc();

                ASSERT(node);

                SelectBound(com, minBound, maxBound, i);

                minBound[1] = yMin;

                maxBound[1] = yMax;

                if (this == &tree->root)
                    node->Initialise(tree, -1, minBound, maxBound);
                else
                    node->Initialise(tree, tree->nodes.GetIndex(this), minBound, maxBound);

                children[i] = tree->nodes.GetIndex(node);

                //ASSERT(children[i] != 159);

                tree->nodes[children[i]].Build(sectorTris, level + 1);
            }
        }
        else
        {
            children[i] = -1;
        }
    }
}
#pragma optimize( "", on )
void neTreeNode::MakeLeaf(neSimpleArray<s32> &tris)
{
    triangleIndices.Reserve(tris.GetUsedCount(), tree->alloc);

    s32 i;

    for (i = 0; i < tris.GetUsedCount(); i++)
    {
        s32 * n = triangleIndices.Alloc();

        ASSERT(n);

        *n = tris[i];
    }

    for (i = 0; i < 4; i++)
    {
        children[i] = -1;
    }
}

bool neTreeNode::IsOverlapped(const neV3 & minBound, const neV3 & maxBound)
{
    const s32 _min = 0;
    const s32 _max = 1;

    if (minBound[1] > bounds[1][_max])
        return false;

    if (maxBound[1] < bounds[1][_min])
        return false;

    if (minBound[0] > bounds[0][_max])
        return false;

    if (maxBound[0] < bounds[0][_min])
        return false;

    if (minBound[2] > bounds[2][_max])
        return false;

    if (maxBound[2] < bounds[2][_min])
        return false;

    return true;
}

void neTreeNode::DrawTriangles()
{
#if 0
    s32 i;

    neV3 red;

    red.Set(1.0f, 0.2f, 0.2f);

    for (i = 0; i < triangleIndices.GetUsedCount(); i++)
    {
        neTriangle_ * tri = &tree->triangles[triangleIndices[i]];

        neV3 points[4];

        points[0] = tree->vertices[tri->indices[0]];
        points[1] = tree->vertices[tri->indices[1]];
        points[2] = tree->vertices[tri->indices[2]];
        points[3] = tree->vertices[tri->indices[0]];

        DrawLine(red, points, 4);
    }
#endif
}

void neTreeNode::DrawBounds()
{
#if 0
    neV3 points[5];

    neV3 white;

    white.Set(1.0f, 1.0f, 1.0f);

    points[0].Set(bounds[0][0], bounds[1][0], bounds[2][0]);
    points[1].Set(bounds[0][0], bounds[1][0], bounds[2][1]);
    points[2].Set(bounds[0][1], bounds[1][0], bounds[2][1]);
    points[3].Set(bounds[0][1], bounds[1][0], bounds[2][0]);
    points[4].Set(bounds[0][0], bounds[1][0], bounds[2][0]);
    DrawLine(white, points, 5);
    
    points[0].Set(bounds[0][0], bounds[1][1], bounds[2][0]);
    points[1].Set(bounds[0][0], bounds[1][1], bounds[2][1]);
    points[2].Set(bounds[0][1], bounds[1][1], bounds[2][1]);
    points[3].Set(bounds[0][1], bounds[1][1], bounds[2][0]);
    points[4].Set(bounds[0][0], bounds[1][1], bounds[2][0]);
    DrawLine(white, points, 5);
    
    
    points[0].Set(bounds[0][0], bounds[1][0], bounds[2][0]);
    points[1].Set(bounds[0][0], bounds[1][1], bounds[2][0]);
    DrawLine(white, points, 2);
    points[0].Set(bounds[0][0], bounds[1][0], bounds[2][1]);
    points[1].Set(bounds[0][0], bounds[1][1], bounds[2][1]);
    DrawLine(white, points, 2);
    points[0].Set(bounds[0][1], bounds[1][0], bounds[2][0]);
    points[1].Set(bounds[0][1], bounds[1][1], bounds[2][0]);
    DrawLine(white, points, 2);
    points[0].Set(bounds[0][1], bounds[1][0], bounds[2][1]);
    points[1].Set(bounds[0][1], bounds[1][1], bounds[2][1]);
    DrawLine(white, points, 2);
#endif
}

void neTreeNode::GetCandidateNodes(neSimpleArray<neTreeNode*> & nodes, const neV3 & minBound, const neV3 & maxBound, s32 level)
{
    if (!IsOverlapped(minBound, maxBound))
    
        return;
/*  
    if (level >= 3)
    {
        DrawBounds();
        //DrawTriangles();
    }
*/  if (this->triangleIndices.GetUsedCount() > 0)
    {
        neTreeNode ** n = nodes.Alloc();

        ASSERT(n);

        *n = this;
/*
        if (level >= 4)
        {
            DrawBounds();
            DrawTriangles();
        }
*/
        return;
    }

    s32 i;

    for (i = 0; i < 4; i++)
    {
        if (children[i] != -1)
        {
            neTreeNode * c = &tree->nodes[children[i]];

            c->GetCandidateNodes(nodes, minBound, maxBound, level + 1);
        }
    }
}

/****************************************************************************
*
*   neTriangleTree::neTriangleTree()
*
****************************************************************************/ 

neTriangleTree::neTriangleTree()
{
    alloc = NULL;

    vertexCount = 0;

    vertices = NULL;

    sim = NULL;
}
#pragma optimize( "", off )
void TreeBuild(neTriangleTree * tree, s32 nodeIndex, neSimpleArray<s32> & triIndex, s32 level)
{
//  if (nodeIndex == 769)
//      ASSERT(0);

    neV3 maxBound, minBound;

    neV3 com;
    
    FindCenterOfMass(tree, triIndex, &com);

    s32 i;

    if (level > 4)
    {
        tree->GetNode(nodeIndex).MakeLeaf(triIndex);

        return;
    }
    if (triIndex.GetUsedCount() < 10)
    {
        tree->GetNode(nodeIndex).MakeLeaf(triIndex);

        return;
    }

    for (i = 0; i < 4; i++)
    {
        neSimpleArray<s32> sectorTris;

        tree->GetNode(nodeIndex).CountTriangleInSector(triIndex, sectorTris, com, i);

        if (sectorTris.GetUsedCount())
        {
            if (sectorTris.GetUsedCount() == triIndex.GetUsedCount())
            {
                tree->GetNode(nodeIndex).MakeLeaf(triIndex);

                return;
            }
            else
            {
                FindMinMaxBound(tree, sectorTris, minBound, maxBound);

                f32 yMin = minBound[1];

                f32 yMax = maxBound[1];

                neTreeNode * node = tree->nodes.Alloc();

                ASSERT(node);

                tree->GetNode(nodeIndex).SelectBound(com, minBound, maxBound, i);

                minBound[1] = yMin;

                maxBound[1] = yMax;

                node->Initialise(tree, nodeIndex, minBound, maxBound);

                tree->GetNode(nodeIndex).children[i] = tree->nodes.GetIndex(node);
            
                TreeBuild(tree, tree->GetNode(nodeIndex).children[i], sectorTris, level + 1);
            }
        }
        else
        {
            tree->GetNode(nodeIndex).children[i] = -1;
        }
    }
}
#pragma optimize( "", on )
neTreeNode & neTriangleTree::GetNode(s32 nodeIndex)
{
    if (nodeIndex == -1)
        return root;
    else
    {
        ASSERT(nodeIndex >= 0 && nodeIndex < nodes.GetUsedCount());
        
        return nodes[nodeIndex];
    }
}

/****************************************************************************
*
*   neTriangleTree::BuildTree
*
****************************************************************************/ 

neBool neTriangleTree::BuildTree(neV3 * _vertices, s32 _vertexCount, neTriangle * tris, s32 triCount, neAllocatorAbstract * _alloc)
{
    if (!_vertices || _vertexCount <= 0)
        return false;

    if (!tris || triCount <= 0)
        return false;

    if (_alloc)
        alloc = _alloc;
    else
        alloc = &allocDef;

    if (triangles.GetTotalSize() > 0)
    {
        FreeTree();
    }
    triangles.Reserve(triCount, alloc);

    vertices = (neV3*)alloc->Alloc(sizeof(neV3) * _vertexCount);

    vertexCount = _vertexCount;
    
    s32 i;

    for (i = 0; i < vertexCount; i++)
    {
        vertices[i] = _vertices[i];
    }


    for (i = 0; i < triCount; i++)
    {
        neTriangle * t = triangles.Alloc();

        ASSERT(t);

        *t = tris[i];
    }

    nodes.Reserve(sim->sizeInfo.terrainNodesStartCount, alloc, sim->sizeInfo.terrainNodesGrowByCount);

    neSimpleArray<s32> triIndex;

    triIndex.Reserve(triCount, alloc);

    for (i = 0; i < triCount; i++)
    {
        s32 * j = triIndex.Alloc();

        *j = i;
    }

    neV3 minBound, maxBound;

    FindMinMaxBound(this, triIndex, minBound, maxBound);

    root.Initialise(this, -2, minBound, maxBound);

    //root.Build(triIndex, 0);

    TreeBuild(this, -1, triIndex, 0);

    s32 nodeUsed = nodes.GetUsedCount();

    triIndex.Free();

    return true;
}

void neTriangleTree::FreeTree()
{
    triangles.Free();

    if (vertices)
    {
        alloc->Free((neByte*)vertices);

        vertices = NULL;
    }   

    nodes.Free();

    neV3 minBound, maxBound;
    minBound.SetZero();
    maxBound.SetZero();
    root.Initialise(NULL, 0, minBound, maxBound);

    vertexCount = 0;
}

/****************************************************************************
*
*   neTriangleTree::~neTriangleTree
*
****************************************************************************/ 

neTriangleTree::~neTriangleTree()
{
    FreeTree();
}


/**simulator.cpp**/

//#include "math/ne_type.h"
//#include "math/ne_debug.h"
//#include "tokamak.h"
//#include "containers.h"
//#include "scenery.h"
//#include "collision.h"
//#include "constraint.h"
//#include "rigidbody.h"
//#include "stack.h"
//#include "simulator.h"
//#include "scenery.h"
//#include "message.h"

//#include <assert.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif

char neFixedTimeStepSimulator::logBuffer[256];

//extern void DrawLine(const neV3 & colour, neV3 * startpoint, s32 count);

extern s32 currentMicroStep;

#define NE_HIGH_ENERGY 1.0f

void ChooseAxis(neV3 & x, neV3 & y, const neV3 & normal);

/****************************************************************************
*
*   neFixedTimeStepSimulator::neFixedTimeStepSimulator(
*
****************************************************************************/ 

neFixedTimeStepSimulator::neFixedTimeStepSimulator(const neSimulatorSizeInfo & _sizeInfo, neAllocatorAbstract * alloc, const neV3 * grav)
{
    sizeInfo = _sizeInfo;
    
    if (alloc)
    {
        allocator = alloc;
    }
    else
    {
        allocator = &allocDef;
    }
    neV3 g;

    if (grav)
        g = *grav;
    else
        g.SetZero();

    neFixedTimeStepSimulator::Initialise(g);

    for (int i=0; i < MAX_MATERIAL; i++)
    {
        materials[i].density = 1.0f;
        materials[i].friction = .5f;
        materials[i].resititution = 0.4f;
    }
    perfReport = NULL;

    treeNodes.Reserve(100, allocator, 100);

    triangleIndex.Reserve(200, allocator, 200);

    constraintHeap.Reserve(sizeInfo.constraintsCount , allocator);

    constraintHeaders.Reserve(sizeInfo.constraintSetsCount , allocator);

    //miniConstraintHeap.Reserve(sizeInfo.constraintBufferSize, allocator);

    stackInfoHeap.Reserve(sizeInfo.rigidBodiesCount + sizeInfo.rigidParticleCount , allocator);

    stackHeaderHeap.Reserve(sizeInfo.rigidBodiesCount  + sizeInfo.rigidParticleCount + 100, allocator);

    controllerHeap.Reserve(sizeInfo.controllersCount, allocator);

    sensorHeap.Reserve(sizeInfo.sensorsCount, allocator);

    geometryHeap.Reserve(sizeInfo.geometriesCount, allocator);

    pointerBuffer1.Reserve(1000, allocator, 100);

    pointerBuffer2.Reserve(1000, allocator, 100);

    cresultHeap.Reserve(100, allocator, 100);

    cresultHeap2.Reserve(100, allocator, 100);

    //fastImpulseHeap.Reserve(500, allocator);

    logCallback = NULL;

    collisionCallback = NULL;

    breakageCallback = NULL;

    terrainQueryCallback = NULL;

    customCDRB2RBCallback = NULL;

    customCDRB2ABCallback = NULL;

    logLevel = neSimulator::LOG_OUTPUT_LEVEL_NONE;

//  solver.sim = this;

    fakeCollisionBody.moved = false;

    fakeCollisionBody.sim = this;

    fakeCollisionBody.id = -1;

    fakeCollisionBody.cookies = 0;

    fakeCollisionBody.isActive = true;

    highEnergy = NE_HIGH_ENERGY;

#ifdef _WIN32
    perf = nePerformanceData::Create();

    perf->Init();
#else
    perf = NULL;
#endif

    timeFromLastFrame = 0.0f;

    lastTimeStep = 0;
}

/****************************************************************************
*
*   neFixedTimeStepSimulator::Initialise
*
****************************************************************************/ 
void neFixedTimeStepSimulator::SetGravity(const neV3 & g)
{
    gravity = g;

    gravityVector = gravity;

    gravityVector.Normalize();

    restingSpeed = sqrtf(gravity.Dot(gravity));

    //restingSpeed = sqrtf(restingSpeed * 2.0e-2f) * 4.0f;

    restingSpeed = restingSpeed * 0.3f;

    gravityMag = g.Length();

    if (!neIsFinite(gravityMag))
    {
        gravityMag = 0.0f;
    }
}

void neFixedTimeStepSimulator::Initialise(const neV3& _gravity)
{
    buildCoordList = true;

    SetGravity(_gravity);

    maxRigidBodies = sizeInfo.rigidBodiesCount;

    maxAnimBodies = sizeInfo.animatedBodiesCount;

    maxParticles = sizeInfo.rigidParticleCount;

    if (!rigidBodyHeap.Reserve(maxRigidBodies, allocator))
    {
        sprintf(logBuffer, MSG_MEMORY_ALLOC_FAILED);

        LogOutput(neSimulator::LOG_OUTPUT_LEVEL_ONE);

        return;
    }

    if (!collisionBodyHeap.Reserve(maxAnimBodies, allocator))
    {
        sprintf(logBuffer, MSG_MEMORY_ALLOC_FAILED);

        LogOutput(neSimulator::LOG_OUTPUT_LEVEL_ONE);

        return;
    }

    if (!rigidParticleHeap.Reserve(maxParticles, allocator))
    {
        sprintf(logBuffer, MSG_MEMORY_ALLOC_FAILED);

        LogOutput(neSimulator::LOG_OUTPUT_LEVEL_ONE);

        return;
    }

    region.Initialise(this, neRegion::SORT_DIMENSION_X | neRegion::SORT_DIMENSION_Y | neRegion::SORT_DIMENSION_Z);

    stepSoFar = 0;

    stackHeaderX.Null();

    stackHeaderX.isHeaderX = true;

    stackHeaderX.sim = this;

    fakeCollisionBody.b2w.SetIdentity();
}

/****************************************************************************
*
*   neFixedTimeStepSimulator::SetCollisionCallback
*
****************************************************************************/ 

neCollisionCallback * neFixedTimeStepSimulator::SetCollisionCallback(neCollisionCallback * fn)
{
    neCollisionCallback * ret = collisionCallback;

    collisionCallback = fn;

    return ret;
}

/****************************************************************************
*
*   neFixedTimeStepSimulator::SetLogOutputCallback
*
****************************************************************************/ 

neLogOutputCallback * neFixedTimeStepSimulator::SetLogOutputCallback(neLogOutputCallback * fn)
{
    neLogOutputCallback * ret = logCallback;

    logCallback = fn;

    return ret;
}

/****************************************************************************
*
*   neFixedTimeStepSimulator::SetLogOutputLevel
*
****************************************************************************/ 

void neFixedTimeStepSimulator::SetLogOutputLevel(neSimulator::LOG_OUTPUT_LEVEL lvl)
{
    logLevel = lvl;
}

/****************************************************************************
*
*   neFixedTimeStepSimulator::LogOutput
*
****************************************************************************/ 

void neFixedTimeStepSimulator::LogOutput(neSimulator::LOG_OUTPUT_LEVEL lvl)
{
    if (!logCallback)
        return;

    if (lvl <= logLevel)
        logCallback(logBuffer);
}

/****************************************************************************
*
*   neFixedTimeStepSimulator::SetMaterial
*
****************************************************************************/ 

bool neFixedTimeStepSimulator::SetMaterial(s32 index, f32 friction, f32 restitution, f32 density)
{
    if (index < 0)
        return false;

    if (index >= MAX_MATERIAL)
        return false;

    materials[index].density = density;
    materials[index].friction = friction;
    materials[index].resititution = restitution;

    return true;
}

/****************************************************************************
*
*   neFixedTimeStepSimulator::GetMaterial
*
****************************************************************************/ 

bool neFixedTimeStepSimulator::GetMaterial(s32 index, f32& friction, f32& restitution, f32& density)
{
    if (index < 0)
        return false;

    if (index >= MAX_MATERIAL)
        return false;

    density = materials[index].density;
    friction = materials[index].friction;
    restitution = materials[index].resititution;

    return true;    
}

/****************************************************************************
*
*   neFixedTimeStepSimulator::CreateRigidBody
*
****************************************************************************/ 

neRigidBody_* neFixedTimeStepSimulator::CreateRigidBody(neBool isParticle)
{
    neRigidBody_ * ret;

    if (!isParticle)
    {
        ret = rigidBodyHeap.Alloc(1);

        if (!ret)
        {
            sprintf(logBuffer, MSG_RUN_OUT_RIDIGBODY);

            LogOutput(neSimulator::LOG_OUTPUT_LEVEL_ONE);

            return NULL;
        }

        //ASSERT(ret);

        new (ret) neRigidBody_;

        activeRB.Add(ret);

        ret->id = rigidBodyHeap.GetID(ret);

        ret->subType = NE_RIGID_NORMAL;
    }
    else
    {
        ret = rigidParticleHeap.Alloc(1);

        if (!ret)
        {
            sprintf(logBuffer, MSG_RUN_OUT_RIDIGPARTICLE);

            LogOutput(neSimulator::LOG_OUTPUT_LEVEL_ONE);

            return NULL;
        }
        //ASSERT(ret);

        new (ret) neRigidBody_;

        activeRP.Add(ret);

        ret->id = rigidParticleHeap.GetID(ret) + rigidBodyHeap.Size() + collisionBodyHeap.Size();

        ret->subType = NE_RIGID_PARTICLE;
    }

    ret->col.convexCount = 0;

    ret->col.obb.Initialise();

    ret->sim = this;
    
    return ret;
}

neRigidBody_ * neFixedTimeStepSimulator::CreateRigidBodyFromConvex(TConvex * convex, neRigidBodyBase * originalBody)
{
    //make sure convex belong to this body and
    //this convex is not the only convex on this body

    originalBody->BeginIterateGeometry();

    TConvex * con;

    s32 ccount = 0;

    neBool found = false;

    while (con = originalBody->GetNextGeometry())
    {
        if (con == convex)
        {
            found = true;
        }
        ccount++;
    }

    if (ccount == 1 || !found)
    {
        return NULL;
    }
    neBool isParticle = false;

    if (convex->breakInfo.flag == neGeometry::NE_BREAK_ALL_PARTICLE ||
        convex->breakInfo.flag == neGeometry::NE_BREAK_NORMAL_PARTICLE ||
        convex->breakInfo.flag == neGeometry::NE_BREAK_NEIGHBOUR_PARTICLE)
    {
        isParticle = true;
    }

    neRigidBody_ * newBody = CreateRigidBody(false);

    if (!newBody)
    {
        return NULL;
    }

    newBody->mass = convex->breakInfo.mass;

    newBody->oneOnMass = 1.0f / convex->breakInfo.mass;

    newBody->Ibody.SetIdentity();

    for (s32 i = 0; i < 3; i++)
        newBody->Ibody[i][i] = convex->breakInfo.inertiaTensor[i];

    newBody->IbodyInv.SetInvert(newBody->Ibody);

    convex->breakInfo.flag = neGeometry::NE_BREAK_DISABLE;

    newBody->State().b2w = originalBody->GetB2W() * convex->c2p;

    newBody->State().q.SetupFromMatrix3(newBody->State().rot()); 

    originalBody->col.convexCount--;

    if (originalBody->col.convex == convex)
        originalBody->col.convex = (TConvex*)((TConvexItem *)convex)->next;

    if (originalBody->col.convexCount == 0 && originalBody->isActive)
    {
        region.RemoveBody(originalBody);
    }
    else if (originalBody->col.convexCount == 1)
    {
        originalBody->RecalcBB();
    }

    ((TConvexItem *)convex)->Remove();

    convex->c2p.SetIdentity();

    newBody->col.convex = convex;
    
    newBody->col.convexCount++;

    newBody->RecalcBB();

    region.AddBody(newBody, originalBody);

    return newBody;
}

/****************************************************************************
*
*   neFixedTimeStepSimulator::CreateAnimateBody
*
****************************************************************************/ 

neCollisionBody_* neFixedTimeStepSimulator::CreateCollisionBody()
{
    neCollisionBody_ * ret =  collisionBodyHeap.Alloc(1);

    //ASSERT(ret);

    new (ret) neCollisionBody_;

    activeCB.Add(ret);

    ret->id = collisionBodyHeap.GetID(ret) + rigidBodyHeap.Size();

    ret->col.convexCount = 0;

    ret->col.obb.Initialise();

    ret->sim = this;

    ret->b2w.SetIdentity();
    
    //region.AddBody(ret);

    return ret;
}

/****************************************************************************
*
*   neFixedTimeStepSimulator::Free
*
****************************************************************************/ 

void neFixedTimeStepSimulator::Free(neRigidBodyBase * bb)
{
    if (bb->AsCollisionBody())
    {
        neCollisionBody_* cb = reinterpret_cast<neCollisionBody_*>(bb);

        if (collisionBodyHeap.CheckBelongAndInUse(cb))
        {
            ((neCollisionBody_*)bb)->Free();

            if (bb->isActive)
                activeCB.Remove(cb);
            else
                inactiveCB.Remove(cb);

            collisionBodyHeap.Dealloc(cb, 1);
        }
        else
        {
            sprintf(logBuffer, MSG_TRYING_TO_FREE_INVALID_CB);

            LogOutput(neSimulator::LOG_OUTPUT_LEVEL_ONE);
        }
    }
    else
    {
        neRigidBody_* rb = reinterpret_cast<neRigidBody_*>(bb);

        rb->Free();

        if (rb->IsParticle())
        {
            if (rigidParticleHeap.CheckBelongAndInUse(rb))
            {
                if (rb->isActive)
                    activeRP.Remove(rb);
                else
                    inactiveRP.Remove(rb);

                rigidParticleHeap.Dealloc(rb, 1);
            }
            else
            {
                sprintf(logBuffer, MSG_TRYING_TO_FREE_INVALID_RP);

                LogOutput(neSimulator::LOG_OUTPUT_LEVEL_ONE);
            }
        }
        else
        {
            if (rigidBodyHeap.CheckBelongAndInUse(rb))
            {
                if (rb->isActive)
                    activeRB.Remove(rb);
                else
                    inactiveRB.Remove(rb);

                rigidBodyHeap.Dealloc(rb, 1);
            }
            else
            {
                sprintf(logBuffer, MSG_TRYING_TO_FREE_INVALID_RB);

                LogOutput(neSimulator::LOG_OUTPUT_LEVEL_ONE);
            }
        }
    }
}

/****************************************************************************
*
*   ~neFixedTimeStepSimulator::neFixedTimeStepSimulator
*
****************************************************************************/ 

neFixedTimeStepSimulator::~neFixedTimeStepSimulator()
{
    FreeAllBodies();

    if (perf)
        delete perf;
}

///////////////////////////////////////////////////////////////////

//#define DETAIL_PERF_REPORTING

#ifdef _WIN32
#define UPDATE_PERF_REPORT(n) {if (perfReport) perf->n();}
#else
#define UPDATE_PERF_REPORT(n)
#endif

void neFixedTimeStepSimulator::Advance(nePerformanceReport * _perfReport)
{
    ClearCollisionBodySensors();
        
    UpdateAABB();

    region.Update();

UPDATE_PERF_REPORT(UpdateCDCulling)

    CheckCollision();

UPDATE_PERF_REPORT(UpdateCD);

    CheckTerrainCollision();

UPDATE_PERF_REPORT(UpdateTerrain);

    ResetTotalForce();
        
    ApplyJointDamping();
    
    //Advance Rigid Body Dynamic
    AdvanceDynamicRigidBodies();
        
    //Advance Rigid Particle Dynamic
    AdvanceDynamicParticles();

UPDATE_PERF_REPORT(UpdateDynamic);

    ResetStackHeaderFlag();

    SolveAllConstrain();    

//UPDATE_PERF_REPORT(UpdateConstrain1);

    ResolvePenetration();

    SolveContactConstrain();

UPDATE_PERF_REPORT(UpdateConstrain2);

    //Advance Position

    AdvancePositionRigidBodies();

    AdvancePositionParticles();

UPDATE_PERF_REPORT(UpdatePosition);

    UpdateConstraintControllers();

UPDATE_PERF_REPORT(UpdateControllerCallback);
}

void neFixedTimeStepSimulator::Advance(f32 time, u32 nStep, nePerformanceReport * _perfReport)
{
    _currentTimeStep = time / (f32)nStep;

    oneOnCurrentTimeStep = 1.0f / _currentTimeStep;

    perfReport = _perfReport;

    currentRecord = stepSoFar % NE_RB_MAX_PAST_RECORDS;

#ifdef _WIN32
    if (perfReport)
    {
        for (s32 j = 0; j < nePerformanceReport::NE_PERF_LAST; j++)
        {
            perfReport->time[j] = 0.0f;
        }
        perf->Start();
    }
#endif

    int i;

    for (i = 0; i < (s32)nStep; i++)
    {
        magicNumber = 0;

        Advance(perfReport);
    }

    neCollisionBody_ * cb = activeCB.GetHead();

    while (cb)
    {
        cb->moved = false;

        cb = activeCB.GetNext(cb);
    }
    if (perfReport)
    {
        if (perfReport->reportType == nePerformanceReport::NE_PERF_SAMPLE)
        {
            f32 totalTime = perfReport->time[nePerformanceReport::NE_PERF_TOTAL_TIME] = perf->GetTotalTime();

#ifdef DETAIL_PERF_REPORTING

            perfReport->time[nePerformanceReport::NE_PERF_DYNAMIC] = perf->dynamic / totalTime * 100.0f;
            perfReport->time[nePerformanceReport::NE_PERF_POSITION] = perf->position / totalTime * 100.0f;
            perfReport->time[nePerformanceReport::NE_PERF_COLLISION_DETECTION] = perf->cd / totalTime * 100.0f;
            perfReport->time[nePerformanceReport::NE_PERF_COLLISION_CULLING] = perf->cdCulling / totalTime * 100.0f;
            perfReport->time[nePerformanceReport::NE_PERF_TERRAIN] = perf->terrain / totalTime * 100.0f;
            perfReport->time[nePerformanceReport::NE_PERF_TERRAIN_CULLING] = perf->terrainCulling / totalTime * 100.0f;
            perfReport->time[nePerformanceReport::NE_PERF_CONTRAIN_SOLVING_1] = perf->constrain_1 / totalTime * 100.0f;
            perfReport->time[nePerformanceReport::NE_PERF_CONTRAIN_SOLVING_2] = perf->constrain_2 / totalTime * 100.0f;
            perfReport->time[nePerformanceReport::NE_PERF_CONTROLLER_CALLBACK] = perf->controllerCallback / totalTime * 100.0f;;
#endif
        }
        else
        {
            f32 totalTime = perf->GetTotalTime();

            if (totalTime < 100.0f)
            {
                perfReport->numSample ++;

                perfReport->accTime[nePerformanceReport::NE_PERF_TOTAL_TIME] += totalTime;

#ifdef DETAIL_PERF_REPORTING                
                perfReport->accTime[nePerformanceReport::NE_PERF_DYNAMIC] += (perf->dynamic / totalTime * 100.0f);
                perfReport->accTime[nePerformanceReport::NE_PERF_POSITION] += (perf->position / totalTime * 100.0f);
                perfReport->accTime[nePerformanceReport::NE_PERF_COLLISION_DETECTION] += (perf->cd / totalTime * 100.0f);
                perfReport->accTime[nePerformanceReport::NE_PERF_COLLISION_CULLING] += (perf->cdCulling / totalTime * 100.0f);
                perfReport->accTime[nePerformanceReport::NE_PERF_TERRAIN] += (perf->terrain / totalTime * 100.0f);
                perfReport->accTime[nePerformanceReport::NE_PERF_TERRAIN_CULLING] += (perf->terrainCulling / totalTime * 100.0f);
                perfReport->accTime[nePerformanceReport::NE_PERF_CONTROLLER_CALLBACK] += (perf->controllerCallback / totalTime * 100.0f);
                perfReport->accTime[nePerformanceReport::NE_PERF_CONTRAIN_SOLVING_1] += (perf->constrain_1 / totalTime * 100.0f);
                perfReport->accTime[nePerformanceReport::NE_PERF_CONTRAIN_SOLVING_2] += (perf->constrain_2 / totalTime * 100.0f);
#endif
                perfReport->time[nePerformanceReport::NE_PERF_TOTAL_TIME] = perfReport->accTime[nePerformanceReport::NE_PERF_TOTAL_TIME] / perfReport->numSample;
#ifdef DETAIL_PERF_REPORTING
                perfReport->time[nePerformanceReport::NE_PERF_DYNAMIC] = perfReport->accTime[nePerformanceReport::NE_PERF_DYNAMIC] / perfReport->numSample;
                perfReport->time[nePerformanceReport::NE_PERF_POSITION] = perfReport->accTime[nePerformanceReport::NE_PERF_POSITION] / perfReport->numSample;
                perfReport->time[nePerformanceReport::NE_PERF_COLLISION_DETECTION] = perfReport->accTime[nePerformanceReport::NE_PERF_COLLISION_DETECTION] / perfReport->numSample;
                perfReport->time[nePerformanceReport::NE_PERF_COLLISION_CULLING] = perfReport->accTime[nePerformanceReport::NE_PERF_COLLISION_CULLING] / perfReport->numSample;
                perfReport->time[nePerformanceReport::NE_PERF_TERRAIN] = perfReport->accTime[nePerformanceReport::NE_PERF_TERRAIN] / perfReport->numSample;
                perfReport->time[nePerformanceReport::NE_PERF_TERRAIN_CULLING] = perfReport->accTime[nePerformanceReport::NE_PERF_TERRAIN_CULLING] / perfReport->numSample;
                perfReport->time[nePerformanceReport::NE_PERF_CONTROLLER_CALLBACK] = perfReport->accTime[nePerformanceReport::NE_PERF_CONTROLLER_CALLBACK] / perfReport->numSample;
                perfReport->time[nePerformanceReport::NE_PERF_CONTRAIN_SOLVING_1] = perfReport->accTime[nePerformanceReport::NE_PERF_CONTRAIN_SOLVING_1] / perfReport->numSample;
                perfReport->time[nePerformanceReport::NE_PERF_CONTRAIN_SOLVING_2] = perfReport->accTime[nePerformanceReport::NE_PERF_CONTRAIN_SOLVING_2] / perfReport->numSample;
#endif
            }
        }
    }

    stepSoFar++;
}

void neFixedTimeStepSimulator::Advance(f32 sec, f32 minTimeStep, f32 maxTimeStep, nePerformanceReport * _perfReport)
{
    perfReport = _perfReport;

#ifdef _WIN32
    if (perfReport)
    {
        for (s32 j = 0; j < nePerformanceReport::NE_PERF_LAST; j++)
        {
            perfReport->time[j] = 0.0f;
        }
        perf->Start();
    }
#endif

    const f32 frameDiffTolerance = 0.2f;
    
    f32 timeLeft = sec + timeFromLastFrame;
    
    f32 currentTimeStep = maxTimeStep;

    while (minTimeStep <= timeLeft)
    {
        while (currentTimeStep <= timeLeft)
        {
            if ((lastTimeStep > 0.0f) && neIsFinite(lastTimeStep))
            {
                f32 diffPercent = neAbs((currentTimeStep - lastTimeStep) / lastTimeStep);

                if (diffPercent > frameDiffTolerance) // more than 20% different
                {
                    if (currentTimeStep > lastTimeStep)
                    {
                        currentTimeStep = lastTimeStep * (1.0f + frameDiffTolerance);
                    }
                    else
                    {
                        currentTimeStep = lastTimeStep * (1.0f - frameDiffTolerance);
                    }
                }
            }

            _currentTimeStep = currentTimeStep;

            oneOnCurrentTimeStep = 1.0f / _currentTimeStep;

            currentRecord = stepSoFar % NE_RB_MAX_PAST_RECORDS;

            Advance(perfReport);

            stepSoFar++;
            
            timeLeft -= currentTimeStep;

            lastTimeStep = currentTimeStep;
        }
        currentTimeStep = neMin(timeLeft, maxTimeStep);
    }
    timeFromLastFrame = timeLeft;

    neCollisionBody_ * cb = activeCB.GetHead();

    while (cb)
    {
        cb->moved = false;

        cb = activeCB.GetNext(cb);
    }
    if (perfReport)
    {
        if (perfReport->reportType == nePerformanceReport::NE_PERF_SAMPLE)
        {
            f32 totalTime = perfReport->time[nePerformanceReport::NE_PERF_TOTAL_TIME] = perf->GetTotalTime();

#ifdef DETAIL_PERF_REPORTING

            perfReport->time[nePerformanceReport::NE_PERF_DYNAMIC] = perf->dynamic / totalTime * 100.0f;
            perfReport->time[nePerformanceReport::NE_PERF_POSITION] = perf->position / totalTime * 100.0f;
            perfReport->time[nePerformanceReport::NE_PERF_COLLISION_DETECTION] = perf->cd / totalTime * 100.0f;
            perfReport->time[nePerformanceReport::NE_PERF_COLLISION_CULLING] = perf->cdCulling / totalTime * 100.0f;
            perfReport->time[nePerformanceReport::NE_PERF_TERRAIN] = perf->terrain / totalTime * 100.0f;
            perfReport->time[nePerformanceReport::NE_PERF_TERRAIN_CULLING] = perf->terrainCulling / totalTime * 100.0f;
            perfReport->time[nePerformanceReport::NE_PERF_CONTRAIN_SOLVING_1] = perf->constrain_1 / totalTime * 100.0f;
            perfReport->time[nePerformanceReport::NE_PERF_CONTRAIN_SOLVING_2] = perf->constrain_2 / totalTime * 100.0f;
            perfReport->time[nePerformanceReport::NE_PERF_CONTROLLER_CALLBACK] = perf->controllerCallback / totalTime * 100.0f;;
#endif
        }
        else
        {
            f32 totalTime = perf->GetTotalTime();

            if (totalTime < 100.0f)
            {
                perfReport->numSample ++;

                perfReport->accTime[nePerformanceReport::NE_PERF_TOTAL_TIME] += totalTime;

#ifdef DETAIL_PERF_REPORTING                
                perfReport->accTime[nePerformanceReport::NE_PERF_DYNAMIC] += (perf->dynamic / totalTime * 100.0f);
                perfReport->accTime[nePerformanceReport::NE_PERF_POSITION] += (perf->position / totalTime * 100.0f);
                perfReport->accTime[nePerformanceReport::NE_PERF_COLLISION_DETECTION] += (perf->cd / totalTime * 100.0f);
                perfReport->accTime[nePerformanceReport::NE_PERF_COLLISION_CULLING] += (perf->cdCulling / totalTime * 100.0f);
                perfReport->accTime[nePerformanceReport::NE_PERF_TERRAIN] += (perf->terrain / totalTime * 100.0f);
                perfReport->accTime[nePerformanceReport::NE_PERF_TERRAIN_CULLING] += (perf->terrainCulling / totalTime * 100.0f);
                perfReport->accTime[nePerformanceReport::NE_PERF_CONTROLLER_CALLBACK] += (perf->controllerCallback / totalTime * 100.0f);
                perfReport->accTime[nePerformanceReport::NE_PERF_CONTRAIN_SOLVING_1] += (perf->constrain_1 / totalTime * 100.0f);
                perfReport->accTime[nePerformanceReport::NE_PERF_CONTRAIN_SOLVING_2] += (perf->constrain_2 / totalTime * 100.0f);
#endif
                perfReport->time[nePerformanceReport::NE_PERF_TOTAL_TIME] = perfReport->accTime[nePerformanceReport::NE_PERF_TOTAL_TIME] / perfReport->numSample;
#ifdef DETAIL_PERF_REPORTING
                perfReport->time[nePerformanceReport::NE_PERF_DYNAMIC] = perfReport->accTime[nePerformanceReport::NE_PERF_DYNAMIC] / perfReport->numSample;
                perfReport->time[nePerformanceReport::NE_PERF_POSITION] = perfReport->accTime[nePerformanceReport::NE_PERF_POSITION] / perfReport->numSample;
                perfReport->time[nePerformanceReport::NE_PERF_COLLISION_DETECTION] = perfReport->accTime[nePerformanceReport::NE_PERF_COLLISION_DETECTION] / perfReport->numSample;
                perfReport->time[nePerformanceReport::NE_PERF_COLLISION_CULLING] = perfReport->accTime[nePerformanceReport::NE_PERF_COLLISION_CULLING] / perfReport->numSample;
                perfReport->time[nePerformanceReport::NE_PERF_TERRAIN] = perfReport->accTime[nePerformanceReport::NE_PERF_TERRAIN] / perfReport->numSample;
                perfReport->time[nePerformanceReport::NE_PERF_TERRAIN_CULLING] = perfReport->accTime[nePerformanceReport::NE_PERF_TERRAIN_CULLING] / perfReport->numSample;
                perfReport->time[nePerformanceReport::NE_PERF_CONTROLLER_CALLBACK] = perfReport->accTime[nePerformanceReport::NE_PERF_CONTROLLER_CALLBACK] / perfReport->numSample;
                perfReport->time[nePerformanceReport::NE_PERF_CONTRAIN_SOLVING_1] = perfReport->accTime[nePerformanceReport::NE_PERF_CONTRAIN_SOLVING_1] / perfReport->numSample;
                perfReport->time[nePerformanceReport::NE_PERF_CONTRAIN_SOLVING_2] = perfReport->accTime[nePerformanceReport::NE_PERF_CONTRAIN_SOLVING_2] / perfReport->numSample;
#endif
            }
        }
    }
}

void neFixedTimeStepSimulator::ResetTotalForce()
{
    neRigidBody_ * rb = activeRB.GetHead();

    while (rb)
    {
        rb->totalForce.SetZero();

        rb->totalTorque.SetZero();

        rb = activeRB.GetNext(rb);
    }
    rb = activeRP.GetHead();

    while (rb)
    {
        rb->totalForce.SetZero();

        rb->totalTorque.SetZero();

        rb = activeRP.GetNext(rb);
    }
}

void neFixedTimeStepSimulator::AdvanceDynamicRigidBodies()
{
    neRigidBody_ * rb = activeRB.GetHead();

    idleBodyCount = 0;

    while (rb)
    {
        rb->AdvanceDynamic(_currentTimeStep);

        rb = activeRB.GetNext(rb);
    }
}       

void neFixedTimeStepSimulator::AdvanceDynamicParticles()
{
    neRigidBody_ * rp = activeRP.GetHead();

    while (rp)
    {
        rp->AdvanceDynamic(_currentTimeStep);

        rp = activeRP.GetNext(rp);
    }
}

void neFixedTimeStepSimulator::AdvancePositionRigidBodies()
{
    neRigidBody_ * rb = activeRB.GetHead();

    while (rb)
    {
        rb->needSolveContactDynamic = true;

        if (rb->status != neRigidBody_::NE_RBSTATUS_IDLE)
        {
            if (!rb->_constraintHeader)
            {
                rb->CheckForIdle();
            }

            if (rb->status != neRigidBody_::NE_RBSTATUS_IDLE)
                rb->AdvancePosition(_currentTimeStep);
        }

        rb = activeRB.GetNext(rb);
    }
}
    
void neFixedTimeStepSimulator::AdvancePositionParticles()
{
    neRigidBody_ * rp = activeRP.GetHead();

    while (rp)
    {
        rp->needSolveContactDynamic = true;

        if (rp->status != neRigidBody_::NE_RBSTATUS_IDLE)
        {
            if (!rp->_constraintHeader)
            {
                rp->CheckForIdle();
            }

            if (rp->status != neRigidBody_::NE_RBSTATUS_IDLE)
                rp->AdvancePosition(_currentTimeStep);
        }

        rp = activeRP.GetNext(rp);
    }
}

void neFixedTimeStepSimulator::ApplyJointDamping()
{
    neFreeListItem<neConstraintHeader> * hitem = (neFreeListItem<neConstraintHeader> *)(*constraintHeaders.BeginUsed());

    while (hitem)
    {
        neConstraintHeader * h = (neConstraintHeader *)hitem;

        hitem = hitem->next;

        neFreeListItem<_neConstraint> * citem = (neFreeListItem<_neConstraint> *) h->head;
        
        while (citem)
        {
            _neConstraint * c = (_neConstraint *)citem; 

            citem = citem->next;

            if (c->enable)
                c->ApplyDamping();
        }
    }
}

void neFixedTimeStepSimulator::ClearCollisionBodySensors()
{
    neCollisionBody_ * cb = activeCB.GetHead();

    while (cb)
    {
        if (cb->sensors)
            cb->ClearSensor();

        cb = activeCB.GetNext(cb);
    }

    neRigidBody_ * rp = activeRP.GetHead();

    while (rp)
    {
        if (rp->sensors)
            rp->ClearSensor();

        rp = activeRP.GetNext(rp);
    }

    rp = activeRB.GetHead();

    while (rp)
    {
        if (rp->sensors)
            rp->ClearSensor();

        rp = activeRP.GetNext(rp);
    }
}

void neFixedTimeStepSimulator::UpdateAABB()
{
    neRigidBody_ * rb = activeRB.GetHead();

    while (rb)
    {
        rb->UpdateAABB();

        rb = activeRB.GetNext(rb);
    }
    neRigidBody_ * rp = activeRP.GetHead();

    while (rp)
    {
        rp->UpdateAABB();

        rp = activeRP.GetNext(rp);
    }
    neCollisionBody_ * cb = activeCB.GetHead();

    while (cb)
    {
        if (cb->moved)
            cb->UpdateAABB();

        cb = activeCB.GetNext(cb);
    }
}

/****************************************************************************
*
*   neFixedTimeStepSimulator::CheckCollision
*
****************************************************************************/ 

void neFixedTimeStepSimulator::CheckCollision()
{
    //OutputDebugString("region.Update\n");/////////////////////////////////

    neDLinkList<neOverlappedPair>::iterator oiter;

    neCollisionResult result;

    neV3 backupVector;
    //OutputDebugString("obj 2 obj test\n");/////////////////////////////////

    for (oiter = region.overlappedPairs.BeginUsed(); oiter.Valid(); oiter++)
    {
        //ASSERT((*oiter)->bodyA);
        //ASSERT((*oiter)->bodyB);
        
        neRigidBodyBase * bodyA;
        neRigidBodyBase * bodyB;

        bodyA = (*oiter)->bodyA;
        bodyB = (*oiter)->bodyB;

        neRigidBody_* ra = bodyA->AsRigidBody();
        neRigidBody_* rb = bodyB->AsRigidBody();

        neCollisionBody_* ca = bodyA->AsCollisionBody();
        neCollisionBody_* cb = bodyB->AsCollisionBody();
        
        if (ca && cb)
            continue;

        neCollisionTable::neReponseBitFlag collisionflag = colTable.Get(bodyA->cid, bodyB->cid);

        if (collisionflag == neCollisionTable::RESPONSE_IGNORE)
            continue;
        
        neBool isCustomeCD = false;

        result.penetrate = false;

        if (ca)
        {
            if (rb->status != neRigidBody_::NE_RBSTATUS_IDLE ||
                rb->isShifted ||
                ca->moved)
            {
                if ((rb->isCustomCD || ca->isCustomCD))
                {
                    if (customCDRB2ABCallback)
                    {
                        isCustomeCD = true;

                        neCustomCDInfo cdInfo;

                        memset(&cdInfo, 0, sizeof(cdInfo));

                        if (customCDRB2ABCallback((neRigidBody*)rb, (neAnimatedBody*)ca, cdInfo))
                        {
                            result.penetrate = true;
                            result.bodyA = ca;
                            result.bodyB = rb;
                            result.collisionFrame[2] = -cdInfo.collisionNormal;
                            result.materialIdA = cdInfo.materialIdB;
                            result.materialIdB = cdInfo.materialIdA;
                            result.contactA = cdInfo.worldContactPointB - ca->GetB2W().pos;
                            result.contactB = cdInfo.worldContactPointA - rb->GetB2W().pos;
                            result.contactAWorld = cdInfo.worldContactPointB;
                            result.contactBWorld = cdInfo.worldContactPointA;
                            result.contactABody = ca->GetB2W().rot.TransposeMulV3(result.contactA);
                            result.contactBBody = rb->GetB2W().rot.TransposeMulV3(result.contactB);
                            result.depth = cdInfo.penetrationDepth;
                            ChooseAxis(result.collisionFrame[0], result.collisionFrame[1], result.collisionFrame[2]);
                        }
                    }
                }
                else
                {
                    backupVector = rb->backupVector - ca->backupVector;

                    CollisionTest(result, ca->col, ca->b2w, 
                                            rb->col, rb->State().b2w, backupVector);

                    if (rb->sensors)
                    {
                        CollisionTestSensor(&rb->col.obb,
                                            rb->sensors,
                                            rb->State().b2w,
                                            ca->col,
                                            ca->b2w,
                                            ca);
                    }
                }
            }
        }
        else
        {
            if (cb)
            {
                if (ra->status != neRigidBody_::NE_RBSTATUS_IDLE ||
                    ra->isShifted ||
                    cb->moved)
                {
                    if ((ra->isCustomCD || cb->isCustomCD))
                    {
                        if (customCDRB2ABCallback)
                        {
                            isCustomeCD = true;

                            neCustomCDInfo cdInfo;

                            memset(&cdInfo, 0, sizeof(cdInfo));

                            if (customCDRB2ABCallback((neRigidBody*)ra, (neAnimatedBody*)cb, cdInfo))
                            {
                                result.penetrate = true;
                                result.bodyA = ra;
                                result.bodyB = cb;
                                result.collisionFrame[2] = cdInfo.collisionNormal;
                                result.materialIdA = cdInfo.materialIdA;
                                result.materialIdB = cdInfo.materialIdB;
                                result.contactA = cdInfo.worldContactPointA - ra->GetB2W().pos;
                                result.contactB = cdInfo.worldContactPointB - cb->GetB2W().pos;
                                result.contactAWorld = cdInfo.worldContactPointA;
                                result.contactBWorld = cdInfo.worldContactPointB;
                                result.contactABody = ra->GetB2W().rot.TransposeMulV3(result.contactA);
                                result.contactBBody = cb->GetB2W().rot.TransposeMulV3(result.contactB);
                                result.depth = cdInfo.penetrationDepth;
                                ChooseAxis(result.collisionFrame[0], result.collisionFrame[1], result.collisionFrame[2]);
                            }
                        }
                    }
                    else
                    {
                        backupVector = cb->backupVector - ra->backupVector;

                        CollisionTest(result, ra->col, ra->State().b2w,
                                        cb->col, cb->b2w, backupVector);
                        
                        if (ra->sensors)
                        {
                            CollisionTestSensor(&ra->col.obb,
                                                ra->sensors,
                                                ra->State().b2w,
                                                cb->col,
                                                cb->b2w,
                                                cb);
                        }
                    }
                }
            }
            else
            {
                neBool doCollision = false;
                
                if (ra->GetConstraintHeader() && 
                    (ra->GetConstraintHeader() == rb->GetConstraintHeader()))
                {
                    if (ra->isCollideConnected && rb->isCollideConnected)
                    {
                        if (ra->status != neRigidBody_::NE_RBSTATUS_IDLE ||
                            rb->status != neRigidBody_::NE_RBSTATUS_IDLE)
    
                            doCollision = true;
                    }
                }
                else
                {
                    if (ra->status != neRigidBody_::NE_RBSTATUS_IDLE ||
                        rb->status != neRigidBody_::NE_RBSTATUS_IDLE || 
                        ra->isShifted ||
                        rb->isShifted)
                    {
                        doCollision = true;
                    }
                }
                if (doCollision)
                {
                    if ((ra->isCustomCD || rb->isCustomCD))
                    {
                        if (customCDRB2RBCallback)
                        {
                            isCustomeCD = true;

                            neCustomCDInfo cdInfo;

                            memset(&cdInfo, 0, sizeof(cdInfo));

                            if (customCDRB2RBCallback((neRigidBody*)ra, (neRigidBody*)rb, cdInfo))
                            {
                                result.penetrate = true;
                                result.bodyA = ra;
                                result.bodyB = rb;
                                result.collisionFrame[2] = cdInfo.collisionNormal;
                                result.materialIdA = cdInfo.materialIdA;
                                result.materialIdB = cdInfo.materialIdB;
                                result.contactA = cdInfo.worldContactPointA - ra->GetB2W().pos;
                                result.contactB = cdInfo.worldContactPointB - rb->GetB2W().pos;
                                result.contactAWorld = cdInfo.worldContactPointA;
                                result.contactBWorld = cdInfo.worldContactPointB;
                                result.contactABody = ra->GetB2W().rot.TransposeMulV3(result.contactA);
                                result.contactBBody = rb->GetB2W().rot.TransposeMulV3(result.contactB);
                                result.depth = cdInfo.penetrationDepth;
                                ChooseAxis(result.collisionFrame[0], result.collisionFrame[1], result.collisionFrame[2]);
                            }
                        }
                    }
                    else
                    {
                        backupVector = rb->backupVector - ra->backupVector;

                        CollisionTest(result, ra->col, ra->State().b2w,
                                            rb->col, rb->State().b2w, backupVector);
                        if (ra->sensors)
                        {
                            CollisionTestSensor(&ra->col.obb,
                                                ra->sensors,
                                                ra->State().b2w,
                                                rb->col,
                                                rb->State().b2w,
                                                rb);
                        }
                        if (rb->sensors)
                        {
                            CollisionTestSensor(&rb->col.obb,
                                                rb->sensors,
                                                rb->State().b2w,
                                                ra->col,
                                                ra->State().b2w,
                                                ra);
                        }
                    }
                }
            }
        }
//      if (perfReport)
//          perf.UpdateCD();

        if (result.penetrate)
        {
            neBool bothAnimated = false;

            if (ra && ra->status == neRigidBody_::NE_RBSTATUS_ANIMATED &&
                rb && rb->status == neRigidBody_::NE_RBSTATUS_ANIMATED)
            {
                bothAnimated = true;
            }
            neBool response = true;

            if (!result.collisionFrame[2].IsFinite() || result.collisionFrame[2].IsConsiderZero())
            {
                response = false;
            }

            result.impulseType = IMPULSE_NORMAL;

            if ((collisionflag & neCollisionTable::RESPONSE_IMPULSE) && 
                response &&
                (!bothAnimated))
            {
                result.bodyA = bodyA;
                result.bodyB = bodyB;
                RegisterPenetration(bodyA, bodyB, result);
            }
            if ((collisionflag & neCollisionTable::RESPONSE_CALLBACK) && collisionCallback && !isCustomeCD)
            {
                static neCollisionInfo cinfo;

                cinfo.bodyA = (neByte *)bodyA;
                cinfo.bodyB = (neByte *)bodyB;
                cinfo.typeA = bodyA->btype == NE_OBJECT_COLISION? NE_ANIMATED_BODY : NE_RIGID_BODY;
                cinfo.typeB = bodyB->btype == NE_OBJECT_COLISION? NE_ANIMATED_BODY : NE_RIGID_BODY;
                cinfo.materialIdA = result.materialIdA;
                cinfo.materialIdB = result.materialIdB;
                cinfo.geometryA = (neGeometry*)result.convexA;
                cinfo.geometryB = (neGeometry*)result.convexB;
                cinfo.bodyContactPointA = result.contactABody;
                cinfo.bodyContactPointB = result.contactBBody;
                cinfo.worldContactPointA = result.contactAWorld;
                cinfo.worldContactPointB = result.contactBWorld;
                cinfo.relativeVelocity = result.initRelVelWorld;
                cinfo.collisionNormal = result.collisionFrame[2];

                collisionCallback(cinfo);
            }
        }
    }
}
    //OutputDebugString("terrain test\n");/////////////////////////////////

void neFixedTimeStepSimulator::CheckTerrainCollision()
{
    neCollisionResult result;

    neTreeNode & rootNode = region.GetTriangleTree().GetRoot();

    neT3 identity;

    identity.SetIdentity();

    neCollision & triCol = fakeCollisionBody.col;

    triCol.convexCount = 1;

    triCol.obb.SetTransform(identity);

    neV3 backupVector;

    for (s32 mop = 0; mop < 2; mop++)
    {
        neList<neRigidBody_> * activeList = &activeRB; 
        
        if (mop == 1)
        {
            activeList = &activeRP;

        }
        neRigidBody_ * rb = activeList->GetHead();

        //for (riter = rbHeap.BeginUsed(); riter.Valid(); riter++)
        while (rb)
        {
            neRigidBody_ * bodyA = (rb);

            if (bodyA->status == neRigidBody_::NE_RBSTATUS_IDLE &&
                !bodyA->isShifted)
            {
                rb = activeList->GetNext(rb);

                continue;
            }
            backupVector = -rb->backupVector;
            
            treeNodes.Clear();

            triangleIndex.Clear();
            
            if (!terrainQueryCallback)
            {
                rootNode.GetCandidateNodes(treeNodes, bodyA->minBound, bodyA->maxBound, 0);

                if (treeNodes.GetUsedCount() == 0)
                {
                    rb = activeList->GetNext(rb);
                    
                    continue;
                }

                //printf("node count %d\n", treeNodes.GetUsedCount());
                
                for (s32 i = 0; i < treeNodes.GetUsedCount(); i++)
                {
                    neTreeNode * t = treeNodes[i];

                    for (s32 j = 0; j < t->triangleIndices.GetUsedCount(); j++)
                    {
                        s32 k;

                        for (k = 0; k < triangleIndex.GetUsedCount(); k++)
                        {
                            if (t->triangleIndices[j] == triangleIndex[k])
                                break;
                        }
                        if (k == triangleIndex.GetUsedCount())
                        {
                            s32 * triIndex = triangleIndex.Alloc();

                            //ASSERT(triIndex);
            
                            *triIndex = t->triangleIndices[j];
                        }
                    }
                }

#ifdef _WIN32
if (perfReport)
    perf->UpdateTerrainCulling();
#endif
                triCol.obb.SetTerrain(triangleIndex, region.terrainTree.triangles, region.terrainTree.vertices);

                triCol.convex = &triCol.obb;

                CollisionTest(result, bodyA->col, ((neRigidBody_*)bodyA)->State().b2w,
                                triCol, identity, backupVector);

                if (bodyA->sensors)
                {
                    CollisionTestSensor(&bodyA->col.obb,
                                        bodyA->sensors,
                                        bodyA->State().b2w,
                                        triCol,
                                        identity,
                                        NULL);
                }

            }
            else
            {
                neTriangle * tris;
                s32 triCount;
                s32 * candidates;
                s32 candidateCount;
                neV3 *verts;

                static neSimpleArray<s32> _candArray;
                static neArray<neTriangle_> _triArray;
                
                terrainQueryCallback(bodyA->minBound, bodyA->maxBound, &candidates, &tris, &verts, &candidateCount, &triCount, (neRigidBody*)bodyA);

#ifdef _WIN32
if (perfReport)
    perf->UpdateTerrainCulling();
#endif

                _candArray.MakeFromPointer(candidates, candidateCount);

                _triArray.MakeFromPointer((neTriangle_*)tris, triCount);

                triCol.obb.SetTerrain(_candArray, _triArray, verts);

                triCol.convex = &triCol.obb;

                CollisionTest(result, bodyA->col, ((neRigidBody_*)bodyA)->State().b2w,
                                triCol, identity, backupVector);

                if (bodyA->sensors)
                {
                    CollisionTestSensor(&bodyA->col.obb,
                                        bodyA->sensors,
                                        bodyA->State().b2w,
                                        triCol,
                                        identity,
                                        NULL);
                }
                _candArray.MakeFromPointer(NULL, 0);
                _triArray.MakeFromPointer(NULL, 0);
            }
            if (result.penetrate)
            {
                result.impulseType = IMPULSE_NORMAL;

                neCollisionTable::neReponseBitFlag collisionflag = colTable.Get(bodyA->cid, -1); //-1 is terrain

                if ((collisionflag & neCollisionTable::RESPONSE_IMPULSE) &&
                    bodyA->status != neRigidBody_::NE_RBSTATUS_ANIMATED)
                {
                    result.bodyA = bodyA;
                    result.bodyB = &fakeCollisionBody;

                    RegisterPenetration(bodyA, &fakeCollisionBody, result);
                }
                if ((collisionflag & neCollisionTable::RESPONSE_CALLBACK) && collisionCallback)
                {
                    static neCollisionInfo cinfo;

                    cinfo.bodyA = (neByte *)bodyA;
                    cinfo.bodyB = (neByte *)result.convexB;
                    cinfo.typeA = NE_RIGID_BODY;
                    cinfo.typeB = NE_TERRAIN;
                    cinfo.materialIdA = result.materialIdA;
                    cinfo.materialIdB = result.materialIdB;
                    cinfo.geometryA = (neGeometry*)result.convexA;
                    cinfo.geometryB = NULL;
                    cinfo.bodyContactPointA = result.contactABody;
                    cinfo.bodyContactPointB = result.contactBBody;
                    cinfo.worldContactPointA = result.contactAWorld;
                    cinfo.worldContactPointB = result.contactBWorld;
                    cinfo.relativeVelocity = result.initRelVelWorld;
                    cinfo.collisionNormal = result.collisionFrame[2];

                    collisionCallback(cinfo);
                }
            }

            rb = activeList->GetNext(rb);
        }
    }//mop

    for (s32 mop2 = 0; mop2 < 2; mop2++)
    {
        neList<neRigidBody_> * activeList = &activeRB; 
        
        if (mop2 == 1)
        {
            activeList = &activeRP;

        }
        neRigidBody_ * rb = activeList->GetHead();

        while (rb)
        {
            if (rb->isShifted2)
            {
                rb->isShifted = true;
                rb->isShifted2 = false;
            }
            else
            {
                rb->isShifted = false;
                rb->isShifted2 = false;
            }
            rb = activeList->GetNext(rb);
        }
    }
}
/****************************************************************************
*
*   neFixedTimeStepSimulator::SolveConstrain
*
****************************************************************************/ 
/*
void RecalcRelative(neCollisionResult * cresult)
{
    cresult->initRelVelWorld = cresult->bodyA->VelocityAtPoint(cresult->contactA) * -1.0f;

    if (cresult->bodyB)
        cresult->initRelVelWorld += cresult->bodyB->VelocityAtPoint(cresult->contactB);
    
    cresult->relativeSpeed = cresult->initRelVelWorld.Length();

    if (!cresult->penetrate)
    {
        cresult->collisionFrame[2] = cresult->initRelVelWorld * (1.0f / cresult->relativeSpeed);
    }
    else
    {
        cresult->relativeSpeed = cresult->initRelVelWorld.Dot(cresult->collisionFrame[2]);
        
        if (cresult->relativeSpeed < 0.0f)
            cresult->relativeSpeed = 0.0f;
    }
}
*/
/*
void neFixedTimeStepSimulator::SolveConstrain()
{
    neDLinkList<neConstraintHeader>::iterator chiter;
    
    for (chiter = constraintHeaders.BeginUsed(); chiter.Valid(); chiter++)
    {
        //check if the whole chain is idle
        neConstraintHeader * header = *chiter; 

        neBodyHandle * bodyHandle = header->bodies.GetHead();

        neBool allIdle = true;

        while (bodyHandle)
        {
            neRigidBody_ * rb = bodyHandle->thing->AsRigidBody();

            if (rb && rb->status != neRigidBody_::NE_RBSTATUS_IDLE)
            {
                allIdle = false;

                break;
            }
            bodyHandle = bodyHandle->next;
        }
    
        if (allIdle)
            continue;

        _neConstraint * constraint = header->head;

        constraint = (*chiter)->head;

        while (constraint)
        {
            if (constraint->limitStates[0].enableLimit || 
                constraint->limitStates[1].enableLimit)
            {
                constraint->CheckLimit();
            }
            neFreeListItem<_neConstraint> * item = (neFreeListItem<_neConstraint> *)constraint;

            constraint = (_neConstraint*)(item->next);
        }
    }
}
*/

/****************************************************************************
*
*   neFixedTimeStepSimulator::RegisterPenetration
*
****************************************************************************/ 

void neFixedTimeStepSimulator::RegisterPenetration(neRigidBodyBase * bodyA, neRigidBodyBase * bodyB, neCollisionResult & cresult)
{
    neRigidBody_ * ba = bodyA->AsRigidBody();

    neRigidBody_ * bb = bodyB->AsRigidBody();

    neRestRecord rc;

    neBool isConnected = false;

    if (ba)
    {
        isConnected = ba->IsConstraintNeighbour(bodyB);
    }
    else
    {
        if (bb)
        {
            isConnected = bb->IsConstraintNeighbour(bodyA);
        }
    }
    if (isConnected && !(bodyA->isCollideDirectlyConnected && bodyB->isCollideDirectlyConnected))
    {
        //HandleCollision(bodyA, bodyB, cresult, IMPULSE_NORMAL, 1.0f);
        
        return;
    }
    if (ba && bb)
    {
        if (bb->IsParticle() )
        {
            //ASSERT(!ba->IsParticle());

            CollisionRigidParticle(ba, bb, cresult);

            return;
        }
    }

    rc.depth = cresult.depth;

    f32 alignWithGravity = cresult.collisionFrame[2].Dot(gravityVector);

    //f32 angle = 0.3f;
    f32 angle = 0.3f;

    if (1)//neAbs(alignWithGravity) > angle)
    {
        neV3 velA = bodyA->VelocityAtPoint(cresult.contactA);

        neV3 velB = bodyB->VelocityAtPoint(cresult.contactB);

        cresult.initRelVelWorld = velA - velB;

        if (alignWithGravity < 0.0f) //ba on top
        {
            if (ba)
            {
                cresult.PrepareForSolver();

                HandleCollision(bodyA, bodyB, cresult, IMPULSE_NORMAL, 1.0f);

                neV3 normalBody;

                if (bodyB)
                    normalBody = bodyB->GetB2W().rot.TransposeMulV3(cresult.collisionFrame[2]);
                else
                    normalBody = cresult.collisionFrame[2];

                rc.SetTmp(bodyB, cresult.contactABody, cresult.contactBBody, normalBody, cresult.materialIdA, cresult.materialIdB);

                ba->AddStackInfo(rc);
            }
            else if (bb)
            {
                SimpleShift(cresult);

                cresult.PrepareForSolver();

                HandleCollision(bodyA, bodyB, cresult, IMPULSE_NORMAL, 1.0f);
            }
        }
        else
        {
            if (bb)
            {
                cresult.PrepareForSolver();

                HandleCollision(bodyA, bodyB, cresult, IMPULSE_NORMAL, 1.0f);

                //if (cresult.relativeSpeed < restingSpeed)
                {
                    neV3 normalBody;

                    if (bodyA)
                        normalBody = bodyA->GetB2W().rot.TransposeMulV3(cresult.collisionFrame[2] * -1.0f);
                    else
                        normalBody = cresult.collisionFrame[2] * -1.0f;

                    rc.SetTmp(bodyA, cresult.contactBBody, cresult.contactABody, normalBody, cresult.materialIdB, cresult.materialIdA);

                    bb->AddStackInfo(rc);
                }
            }
            else if (ba)
            {
                SimpleShift(cresult);

                cresult.PrepareForSolver();

                HandleCollision(bodyA, bodyB, cresult, IMPULSE_NORMAL, 1.0f);
            }
        }
    }
    else // not resting collision, resolve now
    {
        if (ba && bb)
        {
            if (!ba->isShifted && ba->status == neRigidBody_::NE_RBSTATUS_IDLE)
            {
                f32 e = ba->Derive().linearVel.Dot(ba->Derive().linearVel);

                e += ba->Derive().angularVel.Dot(ba->Derive().angularVel);

                if (e > highEnergy)
                {
                    SimpleShift(cresult);
                }
                else
                {
                    bb->SetPos( bb->GetPos() - (cresult.collisionFrame[2] * cresult.depth));

                    bb->isShifted2 = true;
                }
            }
            else if (!bb->isShifted && bb->status == neRigidBody_::NE_RBSTATUS_IDLE)
            {
                f32 e = bb->Derive().linearVel.Dot(bb->Derive().linearVel);

                e += bb->Derive().angularVel.Dot(bb->Derive().angularVel);

                if (e > highEnergy)
                {
                    SimpleShift(cresult);
                }
                else
                {
                    ba->SetPos( ba->GetPos() + (cresult.collisionFrame[2] * cresult.depth));

                    ba->isShifted2 = true;
                }
            }
            else
            {
                SimpleShift(cresult);
            }
        }
        else
        {
            SimpleShift(cresult);
        }
        cresult.PrepareForSolver();

        HandleCollision(bodyA, bodyB, cresult, IMPULSE_NORMAL, 1.0f);
    }
}

void neFixedTimeStepSimulator::CollisionRigidParticle(neRigidBody_ * ba, neRigidBody_ * bb, neCollisionResult & cresult)
{
    cresult.PrepareForSolver();

    HandleCollision(ba, bb, cresult, IMPULSE_NORMAL, 1.0f);

    neV3 shift; 
    
    shift = cresult.collisionFrame[2] * cresult.depth;

    bb->SetPos( bb->GetPos() - shift);
}

void neFixedTimeStepSimulator::SimpleShift(const neCollisionResult & cresult)
{
    neV3 shift; shift = cresult.collisionFrame[2] * cresult.depth;

    f32 aratio, bratio;

    neRigidBodyBase * bodyA = cresult.bodyA;

    neRigidBodyBase * bodyB = cresult.bodyB;

    neRigidBody_ * ba = bodyA->AsRigidBody();

    neRigidBody_ * bb = bodyB->AsRigidBody();

    if (!ba)
    {
        aratio = 0.0f;
        bratio = 1.0f;

        if (bb)
            bb->isShifted2 = true;
    }
    else if (!bb)
    {
        aratio = 1.0f;
        bratio = 0.0f;

        if (ba)
            ba->isShifted2 = true;
    }
    else
    {
        ba->isShifted2 = true;
        bb->isShifted2 = true;

        f32 totalMass = ba->mass + bb->mass;

        aratio = bb->mass / totalMass;

        bratio = ba->mass / totalMass;
    }

    if (ba)
    {
        ba->SetPos( ba->GetPos() + (shift * aratio) * 1.0f);
    }
    if (bb)
    {
        bb->SetPos( bb->GetPos() - (shift * bratio) * 1.0f);
    }
}


neBool neFixedTimeStepSimulator::CheckBreakage(neRigidBodyBase * originalBody, TConvex * convex, const neV3 & contactPoint, neV3 & impulse)
{
    f32 impulseMag;

    neV3 breakPlane;

    neM3 rot = originalBody->GetB2W().rot * convex->c2p.rot;

    breakPlane = rot * convex->breakInfo.breakPlane;

    neV3 breakImpulse = impulse;

    breakImpulse.RemoveComponent(breakPlane);

    impulseMag = breakImpulse.Length();

    if (impulseMag < convex->breakInfo.breakMagnitude)
    {
        return false;
    }

    f32 dot = impulse.Dot(breakPlane);

    impulse = breakPlane * dot;

    breakImpulse *= convex->breakInfo.breakMagnitude / impulseMag;

    neRigidBody_* newBody = NULL;

    neV3 newImpulse, newContactPoint;

    newImpulse = breakImpulse * convex->breakInfo.breakAbsorb;

    breakImpulse *= (1.0f - convex->breakInfo.breakAbsorb);

    impulse += breakImpulse;

    neBodyType originalBodyType;

    if (originalBody->AsRigidBody())
    {
        originalBodyType = NE_RIGID_BODY;
    }
    else
    {
        originalBodyType = NE_ANIMATED_BODY;
    }

    switch (convex->breakInfo.flag)
    {
    case neGeometry::NE_BREAK_NORMAL:
    case neGeometry::NE_BREAK_NORMAL_PARTICLE:

        newBody = CreateRigidBodyFromConvex(convex, originalBody);

        newContactPoint = contactPoint - newBody->GetPos();

        newBody->ApplyCollisionImpulse(newImpulse, newContactPoint, IMPULSE_NORMAL);

        break;
    case neGeometry::NE_BREAK_ALL:
    case neGeometry::NE_BREAK_ALL_PARTICLE:

        break;
    case neGeometry::NE_BREAK_NEIGHBOUR:
    case neGeometry::NE_BREAK_NEIGHBOUR_PARTICLE:

        break;
    }
    if (originalBodyType == NE_ANIMATED_BODY)
    {
        impulse = newImpulse;
    }
    if (newBody)
    {
        breakageCallback((neByte *)originalBody, originalBodyType, (neGeometry *)convex, (neRigidBody*)newBody);
    
        return true;
    }
    else
    {
        return false;
    }
}


void neFixedTimeStepSimulator::ResetStackHeaderFlag()
{
    neStackHeaderItem * hitem = (neStackHeaderItem *)(*stackHeaderHeap.BeginUsed());

    while (hitem)
    {
        neStackHeader * sheader = (neStackHeader *) hitem;

        sheader->dynamicSolved = false;

        hitem = hitem->next;
    }
}

/****************************************************************************
*
*   neFixedTimeStepSimulator::SetTerrainMesh
*
****************************************************************************/ 

void neFixedTimeStepSimulator::SetTerrainMesh(neTriangleMesh * tris)
{
    region.MakeTerrain(tris);
}

void neFixedTimeStepSimulator::FreeTerrainMesh()
{
    region.FreeTerrain();
}

neStackHeader * neFixedTimeStepSimulator::NewStackHeader(neStackInfo * sinfo)
{
    neStackHeader * n = stackHeaderHeap.Alloc();

    //ASSERT(n);

    n->Null();

    n->sim = this;

    if (sinfo)
        n->Add(sinfo);

    return n;
}

neConstraintHeader * neFixedTimeStepSimulator::NewConstraintHeader()
{
    neConstraintHeader * ret = constraintHeaders.Alloc();

    ret->Reset();

    return ret;
}

void neFixedTimeStepSimulator::CheckStackHeader()
{
    neStackHeaderItem * item = (neStackHeaderItem *)(*stackHeaderHeap.BeginUsed());

    while (item)
    {
        neStackHeader * h = (neStackHeader *)item;

        //ASSERT(h->infoCount > 0);
        //assert(h->infoCount > 0);
        item = item->next;
    }
}

void neFixedTimeStepSimulator::UpdateConstraintControllers()
{
    neFreeListItem<neConstraintHeader> * hitem = (neFreeListItem<neConstraintHeader> *)(*constraintHeaders.BeginUsed());

    while (hitem)
    {
        neConstraintHeader * h = (neConstraintHeader *)hitem;

        hitem = hitem->next;

        neFreeListItem<_neConstraint> * citem = (neFreeListItem<_neConstraint> *) h->head;
        
        while (citem)
        {
            _neConstraint * c = (_neConstraint *)citem; 

            citem = citem->next;

            if (c->enable)
                c->UpdateController();
        }
    }
}

void neFixedTimeStepSimulator::FreeAllBodies()
{
    neRigidBody_ * rb = activeRB.GetHead();

    while (rb)
    {
        rb->Free();

        neRigidBody_ * rbNext = activeRB.GetNext(rb);

        activeRB.Remove(rb);

        rigidBodyHeap.Dealloc(rb, 1);
        
        rb = rbNext;
    }
    rb = inactiveRB.GetHead();

    while (rb)
    {
        rb->Free();

        neRigidBody_ * rbNext = inactiveRB.GetNext(rb);

        inactiveRB.Remove(rb);

        rigidBodyHeap.Dealloc(rb, 1);
        
        rb = rbNext;
    }

    //ASSERT(activeRB.count == 0);

    //ASSERT(inactiveRB.count == 0);

    activeRB.Reset();

    inactiveRB.Reset();

    ///////////////////////////////////////////////////////////

    neRigidBody_ * rp = activeRP.GetHead();

    while (rp)
    {
        rp->Free();

        neRigidBody_ * rpNext = activeRP.GetNext(rp);

        activeRP.Remove(rp);

        rigidParticleHeap.Dealloc(rp, 1);
        
        rp = rpNext;
    }
    rp = inactiveRP.GetHead();

    while (rp)
    {
        rp->Free();

        neRigidBody_ * rpNext = inactiveRP.GetNext(rp);

        inactiveRP.Remove(rp);

        rigidParticleHeap.Dealloc(rp, 1);
        
        rp = rpNext;
    }

    //ASSERT(activeRP.count == 0);

    //ASSERT(inactiveRP.count == 0);

    activeRP.Reset();

    inactiveRP.Reset();

    ///////////////////////////////////////////////////////////

    neCollisionBody_ * cb = activeCB.GetHead();

    while (cb)
    {
        cb->Free();

        neCollisionBody_ * cbNext = activeCB.GetNext(cb);

        activeCB.Remove(cb);

        collisionBodyHeap.Dealloc(cb, 1);
        
        cb = cbNext;
    }

    cb = inactiveCB.GetHead();

    while (cb)
    {
        cb->Free();

        neCollisionBody_ * cbNext = inactiveCB.GetNext(cb);

        inactiveCB.Remove(cb);

        collisionBodyHeap.Dealloc(cb, 1);
        
        cb = cbNext;
    }

    //ASSERT(activeCB.count == 0);

    //ASSERT(inactiveCB.count == 0);

    activeCB.Reset();

    inactiveCB.Reset();

    //ASSERT(rigidBodyHeap.GetUsedCount() == 0);

    //ASSERT(collisionBodyHeap.GetUsedCount() == 0);

    //ASSERT(constraintHeap.GetUsedCount() == 0);

    //ASSERT(geometryHeap.GetUsedCount() == 0);

    //ASSERT(controllerHeap.GetUsedCount() == 0);

//  ASSERT(miniConstraintHeap.GetUsedCount() == 0);

    //ASSERT(stackInfoHeap.GetUsedCount() == 0);
}

/****************************************************************************
*
*   neFixedTimeStepSimulator::GetMemoryUsage
*
****************************************************************************/ 

void neFixedTimeStepSimulator::GetMemoryAllocated(s32 & memoryAllocated)
{
    memoryAllocated = 0;

    memoryAllocated += rigidBodyHeap.Size() * sizeof(neFreeListItem<neRigidBody_>);

    memoryAllocated += rigidParticleHeap.Size() * sizeof(neFreeListItem<neRigidBody_>);
    
    memoryAllocated += collisionBodyHeap.Size() * sizeof(neFreeListItem<neCollisionBody_>);

    memoryAllocated += treeNodes.GetTotalSize() * sizeof(neTreeNode *);

    memoryAllocated += triangleIndex.GetTotalSize() * sizeof(s32);

    memoryAllocated += constraintHeaders.Size() * sizeof(neFreeListItem<neConstraintHeader>);

    memoryAllocated += constraintHeap.Size() * sizeof(neFreeListItem<_neConstraint>);

//  memoryAllocated += miniConstraintHeap.Size() * sizeof(neFreeListItem<neMiniConstraint>);

    memoryAllocated += controllerHeap.Size() * sizeof(neFreeListItem<neController>);

    memoryAllocated += stackInfoHeap.Size() * sizeof(neFreeListItem<neStackInfo>);

    memoryAllocated += stackHeaderHeap.Size() * sizeof(neFreeListItem<neStackHeader>);

    memoryAllocated += sensorHeap.Size() * sizeof(neFreeListItem<neSensor_>);

    memoryAllocated += geometryHeap.Size() * sizeof(neFreeListItem<TConvex>);

    //memoryAllocated += cresultHeap.Size() * sizeof(neFreeListItem<neCollisionResult>);
    memoryAllocated += cresultHeap.GetTotalSize() * sizeof(neFreeListItem<neCollisionResult>);

    memoryAllocated += pointerBuffer1.GetTotalSize() * sizeof(neByte *);

    memoryAllocated += pointerBuffer2.GetTotalSize() * sizeof(neByte *);

    //region stuff
    memoryAllocated += region.b2b.GetTotalSize() * sizeof(neOverlapped);

    memoryAllocated += region.b2p.GetTotalSize() * sizeof(neOverlapped);

    memoryAllocated += region.newBodies.GetTotalSize() * sizeof(neAddBodyInfo);

    memoryAllocated += region.bodies.Size() * sizeof(neFreeListItem<neRigidBodyBase *>);

    memoryAllocated += region.overlappedPairs.Size() * sizeof(neFreeListItem<neOverlappedPair>);

    memoryAllocated += region.coordLists[0].coordList.Size() * sizeof(neFreeListItem<CCoordListEntry>);

    memoryAllocated += region.coordLists[1].coordList.Size() * sizeof(neFreeListItem<CCoordListEntry>);

    memoryAllocated += region.coordLists[2].coordList.Size() * sizeof(neFreeListItem<CCoordListEntry>);

    memoryAllocated += region.terrainTree.nodes.GetTotalSize() * sizeof(neTreeNode);

    memoryAllocated += region.terrainTree.triangles.GetTotalSize() * sizeof(neTriangle_);
}

/****************************************************************************
*
*   neCollisionTable_::neCollisionTable_
*
****************************************************************************/ 

neCollisionTable_::neCollisionTable_()
{
    for (s32 i = 0 ; i < NE_COLLISION_TABLE_MAX; i++)
    {
        for (s32 j = 0 ; j < NE_COLLISION_TABLE_MAX; j++)
            table[i][j] = table[j][i] = neCollisionTable::RESPONSE_IMPULSE;

        terrainTable[i] = neCollisionTable::RESPONSE_IMPULSE;
    }
}

/****************************************************************************
*
*   neCollisionTable_::~neCollisionTable_
*
****************************************************************************/ 

neCollisionTable_::~neCollisionTable_()
{
}

/****************************************************************************
*
*   neCollisionTable_::Set
*
****************************************************************************/ 

void neCollisionTable_::Set(s32 collisionID1, s32 collisionID2, neCollisionTable::neReponseBitFlag value)
{
    //ASSERT(collisionID1 >= -1 && collisionID1 < neCollisionTable::NE_COLLISION_TABLE_MAX);
    //ASSERT(collisionID2 >= -1 && collisionID2 < neCollisionTable::NE_COLLISION_TABLE_MAX);

    if (collisionID1 == -1 && collisionID2 == -1)
    {
        return;
    }

    if (collisionID1 == -1)
    {
        terrainTable[collisionID2] = value;
    }
    else if (collisionID2 == -1)
    {
        terrainTable[collisionID1] = value;
    }
    else
    {
        table[collisionID1][collisionID2] = value;
        table[collisionID2][collisionID1] = value;
    }
}

/****************************************************************************
*
*   neCollisionTable_::Get
*
****************************************************************************/ 

neCollisionTable::neReponseBitFlag neCollisionTable_::Get(s32 collisionID1, s32 collisionID2)
{
    //ASSERT(collisionID1 >= -1 && collisionID1 < neCollisionTable::NE_COLLISION_TABLE_MAX);
    //ASSERT(collisionID2 >= -1 && collisionID2 < neCollisionTable::NE_COLLISION_TABLE_MAX);

    if (collisionID1 == -1 && collisionID2 == -1)
    {
        return neCollisionTable::RESPONSE_IGNORE;
    }
    
    if (collisionID1 == -1)
    {
        return terrainTable[collisionID2];
    }
    else if (collisionID2 == -1)
    {
        return terrainTable[collisionID1];
    }
    
    return table[collisionID1][collisionID2];
}


/**solver.cpp**/

//#include "math/ne_type.h"
//#include "math/ne_debug.h"
//#include "tokamak.h"
//#include "containers.h"
//#include "scenery.h"
//#include "collision.h"
//#include "constraint.h"
//#include "rigidbody.h"
//#include "stack.h"
//#include "simulator.h"
//#include "scenery.h"
//#include "message.h"

f32 CONSTRAINT_THESHOLD_JOINT = 0.000f;

f32 CONSTRAINT_THESHOLD_CONTACT = 0.001f;

f32 CONSTRAINT_THESHOLD_LIMIT = 0.001f;

f32 CONSTRAINT_CONVERGE_FACTOR_JOINT = .5f;

f32 CONSTRAINT_CONVERGE_FACTOR_CONTACT = .5f;

f32 CONSTRAINT_CONVERGE_FACTOR_LIMIT = 0.5f;

NEINLINE void ApplyCollisionImpulseFast(neRigidBody_ * rb, const neV3 & impulse, const neV3 & contactPoint, s32 currentRecord, neBool immediate = true)
{
    neV3 dv, da;

    dv = impulse * rb->oneOnMass;

    da = contactPoint.Cross(impulse);

//  if (immediate)
        rb->Derive().linearVel += dv;
//  else
//  {
//      rb->totalDV += dv;
//      rb->impulseCount++;
//  }

//  rb->dvRecord[currentRecord] += dv;

//  neV3 dav = rb->Derive().Iinv * da;

//  rb->davRecord[currentRecord] += dav;

//  if (immediate)
    {
        rb->State().angularMom += da;

        rb->Derive().angularVel = rb->Derive().Iinv * rb->State().angularMom;
    }
//  else
//  {
//      rb->totalDA += da;
//      
//      rb->twistCount++;
//  }
}


void neFixedTimeStepSimulator::AddCollisionResult(neCollisionResult & cresult)
{
    neCollisionResult * newcr = cresultHeap2.Alloc(0);

    *newcr = cresult;
}

f32 neCollisionResult::SolveConstraint(neFixedTimeStepSimulator * sim)
{
    neV3 impulse;

    f32 ret = 0.0f;

    neV3 pII; 

    if (neIsConsiderZero(depth))
    {
        pII = kInv * initRelVel;

        impulse = -pII;
    }
    else
    {
        neV3 & desireVel = contactAWorld;//w2c * contactAWorld;

        neV3 tmp = desireVel * CONSTRAINT_CONVERGE_FACTOR_JOINT;
        
        f32 len = depth * CONSTRAINT_CONVERGE_FACTOR_JOINT;//tmp.Length();

        if (len > 0.05f)
        {
            tmp = desireVel * (0.05f / len);
        }
        tmp *= sim->oneOnCurrentTimeStep;// * CONSTRAINT_CONVERGE_FACTOR_JOINT;

        neV3 deltaU = tmp + initRelVel *  CONSTRAINT_CONVERGE_FACTOR_JOINT;

        impulse = kInv * deltaU * -1.0f;
    }
    neRigidBody_ * rb;

    if (bodyA && (rb = bodyA->AsRigidBody()))
    {
        ApplyCollisionImpulseFast(rb, impulse, contactA, sim->currentRecord);

        rb->needRecalc = true;
    }
    if (bodyB && (rb = bodyB->AsRigidBody()))
    {
        neV3 bimpulse = impulse * -1.0f;
        
        ApplyCollisionImpulseFast(rb, bimpulse, contactB, sim->currentRecord);

        rb->needRecalc = true;
    }
    return ret;
}

f32 neCollisionResult::SolveSlider(neFixedTimeStepSimulator * sim)
{
    neV3 impulse;

    if (neIsConsiderZero(finalRelativeSpeed))
    {
        impulse = kInv * initRelVel * -1.0f;

        impulse = impulse - impulse.Dot(contactBWorld) * contactBWorld;
    }
    else
    {
        neV3 & desireVel = contactAWorld;

        neV3 tmp = desireVel * CONSTRAINT_CONVERGE_FACTOR_JOINT;
        
        f32 len = finalRelativeSpeed * CONSTRAINT_CONVERGE_FACTOR_JOINT;//tmp.Length();

        if (len > 0.05f)
        {
            tmp = desireVel * (0.05f / len);
        }
        tmp *= sim->oneOnCurrentTimeStep;// * CONSTRAINT_CONVERGE_FACTOR_JOINT;

        neV3 deltaU = tmp + initRelVel *  CONSTRAINT_CONVERGE_FACTOR_JOINT;

        impulse = kInv * deltaU * -1.0f;

        f32 dot = impulse.Dot(contactBWorld);

        impulse = impulse - dot * contactBWorld;
    }
    neRigidBody_ * rb;

    if (bodyA && (rb = bodyA->AsRigidBody()))
    {
        ApplyCollisionImpulseFast(rb, impulse, contactA, sim->currentRecord);

        rb->needRecalc = true;
    }
    if (bodyB && (rb = bodyB->AsRigidBody()))
    {
        neV3 bimpulse = impulse * -1.0f;
        
        ApplyCollisionImpulseFast(rb, bimpulse, contactB, sim->currentRecord);

        rb->needRecalc = true;
    }
    return 0.0f;
}

f32 neCollisionResult::SolveSliderLimit(neFixedTimeStepSimulator * sim)
{
    ASSERT(depth >= 0.0f);

    neV3 impulse;

    f32 desireSpeed = depth * sim->oneOnCurrentTimeStep * CONSTRAINT_CONVERGE_FACTOR_LIMIT;

    if (desireSpeed > finalRelativeSpeed)
    {
        f32 dU = desireSpeed - finalRelativeSpeed * CONSTRAINT_CONVERGE_FACTOR_LIMIT;

        neV3 deltaU; deltaU = contactBWorld * dU;

        impulse = kInv * deltaU *-1.0f;
    }
    else
    {
        return 0.0f;
    }

    neRigidBody_ * rb;

    if (bodyA && (rb = bodyA->AsRigidBody()))
    {
        ApplyCollisionImpulseFast(rb, impulse, contactA, sim->currentRecord);

        rb->needRecalc = true;
    }
    if (bodyB && (rb = bodyB->AsRigidBody()) && !(sim->solverLastIteration))
    {
        neV3 bimpulse = impulse * -1.0f;
        
        ApplyCollisionImpulseFast(rb, bimpulse, contactB, sim->currentRecord);

        rb->needRecalc = true;
    }
    return 0.0f;
}

f32 neCollisionResult::SolveContact(neFixedTimeStepSimulator * sim)
{
    neV3 impulse1; impulse1.SetZero();
    neV3 impulse2; impulse2.SetZero();

    f32 ret = 0.0f;

    if (initRelVel[2] < 0.0f)
    {   
        if (sim->solverStage == 0)
            impulse1 = sim->CalcNormalImpulse(*this, FALSE);
        else
            impulse1 = sim->CalcNormalImpulse(*this, TRUE);

        initRelVel[2] = 0.0f;
    }
    if (depth > 0.05f)
        depth = 0.05f;

    f32 adjustedDepth = depth - CONSTRAINT_THESHOLD_CONTACT * sim->gravityMag;

    f32 desireNormalSpeed;

    if (depth <= 0.0f) // -ve mean not penetrating
    {

    }
    else if (adjustedDepth <= 0.0f)
    {
    }
    else if (sim->solverStage != 0)
    {
        desireNormalSpeed = adjustedDepth * sim->oneOnCurrentTimeStep * CONSTRAINT_CONVERGE_FACTOR_CONTACT;

        if (desireNormalSpeed > initRelVel[2])
        {
            neV3 deltaU;

            deltaU.Set(0.0f, 0.0f, desireNormalSpeed - initRelVel[2] * CONSTRAINT_CONVERGE_FACTOR_CONTACT);

            //deltaU.Set(- initRelVel[0], - initRelVel[1], desireNormalSpeed - initRelVel[2]);

            ret = neAbs(deltaU[2]);
    
            impulse2 = kInv * deltaU;
        }
    }
    neV3 impulse = impulse1 + impulse2;

    impulse = collisionFrame * impulse;

    neRigidBody_ * rb;

    if (bodyA && (rb = bodyA->AsRigidBody()))
    {
        ApplyCollisionImpulseFast(rb, impulse, contactA, sim->currentRecord, true);

        rb->needRecalc = true;
    }
    if (bodyB && (rb = bodyB->AsRigidBody()) && !(sim->solverLastIteration))
    {
        neV3 bimpulse = impulse * -1.0f;
        
        ApplyCollisionImpulseFast(rb, bimpulse, contactB, sim->currentRecord, true);

        rb->needRecalc = true;
    }
    return ret;
}

f32 neCollisionResult::SolveAngular(f32 pdepth, const neV3 & axis, f32 relAV, neFixedTimeStepSimulator * sim)
{
    neV3 deltaL;

    f32 threshold = 0.00f;

    f32 angularDisplacementNeeded;
    
    if (pdepth > threshold)
    {
        angularDisplacementNeeded = (pdepth - threshold);
    }
    else if (pdepth < -threshold)
    {
        angularDisplacementNeeded = (pdepth + threshold);
    }
    else
    {
        ASSERT(0);
        return 0;
    }
    f32 deltaAng;

    f32 scaledCorrection = angularDisplacementNeeded * CONSTRAINT_CONVERGE_FACTOR_LIMIT;

    neBool applyImpulse = false;
    
    f32 angularDisplacment;

    if (scaledCorrection > 0.0f)
    {
        if (relAV > 0.0f)
        {
            //spining into the limit
            
            deltaAng = -scaledCorrection * sim->oneOnCurrentTimeStep;

            deltaAng -= (relAV * CONSTRAINT_CONVERGE_FACTOR_LIMIT);

            applyImpulse = true;
        }
        else // relAV < 0
        {
            //spining out of the limit, but is it fast enough?
            return 0.0f;
            
            angularDisplacment = relAV * sim->_currentTimeStep;

            f32 d = angularDisplacementNeeded + angularDisplacment;

            if (d > 0.0f)
            {
                deltaAng = -d * CONSTRAINT_CONVERGE_FACTOR_LIMIT * sim->oneOnCurrentTimeStep;

                applyImpulse = true;
            }
        }
    }
    else
    {
        if (relAV < 0.0f)
        {
            //spining into the limit
            deltaAng = -scaledCorrection  * sim->oneOnCurrentTimeStep;

            deltaAng -= (relAV * CONSTRAINT_CONVERGE_FACTOR_LIMIT);

            applyImpulse = true;
        }
        else // relAV > 0
        {
            //spining out of the limit, but is it fast enough?
            return 0.0f;
            angularDisplacment = relAV * sim->_currentTimeStep;

            f32 d = angularDisplacementNeeded + angularDisplacment;

            if (d < 0.0f)
            {
                deltaAng = -d * CONSTRAINT_CONVERGE_FACTOR_LIMIT * sim->oneOnCurrentTimeStep;

                applyImpulse = true;
            }
        }
    }

    if (!applyImpulse)
        return 0;

    neV3 deltaAngVel; deltaAngVel = axis * deltaAng;

    deltaL = kInv * deltaAngVel;

    neRigidBody_ * rb;

    if (bodyA && (rb = bodyA->AsRigidBody()))
    {
        rb->SetAngMom(rb->State().angularMom + deltaL);

        rb->davRecord[rb->sim->stepSoFar % NE_RB_MAX_PAST_RECORDS] += rb->Derive().Iinv * deltaL;

//      rb->totalDA += deltaL;

//      rb->twistCount++;
    }
    if (bodyB && (rb = bodyB->AsRigidBody()))
    {
        rb->SetAngMom(rb->State().angularMom - deltaL);

        rb->davRecord[rb->sim->stepSoFar % NE_RB_MAX_PAST_RECORDS] -= rb->Derive().Iinv * deltaL;

//      rb->totalDA -= deltaL;

//      rb->twistCount++;
    }
    return 0;
}

f32 neCollisionResult::SolveAngular2(const neV3 & axisA, const neV3 & axisB, f32 relAV, f32 desireAV, f32 maxTorque, neFixedTimeStepSimulator * sim)
{
    f32 deltaAng = desireAV - relAV;

    //deltaAng *= 0.5f;

    neV3 deltaAngVel, deltaLA, deltaLB;

    deltaAngVel = axisA * deltaAng;

    deltaLA = kInv * deltaAngVel;

    neV3 torque = deltaLA * sim->oneOnCurrentTimeStep;

    f32 torqueMag = torque.Length();

    if (torqueMag > maxTorque && !neIsConsiderZero(neAbs(maxTorque)))
    {
        deltaLA = torque * (maxTorque * sim->_currentTimeStep / torqueMag);
    }

    neRigidBody_ * rb;

    if (bodyA && (rb = bodyA->AsRigidBody()))
    {
        rb->SetAngMom(rb->State().angularMom + deltaLA);

        rb->davRecord[rb->sim->stepSoFar % NE_RB_MAX_PAST_RECORDS] += rb->Derive().Iinv * deltaLA;

        //rb->totalDA += deltaLA;

        //rb->twistCount++;
    }
    deltaAngVel = axisB * deltaAng;

    deltaLB = kInv * deltaAngVel;

    torque = deltaLB * sim->oneOnCurrentTimeStep;

    torqueMag = torque.Length();

    if (torqueMag > maxTorque/* && !neIsConsiderZero(maxTorque)*/)
    {
        deltaLB = torque * (maxTorque * sim->_currentTimeStep / torqueMag);
    }
    if (bodyB && (rb = bodyB->AsRigidBody()))
    {
        rb->SetAngMom(rb->State().angularMom - deltaLB);

        rb->davRecord[rb->sim->stepSoFar % NE_RB_MAX_PAST_RECORDS] -= rb->Derive().Iinv * deltaLB;

        //rb->totalDA -= deltaLB;

        //rb->twistCount++;
    }
    return 0.0f;
}

f32 neCollisionResult::SolveAngular3(f32 pdepth, const neV3 & axis, f32 relAV, neFixedTimeStepSimulator * sim)
{
    neV3 deltaL;

    f32 threshold = 0.00f;

    //f32 angularDisplacementNeeded = -pdepth;
/*  
    if (pdepth > threshold)
    {
        angularDisplacementNeeded = (pdepth - threshold);
    }
    else if (pdepth < -threshold)
    {
        angularDisplacementNeeded = (pdepth + threshold);
    }
    else
    {
        ASSERT(0);
        return 0;
    }
*/  f32 deltaAng;

    f32 scaledDepth = pdepth * CONSTRAINT_CONVERGE_FACTOR_LIMIT;

    if (scaledDepth > 0.1f)
        scaledDepth = 0.1f;
    else if (scaledDepth < -0.1f)
        scaledDepth = -0.1f;

    neBool applyImpulse = false;
    
//  f32 angularDisplacment;

    if (scaledDepth > 0.0f)
    {
        if (relAV > 0.0f)
        {
            //spining into the limit
            
            deltaAng = -scaledDepth * sim->oneOnCurrentTimeStep;

            deltaAng -= (relAV * CONSTRAINT_CONVERGE_FACTOR_LIMIT);

            applyImpulse = true;
        }
        else //relAV < 0.0f
        {
            //spining out of the limit, but is it fast enough?

/*          deltaAng = -scaledDepth * sim->oneOnCurrentTimeStep;

            angularDisplacment = relAV * sim->_currentTimeStep;

            f32 d = pdepth + angularDisplacment;
            
            if (d > 0.0f)
            {
                deltaAng = -d * CONSTRAINT_CONVERGE_FACTOR_LIMIT * sim->oneOnCurrentTimeStep;

                applyImpulse = true;
            }
*/      }
    }
    else
    {
        if (relAV < 0.0f)
        {
            //spining into the limit
            deltaAng = -scaledDepth  * sim->oneOnCurrentTimeStep;

            deltaAng -= (relAV * CONSTRAINT_CONVERGE_FACTOR_LIMIT);

            applyImpulse = true;
        }
        else //relAV > 0.0f
        {
            //spining out of the limit, but is it fast enough?

/*          angularDisplacment = relAV * sim->_currentTimeStep;

            f32 d = pdepth + angularDisplacment;

            if (d < 0.0f)
            {
                deltaAng = -d * CONSTRAINT_CONVERGE_FACTOR_LIMIT * sim->oneOnCurrentTimeStep;

                applyImpulse = true;
            }
*/      }
    }

    if (!applyImpulse)
        return 0;

    neV3 deltaAngVel; deltaAngVel = axis * deltaAng;

    deltaL = kInv * deltaAngVel;

    neRigidBody_ * rb;

    if (bodyA && (rb = bodyA->AsRigidBody()))
    {
        neV3 deltaLA = k.TransposeMulV3(deltaL);

        rb->SetAngMom(rb->State().angularMom + deltaLA);

        rb->davRecord[rb->sim->stepSoFar % NE_RB_MAX_PAST_RECORDS] += rb->Derive().Iinv * deltaLA;

//      rb->totalDA += deltaL;

//      rb->twistCount++;
    }
    if (bodyB && (rb = bodyB->AsRigidBody()))
    {
        rb->SetAngMom(rb->State().angularMom - deltaL);

        rb->davRecord[rb->sim->stepSoFar % NE_RB_MAX_PAST_RECORDS] -= rb->Derive().Iinv * deltaL;

//      rb->totalDA -= deltaL;

//      rb->twistCount++;
    }
    return 0;
}

f32 neCollisionResult::SolveAngularPrimary(neFixedTimeStepSimulator * sim)
{
    neRigidBody_ * ba = NULL;

    neRigidBody_ * bb = NULL;

    f32 ava = 0.0f, avb = 0.0f;

    if (bodyA)
    {
        ba = bodyA->AsRigidBody();
    }
    if (bodyB)
    {
        bb = bodyB->AsRigidBody();
    }
    if (!ba && !bb)
        return 0.0f;

    SolveAngular(depth, contactBBody/*this is the axis*/, relativeSpeed, sim);

    return 0.0f;
}

f32 neCollisionResult::SolveAngularSecondary(neFixedTimeStepSimulator * sim)
{
    neRigidBody_ * ba = NULL;

    neRigidBody_ * bb = NULL;

    f32 ava = 0.0f, avb = 0.0f;

    if (bodyA && (ba = bodyA->AsRigidBody()))
    {
        ava = ba->Derive().angularVel.Dot(contactABody);
    }
    if (bodyB && (bb = bodyB->AsRigidBody()))
    {
        avb = bb->Derive().angularVel.Dot(contactBBody);
    }
    if (!ba && !bb)
        return 0.0f;

    f32 relAV = ava - avb;

    SolveAngular3(depth, contactBBody, relAV, sim);
/*
    SolveAngular(depth, contactBBody, relAV, sim);

    ava = 0.0f;
    
    avb = 0.0f;

    if (bodyA && (ba = bodyA->AsRigidBody()))
    {
        ava = ba->Derive().angularVel.Dot(contactABody);
    }
    if (bodyB && (bb = bodyB->AsRigidBody()))
    {
        avb = bb->Derive().angularVel.Dot(contactBBody);
    }
    relAV = ava - avb;

    SolveAngular(impulseScale, contactABody, relAV, sim);
*/
    return 0.0f;
}

f32 neCollisionResult::SolveAngularMotorPrimary(neFixedTimeStepSimulator * sim)
{
    neRigidBody_ * ba = NULL;

    neRigidBody_ * bb = NULL;

    f32 ava = 0.0f, avb = 0.0f;

    if (bodyA && (ba = bodyA->AsRigidBody()))
    {
        ava = ba->Derive().angularVel.Dot(contactABody);
    }
    if (bodyB && (bb = bodyB->AsRigidBody()))
    {
        avb = bb->Derive().angularVel.Dot(contactBBody);
    }
    if (!ba && !bb)
        return 0.0f;

    f32 relAV = ava - avb;

    SolveAngular2(contactABody, contactBBody, relAV, finalRelativeSpeed, depth, sim);

    return 0.0f;
}

f32 neCollisionResult::SolveRelativeLinear(neFixedTimeStepSimulator * sim)
{
    f32 velA = 0.0f, velB = 0.0f;

    neRigidBody_ * ba, * bb;

    if (bodyA && (ba = bodyA->AsRigidBody()))
    {
        velA = ba->Derive().linearVel.Dot(contactABody);
    }
    if (bodyB && (bb = bodyB->AsRigidBody()))
    {
        velB = bb->Derive().linearVel.Dot(contactABody);
    }
    if (!ba && !bb)
        return 0.0f;

    f32 speedRel = velA - velB;

    f32 impulseMag = kInv[0][0] * (finalRelativeSpeed - speedRel);

    neV3 impulse;

    f32 mag = impulseMag;

    if (mag > depth)
    {
        impulse = depth * sim->_currentTimeStep * contactABody;
    }
    else if (mag < -depth)
    {
        impulse = -depth * sim->_currentTimeStep * contactABody;
    }
    else
    {
        impulse = impulseMag * contactABody;
    }

    neV3 zeroVector; zeroVector.SetZero();

    if (ba)
        ApplyCollisionImpulseFast(ba, impulse, zeroVector, sim->currentRecord);

    impulse *= -1.f;

    if (bb)
        ApplyCollisionImpulseFast(bb, impulse, zeroVector, sim->currentRecord);

    return 0.0f;
}

void neFixedTimeStepSimulator::SolveContactConstrain()
{
// first solve all single object to terrain/animated body contacts

    cresultHeap2.Clear();

    neStackInfoItem * sitem = (neStackInfoItem * )stackHeaderX.head;

    solverStage = 1;

    while (sitem)
    {
        neStackInfo * sinfo = (neStackInfo *)sitem;

        sitem = sitem->next;

        if (sinfo->body->status == neRigidBody_::NE_RBSTATUS_IDLE)
        {
            continue;
        }
        if (!sinfo->body->needSolveContactDynamic)
        {
            continue;
        }
        sinfo->body->AddContactImpulseRecord(0);

        if (cresultHeap2.GetUsedCount() == 0)
        {
            continue;
        }
        
        neRigidBody_* rb = NULL;
        

        for (s32 tt = 0; tt < cresultHeap2.GetUsedCount(); tt++)
        {
            //neCollisionResult * cr = (neCollisionResult *)ci;
            neCollisionResult * cr = &cresultHeap2[tt];//(neCollisionResult *)ci;

            HandleCollision(cr->bodyA, cr->bodyB, *cr, IMPULSE_CONTACT, 1.0f/*cr->impulseScale*/);

            //ci = ci->next;
            rb = cr->bodyA->AsRigidBody();
        }
        cresultHeap2.Clear();

        ASSERT(rb);

        if (rb->CheckStationary())
        {
            if (rb->IsRestPointStillValid())
            {
                if (rb->CheckRestHull())
                {
                    rb->BecomeIdle();
                }
            }
        }
    }

    // release any empty stack header
    
    neStackHeaderItem * hitem = (neStackHeaderItem *)(*stackHeaderHeap.BeginUsed());

    while (hitem)
    {       
        neStackHeader * sheader = (neStackHeader *) hitem;

        hitem = hitem->next;

        if (sheader->infoCount > 1)
            continue;

        neStackInfo * s = sheader->head;

        neRigidBody_ * rb = s->body;

        if (s->isTerminator)
        {
            ASSERT(rb->AllRestRecordInvalid());
            
            rb->stackInfo = NULL;

            sheader->Remove(s, 1);

            stackInfoHeap.Dealloc(s, 1);

            stackHeaderHeap.Dealloc(sheader);
        }
        else
        {
            sheader->Remove(s, 1);

            stackHeaderHeap.Dealloc(sheader);

            stackHeaderX.Add(s);
        }
    }
    hitem = (neStackHeaderItem *)(*stackHeaderHeap.BeginUsed());

    while (hitem)
    {
        neStackHeader * sheader = (neStackHeader *) hitem;

        hitem = hitem->next;

        if (sheader->isAllIdle || sheader->dynamicSolved)
            continue;

        pointerBuffer2.Clear(); // stack headers

        pointerBuffer1.Clear(); // constraint headers

        contactConstraintHeader.RemoveAll();

        cresultHeap2.Clear();

        sheader->AddToSolver(/*true*/);

#ifdef _WIN32
        perf->UpdateConstrain1();
#endif

        SolveOneConstrainChain(-1.0f, 2);

#ifdef _WIN32
        perf->UpdateConstrain2();
#endif

        if (contactConstraintHeader.StationaryCheck())
        {
            //all of object are stationary enough
            contactConstraintHeader.BecomeIdle(true);
        }
    }
    contactConstraintHeader.RemoveAll();

    cresultHeap2.Clear();
}

f32 neFixedTimeStepSimulator::SolveLocal(neCollisionResult * cr)
{
    f32 ret = 0.0f;

    neV3 velA;

    neV3 velB;

    neRigidBody_ * ba = NULL;

    neRigidBody_ * bb = NULL;

    if (cr->bodyA && cr->bodyA->AsRigidBody())
    {
        ba = cr->bodyA->AsRigidBody();
    }
    if (cr->bodyB && cr->bodyB->AsRigidBody())
    {
        bb = cr->bodyB->AsRigidBody();
    }
    
    switch (cr->impulseType)
    {
    case IMPULSE_CONTACT:
    case IMPULSE_CONSTRAINT:
    case IMPULSE_SLIDER:
        {
            if (ba)
            {
                velA = ba->VelocityAtPoint(cr->contactA);   
            }
            else
            {
                velA.SetZero();
            }
            if (bb)
            {
                velB = bb->VelocityAtPoint(cr->contactB);   
            }
            else
            {
                velB.SetZero();
            }

            neV3 relVel = velA - velB;

            cr->initRelVelWorld = relVel;

            if (cr->impulseType == IMPULSE_CONTACT)
                cr->initRelVel = cr->w2c * relVel;
            else if (cr->impulseType == IMPULSE_SLIDER)
                cr->initRelVel = relVel - relVel.Dot(cr->contactBWorld) * cr->contactBWorld;
            else
                cr->initRelVel = relVel;
        }   
        break;
    
    case IMPULSE_SLIDER_LIMIT_PRIMARY:
        {
            if (ba)
            {
                velA = ba->VelocityAtPoint(cr->contactA);   
            }
            else
            {
                velA.SetZero();
            }
            if (bb)
            {
                velB = bb->VelocityAtPoint(cr->contactB);   
            }
            else
            {
                velB.SetZero();
            }

            neV3 relVel = velA - velB;

            cr->finalRelativeSpeed = relVel.Dot(cr->contactBWorld) * -1.0f;

            //cr->initRelVel = cr->finalRelativeSpeed * cr->contactBWorld;
        }
        break;
        
    case IMPULSE_ANGULAR_LIMIT_PRIMARY:
        {
            f32 wA = 0.0f, wB = 0.0f;

            if (ba)
            {
                wA = ba->Derive().angularVel.Dot(cr->contactBBody);
            }
            if (bb)
            {
                wB = bb->Derive().angularVel.Dot(cr->contactBBody); 
            }
            cr->relativeSpeed = wA - wB;
        }
        break;

    case IMPULSE_ANGULAR_LIMIT_SECONDARY:
        {
            //do nothing
        }

        break;
    }
    switch (cr->impulseType)
    {
    case IMPULSE_CONTACT:

        ret = cr->SolveContact(this);

        break;
    
    case IMPULSE_CONSTRAINT:
        {
            //cr->relativeSpeed = cr->initRelVel.Length();

            //if (!neIsConsiderZero(cr->relativeSpeed))
                ret = cr->SolveConstraint(this);
        }
        break;

    case IMPULSE_SLIDER:
        {
            ret = cr->SolveSlider(this);
        }
        break;
    case IMPULSE_SLIDER_LIMIT_PRIMARY:
        {
            ret = cr->SolveSliderLimit(this);
        }
        break;
    case IMPULSE_ANGULAR_LIMIT_PRIMARY:

        //((_neConstraint*)(cr->convexA))->limitStates[0].EnforcePrimary();

        ret = cr->SolveAngularPrimary(this);

        break;

    case IMPULSE_ANGULAR_LIMIT_SECONDARY:
        
        //((_neConstraint*)(cr->convexA))->limitStates[1].EnforceSecondary();

        ret = cr->SolveAngularSecondary(this);

        break;

    case IMPULSE_ANGULAR_MOTOR_PRIMARY:

        ret = cr->SolveAngularMotorPrimary(this);
        
        break;

    case IMPULSE_ANGULAR_MOTOR_SECONDARY:
        break;

    case IMPULSE_RELATIVE_LINEAR_VELOCITY:

        ret = cr->SolveRelativeLinear(this);

        break;
    }

    return ret;
}

void neFixedTimeStepSimulator::CheckIfStationary()
{
    neBool allStationary = true;
    s32 jj;
    
    for (jj = 0; jj < pointerBuffer1.GetUsedCount(); jj++) // in this loop we apply the total impulse from the 
                                                            // solving stage to the rigid bodies
    {
        neConstraintHeader * ch = (neConstraintHeader*)pointerBuffer1[jj];

        if (!ch->StationaryCheck())
        {
            allStationary = FALSE;
        }
    }// next jj, next constraint

    if (!contactConstraintHeader.StationaryCheck())
    {
        allStationary = FALSE;
    }
    if (allStationary)
    {   
        //make everything idle
        for (jj = 0; jj < pointerBuffer1.GetUsedCount(); jj++) // in this loop we apply the total impulse from the 
                                                                // solving stage to the rigid bodies
        {
            neConstraintHeader * ch = (neConstraintHeader*)pointerBuffer1[jj];

            ch->BecomeIdle();
        }// next jj, next constraint

        contactConstraintHeader.BecomeIdle();
    }
    else
    {
        //make everything idle
        for (jj = 0; jj < pointerBuffer1.GetUsedCount(); jj++) // in this loop we apply the total impulse from the 
                                                                // solving stage to the rigid bodies
        {
            neConstraintHeader * ch = (neConstraintHeader*)pointerBuffer1[jj];

            ch->WakeUp();
        }// next jj, next constraint

        contactConstraintHeader.WakeUp();
    }
}

/****************************************************************************
*
*   neFixedTimeStepSimulator::ResolvePenetration
*
****************************************************************************/ 
void neFixedTimeStepSimulator::ResolvePenetration()
{
    //CheckStackHeader();

    neStackInfoItem * sitem = (neStackInfoItem * )stackHeaderX.head;
/*
    while (sitem)
    {
        neStackInfo * sinfo = (neStackInfo *)sitem;

        sitem = sitem->next;

        if (sinfo->body->status != neRigidBody_::NE_RBSTATUS_IDLE || sinfo->body->isShifted)
            sinfo->Resolve();
    }
*/
    neSimpleArray<neStackHeader*> & activeHeaderBuffer = *((neSimpleArray<neStackHeader*>*)&pointerBuffer1);

    activeHeaderBuffer.Clear();

    // check if any of the stacks are all idle

    neStackHeaderItem * hitem = (neStackHeaderItem *)(*stackHeaderHeap.BeginUsed());

    while (hitem)
    {
        neStackHeader * sheader = (neStackHeader *) hitem;

        sheader->isAllIdle = true;

        neStackInfoItem * sitem = (neStackInfoItem *)sheader->head;

        while (sitem)
        {
            neStackInfo * sinfo = (neStackInfo*) sitem;

            sitem = sitem->next;

            if (sinfo->body->status != neRigidBody_::NE_RBSTATUS_IDLE || sinfo->body->isShifted)
            {
                sheader->isAllIdle = false;

                break;
            }
        }

        hitem = hitem->next;
    }

    //Resolve penetration

    hitem = (neStackHeaderItem *)(*stackHeaderHeap.BeginUsed());

    while (hitem)
    {       
        neStackHeader * sheader = (neStackHeader *) hitem;

        //sheader->CheckHeader();

        if (!sheader->isAllIdle)
        {
            //if (!sheader->dynamicSolved)
            //  sheader->Resolve();

            *activeHeaderBuffer.Alloc() = sheader;
        }
        hitem = hitem->next;
    }

    // check if any of the rest contact are still valid

    neRigidBody_ * rb;

    neList<neRigidBody_> * activeList;
    
    for (s32 j = 0; j < 2; j++)
    {
        if (j == 0)
            activeList = &activeRB;
        else
            activeList = &activeRP;

        rb = activeList->GetHead();

        while (rb)
        {
            //if (((s32)rb->id % 3) != cc)
            //{
            //  rb = activeRB.GetNext(rb);
            //  continue;
            //}
            if (!rb->stackInfo)
            {
                rb = activeList->GetNext(rb);
                continue;
            }
            if (rb->stackInfo->isTerminator)
            {
                rb = activeList->GetNext(rb);
                continue;
            }
            s32 v;
            
            v = rb->CheckContactValidity();

            rb->isShifted = false;
            if (v == 0)
            {
    /*          char ss[256];
                sprintf(ss, "%d disconnected\n", rb->id);
                OutputDebugString(ss);
    */      }
            if (v == 0)
            {
                if (rb->stackInfo->stackHeader->isHeaderX)
                {
                    stackHeaderX.Remove(rb->stackInfo);
    /*
                    char ss[256];
                    sprintf(ss, "disconnect %d from ground\n", rb->id);
                    OutputDebugString(ss);
    */              
                    stackInfoHeap.Dealloc(rb->stackInfo, 1);

                    ASSERT(rb->AllRestRecordInvalid());

                    rb->stackInfo = NULL;                   
                }
            }

            if (v <= 1) //no longer resting
            {
                if (rb->status == neRigidBody_::NE_RBSTATUS_IDLE && rb->_constraintHeader == NULL)
                {
                    rb->WakeUp();
                }
            }
            rb = activeList->GetNext(rb);
        }
    }

/*
    hitem = (neStackHeaderItem *)(*stackHeaderHeap.BeginUsed());
    
    while (hitem)
    {       
        neStackHeader * sheader = (neStackHeader *) hitem;

        sheader->CheckHeader();

        hitem = hitem->next;
    }

    CheckStackHeader();
*/

    // check if the stack set is disconnected

    s32 thisFrame = stepSoFar % 5; //check stack disconnection once every 3 frames

    for (s32 i = 0; i < activeHeaderBuffer.GetUsedCount(); i++)
    {
        s32 g = i % 5;

        if (g == thisFrame)
        {
            if (!activeHeaderBuffer[i]->isAllIdle)
            {
                activeHeaderBuffer[i]->CheckStackDisconnected();
        
                stackHeaderHeap.Dealloc(activeHeaderBuffer[i]);
            }
        }
    }

/*  
    hitem = (neStackHeaderItem *)(*stackHeaderHeap.BeginUsed());
    
    while (hitem)
    {       
        neStackHeader * sheader = (neStackHeader *) hitem;

        sheader->CheckHeader();

        hitem = hitem->next;
    }
*/
    // apply collision impulse

    // check for empty stack sets
    hitem = (neStackHeaderItem *)(*stackHeaderHeap.BeginUsed());

    while (hitem)
    {
        neStackHeader * h = (neStackHeader *) hitem;

        if (h->infoCount == 0)
        {
            hitem = hitem->next;

            stackHeaderHeap.Dealloc(h);
        }
        else
        {
            hitem = hitem->next;
        }
    }

/*
    hitem = (neStackHeaderItem *)(*stackHeaderHeap.BeginUsed());

    while (hitem)
    {       
        neStackHeader * sheader = (neStackHeader *) hitem;

        sheader->CheckHeader();

        hitem = hitem->next;
    }
*/
}

/****************************************************************************
*
*   neFixedTimeStepSimulator::HandleCollision
*
****************************************************************************/ 

neV3 neFixedTimeStepSimulator::CalcNormalImpulse(neCollisionResult & cresult, neBool isContact)
{
    neV3 pI, pII, impulse;

    pI.Set(0.0f, 0.0f, -cresult.initRelVel[2] / cresult.k[2][2]);

    pII = cresult.kInv * cresult.initRelVel;

    pII *= -1.0f;

    neV3 pDiff = pII - pI;

    f32 e = 0.0f;
    f32 et = 1.0f;
    f32 u = 0.0f;

    f32 eA, uA, eB, uB, den;

    GetMaterial(cresult.materialIdA, uA, eA, den);

    GetMaterial(cresult.materialIdB, uB, eB, den);

    if (uA < uB)
        u = uA;
    else
        u = uB;
    
    if (isContact)
    {
        e = 0.0f;

        //u *= 1.2f;

        //if (u > 1.0f)
        //  u = 1.0f;
    }
    else
    {
        //e = (eA + eB) * 0.5f;
        if (eA < eB)
            e = eA;
        else
            e = eB;
    }

    et = 0.0f;//u * 2.0f - 1.0f;

    neV3 candidate = pI * (1.0f + e);
    
    candidate += (1.0f + et) * pDiff; 

    f32 t1 = candidate[0] * candidate[0] + candidate[1] * candidate[1];

    f32 t2 = u * candidate[2];

    t2 = t2 * t2;

    if (t1 > t2)
    {
        f32 kap,
            temp;

        kap = u * (1.0f + e) * pI[2];

        neV3 tempv; tempv.Set( pII[0], pII[1], 0.0f );

        temp = sqrtf(tempv.Dot(tempv ));

        temp -= u * pDiff[2];

        if ( neAbs( temp ) > NE_ZERO )
        {
            kap /= temp;

            impulse = pI * (1.0f + e);

            tempv = pDiff * kap;

            impulse = impulse + tempv;
        }
        else
        {
            impulse.SetZero();
        }
    }
    else
    {
        impulse = candidate;
    }

    return impulse;
}

f32 neFixedTimeStepSimulator::HandleCollision(neRigidBodyBase * bodyA, 
                                               neRigidBodyBase * bodyB, 
                                               neCollisionResult & cresult, neImpulseType impulseType, 
                                               f32 scale)
{
//  ASSERT(impulseType == IMPULSE_NORMAL);

    neM3 w2c; 

    w2c.SetTranspose(cresult.collisionFrame);

    ASSERT(w2c.IsFinite());

    neV3 velA;

    neV3 velB;

    neRigidBody_ * ba = NULL;

    neRigidBody_ * bb = NULL;

    if (bodyA && bodyA->AsRigidBody())
    {
        ba = bodyA->AsRigidBody();
    }
    if (bodyB && bodyB->AsRigidBody())
    {
        bb = bodyB->AsRigidBody();
    }

    if (!ba && !bb)
        return 0;

    if (ba)
    {
        velA = ba->VelocityAtPoint(cresult.contactA);   
    }
    else
    {
        velA.SetZero();
    }
    if (bb)
    {
        velB = bb->VelocityAtPoint(cresult.contactB);   
    }
    else
    {
        velB.SetZero();
    }

    neV3 relVel = velA - velB;

    cresult.initRelVelWorld = relVel;

    cresult.initRelVel = w2c * relVel;

    f32 initSpeed = cresult.relativeSpeed = cresult.initRelVel.Length();

    f32 theshold = -1.0f;

//  neM3 k, kInv;

    neV3 impulse;

    f32 ret = 0.0f;

    switch (impulseType)
    {
    case IMPULSE_NORMAL:
        {
            if (cresult.initRelVel[2] >= 0.0f)
            {
                return 0.0f;
            }

            impulse = CalcNormalImpulse(cresult, FALSE);
        }
        break;

    case IMPULSE_CONTACT:
        {
            cresult.SolveContact(this);
            return 0.0f;
        }
        break;
        
    default:
        ASSERT(0);
        break;
    }

    if (!impulse.IsFinite())
    {
        //ASSERT(0);

        return 0.0f;
    }

    impulse = cresult.collisionFrame * impulse * scale;

    neV3 bimpulse;

    if (impulseType == IMPULSE_NORMAL)
    {
        neBool doBreakCheck = (impulseType == IMPULSE_NORMAL && breakageCallback); 

        if (doBreakCheck)
        {
            if (!ba) // meaning either ca and bb
            {
                if (cresult.convexA->breakInfo.flag != neGeometry::NE_BREAK_DISABLE)
                {
                    CheckBreakage(bodyA, cresult.convexA, cresult.contactAWorld, impulse);
                }
                bimpulse = impulse * -1.0f;

                if (cresult.convexB->breakInfo.flag != neGeometry::NE_BREAK_DISABLE)
                {
                    CheckBreakage(bb, cresult.convexB, cresult.contactBWorld, bimpulse);
                }
                bb->ApplyCollisionImpulse(bimpulse, cresult.contactB, impulseType);

                bb->needRecalc = true;
            }
            else // meaning ba and bb, or ba and cb
            {
                if (!bb) //ba and cb
                {
                    if (cresult.convexB->breakInfo.flag != neGeometry::NE_BREAK_DISABLE)
                    {
                        bimpulse = impulse * -1.0f;

                        CheckBreakage(bodyB, cresult.convexB, cresult.contactBWorld, bimpulse);

                        impulse = bimpulse * -1.0f;
                    }
                    if (cresult.convexA->breakInfo.flag != neGeometry::NE_BREAK_DISABLE)
                    {
                        CheckBreakage(ba, cresult.convexA, cresult.contactAWorld, impulse);
                    }
                    ba->ApplyCollisionImpulse(impulse, cresult.contactA, impulseType);

                    ba->needRecalc = true;
                }
                else // meaning ba and bb
                {
                    if (cresult.convexA->breakInfo.flag != neGeometry::NE_BREAK_DISABLE)
                    {
                        CheckBreakage(ba, cresult.convexA, cresult.contactAWorld, impulse);
                    }
                    bimpulse = impulse * -1.0f;
                    
                    if (cresult.convexB->breakInfo.flag != neGeometry::NE_BREAK_DISABLE)
                    {
                        CheckBreakage(bb, cresult.convexB, cresult.contactBWorld, bimpulse);
                    }
                    ba->ApplyCollisionImpulse(impulse, cresult.contactA, impulseType);

                    ba->needRecalc = true;
        
                    bb->ApplyCollisionImpulse(bimpulse, cresult.contactB, impulseType);

                    bb->needRecalc = true;
                }
            }
        }
        else
        {
            if (ba)
            {
                ba->ApplyCollisionImpulse(impulse, cresult.contactA, impulseType);

                ba->needRecalc = true;
            }
            if (bb)
            {
                bimpulse = impulse * -1.0f;

                bb->ApplyCollisionImpulse(bimpulse, cresult.contactB, impulseType);

                bb->needRecalc = true;
            }
        }
    }
    else
    {
        if (ba)
        {
            ba->ApplyCollisionImpulse(impulse, cresult.contactA, impulseType);

            ba->needRecalc = true;
        }
        if (bb)
        {
            bimpulse = impulse * -1.0f;
            
            bb->ApplyCollisionImpulse(bimpulse, cresult.contactB, impulseType);

            bb->needRecalc = true;
        }
    }
    return ret;
}

#define AUTO_SLEEP_ON

void neFixedTimeStepSimulator::SolveAllConstrain()
{
    if (constraintHeaders.GetUsedCount() == 0)
        return;

    neDLinkList<neConstraintHeader>::iterator chiter;

    for (chiter = constraintHeaders.BeginUsed(); chiter.Valid(); chiter++)
    {
        (*chiter)->solved = false;
    }

    for (chiter = constraintHeaders.BeginUsed(); chiter.Valid(); chiter++)
    {
        if ((*chiter)->solved)
            continue;

        pointerBuffer2.Clear(); // stack headers

        pointerBuffer1.Clear(); // constraint headers

        contactConstraintHeader.RemoveAll();

        cresultHeap.Clear();

        cresultHeap2.Clear();

        neByte ** pt = pointerBuffer1.Alloc();

        *pt = (neByte*)(*chiter);
        
        neBodyHandle * bodyHandle = (*chiter)->bodies.GetHead();

        s32 allIdle = 0;
        s32 nbody = 0;
        neRigidBody_ * lastrb = NULL;

        while (bodyHandle)
        {
            neRigidBody_ * rb = bodyHandle->thing->AsRigidBody();

            if (rb)
            {
                rb->maxErrCResult = NULL;

                nbody++;
            }
        
            if (rb && rb->status != neRigidBody_::NE_RBSTATUS_IDLE)
            {
                allIdle++;
            }
            lastrb = rb;
            bodyHandle = bodyHandle->next;
        }
        if (allIdle == 0)
            continue;

        //lastrb->WakeUpAllJoint();
        
        f32 epsilon = -1.0f;//DEFAULT_CONSTRAINT_EPSILON;

        s32 iteration = -1;

        (*chiter)->AddToSolver(epsilon, iteration); // pointerBuffer2 will be filled after this call

        AddContactConstraint(epsilon, iteration);

        SolveOneConstrainChain(epsilon, iteration);

        CheckIfStationary();
    }
    contactConstraintHeader.RemoveAll();
}

void neFixedTimeStepSimulator::SolveOneConstrainChain(f32 epsilon, s32 iteration)
{
    solverStage = 0;

    if (cresultHeap.GetUsedCount() == 0 && cresultHeap2.GetUsedCount() == 0)
    {
        return;
    }

    if (epsilon == -1.0f)
        epsilon = DEFAULT_CONSTRAINT_EPSILON;

    if (iteration == -1)
    {
        iteration = (s32) (DEFAULT_CONSTRAINT_ITERATION);// * cresultHeap.GetUsedCount());

        if (iteration == 0)
            iteration = 1;
    }
    // pp == 0 is friction stage
    // pp == 1 is penetration stage


    s32 checkSleep = iteration >> 1;

    solverLastIteration = false;
    
    for (s32 pp = 0; pp < 2; pp++)
    {
        if (pp == 1)
        {
            solverStage = 1;
/*
            for (s32 tt = 0; tt < cresultHeap2.GetUsedCount(); tt++)
            {
                neCollisionResult * cr = &cresultHeap2[tt];
                
                ASSERT(cr->impulseType == IMPULSE_CONTACT);

                cr->StartStage2();
            }
*/      }
        s32 it;

        it = iteration;

        for (s32 i = 0; i < it; i++)
        {
            neBool doCheckSleep = false;

            if (pp == 1  && i == checkSleep)
            {
                doCheckSleep = true;
            }
            if (pp == 1 && i == (it -1))
            {
                solverLastIteration = true;
            }
            f32 maxError = 0.0f;

            s32 nConstraint = 0;

            neCollisionResult * cresult = &cresultHeap[0]; //*cresultHeap.BeginUsed();

            s32 tt;

            for (tt = 0; tt < cresultHeap.GetUsedCount(); tt++)
            {
                neCollisionResult * cr = &cresultHeap[tt];

                f32 err = 0.0f;

                err = SolveLocal(cr);

                if (err > maxError)
                    maxError = err;
            }
            //for (tt = 0; tt < cresultHeap2.GetUsedCount(); tt++)
            for (tt = cresultHeap2.GetUsedCount()-1; tt >= 0 ; tt--)
            {
                neCollisionResult * cr = &cresultHeap2[tt];

                f32 err = 0.0f;

                err = SolveLocal(cr);

                if (err > maxError)
                    maxError = err;
            }
            //maxError = 0.0f;

            s32 jj;
            
            for (jj = 0; jj < pointerBuffer1.GetUsedCount(); jj++) // in this loop we apply the total impulse from the 
                                                                    // solving stage to the rigid bodies
            {
                neConstraintHeader * ch = (neConstraintHeader*)pointerBuffer1[jj];

                ch->TraverseApplyConstraint(doCheckSleep);
            }// next jj, next constraint

            contactConstraintHeader.TraverseApplyConstraint(doCheckSleep);

#if 1//AUTO_SLEEP_ON

            //if (doCheckSleep)
            if (pp == 1 && i == (it - 2)) // the second last iteration
            {
                for (tt = 0; tt < cresultHeap2.GetUsedCount(); tt++)
                {
                    neCollisionResult * cr = &cresultHeap2[tt];

                    if (cr->impulseType == IMPULSE_CONTACT)
                    {
                        cr->PrepareForSolver(false, true);
                    }

                }
            }
#endif //AUTO_SLEEP_ON

        }
    }

    cresultHeap.Clear();

    cresultHeap2.Clear();

    ASSERT(cresultHeap.GetUsedCount() == 0);
}
void neFixedTimeStepSimulator::AddContactConstraint(f32 & epsilon, s32 & iteration)
{
    for (s32 i = 0; i < pointerBuffer2.GetUsedCount(); i++)
    {
        neStackHeader * sheader = (neStackHeader *) pointerBuffer2[i];

        sheader->AddToSolver(/*true*/);

        neStackInfoItem * sitem = (neStackInfoItem *)sheader->head;

        while (sitem)
        {
            neStackInfo * sinfo = (neStackInfo*) sitem;

            sitem = sitem->next;

            if (sinfo->body->_constraintHeader &&
                !sinfo->body->_constraintHeader->solved)
            {
                sinfo->body->_constraintHeader->AddToSolver(epsilon, iteration);

                *pointerBuffer1.Alloc() = (neByte *)(sinfo->body->_constraintHeader);
            }
        }
    }
}

/****************************************************************************
*
*   ChooseAxis
*
****************************************************************************/ 

void ChooseAxis(neV3 & x, neV3 & y, const neV3 & normal)
{
    neV3 mag;

    mag[0] = neAbs(normal[0]);
    mag[1] = neAbs(normal[1]);
    mag[2] = neAbs(normal[2]);

    if (mag[0] > mag[1])
    {
        if (mag[0] > mag[2])
        {
            x[0] = (normal[1] + normal[2]) / normal[0] * -1.0f;
            x[1] = 1.0f;
            x[2] = 1.0f;
        }
        else
        {
            x[2] = (normal[0] + normal[1]) / normal[2] * -1.0f;
            x[0] = 1.0f;
            x[1] = 1.0f;
        }
    }
    else if (mag[1] > mag[2])
    {
        x[1] = (normal[0] + normal[2]) / normal[1] * -1.0f;
        x[0] = 1.0f;
        x[2] = 1.0f;
    }
    else
    {
        x[2] = (normal[0] + normal[1]) / normal[2] * -1.0f;
        x[0] = 1.0f;
        x[1] = 1.0f;
    }
    x.Normalize();
    y = normal.Cross(x);
}


/**sphere.cpp**/

//#include "tokamak.h"
//#include "containers.h"
//#include "scenery.h"
//#include "collision.h"
//#include "collision2.h"
//#include "constraint.h"
//#include "rigidbody.h"

#include <assert.h>
#include <stdio.h>

#define INSIDE_BOX_BOUNDARY(_dir) (flag2[_dir] < 0.0f)

#define BOX_SPHERE_DO_TEST(whichCase, _dir) {configuration = whichCase; dir = _dir;}

void Box2SphereTest(neCollisionResult & result, TConvex & boxA, neT3 & transA, TConvex & sphereB, neT3 & transB)
{
    f32 penetration;
    
    result.penetrate = false;

    neV3 sphereCenter;
    
    sphereCenter = (transA.FastInverse() * transB).pos;

    neV3 flag1, flag2;

    for (s32 i = 0; i < 3; i++)
        flag1[i] = sphereCenter[i] < 0.0f ? -1.0f: 1.0f;

    neV3 sphereCenterAbs;

    sphereCenterAbs = sphereCenter * flag1;

    flag2 = sphereCenterAbs - boxA.as.box.boxSize;

    s32 configuration, dir;

    if (INSIDE_BOX_BOUNDARY(1))
    {
        if (INSIDE_BOX_BOUNDARY(2))
            if (INSIDE_BOX_BOUNDARY(0))
                configuration = -1; //center inside the box
            else
                BOX_SPHERE_DO_TEST(0, 0)
        else
            if (INSIDE_BOX_BOUNDARY(0))
                BOX_SPHERE_DO_TEST(0, 2)
            else
                BOX_SPHERE_DO_TEST(1, 1)
    }   
    else if (INSIDE_BOX_BOUNDARY(2))
    {
        if (INSIDE_BOX_BOUNDARY(0))
            BOX_SPHERE_DO_TEST(0, 1)
        else
            BOX_SPHERE_DO_TEST(1, 2)
    }
    else if (INSIDE_BOX_BOUNDARY(0))
    {
        BOX_SPHERE_DO_TEST(1, 0)
    }
    else
    {
        BOX_SPHERE_DO_TEST(2, 0)
    }

    neV3 contactA;

    if (configuration == -1)
    {
        //find the shallowest penetration
        neV3 depth; depth = boxA.as.box.boxSize - flag2;
        s32 k;

        if (depth[0] < depth[1])
        {
            if (depth[0] < depth[2])
            { //x
                k = 0;
            }
            else
            { //z
                k = 2;
            }
        }
        else if (depth[1] < depth[2])
        { //y
            k = 1;
        }
        else
        { //z
            k = 2;
        }
        ASSERT(depth[k] >= 0.0f);

        result.depth = depth[k] + sphereB.Radius();

        result.penetrate = true;

        result.collisionFrame[2] = transA.rot[k] * flag1[k] * -1.0f;

        result.contactB = transB.pos + result.collisionFrame[2] * sphereB.Radius();

        result.contactA = result.contactB - result.collisionFrame[2] * result.depth;
    }
    else if (configuration == 0)
    {
        penetration = sphereB.Radius() + boxA.BoxSize(dir) - sphereCenterAbs[dir];

        if (penetration > 0.0f)
        {
            result.depth = penetration;

            result.penetrate = true;

            result.collisionFrame[2] = transA.rot[dir] * flag1[dir] * -1.0f;

            result.contactB = transB.pos + result.collisionFrame[2] * sphereB.Radius();

            result.contactA = result.contactB - result.collisionFrame[2] * penetration;
        }
    }
    else if (configuration == 1)
    {
        s32 dir1, dir2;

        dir1 = neNextDim1[dir];

        dir2 = neNextDim2[dir];

        contactA[dir] = sphereCenter[dir];

        contactA[dir1] = flag1[dir1] * boxA.BoxSize(dir1);

        contactA[dir2] = flag1[dir2] * boxA.BoxSize(dir2);

        neV3 sub = contactA - sphereCenter;

        f32 lenSq = sub[dir1] * sub[dir1] + 
                    sub[dir2] * sub[dir2];

        if (lenSq > sphereB.RadiusSq())
            return;
        
        f32 len = sqrtf(lenSq);

        sub *= 1.0f / len;

        penetration = sphereB.Radius() - len;

        ASSERT(penetration > 0.0f);

        result.depth = penetration;

        result.penetrate = true;

        result.collisionFrame[2] = transA.rot * sub;

        result.contactA = transA * contactA;

        result.contactB = transB.pos + result.collisionFrame[2] * sphereB.Radius();
    }
    else if (configuration == 2)
    {
        contactA.SetZero();

        for (s32 i = 0; i < 3; i++)
            contactA[i] += flag1[i] * boxA.BoxSize(i);

        neV3 sub = contactA - sphereCenter;

        f32 lenSq = sub.Dot(sub);

        if (lenSq > sphereB.RadiusSq())
            return;
        
        f32 len = sqrtf(lenSq);

        penetration = sphereB.Radius() - len;

        sub *= 1.0f / len;

        ASSERT(penetration > 0.0f);

        result.depth = penetration;

        result.penetrate = true;

        result.collisionFrame[2] = transA.rot * sub;

        result.contactA = transA * contactA;

        result.contactB = transB.pos + result.collisionFrame[2] * sphereB.Radius();
    }
    return;
}

void Sphere2TerrainTest(neCollisionResult & result, TConvex & sphereA, neT3 & transA, TConvex & terrainB)
{
    neSimpleArray<s32> & _triIndex = *terrainB.as.terrain.triIndex;

    s32 triangleCount = _triIndex.GetUsedCount();

    neArray<neTriangle_> & triangleArray = *terrainB.as.terrain.triangles;

    ConvexTestResult res[2];

    s32 finalTriIndex = -1;
    s32 currentRes = 1;
    s32 testRes = 0;

    res[currentRes].depth = -1.0e6f;
    res[currentRes].valid = false;
    res[testRes].depth = 1.0e6f;
    
    s32 terrainMatID = 0;

    for (s32 i = 0; i < triangleCount; i++)
    {
        s32 test = _triIndex[i];

        neTriangle_ * t = &triangleArray[_triIndex[i]];

        TriangleParam triParam;

        triParam.vert[0] = terrainB.vertices[t->indices[0]];
        triParam.vert[1] = terrainB.vertices[t->indices[1]];
        triParam.vert[2] = terrainB.vertices[t->indices[2]];

        triParam.edges[0] = triParam.vert[1] - triParam.vert[0];
        triParam.edges[1] = triParam.vert[2] - triParam.vert[1];
        triParam.edges[2] = triParam.vert[0] - triParam.vert[2];
        triParam.normal = triParam.edges[0].Cross(triParam.edges[1]);
        triParam.normal.Normalize();
        triParam.d = triParam.normal.Dot(triParam.vert[0]);

        if (t->flag == neTriangle::NE_TRI_TRIANGLE)
        {
            if (SphereTriTest(transA.pos, sphereA.Radius(), res[testRes], triParam))
            {
                if (res[testRes].depth > res[currentRes].depth)
                {
                    s32 tmp = testRes;  

                    testRes = currentRes;

                    currentRes = tmp;

                    terrainMatID = t->materialID;

                    finalTriIndex = _triIndex[i];
                }
            }
        }
        else if (t->flag == neTriangle::NE_TRI_HEIGHT_MAP)
        {
        }
        else
        {
            ASSERT(0);
        }
    }
    if (res[currentRes].valid)
    {
        result.penetrate = true;

        result.depth = res[currentRes].depth;

        result.collisionFrame[2] = res[currentRes].contactNormal;

        result.materialIdB = terrainMatID;

        result.contactA = res[currentRes].contactA;

        result.contactB = res[currentRes].contactB;
    }
    else
    {
        result.penetrate = false;
    }
}

void MeasureSphereAndTriEdge(const neV3 & center, f32 radius, ConvexTestResult & result, TriangleParam & tri, s32 whichEdge)
{
    s32 whichVert0, whichVert1;

    whichVert0 = whichEdge;

    whichVert1 = neNextDim1[whichEdge];

    f32 penetrate;
    
    neV3 dir = tri.edges[whichEdge];

    f32 edgeLen = dir.Length();

    if (neIsConsiderZero(edgeLen))
    {
        dir.SetZero();
    }
    else
    {
        dir *= (1.0f / edgeLen);
    }
    neV3 vert2Point = center - tri.vert[whichVert0];
    
    f32 dot = dir.Dot(vert2Point);

    neV3 project = tri.vert[whichVert0] + dot * dir;

    if (dot > 0.0f && dot < edgeLen)
    {
        neV3 diff = center - project;
            
        f32 len = diff.Length();
        
        penetrate = radius - len;

        if (penetrate > 0.0f && penetrate < result.depth && penetrate < radius)
        {
            result.valid = true;

            result.depth = penetrate;

            result.contactNormal = diff * (1.0f / len);

            result.contactA = center - result.contactNormal * radius;

            result.contactB = project;
        }
    }
}

void MeasureSphereAndTriVert(const neV3 & center, f32 radius, ConvexTestResult & result, TriangleParam & tri, s32 whichVert)
{
    neV3 diff = center - tri.vert[whichVert];
        
    f32 len = diff.Length();
    
    f32 penetrate = radius - len;

    if (penetrate > 0.0f)
    {
        result.valid = true;

        result.depth = penetrate;

        result.contactNormal = diff * (1.0f / len);

        result.contactA = center - result.contactNormal * radius;

        result.contactB = tri.vert[whichVert];
    }
}

neBool SphereTriTest(const neV3 & center, f32 radius, ConvexTestResult & result, TriangleParam & tri)
{
    //check sphere and triangle plane
    result.depth = 1.e5f;
    result.valid = false;

    f32 distFromPlane = tri.normal.Dot(center) - tri.d;

    f32 factor = 1.0f;

    if (distFromPlane < 0.0f)
        factor = -1.0f;

    f32 penetrated = radius - distFromPlane * factor;

    if (penetrated <= 0.0f)
        return false;

    neV3 contactB = center - tri.normal * distFromPlane;

    s32 pointInside = tri.IsPointInside(contactB);

    if (pointInside == -1) // inside the triangle
    {
        result.depth = penetrated;

        result.contactA = center - tri.normal * factor * radius; //on the sphere

        result.contactB = contactB;

        result.valid = true;

        result.contactNormal = tri.normal * factor;

        return true;
    }

    switch (pointInside)
    {
    case 0:
        MeasureSphereAndTriVert(center, radius, result, tri, 0);
        break;

    case 1:
        MeasureSphereAndTriEdge(center, radius, result, tri, 0);
        break;

    case 2:
        MeasureSphereAndTriVert(center, radius, result, tri, 1);
        break;

    case 3:
        MeasureSphereAndTriEdge(center, radius, result, tri, 1);
        break;

    case 4:
        MeasureSphereAndTriVert(center, radius, result, tri, 2);
        break;

    case 5:
        MeasureSphereAndTriEdge(center, radius, result, tri, 2);
        break;
    }
    
    return result.valid;
}

void Sphere2SphereTest(neCollisionResult & result, TConvex & sphereA, neT3 & transA, TConvex & sphereB, neT3 & transB)
{
    neV3 sub = transA.pos - transB.pos;

    f32 dot = sub.Dot(sub);

    f32 totalLen = sphereA.Radius() + sphereB.Radius();

    totalLen *= totalLen;

    if (dot >= totalLen)
    {
        result.penetrate = false;

        return;
    }

    if (neIsConsiderZero(dot))
    {
        result.penetrate = false;

        return;
    }
    f32 len = sub.Length();

    sub *= 1.0f / len;

    result.depth = sphereA.Radius() + sphereB.Radius() - len;

    ASSERT(result.depth > 0.0f);

    result.penetrate = true;

    result.collisionFrame[2] = sub;

    result.contactA = transA.pos - sub * sphereA.Radius();

    result.contactB = transB.pos + sub * sphereB.Radius();
}


/**stack.cpp**/

//#include "math/ne_type.h"
//#include "math/ne_debug.h"
//#include "tokamak.h"
//#include "containers.h"
//#include "scenery.h"
//#include "collision.h"
//#include "constraint.h"
//#include "rigidbody.h"
/*
#ifdef _WIN32
#include <windows.h>
#endif
*/
//#include "stack.h"
//#include "simulator.h"
//#include "scenery.h"
/*
#include <algorithm>
#include <assert.h>
*/
s32 neStackHeader::golbalTime = 0;

void neRestRecord::Update()
{
    worldThisBody = body->State().b2w * bodyPoint;

    neRigidBody_ * rb = otherBody->AsRigidBody();

    neCollisionBody_ * cb = otherBody->AsCollisionBody();

    if (cb)
    {
        normalWorld = cb->b2w.rot * normalBody;

        worldOtherBody = cb->b2w * otherBodyPoint;
    }
    else
    {
        normalWorld = rb->State().b2w.rot * normalBody;

        worldOtherBody = rb->State().b2w * otherBodyPoint;
    }
    worldDiff = worldThisBody - worldOtherBody;
    
    normalDiff = worldDiff.Dot(normalWorld); // < 0.0f means penetration

    neV3 v = worldDiff;

    v.RemoveComponent(normalWorld); 

    tangentialDiffSq = v.Dot(v);
}

void neStackInfo::Resolve()
{
    isResolved = true;

    if (isTerminator)
        return;

    ASSERT(body);

    for (s32 i = 0; i < NE_RB_MAX_RESTON_RECORDS; i++)
    {
        if (body->GetRestRecord(i).IsValid())
        {
            neRigidBody_ * rb = body->GetRestRecord(i).GetOtherRigidBody();

            if (rb && rb->stackInfo)
            {
                if (!rb->stackInfo->isResolved)
                    rb->stackInfo->Resolve();
            }
        }
    }
    //body->ResolveRestingPenetration();
}

void neStackInfo::CheckHeader(neStackHeader * sh)
{
    ASSERT(stackHeader == sh);

    neRigidBody_ * b = body->AsRigidBody();

    if (!b)
        return;

    for (s32 i = 0; i < NE_RB_MAX_RESTON_RECORDS; i++)
    {
        if (b->GetRestRecord(i).GetOtherRigidBody())
        {
            neRigidBody_ * otherBody = b->GetRestRecord(i).GetOtherRigidBody();
            
            if (otherBody)
            {
                ASSERT(otherBody->stackInfo);

                ASSERT(otherBody->stackInfo->stackHeader == sh);

                if (!otherBody->stackInfo->isTerminator)
                {
                    otherBody->stackInfo->CheckHeader(sh);
                }
            }
        }
    }
}

#if 0
void neStackInfo::Break()
{
/*  char ss [256];
    sprintf(ss, "break %d\n", body->id);
    OutputDebugString(ss);
*/  
    if (stackHeader->isHeaderX)
    {
        stackHeader->Remove(this);

        body->FreeStackInfo();

        return;
    }
    stackHeader->CheckHeader();

    neStackHeader * newHeader = body->sim->NewStackHeader(NULL);

    for (s32 i = 0; i < NE_RB_MAX_RESTON_RECORDS; i++)
    {
        if (body->GetRestRecord(i).otherBody)
        {
            neRigidBody_* b = (neRigidBody_*)body->GetRestRecord(i).otherBody;

            if (b)
            {
                if (!b->stackInfo) //remove from previous iteration
                    continue;
                
                if (b->stackInfo->stackHeader == newHeader) //migrate from previous iteration
                    continue;
                
                if (b->stackInfo->isTerminator)
                {
                    stackHeader->Remove(b->stackInfo);
                    
                    b->FreeStackInfo();

                    b->ResetRestOnRecords();
                }
                else
                {
                    ASSERT(b->stackInfo->stackHeader == stackHeader);

                    b->MigrateNewHeader(newHeader, stackHeader);

                    body->GetRestRecord(i).Init();
                }
            }
        }
    }
    if (newHeader->infoCount == 0)
    {
        body->sim->stackHeaderHeap.Dealloc(newHeader);
    }
    else
    {
        newHeader->CheckHeader();
    }
    if (stackHeader->infoCount == 1)
    {
        ASSERT(stackHeader->head == this);

        stackHeader->infoCount = 0;

        neStackHeader * h = stackHeader;
        
        h->Remove(this);

        body->sim->stackHeaderHeap.Dealloc(h);

        body->FreeStackInfo();
    }
    else
    {
        body->stackInfo->isTerminator = true;

        stackHeader->CheckHeader();
    }
}
#endif

void neStackHeader::Resolve()
{
    // resolve all penetration under this header

//  ASSERT(head);

//  if (head == NULL)
//      return;

    s32 c = 0;

    neStackInfoItem * item = (neStackInfoItem *) head;

    while (item)
    {
        neStackInfo * sinfo = (neStackInfo *) item;

        ASSERT(sinfo->stackHeader == this);

        sinfo->isResolved = false;

        item = item->next;

        c++;
    }

    item = (neStackInfoItem *) head;

    while (item)
    {
        neStackInfo * sinfo = (neStackInfo *) item;

        item = item->next;

        if (!sinfo->isResolved)
            sinfo->Resolve();
    }
}
/*
void neStackHeader::Purge()
{
    if (!head)
        return;

    neStackInfoItem * item = (neStackInfoItem *) head;

    while (item)
    {
        neStackInfo * sinfo = (neStackInfo *) item;
        
        item = item->next;

        sim->stackInfoHeap.Dealloc(sinfo, 1);
    }
    Null();
}
*/
void neStackHeader::ChangeHeader(neStackHeader * newHeader)
{
    if (!head)
    {
        ASSERT(0);
    }
    neStackInfoItem * item = (neStackInfoItem *) head;

    s32 c = 0;

    while (item)
    {
        neStackInfo * sinfo = (neStackInfo *) item;
        
        item = item->next;

        sinfo->stackHeader = newHeader;

        c++;
    }

    ASSERT(c == infoCount);
    
    ASSERT(newHeader->tail);
    
    neStackInfoItem * newTailItem = (neStackInfoItem *) newHeader->tail;
    
    newTailItem->Concat((neStackInfoItem *)head);

    newHeader->tail = tail;

    newHeader->infoCount += c;
}
/*
s32 pop = 0;

neStackHeader * hell[256];
*/
neBool neStackHeader::CheckStackDisconnected()
{
//  OutputDebugString("start\n");
    //neSimpleArray<neStackInfo*, 1000> stackInfoBuffer;

    neSimpleArray<neByte *> & stackInfoBuffer = sim->pointerBuffer2;

    stackInfoBuffer.Clear();

    neStackInfoItem * item = (neStackInfoItem *) head;

    while (item)
    {
        neStackInfo * sinfo = (neStackInfo *) item;

        sinfo->startTime = 0;
        sinfo->endTime = 0;

        item = item->next;

        ASSERT(sinfo->stackHeader == this);

        Remove(sinfo, 1);

        *stackInfoBuffer.Alloc() = (neByte *)sinfo;
    }

    s32 i;

    for (i = 0; i < stackInfoBuffer.GetUsedCount(); i++)
    {
        neStackInfo * sinfo = (neStackInfo *)stackInfoBuffer[i];

        if (sinfo->isBroken)
            continue;

        neRigidBody_ * rb = sinfo->body;

        for (s32 j = 0; j < NE_RB_MAX_RESTON_RECORDS; j++)
        {
            if (rb->GetRestRecord(j).IsValid())
            {
                neRigidBody_ * otherbody = rb->GetRestRecord(j).GetOtherRigidBody();

                if (otherbody)
                {
                    if (otherbody->stackInfo->stackHeader)
                    {
                        if (sinfo->stackHeader)
                        {
                            if (sinfo->stackHeader != otherbody->stackInfo->stackHeader)
                            {
                                // merge
                                neStackHeader * otherHeader = otherbody->stackInfo->stackHeader;

                                otherHeader->ChangeHeader(sinfo->stackHeader);

                                sim->stackHeaderHeap.Dealloc(otherHeader);
                            }
                        }
                        else
                        {
                            otherbody->stackInfo->stackHeader->Add(sinfo);
                        }
                    }
                    else
                    {
                        if (sinfo->stackHeader)
                        {
                            sinfo->stackHeader->Add(otherbody->stackInfo);
                        }
                        else
                        {
                            neStackHeader * newStackHeader = sim->NewStackHeader(NULL);

                            newStackHeader->dynamicSolved = dynamicSolved;

                            newStackHeader->Add(sinfo);

                            newStackHeader->Add(otherbody->stackInfo);
                        }
                    }
                }
            }
        }
    }
    for (i = 0; i < stackInfoBuffer.GetUsedCount(); i++)
    {
        neStackInfo * sinfo = (neStackInfo *)stackInfoBuffer[i];

        if (!sinfo->stackHeader)
        {
            neRigidBody_ * rb = sinfo->body;

            sim->stackInfoHeap.Dealloc(sinfo, 1);

            for (s32 i = 0; i < NE_MAX_REST_ON; i++)
            {
                rb->GetRestRecord(i).SetInvalid();
            }

            rb->stackInfo = NULL;
        }
    }

/*
    item = (neStackInfoItem *) head;

    //pop = 0;

    for (s32 i = 0; i < stackInfoBuffer.GetUsedCount(); i++)
    {
//      char ss[256];

        neStackInfo * sinfo = (neStackInfo *)stackInfoBuffer[i];

//      sprintf(ss, "starting at %d\n", sinfo->body->id);
//      OutputDebugString(ss);

        neStackHeader * newStackHeader = sim->NewStackHeader(NULL);

//      hell[pop] = newStackHeader;
//      pop++;

        neStackHeader * anotherHeader = sinfo->CheckAcceptNewHeader(newStackHeader);

        if (anotherHeader && (anotherHeader != newStackHeader))
        {
            for (s32 j = 0; j < i ; j++)
            {
                ((neStackInfo *)stackInfoBuffer[j])->ForceAcceptNewHeader(anotherHeader);
            }
        }

//      sprintf(ss, "newStackheader %d count = %d\n",pop, newStackHeader->infoCount);
//      OutputDebugString(ss);

        if (newStackHeader->infoCount == 0)
        {
            sim->stackHeaderHeap.Dealloc(newStackHeader);

//          sprintf(ss, "dealloc %d\n",pop);
//          OutputDebugString(ss);
        }
        
    }
    ASSERT(infoCount == 0);

//  sim->stackHeaderHeap.Dealloc(this);
//  sim->CheckStackHeader();
*/  
    return true; // always dealloc this header
}

#if 0

void neStackHeader::AddToSolver()
{
    neStackInfoItem * sitem = (neStackInfoItem *)head;

    while (sitem)
    {
        neStackInfo * sinfo = (neStackInfo*) sitem;

        sitem = sitem->next;

        if (!sinfo->isTerminator)
            sinfo->body->AddContactImpulseRecord(true);

        sinfo->body->needRecalc = true;

        if (!sinfo->body->GetConstraintHeader())
        {
            sinfo->body->SetConstraintHeader(&sinfo->body->sim->contactConstraintHeader);

            sinfo->body->sim->contactConstraintHeader.bodies.Add(&sinfo->body->constraintHeaderItem);
        }
    }
}

#else

void neStackHeader::AddToSolver()
{
    neStackInfoItem * item = (neStackInfoItem *) head;

    while (item)
    {
        neStackInfo * sinfo = (neStackInfo *) item;

        ASSERT(sinfo->stackHeader == this);

        sinfo->isResolved = false;

        item = item->next;
    }
    item = (neStackInfoItem *) head;

    while (item)
    {
        neStackInfo * sinfo = (neStackInfo *) item;

        item = item->next;

        if (!sinfo->isResolved)
        {
            if (!sinfo->body->GetConstraintHeader())
            {
                sinfo->body->SetConstraintHeader(&sinfo->body->sim->contactConstraintHeader);

                sinfo->body->sim->contactConstraintHeader.bodies.Add(&sinfo->body->constraintHeaderItem);
            }
            if (!sinfo->isTerminator)
                sinfo->AddToSolver(true);
        }
    }
}

void neStackHeader::AddToSolverNoConstraintHeader()
{
    neStackInfoItem * item = (neStackInfoItem *) head;

    while (item)
    {
        neStackInfo * sinfo = (neStackInfo *) item;

        ASSERT(sinfo->stackHeader == this);

        sinfo->isResolved = false;

        item = item->next;
    }
    item = (neStackInfoItem *) head;

    while (item)
    {
        neStackInfo * sinfo = (neStackInfo *) item;

        item = item->next;

        if (!sinfo->isResolved)
        {
            if (!sinfo->isTerminator)
                sinfo->AddToSolver(false);
        }
    }
/*  
    neStackInfoItem * sitem = (neStackInfoItem *)head;

    while (sitem)
    {
        neStackInfo * sinfo = (neStackInfo*) sitem;

        sitem = sitem->next;

        if (!sinfo->isTerminator)
            sinfo->body->AddContactImpulseRecord(false);

        sinfo->body->needRecalc = true;
    }
*/  
}

#endif

void neStackInfo::AddToSolver(neBool addCHeader)
{
    isResolved = true;

    ASSERT (!isTerminator);

    ASSERT(body);

//  body->AddContactImpulseRecord(addCHeader);

    for (s32 i = 0; i < NE_RB_MAX_RESTON_RECORDS; i++)
    {
        if (!body->GetRestRecord(i).IsValid())
        {
            continue;
        }
        neRigidBody_ * rb = body->GetRestRecord(i).GetOtherRigidBody();

        if (!rb || !rb->stackInfo)
        {
            continue;
        }
        if (rb->stackInfo->isResolved)
        {
            continue;
        }
        if (!rb->GetConstraintHeader() && addCHeader)
        {
            rb->SetConstraintHeader(&rb->sim->contactConstraintHeader);

            rb->sim->contactConstraintHeader.bodies.Add(&rb->constraintHeaderItem);
        }
        if (!rb->stackInfo->isTerminator)
            rb->stackInfo->AddToSolver(addCHeader);
    }
    body->AddContactImpulseRecord(addCHeader);
}

void neStackHeader::ResetRigidBodyFlag()
{
    neStackInfoItem * sitem = (neStackInfoItem *)head;

    while (sitem)
    {
        neStackInfo * sinfo = (neStackInfo*) sitem;

        sitem = sitem->next;

        sinfo->body->needRecalc = false;
    }       
}

neStackHeader * neStackInfo::CheckAcceptNewHeader(neStackHeader * newHeader)
{
    // this function is for diagnostic only

    if (startTime > 0) // already visited
    {
        return NULL;
    }

    startTime = ++neStackHeader::golbalTime;

    if (stackHeader) //already visited
    {
        if (stackHeader != newHeader)
        {
            return stackHeader;
        }
        else
        {
            return NULL;
        }
    }
    if (isTerminator)
    {
        newHeader->Add(this);

        return NULL;
    }
    if (isBroken)
    {
        newHeader->Add(this);

        isTerminator = true;

        return NULL;
    }
    neBool anotherHeaderFound = false;

    neStackHeader * anotherHeader = NULL;

    neRigidBody_ * foundBody;

    s32 i;

    for (i = 0; i < NE_RB_MAX_RESTON_RECORDS; i++)
    {
        neRigidBody_* otherBody = (neRigidBody_*)body->GetRestRecord(i).GetOtherRigidBody();

        if (!otherBody)
            continue;
/*
        if (otherBody->AsCollisionBody())
        {
            continue;
        }
*/      ASSERT(otherBody->stackInfo);

        anotherHeader = otherBody->stackInfo->CheckAcceptNewHeader(newHeader);

        ASSERT(anotherHeader != newHeader);

        if (anotherHeader != NULL)
        {
            anotherHeaderFound = true;
            foundBody = otherBody;
            break;
        }
    }
    if (anotherHeaderFound)
    {
        anotherHeader->Add(this);

        for (i = 0; i < NE_RB_MAX_RESTON_RECORDS; i++)
        {
            neRigidBody_* otherBody = (neRigidBody_*)body->GetRestRecord(i).GetOtherRigidBody();

            if (!otherBody)
                continue;
/*
            if (otherBody->AsCollisionBody())
                continue;
*/
            if (otherBody != foundBody)
            {
                if (otherBody->stackInfo->stackHeader != anotherHeader)
                    otherBody->stackInfo->ForceAcceptNewHeader(anotherHeader);
            }
        }
        return stackHeader;
    }
    else
    {
        newHeader->Add(this);

        return NULL;
    }
}

void neStackInfo::ForceAcceptNewHeader(neStackHeader * newHeader)
{
    if (isTerminator)
    {
        if (stackHeader)
            stackHeader->Remove(this);

        newHeader->Add(this);

        return;
    }
    if (isBroken)
    {
        if (stackHeader)
            stackHeader->Remove(this);

        newHeader->Add(this);

        return;
    }
    if (stackHeader)
    {
        if (stackHeader == newHeader)
        {
            return;
        }
        stackHeader->Remove(this);
    }
    newHeader->Add(this);

    for (s32 i = 0; i < NE_RB_MAX_RESTON_RECORDS; i++)
    {
        neRigidBody_* otherBody = (neRigidBody_*)body->GetRestRecord(i).GetOtherRigidBody();

        if (!otherBody)
            continue;
/*
        if (otherBody->AsCollisionBody())
        {
            continue;
        }
*/      ASSERT(otherBody->stackInfo);
        
        otherBody->stackInfo->ForceAcceptNewHeader(newHeader);
    }
}


/**tricollision.cpp**/

//#include "tokamak.h"
//#include "containers.h"
//#include "scenery.h"
//#include "collision.h"
//#include "collision2.h"
//#include "dcd.h"
////#include "rigidbody.h"

#include <assert.h>
#include <stdio.h>

extern s32 currentMicroStep;

//extern void DrawLine(const neV3 & colour, neV3 * startpoint, s32 count);

/****************************************************************************
*
*   Box2TriangleTest
*
****************************************************************************/ 

void Box2TriangleTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB)
{
    ConvexTestResult res;

    BoxTestParam boxParamA;

    boxParamA.convex = &convexA;
    boxParamA.trans = &transA;
    boxParamA.radii[0] = transA.rot[0] * convexA.as.box.boxSize[0];
    boxParamA.radii[1] = transA.rot[1] * convexA.as.box.boxSize[1];
    boxParamA.radii[2] = transA.rot[2] * convexA.as.box.boxSize[2];

    TriangleParam triParam;

    triParam.vert[0] = transB * convexB.vertices[convexB.as.tri.indices[0]];
    triParam.vert[1] = transB * convexB.vertices[convexB.as.tri.indices[1]];
    triParam.vert[2] = transB * convexB.vertices[convexB.as.tri.indices[2]];

    triParam.edges[0] = triParam.vert[1] - triParam.vert[0];
    triParam.edges[1] = triParam.vert[2] - triParam.vert[1];
    triParam.edges[2] = triParam.vert[0] - triParam.vert[2];
    triParam.normal = triParam.edges[1].Cross(triParam.edges[0]);
    triParam.normal.Normalize();
    triParam.d = triParam.normal.Dot(triParam.vert[0]);

    if (boxParamA.TriTest(res, triParam))
    {
        result.penetrate = true;

        result.depth = res.depth;

//      result.collisionFrame[0] = res.contactX;
//      result.collisionFrame[1] = res.contactY;
        result.collisionFrame[2] = res.contactNormal;

        if (res.isEdgeEdge)
        {
            result.contactA = res.contactA;

            result.contactB = res.contactB;
        }
        else
        {
            result.contactA = res.contactA;
            
            result.contactB = res.contactA;
        }
    }
    else
    {
        result.penetrate = false;
    }
}

NEINLINE bool TriangleParam::PointInYProjection(neV3 & point)
{
    f32 sign1, sign2;

    neV3 line1 = point - vert[0];

    neV3 line2 = point - vert[1];

    sign1 = line1[2] * edges[2][0] - line1[0] * edges[2][2];

    sign2 = line2[2] * edges[1][0] - line2[0] * edges[1][2];

    f32 mul = sign1 * sign2;

    if (mul < 0.0f)
        return false;
    
    f32 sign3 = line1[2] * edges[0][0] - line1[0] * edges[0][2];

    mul = sign1 * sign3;

    if (mul < 0.0f)
        return false;

    return true;
/*  
    if (normal[1] > 0.0f)
    {
        return (sign1 < 0.0f);
    }
    else
    {
        return (sign1 > 0.0f);
    }
*/
}

s32 TriangleParam::IsPointInside(const neV3 & point)
{
    //select coordinate
    s32 dim0, dim1, plane;
    f32 clockness; // 1.0 counter clockwise, -1.0 clockwise

    if (neAbs(normal[1]) > neAbs(normal[2]))
    {
        if (neAbs(normal[1]) > neAbs(normal[0])) //use y plane
        {
            plane = 1;
            dim0 = 2;//0;
            dim1 = 0;//2;
        }
        else //use x plane
        {
            plane = 0;
            dim0 = 1;
            dim1 = 2;
        }
    }
    else if (neAbs(normal[2]) > neAbs(normal[0])) //use z plane
    {
        plane = 2;
        dim0 = 0;
        dim1 = 1;
    }
    else //use x plane
    {
        plane = 0;
        dim0 = 1;
        dim1 = 2;
    }

    clockness = normal[plane] > 0.0f ? 1.0f : -1.0f;

    f32 det0, det1, det2;

#define pointA (vert[0])
#define pointB (vert[1])
#define pointC (vert[2])

    det0 = (point[dim0] - pointA[dim0]) * (pointA[dim1] - pointB[dim1]) + 
            (pointA[dim1] - point[dim1]) * (pointA[dim0] - pointB[dim0]);
    
    det1 = (point[dim0] - pointB[dim0]) * (pointB[dim1] - pointC[dim1]) + 
            (pointB[dim1] - point[dim1]) * (pointB[dim0] - pointC[dim0]);
    
    det2 = (point[dim0] - pointC[dim0]) * (pointC[dim1] - pointA[dim1]) + 
            (pointC[dim1] - point[dim1]) * (pointC[dim0] - pointA[dim0]);

    s32 ret;

    if (det0 > 0.0f)
    {
        if (det1 > 0.0f)
        {
            if (det2 > 0.0f)
            {
                ret = -1; // inside
            }
            else
            {
                ret = 5; // outside edge 2
            }
        }
        else 
        {
            if (det2 > 0.0f)
            {
                ret = 3; // outside edge 1
            }
            else
            {
                ret = 4; // outside vertex 2
            }
        }
    }
    else 
    {
        if (det1 > 0.0f)
        {
            if (det2 > 0.0f)
            {
                ret = 1; // outside edge 0
            }
            else
            {
                ret = 0; // outside vertex 0
            }
        }
        else
        {
            if (det2 > 0.0f)
            {
                ret = 2; // outside vertex 1
            }
            else
            {
                ret = -1; // inside
            }
        }
    }

    if (ret == -1)
        return ret;

    if (clockness == -1.0f)
    {
        ret = (ret + 3) % 6;
    }

    return ret;

/*
    if (det0 > 0.0f && det1 > 0.0f && det2 > 0.0f)
        return true;

    if (det0 < 0.0f && det1 < 0.0f && det2 < 0.0f)
        return true;

    return false;
*/
}

void TriangleParam::ConputeExtraInfo()
{
    s32 i;

    for (i = 0; i < 3; i++)
    {
        edgeNormals[i] = normal.Cross(edges[i]);

        edgeNormals[i].Normalize();

        neV3 diff = vert[neNextDim2[i]] - vert[i];

        if (diff.Dot(edgeNormals[i]) > 0.0f)
        {
            edgeNormals[i] *= -1.0f;
        }
    }
    for (i = 0; i < 3; i++)
    {
        vertNormals[i] = edgeNormals[i] + edgeNormals[neNextDim2[i]];

        vertNormals[i].Normalize();
    }
}

void TriangleParam::Transform(const TriangleParam & from, neT3 & trans)
{
    s32 i;

    for (i = 0; i < 3; i++)
    {
        vert[i] = trans * from.vert[i];
    }
    for (i = 0; i < 3; i++)
    {
        edges[i] = vert[neNextDim1[i]] - vert[i];
    }
    normal = trans.rot * from.normal;

    d = normal.Dot(vert[i]);
}

bool BoxTestParam::TriHeightTest(ConvexTestResult & result, TriangleParam & tri)
{
    if (!isVertCalc)
        CalcVertInWorld();

    f32 deepest = 0.0f;

    bool found = false;

    for (s32 i = 0; i < 8; i++)
    {
        if (!tri.PointInYProjection(verts[i])) // vert in tri projection
            continue;

        f32 height = tri.d - tri.normal[0] * verts[i][0] - tri.normal[2] * verts[i][2];

        height /= tri.normal[1];

        f32 penetrate = height - verts[i][1];

        if  (penetrate > deepest)
        {
            deepest = penetrate;

            result.depth = penetrate;

            result.contactA = verts[i];

            result.contactB = verts[i];

            result.contactB[1] = height;//verts[i][1] + penetrate;

            result.valid = true;

            result.contactNormal = tri.normal;

            found = true;
        }
    }
    return found;
}

bool BoxTestParam::TriTest(ConvexTestResult & result, TriangleParam & tri)
{
    result.depth = 1.e5f;
    result.isEdgeEdge = false;
    result.valid = false;

    if (!MeasurePlanePenetration(result, tri.normal, tri.d))
        return false;

    ConvexTestResult result0;

    result0.valid = false;
    result0.depth = result.depth;

    if (MeasureBoxFaceTrianglePenetration(result0, tri, 0) &&
        MeasureBoxFaceTrianglePenetration(result0, tri, 1) &&
        MeasureBoxFaceTrianglePenetration(result0, tri, 2)
        )
    {
        if (result0.valid)
        {
            result = result0;
        }
    }
    else
        return false;

    ConvexTestResult result2;

    result2.valid = false;

    result2.depth = result.depth;

    bool edgeCollided = false;

    if (!MeasureBoxEdgeTriangleEdgePenetration(result2, tri, 0, 0))
        return false;
    if (!MeasureBoxEdgeTriangleEdgePenetration(result2, tri, 0, 1))
        return false;
    if (!MeasureBoxEdgeTriangleEdgePenetration(result2, tri, 0, 2))
        return false;
    if (!MeasureBoxEdgeTriangleEdgePenetration(result2, tri, 1, 0))
        return false;
    if (!MeasureBoxEdgeTriangleEdgePenetration(result2, tri, 1, 1))
        return false;
    if (!MeasureBoxEdgeTriangleEdgePenetration(result2, tri, 1, 2))
        return false;
    if (!MeasureBoxEdgeTriangleEdgePenetration(result2, tri, 2, 0))
        return false;
    if (!MeasureBoxEdgeTriangleEdgePenetration(result2, tri, 2, 1))
        return false;
    if (!MeasureBoxEdgeTriangleEdgePenetration(result2, tri, 2, 2))
        return false;

    if (result2.valid)
        edgeCollided = true;

    if (edgeCollided)
    {
        ConvexTestResult result3;

        if (result2.ComputerEdgeContactPoint(result3))
        {
            result.isEdgeEdge = true;
            result.contactA = result3.contactA;
            result.contactB = result3.contactB;
            result.depth = result2.depth;
            //result.contactX = result2.contactX;// * -1.0f;
            //result.contactY = result2.contactY;
            result.contactNormal = result2.contactNormal;// * -1.0f;
        }
        else
        {
            return result.valid;
        }
    }
    else
    {
        return result.valid;
    }

    return true;
}

NEINLINE bool BoxTestParam::MeasurePlanePenetration(ConvexTestResult & result, const neV3 & normal, f32 d)
{
    f32 dot = normal.Dot(trans->pos);

    f32 penetrated = dot - d;

    neV3 contactPoint = trans->pos;

    neV3 contactNormal;

    if (penetrated < 0.0f)
    {   
        contactNormal = normal * -1.0f;
    }
    else 
    {   
        contactNormal = normal;
        penetrated *= -1.0f;
    }
    neV3 progression = contactNormal * radii;

    neV3 sign;

    sign[0] = progression[0] > 0.0f ? 1.0f: -1.0f;
    sign[1] = progression[1] > 0.0f ? 1.0f: -1.0f;
    sign[2] = progression[2] > 0.0f ? 1.0f: -1.0f;

    penetrated += (progression[0] * sign[0]);
    penetrated += (progression[1] * sign[1]);
    penetrated += (progression[2] * sign[2]);

    contactPoint -= (radii[0] * sign[0]);
    contactPoint -= (radii[1] * sign[1]);
    contactPoint -= (radii[2] * sign[2]);
    
    if (penetrated < 0.0f)
        return false;

    if (penetrated < result.depth)
    {
        result.depth = penetrated;
        result.contactA = contactPoint; 
        result.contactB = contactPoint + contactNormal * penetrated;//need to project point onto triangle face
        result.valid = true;
        result.contactNormal = contactNormal;
        //ChooseAxis(result.contactX, result.contactY, result.contactNormal);
    }
    return true;
}

bool BoxTestParam::MeasureBoxFaceTrianglePenetration(ConvexTestResult & result, TriangleParam & tri, s32 whichFace)
{
    neV3 contactNormal = trans->rot[whichFace];

    f32 triMin;
    f32 triMax;
    s32 minVert = 0;
    s32 maxVert = 0;

    triMin = triMax = contactNormal.Dot(tri.vert[0]);

    f32 dot = contactNormal.Dot(tri.vert[1]);

    if (dot < triMin)
    {
        triMin = dot;
        minVert = 1;
    }
    else if (dot > triMax)
    {
        triMax = dot;
        maxVert = 1;
    }
    dot = contactNormal.Dot(tri.vert[2]);

    if (dot < triMin)
    {
        triMin = dot;
        minVert = 2;
    }
    else if (dot > triMax)
    {
        triMax = dot;
        maxVert = 2;
    }
    f32 p = trans->pos.Dot(contactNormal);
    f32 boxMin = p - convex->as.box.boxSize[whichFace];
    f32 boxMax = p + convex->as.box.boxSize[whichFace];

    if (triMin >= boxMax)
        return false;

    if (triMax <= boxMin)
        return false;

    f32 d1 = boxMax - triMin;
    f32 d2 = triMax - boxMin;

    f32 penetrated;
    neV3 contactPoint;
    bool reverse = false;

    if (d1 < d2)
    {
        penetrated = d1;
        contactNormal *= -1.0f;
        contactPoint = tri.vert[minVert];
        reverse = true;
    }
    else
    {
        penetrated = d2;
        contactPoint = tri.vert[maxVert];
    }
    if (penetrated < result.depth)
    {
        s32 otherAxis1 = (whichFace + 1) % 3;
        
        s32 otherAxis2 = (whichFace + 2) % 3;

        result.depth = penetrated;
        result.contactA = contactPoint;
        result.contactB = contactPoint + contactNormal * penetrated;
        result.valid = true;
        result.contactNormal = contactNormal;
        if (reverse)
            result.contactX = trans->rot[otherAxis1] * -1.0f;
        else
            result.contactX = trans->rot[otherAxis1];

        result.contactY = trans->rot[otherAxis2];
    }
    return true;
}

bool BoxTestParam::MeasureBoxEdgeTriangleEdgePenetration(ConvexTestResult & result, TriangleParam & tri, s32 dim1, s32 dim2)
{
    neV3 edgeNormal = tri.edges[dim2];

    edgeNormal.Normalize();

    if (edgeNormal.IsConsiderZero())
        return true;

    neV3 contactNormal = trans->rot[dim1].Cross(edgeNormal);

    if (contactNormal.IsConsiderZero())
        return true;

    contactNormal.Normalize(); // do we need this?

    if (contactNormal.IsConsiderZero())
        return true;

    neV3 contactPoint = trans->pos;

    s32 otherAxis1 = (dim1 + 1) % 3;
    s32 otherAxis2 = (dim1 + 2) % 3;

    f32 p = contactNormal.Dot(contactPoint);

    f32 dot1,dot2;
    f32 sign1, sign2;

    dot1 = contactNormal.Dot(radii[otherAxis1]);
    dot2 = contactNormal.Dot(radii[otherAxis2]);

    f32 boxMin, boxMax;

    sign1 = dot1 < 0.0f ? -1.0f : 1.0f;
    sign2 = dot2 < 0.0f ? -1.0f : 1.0f;

    boxMax = p + dot1 * sign1;
    boxMax += dot2 * sign2;

    boxMin = p - dot1 * sign1;
    boxMin -= dot2 * sign2;

    f32 triMin;
    f32 triMax;
    
    f32 q = contactNormal.Dot(tri.vert[dim2]);
    f32 r = contactNormal.Dot(tri.vert[(dim2+2)%3]);

    if (q < r)
    {
        triMin = q;
        triMax = r;
    }
    else
    {
        triMin = r;
        triMax = q;
    }

    if (triMin >= boxMax || triMax <= boxMin)
        return false;

    f32 penetrated;

    if (triMin == q)
    {
        contactNormal = contactNormal * -1.0f;
        penetrated = boxMax - triMin;

        contactPoint += (radii[otherAxis1] * sign1);
        contactPoint += (radii[otherAxis2] * sign2);
    }
    else
    {
        penetrated = triMax - boxMin;
        
        contactPoint -= (radii[otherAxis1] * sign1);
        contactPoint -= (radii[otherAxis2] * sign2);
    }
    if (penetrated < result.depth)
    {
        result.depth = penetrated;
        result.contactA = contactPoint;
        result.contactB = contactPoint;
        result.valid = true;
        result.contactNormal = contactNormal;
        //ChooseAxis(result.contactX, result.contactY, contactNormal);

        result.edgeA[0] = contactPoint + radii[dim1];
        result.edgeA[1] = contactPoint - radii[dim1];

        result.edgeB[0] = tri.vert[dim2];
        result.edgeB[1] = tri.vert[(dim2+1)%3];
    }
    return true;
}

/****************************************************************************
*
*   Box2TerrainTest
*
****************************************************************************/ 

//static s32 callCnt = 0;

void Box2TerrainTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB)
{
//  Convex2TerrainTest(result, convexA, transA, convexB);

//  return;

    neSimpleArray<s32> & _triIndex = *convexB.as.terrain.triIndex;

    s32 triangleCount = _triIndex.GetUsedCount();

    neArray<neTriangle_> & triangleArray = *convexB.as.terrain.triangles;

    ConvexTestResult res[2];

    BoxTestParam boxParamA;

    boxParamA.convex = &convexA;
    boxParamA.trans = &transA;
    boxParamA.radii[0] = transA.rot[0] * convexA.as.box.boxSize[0];
    boxParamA.radii[1] = transA.rot[1] * convexA.as.box.boxSize[1];
    boxParamA.radii[2] = transA.rot[2] * convexA.as.box.boxSize[2];

    s32 finalTriIndex = -1;
    s32 currentRes = 1;
    s32 testRes = 0;

    res[currentRes].depth = -1.0e6f;
    res[currentRes].valid = false;
    res[testRes].depth = 1.0e6f;
    
    s32 terrainMatID = 0;

    u32 userData = 0;
/*
    callCnt++;

    if (callCnt == 21)
        ASSERT(0);
*/
    for (s32 i = 0; i < triangleCount; i++)
    {
        s32 test = _triIndex[i];

        neTriangle_ * t = &triangleArray[_triIndex[i]];

        TriangleParam triParam;

        triParam.vert[0] = convexB.vertices[t->indices[0]];
        triParam.vert[1] = convexB.vertices[t->indices[1]];
        triParam.vert[2] = convexB.vertices[t->indices[2]];

        triParam.edges[0] = triParam.vert[1] - triParam.vert[0];
        triParam.edges[1] = triParam.vert[2] - triParam.vert[1];
        triParam.edges[2] = triParam.vert[0] - triParam.vert[2];
        triParam.normal = triParam.edges[0].Cross(triParam.edges[1]);
        triParam.normal.Normalize();
        triParam.d = triParam.normal.Dot(triParam.vert[0]);

        if (t->flag == neTriangle::NE_TRI_TRIANGLE)
        {
            if (boxParamA.TriTest(res[testRes], triParam))
            {
                if (res[testRes].depth > res[currentRes].depth)
                {
                    s32 tmp = testRes;  

                    testRes = currentRes;

                    currentRes = tmp;

                    terrainMatID = t->materialID;

                    finalTriIndex = _triIndex[i];

                    userData = t->userData;
                }
            }
        }
        else if (t->flag == neTriangle::NE_TRI_HEIGHT_MAP)
        {
            if (boxParamA.TriHeightTest(res[testRes], triParam))
            {
                if (res[testRes].depth > res[currentRes].depth)
                {
                    s32 tmp = testRes;  

                    testRes = currentRes;

                    currentRes = tmp;

                    terrainMatID = t->materialID;

                    finalTriIndex = _triIndex[i];

                    userData = t->userData;
                }
            }
        }
        else
        {
            ASSERT(0);
        }
    }
    if (res[currentRes].valid)
    {
/*      {
            neV3 points[4];
            neV3 red;

            neTriangle_ * t =  &triangleArray[finalTriIndex];

            points[0] = convexB.vertices[t->indices[0]];
            points[1] = convexB.vertices[t->indices[1]];
            points[2] = convexB.vertices[t->indices[2]];
            points[3] = convexB.vertices[t->indices[0]];

            DrawLine(red, points, 4);
        }
*/      result.penetrate = true;

        result.depth = res[currentRes].depth;

        result.convexB = (TConvex*)(uintptr_t)userData;

        //result.collisionFrame[0] = res[currentRes].contactX;
        //result.collisionFrame[1] = res[currentRes].contactY;
        result.collisionFrame[2] = res[currentRes].contactNormal;

        result.materialIdB = terrainMatID;

        //if (res[currentRes].isEdgeEdge)
        {
            result.contactA = res[currentRes].contactA;

            result.contactB = res[currentRes].contactB;
        }
        //else
        //{
        //  result.contactA = res[currentRes].contactA;
        //  
        //  result.contactB = res[currentRes].contactB;
        //}
    }
    else
    {
        result.penetrate = false;
    }
}


/**useopcode.cpp**/

#include <stdio.h>
//#include "tokamak.h"
//#include "containers.h"
//#include "scenery.h"
//#include "collision.h"
//#include "collision2.h"
//#include "constraint.h"
//#include "rigidbody.h"
//#include "scenery.h"
//#include "stack.h"
//#include "simulator.h"
//#include "message.h"

#ifdef USE_OPCODE

Opcode::AABBTreeCollider tc;

Opcode::BVTCache ColCache;

IceMaths::Matrix4x4 worldA, worldB;

void GetTriangleOverlap(neCollisionResult & result, s32 i, TConvex & convexA, TConvex & convexB)
{
    const Pair * pair = &tc.GetPairs()[i];

    IceMaths::Point * vertsA = convexA.as.opcodeMesh.vertices;

    IndexedTriangle * trisA = convexA.as.opcodeMesh.triIndices;

    IceMaths::Point * vertsB = convexB.as.opcodeMesh.vertices;

    IndexedTriangle * trisB = convexB.as.opcodeMesh.triIndices;

    IceMaths::Point V0 = worldA * vertsA[trisA[pair->id0].mVRef[0]];

    IceMaths::Point V1 = worldA * vertsA[trisA[pair->id0].mVRef[1]];

    IceMaths::Point V2 = worldA * vertsA[trisA[pair->id0].mVRef[2]];

    IceMaths::Point U0 = worldB * vertsB[trisB[pair->id1].mVRef[0]];

    IceMaths::Point U1 = worldB * vertsB[trisB[pair->id1].mVRef[1]];

    IceMaths::Point U2 = worldB * vertsB[trisB[pair->id1].mVRef[2]];

    if (tc.TriTriOverlap(V0, V1, V2, U0, U1, U2))
    {

    }
}

void Box2OpcodeTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB)
{
    
}

void Sphere2OpcodeTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB)
{
}

void Cylinder2OpcodeTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB)
{
}

void Opcode2TerrainTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB)
{
}

void Opcode2OpcodeTest(neCollisionResult & result, TConvex & convexA, neT3 & transA, TConvex & convexB, neT3 & transB)
{
    tc.SetFirstContact(false);
    tc.SetFullBoxBoxTest(true);
    tc.SetFullPrimBoxTest(true);
    tc.SetTemporalCoherence(false);

    tc.SetPointers0(convexA.as.opcodeMesh.triIndices, convexA.as.opcodeMesh.vertices);
    tc.SetPointers1(convexB.as.opcodeMesh.triIndices, convexB.as.opcodeMesh.vertices);

    // Setup cache
    
    ColCache.Model0 = convexA.as.opcodeMesh.opmodel;
    ColCache.Model1 = convexB.as.opcodeMesh.opmodel;
    
    transA.AssignIceMatrix(worldA);
    transB.AssignIceMatrix(worldB);
    // Collision query

    bool IsOk = tc.Collide(ColCache, &worldA, &worldB);

    if (tc.GetContactStatus() == false)
    {
        result.penetrate = false;
        return;
    }

    u32 npairs = tc.GetNbPairs();

    result.penetrate = true;

    result.depth = 0.0f;

    for (u32 i = 0; i < npairs; i++)
    {
        GetTriangleOverlap(result, i, convexA, convexB);
    }
}

#endif //USE_OPCODE

#endif //TOKAMAK_IMPLEMENTATION

#if defined APP_TEST && defined FWK_H
struct CRenderPrimitive {
    int type = 0;
    vec3 dims = {0,0,0};
    unsigned color = ~0u;
    void SetGraphicBox(float x, float y, float z) {
        type = 0;
        dims = vec3(x,y,z);
    }
    void SetGraphicCylinder(float r, float h) {
        type = 1;
        dims = vec3(r,h,0);
    }
    void SetGraphicSphere(float r) {
        type = 2;
        dims = vec3(r,0,0);
    }
    void SetDiffuseColor(vec4 c) {
        color = rgba(c.r * 255, c.g * 255, c.b * 255, c.a * 255);
    }
    void Render(void *device, neT3* v) { // neT3 is a mat33+pos3
        /*
            mat44
            [RGT.x] [RGT.y] [RGT.z] [  ]
            [UPV.x] [UPV.y] [UPV.z] [  ]
            [BCK.x] [BCK.y] [BCK.z] [  ]
            [POS.x] [POS.y] [POS.z] [US]
            RGT,UPV,BCK = right/up/back vectors, POS = position, US = uniform scale
        */

        vec3 pos = ptr3(&v->pos[0]); // M[12],M[13],M[14]

        ddraw_color(color);

        if(type == 0) {
            ddraw_cube33(pos, dims, &v->rot.M[0][0]); // M[0][0]..M[2][2]
        }

        if(type == 1) {
            vec3 upv = norm3(ptr3(&v->rot.M[1][0])); // M[4],M[5],M[6]
            ddraw_circle(pos, upv, dims.x);
        }

        if(type == 2) {
            float radius = dims.x;
#if 1
            ddraw_sphere(pos, radius);
#else
            ddraw_circle(pos, vec3(1,0,0), radius);
            ddraw_circle(pos, vec3(0,1,0), radius);
            ddraw_circle(pos, vec3(0,0,1), radius);

            vec3 facing = norm3(sub3(pos, camera_get_active()->position));
            ddraw_circle(pos, facing, radius);
#endif
        }
    }
    struct mesh_t {
        void Destroy() {}
    } mMesh;
};
camera_t cam;
void appInit();
void  appInput();
void  appTick();
void  appDraw();
void appQuit();
int main() {
    window_create(0.75, 0);

    appInit();

    while(window_swap()) {
        // fps camera
        do_once cam = camera();
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_move(&cam, wasdecq.x,wasdecq.y,wasdecq.z);
        camera_fps(&cam, mouse.x,mouse.y);
        window_cursor( !active );

        // app
        appInput();
        appTick();
        appDraw();
    }

    appQuit();
}
#endif // APP_TEST && FWK_H
