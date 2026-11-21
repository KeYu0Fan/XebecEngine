#ifndef MATH_GEOMETRY_SEGMENT_H
/* 线段
 * 参数方程: a + (b - a) * t, t ∈ [0, 1]
 * 约定: 零长度线段(a == b)按点处理
 */
#define MATH_GEOMETRY_SEGMENT_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/math/constants.h"
#include "core/math/vector/vector.h"
EXTERN_C_BEGIN


/* 线段 */
typedef struct { vec3 a; vec3 b; } segment;

/* ---- 构造 ---- */
/* 由两端点构造 */
static FORCE_INLINE segment segment_new(vec3 a, vec3 b) { segment s; s.a = a; s.b = b; return s; }
/* 是否有效(端点有限) */
static FORCE_INLINE bool_t segment_is_valid(segment s) {
    return (vec3_is_finite(s.a) && vec3_is_finite(s.b)) ? 1 : 0;
}

/* ---- 度量 ---- */
/* 方向(未归一化) */
static FORCE_INLINE vec3 segment_direction(segment s) { return vec3_sub(s.b, s.a); }
/* 长度平方 */
static FORCE_INLINE real_t segment_length_sq(segment s) { return vec3_length_sq(segment_direction(s)); }
/* 长度 */
static FORCE_INLINE real_t segment_length(segment s) { return math_sqrt(segment_length_sq(s)); }
/* 中点 */
static FORCE_INLINE vec3 segment_center(segment s) { return vec3_scale(vec3_add(s.a, s.b), 0.5f); }
/* 参数 t 处的点(不裁剪, 供外推使用) */
static FORCE_INLINE vec3 segment_at(segment s, real_t t) { return vec3_add(s.a, vec3_scale(segment_direction(s), t)); }

/* ---- 最近点与距离 ---- */
/* 线段上离 p 最近的点, 同时输出参数 t ∈ [0,1] */
static FORCE_INLINE vec3 segment_closest_point_t(segment s, vec3 p, real_t* t) {
    vec3 ab = segment_direction(s);
    real_t len_sq = vec3_dot(ab, ab);
    real_t k = (len_sq < MATH_EPSILON) ? 0.0f : math_clamp(vec3_dot(vec3_sub(p, s.a), ab) / len_sq, 0.0f, 1.0f);
    if (t) *t = k;
    return vec3_add(s.a, vec3_scale(ab, k));
}
/* 线段上离 p 最近的点 */
static FORCE_INLINE vec3 segment_closest_point(segment s, vec3 p) { return segment_closest_point_t(s, p, 0); }
/* 线段到点的距离平方 */
static FORCE_INLINE real_t segment_distance_sq(segment s, vec3 p) {
    return vec3_distance_sq(p, segment_closest_point(s, p));
}
/* 点是否在线段上(距离不超过 eps) */
static FORCE_INLINE bool_t segment_contains_point(segment s, vec3 p, real_t eps) {
    return (segment_distance_sq(s, p) <= eps * eps) ? 1 : 0;
}


EXTERN_C_END
#endif