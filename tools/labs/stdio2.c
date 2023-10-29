#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#if 1
#define  FWK_IMPLEMENTATION
#include "/prj/fwk/engine/joint/fwk.h"
#define vl tempvl
#define test2 test2_
#else
#define vl(fmt, list) fmt // @fixme

static unsigned array_c;
#define array(t) t*
#define array_resize(arr, c) ( (arr) = vrealloc((arr), (c) * sizeof(0[arr])) )
#define array_push(arr, v) ( array_c = array_count(arr), array_c[(arr) = vrealloc((arr), (array_c + 1) * sizeof(0[arr]))] = (v) )
#define array_pop(arr) ( (arr) ? (arr) = vrealloc((arr), (array_count(arr)-1) * sizeof(0[arr])) : (0) )
#define array_count(arr) (int)( (arr) ? vlen(arr) / sizeof(0[arr]) : sizeof(0[arr]) - sizeof(0[arr]) )
#define array_free(arr) ( (arr) ? (vrealloc(arr, 0), 1) : 0 )

void* vrealloc( void* p, size_t sz ) {
    if( !sz ) {
        if( p ) {
            size_t *ret = (size_t*)p - 2;
            ret[0] = 0;
            ret[1] = 0;
            realloc( ret, 0 );
        }
        return 0;
    } else {
        size_t *ret;
        if( !p ) {
            ret = (size_t*)realloc( 0, sizeof(size_t) * 2 + sz );
            ret[0] = sz;
            ret[1] = 0;
        } else {
            ret = (size_t*)p - 2;
            size_t osz = ret[0];
            size_t ocp = ret[1];
            if( sz <= (osz + ocp) ) {
                ret[0] = sz;
                ret[1] = ocp - (sz - osz);
            } else {
                ret = (size_t*)realloc( ret, sizeof(size_t) * 2 + sz * 1.75 );
                ret[0] = sz;
                ret[1] = (size_t)(sz * 1.75) - sz;
            }
        }
        return &ret[2];
    }
}
size_t vlen( void* p ) {
    return p ? 0[ (size_t*)p - 2 ] : 0;
}
#endif




typedef struct FILE2 {
    FILE *fp;
    int64_t pos;
    int errno_;
    array(char) arr;
} FILE2;


void *tmpfile2(void) {
    FILE2 *mem = (FILE2*)calloc(1, sizeof(FILE2));
    mem->fp = tmpfile();
    return mem;
}
void *fopen2(void *buf, const char *mode) {
    FILE2 *mem = (FILE2*)calloc(1, sizeof(FILE2));

    if( mode[0] != 'm' && buf ) {
        mem->fp = fopen((const char *)buf, mode);
    }

    return (FILE*)mem;
}
int fclose2(void* handler) {
    FILE2 *mem = (FILE2*)handler;
    if( mem->arr ) {
        array_free(mem->arr);
    }
    if( mem->fp ) {
        fclose(mem->fp);
    }
    memset(mem, 0, sizeof(FILE2));
    free(mem);
    return 0;
}
// r/w
int fwrite2(const void *buf, int size, int count, void *handler) {
    FILE2 *mem = handler;

    if( mem->fp ) return fwrite(buf, size, count, mem->fp);

    size *= count;
    size_t available = array_count(mem->arr) - mem->pos;

    if( size > available ) {
        if( 1 ) { // if growthable
            array_resize(mem->arr, size - available);
        } else {
            size = available;
        }
    }
    memcpy(mem->arr, buf, size);
    mem->pos += size;

    return count;
}
int fread2(void *buf, int size, int count, void *handler) {
    FILE2 *mem = handler;
    if( mem->fp ) return fread(buf, size, count, mem->fp);

    size_t available = array_count(mem->arr) - mem->pos;
    size_t total = size * count;

    while( (total/size) > count ) {
        total -= size;
    }
    if( total > 0 ) {
        memcpy(buf, mem->arr, total);
        mem->pos += total;
        return total / size;
    }
    return 0;
}
// cursor
int64_t ftello2(void *handler) {
    FILE2 *mem = handler;
    if( mem->fp ) return ftell(mem->fp);
    return mem->pos;
}
int64_t fseeko2(void *handler, int64_t offset, int whence) {
    FILE2 *mem = handler;
    if( mem->fp ) return fseek(mem->fp, offset, whence);
    /**/ if( whence == SEEK_SET ) mem->pos = offset;
    else if( whence == SEEK_CUR ) mem->pos += offset;
    else if( whence == SEEK_END ) mem->pos = array_count(mem->arr) + offset; // - 1 - offset;
    else return -1;
    if( mem->pos >= array_count(mem->arr) ) {
        return -1;
    }
    return mem->pos; // (fpos_t)mem->pos;
}
// buffering
void setbuf2(void *handler, char *buffer) {
    FILE2 *mem = handler;
    if( mem->fp ) setbuf(mem->fp, buffer);
}
int setvbuf2(void *handler, char *buffer, int mode, size_t size) {
    FILE2 *mem = handler;
    if( mem->fp ) return setvbuf2(mem->fp, buffer, mode, size);
    return 0;
}
int fflush2(void *handler) {
    FILE2 *mem = handler;
    if( mem->fp ) return fflush(mem->fp);
    return 0;
}
int ungetc2(int ch, void *handler) {
    FILE2 *mem = handler;
    if( mem->fp ) return ungetc(ch, mem->fp);
    #if 0
    // todo:

    pushes the uint8_t character onto the specified stream so that
    it's available for the next read operation.

    #endif
    return -1;
}
// internal
array(char)* fdata(void *handler) {
    FILE2 *mem = handler;
    if( mem->fp ) return NULL;
    return &mem->arr;
}



#define FILE   void
#define tmpfile tmpfile2
#define fopen  fopen2
#define   ftello ftello2
#define     ftell (int)ftello2
#define       fgetpos fgetpos2
#define   fseeko fseeko2
#define     fseek (int)fseeko2
#define       fsetpos fsetpos2
#define       rewind rewind2
#define   fread fread2
#define     fgets  fgets2
#define     fgetc fgetc2
#define       getc getc2
#define   fwrite fwrite2
#define     fputs fputs2
#define     fputc fputc2
#define       putc putc2
#define   ferror ferror2
#define     feof feof2
#define     clearerr clearerr2
#define   vfprintf vfprintf2 // @fixme: return type
#define     fprintf fprintf2
#define   setbuf setbuf2
#define     setvbuf setvbuf2
#define       fflush fflush2
#define       ungetc ungetc2
#define fclose fclose2

// not doing:
#define freopen(filename,mode,handler) freopen(filename,mode, ((FILE2*)(handler))->fp )
#define freopen(filename,mode,handler) freopen(filename,mode, ((FILE2*)(handler))->fp )
#define fscanf(handler,format,...) fscanf(((FILE2*)(handler))->fp, format, __VA_ARGS__)


// format
// int fscanf(FILE *fp, const char *format, ...);
int vfprintf(FILE *fp, const char *format, va_list args) { // @fixme: return type
    const char *str = vl(format, args);
    return fwrite(str, strlen(str), 1, fp) == 1;
}
int fprintf(FILE *fp, const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    int rc = vfprintf(fp, fmt, va);
    va_end(va);
    return rc;
}

// error handling
int   feof(FILE *fp) { return ((FILE2*)fp)->errno_ == EOF; }
int   ferror(FILE *fp) { return ((FILE2*)fp)->errno_; }
void  clearerr(FILE *fp) { ((FILE2*)fp)->errno_ = 0; }

// aliases
int fgetpos(FILE *fp, fpos_t *pos) { return (*pos = ftell(fp)) != -1L; }
int fsetpos(FILE *fp, const fpos_t *pos) { return fseek(fp, *pos, SEEK_SET); }
void rewind(FILE *fp) { fseek(fp, 0L, SEEK_SET); }

// aliases
int   fgetc(FILE *fp) { int ch = EOF; return fread(&ch, sizeof(char), 1, fp) != 1 ? EOF : ch; }
int   fputc(int ch, FILE *fp) { return fwrite(&ch, sizeof(char), 1, fp) == 1; }
int   getc(FILE *fp) { return fgetc(fp); }
int   putc(int ch, FILE *fp) { return fputc(ch, fp); }
int   fputs(const char *str, FILE *fp) { return fwrite(str, strlen(str), 1, fp) == 1 && fwrite("\n",1,1,fp) == 1; }
char* fgets(char *str, int n, FILE *fp) {
    char *cpy = str;
    int ch = EOF;
    while( n && (ch = fgetc(fp)) != EOF && !strchr("\r\n", ch) ) *str++ = ch, --n;
    while( n && (ch = fgetc(fp)) != EOF &&  strchr("\r\n", ch) ) *str++ = ch, --n;
    return ch == EOF ? NULL : cpy;
}


#include <assert.h>

void test1() {
    char buf[256] = {0};
    FILE *fp = fopen(__FILE__,"rb");
    assert(fp);
    assert(fgets(buf, sizeof(buf), fp));
    assert(~puts(buf));
    assert(0 == fclose(fp));
    assert(~puts("Ok"));
}

void test2() {
    const char *buf = "hello world";
    FILE *fp = fopen(NULL, "rb");
    assert(fwrite(buf, strlen(buf), 1, fp));
    puts(*fdata(fp));
    assert(0 == fclose(fp));
    assert(~puts("Ok"));
}

void test3() {
    FILE *fp = fopen(NULL, "rb");
    assert(fprintf(fp, "hello %s! %d\n", "world", 123));
    puts(*fdata(fp));
    assert(0 == fclose(fp));
    assert(~puts("Ok"));
}

int main() {
    test1();
    test2();
    test3();
}

