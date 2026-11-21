#ifndef MATH_UTILS_CURVE_H
/* 曲线与缓动
 * 插值函数均以 t ∈ [0,1] 为参数; 缓动函数输入超出 [0,1] 时不做限制
 */
#define MATH_UTILS_CURVE_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/math/constants.h"
#include "core/math/vector/vector.h"
EXTERN_C_BEGIN


/* ---- 标量曲线 ---- */
/* 二次贝塞尔 */
static FORCE_INLINE real_t curve_bezier_quadratic(real_t p0, real_t p1, real_t p2, real_t t) {
    real_t it = 1.0f - t;
    return it * it * p0 + 2.0f * it * t * p1 + t * t * p2;
}
/* 三次贝塞尔 */
static FORCE_INLINE real_t curve_bezier_cubic(real_t p0, real_t p1, real_t p2, real_t p3, real_t t) {
    real_t it = 1.0f - t;
    return it * it * it * p0 + 3.0f * it * it * t * p1 + 3.0f * it * t * t * p2 + t * t * t * p3;
}
/* 埃尔米特样条: p0/p1 端点, m0/m1 端点切线 */
static FORCE_INLINE real_t curve_hermite(real_t p0, real_t m0, real_t p1, real_t m1, real_t t) {
    real_t t2 = t * t, t3 = t2 * t;
    return (2.0f * t3 - 3.0f * t2 + 1.0f) * p0 + (t3 - 2.0f * t2 + t) * m0
        + (-2.0f * t3 + 3.0f * t2) * p1 + (t3 - t2) * m1;
}
/* Catmull-Rom 样条(过 p1 -> p2 段) */
static FORCE_INLINE real_t curve_catmull_rom(real_t p0, real_t p1, real_t p2, real_t p3, real_t t) {
    real_t t2 = t * t, t3 = t2 * t;
    return 0.5f * ((2.0f * p1) + (-p0 + p2) * t
        + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2
        + (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

/* ---- 向量曲线 ---- */
/* 三维二次贝塞尔 */
static FORCE_INLINE vec3 curve_bezier_quadratic_v3(vec3 p0, vec3 p1, vec3 p2, real_t t) {
    real_t it = 1.0f - t;
    return vec3_add(vec3_add(vec3_scale(p0, it * it), vec3_scale(p1, 2.0f * it * t)), vec3_scale(p2, t * t));
}
/* 三维三次贝塞尔 */
static FORCE_INLINE vec3 curve_bezier_cubic_v3(vec3 p0, vec3 p1, vec3 p2, vec3 p3, real_t t) {
    real_t it = 1.0f - t;
    return vec3_add(
        vec3_add(vec3_scale(p0, it * it * it), vec3_scale(p1, 3.0f * it * it * t)),
        vec3_add(vec3_scale(p2, 3.0f * it * t * t), vec3_scale(p3, t * t * t)));
}
/* 三维 Catmull-Rom 样条(过 p1 -> p2 段) */
static FORCE_INLINE vec3 curve_catmull_rom_v3(vec3 p0, vec3 p1, vec3 p2, vec3 p3, real_t t) {
    real_t t2 = t * t, t3 = t2 * t;
    vec3 a = vec3_scale(p1, 2.0f);
    vec3 b = vec3_scale(vec3_sub(p2, p0), t);
    vec3 c = vec3_scale(vec3_add(vec3_sub(vec3_scale(p0, 2.0f), vec3_scale(p1, 5.0f)), vec3_sub(vec3_scale(p2, 4.0f), p3)), t2);
    vec3 d = vec3_scale(vec3_add(vec3_sub(vec3_scale(p1, 3.0f), vec3_scale(p2, 3.0f)), vec3_sub(p3, p0)), t3);
    return vec3_scale(vec3_add(vec3_add(a, b), vec3_add(c, d)), 0.5f);
}

/* ---- 缓动 ---- */
/* 线性 */
static FORCE_INLINE real_t curve_ease_linear(real_t t) { return t; }
/* 二次缓入/缓出/进出 */
static FORCE_INLINE real_t curve_ease_in_quad(real_t t) { return t * t; }
static FORCE_INLINE real_t curve_ease_out_quad(real_t t) { return t * (2.0f - t); }
static FORCE_INLINE real_t curve_ease_inout_quad(real_t t) {
    return (t < 0.5f) ? (2.0f * t * t) : (1.0f - 2.0f * (1.0f - t) * (1.0f - t));
}
/* 三次缓入/缓出/进出 */
static FORCE_INLINE real_t curve_ease_in_cubic(real_t t) { return t * t * t; }
static FORCE_INLINE real_t curve_ease_out_cubic(real_t t) {
    real_t it = 1.0f - t;
    return 1.0f - it * it * it;
}
static FORCE_INLINE real_t curve_ease_inout_cubic(real_t t) {
    return (t < 0.5f) ? (4.0f * t * t * t) : (1.0f - 4.0f * (1.0f - t) * (1.0f - t) * (1.0f - t));
}
/* 正弦缓入/缓出/进出 */
static FORCE_INLINE real_t curve_ease_in_sine(real_t t) { return 1.0f - math_cos(t * MATH_HALF_PI); }
static FORCE_INLINE real_t curve_ease_out_sine(real_t t) { return math_sin(t * MATH_HALF_PI); }
static FORCE_INLINE real_t curve_ease_inout_sine(real_t t) { return 0.5f * (1.0f - math_cos(t * MATH_PI)); }
/* 指数缓入/缓出 */
static FORCE_INLINE real_t curve_ease_in_expo(real_t t) {
    return (t <= 0.0f) ? 0.0f : math_pow(2.0f, 10.0f * (t - 1.0f));
}
static FORCE_INLINE real_t curve_ease_out_expo(real_t t) {
    return (t >= 1.0f) ? 1.0f : 1.0f - math_pow(2.0f, -10.0f * t);
}
/* 指数缓入缓出 */
static FORCE_INLINE real_t curve_ease_inout_expo(real_t t) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    return (t < 0.5f) ? 0.5f * math_pow(2.0f, 20.0f * t - 10.0f)
        : 1.0f - 0.5f * math_pow(2.0f, -20.0f * t + 10.0f);
}
/* 回弹缓入/缓出/进出 */
static FORCE_INLINE real_t curve_ease_in_back(real_t t) {
    real_t s = MATH_BACK_OVERSHOOT;
    return (s + 1.0f) * t * t * t - s * t * t;
}
static FORCE_INLINE real_t curve_ease_out_back(real_t t) {
    real_t s = MATH_BACK_OVERSHOOT;
    real_t it = t - 1.0f;
    return 1.0f + (s + 1.0f) * it * it * it + s * it * it;
}
static FORCE_INLINE real_t curve_ease_inout_back(real_t t) {
    real_t s = MATH_BACK_OVERSHOOT * 1.525f;
    t *= 2.0f;
    if (t < 1.0f) return 0.5f * (t * t * ((s + 1.0f) * t - s));
    t -= 2.0f;
    return 0.5f * (t * t * ((s + 1.0f) * t + s) + 2.0f);
}
/* 弹跳缓入/缓出/进出 */
static FORCE_INLINE real_t curve_ease_out_bounce(real_t t) {
    if (t < 1.0f / MATH_BOUNCE_N) return MATH_BOUNCE_SCALE * t * t;
    if (t < 2.0f / MATH_BOUNCE_N) { t -= MATH_BOUNCE_T1 / MATH_BOUNCE_N; return MATH_BOUNCE_SCALE * t * t + MATH_BOUNCE_H1; }
    if (t < 2.5f / MATH_BOUNCE_N) { t -= MATH_BOUNCE_T2 / MATH_BOUNCE_N; return MATH_BOUNCE_SCALE * t * t + MATH_BOUNCE_H2; }
    t -= MATH_BOUNCE_T3 / MATH_BOUNCE_N;
    return MATH_BOUNCE_SCALE * t * t + MATH_BOUNCE_H3;
}
static FORCE_INLINE real_t curve_ease_in_bounce(real_t t) { return 1.0f - curve_ease_out_bounce(1.0f - t); }
static FORCE_INLINE real_t curve_ease_inout_bounce(real_t t) {
    if (t < 0.5f) return 0.5f * (1.0f - curve_ease_out_bounce(1.0f - 2.0f * t));
    return 0.5f * (1.0f + curve_ease_out_bounce(2.0f * t - 1.0f));
}
/* 弹性缓入/缓出/进出 */
static FORCE_INLINE real_t curve_ease_in_elastic(real_t t) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    return -math_pow(2.0f, 10.0f * (t - 1.0f)) * math_sin((t - 0.075f) * MATH_TWO_PI / 3.0f);
}
static FORCE_INLINE real_t curve_ease_out_elastic(real_t t) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    return math_pow(2.0f, -10.0f * t) * math_sin((t - 0.075f) * MATH_TWO_PI / 3.0f) + 1.0f;
}
static FORCE_INLINE real_t curve_ease_inout_elastic(real_t t) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    t *= 2.0f;
    if (t < 1.0f) return -0.5f * math_pow(2.0f, 10.0f * (t - 1.0f)) * math_sin((t - 0.1f) * MATH_TWO_PI / 4.5f);
    t -= 1.0f;
    return math_pow(2.0f, -10.0f * t) * math_sin((t - 0.1f) * MATH_TWO_PI / 4.5f) * 0.5f + 1.0f;
}


EXTERN_C_END
#endif