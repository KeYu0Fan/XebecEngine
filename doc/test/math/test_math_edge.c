#include <stdio.h>
#include <math.h>
#include "core/math/math.h"

static int fails = 0;
#define CHECK(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); fails++; } } while (0)
#define NEAR(a, b, eps) (fabs((double)((a) - (b))) <= (eps))

static unsigned int rng = 7u;
static float fr(void) { rng = rng * 1664525u + 1013904223u; return (float)((rng >> 8) & 0xFFFF) / 65535.0f; }

int main(void)
{
    /* 朝向基: 局部 +Z 对准 forward, +Y 尽量对准 up */
    {
        quat q = quat_from_look_rotation(vec3_new(1, 0, 0), vec3_new(0, 1, 0));
        vec3 f = quat_mul_vec3(q, vec3_new(0, 0, 1));
        vec3 u = quat_mul_vec3(q, vec3_new(0, 1, 0));
        printf("[look] fwd=(%g,%g,%g) up=(%g,%g,%g)\n", f.x, f.y, f.z, u.x, u.y, u.z);
        CHECK(NEAR(f.x, 1, 1e-5f) && NEAR(f.y, 0, 1e-5f) && NEAR(f.z, 0, 1e-5f), "look forward");
        CHECK(NEAR(u.y, 1, 1e-5f), "look up");
    }
    /* 欧拉万向锁 */
    {
        quat q = quat_from_euler(0.4f, MATH_HALF_PI, 0.3f);
        vec3 e = quat_to_euler(q);
        quat q2 = quat_from_euler(e.x, e.y, e.z);
        mat3 a = quat_to_mat3(q), b = quat_to_mat3(q2);
        float err = 0; int i, j;
        for (i = 0; i < 3; i++) for (j = 0; j < 3; j++) { float d = (float)fabs(a.m[i][j] - b.m[i][j]); if (d > err) err = d; }
        printf("[euler] gimbal roundtrip err=%g euler=(%g,%g,%g)\n", err, e.x, e.y, e.z);
        CHECK(err < 2e-3f, "euler gimbal roundtrip");
    }
    /* 正交化 */
    {
        mat3 m = mat3_new(1, 0.2f, 0.1f, 0.3f, 1, 0.25f, 0.05f, 0.1f, 1);
        mat3 o = mat3_orthonormalize(m);
        CHECK(mat3_is_orthogonal(o), "orthonormalize orthogonal");
        CHECK(NEAR(mat3_determinant(o), 1, 1e-4f), "orthonormalize det");
    }
    /* 向量插值反向 */
    {
        vec3 s = vec3_slerp(vec3_new(1, 0, 0), vec3_new(-1, 0, 0), 0.5f);
        printf("[vec3] slerp opposite=(%g,%g,%g) len=%g\n", s.x, s.y, s.z, vec3_length(s));
        CHECK(NEAR(vec3_length(s), 1, 1e-4f), "slerp opposite len");
        CHECK(NEAR(vec3_dot(s, vec3_new(1, 0, 0)), 0, 1e-4f), "slerp opposite perpendicular");
    }
    /* quat 旋转逼近 */
    {
        quat cur = quat_identity(), tg = quat_from_axis_angle(vec3_new(0, 0, 1), 1.0f);
        quat n = quat_rotate_towards(cur, tg, 0.25f);
        real_t ang = quat_angle_between(cur, n);
        printf("[quat] rotate_towards step=%g\n", ang);
        CHECK(NEAR(ang, 0.25f, 1e-4f), "rotate_towards step");
        CHECK(quat_is_equal(quat_rotate_towards(cur, tg, 2.0f), tg), "rotate_towards reach");
    }
    /* 半精度随机往返误差 */
    {
        int i; float worst = 0;
        for (i = 0; i < 2000; i++) {
            float x = (fr() * 2.0f - 1.0f) * 1000.0f;
            float y = half_to_float(half_from_float(x));
            float tol = fmaxf(0.001f * fabsf(x), 6e-5f);
            float e = fabsf(x - y) - tol;
            if (e > worst) worst = e;
        }
        printf("[half] worst excess over tolerance=%g\n", worst);
        CHECK(worst <= 0.0f, "half roundtrip within half precision");
        CHECK(half_to_float(half_from_float(0.0f)) == 0.0f, "half zero");
        CHECK(half_from_float(-0.0f) == 0x8000u, "half negative zero");
    }
    /* 颜色 hues */
    {
        color r = color_from_hsv(0.0f, 1, 1, 1), g = color_from_hsv(1.0f / 3.0f, 1, 1, 1), b = color_from_hsv(2.0f / 3.0f, 1, 1, 1);
        CHECK(NEAR(r.r, 1, 1e-4f) && NEAR(r.g, 0, 1e-4f), "hsv red");
        CHECK(NEAR(g.g, 1, 1e-4f) && NEAR(g.r, 0, 1e-4f), "hsv green");
        CHECK(NEAR(b.b, 1, 1e-4f) && NEAR(b.r, 0, 1e-4f), "hsv blue");
        CHECK(color_to_bgra8(color_new(1, 0, 0, 1)) == 0x0000FFFFULL, "bgra8");
        CHECK(NEAR(color_luminance(color_white()), 1, 1e-5f), "luminance");
    }
    /* 投影: 视锥远平面角点 */
    {
        mat4 view = mat4_look_at(vec3_new(0, 0, -10), vec3_zero(), vec3_new(0, 1, 0));
        mat4 proj = mat4_perspective(1.0f, 1.0f, 0.1f, 100.0f);
        frustum f = frustum_from_matrix(mat4_mul(proj, view));
        vec3 c[MATH_FRUSTUM_CORNER_COUNT];
        CHECK(frustum_corners(f, c), "corners");
        { vec3 v0 = mat4_transform_point(view, c[0]), v4 = mat4_transform_point(view, c[4]);
          printf("[frustum] near z=%g far z=%g\n", v0.z, v4.z);
          CHECK(NEAR(v0.z, 0.1f, 1e-3f), "near plane z");
          CHECK(NEAR(v4.z, 100.0f, 1e-2f), "far plane z"); }
    }
    /* transform: 非均匀缩放下的点逆变换 */
    {
        transform t = transform_new(vec3_new(3, -2, 1), quat_from_axis_angle(vec3_normalize(vec3_new(1, 2, 3)), 0.8f), vec3_new(2, 5, 0.5f));
        vec3 p = vec3_new(1, 2, 3), back = transform_inv_point(t, transform_point(t, p));
        printf("[transform] non-uniform roundtrip=(%g,%g,%g)\n", back.x, back.y, back.z);
        CHECK(NEAR(back.x, p.x, 1e-3f) && NEAR(back.y, p.y, 1e-3f) && NEAR(back.z, p.z, 1e-3f), "non-uniform inv point");
        { mat4 m = transform_inverse_matrix(t);
          vec3 q = mat4_transform_point(m, transform_point(t, p));
          CHECK(NEAR(q.x, p.x, 1e-3f) && NEAR(q.y, p.y, 1e-3f) && NEAR(q.z, p.z, 1e-3f), "inverse matrix roundtrip"); }
    }
    /* 定点数精度 */
    {
        fixed_t a = fixed_from_float(1.0f / 3.0f);
        printf("[fixed] 1/3 -> %g\n", fixed_to_float(a));
        CHECK(NEAR(fixed_to_float(a), 1.0f / 3.0f, 1e-3f), "fixed 1/3");
        CHECK(fixed_div(FIXED_ONE, 0) == 0, "fixed div zero");
    }
    /* 噪声: 值噪声在整数格点为对称分布且 fbm 无越界 */
    {
        noise n; int i; real_t mn = 1e9f, mx = -1e9f;
        n.seed = 99;
        for (i = 0; i < 4000; i++) {
            real_t v = noise_fbm_3d(&n, fr() * 10, fr() * 10, fr() * 10, 5, 2.0f, 0.5f);
            if (v < mn) mn = v;
            if (v > mx) mx = v;
        }
        printf("[noise] fbm3 range [%g, %g]\n", mn, mx);
        CHECK(mn >= -1.0001f && mx <= 1.0001f, "fbm3 range");
    }
    /* 随机: 单位球内 */
    {
        random r; int i, inside = 1; random_seed(&r, 3, 9);
        for (i = 0; i < 2000; i++) if (vec3_length_sq(random_in_unit_sphere(&r)) > 1.0001f) inside = 0;
        CHECK(inside, "random in unit sphere");
        CHECK(NEAR(vec2_length(random_in_unit_circle(&r)), 1, 1e-3f) || vec2_length(random_in_unit_circle(&r)) <= 1.0f, "random in unit circle");
    }

    /* NDC 约定转换: Vulkan -> OpenGL */
    {
        mat4 p = mat4_perspective(1.0f, 1.0f, 0.1f, 100.0f);
        mat4 g = mat4_ndc_to_gl(p);
        vec4 n = mat4_mul_vec4(g, vec4_new(0, 0, 0.1f, 1));
        vec4 f = mat4_mul_vec4(g, vec4_new(0, 0, 100.0f, 1));
        vec4 up = mat4_mul_vec4(g, vec4_new(0, 1, 10, 1));
        printf("[ndc] gl near z=%g far z=%g y(up)=%g\n", n.z / n.w, f.z / f.w, up.y / up.w);
        CHECK(NEAR(n.z / n.w, -1, 1e-4f) && NEAR(f.z / f.w, 1, 1e-4f), "ndc z to gl");
        CHECK(up.y / up.w > 0, "ndc y flip");
    }

    printf("\n%s (%d failures)\n", fails ? "FAIL" : "ALL PASS", fails);
    return fails ? 1 : 0;
}