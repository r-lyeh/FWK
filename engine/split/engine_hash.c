// hash helper utilities

uint32_t hh_mem(const void *data, size_t size) {
    const uint8_t *ptr = (const uint8_t *)data;
    uint32_t hash = 0;
    for (size_t i = 0; i < size; ++i) {
        hash = (hash * 31) + ptr[i];
    }
    return hash;
}

uint32_t hh_str(const char* str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

uint32_t hh_float(float f) {
    union { float f; uint32_t i; } u;
    u.f = f;
    return u.i;
}

uint32_t hh_int(int i) {
    return (uint32_t)i;
}

uint32_t hh_vec2(vec2 v) {
    return hh_float(v.x) ^ (hh_float(v.y) << 1);
}

uint32_t hh_vec3(vec3 v) {
    return hh_float(v.x) ^ (hh_float(v.y) << 1) ^ (hh_float(v.z) << 2);
}

uint32_t hh_vec4(vec4 v) {
    return hh_float(v.x) ^ (hh_float(v.y) << 1) ^ (hh_float(v.z) << 2) ^ (hh_float(v.w) << 3);
}

uint32_t hh_mat33(mat33 m) {
    uint32_t hash = 0;
    for (int i = 0; i < 9; ++i)
        hash ^= hh_float(m[i]) << i;
    return hash;
}

uint32_t hh_mat44(mat44 m) {
    uint32_t hash = 0;
    for (int i = 0; i < 16; ++i)
        hash ^= hh_float(m[i]) << i;
    return hash;
}
