#include <stdio.h>
#include <math.h>
#include "core/math/math.h"

static int fails = 0;
#define CHECK(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); fails++; } } while (0)
#define NEAR(a, b, eps) (fabs((double)((a) - (b))) <= (eps))

int main(void)
{
    /* ---- aabb ---- */
    {
        aabb b = aabb_new(vec3_new(-1, -1, -1), vec3_new(1, 1, 1));
        vec3 c = aabb_center(b), cl = aabb_closest_point(b, vec3_new(2, 0, 0));
        aabb tb = aabb_transform(b, mat4_translate(vec3_new(1, 0, 0)));
        CHECK(NEAR(c.x, 0, 1e-6f), "aabb center");
        CHECK(NEAR(aabb_volume(b), 8, 1e-6f), "aabb volume");
        CHECK(NEAR(cl.x, 1, 1e-6f), "aabb closest");
        CHECK(aabb_contains_point(b, vec3_new(0.5f, 0.5f, 0.5f)), "aabb contains");
        CHECK(!aabb_contains_point(b, vec3_new(1.5f, 0, 0)), "aabb not contains");
        CHECK(NEAR(tb.min.x, 0, 1e-6f), "aabb transform");
        CHECK(!aabb_is_valid(aabb_empty()), "aabb empty invalid");
        CHECK(aabb_overlaps(b, aabb_new(vec3_new(0, 0, 0), vec3_new(2, 2, 2))), "aabb overlaps");
    }
    /* ---- sphere ---- */
    {
        sphere s = sphere_new(vec3_new(0, 0, 0), 1);
        sphere m = sphere_merge(sphere_new(vec3_new(0, 0, 0), 1), sphere_new(vec3_new(3, 0, 0), 1));
        CHECK(sphere_contains_point(s, vec3_new(0, 1, 0)), "sphere contains");
        CHECK(!sphere_contains_point(s, vec3_new(0, 1.1f, 0)), "sphere not contains");
        CHECK(NEAR(m.center.x, 1.5f, 1e-5f) && NEAR(m.radius, 2.5f, 1e-5f), "sphere merge");
        CHECK(sphere_overlaps(s, sphere_new(vec3_new(1.5f, 0, 0), 1)), "sphere overlaps");
    }
    /* ---- plane ---- */
    {
        plane p = plane_from_point_normal(vec3_new(0, 0, 0), vec3_new(0, 0, 2));
        vec3 pr = plane_project_point(p, vec3_new(3, 4, 5));
        CHECK(NEAR(plane_distance_to_point(p, vec3_new(0, 0, 2)), 2, 1e-6f), "plane distance");
        CHECK(NEAR(pr.z, 0, 1e-6f), "plane project");
        { mat4 r = plane_reflect_matrix(p);
          vec3 q = mat4_transform_point(r, vec3_new(0, 0, 3));
          CHECK(NEAR(q.z, -3, 1e-5f) && NEAR(q.x, 0, 1e-5f), "plane reflect matrix"); }
        { mat4 m = mat4_mul(mat4_translate(vec3_new(0, 0, 5)), mat4_rotate_x(0.5f));
          plane p2 = plane_transform(p, m);
          vec3 pt = vec3_new(1, 2, 0);
          vec3 wpt = mat4_transform_point(m, pt);
          CHECK(NEAR(plane_distance_to_point(p2, wpt), 0, 1e-4f), "plane transform keeps points"); }
    }
    /* ---- ray ---- */
    {
        ray r = ray_new(vec3_new(0, 0, -5), vec3_new(0, 0, 1));
        aabb b = aabb_new(vec3_new(-1, -1, -1), vec3_new(1, 1, 1));
        real_t t0, t1;
        ray_hit hit;
        CHECK(intersect_ray_aabb(r, b, &t0, &t1), "ray-aabb hit");
        CHECK(NEAR(t0, 4, 1e-5f) && NEAR(t1, 6, 1e-5f), "ray-aabb t");
        CHECK(intersect_ray_sphere(r, sphere_new(vec3_zero(), 1), &hit), "ray-sphere hit");
        CHECK(NEAR(hit.t, 4, 1e-5f), "ray-sphere t");
        CHECK(intersect_ray_plane(r, plane_from_point_normal(vec3_zero(), vec3_new(0, 0, 1)), &hit), "ray-plane hit");
        CHECK(NEAR(hit.t, 5, 1e-5f) && NEAR(hit.point.z, 0, 1e-5f), "ray-plane point");
        { ray r2 = ray_new(vec3_new(0.25f, 0.25f, -1), vec3_new(0, 0, 1));
          CHECK(intersect_ray_triangle(r2, vec3_zero(), vec3_new(1, 0, 0), vec3_new(0, 1, 0), &hit), "ray-tri hit");
          CHECK(NEAR(hit.t, 1, 1e-5f) && NEAR(hit.point.x, 0.25f, 1e-5f), "ray-tri point"); }
        CHECK(intersect_sphere_aabb(sphere_new(vec3_new(2, 0, 0), 1.5f), b), "sphere-aabb");
        { vec3 cp = intersect_closest_point_triangle(vec3_zero(), vec3_new(1, 0, 0), vec3_new(0, 1, 0), vec3_new(0.25f, 0.25f, 2));
          CHECK(NEAR(cp.x, 0.25f, 1e-5f) && NEAR(cp.y, 0.25f, 1e-5f) && NEAR(cp.z, 0, 1e-5f), "closest tri"); }
    }
    /* ---- transform ---- */
    {
        transform t = transform_new(vec3_new(1, 2, 3), quat_from_euler(0.3f, 0.4f, 0.5f), vec3_new(2, 3, 4));
        vec3 p = vec3_new(0.5f, -1, 2);
        vec3 back = transform_inv_point(t, transform_point(t, p));
        CHECK(NEAR(back.x, p.x, 1e-4f) && NEAR(back.y, p.y, 1e-4f) && NEAR(back.z, p.z, 1e-4f), "transform inv point");
        { transform a = transform_new(vec3_new(1, 2, 3), quat_from_euler(0.1f, 0.2f, 0.3f), vec3_new(2, 2, 2));
          transform b = transform_new(vec3_new(-1, 0, 1), quat_from_euler(0.4f, 0, 0.2f), vec3_new(1, 2, 3));
          transform ab = transform_mul(a, b);
          mat4 m1 = transform_to_mat4(ab), m2 = mat4_mul(transform_to_mat4(a), transform_to_mat4(b));
          CHECK(mat4_is_equal_eps(m1, m2, 1e-4f), "transform mul == matrix mul"); }
        { transform uni = transform_new(vec3_new(1, 2, 3), quat_from_euler(0.3f, 0.4f, 0.5f), vec3_new(2, 2, 2));
          transform inv = transform_inverse(uni);
          mat4 m1 = transform_to_mat4(inv), m2 = mat4_inverse(transform_to_mat4(uni));
          CHECK(mat4_is_equal_eps(m1, m2, 1e-4f), "transform inverse(uniform) == matrix inverse"); }
        { mat4 mi = transform_inverse_matrix(t), mi2 = mat4_inverse(transform_to_mat4(t));
          CHECK(mat4_is_equal_eps(mi, mi2, 1e-4f), "transform inverse matrix exact"); }
    }
    /* ---- frustum ---- */
    {
        mat4 view = mat4_look_at(vec3_new(0, 0, -10), vec3_zero(), vec3_new(0, 1, 0));
        mat4 proj = mat4_perspective(1.0f, 1.0f, 0.1f, 100.0f);
        frustum f = frustum_from_matrix(mat4_mul(proj, view));
        vec3 corners[MATH_FRUSTUM_CORNER_COUNT];
        CHECK(frustum_is_valid(f), "frustum valid");
        CHECK(frustum_contains_point(f, vec3_zero()), "frustum contains origin");
        CHECK(!frustum_contains_point(f, vec3_new(100, 0, 0)), "frustum outside right");
        CHECK(!frustum_contains_point(f, vec3_new(0, 0, -15)), "frustum behind camera");
        CHECK(frustum_intersects_sphere(f, sphere_new(vec3_zero(), 1)), "frustum sphere");
        CHECK(!frustum_intersects_sphere(f, sphere_new(vec3_new(100, 0, 0), 1)), "frustum sphere outside");
        CHECK(frustum_intersects_aabb(f, aabb_new(vec3_new(-1, -1, -1), vec3_new(1, 1, 1))), "frustum aabb");
        CHECK(frustum_corners(f, corners), "frustum corners");
        { vec3 nz = mat4_transform_point(view, corners[0]);
          printf("[frustum] corner0 view=(%g,%g,%g)\n", nz.x, nz.y, nz.z);
          CHECK(NEAR(nz.z, 0.1f, 1e-3f), "frustum near corner z"); }
    }
    /* ---- color ---- */
    {
        color c = color_from_hsv(0.5f, 1.0f, 1.0f, 1.0f);
        real_t h, s, v;
        qty_t packed;
        color_to_hsv(c, &h, &s, &v);
        CHECK(NEAR(c.r, 0, 1e-5f) && NEAR(c.g, 1, 1e-5f) && NEAR(c.b, 1, 1e-5f), "hsv cyan");
        CHECK(NEAR(h, 0.5f, 1e-4f) && NEAR(s, 1, 1e-4f) && NEAR(v, 1, 1e-4f), "hsv roundtrip");
        packed = color_to_rgba8(color_new(1, 0.5f, 0, 1));
        CHECK(packed == 0xFF8000FFULL, "rgba8 pack");
        CHECK(NEAR(color_rgba8(packed).g, 128.0f / 255.0f, 1e-6f), "rgba8 unpack");
        { color lin = color_srgb_to_linear(color_new(0.5f, 0.5f, 0.5f, 1));
          color back = color_linear_to_srgb(lin);
          CHECK(NEAR(back.r, 0.5f, 1e-4f), "srgb roundtrip"); }
    }
    /* ---- half ---- */
    {
        CHECK(half_from_float(1.0f) == 0x3C00u, "half 1.0");
        CHECK(half_from_float(-2.0f) == 0xC000u, "half -2.0");
        CHECK(NEAR(half_to_float(half_from_float(1.0f / 3.0f)), 1.0f / 3.0f, 1e-3f), "half roundtrip");
        CHECK(half_from_float(1e9f) == 0x7C00u, "half overflow inf");
        CHECK(NEAR(half_to_float(0x0001u), 5.9604645e-08f, 1e-12f), "half denormal");
    }
    /* ---- fixed ---- */
    {
        fixed_t a = fixed_from_float(1.5f), b = fixed_from_float(2.0f);
        CHECK(a == FIXED_ONE + FIXED_ONE / 2, "fixed from float");
        CHECK(NEAR(fixed_to_float(fixed_mul(a, b)), 3.0f, 1e-4f), "fixed mul");
        CHECK(NEAR(fixed_to_float(fixed_div(fixed_from_int(3), a)), 2.0f, 1e-4f), "fixed div");
        CHECK(NEAR(fixed_to_float(fixed_sqrt(fixed_from_int(4))), 2.0f, 1e-3f), "fixed sqrt");
    }
    /* ---- random ---- */
    {
        random r1, r2;
        int i, same = 1;
        qty_t lo = 100, hi = 0;
        random_seed(&r1, 42, 1);
        random_seed(&r2, 42, 1);
        for (i = 0; i < 1000; i++) {
            qty_t x = random_next(&r1), y = random_next(&r2);
            if (x != y) same = 0;
            if (x < lo) lo = x;
            if (x > hi) hi = x;
        }
        CHECK(same, "random deterministic");
        CHECK(lo != hi, "random spreads");
        { random r; real_t sum = 0; int k; random_seed(&r, 7, 7);
          for (k = 0; k < 10000; k++) sum += random_float(&r);
          printf("[random] mean=%g (expect ~0.5)\n", sum / 10000.0f);
          CHECK(NEAR(sum / 10000.0f, 0.5f, 0.02f), "random float mean"); }
        { vec3 u = random_unit_vec3(&r1); printf("[random] unit len=%g\n", vec3_length(u));
          CHECK(NEAR(vec3_length(u), 1, 1e-5f), "random unit vec len"); }
    }
    /* ---- noise ---- */
    {
        noise n; int i, inrange = 1;
        real_t amin = 1e9f, amax = -1e9f;
        n.seed = 1234;
        for (i = 0; i < 1000; i++) {
            real_t x = (real_t)i * 0.37f, y = (real_t)i * 0.11f;
            real_t p = noise_perlin_2d(&n, x, y), f = noise_fbm_2d(&n, x, y, 4, 2.0f, 0.5f);
            if (p < -1.2f || p > 1.2f || f < -1.2f || f > 1.2f) inrange = 0;
            if (p < amin) amin = p;
            if (p > amax) amax = p;
        }
        printf("[noise] perlin range [%g, %g]\n", amin, amax);
        CHECK(inrange, "noise in range");
        CHECK(NEAR(noise_perlin_2d(&n, 3.0f, 7.0f), 0, 1e-5f), "perlin lattice zero");
        CHECK(NEAR(noise_value_2d(&n, 3.0f, 7.0f), noise_value_2d(&n, 3.0f, 7.0f), 1e-9f), "noise stable");
    }
    /* ---- curve ---- */
    {
        CHECK(NEAR(curve_bezier_cubic(0, 1, 2, 3, 0), 0, 1e-6f), "bezier start");
        CHECK(NEAR(curve_bezier_cubic(0, 1, 2, 3, 1), 3, 1e-6f), "bezier end");
        CHECK(NEAR(curve_catmull_rom(0, 1, 2, 3, 0), 1, 1e-6f), "catmull start");
        CHECK(NEAR(curve_catmull_rom(0, 1, 2, 3, 1), 2, 1e-6f), "catmull end");
        CHECK(NEAR(curve_ease_out_quad(1), 1, 1e-6f) && NEAR(curve_ease_in_expo(0), 0, 1e-6f), "ease endpoints");
        { vec3 p = curve_bezier_cubic_v3(vec3_zero(), vec3_new(1, 0, 0), vec3_new(2, 0, 0), vec3_new(3, 0, 0), 0.5f);
          CHECK(NEAR(p.x, 1.5f, 1e-6f), "bezier v3"); }
    }

    printf("\n%s (%d failures)\n", fails ? "FAIL" : "ALL PASS", fails);
    return fails ? 1 : 0;
}