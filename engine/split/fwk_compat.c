//-----------------------------------------------------------------------------
// compat (unix & stdio.h)

#if is(tcc) && is(win32) // add missing symbols
const struct in6_addr in6addr_any = IN6ADDR_ANY_INIT;        /* :: */
const struct in6_addr in6addr_loopback;   /* ::1 */
#endif

#if is(win32)
#include <io.h>
#if is(mingw)
#include <unistd.h>
#endif
#else
#include <unistd.h>
#include <sched.h> // sched_setaffinity(), CPU_ZERO(), CPU_COUNT()
#include <sys/ioctl.h>
#endif

#if is(ems)
//#define unlink(x) ((void)(x), 0)
#endif

#if is(win32)
//#define alloca        _alloca
#define atoi64        _atoi64
#define popen         _popen
#define pclose        _pclose
//#define strncasecmp   _strnicmp
#define mkdir(p,m)    mkdir(p)
#define chdir         ifdef(cl, _chdir, chdir)
#if is(cl) || is(tcc)
#define ftruncate     _chsize_s
#endif
#else // gcc
//#include <alloca.h> // mingw64 does not have it
#include <strings.h> // strncasecmp
#define atoi64        atoll
//#define strstri       strcasestr
//#define strcmpi       strcasecmp
#endif

#if   defined MAX_PATH
#define DIR_MAX MAX_PATH
#elif defined PATH_MAX
#define DIR_MAX PATH_MAX
#else
#define DIR_MAX 260
#endif

#if is(win32) // _MSC_VER and __MINGW64__
#include <stdio.h>
#include <windows.h>
#include <share.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
FILE *fmemopen(void *buf, size_t len, const char *type) {
    int fd = -1;
    char temppath[DIR_MAX - 14], filename[DIR_MAX + 1];
    if( GetTempPathA(sizeof(temppath), temppath) )
    if( GetTempFileNameA(temppath, "fwk_temp", 0, filename) )
    if( !_sopen_s(&fd, filename, _O_CREAT | _O_SHORT_LIVED | _O_TEMPORARY | _O_RDWR | _O_BINARY | _O_NOINHERIT, _SH_DENYRW, _S_IREAD | _S_IWRITE) )
    for( FILE *fp = fd != -1 ? _fdopen(fd, "w+b") : NULL; fp; )
    return fwrite(buf, len, 1, fp), rewind(fp), unlink(filename), fp; // no need to _close. fclose(on the returned FILE*) also _closes the file descriptor.
    return fd != -1 ? _close(fd), NULL : NULL;
}
#endif


#if 0
#if !is(cl)
#define tmpfile file_temp
#endif

#define tmpnam(x) file_tempname()
#endif

#if 0
static
const char *pathfile_from_handle(FILE *fp) {
#if is(win32)
    int fd = fileno(fp);
    HANDLE handle = (HANDLE)_get_osfhandle( fd ); // <io.h>
    DWORD size = GetFinalPathNameByHandleW(handle, NULL, 0, VOLUME_NAME_DOS);
    wchar_t name[DIR_MAX] = L"";
    size = GetFinalPathNameByHandleW(handle, name, size, VOLUME_NAME_DOS);
    name[size] = L'\0';
    return wchar16to8(name + 4); // skip \\?\ header
#else
    // In Linux, you can use readlink on /proc/self/fd/NNN where NNN is the file descriptor
    // In OSX:
    //     #include <sys/syslimits.h>
    //     #include <fcntl.h>
    //     char filePath[DIR_MAX];
    //     if (fcntl(fd, F_GETPATH, filePath) != -1) {
    //         // do something with the file path
    //     }
    return 0;
#endif
}
#endif

// -----------------------------------------------------------------------------
// new C macros

#define cast(T)          ifdef(c, void *, decltype(T))
#define literal(T)       ifdef(c, T, (T))

// -----------------------------------------------------------------------------

void fwk_init();
static void fwk_pre_init();
static void fwk_post_init(float);
