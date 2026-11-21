#ifndef MATH_CONSTANTS_H
/* 数学常量 */
#define MATH_CONSTANTS_H

/* 无穷大取自标准库, 避免 MSVC 把常量除零判为编译错误(C2124) */
#include <math.h>

/* 圆周率 */
#define MATH_PI       3.14159265358979323846f
#define MATH_TWO_PI   6.28318530717958647692f
#define MATH_HALF_PI  1.57079632679489661923f
#define MATH_QUARTER_PI 0.78539816339744830961f
/* 2π 别名(周期) */
#define MATH_TAU      MATH_TWO_PI

/* 自然底数 */
#define MATH_E        2.71828182845904523536f

/* 根号2 与 根号1/2 */
#define MATH_SQRT2    1.41421356237309504880f
#define MATH_SQRT1_2  0.70710678118654752440f

/* 精度极限(用于判断浮点数是否为0|是否归一化等) */
#define MATH_EPSILON  1e-6f

/* 角度与弧度转换 */
#define MATH_DEG2RAD  0.01745329251994329577f
#define MATH_RAD2DEG  57.2957795130823208768f
/* π 的倒数 */
#define MATH_INV_PI   0.31830988618379067154f

/* 无穷大 */
#define MATH_INFINITY  HUGE_VALF
#define MATH_NEG_INFINITY (-HUGE_VALF)

/* ---- 插值与判定阈值 ---- */
/* 方向/四元数插值退化为线性插值的点积阈值 */
#define MATH_SLERP_SNAP        0.9995f
/* 视为同一旋转/方向的点积阈值 */
#define MATH_IDENTITY_SNAP     0.9999f
/* 欧拉角万向锁判定(yaw 余弦阈值) */
#define MATH_GIMBAL_EPSILON    1e-3f
/* 构造正交基/朝向时切换 up 轴的分量阈值 */
#define MATH_UP_AXIS_SWITCH    0.99f

/* ---- 平滑跟随(临界阻尼) ---- */
/* 最小平滑时长(秒) */
#define MATH_SMOOTHDAMP_MIN    0.0001f
/* 阻尼角频率系数 */
#define MATH_SMOOTHDAMP_OMEGA  2.0f
/* 指数近似多项式系数 */
#define MATH_SMOOTHDAMP_C1     0.48f
#define MATH_SMOOTHDAMP_C2     0.235f

/* ---- 颜色 ---- */
/* 8 位通道满值 */
#define MATH_COLOR_BYTE_MAX    255.0f
/* sRGB 转换分段点与系数 */
#define MATH_SRGB_KNEE         0.04045f
#define MATH_SRGB_KNEE_INV     0.0031308f
#define MATH_SRGB_SLOPE        12.92f
#define MATH_SRGB_OFFSET       0.055f
#define MATH_SRGB_SCALE        1.055f
#define MATH_SRGB_GAMMA        2.4f
/* 相对亮度权重(Rec.709) */
#define MATH_LUMA_R            0.2126f
#define MATH_LUMA_G            0.7152f
#define MATH_LUMA_B            0.0722f
/* HSV 色环分段数(整数用于扇区下标, 浮点用于缩放) */
#define MATH_HSV_SECTOR_COUNT  6
#define MATH_HSV_SECTORS       6.0f
/* HSV 色环中绿/蓝所在扇区的偏移 */
#define MATH_HSV_SECTOR_OFFSET_G 2.0f
#define MATH_HSV_SECTOR_OFFSET_B 4.0f

/* ---- 位级换算 ---- */
/* 2^24 与 2^-24 (24 位随机采样/半精度次正规数) */
#define MATH_POW2_24           5.9604644775390625e-08f
#define MATH_POW2_24_INV       16777216.0f
/* Q16.16 定点数小数位 */
#define MATH_FIXED_SHIFT       16

/* ---- 随机(PCG32) ---- */
/* LCG 乘数 */
#define MATH_PCG_MULT          6364136223846793005ULL
/* 拒绝采样最大尝试次数 */
#define MATH_RANDOM_REJECT_TRIES 16

/* ---- 噪声哈希 ---- */
/* 三维哈希乘数 */
#define MATH_NOISE_HASH_X      0x9E3779B97F4A7C15ULL
#define MATH_NOISE_HASH_Y      0xC2B2AE3D27D4EB4FULL
#define MATH_NOISE_HASH_Z      0x165667B19E3779F9ULL
/* 五次平滑插值多项式系数 t^3*(t*(A*t-B)+C) */
#define MATH_NOISE_FADE_A      6.0f
#define MATH_NOISE_FADE_B      15.0f
#define MATH_NOISE_FADE_C      10.0f

/* ---- 缓动 ---- */
/* back 缓动超调量 */
#define MATH_BACK_OVERSHOOT    1.70158f
/* bounce 缓动分段系数 */
#define MATH_BOUNCE_N          2.75f
#define MATH_BOUNCE_SCALE      7.5625f
#define MATH_BOUNCE_T1         1.5f
#define MATH_BOUNCE_T2         2.25f
#define MATH_BOUNCE_T3         2.625f
#define MATH_BOUNCE_H1         0.75f
#define MATH_BOUNCE_H2         0.9375f
#define MATH_BOUNCE_H3         0.984375f

/* ---- 几何 ---- */
/* 视锥平面数量 */
#define MATH_FRUSTUM_PLANE_COUNT 6
/* 视锥平面下标(与 frustum_from_matrix 的写入顺序一致) */
#define MATH_FRUSTUM_NEAR    0
#define MATH_FRUSTUM_FAR     1
#define MATH_FRUSTUM_LEFT    2
#define MATH_FRUSTUM_RIGHT   3
#define MATH_FRUSTUM_TOP     4
#define MATH_FRUSTUM_BOTTOM  5
/* 视锥角点数量(近 4 + 远 4) */
#define MATH_FRUSTUM_CORNER_COUNT 8
/* AABB 角点数量 */
#define MATH_AABB_CORNER_COUNT 8
/* 噪声梯度数量(二维 8 方向 / 三维 12 棱) */
#define MATH_NOISE_GRAD2_COUNT 8
#define MATH_NOISE_GRAD3_COUNT 12

/* ---- 颜色通道 ---- */
/* 每通道位数与掩码 */
#define MATH_COLOR_CHANNEL_BITS 8
#define MATH_COLOR_CHANNEL_MASK ((1u << MATH_COLOR_CHANNEL_BITS) - 1u)
/* 各通道在打包整数中的移位 */
#define MATH_COLOR_SHIFT_R 24
#define MATH_COLOR_SHIFT_G 16
#define MATH_COLOR_SHIFT_B 8
#define MATH_COLOR_SHIFT_A 0
/* 十六进制颜色: 单位数字位数与最大字符数(#RRGGBBAA) */
#define MATH_COLOR_NIBBLE_BITS 4
#define MATH_COLOR_HEX_MAX_DIGITS 8
/* "#RRGGBBAA" + '\0' */
#define MATH_COLOR_HEX_STRING_SIZE 10

/* 矩阵维度与元素数 (供 mat2/mat3/mat4 使用) */
#define MAT2_DIM        2
#define MAT3_DIM        3
#define MAT4_DIM        4

#define MAT2_ELEM_COUNT (MAT2_DIM * MAT2_DIM)   /* 4  */
#define MAT3_ELEM_COUNT (MAT3_DIM * MAT3_DIM)   /* 9  */
#define MAT4_ELEM_COUNT (MAT4_DIM * MAT4_DIM)   /* 16 */


#endif