#ifndef MATH_BASE_H
#define MATH_BASE_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/math/constants.h"
#include <math.h>
EXTERN_C_BEGIN


/* ==================== 基础极值|取整与符号(最底层依赖) ==================== */

/* 返回两数中的较小值 */
static FORCE_INLINE real_t math_min(real_t a, real_t b) { return a < b ? a : b; }

/* 返回两数中的较大值 */
static FORCE_INLINE real_t math_max(real_t a, real_t b) { return a > b ? a : b; }

/* 返回浮点数绝对值 */
static FORCE_INLINE real_t math_abs(real_t x) { return x < 0 ? -x : x; }

/* 向下取整 (例如 1.9 变成 1.0) */
static FORCE_INLINE real_t math_floor(real_t x) { return floorf(x); }

/* 向上取整 (例如 1.1 变成 2.0) */
static FORCE_INLINE real_t math_ceil(real_t x) { return ceilf(x); }

/* 四舍五入 */
static FORCE_INLINE real_t math_round(real_t x) { return roundf(x); }

/* 符号函数: 大于0返回1, 小于0返回-1, 接近0返回0 (自带精度防呆) */
static FORCE_INLINE real_t math_sign(real_t x) {
    if (x > MATH_EPSILON) return 1.0f;
    if (x < -MATH_EPSILON) return -1.0f;
    return 0.0f;
}

/* ==================== 基础代数运算 ==================== */

/* 开平方根 */
static FORCE_INLINE real_t math_sqrt(real_t x) { return sqrtf(x); }
/* 直角三角形斜边 sqrt(x²+y²), 中间结果不溢出 */
static FORCE_INLINE real_t math_hypot(real_t x, real_t y) { return hypotf(x, y); }
/* 复制符号: 取 |x| 与 y 的符号 */
static FORCE_INLINE real_t math_copysign(real_t x, real_t y) { return copysignf(x, y); }

/* 快速求平方 (比 math_pow(x, 2) 快无数倍) */
static FORCE_INLINE real_t math_square(real_t x) { return x * x; }

/* 求幂 (x 的 y 次方) */
static FORCE_INLINE real_t math_pow(real_t x, real_t y) { return powf(x, y); }

/* ==================== 三角函数 (入参/出参均为弧度) ==================== */

/* 正弦 */
static FORCE_INLINE real_t math_sin(real_t x) { return sinf(x); }

/* 余弦 */
static FORCE_INLINE real_t math_cos(real_t x) { return cosf(x); }

/* 正切 */
static FORCE_INLINE real_t math_tan(real_t x) { return tanf(x); }

/* 反余弦 (返回 0 到 PI) */
static FORCE_INLINE real_t math_acos(real_t x) { return acosf(x); }

/* 反正弦 (返回 -PI/2 到 PI/2) */
static FORCE_INLINE real_t math_asin(real_t x) { return asinf(x); }

/* 反正切 (返回 -PI/2 到 PI/2) */
static FORCE_INLINE real_t math_atan(real_t x) { return atanf(x); }

/* 双参数反正切 (根据 x,y 符号返回 -PI 到 PI, 算向量夹角必用) */
static FORCE_INLINE real_t math_atan2(real_t y, real_t x) { return atan2f(y, x); }

/* ==================== 范围|插值与进阶 ==================== */

/* 将值限制在 [min, max] 范围内 */
static FORCE_INLINE real_t math_clamp(real_t val, real_t min, real_t max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

/* 线性插值: t 从 0 到 1, 返回 a 到 b 之间的值 */
static FORCE_INLINE real_t math_lerp(real_t a, real_t b, real_t t) {
    return a + (b - a) * t;
}

/* 取小数部分 (例如 3.14 返回 0.14) */
static FORCE_INLINE real_t math_fract(real_t x) {
    return x - math_floor(x);
}

/* 浮点数取模 (常用于角度折返) */
static FORCE_INLINE real_t math_fmod(real_t x, real_t y) {
    return fmodf(x, y);
}

/* 平滑过渡 (S型曲线, 用于光影渐变\相机缓动, 比 lerp 顺滑) */
static FORCE_INLINE real_t math_smoothstep(real_t edge0, real_t edge1, real_t x) {
    real_t t = math_clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

/* 更平滑的五次过渡 6t^5-15t^4+10t^3 (t 限制在 [0,1]) */
static FORCE_INLINE real_t math_smootherstep(real_t t) {
    t = math_clamp(t, 0.0f, 1.0f);
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

/* 安全比较两个浮点数是否相等 (解决 0.1+0.2 != 0.3 的精度问题) */
static FORCE_INLINE bool_t math_is_equal(real_t a, real_t b) {
    return math_abs(a - b) < MATH_EPSILON;
}

/* 循环折返: x 超出 [min,max] 后从另一边折返 (例如地图边界循环) */
static FORCE_INLINE real_t math_wrap(real_t x, real_t min, real_t max) {
    real_t range = max - min;
    if (range < MATH_EPSILON) return min;
    return x - range * math_floor((x - min) / range);
}

/* 平滑追踪: 以固定步长逼近目标 (常用于摄像机跟随) */
static FORCE_INLINE real_t math_move_towards(real_t current, real_t target, real_t max_delta) {
    if (math_abs(target - current) <= max_delta) return target;
    return current + math_sign(target - current) * max_delta;
}

/* ==================== 单位与区间 ==================== */

/* 角度转弧度 */
static FORCE_INLINE real_t math_radians(real_t deg) { return deg * MATH_DEG2RAD; }

/* 弧度转角度 */
static FORCE_INLINE real_t math_degrees(real_t rad) { return rad * MATH_RAD2DEG; }

/* 饱和到 [0, 1] */
static FORCE_INLINE real_t math_saturate(real_t x) {
    if (x < 0.0f) return 0.0f;
    if (x > 1.0f) return 1.0f;
    return x;
}

/* 线性插值, t 限制在 [0, 1] */
static FORCE_INLINE real_t math_lerp_clamped(real_t a, real_t b, real_t t) {
    return math_lerp(a, b, math_saturate(t));
}

/* 区间重映射: x 从 [a0, a1] 映射到 [b0, b1] 不限制范围
 * 注意: a0 == a1 时返回 b0 */
static FORCE_INLINE real_t math_remap(real_t x, real_t a0, real_t a1, real_t b0, real_t b1) {
    real_t d = a1 - a0;
    if (math_abs(d) < MATH_EPSILON) return b0;
    return b0 + (x - a0) * (b1 - b0) / d;
}

/* 区间重映射并限制到目标区间 */
static FORCE_INLINE real_t math_remap_clamped(real_t x, real_t a0, real_t a1, real_t b0, real_t b1) {
    real_t lo = math_min(b0, b1), hi = math_max(b0, b1);
    return math_clamp(math_remap(x, a0, a1, b0, b1), lo, hi);
}

/* 对齐到步长 step 的最近倍数; step 为 0 时原样返回 */
static FORCE_INLINE real_t math_snap(real_t x, real_t step) {
    if (math_abs(step) < MATH_EPSILON) return x;
    return math_floor(x / step + 0.5f) * step;
}

/* 乒乓循环: 在 [0, len] 之间往返, 常用于往返动画
 * 注意: len 非正时返回 0 */
static FORCE_INLINE real_t math_pingpong(real_t x, real_t len) {
    if (len <= 0.0f) return 0.0f;
    real_t t = math_fmod(x, len * 2.0f);
    if (t < 0.0f) t += len * 2.0f;
    return len - math_abs(t - len);
}

/* 角度折返到 [-PI, PI] */
static FORCE_INLINE real_t math_wrap_angle(real_t x) {
    return math_wrap(x + MATH_PI, 0.0f, MATH_TWO_PI) - MATH_PI;
}

/* 两角的最短有符号差 b - a, 范围 (-PI, PI] */
static FORCE_INLINE real_t math_angle_delta(real_t a, real_t b) {
    return math_wrap_angle(b - a);
}

/* 角度插值: 沿最短弧从 a 转到 b */
static FORCE_INLINE real_t math_lerp_angle(real_t a, real_t b, real_t t) {
    return a + math_angle_delta(a, b) * t;
}

/* ==================== 指数|对数|取整 ==================== */

/* 自然指数 */
static FORCE_INLINE real_t math_exp(real_t x) { return expf(x); }
/* 自然对数 x 必须大于 0 */
static FORCE_INLINE real_t math_log(real_t x) { return logf(x); }
/* 以 2 为底对数 x 必须大于 0 */
static FORCE_INLINE real_t math_log2(real_t x) { return log2f(x); }
/* 以 10 为底对数 x 必须大于 0 */
static FORCE_INLINE real_t math_log10(real_t x) { return log10f(x); }
/* 平方根倒数 1/sqrt(x) 物理与归一化常用; x 为 0 时得到 inf */
static FORCE_INLINE real_t math_rsqrt(real_t x) { return 1.0f / sqrtf(x); }
/* 向零取整 */
static FORCE_INLINE real_t math_trunc(real_t x) { return truncf(x); }

/* 向下取整并转整数; 超出整数范围时结果未定义 */
static FORCE_INLINE sqty_t math_floor_int(real_t x) { return (sqty_t)floorf(x); }
/* 向上取整并转整数; 超出整数范围时结果未定义 */
static FORCE_INLINE sqty_t math_ceil_int(real_t x) { return (sqty_t)ceilf(x); }
/* 四舍五入并转整数; 超出整数范围时结果未定义 */
static FORCE_INLINE sqty_t math_round_int(real_t x) { return (sqty_t)roundf(x); }

/* ==================== 数值判定 ==================== */

/* 是否为 NaN; 依赖算术语义, -ffast-math 下结果不可靠 */
static FORCE_INLINE bool_t math_is_nan(real_t x) { return (x == x) ? 0 : 1; }
/* 是否为 ±Inf; 依赖算术语义, -ffast-math 下结果不可靠 */
static FORCE_INLINE bool_t math_is_inf(real_t x) { return (x == x && x - x != 0.0f) ? 1 : 0; }
/* 是否有限(非 NaN 且非 Inf) */
static FORCE_INLINE bool_t math_is_finite(real_t x) { return (x == x && x - x == 0.0f) ? 1 : 0; }
/* 指定容差比较; 差 <= eps 视为相等 */
static FORCE_INLINE bool_t math_is_equal_eps(real_t a, real_t b, real_t eps) {
    return (math_abs(a - b) <= eps) ? 1 : 0;
}

/* ==================== 平滑跟随 ==================== */

/* 指数平滑跟随(临界阻尼), 常用于摄像机/弹簧参数
 * cur 当前值 target 目标值 vel 速度状态(读写) smooth_time 平滑时长 dt 帧时间
 * 返回新值; vel 为 0 时退化为按 smooth_time 的线性逼近
 * 注意: smooth_time 会被限制为不小于 0.0001 */
static FORCE_INLINE real_t math_smoothdamp(real_t cur, real_t target, real_t* vel, real_t smooth_time, real_t dt) {
    if (vel == 0) return math_lerp(cur, target, math_saturate(dt / math_max(smooth_time, MATH_SMOOTHDAMP_MIN)));
    smooth_time = math_max(smooth_time, MATH_SMOOTHDAMP_MIN);
    real_t omega = MATH_SMOOTHDAMP_OMEGA / smooth_time;
    real_t x = omega * dt;
    real_t e = 1.0f / (1.0f + x + MATH_SMOOTHDAMP_C1 * x * x + MATH_SMOOTHDAMP_C2 * x * x * x);
    real_t change = cur - target;
    real_t temp = (*vel + omega * change) * dt;
    *vel = (*vel - omega * temp) * e;
    return target + (change + temp) * e;
}

/* ==================== 整型工具 ==================== */

/* 整型最小值 */
static FORCE_INLINE sqty_t math_imin(sqty_t a, sqty_t b) { return a < b ? a : b; }
/* 整型最大值 */
static FORCE_INLINE sqty_t math_imax(sqty_t a, sqty_t b) { return a > b ? a : b; }

/* 向上取到 2 的幂; 0 与 1 返回 1 */
static FORCE_INLINE qty_t math_next_pow2(qty_t x) {
    if (x <= 1) return 1;
    x--;
    x |= x >> 1;  x |= x >> 2;  x |= x >> 4;
    x |= x >> 8;  x |= x >> 16; x |= x >> 32;
    return x + 1;
}

/* 向下取整的 log2; 0 返回 0 */
static FORCE_INLINE qty_t math_log2_int(qty_t x) {
    qty_t r = 0;
    while (x > 1) { x >>= 1; r++; }
    return r;
}

/* 是否为 2 的幂(0 不是) */
static FORCE_INLINE bool_t math_is_pow2(qty_t x) {
    return (x != 0 && (x & (x - 1)) == 0) ? 1 : 0;
}

/* 整型限制到 [lo, hi] */
static FORCE_INLINE sqty_t math_iclamp(sqty_t v, sqty_t lo, sqty_t hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}


EXTERN_C_END
#endif