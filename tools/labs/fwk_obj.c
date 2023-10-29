// ---

void *obj_copy(void **dst, const void *src) {
    if(!*dst) return *dst = obj_clone(src);

    if( obj_typeeq(*dst, src) ) {
        return memcpy(*dst, src, obj_sizeof(src));
    }

    return NULL;
}

void *obj_mutate(void **dst_, const void *src) {
    // mutate a class. ie, convert a given object class into a different one,
    // while preserving the original metas and references as much as possible.
    //
    // @fixme: systems might be tracking objects in the future. the fact that we
    // can reallocate a pointer (and hence, change its address) seems way too dangerous,
    // as the tracking systems could crash when referencing a mutated object.
    // solutions: do not reallocate if sizeof(new_class) > sizeof(old_class) maybe? good enough?
    // also, optimization hint: no need to reallocate if both sizes matches, just copy contents.

    if(!*dst_) return *dst_ = obj_clone(src);

    void *dst = *dst_;
    dtor(dst);

        unsigned src_sz = obj_sizeof(src);
        unsigned src_id = obj_typeid(src);

        void *dst_ptr = *((void**)dst - 1);
        unsigned payload = (OBJPAYLOAD16(dst_ptr) & 255) | src_id << 8;
        FREE( OBJUNBOX(dst_ptr) );
        *((void**)dst - 1) = OBJBOX( STRDUP( OBJUNBOX(*((void**)src - 1)) ), payload);

        void *base = (void*)((void**)dst - 1);
        base = REALLOC(base, src_sz + sizeof(void*));
        *dst_ = (char*)base + sizeof(void*);
        dst = (char*)base + sizeof(void*);
        memcpy(dst, src, src_sz);

    ctor(dst);
    return dst;
}
