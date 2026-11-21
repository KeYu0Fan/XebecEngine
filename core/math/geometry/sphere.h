#ifndef MATH_GEOMETRY_SPHERE_H
/* 球形包围体
 * 约定: radius < 0 视为空球, 空球不参与求交
 */
#define MATH_GEOMETRY_SPHERE_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/math/constants.h"
#include "core/math/vector/vector.h"
EXTERN_C_BEGIN


/* 球 */
typedef struct { vec3 center; real_t radius; } sphere;

/* ---- 构造 ---- */
/* 由球心与半径构造 */
static FORCE_INLINE sphere sphere_new(vec3 center, real_t radius) {
    sphere s; s.center = center; s.radius = radius; return s;
}
/* 空球(不可见) */
static FORCE_INLINE sphere sphere_empty(void) { return sphere_new(vec3_zero(), -1.0f); }
/* 是否有效(半径非负且有限) */
static FORCE_INLINE bool_t sphere_is_valid(sphere s) {
    return (s.radius >= 0.0f && math_is_finite(s.radius) && vec3_is_finite(s.center)) ? 1 : 0;
}

/* ---- 包含与相交(同类) ---- */
/* 是否包含点(边界视为包含) */
static FORCE_INLINE bool_t sphere_contains_point(sphere s, vec3 p) {
    if (!sphere_is_valid(s)) return 0;
    return (vec3_distance_sq(p, s.center) <= s.radius * s.radius) ? 1 : 0;
}
/* 是否完全包含另一个球 */
static FORCE_INLINE bool_t sphere_contains_sphere(sphere s, sphere o) {
    if (!sphere_is_valid(s) || !sphere_is_valid(o)) return 0;
    real_t d = vec3_distance(s.center, o.center);
    return (d + o.radius - s.radius <= MATH_EPSILON) ? 1 : 0;
}
/* 是否与另一个球重叠(边界接触视为重叠) */
static FORCE_INLINE bool_t sphere_overlaps(sphere s, sphere o) {
    if (!sphere_is_valid(s) || !sphere_is_valid(o)) return 0;
    real_t r = s.radius + o.radius;
    return (vec3_distance_sq(s.center, o.center) <= r * r) ? 1 : 0;
}

/* ---- 扩展与合并 ---- */
/* 扩展以包含点(球心不变) */
static FORCE_INLINE sphere sphere_expand_point(sphere s, vec3 p) {
    if (!sphere_is_valid(s)) return sphere_new(p, 0.0f);
    real_t d = vec3_distance(s.center, p);
    return sphere_new(s.center, math_max(s.radius, d));
}
/* 合并两个球为最小包围球 */
static FORCE_INLINE sphere sphere_merge(sphere a, sphere b) {
    if (!sphere_is_valid(a)) return b;
    if (!sphere_is_valid(b)) return a;
    vec3 d = vec3_sub(b.center, a.center);
    real_t dist = vec3_length(d);
    if (dist + b.radius <= a.radius) return a;
    if (dist + a.radius <= b.radius) return b;
    real_t r = (dist + a.radius + b.radius) * 0.5f;
    if (dist < MATH_EPSILON) return sphere_new(a.center, math_max(a.radius, b.radius));
    return sphere_new(vec3_add(a.center, vec3_scale(d, (r - a.radius) / dist)), r);
}

/* ---- 距离查询 ---- */
/* 球面上离 p 最近的点; p 与球心重合时返回球心 */
static FORCE_INLINE vec3 sphere_closest_point(sphere s, vec3 p) {
    vec3 d = vec3_sub(p, s.center);
    real_t len = vec3_length(d);
    if (len < MATH_EPSILON) return s.center;
    return vec3_add(s.center, vec3_scale(d, s.radius / len));
}
/* 球到点的距离平方(点在球内为 0) */
static FORCE_INLINE real_t sphere_distance_sq(sphere s, vec3 p) {
    real_t d = vec3_distance(s.center, p) - s.radius;
    return (d > 0.0f) ? d * d : 0.0f;
}


EXTERN_C_END
#endif