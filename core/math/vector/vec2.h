#ifndef MATH_VECTOR_VEC2_H
#define MATH_VECTOR_VEC2_H
#include "core/platform.h"
#include "core/typedef.h"
#include "core/math/scalar.h"
EXTERN_C_BEGIN


typedef struct { real_t x, y; } vec2;

/* ---- 构造 ---- */
/* 构造新向量 */
static FORCE_INLINE vec2 vec2_new(real_t x, real_t y) { vec2 v; v.x = x; v.y = y; return v; }
/* 零向量 (0,0) */
static FORCE_INLINE vec2 vec2_zero(void) { return vec2_new(0.0f, 0.0f); }
/* 全1向量 (1,1) */
static FORCE_INLINE vec2 vec2_one(void) { return vec2_new(1.0f, 1.0f); }

/* ---- 基础算术 ---- */
/* 加法 */
static FORCE_INLINE vec2 vec2_add(vec2 a, vec2 b) { return vec2_new(a.x + b.x, a.y + b.y); }
/* 减法 */
static FORCE_INLINE vec2 vec2_sub(vec2 a, vec2 b) { return vec2_new(a.x - b.x, a.y - b.y); }
/* 逐元素乘 */
static FORCE_INLINE vec2 vec2_mul(vec2 a, vec2 b) { return vec2_new(a.x * b.x, a.y * b.y); }
/* 逐元素除(不检查除零, 调用方需保证 b 的分量非零) */
static FORCE_INLINE vec2 vec2_div(vec2 a, vec2 b) { return vec2_new(a.x / b.x, a.y / b.y); }
/* 标量乘法 */
static FORCE_INLINE vec2 vec2_scale(vec2 v, real_t s) { return vec2_new(v.x * s, v.y * s); }
/* 取反 */
static FORCE_INLINE vec2 vec2_negate(vec2 v) { return vec2_new(-v.x, -v.y); }

/* ---- 测量 ---- */
/* 点乘 */
static FORCE_INLINE real_t vec2_dot(vec2 a, vec2 b) { return a.x * b.x + a.y * b.y; }
/* 长度平方 (省去开方) */
static FORCE_INLINE real_t vec2_length_sq(vec2 v) { return vec2_dot(v, v); }
/* 向量长度 (模长) */
static FORCE_INLINE real_t vec2_length(vec2 v) { return math_sqrt(vec2_length_sq(v)); }
/* 两点距离平方 */
static FORCE_INLINE real_t vec2_distance_sq(vec2 a, vec2 b) { return vec2_length_sq(vec2_sub(a, b)); }
/* 两点距离 */
static FORCE_INLINE real_t vec2_distance(vec2 a, vec2 b) { return math_sqrt(vec2_distance_sq(a, b)); }

/* ---- 归一化与插值 ---- */
 /* 归一化 (防除以0) */
static FORCE_INLINE vec2 vec2_normalize(vec2 v) {
    real_t len = vec2_length(v);
    return (len > MATH_EPSILON) ? vec2_scale(v, 1.0f / len) : vec2_zero();
}
/* 归一化失败时返回 fallback, 方向计算更安全
 * 注意: fallback 原样返回, 函数不做任何归一化或校验；
 *       若调用方需要单位向量结果, 请自行保证 fallback 为单位向量 */
static FORCE_INLINE vec2 vec2_normalize_or(vec2 v, vec2 fallback) {
    real_t len = vec2_length(v);
    return (len > MATH_EPSILON) ? vec2_scale(v, 1.0f / len) : fallback;
}
/* 线性插值 */
static FORCE_INLINE vec2 vec2_lerp(vec2 a, vec2 b, real_t t)
{
    return vec2_new(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
}
/* 归一化线性插值(方向插值)
 * 注意: 当 a 与 b 接近反向(a ≈ -b)时, 插值中间结果可能退化为零向量, 
 *       此时 normalize 返回零向量；若需稳定的方向插值请改用 slerp */
static FORCE_INLINE vec2 vec2_nlerp(vec2 a, vec2 b, real_t t) {
    return vec2_normalize(vec2_lerp(a, b, t));
}

/* ---- 极值与比较 ---- */
 /* 逐元素最小值 */
static FORCE_INLINE vec2 vec2_min(vec2 a, vec2 b)
{
    return vec2_new(math_min(a.x, b.x), math_min(a.y, b.y));
}
/* 逐元素最大值 */
static FORCE_INLINE vec2 vec2_max(vec2 a, vec2 b) { return vec2_new(math_max(a.x, b.x), math_max(a.y, b.y)); }
/* 逐元素范围限制 */
static FORCE_INLINE vec2 vec2_clamp(vec2 v, vec2 min, vec2 max)
{
    return vec2_new(math_clamp(v.x, min.x, max.x), math_clamp(v.y, min.y, max.y));
}
/* 安全浮点比较 */
static FORCE_INLINE bool_t vec2_is_equal(vec2 a, vec2 b)
{
    return (math_is_equal(a.x, b.x) && math_is_equal(a.y, b.y)) ? 1 : 0;
}

/* ---- 2D 进阶 ---- */
/* 2D 叉积(标量), >0 表示 b 在 a 逆时针侧 */
static FORCE_INLINE real_t vec2_cross(vec2 a, vec2 b) { return a.x * b.y - a.y * b.x; }
/* 逆时针旋转 90° 的垂直向量 */
static FORCE_INLINE vec2 vec2_perp(vec2 v) { return vec2_new(-v.y, v.x); }
/* 绕原点旋转 angle 弧度 */
static FORCE_INLINE vec2 vec2_rotate(vec2 v, real_t angle) {
    real_t c = math_cos(angle), s = math_sin(angle);
    return vec2_new(v.x * c - v.y * s, v.x * s + v.y * c);
}
/* 向量夹角(弧度), 任一为零向量返回 0
 * 注意: 分别判断两向量长度, 避免用长度^2与 MATH_EPSILON 直接比较造成量纲不一致 */
static FORCE_INLINE real_t vec2_angle(vec2 a, vec2 b) {
    real_t la = vec2_length(a), lb = vec2_length(b);
    return (la < MATH_EPSILON || lb < MATH_EPSILON) ? 0.0f
        : math_acos(math_clamp(vec2_dot(a, b) / (la * lb), -1.0f, 1.0f));
}
/* 返回 a 在 b 方向上的投影分量 */
static FORCE_INLINE vec2 vec2_project(vec2 a, vec2 b) {
    real_t denom = vec2_dot(b, b);
    return (denom < MATH_EPSILON * MATH_EPSILON) ? vec2_zero()
        : vec2_scale(b, vec2_dot(a, b) / denom);
}
/* 反射, normal 需为单位向量 */
static FORCE_INLINE vec2 vec2_reflect(vec2 incident, vec2 normal) {
    return vec2_sub(incident, vec2_scale(normal, 2.0f * vec2_dot(incident, normal)));
}
/* 朝 target 平滑移动, 单帧步长不超过 max_step
 * 注意: max_step 应为非负值；若传入负值将沿反方向远离 target */
static FORCE_INLINE vec2 vec2_move_towards(vec2 current, vec2 target, real_t max_step) {
    vec2 diff = vec2_sub(target, current);
    real_t dist = vec2_length(diff);
    return (dist <= max_step || dist < MATH_EPSILON) ? target
        : vec2_add(current, vec2_scale(diff, max_step / dist));
}

/* ---- 逐元素工具 ---- */
/* 逐元素绝对值 */
static FORCE_INLINE vec2 vec2_abs(vec2 v) { return vec2_new(math_abs(v.x), math_abs(v.y)); }
/* 逐元素符号(-1 / 0 / +1) */
static FORCE_INLINE vec2 vec2_sign(vec2 v) {
    return vec2_new((real_t)(v.x > 0.0f) - (real_t)(v.x < 0.0f),
        (real_t)(v.y > 0.0f) - (real_t)(v.y < 0.0f));
}
/* 用同一标量填充所有分量 */
static FORCE_INLINE vec2 vec2_splat(real_t s) { return vec2_new(s, s); }

/* ---- 逐元素取整与步进 ---- */
/* 逐元素对齐到步长 */
static FORCE_INLINE vec2 vec2_snap(vec2 v, real_t step) { return vec2_new(math_snap(v.x, step), math_snap(v.y, step)); }
/* 逐元素向下取整 */
static FORCE_INLINE vec2 vec2_floor(vec2 v) { return vec2_new(math_floor(v.x), math_floor(v.y)); }
/* 逐元素向上取整 */
static FORCE_INLINE vec2 vec2_ceil(vec2 v) { return vec2_new(math_ceil(v.x), math_ceil(v.y)); }
/* 逐元素四舍五入 */
static FORCE_INLINE vec2 vec2_round(vec2 v) { return vec2_new(math_round(v.x), math_round(v.y)); }
/* 限制长度不超过 max_len */
static FORCE_INLINE vec2 vec2_clamp_length(vec2 v, real_t max_len) {
    real_t len = vec2_length(v);
    if (len <= max_len || len < MATH_EPSILON) return v;
    return vec2_scale(v, max_len / len);
}

/* ---- 方向与旋转 ---- */
/* 单位化别名; 零向量返回零向量 */
static FORCE_INLINE vec2 vec2_direction(vec2 v) { return vec2_normalize(v); }
/* 绕 pivot 旋转 angle 弧度 */
static FORCE_INLINE vec2 vec2_rotate_around(vec2 v, vec2 pivot, real_t angle) {
    vec2 d = vec2_sub(v, pivot);
    real_t c = math_cos(angle), s = math_sin(angle);
    return vec2_new(pivot.x + d.x * c - d.y * s, pivot.y + d.x * s + d.y * c);
}
/* 由弧度与长度构造向量 */
static FORCE_INLINE vec2 vec2_from_angle(real_t angle, real_t length) {
    return vec2_new(math_cos(angle) * length, math_sin(angle) * length);
}
/* 取向量极角 [-PI, PI] */
static FORCE_INLINE real_t vec2_angle_of(vec2 v) { return math_atan2(v.y, v.x); }
/* 方向插值: 角度沿最短弧插值, 长度线性插值; 零向量退化为 lerp */
static FORCE_INLINE vec2 vec2_slerp(vec2 a, vec2 b, real_t t) {
    real_t la = vec2_length(a), lb = vec2_length(b);
    if (la < MATH_EPSILON || lb < MATH_EPSILON) return vec2_lerp(a, b, t);
    return vec2_from_angle(math_lerp_angle(vec2_angle_of(a), vec2_angle_of(b), t), math_lerp(la, lb, t));
}
/* 朝 target 方向旋转, 单次角步长不超过 max_angle 弧度; 已到达时返回单位目标方向 */
static FORCE_INLINE vec2 vec2_rotate_towards(vec2 current, vec2 target, real_t max_angle) {
    vec2 cur = vec2_normalize(current), tg = vec2_normalize(target);
    real_t d = math_angle_delta(vec2_angle_of(cur), vec2_angle_of(tg));
    if (math_abs(d) <= max_angle || math_abs(d) < MATH_EPSILON) return tg;
    return vec2_from_angle(vec2_angle_of(cur) + ((d > 0.0f) ? max_angle : -max_angle), 1.0f);
}

/* ---- 判定与统计 ---- */
/* 是否所有分量有限 */
static FORCE_INLINE bool_t vec2_is_finite(vec2 v) {
    return (math_is_finite(v.x) && math_is_finite(v.y)) ? 1 : 0;
}
/* 是否为零向量(逐分量为 0) */
static FORCE_INLINE bool_t vec2_is_zero(vec2 v) { return (v.x == 0.0f && v.y == 0.0f) ? 1 : 0; }
/* 是否已单位化 */
static FORCE_INLINE bool_t vec2_is_normalized(vec2 v) {
    return (math_abs(vec2_length_sq(v) - 1.0f) < MATH_EPSILON) ? 1 : 0;
}
/* 最大分量 */
static FORCE_INLINE real_t vec2_max_component(vec2 v) { return math_max(v.x, v.y); }
/* 最小分量 */
static FORCE_INLINE real_t vec2_min_component(vec2 v) { return math_min(v.x, v.y); }
/* 分量和 */
static FORCE_INLINE real_t vec2_sum(vec2 v) { return v.x + v.y; }


EXTERN_C_END
#endif