#ifndef MATH_GEOMETRY_CAPSULE_H
/* 胶囊体
 * 表示: 线段(a, b) + 半径, 即到线段距离不超过 radius 的点集
 * 约定: radius < 0 视为空; 零长度线段退化为球
 */
#define MATH_GEOMETRY_CAPSULE_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/math/constants.h"
#include "core/math/vector/vector.h"
#include "core/math/geometry/segment.h"
EXTERN_C_BEGIN


/* 胶囊体 */
typedef struct { vec3 a; vec3 b; real_t radius; } capsule;

/* ---- 构造 ---- */
/* 由两端点与半径构造 */
static FORCE_INLINE capsule capsule_new(vec3 a, vec3 b, real_t radius) {
    capsule c; c.a = a; c.b = b; c.radius = radius; return c;
}
/* 由线段与半径构造 */
static FORCE_INLINE capsule capsule_from_segment(segment s, real_t radius) {
    return capsule_new(s.a, s.b, radius);
}
/* 是否有效(半径非负且端点有限) */
static FORCE_INLINE bool_t capsule_is_valid(capsule c) {
    return (c.radius >= 0.0f && math_is_finite(c.radius) && vec3_is_finite(c.a) && vec3_is_finite(c.b)) ? 1 : 0;
}

/* ---- 度量 ---- */
/* 中线(线段) */
static FORCE_INLINE segment capsule_segment(capsule c) { return segment_new(c.a, c.b); }
/* 中心 */
static FORCE_INLINE vec3 capsule_center(capsule c) { return segment_center(capsule_segment(c)); }
/* 中线长度 */
static FORCE_INLINE real_t capsule_height(capsule c) { return segment_length(capsule_segment(c)); }
/* 中线方向(未归一化) */
static FORCE_INLINE vec3 capsule_direction(capsule c) { return segment_direction(capsule_segment(c)); }

/* ---- 距离与包含 ---- */
/* 胶囊体上离 p 最近的点; p 在体内时返回 p 本身 */
static FORCE_INLINE vec3 capsule_closest_point(capsule c, vec3 p) {
    vec3 s = segment_closest_point(capsule_segment(c), p);
    vec3 d = vec3_sub(p, s);
    real_t len = vec3_length(d);
    if (len <= c.radius) return p;
    if (len < MATH_EPSILON) return s;
    return vec3_add(s, vec3_scale(d, c.radius / len));
}
/* 胶囊体到点的距离平方(点在体内为 0) */
static FORCE_INLINE real_t capsule_distance_sq(capsule c, vec3 p) {
    real_t d = segment_distance_sq(capsule_segment(c), p);
    real_t len = math_sqrt(d) - c.radius;
    return (len > 0.0f) ? len * len : 0.0f;
}
/* 是否包含点(边界视为包含) */
static FORCE_INLINE bool_t capsule_contains_point(capsule c, vec3 p) {
    return (capsule_distance_sq(c, p) <= 0.0f) ? 1 : 0;
}


EXTERN_C_END
#endif