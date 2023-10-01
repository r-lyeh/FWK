// -----------------------------------------------------------------------------
// factory of handle ids, based on code by randy gaul (PD/Zlib licensed)
// - rlyeh, public domain
//
// [src] http://www.randygaul.net/wp-content/uploads/2021/04/handle_table.cpp
// [ref] http://bitsquid.blogspot.com.es/2011/09/managing-decoupling-part-4-id-lookup.html
// [ref] http://glampert.com/2016/05-04/dissecting-idhashindex/
// [ref] https://github.com/nlguillemot/dof/blob/master/viewer/packed_freelist.h
// [ref] https://gist.github.com/pervognsen/ffd89e45b5750e9ce4c6c8589fc7f253

// convert between hard refs (pointers) and weak refs (ids)
uintptr_t id_make(void *ptr);
void *     id_handle(uintptr_t id);
void       id_dispose(uintptr_t id);
bool        id_valid(uintptr_t id);

// configuration:
// ideally, these two should be 32 each. they were changed to fit our OBJHEADER bits
#ifndef ID_INDEX_BITS
#define ID_INDEX_BITS 16
#endif
#ifndef ID_COUNT_BITS
#define ID_COUNT_BITS  3
#endif
// you cannot change this one: the number of ID_DATA_BITS you can store in a handle depends on ID_COUNT_BITS
#define ID_DATA_BITS (64-ID_COUNT_BITS)
