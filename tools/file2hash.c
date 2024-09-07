// cl file2hash.c /O2 /Oy /MT /DNDEBUG /link setargv.obj
// tcc file2hash.c -O2

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>

uint64_t hash_64(uint64_t x) {
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}

uint64_t hash_bin(const void* ptr, unsigned len) {
    uint64_t hash = 14695981039346656037ULL; // hash(0),mul(131) faster than fnv1a, a few more collisions though
    for( unsigned char *str = (unsigned char *)ptr; len--; )
    hash = ( (unsigned char)*str++ ^ hash ) * 0x100000001b3ULL;
    return hash;
}

uint64_t hash_str(const char* str) {
    return hash_bin(str, strlen(str)+1);
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *file_load_fast(const char *filename, int *len) {
    char namepath[1024];
    sprintf(namepath, "engine/%s", filename);
    FILE *fp = fopen(namepath, "rb");
    if( fp ) {
        fseek(fp, 0L, SEEK_END);
        size_t sz = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        char *buffer = malloc(sz+1);
        sz *= fread(buffer,sz,1,fp) == 1;
        buffer[sz] = 0; 
        if(len) *len = (int)sz;
        fclose(fp);
        return buffer;
    }
    if (len) *len = 0;
    return 0;
}

char *file_load(const char *filename, int *len) { // @todo: 32 counters/thread enough?
    FILE *fp = fopen(filename, "rb");
    if( fp ) {
        fseek(fp, 0L, SEEK_END);
        size_t sz = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        char *buffer = malloc(sz+1);
        sz *= fread(buffer,sz,1,fp) == 1;
        buffer[sz] = 0;
        if(len) *len = (int)sz;
        fclose(fp);

        // Process for include directives
        char *read_buffer = malloc(sz + 1);
        memcpy(read_buffer, buffer, sz);
        read_buffer[sz] = '\0';
        char *line = strtok(read_buffer, "\n");
        while (line) {
            if (strncmp(line, "#include", 8) == 0) {
                char *include_file = strchr(line, '"');
                if (include_file) {
                    include_file++; // Move past the opening quote
                    char *end_quote = strchr(include_file, '"');
                    if (end_quote) {
                        *end_quote = '\0'; // Null-terminate the filename
                        int include_len;
                        char *include_content = file_load_fast(include_file, &include_len);
                        if (include_content) {
                            // Append include content to buffer
                            buffer = realloc(buffer, sz + include_len + 1);
                            memcpy(buffer + sz, include_content, include_len);
                            sz += include_len;
                            buffer[sz] = 0;
                            free(include_content);
                        }
                    }
                }
            }
            line = strtok(NULL, "\n");
        }

        free(read_buffer);
        if(len) *len = (int)sz;
        return buffer; // @fixme: return 0 on error instead?
    }
    if (len) *len = 0;
    return 0;
}

uint64_t file_stamp(const char *fname) {
    struct stat st;
    return stat(fname, &st) < 0 ? 0ULL : st.st_mtime;
}

int main(int argc, char **argv) {
    if( argc < 2 ) return printf("%s [infiles...] [-- strings...]\n", argv[0]), 0;

    int strings = 0;
    uint64_t hash = 0;
    for(int i = 1; i < argc; ++i) {
        if( !strcmp(argv[i], "--") ) strings = 1;
        else {
            int size = 0; char *ptr = strings ? 0 : file_load(argv[i], &size);
            uint64_t hash_contents = ptr ? hash_bin(ptr, size) : hash_str(argv[i]);
            uint64_t hash_modt = ptr ? hash_64( file_stamp(argv[i]) ) : 0;
            uint64_t hash_size = ptr ? hash_64( size ) : strlen(argv[i]);
            // if( ptr ) free(ptr); // speed optimization, do not free()

            hash ^= hash_contents /* ^ hash_modt */ ^ hash_size;
        }
    }

    printf("%llu\n", hash);
    return hash & 0x7FFFFFFF;
}
