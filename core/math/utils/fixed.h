#ifndef MATH_UTILS_FIXED_H
/* 定点数 (Q16.16)
 * 用于确定性物理/网络同步: 全部运算基于整型, 结果与浮点环境无关
 * 表示范围约 [-32768, 32767.99998], 精度 1/65536
 */
#define MATH_UTILS_FIXED_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/math/constants.h"
#include "core/math/STATIC/base.h"
EXTERN_C_BEGIN


/* Q16.16 定点数: 恰好 32 位 */
typedef s32_t fixed_t;

/* 小数位数(定义在 constants.h, 便于统一替换) */
#define FIXED_SHIFT MATH_FIXED_SHIFT
/* 1.0 的定点表示 */
#define FIXED_ONE   ((fixed_t)1 << FIXED_SHIFT)

/* ---- 转换 ---- */
/* 浮点 -> 定点, 四舍五入; 超出 32 位范围时饱和 */
static FORCE_INLINE fixed_t fixed_from_float(real_t x) {
    real_t v = math_round(x * (real_t)FIXED_ONE);
    if (v > 2147483520.0f) v = 2147483520.0f;      /* 2^31 向下取整到 64 的倍数 */
    if (v < -2147483648.0f) v = -2147483648.0f;
    return (fixed_t)v;
}
/* 定点 -> 浮点 */
static FORCE_INLINE real_t fixed_to_float(fixed_t v) { return (real_t)v * (1.0f / (real_t)FIXED_ONE); }
/* 整数 -> 定点; 超出 Q16.16 整数范围时饱和 */
static FORCE_INLINE fixed_t fixed_from_int(sqty_t v) {
    if (v > 32767) v = 32767;
    if (v < -32768) v = -32768;
    return (fixed_t)(v * (sqty_t)FIXED_ONE);
}
/* 定点 -> 整数(向零截断) */
static FORCE_INLINE sqty_t fixed_to_int(fixed_t v) { return (sqty_t)(v / FIXED_ONE); }

/* ---- 运算 ---- */
/* 加法 */
static FORCE_INLINE fixed_t fixed_add(fixed_t a, fixed_t b) { return (fixed_t)(a + b); }
/* 减法 */
static FORCE_INLINE fixed_t fixed_sub(fixed_t a, fixed_t b) { return (fixed_t)(a - b); }
/* 乘法 */
static FORCE_INLINE fixed_t fixed_mul(fixed_t a, fixed_t b) {
    return (fixed_t)(((sqty_t)a * (sqty_t)b) >> FIXED_SHIFT);
}
/* 除法; b 为 0 时返回 0 */
static FORCE_INLINE fixed_t fixed_div(fixed_t a, fixed_t b) {
    if (b == 0) return 0;
    return (fixed_t)((((sqty_t)a) << FIXED_SHIFT) / (sqty_t)b);
}
/* 绝对值 */
static FORCE_INLINE fixed_t fixed_abs(fixed_t v) { return (v < 0) ? (fixed_t)(-v) : v; }
/* 最小值 */
static FORCE_INLINE fixed_t fixed_min(fixed_t a, fixed_t b) { return (a < b) ? a : b; }
/* 最大值 */
static FORCE_INLINE fixed_t fixed_max(fixed_t a, fixed_t b) { return (a > b) ? a : b; }
/* 限制到 [lo, hi] */
static FORCE_INLINE fixed_t fixed_clamp(fixed_t v, fixed_t lo, fixed_t hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}
/* 符号: 负数/零/正数分别返回 -1/0/+1 的定点表示 */
static FORCE_INLINE fixed_t fixed_sign(fixed_t v) {
    return (v > 0) ? FIXED_ONE : ((v < 0) ? -FIXED_ONE : 0);
}
/* 平方根(整数逐位法); v <= 0 时返回 0 */
static FORCE_INLINE fixed_t fixed_sqrt(fixed_t v) {
    sqty_t val, res, bit;
    if (v <= 0) return 0;
    val = (sqty_t)v << FIXED_SHIFT;
    res = 0;
    bit = (sqty_t)1 << 62;
    while (bit > val) bit >>= 2;
    while (bit != 0) {
        if (val >= res + bit) {
            val -= res + bit;
            res = (res >> 1) + bit;
        }
        else res >>= 1;
        bit >>= 2;
    }
    return (fixed_t)res;
}
/* 线性插值 a + (b - a) * t; t 为定点数 [0, FIXED_ONE] */
static FORCE_INLINE fixed_t fixed_lerp(fixed_t a, fixed_t b, fixed_t t) {
    return fixed_add(a, fixed_mul(fixed_sub(b, a), t));
}


EXTERN_C_END
#endif