#include <stdarg.h>

char* tempvl(const char *fmt, va_list vl) {
    va_list copy;
    va_copy(copy, vl);
    int sz = /*stbsp_*/vsnprintf( 0, 0, fmt, copy ) + 1;
    va_end(copy);

    int reqlen = sz;
#if 0
    enum { STACK_ALLOC = 16384 };
    static __thread char buf[STACK_ALLOC];
#else
    enum { STACK_ALLOC = 128*1024 };
    static __thread char *buf = 0; if(!buf) buf = REALLOC(0, STACK_ALLOC); // @leak
#endif
    static __thread int cur = 0, len = STACK_ALLOC - 1; //printf("string stack %d/%d\n", cur, STACK_ALLOC);

    assert(reqlen < STACK_ALLOC && "no stack enough, increase STACK_ALLOC variable above");
    char* ptr = buf + (cur *= (cur+reqlen) < len, (cur += reqlen) - reqlen);

    /*stbsp_*/vsnprintf( ptr, sz, fmt, vl );
    return (char *)ptr;
}
char* tempva(const char *fmt, ...) {
    va_list vl;
    va_start(vl, fmt);
    char *s = tempvl(fmt, vl);
    va_end(vl);
    return s;
}

char* (strcatf)(char **src_, const char *buf) {
    char *src = *src_;
        if(!buf) return src;
        // if(!buf[0]) return src;
        int srclen = (src ? strlen(src) : 0), buflen = strlen(buf);
        src = (char*)REALLOC(src, srclen + buflen + 1 );
        memcpy(src + srclen, buf, buflen + 1 );
    *src_ = src;
    return src;
}

// -----------------------------------------------------------------------------
// string utils

int strmatch(const char *s, const char *wildcard) {
    // returns true if wildcard matches
    if( *wildcard=='\0' ) return !*s;
    if( *wildcard=='*' )  return strmatch(s, wildcard+1) || (*s && strmatch(s+1, wildcard));
    if( *wildcard=='?' )  return *s && (*s != '.') && strmatch(s+1, wildcard+1);
    return (*s == *wildcard) && strmatch(s+1, wildcard+1);
}
int strmatchi(const char *s, const char *wildcard) {
    return strmatch(strlower(s), strlower(wildcard));
}

int strcmp_qsort(const void *a, const void *b) {
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcmp(*ia, *ib);
}
int strcmpi_qsort(const void *a, const void *b) {
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcmpi(*ia, *ib);
}

bool strbeg(const char *a, const char *b) { // returns true if both strings match at beginning. case sensitive
    return strncmp(a, b, strlen(b)) ? false : true; // strstr(a,b) == a
}
bool strend(const char *a, const char *b) { // returns true if both strings match at end. case sensitive
    int la = strlen(a), lb = strlen(b);
    if( la < lb ) return false;
    return strncmp(a + la - lb, b, lb) ? false : true;
}
/*
int main() {
    printf("strbeg(abc abc) = %d\n", strbeg("abc", "abc"));
    printf("strbeg(abc abcd) = %d\n", strbeg("abc", "abcd"));
    printf("strbeg(abcd abc) = %d\n", strbeg("abcd", "abc"));
    printf("strbeg(abc (empty)) = %d\n", strbeg("abc", ""));
    printf("strbeg((empty) abc) = %d\n", strbeg("", "abc"));
    printf("strbeg(123 abcd) = %d\n", strbeg("123", "abcd"));
    printf("strbeg(abcd 123) = %d\n", strbeg("abcd", "123"));
    puts("---");
    printf("strend(abc abc) = %d\n", strend("abc", "abc"));
    printf("strend(abc 0abc) = %d\n", strend("abc", "0abc"));
    printf("strend(abcd bcd) = %d\n", strend("abcd", "bcd"));
    printf("strend(abc (empty)) = %d\n", strend("abc", ""));
    printf("strend((empty) abc) = %d\n", strend("", "abc"));
    printf("strend(123 abcd) = %d\n", strend("123", "abcd"));
    printf("strend(abcd 123) = %d\n", strend("abcd", "123"));
}
*/

bool strbegi(const char *a, const char *b) { // returns true if both strings match at beginning. case insensitive
    int la = strlen(a), lb = strlen(b);
    if( la < lb ) return 0;
    if( lb == 0 ) return 1;
    int len = la < lb ? la : lb;
    for( int i = 0; i < len; ++i ) {
        if( tolower(a[i]) != tolower(b[i]) ) {
            return false;
        }
    }
    return true;
}
bool strendi(const char *src, const char *sub) { // returns true if both strings match at end. case insensitive
    int srclen = strlen(src);
    int sublen = strlen(sub);
    if( sublen > srclen ) return 0;
    return !strcmpi(src + srclen - sublen, sub);
}


// Find substring in string, case insensitive. Alias for strcasestr()
// Returns first char of coincidence, or NULL.
const char *strstri( const char *src, const char *sub ){
    while( *src++ ) {
        for( const char *s = src-1, *f = sub, *c = s; ; ++f, ++c) {
            if(!*f) return s;
            if(!*c) return NULL;
            if(tolower(*c) != tolower(*f)) break;
        }
    }
    return NULL;
}

char *strupper(const char *str) {
    char *s = va("%s", str), *bak = s;
    while(*s++) s[-1] = toupper(s[-1]);
    return bak;
}
char *strlower(const char *str) {
    char *s = va("%s", str), *bak = s;
    while(*s++) s[-1] = tolower(s[-1]);
    return bak;
}

#ifndef __APPLE__ // BSD provides these
// Safely concatenate two strings. Always NUL terminates (unless dstcap == 0).
// Returns length of operation; if retval >= dstcap, truncation occurred.
size_t strlcat(char *dst, const char *src, size_t dstcap) {
    int dl = strlen(dst), sl = strlen(src);
    if( dstcap ) snprintf(dst + dl, dstcap - dl, "%s", src);
    return dl + sl;
}
// Safely copy two strings. Always NUL terminates (unless dstcap == 0).
// Copy src to string dst of size dstcap. Copies at most dstcap-1 characters.
// Returns length of input; if retval >= dstcap, truncation occurred.
size_t strlcpy(char *dst, const char *src, size_t dstcap) {
    int sl = strlen(src);
    if( dstcap ) snprintf(dst, dstcap, "%*s", sl, src);
    return sl;// count does not include NUL
}
#endif

char *string8(const wchar_t *str) { // from wchar16(win) to utf8/ascii
    int i = 0, n = wcslen(str) * 6 - 1;
    static __thread char error[1] = {0}, buffer[2048]; assert( n < 2048 );
    while( *str ) {
       if (*str < 0x80) {
          if (i+1 > n) return error;
          buffer[i++] = (char) *str++;
       } else if (*str < 0x800) {
          if (i+2 > n) return error;
          buffer[i++] = 0xc0 + (*str >> 6);
          buffer[i++] = 0x80 + (*str & 0x3f);
          str += 1;
       } else if (*str >= 0xd800 && *str < 0xdc00) {
          uint32_t c;
          if (i+4 > n) return error;
          c = ((str[0] - 0xd800) << 10) + ((str[1]) - 0xdc00) + 0x10000;
          buffer[i++] = 0xf0 + (c >> 18);
          buffer[i++] = 0x80 + ((c >> 12) & 0x3f);
          buffer[i++] = 0x80 + ((c >>  6) & 0x3f);
          buffer[i++] = 0x80 + ((c      ) & 0x3f);
          str += 2;
       } else if (*str >= 0xdc00 && *str < 0xe000) {
          return error;
       } else {
          if (i+3 > n) return error;
          buffer[i++] = 0xe0 + (*str >> 12);
          buffer[i++] = 0x80 + ((*str >> 6) & 0x3f);
          buffer[i++] = 0x80 + ((*str     ) & 0x3f);
          str += 1;
       }
    }
    buffer[i] = 0;
    return va("%s", buffer);
}

char *strrepl(char **string, const char *target, const char *replace) { // may reallocate input string if needed
    //if new text is shorter than old one,then no need to heap, replace inline
    int rlen = strlen(replace), tlen = strlen(target), diff = tlen - rlen;
    if( diff >= 0 ) return strswap(*string, target, replace);

    char *buf = 0, *aux = *string;
    for( int tgtlen = tlen; tgtlen && aux[0]; ) {
        char *found = strstr(aux, target);
        if( found ) {
            strcatf(&buf, "%.*s%s", (int)(found - aux), aux, replace);
            aux += (found - aux) + tgtlen;
        } else {
            strcatf(&buf, "%s", aux);
            break;
        }
    }
    if( buf ) {
        // strcpy(*string, buf);
        char *s = *string;
        if(s) s[0] = 0;
        strcatf(&s, "%s", buf);
        *string = s;

        FREE( buf );
    }
    return *string;
}
char *strswap(char *copy, const char *target, const char *replace) { // replaced only if new text is shorter than old one
    int rlen = strlen(replace), diff = strlen(target) - rlen;
    if( diff >= 0 ) {
        for( char *s = copy, *e = s + strlen(copy); /*s < e &&*/ 0 != (s = strstr(s, target)); ) {
            if( rlen ) s = (char*)memcpy( s, replace, rlen ) + rlen;
            if( diff ) memmove( s, s + diff, (e - (s + diff)) + 1 );
        }
    }
    return copy;
}
char *strcut(char *copy, const char *target) {
    return strswap(copy, target, "");
}

const char *strlerp(unsigned numpairs, const char **pairs, const char *str) { // using key-value pairs, null-terminated
    if( !pairs[0] ) {
        return str;
    }
    // find & replace all tokens; @fixme: optimize me
    char *buf = REALLOC(0, 128*1024); strcpy(buf, str);
    for( unsigned i = 0; i < numpairs; ++i ) {
        const char *token = pairs[i*2+0];
        const char *repl = pairs[i*2+1];
        while(strstr(buf, token)) {
            strrepl(&buf, token, repl);
        }
    }
    char *ret = va("%s", buf);
    FREE(buf);
    return ret;
}

array(char*) strsplit(const char *str, const char *separators) {
    static __thread int slot = 0;
    static __thread char *buf[16] = {0};
    static __thread array(char*) list[16] = {0};

    slot = (slot+1) % 16;
    array_resize(list[slot], 0);
    *(buf[slot] = REALLOC(buf[slot], strlen(str)*2+1)) = '\0'; // *2 to backup pathological case where input str is only separators && include == 1

    for(char *dst = buf[slot]; str && *str; ) {
        // count literal run && terminators
        int run = strcspn(str, separators);
        int end = strspn(str + run, separators);

        // append literal run
        if( run ) {
            array_push(list[slot], dst);
            memmove(dst,str,run); dst[run] = '\0'; //strncpy(dst, str, run)
            dst += run + 1;
        }
#if defined SPLIT_INCLUDE_SEPARATORS && SPLIT_INCLUDE_SEPARATORS
        // mode: append all separators: "1++2" -> "1" "+" "+" "2"
        for( int i = 0; i < end; ++i ) {
            array_push(list[slot], dst);
            dst[0] = str[ run + i ];
            dst[1] = '\0';
            dst += 2;
        }
#endif

        // skip both
        str += run + end;
    }

    return list[slot];
}
char* strjoin(array(char*) list, const char *separator) {
    static __thread int slot = 0;
    static __thread char* mems[16] = {0};

    slot = (slot+1) % 16;

    int num_list = array_count(list);
    int len = 0, inc = 0, seplen = strlen(separator);
    for( int i = 0; (num_list > 0 ? i < num_list : !!list[i]); ++i ) {
        len += strlen(list[i]) + inc;
        inc = seplen;
    }

    mems[slot] = REALLOC(mems[slot], len+1);
    char *p = mems[slot]; *p = 0;
    const char *sep = "";
    for( int i = 0; (num_list > 0 ? i < num_list : !!list[i]); ++i ) {
        p += sprintf(p, "%s%s", sep, list[i]);
        sep = separator;
    }

    return mems[slot];
}

static
const char *extract_utf32(const char *s, uint32_t *out) {
    /**/ if( (s[0] & 0x80) == 0x00 ) return *out = (s[0]), s + 1;
    else if( (s[0] & 0xe0) == 0xc0 ) return *out = (s[0] & 31) <<  6 | (s[1] & 63), s + 2;
    else if( (s[0] & 0xf0) == 0xe0 ) return *out = (s[0] & 15) << 12 | (s[1] & 63) <<  6 | (s[2] & 63), s + 3;
    else if( (s[0] & 0xf8) != 0xf0 ) return *out = (s[0] &  7) << 18 | (s[1] & 63) << 12 | (s[2] & 63) << 8 | (s[3] & 63), s + 4;
    return *out = 0, s + 0;
}
array(uint32_t) string32( const char *utf8 ) {
    static __thread int slot = 0; slot = (slot+1) % 16;
    static __thread array(uint32_t) out[16] = {0}; array_resize(out[slot], 0);

    //int worstlen = strlen(utf8) + 1; array_reserve(out, worstlen);
    while( *utf8 ) {
        uint32_t unicode = 0;
        utf8 = extract_utf32( utf8, &unicode );
        array_push(out[slot], unicode);
    }
    return out[slot];
}

