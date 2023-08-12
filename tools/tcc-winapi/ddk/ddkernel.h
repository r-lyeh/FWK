/* $Id$
 *
 * COPYRIGHT:            This file is in the public domain.
 * PROJECT:              ReactOS kernel
 * FILE:
 * PURPOSE:              Directx headers
 * PROGRAMMER:           Magnus Olsen (greatlrd)
 *
 */

#ifndef __DDKM_INCLUDED__
#define __DDKM_INCLUDED__

#if defined (_WIN32) && !defined (_NO_COM)
DEFINE_GUID (IID_IDirectDrawKernel,        0x8D56C120,0x6A08,0x11D0,0x9B,0x06,0x00,0xA0,0xC9,0x03,0xA3,0xB8);
DEFINE_GUID (IID_IDirectDrawSurfaceKernel, 0x60755DA0,0x6A40,0x11D0,0x9B,0x06,0x00,0xA0,0xC9,0x03,0xA3,0xB8);
#endif

typedef struct _DDKERNELCAPS
{
  DWORD dwSize;
  DWORD dwCaps;
  DWORD dwIRQCaps;
} DDKERNELCAPS, *LPDDKERNELCAPS;

#define DDKERNELCAPS_SKIPFIELDS			     0x00000001
#define DDKERNELCAPS_AUTOFLIP		     	 0x00000002
#define DDKERNELCAPS_SETSTATE		     	 0x00000004
#define DDKERNELCAPS_LOCK		     	     0x00000008
#define DDKERNELCAPS_FLIPVIDEOPORT	     	 0x00000010
#define DDKERNELCAPS_FLIPOVERLAY		     0x00000020
#define DDKERNELCAPS_CAPTURE_SYSMEM		     0x00000040
#define DDKERNELCAPS_CAPTURE_NONLOCALVIDMEM	 0x00000080
#define DDKERNELCAPS_FIELDPOLARITY	      	 0x00000100
#define DDKERNELCAPS_CAPTURE_INVERTED		 0x00000200
#define DDIRQ_DISPLAY_VSYNC			         0x00000001
#define DDIRQ_RESERVED1				         0x00000002
#define DDIRQ_VPORT0_VSYNC         			 0x00000004
#define DDIRQ_VPORT0_LINE		         	 0x00000008
#define DDIRQ_VPORT1_VSYNC         			 0x00000010
#define DDIRQ_VPORT1_LINE		         	 0x00000020
#define DDIRQ_VPORT2_VSYNC         			 0x00000040
#define DDIRQ_VPORT2_LINE		         	 0x00000080
#define DDIRQ_VPORT3_VSYNC	         		 0x00000100
#define DDIRQ_VPORT3_LINE		         	 0x00000200
#define DDIRQ_VPORT4_VSYNC         			 0x00000400
#define DDIRQ_VPORT4_LINE         			 0x00000800
#define DDIRQ_VPORT5_VSYNC	         		 0x00001000
#define DDIRQ_VPORT5_LINE         			 0x00002000
#define DDIRQ_VPORT6_VSYNC	         		 0x00004000
#define DDIRQ_VPORT6_LINE	         		 0x00008000
#define DDIRQ_VPORT7_VSYNC	         		 0x00010000
#define DDIRQ_VPORT7_LINE	         		 0x00020000
#define DDIRQ_VPORT8_VSYNC	         		 0x00040000
#define DDIRQ_VPORT8_LINE         			 0x00080000
#define DDIRQ_VPORT9_VSYNC		         	 0x00010000
#define DDIRQ_VPORT9_LINE         			 0x00020000

typedef struct IDirectDrawKernel* LPDIRECTDRAWKERNEL;
typedef struct IDirectDrawSurfaceKernel* LPDIRECTDRAWSURFACEKERNEL;

#if defined(_WIN32) && !defined(_NO_COM)

#undef INTERFACE
#define INTERFACE IDirectDrawKernel
DECLARE_INTERFACE_ (IDirectDrawKernel, IUnknown)
{
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
	STDMETHOD(GetKernelHandle) (THIS_ ULONG*) PURE;
	STDMETHOD(ReleaseKernelHandle) (THIS) PURE;
};

#undef INTERFACE
#define INTERFACE IDirectDrawSurfaceKernel
DECLARE_INTERFACE_ (IDirectDrawSurfaceKernel, IUnknown)
{
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
	STDMETHOD(GetKernelHandle) (THIS_ ULONG*) PURE;
	STDMETHOD(ReleaseKernelHandle) (THIS) PURE;
};

#undef INTERFACE
#endif // defined(_WIN32) && !defined(_NO_COM)

#ifdef __cplusplus
};
#endif

#endif




