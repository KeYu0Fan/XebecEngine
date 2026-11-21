#ifndef MATH_GEOMETRY_FRUSTUM_H
/* 视锥体
 * 由 6 个平面组成, 顺序固定: 0 近 1 远 2 左 3 右 4 上 5 下
 * 存储: 6 个 16 字节平面连续排列(共 96 字节), 与矩阵的行连续布局一致
 *       每个平面用第 4 个分量填充(同 vec3 的 w), 便于按 16 字节步长做 SIMD 访问
 * 平面朝内(法线指向视锥内部), 与 mat4 的 Vulkan NDC 约定一致:
 *   NDC 范围 x,y ∈ [-1,1], z ∈ [0,1], 相机空间手系由投影矩阵决定
 */
#define MATH_GEOMETRY_FRUSTUM_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/math/constants.h"
#include "core/math/vector/vector.h"
#include "core/math/matrix/mat4.h"
#include "core/math/geometry/plane.h"
#include "core/math/geometry/aabb.h"
#include "core/math/geometry/sphere.h"
EXTERN_C_BEGIN


/* 视锥体: 6 个平面, 按 16 字节步长连续排列 */
typedef struct { plane planes[MATH_FRUSTUM_PLANE_COUNT]; } frustum;

/* ---- 构造 ---- */
/* 由 视图 * 投影 矩阵提取 6 个平面(Gribb-Hartmann)
 * 注意: 矩阵约定为列向量 M*v 且 NDC z ∈ [0,1]; OpenGL 的 z ∈ [-1,1] 需另做处理 */
static FORCE_INLINE frustum frustum_from_matrix(mat4 m) {
    frustum f;
    vec4 r0 = vec4_new(m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3]);
    vec4 r1 = vec4_new(m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3]);
    vec4 r2 = vec4_new(m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3]);
    vec4 r3 = vec4_new(m.m[3][0], m.m[3][1], m.m[3][2], m.m[3][3]);
    vec4 p[MATH_FRUSTUM_PLANE_COUNT];
    qty_t i;
    p[MATH_FRUSTUM_NEAR] = r2;                 /* 近: z >= 0 */
    p[MATH_FRUSTUM_FAR] = vec4_sub(r3, r2);    /* 远: z <= w */
    p[MATH_FRUSTUM_LEFT] = vec4_add(r3, r0);   /* 左: x >= -w */
    p[MATH_FRUSTUM_RIGHT] = vec4_sub(r3, r0);  /* 右: x <= w */
    p[MATH_FRUSTUM_TOP] = vec4_sub(r3, r1);    /* 上: y <= w */
    p[MATH_FRUSTUM_BOTTOM] = vec4_add(r3, r1); /* 下: y >= -w */
    for (i = 0; i < MATH_FRUSTUM_PLANE_COUNT; i++) {
        real_t len = math_sqrt(p[i].x * p[i].x + p[i].y * p[i].y + p[i].z * p[i].z);
        if (len < MATH_EPSILON) f.planes[i] = plane_empty();
        else f.planes[i] = plane_from_vec4(vec4_new(p[i].x / len, p[i].y / len, p[i].z / len, p[i].w / len));
    }
    return f;
}
/* 是否 6 个平面都有效 */
static FORCE_INLINE bool_t frustum_is_valid(frustum f) {
    qty_t i;
    for (i = 0; i < MATH_FRUSTUM_PLANE_COUNT; i++) if (!plane_is_valid(f.planes[i])) return 0;
    return 1;
}

/* ---- 剔除测试 ---- */
/* 点是否在视锥内(含边界) */
static FORCE_INLINE bool_t frustum_contains_point(frustum f, vec3 p) {
    qty_t i;
    if (!frustum_is_valid(f)) return 0;
    for (i = 0; i < MATH_FRUSTUM_PLANE_COUNT; i++) if (plane_distance_to_point(f.planes[i], p) < 0.0f) return 0;
    return 1;
}
/* 球是否完全在视锥内 */
static FORCE_INLINE bool_t frustum_contains_sphere(frustum f, sphere s) {
    qty_t i;
    if (!frustum_is_valid(f) || !sphere_is_valid(s)) return 0;
    for (i = 0; i < MATH_FRUSTUM_PLANE_COUNT; i++) if (plane_distance_to_point(f.planes[i], s.center) < s.radius) return 0;
    return 1;
}
/* 球是否与视锥相交(保守, 边界接触算相交) */
static FORCE_INLINE bool_t frustum_intersects_sphere(frustum f, sphere s) {
    qty_t i;
    if (!frustum_is_valid(f) || !sphere_is_valid(s)) return 0;
    for (i = 0; i < MATH_FRUSTUM_PLANE_COUNT; i++) if (plane_distance_to_point(f.planes[i], s.center) < -s.radius) return 0;
    return 1;
}
/* 盒是否与视锥相交(保守测试: 用每个平面的正顶点, 可能误判为相交但不会漏判) */
static FORCE_INLINE bool_t frustum_intersects_aabb(frustum f, aabb b) {
    qty_t i;
    if (!frustum_is_valid(f) || !aabb_is_valid(b)) return 0;
    for (i = 0; i < MATH_FRUSTUM_PLANE_COUNT; i++) {
        vec3 n = plane_normal(f.planes[i]);
        vec3 p = vec3_new((n.x >= 0.0f) ? b.max.x : b.min.x,
            (n.y >= 0.0f) ? b.max.y : b.min.y,
            (n.z >= 0.0f) ? b.max.z : b.min.z);
        if (plane_distance_to_point(f.planes[i], p) < 0.0f) return 0;
    }
    return 1;
}
/* 盒是否完全在视锥内(用每个平面的负顶点测试, 不会误判) */
static FORCE_INLINE bool_t frustum_contains_aabb(frustum f, aabb b) {
    qty_t i;
    if (!frustum_is_valid(f) || !aabb_is_valid(b)) return 0;
    for (i = 0; i < MATH_FRUSTUM_PLANE_COUNT; i++) {
        vec3 n = plane_normal(f.planes[i]);
        vec3 p = vec3_new((n.x <= 0.0f) ? b.max.x : b.min.x,
            (n.y <= 0.0f) ? b.max.y : b.min.y,
            (n.z <= 0.0f) ? b.max.z : b.min.z);
        if (plane_distance_to_point(f.planes[i], p) < 0.0f) return 0;
    }
    return 1;
}

/* ---- 角点 ---- */
/* 三个平面求交点; 退化(平行)时返回 0 */
static FORCE_INLINE bool_t frustum_corner(plane a, plane b, plane c, vec3* out) {
    vec3 na = plane_normal(a), nb = plane_normal(b), nc = plane_normal(c);
    mat3 m = mat3_new(na.x, na.y, na.z, nb.x, nb.y, nb.z, nc.x, nc.y, nc.z);
    real_t det = mat3_determinant(m);
    if (math_abs(det) < MATH_EPSILON) return 0;
    *out = mat3_mul_vec3(mat3_inverse(m), vec3_new(-plane_distance(a), -plane_distance(b), -plane_distance(c)));
    return 1;
}
/* 求 8 个角点, 顺序为: 近平面 左下 右下 右上 左上, 远平面同序
 * 注意: NDC 的 y 向下, 命名中的"上/下"按 NDC 轴而非世界轴 */
static FORCE_INLINE bool_t frustum_corners(frustum f, vec3 out[MATH_FRUSTUM_CORNER_COUNT]) {
    bool_t ok = 1;
    ok &= frustum_corner(f.planes[MATH_FRUSTUM_NEAR], f.planes[MATH_FRUSTUM_LEFT], f.planes[MATH_FRUSTUM_BOTTOM], &out[0]);
    ok &= frustum_corner(f.planes[MATH_FRUSTUM_NEAR], f.planes[MATH_FRUSTUM_RIGHT], f.planes[MATH_FRUSTUM_BOTTOM], &out[1]);
    ok &= frustum_corner(f.planes[MATH_FRUSTUM_NEAR], f.planes[MATH_FRUSTUM_RIGHT], f.planes[MATH_FRUSTUM_TOP], &out[2]);
    ok &= frustum_corner(f.planes[MATH_FRUSTUM_NEAR], f.planes[MATH_FRUSTUM_LEFT], f.planes[MATH_FRUSTUM_TOP], &out[3]);
    ok &= frustum_corner(f.planes[MATH_FRUSTUM_FAR], f.planes[MATH_FRUSTUM_LEFT], f.planes[MATH_FRUSTUM_BOTTOM], &out[4]);
    ok &= frustum_corner(f.planes[MATH_FRUSTUM_FAR], f.planes[MATH_FRUSTUM_RIGHT], f.planes[MATH_FRUSTUM_BOTTOM], &out[5]);
    ok &= frustum_corner(f.planes[MATH_FRUSTUM_FAR], f.planes[MATH_FRUSTUM_RIGHT], f.planes[MATH_FRUSTUM_TOP], &out[6]);
    ok &= frustum_corner(f.planes[MATH_FRUSTUM_FAR], f.planes[MATH_FRUSTUM_LEFT], f.planes[MATH_FRUSTUM_TOP], &out[7]);
    return ok;
}


EXTERN_C_END
#endif