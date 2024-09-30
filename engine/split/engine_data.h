// -----------------------------------------------------------------------------
// data framework (json5, xml, compression) @todo:kvdb
// - rlyeh, public domain
//
// @todo: vec2,vec3,vec4

typedef union json_t { char* s; double f; int64_t i; uintptr_t p; array(union json_t) arr; } json_t;

// json api

API bool            json_push(const char *json_content);
API const char*         json_key(const char *keypath);
API json_t*             json_find(const char *type_keypath);
API json_t              json_get(const char *type_keypath);
API int                 json_count(const char *keypath);
#define                 json_int(...)    (json_get(va("i" __VA_ARGS__)).i)
#define                 json_float(...)  (json_get(va("f" __VA_ARGS__)).f)
#define                 json_string(...) (json_get(va("s" __VA_ARGS__)).s)
#define                 json_key(...)     json_key(va(__VA_ARGS__))
#define                 json_count(...)   json_count(va(__VA_ARGS__))
API bool            json_pop();

// xml api

API int             xml_push(const char *xml_content);
API const char *        xml_string(char *key);
API unsigned            xml_count(char *key);
API array(char)         xml_blob(char *key);
#define                 xml_string(...) xml_string(va(__VA_ARGS__))       // syntax sugar: string
#define                 xml_int(...)    atoi(xml_string(__VA_ARGS__))     // syntax sugar: int
#define                 xml_float(...)  atof(xml_string(__VA_ARGS__))     // syntax sugar: float
#define                 xml_blob(...)   xml_blob(va(__VA_ARGS__))         // syntax sugar: base64 blob
#define                 xml_count(...)  xml_count(va(__VA_ARGS__))        // syntax sugar: count nodes
API void            xml_pop();

API bool data_tests();
