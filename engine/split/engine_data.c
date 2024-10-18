
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
    if( node && strchr(key, '@') ) return (const char *)node;
    if( node && strchr(key, '$') ) return (const char *)node;
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
    array(char) out = base64_decode(data, strlen(data)); // either array of chars (ok) or null (error)
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
