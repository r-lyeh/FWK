// string framework
// - rlyeh, public domain

// string: temporary api (stack)
API char*   tempvl(const char *fmt, va_list);
API char*   tempva(const char *fmt, ...);
#define     va(...) (((&printf) || printf(__VA_ARGS__), tempva(__VA_ARGS__)))  // vs2015 check trick
#define     vac (const char*)va

// string: allocated api (heap). FREE() after use
API char*   strcatf(char **s, const char *buf);
#define     strcatf(s,fmt,...)  strcatf((s), va(fmt, __VA_ARGS__))
#define     stringf(fmt,...)    STRDUP(va(fmt, __VA_ARGS__)) // (strcatf)(0, va(fmt, __VA_ARGS__))


#if is(cl) || (is(tcc) && is(win32))
#if!is(cl)
char* strtok_s(char* str,const char* delimiters,char** context); // tcc misses this in <string.h>
#endif
#define strtok_r strtok_s
#endif

#if 1
#define each_substring(str, delims, keyname) \
    ( char *str_ = (char*)(str); str_; str_ = 0 ) \
    for( int len_ = strlen(str_) + 1, heap_ = len_ < 1024; len_ > 1; len_ = 0 ) \
    for( char *ptr_ = (heap_ ? REALLOC(0, len_) : ALLOCA(len_)), *cpy_ = (snprintf(ptr_, len_, "%s", str_), ptr_); ptr_; (heap_ ? REALLOC(ptr_, 0) : 0), ptr_ = 0 ) \
    for( char *next_token = 0, *keyname = strtok_r(cpy_, delims, &next_token); keyname; keyname = strtok_r(NULL, delims, &next_token) )
#else
#define each_substring(str, delims, keyname) \
    ( char** tokens_ = strsplit((str), (delims)), *keyname = 0; tokens_; tokens_ = 0) \
    for( int i_ = 0, end_ = array_count(tokens_); i_ < (keyname = tokens_[i_], end_); ++i_ )
#endif

// utils

API int          strmatch(const char *s, const char *wildcard);
API int          strmatchi(const char *s, const char *wildcard);

API int          strcmp_qsort(const void *a, const void *b);
API int          strcmpi_qsort(const void *a, const void *b);

API bool         strbeg(const char *src, const char *sub); // returns true if both strings match at beginning. case sensitive
API bool         strend(const char *src, const char *sub); // returns true if both strings match at end. case sensitive

API bool         strbegi(const char *src, const char *sub);  // returns true if both strings match at beginning. case insensitive
API bool         strendi(const char *src, const char *sub);  // returns true if both strings match at end. case insensitive
API const char * strstri(const char *src, const char *sub);  // returns find first substring in string. case insensitive.
#define          strcmpi  ifdef(cl, _stricmp, strcasecmp)

API char *       strupper(const char *str);
API char *       strlower(const char *str);

API char *       strrepl(char **copy, const char *target, const char *replace); // replace any 'target' as 'repl' in 'copy'. 'copy' may change (heap). returns 'copy'
API char *       strswap(char *copy, const char *target, const char *replace);  // replaced inline only if repl is shorter than target. no allocations.
API char *       strcut(char *copy, const char *target);                        // remove any 'target' in 'copy'. returns 'copy'

API const char * strlerp(unsigned numpairs, const char **pairs, const char *str); // using key-value pairs, null-terminated

#ifndef __APPLE__ // BSD provides these
API size_t       strlcat(char *dst, const char *src, size_t dstcap); // concat 2 strings safely. always NUL terminates. may truncate.
API size_t       strlcpy(char *dst, const char *src, size_t dstcap); // copy 2 strings safely. always NUL terminates. truncates if retval>=dstcap
#endif

/// split `string` after any of `delimiters` character is found.
/// returns temporary array of split strings. see: strjoin
/// > array(char*) tokens = strsplit("hello! world!", " !"); // [0]="hello",[1]="world",
API array(char*)    strsplit(const char *string, const char *delimiters);

/// concatenate all elements within `list`, with `separator` string in between.
/// returns: temporary joint string. see: strsplit
/// > array(char*) tokens = strsplit("hello! world!", " !"); // [0]="hello",[1]="world",
/// > char *joint = strjoin(tokens, "+"); // joint="hello+world"
API char*           strjoin(array(char*) list, const char *separator);

API char*           string8(const wchar_t *str);  /// convert from wchar16(win) to utf8/ascii
API array(uint32_t) string32( const char *utf8 ); /// convert from utf8 to utf32

API const char*     codepoint_to_utf8(unsigned cp);

// -----------------------------------------------------------------------------
// ## string interning (quarks)
// - rlyeh, public domain.

API unsigned    intern( const char *string );
API const char *quark( unsigned key );

typedef struct quarks_db {
    array(char) blob;
    array(vec2i) entries;
} quarks_db;

API unsigned    quark_intern( quarks_db*, const char *string );
API const char *quark_string( quarks_db*, unsigned key );

// -----------------------------------------------------------------------------
// ## localization kit (I18N, L10N)

API bool  kit_load( const char *filename ); // load translations file (xlsx)
API bool  kit_merge( const char *filename ); // merge translations file into existing context
API void  kit_insert( const char *id, const char *translation ); // insert single translation unit
API void  kit_clear(); // delete all translations

API void  kit_set( const char *variable, const char *value ); // set context variable
API void  kit_reset(); // reset all variables in context
API void  kit_dump_state( FILE *fp ); // debug

API char* kit_translate2( const char *id, const char *langcode_iso639_1 ); // perform a translation given explicit locale

API void  kit_locale( const char *langcode_iso639_1 ); // set current locale: enUS, ptBR, esES, ...
API char* kit_translate( const char *id ); // perform a translation, given current locale
