#ifndef MATH_GEOMETRY_OBB_H
/* 有向包围盒
 * 表示: 中心 + 半尺寸 + 朝向矩阵(列为主轴, 需正交单位)
 * 约定: half 任一分量为负视为空; rotation 通常为单位阵或纯旋转
 */
#define MATH_GEOMETRY_OBB_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/math/constants.h"
#include "core/math/vector/vector.h"
#include "core/math/matrix/mat3.h"
#include "core/math/geometry/aabb.h"
EXTERN_C_BEGIN


/* 有向包围盒 */
typedef struct { vec3 center; vec3 half; mat3 rotation; } obb;

/* ---- 构造 ---- */
/* 由中心、半尺寸与朝向构造 */
static FORCE_INLINE obb obb_new(vec3 center, vec3 half, mat3 rotation) {
    obb b; b.center = center; b.half = half; b.rotation = rotation; return b;
}
/* 由 AABB 与朝向构造(rotation 传 mat3_identity() 即世界轴对齐) */
static FORCE_INLINE obb obb_from_aabb(aabb a, mat3 rotation) {
    return obb_new(aabb_center(a), aabb_half(a), rotation);
}
/* 是否有效(半尺寸非负且中心有限) */
static FORCE_INLINE bool_t obb_is_valid(obb b) {
    return (b.half.x >= 0.0f && b.half.y >= 0.0f && b.half.z >= 0.0f &&
        vec3_is_finite(b.center) && vec3_is_finite(b.half)) ? 1 : 0;
}

/* ---- 轴与角点 ---- */
/* 第 i 条主轴(0..2), 单位向量 */
static FORCE_INLINE vec3 obb_axis(obb b, qty_t i) {
    return (i == 0) ? vec3_new(b.rotation.m[0][0], b.rotation.m[1][0], b.rotation.m[2][0])
        : (i == 1) ? vec3_new(b.rotation.m[0][1], b.rotation.m[1][1], b.rotation.m[2][1])
        : vec3_new(b.rotation.m[0][2], b.rotation.m[1][2], b.rotation.m[2][2]);
}
/* 第 i 条半尺寸分量 */
static FORCE_INLINE real_t obb_extent(obb b, qty_t i) {
    return (i == 0) ? b.half.x : ((i == 1) ? b.half.y : b.half.z);
}
/* 第 i 个角点(位 0/1/2 对应三条主轴取负/正) */
static FORCE_INLINE vec3 obb_corner(obb b, qty_t i) {
    vec3 p = b.center;
    qty_t k;
    for (k = 0; k < 3; k++) {
        real_t s = ((i >> k) & 1u) ? obb_extent(b, k) : -obb_extent(b, k);
        p = vec3_add(p, vec3_scale(obb_axis(b, k), s));
    }
    return p;
}

/* ---- 包含与距离 ---- */
/* 世界点是否在盒内(边界视为包含) */
static FORCE_INLINE bool_t obb_contains_point(obb b, vec3 p) {
    vec3 d = vec3_sub(p, b.center);
    qty_t i;
    for (i = 0; i < 3; i++) {
        if (math_abs(vec3_dot(d, obb_axis(b, i))) > obb_extent(b, i) + MATH_EPSILON) return 0;
    }
    return 1;
}
/* 盒上离 p 最近的点; p 在盒内时返回 p 本身 */
static FORCE_INLINE vec3 obb_closest_point(obb b, vec3 p) {
    vec3 d = vec3_sub(p, b.center);
    vec3 q = b.center;
    qty_t i;
    for (i = 0; i < 3; i++) {
        real_t local = math_clamp(vec3_dot(d, obb_axis(b, i)), -obb_extent(b, i), obb_extent(b, i));
        q = vec3_add(q, vec3_scale(obb_axis(b, i), local));
    }
    return q;
}
/* 盒到点的距离平方(点在盒内为 0) */
static FORCE_INLINE real_t obb_distance_sq(obb b, vec3 p) {
    return vec3_distance_sq(p, obb_closest_point(b, p));
}

/* ---- 转换 ---- */
/* 世界轴对齐包围盒(旋转后通常变大) */
static FORCE_INLINE aabb obb_to_aabb(obb b) {
    vec3 e = vec3_new(
        math_abs(b.rotation.m[0][0]) * b.half.x + math_abs(b.rotation.m[0][1]) * b.half.y + math_abs(b.rotation.m[0][2]) * b.half.z,
        math_abs(b.rotation.m[1][0]) * b.half.x + math_abs(b.rotation.m[1][1]) * b.half.y + math_abs(b.rotation.m[1][2]) * b.half.z,
        math_abs(b.rotation.m[2][0]) * b.half.x + math_abs(b.rotation.m[2][1]) * b.half.y + math_abs(b.rotation.m[2][2]) * b.half.z);
    return aabb_new(vec3_sub(b.center, e), vec3_add(b.center, e));
}


EXTERN_C_END
#endif