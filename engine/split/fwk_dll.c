#if is(win32)
#   include <winsock2.h>
#   define dlopen(name,mode)    (void*)( (name) ? LoadLibraryA(name) : GetModuleHandle(NULL))
#   define dlsym(handle,symbol) GetProcAddress((HMODULE)handle, symbol )
#   define dlclose(handle)      0
#else
#   include <dlfcn.h>
#endif

void* dll(const char *filename, const char *symbol) {
/*
    char *buf, *base = file_name(filename);
    if( file_exists(buf = va("%s", base)) ||
        file_exists(buf = va("%s.dll", base)) ||
        file_exists(buf = va("%s.so", base)) ||
        file_exists(buf = va("lib%s.so", base)) ||
        file_exists(buf = va("%s.dylib", base)) ) {
        filename = buf;
    }
*/
    void *dll = dlopen(filename, RTLD_NOW | RTLD_LOCAL);
    dll = dll ? dlsym(dll, symbol) : 0;
    return dll;
}

#if 0 // demo: cl demo.c /LD && REM dll
EXPORT int add2(int a, int b) { return a + b; }
int main() { int (*adder)() = dll("demo.dll", "add2"); printf("%d\n", adder(2,3)); }
#endif
