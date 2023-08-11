import os
import sys
import ctypes
import cffi

ffi = cffi.FFI()
with open('./fwk.lua') as f:
    lines = [line for line in f if not line.startswith('#')]
    lines = [line for line in lines if not 'va_list' in line]
    lines = [line for line in lines if not 'inline ' in line]
    lines = [line for line in lines if not line.startswith('typedef union ') ]
    lines = [line for line in lines if not '//lcpp INF' in line ]
    data = ''.join(lines)
    data = data[data.find('[[')+2:data.find(']]')]
    data = '''
    typedef struct vec2i { int x,y; } vec2i;
    typedef struct vec3i { int x,y,z; } vec3i;
    typedef struct vec2 { float x,y; } vec2;
    typedef struct vec3 { float x,y,z; } vec3;
    typedef struct vec4 { float x,y,z,w; } vec4;
    typedef struct quat { float x,y,z,w; } quat;
    typedef union frustum frustum;
    typedef union json_t json_t;
    ''' + data
    ffi.cdef(data)
fwk = ffi.dlopen('./fwk.dll')
