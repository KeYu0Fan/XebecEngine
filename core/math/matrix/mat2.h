#ifndef MATH_MATRIX_MAT2_H
#define MATH_MATRIX_MAT2_H
#include "core/platform.h"
#include "core/typedef.h"
#include "core/math/scalar.h"
#include "core/math/constants.h"
#include "core/math/vector/vec2.h"
EXTERN_C_BEGIN


typedef struct { real_t m[MAT2_DIM][MAT2_DIM]; } mat2;

/* ---- 构造 ---- */
/* 零矩阵 */
static FORCE_INLINE mat2 mat2_zero(void) { mat2 r = { 0 }; return r; }
/* 单位矩阵 */
static FORCE_INLINE mat2 mat2_identity(void) { mat2 r = mat2_zero(); r.m[0][0] = 1.0f; r.m[1][1] = 1.0f; return r; }
/* 由 4 个标量按行构造: m[0][0], m[0][1], m[1][0], m[1][1] */
static FORCE_INLINE mat2 mat2_new(real_t m00, real_t m01, real_t m10, real_t m11)
{
    mat2 r; r.m[0][0] = m00; r.m[0][1] = m01; r.m[1][0] = m10; r.m[1][1] = m11; return r;
}
/* 由列向量构造; 内存仍为行主序 m[row][col], 列 j 写入 m[i][j] */
static FORCE_INLINE mat2 mat2_from_cols(vec2 c0, vec2 c1) {
    mat2 r;
    r.m[0][0] = c0.x; r.m[0][1] = c1.x;
    r.m[1][0] = c0.y; r.m[1][1] = c1.y;
    return r;
}
/* 对角阵 diag(d.x, d.y); 与 mat2_scale(d) 等价 */
static FORCE_INLINE mat2 mat2_diag(vec2 d) { mat2 r = mat2_zero(); r.m[0][0] = d.x; r.m[1][1] = d.y; return r; }

/* ---- 访问器 ---- */
/* 取第 j 列 (0..1) */
static FORCE_INLINE vec2 mat2_col(mat2 m, qty_t j) { return vec2_new(m.m[0][j], m.m[1][j]); }
/* 取第 i 行 (0..1) */
static FORCE_INLINE vec2 mat2_row(mat2 m, qty_t i) { return vec2_new(m.m[i][0], m.m[i][1]); }
/* 写第 j 列, 返回修改后的副本 */
static FORCE_INLINE mat2 mat2_set_col(mat2 m, qty_t j, vec2 v) { m.m[0][j] = v.x; m.m[1][j] = v.y; return m; }
/* 写第 i 行, 返回修改后的副本 */
static FORCE_INLINE mat2 mat2_set_row(mat2 m, qty_t i, vec2 v) { m.m[i][0] = v.x; m.m[i][1] = v.y; return m; }
/* 拆成列向量组, 与 mat2_from_cols 对偶 */
static FORCE_INLINE void mat2_to_cols(mat2 m, vec2* c0, vec2* c1) {
    *c0 = mat2_col(m, 0); *c1 = mat2_col(m, 1);
}
/* 提取对角线, 与 mat2_diag 对偶 */
static FORCE_INLINE vec2 mat2_extract_diag(mat2 m) { return vec2_new(m.m[0][0], m.m[1][1]); }
/* 仅改对角线, 其余元素不变 */
static FORCE_INLINE mat2 mat2_set_diag(mat2 m, vec2 d) { m.m[0][0] = d.x; m.m[1][1] = d.y; return m; }

/* ---- 基础运算 ---- */
/* 矩阵加法 */
static FORCE_INLINE mat2 mat2_add(mat2 a, mat2 b)
{
    mat2 r; qty_t i, j; for (i = 0;i < MAT2_DIM;i++)for (j = 0;j < MAT2_DIM;j++)r.m[i][j] = a.m[i][j] + b.m[i][j]; return r;
}
/* 矩阵减法 */
static FORCE_INLINE mat2 mat2_sub(mat2 a, mat2 b)
{
    mat2 r; qty_t i, j; for (i = 0;i < MAT2_DIM;i++)for (j = 0;j < MAT2_DIM;j++)r.m[i][j] = a.m[i][j] - b.m[i][j]; return r;
}
/* 逐元素取反 */
static FORCE_INLINE mat2 mat2_negate(mat2 m)
{
    mat2 r; qty_t i, j; for (i = 0;i < MAT2_DIM;i++)for (j = 0;j < MAT2_DIM;j++)r.m[i][j] = -m.m[i][j]; return r;
}
/* 标量乘法 */
static FORCE_INLINE mat2 mat2_mul_scalar(mat2 m, real_t s)
{
    mat2 r; qty_t i, j; for (i = 0;i < MAT2_DIM;i++)for (j = 0;j < MAT2_DIM;j++)r.m[i][j] = m.m[i][j] * s; return r;
}
/* 标量除法; s 为 0 时行为依赖浮点, 不报错 */
static FORCE_INLINE mat2 mat2_div_scalar(mat2 m, real_t s)
{
    mat2 r; qty_t i, j; for (i = 0;i < MAT2_DIM;i++)for (j = 0;j < MAT2_DIM;j++)r.m[i][j] = m.m[i][j] / s; return r;
}
/* Hadamard 积(逐元素乘), 区别于 mat2_mul(矩阵乘) */
static FORCE_INLINE mat2 mat2_cwise_mul(mat2 a, mat2 b)
{
    mat2 r; qty_t i, j; for (i = 0;i < MAT2_DIM;i++)for (j = 0;j < MAT2_DIM;j++)r.m[i][j] = a.m[i][j] * b.m[i][j]; return r;
}
/* 逐元素除; b 含 0 时行为依赖浮点, 不报错 */
static FORCE_INLINE mat2 mat2_cwise_div(mat2 a, mat2 b)
{
    mat2 r; qty_t i, j; for (i = 0;i < MAT2_DIM;i++)for (j = 0;j < MAT2_DIM;j++)r.m[i][j] = a.m[i][j] / b.m[i][j]; return r;
}
/* 线性插值 a*(1-t) + b*t */
static FORCE_INLINE mat2 mat2_lerp(mat2 a, mat2 b, real_t t)
{
    mat2 r; qty_t i, j; for (i = 0;i < MAT2_DIM;i++)for (j = 0;j < MAT2_DIM;j++)r.m[i][j] = a.m[i][j] + (b.m[i][j] - a.m[i][j]) * t; return r;
}

/* ---- 矩阵运算 ---- */
/* 矩阵乘法 C = A * B
 * 列向量约定: mat2_mul(A, B) * v == A * (B * v), 即先应用 B 再应用 A */
static FORCE_INLINE mat2 mat2_mul(mat2 a, mat2 b) {
    mat2 r;
    r.m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0];
    r.m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1];
    r.m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0];
    r.m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1];
    return r;
}
/* 乘向量 */
static FORCE_INLINE vec2 mat2_mul_vec2(mat2 m, vec2 v)
{
    return vec2_new(m.m[0][0] * v.x + m.m[0][1] * v.y, m.m[1][0] * v.x + m.m[1][1] * v.y);
}
/* 转置 */
static FORCE_INLINE mat2 mat2_transpose(mat2 m) { mat2 r = m; r.m[0][1] = m.m[1][0]; r.m[1][0] = m.m[0][1]; return r; }

/* ---- 求逆 ---- */
/* 行列式 */
static FORCE_INLINE real_t mat2_determinant(mat2 m) { return m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0]; }
/* 伴随矩阵; inverse = adjugate / determinant */
static FORCE_INLINE mat2 mat2_adjugate(mat2 m)
{
    return mat2_new(m.m[1][1], -m.m[0][1], -m.m[1][0], m.m[0][0]);
}
/* 求逆, 退化时返回 fallback
 * 注: 以绝对阈值 MATH_EPSILON 判断退化; 极小缩放(如 scale(1e-3))的可逆矩阵
 *     可能因 det 过小而误判为退化若场景涉及大幅缩放, 可改为按矩阵最大元素
 *     相对缩放后再比较, 或由调用方提供容差 */
static FORCE_INLINE mat2 mat2_inverse_or(mat2 m, mat2 fallback) {
    real_t det = mat2_determinant(m);
    if (!(det == det) || det - det != 0) return fallback;   /* NaN / ±Inf */
    if (math_abs(det) < MATH_EPSILON) return fallback;
    return mat2_mul_scalar(mat2_adjugate(m), 1.0f / det);
}
/* 求逆, 退化时返回单位阵 */
static FORCE_INLINE mat2 mat2_inverse(mat2 m) { return mat2_inverse_or(m, mat2_identity()); }
/* 逆转置(2D 法线矩阵) */
static FORCE_INLINE mat2 mat2_inverse_transpose(mat2 m) { return mat2_transpose(mat2_inverse(m)); }

/* ---- 比较 ---- */
/* 安全比较; bool_t 语义同 bool: 1 相等 / 0 不等 */
static FORCE_INLINE bool_t mat2_is_equal(mat2 a, mat2 b) {
    qty_t i, j;
    for (i = 0; i < MAT2_DIM; i++)
        for (j = 0; j < MAT2_DIM; j++)
            if (!math_is_equal(a.m[i][j], b.m[i][j])) return 0;
    return 1;
}
/* 指定容差比较; 差 <= eps 视为相等 */
static FORCE_INLINE bool_t mat2_is_equal_eps(mat2 a, mat2 b, real_t eps) {
    qty_t i, j;
    for (i = 0; i < MAT2_DIM; i++)
        for (j = 0; j < MAT2_DIM; j++)
            if (!(math_abs(a.m[i][j] - b.m[i][j]) <= eps)) return 0;   /* NaN 判不等 */
    return 1;
}
/* 全部元素有限 (排除 NaN / ±Inf); 依赖算术语义, -ffast-math 下结果不可靠 */
static FORCE_INLINE bool_t mat2_is_finite(mat2 m) {
    qty_t i, j;
    for (i = 0; i < MAT2_DIM; i++) for (j = 0; j < MAT2_DIM; j++) {
        real_t v = m.m[i][j];
        if (!(v == v) || v - v != 0) return 0;
    }
    return 1;
}
/* 是否单位阵(默认容差) */
static FORCE_INLINE bool_t mat2_is_identity(mat2 m) {
    qty_t i, j;
    for (i = 0; i < MAT2_DIM; i++) for (j = 0; j < MAT2_DIM; j++)
        if (!math_is_equal(m.m[i][j], (i == j) ? 1.0f : 0.0f)) return 0;
    return 1;
}
/* 列向量是否两两正交且单位长(默认容差) */
static FORCE_INLINE bool_t mat2_is_orthogonal(mat2 m) {
    vec2 c0 = mat2_col(m, 0), c1 = mat2_col(m, 1);
    return math_abs(vec2_dot(c0, c0) - 1.0f) < MATH_EPSILON &&
        math_abs(vec2_dot(c1, c1) - 1.0f) < MATH_EPSILON &&
        math_abs(vec2_dot(c0, c1)) < MATH_EPSILON;
}
/* 是否对称矩阵 */
static FORCE_INLINE bool_t mat2_is_symmetric(mat2 m) {
    return math_is_equal(m.m[0][1], m.m[1][0]) ? 1 : 0;
}
/* 是否为纯旋转(正交且行列式为 1, 排除镜像) */
static FORCE_INLINE bool_t mat2_is_rotation(mat2 m) {
    return (mat2_is_orthogonal(m) && mat2_determinant(m) > 0.0f) ? 1 : 0;
}
/* 施密特正交化: 使列向量正交单位化(修复累积误差)
 * 注意: 退化列(长度为 0)取标准基补齐, 不报错 */
static FORCE_INLINE mat2 mat2_orthonormalize(mat2 m) {
    vec2 c0 = mat2_col(m, 0), c1 = mat2_col(m, 1);
    c0 = vec2_normalize(c0);
    if (vec2_length_sq(c0) < MATH_EPSILON) c0 = vec2_new(1.0f, 0.0f);
    c1 = vec2_sub(c1, vec2_scale(c0, vec2_dot(c1, c0)));
    c1 = vec2_normalize(c1);
    if (vec2_length_sq(c1) < MATH_EPSILON) c1 = vec2_new(-c0.y, c0.x);
    return mat2_from_cols(c0, c1);
}
/* 提取 2D 旋转角(弧度) */
static FORCE_INLINE real_t mat2_to_angle(mat2 m) { return math_atan2(m.m[1][0], m.m[0][0]); }
/* 矩阵迹 */
static FORCE_INLINE real_t mat2_trace(mat2 m) { return m.m[0][0] + m.m[1][1]; }
/* Frobenius 内积(矩阵点积) sum(a_ij * b_ij); a == b 时等于 length_sq */
static FORCE_INLINE real_t mat2_dot(mat2 a, mat2 b) {
    qty_t i, j; real_t s = 0.0f;
    for (i = 0; i < MAT2_DIM; i++) for (j = 0; j < MAT2_DIM; j++) s += a.m[i][j] * b.m[i][j];
    return s;
}
/* Frobenius 范数平方 sum(a_ij^2); 用于仅比较大小时避免开方 */
static FORCE_INLINE real_t mat2_length_sq(mat2 m) {
    qty_t i, j; real_t s = 0.0f;
    for (i = 0; i < MAT2_DIM; i++) for (j = 0; j < MAT2_DIM; j++) s += m.m[i][j] * m.m[i][j];
    return s;
}
/* Frobenius 范数 sqrt(sum(a_ij^2)) */
static FORCE_INLINE real_t mat2_length(mat2 m) { return math_sqrt(mat2_length_sq(m)); }

/* ---- 变换 ---- */
/* 2D 旋转矩阵, 右手逆时针(数学约定); LH 世界下视觉为顺时针 */
static FORCE_INLINE mat2 mat2_rotate(real_t angle) {
    mat2 r = mat2_identity(); real_t c = math_cos(angle), s = math_sin(angle);
    r.m[0][0] = c; r.m[0][1] = -s; r.m[1][0] = s; r.m[1][1] = c; return r;
}
/* 2D 缩放矩阵(构造非均匀缩放) */
static FORCE_INLINE mat2 mat2_scale(vec2 s) { mat2 r = mat2_identity(); r.m[0][0] = s.x; r.m[1][1] = s.y; return r; }

/* ---- 数组互操作 ---- */
/* 行主序写入 out[MAT2_ELEM_COUNT] */
static FORCE_INLINE void mat2_to_array(mat2 m, real_t out[MAT2_ELEM_COUNT]) {
    qty_t i, j; for (i = 0; i < MAT2_DIM; i++) for (j = 0; j < MAT2_DIM; j++) out[i * MAT2_DIM + j] = m.m[i][j];
}
/* 行主序从 in[MAT2_ELEM_COUNT] 构造 */
static FORCE_INLINE mat2 mat2_from_array(const real_t in[MAT2_ELEM_COUNT]) {
    mat2 r; qty_t i, j; for (i = 0; i < MAT2_DIM; i++) for (j = 0; j < MAT2_DIM; j++) r.m[i][j] = in[i * MAT2_DIM + j];
    return r;
}


EXTERN_C_END
#endif