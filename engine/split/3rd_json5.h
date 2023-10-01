// JSON5 + SJSON parser module
//
// License:
// This software is dual-licensed to the public domain and under the following
// license: you are granted a perpetual, irrevocable license to copy, modify,
// publish, and distribute this file as you see fit.
// No warranty is implied, use at your own risk.
//
// Credits:
// r-lyeh (fork)
// Dominik Madarasz (@zaklaus) (original code)

#ifndef JSON5_H
#define JSON5_H

#ifndef JSON5_ASSERT
#define JSON5_ASSERT do { printf("JSON5: Error L%d while parsing '%c' in '%.16s'\n", __LINE__, p[0], p); assert(0); } while(0)
#endif

#include <stdint.h>
#include <stdio.h>

typedef enum json5_type {
    JSON5_UNDEFINED, // 0
    JSON5_NULL,      // 1
    JSON5_BOOL,      // 2
    JSON5_OBJECT,    // 3
    JSON5_STRING,    // 4
    JSON5_ARRAY,     // 5
    JSON5_INTEGER,   // 6
    JSON5_REAL,      // 7
} json5_type;

typedef struct json5 {
    char*      name;
#ifdef NDEBUG
    unsigned   type : 3;
#else
    json5_type type;
#endif
    unsigned   count : 29;
    union {
        struct json5* array;
        struct json5* nodes;
        int64_t   integer;
        double    real;
        char*     string;
        int       boolean;
    };
} json5;

char* json5_parse(json5 *root, char *source, int flags);
void  json5_write(FILE *fp, const json5 *root);
void  json5_free(json5 *root);

#endif // JSON5_H

// json5 ----------------------------------------------------------------------

#ifdef JSON5_C
//#pragma once
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

char *json5__trim(char *p) {
    while (*p) {
        /**/ if( isspace(*p) ) ++p;
        else if( p[0] == '/' && p[1] == '*' ) { // skip C comment
            for( p += 2; *p && !(p[0] == '*' && p[1] == '/'); ++p) {}
            if( *p ) p += 2;
        }
        else if( p[0] == '/' && p[1] == '/' ) { // skip C++ comment
            for( p += 2; *p && p[0] != '\n'; ++p) {}
            if( *p ) ++p;
        }
        else break;
    }
    return p;
}

char *json5__parse_value(json5 *obj, char *p, char **err_code);

char *json5__parse_string(json5 *obj, char *p, char **err_code) {
    assert(obj && p);

    if( *p == '"' || *p == '\'' || *p == '`' ) {
        obj->type = JSON5_STRING;
        obj->string = p + 1;

        char eos_char = *p, *b = obj->string, *e = b;
        while (*e) {
            /**/ if( *e == '\\' && (e[1] == eos_char) ) ++e;
            else if( *e == '\\' && (e[1] == '\r' || e[1] == '\n') ) *e = ' ';
            else if( *e == eos_char ) break;
            ++e;
        }

        *e = '\0';
        return p = e + 1;
    }

    //JSON5_ASSERT; *err_code = "json5_error_invalid_value";
    return NULL;
}

char *json5__parse_object(json5 *obj, char *p, char **err_code) {
    assert(obj && p);

    if( 1 /* *p == '{' */ ) { /* <-- for SJSON */
        int skip = *p == '{'; /* <-- for SJSON */

        obj->type = JSON5_OBJECT;
        obj->nodes = 0;
        obj->count = 0;

        while (*p) {
            json5 node = { 0 };

            do { p = json5__trim(p + skip); skip = 1; } while( *p == ',' );

            if( *p == '}' ) {
                ++p;
                break;
            }
            // @todo: is_unicode() (s[0] == '\\' && isxdigit(s[1]) && isxdigit(s[2]) && isxdigit(s[3]) && isxdigit(s[4]))) {
            else if( isalnum(*p) || *p == '_' || *p == '$' || *p == '.' ) { // also || is_unicode(p)
                node.name = p;

                do {
                    ++p;
                } while (*p && (isalnum(*p) || *p == '_' || *p == '$' || *p == '.') ); // also || is_unicode(p)

                char *e = p;
                p = json5__trim(p);
                *e = '\0';
            }
            else { //if( *p == '"' || *p == '\'' || *p == '`' ) {
                char *ps = json5__parse_string(&node, p, err_code);
                if( !ps ) {
                    return NULL;
                }
                p = ps;
                node.name = node.string;
                p = json5__trim(p);
            }

            // @todo: https://www.ecma-international.org/ecma-262/5.1/#sec-7.6
            if( !(node.name && node.name[0]) ) { // !json5__validate_name(node.name) ) {
                JSON5_ASSERT; *err_code = "json5_error_invalid_name";
                return NULL;
            }

            if( !p || (*p && (*p != ':' && *p != '=' /* <-- for SJSON */)) ) {
                JSON5_ASSERT; *err_code = "json5_error_invalid_name";
                return NULL;
            }
            p = json5__trim(p + 1);
            p = json5__parse_value(&node, p, err_code);

            if( *err_code[0] ) {
                return NULL;
            }

            if( node.type != JSON5_UNDEFINED ) {
                array_push(obj->nodes, node);
                ++obj->count;
            }

            if( *p == '}') { ++p; break; }
        }

        return p;
    }

    JSON5_ASSERT; *err_code = "json5_error_invalid_value";
    return NULL;
}

char *json5__parse_value(json5 *obj, char *p, char **err_code) {
    assert(obj && p);

    p = json5__trim(p);

    char *is_string = json5__parse_string(obj, p, err_code);

    if( is_string ) {
        p = is_string;
        if( *err_code[0] ) {
            return NULL;
        }
    }
    else if( *p == '{' ) {
        p = json5__parse_object( obj, p, err_code );
        if( *err_code[0] ) {
            return NULL;
        }
    }
    else if( *p == '[' ) {
        obj->type = JSON5_ARRAY;
        obj->array = 0;
        obj->count = 0;

        while (*p) {
            json5 elem = { 0 };

            do { p = json5__trim(p + 1); } while( *p == ',' );
            if( *p == ']') { ++p; break; }

            p = json5__parse_value(&elem, p, err_code);

            if( *err_code[0] ) {
                return NULL;
            }

            if( elem.type != JSON5_UNDEFINED ) {
                array_push(obj->array, elem);
                ++obj->count;
            }
            if (*p == ']') { ++p; break; }
        }
    }
    else if( isalpha(*p) || (*p == '-' && !isdigit(p[1])) ) {
        const char *labels[] = { "null", "on","true", "off","false", "nan","NaN", "-nan","-NaN", "inf","Infinity", "-inf","-Infinity", 0 };
        const int lenghts[] = { 4, 2,4, 3,5, 3,3, 4,4, 3,8, 4,9 };
        for( int i = 0; labels[i]; ++i ) {
            if( !strncmp(p, labels[i], lenghts[i] ) ) {
                p += lenghts[i];
#ifdef _MSC_VER // somehow, NaN is apparently signed in MSC
                /**/ if( i >= 5 ) obj->type = JSON5_REAL, obj->real = i >= 11 ? -INFINITY : i >= 9 ? INFINITY : i >= 7 ?  NAN :-NAN;
#else
                /**/ if( i >= 5 ) obj->type = JSON5_REAL, obj->real = i >= 11 ? -INFINITY : i >= 9 ? INFINITY : i >= 7 ? -NAN : NAN;
#endif
                else if( i >= 1 ) obj->type = JSON5_BOOL, obj->boolean = i <= 2;
                else              obj->type = JSON5_NULL;
                break;
            }
        }
        if( obj->type == JSON5_UNDEFINED ) {
            JSON5_ASSERT; *err_code = "json5_error_invalid_value";
            return NULL;
        }
    }
    else if( isdigit(*p) || *p == '+' || *p == '-' || *p == '.' ) {
        char buffer[32] = {0}, *buf = buffer, is_hex = 0, is_dbl = 0;
        while( *p && strchr("+-.xX0123456789aAbBcCdDeEfF", *p)) {
            is_hex |= (*p | 32) == 'x';
            is_dbl |= *p == '.';
            *buf++ = *p++;
        }
        obj->type = is_dbl ? JSON5_REAL : JSON5_INTEGER;
        long long unsigned int llu;
        long long int lli;
        /**/ if( is_dbl ) sscanf( buffer, "%lf", &obj->real );
        else if( is_hex ) sscanf( buffer, "%llx", &llu ), obj->integer = llu; // SCNx64 -> inttypes.h
        else              sscanf( buffer, "%lld", &lli ), obj->integer = lli; // SCNd64 -> inttypes.h
    }
    else {
        return NULL;
    }
    return p;
}

char *json5_parse(json5 *root, char *p, int flags) {
    char *err_code = "";
    *root = (json5) {0};

    if( p && p[0] ) {
        p = json5__trim(p);
        if( *p == '[' ) { /* <-- for SJSON */
            json5__parse_value(root, p, &err_code);
        } else {
            json5__parse_object(root, p, &err_code); /* <-- for SJSON */
        }
    } else {
        root->type = JSON5_OBJECT;
    }

    return err_code[0] ? err_code : 0;
}

void json5_free(json5 *root) {
    if( root->type == JSON5_ARRAY && root->array ) {
        for( int i = 0, cnt = array_count(root->array); i < cnt; ++i ) {
            json5_free(root->array + i);
        }
        array_free(root->array);
    }

    if( root->type == JSON5_OBJECT && root->nodes ) {
        for( int i = 0, cnt = array_count(root->nodes); i < cnt; ++i ) {
            json5_free(root->nodes + i);
        }
        array_free(root->nodes);
    }

    *root = (json5) {0}; // needed?
}

void json5_write(FILE *fp, const json5 *o) {
    static __thread int indent = 0;
    int tabs = 1; // 0,1,2,4,8
    if( o->name ) {
        fprintf(fp, "%*.s\"%s\"%s", indent * tabs, "", o->name, tabs ? ": " : ":");
    }
    /**/ if( o->type == JSON5_NULL ) fprintf(fp, "%s", "null");
    else if( o->type == JSON5_BOOL ) fprintf(fp, "%s", o->boolean ? "true" : "false");
    else if( o->type == JSON5_INTEGER ) fprintf(fp, "%lld", (long long int)o->integer);
    else if( o->type == JSON5_REAL ) {
        /**/ if( isnan(o->real) ) fprintf(fp, "%s", signbit(o->real) ? "-nan" : "nan" );
        else if( isinf(o->real) ) fprintf(fp, "%s", signbit(o->real) ? "-inf" : "inf" );
        else fprintf(fp, "%1.8e", o->real); // %1.8e from google:"randomascii 100 digits" ; %.4llf for compactness
    }
    #if 0
    else if( o->type == JSON5_STRING ) { // write (escaped) string
        char chars[] = "\\\"\n\r\b\f\v", remap[] = "\\\"nrbfv", esc[256];
        for( int i = 0; chars[i]; ++i ) esc[ chars[i] ] = remap[i];

        const char *b = o->string, *e = strpbrk(b, chars), *sep = "\"";
        while( e ) {
            fprintf(fp, "%s%.*s\\%c", sep, (int)(e - b), b, esc[(unsigned char)*e] );
            e = strpbrk( b = e + 1, chars);
            sep = "";
        }
        fprintf(fp, "%s%s\"", sep, b);
    }
    #else
    else if( o->type == JSON5_STRING ) { // write string
        fprintf(fp, "\"%s\"", o->string);
    }
    #endif
    else if( o->type == JSON5_ARRAY ) {
        const char *sep = "";
        fprintf(fp, "%s", tabs ? "[ " : "[");
        for( int i = 0, cnt = o->count; i < cnt; ++i ) {
            fprintf(fp, "%s", sep); sep = tabs ? ", " : ",";
            json5_write(fp, o->array + i);
        }
        fprintf(fp, "%s", tabs ? " ]" : "]");
    }
    else if( o->type == JSON5_OBJECT ) {
        const char *sep = "";
        fprintf(fp, "%*.s{%s", 0 * (++indent) * tabs, "", tabs ? "\n":"");
        for( int i = 0, cnt = o->count; i < cnt; ++i ) {
            fprintf(fp, "%s", sep); sep = tabs ? ",\n" : ",";
            json5_write(fp, o->nodes + i);
        }
        fprintf(fp, "%s%*.s}", tabs ? "\n":"", (--indent) * tabs, "");
    } else {
        char p[16] = {0};
        JSON5_ASSERT; /* "json5_error_invalid_value"; */
    }
}

#ifdef JSON5_BENCH
#include <time.h>
int main() {
    // https://www.reddit.com/r/datasets/comments/1uyd0t/200000_jeopardy_questions_in_a_json_file/
    char *content = 0;
    for( FILE *fp = fopen("jeopardy.json", "rb"); fp; fclose(fp), fp = 0 ) {
        fseek(fp, 0L, SEEK_END);
        size_t pos = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        content = (char*)malloc( pos + 1 );
        fread(content, 1, pos, fp);
        content[pos] = 0;
    }

    if( content ) {
        clock_t start = clock();
        json5 root = {0};
        char *error = json5_parse(&root, content, 0);
        clock_t end = clock();
        double delta = ( end - start ) / (double)CLOCKS_PER_SEC;

        if( !error ) {
            printf("Parsing time: %.3fms\n", delta*1000);
            printf("Total nodes: %d\n", array_count(root.array));
            printf("Category: %s, air date: %s\nQuestion: %s\n", root.array[0].nodes[0].string,
                   root.array[0].nodes[1].string,
                   root.array[0].nodes[2].string);
        } else {
            printf("Error: %s\n", error);
        }

        json5_free(&root);
        free(content);
    }
}
#define main main__
#endif

#ifdef JSON5_DEMO
int main() {
    char source5[] =
    "  // comments\n" /* json5 sample */
    "  unquoted: 'and you can quote me on that',\n"
    "  singleQuotes: 'I can use \"double quotes\" here',\n"
    "  lineBreaks : \"Look, Mom! \\\n"
    "No \\n's!\",\n"
    "  hexadecimal: 0x100,\n"
    "  leadingDecimalPoint: .8675309, andTrailing: 8675309.,\n"
    "  positiveSign: +1,\n"
    "  trailingComma: 'in objects', andIn: ['arrays', ],\n"
    "  \"backwardsCompatible\": \"with JSON\",\n"
    ""
    "  ip = \"127.0.0.1\"\n" /* sjson sample */
    "  port = 8888\n"
    ""
    "  /* comment //nested comment*/\n" /* tests */
    "  // comment /*nested comment*/\n"
    "  nil: null,"
    "  \"+lšctžýáíé=:\": true,,,,"
    "  huge: 2.2239333e5, "
    "  array: [+1,2,-3,4,5],    "
    "  hello: 'world /*comment in string*/ //again', "
    "  abc: 42.67, def: false, "
    "  children : { a: 1, b: 2, },"
    "  invalids : [ nan, NaN, -nan, -NaN, inf, Infinity, -inf, -Infinity ],"
    ""
    "  multiline: `this is\n"
    "a multiline string\n"
    "yeah`"
    "}\n";

    json5 root = { 0 };
    char *error = json5_parse(&root, source5, 0);
    if( error ) {
        printf("Error: %s\n", error);
    } else {
        json5_write(stdout, &root);
    }
    json5_free(&root);
}
#define main main__
#endif

#endif // JSON5_C
