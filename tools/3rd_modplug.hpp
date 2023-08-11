// ModPlug-XMMS and libmodplug are now in the public domain.

/*
 * This source code is public domain.
 *
 * Authors: Kenton Varda <temporal@gauge3d.org> (C interface wrapper)
 */

#ifndef MODPLUG_H__INCLUDED
#define MODPLUG_H__INCLUDED
#include <stdint.h>
#include <math.h>
#define sinf sin

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
# if defined(MODPLUG_BUILD) && defined(DLL_EXPORT)	/* building libmodplug as a dll for windows */
#   define MODPLUG_EXPORT __declspec(dllexport)
# elif defined(MODPLUG_BUILD) || defined(MODPLUG_STATIC)	/* building or using static libmodplug for windows */
#   define MODPLUG_EXPORT
# else
#   define MODPLUG_EXPORT __declspec(dllimport)			/* using libmodplug dll for windows */
# endif
#elif defined(__OS2__) && defined(__WATCOMC__)
# if defined(MODPLUG_BUILD) && defined(__SW_BD)		/* building libmodplug as a dll for os/2 */
#   define MODPLUG_EXPORT __declspec(dllexport)
# else
#   define MODPLUG_EXPORT					/* using dll or static libmodplug for os/2 */
# endif
#elif defined(MODPLUG_BUILD) && defined(SYM_VISIBILITY)
#   define MODPLUG_EXPORT __attribute__((visibility("default")))
#else
#define MODPLUG_EXPORT
#endif

struct _ModPlugFile;
typedef struct _ModPlugFile ModPlugFile;

struct _ModPlugNote {
	unsigned char Note;
	unsigned char Instrument;
	unsigned char VolumeEffect;
	unsigned char Effect;
	unsigned char Volume;
	unsigned char Parameter;
};
typedef struct _ModPlugNote ModPlugNote;

typedef void (*ModPlugMixerProc)(int*, unsigned long, unsigned long);

/* Load a mod file.  [data] should point to a block of memory containing the complete
 * file, and [size] should be the size of that block.
 * Return the loaded mod file on success, or NULL on failure. */
MODPLUG_EXPORT ModPlugFile* ModPlug_Load(const void* data, int size);
/* Unload a mod file. */
MODPLUG_EXPORT void ModPlug_Unload(ModPlugFile* file);

/* Read sample data into the buffer.  Returns the number of bytes read.  If the end
 * of the mod has been reached, zero is returned. */
MODPLUG_EXPORT int  ModPlug_Read(ModPlugFile* file, void* buffer, int size);

/* Get the name of the mod.  The returned buffer is stored within the ModPlugFile
 * structure and will remain valid until you unload the file. */
MODPLUG_EXPORT const char* ModPlug_GetName(ModPlugFile* file);

/* Get the length of the mod, in milliseconds.  Note that this result is not always
 * accurate, especially in the case of mods with loops. */
MODPLUG_EXPORT int ModPlug_GetLength(ModPlugFile* file);

/* Seek to a particular position in the song.  Note that seeking and MODs don't mix very
 * well.  Some mods will be missing instruments for a short time after a seek, as ModPlug
 * does not scan the sequence backwards to find out which instruments were supposed to be
 * playing at that time.  (Doing so would be difficult and not very reliable.)  Also,
 * note that seeking is not very exact in some mods -- especially those for which
 * ModPlug_GetLength() does not report the full length. */
MODPLUG_EXPORT void ModPlug_Seek(ModPlugFile* file, int millisecond);

enum _ModPlug_Flags
{
	MODPLUG_ENABLE_OVERSAMPLING     = 1 << 0,  /* Enable oversampling (*highly* recommended) */
	MODPLUG_ENABLE_NOISE_REDUCTION  = 1 << 1,  /* Enable noise reduction */
	MODPLUG_ENABLE_REVERB           = 1 << 2,  /* Enable reverb */
	MODPLUG_ENABLE_MEGABASS         = 1 << 3,  /* Enable megabass */
	MODPLUG_ENABLE_SURROUND         = 1 << 4   /* Enable surround sound. */
};

enum _ModPlug_ResamplingMode
{
	MODPLUG_RESAMPLE_NEAREST = 0,  /* No interpolation (very fast, extremely bad sound quality) */
	MODPLUG_RESAMPLE_LINEAR  = 1,  /* Linear interpolation (fast, good quality) */
	MODPLUG_RESAMPLE_SPLINE  = 2,  /* Cubic spline interpolation (high quality) */
	MODPLUG_RESAMPLE_FIR     = 3   /* 8-tap fir filter (extremely high quality) */
};

typedef struct _ModPlug_Settings
{
	int mFlags;  /* One or more of the MODPLUG_ENABLE_* flags above, bitwise-OR'ed */

	/* Note that ModPlug always decodes sound at 44100kHz, 32 bit, stereo and then
	 * down-mixes to the settings you choose. */
	int mChannels;       /* Number of channels - 1 for mono or 2 for stereo */
	int mBits;           /* Bits per sample - 8, 16, or 32 */
	int mFrequency;      /* Sampling rate - 11025, 22050, or 44100 */
	int mResamplingMode; /* One of MODPLUG_RESAMPLE_*, above */

	int mStereoSeparation; /* Stereo separation, 1 - 256 */
	int mMaxMixChannels; /* Maximum number of mixing channels (polyphony), 32 - 256 */

	int mReverbDepth;    /* Reverb level 0(quiet)-100(loud)      */
	int mReverbDelay;    /* Reverb delay in ms, usually 40-200ms */
	int mBassAmount;     /* XBass level 0(quiet)-100(loud)       */
	int mBassRange;      /* XBass cutoff in Hz 10-100            */
	int mSurroundDepth;  /* Surround level 0(quiet)-100(heavy)   */
	int mSurroundDelay;  /* Surround delay in ms, usually 5-40ms */
	int mLoopCount;      /* Number of times to loop.  Zero prevents looping.
			      * -1 loops forever. */
} ModPlug_Settings;

/* Get and set the mod decoder settings.  All options, except for channels, bits-per-sample,
 * sampling rate, and loop count, will take effect immediately.  Those options which don't
 * take effect immediately will take effect the next time you load a mod. */
MODPLUG_EXPORT void ModPlug_GetSettings(ModPlug_Settings* settings);
MODPLUG_EXPORT void ModPlug_SetSettings(const ModPlug_Settings* settings);

/* New ModPlug API Functions */
/* NOTE: Master Volume (1-512) */
MODPLUG_EXPORT unsigned int ModPlug_GetMasterVolume(ModPlugFile* file) ;
MODPLUG_EXPORT void ModPlug_SetMasterVolume(ModPlugFile* file,unsigned int cvol) ;

MODPLUG_EXPORT int ModPlug_GetCurrentSpeed(ModPlugFile* file);
MODPLUG_EXPORT int ModPlug_GetCurrentTempo(ModPlugFile* file);
MODPLUG_EXPORT int ModPlug_GetCurrentOrder(ModPlugFile* file);
MODPLUG_EXPORT int ModPlug_GetCurrentPattern(ModPlugFile* file);
MODPLUG_EXPORT int ModPlug_GetCurrentRow(ModPlugFile* file);
MODPLUG_EXPORT int ModPlug_GetPlayingChannels(ModPlugFile* file);

MODPLUG_EXPORT void ModPlug_SeekOrder(ModPlugFile* file,int order);
MODPLUG_EXPORT int ModPlug_GetModuleType(ModPlugFile* file);
MODPLUG_EXPORT char* ModPlug_GetMessage(ModPlugFile* file);

#define MODPLUG_NO_FILESAVE /* experimental yet.  must match stdafx.h. */
#ifndef MODPLUG_NO_FILESAVE
/*
 * EXPERIMENTAL Export Functions
 */
/*Export to a Scream Tracker 3 S3M module. EXPERIMENTAL (only works on Little-Endian platforms)*/
MODPLUG_EXPORT char ModPlug_ExportS3M(ModPlugFile* file, const char* filepath);

/*Export to an Extended Module (XM). EXPERIMENTAL (only works on Little-Endian platforms)*/
MODPLUG_EXPORT char ModPlug_ExportXM(ModPlugFile* file, const char* filepath);

/*Export to an Amiga MOD file. EXPERIMENTAL.*/
MODPLUG_EXPORT char ModPlug_ExportMOD(ModPlugFile* file, const char* filepath);

/*Export to an Impulse Tracker IT file. Should work OK in Little-Endian & Big-Endian platforms :-) */
MODPLUG_EXPORT char ModPlug_ExportIT(ModPlugFile* file, const char* filepath);
#endif /* MODPLUG_NO_FILESAVE */

MODPLUG_EXPORT unsigned int ModPlug_NumInstruments(ModPlugFile* file);
MODPLUG_EXPORT unsigned int ModPlug_NumSamples(ModPlugFile* file);
MODPLUG_EXPORT unsigned int ModPlug_NumPatterns(ModPlugFile* file);
MODPLUG_EXPORT unsigned int ModPlug_NumChannels(ModPlugFile* file);
MODPLUG_EXPORT unsigned int ModPlug_SampleName(ModPlugFile* file, unsigned int qual, char* buff);
MODPLUG_EXPORT unsigned int ModPlug_InstrumentName(ModPlugFile* file, unsigned int qual, char* buff);

/*
 * Retrieve pattern note-data
 */
MODPLUG_EXPORT ModPlugNote* ModPlug_GetPattern(ModPlugFile* file, int pattern, unsigned int* numrows);

/*
 * =================
 * Mixer callback
 * =================
 *
 * Use this callback if you want to 'modify' the mixed data of LibModPlug.
 * 
 * void proc(int* buffer,unsigned long channels,unsigned long nsamples) ;
 *
 * 'buffer': A buffer of mixed samples
 * 'channels': N. of channels in the buffer
 * 'nsamples': N. of samples in the buffeer (without taking care of n.channels)
 *
 * (Samples are signed 32-bit integers)
 */
MODPLUG_EXPORT void ModPlug_InitMixerCallback(ModPlugFile* file,ModPlugMixerProc proc) ;
MODPLUG_EXPORT void ModPlug_UnloadMixerCallback(ModPlugFile* file) ;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MODPLUG_H__INCLUDED */
#ifdef MODPLUG_CPP		
#pragma once 			
/*
 * This source code is public domain.
 *
 * Authors: Rani Assaf <rani@magic.metawire.com>,
 *          Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (endian and char fixes for PPC)
 */

#ifndef _STDAFX_H_
#define _STDAFX_H_

/* Autoconf detection of stdint/inttypes */
#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
# include "config.h"
# define CONFIG_H_INCLUDED 1
#endif
#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif

/* disable AGC and FILESAVE for all targets for uniformity. */
#define NO_AGC
#define MODPLUG_NO_FILESAVE

#ifdef _WIN32

#ifdef _MSC_VER
#pragma warning (disable:4201)
#pragma warning (disable:4514)
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h> /* for WAVE_FORMAT_PCM */
#include <stdio.h>
#include <malloc.h>
#if defined(_MSC_VER) && (_MSC_VER < 1600)
typedef signed char    int8_t;
typedef signed short   int16_t;
typedef signed int     int32_t;
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
#else
#include <stdint.h>
#endif

#define sleep(_ms)      Sleep(_ms * 1000)

inline void ProcessPlugins(int n) { (void)n; }

#undef strcasecmp
#undef strncasecmp
#define strcasecmp(a,b)     _stricmp(a,b)
#define strncasecmp(a,b,c)  _strnicmp(a,b,c)

#if defined(_MSC_VER) || defined(__MINGW32__)
#define HAVE_SINF 1
#endif

#ifndef isblank
#define isblank(c) ((c) == ' ' || (c) == '\t')
#endif

#else

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

typedef int8_t CHAR;
typedef uint8_t UCHAR;
typedef uint8_t* PUCHAR;
typedef uint16_t USHORT;
typedef uint32_t ULONG;
typedef uint32_t UINT;
typedef uint32_t DWORD;
typedef int32_t LONG;
typedef int64_t LONGLONG;
typedef int32_t* LPLONG;
typedef uint32_t* LPDWORD;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef uint8_t* LPBYTE;
typedef bool BOOL; /* FIXME: must be 'int' */
typedef char* LPSTR;
typedef void* LPVOID;
typedef uint16_t* LPWORD;
typedef const char* LPCSTR;
typedef void* PVOID;
typedef void VOID;

#define LPCTSTR LPCSTR
#define lstrcpyn strncpy
#define lstrcpy strcpy
#define lstrcmp strcmp
#define wsprintf sprintf

#define WAVE_FORMAT_PCM 1

inline void ProcessPlugins(int n) { (void)n; }

#ifndef FALSE
#define FALSE	false
#endif

#ifndef TRUE
#define TRUE	true
#endif

#endif /* _WIN32 */

#if defined(_WIN32) || defined(__CYGWIN__)
# if defined(MODPLUG_BUILD) && defined(DLL_EXPORT)	/* building libmodplug as a dll for windows */
#   define MODPLUG_EXPORT __declspec(dllexport)
# elif defined(MODPLUG_BUILD) || defined(MODPLUG_STATIC)	/* building or using static libmodplug for windows */
#   define MODPLUG_EXPORT
# else
#   define MODPLUG_EXPORT __declspec(dllimport)			/* using libmodplug dll for windows */
# endif
#elif defined(__OS2__) && defined(__WATCOMC__)
# if defined(MODPLUG_BUILD) && defined(__SW_BD)		/* building libmodplug as a dll for os/2 */
#   define MODPLUG_EXPORT __declspec(dllexport)
# else
#   define MODPLUG_EXPORT					/* using dll or static libmodplug for os/2 */
# endif
#elif defined(MODPLUG_BUILD) && defined(SYM_VISIBILITY)
#   define MODPLUG_EXPORT __attribute__((visibility("default")))
#else
#define MODPLUG_EXPORT
#endif


#if defined(_WIN32) && defined(_mm_free)
#undef _mm_free
#endif

#define MMSTREAM            FILE
#define _mm_fopen(name,mode)        fopen(name,mode)
#define _mm_fgets(f,buf,sz)     fgets(buf,sz,f)
#define _mm_fseek(f,pos,whence)     fseek(f,pos,whence)
#define _mm_ftell(f)            ftell(f)
#define _mm_read_UBYTES(buf,sz,f)   fread(buf,sz,1,f)
#define _mm_read_SBYTES(buf,sz,f)   fread(buf,sz,1,f)
#define _mm_feof(f)         feof(f)
#define _mm_fclose(f)           fclose(f)
#define DupStr(h,buf,sz)        strdup(buf)
#define _mm_calloc(h,n,sz)      calloc(n,sz)
#define _mm_recalloc(h,buf,sz,elsz) realloc(buf,sz)
#define _mm_free(h,p)           free(p)

typedef struct {
    char *mm;
    int sz;
    int pos;
    int error;
} MMFILE;

static MMFILE *mmfopen(const char *name, const char *mode)
{
    FILE *fp;
    MMFILE *mmfile = NULL;
    long len;
    if( *mode != 'r' ) return NULL;
    fp = fopen(name, mode);
    if( !fp ) return NULL;
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    if ( len > 0 )
        mmfile = (MMFILE *)malloc(len+sizeof(MMFILE));
    if( !mmfile || len <= 0 ) {
        fclose(fp);
        return NULL;
    }
    fseek(fp, 0, SEEK_SET);
    fread(&mmfile[1],1,len,fp);
    fclose(fp);
    mmfile->mm = (char *)&mmfile[1];
    mmfile->sz = len;
    mmfile->pos = 0;
    return mmfile;
}

static void mmfclose(MMFILE *mmfile)
{
    free(mmfile);
}

static bool mmfeof(MMFILE *mmfile)
{
    if( mmfile->pos < 0 ) return TRUE;
    if( mmfile->pos < mmfile->sz ) return FALSE;
    return TRUE;
}

static int mmfgetc(MMFILE *mmfile)
{
    int b;
    if( mmfeof(mmfile) ) return EOF;
    b = mmfile->mm[mmfile->pos];
    mmfile->pos++;
    if( b=='\r' && !mmfeof(mmfile) && mmfile->mm[mmfile->pos] == '\n' ) {
        b = '\n';
        mmfile->pos++;
    }
    return b;
}

static void mmfgets(char buf[], unsigned int bufsz, MMFILE *mmfile)
{
    int i,b;
    for( i=0; i<(int)bufsz-1; i++ ) {
        b = mmfgetc(mmfile);
        if( b==EOF ) break;
        buf[i] = b;
        if( b == '\n' ) break;
    }
    buf[i] = '\0';
}

static long mmftell(MMFILE *mmfile)
{
    return mmfile->pos;
}

static void mmfseek(MMFILE *mmfile, long p, int whence)
{
    int newpos = mmfile->pos;
    switch(whence) {
        case SEEK_SET:
            newpos = p;
            break;
        case SEEK_CUR:
            newpos += p;
            break;
        case SEEK_END:
            newpos = mmfile->sz + p;
            break;
    }
    if (newpos < mmfile->sz)
        mmfile->pos = newpos;
    else {
        mmfile->error = 1;
//      printf("WARNING: seeking too far\n");
    }
}

static void mmreadUBYTES(BYTE *buf, long sz, MMFILE *mmfile)
{
    int sztr = sz;
    // do not overread.
    if (sz > mmfile->sz - mmfile->pos)
        sztr = mmfile->sz - mmfile->pos;
    memcpy(buf, &mmfile->mm[mmfile->pos], sztr);
    mmfile->pos += sz;
    // if truncated read, populate the rest of the array with zeros.
    if (sz > sztr)
        memset(buf+sztr, 0, sz-sztr);
}

static void mmreadSBYTES(char *buf, long sz, MMFILE *mmfile)
{
    // do not overread.
    if (sz > mmfile->sz - mmfile->pos)
        sz = mmfile->sz - mmfile->pos;
    memcpy(buf, &mmfile->mm[mmfile->pos], sz);
    mmfile->pos += sz;
}


#endif
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (endian and char fixes for PPC)
*/

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
//#include "config.h"
#define CONFIG_H_INCLUDED 1
#endif

#ifndef __SNDFILE_H
#define __SNDFILE_H

#ifdef UNDER_CE
int _strnicmp(const char *str1,const char *str2, int n);
#endif

#ifndef LPCBYTE
typedef const BYTE * LPCBYTE;
#endif

#define MOD_AMIGAC2			0x1AB
#define MAX_SAMPLE_LENGTH	16000000
#define MAX_SAMPLE_RATE		192000
#define MAX_ORDERS			256
#define MAX_PATTERNS		240
#define MAX_SAMPLES			240
#define MAX_INSTRUMENTS		MAX_SAMPLES
#ifdef MODPLUG_FASTSOUNDLIB
#define MAX_CHANNELS		80
#else
#define MAX_CHANNELS		128
#endif
#define MAX_BASECHANNELS	64
#define MAX_ENVPOINTS		32
#define MIN_PERIOD			0x0020
#define MAX_PERIOD			0xFFFF
#define MAX_PATTERNNAME		32
#define MAX_CHANNELNAME		20
#define MAX_INFONAME		80
#define MAX_EQ_BANDS		6
#define MAX_MIXPLUGINS		8


#define MOD_TYPE_NONE		0x00
#define MOD_TYPE_MOD		0x01
#define MOD_TYPE_S3M		0x02
#define MOD_TYPE_XM			0x04
#define MOD_TYPE_MED		0x08
#define MOD_TYPE_MTM		0x10
#define MOD_TYPE_IT			0x20
#define MOD_TYPE_669		0x40
#define MOD_TYPE_ULT		0x80
#define MOD_TYPE_STM		0x100
#define MOD_TYPE_FAR		0x200
#define MOD_TYPE_WAV		0x400
#define MOD_TYPE_AMF		0x800
#define MOD_TYPE_AMS		0x1000
#define MOD_TYPE_DSM		0x2000
#define MOD_TYPE_MDL		0x4000
#define MOD_TYPE_OKT		0x8000
#define MOD_TYPE_MID		0x10000
#define MOD_TYPE_DMF		0x20000
#define MOD_TYPE_PTM		0x40000
#define MOD_TYPE_DBM		0x80000
#define MOD_TYPE_MT2		0x100000
#define MOD_TYPE_AMF0		0x200000
#define MOD_TYPE_PSM		0x400000
#define MOD_TYPE_J2B		0x800000
#define MOD_TYPE_ABC		0x1000000
#define MOD_TYPE_PAT		0x2000000
#define MOD_TYPE_UMX		0x80000000 // Fake type
#define MAX_MODTYPE		24



// Channel flags:
// Bits 0-7:	Sample Flags
#define CHN_16BIT               0x01
#define CHN_LOOP                0x02
#define CHN_PINGPONGLOOP        0x04
#define CHN_SUSTAINLOOP         0x08
#define CHN_PINGPONGSUSTAIN     0x10
#define CHN_PANNING             0x20
#define CHN_STEREO              0x40
#define CHN_PINGPONGFLAG	0x80
// Bits 8-31:	Channel Flags
#define CHN_MUTE                0x100
#define CHN_KEYOFF              0x200
#define CHN_NOTEFADE		0x400
#define CHN_SURROUND            0x800
#define CHN_NOIDO               0x1000
#define CHN_HQSRC               0x2000
#define CHN_FILTER              0x4000
#define CHN_VOLUMERAMP		0x8000
#define CHN_VIBRATO             0x10000
#define CHN_TREMOLO             0x20000
#define CHN_PANBRELLO		0x40000
#define CHN_PORTAMENTO		0x80000
#define CHN_GLISSANDO		0x100000
#define CHN_VOLENV              0x200000
#define CHN_PANENV              0x400000
#define CHN_PITCHENV		0x800000
#define CHN_FASTVOLRAMP		0x1000000
#define CHN_EXTRALOUD		0x2000000
#define CHN_REVERB              0x4000000
#define CHN_NOREVERB		0x8000000


#define ENV_VOLUME              0x0001
#define ENV_VOLSUSTAIN		0x0002
#define ENV_VOLLOOP             0x0004
#define ENV_PANNING             0x0008
#define ENV_PANSUSTAIN		0x0010
#define ENV_PANLOOP             0x0020
#define ENV_PITCH               0x0040
#define ENV_PITCHSUSTAIN	0x0080
#define ENV_PITCHLOOP		0x0100
#define ENV_SETPANNING		0x0200
#define ENV_FILTER              0x0400
#define ENV_VOLCARRY		0x0800
#define ENV_PANCARRY		0x1000
#define ENV_PITCHCARRY		0x2000

#define CMD_NONE                        0
#define CMD_ARPEGGIO			1
#define CMD_PORTAMENTOUP		2
#define CMD_PORTAMENTODOWN		3
#define CMD_TONEPORTAMENTO		4
#define CMD_VIBRATO                     5
#define CMD_TONEPORTAVOL		6
#define CMD_VIBRATOVOL			7
#define CMD_TREMOLO                     8
#define CMD_PANNING8			9
#define CMD_OFFSET                      10
#define CMD_VOLUMESLIDE			11
#define CMD_POSITIONJUMP		12
#define CMD_VOLUME                      13
#define CMD_PATTERNBREAK		14
#define CMD_RETRIG                      15
#define CMD_SPEED                       16
#define CMD_TEMPO                       17
#define CMD_TREMOR                      18
#define CMD_MODCMDEX			19
#define CMD_S3MCMDEX			20
#define CMD_CHANNELVOLUME		21
#define CMD_CHANNELVOLSLIDE		22
#define CMD_GLOBALVOLUME		23
#define CMD_GLOBALVOLSLIDE		24
#define CMD_KEYOFF                      25
#define CMD_FINEVIBRATO			26
#define CMD_PANBRELLO			27
#define CMD_XFINEPORTAUPDOWN            28
#define CMD_PANNINGSLIDE		29
#define CMD_SETENVPOSITION		30
#define CMD_MIDI                        31


// Volume Column commands
#define VOLCMD_VOLUME			1
#define VOLCMD_PANNING			2
#define VOLCMD_VOLSLIDEUP		3
#define VOLCMD_VOLSLIDEDOWN		4
#define VOLCMD_FINEVOLUP		5
#define VOLCMD_FINEVOLDOWN		6
#define VOLCMD_VIBRATOSPEED		7
#define VOLCMD_VIBRATO			8
#define VOLCMD_PANSLIDELEFT		9
#define VOLCMD_PANSLIDERIGHT	        10
#define VOLCMD_TONEPORTAMENTO	        11
#define VOLCMD_PORTAUP			12
#define VOLCMD_PORTADOWN		13

#define RSF_16BIT		0x04
#define RSF_STEREO		0x08

#define RS_PCM8S		0	// 8-bit signed
#define RS_PCM8U		1	// 8-bit unsigned
#define RS_PCM8D		2	// 8-bit delta values
#define RS_ADPCM4		3	// 4-bit ADPCM-packed
#define RS_PCM16D		4	// 16-bit delta values
#define RS_PCM16S		5	// 16-bit signed
#define RS_PCM16U		6	// 16-bit unsigned
#define RS_PCM16M		7	// 16-bit motorola order
#define RS_STPCM8S		(RS_PCM8S|RSF_STEREO)  // stereo 8-bit signed
#define RS_STPCM8U		(RS_PCM8U|RSF_STEREO)  // stereo 8-bit unsigned
#define RS_STPCM8D		(RS_PCM8D|RSF_STEREO)  // stereo 8-bit delta values
#define RS_STPCM16S		(RS_PCM16S|RSF_STEREO) // stereo 16-bit signed
#define RS_STPCM16U		(RS_PCM16U|RSF_STEREO) // stereo 16-bit unsigned
#define RS_STPCM16D		(RS_PCM16D|RSF_STEREO) // stereo 16-bit delta values
#define RS_STPCM16M		(RS_PCM16M|RSF_STEREO) // stereo 16-bit signed big endian
// IT 2.14 compressed samples
#define RS_IT2148		0x10
#define RS_IT21416		0x14
#define RS_IT2158		0x12
#define RS_IT21516		0x16
// AMS Packed Samples
#define RS_AMS8			0x11
#define RS_AMS16		0x15
// DMF Huffman compression
#define RS_DMF8			0x13
#define RS_DMF16		0x17
// MDL Huffman compression
#define RS_MDL8			0x20
#define RS_MDL16		0x24
#define RS_PTM8DTO16	0x25
// Stereo Interleaved Samples
#define RS_STIPCM8S		(RS_PCM8S|0x40|RSF_STEREO)	// stereo 8-bit signed
#define RS_STIPCM8U		(RS_PCM8U|0x40|RSF_STEREO)	// stereo 8-bit unsigned
#define RS_STIPCM16S	(RS_PCM16S|0x40|RSF_STEREO)	// stereo 16-bit signed
#define RS_STIPCM16U	(RS_PCM16U|0x40|RSF_STEREO)	// stereo 16-bit unsigned
#define RS_STIPCM16M	(RS_PCM16M|0x40|RSF_STEREO)	// stereo 16-bit signed big endian
// 24-bit signed
#define RS_PCM24S		(RS_PCM16S|0x80)			// mono 24-bit signed
#define RS_STIPCM24S	(RS_PCM16S|0x80|RSF_STEREO)	// stereo 24-bit signed
#define RS_PCM32S		(RS_PCM16S|0xC0)			// mono 24-bit signed
#define RS_STIPCM32S	(RS_PCM16S|0xC0|RSF_STEREO)	// stereo 24-bit signed

// NNA types
#define NNA_NOTECUT		0
#define NNA_CONTINUE	1
#define NNA_NOTEOFF		2
#define NNA_NOTEFADE	3

// DCT types
#define DCT_NONE		0
#define DCT_NOTE		1
#define DCT_SAMPLE		2
#define DCT_INSTRUMENT	3

// DNA types
#define DNA_NOTECUT		0
#define DNA_NOTEOFF		1
#define DNA_NOTEFADE	2

// Mixer Hardware-Dependent features
#define SYSMIX_ENABLEMMX	0x01
#define SYSMIX_WINDOWSNT	0x02
#define SYSMIX_SLOWCPU		0x04
#define SYSMIX_FASTCPU		0x08

// Module flags
#define SONG_EMBEDMIDICFG	0x0001
#define SONG_FASTVOLSLIDES	0x0002
#define SONG_ITOLDEFFECTS	0x0004
#define SONG_ITCOMPATMODE	0x0008
#define SONG_LINEARSLIDES	0x0010
#define SONG_PATTERNLOOP	0x0020
#define SONG_STEP			0x0040
#define SONG_PAUSED			0x0080
#define SONG_FADINGSONG		0x0100
#define SONG_ENDREACHED		0x0200
#define SONG_GLOBALFADE		0x0400
#define SONG_CPUVERYHIGH	0x0800
#define SONG_FIRSTTICK		0x1000
#define SONG_MPTFILTERMODE	0x2000
#define SONG_SURROUNDPAN	0x4000
#define SONG_EXFILTERRANGE	0x8000
#define SONG_AMIGALIMITS	0x10000

// Global Options (Renderer)
#define SNDMIX_REVERSESTEREO	0x0001
#define SNDMIX_NOISEREDUCTION	0x0002
#define SNDMIX_AGC				0x0004
#define SNDMIX_NORESAMPLING		0x0008
#define SNDMIX_HQRESAMPLER		0x0010
#define SNDMIX_MEGABASS			0x0020
#define SNDMIX_SURROUND			0x0040
#define SNDMIX_REVERB			0x0080
#define SNDMIX_EQ				0x0100
#define SNDMIX_SOFTPANNING		0x0200
#define SNDMIX_ULTRAHQSRCMODE	0x0400
// Misc Flags (can safely be turned on or off)
#define SNDMIX_DIRECTTODISK		0x10000
#define SNDMIX_ENABLEMMX		0x20000
#define SNDMIX_NOBACKWARDJUMPS	0x40000
#define SNDMIX_MAXDEFAULTPAN	0x80000	// Used by the MOD loader


// Reverb Types (GM2 Presets)
enum {
	REVERBTYPE_SMALLROOM,
	REVERBTYPE_MEDIUMROOM,
	REVERBTYPE_LARGEROOM,
	REVERBTYPE_SMALLHALL,
	REVERBTYPE_MEDIUMHALL,
	REVERBTYPE_LARGEHALL,
	NUM_REVERBTYPES
};


enum {
	SRCMODE_NEAREST,
	SRCMODE_LINEAR,
	SRCMODE_SPLINE,
	SRCMODE_POLYPHASE,
	NUM_SRC_MODES
};


// Sample Struct
typedef struct _MODINSTRUMENT
{
	UINT nLength,nLoopStart,nLoopEnd;
	UINT nSustainStart, nSustainEnd;
	signed char *pSample;
	UINT nC4Speed;
	WORD nPan;
	WORD nVolume;
	WORD nGlobalVol;
	WORD uFlags;
	signed char RelativeTone;
	signed char nFineTune;
	BYTE nVibType;
	BYTE nVibSweep;
	BYTE nVibDepth;
	BYTE nVibRate;
	CHAR name[22];
} MODINSTRUMENT;


// Instrument Struct
typedef struct _INSTRUMENTHEADER
{
	UINT nFadeOut;
	DWORD dwFlags;
	WORD nGlobalVol;
	WORD nPan;
	WORD VolPoints[MAX_ENVPOINTS];
	WORD PanPoints[MAX_ENVPOINTS];
	WORD PitchPoints[MAX_ENVPOINTS];
	BYTE VolEnv[MAX_ENVPOINTS];
	BYTE PanEnv[MAX_ENVPOINTS];
	BYTE PitchEnv[MAX_ENVPOINTS];
	BYTE Keyboard[128];
	BYTE NoteMap[128];

	BYTE nVolEnv;
	BYTE nPanEnv;
	BYTE nPitchEnv;
	BYTE nVolLoopStart;
	BYTE nVolLoopEnd;
	BYTE nVolSustainBegin;
	BYTE nVolSustainEnd;
	BYTE nPanLoopStart;
	BYTE nPanLoopEnd;
	BYTE nPanSustainBegin;
	BYTE nPanSustainEnd;
	BYTE nPitchLoopStart;
	BYTE nPitchLoopEnd;
	BYTE nPitchSustainBegin;
	BYTE nPitchSustainEnd;
	BYTE nNNA;
	BYTE nDCT;
	BYTE nDNA;
	BYTE nPanSwing;
	BYTE nVolSwing;
	BYTE nIFC;
	BYTE nIFR;
	WORD wMidiBank;
	BYTE nMidiProgram;
	BYTE nMidiChannel;
	BYTE nMidiDrumKey;
	signed char nPPS;
	unsigned char nPPC;
	CHAR name[32];
	CHAR filename[12];
} INSTRUMENTHEADER;


// Channel Struct
typedef struct _MODCHANNEL
{
	// First 32-bytes: Most used mixing information: don't change it
	signed char * pCurrentSample;
	DWORD nPos;
	DWORD nPosLo;	// actually 16-bit
	LONG nInc;		// 16.16
	LONG nRightVol;
	LONG nLeftVol;
	LONG nRightRamp;
	LONG nLeftRamp;
	// 2nd cache line
	DWORD nLength;
	DWORD dwFlags;
	DWORD nLoopStart;
	DWORD nLoopEnd;
	LONG nRampRightVol;
	LONG nRampLeftVol;
	LONG nFilter_Y1, nFilter_Y2, nFilter_Y3, nFilter_Y4;
	LONG nFilter_A0, nFilter_B0, nFilter_B1;
	LONG nROfs, nLOfs;
	LONG nRampLength;
	// Information not used in the mixer
	signed char * pSample;
	LONG nNewRightVol, nNewLeftVol;
	LONG nRealVolume, nRealPan;
	LONG nVolume, nPan, nFadeOutVol;
	LONG nPeriod, nC4Speed, nPortamentoDest;
	INSTRUMENTHEADER *pHeader;
	MODINSTRUMENT *pInstrument;
	DWORD nVolEnvPosition, nPanEnvPosition, nPitchEnvPosition;
	DWORD nMasterChn, nVUMeter;
	LONG nGlobalVol, nInsVol;
	LONG nFineTune, nTranspose;
	LONG nPortamentoSlide, nAutoVibDepth;
	UINT nAutoVibPos, nVibratoPos, nTremoloPos, nPanbrelloPos;
	// 16-bit members
	signed short nVolSwing, nPanSwing;
	// 8-bit members
	BYTE nNote, nNNA;
	BYTE nNewNote, nNewIns, nCommand, nArpeggio;
	BYTE nOldVolumeSlide, nOldFineVolUpDown;
	BYTE nOldPortaUpDown, nOldFinePortaUpDown;
	BYTE nOldPanSlide, nOldChnVolSlide;
	BYTE nVibratoType, nVibratoSpeed, nVibratoDepth;
	BYTE nTremoloType, nTremoloSpeed, nTremoloDepth;
	BYTE nPanbrelloType, nPanbrelloSpeed, nPanbrelloDepth;
	BYTE nOldCmdEx, nOldVolParam, nOldTempo;
	BYTE nOldOffset, nOldHiOffset;
	BYTE nCutOff, nResonance;
	BYTE nRetrigCount, nRetrigParam;
	BYTE nTremorCount, nTremorParam;
	BYTE nPatternLoop, nPatternLoopCount;
	BYTE nRowNote, nRowInstr;
	BYTE nRowVolCmd, nRowVolume;
	BYTE nRowCommand, nRowParam;
	BYTE nLeftVU, nRightVU;
	BYTE nActiveMacro, nPadding;
} MODCHANNEL;


typedef struct _MODCHANNELSETTINGS
{
	UINT nPan;
	UINT nVolume;
	DWORD dwFlags;
	UINT nMixPlugin;
        char szName[MAX_CHANNELNAME];        // changed from CHAR
} MODCHANNELSETTINGS;


typedef struct _MODCOMMAND
{
	BYTE note;
	BYTE instr;
	BYTE volcmd;
	BYTE command;
	BYTE vol;
	BYTE param;
} MODCOMMAND, *LPMODCOMMAND;

////////////////////////////////////////////////////////////////////
// Mix Plugins
#define MIXPLUG_MIXREADY			0x01	// Set when cleared

class MODPLUG_EXPORT IMixPlugin
{
public:
	virtual ~IMixPlugin() {};
	virtual int AddRef() = 0;
	virtual int Release() = 0;
	virtual void SaveAllParameters() = 0;
	virtual void RestoreAllParameters() = 0;
	virtual void Process(float *pOutL, float *pOutR, unsigned long nSamples) = 0;
	virtual void Init(unsigned long nFreq, int bReset) = 0;
	virtual void MidiSend(DWORD dwMidiCode) = 0;
	virtual void MidiCommand(UINT nMidiCh, UINT nMidiProg, UINT note, UINT vol) = 0;
};


#define MIXPLUG_INPUTF_MASTEREFFECT		0x01	// Apply to master mix
#define MIXPLUG_INPUTF_BYPASS			0x02	// Bypass effect
#define MIXPLUG_INPUTF_WETMIX			0x04	// Wet Mix (dry added)

typedef struct _SNDMIXPLUGINSTATE
{
	DWORD dwFlags;					// MIXPLUG_XXXX
	LONG nVolDecayL, nVolDecayR;	// Buffer click removal
	int *pMixBuffer;				// Stereo effect send buffer
	float *pOutBufferL;				// Temp storage for int -> float conversion
	float *pOutBufferR;
} SNDMIXPLUGINSTATE, *PSNDMIXPLUGINSTATE;

typedef struct _SNDMIXPLUGININFO
{
	DWORD dwPluginId1;
	DWORD dwPluginId2;
	DWORD dwInputRouting;	// MIXPLUG_INPUTF_XXXX
	DWORD dwOutputRouting;	// 0=mix 0x80+=fx
	DWORD dwReserved[4];	// Reserved for routing info
	CHAR szName[32];
	CHAR szLibraryName[64];	// original DLL name
} SNDMIXPLUGININFO, *PSNDMIXPLUGININFO; // Size should be 128

typedef struct _SNDMIXPLUGIN
{
	IMixPlugin *pMixPlugin;
	PSNDMIXPLUGINSTATE pMixState;
	ULONG nPluginDataSize;
	PVOID pPluginData;
	SNDMIXPLUGININFO Info;
} SNDMIXPLUGIN, *PSNDMIXPLUGIN;

typedef	BOOL (*PMIXPLUGINCREATEPROC)(PSNDMIXPLUGIN);

////////////////////////////////////////////////////////////////////

enum {
	MIDIOUT_START=0,
	MIDIOUT_STOP,
	MIDIOUT_TICK,
	MIDIOUT_NOTEON,
	MIDIOUT_NOTEOFF,
	MIDIOUT_VOLUME,
	MIDIOUT_PAN,
	MIDIOUT_BANKSEL,
	MIDIOUT_PROGRAM,
};


typedef struct MODMIDICFG
{
        char szMidiGlb[9*32];      // changed from CHAR
        char szMidiSFXExt[16*32];  // changed from CHAR
        char szMidiZXXExt[128*32]; // changed from CHAR
} MODMIDICFG, *LPMODMIDICFG;

#define NOTE_MAX                        120 //Defines maximum notevalue as well as maximum number of notes.

typedef VOID (* LPSNDMIXHOOKPROC)(int *, unsigned long, unsigned long); // buffer, samples, channels



//==============
class MODPLUG_EXPORT CSoundFile
//==============
{
public:	// Static Members
	static UINT m_nXBassDepth;
	static UINT m_nXBassRange;
	static UINT m_nReverbDepth;
	static UINT m_nReverbDelay;
	static UINT gnReverbType;
	static UINT m_nProLogicDepth;
	static UINT m_nProLogicDelay;
	static UINT m_nStereoSeparation;
	static UINT m_nMaxMixChannels;
	static LONG m_nStreamVolume;
	static DWORD gdwSysInfo;
	static DWORD gdwSoundSetup;
	static DWORD gdwMixingFreq;
	static DWORD gnBitsPerSample;
	static DWORD gnChannels;
	static UINT gnAGC;
	static UINT gnVolumeRampSamples;
	static UINT gnVUMeter;
	static UINT gnCPUUsage;
	static LPSNDMIXHOOKPROC gpSndMixHook;
	static PMIXPLUGINCREATEPROC gpMixPluginCreateProc;

public:	// for Editing
	MODCHANNEL Chn[MAX_CHANNELS];					// Channels
	UINT ChnMix[MAX_CHANNELS];						// Channels to be mixed
	MODINSTRUMENT Ins[MAX_SAMPLES];					// Instruments
	INSTRUMENTHEADER *Headers[MAX_INSTRUMENTS];		// Instrument Headers
	MODCHANNELSETTINGS ChnSettings[MAX_BASECHANNELS]; // Channels settings
	MODCOMMAND *Patterns[MAX_PATTERNS];				// Patterns
	WORD PatternSize[MAX_PATTERNS];					// Patterns Lengths
	BYTE Order[MAX_ORDERS];							// Pattern Orders
	MODMIDICFG m_MidiCfg;							// Midi macro config table
	SNDMIXPLUGIN m_MixPlugins[MAX_MIXPLUGINS];		// Mix plugins
	UINT m_nDefaultSpeed, m_nDefaultTempo, m_nDefaultGlobalVolume;
	DWORD m_dwSongFlags;							// Song flags SONG_XXXX
	UINT m_nChannels, m_nMixChannels, m_nMixStat, m_nBufferCount;
	UINT m_nType, m_nSamples, m_nInstruments;
	UINT m_nTickCount, m_nTotalCount, m_nPatternDelay, m_nFrameDelay;
	UINT m_nMusicSpeed, m_nMusicTempo;
	UINT m_nNextRow, m_nRow, m_nNextStartRow;
	UINT m_nPattern,m_nCurrentPattern,m_nNextPattern,m_nRestartPos;
	UINT m_nMasterVolume, m_nGlobalVolume, m_nSongPreAmp;
	UINT m_nFreqFactor, m_nTempoFactor, m_nOldGlbVolSlide;
	LONG m_nMinPeriod, m_nMaxPeriod, m_nRepeatCount, m_nInitialRepeatCount;
	DWORD m_nGlobalFadeSamples, m_nGlobalFadeMaxSamples;
	UINT m_nMaxOrderPosition;
	UINT m_nPatternNames;
	LPSTR m_lpszSongComments, m_lpszPatternNames;
	char m_szNames[MAX_INSTRUMENTS][32];    // changed from CHAR
	CHAR CompressionTable[16];

public:
	CSoundFile();
	~CSoundFile();

public:
	BOOL Create(LPCBYTE lpStream, DWORD dwMemLength=0);
	BOOL Destroy();
	UINT GetType() const { return m_nType; }
	UINT GetNumChannels() const;
	UINT GetLogicalChannels() const { return m_nChannels; }
	BOOL SetMasterVolume(UINT vol, BOOL bAdjustAGC=FALSE);
	UINT GetMasterVolume() const { return m_nMasterVolume; }
	UINT GetNumPatterns() const;
	UINT GetNumInstruments() const;
	UINT GetNumSamples() const { return m_nSamples; }
	UINT GetCurrentPos() const;
	UINT GetCurrentPattern() const { return m_nPattern; }
	UINT GetCurrentOrder() const { return m_nCurrentPattern; }
	UINT GetSongComments(LPSTR s, UINT cbsize, UINT linesize=32);
	UINT GetRawSongComments(LPSTR s, UINT cbsize, UINT linesize=32);
	UINT GetMaxPosition() const;
	void SetCurrentPos(UINT nPos);
	void SetCurrentOrder(UINT nOrder);
	void GetTitle(LPSTR s) const { lstrcpyn(s,m_szNames[0],32); }
	LPCSTR GetTitle() const { return m_szNames[0]; }
	UINT GetSampleName(UINT nSample,LPSTR s=NULL) const;
	UINT GetInstrumentName(UINT nInstr,LPSTR s=NULL) const;
	UINT GetMusicSpeed() const { return m_nMusicSpeed; }
	UINT GetMusicTempo() const { return m_nMusicTempo; }
	DWORD GetLength(BOOL bAdjust, BOOL bTotal=FALSE);
	DWORD GetSongTime() { return GetLength(FALSE, TRUE); }
	void SetRepeatCount(int n) { m_nRepeatCount = n; m_nInitialRepeatCount = n; }
	int GetRepeatCount() const { return m_nRepeatCount; }
	BOOL IsPaused() const {	return (m_dwSongFlags & SONG_PAUSED) ? TRUE : FALSE; }
	void LoopPattern(int nPat, int nRow=0);
	void CheckCPUUsage(UINT nCPU);
	BOOL SetPatternName(UINT nPat, LPCSTR lpszName);
	BOOL GetPatternName(UINT nPat, LPSTR lpszName, UINT cbSize=MAX_PATTERNNAME) const;
	// Module Loaders
	BOOL ReadXM(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadS3M(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadMod(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadMed(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadMTM(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadSTM(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadIT(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL Read669(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadUlt(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadWav(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadDSM(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadFAR(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadAMS(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadAMS2(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadMDL(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadOKT(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadDMF(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadPTM(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadDBM(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadAMF(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadMT2(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadPSM(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadJ2B(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadUMX(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadABC(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL TestABC(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadMID(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL TestMID(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL ReadPAT(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL TestPAT(LPCBYTE lpStream, DWORD dwMemLength);
	// Save Functions
#ifndef MODPLUG_NO_FILESAVE
	UINT WriteSample(FILE *f, MODINSTRUMENT *pins, UINT nFlags, UINT nMaxLen=0);
	BOOL SaveXM(LPCSTR lpszFileName, UINT nPacking=0);
	BOOL SaveS3M(LPCSTR lpszFileName, UINT nPacking=0);
	BOOL SaveMod(LPCSTR lpszFileName, UINT nPacking=0);
	BOOL SaveIT(LPCSTR lpszFileName, UINT nPacking=0);
#endif // MODPLUG_NO_FILESAVE
	// MOD Convert function
	UINT GetBestSaveFormat() const;
	UINT GetSaveFormats() const;
	void ConvertModCommand(MODCOMMAND *) const;
	void S3MConvert(MODCOMMAND *m, BOOL bIT) const;
	void S3MSaveConvert(UINT *pcmd, UINT *pprm, BOOL bIT) const;
	WORD ModSaveCommand(const MODCOMMAND *m, BOOL bXM) const;

public:
	// Real-time sound functions
	VOID ResetChannels();

	UINT Read(LPVOID lpBuffer, UINT cbBuffer);
	UINT CreateStereoMix(int count);
	BOOL FadeSong(UINT msec);
	BOOL GlobalFadeSong(UINT msec);
	UINT GetTotalTickCount() const { return m_nTotalCount; }
	VOID ResetTotalTickCount() { m_nTotalCount = 0; }

public:
	// Mixer Config
	static BOOL InitPlayer(BOOL bReset=FALSE);
	static BOOL SetMixConfig(UINT nStereoSeparation, UINT nMaxMixChannels);
	static BOOL SetWaveConfig(UINT nRate,UINT nBits,UINT nChannels,BOOL bMMX=FALSE);
	static BOOL SetResamplingMode(UINT nMode); // SRCMODE_XXXX
	static BOOL IsStereo() { return (gnChannels > 1) ? TRUE : FALSE; }
	static DWORD GetSampleRate() { return gdwMixingFreq; }
	static DWORD GetBitsPerSample() { return gnBitsPerSample; }
	static DWORD InitSysInfo();
	static DWORD GetSysInfo() { return gdwSysInfo; }
	// AGC
	static BOOL GetAGC() { return (gdwSoundSetup & SNDMIX_AGC) ? TRUE : FALSE; }
	static void SetAGC(BOOL b);
	static void ResetAGC();
	static void ProcessAGC(int count);

	//GCCFIX -- added these functions back in!
	static BOOL SetWaveConfigEx(BOOL bSurround,BOOL bNoOverSampling,BOOL bReverb,BOOL hqido,BOOL bMegaBass,BOOL bNR,BOOL bEQ);
	// DSP Effects
	static void InitializeDSP(BOOL bReset);
	static void ProcessStereoDSP(int count);
	static void ProcessMonoDSP(int count);
	// [Reverb level 0(quiet)-100(loud)], [delay in ms, usually 40-200ms]
	static BOOL SetReverbParameters(UINT nDepth, UINT nDelay);
	// [XBass level 0(quiet)-100(loud)], [cutoff in Hz 10-100]
	static BOOL SetXBassParameters(UINT nDepth, UINT nRange);
	// [Surround level 0(quiet)-100(heavy)] [delay in ms, usually 5-40ms]
	static BOOL SetSurroundParameters(UINT nDepth, UINT nDelay);
public:
	BOOL ReadNote();
	BOOL ProcessRow();
	BOOL ProcessEffects();
	UINT GetNNAChannel(UINT nChn) const;
	void CheckNNA(UINT nChn, UINT instr, int note, BOOL bForceCut);
	void NoteChange(UINT nChn, int note, BOOL bPorta=FALSE, BOOL bResetEnv=TRUE);
	void InstrumentChange(MODCHANNEL *pChn, UINT instr, BOOL bPorta=FALSE,BOOL bUpdVol=TRUE,BOOL bResetEnv=TRUE);
	// Channel Effects
	void PortamentoUp(MODCHANNEL *pChn, UINT param);
	void PortamentoDown(MODCHANNEL *pChn, UINT param);
	void FinePortamentoUp(MODCHANNEL *pChn, UINT param);
	void FinePortamentoDown(MODCHANNEL *pChn, UINT param);
	void ExtraFinePortamentoUp(MODCHANNEL *pChn, UINT param);
	void ExtraFinePortamentoDown(MODCHANNEL *pChn, UINT param);
	void TonePortamento(MODCHANNEL *pChn, UINT param);
	void Vibrato(MODCHANNEL *pChn, UINT param);
	void FineVibrato(MODCHANNEL *pChn, UINT param);
	void VolumeSlide(MODCHANNEL *pChn, UINT param);
	void PanningSlide(MODCHANNEL *pChn, UINT param);
	void ChannelVolSlide(MODCHANNEL *pChn, UINT param);
	void FineVolumeUp(MODCHANNEL *pChn, UINT param);
	void FineVolumeDown(MODCHANNEL *pChn, UINT param);
	void Tremolo(MODCHANNEL *pChn, UINT param);
	void Panbrello(MODCHANNEL *pChn, UINT param);
	void RetrigNote(UINT nChn, UINT param);
	void NoteCut(UINT nChn, UINT nTick);
	void KeyOff(UINT nChn);
	int PatternLoop(MODCHANNEL *, UINT param);
	void ExtendedMODCommands(UINT nChn, UINT param);
	void ExtendedS3MCommands(UINT nChn, UINT param);
	void ExtendedChannelEffect(MODCHANNEL *, UINT param);
	void ProcessMidiMacro(UINT nChn, LPCSTR pszMidiMacro, UINT param=0);
	void SetupChannelFilter(MODCHANNEL *pChn, BOOL bReset, int flt_modifier=256) const;
	// Low-Level effect processing
	void DoFreqSlide(MODCHANNEL *pChn, LONG nFreqSlide);
	// Global Effects
	void SetTempo(UINT param);
	void SetSpeed(UINT param);
	void GlobalVolSlide(UINT param);
	DWORD IsSongFinished(UINT nOrder, UINT nRow) const;
	BOOL IsValidBackwardJump(UINT nStartOrder, UINT nStartRow, UINT nJumpOrder, UINT nJumpRow) const;
	// Read/Write sample functions
	signed char GetDeltaValue(signed char prev, UINT n) const { return (signed char)(prev + CompressionTable[n & 0x0F]); }
	UINT PackSample(int &sample, int next);
	BOOL CanPackSample(LPSTR pSample, UINT nLen, UINT nPacking, BYTE *result=NULL);
	UINT ReadSample(MODINSTRUMENT *pIns, UINT nFlags, LPCSTR pMemFile, DWORD dwMemLength);
	BOOL DestroySample(UINT nSample);
	BOOL DestroyInstrument(UINT nInstr);
	BOOL IsSampleUsed(UINT nSample);
	BOOL IsInstrumentUsed(UINT nInstr);
	BOOL RemoveInstrumentSamples(UINT nInstr);
	UINT DetectUnusedSamples(BOOL *);
	BOOL RemoveSelectedSamples(BOOL *);
	void AdjustSampleLoop(MODINSTRUMENT *pIns);
	// I/O from another sound file
	BOOL ReadInstrumentFromSong(UINT nInstr, CSoundFile *, UINT nSrcInstrument);
	BOOL ReadSampleFromSong(UINT nSample, CSoundFile *, UINT nSrcSample);
	// Period/Note functions
	UINT GetNoteFromPeriod(UINT period) const;
	UINT GetPeriodFromNote(UINT note, int nFineTune, UINT nC4Speed) const;
	UINT GetFreqFromPeriod(UINT period, UINT nC4Speed, int nPeriodFrac=0) const;
	// Misc functions
	MODINSTRUMENT *GetSample(UINT n) { return Ins+n; }
	void ResetMidiCfg();
	UINT MapMidiInstrument(DWORD dwProgram, UINT nChannel, UINT nNote);
	BOOL ITInstrToMPT(const void *p, INSTRUMENTHEADER *penv, UINT trkvers);
	UINT SaveMixPlugins(FILE *f=NULL, BOOL bUpdate=TRUE);
	UINT LoadMixPlugins(const void *pData, UINT nLen);
#ifndef NO_FILTER
	DWORD CutOffToFrequency(UINT nCutOff, int flt_modifier=256) const; // [0-255] => [1-10KHz]
#endif

	// Static helper functions
public:
	static DWORD TransposeToFrequency(int transp, int ftune=0);
	static int FrequencyToTranspose(DWORD freq);
	static void FrequencyToTranspose(MODINSTRUMENT *psmp);

	// System-Dependant functions
public:
	static MODCOMMAND *AllocatePattern(UINT rows, UINT nchns);
	static signed char* AllocateSample(UINT nbytes);
	static void FreePattern(LPVOID pat);
	static void FreeSample(LPVOID p);
	static UINT Normalize24BitBuffer(LPBYTE pbuffer, UINT cbsizebytes, DWORD lmax24, DWORD dwByteInc);
};


// inline DWORD BigEndian(DWORD x) { return ((x & 0xFF) << 24) | ((x & 0xFF00) << 8) | ((x & 0xFF0000) >> 8) | ((x & 0xFF000000) >> 24); }
// inline WORD BigEndianW(WORD x) { return (WORD)(((x >> 8) & 0xFF) | ((x << 8) & 0xFF00)); }


//////////////////////////////////////////////////////////
// WAVE format information

#pragma pack(1)

// Standard IFF chunks IDs
#define IFFID_FORM		0x4d524f46
#define IFFID_RIFF		0x46464952
#define IFFID_WAVE		0x45564157
#define IFFID_LIST		0x5453494C
#define IFFID_INFO		0x4F464E49

// IFF Info fields
#define IFFID_ICOP		0x504F4349
#define IFFID_IART		0x54524149
#define IFFID_IPRD		0x44525049
#define IFFID_INAM		0x4D414E49
#define IFFID_ICMT		0x544D4349
#define IFFID_IENG		0x474E4549
#define IFFID_ISFT		0x54465349
#define IFFID_ISBJ		0x4A425349
#define IFFID_IGNR		0x524E4749
#define IFFID_ICRD		0x44524349

// Wave IFF chunks IDs
#define IFFID_wave		0x65766177
#define IFFID_fmt		0x20746D66
#define IFFID_wsmp		0x706D7377
#define IFFID_pcm		0x206d6370
#define IFFID_data		0x61746164
#define IFFID_smpl		0x6C706D73
#define IFFID_xtra		0x61727478

typedef struct WAVEFILEHEADER
{
	DWORD id_RIFF;		// "RIFF"
	DWORD filesize;		// file length-8
	DWORD id_WAVE;
} WAVEFILEHEADER;


typedef struct WAVEFORMATHEADER
{
	DWORD id_fmt;		// "fmt "
	DWORD hdrlen;		// 16
	WORD format;		// 1
	WORD channels;		// 1:mono, 2:stereo
	DWORD freqHz;		// sampling freq
	DWORD bytessec;		// bytes/sec=freqHz*samplesize
	WORD samplesize;	// sizeof(sample)
	WORD bitspersample;	// bits per sample (8/16)
} WAVEFORMATHEADER;


typedef struct WAVEDATAHEADER
{
	DWORD id_data;		// "data"
	DWORD length;		// length of data
} WAVEDATAHEADER;


typedef struct WAVESMPLHEADER
{
	// SMPL
	DWORD smpl_id;		// "smpl"	-> 0x6C706D73
	DWORD smpl_len;		// length of smpl: 3Ch	(54h with sustain loop)
	DWORD dwManufacturer;
	DWORD dwProduct;
	DWORD dwSamplePeriod;	// 1000000000/freqHz
	DWORD dwBaseNote;	// 3Ch = C-4 -> 60 + RelativeTone
	DWORD dwPitchFraction;
	DWORD dwSMPTEFormat;
	DWORD dwSMPTEOffset;
	DWORD dwSampleLoops;	// number of loops
	DWORD cbSamplerData;
} WAVESMPLHEADER;


typedef struct SAMPLELOOPSTRUCT
{
	DWORD dwIdentifier;
	DWORD dwLoopType;		// 0=normal, 1=bidi
	DWORD dwLoopStart;
	DWORD dwLoopEnd;		// Byte offset ?
	DWORD dwFraction;
	DWORD dwPlayCount;		// Loop Count, 0=infinite
} SAMPLELOOPSTRUCT;


typedef struct WAVESAMPLERINFO
{
	WAVESMPLHEADER wsiHdr;
	SAMPLELOOPSTRUCT wsiLoops[2];
} WAVESAMPLERINFO;


typedef struct WAVELISTHEADER
{
	DWORD list_id;	// "LIST" -> 0x5453494C
	DWORD list_len;
	DWORD info;		// "INFO"
} WAVELISTHEADER;


typedef struct WAVEEXTRAHEADER
{
	DWORD xtra_id;	// "xtra"	-> 0x61727478
	DWORD xtra_len;
	DWORD dwFlags;
	WORD  wPan;
	WORD  wVolume;
	WORD  wGlobalVol;
	WORD  wReserved;
	BYTE nVibType;
	BYTE nVibSweep;
	BYTE nVibDepth;
	BYTE nVibRate;
} WAVEEXTRAHEADER;

#pragma pack()

///////////////////////////////////////////////////////////
// Low-level Mixing functions

#define MIXBUFFERSIZE		512
#define MIXING_ATTENUATION	4
#define MIXING_CLIPMIN		(-0x08000000)
#define MIXING_CLIPMAX		(0x07FFFFFF)
#define VOLUMERAMPPRECISION	12
#define FADESONGDELAY		100
#define EQ_BUFFERSIZE		(MIXBUFFERSIZE)
#define AGC_PRECISION		9
#define AGC_UNITY			(1 << AGC_PRECISION)

// Calling conventions
#if defined(_MSC_VER) && defined(_M_IX86)
#define MPPASMCALL	__cdecl
#define MPPFASTCALL	__fastcall
#else
#define MPPASMCALL
#define MPPFASTCALL
#endif

#define MOD2XMFineTune(k)	((int)( (signed char)((k)<<4) ))
#define XM2MODFineTune(k)	((int)( (k>>4)&0x0f ))

int _muldiv(long a, long b, long c);
int _muldivr(long a, long b, long c);


// Byte swapping functions from the GNU C Library and libsdl

/* Swap bytes in 16 bit value.  */
#ifdef __GNUC__
# define bswap_16(x) \
    (__extension__							      \
     ({ unsigned short int __bsx = (x);					      \
        ((((__bsx) >> 8) & 0xff) | (((__bsx) & 0xff) << 8)); }))
#else
static __inline unsigned short int
bswap_16 (unsigned short int __bsx)
{
  return ((((__bsx) >> 8) & 0xff) | (((__bsx) & 0xff) << 8));
}
#endif

/* Swap bytes in 32 bit value.  */
#ifdef __GNUC__
# define bswap_32(x) \
    (__extension__							      \
     ({ unsigned int __bsx = (x);					      \
        ((((__bsx) & 0xff000000) >> 24) | (((__bsx) & 0x00ff0000) >>  8) |    \
	 (((__bsx) & 0x0000ff00) <<  8) | (((__bsx) & 0x000000ff) << 24)); }))
#else
static __inline unsigned int
bswap_32 (unsigned int __bsx)
{
  return ((((__bsx) & 0xff000000) >> 24) | (((__bsx) & 0x00ff0000) >>  8) |
	  (((__bsx) & 0x0000ff00) <<  8) | (((__bsx) & 0x000000ff) << 24));
}
#endif

#if (defined ARM) && (defined _WIN32_WCE)
static __inline unsigned short int
ARM_get16(const void *data)
{
	unsigned short int s;
	memcpy(&s,data,sizeof(s));
	return s;
}

static __inline unsigned int
ARM_get32(const void *data)
{
	unsigned int s;
	memcpy(&s,data,sizeof(s));
	return s;
}

#define bswapLE16(X) ARM_get16(&X)
#define bswapLE32(X) ARM_get32(&X)
#define bswapBE16(X) bswap_16(ARM_get16(&X))
#define bswapBE32(X) bswap_32(ARM_get32(&X))

// From libsdl
#elif defined(WORDS_BIGENDIAN) && WORDS_BIGENDIAN
#define bswapLE16(X) bswap_16(X)
#define bswapLE32(X) bswap_32(X)
#define bswapBE16(X) (X)
#define bswapBE32(X) (X)
#else
#define bswapLE16(X) (X)
#define bswapLE32(X) (X)
#define bswapBE16(X) bswap_16(X)
#define bswapBE32(X) bswap_32(X)
#endif

#endif
#ifndef _ITDEFS_H_
#define _ITDEFS_H_

#pragma pack(1)

typedef struct tagITFILEHEADER
{
	DWORD id;			// 0x4D504D49
	CHAR songname[26];
	WORD reserved1;		// 0x1004
	WORD ordnum;
	WORD insnum;
	WORD smpnum;
	WORD patnum;
	WORD cwtv;
	WORD cmwt;
	WORD flags;
	WORD special;
	BYTE globalvol;
	BYTE mv;
	BYTE speed;
	BYTE tempo;
	BYTE sep;
	BYTE zero;
	WORD msglength;
	DWORD msgoffset;
	DWORD reserved2;
	BYTE chnpan[64];
	BYTE chnvol[64];
} ITFILEHEADER;


typedef struct tagITENVELOPE
{
	BYTE flags;
	BYTE num;
	BYTE lpb;
	BYTE lpe;
	BYTE slb;
	BYTE sle;
	BYTE data[25*3];
	BYTE reserved;
} ITENVELOPE;

// Old Impulse Instrument Format (cmwt < 0x200)
typedef struct tagITOLDINSTRUMENT
{
	DWORD id;			// IMPI = 0x49504D49
	CHAR filename[12];	// DOS file name
	BYTE zero;
	BYTE flags;
	BYTE vls;
	BYTE vle;
	BYTE sls;
	BYTE sle;
	WORD reserved1;
	WORD fadeout;
	BYTE nna;
	BYTE dnc;
	WORD trkvers;
	BYTE nos;
	BYTE reserved2;
	CHAR name[26];
	WORD reserved3[3];
	BYTE keyboard[240];
	BYTE volenv[200];
	BYTE nodes[50];
} ITOLDINSTRUMENT;


// Impulse Instrument Format
typedef struct tagITINSTRUMENT
{
	DWORD id;
	CHAR filename[12];
	BYTE zero;
	BYTE nna;
	BYTE dct;
	BYTE dca;
	WORD fadeout;
	signed char pps;
	BYTE ppc;
	BYTE gbv;
	BYTE dfp;
	BYTE rv;
	BYTE rp;
	WORD trkvers;
	BYTE nos;
	BYTE reserved1;
	CHAR name[26];
	BYTE ifc;
	BYTE ifr;
	BYTE mch;
	BYTE mpr;
	WORD mbank;
	BYTE keyboard[240];
	ITENVELOPE volenv;
	ITENVELOPE panenv;
	ITENVELOPE pitchenv;
	BYTE dummy[4]; // was 7, but IT v2.17 saves 554 bytes
} ITINSTRUMENT;


// IT Sample Format
typedef struct ITSAMPLESTRUCT
{
	DWORD id;		// 0x53504D49
	CHAR filename[12];
	BYTE zero;
	BYTE gvl;
	BYTE flags;
	BYTE vol;
	CHAR name[26];
	BYTE cvt;
	BYTE dfp;
	DWORD length;
	DWORD loopbegin;
	DWORD loopend;
	DWORD C5Speed;
	DWORD susloopbegin;
	DWORD susloopend;
	DWORD samplepointer;
	BYTE vis;
	BYTE vid;
	BYTE vir;
	BYTE vit;
} ITSAMPLESTRUCT;

#pragma pack()

extern BYTE autovibit2xm[8];
extern BYTE autovibxm2it[8];

#endif
#ifndef LOAD_PAT_H
#define LOAD_PAT_H

#ifdef __cplusplus
extern "C" {
#endif

void pat_init_patnames(void);
void pat_resetsmp(void);
int pat_numinstr(void);
int pat_numsmp(void);
int pat_smptogm(int smp);
int pat_gmtosmp(int gm);
int pat_gm_drumnr(int n);
int pat_gm_drumnote(int n);
const char *pat_gm_name(int gm);
int pat_modnote(int midinote);
int pat_smplooped(int smp);
BOOL PAT_Load_Instruments(void *c);

#ifdef __cplusplus
}
#endif

#endif
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
 */
#pragma once
//#include "stdafx.h"
//#include "sndfile.h"

#ifndef MODPLUG_FASTSOUNDLIB
//#pragma data_seg(".tables")
#endif

static const BYTE ImpulseTrackerPortaVolCmd[16] =
{
	0x00, 0x01, 0x04, 0x08, 0x10, 0x20, 0x40, 0x60,
	0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

// Period table for Protracker octaves 0-5:
static const WORD ProTrackerPeriodTable[6*12] =
{
	1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960,907,
	856,808,762,720,678,640,604,570,538,508,480,453,
	428,404,381,360,339,320,302,285,269,254,240,226,
	214,202,190,180,170,160,151,143,135,127,120,113,
	107,101,95,90,85,80,75,71,67,63,60,56,
	53,50,47,45,42,40,37,35,33,31,30,28
};


static const WORD ProTrackerTunedPeriods[16*12] = 
{
	1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960,907,
	1700,1604,1514,1430,1348,1274,1202,1134,1070,1010,954,900,
	1688,1592,1504,1418,1340,1264,1194,1126,1064,1004,948,894,
	1676,1582,1492,1408,1330,1256,1184,1118,1056,996,940,888,
	1664,1570,1482,1398,1320,1246,1176,1110,1048,990,934,882,
	1652,1558,1472,1388,1310,1238,1168,1102,1040,982,926,874,
	1640,1548,1460,1378,1302,1228,1160,1094,1032,974,920,868,
	1628,1536,1450,1368,1292,1220,1150,1086,1026,968,914,862,
	1814,1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960,
	1800,1700,1604,1514,1430,1350,1272,1202,1134,1070,1010,954,
	1788,1688,1592,1504,1418,1340,1264,1194,1126,1064,1004,948,
	1774,1676,1582,1492,1408,1330,1256,1184,1118,1056,996,940,
	1762,1664,1570,1482,1398,1320,1246,1176,1110,1048,988,934,
	1750,1652,1558,1472,1388,1310,1238,1168,1102,1040,982,926,
	1736,1640,1548,1460,1378,1302,1228,1160,1094,1032,974,920,
	1724,1628,1536,1450,1368,1292,1220,1150,1086,1026,968,914 
};


// S3M C-4 periods
static const WORD FreqS3MTable[16] = 
{
	1712,1616,1524,1440,1356,1280,
	1208,1140,1076,1016,960,907,
	0,0,0,0
};


// S3M FineTune frequencies
static const WORD S3MFineTuneTable[16] = 
{
	7895,7941,7985,8046,8107,8169,8232,8280,
	8363,8413,8463,8529,8581,8651,8723,8757,	// 8363*2^((i-8)/(12*8))
};


// Sinus table
static const int16_t ModSinusTable[64] =
{
	0,12,25,37,49,60,71,81,90,98,106,112,117,122,125,126,
	127,126,125,122,117,112,106,98,90,81,71,60,49,37,25,12,
	0,-12,-25,-37,-49,-60,-71,-81,-90,-98,-106,-112,-117,-122,-125,-126,
	-127,-126,-125,-122,-117,-112,-106,-98,-90,-81,-71,-60,-49,-37,-25,-12
};

// Triangle wave table (ramp down)
static const int16_t ModRampDownTable[64] =
{
	0,-4,-8,-12,-16,-20,-24,-28,-32,-36,-40,-44,-48,-52,-56,-60,
	-64,-68,-72,-76,-80,-84,-88,-92,-96,-100,-104,-108,-112,-116,-120,-124,
	127,123,119,115,111,107,103,99,95,91,87,83,79,75,71,67,
	63,59,55,51,47,43,39,35,31,27,23,19,15,11,7,3
};

// Square wave table
static const int16_t ModSquareTable[64] =
{
	127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
	127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
	-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,
	-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127
};

// Random wave table
static const int16_t ModRandomTable[64] =
{
	98,-127,-43,88,102,41,-65,-94,125,20,-71,-86,-70,-32,-16,-96,
	17,72,107,-5,116,-69,-62,-40,10,-61,65,109,-18,-38,-13,-76,
	-23,88,21,-94,8,106,21,-112,6,109,20,-88,-30,9,-127,118,
	42,-34,89,-4,-51,-72,21,-29,112,123,84,-101,-92,98,-54,-95
};


// volume fade tables for Retrig Note:
static const int8_t retrigTable1[16] =
{ 0, 0, 0, 0, 0, 0, 10, 8, 0, 0, 0, 0, 0, 0, 24, 32 };

static const int8_t retrigTable2[16] =
{ 0, -1, -2, -4, -8, -16, 0, 0, 0, 1, 2, 4, 8, 16, 0, 0 };



static const WORD XMPeriodTable[104] = 
{
	907,900,894,887,881,875,868,862,856,850,844,838,832,826,820,814,
	808,802,796,791,785,779,774,768,762,757,752,746,741,736,730,725,
	720,715,709,704,699,694,689,684,678,675,670,665,660,655,651,646,
	640,636,632,628,623,619,614,610,604,601,597,592,588,584,580,575,
	570,567,563,559,555,551,547,543,538,535,532,528,524,520,516,513,
	508,505,502,498,494,491,487,484,480,477,474,470,467,463,460,457,
	453,450,447,443,440,437,434,431
};


static const uint32_t XMLinearTable[768] = 
{
	535232,534749,534266,533784,533303,532822,532341,531861,
	531381,530902,530423,529944,529466,528988,528511,528034,
	527558,527082,526607,526131,525657,525183,524709,524236,
	523763,523290,522818,522346,521875,521404,520934,520464,
	519994,519525,519057,518588,518121,517653,517186,516720,
	516253,515788,515322,514858,514393,513929,513465,513002,
	512539,512077,511615,511154,510692,510232,509771,509312,
	508852,508393,507934,507476,507018,506561,506104,505647,
	505191,504735,504280,503825,503371,502917,502463,502010,
	501557,501104,500652,500201,499749,499298,498848,498398,
	497948,497499,497050,496602,496154,495706,495259,494812,
	494366,493920,493474,493029,492585,492140,491696,491253,
	490809,490367,489924,489482,489041,488600,488159,487718,
	487278,486839,486400,485961,485522,485084,484647,484210,
	483773,483336,482900,482465,482029,481595,481160,480726,
	480292,479859,479426,478994,478562,478130,477699,477268,
	476837,476407,475977,475548,475119,474690,474262,473834,
	473407,472979,472553,472126,471701,471275,470850,470425,
	470001,469577,469153,468730,468307,467884,467462,467041,
	466619,466198,465778,465358,464938,464518,464099,463681,
	463262,462844,462427,462010,461593,461177,460760,460345,
	459930,459515,459100,458686,458272,457859,457446,457033,
	456621,456209,455797,455386,454975,454565,454155,453745,
	453336,452927,452518,452110,451702,451294,450887,450481,
	450074,449668,449262,448857,448452,448048,447644,447240,
	446836,446433,446030,445628,445226,444824,444423,444022,
	443622,443221,442821,442422,442023,441624,441226,440828,
	440430,440033,439636,439239,438843,438447,438051,437656,
	437261,436867,436473,436079,435686,435293,434900,434508,
	434116,433724,433333,432942,432551,432161,431771,431382,
	430992,430604,430215,429827,429439,429052,428665,428278,
	427892,427506,427120,426735,426350,425965,425581,425197,
	424813,424430,424047,423665,423283,422901,422519,422138,
	421757,421377,420997,420617,420237,419858,419479,419101,
	418723,418345,417968,417591,417214,416838,416462,416086,
	415711,415336,414961,414586,414212,413839,413465,413092,
	412720,412347,411975,411604,411232,410862,410491,410121,
	409751,409381,409012,408643,408274,407906,407538,407170,
	406803,406436,406069,405703,405337,404971,404606,404241,
	403876,403512,403148,402784,402421,402058,401695,401333,
	400970,400609,400247,399886,399525,399165,398805,398445,
	398086,397727,397368,397009,396651,396293,395936,395579,
	395222,394865,394509,394153,393798,393442,393087,392733,
	392378,392024,391671,391317,390964,390612,390259,389907,
	389556,389204,388853,388502,388152,387802,387452,387102,
	386753,386404,386056,385707,385359,385012,384664,384317,
	383971,383624,383278,382932,382587,382242,381897,381552,
	381208,380864,380521,380177,379834,379492,379149,378807,

	378466,378124,377783,377442,377102,376762,376422,376082,
	375743,375404,375065,374727,374389,374051,373714,373377,
	373040,372703,372367,372031,371695,371360,371025,370690,
	370356,370022,369688,369355,369021,368688,368356,368023,
	367691,367360,367028,366697,366366,366036,365706,365376,
	365046,364717,364388,364059,363731,363403,363075,362747,
	362420,362093,361766,361440,361114,360788,360463,360137,
	359813,359488,359164,358840,358516,358193,357869,357547,
	357224,356902,356580,356258,355937,355616,355295,354974,
	354654,354334,354014,353695,353376,353057,352739,352420,
	352103,351785,351468,351150,350834,350517,350201,349885,
	349569,349254,348939,348624,348310,347995,347682,347368,
	347055,346741,346429,346116,345804,345492,345180,344869,
	344558,344247,343936,343626,343316,343006,342697,342388,
	342079,341770,341462,341154,340846,340539,340231,339924,
	339618,339311,339005,338700,338394,338089,337784,337479,
	337175,336870,336566,336263,335959,335656,335354,335051,
	334749,334447,334145,333844,333542,333242,332941,332641,
	332341,332041,331741,331442,331143,330844,330546,330247,
	329950,329652,329355,329057,328761,328464,328168,327872,
	327576,327280,326985,326690,326395,326101,325807,325513,
	325219,324926,324633,324340,324047,323755,323463,323171,
	322879,322588,322297,322006,321716,321426,321136,320846,
	320557,320267,319978,319690,319401,319113,318825,318538,
	318250,317963,317676,317390,317103,316817,316532,316246,
	315961,315676,315391,315106,314822,314538,314254,313971,
	313688,313405,313122,312839,312557,312275,311994,311712,
	311431,311150,310869,310589,310309,310029,309749,309470,
	309190,308911,308633,308354,308076,307798,307521,307243,
	306966,306689,306412,306136,305860,305584,305308,305033,
	304758,304483,304208,303934,303659,303385,303112,302838,
	302565,302292,302019,301747,301475,301203,300931,300660,
	300388,300117,299847,299576,299306,299036,298766,298497,
	298227,297958,297689,297421,297153,296884,296617,296349,
	296082,295815,295548,295281,295015,294749,294483,294217,
	293952,293686,293421,293157,292892,292628,292364,292100,
	291837,291574,291311,291048,290785,290523,290261,289999,
	289737,289476,289215,288954,288693,288433,288173,287913,
	287653,287393,287134,286875,286616,286358,286099,285841,
	285583,285326,285068,284811,284554,284298,284041,283785,
	283529,283273,283017,282762,282507,282252,281998,281743,
	281489,281235,280981,280728,280475,280222,279969,279716,
	279464,279212,278960,278708,278457,278206,277955,277704,
	277453,277203,276953,276703,276453,276204,275955,275706,
	275457,275209,274960,274712,274465,274217,273970,273722,
	273476,273229,272982,272736,272490,272244,271999,271753,
	271508,271263,271018,270774,270530,270286,270042,269798,
	269555,269312,269069,268826,268583,268341,268099,267857 
};


static const int8_t ft2VibratoTable[256] = 
{
	0,-2,-3,-5,-6,-8,-9,-11,-12,-14,-16,-17,-19,-20,-22,-23,
	-24,-26,-27,-29,-30,-32,-33,-34,-36,-37,-38,-39,-41,-42,
	-43,-44,-45,-46,-47,-48,-49,-50,-51,-52,-53,-54,-55,-56,
	-56,-57,-58,-59,-59,-60,-60,-61,-61,-62,-62,-62,-63,-63,
	-63,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-63,-63,
	-63,-62,-62,-62,-61,-61,-60,-60,-59,-59,-58,-57,-56,-56,
	-55,-54,-53,-52,-51,-50,-49,-48,-47,-46,-45,-44,-43,-42,
	-41,-39,-38,-37,-36,-34,-33,-32,-30,-29,-27,-26,-24,-23,
	-22,-20,-19,-17,-16,-14,-12,-11,-9,-8,-6,-5,-3,-2,0,
	2,3,5,6,8,9,11,12,14,16,17,19,20,22,23,24,26,27,29,30,
	32,33,34,36,37,38,39,41,42,43,44,45,46,47,48,49,50,51,
	52,53,54,55,56,56,57,58,59,59,60,60,61,61,62,62,62,63,
	63,63,64,64,64,64,64,64,64,64,64,64,64,63,63,63,62,62,
	62,61,61,60,60,59,59,58,57,56,56,55,54,53,52,51,50,49,
	48,47,46,45,44,43,42,41,39,38,37,36,34,33,32,30,29,27,
	26,24,23,22,20,19,17,16,14,12,11,9,8,6,5,3,2 
};



static const DWORD FineLinearSlideUpTable[16] =
{
	65536, 65595, 65654, 65714,	65773, 65832, 65892, 65951,
	66011, 66071, 66130, 66190, 66250, 66309, 66369, 66429
};


static const DWORD FineLinearSlideDownTable[16] =
{
	65535, 65477, 65418, 65359, 65300, 65241, 65182, 65123,
	65065, 65006, 64947, 64888, 64830, 64772, 64713, 64645
};


static const DWORD LinearSlideUpTable[256] = 
{
	65536, 65773, 66010, 66249, 66489, 66729, 66971, 67213, 
	67456, 67700, 67945, 68190, 68437, 68685, 68933, 69182, 
	69432, 69684, 69936, 70189, 70442, 70697, 70953, 71209, 
	71467, 71725, 71985, 72245, 72507, 72769, 73032, 73296, 
	73561, 73827, 74094, 74362, 74631, 74901, 75172, 75444, 
	75717, 75991, 76265, 76541, 76818, 77096, 77375, 77655, 
	77935, 78217, 78500, 78784, 79069, 79355, 79642, 79930, 
	80219, 80509, 80800, 81093, 81386, 81680, 81976, 82272, 
	82570, 82868, 83168, 83469, 83771, 84074, 84378, 84683, 
	84989, 85297, 85605, 85915, 86225, 86537, 86850, 87164, 
	87480, 87796, 88113, 88432, 88752, 89073, 89395, 89718, 
	90043, 90369, 90695, 91023, 91353, 91683, 92015, 92347, 
	92681, 93017, 93353, 93691, 94029, 94370, 94711, 95053, 
	95397, 95742, 96088, 96436, 96785, 97135, 97486, 97839, 
	98193, 98548, 98904, 99262, 99621, 99981, 100343, 100706, 
	101070, 101435, 101802, 102170, 102540, 102911, 103283, 103657, 
	104031, 104408, 104785, 105164, 105545, 105926, 106309, 106694, 
	107080, 107467, 107856, 108246, 108637, 109030, 109425, 109820, 
	110217, 110616, 111016, 111418, 111821, 112225, 112631, 113038, 
	113447, 113857, 114269, 114682, 115097, 115514, 115931, 116351, 
	116771, 117194, 117618, 118043, 118470, 118898, 119328, 119760, 
	120193, 120628, 121064, 121502, 121941, 122382, 122825, 123269, 
	123715, 124162, 124611, 125062, 125514, 125968, 126424, 126881, 
	127340, 127801, 128263, 128727, 129192, 129660, 130129, 130599, 
	131072, 131546, 132021, 132499, 132978, 133459, 133942, 134426, 
	134912, 135400, 135890, 136381, 136875, 137370, 137866, 138365, 
	138865, 139368, 139872, 140378, 140885, 141395, 141906, 142419, 
	142935, 143451, 143970, 144491, 145014, 145538, 146064, 146593, 
	147123, 147655, 148189, 148725, 149263, 149803, 150344, 150888, 
	151434, 151982, 152531, 153083, 153637, 154192, 154750, 155310, 
	155871, 156435, 157001, 157569, 158138, 158710, 159284, 159860, 
	160439, 161019, 161601, 162186, 162772, 163361, 163952, 164545, 
};


static const DWORD LinearSlideDownTable[256] = 
{
	65536, 65299, 65064, 64830, 64596, 64363, 64131, 63900, 
	63670, 63440, 63212, 62984, 62757, 62531, 62305, 62081, 
	61857, 61634, 61412, 61191, 60970, 60751, 60532, 60314, 
	60096, 59880, 59664, 59449, 59235, 59021, 58809, 58597, 
	58385, 58175, 57965, 57757, 57548, 57341, 57134, 56928, 
	56723, 56519, 56315, 56112, 55910, 55709, 55508, 55308, 
	55108, 54910, 54712, 54515, 54318, 54123, 53928, 53733, 

	53540, 53347, 53154, 52963, 52772, 52582, 52392, 52204, 
	52015, 51828, 51641, 51455, 51270, 51085, 50901, 50717, 
	50535, 50353, 50171, 49990, 49810, 49631, 49452, 49274, 
	49096, 48919, 48743, 48567, 48392, 48218, 48044, 47871, 
	47698, 47526, 47355, 47185, 47014, 46845, 46676, 46508, 
	46340, 46173, 46007, 45841, 45676, 45511, 45347, 45184, 
	45021, 44859, 44697, 44536, 44376, 44216, 44056, 43898, 
	43740, 43582, 43425, 43268, 43112, 42957, 42802, 42648, 
	42494, 42341, 42189, 42037, 41885, 41734, 41584, 41434, 
	41285, 41136, 40988, 40840, 40693, 40546, 40400, 40254, 
	40109, 39965, 39821, 39677, 39534, 39392, 39250, 39108, 
	38967, 38827, 38687, 38548, 38409, 38270, 38132, 37995, 
	37858, 37722, 37586, 37450, 37315, 37181, 37047, 36913, 
	36780, 36648, 36516, 36384, 36253, 36122, 35992, 35862, 
	35733, 35604, 35476, 35348, 35221, 35094, 34968, 34842, 
	34716, 34591, 34466, 34342, 34218, 34095, 33972, 33850, 
	33728, 33606, 33485, 33364, 33244, 33124, 33005, 32886, 
	32768, 32649, 32532, 32415, 32298, 32181, 32065, 31950, 
	31835, 31720, 31606, 31492, 31378, 31265, 31152, 31040, 
	30928, 30817, 30706, 30595, 30485, 30375, 30266, 30157, 
	30048, 29940, 29832, 29724, 29617, 29510, 29404, 29298, 
	29192, 29087, 28982, 28878, 28774, 28670, 28567, 28464, 
	28361, 28259, 28157, 28056, 27955, 27854, 27754, 27654, 
	27554, 27455, 27356, 27257, 27159, 27061, 26964, 26866, 
	26770, 26673, 26577, 26481, 26386, 26291, 26196, 26102, 
};


static const int SpectrumSinusTable[256*2] = 
{
	0, 1, 1, 2, 3, 3, 4, 5, 6, 7, 7, 8, 9, 10, 10, 11, 
	12, 13, 14, 14, 15, 16, 17, 17, 18, 19, 20, 20, 21, 22, 22, 23, 
	24, 25, 25, 26, 27, 28, 28, 29, 30, 30, 31, 32, 32, 33, 34, 34, 
	35, 36, 36, 37, 38, 38, 39, 39, 40, 41, 41, 42, 42, 43, 44, 44, 
	45, 45, 46, 46, 47, 47, 48, 48, 49, 49, 50, 50, 51, 51, 52, 52, 
	53, 53, 53, 54, 54, 55, 55, 55, 56, 56, 57, 57, 57, 58, 58, 58, 
	59, 59, 59, 59, 60, 60, 60, 60, 61, 61, 61, 61, 61, 62, 62, 62, 
	62, 62, 62, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 
	63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 62, 62, 
	62, 62, 62, 62, 61, 61, 61, 61, 61, 60, 60, 60, 60, 59, 59, 59, 
	59, 58, 58, 58, 57, 57, 57, 56, 56, 55, 55, 55, 54, 54, 53, 53, 
	53, 52, 52, 51, 51, 50, 50, 49, 49, 48, 48, 47, 47, 46, 46, 45, 
	45, 44, 44, 43, 42, 42, 41, 41, 40, 39, 39, 38, 38, 37, 36, 36, 
	35, 34, 34, 33, 32, 32, 31, 30, 30, 29, 28, 28, 27, 26, 25, 25, 
	24, 23, 22, 22, 21, 20, 20, 19, 18, 17, 17, 16, 15, 14, 14, 13, 
	12, 11, 10, 10, 9, 8, 7, 7, 6, 5, 4, 3, 3, 2, 1, 0, 
	0, -1, -1, -2, -3, -3, -4, -5, -6, -7, -7, -8, -9, -10, -10, -11, 
	-12, -13, -14, -14, -15, -16, -17, -17, -18, -19, -20, -20, -21, -22, -22, -23, 
	-24, -25, -25, -26, -27, -28, -28, -29, -30, -30, -31, -32, -32, -33, -34, -34, 
	-35, -36, -36, -37, -38, -38, -39, -39, -40, -41, -41, -42, -42, -43, -44, -44, 
	-45, -45, -46, -46, -47, -47, -48, -48, -49, -49, -50, -50, -51, -51, -52, -52, 
	-53, -53, -53, -54, -54, -55, -55, -55, -56, -56, -57, -57, -57, -58, -58, -58, 
	-59, -59, -59, -59, -60, -60, -60, -60, -61, -61, -61, -61, -61, -62, -62, -62, 
	-62, -62, -62, -63, -63, -63, -63, -63, -63, -63, -63, -63, -63, -63, -63, -63, 
	-63, -63, -63, -63, -63, -63, -63, -63, -63, -63, -63, -63, -63, -63, -62, -62, 
	-62, -62, -62, -62, -61, -61, -61, -61, -61, -60, -60, -60, -60, -59, -59, -59, 
	-59, -58, -58, -58, -57, -57, -57, -56, -56, -55, -55, -55, -54, -54, -53, -53, 
	-53, -52, -52, -51, -51, -50, -50, -49, -49, -48, -48, -47, -47, -46, -46, -45, 
	-45, -44, -44, -43, -42, -42, -41, -41, -40, -39, -39, -38, -38, -37, -36, -36, 
	-35, -34, -34, -33, -32, -32, -31, -30, -30, -29, -28, -28, -27, -26, -25, -25, 
	-24, -23, -22, -22, -21, -20, -20, -19, -18, -17, -17, -16, -15, -14, -14, -13, 
	-12, -11, -10, -10, -9, -8, -7, -7, -6, -5, -4, -3, -3, -2, -1, 0, 
};


/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
 *          Markus Fick <webmaster@mark-f.de> spline + fir-resampler
*/

//#include "stdafx.h"
//#include "sndfile.h"
#include <math.h>

#if defined(_MSC_VER) && defined(_M_IX86)
#pragma bss_seg(".modplug")
#endif

// Front Mix Buffer (Also room for interleaved rear mix)
int MixSoundBuffer[MIXBUFFERSIZE*4];

// Reverb Mix Buffer
#ifndef MODPLUG_NO_REVERB
int MixReverbBuffer[MIXBUFFERSIZE*2];
extern UINT gnReverbSend;
#endif

#ifndef MODPLUG_FASTSOUNDLIB
int MixRearBuffer[MIXBUFFERSIZE*2];
float MixFloatBuffer[MIXBUFFERSIZE*2];
#endif

#if defined(_MSC_VER) && defined(_M_IX86)
#pragma bss_seg()
#endif


extern LONG gnDryROfsVol;
extern LONG gnDryLOfsVol;
extern LONG gnRvbROfsVol;
extern LONG gnRvbLOfsVol;

// 4x256 taps polyphase FIR resampling filter
extern short int gFastSinc[];
extern short int gKaiserSinc[]; // 8-taps polyphase
/*
 *-----------------------------------------------------------------------------
 cubic spline interpolation doc,
   (derived from "digital image warping", g. wolberg)

   interpolation polynomial: f(x) = A3*(x-floor(x))**3 + A2*(x-floor(x))**2 +
     A1*(x-floor(x)) + A0

   with Y = equispaced data points (dist=1), YD = first derivates of data points and IP = floor(x)
   the A[0..3] can be found by solving
     A0  = Y[IP]
     A1  = YD[IP]
     A2  = 3*(Y[IP+1]-Y[IP])-2.0*YD[IP]-YD[IP+1]
     A3  = -2.0 * (Y[IP+1]-Y[IP]) + YD[IP] - YD[IP+1]

   with the first derivates as
     YD[IP]    = 0.5 * (Y[IP+1] - Y[IP-1]);
     YD[IP+1]  = 0.5 * (Y[IP+2] - Y[IP])

   the coefs becomes
     A0 = Y[IP]
     A1 = YD[IP]
        =  0.5*(Y[IP+1] - Y[IP-1]);
     A2 =  3.0*(Y[IP+1]-Y[IP])-2.0*YD[IP]-YD[IP+1]
        =  3.0*(Y[IP+1]-Y[IP]) - 0.5*2.0*(Y[IP+1]-Y[IP-1]) - 0.5*(Y[IP+2]-Y[IP])
        =  3.0*Y[IP+1] - 3.0*Y[IP] - Y[IP+1] + Y[IP-1] - 0.5*Y[IP+2] + 0.5*Y[IP]
        = -0.5*Y[IP+2] + 2.0 * Y[IP+1] - 2.5*Y[IP] + Y[IP-1]
        = Y[IP-1] + 2 * Y[IP+1] - 0.5 * (5.0 * Y[IP] + Y[IP+2])
     A3 = -2.0*(Y[IP+1]-Y[IP]) + YD[IP] + YD[IP+1]
        = -2.0*Y[IP+1] + 2.0*Y[IP] + 0.5*(Y[IP+1]-Y[IP-1]) + 0.5*(Y[IP+2]-Y[IP])
        = -2.0*Y[IP+1] + 2.0*Y[IP] + 0.5*Y[IP+1] - 0.5*Y[IP-1] + 0.5*Y[IP+2] - 0.5*Y[IP]
        =  0.5 * Y[IP+2] - 1.5 * Y[IP+1] + 1.5 * Y[IP] - 0.5 * Y[IP-1]
	=  0.5 * (3.0 * (Y[IP] - Y[IP+1]) - Y[IP-1] + YP[IP+2])

   then interpolated data value is (horner rule)
     out = (((A3*x)+A2)*x+A1)*x+A0

   this gives parts of data points Y[IP-1] to Y[IP+2] of
     part       x**3    x**2    x**1    x**0
      Y[IP-1]    -0.5     1      -0.5    0
      Y[IP]       1.5    -2.5     0      1
      Y[IP+1]    -1.5     2       0.5    0
      Y[IP+2]     0.5    -0.5     0      0
 *---------------------------------------------------------------------------
 */
// number of bits used to scale spline coefs
#define SPLINE_QUANTBITS	14
#define SPLINE_QUANTSCALE	(1L<<SPLINE_QUANTBITS)
#define SPLINE_8SHIFT		(SPLINE_QUANTBITS-8)
#define SPLINE_16SHIFT		(SPLINE_QUANTBITS)
// forces coefsset to unity gain
#define SPLINE_CLAMPFORUNITY
// log2(number) of precalculated splines (range is [4..14])
#define SPLINE_FRACBITS 10
#define SPLINE_LUTLEN (1L<<SPLINE_FRACBITS)

class CzCUBICSPLINE
{	public:
		CzCUBICSPLINE( );
		~CzCUBICSPLINE( );
		static signed short lut[4*(1L<<SPLINE_FRACBITS)];
};

signed short CzCUBICSPLINE::lut[4*(1L<<SPLINE_FRACBITS)];

CzCUBICSPLINE::CzCUBICSPLINE( )
{	int _LIi;
	int _LLen		= (1L<<SPLINE_FRACBITS);
	float _LFlen	= 1.0f / (float)_LLen;
	float _LScale	= (float)SPLINE_QUANTSCALE;
	for(_LIi=0;_LIi<_LLen;_LIi++)
	{	float _LCm1, _LC0, _LC1, _LC2;
		float _LX = ((float)_LIi)*_LFlen;
		int _LSum,_LIdx	= _LIi<<2;
		_LCm1 = (float)floor( 0.5 + _LScale*(-0.5*_LX*_LX*_LX + 1.0*_LX*_LX - 0.5*_LX ) );
		_LC0 = (float)floor( 0.5 + _LScale*( 1.5*_LX*_LX*_LX - 2.5*_LX*_LX + 1.0 ) );
		_LC1 = (float)floor( 0.5 + _LScale*(-1.5*_LX*_LX*_LX + 2.0*_LX*_LX + 0.5*_LX ) );
		_LC2 = (float)floor( 0.5 + _LScale*( 0.5*_LX*_LX*_LX - 0.5*_LX*_LX) );
		lut[_LIdx+0] = (signed short)( (_LCm1 < -_LScale) ? -_LScale : ((_LCm1 > _LScale) ? _LScale : _LCm1) );
		lut[_LIdx+1] = (signed short)( (_LC0  < -_LScale) ? -_LScale : ((_LC0  > _LScale) ? _LScale : _LC0 ) );
		lut[_LIdx+2] = (signed short)( (_LC1  < -_LScale) ? -_LScale : ((_LC1  > _LScale) ? _LScale : _LC1 ) );
		lut[_LIdx+3] = (signed short)( (_LC2  < -_LScale) ? -_LScale : ((_LC2  > _LScale) ? _LScale : _LC2 ) );
#ifdef SPLINE_CLAMPFORUNITY
		_LSum = lut[_LIdx+0]+lut[_LIdx+1]+lut[_LIdx+2]+lut[_LIdx+3];
		if( _LSum != SPLINE_QUANTSCALE )
		{	int _LMax = _LIdx;
			if( lut[_LIdx+1]>lut[_LMax] ) _LMax = _LIdx+1;
			if( lut[_LIdx+2]>lut[_LMax] ) _LMax = _LIdx+2;
			if( lut[_LIdx+3]>lut[_LMax] ) _LMax = _LIdx+3;
			lut[_LMax] += ((signed short)SPLINE_QUANTSCALE-_LSum);
		}
#endif
	}
}

CzCUBICSPLINE::~CzCUBICSPLINE( )
{	// nothing todo
}

CzCUBICSPLINE sspline;

/*
  ------------------------------------------------------------------------------
   fir interpolation doc,
     (derived from "an engineer's guide to fir digital filters", n.j. loy)

     calculate coefficients for ideal lowpass filter (with cutoff = fc in
	0..1 (mapped to 0..nyquist))
     c[-N..N] = (i==0) ? fc : sin(fc*pi*i)/(pi*i)

     then apply selected window to coefficients
      c[-N..N] *= w(0..N)
     with n in 2*N and w(n) being a window function (see loy)

     then calculate gain and scale filter coefs to have unity gain.
  ------------------------------------------------------------------------------
*/
// quantizer scale of window coefs
#define WFIR_QUANTBITS		15
#define WFIR_QUANTSCALE		(1L<<WFIR_QUANTBITS)
#define WFIR_8SHIFT			(WFIR_QUANTBITS-8)
#define WFIR_16BITSHIFT		(WFIR_QUANTBITS)
// log2(number)-1 of precalculated taps range is [4..12]
#define WFIR_FRACBITS		10
#define WFIR_LUTLEN			((1L<<(WFIR_FRACBITS+1))+1)
// number of samples in window
#define WFIR_LOG2WIDTH		3
#define WFIR_WIDTH			(1L<<WFIR_LOG2WIDTH)
#define WFIR_SMPSPERWING	((WFIR_WIDTH-1)>>1)
// cutoff (1.0 == pi/2)
#define WFIR_CUTOFF		0.90f
// wfir type
#define WFIR_HANN		0
#define WFIR_HAMMING		1
#define WFIR_BLACKMANEXACT	2
#define WFIR_BLACKMAN3T61	3
#define WFIR_BLACKMAN3T67	4
#define WFIR_BLACKMAN4T92	5
#define WFIR_BLACKMAN4T74	6
#define WFIR_KAISER4T		7
#define WFIR_TYPE		WFIR_BLACKMANEXACT
// wfir help
#ifndef M_zPI
#define M_zPI		3.1415926535897932384626433832795
#endif
#define M_zEPS		1e-8
#define M_zBESSELEPS	1e-21

class CzWINDOWEDFIR
{
public:
	CzWINDOWEDFIR( );
	~CzWINDOWEDFIR( );
	float coef( int _PCnr, float _POfs, float _PCut, int _PWidth, int _PType )
//OLD args to coef: float _PPos, float _PFc, int _PLen )
	{
		double	_LWidthM1       = _PWidth-1;
		double	_LWidthM1Half   = 0.5*_LWidthM1;
		double	_LPosU          = ((double)_PCnr - _POfs);
		double	_LPos           = _LPosU-_LWidthM1Half;
		double	_LPIdl          = 2.0*M_zPI/_LWidthM1;
		double	_LWc,_LSi;
		if( fabs(_LPos)<M_zEPS ) {
			_LWc	= 1.0;
			_LSi	= _PCut;
		} else {
			switch( _PType )
			{
			case WFIR_HANN:
				_LWc = 0.50 - 0.50 * cos(_LPIdl*_LPosU);
				break;
			case WFIR_HAMMING:
				_LWc = 0.54 - 0.46 * cos(_LPIdl*_LPosU);
				break;
			case WFIR_BLACKMANEXACT:
				_LWc = 0.42 - 0.50 * cos(_LPIdl*_LPosU) +
					0.08 * cos(2.0*_LPIdl*_LPosU);
				break;
			case WFIR_BLACKMAN3T61:
				_LWc = 0.44959 - 0.49364 * cos(_LPIdl*_LPosU) +
					0.05677 * cos(2.0*_LPIdl*_LPosU);
				break;
			case WFIR_BLACKMAN3T67:
				_LWc = 0.42323 - 0.49755 * cos(_LPIdl*_LPosU) +
					0.07922 * cos(2.0*_LPIdl*_LPosU);
				break;
			case WFIR_BLACKMAN4T92:
				_LWc = 0.35875 - 0.48829 * cos(_LPIdl*_LPosU) +
					0.14128 * cos(2.0*_LPIdl*_LPosU) -
					0.01168 * cos(3.0*_LPIdl*_LPosU);
				break;
			case WFIR_BLACKMAN4T74:
				_LWc = 0.40217 - 0.49703 * cos(_LPIdl*_LPosU) +
					0.09392 * cos(2.0*_LPIdl*_LPosU) -
					0.00183 * cos(3.0*_LPIdl*_LPosU);
				break;
			case WFIR_KAISER4T:
				_LWc = 0.40243 - 0.49804 * cos(_LPIdl*_LPosU) +
					0.09831 * cos(2.0*_LPIdl*_LPosU) -
					0.00122 * cos(3.0*_LPIdl*_LPosU);
				break;
			default:
				_LWc = 1.0;
				break;
			}
			_LPos	 *= M_zPI;
			_LSi	 = sin(_PCut*_LPos)/_LPos;
		}
		return (float)(_LWc*_LSi);
	}
	static signed short lut[WFIR_LUTLEN*WFIR_WIDTH];
};

signed short CzWINDOWEDFIR::lut[WFIR_LUTLEN*WFIR_WIDTH];

CzWINDOWEDFIR::CzWINDOWEDFIR()
{
	int _LPcl;
	float _LPcllen	= (float)(1L<<WFIR_FRACBITS);	// number of precalculated lines for 0..1 (-1..0)
	float _LNorm	= 1.0f / (float)(2.0f * _LPcllen);
	float _LCut		= WFIR_CUTOFF;
	float _LScale	= (float)WFIR_QUANTSCALE;
	for( _LPcl=0;_LPcl<WFIR_LUTLEN;_LPcl++ )
	{
		float _LGain,_LCoefs[WFIR_WIDTH];
		float _LOfs		= ((float)_LPcl-_LPcllen)*_LNorm;
		int _LCc,_LIdx	= _LPcl<<WFIR_LOG2WIDTH;
		for( _LCc=0,_LGain=0.0f;_LCc<WFIR_WIDTH;_LCc++ )
		{	_LGain	+= (_LCoefs[_LCc] = coef( _LCc, _LOfs, _LCut, WFIR_WIDTH, WFIR_TYPE ));
		}
		_LGain = 1.0f/_LGain;
		for( _LCc=0;_LCc<WFIR_WIDTH;_LCc++ )
		{	float _LCoef = (float)floor( 0.5 + _LScale*_LCoefs[_LCc]*_LGain );
		lut[_LIdx+_LCc] = (signed short)( (_LCoef<-_LScale)?-_LScale:((_LCoef>_LScale)?_LScale:_LCoef) );
		}
	}
}

CzWINDOWEDFIR::~CzWINDOWEDFIR()
{	// nothing todo
}

CzWINDOWEDFIR sfir;

// ----------------------------------------------------------------------------
// MIXING MACROS
// ----------------------------------------------------------------------------
#if defined(__cplusplus) && (__cplusplus >= 201402L)
#define REGISTER
#else
#define REGISTER register
#endif

#define SNDMIX_BEGINSAMPLELOOP8\
	REGISTER MODCHANNEL * const pChn = pChannel;\
	nPos = pChn->nPosLo;\
	const signed char *p = (signed char *)(pChn->pCurrentSample+pChn->nPos);\
	if (pChn->dwFlags & CHN_STEREO) p += pChn->nPos;\
	int *pvol = pbuffer;\
	do {

#define SNDMIX_BEGINSAMPLELOOP16\
	REGISTER MODCHANNEL * const pChn = pChannel;\
	nPos = pChn->nPosLo;\
	const signed short *p = (signed short *)(pChn->pCurrentSample+(pChn->nPos*2));\
	if (pChn->dwFlags & CHN_STEREO) p += pChn->nPos;\
	int *pvol = pbuffer;\
	do {

#define SNDMIX_ENDSAMPLELOOP\
		nPos += pChn->nInc;\
	} while (pvol < pbufmax);\
	pChn->nPos += nPos >> 16;\
	pChn->nPosLo = nPos & 0xFFFF;

#define SNDMIX_ENDSAMPLELOOP8	SNDMIX_ENDSAMPLELOOP
#define SNDMIX_ENDSAMPLELOOP16	SNDMIX_ENDSAMPLELOOP

//////////////////////////////////////////////////////////////////////////////
// Mono

// No interpolation
#define SNDMIX_GETMONOVOL8NOIDO\
	int vol = p[nPos >> 16] << 8;

#define SNDMIX_GETMONOVOL16NOIDO\
	int vol = p[nPos >> 16];

// Linear Interpolation
#define SNDMIX_GETMONOVOL8LINEAR\
	int poshi = nPos >> 16;\
	int poslo = (nPos >> 8) & 0xFF;\
	int srcvol = p[poshi];\
	int destvol = p[poshi+1];\
	int vol = (srcvol<<8) + ((int)(poslo * (destvol - srcvol)));

#define SNDMIX_GETMONOVOL16LINEAR\
	int poshi = nPos >> 16;\
	int poslo = (nPos >> 8) & 0xFF;\
	int srcvol = p[poshi];\
	int destvol = p[poshi+1];\
	int vol = srcvol + ((int)(poslo * (destvol - srcvol)) >> 8);

// spline interpolation (2 guard bits should be enough???)
#define SPLINE_FRACSHIFT ((16-SPLINE_FRACBITS)-2)
#define SPLINE_FRACMASK  (((1L<<(16-SPLINE_FRACSHIFT))-1)&~3)

#define SNDMIX_GETMONOVOL8SPLINE \
	int poshi	= nPos >> 16; \
	int poslo	= (nPos >> SPLINE_FRACSHIFT) & SPLINE_FRACMASK; \
	int vol		= (CzCUBICSPLINE::lut[poslo  ]*(int)p[poshi-1] + \
	               CzCUBICSPLINE::lut[poslo+1]*(int)p[poshi  ] + \
	               CzCUBICSPLINE::lut[poslo+3]*(int)p[poshi+2] + \
	               CzCUBICSPLINE::lut[poslo+2]*(int)p[poshi+1]) >> SPLINE_8SHIFT;

#define SNDMIX_GETMONOVOL16SPLINE \
	int poshi	= nPos >> 16; \
	int poslo	= (nPos >> SPLINE_FRACSHIFT) & SPLINE_FRACMASK; \
	int vol		= (CzCUBICSPLINE::lut[poslo  ]*(int)p[poshi-1] + \
	               CzCUBICSPLINE::lut[poslo+1]*(int)p[poshi  ] + \
	               CzCUBICSPLINE::lut[poslo+3]*(int)p[poshi+2] + \
	               CzCUBICSPLINE::lut[poslo+2]*(int)p[poshi+1]) >> SPLINE_16SHIFT;


// fir interpolation
#define WFIR_FRACSHIFT	(16-(WFIR_FRACBITS+1+WFIR_LOG2WIDTH))
#define WFIR_FRACMASK	((((1L<<(17-WFIR_FRACSHIFT))-1)&~((1L<<WFIR_LOG2WIDTH)-1)))
#define WFIR_FRACHALVE	(1L<<(16-(WFIR_FRACBITS+2)))

#define SNDMIX_GETMONOVOL8FIRFILTER \
	int poshi  = nPos >> 16;\
	int poslo  = (nPos & 0xFFFF);\
	int firidx = ((poslo+WFIR_FRACHALVE)>>WFIR_FRACSHIFT) & WFIR_FRACMASK; \
	int vol    = (CzWINDOWEDFIR::lut[firidx+0]*(int)p[poshi+1-4]);	\
            vol   += (CzWINDOWEDFIR::lut[firidx+1]*(int)p[poshi+2-4]);	\
            vol   += (CzWINDOWEDFIR::lut[firidx+2]*(int)p[poshi+3-4]);	\
            vol   += (CzWINDOWEDFIR::lut[firidx+3]*(int)p[poshi+4-4]);	\
            vol   += (CzWINDOWEDFIR::lut[firidx+4]*(int)p[poshi+5-4]);	\
            vol   += (CzWINDOWEDFIR::lut[firidx+5]*(int)p[poshi+6-4]);	\
            vol   += (CzWINDOWEDFIR::lut[firidx+6]*(int)p[poshi+7-4]);	\
            vol   += (CzWINDOWEDFIR::lut[firidx+7]*(int)p[poshi+8-4]);	\
            vol  >>= WFIR_8SHIFT;

#define SNDMIX_GETMONOVOL16FIRFILTER \
    int poshi  = nPos >> 16;\
    int poslo  = (nPos & 0xFFFF);\
    int firidx = ((poslo+WFIR_FRACHALVE)>>WFIR_FRACSHIFT) & WFIR_FRACMASK; \
    int vol1   = (CzWINDOWEDFIR::lut[firidx+0]*(int)p[poshi+1-4]);	\
        vol1  += (CzWINDOWEDFIR::lut[firidx+1]*(int)p[poshi+2-4]);	\
        vol1  += (CzWINDOWEDFIR::lut[firidx+2]*(int)p[poshi+3-4]);	\
        vol1  += (CzWINDOWEDFIR::lut[firidx+3]*(int)p[poshi+4-4]);	\
    int vol2   = (CzWINDOWEDFIR::lut[firidx+4]*(int)p[poshi+5-4]);	\
	vol2  += (CzWINDOWEDFIR::lut[firidx+5]*(int)p[poshi+6-4]);	\
	vol2  += (CzWINDOWEDFIR::lut[firidx+6]*(int)p[poshi+7-4]);	\
	vol2  += (CzWINDOWEDFIR::lut[firidx+7]*(int)p[poshi+8-4]);	\
    int vol    = ((vol1>>1)+(vol2>>1)) >> (WFIR_16BITSHIFT-1);

/////////////////////////////////////////////////////////////////////////////
// Stereo

// No interpolation
#define SNDMIX_GETSTEREOVOL8NOIDO\
    int vol_l = p[(nPos>>16)*2] << 8;\
    int vol_r = p[(nPos>>16)*2+1] << 8;

#define SNDMIX_GETSTEREOVOL16NOIDO\
    int vol_l = p[(nPos>>16)*2];\
    int vol_r = p[(nPos>>16)*2+1];

// Linear Interpolation
#define SNDMIX_GETSTEREOVOL8LINEAR\
    int poshi = nPos >> 16;\
    int poslo = (nPos >> 8) & 0xFF;\
    int srcvol_l = p[poshi*2];\
    int vol_l = (srcvol_l<<8) + ((int)(poslo * (p[poshi*2+2] - srcvol_l)));\
    int srcvol_r = p[poshi*2+1];\
    int vol_r = (srcvol_r<<8) + ((int)(poslo * (p[poshi*2+3] - srcvol_r)));

#define SNDMIX_GETSTEREOVOL16LINEAR\
    int poshi = nPos >> 16;\
    int poslo = (nPos >> 8) & 0xFF;\
    int srcvol_l = p[poshi*2];\
    int vol_l = srcvol_l + ((int)(poslo * (p[poshi*2+2] - srcvol_l)) >> 8);\
    int srcvol_r = p[poshi*2+1];\
    int vol_r = srcvol_r + ((int)(poslo * (p[poshi*2+3] - srcvol_r)) >> 8);\

// Spline Interpolation
#define SNDMIX_GETSTEREOVOL8SPLINE \
    int poshi	= nPos >> 16; \
    int poslo	= (nPos >> SPLINE_FRACSHIFT) & SPLINE_FRACMASK; \
    int vol_l	= (CzCUBICSPLINE::lut[poslo  ]*(int)p[(poshi-1)*2  ] + \
	           CzCUBICSPLINE::lut[poslo+1]*(int)p[(poshi  )*2  ] + \
	           CzCUBICSPLINE::lut[poslo+2]*(int)p[(poshi+1)*2  ] + \
	           CzCUBICSPLINE::lut[poslo+3]*(int)p[(poshi+2)*2  ]) >> SPLINE_8SHIFT; \
    int vol_r	= (CzCUBICSPLINE::lut[poslo  ]*(int)p[(poshi-1)*2+1] + \
	           CzCUBICSPLINE::lut[poslo+1]*(int)p[(poshi  )*2+1] + \
	           CzCUBICSPLINE::lut[poslo+2]*(int)p[(poshi+1)*2+1] + \
	           CzCUBICSPLINE::lut[poslo+3]*(int)p[(poshi+2)*2+1]) >> SPLINE_8SHIFT;

#define SNDMIX_GETSTEREOVOL16SPLINE \
    int poshi	= nPos >> 16; \
    int poslo	= (nPos >> SPLINE_FRACSHIFT) & SPLINE_FRACMASK; \
    int vol_l	= (CzCUBICSPLINE::lut[poslo  ]*(int)p[(poshi-1)*2  ] + \
	           CzCUBICSPLINE::lut[poslo+1]*(int)p[(poshi  )*2  ] + \
	           CzCUBICSPLINE::lut[poslo+2]*(int)p[(poshi+1)*2  ] + \
	           CzCUBICSPLINE::lut[poslo+3]*(int)p[(poshi+2)*2  ]) >> SPLINE_16SHIFT; \
    int vol_r	= (CzCUBICSPLINE::lut[poslo  ]*(int)p[(poshi-1)*2+1] + \
	           CzCUBICSPLINE::lut[poslo+1]*(int)p[(poshi  )*2+1] + \
	           CzCUBICSPLINE::lut[poslo+2]*(int)p[(poshi+1)*2+1] + \
	           CzCUBICSPLINE::lut[poslo+3]*(int)p[(poshi+2)*2+1]) >> SPLINE_16SHIFT;

// fir interpolation
#define SNDMIX_GETSTEREOVOL8FIRFILTER \
    int poshi   = nPos >> 16;\
    int poslo   = (nPos & 0xFFFF);\
    int firidx  = ((poslo+WFIR_FRACHALVE)>>WFIR_FRACSHIFT) & WFIR_FRACMASK; \
    int vol_l   = (CzWINDOWEDFIR::lut[firidx+0]*(int)p[(poshi+1-4)*2  ]);   \
	vol_l  += (CzWINDOWEDFIR::lut[firidx+1]*(int)p[(poshi+2-4)*2  ]);   \
	vol_l  += (CzWINDOWEDFIR::lut[firidx+2]*(int)p[(poshi+3-4)*2  ]);   \
        vol_l  += (CzWINDOWEDFIR::lut[firidx+3]*(int)p[(poshi+4-4)*2  ]);   \
        vol_l  += (CzWINDOWEDFIR::lut[firidx+4]*(int)p[(poshi+5-4)*2  ]);   \
	vol_l  += (CzWINDOWEDFIR::lut[firidx+5]*(int)p[(poshi+6-4)*2  ]);   \
	vol_l  += (CzWINDOWEDFIR::lut[firidx+6]*(int)p[(poshi+7-4)*2  ]);   \
        vol_l  += (CzWINDOWEDFIR::lut[firidx+7]*(int)p[(poshi+8-4)*2  ]);   \
	vol_l >>= WFIR_8SHIFT; \
    int vol_r   = (CzWINDOWEDFIR::lut[firidx+0]*(int)p[(poshi+1-4)*2+1]);   \
	vol_r  += (CzWINDOWEDFIR::lut[firidx+1]*(int)p[(poshi+2-4)*2+1]);   \
	vol_r  += (CzWINDOWEDFIR::lut[firidx+2]*(int)p[(poshi+3-4)*2+1]);   \
	vol_r  += (CzWINDOWEDFIR::lut[firidx+3]*(int)p[(poshi+4-4)*2+1]);   \
	vol_r  += (CzWINDOWEDFIR::lut[firidx+4]*(int)p[(poshi+5-4)*2+1]);   \
        vol_r  += (CzWINDOWEDFIR::lut[firidx+5]*(int)p[(poshi+6-4)*2+1]);   \
        vol_r  += (CzWINDOWEDFIR::lut[firidx+6]*(int)p[(poshi+7-4)*2+1]);   \
        vol_r  += (CzWINDOWEDFIR::lut[firidx+7]*(int)p[(poshi+8-4)*2+1]);   \
        vol_r >>= WFIR_8SHIFT;

#define SNDMIX_GETSTEREOVOL16FIRFILTER \
    int poshi   = nPos >> 16;\
    int poslo   = (nPos & 0xFFFF);\
    int firidx  = ((poslo+WFIR_FRACHALVE)>>WFIR_FRACSHIFT) & WFIR_FRACMASK; \
    int vol1_l  = (CzWINDOWEDFIR::lut[firidx+0]*(int)p[(poshi+1-4)*2  ]);   \
	vol1_l += (CzWINDOWEDFIR::lut[firidx+1]*(int)p[(poshi+2-4)*2  ]);   \
        vol1_l += (CzWINDOWEDFIR::lut[firidx+2]*(int)p[(poshi+3-4)*2  ]);   \
	vol1_l += (CzWINDOWEDFIR::lut[firidx+3]*(int)p[(poshi+4-4)*2  ]);   \
   int vol2_l  = (CzWINDOWEDFIR::lut[firidx+4]*(int)p[(poshi+5-4)*2  ]);    \
       vol2_l += (CzWINDOWEDFIR::lut[firidx+5]*(int)p[(poshi+6-4)*2  ]);    \
       vol2_l += (CzWINDOWEDFIR::lut[firidx+6]*(int)p[(poshi+7-4)*2  ]);    \
       vol2_l += (CzWINDOWEDFIR::lut[firidx+7]*(int)p[(poshi+8-4)*2  ]);    \
   int vol_l   = ((vol1_l>>1)+(vol2_l>>1)) >> (WFIR_16BITSHIFT-1); \
   int vol1_r  = (CzWINDOWEDFIR::lut[firidx+0]*(int)p[(poshi+1-4)*2+1]);    \
       vol1_r += (CzWINDOWEDFIR::lut[firidx+1]*(int)p[(poshi+2-4)*2+1]);    \
       vol1_r += (CzWINDOWEDFIR::lut[firidx+2]*(int)p[(poshi+3-4)*2+1]);    \
       vol1_r += (CzWINDOWEDFIR::lut[firidx+3]*(int)p[(poshi+4-4)*2+1]);    \
   int vol2_r  = (CzWINDOWEDFIR::lut[firidx+4]*(int)p[(poshi+5-4)*2+1]);    \
       vol2_r += (CzWINDOWEDFIR::lut[firidx+5]*(int)p[(poshi+6-4)*2+1]);    \
       vol2_r += (CzWINDOWEDFIR::lut[firidx+6]*(int)p[(poshi+7-4)*2+1]);    \
       vol2_r += (CzWINDOWEDFIR::lut[firidx+7]*(int)p[(poshi+8-4)*2+1]);    \
   int vol_r   = ((vol1_r>>1)+(vol2_r>>1)) >> (WFIR_16BITSHIFT-1);

/////////////////////////////////////////////////////////////////////////////

#define SNDMIX_STOREMONOVOL\
	pvol[0] += vol * pChn->nRightVol;\
	pvol[1] += vol * pChn->nLeftVol;\
	pvol += 2;

#define SNDMIX_STORESTEREOVOL\
	pvol[0] += vol_l * pChn->nRightVol;\
	pvol[1] += vol_r * pChn->nLeftVol;\
	pvol += 2;

#define SNDMIX_STOREFASTMONOVOL\
	int v = vol * pChn->nRightVol;\
	pvol[0] += v;\
	pvol[1] += v;\
	pvol += 2;

#define SNDMIX_RAMPMONOVOL\
	nRampLeftVol += pChn->nLeftRamp;\
	nRampRightVol += pChn->nRightRamp;\
	pvol[0] += vol * (nRampRightVol >> VOLUMERAMPPRECISION);\
	pvol[1] += vol * (nRampLeftVol >> VOLUMERAMPPRECISION);\
	pvol += 2;

#define SNDMIX_RAMPFASTMONOVOL\
	nRampRightVol += pChn->nRightRamp;\
	int fastvol = vol * (nRampRightVol >> VOLUMERAMPPRECISION);\
	pvol[0] += fastvol;\
	pvol[1] += fastvol;\
	pvol += 2;

#define SNDMIX_RAMPSTEREOVOL\
	nRampLeftVol += pChn->nLeftRamp;\
	nRampRightVol += pChn->nRightRamp;\
	pvol[0] += vol_l * (nRampRightVol >> VOLUMERAMPPRECISION);\
	pvol[1] += vol_r * (nRampLeftVol >> VOLUMERAMPPRECISION);\
	pvol += 2;


///////////////////////////////////////////////////
// Resonant Filters

// Mono
#define MIX_BEGIN_FILTER\
	int fy1 = pChannel->nFilter_Y1;\
	int fy2 = pChannel->nFilter_Y2;\

#define MIX_END_FILTER\
	pChannel->nFilter_Y1 = fy1;\
	pChannel->nFilter_Y2 = fy2;

#define SNDMIX_PROCESSFILTER\
	vol = (vol * pChn->nFilter_A0 + fy1 * pChn->nFilter_B0 + fy2 * pChn->nFilter_B1 + 4096) >> 13;\
	fy2 = fy1;\
	fy1 = vol;\

// Stereo
#define MIX_BEGIN_STEREO_FILTER\
	int fy1 = pChannel->nFilter_Y1;\
	int fy2 = pChannel->nFilter_Y2;\
	int fy3 = pChannel->nFilter_Y3;\
	int fy4 = pChannel->nFilter_Y4;\

#define MIX_END_STEREO_FILTER\
	pChannel->nFilter_Y1 = fy1;\
	pChannel->nFilter_Y2 = fy2;\
	pChannel->nFilter_Y3 = fy3;\
	pChannel->nFilter_Y4 = fy4;\

#define SNDMIX_PROCESSSTEREOFILTER\
	vol_l = (vol_l * pChn->nFilter_A0 + fy1 * pChn->nFilter_B0 + fy2 * pChn->nFilter_B1 + 4096) >> 13;\
	vol_r = (vol_r * pChn->nFilter_A0 + fy3 * pChn->nFilter_B0 + fy4 * pChn->nFilter_B1 + 4096) >> 13;\
	fy2 = fy1; fy1 = vol_l;\
	fy4 = fy3; fy3 = vol_r;\

//////////////////////////////////////////////////////////
// Interfaces

typedef VOID (MPPASMCALL * LPMIXINTERFACE)(MODCHANNEL *, int *, int *);

#define BEGIN_MIX_INTERFACE(func)\
	VOID MPPASMCALL func(MODCHANNEL *pChannel, int *pbuffer, int *pbufmax)\
	{\
		LONG nPos;

#define END_MIX_INTERFACE()\
		SNDMIX_ENDSAMPLELOOP\
	}

// Volume Ramps
#define BEGIN_RAMPMIX_INTERFACE(func)\
	BEGIN_MIX_INTERFACE(func)\
		LONG nRampRightVol = pChannel->nRampRightVol;\
		LONG nRampLeftVol = pChannel->nRampLeftVol;

#define END_RAMPMIX_INTERFACE()\
		SNDMIX_ENDSAMPLELOOP\
		pChannel->nRampRightVol = nRampRightVol;\
		pChannel->nRightVol = nRampRightVol >> VOLUMERAMPPRECISION;\
		pChannel->nRampLeftVol = nRampLeftVol;\
		pChannel->nLeftVol = nRampLeftVol >> VOLUMERAMPPRECISION;\
	}

#define BEGIN_FASTRAMPMIX_INTERFACE(func)\
	BEGIN_MIX_INTERFACE(func)\
		LONG nRampRightVol = pChannel->nRampRightVol;

#define END_FASTRAMPMIX_INTERFACE()\
		SNDMIX_ENDSAMPLELOOP\
		pChannel->nRampRightVol = nRampRightVol;\
		pChannel->nRampLeftVol = nRampRightVol;\
		pChannel->nRightVol = nRampRightVol >> VOLUMERAMPPRECISION;\
		pChannel->nLeftVol = pChannel->nRightVol;\
	}


// Mono Resonant Filters
#define BEGIN_MIX_FLT_INTERFACE(func)\
	BEGIN_MIX_INTERFACE(func)\
	MIX_BEGIN_FILTER


#define END_MIX_FLT_INTERFACE()\
	SNDMIX_ENDSAMPLELOOP\
	MIX_END_FILTER\
	}

#define BEGIN_RAMPMIX_FLT_INTERFACE(func)\
	BEGIN_MIX_INTERFACE(func)\
		LONG nRampRightVol = pChannel->nRampRightVol;\
		LONG nRampLeftVol = pChannel->nRampLeftVol;\
		MIX_BEGIN_FILTER

#define END_RAMPMIX_FLT_INTERFACE()\
		SNDMIX_ENDSAMPLELOOP\
		MIX_END_FILTER\
		pChannel->nRampRightVol = nRampRightVol;\
		pChannel->nRightVol = nRampRightVol >> VOLUMERAMPPRECISION;\
		pChannel->nRampLeftVol = nRampLeftVol;\
		pChannel->nLeftVol = nRampLeftVol >> VOLUMERAMPPRECISION;\
	}

// Stereo Resonant Filters
#define BEGIN_MIX_STFLT_INTERFACE(func)\
	BEGIN_MIX_INTERFACE(func)\
	MIX_BEGIN_STEREO_FILTER


#define END_MIX_STFLT_INTERFACE()\
	SNDMIX_ENDSAMPLELOOP\
	MIX_END_STEREO_FILTER\
	}

#define BEGIN_RAMPMIX_STFLT_INTERFACE(func)\
	BEGIN_MIX_INTERFACE(func)\
		LONG nRampRightVol = pChannel->nRampRightVol;\
		LONG nRampLeftVol = pChannel->nRampLeftVol;\
		MIX_BEGIN_STEREO_FILTER

#define END_RAMPMIX_STFLT_INTERFACE()\
		SNDMIX_ENDSAMPLELOOP\
		MIX_END_STEREO_FILTER\
		pChannel->nRampRightVol = nRampRightVol;\
		pChannel->nRightVol = nRampRightVol >> VOLUMERAMPPRECISION;\
		pChannel->nRampLeftVol = nRampLeftVol;\
		pChannel->nLeftVol = nRampLeftVol >> VOLUMERAMPPRECISION;\
	}


/////////////////////////////////////////////////////
//

void MPPASMCALL X86_InitMixBuffer(int *pBuffer, UINT nSamples);
void MPPASMCALL X86_EndChannelOfs(MODCHANNEL *pChannel, int *pBuffer, UINT nSamples);
void MPPASMCALL X86_StereoFill(int *pBuffer, UINT nSamples, LPLONG lpROfs, LPLONG lpLOfs);
void X86_StereoMixToFloat(const int *, float *, float *, UINT nCount);
void X86_FloatToStereoMix(const float *pIn1, const float *pIn2, int *pOut, UINT nCount);

/////////////////////////////////////////////////////
// Mono samples functions

BEGIN_MIX_INTERFACE(Mono8BitMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8NOIDO
	SNDMIX_STOREMONOVOL
END_MIX_INTERFACE()

BEGIN_MIX_INTERFACE(Mono16BitMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16NOIDO
	SNDMIX_STOREMONOVOL
END_MIX_INTERFACE()

BEGIN_MIX_INTERFACE(Mono8BitLinearMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8LINEAR
	SNDMIX_STOREMONOVOL
END_MIX_INTERFACE()

BEGIN_MIX_INTERFACE(Mono16BitLinearMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16LINEAR
	SNDMIX_STOREMONOVOL
END_MIX_INTERFACE()

BEGIN_MIX_INTERFACE(Mono8BitSplineMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8SPLINE
	SNDMIX_STOREMONOVOL
END_MIX_INTERFACE()

BEGIN_MIX_INTERFACE(Mono16BitSplineMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16SPLINE
	SNDMIX_STOREMONOVOL
END_MIX_INTERFACE()

BEGIN_MIX_INTERFACE(Mono8BitFirFilterMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8FIRFILTER
	SNDMIX_STOREMONOVOL
END_MIX_INTERFACE()

BEGIN_MIX_INTERFACE(Mono16BitFirFilterMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16FIRFILTER
	SNDMIX_STOREMONOVOL
END_MIX_INTERFACE()


// Volume Ramps
BEGIN_RAMPMIX_INTERFACE(Mono8BitRampMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8NOIDO
	SNDMIX_RAMPMONOVOL
END_RAMPMIX_INTERFACE()

BEGIN_RAMPMIX_INTERFACE(Mono16BitRampMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16NOIDO
	SNDMIX_RAMPMONOVOL
END_RAMPMIX_INTERFACE()

BEGIN_RAMPMIX_INTERFACE(Mono8BitLinearRampMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8LINEAR
	SNDMIX_RAMPMONOVOL
END_RAMPMIX_INTERFACE()

BEGIN_RAMPMIX_INTERFACE(Mono16BitLinearRampMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16LINEAR
	SNDMIX_RAMPMONOVOL
END_RAMPMIX_INTERFACE()

BEGIN_RAMPMIX_INTERFACE(Mono8BitSplineRampMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8SPLINE
	SNDMIX_RAMPMONOVOL
END_RAMPMIX_INTERFACE()

BEGIN_RAMPMIX_INTERFACE(Mono16BitSplineRampMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16SPLINE
	SNDMIX_RAMPMONOVOL
END_RAMPMIX_INTERFACE()

BEGIN_RAMPMIX_INTERFACE(Mono8BitFirFilterRampMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8FIRFILTER
	SNDMIX_RAMPMONOVOL
END_RAMPMIX_INTERFACE()

BEGIN_RAMPMIX_INTERFACE(Mono16BitFirFilterRampMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16FIRFILTER
	SNDMIX_RAMPMONOVOL
END_RAMPMIX_INTERFACE()


//////////////////////////////////////////////////////
// Fast mono mix for leftvol=rightvol (1 less imul)

BEGIN_MIX_INTERFACE(FastMono8BitMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8NOIDO
	SNDMIX_STOREFASTMONOVOL
END_MIX_INTERFACE()

BEGIN_MIX_INTERFACE(FastMono16BitMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16NOIDO
	SNDMIX_STOREFASTMONOVOL
END_MIX_INTERFACE()

BEGIN_MIX_INTERFACE(FastMono8BitLinearMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8LINEAR
	SNDMIX_STOREFASTMONOVOL
END_MIX_INTERFACE()

BEGIN_MIX_INTERFACE(FastMono16BitLinearMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16LINEAR
	SNDMIX_STOREFASTMONOVOL
END_MIX_INTERFACE()

BEGIN_MIX_INTERFACE(FastMono8BitSplineMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8SPLINE
	SNDMIX_STOREFASTMONOVOL
END_MIX_INTERFACE()

BEGIN_MIX_INTERFACE(FastMono16BitSplineMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16SPLINE
	SNDMIX_STOREFASTMONOVOL
END_MIX_INTERFACE()

BEGIN_MIX_INTERFACE(FastMono8BitFirFilterMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8FIRFILTER
	SNDMIX_STOREFASTMONOVOL
END_MIX_INTERFACE()

BEGIN_MIX_INTERFACE(FastMono16BitFirFilterMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16FIRFILTER
	SNDMIX_STOREFASTMONOVOL
END_MIX_INTERFACE()


// Fast Ramps
BEGIN_FASTRAMPMIX_INTERFACE(FastMono8BitRampMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8NOIDO
	SNDMIX_RAMPFASTMONOVOL
END_FASTRAMPMIX_INTERFACE()

BEGIN_FASTRAMPMIX_INTERFACE(FastMono16BitRampMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16NOIDO
	SNDMIX_RAMPFASTMONOVOL
END_FASTRAMPMIX_INTERFACE()

BEGIN_FASTRAMPMIX_INTERFACE(FastMono8BitLinearRampMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8LINEAR
	SNDMIX_RAMPFASTMONOVOL
END_FASTRAMPMIX_INTERFACE()

BEGIN_FASTRAMPMIX_INTERFACE(FastMono16BitLinearRampMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16LINEAR
	SNDMIX_RAMPFASTMONOVOL
END_FASTRAMPMIX_INTERFACE()

BEGIN_FASTRAMPMIX_INTERFACE(FastMono8BitSplineRampMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8SPLINE
	SNDMIX_RAMPFASTMONOVOL
END_FASTRAMPMIX_INTERFACE()

BEGIN_FASTRAMPMIX_INTERFACE(FastMono16BitSplineRampMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16SPLINE
	SNDMIX_RAMPFASTMONOVOL
END_FASTRAMPMIX_INTERFACE()

BEGIN_FASTRAMPMIX_INTERFACE(FastMono8BitFirFilterRampMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8FIRFILTER
	SNDMIX_RAMPFASTMONOVOL
END_FASTRAMPMIX_INTERFACE()

BEGIN_FASTRAMPMIX_INTERFACE(FastMono16BitFirFilterRampMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16FIRFILTER
	SNDMIX_RAMPFASTMONOVOL
END_FASTRAMPMIX_INTERFACE()


//////////////////////////////////////////////////////
// Stereo samples

BEGIN_MIX_INTERFACE(Stereo8BitMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETSTEREOVOL8NOIDO
	SNDMIX_STORESTEREOVOL
END_MIX_INTERFACE()

BEGIN_MIX_INTERFACE(Stereo16BitMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETSTEREOVOL16NOIDO
	SNDMIX_STORESTEREOVOL
END_MIX_INTERFACE()

BEGIN_MIX_INTERFACE(Stereo8BitLinearMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETSTEREOVOL8LINEAR
	SNDMIX_STORESTEREOVOL
END_MIX_INTERFACE()

BEGIN_MIX_INTERFACE(Stereo16BitLinearMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETSTEREOVOL16LINEAR
	SNDMIX_STORESTEREOVOL
END_MIX_INTERFACE()

BEGIN_MIX_INTERFACE(Stereo8BitSplineMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETSTEREOVOL8SPLINE
	SNDMIX_STORESTEREOVOL
END_MIX_INTERFACE()

BEGIN_MIX_INTERFACE(Stereo16BitSplineMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETSTEREOVOL16SPLINE
	SNDMIX_STORESTEREOVOL
END_MIX_INTERFACE()

BEGIN_MIX_INTERFACE(Stereo8BitFirFilterMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETSTEREOVOL8FIRFILTER
	SNDMIX_STORESTEREOVOL
END_MIX_INTERFACE()

BEGIN_MIX_INTERFACE(Stereo16BitFirFilterMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETSTEREOVOL16FIRFILTER
	SNDMIX_STORESTEREOVOL
END_MIX_INTERFACE()


// Volume Ramps
BEGIN_RAMPMIX_INTERFACE(Stereo8BitRampMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETSTEREOVOL8NOIDO
	SNDMIX_RAMPSTEREOVOL
END_RAMPMIX_INTERFACE()

BEGIN_RAMPMIX_INTERFACE(Stereo16BitRampMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETSTEREOVOL16NOIDO
	SNDMIX_RAMPSTEREOVOL
END_RAMPMIX_INTERFACE()

BEGIN_RAMPMIX_INTERFACE(Stereo8BitLinearRampMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETSTEREOVOL8LINEAR
	SNDMIX_RAMPSTEREOVOL
END_RAMPMIX_INTERFACE()

BEGIN_RAMPMIX_INTERFACE(Stereo16BitLinearRampMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETSTEREOVOL16LINEAR
	SNDMIX_RAMPSTEREOVOL
END_RAMPMIX_INTERFACE()

BEGIN_RAMPMIX_INTERFACE(Stereo8BitSplineRampMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETSTEREOVOL8SPLINE
	SNDMIX_RAMPSTEREOVOL
END_RAMPMIX_INTERFACE()

BEGIN_RAMPMIX_INTERFACE(Stereo16BitSplineRampMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETSTEREOVOL16SPLINE
	SNDMIX_RAMPSTEREOVOL
END_RAMPMIX_INTERFACE()

BEGIN_RAMPMIX_INTERFACE(Stereo8BitFirFilterRampMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETSTEREOVOL8FIRFILTER
	SNDMIX_RAMPSTEREOVOL
END_RAMPMIX_INTERFACE()

BEGIN_RAMPMIX_INTERFACE(Stereo16BitFirFilterRampMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETSTEREOVOL16FIRFILTER
	SNDMIX_RAMPSTEREOVOL
END_RAMPMIX_INTERFACE()



//////////////////////////////////////////////////////
// Resonant Filter Mix

#ifndef NO_FILTER

// Mono Filter Mix
BEGIN_MIX_FLT_INTERFACE(FilterMono8BitMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8NOIDO
	SNDMIX_PROCESSFILTER
	SNDMIX_STOREMONOVOL
END_MIX_FLT_INTERFACE()

BEGIN_MIX_FLT_INTERFACE(FilterMono16BitMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16NOIDO
	SNDMIX_PROCESSFILTER
	SNDMIX_STOREMONOVOL
END_MIX_FLT_INTERFACE()

BEGIN_MIX_FLT_INTERFACE(FilterMono8BitLinearMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8LINEAR
	SNDMIX_PROCESSFILTER
	SNDMIX_STOREMONOVOL
END_MIX_FLT_INTERFACE()

BEGIN_MIX_FLT_INTERFACE(FilterMono16BitLinearMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16LINEAR
	SNDMIX_PROCESSFILTER
	SNDMIX_STOREMONOVOL
END_MIX_FLT_INTERFACE()

BEGIN_MIX_FLT_INTERFACE(FilterMono8BitSplineMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8SPLINE
	SNDMIX_PROCESSFILTER
	SNDMIX_STOREMONOVOL
END_MIX_FLT_INTERFACE()

BEGIN_MIX_FLT_INTERFACE(FilterMono16BitSplineMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16SPLINE
	SNDMIX_PROCESSFILTER
	SNDMIX_STOREMONOVOL
END_MIX_FLT_INTERFACE()

BEGIN_MIX_FLT_INTERFACE(FilterMono8BitFirFilterMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8FIRFILTER
	SNDMIX_PROCESSFILTER
	SNDMIX_STOREMONOVOL
END_MIX_FLT_INTERFACE()

BEGIN_MIX_FLT_INTERFACE(FilterMono16BitFirFilterMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16FIRFILTER
	SNDMIX_PROCESSFILTER
	SNDMIX_STOREMONOVOL
END_MIX_FLT_INTERFACE()

// Filter + Ramp
BEGIN_RAMPMIX_FLT_INTERFACE(FilterMono8BitRampMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8NOIDO
	SNDMIX_PROCESSFILTER
	SNDMIX_RAMPMONOVOL
END_RAMPMIX_FLT_INTERFACE()

BEGIN_RAMPMIX_FLT_INTERFACE(FilterMono16BitRampMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16NOIDO
	SNDMIX_PROCESSFILTER
	SNDMIX_RAMPMONOVOL
END_RAMPMIX_FLT_INTERFACE()

BEGIN_RAMPMIX_FLT_INTERFACE(FilterMono8BitLinearRampMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8LINEAR
	SNDMIX_PROCESSFILTER
	SNDMIX_RAMPMONOVOL
END_RAMPMIX_FLT_INTERFACE()

BEGIN_RAMPMIX_FLT_INTERFACE(FilterMono16BitLinearRampMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16LINEAR
	SNDMIX_PROCESSFILTER
	SNDMIX_RAMPMONOVOL
END_RAMPMIX_FLT_INTERFACE()

BEGIN_RAMPMIX_FLT_INTERFACE(FilterMono8BitSplineRampMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8SPLINE
	SNDMIX_PROCESSFILTER
	SNDMIX_RAMPMONOVOL
END_RAMPMIX_FLT_INTERFACE()

BEGIN_RAMPMIX_FLT_INTERFACE(FilterMono16BitSplineRampMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16SPLINE
	SNDMIX_PROCESSFILTER
	SNDMIX_RAMPMONOVOL
END_RAMPMIX_FLT_INTERFACE()

BEGIN_RAMPMIX_FLT_INTERFACE(FilterMono8BitFirFilterRampMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETMONOVOL8FIRFILTER
	SNDMIX_PROCESSFILTER
	SNDMIX_RAMPMONOVOL
END_RAMPMIX_FLT_INTERFACE()

BEGIN_RAMPMIX_FLT_INTERFACE(FilterMono16BitFirFilterRampMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETMONOVOL16FIRFILTER
	SNDMIX_PROCESSFILTER
	SNDMIX_RAMPMONOVOL
END_RAMPMIX_FLT_INTERFACE()


// Stereo Filter Mix
BEGIN_MIX_STFLT_INTERFACE(FilterStereo8BitMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETSTEREOVOL8NOIDO
	SNDMIX_PROCESSSTEREOFILTER
	SNDMIX_STORESTEREOVOL
END_MIX_STFLT_INTERFACE()

BEGIN_MIX_STFLT_INTERFACE(FilterStereo16BitMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETSTEREOVOL16NOIDO
	SNDMIX_PROCESSSTEREOFILTER
	SNDMIX_STORESTEREOVOL
END_MIX_STFLT_INTERFACE()

BEGIN_MIX_STFLT_INTERFACE(FilterStereo8BitLinearMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETSTEREOVOL8LINEAR
	SNDMIX_PROCESSSTEREOFILTER
	SNDMIX_STORESTEREOVOL
END_MIX_STFLT_INTERFACE()

BEGIN_MIX_STFLT_INTERFACE(FilterStereo16BitLinearMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETSTEREOVOL16LINEAR
	SNDMIX_PROCESSSTEREOFILTER
	SNDMIX_STORESTEREOVOL
END_MIX_STFLT_INTERFACE()

BEGIN_MIX_STFLT_INTERFACE(FilterStereo8BitSplineMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETSTEREOVOL8SPLINE
	SNDMIX_PROCESSSTEREOFILTER
	SNDMIX_STORESTEREOVOL
END_MIX_STFLT_INTERFACE()

BEGIN_MIX_STFLT_INTERFACE(FilterStereo16BitSplineMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETSTEREOVOL16SPLINE
	SNDMIX_PROCESSSTEREOFILTER
	SNDMIX_STORESTEREOVOL
END_MIX_STFLT_INTERFACE()

BEGIN_MIX_STFLT_INTERFACE(FilterStereo8BitFirFilterMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETSTEREOVOL8FIRFILTER
	SNDMIX_PROCESSSTEREOFILTER
	SNDMIX_STORESTEREOVOL
END_MIX_STFLT_INTERFACE()

BEGIN_MIX_STFLT_INTERFACE(FilterStereo16BitFirFilterMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETSTEREOVOL16FIRFILTER
	SNDMIX_PROCESSSTEREOFILTER
	SNDMIX_STORESTEREOVOL
END_MIX_STFLT_INTERFACE()

// Stereo Filter + Ramp
BEGIN_RAMPMIX_STFLT_INTERFACE(FilterStereo8BitRampMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETSTEREOVOL8NOIDO
	SNDMIX_PROCESSSTEREOFILTER
	SNDMIX_RAMPSTEREOVOL
END_RAMPMIX_STFLT_INTERFACE()

BEGIN_RAMPMIX_STFLT_INTERFACE(FilterStereo16BitRampMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETSTEREOVOL16NOIDO
	SNDMIX_PROCESSSTEREOFILTER
	SNDMIX_RAMPSTEREOVOL
END_RAMPMIX_STFLT_INTERFACE()

BEGIN_RAMPMIX_STFLT_INTERFACE(FilterStereo8BitLinearRampMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETSTEREOVOL8LINEAR
	SNDMIX_PROCESSSTEREOFILTER
	SNDMIX_RAMPSTEREOVOL
END_RAMPMIX_STFLT_INTERFACE()

BEGIN_RAMPMIX_STFLT_INTERFACE(FilterStereo16BitLinearRampMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETSTEREOVOL16LINEAR
	SNDMIX_PROCESSSTEREOFILTER
	SNDMIX_RAMPSTEREOVOL
END_RAMPMIX_STFLT_INTERFACE()

BEGIN_RAMPMIX_STFLT_INTERFACE(FilterStereo8BitSplineRampMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETSTEREOVOL8SPLINE
	SNDMIX_PROCESSSTEREOFILTER
	SNDMIX_RAMPSTEREOVOL
END_RAMPMIX_STFLT_INTERFACE()

BEGIN_RAMPMIX_STFLT_INTERFACE(FilterStereo16BitSplineRampMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETSTEREOVOL16SPLINE
	SNDMIX_PROCESSSTEREOFILTER
	SNDMIX_RAMPSTEREOVOL
END_RAMPMIX_STFLT_INTERFACE()

BEGIN_RAMPMIX_STFLT_INTERFACE(FilterStereo8BitFirFilterRampMix)
	SNDMIX_BEGINSAMPLELOOP8
	SNDMIX_GETSTEREOVOL8FIRFILTER
	SNDMIX_PROCESSSTEREOFILTER
	SNDMIX_RAMPSTEREOVOL
END_RAMPMIX_STFLT_INTERFACE()

BEGIN_RAMPMIX_STFLT_INTERFACE(FilterStereo16BitFirFilterRampMix)
	SNDMIX_BEGINSAMPLELOOP16
	SNDMIX_GETSTEREOVOL16FIRFILTER
	SNDMIX_PROCESSSTEREOFILTER
	SNDMIX_RAMPSTEREOVOL
END_RAMPMIX_STFLT_INTERFACE()


#else
// Mono
#define FilterMono8BitMix					Mono8BitMix
#define FilterMono16BitMix					Mono16BitMix
#define FilterMono8BitLinearMix				Mono8BitLinearMix
#define FilterMono16BitLinearMix			Mono16BitLinearMix
#define FilterMono8BitSplineMix				Mono8BitSplineMix
#define FilterMono16BitSplineMix			Mono16BitSplineMix
#define FilterMono8BitFirFilterMix			Mono8BitFirFilterMix
#define FilterMono16BitFirFilterMix			Mono16BitFirFilterMix
#define FilterMono8BitRampMix				Mono8BitRampMix
#define FilterMono16BitRampMix				Mono16BitRampMix
#define FilterMono8BitLinearRampMix			Mono8BitLinearRampMix
#define FilterMono16BitLinearRampMix		Mono16BitLinearRampMix
#define FilterMono8BitSplineRampMix			Mono8BitSplineRampMix
#define FilterMono16BitSplineRampMix		Mono16BitSplineRampMix
#define FilterMono8BitFirFilterRampMix		Mono8BitFirFilterRampMix
#define FilterMono16BitFirFilterRampMix		Mono16BitFirFilterRampMix
// Stereo
#define FilterStereo8BitMix					Stereo8BitMix
#define FilterStereo16BitMix				Stereo16BitMix
#define FilterStereo8BitLinearMix			Stereo8BitLinearMix
#define FilterStereo16BitLinearMix			Stereo16BitLinearMix
#define FilterStereo8BitSplineMix			Stereo8BitSplineMix
#define FilterStereo16BitSplineMix			Stereo16BitSplineMix
#define FilterStereo8BitFirFilterMix		Stereo8BitFirFilterMix
#define FilterStereo16BitFirFilterMix		Stereo16BitFirFilterMix
#define FilterStereo8BitRampMix				Stereo8BitRampMix
#define FilterStereo16BitRampMix			Stereo16BitRampMix
#define FilterStereo8BitLinearRampMix		Stereo8BitLinearRampMix
#define FilterStereo16BitLinearRampMix		Stereo16BitLinearRampMix
#define FilterStereo8BitSplineRampMix		Stereo8BitSplineRampMix
#define FilterStereo16BitSplineRampMix		Stereo16BitSplineRampMix
#define FilterStereo8BitFirFilterRampMix	Stereo8BitFirFilterRampMix
#define FilterStereo16BitFirFilterRampMix	Stereo16BitFirFilterRampMix

#endif

///////////////////////////////////////////////////////////////////////////////
//
// Mix function tables
//
//
// Index is as follow:
//	[b1-b0]	format (8-bit-mono, 16-bit-mono, 8-bit-stereo, 16-bit-stereo)
//	[b2]	ramp
//	[b3]	filter
//	[b5-b4]	src type
//

#define MIXNDX_16BIT        0x01
#define MIXNDX_STEREO       0x02
#define MIXNDX_RAMP         0x04
#define MIXNDX_FILTER       0x08
#define MIXNDX_LINEARSRC    0x10
#define MIXNDX_SPLINESRC    0x20
#define MIXNDX_FIRSRC       0x30

const LPMIXINTERFACE gpMixFunctionTable[2*2*16] =
{
	// No SRC
	Mono8BitMix, Mono16BitMix, Stereo8BitMix, Stereo16BitMix,
	Mono8BitRampMix, Mono16BitRampMix, Stereo8BitRampMix,
            Stereo16BitRampMix,
	// No SRC, Filter
	FilterMono8BitMix, FilterMono16BitMix, FilterStereo8BitMix,
        FilterStereo16BitMix, FilterMono8BitRampMix, FilterMono16BitRampMix,
	FilterStereo8BitRampMix, FilterStereo16BitRampMix,
	// Linear SRC
	Mono8BitLinearMix, Mono16BitLinearMix, Stereo8BitLinearMix,
	Stereo16BitLinearMix, Mono8BitLinearRampMix, Mono16BitLinearRampMix,
	Stereo8BitLinearRampMix,Stereo16BitLinearRampMix,
	// Linear SRC, Filter
	FilterMono8BitLinearMix, FilterMono16BitLinearMix,
        FilterStereo8BitLinearMix, FilterStereo16BitLinearMix,
	FilterMono8BitLinearRampMix, FilterMono16BitLinearRampMix,
        FilterStereo8BitLinearRampMix, FilterStereo16BitLinearRampMix,

	// FirFilter SRC
	Mono8BitSplineMix, Mono16BitSplineMix, Stereo8BitSplineMix,
        Stereo16BitSplineMix, Mono8BitSplineRampMix, Mono16BitSplineRampMix,
	Stereo8BitSplineRampMix,Stereo16BitSplineRampMix,
	// Spline SRC, Filter
	FilterMono8BitSplineMix, FilterMono16BitSplineMix,
        FilterStereo8BitSplineMix, FilterStereo16BitSplineMix,
	FilterMono8BitSplineRampMix, FilterMono16BitSplineRampMix,
        FilterStereo8BitSplineRampMix, FilterStereo16BitSplineRampMix,

	// FirFilter  SRC
	Mono8BitFirFilterMix, Mono16BitFirFilterMix, Stereo8BitFirFilterMix,
	Stereo16BitFirFilterMix, Mono8BitFirFilterRampMix,
        Mono16BitFirFilterRampMix, Stereo8BitFirFilterRampMix,
        Stereo16BitFirFilterRampMix,
	// FirFilter  SRC, Filter
	FilterMono8BitFirFilterMix, FilterMono16BitFirFilterMix,
        FilterStereo8BitFirFilterMix, FilterStereo16BitFirFilterMix,
	FilterMono8BitFirFilterRampMix, FilterMono16BitFirFilterRampMix,
        FilterStereo8BitFirFilterRampMix, FilterStereo16BitFirFilterRampMix
};

const LPMIXINTERFACE gpFastMixFunctionTable[2*2*16] =
{
	// No SRC
	FastMono8BitMix, FastMono16BitMix, Stereo8BitMix, Stereo16BitMix,
	FastMono8BitRampMix, FastMono16BitRampMix, Stereo8BitRampMix,
        Stereo16BitRampMix,
	// No SRC, Filter
	FilterMono8BitMix, FilterMono16BitMix, FilterStereo8BitMix,
        FilterStereo16BitMix, FilterMono8BitRampMix, FilterMono16BitRampMix,
        FilterStereo8BitRampMix, FilterStereo16BitRampMix,
	// Linear SRC
	FastMono8BitLinearMix, FastMono16BitLinearMix, Stereo8BitLinearMix,
        Stereo16BitLinearMix, FastMono8BitLinearRampMix,
        FastMono16BitLinearRampMix, Stereo8BitLinearRampMix,
        Stereo16BitLinearRampMix,
	// Linear SRC, Filter
	FilterMono8BitLinearMix, FilterMono16BitLinearMix,
        FilterStereo8BitLinearMix, FilterStereo16BitLinearMix,
	FilterMono8BitLinearRampMix, FilterMono16BitLinearRampMix,
        FilterStereo8BitLinearRampMix, FilterStereo16BitLinearRampMix,

	// Spline SRC
	Mono8BitSplineMix, Mono16BitSplineMix, Stereo8BitSplineMix,
        Stereo16BitSplineMix, Mono8BitSplineRampMix, Mono16BitSplineRampMix,
        Stereo8BitSplineRampMix, Stereo16BitSplineRampMix,
	// Spline SRC, Filter
	FilterMono8BitSplineMix, FilterMono16BitSplineMix,
        FilterStereo8BitSplineMix, FilterStereo16BitSplineMix,
	FilterMono8BitSplineRampMix, FilterMono16BitSplineRampMix,
        FilterStereo8BitSplineRampMix, FilterStereo16BitSplineRampMix,

	// FirFilter SRC
	Mono8BitFirFilterMix, Mono16BitFirFilterMix, Stereo8BitFirFilterMix,
        Stereo16BitFirFilterMix, Mono8BitFirFilterRampMix,
        Mono16BitFirFilterRampMix, Stereo8BitFirFilterRampMix,
        Stereo16BitFirFilterRampMix,
	// FirFilter SRC, Filter
	FilterMono8BitFirFilterMix, FilterMono16BitFirFilterMix,
        FilterStereo8BitFirFilterMix, FilterStereo16BitFirFilterMix,
	FilterMono8BitFirFilterRampMix, FilterMono16BitFirFilterRampMix,
        FilterStereo8BitFirFilterRampMix, FilterStereo16BitFirFilterRampMix,
};


/////////////////////////////////////////////////////////////////////////

static LONG MPPFASTCALL GetSampleCount(MODCHANNEL *pChn, LONG nSamples)
//---------------------------------------------------------------------
{
	LONG nLoopStart = (pChn->dwFlags & CHN_LOOP) ? pChn->nLoopStart : 0;
	LONG nInc = pChn->nInc;

	if ((nSamples <= 0) || (!nInc) || (!pChn->nLength)) return 0;
	// Under zero ?
	if ((LONG)pChn->nPos < nLoopStart)
	{
		if (nInc < 0)
		{
			// Invert loop for bidi loops
			LONG nDelta = ((nLoopStart - pChn->nPos) << 16) - (pChn->nPosLo & 0xffff);
			pChn->nPos = nLoopStart | (nDelta>>16);
			pChn->nPosLo = nDelta & 0xffff;
			if (((LONG)pChn->nPos < nLoopStart) ||
				(pChn->nPos >= (nLoopStart+pChn->nLength)/2))
			{
				pChn->nPos = nLoopStart; pChn->nPosLo = 0;
			}
			nInc = -nInc;
			pChn->nInc = nInc;
			pChn->dwFlags &= ~(CHN_PINGPONGFLAG); // go forward
			if ((!(pChn->dwFlags & CHN_LOOP)) || (pChn->nPos >= pChn->nLength))
			{
				pChn->nPos = pChn->nLength;
				pChn->nPosLo = 0;
				return 0;
			}
		} else
		{
			// We probably didn't hit the loop end yet
			// (first loop), so we do nothing
			if ((LONG)pChn->nPos < 0) pChn->nPos = 0;
		}
	} else
	// Past the end
	if (pChn->nPos >= pChn->nLength)
	{
		if (!(pChn->dwFlags & CHN_LOOP)) return 0; // not looping -> stop this channel
		if (pChn->dwFlags & CHN_PINGPONGLOOP)
		{
			// Invert loop
			if (nInc > 0)
			{
				nInc = -nInc;
				pChn->nInc = nInc;
			}
			pChn->dwFlags |= CHN_PINGPONGFLAG;
			// adjust loop position
			LONG nDeltaHi = (pChn->nPos - pChn->nLength);
			LONG nDeltaLo = 0x10000 - (pChn->nPosLo & 0xffff);
			pChn->nPos = pChn->nLength - nDeltaHi - (nDeltaLo>>16);
			pChn->nPosLo = nDeltaLo & 0xffff;
			if ((pChn->nPos <= pChn->nLoopStart) ||
			(pChn->nPos >= pChn->nLength))
				pChn->nPos = pChn->nLength-1;
		} else
		{
			if (nInc < 0) // This is a bug
			{
				nInc = -nInc;
				pChn->nInc = nInc;
			}
			// Restart at loop start
			pChn->nPos += nLoopStart - pChn->nLength;
			if ((LONG)pChn->nPos < nLoopStart)
				pChn->nPos = pChn->nLoopStart;
		}
	}
	LONG nPos = pChn->nPos;
	// too big increment, and/or too small loop length
	if (nPos < nLoopStart)
	{
		if ((nPos < 0) || (nInc < 0)) return 0;
	}
	if ((nPos < 0) || (nPos >= (LONG)pChn->nLength)) return 0;
	LONG nPosLo = (USHORT)pChn->nPosLo, nSmpCount = nSamples;
	if (nInc < 0)
	{
		LONG nInv = -nInc;
		LONG maxsamples = 16384 / ((nInv>>16)+1);
		if (maxsamples < 2) maxsamples = 2;
		if (nSamples > maxsamples) nSamples = maxsamples;
		LONG nDeltaHi = (nInv>>16) * (nSamples - 1);
		LONG nDeltaLo = (nInv&0xffff) * (nSamples - 1);
		LONG nPosDest = nPos - nDeltaHi + ((nPosLo - nDeltaLo) >> 16);
		if (nPosDest < nLoopStart)
		{
			nSmpCount = (ULONG)(((((LONGLONG)nPos - nLoopStart) << 16) + nPosLo - 1) / nInv) + 1;
		}
	} else
	{
		LONG maxsamples = 16384 / ((nInc>>16)+1);
		if (maxsamples < 2) maxsamples = 2;
		if (nSamples > maxsamples) nSamples = maxsamples;
		LONG nDeltaHi = (nInc>>16) * (nSamples - 1);
		LONG nDeltaLo = (nInc&0xffff) * (nSamples - 1);
		LONG nPosDest = nPos + nDeltaHi + ((nPosLo + nDeltaLo)>>16);
		if (nPosDest >= (LONG)pChn->nLength)
		{
			nSmpCount = (ULONG)(((((LONGLONG)pChn->nLength - nPos) << 16) - nPosLo - 1) / nInc) + 1;
		}
	}
	if (nSmpCount <= 1) return 1;
	if (nSmpCount > nSamples) return nSamples;
	return nSmpCount;
}


UINT CSoundFile::CreateStereoMix(int count)
//-----------------------------------------
{
	LPLONG pOfsL, pOfsR;
	DWORD nchused, nchmixed;

	if (!count) return 0;
#ifndef MODPLUG_FASTSOUNDLIB
	if (gnChannels > 2) X86_InitMixBuffer(MixRearBuffer, count*2);
#endif
	nchused = nchmixed = 0;
	for (UINT nChn=0; nChn<m_nMixChannels; nChn++)
	{
		const LPMIXINTERFACE *pMixFuncTable;
		MODCHANNEL * const pChannel = &Chn[ChnMix[nChn]];
		UINT nFlags;//, nMasterCh
		LONG nSmpCount;
		int nsamples;
		int *pbuffer;

		if (!pChannel->pCurrentSample) continue;
		//nMasterCh = (ChnMix[nChn] < m_nChannels) ? ChnMix[nChn]+1 : pChannel->nMasterChn;
		pOfsR = &gnDryROfsVol;
		pOfsL = &gnDryLOfsVol;
		nFlags = 0;
		if (pChannel->dwFlags & CHN_16BIT) nFlags |= MIXNDX_16BIT;
		if (pChannel->dwFlags & CHN_STEREO) nFlags |= MIXNDX_STEREO;
	#ifndef NO_FILTER
		if (pChannel->dwFlags & CHN_FILTER) nFlags |= MIXNDX_FILTER;
	#endif
		if (!(pChannel->dwFlags & CHN_NOIDO))
		{
			// use hq-fir mixer?
			if( (gdwSoundSetup & (SNDMIX_HQRESAMPLER|SNDMIX_ULTRAHQSRCMODE)) ==
				(SNDMIX_HQRESAMPLER|SNDMIX_ULTRAHQSRCMODE) )
				nFlags += MIXNDX_FIRSRC;
			else if( (gdwSoundSetup & (SNDMIX_HQRESAMPLER)) == SNDMIX_HQRESAMPLER )
				nFlags += MIXNDX_SPLINESRC;
			else
				nFlags += MIXNDX_LINEARSRC; // use
		}
		if ((nFlags < 0x40) && (pChannel->nLeftVol == pChannel->nRightVol)
		 && ((!pChannel->nRampLength) || (pChannel->nLeftRamp == pChannel->nRightRamp)))
		{
			pMixFuncTable = gpFastMixFunctionTable;
		} else
		{
			pMixFuncTable = gpMixFunctionTable;
		}
		nsamples = count;
#ifndef MODPLUG_NO_REVERB
		pbuffer = (gdwSoundSetup & SNDMIX_REVERB) ? MixReverbBuffer : MixSoundBuffer;
		if (pChannel->dwFlags & CHN_NOREVERB) pbuffer = MixSoundBuffer;
		if (pChannel->dwFlags & CHN_REVERB) pbuffer = MixReverbBuffer;
		if (pbuffer == MixReverbBuffer)
		{
			if (!gnReverbSend) memset(MixReverbBuffer, 0, count * 8);
			gnReverbSend += count;
		}
#else
		pbuffer = MixSoundBuffer;
#endif
		nchused++;
		////////////////////////////////////////////////////
	SampleLooping:
		UINT nrampsamples = nsamples;
		if (pChannel->nRampLength > 0)
		{
			if ((LONG)nrampsamples > pChannel->nRampLength) nrampsamples = pChannel->nRampLength;
		}
		if ((nSmpCount = GetSampleCount(pChannel, nrampsamples)) <= 0)
		{
			// Stopping the channel
			pChannel->pCurrentSample = NULL;
			pChannel->nLength = 0;
			pChannel->nPos = 0;
			pChannel->nPosLo = 0;
			pChannel->nRampLength = 0;
			X86_EndChannelOfs(pChannel, pbuffer, nsamples);
			*pOfsR += pChannel->nROfs;
			*pOfsL += pChannel->nLOfs;
			pChannel->nROfs = pChannel->nLOfs = 0;
			pChannel->dwFlags &= ~CHN_PINGPONGFLAG;
			continue;
		}
		// Should we mix this channel ?
		UINT naddmix;
		if (((nchmixed >= m_nMaxMixChannels) && (!(gdwSoundSetup & SNDMIX_DIRECTTODISK)))
		 || ((!pChannel->nRampLength) && (!(pChannel->nLeftVol|pChannel->nRightVol))))
		{
			LONG delta = (pChannel->nInc * (LONG)nSmpCount) + (LONG)pChannel->nPosLo;
			pChannel->nPosLo = delta & 0xFFFF;
			pChannel->nPos += (delta >> 16);
			pChannel->nROfs = pChannel->nLOfs = 0;
			pbuffer += nSmpCount*2;
			naddmix = 0;
		} else
		// Do mixing
		{
			// Choose function for mixing
			LPMIXINTERFACE pMixFunc;
			pMixFunc = (pChannel->nRampLength) ? pMixFuncTable[nFlags|MIXNDX_RAMP] : pMixFuncTable[nFlags];
			int *pbufmax = pbuffer + (nSmpCount*2);
			pChannel->nROfs = - *(pbufmax-2);
			pChannel->nLOfs = - *(pbufmax-1);
			pMixFunc(pChannel, pbuffer, pbufmax);
			pChannel->nROfs += *(pbufmax-2);
			pChannel->nLOfs += *(pbufmax-1);
			pbuffer = pbufmax;
			naddmix = 1;

		}
		nsamples -= nSmpCount;
		if (pChannel->nRampLength)
		{
			pChannel->nRampLength -= nSmpCount;
			if (pChannel->nRampLength <= 0)
			{
				pChannel->nRampLength = 0;
				pChannel->nRightVol = pChannel->nNewRightVol;
				pChannel->nLeftVol = pChannel->nNewLeftVol;
				pChannel->nRightRamp = pChannel->nLeftRamp = 0;
				if ((pChannel->dwFlags & CHN_NOTEFADE) && (!(pChannel->nFadeOutVol)))
				{
					pChannel->nLength = 0;
					pChannel->pCurrentSample = NULL;
				}
			}
		}
		if (nsamples > 0) goto SampleLooping;
		nchmixed += naddmix;
	}
	return nchused;
}


#ifdef _MSC_VER
#pragma warning (disable:4100)
#endif

// Clip and convert to 8 bit
#if defined(_MSC_VER) && defined(_M_IX86)
__declspec(naked) DWORD MPPASMCALL X86_Convert32To8(LPVOID lp16, int *pBuffer, DWORD lSampleCount, LPLONG lpMin, LPLONG lpMax)
//------------------------------------------------------------------------------
{
    _asm {
        push ebx
	push esi
	push edi
	mov ebx, 16[esp]		// ebx = 8-bit buffer
	mov esi, 20[esp]		// esi = pBuffer
	mov edi, 24[esp]		// edi = lSampleCount
	mov eax, 28[esp]
	mov ecx, dword ptr [eax]	// ecx = clipmin
	mov eax, 32[esp]
	mov edx, dword ptr [eax]	// edx = clipmax
cliploop:
	mov eax, dword ptr [esi]
	inc ebx
	cdq
	and edx, (1 << (24-MIXING_ATTENUATION)) - 1
	add eax, edx
	cmp eax, MIXING_CLIPMIN
	jl cliplow
	cmp eax, MIXING_CLIPMAX
	jg cliphigh
	cmp eax, ecx
	jl updatemin
	cmp eax, edx
	jg updatemax
cliprecover:
	add esi, 4
	sar eax, 24-MIXING_ATTENUATION
	xor eax, 0x80
	dec edi
	mov byte ptr [ebx-1], al
	jnz cliploop
	mov eax, 28[esp]
	mov dword ptr [eax], ecx
	mov eax, 32[esp]
	mov dword ptr [eax], edx
	mov eax, 24[esp]
	pop edi
	pop esi
	pop ebx
	ret
updatemin:
	mov ecx, eax
	jmp cliprecover
updatemax:
	mov edx, eax
	jmp cliprecover
cliplow:
	mov ecx, MIXING_CLIPMIN
	mov edx, MIXING_CLIPMAX
	mov eax, MIXING_CLIPMIN
	jmp cliprecover
cliphigh:
	mov ecx, MIXING_CLIPMIN
	mov edx, MIXING_CLIPMAX
	mov eax, MIXING_CLIPMAX
	jmp cliprecover
	}
}
#else //MSC_VER
//---GCCFIX: Asm replaced with C function
// The C version was written by Rani Assaf <rani@magic.metawire.com>, I believe
DWORD MPPASMCALL X86_Convert32To8(LPVOID lp8, int *pBuffer, DWORD lSampleCount, LPLONG lpMin, LPLONG lpMax)
{
	int vumin = *lpMin, vumax = *lpMax;
	unsigned char *p = (unsigned char *)lp8;
	for (UINT i=0; i<lSampleCount; i++)
	{
		int n = pBuffer[i];
		if (n < MIXING_CLIPMIN)
			n = MIXING_CLIPMIN;
		else if (n > MIXING_CLIPMAX)
			n = MIXING_CLIPMAX;
		if (n < vumin)
			vumin = n;
		else if (n > vumax)
			vumax = n;
		p[i] = (n >> (24-MIXING_ATTENUATION)) ^ 0x80;	// 8-bit unsigned
	}
	*lpMin = vumin;
	*lpMax = vumax;
	return lSampleCount;
}
#endif //MSC_VER, else

#if defined(_MSC_VER) && defined(_M_IX86)
// Clip and convert to 16 bit
__declspec(naked) DWORD MPPASMCALL X86_Convert32To16(LPVOID lp16, int *pBuffer, DWORD lSampleCount, LPLONG lpMin, LPLONG lpMax)
//------------------------------------------------------------------------------
{
	_asm {
	push ebx
	push esi
	push edi
	mov ebx, 16[esp]		// ebx = 16-bit buffer
	mov eax, 28[esp]
	mov esi, 20[esp]		// esi = pBuffer
	mov ecx, dword ptr [eax]	// ecx = clipmin
	mov edi, 24[esp]		// edi = lSampleCount
	mov eax, 32[esp]
	push ebp
	mov ebp, dword ptr [eax]	// edx = clipmax
cliploop:
	mov eax, dword ptr [esi]
	add ebx, 2
	cdq
	and edx, (1 << (16-MIXING_ATTENUATION)) - 1
	add esi, 4
	add eax, edx
	cmp eax, MIXING_CLIPMIN
	jl cliplow
	cmp eax, MIXING_CLIPMAX
	jg cliphigh
	cmp eax, ecx
	jl updatemin
	cmp eax, ebp
	jg updatemax
cliprecover:
	sar eax, 16-MIXING_ATTENUATION
	dec edi
	mov word ptr [ebx-2], ax
	jnz cliploop
	mov edx, ebp
	pop ebp
	mov eax, 28[esp]
	mov dword ptr [eax], ecx
	mov eax, 32[esp]
	mov dword ptr [eax], edx
	mov eax, 24[esp]
	pop edi
	shl eax, 1
	pop esi
	pop ebx
	ret
updatemin:
	mov ecx, eax
	jmp cliprecover
updatemax:
	mov ebp, eax
	jmp cliprecover
cliplow:
	mov ecx, MIXING_CLIPMIN
	mov ebp, MIXING_CLIPMAX
	mov eax, MIXING_CLIPMIN
	jmp cliprecover
cliphigh:
	mov ecx, MIXING_CLIPMIN
	mov ebp, MIXING_CLIPMAX
	mov eax, MIXING_CLIPMAX
	jmp cliprecover
	}
}
#else //MSC_VER
//---GCCFIX: Asm replaced with C function
// The C version was written by Rani Assaf <rani@magic.metawire.com>, I believe
DWORD MPPASMCALL X86_Convert32To16(LPVOID lp16, int *pBuffer, DWORD lSampleCount, LPLONG lpMin, LPLONG lpMax)
{
	int vumin = *lpMin, vumax = *lpMax;
	signed short *p = (signed short *)lp16;
	for (UINT i=0; i<lSampleCount; i++)
	{
		int n = pBuffer[i];
		if (n < MIXING_CLIPMIN)
			n = MIXING_CLIPMIN;
		else if (n > MIXING_CLIPMAX)
			n = MIXING_CLIPMAX;
		if (n < vumin)
			vumin = n;
		else if (n > vumax)
			vumax = n;
		p[i] = n >> (16-MIXING_ATTENUATION);	// 16-bit signed
	}
	*lpMin = vumin;
	*lpMax = vumax;
	return lSampleCount * 2;
}
#endif //MSC_VER, else

#if defined(_MSC_VER) && defined(_M_IX86)
// Clip and convert to 24 bit
__declspec(naked) DWORD MPPASMCALL X86_Convert32To24(LPVOID lp16, int *pBuffer, DWORD lSampleCount, LPLONG lpMin, LPLONG lpMax)
//------------------------------------------------------------------------------
{
	_asm {
	push ebx
	push esi
	push edi
	mov ebx, 16[esp]		// ebx = 8-bit buffer
	mov esi, 20[esp]		// esi = pBuffer
	mov edi, 24[esp]		// edi = lSampleCount
	mov eax, 28[esp]
	mov ecx, dword ptr [eax]	// ecx = clipmin
	mov eax, 32[esp]
	push ebp
	mov edx, dword ptr [eax]	// edx = clipmax
cliploop:
	mov eax, dword ptr [esi]
	mov ebp, eax
	sar ebp, 31
	and ebp, (1 << (8-MIXING_ATTENUATION)) - 1
	add eax, ebp
	cmp eax, MIXING_CLIPMIN
	jl cliplow
	cmp eax, MIXING_CLIPMAX
	jg cliphigh
	cmp eax, ecx
	jl updatemin
	cmp eax, edx
	jg updatemax
cliprecover:
	add ebx, 3
	sar eax, 8-MIXING_ATTENUATION
	add esi, 4
	mov word ptr [ebx-3], ax
	shr eax, 16
	dec edi
	mov byte ptr [ebx-1], al
	jnz cliploop
	pop ebp
	mov eax, 28[esp]
	mov dword ptr [eax], ecx
	mov eax, 32[esp]
	mov dword ptr [eax], edx
	mov edx, 24[esp]
	mov eax, edx
	pop edi
	shl eax, 1
	pop esi
	add eax, edx
	pop ebx
	ret
updatemin:
	mov ecx, eax
	jmp cliprecover
updatemax:
	mov edx, eax
	jmp cliprecover
cliplow:
	mov ecx, MIXING_CLIPMIN
	mov edx, MIXING_CLIPMAX
	mov eax, MIXING_CLIPMIN
	jmp cliprecover
cliphigh:
	mov ecx, MIXING_CLIPMIN
	mov edx, MIXING_CLIPMAX
	mov eax, MIXING_CLIPMAX
	jmp cliprecover
	}
}
#else //MSC_VER
//---GCCFIX: Asm replaced with C function
DWORD MPPASMCALL X86_Convert32To24(LPVOID lp16, int *pBuffer, DWORD lSampleCount, LPLONG lpMin, LPLONG lpMax)
{
	UINT i ;
	int vumin = *lpMin, vumax = *lpMax;
	int n,p ;
	unsigned char* buf = (unsigned char*)lp16 ;

	for ( i=0; i<lSampleCount; i++)
	{
		n = pBuffer[i];
		if (n < MIXING_CLIPMIN)
			n = MIXING_CLIPMIN;
		else if (n > MIXING_CLIPMAX)
			n = MIXING_CLIPMAX;
		if (n < vumin)
			vumin = n;
		else if (n > vumax)
			vumax = n;
		p = n >> (8-MIXING_ATTENUATION) ; // 24-bit signed
#ifdef WORDS_BIGENDIAN
		buf[i*3+0] = (p >> 16) & 0xFF;
		buf[i*3+1] = (p >> 8)  & 0xFF;
		buf[i*3+2] = (p >> 0)  & 0xFF;
#else
		buf[i*3+0] = (p >> 0)  & 0xFF;
		buf[i*3+1] = (p >> 8)  & 0xFF;
		buf[i*3+2] = (p >> 16) & 0xFF;
#endif
	}
	*lpMin = vumin;
	*lpMax = vumax;
	return lSampleCount * 3;
}
#endif

#if defined(_MSC_VER) && defined(_M_IX86)
// Clip and convert to 32 bit
__declspec(naked) DWORD MPPASMCALL X86_Convert32To32(LPVOID lp16, int *pBuffer, DWORD lSampleCount, LPLONG lpMin, LPLONG lpMax)
//------------------------------------------------------------------------------
{
	_asm {
	push ebx
	push esi
	push edi
	mov ebx, 16[esp]			// ebx = 32-bit buffer
	mov esi, 20[esp]			// esi = pBuffer
	mov edi, 24[esp]			// edi = lSampleCount
	mov eax, 28[esp]
	mov ecx, dword ptr [eax]	// ecx = clipmin
	mov eax, 32[esp]
	mov edx, dword ptr [eax]	// edx = clipmax
cliploop:
	mov eax, dword ptr [esi]
	add ebx, 4
	add esi, 4
	cmp eax, MIXING_CLIPMIN
	jl cliplow
	cmp eax, MIXING_CLIPMAX
	jg cliphigh
	cmp eax, ecx
	jl updatemin
	cmp eax, edx
	jg updatemax
cliprecover:
	shl eax, MIXING_ATTENUATION
	dec edi
	mov dword ptr [ebx-4], eax
	jnz cliploop
	mov eax, 28[esp]
	mov dword ptr [eax], ecx
	mov eax, 32[esp]
	mov dword ptr [eax], edx
	mov edx, 24[esp]
	pop edi
	mov eax, edx
	pop esi
	shl eax, 2
	pop ebx
	ret
updatemin:
	mov ecx, eax
	jmp cliprecover
updatemax:
	mov edx, eax
	jmp cliprecover
cliplow:
	mov ecx, MIXING_CLIPMIN
	mov edx, MIXING_CLIPMAX
	mov eax, MIXING_CLIPMIN
	jmp cliprecover
cliphigh:
	mov ecx, MIXING_CLIPMIN
	mov edx, MIXING_CLIPMAX
	mov eax, MIXING_CLIPMAX
	jmp cliprecover
	}
}
#else
//---GCCFIX: Asm replaced with C function
DWORD MPPASMCALL X86_Convert32To32(LPVOID lp16, int *pBuffer, DWORD lSampleCount, LPLONG lpMin, LPLONG lpMax)
{
	UINT i ;
	int vumin = *lpMin, vumax = *lpMax;
	int32_t *p = (int32_t *)lp16;

	for ( i=0; i<lSampleCount; i++)
	{
		int n = pBuffer[i];
		if (n < MIXING_CLIPMIN)
			n = MIXING_CLIPMIN;
		else if (n > MIXING_CLIPMAX)
			n = MIXING_CLIPMAX;
		if (n < vumin)
			vumin = n;
		else if (n > vumax)
			vumax = n;
		p[i] = n << MIXING_ATTENUATION;	// 32-bit signed
	}
	*lpMin = vumin;
	*lpMax = vumax;
	return lSampleCount * 4;
}
#endif


#if defined(_MSC_VER) && defined(_M_IX86)
void MPPASMCALL X86_InitMixBuffer(int *pBuffer, UINT nSamples)
//------------------------------------------------------------
{
	_asm {
	mov ecx, nSamples
	mov esi, pBuffer
	xor eax, eax
	mov edx, ecx
	shr ecx, 2
	and edx, 3
	jz unroll4x
loop1x:
	add esi, 4
	dec edx
	mov dword ptr [esi-4], eax
	jnz loop1x
unroll4x:
	or ecx, ecx
	jnz loop4x
	jmp done
loop4x:
	add esi, 16
	dec ecx
	mov dword ptr [esi-16], eax
	mov dword ptr [esi-12], eax
	mov dword ptr [esi-8], eax
	mov dword ptr [esi-4], eax
	jnz loop4x
done:;
	}
}
#else
//---GCCFIX: Asm replaced with C function
// Will fill in later.
void MPPASMCALL X86_InitMixBuffer(int *pBuffer, UINT nSamples)
{
	memset(pBuffer, 0, nSamples * sizeof(int));
}
#endif


#if defined(_MSC_VER) && defined(_M_IX86)
__declspec(naked) void MPPASMCALL X86_InterleaveFrontRear(int *pFrontBuf, int *pRearBuf, DWORD nSamples)
//------------------------------------------------------------------------------
{
	_asm {
	push ebx
	push ebp
	push esi
	push edi
	mov ecx, 28[esp] // ecx = samplecount
	mov esi, 20[esp] // esi = front buffer
	mov edi, 24[esp] // edi = rear buffer
	lea esi, [esi+ecx*4]	// esi = &front[N]
	lea edi, [edi+ecx*4]	// edi = &rear[N]
	lea ebx, [esi+ecx*4]	// ebx = &front[N*2]
interleaveloop:
	mov eax, dword ptr [esi-8]
	mov edx, dword ptr [esi-4]
	sub ebx, 16
	mov ebp, dword ptr [edi-8]
	mov dword ptr [ebx], eax
	mov dword ptr [ebx+4], edx
	mov eax, dword ptr [edi-4]
	sub esi, 8
	sub edi, 8
	dec ecx
	mov dword ptr [ebx+8], ebp
	mov dword ptr [ebx+12], eax
	jnz interleaveloop
	pop edi
	pop esi
	pop ebp
	pop ebx
	ret
	}
}
#else
//---GCCFIX: Asm replaced with C function
// Multichannel not supported.
void MPPASMCALL X86_InterleaveFrontRear(int *pFrontBuf, int *pRearBuf, DWORD nSamples)
{
}
#endif


#if defined(_MSC_VER) && defined(_M_IX86)
VOID MPPASMCALL X86_MonoFromStereo(int *pMixBuf, UINT nSamples)
//-------------------------------------------------------------
{
	_asm {
	mov ecx, nSamples
	mov esi, pMixBuf
	mov edi, esi
stloop:
	mov eax, dword ptr [esi]
	mov edx, dword ptr [esi+4]
	add edi, 4
	add esi, 8
	add eax, edx
	sar eax, 1
	dec ecx
	mov dword ptr [edi-4], eax
	jnz stloop
	}
}
#else
//---GCCFIX: Asm replaced with C function
VOID MPPASMCALL X86_MonoFromStereo(int *pMixBuf, UINT nSamples)
{
	UINT j;
	for(UINT i = 0; i < nSamples; i++)
	{
		j = i << 1;
		pMixBuf[i] = (pMixBuf[j] + pMixBuf[j + 1]) >> 1;
	}
}
#endif

#define OFSDECAYSHIFT	8
#define OFSDECAYMASK	0xFF


#if defined(_MSC_VER) && defined(_M_IX86)
void MPPASMCALL X86_StereoFill(int *pBuffer, UINT nSamples, LPLONG lpROfs, LPLONG lpLOfs)
//------------------------------------------------------------------------------
{
	_asm {
	mov edi, pBuffer
	mov ecx, nSamples
	mov eax, lpROfs
	mov edx, lpLOfs
	mov eax, [eax]
	mov edx, [edx]
	or ecx, ecx
	jz fill_loop
	mov ebx, eax
	or ebx, edx
	jz fill_loop
ofsloop:
	mov ebx, eax
	mov esi, edx
	neg ebx
	neg esi
	sar ebx, 31
	sar esi, 31
	and ebx, OFSDECAYMASK
	and esi, OFSDECAYMASK
	add ebx, eax
	add esi, edx
	sar ebx, OFSDECAYSHIFT
	sar esi, OFSDECAYSHIFT
	sub eax, ebx
	sub edx, esi
	mov ebx, eax
	or ebx, edx
	jz fill_loop
	add edi, 8
	dec ecx
	mov [edi-8], eax
	mov [edi-4], edx
	jnz ofsloop
fill_loop:
	mov ebx, ecx
	and ebx, 3
	jz fill4x
fill1x:
	mov [edi], eax
	mov [edi+4], edx
	add edi, 8
	dec ebx
	jnz fill1x
fill4x:
	shr ecx, 2
	or ecx, ecx
	jz done
fill4xloop:
	mov [edi], eax
	mov [edi+4], edx
	mov [edi+8], eax
	mov [edi+12], edx
	add edi, 8*4
	dec ecx
	mov [edi-16], eax
	mov [edi-12], edx
	mov [edi-8], eax
	mov [edi-4], edx
	jnz fill4xloop
done:
	mov esi, lpROfs
	mov edi, lpLOfs
	mov [esi], eax
	mov [edi], edx
	}
}
#else
//---GCCFIX: Asm replaced with C function
#define OFSDECAYSHIFT    8
#define OFSDECAYMASK     0xFF
void MPPASMCALL X86_StereoFill(int *pBuffer, UINT nSamples, LPLONG lpROfs, LPLONG lpLOfs)
//----------------------------------------------------------------------------
{
	int rofs = *lpROfs;
	int lofs = *lpLOfs;

	if ((!rofs) && (!lofs))
	{
		X86_InitMixBuffer(pBuffer, nSamples*2);
		return;
	}
	for (UINT i=0; i<nSamples; i++)
	{
		int x_r = (rofs + (((-rofs)>>31) & OFSDECAYMASK)) >> OFSDECAYSHIFT;
		int x_l = (lofs + (((-lofs)>>31) & OFSDECAYMASK)) >> OFSDECAYSHIFT;
		rofs -= x_r;
		lofs -= x_l;
		pBuffer[i*2] = x_r;
		pBuffer[i*2+1] = x_l;
	}
	*lpROfs = rofs;
	*lpLOfs = lofs;
}
#endif

#if defined(_MSC_VER) && defined(_M_IX86)
void MPPASMCALL X86_EndChannelOfs(MODCHANNEL *pChannel, int *pBuffer, UINT nSamples)
//------------------------------------------------------------------------------
{
	_asm {
	mov esi, pChannel
	mov edi, pBuffer
	mov ecx, nSamples
	mov eax, dword ptr [esi+MODCHANNEL.nROfs]
	mov edx, dword ptr [esi+MODCHANNEL.nLOfs]
	or ecx, ecx
	jz brkloop
ofsloop:
	mov ebx, eax
	mov esi, edx
	neg ebx
	neg esi
	sar ebx, 31
	sar esi, 31
	and ebx, OFSDECAYMASK
	and esi, OFSDECAYMASK
	add ebx, eax
	add esi, edx
	sar ebx, OFSDECAYSHIFT
	sar esi, OFSDECAYSHIFT
	sub eax, ebx
	sub edx, esi
	mov ebx, eax
	add dword ptr [edi], eax
	add dword ptr [edi+4], edx
	or ebx, edx
	jz brkloop
	add edi, 8
	dec ecx
	jnz ofsloop
brkloop:
	mov esi, pChannel
	mov dword ptr [esi+MODCHANNEL.nROfs], eax
	mov dword ptr [esi+MODCHANNEL.nLOfs], edx
	}
}
#else
//---GCCFIX: Asm replaced with C function
// Will fill in later.
void MPPASMCALL X86_EndChannelOfs(MODCHANNEL *pChannel, int *pBuffer, UINT nSamples)
{
	int rofs = pChannel->nROfs;
	int lofs = pChannel->nLOfs;

	if ((!rofs) && (!lofs)) return;
	for (UINT i=0; i<nSamples; i++)
	{
		int x_r = (rofs + (((-rofs)>>31) & OFSDECAYMASK)) >> OFSDECAYSHIFT;
		int x_l = (lofs + (((-lofs)>>31) & OFSDECAYMASK)) >> OFSDECAYSHIFT;
		rofs -= x_r;
		lofs -= x_l;
		pBuffer[i*2] += x_r;
		pBuffer[i*2+1] += x_l;
	}
	pChannel->nROfs = rofs;
	pChannel->nLOfs = lofs;
}
#endif


//////////////////////////////////////////////////////////////////////////////////
// Automatic Gain Control

#ifndef NO_AGC

// Limiter
#define MIXING_LIMITMAX		(0x08100000)
#define MIXING_LIMITMIN		(-MIXING_LIMITMAX)

#if defined(_MSC_VER) && defined(_M_IX86)
__declspec(naked) UINT MPPASMCALL X86_AGC(int *pBuffer, UINT nSamples, UINT nAGC)
//------------------------------------------------------------------------------
{
	__asm {
	push ebx
	push ebp
	push esi
	push edi
	mov esi, 20[esp]	// esi = pBuffer+i
	mov ecx, 24[esp]	// ecx = i
	mov edi, 28[esp]	// edi = AGC (0..256)
agcloop:
	mov eax, dword ptr [esi]
	imul edi
	shrd eax, edx, AGC_PRECISION
	add esi, 4
	cmp eax, MIXING_LIMITMIN
	jl agcupdate
	cmp eax, MIXING_LIMITMAX
	jg agcupdate
agcrecover:
	dec ecx
	mov dword ptr [esi-4], eax
	jnz agcloop
	mov eax, edi
	pop edi
	pop esi
	pop ebp
	pop ebx
	ret
agcupdate:
	dec edi
	jmp agcrecover
	}
}
#pragma warning (default:4100)
#else
// Version for GCC
UINT MPPASMCALL X86_AGC(int *pBuffer, UINT nSamples, UINT nAGC)
{
	int x;

	while(nSamples)
	{
		x = ((int64_t)(*pBuffer) * nAGC) >> AGC_PRECISION;

		if((x < MIXING_LIMITMIN) || (x > MIXING_LIMITMAX))
		nAGC--;

		*pBuffer = x;

		pBuffer++;
		nSamples--;
	}

	return nAGC;
}
#endif

void CSoundFile::ProcessAGC(int count)
//------------------------------------
{
	static DWORD gAGCRecoverCount = 0;
	UINT agc = X86_AGC(MixSoundBuffer, count, gnAGC);
	// Some kind custom law, so that the AGC stays quite stable, but slowly
	// goes back up if the sound level stays below a level inversely
	// proportional to the AGC level. (J'me comprends)
	if ((agc >= gnAGC) && (gnAGC < AGC_UNITY) && (gnVUMeter < (0xFF - (gnAGC >> (AGC_PRECISION-7))) ))
	{
		gAGCRecoverCount += count;
		UINT agctimeout = gdwMixingFreq + gnAGC;
		if (gnChannels >= 2) agctimeout <<= 1;
		if (gAGCRecoverCount >= agctimeout)
		{
			gAGCRecoverCount = 0;
			gnAGC++;
		}
	} else
	{
		gnAGC = agc;
		gAGCRecoverCount = 0;
	}
}


void CSoundFile::ResetAGC()
//-------------------------
{
	gnAGC = AGC_UNITY;
}
#endif // NO_AGC
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (endian and char fixes for PPC)
*/

////////////////////////////////////////////////////////////
// 669 Composer / UNIS 669 module loader
////////////////////////////////////////////////////////////

//#include "stdafx.h"
//#include "sndfile.h"

//#pragma warning(disable:4244)

typedef struct tagFILEHEADER669
{
	WORD sig;				// 'if' or 'JN'
	char songmessage[108];			// Song Message
	BYTE samples;			// number of samples (1-64)
	BYTE patterns;			// number of patterns (1-128)
	BYTE restartpos;
	BYTE orders[128];
	BYTE tempolist[128];
	BYTE breaks[128];
} FILEHEADER669;

typedef struct tagSAMPLE669
{
	BYTE filename[13];
	BYTE length[4];	// when will somebody think about DWORD align ???
	BYTE loopstart[4];
	BYTE loopend[4];
} SAMPLE669;

static DWORD lengthArrayToDWORD(const BYTE length[4]) {
	DWORD len = (length[3] << 24) +
		    (length[2] << 16) +
		    (length[1] << 8) +
		    (length[0]);

	return(len);
}


BOOL CSoundFile::Read669(const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
//	BOOL b669Ext;
	const FILEHEADER669 *pfh = (const FILEHEADER669 *)lpStream;
	const SAMPLE669 *psmp = (const SAMPLE669 *)(lpStream + 0x1F1);
	DWORD dwMemPos = 0;

	if ((!lpStream) || (dwMemLength < sizeof(FILEHEADER669))) return FALSE;
	if ((bswapLE16(pfh->sig) != 0x6669) && (bswapLE16(pfh->sig) != 0x4E4A)) return FALSE;
//	b669Ext = (bswapLE16(pfh->sig) == 0x4E4A) ? TRUE : FALSE;
	if ((!pfh->samples) || (pfh->samples > 64) || (pfh->restartpos >= 128)
	 || (!pfh->patterns) || (pfh->patterns > 128)) return FALSE;
	DWORD dontfuckwithme = 0x1F1 + pfh->samples * sizeof(SAMPLE669) + pfh->patterns * 0x600;
	if (dontfuckwithme > dwMemLength) return FALSE;
	for (UINT ichk=0; ichk<pfh->samples; ichk++)
	{
		DWORD len = lengthArrayToDWORD(psmp[ichk].length);
		dontfuckwithme += len;
	}
	if (dontfuckwithme > dwMemLength) return FALSE;
	// That should be enough checking: this must be a 669 module.
	m_nType = MOD_TYPE_669;
	m_dwSongFlags |= SONG_LINEARSLIDES;
	m_nMinPeriod = 28 << 2;
	m_nMaxPeriod = 1712 << 3;
	m_nDefaultTempo = 78;
	m_nDefaultSpeed = 6;
	m_nChannels = 8;
	memcpy(m_szNames[0], pfh->songmessage, 16);
	m_nSamples = pfh->samples;
	for (UINT nins=1; nins<=m_nSamples; nins++, psmp++)
	{
		DWORD len = lengthArrayToDWORD(psmp->length);
		DWORD loopstart = lengthArrayToDWORD(psmp->loopstart);
		DWORD loopend = lengthArrayToDWORD(psmp->loopend);
		if (len > MAX_SAMPLE_LENGTH) len = MAX_SAMPLE_LENGTH;
		if ((loopend > len) && (!loopstart)) loopend = 0;
		if (loopend > len) loopend = len;
		if (loopstart + 4 >= loopend) loopstart = loopend = 0;
		Ins[nins].nLength = len;
		Ins[nins].nLoopStart = loopstart;
		Ins[nins].nLoopEnd = loopend;
		if (loopend) Ins[nins].uFlags |= CHN_LOOP;
		memcpy(m_szNames[nins], psmp->filename, 13);
		Ins[nins].nVolume = 256;
		Ins[nins].nGlobalVol = 64;
		Ins[nins].nPan = 128;
	}
	// Song Message
	m_lpszSongComments = new char[109];
	memcpy(m_lpszSongComments, pfh->songmessage, 108);
	m_lpszSongComments[108] = 0;
	// Reading Orders
	memcpy(Order, pfh->orders, 128);
	m_nRestartPos = pfh->restartpos;
	if (Order[m_nRestartPos] >= pfh->patterns) m_nRestartPos = 0;
	// Reading Pattern Break Locations
	for (UINT npan=0; npan<8; npan++)
	{
		ChnSettings[npan].nPan = (npan & 1) ? 0x30 : 0xD0;
		ChnSettings[npan].nVolume = 64;
	}
	// Reading Patterns
	dwMemPos = 0x1F1 + pfh->samples * 25;
	for (UINT npat=0; npat<pfh->patterns; npat++)
	{
		Patterns[npat] = AllocatePattern(64, m_nChannels);
		if (!Patterns[npat]) break;
		PatternSize[npat] = 64;
		MODCOMMAND *m = Patterns[npat];
		const BYTE *p = lpStream + dwMemPos;
		for (UINT row=0; row<64; row++)
		{
			MODCOMMAND *mspeed = m;
			if ((row == pfh->breaks[npat]) && (row != 63))
			{
				for (UINT i=0; i<8; i++)
				{
					m[i].command = CMD_PATTERNBREAK;
					m[i].param = 0;
				}
			}
			for (UINT n=0; n<8; n++, m++, p+=3)
			{
				UINT note = p[0] >> 2;
				UINT instr = ((p[0] & 0x03) << 4) | (p[1] >> 4);
				UINT vol = p[1] & 0x0F;
				if (p[0] < 0xFE)
				{
					m->note = note + 37;
					m->instr = instr + 1;
				}
				if (p[0] <= 0xFE)
				{
					m->volcmd = VOLCMD_VOLUME;
					m->vol = (vol << 2) + 2;
				}
				if (p[2] != 0xFF)
				{
					UINT command = p[2] >> 4;
					UINT param = p[2] & 0x0F;
					switch(command)
					{
					case 0x00:	command = CMD_PORTAMENTOUP; break;
					case 0x01:	command = CMD_PORTAMENTODOWN; break;
					case 0x02:	command = CMD_TONEPORTAMENTO; break;
					case 0x03:	command = CMD_MODCMDEX; param |= 0x50; break;
					case 0x04:	command = CMD_VIBRATO; param |= 0x40; break;
					case 0x05:	if (param) command = CMD_SPEED; else command = 0; break;
					case 0x06:	if (param == 0) { command = CMD_PANNINGSLIDE; param = 0xFE; }
							else if (param == 1) { command = CMD_PANNINGSLIDE; param = 0xEF; }
							else command = 0;
							break;
					default:	command = 0;
					}
					if (command)
					{
						if (command == CMD_SPEED) mspeed = NULL;
						m->command = command;
						m->param = param;
					}
				}
			}
			if ((!row) && (mspeed))
			{
				for (UINT i=0; i<8; i++) if (!mspeed[i].command)
				{
					mspeed[i].command = CMD_SPEED;
					mspeed[i].param = pfh->tempolist[npat];
					break;
				}
			}
		}
		dwMemPos += 0x600;
	}
	// Reading Samples
	for (UINT n=1; n<=m_nSamples; n++)
	{
		UINT len = Ins[n].nLength;
		if (dwMemPos >= dwMemLength) break;
		if (len > 4) ReadSample(&Ins[n], RS_PCM8U, (LPSTR)(lpStream+dwMemPos), dwMemLength - dwMemPos);
		dwMemPos += len;
	}
	return TRUE;
}
/*

 MikMod Sound System

  By Jake Stine of Divine Entertainment (1996-2000)

 Support:
  If you find problems with this code, send mail to:
    air@divent.org

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 -----------------------------------------
 Module: LOAD_ABC

  ABC module loader.
	by Peter Grootswagers (2006)
	<email:pgrootswagers@planet.nl>

 Portability:
	All systems - all compilers (hopefully)
*/

#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <math.h>
#include <ctype.h>
#ifndef _WIN32
#include <unistd.h>  /* sleep() */
#endif

//#include "stdafx.h"
//#include "sndfile.h"

//#include "load_pat.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#define putenv _putenv
#define strdup _strdup
#endif

#define MAXABCINCLUDES 8
#define MAXCHORDNAMES 80
#define ABC_ENV_DUMPTRACKS	"MMABC_DUMPTRACKS"
#define ABC_ENV_NORANDOMPICK	"MMABC_NO_RANDOM_PICK"

// gchords use tracks with vpos 1 thru 7
// drums use track with vpos 8
// voice chords use vpos 0 and vpos from 11 up
#define GCHORDBPOS 1
#define GCHORDFPOS 2
#define GCHORDCPOS 3
#define DRUMPOS   8
#define DRONEPOS1 9
#define DRONEPOS2 10

// in the patterns a whole note at unmodified tempo is 16 rows
#define ROWSPERNOTE 16
// a 1/64-th note played in triool equals a 1/96-th note, to be able
// to play them and also to play the 1/64-th we need a resolution of 192
// because 2/192 = 1/96 and 3/192 = 1/64
#define RESOLUTION	192

/**********************************************************************/

#pragma pack(1)

typedef enum {
	note,
	octave,
	smpno,
	volume,
	effect,
	effoper
} ABCEVENT_X_NOTE;

typedef enum {
	none,
	trill,
	bow,
	accent
} ABCEVENT_X_EFFECT;

typedef enum {
	cmdflag,
	command,
	chordnum,
	chordnote,
	chordbase,
	jumptype
} ABCEVENT_X_CMD;

typedef enum {
	cmdsegno   = '$',
	cmdcapo    = 'B',
	cmdchord   = 'C',
	cmdfine    = 'F',
	cmdhide    = 'H',
	cmdjump    = 'J',
	cmdloop    = 'L',
	cmdcoda    = 'O',
	cmdpartbrk = 'P',
	cmdsync    = 'S',
	cmdtempo   = 'T',
	cmdvariant = 'V',
	cmdtocoda  = 'X'
} ABCEVENT_CMD;

typedef enum {
	jumpnormal,
	jumpfade,
	jumpdacapo,
	jumpdcfade,
	jumpdasegno,
	jumpdsfade,
	jumpfine,
	jumptocoda,
	jumpvariant,
	jumpnot
} ABCEVENT_JUMPTYPE;

typedef struct _ABCEVENT
{
	struct _ABCEVENT *next;
	uint32_t tracktick;
	union {
		uint8_t par[6];
		struct {
			uint8_t flg;
			uint8_t cmd;
			uint32_t lpar;	// for variant selections, bit pattern
		};
	};
	uint8_t part;
	uint8_t tiednote;
} ABCEVENT;

typedef struct _ABCTRACK
{
	struct _ABCTRACK *next;
	ABCEVENT *head;
	ABCEVENT *tail;
	ABCEVENT *capostart;
	ABCEVENT *tienote;
	int transpose;
	int octave_shift;
	uint32_t slidevoltime;	// for crescendo and diminuendo
	int slidevol; // -2:fade away, -1:diminuendo, 0:none, +1:crescendo
	uint8_t vno; // 0 is track is free for use, from previous song in multi-songbook
	uint8_t vpos; // 0 is main voice, other is subtrack for gchords, gchords or drumnotes
	uint8_t tiedvpos;
	uint8_t mute;
	uint8_t chan; // 10 is percussion channel, any other is melodic channel
	uint8_t volume;
	uint8_t instr;	// current instrument for this track
	uint8_t legato;
	char v[22];	// first twenty characters are significant
} ABCTRACK;

typedef struct _ABCMACRO
{
	struct _ABCMACRO *next;
	char *name;
	char *subst;
	char *n;
} ABCMACRO;

typedef struct _ABCHANDLE
{
	ABCMACRO *macro;
	ABCMACRO *umacro;
	ABCTRACK *track;
	long int pickrandom;
	unsigned int len;
	int speed;
	char *line;
	char *beatstring;
	uint8_t beat[4]; // a:first note, b:strong notes, c:weak notes, n:strong note every n
	char gchord[80];	// last setting for gchord
	char drum[80]; // last setting for drum
	char drumins[80]; // last setting for drum
	char drumvol[80]; // last setting for drum
	uint32_t barticks;
	// parse variables, declared here to avoid parameter pollution
	int abcchordvol, abcchordprog, abcbassvol, abcbassprog;
	int ktrans;
	int drumon, gchordon, droneon;
	int dronegm, dronepitch[2], dronevol[2];
	ABCTRACK *tp, *tpc, *tpr;
	uint32_t tracktime;
} ABCHANDLE;

#pragma pack()

/**********************************************************************/

static int global_voiceno, global_octave_shift, global_tempo_factor, global_tempo_divider;
static char global_part;
static uint32_t global_songstart;
/* Named guitar chords */
static char chordname[MAXCHORDNAMES][8];
static int chordnotes[MAXCHORDNAMES][6];
static int chordlen[MAXCHORDNAMES];
static int chordsnamed = 0;

static const char *sig[] = {
	" C D EF G A Bc d ef g a b",	// 7 sharps C#
	" C D EF G AB c d ef g ab ",	// 6 sharps F#
	" C DE F G AB c de f g ab ",	// 5 sharps B
	" C DE F GA B c de f ga b ",	// 4 sharps E
	" CD E F GA B cd e f ga b ",	// 3 sharps A
	" CD E FG A B cd e fg a b ",	// 2 sharps D
	" C D E FG A Bc d e fg a b",	// 1 sharps G
	" C D EF G A Bc d ef g a b",	// 0 sharps C
	" C D EF G AB c d ef g ab ",	// 1 flats  F
	" C DE F G AB c de f g ab ",	// 2 flats  Bb
	" C DE F GA B c de f ga b ",	// 3 flats  Eb
	" CD E F GA B cd e f ga b ",	// 4 flats  Ab
	" CD E FG A B cd e fg a b ",	// 5 flats  Db
	"C D E FG A Bc d e fg a b ",	// 6 flats  Gb
	"C D EF G A Bc d ef g a b ",	// 7 flats  Cb
// 0123456789012345678901234
};

static const char *keySigs[] = {
/* 0....:....1....:....2....:....3....:....4....:....5. */
	"7 sharps: C#    A#m   G#Mix D#Dor E#Phr F#Lyd B#Loc ",
	"6 sharps: F#    D#m   C#Mix G#Dor A#Phr BLyd  E#Loc ",
	"5 sharps: B     G#m   F#Mix C#Dor D#Phr ELyd  A#Loc ",
	"4 sharps: E     C#m   BMix  F#Dor G#Phr ALyd  D#Loc ",
	"3 sharps: A     F#m   EMix  BDor  C#Phr DLyd  G#Loc ",
	"2 sharps: D     Bm    AMix  EDor  F#Phr GLyd  C#Loc ",
	"1 sharp : G     Em    DMix  ADor  BPhr  CLyd  F#Loc ",
	"0 sharps: C     Am    GMix  DDor  EPhr  FLyd  BLoc  ",
	"1 flat  : F     Dm    CMix  GDor  APhr  BbLyd ELoc  ",
	"2 flats : Bb    Gm    FMix  CDor  DPhr  EbLyd ALoc  ",
	"3 flats : Eb    Cm    BbMix FDor  GPhr  AbLyd DLoc  ",
	"4 flats : Ab    Fm    EbMix BbDor CPhr  DbLyd GLoc  ",
	"5 flats : Db    Bbm   AbMix EbDor FPhr  GbLyd CLoc  ",
	"6 flats : Gb    Ebm   DbMix AbDor BbPhr CbLyd FLoc  ",
	"7 flats : Cb    Abm   GbMix DbDor EbPhr FbLyd BbLoc ",
	0
};

// local prototypes
static int abc_getnumber(const char *p, int *number);
static ABCTRACK *abc_locate_track(ABCHANDLE *h, const char *voice, int pos);
static void	abc_add_event(ABCHANDLE *h, ABCTRACK *tp, ABCEVENT *e);
static void abc_add_setloop(ABCHANDLE *h, ABCTRACK *tp, uint32_t tracktime);
static void abc_add_setjumploop(ABCHANDLE *h, ABCTRACK *tp, uint32_t tracktime, ABCEVENT_JUMPTYPE j);
static uint32_t abc_pattracktime(ABCHANDLE *h, uint32_t tracktime);
static int abc_patno(ABCHANDLE *h, uint32_t tracktime);


static int abc_isvalidchar(char c) {
	return(isalpha(c) || isdigit(c) || isspace(c) || c == '%' || c == ':');
}
#if 0
static const char *abc_skipspace(const char *p)
{
	while (*p && isspace(*p))
		p++;
	return p;
}
#endif

static void abc_extractkeyvalue(char *key, size_t key_max,
								char *value, size_t value_max, const char *src)
{
	while (*src && isspace(*src))
		src++;

	size_t key_size;
	for (key_size = 0; key_size < key_max - 1 && *src;) {
		if (*src == '=') {
			src++;
			break;
		}
		key[key_size++] = *src++;
	}
	while (key_size > 0 && isspace(key[key_size - 1]))
		key_size--;
	key[key_size] = '\0';

	while (*src && isspace(*src))
		src++;

	size_t value_size;
	for (value_size = 0; value_size < value_max - 1 && *src;)
		value[value_size++] = *src++;
	while (value_size > 0 && isspace(value[value_size - 1]))
		value_size--;
	value[value_size] = '\0';

	key[key_max-1] = '\0';
	value[value_max-1] = '\0';
}

static void abc_message(const char *s1, const char *s2)
{
	char txt[256];
	if( strlen(s1) + strlen(s2) > 255 ) return;
	sprintf(txt, s1, s2);
	fprintf(stderr, "load_abc > %s\n", txt);
}

static uint32_t modticks(uint32_t abcticks)
{
	return abcticks / RESOLUTION;
}

static uint32_t abcticks(uint32_t modticks)
{
	return modticks * RESOLUTION;
}

static uint32_t notelen_notediv_to_ticks(int speed, int len, int div)
{
	uint32_t u;
	if (div == 0) return 1;
	u = (ROWSPERNOTE * RESOLUTION * speed * len * global_tempo_factor) / (div * global_tempo_divider);
	return u;
}

static void abc_dumptracks(ABCHANDLE *h, const char *p)
{
	ABCTRACK *t;
	ABCEVENT *e;
	int n,pat,row,tck;
	char nn[3];
	if( !h ) return;
	for( t=h->track; t; t=t->next ) {
		printf("track %d.%d chan=%d %s\n", (int)(t->vno), (int)(t->vpos),
		                                   (int)(t->chan), (char *)(t->v));
		if( strcmp(p,"nonotes") )
			n = 1;
		else
			n = 0;
		for( e=t->head; e; e=e->next ) {
			tck = modticks(e->tracktick);
			row = tck / h->speed;
			pat = row / 64;
			tck = tck % h->speed;
			row = row % 64;
			nn[0] = ( e->tracktick % abcticks(h->speed * 64) ) ? ' ': '-';
			if( e->flg == 1 ) {
				printf("  %6d.%02d.%d%c%c %d.%d %s ",
				       pat, row, tck, nn[0], (int)(e->part), (int)(t->vno),
				       (int)(t->vpos), (char *)(t->v));
				if( e->cmd == cmdchord ) {
					nn[0] = "CCCDDEFFGGAABccddeffggaabb"[e->par[chordnote]];
					nn[1] = "b # #  # # #  # #  # # # #"[e->par[chordnote]];
					nn[2] = '\0';
					if( isspace(nn[1]) ) nn[1] = '\0';
					printf("CMD %c: gchord %s%s",
					       (char)(e->cmd), nn, chordname[e->par[chordnum]]);
					if( e->par[chordbase] != e->par[chordnote] ) {
						nn[0] = "CCCDDEFFGGAABccddeffggaabb"[e->par[chordbase]];
						nn[1] = "b # #  # # #  # #  # # # #"[e->par[chordbase]];
						nn[2] = '\0';
						printf("/%s", nn);
					}
					printf("\n");
				}
				else
					printf("CMD %c @%p 0x%08lX\n",
					       (char)(e->cmd), e,
					       (unsigned long)(e->lpar));
				if( strcmp(p,"nonotes") )
					n = 1;
				else
					n = 0;
			}
			else if( n ) {
				printf("  %6d.%02d.%d%c%c %d.%d %s ", pat, row, tck, nn[0], e->part, t->vno, t->vpos, t->v);
				if( e->par[note] ) {
					nn[0] = "CCCDDEFFGGAABccddeffggaabb"[e->par[note]-23];
					nn[1] = "b # #  # # #  # #  # # # #"[e->par[note]-23];
					nn[2] = '\0';
				}
				else strcpy(nn,"--");
				printf("NOTE %s octave %d inst %s vol %03d\n",
					nn, e->par[octave], pat_gm_name(pat_smptogm(e->par[smpno])),e->par[volume]);
				if( strcmp(p,"all") )
					n = 0;
			}
		}
	}
}


// =====================================================================================
static ABCEVENT *abc_new_event(ABCHANDLE *h, uint32_t abctick, const char data[])
// =====================================================================================
{
	ABCEVENT   *retval;
	int i;

	retval = (ABCEVENT *)_mm_calloc(h->trackhandle, 1,sizeof(ABCEVENT));
	retval->next        = NULL;
	retval->tracktick   = abctick;
	for( i=0; i<6; i++ )
	    retval->par[i]  = data[i];
	retval->part = global_part;
	retval->tiednote = 0;
	return retval;
}

// =============================================================================
static ABCEVENT *abc_copy_event(ABCHANDLE *h, ABCEVENT *se)
// =============================================================================
{
	ABCEVENT *e;
	e = (ABCEVENT *)_mm_calloc(h->trackhandle, 1,sizeof(ABCEVENT));
	e->next        = NULL;
	e->tracktick   = se->tracktick;
	e->flg         = se->flg;
	e->cmd         = se->cmd;
	e->lpar        = se->lpar;
	e->part        = se->part;
	return e;
}

// =============================================================================
static void abc_new_macro(ABCHANDLE *h, const char *m)
// =============================================================================
{
	ABCMACRO *retval;
	char key[256], value[256];
	abc_extractkeyvalue(key, sizeof(key), value, sizeof(value), m);

	retval = (ABCMACRO *)_mm_calloc(h->macrohandle, 1,sizeof(ABCMACRO));
	retval->name  = DupStr(h->macrohandle, key, strlen(key));
	retval->n     = strrchr(retval->name, 'n'); // for transposing macro's
	retval->subst = DupStr(h->macrohandle, value, strlen(value));
	retval->next  = h->macro;
	h->macro      = retval;
}

// =============================================================================
static void abc_new_umacro(ABCHANDLE *h, const char *m)
// =============================================================================
{
	ABCMACRO *retval, *mp;
	char key[256], value[256];
	abc_extractkeyvalue(key, sizeof(key), value, sizeof(value), m);
	if( strlen(key) > 1 || strchr("~HIJKLMNOPQRSTUVWXY",toupper(key[0])) == 0 ) return;
	while( char *q = strchr(key, '!') )
		*q = '+'; // translate oldstyle to newstyle
	if( !strcmp(key,"+nil+") ) { // delete a macro
		mp = NULL;
		for( retval=h->umacro; retval; retval = retval->next ) {
			if( retval->name[0] == key[0] ) {	// delete this one
				if( mp ) mp->next = retval->next;
				else h->umacro = retval->next;
				_mm_free(h->macrohandle, retval);
				return;
			}
			mp = retval;
		}
		return;
	}
	retval = (ABCMACRO *)_mm_calloc(h->macrohandle, 1,sizeof(ABCMACRO));
	retval->name  = DupStr(h->macrohandle, key, 1);
	retval->subst = DupStr(h->macrohandle, value, strlen(value));
	retval->n     = 0;
	retval->next  = h->umacro; // by placing it up front we mask out the old macro until we +nil+ it
	h->umacro      = retval;
}

// =============================================================================
static ABCTRACK *abc_new_track(ABCHANDLE *h, const char *voice, int pos)
// =============================================================================
{
	ABCTRACK *retval;
	if( !pos ) global_voiceno++;
	retval = (ABCTRACK *)_mm_calloc(h->trackhandle, 1,sizeof(ABCTRACK));
	retval->next         = NULL;
	retval->vno          = global_voiceno;
	retval->vpos         = pos;
	retval->tiedvpos     = pos;
	retval->instr        = 1;
	strncpy(retval->v, voice, 20);
	retval->v[20]        = '\0';
	retval->head         = NULL;
	retval->tail         = NULL;
	retval->capostart    = NULL;
	retval->tienote      = NULL;
	retval->mute         = 0;
	retval->chan         = 0;
	retval->transpose    = 0;
	retval->volume       = h->track? h->track->volume: 120;
	retval->slidevoltime = 0;
	retval->slidevol     = 0;
	retval->legato       = 0;
	return retval;
}

static int abc_numtracks(ABCHANDLE *h)
{
	int n;
	ABCTRACK *t;
	n=0;
	for( t = h->track; t; t=t->next )
		n++;
	return n;
}

static int abc_interval(const char *s, const char *d)
{
	const char *p;
	int i,j,k;
	int n,oct,m[2];
	for( j=0; j<2; j++ ) {
		if( j ) p = d;
		else p = s;
		switch(p[0]) {
			case '^':
				n = p[1];
				i = 2;
				break;
			case '_':
				n = p[1];
				i = 2;
				break;
			case '=':
				n = p[1];
				i = 2;
				break;
			default:
				n = p[0];
				i = 1;
				break;
		}
		for( k=0; k<25; k++ )
			if( n == sig[7][k] )
				break;
		oct = 4;	// ABC note pitch C is C4 and pitch c is C5
		if( k > 12 ) {
			oct++;
			k -= 12;
		}
		while( p[i] == ',' || p[i] == '\'' ) {
			if( p[i] == ',' )
				oct--;
			else
				oct++;
			i++;
		}
		m[j] = k + 12 * oct;
	}
	return m[0] - m[1];
}

static int abc_transpose(const char *v)
{
	int i,j,t;
	const char *m = "B", *mv = "";
	t = 0;
	global_octave_shift = 99;
	for( ; *v && *v != ']'; v++ ) {
		if( !strncasecmp(v,"t=",2) ) {
			v+=2;
			if( *v=='-' )	{
				j = -1;
				v++;
			}
			else j = 1;
			v+=abc_getnumber(v,&i);
			t += i * j;
			global_octave_shift = 0;
		}
		if( !strncasecmp(v,"octave=",7) ) {
			v+=7;
			if( *v=='-' )	{
				j = -1;
				v++;
			}
			else j = 1;
			v+=abc_getnumber(v,&i);
			t += i * j * 12;
			global_octave_shift = 0;
		}
		if( !strncasecmp(v,"transpose=",10) ) {
			v+=10;
			if( *v=='-' )	{
				j = -1;
				v++;
			}
			else j = 1;
			v+=abc_getnumber(v,&i);
			t += i * j;
			global_octave_shift = 0;
		}
		if( !strncasecmp(v,"octave=",7) ) { // used in kv304*.abc
			v+=7;
			if( *v=='-' )	{
				j = -1;
				v++;
			}
			else j = 1;
			v+=abc_getnumber(v,&i);
			t += i * j * 12;
			global_octave_shift = 0;
		}
		if( !strncasecmp(v,"m=",2) ) {
			v += 2;
			mv = v; // get the pitch for the middle staff line
			while( *v && *v != ' ' && *v != ']' ) v++;
			global_octave_shift = 0;
		}
		if( !strncasecmp(v,"middle=",7) ) {
			v += 7;
			mv = v; // get the pitch for the middle staff line
			while( *v && *v != ' ' && *v != ']' ) v++;
			global_octave_shift = 0;
		}
		if( !strncasecmp(v,"clef=",5) )
			v += 5;
		j = 1;
		if( !strncasecmp(v,"treble",6) ) {
			j = 0;
			v += 6;
			switch( *v ) {
				case '1':	v++; m = "d";	break;
				case '2': v++;
				default: m = "B";	break;
				case '3': v++; m = "G";	break;
				case '4': v++; m = "E";	break;
				case '5': v++; m = "C";	break;
			}
			global_octave_shift = 0;
		}
		if( j && !strncasecmp(v,"bass",4) ) {
			j = 0;
			v += 4;
			switch( *v ) {
				case '1':	v++; m = "C";	break;
				case '2': v++; m = "A,";	break;
				case '3': v++; m = "F,";	break;
				case '4': v++;
				default: m = "D,";	break;
				case '5': v++; m = "B,,";	break;
			}
			if( global_octave_shift == 99 )
				global_octave_shift = -2;
		}
		if( j && !strncasecmp(v,"tenor",5) ) {
			j = 0;
			v += 5;
			switch( *v ) {
				case '1':	v++; m = "G";	break;
				case '2': v++; m = "E";	break;
				case '3': v++; m = "C";	break;
				case '4': v++;
				default: m = "A,";	break;
				case '5': v++; m = "F,";	break;
			}
			if( global_octave_shift == 99 )
				global_octave_shift = 1;
		}
		if( j && !strncasecmp(v,"alto",4) ) {
			j = 0;
			v += 4;
			switch( *v ) {
				case '1':	v++; m = "G";	break;
				case '2': v++; m = "E";	break;
				case '3': v++;
				default: m = "C";	break;
				case '4': v++; m = "A,";	break;
				case '5': v++; m = "F,";	break;
			}
			if( global_octave_shift == 99 )
				global_octave_shift = 1;
		}
		if( j && strchr("+-",*v) && *v && v[1]=='8' ) {
			switch(*v) {
				case '+':
					t += 12;
					break;
				case '-':
					t -= 12;
					break;
			}
			v += 2;
			if( !strncasecmp(v,"va",2) ) v += 2;
			global_octave_shift = 0;
			j = 0;
		}
		if( j ) {
			while( *v && *v != ' ' && *v != ']' ) v++;
		}

		// Already reached the end of the string..
		if (!*v)
			break;
	}
	if( strlen(mv) > 0 ) // someone set the middle note
		t += abc_interval(mv, m);
	if( global_octave_shift == 99 )
		global_octave_shift = 0;
	return t;
}

// =============================================================================
static ABCTRACK *abc_locate_track(ABCHANDLE *h, const char *voice, int pos)
// =============================================================================
{
	ABCTRACK *tr, *prev, *trunused;
	char vc[21];
	int i, trans=0, voiceno=0, instrno = 1, channo = 0;
	for( ; *voice == ' '; voice++ ) ;	// skip leading spaces
	for( i=0; i+1 < (int) sizeof(vc) && *voice && *voice != ']' && *voice != '%' && !isspace(*voice); voice++ )	// can work with inline voice instructions
		vc[i++] = *voice;
	vc[i] = '\0';
	prev = NULL;
	trunused = NULL;
	if( !pos )	trans = abc_transpose(voice);
	for( tr=h->track; tr; tr=tr->next ) {
		if( tr->vno == 0 ) {
			if( !trunused ) trunused = tr; // must reuse mastertrack (h->track) as first
		}
		else {
			if( !strncasecmp(tr->v, vc, 20) ) {
				if( tr->vpos == pos )
					return tr;
				trans = tr->transpose;
				global_octave_shift = tr->octave_shift;
				voiceno = tr->vno;
				instrno = tr->instr;
				channo  = tr->chan;
			}
		}
		prev = tr;
	}
	if( trunused ) {
		tr = trunused;
		if( pos ) {
			tr->vno   = voiceno;
			tr->instr = instrno;
			tr->chan  = channo;
		}
		else {
			global_voiceno++;
			tr->vno   = global_voiceno;
			tr->instr = 1;
			tr->chan  = 0;
		}
		tr->vpos         = pos;
		tr->tiedvpos     = pos;
		strncpy(tr->v, vc, 20);
		tr->v[20]        = '\0';
		tr->mute         = 0;
		tr->transpose    = trans;
		tr->octave_shift = global_octave_shift;
		tr->volume       = h->track->volume;
		tr->tienote      = NULL;
		tr->legato       = 0;
		return tr;
	}
	tr = abc_new_track(h, vc, pos);
	if( pos ) {
		tr->vno   = voiceno;
		tr->instr = instrno;
		tr->chan  = channo;
	}
	tr->transpose    = trans;
	tr->octave_shift = global_octave_shift;
	if( prev ) prev->next = tr;
	else	h->track = tr;
	return tr;
}

// =============================================================================
static ABCTRACK *abc_check_track(ABCHANDLE *h, ABCTRACK *tp)
// =============================================================================
{
	if( !tp ) {
		tp = abc_locate_track(h, "", 0);	// must work for voiceless abc too...
		tp->transpose = h->ktrans;
	}
	return tp;
}

static void abc_add_capo(ABCHANDLE *h, ABCTRACK *tp, uint32_t tracktime)
{
	ABCEVENT *e;
	char d[6];
	d[0] = d[1] =	d[2] = d[3] = d[4] = d[5] = 0;
	d[cmdflag] = 1;
	d[command] = cmdcapo;
	e = abc_new_event(h, tracktime, d);
	tp->capostart = e;
	abc_add_event(h, tp, e); // do this last (recursion danger)
}

static void abc_add_segno(ABCHANDLE *h, ABCTRACK *tp, uint32_t tracktime)
{
	ABCEVENT *e;
	char d[6];
	d[0] = d[1] =	d[2] = d[3] = d[4] = d[5] = 0;
	d[cmdflag] = 1;
	d[command] = cmdsegno;
	e = abc_new_event(h, tracktime, d);
	abc_add_event(h, tp, e);
}

static void abc_add_coda(ABCHANDLE *h, ABCTRACK *tp, uint32_t tracktime)
{
	ABCEVENT *e;
	char d[6];
	d[0] = d[1] =	d[2] = d[3] = d[4] = d[5] = 0;
	d[cmdflag] = 1;
	d[command] = cmdcoda;
	e = abc_new_event(h, tracktime, d);
	abc_add_event(h, tp, e);
}

static void abc_add_fine(ABCHANDLE *h, ABCTRACK *tp, uint32_t tracktime)
{
	ABCEVENT *e;
	char d[6];
	d[0] = d[1] =	d[2] = d[3] = d[4] = d[5] = 0;
	d[cmdflag] = 1;
	d[command] = cmdfine;
	e = abc_new_event(h, tracktime, d);
	abc_add_event(h, tp, e);
}

static void abc_add_tocoda(ABCHANDLE *h, ABCTRACK *tp, uint32_t tracktime)
{
	ABCEVENT *e;
	char d[6];
	d[0] = d[1] =	d[2] = d[3] = d[4] = d[5] = 0;
	d[cmdflag] = 1;
	d[command] = cmdtocoda;
	e = abc_new_event(h, tracktime, d);
	abc_add_event(h, tp, e);
}

// first track is dirigent, remove all control events from other tracks
// to keep the information where the events should be relative to note events
// in the same tick the ticks are octated and four added for note events
// the control events that come before the note events get a decremented tick,
// those that come after get an incremented tick, for example:
//             ctrl ctrl note ctrl ctrl  note
//   original: t    t    t    t    t+1   t+1
//   recoded:  8t+1 8t+2 8t+4 8t+5 8t+11 8t+12
static void abc_remove_unnecessary_events(ABCHANDLE *h)
{
	ABCTRACK *tp,*ptp;
	ABCEVENT *ep, *el;
	uint32_t ct, et;
	int d;
	ptp = NULL;
	for( tp=h->track; tp; tp=tp->next ) {
		el = NULL;
		ep = tp->head;
		ct = 0;
		d  = -3;
		while( ep ) {
			et = ep->tracktick;
			ep->tracktick <<= 3;
			ep->tracktick += 4;
			if( ep->flg == 1 ) {
				ep->tracktick += d;
				d++;
				if( d == 0 ) d = -1;
				if( d == 4 ) d =  3;
				if( tp!=h->track ) ep->cmd = cmdhide;
				switch( ep->cmd ) {
					case cmdhide:
					case cmdsync:
						if( el ) {
							el->next = ep->next;
							if( !el->next )
								tp->tail = el;
							_mm_free(h->trackhandle,ep);
							ep = el->next;
						}
						else {
							tp->head = ep->next;
							if( !tp->head )
								tp->tail = NULL;
							_mm_free(h->trackhandle,ep);
							ep = tp->head;
						}
						break;
					default:
						el = ep;
						ep = ep->next;
						break;
				}
			}
			else {
				el = ep;
				ep = ep->next;
				d  = 1;
			}
			if( et > ct )
				d = -3;
			ct = et;
		}
		if( !tp->head ) { // no need to keep empty tracks...
			if( ptp ) {
				ptp->next = tp->next;
				_mm_free(h->trackhandle,tp);
				tp = ptp;
			}
			else if (tp->next) {
				h->track = tp->next;
				_mm_free(h->trackhandle,tp);
				tp = h->track;
			} else {
				break;
			}
		}
		ptp = tp;	// remember previous track
	}
}

// set ticks back, and handle partbreaks
static void abc_retick_events(ABCHANDLE *h)
{
	ABCTRACK *tp;
	ABCEVENT *ep;
	uint32_t et, tt=0, at = abcticks(64 * h->speed);
	for( tp=h->track; tp; tp=tp->next ) {
		// make ticks relative
		tt = 0;
		for( ep=tp->head; ep; ep=ep->next ) {
			et = ep->tracktick >> 3;
			ep->tracktick = et - tt;
			tt = et;
		}
		// make ticks absolute again, skipping no-op partbreaks
		tt = 0;
		for( ep=tp->head; ep; ep=ep->next ) {
			ep->tracktick += tt;
			tt = ep->tracktick;
			if( ep->flg == 1 && ep->cmd == cmdpartbrk ) {
				if( tt % at ) {
					tt += at;
					tt /= at;
					tt *= at;
					ep->tracktick -= abcticks(h->speed); // break plays current row
				}
				else ep->cmd = cmdhide;
			}
		}
	}
}

// make sure every track has the control events it needs, this way it is not
// necessary to have redundant +segno+ +D.C.+ etc in the voices, the first voice
// is the master, it is pointed to by the member 'track' in the ABCHANDLE
static void abc_synchronise_tracks(ABCHANDLE *h)
{
	ABCTRACK *tp;
	uint32_t tm;	// tracktime in master
	ABCEVENT *em, *es, *et, *ec;	// events in master, slave, slave temporary and copied event
	if( !h || !h->track ) return;
	abc_remove_unnecessary_events(h);
	for( tp = h->track->next; tp; tp = tp->next ) {
		for( em=h->track->head; em; em=em->next ) {
			if( em->flg == 1 ) { // some kind of control event
				switch( em->cmd ) {
					case cmdchord:
					case cmdhide:
					case cmdtempo:
					case cmdsync:
						break;
					default:	// check to see if copy is necessary
						ec = abc_copy_event(h, em);
						tm = em->tracktick;
						es = tp->head; // allways search from the begin...
						for( et=es; et && et->tracktick <= tm; et=et->next )
							es = et;
						if( es == NULL || es->tracktick > tm ) {	// special case: head of track
							ec->next = es;
							tp->head = ec;
						}
						else {
							ec->next = es->next;
							es->next = ec;
						}
						break;
				}
			}
		}
	}
	abc_retick_events(h);
}

static void	abc_add_event(ABCHANDLE *h, ABCTRACK *tp, ABCEVENT *e)
{
	if( !tp->capostart ) abc_add_capo(h, tp, global_songstart);
	if( tp->tail ) {
		tp->tail->next = e;
		tp->tail = e;
	}
	else {
		tp->head = e;
		tp->tail = e;
	}
}

static void abc_add_partbreak(ABCHANDLE *h, ABCTRACK *tp, uint32_t tracktime)
{
	ABCEVENT *e;
	char d[6];
	d[0] = d[1] =	d[2] = d[3] = d[4] = d[5] = 0;
	d[cmdflag] = 1;
	d[command] = cmdpartbrk;
	e = abc_new_event(h, tracktime, d);
	abc_add_event(h, tp, e);
}

static void abc_add_tempo_event(ABCHANDLE *h, ABCTRACK *tp, uint32_t tracktime, int tempo)
{
	ABCEVENT *e;
	char d[6];
	d[0] = d[1] =	d[2] = d[3] = d[4] = d[5] = 0;
	d[cmdflag] = 1;
	d[command] = cmdtempo;
	e = abc_new_event(h, tracktime, d);
	e->lpar = tempo;
	abc_add_event(h, tp, e);
}

static void abc_add_noteoff(ABCHANDLE *h, ABCTRACK *tp, uint32_t tracktime)
{
	ABCEVENT *e;
	char d[6];
	d[note]    = 0;
	d[octave]  = 0;
	d[smpno]   = pat_gmtosmp(tp->instr);
	d[volume]  = 0;
	d[effect]  = 0;
	d[effoper] = 0;
	e = abc_new_event(h, tracktime, d);
	abc_add_event(h, tp, e);
}

static int abc_dynamic_volume(ABCTRACK *tp, uint32_t tracktime, int vol)
{
	uint32_t slidetime;
	int voldelta;
	if( tp->mute ) return 0;
	if( tp->slidevol == 0 ) return vol;
	if( tracktime < tp->slidevoltime ) return vol;
	slidetime = modticks(tracktime - tp->slidevoltime);
	voldelta = (slidetime * 15) / 64 / 6;	// slide from say mf up to f in one pattern's time
	if( tp->slidevol > -2 && voldelta > 15 ) voldelta = 15;	// never to much dynamics
	if( tp->slidevol > 0 ) vol += voldelta;
	else vol -= voldelta;
	if( vol < 2 ) vol = 2; // xmms divides this by 2....
	if( vol > 127 ) vol = 127;
	return vol;
}

static void abc_track_untie_short_chordnotes(ABCHANDLE *h)
{
	ABCTRACK *tp;
	int vn;
	tp = h->tp;
	vn = tp->vno;
	for( tp = h->track; tp; tp = tp->next )
		if( tp != h->tp && tp->vno == vn && tp->tienote ) {
			abc_message("short notes in chord can not be tied:\n%s", h->line);
			tp->tienote = 0;
		}
}

static void abc_track_clear_tiednote(ABCHANDLE *h)
{
	ABCTRACK *tp;
	int vn;
	tp = h->tp;
	vn = tp->vno;
	for( tp = h->track; tp; tp = tp->next )
		if( tp->vno == vn ) tp->tienote = 0;
}

static void abc_track_clear_tiedvpos(ABCHANDLE *h)
{
	ABCTRACK *tp;
	int vn;
	tp = h->tp;
	vn = tp->vno;
	for( tp = h->track; tp; tp = tp->next )
		if( tp->vno == vn ) tp->tiedvpos = tp->vpos;
}

static ABCTRACK *abc_track_with_note_tied(ABCHANDLE *h, uint32_t tracktime, int n, int oct)
{
	int vn, vp;
	ABCTRACK *tp;
	ABCEVENT *e;
	tp = h->tp;
	vn = tp->vno;
	vp = tp->vpos;
	for( tp = h->track; tp; tp = tp->next ) {
		if( tp->vno == vn ) {
			e = tp->tienote;
			if( e && e->tracktick < tracktime
			&& e->par[octave] == oct && abs(e->par[note] - n) < 3 ) {
				if( tp->vpos != vp ) tp->tiedvpos = vp;
				h->tp = tp;
				return tp;
			}
		}
	}
	tp = h->tp;
	vp = tp->tiedvpos;
	if( tp->vpos != vp ) {
		// chord note track already returned in previous call
		for( tp = h->track; tp; tp = tp->next ) {
			if( tp->vno == vn && tp->vpos == vp ) {
				tp->tiedvpos = h->tp->vpos;
				h->tp = tp;
				return tp;
			}
		}
	}
	return h->tp;
}

static int abc_add_noteon(ABCHANDLE *h, int ch, const char *p, uint32_t tracktime, char *barkey, int vol, ABCEVENT_X_EFFECT fx, int fxop)
{
	ABCEVENT *e;
	ABCTRACK *tp;
	int i,j,k;
	int n,oct;
	char d[6];
	tp = h->tp;
	switch(ch) {
		case '^':
			if( p[0] == '^' ) {
				n = p[1];
				i = 2;
				ch = 'x';
			}
			else {
				n = p[0];
				i = 1;
			}
			break;
		case '_':
			if( p[0] == '_' ) {
				n = p[1];
				i = 2;
				ch = 'b';
			}
			else {
				n = p[0];
				i = 1;
			}
			break;
		case '=':
			n = p[0];
			i = 1;
			break;
		default:
			n = ch;
			i = 0;
			break;
	}
	// nothing good inside - early exit
	if ( n == 0 )
		return 0;
	for( k=0; k<51; k++ ) {
		if( n == barkey[k] )
			break;
	}
	j = k;
	if( k > 24 )
		k -= 25; // had something like A# over Bb key F signature....
	if( i ) {
		// propagate accidentals if necessary
		// DON'T do redundant accidentals they're always relative to C-scale
		for( k=0; k<25; k++ ) {
			if( n == sig[7][k] )
				break;
		}
		if( k < 25 ) { // only do real notes...
		switch(ch) {
			case 'x':
				k++;
			case '^':
				k++;
				break;
			case 'b':
				k--;
			case '_':
				k--;
				break;
			case '=':
						break;
				}
			if( j < 25 ) // was it not A# over Bb?
			barkey[j] = ' ';
			if ( k >= 0 )
				barkey[k] = n;
		}
	}
	oct = 3;	// ABC note pitch C is C4 and pitch c is C5
	if( k < 25 ) {
		k += tp->transpose;
		while( k > 12 ) {
			oct++;
			k -= 12;
		}
		while( k < 0 ) {
			oct--;
			k += 12;
		}
		d[note] = 23 + k;	// C0 is midi notenumber 24
	}
	else
		d[note] = 0; // someone has doen ^X3 or something like it...
	while( p[i] && strchr(",'",p[i]) ) {
		if( p[i]==',' ) oct--;
		else oct++;
		i++;
		tp->octave_shift = 0;	// forget we ever had to look at it
	}
	if( tp->octave_shift )
		tp->transpose += 12 * tp->octave_shift;
	oct += tp->octave_shift;
	tp->octave_shift = 0;	// after the first note we never have to look at it again
	if( oct < 0 ) oct = 0;
	if( oct > 9 ) oct = 9;
	d[octave]  = oct;
	d[smpno]   = pat_gmtosmp(tp->instr);
	d[volume]  = abc_dynamic_volume(tp, tracktime, vol);
	d[effect]  = fx; // effect
	d[effoper] = fxop;
	tp = abc_track_with_note_tied(h, tracktime, d[note], oct);
	if( tp->tienote ) {
		if( tp->tienote->par[note] != d[note] ) {
			if( abs(tp->tienote->par[note] - d[note]) < 3 ) {
				// may be tied over bar symbol, recover local accidental to barkey
				k = tp->tienote->par[note] - 23 - tp->transpose;
				while( k < 0 ) k += 12;
				while( k > 12 ) k -= 12;
				if( (isupper(n) && barkey[k+12] == ' ') || (islower(n) && barkey[k] == ' ') ) {
					barkey[j] = ' ';
					if( isupper(n) )
						barkey[k] = n;
					else
						barkey[k+12] = n;
					d[note]   = tp->tienote->par[note];
					d[octave] = tp->tienote->par[octave];
				}
			}
		}
	}
	if( tp->tienote
	  && tp->tienote->par[note]   == d[note]
	  && tp->tienote->par[octave] == d[octave] ) {
		for( e = tp->tienote; e; e = e->next ) {
			if( e->par[note] == 0 && e->par[octave] == 0 ) {	// undo noteoff
				e->flg = 1;
				e->cmd = cmdhide;
				e->lpar = 0;
				break;
			}
		}
		tp->tienote->tiednote = 1; // mark him for the pattern writers
		for( j=i; isdigit(p[j]) || p[j]=='/'; j++ ) ; // look ahead to see if this one is tied too
		if( p[j] != '-' ) // is this note tied too?
			tp->tienote = NULL; // if not the tie ends here...
		return i;
	}
	tp->tienote = NULL;
	if( tp->tail
	  && tp->tail->tracktick == tracktime
	  && tp->tail->par[note]      == 0
	  && tp->tail->par[octave]    == 0 ) {
		for( j=0; j<6; j++ )
		     tp->tail->par[j] = d[j];
	}
	else {
		e = abc_new_event(h, tracktime, d);
		abc_add_event(h, tp, e);
	}
	if( i > 0 && p[i-1] == '"' ) {
		i--; // someone coded a weird note like ^"E"
		abc_message("strange note encountered scanning %s", h->line);
	}
	return i;
}

static void abc_add_dronenote(ABCHANDLE *h, ABCTRACK *tp, uint32_t tracktime, int nnum, int vol)
{
	ABCEVENT *e;
	int j,k;
	int oct;
	char d[6];
	oct = -1;	// ABC note pitch C is C4 and pitch c is C5
	k = nnum + 1;
	while( k > 12 ) {
		oct++;
		k -= 12;
	}
	while( k < 0 ) {
		oct--;
		k += 12;
	}
	if( oct < 0 ) oct = 0;
	d[note] = 23 + k;	// C0 is midi notenumber 24
	d[octave]  = oct;
	d[smpno]   = pat_gmtosmp(tp->instr);
	d[volume]  = abc_dynamic_volume(tp, tracktime, vol);
	d[effect]  = 0; // effect
	d[effoper] = 0;
	if( tp->tail
	  && tp->tail->tracktick == tracktime
	  && tp->tail->par[note]      == 0
	  && tp->tail->par[octave]    == 0 ) {
		for( j=0; j<6; j++ )
		     tp->tail->par[j] = d[j];
	}
	else {
		e = abc_new_event(h, tracktime, d);
		abc_add_event(h, tp, e);
	}
}

static void abc_add_chordnote(ABCHANDLE *h, ABCTRACK *tp, uint32_t tracktime, int nnum, int vol)
{
	abc_add_dronenote(h, tp, tracktime, nnum + 23, tp->mute? 0: vol);
}

static void abc_add_drumnote(ABCHANDLE *h, ABCTRACK *tp, uint32_t tracktime, int nnum, int vol)
{
	abc_add_dronenote(h, tp, tracktime, nnum, tp->mute? 0: vol);
}

static void abc_add_variant_start(ABCHANDLE *h, ABCTRACK *tp, uint32_t tracktime, int n)
{
	ABCEVENT *e;
	char d[6];
	d[0] = d[1] =	d[2] = d[3] = d[4] = d[5] = 0;
	d[cmdflag] = 1;
	d[command] = cmdvariant;
	e = abc_new_event(h, tracktime, d);
	e->lpar = 1<<n;
	abc_add_event(h, tp, e);
}

static void abc_add_variant_choise(ABCTRACK *tp, int n)
{
  tp->tail->lpar |= 1<<n;
}

static void	abc_add_chord(const char *p, ABCHANDLE *h, ABCTRACK *tp, uint32_t tracktime)
{
	ABCEVENT *e;
	char d[6];
	char s[8];
	int i;
	const char *n = " C D EF G A Bc d ef g a b";
	d[0] = d[1] =	d[2] = d[3] = d[4] = d[5] = 0;
	d[cmdflag] = 1;
	d[command] = cmdchord;
	if( p[0] == '(' ) p++;	// chord between parens like: (C)
	for( i=0; n[i]; i++ )
		if( *p == n[i] ) {
			d[chordnote] = i;
			break;
		}
	if (*p) p++;
	switch(*p) {
		case 'b':
			d[chordnote]--;
			p++;
			break;
		case '#':
			d[chordnote]++;
			p++;
			break;
	}
	d[chordbase] = d[chordnote];
	for( i=0; i < (int)sizeof(s) - 1 && p[i] && p[i] != '"' && p[i] != '/' && p[i] != '(' && p[i] != ')' && p[i] != ' '; i++ )
		s[i] = p[i];
	s[i] = '\0';
	p = &p[i];
	if( *p=='/' ) {
		p++;
		for( i=0; n[i]; i++ )
			if( *p == n[i] ) {
				d[chordbase] = i;
				break;
			}
		if (*p) p++;
		switch(*p) {
			case 'b':
				d[chordbase]--;
				p++;
				break;
			case '#':
				d[chordbase]++;
				p++;
				break;
		}
	}
	for( i=0; i<chordsnamed; i++ )
		if( !strcmp(s, chordname[i]) ) {
			d[chordnum] = i;
			break;
		}
	if( i==chordsnamed ) {
		abc_message("Failure: unrecognized chordname %s",s);
		return;
	}
	e = abc_new_event(h, tracktime, d);
	abc_add_event(h, tp, e);
}

static void abc_add_setloop(ABCHANDLE *h, ABCTRACK *tp, uint32_t tracktime)
{
	ABCEVENT *e;
	char d[6];
	d[0] = d[1] =	d[2] = d[3] = d[4] = d[5] = 0;
	d[cmdflag] = 1;
	d[command] = cmdloop;
	e = abc_new_event(h, tracktime, d);
	abc_add_event(h, tp, e);
}

static void abc_fade_track(ABCTRACK *tp, ABCEVENT *e)
{
	while(e) {
		if( e->flg != 1 && e->par[note] != 0 )
			e->par[volume] = abc_dynamic_volume(tp, e->tracktick, e->par[volume]);
		e = e->next;
	}
}

static void abc_add_setjumploop(ABCHANDLE *h, ABCTRACK *tp, uint32_t tracktime, ABCEVENT_JUMPTYPE j)
{
	ABCEVENT *e;
	char d[8];
	d[0] = d[1] =	d[2] = d[3] = d[4] = d[5] = 0;
	d[cmdflag] = 1;
	d[command] = cmdjump;
	d[jumptype] = j;
	e = abc_new_event(h, tracktime, d);
	abc_add_event(h, tp, e);
}

static void abc_add_sync(ABCHANDLE *h, ABCTRACK *tp, uint32_t tracktime)
{
	ABCEVENT *e;
	char d[6];
	e = tp->tail;
	if( e && e->tracktick == tracktime ) return;
	if( e && e->flg == 1 && e->cmd == cmdsync ) {
		e->tracktick = tracktime;
		return;
	}
	d[0] = d[1] =	d[2] = d[3] = d[4] = d[5] = 0;
	d[cmdflag] = 1;
	d[command] = cmdsync;
	e = abc_new_event(h, tracktime, d);
	abc_add_event(h, tp, e);
}

static void abc_add_gchord_syncs(ABCHANDLE *h, ABCTRACK *tpc, uint32_t tracktime)
{
	ABCTRACK *tp;
	int i;
	for( i = GCHORDBPOS; i < DRUMPOS; i++ ) {
		tp = abc_locate_track(h, tpc->v, i);
		abc_add_sync(h,tp,tracktime);
	}
}

static void abc_add_drum_sync(ABCHANDLE *h, ABCTRACK *tpr, uint32_t tracktime)
{
	ABCTRACK *tp;
	tp = abc_locate_track(h, tpr->v, DRUMPOS);
	abc_add_sync(h,tp,tracktime);
}

static int abc_getnumber(const char *p, int *number)
{
	int i,h;
	i = 0;
	h = 0;
	while( isdigit(p[i]) ) {
		if (i < 9)
			h = 10 * h + p[i] - '0';
		i++;
	}
	if( i==0 )
		*number = 1;
	else
		*number = h;
	return i;
}

static int abc_getexpr(const char *p, int *number)
{
	int i, term, total;
	i = 0;
	while( isspace(p[i]) )
		i++;
	if( p[i] == '(' ) {
		i += abc_getexpr(p+i+1, number);
		while( p[i] && (p[i] != ')') )
			i++;
		return i;
		}
	i += abc_getnumber(p+i, &total);
	while( isspace(p[i]) )
		i++;
	while( p[i] == '+' ) {
		i += 1 + abc_getexpr(p+i+1, &term);
		total += term;
		while( isspace(p[i]) )
			i++;
	}
	*number = total;
	return i;
}

static int abc_notelen(const char *p, int *len, int *div)
{
	int i,h,k;
	i = abc_getnumber(p,len);
	h = 1;
	while( p[i] == '/' ) {
		h *= 2;
		i++;
	}
	if( isdigit(p[i]) ) {
		h /= 2;
		i += abc_getnumber(p+i,&k);
	}
	else k = 1;
	*div = h * k;
	if ( *div > 4096 ) *div = 4096;
	if ( *div <= 0 ) *div = 1;
	return i;
}

static int abc_brokenrithm(const char *p, int *nl, int *nd, int *b, int hornpipe)
{
	switch( *b ) {
		case '<':
			*nl *= 3;
			*nd *= 2;
			hornpipe = 0;
			break;
		case '>':
			*nd *= 2;
			hornpipe = 0;
			break;
	}
	*b = *p;
	switch( *b ) {
		case '>':
			*nl *= 3;
			*nd *= 2;
			return 1;
		case '<':
			*nd *= 2;
			return 1;
		default:
			*b = 0;
			break;
	}
	if( hornpipe ) { // still true then make 1/8 notes broken rithme
		if( *nl == 1 && *nd == 1 ) {
			*b = '>';
			*nl = 3;
			*nd = 2;
		}
	}
	return 0;
}

// put p notes in the time q for the next r notes
static int abc_tuplet(int *nl, int *nd, int p, int q, int r)
{
	if( !r ) return 0;
	*nl *= q;
	*nd *= p;
	return r - 1;
}

// evaluate [Q:"string" n1/m1 n2/m2 n3/m3 n4/m4=bpm "string"]
// minimal form [Q:"string"]
// most used form [Q: 1/4=120]
static int abc_extract_tempo(const char *p, int invoice)
{
	int nl, nd, ns, in, tempo;
	int nl1=0, nd1, notes, state;
	const char *q;
	in = 0;
	nl = 0;
	nd = 1;
	ns = 120;
	notes = 0;
	state = 0;
	for( q=p; *q; q++ ) {
		if( in ) {
			if( *q=='"' )
				in = 0;
		}
		else {
			if( *q == ']' ) break;
			switch( *q ) {
				case '"':
					in = 1;
					break;
				case '/':
					notes++;
					state = 1;
					nl1 = ns;
					break;
				case '=':
					break;
				default:
					if( isdigit(*q) ) {
						if( state ) {
							q+=abc_getnumber(q,&nd1)-1;
							state = 0;
							nl = nl * nd1 + nl1 * nd;
							nd = nd * nd1;
						}
						else
							q+=abc_getnumber(q,&ns)-1;
					}
					break;
			}
		}
	}
	if( !notes ) {
		nl = 1;
		nd = 4;
	}
	if( !nd ) tempo = 120;
	else tempo = ns * nl * 4 / nd; // mod tempo is really BPM where one B is equal to a quartnote
	if( tempo <= 0 )
		tempo = 120;
	if( invoice ) {
		nl = global_tempo_factor;
		nd = global_tempo_divider;
	}
	global_tempo_factor = 1;
	global_tempo_divider = 1;
	while( tempo/global_tempo_divider > 255 )
		global_tempo_divider++;
	tempo /= global_tempo_divider;
	while( tempo * global_tempo_factor < 256 )
		global_tempo_factor++;
	global_tempo_factor--;
	tempo *= global_tempo_factor;
	if( tempo * 3 < 512 ) {
		global_tempo_factor *= 3;
		global_tempo_divider *= 2;
		tempo = (tempo * 3) / 2;
	}
	if( invoice ) {
		if( nl != global_tempo_factor || nd != global_tempo_divider ) {
			ns = (tempo * nl * global_tempo_divider) / (nd * global_tempo_factor);
			if( ns > 31 && ns < 256 ) {
				tempo = ns;
				global_tempo_factor = nl;
				global_tempo_divider = nd;
			}
			else
				abc_message("Failure: inconvenient tempo change in middle of voice (%s)", p);
		}
	}
	return tempo;
}

static void	abc_set_parts(char **d, char *p)
{
	int i,k,m,n;
	size_t j, size;
	char *q;
	if( *d )	free(*d);
	*d = 0;
	if( !p ) return;
	for( i=0; p[i] && p[i] != '%'; i++ ) {
		if( !strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZ().0123456789 ",p[i]) ) {
			abc_message("invalid characters in part string scanning P:%s", p);
			return;
		}
	}
	// decode constructs like "((AB)2.(CD)2)3.(AB)E2" to "ABABCDCDABABCDCDABABCDCDABEE"
	// first compute needed storage...
	j=0;
	k=0;
	for( i=0; p[i] && p[i] != '%'; i++ ) {
		if( isupper(p[i]) ) {
			j++;
		}
		if( isdigit(p[i]) ) {
			n=abc_getnumber(p+i, &k);
			if( k == 0 )
				k = 1;
			if (k > 1e5 ) k = 1e5;
			if( p[i-1] == ')' )
				j *= k;	// never mind multiple parens, just take the worst case
			else
				j += k-1;
			i += n-1;
		}
	}
	// more than 10 million part segments is excessive.
	size = ( j >= 1e7 )? 1e7 - 1 : j;
	q = (char *)_mm_calloc(h, size + 1, sizeof(char)); // enough storage for the worst case
	// now copy bytes from p to *d, taking parens and digits in account
	j = 0;
	for( i=0; p[i] && p[i] != '%' && j < size && i < (int)size; i++ ) {
		if( isdigit(p[i]) || isupper(p[i]) || p[i] == '(' || p[i] == ')' ) {
			if( p[i] == ')' ) {
				for( n=j; n > 0 && q[n-1] != '('; n-- )	;	// find open paren in q
				// q[n+1] to q[j] contains the substring that must be repeated
				if( n > 0 ) {
					for( k = n; k<(int)j; k++ ) q[k-1] = q[k];	// shift to the left...
					j--;
				}
				else {
					abc_message("Warning: Unbalanced right parens in P: definition %s",p);
					break;
				}
				n = j - n + 1;	// number of repeatable characters
				i += abc_getnumber(p+i+1, &k);
				if ( k > 1e5 ) k = 1e5;
				while( k-- > 1 && j+n < size ) {
					for( m=0; m<n; m++ ) {
						q[j] = q[j-n];
						j++;
					}
				}
				continue;
			}
			if( isdigit(p[i]) ) {
				n = abc_getnumber(p+i,&k);
				i += n - 1;
				// if nothing is ready to 'repeat', skip it.
				if (j==0) continue;
				while( k-- > 1 && j < size ) {
					q[j] = q[j-1];
					j++;
				}
				continue;
			}
			q[j] = p[i];
			j++;
		}
	}
	q[j] = '\0';
	// remove any left over parens
	for( i=0; i<(int)j; i++ ) {
		if( q[i] == '(' ) {
			abc_message("Warning: Unbalanced left parens in P: definition %s",p);
			for( k=i; k<(int)j; k++ ) q[k] = q[k+1];
			j--;
		}
	}
	*d = q;
}

static void abc_appendpart(ABCHANDLE *h, ABCTRACK *tp, uint32_t pt1, uint32_t pt2)
{
	ABCEVENT *e, *ec;
	uint32_t dt;
	dt = tp->tail->tracktick - pt1;
	for( e=tp->head; e && e->tracktick <= pt2; e=e->next ) {
		if( e->tracktick >= pt1 ) {
			if( e->flg != 1 || e->cmd == cmdsync || e->cmd == cmdchord ) {
				if( e != tp->tail ) {
					// copy this event at tail
					ec = abc_copy_event(h,e);
					ec->tracktick += dt;
					ec->part = '*';
					tp->tail->next = ec;
					tp->tail = ec;
				}
			}
		}
	}
	abc_add_sync(h, tp, pt2 + dt); // make sure there is progression...
}

static uint32_t abc_pattracktime(ABCHANDLE *h, uint32_t tracktime)
{
	ABCEVENT *e;
	uint32_t dt,et;
	if(!h || !h->track || !h->track->head ) return 0;
	uint32_t pt=abcticks(64 * h->speed);
	dt = 0;
	for( e=h->track->head; e && e->tracktick <= tracktime; e=e->next ) {
		if( e->flg == 1 && e->cmd == cmdpartbrk ) {
			et = e->tracktick + dt;
			if( et % pt ) {
				et += pt;
				et /= pt;
				et *= pt;
				dt = et - e->tracktick;
			}
		}
	}
	return (tracktime + dt);
}

static int abc_patno(ABCHANDLE *h, uint32_t tracktime)
{
	return modticks(abc_pattracktime(h, tracktime)) / 64 / h->speed;
}

static void abc_stripoff(ABCHANDLE *h, ABCTRACK *tp, uint32_t tt)
{
	ABCEVENT *e1, *e2;
	e2 = NULL;
	for( e1 = tp->head; e1 && e1->tracktick <= tt; e1=e1->next )
		e2 = e1;
	if( e2 ) {
		e1 = e2->next;
		tp->tail = e2;
		e2->next = NULL;
	}
	else {
		e1 = tp->tail;
		tp->head = NULL;
		tp->tail = NULL;
	}
	while( e1 ) {
		e2 = e1->next;
		_mm_free(h->trackhandle,e1);
		e1 = e2;
	}
}

static void abc_keeptiednotes(ABCHANDLE *h, uint32_t fromtime, uint32_t totime) {
	ABCTRACK *tp;
	ABCEVENT *e,*n,*f;
	if( totime <= fromtime ) return;
	for( tp=h->track; tp; tp=tp->next ) {
		if( tp->vno ) { // if track is in use...
			n = NULL;
			for( e=tp->head; e && e->tracktick < fromtime; e = e->next )
				if( e->flg != 1 ) n = e; // remember it when it is a note event
			if( n && n->tiednote ) { // we've a candidate to tie over the break
				while( e && e->tracktick < totime ) e=e->next; // skip to other part
				if( e && e->tracktick == totime ) { // if this is on begin row of this part
					f = NULL;
					while( !f && e && e->tracktick == totime ) {
						if( e->flg != 1 ) f = e;
						e = e->next;
					}
					if( f && f->par[note] ) { // pfoeie, we've found a candidate
						if( abs(n->par[note] - f->par[note]) < 3 ) { // undo the note on
							f->flg = 1;
							f->cmd = cmdhide;
							f->lpar = 0;
						}
					}
				}
			}
		}
	}
}

static uint32_t abc_fade_tracks(ABCHANDLE *h, char *abcparts, uint32_t ptt[27])
{
	ABCTRACK *tp;
	ABCEVENT *e0;
	char *p;
	int vol;
	uint32_t pt1, pt2;
	uint32_t tt;
	tt = h->track->tail->tracktick;
	for( tp=h->track->next; tp; tp=tp->next ) {
		if( !tp->tail ) abc_add_sync(h, tp, tt); // no empty tracks please...
		if( tp->tail->tracktick > tt ) abc_stripoff(h, tp, tt); // should not happen....
		if( tp->tail->tracktick < tt ) abc_add_sync(h, tp, tt);
	}
	for( tp=h->track; tp; tp=tp->next ) {
		vol = 127;
		e0 = tp->tail;
		if( tp->slidevol != -2 ) {
			tp->slidevol = -2;
			tp->slidevoltime = e0->tracktick;
		}
		tp->mute = 0; // unmute track for safety, notes in a muted track already have zero volume...
		while( vol > 5 && tp->slidevoltime != 0) {
			for( p=abcparts; *p && vol > 5; p++ ) {
				pt1 = ptt[*p-'A'];
				pt2 = ptt[*p-'A'+1];
				abc_appendpart(h, tp, pt1, pt2);
				vol = abc_dynamic_volume(tp, tp->tail->tracktick, 127);
			}
		}
		abc_fade_track(tp,e0);
	}
	return h->track->tail->tracktick;
}

static void abc_song_to_parts(ABCHANDLE *h, char **abcparts, BYTE partp[27][2])
{
	uint32_t starttick;
	ABCEVENT *e;
	int i, fading, loop, normal, partno, partsegno, partloop, partcoda, parttocoda, partfine, skip, x, y;
	int vmask[27],nextp[27];
	uint32_t ptt[27];
	char buf[256];	// must be enough, mod's cannot handle more than 240 patterns
	char *pfade;
	if( !h || !h->track || !h->track->capostart ) return;
	strcpy(buf,"A");	// initialize our temporary array
	i = 1;
	loop = 1;
	partno = 0;
	partsegno = 0;
	partloop = 0;
	partcoda = -1;
	parttocoda = -1;
	partfine = -1;
	starttick = h->track->capostart->tracktick;
	memset(ptt, 0, sizeof(ptt));
	ptt[0] = starttick;
	vmask[0] = -1;
	nextp[0] = 1;
	for( e=h->track->capostart; e; e=e->next ) {
		if( e->flg == 1 ) {
			switch( e->cmd ) {
				case cmdpartbrk:
					if( e->tracktick > starttick) {
						starttick = e->tracktick;	// do not make empty parts
						if( partno < 26 ) {
							partno++;
							ptt[partno] = starttick;
						}
						if( i < 255 )	buf[i++] = partno+'A';
						vmask[partno] = -1;
						nextp[partno] = partno+1;
					}
					break;
				case cmdloop:
					partloop = partno;
					loop = 1; // start counting anew...
					break;
				case cmdvariant:
					vmask[partno] = e->lpar;
					break;
				case cmdjump:
					x = 0;
					fading = 0;
					normal = 0;
					skip   = 0;
					pfade  = &buf[i];
					switch( e->par[jumptype] ) {
						case jumpfade:
							fading = 1;
						case jumpnormal:
							normal = 1;
							x = partloop;
							loop++;
							break;
						case jumpdsfade:
							fading = 1;
						case jumpdasegno:
							x = partsegno;
							break;
						case jumpdcfade:
							fading = 1;
						case jumpdacapo:
							x = 0;
							break;
						default:
							// defaults set above.
							break;
					}
					if( vmask[partno] != -1 ) nextp[partno] = x;
					if( partno < 26 ) ptt[partno+1] = e->tracktick; // for handling ties over breaks
					while( x <= partno ) {
						if( skip == 1 && x == partcoda ) skip = 0;
						y = !skip;
						if( y ) {
							if( !normal ) {
								if( x == partfine ) skip = 2;
								if( x == parttocoda ) skip = 1;
								y = !skip;
							}
							if( !(vmask[x] & (1<<loop)) ) y = 0;
						}
						if( y ) {
							if( i < 255 )	buf[i++] = x+'A';
							if( nextp[x] != x + 1 ) loop++;
							x = nextp[x];
						}
						else
							x++;
					}
					if( fading && partno < 25 && i < 254 ) {	// add single part with fading tracks
						partno++;
						ptt[partno] = e->tracktick;
						buf[i] = '\0'; // close up pfade with zero byte
						starttick = abc_fade_tracks(h, pfade, ptt);
						buf[i++] = partno+'A';
						partno++;
						ptt[partno] = starttick;
						buf[i++] = partno+'A'; // one extra to throw away...
						e = h->track->tail; // this is the edge of the world captain...
					}
					break;
				case cmdtocoda:
					parttocoda = partno;
					break;
				case cmdcoda:
					partcoda = partno;
					break;
				case cmdfine:
					partfine = partno;
					break;
				case cmdsegno:
					partsegno = partno;
					break;
			}
		}
		e->part = partno+'a';	// small caps for generated parts...
	}
	i--; // strip off last partno
	if( partno > 0 ) partno--;
	buf[i] = '\0';
	if( i > 1 ) {
		for( i=1; buf[i]; i++ ) {
			if( buf[i] != buf[i-1] + 1 ) {
				x = buf[i-1] - 'A';
				y = buf[i] - 'A';
				if (x < 26 && y < 26)
					abc_keeptiednotes(h, ptt[x+1], ptt[y]);
			}
		}
	}
	starttick = h->track->tail->tracktick;
	ptt[partno+1] = starttick;
	for( i=0; i<=partno; i++ ) {
		partp[i][0] = abc_patno(h, ptt[i]);
		partp[i][1] = abc_patno(h, ptt[i+1]);
	}
	// calculate end point of last part
	starttick = abc_pattracktime(h, starttick);
	if( starttick % abcticks(64 * h->speed) )
		partp[partno][1]++;
	abc_set_parts(abcparts, buf);
}

// =====================================================================================
static char *abc_fgets(MMFILE *mmfile, char buf[], unsigned int bufsz)
// =====================================================================================
{
	if( mmfeof(mmfile) ) return NULL;
	mmfgets(buf,bufsz,mmfile);
	return buf;
}

// =====================================================================================
static char *abc_fgetbytes(MMFILE *mmfile, char buf[], unsigned int bufsz)
// =====================================================================================
{
	unsigned int i;
	long pos;
	if( mmfeof(mmfile) ) return NULL;
	for( i=0; i<bufsz-2; i++ ) {
		buf[i] = (char)mmfgetc(mmfile);
		if( buf[i] == '\n' ) break;
		if( buf[i] == '\r' ) {
			pos = mmftell(mmfile);
			// peek at next char
			if( mmfgetc(mmfile) != '\n' )	mmfseek(mmfile, pos, SEEK_SET);
			buf[i] = '\n';
			break;
		}
	}
	if( i != bufsz-2 && buf[i] == '\n' ) i++;
	buf[i] = '\0';
	return buf;
}

static void abc_substitute(ABCHANDLE *h, char *target, char *s)
{
	char *p, *q;
	int i;
	int l = strlen(target);
	int n = strlen(s);
	if (l <= 0 ||n <= 0 || strstr(s, target) || abs(n-l) > 10e3)
		return;
	while ((p=strstr(h->line, target)) != NULL) {
		if( (i=strlen(h->line)) + n - l >= (int)h->len ) {
			int reqsize = h->len<<1;
			while (i + n - l >= reqsize) reqsize = reqsize<<1;
			h->line = (char *)_mm_recalloc(h->allochandle, h->line, reqsize, sizeof(char));
			h->len = reqsize;
			p=strstr(h->line, target);
		}
		if( n > l ) {
			for( q=&h->line[i]; q>p; q-- ) q[n-l] = q[0];
			for( q=s; *q; q++ ) *p++ = *q;
		}
		else {
			strcpy(p,s);
			for( q=p+l; *q; q++ ) *p++ = *q;
		}
		// ensure end of string is initialized
		*p = 0;
	}
}

static void abc_preprocess(ABCHANDLE *h, ABCMACRO *m)
{
	int i, j, k, l, a, b;
	if( m->n ) {
		k = m->n - m->name;
		for( i=0; i<14; i++ ) {
			char *t = new char[strlen(m->name) + 1];
			strcpy(t, m->name);
			t[k] = "CDEFGABcdefgab"[i];
			l = strlen(m->subst);
			char *s = new char[2 * l + 1];
			char *p = s;
			for( j=0; j<l; j++ ) {
				a = m->subst[j];
				if( a > 'g' && islower(a) ) {
					b = a - 'n';
					a = "CDEFGABCDEFGABcdefgabcdefgabcdefgab"[i+b+7];
					*p++ = a;
					if( i+b < 0 )
						*p++ = ',';
					else if( i+b > 13 )
						*p++ = '\'';
				}
				else *p++ = a;
			}
			*p = '\0';
			abc_substitute(h, t, s);
			delete[] s;
			delete[] t;
		}
	}
	else
		abc_substitute(h, m->name, m->subst);
}

static char *abc_gets(ABCHANDLE *h, MMFILE *mmfile)
{
	int i;
	ABCMACRO *mp;
	if( !h->len ) {
		h->len = 64;	// initial line size, adequate for most abc's
		h->line = (char *)_mm_calloc(h->allochandle, h->len, sizeof(char));
	}
	if( abc_fgetbytes(mmfile, h->line, h->len) ) {
		while( (i=strlen(h->line)) > (int)(h->len - 3) ) {
			// line too short, double it
			h->line = (char *)_mm_recalloc(h->allochandle, h->line, h->len<<1, sizeof(char));
			if( h->line[i-1] != '\n' )
				abc_fgetbytes(mmfile, &h->line[i], h->len);
			h->len <<= 1;
		}
		h->line[i ? i-1 : 0] = '\0'; // strip off newline / force null termination
		for( mp=h->macro; mp && mp->name; mp=mp->next )
			abc_preprocess(h,mp);
		return h->line;
	}
	return NULL;
}

static int abc_parse_decorations(ABCHANDLE *h, ABCTRACK *tp, const char *p)
{
	int vol=0;
	if( !strncmp(p,"mp",2) ) vol = 75;
	if( !strncmp(p,"mf",2) ) vol = 90;
	if( !strncmp(p,"sfz",3) ) vol = 100;
	if( *p == 'p' ) {
		vol = 60;
		while( *p && *p++ == 'p' ) vol -= 15;
		if( vol < 1 ) vol = 1;
	}
	if( *p == 'f' ) {
		vol = 105;
		while( *p && *p++ == 'f' ) vol += 15;
		if( vol > 135 ) vol = 127;	// ffff
		if( vol > 127 ) vol = 125;	// fff
	}
	if( vol ) {
		tp->volume = vol;
		if( tp == h->track ) {	// copy volume over to all voice tracks
			for( ; tp; tp=tp->next ) {
				if( tp->vpos == 0 || tp->vpos > DRONEPOS2 ) tp->volume = vol;
			}
			tp = h->track;
		}
	}
	return tp->volume;
}

// =====================================================================================
BOOL CSoundFile::TestABC(const BYTE *lpStream, DWORD dwMemLength)
// =====================================================================================
{
	char id[128];
	int hasText = 0;
	// scan file for first K: line (last in header)
	MMFILE mmfile;
	mmfile.mm = (char *)lpStream;
	mmfile.sz = dwMemLength;
	mmfseek(&mmfile,0,SEEK_SET);
	int ppos = mmfile.pos;

	while(abc_fgets(&mmfile,id,128)) {
		if (id[0] == 0 && hasText == 0 && mmfile.pos < ppos + 120) return(0); //probably binary
		if (id[0] == 0) continue; // blank line.

		if (!abc_isvalidchar(id[0])  || !abc_isvalidchar(id[1])) {
			return(0); // probably not an ABC.
		}
		if(id[0]=='K'
			&& id[1]==':'
			&& (isalpha(id[2]) || isspace(id[2])) ) return 1;
		// disable binary error if have any "tag"
		if((id[0]>='A' && id[0]<='Z')
			&& id[1]==':'
			&& (isalpha(id[2]) || isspace(id[2])) ) hasText = 1;
	}
	return 0;
}

// =====================================================================================
static ABCHANDLE *ABC_Init(void)
{
	static char buf[40];
	ABCHANDLE   *retval;
	char *p;
	retval = (ABCHANDLE *)calloc(1,sizeof(ABCHANDLE));
	if( !retval ) return NULL;
	retval->track       = NULL;
	retval->macro       = NULL;
	retval->umacro      = NULL;
	retval->beatstring  = NULL;
	retval->pickrandom  = 0;
	retval->len         = 0;
	retval->line        = NULL;
	strcpy(retval->gchord, "");
	retval->barticks    = 0;
	p = getenv(ABC_ENV_NORANDOMPICK);
	if( p ) {
		if( isdigit(*p) )
			retval->pickrandom = atoi(p);
		if( *p == '-' ) {
			retval->pickrandom = atoi(p+1)-1; // xmms preloads the file
			sprintf(buf,"%s=-%ld",ABC_ENV_NORANDOMPICK,retval->pickrandom+2);
			putenv(buf);
		}
	}
	else {
		srand((unsigned int)time(0));	// initialize random generator with seed
		retval->pickrandom = 1+(int)(10000.0*rand()/(RAND_MAX+1.0));
		// can handle pickin' from songbooks with 10.000 songs
		sprintf(buf,"%s=-%ld",ABC_ENV_NORANDOMPICK,retval->pickrandom); // xmms preloads the file
		putenv(buf);
	}
	return retval;
}

static void ABC_CleanupTrackEvents(ABCTRACK *tp)
{
	ABCEVENT *ep, *en;
	if( tp ) {
		for( ep=tp->head; ep; ep = en ) {
			en=ep->next;
			free(ep);
		}
		tp->head = NULL;
	}
}

static void ABC_CleanupMacro(ABCMACRO *m)
{
	if( m->name )
		free(m->name);
	if( m->subst )
		free(m->subst);
	free(m);
}

// =====================================================================================
static void ABC_CleanupTracks(ABCHANDLE *handle)
// =====================================================================================
{
	ABCTRACK *tp, *tn;
	if(handle) {
		for( tp=handle->track; tp; tp = tn ) {
			tn=tp->next;
			ABC_CleanupTrackEvents(tp);
			free(tp);
		}
		handle->track = NULL;
	}
}

// =====================================================================================
static void ABC_CleanupMacros(ABCHANDLE *handle)
// =====================================================================================
{
	ABCMACRO *mp, *mn;
	if(handle) {
		for( mp=handle->macro; mp; mp = mn ) {
			mn=mp->next;
			ABC_CleanupMacro(mp);
		}
		for( mp=handle->umacro; mp; mp = mn ) {
			mn=mp->next;
			ABC_CleanupMacro(mp);
		}
		handle->macro = NULL;
		handle->umacro = NULL;
	}
}

// =====================================================================================
static void ABC_Cleanup(ABCHANDLE *handle)
// =====================================================================================
{
	if(handle) {
		ABC_CleanupMacros(handle);
		ABC_CleanupTracks(handle);
		if( handle->line )
			free(handle->line);
		if( handle->beatstring )
			free(handle->beatstring);
		free(handle);
	}
}

static int abc_is_global_event(ABCEVENT *e)
{
	return e->flg == 1 && (e->cmd == cmdtempo || e->cmd == cmdpartbrk);
}

static ABCEVENT *abc_next_global(ABCEVENT *e)
{
	for( ; e && !abc_is_global_event(e); e=e->next ) ;
	return e;
}

static ABCEVENT *abc_next_note(ABCEVENT *e)
{
	for( ; e && e->flg == 1; e=e->next ) ;
	return e;
}

// =============================================================================
static int ABC_ReadPatterns(MODCOMMAND *pattern[], WORD psize[], ABCHANDLE *h, int numpat, int channels)
// =====================================================================================
{
	int pat,row,i,ch,trillbits;
	BYTE n,ins,vol;
	ABCTRACK *t;
	ABCEVENT *e, *en, *ef, *el;
	uint32_t tt1, tt2;
	MODCOMMAND *m;
	int patbrk, tempo;
	if( numpat > MAX_PATTERNS ) numpat = MAX_PATTERNS;

	// initialize start points of event list in tracks
	for( t = h->track; t; t = t->next ) t->capostart = t->head;
	trillbits = 0; // trill effect admininstration: one bit per channel, max 32 channnels
	for( pat = 0; pat < numpat; pat++ ) {
		pattern[pat] = CSoundFile::AllocatePattern(64, channels);
		if( !pattern[pat] ) return 0;
		psize[pat] = 64;
		for( row = 0; row < 64; row++ ) {
			tt1 = abcticks((pat * 64 + row ) * h->speed);
			tt2 = tt1 + abcticks(h->speed);
			ch = 0;
			tempo = 0;
			patbrk = 0;
			if ( h->track )
			for( e=abc_next_global(h->track->capostart); e && e->tracktick < tt2; e=abc_next_global(e->next) ) {
				if( e && e->tracktick >= tt1 ) {	// we have a tempo event in this row
					switch( e->cmd ) {
						case cmdtempo:
							tempo = e->lpar;
							break;
						case cmdpartbrk:
							patbrk = 1;
							break;
					}
				}
			}
			for( t = h->track; t; t = t->next ) {
				for( e=abc_next_note(t->capostart); e && e->tracktick < tt1; e=abc_next_note(e->next) ) ;
				i = 0;
				ef = NULL;
				en = e;
				el = e;
				for( ; e && e->tracktick < tt2; e=abc_next_note(e->next) ) {	// we have a note event in this row
					t->capostart = e;
					i++;
					if( e->par[volume] ) {
						if( !ef ) ef = e;
						el = e;
					}
				}
				m = &pattern[pat][row * channels + ch];
				m->param   = 0;
				m->command = CMD_NONE;
				if( i ) {
					trillbits &= ~(1<<ch);
					if( i == 1 || ef == el || !ef ) { // only one event in this row
						if( ef ) e = ef;
						else e = en;
						el = t->capostart;
						i  = e->par[note] + ((e->par[octave])*12);
						if( t->chan == 10 ) {
							n   = pat_gm_drumnote(i) + 23;
							ins = pat_gmtosmp(pat_gm_drumnr(i));
						}
						else {
							n   = pat_modnote(i);
							ins = e->par[smpno];
						}
							vol = e->par[volume]/2;
						if( e->par[volume] > 0 ) {
							if( e->par[effect] == accent ) vol += vol / 20;
							if( vol > 64 ) vol = 64;
							if( el->par[volume] == 0 ) { // note cut
								m->param   = el->tracktick - tt1;
								m->command = CMD_S3MCMDEX;
								m->param  |= 0xC0;
							}
							else {
								switch( e->par[effect] ) {
									case trill:
										m->command = CMD_VIBRATO;
										m->param   = 0xC2;	// speed 12 depth 2
										trillbits |= (1<<ch);
										break;
									case bow:
										m->command = CMD_XFINEPORTAUPDOWN;
										m->param  |= (e->par[effoper])? 0x12: 0x22;
										break;
									default:
										m->param = modticks(e->tracktick - tt1);
										if( m->param ) { // note delay
											m->command = CMD_S3MCMDEX;
											m->param  |= 0xD0;
										}
										break;
								}
							}
						}
						m->instr  = ins;
						m->note   = n; // <- normal note
						m->volcmd = VOLCMD_VOLUME;
						m->vol    = vol;
					}
					else {
						// two notes in one row, use FINEPITCHSLIDE runonce effect
						// start first note on first tick and framedly runonce on seconds note tick
						// use volume and instrument of last note
						if( t->chan == 10 ) {
							i   = el->par[note] + ((el->par[octave])*12);
							n   = pat_gm_drumnote(i) + 23;
							ins = pat_gmtosmp(pat_gm_drumnr(i));
							i   = n; // cannot change instrument here..
						}
						else {
							i   = ef->par[note] + ((ef->par[octave])*12);
							n   = pat_modnote(i);
							ins = el->par[smpno];
							i   = pat_modnote(el->par[note] + ((el->par[octave])*12));
						}
						vol = el->par[volume]/2;
						if( vol > 64 ) vol = 64;
						m->instr  = ins;
						m->note   = n; // <- normal note
						m->volcmd = VOLCMD_VOLUME;
						m->vol    = vol;
						m->param  = ((i > n)?i-n:n-i);
						if( m->param < 16 ) {
							if( m->param ) {
								m->command = CMD_XFINEPORTAUPDOWN;
								m->param  |= (i > n)? 0x10: 0x20;
							}
							else {	// retrigger same note...
								m->command = CMD_RETRIG;
								m->param   = modticks(el->tracktick - tt1);
							}
						}
						else
							m->command = (i > n)? CMD_PORTAMENTOUP: CMD_PORTAMENTODOWN;
					}
				}
				else { // no new notes, keep on trilling...
					if( trillbits & (1<<ch) ) {
						m = &pattern[pat][row * channels + ch];
						m->command = CMD_VIBRATO;
						m->param   = 0;	// inherited from first effect
						m->instr   = 0;
						m->note    = 0;
						m->volcmd  = 0;
						m->vol     = 0;
					}
				}
				if( m->param == 0 && m->command == CMD_NONE ) {
					if( tempo ) {
						m->command = CMD_TEMPO;
						m->param   = tempo;
						tempo      = 0;
					}
					else {
						if( patbrk ) {
							m->command = CMD_PATTERNBREAK;
							patbrk     = 0;
						}
					}
				}
				ch++;
			}
			if( tempo || patbrk ) return 1;
		}
	}
	return 0;
}

static int ABC_Key(const char *p)
{
	int i,j;
	char c[8];
	const char *q;
	while( isspace(*p) ) p++;
	q = p;
	memset(c, 0, 8);
	for( i=0; i<8 && *p && *p != ']'; p++ ) {
		if( isspace(*p) ) {
			while( isspace(*p) ) p++;
			if( strncasecmp(p, "min", 3) && strncasecmp(p, "maj", 3) )
				break;
		}
		c[i] = *p;
		i++;
	}
	if( !strcmp(c,"Hp") || !strcmp(c,"HP") ) // highland pipes
		strcpy(c,"Bm");	// two sharps at c and f
	if( !strcasecmp(c+1, "minor") ) i=2;
	if( !strcasecmp(c+2, "minor") ) i=3;
	if( !strcasecmp(c+1, "major") ) i=1;
	if( !strcasecmp(c+2, "major") ) i=2;
	if( !strcasecmp(c+1, "min") ) i=2;
	if( !strcasecmp(c+2, "min") ) i=3;
	if( !strcasecmp(c+1, "maj") ) i=1;
	if( !strcasecmp(c+2, "maj") ) i=2;
	for( ; i<6; i++ )
		c[i] = ' ';
	for( i=0; keySigs[i]; i++ ) {
		for( j=10; j<46; j+=6 )
			if( !strncasecmp(keySigs[i]+j, c, 6) )
				return i;
	}
	abc_message("Failure: Unrecognised K: field %s", q);
	return 7;
}

static char *abc_skip_word(char *p)
{
	while( isspace(*p) ) p++;
	while( *p && !isspace(*p) && *p != ']') p++;
	while( isspace(*p) ) p++;
	return p;
}

static uint32_t abc_tracktime(ABCTRACK *tp)
{
	uint32_t tracktime;
	if( tp->tail ) tracktime = tp->tail->tracktick;
	else tracktime = 0;
	if( tracktime < global_songstart )
		tracktime = global_songstart;
	return tracktime;
}

static void abc_addchordname(const char *s, int len, const int *notes)
// adds chord name and note set to list of known chords
{
	int i, j;
	if(strlen(s) > 7) {
		abc_message("Failure: Chord name cannot exceed 7 characters, %s", s);
		return;
	}
	if(len > 6) {
		abc_message("Failure: Named chord cannot have more than 6 notes, %s", s);
		return;
	}
	for( i=0; i < chordsnamed; i++ ) {
		if(strcmp(s, chordname[i]) == 0) {
			/* change chord */
			chordlen[i] = len;
			for(j = 0; j < len; j++) chordnotes[i][j] = notes[j];
			return;
		}
	}
	if(chordsnamed > MAXCHORDNAMES - 1)
		abc_message("Failure: Too many Guitar Chord Names used, %s", s);
	else {
		strcpy(chordname[chordsnamed], s);
		chordlen[chordsnamed] = len;
		for(j = 0; j < len; j++) chordnotes[chordsnamed][j] = notes[j];
		chordsnamed++;
	}
}

static void abc_setup_chordnames()
// set up named guitar chords
{
	static const int list_Maj[3] = { 0, 4, 7 };
	static const int list_m[3] = { 0, 3, 7 };
	static const int list_7[4] = { 0, 4, 7, 10 };
	static const int list_m7[4] = { 0, 3, 7, 10 };
	static const int list_maj7[4] = { 0, 4, 7, 11 };
	static const int list_M7[4] = { 0, 4, 7, 11 };
	static const int list_6[4] = { 0, 4, 7, 9 };
	static const int list_m6[4] = { 0, 3, 7, 9 };
	static const int list_aug[3] = { 0, 4, 8 };
	static const int list_plus[3] = { 0, 4, 8 };
	static const int list_aug7[4] = { 0, 4, 8, 10 };
	static const int list_dim[3] = { 0, 3, 6 };
	static const int list_dim7[4] = { 0, 3, 6, 9 };
	static const int list_9[5] = { 0, 4, 7, 10, 2 };
	static const int list_m9[5] = { 0, 3, 7, 10, 2 };
	static const int list_maj9[5] = { 0, 4, 7, 11, 2 };
	static const int list_M9[5] = { 0, 4, 7, 11, 2 };
	static const int list_11[6] = { 0, 4, 7, 10, 2, 5 };
	static const int list_dim9[5] = { 0, 4, 7, 10, 13 };
	static const int list_sus[3] = { 0, 5, 7 };
	static const int list_sus9[3] = { 0, 2, 7 };
	static const int list_7sus[4] = { 0, 5, 7, 10 };
	static const int list_7sus4[4] = { 0, 5, 7, 10 };
	static const int list_7sus9[4] = { 0, 2, 7, 10 };
	static const int list_9sus4[5] = { 0, 5, 10, 14, 19 };
	static const int list_5[2] = { 0, 7 };
	static const int list_13[6] = { 0, 4, 7, 10, 16, 21 };

	chordsnamed = 0;
	abc_addchordname("", 3, list_Maj);
	abc_addchordname("m", 3, list_m);
	abc_addchordname("7", 4, list_7);
	abc_addchordname("m7", 4, list_m7);
	abc_addchordname("maj7", 4, list_maj7);
	abc_addchordname("M7", 4, list_M7);
	abc_addchordname("6", 4, list_6);
	abc_addchordname("m6", 4, list_m6);
	abc_addchordname("aug", 3, list_aug);
	abc_addchordname("+", 3, list_plus);
	abc_addchordname("aug7", 4, list_aug7);
	abc_addchordname("7+", 4, list_aug7);
	abc_addchordname("dim", 3, list_dim);
	abc_addchordname("dim7", 4, list_dim7);
	abc_addchordname("9", 5, list_9);
	abc_addchordname("m9", 5, list_m9);
	abc_addchordname("maj9", 5, list_maj9);
	abc_addchordname("M9", 5, list_M9);
	abc_addchordname("11", 6, list_11);
	abc_addchordname("dim9", 5, list_dim9);
	abc_addchordname("sus", 3, list_sus);
	abc_addchordname("sus9", 3, list_sus9);
	abc_addchordname("7sus", 4, list_7sus);
	abc_addchordname("7sus4", 4, list_7sus4);
	abc_addchordname("7sus9", 4, list_7sus9);
	abc_addchordname("9sus4", 5, list_9sus4);
	abc_addchordname("5", 2, list_5);
	abc_addchordname("13", 6, list_13);
}

static int abc_MIDI_getnumber(const char *p)
{
	int n;
	while( isspace(*p) ) p++;
	abc_getnumber(p, &n);
	if( n < 0 )   n = 0;
	if( n > 127 ) n = 127;
	return n;
}

static int abc_MIDI_getprog(const char *p)
{
	int n;
	while( isspace(*p) ) p++;
	abc_getnumber(p, &n);
	if( n < 1 )   n = 1;
	if( n > 128 ) n = 128;
	return n;
}

// MIDI drone <instr0> <pitch1> <pitch2> <vel1> <vel2>
static void abc_MIDI_drone(const char *p, int *gm, int *ptch, int *vol)
{
	int i;
	while( isspace(*p) ) p++;
	p += abc_getnumber(p, &i);
	i++;	// adjust for 1..128
	if( i>0 && i < 129 )
		*gm = i;
	else
		*gm = 71;	// bassoon
	while( isspace(*p) ) p++;
	p += abc_getnumber(p, &i);
	if( i>0 && i < 127 )
		ptch[0] = i;
	else
		ptch[0] = 45;
	while( isspace(*p) ) p++;
	p += abc_getnumber(p, &i);
	if( i>0 && i < 127 )
		ptch[1] = i;
	else
		ptch[1] = 33;
	while( isspace(*p) ) p++;
	p += abc_getnumber(p, &i);
	if( i>0 && i < 127 )
		vol[0] = i;
	else
		vol[0] = 80;
	while( isspace(*p) ) p++;
	p += abc_getnumber(p, &i);
	if( i>0 && i < 127 )
		vol[1] = i;
	else
		vol[1] = 80;
}

static void abc_chan_to_tracks(ABCHANDLE *h, int tno, int ch)
{
	ABCTRACK *tp;
	if( tno>0 && tno<33 ) {
		for( tp=h->track; tp; tp=tp->next ) {
			if( tp->vno == tno && (tp->vpos < GCHORDBPOS || tp->vpos > DRONEPOS2) )
				tp->chan = ch;
		}
	}
}

// %%MIDI channel int1
// channel numbers are 1-16
static void abc_MIDI_channel(const char *p, ABCTRACK *tp, ABCHANDLE *h)
{
	int i1, i2;
	i1 = tp? tp->vno: 1;
	for( ; *p && isspace(*p); p++ ) ;
	if( isdigit(*p) ) {
		p += abc_getnumber(p, &i2);
		if( i2 >= 1 && i2 <= 16 )
			abc_chan_to_tracks(h, i1, i2); // we start at 1
	}
}

static void abc_instr_to_tracks(ABCHANDLE *h, int tno, int gm)
{
	ABCTRACK *tp;
	if( tno>0 && tno<33 && gm>0 && gm<129 ) {
		for( tp=h->track; tp; tp=tp->next ) {
			if( tp->vno == tno && (tp->vpos < GCHORDBPOS || tp->vpos > DRONEPOS2) )
				tp->instr = gm;
		}
	}
}

// %%MIDI program [int1] <int2>
// instrument numbers are 0-127
static void abc_MIDI_program(const char *p, ABCTRACK *tp, ABCHANDLE *h)
{
	int i1, i2;
	i1 = tp? tp->vno: 1;
	for( ; *p && isspace(*p); p++ ) ;
	if( isdigit(*p) ) {
		p += abc_getnumber(p, &i2);
		for( ; *p && isspace(*p); p++ ) ;
		if( isdigit(*p) ) {
			i1 = i2;
			abc_getnumber(p, &i2);
		}
		abc_instr_to_tracks(h, i1, i2 + 1); // we start at 1
	}
}

static void abc_mute_voice(ABCHANDLE *h, ABCTRACK *tp, int m)
{
	ABCTRACK *t;
	for( t=h->track; t; t=t->next ) {
		if( t->vno == tp->vno ) t->mute = m;
	}
}

// %%MIDI voice [<ID>] [instrument=<integer> [bank=<integer>]] [mute]
// instrument numbers are 1-128
static void abc_MIDI_voice(const char *p, ABCTRACK *tp, ABCHANDLE *h)
{
	int i1, i2;
	for( ; *p && isspace(*p); p++ ) ;
	if( strncmp(p,"instrument=",11) && strncmp(p,"mute",4) ) {
		tp = abc_locate_track(h, p, 0);
		for( ; *p && !isspace(*p); p++ ) ;
		for( ; *p && isspace(*p); p++ ) ;
	}
	i1 = tp? tp->vno: 1;
	i2 = 0;
	if( !strncmp(p,"instrument=",11) && isdigit(p[11]) ) {
		p += 11;
		p += abc_getnumber(p, &i2);
		for( ; *p && isspace(*p); p++ ) ;
		if( !strncmp(p,"bank=",5) && isdigit(p[5]) ) {
			for( ; *p && !isspace(*p); p++ ) ;
			for( ; *p && isspace(*p); p++ ) ;
		}
	}
	if( tp ) abc_mute_voice(h,tp,0);
	if( !strncmp(p,"mute",4) && (p[4]=='\0' || p[4]=='%' || isspace(p[4])) ) {
		if( tp ) abc_mute_voice(h,tp,1);
	}
	abc_instr_to_tracks(h, i1, i2); // starts already at 1 (draft 4.0)
}

// %%MIDI chordname <string> <int1> <int2> ... <int6>
static void abc_MIDI_chordname(const char *p)
{
	char name[20];
	int i;

	for(; *p && isspace(*p); p++)
		;
	i = 0;
	while ((i < 19) && (*p != ' ') && (*p != '\0')) {
		name[i] = *p;
		p = p + 1;
		i = i + 1;
	}
	name[i] = '\0';
	if(*p != ' ') {
		abc_message("Failure: Bad format for chordname command, %s", p);
	}
	else {
		int notes[6];
		i = 0;
		memset(notes, 0, sizeof(notes));
		while ((i < 6) && isspace(*p)) {
			for(; *p && isspace(*p); p++)
				;
			p += abc_getnumber(p, &notes[i]);
			i = i + 1;
		}
		abc_addchordname(name, i, notes);
	}
}

// %%MIDI drum <string> <inst 1> ... <inst n> <vol 1> ... <vol n>
// instrument numbers are 0-127
static int abc_MIDI_drum(const char *p, ABCHANDLE *h)
{
	char *q;
	int i, n, m, len;
	while( isspace(*p) ) p++;
	if( !strncmp(p,"on",2) && (isspace(p[2]) || p[2] == '\0') ) return 2;
	if( !strncmp(p,"off",3) && (isspace(p[3]) || p[3] == '\0') ) return 1;
	n = 0; len = 0;
	for( q = h->drum; *p && !isspace(*p); p++ ) {
		if( !strchr("dz0123456789",*p) ) break;
		*q++ = *p; len++;
		if( !isdigit(*p) && len < (int)sizeof(h->drum)-1 ) {
			if( !isdigit(p[1]) ) { *q++ = '1'; len ++; }
			n++; // count the silences too....
		}
		if (len >= (int)sizeof(h->drum)-1) {
			// consume the rest of the input
			// definitely enough "drum last state" stored.
			while ( *p && !isspace(*p) ) p++;
			break;
		}
	}
	*q = '\0';
	q = h->drumins;
	for( i = 0; i<n; i++ ) {
		if( h->drum[i*2] == 'd' ) {
			while( *p && isspace(*p) ) p++;
			if( !isdigit(*p) ) {
				m = 0;
				while( *p && !isspace(*p) ) p++;
			}
			else
				p += abc_getnumber(p,&m);
			q[i] = m + 1; // we start at 1
		}
		else q[i] = 0;
	}
	q = h->drumvol;
	for( i = 0; i<n; i++ ) {
		if( h->drum[i*2] == 'd' ) {
			while( *p && isspace(*p) ) p++;
			if( !isdigit(*p) ) {
				m = 0;
				while( *p && !isspace(*p) ) p++;
			}
			else
				p += abc_getnumber(p,&m);
			q[i] = m;
		}
		else q[i] = 0;
	}
	return 0;
}

// %%MIDI gchord <string>
static int abc_MIDI_gchord(const char *p, ABCHANDLE *h)
{
	char *q;
	int len = 0;
	while( isspace(*p) ) p++;
	if( !strncmp(p,"on",2) && (isspace(p[2]) || p[2] == '\0') ) return 2;
	if( !strncmp(p,"off",3) && (isspace(p[3]) || p[3] == '\0') ) return 1;
	for( q = h->gchord; *p && !isspace(*p); p++ ) {
		if( !strchr("fbcz0123456789ghijGHIJ",*p) ) break;
		*q++ = *p; len++;
		if( !isdigit(*p) && len < (int)sizeof(h->gchord)-1 && !isdigit(p[1]) ) { *q++ = '1'; len ++; }
		if (len >= (int)sizeof(h->gchord)-1) {
			// consume the rest of the input
			// definitely enough "drum last state" stored.
			while ( *p && !isspace(*p) ) p++;
			break;
		}
	}
	*q = '\0';
	return 0;
}

static void abc_metric_gchord(ABCHANDLE *h, int mlen, int mdiv)
{
	switch( 16 * mlen + mdiv ) {
		case 0x24:
		case 0x44:
		case 0x22:
			abc_MIDI_gchord("fzczfzcz", h);
			break;
		case 0x64:
		case 0x32:
			abc_MIDI_gchord("fzczczfzczcz", h);
			break;
		case 0x34:
		case 0x38:
			abc_MIDI_gchord("fzczcz", h);
			break;
		case 0x68:
			abc_MIDI_gchord("fzcfzc", h);
			break;
		case 0x98:
			abc_MIDI_gchord("fzcfzcfzc", h);
			break;
		case 0xc8:
			abc_MIDI_gchord("fzcfzcfzcfzc", h);
			break;
		default:
		{
			int dest = 0;
			if( mlen % 3 == 0 )
				abc_MIDI_gchord("fzcfzcfzcfzcfzcfzcfzcfzcfzc", h);
			else
				abc_MIDI_gchord("fzczfzczfzczfzczfzczfzczfzcz", h);

			if( mdiv == 8 )	dest = mlen*2;
			else dest = mlen*4;
			if (dest >= (int)sizeof(h->gchord))
				dest = (int)sizeof(h->gchord) - 1;
			h->gchord[dest] = '\0';
		}
		break;
	}
}

static void abc_MIDI_legato(const char *p, ABCTRACK *tp)
{
	for( ; *p && isspace(*p); p++ ) ;
	if( !strncmp(p,"off",3) ) tp->legato = 0;
	else tp->legato = 1;
}

static void abc_M_field(const char *p, int *mlen, int *mdiv)
{
	if( !strncmp(p,"none",4) ) {
		*mlen = 1;
		*mdiv = 1;
		return;
	}
	if( !strncmp(p,"C|",2) ) {
		*mlen = 2;
		*mdiv = 2;
		return;
	}
	if( *p == 'C' ) {
		*mlen = 4;
		*mdiv = 4;
		return;
	}
	p += abc_getexpr(p,mlen);
	sscanf(p," / %d", mdiv);
}

static int abc_drum_steps(const char *dch)
{
	const char *p;
	int i=0;
	for( p=dch; *p; p++ ) {
		if( isdigit(*p) ) i += *p - '0';;
	}
	return i;
}

static void abc_add_drum(ABCHANDLE *h, uint32_t tracktime, uint32_t bartime)
{
	ABCEVENT *e;
	ABCTRACK *tp;
	uint32_t etime, ctime , rtime, stime;
	int i, g, steps, gnote, gsteps, nnum;
	steps = abc_drum_steps(h->drum);
	ctime = h->barticks;
	// look up the last event in tpr drumtrack
	tp = abc_locate_track(h, h->tpr->v, DRUMPOS);
	e = tp->tail;
	etime = e? e->tracktick: bartime;
	if( etime > tracktime ) return;
	if( etime < bartime ) rtime = h->barticks - ((bartime - etime) % h->barticks);
	else rtime = (etime - bartime) % h->barticks;
	stime = ctime*steps;
	rtime *= steps;
	rtime += stime;
	gsteps = strlen(h->drum)/2;
	g = 0;
	while( rtime > stime ) {
		rtime -= ctime*(h->drum[g*2+1] - '0');
		if( ++g == gsteps ) g = 0;
	}
	stime = (tracktime - etime) * steps;
	rtime = 0;

	// if no drumsteps, there is nothing we can do anyway.
	if( steps == 0 )
		return;

	while( rtime < stime ) {
		gnote = h->drum[g*2];
		i = h->drum[g*2+1] - '0';
		if( gnote=='d') {
			tp->instr = pat_gm_drumnr(h->drumins[g]-1);
			nnum      = pat_gm_drumnote(h->drumins[g]);
			abc_add_drumnote(h, tp, etime + rtime/steps, nnum, h->drumvol[g]);
			abc_add_noteoff(h,tp,etime + ( rtime + ctime * i )/steps);
		}
		if( ++g == gsteps ) g = 0;
		rtime += ctime * i;
	}
}

static int abc_gchord_steps(const char *gch)
{
	const char *p;
	int i=0;
	for( p=gch; *p; p++ )
		if( isdigit(*p) ) i += *p - '0';
	return i;
}

static void abc_add_gchord(ABCHANDLE *h, uint32_t tracktime, uint32_t bartime)
{
	ABCEVENT *e, *c;
	ABCTRACK *tp;
	uint32_t etime, ctime , rtime, stime, modbarticks;
	int i, g, steps, gnote, gcnum, gsteps, nnum, glen;
	// look up the last chord event in tpc
	c = 0;
	for( e = h->tpc->head; e; e = e->next )
		if( e->flg == 1 && e->cmd == cmdchord )
			c = e;
	if( !c ) return;
	gcnum = c->par[chordnum];
	steps = abc_gchord_steps(h->gchord);
	ctime = h->barticks;
	// if chord time is 0, its useless to process further
	if (!ctime) return;
	etime = 0;
	for( i = GCHORDBPOS; i < DRUMPOS; i++ ) {
		tp = abc_locate_track(h, h->tpc->v, i);
		e = tp->tail;
		if( !e ) e = c;
		stime = e->tracktick;
		if( stime > etime )	etime = stime;
	}
	if( etime > tracktime ) return;
	modbarticks = h->barticks ? h->barticks : 1;
	if( etime < bartime ) rtime = h->barticks - ((bartime - etime) % modbarticks);
	else rtime = (etime - bartime) % modbarticks;
	stime = ctime * steps;
	rtime *= steps;
	rtime += stime;
	gsteps = strlen(h->gchord);
	g = 0;
	while( rtime > stime ) {
		glen = h->gchord[2*g+1] - '0';
		rtime -= ctime * glen;
		if( ++g == gsteps ) g = 0;
	}
	stime = (tracktime - etime) * steps;
	rtime = 0;
	while( rtime < stime ) {
		gnote = h->gchord[2*g];
		glen  = h->gchord[2*g+1] - '0';
		if( ++g == gsteps ) g = 0;
		switch(gnote) {
			case 'b':
				tp = abc_locate_track(h, h->tpc->v, GCHORDFPOS);
				tp->instr = h->abcbassprog;
				nnum = c->par[chordnote]+chordnotes[gcnum][0]+24;
				abc_add_chordnote(h, tp, etime + rtime/steps, nnum, h->abcbassvol);
				abc_add_noteoff(h,tp,etime + ( rtime + ctime * glen )/steps);
			case 'c':
				for( i = 1; i < chordlen[gcnum]; i++ ) {
					tp = abc_locate_track(h, h->tpc->v, i+GCHORDFPOS);
					tp->instr = h->abcchordprog;
					nnum = c->par[chordnote]+chordnotes[gcnum][i]+24;
					abc_add_chordnote(h, tp, etime + rtime/steps, nnum, h->abcchordvol);
					abc_add_noteoff(h,tp,etime + ( rtime + ctime * glen )/steps);
				}
				rtime += ctime * glen;
				break;
			case 'f':
				tp = abc_locate_track(h, h->tpc->v, GCHORDFPOS);
				tp->instr = h->abcbassprog;
				nnum = c->par[chordbase]+12;
				abc_add_chordnote(h, tp, etime + rtime/steps, nnum, h->abcbassvol);
				rtime += ctime * glen;
				abc_add_noteoff(h,tp,etime + rtime/steps);
				break;
			case 'g':
			case 'h':
			case 'i':
			case 'j':
			case 'G':
			case 'H':
			case 'I':
			case 'J':
				i = toupper(gnote) - 'G';
				nnum = 0;
				if( i < chordlen[gcnum] ) {
					tp = abc_locate_track(h, h->tpc->v, GCHORDFPOS+i+1);
					tp->instr = h->abcchordprog;
					nnum = c->par[chordnote]+chordnotes[gcnum][i]+24;
					if( isupper(gnote) ) nnum -= 12;
					abc_add_chordnote(h, tp, etime + rtime/steps, nnum, h->abcchordvol);
				}
				rtime += ctime * glen;
				if( nnum ) abc_add_noteoff(h,tp,etime + rtime/steps);
				break;
			case 'z':
				rtime += ctime * glen;
				break;
		}
	}
}

// %%MIDI beat a b c n
//
// controls the way note velocities are selected. The first note in a bar has
// velocity a. Other "strong" notes have velocity b and all the rest have velocity
// c. a, b and c must be in the range 0-128. The parameter n determines which
// notes are "strong". If the time signature is x/y, then each note is given
// a position number k = 0, 1, 2 .. x-1 within each bar. Note that the units for
// n are not the unit note length. If k is a multiple of n, then the note is
// "strong". The volume specifiers !ppp! to !fff! are equivalent to the
// following :
//
// !ppp! = %%MIDI beat 30 20 10 1
// !pp!  = %%MIDI beat 45 35 20 1
// !p!   = %%MIDI beat 60 50 35 1
// !mp!  = %%MIDI beat 75 65 50 1
// !mf!  = %%MIDI beat 90 80 65 1
// !f!   = %%MIDI beat 105 95 80 1
// !ff!  = %%MIDI beat 120 110 95 1
// !fff! = %%MIDI beat 127 125 110 1
static void abc_MIDI_beat(ABCHANDLE *h, const char *p)
{
	int i,j;
	h->beat[0] = 127;
	h->beat[1] = 125;
	h->beat[2] = 110;
	h->beat[3] = 1;
	for( j=0; j<4; j++ ) {
		while( isspace(*p) ) p++;
		if( *p ) {
			p += abc_getnumber(p, &i);
			if( i < 0   ) i = 0;
			if( i > 127 ) i = 127;
			h->beat[j] = i;
		}
	}
	if( h->beat[3] == 0 ) h->beat[3] = 1; // BB Ruud says: do not let you make mad
}

//
// %%MIDI beatstring <string of f, m and p>
//
// This provides an alternative way of specifying where the strong and weak
// stresses fall within a bar. 'f' means velocity a (normally strong), 'm'
// means velocity b (medium velocity) and 'p' means velocity c (soft velocity).
// For example, if the time signature is 7/8 with stresses on the first, fourth
// and sixth notes in the bar, we could use the following
//
// %%MIDI beatstring fppmpmp
static void abc_MIDI_beatstring(ABCHANDLE *h, const char *p)
{
	while( isspace(*p) ) p++;
	if( h->beatstring ) _mm_free(h->allochandle, h->beatstring);
	if( strlen(p) )
		h->beatstring = DupStr(h->allochandle,p,strlen(p)+1);
	else
		h->beatstring = NULL;
}

static int abc_beat_vol(ABCHANDLE *h, int abcvol, int barpos)
{
	int vol;
	if( h->beatstring ) {
		vol = (h->beat[2] * 9) / 10;
		if( barpos < (int)strlen(h->beatstring) ) {
			switch(h->beatstring[barpos]) {
				case 'f':
					vol = h->beat[0];
					break;
				case 'm':
					vol = h->beat[1];
					break;
				case 'p':
					vol = h->beat[2];
					break;
				default:
					break;
			}
		}
	}
	else {
		if( (barpos % h->beat[3]) == 0 ) {
			if( barpos )
				vol = h->beat[1];
			else
				vol = h->beat[0];
		}
		else
			vol = h->beat[2];
	}
	vol *= abcvol;
	vol /= 128;
	return vol;
}

static void abc_init_partpat(BYTE partp[27][2])
{
	int i;
	for( i=0; i<27; i++ ) {
		partp[i][0] = 0xff;
		partp[i][1] = 0;
	}
}

static int abc_partpat_to_orderlist(BYTE partp[27][2], const char *abcparts, ABCHANDLE *h, BYTE **list, int orderlen)
{
	int t, partsused;
	const char *p;
	BYTE *orderlist = *list;
	static int ordersize = 0;
	if( *list == NULL ) {
		ordersize = 128;
		orderlist = (BYTE *)_mm_calloc(h->ho, ordersize, sizeof(BYTE));
		*list = orderlist;
	}
	if( abcparts ) {
		partsused = 0;
		for( p = abcparts; *p; p++ ) {
			if (*p < 'A' || *p > 'Z') break;
			for( t = partp[*p - 'A'][0]; t < partp[*p - 'A'][1]; t++ ) {
				if( orderlen == ordersize ) {
					ordersize <<= 1;
					if (ordersize == 0) ordersize = 2;
					orderlist = (BYTE *)_mm_recalloc(h->ho, orderlist, ordersize, sizeof(BYTE));
					*list = orderlist;
				}
				orderlist[orderlen] = t;
				orderlen++;
				partsused++;
			}
		}
		if( partsused ) return orderlen;
	}
	// some fool wrote a P: string in the header but didn't use P: in the body
	for( t = partp[26][0]; t < partp[26][1]; t++ ) {
		if( orderlen == ordersize ) {
			ordersize <<= 1;
			if (ordersize == 0) ordersize = 2;
			orderlist = (BYTE *)_mm_recalloc(h->ho, orderlist, ordersize, sizeof(BYTE));
			*list = orderlist;
		}
		orderlist[orderlen] = t;
		orderlen++;
	}
	return orderlen;
}

static void abc_globalslide(ABCHANDLE *h, uint32_t tracktime, int slide)
{
	ABCTRACK *tp;
	ABCEVENT *e;
	int hslide;
	hslide = h->track? h->track->slidevol: slide;
	for( tp=h->track; tp; tp = tp->next ) {
		if( slide ) {
			tp->slidevoltime = tracktime;
			if( slide == 2 )
				tp->slidevol = 0;
		}
		if( tp->slidevol > -2 && slide < 2 )
			tp->slidevol = slide;
	}
	if( h->track && h->track->tail
	&& hslide != slide && slide == -2
	&& h->track->tail->tracktick >= tracktime ) {
		// need to update jumptypes in mastertrack from tracktime on...
		for( e=h->track->head; e; e=e->next )	{
			if( e->flg == 1 && e->cmd == cmdjump && e->tracktick >= tracktime ) {
				switch( e->par[jumptype] ) {
					case jumpnormal:
					case jumpfade:
						e->par[jumptype] = jumpfade;
						break;
					case jumpdacapo:
					case jumpdcfade:
						e->par[jumptype] = jumpdcfade;
						break;
					case jumpdasegno:
					case jumpdsfade:
						e->par[jumptype] = jumpdsfade;
						break;
				}
			}
		}
	}
}

static void abc_recalculate_tracktime(ABCHANDLE *h) {
	ABCTRACK *ttp;
	h->tracktime = 0;
	for( ttp=h->track; ttp; ttp=ttp->next )
		if( ttp->tail && ttp->tail->tracktick > h->tracktime )
			h->tracktime = ttp->tail->tracktick;
}

static void abc_MIDI_command(ABCHANDLE *h, char *p, char delim) {
	int t;
	// interpret some of the possibilitys
	if( !strncmp(p,"bassprog",8)    && isspace(p[8]) ) h->abcbassprog = abc_MIDI_getprog(p+8)+1;
	if( !strncmp(p,"bassvol",7)     && isspace(p[7]) ) h->abcbassvol = abc_MIDI_getnumber(p+7);
	if( !strncmp(p,"beat",4)        && isspace(p[4]) ) abc_MIDI_beat(h, p+4);
	if( !strncmp(p,"beatstring",10) && isspace(p[10]) ) abc_MIDI_beatstring(h, p+4);
	if( !strncmp(p,"chordname",9)   && isspace(p[9]) ) abc_MIDI_chordname(p+9);
	if( !strncmp(p,"chordprog",9)   && isspace(p[9]) ) h->abcchordprog = abc_MIDI_getprog(p+9)+1;
	if( !strncmp(p,"chordvol",8)    && isspace(p[8]) ) h->abcchordvol = abc_MIDI_getnumber(p+8);
	if( !strncmp(p,"drone",5)       && isspace(p[5]) ) abc_MIDI_drone(p+5, &h->dronegm, h->dronepitch, h->dronevol);
	if( !strncmp(p,"droneoff",8)    && (p[8]=='\0' || p[8]==delim || isspace(p[8])) ) h->droneon = 0;
	if( !strncmp(p,"droneon",7)     && (p[7]=='\0' || p[7]==delim || isspace(p[7])) ) h->droneon = 1;
	t = h->drumon;
	if( !strncmp(p,"drum",4)        && isspace(p[4]) ) {
		h->drumon = abc_MIDI_drum(p+4, h);
		if( h->drumon ) --h->drumon;
		else h->drumon = t;
	}
	if( !strncmp(p,"drumoff",7)     && (p[7]=='\0' || p[7]==delim || isspace(p[7])) ) h->drumon = 0;
	if( !strncmp(p,"drumon",6)      && (p[6]=='\0' || p[6]==delim || isspace(p[6])) ) h->drumon = 1;
	if( t != h->drumon ) {
		if( h->drumon && !h->tpr ) h->tpr = h->track;
		if( h->tpr ) abc_add_drum_sync(h, h->tpr, h->tracktime); // don't start drumming from the beginning of time!
		if( h->tpr && !h->drumon ) h->tpr = NULL;
	}
	t = h->gchordon;
	if( !strncmp(p,"gchord",6)      && (p[6]=='\0' || p[6]==delim || isspace(p[6])) ) {
		h->gchordon = abc_MIDI_gchord(p+6, h);
		if( h->gchordon ) --h->gchordon;
		else h->gchordon = t;
	}
	if( !strncmp(p,"gchordoff",9)   && (p[9]=='\0' || p[9]==delim || isspace(p[9])) ) h->gchordon = 0;
	if( !strncmp(p,"gchordon",8)    && (p[8]=='\0' || p[8]==delim || isspace(p[8])) ) h->gchordon = 1;
	if( t != h->gchordon ) {
		if( h->tpc ) abc_add_gchord_syncs(h, h->tpc, h->tracktime);
	}
	if( !strncmp(p,"channel",7)     && isspace(p[7]) )
		abc_MIDI_channel(p+8, h->tp = abc_check_track(h, h->tp), h);
	if( !strncmp(p,"program",7)     && isspace(p[7]) )
		abc_MIDI_program(p+8, h->tp = abc_check_track(h, h->tp), h);
	if( !strncmp(p,"voice",5)       && isspace(p[5]) )
		abc_MIDI_voice(p+6, h->tp = abc_check_track(h, h->tp), h);
	if( !strncmp(p,"legato",6)      && (p[6]=='\0' || p[6]==delim || isspace(p[6])) )
		abc_MIDI_legato(p+6, h->tp = abc_check_track(h, h->tp));
}

// continuate line that ends with a backslash, can't do this in abc_gets because voice lines
// can have comment lines in between that must be parsed properly, for example:
//  [V:1] cdef gabc' |\ << continuation backslash
//  %%MIDI program 25
//        c'bag fedc |
// informational lines can have this too, so it is rather convoluted code...
static char *abc_continuated(ABCHANDLE *h, MMFILE *mmf, char *p) {
	char *pm, *p1, *p2 = 0;
	int continued;
	pm = p;
	while( pm[strlen(pm)-1]=='\\' ) {
		p1 = strdup(pm);
		if( p2 ) free(p2);
		continued = 1;
		while( continued ) {
			continued = 0;
			pm = abc_gets(h, mmf);
			if( !pm ) {
				abc_message("line not properly continued\n%s", p1);
				return p1;
			}
			while( *pm && isspace(*pm) ) ++pm;
			if( !strncmp(pm,"%%",2) ) {
				for( p2 = pm+2; *p2 && isspace(*p2); p2++ ) ;
				if( !strncmp(p2,"MIDI",4) && (p2[4]=='=' || isspace(p2[4])) ) {
					for( p2+=5; *p2 && isspace(*p2); p2++ ) ;
					if( *p2 == '=' )
						for( p2+=1; *p2 && isspace(*p2); p2++ ) ;
					abc_MIDI_command(h,p2,'%');
				}
				continued = 1;
			}
		}
		p2 = (char *)malloc(strlen(p1)+strlen(pm));
		if( !p2 ) {
			abc_message("macro line too long\n%s", p1);
			return p1;
		}
		p1[strlen(p1)-1] = '\0'; // strip off the backslash
		strcpy(p2,p1);
		strcat(p2,pm);
		pm = p2;
		free(p1);
	}
	return pm;
}

// =====================================================================================
BOOL CSoundFile::ReadABC(const uint8_t *lpStream, DWORD dwMemLength)
{
	static int avoid_reentry = 0;
	ABCHANDLE *h;
	uint32_t numpat;
	MMFILE mm, *mmfile;
	uint32_t t;
	char	*line, *p, *pp, ch, ch0=0;
	char barsig[52];	// for propagated accidental key signature within bar
	char *abcparts;
	uint8_t partpat[27][2], *orderlist = NULL;
	int orderlen = 0;
	enum { NOWHERE, INBETWEEN, INHEAD, INBODY, INSKIPFORX, INSKIPFORQUOTE } abcstate;
	ABCEVENT_JUMPTYPE j;
	ABCEVENT_X_EFFECT abceffect;
	int abceffoper;
	int abcxcount=0, abcxwanted=0, abcxnumber=1;
	int abckey, abcrate, abcchord, abcvol, abcbeatvol, abcnoslurs, abcnolegato, abcfermata, abcarpeggio, abcto;
	int abctempo;
	int cnotelen=0, cnotediv=0, snotelen, snotediv, mnotelen, mnotediv, notelen, notediv;
	// c for chords, s for standard L: setting, m for M: barlength
	int abchornpipe, brokenrithm, tupletp, tupletq, tupletr;
	int ktempo;
	uint32_t abcgrace=0, bartime, thistime=0;
	ABCTRACK *tpd, *ttp;
	ABCMACRO *mp;
	int mmsp;
	MMFILE *mmstack[MAXABCINCLUDES];
	if( !TestABC(lpStream, dwMemLength) ) return FALSE;
	h = ABC_Init();
	if( !h ) return FALSE;
	mmfile = &mm;
	mm.mm = (char *)lpStream;
	mm.sz = dwMemLength;
	mm.pos = 0;
	while( avoid_reentry ) sleep(1);
	avoid_reentry = 1;
	pat_resetsmp();
	pat_init_patnames();
	m_nDefaultTempo = 0;
	global_voiceno = 0;
	abckey = 0;
	h->tracktime = 0;
	global_songstart = 0;
	h->speed = 6;
	abcrate = 240;
	global_tempo_factor = 2;
	global_tempo_divider = 1;
	abctempo = 0;
	ktempo = 0;
	abceffect = none;
	abceffoper = 0;
	abcvol = 120;
	h->abcchordvol  = abcvol;
	h->abcbassvol   = abcvol;
	h->abcchordprog = 25; // acoustic guitar
	h->abcbassprog  = 33; // acoustic bass
	abcparts = 0;
	abcnoslurs = 1;
	abcnolegato = 1;
	abcfermata = 0;
	abcarpeggio = 0;
	abcto = 0;
	snotelen = 0;
	snotediv = 0;
	mnotelen = 1;
	mnotediv = 1;
	abchornpipe = 0;
	brokenrithm = 0;
	tupletp = 0;
	tupletq = 0;
	tupletr = 0;
	h->ktrans = 0;
	h->drumon = 0;
	h->gchordon = 1;
	h->droneon = 0;
	h->tracktime = 0;
	bartime = 0;
	h->tp = NULL;
	h->tpc = NULL;
	h->tpr = NULL;
	tpd = NULL;
	h->dronegm       = 71;
	h->dronepitch[0] = 45;
	h->dronepitch[1] = 33;
	h->dronevol[0]   = 80;
	h->dronevol[1]   = 80;
	abc_new_umacro(h, "v = +downbow+");
	abc_new_umacro(h, "u = +upbow+");
	abc_new_umacro(h, "O = +coda+");
	abc_new_umacro(h, "S = +segno+");
	abc_new_umacro(h, "P = +uppermordent+");
	abc_new_umacro(h, "M = +lowermordent+");
	abc_new_umacro(h, "L = +emphasis+");
	abc_new_umacro(h, "H = +fermata+");
	abc_new_umacro(h, "T = +trill+");
	abc_new_umacro(h, "~ = +roll+");
	abc_setup_chordnames();
	abc_init_partpat(partpat);
	abc_MIDI_beat(h, ""); // reset beat array
	abc_MIDI_beatstring(h, ""); // reset beatstring
	mmsp       = 1;
	mmstack[0] = mmfile;
	mmfseek(mmfile,0,SEEK_SET);
	abcstate = NOWHERE;
	if( h->pickrandom ) {
		abcstate = INSKIPFORX;
		abcxcount = 0;
		mmfseek(mmfile,0,SEEK_SET);
		while ((line=abc_gets(h, mmfile)) != NULL) {
			for( p=line; isspace(*p); p++ ) ;
			if( !strncmp(p,"X:",2) ) abcxcount++;
		}
		if( abcxcount == 0 )
			abcstate = NOWHERE;
		else
			abcxwanted = (h->pickrandom - 1) % abcxcount;
		abcxcount = 0;
		mmfseek(mmfile,0,SEEK_SET);
	}
	while( mmsp > 0 ) {
		mmsp--;
		while ((line=abc_gets(h, mmstack[mmsp])) != NULL) {
			char blankline[3] = "% ";
			for( p=line; isspace(*p); p++ ) ;
			switch(abcstate) {
				case INSKIPFORX:
					if( !strncmp(p,"X:",2) ) {
						if( abcxcount++ != abcxwanted )
							break;
					}
					// fall through
				case INBETWEEN:
					if( !strncmp(p,"X:",2) ) {
						abcstate = INHEAD;
						memset(m_szNames[0], 0, 32);
						for( p+=2; isspace(*p); p++ ) ;
						abcxnumber = atoi(p);
						abchornpipe = 0;
						h->droneon = 0;
						h->dronegm       = 71;
						h->dronepitch[0] = 45;
						h->dronepitch[1] = 33;
						h->dronevol[0]   = 80;
						h->dronevol[1]   = 80;
						for( ttp = h->track; ttp; ttp=ttp->next ) {
							ttp->vno = 0;	// mark track unused
							ttp->capostart = NULL;
						}
						h->tp  = NULL; // forget old voices
						h->tpc = NULL;
						h->tpr = NULL;
						global_voiceno = 0;
						abc_set_parts(&abcparts, 0);
						abcgrace = 0;
						h->ktrans = 0;
						ktempo = 0;
						h->gchordon = 1;
						h->drumon = 0;
						global_songstart = h->tracktime;
						abc_MIDI_beat(h, ""); // reset beat array
						abc_MIDI_beatstring(h, ""); // reset beatstring
						strcpy(h->gchord, ""); // reset gchord string
						abcnolegato = 1; // reset legato switch
					}
					break;
				case NOWHERE:
					if( p[0] != '\0' && p[1] == ':' ) {
						abcstate = INHEAD;
						abc_set_parts(&abcparts, 0);
						strcpy(h->gchord, "");
						if( h->drumon && h->tpr ) abc_add_drum_sync(h, h->tpr, h->tracktime);
						if( h->tpc && !h->gchordon ) abc_add_gchord_syncs(h, h->tpc, h->tracktime);
						h->gchordon = 1;
						h->drumon = 0;
					}
					else
						break;
				case INHEAD:
					if( !strncmp(p,"L:",2) ) {
						sscanf(p+2," %d / %d", &snotelen, &snotediv);
						break;
					}
					if( !strncmp(p,"M:",2) ) {
						abc_M_field(p+2, &mnotelen, &mnotediv);
						break;
					}
					if( !strncmp(p,"P:",2) ) {
						abc_set_parts(&abcparts, p+2);
						break;
					}
					if( !strncmp(p,"Q:",2) ) {
						abctempo = abc_extract_tempo(p+2,0);
						ktempo = 1;
						if( h->track ) {
							// make h->tracktime start of a new age...
							abc_add_partbreak(h, h->track, h->tracktime);
							abc_add_tempo_event(h, h->track, h->tracktime, abctempo);
						}
						if( m_nDefaultTempo == 0 ) m_nDefaultTempo = abctempo;
						break;
					}
					if( !strncmp(p,"T:",2) ) {
						char buf[200];
						if( strchr(p,'%') ) *strchr(p,'%') = '\0';
						for( t=strlen(p)-1; isspace(p[t]); t-- )
							p[t]='\0';
						for( t=2; isspace(p[t]); t++ ) ;
						strcpy(buf,m_szNames[0]);
						if( strlen(buf) ) strcat(buf," "); // add a space
						// don't go past 200 bytes.
						strncat(buf, p+t, 200-strlen(buf)-1);
						if( strlen(buf) > 31 ) buf[31] = '\0'; // chop it of
						strcpy(m_szNames[0], buf);
						break;
					}
					if( !strncmp(p,"R:",2) ) {
						for( p+=2; isspace(*p); p++ ) ;
						if( !strncmp(p,"hornpipe",8) && (isspace(p[8]) || p[8]=='\0') ) abchornpipe = 1;
						else abchornpipe = 0;
						break;
					}
					if( !strncmp(p,"V:",2) ) {
						for( t=2; p[t]==' '; t++ ) ;
						h->tp = abc_locate_track(h, p+t, 0);
						abcvol = h->tp->volume;
						abcnolegato = !h->tp->legato;
						if( !abcnolegato ) abcnoslurs = 0;
						break;
					}
					if( !strncmp(p,"K:",2) ) {
						abcstate = INBODY;
						abckey = ABC_Key(p+2);
						sprintf(barsig, "%s%s", sig[abckey], sig[abckey]);	// reset the key signature
						p = abc_skip_word(p+2);
						h->ktrans = abc_transpose(p);
						p = blankline; // force skip rest of line
						if( snotelen == 0 ) {	// calculate default notelen from meter M:
							if( mnotediv == 0 ) mnotediv = mnotelen = 1;	// do'nt get nuked
							snotelen = 100 * mnotelen / mnotediv;
							if( snotelen > 74 )
								snotediv = 8;
							else
								snotediv = 16;
							snotelen = 1;
						}
						abceffect = none;
						abceffoper = 0;
						if( !(snotelen == 1 && snotediv == 8) ) abchornpipe = 0; // no matter what they said at R:
						brokenrithm = 0;
						global_part = ' ';
						abcgrace = 0;
						abcnoslurs = abcnolegato;
						abcto = 0;
						h->tpc = NULL; // reset chord track
						tpd = NULL; // reset drone track
						h->tpr = NULL; // reset drum  track
						if( !strlen(h->gchord) ) abc_metric_gchord(h, mnotelen, mnotediv);
						h->barticks = notelen_notediv_to_ticks(h->speed, mnotelen, mnotediv);
						if( abctempo && !ktempo ) {	// did not set tempo in this songpiece so reset to abcrate
							abctempo = 0;
							global_tempo_factor = 2;
							global_tempo_divider = 1;
							if( h->track ) {
								// make h->tracktime start of a new age...
								abc_add_partbreak(h, h->track, h->tracktime);
								abc_add_tempo_event(h, h->track, h->tracktime, abcrate);
							}
							if( m_nDefaultTempo == 0 ) m_nDefaultTempo = abcrate;
						}
						abc_init_partpat(partpat);
						partpat[26][0] = abc_patno(h, h->tracktime);
						partpat[26][1] = 0;
						abc_globalslide(h, h->tracktime, 2); // reset all volumeslides
						break;
					}
					if( !strlen(p) )
						abcstate = INBETWEEN;
					break;
				case INSKIPFORQUOTE:
						while( *p && *p != '"' )
							p++;
						if( *p == '\0' )
							break;
						abcstate = INBODY;
						// fall through
				case INBODY:
					if( !strlen(p) && h->track ) { // end of this song
						abcstate = h->pickrandom? INSKIPFORX: INBETWEEN;
						// last but not least shut off all pending events
						abc_recalculate_tracktime(h);
						for( ttp=h->track; ttp; ttp=ttp->next )
							abc_add_noteoff(h,ttp,h->tracktime);
						abc_add_partbreak(h, h->track, h->tracktime);
						t = abc_patno(h, h->tracktime);
						if( abc_pattracktime(h, h->tracktime) % abcticks(64 * h->speed) ) t++;
						if(	global_part == ' ' ) {
							partpat[26][1] = t;
							if( abcparts ) {
								for( t=0; t<26; t++ )
									if( partpat[t][0] < partpat[t][1] ) break;
								if( t == 26 ) {
									abc_message("parts (%s) set but not used", abcparts);
									abc_set_parts(&abcparts, 0);	// forget the parts array
								}
							}
						}
						else
							partpat[global_part - 'A'][1] = t;
						if( !abcparts ) abc_song_to_parts(h, &abcparts, partpat);
						orderlen = abc_partpat_to_orderlist(partpat, abcparts, h, &orderlist, orderlen);
					}
					if( !strncmp(p,"V:",2) ) {
						for( t=2; p[t]==' '; t++ ) ;
						h->tp = abc_locate_track(h, p+t, 0);
						sprintf(barsig, "%s%s", sig[abckey], sig[abckey]);	// reset the key signature
						abcgrace = 0;
						brokenrithm = 0;
						h->tracktime = abc_tracktime(h->tp);
						bartime = h->tracktime; // it is not friendly to break voices in the middle of a track...
						abcnolegato = !h->tp->legato;
						if( !abcnolegato ) abcnoslurs = 0;
						*p = '%';	// make me skip the rest of the line....
					}
					if( !strncmp(p,"K:",2) ) {
						abckey = ABC_Key(p+2);
						sprintf(barsig, "%s%s", sig[abckey], sig[abckey]);	// reset the key signature
						p = abc_skip_word(p+2);
						h->ktrans = abc_transpose(p);
						p = blankline;	// make me skip the rest of the line....
					}
					if( !strncmp(p,"L:",2) ) {
						sscanf(p+2," %d / %d", &snotelen, &snotediv);
						*p = '%';	// make me skip the rest of the line....
					}
					if( !strncmp(p,"M:",2) ) {
						abc_M_field(p+2, &mnotelen, &mnotediv);
						h->barticks = notelen_notediv_to_ticks(h->speed, mnotelen, mnotediv);
						*p = '%';	// make me skip the rest of the line....
					}
					if( !strncmp(p,"Q:",2) ) {
						abctempo = abc_extract_tempo(p+2,ch0=='\\');
						if( !h->track ) {
							h->tp = abc_check_track(h, h->track);
							h->tp->vno = 0; // mark reuseable (temporarely, until first notes come up)
						}
						abc_add_tempo_event(h, h->track, h->tracktime, abctempo);
						*p = '%';	// make me skip the rest of the line....
					}
					if( !strncmp(p,"T:",2) ) {
						char buf[200];
						if( strchr(p,'%') ) *strchr(p,'%') = '\0';
						for( t=strlen(p)-1; isspace(p[t]); t-- )
							p[t]='\0';
						for( t=2; isspace(p[t]); t++ ) ;
						strcpy(buf,m_szNames[0]);
						if( strlen(buf) + strlen(p+t) > 198 ) p[t+198-strlen(buf)] = '\0'; // chop it of
						if( strlen(buf) ) strcat(buf," "); // add a space
						strcat(buf, p+t);
						if( strlen(buf) > 31 ) buf[31] = '\0'; // chop it of
						strcpy(m_szNames[0], buf);
						*p = '%';	// make me skip the rest of the line....
					}
					break;
			}
			if( !strncmp(p,"m:",2) ) {
				if( abcstate != INSKIPFORX ) {
					char *pm = p;
					if (mmstack[mmsp]->pos < (LONG)dwMemLength) {
						pm = abc_continuated(h, mmstack[mmsp], p);
						if (pm+2)
							abc_new_macro(h, pm+2);
					}
					if( pm != p ) {
						free(pm);
						if( h->tp ) abcnolegato = !h->tp->legato;
						if( !abcnolegato ) abcnoslurs = 0;
					}
				}
				// cannot edit p, as it might not exist due to continuated.
				p = blankline; // skip rest of line
			}
			if( !strncmp(p,"U:",2) ) {
				abc_new_umacro(h, p+2);
				*p = '%'; // skip rest of line
			}
			if( !strncmp(p,"w:",2) ) {	// inline lyrics
				*p = '%'; // skip rest of line
			}
			if( !strncmp(p,"W:",2) ) {	// lyrics at end of song body
				*p = '%'; // skip rest of line
			}
			if( !strncmp(p,"d:",2) ) {	// oldstyle decorations
				abc_message("warning: old style decorations not handled\n%s", p);
				*p = '%'; // skip rest of line
			}
			if( !strncmp(p,"s:",2) ) {	// newstyle decorations (symbols)
				abc_message("warning: new style decorations not handled\n%s", p);
				*p = '%'; // skip rest of line
			}
			if( !strncmp(p,"I:",2) && abcstate != INSKIPFORX ) { // handle like oldstyle '%%command' lines
				p[0]= '%';
				p[1]= '%';
			}
			if( !strncmp(p,"%%",2) ) {
				for( p+=2; *p && isspace(*p); p++ ) ;
				if( !strncmp(p,"abc-include",11) && isspace(p[11]) ) {
					for( t=12; isspace(p[t]); t++ ) ;
					if( p[t] ) {
						mmsp++;
						if( mmsp == MAXABCINCLUDES ) {
							mmsp--;
							abc_message("failure: too many abc-include's, %s", &p[t]);
						} else {
							mmstack[mmsp] = mmfopen(&p[t], "r");
							if( !mmstack[mmsp] ) {
								mmsp--;
								abc_message("failure: abc-include file %s not found", &p[t]);
							}
						}
					}
					else abc_message("failure: abc-include missing file name, %s", p);
				}
				if( !strncmp(p,"MIDI",4) && (p[4]=='=' || isspace(p[4])) && abcstate != INSKIPFORX ) {
					for( p+=5; *p && isspace(*p); p++ ) ;
					if( *p == '=' )
						for( p+=1; *p && isspace(*p); p++ ) ;
					abc_MIDI_command(h,p,'%');
					if( h->tp ) abcnolegato = !h->tp->legato;
					if( !abcnolegato ) abcnoslurs = 0;
				}
				if(*p) *p = '%'; // skip rest of line
			}
			if( abcstate == INBODY ) {
				if( *p == 'P' && p[1] == ':' ) {	// a line with a part indication
					if( abcparts != NULL ) {
						// make h->tracktime start of a new age...
						if( !h->track ) {
							h->tp = abc_check_track(h, h->track);
							h->tp->vno = 0; // mark reuseable (temporarely, until first notes come up)
						}
						h->tracktime = h->track? abc_tracktime(h->track): 0; // global parts are voice independent
						abc_add_partbreak(h, h->track, h->tracktime);
						t = abc_patno(h, h->tracktime);
						if(	global_part == ' ' ) {
							partpat[26][1] = t;
							if( abcparts ) {
								for( t=0; t<26; t++ )
									if( partpat[t][0] < partpat[t][1] ) break;
								if( t == 26 ) {
									abc_message("parts (%s) set but not used", abcparts);
									abc_set_parts(&abcparts, 0);	// forget the parts array
								}
							}
						}
						else
							partpat[global_part - 'A'][1] = t;
						// give every new coming abcevent the desired part indication
						while( p[2]==' ' || p[2]=='.' ) p++;	// skip blancs and dots
						if( isupper(p[2]) )
							global_part = p[2];
						else
							global_part = ' ';
						if(	global_part == ' ' )
							partpat[26][0] = t;
						else
							partpat[global_part - 'A'][0] = t;
					}
					*p = '%';	// make me skip the rest of the line....
				}
				if( h->droneon && !tpd ) {
					tpd = h->track;
					if( tpd ) {
						tpd = abc_locate_track(h, tpd->v, DRONEPOS1);
						tpd->instr = h->dronegm;
						abc_add_dronenote(h, tpd, h->tracktime, h->dronepitch[0], h->dronevol[0]);
						tpd = abc_locate_track(h, tpd->v, DRONEPOS2);
						tpd->instr = h->dronegm;
						abc_add_dronenote(h, tpd, h->tracktime, h->dronepitch[1], h->dronevol[1]);
					}
				}
				if( tpd && !h->droneon ) {
					tpd = abc_locate_track(h, tpd->v, DRONEPOS1);
					abc_add_noteoff(h, tpd, h->tracktime);
					tpd = abc_locate_track(h, tpd->v, DRONEPOS2);
					abc_add_noteoff(h, tpd, h->tracktime);
					tpd = NULL;
				}
				if( h->drumon && !h->tpr ) {
					h->tpr = h->track;
					if( h->tpr ) abc_add_drum_sync(h, h->tpr, h->tracktime); // don't start drumming from the beginning of time!
				}
				if( h->tpr && !h->drumon ) h->tpr = NULL;
				if( *p && *p != '%' ) {	// skip uninteresting lines
					// plough thru the songline gathering mos....
					ch0 = ' ';
					pp = 0;
					while (*p && (ch = *p++) != '\0') {
						if( !pp && isalpha(ch) && *p != ':' ) { // maybe a macro
							for( mp=h->umacro; mp; mp=mp->next ) {
								if( ch == mp->name[0] ) {
									pp = p;
									p = mp->subst;
									ch = *p;
									if( ch ) p++;
									break;
								}
							}
						}
						switch(ch) {
							case '%':
								abcto = 0;
								while( *p ) p++;
								break;
							case '[':	// chord follows or some inline field
								abcto = 0;
								if( *p=='|' ) break; // [| a thick-thin bar line, loop around and let case '|' handle it
								if( !strncmp(p,"V:",2) ) {	// inline voice change
									for( t=2; isspace(p[t]); t++ ) ;
									h->tp = abc_locate_track(h, p+t, 0);
									for( ; *p && *p != ']'; p++ ) ;
									abcgrace = 0;
									brokenrithm = 0;
									sprintf(barsig, "%s%s", sig[abckey], sig[abckey]);	// reset the key signature
									h->tracktime = abc_tracktime(h->tp);
									bartime = h->tracktime; // it is not wise to break voices in the middle of a track...
									abcvol = h->tp->volume;
									abcnolegato = !h->tp->legato;
									if( !abcnolegato ) abcnoslurs = 0;
									break;
								}
								if( !strncmp(p,"K:",2) ) {
									abckey = ABC_Key(p+2);
									sprintf(barsig, "%s%s", sig[abckey], sig[abckey]);	// reset the key signature
									p = abc_skip_word(p+2);
									h->ktrans = abc_transpose(p);
									for( ; *p && *p != ']'; p++ ) ;
									break;
								}
								if( !strncmp(p,"M:",2) ) {
									abc_M_field(p+2, &mnotelen, &mnotediv);
									for( ; *p && *p != ']'; p++ ) ;
									h->barticks = notelen_notediv_to_ticks(h->speed, mnotelen, mnotediv);
									break;
								}
								if( !strncmp(p,"P:",2) ) {	// a [P:X] field inline
									if( abcparts != NULL ) {
										// make h->tracktime start of a new age...
										if( h->track )
											abc_add_partbreak(h, h->track, h->tracktime);
										t = abc_patno(h, h->tracktime);
										if(	global_part == ' ' )
											partpat[26][1] = t;
										else
											partpat[global_part - 'A'][1] = t;
										// give every new coming abcevent the desired part indication
										while( isspace(p[2]) || p[2]=='.' ) p++;	// skip blancs and dots
										if( isupper(p[2]) )
											global_part = p[2];
										else
											global_part = ' ';
										if(	global_part == ' ' )
											partpat[26][0] = t;
										else
											partpat[global_part - 'A'][0] = t;
									}
									for( ; *p && *p != ']'; p++ ) ;
									break;
								}
								if( !strncmp(p,"Q:",2) ) {
									abctempo = abc_extract_tempo(p+2,1);
									for( ; *p && *p != ']'; p++ ) ;
									if( h->track )
										abc_add_tempo_event(h, h->track, h->tracktime, abctempo);
									break;
								}
								if( !strncmp(p,"I:",2) ) { // interpret some of the possibilitys
									for( p += 2; isspace(*p); p++ ) ;
									if( !strncmp(p,"MIDI",4) && (p[4]=='=' || isspace(p[4])) ) { // interpret some of the possibilitys
										for( p += 4; isspace(*p); p++ ) ;
										if( *p == '=' )
											for( p += 1; isspace(*p); p++ ) ;
										abc_MIDI_command(h, p, ']');
										if( h->tp ) abcnolegato = !h->tp->legato;
										if( !abcnolegato ) abcnoslurs = 0;
									}
									for( ; *p && *p != ']'; p++ ) ; // skip rest of inline field
								}
								if( *p && p[1] == ':' ) { // some other kind of inline field
									for( ; *p && *p != ']'; p++ ) ;
									break;
								}
								if( *p && strchr("abcdefgABCDEFG^_=",*p) ) {
									int cnl[8],cnd[8],vnl,nl0=0,nd0=0,barticks;	// for chords with notes of varying length
									memset(cnl, 0, sizeof(cnl));
									memset(cnd, 0, sizeof(cnd));
									barticks = notelen_notediv_to_ticks(h->speed,1,mnotediv);
									if (barticks == 0) barticks = 1;
									abcchord = 0;
									vnl = 0;
									h->tp = abc_check_track(h, h->tp);
									abc_track_clear_tiedvpos(h);
									abcbeatvol = abc_beat_vol(h, abcvol, (h->tracktime - bartime)/barticks);
									while (*p && (ch=*p++) != '\0' && (ch != ']')) {
										h->tp = abc_locate_track(h, h->tp->v, abcchord? abcchord+DRONEPOS2: 0);
										p += abc_add_noteon(h, ch, p, h->tracktime, barsig, abcbeatvol, abceffect, abceffoper);
										p += abc_notelen(p, &notelen, &notediv);
										if( *p == '-' ) {
											p++;
											if( h->tp->tail && h->tp->tail->flg != 1 )
											h->tp->tienote = h->tp->tail;
										}
										if( abcchord<8 ) {
											cnl[abcchord] = notelen;
											cnd[abcchord] = notediv;
										}
										if( abcchord==0 ) {
											cnotelen = notelen;
											cnotediv = notediv;
											nl0 = notelen;
											nd0 = notediv;
										}
										else {
											if( cnotelen != notelen || cnotediv != notediv ) {
												vnl = 1;
												// update to longest duration
												if( cnotelen * notediv < notelen * cnotediv ) {
													cnotelen = notelen;
													cnotediv = notediv;
													abc_track_untie_short_chordnotes(h);
												}
												if( cnotelen * notediv > notelen * cnotediv ) {
													if( h->tp->tienote ) {
														abc_message("short notes in chord can not be tied:\n%s", h->line);
														h->tp->tienote = 0; // short chord notes cannot be tied...
													}
												}
												// update to shortest duration
												if( nl0 * notediv > notelen * nd0 ) {
													nl0 = notelen;
													nd0 = notediv;
												}
											}
										}
										abcchord++;
									}
									p += abc_notelen(p, &notelen, &notediv);
									if( (ch = *p) == '-' ) p++;	// tied chord...
									if( abcarpeggio ) {	// update starttime in the noteon events...
										thistime = notelen_notediv_to_ticks(h->speed, nl0*notelen*snotelen, nd0*notediv*snotediv)/abcchord;
										if( thistime > abcticks(h->speed) ) thistime = abcticks(h->speed);
										for( nl0=1; nl0<abcchord; nl0++ ) {
											h->tp = abc_locate_track(h, h->tp->v, nl0+DRONEPOS2);
											if( h->tp->tail )
												h->tp->tail->tracktick = h->tracktime + thistime * nl0;
										}
									}
									notelen *= cnotelen;
									notediv *= cnotediv;
									tupletr = abc_tuplet(&notelen, &notediv, tupletp, tupletq, tupletr);
									while( isspace(*p) ) p++;	// allow spacing in broken rithm notation
									p += abc_brokenrithm(p, &notelen, &notediv, &brokenrithm, abchornpipe);
									thistime = notelen_notediv_to_ticks(h->speed, notelen*snotelen, notediv*snotediv);
									if( abcfermata ) {
										thistime <<= 1;
										abcfermata = 0;
									}
									if( thistime > abcgrace ) {
										thistime -= abcgrace;
										abcgrace = 0;
									}
									else {
										abcgrace -= thistime;
										thistime = abcticks(h->speed);
										abcgrace += abcticks(h->speed);
									}
									h->tracktime += thistime;
									while( abcchord>0 ) {
										abcchord--;
										h->tp = abc_locate_track(h, h->tp->v, abcchord? abcchord+DRONEPOS2: 0);
										if( vnl && (abcchord < 8) && cnotelen && cnd[abcchord] &&
											(cnl[abcchord] != cnotelen || cnd[abcchord] != cnotediv) ) {
											abc_add_noteoff(h, h->tp,
												h->tracktime - thistime
											 	+ (thistime * cnl[abcchord] * cnotediv)/(cnd[abcchord] * cnotelen) );
										}
										else {
											if( ch=='-' && h->tp->tail && h->tp->tail->flg != 1 )
												h->tp->tienote = h->tp->tail;	// copy noteon event to tienote in track
											if( thistime > abcticks(h->speed) )
												abc_add_noteoff(h, h->tp, h->tracktime - abcnoslurs);
											else
												abc_add_noteoff(h, h->tp, h->tracktime);
										}
									}
									if( h->gchordon && (h->tp == h->tpc) )
										abc_add_gchord(h, h->tracktime, bartime);
									if( h->drumon && (h->tp == h->tpr) )
										abc_add_drum(h, h->tracktime, bartime);
									abcarpeggio = 0;
									if( abceffoper != 255 ) abceffect = none;
									break;
								}
								if( isdigit(*p) ) {	// different endings in repeats [i,j,n-r,s,...
									h->tp = abc_check_track(h, h->tp);
									abc_add_partbreak(h, h->tp, h->tracktime);
									p += abc_getnumber(p, &notelen);
									abc_add_variant_start(h, h->tp, h->tracktime, notelen);
									while( *p==',' || *p=='-' ) {
										if( *p==',' ) {
											p++;
											p += abc_getnumber(p, &notelen);
											abc_add_variant_choise(h->tp, notelen);
										}
										else {
											p++;
											p += abc_getnumber(p, &notediv);
											while( notelen < notediv ) {
												notelen++;
												abc_add_variant_choise(h->tp, notelen);
											}
										}
									}
									break;
								}
								// collect the notes in the chord
								break;
							case '(':	// slurs follow or some tuplet (duplet, triplet etc.)
								abcto = 0;
								if( isdigit(*p) ) {
									p += abc_getnumber(p,&tupletp);
									tupletr = tupletp;	// ABC draft 2.0 (4.13): if r is not given it defaults to p
									switch( tupletp ) {	// ABC draft 2.0 (4.13): q defaults depending on p and time signature
										case 2: case 4: case 8:
											tupletq = 3;
											break;
										case 3: case 6:
											tupletq = 2;
											break;
										default:
											if( snotediv == 8 )
												tupletq = 3;
											else
												tupletq = 2;
											break;
									}
									if( *p==':' ) {
										p++;
										if( isdigit(*p) ) p += abc_getnumber(p,&tupletq);
										if( *p==':' ) {
											p++;
											if( isdigit(*p) ) p += abc_getnumber(p,&tupletr);
										}
									}
								}
								else
									abcnoslurs=0;
								break;
							case ')':	// end of slurs
								abcto = 0;
								abcnoslurs = abcnolegato;
								break;
							case '{':	// grace notes follow
								{
								abcto = 0;
								h->tp = abc_check_track(h, h->tp);
								abc_track_clear_tiedvpos(h);
								abcgrace = 0;
								int barticks = notelen_notediv_to_ticks(h->speed,1,mnotediv);
								if (barticks == 0) barticks = 1;
								abcbeatvol = abc_beat_vol(h, abcvol, (h->tracktime - bartime)/barticks);
								while (*p && (ch=*p++) != '\0' && (ch != '}')) {
									p += abc_add_noteon(h, ch, p, h->tracktime+abcgrace, barsig, abcbeatvol, none, 0);
									p += abc_notelen(p, &notelen, &notediv);
									if( *p=='-' ) {
										p++;
										if( h->tp->tail->flg != 1 )
										h->tp->tienote = h->tp->tail;
									}
									notediv *= 4;	// grace notes factor 4 shorter (1/8 => 1/32)
									abcgrace += notelen_notediv_to_ticks(h->speed, notelen*snotelen, notediv*snotediv);
									abc_add_noteoff(h, h->tp, h->tracktime + abcgrace);
								}
								h->tracktime += abcgrace;
								abc_add_sync(h, h->tp, h->tracktime);
								if( h->gchordon && (h->tp == h->tpc) )
									abc_add_gchord(h, h->tracktime, bartime);
								if( h->drumon && (h->tp == h->tpr) )
									abc_add_drum(h, h->tracktime, bartime);
								}
								break;
							case '|':	// bar symbols
								abcto = 0;
								if( h->gchordon && h->tp && (h->tp == h->tpc) )
									abc_add_gchord(h, h->tracktime, bartime);
								if( h->drumon && (h->tp == h->tpr) )
									abc_add_drum(h, h->tracktime, bartime);
								sprintf(barsig, "%s%s", sig[abckey], sig[abckey]);	// reset the key signature
								bartime = h->tracktime;
								if( h->tp && h->tp->vpos ) h->tp = abc_locate_track(h, h->tp->v, 0); // reset from voice overlay
								if( isdigit(*p) ) {	// different endings in repeats |i,j,n-r,s,...
									h->tp = abc_check_track(h, h->tp);
									abc_add_partbreak(h, h->tp, h->tracktime);
									p += abc_getnumber(p, &notelen);
									abc_add_variant_start(h, h->tp, h->tracktime, notelen);
									while( *p==',' || *p=='-' ) {
										if( *p==',' ) {
											p++;
											p += abc_getnumber(p, &notelen);
											abc_add_variant_choise(h->tp, notelen);
										}
										else {
											p++;
											p += abc_getnumber(p, &notediv);
											while( notelen < notediv ) {
												notelen++;
												abc_add_variant_choise(h->tp, notelen);
											}
										}
									}
									break;
								}
								if( *p==':' ) {	// repeat start
									p++;
									h->tp = abc_check_track(h, h->tp);
									abc_add_partbreak(h, h->tp, h->tracktime);
									abc_add_setloop(h, h->tp, h->tracktime);
								}
								break;
							case '&': // voice overlay
								abcto = 0;
								h->tracktime = bartime;
								h->tp = abc_check_track(h, h->tp);
								t = h->tp->vpos;
								h->tp = abc_locate_track(h, h->tp->v, t? t+1: DRONEPOS2+1);
								break;
							case ']':	// staff break, end of song
								abcto = 0;
								break;
							case ':': // repeat jump
								abcto = 0;
								h->tp = abc_check_track(h, h->tp);
								j = (h->tp->slidevol == -2)? jumpfade: jumpnormal;
								abc_add_setjumploop(h, h->tp, h->tracktime, j);
								abc_add_partbreak(h, h->tp, h->tracktime);
								if( *p==':' ) {	// repeat start without intermediate bar symbol
									p++;
									abc_add_setloop(h, h->tp, h->tracktime);
								}
								break;
							case '"':	// chord notation
								if( !strchr("_^<>@", *p) && !isdigit(*p) ) { // if it's not a annotation string
									h->tp = abc_check_track(h, h->tp);
									if( !h->tpc ) h->tpc = abc_locate_track(h, h->tp->v, 0);
 									if( h->tp == h->tpc ) abc_add_chord(p, h, h->tpc, h->tracktime); // only do chords for one voice
								}
								abcto = 0;
								while (*p && (ch=*p++) != '\0' && (ch != '"')) {
									if( !strncasecmp(p,"fade",4) && h->track && h->track->slidevol > -2 )
										abc_globalslide(h, h->tracktime, -2); // set volumeslide to fade away...
									if( !strncasecmp(p,"to coda",7) ) {
										h->tp = abc_check_track(h, h->tp);
										abc_add_partbreak(h, h->tp, h->tracktime);
										abc_add_tocoda(h, h->tp, h->tracktime);
										p+=7;
										abcto = -1;
									}
									else
										if( !isspace(*p) ) abcto = 0;
									if( !strncasecmp(p,"to",2) && (isspace(p[2]) || p[2] == '"') ) abcto = 1;
								}
								if( !ch ) abcstate = INSKIPFORQUOTE;
								break;
							case '\\':	// skip the rest of this line, should be the end of the line anyway
								while( *p ) p++;
								ch = '\\'; // remember for invoice tempo changes....
								break;
							case '!':	// line break, or deprecated old style decoration
							case '+': // decorations new style
								if( !strncmp(p,"coda",4) && p[4] == ch ) {
									h->tp = abc_check_track(h, h->tp);
									if( abcto ) {
										if( abcto > 0 ) {
											abc_add_partbreak(h, h->tp, h->tracktime);
											abc_add_tocoda(h, h->tp, h->tracktime);
										}
									}
									else {
										abc_add_partbreak(h, h->tp, h->tracktime);
										abc_add_coda(h, h->tp, h->tracktime);
									}
									p += 5;
									abcto = 0;
									break;
								}
								abcto = 0;
								if( !strncmp(p,"arpeggio",8) && p[8] == ch ) {
									abcarpeggio = 1;
									p += 9;
									break;
								}
								if( !strncmp(p,"crescendo(",10) && p[10] == ch ) {
									h->tp = abc_check_track(h, h->tp);
									abc_globalslide(h, h->tracktime, 1);
									p += 11;
									break;
								}
								if( !strncmp(p,"crescendo)",10) && p[10] == ch ) {
									h->tp = abc_check_track(h, h->tp);
									abc_globalslide(h, h->tracktime, 0);
									p += 11;
									break;
								}
								if( !strncmp(p,"<(",2) && p[2] == ch ) {
									h->tp = abc_check_track(h, h->tp);
									abc_globalslide(h, h->tracktime, 1);
									p += 3;
									break;
								}
								if( !strncmp(p,"<)",2) && p[2] == ch ) {
									h->tp = abc_check_track(h, h->tp);
									abc_globalslide(h, h->tracktime, 0);
									p += 3;
									break;
								}
								if( !strncmp(p,"dimimuendo(",11) && p[11] == ch ) {
									h->tp = abc_check_track(h, h->tp);
									abc_globalslide(h, h->tracktime, -1);
									p += 12;
									break;
								}
								if( !strncmp(p,"diminuendo)",11) && p[11] == ch ) {
									h->tp = abc_check_track(h, h->tp);
									abc_globalslide(h, h->tracktime, 0);
									p += 12;
									break;
								}
								if( !strncmp(p,">(",2) && p[2] == ch ) {
									h->tp = abc_check_track(h, h->tp);
									abc_globalslide(h, h->tracktime, -1);
									p += 3;
									break;
								}
								if( !strncmp(p,">)",2) && p[2] == ch ) {
									h->tp = abc_check_track(h, h->tp);
									abc_globalslide(h, h->tracktime, 0);
									p += 3;
									break;
								}
								if( !strncmp(p,"upbow",5) && p[5] == ch ) {
									abceffect = bow;
									abceffoper = 1;
									p += 6;
									break;
								}
								if( !strncmp(p,"downbow",7) && p[7] == ch ) {
									abceffect = bow;
									abceffoper = 0;
									p += 8;
									break;
								}
								if( !strncmp(p,"trill",5) && p[5] == ch ) {
									abceffect = trill;
									abceffoper = 0;
									p += 6;
									break;
								}
								if( !strncmp(p,"trill(",6) && p[6] == ch ) {
									abceffect = trill;
									abceffoper = 255;
									p += 7;
									break;
								}
								if( !strncmp(p,"trill)",6) && p[6] == ch ) {
									abceffect = none;
									abceffoper = 0;
									p += 7;
									break;
								}
								if( !strncmp(p,"accent",6) && p[6] == ch ) {
									abceffect = accent;
									abceffoper = 0;
									p += 7;
									break;
								}
								if( !strncmp(p,"emphasis",8) && p[8] == ch ) {
									abceffect = accent;
									abceffoper = 0;
									p += 9;
									break;
								}
								if( !strncmp(p,">",1) && p[1] == ch ) {
									abceffect = accent;
									abceffoper = 0;
									p += 2;
									break;
								}
								if( !strncmp(p,"fermata",7) && p[7] == ch ) {
									abcfermata = 1;
									p += 8;
									break;
								}
								if( !strncmp(p,"fine",4) && p[4] == ch ) {
									h->tp = abc_check_track(h, h->tp);
									abc_add_partbreak(h, h->tp, h->tracktime);
									abc_add_fine(h, h->tp, h->tracktime);
									p += 5;
									break;
								}
								if( !strncmp(p,"segno",5) && p[5] == ch ) {
									h->tp = abc_check_track(h, h->tp);
									abc_add_partbreak(h, h->tp, h->tracktime);
									abc_add_segno(h, h->tp, h->tracktime);
									p += 6;
									break;
								}
								if( !strncmp(p,"tocoda",6) && p[6] == ch ) {
									h->tp = abc_check_track(h, h->tp);
									abc_add_partbreak(h, h->tp, h->tracktime);
									abc_add_tocoda(h, h->tp, h->tracktime);
									p += 7;
									break;
								}
								if( !strncmp(p,"D.C.",4) && p[4] == ch ) {
									h->tp = abc_check_track(h, h->tp);
									j = (h->tp->slidevol == -2)? jumpdcfade: jumpdacapo;
									abc_add_setjumploop(h, h->tp, h->tracktime, j);
									abc_add_partbreak(h, h->tp, h->tracktime);
									p += 5;
									break;
								}
								if( !strncmp(p,"D.S.",4) && p[4] == ch ) {
									h->tp = abc_check_track(h, h->tp);
									j = (h->tp->slidevol == -2)? jumpdsfade: jumpdasegno;
									abc_add_setjumploop(h, h->tp, h->tracktime, j);
									abc_add_partbreak(h, h->tp, h->tracktime);
									p += 5;
									break;
								}
								if( !strncmp(p,"dacapo",6) && p[6] == ch ) {
									h->tp = abc_check_track(h, h->tp);
									j = (h->tp->slidevol == -2)? jumpdcfade: jumpdacapo;
									abc_add_setjumploop(h, h->tp, h->tracktime, j);
									abc_add_partbreak(h, h->tp, h->tracktime);
									p += 7;
									break;
								}
								if( !strncmp(p,"dacoda",6) && p[6] == ch ) {
									h->tp = abc_check_track(h, h->tp);
									j = (h->tp->slidevol == -2)? jumpdcfade: jumpdacapo;
									abc_add_setjumploop(h, h->tp, h->tracktime, j);
									abc_add_partbreak(h, h->tp, h->tracktime);
									p += 7;
									break;
								}
								if( ch == '!' ) {
									for( t=0; p[t] && strchr("|[:]!",p[t])==0 && !isspace(p[t]); t++ ) ;
									if( p[t] == '!' ) {	// volume and other decorations, deprecated
										h->tp = abc_check_track(h, h->tp);
										abcvol = abc_parse_decorations(h, h->tp, p);
										p = &p[t+1];
									}
								}
								else {
									h->tp = abc_check_track(h, h->tp);
									abcvol = abc_parse_decorations(h, h->tp, p);
									while (*p && (ch=*p++) != '\0' && (ch != '+'))
										;
								}
								break;
							case '`': // back quotes are for readability
								break;
							case '.':	// staccato marks
								break;
							default:	// some kinda note must follow
								if( strchr("abcdefgABCDEFG^_=X",ch) ) {
									int barticks = notelen_notediv_to_ticks(h->speed, 1, mnotediv);
									if (barticks == 0) barticks = 1;
									h->tp = abc_check_track(h, h->tp);
									abc_track_clear_tiedvpos(h);
									abcbeatvol = abc_beat_vol(h, abcvol, (h->tracktime - bartime)/barticks);
									p += abc_add_noteon(h, ch, p, h->tracktime, barsig, abcbeatvol, abceffect, abceffoper);
									if( abceffoper != 255 ) abceffect = none;
									p += abc_notelen(p, &notelen, &notediv);
									if( *p=='-' ) {
										p++;
										if( h->tp->tail && h->tp->tail->flg != 1 )
										h->tp->tienote = h->tp->tail;
									}
									tupletr = abc_tuplet(&notelen, &notediv, tupletp, tupletq, tupletr);
									while( isspace(*p) ) p++;	// allow spacing in broken rithm notation
									p += abc_brokenrithm(p, &notelen, &notediv, &brokenrithm, abchornpipe);
									thistime = notelen_notediv_to_ticks(h->speed, notelen*snotelen, notediv*snotediv);
									if( abcfermata ) {
										thistime <<= 1;
										abcfermata = 0;
									}
									if( thistime > abcgrace ) {
										thistime -= abcgrace;
										abcgrace = 0;
									}
									else {
										abcgrace -= thistime;
										thistime = abcticks(h->speed);
										abcgrace += abcticks(h->speed);
									}
									h->tracktime += thistime;
									if( thistime > abcticks(h->speed) )
										abc_add_noteoff(h, h->tp, h->tracktime - abcnoslurs - (( ch0 == '.')? thistime / 2: 0));
									else
										abc_add_noteoff(h, h->tp, h->tracktime);
									abc_add_sync(h, h->tp, h->tracktime);
									if( h->gchordon && (h->tp == h->tpc) )
										abc_add_gchord(h, h->tracktime, bartime);
									if( h->drumon && (h->tp == h->tpr) )
										abc_add_drum(h, h->tracktime, bartime);
									abcarpeggio = 0;
									break;
								}
								if( strchr("zx",ch) ) {
									h->tp = abc_check_track(h, h->tp);
									abc_track_clear_tiednote(h);
									p += abc_notelen(p, &notelen, &notediv);
									tupletr = abc_tuplet(&notelen, &notediv, tupletp, tupletq, tupletr);
									while( isspace(*p) ) p++;	// allow spacing in broken rithm notation
									p += abc_brokenrithm(p, &notelen, &notediv, &brokenrithm, abchornpipe);
									thistime = notelen_notediv_to_ticks(h->speed, notelen*snotelen, notediv*snotediv);
									if( abcfermata ) {
										thistime <<= 1;
										abcfermata = 0;
									}
									if( thistime > abcgrace ) {
										thistime -= abcgrace;
										abcgrace = 0;
									}
									else {
										abcgrace -= thistime;
										thistime = abcticks(h->speed);
										abcgrace += abcticks(h->speed);
									}
									h->tracktime += thistime;
									abc_add_sync(h, h->tp, h->tracktime);
									if( h->gchordon && (h->tp == h->tpc) )
										abc_add_gchord(h, h->tracktime, bartime);
									if( h->drumon && (h->tp == h->tpr) )
										abc_add_drum(h, h->tracktime, bartime);
									abcarpeggio = 0;
									break;
								}
								if( strchr("Z",ch) ) {
									h->tp = abc_check_track(h, h->tp);
									abc_track_clear_tiednote(h);
									p += abc_notelen(p, &notelen, &notediv);
									thistime = notelen_notediv_to_ticks(h->speed, notelen*mnotelen, notediv*mnotediv);
									if( abcfermata ) {
										thistime <<= 1;
										abcfermata = 0;
									}
									if( thistime > abcgrace ) {
										thistime -= abcgrace;
										abcgrace = 0;
									}
									else {
										abcgrace -= thistime;
										thistime = abcticks(h->speed);
										abcgrace += abcticks(h->speed);
									}
									h->tracktime += thistime;
									sprintf(barsig, "%s%s", sig[abckey], sig[abckey]);	// reset the key signature
									abc_add_sync(h, h->tp, h->tracktime);
									if( h->gchordon && (h->tp == h->tpc) )
										abc_add_gchord(h, h->tracktime, bartime);
									if( h->drumon && (h->tp == h->tpr) )
										abc_add_drum(h, h->tracktime, bartime);
									abcarpeggio = 0;
									break;
								}
								if( isalpha(ch) && *p==':' ) {
									// some unprocessed field line?
									while( *p ) p++;	// skip it
									break;
								}
								break;
						}
						ch0 = ch;	// remember previous char, can be staccato dot...
						if( pp ) {	// did we have a U: macro substitution?
							if( !*p ) {
								p = pp;
								pp = 0;
							}
						}
					}
				}
			}
		}
		if( mmsp ) mmfclose(mmstack[mmsp]);
	}
	ABC_CleanupMacros(h);	// we dont need them anymore
	if( !h->track ) {
		char buf[11];
		sprintf(buf,"%u", abcxnumber);
		abc_message("abc X:%s has no body", buf);
		h->track = abc_check_track(h, h->track); // for sanity...
	}
	if( abcstate == INBODY ) {
		// last but not least shut off all pending events
		abc_recalculate_tracktime(h);
		for( ttp=h->track; ttp; ttp=ttp->next )
			abc_add_noteoff(h,ttp,h->tracktime);
		abc_add_partbreak(h, h->track, h->tracktime);
		t = abc_patno(h, h->tracktime);
		if( abc_pattracktime(h, h->tracktime) % abcticks(64 * h->speed) ) t++;
		if(	global_part == ' ' ) {
			partpat[26][1] = t;
			if( abcparts ) {
				for( t=0; t<26; t++ )
					if( partpat[t][0] < partpat[t][1] ) break;
				if( t == 26 ) {
					abc_message("parts (%s) set but not used", abcparts);
					abc_set_parts(&abcparts, 0);	// forget the parts array
				}
			}
		}
		else
			partpat[global_part - 'A'][1] = t;
		if( !abcparts ) abc_song_to_parts(h, &abcparts, partpat);
		orderlen = abc_partpat_to_orderlist(partpat, abcparts, h, &orderlist, orderlen);
	}
	abc_synchronise_tracks(h);	// distribute all control events
	abc_recalculate_tracktime(h);
/*
	abctrack:
		tracktick		long
		note				byte
		octave			byte
		instrument	byte
		effects			byte

	tick = tracktick modulo speed
	row  = (tracktick div speed) modulo 64
	pat  = (tracktick div speed) div 64
	ord  = calculated
*/
	if ((p=getenv(ABC_ENV_DUMPTRACKS)) != NULL) {
		printf("P:%s\n",abcparts);
		for( t=0; t<26; t++ )
			if( partpat[t][1] >= partpat[t][0] )
				printf("  %c ",t+'A');
		if( partpat[26][1] >= partpat[26][0] )
			printf("All");
		printf("\n");
		for( t=0; t<27; t++ )
			if( partpat[t][1] >= partpat[t][0] )
				printf("%3d ",partpat[t][0]);
		printf("\n");
		for( t=0; t<27; t++ )
			if( partpat[t][1] >= partpat[t][0] )
				printf("%3d ",partpat[t][1]);
		printf("\n");
		for( t=0; (int)t<orderlen; t++ )
			printf("%3d ",t);
		printf("\n");
		for( t=0; (int)t<orderlen; t++ )
			printf("%3d ",orderlist[t]);
		printf("\n");
		abc_dumptracks(h,p);
	}
	// set module variables
	if( abctempo == 0 ) abctempo = abcrate;
	if( m_nDefaultTempo == 0 ) m_nDefaultTempo = abctempo;
	m_nType         = MOD_TYPE_ABC;
	numpat          = 1+(modticks(h->tracktime) / h->speed / 64);
	if( numpat > MAX_PATTERNS )
		numpat = MAX_PATTERNS;
	m_nDefaultSpeed = h->speed;
	m_nChannels     = abc_numtracks(h);
	m_dwSongFlags   = SONG_LINEARSLIDES;
	m_nMinPeriod    = 28 << 2;
	m_nMaxPeriod    = 1712 << 3;
	// orderlist
	for(t=0; t < (uint32_t)orderlen; t++){
		if( t >= MAX_ORDERS )
			break;
		Order[t] = orderlist[t];
	}
	free(orderlist);	// get rid of orderlist memory
	// ==============================
	// Load the pattern info now!
	if( ABC_ReadPatterns(Patterns, PatternSize, h, numpat, m_nChannels) ) {
		// :^(  need one more channel to handle the global events ;^b
		m_nChannels++;
		h->tp = abc_locate_track(h, "", 99);
		abc_add_sync(h, h->tp, h->tracktime);
		for( t=0; t<numpat; t++ ) {
			FreePattern(Patterns[t]);
			Patterns[t] = NULL;
		}
		ABC_ReadPatterns(Patterns, PatternSize, h, numpat, m_nChannels);
	}
	// load instruments after building the patterns (chan == 10 track handling)
	if( !PAT_Load_Instruments(this) ) {
		avoid_reentry = 0;
		return FALSE;
	}
	// ============================================================
	// set panning positions
	if( m_nChannels > MAX_BASECHANNELS )
		m_nChannels = MAX_BASECHANNELS;
	for(t=0; t<m_nChannels; t++) {
		ChnSettings[t].nPan = 0x30+((t+2)%5)*((0xD0 - 0x30)/5);     // 0x30 = std s3m val
		ChnSettings[t].nVolume = 64;
	}
	avoid_reentry = 0; // it is safe now, I'm finished
	abc_set_parts(&abcparts, 0);	// free the parts array
	ABC_Cleanup(h);	// we dont need it anymore
	return 1;
}
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

///////////////////////////////////////////////////
//
// AMF module loader
//
// There is 2 types of AMF files:
// - ASYLUM Music Format
// - Advanced Music Format(DSM)
//
///////////////////////////////////////////////////
//#include "stdafx.h"
//#include "sndfile.h"

//#define AMFLOG

//#pragma warning(disable:4244)

#pragma pack(1)

typedef struct _AMFFILEHEADER
{
	UCHAR szAMF[3];
	UCHAR version;
	CHAR title[32];
	UCHAR numsamples;
	UCHAR numorders;
	USHORT numtracks;
	UCHAR numchannels;
} AMFFILEHEADER;

typedef struct _AMFSAMPLE
{
	UCHAR type;
	CHAR  samplename[32];
	CHAR  filename[13];
	ULONG offset;
	ULONG length;
	USHORT c2spd;
	UCHAR volume;
} AMFSAMPLE;


#pragma pack()


#ifdef AMFLOG
extern void Log(LPCSTR, ...);
#endif

VOID AMF_Unpack(MODCOMMAND *pPat, const BYTE *pTrack, UINT nRows, UINT nChannels)
//-------------------------------------------------------------------------------
{
	UINT lastinstr = 0;
	UINT nTrkSize = bswapLE16(*(USHORT *)pTrack);
	nTrkSize += (UINT)pTrack[2] << 16;
	pTrack += 3;
	while (nTrkSize--)
	{
		UINT row = pTrack[0];
		UINT cmd = pTrack[1];
		UINT arg = pTrack[2];
		if (row >= nRows) break;
		MODCOMMAND *m = pPat + row * nChannels;
		if (cmd < 0x7F) // note+vol
		{
			m->note = cmd+1;
			if (!m->instr) m->instr = lastinstr;
			m->volcmd = VOLCMD_VOLUME;
			m->vol = arg;
		} else
		if (cmd == 0x7F) // duplicate row
		{
			signed char rdelta = (signed char)arg;
			int rowsrc = (int)row + (int)rdelta;
			if ((rowsrc >= 0) && (rowsrc < (int)nRows)) memcpy(m, &pPat[rowsrc*nChannels],sizeof(pPat[rowsrc*nChannels]));
		} else
		if (cmd == 0x80) // instrument
		{
			m->instr = arg+1;
			lastinstr = m->instr;
		} else
		if (cmd == 0x83) // volume
		{
			m->volcmd = VOLCMD_VOLUME;
			m->vol = arg;
		} else
		// effect
		{
			UINT command = cmd & 0x7F;
			UINT param = arg;
			switch(command)
			{
			// 0x01: Set Speed
			case 0x01:	command = CMD_SPEED; break;
			// 0x02: Volume Slide
			// 0x0A: Tone Porta + Vol Slide
			// 0x0B: Vibrato + Vol Slide
			case 0x02:	command = CMD_VOLUMESLIDE;
			case 0x0A:	if (command == 0x0A) command = CMD_TONEPORTAVOL;
			case 0x0B:	if (command == 0x0B) command = CMD_VIBRATOVOL;
						if (param & 0x80) param = (-(signed char)param)&0x0F;
						else param = (param&0x0F)<<4;
						break;
			// 0x04: Porta Up/Down
			case 0x04:	if (param & 0x80) { command = CMD_PORTAMENTOUP; param = (-(signed char)param)&0x7F; }
						else { command = CMD_PORTAMENTODOWN; } break;
			// 0x06: Tone Portamento
			case 0x06:	command = CMD_TONEPORTAMENTO; break;
			// 0x07: Tremor
			case 0x07:	command = CMD_TREMOR; break;
			// 0x08: Arpeggio
			case 0x08:	command = CMD_ARPEGGIO; break;
			// 0x09: Vibrato
			case 0x09:	command = CMD_VIBRATO; break;
			// 0x0C: Pattern Break
			case 0x0C:	command = CMD_PATTERNBREAK; break;
			// 0x0D: Position Jump
			case 0x0D:	command = CMD_POSITIONJUMP; break;
			// 0x0F: Retrig
			case 0x0F:	command = CMD_RETRIG; break;
			// 0x10: Offset
			case 0x10:	command = CMD_OFFSET; break;
			// 0x11: Fine Volume Slide
			case 0x11:	if (param) { command = CMD_VOLUMESLIDE;
							if (param & 0x80) param = 0xF0|((-(signed char)param)&0x0F);
							else param = 0x0F|((param&0x0F)<<4);
						} else command = 0; break;
			// 0x12: Fine Portamento
			// 0x16: Extra Fine Portamento
			case 0x12:
			case 0x16:	if (param) { int mask = (command == 0x16) ? 0xE0 : 0xF0;
							command = (param & 0x80) ? CMD_PORTAMENTOUP : CMD_PORTAMENTODOWN;
							if (param & 0x80) param = mask|((-(signed char)param)&0x0F);
							else param |= mask;
						} else command = 0; break;
			// 0x13: Note Delay
			case 0x13:	command = CMD_S3MCMDEX; param = 0xD0|(param & 0x0F); break;
			// 0x14: Note Cut
			case 0x14:	command = CMD_S3MCMDEX; param = 0xC0|(param & 0x0F); break;
			// 0x15: Set Tempo
			case 0x15:	command = CMD_TEMPO; break;
			// 0x17: Panning
			case 0x17:	param = (param+64)&0x7F;
						if (m->command) { if (!m->volcmd) { m->volcmd = VOLCMD_PANNING;  m->vol = param/2; } command = 0; }
						else { command = CMD_PANNING8; }
				break;
			// Unknown effects
			default:	command = param = 0;
			}
			if (command)
			{
				m->command = command;
				m->param = param;
			}
		}
		pTrack += 3;
	}
}



BOOL CSoundFile::ReadAMF(LPCBYTE lpStream, const DWORD dwMemLength)
//-----------------------------------------------------------
{
	const AMFFILEHEADER *pfh = (AMFFILEHEADER *)lpStream;
	DWORD dwMemPos;

	if ((!lpStream) || (dwMemLength < 2048)) return FALSE;
	if ((!strncmp((LPCTSTR)lpStream, "ASYLUM Music Format V1.0", 25)) && (dwMemLength > 4096))
	{
		UINT numorders, numpats, numsamples;

		dwMemPos = 32;
		numpats = lpStream[dwMemPos+3];
		numorders = lpStream[dwMemPos+4];
		numsamples = 64;
		dwMemPos += 6;
		if ((!numpats) || (numpats > MAX_PATTERNS) || (!numorders)
		 || (numpats*64*32 + 294 + 37*64 >= dwMemLength)) return FALSE;
		m_nType = MOD_TYPE_AMF0;
		m_nChannels = 8;
		m_nInstruments = 0;
		m_nSamples = 31;
		m_nDefaultTempo = 125;
		m_nDefaultSpeed = 6;
		for (UINT iOrd=0; iOrd<MAX_ORDERS; iOrd++)
		{
			Order[iOrd] = (iOrd < numorders) ? lpStream[dwMemPos+iOrd] : 0xFF;
		}
		dwMemPos = 294; // ???
		for (UINT iSmp=0; iSmp<numsamples; iSmp++)
		{
			MODINSTRUMENT *psmp = &Ins[iSmp+1];
			memcpy(m_szNames[iSmp+1], lpStream+dwMemPos, 22);
			m_szNames[iSmp+1][21] = '\0';
			psmp->nFineTune = MOD2XMFineTune(lpStream[dwMemPos+22]);
			psmp->nVolume = lpStream[dwMemPos+23];
			psmp->nGlobalVol = 64;
			if (psmp->nVolume > 0x40) psmp->nVolume = 0x40;
			psmp->nVolume <<= 2;
			psmp->nLength = bswapLE32(*((LPDWORD)(lpStream+dwMemPos+25)));
			psmp->nLoopStart = bswapLE32(*((LPDWORD)(lpStream+dwMemPos+29)));
			psmp->nLoopEnd = psmp->nLoopStart + bswapLE32(*((LPDWORD)(lpStream+dwMemPos+33)));
			if ((psmp->nLoopEnd > psmp->nLoopStart) && (psmp->nLoopEnd <= psmp->nLength))
			{
				psmp->uFlags = CHN_LOOP;
			} else
			{
				psmp->nLoopStart = psmp->nLoopEnd = 0;
			}
			if ((psmp->nLength) && (iSmp>31)) m_nSamples = iSmp+1;
			dwMemPos += 37;
		}
		for (UINT iPat=0; iPat<numpats; iPat++)
		{
			MODCOMMAND *p = AllocatePattern(64, m_nChannels);
			if (!p) break;
			Patterns[iPat] = p;
			PatternSize[iPat] = 64;
			const UCHAR *pin = lpStream + dwMemPos;
			for (UINT i=0; i<8*64; i++)
			{
				p->note = 0;

				if (pin[0])
				{
					p->note = pin[0] + 13;
				}
				p->instr = pin[1];
				p->command = pin[2];
				p->param = pin[3];
				if (p->command > 0x0F)
				{
				#ifdef AMFLOG
					Log("0x%02X.0x%02X ?", p->command, p->param);
				#endif
					p->command = 0;
				}
				ConvertModCommand(p);
				pin += 4;
				p++;
			}
			dwMemPos += 64*32;
		}
		// Read samples
		for (UINT iData=0; iData<m_nSamples; iData++)
		{
			MODINSTRUMENT *psmp = &Ins[iData+1];
			if (psmp->nLength)
			{
				if (dwMemPos > dwMemLength) return FALSE;
				dwMemPos += ReadSample(psmp, RS_PCM8S, (LPCSTR)(lpStream+dwMemPos), dwMemLength-dwMemPos);
			}
		}
		return TRUE;
	}
	////////////////////////////
	// DSM/AMF
	USHORT *ptracks[MAX_PATTERNS];
	DWORD sampleseekpos[MAX_SAMPLES];

	if ((pfh->szAMF[0] != 'A') || (pfh->szAMF[1] != 'M') || (pfh->szAMF[2] != 'F')
	 || (pfh->version < 10) || (pfh->version > 14) || (!bswapLE16(pfh->numtracks))
	 || (!pfh->numorders) || (pfh->numorders > MAX_PATTERNS)
	 || (!pfh->numsamples) || (pfh->numsamples >= MAX_SAMPLES)
	 || (pfh->numchannels < 4) || (pfh->numchannels > 32))
		return FALSE;
	memcpy(m_szNames[0], pfh->title, 32);
	m_szNames[0][31] = '\0';
	dwMemPos = sizeof(AMFFILEHEADER);
	m_nType = MOD_TYPE_AMF;
	m_nChannels = pfh->numchannels;
	m_nSamples = pfh->numsamples;
	m_nInstruments = 0;
	// Setup Channel Pan Positions
	if (pfh->version >= 11)
	{
		signed char *panpos = (signed char *)(lpStream + dwMemPos);
		UINT nchannels = (pfh->version >= 13) ? 32 : 16;
		for (UINT i=0; i<nchannels; i++)
		{
			int pan = (panpos[i] + 64) * 2;
			if (pan < 0) pan = 0;
			if (pan > 256) { pan = 128; ChnSettings[i].dwFlags |= CHN_SURROUND; }
			ChnSettings[i].nPan = pan;
		}
		dwMemPos += nchannels;
	} else
	{
		for (UINT i=0; i<16; i++)
		{
			ChnSettings[i].nPan = (lpStream[dwMemPos+i] & 1) ? 0x30 : 0xD0;
		}
		dwMemPos += 16;
	}
	// Get Tempo/Speed
	m_nDefaultTempo = 125;
	m_nDefaultSpeed = 6;
	if (pfh->version >= 13)
	{
		if (lpStream[dwMemPos] >= 32) m_nDefaultTempo = lpStream[dwMemPos];
		if (lpStream[dwMemPos+1] <= 32) m_nDefaultSpeed = lpStream[dwMemPos+1];
		dwMemPos += 2;
	}
	// Setup sequence list
	for (UINT iOrd=0; iOrd<MAX_ORDERS; iOrd++)
	{
		if (dwMemPos + 4 > dwMemLength) return TRUE;
		Order[iOrd] = 0xFF;
		if (iOrd < pfh->numorders)
		{
			Order[iOrd] = iOrd;
			PatternSize[iOrd] = 64;
			if (pfh->version >= 14)
			{
				if (dwMemPos + m_nChannels * sizeof(USHORT) + 2 > dwMemLength) return FALSE;
				PatternSize[iOrd] = bswapLE16(*(USHORT *)(lpStream+dwMemPos));
				dwMemPos += 2;
			} else
			{
				if (dwMemPos + m_nChannels * sizeof(USHORT) > dwMemLength) return FALSE;
			}
			ptracks[iOrd] = (USHORT *)(lpStream+dwMemPos);
			dwMemPos += m_nChannels * sizeof(USHORT);
		}
	}
	if (dwMemPos + m_nSamples * (sizeof(AMFSAMPLE)+8) > dwMemLength) return TRUE;
	// Read Samples
	UINT maxsampleseekpos = 0;
	for (UINT iIns=0; iIns<m_nSamples; iIns++)
	{
		MODINSTRUMENT *pins = &Ins[iIns+1];
		const AMFSAMPLE *psh = (AMFSAMPLE *)(lpStream + dwMemPos);

		dwMemPos += sizeof(AMFSAMPLE);
		memcpy(m_szNames[iIns+1], psh->samplename, 32);
		m_szNames[iIns+1][31] = '\0';
		memcpy(pins->name, psh->filename, 13);
		pins->name[12] = '\0';
		pins->nLength = bswapLE32(psh->length);
		pins->nC4Speed = bswapLE16(psh->c2spd);
		pins->nGlobalVol = 64;
		pins->nVolume = psh->volume * 4;
		if (pfh->version >= 11)
		{
			pins->nLoopStart = bswapLE32(*(DWORD *)(lpStream+dwMemPos));
			pins->nLoopEnd = bswapLE32(*(DWORD *)(lpStream+dwMemPos+4));
			dwMemPos += 8;
		} else
		{
			pins->nLoopStart = bswapLE16(*(WORD *)(lpStream+dwMemPos));
			pins->nLoopEnd = pins->nLength;
			dwMemPos += 2;
		}
		sampleseekpos[iIns] = 0;
		if ((psh->type) && (bswapLE32(psh->offset) < dwMemLength-1))
		{
			sampleseekpos[iIns] = bswapLE32(psh->offset);
			if (bswapLE32(psh->offset) > maxsampleseekpos)
				maxsampleseekpos = bswapLE32(psh->offset);
			if ((pins->nLoopEnd > pins->nLoopStart + 2)
			 && (pins->nLoopEnd <= pins->nLength)) pins->uFlags |= CHN_LOOP;
		}
	}
	// Read Track Mapping Table
	USHORT *pTrackMap = (USHORT *)(lpStream+dwMemPos);
	UINT realtrackcnt = 0;
	dwMemPos += pfh->numtracks * sizeof(USHORT);
	if (dwMemPos >= dwMemLength)
		return TRUE;

	for (UINT iTrkMap=0; iTrkMap<pfh->numtracks; iTrkMap++)
	{
		if (realtrackcnt < pTrackMap[iTrkMap]) realtrackcnt = pTrackMap[iTrkMap];
	}
	// Store tracks positions
	BYTE **pTrackData = new BYTE *[realtrackcnt];
	memset(pTrackData, 0, sizeof(BYTE *) * realtrackcnt);
	for (UINT iTrack=0; iTrack<realtrackcnt; iTrack++) if (dwMemPos <= dwMemLength - 3)
	{
		UINT nTrkSize = bswapLE16(*(USHORT *)(lpStream+dwMemPos));
		nTrkSize += (UINT)lpStream[dwMemPos+2] << 16;
		if (dwMemPos + nTrkSize * 3 + 3 <= dwMemLength)
		{
			pTrackData[iTrack] = (BYTE *)(lpStream + dwMemPos);
		}
		dwMemPos += nTrkSize * 3 + 3;
	}
	// Create the patterns from the list of tracks
	for (UINT iPat=0; iPat<pfh->numorders; iPat++)
	{
		MODCOMMAND *p = AllocatePattern(PatternSize[iPat], m_nChannels);
		if (!p) break;
		Patterns[iPat] = p;
		for (UINT iChn=0; iChn<m_nChannels; iChn++)
		{
			UINT nTrack = bswapLE16(ptracks[iPat][iChn]);
			if ((nTrack) && (nTrack <= pfh->numtracks))
			{
				UINT realtrk = bswapLE16(pTrackMap[nTrack-1]);
				if (realtrk)
				{
					realtrk--;
					if ((realtrk < realtrackcnt) && (pTrackData[realtrk]))
					{
						AMF_Unpack(p+iChn, pTrackData[realtrk], PatternSize[iPat], m_nChannels);
					}
				}
			}
		}
	}
	delete[] pTrackData;
	// Read Sample Data
	for (UINT iSeek=1; iSeek<=maxsampleseekpos; iSeek++)
	{
		if (dwMemPos >= dwMemLength) break;
		for (UINT iSmp=0; iSmp<m_nSamples; iSmp++) if (iSeek == sampleseekpos[iSmp])
		{
			MODINSTRUMENT *pins = &Ins[iSmp+1];
			dwMemPos += ReadSample(pins, RS_PCM8U, (LPCSTR)(lpStream+dwMemPos), dwMemLength-dwMemPos);
			break;
		}
	}
	return TRUE;
}
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

//////////////////////////////////////////////
// AMS module loader                        //
//////////////////////////////////////////////
//#include "stdafx.h"
//#include "sndfile.h"

//#pragma warning(disable:4244)

#pragma pack(1)

typedef struct AMSFILEHEADER
{
	char szHeader[7];	// "Extreme"   // changed from CHAR
	BYTE verlo, verhi;	// 0x??,0x01
	BYTE chncfg;
	BYTE samples;
	WORD patterns;
	WORD orders;
	BYTE vmidi;
	WORD extra;
} AMSFILEHEADER;

typedef struct AMSSAMPLEHEADER
{
	DWORD length;
	DWORD loopstart;
	DWORD loopend;
	BYTE finetune_and_pan;
	WORD samplerate;	// C-2 = 8363
	BYTE volume;		// 0-127
	BYTE infobyte;
} AMSSAMPLEHEADER;

#pragma pack()


static BOOL AMSUnpackCheck(const BYTE *lpStream, DWORD dwMemLength, MODINSTRUMENT *ins);

BOOL CSoundFile::ReadAMS(LPCBYTE lpStream, DWORD dwMemLength)
//-----------------------------------------------------------
{
//	BYTE pkinf[MAX_SAMPLES];
	const AMSFILEHEADER *pfh = (AMSFILEHEADER *)lpStream;
	DWORD dwMemPos;
	UINT tmp, tmp2;

	if ((!lpStream) || (dwMemLength < 1024)) return FALSE;
	if ((pfh->verhi != 0x01) || (strncmp(pfh->szHeader, "Extreme", 7))
	 || (!pfh->patterns) || (!pfh->orders) || (!pfh->samples) || (pfh->samples >= MAX_SAMPLES)
	 || (pfh->patterns > MAX_PATTERNS) || (pfh->orders > MAX_ORDERS))
	{
		return ReadAMS2(lpStream, dwMemLength);
	}
	dwMemPos = sizeof(AMSFILEHEADER) + pfh->extra;
	if (dwMemPos + pfh->samples * sizeof(AMSSAMPLEHEADER) + 256 >= dwMemLength) return FALSE;
	m_nType = MOD_TYPE_AMS;
	m_nInstruments = 0;
	m_nChannels = (pfh->chncfg & 0x1F) + 1;
	m_nSamples = pfh->samples;
	for (UINT nSmp=1; nSmp<=m_nSamples; nSmp++, dwMemPos += sizeof(AMSSAMPLEHEADER))
	{
		const AMSSAMPLEHEADER *psh = (AMSSAMPLEHEADER *)(lpStream + dwMemPos);
		MODINSTRUMENT *pins = &Ins[nSmp];
		pins->nLength = psh->length;
		pins->nLoopStart = psh->loopstart;
		pins->nLoopEnd = psh->loopend;
		pins->nGlobalVol = 64;
		pins->nVolume = psh->volume << 1;
		pins->nC4Speed = psh->samplerate;
		pins->nPan = (psh->finetune_and_pan & 0xF0);
		if (pins->nPan < 0x80) pins->nPan += 0x10;
		pins->nFineTune = MOD2XMFineTune(psh->finetune_and_pan & 0x0F);
		pins->uFlags = (psh->infobyte & 0x80) ? CHN_16BIT : 0;
		if ((pins->nLoopEnd <= pins->nLength) && (pins->nLoopStart+4 <= pins->nLoopEnd)) pins->uFlags |= CHN_LOOP;
	//	pkinf[nSmp] = psh->infobyte;
	}
	// Read Song Name
	tmp = lpStream[dwMemPos++];
	if (dwMemPos + tmp + 1 >= dwMemLength) return TRUE;
	tmp2 = (tmp < 32) ? tmp : 31;
	if (tmp2) memcpy(m_szNames[0], lpStream+dwMemPos, tmp2);
	m_szNames[0][tmp2] = 0;
	dwMemPos += tmp;
	// Read sample names
	for (UINT sNam=1; sNam<=m_nSamples; sNam++)
	{
		if (dwMemPos + 32 >= dwMemLength) return TRUE;
		tmp = lpStream[dwMemPos++];
		tmp2 = (tmp < 32) ? tmp : 31;
		if (tmp2) memcpy(m_szNames[sNam], lpStream+dwMemPos, tmp2);
		dwMemPos += tmp;
	}
	// Skip Channel names
	for (UINT cNam=0; cNam<m_nChannels; cNam++)
	{
		if (dwMemPos + 32 >= dwMemLength) return TRUE;
		tmp = lpStream[dwMemPos++];
		dwMemPos += tmp;
	}
	// Read Pattern Names
	m_lpszPatternNames = new char[pfh->patterns * 32];  // changed from CHAR
	if (!m_lpszPatternNames) return TRUE;
	m_nPatternNames = pfh->patterns;
	memset(m_lpszPatternNames, 0, m_nPatternNames * 32);
	for (UINT pNam=0; pNam < m_nPatternNames; pNam++)
	{
		if (dwMemPos + 32 >= dwMemLength) return TRUE;
		tmp = lpStream[dwMemPos++];
		tmp2 = (tmp < 32) ? tmp : 31;
		if (tmp2) memcpy(m_lpszPatternNames+pNam*32, lpStream+dwMemPos, tmp2);
		dwMemPos += tmp;
	}
	// Read Song Comments
	if (dwMemPos + 2 > dwMemLength) return TRUE;
	tmp = *((WORD *)(lpStream+dwMemPos));
	dwMemPos += 2;
	if (tmp >= dwMemLength || dwMemPos > dwMemLength - tmp) return TRUE;
	if (tmp)
	{
		m_lpszSongComments = new char[tmp+1];  // changed from CHAR
		if (!m_lpszSongComments) return TRUE;
		memset(m_lpszSongComments, 0, tmp+1);
		memcpy(m_lpszSongComments, lpStream + dwMemPos, tmp);
		dwMemPos += tmp;
	}
	// Read Order List
	if (2*pfh->orders >= dwMemLength || dwMemPos > dwMemLength - 2*pfh->orders) return TRUE;
	for (UINT iOrd=0; iOrd<pfh->orders; iOrd++, dwMemPos += 2)
	{
		UINT n = *((WORD *)(lpStream+dwMemPos));
		Order[iOrd] = (BYTE)n;
	}
	// Read Patterns
	for (UINT iPat=0; iPat<pfh->patterns; iPat++)
	{
		if (dwMemPos + 4 >= dwMemLength) return TRUE;
		UINT len = *((DWORD *)(lpStream + dwMemPos));
		dwMemPos += 4;
		if ((len >= dwMemLength) || (dwMemPos > dwMemLength - len)) return TRUE;
		PatternSize[iPat] = 64;
		MODCOMMAND *m = AllocatePattern(PatternSize[iPat], m_nChannels);
		if (!m) return TRUE;
		Patterns[iPat] = m;
		const BYTE *p = lpStream + dwMemPos;
		UINT row = 0, i = 0;
		while ((row < PatternSize[iPat]) && (i+2 < len))
		{
			BYTE b0 = p[i++];
			BYTE b1 = p[i++];
			BYTE b2 = 0;
			UINT ch = b0 & 0x3F;
			// Note+Instr
			if (!(b0 & 0x40))
			{
				if (i+1 > len) break;
				b2 = p[i++];
				if (ch < m_nChannels)
				{
					if (b1 & 0x7F) m[ch].note = (b1 & 0x7F) + 25;
					m[ch].instr = b2;
				}
				if (b1 & 0x80)
				{
					if (i+1 > len) break;
					b0 |= 0x40;
					b1 = p[i++];
				}
			}
			// Effect
			if (b0 & 0x40)
			{
			anothercommand:
				if (b1 & 0x40)
				{
					if (ch < m_nChannels)
					{
						m[ch].volcmd = VOLCMD_VOLUME;
						m[ch].vol = b1 & 0x3F;
					}
				} else
				{
					if (i+1 > len) break;
					b2 = p[i++];
					if (ch < m_nChannels)
					{
						UINT cmd = b1 & 0x3F;
						if (cmd == 0x0C)
						{
							m[ch].volcmd = VOLCMD_VOLUME;
							m[ch].vol = b2 >> 1;
						} else
						if (cmd == 0x0E)
						{
							if (!m[ch].command)
							{
								UINT command = CMD_S3MCMDEX;
								UINT param = b2;
								switch(param & 0xF0)
								{
								case 0x00:	if (param & 0x08) { param &= 0x07; param |= 0x90; } else {command=param=0;} break;
								case 0x10:	command = CMD_PORTAMENTOUP; param |= 0xF0; break;
								case 0x20:	command = CMD_PORTAMENTODOWN; param |= 0xF0; break;
								case 0x30:	param = (param & 0x0F) | 0x10; break;
								case 0x40:	param = (param & 0x0F) | 0x30; break;
								case 0x50:	param = (param & 0x0F) | 0x20; break;
								case 0x60:	param = (param & 0x0F) | 0xB0; break;
								case 0x70:	param = (param & 0x0F) | 0x40; break;
								case 0x90:	command = CMD_RETRIG; param &= 0x0F; break;
								case 0xA0:	if (param & 0x0F) { command = CMD_VOLUMESLIDE; param = (param << 4) | 0x0F; } else command=param=0; break;
								case 0xB0:	if (param & 0x0F) { command = CMD_VOLUMESLIDE; param |= 0xF0; } else command=param=0; break;
								}
								m[ch].command = command;
								m[ch].param = param;
							}
						} else
						{
							m[ch].command = cmd;
							m[ch].param = b2;
							ConvertModCommand(&m[ch]);
						}
					}
				}
				if (b1 & 0x80)
				{
					if (i+1 > len) break;
					b1 = p[i++];
					if (i <= len) goto anothercommand;
				}
			}
			if (b0 & 0x80)
			{
				row++;
				m += m_nChannels;
			}
		}
		dwMemPos += len;
	}
	// Read Samples
	for (UINT iSmp=1; iSmp<=m_nSamples; iSmp++) if (Ins[iSmp].nLength)
	{
		if (dwMemPos >= dwMemLength - 9) return TRUE;
		UINT flags = (Ins[iSmp].uFlags & CHN_16BIT) ? RS_AMS16 : RS_AMS8;
		if (!AMSUnpackCheck(lpStream+dwMemPos, dwMemLength-dwMemPos, &Ins[iSmp])) break;
		dwMemPos += ReadSample(&Ins[iSmp], flags, (LPCSTR)(lpStream+dwMemPos), dwMemLength-dwMemPos);
	}
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// AMS 2.2 loader

#pragma pack(1)

typedef struct AMS2FILEHEADER
{
	DWORD dwHdr1;		// AMShdr
	WORD wHdr2;
	BYTE b1A;			// 0x1A
	BYTE titlelen;		// 30-bytes max
	CHAR szTitle[30];	// [titlelen]
} AMS2FILEHEADER;

typedef struct AMS2SONGHEADER
{
	WORD version;
	BYTE instruments;
	WORD patterns;
	WORD orders;
	WORD bpm;
	BYTE speed;
	BYTE channels;
	BYTE commands;
	BYTE rows;
	WORD flags;
} AMS2SONGHEADER;

typedef struct AMS2INSTRUMENT
{
	BYTE samples;
	BYTE notemap[NOTE_MAX];
} AMS2INSTRUMENT;

typedef struct AMS2ENVELOPE
{
	BYTE speed;
	BYTE sustain;
	BYTE loopbegin;
	BYTE loopend;
	BYTE points;
	BYTE info[3];
} AMS2ENVELOPE;

typedef struct AMS2SAMPLE
{
	DWORD length;
	DWORD loopstart;
	DWORD loopend;
	WORD frequency;
	BYTE finetune;
	WORD c4speed;
	CHAR transpose;
	BYTE volume;
	BYTE flags;
} AMS2SAMPLE;


#pragma pack()



BOOL CSoundFile::ReadAMS2(LPCBYTE lpStream, DWORD dwMemLength)
//------------------------------------------------------------
{
	const AMS2FILEHEADER *pfh = (AMS2FILEHEADER *)lpStream;
	const AMS2SONGHEADER *psh;
	DWORD dwMemPos;
	BYTE smpmap[16];
	BYTE packedsamples[MAX_SAMPLES];

	if ((pfh->dwHdr1 != 0x68534D41) || (pfh->wHdr2 != 0x7264)
	 || (pfh->b1A != 0x1A) || (pfh->titlelen > 30)) return FALSE;
	dwMemPos = pfh->titlelen + 8;
	psh = (AMS2SONGHEADER *)(lpStream + dwMemPos);
	if (((psh->version & 0xFF00) != 0x0200) || (!psh->instruments)
	 || (psh->instruments >= MAX_INSTRUMENTS) || (!psh->patterns) || (!psh->orders)) return FALSE;
	dwMemPos += sizeof(AMS2SONGHEADER);
	if (pfh->titlelen)
	{
		memcpy(m_szNames, pfh->szTitle, pfh->titlelen);
		m_szNames[0][pfh->titlelen] = 0;
	}
	m_nType = MOD_TYPE_AMS;
	m_nChannels = 32;
	m_nDefaultTempo = psh->bpm >> 8;
	m_nDefaultSpeed = psh->speed;
	m_nInstruments = psh->instruments;
	m_nSamples = 0;
	if (psh->flags & 0x40) m_dwSongFlags |= SONG_LINEARSLIDES;
	for (UINT nIns=1; nIns<=m_nInstruments; nIns++)
	{
		if (dwMemPos >= dwMemLength) return TRUE;
		UINT insnamelen = lpStream[dwMemPos];
		const CHAR *pinsname = (CHAR *)(lpStream+dwMemPos+1);
		dwMemPos += insnamelen + 1;
		const AMS2INSTRUMENT *pins = (AMS2INSTRUMENT *)(lpStream + dwMemPos);
		dwMemPos += sizeof(AMS2INSTRUMENT);
		const AMS2ENVELOPE *volenv, *panenv, *pitchenv;
		if (dwMemPos + sizeof(AMS2ENVELOPE) > dwMemLength) return TRUE;
		volenv = (AMS2ENVELOPE *)(lpStream+dwMemPos);
		dwMemPos += 5 + volenv->points*3;
		if (dwMemPos + sizeof(AMS2ENVELOPE) > dwMemLength) return TRUE;
		panenv = (AMS2ENVELOPE *)(lpStream+dwMemPos);
		dwMemPos += 5 + panenv->points*3;
		if (dwMemPos + sizeof(AMS2ENVELOPE) > dwMemLength) return TRUE;
		pitchenv = (AMS2ENVELOPE *)(lpStream+dwMemPos);
		dwMemPos += 5 + pitchenv->points*3;
		if (dwMemPos >= dwMemLength) return TRUE;
		INSTRUMENTHEADER *penv = new INSTRUMENTHEADER;
		if (!penv) return TRUE;
		memset(smpmap, 0, sizeof(smpmap));
		memset(penv, 0, sizeof(INSTRUMENTHEADER));
		for (UINT ismpmap=0; ismpmap<pins->samples; ismpmap++)
		{
			if ((ismpmap >= 16) || (m_nSamples+1 >= MAX_SAMPLES)) break;
			m_nSamples++;
			smpmap[ismpmap] = m_nSamples;
		}
		penv->nGlobalVol = 64;
		penv->nPan = 128;
		penv->nPPC = 60;
		Headers[nIns] = penv;
		if (insnamelen)
		{
			if (insnamelen > 31) insnamelen = 31;
			memcpy(penv->name, pinsname, insnamelen);
			penv->name[insnamelen] = 0;
		}
		for (UINT inotemap=0; inotemap<NOTE_MAX; inotemap++)
		{
			penv->NoteMap[inotemap] = inotemap+1;
			penv->Keyboard[inotemap] = smpmap[pins->notemap[inotemap] & 0x0F];
		}
		// Volume Envelope
		{
			UINT pos = 0;
			penv->nVolEnv = (volenv->points > 16) ? 16 : volenv->points;
			penv->nVolSustainBegin = penv->nVolSustainEnd = volenv->sustain;
			penv->nVolLoopStart = volenv->loopbegin;
			penv->nVolLoopEnd = volenv->loopend;
			for (UINT i=0; i<penv->nVolEnv; i++)
			{
				penv->VolEnv[i] = (BYTE)((volenv->info[i*3+2] & 0x7F) >> 1);
				pos += volenv->info[i*3] + ((volenv->info[i*3+1] & 1) << 8);
				penv->VolPoints[i] = (WORD)pos;
			}
		}
		if (dwMemPos + 5 > dwMemLength) return TRUE;
		penv->nFadeOut = (((lpStream[dwMemPos+2] & 0x0F) << 8) | (lpStream[dwMemPos+1])) << 3;
		UINT envflags = lpStream[dwMemPos+3];
		if (envflags & 0x01) penv->dwFlags |= ENV_VOLLOOP;
		if (envflags & 0x02) penv->dwFlags |= ENV_VOLSUSTAIN;
		if (envflags & 0x04) penv->dwFlags |= ENV_VOLUME;
		dwMemPos += 5;
		// Read Samples
		for (UINT ismp=0; ismp<pins->samples; ismp++)
		{
			if (dwMemPos + 1 > dwMemLength) return TRUE;
			MODINSTRUMENT *psmp = ((ismp < 16) && (smpmap[ismp])) ? &Ins[smpmap[ismp]] : NULL;
			UINT smpnamelen = lpStream[dwMemPos];
			if (dwMemPos + smpnamelen + 1 > dwMemLength) return TRUE;
			if ((psmp) && (smpnamelen) && (smpnamelen <= 22))
			{
				memcpy(m_szNames[smpmap[ismp]], lpStream+dwMemPos+1, smpnamelen);
			}
			dwMemPos += smpnamelen + 1;
			if (dwMemPos + sizeof(AMS2SAMPLE) > dwMemLength) return TRUE;
			if (psmp)
			{
				const AMS2SAMPLE *pams = (AMS2SAMPLE *)(lpStream+dwMemPos);
				psmp->nGlobalVol = 64;
				psmp->nPan = 128;
				psmp->nLength = pams->length;
				psmp->nLoopStart = pams->loopstart;
				psmp->nLoopEnd = pams->loopend;
				psmp->nC4Speed = pams->c4speed;
				psmp->RelativeTone = pams->transpose;
				psmp->nVolume = pams->volume / 2;
				packedsamples[smpmap[ismp]] = pams->flags;
				if (pams->flags & 0x04) psmp->uFlags |= CHN_16BIT;
				if (pams->flags & 0x08) psmp->uFlags |= CHN_LOOP;
				if (pams->flags & 0x10) psmp->uFlags |= CHN_PINGPONGLOOP;
			}
			dwMemPos += sizeof(AMS2SAMPLE);
		}
	}
	if (dwMemPos + 256 >= dwMemLength) return TRUE;
	// Comments
	{
		UINT composernamelen = lpStream[dwMemPos];
		if (composernamelen)
		{
			m_lpszSongComments = new char[composernamelen+1]; // changed from CHAR
			if (m_lpszSongComments)
			{
				memcpy(m_lpszSongComments, lpStream+dwMemPos+1, composernamelen);
				m_lpszSongComments[composernamelen] = 0;
			}
		}
		dwMemPos += composernamelen + 1;
		// channel names
		for (UINT i=0; i<32; i++)
		{
			UINT chnnamlen = lpStream[dwMemPos];
			if ((chnnamlen) && (chnnamlen < MAX_CHANNELNAME))
			{
				memcpy(ChnSettings[i].szName, lpStream+dwMemPos+1, chnnamlen);
			}
			dwMemPos += chnnamlen + 1;
			if (dwMemPos + chnnamlen + 256 >= dwMemLength) return TRUE;
		}
		// packed comments (ignored)
		UINT songtextlen = *((LPDWORD)(lpStream+dwMemPos));
		dwMemPos += songtextlen;
		if (dwMemPos + 256 >= dwMemLength) return TRUE;
	}
	// Order List
	{
		for (UINT i=0; i<MAX_ORDERS; i++)
		{
			Order[i] = 0xFF;
			if (dwMemPos + 2 >= dwMemLength) return TRUE;
			if (i < psh->orders)
			{
				Order[i] = lpStream[dwMemPos];
				dwMemPos += 2;
			}
		}
	}
	// Pattern Data
	for (UINT ipat=0; ipat<psh->patterns; ipat++)
	{
		if (dwMemPos+8 >= dwMemLength) return TRUE;
		UINT packedlen = *((LPDWORD)(lpStream+dwMemPos));
		UINT numrows = 1 + (UINT)(lpStream[dwMemPos+4]);
		//UINT patchn = 1 + (UINT)(lpStream[dwMemPos+5] & 0x1F);
		//UINT patcmds = 1 + (UINT)(lpStream[dwMemPos+5] >> 5);
		UINT patnamlen = lpStream[dwMemPos+6];
		dwMemPos += 4;
		if ((ipat < MAX_PATTERNS) && (packedlen < dwMemLength-dwMemPos) && (numrows >= 8))
		{
			if ((patnamlen) && (patnamlen < MAX_PATTERNNAME))
			{
				char s[MAX_PATTERNNAME]; // changed from CHAR
				memcpy(s, lpStream+dwMemPos+3, patnamlen);
				s[patnamlen] = 0;
				SetPatternName(ipat, s);
			}
			PatternSize[ipat] = numrows;
			Patterns[ipat] = AllocatePattern(numrows, m_nChannels);
			if (!Patterns[ipat]) return TRUE;
			// Unpack Pattern Data
			LPCBYTE psrc = lpStream + dwMemPos;
			UINT pos = 3 + patnamlen;
			UINT row = 0;
			while ((pos < packedlen) && (row < numrows))
			{
				MODCOMMAND *m = Patterns[ipat] + row * m_nChannels;
				UINT byte1 = psrc[pos++];
				UINT ch = byte1 & 0x1F;
				// Read Note + Instr
				if (!(byte1 & 0x40))
				{
					UINT byte2 = psrc[pos++];
					UINT note = byte2 & 0x7F;
					if (note) m[ch].note = (note > 1) ? (note-1) : 0xFF;
					m[ch].instr = psrc[pos++];
					// Read Effect
					while (byte2 & 0x80)
					{
						byte2 = psrc[pos++];
						if (byte2 & 0x40)
						{
							m[ch].volcmd = VOLCMD_VOLUME;
							m[ch].vol = byte2 & 0x3F;
						} else
						{
							UINT command = byte2 & 0x3F;
							UINT param = psrc[pos++];
							if (command == 0x0C)
							{
								m[ch].volcmd = VOLCMD_VOLUME;
								m[ch].vol = param / 2;
							} else
							if (command < 0x10)
							{
								m[ch].command = command;
								m[ch].param = param;
								ConvertModCommand(&m[ch]);
							} else
							{
								// TODO: AMS effects
							}
						}
					}
				}
				if (byte1 & 0x80) row++;
			}
		}
		dwMemPos += packedlen;
	}
	// Read Samples
	for (UINT iSmp=1; iSmp<=m_nSamples; iSmp++) if (Ins[iSmp].nLength)
	{
		if (dwMemPos >= dwMemLength - 9) return TRUE;
		UINT flags;
		if (packedsamples[iSmp] & 0x03)
		{
			flags = (Ins[iSmp].uFlags & CHN_16BIT) ? RS_AMS16 : RS_AMS8;
			if (!AMSUnpackCheck(lpStream+dwMemPos, dwMemLength-dwMemPos, &Ins[iSmp])) break;
		} else
		{
			flags = (Ins[iSmp].uFlags & CHN_16BIT) ? RS_PCM16S : RS_PCM8S;
		}
		dwMemPos += ReadSample(&Ins[iSmp], flags, (LPCSTR)(lpStream+dwMemPos), dwMemLength-dwMemPos);
	}
	return TRUE;
}


// Precheck AMS packed sample size to determine whether or not it could fit the actual size.
static BOOL AMSUnpackCheck(const BYTE *lpStream, DWORD dwMemLength, MODINSTRUMENT *ins)
// -----------------------------------------------------------------------------------
{
	if (dwMemLength < 9) return FALSE;
	DWORD packedbytes = *((DWORD *)(lpStream + 4));

	DWORD samplebytes = ins->nLength;
	if (samplebytes > MAX_SAMPLE_LENGTH) samplebytes = MAX_SAMPLE_LENGTH;
	if (ins->uFlags & CHN_16BIT) samplebytes *= 2;

	// RLE can pack a run of up to 255 bytes into 3 bytes.
	DWORD packedmin = (samplebytes * 3) >> 8;
	if (packedbytes < packedmin)
	{
		samplebytes = packedbytes * (255 / 3) + 2;
		ins->nLength = samplebytes;
		if (ins->uFlags & CHN_16BIT) ins->nLength >>= 1;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////
// AMS Sample unpacking

void AMSUnpack(const char *psrc, UINT inputlen, char *pdest, UINT dmax, char packcharacter)
{
	UINT tmplen = dmax;
	signed char *amstmp = new signed char[tmplen];

	if (!amstmp) return;
	// Unpack Loop
	{
		signed char *p = amstmp;
		UINT i=0, j=0;
		while ((i < inputlen) && (j < tmplen))
		{
			signed char ch = psrc[i++];
			if (ch == packcharacter)
			{
				if (i >= inputlen) break;
				BYTE ch2 = psrc[i++];
				if (ch2)
				{
					if (i >= inputlen) break;
					ch = psrc[i++];
					while (ch2--)
					{
						p[j++] = ch;
						if (j >= tmplen) break;
					}
				} else p[j++] = packcharacter;
			} else p[j++] = ch;
		}
		if (j < tmplen)
		{
			// Truncated or invalid; don't try to unpack this.
			delete[] amstmp;
			return;
		}
	}
	// Bit Unpack Loop
	{
		signed char *p = amstmp;
		UINT bitcount = 0x80, dh;
		UINT k=0;
		for (UINT i=0; i<dmax; i++)
		{
			BYTE al = *p++;
			dh = 0;
			for (UINT count=0; count<8; count++)
			{
				UINT bl = al & bitcount;
				bl = ((bl|(bl<<8)) >> ((dh+8-count) & 7)) & 0xFF;
				bitcount = ((bitcount|(bitcount<<8)) >> 1) & 0xFF;
				pdest[k++] |= bl;
				if (k >= dmax)
				{
					k = 0;
					dh++;
				}
			}
			bitcount = ((bitcount|(bitcount<<8)) >> dh) & 0xFF;
		}
	}
	// Delta Unpack
	{
		signed char old = 0;
		for (UINT i=0; i<dmax; i++)
		{
			int pos = ((LPBYTE)pdest)[i];
			if ((pos != 128) && (pos & 0x80)) pos = -(pos & 0x7F);
			old -= (signed char)pos;
			pdest[i] = old;
		}
	}
	delete[] amstmp;
}

/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (endian and char fixes for PPC)
*/

///////////////////////////////////////////////////////////////
//
// DigiBooster Pro Module Loader (*.dbm)
//
// Note: this loader doesn't handle multiple songs
//
///////////////////////////////////////////////////////////////

//#include "stdafx.h"
//#include "sndfile.h"

//#pragma warning(disable:4244)

#define DBM_FILE_MAGIC	0x304d4244
#define DBM_ID_NAME		0x454d414e
#define DBM_NAMELEN		0x2c000000
#define DBM_ID_INFO		0x4f464e49
#define DBM_INFOLEN		0x0a000000
#define DBM_ID_SONG		0x474e4f53
#define DBM_ID_INST		0x54534e49
#define DBM_ID_VENV		0x564e4556
#define DBM_ID_PATT		0x54544150
#define DBM_ID_SMPL		0x4c504d53

#pragma pack(1)

typedef struct DBMFILEHEADER
{
	DWORD dbm_id;		// "DBM0" = 0x304d4244
	WORD trkver;		// Tracker version: 02.15
	WORD reserved;
	DWORD name_id;		// "NAME" = 0x454d414e
	DWORD name_len;		// name length: always 44
	CHAR songname[44];
	DWORD info_id;		// "INFO" = 0x4f464e49
	DWORD info_len;		// 0x0a000000
	WORD instruments;
	WORD samples;
	WORD songs;
	WORD patterns;
	WORD channels;
	DWORD song_id;		// "SONG" = 0x474e4f53
	DWORD song_len;
	CHAR songname2[44];
	WORD orders;
//	WORD orderlist[0];	// orderlist[orders] in words
} DBMFILEHEADER;

typedef struct DBMINSTRUMENT
{
	CHAR name[30];
	WORD sampleno;
	WORD volume;
	DWORD finetune;
	DWORD loopstart;
	DWORD looplen;
	WORD panning;
	WORD flags;
} DBMINSTRUMENT;

typedef struct DBMENVELOPE
{
	WORD instrument;
	BYTE flags;
	BYTE numpoints;
	BYTE sustain1;
	BYTE loopbegin;
	BYTE loopend;
	BYTE sustain2;
	WORD volenv[2*32];
} DBMENVELOPE;

typedef struct DBMPATTERN
{
	WORD rows;
	DWORD packedsize;
	BYTE patterndata[2];	// [packedsize]
} DBMPATTERN;

typedef struct DBMSAMPLE
{
	DWORD flags;
	DWORD samplesize;
	BYTE sampledata[2];		// [samplesize]
} DBMSAMPLE;

#pragma pack()


BOOL CSoundFile::ReadDBM(const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
	const DBMFILEHEADER *pfh = (DBMFILEHEADER *)lpStream;
	DWORD dwMemPos;
	UINT nOrders, nSamples, nInstruments, nPatterns;
	
	if ((!lpStream) || (dwMemLength <= sizeof(DBMFILEHEADER)) || (!pfh->channels)
	 || (pfh->dbm_id != bswapLE32(DBM_FILE_MAGIC)) || (!pfh->songs) || (pfh->song_id != bswapLE32(DBM_ID_SONG))
	 || (pfh->name_id != bswapLE32(DBM_ID_NAME)) || (pfh->name_len != bswapLE32(DBM_NAMELEN))
	 || (pfh->info_id != bswapLE32(DBM_ID_INFO)) || (pfh->info_len != bswapLE32(DBM_INFOLEN))) return FALSE;
	dwMemPos = sizeof(DBMFILEHEADER);
	nOrders = bswapBE16(pfh->orders);
	if (dwMemPos + 2 * nOrders + 8*3 >= dwMemLength) return FALSE;
	nInstruments = bswapBE16(pfh->instruments);
	nSamples = bswapBE16(pfh->samples);
	nPatterns = bswapBE16(pfh->patterns);
	m_nType = MOD_TYPE_DBM;
	m_nChannels = bswapBE16(pfh->channels);
	if (m_nChannels < 4) m_nChannels = 4;
	if (m_nChannels > 64) m_nChannels = 64;
	memcpy(m_szNames[0], (pfh->songname[0]) ? pfh->songname : pfh->songname2, 32);
	m_szNames[0][31] = 0;
	for (UINT iOrd=0; iOrd < nOrders; iOrd++)
	{
		Order[iOrd] = lpStream[dwMemPos+iOrd*2+1];
		if (iOrd >= MAX_ORDERS-2) break;
	}
	dwMemPos += 2*nOrders;
	while (dwMemPos + 10 < dwMemLength)
	{
		DWORD chunk_id = ((LPDWORD)(lpStream+dwMemPos))[0];
		DWORD chunk_size = bswapBE32(((LPDWORD)(lpStream+dwMemPos))[1]);
		DWORD chunk_pos;
		
		dwMemPos += 8;
		chunk_pos = dwMemPos;
		if ((dwMemPos + chunk_size > dwMemLength) || (chunk_size > dwMemLength)) break;
		dwMemPos += chunk_size;
		// Instruments
		if (chunk_id == bswapLE32(DBM_ID_INST))
		{
			// Skip duplicate chunks.
			if (m_nInstruments) continue;

			if (nInstruments >= MAX_INSTRUMENTS) nInstruments = MAX_INSTRUMENTS-1;
			for (UINT iIns=0; iIns<nInstruments; iIns++)
			{
				MODINSTRUMENT *psmp;
				INSTRUMENTHEADER *penv;
				DBMINSTRUMENT *pih;
				UINT nsmp;

				if (chunk_pos + sizeof(DBMINSTRUMENT) > dwMemPos) break;
				if ((penv = new INSTRUMENTHEADER) == NULL) break;
				pih = (DBMINSTRUMENT *)(lpStream+chunk_pos);
				nsmp = bswapBE16(pih->sampleno);
				psmp = ((nsmp) && (nsmp < MAX_SAMPLES)) ? &Ins[nsmp] : NULL;
				memset(penv, 0, sizeof(INSTRUMENTHEADER));
				memcpy(penv->name, pih->name, 30);
				if (psmp)
				{
					memcpy(m_szNames[nsmp], pih->name, 30);
					m_szNames[nsmp][30] = 0;
				}
				Headers[iIns+1] = penv;
				penv->nFadeOut = 1024;	// ???
				penv->nGlobalVol = 64;
				penv->nPan = bswapBE16(pih->panning);
				if ((penv->nPan) && (penv->nPan < 256))
					penv->dwFlags = ENV_SETPANNING;
				else
					penv->nPan = 128;
				penv->nPPC = 5*12;
				for (UINT i=0; i<NOTE_MAX; i++)
				{
					penv->Keyboard[i] = nsmp;
					penv->NoteMap[i] = i+1;
				}
				// Sample Info
				if (psmp)
				{
					DWORD sflags = bswapBE16(pih->flags);
					psmp->nVolume = bswapBE16(pih->volume) * 4;
					if ((!psmp->nVolume) || (psmp->nVolume > 256)) psmp->nVolume = 256;
					psmp->nGlobalVol = 64;
					psmp->nC4Speed = bswapBE32(pih->finetune);
					int f2t = FrequencyToTranspose(psmp->nC4Speed);
					psmp->RelativeTone = f2t >> 7;
					psmp->nFineTune = f2t & 0x7F;
					if ((pih->looplen) && (sflags & 3))
					{
						psmp->nLoopStart = bswapBE32(pih->loopstart);
						psmp->nLoopEnd = psmp->nLoopStart + bswapBE32(pih->looplen);
						psmp->uFlags |= CHN_LOOP;
						psmp->uFlags &= ~CHN_PINGPONGLOOP;
						if (sflags & 2) psmp->uFlags |= CHN_PINGPONGLOOP;
					}
				}
				chunk_pos += sizeof(DBMINSTRUMENT);
				m_nInstruments = iIns+1;
			}
		} else
		// Volume Envelopes
		if (chunk_id == bswapLE32(DBM_ID_VENV))
		{
			UINT nEnvelopes = lpStream[chunk_pos+1];
			
			chunk_pos += 2;
			for (UINT iEnv=0; iEnv<nEnvelopes; iEnv++)
			{
				DBMENVELOPE *peh;
				UINT nins;
				
				if (chunk_pos + sizeof(DBMENVELOPE) > dwMemPos) break;
				peh = (DBMENVELOPE *)(lpStream+chunk_pos);
				nins = bswapBE16(peh->instrument);
				if ((nins) && (nins < MAX_INSTRUMENTS) && (Headers[nins]) && (peh->numpoints))
				{
					INSTRUMENTHEADER *penv = Headers[nins];

					if (peh->flags & 1) penv->dwFlags |= ENV_VOLUME;
					if (peh->flags & 2) penv->dwFlags |= ENV_VOLSUSTAIN;
					if (peh->flags & 4) penv->dwFlags |= ENV_VOLLOOP;
					penv->nVolEnv = peh->numpoints + 1;
					if (penv->nVolEnv > MAX_ENVPOINTS) penv->nVolEnv = MAX_ENVPOINTS;
					penv->nVolLoopStart = peh->loopbegin;
					penv->nVolLoopEnd = peh->loopend;
					penv->nVolSustainBegin = penv->nVolSustainEnd = peh->sustain1;
					for (UINT i=0; i<penv->nVolEnv; i++)
					{
						penv->VolPoints[i] = bswapBE16(peh->volenv[i*2]);
						penv->VolEnv[i] = (BYTE)bswapBE16(peh->volenv[i*2+1]);
					}
				}
				chunk_pos += sizeof(DBMENVELOPE);
			}
		} else
		// Packed Pattern Data
		if (chunk_id == bswapLE32(DBM_ID_PATT))
		{
			if (nPatterns > MAX_PATTERNS) nPatterns = MAX_PATTERNS;
			for (UINT iPat=0; iPat<nPatterns; iPat++)
			{
				DBMPATTERN *pph;
				DWORD pksize;
				UINT nRows;

				// Skip duplicate chunks.
				if (Patterns[iPat]) break;

				if (chunk_pos + sizeof(DBMPATTERN) > dwMemPos) break;
				pph = (DBMPATTERN *)(lpStream+chunk_pos);
				pksize = bswapBE32(pph->packedsize);
				if ((chunk_pos + pksize + 6 > dwMemPos) || (pksize > dwMemPos)) break;
				nRows = bswapBE16(pph->rows);
				if ((nRows >= 4) && (nRows <= 256))
				{
					MODCOMMAND *m = AllocatePattern(nRows, m_nChannels);
					if (m)
					{
						LPBYTE pkdata = (LPBYTE)pph->patterndata;
						UINT row = 0;
						UINT i = 0;

						PatternSize[iPat] = nRows;
						Patterns[iPat] = m;
						while ((i+3<pksize) && (row < nRows))
						{
							UINT ch = pkdata[i++];

							if (ch)
							{
								BYTE b = pkdata[i++];
								ch--;
								if (ch < m_nChannels)
								{
									if (b & 0x01)
									{
										UINT note = pkdata[i++];

										if (note == 0x1F) note = 0xFF; else
										if ((note) && (note < 0xFE))
										{
											note = ((note >> 4)*12) + (note & 0x0F) + 13;
										}
										m[ch].note = note;
									}
									if (b & 0x02) m[ch].instr = pkdata[i++];
									if (b & 0x3C)
									{
										UINT cmd1 = 0xFF, param1 = 0, cmd2 = 0xFF, param2 = 0;
										if (b & 0x04) cmd1 = (UINT)pkdata[i++];
										if (b & 0x08) param1 = pkdata[i++];
										if (b & 0x10) cmd2 = (UINT)pkdata[i++];
										if (b & 0x20) param2 = pkdata[i++];
										if (cmd1 == 0x0C)
										{
											m[ch].volcmd = VOLCMD_VOLUME;
											m[ch].vol = param1;
											cmd1 = 0xFF;
										} else
										if (cmd2 == 0x0C)
										{
											m[ch].volcmd = VOLCMD_VOLUME;
											m[ch].vol = param2;
											cmd2 = 0xFF;
										}
										if ((cmd1 > 0x13) || ((cmd1 >= 0x10) && (cmd2 < 0x10)))
										{
											cmd1 = cmd2;
											param1 = param2;
											cmd2 = 0xFF;
										}
										if (cmd1 <= 0x13)
										{
											m[ch].command = cmd1;
											m[ch].param = param1;
											ConvertModCommand(&m[ch]);
										}
									}
								} else
								{
									if (b & 0x01) i++;
									if (b & 0x02) i++;
									if (b & 0x04) i++;
									if (b & 0x08) i++;
									if (b & 0x10) i++;
									if (b & 0x20) i++;
								}
							} else
							{
								row++;
								m += m_nChannels;
							}
						}
					}
				}
				chunk_pos += 6 + pksize;
			}
		} else
		// Reading Sample Data
		if (chunk_id == bswapLE32(DBM_ID_SMPL))
		{
			if (nSamples >= MAX_SAMPLES) nSamples = MAX_SAMPLES-1;
			m_nSamples = nSamples;
			for (UINT iSmp=1; iSmp<=nSamples; iSmp++)
			{
				MODINSTRUMENT *pins;
				DBMSAMPLE *psh;
				DWORD samplesize;
				DWORD sampleflags;

				if (chunk_pos + sizeof(DBMSAMPLE) >= dwMemPos) break;
				psh = (DBMSAMPLE *)(lpStream+chunk_pos);
				chunk_pos += 8;
				samplesize = bswapBE32(psh->samplesize);
				sampleflags = bswapBE32(psh->flags);
				pins = &Ins[iSmp];
				pins->nLength = samplesize;
				if (sampleflags & 2)
				{
					pins->uFlags |= CHN_16BIT;
					samplesize <<= 1;
				}
				if ((chunk_pos+samplesize > dwMemPos) || (samplesize > dwMemLength)) break;
				if (sampleflags & 3)
				{
					ReadSample(pins, (pins->uFlags & CHN_16BIT) ? RS_PCM16M : RS_PCM8S,
								(LPSTR)(psh->sampledata), samplesize);
				}
				chunk_pos += samplesize;
			}
		}
	}
	return TRUE;
}
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

///////////////////////////////////////////////////////
// DMF DELUSION DIGITAL MUSIC FILEFORMAT (X-Tracker) //
///////////////////////////////////////////////////////
//#include "stdafx.h"
//#include "sndfile.h"

//#define DMFLOG

//#pragma warning(disable:4244)

#pragma pack(1)

typedef struct DMFHEADER
{
	DWORD id;				// "DDMF" = 0x464d4444
	BYTE version;			// 4
	CHAR trackername[8];	// "XTRACKER"
	CHAR songname[30];
	CHAR composer[20];
	BYTE date[3];
} DMFHEADER;

typedef struct DMFINFO
{
	DWORD id;			// "INFO"
	DWORD infosize;
} DMFINFO;

typedef struct DMFSEQU
{
	DWORD id;			// "SEQU"
	DWORD seqsize;
	WORD loopstart;
	WORD loopend;
	WORD sequ[2];
} DMFSEQU;

typedef struct DMFPATT
{
	DWORD id;			// "PATT"
	DWORD patsize;
	WORD numpat;		// 1-1024
	BYTE tracks;
	BYTE firstpatinfo;
} DMFPATT;

typedef struct DMFTRACK
{
	BYTE tracks;
	BYTE beat;		// [hi|lo] -> hi=ticks per beat, lo=beats per measure
	WORD ticks;		// max 512
	DWORD jmpsize;
} DMFTRACK;

typedef struct DMFSMPI
{
	DWORD id;
	DWORD size;
	BYTE samples;
} DMFSMPI;

typedef struct DMFSAMPLE
{
	DWORD len;
	DWORD loopstart;
	DWORD loopend;
	WORD c3speed;
	BYTE volume;
	BYTE flags;
} DMFSAMPLE;

#pragma pack()


#ifdef DMFLOG
extern void Log(LPCSTR s, ...);
#endif


BOOL CSoundFile::ReadDMF(const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
	const DMFHEADER *pfh = (DMFHEADER *)lpStream;
	const DMFINFO *psi;
	const DMFPATT *patt;
	const DMFSEQU *sequ;
	DWORD dwMemPos;
	BYTE infobyte[32];
	BYTE smplflags[MAX_SAMPLES], hasSMPI = 0, hasSMPD = 0;

	if ((!lpStream) || (dwMemLength < 1024)) return FALSE;
	if ((pfh->id != 0x464d4444) || (!pfh->version) || (pfh->version & 0xF0)) return FALSE;
	dwMemPos = 66;
	memcpy(m_szNames[0], pfh->songname, 30);
	m_szNames[0][30] = 0;
	m_nType = MOD_TYPE_DMF;
	m_nChannels = 0;
#ifdef DMFLOG
	Log("DMF version %d: \"%s\": %d bytes (0x%04X)\n", pfh->version, m_szNames[0], dwMemLength, dwMemLength);
#endif
	while (dwMemPos < dwMemLength - 7)
	{
		DWORD id = *((LPDWORD)(lpStream+dwMemPos));

		switch(id)
		{
		// "INFO"
		case 0x4f464e49:
		// "CMSG"
		case 0x47534d43:
			psi = (DMFINFO *)(lpStream+dwMemPos);
			if (id == 0x47534d43) dwMemPos++;
			if ((psi->infosize > dwMemLength) || (dwMemPos + 8 > dwMemLength - psi->infosize)) goto dmfexit;
			if ((psi->infosize >= 8) && (!m_lpszSongComments))
			{
			    m_lpszSongComments = new char[psi->infosize]; // changed from CHAR
				if (m_lpszSongComments)
				{
					for (UINT i=0; i<psi->infosize-1; i++)
					{
						CHAR c = lpStream[dwMemPos+8+i];
						if ((i % 40) == 39)
							m_lpszSongComments[i] = 0x0d;
						else
							m_lpszSongComments[i] = (c < ' ') ? ' ' : c;
					}
					m_lpszSongComments[psi->infosize-1] = 0;
				}
			}
			dwMemPos += psi->infosize + 8 - 1;
			break;

		// "SEQU"
		case 0x55514553:
			sequ = (DMFSEQU *)(lpStream+dwMemPos);
			if ((sequ->seqsize >= dwMemLength) || (dwMemPos + 8 > dwMemLength - sequ->seqsize)) goto dmfexit;
			if (sequ->seqsize >= 4)
			{
				UINT nseq = (sequ->seqsize - 4) >> 1;
				if (nseq >= MAX_ORDERS-1) nseq = MAX_ORDERS-1;
				if (sequ->loopstart < nseq) m_nRestartPos = sequ->loopstart;
				for (UINT i=0; i<nseq; i++) Order[i] = (BYTE)sequ->sequ[i];
			}
			dwMemPos += sequ->seqsize + 8;
			break;

		// "PATT"
		case 0x54544150:
			patt = (DMFPATT *)(lpStream+dwMemPos);
			if ((patt->patsize >= dwMemLength) || (dwMemPos + 8 > dwMemLength - patt->patsize)) goto dmfexit;
			if (patt->patsize >= 4 && !m_nChannels)
			{
				UINT numpat;
				DWORD dwPos = dwMemPos + 11;
				numpat = patt->numpat;
				if (numpat > MAX_PATTERNS) numpat = MAX_PATTERNS;
				m_nChannels = patt->tracks;
				if (m_nChannels < patt->firstpatinfo) m_nChannels = patt->firstpatinfo;
				if (m_nChannels > 32) m_nChannels = 32;
				if (m_nChannels < 4) m_nChannels = 4;
				for (UINT npat=0; npat<numpat; npat++)
				{
					const DMFTRACK *pt = (DMFTRACK *)(lpStream+dwPos);
					if (dwPos + 8 >= dwMemLength) break;
				#ifdef DMFLOG
					Log("Pattern #%d: %d tracks, %d rows\n", npat, pt->tracks, pt->ticks);
				#endif
					UINT tracks = pt->tracks;
					if (tracks > 32) tracks = 32;
					UINT ticks = pt->ticks;
					if (ticks > 256) ticks = 256;
					if (ticks < 16) ticks = 16;
					dwPos += 8;
					if ((pt->jmpsize >= dwMemLength) || (dwPos + 4 > dwMemLength - pt->jmpsize)) break;
					PatternSize[npat] = (WORD)ticks;
					MODCOMMAND *m = AllocatePattern(PatternSize[npat], m_nChannels);
					if (!m) goto dmfexit;
					Patterns[npat] = m;
					DWORD d = dwPos;
					dwPos += pt->jmpsize;
					UINT ttype = 1;
					UINT tempo = 125;
					UINT glbinfobyte = 0;
					UINT pbeat = (pt->beat & 0xf0) ? pt->beat>>4 : 8;
					BOOL tempochange = (pt->beat & 0xf0) ? TRUE : FALSE;
					memset(infobyte, 0, sizeof(infobyte));
					for (UINT row=0; row<ticks; row++)
					{
						MODCOMMAND *p = &m[row*m_nChannels];
						// Parse track global effects
						if (!glbinfobyte)
						{
							if (d+1 > dwPos) break;
							BYTE info = lpStream[d++];
							BYTE infoval = 0;
							if ((info & 0x80) && (d < dwPos)) glbinfobyte = lpStream[d++];
							info &= 0x7f;
							if ((info) && (d < dwPos)) infoval = lpStream[d++];
							switch(info)
							{
							case 1:	ttype = 0; tempo = infoval; tempochange = TRUE; break;
							case 2: ttype = 1; tempo = infoval; tempochange = TRUE; break;
							case 3: pbeat = infoval>>4; tempochange = ttype; break;
							#ifdef DMFLOG
							default: if (info) Log("GLB: %02X.%02X\n", info, infoval);
							#endif
							}
						} else
						{
							glbinfobyte--;
						}
						// Parse channels
						for (UINT i=0; i<tracks; i++) if (!infobyte[i])
						{
							if (d+1 > dwPos) break;
							MODCOMMAND cmd = {0,0,0,0,0,0};
							BYTE info = lpStream[d++];
							if (info & 0x80)
							{
								if (d+1 > dwPos) break;
								infobyte[i] = lpStream[d++];
							}
							// Instrument
							if (info & 0x40)
							{
								if (d+1 > dwPos) break;
								cmd.instr = lpStream[d++];
							}
							// Note
							if (info & 0x20)
							{
								if (d+1 > dwPos) break;
								cmd.note = lpStream[d++];
								if ((cmd.note) && (cmd.note < 0xfe)) cmd.note &= 0x7f;
								if ((cmd.note) && (cmd.note < 128)) cmd.note += 24;
							}
							// Volume
							if (info & 0x10)
							{
								if (d+1 > dwPos) break;
								cmd.volcmd = VOLCMD_VOLUME;
								cmd.vol = (lpStream[d++]+3)>>2;
							}
							// Effect 1
							if (info & 0x08)
							{
								if (d+2 > dwPos) break;

								BYTE efx = lpStream[d++];
								BYTE eval = lpStream[d++];
								switch(efx)
								{
								// 1: Key Off
								case 1: if (!cmd.note) cmd.note = 0xFE; break;
								// 2: Set Loop
								// 4: Sample Delay
								case 4: if (eval&0xe0) { cmd.command = CMD_S3MCMDEX; cmd.param = (eval>>5)|0xD0; } break;
								// 5: Retrig
								case 5: if (eval&0xe0) { cmd.command = CMD_RETRIG; cmd.param = (eval>>5); } break;
								// 6: Offset
								case 6: cmd.command = CMD_OFFSET; cmd.param = eval; break;
								#ifdef DMFLOG
								default: Log("FX1: %02X.%02X\n", efx, eval);
								#endif
								}
							}
							// Effect 2
							if (info & 0x04)
							{
								if (d+2 > dwPos) break;

								BYTE efx = lpStream[d++];
								BYTE eval = lpStream[d++];
								switch(efx)
								{
								// 1: Finetune
								case 1: if (eval&0xf0) { cmd.command = CMD_S3MCMDEX; cmd.param = (eval>>4)|0x20; } break;
								// 2: Note Delay
								case 2: if (eval&0xe0) { cmd.command = CMD_S3MCMDEX; cmd.param = (eval>>5)|0xD0; } break;
								// 3: Arpeggio
								case 3: if (eval) { cmd.command = CMD_ARPEGGIO; cmd.param = eval; } break;
								// 4: Portamento Up
								case 4: cmd.command = CMD_PORTAMENTOUP; cmd.param = (eval >= 0xe0) ? 0xdf : eval; break;
								// 5: Portamento Down
								case 5: cmd.command = CMD_PORTAMENTODOWN; cmd.param = (eval >= 0xe0) ? 0xdf : eval; break;
								// 6: Tone Portamento
								case 6: cmd.command = CMD_TONEPORTAMENTO; cmd.param = eval; break;
								// 8: Vibrato
								case 8: cmd.command = CMD_VIBRATO; cmd.param = eval; break;
								// 12: Note cut
								case 12: if (eval & 0xe0) { cmd.command = CMD_S3MCMDEX; cmd.param = (eval>>5)|0xc0; }
									 else if (!cmd.note) { cmd.note = 0xfe; }
									 break;
								#ifdef DMFLOG
								default: Log("FX2: %02X.%02X\n", efx, eval);
								#endif
								}
							}
							// Effect 3
							if (info & 0x02)
							{
								if (d+2 > dwPos) break;

								BYTE efx = lpStream[d++];
								BYTE eval = lpStream[d++];
								switch(efx)
								{
								// 1: Vol Slide Up
								case 1: if (eval == 0xff) break;
										eval = (eval+3)>>2; if (eval > 0x0f) eval = 0x0f;
										cmd.command = CMD_VOLUMESLIDE; cmd.param = eval<<4; break;
								// 2: Vol Slide Down
								case 2:	if (eval == 0xff) break;
										eval = (eval+3)>>2; if (eval > 0x0f) eval = 0x0f;
										cmd.command = CMD_VOLUMESLIDE; cmd.param = eval; break;
								// 7: Set Pan
								case 7: if (!cmd.volcmd) { cmd.volcmd = VOLCMD_PANNING; cmd.vol = (eval+3)>>2; }
										else { cmd.command = CMD_PANNING8; cmd.param = eval; } break;
								// 8: Pan Slide Left
								case 8: eval = (eval+3)>>2; if (eval > 0x0f) eval = 0x0f;
										cmd.command = CMD_PANNINGSLIDE; cmd.param = eval<<4; break;
								// 9: Pan Slide Right
								case 9: eval = (eval+3)>>2; if (eval > 0x0f) eval = 0x0f;
										cmd.command = CMD_PANNINGSLIDE; cmd.param = eval; break;
								#ifdef DMFLOG
								default: Log("FX3: %02X.%02X\n", efx, eval);
								#endif

								}
							}
							// Store effect
							if (i < m_nChannels) p[i] = cmd;
							if (d > dwPos)
							{
							#ifdef DMFLOG
								Log("Unexpected EOP: row=%d\n", row);
							#endif
								break;
							}
						} else
						{
							infobyte[i]--;
						}

						// Find free channel for tempo change
						if (tempochange)
						{
							tempochange = FALSE;
							UINT speed=6, modtempo=tempo;
							UINT rpm = ((ttype) && (pbeat)) ? tempo*pbeat : (tempo+1)*15;
							for (speed=30; speed>1; speed--)
							{
								modtempo = rpm*speed/24;
								if (modtempo <= 200) break;
								if ((speed < 6) && (modtempo < 256)) break;
							}
						#ifdef DMFLOG
							Log("Tempo change: ttype=%d pbeat=%d tempo=%3d -> speed=%d tempo=%d\n",
								ttype, pbeat, tempo, speed, modtempo);
						#endif
							for (UINT ich=0; ich<m_nChannels; ich++) if (!p[ich].command)
							{
								if (speed)
								{
									p[ich].command = CMD_SPEED;
									p[ich].param = (BYTE)speed;
									speed = 0;
								} else
								if ((modtempo >= 32) && (modtempo < 256))
								{
									p[ich].command = CMD_TEMPO;
									p[ich].param = (BYTE)modtempo;
									modtempo = 0;
								} else
								{
									break;
								}
							}
						}
						if (d >= dwPos) break;
					}
				#ifdef DMFLOG
					Log(" %d/%d bytes remaining\n", dwPos-d, pt->jmpsize);
				#endif
					if (dwPos + 8 >= dwMemLength) break;
				}
			}
			dwMemPos += patt->patsize + 8;
			break;

		// "SMPI": Sample Info
		case 0x49504d53:
			{
				hasSMPI = 1;
				const DMFSMPI *pds = (DMFSMPI *)(lpStream+dwMemPos);
				if ((pds->size >= dwMemLength) || (dwMemPos + 8 > dwMemLength - pds->size)) goto dmfexit;
				if (pds->size >= 1)
				{
					DWORD dwPos = dwMemPos + 9;
					m_nSamples = pds->samples;
					if (m_nSamples >= MAX_SAMPLES) m_nSamples = MAX_SAMPLES-1;
					for (UINT iSmp=1; iSmp<=m_nSamples; iSmp++)
					{
						if (dwPos >= dwMemPos + pds->size + 8) break;
						UINT namelen = lpStream[dwPos];
						smplflags[iSmp] = 0;
						if (dwPos+namelen+1+sizeof(DMFSAMPLE) > dwMemPos+pds->size+8) break;
						if (namelen)
						{
							UINT rlen = (namelen < 32) ? namelen : 31;
							memcpy(m_szNames[iSmp], lpStream+dwPos+1, rlen);
							m_szNames[iSmp][rlen] = 0;
						}
						dwPos += namelen + 1;
						const DMFSAMPLE *psh = (DMFSAMPLE *)(lpStream+dwPos);
						MODINSTRUMENT *psmp = &Ins[iSmp];
						psmp->nLength = psh->len;
						psmp->nLoopStart = psh->loopstart;
						psmp->nLoopEnd = psh->loopend;
						psmp->nC4Speed = psh->c3speed;
						psmp->nGlobalVol = 64;
						psmp->nVolume = (psh->volume) ? ((WORD)psh->volume)+1 : (WORD)256;
						psmp->uFlags = (psh->flags & 2) ? CHN_16BIT : 0;
						if (psmp->uFlags & CHN_16BIT) psmp->nLength >>= 1;
						if (psh->flags & 1) psmp->uFlags |= CHN_LOOP;
						smplflags[iSmp] = psh->flags;
						dwPos += (pfh->version < 8) ? 22 : 30;
					#ifdef DMFLOG
						Log("SMPI %d/%d: len=%d flags=0x%02X\n", iSmp, m_nSamples, psmp->nLength, psh->flags);
					#endif
					}
				}
				dwMemPos += pds->size + 8;
			}
			break;

		// "SMPD": Sample Data
		case 0x44504d53:
			{
				DWORD dwPos = dwMemPos + 8;
				UINT ismpd = 0;
				for (UINT iSmp=1; iSmp<=m_nSamples && !hasSMPD; iSmp++)
				{
					ismpd++;
					DWORD pksize;
					if (dwPos + 4 >= dwMemLength)
					{
					#ifdef DMFLOG
						Log("Unexpected EOF at sample %d/%d! (pos=%d)\n", iSmp, m_nSamples, dwPos);
					#endif
						break;
					}
					pksize = *((LPDWORD)(lpStream+dwPos));
				#ifdef DMFLOG
					Log("sample %d: pos=0x%X pksize=%d ", iSmp, dwPos, pksize);
					Log("len=%d flags=0x%X [%08X]\n", Ins[iSmp].nLength, smplflags[ismpd], *((LPDWORD)(lpStream+dwPos+4)));
				#endif
					dwPos += 4;
					if (pksize > dwMemLength - dwPos)
					{
					#ifdef DMFLOG
						Log("WARNING: pksize=%d, but only %d bytes left\n", pksize, dwMemLength-dwPos);
					#endif
						pksize = dwMemLength - dwPos;
					}
					if ((pksize) && (iSmp <= m_nSamples))
					{
						UINT flags = (Ins[iSmp].uFlags & CHN_16BIT) ? RS_PCM16S : RS_PCM8S;
						if (hasSMPI && smplflags[ismpd] & 4)
							flags = (Ins[iSmp].uFlags & CHN_16BIT) ? RS_DMF16 : RS_DMF8;
						ReadSample(&Ins[iSmp], flags, (LPSTR)(lpStream+dwPos), pksize);
					}
					dwPos += pksize;
				}
				hasSMPD = 1;
				dwMemPos = dwPos;
			}
			break;

		// "ENDE": end of file
		case 0x45444e45:
			goto dmfexit;
		
		// Unrecognized id, or "ENDE" field
		default:
			dwMemPos += 4;
			break;
		}
	}
dmfexit:
	if (!m_nChannels)
	{
		if (!m_nSamples)
		{
			m_nType = MOD_TYPE_NONE;
			return FALSE;
		}
		m_nChannels = 4;
	}
	return TRUE;
}


///////////////////////////////////////////////////////////////////////
// DMF Compression

#pragma pack(1)

typedef struct DMF_HNODE
{
	short int left, right;
	BYTE value;
} DMF_HNODE;

typedef struct DMF_HTREE
{
	LPBYTE ibuf, ibufmax;
	DWORD bitbuf;
	UINT bitnum;
	UINT lastnode, nodecount;
	DMF_HNODE nodes[256];
} DMF_HTREE;

#pragma pack()


// DMF Huffman ReadBits
BYTE DMFReadBits(DMF_HTREE *tree, UINT nbits)
//-------------------------------------------
{
	BYTE x = 0, bitv = 1;
	while (nbits--)
	{
		if (tree->bitnum)
		{
			tree->bitnum--;
		} else
		if (tree->ibuf < tree->ibufmax) {
			tree->bitbuf = *(tree->ibuf++);
			tree->bitnum = 7;
		}
		if (tree->bitbuf & 1) x |= bitv;
		bitv <<= 1;
		tree->bitbuf >>= 1;
	}
	return x;
}

//
// tree: [8-bit value][12-bit index][12-bit index] = 32-bit
//

void DMFNewNode(DMF_HTREE *tree)
//------------------------------
{
	BYTE isleft, isright;
	UINT actnode;

	actnode = tree->nodecount;
	if (actnode > 255) return;
	tree->nodes[actnode].value = DMFReadBits(tree, 7);
	isleft = DMFReadBits(tree, 1);
	isright = DMFReadBits(tree, 1);
	actnode = tree->lastnode;
	if (actnode > 255) return;
	tree->nodecount++;
	tree->lastnode = tree->nodecount;
	if (isleft)
	{
		tree->nodes[actnode].left = tree->lastnode;
		DMFNewNode(tree);
	} else
	{
		tree->nodes[actnode].left = -1;
	}
	tree->lastnode = tree->nodecount;
	if (isright)
	{
		tree->nodes[actnode].right = tree->lastnode;
		DMFNewNode(tree);
	} else
	{
		tree->nodes[actnode].right = -1;
	}
}


int DMFUnpack(LPBYTE psample, LPBYTE ibuf, LPBYTE ibufmax, UINT maxlen)
//----------------------------------------------------------------------
{
	DMF_HTREE tree;
	UINT actnode;
	BYTE value, sign, delta = 0;

	memset(&tree, 0, sizeof(tree));
	tree.ibuf = ibuf;
	tree.ibufmax = ibufmax;
	DMFNewNode(&tree);
	value = 0;

	if (tree.ibuf >= ibufmax) return tree.ibuf - ibuf;

	for (UINT i=0; i<maxlen; i++)
	{
		if ((tree.ibuf >= tree.ibufmax) && (!tree.bitnum))
		{
		#ifdef DMFLOG
			Log("DMFUnpack: unexpected EOF at output byte %d / %d\n", i, maxlen);
		#endif
			break;
		}
		actnode = 0;
		sign = DMFReadBits(&tree, 1);
		do
		{
			if (DMFReadBits(&tree, 1))
				actnode = tree.nodes[actnode].right;
			else
				actnode = tree.nodes[actnode].left;
			if (actnode > 255) break;
			delta = tree.nodes[actnode].value;
			if ((tree.ibuf >= tree.ibufmax) && (!tree.bitnum)) break;
		} while ((tree.nodes[actnode].left >= 0) && (tree.nodes[actnode].right >= 0));
		if (sign) delta ^= 0xFF;
		value += delta;
		psample[i] = (i) ? value : 0;
	}
#ifdef DMFLOG
//	Log("DMFUnpack: %d remaining bytes\n", tree.ibufmax-tree.ibuf);
#endif
	return tree.ibuf - ibuf;
}


/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

//////////////////////////////////////////////
// DSIK Internal Format (DSM) module loader //
//////////////////////////////////////////////
//#include "stdafx.h"
//#include "sndfile.h"

#pragma pack(1)

#define DSMID_RIFF	0x46464952	// "RIFF"
#define DSMID_DSMF	0x464d5344	// "DSMF"
#define DSMID_SONG	0x474e4f53	// "SONG"
#define DSMID_INST	0x54534e49	// "INST"
#define DSMID_PATT	0x54544150	// "PATT"


typedef struct DSMNOTE
{
	BYTE note,ins,vol,cmd,inf;
} DSMNOTE;


typedef struct DSMINST
{
	DWORD id_INST;
	DWORD inst_len;
	CHAR filename[13];
	BYTE flags;
	BYTE flags2;
	BYTE volume;
	DWORD length;
	DWORD loopstart;
	DWORD loopend;
	DWORD reserved1;
	WORD c2spd;
	WORD reserved2;
	CHAR samplename[28];
} DSMINST;


typedef struct DSMFILEHEADER
{
	DWORD id_RIFF;	// "RIFF"
	DWORD riff_len;
	DWORD id_DSMF;	// "DSMF"
	DWORD id_SONG;	// "SONG"
	DWORD song_len;
} DSMFILEHEADER;


typedef struct DSMSONG
{
	CHAR songname[28];
	WORD reserved1;
	WORD flags;
	DWORD reserved2;
	WORD numord;
	WORD numsmp;
	WORD numpat;
	WORD numtrk;
	BYTE globalvol;
	BYTE mastervol;
	BYTE speed;
	BYTE bpm;
	BYTE panpos[16];
	BYTE orders[128];
} DSMSONG;

typedef struct DSMPATT
{
	DWORD id_PATT;
	DWORD patt_len;
	BYTE dummy1;
	BYTE dummy2;
} DSMPATT;

#pragma pack()


BOOL CSoundFile::ReadDSM(LPCBYTE lpStream, DWORD dwMemLength)
//-----------------------------------------------------------
{
	DSMFILEHEADER *pfh = (DSMFILEHEADER *)lpStream;
	DSMSONG *psong;
	DWORD dwMemPos;
	UINT nPat, nSmp;

	if ((!lpStream) || (dwMemLength < 1024) || (pfh->id_RIFF != DSMID_RIFF)
	 || (pfh->riff_len + 8 > dwMemLength) || (pfh->riff_len < 1024)
	 || (pfh->id_DSMF != DSMID_DSMF) || (pfh->id_SONG != DSMID_SONG)
	 || (pfh->song_len > dwMemLength)) return FALSE;
	psong = (DSMSONG *)(lpStream + sizeof(DSMFILEHEADER));
	dwMemPos = sizeof(DSMFILEHEADER) + pfh->song_len;
	m_nType = MOD_TYPE_DSM;
	m_nChannels = psong->numtrk;
	if (m_nChannels < 4) m_nChannels = 4;
	if (m_nChannels > 16) m_nChannels = 16;
	m_nSamples = psong->numsmp;
	if (m_nSamples >= MAX_SAMPLES) m_nSamples = MAX_SAMPLES - 1;
	m_nDefaultSpeed = psong->speed;
	m_nDefaultTempo = psong->bpm;
	m_nDefaultGlobalVolume = psong->globalvol << 2;
	if ((!m_nDefaultGlobalVolume) || (m_nDefaultGlobalVolume > 256)) m_nDefaultGlobalVolume = 256;
	m_nSongPreAmp = psong->mastervol & 0x7F;
	for (UINT iOrd=0; iOrd<sizeof(psong->orders); iOrd++)
	{
		Order[iOrd] = (BYTE)((iOrd < psong->numord) ? psong->orders[iOrd] : 0xFF);
	}
	for (UINT iPan=0; iPan<16; iPan++)
	{
		ChnSettings[iPan].nPan = 0x80;
		if (psong->panpos[iPan] <= 0x80)
		{
			ChnSettings[iPan].nPan = psong->panpos[iPan] << 1;
		}
	}
	memcpy(m_szNames[0], psong->songname, 28);
	nPat = 0;
	nSmp = 1;
	while (dwMemPos < dwMemLength - 8)
	{
		DSMPATT *ppatt = (DSMPATT *)(lpStream + dwMemPos);
		DSMINST *pins = (DSMINST *)(lpStream+dwMemPos);
		// Reading Patterns
		if (ppatt->id_PATT == DSMID_PATT)
		{
			dwMemPos += 8;
			if (dwMemPos + ppatt->patt_len >= dwMemLength) break;
			DWORD dwPos = dwMemPos;
			dwMemPos += ppatt->patt_len;
			MODCOMMAND *m = AllocatePattern(64, m_nChannels);
			if (!m) break;
			PatternSize[nPat] = 64;
			Patterns[nPat] = m;
			UINT row = 0;
			while ((row < 64) && (dwPos + 2 <= dwMemPos))
			{
				UINT flag = lpStream[dwPos++];
				if (flag)
				{
					UINT ch = (flag & 0x0F) % m_nChannels;
					if (flag & 0x80)
					{
						UINT note = lpStream[dwPos++];
						if (note)
						{
							if (note <= 12*9) note += 12;
							m[ch].note = (BYTE)note;
						}
					}
					if (flag & 0x40)
					{
						m[ch].instr = lpStream[dwPos++];
					}
					if (flag & 0x20)
					{
						m[ch].volcmd = VOLCMD_VOLUME;
						m[ch].vol = lpStream[dwPos++];
					}
					if (flag & 0x10)
					{
						UINT command = lpStream[dwPos++];
						UINT param = lpStream[dwPos++];
						switch(command)
						{
						// 4-bit Panning
						case 0x08:
							switch(param & 0xF0)
							{
							case 0x00: param <<= 4; break;
							case 0x10: command = 0x0A; param = (param & 0x0F) << 4; break;
							case 0x20: command = 0x0E; param = (param & 0x0F) | 0xA0; break;
							case 0x30: command = 0x0E; param = (param & 0x0F) | 0x10; break;
							case 0x40: command = 0x0E; param = (param & 0x0F) | 0x20; break;
							default: command = 0;
							}
							break;
						// Portamentos
						case 0x11:
						case 0x12:
							command &= 0x0F;
							break;
						// 3D Sound (?)
						case 0x13:
							command = 'X' - 55;
							param = 0x91;
							break;
						default:
							// Volume + Offset (?)
							command = ((command & 0xF0) == 0x20) ? 0x09 : 0;
						}
						m[ch].command = (BYTE)command;
						m[ch].param = (BYTE)param;
						if (command) ConvertModCommand(&m[ch]);
					}
				} else
				{
					m += m_nChannels;
					row++;
				}
			}
			nPat++;
		} else
		// Reading Samples
		if ((nSmp <= m_nSamples) && (pins->id_INST == DSMID_INST))
		{
			if (dwMemPos + pins->inst_len >= dwMemLength - 8) break;
			DWORD dwPos = dwMemPos + sizeof(DSMINST);
			dwMemPos += 8 + pins->inst_len;
			memcpy(m_szNames[nSmp], pins->samplename, 28);
			MODINSTRUMENT *psmp = &Ins[nSmp];
			memcpy(psmp->name, pins->filename, 13);
			psmp->nGlobalVol = 64;
			psmp->nC4Speed = pins->c2spd;
			psmp->uFlags = (WORD)((pins->flags & 1) ? CHN_LOOP : 0);
			psmp->nLength = pins->length;
			psmp->nLoopStart = pins->loopstart;
			psmp->nLoopEnd = pins->loopend;
			psmp->nVolume = (WORD)(pins->volume << 2);
			if (psmp->nVolume > 256) psmp->nVolume = 256;
			UINT smptype = (pins->flags & 2) ? RS_PCM8S : RS_PCM8U;
			ReadSample(psmp, smptype, (LPCSTR)(lpStream+dwPos), dwMemLength - dwPos);
			nSmp++;
		} else
		{
			break;
		}
	}
	return TRUE;
}

/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

////////////////////////////////////////
// Farandole (FAR) module loader	  //
////////////////////////////////////////
//#include "stdafx.h"
//#include "sndfile.h"

//#pragma warning(disable:4244)

#define FARFILEMAGIC	0xFE524146	// "FAR"

#pragma pack(1)

typedef struct FARHEADER1
{
	DWORD id;				// file magic FAR=
	CHAR songname[40];		// songname
	CHAR magic2[3];			// 13,10,26
	WORD headerlen;			// remaining length of header in bytes
	BYTE version;			// 0xD1
	BYTE onoff[16];
	BYTE edit1[9];
	BYTE speed;
	BYTE panning[16];
	BYTE edit2[4];
	WORD stlen;
} FARHEADER1;

typedef struct FARHEADER2
{
	BYTE orders[256];
	BYTE numpat;
	BYTE snglen;
	BYTE loopto;
	WORD patsiz[256];
} FARHEADER2;

typedef struct FARSAMPLE
{
	CHAR samplename[32];
	DWORD length;
	BYTE finetune;
	BYTE volume;
	DWORD reppos;
	DWORD repend;
	BYTE type;
	BYTE loop;
} FARSAMPLE;

#pragma pack()


BOOL CSoundFile::ReadFAR(const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
	const FARHEADER1 *pmh1 = (const FARHEADER1 *)lpStream;
	const FARHEADER2 *pmh2;
	DWORD dwMemPos = sizeof(FARHEADER1);
	UINT headerlen, stlen;
	BYTE samplemap[8];

	if ((!lpStream) || (dwMemLength < 1024) || (bswapLE32(pmh1->id) != FARFILEMAGIC)
	 || (pmh1->magic2[0] != 13) || (pmh1->magic2[1] != 10) || (pmh1->magic2[2] != 26)) return FALSE;
	headerlen = bswapLE16(pmh1->headerlen);
	stlen = bswapLE16( pmh1->stlen );
	if ((headerlen >= dwMemLength) || (dwMemPos + stlen + sizeof(FARHEADER2) >= dwMemLength)) return FALSE;
	// Globals
	m_nType = MOD_TYPE_FAR;
	m_nChannels = 16;
	m_nInstruments = 0;
	m_nSamples = 0;
	m_nSongPreAmp = 0x20;
	m_nDefaultSpeed = pmh1->speed;
	m_nDefaultTempo = 80;
	m_nDefaultGlobalVolume = 256;

	memcpy(m_szNames[0], pmh1->songname, 32);
	// Channel Setting
	for (UINT nchpan=0; nchpan<16; nchpan++)
	{
		ChnSettings[nchpan].dwFlags = 0;
		ChnSettings[nchpan].nPan = ((pmh1->panning[nchpan] & 0x0F) << 4) + 8;
		ChnSettings[nchpan].nVolume = 64;
	}
	// Reading comment
	if (stlen)
	{
		UINT szLen = stlen;
		if (szLen > dwMemLength - dwMemPos) szLen = dwMemLength - dwMemPos;
		if ((m_lpszSongComments = new char[szLen + 1]) != NULL)
		{
			memcpy(m_lpszSongComments, lpStream+dwMemPos, szLen);
			m_lpszSongComments[szLen] = 0;
		}
		dwMemPos += stlen;
	}
	// Reading orders
	if (sizeof(FARHEADER2) > dwMemLength - dwMemPos) return TRUE;
	pmh2 = (const FARHEADER2 *)(lpStream + dwMemPos);
	dwMemPos += sizeof(FARHEADER2);
	if (dwMemPos >= dwMemLength) return TRUE;
	for (UINT iorder=0; iorder<MAX_ORDERS; iorder++)
	{
		Order[iorder] = (iorder <= pmh2->snglen) ? pmh2->orders[iorder] : 0xFF;
	}
	m_nRestartPos = pmh2->loopto;
	// Reading Patterns
	dwMemPos += headerlen - (869 + stlen);
	if (dwMemPos >= dwMemLength) return TRUE;

	// end byteswap of pattern data

	WORD *patsiz = (WORD *)pmh2->patsiz;
	for (UINT ipat=0; ipat<256; ipat++) if (patsiz[ipat])
	{
		UINT patlen = bswapLE16(patsiz[ipat]);
		if ((ipat >= MAX_PATTERNS) || (patlen < 2))
		{
			dwMemPos += patlen;
			continue;
		}
		if (dwMemPos + patlen >= dwMemLength) return TRUE;
		UINT max  = (patlen - 2) & ~3;
		UINT rows = (patlen - 2) >> 6;
		if (!rows)
		{
			dwMemPos += patlen;
			continue;
		}
		if (rows > 256) rows = 256;
		if (rows < 16) rows = 16;
		if (max > rows*16*4) max = rows*16*4;
		PatternSize[ipat] = rows;
		if ((Patterns[ipat] = AllocatePattern(rows, m_nChannels)) == NULL) return TRUE;
		MODCOMMAND *m = Patterns[ipat];
		UINT patbrk = lpStream[dwMemPos];
		const BYTE *p = lpStream + dwMemPos + 2;
		for (UINT len=0; len<max; len += 4, m++)
		{
			BYTE note = p[len];
			BYTE ins = p[len+1];
			BYTE vol = p[len+2];
			BYTE eff = p[len+3];
			if (note)
			{
				m->instr = ins + 1;
				m->note = note + 36;
			}
			if (vol >= 0x01 && vol <= 0x10)
			{
				m->volcmd = VOLCMD_VOLUME;
				m->vol = (vol - 1) << 2;
			}
			switch(eff & 0xF0)
			{
			// 1.x: Portamento Up
			case 0x10:
				m->command = CMD_PORTAMENTOUP;
				m->param = eff & 0x0F;
				break;
			// 2.x: Portamento Down
			case 0x20:
				m->command = CMD_PORTAMENTODOWN;
				m->param = eff & 0x0F;
				break;
			// 3.x: Tone-Portamento
			case 0x30:
				m->command = CMD_TONEPORTAMENTO;
				m->param = (eff & 0x0F) << 2;
				break;
			// 4.x: Retrigger
			case 0x40:
				m->command = CMD_RETRIG;
				m->param = 6 / (1+(eff&0x0F)) + 1;
				break;
			// 5.x: Set Vibrato Depth
			case 0x50:
				m->command = CMD_VIBRATO;
				m->param = (eff & 0x0F);
				break;
			// 6.x: Set Vibrato Speed
			case 0x60:
				m->command = CMD_VIBRATO;
				m->param = (eff & 0x0F) << 4;
				break;
			// 7.x: Vol Slide Up
			case 0x70:
				m->command = CMD_VOLUMESLIDE;
				m->param = (eff & 0x0F) << 4;
				break;
			// 8.x: Vol Slide Down
			case 0x80:
				m->command = CMD_VOLUMESLIDE;
				m->param = (eff & 0x0F);
				break;
			// A.x: Port to vol
			case 0xA0:
				m->volcmd = VOLCMD_VOLUME;
				m->vol = ((eff & 0x0F) << 2) + 4;
				break;
			// B.x: Set Balance
			case 0xB0:
				m->command = CMD_PANNING8;
				m->param = (eff & 0x0F) << 4;
				break;
			// F.x: Set Speed
			case 0xF0:
				m->command = CMD_SPEED;
				m->param = eff & 0x0F;
				break;
			default:
				if ((patbrk) &&	(patbrk+1 == (len >> 6)) && (patbrk+1 != rows-1))
				{
					m->command = CMD_PATTERNBREAK;
					patbrk = 0;
				}
			}
		}
		dwMemPos += patlen;
	}
	// Reading samples
	if (dwMemPos + 8 >= dwMemLength) return TRUE;
	memcpy(samplemap, lpStream+dwMemPos, 8);
	dwMemPos += 8;
	MODINSTRUMENT *pins = &Ins[1];
	for (UINT ismp=0; ismp<64; ismp++, pins++) if (samplemap[ismp >> 3] & (1 << (ismp & 7)))
	{
		if (dwMemPos + sizeof(FARSAMPLE) > dwMemLength) return TRUE;
		const FARSAMPLE *pfs = reinterpret_cast<const FARSAMPLE*>(lpStream + dwMemPos);
		dwMemPos += sizeof(FARSAMPLE);
		m_nSamples = ismp + 1;
		memcpy(m_szNames[ismp+1], pfs->samplename, 32);
		const DWORD length = bswapLE32( pfs->length ) ; /* endian fix - Toad */
		pins->nLength = length ;
		pins->nLoopStart = bswapLE32(pfs->reppos) ;
		pins->nLoopEnd = bswapLE32(pfs->repend) ;
		pins->nFineTune = 0;
		pins->nC4Speed = 8363*2;
		pins->nGlobalVol = 64;
		pins->nVolume = pfs->volume << 4;
		pins->uFlags = 0;
		if ((pins->nLength > 3) && (dwMemPos + 4 < dwMemLength))
		{
			if (pfs->type & 1)
			{
				pins->uFlags |= CHN_16BIT;
				pins->nLength >>= 1;
				pins->nLoopStart >>= 1;
				pins->nLoopEnd >>= 1;
			}
			if ((pfs->loop & 8) && (pins->nLoopEnd > 4)) pins->uFlags |= CHN_LOOP;
			ReadSample(pins, (pins->uFlags & CHN_16BIT) ? RS_PCM16S : RS_PCM8S,
						(LPSTR)(lpStream+dwMemPos), dwMemLength - dwMemPos);
		}
		dwMemPos += length;
	}
	return TRUE;
}
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (Endian and char fixes for PPC)
 *          Marco Trillo     <toad@arsystel.com> (Endian fixes for SaveIT, XM->IT Sample Converter)
 *
*/

//#include "stdafx.h"
//#include "sndfile.h"
//#include "it_defs.h"

#ifdef _MSC_VER
#pragma warning(disable:4244)
#endif

BYTE autovibit2xm[8] =
{ 0, 3, 1, 4, 2, 0, 0, 0 };

BYTE autovibxm2it[8] =
{ 0, 2, 4, 1, 3, 0, 0, 0 };

//////////////////////////////////////////////////////////
// Impulse Tracker IT file support

// for conversion of XM samples
// extern WORD XMPeriodTable[96+8];
// extern UINT XMLinearTable[768];


BOOL CSoundFile::ITInstrToMPT(const void *p, INSTRUMENTHEADER *penv, UINT trkvers)
//--------------------------------------------------------------------------------
{
	if (trkvers < 0x0200)
	{
		const ITOLDINSTRUMENT *pis = (const ITOLDINSTRUMENT *)p;
		memcpy(penv->name, pis->name, 26);
		memcpy(penv->filename, pis->filename, 12);
		penv->nFadeOut = bswapLE16(pis->fadeout) << 6;
		penv->nGlobalVol = 64;
		for (UINT j=0; j<NOTE_MAX; j++)
		{
			UINT note = pis->keyboard[j*2];
			UINT ins = pis->keyboard[j*2+1];
			if (ins < MAX_SAMPLES) penv->Keyboard[j] = ins;
			if (note < 128) penv->NoteMap[j] = note+1;
			else if (note >= 0xFE) penv->NoteMap[j] = note;
		}
		if (pis->flags & 0x01) penv->dwFlags |= ENV_VOLUME;
		if (pis->flags & 0x02) penv->dwFlags |= ENV_VOLLOOP;
		if (pis->flags & 0x04) penv->dwFlags |= ENV_VOLSUSTAIN;
		penv->nVolLoopStart = pis->vls;
		penv->nVolLoopEnd = pis->vle;
		penv->nVolSustainBegin = pis->sls;
		penv->nVolSustainEnd = pis->sle;
		penv->nVolEnv = 25;
		for (UINT ev=0; ev<25; ev++)
		{
			if ((penv->VolPoints[ev] = pis->nodes[ev*2]) == 0xFF)
			{
				penv->nVolEnv = ev;
				break;
			}
			penv->VolEnv[ev] = pis->nodes[ev*2+1];
		}
		penv->nNNA = pis->nna;
		penv->nDCT = pis->dnc;
		penv->nPan = 0x80;
	} else
	{
		const ITINSTRUMENT *pis = (const ITINSTRUMENT *)p;
		memcpy(penv->name, pis->name, 26);
		memcpy(penv->filename, pis->filename, 12);
		penv->nMidiProgram = pis->mpr;
		penv->nMidiChannel = pis->mch;
		penv->wMidiBank = bswapLE16(pis->mbank);
		penv->nFadeOut = bswapLE16(pis->fadeout) << 5;
		penv->nGlobalVol = pis->gbv >> 1;
		if (penv->nGlobalVol > 64) penv->nGlobalVol = 64;
		for (UINT j=0; j<NOTE_MAX; j++)
		{
			UINT note = pis->keyboard[j*2];
			UINT ins = pis->keyboard[j*2+1];
			if (ins < MAX_SAMPLES) penv->Keyboard[j] = ins;
			if (note < 128) penv->NoteMap[j] = note+1;
			else if (note >= 0xFE) penv->NoteMap[j] = note;
		}
		// Volume Envelope
		if (pis->volenv.flags & 1) penv->dwFlags |= ENV_VOLUME;
		if (pis->volenv.flags & 2) penv->dwFlags |= ENV_VOLLOOP;
		if (pis->volenv.flags & 4) penv->dwFlags |= ENV_VOLSUSTAIN;
		if (pis->volenv.flags & 8) penv->dwFlags |= ENV_VOLCARRY;
		penv->nVolEnv = pis->volenv.num;
		if (penv->nVolEnv > 25) penv->nVolEnv = 25;

		penv->nVolLoopStart = pis->volenv.lpb;
		penv->nVolLoopEnd = pis->volenv.lpe;
		penv->nVolSustainBegin = pis->volenv.slb;
		penv->nVolSustainEnd = pis->volenv.sle;
		// Panning Envelope
		if (pis->panenv.flags & 1) penv->dwFlags |= ENV_PANNING;
		if (pis->panenv.flags & 2) penv->dwFlags |= ENV_PANLOOP;
		if (pis->panenv.flags & 4) penv->dwFlags |= ENV_PANSUSTAIN;
		if (pis->panenv.flags & 8) penv->dwFlags |= ENV_PANCARRY;
		penv->nPanEnv = pis->panenv.num;
		if (penv->nPanEnv > 25) penv->nPanEnv = 25;
		penv->nPanLoopStart = pis->panenv.lpb;
		penv->nPanLoopEnd = pis->panenv.lpe;
		penv->nPanSustainBegin = pis->panenv.slb;
		penv->nPanSustainEnd = pis->panenv.sle;
		// Pitch Envelope
		if (pis->pitchenv.flags & 1) penv->dwFlags |= ENV_PITCH;
		if (pis->pitchenv.flags & 2) penv->dwFlags |= ENV_PITCHLOOP;
		if (pis->pitchenv.flags & 4) penv->dwFlags |= ENV_PITCHSUSTAIN;
		if (pis->pitchenv.flags & 8) penv->dwFlags |= ENV_PITCHCARRY;
		if (pis->pitchenv.flags & 0x80) penv->dwFlags |= ENV_FILTER;
		penv->nPitchEnv = pis->pitchenv.num;
		if (penv->nPitchEnv > 25) penv->nPitchEnv = 25;
		penv->nPitchLoopStart = pis->pitchenv.lpb;
		penv->nPitchLoopEnd = pis->pitchenv.lpe;
		penv->nPitchSustainBegin = pis->pitchenv.slb;
		penv->nPitchSustainEnd = pis->pitchenv.sle;
		// Envelopes Data
		for (UINT ev=0; ev<25; ev++)
		{
			penv->VolEnv[ev] = pis->volenv.data[ev*3];
			penv->VolPoints[ev] = (pis->volenv.data[ev*3+2] << 8) | (pis->volenv.data[ev*3+1]);
			penv->PanEnv[ev] = pis->panenv.data[ev*3] + 32;
			penv->PanPoints[ev] = (pis->panenv.data[ev*3+2] << 8) | (pis->panenv.data[ev*3+1]);
			penv->PitchEnv[ev] = pis->pitchenv.data[ev*3] + 32;
			penv->PitchPoints[ev] = (pis->pitchenv.data[ev*3+2] << 8) | (pis->pitchenv.data[ev*3+1]);
		}
		penv->nNNA = pis->nna;
		penv->nDCT = pis->dct;
		penv->nDNA = pis->dca;
		penv->nPPS = pis->pps;
		penv->nPPC = pis->ppc;
		penv->nIFC = pis->ifc;
		penv->nIFR = pis->ifr;
		penv->nVolSwing = pis->rv;
		penv->nPanSwing = pis->rp;
		penv->nPan = (pis->dfp & 0x7F) << 2;
		if (penv->nPan > 256) penv->nPan = 128;
		if (pis->dfp < 0x80) penv->dwFlags |= ENV_SETPANNING;
	}
	if ((penv->nVolLoopStart >= 25) || (penv->nVolLoopEnd >= 25)) penv->dwFlags &= ~ENV_VOLLOOP;
	if ((penv->nVolSustainBegin >= 25) || (penv->nVolSustainEnd >= 25)) penv->dwFlags &= ~ENV_VOLSUSTAIN;
	return TRUE;
}


BOOL CSoundFile::ReadIT(const BYTE *lpStream, DWORD dwMemLength)
//--------------------------------------------------------------
{
	DWORD dwMemPos = sizeof(ITFILEHEADER);
	DWORD inspos[MAX_INSTRUMENTS];
	DWORD smppos[MAX_SAMPLES];
	DWORD patpos[MAX_PATTERNS];
	BYTE chnmask[64];//, channels_used[64]
	MODCOMMAND lastvalue[64];
	UINT j;

	if ((!lpStream) || (dwMemLength < sizeof(ITFILEHEADER))) return FALSE;
	ITFILEHEADER pifh = *(ITFILEHEADER *)lpStream;

	pifh.id = bswapLE32(pifh.id);
	pifh.reserved1 = bswapLE16(pifh.reserved1);
	pifh.ordnum = bswapLE16(pifh.ordnum);
	pifh.insnum = bswapLE16(pifh.insnum);
	pifh.smpnum = bswapLE16(pifh.smpnum);
	pifh.patnum = bswapLE16(pifh.patnum);
	pifh.cwtv = bswapLE16(pifh.cwtv);
	pifh.cmwt = bswapLE16(pifh.cmwt);
	pifh.flags = bswapLE16(pifh.flags);
	pifh.special = bswapLE16(pifh.special);
	pifh.msglength = bswapLE16(pifh.msglength);
	pifh.msgoffset = bswapLE32(pifh.msgoffset);
	pifh.reserved2 = bswapLE32(pifh.reserved2);

	if ((pifh.id != 0x4D504D49) || (pifh.insnum >= MAX_INSTRUMENTS)
	 || (!pifh.smpnum) || (pifh.smpnum >= MAX_INSTRUMENTS) || (!pifh.ordnum)) return FALSE;
	if (dwMemPos + pifh.ordnum + pifh.insnum*4
	 + pifh.smpnum*4 + pifh.patnum*4 > dwMemLength) return FALSE;
	m_nType = MOD_TYPE_IT;
	if (pifh.flags & 0x08) m_dwSongFlags |= SONG_LINEARSLIDES;
	if (pifh.flags & 0x10) m_dwSongFlags |= SONG_ITOLDEFFECTS;
	if (pifh.flags & 0x20) m_dwSongFlags |= SONG_ITCOMPATMODE;
	if (pifh.flags & 0x80) m_dwSongFlags |= SONG_EMBEDMIDICFG;
	if (pifh.flags & 0x1000) m_dwSongFlags |= SONG_EXFILTERRANGE;
	memcpy(m_szNames[0], pifh.songname, 26);
	m_szNames[0][26] = 0;
	// Global Volume
	if (pifh.globalvol)
	{
		m_nDefaultGlobalVolume = pifh.globalvol << 1;
		if (!m_nDefaultGlobalVolume) m_nDefaultGlobalVolume = 256;
		if (m_nDefaultGlobalVolume > 256) m_nDefaultGlobalVolume = 256;
	}
	if (pifh.speed) m_nDefaultSpeed = pifh.speed;
	if (pifh.tempo) m_nDefaultTempo = pifh.tempo;
	m_nSongPreAmp = pifh.mv & 0x7F;
	// Reading Channels Pan Positions
	for (int ipan=0; ipan<64; ipan++) if (pifh.chnpan[ipan] != 0xFF)
	{
		ChnSettings[ipan].nVolume = pifh.chnvol[ipan];
		ChnSettings[ipan].nPan = 128;
		if (pifh.chnpan[ipan] & 0x80) ChnSettings[ipan].dwFlags |= CHN_MUTE;
		UINT n = pifh.chnpan[ipan] & 0x7F;
		if (n <= 64) ChnSettings[ipan].nPan = n << 2;
		if (n == 100) ChnSettings[ipan].dwFlags |= CHN_SURROUND;
	}
	if (m_nChannels < 4) m_nChannels = 4;
	// Reading Song Message
	if ((pifh.special & 0x01) && (pifh.msglength) && (pifh.msglength <= dwMemLength) && (pifh.msgoffset < dwMemLength - pifh.msglength))
	{
		m_lpszSongComments = new char[pifh.msglength+1];
		if (m_lpszSongComments)
		{
			memcpy(m_lpszSongComments, lpStream+pifh.msgoffset, pifh.msglength);
			m_lpszSongComments[pifh.msglength] = 0;
		}
	}
	// Reading orders
	UINT nordsize = pifh.ordnum;
	if (nordsize > MAX_ORDERS) nordsize = MAX_ORDERS;
	memcpy(Order, lpStream+dwMemPos, nordsize);
	dwMemPos += pifh.ordnum;
	// Reading Instrument Offsets
	memset(inspos, 0, sizeof(inspos));
	UINT inspossize = pifh.insnum;
	if (inspossize > MAX_INSTRUMENTS) inspossize = MAX_INSTRUMENTS;
	inspossize <<= 2;
	memcpy(inspos, lpStream+dwMemPos, inspossize);
	for (j=0; j < (inspossize>>2); j++) {
	    inspos[j] = bswapLE32(inspos[j]);
	}
	dwMemPos += pifh.insnum * 4;
	// Reading Samples Offsets
	memset(smppos, 0, sizeof(smppos));
	UINT smppossize = pifh.smpnum;
	if (smppossize > MAX_SAMPLES) smppossize = MAX_SAMPLES;
	smppossize <<= 2;
	memcpy(smppos, lpStream+dwMemPos, smppossize);
	for (j=0; j < (smppossize>>2); j++) {
	    smppos[j] = bswapLE32(smppos[j]);
	}
	dwMemPos += pifh.smpnum * 4;
	// Reading Patterns Offsets
	memset(patpos, 0, sizeof(patpos));
	UINT patpossize = pifh.patnum;
	if (patpossize > MAX_PATTERNS) patpossize = MAX_PATTERNS;
	patpossize <<= 2;
	memcpy(patpos, lpStream+dwMemPos, patpossize);
	for (j=0; j < (patpossize>>2); j++) {
	    patpos[j] = bswapLE32(patpos[j]);
	}
	dwMemPos += pifh.patnum * 4;
	// Reading IT Extra Info
	if (dwMemPos + 2 < dwMemLength)
	{
		UINT nflt = bswapLE16(*((WORD *)(lpStream + dwMemPos)));
		dwMemPos += 2;
		if (dwMemPos + nflt * 8 < dwMemLength) dwMemPos += nflt * 8;
	}
	// Reading Midi Output & Macros
	if (m_dwSongFlags & SONG_EMBEDMIDICFG)
	{
		if (dwMemPos + sizeof(MODMIDICFG) < dwMemLength)
		{
			memcpy(&m_MidiCfg, lpStream+dwMemPos, sizeof(MODMIDICFG));
			dwMemPos += sizeof(MODMIDICFG);
		}
	}
	// Read pattern names: "PNAM"
	if ((dwMemPos + 8 < dwMemLength) && (bswapLE32(*((DWORD *)(lpStream+dwMemPos))) == 0x4d414e50))
	{
		UINT len = bswapLE32(*((DWORD *)(lpStream+dwMemPos+4)));
		dwMemPos += 8;
		if ((dwMemPos + len <= dwMemLength) && (len <= MAX_PATTERNS*MAX_PATTERNNAME) && (len >= MAX_PATTERNNAME))
		{
			m_lpszPatternNames = new char[len];
			if (m_lpszPatternNames)
			{
				m_nPatternNames = len / MAX_PATTERNNAME;
				memcpy(m_lpszPatternNames, lpStream+dwMemPos, len);
			}
			dwMemPos += len;
		}
	}
	// 4-channels minimum
	m_nChannels = 4;
	// Read channel names: "CNAM"
	if ((dwMemPos + 8 < dwMemLength) && (bswapLE32(*((DWORD *)(lpStream+dwMemPos))) == 0x4d414e43))
	{
		UINT len = bswapLE32(*((DWORD *)(lpStream+dwMemPos+4)));
		dwMemPos += 8;
		if ((dwMemPos + len <= dwMemLength) && (len <= 64*MAX_CHANNELNAME))
		{
			UINT n = len / MAX_CHANNELNAME;
			if (n > m_nChannels) m_nChannels = n;
			for (UINT i=0; i<n; i++)
			{
				memcpy(ChnSettings[i].szName, (lpStream+dwMemPos+i*MAX_CHANNELNAME), MAX_CHANNELNAME);
				ChnSettings[i].szName[MAX_CHANNELNAME-1] = 0;
			}
			dwMemPos += len;
		}
	}
	// Read mix plugins information
	if (dwMemPos + 8 < dwMemLength)
	{
		dwMemPos += LoadMixPlugins(lpStream+dwMemPos, dwMemLength-dwMemPos);
	}
	// Checking for unused channels
	UINT npatterns = pifh.patnum;
	if (npatterns > MAX_PATTERNS) npatterns = MAX_PATTERNS;
	for (UINT patchk=0; patchk<npatterns; patchk++)
	{
		memset(chnmask, 0, sizeof(chnmask));
		if ((!patpos[patchk]) || ((DWORD)patpos[patchk] >= dwMemLength - 4)) continue;
		UINT len = bswapLE16(*((WORD *)(lpStream+patpos[patchk])));
		UINT rows = bswapLE16(*((WORD *)(lpStream+patpos[patchk]+2)));
		if ((rows < 4) || (rows > 256)) continue;
		if (8+len > dwMemLength || patpos[patchk] > dwMemLength - (8+len)) continue;
		UINT i = 0;
		const BYTE *p = lpStream+patpos[patchk]+8;
		UINT nrow = 0;
		while (nrow<rows)
		{
			if (i >= len) break;
			BYTE b = p[i++];
			if (!b)
			{
				nrow++;
				continue;
			}
			UINT ch = b & 0x7F;
			if (ch) ch = (ch - 1) & 0x3F;
			if (b & 0x80)
			{
				if (i >= len) break;
				chnmask[ch] = p[i++];
			}
			// Channel used
			if (chnmask[ch] & 0x0F)
			{
				if ((ch >= m_nChannels) && (ch < 64)) m_nChannels = ch+1;
			}
			// Note
			if (chnmask[ch] & 1) i++;
			// Instrument
			if (chnmask[ch] & 2) i++;
			// Volume
			if (chnmask[ch] & 4) i++;
			// Effect
			if (chnmask[ch] & 8) i += 2;
			if (i >= len) break;
		}
	}
	// Reading Instruments
	m_nInstruments = 0;
	if (pifh.flags & 0x04) m_nInstruments = pifh.insnum;
	if (m_nInstruments >= MAX_INSTRUMENTS) m_nInstruments = MAX_INSTRUMENTS-1;
	for (UINT nins=0; nins<m_nInstruments; nins++)
	{
		if ((inspos[nins] > 0) && dwMemLength > sizeof(ITOLDINSTRUMENT) &&
			(inspos[nins] < dwMemLength - sizeof(ITOLDINSTRUMENT)))
		{
			INSTRUMENTHEADER *penv = new INSTRUMENTHEADER;
			if (!penv) continue;
			Headers[nins+1] = penv;
			memset(penv, 0, sizeof(INSTRUMENTHEADER));
			ITInstrToMPT(lpStream + inspos[nins], penv, pifh.cmwt);
		}
	}
	// Reading Samples
	m_nSamples = pifh.smpnum;
	if (m_nSamples >= MAX_SAMPLES) m_nSamples = MAX_SAMPLES-1;
	for (UINT nsmp=0; nsmp<pifh.smpnum; nsmp++) if ((smppos[nsmp]) && (smppos[nsmp] <= dwMemLength - sizeof(ITSAMPLESTRUCT)))
	{
		ITSAMPLESTRUCT pis = *(ITSAMPLESTRUCT *)(lpStream+smppos[nsmp]);
		pis.id = bswapLE32(pis.id);
		pis.length = bswapLE32(pis.length);
		pis.loopbegin = bswapLE32(pis.loopbegin);
		pis.loopend = bswapLE32(pis.loopend);
		pis.C5Speed = bswapLE32(pis.C5Speed);
		pis.susloopbegin = bswapLE32(pis.susloopbegin);
		pis.susloopend = bswapLE32(pis.susloopend);
		pis.samplepointer = bswapLE32(pis.samplepointer);

		if (pis.id == 0x53504D49)
		{
			MODINSTRUMENT *pins = &Ins[nsmp+1];
			memcpy(pins->name, pis.filename, 12);
			pins->uFlags = 0;
			pins->nLength = 0;
			pins->nLoopStart = pis.loopbegin;
			pins->nLoopEnd = pis.loopend;
			pins->nSustainStart = pis.susloopbegin;
			pins->nSustainEnd = pis.susloopend;
			pins->nC4Speed = pis.C5Speed;
			if (!pins->nC4Speed) pins->nC4Speed = 8363;
			if (pis.C5Speed < 256) pins->nC4Speed = 256;
			pins->nVolume = pis.vol << 2;
			if (pins->nVolume > 256) pins->nVolume = 256;
			pins->nGlobalVol = pis.gvl;
			if (pins->nGlobalVol > 64) pins->nGlobalVol = 64;
			if (pis.flags & 0x10) pins->uFlags |= CHN_LOOP;
			if (pis.flags & 0x20) pins->uFlags |= CHN_SUSTAINLOOP;
			if (pis.flags & 0x40) pins->uFlags |= CHN_PINGPONGLOOP;
			if (pis.flags & 0x80) pins->uFlags |= CHN_PINGPONGSUSTAIN;
			pins->nPan = (pis.dfp & 0x7F) << 2;
			if (pins->nPan > 256) pins->nPan = 256;
			if (pis.dfp & 0x80) pins->uFlags |= CHN_PANNING;
			pins->nVibType = autovibit2xm[pis.vit & 7];
			pins->nVibRate = pis.vis;
			pins->nVibDepth = pis.vid & 0x7F;
			pins->nVibSweep = (pis.vir + 3) / 4;
			if ((pis.samplepointer) && (pis.samplepointer < dwMemLength) && (pis.length))
			{
				pins->nLength = pis.length;
				if (pins->nLength > MAX_SAMPLE_LENGTH) pins->nLength = MAX_SAMPLE_LENGTH;
				UINT flags = (pis.cvt & 1) ? RS_PCM8S : RS_PCM8U;
				if (pis.flags & 2)
				{
					flags += 5;
					pins->uFlags |= CHN_16BIT;
					// IT 2.14 16-bit packed sample ?
					if (pis.flags & 8) flags = ((pifh.cmwt >= 0x215) && (pis.cvt & 4)) ? RS_IT21516 : RS_IT21416;
					if (pis.flags & 4) flags |= RSF_STEREO;
				} else
				{
					if (pis.cvt == 0xFF) flags = RS_ADPCM4; else
					// IT 2.14 8-bit packed sample ?
					if (pis.flags & 8)	flags =	((pifh.cmwt >= 0x215) && (pis.cvt & 4)) ? RS_IT2158 : RS_IT2148;
					if (pis.flags & 4) flags |= RSF_STEREO;
				}
				ReadSample(&Ins[nsmp+1], flags, (LPSTR)(lpStream+pis.samplepointer), dwMemLength - pis.samplepointer);
			}
		}
		memcpy(m_szNames[nsmp+1], pis.name, 26);
	}
	// Reading Patterns
	for (UINT npat=0; npat<npatterns; npat++)
	{
		if ((!patpos[npat]) || ((DWORD)patpos[npat] >= dwMemLength - 4))
		{
			PatternSize[npat] = 64;
			Patterns[npat] = AllocatePattern(64, m_nChannels);
			continue;
		}

		UINT len = bswapLE16(*((WORD *)(lpStream+patpos[npat])));
		UINT rows = bswapLE16(*((WORD *)(lpStream+patpos[npat]+2)));
		if ((rows < 4) || (rows > 256)) continue;
		if (8+len > dwMemLength || patpos[npat] > dwMemLength - (8+len)) continue;
		PatternSize[npat] = rows;
		if ((Patterns[npat] = AllocatePattern(rows, m_nChannels)) == NULL) continue;
		memset(lastvalue, 0, sizeof(lastvalue));
		memset(chnmask, 0, sizeof(chnmask));
		MODCOMMAND *m = Patterns[npat];
		UINT i = 0;
		const BYTE *p = lpStream+patpos[npat]+8;
		UINT nrow = 0;
		while (nrow<rows)
		{
			if (i >= len) break;
			BYTE b = p[i++];
			if (!b)
			{
				nrow++;
				m+=m_nChannels;
				continue;
			}
			UINT ch = b & 0x7F;
			if (ch) ch = (ch - 1) & 0x3F;
			if (b & 0x80)
			{
				if (i >= len) break;
				chnmask[ch] = p[i++];
			}
			if ((chnmask[ch] & 0x10) && (ch < m_nChannels))
			{
				m[ch].note = lastvalue[ch].note;
			}
			if ((chnmask[ch] & 0x20) && (ch < m_nChannels))
			{
				m[ch].instr = lastvalue[ch].instr;
			}
			if ((chnmask[ch] & 0x40) && (ch < m_nChannels))
			{
				m[ch].volcmd = lastvalue[ch].volcmd;
				m[ch].vol = lastvalue[ch].vol;
			}
			if ((chnmask[ch] & 0x80) && (ch < m_nChannels))
			{
				m[ch].command = lastvalue[ch].command;
				m[ch].param = lastvalue[ch].param;
			}
			if (chnmask[ch] & 1)	// Note
			{
				if (i >= len) break;
				UINT note = p[i++];
				if (ch < m_nChannels)
				{
					if (note < 0x80) note++;
					m[ch].note = note;
					lastvalue[ch].note = note;
				//	channels_used[ch] = TRUE;
				}
			}
			if (chnmask[ch] & 2)
			{
				if (i >= len) break;
				UINT instr = p[i++];
				if (ch < m_nChannels)
				{
					m[ch].instr = instr;
					lastvalue[ch].instr = instr;
				}
			}
			if (chnmask[ch] & 4)
			{
				if (i >= len) break;
				UINT vol = p[i++];
				if (ch < m_nChannels)
				{
					// 0-64: Set Volume
					if (vol <= 64) { m[ch].volcmd = VOLCMD_VOLUME; m[ch].vol = vol; } else
					// 128-192: Set Panning
					if ((vol >= 128) && (vol <= 192)) { m[ch].volcmd = VOLCMD_PANNING; m[ch].vol = vol - 128; } else
					// 65-74: Fine Volume Up
					if (vol < 75) { m[ch].volcmd = VOLCMD_FINEVOLUP; m[ch].vol = vol - 65; } else
					// 75-84: Fine Volume Down
					if (vol < 85) { m[ch].volcmd = VOLCMD_FINEVOLDOWN; m[ch].vol = vol - 75; } else
					// 85-94: Volume Slide Up
					if (vol < 95) { m[ch].volcmd = VOLCMD_VOLSLIDEUP; m[ch].vol = vol - 85; } else
					// 95-104: Volume Slide Down
					if (vol < 105) { m[ch].volcmd = VOLCMD_VOLSLIDEDOWN; m[ch].vol = vol - 95; } else
					// 105-114: Pitch Slide Up
					if (vol < 115) { m[ch].volcmd = VOLCMD_PORTADOWN; m[ch].vol = vol - 105; } else
					// 115-124: Pitch Slide Down
					if (vol < 125) { m[ch].volcmd = VOLCMD_PORTAUP; m[ch].vol = vol - 115; } else
					// 193-202: Portamento To
					if ((vol >= 193) && (vol <= 202)) { m[ch].volcmd = VOLCMD_TONEPORTAMENTO; m[ch].vol = vol - 193; } else
					// 203-212: Vibrato
					if ((vol >= 203) && (vol <= 212)) { m[ch].volcmd = VOLCMD_VIBRATOSPEED; m[ch].vol = vol - 203; }
					lastvalue[ch].volcmd = m[ch].volcmd;
					lastvalue[ch].vol = m[ch].vol;
				}
			}
			// Reading command/param
			if (chnmask[ch] & 8)
			{
				if (i > len - 2) break;
				UINT cmd = p[i++];
				UINT param = p[i++];
				if (ch < m_nChannels)
				{
					if (cmd)
					{
						m[ch].command = cmd;
						m[ch].param = param;
						S3MConvert(&m[ch], TRUE);
						lastvalue[ch].command = m[ch].command;
						lastvalue[ch].param = m[ch].param;
					}
				}
			}
		}
	}
	for (UINT ncu=0; ncu<MAX_BASECHANNELS; ncu++)
	{
		if (ncu>=m_nChannels)
		{
			ChnSettings[ncu].nVolume = 64;
			ChnSettings[ncu].dwFlags &= ~CHN_MUTE;
		}
	}
	m_nMinPeriod = 8;
	m_nMaxPeriod = 0xF000;
	return TRUE;
}


#ifndef MODPLUG_NO_FILESAVE
//#define SAVEITTIMESTAMP
#ifdef _MSC_VER
#pragma warning(disable:4100)
#endif

static inline UINT ConvertVolParam(UINT value)
//--------------------------------------------
{
	return (value > 9)  ? 9 : value;
}

BOOL CSoundFile::SaveIT(LPCSTR lpszFileName, UINT nPacking)
//---------------------------------------------------------
{
	DWORD dwPatNamLen, dwChnNamLen;
	ITFILEHEADER header, writeheader;
	ITINSTRUMENT iti, writeiti;
	ITSAMPLESTRUCT itss;
	BYTE smpcount[MAX_SAMPLES];
	DWORD inspos[MAX_INSTRUMENTS];
	DWORD patpos[MAX_PATTERNS];
	DWORD smppos[MAX_SAMPLES];
	DWORD dwPos = 0, dwHdrPos = 0, dwExtra = 2;
	WORD patinfo[4];
	BYTE chnmask[64];
	BYTE buf[512];
	MODCOMMAND lastvalue[64];
	FILE *f;


	if ((!lpszFileName) || ((f = fopen(lpszFileName, "wb")) == NULL)) return FALSE;
	memset(inspos, 0, sizeof(inspos));
	memset(patpos, 0, sizeof(patpos));
	memset(smppos, 0, sizeof(smppos));
	// Writing Header
	memset(&header, 0, sizeof(header));
	dwPatNamLen = 0;
	dwChnNamLen = 0;
	header.id = 0x4D504D49; // IMPM
	lstrcpyn((char *)header.songname, m_szNames[0], 27);
	header.reserved1 = 0x1004;
	header.ordnum = 0;
	while ((header.ordnum < MAX_ORDERS) && (Order[header.ordnum] < 0xFF)) header.ordnum++;
	if (header.ordnum < MAX_ORDERS) Order[header.ordnum++] = 0xFF;
	header.insnum = m_nInstruments;
	header.smpnum = m_nSamples;
	header.patnum = MAX_PATTERNS;
	while ((header.patnum > 0) && (!Patterns[header.patnum-1])) header.patnum--;
	header.cwtv = 0x217;
	header.cmwt = 0x200;
	header.flags = 0x0001;
	header.special = 0x0006;
	if (m_nInstruments) header.flags |= 0x04;
	if (m_dwSongFlags & SONG_LINEARSLIDES) header.flags |= 0x08;
	if (m_dwSongFlags & SONG_ITOLDEFFECTS) header.flags |= 0x10;
	if (m_dwSongFlags & SONG_ITCOMPATMODE) header.flags |= 0x20;
	if (m_dwSongFlags & SONG_EXFILTERRANGE) header.flags |= 0x1000;
	header.globalvol = m_nDefaultGlobalVolume >> 1;
	header.mv = m_nSongPreAmp;
	// clip song pre-amp values (between 0x20 and 0x7f)
	if (header.mv < 0x20) header.mv = 0x20;
	if (header.mv > 0x7F) header.mv = 0x7F;
	header.speed = m_nDefaultSpeed;
	header.tempo = m_nDefaultTempo;
	header.sep = m_nStereoSeparation;
	dwHdrPos = sizeof(header) + header.ordnum;
	// Channel Pan and Volume
	memset(header.chnpan, 0xFF, 64);
	memset(header.chnvol, 64, 64);
	for (UINT ich=0; ich<m_nChannels; ich++)
	{
		header.chnpan[ich] = ChnSettings[ich].nPan >> 2;
		if (ChnSettings[ich].dwFlags & CHN_SURROUND) header.chnpan[ich] = 100;
		header.chnvol[ich] = ChnSettings[ich].nVolume;
		if (ChnSettings[ich].dwFlags & CHN_MUTE) header.chnpan[ich] |= 0x80;
		if (ChnSettings[ich].szName[0])
		{
			dwChnNamLen = (ich+1) * MAX_CHANNELNAME;
		}
	}
	if (dwChnNamLen) dwExtra += dwChnNamLen + 8;
#ifdef SAVEITTIMESTAMP
	dwExtra += 8; // Time Stamp
#endif
	if (m_dwSongFlags & SONG_EMBEDMIDICFG)
	{
		header.flags |= 0x80;
		header.special |= 0x08;
		dwExtra += sizeof(MODMIDICFG);
	}
	// Pattern Names
	if ((m_nPatternNames) && (m_lpszPatternNames))
	{
		dwPatNamLen = m_nPatternNames * MAX_PATTERNNAME;
		while ((dwPatNamLen >= MAX_PATTERNNAME) && (!m_lpszPatternNames[dwPatNamLen-MAX_PATTERNNAME])) dwPatNamLen -= MAX_PATTERNNAME;
		if (dwPatNamLen < MAX_PATTERNNAME) dwPatNamLen = 0;
		if (dwPatNamLen) dwExtra += dwPatNamLen + 8;
	}
	// Mix Plugins
	dwExtra += SaveMixPlugins(NULL, TRUE);
	// Comments
	if (m_lpszSongComments)
	{
		header.special |= 1;
		header.msglength = strlen(m_lpszSongComments)+1;
		header.msgoffset = dwHdrPos + dwExtra + header.insnum*4 + header.patnum*4 + header.smpnum*4;
	}
	// Write file header
	memcpy(&writeheader, &header, sizeof(header));

	// Byteswap header information
	writeheader.id = bswapLE32(writeheader.id);
	writeheader.reserved1 = bswapLE16(writeheader.reserved1);
	writeheader.ordnum = bswapLE16(writeheader.ordnum);
	writeheader.insnum = bswapLE16(writeheader.insnum);
	writeheader.smpnum = bswapLE16(writeheader.smpnum);
	writeheader.patnum = bswapLE16(writeheader.patnum);
	writeheader.cwtv = bswapLE16(writeheader.cwtv);
	writeheader.cmwt = bswapLE16(writeheader.cmwt);
	writeheader.flags = bswapLE16(writeheader.flags);
	writeheader.special = bswapLE16(writeheader.special);
	writeheader.msglength = bswapLE16(writeheader.msglength);
	writeheader.msgoffset = bswapLE32(writeheader.msgoffset);
	writeheader.reserved2 = bswapLE32(writeheader.reserved2);

	fwrite(&writeheader, 1, sizeof(writeheader), f);

	fwrite(Order, 1, header.ordnum, f);
	if (header.insnum) fwrite(inspos, 4, header.insnum, f);
	if (header.smpnum) fwrite(smppos, 4, header.smpnum, f);
	if (header.patnum) fwrite(patpos, 4, header.patnum, f);
	// Writing editor history information
	{
#ifdef SAVEITTIMESTAMP
		SYSTEMTIME systime;
		FILETIME filetime;
		WORD timestamp[4];
		WORD nInfoEx = 1;
		memset(timestamp, 0, sizeof(timestamp));
		fwrite(&nInfoEx, 1, 2, f);
		GetSystemTime(&systime);
		SystemTimeToFileTime(&systime, &filetime);
		FileTimeToDosDateTime(&filetime, &timestamp[0], &timestamp[1]);
		fwrite(timestamp, 1, 8, f);
#else
		WORD nInfoEx = 0;
		fwrite(&nInfoEx, 1, 2, f);
#endif
	}
	// Writing midi cfg
	if (header.flags & 0x80)
	{
		fwrite(&m_MidiCfg, 1, sizeof(MODMIDICFG), f);
	}
	// Writing pattern names
	if (dwPatNamLen)
	{
		DWORD d = bswapLE32(0x4d414e50);
		UINT len= bswapLE32(dwPatNamLen);
		fwrite(&d, 1, 4, f);
		fwrite(&len, 1, 4, f);
		fwrite(m_lpszPatternNames, 1, dwPatNamLen, f);
	}
	// Writing channel Names
	if (dwChnNamLen)
	{
		DWORD d = bswapLE32(0x4d414e43);
		UINT len= bswapLE32(dwChnNamLen);
		fwrite(&d, 1, 4, f);
		fwrite(&len, 1, 4, f);
		UINT nChnNames = dwChnNamLen / MAX_CHANNELNAME;
		for (UINT inam=0; inam<nChnNames; inam++)
		{
			fwrite(ChnSettings[inam].szName, 1, MAX_CHANNELNAME, f);
		}
	}
	// Writing mix plugins info
	SaveMixPlugins(f, FALSE);
	// Writing song message
	dwPos = dwHdrPos + dwExtra + (header.insnum + header.smpnum + header.patnum) * 4;
	if (header.special & 1)
	{
		dwPos += strlen(m_lpszSongComments) + 1;
		fwrite(m_lpszSongComments, 1, strlen(m_lpszSongComments)+1, f);
	}
	// Writing instruments
	for (UINT nins=1; nins<=header.insnum; nins++)
	{
		memset(&iti, 0, sizeof(iti));
		iti.id = 0x49504D49;	// "IMPI"
		iti.trkvers = 0x211;
		if (Headers[nins])
		{
			INSTRUMENTHEADER *penv = Headers[nins];
			memset(smpcount, 0, sizeof(smpcount));
			memcpy(iti.filename, penv->filename, 12);
			memcpy(iti.name, penv->name, 26);
			iti.mbank = penv->wMidiBank;
			iti.mpr = penv->nMidiProgram;
			iti.mch = penv->nMidiChannel;
			iti.nna = penv->nNNA;
			iti.dct = penv->nDCT;
			iti.dca = penv->nDNA;
			iti.fadeout = penv->nFadeOut >> 5;
			iti.pps = penv->nPPS;
			iti.ppc = penv->nPPC;
			iti.gbv = (BYTE)(penv->nGlobalVol << 1);
			iti.dfp = (BYTE)penv->nPan >> 2;
			if (!(penv->dwFlags & ENV_SETPANNING)) iti.dfp |= 0x80;
			iti.rv = penv->nVolSwing;
			iti.rp = penv->nPanSwing;
			iti.ifc = penv->nIFC;
			iti.ifr = penv->nIFR;
			iti.nos = 0;
			for (UINT i=0; i<NOTE_MAX; i++) if (penv->Keyboard[i] < MAX_SAMPLES)
			{
				UINT smp = penv->Keyboard[i];
				if ((smp) && (!smpcount[smp]))
				{
					smpcount[smp] = 1;
					iti.nos++;
				}
				iti.keyboard[i*2] = penv->NoteMap[i] - 1;
				iti.keyboard[i*2+1] = smp;
			}
			// Writing Volume envelope
			if (penv->dwFlags & ENV_VOLUME) iti.volenv.flags |= 0x01;
			if (penv->dwFlags & ENV_VOLLOOP) iti.volenv.flags |= 0x02;
			if (penv->dwFlags & ENV_VOLSUSTAIN) iti.volenv.flags |= 0x04;
			if (penv->dwFlags & ENV_VOLCARRY) iti.volenv.flags |= 0x08;
			iti.volenv.num = (BYTE)penv->nVolEnv;
			iti.volenv.lpb = (BYTE)penv->nVolLoopStart;
			iti.volenv.lpe = (BYTE)penv->nVolLoopEnd;
			iti.volenv.slb = penv->nVolSustainBegin;
			iti.volenv.sle = penv->nVolSustainEnd;
			// Writing Panning envelope
			if (penv->dwFlags & ENV_PANNING) iti.panenv.flags |= 0x01;
			if (penv->dwFlags & ENV_PANLOOP) iti.panenv.flags |= 0x02;
			if (penv->dwFlags & ENV_PANSUSTAIN) iti.panenv.flags |= 0x04;
			if (penv->dwFlags & ENV_PANCARRY) iti.panenv.flags |= 0x08;
			iti.panenv.num = (BYTE)penv->nPanEnv;
			iti.panenv.lpb = (BYTE)penv->nPanLoopStart;
			iti.panenv.lpe = (BYTE)penv->nPanLoopEnd;
			iti.panenv.slb = penv->nPanSustainBegin;
			iti.panenv.sle = penv->nPanSustainEnd;
			// Writing Pitch Envelope
			if (penv->dwFlags & ENV_PITCH) iti.pitchenv.flags |= 0x01;
			if (penv->dwFlags & ENV_PITCHLOOP) iti.pitchenv.flags |= 0x02;
			if (penv->dwFlags & ENV_PITCHSUSTAIN) iti.pitchenv.flags |= 0x04;
			if (penv->dwFlags & ENV_PITCHCARRY) iti.pitchenv.flags |= 0x08;
			if (penv->dwFlags & ENV_FILTER) iti.pitchenv.flags |= 0x80;
			iti.pitchenv.num = (BYTE)penv->nPitchEnv;
			iti.pitchenv.lpb = (BYTE)penv->nPitchLoopStart;
			iti.pitchenv.lpe = (BYTE)penv->nPitchLoopEnd;
			iti.pitchenv.slb = (BYTE)penv->nPitchSustainBegin;
			iti.pitchenv.sle = (BYTE)penv->nPitchSustainEnd;
			// Writing Envelopes data
			for (UINT ev=0; ev<25; ev++)
			{
				iti.volenv.data[ev*3] = penv->VolEnv[ev];
				iti.volenv.data[ev*3+1] = penv->VolPoints[ev] & 0xFF;
				iti.volenv.data[ev*3+2] = penv->VolPoints[ev] >> 8;
				iti.panenv.data[ev*3] = penv->PanEnv[ev] - 32;
				iti.panenv.data[ev*3+1] = penv->PanPoints[ev] & 0xFF;
				iti.panenv.data[ev*3+2] = penv->PanPoints[ev] >> 8;
				iti.pitchenv.data[ev*3] = penv->PitchEnv[ev] - 32;
				iti.pitchenv.data[ev*3+1] = penv->PitchPoints[ev] & 0xFF;
				iti.pitchenv.data[ev*3+2] = penv->PitchPoints[ev] >> 8;
			}
		} else
		// Save Empty Instrument
		{
			for (UINT i=0; i<NOTE_MAX; i++) iti.keyboard[i*2] = i;
			iti.ppc = 5*12;
			iti.gbv = 128;
			iti.dfp = 0x20;
			iti.ifc = 0xFF;
		}
		if (!iti.nos) iti.trkvers = 0;
		// Writing instrument
		inspos[nins-1] = dwPos;
		dwPos += sizeof(ITINSTRUMENT);

		memcpy(&writeiti, &iti, sizeof(ITINSTRUMENT));

		writeiti.fadeout = bswapLE16(writeiti.fadeout);
		writeiti.id = bswapLE32(writeiti.id);
		writeiti.trkvers = bswapLE16(writeiti.trkvers);
		writeiti.mbank = bswapLE16(writeiti.mbank);

		fwrite(&writeiti, 1, sizeof(ITINSTRUMENT), f);
	}
	// Writing sample headers
	memset(&itss, 0, sizeof(itss));
	for (UINT hsmp=0; hsmp<header.smpnum; hsmp++)
	{
		smppos[hsmp] = dwPos;
		dwPos += sizeof(ITSAMPLESTRUCT);
		fwrite(&itss, 1, sizeof(ITSAMPLESTRUCT), f);
	}
	// Writing Patterns
	for (UINT npat=0; npat<header.patnum; npat++)
	{
		DWORD dwPatPos = dwPos;
		UINT len;
		if (!Patterns[npat]) continue;
		patpos[npat] = dwPos;
		patinfo[0] = 0;
		patinfo[1] = bswapLE16(PatternSize[npat]);
		patinfo[2] = 0;
		patinfo[3] = 0;
		// Check for empty pattern
		if (PatternSize[npat] == 64)
		{
			MODCOMMAND *pzc = Patterns[npat];
			UINT iz, nz = PatternSize[npat] * m_nChannels;
			for (iz=0; iz<nz; iz++)
			{
				if ((pzc[iz].note) || (pzc[iz].instr)
				 || (pzc[iz].volcmd) || (pzc[iz].command)) break;
			}
			if (iz == nz)
			{
				patpos[npat] = 0;
				continue;
			}
		}
		fwrite(patinfo, 8, 1, f);
		dwPos += 8;
		memset(chnmask, 0xFF, sizeof(chnmask));
		memset(lastvalue, 0, sizeof(lastvalue));
		MODCOMMAND *m = Patterns[npat];
		for (UINT row=0; row<PatternSize[npat]; row++)
		{
			len = 0;
			for (UINT ch=0; ch<m_nChannels; ch++, m++)
			{
				BYTE b = 0;
				UINT command = m->command;
				UINT param = m->param;
				UINT vol = 0xFF;
				UINT note = m->note;
				if (note) b |= 1;
				if ((note) && (note < 0x80)) note--; // 0xfe->0x80 --Toad
				if (m->instr) b |= 2;
				if (m->volcmd)
				{
					UINT volcmd = m->volcmd;
					switch(volcmd)
					{
					case VOLCMD_VOLUME:			vol = m->vol; if (vol > 64) vol = 64; break;
					case VOLCMD_PANNING:		vol = m->vol + 128; if (vol > 192) vol = 192; break;
					case VOLCMD_VOLSLIDEUP:		vol = 85 + ConvertVolParam(m->vol); break;
					case VOLCMD_VOLSLIDEDOWN:	vol = 95 + ConvertVolParam(m->vol); break;
					case VOLCMD_FINEVOLUP:		vol = 65 + ConvertVolParam(m->vol); break;
					case VOLCMD_FINEVOLDOWN:	vol = 75 + ConvertVolParam(m->vol); break;
					case VOLCMD_VIBRATOSPEED:	vol = 203 + ConvertVolParam(m->vol); break;
					case VOLCMD_VIBRATO:		vol = 203; break;
					case VOLCMD_TONEPORTAMENTO:	vol = 193 + ConvertVolParam(m->vol); break;
					case VOLCMD_PORTADOWN:		vol = 105 + ConvertVolParam(m->vol); break;
					case VOLCMD_PORTAUP:		vol = 115 + ConvertVolParam(m->vol); break;
					default:					vol = 0xFF;
					}
				}
				if (vol != 0xFF) b |= 4;
				if (command)
				{
					S3MSaveConvert(&command, &param, TRUE);
					if (command) b |= 8;
				}
				// Packing information
				if (b)
				{
					// Same note ?
					if (b & 1)
					{
						if ((note == lastvalue[ch].note) && (lastvalue[ch].volcmd & 1))
						{
							b &= ~1;
							b |= 0x10;
						} else
						{
							lastvalue[ch].note = note;
							lastvalue[ch].volcmd |= 1;
						}
					}
					// Same instrument ?
					if (b & 2)
					{
						if ((m->instr == lastvalue[ch].instr) && (lastvalue[ch].volcmd & 2))
						{
							b &= ~2;
							b |= 0x20;
						} else
						{
							lastvalue[ch].instr = m->instr;
							lastvalue[ch].volcmd |= 2;
						}
					}
					// Same volume column byte ?
					if (b & 4)
					{
						if ((vol == lastvalue[ch].vol) && (lastvalue[ch].volcmd & 4))
						{
							b &= ~4;
							b |= 0x40;
						} else
						{
							lastvalue[ch].vol = vol;
							lastvalue[ch].volcmd |= 4;
						}
					}
					// Same command / param ?
					if (b & 8)
					{
						if ((command == lastvalue[ch].command) && (param == lastvalue[ch].param) && (lastvalue[ch].volcmd & 8))
						{
							b &= ~8;
							b |= 0x80;
						} else
						{
							lastvalue[ch].command = command;
							lastvalue[ch].param = param;
							lastvalue[ch].volcmd |= 8;
						}
					}
					if (b != chnmask[ch])
					{
						chnmask[ch] = b;
						buf[len++] = (ch+1) | 0x80;
						buf[len++] = b;
					} else
					{
						buf[len++] = ch+1;
					}
					if (b & 1) buf[len++] = note;
					if (b & 2) buf[len++] = m->instr;
					if (b & 4) buf[len++] = vol;
					if (b & 8)
					{
						buf[len++] = command;
						buf[len++] = param;
					}
				}
			}
			buf[len++] = 0;
			dwPos += len;
			patinfo[0] += len;
			fwrite(buf, 1, len, f);
		}
		fseek(f, dwPatPos, SEEK_SET);
		patinfo[0] = bswapLE16(patinfo[0]); // byteswap -- Toad
		fwrite(patinfo, 8, 1, f);
		fseek(f, dwPos, SEEK_SET);
	}
	// Writing Sample Data
	for (UINT nsmp=1; nsmp<=header.smpnum; nsmp++)
	{
		MODINSTRUMENT *psmp = &Ins[nsmp];
		memset(&itss, 0, sizeof(itss));
		memcpy(itss.filename, psmp->name, 12);
		memcpy(itss.name, m_szNames[nsmp], 26);
		itss.id = 0x53504D49;
		itss.gvl = (BYTE)psmp->nGlobalVol;
		if (m_nInstruments)
		{
			for (UINT iu=1; iu<=m_nInstruments; iu++) if (Headers[iu])
			{
				INSTRUMENTHEADER *penv = Headers[iu];
				for (UINT ju=0; ju<128; ju++) if (penv->Keyboard[ju] == nsmp)
				{
					itss.flags = 0x01;
					break;
				}
			}
		} else
		{
			itss.flags = 0x01;
		}
		if (psmp->uFlags & CHN_LOOP) itss.flags |= 0x10;
		if (psmp->uFlags & CHN_SUSTAINLOOP) itss.flags |= 0x20;
		if (psmp->uFlags & CHN_PINGPONGLOOP) itss.flags |= 0x40;
		if (psmp->uFlags & CHN_PINGPONGSUSTAIN) itss.flags |= 0x80;
		itss.C5Speed = psmp->nC4Speed;
		if (!itss.C5Speed) // if no C5Speed assume it is XM Sample
		{
			UINT period;

			/**
			 * C5 note => number 61, but in XM samples:
			 * RealNote = Note + RelativeTone
			 */
			period = GetPeriodFromNote(61+psmp->RelativeTone, psmp->nFineTune, 0);

			if (period)
				itss.C5Speed = GetFreqFromPeriod(period, 0, 0);
			/**
			 * If it didn`t work, it may not be a XM file;
			 * so put the default C5Speed, 8363Hz.
			 */
	 		if (!itss.C5Speed) itss.C5Speed = 8363;
		}

		itss.length = psmp->nLength;
		itss.loopbegin = psmp->nLoopStart;
		itss.loopend = psmp->nLoopEnd;
		itss.susloopbegin = psmp->nSustainStart;
		itss.susloopend = psmp->nSustainEnd;
		itss.vol = psmp->nVolume >> 2;
		itss.dfp = psmp->nPan >> 2;
		itss.vit = autovibxm2it[psmp->nVibType & 7];
		itss.vis = psmp->nVibRate;
		itss.vid = psmp->nVibDepth;
		itss.vir = (psmp->nVibSweep < 64) ? psmp->nVibSweep * 4 : 255;
		if (psmp->uFlags & CHN_PANNING) itss.dfp |= 0x80;
		if ((psmp->pSample) && (psmp->nLength)) itss.cvt = 0x01;
		UINT flags = RS_PCM8S;
#ifndef NO_PACKING
		if (nPacking)
		{
			if ((!(psmp->uFlags & (CHN_16BIT|CHN_STEREO)))
			 && (CanPackSample((char *)psmp->pSample, psmp->nLength, nPacking)))
			{
				flags = RS_ADPCM4;
				itss.cvt = 0xFF;
			}
		} else
#endif // NO_PACKING
		{
			if (psmp->uFlags & CHN_STEREO)
			{
				flags = RS_STPCM8S;
				itss.flags |= 0x04;
			}
			if (psmp->uFlags & CHN_16BIT)
			{
				itss.flags |= 0x02;
				flags = (psmp->uFlags & CHN_STEREO) ? RS_STPCM16S : RS_PCM16S;
			}
		}
		itss.samplepointer = dwPos;
		fseek(f, smppos[nsmp-1], SEEK_SET);

		itss.id = bswapLE32(itss.id);
		itss.length = bswapLE32(itss.length);
		itss.loopbegin = bswapLE32(itss.loopbegin);
		itss.loopend = bswapLE32(itss.loopend);
		itss.C5Speed = bswapLE32(itss.C5Speed);
		itss.susloopbegin = bswapLE32(itss.susloopbegin);
		itss.susloopend = bswapLE32(itss.susloopend);
		itss.samplepointer = bswapLE32(itss.samplepointer);

		fwrite(&itss, 1, sizeof(ITSAMPLESTRUCT), f);
		fseek(f, dwPos, SEEK_SET);
		if ((psmp->pSample) && (psmp->nLength))
		{
			dwPos += WriteSample(f, psmp, flags);
		}
	}
	// Updating offsets
	fseek(f, dwHdrPos, SEEK_SET);

	/* <Toad> Now we can byteswap them ;-) */
	UINT WW;
	UINT WX;
	WX = (UINT)header.insnum;
	WX <<= 2;
	for (WW=0; WW < (WX>>2); WW++)
	       inspos[WW] = bswapLE32(inspos[WW]);

	WX = (UINT)header.smpnum;
	WX <<= 2;
	for (WW=0; WW < (WX>>2); WW++)
	       smppos[WW] = bswapLE32(smppos[WW]);

	WX=(UINT)header.patnum;
	WX <<= 2;
	for (WW=0; WW < (WX>>2); WW++)
	       patpos[WW] = bswapLE32(patpos[WW]);

	if (header.insnum) fwrite(inspos, 4, header.insnum, f);
	if (header.smpnum) fwrite(smppos, 4, header.smpnum, f);
	if (header.patnum) fwrite(patpos, 4, header.patnum, f);
	fclose(f);
	return TRUE;
}

#ifdef _MSC_VER
//#pragma warning(default:4100)
#endif
#endif // MODPLUG_NO_FILESAVE

//////////////////////////////////////////////////////////////////////////////
// IT 2.14 compression

DWORD ITReadBits(DWORD &bitbuf, UINT &bitnum, LPBYTE &ibuf, LPBYTE ibufend, CHAR n)
//-----------------------------------------------------------------
{
	DWORD retval = 0;
	UINT i = n;

	// explicit if read 0 bits, then return 0
	if (i == 0)
		return(0);

	if (n > 0)
	{
		do
		{
			if (!bitnum)
			{
				if (ibuf >= ibufend)
					return 0;

				bitbuf = *ibuf++;
				bitnum = 8;
			}
			retval >>= 1;
			retval |= bitbuf << 31;
			bitbuf >>= 1;
			bitnum--;
			i--;
		} while (i);
		i = n;
	}
	return (retval >> (32-i));
}

#define IT215_SUPPORT
DWORD ITUnpack8Bit(signed char *pSample, DWORD dwLen, LPBYTE lpMemFile, DWORD dwMemLength, DWORD channels, BOOL b215)
//-------------------------------------------------------------------------------------------------------------------
{
	signed char *pDst = pSample;
	LPBYTE pSrc = lpMemFile;
	DWORD writePos = 0;
	LPBYTE pStop = lpMemFile + dwMemLength;
//	DWORD wHdr = 0;
	DWORD wCount = 0;
	DWORD bitbuf = 0;
	UINT bitnum = 0;
	BYTE bLeft = 0, bTemp = 0, bTemp2 = 0;


	while (dwLen)
	{
		if (!wCount)
		{
			wCount = 0x8000;
		//	wHdr = bswapLE16(*((LPWORD)pSrc));
			pSrc += 2;
			bLeft = 9;
			bTemp = bTemp2 = 0;
			bitbuf = bitnum = 0;
		}

		DWORD d = wCount;
		if (d > dwLen) d = dwLen;
		// Unpacking
		DWORD dwPos = 0;
		do
		{
			WORD wBits = (WORD)ITReadBits(bitbuf, bitnum, pSrc, pStop, bLeft);
			if (bLeft < 7)
			{
				DWORD i = 1 << (bLeft-1);
				DWORD j = wBits & 0xFFFF;
				if (i != j) goto UnpackByte;
				wBits = (WORD)(ITReadBits(bitbuf, bitnum, pSrc, pStop, 3) + 1) & 0xFF;
				bLeft = ((BYTE)wBits < bLeft) ? (BYTE)wBits : (BYTE)((wBits+1) & 0xFF);
				goto Next;
			}
			if (bLeft < 9)
			{
				WORD i = (0xFF >> (9 - bLeft)) + 4;
				WORD j = i - 8;
				if ((wBits <= j) || (wBits > i)) goto UnpackByte;
				wBits -= j;
				bLeft = ((BYTE)(wBits & 0xFF) < bLeft) ? (BYTE)(wBits & 0xFF) : (BYTE)((wBits+1) & 0xFF);
				goto Next;
			}
			if (bLeft >= 10) goto SkipByte;
			if (wBits >= 256)
			{
				bLeft = (BYTE)(wBits + 1) & 0xFF;
				goto Next;
			}
		UnpackByte:
			if (bLeft < 8)
			{
				BYTE shift = 8 - bLeft;
				signed char c = (signed char)(wBits << shift);
				c >>= shift;
				wBits = (WORD)c;
			}
			wBits += bTemp;
			bTemp = (BYTE)wBits;
			bTemp2 += bTemp;
#ifdef IT215_SUPPORT
			pDst[writePos] = (b215) ? bTemp2 : bTemp;
#else
			pDst[writePos] = bTemp;
#endif
		SkipByte:
			dwPos++;
			writePos += channels;
		Next:
			if (pSrc >= pStop + 1) return (DWORD)(pSrc - lpMemFile);
		} while (dwPos < d);
		// Move On
		wCount -= d;
		dwLen -= d;
	}
	return (DWORD)(pSrc - lpMemFile);
}


DWORD ITUnpack16Bit(signed char *pSample, DWORD dwLen, LPBYTE lpMemFile, DWORD dwMemLength, DWORD channels, BOOL b215)
//--------------------------------------------------------------------------------------------------------------------
{
	signed short *pDst = (signed short *)pSample;
	LPBYTE pSrc = lpMemFile;
	DWORD writePos = 0;
	LPBYTE pStop = lpMemFile + dwMemLength;
//	DWORD wHdr = 0;
	DWORD wCount = 0;
	DWORD bitbuf = 0;
	UINT bitnum = 0;
	BYTE bLeft = 0;
	signed short wTemp = 0, wTemp2 = 0;

	while (dwLen)
	{
		if (!wCount)
		{
			wCount = 0x4000;
		//	wHdr = bswapLE16(*((LPWORD)pSrc));
			pSrc += 2;
			bLeft = 17;
			wTemp = wTemp2 = 0;
			bitbuf = bitnum = 0;
		}
		DWORD d = wCount;
		if (d > dwLen) d = dwLen;
		// Unpacking
		DWORD dwPos = 0;
		do
		{
			DWORD dwBits = ITReadBits(bitbuf, bitnum, pSrc, pStop, bLeft);
			if (bLeft < 7)
			{
				DWORD i = 1 << (bLeft-1);
				DWORD j = dwBits;
				if (i != j) goto UnpackByte;
				dwBits = ITReadBits(bitbuf, bitnum, pSrc, pStop, 4) + 1;
				bLeft = ((BYTE)(dwBits & 0xFF) < bLeft) ? (BYTE)(dwBits & 0xFF) : (BYTE)((dwBits+1) & 0xFF);
				goto Next;
			}
			if (bLeft < 17)
			{
				DWORD i = (0xFFFF >> (17 - bLeft)) + 8;
				DWORD j = (i - 16) & 0xFFFF;
				if ((dwBits <= j) || (dwBits > (i & 0xFFFF))) goto UnpackByte;
				dwBits -= j;
				bLeft = ((BYTE)(dwBits & 0xFF) < bLeft) ? (BYTE)(dwBits & 0xFF) : (BYTE)((dwBits+1) & 0xFF);
				goto Next;
			}
			if (bLeft >= 18) goto SkipByte;
			if (dwBits >= 0x10000)
			{
				bLeft = (BYTE)(dwBits + 1) & 0xFF;
				goto Next;
			}
		UnpackByte:
			if (bLeft < 16)
			{
				BYTE shift = 16 - bLeft;
				signed short c = (signed short)(dwBits << shift);
				c >>= shift;
				dwBits = (DWORD)c;
			}
			dwBits += wTemp;
			wTemp = (signed short)dwBits;
			wTemp2 += wTemp;
#ifdef IT215_SUPPORT
			pDst[writePos] = (b215) ? wTemp2 : wTemp;
#else
			pDst[writePos] = wTemp;
#endif
		SkipByte:
			dwPos++;
			writePos += channels;
		Next:
			if (pSrc >= pStop + 1) return (DWORD)(pSrc - lpMemFile);
		} while (dwPos < d);
		// Move On
		wCount -= d;
		dwLen -= d;
		if (pSrc >= pStop) break;
	}
	return (DWORD)(pSrc - lpMemFile);
}


UINT CSoundFile::SaveMixPlugins(FILE *f, BOOL bUpdate)
//----------------------------------------------------
{
	DWORD chinfo[64];
	CHAR s[32];
	DWORD nPluginSize, writeSwapDWORD;
	SNDMIXPLUGININFO writePluginInfo;
	UINT nTotalSize = 0;
	UINT nChInfo = 0;

	for (UINT i=0; i<MAX_MIXPLUGINS; i++)
	{
		PSNDMIXPLUGIN p = &m_MixPlugins[i];
		if ((p->Info.dwPluginId1) || (p->Info.dwPluginId2))
		{
			nPluginSize = sizeof(SNDMIXPLUGININFO)+4; // plugininfo+4 (datalen)
			if ((p->pMixPlugin) && (bUpdate))
			{
				p->pMixPlugin->SaveAllParameters();
			}
			if (p->pPluginData)
			{
				nPluginSize += p->nPluginDataSize;
			}
			if (f)
			{
				s[0] = 'F';
				s[1] = 'X';
				s[2] = '0' + (i/10);
				s[3] = '0' + (i%10);
				fwrite(s, 1, 4, f);
				writeSwapDWORD = bswapLE32(nPluginSize);
				fwrite(&writeSwapDWORD, 1, 4, f);

				// Copy Information To Be Written for ByteSwapping
				memcpy(&writePluginInfo, &p->Info, sizeof(SNDMIXPLUGININFO));
				writePluginInfo.dwPluginId1 = bswapLE32(p->Info.dwPluginId1);
				writePluginInfo.dwPluginId2 = bswapLE32(p->Info.dwPluginId2);
				writePluginInfo.dwInputRouting = bswapLE32(p->Info.dwInputRouting);
				writePluginInfo.dwOutputRouting = bswapLE32(p->Info.dwOutputRouting);
				for (UINT j=0; j<4; j++) {
				    writePluginInfo.dwReserved[j] = bswapLE32(p->Info.dwReserved[j]);
				}

				fwrite(&writePluginInfo, 1, sizeof(SNDMIXPLUGININFO), f);
				writeSwapDWORD = bswapLE32(m_MixPlugins[i].nPluginDataSize);
				fwrite(&writeSwapDWORD, 1, 4, f);
				if (m_MixPlugins[i].pPluginData)
				{
					fwrite(m_MixPlugins[i].pPluginData, 1, m_MixPlugins[i].nPluginDataSize, f);
				}
			}
			nTotalSize += nPluginSize + 8;
		}
	}
	for (UINT j=0; j<m_nChannels; j++)
	{
		if (j < 64)
		{
			if ((chinfo[j] = ChnSettings[j].nMixPlugin) != 0)
			{
				nChInfo = j+1;
				chinfo[j] = bswapLE32(chinfo[j]); // inplace BS
			}
		}
	}
	if (nChInfo)
	{
		if (f)
		{
			nPluginSize = bswapLE32(0x58464843);
			fwrite(&nPluginSize, 1, 4, f);
			nPluginSize = nChInfo*4;
			writeSwapDWORD = bswapLE32(nPluginSize);
			fwrite(&writeSwapDWORD, 1, 4, f);
			fwrite(chinfo, 1, nPluginSize, f);
		}
		nTotalSize += nChInfo*4 + 8;
	}
	return nTotalSize;
}


UINT CSoundFile::LoadMixPlugins(const void *pData, UINT nLen)
//-----------------------------------------------------------
{
	const BYTE *p = (const BYTE *)pData;
	UINT nPos = 0;

	while (nPos+8 < nLen)
	{
		DWORD nPluginSize;
		UINT nPlugin;

		nPluginSize = bswapLE32(*(DWORD *)(p+nPos+4));
		if (nPluginSize > nLen-nPos-8) break;;
		if ((bswapLE32(*(DWORD *)(p+nPos))) == 0x58464843)
		{
			for (UINT ch=0; ch<64; ch++) if (ch*4 < nPluginSize)
			{
				ChnSettings[ch].nMixPlugin = bswapLE32(*(DWORD *)(p+nPos+8+ch*4));
			}
		} else
		{
			if ((p[nPos] != 'F') || (p[nPos+1] != 'X')
			 || (p[nPos+2] < '0') || (p[nPos+3] < '0'))
			{
				break;
			}
			nPlugin = (p[nPos+2]-'0')*10 + (p[nPos+3]-'0');
			if ((nPlugin < MAX_MIXPLUGINS) && (nPluginSize >= sizeof(SNDMIXPLUGININFO)+4))
			{
				DWORD dwExtra = bswapLE32(*(DWORD *)(p+nPos+8+sizeof(SNDMIXPLUGININFO)));
				m_MixPlugins[nPlugin].Info = *(const SNDMIXPLUGININFO *)(p+nPos+8);
				m_MixPlugins[nPlugin].Info.dwPluginId1 = bswapLE32(m_MixPlugins[nPlugin].Info.dwPluginId1);
				m_MixPlugins[nPlugin].Info.dwPluginId2 = bswapLE32(m_MixPlugins[nPlugin].Info.dwPluginId2);
				m_MixPlugins[nPlugin].Info.dwInputRouting = bswapLE32(m_MixPlugins[nPlugin].Info.dwInputRouting);
				m_MixPlugins[nPlugin].Info.dwOutputRouting = bswapLE32(m_MixPlugins[nPlugin].Info.dwOutputRouting);
				for (UINT j=0; j<4; j++) {
				    m_MixPlugins[nPlugin].Info.dwReserved[j] = bswapLE32(m_MixPlugins[nPlugin].Info.dwReserved[j]);
				}
				if ((dwExtra) && (dwExtra <= nPluginSize-sizeof(SNDMIXPLUGININFO)-4))
				{
					m_MixPlugins[nPlugin].nPluginDataSize = 0;
					m_MixPlugins[nPlugin].pPluginData = new signed char [dwExtra];
					if (m_MixPlugins[nPlugin].pPluginData)
					{
						m_MixPlugins[nPlugin].nPluginDataSize = dwExtra;
						memcpy(m_MixPlugins[nPlugin].pPluginData, p+nPos+8+sizeof(SNDMIXPLUGININFO)+4, dwExtra);
					}
				}
			}
		}
		nPos += nPluginSize + 8;
	}
	return nPos;
}
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/


///////////////////////////////////////////////////
//
// J2B module loader
//
///////////////////////////////////////////////////
//#include "stdafx.h"
//#include "sndfile.h"

/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

//////////////////////////////////////////////
// DigiTracker (MDL) module loader          //
//////////////////////////////////////////////
//#include "stdafx.h"
//#include "sndfile.h"

//#pragma warning(disable:4244)

typedef struct MDLSONGHEADER
{
	DWORD id;	// "DMDL" = 0x4C444D44
	BYTE version;
} MDLSONGHEADER;


typedef struct MDLINFOBLOCK
{
	CHAR songname[32];
	CHAR composer[20];
	WORD norders;
	WORD repeatpos;
	BYTE globalvol;
	BYTE speed;
	BYTE tempo;
	BYTE channelinfo[32];
	BYTE seq[256];
} MDLINFOBLOCK;


typedef struct MDLPATTERNDATA
{
	BYTE channels;
	BYTE lastrow;	// nrows = lastrow+1
	CHAR name[16];
	WORD data[1];
} MDLPATTERNDATA;


void ConvertMDLCommand(MODCOMMAND *m, UINT eff, UINT data)
//--------------------------------------------------------
{
	UINT command = 0, param = data;
	switch(eff)
	{
	case 0x01:	command = CMD_PORTAMENTOUP; break;
	case 0x02:	command = CMD_PORTAMENTODOWN; break;
	case 0x03:	command = CMD_TONEPORTAMENTO; break;
	case 0x04:	command = CMD_VIBRATO; break;
	case 0x05:	command = CMD_ARPEGGIO; break;
	case 0x07:	command = (param < 0x20) ? CMD_SPEED : CMD_TEMPO; break;
	case 0x08:	command = CMD_PANNING8; param <<= 1; break;
	case 0x0B:	command = CMD_POSITIONJUMP; break;
	case 0x0C:	command = CMD_GLOBALVOLUME; break;
	case 0x0D:	command = CMD_PATTERNBREAK; param = (data & 0x0F) + (data>>4)*10; break;
	case 0x0E:
		command = CMD_S3MCMDEX;
		switch(data & 0xF0)
		{
		case 0x00:	command = 0; break; // What is E0x in MDL (there is a bunch) ?
		case 0x10:	if (param & 0x0F) { param |= 0xF0; command = CMD_PANNINGSLIDE; } else command = 0; break;
		case 0x20:	if (param & 0x0F) { param = (param << 4) | 0x0F; command = CMD_PANNINGSLIDE; } else command = 0; break;
		case 0x30:	param = (data & 0x0F) | 0x10; break; // glissando
		case 0x40:	param = (data & 0x0F) | 0x30; break; // vibrato waveform
		case 0x60:	param = (data & 0x0F) | 0xB0; break;
		case 0x70:	param = (data & 0x0F) | 0x40; break; // tremolo waveform
		case 0x90:	command = CMD_RETRIG; param &= 0x0F; break;
		case 0xA0:	param = (data & 0x0F) << 4; command = CMD_GLOBALVOLSLIDE; break;
		case 0xB0:	param = data & 0x0F; command = CMD_GLOBALVOLSLIDE; break;
		case 0xF0:	param = ((data >> 8) & 0x0F) | 0xA0; break;
		}
		break;
	case 0x0F:	command = CMD_SPEED; break;
	case 0x10:	if ((param & 0xF0) != 0xE0) { command = CMD_VOLUMESLIDE; if ((param & 0xF0) == 0xF0) param = ((param << 4) | 0x0F); else param >>= 2; } break;
	case 0x20:	if ((param & 0xF0) != 0xE0) { command = CMD_VOLUMESLIDE; if ((param & 0xF0) != 0xF0) param >>= 2; } break;
	case 0x30:	command = CMD_RETRIG; break;
	case 0x40:	command = CMD_TREMOLO; break;
	case 0x50:	command = CMD_TREMOR; break;
	case 0xEF:	if (param > 0xFF) param = 0xFF; command = CMD_OFFSET; break;
	}
	if (command)
	{
		m->command = command;
		m->param = param;
	}
}


void UnpackMDLTrack(MODCOMMAND *pat, UINT nChannels, UINT nRows, UINT nTrack, const BYTE *lpTracks, UINT len)
//-------------------------------------------------------------------------------------------------
{
	MODCOMMAND cmd, *m = pat;
	UINT pos = 0, row = 0, i;

	cmd.note = cmd.instr = 0;
	cmd.volcmd = cmd.vol = 0;
	cmd.command = cmd.param = 0;
	while ((row < nRows) && (pos < len))
	{
		UINT xx;
		BYTE b = lpTracks[pos++];
		xx = b >> 2;
		switch(b & 0x03)
		{
		case 0x01:
			for (i=0; i<=xx; i++)
			{
				if (row) *m = *(m-nChannels);
				m += nChannels;
				row++;
				if (row >= nRows) break;
			}
			break;

		case 0x02:
			if (xx < row) *m = pat[nChannels*xx];
			m += nChannels;
			row++;
			break;

		case 0x03:
			{
				cmd.note = (xx & 0x01) ? (pos < len ? lpTracks[pos++] : 0) : 0;
				cmd.instr = (xx & 0x02) ? (pos < len ? lpTracks[pos++] : 0) : 0;
				cmd.volcmd = cmd.vol = 0;
				cmd.command = cmd.param = 0;
				if ((cmd.note < NOTE_MAX-12) && (cmd.note)) cmd.note += 12;
				UINT volume = (xx & 0x04) ? (pos < len ? lpTracks[pos++] : 0) : 0;
				UINT commands = (xx & 0x08) ? (pos < len ? lpTracks[pos++] : 0) : 0;
				UINT command1 = commands & 0x0F;
				UINT command2 = commands & 0xF0;
				UINT param1 = (xx & 0x10) ? (pos < len ? lpTracks[pos++] : 0) : 0;
				UINT param2 = (xx & 0x20) ? (pos < len ? lpTracks[pos++] : 0) : 0;
				if ((command1 == 0x0E) && ((param1 & 0xF0) == 0xF0) && (!command2))
				{
					param1 = ((param1 & 0x0F) << 8) | param2;
					command1 = 0xEF;
					command2 = param2 = 0;
				}
				if (volume)
				{
					cmd.volcmd = VOLCMD_VOLUME;
					cmd.vol = (volume+1) >> 2;
				}
				ConvertMDLCommand(&cmd, command1, param1);
				if ((cmd.command != CMD_SPEED)
				 && (cmd.command != CMD_TEMPO)
				 && (cmd.command != CMD_PATTERNBREAK))
					ConvertMDLCommand(&cmd, command2, param2);
				*m = cmd;
				m += nChannels;
				row++;
			}
			break;

		// Empty Slots
		default:
			row += xx+1;
			m += (xx+1)*nChannels;
			if (row >= nRows) break;
		}
	}
}



BOOL CSoundFile::ReadMDL(const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
	DWORD dwMemPos, dwPos, blocklen, dwTrackPos;
	const MDLSONGHEADER *pmsh = (const MDLSONGHEADER *)lpStream;
	const MDLINFOBLOCK *pmib;
	UINT i,j, norders = 0, npatterns = 0, ntracks = 0;
	UINT ninstruments = 0, nsamples = 0;
	WORD block;
	WORD patterntracks[MAX_PATTERNS*32];
	BYTE smpinfo[MAX_SAMPLES];
	BYTE insvolenv[MAX_INSTRUMENTS];
	BYTE inspanenv[MAX_INSTRUMENTS];
	LPCBYTE pvolenv, ppanenv, ppitchenv;
	UINT nvolenv, npanenv, npitchenv;

	if ((!lpStream) || (dwMemLength < 1024)) return FALSE;
	if ((pmsh->id != 0x4C444D44) || ((pmsh->version & 0xF0) > 0x10)) return FALSE;
	const UINT hdrLen = (pmsh->version>0)? 59 : 57;
	memset(patterntracks, 0, sizeof(patterntracks));
	memset(smpinfo, 0, sizeof(smpinfo));
	memset(insvolenv, 0, sizeof(insvolenv));
	memset(inspanenv, 0, sizeof(inspanenv));
	dwMemPos = 5;
	dwTrackPos = 0;
	pvolenv = ppanenv = ppitchenv = NULL;
	nvolenv = npanenv = npitchenv = 0;
	m_nSamples = m_nInstruments = 0;
	while (dwMemPos+6 < dwMemLength)
	{
		const BYTE *pp = lpStream + dwMemPos;
		block = pp[0] | (pp[1] << 8);
		blocklen = pp[2] | (pp[3] << 8) | (pp[4] << 16) | (pp[5] << 24);
		dwMemPos += 6;
		if (blocklen > dwMemLength - dwMemPos)
		{
			if (dwMemPos == 11) return FALSE;
			break;
		}
		switch(block)
		{
		// IN: infoblock
		case 0x4E49:
			if (blocklen < sizeof(MDLINFOBLOCK)) break;
			pmib = (const MDLINFOBLOCK *)(lpStream+dwMemPos);
			memcpy(m_szNames[0], pmib->songname, 32);
			m_szNames[0][31] = 0;
			norders = pmib->norders;
			if (norders > MAX_ORDERS) norders = MAX_ORDERS;
			if (blocklen < sizeof(MDLINFOBLOCK) + norders - sizeof(pmib->seq)) return FALSE;
			m_nRestartPos = pmib->repeatpos;
			m_nDefaultGlobalVolume = pmib->globalvol;
			m_nDefaultTempo = pmib->tempo;
			m_nDefaultSpeed = pmib->speed;
			m_nChannels = 4;
			for (i=0; i<32; i++)
			{
				ChnSettings[i].nVolume = 64;
				ChnSettings[i].nPan = (pmib->channelinfo[i] & 0x7F) << 1;
				if (pmib->channelinfo[i] & 0x80)
					ChnSettings[i].dwFlags |= CHN_MUTE;
				else
					m_nChannels = i+1;
			}
			for (j=0; j<norders; j++) Order[j] = pmib->seq[j];
			break;
		// ME: song message
		case 0x454D:
			if (blocklen)
			{
				if (m_lpszSongComments) delete [] m_lpszSongComments;
				m_lpszSongComments = new char[blocklen];
				if (m_lpszSongComments)
				{
					memcpy(m_lpszSongComments, lpStream+dwMemPos, blocklen);
					m_lpszSongComments[blocklen-1] = 0;
				}
			}
			break;
		// PA: Pattern Data
		case 0x4150:
			npatterns = lpStream[dwMemPos];
			if (npatterns > MAX_PATTERNS) npatterns = MAX_PATTERNS;
			dwPos = dwMemPos + 1;
			for (i=0; i<npatterns; i++)
			{
				const BYTE *data;
				UINT ch;
				if (pmsh->version == 0) {
					if (m_nChannels < 32) m_nChannels = 32;
					ch = 32;
				} else {
					if (dwPos+18 >= dwMemLength) break;
					const MDLPATTERNDATA *pmpd = (const MDLPATTERNDATA *)(lpStream + dwPos);
					if (pmpd->channels > 32) break;
					PatternSize[i] = pmpd->lastrow+1;
					if (m_nChannels < pmpd->channels) m_nChannels = pmpd->channels;
					ch = pmpd->channels;
				}

				if (2 * ch >= dwMemLength - dwPos) break;
				data = lpStream + dwPos;
				dwPos += 2 * ch;
				for (j=0; j<ch && j<m_nChannels; j++, data+=2)
				{
					patterntracks[i*32+j] = data[0] | (data[1] << 8);
				}
			}
			break;
		// TR: Track Data
		case 0x5254:
			if (blocklen < 2) break;
			if (dwTrackPos) break;
			pp = lpStream + dwMemPos;
			ntracks = pp[0] | (pp[1] << 8);
			dwTrackPos = dwMemPos+2;
			break;
		// II: Instruments
		case 0x4949:
			ninstruments = lpStream[dwMemPos];
			dwPos = dwMemPos+1;
			if (blocklen < sizeof(INSTRUMENTHEADER)*ninstruments + 1) break;

			for (i=0; i<ninstruments; i++)
			{
				UINT nins = lpStream[dwPos];
				if ((nins >= MAX_INSTRUMENTS) || (!nins)) break;
				if (m_nInstruments < nins) m_nInstruments = nins;
				if (!Headers[nins])
				{
					UINT note = 12;
					if ((Headers[nins] = new INSTRUMENTHEADER) == NULL) break;
					INSTRUMENTHEADER *penv = Headers[nins];
					memset(penv, 0, sizeof(INSTRUMENTHEADER));
					if (dwPos > dwMemLength - 34) break;
					memcpy(penv->name, lpStream+dwPos+2, 32);
					penv->nGlobalVol = 64;
					penv->nPPC = 5*12;
					if (34 + 14u*lpStream[dwPos+1] > dwMemLength - dwPos) break;
					for (j=0; j<lpStream[dwPos+1]; j++)
					{
						const BYTE *ps = lpStream+dwPos+34+14*j;
						while ((note < (UINT)(ps[1]+12)) && (note < NOTE_MAX))
						{
							penv->NoteMap[note] = note+1;
							if (ps[0] < MAX_SAMPLES)
							{
								int ismp = ps[0];
								penv->Keyboard[note] = ps[0];
								Ins[ismp].nVolume = ps[2];
								Ins[ismp].nPan = ps[4] << 1;
								Ins[ismp].nVibType = ps[11];
								Ins[ismp].nVibSweep = ps[10];
								Ins[ismp].nVibDepth = ps[9];
								Ins[ismp].nVibRate = ps[8];
							}
							penv->nFadeOut = (ps[7] << 8) | ps[6];
							if (penv->nFadeOut == 0xFFFF) penv->nFadeOut = 0;
							note++;
						}
						// Use volume envelope ?
						if (ps[3] & 0x80)
						{
							penv->dwFlags |= ENV_VOLUME;
							insvolenv[nins] = (ps[3] & 0x3F) + 1;
						}
						// Use panning envelope ?
						if (ps[5] & 0x80)
						{
							penv->dwFlags |= ENV_PANNING;
							inspanenv[nins] = (ps[5] & 0x3F) + 1;
						}
					}
				}
				dwPos += 34 + 14*lpStream[dwPos+1];
				if (dwPos > dwMemLength - 2) break;
			}
			for (j=1; j<=m_nInstruments; j++) if (!Headers[j])
			{
				Headers[j] = new INSTRUMENTHEADER;
				if (Headers[j]) memset(Headers[j], 0, sizeof(INSTRUMENTHEADER));
			}
			break;
		// VE: Volume Envelope
		case 0x4556:
			if (nvolenv || (nvolenv = lpStream[dwMemPos]) == 0) break;
			if (dwMemPos + nvolenv*33 + 1 <= dwMemLength) pvolenv = lpStream + dwMemPos + 1;
			break;
		// PE: Panning Envelope
		case 0x4550:
			if (npanenv || (npanenv = lpStream[dwMemPos]) == 0) break;
			if (dwMemPos + npanenv*33 + 1 <= dwMemLength) ppanenv = lpStream + dwMemPos + 1;
			break;
		// FE: Pitch Envelope
		case 0x4546:
			if (npitchenv || (npitchenv = lpStream[dwMemPos]) == 0) break;
			if (dwMemPos + npitchenv*33 + 1 <= dwMemLength) ppitchenv = lpStream + dwMemPos + 1;
			break;
		// IS: Sample Infoblock
		case 0x5349:
			nsamples = lpStream[dwMemPos];
			i = blocklen / hdrLen;
			if (i< nsamples) nsamples = i;
			dwPos = dwMemPos+1;
			for (i=0; i<nsamples; i++, dwPos += hdrLen)
			{
				UINT nins = lpStream[dwPos];
				if ((nins >= MAX_SAMPLES) || (!nins)) continue;
				if (m_nSamples < nins) m_nSamples = nins;
				MODINSTRUMENT *pins = &Ins[nins];
				memcpy(m_szNames[nins], lpStream+dwPos+1, 32);
				m_szNames[nins][31] = 0;
				memcpy(pins->name, lpStream+dwPos+33, 8);
				pp = lpStream + dwPos + 41;
				pins->nC4Speed = pp[0] | (pp[1] << 8); pp += 2;
				if (pmsh->version > 0) {
					pins->nC4Speed |= (pp[0] << 16) | (pp[1] << 24); pp += 2;
				}
				pins->nLength = pp[0] | (pp[1] << 8) | (pp[2] << 16) | (pp[3] << 24); pp += 4;
				pins->nLoopStart = pp[0] | (pp[1] << 8) | (pp[2] << 16) | (pp[3] << 24); pp += 4;
				pins->nLoopEnd = pins->nLoopStart + (pp[0] | (pp[1] << 8) | (pp[2] << 16) | (pp[3] << 24));
				if (pins->nLoopEnd > pins->nLoopStart) pins->uFlags |= CHN_LOOP;
				pins->nGlobalVol = 64;
				if (pmsh->version == 0) pins->nVolume = pp[4];
				if (lpStream[dwPos+hdrLen-1] & 0x01)
				{
					pins->uFlags |= CHN_16BIT;
					pins->nLength >>= 1;
					pins->nLoopStart >>= 1;
					pins->nLoopEnd >>= 1;
				}
				if (lpStream[dwPos+hdrLen-1] & 0x02) pins->uFlags |= CHN_PINGPONGLOOP;
				smpinfo[nins] = (lpStream[dwPos+hdrLen-1] >> 2) & 3;
			}
			break;
		// SA: Sample Data
		case 0x4153:
			dwPos = dwMemPos;
			for (i=1; i<=m_nSamples; i++) if ((Ins[i].nLength) && (!Ins[i].pSample) && (smpinfo[i] != 3) && (dwPos < dwMemLength))
			{
				MODINSTRUMENT *pins = &Ins[i];
				UINT flags = (pins->uFlags & CHN_16BIT) ? RS_PCM16S : RS_PCM8S;
				pp = lpStream + dwPos;
				if (!smpinfo[i])
				{
					dwPos += ReadSample(pins, flags, (LPCSTR)pp, dwMemLength - dwPos);
				} else
				{
					DWORD dwLen = pp[0] | (pp[1] << 8) | (pp[2] << 16) | (pp[3] << 24); pp += 4;
					dwPos += 4;
					if ((dwLen <= dwMemLength) && (dwPos <= dwMemLength - dwLen) && (dwLen > 4))
					{
						flags = (pins->uFlags & CHN_16BIT) ? RS_MDL16 : RS_MDL8;
						ReadSample(pins, flags, (LPCSTR)pp, dwLen);
					}
					dwPos += dwLen;
				}
			}
			break;
		}
		dwMemPos += blocklen;
	}
	// Unpack Patterns
	if ((dwTrackPos) && (npatterns) && (m_nChannels) && (ntracks))
	{
		for (UINT ipat=0; ipat<npatterns; ipat++)
		{
			if ((Patterns[ipat] = AllocatePattern(PatternSize[ipat], m_nChannels)) == NULL) break;
			for (UINT chn=0; chn<m_nChannels; chn++) if ((patterntracks[ipat*32+chn]) && (patterntracks[ipat*32+chn] <= ntracks))
			{
			    const BYTE *lpTracks = lpStream + dwTrackPos;
				UINT len = 0;
				if (dwTrackPos + 2 < dwMemLength)
				  len = lpTracks[0] | (lpTracks[1] << 8);
					
			    if (len < dwMemLength-dwTrackPos)
				{
					MODCOMMAND *m = Patterns[ipat] + chn;
					UINT nTrack = patterntracks[ipat*32+chn];

					lpTracks += 2;
					for (UINT ntrk=1; ntrk<nTrack && lpTracks < (dwMemLength + lpStream - len - 2); ntrk++)
					{
						lpTracks += len;
						len = lpTracks[0] | (lpTracks[1] << 8);
						lpTracks += 2;
					}

					if ( len > dwMemLength - (lpTracks - lpStream) ) len = 0;

					UnpackMDLTrack(m, m_nChannels, PatternSize[ipat], nTrack, lpTracks, len);
			    }
			}
		}
	}
	// Set up envelopes
	for (UINT iIns=1; iIns<=m_nInstruments; iIns++) if (Headers[iIns])
	{
		INSTRUMENTHEADER *penv = Headers[iIns];
		// Setup volume envelope
		if ((nvolenv) && (pvolenv) && (insvolenv[iIns]))
		{
			LPCBYTE pve = pvolenv;
			for (UINT nve=0; nve<nvolenv; nve++, pve+=33) if (pve[0]+1 == insvolenv[iIns])
			{
				WORD vtick = 1;
				penv->nVolEnv = 15;
				for (UINT iv=0; iv<15; iv++)
				{
					if (iv) vtick += pve[iv*2+1];
					penv->VolPoints[iv] = vtick;
					penv->VolEnv[iv] = pve[iv*2+2];
					if (!pve[iv*2+1])
					{
						penv->nVolEnv = iv+1;
						break;
					}
				}
				penv->nVolSustainBegin = penv->nVolSustainEnd = pve[31] & 0x0F;
				if (pve[31] & 0x10) penv->dwFlags |= ENV_VOLSUSTAIN;
				if (pve[31] & 0x20) penv->dwFlags |= ENV_VOLLOOP;
				penv->nVolLoopStart = pve[32] & 0x0F;
				penv->nVolLoopEnd = pve[32] >> 4;
			}
		}
		// Setup panning envelope
		if ((npanenv) && (ppanenv) && (inspanenv[iIns]))
		{
			LPCBYTE ppe = ppanenv;
			for (UINT npe=0; npe<npanenv; npe++, ppe+=33) if (ppe[0]+1 == inspanenv[iIns])
			{
				WORD vtick = 1;
				penv->nPanEnv = 15;
				for (UINT iv=0; iv<15; iv++)
				{
					if (iv) vtick += ppe[iv*2+1];
					penv->PanPoints[iv] = vtick;
					penv->PanEnv[iv] = ppe[iv*2+2];
					if (!ppe[iv*2+1])
					{
						penv->nPanEnv = iv+1;
						break;
					}
				}
				if (ppe[31] & 0x10) penv->dwFlags |= ENV_PANSUSTAIN;
				if (ppe[31] & 0x20) penv->dwFlags |= ENV_PANLOOP;
				penv->nPanLoopStart = ppe[32] & 0x0F;
				penv->nPanLoopEnd = ppe[32] >> 4;
			}
		}
	}
	m_dwSongFlags |= SONG_LINEARSLIDES;
	m_nType = MOD_TYPE_MDL;
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////
// MDL Sample Unpacking

// MDL Huffman ReadBits compression
WORD MDLReadBits(DWORD &bitbuf, UINT &bitnum, LPBYTE &ibuf, CHAR n)
//-----------------------------------------------------------------
{
	WORD v = (WORD)(bitbuf & ((1 << n) - 1) );
	bitbuf >>= n;
	bitnum -= n;
	if (bitnum <= 24)
	{
		bitbuf |= (((DWORD)(*ibuf++)) << bitnum);
		bitnum += 8;
	}
	return v;
}
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (endian and char fixes for PPC)
*/

//#include "stdafx.h"
//#include "sndfile.h"

//#define MED_LOG

#ifdef MED_LOG
extern void Log(LPCSTR s, ...);
#endif

//////////////////////////////////////////////////////////
// OctaMed MED file support (import only)
//
// Lookup table for bpm values.
static const BYTE bpmvals[10] = { 179,164,152,141,131,123,116,110,104,99 };


// flags
#define	MMD_FLAG_FILTERON	0x1
#define	MMD_FLAG_JUMPINGON	0x2
#define	MMD_FLAG_JUMP8TH	0x4
#define	MMD_FLAG_INSTRSATT	0x8 // instruments are attached (this is a module)
#define	MMD_FLAG_VOLHEX		0x10
#define MMD_FLAG_STSLIDE	0x20 // SoundTracker mode for slides
#define MMD_FLAG_8CHANNEL	0x40 // OctaMED 8 channel song
#define	MMD_FLAG_SLOWHQ		0x80 // HQ slows playing speed (V2-V4 compatibility)
// flags2
#define MMD_FLAG2_BMASK		0x1F
#define MMD_FLAG2_BPM		0x20
#define	MMD_FLAG2_MIX		0x80 // uses Mixing (V7+)
// flags3:
#define	MMD_FLAG3_STEREO	0x1	// mixing in Stereo mode
#define	MMD_FLAG3_FREEPAN	0x2	// free panning
#define MMD_FLAG3_GM		0x4 // module designed for GM/XG compatibility


// generic MMD tags
#define	MMDTAG_END		0
#define	MMDTAG_PTR		0x80000000	// data needs relocation
#define	MMDTAG_MUSTKNOW	0x40000000	// loader must fail if this isn't recognized
#define	MMDTAG_MUSTWARN	0x20000000	// loader must warn if this isn't recognized

// ExpData tags
// # of effect groups, including the global group (will
// override settings in MMDSong struct), default = 1
#define	MMDTAG_EXP_NUMFXGROUPS	1
#define	MMDTAG_TRK_NAME		(MMDTAG_PTR|1)	// trackinfo tags
#define	MMDTAG_TRK_NAMELEN	2				// namelen includes zero term.
#define	MMDTAG_TRK_FXGROUP	3
// effectinfo tags
#define	MMDTAG_FX_ECHOTYPE	1
#define MMDTAG_FX_ECHOLEN	2
#define	MMDTAG_FX_ECHODEPTH	3
#define	MMDTAG_FX_STEREOSEP	4
#define	MMDTAG_FX_GROUPNAME	(MMDTAG_PTR|5)	// the Global Effects group shouldn't have name saved!
#define	MMDTAG_FX_GRPNAMELEN 6	// namelen includes zero term.

#pragma pack(1)

typedef struct tagMEDMODULEHEADER
{
	DWORD id;		// MMD1-MMD3
	DWORD modlen;	// Size of file
	DWORD song;		// Position in file for this song
	WORD psecnum;
	WORD pseq;
	DWORD blockarr;	// Position in file for blocks
	DWORD mmdflags;
	DWORD smplarr;	// Position in file for samples
	DWORD reserved;
	DWORD expdata;	// Absolute offset in file for ExpData (0 if not present)
	DWORD reserved2;
	WORD pstate;
	WORD pblock;
	WORD pline;
	WORD pseqnum;
	WORD actplayline;
	BYTE counter;
	BYTE extra_songs;	// # of songs - 1
} MEDMODULEHEADER;


typedef struct tagMMD0SAMPLE
{
	WORD rep, replen;
	BYTE midich;
	BYTE midipreset;
	BYTE svol;
	signed char strans;
} MMD0SAMPLE;


// Sample header is immediately followed by sample data...
typedef struct tagMMDSAMPLEHEADER
{
	DWORD length;     // length of *one* *unpacked* channel in *bytes*
	WORD type;
				// if non-negative
					// bits 0-3 reserved for multi-octave instruments, not supported on the PC
					// 0x10: 16 bit (otherwise 8 bit)
					// 0x20: Stereo (otherwise mono)
					// 0x40: Uses DeltaCode
					// 0x80: Packed data
				// -1: Synth
				// -2: Hybrid
	// if type indicates packed data, these fields follow, otherwise we go right to the data
	WORD packtype;	// Only 1 = ADPCM is supported
	WORD subtype;	// Packing subtype
		// ADPCM subtype
		// 1: g723_40
		// 2: g721
		// 3: g723_24
	BYTE commonflags;	// flags common to all packtypes (none defined so far)
	BYTE packerflags;	// flags for the specific packtype
	ULONG leftchlen;	// packed length of left channel in bytes
	ULONG rightchlen;	// packed length of right channel in bytes (ONLY PRESENT IN STEREO SAMPLES)
	BYTE SampleData[1];	// Sample Data
} MMDSAMPLEHEADER;


// MMD0/MMD1 song header
typedef struct tagMMD0SONGHEADER
{
	MMD0SAMPLE sample[63];
	WORD numblocks;		// # of blocks
	WORD songlen;		// # of entries used in playseq
	BYTE playseq[256];	// Play sequence
	WORD deftempo;		// BPM tempo
	signed char playtransp;	// Play transpose
	BYTE flags;			// 0x10: Hex Volumes | 0x20: ST/NT/PT Slides | 0x40: 8 Channels song
	BYTE flags2;		// [b4-b0]+1: Tempo LPB, 0x20: tempo mode, 0x80: mix_conv=on
	BYTE tempo2;		// tempo TPL
	BYTE trkvol[16];	// track volumes
	BYTE mastervol;		// master volume
	BYTE numsamples;	// # of samples (max=63)
} MMD0SONGHEADER;


// MMD2/MMD3 song header
typedef struct tagMMD2SONGHEADER
{
	MMD0SAMPLE sample[63];
	WORD numblocks;		// # of blocks
	WORD numsections;	// # of sections
	DWORD playseqtable;	// filepos of play sequence
	DWORD sectiontable;	// filepos of sections table (WORD array)
	DWORD trackvols;	// filepos of tracks volume (BYTE array)
	WORD numtracks;		// # of tracks (max 64)
	WORD numpseqs;		// # of play sequences
	DWORD trackpans;	// filepos of tracks pan values (BYTE array)
	LONG flags3;		// 0x1:stereo_mix, 0x2:free_panning, 0x4:GM/XG compatibility
	WORD voladj;		// vol_adjust (set to 100 if 0)
	WORD channels;		// # of channels (4 if =0)
	BYTE mix_echotype;	// 1:normal,2:xecho
	BYTE mix_echodepth;	// 1..6
	WORD mix_echolen;	// > 0
	signed char mix_stereosep;	// -4..4
	BYTE pad0[223];
	WORD deftempo;		// BPM tempo
	signed char playtransp;	// play transpose
	BYTE flags;			// 0x1:filteron, 0x2:jumpingon, 0x4:jump8th, 0x8:instr_attached, 0x10:hex_vol, 0x20:PT_slides, 0x40:8ch_conv,0x80:hq slows playing speed
	BYTE flags2;		// 0x80:mix_conv=on, [b4-b0]+1:tempo LPB, 0x20:tempo_mode
	BYTE tempo2;		// tempo TPL
	BYTE pad1[16];
	BYTE mastervol;		// master volume
	BYTE numsamples;	// # of samples (max 63)
} MMD2SONGHEADER;

// For MMD0 the note information is held in 3 bytes, byte0, byte1, byte2.  For reference we
// number the bits in each byte 0..7, where 0 is the low bit.
// The note is held as bits 5..0 of byte0
// The instrument is encoded in 6 bits,  bits 7 and 6 of byte0 and bits 7,6,5,4 of byte1
// The command number is bits 3,2,1,0 of byte1, command data is in byte2:
// For command 0, byte2 represents the second data byte, otherwise byte2
// represents the first data byte.
typedef struct tagMMD0BLOCK
{
	BYTE numtracks;
	BYTE lines;		// File value is 1 less than actual, so 0 -> 1 line
} MMD0BLOCK;		// BYTE data[lines+1][tracks][3];


// For MMD1,MMD2,MMD3 the note information is carried in 4 bytes, byte0, byte1,
// byte2 and byte3
// The note is held as byte0 (values above 0x84 are ignored)
// The instrument is held as byte1
// The command number is held as byte2, command data is in byte3
// For commands 0 and 0x19 byte3 represents the second data byte,
// otherwise byte2 represents the first data byte.
typedef struct tagMMD1BLOCK
{
	WORD numtracks;	// Number of tracks, may be > 64, but then that data is skipped.
	WORD lines;		// Stored value is 1 less than actual, so 0 -> 1 line
	DWORD info;		// Offset of BlockInfo (if 0, no block_info is present)
} MMD1BLOCK;


typedef struct tagMMD1BLOCKINFO
{
	DWORD hlmask;		// Unimplemented - ignore
	DWORD blockname;	// file offset of block name
	DWORD blocknamelen;	// length of block name (including term. 0)
	DWORD pagetable;	// file offset of command page table
	DWORD cmdexttable;	// file offset of command extension table
	DWORD reserved[4];	// future expansion
} MMD1BLOCKINFO;


// A set of play sequences is stored as an array of ULONG files offsets
// Each offset points to the play sequence itself.
typedef struct tagMMD2PLAYSEQ
{
	CHAR name[32];
	DWORD command_offs;	// filepos of command table
	DWORD reserved;
	WORD length;
	WORD seq[512];	// skip if > 0x8000
} MMD2PLAYSEQ;


// A command table contains commands that effect a particular play sequence
// entry.  The only commands read in are STOP or POSJUMP, all others are ignored
// POSJUMP is presumed to have extra bytes containing a WORD for the position
typedef struct tagMMDCOMMAND
{
	WORD offset;		// Offset within current sequence entry
	BYTE cmdnumber;		// STOP (537) or POSJUMP (538) (others skipped)
	BYTE extra_count;
	BYTE extra_bytes[4];// [extra_count];
} MMDCOMMAND;  // Last entry has offset == 0xFFFF, cmd_number == 0 and 0 extrabytes


typedef struct tagMMD0EXP
{
	DWORD nextmod;			// File offset of next Hdr
	DWORD exp_smp;			// Pointer to extra instrument data
	WORD s_ext_entries;		// Number of extra instrument entries
	WORD s_ext_entrsz;		// Size of extra instrument data
	DWORD annotxt;
	DWORD annolen;
	DWORD iinfo;			// Instrument names
	WORD i_ext_entries;
	WORD i_ext_entrsz;
	DWORD jumpmask;
	DWORD rgbtable;
	BYTE channelsplit[4];	// Only used if 8ch_conv (extra channel for every nonzero entry)
	DWORD n_info;
	DWORD songname;			// Song name
	DWORD songnamelen;
	DWORD dumps;
	DWORD mmdinfo;
	DWORD mmdrexx;
	DWORD mmdcmd3x;
	DWORD trackinfo_ofs;	// ptr to song->numtracks ptrs to tag lists
	DWORD effectinfo_ofs;	// ptr to group ptrs
	DWORD tag_end;
} MMD0EXP;

#pragma pack()



static void MedConvert(MODCOMMAND *p, const MMD0SONGHEADER *pmsh)
//---------------------------------------------------------------
{
	UINT command = p->command;
	UINT param = p->param;
	switch(command)
	{
	case 0x00:	if (param) command = CMD_ARPEGGIO; else command = 0; break;
	case 0x01:	command = CMD_PORTAMENTOUP; break;
	case 0x02:	command = CMD_PORTAMENTODOWN; break;
	case 0x03:	command = CMD_TONEPORTAMENTO; break;
	case 0x04:	command = CMD_VIBRATO; break;
	case 0x05:	command = CMD_TONEPORTAVOL; break;
	case 0x06:	command = CMD_VIBRATOVOL; break;
	case 0x07:	command = CMD_TREMOLO; break;
	case 0x0A:	if (param & 0xF0) param &= 0xF0; command = CMD_VOLUMESLIDE; if (!param) command = 0; break;
	case 0x0B:	command = CMD_POSITIONJUMP; break;
	case 0x0C:	command = CMD_VOLUME;
				if (pmsh->flags & MMD_FLAG_VOLHEX)
				{
					if (param < 0x80)
					{
						param = (param+1) / 2;
					} else command = 0;
				} else
				{
					if (param <= 0x99)
					{
						param = (param >> 4)*10+((param & 0x0F) % 10);
						if (param > 64) param = 64;
					} else command = 0;
				}
				break;
	case 0x09:	command = (param < 0x20) ? CMD_SPEED : CMD_TEMPO; break;
	case 0x0D:	if (param & 0xF0) param &= 0xF0; command = CMD_VOLUMESLIDE; if (!param) command = 0; break;
	case 0x0F:	// Set Tempo / Special
		// F.00 = Pattern Break
		if (!param)	command = CMD_PATTERNBREAK;	else
		// F.01 - F.F0: Set tempo/speed
		if (param <= 0xF0)
		{
			if (pmsh->flags & MMD_FLAG_8CHANNEL)
			{
				param = (param > 10) ? 99 : bpmvals[param-1];
			} else
			// F.01 - F.0A: Set Speed
			if (param <= 0x0A)
			{
				command = CMD_SPEED;
			} else
			// Old tempo
			if (!(pmsh->flags2 & MMD_FLAG2_BPM))
			{
				param = _muldiv(param, 5*715909, 2*474326);
			}
			// F.0B - F.F0: Set Tempo (assumes LPB=4)
			if (param > 0x0A)
			{
				command = CMD_TEMPO;
				if (param < 0x21) param = 0x21;
				if (param > 240) param = 240;
			}
		} else
		switch(param)
		{
		// F.F1: Retrig 2x
		case 0xF1:
			command = CMD_MODCMDEX;
			param = 0x93;
			break;
		// F.F2: Note Delay 2x
		case 0xF2:
			command = CMD_MODCMDEX;
			param = 0xD3;
			break;
		// F.F3: Retrig 3x
		case 0xF3:
			command = CMD_MODCMDEX;
			param = 0x92;
			break;
		// F.F4: Note Delay 1/3
		case 0xF4:
			command = CMD_MODCMDEX;
			param = 0xD2;
			break;
		// F.F5: Note Delay 2/3
		case 0xF5:
			command = CMD_MODCMDEX;
			param = 0xD4;
			break;
		// F.F8: Filter Off
		case 0xF8:
			command = CMD_MODCMDEX;
			param = 0x00;
			break;
		// F.F9: Filter On
		case 0xF9:
			command = CMD_MODCMDEX;
			param = 0x01;
			break;
		// F.FD: Very fast tone-portamento
		case 0xFD:
			command = CMD_TONEPORTAMENTO;
			param = 0xFF;
			break;
		// F.FE: End Song
		case 0xFE:
			command = CMD_SPEED;
			param = 0;
			break;
		// F.FF: Note Cut
		case 0xFF:
			command = CMD_MODCMDEX;
			param = 0xC0;
			break;
		default:
#ifdef MED_LOG
			Log("Unknown Fxx command: cmd=0x%02X param=0x%02X\n", command, param);
#endif
			param = command = 0;
		}
		break;
	// 11.0x: Fine Slide Up
	case 0x11:
		command = CMD_MODCMDEX;
		if (param > 0x0F) param = 0x0F;
		param |= 0x10;
		break;
	// 12.0x: Fine Slide Down
	case 0x12:
		command = CMD_MODCMDEX;
		if (param > 0x0F) param = 0x0F;
		param |= 0x20;
		break;
	// 14.xx: Vibrato
	case 0x14:
		command = CMD_VIBRATO;
		break;
	// 15.xx: FineTune
	case 0x15:
		command = CMD_MODCMDEX;
		param &= 0x0F;
		param |= 0x50;
		break;
	// 16.xx: Pattern Loop
	case 0x16:
		command = CMD_MODCMDEX;
		if (param > 0x0F) param = 0x0F;
		param |= 0x60;
		break;
	// 18.xx: Note Cut
	case 0x18:
		command = CMD_MODCMDEX;
		if (param > 0x0F) param = 0x0F;
		param |= 0xC0;
		break;
	// 19.xx: Sample Offset
	case 0x19:
		command = CMD_OFFSET;
		break;
	// 1A.0x: Fine Volume Up
	case 0x1A:
		command = CMD_MODCMDEX;
		if (param > 0x0F) param = 0x0F;
		param |= 0xA0;
		break;
	// 1B.0x: Fine Volume Down
	case 0x1B:
		command = CMD_MODCMDEX;
		if (param > 0x0F) param = 0x0F;
		param |= 0xB0;
		break;
	// 1D.xx: Pattern Break
	case 0x1D:
		command = CMD_PATTERNBREAK;
		break;
	// 1E.0x: Pattern Delay
	case 0x1E:
		command = CMD_MODCMDEX;
		if (param > 0x0F) param = 0x0F;
		param |= 0xE0;
		break;
	// 1F.xy: Retrig
	case 0x1F:
		command = CMD_RETRIG;
		param &= 0x0F;
		break;
	// 2E.xx: set panning
	case 0x2E:
		command = CMD_MODCMDEX;
		param = ((param + 0x10) & 0xFF) >> 1;
		if (param > 0x0F) param = 0x0F;
		param |= 0x80;
		break;
	default:
#ifdef MED_LOG
		// 0x2E ?
		Log("Unknown command: cmd=0x%02X param=0x%02X\n", command, param);
#endif
		command = param = 0;
	}
	p->command = command;
	p->param = param;
}


BOOL CSoundFile::ReadMed(const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
	const MEDMODULEHEADER *pmmh;
	const MMD0SONGHEADER *pmsh;
	const MMD2SONGHEADER *pmsh2;
	const MMD0EXP *pmex;
	DWORD dwBlockArr, dwSmplArr, dwExpData, wNumBlocks;
	LPDWORD pdwTable;
	CHAR version;
	UINT deftempo;
	int playtransp = 0;

	if ((!lpStream) || (dwMemLength < 0x200)) return FALSE;
	pmmh = (MEDMODULEHEADER *)lpStream;
	if (((pmmh->id & 0x00FFFFFF) != 0x444D4D) || (!pmmh->song)) return FALSE;
	// Check for 'MMDx'
	DWORD dwSong = bswapBE32(pmmh->song);
	if ((dwSong >= dwMemLength) || (dwSong + sizeof(MMD0SONGHEADER) >= dwMemLength)) return FALSE;
	version = (signed char)((pmmh->id >> 24) & 0xFF);
	if ((version < '0') || (version > '3')) return FALSE;
#ifdef MED_LOG
	Log("\nLoading MMD%c module (flags=0x%02X)...\n", version, bswapBE32(pmmh->mmdflags));
	Log("  modlen   = %d\n", bswapBE32(pmmh->modlen));
	Log("  song     = 0x%08X\n", bswapBE32(pmmh->song));
	Log("  psecnum  = %d\n", bswapBE16(pmmh->psecnum));
	Log("  pseq     = %d\n", bswapBE16(pmmh->pseq));
	Log("  blockarr = 0x%08X\n", bswapBE32(pmmh->blockarr));
	Log("  mmdflags = 0x%08X\n", bswapBE32(pmmh->mmdflags));
	Log("  smplarr  = 0x%08X\n", bswapBE32(pmmh->smplarr));
	Log("  reserved = 0x%08X\n", bswapBE32(pmmh->reserved));
	Log("  expdata  = 0x%08X\n", bswapBE32(pmmh->expdata));
	Log("  reserved2= 0x%08X\n", bswapBE32(pmmh->reserved2));
	Log("  pstate   = %d\n", bswapBE16(pmmh->pstate));
	Log("  pblock   = %d\n", bswapBE16(pmmh->pblock));
	Log("  pline    = %d\n", bswapBE16(pmmh->pline));
	Log("  pseqnum  = %d\n", bswapBE16(pmmh->pseqnum));
	Log("  actplayline=%d\n", bswapBE16(pmmh->actplayline));
	Log("  counter  = %d\n", pmmh->counter);
	Log("  extra_songs = %d\n", pmmh->extra_songs);
	Log("\n");
#endif
	m_nType = MOD_TYPE_MED;
	m_nSongPreAmp = 0x20;
	dwBlockArr = bswapBE32(pmmh->blockarr);
	dwSmplArr = bswapBE32(pmmh->smplarr);
	dwExpData = bswapBE32(pmmh->expdata);
	if ((dwExpData) && (dwExpData < dwMemLength - sizeof(MMD0EXP)))
		pmex = (MMD0EXP *)(lpStream+dwExpData);
	else
		pmex = NULL;
	pmsh = (MMD0SONGHEADER *)(lpStream + dwSong);
	pmsh2 = (MMD2SONGHEADER *)pmsh;
#ifdef MED_LOG
	if (version < '2')
	{
		Log("MMD0 Header:\n");
		Log("  numblocks  = %d\n", bswapBE16(pmsh->numblocks));
		Log("  songlen    = %d\n", bswapBE16(pmsh->songlen));
		Log("  playseq    = ");
		for (UINT idbg1=0; idbg1<16; idbg1++) Log("%2d, ", pmsh->playseq[idbg1]);
		Log("...\n");
		Log("  deftempo   = 0x%04X\n", bswapBE16(pmsh->deftempo));
		Log("  playtransp = %d\n", (signed char)pmsh->playtransp);
		Log("  flags(1,2) = 0x%02X, 0x%02X\n", pmsh->flags, pmsh->flags2);
		Log("  tempo2     = %d\n", pmsh->tempo2);
		Log("  trkvol     = ");
		for (UINT idbg2=0; idbg2<16; idbg2++) Log("0x%02X, ", pmsh->trkvol[idbg2]);
		Log("...\n");
		Log("  mastervol  = 0x%02X\n", pmsh->mastervol);
		Log("  numsamples = %d\n", pmsh->numsamples);
	} else
	{
		Log("MMD2 Header:\n");
		Log("  numblocks  = %d\n", bswapBE16(pmsh2->numblocks));
		Log("  numsections= %d\n", bswapBE16(pmsh2->numsections));
		Log("  playseqptr = 0x%04X\n", bswapBE32(pmsh2->playseqtable));
		Log("  sectionptr = 0x%04X\n", bswapBE32(pmsh2->sectiontable));
		Log("  trackvols  = 0x%04X\n", bswapBE32(pmsh2->trackvols));
		Log("  numtracks  = %d\n", bswapBE16(pmsh2->numtracks));
		Log("  numpseqs   = %d\n", bswapBE16(pmsh2->numpseqs));
		Log("  trackpans  = 0x%04X\n", bswapBE32(pmsh2->trackpans));
		Log("  flags3     = 0x%08X\n", bswapBE32(pmsh2->flags3));
		Log("  voladj     = %d\n", bswapBE16(pmsh2->voladj));
		Log("  channels   = %d\n", bswapBE16(pmsh2->channels));
		Log("  echotype   = %d\n", pmsh2->mix_echotype);
		Log("  echodepth  = %d\n", pmsh2->mix_echodepth);
		Log("  echolen    = %d\n", bswapBE16(pmsh2->mix_echolen));
		Log("  stereosep  = %d\n", (signed char)pmsh2->mix_stereosep);
		Log("  deftempo   = 0x%04X\n", bswapBE16(pmsh2->deftempo));
		Log("  playtransp = %d\n", (signed char)pmsh2->playtransp);
		Log("  flags(1,2) = 0x%02X, 0x%02X\n", pmsh2->flags, pmsh2->flags2);
		Log("  tempo2     = %d\n", pmsh2->tempo2);
		Log("  mastervol  = 0x%02X\n", pmsh2->mastervol);
		Log("  numsamples = %d\n", pmsh->numsamples);
	}
	Log("\n");
#endif
	wNumBlocks = bswapBE16(pmsh->numblocks);
	m_nChannels = 4;
	m_nSamples = pmsh->numsamples;
	if (m_nSamples > 63) m_nSamples = 63;
	// Tempo
	m_nDefaultTempo = 125;
	deftempo = bswapBE16(pmsh->deftempo);
	if (!deftempo) deftempo = 125;
	if (pmsh->flags2 & MMD_FLAG2_BPM)
	{
		UINT tempo_tpl = (pmsh->flags2 & MMD_FLAG2_BMASK) + 1;
		if (!tempo_tpl) tempo_tpl = 4;
		deftempo *= tempo_tpl;
		deftempo /= 4;
	#ifdef MED_LOG
		Log("newtempo: %3d bpm (bpm=%3d lpb=%2d)\n", deftempo, bswapBE16(pmsh->deftempo), (pmsh->flags2 & MMD_FLAG2_BMASK)+1);
	#endif
	} else
	{
		if (pmsh->flags & MMD_FLAG_8CHANNEL && deftempo > 0 && deftempo <= 10)
		{
			deftempo = bpmvals[deftempo-1];
		} else {
			deftempo = _muldiv(deftempo, 5*715909, 2*474326);
		}
	#ifdef MED_LOG
		Log("oldtempo: %3d bpm (bpm=%3d)\n", deftempo, bswapBE16(pmsh->deftempo));
	#endif
	}
	// Speed
	m_nDefaultSpeed = pmsh->tempo2;
	if (!m_nDefaultSpeed) m_nDefaultSpeed = 6;
	if (deftempo < 0x21) deftempo = 0x21;
	if (deftempo > 255)
	{
		while ((m_nDefaultSpeed > 3) && (deftempo > 260))
		{
			deftempo = (deftempo * (m_nDefaultSpeed - 1)) / m_nDefaultSpeed;
			m_nDefaultSpeed--;
		}
		if (deftempo > 255) deftempo = 255;
	}
	m_nDefaultTempo = deftempo;
	// Reading Samples
	for (UINT iSHdr=0; iSHdr<m_nSamples; iSHdr++)
	{
		MODINSTRUMENT *pins = &Ins[iSHdr+1];
		pins->nLoopStart = bswapBE16(pmsh->sample[iSHdr].rep) << 1;
		pins->nLoopEnd = pins->nLoopStart + (bswapBE16(pmsh->sample[iSHdr].replen) << 1);
		pins->nVolume = (pmsh->sample[iSHdr].svol << 2);
		pins->nGlobalVol = 64;
		if (pins->nVolume > 256) pins->nVolume = 256;
		pins->RelativeTone = -12 * pmsh->sample[iSHdr].strans;
		pins->nPan = 128;
		if (pins->nLoopEnd) pins->uFlags |= CHN_LOOP;
	}
	// Common Flags
	if (!(pmsh->flags & 0x20)) m_dwSongFlags |= SONG_FASTVOLSLIDES;
	// Reading play sequence
	if (version < '2')
	{
		UINT nbo = pmsh->songlen >> 8;
		if (nbo >= MAX_ORDERS) nbo = MAX_ORDERS-1;
		if (!nbo) nbo = 1;
		memcpy(Order, pmsh->playseq, nbo);
		playtransp = pmsh->playtransp;
	} else
	{
		UINT nOrders, nSections;
		UINT nTrks = bswapBE16(pmsh2->numtracks);
		if ((nTrks >= 4) && (nTrks <= 32)) m_nChannels = nTrks;
		DWORD playseqtable = bswapBE32(pmsh2->playseqtable);
		UINT numplayseqs = bswapBE16(pmsh2->numpseqs);
		if (!numplayseqs) numplayseqs = 1;
		nOrders = 0;
		nSections = bswapBE16(pmsh2->numsections);
		DWORD sectiontable = bswapBE32(pmsh2->sectiontable);
		if ((!nSections) || (!sectiontable) || (sectiontable >= dwMemLength-2)) nSections = 1;
		nOrders = 0;
		for (UINT iSection=0; iSection<nSections; iSection++)
		{
			UINT nplayseq = 0;
			if ((sectiontable) && (sectiontable < dwMemLength-2))
			{
				nplayseq = lpStream[sectiontable+1];
				sectiontable += 2; // WORDs
			} else
			{
				nSections = 0;
			}
			UINT pseq = 0;

			if ((playseqtable) && (playseqtable < dwMemLength - 4) && ((nplayseq+1)*4 < dwMemLength - playseqtable))
			{
				pseq = bswapBE32(((LPDWORD)(lpStream+playseqtable))[nplayseq]);
			}
			if ((pseq) && dwMemLength > sizeof(MMD2PLAYSEQ) &&
				(pseq < dwMemLength - sizeof(MMD2PLAYSEQ)))
			{
				const MMD2PLAYSEQ *pmps = (MMD2PLAYSEQ *)(lpStream + pseq);
				if (!m_szNames[0][0]) memcpy(m_szNames[0], pmps->name, 31);
				UINT n = bswapBE16(pmps->length);
				if (n < (dwMemLength - (pseq + sizeof(*pmps)) + sizeof(pmps->seq)) / sizeof(pmps->seq[0]))
				{
					for (UINT i=0; i<n; i++)
					{
						UINT seqval = pmps->seq[i] >> 8;
						if ((seqval < wNumBlocks) && (nOrders < MAX_ORDERS-1))
						{
							Order[nOrders++] = seqval;
						}
					}
				}
			}
		}
		playtransp = pmsh2->playtransp;
		while (nOrders < MAX_ORDERS) Order[nOrders++] = 0xFF;
	}
	// Reading Expansion structure
	if (pmex)
	{
		// Channel Split
		if ((m_nChannels == 4) && (pmsh->flags & MMD_FLAG_8CHANNEL))
		{
			for (UINT i8ch=0; i8ch<4; i8ch++)
			{
				if (pmex->channelsplit[i8ch]) m_nChannels++;
			}
		}
		// Song Comments
		uint32_t annotxt = bswapBE32(pmex->annotxt);
		uint32_t annolen = bswapBE32(pmex->annolen);
		if ((annotxt) && (annolen) && (annotxt + annolen > annotxt) // overflow checks.
				&& (annotxt+annolen <= dwMemLength))
		{
			m_lpszSongComments = new char[annolen+1];
			memcpy(m_lpszSongComments, lpStream+annotxt, annolen);
			m_lpszSongComments[annolen] = 0;
		}
		// Song Name
		uint32_t songname = bswapBE32(pmex->songname);
		uint32_t songnamelen = bswapBE32(pmex->songnamelen);
		if ((songname) && (songnamelen) && (songname+songnamelen > songname)
				&& (songname+songnamelen <= dwMemLength))
		{
			if (songnamelen > 31) songnamelen = 31;
			memcpy(m_szNames[0], lpStream+songname, songnamelen);
			m_szNames[0][31] = '\0';
		}
		// Sample Names
		DWORD smpinfoex = bswapBE32(pmex->iinfo);
		if (smpinfoex)
		{
			DWORD iinfoptr = bswapBE32(pmex->iinfo);
			UINT ientries = bswapBE16(pmex->i_ext_entries);
			UINT ientrysz = bswapBE16(pmex->i_ext_entrsz);

			if ((iinfoptr) && (ientrysz < 256) &&
			 (ientries*ientrysz < dwMemLength) &&
			 (iinfoptr < dwMemLength - (ientries*ientrysz)))
			{
				LPCSTR psznames = (LPCSTR)(lpStream + iinfoptr);
				UINT maxnamelen = ientrysz;
				// copy a max of 32 bytes.
				if (maxnamelen > 32) maxnamelen = 32;
				for (UINT i=0; i<ientries; i++) if (i < m_nSamples)
				{
					lstrcpyn(m_szNames[i+1], psznames + i*ientrysz, maxnamelen);
					m_szNames[i+1][31] = '\0';
				}
			}
		}
		// Track Names
		DWORD trackinfo_ofs = bswapBE32(pmex->trackinfo_ofs);
		if ((trackinfo_ofs) && (trackinfo_ofs < dwMemLength) && (m_nChannels * 4 < dwMemLength - trackinfo_ofs))
		{
			DWORD *ptrktags = (DWORD *)(lpStream + trackinfo_ofs);
			for (UINT i=0; i<m_nChannels; i++)
			{
				DWORD trknameofs = 0, trknamelen = 0;
				DWORD trktagofs = bswapBE32(ptrktags[i]);
				if (trktagofs)
				{
					while (trktagofs < dwMemLength - 8)
					{
						DWORD ntag = bswapBE32(*(DWORD *)(lpStream + trktagofs));
						if (ntag == MMDTAG_END) break;
						DWORD tagdata = bswapBE32(*(DWORD *)(lpStream + trktagofs + 4));
						switch(ntag)
						{
						case MMDTAG_TRK_NAMELEN:	trknamelen = tagdata; break;
						case MMDTAG_TRK_NAME:		trknameofs = tagdata; break;
						}
						trktagofs += 8;
					}
					if (trknamelen > MAX_CHANNELNAME) trknamelen = MAX_CHANNELNAME;
					if ((trknameofs) && (trknamelen < dwMemLength) && (trknameofs < dwMemLength - trknamelen))
					{
						lstrcpyn(ChnSettings[i].szName, (LPCSTR)(lpStream+trknameofs), trknamelen);
						ChnSettings[i].szName[MAX_CHANNELNAME-1] = '\0';
					}
				}
			}
		}
	}
	// Reading samples
	if (dwSmplArr > dwMemLength - 4*m_nSamples) return TRUE;
	pdwTable = (LPDWORD)(lpStream + dwSmplArr);
	for (UINT iSmp=0; iSmp<m_nSamples; iSmp++) if (pdwTable[iSmp])
	{
		UINT dwPos = bswapBE32(pdwTable[iSmp]);
		if ((dwPos >= dwMemLength) || (dwPos + sizeof(MMDSAMPLEHEADER) >= dwMemLength)) continue;
		MMDSAMPLEHEADER *psdh = (MMDSAMPLEHEADER *)(lpStream + dwPos);
		UINT len = bswapBE32(psdh->length);
	#ifdef MED_LOG
		Log("SampleData %d: stype=0x%02X len=%d\n", iSmp, bswapBE16(psdh->type), len);
	#endif
		if ((len > MAX_SAMPLE_LENGTH) || (dwPos + len + 6 > dwMemLength)) len = 0;
		UINT flags = RS_PCM8S, stype = bswapBE16(psdh->type);
		LPSTR psdata = (LPSTR)(lpStream + dwPos + 6);
		UINT bLimit = dwMemLength - dwPos - 6;
		if (stype & 0x80)
		{
			psdata += (stype & 0x20) ? 14 : 6;
			bLimit -= (stype & 0x20) ? 14 : 6;
		} else
		{
			if (stype & 0x10)
			{
				Ins[iSmp+1].uFlags |= CHN_16BIT;
				len /= 2;
				flags = (stype & 0x20) ? RS_STPCM16M : RS_PCM16M;
			} else
			{
				flags = (stype & 0x20) ? RS_STPCM8S : RS_PCM8S;
			}
			if (stype & 0x20) len /= 2;
		}
		Ins[iSmp+1].nLength = len;
		ReadSample(&Ins[iSmp+1], flags, psdata, bLimit);
	}
	// Reading patterns (blocks)
	if (wNumBlocks > MAX_PATTERNS) wNumBlocks = MAX_PATTERNS;
	if ((!dwBlockArr) || (dwMemLength < 4*wNumBlocks) ||
		(dwBlockArr > dwMemLength - 4*wNumBlocks)) return TRUE;
	pdwTable = (LPDWORD)(lpStream + dwBlockArr);
	playtransp += (version == '3') ? 24 : 48;
	for (UINT iBlk=0; iBlk<wNumBlocks; iBlk++)
	{
		UINT dwPos = bswapBE32(pdwTable[iBlk]);
		if ((!dwPos) || (dwPos >= dwMemLength) || (dwPos >= dwMemLength - 8)) continue;
		UINT lines = 64, tracks = 4;
		if (version == '0')
		{
			const MMD0BLOCK *pmb = (const MMD0BLOCK *)(lpStream + dwPos);
			lines = pmb->lines + 1;
			tracks = pmb->numtracks;
			if (!tracks) tracks = m_nChannels;
			if ((Patterns[iBlk] = AllocatePattern(lines, m_nChannels)) == NULL) continue;
			PatternSize[iBlk] = lines;
			MODCOMMAND *p = Patterns[iBlk];
			LPBYTE s = (LPBYTE)(lpStream + dwPos + 2);
			UINT maxlen = tracks*lines*3;
			if (maxlen + dwPos > dwMemLength - 2) break;
			for (UINT y=0; y<lines; y++)
			{
				for (UINT x=0; x<tracks; x++, s+=3) if (x < m_nChannels)
				{
					BYTE note = s[0] & 0x3F;
					BYTE instr = s[1] >> 4;
					if (s[0] & 0x80) instr |= 0x10;
					if (s[0] & 0x40) instr |= 0x20;
					if ((note) && (note <= 132)) p->note = note + playtransp;
					p->instr = instr;
					p->command = s[1] & 0x0F;
					p->param = s[2];
					// if (!iBlk) Log("%02X.%02X.%02X | ", s[0], s[1], s[2]);
					MedConvert(p, pmsh);
					p++;
				}
				//if (!iBlk) Log("\n");
			}
		} else
		{
			const MMD1BLOCK *pmb = (MMD1BLOCK *)(lpStream + dwPos);
		#ifdef MED_LOG
			Log("MMD1BLOCK:   lines=%2d, tracks=%2d, offset=0x%04X\n",
				bswapBE16(pmb->lines), bswapBE16(pmb->numtracks), bswapBE32(pmb->info));
		#endif
			const MMD1BLOCKINFO *pbi = NULL;
			BYTE *pcmdext = NULL;
			lines = (pmb->lines >> 8) + 1;
			tracks = pmb->numtracks >> 8;
			if (!tracks) tracks = m_nChannels;
			if ((Patterns[iBlk] = AllocatePattern(lines, m_nChannels)) == NULL) continue;
			PatternSize[iBlk] = (WORD)lines;
			DWORD dwBlockInfo = bswapBE32(pmb->info);
			if ((dwBlockInfo) && (dwBlockInfo < dwMemLength - sizeof(MMD1BLOCKINFO)))
			{
				pbi = (MMD1BLOCKINFO *)(lpStream + dwBlockInfo);
			#ifdef MED_LOG
				Log("  BLOCKINFO: blockname=0x%04X namelen=%d pagetable=0x%04X &cmdexttable=0x%04X\n",
					bswapBE32(pbi->blockname), bswapBE32(pbi->blocknamelen), bswapBE32(pbi->pagetable), bswapBE32(pbi->cmdexttable));
			#endif
				if ((pbi->blockname) && (pbi->blocknamelen))
				{
					DWORD nameofs = bswapBE32(pbi->blockname);
					UINT namelen = bswapBE32(pbi->blocknamelen);
					if ((namelen < dwMemLength) && (nameofs < dwMemLength - namelen))
					{
						// SetPatternName expects a nul-terminated string.
						char blockname[MAX_PATTERNNAME];
						if (namelen >= MAX_PATTERNNAME) namelen = MAX_PATTERNNAME - 1;
						memcpy(blockname, lpStream + nameofs, namelen);
						blockname[namelen] = '\0';

						SetPatternName(iBlk, blockname);
					}
				}
				if (pbi->cmdexttable)
				{
					DWORD cmdexttable = bswapBE32(pbi->cmdexttable);
					if (cmdexttable < dwMemLength - 4)
					{
						cmdexttable = bswapBE32(*(DWORD *)(lpStream + cmdexttable));
						if ((cmdexttable) && (cmdexttable <= dwMemLength - lines*tracks))
						{
							pcmdext = (BYTE *)(lpStream + cmdexttable);
						}
					}
				}
			}
			MODCOMMAND *p = Patterns[iBlk];
			LPBYTE s = (LPBYTE)(lpStream + dwPos + 8);
			UINT maxlen = tracks*lines*4;
			if (maxlen + dwPos > dwMemLength - 8) break;
			for (UINT y=0; y<lines; y++)
			{
				for (UINT x=0; x<tracks; x++, s+=4) if (x < m_nChannels)
				{
					BYTE note = s[0];
					if ((note) && (note <= 132))
					{
						int rnote = note + playtransp;
						if (rnote < 1) rnote = 1;
						if (rnote > NOTE_MAX) rnote = NOTE_MAX;
						p->note = (BYTE)rnote;
					}
					p->instr = s[1];
					p->command = s[2];
					p->param = s[3];
					if (pcmdext) p->vol = pcmdext[x];
					MedConvert(p, pmsh);
					p++;
				}
				if (pcmdext) pcmdext += tracks;
			}
		}
	}
	// Setup channel pan positions
	for (UINT iCh=0; iCh<m_nChannels; iCh++)
	{
		ChnSettings[iCh].nPan = (((iCh&3) == 1) || ((iCh&3) == 2)) ? 0xC0 : 0x40;
		ChnSettings[iCh].nVolume = 64;
	}
	return TRUE;
}
/*

 MikMod Sound System

  By Jake Stine of Divine Entertainment (1996-2000)

 Support:
  If you find problems with this code, send mail to:
    air@divent.org

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 -----------------------------------------
 Module: LOAD_MID

  MID module loader.
	by Peter Grootswagers (2006)
	<email:pgrootswagers@planet.nl>

 Portability:
	All systems - all compilers (hopefully)
*/

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#ifndef _WIN32
#include <unistd.h> /* sleep() */
#endif

//#include "stdafx.h"
//#include "sndfile.h"
#define PAN_LEFT    0x30
#define PAN_RIGHT   0xD0
#define MAX_POLYPHONY 16  // max notes in one midi channel
#define MAX_TRACKS    (MAX_BASECHANNELS-6)  // max mod tracks
#define WHEELSHIFT    10  // how many bits the 13bit midi wheel value must shift right

//#include "load_pat.h"

#define ROWSPERNOTE 16
#define ENV_MMMID_SPEED "MMMID_SPEED"
#define ENV_MMMID_DEBUG "MMMID_DEBUG"
#define ENV_MMMID_VERBOSE "MMMID_VERBOSE"

/**********************************************************************/

typedef enum {
	mid_none,
	wheeldown,
	wheelup,
	fxbrk,
	tmpo,
	fxsync,
	modwheel,
	mainvol,
	prog
} MIDEVENT_X_EFFECT;

typedef struct _MIDEVENT
{
	struct _MIDEVENT *next;
	ULONG	tracktick;
	BYTE flg; // 1 = note present
	BYTE note;
	BYTE volume;
	BYTE smpno;
	BYTE fx;
	BYTE fxparam;
} MIDEVENT;

typedef struct _MIDTRACK
{
	struct _MIDTRACK *next;
	MIDEVENT *head;
	MIDEVENT *tail;
	MIDEVENT *workevent; // keeps track of events in track
	int balance; // last balance on this track
	ULONG vtracktick; // tracktick of last note event (on or off)
	BYTE chan;
	BYTE vpos;	// 0xff is track is free for use, otherwise it's the note playing on this track
	BYTE volume; // last note volume on this track
	BYTE instr;	// current instrument for this track
} MIDTRACK;

#if defined(_WIN32) && defined(_mm_free)
#undef _mm_free
#endif

#define MMSTREAM										FILE
#define _mm_fseek(f,pos,whence)			fseek(f,pos,whence)
#define _mm_read_UBYTES(buf,sz,f)		fread(buf,sz,1,f)
#define _mm_read_SBYTES(buf,sz,f)		fread(buf,sz,1,f)
#define DupStr(h,buf,sz)			strdup(buf)
#define _mm_calloc(h,n,sz)			calloc(n,sz)
#define _mm_recalloc(h,buf,sz,elsz)	realloc(buf,sz)
#define _mm_free(h,p)				free(p)

/**********************************************************************/

typedef struct _MIDHANDLE
{
	const BYTE *mm;
	unsigned long sz;
	unsigned long pos;
	int err;
	MIDTRACK *track;
	MIDTRACK *tp;
	ULONG tracktime;
	const char *debug;
	const char *verbose;
	int speed;
	int midispeed;
	int midiformat;
	int resolution;
	int miditracks;
	int divider;
	int tempo;
	int percussion;
	long deltatime;
} MIDHANDLE;

static void mid_dump_tracks(MIDHANDLE *h)
{
	MIDTRACK *tr;
	MIDEVENT *e;
	int t;
	printf("tracktime  = %ld\n", (long)(h->tracktime));
	printf("speed      = %d\n", h->speed);
	printf("midispeed  = %d\n", h->midispeed);
	printf("midiformat = %d\n", h->midiformat);
	printf("resolution = %d\n", h->resolution);
	printf("miditracks = %d\n", h->miditracks);
	printf("divider    = %d\n", h->divider);
	printf("tempo      = %d\n", h->tempo);
	printf("percussion = %d\n", h->percussion);
	printf("deltatime  = %ld\n", h->deltatime);
	t = 0;
	for( tr=h->track; tr; tr = tr->next ) {
		t++;
		printf("TRACK %2d chan=%d note=0x%02x vol=%d pan=0x%02x instr=%d\n", t, tr->chan + 1, tr->vpos, tr->balance, tr->volume, tr->instr);
		for( e=tr->head; e; e=e->next ) {
			printf("%2d %6ld %s %3d %3d %3d ",
			       t, (long)(e->tracktick),
			       e->flg? "NOTE": "CTRL", e->note, e->volume, e->smpno);
			switch(	e->fx ) {
				case fxbrk: printf("fxbrk\n");break;
				case fxsync: printf("fxsync\n");break;
				case prog:	printf("prog %d\n", e->fxparam);break;
				case mainvol:	printf("mainvol %d\n", e->fxparam);break;
				case modwheel:	printf("modwheel %d\n", e->fxparam);break;
				case wheeldown:	printf("wheeldown %d\n", e->fxparam);break;
				case wheelup:	printf("wheelup %d\n", e->fxparam);break;
				case tmpo:	printf("tmpo %d\n", e->fxparam);break;
				default: printf("\n");break;
			}
		}
	}
}

static void mid_message(const char *s1, const char *s2)
{
	char txt[256];
	if( strlen(s1) + strlen(s2) > 255 ) return;
	sprintf(txt, s1, s2);
	fprintf(stderr, "load_mid > %s\n", txt);
}

static ULONG miditicks(MIDHANDLE *h, ULONG modtick)
{
	return modtick * h->divider / ROWSPERNOTE / h->speed;
}

static ULONG modticks(MIDHANDLE *h, ULONG miditick)
{
	return miditick * ROWSPERNOTE * h->speed / h->divider;
}

static void mid_adjust_for_optimal_tempo(MIDHANDLE *h, int maxtempo)
{
	// the tempo is adjusted so that the maximum tempo is 255
	// this way we have the biggest change that very short notes get played
	// and we make sure the tempo doesn't become too large or too small
	// if the piece in hand isn't so weird it changes tempo from 20 to 255, that is.
	// tempo is only registered in first track (h->track) because it is a global event
	MIDEVENT *e;
	int d, t;
	if( maxtempo < 1 ) return;
	d = h->divider;
	t = maxtempo;
	h->divider = (t * d) / 255;
	while( (h->midispeed = miditicks(h, h->speed)) < h->speed ) {
		++t;
		h->divider = (t * d) / 255;
	}
	if( h->verbose && t > maxtempo )
		printf("Adjusted maximum tempo from %d to %d to get %d miditicks per patternrow\n",
			maxtempo, 2 * maxtempo - t, h->midispeed);
	if( h->track ) {
		for( e=h->track->head; e; e=e->next ) {
			if( e->fx == tmpo )
				e->fxparam = (255 * e->fxparam ) / t;
		}
	}
}

// =====================================================================================
static MIDEVENT *mid_new_event(MIDHANDLE *h)
// =====================================================================================
{
    MIDEVENT   *retval;

    retval = (MIDEVENT *)_mm_calloc(h->trackhandle, 1,sizeof(MIDEVENT));
		retval->next      = NULL;
    retval->tracktick = h->tracktime;
		retval->flg       = 0;
		retval->note      = 0;
		retval->volume    = 0;
		retval->smpno     = 0;
		retval->fx        = mid_none;
		retval->fxparam   = 0;
    return retval;
}

// =====================================================================================
static MIDTRACK *mid_new_track(MIDHANDLE *h, int mch, int pos)
// =====================================================================================
{
	MIDTRACK *retval;
	retval = (MIDTRACK *)_mm_calloc(h->trackhandle, 1,sizeof(MIDTRACK));
	retval->next       = NULL;
	retval->vpos       = pos;
	retval->instr      = 1;
	retval->chan       = mch;
	retval->head       = NULL;
	retval->tail       = NULL;
	retval->workevent  = NULL;
	retval->vtracktick = 0;
	retval->volume     = h->track? h->track->volume: 120;
	retval->balance    = 64;
	return retval;
}

static int mid_numtracks(MIDHANDLE *h)
{
	int n;
	MIDTRACK *t;
	n=0;
	for( t = h->track; t; t=t->next )
		n++;
	return n;
}

// find out how many midichannel we have
static int mid_numchans(MIDHANDLE *h)
{
	int i,c,n;
	MIDTRACK *t;
	c = 0;
	for( t = h->track; t; t=t->next )
		c |= (1<<t->chan);
	n = 0;
	for( i=0; i<16; i++ )
		if( c & (1<<i) ) n++;
	return n;
}

// find out which ordinal a midichannel has
static int mid_ordchan(MIDHANDLE *h, int mch)
{
	int i,c,n;
	MIDTRACK *t;
	c = 0;
	for( t = h->track; t; t=t->next )
		c |= (1<<t->chan);
	n = 0;
	for( i=0; i<mch; i++ )
		if( c & (1<<i) ) n++;
	return n;
}

static void mid_rewind_tracks(MIDHANDLE *h)
{
	MIDTRACK *tr;
	h->tracktime = 0;
	for( tr = h->track; tr; tr = tr->next ) {
		tr->vpos       = 0xff;
		tr->workevent  = tr->head;
		tr->vtracktick = 0;
	}
}

static void mid_update_track(MIDTRACK *tr)
{
	MIDEVENT *e;
	e = tr->workevent;
	if( e->flg )	{
		if( e->volume ) tr->vpos = e->note;
		else tr->vpos = 0xff;
		tr->volume = e->volume;
		tr->vtracktick = e->tracktick;
	}
	if( e->fx == prog ) tr->instr = e->fxparam;
}

static void mid_sync_track(MIDTRACK *tr, ULONG tracktime)
{
	MIDEVENT *e;
	e = tr->workevent;
	if( e && e->tracktick > tracktime ) e = tr->head; // start again....
	for( ; e && e->tracktick <= tracktime; e=e->next ) {
		tr->workevent = e;
		mid_update_track(tr);
	}
}

// =====================================================================================
static MIDTRACK *mid_find_track(MIDHANDLE *h, int mch, int pos)
// =====================================================================================
{
	MIDTRACK *tr;
	for( tr=h->track; tr; tr=tr->next ) {
		mid_sync_track(tr, h->tracktime);
		if( tr->chan == mch && tr->vpos == pos )
			return tr;
	}
	return NULL;
}

// =====================================================================================
static MIDTRACK *mid_locate_track(MIDHANDLE *h, int mch, int pos)
// =====================================================================================
{
	MIDTRACK *tr, *prev, *trunused;
	MIDEVENT *e;
	int instrno = 1;
	int polyphony;
	int vol = 0, bal = 0;
	int numtracks;
	ULONG tmin;
	prev = NULL;
	trunused = NULL;
	polyphony = 0;
	numtracks = 0;
	tmin = h->midispeed; // minimal distance between note events in track
	// look up track with desired channel and pos (note)
	for( tr=h->track; tr; tr=tr->next ) {
		mid_sync_track(tr, h->tracktime);
		if( tr->chan == mch ) {
			if( tr->vpos == pos )
				return tr;
			if( tr->vpos == 0xff ) {
				// check if track with silence is quiet long enough
				if( h->tracktime > tr->vtracktick + tmin ) trunused = tr;
			}
			else vol = tr->volume;
			instrno = tr->instr;
			bal = tr->balance;
			polyphony++;
		}
		numtracks++;
		prev = tr;
	}
	if( trunused ) {
		trunused->vpos = pos;
		return trunused;
	}
	if( polyphony > MAX_POLYPHONY || (polyphony > 0 && numtracks > MAX_TRACKS) ) { // do not use up too much channels
		for( tr=h->track; tr; tr=tr->next ) {
			if( tr->chan == mch ) {
				e = tr->workevent;
				if (!e) {
					trunused = tr;
					break;
				}
				if( h->tracktime > e->tracktick + tmin ) {
					tmin = h->tracktime - e->tracktick;
					trunused = tr;
				}
			}
		}
		if( trunused ) {
			trunused->vpos = pos;
			return trunused;
		}
	}
	if( numtracks > MAX_TRACKS ) { // we can not allocate new tracks
		tmin = 0;
		for( tr=h->track; tr; tr=tr->next ) {
			if( tr->chan == mch ) {
				e = tr->workevent;
				if (!e) {
					trunused = tr;
					break;
				}
				if( h->tracktime >= e->tracktick + tmin ) {
					tmin = h->tracktime - e->tracktick;
					trunused = tr;
				}
			}
		}
		if( trunused ) {
			trunused->vpos = pos;
			return trunused;
		}
		tmin = 0;
		for( tr=h->track; tr; tr=tr->next ) {
			e = tr->workevent;
			if (!e) {
				trunused = tr;
				break;
			}
			if( h->tracktime >= e->tracktick + tmin ) {
				tmin = h->tracktime - e->tracktick;
				trunused = tr;
			}
		}
		if( trunused ) {
			trunused->vpos = pos;
			trunused->chan = mch;
			return trunused;
		}
	}
	tr = mid_new_track(h, mch, pos);
	tr->instr  = instrno;
	tr->volume = vol;
	tr->balance = bal;
	if( prev ) prev->next = tr;
	else	h->track = tr;
	return tr;
}

static void	mid_add_event(MIDHANDLE *h, MIDTRACK *tp, MIDEVENT *e)
{
	MIDEVENT *ew, *ep;
	ep = NULL;
	ew = tp->workevent;
	if( ew && ew->tracktick > e->tracktick ) ew = tp->head; // start again from the beginning...
	for( ; ew && ew->tracktick <= e->tracktick; ew = ew->next ) {
		ep = ew;
		tp->workevent = ew;
		mid_update_track(tp);
	}
	if( ep ) {
		ep->next = e;
		e->next = ew;
	}
	else {
		e->next = tp->head;
		tp->head = e;
	}
	if( !e->next )
		tp->tail = e;
	tp->workevent = e;
	mid_update_track(tp);
}

static void mid_add_tempo_event(MIDHANDLE *h, int tempo)
{
	MIDEVENT *e;
	e = mid_new_event(h);
	e->flg = 0;
	e->fx = tmpo;
	e->fxparam = tempo;
	mid_add_event(h, h->track, e);
}

static void mid_add_partbreak(MIDHANDLE *h)
{
	MIDEVENT *e;
	e = mid_new_event(h);
	e->flg = 0;
	e->fx = fxbrk;
	mid_add_event(h, h->track, e);
}

static void mid_add_noteoff(MIDHANDLE *h, MIDTRACK *tp)
{
	MIDEVENT *e;
	e = mid_new_event(h);
	e->flg = 1;
	e->note = tp->vpos;
	e->smpno = tp->instr;
	mid_add_event(h, tp, e);
}

static void mid_add_noteon(MIDHANDLE *h, MIDTRACK *tp, int n, int vol)
{
	MIDEVENT *e;
	e = mid_new_event(h);
	e->flg = 1;
	e->note = n;
	e->smpno = tp->instr;
	e->volume = vol;
	mid_add_event(h, tp, e);
}

static BYTE modtremolo(int midimod)
{
	int m;
	if( midimod == 0 ) return 0;
	if( midimod > 63 ) {
		m = (128 - midimod) / 4;
		if( m==0 ) m = 1;
		return m|0xf0; // find slide down
	}
	m = midimod / 4;
	if( m==0 ) m = 1;
	return (m<<4)|0x0f; // find slide up
}

// =====================================================================================
static void mid_mod_wheel(MIDHANDLE *h, int mch, int mod)
// =====================================================================================
{
	MIDTRACK *tr;
	MIDEVENT *e;
	for( tr=h->track; tr; tr=tr->next ) {
		if( tr->chan == mch ) {
			mid_sync_track(tr, h->tracktime);
			if( tr->vpos != 0xff ) { // only on tracks with notes on...
				e = mid_new_event(h);
				e->flg = 0;
				e->fx = modwheel;
				e->fxparam = modtremolo(mod);
				mid_add_event(h, tr, e);
			}
		}
	}
}

// =====================================================================================
static void mid_main_volume(MIDHANDLE *h, int mch, int vol)
// =====================================================================================
{
	MIDTRACK *tr;
	MIDEVENT *e;
	for( tr=h->track; tr; tr=tr->next ) {
		if( tr->chan == mch ) {
			e = mid_new_event(h);
			e->flg = 0;
			e->fx = mainvol;
			e->fxparam = vol;
			mid_add_event(h, tr, e);
		}
	}
}

// transform 0..63..127 to left..center..right in 2n+1 areas
static int modpan(int midipan, int n)
{
	int npan, area, x;
	x    = 2 * n + 1;
	area = (midipan * x * (PAN_RIGHT - PAN_LEFT))>>7;
	npan = (PAN_LEFT * x + area) / x;
	return npan;
}

// =====================================================================================
static void mid_pan(MIDHANDLE *h, int mch, int pan)
// =====================================================================================
{
	MIDTRACK *tr;
	int hits;
	hits = 0;
	for( tr=h->track; tr; tr=tr->next ) {
		if( tr->chan == mch ) {
			hits++;
			tr->balance = pan;
		}
	}
	if( !hits ) {
		tr = mid_locate_track(h, mch, 0xff);
		tr->balance = pan;
	}
}

// =====================================================================================
static void mid_add_program(MIDHANDLE *h, int mch, int pr)
// =====================================================================================
{
	MIDTRACK *tr;
	MIDEVENT *e;
	int hits;
	hits = 0;
	for( tr=h->track; tr; tr=tr->next ) {
		if( tr->chan == mch ) {
			hits++;
			e = mid_new_event(h);
			e->flg = 0;
			e->fx = prog;
			e->fxparam = pat_gmtosmp(pr + 1);
			mid_add_event(h, tr, e);
		}
	}
	if( !hits ) {
		tr = mid_locate_track(h, mch, 0xff);
		e = mid_new_event(h);
		e->flg = 0;
		e->fx = prog;
		e->fxparam = pat_gmtosmp(pr + 1);
		mid_add_event(h, tr, e);
	}
}

// =====================================================================================
static void mid_all_notes_off(MIDHANDLE *h, int mch)
// =====================================================================================
{
	MIDTRACK *tr;
	if( h->debug ) printf("%ld %d all notes off\n",(long)(h->tracktime), mch+1);
	for( tr=h->track; tr; tr=tr->next ) {
		if( tr->chan == mch || mch == -1 ) {
			mid_sync_track(tr, h->tracktime);
			if( tr->vpos != 0xff )
				mid_add_noteoff(h, tr);
		}
	}
}

static void mid_add_sync(MIDHANDLE *h, MIDTRACK *tp)
{
	MIDEVENT *e;
	e = mid_new_event(h);
	e->flg = 0;
	e->fx = fxsync;
	mid_add_event(h, tp, e);
}

static BYTE mid_to_mod_wheel(unsigned int midwheel)
{
	unsigned int i;
	if( midwheel == 0 ) return 0;
	i = midwheel >> WHEELSHIFT;
	return i+1;
}

static void mid_add_wheel(MIDHANDLE *h, MIDTRACK *tp, int wheel)
{
	MIDEVENT *e;
	e = mid_new_event(h);
	e->flg = 0;
	if( wheel < 0 ) {
		e->fx = wheeldown;
		e->fxparam = mid_to_mod_wheel(-wheel);
	}
	else {
		e->fx = wheelup;
		e->fxparam = mid_to_mod_wheel(wheel);
	}
	mid_add_event(h, tp, e);
}

static void mid_add_pitchwheel(MIDHANDLE *h, int mch, int wheel)
{
	MIDTRACK *tr;
	int hits;
	hits = 0;
	for( tr=h->track; tr; tr=tr->next ) {
		if( tr->chan == mch ) {
			hits++;
			mid_sync_track(tr, h->tracktime);
			if( tr->vpos != 0xff ) // only on tracks with notes on...
				mid_add_wheel(h, tr, wheel);
		}
	}
	if( !hits ) { // special case in midiformat 1 events in first track...
		tr = mid_locate_track(h, mch, 0xff);
		mid_add_wheel(h, tr, wheel);
	}
}

static uint32_t mid_read_long(MIDHANDLE *h)
{
	BYTE buf[4];
	if (h->pos > h->sz - 4) {
		h->err = EOF;
		return 0;
	}
	memcpy(buf, h->mm + h->pos, 4);
	h->pos += 4;
	return (buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|buf[3];
}

static short int mid_read_short(MIDHANDLE *h)
{
	BYTE buf[2];
	if (h->pos > h->sz - 2) {
		h->err = EOF;
		return 0;
	}
	memcpy(buf, h->mm + h->pos, 2);
	h->pos += 2;
	return (buf[0]<<8)|buf[1];
}

static BYTE mid_read_byte(MIDHANDLE *h)
{
	if (h->pos >= h->sz) {
		h->err = EOF;
		return 0;
	}
	return h->mm[h->pos++];
}

static unsigned long mid_read_bytes(void *dest, unsigned long sz, MIDHANDLE *h)
{
	if (sz > h->sz || h->pos > h->sz - sz) {
		h->err = EOF;
		return 0;
	}
	memcpy(dest, h->mm + h->pos, sz);
	h->pos += sz;
	return sz;
}

static int mid_read_delta(MIDHANDLE *h)
{
	BYTE bits;
	int i, d;
	d = 0;
	for( i=0; i<4; ) {
		bits = mid_read_byte(h);
		i++;
		d = (d<<7)|(bits&0x7f);
		if( !(bits & 0x80) )
			break;
	}
	h->deltatime = d;
	return i;
}

// =====================================================================================
BOOL CSoundFile::TestMID(const BYTE *lpStream, DWORD dwMemLength)
// =====================================================================================
{
	char id[5];
	MIDHANDLE h;
	if (dwMemLength < 14) return FALSE;
	h.mm = lpStream;
	h.sz = dwMemLength;
	h.pos = 0;
	h.err = 0;
	mid_read_bytes(id, 4, &h);
	id[4] = '\0';
	return !strcmp(id,"MThd") && mid_read_long(&h) == 6;
}

// =====================================================================================
static MIDHANDLE *MID_Init(void)
{
	MIDHANDLE *retval;
	retval = (MIDHANDLE *)calloc(1,sizeof(MIDHANDLE));
	if( !retval ) return NULL;
	retval->track      = NULL;
	retval->percussion = 0;
	retval->debug      = NULL;
	retval->miditracks = 0;
	return retval;
}

static void MID_CleanupTrack(MIDTRACK *tp)
{
	MIDEVENT *ep, *en;
	if( tp ) {
		for( ep=tp->head; ep; ep = en ) {
			en=ep->next;
			free(ep);
		}
		tp->head = NULL;
	}
}

// =====================================================================================
static void MID_CleanupTracks(MIDHANDLE *handle)
// =====================================================================================
{
	MIDTRACK *tp, *tn;
	if(handle) {
		for( tp=handle->track; tp; tp = tn ) {
			tn=tp->next;
			MID_CleanupTrack(tp);
			free(tp);
		}
		handle->track = NULL;
	}
}

// =====================================================================================
static void MID_Cleanup(MIDHANDLE *handle)
// =====================================================================================
{
	if(handle) {
		MID_CleanupTracks(handle);
		free(handle);
		handle = 0;
	}
}

static int mid_is_global_event(MIDEVENT *e)
{
	return (e->fx == tmpo || e->fx == fxbrk);
}

static MIDEVENT *mid_next_global(MIDEVENT *e)
{
	for( ; e && !mid_is_global_event(e); e=e->next ) ;
	return e;
}

static MIDEVENT *mid_next_fx(MIDEVENT *e)
{
	for( ; e && e->fx == mid_none; e=e->next ) ;
	return e;
}

static int mid_is_note_event(MIDEVENT *e)
{
#ifdef LOOPED_NOTES_OFF
	return (e->flg == 0);
#else
	if( e->flg == 0 ) return 0;
	if( e->volume ) return 1;
	return pat_smplooped(e->smpno); // let non looping samples die out...
#endif
}

static MIDEVENT *mid_next_note(MIDEVENT *e)
{
	for( ; e && !mid_is_note_event(e); e=e->next ) ;
	return e;
}

// =====================================================================================
static int MID_ReadPatterns(MODCOMMAND *pattern[], WORD psize[], MIDHANDLE *h, int numpat, int channels)
// =====================================================================================
{
	int pat,row,i,ch;
	BYTE n,ins,vol;
	MIDTRACK *t;
	MIDEVENT *e, *en, *ef, *el;
	ULONG tt1, tt2;
	MODCOMMAND *m;
	int patbrk, tempo;
	if( numpat > MAX_PATTERNS ) numpat = MAX_PATTERNS;

	// initialize start points of event list in tracks
	for( t = h->track; t; t = t->next ) t->workevent = t->head;
	for( pat = 0; pat < numpat; pat++ ) {
		pattern[pat] = CSoundFile::AllocatePattern(64, channels);
		if( !pattern[pat] ) return 0;
		psize[pat] = 64;
		for( row = 0; row < 64; row++ ) {
			tt1 = miditicks(h, (pat * 64 + row ) * h->speed);
			tt2 = tt1 + h->midispeed;
			ch = 0;
			tempo = 0;
			patbrk = 0;
			if ( h->track )
			for( e=mid_next_global(h->track->workevent); e && e->tracktick < tt2; e=mid_next_global(e->next) ) {
				if( e && e->tracktick >= tt1 ) {	// we have a controller event in this row
					switch( e->fx ) {
						case tmpo:
							tempo = e->fxparam;
							break;
						case fxbrk:
							patbrk = 1;
							break;
					}
				}
			}
			for( t = h->track; t; t = t->next ) {
				m = &pattern[pat][row * channels + ch];
				m->param   = 0;
				m->command = CMD_NONE;
				for( e=mid_next_fx(t->workevent); e && e->tracktick < tt2; e=mid_next_fx(e->next) ) {
					if( e && e->tracktick >= tt1 ) {	// we have a controller event in this row
						switch( e->fx ) {
							case modwheel:
								m->param   = e->fxparam;
								m->command = CMD_VOLUMESLIDE;
								break;
							case wheelup:
								m->param   = e->fxparam|0x10;
								m->command = CMD_XFINEPORTAUPDOWN;
								break;
							case wheeldown:
								m->param   = e->fxparam|0x20;
								m->command = CMD_XFINEPORTAUPDOWN;
								break;
						}
					}
				}
				for( e=mid_next_note(t->workevent); e && e->tracktick < tt1; e=mid_next_note(e->next) )
					t->workevent = e;
				i = 0;
				ef = NULL;
				en = e;
				el = e;
				for( ; e && e->tracktick < tt2; e=mid_next_note(e->next) ) {	// we have a note event in this row
					t->workevent = e;
					i++;
					if( e->volume ) {
						if( !ef ) ef = e;
						el = e;
					}
				}
				if( i ) {
					if( i == 1 || ef == el || !ef ) { // only one event in this row or a note on with some note off
						if( ef ) e = ef;
						else e = en;
						el = t->workevent;
						n   = pat_modnote(e->note);
						ins = e->smpno;
						if( e->volume == 0 ) {
							m->param = (BYTE)modticks(h, e->tracktick - tt1);
							if( m->param ) { // note cut
								m->command = CMD_S3MCMDEX;
								m->param  |= 0xC0;
							}
							else {
								m->param   = 0;
								m->command = CMD_KEYOFF;
							}
							vol = 0;
						}
						else {
							vol = e->volume/2;
							if( el->volume == 0 ) {
								m->param = (BYTE)modticks(h, el->tracktick - tt1);
								if( m->param ) { // note cut
									m->command = CMD_S3MCMDEX;
									m->param  |= 0xC0;
								}
							}
							else {
								m->param = (BYTE)modticks(h, e->tracktick - tt1);
								if( m->param ) { // note delay
									m->command = CMD_S3MCMDEX;
									m->param  |= 0xD0;
								}
							}
						}
						m->instr  = ins;
						m->note   = n; // <- normal note
						m->volcmd = VOLCMD_VOLUME;
						m->vol    = vol;
					}
					else {
						// two notes in one row, use FINEPITCHSLIDE runonce effect
						// start first note on first tick and framedly runonce on seconds note tick
						// use volume and instrument of last note
						n   = pat_modnote(ef->note);
						i   = pat_modnote(el->note);
						ins = el->smpno;
						vol = el->volume/2;
						if( vol > 64 ) vol = 64;
						m->instr  = ins;
						m->note   = n; // <- normal note
						m->volcmd = VOLCMD_VOLUME;
						m->vol    = vol;
						m->param  = ((i > n)?i-n:n-i);
						if( m->param < 16 ) {
							if( m->param ) {
								m->command = CMD_XFINEPORTAUPDOWN;
								m->param |= (i > n)? 0x10: 0x20;
							}
							else {	// retrigger same note...
								m->command = CMD_RETRIG;
								m->param = (BYTE)modticks(h, el->tracktick - tt1);
							}
						}
						else
							m->command = (i > n)? CMD_PORTAMENTOUP: CMD_PORTAMENTODOWN;
					}
				}
				if( m->param == 0 && m->command == CMD_NONE ) {
					if( tempo ) {
						m->command = CMD_TEMPO;
						m->param   = tempo;
						tempo = 0;
					}
					else {
						if( patbrk ) {
							m->command = CMD_PATTERNBREAK;
							patbrk = 0;
						}
					}
				}
				ch++;
			}
			if( tempo || patbrk ) return 1;
		}
	}
	return 0;
}

static ULONG mid_next_tracktick(MIDEVENT *e)
{
	MIDEVENT *en;
	en = e->next;
	if( en ) return en->tracktick;
	return 0x7fffffff; // practically indefinite
}

// cut off alle events that follow the given event
static void mid_stripoff(MIDTRACK *tp, MIDEVENT *e)
{
	MIDEVENT *ep, *en;
	for( ep=e->next; ep; ep = en ) {
		en=ep->next;
		free(ep);
	}
	e->next  = NULL;
	tp->tail = e;
	tp->workevent = tp->head;
	mid_sync_track(tp, e->tracktick);
}

static void mid_notes_to_percussion(MIDTRACK *tp, ULONG adjust, ULONG tmin)
{
	MIDEVENT *e, *lno = 0;
	int n = 0,v;
	ULONG ton, toff = 0, tnext;
	v = 0x7f; // as loud as it gets
	ton = 0;
	for( e=tp->head; e; e=e->next ) {
		if( e->tracktick < adjust ) e->tracktick = 0;
		else e->tracktick -= adjust;
		if( e->flg == 1 ) {
			if( e->volume > 0 ) {
				n = e->note;
				e->smpno = pat_gmtosmp(pat_gm_drumnr(n));
				e->note = pat_gm_drumnote(n);
				e->volume = (v * e->volume) / 128;
				if( v && !e->volume ) e->volume = 1;
				ton = e->tracktick;
			}
			else {
				toff = ton + tmin;
				if( toff > e->tracktick ) {
					tnext = mid_next_tracktick(e);
					if( toff + tmin < tnext ) e->tracktick = toff;
					else {
						if( toff < tnext ) e->tracktick = toff - 1;
						else e->tracktick = tnext - 1;
					}
				}
				toff = e->tracktick;
				lno = e;
			}
		}
		else {
			if( e->fx == mainvol ) {
				v = e->fxparam;
				if( !v && ton > toff ) {
					e->flg = 1;
					e->volume = 0;
					e->note = pat_gm_drumnote(n);
					toff = e->tracktick;
					lno = e;
				}
			}
		}
	}
	if( ton > toff ) {
		char info[64];
		sprintf(info,"%ld > %ld note %d", (long)ton, (long)toff, n);
		mid_message("drum track ends with note on (%s)", info);
	}
	if( lno && lno->next ) mid_stripoff(tp, lno);
}

static void mid_prog_to_notes(MIDTRACK *tp, ULONG adjust, ULONG tmin)
{
	MIDEVENT *e, *lno = 0;
	int i = 0, n = 0, v = 0x7f;
	ULONG ton, toff = 0, tnext;
	ton = 0;
	for( e=tp->head; e; e=e->next ) {
		if( e->tracktick < adjust ) e->tracktick = 0;
		else e->tracktick -= adjust;
		if( e->flg == 1 ) {
			if( !i ) i = pat_gmtosmp(1); // happens in eternal2.mid
			e->smpno = i;
			n = e->note;
			if( e->volume > 0 ) {
				e->volume = (v * e->volume) / 128;
				if( v && !e->volume ) e->volume = 1;
				ton = e->tracktick;
			}
			else {
				toff = ton + tmin;
				if( toff > e->tracktick ) {
					tnext = mid_next_tracktick(e);
					if( toff + tmin < tnext ) e->tracktick = toff;
					else {
						if( toff < tnext ) e->tracktick = toff - 1;
						else e->tracktick = tnext - 1;
					}
				}
				toff = e->tracktick;
				lno = e;
			}
		}
		else {
			if( e->fx == prog ) i = e->fxparam;
			if( e->fx == mainvol ) {
				v = e->fxparam;
				if( !v && ton > toff ) {
					e->flg = 1;
					e->volume = 0;
					e->note = n;
					toff = e->tracktick;
					lno = e;
				}
			}
		}
	}
	if( ton > toff ) {
		char info[128];
		sprintf(info,"channel %d, %ld > %ld note %d", tp->chan + 1, (long)ton, (long)toff, n);
		mid_message("melody track ends with note on (%s)", info);
	}
	if( lno && lno->next ) mid_stripoff(tp, lno);
}

static int midiword(BYTE *b)
{
	int i;
	i = (b[0]&0x7f)|((b[1]&0x7f)<<7);
	return i;
}

static int midishort(BYTE *b)
{
	return midiword(b) - 0x2000;
}

ULONG mid_first_noteonevent_tick(MIDEVENT *e)
{
	while( e && (e->flg == 0 || e->volume == 0) ) e=e->next;
	if( !e ) return 0x7fffffff;
	return e->tracktick;
}

// =====================================================================================
BOOL CSoundFile::ReadMID(const BYTE *lpStream, DWORD dwMemLength)
{
	static int avoid_reentry = 0;
	MIDHANDLE *h;
	int ch, dmulti, maxtempo, panlow, panhigh, numchans, numtracks;
	MIDTRACK *ttp;
	uint32_t t, numpats;
	char buf[256];
	uint32_t miditracklen;
	BYTE runningstatus;
	BYTE cmd;
	BYTE midibyte[2];
	long metalen, delta;
	BYTE *p;
	while( avoid_reentry ) sleep(1);
	avoid_reentry = 1;
	if( !TestMID(lpStream, dwMemLength) ) goto ErrorExit;
	h = MID_Init();
	if( !h ) goto ErrorExit;
	h->mm = lpStream;
	h->sz = dwMemLength;
	h->err = 0;
	h->debug = getenv(ENV_MMMID_DEBUG);
	h->verbose = getenv(ENV_MMMID_VERBOSE);
	pat_resetsmp();
	pat_init_patnames();

	h->pos = 8;
	h->midiformat = mid_read_short(h);
	h->miditracks = mid_read_short(h);
	h->resolution = mid_read_short(h);
	if (h->err) goto ErrorCleanup;

	// at this point the h->pos is positioned at first miditrack
	if( h->midiformat == 0 ) h->miditracks = 1;
	if( h->resolution & 0x8000 )
		h->divider = ((h->resolution & 0x7f00)>>8)*(h->resolution & 0xff);
	else
		h->divider = h->resolution;
	h->divider <<= 2; // ticks per quartnote ==> ticks per note
	if (!h->divider) h->divider = 1;
	h->tempo = 122;
	m_nDefaultTempo = 0;
	h->tracktime = 0;
	h->speed = 6;
	if (h->miditracks == 0) goto ErrorCleanup;

	p = (BYTE *)getenv(ENV_MMMID_SPEED);
	if( p && isdigit(*p) && p[0] != '0' && p[1] == '\0' ) {
		// transform speed
		t = *p - '0';
		h->speed *= t;
		h->divider *= t;
		h->speed /= 6;
		h->divider /= 6;
	}
	// calculate optimal delta multiplier dmulti keeping tempo adjustments
	// from 10 to 255 in mind (hoping there will be no midi's with tempo's
	// lower than 10, that is sooo sick...)
	// this is necessary for the tracks to patterns routine
	dmulti = 1;
	maxtempo = h->divider;
	while( (h->midispeed = miditicks(h, h->speed)) * 10 < 255 * h->speed ) {
		++dmulti;
		h->divider = maxtempo * dmulti;
	}
	h->tp = NULL;
	memset(buf,0,sizeof(buf));
	strcpy(m_szNames[0], "");
	maxtempo = 0;
	panlow   = 64;
	panhigh  = 64;
	if( h->verbose ) {
		printf("Scanning MIDI with format: %d resolution: %d tracks: %d\n",
			h->midiformat,
			h->resolution,
			h->miditracks);
	}
	if( h->verbose && dmulti > 1 ) {
		printf("Multiplying resolution and deltatimes by %d to get %d miditicks per patternrow\n",
			dmulti, h->midispeed);
	}
	for( t=0; t<(uint32_t)h->miditracks; t++ ) {
		if( h->verbose ) printf("Parsing track %d\n", t+1);
		if (mid_read_bytes(buf,4,h) < 4) {
			buf[0] = '\0'; // make sure start is \0
		}
		buf[4] = '\0';
		if( strcmp(buf,"MTrk") ) {
			mid_message("invalid track-chunk '%s' is not 'MTrk'",buf);
			goto ErrorCleanup;
		}
		miditracklen = mid_read_long(h);
		if (h->err || h->sz < miditracklen) continue;
		runningstatus = 0;
		if( t && h->midiformat == 1 ) mid_rewind_tracks(h); // tracks sound simultaneously
		while( miditracklen > 0 ) {
			if (h->err) break;
			miditracklen -= mid_read_delta(h);
			midibyte[0] = mid_read_byte(h);
			miditracklen--;
			if( midibyte[0] & 0x80 ) {
				runningstatus = midibyte[0];
				switch( runningstatus ) {
					case 0xf1:
					case 0xf4:
					case 0xf5:
					case 0xf6:
					case 0xf7:
					case 0xf8:
					case 0xf9:
					case 0xfa:
					case 0xfb:
					case 0xfc:
					case 0xfd:
					case 0xfe:
						break;
					default:
						midibyte[0] = mid_read_byte(h);
						miditracklen--;
						break;
				}
			}
			h->tracktime += dmulti * h->deltatime;
			ch = runningstatus & 0x0f;
			cmd = runningstatus & 0xf0;
			switch( cmd ) {
				case 0x80: // note off
					midibyte[1] = mid_read_byte(h);
					miditracklen--;
					ttp = mid_find_track(h, ch, midibyte[0]);
					if( ttp ) mid_add_noteoff(h, ttp);
					if( h->debug )
						printf("%2d %08ld       Note off: ch %d 0x%02x 0x%02x\n",
						       t, (long)(h->tracktime),
						       ch + 1, midibyte[0], midibyte[1]);
					break;
				case 0x90: // note on
					midibyte[1] = mid_read_byte(h);
					miditracklen--;
					if( midibyte[1] ) {
						ttp = mid_locate_track(h, ch, midibyte[0]);
						mid_add_noteon(h, ttp, midibyte[0], midibyte[1]);
						if( h->debug )
							printf("%2d %08ld Note  on: ch %d 0x%02x 0x%02x\n",
							       t, (long)(h->tracktime),
							       ch + 1, midibyte[0], midibyte[1]);
					}
					else {
						ttp = mid_find_track(h, ch, midibyte[0]);
						if( ttp ) mid_add_noteoff(h, ttp);
						if( h->debug )
							printf("%2d %08ld note off: ch %d 0x%02x\n",
							t, (long)(h->tracktime),
							ch + 1, midibyte[0]);
					}
					break;
				case 0xa0: // polyphonic key pressure
					midibyte[1] = mid_read_byte(h);
					miditracklen--;
					if( h->debug )
						printf("%2d %08ld polyphonic key pressure: ch %d 0x%02x 0x%02x\n", t, (long)(h->tracktime), ch + 1, midibyte[0], midibyte[1]);
					break;
				case 0xb0: // control change
					midibyte[1] = mid_read_byte(h);
					miditracklen--;
					switch(midibyte[0]) {
						case 0x01: // mod wheel
							mid_mod_wheel(h, ch, midibyte[1]);
							break;
						case 0x07: // main volume
							mid_main_volume(h, ch, midibyte[1]);
							break;
						case 0x0a: // pan
							if( midibyte[1] < panlow )  panlow  = midibyte[1];
							if( midibyte[1] > panhigh ) panhigh = midibyte[1];
							mid_pan(h, ch, midibyte[1]);
							break;
						case 0x0b: // expression
							break;
						case 0x7b:
							if( midibyte[1] == 0x00 ) // all notes off
								mid_all_notes_off(h, ch);
							break;
						default:
							break;
					}
					if( h->debug )
						printf("%2d %08ld control change: ch %d 0x%02x 0x%02x\n",
						t, (long)(h->tracktime), ch + 1, midibyte[0], midibyte[1]);
					break;
				case 0xc0: // program change
					mid_add_program(h, ch, midibyte[0]);
					if( h->debug )
						printf("%2d %08ld program change: ch %d %d\n",
						t, (long)(h->tracktime), ch + 1, midibyte[0]);
					break;
				case 0xd0: // channel pressure
					if( h->debug )
						printf("%2d %08ld channel pressure: ch %d 0x%02x\n", t, (long)(h->tracktime), ch + 1, midibyte[0]);
					break;
				case 0xe0: // pitch wheel change
					midibyte[1] = mid_read_byte(h);
					miditracklen--;
					if( h->debug )
						printf("%2d %08ld pitch wheel change: ch %d %d\n",
						t, (long)(h->tracktime), ch + 1, midishort(midibyte));
					mid_add_pitchwheel(h, ch, midishort(midibyte));
					break;
				case 0xf0: // system & realtime
					switch( runningstatus ) {
						case 0xf0:	// sysex
							if( h->debug ) printf("%2d %08ld sysex: 0x%02x",
								t, (long)(h->tracktime), midibyte[0]);
							while( midibyte[0] != 0xf7 ) {
								midibyte[0] = mid_read_byte(h);
								if (h->err) break;
								miditracklen--;
								if( h->debug ) printf(" %02X", midibyte[0]);
							}
							if( h->debug ) printf("\n");
							break;
						case 0xf2:	// song position pointer
							midibyte[1] = mid_read_byte(h);
							miditracklen--;
							if( h->debug )
								printf("%2d %08ld song position pointer: %d",
								t, (long)(h->tracktime), midishort(midibyte));
							break;
						case 0xf7:
							delta = h->deltatime;
							miditracklen -= mid_read_delta(h);
							metalen = h->deltatime;
							if( h->debug )
								printf("%2d %08ld sysex continued: %ld",
								t, (long)(h->tracktime), metalen);
							while( metalen > 0 ) {
								midibyte[1] = mid_read_byte(h);
								if (h->err) break;
								metalen--;
								miditracklen--;
								if( h->debug ) printf(" %02X", midibyte[1]);
							}
							h->deltatime = delta;
							break;
						case 0xff: // meta event
							delta = h->deltatime;
							miditracklen -= mid_read_delta(h);
							metalen = h->deltatime;
							if( metalen > 31 ) metalen = 31;
							if( metalen ) {
								if (!mid_read_bytes(buf, metalen, h)) break;
								miditracklen -= metalen;
							}
							buf[metalen] = '\0';
							metalen = h->deltatime - metalen;
							while( metalen > 0 ) {
								midibyte[1] = mid_read_byte(h);
								if (h->err) break;
								metalen--;
								miditracklen--;
							}
							h->deltatime = delta;
							switch( midibyte[0] ) {
								case 0x03: // type: track name
									if( h->debug )
										printf("%2d %08ld META trackname:%s\n", t, (long)(h->tracktime), buf);
									if( m_szNames[0][0] == '\0' )
										strcpy(m_szNames[0], buf);
									break;
								case 0x51: // type: tempo
									p=(BYTE *)buf;
									delta = (p[0]<<16)|(p[1]<<8)|p[2];
									if( delta )
										h->tempo = 60000000 / delta;
									if( h->debug ) printf("%2d %08ld META tempo:%d\n", t, (long)(h->tracktime), h->tempo);
									if( m_nDefaultTempo == 0 ) m_nDefaultTempo = h->tempo;
									else {
										ttp = h->track;
										if( !ttp ) mid_locate_track(h, 0, 0xff);
										mid_add_tempo_event(h,h->tempo);
									}
									if( h->tempo > maxtempo ) maxtempo = h->tempo;
									break;
								case 0x2f: // type: end of track
									if( h->debug ) printf("%2d %08ld META end of track\n", t, (long)(h->tracktime));
									if( miditracklen > 0 ) {
										sprintf(buf, "%u", miditracklen);
										mid_message("Meta event not at end of track, %s bytes left in track", buf);
										miditracklen = 0;
									}
									break;
								default:
									if( h->debug ) printf("%2d %08ld META type 0x%02x\n", t, (long)(h->tracktime), midibyte[0]);
									break;
							}
							break;
						default:
							if( h->debug ) printf("%2d %08ld System type 0x%02x\n", t, (long)(h->tracktime), midibyte[0]);
							break;
					}
					break;
				default:   // no running status, just skip it...
					if( h->debug ) printf("%2d %08ld unknown runningstatus: 0x%02x skipped:0x%02x\n", t, (long)(h->tracktime), runningstatus, midibyte[0]);
					break;
			}
			if( miditracklen < 1 && (runningstatus != 0xff || midibyte[0] != 0x2f) ) {
				delta = h->pos;
				if (!mid_read_bytes(buf,4,h)) break;
				buf[4] = '\0';
				if( strcmp(buf,"MTrk") ) {
					miditracklen = 0x7fffffff;
					mid_message("Meta event not at end of track, %s bytes left in track", "superfluous");
				}
				else
					mid_message("Meta event not at end of track, %s bytes left in track", "no");
				h->pos = delta;
			}
		}
	}
	if( h->verbose ) printf("Determining percussion channel\n");
	// get the lowest event time and the used channels
	delta = 0x7fffffff;
	metalen = 0; // use as bit bucket for used channels
	for( ttp=h->track; ttp; ttp=ttp->next ) {
		metalen |= (1<<ttp->chan);
		if( ttp->head ) {
			ULONG tt;
			tt = mid_first_noteonevent_tick(ttp->head);
			if( tt < (ULONG)delta )
				delta = tt;
		}
	}
	if( metalen & 0x03ff ) {
		if( (metalen & 0x0f00) == 0x0400 )
			h->percussion = 10; // buggy sng2mid uses channel 10
		else
			h->percussion = 9;
	}
	else h->percussion = 15;
	if( h->verbose )
		printf("Percussion channel is %d\nStripping off silences and other optimalisations\n", h->percussion + 1);
	// last but not least shut off all pending events, transform drumnotes when appropriate
	// strip off silences at begin and end and get the greatest tracktime
	h->tracktime = 0;
	metalen = h->midispeed;
	for( ttp=h->track; ttp; ttp=ttp->next ) {
		if( ttp->chan == h->percussion )
			mid_notes_to_percussion(ttp, delta, metalen);
		else
			mid_prog_to_notes(ttp, delta, metalen);
		if( ttp->tail && ttp->tail->tracktick > h->tracktime )
			h->tracktime = ttp->tail->tracktick;
	}

	h->tracktime += h->divider >> 2; // add one quartnote to the song for silence
	if ( h->track )
		mid_add_partbreak(h);
	if( h->debug )
		mid_dump_tracks(h);
	numchans = mid_numchans(h);
	if( panlow > 48 || panhigh < 80 ) {
		for( ttp=h->track; ttp; ttp=ttp->next ) {
			ttp->balance = ((0x40*numchans+0x80*mid_ordchan(h, ttp->chan))/numchans)&0x7f;
		}
	}
	// set module variables
	numtracks = mid_numtracks(h);
	if( m_nDefaultTempo == 0 ) m_nDefaultTempo = h->tempo;
	if( maxtempo == 0 ) maxtempo = h->tempo;
	if( maxtempo != 255 ) {
		if( h->verbose ) printf("Adjusting tempo %d to 255\n", maxtempo);
		mid_adjust_for_optimal_tempo(h, maxtempo);
	}
	if( maxtempo > 0 ) m_nDefaultTempo = (255 * m_nDefaultTempo) / maxtempo;

	numpats = 1 + (modticks(h, h->tracktime) / h->speed / 64 );
	if (numpats > MAX_PATTERNS) numpats = MAX_PATTERNS;

	if( h->verbose ) printf("Generating %d patterns with speed %d\n", numpats, h->speed);
	m_nType         = MOD_TYPE_MID;
	m_nDefaultSpeed = h->speed;
	m_nChannels     = numtracks;
	m_dwSongFlags   = SONG_LINEARSLIDES;
	m_nMinPeriod    = 28 << 2;
	m_nMaxPeriod    = 1712 << 3;
	if (m_nChannels == 0) goto ErrorCleanup;

	// orderlist
	for(t=0; t < numpats; t++)
		Order[t] = t;
	if( !PAT_Load_Instruments(this) ) goto ErrorCleanup;

	// ==============================
	// Load the pattern info now!
	if( MID_ReadPatterns(Patterns, PatternSize, h, numpats, m_nChannels) ) {
		// :^(  need one more channel to handle the global events ;^b
		m_nChannels++;
		h->tp = mid_new_track(h, h->track->chan, 0xff);
		for( ttp=h->track; ttp->next; ttp=ttp->next ) ;
		ttp->next = h->tp;
		mid_add_sync(h, h->tp);
		for( t=0; t<numpats; t++ ) {
			FreePattern(Patterns[t]);
			Patterns[t] = NULL;
		}
		MID_ReadPatterns(Patterns, PatternSize, h, numpats, m_nChannels);
	}
	// ============================================================
	// set panning positions
	t = 0;
	for( ttp=h->track; ttp; ttp=ttp->next ) {
		ChnSettings[t].nPan    = modpan(ttp->balance, numchans / 2);
		ChnSettings[t].nVolume = 64;
		t++;
	}
	if( h->verbose ) printf("Cleanup.\n");
	MID_Cleanup(h);	// we dont need it anymore
	avoid_reentry = 0; // it is safe now, I'm finished
	return TRUE;

ErrorCleanup:
	MID_Cleanup(h);
ErrorExit:
	avoid_reentry = 0;
	return FALSE;
}
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (endian and char fixes for PPC)
*/

//#include "stdafx.h"
//#include "sndfile.h"
//#include "tables.h"

#ifdef _MSC_VER
//#pragma warning(disable:4244)
#endif

//////////////////////////////////////////////////////////
// ProTracker / NoiseTracker MOD/NST file support

void CSoundFile::ConvertModCommand(MODCOMMAND *m) const
//-----------------------------------------------------
{
	UINT command = m->command, param = m->param;

	switch(command)
	{
	case 0x00:	if (param) command = CMD_ARPEGGIO; break;
	case 0x01:	command = CMD_PORTAMENTOUP; break;
	case 0x02:	command = CMD_PORTAMENTODOWN; break;
	case 0x03:	command = CMD_TONEPORTAMENTO; break;
	case 0x04:	command = CMD_VIBRATO; break;
	case 0x05:	command = CMD_TONEPORTAVOL; if (param & 0xF0) param &= 0xF0; break;
	case 0x06:	command = CMD_VIBRATOVOL; if (param & 0xF0) param &= 0xF0; break;
	case 0x07:	command = CMD_TREMOLO; break;
	case 0x08:	command = CMD_PANNING8; break;
	case 0x09:	command = CMD_OFFSET; break;
	case 0x0A:	command = CMD_VOLUMESLIDE; if (param & 0xF0) param &= 0xF0; break;
	case 0x0B:	command = CMD_POSITIONJUMP; break;
	case 0x0C:	command = CMD_VOLUME; break;
	case 0x0D:	command = CMD_PATTERNBREAK; param = ((param >> 4) * 10) + (param & 0x0F); break;
	case 0x0E:	command = CMD_MODCMDEX; break;
	case 0x0F:	command = (param <= (UINT)((m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2)) ? 0x1F : 0x20)) ? CMD_SPEED : CMD_TEMPO;
			if((param == 0xFF) && (m_nSamples == 15))
			    command = 0;
			break;
	// Extension for XM extended effects
	case 'G' - 55:	command = CMD_GLOBALVOLUME; break;
	case 'H' - 55:	command = CMD_GLOBALVOLSLIDE; if (param & 0xF0) param &= 0xF0; break;
	case 'K' - 55:	command = CMD_KEYOFF; break;
	case 'L' - 55:	command = CMD_SETENVPOSITION; break;
	case 'M' - 55:	command = CMD_CHANNELVOLUME; break;
	case 'N' - 55:	command = CMD_CHANNELVOLSLIDE; break;
	case 'P' - 55:	command = CMD_PANNINGSLIDE; if (param & 0xF0) param &= 0xF0; break;
	case 'R' - 55:	command = CMD_RETRIG; break;
	case 'T' - 55:	command = CMD_TREMOR; break;
	case 'X' - 55:	command = CMD_XFINEPORTAUPDOWN;	break;
	case 'Y' - 55:	command = CMD_PANBRELLO; break;
	case 'Z' - 55:	command = CMD_MIDI;	break;
	default:	command = 0;
	}
	m->command = command;
	m->param = param;
}


WORD CSoundFile::ModSaveCommand(const MODCOMMAND *m, BOOL bXM) const
//------------------------------------------------------------------
{
	UINT command = m->command & 0x3F, param = m->param;

	switch(command)
	{
	case 0:						command = param = 0; break;
	case CMD_ARPEGGIO:			command = 0; break;
	case CMD_PORTAMENTOUP:
		if (m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_STM))
		{
			if ((param & 0xF0) == 0xE0) { command=0x0E; param=((param & 0x0F) >> 2)|0x10; break; }
			else if ((param & 0xF0) == 0xF0) { command=0x0E; param &= 0x0F; param|=0x10; break; }
		}
		command = 0x01;
		break;
	case CMD_PORTAMENTODOWN:
		if (m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_STM))
		{
			if ((param & 0xF0) == 0xE0) { command=0x0E; param=((param & 0x0F) >> 2)|0x20; break; }
			else if ((param & 0xF0) == 0xF0) { command=0x0E; param &= 0x0F; param|=0x20; break; }
		}
		command = 0x02;
		break;
	case CMD_TONEPORTAMENTO:	command = 0x03; break;
	case CMD_VIBRATO:			command = 0x04; break;
	case CMD_TONEPORTAVOL:		command = 0x05; break;
	case CMD_VIBRATOVOL:		command = 0x06; break;
	case CMD_TREMOLO:			command = 0x07; break;
	case CMD_PANNING8:			
		command = 0x08;
		if (bXM)
		{
			if ((m_nType != MOD_TYPE_IT) && (m_nType != MOD_TYPE_XM) && (param <= 0x80))
			{
				param <<= 1;
				if (param > 255) param = 255;
			}
		} else
		{
			if ((m_nType == MOD_TYPE_IT) || (m_nType == MOD_TYPE_XM)) param >>= 1;
		}
		break;
	case CMD_OFFSET:			command = 0x09; break;
	case CMD_VOLUMESLIDE:		command = 0x0A; break;
	case CMD_POSITIONJUMP:		command = 0x0B; break;
	case CMD_VOLUME:			command = 0x0C; break;
	case CMD_PATTERNBREAK:		command = 0x0D; param = ((param / 10) << 4) | (param % 10); break;
	case CMD_MODCMDEX:			command = 0x0E; break;
	case CMD_SPEED:				command = 0x0F; if (param > 0x20) param = 0x20; break;
	case CMD_TEMPO:				if (param > 0x20) { command = 0x0F; break; }
	case CMD_GLOBALVOLUME:		command = 'G' - 55; break;
	case CMD_GLOBALVOLSLIDE:	command = 'H' - 55; break;
	case CMD_KEYOFF:			command = 'K' - 55; break;
	case CMD_SETENVPOSITION:	command = 'L' - 55; break;
	case CMD_CHANNELVOLUME:		command = 'M' - 55; break;
	case CMD_CHANNELVOLSLIDE:	command = 'N' - 55; break;
	case CMD_PANNINGSLIDE:		command = 'P' - 55; break;
	case CMD_RETRIG:			command = 'R' - 55; break;
	case CMD_TREMOR:			command = 'T' - 55; break;
	case CMD_XFINEPORTAUPDOWN:	command = 'X' - 55; break;
	case CMD_PANBRELLO:			command = 'Y' - 55; break;
	case CMD_MIDI:				command = 'Z' - 55; break;
	case CMD_S3MCMDEX:
		switch(param & 0xF0)
		{
		case 0x10:	command = 0x0E; param = (param & 0x0F) | 0x30; break;
		case 0x20:	command = 0x0E; param = (param & 0x0F) | 0x50; break;
		case 0x30:	command = 0x0E; param = (param & 0x0F) | 0x40; break;
		case 0x40:	command = 0x0E; param = (param & 0x0F) | 0x70; break;
		case 0x90:	command = 'X' - 55; break;
		case 0xB0:	command = 0x0E; param = (param & 0x0F) | 0x60; break;
		case 0xA0:
		case 0x50:
		case 0x70:
		case 0x60:	command = param = 0; break;
		default:	command = 0x0E; break;
		}
		break;
	default:		command = param = 0;
	}
	return (WORD)((command << 8) | (param));
}

#pragma pack(1)

typedef struct _MODSAMPLE
{
	CHAR name[22];
	WORD length;
	BYTE finetune;
	BYTE volume;
	WORD loopstart;
	WORD looplen;
} MODSAMPLE, *PMODSAMPLE;

typedef struct _MODMAGIC
{
	BYTE nOrders;
	BYTE nRestartPos;
	BYTE Orders[128];
        char Magic[4];          // changed from CHAR
} MODMAGIC, *PMODMAGIC;

#pragma pack()

static BOOL IsValidName(LPCSTR s, int length, CHAR minChar)
//-----------------------------------------------------------------
{
	int i, nt;
	for (i = 0, nt = 0; i < length; i++)
	{
		if(s[i])
		{
			if (nt) return FALSE;// garbage after null
			if (s[i] < minChar) return FALSE;// caller says it's garbage
		}
		else if (!nt) nt = i;// found null terminator
	}
	return TRUE;
}

static BOOL IsMagic(LPCSTR s1, LPCSTR s2)
{
	return ((*(DWORD *)s1) == (*(DWORD *)s2)) ? TRUE : FALSE;
}

BOOL CSoundFile::ReadMod(const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
        char s[1024];          // changed from CHAR
	DWORD dwMemPos, dwTotalSampleLen;
	PMODMAGIC pMagic;
	UINT nErr;

	if ((!lpStream) || (dwMemLength < 0x600)) return FALSE;
	dwMemPos = 20;
	m_nSamples = 31;
	m_nChannels = 4;
	pMagic = (PMODMAGIC)(lpStream+dwMemPos+sizeof(MODSAMPLE)*31);
	// Check Mod Magic
	memcpy(s, pMagic->Magic, 4);
	if ((IsMagic(s, "M.K.")) || (IsMagic(s, "M!K!"))
	 || (IsMagic(s, "M&K!")) || (IsMagic(s, "N.T."))) m_nChannels = 4; else
	if ((IsMagic(s, "CD81")) || (IsMagic(s, "OKTA"))) m_nChannels = 8; else
	if (IsMagic(s, "CD61")) m_nChannels = 6; else
	if ((s[0]=='F') && (s[1]=='L') && (s[2]=='T') && (s[3]>='4') && (s[3]<='9')) m_nChannels = s[3] - '0'; else
	if ((s[0]>='2') && (s[0]<='9') && (s[1]=='C') && (s[2]=='H') && (s[3]=='N')) m_nChannels = s[0] - '0'; else
	if ((s[0]=='1') && (s[1]>='0') && (s[1]<='9') && (s[2]=='C') && (s[3]=='H')) m_nChannels = s[1] - '0' + 10; else
	if ((s[0]=='2') && (s[1]>='0') && (s[1]<='9') && (s[2]=='C') && (s[3]=='H')) m_nChannels = s[1] - '0' + 20; else
	if ((s[0]=='3') && (s[1]>='0') && (s[1]<='2') && (s[2]=='C') && (s[3]=='H')) m_nChannels = s[1] - '0' + 30; else
	if ((s[0]=='T') && (s[1]=='D') && (s[2]=='Z') && (s[3]>='4') && (s[3]<='9')) m_nChannels = s[3] - '0'; else
	if (IsMagic(s,"16CN")) m_nChannels = 16; else
	if (IsMagic(s,"32CN")) m_nChannels = 32;
	else {
		if (!IsValidName((LPCSTR)lpStream, 20, ' '))
			return FALSE;
		m_nSamples = 15;
	}
	// Load Samples
	nErr = 0;
	dwTotalSampleLen = 0;
	for	(UINT i=1; i<=m_nSamples; i++)
	{
		PMODSAMPLE pms = (PMODSAMPLE)(lpStream+dwMemPos);
		MODINSTRUMENT *psmp = &Ins[i];
		UINT loopstart, looplen;

		if (m_nSamples == 15)
		{
			if (!IsValidName((LPCSTR)pms->name, 22, 14)) return FALSE;
			if (pms->finetune>>4) return FALSE;
			if (pms->volume > 64) return FALSE;
			if (bswapBE16(pms->length) > 32768) return FALSE;
		}

		memcpy(m_szNames[i], pms->name, 22);
		m_szNames[i][22] = 0;
		psmp->uFlags = 0;
		psmp->nLength = bswapBE16(pms->length)*2;
		dwTotalSampleLen += psmp->nLength;
		psmp->nFineTune = MOD2XMFineTune(pms->finetune & 0x0F);
		psmp->nVolume = 4*pms->volume;
		if (psmp->nVolume > 256) { psmp->nVolume = 256; nErr++; }
		psmp->nGlobalVol = 64;
		psmp->nPan = 128;
		loopstart = bswapBE16(pms->loopstart)*2;
		looplen = bswapBE16(pms->looplen)*2;
		// Fix loops
		if ((looplen > 2) && (loopstart+looplen > psmp->nLength)
		 && (loopstart/2+looplen <= psmp->nLength))
		{
			loopstart /= 2;
		}
		psmp->nLoopStart = loopstart;
		psmp->nLoopEnd = loopstart + looplen;
		if (psmp->nLength < 4) psmp->nLength = 0;
		if (psmp->nLength)
		{
			if (psmp->nLoopStart >= psmp->nLength) { psmp->nLoopStart = psmp->nLength-1; }
			if (psmp->nLoopEnd > psmp->nLength) { psmp->nLoopEnd = psmp->nLength; }

			if ((psmp->nLoopStart > psmp->nLoopEnd) || (psmp->nLoopEnd <= 8)
			 || (psmp->nLoopEnd - psmp->nLoopStart <= 4))
			{
				psmp->nLoopStart = 0;
				psmp->nLoopEnd = 0;
			}
			if (psmp->nLoopEnd > psmp->nLoopStart)
			{
				psmp->uFlags |= CHN_LOOP;
			}
		}
		dwMemPos += sizeof(MODSAMPLE);
	}
	if ((m_nSamples == 15) && (dwTotalSampleLen > dwMemLength * 4)) return FALSE;
	pMagic = (PMODMAGIC)(lpStream+dwMemPos);
	dwMemPos += sizeof(MODMAGIC);
	if (m_nSamples == 15) {
		dwMemPos -= 4;
		if (pMagic->nOrders > 128) return FALSE;
	}
	memset(Order, 0,sizeof(Order));
	memcpy(Order, pMagic->Orders, 128);

	UINT nbp, nbpbuggy, nbpbuggy2, norders;

	norders = pMagic->nOrders;
	if ((!norders) || (norders > 0x80))
	{
		norders = 0x80;
		while ((norders > 1) && (!Order[norders-1])) norders--;
	}
	nbpbuggy = 0;
	nbpbuggy2 = 0;
	nbp = 0;
	for (UINT iord=0; iord<128; iord++)
	{
		UINT i = Order[iord];
		if ((i < 0x80) && (nbp <= i))
		{
			nbp = i+1;
			if (iord<norders) nbpbuggy = nbp;
		}
		if (i >= nbpbuggy2) nbpbuggy2 = i+1;
	}
	for (UINT iend=norders; iend<MAX_ORDERS; iend++) Order[iend] = 0xFF;
	norders--;
	m_nRestartPos = pMagic->nRestartPos;
	if (m_nRestartPos >= 0x78) m_nRestartPos = 0;
	if (m_nRestartPos + 1 >= (UINT)norders) m_nRestartPos = 0;
	if (!nbp) return FALSE;
	DWORD dwWowTest = dwTotalSampleLen+dwMemPos;
	if ((IsMagic(pMagic->Magic, "M.K.")) && (dwWowTest + nbp*8*256 == dwMemLength)) m_nChannels = 8;
	if ((nbp != nbpbuggy) && (dwWowTest + nbp*m_nChannels*256 != dwMemLength))
	{
		if (dwWowTest + nbpbuggy*m_nChannels*256 == dwMemLength) nbp = nbpbuggy;
		else nErr += 8;
	} else
	if ((nbpbuggy2 > nbp) && (dwWowTest + nbpbuggy2*m_nChannels*256 == dwMemLength))
	{
		nbp = nbpbuggy2;
	}
	if ((dwWowTest < 0x600) || (dwWowTest > dwMemLength)) nErr += 8;
	if ((m_nSamples == 15) && (nErr >= 16)) return FALSE;
	// Default settings	
	m_nType = MOD_TYPE_MOD;
	m_nDefaultSpeed = 6;
	m_nDefaultTempo = 125;
	m_nMinPeriod = 14 << 2;
	m_nMaxPeriod = 3424 << 2;
	memcpy(m_szNames, lpStream, 20);
	// Setting channels pan
	for (UINT ich=0; ich<m_nChannels; ich++)
	{
		ChnSettings[ich].nVolume = 64;
		if (gdwSoundSetup & SNDMIX_MAXDEFAULTPAN)
			ChnSettings[ich].nPan = (((ich&3)==1) || ((ich&3)==2)) ? 256 : 0;
		else
			ChnSettings[ich].nPan = (((ich&3)==1) || ((ich&3)==2)) ? 0xC0 : 0x40;
	}
	// Reading channels
	for (UINT ipat=0; ipat<nbp; ipat++)
	{
		if (ipat < MAX_PATTERNS)
		{
			if ((Patterns[ipat] = AllocatePattern(64, m_nChannels)) == NULL) break;
			PatternSize[ipat] = 64;
			if (dwMemPos + m_nChannels*256 >= dwMemLength) break;
			MODCOMMAND *m = Patterns[ipat];
			LPCBYTE p = lpStream + dwMemPos;
			for (UINT j=m_nChannels*64; j; m++,p+=4,j--)
			{
				BYTE A0=p[0], A1=p[1], A2=p[2], A3=p[3];
				UINT n = ((((UINT)A0 & 0x0F) << 8) | (A1));
				if ((n) && (n != 0xFFF)) m->note = GetNoteFromPeriod(n << 2);
				m->instr = ((UINT)A2 >> 4) | (A0 & 0x10);
				m->command = A2 & 0x0F;
				m->param = A3;
				if ((m->command) || (m->param)) ConvertModCommand(m);
			}
		}
		dwMemPos += m_nChannels*256;
	}
	// Reading instruments
	DWORD dwErrCheck = 0;
	for (UINT ismp=1; ismp<=m_nSamples; ismp++) if (Ins[ismp].nLength)
	{
		LPSTR p = (LPSTR)(lpStream+dwMemPos);
		UINT flags = 0;
		if (dwMemPos + 5 >= dwMemLength) break;
		if (! strncmp(p, "ADPCM", 5))
		{
			flags = 3;
			p += 5;
			dwMemPos += 5;
		}
		DWORD dwSize = ReadSample(&Ins[ismp], flags, p, dwMemLength - dwMemPos);
		if (dwSize)
		{
			dwMemPos += dwSize;
			dwErrCheck++;
		}
	}
#ifdef MODPLUG_TRACKER
	return TRUE;
#else
	return (dwErrCheck) ? TRUE : FALSE;
#endif
}


#ifndef MODPLUG_NO_FILESAVE

#ifdef _MSC_VER
#pragma warning(disable:4100)
#endif

BOOL CSoundFile::SaveMod(LPCSTR lpszFileName, UINT nPacking)
//----------------------------------------------------------
{
	BYTE insmap[32];
	UINT inslen[32];
	BYTE bTab[32];
	BYTE ord[128];
	FILE *f;

	if ((!m_nChannels) || (!lpszFileName)) return FALSE;
	if ((f = fopen(lpszFileName, "wb")) == NULL) return FALSE;
	memset(ord, 0, sizeof(ord));
	memset(inslen, 0, sizeof(inslen));
	if (m_nInstruments)
	{
		memset(insmap, 0, sizeof(insmap));
		for (UINT i=1; i<32; i++) if (Headers[i])
		{
			for (UINT j=0; j<128; j++) if (Headers[i]->Keyboard[j])
			{
				insmap[i] = Headers[i]->Keyboard[j];
				break;
			}
		}
	} else
	{
		for (UINT i=0; i<32; i++) insmap[i] = (BYTE)i;
	}
	// Writing song name
	fwrite(m_szNames, 20, 1, f);
	// Writing instrument definition
	for (UINT iins=1; iins<=31; iins++)
	{
		MODINSTRUMENT *pins = &Ins[insmap[iins]];
		memcpy(bTab, m_szNames[iins],22);
		inslen[iins] = pins->nLength;
		if (inslen[iins] > 0x1fff0) inslen[iins] = 0x1fff0;
		bTab[22] = inslen[iins] >> 9;
		bTab[23] = inslen[iins] >> 1;
		if (pins->RelativeTone < 0) bTab[24] = 0x08; else
		if (pins->RelativeTone > 0) bTab[24] = 0x07; else
		bTab[24] = (BYTE)XM2MODFineTune(pins->nFineTune);
		bTab[25] = pins->nVolume >> 2;
		bTab[26] = pins->nLoopStart >> 9;
		bTab[27] = pins->nLoopStart >> 1;
		bTab[28] = (pins->nLoopEnd - pins->nLoopStart) >> 9;
		bTab[29] = (pins->nLoopEnd - pins->nLoopStart) >> 1;
		fwrite(bTab, 30, 1, f);
	}
	// Writing number of patterns
	UINT nbp=0, norders=128;
	for (UINT iord=0; iord<128; iord++)
	{
		if (Order[iord] == 0xFF)
		{
			norders = iord;
			break;
		}
		if ((Order[iord] < 0x80) && (nbp<=Order[iord])) nbp = Order[iord]+1;
	}
	bTab[0] = norders;
	bTab[1] = m_nRestartPos;
	fwrite(bTab, 2, 1, f);
	// Writing pattern list
	if (norders) memcpy(ord, Order, norders);
	fwrite(ord, 128, 1, f);
	// Writing signature
	char bTab_buf[] = "0CHN"; bTab_buf[0] = m_nChannels + '0';
	fwrite(m_nChannels == 4 ? "M.K." : bTab_buf, 4, 1, f);
	// Writing patterns
	for (UINT ipat=0; ipat<nbp; ipat++) if (Patterns[ipat])
	{
		BYTE s[64*4];
		MODCOMMAND *m = Patterns[ipat];
		for (UINT i=0; i<64; i++) if (i < PatternSize[ipat])
		{
			LPBYTE p=s;
			for (UINT c=0; c<m_nChannels; c++,p+=4,m++)
			{
				UINT param = ModSaveCommand(m, FALSE);
				UINT command = param >> 8;
				param &= 0xFF;
				if (command > 0x0F) command = param = 0;
				if ((m->vol >= 0x10) && (m->vol <= 0x50) && (!command) && (!param)) { command = 0x0C; param = m->vol - 0x10; }
				UINT period = m->note;
				if (period)
				{
					if (period < 37) period = 37;
					period -= 37;
					if (period >= 6*12) period = 6*12-1;
					period = ProTrackerPeriodTable[period];
				}
				UINT instr = (m->instr > 31) ? 0 : m->instr;
				p[0] = ((period >> 8) & 0x0F) | (instr & 0x10);
				p[1] = period & 0xFF;
				p[2] = ((instr & 0x0F) << 4) | (command & 0x0F);
				p[3] = param;
			}
			fwrite(s, m_nChannels, 4, f);
		} else
		{
			memset(s, 0, m_nChannels*4);
			fwrite(s, m_nChannels, 4, f);
		}
	}
	// Writing instruments
	for (UINT ismpd=1; ismpd<=31; ismpd++) if (inslen[ismpd])
	{
		MODINSTRUMENT *pins = &Ins[insmap[ismpd]];
		UINT flags = RS_PCM8S;
#ifndef NO_PACKING
		if (!(pins->uFlags & (CHN_16BIT|CHN_STEREO)))
		{
			if ((nPacking) && (CanPackSample((char *)pins->pSample, inslen[ismpd], nPacking)))
			{
				fwrite("ADPCM", 1, 5, f);
				flags = RS_ADPCM4;
			}
		}
#endif
		WriteSample(f, pins, flags, inslen[ismpd]);
	}
	fclose(f);
	return TRUE;
}

#ifdef _MSC_VER
#pragma warning(default:4100)
#endif

#endif // MODPLUG_NO_FILESAVE
//#include "stdafx.h"
//#include "sndfile.h"

//#define MT2DEBUG

#pragma pack(1)

typedef struct _MT2FILEHEADER
{
	DWORD dwMT20;	// 0x3032544D "MT20"
	DWORD dwSpecial;
	WORD wVersion;
	CHAR szTrackerName[32];	// "MadTracker 2.0"
	CHAR szSongName[64];
	WORD nOrders;
	WORD wRestart;
	WORD wPatterns;
	WORD wChannels;
	WORD wSamplesPerTick;
	BYTE bTicksPerLine;
	BYTE bLinesPerBeat;
	DWORD fulFlags; // b0=packed patterns
	WORD wInstruments;
	WORD wSamples;
	BYTE Orders[256];
} MT2FILEHEADER;

typedef struct _MT2PATTERN
{
	WORD wLines;
	DWORD wDataLen;
} MT2PATTERN;

typedef struct _MT2COMMAND
{
	BYTE note;	// 0=nothing, 97=note off
	BYTE instr;
	BYTE vol;
	BYTE pan;
	BYTE fxcmd;
	BYTE fxparam1;
	BYTE fxparam2;
} MT2COMMAND;

typedef struct _MT2DRUMSDATA
{
	WORD wDrumPatterns;
	WORD wDrumSamples[8];
	BYTE DrumPatternOrder[256];
} MT2DRUMSDATA;

typedef struct _MT2AUTOMATION
{
	DWORD dwFlags;
	DWORD dwEffectId;
	DWORD nEnvPoints;
} MT2AUTOMATION;

typedef struct _MT2INSTRUMENT
{
	CHAR szName[32];
	DWORD dwDataLen;
	WORD wSamples;
	BYTE GroupsMapping[96];
	BYTE bVibType;
	BYTE bVibSweep;
	BYTE bVibDepth;
	BYTE bVibRate;
	WORD wFadeOut;
	WORD wNNA;
	WORD wInstrFlags;
	WORD wEnvFlags1;
	WORD wEnvFlags2;
} MT2INSTRUMENT;

typedef struct _MT2ENVELOPE
{
	BYTE nFlags;
	BYTE nPoints;
	BYTE nSustainPos;
	BYTE nLoopStart;
	BYTE nLoopEnd;
	BYTE bReserved[3];
	BYTE EnvData[64];
} MT2ENVELOPE;

typedef struct _MT2SYNTH
{
	BYTE nSynthId;
	BYTE nFxId;
	WORD wCutOff;
	BYTE nResonance;
	BYTE nAttack;
	BYTE nDecay;
	BYTE bReserved[25];
} MT2SYNTH;

typedef struct _MT2SAMPLE
{
	CHAR szName[32];
	DWORD dwDataLen;
	DWORD dwLength;
	DWORD dwFrequency;
	BYTE nQuality;
	BYTE nChannels;
	BYTE nFlags;
	BYTE nLoop;
	DWORD dwLoopStart;
	DWORD dwLoopEnd;
	WORD wVolume;
	BYTE nPan;
	BYTE nBaseNote;
	WORD wSamplesPerBeat;
} MT2SAMPLE;

typedef struct _MT2GROUP
{
	BYTE nSmpNo;
	BYTE nVolume;	// 0-128
	BYTE nFinePitch;
	BYTE Reserved[5];
} MT2GROUP;

#pragma pack()


static int calcNumOnes(int number) {
	int cnt = 0;

	while(number)
	{
		number &= (number -1);
		cnt ++;
	}

	return(cnt);
}


static VOID ConvertMT2Command(CSoundFile *that, MODCOMMAND *m, const MT2COMMAND *p)
//---------------------------------------------------------------------------
{
	// Note
	m->note = 0;
	if (p->note) m->note = (p->note > 96) ? 0xFF : p->note+12;
	// Instrument
	m->instr = p->instr;
	// Volume Column
	if ((p->vol >= 0x10) && (p->vol <= 0x90))
	{
		m->volcmd = VOLCMD_VOLUME;
		m->vol = (p->vol - 0x10) >> 1;
	} else
	if ((p->vol >= 0xA0) && (p->vol <= 0xAF))
	{
		m->volcmd = VOLCMD_VOLSLIDEDOWN;
		m->vol = (p->vol & 0x0f);
	} else
	if ((p->vol >= 0xB0) && (p->vol <= 0xBF))
	{
		m->volcmd = VOLCMD_VOLSLIDEUP;
		m->vol = (p->vol & 0x0f);
	} else
	if ((p->vol >= 0xC0) && (p->vol <= 0xCF))
	{
		m->volcmd = VOLCMD_FINEVOLDOWN;
		m->vol = (p->vol & 0x0f);
	} else
	if ((p->vol >= 0xD0) && (p->vol <= 0xDF))
	{
		m->volcmd = VOLCMD_FINEVOLUP;
		m->vol = (p->vol & 0x0f);
	} else
	{
		m->volcmd = 0;
		m->vol = 0;
	}
	// Effects
	m->command = 0;
	m->param = 0;
	if ((p->fxcmd) || (p->fxparam1) || (p->fxparam2))
	{
		if (!p->fxcmd)
		{
			m->command = p->fxparam2;
			m->param = p->fxparam1;
			that->ConvertModCommand(m);
		} else
		{
			// TODO: MT2 Effects
		}
	}
}


BOOL CSoundFile::ReadMT2(LPCBYTE lpStream, DWORD dwMemLength)
//-----------------------------------------------------------
{
	const MT2FILEHEADER *pfh = (MT2FILEHEADER *)lpStream;
	DWORD dwMemPos, dwDrumDataPos, dwExtraDataPos;
	UINT nDrumDataLen, nExtraDataLen;
	const MT2DRUMSDATA *pdd;
	const MT2INSTRUMENT *InstrMap[255];
	const MT2SAMPLE *SampleMap[256];

	if ((!lpStream) || (dwMemLength < sizeof(MT2FILEHEADER) + 4)
	 || (pfh->dwMT20 != 0x3032544D)
	 || (pfh->wVersion < 0x0200) || (pfh->wVersion >= 0x0300)
	 || (pfh->wChannels < 4) || (pfh->wChannels > 64)) return FALSE;
	pdd = NULL;
	m_nType = MOD_TYPE_MT2;
	m_nChannels = pfh->wChannels;
	m_nRestartPos = pfh->wRestart;
	m_nDefaultSpeed = pfh->bTicksPerLine;
	m_nDefaultTempo = 125;
	if ((pfh->wSamplesPerTick > 100) && (pfh->wSamplesPerTick < 5000))
	{
		m_nDefaultTempo = 110250 / pfh->wSamplesPerTick;
	}
	for (UINT iOrd=0; iOrd<MAX_ORDERS; iOrd++)
	{
		Order[iOrd] = (BYTE)((iOrd < pfh->nOrders) ? pfh->Orders[iOrd] : 0xFF);
	}
	memcpy(m_szNames[0], pfh->szSongName, 32);
	m_szNames[0][31] = 0;
	dwMemPos = sizeof(MT2FILEHEADER);
	if (dwMemPos+2 > dwMemLength) return TRUE;
	nDrumDataLen = *(WORD *)(lpStream + dwMemPos);
	dwDrumDataPos = dwMemPos + 2;
	if (nDrumDataLen >= 2) pdd = (MT2DRUMSDATA *)(lpStream+dwDrumDataPos);
	dwMemPos += 2 + nDrumDataLen;
#ifdef MT2DEBUG

	Log("MT2 v%03X: \"%s\" (flags=%04X)\n", pfh->wVersion, m_szNames[0], pfh->fulFlags);
	Log("%d Channels, %d Patterns, %d Instruments, %d Samples\n", pfh->wChannels, pfh->wPatterns, pfh->wInstruments, pfh->wSamples);
	Log("Drum Data: %d bytes @%04X\n", nDrumDataLen, dwDrumDataPos);
#endif
	if (dwMemPos >= dwMemLength-12) return TRUE;
	if (!*(DWORD *)(lpStream+dwMemPos)) dwMemPos += 4;
	if (!*(DWORD *)(lpStream+dwMemPos)) dwMemPos += 4;
	nExtraDataLen = *(DWORD *)(lpStream+dwMemPos);
	dwExtraDataPos = dwMemPos + 4;
	dwMemPos += 4;
#ifdef MT2DEBUG
	Log("Extra Data: %d bytes @%04X\n", nExtraDataLen, dwExtraDataPos);
#endif
	if (dwMemPos + nExtraDataLen >= dwMemLength) return TRUE;
	while (dwMemPos+8 < dwExtraDataPos + nExtraDataLen)
	{
		DWORD dwId = *(DWORD *)(lpStream+dwMemPos);
		DWORD dwLen = *(DWORD *)(lpStream+dwMemPos+4);
		dwMemPos += 8;
		if (dwLen >= dwMemLength || dwMemPos > dwMemLength - dwLen) return TRUE;
#ifdef MT2DEBUG
		CHAR s[5];
		memcpy(s, &dwId, 4);
		s[4] = 0;
		Log("pos=0x%04X: %s: %d bytes\n", dwMemPos-8, s, dwLen);
#endif
		switch(dwId)
		{
		// MSG
		case 0x0047534D:
			if ((dwLen > 3) && (!m_lpszSongComments))
			{
				DWORD nTxtLen = dwLen;
				if (nTxtLen > 32000) nTxtLen = 32000;
				m_lpszSongComments = new char[nTxtLen];  // changed from CHAR
				if (m_lpszSongComments)
				{
					memcpy(m_lpszSongComments, lpStream+dwMemPos+1, nTxtLen-1);
					m_lpszSongComments[nTxtLen-1] = 0;
				}
			}
			break;
		// SUM -> author name (or "Unregistered")
		// TMAP
		// TRKS
		case 0x534b5254:
			break;
		}
		dwMemPos += dwLen;
	}
	// Load Patterns
	dwMemPos = dwExtraDataPos + nExtraDataLen;
	for (UINT iPat=0; iPat<pfh->wPatterns; iPat++) if (dwMemPos < dwMemLength-6)
	{
		const MT2PATTERN *pmp = (MT2PATTERN *)(lpStream+dwMemPos);
		UINT wDataLen = (pmp->wDataLen + 1) & ~1;
		dwMemPos += 6;
		if (dwMemPos > dwMemLength - wDataLen || wDataLen > dwMemLength) break;

		UINT nLines = pmp->wLines;
		if ((iPat < MAX_PATTERNS) && (nLines > 0) && (nLines <= 256))
		{
	#ifdef MT2DEBUG
			Log("Pattern #%d @%04X: %d lines, %d bytes\n", iPat, dwMemPos-6, nLines, pmp->wDataLen);
	#endif
			PatternSize[iPat] = nLines;
			Patterns[iPat] = AllocatePattern(nLines, m_nChannels);
			if (!Patterns[iPat]) return TRUE;
			MODCOMMAND *m = Patterns[iPat];
			UINT len = wDataLen;
			if (len <= 4) return TRUE;

			if (pfh->fulFlags & 1) // Packed Patterns
			{
				const BYTE *p = lpStream+dwMemPos;
				UINT pos = 0, row=0, ch=0;
				while (pos < len - 4)
				{
					MT2COMMAND cmd;
					UINT infobyte = p[pos++];
					UINT rptcount = 0;
					if (infobyte == 0xff)
					{
						if (pos + 2 > len) break;
						rptcount = p[pos++];
						infobyte = p[pos++];
				#if 0
						Log("(%d.%d) FF(%02X).%02X\n", row, ch, rptcount, infobyte);
					} else
					{
						Log("(%d.%d) %02X\n", row, ch, infobyte);
				#endif
					}
					if (infobyte & 0x7f)
					{
						UINT patpos = row*m_nChannels+ch;
						cmd.note = cmd.instr = cmd.vol = cmd.pan = cmd.fxcmd = cmd.fxparam1 = cmd.fxparam2 = 0;
						if (pos >= len - calcNumOnes(infobyte & 0x7F)) break;
						if (infobyte & 1) cmd.note = p[pos++];
						if (infobyte & 2) cmd.instr = p[pos++];
						if (infobyte & 4) cmd.vol = p[pos++];
						if (infobyte & 8) cmd.pan = p[pos++];
						if (infobyte & 16) cmd.fxcmd = p[pos++];
						if (infobyte & 32) cmd.fxparam1 = p[pos++];
						if (infobyte & 64) cmd.fxparam2 = p[pos++];
					#ifdef MT2DEBUG
						if (cmd.fxcmd)
						{
							Log("(%d.%d) MT2 FX=%02X.%02X.%02X\n", row, ch, cmd.fxcmd, cmd.fxparam1, cmd.fxparam2);
						}
					#endif
						ConvertMT2Command(this, &m[patpos], &cmd);
					}
					row += rptcount+1;
					while (row >= nLines) { row-=nLines; ch++; }
					if (ch >= m_nChannels) break;
				}
			} else
			{
				const MT2COMMAND *p = (MT2COMMAND *)(lpStream+dwMemPos);
				UINT pos = 0;
				UINT n = 0;
				while ((pos + sizeof(MT2COMMAND) <= len) && (n < m_nChannels*nLines))
				{
					ConvertMT2Command(this, m, p);
					pos += sizeof(MT2COMMAND);
					n++;
					p++;
					m++;
				}
			}
		}
		dwMemPos += wDataLen;
	}
	// Skip Drum Patterns
	if (pdd)
	{
	#ifdef MT2DEBUG
		Log("%d Drum Patterns at offset 0x%08X\n", pdd->wDrumPatterns, dwMemPos);
	#endif
		for (UINT iDrm=0; iDrm<pdd->wDrumPatterns; iDrm++)
		{
			if (dwMemPos > dwMemLength-2) return TRUE;
			UINT nLines = *(WORD *)(lpStream+dwMemPos);
		#ifdef MT2DEBUG
			if (nLines != 64) Log("Drum Pattern %d: %d Lines @%04X\n", iDrm, nLines, dwMemPos);
		#endif
			dwMemPos += 2 + nLines * 32;
		}
	}
	// Automation
	if (pfh->fulFlags & 2)
	{
	#ifdef MT2DEBUG
		Log("Automation at offset 0x%08X\n", dwMemPos);
	#endif
		UINT nAutoCount = m_nChannels;
		if (pfh->fulFlags & 0x10) nAutoCount++; // Master Automation
		if ((pfh->fulFlags & 0x08) && (pdd)) nAutoCount += 8; // Drums Automation
		nAutoCount *= pfh->wPatterns;
		for (UINT iAuto=0; iAuto<nAutoCount; iAuto++)
		{
			if (dwMemPos+12 >= dwMemLength) return TRUE;
			const MT2AUTOMATION *pma = (MT2AUTOMATION *)(lpStream+dwMemPos);
			dwMemPos += (pfh->wVersion <= 0x201) ? 4 : 8;
			for (UINT iEnv=0; iEnv<14; iEnv++)
			{
				if (pma->dwFlags & (1 << iEnv))
				{
				#ifdef MT2DEBUG
					UINT nPoints = *(DWORD *)(lpStream+dwMemPos);
					Log("  Env[%d/%d] %04X @%04X: %d points\n", iAuto, nAutoCount, 1 << iEnv, dwMemPos-8, nPoints);
				#endif
					dwMemPos += 260;
				}
			}
		}
	}
	// Load Instruments
#ifdef MT2DEBUG
	Log("Loading instruments at offset 0x%08X\n", dwMemPos);
#endif
	memset(InstrMap, 0, sizeof(InstrMap));
	m_nInstruments = (pfh->wInstruments < MAX_INSTRUMENTS) ? pfh->wInstruments : MAX_INSTRUMENTS-1;
	for (UINT iIns=1; iIns<=255; iIns++)
	{
		if (dwMemPos+36 > dwMemLength) return TRUE;
		const MT2INSTRUMENT *pmi = (MT2INSTRUMENT *)(lpStream+dwMemPos);
		INSTRUMENTHEADER *penv = NULL;
		if (iIns <= m_nInstruments)
		{
			penv = new INSTRUMENTHEADER;
			Headers[iIns] = penv;
			if (penv)
			{
				memset(penv, 0, sizeof(INSTRUMENTHEADER));
				memcpy(penv->name, pmi->szName, 32);
				penv->nGlobalVol = 64;
				penv->nPan = 128;
				for (UINT i=0; i<NOTE_MAX; i++)
				{
					penv->NoteMap[i] = i+1;
				}
			}
		}
	#ifdef MT2DEBUG
		if (iIns <= pfh->wInstruments) Log("  Instrument #%d at offset %04X: %d bytes\n", iIns, dwMemPos, pmi->dwDataLen);
	#endif
		if (pmi->dwDataLen > dwMemLength - (dwMemPos+36)) return TRUE;
		if (pmi->dwDataLen > 0)
		{
			if (dwMemPos + sizeof(MT2INSTRUMENT) - 4 > dwMemLength) return TRUE;
			InstrMap[iIns-1] = pmi;
			if (penv && pmi->dwDataLen >= sizeof(MT2INSTRUMENT) - 40)
			{
				penv->nFadeOut = pmi->wFadeOut;
				penv->nNNA = pmi->wNNA & 3;
				penv->nDCT = (pmi->wNNA>>8) & 3;
				penv->nDNA = (pmi->wNNA>>12) & 3;
				MT2ENVELOPE *pehdr[4];
				WORD *pedata[4];
				if (pfh->wVersion <= 0x201)
				{
					DWORD dwEnvPos = dwMemPos + sizeof(MT2INSTRUMENT) - 4;
					if (dwEnvPos + 2*sizeof(MT2ENVELOPE) > dwMemLength) return TRUE;
					pehdr[0] = (MT2ENVELOPE *)(lpStream+dwEnvPos);
					pehdr[1] = (MT2ENVELOPE *)(lpStream+dwEnvPos+8);
					pehdr[2] = pehdr[3] = NULL;
					pedata[0] = (WORD *)(lpStream+dwEnvPos+16);
					pedata[1] = (WORD *)(lpStream+dwEnvPos+16+64);
					pedata[2] = pedata[3] = NULL;
				} else
				{
					DWORD dwEnvPos = dwMemPos + sizeof(MT2INSTRUMENT);
					if (dwEnvPos > dwMemLength) return TRUE;
					for (UINT i=0; i<4; i++)
					{
						if (pmi->wEnvFlags1 & (1<<i))
						{
							if (dwEnvPos + sizeof(MT2ENVELOPE) > dwMemLength) return TRUE;
							pehdr[i] = (MT2ENVELOPE *)(lpStream+dwEnvPos);
							pedata[i] = (WORD *)pehdr[i]->EnvData;
							dwEnvPos += sizeof(MT2ENVELOPE);
						} else
						{
							pehdr[i] = NULL;
							pedata[i] = NULL;
						}
					}

					// envelopes exceed file length?
					if (dwEnvPos > dwMemLength) return TRUE;

				}
				// Load envelopes
				for (UINT iEnv=0; iEnv<4; iEnv++) if (pehdr[iEnv])
				{
					const MT2ENVELOPE *pme = pehdr[iEnv];
					WORD *pEnvPoints = NULL;
					BYTE *pEnvData = NULL;
				#ifdef MT2DEBUG
					Log("  Env %d.%d @%04X: %d points\n", iIns, iEnv, (UINT)(((BYTE *)pme)-lpStream), pme->nPoints);
				#endif
					switch(iEnv)
					{
					// Volume Envelope
					case 0:
						if (pme->nFlags & 1) penv->dwFlags |= ENV_VOLUME;
						if (pme->nFlags & 2) penv->dwFlags |= ENV_VOLSUSTAIN;
						if (pme->nFlags & 4) penv->dwFlags |= ENV_VOLLOOP;
						penv->nVolEnv = (pme->nPoints > 16) ? 16 : pme->nPoints;
						penv->nVolSustainBegin = penv->nVolSustainEnd = pme->nSustainPos;
						penv->nVolLoopStart = pme->nLoopStart;
						penv->nVolLoopEnd = pme->nLoopEnd;
						pEnvPoints = penv->VolPoints;
						pEnvData = penv->VolEnv;
						break;

					// Panning Envelope
					case 1:
						if (pme->nFlags & 1) penv->dwFlags |= ENV_PANNING;
						if (pme->nFlags & 2) penv->dwFlags |= ENV_PANSUSTAIN;
						if (pme->nFlags & 4) penv->dwFlags |= ENV_PANLOOP;
						penv->nPanEnv = (pme->nPoints > 16) ? 16 : pme->nPoints;
						penv->nPanSustainBegin = penv->nPanSustainEnd = pme->nSustainPos;
						penv->nPanLoopStart = pme->nLoopStart;
						penv->nPanLoopEnd = pme->nLoopEnd;
						pEnvPoints = penv->PanPoints;
						pEnvData = penv->PanEnv;
						break;

					// Pitch/Filter envelope
					default:
						if (pme->nFlags & 1) penv->dwFlags |= (iEnv==3) ? (ENV_PITCH|ENV_FILTER) : ENV_PITCH;
						if (pme->nFlags & 2) penv->dwFlags |= ENV_PITCHSUSTAIN;
						if (pme->nFlags & 4) penv->dwFlags |= ENV_PITCHLOOP;
						penv->nPitchEnv = (pme->nPoints > 16) ? 16 : pme->nPoints;
						penv->nPitchSustainBegin = penv->nPitchSustainEnd = pme->nSustainPos;
						penv->nPitchLoopStart = pme->nLoopStart;
						penv->nPitchLoopEnd = pme->nLoopEnd;
						pEnvPoints = penv->PitchPoints;
						pEnvData = penv->PitchEnv;
					}
					// Envelope data
					if ((pEnvPoints) && (pEnvData) && (pedata[iEnv]))
					{
						WORD *psrc = pedata[iEnv];
						for (UINT i=0; i<16; i++)
						{
							pEnvPoints[i] = psrc[i*2];
							pEnvData[i] = (BYTE)psrc[i*2+1];
						}
					}
				}
			}
			dwMemPos += pmi->dwDataLen + 36;
			if (pfh->wVersion > 0x201) dwMemPos += 4; // ?
		} else
		{
			dwMemPos += 36;
		}
	}
#ifdef MT2DEBUG
	Log("Loading samples at offset 0x%08X\n", dwMemPos);
#endif
	memset(SampleMap, 0, sizeof(SampleMap));
	m_nSamples = (pfh->wSamples < MAX_SAMPLES) ? pfh->wSamples : MAX_SAMPLES-1;
	for (UINT iSmp=1; iSmp<=256; iSmp++)
	{
		if (dwMemPos > dwMemLength - 36) return TRUE;
		const MT2SAMPLE *pms = (MT2SAMPLE *)(lpStream+dwMemPos);
	#ifdef MT2DEBUG
		if (iSmp <= m_nSamples) Log("  Sample #%d at offset %04X: %d bytes\n", iSmp, dwMemPos, pms->dwDataLen);
	#endif
		if (iSmp < MAX_SAMPLES)
		{
			memcpy(m_szNames[iSmp], pms->szName, 32);
		}
		if (pms->dwDataLen > dwMemLength - (dwMemPos+36)) return TRUE;
		if (pms->dwDataLen > 0)
		{
			SampleMap[iSmp-1] = pms;
			if (iSmp < MAX_SAMPLES)
			{
				MODINSTRUMENT *psmp = &Ins[iSmp];
				psmp->nGlobalVol = 64;
				if (dwMemPos+sizeof(MT2SAMPLE) > dwMemLength) return TRUE;
				psmp->nVolume = (pms->wVolume >> 7);
				psmp->nPan = (pms->nPan == 0x80) ? 128 : (pms->nPan^0x80);
				psmp->nLength = pms->dwLength;
				psmp->nC4Speed = pms->dwFrequency;
				psmp->nLoopStart = pms->dwLoopStart;
				psmp->nLoopEnd = pms->dwLoopEnd;
				FrequencyToTranspose(psmp);
				psmp->RelativeTone -= pms->nBaseNote - 49;
				psmp->nC4Speed = TransposeToFrequency(psmp->RelativeTone, psmp->nFineTune);
				if (pms->nQuality == 2) { psmp->uFlags |= CHN_16BIT; psmp->nLength >>= 1; }
				if (pms->nChannels == 2) { psmp->nLength >>= 1; }
				if (pms->nLoop == 1) psmp->uFlags |= CHN_LOOP;
				if (pms->nLoop == 2) psmp->uFlags |= CHN_LOOP|CHN_PINGPONGLOOP;
			}
			dwMemPos += pms->dwDataLen + 36;
		} else
		{
			dwMemPos += 36;
		}
	}
#ifdef MT2DEBUG
	Log("Loading groups at offset 0x%08X\n", dwMemPos);
#endif
	for (UINT iMap=0; iMap<255; iMap++) if (InstrMap[iMap])
	{
		const MT2INSTRUMENT *pmi = InstrMap[iMap];
		INSTRUMENTHEADER *penv = NULL;
		if (iMap<m_nInstruments) penv = Headers[iMap+1];
		for (UINT iGrp=0; iGrp<pmi->wSamples; iGrp++)
		{
			if (penv && dwMemPos < dwMemLength && dwMemPos < dwMemLength - 8)
			{
				const MT2GROUP *pmg = (MT2GROUP *)(lpStream+dwMemPos);
				for (UINT i=0; i<96; i++)
				{
					if (pmi->GroupsMapping[i] == iGrp)
					{
						UINT nSmp = pmg->nSmpNo+1;
						penv->Keyboard[i+12] = (BYTE)nSmp;
						if (nSmp <= m_nSamples)
						{
							Ins[nSmp].nVibType = pmi->bVibType;
							Ins[nSmp].nVibSweep = pmi->bVibSweep;
							Ins[nSmp].nVibDepth = pmi->bVibDepth;
							Ins[nSmp].nVibRate = pmi->bVibRate;
						}
					}
				}
			}
			dwMemPos += 8;
		}
	}
#ifdef MT2DEBUG
	Log("Loading sample data at offset 0x%08X\n", dwMemPos);
#endif
	for (UINT iData=0; iData<256; iData++) if ((iData < m_nSamples) && (SampleMap[iData]))
	{
		const MT2SAMPLE *pms = SampleMap[iData];
		MODINSTRUMENT *psmp = &Ins[iData+1];
		if (!(pms->nFlags & 5))
		{
			if (psmp->nLength > 0 && dwMemPos < dwMemLength)
			{
			#ifdef MT2DEBUG
				Log("  Reading sample #%d at offset 0x%04X (len=%d)\n", iData+1, dwMemPos, psmp->nLength);
			#endif
				UINT rsflags;

				if (pms->nChannels == 2)
					rsflags = (psmp->uFlags & CHN_16BIT) ? RS_STPCM16D : RS_STPCM8D;
				else
					rsflags = (psmp->uFlags & CHN_16BIT) ? RS_PCM16D : RS_PCM8D;

				dwMemPos += ReadSample(psmp, rsflags, (LPCSTR)(lpStream+dwMemPos), dwMemLength-dwMemPos);
			}
		} else
		if (dwMemPos < dwMemLength-4)
		{
			UINT nNameLen = *(DWORD *)(lpStream+dwMemPos);
			dwMemPos += nNameLen + 16;
		}
		if (dwMemPos >= dwMemLength-4) break;
	}
	return TRUE;
}
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

//#include "stdafx.h"
//#include "sndfile.h"

//#pragma warning(disable:4244)

//////////////////////////////////////////////////////////
// MTM file support (import only)

#pragma pack(1)


typedef struct tagMTMSAMPLE
{
        char samplename[22];      // changed from CHAR
	DWORD length;
	DWORD reppos;
	DWORD repend;
	CHAR finetune;
	BYTE volume;
	BYTE attribute;
} MTMSAMPLE;


typedef struct tagMTMHEADER
{
	char id[4];	    // MTM file marker + version // changed from CHAR
	char songname[20];  // ASCIIZ songname  // changed from CHAR
	WORD numtracks;	    // number of tracks saved
	BYTE lastpattern;   // last pattern number saved
	BYTE lastorder;	    // last order number to play (songlength-1)
	WORD commentsize;   // length of comment field
	BYTE numsamples;    // number of samples saved
	BYTE attribute;	    // attribute byte (unused)
	BYTE beatspertrack;
	BYTE numchannels;   // number of channels used
	BYTE panpos[32];    // voice pan positions
} MTMHEADER;


#pragma pack()


BOOL CSoundFile::ReadMTM(LPCBYTE lpStream, DWORD dwMemLength)
//-----------------------------------------------------------
{
	MTMHEADER *pmh = (MTMHEADER *)lpStream;
	DWORD dwMemPos = 66;

	if ((!lpStream) || (dwMemLength < 0x100)) return FALSE;
	if ((strncmp(pmh->id, "MTM", 3)) || (pmh->numchannels > 32)
	 || (pmh->numsamples >= MAX_SAMPLES) || (!pmh->numsamples)
	 || (!pmh->numtracks) || (!pmh->numchannels)
	 || (!pmh->lastpattern) || (pmh->lastpattern >= MAX_PATTERNS))
		return FALSE;
	strncpy(m_szNames[0], pmh->songname, 20);
	m_szNames[0][20] = 0;
	if (dwMemPos + 37*pmh->numsamples + 128 + 192*pmh->numtracks
	 + 64 * (pmh->lastpattern+1) + pmh->commentsize >= dwMemLength) 
		return FALSE;
	m_nType = MOD_TYPE_MTM;
	m_nSamples = pmh->numsamples;
	m_nChannels = pmh->numchannels;
	// Reading instruments
	for	(UINT i=1; i<=m_nSamples; i++)
	{
		MTMSAMPLE *pms = (MTMSAMPLE *)(lpStream + dwMemPos);
		strncpy(m_szNames[i], pms->samplename, 22);
		m_szNames[i][22] = 0;
		Ins[i].nVolume = pms->volume << 2;
		Ins[i].nGlobalVol = 64;
		DWORD len = pms->length;
		if ((len > 4) && (len <= MAX_SAMPLE_LENGTH))
		{
			Ins[i].nLength = len;
			Ins[i].nLoopStart = pms->reppos;
			Ins[i].nLoopEnd = pms->repend;
			if (Ins[i].nLoopEnd > Ins[i].nLength) 
				Ins[i].nLoopEnd = Ins[i].nLength;
			if (Ins[i].nLoopStart + 4 >= Ins[i].nLoopEnd) 
				Ins[i].nLoopStart = Ins[i].nLoopEnd = 0;
			if (Ins[i].nLoopEnd) Ins[i].uFlags |= CHN_LOOP;
			Ins[i].nFineTune = MOD2XMFineTune(pms->finetune);
			if (pms->attribute & 0x01)
			{
				Ins[i].uFlags |= CHN_16BIT;
				Ins[i].nLength >>= 1;
				Ins[i].nLoopStart >>= 1;
				Ins[i].nLoopEnd >>= 1;
			}
			Ins[i].nPan = 128;
		}
		dwMemPos += 37;
	}
	// Setting Channel Pan Position
	for (UINT ich=0; ich<m_nChannels; ich++)
	{
		ChnSettings[ich].nPan = ((pmh->panpos[ich] & 0x0F) << 4) + 8;
		ChnSettings[ich].nVolume = 64;
	}
	// Reading pattern order
	memcpy(Order, lpStream + dwMemPos, pmh->lastorder+1);
	dwMemPos += 128;
	// Reading Patterns
	LPCBYTE pTracks = lpStream + dwMemPos;
	dwMemPos += 192 * pmh->numtracks;
	LPWORD pSeq = (LPWORD)(lpStream + dwMemPos);
	for (UINT pat=0; pat<=pmh->lastpattern; pat++)
	{
		PatternSize[pat] = 64;
		if ((Patterns[pat] = AllocatePattern(64, m_nChannels)) == NULL) break;
		for (UINT n=0; n<32; n++) if ((pSeq[n]) && (pSeq[n] <= pmh->numtracks) && (n < m_nChannels))
		{
			LPCBYTE p = pTracks + 192 * (pSeq[n]-1);
			MODCOMMAND *m = Patterns[pat] + n;
			for (UINT i=0; i<64; i++, m+=m_nChannels, p+=3)
			{
				if (p[0] & 0xFC) m->note = (p[0] >> 2) + 37;
				m->instr = ((p[0] & 0x03) << 4) | (p[1] >> 4);
				UINT cmd = p[1] & 0x0F;
				UINT param = p[2];
				if (cmd == 0x0A)
				{
					if (param & 0xF0) param &= 0xF0; else param &= 0x0F;
				}
				m->command = cmd;
				m->param = param;
				if ((cmd) || (param)) ConvertModCommand(m);
			}
		}
		pSeq += 32;
	}
	dwMemPos += 64*(pmh->lastpattern+1);
	if ((pmh->commentsize) && (dwMemPos + pmh->commentsize < dwMemLength))
	{
		UINT n = pmh->commentsize;
		m_lpszSongComments = new char[n+1];
		if (m_lpszSongComments)
		{
			memcpy(m_lpszSongComments, lpStream+dwMemPos, n);
			m_lpszSongComments[n] = 0;
			for (UINT i=0; i<n; i++)
			{
				if (!m_lpszSongComments[i])
				{
					m_lpszSongComments[i] = ((i+1) % 40) ? 0x20 : 0x0D;
				}
			}
		}
	}
	dwMemPos += pmh->commentsize;
	// Reading Samples
	for (UINT ismp=1; ismp<=m_nSamples; ismp++)
	{
		if (dwMemPos >= dwMemLength) break;
		dwMemPos += ReadSample(&Ins[ismp], (Ins[ismp].uFlags & CHN_16BIT) ? RS_PCM16U : RS_PCM8U,
								(LPSTR)(lpStream + dwMemPos), dwMemLength - dwMemPos);
	}
	m_nMinPeriod = 64;
	m_nMaxPeriod = 32767;
	return TRUE;
}

/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (endian and char fixes for PPC)
*/

//////////////////////////////////////////////
// Oktalyzer (OKT) module loader            //
//////////////////////////////////////////////
//#include "stdafx.h"
//#include "sndfile.h"

//#pragma warning(disable:4244)

#define MAGIC(a,b,c,d) (((a) << 24UL) | ((b) << 16UL) | ((c) << 8UL) | (d))

#pragma pack(1)
typedef struct OKTFILEHEADER
{
	DWORD okta;		// "OKTA"
	DWORD song;		// "SONG"
	DWORD cmod;		// "CMOD"
	DWORD cmodlen;
	BYTE chnsetup[8];
	DWORD samp;		// "SAMP"
	DWORD samplen;
} OKTFILEHEADER;

typedef struct OKTSAMPLE
{
	CHAR name[20];
	DWORD length;
	WORD loopstart;
	WORD looplen;
	BYTE pad1;
	BYTE volume;
	BYTE pad2;
	BYTE pad3;
} OKTSAMPLE;
#pragma pack()


static DWORD readBE32(const BYTE *v)
{
	return (v[0] << 24UL) | (v[1] << 16UL) | (v[2] << 8UL) | v[3];
}

BOOL CSoundFile::ReadOKT(const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
	const OKTFILEHEADER *pfh = (OKTFILEHEADER *)lpStream;
	DWORD dwMemPos = sizeof(OKTFILEHEADER), dwSize;
	UINT nsamples = 0, norders = 0;//, npatterns = 0

	if ((!lpStream) || (dwMemLength < 1024)) return FALSE;
	if ((bswapBE32(pfh->okta) != MAGIC('O','K','T','A'))
	 || (bswapBE32(pfh->song) != MAGIC('S','O','N','G'))
	 || (bswapBE32(pfh->cmod) != MAGIC('C','M','O','D'))
	 || (bswapBE32(pfh->cmodlen) != 8)
	 || (pfh->chnsetup[0]) || (pfh->chnsetup[2])
	 || (pfh->chnsetup[4]) || (pfh->chnsetup[6])
	 || (bswapBE32(pfh->samp) != MAGIC('S','A','M','P'))) return FALSE;
	m_nType = MOD_TYPE_OKT;
	m_nChannels = 4 + pfh->chnsetup[1] + pfh->chnsetup[3] + pfh->chnsetup[5] + pfh->chnsetup[7];
	if (m_nChannels > MAX_CHANNELS) m_nChannels = MAX_CHANNELS;
	nsamples = bswapBE32(pfh->samplen) >> 5;
	m_nSamples = nsamples;
	if (m_nSamples >= MAX_SAMPLES) m_nSamples = MAX_SAMPLES-1;
	// Reading samples
	for (UINT smp=1; smp <= nsamples; smp++)
	{
		if (dwMemPos >= dwMemLength - sizeof(OKTSAMPLE)) return TRUE;
		if (smp < MAX_SAMPLES)
		{
			const OKTSAMPLE *psmp = (const OKTSAMPLE *)(lpStream + dwMemPos);
			MODINSTRUMENT *pins = &Ins[smp];

			memcpy(m_szNames[smp], psmp->name, 20);
			pins->uFlags = 0;
			pins->nLength = bswapBE32(psmp->length) & ~1;
			pins->nLoopStart = bswapBE16(psmp->loopstart);
			pins->nLoopEnd = pins->nLoopStart + bswapBE16(psmp->looplen);
			if (pins->nLoopStart + 2 < pins->nLoopEnd) pins->uFlags |= CHN_LOOP;
			pins->nGlobalVol = 64;
			pins->nVolume = psmp->volume << 2;
			pins->nC4Speed = 8363;
		}
		dwMemPos += sizeof(OKTSAMPLE);
	}
	// SPEE
	if (dwMemPos >= dwMemLength - 12) return TRUE;
	if (readBE32(lpStream + dwMemPos) == MAGIC('S','P','E','E'))
	{
		m_nDefaultSpeed = lpStream[dwMemPos+9];

		dwSize = readBE32(lpStream + dwMemPos + 4);
		if (dwSize > dwMemLength - 8 || dwMemPos > dwMemLength - dwSize - 8) return TRUE;
		dwMemPos += dwSize + 8;
	}
	// SLEN
	if (dwMemPos + 10 > dwMemLength) return TRUE;
	if (readBE32(lpStream + dwMemPos) == MAGIC('S','L','E','N'))
	{
	//	npatterns = lpStream[dwMemPos+9];

		dwSize = readBE32(lpStream + dwMemPos + 4);
		if (dwSize > dwMemLength - 8 || dwMemPos > dwMemLength - dwSize - 8) return TRUE;
		dwMemPos += dwSize + 8;
	}
	// PLEN
	if (dwMemPos + 10 > dwMemLength) return TRUE;
	if (readBE32(lpStream + dwMemPos) == MAGIC('P','L','E','N'))
	{
		norders = lpStream[dwMemPos+9];

		dwSize = readBE32(lpStream + dwMemPos + 4);
		if (dwSize > dwMemLength - 8 || dwMemPos > dwMemLength - dwSize - 8) return TRUE;
		dwMemPos += dwSize + 8;
	}
	// PATT
	if (dwMemPos + 8 > dwMemLength) return TRUE;
	if (readBE32(lpStream + dwMemPos) == MAGIC('P','A','T','T'))
	{
		UINT orderlen = norders;
		if (orderlen >= MAX_ORDERS) orderlen = MAX_ORDERS-1;
		if (dwMemPos + 8 + orderlen > dwMemLength) return TRUE;
		for (UINT i=0; i<orderlen; i++) Order[i] = lpStream[dwMemPos+8+i];
		for (UINT j=orderlen; j>1; j--) { if (Order[j-1]) break; Order[j-1] = 0xFF; }

		dwSize = readBE32(lpStream + dwMemPos + 4);
		if (dwSize > dwMemLength - 8 || dwMemPos > dwMemLength - dwSize - 8) return TRUE;
		dwMemPos += dwSize + 8;
	}
	// PBOD
	UINT npat = 0;
	while ((dwMemPos < dwMemLength - 10) && (readBE32(lpStream + dwMemPos) == MAGIC('P','B','O','D')))
	{
		DWORD dwPos = dwMemPos + 10;
		UINT rows = lpStream[dwMemPos+9];
		if (!rows) rows = 64;
		if (npat < MAX_PATTERNS)
		{
			if ((Patterns[npat] = AllocatePattern(rows, m_nChannels)) == NULL) return TRUE;
			MODCOMMAND *m = Patterns[npat];
			PatternSize[npat] = rows;
			UINT imax = m_nChannels*rows;
			for (UINT i=0; i<imax; i++, m++, dwPos+=4)
			{
				if (dwPos+4 > dwMemLength) break;
				const BYTE *p = lpStream+dwPos;
				UINT note = p[0];
				if (note)
				{
					m->note = note + 48;
					m->instr = p[1] + 1;
				}
				UINT command = p[2];
				UINT param = p[3];
				m->param = param;
				switch(command)
				{
				// 0: no effect
				case 0:
					break;
				// 1: Portamento Up
				case 1:
				case 17:
				case 30:
					if (param) m->command = CMD_PORTAMENTOUP;
					break;
				// 2: Portamento Down
				case 2:
				case 13:
				case 21:
					if (param) m->command = CMD_PORTAMENTODOWN;
					break;
				// 10: Arpeggio
				case 10:
				case 11:
				case 12:
					m->command = CMD_ARPEGGIO;
					break;
				// 15: Filter
				case 15:
					m->command = CMD_MODCMDEX;
					m->param = param & 0x0F;
					break;
				// 25: Position Jump
				case 25:
					m->command = CMD_POSITIONJUMP;
					break;
				// 28: Set Speed
				case 28:
					m->command = CMD_SPEED;
					break;
				// 31: Volume Control
				case 31:
					if (param <= 0x40) m->command = CMD_VOLUME; else
					if (param <= 0x50) { m->command = CMD_VOLUMESLIDE; m->param &= 0x0F; if (!m->param) m->param = 0x0F; } else
					if (param <= 0x60) { m->command = CMD_VOLUMESLIDE; m->param = (param & 0x0F) << 4; if (!m->param) m->param = 0xF0; } else
					if (param <= 0x70) { m->command = CMD_MODCMDEX; m->param = 0xB0 | (param & 0x0F); if (!(param & 0x0F)) m->param = 0xBF; } else
					if (param <= 0x80) { m->command = CMD_MODCMDEX; m->param = 0xA0 | (param & 0x0F); if (!(param & 0x0F)) m->param = 0xAF; }
					break;
				}
			}
		}
		npat++;

		dwSize = readBE32(lpStream + dwMemPos + 4);
		if (dwSize > dwMemLength - 8 || dwMemPos > dwMemLength - dwSize - 8) return TRUE;
		dwMemPos += dwSize + 8;
	}
	// SBOD
	UINT nsmp = 1;
	while ((dwMemPos < dwMemLength-10) && (readBE32(lpStream + dwMemPos) == MAGIC('S','B','O','D')))
	{
		if (nsmp < MAX_SAMPLES) ReadSample(&Ins[nsmp], RS_PCM8S, (LPSTR)(lpStream+dwMemPos+8), dwMemLength-dwMemPos-8);
		nsmp++;

		dwSize = readBE32(lpStream + dwMemPos + 4);
		if (dwSize > dwMemLength - 8 || dwMemPos > dwMemLength - dwSize - 8) return TRUE;
		dwMemPos += dwSize + 8;
	}
	return TRUE;
}
/*

 MikMod Sound System

  By Jake Stine of Divine Entertainment (1996-2000)

 Support:
  If you find problems with this code, send mail to:
    air@divent.org

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 -----------------------------------------
 Module: LOAD_PAT

  PAT sample loader.
	by Peter Grootswagers (2006)
	<email:pgrootswagers@planet.nl>

 It's primary purpose is loading samples for the .abc and .mid modules
 Can also be used stand alone, in that case a tune (frere Jacques)
 is generated using al samples available in the .pat file

 Portability:
	All systems - all compilers (hopefully)
*/

#include <stdlib.h>
#include <time.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <math.h>
#include <ctype.h>
#include <limits.h> /* PATH_MAX */
#ifndef _WIN32
#include <unistd.h>  /* sleep() */
#endif

//#include "stdafx.h"
//#include "sndfile.h"
#ifndef PATH_MAX
#define PATH_MAX 256
#endif

//#include "load_pat.h"

#if defined(_WIN32)||defined(__OS2__)
#define DIRDELIM		'\\'
#define TIMIDITYCFG	"C:\\TIMIDITY\\TIMIDITY.CFG"
#define PATHFORPAT	"C:\\TIMIDITY\\INSTRUMENTS"
#else
#define DIRDELIM		'/'
#define TIMIDITYCFG	"/etc/timidity.cfg" /*"/usr/share/timidity/timidity.cfg"*/
#define PATHFORPAT	"/usr/share/timidity/instruments"
#endif

#define PAT_ENV_PATH2CFG			"MMPAT_PATH_TO_CFG"

// 128 gm and 63 drum
#define MAXSMP				191

static char midipat[MAXSMP][PATH_MAX];
static char pathforpat[PATH_MAX];
static char timiditycfg[PATH_MAX];

#pragma pack(1)

typedef struct {
	char header[12];	// ascizz GF1PATCH110
	char gravis_id[10];	// allways ID#000002
	char description[60];
	BYTE instruments;
	BYTE voices;
	BYTE channels;
	WORD waveforms;
	WORD master_volume;
	DWORD data_size;
	char reserved[36];
} PatchHeader;

typedef struct {
	WORD instrument_id;
	char instrument_name[16];
	DWORD instrument_size;
	BYTE layers;
	char reserved[40];
} InstrumentHeader;

typedef struct {
	BYTE layer_dup;
	BYTE layer_id;
	DWORD layer_size;
	BYTE samples;
	char reserved[40];
} LayerHeader;

typedef struct {
	char wave_name[7];
	BYTE fractions;
	DWORD wave_size;
	DWORD start_loop;
	DWORD end_loop;
	WORD sample_rate;
	DWORD low_frequency ;
	DWORD high_frequency;
	DWORD root_frequency;
	short int tune;
	BYTE balance;
	BYTE envelope_rate[6];
	BYTE envelope_offset[6];
	BYTE tremolo_sweep;
	BYTE tremolo_rate;
	BYTE tremolo_depth;
	BYTE vibrato_sweep;
	BYTE vibrato_rate;
	BYTE vibrato_depth;
	BYTE modes;
	DWORD scale_frequency;
	DWORD scale_factor;
	char reserved[32];
} WaveHeader;

// WaveHeader.modes bits
#define PAT_16BIT     1
#define PAT_UNSIGNED  2
#define PAT_LOOP      4
#define PAT_PINGPONG  8
#define PAT_BACKWARD 16
#define PAT_SUSTAIN  32
#define PAT_ENVELOPE 64
#define PAT_CLAMPED 128

#define C4SPD	8363
#define C4mHz	523251
#define C4	523.251f
#define PI	3.141592653589793f
#define OMEGA	((2.0f * PI * C4)/(float)C4SPD)

/**********************************************************************/
static BYTE pat_gm_used[MAXSMP];
static BYTE pat_loops[MAXSMP];

/**********************************************************************/

typedef struct _PATHANDLE
{
	char patname[16];
	int samples;
} PATHANDLE;

// local prototypes
static int pat_getopt(const char *s, const char *o, int dflt);

static void pat_message(const char *s1, const char *s2)
{
	char txt[256];
	if( strlen(s1) + strlen(s2) > 255 ) return;
	sprintf(txt, s1, s2);
	fprintf(stderr, "load_pat > %s\n", txt);
}

void pat_resetsmp(void)
{
	int i;
	for( i=0; i<MAXSMP; i++ ) {
		pat_loops[i] = 0;
		pat_gm_used[i] = 0;
	}
}

int pat_numsmp()
{
	return strlen((const char *)pat_gm_used);
}

int pat_numinstr(void)
{
	return strlen((const char *)pat_gm_used);
}

int pat_smptogm(int smp)
{
	if( smp && smp < MAXSMP && pat_gm_used[smp - 1] < MAXSMP )
		return pat_gm_used[smp - 1];
	return 1;
}

int pat_gmtosmp(int gm)
{
	int smp;
	for( smp=0; pat_gm_used[smp]; smp++ )
		if( pat_gm_used[smp] == gm )
			return smp+1;
	if( smp < MAXSMP ) {
		pat_gm_used[smp] = gm;
		return smp+1;
	}
	return 1;
}

int pat_smplooped(int smp)
{
	if( smp < MAXSMP ) return pat_loops[smp - 1];
	return 1;
}

const char *pat_gm_name(int gm)
{
	static char buf[40];
	if( gm < 1 || gm > MAXSMP ) {
		sprintf(buf, "invalid gm %d", gm);
		return buf;
	}
	return midipat[gm - 1];
}

int pat_gm_drumnr(int n)
{
	if( n < 25 ) return 129;
	if( n+129-25 < MAXSMP )
		return 129+n-25; // timidity.cfg drum patches start at 25
	return MAXSMP;
}

int pat_gm_drumnote(int n)
{
	char *p;
	p = strchr(midipat[pat_gm_drumnr(n)-1], ':');
	if( p ) return pat_getopt(p+1, "note", n);
	return n;
}

static float pat_sinus(int i)
{
	float res = sinf(OMEGA * (float)i);
	return res;
}

static float pat_square(int i)
{
	float res = 30.0f * sinf(OMEGA * (float)i);
	if( res > 0.99f ) return 0.99f;
	if( res < -0.99f ) return -0.99f;
	return res;
}

static float pat_sawtooth(int i)
{
	float res = OMEGA * (float)i;
	while( res > 2 * PI )
		res -= 2 * PI;
	i = 2;
	if( res > PI ) {
		res = PI - res;
		i = -2;
	}
	res = (float)i * res / PI;
	if( res > 0.9f ) return 1.0f - res;
	if( res < -0.9f ) return 1.0f + res;
	return res;
}

typedef float (*PAT_SAMPLE_FUN)(int);

static PAT_SAMPLE_FUN pat_fun[] = { pat_sinus, pat_square, pat_sawtooth };

#if defined(_WIN32) && defined(_mm_free)
#undef _mm_free
#endif

#define MMSTREAM				FILE
#define _mm_fopen(name,mode)			fopen(name,mode)
#define _mm_fgets(f,buf,sz)			fgets(buf,sz,f)
#define _mm_fseek(f,pos,whence)			fseek(f,pos,whence)
#define _mm_ftell(f)				ftell(f)
#define _mm_read_UBYTES(buf,sz,f)		fread(buf,sz,1,f)
#define _mm_read_SBYTES(buf,sz,f)		fread(buf,sz,1,f)
#define _mm_feof(f)				feof(f)
#define _mm_fclose(f)				fclose(f)
#define DupStr(h,buf,sz)			strdup(buf)
#define _mm_calloc(h,n,sz)			calloc(n,sz)
#define _mm_recalloc(h,buf,sz,elsz)	realloc(buf,sz)
#define _mm_free(h,p)				free(p)

long _mm_getfsize(MMSTREAM *mmpat) {
	long fsize;
	_mm_fseek(mmpat, 0L, SEEK_END);
	fsize = _mm_ftell(mmpat);
	_mm_fseek(mmpat, 0L, SEEK_SET);
	return(fsize);
}

void pat_init_patnames(void)
{
	int z, i, nsources, isdrumset, nskip, pfnlen;
	char *p, *q;
	char line[PATH_MAX];
	char cfgsources[5][PATH_MAX] = {{0}, {0}, {0}, {0}, {0}};
	MMSTREAM *mmcfg;
	strncpy(pathforpat, PATHFORPAT, PATH_MAX);
	strncpy(timiditycfg, TIMIDITYCFG, PATH_MAX);
	p = getenv(PAT_ENV_PATH2CFG);
	if( p ) {
		strncpy(timiditycfg, p, PATH_MAX - 14);
		strncpy(pathforpat, p, PATH_MAX - 13);
		strcat(timiditycfg, "/timidity.cfg");
		strcat(pathforpat, "/instruments");
	}
	strncpy(cfgsources[0], timiditycfg, PATH_MAX - 1);
	cfgsources[0][PATH_MAX - 1] = '\0';
	nsources = 1;

	for( i=0; i<MAXSMP; i++ )	midipat[i][0] = '\0';

	for ( z=0; z<5; z++ ) {
		if (cfgsources[z][0] == 0) continue;
		mmcfg = _mm_fopen(cfgsources[z],"r");
		if( !mmcfg ) {
			pat_message("can not open %s, use environment variable " PAT_ENV_PATH2CFG " for the directory", cfgsources[z]);
		}
		else {
			// read in bank 0 and drum patches
			isdrumset = 0;
			_mm_fgets(mmcfg, line, PATH_MAX);
			while( !_mm_feof(mmcfg) ) {
			p = line;
			while ( isspace(*p) ) p ++;
			if( isdigit(p[0]) ) {
				// get pat number
				i = atoi(p);
				while ( isdigit(*p) ) p ++;
				while ( isspace(*p) ) p ++;
				// get pat file name
				if( *p && i < MAXSMP && i >= 0 && *p != '#' ) {
					q = isdrumset ? midipat[pat_gm_drumnr(i)-1] : midipat[i];
					pfnlen = 0;
					while( *p && !isspace(*p) && *p != '#' && pfnlen < 128 ) {
						pfnlen ++;
						*q++ = *p++;
					}
					if( isblank(*p) && *(p+1) != '#' && pfnlen < 128 ) {
						*q++ = ':'; pfnlen ++;
						while( isspace(*p) ) {
							while( isspace(*p) ) p++;
							if ( *p == '#' ) { // comment

							} else while( *p && !isspace(*p) && pfnlen < 128 ) {
								pfnlen ++;
								*q++ = *p++;
							}
							if( isspace(*p) ) { *q++ = ' '; pfnlen++; }
						}
					}
					*q++ = '\0';
				}
			}
			else if( !strncmp(p,"bank",4) ) isdrumset = 0;
			else if( !strncmp(p,"drumset",7) ) isdrumset = 1;
			else if( !strncmp(p,"soundfont",9) ) {
				fprintf(stderr, "warning: soundfont directive unsupported!\n");
			}
			else if( !strncmp(p,"dir",3) )  {
				p += 3;
				while ( isspace(*p) ) p ++;
				q = p + strlen(p);
				if(q > p) {
					--q;
					while ( q > p && isspace(*q) ) *(q--) = 0;
					strncpy(pathforpat, p, PATH_MAX - 1);
					pathforpat[PATH_MAX - 1] = 0;
				}
			}
			else if( !strncmp(p,"source",6) && nsources < 5 ) {
				q = cfgsources[nsources];
				p += 6;
				while ( isspace(*p) ) p ++;
				pfnlen = 0;
				while ( *p && *p != '#' && !isspace(*p) && pfnlen < 128 ) {
					pfnlen ++;
					*q++ = *p++;
				}
				*q = 0; // null termination
				nsources++;
			}
			_mm_fgets(mmcfg, line, PATH_MAX);

			} /* end file parsing */
			_mm_fclose(mmcfg);
		}
	}
	q = midipat[0];
	nskip = 0;
	// make all empty patches duplicates the previous valid one.
	for( i=0; i<MAXSMP; i++ )	{
		if( midipat[i][0] ) q = midipat[i];
		else {
			if( midipat[i] != q)
				strcpy(midipat[i], q);
			if( midipat[i][0] == '\0' ) nskip++;
		}
	}
	if( nskip ) {
		for( i=MAXSMP; i-- > 0; )	{
			if( midipat[i][0] ) q = midipat[i];
			else if( midipat[i] != q )
				strcpy(midipat[i], q);
		}
	}
}

static char *pat_build_path(char *fname, int pat)
{
	char *ps, *p;
	char *patfile = midipat[pat];
	int has_ext = 0, isabspath = (patfile[0] == '/');
	if ( isabspath ) patfile ++;
	ps = strrchr(patfile, ':');
	if( ps ) {
		sprintf(fname, "%s%c%s", isabspath ? "" : pathforpat, DIRDELIM, patfile);
		strcpy(strrchr(fname, ':'), ".pat");
		return ps;
	}
	p = strrchr(patfile, '.');
	if(p && !strcasecmp(p, ".pat")) has_ext = 1;
	sprintf(fname, "%s%c%s%s", isabspath ? "" : pathforpat, DIRDELIM, patfile, has_ext ? "" : ".pat");
	return 0;
}

static void pat_read_patname(PATHANDLE *h, MMFILE *mmpat) {
	InstrumentHeader ih;
	mmfseek(mmpat,sizeof(PatchHeader), SEEK_SET);
	mmreadUBYTES((BYTE *)&ih, sizeof(InstrumentHeader), mmpat);
	strncpy(h->patname, ih.instrument_name, 16);
	h->patname[15] = '\0';
}

static void pat_read_layerheader(MMSTREAM *mmpat, LayerHeader *hl)
{
	_mm_fseek(mmpat,sizeof(PatchHeader)+sizeof(InstrumentHeader), SEEK_SET);
	_mm_read_UBYTES((BYTE *)hl, sizeof(LayerHeader), mmpat);
}

static void pat_get_layerheader(MMFILE *mmpat, LayerHeader *hl)
{
	InstrumentHeader ih;
	mmfseek(mmpat,sizeof(PatchHeader), SEEK_SET);
	mmreadUBYTES((BYTE *)&ih, sizeof(InstrumentHeader), mmpat);
	mmreadUBYTES((BYTE *)hl, sizeof(LayerHeader), mmpat);
	strncpy(hl->reserved, ih.instrument_name, 40);
}

static int pat_read_numsmp(MMFILE *mmpat) {
	LayerHeader hl;
	pat_get_layerheader(mmpat, &hl);
	return hl.samples;
}

static void pat_read_waveheader(MMSTREAM *mmpat, WaveHeader *hw, int layer)
{
	long int pos, bestpos=0;
	LayerHeader hl;
	ULONG bestfreq, freqdist;
	int i;
	// read the very first and maybe only sample
	pat_read_layerheader(mmpat, &hl);
	if (hl.samples > MAXSMP) hl.samples = MAXSMP;
	if( hl.samples > 1 ) {
		if( layer ) {
			if( layer > hl.samples ) layer = hl.samples; // you don't fool me....
			for( i=1; i<layer; i++ ) {
				_mm_read_UBYTES((BYTE *)hw, sizeof(WaveHeader), mmpat);
				_mm_fseek(mmpat, hw->wave_size, SEEK_CUR);
			}
		}
		else {
			bestfreq = C4mHz * 1000;	// big enough
			for( i=0; i<hl.samples; i++ ) {
				pos = _mm_ftell(mmpat);
				_mm_read_UBYTES((BYTE *)hw, sizeof(WaveHeader), mmpat);
				if( hw->root_frequency > C4mHz )
					freqdist = hw->root_frequency - C4mHz;
				else
					freqdist = 2 * (C4mHz - hw->root_frequency);
				if( freqdist < bestfreq ) {
					bestfreq = freqdist;
					bestpos  = pos;
				}
				_mm_fseek(mmpat, hw->wave_size, SEEK_CUR);
			}
			// if invalid bestpos, assume the start.
			if( bestpos < 0 )
				bestpos = 0;
			_mm_fseek(mmpat, bestpos, SEEK_SET);
		}
	}
	_mm_read_UBYTES((BYTE *)hw, sizeof(WaveHeader), mmpat);
	strncpy(hw->reserved, hl.reserved, 32);
	hw->reserved[31] = 0;
	if( hw->start_loop >= hw->wave_size ) {
		hw->start_loop = 0;
		hw->end_loop = 0;
		hw->modes &= ~PAT_LOOP; // mask off loop indicator
	}
	if( hw->end_loop > hw->wave_size )
		hw->end_loop = hw->wave_size;
}

static void pat_get_waveheader(MMFILE *mmpat, WaveHeader *hw, int layer)
{
	long int pos, bestpos=0;
	LayerHeader hl;
	ULONG bestfreq, freqdist;
	int i;
	// read the very first and maybe only sample
	pat_get_layerheader(mmpat, &hl);
	if( hl.samples > 1 ) {
		if( layer ) {
			if( layer > hl.samples ) layer = hl.samples; // you don't fool me....
			for( i=1; i<layer; i++ ) {
				mmreadUBYTES((BYTE *)hw, sizeof(WaveHeader), mmpat);
				mmfseek(mmpat, hw->wave_size, SEEK_CUR);
				if ( mmpat->error ) {
					hw->wave_size = 0;
					return;
				}
			}
		}
		else {
			bestfreq = C4mHz * 1000;	// big enough
			for( i=0; i<hl.samples; i++ ) {
				pos = mmftell(mmpat);
				mmreadUBYTES((BYTE *)hw, sizeof(WaveHeader), mmpat);
				if( hw->root_frequency > C4mHz )
					freqdist = hw->root_frequency - C4mHz;
				else
					freqdist = 2 * (C4mHz - hw->root_frequency);
				if( freqdist < bestfreq ) {
					bestfreq = freqdist;
					bestpos  = pos;
				}
				mmfseek(mmpat, hw->wave_size, SEEK_CUR);
			}
			mmfseek(mmpat, bestpos, SEEK_SET);
		}
	}
	mmreadUBYTES((BYTE *)hw, sizeof(WaveHeader), mmpat);
	if( hw->start_loop >= hw->wave_size ) {
		hw->start_loop = 0;
		hw->end_loop = 0;
		hw->modes &= ~PAT_LOOP; // mask off loop indicator
	}
	if( hw->end_loop > hw->wave_size )
		hw->end_loop = hw->wave_size;
}

static int pat_readpat_attr(int pat, WaveHeader *hw, int layer)
{
	char fname[128];
	unsigned long fsize;
	MMSTREAM *mmpat;
	pat_build_path(fname, pat);
	mmpat = _mm_fopen(fname, "rb");
	if( !mmpat )
		return 0;
	fsize = _mm_getfsize(mmpat);
	pat_read_waveheader(mmpat, hw, layer);
	_mm_fclose(mmpat);
	if (hw->wave_size > fsize)
		return 0;
	return 1;
}

static void pat_amplify(char *b, int num, int amp, int m)
{
	char *pb;
	BYTE *pu;
	short int *pi;
	WORD *pw;
	int i,n,v;
	n = num;
	if( m & PAT_16BIT ) { // 16 bit
		n >>= 1;
		if( m & 2 ) {	// unsigned
			pw = (WORD *)b;
			for( i=0; i<n; i++ ) {
				v = (((int)(*pw) - 0x8000) * amp) / 100;
				if( v < -0x8000 ) v = -0x8000;
				if( v >  0x7fff ) v =  0x7fff;
				*pw++ = v + 0x8000;
			}
		}
		else {
			pi = (short int *)b;
			for( i=0; i<n; i++ ) {
				v = ((*pi) * amp) / 100;
				if( v < -0x8000 ) v = -0x8000;
				if( v >  0x7fff ) v =  0x7fff;
				*pi++ = v;
			}
		}
	}
	else {
		if( m & 2 ) {	// unsigned
			pu = (BYTE *)b;
			for( i=0; i<n; i++ ) {
				v = (((int)(*pu) - 0x80) * amp) / 100;
				if( v < -0x80 ) v = -0x80;
				if( v >  0x7f ) v =  0x7f;
				*pu++ = v + 0x80;
			}
		}
		else {
			pb = (char *)b;
			for( i=0; i<n; i++ ) {
				v = ((*pb) * amp) / 100;
				if( v < -0x80 ) v = -0x80;
				if( v >  0x7f ) v =  0x7f;
				*pb++ = v;
			}
		}
	}
}

static int pat_getopt(const char *s, const char *o, int dflt)
{
	const char *p;
	if( !s ) return dflt;
	p = strstr(s,o);
	if( !p ) return dflt;
	return atoi(strchr(p,'=')+1);
}

static void pat_readpat(int pat, char *dest, int num)
{
	static int readlasttime = 0, wavesize = 0;
	static MMSTREAM *mmpat = 0;
	static char *opt = 0;
	int amp;
	char fname[128];
	WaveHeader hw;
	if( !readlasttime ) {
		opt=pat_build_path(fname, pat);
		mmpat = _mm_fopen(fname, "rb");
		if( !mmpat )
			return;
		pat_read_waveheader(mmpat, &hw, 0);
		wavesize = hw.wave_size;
	}
	_mm_read_SBYTES(dest, num, mmpat);
	amp = pat_getopt(opt,"amp",100);
	if( amp != 100 ) pat_amplify(dest, num, amp, hw.modes);
	readlasttime += num;
	if( readlasttime < wavesize ) return;
	readlasttime = 0;
	_mm_fclose(mmpat);
	mmpat = 0;
}

static BOOL dec_pat_Decompress16Bit(short int *dest, int cbcount, int samplenum)
{
	int i;
	PAT_SAMPLE_FUN f;
	if( samplenum < MAXSMP ) pat_readpat(samplenum, (char *)dest, cbcount*2);
	else {
		f = pat_fun[(samplenum - MAXSMP) % 3];
		for( i=0; i<cbcount; i++ )
			dest[i] = (short int)(32000.0*f(i));
	}
	return cbcount;
}

// convert 8 bit data to 16 bit!
// We do the conversion in reverse so that the data we're converting isn't overwritten
// by the result.
static void	pat_blowup_to16bit(short int *dest, int cbcount) {
	char *s;
	short int *d;
	int t;
	s  = (char *)dest;
	d  = dest;
	s += cbcount;
	d += cbcount;
	for(t=0; t<cbcount; t++)
	{
		s--;
		d--;
		*d = (*s) << 8;
	}
}

static BOOL dec_pat_Decompress8Bit(short int *dest, int cbcount, int samplenum)
{
	int i;
	PAT_SAMPLE_FUN f;
	if( samplenum < MAXSMP ) {
		pat_readpat(samplenum, (char *)dest, cbcount);
		pat_blowup_to16bit(dest, cbcount);
	} else {
		f = pat_fun[(samplenum - MAXSMP) % 3];
		for( i=0; i<cbcount; i++ )
			dest[i] = (short int)(120.0*f(i)) << 8;
	}

	return cbcount;
}

// =====================================================================================
BOOL CSoundFile::TestPAT(const BYTE *lpStream, DWORD dwMemLength)
// =====================================================================================
{
	PatchHeader ph;
	if( dwMemLength < sizeof(PatchHeader) ) return 0;
	memcpy((BYTE *)&ph, lpStream, sizeof(PatchHeader));
	if( !strcmp(ph.header,"GF1PATCH110") && !strcmp(ph.gravis_id,"ID#000002") ) return 1;
	return 0;
}

// =====================================================================================
static PATHANDLE *PAT_Init(void)
{
	return (PATHANDLE *)calloc(1,sizeof(PATHANDLE));
}

// =====================================================================================
static void PAT_Cleanup(PATHANDLE *handle)
// =====================================================================================
{
	if(handle) {
		free(handle);
	}
}

static char tune[] = "c d e c|c d e c|e f g..|e f g..|gagfe c|gagfe c|c G c..|c G c..|";
static int pat_note(int abc)
{
	switch( abc ) {
		case 'C': return 48;
		case 'D': return 50;
		case 'E': return 52;
		case 'F': return 53;
		case 'G': return 55;
		case 'A': return 57;
		case 'B': return 59;
		case 'c': return 60;
		case 'd': return 62;
		case 'e': return 64;
		case 'f': return 65;
		case 'g': return 67;
		case 'a': return 69;
		case 'b': return 71;
		default:
			break;
	}
	return 0;
}

int pat_modnote(int midinote)
{
	int n;
	n = midinote;
	n += 13;
	return n;
}

// =====================================================================================
static void PAT_ReadPatterns(MODCOMMAND *pattern[], WORD psize[], PATHANDLE *h, int numpat)
// =====================================================================================
{
	int pat,row,i,ch;
	BYTE n,ins,vol;
	int t;
	int tt1, tt2;
	MODCOMMAND *m;
	if( numpat > MAX_PATTERNS ) numpat = MAX_PATTERNS;

	tt2 = (h->samples - 1) * 16 + 128;
	for( pat = 0; pat < numpat; pat++ ) {
		pattern[pat] = CSoundFile::AllocatePattern(64, h->samples);
		if( !pattern[pat] ) return;
		psize[pat] = 64;
		for( row = 0; row < 64; row++ ) {
			tt1 = (pat * 64 + row);
			for( ch = 0; ch < h->samples; ch++ ) {
				t = tt1 - ch * 16;
				m = &pattern[pat][row * h->samples + ch];
				m->param   = 0;
				m->command = CMD_NONE;
				if( t >= 0 ) {
					i = tt2 - 16 * ((h->samples - 1 - ch) & 3);
					if( tt1 < i ) {
						t = t % 64;
						if( isalpha(tune[t]) ) {
							n   = pat_modnote(pat_note(tune[t]));
							ins = ch + 1;
							vol = 40;
							if( (t % 16) == 0 ) {
								vol += vol / 10;
								if( vol > 64 ) vol = 64;
							}
							m->instr  = ins;
							m->note   = n; // <- normal note
							m->volcmd = VOLCMD_VOLUME;
							m->vol    = vol;
						}
						if( tt1 ==  i - 1 && ch == 0 && row < 63 ) {
							m->command = CMD_PATTERNBREAK;
						}
					}
					else {
						if( tt1 == i ) {
							m->param   = 0;
							m->command = CMD_KEYOFF;
							m->volcmd = VOLCMD_VOLUME;
							m->vol    = 0;
						}
					}
				}
			}
		}
	}
}

// calculate the best speed that approximates the pat root frequency as a C note
static ULONG pat_patrate_to_C4SPD(ULONG patRate , ULONG patMilliHz)
{
	ULONG u;
	double x, y;
	u = patMilliHz;
	x = 0.1 * patRate;
	x = x * C4mHz;
	y = u * 0.4;
	x = x / y;
	u = (ULONG)(x+0.5);
	return u;
}

// return relative position in samples for the rate starting with offset start ending with offset end
static int pat_envelope_rpos(int rate, int start, int end)
{
	int r, p, t, s;
	// rate byte is 3 bits exponent and 6 bits increment size
	//   eeiiiiii
	// every 8 to the power ee the volume is incremented/decremented by iiiiii
	// Thank you Gravis for this weirdness...
	r = 3 - ((rate >> 6) & 3) * 3;
	p = rate & 0x3f;
	if( !p ) return 0;
	t = end - start;
	if( !t ) return 0;
	if (t < 0) t = -t;
	s = (t << r)/ p;
	return s;
}

static void	pat_modenv(WaveHeader *hw, int mpos[6], int mvol[6])
{
	int i, sum, s;
	BYTE *prate = hw->envelope_rate, *poffset = hw->envelope_offset;
	for( i=0; i<6; i++ ) {
		mpos[i] = 0;
		mvol[i] = 64;
	}
	if( !memcmp(prate, "??????", 6) || poffset[5] >= 100 ) return; // weird rates or high env end volume
	if( !(hw->modes & PAT_SUSTAIN) ) return; // no sustain thus no need for envelope
	s = hw->wave_size;
	if (s == 0) return;
	if( hw->modes & PAT_16BIT )
		s >>= 1;
	// offsets 0 1 2 3 4 5 are distributed over 0 2 4 6 8 10, the odd numbers are set in between
	sum = 0;
	for( i=0; i<6; i++ ) {
		mvol[i] = poffset[i];
		mpos[i] = pat_envelope_rpos(prate[i], i? poffset[i-1]: 0, poffset[i]);
		sum += mpos[i];
	}
	if( sum == 0 ) return;
	if( sum > s ) {
		for( i=0; i<6; i++ )
			mpos[i] = (s * mpos[i]) / sum;
	}
	for( i=1; i<6; i++ )
		mpos[i] += mpos[i-1];
	for( i=0; i<6 ; i++ ) {
		mpos[i] = (256 * mpos[i]) / s;
		mpos[i]++;
		if( i > 0 && mpos[i] <= mpos[i-1] ) {
			if( mvol[i] == mvol[i-1] ) mpos[i] = mpos[i-1];
			else mpos[i] = mpos[i-1] + 1;
		}
		if( mpos[i] > 256 ) mpos[i] = 256;
	}
	mvol[5] = 0; // kill Bill....
}

static void pat_setpat_inst(WaveHeader *hw, INSTRUMENTHEADER *d, int smp)
{
	int u, inuse;
	int envpoint[6], envvolume[6];
	d->nMidiProgram = 0;
	d->nFadeOut = 0;
	d->nPan = 128;
	d->nPPC = 5*12;
	d->dwFlags = 0;
	if( hw->modes & PAT_ENVELOPE ) d->dwFlags |= ENV_VOLUME;
	if( hw->modes & PAT_SUSTAIN ) d->dwFlags |= ENV_VOLSUSTAIN;
	if( (hw->modes & PAT_LOOP) && (hw->start_loop != hw->end_loop) ) d->dwFlags |= ENV_VOLLOOP;
	d->nVolEnv = 6;
	//if (!d->nVolEnv) d->dwFlags &= ~ENV_VOLUME;
	d->nPanEnv = 0;
	d->nVolSustainBegin = 1;
	d->nVolSustainEnd   = 1;
	d->nVolLoopStart    = 1;
	d->nVolLoopEnd      = 2;
	d->nPanSustainBegin = 0;
	d->nPanSustainEnd   = 0;
	d->nPanLoopStart    = 0;
	d->nPanLoopEnd      = 0;
	d->nGlobalVol = 64;
	pat_modenv(hw, envpoint, envvolume);
	inuse = 0;
	for( u=0; u<6; u++)
	{
		if( envvolume[u] != 64 ) inuse = 1;
		d->VolPoints[u] = envpoint[u];
		d->VolEnv[u]    = envvolume[u];
		d->PanPoints[u] = 0;
		d->PanEnv[u]    = 0;
		if (u)
		{
			if (d->VolPoints[u] < d->VolPoints[u-1])
			{
				d->VolPoints[u] &= 0xFF;
				d->VolPoints[u] += d->VolPoints[u-1] & 0xFF00;
				if (d->VolPoints[u] < d->VolPoints[u-1]) d->VolPoints[u] += 0x100;
			}
		}
	}
	if( !inuse ) d->nVolEnv = 0;
	for( u=0; u<128; u++)
	{
		d->NoteMap[u] = u+1;
		d->Keyboard[u] = smp;
	}
}

static void PATinst(INSTRUMENTHEADER *d, int smp, int gm)
{
	WaveHeader hw;
	if( pat_readpat_attr(gm-1, &hw, 0) ) {
		pat_setpat_inst(&hw, d, smp);
	}
	else {
		hw.modes = PAT_16BIT|PAT_ENVELOPE|PAT_SUSTAIN|PAT_LOOP;
		hw.start_loop = 0;
		hw.end_loop = 30000;
		hw.wave_size  = 30000;
//  envelope rates and offsets pinched from timidity's acpiano.pat sample no 1
		hw.envelope_rate[0] = 0x3f;
		hw.envelope_rate[1] = 0x3f;
		hw.envelope_rate[2] = 0x3f;
		hw.envelope_rate[3] = 0x08|(3<<6);
		hw.envelope_rate[4] = 0x3f;
		hw.envelope_rate[5] = 0x3f;
		hw.envelope_offset[0] = 246;
		hw.envelope_offset[1] = 246;
		hw.envelope_offset[2] = 246;
		hw.envelope_offset[3] = 0;
		hw.envelope_offset[4] = 0;
		hw.envelope_offset[5] = 0;
		strncpy(hw.reserved, midipat[gm-1], sizeof(hw.reserved));
		hw.reserved[sizeof(hw.reserved) - 1] = '\0';
		pat_setpat_inst(&hw, d, smp);
	}
	/* strncpy 0-inits the entire field. */
	strncpy((char *)d->name, hw.reserved[0] ? hw.reserved : midipat[gm-1], 32);
	d->name[31] = '\0';

	strncpy((char *)d->filename, midipat[gm-1], 12);
	d->filename[11] = '\0';
}

static void pat_setpat_attr(WaveHeader *hw, MODINSTRUMENT *q)
{
	q->nC4Speed   = pat_patrate_to_C4SPD(hw->sample_rate , hw->root_frequency);
	q->nLength    = hw->wave_size;
	q->nLoopStart = hw->start_loop;
	q->nLoopEnd   = hw->end_loop;
	q->nVolume    = 256;
	if( hw->modes & PAT_16BIT ) {
		q->nLength    >>= 1;
		q->nLoopStart >>= 1;
		q->nLoopEnd   >>= 1;
	}
	if( hw->modes & PAT_LOOP ) {
		q->uFlags |= CHN_LOOP;
		if( hw->modes & PAT_PINGPONG ) q->uFlags |= CHN_PINGPONGSUSTAIN;
		if( hw->modes & PAT_SUSTAIN ) q->uFlags |= CHN_SUSTAINLOOP;
	}
}

// ==========================
// Load those darned Samples!
static void PATsample(CSoundFile *cs, MODINSTRUMENT *q, int smp, int gm)
{
	WaveHeader hw;
	char s[PATH_MAX + 32];
	sprintf(s, "%d:%s", smp-1, midipat[gm-1]);
	s[31] = '\0';

#if defined(__GNUC__) && __GNUC__ >= 8
/* GCC's warning is broken and ignores the manual termination in this case. */
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
	/* strncpy 0-inits the entire field. */
	strncpy(cs->m_szNames[smp], s, 32);
	cs->m_szNames[smp][31] = '\0';

	q->nGlobalVol = 64;
	q->nPan       = 128;
	q->uFlags     = CHN_16BIT;
	if( pat_readpat_attr(gm-1, &hw, 0) ) {
		char *p;
		pat_setpat_attr(&hw, q);
		pat_loops[smp-1] = (q->uFlags & CHN_LOOP)? 1: 0;
		if( hw.modes & PAT_16BIT ) p = (char *)malloc(hw.wave_size);
		else p = (char *)malloc(hw.wave_size * sizeof(char)*2);
		if( p ) {
			if( hw.modes & PAT_16BIT ) {
				dec_pat_Decompress16Bit((short int *)p, hw.wave_size>>1, gm - 1);
				cs->ReadSample(q, (hw.modes&PAT_UNSIGNED)?RS_PCM16U:RS_PCM16S, (LPSTR)p, hw.wave_size);
			}
			else {
				dec_pat_Decompress8Bit((short int *)p, hw.wave_size, gm - 1);
				cs->ReadSample(q, (hw.modes&PAT_UNSIGNED)?RS_PCM16U:RS_PCM16S, (LPSTR)p, hw.wave_size * sizeof(short int));
			}
			free(p);
		}
	}
	else {
		char *p;
		q->nC4Speed   = C4SPD;
		q->nLength    = 30000;
		q->nLoopStart = 0;
		q->nLoopEnd   = 30000;
		q->nVolume    = 256;
		q->uFlags    |= CHN_LOOP;
		q->uFlags    |= CHN_16BIT;
		p = (char *)malloc(q->nLength*sizeof(char)*2);
		if( p ) {
			dec_pat_Decompress8Bit((short int *)p, q->nLength, smp + MAXSMP - 1);
			cs->ReadSample(q, RS_PCM16S, (LPSTR)p, q->nLength*2);
			free(p);
		}
	}
}

// =====================================================================================
BOOL PAT_Load_Instruments(void *c)
{
	uint32_t t;
	CSoundFile *of=(CSoundFile *)c;
	if( !pat_numsmp() ) pat_gmtosmp(1); // make sure there is a sample
	of->m_nSamples     = pat_numsmp() + 1; // xmms modplug does not use slot zero
	of->m_nInstruments = pat_numinstr() + 1;
	for(t=1; t<of->m_nInstruments; t++) { // xmms modplug doesn't use slot zero
		if( (of->Headers[t] = new INSTRUMENTHEADER) == NULL ) return FALSE;
		memset(of->Headers[t], 0, sizeof(INSTRUMENTHEADER));
		PATinst(of->Headers[t], t, pat_smptogm(t));
	}
	for(t=1; t<of->m_nSamples; t++) { // xmms modplug doesn't use slot zero
		PATsample(of, &of->Ins[t], t, pat_smptogm(t));
	}
	// copy last of the mohicans to entry 0 for XMMS modinfo to work....
	t = of->m_nInstruments - 1;
	if( (of->Headers[0] = new INSTRUMENTHEADER) == NULL ) return FALSE;
	memcpy(of->Headers[0], of->Headers[t], sizeof(INSTRUMENTHEADER));
	memset(of->Headers[0]->name, 0, 32);
	strncpy((char *)of->Headers[0]->name, "Timidity GM patches", 32);
	t = of->m_nSamples - 1;
	memcpy(&of->Ins[0], &of->Ins[t], sizeof(MODINSTRUMENT));
	return TRUE;
}

// =====================================================================================
BOOL CSoundFile::ReadPAT(const BYTE *lpStream, DWORD dwMemLength)
{
	static int avoid_reentry = 0;
	char buf[60];
	int t;
	PATHANDLE *h;
	int numpat;
	MMFILE mm, *mmfile;
	MODINSTRUMENT *q;
	INSTRUMENTHEADER *d;
	if( !TestPAT(lpStream, dwMemLength) ) return FALSE;
	h = PAT_Init();
	if( !h ) return FALSE;
	mmfile = &mm;
	mm.mm = (char *)lpStream;
	mm.sz = dwMemLength;
	mm.pos = 0;
	mm.error = 0;
	while( avoid_reentry ) sleep(1);
	avoid_reentry = 1;
	pat_read_patname(h, mmfile);
	h->samples = pat_read_numsmp(mmfile);
	if( strlen(h->patname) )
		sprintf(buf,"%s canon %d-v (Fr. Jacques)", h->patname, h->samples);
	else
		sprintf(buf,"%d-voice canon (Fr. Jacques)", h->samples);
	if( strlen(buf) > 31 ) buf[31] = '\0'; // chop it of
	strcpy(m_szNames[0], buf);
	m_nDefaultTempo = 60; // 120 / 2
	t = (h->samples - 1) * 16 + 128;
	if( t % 64 ) t += 64;
	t = t / 64;
	m_nType         = MOD_TYPE_PAT;
	m_nInstruments  = h->samples >= MAX_INSTRUMENTS-1 ? MAX_INSTRUMENTS-1 : h->samples + 1; // we know better but use each sample in the pat...
	m_nSamples      = h->samples >= MAX_SAMPLES-1 ? MAX_SAMPLES-1 : h->samples + 1; // xmms modplug does not use slot zero
	m_nDefaultSpeed = 6;
	m_nChannels     = h->samples;
	numpat          = t;

	m_dwSongFlags   = SONG_LINEARSLIDES;
	m_nMinPeriod    = 28 << 2;
	m_nMaxPeriod    = 1712 << 3;
	// orderlist
	for(t=0; t < numpat; t++)
		Order[t] = t;
	for(t=1; t<(int)m_nInstruments; t++) { // xmms modplug doesn't use slot zero
		WaveHeader hw;
		char s[32];
		if( (d = new INSTRUMENTHEADER) == NULL ) {
			avoid_reentry = 0;
			return FALSE;
		}
		memset(d, 0, sizeof(INSTRUMENTHEADER));
		Headers[t] = d;
		sprintf(s, "%s", h->patname);
		s[31] = '\0';
		memset(d->name, 0, 32);
		strcpy((char *)d->name, s);
		s[11] = '\0';
		memset(d->filename, 0, 12);
		strcpy((char *)d->filename, s);
		pat_get_waveheader(mmfile, &hw, t);
		pat_setpat_inst(&hw, d, t);
	}
	for(t=1; t<(int)m_nSamples; t++) { // xmms modplug doesn't use slot zero
		WaveHeader hw;
		char s[32];
		char *p;
		q = &Ins[t];	// we do not use slot zero
		q->nGlobalVol = 64;
		q->nPan       = 128;
		q->uFlags     = CHN_16BIT;
		pat_get_waveheader(mmfile, &hw, t);
		pat_setpat_attr(&hw, q);
		memset(s,0,32);
		if( hw.wave_name[0] )
			sprintf(s, "%d:%s", t, hw.wave_name);
		else {
			if( h->patname[0] )
				sprintf(s, "%d:%s", t, h->patname);
			else
				sprintf(s, "%d:Untitled GM patch", t);
		}
		s[31] = '\0';
		memset(m_szNames[t], 0, 32);
		strcpy(m_szNames[t], s);
		if ( hw.wave_size == 0 ) p = NULL;
		else if( hw.modes & PAT_16BIT ) p = (char *)malloc(hw.wave_size);
		else p = (char *)malloc(hw.wave_size * sizeof(char) * 2);
		if( p ) {
			mmreadSBYTES(p, hw.wave_size, mmfile);
			if( hw.modes & PAT_16BIT ) {
				ReadSample(q, (hw.modes&PAT_UNSIGNED)?RS_PCM16U:RS_PCM16S, (LPSTR)p, hw.wave_size);
			}
			else {
				pat_blowup_to16bit((short int *)p, hw.wave_size);
				ReadSample(q, (hw.modes&PAT_UNSIGNED)?RS_PCM16U:RS_PCM16S, (LPSTR)p, hw.wave_size * sizeof(short int));
			}
			free(p);
		}
	}
	// copy last of the mohicans to entry 0 for XMMS modinfo to work....
	t = m_nInstruments - 1;
	if( (Headers[0] = new INSTRUMENTHEADER) == NULL ) {
		avoid_reentry = 0;
		return FALSE;
	}
	memcpy(Headers[0], Headers[t], sizeof(INSTRUMENTHEADER));
	memset(Headers[0]->name, 0, 32);
	if( h->patname[0] )
		strncpy((char *)Headers[0]->name, h->patname, 32);
	else
		strncpy((char *)Headers[0]->name, "Timidity GM patch", 32);
	t = m_nSamples - 1;
	memcpy(&Ins[0], &Ins[t], sizeof(MODINSTRUMENT));
	// ==============================
	// Load the pattern info now!
	PAT_ReadPatterns(Patterns, PatternSize, h, numpat);
	// ============================================================
	// set panning positions
	for(t=0; t<(int)m_nChannels; t++) {
		ChnSettings[t].nPan = 0x30+((t+2)%5)*((0xD0 - 0x30)/5);     // 0x30 = std s3m val
		ChnSettings[t].nVolume = 64;
	}
	avoid_reentry = 0; // it is safe now, I'm finished
	PAT_Cleanup(h);	// we dont need it anymore
	return 1;
}
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/


///////////////////////////////////////////////////
//
// PSM module loader
//
///////////////////////////////////////////////////
//#include "stdafx.h"
//#include "sndfile.h"

//#define PSM_LOG

#define PSM_ID_NEW	0x204d5350
#define PSM_ID_OLD	0xfe4d5350
#define IFFID_FILE	0x454c4946
#define IFFID_TITL	0x4c544954
#define IFFID_SDFT	0x54464453
#define IFFID_PBOD	0x444f4250
#define IFFID_SONG	0x474e4f53
#define IFFID_PATT	0x54544150
#define IFFID_DSMP	0x504d5344
#define IFFID_OPLH	0x484c504f

#pragma pack(1)

typedef struct _PSMCHUNK
{
	DWORD id;
	DWORD len;
	DWORD listid;
} PSMCHUNK;

void swap_PSMCHUNK(PSMCHUNK* p){
	p->id = bswapLE32(p->id);
	p->len = bswapLE32(p->len);
	p->listid = bswapLE32(p->listid);
}

typedef struct _PSMSONGHDR
{
	CHAR songname[8];	// "MAINSONG"
	BYTE reserved1;
	BYTE reserved2;
	BYTE channels;
} PSMSONGHDR;

typedef struct _PSMPATTERN
{
	DWORD size;
	DWORD name;
	WORD rows;
	WORD reserved1;
	BYTE data[4];
} PSMPATTERN;

void swap_PSMPATTERN(PSMPATTERN* p){
	p->size = bswapLE32(p->size);
	p->name = bswapLE32(p->name);
	p->rows = bswapLE16(p->rows);
}

typedef struct _PSMSAMPLE
{
	BYTE flags;
	CHAR songname[8];
	DWORD smpid;
	CHAR samplename[34];
	DWORD reserved1;
	BYTE reserved2;
	BYTE insno;
	BYTE reserved3;
	DWORD length;
	DWORD loopstart;
	DWORD loopend;
	WORD reserved4;
	BYTE defvol;
	DWORD reserved5;
	DWORD samplerate;
	BYTE reserved6[19];
} PSMSAMPLE;

#pragma pack()

void swap_PSMSAMPLE(PSMSAMPLE* p){
	p->smpid = bswapLE32(p->smpid);
	p->length = bswapLE32(p->length);
	p->loopstart = bswapLE32(p->loopstart);
	p->loopend = bswapLE32(p->loopend);
	p->samplerate = bswapLE32(p->samplerate);
}


BOOL CSoundFile::ReadPSM(LPCBYTE lpStream, DWORD dwMemLength)
//-----------------------------------------------------------
{
	PSMCHUNK pfh;
	DWORD dwMemPos, dwSongPos;
//	DWORD smpnames[MAX_SAMPLES];
	DWORD patptrs[MAX_PATTERNS];
	BYTE samplemap[MAX_SAMPLES];
	UINT nPatterns = 0;

	if (dwMemLength < 256) return FALSE;

	// Swap chunk
	pfh = *(const PSMCHUNK *)lpStream;
	swap_PSMCHUNK(&pfh);

	// Chunk0: "PSM ",filesize,"FILE"
	if (pfh.id == PSM_ID_OLD)
	{
	#ifdef PSM_LOG
		Log("Old PSM format not supported\n");
	#endif
		return FALSE;
	}
	if ((pfh.id != PSM_ID_NEW) || (pfh.len+12 > dwMemLength) || (pfh.listid != IFFID_FILE)) return FALSE;
	m_nType = MOD_TYPE_PSM;
	m_nChannels = 16;
	m_nSamples = 0;
	nPatterns = 0;
	dwMemPos = 12;
	dwSongPos = 0;
	for (UINT iChPan=0; iChPan<16; iChPan++)
	{
		UINT pan = (((iChPan & 3) == 1) || ((iChPan&3)==2)) ? 0xC0 : 0x40;
		ChnSettings[iChPan].nPan = pan;
	}
	while (dwMemPos+8 < dwMemLength)
	{
		PSMCHUNK pchunk = *(const PSMCHUNK *)(lpStream+dwMemPos);
		swap_PSMCHUNK(&pchunk);
		if ((pchunk.len >= dwMemLength - 8) || (dwMemPos + pchunk.len + 8 > dwMemLength)) break;
		dwMemPos += 8;
		PUCHAR pdata = (PUCHAR)(lpStream+dwMemPos);
		ULONG len = pchunk.len;
		if (len) switch(pchunk.id)
		{
		// "TITL": Song title
		case IFFID_TITL:
			if (!pdata[0]) { pdata++; len--; }
			memcpy(m_szNames[0], pdata, (len>31) ? 31 : len);
			m_szNames[0][31] = 0;
			break;
		// "PBOD": Pattern
		case IFFID_PBOD:
			if ((len >= 12) && (nPatterns < MAX_PATTERNS))
			{
				patptrs[nPatterns++] = dwMemPos-8;
			}
			break;
		// "SONG": Song description
		case IFFID_SONG:
			if ((len >= sizeof(PSMSONGHDR)+8) && (!dwSongPos))
			{
				dwSongPos = dwMemPos - 8;
			}
			break;
		// "DSMP": Sample Data
		case IFFID_DSMP:
			if ((len >= sizeof(PSMSAMPLE)) && (m_nSamples+1 < MAX_SAMPLES))
			{
				m_nSamples++;
				MODINSTRUMENT *pins = &Ins[m_nSamples];
				PSMSAMPLE psmp = *(PSMSAMPLE *)pdata;
				swap_PSMSAMPLE(&psmp);
			//	smpnames[m_nSamples] = psmp.smpid;
				memcpy(m_szNames[m_nSamples], psmp.samplename, 31);
				m_szNames[m_nSamples][31] = 0;
				samplemap[m_nSamples-1] = (BYTE)m_nSamples;
				// Init sample
				pins->nGlobalVol = 0x40;
				pins->nC4Speed = psmp.samplerate;
				pins->nLength = psmp.length;
				pins->nLoopStart = psmp.loopstart;
				pins->nLoopEnd = psmp.loopend;
				pins->nPan = 128;
				pins->nVolume = (psmp.defvol+1) * 2;
				pins->uFlags = (psmp.flags & 0x80) ? CHN_LOOP : 0;
				if (pins->nLoopStart > 0) pins->nLoopStart--;
				// Point to sample data
				pdata += 0x60;
				len -= 0x60;
				// Load sample data
				if ((pins->nLength > 3) && (len > 3))
				{
					ReadSample(pins, RS_PCM8D, (LPCSTR)pdata, len);
				} else
				{
					pins->nLength = 0;
				}
			}
			break;
	#if 0
		default:
			{
				CHAR s[8], s2[64];
				*(DWORD *)s = pchunk.id;
				s[4] = 0;
				wsprintf(s2, "%s: %4d bytes @ %4d\n", s, pchunk.len, dwMemPos);
				OutputDebugString(s2);
			}
	#endif
		}
		dwMemPos += pchunk.len;
	}
	// Step #1: convert song structure
	const PSMSONGHDR *pSong = (const PSMSONGHDR *)(lpStream+dwSongPos+8);
	if ((!dwSongPos) || (pSong->channels < 2) || (pSong->channels > 32)) return TRUE;
	m_nChannels = pSong->channels;
	// Valid song header -> convert attached chunks
	{
		DWORD dwSongEnd = dwSongPos + 8 + *(DWORD *)(lpStream+dwSongPos+4);
		dwMemPos = dwSongPos + 8 + 11; // sizeof(PSMCHUNK)+sizeof(PSMSONGHDR)
		while (dwMemPos + 8 < dwSongEnd)
		{
			PSMCHUNK pchunk = *(const PSMCHUNK *)(lpStream+dwMemPos);
			swap_PSMCHUNK(&pchunk);
			dwMemPos += 8;
			if ((pchunk.len > dwSongEnd) || (dwMemPos + pchunk.len > dwSongEnd)) break;
			PUCHAR pdata = (PUCHAR)(lpStream+dwMemPos);
			ULONG len = pchunk.len;
			switch(pchunk.id)
			{
			case IFFID_OPLH:
				if (len >= 0x20)
				{
					UINT pos = len - 3;
					while (pos > 5)
					{
						BOOL bFound = FALSE;
						pos -= 5;
						DWORD dwName = *(DWORD *)(pdata+pos);
						for (UINT i=0; i<nPatterns; i++)
						{
							DWORD dwPatName = ((const PSMPATTERN *)(lpStream+patptrs[i]+8))->name;
							if (dwName == dwPatName)
							{
								bFound = TRUE;
								break;
							}
						}
						if ((!bFound) && (pdata[pos+1] > 0) && (pdata[pos+1] <= 0x10)
						 && (pdata[pos+3] > 0x40) && (pdata[pos+3] < 0xC0))
						{
							m_nDefaultSpeed = pdata[pos+1];
							m_nDefaultTempo = pdata[pos+3];
							break;
						}
					}
					UINT iOrd = 0;
					while ((pos+5<len) && (iOrd < MAX_ORDERS))
					{
						DWORD dwName = *(DWORD *)(pdata+pos);
						for (UINT i=0; i<nPatterns; i++)
						{
							DWORD dwPatName = ((const PSMPATTERN *)(lpStream+patptrs[i]+8))->name;
							if (dwName == dwPatName)
							{
								Order[iOrd++] = i;
								break;
							}
						}
						pos += 5;
					}
				}
				break;
			}
			dwMemPos += pchunk.len;
		}
	}

	// Step #2: convert patterns
	for (UINT nPat=0; nPat<nPatterns; nPat++)
	{
		PSMPATTERN pPsmPat = *(const PSMPATTERN *)(lpStream+patptrs[nPat]+8);
		swap_PSMPATTERN(&pPsmPat);
		PSMCHUNK pchunk = *(const PSMCHUNK *)(lpStream+patptrs[nPat]);
		swap_PSMCHUNK(&pchunk);

		ULONG len = pchunk.len - 12;
		UINT nRows = pPsmPat.rows;
		if (len > pPsmPat.size) len = pPsmPat.size;
		if ((nRows < 64) || (nRows > 256)) nRows = 64;
		PatternSize[nPat] = nRows;
		if ((Patterns[nPat] = AllocatePattern(nRows, m_nChannels)) == NULL) break;
		MODCOMMAND *m = Patterns[nPat];
		const BYTE *p = lpStream + patptrs[nPat] + 20;
		MODCOMMAND *sp, dummy;
		UINT pos = 0;
		UINT row = 0;
		UINT rowlim;
	#ifdef PSM_LOG
		Log("Pattern %d at offset 0x%04X\n", nPat, (DWORD)(p - (BYTE *)lpStream));
	#endif
		UINT flags, ch;
		rowlim = bswapLE16(pPsmPat.reserved1)-2;
		while ((row < nRows) && (pos+3 < len))
		{
			if ((pos+1) >= rowlim) {
				pos = rowlim;
				rowlim = (((int)p[pos+1])<<8)
					| ((int)p[pos+0]);
				m += m_nChannels;
				row++;
				rowlim += pos;
				pos += 2;
			}
			if (row >= nRows) continue;

			flags = p[pos++];
			ch = p[pos++];
			if (ch >= m_nChannels) {
				sp = &dummy;
			} else {
				sp = &m[ch];
			}
			// Note + Instr
			if ((flags & 0x80) && (pos+1 < len))
			{
				UINT note = p[pos++];
				note = (note>>4)*12+(note&0x0f)+12+1;
				if (note > 0x80) note = 0;
				sp->note = note;
			}
			if ((flags & 0x40) && (pos+1 < len))
			{
				UINT nins = p[pos++];
				if (nins >= m_nSamples) {
			#ifdef PSM_LOG
				//if (!nPat) Log("note+ins: %02X.%02X\n", note, nins);
				if ((!nPat) && (nins >= m_nSamples)) Log("WARNING: invalid instrument number (%d)\n", nins);
			#endif
				} else {
					sp->instr = samplemap[nins];
				}
			}
			// Volume
			if ((flags & 0x20) && (pos < len))
			{
				sp->volcmd = VOLCMD_VOLUME;
				sp->vol = p[pos++] / 2;
			}
			// Effect
			if ((flags & 0x10) && (pos+1 < len))
			{
				UINT command = p[pos++];
				UINT param = p[pos++];
				// Convert effects
				switch(command)
				{
				// 01: fine volslide up
				case 0x01:	command = CMD_VOLUMESLIDE; param |= 0x0f;
						if (param == 15) param=31;
						break;
                                // 02: volslide up
                                case 0x02:      command = CMD_VOLUMESLIDE; param>>=1; param<<=4; break;
                                // 03: fine volslide down
                                case 0x03:	command = CMD_VOLUMESLIDE; param>>=4; param |= 0xf0;
						if (param == 240) param=241;
						break;
				// 04: fine volslide down
				case 0x04:	command = CMD_VOLUMESLIDE; param>>=4; param |= 0xf0; break;
				// 0C: portamento up
				case 0x0C:	command = CMD_PORTAMENTOUP; param = (param+1)/2; break;
				// 0E: portamento down
				case 0x0E:	command = CMD_PORTAMENTODOWN; param = (param+1)/2; break;
				// 0F: tone portamento
				case 0x0F:	command = CMD_TONEPORTAMENTO; param = param/4; break;
				// 15: vibrato
                                case 0x15:	command = CMD_VIBRATO; break;
				// 29: sample offset
				case 0x29:	pos += 2; break;
				// 2A: retrigger note
				case 0x2A:	command = CMD_RETRIG; break;
				// 33: Position Jump
				case 0x33:	command = CMD_POSITIONJUMP; break;
				// 34: Pattern break
				case 0x34:	command = CMD_PATTERNBREAK; break;
				// 3D: speed
				case 0x3D:	command = CMD_SPEED; break;
				// 3E: tempo
				case 0x3E:	command = CMD_TEMPO; break;
				// Unknown
				default:
				#ifdef PSM_LOG
					Log("Unknown PSM effect pat=%d row=%d ch=%d: %02X.%02X\n", nPat, row, ch, command, param);
				#endif
					command = param = 0;
				}
				sp->command = (BYTE)command;
				sp->param = (BYTE)param;
			}
		}
	#ifdef PSM_LOG
		if (pos < len)
		{
			Log("Pattern %d: %d/%d[%d] rows (%d bytes) -> %d bytes left\n", nPat, row, nRows, pPsmPat.rows, pPsmPat.size, len-pos);
		}
	#endif
	}

	// Done (finally!)
	return TRUE;
}


//////////////////////////////////////////////////////////////
//
// PSM Old Format
//

/*

CONST
  c_PSM_MaxOrder   = $FF;
  c_PSM_MaxSample  = $FF;
  c_PSM_MaxChannel = $0F;

 TYPE
  PPSM_Header = ^TPSM_Header;
  TPSM_Header = RECORD
                 PSM_Sign                   : ARRAY[01..04] OF CHAR; { PSM + #254 }
                 PSM_SongName               : ARRAY[01..58] OF CHAR;
                 PSM_Byte00                 : BYTE;
                 PSM_Byte1A                 : BYTE;
                 PSM_Unknown00              : BYTE;
                 PSM_Unknown01              : BYTE;
                 PSM_Unknown02              : BYTE;
                 PSM_Speed                  : BYTE;
                 PSM_Tempo                  : BYTE;
                 PSM_Unknown03              : BYTE;
                 PSM_Unknown04              : WORD;
                 PSM_OrderLength            : WORD;
                 PSM_PatternNumber          : WORD;
                 PSM_SampleNumber           : WORD;
                 PSM_ChannelNumber          : WORD;
                 PSM_ChannelUsed            : WORD;
                 PSM_OrderPosition          : LONGINT;
                 PSM_ChannelSettingPosition : LONGINT;
                 PSM_PatternPosition        : LONGINT;
                 PSM_SamplePosition         : LONGINT;
                { *** perhaps there are some more infos in a larger header,
                      but i have not decoded it and so it apears here NOT }
                END;

  PPSM_Sample = ^TPSM_Sample;
  TPSM_Sample = RECORD
                 PSM_SampleFileName  : ARRAY[01..12] OF CHAR;
                 PSM_SampleByte00    : BYTE;
                 PSM_SampleName      : ARRAY[01..22] OF CHAR;
                 PSM_SampleUnknown00 : ARRAY[01..02] OF BYTE;
                 PSM_SamplePosition  : LONGINT;
                 PSM_SampleUnknown01 : ARRAY[01..04] OF BYTE;
                 PSM_SampleNumber    : BYTE;
                 PSM_SampleFlags     : WORD;
                 PSM_SampleLength    : LONGINT;
                 PSM_SampleLoopBegin : LONGINT;
                 PSM_SampleLoopEnd   : LONGINT;
                 PSM_Unknown03       : BYTE;
                 PSM_SampleVolume    : BYTE;
                 PSM_SampleC5Speed   : WORD;
                END;

  PPSM_SampleList = ^TPSM_SampleList;
  TPSM_SampleList = ARRAY[01..c_PSM_MaxSample] OF TPSM_Sample;

  PPSM_Order = ^TPSM_Order;
  TPSM_Order = ARRAY[00..c_PSM_MaxOrder] OF BYTE;

  PPSM_ChannelSettings = ^TPSM_ChannelSettings;
  TPSM_ChannelSettings = ARRAY[00..c_PSM_MaxChannel] OF BYTE;

 CONST
  PSM_NotesInPattern   : BYTE = $00;
  PSM_ChannelInPattern : BYTE = $00;

 CONST
  c_PSM_SetSpeed = 60;

 FUNCTION PSM_Size(FileName : STRING;FilePosition : LONGINT) : LONGINT;
  BEGIN
  END;

 PROCEDURE PSM_UnpackPattern(VAR Source,Destination;PatternLength : WORD);
  VAR
   Witz : ARRAY[00..04] OF WORD;
   I1,I2        : WORD;
   I3,I4        : WORD;
   TopicalByte  : ^BYTE;
   Pattern      : PUnpackedPattern;
   ChannelP     : BYTE;
   NoteP        : BYTE;
   InfoByte     : BYTE;
   CodeByte     : BYTE;
   InfoWord     : WORD;
   Effect       : BYTE;
   Opperand     : BYTE;
   Panning      : BYTE;
   Volume       : BYTE;
   PrevInfo     : BYTE;
   InfoIndex    : BYTE;
  BEGIN
   Pattern     := @Destination;
   TopicalByte := @Source;
  { *** Initialize patttern }
   FOR I2 := 0 TO c_Maximum_NoteIndex DO
    FOR I3 := 0 TO c_Maximum_ChannelIndex DO
     BEGIN
      Pattern^[I2,I3,c_Pattern_NoteIndex]     := $FF;
      Pattern^[I2,I3,c_Pattern_SampleIndex]   := $00;
      Pattern^[I2,I3,c_Pattern_VolumeIndex]   := $FF;
      Pattern^[I2,I3,c_Pattern_PanningIndex]  := $FF;
      Pattern^[I2,I3,c_Pattern_EffectIndex]   := $00;
      Pattern^[I2,I3,c_Pattern_OpperandIndex] := $00;
     END;
  { *** Byte-pointer on first pattern-entry }
   ChannelP    := $00;
   NoteP       := $00;
   InfoByte    := $00;
   PrevInfo    := $00;
   InfoIndex   := $02;
  { *** read notes in pattern }
   PSM_NotesInPattern   := TopicalByte^; INC(TopicalByte); DEC(PatternLength); INC(InfoIndex);
   PSM_ChannelInPattern := TopicalByte^; INC(TopicalByte); DEC(PatternLength); INC(InfoIndex);
  { *** unpack pattern }
   WHILE (INTEGER(PatternLength) > 0) AND (NoteP < c_Maximum_NoteIndex) DO
    BEGIN
    { *** Read info-byte }
     InfoByte := TopicalByte^; INC(TopicalByte); DEC(PatternLength); INC(InfoIndex);
     IF InfoByte <> $00 THEN
      BEGIN
       ChannelP := InfoByte AND $0F;
       IF InfoByte AND 128 = 128 THEN { note and sample }
        BEGIN
        { *** read note }
         CodeByte := TopicalByte^; INC(TopicalByte); DEC(PatternLength);
         DEC(CodeByte);
         CodeByte := CodeByte MOD 12 * 16 + CodeByte DIV 12 + 2;
         Pattern^[NoteP,ChannelP,c_Pattern_NoteIndex] := CodeByte;
        { *** read sample }
         CodeByte := TopicalByte^; INC(TopicalByte); DEC(PatternLength);
         Pattern^[NoteP,ChannelP,c_Pattern_SampleIndex] := CodeByte;
        END;
       IF InfoByte AND 64 = 64 THEN { Volume }
        BEGIN
         CodeByte := TopicalByte^; INC(TopicalByte); DEC(PatternLength);
         Pattern^[NoteP,ChannelP,c_Pattern_VolumeIndex] := CodeByte;
        END;
       IF InfoByte AND 32 = 32 THEN { effect AND opperand }
        BEGIN
         Effect   := TopicalByte^; INC(TopicalByte); DEC(PatternLength);
         Opperand := TopicalByte^; INC(TopicalByte); DEC(PatternLength);
         CASE Effect OF
          c_PSM_SetSpeed:
           BEGIN
            Effect := c_I_Set_Speed;
           END;
          ELSE
           BEGIN
            Effect   := c_I_NoEffect;
            Opperand := $00;
           END;
         END;
         Pattern^[NoteP,ChannelP,c_Pattern_EffectIndex]   := Effect;
         Pattern^[NoteP,ChannelP,c_Pattern_OpperandIndex] := Opperand;
        END;
      END ELSE INC(NoteP);
    END;
  END;

 PROCEDURE PSM_Load(FileName : STRING;FilePosition : LONGINT;VAR Module : PModule;VAR ErrorCode : WORD);
 { *** caution : Module has to be inited before!!!! }
  VAR
   Header             : PPSM_Header;
   Sample             : PPSM_SampleList;
   Order              : PPSM_Order;
   ChannelSettings    : PPSM_ChannelSettings;
   MultiPurposeBuffer : PByteArray;
   PatternBuffer      : PUnpackedPattern;
   TopicalParaPointer : WORD;

   InFile : FILE;
   I1,I2  : WORD;
   I3,I4  : WORD;
   TempW  : WORD;
   TempB  : BYTE;
   TempP  : PByteArray;
   TempI  : INTEGER;
  { *** copy-vars for loop-extension }
   CopySource      : LONGINT;
   CopyDestination : LONGINT;
   CopyLength      : LONGINT;
  BEGIN
  { *** try to open file }
   ASSIGN(InFile,FileName);
{$I-}
   RESET(InFile,1);
{$I+}
   IF IORESULT <> $00 THEN
    BEGIN
     EXIT;
    END;
{$I-}
  { *** seek start of module }
   IF FILESIZE(InFile) < FilePosition THEN
    BEGIN
     EXIT;
    END;
   SEEK(InFile,FilePosition);
  { *** look for enough memory for temporary variables }
   IF MEMAVAIL < SIZEOF(TPSM_Header)       + SIZEOF(TPSM_SampleList) +
                 SIZEOF(TPSM_Order)        + SIZEOF(TPSM_ChannelSettings) +
                 SIZEOF(TByteArray)        + SIZEOF(TUnpackedPattern)
   THEN
    BEGIN
     EXIT;
    END;
  { *** init dynamic variables }
   NEW(Header);
   NEW(Sample);
   NEW(Order);
   NEW(ChannelSettings);
   NEW(MultiPurposeBuffer);
   NEW(PatternBuffer);
  { *** read header }
   BLOCKREAD(InFile,Header^,SIZEOF(TPSM_Header));
  { *** test if this is a DSM-file }
   IF NOT ((Header^.PSM_Sign[1] = 'P') AND (Header^.PSM_Sign[2] = 'S')   AND
           (Header^.PSM_Sign[3] = 'M') AND (Header^.PSM_Sign[4] = #254)) THEN
    BEGIN
     ErrorCode := c_NoValidFileFormat;
     CLOSE(InFile);
     EXIT;
    END;
  { *** read order }
   SEEK(InFile,FilePosition + Header^.PSM_OrderPosition);
   BLOCKREAD(InFile,Order^,Header^.PSM_OrderLength);
  { *** read channelsettings }
   SEEK(InFile,FilePosition + Header^.PSM_ChannelSettingPosition);
   BLOCKREAD(InFile,ChannelSettings^,SIZEOF(TPSM_ChannelSettings));
  { *** read samplelist }
   SEEK(InFile,FilePosition + Header^.PSM_SamplePosition);
   BLOCKREAD(InFile,Sample^,Header^.PSM_SampleNumber * SIZEOF(TPSM_Sample));
  { *** copy header to intern NTMIK-structure }
   Module^.Module_Sign                 := 'MF';
   Module^.Module_FileFormatVersion    := $0100;
   Module^.Module_SampleNumber         := Header^.PSM_SampleNumber;
   Module^.Module_PatternNumber        := Header^.PSM_PatternNumber;
   Module^.Module_OrderLength          := Header^.PSM_OrderLength;
   Module^.Module_ChannelNumber        := Header^.PSM_ChannelNumber+1;
   Module^.Module_Initial_GlobalVolume := 64;
   Module^.Module_Initial_MasterVolume := $C0;
   Module^.Module_Initial_Speed        := Header^.PSM_Speed;
   Module^.Module_Initial_Tempo        := Header^.PSM_Tempo;
{ *** paragraph 01 start }
   Module^.Module_Flags                := c_Module_Flags_ZeroVolume        * BYTE(1) +
                                          c_Module_Flags_Stereo            * BYTE(1) +
                                          c_Module_Flags_ForceAmigaLimits  * BYTE(0) +
                                          c_Module_Flags_Panning           * BYTE(1) +
                                          c_Module_Flags_Surround          * BYTE(1) +
                                          c_Module_Flags_QualityMixing     * BYTE(1) +
                                          c_Module_Flags_FastVolumeSlides  * BYTE(0) +
                                          c_Module_Flags_SpecialCustomData * BYTE(0) +
                                          c_Module_Flags_SongName          * BYTE(1);
   I1 := $01;
   WHILE (Header^.PSM_SongName[I1] > #00) AND (I1 < c_Module_SongNameLength) DO
    BEGIN
     Module^.Module_Name[I1] := Header^.PSM_SongName[I1];
     INC(I1);
    END;
   Module^.Module_Name[c_Module_SongNameLength] := #00;
  { *** Init channelsettings }
   FOR I1 := 0 TO c_Maximum_ChannelIndex DO
    BEGIN
     IF I1 < Header^.PSM_ChannelUsed THEN
      BEGIN
      { *** channel enabled }
       Module^.Module_ChannelSettingPointer^[I1].ChannelSettings_GlobalVolume := 64;
       Module^.Module_ChannelSettingPointer^[I1].ChannelSettings_Panning      := (ChannelSettings^[I1]) * $08;
       Module^.Module_ChannelSettingPointer^[I1].ChannelSettings_Code         := I1 + $10 * BYTE(ChannelSettings^[I1] > $08) +
                                             c_ChannelSettings_Code_ChannelEnabled   * BYTE(1) +
                                             c_ChannelSettings_Code_ChannelDigital   * BYTE(1);
       Module^.Module_ChannelSettingPointer^[I1].ChannelSettings_Controls     :=
                                             c_ChannelSettings_Controls_EnhancedMode * BYTE(1) +
                                             c_ChannelSettings_Controls_SurroundMode * BYTE(0);
      END
     ELSE
      BEGIN
      { *** channel disabled }
       Module^.Module_ChannelSettingPointer^[I1].ChannelSettings_GlobalVolume := $00;
       Module^.Module_ChannelSettingPointer^[I1].ChannelSettings_Panning      := $00;
       Module^.Module_ChannelSettingPointer^[I1].ChannelSettings_Code         := $00;
       Module^.Module_ChannelSettingPointer^[I1].ChannelSettings_Controls     := $00;
      END;
    END;
  { *** init and copy order }
   FILLCHAR(Module^.Module_OrderPointer^,c_Maximum_OrderIndex+1,$FF);
   MOVE(Order^,Module^.Module_OrderPointer^,Header^.PSM_OrderLength);
  { *** read pattern }
   SEEK(InFile,FilePosition + Header^.PSM_PatternPosition);
   NTMIK_LoaderPatternNumber := Header^.PSM_PatternNumber-1;
   FOR I1 := 0 TO Header^.PSM_PatternNumber-1 DO
    BEGIN
     NTMIK_LoadPatternProcedure;
    { *** read length }
     BLOCKREAD(InFile,TempW,2);
    { *** read pattern }
     BLOCKREAD(InFile,MultiPurposeBuffer^,TempW-2);
    { *** unpack pattern and set notes per channel to 64 }
     PSM_UnpackPattern(MultiPurposeBuffer^,PatternBuffer^,TempW);
     NTMIK_PackPattern(MultiPurposeBuffer^,PatternBuffer^,PSM_NotesInPattern);
     TempW := WORD(256) * MultiPurposeBuffer^[01] + MultiPurposeBuffer^[00];
     GETMEM(Module^.Module_PatternPointer^[I1],TempW);
     MOVE(MultiPurposeBuffer^,Module^.Module_PatternPointer^[I1]^,TempW);
    { *** next pattern }
    END;
  { *** read samples }
   NTMIK_LoaderSampleNumber := Header^.PSM_SampleNumber;
   FOR I1 := 1 TO Header^.PSM_SampleNumber DO
    BEGIN
     NTMIK_LoadSampleProcedure;
    { *** get index for sample }
     I3 := Sample^[I1].PSM_SampleNumber;
    { *** clip PSM-sample }
     IF Sample^[I1].PSM_SampleLoopEnd > Sample^[I1].PSM_SampleLength
     THEN Sample^[I1].PSM_SampleLoopEnd := Sample^[I1].PSM_SampleLength;
    { *** init intern sample }
     NEW(Module^.Module_SamplePointer^[I3]);
     FILLCHAR(Module^.Module_SamplePointer^[I3]^,SIZEOF(TSample),$00);
     FILLCHAR(Module^.Module_SamplePointer^[I3]^.Sample_SampleName,c_Sample_SampleNameLength,#32);
     FILLCHAR(Module^.Module_SamplePointer^[I3]^.Sample_FileName,c_Sample_FileNameLength,#32);
    { *** copy informations to intern sample }
     I2 := $01;
     WHILE (Sample^[I1].PSM_SampleName[I2] > #00) AND (I2 < c_Sample_SampleNameLength) DO
      BEGIN
       Module^.Module_SamplePointer^[I3]^.Sample_SampleName[I2] := Sample^[I1].PSM_SampleName[I2];
       INC(I2);
      END;
     Module^.Module_SamplePointer^[I3]^.Sample_Sign              := 'DF';
     Module^.Module_SamplePointer^[I3]^.Sample_FileFormatVersion := $00100;
     Module^.Module_SamplePointer^[I3]^.Sample_Position          := $00000000;
     Module^.Module_SamplePointer^[I3]^.Sample_Selector          := $0000;
     Module^.Module_SamplePointer^[I3]^.Sample_Volume            := Sample^[I1].PSM_SampleVolume;
     Module^.Module_SamplePointer^[I3]^.Sample_LoopCounter       := $00;
     Module^.Module_SamplePointer^[I3]^.Sample_C5Speed           := Sample^[I1].PSM_SampleC5Speed;
     Module^.Module_SamplePointer^[I3]^.Sample_Length            := Sample^[I1].PSM_SampleLength;
     Module^.Module_SamplePointer^[I3]^.Sample_LoopBegin         := Sample^[I1].PSM_SampleLoopBegin;
     Module^.Module_SamplePointer^[I3]^.Sample_LoopEnd           := Sample^[I1].PSM_SampleLoopEnd;
    { *** now it's time for the flags }
     Module^.Module_SamplePointer^[I3]^.Sample_Flags :=
                                 c_Sample_Flags_DigitalSample      * BYTE(1) +
                                 c_Sample_Flags_8BitSample         * BYTE(1) +
                                 c_Sample_Flags_UnsignedSampleData * BYTE(1) +
                                 c_Sample_Flags_Packed             * BYTE(0) +
                                 c_Sample_Flags_LoopCounter        * BYTE(0) +
                                 c_Sample_Flags_SampleName         * BYTE(1) +
                                 c_Sample_Flags_LoopActive         *
                             BYTE(Sample^[I1].PSM_SampleFlags AND (LONGINT(1) SHL 15) = (LONGINT(1) SHL 15));
    { *** alloc memory for sample-data }
     E_Getmem(Module^.Module_SamplePointer^[I3]^.Sample_Selector,
              Module^.Module_SamplePointer^[I3]^.Sample_Position,
              Module^.Module_SamplePointer^[I3]^.Sample_Length + c_LoopExtensionSize);
    { *** read out data }
     EPT(TempP).p_Selector := Module^.Module_SamplePointer^[I3]^.Sample_Selector;
     EPT(TempP).p_Offset   := $0000;
     SEEK(InFile,Sample^[I1].PSM_SamplePosition);
     E_BLOCKREAD(InFile,TempP^,Module^.Module_SamplePointer^[I3]^.Sample_Length);
    { *** 'coz the samples are signed in a DSM-file -> PC-fy them }
     IF Module^.Module_SamplePointer^[I3]^.Sample_Length > 4 THEN
      BEGIN
       CopyLength := Module^.Module_SamplePointer^[I3]^.Sample_Length;
      { *** decode sample }
       ASM
        DB 066h; MOV CX,WORD PTR CopyLength
       { *** load sample selector }
                 MOV ES,WORD PTR TempP[00002h]
        DB 066h; XOR SI,SI
        DB 066h; XOR DI,DI
                 XOR AH,AH
       { *** conert all bytes }
                @@MainLoop:
        DB 026h; DB 067h; LODSB
                 ADD AL,AH
                 MOV AH,AL
        DB 067h; STOSB
        DB 066h; LOOP @@MainLoop
       END;
      { *** make samples unsigned }
       ASM
        DB 066h; MOV CX,WORD PTR CopyLength
       { *** load sample selector }
                 MOV ES,WORD PTR TempP[00002h]
        DB 066h; XOR SI,SI
        DB 066h; XOR DI,DI
       { *** conert all bytes }
                @@MainLoop:
        DB 026h; DB 067h; LODSB
                 SUB AL,080h
        DB 067h; STOSB
        DB 066h; LOOP @@MainLoop
       END;
      { *** Create Loop-Extension }
       IF Module^.Module_SamplePointer^[I3]^.Sample_Flags AND c_Sample_Flags_LoopActive = c_Sample_Flags_LoopActive THEN
        BEGIN
         CopySource      := Module^.Module_SamplePointer^[I3]^.Sample_LoopBegin;
         CopyDestination := Module^.Module_SamplePointer^[I3]^.Sample_LoopEnd;
         CopyLength      := CopyDestination - CopySource;
         ASM
         { *** load sample-selector }
                   MOV ES,WORD PTR TempP[00002h]
          DB 066h; MOV DI,WORD PTR CopyDestination
         { *** calculate number of full sample-loops to copy }
                   XOR DX,DX
                   MOV AX,c_LoopExtensionSize
                   MOV BX,WORD PTR CopyLength
                   DIV BX
                   OR AX,AX
                   JE @@NoFullLoop
         { *** copy some full-loops (size=bx) }
                   MOV CX,AX
                  @@InnerLoop:
                   PUSH CX
          DB 066h; MOV SI,WORD PTR CopySource
                   MOV CX,BX
          DB 0F3h; DB 026h,067h,0A4h { REP MOVS BYTE PTR ES:[EDI],ES:[ESI] }
                   POP CX
                   LOOP @@InnerLoop
                  @@NoFullLoop:
         { *** calculate number of rest-bytes to copy }
          DB 066h; MOV SI,WORD PTR CopySource
                   MOV CX,DX
          DB 0F3h; DB 026h,067h,0A4h { REP MOVS BYTE PTR ES:[EDI],ES:[ESI] }
         END;
        END
       ELSE
        BEGIN
         CopyDestination := Module^.Module_SamplePointer^[I3]^.Sample_Length;
         ASM
         { *** load sample-selector }
                   MOV ES,WORD PTR TempP[00002h]
          DB 066h; MOV DI,WORD PTR CopyDestination
         { *** clear extension }
                   MOV CX,c_LoopExtensionSize
                   MOV AL,080h
          DB 0F3h; DB 067h,0AAh       { REP STOS BYTE PTR ES:[EDI] }
         END;
        END;
      END;
    { *** next sample }
    END;
  { *** init period-ranges }
   NTMIK_MaximumPeriod := $0000D600 SHR 1;
   NTMIK_MinimumPeriod := $0000D600 SHR 8;
  { *** close file }
   CLOSE(InFile);
  { *** dispose all dynamic variables }
   DISPOSE(Header);
   DISPOSE(Sample);
   DISPOSE(Order);
   DISPOSE(ChannelSettings);
   DISPOSE(MultiPurposeBuffer);
   DISPOSE(PatternBuffer);
  { *** set errorcode to noerror }
   ErrorCode := c_NoError;
  END;

*/
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (endian and char fixes for PPC)
*/

//////////////////////////////////////////////
// PTM PolyTracker module loader            //
//////////////////////////////////////////////
//#include "stdafx.h"
//#include "sndfile.h"

//#pragma warning(disable:4244)

#pragma pack(1)

typedef struct PTMFILEHEADER
{
	CHAR songname[28];		// name of song, asciiz string
	CHAR eof;				// 26
	BYTE version_lo;		// 03 version of file, currently 0203h
	BYTE version_hi;		// 02
	BYTE reserved1;			// reserved, set to 0
	WORD norders;			// number of orders (0..256)
	WORD nsamples;			// number of instruments (1..255)
	WORD npatterns;			// number of patterns (1..128)
	WORD nchannels;			// number of channels (voices) used (1..32)
	WORD fileflags;			// set to 0
	WORD reserved2;			// reserved, set to 0
	DWORD ptmf_id;			// song identification, 'PTMF' or 0x464d5450
	BYTE reserved3[16];		// reserved, set to 0
	BYTE chnpan[32];		// channel panning settings, 0..15, 0 = left, 7 = middle, 15 = right
	BYTE orders[256];		// order list, valid entries 0..nOrders-1
	WORD patseg[128];		// pattern offsets (*16)
} PTMFILEHEADER, *LPPTMFILEHEADER;

#define SIZEOF_PTMFILEHEADER	608


typedef struct PTMSAMPLE
{
	BYTE sampletype;		// sample type (bit array)
	CHAR filename[12];		// name of external sample file
	BYTE volume;			// default volume
	WORD nC4Spd;			// C4 speed
	WORD sampleseg;			// sample segment (used internally)
	WORD fileofs[2];		// offset of sample data
	WORD length[2];			// sample size (in bytes)
	WORD loopbeg[2];		// start of loop
	WORD loopend[2];		// end of loop
	WORD gusdata[8];
	char  samplename[28];	// name of sample, asciiz  // changed from CHAR
	DWORD ptms_id;			// sample identification, 'PTMS' or 0x534d5450
} PTMSAMPLE;

#define SIZEOF_PTMSAMPLE	80

#pragma pack()


static uint32_t BS2WORD(uint16_t w[2]) {
	uint32_t u32 = (w[1] << 16) + w[0];
	return(bswapLE32(u32));
}

BOOL CSoundFile::ReadPTM(const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
	DWORD dwMemPos;
	UINT nOrders;

	if ((!lpStream) || (dwMemLength < sizeof(PTMFILEHEADER))) return FALSE;
	PTMFILEHEADER pfh = *(LPPTMFILEHEADER)lpStream;

	pfh.norders = bswapLE16(pfh.norders);
	pfh.nsamples = bswapLE16(pfh.nsamples);
	pfh.npatterns = bswapLE16(pfh.npatterns);
	pfh.nchannels = bswapLE16(pfh.nchannels);
	pfh.fileflags = bswapLE16(pfh.fileflags);
	pfh.reserved2 = bswapLE16(pfh.reserved2);
	pfh.ptmf_id = bswapLE32(pfh.ptmf_id);
	for (UINT j=0; j<128; j++)
        {
	        pfh.patseg[j] = bswapLE16(pfh.patseg[j]);
	}

	if ((pfh.ptmf_id != 0x464d5450) || (!pfh.nchannels)
	 || (pfh.nchannels > 32)
	 || (pfh.norders > 256) || (!pfh.norders)
	 || (!pfh.nsamples) || (pfh.nsamples > 255)
	 || (!pfh.npatterns) || (pfh.npatterns > 128)
	 || (SIZEOF_PTMFILEHEADER+pfh.nsamples*SIZEOF_PTMSAMPLE >= (int)dwMemLength)) return FALSE;
	memcpy(m_szNames[0], pfh.songname, 28);
	m_szNames[0][28] = 0;
	m_nType = MOD_TYPE_PTM;
	m_nChannels = pfh.nchannels;
	m_nSamples = (pfh.nsamples < MAX_SAMPLES) ? pfh.nsamples : MAX_SAMPLES-1;
	dwMemPos = SIZEOF_PTMFILEHEADER;
	nOrders = (pfh.norders < MAX_ORDERS) ? pfh.norders : MAX_ORDERS-1;
	memcpy(Order, pfh.orders, nOrders);
	for (UINT ipan=0; ipan<m_nChannels; ipan++)
	{
		ChnSettings[ipan].nVolume = 64;
		ChnSettings[ipan].nPan = ((pfh.chnpan[ipan] & 0x0F) << 4) + 4;
	}
	for (UINT ismp=0; ismp<m_nSamples; ismp++, dwMemPos += SIZEOF_PTMSAMPLE)
	{
		MODINSTRUMENT *pins = &Ins[ismp+1];
		PTMSAMPLE *psmp = (PTMSAMPLE *)(lpStream+dwMemPos);

		lstrcpyn(m_szNames[ismp+1], psmp->samplename, 28);
		memcpy(pins->name, psmp->filename, 12);
		pins->name[12] = 0;
		pins->nGlobalVol = 64;
		pins->nPan = 128;
		pins->nVolume = psmp->volume << 2;
		pins->nC4Speed = bswapLE16(psmp->nC4Spd) << 1;
		pins->uFlags = 0;
		if ((psmp->sampletype & 3) == 1)
		{
			UINT smpflg = RS_PCM8D;
			pins->nLength = BS2WORD(psmp->length);
			pins->nLoopStart = BS2WORD(psmp->loopbeg);
			pins->nLoopEnd = BS2WORD(psmp->loopend);
			DWORD samplepos = BS2WORD(psmp->fileofs);
			if (psmp->sampletype & 4) pins->uFlags |= CHN_LOOP;
			if (psmp->sampletype & 8) pins->uFlags |= CHN_PINGPONGLOOP;
			if (psmp->sampletype & 16)
			{
				pins->uFlags |= CHN_16BIT;
				pins->nLength >>= 1;
				pins->nLoopStart >>= 1;
				pins->nLoopEnd >>= 1;
				smpflg = RS_PTM8DTO16;
			}
			if ((pins->nLength) && (samplepos) && (samplepos < dwMemLength))
			{
				ReadSample(pins, smpflg, (LPSTR)(lpStream+samplepos), dwMemLength-samplepos);
			}
		}
	}
	// Reading Patterns
	for (UINT ipat=0; ipat<pfh.npatterns; ipat++)
	{
		dwMemPos = ((UINT)pfh.patseg[ipat]) << 4;
		if ((!dwMemPos) || (dwMemPos >= dwMemLength)) continue;
		PatternSize[ipat] = 64;
		if ((Patterns[ipat] = AllocatePattern(64, m_nChannels)) == NULL) break;
		//
		MODCOMMAND *m = Patterns[ipat];
		for (UINT row=0; ((row < 64) && (dwMemPos < dwMemLength)); )
		{
			UINT b = lpStream[dwMemPos++];

			if (dwMemPos >= dwMemLength) break;
			if (b)
			{
				UINT nChn = b & 0x1F;
				MODCOMMAND &selm = m[nChn < m_nChannels ? nChn : 0];

				if (b & 0x20)
				{
					if (dwMemPos + 2 > dwMemLength) break;
					selm.note = lpStream[dwMemPos++];
					selm.instr = lpStream[dwMemPos++];
				}
				if (b & 0x40)
				{
					if (dwMemPos + 2 > dwMemLength) break;
					selm.command = lpStream[dwMemPos++];
					selm.param = lpStream[dwMemPos++];
					if ((selm.command == 0x0E) && ((selm.param & 0xF0) == 0x80))
					{
						selm.command = CMD_S3MCMDEX;
					} else
					if (selm.command < 0x10)
					{
						ConvertModCommand(&selm);
					} else
					{
						switch(selm.command)
						{
						case 16:
							selm.command = CMD_GLOBALVOLUME;
							break;
						case 17:
							selm.command = CMD_RETRIG;
							break;
						case 18:
							selm.command = CMD_FINEVIBRATO;
							break;
						default:
							selm.command = 0;
						}
					}
				}
				if (b & 0x80)
				{
					if (dwMemPos >= dwMemLength) break;
					selm.volcmd = VOLCMD_VOLUME;
					selm.vol = lpStream[dwMemPos++];
				}
			} else
			{
				row++;
				m += m_nChannels;
			}
		}
	}
	return TRUE;
}
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (endian and char fixes for PPC)
*/

//#include "stdafx.h"
//#include "sndfile.h"
//#include "tables.h"

#ifdef _MSC_VER
//#pragma warning(disable:4244)
#endif

//////////////////////////////////////////////////////
// ScreamTracker S3M file support

#pragma pack(1)
typedef struct tagS3MSAMPLESTRUCT
{
	BYTE type;
	CHAR dosname[12];
	BYTE hmem;
	WORD memseg;
	DWORD length;
	DWORD loopbegin;
	DWORD loopend;
	BYTE vol;
	BYTE bReserved;
	BYTE pack;
	BYTE flags;
	DWORD finetune;
	DWORD dwReserved;
	WORD intgp;
	WORD int512;
	DWORD lastused;
	CHAR name[28];
	CHAR scrs[4];
} S3MSAMPLESTRUCT;


typedef struct tagS3MFILEHEADER
{
	CHAR name[28];
	BYTE b1A;
	BYTE type;
	WORD reserved1;
	WORD ordnum;
	WORD insnum;
	WORD patnum;
	WORD flags;
	WORD cwtv;
	WORD version;
	DWORD scrm;	// "SCRM" = 0x4D524353
	BYTE globalvol;
	BYTE speed;
	BYTE tempo;
	BYTE mastervol;
	BYTE ultraclicks;
	BYTE panning_present;
	BYTE reserved2[8];
	WORD special;
	BYTE channels[32];
} S3MFILEHEADER;
#pragma pack()


void CSoundFile::S3MConvert(MODCOMMAND *m, BOOL bIT) const
//--------------------------------------------------------
{
	UINT command = m->command;
	UINT param = m->param;
	switch (command + 0x40)
	{
	case 'A':	command = CMD_SPEED; break;
	case 'B':	command = CMD_POSITIONJUMP; break;
	case 'C':	command = CMD_PATTERNBREAK; if (!bIT) param = (param >> 4) * 10 + (param & 0x0F); break;
	case 'D':	command = CMD_VOLUMESLIDE; break;
	case 'E':	command = CMD_PORTAMENTODOWN; break;
	case 'F':	command = CMD_PORTAMENTOUP; break;
	case 'G':	command = CMD_TONEPORTAMENTO; break;
	case 'H':	command = CMD_VIBRATO; break;
	case 'I':	command = CMD_TREMOR; break;
	case 'J':	command = CMD_ARPEGGIO; break;
	case 'K':	command = CMD_VIBRATOVOL; break;
	case 'L':	command = CMD_TONEPORTAVOL; break;
	case 'M':	command = CMD_CHANNELVOLUME; break;
	case 'N':	command = CMD_CHANNELVOLSLIDE; break;
	case 'O':	command = CMD_OFFSET; break;
	case 'P':	command = CMD_PANNINGSLIDE; break;
	case 'Q':	command = CMD_RETRIG; break;
	case 'R':	command = CMD_TREMOLO; break;
	case 'S':	command = CMD_S3MCMDEX; break;
	case 'T':	command = CMD_TEMPO; break;
	case 'U':	command = CMD_FINEVIBRATO; break;
	case 'V':	command = CMD_GLOBALVOLUME; break;
	case 'W':	command = CMD_GLOBALVOLSLIDE; break;
	case 'X':	command = CMD_PANNING8; break;
	case 'Y':	command = CMD_PANBRELLO; break;
	case 'Z':	command = CMD_MIDI; break;
	case '\\':  command = CMD_MIDI; break;
	default:	command = 0;
	}
	m->command = command;
	m->param = param;
}


void CSoundFile::S3MSaveConvert(UINT *pcmd, UINT *pprm, BOOL bIT) const
//---------------------------------------------------------------------
{
	UINT command = *pcmd;
	UINT param = *pprm;
	switch(command)
	{
	case CMD_SPEED:				command = 'A'; break;
	case CMD_POSITIONJUMP:		command = 'B'; break;
	case CMD_PATTERNBREAK:		command = 'C'; if (!bIT) param = ((param / 10) << 4) + (param % 10); break;
	case CMD_VOLUMESLIDE:		command = 'D'; break;
	case CMD_PORTAMENTODOWN:	command = 'E'; if ((param >= 0xE0) && (m_nType & (MOD_TYPE_MOD|MOD_TYPE_XM))) param = 0xDF; break;
	case CMD_PORTAMENTOUP:		command = 'F'; if ((param >= 0xE0) && (m_nType & (MOD_TYPE_MOD|MOD_TYPE_XM))) param = 0xDF; break;
	case CMD_TONEPORTAMENTO:	command = 'G'; break;
	case CMD_VIBRATO:			command = 'H'; break;
	case CMD_TREMOR:			command = 'I'; break;
	case CMD_ARPEGGIO:			command = 'J'; break;
	case CMD_VIBRATOVOL:		command = 'K'; break;
	case CMD_TONEPORTAVOL:		command = 'L'; break;
	case CMD_CHANNELVOLUME:		command = 'M'; break;
	case CMD_CHANNELVOLSLIDE:	command = 'N'; break;
	case CMD_OFFSET:			command = 'O'; break;
	case CMD_PANNINGSLIDE:		command = 'P'; break;
	case CMD_RETRIG:			command = 'Q'; break;
	case CMD_TREMOLO:			command = 'R'; break;
	case CMD_S3MCMDEX:			command = 'S'; break;
	case CMD_TEMPO:				command = 'T'; break;
	case CMD_FINEVIBRATO:		command = 'U'; break;
	case CMD_GLOBALVOLUME:		command = 'V'; break;
	case CMD_GLOBALVOLSLIDE:	command = 'W'; break;
	case CMD_PANNING8:
		command = 'X';
		if ((bIT) && (m_nType != MOD_TYPE_IT) && (m_nType != MOD_TYPE_XM))
		{
			if (param == 0xA4) { command = 'S'; param = 0x91; }	else
			if (param <= 0x80) { param <<= 1; if (param > 255) param = 255; } else
			command = param = 0;
		} else
		if ((!bIT) && ((m_nType == MOD_TYPE_IT) || (m_nType == MOD_TYPE_XM)))
		{
			param >>= 1;
		}
		break;
	case CMD_PANBRELLO:			command = 'Y'; break;
	case CMD_MIDI:				command = 'Z'; break;
	case CMD_XFINEPORTAUPDOWN:
		if (param & 0x0F) switch(param & 0xF0)
		{
		case 0x10:	command = 'F'; param = (param & 0x0F) | 0xE0; break;
		case 0x20:	command = 'E'; param = (param & 0x0F) | 0xE0; break;
		case 0x90:	command = 'S'; break;
		default:	command = param = 0;
		} else command = param = 0;
		break;
	case CMD_MODCMDEX:
		command = 'S';
		switch(param & 0xF0)
		{
		case 0x00:	command = param = 0; break;
		case 0x10:	command = 'F'; param |= 0xF0; break;
		case 0x20:	command = 'E'; param |= 0xF0; break;
		case 0x30:	param = (param & 0x0F) | 0x10; break;
		case 0x40:	param = (param & 0x0F) | 0x30; break;
		case 0x50:	param = (param & 0x0F) | 0x20; break;
		case 0x60:	param = (param & 0x0F) | 0xB0; break;
		case 0x70:	param = (param & 0x0F) | 0x40; break;
		case 0x90:	command = 'Q'; param &= 0x0F; break;
		case 0xA0:	if (param & 0x0F) { command = 'D'; param = (param << 4) | 0x0F; } else command=param=0; break;
		case 0xB0:	if (param & 0x0F) { command = 'D'; param |= 0xF0; } else command=param=0; break;
		}
		break;
	default:	command = param = 0;
	}
	command &= ~0x40;
	*pcmd = command;
	*pprm = param;
}

static DWORD boundInput(DWORD input, DWORD smin, DWORD smax)
{
	if (input > smax) input = smax;
	else if (input < smin) input = 0;
	return(input);
}


BOOL CSoundFile::ReadS3M(const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
	UINT insnum,patnum,nins,npat;
	DWORD insfile[MAX_SAMPLES];
	WORD ptr[256];
	DWORD dwMemPos;
	BYTE insflags[MAX_SAMPLES], inspack[MAX_SAMPLES];

	if ((!lpStream) || (dwMemLength <= sizeof(S3MFILEHEADER)+sizeof(S3MSAMPLESTRUCT)+64)) return FALSE;
	S3MFILEHEADER psfh = *(S3MFILEHEADER *)lpStream;

	psfh.reserved1 = bswapLE16(psfh.reserved1);
	psfh.ordnum = bswapLE16(psfh.ordnum);
	psfh.insnum = bswapLE16(psfh.insnum);
	psfh.patnum = bswapLE16(psfh.patnum);
	psfh.flags = bswapLE16(psfh.flags);
	psfh.cwtv = bswapLE16(psfh.cwtv);
	psfh.version = bswapLE16(psfh.version);
	psfh.scrm = bswapLE32(psfh.scrm);
	psfh.special = bswapLE16(psfh.special);

	if (psfh.scrm != 0x4D524353) return FALSE;
	dwMemPos = 0x60;
	m_nType = MOD_TYPE_S3M;
	memset(m_szNames,0,sizeof(m_szNames));
	memcpy(m_szNames[0], psfh.name, 28);
	// Speed
	m_nDefaultSpeed = psfh.speed;
	if (m_nDefaultSpeed < 1) m_nDefaultSpeed = 6;
	if (m_nDefaultSpeed > 0x1F) m_nDefaultSpeed = 0x1F;
	// Tempo
	m_nDefaultTempo = psfh.tempo;
	if (m_nDefaultTempo < 40) m_nDefaultTempo = 40;
	if (m_nDefaultTempo > 240) m_nDefaultTempo = 240;
	// Global Volume
	m_nDefaultGlobalVolume = psfh.globalvol << 2;
	if ((!m_nDefaultGlobalVolume) || (m_nDefaultGlobalVolume > 256)) m_nDefaultGlobalVolume = 256;
	m_nSongPreAmp = psfh.mastervol & 0x7F;
	// Channels
	m_nChannels = 4;
	for (UINT ich=0; ich<32; ich++)
	{
		ChnSettings[ich].nPan = 128;
		ChnSettings[ich].nVolume = 64;

		ChnSettings[ich].dwFlags = CHN_MUTE;
		if (psfh.channels[ich] != 0xFF)
		{
			m_nChannels = ich+1;
			UINT b = psfh.channels[ich] & 0x0F;
			ChnSettings[ich].nPan = (b & 8) ? 0xC0 : 0x40;
			ChnSettings[ich].dwFlags = 0;
		}
	}
	if (m_nChannels < 4) m_nChannels = 4;
	if ((psfh.cwtv < 0x1320) || (psfh.flags & 0x40)) m_dwSongFlags |= SONG_FASTVOLSLIDES;
	// Reading pattern order
	UINT iord = psfh.ordnum;
	if (iord<1) iord = 1;
	if (iord > MAX_ORDERS) iord = MAX_ORDERS;
	if (dwMemPos + iord + 1 >= dwMemLength) return FALSE;
	if (iord)
	{
		if (dwMemPos + iord > dwMemLength) return FALSE;
		memcpy(Order, lpStream+dwMemPos, iord);
		dwMemPos += iord;
	}
	if ((iord & 1) && (lpStream[dwMemPos] == 0xFF)) dwMemPos++;
	// Reading file pointers
	insnum = nins = psfh.insnum;
	if (insnum >= MAX_SAMPLES) insnum = MAX_SAMPLES-1;
	m_nSamples = insnum;
	patnum = npat = psfh.patnum;
	if (patnum > MAX_PATTERNS) patnum = MAX_PATTERNS;
	memset(ptr, 0, sizeof(ptr));

	// Ignore file if it has a corrupted header.
	if (nins+npat > 256) return FALSE;

	if (nins+npat)
	{
		if (dwMemPos + 2*(nins+npat) >= dwMemLength) return FALSE;
		memcpy(ptr, lpStream+dwMemPos, 2*(nins+npat));
		dwMemPos += 2*(nins+npat);
		for (UINT j = 0; j < (nins+npat); ++j) {
		        ptr[j] = bswapLE16(ptr[j]);
		}
		if (psfh.panning_present == 252)
		{
			const BYTE *chnpan = lpStream+dwMemPos;
			if (dwMemPos > dwMemLength - 32) return FALSE;
			for (UINT i=0; i<32; i++) if (chnpan[i] & 0x20)
			{
				ChnSettings[i].nPan = ((chnpan[i] & 0x0F) << 4) + 8;
			}
		}
	}
	if (!m_nChannels) return TRUE;
	// Reading instrument headers
	memset(insfile, 0, sizeof(insfile));
	for (UINT iSmp=1; iSmp<=insnum; iSmp++)
	{
		UINT nInd = ((DWORD)ptr[iSmp-1])*16;
		if ((!nInd) || (nInd + 0x50 > dwMemLength)) {
			// initialize basic variables.
			insflags[iSmp-1] = 0;
			inspack[iSmp-1] = 0;
			continue;
		}
		S3MSAMPLESTRUCT pSmp;
		memcpy(&pSmp, lpStream+nInd, 0x50);
		memcpy(Ins[iSmp].name, pSmp.dosname, 12);
		insflags[iSmp-1] = pSmp.flags;
		inspack[iSmp-1] = pSmp.pack;
		memcpy(m_szNames[iSmp], pSmp.name, 28);
		m_szNames[iSmp][28] = 0;
		if ((pSmp.type==1) && (pSmp.scrs[2]=='R') && (pSmp.scrs[3]=='S'))
		{
			Ins[iSmp].nLength = boundInput(bswapLE32(pSmp.length), 4, MAX_SAMPLE_LENGTH);
			Ins[iSmp].nLoopStart = boundInput(bswapLE32(pSmp.loopbegin), 4, Ins[iSmp].nLength - 1);
			Ins[iSmp].nLoopEnd = boundInput(bswapLE32(pSmp.loopend), 4, Ins[iSmp].nLength);
			Ins[iSmp].nVolume = boundInput(pSmp.vol, 0, 64) << 2;
			Ins[iSmp].nGlobalVol = 64;
			if (pSmp.flags&1) Ins[iSmp].uFlags |= CHN_LOOP;
			UINT j = bswapLE32(pSmp.finetune);
			if (!j) j = 8363;
			if (j < 1024) j = 1024;
			Ins[iSmp].nC4Speed = j;
			insfile[iSmp] = (pSmp.hmem << 20) + (bswapLE16(pSmp.memseg) << 4);
			// offset is invalid - ignore this sample.
			if (insfile[iSmp] > dwMemLength) insfile[iSmp] = 0;
			else if (insfile[iSmp]) {
				// ignore duplicate samples.
				for (int z=iSmp-1; z>=0; z--)
					if (insfile[iSmp] == insfile[z])
						insfile[iSmp] = 0;
			}
			if ((Ins[iSmp].nLoopStart >= Ins[iSmp].nLoopEnd) || (Ins[iSmp].nLoopEnd - Ins[iSmp].nLoopStart < 8))
				Ins[iSmp].nLoopStart = Ins[iSmp].nLoopEnd = 0;
			Ins[iSmp].nPan = 0x80;
		}
	}
	// Reading patterns
	for (UINT iPat=0; iPat<patnum; iPat++)
	{
		UINT nInd = ((DWORD)ptr[nins+iPat]) << 4;
		if (nInd + 0x40 > dwMemLength) continue;
		WORD len = bswapLE16(*((WORD *)(lpStream+nInd)));
		nInd += 2;
		PatternSize[iPat] = 64;
		if ((!len) || (nInd + len > dwMemLength - 6)
		 || ((Patterns[iPat] = AllocatePattern(64, m_nChannels)) == NULL)) continue;
		LPBYTE src = (LPBYTE)(lpStream+nInd);
		// Unpacking pattern
		MODCOMMAND *p = Patterns[iPat];
		UINT row = 0;
		UINT j = 0;
		while (j < len)
		{
			BYTE b = src[j++];
			if (!b)
			{
				if (++row >= 64) break;
			} else
			{
				UINT chn = b & 0x1F;
				if (chn < m_nChannels)
				{
					MODCOMMAND *m = &p[row*m_nChannels+chn];
					if (b & 0x20)
					{
						m->note = src[j++];
						if (m->note < 0xF0) m->note = (m->note & 0x0F) + 12*(m->note >> 4) + 13;
						else if (m->note == 0xFF) m->note = 0;
						m->instr = src[j++];
					}
					if (b & 0x40)
					{
						UINT vol = src[j++];
						if ((vol >= 128) && (vol <= 192))
						{
							vol -= 128;
							m->volcmd = VOLCMD_PANNING;
						} else
						{
							if (vol > 64) vol = 64;
							m->volcmd = VOLCMD_VOLUME;
						}
						m->vol = vol;
					}
					if (b & 0x80)
					{
						m->command = src[j++];
						m->param = src[j++];
						if (m->command) S3MConvert(m, FALSE);
					}
				} else
				{
					if (b & 0x20) j += 2;
					if (b & 0x40) j++;
					if (b & 0x80) j += 2;
				}
				if (j >= len) break;
			}
		}
	}
	// Reading samples
	for (UINT iRaw=1; iRaw<=insnum; iRaw++) if ((Ins[iRaw].nLength) && (insfile[iRaw]))
	{
		UINT flags = (psfh.version == 1) ? RS_PCM8S : RS_PCM8U;
		if (insflags[iRaw-1] & 4) flags += 5;
		if (insflags[iRaw-1] & 2) flags |= RSF_STEREO;
		if (inspack[iRaw-1] == 4) flags = RS_ADPCM4;
		dwMemPos = insfile[iRaw];
		if (dwMemPos < dwMemLength)
			ReadSample(&Ins[iRaw], flags, (LPSTR)(lpStream + dwMemPos), dwMemLength - dwMemPos);
	}
	m_nMinPeriod = 64;
	m_nMaxPeriod = 32767;
	if (psfh.flags & 0x10) m_dwSongFlags |= SONG_AMIGALIMITS;
	return TRUE;
}


#ifndef MODPLUG_NO_FILESAVE

#ifdef _MSC_VER
#pragma warning(disable:4100)
#endif

static BYTE S3MFiller[16] =
{
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
};


BOOL CSoundFile::SaveS3M(LPCSTR lpszFileName, UINT nPacking)
//----------------------------------------------------------
{
	FILE *f;
	BYTE header[0x60];
	UINT nbo,nbi,nbp,i;
	WORD patptr[128];
	WORD insptr[128];
	BYTE buffer[5*1024];
	S3MSAMPLESTRUCT insex[128];

	if ((!m_nChannels) || (!lpszFileName)) return FALSE;
	if ((f = fopen(lpszFileName, "wb")) == NULL) return FALSE;
	// Writing S3M header
	memset(header, 0, sizeof(header));
	memset(insex, 0, sizeof(insex));
	memcpy(header, m_szNames[0], 0x1C);
	header[0x1B] = 0;
	header[0x1C] = 0x1A;
	header[0x1D] = 0x10;
	nbo = (GetNumPatterns() + 15) & 0xF0;
	if (!nbo) nbo = 16;
	header[0x20] = nbo & 0xFF;
	header[0x21] = nbo >> 8;
	nbi = m_nInstruments;
	if (!nbi) nbi = m_nSamples;
	if (nbi > 99) nbi = 99;
	header[0x22] = nbi & 0xFF;
	header[0x23] = nbi >> 8;
	nbp = 0;
	for (i=0; Patterns[i]; i++) { nbp = i+1; if (nbp >= MAX_PATTERNS) break; }
	for (i=0; i<MAX_ORDERS; i++) if ((Order[i] < MAX_PATTERNS) && (Order[i] >= nbp)) nbp = Order[i] + 1;
	header[0x24] = nbp & 0xFF;
	header[0x25] = nbp >> 8;
	if (m_dwSongFlags & SONG_FASTVOLSLIDES) header[0x26] |= 0x40;
	if ((m_nMaxPeriod < 20000) || (m_dwSongFlags & SONG_AMIGALIMITS)) header[0x26] |= 0x10;
	header[0x28] = 0x20;
	header[0x29] = 0x13;
	header[0x2A] = 0x02; // Version = 1 => Signed samples
	header[0x2B] = 0x00;
	header[0x2C] = 'S';
	header[0x2D] = 'C';
	header[0x2E] = 'R';
	header[0x2F] = 'M';
	header[0x30] = m_nDefaultGlobalVolume >> 2;
	header[0x31] = m_nDefaultSpeed;
	header[0x32] = m_nDefaultTempo;
	header[0x33] = ((m_nSongPreAmp < 0x20) ? 0x20 : m_nSongPreAmp) | 0x80;	// Stereo
	header[0x35] = 0xFC;
	for (i=0; i<32; i++)
	{
		if (i < m_nChannels)
		{
			UINT tmp = (i & 0x0F) >> 1;
			header[0x40+i] = (i & 0x10) | ((i & 1) ? 8+tmp : tmp);
		} else header[0x40+i] = 0xFF;
	}
	fwrite(header, 0x60, 1, f);
	fwrite(Order, nbo, 1, f);
	memset(patptr, 0, sizeof(patptr));
	memset(insptr, 0, sizeof(insptr));
	UINT ofs0 = 0x60 + nbo;
	UINT ofs1 = ((0x60 + nbo + nbi*2 + nbp*2 + 15) & 0xFFF0) + 0x20;
	UINT ofs = ofs1;

	for (i=0; i<nbi; i++) insptr[i] = (WORD)((ofs + i*0x50) / 16);
	for (i=0; i<nbp; i++) patptr[i] = (WORD)((ofs + nbi*0x50) / 16);
	fwrite(insptr, nbi, 2, f);
	fwrite(patptr, nbp, 2, f);
	if (header[0x35] == 0xFC)
	{
		BYTE chnpan[32];
		for (i=0; i<32; i++)
		{
			chnpan[i] = 0x20 | (ChnSettings[i].nPan >> 4);
		}
		fwrite(chnpan, 0x20, 1, f);
	}
	if ((nbi*2+nbp*2) & 0x0F)
	{
		fwrite(S3MFiller, 0x10 - ((nbi*2+nbp*2) & 0x0F), 1, f);
	}
	ofs1 = ftell(f);
	fwrite(insex, nbi, 0x50, f);
	// Packing patterns
	ofs += nbi*0x50;
	for (i=0; i<nbp; i++)
	{
		WORD len = 64;
		memset(buffer, 0, sizeof(buffer));
		patptr[i] = ofs / 16;
		if (Patterns[i])
		{
			len = 2;
			MODCOMMAND *p = Patterns[i];
			for (int row=0; row<64; row++) if (row < PatternSize[i])
			{
				for (UINT j=0; j<m_nChannels; j++)
				{
					UINT b = j;
					MODCOMMAND *m = &p[row*m_nChannels+j];
					UINT note = m->note;
					UINT volcmd = m->volcmd;
					UINT vol = m->vol;
					UINT command = m->command;
					UINT param = m->param;

					if ((note) || (m->instr)) b |= 0x20;
					if (!note) note = 0xFF; else
					if (note >= 0xFE) note = 0xFE; else
					if (note < 13) note = 0; else note -= 13;
					if (note < 0xFE) note = (note % 12) + ((note / 12) << 4);
					if (command == CMD_VOLUME)
					{
						command = 0;
						if (param > 64) param = 64;
						volcmd = VOLCMD_VOLUME;
						vol = param;
					}
					if (volcmd == VOLCMD_VOLUME) b |= 0x40; else
					if (volcmd == VOLCMD_PANNING) { vol |= 0x80; b |= 0x40; }
					if (command)
					{
						S3MSaveConvert(&command, &param, FALSE);
						if (command) b |= 0x80;
					}
					if (b & 0xE0)
					{
						buffer[len++] = b;
						if (b & 0x20)
						{
							buffer[len++] = note;
							buffer[len++] = m->instr;
						}
						if (b & 0x40)
						{
							buffer[len++] = vol;
						}
						if (b & 0x80)
						{
							buffer[len++] = command;
							buffer[len++] = param;
						}
						if (len > sizeof(buffer) - 20) break;
					}
				}
				buffer[len++] = 0;
				if (len > sizeof(buffer) - 20) break;
			}
		}
		buffer[0] = (len - 2) & 0xFF;
		buffer[1] = (len - 2) >> 8;
		len = (len+15) & (~0x0F);
		fwrite(buffer, len, 1, f);
		ofs += len;
	}
	// Writing samples
	for (i=1; i<=nbi; i++)
	{
		MODINSTRUMENT *pins = &Ins[i];
		if (m_nInstruments)
		{
			pins = Ins;
			if (Headers[i])
			{
				for (UINT j=0; j<128; j++)
				{
					UINT n = Headers[i]->Keyboard[j];
					if ((n) && (n < MAX_INSTRUMENTS))
					{
						pins = &Ins[n];
						break;
					}
				}
			}
		}
		memcpy(insex[i-1].dosname, pins->name, 12);
		memcpy(insex[i-1].name, m_szNames[i], 28);
		memcpy(insex[i-1].scrs, "SCRS", 4);
		insex[i-1].hmem = (BYTE)((DWORD)ofs >> 20);
		insex[i-1].memseg = (WORD)((DWORD)ofs >> 4);
		if (pins->pSample)
		{
			insex[i-1].type = 1;
			insex[i-1].length = pins->nLength;
			insex[i-1].loopbegin = pins->nLoopStart;
			insex[i-1].loopend = pins->nLoopEnd;
			insex[i-1].vol = pins->nVolume / 4;
			insex[i-1].flags = (pins->uFlags & CHN_LOOP) ? 1 : 0;
			if (pins->nC4Speed)
				insex[i-1].finetune = pins->nC4Speed;
			else
				insex[i-1].finetune = TransposeToFrequency(pins->RelativeTone, pins->nFineTune);
			UINT flags = RS_PCM8U;
#ifndef NO_PACKING
			if (nPacking)
			{
				if ((!(pins->uFlags & (CHN_16BIT|CHN_STEREO)))
				 && (CanPackSample((char *)pins->pSample, pins->nLength, nPacking)))
				{
					insex[i-1].pack = 4;
					flags = RS_ADPCM4;
				}
			} else
#endif // NO_PACKING
			{
				if (pins->uFlags & CHN_16BIT)
				{
					insex[i-1].flags |= 4;
					flags = RS_PCM16U;
				}
				if (pins->uFlags & CHN_STEREO)
				{
					insex[i-1].flags |= 2;
					flags = (pins->uFlags & CHN_16BIT) ? RS_STPCM16U : RS_STPCM8U;
				}
			}
			DWORD len = WriteSample(f, pins, flags);
			if (len & 0x0F)
			{
				fwrite(S3MFiller, 0x10 - (len & 0x0F), 1, f);
			}
			ofs += (len + 15) & (~0x0F);
		} else
		{
			insex[i-1].length = 0;
		}
	}
	// Updating parapointers
	fseek(f, ofs0, SEEK_SET);
	fwrite(insptr, nbi, 2, f);
	fwrite(patptr, nbp, 2, f);
	fseek(f, ofs1, SEEK_SET);
	fwrite(insex, 0x50, nbi, f);
	fclose(f);
	return TRUE;
}

#ifdef _MSC_VER
#pragma warning(default:4100)
#endif

#endif // MODPLUG_NO_FILESAVE
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

//#include "stdafx.h"
//#include "sndfile.h"

//#pragma warning(disable:4244)

#pragma pack(1)

typedef struct tagSTMNOTE
{
	BYTE note;
	BYTE insvol;
	BYTE volcmd;
	BYTE cmdinf;
} STMNOTE;


// Raw STM sampleinfo struct:
typedef struct tagSTMSAMPLE
{
	CHAR filename[14];	// Can't have long comments - just filename comments :)
	WORD reserved;		// ISA in memory when in ST 2
	WORD length;		// Sample length
	WORD loopbeg;		// Loop start point
	WORD loopend;		// Loop end point
	BYTE volume;		// Volume
	BYTE reserved2;		// More reserved crap
	WORD c2spd;			// Good old c2spd
	BYTE reserved3[6];	// Yet more of PSi's reserved crap
} STMSAMPLE;


// Raw STM header struct:
typedef struct tagSTMHEADER
{
	char songname[20];      // changed from CHAR
	char trackername[8];	// !SCREAM! for ST 2.xx  // changed from CHAR
	CHAR unused;			// 0x1A
	CHAR filetype;			// 1=song, 2=module (only 2 is supported, of course) :)
	CHAR ver_major;			// Like 2
	CHAR ver_minor;			// "ditto"
	BYTE inittempo;			// initspeed= stm inittempo>>4
	BYTE numpat;			// number of patterns
	BYTE globalvol;			// <- WoW! a RiGHT TRiANGLE =8*)
	BYTE reserved[13];		// More of PSi's internal crap
	STMSAMPLE sample[31];	// STM sample data
	BYTE patorder[128];		// Docs say 64 - actually 128
} STMHEADER;

#pragma pack()



BOOL CSoundFile::ReadSTM(const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
	const STMHEADER *phdr = (STMHEADER *)lpStream;
	DWORD dwMemPos = 0;

	if ((!lpStream) || (dwMemLength < sizeof(STMHEADER))) return FALSE;
	if ((phdr->filetype != 2) || (phdr->unused != 0x1A)
	 || ((strncmp(phdr->trackername, "!Scream!", 8))
	  && (strncmp(phdr->trackername, "BMOD2STM", 8)))) return FALSE;
	memcpy(m_szNames[0], phdr->songname, 20);
	// Read STM header
	m_nType = MOD_TYPE_STM;
	m_nSamples = 31;
	m_nChannels = 4;
	m_nInstruments = 0;
	m_nMinPeriod = 64;
	m_nMaxPeriod = 0x7FFF;
	m_nDefaultSpeed = phdr->inittempo >> 4;
	if (m_nDefaultSpeed < 1) m_nDefaultSpeed = 1;
	m_nDefaultTempo = 125;
	m_nDefaultGlobalVolume = phdr->globalvol << 2;
	if (m_nDefaultGlobalVolume > 256) m_nDefaultGlobalVolume = 256;
	memcpy(Order, phdr->patorder, 128);
	// Setting up channels
	for (UINT nSet=0; nSet<4; nSet++)
	{
		ChnSettings[nSet].dwFlags = 0;
		ChnSettings[nSet].nVolume = 64;
		ChnSettings[nSet].nPan = (nSet & 1) ? 0x40 : 0xC0;
	}
	// Reading samples
	for (UINT nIns=0; nIns<31; nIns++)
	{
		MODINSTRUMENT *pIns = &Ins[nIns+1];
		const STMSAMPLE *pStm = &phdr->sample[nIns];  // STM sample data
		memcpy(pIns->name, pStm->filename, 13);
		memcpy(m_szNames[nIns+1], pStm->filename, 12);
		pIns->nC4Speed = bswapLE16(pStm->c2spd);
		pIns->nGlobalVol = 64;
		pIns->nVolume = pStm->volume << 2;
		if (pIns->nVolume > 256) pIns->nVolume = 256;
		pIns->nLength = bswapLE16(pStm->length);
		if ((pIns->nLength < 4) || (!pIns->nVolume)) pIns->nLength = 0;
		pIns->nLoopStart = bswapLE16(pStm->loopbeg);
		pIns->nLoopEnd = bswapLE16(pStm->loopend);
		if ((pIns->nLoopEnd > pIns->nLoopStart) && (pIns->nLoopEnd != 0xFFFF)) pIns->uFlags |= CHN_LOOP;
	}
	dwMemPos = sizeof(STMHEADER);
	for (UINT nOrd=0; nOrd<MAX_ORDERS; nOrd++) if (Order[nOrd] >= 99) Order[nOrd] = 0xFF;
	UINT nPatterns = phdr->numpat;
	if (nPatterns > MAX_PATTERNS) nPatterns = MAX_PATTERNS;
	for (UINT nPat=0; nPat<nPatterns; nPat++)
	{
		if (dwMemPos + 64*4*4 > dwMemLength) return TRUE;
		PatternSize[nPat] = 64;
		if ((Patterns[nPat] = AllocatePattern(64, m_nChannels)) == NULL) return TRUE;
		MODCOMMAND *m = Patterns[nPat];
		const STMNOTE *p = (const STMNOTE *)(lpStream + dwMemPos);
		for (UINT n=0; n<64*4; n++, p++, m++)
		{
			UINT note,ins,vol,cmd;
			// extract the various information from the 4 bytes that
			// make up a single note
			note = p->note;
			ins = p->insvol >> 3;
			vol = (p->insvol & 0x07) + (p->volcmd >> 1);
			cmd = p->volcmd & 0x0F;
			if ((ins) && (ins < 32)) m->instr = ins;
			// special values of [SBYTE0] are handled here ->
			// we have no idea if these strange values will ever be encountered
			// but it appears as though stms sound correct.
			if ((note == 0xFE) || (note == 0xFC)) m->note = 0xFE; else
			// if note < 251, then all three bytes are stored in the file
			if (note < 0xFC) m->note = (note >> 4)*12 + (note&0xf) + 37;
			if (vol <= 64) { m->volcmd = VOLCMD_VOLUME; m->vol = vol; }
			m->param = p->cmdinf;
			switch(cmd)
			{
			// Axx set speed to xx
			case 1:	m->command = CMD_SPEED; m->param >>= 4; break;
			// Bxx position jump
			case 2:	m->command = CMD_POSITIONJUMP; break;
			// Cxx patternbreak to row xx
			case 3:	m->command = CMD_PATTERNBREAK; m->param = (m->param & 0xF0) * 10 + (m->param & 0x0F);	break;
			// Dxy volumeslide
			case 4:	m->command = CMD_VOLUMESLIDE; break;
			// Exy toneslide down
			case 5:	m->command = CMD_PORTAMENTODOWN; break;
			// Fxy toneslide up
			case 6:	m->command = CMD_PORTAMENTOUP; break;
			// Gxx Tone portamento,speed xx
			case 7:	m->command = CMD_TONEPORTAMENTO; break;
			// Hxy vibrato
			case 8:	m->command = CMD_VIBRATO; break;
			// Ixy tremor, ontime x, offtime y
			case 9:	m->command = CMD_TREMOR; break;
			// Jxy arpeggio
			case 10: m->command = CMD_ARPEGGIO; break;
			// Kxy Dual command H00 & Dxy
			case 11: m->command = CMD_VIBRATOVOL; break;
			// Lxy Dual command G00 & Dxy
			case 12: m->command = CMD_TONEPORTAVOL; break;
			// Xxx amiga command 8xx
			case 0x18:	m->command = CMD_PANNING8; break;
			default:
				m->command = m->param = 0;
			}
		}
		dwMemPos += 64*4*4;
	}
	// Reading Samples
	for (UINT nSmp=1; nSmp<=31; nSmp++)
	{
		MODINSTRUMENT *pIns = &Ins[nSmp];
		dwMemPos = (dwMemPos + 15) & (~15);
		if (pIns->nLength)
		{
			UINT nPos = ((UINT)phdr->sample[nSmp-1].reserved) << 4;
			if ((nPos >= sizeof(STMHEADER)) && (nPos+pIns->nLength <= dwMemLength)) dwMemPos = nPos;
			if (dwMemPos < dwMemLength)
			{
				dwMemPos += ReadSample(pIns, RS_PCM8S, (LPSTR)(lpStream+dwMemPos),dwMemLength-dwMemPos);
			}
		}
	}
	return TRUE;
}
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

//#include "stdafx.h"
//#include "sndfile.h"

//#pragma warning(disable:4244)

#define ULT_16BIT   0x04
#define ULT_LOOP    0x08
#define ULT_BIDI    0x10

#pragma pack(1)

// Raw ULT header struct:
typedef struct tagULTHEADER
{
        char id[15];             // changed from CHAR
        char songtitle[32];      // changed from CHAR
	BYTE reserved;
} ULTHEADER;


// Raw ULT sampleinfo struct:
typedef struct tagULTSAMPLE
{
	CHAR samplename[32];
	CHAR dosname[12];
	LONG loopstart;
	LONG loopend;
	LONG sizestart;
	LONG sizeend;
	BYTE volume;
	BYTE flags;
	WORD finetune;
} ULTSAMPLE;

#pragma pack()


BOOL CSoundFile::ReadUlt(const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
	ULTHEADER *pmh = (ULTHEADER *)lpStream;
	ULTSAMPLE *pus;
	UINT nos, nop;
	DWORD dwMemPos = 0;

	// try to read module header
	if ((!lpStream) || (dwMemLength < 0x100)) return FALSE;
	if (strncmp(pmh->id,"MAS_UTrack_V00",14)) return FALSE;
	// Warning! Not supported ULT format, trying anyway
	// if ((pmh->id[14] < '1') || (pmh->id[14] > '4')) return FALSE;
	m_nType = MOD_TYPE_ULT;
	m_nDefaultSpeed = 6;
	m_nDefaultTempo = 125;
	memcpy(m_szNames[0], pmh->songtitle, 32);
	m_szNames[0][31] = '\0';
	// read songtext
	dwMemPos = sizeof(ULTHEADER);
	if ((pmh->reserved) && (dwMemPos + pmh->reserved * 32 < dwMemLength))
	{
		UINT len = pmh->reserved * 32;
		m_lpszSongComments = new char[len + 1 + pmh->reserved];
		if (m_lpszSongComments)
		{
			for (UINT l=0; l<pmh->reserved; l++)
			{
				memcpy(m_lpszSongComments+l*33, lpStream+dwMemPos+l*32, 32);
				m_lpszSongComments[l*33+32] = 0x0D;
			}
			m_lpszSongComments[len] = 0;
		}
		dwMemPos += len;
	}
	if (dwMemPos >= dwMemLength) return TRUE;
	nos = lpStream[dwMemPos++];
	m_nSamples = nos;
	if (m_nSamples >= MAX_SAMPLES) m_nSamples = MAX_SAMPLES-1;
	UINT smpsize = 64;
	if (pmh->id[14] >= '4')	smpsize += 2;
	if (dwMemPos + nos*smpsize + 256 + 2 > dwMemLength) return TRUE;
	for (UINT ins=1; ins<=nos; ins++, dwMemPos+=smpsize) if (ins<=m_nSamples)
	{
		pus	= (ULTSAMPLE *)(lpStream+dwMemPos);
		MODINSTRUMENT *pins = &Ins[ins];
		memcpy(m_szNames[ins], pus->samplename, 32);
		m_szNames[ins][31] = '\0';
		memcpy(pins->name, pus->dosname, 12);
		pins->nLoopStart = pus->loopstart;
		pins->nLoopEnd = pus->loopend;
		pins->nLength = pus->sizeend - pus->sizestart;
		pins->nVolume = pus->volume;
		pins->nGlobalVol = 64;
		pins->nC4Speed = 8363;
		if (pmh->id[14] >= '4')
		{
			pins->nC4Speed = pus->finetune;
		}
		if (pus->flags & ULT_LOOP) pins->uFlags |= CHN_LOOP;
		if (pus->flags & ULT_BIDI) pins->uFlags |= CHN_PINGPONGLOOP;
		if (pus->flags & ULT_16BIT)
		{
			pins->uFlags |= CHN_16BIT;
			pins->nLoopStart >>= 1;
			pins->nLoopEnd >>= 1;
		}
	}
	memcpy(Order, lpStream+dwMemPos, 256);
	dwMemPos += 256;
	m_nChannels = lpStream[dwMemPos] + 1;
	nop = lpStream[dwMemPos+1] + 1;
	dwMemPos += 2;
	if (m_nChannels > 32) m_nChannels = 32;
	// Default channel settings
	for (UINT nSet=0; nSet<m_nChannels; nSet++)
	{
		ChnSettings[nSet].nVolume = 64;
		ChnSettings[nSet].nPan = (nSet & 1) ? 0x40 : 0xC0;
	}
	// read pan position table for v1.5 and higher
	if(pmh->id[14]>='3')
	{
		if (dwMemPos + m_nChannels > dwMemLength) return TRUE;
		for(UINT t=0; t<m_nChannels; t++)
		{
			ChnSettings[t].nPan = (lpStream[dwMemPos++] << 4) + 8;
			if (ChnSettings[t].nPan > 256) ChnSettings[t].nPan = 256;
		}
	}
	// Allocating Patterns
	for (UINT nAllocPat=0; nAllocPat<nop; nAllocPat++)
	{
		if (nAllocPat < MAX_PATTERNS)
		{
			PatternSize[nAllocPat] = 64;
			Patterns[nAllocPat] = AllocatePattern(64, m_nChannels);
		}
	}
	// Reading Patterns
	for (UINT nChn=0; nChn<m_nChannels; nChn++)
	{
		for (UINT nPat=0; nPat<nop; nPat++)
		{
			MODCOMMAND *pat = NULL;

			if (nPat < MAX_PATTERNS)
			{
				pat = Patterns[nPat];
				if (pat) pat += nChn;
			}
			UINT row = 0;
			while (row < 64)
			{
				if (dwMemPos > dwMemLength - 5) return TRUE;
				UINT rep = 1;
				UINT note = lpStream[dwMemPos++];
				if (note == 0xFC)
				{
					rep = lpStream[dwMemPos];
					note = lpStream[dwMemPos+1];
					dwMemPos += 2;
					if (dwMemPos > dwMemLength - 4) return TRUE;
				}

				UINT instr = lpStream[dwMemPos++];
				UINT eff = lpStream[dwMemPos++];
				UINT dat1 = lpStream[dwMemPos++];
				UINT dat2 = lpStream[dwMemPos++];
				UINT cmd1 = eff & 0x0F;
				UINT cmd2 = eff >> 4;
				if (cmd1 == 0x0C) dat1 >>= 2; else
				if (cmd1 == 0x0B) { cmd1 = dat1 = 0; }
				if (cmd2 == 0x0C) dat2 >>= 2; else
				if (cmd2 == 0x0B) { cmd2 = dat2 = 0; }
				while ((rep != 0) && (row < 64))
				{
					if (pat)
					{
						pat->instr = instr;
						if (note) pat->note = note + 36;
						if (cmd1 | dat1)
						{
							if (cmd1 == 0x0C)
							{
								pat->volcmd = VOLCMD_VOLUME;
								pat->vol = dat1;
							} else
							{
								pat->command = cmd1;
								pat->param = dat1;
								ConvertModCommand(pat);
							}
						}
						if (cmd2 == 0x0C)
						{
							pat->volcmd = VOLCMD_VOLUME;
							pat->vol = dat2;
						} else
						if ((cmd2 | dat2) && (!pat->command))
						{
							pat->command = cmd2;
							pat->param = dat2;
							ConvertModCommand(pat);
						}
						pat += m_nChannels;
					}
					row++;
					rep--;
				}
			}
		}
	}
	// Reading Instruments
	for (UINT smp=1; smp<=m_nSamples; smp++) if (Ins[smp].nLength)
	{
		if (dwMemPos >= dwMemLength) return TRUE;
		UINT flags = (Ins[smp].uFlags & CHN_16BIT) ? RS_PCM16S : RS_PCM8S;
		dwMemPos += ReadSample(&Ins[smp], flags, (LPSTR)(lpStream+dwMemPos), dwMemLength - dwMemPos);
	}
	return TRUE;
}
/*
 * This source code is public domain.
 *
 * Epic Games Unreal UMX container loading for libmodplug
 * Written by O. Sezer <sezero@users.sourceforge.net>
 * UPKG parsing partially based on Unreal Media Ripper (UMR) v0.3
 * by Andy Ward <wardwh@swbell.net>, with additional updates
 * by O. Sezer - see git repo at https://github.com/sezero/umr.git
 * Retrieves the offset, size and object type directly from umx.
*/

//#include "stdafx.h"
//#include "sndfile.h"


typedef LONG fci_t;		/* FCompactIndex */

#define UPKG_HDR_TAG	0x9e2a83c1

struct _genhist {	/* for upkg versions >= 68 */
	LONG export_count;
	LONG name_count;
};

struct upkg_hdr {
	DWORD tag;	/* UPKG_HDR_TAG */
	LONG file_version;
	DWORD pkg_flags;
	LONG name_count;	/* number of names in name table (>= 0) */
	LONG name_offset;		/* offset to name table  (>= 0) */
	LONG export_count;	/* num. exports in export table  (>= 0) */
	LONG export_offset;		/* offset to export table (>= 0) */
	LONG import_count;	/* num. imports in export table  (>= 0) */
	LONG import_offset;		/* offset to import table (>= 0) */

	/* number of GUIDs in heritage table (>= 1) and table's offset:
	 * only with versions < 68. */
	LONG heritage_count;
	LONG heritage_offset;
	/* with versions >= 68:  a GUID, a dword for generation count
	 * and export_count and name_count dwords for each generation: */
	DWORD guid[4];
	LONG generation_count;
#define UPKG_HDR_SIZE 64			/* 64 bytes up until here */
	/*struct _genhist *gen;*/
};

#define UMUSIC_IT	0
#define UMUSIC_S3M	1
#define UMUSIC_XM	2
#define UMUSIC_MOD	3
#define UMUSIC_WAV	4
#define UMUSIC_MP2	5

static const char *mustype[] = {
	"IT", "S3M", "XM", "MOD",
	NULL
};

/* decode an FCompactIndex.
 * original documentation by Tim Sweeney was at
 * http://unreal.epicgames.com/Packages.htm
 * also see Unreal Wiki:
 * http://wiki.beyondunreal.com/Legacy:Package_File_Format/Data_Details
 */
static fci_t get_fci (const char *in, int *pos)
{
	LONG a;
	int size;

	size = 1;
	a = in[0] & 0x3f;

	if (in[0] & 0x40) {
		size++;
		a |= (in[1] & 0x7f) << 6;

		if (in[1] & 0x80) {
			size++;
			a |= (in[2] & 0x7f) << 13;

			if (in[2] & 0x80) {
				size++;
				a |= (in[3] & 0x7f) << 20;

				if (in[3] & 0x80) {
					size++;
					a |= (in[4] & 0x3f) << 27;
				}
			}
		}
	}

	if (in[0] & 0x80)
		a = -a;

	*pos += size;

	return a;
}

static int get_objtype (const BYTE *membase, LONG memlen,
			LONG ofs, int type)
{
	if (type == UMUSIC_IT) {
	_retry:
		if (memcmp(membase + ofs, "IMPM", 4) == 0)
			return UMUSIC_IT;
		return -1;
	}
	if (type == UMUSIC_XM) {
		if (memcmp(membase + ofs, "Extended Module: ", 17) != 0)
			return -1;
		if (*(membase + ofs + 37) != 0x1a) return -1;
		return UMUSIC_XM;
	}

	if (type == UMUSIC_S3M) {
		if (memcmp(membase + ofs + 44, "SCRM", 4) == 0)
			return UMUSIC_S3M;
		/*return -1;*/
		/* SpaceMarines.umx and Starseek.umx from Return to NaPali
		 * report as "s3m" whereas the actual music format is "it" */
		goto _retry;
	}

	if (type == UMUSIC_MOD) {
		membase += ofs + 1080;
		if (memcmp(membase, "M.K.", 4) == 0 || memcmp(membase, "M!K!", 4) == 0)
			return UMUSIC_MOD;
		return -1;
	}

	return -1;
}

static int read_export (const BYTE *membase, LONG memlen,
			const struct upkg_hdr *hdr,
			LONG *ofs, LONG *objsize)
{
	char buf[40];
	int idx = 0, t;

	memcpy(buf, membase + *ofs, 40);

	if (hdr->file_version < 40) idx += 8;	/* 00 00 00 00 00 00 00 00 */
	if (hdr->file_version < 60) idx += 16;	/* 81 00 00 00 00 00 FF FF FF FF FF FF FF FF 00 00 */
	get_fci(&buf[idx], &idx);		/* skip junk */
	t = get_fci(&buf[idx], &idx);		/* type_name */
	if (hdr->file_version > 61) idx += 4;	/* skip export size */
	*objsize = get_fci(&buf[idx], &idx);
	*ofs += idx;	/* offset for real data */

	return t;	/* return type_name index */
}

static int read_typname(const BYTE *membase, LONG memlen,
			const struct upkg_hdr *hdr,
			int idx, char *out)
{
	int i, s;
	long l, ofs, siz;
	char buf[64];

	if (idx >= hdr->name_count) return -1;
	memset(buf, 0, 64);
	for (i = 0, l = 0; i <= idx; i++) {
		if ((ofs = hdr->name_offset + l) >= memlen)
			return -1;
		if ((siz = memlen - ofs) > 63) siz = 63;
		memcpy(buf, membase + ofs, siz);
		if (hdr->file_version >= 64) {
			s = *(signed char *)buf; /* numchars *including* terminator */
			if (s <= 0) return -1;
			l += s + 5;	/* 1 for buf[0], 4 for int32_t name_flags */
		} else {
			l += (long)strlen(buf);
			l +=  5;	/* 1 for terminator, 4 for int32_t name_flags */
		}
	}

	strcpy(out, (hdr->file_version >= 64)? &buf[1] : buf);
	return 0;
}

static void umx_strupr(char *str)
{
	while (*str) {
		if (*str >= 'a' && *str <= 'z') {
		    *str -= ('a' - 'A');
		}
		str++;
	}
}

static int probe_umx   (const BYTE *membase, LONG memlen,
			const struct upkg_hdr *hdr,
			LONG *ofs, LONG *objsize)
{
	int i, idx, t;
	LONG s, pos;
	char buf[64];

	if (hdr->name_offset	>= memlen ||
	    hdr->export_offset	>= memlen ||
	    hdr->import_offset	>= memlen) {
		return -1;
	}

	/* Find the offset and size of the first IT, S3M or XM
	 * by parsing the exports table. The umx files should
	 * have only one export. Kran32.umx from Unreal has two,
	 * but both pointing to the same music. */
	s = memlen - hdr->export_offset;
	if (s <= 0) return -1;
	if (s > 64) s = 64;
	memcpy(buf, membase + hdr->export_offset, s);
	for (; s < 64; ++s) buf[s] = 0x0; /* really? */

	idx = 0;

	get_fci(&buf[idx], &idx);	/* skip class_index */
	get_fci(&buf[idx], &idx);	/* skip super_index */
	if (hdr->file_version >= 60) idx += 4; /* skip int32 package_index */
	get_fci(&buf[idx], &idx);	/* skip object_name */
	idx += 4;			/* skip int32 object_flags */

	s = get_fci(&buf[idx], &idx);	/* get serial_size */
	if (s <= 0) return -1;
	pos = get_fci(&buf[idx],&idx);	/* get serial_offset */
	if (pos < 0 || pos > memlen - 40) return -1;

	if ((t = read_export(membase, memlen, hdr, &pos, &s)) < 0) return -1;
	if (s <= 0 || s > memlen - pos) return -1;

	if (read_typname(membase, memlen, hdr, t, buf) < 0) return -1;
	umx_strupr(buf);
	for (i = 0; mustype[i] != NULL; i++) {
		if (!strcmp(buf, mustype[i])) {
			t = i;
			break;
		}
	}
	if (mustype[i] == NULL) return -1;
	if ((t = get_objtype(membase, memlen, pos, t)) < 0) return -1;

	*ofs = pos;
	*objsize = s;
	return t;
}

static int probe_header (void *header)
{
	struct upkg_hdr *hdr;
	unsigned char *p;
	DWORD *swp;
	int i;

	/* byte swap the header - all members are 32 bit LE values */
	p = (unsigned char *) header;
	swp = (DWORD *) header;
	for (i = 0; i < UPKG_HDR_SIZE/4; i++, p += 4) {
		swp[i] = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
	}

	hdr = (struct upkg_hdr *) header;
	if (hdr->tag != UPKG_HDR_TAG) {
		return -1;
	}
	if (hdr->name_count	< 0	||
	    hdr->export_count	< 0	||
	    hdr->import_count	< 0	||
	    hdr->name_offset	< 36	||
	    hdr->export_offset	< 36	||
	    hdr->import_offset	< 36	) {
		return -1;
	}

#if 1 /* no need being overzealous */
	return 0;
#else
	switch (hdr->file_version) {
	case 35: case 37:	/* Unreal beta - */
	case 40: case 41:				/* 1998 */
	case 61:/* Unreal */
	case 62:/* Unreal Tournament */
	case 63:/* Return to NaPali */
	case 64:/* Unreal Tournament */
	case 66:/* Unreal Tournament */
	case 68:/* Unreal Tournament */
	case 69:/* Tactical Ops */
	case 83:/* Mobile Forces */
		return 0;
	}

	return -1;/* Unknown upkg version for an UMX */
#endif /* #if 0  */
}

static int process_upkg (const BYTE *membase, LONG memlen,
			 LONG *ofs, LONG *objsize)
{
	char header[UPKG_HDR_SIZE];

	memcpy(header, membase, UPKG_HDR_SIZE);
	if (probe_header(header) < 0)
		return -1;

	return probe_umx(membase, memlen, (struct upkg_hdr *)header, ofs, objsize);
}

BOOL CSoundFile::ReadUMX(const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
	int type;
	LONG ofs = 0, size = 0;

	if (!lpStream || dwMemLength < 0x800 || dwMemLength > 0x7fffffff)
		return FALSE;
	type = process_upkg(lpStream, (LONG)dwMemLength, &ofs, &size);
	if (type < 0) return FALSE;

	// Rip Mods from UMX
	switch (type) {
	case UMUSIC_IT:  return ReadIT(lpStream + ofs, size);
	case UMUSIC_S3M: return ReadS3M(lpStream + ofs, size);
	case UMUSIC_XM:  return ReadXM(lpStream + ofs, size);
	case UMUSIC_MOD: return ReadMod(lpStream + ofs, size);
	}
	return FALSE;
}

/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

//#include "stdafx.h"
//#include "sndfile.h"

#ifndef WAVE_FORMAT_EXTENSIBLE
#define WAVE_FORMAT_EXTENSIBLE	0xFFFE
#endif

/////////////////////////////////////////////////////////////
// WAV file support

BOOL CSoundFile::ReadWav(const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
	DWORD dwMemPos = 0;
	const WAVEFILEHEADER *phdr = (WAVEFILEHEADER *)lpStream;
	const WAVEFORMATHEADER *pfmt = (WAVEFORMATHEADER *)(lpStream + sizeof(WAVEFILEHEADER));
	if ((!lpStream) || (dwMemLength < sizeof(WAVEFILEHEADER)+sizeof(WAVEFORMATHEADER))) return FALSE;
	if ((phdr->id_RIFF != IFFID_RIFF) || (phdr->id_WAVE != IFFID_WAVE)
	 || (pfmt->id_fmt != IFFID_fmt)) return FALSE;
	dwMemPos = sizeof(WAVEFILEHEADER) + 8 + pfmt->hdrlen;
	if ((dwMemPos >= dwMemLength - 8)
	 || ((pfmt->format != WAVE_FORMAT_PCM) && (pfmt->format != WAVE_FORMAT_EXTENSIBLE))
	 || (pfmt->channels > 4)
	 || (!pfmt->channels)
	 || (!pfmt->freqHz)
	 || (pfmt->bitspersample & 7)
	 || (pfmt->bitspersample < 8)
	 || (pfmt->bitspersample > 32))  return FALSE;
	const WAVEDATAHEADER *pdata;
	for (;;)
	{
		pdata = (WAVEDATAHEADER *)(lpStream + dwMemPos);
		if (pdata->id_data == IFFID_data) break;
		if (pdata->length >= dwMemLength || dwMemPos > dwMemLength - pdata->length) return FALSE;
		dwMemPos += pdata->length + 8;
		if (dwMemPos >= dwMemLength - 8) return FALSE;
	}
	m_nType = MOD_TYPE_WAV;
	m_nSamples = 0;
	m_nInstruments = 0;
	m_nChannels = 4;
	m_nDefaultSpeed = 8;
	m_nDefaultTempo = 125;
	m_dwSongFlags |= SONG_LINEARSLIDES; // For no resampling
	Order[0] = 0;
	Order[1] = 0xFF;
	PatternSize[0] = PatternSize[1] = 64;
	if ((Patterns[0] = AllocatePattern(64, 4)) == NULL) return TRUE;
	if ((Patterns[1] = AllocatePattern(64, 4)) == NULL) return TRUE;
	UINT samplesize = (pfmt->channels * pfmt->bitspersample) >> 3;
	UINT len = pdata->length, bytelen;
	if (len > dwMemLength - 8 - dwMemPos) len = dwMemLength - dwMemPos - 8;
	len /= samplesize;
	bytelen = len;
	if (pfmt->bitspersample >= 16) bytelen *= 2;
	if (len > MAX_SAMPLE_LENGTH) len = MAX_SAMPLE_LENGTH;
	if (!len) return TRUE;
	// Setting up module length
	DWORD dwTime = ((len * 50) / pfmt->freqHz) + 1;
	DWORD framesperrow = (dwTime + 63) / 63;
	if (framesperrow < 4) framesperrow = 4;
	UINT norders = 1;
	while (framesperrow >= 0x20)
	{
		Order[norders++] = 1;
		Order[norders] = 0xFF;
		framesperrow = (dwTime + (64 * norders - 1)) / (64 * norders);
		if (norders >= MAX_ORDERS-1) break;
	}
	m_nDefaultSpeed = framesperrow;
	for (UINT iChn=0; iChn<4; iChn++)
	{
		ChnSettings[iChn].nPan = (iChn & 1) ? 256 : 0;
		ChnSettings[iChn].nVolume = 64;
		ChnSettings[iChn].dwFlags = 0;
	}
	// Setting up speed command
	MODCOMMAND *pcmd = Patterns[0];
	pcmd[0].command = CMD_SPEED;
	pcmd[0].param = (BYTE)m_nDefaultSpeed;
	pcmd[0].note = 5*12+1;
	pcmd[0].instr = 1;
	pcmd[1].note = pcmd[0].note;
	pcmd[1].instr = pcmd[0].instr;
	m_nSamples = pfmt->channels;
	// Support for Multichannel Wave
	for (UINT nChn=0; nChn<m_nSamples; nChn++)
	{
		MODINSTRUMENT *pins = &Ins[nChn+1];
		pcmd[nChn].note = pcmd[0].note;
		pcmd[nChn].instr = (BYTE)(nChn+1);
		pins->nLength = len;
		pins->nC4Speed = pfmt->freqHz;
		pins->nVolume = 256;
		pins->nPan = 128;
		pins->nGlobalVol = 64;
		pins->uFlags = (WORD)((pfmt->bitspersample >= 16) ? CHN_16BIT : 0);
		pins->uFlags |= CHN_PANNING;
		if (m_nSamples > 1)
		{
			switch(nChn)
			{
			case 0:	pins->nPan = 0; break;
			case 1:	pins->nPan = 256; break;
			case 2: pins->nPan = (WORD)((m_nSamples == 3) ? 128 : 64); pcmd[nChn].command = CMD_S3MCMDEX; pcmd[nChn].param = 0x91; break;
			case 3: pins->nPan = 192; pcmd[nChn].command = CMD_S3MCMDEX; pcmd[nChn].param = 0x91; break;
			default: pins->nPan = 128; break;
			}
		}
		if ((pins->pSample = AllocateSample(bytelen+8)) == NULL) return TRUE;
		if (pfmt->bitspersample >= 16)
		{
			int slsize = pfmt->bitspersample >> 3;
			signed short *p = (signed short *)pins->pSample;
			signed char *psrc = (signed char *)(lpStream+dwMemPos+8+nChn*slsize+slsize-2);
			for (UINT i=0; i<len; i++)
			{
				p[i] = *((signed short *)psrc);
				psrc += samplesize;
			}
			p[len+1] = p[len] = p[len-1];
		} else
		{
			signed char *p = (signed char *)pins->pSample;
			signed char *psrc = (signed char *)(lpStream+dwMemPos+8+nChn);
			for (UINT i=0; i<len; i++)
			{
				p[i] = (signed char)((*psrc) + 0x80);
				psrc += samplesize;
			}
			p[len+1] = p[len] = p[len-1];
		}
	}
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// IMA ADPCM Support

#pragma pack(1)

typedef struct IMAADPCMBLOCK
{
	WORD sample;
	BYTE index;
	BYTE Reserved;
} DVI_ADPCMBLOCKHEADER;

#pragma pack()

static const int gIMAUnpackTable[90] =
{
  7,     8,     9,    10,    11,    12,    13,    14,
  16,    17,    19,    21,    23,    25,    28,    31,
  34,    37,    41,    45,    50,    55,    60,    66,
  73,    80,    88,    97,   107,   118,   130,   143,
  157,   173,   190,   209,   230,   253,   279,   307,
  337,   371,   408,   449,   494,   544,   598,   658,
  724,   796,   876,   963,  1060,  1166,  1282,  1411,
  1552,  1707,  1878,  2066,  2272,  2499,  2749,  3024,
  3327,  3660,  4026,  4428,  4871,  5358,  5894,  6484,
  7132,  7845,  8630,  9493, 10442, 11487, 12635, 13899,
  15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
  32767, 0
};


BOOL IMAADPCMUnpack16(signed short *pdest, UINT nLen, LPBYTE psrc, DWORD dwBytes, UINT pkBlkAlign)
//------------------------------------------------------------------------------------------------
{
	static const int gIMAIndexTab[8] =  { -1, -1, -1, -1, 2, 4, 6, 8 };
	UINT nPos;
	int value;

	if ((nLen < 4) || (!pdest) || (!psrc)
	 || (pkBlkAlign < 5) || (pkBlkAlign > dwBytes)) return FALSE;
	nPos = 0;
	while ((nPos < nLen) && (dwBytes > 4))
	{
		int nIndex;
		value = *((short int *)psrc);
		nIndex = psrc[2];
		psrc += 4;
		dwBytes -= 4;
		pdest[nPos++] = (short int)value;
		for (UINT i=0; ((i<(pkBlkAlign-4)*2) && (nPos < nLen) && (dwBytes)); i++)
		{
			BYTE delta;
			if (i & 1)
			{
				delta = (BYTE)(((*(psrc++)) >> 4) & 0x0F);
				dwBytes--;
			} else
			{
				delta = (BYTE)((*psrc) & 0x0F);
			}
			int v = gIMAUnpackTable[nIndex] >> 3;
			if (delta & 1) v += gIMAUnpackTable[nIndex] >> 2;
			if (delta & 2) v += gIMAUnpackTable[nIndex] >> 1;
			if (delta & 4) v += gIMAUnpackTable[nIndex];
			if (delta & 8) value -= v; else value += v;
			nIndex += gIMAIndexTab[delta & 7];
			if (nIndex < 0) nIndex = 0; else
			if (nIndex > 88) nIndex = 88;
			if (value > 32767) value = 32767; else
			if (value < -32768) value = -32768;
			pdest[nPos++] = (short int)value;
		}
	}
	return TRUE;
}
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (endian and char fixes for PPC)
*/

//#include "stdafx.h"
//#include "sndfile.h"

////////////////////////////////////////////////////////
// FastTracker II XM file support

#ifdef _MSC_VER
#pragma warning(disable:4244)
#endif

#pragma pack(1)
typedef struct tagXMFILEHEADER
{
	DWORD size;
	WORD norder;
	WORD restartpos;
	WORD channels;
	WORD patterns;
	WORD instruments;
	WORD flags;
	WORD speed;
	WORD tempo;
	BYTE order[256];
} XMFILEHEADER;


typedef struct tagXMINSTRUMENTHEADER
{
	DWORD size;
	CHAR name[22];
	BYTE type;
	BYTE samples;
	BYTE samplesh;
} XMINSTRUMENTHEADER;


typedef struct tagXMSAMPLEHEADER
{
	DWORD shsize;
	BYTE snum[96];
	WORD venv[24];
	WORD penv[24];
	BYTE vnum, pnum;
	BYTE vsustain, vloops, vloope, psustain, ploops, ploope;
	BYTE vtype, ptype;
	BYTE vibtype, vibsweep, vibdepth, vibrate;
	WORD volfade;
	WORD res;
	BYTE reserved1[20];
} XMSAMPLEHEADER;

typedef struct tagXMSAMPLESTRUCT
{
	DWORD samplen;
	DWORD loopstart;
	DWORD looplen;
	BYTE vol;
	signed char finetune;
	BYTE type;
	BYTE pan;
	signed char relnote;
	BYTE res;
	char name[22];
} XMSAMPLESTRUCT;
#pragma pack()


BOOL CSoundFile::ReadXM(const BYTE *lpStream, DWORD dwMemLength)
//--------------------------------------------------------------
{
	XMSAMPLEHEADER xmsh;
	XMSAMPLESTRUCT xmss;
	DWORD dwMemPos, dwHdrSize;
	WORD norders=0, restartpos=0, channels=0, patterns=0, instruments=0;
	WORD xmflags=0, deftempo=125, defspeed=6;
	BOOL InstUsed[256];
	BYTE channels_used[MAX_CHANNELS];
	BYTE pattern_map[256];
	BOOL samples_used[MAX_SAMPLES];
	UINT unused_samples;
	tagXMFILEHEADER xmhead;

	m_nChannels = 0;
	if ((!lpStream) || (dwMemLength < 0x200)) return FALSE;
	if (strncmp((LPCSTR)lpStream, "Extended Module:", 16)) return FALSE;

	memcpy(m_szNames[0], lpStream+17, 20);
	xmhead = *(tagXMFILEHEADER *)(lpStream+60);
	dwHdrSize = bswapLE32(xmhead.size);
	norders = bswapLE16(xmhead.norder);
	restartpos = bswapLE16(xmhead.restartpos);
	channels = bswapLE16(xmhead.channels);

	if ((!dwHdrSize) || dwHdrSize > dwMemLength - 60) return FALSE;
	if ((!norders) || (norders > MAX_ORDERS)) return FALSE;
	if ((!channels) || (channels > 64)) return FALSE;
	m_nType = MOD_TYPE_XM;
	m_nMinPeriod = 27;
	m_nMaxPeriod = 54784;
	m_nChannels = channels;
	if (restartpos < norders) m_nRestartPos = restartpos;
	patterns = bswapLE16(xmhead.patterns);
	if (patterns > 256) patterns = 256;
	instruments = bswapLE16(xmhead.instruments);
	if (instruments >= MAX_INSTRUMENTS) instruments = MAX_INSTRUMENTS-1;
	m_nInstruments = instruments;
	m_nSamples = 0;
	xmflags = bswapLE16(xmhead.flags);
	if (xmflags & 1) m_dwSongFlags |= SONG_LINEARSLIDES;
	if (xmflags & 0x1000) m_dwSongFlags |= SONG_EXFILTERRANGE;
	defspeed = bswapLE16(xmhead.speed);
	deftempo = bswapLE16(xmhead.tempo);
	if ((deftempo >= 32) && (deftempo < 256)) m_nDefaultTempo = deftempo;
	if ((defspeed > 0) && (defspeed < 40)) m_nDefaultSpeed = defspeed;
	memcpy(Order, lpStream+80, norders);
	memset(InstUsed, 0, sizeof(InstUsed));
	if (patterns > MAX_PATTERNS)
	{
		UINT i, j;
		for (i=0; i<norders; i++)
		{
			if (Order[i] < patterns) InstUsed[Order[i]] = TRUE;
		}
		j = 0;
		for (i=0; i<256; i++)
		{
			if (InstUsed[i]) pattern_map[i] = j++;
		}
		for (i=0; i<256; i++)
		{
			if (!InstUsed[i])
			{
				pattern_map[i] = (j < MAX_PATTERNS) ? j : 0xFE;
				j++;
			}
		}
		for (i=0; i<norders; i++)
		{
			Order[i] = pattern_map[Order[i]];
		}
	} else
	{
		for (UINT i=0; i<256; i++) pattern_map[i] = i;
	}
	memset(InstUsed, 0, sizeof(InstUsed));
	dwMemPos = dwHdrSize + 60;
	if (dwMemPos + 8 >= dwMemLength) return TRUE;
	// Reading patterns
	memset(channels_used, 0, sizeof(channels_used));
	for (UINT ipat=0; ipat<patterns; ipat++)
	{
		UINT ipatmap = pattern_map[ipat];
		DWORD dwSize = 0;
		WORD rows=64, packsize=0;
		dwSize = bswapLE32(*((DWORD *)(lpStream+dwMemPos)));
		while ((dwMemPos + dwSize >= dwMemLength) || (dwSize & 0xFFFFFF00))
		{
			if (dwMemPos + 4 >= dwMemLength) break;
			dwMemPos++;
			dwSize = bswapLE32(*((DWORD *)(lpStream+dwMemPos)));
		}
		if (dwMemPos + 9 > dwMemLength) return TRUE;
		rows = bswapLE16(*((WORD *)(lpStream+dwMemPos+5)));
		if ((!rows) || (rows > 256)) rows = 64;
		packsize = bswapLE16(*((WORD *)(lpStream+dwMemPos+7)));
		if (dwMemPos + dwSize + 4 > dwMemLength) return TRUE;
		dwMemPos += dwSize;
		if (dwMemPos + packsize + 4 > dwMemLength) return TRUE;
		MODCOMMAND *p;
		if (ipatmap < MAX_PATTERNS)
		{
			PatternSize[ipatmap] = rows;
			if ((Patterns[ipatmap] = AllocatePattern(rows, m_nChannels)) == NULL) return TRUE;
			if (!packsize) continue;
			p = Patterns[ipatmap];
		} else p = NULL;
		const BYTE *src = lpStream+dwMemPos;
		UINT j=0;
		for (UINT row=0; row<rows; row++)
		{
			for (UINT chn=0; chn<m_nChannels; chn++)
			{
				if ((p) && (j < packsize))
				{
					BYTE b = src[j++];
					UINT vol = 0;
					if (b & 0x80)
					{
						if (b & 1) p->note = j < packsize ? src[j++] : 0;
						if (b & 2) p->instr = j < packsize ? src[j++] : 0;
						if (b & 4) vol = j < packsize ? src[j++] : 0;
						if (b & 8) p->command = j < packsize ? src[j++] : 0;
						if (b & 16) p->param = j < packsize ? src[j++] : 0;
					} else
					{
						if (j + 5 > packsize) break;
						p->note = b;
						p->instr = src[j++];
						vol = src[j++];
						p->command = src[j++];
						p->param = src[j++];
					}
					if (p->note == 97) p->note = 0xFF; else
					if ((p->note) && (p->note < 97)) p->note += 12;
					if (p->note) channels_used[chn] = 1;
					if (p->command | p->param) ConvertModCommand(p);
					if (p->instr == 0xff) p->instr = 0;
					if (p->instr) InstUsed[p->instr] = TRUE;
					if ((vol >= 0x10) && (vol <= 0x50))
					{
						p->volcmd = VOLCMD_VOLUME;
						p->vol = vol - 0x10;
					} else
					if (vol >= 0x60)
					{
						UINT v = vol & 0xF0;
						vol &= 0x0F;
						p->vol = vol;
						switch(v)
						{
						// 60-6F: Volume Slide Down
						case 0x60:	p->volcmd = VOLCMD_VOLSLIDEDOWN; break;
						// 70-7F: Volume Slide Up:
						case 0x70:	p->volcmd = VOLCMD_VOLSLIDEUP; break;
						// 80-8F: Fine Volume Slide Down
						case 0x80:	p->volcmd = VOLCMD_FINEVOLDOWN; break;
						// 90-9F: Fine Volume Slide Up
						case 0x90:	p->volcmd = VOLCMD_FINEVOLUP; break;
						// A0-AF: Set Vibrato Speed
						case 0xA0:	p->volcmd = VOLCMD_VIBRATOSPEED; break;
						// B0-BF: Vibrato
						case 0xB0:	p->volcmd = VOLCMD_VIBRATO; break;
						// C0-CF: Set Panning
						case 0xC0:	p->volcmd = VOLCMD_PANNING; p->vol = (vol << 2) + 2; break;
						// D0-DF: Panning Slide Left
						case 0xD0:	p->volcmd = VOLCMD_PANSLIDELEFT; break;
						// E0-EF: Panning Slide Right
						case 0xE0:	p->volcmd = VOLCMD_PANSLIDERIGHT; break;
						// F0-FF: Tone Portamento
						case 0xF0:	p->volcmd = VOLCMD_TONEPORTAMENTO; break;
						}
					}
					p++;
				} else
				if (j < packsize)
				{
					BYTE b = src[j++];
					if (b & 0x80)
					{
						if (b & 1) j++;
						if (b & 2) j++;
						if (b & 4) j++;
						if (b & 8) j++;
						if (b & 16) j++;
					} else j += 4;
				} else break;
			}
		}
		dwMemPos += packsize;
	}
	// Wrong offset check
	while (dwMemPos + 4 < dwMemLength)
	{
		DWORD d = bswapLE32(*((DWORD *)(lpStream+dwMemPos)));
		if (d < 0x300) break;
		dwMemPos++;
	}
	memset(samples_used, 0, sizeof(samples_used));
	unused_samples = 0;
	// Reading instruments
	for (UINT iIns=1; iIns<=instruments; iIns++)
	{
		XMINSTRUMENTHEADER *pih;
		BYTE flags[32];
		DWORD samplesize[32];
		UINT samplemap[32];
		WORD nsamples;
		DWORD pihlen;

		if (dwMemPos + sizeof(XMINSTRUMENTHEADER) >= dwMemLength) return TRUE;
		pih = (XMINSTRUMENTHEADER *)(lpStream+dwMemPos);
		pihlen = bswapLE32(pih->size);
		if (pihlen >= dwMemLength || dwMemPos > dwMemLength - pihlen) return TRUE;
		if ((Headers[iIns] = new INSTRUMENTHEADER) == NULL) continue;
		memset(Headers[iIns], 0, sizeof(INSTRUMENTHEADER));
		memcpy(Headers[iIns]->name, pih->name, 22);
		if ((nsamples = pih->samples) > 0)
		{
			if (dwMemPos + sizeof(XMINSTRUMENTHEADER) + sizeof(XMSAMPLEHEADER) > dwMemLength) return TRUE;
			memcpy(&xmsh, lpStream+dwMemPos+sizeof(XMINSTRUMENTHEADER), sizeof(XMSAMPLEHEADER));
			xmsh.shsize = bswapLE32(xmsh.shsize);
			for (int i = 0; i < 24; ++i) {
			  xmsh.venv[i] = bswapLE16(xmsh.venv[i]);
			  xmsh.penv[i] = bswapLE16(xmsh.penv[i]);
			}
			xmsh.volfade = bswapLE16(xmsh.volfade);
			xmsh.res = bswapLE16(xmsh.res);
			dwMemPos += pihlen;
		} else
		{
			if (pihlen) dwMemPos += pihlen;
			else dwMemPos += sizeof(XMINSTRUMENTHEADER);
			continue;
		}
		memset(samplemap, 0, sizeof(samplemap));
		if (nsamples > 32) return TRUE;
		UINT newsamples = m_nSamples;
		for (UINT nmap=0; nmap<nsamples; nmap++)
		{
			UINT n = m_nSamples+nmap+1;
			if (n >= MAX_SAMPLES)
			{
				n = m_nSamples;
				while (n > 0)
				{
					if (!Ins[n].pSample)
					{
						for (UINT xmapchk=0; xmapchk < nmap; xmapchk++)
						{
							if (samplemap[xmapchk] == n) goto alreadymapped;
						}
						for (UINT clrs=1; clrs<iIns; clrs++) if (Headers[clrs])
						{
							INSTRUMENTHEADER *pks = Headers[clrs];
							for (UINT ks=0; ks<128; ks++)
							{
								if (pks->Keyboard[ks] == n) pks->Keyboard[ks] = 0;
							}
						}
						break;
					}
				alreadymapped:
					n--;
				}
#ifndef MODPLUG_FASTSOUNDLIB
				// Damn! more than 200 samples: look for duplicates
				if (!n)
				{
					if (!unused_samples)
					{
						unused_samples = DetectUnusedSamples(samples_used);
						if (!unused_samples) unused_samples = 0xFFFF;
					}
					if ((unused_samples) && (unused_samples != 0xFFFF))
					{
						for (UINT iext=m_nSamples; iext>=1; iext--) if (!samples_used[iext])
						{
							unused_samples--;
							samples_used[iext] = TRUE;
							DestroySample(iext);
							n = iext;
							for (UINT mapchk=0; mapchk<nmap; mapchk++)
							{
								if (samplemap[mapchk] == n) samplemap[mapchk] = 0;
							}
							for (UINT clrs=1; clrs<iIns; clrs++) if (Headers[clrs])
							{
								INSTRUMENTHEADER *pks = Headers[clrs];
								for (UINT ks=0; ks<128; ks++)
								{
									if (pks->Keyboard[ks] == n) pks->Keyboard[ks] = 0;
								}
							}
							memset(&Ins[n], 0, sizeof(Ins[0]));
							break;
						}
					}
				}
#endif // MODPLUG_FASTSOUNDLIB
			}
			if (newsamples < n) newsamples = n;
			samplemap[nmap] = n;
		}
		m_nSamples = newsamples;
		// Reading Volume Envelope
		INSTRUMENTHEADER *penv = Headers[iIns];
		penv->nMidiProgram = pih->type;
		penv->nFadeOut = xmsh.volfade;
		penv->nPan = 128;
		penv->nPPC = 5*12;
		if (xmsh.vtype & 1) penv->dwFlags |= ENV_VOLUME;
		if (xmsh.vtype & 2) penv->dwFlags |= ENV_VOLSUSTAIN;
		if (xmsh.vtype & 4) penv->dwFlags |= ENV_VOLLOOP;
		if (xmsh.ptype & 1) penv->dwFlags |= ENV_PANNING;
		if (xmsh.ptype & 2) penv->dwFlags |= ENV_PANSUSTAIN;
		if (xmsh.ptype & 4) penv->dwFlags |= ENV_PANLOOP;
		if (xmsh.vnum > 12) xmsh.vnum = 12;
		if (xmsh.pnum > 12) xmsh.pnum = 12;
		penv->nVolEnv = xmsh.vnum;
		if (!xmsh.vnum) penv->dwFlags &= ~ENV_VOLUME;
		if (!xmsh.pnum) penv->dwFlags &= ~ENV_PANNING;
		penv->nPanEnv = xmsh.pnum;
		penv->nVolSustainBegin = penv->nVolSustainEnd = xmsh.vsustain;
		if (xmsh.vsustain >= 12) penv->dwFlags &= ~ENV_VOLSUSTAIN;
		penv->nVolLoopStart = xmsh.vloops;
		penv->nVolLoopEnd = xmsh.vloope;
		if (penv->nVolLoopEnd >= 12) penv->nVolLoopEnd = 0;
		if (penv->nVolLoopStart >= penv->nVolLoopEnd) penv->dwFlags &= ~ENV_VOLLOOP;
		penv->nPanSustainBegin = penv->nPanSustainEnd = xmsh.psustain;
		if (xmsh.psustain >= 12) penv->dwFlags &= ~ENV_PANSUSTAIN;
		penv->nPanLoopStart = xmsh.ploops;
		penv->nPanLoopEnd = xmsh.ploope;
		if (penv->nPanLoopEnd >= 12) penv->nPanLoopEnd = 0;
		if (penv->nPanLoopStart >= penv->nPanLoopEnd) penv->dwFlags &= ~ENV_PANLOOP;
		penv->nGlobalVol = 64;
		for (UINT ienv=0; ienv<12; ienv++)
		{
			penv->VolPoints[ienv] = (WORD)xmsh.venv[ienv*2];
			penv->VolEnv[ienv] = (BYTE)xmsh.venv[ienv*2+1];
			penv->PanPoints[ienv] = (WORD)xmsh.penv[ienv*2];
			penv->PanEnv[ienv] = (BYTE)xmsh.penv[ienv*2+1];
			if (ienv)
			{
				if (penv->VolPoints[ienv] < penv->VolPoints[ienv-1])
				{
					penv->VolPoints[ienv] &= 0xFF;
					penv->VolPoints[ienv] += penv->VolPoints[ienv-1] & 0xFF00;
					if (penv->VolPoints[ienv] < penv->VolPoints[ienv-1]) penv->VolPoints[ienv] += 0x100;
				}
				if (penv->PanPoints[ienv] < penv->PanPoints[ienv-1])
				{
					penv->PanPoints[ienv] &= 0xFF;
					penv->PanPoints[ienv] += penv->PanPoints[ienv-1] & 0xFF00;
					if (penv->PanPoints[ienv] < penv->PanPoints[ienv-1]) penv->PanPoints[ienv] += 0x100;
				}
			}
		}
		for (UINT j=0; j<96; j++)
		{
			penv->NoteMap[j+12] = j+1+12;
			if (xmsh.snum[j] < nsamples)
				penv->Keyboard[j+12] = samplemap[xmsh.snum[j]];
		}
		// Reading samples
		for (UINT ins=0; ins<nsamples; ins++)
		{
			if ((dwMemPos + sizeof(xmss) > dwMemLength)
			 || (xmsh.shsize >= dwMemLength) || (dwMemPos > dwMemLength - xmsh.shsize)) return TRUE;
			memcpy(&xmss, lpStream+dwMemPos, sizeof(xmss));
			xmss.samplen = bswapLE32(xmss.samplen);
			xmss.loopstart = bswapLE32(xmss.loopstart);
			xmss.looplen = bswapLE32(xmss.looplen);
			dwMemPos += xmsh.shsize;
			flags[ins] = (xmss.type & 0x10) ? RS_PCM16D : RS_PCM8D;
			if (xmss.type & 0x20) flags[ins] = (xmss.type & 0x10) ? RS_STPCM16D : RS_STPCM8D;
			samplesize[ins] = xmss.samplen;
			if (!samplemap[ins]) continue;
			if (xmss.type & 0x10)
			{
				xmss.looplen >>= 1;
				xmss.loopstart >>= 1;
				xmss.samplen >>= 1;
			}
			if (xmss.type & 0x20)
			{
				xmss.looplen >>= 1;
				xmss.loopstart >>= 1;
				xmss.samplen >>= 1;
			}
			if (xmss.samplen > MAX_SAMPLE_LENGTH) xmss.samplen = MAX_SAMPLE_LENGTH;
			if (xmss.loopstart >= xmss.samplen) xmss.type &= ~3;
			xmss.looplen += xmss.loopstart;
			if (xmss.looplen > xmss.samplen) xmss.looplen = xmss.samplen;
			if (!xmss.looplen) xmss.type &= ~3;
			UINT imapsmp = samplemap[ins];
			memcpy(m_szNames[imapsmp], xmss.name, 22);
			m_szNames[imapsmp][22] = 0;
			MODINSTRUMENT *pins = &Ins[imapsmp];
			pins->nLength = (xmss.samplen > MAX_SAMPLE_LENGTH) ? MAX_SAMPLE_LENGTH : xmss.samplen;
			pins->nLoopStart = xmss.loopstart;
			pins->nLoopEnd = xmss.looplen;
			if (pins->nLoopEnd > pins->nLength) pins->nLoopEnd = pins->nLength;
			if (pins->nLoopStart >= pins->nLoopEnd)
			{
				pins->nLoopStart = pins->nLoopEnd = 0;
			}
			if (xmss.type & 3) pins->uFlags |= CHN_LOOP;
			if (xmss.type & 2) pins->uFlags |= CHN_PINGPONGLOOP;
			pins->nVolume = xmss.vol << 2;
			if (pins->nVolume > 256) pins->nVolume = 256;
			pins->nGlobalVol = 64;
			if ((xmss.res == 0xAD) && (!(xmss.type & 0x30)))
			{
				flags[ins] = RS_ADPCM4;
				samplesize[ins] = (samplesize[ins]+1)/2 + 16;
			}
			pins->nFineTune = xmss.finetune;
			pins->RelativeTone = (int)xmss.relnote;
			pins->nPan = xmss.pan;
			pins->uFlags |= CHN_PANNING;
			pins->nVibType = xmsh.vibtype;
			pins->nVibSweep = xmsh.vibsweep;
			pins->nVibDepth = xmsh.vibdepth;
			pins->nVibRate = xmsh.vibrate;
			memcpy(pins->name, xmss.name, 22);
			pins->name[21] = 0;
		}
#if 0
		if ((xmsh.reserved2 > nsamples) && (xmsh.reserved2 <= 16))
		{
			dwMemPos += (((UINT)xmsh.reserved2) - nsamples) * xmsh.shsize;
		}
#endif
		for (UINT ismpd=0; ismpd<nsamples; ismpd++)
		{
			if ((samplemap[ismpd]) && (samplesize[ismpd]) && (dwMemPos < dwMemLength))
			{
				ReadSample(&Ins[samplemap[ismpd]], flags[ismpd], (LPSTR)(lpStream + dwMemPos), dwMemLength - dwMemPos);
			}
			dwMemPos += samplesize[ismpd];
			if (dwMemPos >= dwMemLength) break;
		}
	}
	// Read song comments: "TEXT"
	if ((dwMemPos + 8 < dwMemLength) && (bswapLE32(*((DWORD *)(lpStream+dwMemPos))) == 0x74786574))
	{
		UINT len = *((DWORD *)(lpStream+dwMemPos+4));
		dwMemPos += 8;
		if ((dwMemPos + len <= dwMemLength) && (len < 16384))
		{
			m_lpszSongComments = new char[len+1];
			if (m_lpszSongComments)
			{
				memcpy(m_lpszSongComments, lpStream+dwMemPos, len);
				m_lpszSongComments[len] = 0;
			}
			dwMemPos += len;
		}
	}
	// Read midi config: "MIDI"
	if ((dwMemPos + 8 < dwMemLength) && (bswapLE32(*((DWORD *)(lpStream+dwMemPos))) == 0x4944494D))
	{
		UINT len = *((DWORD *)(lpStream+dwMemPos+4));
		dwMemPos += 8;
		if (len >= dwMemLength || dwMemPos > dwMemLength - len) return TRUE;
		if (len == sizeof(MODMIDICFG))
		{
			memcpy(&m_MidiCfg, lpStream+dwMemPos, len);
			m_dwSongFlags |= SONG_EMBEDMIDICFG;
		}
	}
	// Read pattern names: "PNAM"
	if ((dwMemPos + 8 < dwMemLength) && (bswapLE32(*((DWORD *)(lpStream+dwMemPos))) == 0x4d414e50))
	{
		UINT len = *((DWORD *)(lpStream+dwMemPos+4));
		dwMemPos += 8;
		if (len >= dwMemLength || dwMemPos > dwMemLength - len) return TRUE;
		if ((len <= MAX_PATTERNS*MAX_PATTERNNAME) && (len >= MAX_PATTERNNAME))
		{
			m_lpszPatternNames = new char[len];

			if (m_lpszPatternNames)
			{
				m_nPatternNames = len / MAX_PATTERNNAME;
				memcpy(m_lpszPatternNames, lpStream+dwMemPos, len);
			}
			dwMemPos += len;
		}
	}
	// Read channel names: "CNAM"
	if ((dwMemPos + 8 < dwMemLength) && (bswapLE32(*((DWORD *)(lpStream+dwMemPos))) == 0x4d414e43))
	{
		UINT len = *((DWORD *)(lpStream+dwMemPos+4));
		dwMemPos += 8;
		if (len >= dwMemLength || dwMemPos > dwMemLength - len) return TRUE;
		if (len <= MAX_BASECHANNELS*MAX_CHANNELNAME)
		{
			UINT n = len / MAX_CHANNELNAME;
			for (UINT i=0; i<n; i++)
			{
				memcpy(ChnSettings[i].szName, (lpStream+dwMemPos+i*MAX_CHANNELNAME), MAX_CHANNELNAME);
				ChnSettings[i].szName[MAX_CHANNELNAME-1] = 0;
			}
			dwMemPos += len;
		}
	}
	// Read mix plugins information
	if (dwMemPos + 8 < dwMemLength)
	{
		dwMemPos += LoadMixPlugins(lpStream+dwMemPos, dwMemLength-dwMemPos);
	}
	return TRUE;
}


#ifndef MODPLUG_NO_FILESAVE

BOOL CSoundFile::SaveXM(LPCSTR lpszFileName, UINT nPacking)
//---------------------------------------------------------
{
	BYTE s[64*64*5];
	XMFILEHEADER header;
	XMINSTRUMENTHEADER xmih;
	XMSAMPLEHEADER xmsh;
	XMSAMPLESTRUCT xmss;
	BYTE smptable[32];
	BYTE xmph[9];
	FILE *f;
	int i;

	if ((!m_nChannels) || (!lpszFileName)) return FALSE;
	if ((f = fopen(lpszFileName, "wb")) == NULL) return FALSE;
	fwrite("Extended Module: ", 17, 1, f);
	fwrite(m_szNames[0], 20, 1, f);
	s[0] = 0x1A;
	lstrcpy((LPSTR)&s[1], (nPacking) ? "MOD Plugin packed   " : "FastTracker v2.00   ");
	s[21] = 0x04;
	s[22] = 0x01;
	fwrite(s, 23, 1, f);
	// Writing song header
	memset(&header, 0, sizeof(header));
	header.size = sizeof(XMFILEHEADER);
	header.norder = 0;
	header.restartpos = m_nRestartPos;
	header.channels = m_nChannels;
	header.patterns = 0;
	for (i=0; i<MAX_ORDERS; i++)
	{
		if (Order[i] == 0xFF) break;
		header.norder++;
		if ((Order[i] >= header.patterns) && (Order[i] < MAX_PATTERNS)) header.patterns = Order[i]+1;
	}
	header.instruments = m_nInstruments;
	if (!header.instruments) header.instruments = m_nSamples;
	header.flags = (m_dwSongFlags & SONG_LINEARSLIDES) ? 0x01 : 0x00;
	if (m_dwSongFlags & SONG_EXFILTERRANGE) header.flags |= 0x1000;
	header.tempo = m_nDefaultTempo;
	header.speed = m_nDefaultSpeed;
	memcpy(header.order, Order, header.norder);
	fwrite(&header, 1, sizeof(header), f);
	// Writing patterns
	for (i=0; i<header.patterns; i++) if (Patterns[i])
	{
		MODCOMMAND *p = Patterns[i];
		UINT len = 0;

		memset(&xmph, 0, sizeof(xmph));
		xmph[0] = 9;
		xmph[5] = (BYTE)(PatternSize[i] & 0xFF);
		xmph[6] = (BYTE)(PatternSize[i] >> 8);
		for (UINT j=m_nChannels*PatternSize[i]; j; j--,p++)
		{
			UINT note = p->note;
			UINT param = ModSaveCommand(p, TRUE);
			UINT command = param >> 8;
			param &= 0xFF;
			if (note >= 0xFE) note = 97; else
			if ((note <= 12) || (note > 96+12)) note = 0; else
			note -= 12;
			UINT vol = 0;
			if (p->volcmd)
			{
				UINT volcmd = p->volcmd;
				switch(volcmd)
				{
				case VOLCMD_VOLUME:			vol = 0x10 + p->vol; break;
				case VOLCMD_VOLSLIDEDOWN:	vol = 0x60 + (p->vol & 0x0F); break;
				case VOLCMD_VOLSLIDEUP:		vol = 0x70 + (p->vol & 0x0F); break;
				case VOLCMD_FINEVOLDOWN:	vol = 0x80 + (p->vol & 0x0F); break;
				case VOLCMD_FINEVOLUP:		vol = 0x90 + (p->vol & 0x0F); break;
				case VOLCMD_VIBRATOSPEED:	vol = 0xA0 + (p->vol & 0x0F); break;
				case VOLCMD_VIBRATO:		vol = 0xB0 + (p->vol & 0x0F); break;
				case VOLCMD_PANNING:		vol = 0xC0 + (p->vol >> 2); if (vol > 0xCF) vol = 0xCF; break;
				case VOLCMD_PANSLIDELEFT:	vol = 0xD0 + (p->vol & 0x0F); break;
				case VOLCMD_PANSLIDERIGHT:	vol = 0xE0 + (p->vol & 0x0F); break;
				case VOLCMD_TONEPORTAMENTO:	vol = 0xF0 + (p->vol & 0x0F); break;
				}
			}
			if ((note) && (p->instr) && (vol > 0x0F) && (command) && (param))
			{
				s[len++] = note;
				s[len++] = p->instr;
				s[len++] = vol;
				s[len++] = command;
				s[len++] = param;
			} else
			{
				BYTE b = 0x80;
				if (note) b |= 0x01;
				if (p->instr) b |= 0x02;
				if (vol >= 0x10) b |= 0x04;
				if (command) b |= 0x08;
				if (param) b |= 0x10;
				s[len++] = b;
				if (b & 1) s[len++] = note;
				if (b & 2) s[len++] = p->instr;
				if (b & 4) s[len++] = vol;
				if (b & 8) s[len++] = command;
				if (b & 16) s[len++] = param;
			}
			if (len > sizeof(s) - 5) break;
		}
		xmph[7] = (BYTE)(len & 0xFF);
		xmph[8] = (BYTE)(len >> 8);
		fwrite(xmph, 1, 9, f);
		fwrite(s, 1, len, f);
	} else
	{
		memset(&xmph, 0, sizeof(xmph));
		xmph[0] = 9;
		xmph[5] = (BYTE)(PatternSize[i] & 0xFF);
		xmph[6] = (BYTE)(PatternSize[i] >> 8);
		fwrite(xmph, 1, 9, f);
	}
	// Writing instruments
	for (i=1; i<=header.instruments; i++)
	{
		MODINSTRUMENT *pins;
		BYTE flags[32];

		memset(&xmih, 0, sizeof(xmih));
		memset(&xmsh, 0, sizeof(xmsh));
		xmih.size = sizeof(xmih) + sizeof(xmsh);
		memcpy(xmih.name, m_szNames[i], 22);
		xmih.type = 0;
		xmih.samples = 0;
		if (m_nInstruments)
		{
			INSTRUMENTHEADER *penv = Headers[i];
			if (penv)
			{
				memcpy(xmih.name, penv->name, 22);
				xmih.type = penv->nMidiProgram;
				xmsh.volfade = penv->nFadeOut;
				xmsh.vnum = (BYTE)penv->nVolEnv;
				xmsh.pnum = (BYTE)penv->nPanEnv;
				if (xmsh.vnum > 12) xmsh.vnum = 12;
				if (xmsh.pnum > 12) xmsh.pnum = 12;
				for (UINT ienv=0; ienv<12; ienv++)
				{
					xmsh.venv[ienv*2] = penv->VolPoints[ienv];
					xmsh.venv[ienv*2+1] = penv->VolEnv[ienv];
					xmsh.penv[ienv*2] = penv->PanPoints[ienv];
					xmsh.penv[ienv*2+1] = penv->PanEnv[ienv];
				}
				if (penv->dwFlags & ENV_VOLUME) xmsh.vtype |= 1;
				if (penv->dwFlags & ENV_VOLSUSTAIN) xmsh.vtype |= 2;
				if (penv->dwFlags & ENV_VOLLOOP) xmsh.vtype |= 4;
				if (penv->dwFlags & ENV_PANNING) xmsh.ptype |= 1;
				if (penv->dwFlags & ENV_PANSUSTAIN) xmsh.ptype |= 2;
				if (penv->dwFlags & ENV_PANLOOP) xmsh.ptype |= 4;
				xmsh.vsustain = (BYTE)penv->nVolSustainBegin;
				xmsh.vloops = (BYTE)penv->nVolLoopStart;
				xmsh.vloope = (BYTE)penv->nVolLoopEnd;
				xmsh.psustain = (BYTE)penv->nPanSustainBegin;
				xmsh.ploops = (BYTE)penv->nPanLoopStart;
				xmsh.ploope = (BYTE)penv->nPanLoopEnd;
				for (UINT j=0; j<96; j++) if (penv->Keyboard[j+12])
				{
					UINT k;
					for (k=0; k<xmih.samples; k++)	if (smptable[k] == penv->Keyboard[j+12]) break;
					if (k == xmih.samples)
					{
						smptable[xmih.samples++] = penv->Keyboard[j+12];
					}
					if (xmih.samples >= 32) break;
					xmsh.snum[j] = k;
				}
//				xmsh.reserved2 = xmih.samples;
			}
		} else
		{
			xmih.samples = 1;
//			xmsh.reserved2 = 1;
			smptable[0] = i;
		}
		xmsh.shsize = (xmih.samples) ? 40 : 0;
		fwrite(&xmih, 1, sizeof(xmih), f);
		if (smptable[0])
		{
			MODINSTRUMENT *pvib = &Ins[smptable[0]];
			xmsh.vibtype = pvib->nVibType;
			xmsh.vibsweep = pvib->nVibSweep;
			xmsh.vibdepth = pvib->nVibDepth;
			xmsh.vibrate = pvib->nVibRate;
		}
		fwrite(&xmsh, 1, xmih.size - sizeof(xmih), f);
		if (!xmih.samples) continue;
		for (UINT ins=0; ins<xmih.samples; ins++)
		{
			memset(&xmss, 0, sizeof(xmss));
			if (smptable[ins]) memcpy(xmss.name, m_szNames[smptable[ins]], 22);
			pins = &Ins[smptable[ins]];
			xmss.samplen = pins->nLength;
			xmss.loopstart = pins->nLoopStart;
			xmss.looplen = pins->nLoopEnd - pins->nLoopStart;
			xmss.vol = pins->nVolume / 4;
			xmss.finetune = (char)pins->nFineTune;
			xmss.type = 0;
			if (pins->uFlags & CHN_LOOP) xmss.type = (pins->uFlags & CHN_PINGPONGLOOP) ? 2 : 1;
			flags[ins] = RS_PCM8D;
#ifndef NO_PACKING
			if (nPacking)
			{
				if ((!(pins->uFlags & (CHN_16BIT|CHN_STEREO)))
				 && (CanPackSample((char *)pins->pSample, pins->nLength, nPacking)))
				{
					flags[ins] = RS_ADPCM4;
					xmss.res = 0xAD;
				}
			} else
#endif
			{
				if (pins->uFlags & CHN_16BIT)
				{
					flags[ins] = RS_PCM16D;
					xmss.type |= 0x10;
					xmss.looplen *= 2;
					xmss.loopstart *= 2;
					xmss.samplen *= 2;
				}
				if (pins->uFlags & CHN_STEREO)
				{
					flags[ins] = (pins->uFlags & CHN_16BIT) ? RS_STPCM16D : RS_STPCM8D;
					xmss.type |= 0x20;
					xmss.looplen *= 2;
					xmss.loopstart *= 2;
					xmss.samplen *= 2;
				}
			}
			xmss.pan = 255;
			if (pins->nPan < 256) xmss.pan = (BYTE)pins->nPan;
			xmss.relnote = (signed char)pins->RelativeTone;
			fwrite(&xmss, 1, xmsh.shsize, f);
		}
		for (UINT ismpd=0; ismpd<xmih.samples; ismpd++)
		{
			pins = &Ins[smptable[ismpd]];
			if (pins->pSample)
			{
#ifndef NO_PACKING
				if ((flags[ismpd] == RS_ADPCM4) && (xmih.samples>1)) CanPackSample((char *)pins->pSample, pins->nLength, nPacking);
#endif // NO_PACKING
				WriteSample(f, pins, flags[ismpd]);
			}
		}
	}
	// Writing song comments
	if ((m_lpszSongComments) && (m_lpszSongComments[0]))
	{
		DWORD d = 0x74786574;
		fwrite(&d, 1, 4, f);
		d = strlen(m_lpszSongComments);
		fwrite(&d, 1, 4, f);
		fwrite(m_lpszSongComments, 1, d, f);
	}
	// Writing midi cfg
	if (m_dwSongFlags & SONG_EMBEDMIDICFG)
	{
		DWORD d = 0x4944494D;
		fwrite(&d, 1, 4, f);
		d = sizeof(MODMIDICFG);
		fwrite(&d, 1, 4, f);
		fwrite(&m_MidiCfg, 1, sizeof(MODMIDICFG), f);
	}
	// Writing Pattern Names
	if ((m_nPatternNames) && (m_lpszPatternNames))
	{
		DWORD dwLen = m_nPatternNames * MAX_PATTERNNAME;
		while ((dwLen >= MAX_PATTERNNAME) && (!m_lpszPatternNames[dwLen-MAX_PATTERNNAME])) dwLen -= MAX_PATTERNNAME;
		if (dwLen >= MAX_PATTERNNAME)
		{
			DWORD d = 0x4d414e50;
			fwrite(&d, 1, 4, f);
			fwrite(&dwLen, 1, 4, f);
			fwrite(m_lpszPatternNames, 1, dwLen, f);
		}
	}
	// Writing Channel Names
	{
		UINT nChnNames = 0;
		for (UINT inam=0; inam<m_nChannels; inam++)
		{
			if (ChnSettings[inam].szName[0]) nChnNames = inam+1;
		}
		// Do it!
		if (nChnNames)
		{
			DWORD dwLen = nChnNames * MAX_CHANNELNAME;
			DWORD d = 0x4d414e43;
			fwrite(&d, 1, 4, f);
			fwrite(&dwLen, 1, 4, f);
			for (UINT inam=0; inam<nChnNames; inam++)
			{
				fwrite(ChnSettings[inam].szName, 1, MAX_CHANNELNAME, f);
			}
		}
	}
	// Save mix plugins information
	SaveMixPlugins(f);
	fclose(f);
	return TRUE;
}

#endif // MODPLUG_NO_FILESAVE
/*
 * This source code is public domain.
 *
 * Handles unpacking of Powerpack PP20
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

//#include "stdafx.h"
//#include "sndfile.h"

BOOL PP20_Unpack(LPCBYTE *ppMemFile, LPDWORD pdwMemLength);

#pragma pack(1)
typedef struct MMCMPFILEHEADER
{
	char id[8]; /* string 'ziRCONia' */
	WORD hdrsize; /* sizeof MMCMPHEADER */
} MMCMPFILEHEADER, *LPMMCMPFILEHEADER;

typedef struct MMCMPHEADER
{
	WORD version;
	WORD nblocks;
	DWORD filesize;
	DWORD blktable;
	BYTE glb_comp;
	BYTE fmt_comp;
} MMCMPHEADER, *LPMMCMPHEADER;

typedef struct MMCMPBLOCK
{
	DWORD unpk_size;
	DWORD pk_size;
	DWORD xor_chk;
	WORD sub_blk;
	WORD flags;
	WORD tt_entries;
	USHORT num_bits;
} MMCMPBLOCK, *LPMMCMPBLOCK;

typedef struct MMCMPSUBBLOCK
{
	DWORD unpk_pos;
	DWORD unpk_size;
} MMCMPSUBBLOCK, *LPMMCMPSUBBLOCK;
#pragma pack()

/* make sure of structure sizes */
typedef int chk_MMCMPFILEHEADER[(sizeof(struct MMCMPFILEHEADER) == 10) * 2 - 1];
typedef int chk_MMCMPHEADER[(sizeof(struct MMCMPHEADER) == 14) * 2 - 1];
typedef int chk_MMCMPBLOCK[(sizeof(struct MMCMPBLOCK) == 20) * 2 - 1];
typedef int chk_MMCMPSUBBLOCK[(sizeof(struct MMCMPSUBBLOCK) == 8) * 2 - 1];

#define MMCMP_COMP		0x0001
#define MMCMP_DELTA		0x0002
#define MMCMP_16BIT		0x0004
#define MMCMP_STEREO	0x0100
#define MMCMP_ABS16		0x0200
#define MMCMP_ENDIAN	0x0400

typedef struct MMCMPBITBUFFER
{
	UINT bitcount;
	DWORD bitbuffer;
	LPCBYTE pSrc;
	LPCBYTE pEnd;

	DWORD GetBits(UINT nBits);
} MMCMPBITBUFFER;


DWORD MMCMPBITBUFFER::GetBits(UINT nBits)
{
	DWORD d;
	if (!nBits) return 0;
	while (bitcount < 24)
	{
		bitbuffer |= ((pSrc < pEnd) ? *pSrc++ : 0) << bitcount;
		bitcount += 8;
	}
	d = bitbuffer & ((1 << nBits) - 1);
	bitbuffer >>= nBits;
	bitcount -= nBits;
	return d;
}

//#define MMCMP_LOG

#ifdef MMCMP_LOG
extern void Log(LPCSTR s, ...);
#endif

static const DWORD MMCMP8BitCommands[8] =
{
	0x01, 0x03,	0x07, 0x0F,	0x1E, 0x3C,	0x78, 0xF8
};

static const UINT MMCMP8BitFetch[8] =
{
	3, 3, 3, 3, 2, 1, 0, 0
};

static const DWORD MMCMP16BitCommands[16] =
{
	0x01, 0x03,	0x07, 0x0F,	0x1E, 0x3C,	0x78, 0xF0,
	0x1F0, 0x3F0, 0x7F0, 0xFF0, 0x1FF0, 0x3FF0, 0x7FF0, 0xFFF0
};

static const UINT MMCMP16BitFetch[16] =
{
	4, 4, 4, 4, 3, 2, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};


static void swap_mfh(LPMMCMPFILEHEADER fh)
{
	fh->hdrsize = bswapLE16(fh->hdrsize);
}

static void swap_mmh(LPMMCMPHEADER mh)
{
	mh->version = bswapLE16(mh->version);
	mh->nblocks = bswapLE16(mh->nblocks);
	mh->filesize = bswapLE32(mh->filesize);
	mh->blktable = bswapLE32(mh->blktable);
}

static void swap_block (LPMMCMPBLOCK blk)
{
	blk->unpk_size = bswapLE32(blk->unpk_size);
	blk->pk_size = bswapLE32(blk->pk_size);
	blk->xor_chk = bswapLE32(blk->xor_chk);
	blk->sub_blk = bswapLE16(blk->sub_blk);
	blk->flags = bswapLE16(blk->flags);
	blk->tt_entries = bswapLE16(blk->tt_entries);
	blk->num_bits = bswapLE16(blk->num_bits);
}

static void swap_subblock (LPMMCMPSUBBLOCK sblk)
{
	sblk->unpk_pos = bswapLE32(sblk->unpk_pos);
	sblk->unpk_size = bswapLE32(sblk->unpk_size);
}

static BOOL MMCMP_IsDstBlockValid(const MMCMPSUBBLOCK *psub, DWORD dstlen)
{
	if (psub->unpk_pos >= dstlen) return FALSE;
	if (psub->unpk_size > dstlen) return FALSE;
	if (psub->unpk_size > dstlen - psub->unpk_pos) return FALSE;
	return TRUE;
}


BOOL MMCMP_Unpack(LPCBYTE *ppMemFile, LPDWORD pdwMemLength)
{
	DWORD dwMemLength;
	LPCBYTE lpMemFile;
	LPBYTE pBuffer,pBufEnd;
	LPMMCMPFILEHEADER pmfh;
	LPMMCMPHEADER pmmh;
	const DWORD *pblk_table;
	DWORD dwFileSize;
	BYTE tmp0[32], tmp1[32];

	if (PP20_Unpack(ppMemFile, pdwMemLength))
	{
		return TRUE;
	}

	dwMemLength = *pdwMemLength;
	lpMemFile = *ppMemFile;
	if ((dwMemLength < 256) || (!lpMemFile)) return FALSE;
	memcpy(tmp0, lpMemFile, 24);
	pmfh = (LPMMCMPFILEHEADER)(tmp0);
	pmmh = (LPMMCMPHEADER)(tmp0+10);
	swap_mfh(pmfh);
	swap_mmh(pmmh);

	if ((memcmp(pmfh->id,"ziRCONia",8) != 0) || (pmfh->hdrsize != 14))
		return FALSE;
	if ((!pmmh->nblocks) || (pmmh->filesize < 16) || (pmmh->filesize > 0x8000000) ||
	    (pmmh->blktable >= dwMemLength) || (pmmh->blktable + 4*pmmh->nblocks > dwMemLength)) {
		return FALSE;
	}
	dwFileSize = pmmh->filesize;
	if ((pBuffer = (LPBYTE)calloc(1, (dwFileSize + 31) & ~15)) == NULL)
		return FALSE;
	pBufEnd = pBuffer + dwFileSize;
	pblk_table = (const DWORD *)(lpMemFile+pmmh->blktable);
	for (UINT nBlock=0; nBlock<pmmh->nblocks; nBlock++)
	{
		DWORD dwMemPos = bswapLE32(pblk_table[nBlock]);
		DWORD dwSubPos;
		LPMMCMPBLOCK pblk;
		LPMMCMPSUBBLOCK psubblk;

		if (dwMemPos >= dwMemLength - 20)
			goto err;
		memcpy(tmp1,lpMemFile+dwMemPos,28);
		pblk = (LPMMCMPBLOCK)(tmp1);
		psubblk = (LPMMCMPSUBBLOCK)(tmp1+20);
		swap_block(pblk);
		swap_subblock(psubblk);

		if (!pblk->unpk_size || !pblk->pk_size || !pblk->sub_blk)
			goto err;
		if (pblk->pk_size <= pblk->tt_entries)
			goto err;
		if (pblk->sub_blk*8 >= dwMemLength - dwMemPos - 20)
			goto err;
		if (pblk->flags & MMCMP_COMP) {
			if (pblk->flags & MMCMP_16BIT) {
				if (pblk->num_bits >= 16)
					goto err;
			}
			else {
				if (pblk->num_bits >=  8)
					goto err;
			}
		}

		dwSubPos = dwMemPos + 20;
		dwMemPos += 20 + pblk->sub_blk*8;
#ifdef MMCMP_LOG
		Log("block %d: flags=%04X sub_blocks=%d", nBlock, (UINT)pblk->flags, (UINT)pblk->sub_blk);
		Log(" pksize=%d unpksize=%d", pblk->pk_size, pblk->unpk_size);
		Log(" tt_entries=%d num_bits=%d\n", pblk->tt_entries, pblk->num_bits);
#endif
		if (!(pblk->flags & MMCMP_COMP))
		{ /* Data is not packed */
			UINT i=0;
			while (1) {
#ifdef MMCMP_LOG
				Log("  Unpacked sub-block %d: offset %d, size=%d\n", i, psubblk->unpk_pos, psubblk->unpk_size);
#endif
				if (!MMCMP_IsDstBlockValid(psubblk, dwFileSize))
					goto err;
				memcpy(pBuffer+psubblk->unpk_pos, lpMemFile+dwMemPos, psubblk->unpk_size);
				dwMemPos += psubblk->unpk_size;
				if (++i == pblk->sub_blk) break;
				memcpy(tmp1+20,lpMemFile+dwSubPos+i*8,8);
				swap_subblock(psubblk);
			}
		}
		else if (pblk->flags & MMCMP_16BIT)
		{ /* Data is 16-bit packed */
			MMCMPBITBUFFER bb;
			LPBYTE pDest = pBuffer + psubblk->unpk_pos;
			DWORD dwSize = psubblk->unpk_size;
			DWORD dwPos = 0;
			UINT numbits = pblk->num_bits;
			UINT subblk = 0, oldval = 0;

#ifdef MMCMP_LOG
			Log("  16-bit block: pos=%d size=%d ", psubblk->unpk_pos, psubblk->unpk_size);
			if (pblk->flags & MMCMP_DELTA) Log("DELTA ");
			if (pblk->flags & MMCMP_ABS16) Log("ABS16 ");
			Log("\n");
#endif
			if (!MMCMP_IsDstBlockValid(psubblk, dwFileSize))
				goto err;
			bb.bitcount = 0;
			bb.bitbuffer = 0;
			bb.pSrc = lpMemFile+dwMemPos+pblk->tt_entries;
			bb.pEnd = lpMemFile+dwMemPos+pblk->pk_size;
			while (1)
			{
				UINT newval = 0x10000;
				DWORD d = bb.GetBits(numbits+1);

				if (d >= MMCMP16BitCommands[numbits])
				{
					UINT nFetch = MMCMP16BitFetch[numbits];
					UINT newbits = bb.GetBits(nFetch) + ((d - MMCMP16BitCommands[numbits]) << nFetch);
					if (newbits != numbits)
					{
						numbits = newbits & 0x0F;
					} else
					{
						if ((d = bb.GetBits(4)) == 0x0F)
						{
							if (bb.GetBits(1)) break;
							newval = 0xFFFF;
						} else
						{
							newval = 0xFFF0 + d;
						}
					}
				} else
				{
					newval = d;
				}
				if (newval < 0x10000)
				{
					newval = (newval & 1) ? (UINT)(-(LONG)((newval+1) >> 1)) : (UINT)(newval >> 1);
					if (pblk->flags & MMCMP_DELTA)
					{
						newval += oldval;
						oldval = newval;
					} else
					if (!(pblk->flags & MMCMP_ABS16))
					{
						newval ^= 0x8000;
					}
					if (pBufEnd - pDest < 2) goto err;
					dwPos += 2;
					*pDest++ = (BYTE) (((WORD)newval) & 0xff);
					*pDest++ = (BYTE) (((WORD)newval) >> 8);
				}
				if (dwPos >= dwSize)
				{
					if (++subblk == pblk->sub_blk) break;
					dwPos = 0;
					memcpy(tmp1+20,lpMemFile+dwSubPos+subblk*8,8);
					swap_subblock(psubblk);
					if (!MMCMP_IsDstBlockValid(psubblk, dwFileSize))
						goto err;
					dwSize = psubblk->unpk_size;
					pDest = pBuffer + psubblk->unpk_pos;
				}
			}
		}
		else
		{ /* Data is 8-bit packed */
			MMCMPBITBUFFER bb;
			LPBYTE pDest = pBuffer + psubblk->unpk_pos;
			DWORD dwSize = psubblk->unpk_size;
			DWORD dwPos = 0;
			UINT numbits = pblk->num_bits;
			UINT subblk = 0, oldval = 0;
			LPCBYTE ptable = lpMemFile+dwMemPos;

			if (!MMCMP_IsDstBlockValid(psubblk, dwFileSize))
				goto err;
			bb.bitcount = 0;
			bb.bitbuffer = 0;
			bb.pSrc = lpMemFile+dwMemPos+pblk->tt_entries;
			bb.pEnd = lpMemFile+dwMemPos+pblk->pk_size;
			while (1)
			{
				UINT newval = 0x100;
				DWORD d = bb.GetBits(numbits+1);

				if (d >= MMCMP8BitCommands[numbits])
				{
					UINT nFetch = MMCMP8BitFetch[numbits];
					UINT newbits = bb.GetBits(nFetch) + ((d - MMCMP8BitCommands[numbits]) << nFetch);
					if (newbits != numbits)
					{
						numbits = newbits & 0x07;
					} else
					{
						if ((d = bb.GetBits(3)) == 7)
						{
							if (bb.GetBits(1)) break;
							newval = 0xFF;
						} else
						{
							newval = 0xF8 + d;
						}
					}
				} else
				{
					newval = d;
				}
				if (newval < 0x100)
				{
					int n = ptable[newval];
					if (pblk->flags & MMCMP_DELTA)
					{
						n += oldval;
						oldval = n;
					}
					pDest[dwPos++] = (BYTE)n;
				}
				if (dwPos >= dwSize)
				{
					if (++subblk == pblk->sub_blk) break;
					dwPos = 0;
					memcpy(tmp1+20,lpMemFile+dwSubPos+subblk*8,8);
					swap_subblock(psubblk);
					if (!MMCMP_IsDstBlockValid(psubblk, dwFileSize))
						goto err;
					dwSize = psubblk->unpk_size;
					pDest = pBuffer + psubblk->unpk_pos;
				}
			}
		}
	}
	*ppMemFile = pBuffer;
	*pdwMemLength = dwFileSize;
	return TRUE;

  err:
	free(pBuffer);
	return FALSE;
}


/* PowerPack PP20 Unpacker */

/* Code from Heikki Orsila's amigadepack 0.02
 * based on code by Stuart Caie <kyzer@4u.net>
 * This software is in the Public Domain
 *
 * Modified for xmp by Claudio Matsuoka, 08/2007
 * - merged mld's checks from the old depack sources. Original credits:
 *   - corrupt file and data detection
 *     (thanks to Don Adan and Dirk Stoecker for help and infos)
 *   - implemeted "efficiency" checks
 *   - further detection based on code by Georg Hoermann
 *
 * Modified for xmp by Claudio Matsuoka, 05/2013
 * - decryption code removed
 *
 * Modified for libmodplug by O. Sezer, Apr. 2015
 */

#define PP_READ_BITS(nbits, var) do {                          \
  bit_cnt = (nbits);                                           \
  while (bits_left < bit_cnt) {                                \
    if (buf_src <= src) return 0; /* out of source bits */     \
    bit_buffer |= (*--buf_src << bits_left);                   \
    bits_left += 8;                                            \
  }                                                            \
  (var) = 0;                                                   \
  bits_left -= bit_cnt;                                        \
  while (bit_cnt--) {                                          \
    (var) = ((var) << 1) | (bit_buffer & 1);                   \
    bit_buffer >>= 1;                                          \
  }                                                            \
} while(0)

#define PP_BYTE_OUT(byte) do {                                 \
  if (out <= dest) return 0; /* output overflow */             \
  *--out = (byte);                                             \
  written++;                                                   \
} while (0)

static BOOL ppDecrunch(LPCBYTE src, LPBYTE dest,
                       LPCBYTE offset_lens,
                       DWORD src_len, DWORD dest_len,
                       BYTE skip_bits)
{
  DWORD bit_buffer, x, todo, offbits, offset, written;
  LPCBYTE buf_src;
  LPBYTE out, dest_end;
  BYTE bits_left, bit_cnt;

  /* set up input and output pointers */
  buf_src = src + src_len;
  out = dest_end = dest + dest_len;

  written = 0;
  bit_buffer = 0;
  bits_left = 0;

  /* skip the first few bits */
  PP_READ_BITS(skip_bits, x);

  /* while there are input bits left */
  while (written < dest_len) {
    PP_READ_BITS(1, x);
    if (x == 0) {
      /* 1bit==0: literal, then match. 1bit==1: just match */
      todo = 1; do { PP_READ_BITS(2, x); todo += x; } while (x == 3);
      while (todo--) { PP_READ_BITS(8, x); PP_BYTE_OUT(x); }

      /* should we end decoding on a literal, break out of the main loop */
      if (written == dest_len) break;
    }

    /* match: read 2 bits for initial offset bitlength / match length */
    PP_READ_BITS(2, x);
    offbits = offset_lens[x];
    todo = x+2;
    if (x == 3) {
      PP_READ_BITS(1, x);
      if (x==0) offbits = 7;
      PP_READ_BITS(offbits, offset);
      do { PP_READ_BITS(3, x); todo += x; } while (x == 7);
    }
    else {
      PP_READ_BITS(offbits, offset);
    }
    if ((out + offset) >= dest_end) return 0; /* match overflow */
    while (todo--) { x = out[offset]; PP_BYTE_OUT(x); }
  }

  /* all output bytes written without error */
  return 1;
  /* return (src == buf_src) ? 1 : 0; */
}

BOOL PP20_Unpack(LPCBYTE *ppMemFile, LPDWORD pdwMemLength)
{
	DWORD dwMemLength = *pdwMemLength;
	LPCBYTE lpMemFile = *ppMemFile;
	DWORD dwDstLen;
	BYTE tmp[4], skip;
	LPBYTE pBuffer;

	if ((!lpMemFile) || (dwMemLength < 256) || (memcmp(lpMemFile,"PP20",4) != 0))
		return FALSE;
	if (dwMemLength & 3) /* file length should be a multiple of 4 */
		return FALSE;

	/* PP FORMAT:
	 *      1 longword identifier           'PP20' or 'PX20'
	 *     [1 word checksum (if 'PX20')     $ssss]
	 *      1 longword efficiency           $eeeeeeee
	 *      X longwords crunched file       $cccccccc,$cccccccc,...
	 *      1 longword decrunch info        'decrlen' << 8 | '8 bits other info'
	 */

	memcpy(tmp,&lpMemFile[dwMemLength-4],4);
	dwDstLen = (tmp[0]<<16) | (tmp[1]<<8) | tmp[2];
	skip = tmp[3];
	if (skip > 32) return 0;

	/* original pp20 only support efficiency
	 * from 9 9 9 9 up to 9 10 12 13, afaik,
	 * but the xfd detection code says this...
	 *
	 * move.l 4(a0),d0
	 * cmp.b #9,d0
	 * blo.b .Exit
	 * and.l #$f0f0f0f0,d0
	 * bne.s .Exit
	 */
	memcpy(tmp,&lpMemFile[4],4);
	if ((tmp[0] < 9) || (tmp[0] & 0xf0)) return FALSE;
	if ((tmp[1] < 9) || (tmp[1] & 0xf0)) return FALSE;
	if ((tmp[2] < 9) || (tmp[2] & 0xf0)) return FALSE;
	if ((tmp[3] < 9) || (tmp[3] & 0xf0)) return FALSE;

	//Log("PP20 detected: Packed length=%d, Unpacked length=%d\n", dwMemLength, dwDstLen);
	if ((dwDstLen < 512) || (dwDstLen > 0x400000) || (dwDstLen > 16*dwMemLength))
		return FALSE;
	if ((pBuffer = (LPBYTE)calloc(1, (dwDstLen + 31) & ~15)) == NULL)
		return FALSE;

	if (!ppDecrunch(lpMemFile+8, pBuffer, tmp, dwMemLength-12, dwDstLen, skip)) {
		free(pBuffer);
		return FALSE;
	}

	*ppMemFile = pBuffer;
	*pdwMemLength = dwDstLen;
	return TRUE;
}

/*
 * This source code is public domain.
 *
 * Authors: Kenton Varda <temporal@gauge3d.org> (C interface wrapper)
 */

//#include "stdafx.h"
//#include "modplug.h"
//#include "sndfile.h"

struct _ModPlugFile
{
	CSoundFile mSoundFile;
};

namespace ModPlug
{
	ModPlug_Settings gSettings =
	{
		MODPLUG_ENABLE_OVERSAMPLING | MODPLUG_ENABLE_NOISE_REDUCTION,

		2, // mChannels
		16, // mBits
		44100, // mFrequency
		MODPLUG_RESAMPLE_LINEAR, //mResamplingMode

		128, // mStereoSeparation
		32, // mMaxMixChannels
		0,
		0,
		0,
		0,
		0,
		0,
		0
	};

	int gSampleSize;

	void UpdateSettings(bool updateBasicConfig)
	{
		if(gSettings.mFlags & MODPLUG_ENABLE_REVERB)
		{
			CSoundFile::SetReverbParameters(gSettings.mReverbDepth,
			                                gSettings.mReverbDelay);
		}

		if(gSettings.mFlags & MODPLUG_ENABLE_MEGABASS)
		{
			CSoundFile::SetXBassParameters(gSettings.mBassAmount,
			                               gSettings.mBassRange);
		}
		else // modplug seems to ignore the SetWaveConfigEx() setting for bass boost
			CSoundFile::SetXBassParameters(0, 0);

		if(gSettings.mFlags & MODPLUG_ENABLE_SURROUND)
		{
			CSoundFile::SetSurroundParameters(gSettings.mSurroundDepth,
			                                  gSettings.mSurroundDelay);
		}

		if(updateBasicConfig)
		{
			CSoundFile::SetWaveConfig(gSettings.mFrequency,
                                                  gSettings.mBits,
			                          gSettings.mChannels);
			CSoundFile::SetMixConfig(gSettings.mStereoSeparation,
                                                 gSettings.mMaxMixChannels);

			gSampleSize = gSettings.mBits / 8 * gSettings.mChannels;
		}

		CSoundFile::SetWaveConfigEx(gSettings.mFlags & MODPLUG_ENABLE_SURROUND,
		                            !(gSettings.mFlags & MODPLUG_ENABLE_OVERSAMPLING),
		                            gSettings.mFlags & MODPLUG_ENABLE_REVERB,
		                            true,
		                            gSettings.mFlags & MODPLUG_ENABLE_MEGABASS,
		                            gSettings.mFlags & MODPLUG_ENABLE_NOISE_REDUCTION,
		                            false);
		CSoundFile::SetResamplingMode(gSettings.mResamplingMode);
	}
}

ModPlugFile* ModPlug_Load(const void* data, int size)
{
	ModPlugFile* result = new ModPlugFile;
	ModPlug::UpdateSettings(true);
	if(result->mSoundFile.Create((const BYTE*)data, size))
	{
		result->mSoundFile.SetRepeatCount(ModPlug::gSettings.mLoopCount);
		return result;
	}
	else
	{
		delete result;
		return NULL;
	}
}

void ModPlug_Unload(ModPlugFile* file)
{
	file->mSoundFile.Destroy();
	delete file;
}

int ModPlug_Read(ModPlugFile* file, void* buffer, int size)
{
	return file->mSoundFile.Read(buffer, size) * ModPlug::gSampleSize;
}

const char* ModPlug_GetName(ModPlugFile* file)
{
	return file->mSoundFile.GetTitle();
}

int ModPlug_GetLength(ModPlugFile* file)
{
	return file->mSoundFile.GetSongTime() * 1000;
}

void ModPlug_InitMixerCallback(ModPlugFile* file,ModPlugMixerProc proc)
{
	file->mSoundFile.gpSndMixHook = (LPSNDMIXHOOKPROC)proc ;
	return;
}

void ModPlug_UnloadMixerCallback(ModPlugFile* file)
{
	file->mSoundFile.gpSndMixHook = NULL;
	return ;
}

unsigned int ModPlug_GetMasterVolume(ModPlugFile* file)
{
	return (unsigned int)file->mSoundFile.m_nMasterVolume;
}

void ModPlug_SetMasterVolume(ModPlugFile* file,unsigned int cvol)
{
	(void)file->mSoundFile.SetMasterVolume( (UINT)cvol,
						FALSE );
	return ;
}

int ModPlug_GetCurrentSpeed(ModPlugFile* file)
{
	return file->mSoundFile.m_nMusicSpeed;
}

int ModPlug_GetCurrentTempo(ModPlugFile* file)
{
	return file->mSoundFile.m_nMusicTempo;
}

int ModPlug_GetCurrentOrder(ModPlugFile* file)
{
	return file->mSoundFile.GetCurrentOrder();
}

int ModPlug_GetCurrentPattern(ModPlugFile* file)
{
	return file->mSoundFile.GetCurrentPattern();
}

int ModPlug_GetCurrentRow(ModPlugFile* file)
{
	return file->mSoundFile.m_nRow;
}

int ModPlug_GetPlayingChannels(ModPlugFile* file)
{
	return ( file->mSoundFile.m_nMixChannels < file->mSoundFile.m_nMaxMixChannels ? file->mSoundFile.m_nMixChannels : file->mSoundFile.m_nMaxMixChannels );
}

void ModPlug_SeekOrder(ModPlugFile* file,int order)
{
	file->mSoundFile.SetCurrentOrder(order);
}

int ModPlug_GetModuleType(ModPlugFile* file)
{
	return file->mSoundFile.m_nType;
}

char* ModPlug_GetMessage(ModPlugFile* file)
{
	return file->mSoundFile.m_lpszSongComments;
}

#ifndef MODPLUG_NO_FILESAVE
char ModPlug_ExportS3M(ModPlugFile* file,const char* filepath)
{
	return (char)file->mSoundFile.SaveS3M(filepath,0);
}

char ModPlug_ExportXM(ModPlugFile* file,const char* filepath)
{
	return (char)file->mSoundFile.SaveXM(filepath,0);
}

char ModPlug_ExportMOD(ModPlugFile* file,const char* filepath)
{
	return (char)file->mSoundFile.SaveMod(filepath,0);
}

char ModPlug_ExportIT(ModPlugFile* file,const char* filepath)
{
	return (char)file->mSoundFile.SaveIT(filepath,0);
}
#endif // MODPLUG_NO_FILESAVE

unsigned int ModPlug_NumInstruments(ModPlugFile* file)
{
	return file->mSoundFile.m_nInstruments;
}

unsigned int ModPlug_NumSamples(ModPlugFile* file)
{
	return file->mSoundFile.m_nSamples;
}

unsigned int ModPlug_NumPatterns(ModPlugFile* file)
{
	return file->mSoundFile.GetNumPatterns();
}

unsigned int ModPlug_NumChannels(ModPlugFile* file)
{
	return file->mSoundFile.GetNumChannels();
}

unsigned int ModPlug_SampleName(ModPlugFile* file,unsigned int qual,char* buff)
{
	return file->mSoundFile.GetSampleName(qual,buff);
}

unsigned int ModPlug_InstrumentName(ModPlugFile* file,unsigned int qual,char* buff)
{
	return file->mSoundFile.GetInstrumentName(qual,buff);
}

ModPlugNote* ModPlug_GetPattern(ModPlugFile* file,int pattern,unsigned int* numrows) {
	if ( pattern<MAX_PATTERNS && pattern >= 0) {
		if (file->mSoundFile.Patterns[pattern]) {
			if (numrows) *numrows=(unsigned int)file->mSoundFile.PatternSize[pattern];
			return (ModPlugNote*)file->mSoundFile.Patterns[pattern];
		}
	}
	return NULL;
}

void ModPlug_Seek(ModPlugFile* file, int millisecond)
{
	int maxpos;
	int maxtime = file->mSoundFile.GetSongTime() * 1000;
	float postime;

	if(millisecond > maxtime)
		millisecond = maxtime;
	maxpos = file->mSoundFile.GetMaxPosition();
	postime = 0.0f;
	if (maxtime != 0)
		postime = (float)maxpos / (float)maxtime;

	file->mSoundFile.SetCurrentPos((int)(millisecond * postime));
}

void ModPlug_GetSettings(ModPlug_Settings* settings)
{
	memcpy(settings, &ModPlug::gSettings, sizeof(ModPlug_Settings));
}

void ModPlug_SetSettings(const ModPlug_Settings* settings)
{
	memcpy(&ModPlug::gSettings, settings, sizeof(ModPlug_Settings));
	ModPlug::UpdateSettings(false); // do not update basic config.
}
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (endian and char fixes for PPC)
*/

#include <math.h> //for GCCFIX
//#include "stdafx.h"
//#include "sndfile.h"

#define MMCMP_SUPPORT

#ifdef MMCMP_SUPPORT
extern BOOL MMCMP_Unpack(LPCBYTE *ppMemFile, LPDWORD pdwMemLength);
#endif

// External decompressors
extern void AMSUnpack(const char *psrc, UINT inputlen, char *pdest, UINT dmax, char packcharacter);
extern WORD MDLReadBits(DWORD &bitbuf, UINT &bitnum, LPBYTE &ibuf, CHAR n);
extern int DMFUnpack(LPBYTE psample, LPBYTE ibuf, LPBYTE ibufmax, UINT maxlen);
extern DWORD ITUnpack8Bit(signed char *pSample, DWORD dwLen, LPBYTE lpMemFile, DWORD dwMemLength, DWORD channels, BOOL b215);
extern DWORD ITUnpack16Bit(signed char *pSample, DWORD dwLen, LPBYTE lpMemFile, DWORD dwMemLength, DWORD channels, BOOL b215);


#define MAX_PACK_TABLES		3


// Compression table
static const signed char UnpackTable[MAX_PACK_TABLES][16] =
//--------------------------------------------
{
	// CPU-generated dynamic table
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	// u-Law table
	{0, 1, 2, 4, 8, 16, 32, 64,
	-1, -2, -4, -8, -16, -32, -48, -64},
	// Linear table
	{0, 1, 2, 3, 5, 7, 12, 19,
	-1, -2, -3, -5, -7, -12, -19, -31}
};


//////////////////////////////////////////////////////////
// CSoundFile

CSoundFile::CSoundFile()
//----------------------
{
	m_nType = MOD_TYPE_NONE;
	m_dwSongFlags = 0;
	m_nChannels = 0;
	m_nMixChannels = 0;
	m_nSamples = 0;
	m_nInstruments = 0;
	m_nPatternNames = 0;
	m_lpszPatternNames = NULL;
	m_lpszSongComments = NULL;
	m_nFreqFactor = m_nTempoFactor = 128;
	m_nMasterVolume = 128;
	m_nMinPeriod = 0x20;
	m_nMaxPeriod = 0x7FFF;
	m_nRepeatCount = 0;
	memset(Chn, 0, sizeof(Chn));
	memset(ChnMix, 0, sizeof(ChnMix));
	memset(Ins, 0, sizeof(Ins));
	memset(ChnSettings, 0, sizeof(ChnSettings));
	memset(Headers, 0, sizeof(Headers));
	memset(Order, 0xFF, sizeof(Order));
	memset(Patterns, 0, sizeof(Patterns));
	memset(m_szNames, 0, sizeof(m_szNames));
	memset(m_MixPlugins, 0, sizeof(m_MixPlugins));
}


CSoundFile::~CSoundFile()
//-----------------------
{
	Destroy();
}


BOOL CSoundFile::Create(LPCBYTE lpStream, DWORD dwMemLength)
//----------------------------------------------------------
{
	int i;

	m_nType = MOD_TYPE_NONE;
	m_dwSongFlags = 0;
	m_nChannels = 0;
	m_nMixChannels = 0;
	m_nSamples = 0;
	m_nInstruments = 0;
	m_nFreqFactor = m_nTempoFactor = 128;
	m_nMasterVolume = 128;
	m_nDefaultGlobalVolume = 256;
	m_nGlobalVolume = 256;
	m_nOldGlbVolSlide = 0;
	m_nDefaultSpeed = 6;
	m_nDefaultTempo = 125;
	m_nPatternDelay = 0;
	m_nFrameDelay = 0;
	m_nNextRow = 0;
	m_nRow = 0;
	m_nNextStartRow = 0;
	m_nPattern = 0;
	m_nCurrentPattern = 0;
	m_nNextPattern = 0;
	m_nRestartPos = 0;
	m_nMinPeriod = 16;
	m_nMaxPeriod = 32767;
	m_nSongPreAmp = 0x30;
	m_nPatternNames = 0;
	m_nMaxOrderPosition = 0;
	m_lpszPatternNames = NULL;
	m_lpszSongComments = NULL;
	memset(Ins, 0, sizeof(Ins));
	memset(ChnMix, 0, sizeof(ChnMix));
	memset(Chn, 0, sizeof(Chn));
	memset(Headers, 0, sizeof(Headers));
	memset(Order, 0xFF, sizeof(Order));
	memset(Patterns, 0, sizeof(Patterns));
	memset(m_szNames, 0, sizeof(m_szNames));
	memset(m_MixPlugins, 0, sizeof(m_MixPlugins));
	ResetMidiCfg();
	for (UINT npt=0; npt<MAX_PATTERNS; npt++) PatternSize[npt] = 64;
	for (UINT nch=0; nch<MAX_BASECHANNELS; nch++)
	{
		ChnSettings[nch].nPan = 128;
		ChnSettings[nch].nVolume = 64;
		ChnSettings[nch].dwFlags = 0;
		ChnSettings[nch].szName[0] = 0;
	}
	if (lpStream)
	{
#ifdef MMCMP_SUPPORT
		BOOL bMMCmp = MMCMP_Unpack(&lpStream, &dwMemLength);
#endif
		if ((!ReadXM(lpStream, dwMemLength))
		 && (!ReadS3M(lpStream, dwMemLength))
		 && (!ReadIT(lpStream, dwMemLength))
		 && (!ReadWav(lpStream, dwMemLength))
#ifndef MODPLUG_BASIC_SUPPORT
/* Sequencer File Format Support */
		 && (!ReadABC(lpStream, dwMemLength))
		 && (!ReadMID(lpStream, dwMemLength))
		 && (!ReadPAT(lpStream, dwMemLength))
		 && (!ReadSTM(lpStream, dwMemLength))
		 && (!ReadMed(lpStream, dwMemLength))
		 && (!ReadMTM(lpStream, dwMemLength))
		 && (!ReadMDL(lpStream, dwMemLength))
		 && (!ReadDBM(lpStream, dwMemLength))
		 && (!Read669(lpStream, dwMemLength))
		 && (!ReadFAR(lpStream, dwMemLength))
		 && (!ReadAMS(lpStream, dwMemLength))
		 && (!ReadOKT(lpStream, dwMemLength))
		 && (!ReadPTM(lpStream, dwMemLength))
		 && (!ReadUlt(lpStream, dwMemLength))
		 && (!ReadDMF(lpStream, dwMemLength))
		 && (!ReadDSM(lpStream, dwMemLength))
		 && (!ReadUMX(lpStream, dwMemLength))
		 && (!ReadAMF(lpStream, dwMemLength))
		 && (!ReadPSM(lpStream, dwMemLength))
		 && (!ReadMT2(lpStream, dwMemLength))
#endif // MODPLUG_BASIC_SUPPORT
		 && (!ReadMod(lpStream, dwMemLength))) m_nType = MOD_TYPE_NONE;
#ifdef MMCMP_SUPPORT
		if (bMMCmp)
		{
			free((void*)lpStream);
			lpStream = NULL;
		}
#endif
	}
	// Adjust song names
	for (i=0; i<MAX_SAMPLES; i++)
	{
		LPSTR p = m_szNames[i];
		int j = 31;
		p[j] = 0;
		while ((j>=0) && (p[j]<=' ')) p[j--] = 0;
		while (j>=0)
		{
			if (((BYTE)p[j]) < ' ') p[j] = ' ';
			j--;
		}
	}
	// Adjust channels
	for (i=0; i<MAX_BASECHANNELS; i++)
	{
		if (ChnSettings[i].nVolume > 64) ChnSettings[i].nVolume = 64;
		if (ChnSettings[i].nPan > 256) ChnSettings[i].nPan = 128;
		Chn[i].nPan = ChnSettings[i].nPan;
		Chn[i].nGlobalVol = ChnSettings[i].nVolume;
		Chn[i].dwFlags = ChnSettings[i].dwFlags;
		Chn[i].nVolume = 256;
		Chn[i].nCutOff = 0x7F;
	}
	// Checking instruments
	MODINSTRUMENT *pins = Ins;

	for (i=0; i<MAX_INSTRUMENTS; i++, pins++)
	{
		if (pins->pSample)
		{
			if (pins->nLoopEnd > pins->nLength) pins->nLoopEnd = pins->nLength;
			if (pins->nLoopStart + 3 >= pins->nLoopEnd)
			{
				pins->nLoopStart = 0;
				pins->nLoopEnd = 0;
			}
			if (pins->nSustainEnd > pins->nLength) pins->nSustainEnd = pins->nLength;
			if (pins->nSustainStart + 3 >= pins->nSustainEnd)
			{
				pins->nSustainStart = 0;
				pins->nSustainEnd = 0;
			}
		} else
		{
			pins->nLength = 0;
			pins->nLoopStart = 0;
			pins->nLoopEnd = 0;
			pins->nSustainStart = 0;
			pins->nSustainEnd = 0;
		}
		if (!pins->nLoopEnd) pins->uFlags &= ~CHN_LOOP;
		if (!pins->nSustainEnd) pins->uFlags &= ~CHN_SUSTAINLOOP;
		if (pins->nGlobalVol > 64) pins->nGlobalVol = 64;
	}
	// Check invalid instruments
	while ((m_nInstruments > 0) && (!Headers[m_nInstruments]))
		m_nInstruments--;
	// Set default values
	if (m_nSongPreAmp < 0x20) m_nSongPreAmp = 0x20;
	if (m_nDefaultTempo < 32) m_nDefaultTempo = 125;
	if (!m_nDefaultSpeed) m_nDefaultSpeed = 6;
	m_nMusicSpeed = m_nDefaultSpeed;
	m_nMusicTempo = m_nDefaultTempo;
	m_nGlobalVolume = m_nDefaultGlobalVolume;
	m_nNextPattern = 0;
	m_nCurrentPattern = 0;
	m_nPattern = 0;
	m_nBufferCount = 0;
	m_nTickCount = m_nMusicSpeed;
	m_nNextRow = 0;
	m_nRow = 0;
	m_nNextStartRow = 0;
	if ((m_nRestartPos >= MAX_ORDERS) || (Order[m_nRestartPos] >= MAX_PATTERNS)) m_nRestartPos = 0;
	// Load plugins
	if (gpMixPluginCreateProc)
	{
		for (UINT iPlug=0; iPlug<MAX_MIXPLUGINS; iPlug++)
		{
			if ((m_MixPlugins[iPlug].Info.dwPluginId1)
			 || (m_MixPlugins[iPlug].Info.dwPluginId2))
			{
				gpMixPluginCreateProc(&m_MixPlugins[iPlug]);
				if (m_MixPlugins[iPlug].pMixPlugin)
				{
					m_MixPlugins[iPlug].pMixPlugin->RestoreAllParameters();
				}
			}
		}
	}
	if (m_nType)
	{
		UINT maxpreamp = 0x10+(m_nChannels*8);
		if (maxpreamp > 100) maxpreamp = 100;
		if (m_nSongPreAmp > maxpreamp) m_nSongPreAmp = maxpreamp;
		return TRUE;
	}
	return FALSE;
}


BOOL CSoundFile::Destroy()

//------------------------
{
	int i;
	for (i=0; i<MAX_PATTERNS; i++) if (Patterns[i])
	{
		FreePattern(Patterns[i]);
		Patterns[i] = NULL;
	}
	m_nPatternNames = 0;
	if (m_lpszPatternNames)
	{
		delete [] m_lpszPatternNames;
		m_lpszPatternNames = NULL;
	}
	if (m_lpszSongComments)
	{
		delete [] m_lpszSongComments;
		m_lpszSongComments = NULL;
	}
	for (i=1; i<MAX_SAMPLES; i++)
	{
		MODINSTRUMENT *pins = &Ins[i];
		if (pins->pSample)
		{
			FreeSample(pins->pSample);
			pins->pSample = NULL;
		}
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		if (Headers[i])
		{
			delete Headers[i];
			Headers[i] = NULL;
		}
	}
	for (i=0; i<MAX_MIXPLUGINS; i++)
	{
		if ((m_MixPlugins[i].nPluginDataSize) && (m_MixPlugins[i].pPluginData))
		{
			m_MixPlugins[i].nPluginDataSize = 0;
			delete [] (signed char*)m_MixPlugins[i].pPluginData;
			m_MixPlugins[i].pPluginData = NULL;
		}
		m_MixPlugins[i].pMixState = NULL;
		if (m_MixPlugins[i].pMixPlugin)
		{
			m_MixPlugins[i].pMixPlugin->Release();
			m_MixPlugins[i].pMixPlugin = NULL;
		}
	}
	m_nType = MOD_TYPE_NONE;
	m_nChannels = m_nSamples = m_nInstruments = 0;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// Memory Allocation

MODCOMMAND *CSoundFile::AllocatePattern(UINT rows, UINT nchns)
//------------------------------------------------------------
{
	MODCOMMAND *p = new MODCOMMAND[rows*nchns];
	if (p) memset(p, 0, rows*nchns*sizeof(MODCOMMAND));
	return p;
}


void CSoundFile::FreePattern(LPVOID pat)
//--------------------------------------
{
	if (pat) delete [] (signed char*)pat;
}


signed char* CSoundFile::AllocateSample(UINT nbytes)
//-------------------------------------------
{
	signed char * p = (signed char *)calloc(1, (nbytes+39) & ~7);
	if (p) p += 16;
	return p;
}


void CSoundFile::FreeSample(LPVOID p)
//-----------------------------------
{
	if (p) {
		free((char*)p - 16);
	}
}


//////////////////////////////////////////////////////////////////////////
// Misc functions

void CSoundFile::ResetMidiCfg()
//-----------------------------
{
	memset(&m_MidiCfg, 0, sizeof(m_MidiCfg));
	lstrcpy(&m_MidiCfg.szMidiGlb[MIDIOUT_START*32], "FF");
	lstrcpy(&m_MidiCfg.szMidiGlb[MIDIOUT_STOP*32], "FC");
	lstrcpy(&m_MidiCfg.szMidiGlb[MIDIOUT_NOTEON*32], "9c n v");
	lstrcpy(&m_MidiCfg.szMidiGlb[MIDIOUT_NOTEOFF*32], "9c n 0");
	lstrcpy(&m_MidiCfg.szMidiGlb[MIDIOUT_PROGRAM*32], "Cc p");
	lstrcpy(&m_MidiCfg.szMidiSFXExt[0], "F0F000z");
	for (int iz=0; iz<16; iz++) /*w*/sprintf(&m_MidiCfg.szMidiZXXExt[iz*32], "F0F001%02X", iz*8);
}


UINT CSoundFile::GetNumChannels() const
//-------------------------------------
{
	UINT n = 0;
	for (UINT i=0; i<m_nChannels; i++) if (ChnSettings[i].nVolume) n++;
	return n;
}


UINT CSoundFile::GetSongComments(LPSTR s, UINT len, UINT linesize)
//----------------------------------------------------------------
{
	LPCSTR p = m_lpszSongComments;
	if (!p) return 0;
	UINT i = 2, ln=0;
	if ((len) && (s)) s[0] = '\x0D';
	if ((len > 1) && (s)) s[1] = '\x0A';
	while ((*p)	&& (i+2 < len))
	{
		BYTE c = (BYTE)*p++;
		if ((c == 0x0D) || ((c == ' ') && (ln >= linesize)))
			{ if (s) { s[i++] = '\x0D'; s[i++] = '\x0A'; } else i+= 2; ln=0; }
		else
		if (c >= 0x20) { if (s) s[i++] = c; else i++; ln++; }
	}
	if (s) s[i] = 0;
	return i;
}


UINT CSoundFile::GetRawSongComments(LPSTR s, UINT len, UINT linesize)
//-------------------------------------------------------------------
{
	LPCSTR p = m_lpszSongComments;
	if (!p) return 0;
	UINT i = 0, ln=0;
	while ((*p)	&& (i < len-1))
	{
		BYTE c = (BYTE)*p++;
		if ((c == 0x0D)	|| (c == 0x0A))
		{
			if (ln)
			{
				while (ln < linesize) { if (s) s[i] = ' '; i++; ln++; }
				ln = 0;
			}
		} else
		if ((c == ' ') && (!ln))
		{
			UINT k=0;
			while ((p[k]) && (p[k] >= ' '))	k++;
			if (k <= linesize)
			{
				if (s) s[i] = ' ';
				i++;
				ln++;
			}
		} else
		{
			if (s) s[i] = c;
			i++;
			ln++;
			if (ln == linesize) ln = 0;
		}
	}
	if (ln)
	{
		while ((ln < linesize) && (i < len))
		{
			if (s) s[i] = ' ';
			i++;
			ln++;
		}
	}
	if (s) s[i] = 0;
	return i;
}


BOOL CSoundFile::SetWaveConfig(UINT nRate,UINT nBits,UINT nChannels,BOOL bMMX)
//----------------------------------------------------------------------------
{
	BOOL bReset = FALSE;
	DWORD d = gdwSoundSetup & ~SNDMIX_ENABLEMMX;
	if (bMMX) d |= SNDMIX_ENABLEMMX;
	if ((gdwMixingFreq != nRate) || (gnBitsPerSample != nBits) || (gnChannels != nChannels) || (d != gdwSoundSetup)) bReset = TRUE;
	gnChannels = nChannels;
	gdwSoundSetup = d;
	gdwMixingFreq = nRate;
	gnBitsPerSample = nBits;
	InitPlayer(bReset);
	return TRUE;
}

BOOL CSoundFile::SetMixConfig(UINT nStereoSeparation, UINT nMaxMixChannels)
//-------------------------------------------------------------------------
{
	if (nMaxMixChannels < 2) return FALSE;

	m_nMaxMixChannels = nMaxMixChannels;
	m_nStereoSeparation = nStereoSeparation;
	return TRUE;
}


BOOL CSoundFile::SetResamplingMode(UINT nMode)
//--------------------------------------------
{
	DWORD d = gdwSoundSetup & ~(SNDMIX_NORESAMPLING|SNDMIX_HQRESAMPLER|SNDMIX_ULTRAHQSRCMODE);
	switch(nMode)
	{
	case SRCMODE_NEAREST:	d |= SNDMIX_NORESAMPLING; break;
	case SRCMODE_LINEAR:	break;
	case SRCMODE_SPLINE:	d |= SNDMIX_HQRESAMPLER; break;
	case SRCMODE_POLYPHASE:	d |= (SNDMIX_HQRESAMPLER|SNDMIX_ULTRAHQSRCMODE); break;
	default:
		return FALSE;
	}
	gdwSoundSetup = d;
	return TRUE;
}


BOOL CSoundFile::SetMasterVolume(UINT nVol, BOOL bAdjustAGC)
//----------------------------------------------------------
{
	if (nVol < 1) nVol = 1;
	if (nVol > 0x200) nVol = 0x200;	// x4 maximum
	if ((nVol < m_nMasterVolume) && (nVol) && (gdwSoundSetup & SNDMIX_AGC) && (bAdjustAGC))
	{
		gnAGC = gnAGC * m_nMasterVolume / nVol;
		if (gnAGC > AGC_UNITY) gnAGC = AGC_UNITY;
	}
	m_nMasterVolume = nVol;
	return TRUE;
}


void CSoundFile::SetAGC(BOOL b)
//-----------------------------
{
	if (b)
	{
		if (!(gdwSoundSetup & SNDMIX_AGC))
		{
			gdwSoundSetup |= SNDMIX_AGC;
			gnAGC = AGC_UNITY;
		}
	} else gdwSoundSetup &= ~SNDMIX_AGC;
}


UINT CSoundFile::GetNumPatterns() const
//-------------------------------------
{
	UINT i = 0;
	while ((i < MAX_ORDERS) && (Order[i] < 0xFF)) i++;
	return i;
}


UINT CSoundFile::GetNumInstruments() const
//----------------------------------------
{
	UINT n=0;
	for (UINT i=0; i<MAX_INSTRUMENTS; i++) if (Ins[i].pSample) n++;
	return n;
}


UINT CSoundFile::GetMaxPosition() const
//-------------------------------------
{
	UINT max = 0;
	UINT i = 0;

	while ((i < MAX_ORDERS) && (Order[i] != 0xFF))
	{
		if (Order[i] < MAX_PATTERNS) max += PatternSize[Order[i]];
		i++;
	}
	return max;
}


UINT CSoundFile::GetCurrentPos() const
//------------------------------------
{
	UINT pos = 0;

	for (UINT i=0; i<m_nCurrentPattern; i++) if (Order[i] < MAX_PATTERNS)
		pos += PatternSize[Order[i]];
	return pos + m_nRow;
}


void CSoundFile::SetCurrentPos(UINT nPos)
//---------------------------------------
{
	UINT i, nPattern;

	for (i=0; i<MAX_CHANNELS; i++)
	{
		Chn[i].nNote = Chn[i].nNewNote = Chn[i].nNewIns = 0;
		Chn[i].pInstrument = NULL;
		Chn[i].pHeader = NULL;
		Chn[i].nPortamentoDest = 0;
		Chn[i].nCommand = 0;
		Chn[i].nPatternLoopCount = 0;
		Chn[i].nPatternLoop = 0;
		Chn[i].nFadeOutVol = 0;
		Chn[i].dwFlags |= CHN_KEYOFF|CHN_NOTEFADE;
		Chn[i].nTremorCount = 0;
	}
	if (!nPos)
	{
		for (i=0; i<MAX_CHANNELS; i++)
		{
			Chn[i].nPeriod = 0;
			Chn[i].nPos = Chn[i].nLength = 0;
			Chn[i].nLoopStart = 0;
			Chn[i].nLoopEnd = 0;
			Chn[i].nROfs = Chn[i].nLOfs = 0;
			Chn[i].pSample = NULL;
			Chn[i].pInstrument = NULL;
			Chn[i].pHeader = NULL;
			Chn[i].nCutOff = 0x7F;
			Chn[i].nResonance = 0;
			Chn[i].nLeftVol = Chn[i].nRightVol = 0;
			Chn[i].nNewLeftVol = Chn[i].nNewRightVol = 0;
			Chn[i].nLeftRamp = Chn[i].nRightRamp = 0;
			Chn[i].nVolume = 256;
			if (i < MAX_BASECHANNELS)
			{
				Chn[i].dwFlags = ChnSettings[i].dwFlags;
				Chn[i].nPan = ChnSettings[i].nPan;
				Chn[i].nGlobalVol = ChnSettings[i].nVolume;
			} else
			{
				Chn[i].dwFlags = 0;
				Chn[i].nPan = 128;
				Chn[i].nGlobalVol = 64;
			}
		}
		m_nGlobalVolume = m_nDefaultGlobalVolume;
		m_nMusicSpeed = m_nDefaultSpeed;
		m_nMusicTempo = m_nDefaultTempo;
	}
	m_dwSongFlags &= ~(SONG_PATTERNLOOP|SONG_CPUVERYHIGH|SONG_FADINGSONG|SONG_ENDREACHED|SONG_GLOBALFADE);
	for (nPattern = 0; nPattern < MAX_ORDERS; nPattern++)
	{
		UINT ord = Order[nPattern];
		if (ord == 0xFE) continue;
		if (ord == 0xFF) break;
		if (ord < MAX_PATTERNS)
		{
			if (nPos < (UINT)PatternSize[ord]) break;
			nPos -= PatternSize[ord];
		}
	}
	// Buggy position ?
	if ((nPattern >= MAX_ORDERS)
	 || (Order[nPattern] >= MAX_PATTERNS)
	 || (nPos >= PatternSize[Order[nPattern]]))
	{
		nPos = 0;
		nPattern = 0;
	}
	UINT nRow = nPos;
	if ((nRow) && (Order[nPattern] < MAX_PATTERNS))
	{
		MODCOMMAND *p = Patterns[Order[nPattern]];
		if ((p) && (nRow < PatternSize[Order[nPattern]]))
		{
			BOOL bOk = FALSE;
			while ((!bOk) && (nRow > 0))
			{
				UINT n = nRow * m_nChannels;
				for (UINT k=0; k<m_nChannels; k++, n++)
				{
					if (p[n].note)
					{
						bOk = TRUE;
						break;
					}
				}
				if (!bOk) nRow--;
			}
		}
	}
	m_nNextPattern = nPattern;
	m_nNextRow = nRow;
	m_nNextStartRow = 0;
	m_nTickCount = m_nMusicSpeed;
	m_nBufferCount = 0;
	m_nPatternDelay = 0;
	m_nFrameDelay = 0;
}


void CSoundFile::SetCurrentOrder(UINT nPos)
//-----------------------------------------
{
	while ((nPos < MAX_ORDERS) && (Order[nPos] == 0xFE)) nPos++;
	if ((nPos >= MAX_ORDERS) || (Order[nPos] >= MAX_PATTERNS)) return;
	for (UINT j=0; j<MAX_CHANNELS; j++)
	{
		Chn[j].nPeriod = 0;
		Chn[j].nNote = 0;
		Chn[j].nPortamentoDest = 0;
		Chn[j].nCommand = 0;
		Chn[j].nPatternLoopCount = 0;
		Chn[j].nPatternLoop = 0;
		Chn[j].nTremorCount = 0;
	}
	if (!nPos)
	{
		SetCurrentPos(0);
	} else
	{
		m_nNextPattern = nPos;
		m_nRow = m_nNextRow = m_nNextStartRow = 0;
		m_nPattern = 0;
		m_nTickCount = m_nMusicSpeed;
		m_nBufferCount = 0;
		m_nTotalCount = 0;
		m_nPatternDelay = 0;
		m_nFrameDelay = 0;
	}
	m_dwSongFlags &= ~(SONG_PATTERNLOOP|SONG_CPUVERYHIGH|SONG_FADINGSONG|SONG_ENDREACHED|SONG_GLOBALFADE);
}


void CSoundFile::ResetChannels()
//------------------------------
{
	m_dwSongFlags &= ~(SONG_CPUVERYHIGH|SONG_FADINGSONG|SONG_ENDREACHED|SONG_GLOBALFADE);
	m_nBufferCount = 0;
	for (UINT i=0; i<MAX_CHANNELS; i++)
	{
		Chn[i].nROfs = Chn[i].nLOfs = 0;
	}
}


void CSoundFile::LoopPattern(int nPat, int nRow)
//----------------------------------------------
{
	if ((nPat < 0) || (nPat >= MAX_PATTERNS) || (!Patterns[nPat]))
	{
		m_dwSongFlags &= ~SONG_PATTERNLOOP;
	} else
	{
		if ((nRow < 0) || (nRow >= PatternSize[nPat])) nRow = 0;
		m_nPattern = nPat;
		m_nRow = m_nNextRow = nRow;
		m_nTickCount = m_nMusicSpeed;
		m_nPatternDelay = 0;
		m_nFrameDelay = 0;
		m_nBufferCount = 0;
		m_dwSongFlags |= SONG_PATTERNLOOP;
	}
}


UINT CSoundFile::GetBestSaveFormat() const
//----------------------------------------
{
	if ((!m_nSamples) || (!m_nChannels)) return MOD_TYPE_NONE;
	if (!m_nType) return MOD_TYPE_NONE;
	if (m_nType & (MOD_TYPE_MOD|MOD_TYPE_OKT))
		return MOD_TYPE_MOD;
	if (m_nType & (MOD_TYPE_S3M|MOD_TYPE_STM|MOD_TYPE_ULT|MOD_TYPE_FAR|MOD_TYPE_PTM))
		return MOD_TYPE_S3M;
	if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MED|MOD_TYPE_MTM|MOD_TYPE_MT2))
		return MOD_TYPE_XM;
	return MOD_TYPE_IT;
}


UINT CSoundFile::GetSaveFormats() const
//-------------------------------------
{
	UINT n = 0;
	if ((!m_nSamples) || (!m_nChannels) || (m_nType == MOD_TYPE_NONE)) return 0;
	if (m_nType & MOD_TYPE_MOD)
		n |= MOD_TYPE_MOD;
	if (m_nType & MOD_TYPE_S3M)
		n |= MOD_TYPE_S3M;
	// Can always save to XM & IT
	n |= MOD_TYPE_XM | MOD_TYPE_IT;
	if (!m_nInstruments)
	{
		if (m_nSamples < 32) n |= MOD_TYPE_MOD;
		n |= MOD_TYPE_S3M;
	}
	return n;
}


UINT CSoundFile::GetSampleName(UINT nSample,LPSTR s) const
//--------------------------------------------------------
{
        char sztmp[40] = "";      // changed from CHAR
	if (nSample < MAX_SAMPLES)
		memcpy(sztmp, m_szNames[nSample], 32);
	sztmp[31] = 0;
	if (s) strcpy(s, sztmp);
	return strlen(sztmp);
}


UINT CSoundFile::GetInstrumentName(UINT nInstr,LPSTR s) const
//-----------------------------------------------------------
{
        char sztmp[40] = "";  // changed from CHAR
	if ((nInstr >= MAX_INSTRUMENTS) || (!Headers[nInstr]))
	{
		if (s) *s = 0;
		return 0;
	}
	INSTRUMENTHEADER *penv = Headers[nInstr];
	memcpy(sztmp, penv->name, 32);
	sztmp[31] = 0;
	if (s) strcpy(s, sztmp);
	return strlen(sztmp);
}


#ifndef NO_PACKING
UINT CSoundFile::PackSample(int &sample, int next)
//------------------------------------------------
{
	UINT i = 0;
	int delta = next - sample;
	if (delta >= 0)
	{
		for (i=0; i<7; i++) if (delta <= (int)CompressionTable[i+1]) break;
	} else
	{
		for (i=8; i<15; i++) if (delta >= (int)CompressionTable[i+1]) break;
	}
	sample += (int)CompressionTable[i];
	return i;
}


BOOL CSoundFile::CanPackSample(LPSTR pSample, UINT nLen, UINT nPacking, BYTE *result)
//-----------------------------------------------------------------------------------
{
	int pos, old, oldpos, besttable = 0;
	DWORD dwErr, dwTotal, dwResult;
	int i,j;

	if (result) *result = 0;
	if ((!pSample) || (nLen < 1024)) return FALSE;
	// Try packing with different tables
	dwResult = 0;
	for (j=1; j<MAX_PACK_TABLES; j++)
	{
		memcpy(CompressionTable, UnpackTable[j], 16);
		dwErr = 0;
		dwTotal = 1;
		old = pos = oldpos = 0;
		for (i=0; i<(int)nLen; i++)
		{
			int s = (int)pSample[i];
			PackSample(pos, s);
			dwErr += abs(pos - oldpos);
			dwTotal += abs(s - old);
			old = s;
			oldpos = pos;
		}
		dwErr = _muldiv(dwErr, 100, dwTotal);
		if (dwErr >= dwResult)
		{
			dwResult = dwErr;
			besttable = j;
		}
	}
	memcpy(CompressionTable, UnpackTable[besttable], 16);
	if (result)
	{
		if (dwResult > 100) *result	= 100; else *result = (BYTE)dwResult;
	}
	return (dwResult >= nPacking) ? TRUE : FALSE;
}
#endif // NO_PACKING

#ifndef MODPLUG_NO_FILESAVE

UINT CSoundFile::WriteSample(FILE *f, MODINSTRUMENT *pins, UINT nFlags, UINT nMaxLen)
//-----------------------------------------------------------------------------------
{
	UINT len = 0, bufcount;
	signed char buffer[4096];
	signed char *pSample = (signed char *)pins->pSample;
	UINT nLen = pins->nLength;

	if ((nMaxLen) && (nLen > nMaxLen)) nLen = nMaxLen;
	if ((!pSample) || (f == NULL) || (!nLen)) return 0;
	switch(nFlags)
	{
#ifndef NO_PACKING
	// 3: 4-bit ADPCM data
	case RS_ADPCM4:
		{
			int pos;
			len = (nLen + 1) / 2;
			fwrite(CompressionTable, 16, 1, f);
			bufcount = 0;
			pos = 0;
			for (UINT j=0; j<len; j++)
			{
				BYTE b;
				// Sample #1
				b = PackSample(pos, (int)pSample[j*2]);
				// Sample #2
				b |= PackSample(pos, (int)pSample[j*2+1]) << 4;
				buffer[bufcount++] = (signed char)b;
				if (bufcount >= sizeof(buffer))
				{
					fwrite(buffer, 1, bufcount, f);
					bufcount = 0;
				}
			}
			if (bufcount) fwrite(buffer, 1, bufcount, f);
			len += 16;
		}
		break;
#endif // NO_PACKING

	// 16-bit samples
	case RS_PCM16U:
	case RS_PCM16D:
	case RS_PCM16S:
		{
			int16_t *p = (int16_t *)pSample;
			int s_old = 0, s_ofs;
			len = nLen * 2;
			bufcount = 0;
			s_ofs = (nFlags == RS_PCM16U) ? 0x8000 : 0;
			for (UINT j=0; j<nLen; j++)
			{
				int s_new = *p;
				p++;
				if (pins->uFlags & CHN_STEREO)
				{
					s_new = (s_new + (*p) + 1) >> 1;
					p++;
				}
				if (nFlags == RS_PCM16D)
				{
					int16_t temp = bswapLE16((int16_t)(s_new - s_old));
					*((int16_t*)(&buffer[bufcount])) = temp;
					s_old = s_new;
				} else
				{
					int16_t temp = bswapLE16((int16_t)(s_new + s_ofs));
					*((int16_t *)(&buffer[bufcount])) = temp;
				}
				bufcount += 2;
				if (bufcount >= sizeof(buffer) - 1)
				{
					fwrite(buffer, 1, bufcount, f);
					bufcount = 0;
				}
			}
			if (bufcount) fwrite(buffer, 1, bufcount, f);
		}
		break;


	// 8-bit Stereo samples (not interleaved)
	case RS_STPCM8S:
	case RS_STPCM8U:
	case RS_STPCM8D:
		{
			int s_ofs = (nFlags == RS_STPCM8U) ? 0x80 : 0;
			for (UINT iCh=0; iCh<2; iCh++)
			{
				signed char *p = pSample + iCh;
				int s_old = 0;

				bufcount = 0;
				for (UINT j=0; j<nLen; j++)
				{
					int s_new = *p;
					p += 2;
					if (nFlags == RS_STPCM8D)
					{
						buffer[bufcount++] = (signed char)(s_new - s_old);
						s_old = s_new;
					} else
					{
						buffer[bufcount++] = (signed char)(s_new + s_ofs);
					}
					if (bufcount >= sizeof(buffer))
					{
						fwrite(buffer, 1, bufcount, f);
						bufcount = 0;
					}
				}
				if (bufcount) fwrite(buffer, 1, bufcount, f);
			}
		}
		len = nLen * 2;
		break;

	// 16-bit Stereo samples (not interleaved)
	case RS_STPCM16S:
	case RS_STPCM16U:
	case RS_STPCM16D:
		{
			int s_ofs = (nFlags == RS_STPCM16U) ? 0x8000 : 0;
			for (UINT iCh=0; iCh<2; iCh++)
			{
				int16_t *p = ((int16_t *)pSample) + iCh;
				int s_old = 0;

				bufcount = 0;
				for (UINT j=0; j<nLen; j++)
				{
					int s_new = *p;
					p += 2;
					if (nFlags == RS_STPCM16D)
					{
						int16_t temp = bswapLE16((int16_t)(s_new - s_old));
						*((int16_t *)(&buffer[bufcount])) = temp;
						s_old = s_new;
					} else
					{
						int16_t temp = bswapLE16((int16_t)(s_new - s_ofs));
						*((int16_t*)(&buffer[bufcount])) = temp;
					}
					bufcount += 2;
					if (bufcount >= sizeof(buffer))
					{
						fwrite(buffer, 1, bufcount, f);
						bufcount = 0;
					}
				}
				if (bufcount) fwrite(buffer, 1, bufcount, f);
			}
		}
		len = nLen*4;
		break;

	//	Stereo signed interleaved
	case RS_STIPCM8S:
	case RS_STIPCM16S:
		len = nLen * 2;
		if (nFlags == RS_STIPCM16S) len *= 2;
		fwrite(pSample, 1, len, f);
		break;

	// Default: assume 8-bit PCM data
	default:
		len = nLen;
		bufcount = 0;
		{
			signed char *p = pSample;
			int sinc = (pins->uFlags & CHN_16BIT) ? 2 : 1;
			int s_old = 0, s_ofs = (nFlags == RS_PCM8U) ? 0x80 : 0;
			if (pins->uFlags & CHN_16BIT) p++;
			for (UINT j=0; j<len; j++)
			{
				int s_new = (signed char)(*p);
				p += sinc;
				if (pins->uFlags & CHN_STEREO)
				{
					s_new = (s_new + ((int)*p) + 1) >> 1;
					p += sinc;
				}
				if (nFlags == RS_PCM8D)
				{
					buffer[bufcount++] = (signed char)(s_new - s_old);
					s_old = s_new;
				} else
				{
					buffer[bufcount++] = (signed char)(s_new + s_ofs);
				}
				if (bufcount >= sizeof(buffer))
				{
					fwrite(buffer, 1, bufcount, f);
					bufcount = 0;
				}
			}
			if (bufcount) fwrite(buffer, 1, bufcount, f);
		}
	}
	return len;
}

#endif // MODPLUG_NO_FILESAVE


// Flags:
//	0 = signed 8-bit PCM data (default)
//	1 = unsigned 8-bit PCM data
//	2 = 8-bit ADPCM data with linear table
//	3 = 4-bit ADPCM data
//	4 = 16-bit ADPCM data with linear table
//	5 = signed 16-bit PCM data
//	6 = unsigned 16-bit PCM data


UINT CSoundFile::ReadSample(MODINSTRUMENT *pIns, UINT nFlags, LPCSTR lpMemFile, DWORD dwMemLength)
//------------------------------------------------------------------------------
{
	UINT len = 0, mem;

	// Disable >2Gb samples,(preventing buffer overflow in AllocateSample)
	if ((!pIns) || ((int)pIns->nLength < 4) || (!lpMemFile)) return 0;
	if (pIns->nLength > MAX_SAMPLE_LENGTH) pIns->nLength = MAX_SAMPLE_LENGTH;
	mem = pIns->nLength+6;
	pIns->uFlags &= ~(CHN_16BIT|CHN_STEREO);
	if (nFlags & RSF_16BIT)
	{
		mem *= 2;
		pIns->uFlags |= CHN_16BIT;
	}
	if (nFlags & RSF_STEREO)
	{
		mem *= 2;
		pIns->uFlags |= CHN_STEREO;
	}
	if ((pIns->pSample = AllocateSample(mem)) == NULL)
	{
		pIns->nLength = 0;
		return 0;
	}
	switch(nFlags)
	{
	// 1: 8-bit unsigned PCM data
	case RS_PCM8U:
		{
			len = pIns->nLength;
			if (len > dwMemLength) len = pIns->nLength = dwMemLength;
			signed char *pSample = pIns->pSample;
			for (UINT j=0; j<len; j++) pSample[j] = (signed char)(lpMemFile[j] - 0x80);
		}
		break;

	// 2: 8-bit ADPCM data with linear table
	case RS_PCM8D:
		{
			len = pIns->nLength;
			if (len > dwMemLength) break;
			signed char *pSample = pIns->pSample;
			const signed char *p = (const signed char *)lpMemFile;
			int delta = 0;

			for (UINT j=0; j<len; j++)
			{
				delta += p[j];
				*pSample++ = (signed char)delta;
			}
		}
		break;

	// 3: 4-bit ADPCM data
	case RS_ADPCM4:
		{
			len = (pIns->nLength + 1) / 2;
			if (len > dwMemLength - 16 || dwMemLength < 16) break;
			memcpy(CompressionTable, lpMemFile, 16);
			lpMemFile += 16;
			signed char *pSample = pIns->pSample;
			signed char delta = 0;
			for (UINT j=0; j<len; j++)
			{
				BYTE b0 = (BYTE)lpMemFile[j];
				BYTE b1 = (BYTE)(lpMemFile[j] >> 4);
				delta = (signed char)GetDeltaValue((int)delta, b0);
				pSample[0] = delta;
				delta = (signed char)GetDeltaValue((int)delta, b1);
				pSample[1] = delta;
				pSample += 2;
			}
			len += 16;
		}
		break;

	// 4: 16-bit ADPCM data with linear table
	case RS_PCM16D:
		{
			len = pIns->nLength * 2;
			if (len > dwMemLength) break;
			int16_t *pSample = (int16_t *)pIns->pSample;
			int16_t *p = (int16_t *)lpMemFile;
			int delta16 = 0;
			for (UINT j=0; j<len; j+=2)
			{
				delta16 += bswapLE16(*p++);
				*pSample++ = (int16_t )delta16;
			}
		}
		break;

	// 5: 16-bit signed PCM data
	case RS_PCM16S:
	        {
		len = pIns->nLength * 2;
		if (len <= dwMemLength) memcpy(pIns->pSample, lpMemFile, len);
			int16_t *pSample = (int16_t *)pIns->pSample;
			for (UINT j=0; j<len; j+=2)
			{
				int16_t rawSample = *pSample;
			        *pSample++ = bswapLE16(rawSample);
			}
		}
		break;

	// 16-bit signed mono PCM motorola byte order
	case RS_PCM16M:
		len = pIns->nLength * 2;
		if (len > dwMemLength) len = dwMemLength & ~1;
		if (len > 1)
		{
			signed char *pSample = (signed char *)pIns->pSample;
			signed char *pSrc = (signed char *)lpMemFile;
			for (UINT j=0; j<len; j+=2)
			{
			  	// pSample[j] = pSrc[j+1];
				// pSample[j+1] = pSrc[j];
			        *((uint16_t *)(pSample+j)) = bswapBE16(*((uint16_t *)(pSrc+j)));
			}
		}
		break;

	// 6: 16-bit unsigned PCM data
	case RS_PCM16U:
		{
			len = pIns->nLength * 2;
			if (len > dwMemLength) break;
			int16_t *pSample = (int16_t *)pIns->pSample;
			int16_t *pSrc = (int16_t *)lpMemFile;
			for (UINT j=0; j<len; j+=2) *pSample++ = bswapLE16(*(pSrc++)) - 0x8000;
		}
		break;

	// 16-bit signed stereo big endian
	case RS_STPCM16M:
		len = pIns->nLength * 2;
		if (len*2 <= dwMemLength)
		{
			signed char *pSample = (signed char *)pIns->pSample;
			signed char *pSrc = (signed char *)lpMemFile;
			for (UINT j=0; j<len; j+=2)
			{
			        // pSample[j*2] = pSrc[j+1];
				// pSample[j*2+1] = pSrc[j];
				// pSample[j*2+2] = pSrc[j+1+len];
				// pSample[j*2+3] = pSrc[j+len];
			        *((uint16_t *)(pSample+j*2)) = bswapBE16(*((uint16_t *)(pSrc+j)));
				*((uint16_t *)(pSample+j*2+2)) = bswapBE16(*((uint16_t *)(pSrc+j+len)));
			}
			len *= 2;
		}
		break;

	// 8-bit stereo samples
	case RS_STPCM8S:
	case RS_STPCM8U:
	case RS_STPCM8D:
		{
			int iadd_l = 0, iadd_r = 0;
			if (nFlags == RS_STPCM8U) { iadd_l = iadd_r = -128; }
			len = pIns->nLength;
			signed char *psrc = (signed char *)lpMemFile;
			signed char *pSample = (signed char *)pIns->pSample;
			if (len*2 > dwMemLength) break;
			for (UINT j=0; j<len; j++)
			{
				pSample[j*2] = (signed char)(psrc[0] + iadd_l);
				pSample[j*2+1] = (signed char)(psrc[len] + iadd_r);
				psrc++;
				if (nFlags == RS_STPCM8D)
				{
					iadd_l = pSample[j*2];
					iadd_r = pSample[j*2+1];
				}
			}
			len *= 2;
		}
		break;

	// 16-bit stereo samples
	case RS_STPCM16S:
	case RS_STPCM16U:
	case RS_STPCM16D:
		{
			int iadd_l = 0, iadd_r = 0;
			if (nFlags == RS_STPCM16U) { iadd_l = iadd_r = -0x8000; }
			len = pIns->nLength;
			int16_t *psrc = (int16_t *)lpMemFile;
			int16_t *pSample = (int16_t *)pIns->pSample;
			if (len*4 > dwMemLength) break;
			for (UINT j=0; j<len; j++)
			{
				pSample[j*2] = (int16_t) (bswapLE16(psrc[0]) + iadd_l);
				pSample[j*2+1] = (int16_t) (bswapLE16(psrc[len]) + iadd_r);
				psrc++;
				if (nFlags == RS_STPCM16D)
				{
					iadd_l = pSample[j*2];
					iadd_r = pSample[j*2+1];
				}
			}
			len *= 4;
		}
		break;

	// IT 2.14 compressed samples
	case RS_IT2148:
	case RS_IT21416:
	case RS_IT2158:
	case RS_IT21516:
		len = dwMemLength;
		if (len < 4) break;
		if ((nFlags == RS_IT2148) || (nFlags == RS_IT2158))
			ITUnpack8Bit(pIns->pSample, pIns->nLength, (LPBYTE)lpMemFile, dwMemLength, 1, (nFlags == RS_IT2158));
		else
			ITUnpack16Bit(pIns->pSample, pIns->nLength, (LPBYTE)lpMemFile, dwMemLength, 1, (nFlags == RS_IT21516));
		break;

	case RS_IT2148 | RSF_STEREO:
	case RS_IT21416 | RSF_STEREO:
	case RS_IT2158 | RSF_STEREO:
	case RS_IT21516 | RSF_STEREO:
		len = dwMemLength;
		if (len < 4) break;
		if ((nFlags == (RS_IT2148 | RSF_STEREO)) || (nFlags == (RS_IT2158 | RSF_STEREO)))
		{
			DWORD offset = ITUnpack8Bit(pIns->pSample, pIns->nLength, (LPBYTE)lpMemFile, dwMemLength, 2, (nFlags == (RS_IT2158 | RSF_STEREO)));
			ITUnpack8Bit(pIns->pSample + 1, pIns->nLength, (LPBYTE)lpMemFile + offset, dwMemLength - offset, 2, (nFlags == (RS_IT2158 | RSF_STEREO)));
		} else
		{
			DWORD offset = ITUnpack16Bit(pIns->pSample, pIns->nLength, (LPBYTE)lpMemFile, dwMemLength, 2, (nFlags == (RS_IT21516 | RSF_STEREO)));
			ITUnpack16Bit(pIns->pSample + 2, pIns->nLength, (LPBYTE)lpMemFile + offset, dwMemLength - offset, 2, (nFlags == (RS_IT21516 | RSF_STEREO)));
		}
		break;

#ifndef MODPLUG_BASIC_SUPPORT
#ifndef MODPLUG_FASTSOUNDLIB
	// 8-bit interleaved stereo samples
	case RS_STIPCM8S:
	case RS_STIPCM8U:
		{
			int iadd = 0;
			if (nFlags == RS_STIPCM8U) { iadd = -0x80; }
			len = pIns->nLength;
			if (len*2 > dwMemLength) len = dwMemLength >> 1;
			LPBYTE psrc = (LPBYTE)lpMemFile;
			LPBYTE pSample = (LPBYTE)pIns->pSample;
			for (UINT j=0; j<len; j++)
			{
				pSample[j*2] = (signed char)(psrc[0] + iadd);
				pSample[j*2+1] = (signed char)(psrc[1] + iadd);
				psrc+=2;
			}
			len *= 2;
		}
		break;

	// 16-bit interleaved stereo samples
	case RS_STIPCM16S:
	case RS_STIPCM16U:
		{
			int iadd = 0;
			if (nFlags == RS_STIPCM16U) iadd = -32768;
			len = pIns->nLength;
			if (len*4 > dwMemLength) len = dwMemLength >> 2;
			int16_t *psrc = (int16_t *)lpMemFile;
			int16_t *pSample = (int16_t *)pIns->pSample;
			for (UINT j=0; j<len; j++)
			{
				pSample[j*2] = (int16_t)(bswapLE16(psrc[0]) + iadd);
				pSample[j*2+1] = (int16_t)(bswapLE16(psrc[1]) + iadd);
				psrc += 2;
			}
			len *= 4;
		}
		break;

	// AMS compressed samples
	case RS_AMS8:
	case RS_AMS16:
		len = 9;
		if (dwMemLength > 9)
		{
			const char *psrc = lpMemFile;
			char packcharacter = lpMemFile[8], *pdest = (char *)pIns->pSample;
			UINT smplen = bswapLE32(*((LPDWORD)(lpMemFile+4)));
			if (smplen > dwMemLength - 9) smplen = dwMemLength - 9;
			len += smplen;
			UINT dmax = pIns->nLength;
			if (pIns->uFlags & CHN_16BIT) dmax <<= 1;
			AMSUnpack(psrc+9, smplen, pdest, dmax, packcharacter);
		}
		break;

	// PTM 8bit delta to 16-bit sample
	case RS_PTM8DTO16:
		{
			UINT j;
			len = pIns->nLength * 2;
			if (len > dwMemLength) break;
			int8_t *pSample = (int8_t *)pIns->pSample;
			int8_t delta8 = 0;
			for (j=0; j<len; j++)
			{
				delta8 += lpMemFile[j];
				*pSample++ = delta8;
			}
			uint16_t *pSampleW = (uint16_t *)pIns->pSample;
			for (j=0; j<len; j+=2)   // swaparoni!
			{
				uint16_t rawSample = *pSampleW;
			        *pSampleW++ = bswapLE16(rawSample);
			}
		}
		break;

	// Huffman MDL compressed samples
	case RS_MDL8:
	case RS_MDL16:
		len = dwMemLength;
		if (len >= 4)
		{
			LPBYTE pSample = (LPBYTE)pIns->pSample;
			LPBYTE ibuf = (LPBYTE)lpMemFile;
			DWORD bitbuf = bswapLE32(*((DWORD *)ibuf));
			UINT bitnum = 32;
			BYTE dlt = 0, lowbyte = 0;
			LPBYTE ibufend = (LPBYTE)lpMemFile + dwMemLength - 1;
			ibuf += 4;
			for (UINT j=0; j<pIns->nLength && ibuf < ibufend; j++)
			{
				BYTE hibyte;
				BYTE sign;
				if (nFlags == RS_MDL16) lowbyte = (BYTE)MDLReadBits(bitbuf, bitnum, ibuf, 8);
				sign = (BYTE)MDLReadBits(bitbuf, bitnum, ibuf, 1);
				if (MDLReadBits(bitbuf, bitnum, ibuf, 1))
				{
					hibyte = (BYTE)MDLReadBits(bitbuf, bitnum, ibuf, 3);
				} else
				{
					hibyte = 8;
					while (ibuf < ibufend && !MDLReadBits(bitbuf, bitnum, ibuf, 1))
						hibyte += 0x10;
					if (ibuf < ibufend)
						hibyte += MDLReadBits(bitbuf, bitnum, ibuf, 4);
				}
				if (sign) hibyte = ~hibyte;
				dlt += hibyte;
				if (nFlags != RS_MDL16)
					pSample[j] = dlt;
				else
				{
					pSample[j<<1] = lowbyte;
					pSample[(j<<1)+1] = dlt;
				}
			}
		}
		break;

	case RS_DMF8:
	case RS_DMF16:
		len = dwMemLength;
		if (len >= 4)
		{
			UINT maxlen = pIns->nLength;
			if (pIns->uFlags & CHN_16BIT) maxlen <<= 1;
			LPBYTE ibuf = (LPBYTE)lpMemFile, ibufmax = (LPBYTE)(lpMemFile+dwMemLength);
			len = DMFUnpack((LPBYTE)pIns->pSample, ibuf, ibufmax, maxlen);
		}
		break;

#ifdef MODPLUG_TRACKER
	// PCM 24-bit signed -> load sample, and normalize it to 16-bit
	case RS_PCM24S:
	case RS_PCM32S:
		len = pIns->nLength * 3;
		if (nFlags == RS_PCM32S) len += pIns->nLength;
		if (len > dwMemLength) break;
		if (len > 4*8)
		{
			UINT slsize = (nFlags == RS_PCM32S) ? 4 : 3;
			LPBYTE pSrc = (LPBYTE)lpMemFile;
			LONG max = 255;
			if (nFlags == RS_PCM32S) pSrc++;
			for (UINT j=0; j<len; j+=slsize)
			{
				LONG l = ((((pSrc[j+2] << 8) + pSrc[j+1]) << 8) + pSrc[j]) << 8;
				l /= 256;
				if (l > max) max = l;
				if (-l > max) max = -l;
			}
			max = (max / 128) + 1;
			int16_t *pDest = (int16_t *)pIns->pSample;
			for (UINT k=0; k<len; k+=slsize)
			{
				LONG l = ((((pSrc[k+2] << 8) + pSrc[k+1]) << 8) + pSrc[k]) << 8;
				*pDest++ = (uint16_t)(l / max);
			}
		}
		break;

	// Stereo PCM 24-bit signed -> load sample, and normalize it to 16-bit
	case RS_STIPCM24S:
	case RS_STIPCM32S:
		len = pIns->nLength * 6;
		if (nFlags == RS_STIPCM32S) len += pIns->nLength * 2;
		if (len > dwMemLength) break;
		if (len > 8*8)
		{
			UINT slsize = (nFlags == RS_STIPCM32S) ? 4 : 3;
			LPBYTE pSrc = (LPBYTE)lpMemFile;
			LONG max = 255;
			if (nFlags == RS_STIPCM32S) pSrc++;
			for (UINT j=0; j<len; j+=slsize)
			{
				LONG l = ((((pSrc[j+2] << 8) + pSrc[j+1]) << 8) + pSrc[j]) << 8;
				l /= 256;
				if (l > max) max = l;
				if (-l > max) max = -l;
			}
			max = (max / 128) + 1;
			int16_t *pDest = (int16_t *)pIns->pSample;
			for (UINT k=0; k<len; k+=slsize)
			{
				LONG lr = ((((pSrc[k+2] << 8) + pSrc[k+1]) << 8) + pSrc[k]) << 8;
				k += slsize;
				LONG ll = ((((pSrc[k+2] << 8) + pSrc[k+1]) << 8) + pSrc[k]) << 8;
				pDest[0] = (int16_t)ll;
				pDest[1] = (int16_t)lr;
				pDest += 2;
			}
		}
		break;

	// 16-bit signed big endian interleaved stereo
	case RS_STIPCM16M:
		{
			len = pIns->nLength;
			if (len*4 > dwMemLength) len = dwMemLength >> 2;
			LPCBYTE psrc = (LPCBYTE)lpMemFile;
			int16_t *pSample = (int16_t *)pIns->pSample;
			for (UINT j=0; j<len; j++)
			{
				pSample[j*2] = (int16_t)(((UINT)psrc[0] << 8) | (psrc[1]));
				pSample[j*2+1] = (int16_t)(((UINT)psrc[2] << 8) | (psrc[3]));
				psrc += 4;
			}
			len *= 4;
		}
		break;

#endif // MODPLUG_TRACKER
#endif // !MODPLUG_FASTSOUNDLIB
#endif // !MODPLUG_BASIC_SUPPORT

	// Default: 8-bit signed PCM data
	default:
		len = pIns->nLength;
		if (len > dwMemLength) len = pIns->nLength = dwMemLength;
		memcpy(pIns->pSample, lpMemFile, len);
	}
	if (len > dwMemLength)
	{
		if (pIns->pSample)
		{
			pIns->nLength = 0;
			FreeSample(pIns->pSample);
			pIns->pSample = NULL;
		}
		return 0;
	}
	AdjustSampleLoop(pIns);
	return len;
}


void CSoundFile::AdjustSampleLoop(MODINSTRUMENT *pIns)
//----------------------------------------------------
{
	if (!pIns->pSample) return;
	if (pIns->nLength > MAX_SAMPLE_LENGTH) pIns->nLength = MAX_SAMPLE_LENGTH;
	if (pIns->nLoopEnd > pIns->nLength) pIns->nLoopEnd = pIns->nLength;
	if (pIns->nLoopStart > pIns->nLength+2) pIns->nLoopStart = pIns->nLength+2;
	if (pIns->nLoopStart+2 >= pIns->nLoopEnd)
	{
		pIns->nLoopStart = pIns->nLoopEnd = 0;
		pIns->uFlags &= ~CHN_LOOP;
	}
	UINT len = pIns->nLength;
	if (pIns->uFlags & CHN_16BIT)
	{
		int16_t *pSample = (int16_t *)pIns->pSample;
		// Adjust end of sample
		if (pIns->uFlags & CHN_STEREO)
		{
			pSample[len*2+6] = pSample[len*2+4] = pSample[len*2+2] = pSample[len*2] = 0;
			pSample[len*2+7] = pSample[len*2+5] = pSample[len*2+3] = pSample[len*2+1] = 0;
		} else
		{
			pSample[len+4] = pSample[len+3] = pSample[len+2] = pSample[len+1] = pSample[len] = 0;
		}
		if ((pIns->uFlags & (CHN_LOOP|CHN_PINGPONGLOOP|CHN_STEREO)) == CHN_LOOP)
		{
			// Fix bad loops
			if ((pIns->nLoopEnd+3 >= pIns->nLength) || (m_nType & MOD_TYPE_S3M))
			{
				pSample[pIns->nLoopEnd] = pSample[pIns->nLoopStart];
				pSample[pIns->nLoopEnd+1] = pSample[pIns->nLoopStart+1];
				pSample[pIns->nLoopEnd+2] = pSample[pIns->nLoopStart+2];
				pSample[pIns->nLoopEnd+3] = pSample[pIns->nLoopStart+3];
				pSample[pIns->nLoopEnd+4] = pSample[pIns->nLoopStart+4];
			}
		}
	} else
	{
		signed char *pSample = pIns->pSample;
#ifndef MODPLUG_FASTSOUNDLIB
		// Crappy samples (except chiptunes) ?
		if ((pIns->nLength > 0x100) && (m_nType & (MOD_TYPE_MOD|MOD_TYPE_S3M))
		 && (!(pIns->uFlags & CHN_STEREO)))
		{
			int smpend = pSample[pIns->nLength-1], smpfix = 0, kscan;
			for (kscan=pIns->nLength-1; kscan>0; kscan--)
			{
				smpfix = pSample[kscan-1];
				if (smpfix != smpend) break;
			}
			int delta = smpfix - smpend;
			if (((!(pIns->uFlags & CHN_LOOP)) || (kscan > (int)pIns->nLoopEnd))
			 && ((delta < -8) || (delta > 8)))
			{
				while (kscan<(int)pIns->nLength)
				{
					if (!(kscan & 7))
					{
						if (smpfix > 0) smpfix--;
						if (smpfix < 0) smpfix++;
					}
					pSample[kscan] = (signed char)smpfix;
					kscan++;
				}
			}
		}
#endif
		// Adjust end of sample
		if (pIns->uFlags & CHN_STEREO)
		{
			pSample[len*2+6] = pSample[len*2+4] = pSample[len*2+2] = pSample[len*2] = 0;
			pSample[len*2+7] = pSample[len*2+5] = pSample[len*2+3] = pSample[len*2+1] = 0;

		} else
		{
			pSample[len+4] = pSample[len+3] = pSample[len+2] = pSample[len+1] = pSample[len] = 0;
		}
		if ((pIns->uFlags & (CHN_LOOP|CHN_PINGPONGLOOP|CHN_STEREO)) == CHN_LOOP)
		{
			if ((pIns->nLoopEnd+3 >= pIns->nLength) || (m_nType & (MOD_TYPE_MOD|MOD_TYPE_S3M)))
			{
				pSample[pIns->nLoopEnd] = pSample[pIns->nLoopStart];
				pSample[pIns->nLoopEnd+1] = pSample[pIns->nLoopStart+1];
				pSample[pIns->nLoopEnd+2] = pSample[pIns->nLoopStart+2];
				pSample[pIns->nLoopEnd+3] = pSample[pIns->nLoopStart+3];
				pSample[pIns->nLoopEnd+4] = pSample[pIns->nLoopStart+4];
			}
		}
	}
}


/////////////////////////////////////////////////////////////
// Transpose <-> Frequency conversions

// returns 8363*2^((transp*128+ftune)/(12*128))
DWORD CSoundFile::TransposeToFrequency(int transp, int ftune)
//-----------------------------------------------------------
{
#if defined(_MSC_VER) && defined(_M_IX86)
	const float _fbase = 8363;
	const float _factor = 1.0f/(12.0f*128.0f);
	int result;
	DWORD freq;

	transp = (transp << 7) + ftune;
	_asm {
	fild transp
	fld _factor
	fmulp st(1), st(0)
	fist result
	fisub result
	f2xm1
	fild result
	fld _fbase
	fscale
	fstp st(1)
	fmul st(1), st(0)
	faddp st(1), st(0)
	fistp freq
	}
	UINT derr = freq % 11025;
	if (derr <= 8) freq -= derr;
	if (derr >= 11015) freq += 11025-derr;
	derr = freq % 1000;
	if (derr <= 5) freq -= derr;
	if (derr >= 995) freq += 1000-derr;
	return freq;
#else
	//---GCCFIX:  Removed assembly.
	return (DWORD)(8363*pow(2, (double)(transp*128+ftune)/(1536)));
#endif
}


// returns 12*128*log2(freq/8363)
int CSoundFile::FrequencyToTranspose(DWORD freq)
//----------------------------------------------
{
#if defined(_MSC_VER) && defined(_M_IX86)
	const float _f1_8363 = 1.0f / 8363.0f;
	const float _factor = 128 * 12;
	LONG result;

	if (!freq) return 0;
	_asm {
	fld _factor
	fild freq
	fld _f1_8363
	fmulp st(1), st(0)
	fyl2x
	fistp result
	}
	return result;
#else
	//---GCCFIX: Removed assembly.
	return int(1536*(log(freq/8363.0)/log(2.0)));
#endif
}


void CSoundFile::FrequencyToTranspose(MODINSTRUMENT *psmp)
//--------------------------------------------------------
{
	int f2t = FrequencyToTranspose(psmp->nC4Speed);
	int transp = f2t >> 7;
	int ftune = f2t & 0x7F;
	if (ftune > 80)
	{
		transp++;
		ftune -= 128;
	}
	if (transp > 127) transp = 127;
	if (transp < -127) transp = -127;
	psmp->RelativeTone = transp;
	psmp->nFineTune = ftune;
}


void CSoundFile::CheckCPUUsage(UINT nCPU)
//---------------------------------------
{
	if (nCPU > 100) nCPU = 100;
	gnCPUUsage = nCPU;
	if (nCPU < 90)
	{
		m_dwSongFlags &= ~SONG_CPUVERYHIGH;
	} else
	if ((m_dwSongFlags & SONG_CPUVERYHIGH) && (nCPU >= 94))
	{
		UINT i=MAX_CHANNELS;
		while (i >= 8)
		{
			i--;
			if (Chn[i].nLength)
			{
				Chn[i].nLength = Chn[i].nPos = 0;
				nCPU -= 2;
				if (nCPU < 94) break;
			}
		}
	} else
	if (nCPU > 90)
	{
		m_dwSongFlags |= SONG_CPUVERYHIGH;
	}
}


BOOL CSoundFile::SetPatternName(UINT nPat, LPCSTR lpszName)
//---------------------------------------------------------
{
        char szName[MAX_PATTERNNAME] = "";
	// check input arguments
	if (nPat >= MAX_PATTERNS) return FALSE;
	if (lpszName == NULL) return(FALSE);

	if (lpszName) lstrcpyn(szName, lpszName, MAX_PATTERNNAME);
	szName[MAX_PATTERNNAME-1] = 0;
	if (!m_lpszPatternNames) m_nPatternNames = 0;
	if (nPat >= m_nPatternNames)
	{
		if (!lpszName[0]) return TRUE;
		UINT len = (nPat+1)*MAX_PATTERNNAME;
		char *p = new char[len];
		if (!p) return FALSE;
		memset(p, 0, len);
		if (m_lpszPatternNames)
		{
			memcpy(p, m_lpszPatternNames, m_nPatternNames * MAX_PATTERNNAME);
			delete [] m_lpszPatternNames;
			m_lpszPatternNames = NULL;
		}
		m_lpszPatternNames = p;
		m_nPatternNames = nPat + 1;
	}
	memcpy(m_lpszPatternNames + nPat * MAX_PATTERNNAME, szName, MAX_PATTERNNAME);
	return TRUE;
}


BOOL CSoundFile::GetPatternName(UINT nPat, LPSTR lpszName, UINT cbSize) const
//---------------------------------------------------------------------------
{
	if ((!lpszName) || (!cbSize)) return FALSE;
	lpszName[0] = 0;
	if (cbSize > MAX_PATTERNNAME) cbSize = MAX_PATTERNNAME;
	if ((m_lpszPatternNames) && (nPat < m_nPatternNames))
	{
		memcpy(lpszName, m_lpszPatternNames + nPat * MAX_PATTERNNAME, cbSize);
		lpszName[cbSize-1] = 0;
		return TRUE;
	}
	return FALSE;
}


#ifndef MODPLUG_FASTSOUNDLIB

UINT CSoundFile::DetectUnusedSamples(BOOL *pbIns)
//-----------------------------------------------
{
	UINT nExt = 0;

	if (!pbIns) return 0;
	if (m_nInstruments)
	{
		memset(pbIns, 0, MAX_SAMPLES * sizeof(BOOL));
		for (UINT ipat=0; ipat<MAX_PATTERNS; ipat++)
		{
			MODCOMMAND *p = Patterns[ipat];
			if (p)
			{
				UINT jmax = PatternSize[ipat] * m_nChannels;
				for (UINT j=0; j<jmax; j++, p++)
				{
					if ((p->note) && (p->note <= NOTE_MAX))
					{
						if ((p->instr) && (p->instr < MAX_INSTRUMENTS))
						{
							INSTRUMENTHEADER *penv = Headers[p->instr];
							if (penv)
							{
								UINT n = penv->Keyboard[p->note-1];
								if (n < MAX_SAMPLES) pbIns[n] = TRUE;
							}
						} else
						{
							for (UINT k=1; k<=m_nInstruments; k++)
							{
								INSTRUMENTHEADER *penv = Headers[k];
								if (penv)
								{
									UINT n = penv->Keyboard[p->note-1];
									if (n < MAX_SAMPLES) pbIns[n] = TRUE;
								}
							}
						}
					}
				}
			}
		}
		for (UINT ichk=1; ichk<=m_nSamples; ichk++)
		{
			if ((!pbIns[ichk]) && (Ins[ichk].pSample)) nExt++;
		}
	}
	return nExt;
}


BOOL CSoundFile::RemoveSelectedSamples(BOOL *pbIns)
//-------------------------------------------------
{
	if (!pbIns) return FALSE;
	for (UINT j=1; j<MAX_SAMPLES; j++)
	{
		if ((!pbIns[j]) && (Ins[j].pSample))
		{
			DestroySample(j);
			if ((j == m_nSamples) && (j > 1)) m_nSamples--;
		}
	}
	return TRUE;
}


BOOL CSoundFile::DestroySample(UINT nSample)
//------------------------------------------
{
	if ((!nSample) || (nSample >= MAX_SAMPLES)) return FALSE;
	if (!Ins[nSample].pSample) return TRUE;
	MODINSTRUMENT *pins = &Ins[nSample];
	signed char *pSample = pins->pSample;
	pins->pSample = NULL;
	pins->nLength = 0;
	pins->uFlags &= ~(CHN_16BIT);
	for (UINT i=0; i<MAX_CHANNELS; i++)
	{
		if (Chn[i].pSample == pSample)
		{
			Chn[i].nPos = Chn[i].nLength = 0;
			Chn[i].pSample = Chn[i].pCurrentSample = NULL;
		}
	}
	FreeSample(pSample);
	return TRUE;
}

#endif // MODPLUG_FASTSOUNDLIB
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

//#include "stdafx.h"
//#include "sndfile.h"
//#include "tables.h"

#ifdef MODPLUG_TRACKER
#define ENABLE_STEREOVU
#endif

// Volume ramp length, in 1/10 ms
#define VOLUMERAMPLEN	146	// 1.46ms = 64 samples at 44.1kHz

// VU-Meter
#define VUMETER_DECAY		4

// SNDMIX: These are global flags for playback control (first two configurable via SetMixConfig)
UINT CSoundFile::m_nStereoSeparation = 128;
UINT CSoundFile::m_nMaxMixChannels = 32;
LONG CSoundFile::m_nStreamVolume = 0x8000;
// Mixing Configuration (SetWaveConfig)
DWORD CSoundFile::gdwSysInfo = 0;
DWORD CSoundFile::gnChannels = 1;
DWORD CSoundFile::gdwSoundSetup = 0;
DWORD CSoundFile::gdwMixingFreq = 44100;
DWORD CSoundFile::gnBitsPerSample = 16;
// Mixing data initialized in
UINT CSoundFile::gnAGC = AGC_UNITY;
UINT CSoundFile::gnVolumeRampSamples = 64;
UINT CSoundFile::gnVUMeter = 0;
UINT CSoundFile::gnCPUUsage = 0;
LPSNDMIXHOOKPROC CSoundFile::gpSndMixHook = NULL;
PMIXPLUGINCREATEPROC CSoundFile::gpMixPluginCreateProc = NULL;
LONG gnDryROfsVol = 0;
LONG gnDryLOfsVol = 0;
LONG gnRvbROfsVol = 0;
LONG gnRvbLOfsVol = 0;
int gbInitPlugins = 0;

typedef DWORD (MPPASMCALL * LPCONVERTPROC)(LPVOID, int *, DWORD, LPLONG, LPLONG);

extern DWORD MPPASMCALL X86_Convert32To8(LPVOID lpBuffer, int *, DWORD nSamples, LPLONG, LPLONG);
extern DWORD MPPASMCALL X86_Convert32To16(LPVOID lpBuffer, int *, DWORD nSamples, LPLONG, LPLONG);
extern DWORD MPPASMCALL X86_Convert32To24(LPVOID lpBuffer, int *, DWORD nSamples, LPLONG, LPLONG);
extern DWORD MPPASMCALL X86_Convert32To32(LPVOID lpBuffer, int *, DWORD nSamples, LPLONG, LPLONG);
extern UINT MPPASMCALL X86_AGC(int *pBuffer, UINT nSamples, UINT nAGC);
extern VOID MPPASMCALL X86_Dither(int *pBuffer, UINT nSamples, UINT nBits);
extern VOID MPPASMCALL X86_InterleaveFrontRear(int *pFrontBuf, int *pRearBuf, DWORD nSamples);
extern VOID MPPASMCALL X86_StereoFill(int *pBuffer, UINT nSamples, LPLONG lpROfs, LPLONG lpLOfs);
extern VOID MPPASMCALL X86_MonoFromStereo(int *pMixBuf, UINT nSamples);

extern int MixSoundBuffer[MIXBUFFERSIZE*4];
extern int MixRearBuffer[MIXBUFFERSIZE*2];
UINT gnReverbSend;


// Log tables for pre-amp
// We don't want the tracker to get too loud
const UINT PreAmpTable[16] =
{
	0x60, 0x60, 0x60, 0x70,	// 0-7
	0x80, 0x88, 0x90, 0x98,	// 8-15
	0xA0, 0xA4, 0xA8, 0xB0,	// 16-23
	0xB4, 0xB8, 0xBC, 0xC0,	// 24-31
};

const UINT PreAmpAGCTable[16] =
{
	0x60, 0x60, 0x60, 0x60,
	0x68, 0x70, 0x78, 0x80,
	0x84, 0x88, 0x8C, 0x90,
	0x94, 0x98, 0x9C, 0xA0,
};


// Return (a*b)/c - no divide error
int _muldiv(long a, long b, long c)
{
#if defined(_MSC_VER) && defined(_M_IX86)
	int sign, result;
	_asm {
	mov eax, a
	mov ebx, b
	or eax, eax
	mov edx, eax
	jge aneg
	neg eax
aneg:
	xor edx, ebx
	or ebx, ebx
	mov ecx, c
	jge bneg
	neg ebx
bneg:
	xor edx, ecx
	or ecx, ecx
	mov sign, edx
	jge cneg
	neg ecx
cneg:
	mul ebx
	cmp edx, ecx
	jae diverr
	div ecx
	jmp ok
diverr:
	mov eax, 0x7fffffff
ok:
	mov edx, sign
	or edx, edx
	jge rneg
	neg eax
rneg:
	mov result, eax
	}
	return result;
#else
	return ((uint64_t) a * (uint64_t) b ) / c;
#endif
}

// Return (a*b+c/2)/c - no divide error
int _muldivr(long a, long b, long c)
{
#if defined(_MSC_VER) && defined(_M_IX86)
	int sign, result;
	_asm {
	mov eax, a
	mov ebx, b
	or eax, eax
	mov edx, eax
	jge aneg
	neg eax
aneg:
	xor edx, ebx
	or ebx, ebx
	mov ecx, c
	jge bneg
	neg ebx
bneg:
	xor edx, ecx
	or ecx, ecx
	mov sign, edx
	jge cneg
	neg ecx
cneg:
	mul ebx
	mov ebx, ecx
	shr ebx, 1
	add eax, ebx
	adc edx, 0
	cmp edx, ecx
	jae diverr
	div ecx
	jmp ok
diverr:
	mov eax, 0x7fffffff
ok:
	mov edx, sign
	or edx, edx
	jge rneg
	neg eax
rneg:
	mov result, eax
	}
	return result;
#else
	return ((uint64_t) a * (uint64_t) b + (c >> 1)) / c;
#endif
}


BOOL CSoundFile::InitPlayer(BOOL bReset)
//--------------------------------------
{
	if (m_nMaxMixChannels > MAX_CHANNELS) m_nMaxMixChannels = MAX_CHANNELS;
	if (gdwMixingFreq < 4000) gdwMixingFreq = 4000;
	if (gdwMixingFreq > MAX_SAMPLE_RATE) gdwMixingFreq = MAX_SAMPLE_RATE;
	gnVolumeRampSamples = (gdwMixingFreq * VOLUMERAMPLEN) / 100000;
	if (gnVolumeRampSamples < 8) gnVolumeRampSamples = 8;
	gnDryROfsVol = gnDryLOfsVol = 0;
	gnRvbROfsVol = gnRvbLOfsVol = 0;
	if (bReset)
	{
		gnVUMeter = 0;
		gnCPUUsage = 0;
	}
	gbInitPlugins = (bReset) ? 3 : 1;
	InitializeDSP(bReset);
	return TRUE;
}


BOOL CSoundFile::FadeSong(UINT msec)
//----------------------------------
{
	LONG nsamples = _muldiv(msec, gdwMixingFreq, 1000);
	if (nsamples <= 0) return FALSE;
	if (nsamples > 0x100000) nsamples = 0x100000;
	m_nBufferCount = nsamples;
	LONG nRampLength = m_nBufferCount;
	// Ramp everything down
	for (UINT noff=0; noff < m_nMixChannels; noff++)
	{
		MODCHANNEL *pramp = &Chn[ChnMix[noff]];
		if (!pramp) continue;
		pramp->nNewLeftVol = pramp->nNewRightVol = 0;
		pramp->nRightRamp = (-pramp->nRightVol << VOLUMERAMPPRECISION) / nRampLength;
		pramp->nLeftRamp = (-pramp->nLeftVol << VOLUMERAMPPRECISION) / nRampLength;
		pramp->nRampRightVol = pramp->nRightVol << VOLUMERAMPPRECISION;
		pramp->nRampLeftVol = pramp->nLeftVol << VOLUMERAMPPRECISION;
		pramp->nRampLength = nRampLength;
		pramp->dwFlags |= CHN_VOLUMERAMP;
	}
	m_dwSongFlags |= SONG_FADINGSONG;
	return TRUE;
}


BOOL CSoundFile::GlobalFadeSong(UINT msec)
//----------------------------------------
{
	if (m_dwSongFlags & SONG_GLOBALFADE) return FALSE;
	m_nGlobalFadeMaxSamples = _muldiv(msec, gdwMixingFreq, 1000);
	m_nGlobalFadeSamples = m_nGlobalFadeMaxSamples;
	m_dwSongFlags |= SONG_GLOBALFADE;
	return TRUE;
}


UINT CSoundFile::Read(LPVOID lpDestBuffer, UINT cbBuffer)
//-------------------------------------------------------
{
	LPBYTE lpBuffer = (LPBYTE)lpDestBuffer;
	LPCONVERTPROC pCvt = X86_Convert32To8;
	UINT lRead, lMax, lSampleSize, lCount, lSampleCount, nStat=0;
	LONG nVUMeterMin = 0x7FFFFFFF, nVUMeterMax = -0x7FFFFFFF;
	UINT nMaxPlugins;

	{
		nMaxPlugins = MAX_MIXPLUGINS;
		while ((nMaxPlugins > 0) && (!m_MixPlugins[nMaxPlugins-1].pMixPlugin)) nMaxPlugins--;
	}
	m_nMixStat = 0;
	lSampleSize = gnChannels;
	if (gnBitsPerSample == 16) { lSampleSize *= 2; pCvt = X86_Convert32To16; }
#ifndef MODPLUG_FASTSOUNDLIB
	else if (gnBitsPerSample == 24) { lSampleSize *= 3; pCvt = X86_Convert32To24; }
	else if (gnBitsPerSample == 32) { lSampleSize *= 4; pCvt = X86_Convert32To32; }
#endif
	lMax = cbBuffer / lSampleSize;
	if ((!lMax) || (!lpBuffer) || (!m_nChannels)) return 0;
	lRead = lMax;
	if (m_dwSongFlags & SONG_ENDREACHED) goto MixDone;
	while (lRead > 0)
	{
		// Update Channel Data
		if (!m_nBufferCount)
		{
#ifndef MODPLUG_FASTSOUNDLIB
			if (m_dwSongFlags & SONG_FADINGSONG)
			{
				m_dwSongFlags |= SONG_ENDREACHED;
				m_nBufferCount = lRead;
			} else
#endif
			if (!ReadNote())
			{
#ifndef MODPLUG_FASTSOUNDLIB
				if (!FadeSong(FADESONGDELAY))
#endif
				{
					m_dwSongFlags |= SONG_ENDREACHED;
					if (lRead == lMax) goto MixDone;
					m_nBufferCount = lRead;
				}
			}
		}
		lCount = m_nBufferCount;
		if (lCount > MIXBUFFERSIZE) lCount = MIXBUFFERSIZE;
		if (lCount > lRead) lCount = lRead;
		if (!lCount) break;
		lSampleCount = lCount;
#ifndef MODPLUG_NO_REVERB
		gnReverbSend = 0;
#endif
		// Resetting sound buffer
		X86_StereoFill(MixSoundBuffer, lSampleCount, &gnDryROfsVol, &gnDryLOfsVol);
		if (gnChannels >= 2)
		{
			lSampleCount *= 2;
			m_nMixStat += CreateStereoMix(lCount);
			ProcessStereoDSP(lCount);
		} else
		{
			m_nMixStat += CreateStereoMix(lCount);
			if (nMaxPlugins) ProcessPlugins(lCount);
			ProcessStereoDSP(lCount);
			X86_MonoFromStereo(MixSoundBuffer, lCount);
		}
		nStat++;
#ifndef NO_AGC
		// Automatic Gain Control
		if (gdwSoundSetup & SNDMIX_AGC) ProcessAGC(lSampleCount);
#endif
		UINT lTotalSampleCount = lSampleCount;
#ifndef MODPLUG_FASTSOUNDLIB
		// Multichannel
		if (gnChannels > 2)
		{
			X86_InterleaveFrontRear(MixSoundBuffer, MixRearBuffer, lSampleCount);
			lTotalSampleCount *= 2;
		}
		// Hook Function
		if (gpSndMixHook)
		{
			gpSndMixHook(MixSoundBuffer, lTotalSampleCount, gnChannels);
		}
#endif
		// Perform clipping + VU-Meter
		lpBuffer += pCvt(lpBuffer, MixSoundBuffer, lTotalSampleCount, &nVUMeterMin, &nVUMeterMax);
		// Buffer ready
		lRead -= lCount;
		m_nBufferCount -= lCount;
	}
MixDone:
	if (lRead) memset(lpBuffer, (gnBitsPerSample == 8) ? 0x80 : 0, lRead * lSampleSize);
	// VU-Meter
	nVUMeterMin >>= (24-MIXING_ATTENUATION);
	nVUMeterMax >>= (24-MIXING_ATTENUATION);
	if (nVUMeterMax < nVUMeterMin) nVUMeterMax = nVUMeterMin;
	if ((gnVUMeter = (UINT)(nVUMeterMax - nVUMeterMin)) > 0xFF) gnVUMeter = 0xFF;
	if (nStat) { m_nMixStat += nStat-1; m_nMixStat /= nStat; }
	return lMax - lRead;
}


/////////////////////////////////////////////////////////////////////////////
// Handles navigation/effects

BOOL CSoundFile::ProcessRow()
//---------------------------
{
	if (++m_nTickCount >= m_nMusicSpeed * (m_nPatternDelay+1) + m_nFrameDelay)
	{
		m_nPatternDelay = 0;
		m_nFrameDelay = 0;
		m_nTickCount = 0;
		m_nRow = m_nNextRow;
		// Reset Pattern Loop Effect
		if (m_nCurrentPattern != m_nNextPattern) m_nCurrentPattern = m_nNextPattern;
		// Check if pattern is valid
		if (!(m_dwSongFlags & SONG_PATTERNLOOP))
		{
			m_nPattern = (m_nCurrentPattern < MAX_ORDERS) ? Order[m_nCurrentPattern] : 0xFF;
			if ((m_nPattern < MAX_PATTERNS) && (!Patterns[m_nPattern])) m_nPattern = 0xFE;
			while (m_nPattern >= MAX_PATTERNS)
			{
				// End of song ?
				if ((m_nPattern == 0xFF) || (m_nCurrentPattern >= MAX_ORDERS))
				{
					//if (!m_nRepeatCount)
						return FALSE;     //never repeat entire song
					if (!m_nRestartPos)
					{
						m_nMusicSpeed = m_nDefaultSpeed;
						m_nMusicTempo = m_nDefaultTempo;
						m_nGlobalVolume = m_nDefaultGlobalVolume;
						for (UINT i=0; i<MAX_CHANNELS; i++)
						{
							Chn[i].dwFlags |= CHN_NOTEFADE | CHN_KEYOFF;
							Chn[i].nFadeOutVol = 0;
							if (i < m_nChannels)
							{
								Chn[i].nGlobalVol = ChnSettings[i].nVolume;
								Chn[i].nVolume = ChnSettings[i].nVolume;
								Chn[i].nPan = ChnSettings[i].nPan;
								Chn[i].nPanSwing = Chn[i].nVolSwing = 0;
								Chn[i].nOldVolParam = 0;
								Chn[i].nOldOffset = 0;
								Chn[i].nOldHiOffset = 0;
								Chn[i].nPortamentoDest = 0;
								if (!Chn[i].nLength)
								{
									Chn[i].dwFlags = ChnSettings[i].dwFlags;
									Chn[i].nLoopStart = 0;
									Chn[i].nLoopEnd = 0;
									Chn[i].pHeader = NULL;
									Chn[i].pSample = NULL;
									Chn[i].pInstrument = NULL;
								}
							}
						}
					}
//					if (m_nRepeatCount > 0) m_nRepeatCount--;
					m_nCurrentPattern = m_nRestartPos;
					m_nRow = 0;
					if ((Order[m_nCurrentPattern] >= MAX_PATTERNS) || (!Patterns[Order[m_nCurrentPattern]])) return FALSE;
				} else
				{
					m_nCurrentPattern++;
				}
				m_nPattern = (m_nCurrentPattern < MAX_ORDERS) ? Order[m_nCurrentPattern] : 0xFF;
				if ((m_nPattern < MAX_PATTERNS) && (!Patterns[m_nPattern])) m_nPattern = 0xFE;
			}
			m_nNextPattern = m_nCurrentPattern;
		}
		// Weird stuff?
		if ((m_nPattern >= MAX_PATTERNS) || (!Patterns[m_nPattern]) ||
			PatternSize[m_nPattern] == 0) return FALSE;
		// Should never happen
		if (m_nRow >= PatternSize[m_nPattern]) m_nRow = 0;
		m_nNextRow = m_nRow + 1;
		if (m_nNextRow >= PatternSize[m_nPattern])
		{
			if (!(m_dwSongFlags & SONG_PATTERNLOOP)) m_nNextPattern = m_nCurrentPattern + 1;
			m_nNextRow = m_nNextStartRow;
			m_nNextStartRow = 0;
		}
		// Reset channel values
		MODCHANNEL *pChn = Chn;
		MODCOMMAND *m = Patterns[m_nPattern] + m_nRow * m_nChannels;
		for (UINT nChn=0; nChn<m_nChannels; pChn++, nChn++, m++)
		{
			pChn->nRowNote = m->note;
			pChn->nRowInstr = m->instr;
			pChn->nRowVolCmd = m->volcmd;
			pChn->nRowVolume = m->vol;
			pChn->nRowCommand = m->command;
			pChn->nRowParam = m->param;

			pChn->nLeftVol = pChn->nNewLeftVol;
			pChn->nRightVol = pChn->nNewRightVol;
			pChn->dwFlags &= ~(CHN_PORTAMENTO | CHN_VIBRATO | CHN_TREMOLO | CHN_PANBRELLO);
			pChn->nCommand = 0;
		}
	}
	// Should we process tick0 effects?
	if (!m_nMusicSpeed) m_nMusicSpeed = 1;
	m_dwSongFlags |= SONG_FIRSTTICK;
	if (m_nTickCount)
	{
		m_dwSongFlags &= ~SONG_FIRSTTICK;
		if ((!(m_nType & MOD_TYPE_XM)) && (m_nTickCount < m_nMusicSpeed * (1 + m_nPatternDelay)))
		{
			if (!(m_nTickCount % m_nMusicSpeed)) m_dwSongFlags |= SONG_FIRSTTICK;
		}

	}
	// Update Effects
	return ProcessEffects();
}


////////////////////////////////////////////////////////////////////////////////////////////
// Handles envelopes & mixer setup

BOOL CSoundFile::ReadNote()
//-------------------------
{
	if (!ProcessRow()) return FALSE;
	////////////////////////////////////////////////////////////////////////////////////
	m_nTotalCount++;
	if (!m_nMusicTempo) return FALSE;
	m_nBufferCount = (gdwMixingFreq * 5 * m_nTempoFactor) / (m_nMusicTempo << 8);
	// Master Volume + Pre-Amplification / Attenuation setup
	DWORD nMasterVol;
	{
		int nchn32 = (m_nChannels < 32) ? m_nChannels : 31;
		if ((m_nType & MOD_TYPE_IT) && (m_nInstruments) && (nchn32 < 6)) nchn32 = 6;
		int realmastervol = m_nMasterVolume;
		if (realmastervol > 0x80)
		{
			realmastervol = 0x80 + ((realmastervol - 0x80) * (nchn32+4)) / 16;
		}
		UINT attenuation = (gdwSoundSetup & SNDMIX_AGC) ? PreAmpAGCTable[nchn32>>1] : PreAmpTable[nchn32>>1];
		DWORD mastervol = (realmastervol * (m_nSongPreAmp + 0x10)) >> 6;
		if (mastervol > 0x200) mastervol = 0x200;
		if ((m_dwSongFlags & SONG_GLOBALFADE) && (m_nGlobalFadeMaxSamples))
		{
			mastervol = _muldiv(mastervol, m_nGlobalFadeSamples, m_nGlobalFadeMaxSamples);
		}
		nMasterVol = (mastervol << 7) / attenuation;
		if (nMasterVol > 0x180) nMasterVol = 0x180;
	}
	////////////////////////////////////////////////////////////////////////////////////
	// Update channels data
	m_nMixChannels = 0;
	MODCHANNEL *pChn = Chn;
	for (UINT nChn=0; nChn<MAX_CHANNELS; nChn++,pChn++)
	{
		if ((pChn->dwFlags & CHN_NOTEFADE) && (!(pChn->nFadeOutVol|pChn->nRightVol|pChn->nLeftVol)))
		{
			pChn->nLength = 0;
			pChn->nROfs = pChn->nLOfs = 0;
		}
		// Check for unused channel
		if ((pChn->dwFlags & CHN_MUTE) || ((nChn >= m_nChannels) && (!pChn->nLength)))
		{
			pChn->nVUMeter = 0;
#ifdef ENABLE_STEREOVU
			pChn->nLeftVU = pChn->nRightVU = 0;
#endif
			continue;
		}
		// Reset channel data
		pChn->nInc = 0;
		pChn->nRealVolume = 0;
		pChn->nRealPan = pChn->nPan + pChn->nPanSwing;
		if (pChn->nRealPan < 0) pChn->nRealPan = 0;
		if (pChn->nRealPan > 256) pChn->nRealPan = 256;
		pChn->nRampLength = 0;
		// Calc Frequency
		if ((pChn->nPeriod)	&& (pChn->nLength))
		{
			int vol = pChn->nVolume + pChn->nVolSwing;

			if (vol < 0) vol = 0;
			if (vol > 256) vol = 256;
			// Tremolo
			if (pChn->dwFlags & CHN_TREMOLO)
			{
				UINT trempos = pChn->nTremoloPos & 0x3F;
				if (vol > 0)
				{
					int tremattn = (m_nType & MOD_TYPE_XM) ? 5 : 6;
					switch (pChn->nTremoloType & 0x03)
					{
					case 1:
						vol += (ModRampDownTable[trempos] * (int)pChn->nTremoloDepth) >> tremattn;
						break;
					case 2:
						vol += (ModSquareTable[trempos] * (int)pChn->nTremoloDepth) >> tremattn;
						break;
					case 3:
						vol += (ModRandomTable[trempos] * (int)pChn->nTremoloDepth) >> tremattn;
						break;
					default:
						vol += (ModSinusTable[trempos] * (int)pChn->nTremoloDepth) >> tremattn;
					}
				}
				if ((m_nTickCount) || ((m_nType & (MOD_TYPE_STM|MOD_TYPE_S3M|MOD_TYPE_IT)) && (!(m_dwSongFlags & SONG_ITOLDEFFECTS))))
				{
					pChn->nTremoloPos = (trempos + pChn->nTremoloSpeed) & 0x3F;
				}
			}
			// Tremor
			if (pChn->nCommand == CMD_TREMOR)
			{
				UINT n = (pChn->nTremorParam >> 4) + (pChn->nTremorParam & 0x0F);
				UINT ontime = pChn->nTremorParam >> 4;
				if ((!(m_nType & MOD_TYPE_IT)) || (m_dwSongFlags & SONG_ITOLDEFFECTS)) { n += 2; ontime++; }
				UINT tremcount = (UINT)pChn->nTremorCount;
				if (tremcount >= n) tremcount = 0;
				if ((m_nTickCount) || (m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT)))
				{
					if (tremcount >= ontime) vol = 0;
					pChn->nTremorCount = (BYTE)(tremcount + 1);
				}
				pChn->dwFlags |= CHN_FASTVOLRAMP;
			}
			// Clip volume
			if (vol < 0) vol = 0;
			if (vol > 0x100) vol = 0x100;
			vol <<= 6;
			// Process Envelopes
			if (pChn->pHeader)
			{
				INSTRUMENTHEADER *penv = pChn->pHeader;
				// Volume Envelope
				if ((pChn->dwFlags & CHN_VOLENV) && (penv->nVolEnv))
				{
					int envpos = pChn->nVolEnvPosition;
					UINT pt = penv->nVolEnv - 1;
					for (UINT i=0; i<(UINT)(penv->nVolEnv-1); i++)
					{
						if (envpos <= penv->VolPoints[i])
						{
							pt = i;
							break;
						}
					}
					int x2 = penv->VolPoints[pt];
					int x1, envvol;
					if (envpos >= x2)
					{
						envvol = penv->VolEnv[pt] << 2;
						x1 = x2;
					} else
					if (pt)
					{
						envvol = penv->VolEnv[pt-1] << 2;
						x1 = penv->VolPoints[pt-1];
					} else
					{
						envvol = 0;
						x1 = 0;
					}
					if (envpos > x2) envpos = x2;
					if ((x2 > x1) && (envpos > x1))
					{
						envvol += ((envpos - x1) * (((int)penv->VolEnv[pt]<<2) - envvol)) / (x2 - x1);
					}
					if (envvol < 0) envvol = 0;
					if (envvol > 256) envvol = 256;
					vol = (vol * envvol) >> 8;
				}
				// Panning Envelope
				if ((pChn->dwFlags & CHN_PANENV) && (penv->nPanEnv))
				{
					int envpos = pChn->nPanEnvPosition;
					UINT pt = penv->nPanEnv - 1;
					for (UINT i=0; i<(UINT)(penv->nPanEnv-1); i++)
					{
						if (envpos <= penv->PanPoints[i])
						{
							pt = i;
							break;
						}
					}
					int x2 = penv->PanPoints[pt], y2 = penv->PanEnv[pt];
					int x1, envpan;
					if (envpos >= x2)
					{
						envpan = y2;
						x1 = x2;
					} else
					if (pt)
					{
						envpan = penv->PanEnv[pt-1];
						x1 = penv->PanPoints[pt-1];
					} else
					{
						envpan = 128;
						x1 = 0;
					}
					if ((x2 > x1) && (envpos > x1))
					{
						envpan += ((envpos - x1) * (y2 - envpan)) / (x2 - x1);
					}
					if (envpan < 0) envpan = 0;
					if (envpan > 64) envpan = 64;
					int pan = pChn->nPan;
					if (pan >= 128)
					{
						pan += ((envpan - 32) * (256 - pan)) / 32;
					} else
					{
						pan += ((envpan - 32) * (pan)) / 32;
					}
					if (pan < 0) pan = 0;
					if (pan > 256) pan = 256;
					pChn->nRealPan = pan;
				}
				// FadeOut volume
				if (pChn->dwFlags & CHN_NOTEFADE)
				{
					UINT fadeout = penv->nFadeOut;
					if (fadeout)
					{
						pChn->nFadeOutVol -= fadeout << 1;
						if (pChn->nFadeOutVol <= 0) pChn->nFadeOutVol = 0;
						vol = (vol * pChn->nFadeOutVol) >> 16;
					} else
					if (!pChn->nFadeOutVol)
					{
						vol = 0;
					}
				}
				// Pitch/Pan separation
				if ((penv->nPPS) && (pChn->nRealPan) && (pChn->nNote))
				{
					int pandelta = (int)pChn->nRealPan + (int)((int)(pChn->nNote - penv->nPPC - 1) * (int)penv->nPPS) / (int)8;
					if (pandelta < 0) pandelta = 0;
					if (pandelta > 256) pandelta = 256;
					pChn->nRealPan = pandelta;
				}
			} else
			{
				// No Envelope: key off => note cut
				if (pChn->dwFlags & CHN_NOTEFADE) // 1.41-: CHN_KEYOFF|CHN_NOTEFADE
				{
					pChn->nFadeOutVol = 0;
					vol = 0;
				}
			}
			// vol is 14-bits
			if (vol)
			{
				// IMPORTANT: pChn->nRealVolume is 14 bits !!!
				// -> _muldiv( 14+8, 6+6, 18); => RealVolume: 14-bit result (22+12-20)
				pChn->nRealVolume = _muldiv(vol * m_nGlobalVolume, pChn->nGlobalVol * pChn->nInsVol, 1 << 20);
			}
			if (pChn->nPeriod < m_nMinPeriod) pChn->nPeriod = m_nMinPeriod;
			int period = pChn->nPeriod;
			if ((pChn->dwFlags & (CHN_GLISSANDO|CHN_PORTAMENTO)) ==	(CHN_GLISSANDO|CHN_PORTAMENTO))
			{
				period = GetPeriodFromNote(GetNoteFromPeriod(period), pChn->nFineTune, pChn->nC4Speed);
			}

			// Arpeggio ?
			if (pChn->nCommand == CMD_ARPEGGIO)
			{
				switch(m_nTickCount % 3)
				{
				case 1:	period = GetPeriodFromNote(pChn->nNote + (pChn->nArpeggio >> 4), pChn->nFineTune, pChn->nC4Speed); break;
				case 2:	period = GetPeriodFromNote(pChn->nNote + (pChn->nArpeggio & 0x0F), pChn->nFineTune, pChn->nC4Speed); break;
				}
			}

			if (m_dwSongFlags & SONG_AMIGALIMITS)
			{
				if (period < 113*4) period = 113*4;
				if (period > 856*4) period = 856*4;
			}

			// Pitch/Filter Envelope
			if ((pChn->pHeader) && (pChn->dwFlags & CHN_PITCHENV) && (pChn->pHeader->nPitchEnv))
			{
				INSTRUMENTHEADER *penv = pChn->pHeader;
				int envpos = pChn->nPitchEnvPosition;
				UINT pt = penv->nPitchEnv - 1;
				for (UINT i=0; i<(UINT)(penv->nPitchEnv-1); i++)
				{
					if (envpos <= penv->PitchPoints[i])
					{
						pt = i;
						break;
					}
				}
				int x2 = penv->PitchPoints[pt];
				int x1, envpitch;
				if (envpos >= x2)
				{
					envpitch = (((int)penv->PitchEnv[pt]) - 32) * 8;
					x1 = x2;
				} else
				if (pt)
				{
					envpitch = (((int)penv->PitchEnv[pt-1]) - 32) * 8;
					x1 = penv->PitchPoints[pt-1];
				} else
				{
					envpitch = 0;
					x1 = 0;
				}
				if (envpos > x2) envpos = x2;
				if ((x2 > x1) && (envpos > x1))
				{
					int envpitchdest = (((int)penv->PitchEnv[pt]) - 32) * 8;
					envpitch += ((envpos - x1) * (envpitchdest - envpitch)) / (x2 - x1);
				}
				if (envpitch < -256) envpitch = -256;
				if (envpitch > 256) envpitch = 256;
				// Filter Envelope: controls cutoff frequency
				if (penv->dwFlags & ENV_FILTER)
				{
#ifndef NO_FILTER
					SetupChannelFilter(pChn, (pChn->dwFlags & CHN_FILTER) ? FALSE : TRUE, envpitch);
#endif // NO_FILTER
				} else
				// Pitch Envelope
				{
					int l = envpitch;
					if (l < 0)
					{
						l = -l;
						if (l > 255) l = 255;
						period = _muldiv(period, LinearSlideUpTable[l], 0x10000);
					} else
					{
						if (l > 255) l = 255;
						period = _muldiv(period, LinearSlideDownTable[l], 0x10000);
					}
				}
			}

			// Vibrato
			if (pChn->dwFlags & CHN_VIBRATO)
			{
				UINT vibpos = pChn->nVibratoPos;
				LONG vdelta;
				switch (pChn->nVibratoType & 0x03)
				{
				case 1:
					vdelta = ModRampDownTable[vibpos];
					break;
				case 2:
					vdelta = ModSquareTable[vibpos];
					break;
				case 3:
					vdelta = ModRandomTable[vibpos];
					break;
				default:
					vdelta = ModSinusTable[vibpos];
				}
				UINT vdepth = ((m_nType != MOD_TYPE_IT) || (m_dwSongFlags & SONG_ITOLDEFFECTS)) ? 6 : 7;
				vdelta = (vdelta * (int)pChn->nVibratoDepth) >> vdepth;
				if ((m_dwSongFlags & SONG_LINEARSLIDES) && (m_nType & MOD_TYPE_IT))
				{
					LONG l = vdelta;
					if (l < 0)
					{
						l = -l;
						vdelta = _muldiv(period, LinearSlideDownTable[l >> 2], 0x10000) - period;
						if (l & 0x03) vdelta += _muldiv(period, FineLinearSlideDownTable[l & 0x03], 0x10000) - period;

					} else
					{
						vdelta = _muldiv(period, LinearSlideUpTable[l >> 2], 0x10000) - period;
						if (l & 0x03) vdelta += _muldiv(period, FineLinearSlideUpTable[l & 0x03], 0x10000) - period;

					}
				}
				period += vdelta;
				if ((m_nTickCount) || ((m_nType & MOD_TYPE_IT) && (!(m_dwSongFlags & SONG_ITOLDEFFECTS))))
				{
					pChn->nVibratoPos = (vibpos + pChn->nVibratoSpeed) & 0x3F;
				}
			}
			// Panbrello
			if (pChn->dwFlags & CHN_PANBRELLO)
			{
				UINT panpos = ((pChn->nPanbrelloPos+0x10) >> 2) & 0x3F;
				LONG pdelta;
				switch (pChn->nPanbrelloType & 0x03)
				{
				case 1:
					pdelta = ModRampDownTable[panpos];
					break;
				case 2:
					pdelta = ModSquareTable[panpos];
					break;
				case 3:
					pdelta = ModRandomTable[panpos];
					break;
				default:
					pdelta = ModSinusTable[panpos];
				}
				pChn->nPanbrelloPos += pChn->nPanbrelloSpeed;
				pdelta = ((pdelta * (int)pChn->nPanbrelloDepth) + 2) >> 3;
				pdelta += pChn->nRealPan;
				if (pdelta < 0) pdelta = 0;
				if (pdelta > 256) pdelta = 256;
				pChn->nRealPan = pdelta;
			}
			int nPeriodFrac = 0;
			// Instrument Auto-Vibrato
			if ((pChn->pInstrument) && (pChn->pInstrument->nVibDepth))
			{
				MODINSTRUMENT *pins = pChn->pInstrument;
				if (pins->nVibSweep == 0)
				{
					pChn->nAutoVibDepth = pins->nVibDepth << 8;
				} else
				{
					if (m_nType & MOD_TYPE_IT)
					{
						pChn->nAutoVibDepth += pins->nVibSweep << 3;
					} else
					if (!(pChn->dwFlags & CHN_KEYOFF))
					{
						pChn->nAutoVibDepth += (pins->nVibDepth << 8) /	pins->nVibSweep;
					}
					if ((pChn->nAutoVibDepth >> 8) > pins->nVibDepth)
						pChn->nAutoVibDepth = pins->nVibDepth << 8;
				}
				pChn->nAutoVibPos += pins->nVibRate;
				int val;
				switch(pins->nVibType)
				{
				case 4:	// Random
					val = ModRandomTable[pChn->nAutoVibPos & 0x3F];
					pChn->nAutoVibPos++;
					break;
				case 3:	// Ramp Down
					val = ((0x40 - (pChn->nAutoVibPos >> 1)) & 0x7F) - 0x40;
					break;
				case 2:	// Ramp Up
					val = ((0x40 + (pChn->nAutoVibPos >> 1)) & 0x7f) - 0x40;
					break;
				case 1:	// Square
					val = (pChn->nAutoVibPos & 128) ? +64 : -64;
					break;
				default:	// Sine
					val = ft2VibratoTable[pChn->nAutoVibPos & 255];
				}
				int n =	((val * pChn->nAutoVibDepth) >> 8);
				if (m_nType & MOD_TYPE_IT)
				{
					int df1, df2;
					if (n < 0)
					{
						n = -n;
						UINT n1 = n >> 8;
						df1 = LinearSlideUpTable[n1];
						df2 = LinearSlideUpTable[n1+1];
					} else
					{
						UINT n1 = n >> 8;
						df1 = LinearSlideDownTable[n1];
						df2 = LinearSlideDownTable[n1+1];
					}
					n >>= 2;
					period = _muldiv(period, df1 + ((df2-df1)*(n&0x3F)>>6), 256);
					nPeriodFrac = period & 0xFF;
					period >>= 8;
				} else
				{
					period += (n >> 6);
				}
			}
			// Final Period
			if (period <= m_nMinPeriod)
			{
				if (m_nType & MOD_TYPE_S3M) pChn->nLength = 0;
				period = m_nMinPeriod;
			}
			if (period > m_nMaxPeriod)
			{
				if ((m_nType & MOD_TYPE_IT) || (period >= 0x100000))
				{
					pChn->nFadeOutVol = 0;
					pChn->dwFlags |= CHN_NOTEFADE;
					pChn->nRealVolume = 0;
				}
				period = m_nMaxPeriod;
				nPeriodFrac = 0;
			}
			UINT freq = GetFreqFromPeriod(period, pChn->nC4Speed, nPeriodFrac);
			if ((m_nType & MOD_TYPE_IT) && (freq < 256))
			{
				pChn->nFadeOutVol = 0;
				pChn->dwFlags |= CHN_NOTEFADE;
				pChn->nRealVolume = 0;
			}
			UINT ninc = _muldiv(freq, 0x10000, gdwMixingFreq);
			if ((ninc >= 0xFFB0) && (ninc <= 0x10090)) ninc = 0x10000;
			if (m_nFreqFactor != 128) ninc = (ninc * m_nFreqFactor) >> 7;
			if (ninc > 0xFF0000) ninc = 0xFF0000;
			pChn->nInc = (ninc+1) & ~3;
		}

		// Increment envelope position
		if (pChn->pHeader)
		{
			INSTRUMENTHEADER *penv = pChn->pHeader;
			// Volume Envelope
			if (pChn->dwFlags & CHN_VOLENV)
			{
				// Increase position
				pChn->nVolEnvPosition++;
				// Volume Loop ?
				if (penv->dwFlags & ENV_VOLLOOP)
				{
					UINT volloopend = penv->VolPoints[penv->nVolLoopEnd];
					if (m_nType != MOD_TYPE_XM) volloopend++;
					if (pChn->nVolEnvPosition == volloopend)
					{
						pChn->nVolEnvPosition = penv->VolPoints[penv->nVolLoopStart];
						if ((penv->nVolLoopEnd == penv->nVolLoopStart) && (!penv->VolEnv[penv->nVolLoopStart])
						 && ((!(m_nType & MOD_TYPE_XM)) || (penv->nVolLoopEnd+1 == penv->nVolEnv)))
						{
							pChn->dwFlags |= CHN_NOTEFADE;
							pChn->nFadeOutVol = 0;
						}
					}
				}
				// Volume Sustain ?
				if ((penv->dwFlags & ENV_VOLSUSTAIN) && (!(pChn->dwFlags & CHN_KEYOFF)))
				{
					if (pChn->nVolEnvPosition == (UINT)penv->VolPoints[penv->nVolSustainEnd]+1)
						pChn->nVolEnvPosition = penv->VolPoints[penv->nVolSustainBegin];
				} else
				// End of Envelope ?
				if (pChn->nVolEnvPosition > penv->VolPoints[penv->nVolEnv - 1])
				{
					if ((m_nType & MOD_TYPE_IT) || (pChn->dwFlags & CHN_KEYOFF)) pChn->dwFlags |= CHN_NOTEFADE;
					pChn->nVolEnvPosition = penv->VolPoints[penv->nVolEnv - 1];
					if ((!penv->VolEnv[penv->nVolEnv-1]) && ((nChn >= m_nChannels) || (m_nType & MOD_TYPE_IT)))
					{
						pChn->dwFlags |= CHN_NOTEFADE;
						pChn->nFadeOutVol = 0;

						pChn->nRealVolume = 0;
					}
				}
			}
			// Panning Envelope
			if (pChn->dwFlags & CHN_PANENV)
			{
				pChn->nPanEnvPosition++;
				if (penv->dwFlags & ENV_PANLOOP)
				{
					UINT panloopend = penv->PanPoints[penv->nPanLoopEnd];
					if (m_nType != MOD_TYPE_XM) panloopend++;
					if (pChn->nPanEnvPosition == panloopend)
						pChn->nPanEnvPosition = penv->PanPoints[penv->nPanLoopStart];
				}
				// Panning Sustain ?
				if ((penv->dwFlags & ENV_PANSUSTAIN) && (pChn->nPanEnvPosition == (UINT)penv->PanPoints[penv->nPanSustainEnd]+1)
				 && (!(pChn->dwFlags & CHN_KEYOFF)))
				{
					// Panning sustained
					pChn->nPanEnvPosition = penv->PanPoints[penv->nPanSustainBegin];
				} else
				{
					if (pChn->nPanEnvPosition > penv->PanPoints[penv->nPanEnv - 1])
						pChn->nPanEnvPosition = penv->PanPoints[penv->nPanEnv - 1];
				}
			}
			// Pitch Envelope
			if (pChn->dwFlags & CHN_PITCHENV)
			{
				// Increase position
				pChn->nPitchEnvPosition++;
				// Pitch Loop ?
				if (penv->dwFlags & ENV_PITCHLOOP)
				{
					if (pChn->nPitchEnvPosition >= penv->PitchPoints[penv->nPitchLoopEnd])
						pChn->nPitchEnvPosition = penv->PitchPoints[penv->nPitchLoopStart];
				}
				// Pitch Sustain ?
				if ((penv->dwFlags & ENV_PITCHSUSTAIN) && (!(pChn->dwFlags & CHN_KEYOFF)))
				{
					if (pChn->nPitchEnvPosition == (UINT)penv->PitchPoints[penv->nPitchSustainEnd]+1)
						pChn->nPitchEnvPosition = penv->PitchPoints[penv->nPitchSustainBegin];
				} else
				{
					if (pChn->nPitchEnvPosition > penv->PitchPoints[penv->nPitchEnv - 1])
						pChn->nPitchEnvPosition = penv->PitchPoints[penv->nPitchEnv - 1];
				}
			}
		}
#ifdef MODPLUG_PLAYER
		// Limit CPU -> > 80% -> don't ramp
		if ((gnCPUUsage >= 80) && (!pChn->nRealVolume))
		{
			pChn->nLeftVol = pChn->nRightVol = 0;
		}
#endif // MODPLUG_PLAYER
		// Volume ramping
		pChn->dwFlags &= ~CHN_VOLUMERAMP;
		if ((pChn->nRealVolume) || (pChn->nLeftVol) || (pChn->nRightVol))
			pChn->dwFlags |= CHN_VOLUMERAMP;
#ifdef MODPLUG_PLAYER
		// Decrease VU-Meter
		if (pChn->nVUMeter > VUMETER_DECAY)	pChn->nVUMeter -= VUMETER_DECAY; else pChn->nVUMeter = 0;
#endif // MODPLUG_PLAYER
#ifdef ENABLE_STEREOVU
		if (pChn->nLeftVU > VUMETER_DECAY) pChn->nLeftVU -= VUMETER_DECAY; else pChn->nLeftVU = 0;
		if (pChn->nRightVU > VUMETER_DECAY) pChn->nRightVU -= VUMETER_DECAY; else pChn->nRightVU = 0;
#endif
		// Check for too big nInc
		if (((pChn->nInc >> 16) + 1) >= (LONG)(pChn->nLoopEnd - pChn->nLoopStart)) pChn->dwFlags &= ~CHN_LOOP;
		pChn->nNewRightVol = pChn->nNewLeftVol = 0;
		pChn->pCurrentSample = ((pChn->pSample) && (pChn->nLength) && (pChn->nInc)) ? pChn->pSample : NULL;
		if (pChn->pCurrentSample)
		{
			// Update VU-Meter (nRealVolume is 14-bit)
#ifdef MODPLUG_PLAYER
			UINT vutmp = pChn->nRealVolume >> (14 - 8);
			if (vutmp > 0xFF) vutmp = 0xFF;
			if (pChn->nVUMeter >= 0x100) pChn->nVUMeter = vutmp;
			vutmp >>= 1;
			if (pChn->nVUMeter < vutmp)	pChn->nVUMeter = vutmp;
#endif // MODPLUG_PLAYER
#ifdef ENABLE_STEREOVU
			UINT vul = (pChn->nRealVolume * pChn->nRealPan) >> 14;
			if (vul > 127) vul = 127;
			if (pChn->nLeftVU > 127) pChn->nLeftVU = (BYTE)vul;
			vul >>= 1;
			if (pChn->nLeftVU < vul) pChn->nLeftVU = (BYTE)vul;
			UINT vur = (pChn->nRealVolume * (256-pChn->nRealPan)) >> 14;
			if (vur > 127) vur = 127;
			if (pChn->nRightVU > 127) pChn->nRightVU = (BYTE)vur;
			vur >>= 1;
			if (pChn->nRightVU < vur) pChn->nRightVU = (BYTE)vur;
#endif
#ifdef MODPLUG_TRACKER
			UINT kChnMasterVol = (pChn->dwFlags & CHN_EXTRALOUD) ? 0x100 : nMasterVol;
#else
#define		kChnMasterVol	nMasterVol
#endif // MODPLUG_TRACKER
			// Adjusting volumes
			if (gnChannels >= 2)
			{
				int pan = ((int)pChn->nRealPan) - 128;
				pan *= (int)m_nStereoSeparation;
				pan /= 128;
				pan += 128;

				if (pan < 0) pan = 0;
				if (pan > 256) pan = 256;
#ifndef MODPLUG_FASTSOUNDLIB
				if (gdwSoundSetup & SNDMIX_REVERSESTEREO) pan = 256 - pan;
#endif
				LONG realvol = (pChn->nRealVolume * kChnMasterVol) >> (8-1);
				if (gdwSoundSetup & SNDMIX_SOFTPANNING)
				{
					if (pan < 128)
					{
						pChn->nNewLeftVol = (realvol * pan) >> 8;
						pChn->nNewRightVol = (realvol * 128) >> 8;
					} else
					{
						pChn->nNewLeftVol = (realvol * 128) >> 8;
						pChn->nNewRightVol = (realvol * (256 - pan)) >> 8;
					}
				} else
				{
					pChn->nNewLeftVol = (realvol * pan) >> 8;
					pChn->nNewRightVol = (realvol * (256 - pan)) >> 8;
				}
			} else
			{
				pChn->nNewRightVol = (pChn->nRealVolume * kChnMasterVol) >> 8;
				pChn->nNewLeftVol = pChn->nNewRightVol;
			}
			// Clipping volumes
			if (pChn->nNewRightVol > 0xFFFF) pChn->nNewRightVol = 0xFFFF;
			if (pChn->nNewLeftVol > 0xFFFF) pChn->nNewLeftVol = 0xFFFF;
			// Check IDO
			if (gdwSoundSetup & SNDMIX_NORESAMPLING)
			{
				pChn->dwFlags |= CHN_NOIDO;
			} else
			{
				pChn->dwFlags &= ~(CHN_NOIDO|CHN_HQSRC);
				if( pChn->nInc == 0x10000 )
				{	pChn->dwFlags |= CHN_NOIDO;
				}
				else
				{	if( ((gdwSoundSetup & SNDMIX_HQRESAMPLER) == 0) && ((gdwSoundSetup & SNDMIX_ULTRAHQSRCMODE) == 0) )
					{	if (pChn->nInc >= 0xFF00) pChn->dwFlags |= CHN_NOIDO;
					}
				}
			}
			pChn->nNewRightVol >>= MIXING_ATTENUATION;
			pChn->nNewLeftVol >>= MIXING_ATTENUATION;
			pChn->nRightRamp = pChn->nLeftRamp = 0;
			// Dolby Pro-Logic Surround
			if ((pChn->dwFlags & CHN_SURROUND) && (gnChannels <= 2)) pChn->nNewLeftVol = - pChn->nNewLeftVol;
			// Checking Ping-Pong Loops
			if (pChn->dwFlags & CHN_PINGPONGFLAG) pChn->nInc = -pChn->nInc;
			// Setting up volume ramp
			if ((pChn->dwFlags & CHN_VOLUMERAMP)
			 && ((pChn->nRightVol != pChn->nNewRightVol)
			  || (pChn->nLeftVol != pChn->nNewLeftVol)))
			{
				LONG nRampLength = gnVolumeRampSamples;
				LONG nRightDelta = ((pChn->nNewRightVol - pChn->nRightVol) << VOLUMERAMPPRECISION);
				LONG nLeftDelta = ((pChn->nNewLeftVol - pChn->nLeftVol) << VOLUMERAMPPRECISION);
#ifndef MODPLUG_FASTSOUNDLIB
				if ((gdwSoundSetup & SNDMIX_DIRECTTODISK)
				 || ((gdwSysInfo & (SYSMIX_ENABLEMMX|SYSMIX_FASTCPU))
				  && (gdwSoundSetup & SNDMIX_HQRESAMPLER) && (gnCPUUsage <= 20)))
				{
					if ((pChn->nRightVol|pChn->nLeftVol) && (pChn->nNewRightVol|pChn->nNewLeftVol) && (!(pChn->dwFlags & CHN_FASTVOLRAMP)))
					{
						nRampLength = m_nBufferCount;
						if (nRampLength > (1 << (VOLUMERAMPPRECISION-1))) nRampLength = (1 << (VOLUMERAMPPRECISION-1));
						if (nRampLength < (LONG)gnVolumeRampSamples) nRampLength = gnVolumeRampSamples;
					}
				}
#endif
				pChn->nRightRamp = nRightDelta / nRampLength;
				pChn->nLeftRamp = nLeftDelta / nRampLength;
				pChn->nRightVol = pChn->nNewRightVol - ((pChn->nRightRamp * nRampLength) >> VOLUMERAMPPRECISION);
				pChn->nLeftVol = pChn->nNewLeftVol - ((pChn->nLeftRamp * nRampLength) >> VOLUMERAMPPRECISION);
				if (pChn->nRightRamp|pChn->nLeftRamp)
				{
					pChn->nRampLength = nRampLength;
				} else
				{
					pChn->dwFlags &= ~CHN_VOLUMERAMP;
					pChn->nRightVol = pChn->nNewRightVol;
					pChn->nLeftVol = pChn->nNewLeftVol;
				}
			} else
			{
				pChn->dwFlags &= ~CHN_VOLUMERAMP;
				pChn->nRightVol = pChn->nNewRightVol;
				pChn->nLeftVol = pChn->nNewLeftVol;
			}
			pChn->nRampRightVol = pChn->nRightVol << VOLUMERAMPPRECISION;
			pChn->nRampLeftVol = pChn->nLeftVol << VOLUMERAMPPRECISION;
			// Adding the channel in the channel list
			ChnMix[m_nMixChannels++] = nChn;
			if (m_nMixChannels >= MAX_CHANNELS) break;
		} else
		{
#ifdef ENABLE_STEREOVU
			// Note change but no sample
			if (pChn->nLeftVU > 128) pChn->nLeftVU = 0;
			if (pChn->nRightVU > 128) pChn->nRightVU = 0;
#endif
			if (pChn->nVUMeter > 0xFF) pChn->nVUMeter = 0;
			pChn->nLeftVol = pChn->nRightVol = 0;
			pChn->nLength = 0;
		}
	}
	// Checking Max Mix Channels reached: ordering by volume
	if ((m_nMixChannels >= m_nMaxMixChannels) && (!(gdwSoundSetup & SNDMIX_DIRECTTODISK)))
	{
		for (UINT i=0; i<m_nMixChannels; i++)
		{
			UINT j=i;
			while ((j+1<m_nMixChannels) && (Chn[ChnMix[j]].nRealVolume < Chn[ChnMix[j+1]].nRealVolume))
			{
				UINT n = ChnMix[j];
				ChnMix[j] = ChnMix[j+1];
				ChnMix[j+1] = n;
				j++;
			}
		}
	}
	if (m_dwSongFlags & SONG_GLOBALFADE)
	{
		if (!m_nGlobalFadeSamples)
		{
			m_dwSongFlags |= SONG_ENDREACHED;
			return FALSE;
		}
		if (m_nGlobalFadeSamples > m_nBufferCount)
			m_nGlobalFadeSamples -= m_nBufferCount;
		else
			m_nGlobalFadeSamples = 0;
	}
	return TRUE;
}
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

//#include "stdafx.h"
//#include "sndfile.h"

#ifdef MODPLUG_FASTSOUNDLIB
#define MODPLUG_NO_REVERB
#endif


// Delayed Surround Filters
#ifndef MODPLUG_FASTSOUNDLIB
#define nDolbyHiFltAttn		6
#define nDolbyHiFltMask		3
#define DOLBYATTNROUNDUP	31
#else
#define nDolbyHiFltAttn		3
#define nDolbyHiFltMask		3
#define DOLBYATTNROUNDUP	3
#endif

// Bass Expansion
#define XBASS_DELAY			14	// 2.5 ms

// Buffer Sizes
#define XBASSBUFFERSIZE		64		// 2 ms at 50KHz
#define FILTERBUFFERSIZE	64		// 1.25 ms
#define SURROUNDBUFFERSIZE	((MAX_SAMPLE_RATE * 50) / 1000)
#define REVERBBUFFERSIZE	((MAX_SAMPLE_RATE * 200) / 1000)
#define REVERBBUFFERSIZE2	((REVERBBUFFERSIZE*13) / 17)
#define REVERBBUFFERSIZE3	((REVERBBUFFERSIZE*7) / 13)
#define REVERBBUFFERSIZE4	((REVERBBUFFERSIZE*7) / 19)


// DSP Effects: PUBLIC members
UINT CSoundFile::m_nXBassDepth = 6;
UINT CSoundFile::m_nXBassRange = XBASS_DELAY;
UINT CSoundFile::m_nReverbDepth = 1;
UINT CSoundFile::m_nReverbDelay = 100;
UINT CSoundFile::m_nProLogicDepth = 12;
UINT CSoundFile::m_nProLogicDelay = 20;

////////////////////////////////////////////////////////////////////
// DSP Effects internal state

// Bass Expansion: low-pass filter
static LONG nXBassSum = 0;
static LONG nXBassBufferPos = 0;
static LONG nXBassDlyPos = 0;
static LONG nXBassMask = 0;

// Noise Reduction: simple low-pass filter
static LONG nLeftNR = 0;
static LONG nRightNR = 0;

// Surround Encoding: 1 delay line + low-pass filter + high-pass filter
static LONG nSurroundSize = 0;
static LONG nSurroundPos = 0;
static LONG nDolbyDepth = 0;
static LONG nDolbyLoDlyPos = 0;
static LONG nDolbyLoFltPos = 0;
static LONG nDolbyLoFltSum = 0;
static LONG nDolbyHiFltPos = 0;
static LONG nDolbyHiFltSum = 0;

// Reverb: 4 delay lines + high-pass filter + low-pass filter
#ifndef MODPLUG_NO_REVERB
static LONG nReverbSize = 0;
static LONG nReverbBufferPos = 0;
static LONG nReverbSize2 = 0;
static LONG nReverbBufferPos2 = 0;
static LONG nReverbSize3 = 0;
static LONG nReverbBufferPos3 = 0;
static LONG nReverbSize4 = 0;
static LONG nReverbBufferPos4 = 0;
static LONG nReverbLoFltSum = 0;
static LONG nReverbLoFltPos = 0;
static LONG nReverbLoDlyPos = 0;
static LONG nFilterAttn = 0;
static LONG gRvbLowPass[8];
static LONG gRvbLPPos = 0;
static LONG gRvbLPSum = 0;
static LONG ReverbLoFilterBuffer[XBASSBUFFERSIZE];
static LONG ReverbLoFilterDelay[XBASSBUFFERSIZE];
static LONG ReverbBuffer[REVERBBUFFERSIZE];
static LONG ReverbBuffer2[REVERBBUFFERSIZE2];
static LONG ReverbBuffer3[REVERBBUFFERSIZE3];
static LONG ReverbBuffer4[REVERBBUFFERSIZE4];
#endif
static LONG XBassBuffer[XBASSBUFFERSIZE];
static LONG XBassDelay[XBASSBUFFERSIZE];
static LONG DolbyLoFilterBuffer[XBASSBUFFERSIZE];
static LONG DolbyLoFilterDelay[XBASSBUFFERSIZE];
static LONG DolbyHiFilterBuffer[FILTERBUFFERSIZE];
static LONG SurroundBuffer[SURROUNDBUFFERSIZE];

// Access the main temporary mix buffer directly: avoids an extra pointer
extern int MixSoundBuffer[MIXBUFFERSIZE*4];
//cextern int MixReverbBuffer[MIXBUFFERSIZE*2];
extern int MixReverbBuffer[MIXBUFFERSIZE*2];

static UINT GetMaskFromSize(UINT len)
//-----------------------------------
{
	UINT n = 2;
	while (n <= len) n <<= 1;
	return ((n >> 1) - 1);
}


void CSoundFile::InitializeDSP(BOOL bReset)
//-----------------------------------------
{
	if (!m_nReverbDelay) m_nReverbDelay = 100;
	if (!m_nXBassRange) m_nXBassRange = XBASS_DELAY;
	if (!m_nProLogicDelay) m_nProLogicDelay = 20;
	if (m_nXBassDepth > 8) m_nXBassDepth = 8;
	if (m_nXBassDepth < 2) m_nXBassDepth = 2;
	if (bReset)
	{
		// Noise Reduction
		nLeftNR = nRightNR = 0;
	}
	// Pro-Logic Surround
	nSurroundPos = nSurroundSize = 0;
	nDolbyLoFltPos = nDolbyLoFltSum = nDolbyLoDlyPos = 0;
	nDolbyHiFltPos = nDolbyHiFltSum = 0;
	if (gdwSoundSetup & SNDMIX_SURROUND)
	{
		memset(DolbyLoFilterBuffer, 0, sizeof(DolbyLoFilterBuffer));
		memset(DolbyHiFilterBuffer, 0, sizeof(DolbyHiFilterBuffer));
		memset(DolbyLoFilterDelay, 0, sizeof(DolbyLoFilterDelay));
		memset(SurroundBuffer, 0, sizeof(SurroundBuffer));
		nSurroundSize = (gdwMixingFreq * m_nProLogicDelay) / 1000;
		if (nSurroundSize > SURROUNDBUFFERSIZE) nSurroundSize = SURROUNDBUFFERSIZE;
		if (m_nProLogicDepth < 8) nDolbyDepth = (32 >> m_nProLogicDepth) + 32;
		else nDolbyDepth = (m_nProLogicDepth < 16) ? (8 + (m_nProLogicDepth - 8) * 7) : 64;
		nDolbyDepth >>= 2;
	}
	// Reverb Setup
#ifndef MODPLUG_NO_REVERB
	if (gdwSoundSetup & SNDMIX_REVERB)
	{
		UINT nrs = (gdwMixingFreq * m_nReverbDelay) / 1000;
		UINT nfa = m_nReverbDepth+1;
		if (nrs > REVERBBUFFERSIZE) nrs = REVERBBUFFERSIZE;
		if ((bReset) || (nrs != (UINT)nReverbSize) || (nfa != (UINT)nFilterAttn))
		{
			nFilterAttn = nfa;
			nReverbSize = nrs;
			nReverbBufferPos = nReverbBufferPos2 = nReverbBufferPos3 = nReverbBufferPos4 = 0;
			nReverbLoFltSum = nReverbLoFltPos = nReverbLoDlyPos = 0;
			gRvbLPSum = gRvbLPPos = 0;
			nReverbSize2 = (nReverbSize * 13) / 17;
			if (nReverbSize2 > REVERBBUFFERSIZE2) nReverbSize2 = REVERBBUFFERSIZE2;
			nReverbSize3 = (nReverbSize * 7) / 13;
			if (nReverbSize3 > REVERBBUFFERSIZE3) nReverbSize3 = REVERBBUFFERSIZE3;
			nReverbSize4 = (nReverbSize * 7) / 19;
			if (nReverbSize4 > REVERBBUFFERSIZE4) nReverbSize4 = REVERBBUFFERSIZE4;
			memset(ReverbLoFilterBuffer, 0, sizeof(ReverbLoFilterBuffer));
			memset(ReverbLoFilterDelay, 0, sizeof(ReverbLoFilterDelay));
			memset(ReverbBuffer, 0, sizeof(ReverbBuffer));
			memset(ReverbBuffer2, 0, sizeof(ReverbBuffer2));
			memset(ReverbBuffer3, 0, sizeof(ReverbBuffer3));
			memset(ReverbBuffer4, 0, sizeof(ReverbBuffer4));
			memset(gRvbLowPass, 0, sizeof(gRvbLowPass));
		}
	} else nReverbSize = 0;
#endif
	BOOL bResetBass = FALSE;
	// Bass Expansion Reset
	if (gdwSoundSetup & SNDMIX_MEGABASS)
	{
		UINT nXBassSamples = (gdwMixingFreq * m_nXBassRange) / 10000;
		if (nXBassSamples > XBASSBUFFERSIZE) nXBassSamples = XBASSBUFFERSIZE;
		UINT mask = GetMaskFromSize(nXBassSamples);
		if ((bReset) || (mask != (UINT)nXBassMask))
		{
			nXBassMask = mask;
			bResetBass = TRUE;
		}
	} else
	{
		nXBassMask = 0;
		bResetBass = TRUE;
	}
	if (bResetBass)
	{
		nXBassSum = nXBassBufferPos = nXBassDlyPos = 0;
		memset(XBassBuffer, 0, sizeof(XBassBuffer));
		memset(XBassDelay, 0, sizeof(XBassDelay));
	}
}


void CSoundFile::ProcessStereoDSP(int count)
//------------------------------------------
{
#ifndef MODPLUG_NO_REVERB
	// Reverb
	if (gdwSoundSetup & SNDMIX_REVERB)
	{
		int *pr = MixSoundBuffer, *pin = MixReverbBuffer, rvbcount = count;
		do
		{
			int echo = ReverbBuffer[nReverbBufferPos] + ReverbBuffer2[nReverbBufferPos2]
					+ ReverbBuffer3[nReverbBufferPos3] + ReverbBuffer4[nReverbBufferPos4];	// echo = reverb signal
			// Delay line and remove Low Frequencies			// v = original signal
			int echodly = ReverbLoFilterDelay[nReverbLoDlyPos];	// echodly = delayed signal
			ReverbLoFilterDelay[nReverbLoDlyPos] = echo >> 1;
			nReverbLoDlyPos++;
			nReverbLoDlyPos &= 0x1F;
			int n = nReverbLoFltPos;
			nReverbLoFltSum -= ReverbLoFilterBuffer[n];
			int tmp = echo / 128;
			ReverbLoFilterBuffer[n] = tmp;
			nReverbLoFltSum += tmp;
			echodly -= nReverbLoFltSum;
			nReverbLoFltPos = (n + 1) & 0x3F;
			// Reverb
			int v = (pin[0]+pin[1]) >> nFilterAttn;
			pr[0] += pin[0] + echodly;
			pr[1] += pin[1] + echodly;
			v += echodly >> 2;
			ReverbBuffer3[nReverbBufferPos3] = v;
			ReverbBuffer4[nReverbBufferPos4] = v;
			v += echodly >> 4;
			v >>= 1;
			gRvbLPSum -= gRvbLowPass[gRvbLPPos];
			gRvbLPSum += v;
			gRvbLowPass[gRvbLPPos] = v;
			gRvbLPPos++;
			gRvbLPPos &= 7;
			int vlp = gRvbLPSum >> 2;
			ReverbBuffer[nReverbBufferPos] = vlp;
			ReverbBuffer2[nReverbBufferPos2] = vlp;
			if (++nReverbBufferPos >= nReverbSize) nReverbBufferPos = 0;
			if (++nReverbBufferPos2 >= nReverbSize2) nReverbBufferPos2 = 0;
			if (++nReverbBufferPos3 >= nReverbSize3) nReverbBufferPos3 = 0;
			if (++nReverbBufferPos4 >= nReverbSize4) nReverbBufferPos4 = 0;
			pr += 2;
			pin += 2;
		} while (--rvbcount);
	}
#endif
	// Dolby Pro-Logic Surround
	if (gdwSoundSetup & SNDMIX_SURROUND)
	{
		int *pr = MixSoundBuffer, n = nDolbyLoFltPos;
		for (int r=count; r; r--)
		{
			int v = (pr[0]+pr[1]+DOLBYATTNROUNDUP) >> (nDolbyHiFltAttn+1);
#ifndef MODPLUG_FASTSOUNDLIB
			v *= (int)nDolbyDepth;
#endif
			// Low-Pass Filter
			nDolbyHiFltSum -= DolbyHiFilterBuffer[nDolbyHiFltPos];
			DolbyHiFilterBuffer[nDolbyHiFltPos] = v;
			nDolbyHiFltSum += v;
			v = nDolbyHiFltSum;
			nDolbyHiFltPos++;
			nDolbyHiFltPos &= nDolbyHiFltMask;
			// Surround
			int secho = SurroundBuffer[nSurroundPos];
			SurroundBuffer[nSurroundPos] = v;
			// Delay line and remove low frequencies
			v = DolbyLoFilterDelay[nDolbyLoDlyPos];		// v = delayed signal
			DolbyLoFilterDelay[nDolbyLoDlyPos] = secho;	// secho = signal
			nDolbyLoDlyPos++;
			nDolbyLoDlyPos &= 0x1F;
			nDolbyLoFltSum -= DolbyLoFilterBuffer[n];
			int tmp = secho / 64;
			DolbyLoFilterBuffer[n] = tmp;
			nDolbyLoFltSum += tmp;
			v -= nDolbyLoFltSum;
			n++;
			n &= 0x3F;
			// Add echo
			pr[0] += v;
			pr[1] -= v;
			if (++nSurroundPos >= nSurroundSize) nSurroundPos = 0;
			pr += 2;
		}
		nDolbyLoFltPos = n;
	}
	// Bass Expansion
	if (gdwSoundSetup & SNDMIX_MEGABASS)
	{
		int *px = MixSoundBuffer;
		int xba = m_nXBassDepth+1, xbamask = (1 << xba) - 1;
		int n = nXBassBufferPos;
		for (int x=count; x; x--)
		{
			nXBassSum -= XBassBuffer[n];
			int tmp0 = px[0] + px[1];
			int tmp = (tmp0 + ((tmp0 >> 31) & xbamask)) >> xba;
			XBassBuffer[n] = tmp;
			nXBassSum += tmp;
			int v = XBassDelay[nXBassDlyPos];
			XBassDelay[nXBassDlyPos] = px[0];
			px[0] = v + nXBassSum;
			v = XBassDelay[nXBassDlyPos+1];
			XBassDelay[nXBassDlyPos+1] = px[1];
			px[1] = v + nXBassSum;
			nXBassDlyPos = (nXBassDlyPos + 2) & nXBassMask;
			px += 2;
			n++;
			n &= nXBassMask;
		}
		nXBassBufferPos = n;
	}
	// Noise Reduction
	if (gdwSoundSetup & SNDMIX_NOISEREDUCTION)
	{
		int n1 = nLeftNR, n2 = nRightNR;
		int *pnr = MixSoundBuffer;
		for (int nr=count; nr; nr--)
		{
			int vnr = pnr[0] >> 1;
			pnr[0] = vnr + n1;
			n1 = vnr;
			vnr = pnr[1] >> 1;
			pnr[1] = vnr + n2;
			n2 = vnr;
			pnr += 2;
		}
		nLeftNR = n1;
		nRightNR = n2;
	}
}


void CSoundFile::ProcessMonoDSP(int count)
//----------------------------------------
{
#ifndef MODPLUG_NO_REVERB
	// Reverb
	if (gdwSoundSetup & SNDMIX_REVERB)
	{
		int *pr = MixSoundBuffer, rvbcount = count, *pin = MixReverbBuffer;
		do
		{
			int echo = ReverbBuffer[nReverbBufferPos] + ReverbBuffer2[nReverbBufferPos2]
					+ ReverbBuffer3[nReverbBufferPos3] + ReverbBuffer4[nReverbBufferPos4];	// echo = reverb signal
			// Delay line and remove Low Frequencies			// v = original signal
			int echodly = ReverbLoFilterDelay[nReverbLoDlyPos];	// echodly = delayed signal
			ReverbLoFilterDelay[nReverbLoDlyPos] = echo >> 1;
			nReverbLoDlyPos++;
			nReverbLoDlyPos &= 0x1F;
			int n = nReverbLoFltPos;
			nReverbLoFltSum -= ReverbLoFilterBuffer[n];
			int tmp = echo / 128;
			ReverbLoFilterBuffer[n] = tmp;
			nReverbLoFltSum += tmp;
			echodly -= nReverbLoFltSum;
			nReverbLoFltPos = (n + 1) & 0x3F;
			// Reverb
			int v = pin[0] >> (nFilterAttn-1);
			*pr++ += pin[0] + echodly;
			pin++;
			v += echodly >> 2;
			ReverbBuffer3[nReverbBufferPos3] = v;
			ReverbBuffer4[nReverbBufferPos4] = v;
			v += echodly >> 4;
			v >>= 1;
			gRvbLPSum -= gRvbLowPass[gRvbLPPos];
			gRvbLPSum += v;
			gRvbLowPass[gRvbLPPos] = v;
			gRvbLPPos++;
			gRvbLPPos &= 7;
			int vlp = gRvbLPSum >> 2;
			ReverbBuffer[nReverbBufferPos] = vlp;
			ReverbBuffer2[nReverbBufferPos2] = vlp;
			if (++nReverbBufferPos >= nReverbSize) nReverbBufferPos = 0;
			if (++nReverbBufferPos2 >= nReverbSize2) nReverbBufferPos2 = 0;
			if (++nReverbBufferPos3 >= nReverbSize3) nReverbBufferPos3 = 0;
			if (++nReverbBufferPos4 >= nReverbSize4) nReverbBufferPos4 = 0;
		} while (--rvbcount);
	}
#endif
	// Bass Expansion
	if (gdwSoundSetup & SNDMIX_MEGABASS)
	{
		int *px = MixSoundBuffer;
		int xba = m_nXBassDepth, xbamask = (1 << xba)-1;
		int n = nXBassBufferPos;
		for (int x=count; x; x--)
		{
			nXBassSum -= XBassBuffer[n];
			int tmp0 = *px;
			int tmp = (tmp0 + ((tmp0 >> 31) & xbamask)) >> xba;
			XBassBuffer[n] = tmp;
			nXBassSum += tmp;
			int v = XBassDelay[nXBassDlyPos];
			XBassDelay[nXBassDlyPos] = *px;
			*px++ = v + nXBassSum;
			nXBassDlyPos = (nXBassDlyPos + 2) & nXBassMask;
			n++;
			n &= nXBassMask;
		}
		nXBassBufferPos = n;
	}
	// Noise Reduction
	if (gdwSoundSetup & SNDMIX_NOISEREDUCTION)
	{
		int n = nLeftNR;
		int *pnr = MixSoundBuffer;
		for (int nr=count; nr; pnr++, nr--)
		{
			int vnr = *pnr >> 1;
			*pnr = vnr + n;
			n = vnr;
		}
		nLeftNR = n;
	}
}


/////////////////////////////////////////////////////////////////
// Clean DSP Effects interface

// [Reverb level 0(quiet)-100(loud)], [delay in ms, usually 40-200ms]
BOOL CSoundFile::SetReverbParameters(UINT nDepth, UINT nDelay)
//------------------------------------------------------------
{
	if (nDepth > 100) nDepth = 100;
	UINT gain = nDepth / 20;
	if (gain > 4) gain = 4;
	m_nReverbDepth = 4 - gain;
	if (nDelay < 40) nDelay = 40;
	if (nDelay > 250) nDelay = 250;
	m_nReverbDelay = nDelay;
	return TRUE;
}


// [XBass level 0(quiet)-100(loud)], [cutoff in Hz 20-100]
BOOL CSoundFile::SetXBassParameters(UINT nDepth, UINT nRange)
//-----------------------------------------------------------
{
	if (nDepth > 100) nDepth = 100;
	UINT gain = nDepth / 20;
	if (gain > 4) gain = 4;
	m_nXBassDepth = 8 - gain;	// filter attenuation 1/256 .. 1/16
	UINT range = nRange / 5;
	if (range > 5) range -= 5; else range = 0;
	if (nRange > 16) nRange = 16;
	m_nXBassRange = 21 - range;	// filter average on 0.5-1.6ms
	return TRUE;
}


// [Surround level 0(quiet)-100(heavy)] [delay in ms, usually 5-50ms]
BOOL CSoundFile::SetSurroundParameters(UINT nDepth, UINT nDelay)
//--------------------------------------------------------------
{
	UINT gain = (nDepth * 16) / 100;
	if (gain > 16) gain = 16;
	if (gain < 1) gain = 1;
	m_nProLogicDepth = gain;
	if (nDelay < 4) nDelay = 4;
	if (nDelay > 50) nDelay = 50;
	m_nProLogicDelay = nDelay;
	return TRUE;
}

BOOL CSoundFile::SetWaveConfigEx(BOOL bSurround,BOOL bNoOverSampling,BOOL bReverb,BOOL hqido,BOOL bMegaBass,BOOL bNR,BOOL bEQ)
//----------------------------------------------------------------------------------------------------------------------------
{
	DWORD d = gdwSoundSetup & ~(SNDMIX_SURROUND | SNDMIX_NORESAMPLING | SNDMIX_REVERB | SNDMIX_HQRESAMPLER | SNDMIX_MEGABASS | SNDMIX_NOISEREDUCTION | SNDMIX_EQ);
	if (bSurround) d |= SNDMIX_SURROUND;
	if (bNoOverSampling) d |= SNDMIX_NORESAMPLING;
	if (bReverb) d |= SNDMIX_REVERB;
	if (hqido) d |= SNDMIX_HQRESAMPLER;
	if (bMegaBass) d |= SNDMIX_MEGABASS;
	if (bNR) d |= SNDMIX_NOISEREDUCTION;
	if (bEQ) d |= SNDMIX_EQ;
	gdwSoundSetup = d;
	InitPlayer(FALSE);
	return TRUE;
}
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

//#include "stdafx.h"
//#include "sndfile.h"

// AWE32: cutoff = reg[0-255] * 31.25 + 100 -> [100Hz-8060Hz]
// EMU10K1 docs: cutoff = reg[0-127]*62+100
#define FILTER_PRECISION	8192

#ifndef NO_FILTER

#if defined(_MSC_VER) && defined(_M_IX86)
#define _ASM_MATH
#endif

#ifdef _ASM_MATH
// pow(a,b) returns a^^b -> 2^^(b.log2(a))
static float pow(float a, float b)
{
	long tmpint;
	float result;
	_asm {
	fld b				// Load b
	fld a				// Load a
	fyl2x				// ST(0) = b.log2(a)
	fist tmpint			// Store integer exponent
	fisub tmpint		// ST(0) = -1 <= (b*log2(a)) <= 1
	f2xm1				// ST(0) = 2^(x)-1
	fild tmpint			// load integer exponent
	fld1				// Load 1
	fscale				// ST(0) = 2^ST(1)
	fstp ST(1)			// Remove the integer from the stack
	fmul ST(1), ST(0)	// multiply with fractional part
	faddp ST(1), ST(0)	// add integer_part
	fstp result			// Store the result
	}
	return result;
}

#else

#include <math.h>

#endif // _ASM_MATH


DWORD CSoundFile::CutOffToFrequency(UINT nCutOff, int flt_modifier) const
//-----------------------------------------------------------------------
{
	float Fc;

	if (m_dwSongFlags & SONG_EXFILTERRANGE)
		Fc = 110.0f * pow(2.0f, 0.25f + ((float)(nCutOff*(flt_modifier+256)))/(21.0f*512.0f));
	else
		Fc = 110.0f * pow(2.0f, 0.25f + ((float)(nCutOff*(flt_modifier+256)))/(24.0f*512.0f));
	LONG freq = (LONG)Fc;
	if (freq < 120) return 120;
	if (freq > 10000) return 10000;
	if (freq*2 > (LONG)gdwMixingFreq) freq = gdwMixingFreq>>1;
	return (DWORD)freq;
}


// Simple 2-poles resonant filter
void CSoundFile::SetupChannelFilter(MODCHANNEL *pChn, BOOL bReset, int flt_modifier) const
//----------------------------------------------------------------------------------------
{
	float fc = (float)CutOffToFrequency(pChn->nCutOff, flt_modifier);
	float fs = (float)gdwMixingFreq;
	float fg, fb0, fb1;

	fc *= (float)(2.0*3.14159265358/fs);
	float dmpfac = pow(10.0f, -((24.0f / 128.0f)*(float)pChn->nResonance) / 20.0f);
	float d = (1.0f-2.0f*dmpfac)* fc;
	if (d>2.0) d = 2.0;
	d = (2.0f*dmpfac - d)/fc;
	float e = pow(1.0f/fc,2.0f);

	fg=1/(1+d+e);
	fb0=(d+e+e)/(1+d+e);
	fb1=-e/(1+d+e);

	pChn->nFilter_A0 = (int)(fg * FILTER_PRECISION);
	pChn->nFilter_B0 = (int)(fb0 * FILTER_PRECISION);
	pChn->nFilter_B1 = (int)(fb1 * FILTER_PRECISION);

	if (bReset)
	{
		pChn->nFilter_Y1 = pChn->nFilter_Y2 = 0;
		pChn->nFilter_Y3 = pChn->nFilter_Y4 = 0;
	}
	pChn->dwFlags |= CHN_FILTER;
}

#endif // NO_FILTER
/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

//#include "stdafx.h"
#include <stdlib.h>
//#include "sndfile.h"
//#include "tables.h"

#ifdef _MSC_VER
#pragma warning(disable:4244)
#endif

////////////////////////////////////////////////////////////
// Length

DWORD CSoundFile::GetLength(BOOL bAdjust, BOOL bTotal)
//----------------------------------------------------
{
	UINT dwElapsedTime=0, nRow=0, nCurrentPattern=0, nNextPattern=0, nPattern=0;
	UINT nMusicSpeed=m_nDefaultSpeed, nMusicTempo=m_nDefaultTempo, nNextRow=0;
	UINT nMaxRow = 0, nMaxPattern = 0, nNextStartRow = 0;
	LONG nGlbVol = m_nDefaultGlobalVolume, nOldGlbVolSlide = 0;
	BYTE instr[MAX_CHANNELS];
	BYTE notes[MAX_CHANNELS];
	BYTE vols[MAX_CHANNELS];
	BYTE oldparam[MAX_CHANNELS];
	BYTE chnvols[MAX_CHANNELS];
	DWORD patloop[MAX_CHANNELS];

	memset(instr, 0, sizeof(instr));
	memset(notes, 0, sizeof(notes));
	memset(vols, 0xFF, sizeof(vols));
	memset(patloop, 0, sizeof(patloop));
	memset(oldparam, 0, sizeof(oldparam));
	memset(chnvols, 64, sizeof(chnvols));
	for (UINT icv=0; icv<m_nChannels; icv++)
		chnvols[icv] = ChnSettings[icv].nVolume;
	nMaxRow = m_nNextRow;
	nMaxPattern = m_nNextPattern;
	for (;;)
	{
		UINT nSpeedCount = 0;
		nRow = nNextRow;
		nCurrentPattern = nNextPattern;
		// Check if pattern is valid
		nPattern = (nCurrentPattern < MAX_ORDERS) ? Order[nCurrentPattern] : 0xFF;
		while (nPattern >= MAX_PATTERNS)
		{
			// End of song ?
			if ((nPattern == 0xFF) || (nCurrentPattern >= MAX_ORDERS))
			{
				goto EndMod;
			} else
			{
				nCurrentPattern++;
				nPattern = (nCurrentPattern < MAX_ORDERS) ? Order[nCurrentPattern] : 0xFF;
			}
			nNextPattern = nCurrentPattern;
		}
		// Weird stuff?
		if ((nPattern >= MAX_PATTERNS) || (!Patterns[nPattern]) ||
			PatternSize[nPattern] == 0) break;
		// Should never happen
		if (nRow >= PatternSize[nPattern]) nRow = 0;
		// Update next position
		nNextRow = nRow + 1;
		if (nNextRow >= PatternSize[nPattern])
		{
			nNextPattern = nCurrentPattern + 1;
			nNextRow = nNextStartRow;
			nNextStartRow = 0;
		}
		if (!nRow)
		{
			for (UINT ipck=0; ipck<m_nChannels; ipck++) patloop[ipck] = dwElapsedTime;
		}
		if (!bTotal)
		{
			if ((nCurrentPattern > nMaxPattern) || ((nCurrentPattern == nMaxPattern) && (nRow >= nMaxRow)))
			{
				if (bAdjust)
				{
					m_nMusicSpeed = nMusicSpeed;
					m_nMusicTempo = nMusicTempo;
				}
				break;
			}
		}
		MODCHANNEL *pChn = Chn;
		MODCOMMAND *p = Patterns[nPattern] + nRow * m_nChannels;
		for (UINT nChn=0; nChn<m_nChannels; p++,pChn++, nChn++) if (*((DWORD *)p))
		{
			UINT command = p->command;
			UINT param = p->param;
			UINT note = p->note;
			if (p->instr) { instr[nChn] = p->instr; notes[nChn] = 0; vols[nChn] = 0xFF; }
			if ((note) && (note <= NOTE_MAX)) notes[nChn] = note;
			if (p->volcmd == VOLCMD_VOLUME)	{ vols[nChn] = p->vol; }
			if (command) switch (command)
			{
			// Position Jump
			case CMD_POSITIONJUMP:
				if (param <= nCurrentPattern) goto EndMod;
				nNextPattern = param;
				nNextRow = 0;
				nNextStartRow = 0;
				if (bAdjust)
				{
					pChn->nPatternLoopCount = 0;
					pChn->nPatternLoop = 0;
				}
				break;
			// Pattern Break
			case CMD_PATTERNBREAK:
				nNextRow = param;
				nNextPattern = nCurrentPattern + 1;
				nNextStartRow = 0;
				if (bAdjust)
				{
					pChn->nPatternLoopCount = 0;
					pChn->nPatternLoop = 0;
				}
				break;
			// Set Speed
			case CMD_SPEED:
				if (!param) break;
				if ((param <= 0x20) || (m_nType != MOD_TYPE_MOD))
				{
					if (param < 128) nMusicSpeed = param;
				}
				break;
			// Set Tempo
			case CMD_TEMPO:
				if ((bAdjust) && (m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT)))
				{
					if (param) pChn->nOldTempo = param; else param = pChn->nOldTempo;
				}
				if (param >= 0x20) nMusicTempo = param; else
				// Tempo Slide
				if ((param & 0xF0) == 0x10)
				{
					nMusicTempo += param & 0x0F;
					if (nMusicTempo > 255) nMusicTempo = 255;
				} else
				{
					nMusicTempo -= param & 0x0F;
					if (nMusicTempo < 32) nMusicTempo = 32;
				}
				break;
			// Pattern Delay
			case CMD_S3MCMDEX:
				if ((param & 0xF0) == 0x60) { nSpeedCount = param & 0x0F; break; } else
				if ((param & 0xF0) == 0xB0) { param &= 0x0F; param |= 0x60; }
			case CMD_MODCMDEX:
				if ((param & 0xF0) == 0xE0) nSpeedCount = (param & 0x0F) * nMusicSpeed; else
				if ((param & 0xF0) == 0x60)
				{
					if (param & 0x0F) dwElapsedTime += (dwElapsedTime - patloop[nChn]) * (param & 0x0F);
					else {
						patloop[nChn] = dwElapsedTime;
						if (m_nType & MOD_TYPE_XM) nNextStartRow = nRow;
					}
				}
				break;
			}
			if (!bAdjust) continue;
			switch(command)
			{
			// Portamento Up/Down
			case CMD_PORTAMENTOUP:
			case CMD_PORTAMENTODOWN:
				if (param) pChn->nOldPortaUpDown = param;
				break;
			// Tone-Portamento
			case CMD_TONEPORTAMENTO:
				if (param) pChn->nPortamentoSlide = param << 2;
				break;
			// Offset
			case CMD_OFFSET:
				if (param) pChn->nOldOffset = param;
				break;
			// Volume Slide
			case CMD_VOLUMESLIDE:
			case CMD_TONEPORTAVOL:
			case CMD_VIBRATOVOL:
				if (param) pChn->nOldVolumeSlide = param;
				break;
			// Set Volume
			case CMD_VOLUME:
				vols[nChn] = param;
				break;
			// Global Volume
			case CMD_GLOBALVOLUME:
				if (!(m_nType & (MOD_TYPE_IT))) param <<= 1;
				if (param > 128) param = 128;
				nGlbVol = param << 1;
				break;
			// Global Volume Slide
			case CMD_GLOBALVOLSLIDE:
				if (param) nOldGlbVolSlide = param; else param = nOldGlbVolSlide;
				if (((param & 0x0F) == 0x0F) && (param & 0xF0))
				{
					param >>= 4;
					if (m_nType != MOD_TYPE_IT) param <<= 1;
					nGlbVol += param << 1;
				} else
				if (((param & 0xF0) == 0xF0) && (param & 0x0F))
				{
					param = (param & 0x0F) << 1;
					if (m_nType != MOD_TYPE_IT) param <<= 1;
					nGlbVol -= param;
				} else
				if (param & 0xF0)
				{
					param >>= 4;
					param <<= 1;
					if (m_nType != MOD_TYPE_IT) param <<= 1;
					nGlbVol += param * nMusicSpeed;
				} else
				{
					param = (param & 0x0F) << 1;
					if (m_nType != MOD_TYPE_IT) param <<= 1;
					nGlbVol -= param * nMusicSpeed;
				}
				if (nGlbVol < 0) nGlbVol = 0;
				if (nGlbVol > 256) nGlbVol = 256;
				break;
			case CMD_CHANNELVOLUME:
				if (param <= 64) chnvols[nChn] = param;
				break;
			case CMD_CHANNELVOLSLIDE:
				if (param) oldparam[nChn] = param; else param = oldparam[nChn];
				pChn->nOldChnVolSlide = param;
				if (((param & 0x0F) == 0x0F) && (param & 0xF0))
				{
					param = (param >> 4) + chnvols[nChn];
				} else
				if (((param & 0xF0) == 0xF0) && (param & 0x0F))
				{
					if (chnvols[nChn] > (int)(param & 0x0F)) param = chnvols[nChn] - (param & 0x0F);
					else param = 0;
				} else
				if (param & 0x0F)
				{
					param = (param & 0x0F) * nMusicSpeed;
					param = (chnvols[nChn] > param) ? chnvols[nChn] - param : 0;
				} else param = ((param & 0xF0) >> 4) * nMusicSpeed + chnvols[nChn];
				if (param > 64) param = 64;
				chnvols[nChn] = param;
				break;
			}
		}
		nSpeedCount += nMusicSpeed;
		dwElapsedTime += (2500 * nSpeedCount) / nMusicTempo;
	}
EndMod:
	if ((bAdjust) && (!bTotal))
	{
		m_nGlobalVolume = nGlbVol;
		m_nOldGlbVolSlide = nOldGlbVolSlide;
		for (UINT n=0; n<m_nChannels; n++)
		{
			Chn[n].nGlobalVol = chnvols[n];
			if (notes[n]) Chn[n].nNewNote = notes[n];
			if (instr[n]) Chn[n].nNewIns = instr[n];
			if (vols[n] != 0xFF)
			{
				if (vols[n] > 64) vols[n] = 64;
				Chn[n].nVolume = vols[n] << 2;
			}
		}
	}
	return (dwElapsedTime+500) / 1000;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// Effects

void CSoundFile::InstrumentChange(MODCHANNEL *pChn, UINT instr, BOOL bPorta, BOOL bUpdVol, BOOL bResetEnv)
//--------------------------------------------------------------------------------------------------------
{
	BOOL bInstrumentChanged = FALSE;

	if (instr >= MAX_INSTRUMENTS) return;
	INSTRUMENTHEADER *penv = Headers[instr];
	MODINSTRUMENT *psmp = &Ins[instr];
	UINT note = pChn->nNewNote;
	if ((penv) && (note) && (note <= 128))
	{
		if (penv->NoteMap[note-1] >= 0xFE) return;
		UINT n = penv->Keyboard[note-1];
		psmp = ((n) && (n < MAX_SAMPLES)) ? &Ins[n] : NULL;
	} else
	if (m_nInstruments)
	{
		if (note >= 0xFE) return;
		psmp = NULL;
	}
	// Update Volume
	if (bUpdVol) pChn->nVolume = (psmp) ? psmp->nVolume : 0;
	// bInstrumentChanged is used for IT carry-on env option
	if (penv != pChn->pHeader)
	{
		bInstrumentChanged = TRUE;
		pChn->pHeader = penv;
	} else
	{
		// Special XM hack
		if ((bPorta) && (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2)) && (penv)
		&& (pChn->pInstrument) && (psmp != pChn->pInstrument))
		{
			// FT2 doesn't change the sample in this case,
			// but still uses the sample info from the old one (bug?)
			return;
		}
	}
	// Instrument adjust
	pChn->nNewIns = 0;
	if (psmp)
	{
		if (penv)
		{
			pChn->nInsVol = (psmp->nGlobalVol * penv->nGlobalVol) >> 6;
			if (penv->dwFlags & ENV_SETPANNING) pChn->nPan = penv->nPan;
			pChn->nNNA = penv->nNNA;
		} else
		{
			pChn->nInsVol = psmp->nGlobalVol;
		}
		if (psmp->uFlags & CHN_PANNING) pChn->nPan = psmp->nPan;
	}
	// Reset envelopes
	if (bResetEnv)
	{
		if ((!bPorta) || (!(m_nType & MOD_TYPE_IT)) || (m_dwSongFlags & SONG_ITCOMPATMODE)
		 || (!pChn->nLength) || ((pChn->dwFlags & CHN_NOTEFADE) && (!pChn->nFadeOutVol)))
		{
			pChn->dwFlags |= CHN_FASTVOLRAMP;
			if ((m_nType & MOD_TYPE_IT) && (!bInstrumentChanged) && (penv) && (!(pChn->dwFlags & (CHN_KEYOFF|CHN_NOTEFADE))))
			{
				if (!(penv->dwFlags & ENV_VOLCARRY)) pChn->nVolEnvPosition = 0;
				if (!(penv->dwFlags & ENV_PANCARRY)) pChn->nPanEnvPosition = 0;
				if (!(penv->dwFlags & ENV_PITCHCARRY)) pChn->nPitchEnvPosition = 0;
			} else
			{
				pChn->nVolEnvPosition = 0;
				pChn->nPanEnvPosition = 0;
				pChn->nPitchEnvPosition = 0;
			}
			pChn->nAutoVibDepth = 0;
			pChn->nAutoVibPos = 0;
		} else
		if ((penv) && (!(penv->dwFlags & ENV_VOLUME)))
		{
			pChn->nVolEnvPosition = 0;
			pChn->nAutoVibDepth = 0;
			pChn->nAutoVibPos = 0;
		}
	}
	// Invalid sample ?
	if (!psmp)
	{
		pChn->pInstrument = NULL;
		pChn->nInsVol = 0;
		return;
	}
	// Tone-Portamento doesn't reset the pingpong direction flag
	if ((bPorta) && (psmp == pChn->pInstrument))
	{
		if (m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT)) return;
		pChn->dwFlags &= ~(CHN_KEYOFF|CHN_NOTEFADE);
		pChn->dwFlags = (pChn->dwFlags & (0xFFFFFF00 | CHN_PINGPONGFLAG)) | (psmp->uFlags);
	} else
	{
		pChn->dwFlags &= ~(CHN_KEYOFF|CHN_NOTEFADE|CHN_VOLENV|CHN_PANENV|CHN_PITCHENV);
		pChn->dwFlags = (pChn->dwFlags & 0xFFFFFF00) | (psmp->uFlags);
		if (penv)
		{
			if (penv->dwFlags & ENV_VOLUME) pChn->dwFlags |= CHN_VOLENV;
			if (penv->dwFlags & ENV_PANNING) pChn->dwFlags |= CHN_PANENV;
			if (penv->dwFlags & ENV_PITCH) pChn->dwFlags |= CHN_PITCHENV;
			if ((penv->dwFlags & ENV_PITCH) && (penv->dwFlags & ENV_FILTER))
			{
				if (!pChn->nCutOff) pChn->nCutOff = 0x7F;
			}
			if (penv->nIFC & 0x80) pChn->nCutOff = penv->nIFC & 0x7F;
			if (penv->nIFR & 0x80) pChn->nResonance = penv->nIFR & 0x7F;
		}
		pChn->nVolSwing = pChn->nPanSwing = 0;
	}
	pChn->pInstrument = psmp;
	pChn->nLength = psmp->nLength;
	pChn->nLoopStart = psmp->nLoopStart;
	pChn->nLoopEnd = psmp->nLoopEnd;
	pChn->nC4Speed = psmp->nC4Speed;
	pChn->pSample = psmp->pSample;
	pChn->nTranspose = psmp->RelativeTone;
	pChn->nFineTune = psmp->nFineTune;
	if (pChn->dwFlags & CHN_SUSTAINLOOP)
	{
		pChn->nLoopStart = psmp->nSustainStart;
		pChn->nLoopEnd = psmp->nSustainEnd;
		pChn->dwFlags |= CHN_LOOP;
		if (pChn->dwFlags & CHN_PINGPONGSUSTAIN) pChn->dwFlags |= CHN_PINGPONGLOOP;
	}
	if ((pChn->dwFlags & CHN_LOOP) && (pChn->nLoopEnd < pChn->nLength)) pChn->nLength = pChn->nLoopEnd;
}


void CSoundFile::NoteChange(UINT nChn, int note, BOOL bPorta, BOOL bResetEnv)
//---------------------------------------------------------------------------
{
	if (note < 1) return;
	MODCHANNEL * const pChn = &Chn[nChn];
	MODINSTRUMENT *pins = pChn->pInstrument;
	INSTRUMENTHEADER *penv = pChn->pHeader;
	if ((penv) && (note <= 0x80))
	{
		UINT n = penv->Keyboard[note - 1];
		if ((n) && (n < MAX_SAMPLES)) pins = &Ins[n];
		note = penv->NoteMap[note-1];
	}
	// Key Off
	if (note >= 0x80)	// 0xFE or invalid note => key off
	{
		// Key Off
		if (note < 0xFD && m_nType == MOD_TYPE_IT)
		{
			if (m_nInstruments)
				pChn->dwFlags |= CHN_NOTEFADE;
		} else
		{
			KeyOff(nChn);
		}
		// Note Cut
		if (note == 0xFE)
		{
			pChn->dwFlags |= (CHN_NOTEFADE|CHN_FASTVOLRAMP);
			if ((!(m_nType & MOD_TYPE_IT)) || (m_nInstruments)) pChn->nVolume = 0;
			pChn->nFadeOutVol = 0;
		}
		return;
	}
	if (!pins) return;
	if ((!bPorta) && (m_nType & (MOD_TYPE_XM|MOD_TYPE_MED|MOD_TYPE_MT2)))
	{
		pChn->nTranspose = pins->RelativeTone;
		pChn->nFineTune = pins->nFineTune;
	}
	if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2|MOD_TYPE_MED)) note += pChn->nTranspose;
	if (note < 1) note = 1;
	if (note > 132) note = 132;
	pChn->nNote = note;
	if ((!bPorta) || (m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT))) pChn->nNewIns = 0;
	UINT period = GetPeriodFromNote(note, pChn->nFineTune, pChn->nC4Speed);
	if (period)
	{
		if ((!bPorta) || (!pChn->nPeriod)) pChn->nPeriod = period;
		pChn->nPortamentoDest = period;
		if ((!bPorta) || ((!pChn->nLength) && (!(m_nType & MOD_TYPE_S3M))))
		{
			pChn->pInstrument = pins;
			pChn->pSample = pins->pSample;
			pChn->nLength = pins->nLength;
			pChn->nLoopEnd = pins->nLength;
			pChn->nLoopStart = 0;
			pChn->dwFlags = (pChn->dwFlags & 0xFFFFFF00) | (pins->uFlags);
			if (pChn->dwFlags & CHN_SUSTAINLOOP)
			{
				pChn->nLoopStart = pins->nSustainStart;
				pChn->nLoopEnd = pins->nSustainEnd;
				pChn->dwFlags &= ~CHN_PINGPONGLOOP;
				pChn->dwFlags |= CHN_LOOP;
				if (pChn->dwFlags & CHN_PINGPONGSUSTAIN) pChn->dwFlags |= CHN_PINGPONGLOOP;
				if (pChn->nLength > pChn->nLoopEnd) pChn->nLength = pChn->nLoopEnd;
			} else
			if (pChn->dwFlags & CHN_LOOP)
			{
				pChn->nLoopStart = pins->nLoopStart;
				pChn->nLoopEnd = pins->nLoopEnd;
				if (pChn->nLength > pChn->nLoopEnd) pChn->nLength = pChn->nLoopEnd;
			}
			pChn->nPos = 0;
			pChn->nPosLo = 0;
			if (pChn->nVibratoType < 4) pChn->nVibratoPos = ((m_nType & MOD_TYPE_IT) && (!(m_dwSongFlags & SONG_ITOLDEFFECTS))) ? 0x10 : 0;
			if (pChn->nTremoloType < 4) pChn->nTremoloPos = 0;
		}
		if (pChn->nPos >= pChn->nLength) pChn->nPos = pChn->nLoopStart;
	} else bPorta = FALSE;
	if ((!bPorta) || (!(m_nType & MOD_TYPE_IT))
	 || ((pChn->dwFlags & CHN_NOTEFADE) && (!pChn->nFadeOutVol))
	 || ((m_dwSongFlags & SONG_ITCOMPATMODE) && (pChn->nRowInstr)))
	{
		if ((m_nType & MOD_TYPE_IT) && (pChn->dwFlags & CHN_NOTEFADE) && (!pChn->nFadeOutVol))
		{
			pChn->nVolEnvPosition = 0;
			pChn->nPanEnvPosition = 0;
			pChn->nPitchEnvPosition = 0;
			pChn->nAutoVibDepth = 0;
			pChn->nAutoVibPos = 0;
			pChn->dwFlags &= ~CHN_NOTEFADE;
			pChn->nFadeOutVol = 65536;
		}
		if ((!bPorta) || (!(m_dwSongFlags & SONG_ITCOMPATMODE)) || (pChn->nRowInstr))
		{
			if ((!(m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))) || (pChn->nRowInstr))
			{
				pChn->dwFlags &= ~CHN_NOTEFADE;
				pChn->nFadeOutVol = 65536;
			}
		}
	}
	pChn->dwFlags &= ~(CHN_EXTRALOUD|CHN_KEYOFF);
	// Enable Ramping
	if (!bPorta)
	{
		pChn->nVUMeter = 0x100;
		pChn->nLeftVU = pChn->nRightVU = 0xFF;
		pChn->dwFlags &= ~CHN_FILTER;
		pChn->dwFlags |= CHN_FASTVOLRAMP;
		pChn->nRetrigCount = 0;
		pChn->nTremorCount = 0;
		if (bResetEnv)
		{
			pChn->nVolSwing = pChn->nPanSwing = 0;
			if (penv)
			{
				if (!(penv->dwFlags & ENV_VOLCARRY)) pChn->nVolEnvPosition = 0;
				if (!(penv->dwFlags & ENV_PANCARRY)) pChn->nPanEnvPosition = 0;
				if (!(penv->dwFlags & ENV_PITCHCARRY)) pChn->nPitchEnvPosition = 0;
				if (m_nType & MOD_TYPE_IT)
				{
					// Volume Swing
					if (penv->nVolSwing)
					{
						int d = ((LONG)penv->nVolSwing*(LONG)((rand() & 0xFF) - 0x7F)) / 128;
						pChn->nVolSwing = (signed short)((d * pChn->nVolume + 1)/128);
					}
					// Pan Swing
					if (penv->nPanSwing)
					{
						int d = ((LONG)penv->nPanSwing*(LONG)((rand() & 0xFF) - 0x7F)) / 128;
						pChn->nPanSwing = (signed short)d;
					}
				}
			}
			pChn->nAutoVibDepth = 0;
			pChn->nAutoVibPos = 0;
		}
		pChn->nLeftVol = pChn->nRightVol = 0;
		BOOL bFlt = (m_dwSongFlags & SONG_MPTFILTERMODE) ? FALSE : TRUE;
		// Setup Initial Filter for this note
		if (penv)
		{
			if (penv->nIFR & 0x80) { pChn->nResonance = penv->nIFR & 0x7F; bFlt = TRUE; }
			if (penv->nIFC & 0x80) { pChn->nCutOff = penv->nIFC & 0x7F; bFlt = TRUE; }
		} else
		{
			pChn->nVolSwing = pChn->nPanSwing = 0;
		}
#ifndef NO_FILTER
		if ((pChn->nCutOff < 0x7F) && (bFlt)) SetupChannelFilter(pChn, TRUE);
#endif // NO_FILTER
	}
}


UINT CSoundFile::GetNNAChannel(UINT nChn) const
//---------------------------------------------
{
	const MODCHANNEL *pChn = &Chn[nChn];
	// Check for empty channel
	const MODCHANNEL *pi = &Chn[m_nChannels];
	for (UINT i=m_nChannels; i<MAX_CHANNELS; i++, pi++) if (!pi->nLength) return i;
	if (!pChn->nFadeOutVol) return 0;
	// All channels are used: check for lowest volume
	UINT result = 0;
	DWORD vol = 64*65536;	// 25%
	DWORD envpos = 0xFFFFFF;
	const MODCHANNEL *pj = &Chn[m_nChannels];
	for (UINT j=m_nChannels; j<MAX_CHANNELS; j++, pj++)
	{
		if (!pj->nFadeOutVol) return j;
		DWORD v = pj->nVolume;
		if (pj->dwFlags & CHN_NOTEFADE)
			v = v * pj->nFadeOutVol;
		else
			v <<= 16;
		if (pj->dwFlags & CHN_LOOP) v >>= 1;
		if ((v < vol) || ((v == vol) && (pj->nVolEnvPosition > envpos)))
		{
			envpos = pj->nVolEnvPosition;
			vol = v;
			result = j;
		}
	}
	return result;
}


void CSoundFile::CheckNNA(UINT nChn, UINT instr, int note, BOOL bForceCut)
//------------------------------------------------------------------------
{
	MODCHANNEL *pChn = &Chn[nChn];
	INSTRUMENTHEADER *penv = pChn->pHeader, *pHeader = 0;
	signed char *pSample;
	if (note > 0x80) note = 0;
	if (note < 1) return;
	// Always NNA cut - using
	if ((!(m_nType & (MOD_TYPE_IT|MOD_TYPE_MT2))) || (!m_nInstruments) || (bForceCut))
	{
		if ((m_dwSongFlags & SONG_CPUVERYHIGH)
		 || (!pChn->nLength) || (pChn->dwFlags & CHN_MUTE)
		 || ((!pChn->nLeftVol) && (!pChn->nRightVol))) return;
		UINT n = GetNNAChannel(nChn);
		if (!n) return;
		MODCHANNEL *p = &Chn[n];
		// Copy Channel
		*p = *pChn;
		p->dwFlags &= ~(CHN_VIBRATO|CHN_TREMOLO|CHN_PANBRELLO|CHN_MUTE|CHN_PORTAMENTO);
		p->nMasterChn = nChn+1;
		p->nCommand = 0;
		// Cut the note
		p->nFadeOutVol = 0;
		p->dwFlags |= (CHN_NOTEFADE|CHN_FASTVOLRAMP);
		// Stop this channel
		pChn->nLength = pChn->nPos = pChn->nPosLo = 0;
		pChn->nROfs = pChn->nLOfs = 0;
		pChn->nLeftVol = pChn->nRightVol = 0;
		return;
	}
	if (instr >= MAX_INSTRUMENTS) instr = 0;
	pSample = pChn->pSample;
	pHeader = pChn->pHeader;
	if ((instr) && (note))
	{
		pHeader = Headers[instr];
		if (pHeader)
		{
			UINT n = 0;
			if (note <= 0x80)
			{
				n = pHeader->Keyboard[note-1];
				note = pHeader->NoteMap[note-1];
				if ((n) && (n < MAX_SAMPLES)) pSample = Ins[n].pSample;
			}
		} else pSample = NULL;
	}
	if (!penv) return;
	MODCHANNEL *p = pChn;
	for (UINT i=nChn; i<MAX_CHANNELS; p++, i++)
	if ((i >= m_nChannels) || (p == pChn))
	{
		if (((p->nMasterChn == nChn+1) || (p == pChn)) && (p->pHeader))
		{
			BOOL bOk = FALSE;
			// Duplicate Check Type
			switch(p->pHeader->nDCT)
			{
			// Note
			case DCT_NOTE:
				if ((note) && (p->nNote == note) && (pHeader == p->pHeader)) bOk = TRUE;
				break;
			// Sample
			case DCT_SAMPLE:
				if ((pSample) && (pSample == p->pSample)) bOk = TRUE;
				break;
			// Instrument
			case DCT_INSTRUMENT:
				if (pHeader == p->pHeader) bOk = TRUE;
				break;
			}
			// Duplicate Note Action
			if (bOk)
			{
				switch(p->pHeader->nDNA)
				{
				// Cut
				case DNA_NOTECUT:
					KeyOff(i);
					p->nVolume = 0;
					break;
				// Note Off
				case DNA_NOTEOFF:
					KeyOff(i);
					break;
				// Note Fade
				case DNA_NOTEFADE:
					p->dwFlags |= CHN_NOTEFADE;
					break;
				}
				if (!p->nVolume)
				{
					p->nFadeOutVol = 0;
					p->dwFlags |= (CHN_NOTEFADE|CHN_FASTVOLRAMP);
				}
			}
		}
	}
	if (pChn->dwFlags & CHN_MUTE) return;
	// New Note Action
	if ((pChn->nVolume) && (pChn->nLength))
	{
		UINT n = GetNNAChannel(nChn);
		if (n)
		{
			p = &Chn[n];
			// Copy Channel
			*p = *pChn;
			p->dwFlags &= ~(CHN_VIBRATO|CHN_TREMOLO|CHN_PANBRELLO|CHN_MUTE|CHN_PORTAMENTO);
			p->nMasterChn = nChn+1;
			p->nCommand = 0;
			// Key Off the note
			switch(pChn->nNNA)
			{
			case NNA_NOTEOFF:	KeyOff(n); break;
			case NNA_NOTECUT:
				p->nFadeOutVol = 0;
			case NNA_NOTEFADE:	p->dwFlags |= CHN_NOTEFADE; break;
			}
			if (!p->nVolume)
			{
				p->nFadeOutVol = 0;
				p->dwFlags |= (CHN_NOTEFADE|CHN_FASTVOLRAMP);
			}
			// Stop this channel
			pChn->nLength = pChn->nPos = pChn->nPosLo = 0;
			pChn->nROfs = pChn->nLOfs = 0;
		}
	}
}


BOOL CSoundFile::ProcessEffects()
//-------------------------------
{
	MODCHANNEL *pChn = Chn;
	int nBreakRow = -1, nPosJump = -1, nPatLoopRow = -1;

	for (UINT nChn=0; nChn<m_nChannels; nChn++, pChn++)
	{
		UINT instr = pChn->nRowInstr;
		UINT volcmd = pChn->nRowVolCmd;
		UINT vol = pChn->nRowVolume;
		UINT cmd = pChn->nRowCommand;
		UINT param = pChn->nRowParam;
		bool bPorta = ((cmd != CMD_TONEPORTAMENTO) && (cmd != CMD_TONEPORTAVOL) && (volcmd != VOLCMD_TONEPORTAMENTO)) ? FALSE : TRUE;
		UINT nStartTick = 0;

		pChn->dwFlags &= ~CHN_FASTVOLRAMP;
		// Process special effects (note delay, pattern delay, pattern loop)
		if ((cmd == CMD_MODCMDEX) || (cmd == CMD_S3MCMDEX))
		{
			if ((!param) && (m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT))) param = pChn->nOldCmdEx; else pChn->nOldCmdEx = param;
			// Note Delay ?
			if ((param & 0xF0) == 0xD0)
			{
				nStartTick = param & 0x0F;
			} else
			if (!m_nTickCount)
			{
				// Pattern Loop ?
				if ((((param & 0xF0) == 0x60) && (cmd == CMD_MODCMDEX))
				 || (((param & 0xF0) == 0xB0) && (cmd == CMD_S3MCMDEX)))
				{
					int nloop = PatternLoop(pChn, param & 0x0F);
					if (nloop >= 0) nPatLoopRow = nloop;
				} else
				// Pattern Delay
				if ((param & 0xF0) == 0xE0)
				{
					m_nPatternDelay = param & 0x0F;
				}
			}
		}

		// Handles note/instrument/volume changes
		if (m_nTickCount == nStartTick) // can be delayed by a note delay effect
		{
			UINT note = pChn->nRowNote;
			if (instr) pChn->nNewIns = instr;
			// XM: Key-Off + Sample == Note Cut
			if (m_nType & (MOD_TYPE_MOD|MOD_TYPE_XM|MOD_TYPE_MT2))
			{
				if ((note == 0xFF) && ((!pChn->pHeader) || (!(pChn->pHeader->dwFlags & ENV_VOLUME))))
				{
					pChn->dwFlags |= CHN_FASTVOLRAMP;
					pChn->nVolume = 0;
					note = instr = 0;
				}
			}
			if ((!note) && (instr))
			{
				if (m_nInstruments)
				{
					if (pChn->pInstrument) pChn->nVolume = pChn->pInstrument->nVolume;
					if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
					{
						pChn->dwFlags |= CHN_FASTVOLRAMP;
						pChn->nVolEnvPosition = 0;
						pChn->nPanEnvPosition = 0;
						pChn->nPitchEnvPosition = 0;
						pChn->nAutoVibDepth = 0;
						pChn->nAutoVibPos = 0;
						pChn->dwFlags &= ~CHN_NOTEFADE;
						pChn->nFadeOutVol = 65536;
					}
				} else
				{
					if (instr < MAX_SAMPLES) pChn->nVolume = Ins[instr].nVolume;
				}
				if (!(m_nType & MOD_TYPE_IT)) instr = 0;
			}
			// Invalid Instrument ?
			if (instr >= MAX_INSTRUMENTS) instr = 0;
			// Note Cut/Off => ignore instrument
			if (note >= 0xFE) instr = 0;
			if ((note) && (note <= 128)) pChn->nNewNote = note;
			// New Note Action ?
			if ((note) && (note <= 128) && (!bPorta))
			{
				CheckNNA(nChn, instr, note, FALSE);
			}
			// Instrument Change ?
			if (instr)
			{
				MODINSTRUMENT *psmp = pChn->pInstrument;
				InstrumentChange(pChn, instr, bPorta, TRUE);
				pChn->nNewIns = 0;
				// Special IT case: portamento+note causes sample change -> ignore portamento
				if ((m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT))
				 && (psmp != pChn->pInstrument) && (note) && (note < 0x80))
				{
					bPorta = FALSE;
				}
			}
			// New Note ?
			if (note)
			{
				if ((!instr) && (pChn->nNewIns) && (note < 0x80))
				{
					InstrumentChange(pChn, pChn->nNewIns, bPorta, FALSE, (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2)) ? FALSE : TRUE);
					pChn->nNewIns = 0;
				}
				NoteChange(nChn, note, bPorta, (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2)) ? FALSE : TRUE);
				if ((bPorta) && (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2)) && (instr))
				{
					pChn->dwFlags |= CHN_FASTVOLRAMP;
					pChn->nVolEnvPosition = 0;
					pChn->nPanEnvPosition = 0;
					pChn->nPitchEnvPosition = 0;
					pChn->nAutoVibDepth = 0;
					pChn->nAutoVibPos = 0;
				}
			}
			// Tick-0 only volume commands
			if (volcmd == VOLCMD_VOLUME)
			{
				if (vol > 64) vol = 64;
				pChn->nVolume = vol << 2;
				pChn->dwFlags |= CHN_FASTVOLRAMP;
			} else
			if (volcmd == VOLCMD_PANNING)
			{
				if (vol > 64) vol = 64;
				pChn->nPan = vol << 2;
				pChn->dwFlags |= CHN_FASTVOLRAMP;
			}
		}

		// Volume Column Effect (except volume & panning)
		if ((volcmd > VOLCMD_PANNING) && (m_nTickCount >= nStartTick))
		{
			if (volcmd == VOLCMD_TONEPORTAMENTO)
			{
				if (m_nType & MOD_TYPE_IT)
					TonePortamento(pChn, ImpulseTrackerPortaVolCmd[vol & 0x0F]);
				else
					TonePortamento(pChn, vol * 16);
			} else
			{
				if (vol) pChn->nOldVolParam = vol; else vol = pChn->nOldVolParam;
				switch(volcmd)
				{
				case VOLCMD_VOLSLIDEUP:
					VolumeSlide(pChn, vol << 4);
					break;

				case VOLCMD_VOLSLIDEDOWN:
					VolumeSlide(pChn, vol);
					break;

				case VOLCMD_FINEVOLUP:
					if (m_nType & MOD_TYPE_IT)
					{
						if (m_nTickCount == nStartTick) VolumeSlide(pChn, (vol << 4) | 0x0F);
					} else
						FineVolumeUp(pChn, vol);
					break;

				case VOLCMD_FINEVOLDOWN:
					if (m_nType & MOD_TYPE_IT)
					{
						if (m_nTickCount == nStartTick) VolumeSlide(pChn, 0xF0 | vol);
					} else
						FineVolumeDown(pChn, vol);
					break;

				case VOLCMD_VIBRATOSPEED:
					Vibrato(pChn, vol << 4);
					break;

				case VOLCMD_VIBRATO:
					Vibrato(pChn, vol);
					break;

				case VOLCMD_PANSLIDELEFT:
					PanningSlide(pChn, vol);
					break;

				case VOLCMD_PANSLIDERIGHT:
					PanningSlide(pChn, vol << 4);
					break;

				case VOLCMD_PORTAUP:
					PortamentoUp(pChn, vol << 2);
					break;

				case VOLCMD_PORTADOWN:
					PortamentoDown(pChn, vol << 2);
					break;
				}
			}
		}

		// Effects
		if (cmd) switch (cmd)
		{
		// Set Volume
		case CMD_VOLUME:
			if (!m_nTickCount)
			{
				pChn->nVolume = (param < 64) ? param*4 : 256;
				pChn->dwFlags |= CHN_FASTVOLRAMP;
			}
			break;

		// Portamento Up
		case CMD_PORTAMENTOUP:
			if ((!param) && (m_nType & MOD_TYPE_MOD)) break;
			PortamentoUp(pChn, param);
			break;

		// Portamento Down
		case CMD_PORTAMENTODOWN:
			if ((!param) && (m_nType & MOD_TYPE_MOD)) break;
			PortamentoDown(pChn, param);
			break;

		// Volume Slide
		case CMD_VOLUMESLIDE:
			if ((param) || (m_nType != MOD_TYPE_MOD)) VolumeSlide(pChn, param);
			break;

		// Tone-Portamento
		case CMD_TONEPORTAMENTO:
			TonePortamento(pChn, param);
			break;

		// Tone-Portamento + Volume Slide
		case CMD_TONEPORTAVOL:
			if ((param) || (m_nType != MOD_TYPE_MOD)) VolumeSlide(pChn, param);
			TonePortamento(pChn, 0);
			break;

		// Vibrato
		case CMD_VIBRATO:
			Vibrato(pChn, param);
			break;

		// Vibrato + Volume Slide
		case CMD_VIBRATOVOL:
			if ((param) || (m_nType != MOD_TYPE_MOD)) VolumeSlide(pChn, param);
			Vibrato(pChn, 0);
			break;

		// Set Speed
		case CMD_SPEED:
			if (!m_nTickCount) SetSpeed(param);
			break;

		// Set Tempo
		case CMD_TEMPO:
			if (!m_nTickCount)
			{
				if (m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT))
				{
					if (param) pChn->nOldTempo = param; else param = pChn->nOldTempo;
				}
				SetTempo(param);
			}
			break;

		// Set Offset
		case CMD_OFFSET:
			if (m_nTickCount) break;
			if (param) pChn->nOldOffset = param; else param = pChn->nOldOffset;
			param <<= 8;
			param |= (UINT)(pChn->nOldHiOffset) << 16;
			if ((pChn->nRowNote) && (pChn->nRowNote < 0x80))
			{
				if (bPorta)
					pChn->nPos = param;
				else
					pChn->nPos += param;
				if (pChn->nPos >= pChn->nLength)
				{
					if (!(m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2)))
					{
						pChn->nPos = pChn->nLoopStart;
						if ((m_dwSongFlags & SONG_ITOLDEFFECTS) && (pChn->nLength > 4))
						{
							pChn->nPos = pChn->nLength - 2;
						}
					}
				}
			} else
			if ((param < pChn->nLength) && (m_nType & (MOD_TYPE_MTM|MOD_TYPE_DMF)))
			{
				pChn->nPos = param;
			}
			break;

		// Arpeggio
		case CMD_ARPEGGIO:
			if ((m_nTickCount) || (!pChn->nPeriod) || (!pChn->nNote)) break;
			if ((!param) && (!(m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT)))) break;
			pChn->nCommand = CMD_ARPEGGIO;
			if (param) pChn->nArpeggio = param;
			break;

		// Retrig
		case CMD_RETRIG:
			if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
			{
				if (!(param & 0xF0)) param |= pChn->nRetrigParam & 0xF0;
				if (!(param & 0x0F)) param |= pChn->nRetrigParam & 0x0F;
				param |= 0x100; // increment retrig count on first row
			}
			if (param) pChn->nRetrigParam = (BYTE)(param & 0xFF); else param = pChn->nRetrigParam;
			RetrigNote(nChn, param);
			break;

		// Tremor
		case CMD_TREMOR:
			if (m_nTickCount) break;
			pChn->nCommand = CMD_TREMOR;
			if (param) pChn->nTremorParam = param;
			break;

		// Set Global Volume
		case CMD_GLOBALVOLUME:
			if (m_nTickCount) break;
			if (m_nType != MOD_TYPE_IT) param <<= 1;
			if (param > 128) param = 128;
			m_nGlobalVolume = param << 1;
			break;

		// Global Volume Slide
		case CMD_GLOBALVOLSLIDE:
			GlobalVolSlide(param);
			break;

		// Set 8-bit Panning
		case CMD_PANNING8:
			if (m_nTickCount) break;
			if (!(m_dwSongFlags & SONG_SURROUNDPAN)) pChn->dwFlags &= ~CHN_SURROUND;
			if (m_nType & (MOD_TYPE_IT|MOD_TYPE_XM|MOD_TYPE_MT2))
			{
				pChn->nPan = param;
			} else
			if (param <= 0x80)
			{
				pChn->nPan = param << 1;
			} else
			if (param == 0xA4)
			{
				pChn->dwFlags |= CHN_SURROUND;
				pChn->nPan = 0x80;
			}
			pChn->dwFlags |= CHN_FASTVOLRAMP;
			break;

		// Panning Slide
		case CMD_PANNINGSLIDE:
			PanningSlide(pChn, param);
			break;

		// Tremolo
		case CMD_TREMOLO:
			Tremolo(pChn, param);
			break;

		// Fine Vibrato
		case CMD_FINEVIBRATO:
			FineVibrato(pChn, param);
			break;

		// MOD/XM Exx Extended Commands
		case CMD_MODCMDEX:
			ExtendedMODCommands(nChn, param);
			break;

		// S3M/IT Sxx Extended Commands
		case CMD_S3MCMDEX:
			ExtendedS3MCommands(nChn, param);
			break;

		// Key Off
		case CMD_KEYOFF:
			if (!m_nTickCount) KeyOff(nChn);
			break;

		// Extra-fine porta up/down
		case CMD_XFINEPORTAUPDOWN:
			switch(param & 0xF0)
			{
			case 0x10: ExtraFinePortamentoUp(pChn, param & 0x0F); break;
			case 0x20: ExtraFinePortamentoDown(pChn, param & 0x0F); break;
			// Modplug XM Extensions
			case 0x50:
			case 0x60:
			case 0x70:
			case 0x90:
			case 0xA0: ExtendedS3MCommands(nChn, param); break;
			}
			break;

		// Set Channel Global Volume
		case CMD_CHANNELVOLUME:
			if (m_nTickCount) break;
			if (param <= 64)
			{
				pChn->nGlobalVol = param;
				pChn->dwFlags |= CHN_FASTVOLRAMP;
			}
			break;

		// Channel volume slide
		case CMD_CHANNELVOLSLIDE:
			ChannelVolSlide(pChn, param);
			break;

		// Panbrello (IT)
		case CMD_PANBRELLO:
			Panbrello(pChn, param);
			break;

		// Set Envelope Position
		case CMD_SETENVPOSITION:
			if (!m_nTickCount)
			{
				pChn->nVolEnvPosition = param;
				pChn->nPanEnvPosition = param;
				pChn->nPitchEnvPosition = param;
				if (pChn->pHeader)
				{
					INSTRUMENTHEADER *penv = pChn->pHeader;
					if ((pChn->dwFlags & CHN_PANENV) && (penv->nPanEnv) && (param > penv->PanPoints[penv->nPanEnv-1]))
					{
						pChn->dwFlags &= ~CHN_PANENV;
					}
				}
			}
			break;

		// Position Jump
		case CMD_POSITIONJUMP:
			nPosJump = param;
			m_nNextStartRow = 0;
			break;

		// Pattern Break
		case CMD_PATTERNBREAK:
			nBreakRow = param;
			m_nNextStartRow = 0;
			break;

		// Midi Controller
		case CMD_MIDI:
			if (m_nTickCount) break;
			if (param < 0x80){
				ProcessMidiMacro(nChn, &m_MidiCfg.szMidiSFXExt[pChn->nActiveMacro << 5], param);
			} else {
				ProcessMidiMacro(nChn, &m_MidiCfg.szMidiZXXExt[(param & 0x7F) << 5], 0);
			}
			break;
		}
	}

	// Navigation Effects
	if (!m_nTickCount)
	{
		// Pattern Loop
		if (nPatLoopRow >= 0)
		{
			m_nNextPattern = m_nCurrentPattern;
			m_nNextRow = nPatLoopRow;
			if (m_nPatternDelay) m_nNextRow++;
		} else
		// Pattern Break / Position Jump only if no loop running
		if ((nBreakRow >= 0) || (nPosJump >= 0))
		{
			BOOL bNoLoop = FALSE;
			if (nPosJump < 0) nPosJump = m_nCurrentPattern+1;
			if (nBreakRow < 0) nBreakRow = 0;
			// Modplug Tracker & ModPlugin allow backward jumps
		#ifndef MODPLUG_FASTSOUNDLIB
			if ((nPosJump < (int)m_nCurrentPattern)
			 || ((nPosJump == (int)m_nCurrentPattern) && (nBreakRow <= (int)m_nRow)))
			{
				if (!IsValidBackwardJump(m_nCurrentPattern, m_nRow, nPosJump, nBreakRow))
				{
					if (m_nRepeatCount)
					{
						if (m_nRepeatCount > 0) m_nRepeatCount--;
					} else
					{
					#ifdef MODPLUG_TRACKER
						if (gdwSoundSetup & SNDMIX_NOBACKWARDJUMPS)
					#endif
						// Backward jump disabled
						bNoLoop = TRUE;
						//reset repeat count incase there are multiple loops.
						//(i.e. Unreal tracks)
						m_nRepeatCount = m_nInitialRepeatCount;
					}
				}
			}
		#endif	// MODPLUG_FASTSOUNDLIB
			if (((!bNoLoop) && (nPosJump < MAX_ORDERS))
			 && ((nPosJump != (int)m_nCurrentPattern) || (nBreakRow != (int)m_nRow)))
			{
				if (nPosJump != (int)m_nCurrentPattern)
				{
					for (UINT i=0; i<m_nChannels; i++) Chn[i].nPatternLoopCount = 0;
				}
				m_nNextPattern = nPosJump;
				m_nNextRow = (UINT)nBreakRow;
			}
		}
	}
	return TRUE;
}


////////////////////////////////////////////////////////////
// Channels effects

void CSoundFile::PortamentoUp(MODCHANNEL *pChn, UINT param)
//---------------------------------------------------------
{
	if (param) pChn->nOldPortaUpDown = param; else param = pChn->nOldPortaUpDown;
	if ((m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_STM)) && ((param & 0xF0) >= 0xE0))
	{
		if (param & 0x0F)
		{
			if ((param & 0xF0) == 0xF0)
			{
				FinePortamentoUp(pChn, param & 0x0F);
			} else
			if ((param & 0xF0) == 0xE0)
			{
				ExtraFinePortamentoUp(pChn, param & 0x0F);
			}
		}
		return;
	}
	// Regular Slide
	if (!(m_dwSongFlags & SONG_FIRSTTICK) || (m_nMusicSpeed == 1))  //rewbs.PortaA01fix
	{
		DoFreqSlide(pChn, -(int)(param * 4));
	}
}


void CSoundFile::PortamentoDown(MODCHANNEL *pChn, UINT param)
//-----------------------------------------------------------
{
	if (param) pChn->nOldPortaUpDown = param; else param = pChn->nOldPortaUpDown;
	if ((m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_STM)) && ((param & 0xF0) >= 0xE0))
	{
		if (param & 0x0F)
		{
			if ((param & 0xF0) == 0xF0)
			{
				FinePortamentoDown(pChn, param & 0x0F);
			} else
			if ((param & 0xF0) == 0xE0)
			{
				ExtraFinePortamentoDown(pChn, param & 0x0F);
			}
		}
		return;
	}
	if (!(m_dwSongFlags & SONG_FIRSTTICK) || (m_nMusicSpeed == 1)) { //rewbs.PortaA01fix
		DoFreqSlide(pChn, (int)(param << 2));
	}
}


void CSoundFile::FinePortamentoUp(MODCHANNEL *pChn, UINT param)
//-------------------------------------------------------------
{
	if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
	{
		if (param) pChn->nOldFinePortaUpDown = param; else param = pChn->nOldFinePortaUpDown;
	}
	if (m_dwSongFlags & SONG_FIRSTTICK)
	{
		if ((pChn->nPeriod) && (param))
		{
			if ((m_dwSongFlags & SONG_LINEARSLIDES) && (!(m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))))
			{
				pChn->nPeriod = _muldivr(pChn->nPeriod, LinearSlideDownTable[param & 0x0F], 65536);
			} else
			{
				pChn->nPeriod -= (int)(param * 4);
			}
			if (pChn->nPeriod < 1) pChn->nPeriod = 1;
		}
	}
}


void CSoundFile::FinePortamentoDown(MODCHANNEL *pChn, UINT param)
//---------------------------------------------------------------
{
	if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
	{
		if (param) pChn->nOldFinePortaUpDown = param; else param = pChn->nOldFinePortaUpDown;
	}
	if (m_dwSongFlags & SONG_FIRSTTICK)
	{
		if ((pChn->nPeriod) && (param))
		{
			if ((m_dwSongFlags & SONG_LINEARSLIDES) && (!(m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))))
			{
				pChn->nPeriod = _muldivr(pChn->nPeriod, LinearSlideUpTable[param & 0x0F], 65536);
			} else
			{
				pChn->nPeriod += (int)(param * 4);
			}
			if (pChn->nPeriod > 0xFFFF) pChn->nPeriod = 0xFFFF;
		}
	}
}


void CSoundFile::ExtraFinePortamentoUp(MODCHANNEL *pChn, UINT param)
//------------------------------------------------------------------
{
	if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
	{
		if (param) pChn->nOldFinePortaUpDown = param; else param = pChn->nOldFinePortaUpDown;
	}
	if (m_dwSongFlags & SONG_FIRSTTICK)
	{
		if ((pChn->nPeriod) && (param))
		{
			if ((m_dwSongFlags & SONG_LINEARSLIDES) && (!(m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))))
			{
				pChn->nPeriod = _muldivr(pChn->nPeriod, FineLinearSlideDownTable[param & 0x0F], 65536);
			} else
			{
				pChn->nPeriod -= (int)(param);
			}
			if (pChn->nPeriod < 1) pChn->nPeriod = 1;
		}
	}
}


void CSoundFile::ExtraFinePortamentoDown(MODCHANNEL *pChn, UINT param)
//--------------------------------------------------------------------
{
	if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
	{
		if (param) pChn->nOldFinePortaUpDown = param; else param = pChn->nOldFinePortaUpDown;
	}
	if (m_dwSongFlags & SONG_FIRSTTICK)
	{
		if ((pChn->nPeriod) && (param))
		{
			if ((m_dwSongFlags & SONG_LINEARSLIDES) && (!(m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))))
			{
				pChn->nPeriod = _muldivr(pChn->nPeriod, FineLinearSlideUpTable[param & 0x0F], 65536);
			} else
			{
				pChn->nPeriod += (int)(param);
			}
			if (pChn->nPeriod > 0xFFFF) pChn->nPeriod = 0xFFFF;
		}
	}
}


// Portamento Slide
void CSoundFile::TonePortamento(MODCHANNEL *pChn, UINT param)
//-----------------------------------------------------------
{
	if (param) pChn->nPortamentoSlide = param * 4;
	pChn->dwFlags |= CHN_PORTAMENTO;
	if ((pChn->nPeriod) && (pChn->nPortamentoDest) && (!(m_dwSongFlags & SONG_FIRSTTICK)))
	{
		if (pChn->nPeriod < pChn->nPortamentoDest)
		{
			LONG delta = (int)pChn->nPortamentoSlide;
			if ((m_dwSongFlags & SONG_LINEARSLIDES) && (!(m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))))
			{
				UINT n = pChn->nPortamentoSlide >> 2;
				if (n > 255) n = 255;
				delta = _muldivr(pChn->nPeriod, LinearSlideUpTable[n], 65536) - pChn->nPeriod;
				if (delta < 1) delta = 1;
			}
			pChn->nPeriod += delta;
			if (pChn->nPeriod > pChn->nPortamentoDest) pChn->nPeriod = pChn->nPortamentoDest;
		} else
		if (pChn->nPeriod > pChn->nPortamentoDest)
		{
			LONG delta = - (int)pChn->nPortamentoSlide;
			if ((m_dwSongFlags & SONG_LINEARSLIDES) && (!(m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))))
			{
				UINT n = pChn->nPortamentoSlide >> 2;
				if (n > 255) n = 255;
				delta = _muldivr(pChn->nPeriod, LinearSlideDownTable[n], 65536) - pChn->nPeriod;
				if (delta > -1) delta = -1;
			}
			pChn->nPeriod += delta;
			if (pChn->nPeriod < pChn->nPortamentoDest) pChn->nPeriod = pChn->nPortamentoDest;
		}
	}
}


void CSoundFile::Vibrato(MODCHANNEL *p, UINT param)
//-------------------------------------------------
{
	if (param & 0x0F) p->nVibratoDepth = (param & 0x0F) * 4;
	if (param & 0xF0) p->nVibratoSpeed = (param >> 4) & 0x0F;
	p->dwFlags |= CHN_VIBRATO;
}


void CSoundFile::FineVibrato(MODCHANNEL *p, UINT param)
//-----------------------------------------------------
{
	if (param & 0x0F) p->nVibratoDepth = param & 0x0F;
	if (param & 0xF0) p->nVibratoSpeed = (param >> 4) & 0x0F;
	p->dwFlags |= CHN_VIBRATO;
}


void CSoundFile::Panbrello(MODCHANNEL *p, UINT param)
//---------------------------------------------------
{
	if (param & 0x0F) p->nPanbrelloDepth = param & 0x0F;
	if (param & 0xF0) p->nPanbrelloSpeed = (param >> 4) & 0x0F;
	p->dwFlags |= CHN_PANBRELLO;
}


void CSoundFile::VolumeSlide(MODCHANNEL *pChn, UINT param)
//--------------------------------------------------------
{
	if (param) pChn->nOldVolumeSlide = param; else param = pChn->nOldVolumeSlide;
	LONG newvolume = pChn->nVolume;
	if (m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_STM|MOD_TYPE_AMF))
	{
		if ((param & 0x0F) == 0x0F)
		{
			if (param & 0xF0)
			{
				FineVolumeUp(pChn, (param >> 4));
				return;
			} else
			{
				if ((m_dwSongFlags & SONG_FIRSTTICK) && (!(m_dwSongFlags & SONG_FASTVOLSLIDES)))
				{
					newvolume -= 0x0F * 4;
				}
			}
		} else
		if ((param & 0xF0) == 0xF0)
		{
			if (param & 0x0F)
			{
				FineVolumeDown(pChn, (param & 0x0F));
				return;
			} else
			{
				if ((m_dwSongFlags & SONG_FIRSTTICK) && (!(m_dwSongFlags & SONG_FASTVOLSLIDES)))
				{
					newvolume += 0x0F * 4;
				}
			}
		}
	}
	if ((!(m_dwSongFlags & SONG_FIRSTTICK)) || (m_dwSongFlags & SONG_FASTVOLSLIDES))
	{
		if (param & 0x0F) newvolume -= (int)((param & 0x0F) * 4);
		else newvolume += (int)((param & 0xF0) >> 2);
		if (m_nType & MOD_TYPE_MOD) pChn->dwFlags |= CHN_FASTVOLRAMP;
	}
	if (newvolume < 0) newvolume = 0;
	if (newvolume > 256) newvolume = 256;
	pChn->nVolume = newvolume;
}


void CSoundFile::PanningSlide(MODCHANNEL *pChn, UINT param)
//---------------------------------------------------------
{
	LONG nPanSlide = 0;
	if (param) pChn->nOldPanSlide = param; else param = pChn->nOldPanSlide;
	if (m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_STM))
	{
		if (((param & 0x0F) == 0x0F) && (param & 0xF0))
		{
			if (m_dwSongFlags & SONG_FIRSTTICK)
			{
				param = (param & 0xF0) >> 2;
				nPanSlide = - (int)param;
			}
		} else
		if (((param & 0xF0) == 0xF0) && (param & 0x0F))
		{
			if (m_dwSongFlags & SONG_FIRSTTICK)
			{
				nPanSlide = (param & 0x0F) << 2;
			}
		} else
		{
			if (!(m_dwSongFlags & SONG_FIRSTTICK))
			{
				if (param & 0x0F) nPanSlide = (int)((param & 0x0F) << 2);
				else nPanSlide = -(int)((param & 0xF0) >> 2);
			}
		}
	} else
	{
		if (!(m_dwSongFlags & SONG_FIRSTTICK))
		{
			if (param & 0x0F) nPanSlide = -(int)((param & 0x0F) << 2);
			else nPanSlide = (int)((param & 0xF0) >> 2);
		}
	}
	if (nPanSlide)
	{
		nPanSlide += pChn->nPan;
		if (nPanSlide < 0) nPanSlide = 0;
		if (nPanSlide > 256) nPanSlide = 256;
		pChn->nPan = nPanSlide;
	}
}


void CSoundFile::FineVolumeUp(MODCHANNEL *pChn, UINT param)
//---------------------------------------------------------
{
	if (param) pChn->nOldFineVolUpDown = param; else param = pChn->nOldFineVolUpDown;
	if (m_dwSongFlags & SONG_FIRSTTICK)
	{
		pChn->nVolume += param * 4;
		if (pChn->nVolume > 256) pChn->nVolume = 256;
		if (m_nType & MOD_TYPE_MOD) pChn->dwFlags |= CHN_FASTVOLRAMP;
	}
}


void CSoundFile::FineVolumeDown(MODCHANNEL *pChn, UINT param)
//-----------------------------------------------------------
{
	if (param) pChn->nOldFineVolUpDown = param; else param = pChn->nOldFineVolUpDown;
	if (m_dwSongFlags & SONG_FIRSTTICK)
	{
		pChn->nVolume -= param * 4;
		if (pChn->nVolume < 0) pChn->nVolume = 0;
		if (m_nType & MOD_TYPE_MOD) pChn->dwFlags |= CHN_FASTVOLRAMP;
	}
}


void CSoundFile::Tremolo(MODCHANNEL *p, UINT param)
//-------------------------------------------------
{
	if (param & 0x0F) p->nTremoloDepth = (param & 0x0F) << 2;
	if (param & 0xF0) p->nTremoloSpeed = (param >> 4) & 0x0F;
	p->dwFlags |= CHN_TREMOLO;
}


void CSoundFile::ChannelVolSlide(MODCHANNEL *pChn, UINT param)
//------------------------------------------------------------
{
	LONG nChnSlide = 0;
	if (param) pChn->nOldChnVolSlide = param; else param = pChn->nOldChnVolSlide;
	if (((param & 0x0F) == 0x0F) && (param & 0xF0))
	{
		if (m_dwSongFlags & SONG_FIRSTTICK) nChnSlide = param >> 4;
	} else
	if (((param & 0xF0) == 0xF0) && (param & 0x0F))
	{
		if (m_dwSongFlags & SONG_FIRSTTICK) nChnSlide = - (int)(param & 0x0F);
	} else
	{
		if (!(m_dwSongFlags & SONG_FIRSTTICK))
		{
			if (param & 0x0F) nChnSlide = -(int)(param & 0x0F);
			else nChnSlide = (int)((param & 0xF0) >> 4);
		}
	}
	if (nChnSlide)
	{
		nChnSlide += pChn->nGlobalVol;
		if (nChnSlide < 0) nChnSlide = 0;
		if (nChnSlide > 64) nChnSlide = 64;
		pChn->nGlobalVol = nChnSlide;
	}
}


void CSoundFile::ExtendedMODCommands(UINT nChn, UINT param)
//---------------------------------------------------------
{
	MODCHANNEL *pChn = &Chn[nChn];
	UINT command = param & 0xF0;
	param &= 0x0F;
	switch(command)
	{
	// E0x: Set Filter
	// E1x: Fine Portamento Up
	case 0x10:	if ((param) || (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))) FinePortamentoUp(pChn, param); break;
	// E2x: Fine Portamento Down
	case 0x20:	if ((param) || (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))) FinePortamentoDown(pChn, param); break;
	// E3x: Set Glissando Control
	case 0x30:	pChn->dwFlags &= ~CHN_GLISSANDO; if (param) pChn->dwFlags |= CHN_GLISSANDO; break;
	// E4x: Set Vibrato WaveForm
	case 0x40:	pChn->nVibratoType = param & 0x07; break;
	// E5x: Set FineTune
	case 0x50:	if (m_nTickCount) break;
				pChn->nC4Speed = S3MFineTuneTable[param];
				if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
					pChn->nFineTune = param*2;
				else
					pChn->nFineTune = MOD2XMFineTune(param);
				if (pChn->nPeriod) pChn->nPeriod = GetPeriodFromNote(pChn->nNote, pChn->nFineTune, pChn->nC4Speed);
				break;
	// E6x: Pattern Loop
	// E7x: Set Tremolo WaveForm
	case 0x70:	pChn->nTremoloType = param & 0x07; break;
	// E8x: Set 4-bit Panning
	case 0x80:	if (!m_nTickCount) { pChn->nPan = (param << 4) + 8; pChn->dwFlags |= CHN_FASTVOLRAMP; } break;
	// E9x: Retrig
	case 0x90:	RetrigNote(nChn, param); break;
	// EAx: Fine Volume Up
	case 0xA0:	if ((param) || (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))) FineVolumeUp(pChn, param); break;
	// EBx: Fine Volume Down
	case 0xB0:	if ((param) || (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))) FineVolumeDown(pChn, param); break;
	// ECx: Note Cut
	case 0xC0:	NoteCut(nChn, param); break;
	// EDx: Note Delay
	// EEx: Pattern Delay
	// EFx: MOD: Invert Loop, XM: Set Active Midi Macro
	case 0xF0:	pChn->nActiveMacro = param;	break;
	}
}


void CSoundFile::ExtendedS3MCommands(UINT nChn, UINT param)
//---------------------------------------------------------
{
	MODCHANNEL *pChn = &Chn[nChn];
	UINT command = param & 0xF0;
	param &= 0x0F;
	switch(command)
	{
	// S0x: Set Filter
	// S1x: Set Glissando Control
	case 0x10:	pChn->dwFlags &= ~CHN_GLISSANDO; if (param) pChn->dwFlags |= CHN_GLISSANDO; break;
	// S2x: Set FineTune
	case 0x20:	if (m_nTickCount) break;
				pChn->nC4Speed = S3MFineTuneTable[param & 0x0F];
				pChn->nFineTune = MOD2XMFineTune(param);
				if (pChn->nPeriod) pChn->nPeriod = GetPeriodFromNote(pChn->nNote, pChn->nFineTune, pChn->nC4Speed);
				break;
	// S3x: Set Vibrato WaveForm
	case 0x30:	pChn->nVibratoType = param & 0x07; break;
	// S4x: Set Tremolo WaveForm
	case 0x40:	pChn->nTremoloType = param & 0x07; break;
	// S5x: Set Panbrello WaveForm
	case 0x50:	pChn->nPanbrelloType = param & 0x07; break;
	// S6x: Pattern Delay for x frames
	case 0x60:	m_nFrameDelay = param; break;
	// S7x: Envelope Control
	case 0x70:	if (m_nTickCount) break;
				switch(param)
				{
				case 0:
				case 1:
				case 2:
					{
						MODCHANNEL *bkp = &Chn[m_nChannels];
						for (UINT i=m_nChannels; i<MAX_CHANNELS; i++, bkp++)
						{
							if (bkp->nMasterChn == nChn+1)
							{
								if (param == 1) KeyOff(i); else
								if (param == 2) bkp->dwFlags |= CHN_NOTEFADE; else
									{ bkp->dwFlags |= CHN_NOTEFADE; bkp->nFadeOutVol = 0; }
							}
						}
					}
					break;
				case 3:		pChn->nNNA = NNA_NOTECUT; break;
				case 4:		pChn->nNNA = NNA_CONTINUE; break;
				case 5:		pChn->nNNA = NNA_NOTEOFF; break;
				case 6:		pChn->nNNA = NNA_NOTEFADE; break;
				case 7:		pChn->dwFlags &= ~CHN_VOLENV; break;
				case 8:		pChn->dwFlags |= CHN_VOLENV; break;
				case 9:		pChn->dwFlags &= ~CHN_PANENV; break;
				case 10:	pChn->dwFlags |= CHN_PANENV; break;
				case 11:	pChn->dwFlags &= ~CHN_PITCHENV; break;
				case 12:	pChn->dwFlags |= CHN_PITCHENV; break;
				}
				break;
	// S8x: Set 4-bit Panning
	case 0x80:	if (!m_nTickCount) { pChn->nPan = (param << 4) + 8; pChn->dwFlags |= CHN_FASTVOLRAMP; } break;
	// S9x: Set Surround
	case 0x90:	ExtendedChannelEffect(pChn, param & 0x0F); break;
	// SAx: Set 64k Offset
	case 0xA0:	if (!m_nTickCount)
				{
					pChn->nOldHiOffset = param;
					if ((pChn->nRowNote) && (pChn->nRowNote < 0x80))
					{
						DWORD pos = param << 16;
						if (pos < pChn->nLength) pChn->nPos = pos;
					}
				}
				break;
	// SBx: Pattern Loop
	// SCx: Note Cut
	case 0xC0:	NoteCut(nChn, param); break;
	// SDx: Note Delay
	// case 0xD0:	break;
	// SEx: Pattern Delay for x rows
	// SFx: S3M: Funk Repeat, IT: Set Active Midi Macro
	case 0xF0:	pChn->nActiveMacro = param; break;
	}
}


void CSoundFile::ExtendedChannelEffect(MODCHANNEL *pChn, UINT param)
//------------------------------------------------------------------
{
	// S9x and X9x commands (S3M/XM/IT only)
	if (m_nTickCount) return;
	switch(param & 0x0F)
	{
	// S90: Surround Off
	case 0x00:	pChn->dwFlags &= ~CHN_SURROUND;	break;
	// S91: Surround On
	case 0x01:	pChn->dwFlags |= CHN_SURROUND; pChn->nPan = 128; break;
	////////////////////////////////////////////////////////////
	// Modplug Extensions
	// S98: Reverb Off
	case 0x08:
		pChn->dwFlags &= ~CHN_REVERB;
		pChn->dwFlags |= CHN_NOREVERB;
		break;
	// S99: Reverb On
	case 0x09:
		pChn->dwFlags &= ~CHN_NOREVERB;
		pChn->dwFlags |= CHN_REVERB;
		break;
	// S9A: 2-Channels surround mode
	case 0x0A:
		m_dwSongFlags &= ~SONG_SURROUNDPAN;
		break;
	// S9B: 4-Channels surround mode
	case 0x0B:
		m_dwSongFlags |= SONG_SURROUNDPAN;
		break;
	// S9C: IT Filter Mode
	case 0x0C:
		m_dwSongFlags &= ~SONG_MPTFILTERMODE;
		break;
	// S9D: MPT Filter Mode
	case 0x0D:
		m_dwSongFlags |= SONG_MPTFILTERMODE;
		break;
	// S9E: Go forward
	case 0x0E:
		pChn->dwFlags &= ~(CHN_PINGPONGFLAG);
		break;
	// S9F: Go backward (set position at the end for non-looping samples)
	case 0x0F:
		if ((!(pChn->dwFlags & CHN_LOOP)) && (!pChn->nPos) && (pChn->nLength))
		{
			pChn->nPos = pChn->nLength - 1;
			pChn->nPosLo = 0xFFFF;
		}
		pChn->dwFlags |= CHN_PINGPONGFLAG;
		break;
	}
}


void CSoundFile::ProcessMidiMacro(UINT nChn, LPCSTR pszMidiMacro, UINT param)
//---------------------------------------------------------------------------
{
	MODCHANNEL *pChn = &Chn[nChn];
	DWORD dwMacro = (*((LPDWORD)pszMidiMacro)) & 0x7F5F7F5F;
	// Not Internal Device ?
	if (dwMacro != 0x30463046 && dwMacro != 0x31463046)
	{
		UINT pos = 0, nNib = 0, nBytes = 0;
		DWORD dwMidiCode = 0, dwByteCode = 0;
		while (pos+6 <= 32)
		{
			CHAR cData = pszMidiMacro[pos++];
			if (!cData) break;
			if ((cData >= '0') && (cData <= '9')) { dwByteCode = (dwByteCode<<4) | (cData-'0'); nNib++; } else
			if ((cData >= 'A') && (cData <= 'F')) { dwByteCode = (dwByteCode<<4) | (cData-'A'+10); nNib++; } else
			if ((cData >= 'a') && (cData <= 'f')) { dwByteCode = (dwByteCode<<4) | (cData-'a'+10); nNib++; } else
			if ((cData == 'z') || (cData == 'Z')) { dwByteCode = param & 0x7f; nNib = 2; } else
			if ((cData == 'x') || (cData == 'X')) { dwByteCode = param & 0x70; nNib = 2; } else
			if ((cData == 'y') || (cData == 'Y')) { dwByteCode = (param & 0x0f)<<3; nNib = 2; } else
			if (nNib >= 2)
			{
				nNib = 0;
				dwMidiCode |= dwByteCode << (nBytes*8);
				dwByteCode = 0;
				nBytes++;
				if (nBytes >= 3)
				{
					UINT nMasterCh = (nChn < m_nChannels) ? nChn+1 : pChn->nMasterChn;
					if ((nMasterCh) && (nMasterCh <= m_nChannels))
					{
						UINT nPlug = ChnSettings[nMasterCh-1].nMixPlugin;
						if ((nPlug) && (nPlug <= MAX_MIXPLUGINS))
						{
							IMixPlugin *pPlugin = m_MixPlugins[nPlug-1].pMixPlugin;
							if ((pPlugin) && (m_MixPlugins[nPlug-1].pMixState))
							{
								pPlugin->MidiSend(dwMidiCode);
							}
						}
					}
					nBytes = 0;
					dwMidiCode = 0;
				}
			}

		}
		return;
	}
	// Internal device
	pszMidiMacro += 4;
	// Filter ?
	if (pszMidiMacro[0] == '0')
	{
		CHAR cData1 = pszMidiMacro[2];
		DWORD dwParam = 0;
		if ((cData1 == 'z') || (cData1 == 'Z'))
		{
			dwParam = param;
		} else
		{
			CHAR cData2 = pszMidiMacro[3];
			if ((cData1 >= '0') && (cData1 <= '9')) dwParam += (cData1 - '0') << 4; else
			if ((cData1 >= 'A') && (cData1 <= 'F')) dwParam += (cData1 - 'A' + 0x0A) << 4;
			if ((cData2 >= '0') && (cData2 <= '9')) dwParam += (cData2 - '0'); else
			if ((cData2 >= 'A') && (cData2 <= 'F')) dwParam += (cData2 - 'A' + 0x0A);
		}
		switch(pszMidiMacro[1])
		{
		// F0.F0.00.xx: Set CutOff
		case '0':
			{
				int oldcutoff = pChn->nCutOff;
				if (dwParam < 0x80) pChn->nCutOff = (BYTE)dwParam;
#ifndef NO_FILTER
				oldcutoff -= pChn->nCutOff;

				if (oldcutoff < 0) oldcutoff = -oldcutoff;
				if ((pChn->nVolume > 0) || (oldcutoff < 0x10)
				 || (!(pChn->dwFlags & CHN_FILTER)) || (!(pChn->nLeftVol|pChn->nRightVol)))
					SetupChannelFilter(pChn, (pChn->dwFlags & CHN_FILTER) ? FALSE : TRUE);
#endif // NO_FILTER
			}
			break;

		// F0.F0.01.xx: Set Resonance
		case '1':
			if (dwParam < 0x80) pChn->nResonance = (BYTE)dwParam;
#ifndef NO_FILTER
			SetupChannelFilter(pChn, (pChn->dwFlags & CHN_FILTER) ? FALSE : TRUE);
#endif // NO_FILTER

			break;
		}

	}
}


void CSoundFile::RetrigNote(UINT nChn, UINT param)
//------------------------------------------------
{
	// Retrig: bit 8 is set if it's the new XM retrig
	MODCHANNEL *pChn = &Chn[nChn];
	UINT nRetrigSpeed = param & 0x0F;
	UINT nRetrigCount = pChn->nRetrigCount;
	BOOL bDoRetrig = FALSE;

	if (m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT))
	{
		if (!nRetrigSpeed) nRetrigSpeed = 1;
		if ((nRetrigCount) && (!(nRetrigCount % nRetrigSpeed))) bDoRetrig = TRUE;
		nRetrigCount++;
	} else
	{
		UINT realspeed = nRetrigSpeed;
		if ((param & 0x100) && (pChn->nRowVolCmd == VOLCMD_VOLUME) && (pChn->nRowParam & 0xF0)) realspeed++;
		if ((m_nTickCount) || (param & 0x100))
		{
			if (!realspeed) realspeed = 1;
			if ((!(param & 0x100)) && (m_nMusicSpeed) && (!(m_nTickCount % realspeed))) bDoRetrig = TRUE;
			nRetrigCount++;
		} else if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2)) nRetrigCount = 0;
		if (nRetrigCount >= realspeed)
		{
			if ((m_nTickCount) || ((param & 0x100) && (!pChn->nRowNote))) bDoRetrig = TRUE;
		}
	}
	if (bDoRetrig)
	{
		UINT dv = (param >> 4) & 0x0F;
		if (dv)
		{
			int vol = pChn->nVolume;
			if (retrigTable1[dv])
				vol = (vol * retrigTable1[dv]) >> 4;
			else
				vol += ((int)retrigTable2[dv]) << 2;
			if (vol < 0) vol = 0;
			if (vol > 256) vol = 256;
			pChn->nVolume = vol;
			pChn->dwFlags |= CHN_FASTVOLRAMP;
		}
		UINT nNote = pChn->nNewNote;
		LONG nOldPeriod = pChn->nPeriod;
		if ((nNote) && (nNote <= NOTE_MAX) && (pChn->nLength)) CheckNNA(nChn, 0, nNote, TRUE);
		BOOL bResetEnv = FALSE;
		if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
		{
			if ((pChn->nRowInstr) && (param < 0x100)) { InstrumentChange(pChn, pChn->nRowInstr, FALSE, FALSE); bResetEnv = TRUE; }
			if (param < 0x100) bResetEnv = TRUE;
		}
		NoteChange(nChn, nNote, FALSE, bResetEnv);
		if ((m_nType & MOD_TYPE_IT) && (!pChn->nRowNote) && (nOldPeriod)) pChn->nPeriod = nOldPeriod;
		if (!(m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT))) nRetrigCount = 0;
	}
	pChn->nRetrigCount = (BYTE)nRetrigCount;
}


void CSoundFile::DoFreqSlide(MODCHANNEL *pChn, LONG nFreqSlide)
//-------------------------------------------------------------
{
	// IT Linear slides
	if (!pChn->nPeriod) return;
	if ((m_dwSongFlags & SONG_LINEARSLIDES) && (!(m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))))
	{
		if (nFreqSlide < 0)
		{
			UINT n = (- nFreqSlide) >> 2;
			if (n > 255) n = 255;
			pChn->nPeriod = _muldivr(pChn->nPeriod, LinearSlideDownTable[n], 65536);
		} else
		{
			UINT n = (nFreqSlide) >> 2;

			if (n > 255) n = 255;
			pChn->nPeriod = _muldivr(pChn->nPeriod, LinearSlideUpTable[n], 65536);
		}
	} else
	{
		pChn->nPeriod += nFreqSlide;
	}
	if (pChn->nPeriod < 1)
	{
		pChn->nPeriod = 1;
		if (m_nType & MOD_TYPE_IT)
		{
			pChn->dwFlags |= CHN_NOTEFADE;
			pChn->nFadeOutVol = 0;
		}
	}
}


void CSoundFile::NoteCut(UINT nChn, UINT nTick)
//---------------------------------------------
{
	if (m_nTickCount == nTick)
	{
		MODCHANNEL *pChn = &Chn[nChn];
		// if (m_nInstruments) KeyOff(pChn); ?
		pChn->nVolume = 0;
		pChn->dwFlags |= CHN_FASTVOLRAMP;
	}
}


void CSoundFile::KeyOff(UINT nChn)
//--------------------------------
{
	MODCHANNEL *pChn = &Chn[nChn];
	BOOL bKeyOn = (pChn->dwFlags & CHN_KEYOFF) ? FALSE : TRUE;
	pChn->dwFlags |= CHN_KEYOFF;
	//if ((!pChn->pHeader) || (!(pChn->dwFlags & CHN_VOLENV)))
	if ((pChn->pHeader) && (!(pChn->dwFlags & CHN_VOLENV)))
	{
		pChn->dwFlags |= CHN_NOTEFADE;
	}
	if (!pChn->nLength) return;
	if ((pChn->dwFlags & CHN_SUSTAINLOOP) && (pChn->pInstrument) && (bKeyOn))
	{
		MODINSTRUMENT *psmp = pChn->pInstrument;
		if (psmp->uFlags & CHN_LOOP)
		{
			if (psmp->uFlags & CHN_PINGPONGLOOP)
				pChn->dwFlags |= CHN_PINGPONGLOOP;
			else
				pChn->dwFlags &= ~(CHN_PINGPONGLOOP|CHN_PINGPONGFLAG);
			pChn->dwFlags |= CHN_LOOP;
			pChn->nLength = psmp->nLength;
			pChn->nLoopStart = psmp->nLoopStart;
			pChn->nLoopEnd = psmp->nLoopEnd;
			if (pChn->nLength > pChn->nLoopEnd) pChn->nLength = pChn->nLoopEnd;
		} else
		{
			pChn->dwFlags &= ~(CHN_LOOP|CHN_PINGPONGLOOP|CHN_PINGPONGFLAG);
			pChn->nLength = psmp->nLength;
		}
	}
	if (pChn->pHeader)
	{
		INSTRUMENTHEADER *penv = pChn->pHeader;
		if (((penv->dwFlags & ENV_VOLLOOP) || (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))) && (penv->nFadeOut))
			pChn->dwFlags |= CHN_NOTEFADE;
	}
}


//////////////////////////////////////////////////////////
// CSoundFile: Global Effects


void CSoundFile::SetSpeed(UINT param)
//-----------------------------------
{
	UINT max = (m_nType == MOD_TYPE_IT) ? 256 : 128;
	// Modplug Tracker and Mod-Plugin don't do this check
#ifndef MODPLUG_TRACKER
#ifndef MODPLUG_FASTSOUNDLIB
	// Big Hack!!!
	if ((!param) || (param >= 0x80) || ((m_nType & (MOD_TYPE_MOD|MOD_TYPE_XM|MOD_TYPE_MT2)) && (param >= 0x1E)))
	{
		if (IsSongFinished(m_nCurrentPattern, m_nRow+1))
		{
			GlobalFadeSong(1000);
		}
	}
#endif // MODPLUG_FASTSOUNDLIB
#endif // MODPLUG_TRACKER
	if ((m_nType & MOD_TYPE_S3M) && (param > 0x80)) param -= 0x80;
	if ((param) && (param <= max)) m_nMusicSpeed = param;
}


void CSoundFile::SetTempo(UINT param)
//-----------------------------------
{
	if (param < 0x20)
	{
		// Tempo Slide
		if ((param & 0xF0) == 0x10)
		{
			m_nMusicTempo += (param & 0x0F) * 2;
			if (m_nMusicTempo > 255) m_nMusicTempo = 255;
		} else
		{
			m_nMusicTempo -= (param & 0x0F) * 2;
			if ((LONG)m_nMusicTempo < 32) m_nMusicTempo = 32;
		}
	} else
	{
		m_nMusicTempo = param;
	}
}


int CSoundFile::PatternLoop(MODCHANNEL *pChn, UINT param)
//-------------------------------------------------------
{
	if (param)
	{
		if (pChn->nPatternLoopCount)
		{
			pChn->nPatternLoopCount--;
			if (!pChn->nPatternLoopCount) return -1;
		} else
		{
			MODCHANNEL *p = Chn;
			for (UINT i=0; i<m_nChannels; i++, p++) if (p != pChn)
			{
				// Loop already done
				if (p->nPatternLoopCount) return -1;
			}
			pChn->nPatternLoopCount = param;
		}
		return pChn->nPatternLoop;
	} else
	{
		pChn->nPatternLoop = m_nRow;
		if (m_nType & MOD_TYPE_XM) m_nNextStartRow = m_nRow;
	}
	return -1;
}


void CSoundFile::GlobalVolSlide(UINT param)
//-----------------------------------------
{
	LONG nGlbSlide = 0;
	if (param) m_nOldGlbVolSlide = param; else param = m_nOldGlbVolSlide;
	if (((param & 0x0F) == 0x0F) && (param & 0xF0))
	{
		if (m_dwSongFlags & SONG_FIRSTTICK) nGlbSlide = (param >> 4) * 2;
	} else
	if (((param & 0xF0) == 0xF0) && (param & 0x0F))
	{
		if (m_dwSongFlags & SONG_FIRSTTICK) nGlbSlide = - (int)((param & 0x0F) * 2);
	} else
	{
		if (!(m_dwSongFlags & SONG_FIRSTTICK))
		{
			if (param & 0xF0) nGlbSlide = (int)((param & 0xF0) >> 4) * 2;
			else nGlbSlide = -(int)((param & 0x0F) * 2);
		}
	}
	if (nGlbSlide)
	{
		if (m_nType != MOD_TYPE_IT) nGlbSlide *= 2;
		nGlbSlide += m_nGlobalVolume;
		if (nGlbSlide < 0) nGlbSlide = 0;
		if (nGlbSlide > 256) nGlbSlide = 256;
		m_nGlobalVolume = nGlbSlide;
	}
}


DWORD CSoundFile::IsSongFinished(UINT nStartOrder, UINT nStartRow) const
//----------------------------------------------------------------------
{
	UINT nOrd;

	for (nOrd=nStartOrder; nOrd<MAX_ORDERS; nOrd++)
	{
		UINT nPat = Order[nOrd];
		if (nPat != 0xFE)
		{
			MODCOMMAND *p;

			if (nPat >= MAX_PATTERNS) break;
			p = Patterns[nPat];
			if (p)
			{
				UINT len = PatternSize[nPat] * m_nChannels;
				UINT pos = (nOrd == nStartOrder) ? nStartRow : 0;
				pos *= m_nChannels;
				while (pos < len)
				{
					UINT cmd;
					if ((p[pos].note) || (p[pos].volcmd)) return 0;
					cmd = p[pos].command;
					if (cmd == CMD_MODCMDEX)
					{
						UINT cmdex = p[pos].param & 0xF0;
						if ((!cmdex) || (cmdex == 0x60) || (cmdex == 0xE0) || (cmdex == 0xF0)) cmd = 0;
					}
					if ((cmd) && (cmd != CMD_SPEED) && (cmd != CMD_TEMPO)) return 0;
					pos++;
				}
			}
		}
	}
	return (nOrd < MAX_ORDERS) ? nOrd : MAX_ORDERS-1;
}


BOOL CSoundFile::IsValidBackwardJump(UINT nStartOrder, UINT nStartRow, UINT nJumpOrder, UINT nJumpRow) const
//----------------------------------------------------------------------------------------------------------
{
	while ((nJumpOrder < MAX_PATTERNS) && (Order[nJumpOrder] == 0xFE)) nJumpOrder++;
	if ((nStartOrder >= MAX_PATTERNS) || (nJumpOrder >= MAX_PATTERNS)) return FALSE;
	// Treat only case with jumps in the same pattern
	if (nJumpOrder > nStartOrder) return TRUE;
	if ((nJumpOrder < nStartOrder) || (nJumpRow >= PatternSize[nStartOrder])
	 || (!Patterns[nStartOrder]) || (nStartRow >= 256) || (nJumpRow >= 256)) return FALSE;
	// See if the pattern is being played backward
	BYTE row_hist[256];
	memset(row_hist, 0, sizeof(row_hist));
	UINT nRows = PatternSize[nStartOrder], row = nJumpRow;
	if (nRows > 256) nRows = 256;
	row_hist[nStartRow] = TRUE;
	while ((row < 256) && (!row_hist[row]))
	{
		if (row >= nRows) return TRUE;
		row_hist[row] = TRUE;
		MODCOMMAND *p = Patterns[nStartOrder] + row * m_nChannels;
		row++;
		int breakrow = -1, posjump = 0;
		for (UINT i=0; i<m_nChannels; i++, p++)
		{
			if (p->command == CMD_POSITIONJUMP)
			{
				if (p->param < nStartOrder) return FALSE;
				if (p->param > nStartOrder) return TRUE;
				posjump = TRUE;
			} else
			if (p->command == CMD_PATTERNBREAK)
			{
				breakrow = p->param;
			}
		}
		if (breakrow >= 0)
		{
			if (!posjump) return TRUE;
			row = breakrow;
		}
		if (row >= nRows) return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////
// Note/Period/Frequency functions

UINT CSoundFile::GetNoteFromPeriod(UINT period) const
//---------------------------------------------------
{
	if (!period) return 0;
	if (m_nType & (MOD_TYPE_MED|MOD_TYPE_MOD|MOD_TYPE_MTM|MOD_TYPE_669|MOD_TYPE_OKT|MOD_TYPE_AMF0))
	{
		period >>= 2;
		for (UINT i=0; i<6*12; i++)
		{
			if (period >= ProTrackerPeriodTable[i])
			{
				if ((period != ProTrackerPeriodTable[i]) && (i))
				{
					UINT p1 = ProTrackerPeriodTable[i-1];
					UINT p2 = ProTrackerPeriodTable[i];
					if (p1 - period < (period - p2)) return i+36;
				}
				return i+1+36;
			}
		}
		return 6*12+36;
	} else
	{
		for (UINT i=1; i<NOTE_MAX; i++)
		{
			LONG n = GetPeriodFromNote(i, 0, 0);
			if ((n > 0) && (n <= (LONG)period)) return i;
		}
		return NOTE_MAX;
	}
}



UINT CSoundFile::GetPeriodFromNote(UINT note, int nFineTune, UINT nC4Speed) const
//-------------------------------------------------------------------------------
{
	if ((!note) || (note > 0xF0)) return 0;
	if (m_nType & (MOD_TYPE_IT|MOD_TYPE_S3M|MOD_TYPE_STM|MOD_TYPE_MDL|MOD_TYPE_ULT|MOD_TYPE_WAV
				|MOD_TYPE_FAR|MOD_TYPE_DMF|MOD_TYPE_PTM|MOD_TYPE_AMS|MOD_TYPE_DBM|MOD_TYPE_AMF|MOD_TYPE_PSM))
	{
		note--;
		if (m_dwSongFlags & SONG_LINEARSLIDES)
		{
			return (FreqS3MTable[note % 12] << 5) >> (note / 12);
		} else
		{
			int divider;
			if (!nC4Speed) nC4Speed = 8363;
			// if C4Speed is large, then up shifting may produce a zero divider
			divider = nC4Speed << (note / 12);
			if (!divider) divider = 1e6;
			return _muldiv(8363, (FreqS3MTable[note % 12] << 5), divider);
		}
	} else
	if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
	{
		if (note < 13) note = 13;
		note -= 13;
		if (m_dwSongFlags & SONG_LINEARSLIDES)
		{
			LONG l = ((NOTE_MAX - note) << 6) - (nFineTune / 2);
			if (l < 1) l = 1;
			return (UINT)l;
		} else
		{
			int finetune = nFineTune;
			UINT rnote = (note % 12) << 3;
			UINT roct = note / 12;
			int rfine = finetune / 16;
			int i = rnote + rfine + 8;
			if (i < 0) i = 0;
			if (i >= 104) i = 103;
			UINT per1 = XMPeriodTable[i];
			if ( finetune < 0 )
			{
				rfine--;
				finetune = -finetune;
			} else rfine++;
			i = rnote+rfine+8;
			if (i < 0) i = 0;
			if (i >= 104) i = 103;
			UINT per2 = XMPeriodTable[i];
			rfine = finetune & 0x0F;
			per1 *= 16-rfine;
			per2 *= rfine;
			return ((per1 + per2) << 1) >> roct;
		}
	} else
	{
		note--;
		nFineTune = XM2MODFineTune(nFineTune);
		if ((nFineTune) || (note < 36) || (note >= 36+6*12))
			return (ProTrackerTunedPeriods[nFineTune*12 + note % 12] << 5) >> (note / 12);
		else
			return (ProTrackerPeriodTable[note-36] << 2);
	}
}


UINT CSoundFile::GetFreqFromPeriod(UINT period, UINT nC4Speed, int nPeriodFrac) const
//-----------------------------------------------------------------------------------
{
	if (!period) return 0;
	if (m_nType & (MOD_TYPE_MED|MOD_TYPE_MOD|MOD_TYPE_MTM|MOD_TYPE_669|MOD_TYPE_OKT|MOD_TYPE_AMF0))
	{
		return (3546895L*4) / period;
	} else
	if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
	{
		if (m_dwSongFlags & SONG_LINEARSLIDES)
			return XMLinearTable[period % 768] >> (period / 768);
		else
			return 8363 * 1712L / period;
	} else
	{
		if (m_dwSongFlags & SONG_LINEARSLIDES)
		{
			if (!nC4Speed) nC4Speed = 8363;
			return _muldiv(nC4Speed, 1712L << 8, (period << 8)+nPeriodFrac);
		} else
		{
			return _muldiv(8363, 1712L << 8, (period << 8)+nPeriodFrac);
		}
	}
}
#endif // MODPLUG_CPP	
