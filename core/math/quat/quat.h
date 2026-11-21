#ifndef MATH_QUAT_QUAT_H
#define MATH_QUAT_QUAT_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/math/scalar.h"
#include "core/math/constants.h"
#include "core/math/vector/vector.h"
#include "core/math/matrix/matrix.h"
EXTERN_C_BEGIN

/* 四元数 (16 字节), 约定 (x, y, z, w) 且 w 为标量部
 * 旋转约定: 右手定则(右手逆时针), 与 mat3_rotate_axis 一致
 * 欧拉角约定: 与 mat4_rotate_euler 一致, 即 R = Rz(roll) * Ry(yaw) * Rx(pitch)
 * 矩阵互操作(mats/quat/TRS)见 quat_convert.h, 本文件末尾自动包含 */
typedef struct { real_t x, y, z, w; } quat;

/* 向前声明 */
static FORCE_INLINE quat quat_normalize(quat q);
static FORCE_INLINE quat quat_mul(quat a, quat b);
static FORCE_INLINE quat quat_negate(quat q);
static FORCE_INLINE mat3 quat_to_mat3(quat q);

/* ==================== 1. 构造与常量 ==================== */

/* 构造四元数 */
static FORCE_INLINE quat quat_new(real_t x, real_t y, real_t z, real_t w) {
    quat q; q.x = x; q.y = y; q.z = z; q.w = w; return q;
}

/* 单位四元数 (代表没有任何旋转) */
static FORCE_INLINE quat quat_identity(void) {
    return quat_new(0.0f, 0.0f, 0.0f, 1.0f);
}

/* 零四元数 (不常用, 常用于线性插值初值) */
static FORCE_INLINE quat quat_zero(void) {
    return quat_new(0.0f, 0.0f, 0.0f, 0.0f);
}

/* 通过轴角构造 (轴必须归一化, 角度为弧度)
 * 注意: 轴未归一化时结果角速度不为 1; 需要内部归一化请用 quat_from_axis_angle_safe */
static FORCE_INLINE quat quat_from_axis_angle(vec3 axis, real_t angle) {
    real_t half = angle * 0.5f;
    real_t s = math_sin(half);
    return quat_new(axis.x * s, axis.y * s, axis.z * s, math_cos(half));
}

/* 通过轴角构造, 内部归一化轴; 零向量轴返回单位四元数 */
static FORCE_INLINE quat quat_from_axis_angle_safe(vec3 axis, real_t angle) {
    real_t len = vec3_length(axis);
    if (len < MATH_EPSILON) return quat_identity();
    return quat_from_axis_angle(vec3_scale(axis, 1.0f / len), angle);
}

/* ==================== 2. 转换 ==================== */

/* 欧拉角 (Pitch-X, Yaw-Y, Roll-Z) 转四元数
 * 顺序与 mat4_rotate_euler 一致: R = Rz(roll) * Ry(yaw) * Rx(pitch), 即先绕 X 再 Y 再 Z */
static FORCE_INLINE quat quat_from_euler(real_t pitch, real_t yaw, real_t roll) {
    real_t cp = math_cos(pitch * 0.5f), sp = math_sin(pitch * 0.5f);
    real_t cy = math_cos(yaw * 0.5f), sy = math_sin(yaw * 0.5f);
    real_t cr = math_cos(roll * 0.5f), sr = math_sin(roll * 0.5f);
    quat qx = quat_new(sp, 0.0f, 0.0f, cp);
    quat qy = quat_new(0.0f, sy, 0.0f, cy);
    quat qz = quat_new(0.0f, 0.0f, sr, cr);
    return quat_mul(qz, quat_mul(qy, qx));
}

/* 四元数转欧拉角 (Pitch-X, Yaw-Y, Roll-Z), 与 quat_from_euler 互逆
 * 注意: 万向锁(yaw ≈ ±90°)时 roll 置 0, pitch 由剩余分量解出
 *       判定阈值取 1e-3, 避免接近万向锁时 atan2(0,0) 产生跳变 */
static FORCE_INLINE vec3 quat_to_euler(quat q) {
    mat3 m = quat_to_mat3(quat_normalize(q));
    real_t sy = math_clamp(-m.m[2][0], -1.0f, 1.0f);
    real_t yaw = math_asin(sy);
    real_t cy = math_sqrt(math_max(0.0f, 1.0f - sy * sy));
    if (cy > MATH_GIMBAL_EPSILON) {
        return vec3_new(math_atan2(m.m[2][1], m.m[2][2]), yaw, math_atan2(m.m[1][0], m.m[0][0]));
    }
    return vec3_new(math_atan2(-m.m[1][2], m.m[1][1]), yaw, 0.0f);
}

/* 从 3x3 矩阵提取四元数 (要求矩阵仅包含旋转, 不能有缩放) */
static FORCE_INLINE quat quat_from_mat3(mat3 m) {
    real_t trace = m.m[0][0] + m.m[1][1] + m.m[2][2];
    quat q;
    if (trace > 0.0f) {
        real_t s = math_sqrt(trace + 1.0f) * 2.0f;
        q.w = 0.25f * s; q.x = (m.m[2][1] - m.m[1][2]) / s; q.y = (m.m[0][2] - m.m[2][0]) / s; q.z = (m.m[1][0] - m.m[0][1]) / s;
    }
    else if (m.m[0][0] > m.m[1][1] && m.m[0][0] > m.m[2][2]) {
        real_t s = math_sqrt(1.0f + m.m[0][0] - m.m[1][1] - m.m[2][2]) * 2.0f;
        q.w = (m.m[2][1] - m.m[1][2]) / s; q.x = 0.25f * s; q.y = (m.m[0][1] + m.m[1][0]) / s; q.z = (m.m[0][2] + m.m[2][0]) / s;
    }
    else if (m.m[1][1] > m.m[2][2]) {
        real_t s = math_sqrt(1.0f + m.m[1][1] - m.m[0][0] - m.m[2][2]) * 2.0f;
        q.w = (m.m[0][2] - m.m[2][0]) / s; q.x = (m.m[0][1] + m.m[1][0]) / s; q.y = 0.25f * s; q.z = (m.m[1][2] + m.m[2][1]) / s;
    }
    else {
        real_t s = math_sqrt(1.0f + m.m[2][2] - m.m[0][0] - m.m[1][1]) * 2.0f;
        q.w = (m.m[1][0] - m.m[0][1]) / s; q.x = (m.m[0][2] + m.m[2][0]) / s; q.y = (m.m[1][2] + m.m[2][1]) / s; q.z = 0.25f * s;
    }
    return quat_normalize(q);
}

/* 从 4x4 矩阵提取四元数 */
static FORCE_INLINE quat quat_from_mat4(mat4 m) {
    return quat_from_mat3(mat4_to_mat3(m));
}

/* 从两个方向向量计算旋转 (例如: 计算从 fwd 指向 target 的旋转)
 * 注意: 内部会归一化输入, 零向量输入返回单位四元数 */
static FORCE_INLINE quat quat_from_to_vectors(vec3 from, vec3 to) {
    vec3 a = vec3_normalize(from), b = vec3_normalize(to);
    if (vec3_length_sq(a) < MATH_EPSILON || vec3_length_sq(b) < MATH_EPSILON) return quat_identity();
    real_t d = math_clamp(vec3_dot(a, b), -1.0f, 1.0f);
    if (d >= 1.0f - MATH_EPSILON) return quat_identity();
    if (d <= -1.0f + MATH_EPSILON) {
        vec3 axis = vec3_any_perpendicular(a);
        return quat_from_axis_angle(axis, MATH_PI);
    }
    vec3 axis = vec3_cross(a, b);
    return quat_normalize(quat_new(axis.x, axis.y, axis.z, 1.0f + d));
}

/* 由朝向构造旋转: forward 与 up 均为世界方向, 返回把局部 +Z 对准 forward 的旋转
 * 注意: 零向量 forward 返回单位四元数; up 与 forward 平行时自动选取替代 up */
static FORCE_INLINE quat quat_from_look_rotation(vec3 forward, vec3 up) {
    vec3 f = vec3_normalize(forward);
    if (vec3_length_sq(f) < MATH_EPSILON) return quat_identity();
    vec3 r = vec3_cross(up, f);
    if (vec3_length_sq(r) < MATH_EPSILON) {
        vec3 alt = (math_abs(f.y) < MATH_UP_AXIS_SWITCH) ? vec3_new(0.0f, 1.0f, 0.0f) : vec3_new(1.0f, 0.0f, 0.0f);
        r = vec3_cross(alt, f);
    }
    r = vec3_normalize(r);
    vec3 u = vec3_cross(f, r);
    return quat_from_mat3(mat3_from_cols(r, u, f));
}

/* 四元数转 3x3 矩阵 */
static FORCE_INLINE mat3 quat_to_mat3(quat q) {
    mat3 m = mat3_identity();
    real_t xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
    real_t xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z;
    real_t wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;
    m.m[0][0] = 1.0f - 2.0f * (yy + zz); m.m[0][1] = 2.0f * (xy - wz); m.m[0][2] = 2.0f * (xz + wy);
    m.m[1][0] = 2.0f * (xy + wz); m.m[1][1] = 1.0f - 2.0f * (xx + zz); m.m[1][2] = 2.0f * (yz - wx);
    m.m[2][0] = 2.0f * (xz - wy); m.m[2][1] = 2.0f * (yz + wx); m.m[2][2] = 1.0f - 2.0f * (xx + yy);
    return m;
}

/* 四元数转 4x4 矩阵 */
static FORCE_INLINE mat4 quat_to_mat4(quat q) {
    return mat3_to_mat4(quat_to_mat3(q));
}

/* 拆出轴与角: 轴为单位向量, 角为 [0, PI]
 * 注意: 传入非单位四元数会被归一化; 轴或角指针可为 0 */
static FORCE_INLINE void quat_to_axis_angle(quat q, vec3* axis, real_t* angle) {
    q = quat_normalize(q);
    if (q.w < 0.0f) q = quat_negate(q);
    real_t s = math_sqrt(math_max(0.0f, 1.0f - q.w * q.w));
    real_t a = 2.0f * math_acos(math_clamp(q.w, -1.0f, 1.0f));
    if (axis) *axis = (s < MATH_EPSILON) ? vec3_new(1.0f, 0.0f, 0.0f) : vec3_new(q.x / s, q.y / s, q.z / s);
    if (angle) *angle = a;
}

/* ==================== 3. 基础运算 ==================== */

/* 四元数加法 (主要用于插值计算) */
static FORCE_INLINE quat quat_add(quat a, quat b) { return quat_new(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }

/* 四元数减法 */
static FORCE_INLINE quat quat_sub(quat a, quat b) { return quat_new(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }

/* 四元数标量乘法 */
static FORCE_INLINE quat quat_scale(quat q, real_t s) { return quat_new(q.x * s, q.y * s, q.z * s, q.w * s); }

/* 四元数取反(表示同一旋转的相反符号) */
static FORCE_INLINE quat quat_negate(quat q) { return quat_new(-q.x, -q.y, -q.z, -q.w); }

/* 四元数标量除法; s 为 0 时行为依赖浮点, 不报错 */
static FORCE_INLINE quat quat_div_scalar(quat q, real_t s) { return quat_new(q.x / s, q.y / s, q.z / s, q.w / s); }

/* 四元数乘法 (代表旋转的叠加: a * b 表示先转 b, 再转 a) */
static FORCE_INLINE quat quat_mul(quat a, quat b) {
    return quat_new(
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    );
}

/* 四元数点乘 (判断相似度) */
static FORCE_INLINE real_t quat_dot(quat a, quat b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

/* ==================== 4. 测量与共轭 ==================== */

/* 四元数长度平方 */
static FORCE_INLINE real_t quat_length_sq(quat q) { return quat_dot(q, q); }

/* 四元数长度 */
static FORCE_INLINE real_t quat_length(quat q) { return math_sqrt(quat_length_sq(q)); }

/* 归一化; 长度过小时返回单位四元数 */
static FORCE_INLINE quat quat_normalize(quat q) {
    real_t len = quat_length(q);
    return (len > MATH_EPSILON) ? quat_scale(q, 1.0f / len) : quat_identity();
}

/* 归一化失败时返回 fallback */
static FORCE_INLINE quat quat_normalize_or(quat q, quat fallback) {
    real_t len = quat_length(q);
    return (len > MATH_EPSILON) ? quat_scale(q, 1.0f / len) : fallback;
}

/* 共轭 (反向旋转) */
static FORCE_INLINE quat quat_conjugate(quat q) { return quat_new(-q.x, -q.y, -q.z, q.w); }

/* 旋转逆: 归一化后取共轭; 非单位四元数按旋转语义处理 */
static FORCE_INLINE quat quat_inverse(quat q) { return quat_conjugate(quat_normalize(q)); }

/* 安全比较 */
static FORCE_INLINE bool_t quat_is_equal(quat a, quat b) {
    return (math_is_equal(a.x, b.x) && math_is_equal(a.y, b.y) && math_is_equal(a.z, b.z) && math_is_equal(a.w, b.w)) ? 1 : 0;
}

/* 指定容差比较 */
static FORCE_INLINE bool_t quat_is_equal_eps(quat a, quat b, real_t eps) {
    return (math_abs(a.x - b.x) <= eps && math_abs(a.y - b.y) <= eps &&
        math_abs(a.z - b.z) <= eps && math_abs(a.w - b.w) <= eps) ? 1 : 0;
}

/* 是否所有分量有限 */
static FORCE_INLINE bool_t quat_is_finite(quat q) {
    return (math_is_finite(q.x) && math_is_finite(q.y) && math_is_finite(q.z) && math_is_finite(q.w)) ? 1 : 0;
}

/* 是否已单位化 */
static FORCE_INLINE bool_t quat_is_normalized(quat q) {
    return (math_abs(quat_length_sq(q) - 1.0f) < MATH_EPSILON) ? 1 : 0;
}

/* 是否为零四元数 */
static FORCE_INLINE bool_t quat_is_zero(quat q) { return (quat_length_sq(q) < MATH_EPSILON) ? 1 : 0; }

/* ==================== 5. 属性与旋转 ==================== */

/* 提取旋转角 (弧度), 范围 [0, 2π); q 与 -q 会得到互补角 */
static FORCE_INLINE real_t quat_angle(quat q) {
    return 2.0f * math_acos(math_clamp(quat_normalize(q).w, -1.0f, 1.0f));
}

/* 提取旋转轴 (单位向量); 接近单位四元数时返回 (1,0,0) */
static FORCE_INLINE vec3 quat_axis(quat q) {
    q = quat_normalize(q);
    real_t s = math_sqrt(math_max(0.0f, 1.0f - q.w * q.w));
    return (s < MATH_EPSILON) ? vec3_new(1.0f, 0.0f, 0.0f) : vec3_new(q.x / s, q.y / s, q.z / s);
}

/* 摆动-扭转分解: q = twist * swing, twist 绕 axis 旋转
 * axis 无需归一化; 输出指针可为 0; axis 为零时返回 0 */
static FORCE_INLINE bool_t quat_swing_twist(quat q, vec3 axis, quat* swing, quat* twist) {
    real_t len = vec3_length(axis);
    vec3 a;
    real_t d;
    quat tw, sw;
    if (len < MATH_EPSILON) return 0;
    a = vec3_scale(axis, 1.0f / len);
    q = quat_normalize(q);
    d = vec3_dot(vec3_new(q.x, q.y, q.z), a);
    tw = quat_normalize(quat_new(a.x * d, a.y * d, a.z * d, q.w));
    sw = quat_normalize(quat_mul(quat_conjugate(tw), q));
    if (swing) *swing = sw;
    if (twist) *twist = tw;
    return 1;
}

/* 两个旋转之间的最短夹角 [0, π] */
static FORCE_INLINE real_t quat_angle_between(quat a, quat b) {
    real_t d = math_abs(quat_dot(quat_normalize(a), quat_normalize(b)));
    return 2.0f * math_acos(math_clamp(d, 0.0f, 1.0f));
}

/* 用四元数旋转 3D 向量 (极速版) */
static FORCE_INLINE vec3 quat_mul_vec3(quat q, vec3 v) {
    vec3 qv = vec3_new(q.x, q.y, q.z);
    vec3 t = vec3_scale(vec3_cross(qv, v), 2.0f);
    return vec3_add(vec3_add(v, vec3_scale(t, q.w)), vec3_cross(qv, t));
}

/* 朝 target 旋转, 单次角步长不超过 max_angle 弧度; 已到达时返回 target */
static FORCE_INLINE quat quat_rotate_towards(quat current, quat target, real_t max_angle) {
    quat cur = quat_normalize(current), tg = quat_normalize(target);
    if (quat_dot(cur, tg) < 0.0f) tg = quat_scale(tg, -1.0f);
    quat d = quat_mul(tg, quat_conjugate(cur));
    real_t ang = quat_angle(d);
    if (ang <= max_angle || ang < MATH_EPSILON) return tg;
    return quat_normalize(quat_mul(quat_from_axis_angle(quat_axis(d), max_angle), cur));
}

/* ==================== 6. 插值 ==================== */

/* 普通线性插值 (不保证单位长度, 速度最快) */
static FORCE_INLINE quat quat_lerp(quat a, quat b, real_t t) {
    return quat_add(a, quat_scale(quat_sub(b, a), t));
}

/* 归一化线性插值 (比 slerp 快, 小角度下效果几乎一致) */
static FORCE_INLINE quat quat_nlerp(quat a, quat b, real_t t) {
    real_t dot = quat_dot(a, b);
    if (dot < 0.0f) b = quat_scale(b, -1.0f); /* 走最短路径 */
    return quat_normalize(quat_lerp(a, b, t));
}

/* 球面线性插值 (保证角速度均匀, 用于摄像机旋转等高质量动画)
 * 注意: dot 会被 clamp 到 [-1,1] 以避免浮点误差导致 acos 得到 NaN */
static FORCE_INLINE quat quat_slerp(quat a, quat b, real_t t) {
    real_t dot = math_clamp(quat_dot(quat_normalize(a), quat_normalize(b)), -1.0f, 1.0f);
    a = quat_normalize(a);
    b = quat_normalize(b);
    if (dot < 0.0f) { b = quat_scale(b, -1.0f); dot = -dot; }
    if (dot > MATH_SLERP_SNAP) return quat_nlerp(a, b, t); /* 夹角极小, 退化为 nlerp */

    real_t theta_0 = math_acos(dot);
    real_t theta = theta_0 * t;
    real_t sin_theta = math_sin(theta);
    real_t sin_theta_0 = math_sin(theta_0);
    real_t s0 = math_cos(theta) - dot * sin_theta / sin_theta_0;
    real_t s1 = sin_theta / sin_theta_0;
    return quat_normalize(quat_add(quat_scale(a, s0), quat_scale(b, s1)));
}

/* 四元数对数: 单位四元数 -> 纯虚四元数(轴 * 半角)
 * 注意: 取 w >= 0 的最短表示; 单位四元数返回零四元数 */
static FORCE_INLINE quat quat_log(quat q) {
    q = quat_normalize(q);
    if (q.w < 0.0f) q = quat_negate(q);
    real_t vlen = math_sqrt(q.x * q.x + q.y * q.y + q.z * q.z);
    if (vlen < MATH_EPSILON) return quat_zero();
    real_t s = math_atan2(vlen, q.w) / vlen;
    return quat_new(q.x * s, q.y * s, q.z * s, 0.0f);
}

/* 四元数指数: 纯虚四元数 -> 单位四元数 */
static FORCE_INLINE quat quat_exp(quat q) {
    real_t vlen = math_sqrt(q.x * q.x + q.y * q.y + q.z * q.z);
    if (vlen < MATH_EPSILON) return quat_identity();
    real_t s = math_sin(vlen) / vlen;
    return quat_new(q.x * s, q.y * s, q.z * s, math_cos(vlen));
}

/* SQUAD 控制点: 由前后关键帧计算当前帧的控制点
 * 注意: 三个输入都会被归一化, 相邻帧不能重合 */
static FORCE_INLINE quat quat_squad_control(quat prev, quat cur, quat next) {
    quat inv = quat_conjugate(quat_normalize(cur));
    quat l1 = quat_log(quat_mul(inv, quat_normalize(prev)));
    quat l2 = quat_log(quat_mul(inv, quat_normalize(next)));
    return quat_normalize(quat_mul(quat_normalize(cur), quat_exp(quat_scale(quat_add(l1, l2), -0.25f))));
}

/* SQUAD 插值: 带控制点的球面样条, 角速度连续
 * s0/s1 由 quat_squad_control 计算 */
static FORCE_INLINE quat quat_squad(quat q0, quat q1, quat s0, quat s1, real_t t) {
    quat a = quat_slerp(q0, q1, t);
    quat b = quat_slerp(s0, s1, t);
    return quat_slerp(a, b, 2.0f * t * (1.0f - t));
}

/* ==================== 7. 进阶数学 ==================== */

/* 四元数求幂 (用于平滑加速/减速动画)
 * 注意: 内部归一化并取 w >= 0 的最短表示 */
static FORCE_INLINE quat quat_pow(quat q, real_t t) {
    q = quat_normalize(q);
    if (q.w < 0.0f) q = quat_negate(q);
    if (q.w > MATH_IDENTITY_SNAP) return quat_identity(); /* 接近单位四元数 */
    real_t angle = math_acos(math_clamp(q.w, -1.0f, 1.0f));
    real_t axis_norm = math_sqrt(math_max(0.0f, 1.0f - q.w * q.w));
    real_t new_angle = angle * t;
    real_t s = math_sin(new_angle) / axis_norm;
    return quat_new(q.x * s, q.y * s, q.z * s, math_cos(new_angle));
}


EXTERN_C_END
#include "core/math/quat/quat_convert.h"
#endif