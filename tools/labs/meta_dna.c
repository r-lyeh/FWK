// @todo fsave: fputs(DNA), then fwrite
// @todo fread: fgets(DNA), abort if DNA != read; then fread

#include <stdio.h>

// load/save whole struct acording to its DNA structure
int fload(FILE *infile, void *structure, const char *dna);
int fsave(FILE *outfile, const void *structure, const char *dna);

// set alignment for next fload/fsave call. resets back to 0 after every fload/fsave call.
// value: 0 (auto, default), 1,2,4,8,16 bytes [...]
void falign(unsigned alignment); 

// override default DNA handlers and switch to a different schema and/or serialization format
typedef int (*size_operator)(char fmt, void *addr, int readmode);
typedef int (*call_operator)(void *out, void *addr, char fmt, int count, int readmode);
void foverride(size_operator size, call_operator call);

#if 0
Example Usage
-------------

    struct fat_bootsector {
        uint8_t jump_instruction[3];
        uint8_t oem_name[8];
        uint16_t bytes_per_sector;
        uint8_t sectors_per_cluster;
        uint16_t reserved_sectors;
        uint8_t fat_copies;
        uint16_t max_dirs;
        uint16_t sector_count;
        uint8_t media_descriptor;
        uint16_t sectors_per_fat;
        uint16_t sectors_per_head;
        uint16_t heads;
        uint32_t hidden_sectors;
        uint32_t sector_count2;
    } fat_struct;

Now we can read a binary image of the MBR into this structure:

    mreadf(mbr, "i3c8chchchhchhhdd", &fat_struct);

Directives
----------

Supported directives:

    regex   meaning

    (blank) ignored

    b       read / write uint8_t
    w       read / write uint16_t (uppercase: use vli encoding; as uint8? )
    u       read / write uint32_t (uppercase: use vli encoding; as uint8,16? )
    q       read / write uint64_t (uppercase: use vli encoding; as uint8,16,32?)

    m       read / write micro
    h       read / write half     (uppercase: use smallest representation; as micro?)
    f       read / write float    (uppercase: use smallest representation; as micro,half?)
    d       read / write double   (uppercase: use smallest representation; as micro,half,float?)

    s       read / write string   (uppercase: use smallest representation; quarks?)
    []      read / write buffer

    <       switch to Intel (little endian) byte order
    >       switch to Motorola (big endian) byte order
    (       begin of tight group
    )       end of tight group
    [0-9]+  next item repeated n times
    z       skip one byte of input / emit \0
    *       consume next structure item but do not read / write
#endif

// ----------------------------------------------------------------------------

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef __thread
#define __thread __declspec(thread)
#endif

static __thread int pragma_pack_alignment = 0;

void falign(unsigned alignment) {
    pragma_pack_alignment = alignment;
}

static
int size_fn(char fmt, uint8_t* addr8, int rd) {
    if(addr8) {
        // sizeof pointee data & align operator
        /**/ if(fmt == 'c' || fmt == 'b') return 1;
        else if(fmt == 'w'              ) return 2;
        else if(fmt == 'i' || fmt == 'u') return 4;
        else if(fmt == 'l' || fmt == 'q') return 8;
        else if(fmt == 'f'              ) return 4;
        else if(fmt == 'd'              ) return 8;
        else if(fmt == 's'              ) return !*(char**)addr8 ? 0+1 : !rd ? strlen(*(char**)addr8) + 1 : strcspn(*(char**)addr8,"\x1") + 1;
        return -1;
    } else {
        // sizeof member
        /**/ if(fmt == 'w'              ) return 2;
        else if(fmt == 'i' || fmt == 'u') return 4;
        else if(fmt == 'l' || fmt == 'q') return 8;
        else if(fmt == 'f'              ) return 4;
        else if(fmt == 'd'              ) return 8;
        else if(fmt == 's'              ) return sizeof(void*);
        return 1;
    }
}

static
int call_fn(void *out, uint8_t *addr8, char fmt, int count, int rd) { // rd/wr operator
    FILE *fp = (FILE*)out;
    int iterated_bytes = 0, processed_bytes = 0;
    while( --count >= 0 ) {
        int slot = size_fn(fmt, 0, rd);
        int size = size_fn(fmt, addr8, rd);
        if(rd)
        switch (fmt) {
            default: return -1;
            break; case 'c': case 'b': fscanf(fp, "%c,", (uint8_t*)addr8);
            break; case 'w':           fscanf(fp, "%#04llx,", (uint16_t*)addr8);
            break; case 'i': case 'u': fscanf(fp, "%#08llx,", (uint32_t*)addr8);
            break; case 'l': case 'q': fscanf(fp, "%#16llx,", (uint64_t*)addr8);
            break; case 'f':           fscanf(fp, "%f,", (float*)addr8);
            break; case 'd':           fscanf(fp, "%llf,", (double*)addr8);
            break; case 's':           fscanf(fp, "%[^\x1],", (char*)addr8);
        }
        else
        switch(fmt) {
            default: return -1;
            break; case 'c': case 'b': fprintf(fp, "%c,", (int)*(uint8_t*)addr8);
            break; case 'w':           fprintf(fp, "%#04llx,", (uint64_t)*(uint16_t*)addr8);
            break; case 'i': case 'u': fprintf(fp, "%#08llx,", (uint64_t)*(uint32_t*)addr8);
            break; case 'l': case 'q': fprintf(fp, "%#16llx,", (uint64_t)*(uint64_t*)addr8);
            break; case 'f':           fprintf(fp, "%f,", (float)*(float*)addr8);
            break; case 'd':           fprintf(fp, "%f,", (double)*(double*)addr8);
            break; case 's':           fprintf(fp, "%s\x1,", *(char**)addr8);
        }
        addr8 += slot;
        iterated_bytes += slot;
        processed_bytes += size;
    }
    return iterated_bytes;
}

static size_operator fsize_fn = size_fn;
static call_operator fcall_fn = call_fn;

void foverride(size_operator size, call_operator call) {
    fsize_fn = size;
    fcall_fn = call;
}

int fdump(FILE *out, void *addr, const char *dna, int rd) {
    unsigned pragma_pack = pragma_pack_alignment; pragma_pack_alignment = 0; // reset alignment
    uint8_t *addr8 = (uint8_t*)addr;
    int size = 0, count = 1, skip = 0;
    int last_type = 0;
    int written = 0;
    int align = 0;
    for( int i = 0; dna[i]; ++i) {
        char fmt = dna[i];
        /**/ if(fmt <= 32) continue;
        else if(fmt >= '0' && fmt <= '9') continue;
        else if(fmt == 'z') skip = 1;
        else { 

            // member alignment
            if( last_type != fmt ) { // check if next struct member was found (may need re-alignment)
                if( pragma_pack != 1 ) { // forced (>1) or auto-alignment (0)?
                    //printf("*%p ->", addr8);
                    align = pragma_pack == 0 ? fsize_fn(fmt, 0, rd) : pragma_pack;
                    // Round up to N-byte boundary
                    addr8 = (uint8_t*)(((uintptr_t)(addr8) + ((align) - 1)) & -(align));
                    //printf(" %p\n", addr8);
                }
            }

            last_type = fmt;

            size = fsize_fn(fmt, addr8, rd);
            if( size < 0 ) {
                fprintf(stderr, "parse error, unknown dna sequence '%c'!\n", fmt);
                return -i-1; 
            }
        }

        if( skip ) { skip = 0; continue; }

        char next = dna[i+1];
        if( next >= '0' && next <= '9' ) {
            count = next - '0';
        }
        int bytes = skip || count == 0 ? 0 : call_fn(out, addr8, dna[i], count, rd);
        if( bytes < 0 ) {
            fprintf(stderr, "stream fail. rc: %d\n", bytes);
            return bytes;
        }
        written += bytes;
        addr8 += bytes;
        count = 1;

        fprintf(out, "\n");
    }
    return written;
}

int fsave(FILE *out, const void *structure, const char *dna) {
    return fdump(out, (void*)structure, dna, 0);
}
int fload(FILE *inf, void *structure, const char *dna) {
    return fdump(inf, structure, dna, 1);
}
int fsavefile(const char *outfile, const void *structure, const char *dna) {
	FILE *fp = fopen(outfile, "wb");
	if( !fp ) return 0;
    int bytes = fdump(fp, (void*)structure, dna, 0);
    fclose(fp);
    return bytes;
}
int floadfile(const char *infile, void *structure, const char *dna) {
	FILE *fp = fopen(infile, "rb");
	if( !fp ) return 0;
    int bytes = fdump(fp, (void*)structure, dna, 1);
    fclose(fp);
    return bytes;
}

// ---

#include <assert.h>


//  #pragma pack(1)

struct fat_mbr {
    uint8_t jmp[3]; // b3
    uint8_t oem[8];         // b8
    const char* str; // s
    uint16_t bytes_per_sector;   // w
    uint8_t sectors_per_cluster; // b
    uint16_t reserved_sectors;   // w
    uint8_t fat_copies;          // b
    uint16_t max_dirs;           // w
    uint16_t sector_count;       // w
    uint8_t media_descriptor;    // b
    uint16_t sectors_per_fat;    // w
    uint16_t sectors_per_head;   // w
    uint16_t heads;              // w
    uint32_t hidden_sectors;     // u
    uint32_t sector_count2;      // u
    float    pi;
    char break_alignment;  // b
    double   phi;
};

    #define FAT_MBR_DNA "b3b8s wb wb w wb www uu fbd"

//  #pragma pack(pop)


int main() {

//    foverride(size_fn, write_fn);

    struct fat_mbr mbr = {
        {'a','b','c'},
        {'d','e','f','g','h','i','j','k'},
        "hello 'escaped' \"world\"",
        0x100,'l',
        0x101,'m',
        0x102,
        0x103,'n',
        0x104,
        0x105,
        0x106,
        0x01234567,
        0x89abcdef,
        3.14159f,'o',
        1.6069,
    };

//    fdump(stdout, &mbr, FAT_MBR_DNA); exit(0);
//  printf("%p\n", &mbr.jmp);
//  printf("%p\n", &mbr.oem);
//  printf("%p\n", &mbr.str);
//  printf("%p\n", &mbr.sector_count);

//  falign(0);
    int bytes = fsave(stdout, &mbr, FAT_MBR_DNA);
    printf("%d bytes written\n", bytes);


    typedef struct entitystate {
        struct pos {
            short trTime;
            float trBase[3];
        } pos;
    } entitystate;
    entitystate e = { 123, 3.14159f, 4.14159f, 5.14159f };
    bytes = fsave(stdout, &e, "wfff");
    printf("%d bytes written\n", bytes);


//    exit(0);

    struct fat_mbr zero = {0};
    struct fat_mbr src = mbr;
    struct fat_mbr dst = zero;

    dst = src;
    assert(0 == memcmp(&src,&dst,sizeof(struct fat_mbr)));

    dst = zero;
    assert(0 != memcmp(&src,&dst,sizeof(struct fat_mbr)));

    int sbytes = fsavefile(".temp", &src, FAT_MBR_DNA);
    int lbytes = floadfile(".temp", &dst, FAT_MBR_DNA);
    assert( sbytes == lbytes );
    assert(0 != memcmp(&src,&dst,sizeof(struct fat_mbr)));

    assert(!puts("Ok"));
}
