//#define META_DEMO

#include "fwk.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdio.h>

static
char *reformat(const char *text, const char *blacklist, const char *separator) {
    char **list = strsplit(text, blacklist);
    char *joint = strjoin(list, separator);
    return joint;
}

const char *c_symbols = "{},>&/+=;:"; // "(){}[].,><&*/+=;:!~";
const char *c_symbols_extra = "{},>&/+=;:[]()"; // "(){}[].,><&*/+=;:!~";
const char *c_keywords[] = { "const","long","signed","unsigned","typedef" }; int num_keywords = 4;

typedef struct meta {
    const char *name;
    const char *tags;
} meta;
meta metas[2048] = {
/*00*/ {"typedef", ""},
/*01*/ {"enum", ""},
/*02*/ {"struct", ""},
/*03*/ {"function", ""},

/*04*/ {"bool",   "default 0"},
/*05*/ {"false",  "default 0 bits 1"},
/*06*/ {"true",   "default 1 bits 1"},

/*07*/ {"char",   "default 0 bits 8"},
/*08*/ {"int",    "default 0"},
/*09*/ {"float",  "default 0 bits 32"},
/*10*/ {"double", "default 0 bits 64"},

/*11*/ {"int8_t", "default 0 bits 8"},
/*12*/ {"int16_t", "default 0 bits 16"},
/*13*/ {"int32_t", "default 0 bits 32"},
/*14*/ {"int64_t", "default 0 bits 64"},
/*15*/ {"uint8_t", "default 0 bits 8 signed 0"},
/*16*/ {"uint16_t", "default 0 bits 16 signed 0"},
/*17*/ {"uint32_t", "default 0 bits 32 signed 0"},
/*18*/ {"uint64_t", "default 0 bits 64 signed 0"},
};
int meta_count = 19;
const char *meta_last_name = 0;
const char *meta_last_struct = 0;

meta* meta_find(const char *name) {
    for( int i = 0; i < meta_count; ++i ) {
        if( 0 == strcmp(name, metas[i].name) ) return &metas[i];
    }
    return 0;
}
meta* meta_add(const char *name, const char *subtype, const char *tags ) {
    { meta *t = meta_find(name); if( t ) return t; }

    if( meta_count >= 2048 ) return 0;

    meta t = { STRDUP(name), reformat(STRDUP(tags), " ", " ") };
    metas[meta_count] = t;
    meta_last_struct = !strcmp(subtype, "struct") ? name : meta_last_struct;
    meta_last_name = !strcmp(subtype, "struct") || !strcmp(subtype, "enum") || !strcmp(subtype, "typedef") ? name : subtype;
    return &metas[meta_count++];
}
char* meta_find_value(const char *name, const char *prop) {
    meta *t = meta_find(name);
    if( !t ) return 0;
    char *found = strstr(t->tags, prop);
    if( !found ) return 0;

    static char copy[256] = {0}; // @fixme
    strcpy(copy, found + strlen(prop) + 1);
    for(int i=0; copy[i]; ++i) if(copy[i]==' ') { copy[i] = 0; break; }
    return copy;
}
void meta_inspectf(FILE *fp) {
    if( fp ) {
        char buf[255+1];

        while(fgets(buf, 255, fp)) {
            char *mark = strstr(buf, "//""M");
            if( !mark ) continue;
            char *tags = mark + 3;

            // must reset last_struct
            if( strstr(buf, "typedef") ) {
                meta_last_struct = 0; // typedef is a special case
            }

            // remove symbols
            for( int i = 0; buf[i]; ++i ) {
                // symbols off
                if(strchr(&buf[i] < tags ? c_symbols : c_symbols_extra, buf[i])) { buf[i] = ' '; continue; }
            }

            // remove leading whitespaces
            char *spc = buf;
            while( spc && spc[0] && spc[0] <= 32 ) ++spc;

            // remove trailing linefeeds and whitespaces
            int len = strlen(buf);
            while( len > 0 && buf[len-1] <= 32 ) buf[--len] = 0;

            // split buffer into left|right buffers
            char *left = spc, *right = tags; right[-1] = 0;

            // remove left keywords
            for( int i = 0; left[i]; ++i ) {
                for(int j = 0; j < sizeof(c_keywords)/sizeof(0[c_keywords]); ++j) {
                    int kb = strlen(left+i);
                    int kl = strlen(c_keywords[j]);
                    if( kl <= kb && !memcmp(left+i, c_keywords[j], kl) ) {
                        memset(left+i, ' ', kl);
                        i += kl;
                        break;
                    }
                }
            }

            // debug
            // printf("%s <--> %s\n", left, right);

            // add type
            const char *meta_name = 0;
            const char *meta_t = 0;
            const char *meta_val = "";
            array(char *) tokens = strsplit(left, " \t");
            if(1) for( int i = 0, end = array_count(tokens); i < end; ++i ) {
                /**/ if( strstr(tokens[i],"enum") ) meta_last_name = 0, meta_last_struct = 0, meta_t = "enum";
                else if( strstr(tokens[i],"struct") ) meta_last_name = 0, meta_last_struct = 0, meta_t = "struct";
                else if( strstr(tokens[i],"typedef") ) meta_last_name = 0, meta_last_struct = 0, meta_t = "typedef"; // trimmed. never happens
                else if( strchr(tokens[i],'(') ) meta_last_name = 0, meta_last_struct = 0, meta_name = tokens[i], meta_t = "function";
                else if( !meta_name  ) meta_name = tokens[i];
                else if( !meta_t ) meta_t = tokens[i];
            }

            if( meta_find(meta_name) ) {
                const char *swap = meta_name;
                meta_name = meta_t;
                meta_t = swap;
            }

            // if !meta_find defer(meta_eval) && emit(warning) && keep(iterating);

            bool is_pointer = strstr(meta_name, "*");
            meta_name = (char *)reformat(meta_name, "*", "");
            char *found_sqr = strstr(meta_name, "[");
            int array_len = found_sqr ? atoi(found_sqr+1) : 0;
            if( found_sqr ) *found_sqr = 0;

            bool is_number = false;
            char conv[32] = {0};
            double dbl = atof(meta_t);
            if(!is_number) { sprintf(conv, "%f", dbl); is_number = !strcmp(conv, meta_t); }
            if(!is_number) { sprintf(conv, "%.f", dbl); is_number = !strcmp(conv, meta_t); }
            if(is_number) {
                meta_val = meta_t;
                meta_t = meta_last_name;
            }

            char hints[256] = {0}, *p = hints;
            if( 1 )                             p += sprintf(p, "type %s ", meta_t);
            if( is_pointer || array_len )       p += sprintf(p, "pointer %d ", 1);
            if( array_len )                     p += sprintf(p, "count %d ", array_len);
            if( 1 )                             p += sprintf(p, "%s ", tags);
            if( meta_last_struct )              p += sprintf(p, "struct %s ", meta_last_struct);
            if( meta_val[0] )                   p += sprintf(p, "default %s ", meta_val);
            meta_val = hints;

            // inherited tags (from super type) at the very end, so overriden tags can be found at first place
            if( meta_find(meta_t) )
            p += sprintf(p, "%s ", meta_find(meta_t)->tags );


            if( !meta_find(meta_name) ) {
                meta_add(meta_name, meta_t, meta_val);

                meta *t = meta_find(meta_name);
                //printf("%s,%s\n", t->name, t->tags);
            }

        }
        // clean state
        meta_last_name = 0;
        meta_last_struct = 0;
    }
}

bool meta_save(FILE *fp) {
    if( fp ) 
    for( int i = 0; i < meta_count; ++i ) {
        meta *t = &metas[i];
        fprintf(fp, "%s %s\n", t->name, t->tags);
    }
    return !!fp;
}
bool meta_load(FILE *fp) {
    if( fp ) {
        meta_count = 0;
        meta_last_name = 0;
        meta_last_struct = 0;

        char buf[2048+1];
        while(fgets(buf, 2048, fp)) {
            int bl = strlen(buf); while( bl && buf[bl] <= 32) buf[bl--] = 0;
            char id[128];
            sscanf(buf, "%s", id);
            metas[ meta_count ].name = strdup(id);
            metas[ meta_count ].tags = strdup(buf+strlen(id)+1);
            ++meta_count;
        }
    }
    return !!fp;
}

void meta_inspect(const char *name) {
    FILE *fp = fopen(name, "rb");
    if( !fp ) return;
    meta_inspectf(fp);
    fclose(fp);
}
bool meta_savefile(const char *name) {
    FILE *fp = fopen(name, "wb");
    if( !fp ) return false;
    meta_save(fp); fclose(fp);
    return true;
}
bool meta_loadfile(const char *name) {
    FILE *fp = fopen(name, "rb");
    if( !fp ) return false;
    meta_load(fp); fclose(fp);
    return true;
}


#ifdef META_DEMO
#pragma once

typedef int my_array[8]; //M
typedef const char *string; //M

typedef enum Fruit {    //M bits:8
    Banana = -1,        //M
    Orange = 42         //M
} Fruit;

struct myObject {       //M version:100
    Fruit meal;         //M default:Banana
    int32_t hitpoints;  //M default:100 zigzag:yes
    float shininess;    //M default:+3.5e2 min:0 max:1 fixed:yes
    string text;        //M default:"DEMO"
    bool visible;       //M bits:1 deprecated:1 version:001
    char test;          //M serialize:off
};

int print(int); //M const:true

typedef int dummy; //M extra=separators allowed(too)

int main(int argc, const char **argv) {

#ifndef NDEBUG
    unlink(__FILE__ ".meta");
#endif

    if( meta_loadfile(__FILE__ ".meta") ) {
        puts("Loaded .meta file");
    } else {
        puts("Cannot read .meta file. regenerating...");
        meta_inspect(__FILE__);
        puts("Saving .meta file...");
        meta_savefile(__FILE__ ".meta");
    }

    meta_save( stdout );

    puts(meta_find_value("Orange", "type"));
    puts(meta_find_value("Orange", "bits"));
    puts(meta_find_value("Orange", "default"));
    puts(meta_find_value("visible", "type"));
    puts(meta_find_value("shininess", "fixed"));
}

#define main main__
#endif
