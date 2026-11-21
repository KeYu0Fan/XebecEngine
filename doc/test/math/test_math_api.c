#include <stdio.h>
#include <math.h>
#include "core/math/math.h"

static int fails = 0;
#define CHECK(c, m) do { if (!(c)) { printf("FAIL: %s\n", m); fails++; } } while (0)
#define NEAR(a, b, eps) (fabs((double)((a) - (b))) <= (eps))

int main(void)
{
    /* 标量新增 */
    CHECK(NEAR(math_angle_delta(math_radians(350), math_radians(10)), math_radians(20), 1e-5f), "angle_delta wrap");
    CHECK(NEAR(math_lerp_angle(math_radians(350), math_radians(10), 0.5f), math_radians(360), 1e-4f), "lerp_angle shortest");
    CHECK(NEAR(math_smootherstep(0), 0, 1e-6f) && NEAR(math_smootherstep(1), 1, 1e-6f) && NEAR(math_smootherstep(0.5f), 0.5f, 1e-6f), "smootherstep");
    CHECK(NEAR(math_remap_clamped(20, 0, 10, 0, 1), 1, 1e-6f), "remap_clamped");
    CHECK(math_is_pow2(64) && !math_is_pow2(0) && !math_is_pow2(65), "is_pow2");
    CHECK(math_iclamp(5, 0, 3) == 3 && math_iclamp(-1, 0, 3) == 0, "iclamp");
    CHECK(NEAR(math_hypot(3, 4), 5, 1e-6f), "hypot");
    CHECK(NEAR(math_copysign(3, -1), -3, 1e-6f), "copysign");

    /* 向量新增 */
    {
        vec2 s = vec2_slerp(vec2_new(1, 0), vec2_new(0, 2), 0.5f);
        printf("[vec2] slerp=(%g,%g) len=%g\n", s.x, s.y, vec2_length(s));
        CHECK(NEAR(vec2_angle_of(s), MATH_QUARTER_PI, 1e-5f), "vec2 slerp angle");
        CHECK(NEAR(vec2_length(s), 1.5f, 1e-5f), "vec2 slerp length");
        { vec2 r = vec2_rotate_towards(vec2_new(1, 0), vec2_new(0, 1), 0.1f);
          CHECK(NEAR(vec2_angle_of(r), 0.1f, 1e-5f), "vec2 rotate_towards"); }
        CHECK(vec2_is_equal(vec2_round(vec2_new(1.4f, -1.6f)), vec2_new(1, -2)), "vec2 round");
        CHECK(vec2_is_equal(vec2_floor(vec2_new(1.4f, -1.6f)), vec2_new(1, -2)), "vec2 floor");
        CHECK(vec2_is_equal(vec2_ceil(vec2_new(1.4f, -1.6f)), vec2_new(2, -1)), "vec2 ceil");
        CHECK(vec2_is_equal(vec2_snap(vec2_new(1.26f, 1.26f), 0.5f), vec2_new(1.5f, 1.5f)), "vec2 snap");
        { vec2 c = vec2_clamp_length(vec2_new(10, 0), 2);
          CHECK(NEAR(vec2_length(c), 2, 1e-5f), "vec2 clamp_length"); }
    }
    {
        CHECK(NEAR(vec3_signed_angle(vec3_new(1, 0, 0), vec3_new(0, 1, 0), vec3_new(0, 0, 1)), MATH_HALF_PI, 1e-5f), "vec3 signed angle +");
        CHECK(NEAR(vec3_signed_angle(vec3_new(1, 0, 0), vec3_new(0, -1, 0), vec3_new(0, 0, 1)), -MATH_HALF_PI, 1e-5f), "vec3 signed angle -");
        { vec3 p = vec3_project_on_plane(vec3_new(1, 2, 3), vec3_new(0, 0, 1));
          CHECK(vec3_is_equal(p, vec3_new(1, 2, 0)), "vec3 project on plane"); }
        CHECK(vec3_is_equal(vec3_floor(vec3_new(1.5f, -0.5f, 2.9f)), vec3_new(1, -1, 2)), "vec3 floor");
        { vec3 c = vec3_clamp_length(vec3_new(10, 0, 0), 3);
          CHECK(NEAR(vec3_length(c), 3, 1e-5f), "vec3 clamp_length"); }
    }
    CHECK(vec4_is_normalized(vec4_new(0, 1, 0, 0)) && !vec4_is_normalized(vec4_new(0, 2, 0, 0)), "vec4 normalized");
    CHECK(vec4_is_equal(vec4_from_vec3_w(vec3_new(1, 2, 3), 4), vec4_new(1, 2, 3, 4)), "vec4 from vec3 w");

    /* 矩阵新增 */
    {
        mat3 m = mat3_rotate_z(0.7f);
        CHECK(NEAR(mat2_to_angle(mat2_rotate(0.7f)), 0.7f, 1e-5f), "mat2 to angle");
        { mat2 o = mat2_orthonormalize(mat2_new(2, 0.1f, 0.2f, 1));
          CHECK(mat2_is_rotation(o), "mat2 orthonormalize"); }
        CHECK(mat3_is_equal_eps(m, mat3_rotate_euler(vec3_new(0, 0, 0.7f)), 1e-5f), "mat3 rotate_euler z");
        CHECK(mat3_is_equal_eps(m, mat3_rotate_euler(mat3_to_euler(m)), 1e-5f), "mat3 euler roundtrip");
        { mat3 e = mat3_rotate_euler(vec3_new(0.3f, 0.5f, 0.7f));
          mat3 e4 = mat4_to_mat3(mat4_rotate_euler(vec3_new(0.3f, 0.5f, 0.7f)));
          CHECK(mat3_is_equal_eps(e, e4, 1e-5f), "mat3 euler vs mat4 euler"); }
        CHECK(mat4_is_rotation(mat4_rotate_x(0.4f)) && !mat4_is_rotation(mat4_scale(vec3_new(2, 1, 1))), "mat4 is rotation");
        { mat4 o = mat4_orthonormalize(mat4_mul(mat4_translate(vec3_new(1, 2, 3)), mat4_scale(vec3_new(2, 3, 4))));
          CHECK(mat3_is_rotation(mat4_to_mat3(o)), "mat4 orthonormalize");
          CHECK(NEAR(o.m[0][3], 1, 1e-6f) && NEAR(o.m[1][3], 2, 1e-6f), "mat4 orthonormalize keeps translation"); }
    }

    /* 四元数新增 */
    {
        quat q = quat_from_axis_angle(vec3_new(0, 0, 1), 0.5f);
        quat sw, tw;
        CHECK(quat_is_equal_eps(q, q, 1e-6f), "quat equal eps");
        CHECK(quat_swing_twist(q, vec3_new(0, 0, 1), &sw, &tw), "swing twist ok");
        CHECK(quat_is_equal_eps(tw, q, 1e-4f) && quat_is_equal_eps(sw, quat_identity(), 1e-4f), "swing twist z");
        { quat a = quat_identity(), b = quat_from_axis_angle(vec3_new(0, 0, 1), 1.0f);
          quat sa = quat_squad_control(b, a, b), sb = quat_squad_control(a, b, a);
          CHECK(quat_is_equal_eps(quat_squad(a, b, sa, sb, 0), a, 1e-4f), "squad t0");
          CHECK(quat_is_equal_eps(quat_squad(a, b, sa, sb, 1), b, 1e-4f), "squad t1");
          printf("[quat] squad mid angle=%g\n", quat_angle_between(a, quat_squad(a, b, sa, sb, 0.5f))); }
        { quat l = quat_log(quat_from_axis_angle(vec3_new(1, 0, 0), 0.8f));
          quat e = quat_exp(l);
          CHECK(quat_is_equal_eps(e, quat_from_axis_angle(vec3_new(1, 0, 0), 0.8f), 1e-4f), "log/exp roundtrip"); }
    }

    /* 几何新增 */
    {
        plane pz = plane_from_point_normal(vec3_zero(), vec3_new(0, 0, 1));
        plane px = plane_from_point_normal(vec3_zero(), vec3_new(1, 0, 0));
        vec3 lp, ld;
        CHECK(plane_intersect_plane(pz, px, &lp, &ld), "plane intersect");
        CHECK(NEAR(lp.x, 0, 1e-5f) && NEAR(lp.z, 0, 1e-5f), "plane line point");
        CHECK(NEAR(fabs(ld.y), 1, 1e-5f), "plane line dir");
        CHECK(!plane_intersect_plane(pz, pz, &lp, &ld), "plane parallel");
    }
    {
        mat4 view = mat4_look_at(vec3_new(0, 0, -10), vec3_zero(), vec3_new(0, 1, 0));
        frustum f = frustum_from_matrix(mat4_mul(mat4_perspective(1.0f, 1.0f, 0.1f, 100.0f), view));
        aabb small = aabb_new(vec3_new(-1, -1, -1), vec3_new(1, 1, 1));
        aabb big = aabb_new(vec3_new(-100, -100, -100), vec3_new(100, 100, 100));
        CHECK(frustum_contains_aabb(f, small), "frustum contains aabb");
        CHECK(!frustum_contains_aabb(f, big), "frustum not contains big");
        CHECK(frustum_intersects_aabb(f, big), "frustum intersects big");
    }

    /* 工具新增 */
    {
        color c;
        CHECK(color_from_hex("#FF8000FF", &c), "hex 8");
        CHECK(NEAR(c.r, 1, 1e-6f) && NEAR(c.g, 128.0f / 255.0f, 1e-6f) && NEAR(c.a, 1, 1e-6f), "hex values");
        CHECK(color_from_hex("#f80", &c) && NEAR(c.r, 1, 1e-6f) && NEAR(c.g, 136.0f / 255.0f, 1e-6f), "hex short");
        CHECK(!color_from_hex("#GG0000", &c), "hex invalid");
        { char buf[MATH_COLOR_HEX_STRING_SIZE]; color v = color_new(1, 0.5f, 0, 1);
          CHECK(color_to_hex(v, buf, sizeof(buf)), "to hex");
          printf("[color] hex=%s\n", buf);
          CHECK(buf[0] == '#' && buf[1] == 'F' && buf[2] == 'F' && buf[3] == '8', "to hex digits"); }
        { fixed_t v = fixed_from_float(2.5f);
          CHECK(fixed_clamp(v, 0, FIXED_ONE) == FIXED_ONE, "fixed clamp");
          CHECK(fixed_sign(-v) == -FIXED_ONE && fixed_sign(0) == 0, "fixed sign"); }
        { random r; vec2 u; random_seed(&r, 5, 5); u = random_unit_vec2(&r); printf("[random] unit2 len=%g\n", vec2_length(u));
          CHECK(NEAR(vec2_length(u), 1, 1e-5f), "random unit vec2"); }
        { random r; vec3 p; random_seed(&r, 6, 6); p = random_in_box(&r, vec3_new(1, 2, 3), vec3_new(0.5f, 0.5f, 0.5f));
          CHECK(p.x >= 0.5f && p.x <= 1.5f && p.y >= 1.5f && p.y <= 2.5f, "random in box"); }
        CHECK(NEAR(curve_ease_in_back(0), 0, 1e-6f) && NEAR(curve_ease_in_back(1), 1, 1e-6f), "ease in back");
        CHECK(NEAR(curve_ease_inout_back(0), 0, 1e-6f) && NEAR(curve_ease_inout_back(1), 1, 1e-6f), "ease inout back");
        CHECK(NEAR(curve_ease_in_bounce(0), 0, 1e-6f) && NEAR(curve_ease_in_bounce(1), 1, 1e-6f), "ease in bounce");
        CHECK(NEAR(curve_ease_inout_bounce(0), 0, 1e-6f) && NEAR(curve_ease_inout_bounce(1), 1, 1e-6f), "ease inout bounce");
        CHECK(NEAR(curve_ease_inout_expo(0), 0, 1e-6f) && NEAR(curve_ease_inout_expo(1), 1, 1e-6f), "ease inout expo");
        CHECK(NEAR(curve_ease_in_elastic(0), 0, 1e-6f) && NEAR(curve_ease_in_elastic(1), 1, 1e-6f), "ease in elastic");
        CHECK(NEAR(curve_ease_out_elastic(0), 0, 1e-6f) && NEAR(curve_ease_out_elastic(1), 1, 1e-6f), "ease out elastic");
    }

    printf("\n%s (%d failures)\n", fails ? "FAIL" : "ALL PASS", fails);
    return fails ? 1 : 0;
}