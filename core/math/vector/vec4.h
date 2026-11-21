#ifndef MATH_VECTOR_VEC4_H
#define MATH_VECTOR_VEC4_H
#include "core/platform.h"
#include "core/typedef.h"
#include "core/math/scalar.h"
#include "core/math/vector/vec3.h"
EXTERN_C_BEGIN


typedef struct { real_t x, y, z, w; } vec4;

/* ---- 构造 ---- */
 /* 构造 */
static FORCE_INLINE vec4 vec4_new(real_t x, real_t y, real_t z, real_t w)
{
    vec4 v; v.x = x; v.y = y; v.z = z; v.w = w; return v;
}
/* 零向量 */
static FORCE_INLINE vec4 vec4_zero(void) { return vec4_new(0.0f, 0.0f, 0.0f, 0.0f); }
/* 全1向量 */
static FORCE_INLINE vec4 vec4_one(void) { return vec4_new(1.0f, 1.0f, 1.0f, 1.0f); }

/* ---- 基础算术 ---- */
 /* 加法 */
static FORCE_INLINE vec4 vec4_add(vec4 a, vec4 b) { return vec4_new(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
/* 减法 */
static FORCE_INLINE vec4 vec4_sub(vec4 a, vec4 b) { return vec4_new(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
/* 逐元素乘 */
static FORCE_INLINE vec4 vec4_mul(vec4 a, vec4 b) { return vec4_new(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
/* 逐元素除(不检查除零, 调用方需保证 b 的分量非零)
 * 注意: 若 b 为齐次方向向量(w == 0), w 分量会出现 0/0 得到 NaN
 * 可能产生 inf/NaN
 */
static FORCE_INLINE vec4 vec4_div(vec4 a, vec4 b) { return vec4_new(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w); }
/* 标量乘 */
static FORCE_INLINE vec4 vec4_scale(vec4 v, real_t s) { return vec4_new(v.x * s, v.y * s, v.z * s, v.w * s); }
/* 取反 */
static FORCE_INLINE vec4 vec4_negate(vec4 v) { return vec4_new(-v.x, -v.y, -v.z, -v.w); }

/* ---- 测量 ---- */
 /* 点乘 */
static FORCE_INLINE real_t vec4_dot(vec4 a, vec4 b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
/* 长度平方 */
static FORCE_INLINE real_t vec4_length_sq(vec4 v) { return vec4_dot(v, v); }
/* 模长 */
static FORCE_INLINE real_t vec4_length(vec4 v) { return math_sqrt(vec4_length_sq(v)); }
/* 距离平方 */
static FORCE_INLINE real_t vec4_distance_sq(vec4 a, vec4 b) { return vec4_length_sq(vec4_sub(a, b)); }
/* 距离 */
static FORCE_INLINE real_t vec4_distance(vec4 a, vec4 b) { return math_sqrt(vec4_distance_sq(a, b)); }

/* ---- 归一化与插值 ---- */
/* 归一化 */
static FORCE_INLINE vec4 vec4_normalize(vec4 v) {
    real_t len = vec4_length(v);
    return (len > MATH_EPSILON) ? vec4_scale(v, 1.0f / len) : vec4_zero();
}
/* 归一化失败时返回 fallback
 * 注意: fallback 原样返回, 函数不做任何归一化或校验；
 *       若调用方需要单位向量结果, 请自行保证 fallback 为单位向量 */
static FORCE_INLINE vec4 vec4_normalize_or(vec4 v, vec4 fallback) {
    real_t len = vec4_length(v);
    return (len > MATH_EPSILON) ? vec4_scale(v, 1.0f / len) : fallback;
}
/* 线性插值 */
static FORCE_INLINE vec4 vec4_lerp(vec4 a, vec4 b, real_t t)
{
    return vec4_new(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
}
/* 归一化线性插值(方向插值)
 * 注意: 当 a 与 b 接近反向(a ≈ -b)时, 插值中间结果可能退化为零向量, 
 *       此时 normalize 返回零向量；若需稳定的方向插值请改用 slerp */
static FORCE_INLINE vec4 vec4_nlerp(vec4 a, vec4 b, real_t t) {
    return vec4_normalize(vec4_lerp(a, b, t));
}

/* ---- 极值与比较 ---- */
 /* 逐元素最小 */
static FORCE_INLINE vec4 vec4_min(vec4 a, vec4 b)
{
    return vec4_new(math_min(a.x, b.x), math_min(a.y, b.y), math_min(a.z, b.z), math_min(a.w, b.w));
}
/* 逐元素最大 */
static FORCE_INLINE vec4 vec4_max(vec4 a, vec4 b)
{
    return vec4_new(math_max(a.x, b.x), math_max(a.y, b.y), math_max(a.z, b.z), math_max(a.w, b.w));
}
/* 范围限制 */
static FORCE_INLINE vec4 vec4_clamp(vec4 v, vec4 min, vec4 max)
{
    return vec4_new(
        math_clamp(v.x, min.x, max.x),
        math_clamp(v.y, min.y, max.y),
        math_clamp(v.z, min.z, max.z),
        math_clamp(v.w, min.w, max.w)
    );
}
/* 安全相等 */
static FORCE_INLINE bool_t vec4_is_equal(vec4 a, vec4 b) {
    return (math_is_equal(a.x, b.x) && math_is_equal(a.y, b.y) && math_is_equal(a.z, b.z) && math_is_equal(a.w, b.w)) ? 1 : 0;
}

/* ---- 逐元素工具 ---- */
/* 逐元素绝对值 */
static FORCE_INLINE vec4 vec4_abs(vec4 v) {
    return vec4_new(math_abs(v.x), math_abs(v.y), math_abs(v.z), math_abs(v.w));
}
/* 逐元素符号(-1 / 0 / +1) */
static FORCE_INLINE vec4 vec4_sign(vec4 v) {
    return vec4_new((real_t)(v.x > 0.0f) - (real_t)(v.x < 0.0f),
        (real_t)(v.y > 0.0f) - (real_t)(v.y < 0.0f),
        (real_t)(v.z > 0.0f) - (real_t)(v.z < 0.0f),
        (real_t)(v.w > 0.0f) - (real_t)(v.w < 0.0f));
}
/* 用同一标量填充所有分量 */
static FORCE_INLINE vec4 vec4_splat(real_t s) { return vec4_new(s, s, s, s); }

/* ---- 判定与统计 ---- */
/* 取 xyz 分量, 丢弃 w */
static FORCE_INLINE vec3 vec4_xyz(vec4 v) { return vec3_new(v.x, v.y, v.z); }
/* 是否所有分量有限 */
static FORCE_INLINE bool_t vec4_is_finite(vec4 v) {
    return (math_is_finite(v.x) && math_is_finite(v.y) && math_is_finite(v.z) && math_is_finite(v.w)) ? 1 : 0;
}
/* 是否为零向量(逐分量为 0) */
static FORCE_INLINE bool_t vec4_is_zero(vec4 v) {
    return (v.x == 0.0f && v.y == 0.0f && v.z == 0.0f && v.w == 0.0f) ? 1 : 0;
}
/* 是否已单位化 */
static FORCE_INLINE bool_t vec4_is_normalized(vec4 v) {
    return (math_abs(vec4_length_sq(v) - 1.0f) < MATH_EPSILON) ? 1 : 0;
}
/* 最大分量 */
static FORCE_INLINE real_t vec4_max_component(vec4 v) { return math_max(math_max(v.x, v.y), math_max(v.z, v.w)); }
/* 最小分量 */
static FORCE_INLINE real_t vec4_min_component(vec4 v) { return math_min(math_min(v.x, v.y), math_min(v.z, v.w)); }


EXTERN_C_END
#endif