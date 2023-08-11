layout (local_size_x = 16, local_size_y = 16) in;

layout (rgba32f, binding = 0) uniform image2D img;

void main() {
    ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
    gid.x = clamp(gid.x, 1, int(gl_NumWorkGroups.x*gl_WorkGroupSize.x)-2);
    gid.y = clamp(gid.y, 1, int(gl_NumWorkGroups.y*gl_WorkGroupSize.y)-2);
    vec4 sum = vec4(0);

    for (int y = -1; y <= 1; ++y) {
        for (int x = -1; x <= 1; ++x) {
            sum += imageLoad(img, gid + ivec2(x,y));
        }
    }
    vec4 avg = sum / 9.0;

    barrier();
    imageStore(img, gid, avg);
}