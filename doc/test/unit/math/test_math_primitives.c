#include <stdio.h>
#include <math.h>
#include "core/math/math.h"

static int fails = 0;
#define CHECK(c, m) do { if (!(c)) { printf("FAIL: %s\n", m); fails++; } } while (0)
#define NEAR(a, b, eps) (fabs((double)((a) - (b))) <= (eps))

int main(void)
{
    /* 线段 */
    {
        segment s = segment_new(vec3_new(0, 0, 0), vec3_new(2, 0, 0));
        vec3 q = segment_closest_point(s, vec3_new(1, 1, 0));
        CHECK(NEAR(segment_length(s), 2, 1e-6f), "segment length");
        CHECK(vec3_is_equal(q, vec3_new(1, 0, 0)), "segment closest");
        CHECK(NEAR(segment_distance_sq(s, vec3_new(1, 1, 0)), 1, 1e-6f), "segment distance");
        CHECK(segment_contains_point(s, vec3_new(1, 0, 0), 1e-5f), "segment contains");
    }
    /* 三角形 */
    {
        triangle t = triangle_new(vec3_new(0, 0, 0), vec3_new(1, 0, 0), vec3_new(0, 1, 0));
        vec3 n = triangle_normal(t);
        real_t u, v, w;
        CHECK(NEAR(triangle_area(t), 0.5f, 1e-6f), "triangle area");
        CHECK(NEAR(n.z, 1, 1e-6f), "triangle normal");
        CHECK(vec3_is_equal(triangle_centroid(t), vec3_new(1.0f / 3.0f, 1.0f / 3.0f, 0)), "triangle centroid");
        CHECK(triangle_barycentric(t, vec3_new(0.25f, 0.25f, 0), &u, &v, &w), "triangle barycentric ok");
        CHECK(NEAR(u, 0.5f, 1e-5f) && NEAR(v, 0.25f, 1e-5f) && NEAR(w, 0.25f, 1e-5f), "triangle barycentric values");
        CHECK(triangle_contains_point(t, vec3_new(0.25f, 0.25f, 0), 1e-5f), "triangle contains");
        CHECK(!triangle_contains_point(t, vec3_new(0.75f, 0.75f, 0), 1e-5f), "triangle not contains");
        CHECK(vec3_is_equal(triangle_closest_point(t, vec3_new(0.25f, 0.25f, 2)), vec3_new(0.25f, 0.25f, 0)), "triangle closest");
        CHECK(!triangle_is_degenerate(t), "triangle not degenerate");
        CHECK(triangle_is_degenerate(triangle_new(vec3_zero(), vec3_new(1, 0, 0), vec3_new(2, 0, 0))), "triangle degenerate");
    }
    /* 胶囊体 */
    {
        capsule c = capsule_new(vec3_new(0, 0, 0), vec3_new(0, 2, 0), 1);
        CHECK(capsule_contains_point(c, vec3_new(0, 1, 0.5f)), "capsule contains");
        CHECK(!capsule_contains_point(c, vec3_new(2, 1, 0)), "capsule not contains");
        CHECK(NEAR(capsule_distance_sq(c, vec3_new(0, 1, 2)), 1, 1e-5f), "capsule distance");
        CHECK(vec3_is_equal(capsule_closest_point(c, vec3_new(0, 1, 2)), vec3_new(0, 1, 1)), "capsule closest");
        CHECK(NEAR(capsule_height(c), 2, 1e-6f), "capsule height");
    }
    /* OBB */
    {
        obb b = obb_new(vec3_zero(), vec3_new(1, 2, 3), mat3_identity());
        aabb w = obb_to_aabb(b);
        obb rot = obb_new(vec3_zero(), vec3_new(1, 2, 3), mat3_rotate_z(MATH_HALF_PI));
        CHECK(obb_contains_point(b, vec3_new(0.5f, 1, 2)), "obb contains");
        CHECK(!obb_contains_point(b, vec3_new(2, 0, 0)), "obb not contains");
        CHECK(vec3_is_equal(obb_corner(b, 7), vec3_new(1, 2, 3)), "obb corner");
        CHECK(vec3_is_equal(aabb_size(w), vec3_new(2, 4, 6)), "obb to aabb");
        CHECK(vec3_is_equal(obb_closest_point(b, vec3_new(2, 0, 0)), vec3_new(1, 0, 0)), "obb closest");
        CHECK(!obb_contains_point(rot, vec3_new(0, 1.5f, 0)), "obb rotated contains");
        CHECK(obb_contains_point(rot, vec3_new(1.5f, 0, 0)), "obb rotated contains x");
    }
    /* 线段相交 */
    {
        vec3 p1, p2;
        real_t d2 = intersect_segment_segment(
            segment_new(vec3_new(0, 0, 0), vec3_new(1, 0, 0)),
            segment_new(vec3_new(0, 1, 0), vec3_new(1, 1, 0)), &p1, &p2);
        CHECK(NEAR(d2, 1, 1e-5f), "segment_segment parallel");
        CHECK(NEAR(p1.y, 0, 1e-5f) && NEAR(p2.y, 1, 1e-5f), "segment_segment points");
        d2 = intersect_segment_segment(
            segment_new(vec3_new(0, 0, 0), vec3_new(1, 0, 0)),
            segment_new(vec3_new(0.5f, -1, 0), vec3_new(0.5f, 1, 0)), 0, 0);
        CHECK(NEAR(d2, 0, 1e-5f), "segment_segment crossing");

        { ray_hit h;
          CHECK(intersect_segment_plane(segment_new(vec3_new(0, 0, -1), vec3_new(0, 0, 1)),
              plane_from_point_normal(vec3_zero(), vec3_new(0, 0, 1)), &h), "segment_plane hit");
          CHECK(NEAR(h.t, 0.5f, 1e-5f) && NEAR(h.point.z, 0, 1e-5f), "segment_plane t"); }
        CHECK(intersect_segment_aabb(segment_new(vec3_new(-2, 0, 0), vec3_new(2, 0, 0)),
            aabb_new(vec3_new(-1, -1, -1), vec3_new(1, 1, 1))), "segment_aabb hit");
        CHECK(!intersect_segment_aabb(segment_new(vec3_new(2, 0, 0), vec3_new(3, 0, 0)),
            aabb_new(vec3_new(-1, -1, -1), vec3_new(1, 1, 1))), "segment_aabb miss");
        { triangle t = triangle_new(vec3_zero(), vec3_new(1, 0, 0), vec3_new(0, 1, 0));
          ray_hit h;
          CHECK(intersect_segment_triangle(segment_new(vec3_new(0.25f, 0.25f, -1), vec3_new(0.25f, 0.25f, 1)), t, &h), "segment_triangle hit");
          CHECK(NEAR(h.point.z, 0, 1e-5f), "segment_triangle point"); }
    }
    /* 胶囊体相交 */
    {
        capsule c = capsule_new(vec3_new(0, 0, 0), vec3_new(0, 2, 0), 1);
        CHECK(intersect_capsule_sphere(c, sphere_new(vec3_new(3, 1, 0), 2)), "capsule_sphere touch");
        CHECK(!intersect_capsule_sphere(c, sphere_new(vec3_new(3.01f, 1, 0), 2)), "capsule_sphere miss");
        CHECK(intersect_capsule_plane(c, plane_from_point_normal(vec3_new(0, 0, 0.5f), vec3_new(0, 0, 1))), "capsule_plane near");
        CHECK(!intersect_capsule_plane(c, plane_from_point_normal(vec3_new(0, 0, 1.5f), vec3_new(0, 0, 1))), "capsule_plane far");
        CHECK(intersect_capsule_capsule(c, capsule_new(vec3_new(2, 0, 0), vec3_new(2, 2, 0), 1)), "capsule_capsule touch");
        CHECK(!intersect_capsule_capsule(c, capsule_new(vec3_new(2.1f, 0, 0), vec3_new(2.1f, 2, 0), 1)), "capsule_capsule miss");
    }
    /* OBB 相交 */
    {
        obb a = obb_new(vec3_zero(), vec3_new(1, 1, 1), mat3_identity());
        ray r = ray_new(vec3_new(5, 0, 0), vec3_new(-1, 0, 0));
        real_t t0, t1;
        CHECK(intersect_ray_obb(r, a, &t0, &t1), "ray_obb hit");
        CHECK(NEAR(t0, 4, 1e-5f) && NEAR(t1, 6, 1e-5f), "ray_obb t");
        CHECK(intersect_obb_sphere(a, sphere_new(vec3_new(2, 0, 0), 1)), "obb_sphere touch");
        CHECK(!intersect_obb_sphere(a, sphere_new(vec3_new(2.1f, 0, 0), 1)), "obb_sphere miss");
        CHECK(intersect_obb_obb(a, obb_new(vec3_new(1.9f, 0, 0), vec3_new(1, 1, 1), mat3_identity())), "obb_obb overlap");
        CHECK(!intersect_obb_obb(a, obb_new(vec3_new(2.1f, 0, 0), vec3_new(1, 1, 1), mat3_identity())), "obb_obb separate");
        { obb rb = obb_new(vec3_new(2.5f, 0, 0), vec3_new(1, 1, 1), mat3_rotate_z(MATH_QUARTER_PI));
          CHECK(!intersect_obb_obb(a, rb), "obb_obb rotated separate"); }
        CHECK(intersect_obb_aabb(a, aabb_new(vec3_new(-1, -1, -1), vec3_new(1, 1, 1))), "obb_aabb overlap");
        CHECK(!intersect_obb_aabb(a, aabb_new(vec3_new(2, 2, 2), vec3_new(3, 3, 3))), "obb_aabb separate");
    }

    printf("\n%s (%d failures)\n", fails ? "FAIL" : "ALL PASS", fails);
    return fails ? 1 : 0;
}