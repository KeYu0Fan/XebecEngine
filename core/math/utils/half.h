#ifndef MATH_UTILS_HALF_H
/* 半精度浮点数 (IEEE 754 binary16)
 * 用于 GPU 顶点/颜色打包; 转换按 round-to-nearest-even, 支持 inf/nan/次正规数
 * 位级操作固定为 32 位(binary32), 与 real_t 解耦: real_t 若改为双精度仍可用
 */
#define MATH_UTILS_HALF_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/math/constants.h"
#include "core/math/STATIC/base.h"
EXTERN_C_BEGIN


/* 半精度浮点位模式: 恰好 16 位 */
typedef u16_t half_t;

/* 半精度 -> 单精度; 次正规数按 mant * 2^-24 还原 */
static FORCE_INLINE real_t half_to_float(half_t h) {
    union { u32_t u; f32_t f; } c;
    u32_t sign = ((u32_t)h >> 15) & 0x1u;
    u32_t exp = ((u32_t)h >> 10) & 0x1Fu;
    u32_t mant = (u32_t)h & 0x3FFu;
    if (exp == 0x1Fu) {
        c.u = (sign << 31) | 0x7F800000u | (mant << 13);
    }
    else if (exp != 0u) {
        c.u = (sign << 31) | ((exp + 112u) << 23) | (mant << 13);
    }
    else if (mant == 0u) {
        c.u = sign << 31;
    }
    else {
        c.f = (f32_t)mant * MATH_POW2_24; /* 次正规数 = mant * 2^-24 */
        if (sign) c.f = -c.f;
    }
    return (real_t)c.f;
}

/* 单精度 -> 半精度; 溢出为 inf, 下溢为 0 */
static FORCE_INLINE half_t half_from_float(real_t x) {
    union { f32_t f; u32_t u; } c;
    u32_t sign, exp, mant;
    sqty_t e;
    c.f = (f32_t)x;
    sign = (c.u >> 16) & 0x8000u;
    exp = (c.u >> 23) & 0xFFu;
    mant = c.u & 0x7FFFFFu;
    if (exp == 0xFFu) return (half_t)(sign | 0x7C00u | (mant ? 0x200u : 0u));
    e = (sqty_t)exp - 127 + 15;
    if (e >= 31) return (half_t)(sign | 0x7C00u);
    if (e <= 0) {
        u32_t shift, hm, rem, halfway;
        if (e < -10) return (half_t)sign;
        mant |= 0x800000u;
        shift = (u32_t)(14 - e);
        hm = mant >> shift;
        rem = mant & ((1u << shift) - 1u);
        halfway = 1u << (shift - 1u);
        if (rem > halfway || (rem == halfway && (hm & 1u))) hm++;
        return (half_t)(sign | hm);
    }
    {
        u32_t hm = mant >> 13;
        u32_t rem = mant & 0x1FFFu;
        if (rem > 0x1000u || (rem == 0x1000u && (hm & 1u))) {
            hm++;
            if (hm == 0x400u) {
                hm = 0u;
                e++;
                if (e >= 31) return (half_t)(sign | 0x7C00u);
            }
        }
        return (half_t)(sign | ((u32_t)e << 10) | hm);
    }
}


EXTERN_C_END
#endif