
static
array(char) base64__decode(const char *in_, unsigned inlen) {
    // from libtomcrypt
    #define BASE64_ENCODE_OUT_SIZE(s)   (((s) + 2) / 3 * 4)
    #define BASE64_DECODE_OUT_SIZE(s)   (((s)) / 4 * 3)

#if 1
    unsigned long outlen = BASE64_DECODE_OUT_SIZE(inlen);
    array(char) out_ = 0; array_resize(out_, outlen);

    if( base64_decode((const unsigned char *)in_, (unsigned long)inlen, (unsigned char *)out_, &outlen) != CRYPT_OK ) {
        array_free(out_);
        return 0;
    }

    array_resize(out_, outlen);
    return out_;
#else
    unsigned outlen = BASE64_DECODE_OUT_SIZE(inlen);
    array(char) out_ = 0; array_resize(out_, outlen);

    // based on code by Jon Mayo - November 13, 2003 (PUBLIC DOMAIN)
    uint_least32_t v;
    unsigned ii, io, rem;
    char *out = (char *)out_;
    const unsigned char *in = (const unsigned char *)in_;
    const uint8_t base64dec_tab[256]= {
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255, 62,255,255,
         52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,  0,255,255,
        255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
         15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255, 63,
        255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
         41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    };

    for (io = 0, ii = 0,v = 0, rem = 0; ii < inlen; ii ++) {
        unsigned char ch;
        if (isspace(in[ii]))
            continue;
        if ((in[ii]=='=') || (!in[ii]))
            break; /* stop at = or null character*/
        ch = base64dec_tab[(unsigned char)in[ii]];
        if (ch == 255)
            break; /* stop at a parse error */
        v = (v<<6) | ch;
        rem += 6;
        if (rem >= 8) {
            rem -= 8;
            if (io >= outlen)
                return (array_free(out_), NULL); /* truncation is failure */
            out[io ++] = (v >> rem) & 255;
        }
    }
    if (rem >= 8) {
        rem -= 8;
        if (io >= outlen)
            return (array_free(out_), NULL); /* truncation is failure */
        out[io ++] = (v >> rem) & 255;
    }
    return (array_resize(out_, io), out_);
#endif
}

static array(json5) roots;
static array(char*) sources;

bool json_push(const char *source) {
    char *source_rw = STRDUP(source);
    json5 root = {0};
    char *error = json5_parse(&root, source_rw, 0);
    if( error ) {
        FREE(source_rw);
        return false;
    } else {
        array_push(sources, source_rw);
        array_push(roots, root);
        return true;
    }
}

bool json_pop() {
    if( array_count(roots) > 0 ) {
        FREE(*array_back(sources));
        array_pop(sources);

        json5_free(array_back(roots));
        array_pop(roots);
        return true;
    }
    return false;
}

json5* json_node(const char *keypath) {
    json5 *j = array_back(roots), *r = j;
    for each_substring( keypath, "/[.]", key ) {
        r = 0;
        /**/ if( j->type == JSON5_ARRAY ) r = j = &j->array[atoi(key)];
        else if( j->type == JSON5_OBJECT && isdigit(key[0]) )
        for( int i = 0, seq = atoi(key); !r && i < j->count; ++i ) {
            if( i == seq ) {
                r = j = &j->nodes[i];
                break;
            }
        }
        else if( j->type == JSON5_OBJECT )
        for( int i = 0; !r && i < j->count; ++i ) {
            if( j->nodes[i].name && !strcmp(j->nodes[i].name, key) ) {
                r = j = &j->nodes[i];
                break;
            }
        }
        if( !j ) break;
    }
    return r;
}

int (json_count)(const char *keypath) {
    json5* j = json_node(keypath);
    return j ? j->count : 0;
}

json_t *json_find(const char *type_keypath) {
    char type = type_keypath[0];
    const char *key = type_keypath+1;
    json5 *j = json_node(key);
    if( !j ) return NULL;

    static __thread int slot = 0;
    static __thread json_t buf[128] = {0};
    slot = (slot+1) % 128;

    json_t *v = &buf[slot];
    v->i = j ? j->integer : 0;
    if(type == 's' && (!v->p || j->type == JSON5_NULL)) v->s = ""; // if_null_string
    if(type == 'f' && j && j->type == JSON5_INTEGER) v->f = j->integer;
    return v;
}

json_t json_get(const char *type_keypath) {
    char type = type_keypath[0];
    const char *key = type_keypath+1;
    json5 *j = json_node(key);

    json_t v = {0};
    v.i = j ? j->integer : 0;
    if(type == 's' && (!v.p || j->type == JSON5_NULL)) v.s = ""; // if_null_string
    if(type == 'f' && j && j->type == JSON5_INTEGER) v.f = j->integer;
    return v;
}

const char *(json_key)(const char *keypath) {
    json5 *j = json_node(keypath);
    if( !j ) return "";
    return j->name;
}

// xml impl

static __thread array(char *) xml_sources;
static __thread array(struct xml *) xml_docs;

int xml_push(const char *xml_source) {
    if( xml_source ) {
        char *src = STRDUP(xml_source), *error = 0;
        for( struct xml *doc = xml_parse(src, 0, &error); doc && !error; ) {
            array_push(xml_docs, doc);
            array_push(xml_sources, src);
            return 1;
        }
        if( error ) PRINTF("%s\n", error);
        FREE(src);
    }
    return 0;
}

void xml_pop() {
    if( array_count(xml_docs) ) {
        xml_free( *array_back(xml_docs) );
        array_pop(xml_docs);
        FREE( *array_back(xml_sources) );
        array_pop(xml_sources);
    }
}

static void *xml_path(struct xml *node, char *path, int down) {
    if( !path || !path[0] ) return node;
    if( node ) {

        char type = path[0];
        if( type == '/' ) {
            int sep = strcspn(++path, "/[@$");
            if( !sep ) type = path[0];
            else
            if( 1 ) { // path[ sep ] ) {
                char tag[32]; snprintf(tag, 32, "%.*s", sep, path);
                // Find the first sibling with the given tag name (may be the same node)
                struct xml *next = down ? xml_find_down(node, tag) : xml_find(node, tag);
                return xml_path(next, &path[ sep ], 1);
            }            
        }
        if( type == '$' ) {
            return (void*)( node->down ? xml_text( node->down ) : xml_tag( node ) );
        }
        if( type == '@' ) {
            return (void*)xml_att(node, ++path);
        }
        if( type == '[' ) {
            for( int i = 0, end = atoi(++path); i < end; ++i ) { node = xml_find_next(node, xml_tag(node)); if(!node) return NULL; }
            while( isdigit(path[0]) ) ++path;
            return xml_path(node, ++path, 1);
        }
    }
    return NULL;
}

const char *(xml_string)(char *key) {
    struct xml *node = xml_path(*array_back(xml_docs), key, 0);
    if( !node ) return "(null)";
    if( strchr(key, '@') ) return (const char *)node;
    if( strchr(key, '$') ) return (const char *)node;
    return "";
}
unsigned (xml_count)(char *key) {
    struct xml *node = xml_path(*array_back(xml_docs), key, 0);
    if( !node ) return 0;
    const char *tag = xml_tag(node);
    unsigned count = 1;
    while( (node = xml_find_next(node, tag)) != 0) ++count; 
    return count;
}
array(char) (xml_blob)(char *key) { // base64 blob
    struct xml *node = xml_path(*array_back(xml_docs), key, 0);
    if( !node ) return 0;
    if( !strchr(key, '$') ) return 0;
    const char *data = (const char*)node;
    array(char) out = base64__decode(data, strlen(data)); // either array of chars (ok) or null (error)
    return out;
}

bool data_tests() {
    // data tests (json5)
    const char json5[] =
    "  /* json5 */ // comment\n"
    "  abc: 42.67, def: true, integer:0x100 \n"
    "  huge: 2.2239333e5, \n"
    "  hello: 'world /*comment in string*/ //again', \n"
    "  children : { a: 1, b: 2, c: 3 },\n"
    "  array: [+1,2,-3,4,5],    \n"
    "  invalids : [ nan, NaN, -nan, -NaN, inf, Infinity, -inf, -Infinity ],";
    if( json_push(json5) ) {
        assert( json_float("/abc") == 42.67 );
        assert( json_int("/def") == 1 );
        assert( json_int("/integer") == 0x100 );
        assert( json_float("/huge") > 2.22e5 );
        assert( strlen(json_string("/hello")) == 35 );
        assert( json_int("/children/a") == 1 );
        assert( json_int("/children.b") == 2 );
        assert( json_int("/children[c]") == 3 );
        assert( json_int("/array[%d]", 2) == -3 );
        assert( json_count("/invalids") == 8 );
        assert( isnan(json_float("/invalids[0]")) );
        assert( !json_find("/non_existing") );
        assert( PRINTF("json5 tests OK\n") );
        json_pop();
    }

    // data tests (xml)
    const char *xml = // vfs_read("test1.xml");
    "<!-- XML representation of a person record -->"
    "<person created=\"2006-11-11T19:23\" modified=\"2006-12-31T23:59\">"
    "    <firstName>Robert</firstName>"
    "    <lastName>Smith</lastName>"
    "    <address type=\"home\">"
    "        <street>12345 Sixth Ave</street>"
    "        <city>Anytown</city>"
    "        <state>CA</state>"
    "        <postalCode>98765-4321</postalCode>"
    "    </address>"
    "</person>";
    if( xml_push(xml) ) {
        assert( !strcmp("Robert", xml_string("/person/firstName/$")) );
        assert( !strcmp("Smith", xml_string("/person/lastName/$")) );
        assert( !strcmp("home", xml_string("/person/address/@type")) );
        assert( PRINTF("xml tests OK\n") );
        xml_pop();
    }

    return true;
}

// compression api

static struct zcompressor {
    // id of compressor
    unsigned enumerator;
    // name of compressor
    const char name1, *name4, *name;
    // returns worst case compression estimation for selected flags
    unsigned (*bounds)(unsigned bytes, unsigned flags);
    // returns number of bytes written. 0 if error.
    unsigned (*encode)(const void *in, unsigned inlen, void *out, unsigned outcap, unsigned flags);
    // returns number of excess bytes that will be overwritten when decoding.
    unsigned (*excess)(unsigned flags);
    // returns number of bytes written. 0 if error.
    unsigned (*decode)(const void *in, unsigned inlen, void *out, unsigned outcap);
} zlist[] = {
    { COMPRESS_RAW,     '0', "raw",  "raw",     raw_bounds, raw_encode, raw_excess, raw_decode },
    { COMPRESS_PPP,     'p', "ppp",  "ppp",     ppp_bounds, ppp_encode, ppp_excess, ppp_decode },
    { COMPRESS_ULZ,     'u', "ulz",  "ulz",     ulz_bounds, ulz_encode, ulz_excess, ulz_decode },
    { COMPRESS_LZ4,     '4', "lz4x", "lz4x",    lz4x_bounds, lz4x_encode, lz4x_excess, lz4x_decode },
    { COMPRESS_CRUSH,   'c', "crsh", "crush",   crush_bounds, crush_encode, crush_excess, crush_decode },
    { COMPRESS_DEFLATE, 'd', "defl", "deflate", deflate_bounds, deflate_encode, deflate_excess, deflate_decode },
    { COMPRESS_LZP1,    '1', "lzp1", "lzp1",    lzp1_bounds, lzp1_encode, lzp1_excess, lzp1_decode },
    { COMPRESS_LZMA,    'm', "lzma", "lzma",    lzma_bounds, lzma_encode, lzma_excess, lzma_decode },
    { COMPRESS_BALZ,    'b', "balz", "balz",    balz_bounds, balz_encode, balz_excess, balz_decode },
    { COMPRESS_LZW3,    'w', "lzw3", "lzrw3a",  lzrw3a_bounds, lzrw3a_encode, lzrw3a_excess, lzrw3a_decode },
    { COMPRESS_LZSS,    's', "lzss", "lzss",    lzss_bounds, lzss_encode, lzss_excess, lzss_decode },
    { COMPRESS_BCM,     'B', "bcm",  "bcm",     bcm_bounds, bcm_encode, bcm_excess, bcm_decode },
    { COMPRESS_ZLIB,    'z', "zlib", "zlib",    deflate_bounds, deflatez_encode, deflate_excess, deflatez_decode },
};

enum { COMPRESS_NUM = 14 };

static char *znameof(unsigned flags) {
    static __thread char buf[16];
    snprintf(buf, 16, "%4s.%c", zlist[(flags>>4)&0x0F].name4, "0123456789ABCDEF"[flags&0xF]);
    return buf;
}
unsigned zencode(void *out, unsigned outlen, const void *in, unsigned inlen, unsigned flags) {
    return zlist[(flags >> 4) % COMPRESS_NUM].encode(in, inlen, (uint8_t*)out, outlen, flags & 0x0F);
}
unsigned zdecode(void *out, unsigned outlen, const void *in, unsigned inlen, unsigned flags) {
    return zlist[(flags >> 4) % COMPRESS_NUM].decode((uint8_t*)in, inlen, out, outlen);
}
unsigned zbounds(unsigned inlen, unsigned flags) {
    return zlist[(flags >> 4) % COMPRESS_NUM].bounds(inlen, flags & 0x0F);
}
unsigned zexcess(unsigned flags) {
    return zlist[(flags >> 4) % COMPRESS_NUM].excess(flags & 0x0F);
}
