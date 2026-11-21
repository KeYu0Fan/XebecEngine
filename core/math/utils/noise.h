#ifndef MATH_UTILS_NOISE_H
/* 程序化噪声
 * 全部基于整数哈希, 不使用查找表, 同 seed 在任何平台结果一致
 * 输出范围: 单层噪声约 [-1, 1]; fbm/turbulence 已按振幅和归一化, 同样约 [-1, 1]
 */
#define MATH_UTILS_NOISE_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/math/constants.h"
#include "core/math/vector/vector.h"
EXTERN_C_BEGIN


/* 噪声状态 */
typedef struct { qty_t seed; } noise;

/* 整数坐标哈希 */
static FORCE_INLINE qty_t noise_hash(qty_t seed, sqty_t x, sqty_t y, sqty_t z) {
    qty_t h = seed;
    h ^= (qty_t)x * MATH_NOISE_HASH_X; h = (h << 31) | (h >> 33);
    h ^= (qty_t)y * MATH_NOISE_HASH_Y; h = (h << 27) | (h >> 37);
    h ^= (qty_t)z * MATH_NOISE_HASH_Z; h = (h << 29) | (h >> 35);
    h *= MATH_NOISE_HASH_X;
    h ^= h >> 32;
    return h;
}
/* 哈希 -> [0,1) 浮点 */
static FORCE_INLINE real_t noise_hash_float(qty_t seed, sqty_t x, sqty_t y, sqty_t z) {
    return (real_t)(noise_hash(seed, x, y, z) >> 40) * (1.0f / MATH_POW2_24_INV);
}
/* 五次平滑插值权重 t^3*(t*(A*t-B)+C) */
static FORCE_INLINE real_t noise_fade(real_t t) {
    return t * t * t * (t * (t * MATH_NOISE_FADE_A - MATH_NOISE_FADE_B) + MATH_NOISE_FADE_C);
}
/* 二维梯度(8 方向) */
static FORCE_INLINE void noise_grad2(qty_t h, real_t* gx, real_t* gy) {
    switch (h & (MATH_NOISE_GRAD2_COUNT - 1u)) {
    case 0: *gx = 1.0f; *gy = 0.0f; break;
    case 1: *gx = -1.0f; *gy = 0.0f; break;
    case 2: *gx = 0.0f; *gy = 1.0f; break;
    case 3: *gx = 0.0f; *gy = -1.0f; break;
    case 4: *gx = MATH_SQRT1_2; *gy = MATH_SQRT1_2; break;
    case 5: *gx = -MATH_SQRT1_2; *gy = MATH_SQRT1_2; break;
    case 6: *gx = MATH_SQRT1_2; *gy = -MATH_SQRT1_2; break;
    default: *gx = -MATH_SQRT1_2; *gy = -MATH_SQRT1_2; break;
    }
}
/* 三维梯度(立方体 12 条棱方向) */
static FORCE_INLINE void noise_grad3(qty_t h, real_t* gx, real_t* gy, real_t* gz) {
    switch (h % MATH_NOISE_GRAD3_COUNT) {
    case 0: *gx = 1.0f; *gy = 1.0f; *gz = 0.0f; break;
    case 1: *gx = -1.0f; *gy = 1.0f; *gz = 0.0f; break;
    case 2: *gx = 1.0f; *gy = -1.0f; *gz = 0.0f; break;
    case 3: *gx = -1.0f; *gy = -1.0f; *gz = 0.0f; break;
    case 4: *gx = 1.0f; *gy = 0.0f; *gz = 1.0f; break;
    case 5: *gx = -1.0f; *gy = 0.0f; *gz = 1.0f; break;
    case 6: *gx = 1.0f; *gy = 0.0f; *gz = -1.0f; break;
    case 7: *gx = -1.0f; *gy = 0.0f; *gz = -1.0f; break;
    case 8: *gx = 0.0f; *gy = 1.0f; *gz = 1.0f; break;
    case 9: *gx = 0.0f; *gy = -1.0f; *gz = 1.0f; break;
    case 10: *gx = 0.0f; *gy = 1.0f; *gz = -1.0f; break;
    default: *gx = 0.0f; *gy = -1.0f; *gz = -1.0f; break;
    }
}
/* 单角点梯度点积 */
static FORCE_INLINE real_t noise_dot3(qty_t seed, sqty_t x, sqty_t y, sqty_t z, real_t fx, real_t fy, real_t fz) {
    real_t gx, gy, gz;
    noise_grad3(noise_hash(seed, x, y, z), &gx, &gy, &gz);
    return gx * fx + gy * fy + gz * fz;
}

/* ---- 值噪声 ---- */
/* 二维值噪声, 范围 [-1,1] */
static FORCE_INLINE real_t noise_value_2d(noise* n, real_t x, real_t y) {
    sqty_t ix = (sqty_t)math_floor(x), iy = (sqty_t)math_floor(y);
    real_t fx = x - (real_t)ix, fy = y - (real_t)iy;
    real_t u = noise_fade(fx), v = noise_fade(fy);
    real_t a = noise_hash_float(n->seed, ix, iy, 0), b = noise_hash_float(n->seed, ix + 1, iy, 0);
    real_t c = noise_hash_float(n->seed, ix, iy + 1, 0), d = noise_hash_float(n->seed, ix + 1, iy + 1, 0);
    return (math_lerp(math_lerp(a, b, u), math_lerp(c, d, u), v) * 2.0f) - 1.0f;
}
/* 三维值噪声, 范围 [-1,1] */
static FORCE_INLINE real_t noise_value_3d(noise* n, real_t x, real_t y, real_t z) {
    sqty_t ix = (sqty_t)math_floor(x), iy = (sqty_t)math_floor(y), iz = (sqty_t)math_floor(z);
    real_t fx = x - (real_t)ix, fy = y - (real_t)iy, fz = z - (real_t)iz;
    real_t u = noise_fade(fx), v = noise_fade(fy), w = noise_fade(fz);
    real_t c000 = noise_hash_float(n->seed, ix, iy, iz), c100 = noise_hash_float(n->seed, ix + 1, iy, iz);
    real_t c010 = noise_hash_float(n->seed, ix, iy + 1, iz), c110 = noise_hash_float(n->seed, ix + 1, iy + 1, iz);
    real_t c001 = noise_hash_float(n->seed, ix, iy, iz + 1), c101 = noise_hash_float(n->seed, ix + 1, iy, iz + 1);
    real_t c011 = noise_hash_float(n->seed, ix, iy + 1, iz + 1), c111 = noise_hash_float(n->seed, ix + 1, iy + 1, iz + 1);
    real_t a = math_lerp(math_lerp(c000, c100, u), math_lerp(c010, c110, u), v);
    real_t b = math_lerp(math_lerp(c001, c101, u), math_lerp(c011, c111, u), v);
    return (math_lerp(a, b, w) * 2.0f) - 1.0f;
}

/* ---- 梯度噪声 ---- */
/* 二维 Perlin 噪声, 范围约 [-1,1] */
static FORCE_INLINE real_t noise_perlin_2d(noise* n, real_t x, real_t y) {
    sqty_t ix = (sqty_t)math_floor(x), iy = (sqty_t)math_floor(y);
    real_t fx = x - (real_t)ix, fy = y - (real_t)iy;
    real_t u = noise_fade(fx), v = noise_fade(fy), gx, gy, a, b, c, d;
    noise_grad2(noise_hash(n->seed, ix, iy, 0), &gx, &gy);       a = gx * fx + gy * fy;
    noise_grad2(noise_hash(n->seed, ix + 1, iy, 0), &gx, &gy);   b = gx * (fx - 1.0f) + gy * fy;
    noise_grad2(noise_hash(n->seed, ix, iy + 1, 0), &gx, &gy);   c = gx * fx + gy * (fy - 1.0f);
    noise_grad2(noise_hash(n->seed, ix + 1, iy + 1, 0), &gx, &gy); d = gx * (fx - 1.0f) + gy * (fy - 1.0f);
    return math_lerp(math_lerp(a, b, u), math_lerp(c, d, u), v);
}
/* 三维 Perlin 噪声, 范围约 [-1,1] */
static FORCE_INLINE real_t noise_perlin_3d(noise* n, real_t x, real_t y, real_t z) {
    sqty_t ix = (sqty_t)math_floor(x), iy = (sqty_t)math_floor(y), iz = (sqty_t)math_floor(z);
    real_t fx = x - (real_t)ix, fy = y - (real_t)iy, fz = z - (real_t)iz;
    real_t u = noise_fade(fx), v = noise_fade(fy), w = noise_fade(fz);
    real_t c000 = noise_dot3(n->seed, ix, iy, iz, fx, fy, fz);
    real_t c100 = noise_dot3(n->seed, ix + 1, iy, iz, fx - 1.0f, fy, fz);
    real_t c010 = noise_dot3(n->seed, ix, iy + 1, iz, fx, fy - 1.0f, fz);
    real_t c110 = noise_dot3(n->seed, ix + 1, iy + 1, iz, fx - 1.0f, fy - 1.0f, fz);
    real_t c001 = noise_dot3(n->seed, ix, iy, iz + 1, fx, fy, fz - 1.0f);
    real_t c101 = noise_dot3(n->seed, ix + 1, iy, iz + 1, fx - 1.0f, fy, fz - 1.0f);
    real_t c011 = noise_dot3(n->seed, ix, iy + 1, iz + 1, fx, fy - 1.0f, fz - 1.0f);
    real_t c111 = noise_dot3(n->seed, ix + 1, iy + 1, iz + 1, fx - 1.0f, fy - 1.0f, fz - 1.0f);
    return math_lerp(
        math_lerp(math_lerp(c000, c100, u), math_lerp(c010, c110, u), v),
        math_lerp(math_lerp(c001, c101, u), math_lerp(c011, c111, u), v), w);
}

/* ---- 分形叠加 ---- */
/* 二维 fbm: octaves 层, 每层频率 * lacunarity, 振幅 * gain; 结果按振幅和归一化
 * 注意: octaves <= 0 时返回 0 */
static FORCE_INLINE real_t noise_fbm_2d(noise* n, real_t x, real_t y, sqty_t octaves, real_t lacunarity, real_t gain) {
    real_t sum = 0.0f, amp = 1.0f, norm = 0.0f;
    sqty_t i;
    for (i = 0; i < octaves; i++) {
        sum += noise_perlin_2d(n, x, y) * amp;
        norm += amp;
        x *= lacunarity; y *= lacunarity;
        amp *= gain;
    }
    return (norm > MATH_EPSILON) ? sum / norm : 0.0f;
}
/* 三维 fbm */
static FORCE_INLINE real_t noise_fbm_3d(noise* n, real_t x, real_t y, real_t z, sqty_t octaves, real_t lacunarity, real_t gain) {
    real_t sum = 0.0f, amp = 1.0f, norm = 0.0f;
    sqty_t i;
    for (i = 0; i < octaves; i++) {
        sum += noise_perlin_3d(n, x, y, z) * amp;
        norm += amp;
        x *= lacunarity; y *= lacunarity; z *= lacunarity;
        amp *= gain;
    }
    return (norm > MATH_EPSILON) ? sum / norm : 0.0f;
}
/* 二维湍流(取绝对值叠加), 范围 [0,1] */
static FORCE_INLINE real_t noise_turbulence_2d(noise* n, real_t x, real_t y, sqty_t octaves, real_t lacunarity, real_t gain) {
    real_t sum = 0.0f, amp = 1.0f, norm = 0.0f;
    sqty_t i;
    for (i = 0; i < octaves; i++) {
        sum += math_abs(noise_perlin_2d(n, x, y)) * amp;
        norm += amp;
        x *= lacunarity; y *= lacunarity;
        amp *= gain;
    }
    return (norm > MATH_EPSILON) ? sum / norm : 0.0f;
}
/* 三维湍流, 范围 [0,1] */
static FORCE_INLINE real_t noise_turbulence_3d(noise* n, real_t x, real_t y, real_t z, sqty_t octaves, real_t lacunarity, real_t gain) {
    real_t sum = 0.0f, amp = 1.0f, norm = 0.0f;
    sqty_t i;
    for (i = 0; i < octaves; i++) {
        sum += math_abs(noise_perlin_3d(n, x, y, z)) * amp;
        norm += amp;
        x *= lacunarity; y *= lacunarity; z *= lacunarity;
        amp *= gain;
    }
    return (norm > MATH_EPSILON) ? sum / norm : 0.0f;
}


EXTERN_C_END
#endif