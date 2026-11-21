#ifndef MATH_GEOMETRY_TRIANGLE_H
/* 三角形
 * 顶点顺序决定法线方向: normal = (b - a) × (c - a)
 * 约定: 面积为 0 时视为退化, 退化三角形不参与包含判定
 */
#define MATH_GEOMETRY_TRIANGLE_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/math/constants.h"
#include "core/math/vector/vector.h"
EXTERN_C_BEGIN


/* 三角形 */
typedef struct { vec3 a; vec3 b; vec3 c; } triangle;

/* ---- 构造 ---- */
/* 由三个顶点构造 */
static FORCE_INLINE triangle triangle_new(vec3 a, vec3 b, vec3 c) { triangle t; t.a = a; t.b = b; t.c = c; return t; }
/* 是否有效(顶点有限) */
static FORCE_INLINE bool_t triangle_is_valid(triangle t) {
    return (vec3_is_finite(t.a) && vec3_is_finite(t.b) && vec3_is_finite(t.c)) ? 1 : 0;
}
/* 是否退化(面积过小) */
static FORCE_INLINE bool_t triangle_is_degenerate(triangle t) {
    return (vec3_length_sq(vec3_cross(vec3_sub(t.b, t.a), vec3_sub(t.c, t.a))) < MATH_EPSILON) ? 1 : 0;
}

/* ---- 度量 ---- */
/* 单位法线; 退化时返回零向量 */
static FORCE_INLINE vec3 triangle_normal(triangle t) {
    vec3 n = vec3_cross(vec3_sub(t.b, t.a), vec3_sub(t.c, t.a));
    real_t len = vec3_length(n);
    return (len < MATH_EPSILON) ? vec3_zero() : vec3_scale(n, 1.0f / len);
}
/* 面积 */
static FORCE_INLINE real_t triangle_area(triangle t) {
    return 0.5f * vec3_length(vec3_cross(vec3_sub(t.b, t.a), vec3_sub(t.c, t.a)));
}
/* 重心 */
static FORCE_INLINE vec3 triangle_centroid(triangle t) {
    return vec3_scale(vec3_add(vec3_add(t.a, t.b), t.c), 1.0f / 3.0f);
}

/* ---- 重心坐标与包含 ---- */
/* 计算 p 关于三角形的重心坐标(a 权重 -> *u, b -> *v, c -> *w); 退化时返回 0
 * 注意: p 不要求位于三角形平面上, 结果是其在平面投影的重心坐标 */
static FORCE_INLINE bool_t triangle_barycentric(triangle t, vec3 p, real_t* u, real_t* v, real_t* w) {
    vec3 v0 = vec3_sub(t.b, t.a), v1 = vec3_sub(t.c, t.a), v2 = vec3_sub(p, t.a);
    real_t d00 = vec3_dot(v0, v0), d01 = vec3_dot(v0, v1), d11 = vec3_dot(v1, v1);
    real_t d20 = vec3_dot(v2, v0), d21 = vec3_dot(v2, v1);
    real_t denom = d00 * d11 - d01 * d01;
    real_t vb, wc;
    if (math_abs(denom) < MATH_EPSILON) return 0;
    vb = (d11 * d20 - d01 * d21) / denom;
    wc = (d00 * d21 - d01 * d20) / denom;
    if (u) *u = 1.0f - vb - wc;
    if (v) *v = vb;
    if (w) *w = wc;
    return 1;
}
/* 点是否在三角形内(含边界, 容差 eps); 点需已位于三角形平面附近 */
static FORCE_INLINE bool_t triangle_contains_point(triangle t, vec3 p, real_t eps) {
    real_t u, v, w;
    if (!triangle_barycentric(t, p, &u, &v, &w)) return 0;
    return (u >= -eps && v >= -eps && w >= -eps) ? 1 : 0;
}

/* ---- 最近点 ---- */
/* 三角形上离 p 最近的点(Ericson 区域判定) */
static FORCE_INLINE vec3 triangle_closest_point(triangle t, vec3 p) {
    vec3 ab = vec3_sub(t.b, t.a), ac = vec3_sub(t.c, t.a), ap = vec3_sub(p, t.a);
    real_t d1 = vec3_dot(ab, ap), d2 = vec3_dot(ac, ap);
    if (d1 <= 0.0f && d2 <= 0.0f) return t.a;
    vec3 bp = vec3_sub(p, t.b);
    real_t d3 = vec3_dot(ab, bp), d4 = vec3_dot(ac, bp);
    if (d3 >= 0.0f && d4 <= d3) return t.b;
    real_t vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
        return vec3_add(t.a, vec3_scale(ab, d1 / (d1 - d3)));
    }
    vec3 cp = vec3_sub(p, t.c);
    real_t d5 = vec3_dot(ab, cp), d6 = vec3_dot(ac, cp);
    if (d6 >= 0.0f && d5 <= d6) return t.c;
    real_t vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
        return vec3_add(t.a, vec3_scale(ac, d2 / (d2 - d6)));
    }
    real_t va = d3 * d6 - d5 * d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
        return vec3_add(t.b, vec3_scale(vec3_sub(t.c, t.b), (d4 - d3) / ((d4 - d3) + (d5 - d6))));
    }
    {
        real_t denom = 1.0f / (va + vb + vc);
        real_t v = vb * denom, w = vc * denom;
        return vec3_add(t.a, vec3_add(vec3_scale(ab, v), vec3_scale(ac, w)));
    }
}
/* 三角形到点的距离平方 */
static FORCE_INLINE real_t triangle_distance_sq(triangle t, vec3 p) {
    return vec3_distance_sq(p, triangle_closest_point(t, p));
}


EXTERN_C_END
#endif