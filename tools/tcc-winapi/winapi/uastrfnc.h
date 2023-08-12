/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the w64 mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within this package.
 */
#ifndef _UASTRFNC_H_
#define _UASTRFNC_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef I_X86_
#define ALIGNMENT_MACHINE
#endif

#ifdef ALIGNMENT_MACHINE
#define IS_ALIGNED(p) (((ULONG_PTR)(p) & (sizeof(*(p))-1))==0)

  UNALIGNED WCHAR *ualstrcpynW(UNALIGNED WCHAR *lpString1,UNALIGNED const WCHAR *lpString2,int iMaxLength);
  int ualstrcmpiW(UNALIGNED const WCHAR *dst,UNALIGNED const WCHAR *src);
  int ualstrcmpW(UNALIGNED const WCHAR *src,UNALIGNED const WCHAR *dst);
  size_t ualstrlenW(UNALIGNED const WCHAR *wcs);
  UNALIGNED WCHAR *ualstrcpyW(UNALIGNED WCHAR *dst,UNALIGNED const WCHAR *src);
#else
#define ualstrcpynW StrCpyNW
#define ualstrcmpiW StrCmpIW
#define ualstrcmpW StrCmpW
#define ualstrlenW lstrlenW
#define ualstrcpyW StrCpyW
#endif

#define ualstrcpynA lstrcpynA
#define ualstrcmpiA lstrcmpiA
#define ualstrcmpA lstrcmpA
#define ualstrlenA lstrlenA
#define ualstrcpyA lstrcpyA

#ifdef UNICODE
#define ualstrcpyn ualstrcpynW
#define ualstrcmpi ualstrcmpiW
#define ualstrcmp ualstrcmpW
#define ualstrlen ualstrlenW
#define ualstrcpy ualstrcpyW
#else
#define ualstrcpyn ualstrcpynA
#define ualstrcmpi ualstrcmpiA
#define ualstrcmp ualstrcmpA
#define ualstrlen ualstrlenA
#define ualstrcpy ualstrcpyA
#endif

#ifdef __cplusplus
}
#endif
#endif
