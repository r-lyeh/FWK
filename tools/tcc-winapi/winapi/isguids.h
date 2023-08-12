/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the w64 mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within this package.
 */
#ifndef _ISGUIDS_H_
#define _ISGUIDS_H_

#ifdef UNICODE
#define IID_IUniformResourceLocator IID_IUniformResourceLocatorW
#else
#define IID_IUniformResourceLocator IID_IUniformResourceLocatorA
#endif

DEFINE_GUID(CLSID_InternetShortcut,0xFBF23B40L,0xE3F0,0x101B,0x84,0x88,0x00,0xAA,0x00,0x3E,0x56,0xF8);
DEFINE_GUID(IID_IUniformResourceLocatorA,0xFBF23B80L,0xE3F0,0x101B,0x84,0x88,0x00,0xAA,0x00,0x3E,0x56,0xF8);
DEFINE_GUID(IID_IUniformResourceLocatorW,0xCABB0DA0L,0xDA57,0x11CF,0x99,0x74,0x00,0x20,0xAF,0xD7,0x97,0x62);

#endif
