// -----------------------------------------------------------------------------
// file

#if 0 // ifdef _WIN32
#include <winsock2.h>
#if is(tcc)
    #define CP_UTF8 65001
    int WINAPI MultiByteToWideChar();
    int WINAPI WideCharToMultiByte();
#endif
// widen() ? string1252() ? string16() ? stringw() ?
wchar_t *widen(const char *utf8) { // wide strings (win only)
    int chars = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
    char *buf = va("%.*s", (int)(chars * sizeof(wchar_t)), "");
    return MultiByteToWideChar(CP_UTF8, 0, utf8, -1, (void*)buf, chars), (wchar_t *)buf;
}
#define open8(path,mode)  ifdef(win, _wopen(widen(path))               ,  open(path, mode) )
#define fopen8(path,mode) ifdef(win, _wfopen(widen(path),widen(mode))  ,  fopen(path,mode) )
#define remove8(path)     ifdef(win, _wremove(widen(path))             ,  remove(path)     )
#define rename8(path)     ifdef(win, _wrename(widen(path))             ,  rename(path)     )
#define stat8(path,st)    ifdef(win, _wstat(widen(path),st)            ,  stat(path,st)    ) // _stati64()
#define stat8_t           ifdef(win, _stat                             ,  stat_t           ) // struct _stati64
#endif

char *file_name(const char *pathfile) {
    char *s = strrchr(pathfile, '/'), *t = strrchr(pathfile, '\\');
    return va("%s", s > t ? s+1 : t ? t+1 : pathfile);
}
char *file_base(const char *pathfile) {
    char *s = file_name(pathfile);
    char *e = file_ext(pathfile);
    return s[ strlen(s) - strlen(e) ] = '\0', s;
}
char *file_pathabs( const char *pathfile ) {
    char *out = va("%*.s", DIR_MAX+1, "");
#if is(win32)
    _fullpath(out, pathfile, DIR_MAX);
#else
    realpath(pathfile, out);
#endif
    return out;
}
char *file_path(const char *pathfile) {
    return va("%.*s", (int)(strlen(pathfile)-strlen(file_name(pathfile))), pathfile);
}
char *file_load(const char *filename, int *len) { // @todo: 32 counters/thread enough?
    static __thread array(char) buffer[32] = {0}, *invalid[1];
    static __thread unsigned i = 0; i = (i+1) % 32;

    FILE *fp = filename[0] ? fopen(filename, "rb") : NULL;
    if( fp ) {
        fseek(fp, 0L, SEEK_END);
        size_t sz = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        array_resize(buffer[i], sz+1);
        sz *= fread(buffer[i],sz,1,fp) == 1;
        buffer[i][sz] = 0;
        if(len) *len = (int)sz;
        fclose(fp);
        return buffer[i]; // @fixme: return 0 on error instead?
    }
    if (len) *len = 0;
    return 0;
}
char *file_read(const char *filename) { // @todo: fix leaks
    return file_load(filename, NULL);
}
bool file_write(const char *name, const void *ptr, int len) {
    bool ok = 0;
    for( FILE *fp = name && ptr && len >= 0 ? fopen(name, "wb") : NULL; fp; fclose(fp), fp = 0) {
        ok = fwrite(ptr, len,1, fp) == 1;
    }
    return ok;
}
bool file_append(const char *name, const void *ptr, int len) {
    bool ok = 0;
    for( FILE *fp = name && ptr && len >= 0 ? fopen(name, "a+b") : NULL; fp; fclose(fp), fp = 0) {
        ok = fwrite(ptr, len,1, fp) == 1;
    }
    return ok;
}
static // not exposed
bool file_stat(const char *fname, struct stat *st) {
    // remove ending slashes. win32+tcc does not like them.
    int l = strlen(fname), m = l;
    while( l && (fname[l-1] == '/' || fname[l-1] == '\\') ) --l;
    fname = l == m ? fname : va("%.*s", l, fname);
    return stat(fname, st) >= 0;
}
uint64_t file_stamp(const char *fname) {
    struct stat st;
    return !file_stat(fname, &st) ? 0ULL : st.st_mtime;
}
uint64_t file_stamp10(const char *fname) {
    time_t mtime = (time_t)file_stamp(fname);
    struct tm *ti = localtime(&mtime);
    return atoi64(va("%04d%02d%02d%02d%02d%02d",ti->tm_year+1900,ti->tm_mon+1,ti->tm_mday,ti->tm_hour,ti->tm_min,ti->tm_sec));
}
uint64_t file_size(const char *fname) {
    struct stat st;
    return !file_stat(fname, &st) ? 0ULL : st.st_size;
}
bool file_directory( const char *pathfile ) {
    struct stat st;
    return !file_stat(pathfile, &st) ? 0 : S_IFDIR == ( st.st_mode & S_IFMT );
}
bool file_exist(const char *fname) {
    struct stat st;
    return !file_stat(fname, &st) ? false : true;
}
char *file_normalize(const char *name) {
    char *copy = va("%s", name), *s = copy, c;
#if is(win32)
    for( int i = 0; copy[i]; ++i ) { if(copy[i] == '/') copy[i] = '\\'; if(copy[i] == '\'') copy[i] = '\"'; }
#else
    for( int i = 0; copy[i]; ++i ) { if(copy[i] == '\\') copy[i] = '/'; if(copy[i] == '\"') copy[i] = '\''; }
#endif
    return copy;
}
#if 0
char *file_normalize(const char *name) {
    char *copy = va("%s", name), *s = copy, c;
    // lowercases+digits+underscores+slashes only. anything else is truncated.
    for( ; *name ; ++name ) {
        /**/ if( *name >= 'a' && *name <= 'z' ) *s++ = *name;
        else if( *name >= 'A' && *name <= 'Z' ) *s++ = *name - 'A' + 'a';
        else if( *name >= '0' && *name <= '9' ) *s++ = *name;
        else if( *name == '/' || *name == '\\') *s++ = '/';
        else if( *name <= ' ' || *name == '.' ) *s++ = '_';
    } *s++ = 0;
    // remove dupe slashes
    for( name = s = copy, c = '/'; *name ; ) {
        while( *name && *name != c ) *s++ = *name++;
        if( *name ) *s++ = c;
        while( *name && *name == c ) name++;
    } *s++ = 0;
    // remove dupe underlines
    for( name = s = copy, c = '_'; *name ; ) {
        while( *name && *name != c ) *s++ = *name++;
        if( *name ) *s++ = c;
        while( *name && *name == c ) name++;
    } *s++ = 0;
    return copy;
}
char *file_normalize_with_folder(const char *name) {
    char *s = file_normalize(name);
    char *slash = strrchr(s, '/'); if(slash) *slash = 0;
    char *penultimate = strrchr(s, '/'); if(slash) *slash = '/';
    return penultimate ? penultimate+1 : /*slash ? slash+1 :*/ s;
}
#endif
char *file_ext(const char *name) {
    char *b = file_name(name), *s = strchr(b, '.'); //strrchr(name, '.');
    return va("%s", s ? s : "" ); // , name );
}

char *file_id(const char *pathfile) {
    char *dir = file_path(pathfile); for(int i=0;dir[i];++i) dir[i]=tolower(dir[i]);
    char *base = file_name(pathfile); for(int i=0;base[i];++i) base[i]=tolower(base[i]);
#if 0 // extensionless, larry.mid and larry.txt will collide, diffuse.tga and diffuse.png will match
char *ext = strchr(base, '.'); if (ext) ext[0] = '\0'; // remove all extensions
#else // extensionless for audio/images only (materials: diffuse.tga and diffuse.png will match)
char *ext = strrchr(base, '.'); //if (ext) ext[0] = '\0'; // remove all extensions
    if(ext) if( strstr(".jpg.png.bmp.tga.hdr"".", ext) || strstr(".ogg.mp3.wav.mod.xm.flac"".", ext) || strstr(".mp4.ogv.avi.mkv.wmv.mpg.mpeg"".", ext) ) {
        ext = strchr(base, '.');
        ext[0] = '\0'; //strcpy(ext, "_xxx");
    }
#endif

// if (!dir[0]) return base;
    char *stem = va("%s/%s", dir, base); // file_name(dir);

    // /path2/path1/file2_file1 -> file1_file2/path1/path2
    int len = 0;
    int ids_count = 0;
    char ids[64][64] = { 0 };
    // split path stems
    for each_substring(stem, "/\\@", key) {
        int tokens_count = 0;
        char* tokens[64] = { 0 };
        // split tokens
        for each_substring(key, "[]()_ ", it) {
            tokens[tokens_count++] = va("%s", it);
        }
        // sort alphabetically
        if( tokens_count > 1 ) qsort(tokens, tokens_count, sizeof(char *), strcmp_qsort);
        // concat sorted token1_token2_...
        char built[256]; *built = 0;
        for( int i = 0; i < tokens_count; ++i ) {
            strlcat(built, "_", 256);
            strlcat(built, tokens[i], 256);
        }
        strncpy( ids[ ids_count ], &built[1], 64 );
        len += strlen( ids[ ids_count++ ] );
    }
    // concat in inverse order: file/path1/path2/...
    char buffer[DIR_MAX]; buffer[0] = 0;
    for( int it = ids_count; --it >= 0; ) {
        strcat(buffer, ids[it]);
        strcat(buffer, "/");
    }
    return va("%s", buffer);
}
array(char*) file_list(const char *pathmasks) {
    static __thread array(char*) list = 0; // @fixme: add 16 slots

    for( int i = 0; i < array_count(list); ++i ) {
        FREE(list[i]);
    }
    array_resize(list, 0);

    for each_substring(pathmasks,";",pathmask) {
        char *cwd = 0, *masks = 0;
            char *slash = strrchr(pathmask, '/');
            if( !slash ) cwd = "./", masks = pathmask;
            else {
                masks = va("%s", slash+1);
                cwd = pathmask, slash[1] = '\0';
            }
            if( !masks[0] ) masks = "*";

        ASSERT(strend(cwd, "/"), "Error: dirs like '%s' must end with slash", cwd);

        int recurse = strstr(cwd, "**") || strstr(masks, "**");
        strswap(cwd, "**", "./");

        dir *d = dir_open(cwd, recurse ? "r" : "");
        if( d ) {
            for( int i = 0; i < dir_count(d); ++i ) {
                if( dir_file(d,i) ) {
                    // dir_name() should return full normalized paths "C:/prj/fwk/demos/art/fx/fxBloom.fs". should exclude system dirs as well
                    char *entry = dir_name(d,i);
                    char *fname = file_name(entry);

                    int allowed = 0;
                    for each_substring(masks,";",mask) {
                        allowed |= strmatch(fname, mask);
                    }
                    if( !allowed ) continue;

                    // if( strstr(fname, "/.") ) continue; // @fixme: still needed? useful?

                    // insert copy
                    char *copy = STRDUP(entry);
                    array_push(list, copy);
                }
            }
            dir_close(d);
        }
    }

    array_sort(list, strcmp);
    return list;
}

bool file_move(const char *src, const char *dst) {
    bool ok = file_exist(src) && !file_exist(dst) && 0 == rename(src, dst);
    return ok;
}
bool file_delete(const char *pathfile) {
    if( file_exist(pathfile) ) {
        for( int i = 0; i < 10; ++i ) {
            bool ok = 0 == unlink(pathfile);
            if( ok ) return true;
            sleep_ms(10);
        }
        return false;
    }
    return true;
}
bool file_copy(const char *src, const char *dst) {
    int ok = 0, BUFSIZE = 1 << 20; // 1 MiB
    static __thread char *buffer = 0; do_once buffer = REALLOC(0, BUFSIZE); // @leak
    for( FILE *in = fopen(src, "rb"); in; fclose(in), in = 0) {
        for( FILE *out = fopen(dst, "wb"); out; fclose(out), out = 0, ok = 1) {
            for( int n; !!(n = fread( buffer, 1, BUFSIZE, in )); ){
                if(fwrite( buffer, 1, n, out ) != n)
                return fclose(in), fclose(out), false;
            }
        }
    }
    return ok;
}

char* file_tempname() {
    static __thread int id;
    return va("%s/fwk-temp.%s.%p.%d", app_temp(), getenv(ifdef(win32, "username", "USER")), &id, rand());
}
FILE *file_temp(void) {
    const char *fname = file_tempname();
    FILE *fp = fopen(fname, "w+b");
    if( fp ) unlink(fname);
    return fp;
}

char *file_counter(const char *name) {
    static __thread char outfile[DIR_MAX], init = 0;
    static __thread map(char*, int) ext_counters;
    if(!init) map_init(ext_counters, less_str, hash_str), init = '\1';

    char *base = va("%s",name), *ext = file_ext(name);
    if(ext && ext[0]) *strstr(base, ext) = '\0';

    int *counter = map_find_or_add(ext_counters, ext, 0);
    while( *counter >= 0 ) {
        *counter = *counter + 1;
        sprintf(outfile, "%s(%03d)%s", base, *counter, ext);
        if( !file_exist(outfile) ) {
            return va("%s", outfile);
        }
    }
    return 0;
}

enum { MD5_HASHLEN = 16 };
enum { SHA1_HASHLEN = 20 };
enum { CRC32_HASHLEN = 4 };

void* file_sha1(const char *file) { // 20bytes
    hash_state hs = {0};
    sha1_init(&hs);
    for( FILE *fp = fopen(file, "rb"); fp; fclose(fp), fp = 0) {
        char buf[8192];
        for( int inlen; (inlen = sizeof(buf) * fread(buf, sizeof(buf), 1, fp)); ) {
            sha1_process(&hs, (const unsigned char *)buf, inlen);
        }
    }
    unsigned char *hash = va("%.*s", SHA1_HASHLEN, "");
    sha1_done(&hs, hash);
    return hash;
}

void* file_md5(const char *file) { // 16bytes
    hash_state hs = {0};
    md5_init(&hs);
    for( FILE *fp = fopen(file, "rb"); fp; fclose(fp), fp = 0) {
        char buf[8192];
        for( int inlen; (inlen = sizeof(buf) * fread(buf, sizeof(buf), 1, fp)); ) {
            md5_process(&hs, (const unsigned char *)buf, inlen);
        }
    }
    unsigned char *hash = va("%.*s", MD5_HASHLEN, "");
    md5_done(&hs, hash);
    return hash;
}

void* file_crc32(const char *file) { // 4bytes
    unsigned crc = 0;
    for( FILE *fp = fopen(file, "rb"); fp; fclose(fp), fp = 0) {
        char buf[8192];
        for( int inlen; (inlen = sizeof(buf) * fread(buf, sizeof(buf), 1, fp)); ) {
            crc = zip__crc32(crc, buf, inlen); // unsigned int stbiw__crc32(unsigned char *buffer, int len)
        }
    }
    unsigned char *hash = va("%.*s", (int)sizeof(crc), "");
    memcpy(hash, &crc, sizeof(crc));
    return hash;
}

#if 0
void* crc32_mem(const void *ptr, int inlen) { // 4bytes
    unsigned hash = 0;
    hash = zip__crc32(hash, ptr, inlen); // unsigned int stbiw__crc32(unsigned char *buffer, int len)
    return hash;
}
void* md5_mem(const void *ptr, int inlen) { // 16bytes
    hash_state hs = {0};
    md5_init(&hs);
    md5_process(&hs, (const unsigned char *)ptr, inlen);
    unsigned char *hash = va("%.*s", MD5_HASHLEN, "");
    md5_done(&hs, hash);
    return hash;
}
void* sha1_mem(const void *ptr, int inlen) { // 20bytes
    hash_state hs = {0};
    sha1_init(&hs);
    sha1_process(&hs, (const unsigned char *)ptr, inlen);
    unsigned char *hash = va("%.*s", SHA1_HASHLEN, "");
    sha1_done(&hs, hash);
    return hash;
}
unsigned crc32_mem(unsigned h, const void *ptr_, unsigned len) {
    // based on public domain code by Karl Malbrain
    const uint8_t *ptr = (const uint8_t *)ptr_;
    if (!ptr) return 0;
    const unsigned tbl[16] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c, 0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c };
    for(h = ~h; len--; ) { uint8_t b = *ptr++; h = (h >> 4) ^ tbl[(h & 15) ^ (b & 15)]; h = (h >> 4) ^ tbl[(h & 15) ^ (b >> 4)]; }
    return ~h;
}
uint64_t crc64_mem(uint64_t h, const void *ptr, uint64_t len) {
    // based on public domain code by Lasse Collin
    // also, use poly64 0xC96C5795D7870F42 for crc64-ecma
    static uint64_t crc64_table[256];
    static uint64_t poly64 = UINT64_C(0x95AC9329AC4BC9B5);
    if( poly64 ) {
        for( int b = 0; b < 256; ++b ) {
            uint64_t r = b;
            for( int i = 0; i < 8; ++i ) {
                r = r & 1 ? (r >> 1) ^ poly64 : r >> 1;
            }
            crc64_table[ b ] = r;
            //printf("%016llx\n", crc64_table[b]);
        }
        poly64 = 0;
    }
    const uint8_t *buf = (const uint8_t *)ptr;
    uint64_t crc = ~h; // ~crc;
    while( len != 0 ) {
        crc = crc64_table[(uint8_t)crc ^ *buf++] ^ (crc >> 8);
        --len;
    }
    return ~crc;
}
// https://en.wikipedia.org/wiki/MurmurHash
static inline uint32_t murmur3_scramble(uint32_t k) {
    return k *= 0xcc9e2d51, k = (k << 15) | (k >> 17), k *= 0x1b873593;
}
uint32_t murmur3_mem(const uint8_t* key, size_t len, uint32_t seed) {
    uint32_t h = seed;
    uint32_t k;
    /* Read in groups of 4. */
    for (size_t i = len >> 2; i; i--) {
        // Here is a source of differing results across endiannesses.
        // A swap here has no effects on hash properties though.
        k = *((uint32_t*)key);
        key += sizeof(uint32_t);
        h ^= murmur3_scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }
    /* Read the rest. */
    k = 0;
    for (size_t i = len & 3; i; i--) {
        k <<= 8;
        k |= key[i - 1];
    }
    // A swap is *not* necessary here because the preceeding loop already
    // places the low bytes in the low places according to whatever endianness
    // we use. Swaps only apply when the memory is copied in a chunk.
    h ^= murmur3_scramble(k);
    /* Finalize. */
    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}
#endif

// -----------------------------------------------------------------------------
// storage (emscripten only)

void storage_mount(const char* folder) {
    #if is(ems)
        emscripten_run_script(va("FS.mkdir('%s'); FS.mount(IDBFS, {}, '%s');", folder, folder));
    #else
        (void)folder;
    #endif
}

void storage_read() {
    #if is(ems)
    EM_ASM(
        /* sync from persisted state into memory */
        FS.syncfs(true, function (err) {
            assert(!err);
        });
    );
    #endif
}

void storage_flush() {
    #if is(ems)
    EM_ASM(
        FS.syncfs(false, function (err) {
            assert(!err);
        });
    );
    #endif
}

// -----------------------------------------------------------------------------
// compressed archives

// return list of files inside zipfile
array(char*) file_zip_list(const char *zipfile) {
    static __thread array(char*) list[16] = {0};
    static __thread int count = 0;

    count = (count+1) % 16;
    array_resize(list[count], 0);

    for( zip *z = zip_open(zipfile, "rb"); z; zip_close(z), z = 0) {
        for( unsigned i = 0; i < zip_count(z); ++i ) {
            array_push( list[count], zip_name(z, i) );
        }
    }

    return list[count];
}

// extract single file content from zipfile
array(char) file_zip_extract(const char *zipfile, const char *filename) {
    static __thread array(char) list[16] = {0};
    static __thread int count = 0;

    array(char) out = list[count = (count+1) % 16];
    array_resize(out, 0);

    for( zip *z = zip_open(zipfile, "rb"); z; zip_close(z), z = 0) {
        int index = zip_find(z, filename); // convert entry to index. returns <0 if not found.
        if( index < 0 ) return zip_close(z), out;

        unsigned outlen = zip_size(z, index);
        unsigned excess = zip_excess(z, index);
        array_resize(out, outlen + 1 + excess);
        unsigned ret = zip_extract_inplace(z, index, out, array_count(out));
        if(ret) { out[outlen] = '\0'; array_resize(out, outlen); } else { array_resize(out, 0); }
    }

    return out;
}

// append single file into zipfile. compress with DEFLATE|6. Other compressors are also supported (try LZMA|5, ULZ|9, LZ4X|3, etc.)
bool file_zip_append(const char *zipfile, const char *filename, int clevel) {
    bool ok = false;
    for( zip *z = zip_open(zipfile, "a+b"); z; zip_close(z), z = 0) {
        for( FILE *fp = fopen(filename, "rb"); fp; fclose(fp), fp = 0) {
            ok = zip_append_file(z, filename, "", fp, clevel);
        }
    }
    return ok;
}

// append mem blob into zipfile. compress with DEFLATE|6. Other compressors are also supported (try LZMA|5, ULZ|9, LZ4X|3, etc.)
// @fixme: implement zip_append_mem() and use that instead
bool file_zip_appendmem(const char *zipfile, const char *entryname, const void *ptr, unsigned len, int clevel) {
    bool ok = false;
    if( ptr )
    for( zip *z = zip_open(zipfile, "a+b"); z; zip_close(z), z = 0) {
        ok = zip_append_mem(z, entryname, "", ptr, len, clevel);
    }
    return ok;
}

// -----------------------------------------------------------------------------
// archives

enum { is_zip, is_tar, is_pak, is_dir };

typedef struct archive_dir {
    char* path;
    union {
        int type;
        int size; // for cache only
    };
    union {
        void *archive;
        void *data; // for cache only
        zip* zip_archive;
        tar* tar_archive;
        pak* pak_archive;
    };
    struct archive_dir *next;
} archive_dir;

static archive_dir *dir_mount;
static archive_dir *dir_cache;

#ifndef MAX_CACHED_FILES    // @todo: should this be MAX_CACHED_SIZE (in MiB) instead?
#define MAX_CACHED_FILES 32 // @todo: should we cache the cooked contents instead? ie, stbi() result instead of file.png?
#endif

struct vfs_entry {
    const char *name;
    const char *id;
    unsigned size;
};
static array(struct vfs_entry) vfs_hints;   // mounted raw assets
static array(struct vfs_entry) vfs_entries; // mounted cooked assets

static bool vfs_mount_hints(const char *path);

void vfs_reload() {
    const char *app = app_name();

    array_resize(vfs_hints, 0); // @leak
    array_resize(vfs_entries, 0); // @leak

    // mount virtual filesystems later (mounting order matters: low -> to -> high priority)
#if defined(EMSCRIPTEN)
    vfs_mount("index.zip");
#else
    // mount fused executables
    vfs_mount(va("%s%s%s", app_path(), app_name(), ifdef(win32, ".exe", "")));
    // mount all zipfiles
    for each_array( file_list("*.zip"), char*, file ) vfs_mount(file);
#endif

    // vfs_resolve() will use these art_folder locations as hints when cook-on-demand is in progress.
    // cook-on-demand will not be able to resolve a virtual pathfile if there are no cooked assets on disk,
    // unless there is a record of what's actually on disk somewhere, and that's where the hints belong to.
    if( COOK_ON_DEMAND )
    for each_substring(ART,",",art_folder) {
        vfs_mount_hints(art_folder);
    }
}



#define ARK1         0x41724B31 // 'ArK1' in le, 0x314B7241 41 72 4B 31 otherwise
#define ARK1_PADDING (512 - 40) // 472
#define ARK_PRINTF(f,...) 0 // printf(f,__VA_ARGS__)
#define ARK_SWAP32(x) (x)
#define ARK_SWAP64(x) (x)
#define ARK_REALLOC   REALLOC
static uint64_t ark_fget64( FILE *in ) { uint64_t v; fread( &v, 8, 1, in ); return ARK_SWAP64(v); }
void ark_list( const char *infile, zip **z ) {
    for( FILE *in = fopen(infile, "rb"); in; fclose(in), in = 0 )
    while(!feof(in)) {
        if( 0 != (ftell(in) % ARK1_PADDING) ) fseek(in, ARK1_PADDING - (ftell(in) % ARK1_PADDING), SEEK_CUR);
        ARK_PRINTF("Reading at #%d\n", (int)ftell(in));
        uint64_t mark = ark_fget64(in);
        if( mark != ARK1 ) continue;
        uint64_t stamp = ark_fget64(in);
        uint64_t datalen = ark_fget64(in);
        uint64_t datahash = ark_fget64(in);
        uint64_t namelen = ark_fget64(in);

        *z = zip_open_handle(in, "rb");
        return;
    }
}



static
bool vfs_mount_(const char *path, array(struct vfs_entry) *entries) {
    const char *path_bak = path;

    zip *z = NULL; tar *t = NULL; pak *p = NULL; dir *d = NULL;
    int is_folder = ('/' == path[strlen(path)-1]);
    if( is_folder ) d = dir_open(path, "rb");
    if( is_folder && !d ) return 0;
    if( !is_folder ) z = zip_open(path, "rb");
    if( !is_folder && !z ) t = tar_open(path, "rb");
    if( !is_folder && !z && !t ) p = pak_open(path, "rb");
    if( !is_folder && !z && !t && !p ) ark_list(path, &z); // last resort. try as .ark
    if( !is_folder && !z && !t && !p ) return 0;

    // normalize input -> "././" to ""
    while (path[0] == '.' && path[1] == '/') path += 2;
    path = STRDUP(path);
    if( z || t || p ) {
    // save local path for archives, so we can subtract those from upcoming requests
    if(strrchr(path,'/')) strrchr(path,'/')[1] = '\0';
    } else if(d) 0[(char*)path] = 0;

    // append to mounted archives
    archive_dir *prev = dir_mount, zero = {0};
    *(dir_mount = REALLOC(0, sizeof(archive_dir))) = zero;
    dir_mount->next = prev;
    dir_mount->path = (char*)path;
    dir_mount->archive = z ? (void*)z : t ? (void*)t : p ? (void*)p : (void*)d;
    dir_mount->type = is_folder ? is_dir : z ? is_zip : t ? is_tar : p ? is_pak : -1;
    ASSERT(dir_mount->type >= 0 && dir_mount->type < 4);

    // append list of files to internal listing
    for( archive_dir *dir = dir_mount; dir ; dir = 0 ) { // for(archive_dir *dir = dir_mount; dir; dir = dir->next) {
        assert(dir->type >= 0 && dir->type < 4);
        unsigned (*fn_count[4])(void*) = {(void*)zip_count, (void*)tar_count, (void*)pak_count, (void*)dir_count};
        char*    (*fn_name[4])(void*, unsigned index) = {(void*)zip_name, (void*)tar_name, (void*)pak_name, (void*)dir_name};
        unsigned (*fn_size[4])(void*, unsigned index) = {(void*)zip_size, (void*)tar_size, (void*)pak_size, (void*)dir_size};

        for( unsigned idx = 0, end = fn_count[dir->type](dir->archive); idx < end; ++idx ) {
            assert(idx < end);
            const char *filename = STRDUP( fn_name[dir->type](dir->archive, idx) );
            const char *fileid = STRDUP( file_id(filename) );
            unsigned filesize = fn_size[dir->type](dir->archive, idx);
            // printf("%u) %s %u [%s]\n", idx, filename, filesize, fileid);
            // append to list
            array_push(*entries, (struct vfs_entry){filename, fileid, filesize});
        }

        PRINTF("Mounted VFS volume '%s' (%u entries)\n", path_bak, fn_count[dir->type](dir->archive) );
    }

    return 1;
}
static
bool vfs_mount_hints(const char *path) {
    return vfs_mount_(path, &vfs_hints);
}
bool vfs_mount(const char *path) {
    return vfs_mount_(path, &vfs_entries);
}
array(char*) vfs_list(const char *masks) {
    static __thread array(char*) list = 0; // @fixme: add 16 slots

    for( int i = 0; i < array_count(list); ++i ) {
        FREE(list[i]);
    }
    array_resize(list, 0);

    for each_substring(masks,";",it) {
        if( COOK_ON_DEMAND ) // edge case: any game using only vfs api + cook-on-demand flag will never find any file
        for each_array(file_list(it), char*, item) {
            // insert copy
            char *copy = STRDUP(item);
            array_push(list, copy);
        }

        it = va("*/%s", it);
        // int recurse = !!strstr(it, "**"); // @fixme: support non-recursive
        for( unsigned i = 0; i < array_count(vfs_entries); ++i ) {
            const char *name = vfs_entries[i].name;
            if( strmatch(name, it) ) {
                // insert copy
                char *copy = STRDUP(name);
                array_push(list, copy);
            }
        }
    }

    // sort alphabetically then remove dupes
    array_sort(list, strcmp);
    array_unique(list, strcmp_qsort);

    return list;
}

static
char *vfs_unpack(const char *pathfile, int *size) { // must FREE() after use
    // @todo: add cache here
    char *data = NULL;
    for(archive_dir *dir = dir_mount; dir && !data; dir = dir->next) {
        if( dir->type == is_dir ) {
#if 0 // sandboxed
            char buf[DIR_MAX];
            snprintf(buf, sizeof(buf), "%s%s", dir->path, pathfile);
            data = file_load(buf, size);
#endif
        } else {
            int      (*fn_find[3])(void *, const char *) = {(void*)zip_find, (void*)tar_find, (void*)pak_find};
            void*    (*fn_unpack[3])(void *, unsigned) = {(void*)zip_extract, (void*)tar_extract, (void*)pak_extract};
            unsigned (*fn_size[3])(void *, unsigned) = {(void*)zip_size, (void*)tar_size, (void*)pak_size};

#if 0
            const char* cleanup = pathfile + strbegi(pathfile, dir->path) * strlen(dir->path);
            while (cleanup[0] == '/') ++cleanup;
#else
            const char *cleanup = pathfile;
#endif

            int index = fn_find[dir->type](dir->archive, cleanup);
            data = fn_unpack[dir->type](dir->archive, index);
            if( size ) *size = fn_size[dir->type](dir->archive, index);
        }
        // printf("%c trying %s in %s ...\n", data ? 'Y':'N', pathfile, dir->path);
    }
    //wait_ms(1000); // <-- simulate slow hdd
    return data;
}

const char *vfs_resolve(const char *pathfile) {
    // we dont resolve absolute paths. they dont belong to the vfs
    // if( pathfile[0] == '/' || pathfile[0] == '\\' || pathfile[1] == ':' ) return pathfile;

    char* id = file_id(pathfile);
    // find best match (vfs_entries first)
    for (int i = array_count(vfs_entries); --i >= 0; ) {
        if (strbegi(vfs_entries[i].id, id) ) {
            return vfs_entries[i].name;
        }
    }
    // find best match (vfs_hints later)
    for (int i = array_count(vfs_hints); --i >= 0; ) {
        if (strbegi(vfs_hints[i].id, id) ) {
            return vfs_hints[i].name;
        }
    }
    return pathfile;
}

char* vfs_load(const char *pathfile, int *size_out) { // @todo: fix leaks, vfs_unpack()
    // @fixme: handle \\?\ absolute path (win)
    if (!pathfile[0]) return file_load(pathfile, size_out);
    while( pathfile[0] == '.' && (pathfile[1] == '/' || pathfile[1] == '\\') ) pathfile += 2;
    // if (pathfile[0] == '/' || pathfile[1] == ':') return file_load(pathfile, size_out); // @fixme: handle current cooked /home/FWK or C:/FWK path cases within zipfiles

if( size_out ) *size_out = 0;
if( strend(pathfile, "/") ) return 0; // it's a dir
static __thread map(char*,int) misses = 0, *init = 0; if(!init) init = misses, map_init(misses, less_str, hash_str);
int *found = map_find_or_add_allocated_key(misses, STRDUP(pathfile), -1); // [-1]non-init,[false]could not cook,[true]cooked
if( found && *found == 0 ) {
    return 0;
}

    //{
    // exclude garbage from material names
    // @todo: exclude double slashs in paths
    char *base = file_name(pathfile); if(strchr(base,'+')) base = strchr(base, '+')+1;
    if(base[0] == '\0') return 0; // it's a dir
    char *folder = file_path(pathfile);
    pathfile = va("%s%s", folder, base);

    // solve virtual path
    pathfile = va("%s", vfs_resolve(pathfile));
    base = file_name(pathfile);
    if(base[0] == '\0') return 0; // it's a dir
    folder = file_path(pathfile);
        // ease folders reading by shortening them: /home/rlyeh/prj/fwk/art/demos/audio/coin.wav -> demos/audio/coin.wav
        // or C:/prj/fwk/engine/art/fonts/B612-BoldItalic.ttf -> fonts/B612-BoldItalic.ttf
        static __thread array(char*) art_paths = 0;
        if(!art_paths) for each_substring(ART,",",stem) array_push(art_paths, STRDUP(stem));
        char* pretty_folder = "";
        if( folder ) for( int i = 0; i < array_count(art_paths); ++i ) {
            if( strbeg(folder, art_paths[i]) ) { pretty_folder = folder + strlen(art_paths[i]); break; }
        }
    //}

    int size = 0;
    void *ptr = 0;

#if 0
    // clean pathfile
    while (pathfile[0] == '.' && pathfile[1] == '/') pathfile += 2;
    while (pathfile[0] == '/') ++pathfile;
#endif

    const char *lookup_id = /*file_normalize_with_folder*/(pathfile);

    // search (last item)
    static __thread char  last_item[256] = { 0 };
    static __thread void *last_ptr = 0;
    static __thread int   last_size = 0;
    if( !strcmpi(lookup_id, last_item)) {
        ptr = last_ptr;
        size = last_size;
    }

    // search (cache)
    if( !ptr && !is(osx) ) { // @todo: remove silicon mac M1 hack
        ptr = cache_lookup(lookup_id, &size);
    }

    if( ptr ) {
        PRINTF("Loading VFS (%s)%s (cached)\n", pretty_folder, base);
    } else {
        PRINTF("Loading VFS (%s)%s\n", pretty_folder, base);
    }

    // read cooked asset from mounted disks
    if( !ptr ) {
        ptr = vfs_unpack(pathfile, &size);

        // asset not found? maybe it has not been cooked yet at this point (see --cook-on-demand)
        if( !ptr && COOK_ON_DEMAND ) {

            static thread_mutex_t mutex, *init = 0; if(!init) thread_mutex_init(init = &mutex);
            thread_mutex_lock(&mutex);

// this block saves some boot time (editor --cook-on-demand: boot 1.50s -> 0.90s)
#if 1 // EXPERIMENTAL_DONT_COOK_NON_EXISTING_ASSETS
            static set(char*) disk = 0;
            if(!disk) { set_init_str(disk); for each_substring(ART,",",art_folder) for each_array(file_list(va("%s**", art_folder)), char*, item) set_insert(disk, STRDUP(item)); } // art_folder ends with '/'
            int found = !!set_find(disk, (char*)pathfile);
            if( found )
#endif
            {
                // technically, we should only cook `base` asset at this point. however, cooks on demand can be very
                // expensive. not only because we're invoking an external tool/cook app in here, which is scanning all the
                // cook folders at every call, but also because there is this expensive vfs_reload() call at end of current scope.
                // so, in order to minimize the number of upcoming cook requests, we cook more stuff than needed at this point;
                // just in anticipation of what's likely going to happen in the next frames.
                // so, when asked to cook "models/model.fbx" we actually:
                // - do cook "models/model.* (so, "model.tga","model.png","model.jpg","model.mtl",etc. are also cooked)
                // - do cook "models/*.fbx" as well
                char *dir = file_path(pathfile + ART_SKIP_ROOT);
                char *group1 = dir[0] ? va("\"*/%s%s.*\"", dir, file_base(pathfile)) : base; // -> "*/models/model.*"
                char *group2 = dir[0] ? va("\"*/%s*%s\"", dir, file_ext(pathfile)) : ""; // -> "*/models/*.fbx"

                char *cmd = va("%scook" ifdef(osx,".osx",ifdef(linux,".linux",".exe"))" %s %s --cook-ini=%s --cook-additive --cook-jobs=1 --quiet", TOOLS, group1, group2, COOK_INI);

                // cook groups
                int rc = system(cmd); // atoi(app_exec(cmd));
                if(rc < 0) PANIC("cannot invoke `%scook` (return code %d)", TOOLS, rc);

                vfs_reload(); // @todo: optimize me. it is waaay inefficent to reload the whole VFS layout after cooking a single asset
            }

            thread_mutex_unlock(&mutex);

            // finally, try again
            pathfile = va("%s", vfs_resolve(pathfile));
            ptr = vfs_unpack(pathfile, &size);
        }

        if( ptr ) {
            cache_insert(lookup_id, ptr, size);
        }
    }

    if( ptr && size )
    if( ptr != last_ptr) {
        snprintf(last_item, 256, "%s", lookup_id);
        last_ptr = ptr;
        last_size = size;
    }

    // yet another last resort: redirect vfs_load() calls to file_load()
    // (for environments without tools or cooked assets)
    if(!ptr) {
        if( !have_tools() ) {
            ptr = file_load(pathfile, size_out);
        }
    }

    if(!ptr) {
        PRINTF("Loading %s (not found)\n", pathfile);
    }

*found = ptr ? true : false;

    if( size_out ) *size_out = ptr ? size : 0;
    return ptr;
}
char* vfs_read(const char *pathfile) {
    return vfs_load(pathfile, NULL);
}
int vfs_size(const char *pathfile) {
    int sz;
    return vfs_load(pathfile, &sz), sz;
}


FILE* vfs_handle(const char *pathfile) {
    // @fixme: non-compressible assets (adpcm wavs,mp3,ogg,mp4,avi,...) are taking a suboptimal code path here.
    //         no need to unzip them. just seek within the zipfile and return the *fp at that position
    int sz;
    char *buf = vfs_load(pathfile, &sz);
    FILE *fp = fmemopen(buf ? buf : "", buf ? sz : 0, "r+b");
    ASSERT( fp, "cannot create tempfile" );
    return fp;
}
#if 0
const char *vfs_extract(const char *pathfile) { // extract a vfs file into the local (temp) filesystem
#if 0
    FILE* fp = vfs_handle(pathfile);
    return fp ? pathfile_from_handle(fp) : "";
#else
    int sz;
    char *buf = vfs_load(pathfile, &sz);
    if( !buf ) return "";

    // pool of temp files. recycles after every loop
    enum { MAX_TEMP_FILES = 16 };
    static __thread char temps[MAX_TEMP_FILES][DIR_MAX] = {0};
    static __thread int i = 0;
    if( temps[i][0] ) unlink(temps[i]);
    i = (i + 1) % MAX_TEMP_FILES;
    if(!temps[i][0] ) snprintf(temps[i], DIR_MAX, "%s", file_tempname());
    char *name = temps[i];

    FILE *tmp = fopen(name, "wb"); //unlink(name);
    ASSERT( tmp, "cannot create tempfile %s", name );
    fwrite(buf ? buf : "", 1, buf ? sz : 0, tmp);
    fclose(tmp);
    return name;
#endif
}
#endif


// -----------------------------------------------------------------------------
// cache

static thread_mutex_t cache_mutex; AUTORUN{ thread_mutex_init(&cache_mutex); }

void* cache_lookup(const char *pathfile, int *size) { // find key->value
    if( !MAX_CACHED_FILES ) return 0;
    void* data = 0;
    thread_mutex_lock(&cache_mutex);
    for(archive_dir *dir = dir_cache; dir; dir = dir->next) {
        if( !strcmp(dir->path, pathfile) ) {
            if(size) *size = dir->size;
            data = dir->data;
            break;
        }
    }
    thread_mutex_unlock(&cache_mutex);
    return data;
}
void* cache_insert(const char *pathfile, void *ptr, int size) { // append key/value; return LRU or NULL
    if( !MAX_CACHED_FILES ) return 0;
    if( !ptr || !size ) return 0;

    // keep cached files within limits
    thread_mutex_lock(&cache_mutex);

        // append to cache
        archive_dir zero = {0}, *old = dir_cache;
        *(dir_cache = REALLOC(0, sizeof(archive_dir))) = zero;
        dir_cache->next = old;
        dir_cache->path = STRDUP(pathfile);
        dir_cache->size = size;
        dir_cache->data = REALLOC(0, size+1);
        memcpy(dir_cache->data, ptr, size); size[(char*)dir_cache->data] = 0; // copy+terminator

        void *found = 0;

        static int added = 0;
        if( added < MAX_CACHED_FILES ) {
            ++added;
        } else {
            // remove oldest cache entry
            for( archive_dir *prev = dir_cache, *dir = prev; dir ; prev = dir, dir = dir->next ) {
                if( !dir->next ) {
                    prev->next = 0; // break link
                    found = dir->data;
                    dir->path = REALLOC(dir->path, 0);
                    dir->data = REALLOC(dir->data, 0);
                    dir = REALLOC(dir, 0);
                    break;
                }
            }
        }

    thread_mutex_unlock(&cache_mutex);

    return found;
}

// ----------------------------------------------------------------------------
// ini


/* ini+, extended ini format
// - rlyeh, public domain
//
// # spec
//
//   ; line comment
//   [user]             ; map section name (optional)
//   name=john          ; key and value (mapped here as user.name=john)
//   +surname=doe jr.   ; sub-key and value (mapped here as user.name.surname=doe jr.)
//   age=30             ; numbers
//   color=240          ; key and value \
//   color=253          ; key and value |> array: color[0], color[1] and color[2]
//   color=255          ; key and value /
//   color=             ; remove key/value(s)
//   color=white        ; recreate key; color[1] and color[2] no longer exist
//   []                 ; unmap section
//   -note=keys may start with symbols (except plus and semicolon)
//   -note=linefeeds are either \r, \n or \r\n.
//   -note=utf8 everywhere.
*/

static
char *ini_parse( const char *s ) {
    char *map = 0;
    int mapcap = 0, maplen = 0;
    enum { DEL, REM, TAG, KEY, SUB, VAL } fsm = DEL;
    const char *cut[6] = {0}, *end[6] = {0};
    while( *s ) {
        while( *s && (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') ) ++s;
        /**/ if( *s == ';' ) cut[fsm = REM] = ++s;
        else if( *s == '[' ) cut[fsm = TAG] = ++s;
        else if( *s == '+' ) cut[fsm = SUB] = ++s;
        else if( *s == '=' ) cut[fsm = VAL] = ++s;
        else if( *s > ' ' && *s <= 'z' && *s != ']' ) cut[fsm = KEY] = cut[SUB] = end[SUB] = s;
        else { if( *s ) ++s; continue; }
        /**/ if( fsm == REM ) { while(*s && *s != '\r'&& *s != '\n') ++s; }
        else if( fsm == TAG ) { while(*s && *s != '\r'&& *s != '\n'&& *s != ']') ++s; end[TAG] = s; }
        else if( fsm == KEY ) { while(*s && *s >  ' ' && *s <= 'z' && *s != '=') ++s; end[KEY] = s; }
        else if( fsm == SUB ) { while(*s && *s >  ' ' && *s <= 'z' && *s != '=') ++s; end[SUB] = s; }
        else if( fsm == VAL ) { while(*s && *s >= ' ' && *s <= 127 && *s != ';') ++s; end[VAL] = s;
            while( end[VAL][-1] <= ' ' ) { --end[VAL]; }
            char buf[256] = {0}, *key = buf;
            if( end[TAG] - cut[TAG] ) key += sprintf(key, "%.*s.", (int)(end[TAG] - cut[TAG]), cut[TAG] );
            if( end[KEY] - cut[KEY] ) key += sprintf(key,  "%.*s", (int)(end[KEY] - cut[KEY]), cut[KEY] );
            if( end[SUB] - cut[SUB] ) key += sprintf(key, ".%.*s", (int)(end[SUB] - cut[SUB]), cut[SUB] );
            int reqlen = (key - buf) + 1 + (end[VAL] - cut[VAL]) + 1 + 1;
            if( (reqlen + maplen) >= mapcap ) map = REALLOC( map, mapcap += reqlen + 512 );
            sprintf( map + maplen, "%.*s%c%.*s%c%c", (int)(key - buf), buf, 0, (int)(end[VAL] - cut[VAL]), cut[VAL], 0, 0 );
            maplen += reqlen - 1;
        }
    }
    return map;
}

// @todo: evaluate alt api
// int count = ini_count(filename);
// char *key = ini_key(filename, id);
// char *val = ini_val(filename, id);

void ini_destroy(ini_t x) {
    for each_map(x, char*, k, char*, v) {
        FREE(k);
        FREE(v);
    }
    map_free(x);
}

ini_t ini_from_mem(const char *data) {
    if( !data || !data[0] ) return 0;

    char *kv = ini_parse(data);
    if( !kv ) return 0;

        ini_t map = 0;
        map_init(map, less_str, hash_str);
        for( char *iter = kv; iter[0]; ) {
            char *key = iter; while( *iter++ );
            char *val = iter; while( *iter++ );

            char **found = map_find(map, key);
            if( !found ) map_insert(map, STRDUP(key), STRDUP(val));
            assert( map_find(map,key) );
        }

    FREE( kv );

    return map;
}

ini_t ini(const char *filename) {
    char *kv = file_read(filename);
    if(!kv) kv = vfs_read(filename);
    return ini_from_mem(kv);
}

bool ini_write(const char *filename, const char *section, const char *key, const char *value) {
    // this is a little hacky {
    char *data = file_read(filename);
    if( data && data[0] ) {
        char *begin = strrchr(data, '[');
        char *end = strrchr(data, ']');
        if( begin && end && begin < end ) {
            char *last_section = va("%.*s", (int)(end - begin - 1), begin + 1);
            if( !strcmpi(last_section, section) ) section = 0;
        }
    }
    // }

    char *s = va("%s%s=%s\r\n", section ? va("[%s]\r\n", section) : "", key, value);
    return file_append(filename, s, strlen(s));
}

