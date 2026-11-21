#ifndef MATH_VECTOR_VEC3_H
#define MATH_VECTOR_VEC3_H
#include "core/platform.h"
#include "core/typedef.h"
#include "core/math/scalar.h"
EXTERN_C_BEGIN


/* vec3: 三维向量
 * 注意: 结构体中额外带有 w 字段, 仅作 16 字节对齐 / SIMD 预留的 padding, 
 *       不参与任何三维运算; sizeof(vec3) == sizeof(vec4)(与 vec4 相同)
 *       vec3_new 会将 w 置 0, 因此经 vec3_from_vec4 等转换后 w 不保留原值
 *       进行序列化|IO|数组 stride 或 memcpy 时请勿按 12 字节假设 */
    typedef struct { real_t x, y, z, w; } vec3;

/* ---- 构造 ---- */
 /* 构造新向量(w 作为 padding 被置 0) */
static FORCE_INLINE vec3 vec3_new(real_t x, real_t y, real_t z)
{
    vec3 v; v.x = x; v.y = y; v.z = z; v.w = 0.0f; return v;
}
/* 零向量 */
static FORCE_INLINE vec3 vec3_zero(void) { return vec3_new(0.0f, 0.0f, 0.0f); }
/* 全1向量 */
static FORCE_INLINE vec3 vec3_one(void) { return vec3_new(1.0f, 1.0f, 1.0f); }

/* ---- 基础算术 ---- */
 /* 加法 */
static FORCE_INLINE vec3 vec3_add(vec3 a, vec3 b) { return vec3_new(a.x + b.x, a.y + b.y, a.z + b.z); }
/* 减法 */
static FORCE_INLINE vec3 vec3_sub(vec3 a, vec3 b) { return vec3_new(a.x - b.x, a.y - b.y, a.z - b.z); }
/* 逐元素乘 */
static FORCE_INLINE vec3 vec3_mul(vec3 a, vec3 b) { return vec3_new(a.x * b.x, a.y * b.y, a.z * b.z); }
/* 逐元素除(不检查除零, 调用方需保证 b 的分量非零)
 * 注意: w 为 padding, 不参与除法, 结果 w 由 vec3_new 置 0 */
static FORCE_INLINE vec3 vec3_div(vec3 a, vec3 b) { return vec3_new(a.x / b.x, a.y / b.y, a.z / b.z); }
/* 标量乘 */
static FORCE_INLINE vec3 vec3_scale(vec3 v, real_t s) { return vec3_new(v.x * s, v.y * s, v.z * s); }
/* 取反 */
static FORCE_INLINE vec3 vec3_negate(vec3 v) { return vec3_new(-v.x, -v.y, -v.z); }

/* ---- 测量 ---- */
 /* 点乘 */
static FORCE_INLINE real_t vec3_dot(vec3 a, vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
/* 长度平方 */
static FORCE_INLINE real_t vec3_length_sq(vec3 v) { return vec3_dot(v, v); }
/* 模长 */
static FORCE_INLINE real_t vec3_length(vec3 v) { return math_sqrt(vec3_length_sq(v)); }
/* 距离平方 */
static FORCE_INLINE real_t vec3_distance_sq(vec3 a, vec3 b) { return vec3_length_sq(vec3_sub(a, b)); }
/* 距离 */
static FORCE_INLINE real_t vec3_distance(vec3 a, vec3 b) { return math_sqrt(vec3_distance_sq(a, b)); }

/* ---- 归一化与插值 ---- */
 /* 归一化 */
static FORCE_INLINE vec3 vec3_normalize(vec3 v) {
    real_t len = vec3_length(v);
    return (len > MATH_EPSILON) ? vec3_scale(v, 1.0f / len) : vec3_zero();
}
/* 归一化失败时返回 fallback
 * 注意: fallback 原样返回, 函数不做任何归一化或校验; 
 *       若调用方需要单位向量结果, 请自行保证 fallback 为单位向量 */
static FORCE_INLINE vec3 vec3_normalize_or(vec3 v, vec3 fallback) {
    real_t len = vec3_length(v);
    return (len > MATH_EPSILON) ? vec3_scale(v, 1.0f / len) : fallback;
}
/* 线性插值 */
static FORCE_INLINE vec3 vec3_lerp(vec3 a, vec3 b, real_t t)
{
    return vec3_new(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t);
}
/* 方向插值(结果为单位向量)
 * 注意: 当 a 与 b 接近反向(a ≈ -b)时, 插值中间结果可能退化为零向量, 
 *       此时 normalize 返回零向量; 若需稳定的方向插值请改用 slerp */
static FORCE_INLINE vec3 vec3_nlerp(vec3 a, vec3 b, real_t t) {
    return vec3_normalize(vec3_lerp(a, b, t));
}

/* ---- 极值与比较 ---- */
 /* 最小边界 */
static FORCE_INLINE vec3 vec3_min(vec3 a, vec3 b)
{
    return vec3_new(math_min(a.x, b.x), math_min(a.y, b.y), math_min(a.z, b.z));
}
/* 最大边界 */
static FORCE_INLINE vec3 vec3_max(vec3 a, vec3 b)
{
    return vec3_new(math_max(a.x, b.x), math_max(a.y, b.y), math_max(a.z, b.z));
}
/* 限制范围 */
static FORCE_INLINE vec3 vec3_clamp(vec3 v, vec3 min, vec3 max)
{
    return vec3_new(math_clamp(v.x, min.x, max.x), math_clamp(v.y, min.y, max.y), math_clamp(v.z, min.z, max.z));
}
/* 安全相等 */
static FORCE_INLINE bool_t vec3_is_equal(vec3 a, vec3 b) {
    return (math_is_equal(a.x, b.x) && math_is_equal(a.y, b.y) && math_is_equal(a.z, b.z)) ? 1 : 0;
}

/* ---- 3D 专属进阶 ---- */
 /* 叉乘 (求法线) */
static FORCE_INLINE vec3 vec3_cross(vec3 a, vec3 b)
{
    return vec3_new(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
/* 反射 (光照/物理) */
static FORCE_INLINE vec3 vec3_reflect(vec3 incident, vec3 normal)
{
    return vec3_sub(incident, vec3_scale(normal, 2.0f * vec3_dot(incident, normal)));
}
/* 向量投影 */
static FORCE_INLINE vec3 vec3_project(vec3 a, vec3 b) {
    real_t denom = vec3_dot(b, b);
    return (denom < MATH_EPSILON * MATH_EPSILON) ? vec3_zero() : vec3_scale(b, vec3_dot(a, b) / denom);
}
/* 向量夹角
 * 注意: 分别判断两向量长度, 避免用长度^2与 MATH_EPSILON 直接比较造成量纲不一致 */
static FORCE_INLINE real_t vec3_angle(vec3 a, vec3 b) {
    real_t la = vec3_length(a), lb = vec3_length(b);
    return (la < MATH_EPSILON || lb < MATH_EPSILON) ? 0.0f
        : math_acos(math_clamp(vec3_dot(a, b) / (la * lb), -1.0f, 1.0f));
}
/* 平滑追踪
 * 注意: max_step 应为非负值; 若传入负值将沿反方向远离 target */
static FORCE_INLINE vec3 vec3_move_towards(vec3 current, vec3 target, real_t max_step) {
    vec3 diff = vec3_sub(target, current);
    real_t dist = vec3_length(diff);
    return (dist <= max_step || dist < MATH_EPSILON)
        ? vec3_new(target.x, target.y, target.z)
        : vec3_add(current, vec3_scale(diff, max_step / dist));
}
/* 绕单位轴旋转(Rodrigues 公式), axis 必须已归一化 */
static FORCE_INLINE vec3 vec3_rotate_axis(vec3 v, vec3 axis, real_t angle) {
    real_t c = math_cos(angle), s = math_sin(angle);
    return vec3_add(
        vec3_add(vec3_scale(v, c), vec3_scale(vec3_cross(axis, v), s)),
        vec3_scale(axis, vec3_dot(axis, v) * (1.0f - c))
    );
}
/* 由单位法线 n 构造正交基 t|b(TBN / 相机用)
 * 注意: 要求 n 为单位向量; 若 n 为零向量, t|b 均退化为零向量且不报错
 *       up 的切换阈值为 MATH_UP_AXIS_SWITCH(默认 0.99f), 切换点附近基向量会跳变; 
 *       若用于需平滑过渡的相机基, 可改用按最大分量选择 up */
static FORCE_INLINE void vec3_orthonormal_basis(vec3 n, vec3* t, vec3* b) {
    vec3 up = (math_abs(n.y) < MATH_UP_AXIS_SWITCH) ? vec3_new(0.0f, 1.0f, 0.0f) : vec3_new(1.0f, 0.0f, 0.0f);
    *t = vec3_normalize(vec3_cross(up, n));
    *b = vec3_cross(n, *t);
}

/* ---- 逐元素工具 ---- */
/* 逐元素绝对值 */
static FORCE_INLINE vec3 vec3_abs(vec3 v) {
    return vec3_new(math_abs(v.x), math_abs(v.y), math_abs(v.z));
}
/* 逐元素符号(-1 / 0 / +1) */
static FORCE_INLINE vec3 vec3_sign(vec3 v) {
    return vec3_new((real_t)(v.x > 0.0f) - (real_t)(v.x < 0.0f),
        (real_t)(v.y > 0.0f) - (real_t)(v.y < 0.0f),
        (real_t)(v.z > 0.0f) - (real_t)(v.z < 0.0f));
}
/* 填充 x/y/z, w padding 置 0 */
static FORCE_INLINE vec3 vec3_splat(real_t s) { return vec3_new(s, s, s); }

/* ---- 逐元素取整与步进 ---- */
/* 逐元素对齐到步长 */
static FORCE_INLINE vec3 vec3_snap(vec3 v, real_t step) {
    return vec3_new(math_snap(v.x, step), math_snap(v.y, step), math_snap(v.z, step));
}
/* 逐元素向下取整 */
static FORCE_INLINE vec3 vec3_floor(vec3 v) { return vec3_new(math_floor(v.x), math_floor(v.y), math_floor(v.z)); }
/* 逐元素向上取整 */
static FORCE_INLINE vec3 vec3_ceil(vec3 v) { return vec3_new(math_ceil(v.x), math_ceil(v.y), math_ceil(v.z)); }
/* 逐元素四舍五入 */
static FORCE_INLINE vec3 vec3_round(vec3 v) { return vec3_new(math_round(v.x), math_round(v.y), math_round(v.z)); }
/* 限制长度不超过 max_len */
static FORCE_INLINE vec3 vec3_clamp_length(vec3 v, real_t max_len) {
    real_t len = vec3_length(v);
    if (len <= max_len || len < MATH_EPSILON) return v;
    return vec3_scale(v, max_len / len);
}

/* ---- 方向操作 ---- */
/* 单位化别名; 零向量返回零向量 */
static FORCE_INLINE vec3 vec3_direction(vec3 v) { return vec3_normalize(v); }
/* 标量三重积 a·(b×c) */
static FORCE_INLINE real_t vec3_triple(vec3 a, vec3 b, vec3 c) { return vec3_dot(a, vec3_cross(b, c)); }
/* 返回任意一个与 v 垂直的单位向量; v 为零向量时返回零向量 */
static FORCE_INLINE vec3 vec3_any_perpendicular(vec3 v) {
    real_t ax = math_abs(v.x), ay = math_abs(v.y), az = math_abs(v.z);
    vec3 axis = (ax <= ay && ax <= az) ? vec3_new(1.0f, 0.0f, 0.0f)
        : (ay <= az ? vec3_new(0.0f, 1.0f, 0.0f) : vec3_new(0.0f, 0.0f, 1.0f));
    return vec3_normalize(vec3_cross(v, axis));
}
/* 方向球面插值; 两端先归一化, 反向时退化为绕垂直轴旋转
 * 注意: 零向量输入返回零向量 */
static FORCE_INLINE vec3 vec3_slerp(vec3 a, vec3 b, real_t t) {
    vec3 na = vec3_normalize(a), nb = vec3_normalize(b);
    real_t d = math_clamp(vec3_dot(na, nb), -1.0f, 1.0f);
    if (d > MATH_SLERP_SNAP) return vec3_normalize(vec3_lerp(na, nb, t));
    if (d < -MATH_SLERP_SNAP) return vec3_rotate_axis(na, vec3_any_perpendicular(na), MATH_PI * t);
    real_t theta = math_acos(d);
    vec3 rel = vec3_normalize(vec3_sub(nb, vec3_scale(na, d)));
    return vec3_add(vec3_scale(na, math_cos(theta * t)), vec3_scale(rel, math_sin(theta * t)));
}
/* 朝 target 方向旋转, 单次角步长不超过 max_angle 弧度; 已到达时返回 target */
static FORCE_INLINE vec3 vec3_rotate_towards(vec3 current, vec3 target, real_t max_angle) {
    vec3 cur = vec3_normalize(current), tg = vec3_normalize(target);
    real_t ang = math_acos(math_clamp(vec3_dot(cur, tg), -1.0f, 1.0f));
    if (ang <= max_angle || ang < MATH_EPSILON) return tg;
    return vec3_slerp(cur, tg, max_angle / ang);
}
/* 以 axis 为正向的有符号夹角 [-PI, PI]; axis 无需归一化 */
static FORCE_INLINE real_t vec3_signed_angle(vec3 a, vec3 b, vec3 axis) {
    real_t ang = vec3_angle(a, b);
    return (vec3_dot(vec3_cross(a, b), axis) < 0.0f) ? -ang : ang;
}
/* 投影到过原点、法线为 normal 的平面上; normal 需为单位向量 */
static FORCE_INLINE vec3 vec3_project_on_plane(vec3 v, vec3 normal) {
    return vec3_sub(v, vec3_scale(normal, vec3_dot(v, normal)));
}

/* ---- 判定与统计 ---- */
/* 是否所有分量有限 */
static FORCE_INLINE bool_t vec3_is_finite(vec3 v) {
    return (math_is_finite(v.x) && math_is_finite(v.y) && math_is_finite(v.z)) ? 1 : 0;
}
/* 是否为零向量(逐分量为 0) */
static FORCE_INLINE bool_t vec3_is_zero(vec3 v) {
    return (v.x == 0.0f && v.y == 0.0f && v.z == 0.0f) ? 1 : 0;
}
/* 是否已单位化 */
static FORCE_INLINE bool_t vec3_is_normalized(vec3 v) {
    return (math_abs(vec3_length_sq(v) - 1.0f) < MATH_EPSILON) ? 1 : 0;
}
/* 最大分量 */
static FORCE_INLINE real_t vec3_max_component(vec3 v) { return math_max(v.x, math_max(v.y, v.z)); }
/* 最小分量 */
static FORCE_INLINE real_t vec3_min_component(vec3 v) { return math_min(v.x, math_min(v.y, v.z)); }


EXTERN_C_END
#endif