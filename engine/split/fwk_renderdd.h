// -----------------------------------------------------------------------------
// debugdraw framework
// - rlyeh, public domain.
//
// Credits: Based on work by @glampert https://github.com/glampert/debug-draw (PD)
// [x] grid, axis, frustum, cube, sphere, triangle, square, pentagon, hexagon, circle, normal.
// [x] arrow, point, text, capsule, aabb, plane, flotilla-style locator, boid, bone, ring
// [x] line batching
// [*] line width and stipple
// [*] (proper) gizmo,
// [ ] camera, light bulb, light probe,

API void ddraw_color(unsigned rgb);
API void ddraw_color_push(unsigned rgb);
API void ddraw_color_pop();
//
API void ddraw_ontop(int enabled);
API void ddraw_ontop_push(int enabled);
API void ddraw_ontop_pop();
//
API void ddraw_push_2d();
API void ddraw_pop_2d();
//
API void ddraw_aabb(vec3 minbb, vec3 maxbb);
API void ddraw_aabb_corners(vec3 minbb, vec3 maxbb);
API void ddraw_arrow(vec3 begin, vec3 end);
API void ddraw_axis(float units);
API void ddraw_boid(vec3 pos, vec3 dir);
API void ddraw_bone(vec3 center, vec3 end); // @todo: use me
API void ddraw_bounds(const vec3 points[8]);
API void ddraw_box(vec3 center, vec3 extents);
API void ddraw_capsule(vec3 from, vec3 to, float radius);
API void ddraw_circle(vec3 pos, vec3 n, float radius);
API void ddraw_ring(vec3 pos, vec3 n, float radius);
API void ddraw_cone(vec3 center, vec3 top, float radius);
API void ddraw_cube(vec3 center, float radius);
API void ddraw_cube33(vec3 center, vec3 radius, mat33 M);
API void ddraw_diamond(vec3 from, vec3 to, float size);
API void ddraw_frustum(float projview[16]);
API void ddraw_ground(float scale);
API void ddraw_grid(float scale);
API void ddraw_hexagon(vec3 pos, float radius);
API void ddraw_line(vec3 from, vec3 to);
API void ddraw_line_dashed(vec3 from, vec3 to);
API void ddraw_line_thin(vec3 from, vec3 to);
API void ddraw_normal(vec3 pos, vec3 n);
API void ddraw_pentagon(vec3 pos, float radius);
API void ddraw_plane(vec3 p, vec3 n, float scale);
API void ddraw_point(vec3 from);
API void ddraw_position(vec3 pos, float radius);
API void ddraw_position_dir(vec3 pos, vec3 dir, float radius);
API void ddraw_pyramid(vec3 center, float height, int segments);
API void ddraw_cylinder(vec3 center, float height, int segments);
API void ddraw_sphere(vec3 pos, float radius);
API void ddraw_square(vec3 pos, float radius);
API void ddraw_text(vec3 pos, float scale, const char *text);
API void ddraw_text2d(vec2 pos, const char *text);
API void ddraw_triangle(vec3 p1, vec3 p2, vec3 p3);
//
API void ddraw_prism(vec3 center, float radius, float height, vec3 normal, int segments);
//
API void ddraw_demo();
API void ddraw_flush();
API void ddraw_flush_projview(mat44 proj, mat44 view);
