#ifndef MATH_MATRIX_MAT3_H
#define MATH_MATRIX_MAT3_H
#include "core/platform.h"
#include "core/typedef.h"
#include "core/math/scalar.h"
#include "core/math/constants.h"
#include "core/math/vector/vec3.h"
#include "core/math/matrix/mat2.h"
EXTERN_C_BEGIN


typedef struct { real_t m[MAT3_DIM][MAT3_DIM]; } mat3;

/* ---- 构造 ---- */
/* 零矩阵 */
static FORCE_INLINE mat3 mat3_zero(void) { mat3 r = { 0 }; return r; }
/* 单位阵 */
static FORCE_INLINE mat3 mat3_identity(void)
{
    mat3 r = mat3_zero(); r.m[0][0] = 1.0f; r.m[1][1] = 1.0f; r.m[2][2] = 1.0f; return r;
}
/* 由 9 个标量按行构造 */
static FORCE_INLINE mat3 mat3_new(
    real_t m00, real_t m01, real_t m02,
    real_t m10, real_t m11, real_t m12,
    real_t m20, real_t m21, real_t m22)
{
    mat3 r;
    r.m[0][0] = m00; r.m[0][1] = m01; r.m[0][2] = m02;
    r.m[1][0] = m10; r.m[1][1] = m11; r.m[1][2] = m12;
    r.m[2][0] = m20; r.m[2][1] = m21; r.m[2][2] = m22;
    return r;
}
/* 由列向量构造; 内存仍为行主序 m[row][col], 列 j 写入 m[i][j] */
static FORCE_INLINE mat3 mat3_from_cols(vec3 c0, vec3 c1, vec3 c2) {
    mat3 r;
    r.m[0][0] = c0.x; r.m[0][1] = c1.x; r.m[0][2] = c2.x;
    r.m[1][0] = c0.y; r.m[1][1] = c1.y; r.m[1][2] = c2.y;
    r.m[2][0] = c0.z; r.m[2][1] = c1.z; r.m[2][2] = c2.z;
    return r;
}
/* 对角阵 diag(d.x, d.y, d.z); 与 mat3_scale(d) 等价 */
static FORCE_INLINE mat3 mat3_diag(vec3 d) { mat3 r = mat3_zero(); r.m[0][0] = d.x; r.m[1][1] = d.y; r.m[2][2] = d.z; return r; }

/* ---- 访问器 ---- */
/* 取第 j 列 (0..2) */
static FORCE_INLINE vec3 mat3_col(mat3 m, qty_t j) { return vec3_new(m.m[0][j], m.m[1][j], m.m[2][j]); }
/* 取第 i 行 (0..2) */
static FORCE_INLINE vec3 mat3_row(mat3 m, qty_t i) { return vec3_new(m.m[i][0], m.m[i][1], m.m[i][2]); }
/* 写第 j 列, 返回修改后的副本 */
static FORCE_INLINE mat3 mat3_set_col(mat3 m, qty_t j, vec3 v) { m.m[0][j] = v.x; m.m[1][j] = v.y; m.m[2][j] = v.z; return m; }
/* 写第 i 行, 返回修改后的副本 */
static FORCE_INLINE mat3 mat3_set_row(mat3 m, qty_t i, vec3 v) { m.m[i][0] = v.x; m.m[i][1] = v.y; m.m[i][2] = v.z; return m; }
/* 拆成列向量组, 与 mat3_from_cols 对偶 */
static FORCE_INLINE void mat3_to_cols(mat3 m, vec3* c0, vec3* c1, vec3* c2) {
    *c0 = mat3_col(m, 0); *c1 = mat3_col(m, 1); *c2 = mat3_col(m, 2);
}
/* 提取对角线, 与 mat3_diag 对偶 */
static FORCE_INLINE vec3 mat3_extract_diag(mat3 m) { return vec3_new(m.m[0][0], m.m[1][1], m.m[2][2]); }
/* 仅改对角线, 其余元素不变 */
static FORCE_INLINE mat3 mat3_set_diag(mat3 m, vec3 d) { m.m[0][0] = d.x; m.m[1][1] = d.y; m.m[2][2] = d.z; return m; }

/* ---- 基础运算 ---- */
/* 加法 */
static FORCE_INLINE mat3 mat3_add(mat3 a, mat3 b)
{
    mat3 r;qty_t i, j;  for (i = 0;i < MAT3_DIM;i++)for (j = 0;j < MAT3_DIM;j++)r.m[i][j] = a.m[i][j] + b.m[i][j]; return r;
}
/* 减法 */
static FORCE_INLINE mat3 mat3_sub(mat3 a, mat3 b)
{
    mat3 r; qty_t i, j; for (i = 0;i < MAT3_DIM;i++)for (j = 0;j < MAT3_DIM;j++)r.m[i][j] = a.m[i][j] - b.m[i][j]; return r;
}
/* 逐元素取反 */
static FORCE_INLINE mat3 mat3_negate(mat3 m)
{
    mat3 r; qty_t i, j; for (i = 0;i < MAT3_DIM;i++)for (j = 0;j < MAT3_DIM;j++)r.m[i][j] = -m.m[i][j]; return r;
}
/* 标量乘 */
static FORCE_INLINE mat3 mat3_mul_scalar(mat3 m, real_t s)
{
    mat3 r; qty_t i, j; for (i = 0;i < MAT3_DIM;i++)for (j = 0;j < MAT3_DIM;j++)r.m[i][j] = m.m[i][j] * s; return r;
}
/* 标量除; s 为 0 时行为依赖浮点, 不报错 */
static FORCE_INLINE mat3 mat3_div_scalar(mat3 m, real_t s)
{
    mat3 r; qty_t i, j; for (i = 0;i < MAT3_DIM;i++)for (j = 0;j < MAT3_DIM;j++)r.m[i][j] = m.m[i][j] / s; return r;
}
/* Hadamard 积(逐元素乘), 区别于 mat3_mul(矩阵乘) */
static FORCE_INLINE mat3 mat3_cwise_mul(mat3 a, mat3 b)
{
    mat3 r; qty_t i, j; for (i = 0;i < MAT3_DIM;i++)for (j = 0;j < MAT3_DIM;j++)r.m[i][j] = a.m[i][j] * b.m[i][j]; return r;
}
/* 逐元素除; b 含 0 时行为依赖浮点, 不报错 */
static FORCE_INLINE mat3 mat3_cwise_div(mat3 a, mat3 b)
{
    mat3 r; qty_t i, j; for (i = 0;i < MAT3_DIM;i++)for (j = 0;j < MAT3_DIM;j++)r.m[i][j] = a.m[i][j] / b.m[i][j]; return r;
}
/* 线性插值 a*(1-t) + b*t */
static FORCE_INLINE mat3 mat3_lerp(mat3 a, mat3 b, real_t t)
{
    mat3 r; qty_t i, j; for (i = 0;i < MAT3_DIM;i++)for (j = 0;j < MAT3_DIM;j++)r.m[i][j] = a.m[i][j] + (b.m[i][j] - a.m[i][j]) * t; return r;
}

/* ---- 矩阵运算 ---- */
/* 乘法 C = A * B
 * 列向量约定: mat3_mul(A, B) * v == A * (B * v), 即先应用 B 再应用 A */
static FORCE_INLINE mat3 mat3_mul(mat3 a, mat3 b) {
    mat3 r = mat3_zero();qty_t i, j, k;
    for (i = 0;i < MAT3_DIM;i++) for (j = 0;j < MAT3_DIM;j++) for (k = 0;k < MAT3_DIM;k++) r.m[i][j] += a.m[i][k] * b.m[k][j];
    return r;
}
/* 乘3D向量 */
static FORCE_INLINE vec3 mat3_mul_vec3(mat3 m, vec3 v) {
    return vec3_new(
        m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z,
        m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z,
        m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z
    );
}
/* 转置 */
static FORCE_INLINE mat3 mat3_transpose(mat3 m)
{
    mat3 r; qty_t i, j; for (i = 0;i < MAT3_DIM;i++)for (j = 0;j < MAT3_DIM;j++)r.m[i][j] = m.m[j][i]; return r;
}

/* ---- 求逆 ---- */
/* 行列式 */
static FORCE_INLINE real_t mat3_determinant(mat3 m) {
    return m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[2][1] * m.m[1][2])
        - m.m[0][1] * (m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0])
        + m.m[0][2] * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]);
}
/* 伴随矩阵; inverse = adjugate / determinant */
static FORCE_INLINE mat3 mat3_adjugate(mat3 m) {
    mat3 r;
    r.m[0][0] = (m.m[1][1] * m.m[2][2] - m.m[2][1] * m.m[1][2]);
    r.m[0][1] = -(m.m[0][1] * m.m[2][2] - m.m[0][2] * m.m[2][1]);
    r.m[0][2] = (m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1]);
    r.m[1][0] = -(m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]);
    r.m[1][1] = (m.m[0][0] * m.m[2][2] - m.m[0][2] * m.m[2][0]);
    r.m[1][2] = -(m.m[0][0] * m.m[1][2] - m.m[1][0] * m.m[0][2]);
    r.m[2][0] = (m.m[1][0] * m.m[2][1] - m.m[2][0] * m.m[1][1]);
    r.m[2][1] = -(m.m[0][0] * m.m[2][1] - m.m[2][0] * m.m[0][1]);
    r.m[2][2] = (m.m[0][0] * m.m[1][1] - m.m[1][0] * m.m[0][1]);
    return r;
}
/* 求逆, 退化时返回 fallback
 * 注: 以绝对阈值 MATH_EPSILON 判断退化; 极小缩放(如 scale(1e-3))的可逆矩阵
 *     可能因 det 过小而误判为退化若场景涉及大幅缩放, 可改为按矩阵最大元素
 *     相对缩放后再比较, 或由调用方提供容差 */
static FORCE_INLINE mat3 mat3_inverse_or(mat3 m, mat3 fallback) {
    real_t det = mat3_determinant(m);
    if (!(det == det) || det - det != 0) return fallback;   /* NaN / ±Inf */
    if (math_abs(det) < MATH_EPSILON) return fallback;
    return mat3_mul_scalar(mat3_adjugate(m), 1.0f / det);
}
/* 求逆, 退化时返回单位阵 */
static FORCE_INLINE mat3 mat3_inverse(mat3 m) { return mat3_inverse_or(m, mat3_identity()); }
/* 逆转置(3x3 法线矩阵) */
static FORCE_INLINE mat3 mat3_inverse_transpose(mat3 m) { return mat3_transpose(mat3_inverse(m)); }

/* ---- 比较 / 判定 ---- */
/* 安全比较; bool_t 语义同 bool: 1 相等 / 0 不等 */
static FORCE_INLINE bool_t mat3_is_equal(mat3 a, mat3 b) {
    qty_t i, j;
    for (i = 0; i < MAT3_DIM; i++)
        for (j = 0; j < MAT3_DIM; j++)
            if (!math_is_equal(a.m[i][j], b.m[i][j])) return 0;
    return 1;
}
/* 指定容差比较; 差 <= eps 视为相等 */
static FORCE_INLINE bool_t mat3_is_equal_eps(mat3 a, mat3 b, real_t eps) {
    qty_t i, j;
    for (i = 0; i < MAT3_DIM; i++)
        for (j = 0; j < MAT3_DIM; j++)
            if (!(math_abs(a.m[i][j] - b.m[i][j]) <= eps)) return 0;   /* NaN 判不等 */
    return 1;
}
/* 全部元素有限 (排除 NaN / ±Inf); 依赖算术语义, -ffast-math 下结果不可靠 */
static FORCE_INLINE bool_t mat3_is_finite(mat3 m) {
    qty_t i, j;
    for (i = 0; i < MAT3_DIM; i++) for (j = 0; j < MAT3_DIM; j++) {
        real_t v = m.m[i][j];
        if (!(v == v) || v - v != 0) return 0;
    }
    return 1;
}
/* 是否单位阵(默认容差) */
static FORCE_INLINE bool_t mat3_is_identity(mat3 m) {
    qty_t i, j;
    for (i = 0; i < MAT3_DIM; i++) for (j = 0; j < MAT3_DIM; j++)
        if (!math_is_equal(m.m[i][j], (i == j) ? 1.0f : 0.0f)) return 0;
    return 1;
}
/* 列向量是否两两正交且单位长(默认容差) */
static FORCE_INLINE bool_t mat3_is_orthogonal(mat3 m) {
    vec3 c0 = mat3_col(m, 0), c1 = mat3_col(m, 1), c2 = mat3_col(m, 2);
    return math_abs(vec3_dot(c0, c0) - 1.0f) < MATH_EPSILON &&
        math_abs(vec3_dot(c1, c1) - 1.0f) < MATH_EPSILON &&
        math_abs(vec3_dot(c2, c2) - 1.0f) < MATH_EPSILON &&
        math_abs(vec3_dot(c0, c1)) < MATH_EPSILON &&
        math_abs(vec3_dot(c0, c2)) < MATH_EPSILON &&
        math_abs(vec3_dot(c1, c2)) < MATH_EPSILON;
}
/* 是否对称矩阵 */
static FORCE_INLINE bool_t mat3_is_symmetric(mat3 m) {
    return (math_is_equal(m.m[0][1], m.m[1][0]) && math_is_equal(m.m[0][2], m.m[2][0]) &&
        math_is_equal(m.m[1][2], m.m[2][1])) ? 1 : 0;
}
/* 是否为纯旋转(正交且行列式为 1, 排除镜像) */
static FORCE_INLINE bool_t mat3_is_rotation(mat3 m) {
    return (mat3_is_orthogonal(m) && mat3_determinant(m) > 0.0f) ? 1 : 0;
}
/* 施密特正交化: 使列向量正交单位化(修复累积误差)
 * 注意: 退化列(长度为 0)取标准基补齐, 不报错 */
static FORCE_INLINE mat3 mat3_orthonormalize(mat3 m) {
    vec3 c0 = mat3_col(m, 0), c1 = mat3_col(m, 1), c2 = mat3_col(m, 2);
    c0 = vec3_normalize(c0);
    if (vec3_length_sq(c0) < MATH_EPSILON) c0 = vec3_new(1.0f, 0.0f, 0.0f);
    c1 = vec3_sub(c1, vec3_scale(c0, vec3_dot(c1, c0)));
    c1 = vec3_normalize(c1);
    if (vec3_length_sq(c1) < MATH_EPSILON) c1 = vec3_any_perpendicular(c0);
    c2 = vec3_cross(c0, c1);
    return mat3_from_cols(c0, c1, c2);
}
/* 矩阵迹 */
static FORCE_INLINE real_t mat3_trace(mat3 m) { return m.m[0][0] + m.m[1][1] + m.m[2][2]; }
/* Frobenius 内积(矩阵点积) sum(a_ij * b_ij); a == b 时等于 length_sq */
static FORCE_INLINE real_t mat3_dot(mat3 a, mat3 b) {
    qty_t i, j; real_t s = 0.0f;
    for (i = 0; i < MAT3_DIM; i++) for (j = 0; j < MAT3_DIM; j++) s += a.m[i][j] * b.m[i][j];
    return s;
}
/* Frobenius 范数平方 sum(a_ij^2); 用于仅比较大小时避免开方 */
static FORCE_INLINE real_t mat3_length_sq(mat3 m) {
    qty_t i, j; real_t s = 0.0f;
    for (i = 0; i < MAT3_DIM; i++) for (j = 0; j < MAT3_DIM; j++) s += m.m[i][j] * m.m[i][j];
    return s;
}
/* Frobenius 范数 sqrt(sum(a_ij^2)) */
static FORCE_INLINE real_t mat3_length(mat3 m) { return math_sqrt(mat3_length_sq(m)); }

/* ---- 变换 ---- */
/* 绕X轴, 右手逆时针(数学约定); LH 世界下视觉为顺时针 */
static FORCE_INLINE mat3 mat3_rotate_x(real_t a) {
    mat3 r = mat3_identity(); real_t c = math_cos(a), s = math_sin(a);
    r.m[1][1] = c;r.m[1][2] = -s;r.m[2][1] = s;r.m[2][2] = c;
    return r;
}
/* 绕Y轴, 右手逆时针(数学约定); LH 世界下视觉为顺时针 */
static FORCE_INLINE mat3 mat3_rotate_y(real_t a) {
    mat3 r = mat3_identity(); real_t c = math_cos(a), s = math_sin(a);
    r.m[0][0] = c;r.m[0][2] = s;r.m[2][0] = -s;r.m[2][2] = c;
    return r;
}
/* 绕Z轴, 右手逆时针(数学约定); LH 世界下视觉为顺时针 */
static FORCE_INLINE mat3 mat3_rotate_z(real_t a) {
    mat3 r = mat3_identity();real_t c = math_cos(a), s = math_sin(a);
    r.m[0][0] = c;r.m[0][1] = -s;r.m[1][0] = s;r.m[1][1] = c;
    return r;
}
/* 绕任意轴旋转(Rodrigues 公式), axis 必须已归一化; 右手逆时针(数学约定) */
static FORCE_INLINE mat3 mat3_rotate_axis(vec3 axis, real_t angle) {
    mat3 r = mat3_identity();
    real_t c = math_cos(angle), s = math_sin(angle), t = 1.0f - c;
    real_t x = axis.x, y = axis.y, z = axis.z;
    r.m[0][0] = t * x * x + c;     r.m[0][1] = t * x * y - s * z; r.m[0][2] = t * x * z + s * y;
    r.m[1][0] = t * x * y + s * z; r.m[1][1] = t * y * y + c;     r.m[1][2] = t * y * z - s * x;
    r.m[2][0] = t * x * z - s * y; r.m[2][1] = t * y * z + s * x; r.m[2][2] = t * z * z + c;
    return r;
}
/* 同上, 内部归一化 axis; 零向量时依赖 vec3_normalize 行为, 不报错 */
static FORCE_INLINE mat3 mat3_rotate_axis_safe(vec3 axis, real_t angle) {
    return mat3_rotate_axis(vec3_normalize(axis), angle);
}
/* 非均匀缩放矩阵(构造非均匀缩放) */
static FORCE_INLINE mat3 mat3_scale(vec3 s) {
    mat3 r = mat3_identity();
    r.m[0][0] = s.x; r.m[1][1] = s.y; r.m[2][2] = s.z;
    return r;
}
/* 欧拉角旋转矩阵, 与 mat4_rotate_euler 同序: R = Rz(roll) * Ry(yaw) * Rx(pitch) */
static FORCE_INLINE mat3 mat3_rotate_euler(vec3 euler) {
    return mat3_mul(mat3_rotate_z(euler.z), mat3_mul(mat3_rotate_y(euler.y), mat3_rotate_x(euler.x)));
}
/* 提取欧拉角(Pitch-X, Yaw-Y, Roll-Z), 与 mat3_rotate_euler 互逆
 * 注意: 万向锁(yaw ≈ ±90°)时 roll 置 0, pitch 由剩余分量解出 */
static FORCE_INLINE vec3 mat3_to_euler(mat3 m) {
    real_t sy = math_clamp(-m.m[2][0], -1.0f, 1.0f);
    real_t yaw = math_asin(sy);
    real_t cy = math_sqrt(math_max(0.0f, 1.0f - sy * sy));
    if (cy > MATH_GIMBAL_EPSILON) {
        return vec3_new(math_atan2(m.m[2][1], m.m[2][2]), yaw, math_atan2(m.m[1][0], m.m[0][0]));
    }
    return vec3_new(math_atan2(-m.m[1][2], m.m[1][1]), yaw, 0.0f);
}

/* ---- 转换 ---- */
/* 2x2 -> 3x3: 左上角填入, 其余为单位阵; 不平移 */
static FORCE_INLINE mat3 mat2_to_mat3(mat2 m) {
    mat3 r = mat3_identity();
    qty_t i, j;
    for (i = 0; i < MAT2_DIM; i++) for (j = 0; j < MAT2_DIM; j++) r.m[i][j] = m.m[i][j];
    return r;
}
/* 3x3 -> 2x2: 裁剪左上角; 仅取线性部分 */
static FORCE_INLINE mat2 mat3_to_mat2(mat3 m) {
    mat2 r; qty_t i, j;
    for (i = 0; i < MAT2_DIM; i++) for (j = 0; j < MAT2_DIM; j++) r.m[i][j] = m.m[i][j];
    return r;
}

/* ---- 数组互操作 ---- */
/* 行主序写入 out[MAT3_ELEM_COUNT] */
static FORCE_INLINE void mat3_to_array(mat3 m, real_t out[MAT3_ELEM_COUNT]) {
    qty_t i, j; for (i = 0; i < MAT3_DIM; i++) for (j = 0; j < MAT3_DIM; j++) out[i * MAT3_DIM + j] = m.m[i][j];
}
/* 行主序从 in[MAT3_ELEM_COUNT] 构造 */
static FORCE_INLINE mat3 mat3_from_array(const real_t in[MAT3_ELEM_COUNT]) {
    mat3 r; qty_t i, j; for (i = 0; i < MAT3_DIM; i++) for (j = 0; j < MAT3_DIM; j++) r.m[i][j] = in[i * MAT3_DIM + j];
    return r;
}


EXTERN_C_END
#endif