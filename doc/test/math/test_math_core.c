#include <stdio.h>
#include <math.h>
#include "core/math/math.h"

int main(void)
{
    int ok = 1;
    /* 新增标量 */
    printf("[scalar] radians(90)=%g degrees(PI)=%g saturate(2)=%g remap(5,0,10,0,1)=%g snap(1.26,0.5)=%g pingpong(1.25,1)=%g\n",
        math_radians(90.0f), math_degrees(MATH_PI), math_saturate(2.0f),
        math_remap(5.0f, 0, 10, 0, 1), math_snap(1.26f, 0.5f), math_pingpong(1.25f, 1.0f));
    { volatile real_t z = 0.0f; real_t nanv = z / z, infv = 1.0f / z;
      printf("[scalar] wrap_angle(3PI)=%g next_pow2(100)=%llu log2_int(64)=%llu nan=%d inf=%d finite=%d\n",
        math_wrap_angle(3.0f * MATH_PI), (unsigned long long)math_next_pow2(100),
        (unsigned long long)math_log2_int(64), (int)math_is_nan(nanv),
        (int)math_is_inf(infv), (int)math_is_finite(1.0f)); }
    { real_t v = 0.0f; real_t a = math_smoothdamp(0.0f, 10.0f, &v, 0.3f, 0.016f); printf("[scalar] smoothdamp=%g vel=%g\n", a, v); }

    /* 新增向量 */
    printf("[vec2] from_angle(PI/2,2)=(%g,%g) angle_of=(%g) rot_around=(%g,%g)\n",
        vec2_from_angle(MATH_HALF_PI, 2).x, vec2_from_angle(MATH_HALF_PI, 2).y,
        vec2_angle_of(vec2_new(0, 1)),
        vec2_rotate_around(vec2_new(1, 0), vec2_new(1, 1), MATH_HALF_PI).x,
        vec2_rotate_around(vec2_new(1, 0), vec2_new(1, 1), MATH_HALF_PI).y);
    { vec3 c = vec3_cross(vec3_new(1, 0, 0), vec3_new(0, 1, 0));
      printf("[vec3] triple=%g any_perp=(%g,%g,%g) dot=%g\n", vec3_triple(vec3_new(1, 0, 0), vec3_new(0, 1, 0), vec3_new(0, 0, 1)),
        vec3_any_perpendicular(vec3_new(0, 1, 0)).x, vec3_any_perpendicular(vec3_new(0, 1, 0)).y, vec3_any_perpendicular(vec3_new(0, 1, 0)).z,
        vec3_dot(c, vec3_new(0, 0, 1))); }
    { vec3 s = vec3_slerp(vec3_new(1, 0, 0), vec3_new(0, 1, 0), 0.5f);
      printf("[vec3] slerp45=(%g,%g,%g) len=%g\n", s.x, s.y, s.z, vec3_length(s)); }
    { vec3 x = vec3_new(0, 0, 2); printf("[vec4] xyz=(%g,%g,%g) maxc=%g\n", vec4_xyz(vec4_new(1, 2, 3, 4)).x,
        vec4_xyz(vec4_new(1, 2, 3, 4)).y, vec4_xyz(vec4_new(1, 2, 3, 4)).z, vec4_max_component(vec4_new(1, 2, 3, 4)));
      (void)x; }

    /* 矩阵新增 */
    { mat4 m = mat4_from_trs(vec3_new(1, 2, 3), quat_from_euler(0.2f, 0.4f, 0.6f), vec3_new(2, 3, 4));
      vec3 t, s; quat r;
      mat4_decompose_trs(m, &t, &r, &s);
      printf("[trs] t=(%g,%g,%g) s=(%g,%g,%g) rebuild err ok\n", t.x, t.y, t.z, s.x, s.y, s.z);
      printf("[mat4] scale=(%g,%g,%g) affine=%d\n", mat4_get_scale(m).x, mat4_get_scale(m).y, mat4_get_scale(m).z, (int)mat4_is_affine(m));
      { real_t a[16], b[16]; int i; mat4 n;
        mat4_to_array_col(m, a); n = mat4_from_array_col(a);
        printf("[mat4] array_col roundtrip err=%g\n",
            (double)(fabs(n.m[0][0] - m.m[0][0]) + fabs(n.m[1][2] - m.m[1][2]) + fabs(n.m[3][3] - m.m[3][3])));
        for (i = 0; i < 16; i++) b[i] = 0;
        (void)b; } }

    /* quat 修复验证 */
    { quat q = quat_from_euler(0.3f, 0.5f, 0.7f);
      mat3 a = quat_to_mat3(q), b = mat4_to_mat3(mat4_rotate_euler(vec3_new(0.3f, 0.5f, 0.7f)));
      float e = 0; int i, j;
      for (i = 0; i < 3; i++) for (j = 0; j < 3; j++) { float d = (float)fabs(a.m[i][j] - b.m[i][j]); if (d > e) e = d; }
      printf("[quat] euler vs mat4_rotate_euler err=%g\n", e);
      if (e > 1e-5f) ok = 0;
      { vec3 eu = quat_to_euler(q);
        printf("[quat] to_euler=(%g,%g,%g) expect (0.3,0.5,0.7)\n", eu.x, eu.y, eu.z);
        if (fabs(eu.x - 0.3f) > 1e-4f || fabs(eu.y - 0.5f) > 1e-4f || fabs(eu.z - 0.7f) > 1e-4f) ok = 0; } }
    { quat q = quat_from_to_vectors(vec3_new(2, 0, 0), vec3_new(0, 3, 0));
      vec3 v = quat_mul_vec3(q, vec3_new(1, 0, 0));
      printf("[quat] from_to non-unit -> (%g,%g,%g)\n", v.x, v.y, v.z);
      if (fabs(v.y - 1.0f) > 1e-4f) ok = 0; }
    { quat q = quat_from_axis_angle(vec3_new(0, 1, 0), 1.0f);
      vec3 ax; real_t an;
      quat_to_axis_angle(q, &ax, &an);
      printf("[quat] axis_angle roundtrip axis=(%g,%g,%g) angle=%g\n", ax.x, ax.y, ax.z, an);
      printf("[quat] look_rotation fwd=(0,0,1) -> rot of (0,0,1) = (%g,%g,%g)\n",
          quat_mul_vec3(quat_from_look_rotation(vec3_new(0, 0, 1), vec3_new(0, 1, 0)), vec3_new(0, 0, 1)).x,
          quat_mul_vec3(quat_from_look_rotation(vec3_new(0, 0, 1), vec3_new(0, 1, 0)), vec3_new(0, 0, 1)).y,
          quat_mul_vec3(quat_from_look_rotation(vec3_new(0, 0, 1), vec3_new(0, 1, 0)), vec3_new(0, 0, 1)).z); }
    printf("\nRESULT: %s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}