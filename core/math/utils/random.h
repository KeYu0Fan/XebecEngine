#ifndef MATH_UTILS_RANDOM_H
/* 随机数 (PCG32)
 * 状态保存在 random 结构体内, 无全局状态, 可多线程各自持有
 * 同一 seed/seq 在任何平台产生同一序列, 可用于确定性模拟
 */
#define MATH_UTILS_RANDOM_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/math/constants.h"
#include "core/math/vector/vector.h"
EXTERN_C_BEGIN


/* 随机数状态 */
typedef struct { qty_t state; qty_t inc; } random;

/* ---- 播种与生成 ---- */
/* 初始化: seed 相同则序列相同, seq 用于区分同 seed 的独立流 */
static FORCE_INLINE void random_seed(random* r, qty_t seed, qty_t seq) {
    r->state = 0;
    r->inc = (seq << 1) | 1;
    r->state = r->state * MATH_PCG_MULT + r->inc;  /* 丢弃第一个输出 */
    r->state += seed;
    r->state = r->state * MATH_PCG_MULT + r->inc;
}
/* 下一个 32 位随机数(值域 2^32) */
static FORCE_INLINE qty_t random_next(random* r) {
    qty_t old = r->state;
    u32_t xorshifted, rot;
    r->state = old * MATH_PCG_MULT + r->inc;
    xorshifted = (u32_t)(((old >> 18) ^ old) >> 27);
    rot = (u32_t)(old >> 59);
    return (qty_t)((xorshifted >> rot) | (xorshifted << ((0u - rot) & 31u)));
}

/* ---- 区间 ---- */
/* 整型区间 [min, max); max <= min 时返回 min */
static FORCE_INLINE qty_t random_range(random* r, qty_t min, qty_t max) {
    if (max <= min) return min;
    return min + random_next(r) % (max - min);
}
/* 浮点 [0, 1) */
static FORCE_INLINE real_t random_float(random* r) {
    return (real_t)(random_next(r) >> 8) * (1.0f / MATH_POW2_24_INV);
}
/* 浮点 [min, max) */
static FORCE_INLINE real_t random_float_range(random* r, real_t min, real_t max) {
    return min + (max - min) * random_float(r);
}
/* 以概率 p 返回 1 */
static FORCE_INLINE bool_t random_bool(random* r, real_t p) {
    return (random_float(r) < p) ? 1 : 0;
}

/* ---- 几何分布 ---- */
/* 单位球面上的均匀方向 */
static FORCE_INLINE vec3 random_unit_vec3(random* r) {
    real_t z = 2.0f * random_float(r) - 1.0f;
    real_t a = MATH_TWO_PI * random_float(r);
    real_t radius = math_sqrt(math_max(0.0f, 1.0f - z * z));
    return vec3_new(radius * math_cos(a), radius * math_sin(a), z);
}
/* 单位圆上的均匀方向 */
static FORCE_INLINE vec2 random_unit_vec2(random* r) {
    return vec2_from_angle(MATH_TWO_PI * random_float(r), 1.0f);
}
/* 单位球内均匀分布的点(拒绝采样, 超过尝试次数后退到球面) */
static FORCE_INLINE vec3 random_in_unit_sphere(random* r) {
    qty_t i;
    for (i = 0; i < MATH_RANDOM_REJECT_TRIES; i++) {
        vec3 p = vec3_new(2.0f * random_float(r) - 1.0f, 2.0f * random_float(r) - 1.0f, 2.0f * random_float(r) - 1.0f);
        if (vec3_length_sq(p) <= 1.0f) return p;
    }
    return random_unit_vec3(r);
}
/* 盒内均匀分布的点; center 为中心, half 为半尺寸 */
static FORCE_INLINE vec3 random_in_box(random* r, vec3 center, vec3 half) {
    return vec3_new(
        center.x + (2.0f * random_float(r) - 1.0f) * half.x,
        center.y + (2.0f * random_float(r) - 1.0f) * half.y,
        center.z + (2.0f * random_float(r) - 1.0f) * half.z);
}
/* 单位圆内均匀分布的点 */
static FORCE_INLINE vec2 random_in_unit_circle(random* r) {
    qty_t i;
    for (i = 0; i < MATH_RANDOM_REJECT_TRIES; i++) {
        vec2 p = vec2_new(2.0f * random_float(r) - 1.0f, 2.0f * random_float(r) - 1.0f);
        if (vec2_length_sq(p) <= 1.0f) return p;
    }
    return vec2_from_angle(MATH_TWO_PI * random_float(r), 1.0f);
}


EXTERN_C_END
#endif