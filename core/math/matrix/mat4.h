#ifndef MATH_MATRIX_MAT4_H
#define MATH_MATRIX_MAT4_H
#include "core/platform.h"
#include "core/typedef.h"
#include "core/math/scalar.h"
#include "core/math/constants.h"
#include "core/math/vector/vec3.h"
#include "core/math/vector/vec4.h"
#include "core/math/matrix/mat2.h"
#include "core/math/matrix/mat3.h"
EXTERN_C_BEGIN


/* 4x4 矩阵 (64 字节) */
typedef struct { real_t m[MAT4_DIM][MAT4_DIM]; } mat4;

/* ---- 构造 ---- */
/* 返回一个元素全为 0 的零矩阵 */
static FORCE_INLINE mat4 mat4_zero(void) { mat4 r = { 0 }; return r; }
/* 返回单位矩阵 */
static FORCE_INLINE mat4 mat4_identity(void)
{
    mat4 r = mat4_zero(); r.m[0][0] = 1.0f; r.m[1][1] = 1.0f; r.m[2][2] = 1.0f; r.m[3][3] = 1.0f; return r;
}
/* 根据 16 个参数构造矩阵 */
static FORCE_INLINE mat4 mat4_new(
    real_t m00, real_t m01, real_t m02, real_t m03,
    real_t m10, real_t m11, real_t m12, real_t m13,
    real_t m20, real_t m21, real_t m22, real_t m23,
    real_t m30, real_t m31, real_t m32, real_t m33)
{
    mat4 r;
    r.m[0][0] = m00; r.m[0][1] = m01; r.m[0][2] = m02; r.m[0][3] = m03;
    r.m[1][0] = m10; r.m[1][1] = m11; r.m[1][2] = m12; r.m[1][3] = m13;
    r.m[2][0] = m20; r.m[2][1] = m21; r.m[2][2] = m22; r.m[2][3] = m23;
    r.m[3][0] = m30; r.m[3][1] = m31; r.m[3][2] = m32; r.m[3][3] = m33;
    return r;
}
/* 根据 4 个列向量构造矩阵 */
static FORCE_INLINE mat4 mat4_from_cols(vec4 c0, vec4 c1, vec4 c2, vec4 c3) {
    mat4 r;
    r.m[0][0] = c0.x; r.m[0][1] = c1.x; r.m[0][2] = c2.x; r.m[0][3] = c3.x;
    r.m[1][0] = c0.y; r.m[1][1] = c1.y; r.m[1][2] = c2.y; r.m[1][3] = c3.y;
    r.m[2][0] = c0.z; r.m[2][1] = c1.z; r.m[2][2] = c2.z; r.m[2][3] = c3.z;
    r.m[3][0] = c0.w; r.m[3][1] = c1.w; r.m[3][2] = c2.w; r.m[3][3] = c3.w;
    return r;
}
/* 根据对角线向量构造对角矩阵 */
static FORCE_INLINE mat4 mat4_diag(vec4 d) {
    mat4 r = mat4_zero(); r.m[0][0] = d.x; r.m[1][1] = d.y; r.m[2][2] = d.z; r.m[3][3] = d.w; return r;
}

/* ---- 访问器 ---- */
/* 获取指定列的向量 */
static FORCE_INLINE vec4 mat4_col(mat4 m, qty_t j) { return vec4_new(m.m[0][j], m.m[1][j], m.m[2][j], m.m[3][j]); }
/* 获取指定行的向量 */
static FORCE_INLINE vec4 mat4_row(mat4 m, qty_t i) { return vec4_new(m.m[i][0], m.m[i][1], m.m[i][2], m.m[i][3]); }
/* 设置指定列的向量 */
static FORCE_INLINE mat4 mat4_set_col(mat4 m, qty_t j, vec4 v) {
    m.m[0][j] = v.x; m.m[1][j] = v.y; m.m[2][j] = v.z; m.m[3][j] = v.w; return m;
}
/* 设置指定行的向量 */
static FORCE_INLINE mat4 mat4_set_row(mat4 m, qty_t i, vec4 v) {
    m.m[i][0] = v.x; m.m[i][1] = v.y; m.m[i][2] = v.z; m.m[i][3] = v.w; return m;
}
/* 将矩阵的列提取到 vec4 数组中 */
static FORCE_INLINE void mat4_to_cols(mat4 m, vec4* c0, vec4* c1, vec4* c2, vec4* c3) {
    *c0 = mat4_col(m, 0); *c1 = mat4_col(m, 1); *c2 = mat4_col(m, 2); *c3 = mat4_col(m, 3);
}
/* 获取矩阵的平移分量 */
static FORCE_INLINE vec3 mat4_get_translation(mat4 m) { return vec3_new(m.m[0][3], m.m[1][3], m.m[2][3]); }
/* 设置矩阵的平移分量 */
static FORCE_INLINE mat4 mat4_set_translation(mat4 m, vec3 t) {
    m.m[0][3] = t.x; m.m[1][3] = t.y; m.m[2][3] = t.z; return m;
}
/* 提取矩阵的对角线元素 */
static FORCE_INLINE vec4 mat4_extract_diag(mat4 m) {
    return vec4_new(m.m[0][0], m.m[1][1], m.m[2][2], m.m[3][3]);
}
/* 设置矩阵的对角线元素 */
static FORCE_INLINE mat4 mat4_set_diag(mat4 m, vec4 d) {
    m.m[0][0] = d.x; m.m[1][1] = d.y; m.m[2][2] = d.z; m.m[3][3] = d.w; return m;
}
/* 获取线性部分的三轴缩放(列长度) */
static FORCE_INLINE vec3 mat4_get_scale(mat4 m) {
    return vec3_new(
        vec3_length(vec3_new(m.m[0][0], m.m[1][0], m.m[2][0])),
        vec3_length(vec3_new(m.m[0][1], m.m[1][1], m.m[2][1])),
        vec3_length(vec3_new(m.m[0][2], m.m[1][2], m.m[2][2])));
}
/* 设置线性部分缩放, 保留原旋转方向; 原列长度为 0 时该列取标准基 */
static FORCE_INLINE mat4 mat4_set_scale(mat4 m, vec3 s) {
    vec3 c0 = vec3_new(m.m[0][0], m.m[1][0], m.m[2][0]);
    vec3 c1 = vec3_new(m.m[0][1], m.m[1][1], m.m[2][1]);
    vec3 c2 = vec3_new(m.m[0][2], m.m[1][2], m.m[2][2]);
    c0 = vec3_length_sq(c0) > MATH_EPSILON ? vec3_normalize(c0) : vec3_new(1.0f, 0.0f, 0.0f);
    c1 = vec3_length_sq(c1) > MATH_EPSILON ? vec3_normalize(c1) : vec3_new(0.0f, 1.0f, 0.0f);
    c2 = vec3_length_sq(c2) > MATH_EPSILON ? vec3_normalize(c2) : vec3_new(0.0f, 0.0f, 1.0f);
    m.m[0][0] = c0.x * s.x; m.m[1][0] = c0.y * s.x; m.m[2][0] = c0.z * s.x;
    m.m[0][1] = c1.x * s.y; m.m[1][1] = c1.y * s.y; m.m[2][1] = c1.z * s.y;
    m.m[0][2] = c2.x * s.z; m.m[1][2] = c2.y * s.z; m.m[2][2] = c2.z * s.z;
    return m;
}
/* 是否为仿射矩阵(第 4 行 = 0,0,0,1) */
static FORCE_INLINE bool_t mat4_is_affine(mat4 m) {
    return (math_is_equal(m.m[3][0], 0.0f) && math_is_equal(m.m[3][1], 0.0f) &&
        math_is_equal(m.m[3][2], 0.0f) && math_is_equal(m.m[3][3], 1.0f)) ? 1 : 0;
}

/* ---- 基础运算 ---- */
/* 矩阵元素级加法 */
static FORCE_INLINE mat4 mat4_add(mat4 a, mat4 b)
{
    mat4 r; qty_t i, j; for (i = 0;i < MAT4_DIM;i++)for (j = 0;j < MAT4_DIM;j++)r.m[i][j] = a.m[i][j] + b.m[i][j]; return r;
}
/* 矩阵元素级减法 */
static FORCE_INLINE mat4 mat4_sub(mat4 a, mat4 b)
{
    mat4 r; qty_t i, j; for (i = 0;i < MAT4_DIM;i++)for (j = 0;j < MAT4_DIM;j++)r.m[i][j] = a.m[i][j] - b.m[i][j]; return r;
}
/* 矩阵元素级取反 */
static FORCE_INLINE mat4 mat4_negate(mat4 m)
{
    mat4 r; qty_t i, j; for (i = 0;i < MAT4_DIM;i++)for (j = 0;j < MAT4_DIM;j++)r.m[i][j] = -m.m[i][j]; return r;
}
/* 矩阵标量乘法 */
static FORCE_INLINE mat4 mat4_mul_scalar(mat4 m, real_t s)
{
    mat4 r; qty_t i, j; for (i = 0;i < MAT4_DIM;i++)for (j = 0;j < MAT4_DIM;j++)r.m[i][j] = m.m[i][j] * s; return r;
}
/* 矩阵标量除法; s 为 0 时行为依赖浮点, 不报错 */
static FORCE_INLINE mat4 mat4_div_scalar(mat4 m, real_t s)
{
    mat4 r; qty_t i, j; for (i = 0;i < MAT4_DIM;i++)for (j = 0;j < MAT4_DIM;j++)r.m[i][j] = m.m[i][j] / s; return r;
}
/* Hadamard 积(逐元素乘), 区别于 mat4_mul(矩阵乘) */
static FORCE_INLINE mat4 mat4_cwise_mul(mat4 a, mat4 b)
{
    mat4 r; qty_t i, j; for (i = 0;i < MAT4_DIM;i++)for (j = 0;j < MAT4_DIM;j++)r.m[i][j] = a.m[i][j] * b.m[i][j]; return r;
}
/* 逐元素除; b 含 0 时行为依赖浮点, 不报错 */
static FORCE_INLINE mat4 mat4_cwise_div(mat4 a, mat4 b)
{
    mat4 r; qty_t i, j; for (i = 0;i < MAT4_DIM;i++)for (j = 0;j < MAT4_DIM;j++)r.m[i][j] = a.m[i][j] / b.m[i][j]; return r;
}
/* 矩阵线性插值 */
static FORCE_INLINE mat4 mat4_lerp(mat4 a, mat4 b, real_t t)
{
    mat4 r; qty_t i, j; for (i = 0;i < MAT4_DIM;i++)for (j = 0;j < MAT4_DIM;j++)r.m[i][j] = a.m[i][j] + (b.m[i][j] - a.m[i][j]) * t; return r;
}

/* ---- 矩阵运算 ---- */
/* 标准矩阵乘法 */
static FORCE_INLINE mat4 mat4_mul(mat4 a, mat4 b) {
    mat4 r = mat4_zero();qty_t i, j, k;
    for (i = 0;i < MAT4_DIM;i++) for (j = 0;j < MAT4_DIM;j++) for (k = 0;k < MAT4_DIM;k++) r.m[i][j] += a.m[i][k] * b.m[k][j];
    return r;
}
/* 矩阵乘法加法 */
static FORCE_INLINE mat4 mat4_mul_add(mat4 a, mat4 b, mat4 c) {
    return mat4_add(mat4_mul(a, b), c);
}
/* 矩阵向量变换 (含齐次坐标) */
static FORCE_INLINE vec4 mat4_mul_vec4(mat4 m, vec4 v) {
    return vec4_new(
        m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] * v.w,
        m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] * v.w,
        m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] * v.w,
        m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] * v.w
    );
}
/* 矩阵点变换 (w=1) */
static FORCE_INLINE vec3 mat4_transform_point(mat4 m, vec3 v) {
    return vec3_new(
        m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3],
        m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3],
        m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3]
    );
}
/* 矩阵点变换 (w=1) - 向量重载 */
static FORCE_INLINE vec4 mat4_transform_point_h(mat4 m, vec3 v) {
    return mat4_mul_vec4(m, vec4_new(v.x, v.y, v.z, 1.0f));
}
/* 矩阵方向变换 (w=0, 无平移) */
static FORCE_INLINE vec3 mat4_transform_dir(mat4 m, vec3 v) {
    return vec3_new(
        m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z,
        m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z,
        m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z
    );
}
/* mat4_transform_point 的别名(w=1, 含平移); 方向变换请用 mat4_transform_dir */
static FORCE_INLINE vec3 mat4_mul_vec3(mat4 m, vec3 v) { return mat4_transform_point(m, v); }
/* 矩阵转置 */
static FORCE_INLINE mat4 mat4_transpose(mat4 m)
{
    mat4 r; qty_t i, j; for (i = 0;i < MAT4_DIM;i++)for (j = 0;j < MAT4_DIM;j++)r.m[i][j] = m.m[j][i]; return r;
}

/* ---- 求逆 ---- */
/* 计算 4x4 矩阵行列式 */
static FORCE_INLINE real_t mat4_determinant(mat4 m) {
    real_t a00 = m.m[0][0], a01 = m.m[0][1], a02 = m.m[0][2], a03 = m.m[0][3],
        a10 = m.m[1][0], a11 = m.m[1][1], a12 = m.m[1][2], a13 = m.m[1][3],
        a20 = m.m[2][0], a21 = m.m[2][1], a22 = m.m[2][2], a23 = m.m[2][3],
        a30 = m.m[3][0], a31 = m.m[3][1], a32 = m.m[3][2], a33 = m.m[3][3];
    real_t b00 = a00 * a11 - a01 * a10, b01 = a00 * a12 - a02 * a10,
        b02 = a00 * a13 - a03 * a10, b03 = a01 * a12 - a02 * a11,
        b04 = a01 * a13 - a03 * a11, b05 = a02 * a13 - a03 * a12,
        b06 = a20 * a31 - a21 * a30, b07 = a20 * a32 - a22 * a30,
        b08 = a20 * a33 - a23 * a30, b09 = a21 * a32 - a22 * a31,
        b10 = a21 * a33 - a23 * a31, b11 = a22 * a33 - a23 * a32;
    return b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
}
/* 计算伴随矩阵 */
static FORCE_INLINE mat4 mat4_adjugate(mat4 m) {
    real_t a00 = m.m[0][0], a01 = m.m[0][1], a02 = m.m[0][2], a03 = m.m[0][3],
        a10 = m.m[1][0], a11 = m.m[1][1], a12 = m.m[1][2], a13 = m.m[1][3];
    real_t a20 = m.m[2][0], a21 = m.m[2][1], a22 = m.m[2][2], a23 = m.m[2][3],
        a30 = m.m[3][0], a31 = m.m[3][1], a32 = m.m[3][2], a33 = m.m[3][3];
    real_t b00 = a00 * a11 - a01 * a10, b01 = a00 * a12 - a02 * a10,
        b02 = a00 * a13 - a03 * a10, b03 = a01 * a12 - a02 * a11,
        b04 = a01 * a13 - a03 * a11, b05 = a02 * a13 - a03 * a12;
    real_t b06 = a20 * a31 - a21 * a30, b07 = a20 * a32 - a22 * a30,
        b08 = a20 * a33 - a23 * a30, b09 = a21 * a32 - a22 * a31,
        b10 = a21 * a33 - a23 * a31, b11 = a22 * a33 - a23 * a32;
    mat4 r;
    r.m[0][0] = a11 * b11 - a12 * b10 + a13 * b09; r.m[0][1] = -a01 * b11 + a02 * b10 - a03 * b09;
    r.m[0][2] = a31 * b05 - a32 * b04 + a33 * b03; r.m[0][3] = -a21 * b05 + a22 * b04 - a23 * b03;
    r.m[1][0] = -a10 * b11 + a12 * b08 - a13 * b07; r.m[1][1] = a00 * b11 - a02 * b08 + a03 * b07;
    r.m[1][2] = -a30 * b05 + a32 * b02 - a33 * b01; r.m[1][3] = a20 * b05 - a22 * b02 + a23 * b01;
    r.m[2][0] = a10 * b10 - a11 * b08 + a13 * b06; r.m[2][1] = -a00 * b10 + a01 * b08 - a03 * b06;
    r.m[2][2] = a30 * b04 - a31 * b02 + a33 * b00; r.m[2][3] = -a20 * b04 + a21 * b02 - a23 * b00;
    r.m[3][0] = -a10 * b09 + a11 * b07 - a12 * b06; r.m[3][1] = a00 * b09 - a01 * b07 + a02 * b06;
    r.m[3][2] = -a30 * b03 + a31 * b01 - a32 * b00; r.m[3][3] = a20 * b03 - a21 * b01 + a22 * b00;
    return r;
}
/* 矩阵求逆 (若行列式接近 0, 则返回 fallback) */
static FORCE_INLINE mat4 mat4_inverse_or(mat4 m, mat4 fallback) {
    real_t det = mat4_determinant(m);
    if (!(det == det) || det - det != 0) return fallback;   /* NaN / ±Inf */
    if (math_abs(det) < MATH_EPSILON) return fallback;
    return mat4_mul_scalar(mat4_adjugate(m), 1.0f / det);
}
/* 矩阵求逆 */
static FORCE_INLINE mat4 mat4_inverse(mat4 m) { return mat4_inverse_or(m, mat4_identity()); }
/* 矩阵逆转置 */
static FORCE_INLINE mat4 mat4_inverse_transpose(mat4 m) { return mat4_transpose(mat4_inverse(m)); }

/* ---- 比较 / 判定 ---- */
/* 精确矩阵相等比较 */
static FORCE_INLINE bool_t mat4_is_equal(mat4 a, mat4 b) {
    qty_t i, j;
    for (i = 0; i < MAT4_DIM; i++)
        for (j = 0; j < MAT4_DIM; j++)
            if (!math_is_equal(a.m[i][j], b.m[i][j])) return 0;
    return 1;
}
/* 带阈值的矩阵相等比较 */
static FORCE_INLINE bool_t mat4_is_equal_eps(mat4 a, mat4 b, real_t eps) {
    qty_t i, j;
    for (i = 0; i < MAT4_DIM; i++)
        for (j = 0; j < MAT4_DIM; j++)
            if (!(math_abs(a.m[i][j] - b.m[i][j]) <= eps)) return 0;   /* NaN 判不等 */
    return 1;
}
/* 矩阵元素有限性检查 */
static FORCE_INLINE bool_t mat4_is_finite(mat4 m) {
    qty_t i, j;
    for (i = 0; i < MAT4_DIM; i++) for (j = 0; j < MAT4_DIM; j++) {
        real_t v = m.m[i][j];
        if (!(v == v) || v - v != 0) return 0;
    }
    return 1;
}
/* 单位矩阵判定 */
static FORCE_INLINE bool_t mat4_is_identity(mat4 m) {
    qty_t i, j;
    for (i = 0; i < MAT4_DIM; i++) for (j = 0; j < MAT4_DIM; j++)
        if (!math_is_equal(m.m[i][j], (i == j) ? 1.0f : 0.0f)) return 0;
    return 1;
}
/* 判定左上 3x3 线性部分是否为正交矩阵(列 orthonormal); 不检查第 4 列/行及透视分量 */
static FORCE_INLINE bool_t mat4_is_orthogonal(mat4 m) {
    vec3 c0 = vec3_new(m.m[0][0], m.m[1][0], m.m[2][0]);
    vec3 c1 = vec3_new(m.m[0][1], m.m[1][1], m.m[2][1]);
    vec3 c2 = vec3_new(m.m[0][2], m.m[1][2], m.m[2][2]);
    return math_abs(vec3_dot(c0, c0) - 1.0f) < MATH_EPSILON &&
        math_abs(vec3_dot(c1, c1) - 1.0f) < MATH_EPSILON &&
        math_abs(vec3_dot(c2, c2) - 1.0f) < MATH_EPSILON &&
        math_abs(vec3_dot(c0, c1)) < MATH_EPSILON &&
        math_abs(vec3_dot(c0, c2)) < MATH_EPSILON &&
        math_abs(vec3_dot(c1, c2)) < MATH_EPSILON;
}
/* 是否仿射且线性部分为纯旋转(正交且行列式为 1) */
static FORCE_INLINE bool_t mat4_is_rotation(mat4 m) {
    mat3 b;
    if (!mat4_is_affine(m)) return 0;
    b = mat3_from_cols(
        vec3_new(m.m[0][0], m.m[1][0], m.m[2][0]),
        vec3_new(m.m[0][1], m.m[1][1], m.m[2][1]),
        vec3_new(m.m[0][2], m.m[1][2], m.m[2][2]));
    return mat3_is_rotation(b);
}
/* 对左上 3x3 线性部分做施密特正交化; 平移与第 4 行保持不变 */
static FORCE_INLINE mat4 mat4_orthonormalize(mat4 m) {
    mat3 b = mat3_orthonormalize(mat3_from_cols(
        vec3_new(m.m[0][0], m.m[1][0], m.m[2][0]),
        vec3_new(m.m[0][1], m.m[1][1], m.m[2][1]),
        vec3_new(m.m[0][2], m.m[1][2], m.m[2][2])));
    qty_t i, j;
    for (i = 0; i < MAT3_DIM; i++) for (j = 0; j < MAT3_DIM; j++) m.m[i][j] = b.m[i][j];
    return m;
}
/* 矩阵迹 */
static FORCE_INLINE real_t mat4_trace(mat4 m) { return m.m[0][0] + m.m[1][1] + m.m[2][2] + m.m[3][3]; }
/* Frobenius 内积(矩阵点积) sum(a_ij * b_ij); a == b 时等于 length_sq */
static FORCE_INLINE real_t mat4_dot(mat4 a, mat4 b) {
    qty_t i, j; real_t s = 0.0f;
    for (i = 0; i < MAT4_DIM; i++) for (j = 0; j < MAT4_DIM; j++) s += a.m[i][j] * b.m[i][j];
    return s;
}
/* Frobenius 范数平方 sum(a_ij^2); 用于仅比较大小时避免开方 */
static FORCE_INLINE real_t mat4_length_sq(mat4 m) {
    qty_t i, j; real_t s = 0.0f;
    for (i = 0; i < MAT4_DIM; i++) for (j = 0; j < MAT4_DIM; j++) s += m.m[i][j] * m.m[i][j];
    return s;
}
/* Frobenius 范数 sqrt(sum(a_ij^2)) */
static FORCE_INLINE real_t mat4_length(mat4 m) { return math_sqrt(mat4_length_sq(m)); }

/* ---- 前向声明 ---- */
/* mat3 -> mat4 转换, 供下方 mat4_compose_trs 使用; 实现见"转换"节 */
static FORCE_INLINE mat4 mat3_to_mat4(mat3 m);

/* ---- 核心变换 ---- */
/* 平移矩阵 */
static FORCE_INLINE mat4 mat4_translate(vec3 t)
{
    mat4 r = mat4_identity(); r.m[0][3] = t.x; r.m[1][3] = t.y; r.m[2][3] = t.z; return r;
}
/* 绕 X 轴旋转矩阵 */
static FORCE_INLINE mat4 mat4_rotate_x(real_t a) {
    mat4 r = mat4_identity(); real_t c = math_cos(a), s = math_sin(a);
    r.m[1][1] = c;r.m[1][2] = -s;r.m[2][1] = s;r.m[2][2] = c;
    return r;
}
/* 绕 Y 轴旋转矩阵 */
static FORCE_INLINE mat4 mat4_rotate_y(real_t a) {
    mat4 r = mat4_identity(); real_t c = math_cos(a), s = math_sin(a);
    r.m[0][0] = c;r.m[0][2] = s;r.m[2][0] = -s;r.m[2][2] = c;
    return r;
}
/* 绕 Z 轴旋转矩阵 */
static FORCE_INLINE mat4 mat4_rotate_z(real_t a) {
    mat4 r = mat4_identity(); real_t c = math_cos(a), s = math_sin(a);
    r.m[0][0] = c;r.m[0][1] = -s;r.m[1][0] = s;r.m[1][1] = c;
    return r;
}
/* 绕任意轴旋转矩阵 */
static FORCE_INLINE mat4 mat4_rotate_axis(vec3 axis, real_t angle) {
    mat4 r = mat4_identity(); real_t c = math_cos(angle), s = math_sin(angle), t = 1.0f - c;
    real_t x = axis.x, y = axis.y, z = axis.z;
    r.m[0][0] = t * x * x + c; r.m[0][1] = t * x * y - s * z; r.m[0][2] = t * x * z + s * y;
    r.m[1][0] = t * x * y + s * z; r.m[1][1] = t * y * y + c; r.m[1][2] = t * y * z - s * x;
    r.m[2][0] = t * x * z - s * y; r.m[2][1] = t * y * z + s * x; r.m[2][2] = t * z * z + c;
    return r;
}
/* 绕任意轴旋转矩阵 (带安全归一化) */
static FORCE_INLINE mat4 mat4_rotate_axis_safe(vec3 axis, real_t angle) {
    return mat4_rotate_axis(vec3_normalize(axis), angle);
}
/* 欧拉角旋转矩阵, R = Rz*Ry*Rx; 列向量依次绕 X → Y → Z 旋转 */
static FORCE_INLINE mat4 mat4_rotate_euler(vec3 euler) {
    return mat4_mul(mat4_mul(mat4_rotate_z(euler.z), mat4_rotate_y(euler.y)), mat4_rotate_x(euler.x));
}
/* 缩放矩阵 */
static FORCE_INLINE mat4 mat4_scale(vec3 s)
{
    mat4 r = mat4_identity(); r.m[0][0] = s.x; r.m[1][1] = s.y; r.m[2][2] = s.z; return r;
}
/* 组合 TRS (平移-旋转-缩放) */
static FORCE_INLINE mat4 mat4_compose_trs(vec3 t, mat3 r, vec3 s) {
    mat4 out = mat3_to_mat4(r);
    qty_t i;
    for (i = 0; i < MAT3_DIM; i++) { out.m[i][0] *= s.x; out.m[i][1] *= s.y; out.m[i][2] *= s.z; }
    out.m[0][3] = t.x; out.m[1][3] = t.y; out.m[2][3] = t.z;
    return out;
}
/* TRS 分解(无剪切); 与 mat4_compose_trs 对偶
 * 平移 -> *t, 旋转(线性部分, 已归一化) -> *r, 缩放 -> *s
 * 负缩放归到 s.x; 退化列(长度为 0)时该列旋转取标准基, 不报错 */
static FORCE_INLINE void mat4_decompose_rs(mat4 m, vec3* t, mat3* r, vec3* s) {
    vec3 c0, c1, c2, n0, n1, n2;
    real_t sx, sy, sz;

    c0 = vec3_new(m.m[0][0], m.m[1][0], m.m[2][0]);
    c1 = vec3_new(m.m[0][1], m.m[1][1], m.m[2][1]);
    c2 = vec3_new(m.m[0][2], m.m[1][2], m.m[2][2]);

    sx = vec3_length(c0);
    sy = vec3_length(c1);
    sz = vec3_length(c2);
    if (vec3_dot(vec3_cross(c0, c1), c2) < 0.0f) {
        sx = -sx;   /* 负缩放归到 X */
    }

    *t = mat4_get_translation(m);
    *s = vec3_new(sx, sy, sz);

    if (sx != 0.0f) {
        n0 = vec3_new(c0.x / sx, c0.y / sx, c0.z / sx);
    }
    else {
        n0 = vec3_new(1.0f, 0.0f, 0.0f);
    }
    if (sy != 0.0f) {
        n1 = vec3_new(c1.x / sy, c1.y / sy, c1.z / sy);
    }
    else {
        n1 = vec3_new(0.0f, 1.0f, 0.0f);
    }
    if (sz != 0.0f) {
        n2 = vec3_new(c2.x / sz, c2.y / sz, c2.z / sz);
    }
    else {
        n2 = vec3_new(0.0f, 0.0f, 1.0f);
    }

    *r = mat3_from_cols(n0, n1, n2);
}

/* ---- 投影与视图 ---- */
/* 透视投影矩阵(左手系, Vulkan 约定: NDC Y 向下, Z∈[0,1]) */
static FORCE_INLINE mat4 mat4_perspective(real_t fov, real_t aspect, real_t n, real_t f) {
    mat4 m = mat4_zero(); real_t th = math_tan(fov * 0.5f);
    m.m[0][0] = 1.0f / (aspect * th);
    m.m[1][1] = -1.0f / th;
    m.m[2][2] = f / (f - n);
    m.m[2][3] = -(f * n) / (f - n);
    m.m[3][2] = 1.0f;
    return m;
}
/* 右手系透视投影矩阵(Vulkan 约定: NDC Y 向下, Z∈[0,1]) */
static FORCE_INLINE mat4 mat4_perspective_rh(real_t fov, real_t aspect, real_t n, real_t f) {
    mat4 m = mat4_zero(); real_t th = math_tan(fov * 0.5f);
    m.m[0][0] = 1.0f / (aspect * th);
    m.m[1][1] = -1.0f / th;
    m.m[2][2] = -f / (f - n);
    m.m[2][3] = -(f * n) / (f - n);
    m.m[3][2] = -1.0f;
    return m;
}
/* 无限远透视投影矩阵(左手系, Vulkan 约定: NDC Y 向下, Z∈[0,1]) */
static FORCE_INLINE mat4 mat4_perspective_infinite(real_t fov, real_t aspect, real_t n) {
    mat4 m = mat4_zero(); real_t th = math_tan(fov * 0.5f);
    m.m[0][0] = 1.0f / (aspect * th);
    m.m[1][1] = -1.0f / th;
    m.m[2][2] = 1.0f;
    m.m[2][3] = -n;
    m.m[3][2] = 1.0f;
    return m;
}
/* 视锥体投影矩阵(左手系, Vulkan 约定: NDC Y 向下, Z∈[0,1]) */
static FORCE_INLINE mat4 mat4_frustum(real_t l, real_t r, real_t b, real_t t, real_t n, real_t f) {
    mat4 m = mat4_zero();
    m.m[0][0] = 2.0f * n / (r - l);   m.m[0][2] = -(r + l) / (r - l);
    m.m[1][1] = -2.0f * n / (t - b);  m.m[1][2] = (t + b) / (t - b);
    m.m[2][2] = f / (f - n);          m.m[2][3] = -(f * n) / (f - n);
    m.m[3][2] = 1.0f;
    return m;
}
static FORCE_INLINE mat4 mat4_perspective_off_center(real_t l, real_t r, real_t b, real_t t, real_t n, real_t f) {
    return mat4_frustum(l, r, b, t, n, f);
}
/* 正交投影矩阵(Vulkan 约定: NDC Y 向下, Z∈[0,1]) */
static FORCE_INLINE mat4 mat4_ortho(real_t l, real_t r, real_t b, real_t t, real_t n, real_t f) {
    mat4 m = mat4_identity();
    m.m[0][0] = 2.0f / (r - l);   m.m[0][3] = -(r + l) / (r - l);
    m.m[1][1] = -2.0f / (t - b);  m.m[1][3] = (t + b) / (t - b);
    m.m[2][2] = 1.0f / (f - n);   m.m[2][3] = -n / (f - n);
    return m;
}
/* 2D 正交投影: 世界 (0,0)-(w,h) 映射到视口, Y 向上(y=0 在底部); framebuffer 左上原点需另行翻转 Y */
static FORCE_INLINE mat4 mat4_ortho_2d(real_t w, real_t h) {
    return mat4_ortho(0.0f, w, 0.0f, h, -1.0f, 1.0f);
}
/* NDC 约定转换: 翻转 Y(用于 y 向上/向下的图形 API 互换) */
static FORCE_INLINE mat4 mat4_ndc_y_flip(mat4 m) {
    m.m[1][0] = -m.m[1][0]; m.m[1][1] = -m.m[1][1]; m.m[1][2] = -m.m[1][2]; m.m[1][3] = -m.m[1][3];
    return m;
}
/* NDC 约定转换: 深度 [0,1] -> [-1,1](Vulkan/D3D 转 OpenGL) */
static FORCE_INLINE mat4 mat4_ndc_z_to_gl(mat4 m) {
    qty_t j;
    for (j = 0; j < MAT4_DIM; j++) m.m[2][j] = 2.0f * m.m[2][j] - m.m[3][j];
    return m;
}
/* 投影矩阵从 Vulkan 约定(y 向下, z∈[0,1]) 转为 OpenGL 约定(y 向上, z∈[-1,1])
 * 注意: 只改裁剪空间输出, 世界手系与视图矩阵不受影响 */
static FORCE_INLINE mat4 mat4_ndc_to_gl(mat4 projection) {
    return mat4_ndc_z_to_gl(mat4_ndc_y_flip(projection));
}
/* 视线矩阵 (左手系) */
static FORCE_INLINE mat4 mat4_look_at(vec3 eye, vec3 center, vec3 up) {
    vec3 f = vec3_normalize(vec3_sub(center, eye));
    vec3 r = vec3_normalize(vec3_cross(up, f));
    vec3 u = vec3_cross(f, r);
    mat4 m = mat4_identity();
    m.m[0][0] = r.x; m.m[0][1] = r.y; m.m[0][2] = r.z; m.m[0][3] = -vec3_dot(r, eye);
    m.m[1][0] = u.x; m.m[1][1] = u.y; m.m[1][2] = u.z; m.m[1][3] = -vec3_dot(u, eye);
    m.m[2][0] = f.x; m.m[2][1] = f.y; m.m[2][2] = f.z; m.m[2][3] = -vec3_dot(f, eye);
    return m;
}
/* 视线矩阵 (右手系) */
static FORCE_INLINE mat4 mat4_look_at_rh(vec3 eye, vec3 center, vec3 up) {
    vec3 f = vec3_normalize(vec3_sub(center, eye));
    vec3 r = vec3_normalize(vec3_cross(f, up));
    vec3 u = vec3_cross(r, f);
    mat4 m = mat4_identity();
    m.m[0][0] = r.x; m.m[0][1] = r.y; m.m[0][2] = r.z; m.m[0][3] = -vec3_dot(r, eye);
    m.m[1][0] = u.x; m.m[1][1] = u.y; m.m[1][2] = u.z; m.m[1][3] = -vec3_dot(u, eye);
    m.m[2][0] = -f.x; m.m[2][1] = -f.y; m.m[2][2] = -f.z; m.m[2][3] = vec3_dot(f, eye);
    return m;
}

/* ---- 投影与反投影 ---- */
/* 3D 点投影到窗口坐标; 返回 (窗口 x, 窗口 y, NDC 深度 z∈[0,1]) */
static FORCE_INLINE vec3 mat4_project(vec3 obj, mat4 model, mat4 view, mat4 proj, vec4 viewport) {
    mat4 m = mat4_mul(proj, mat4_mul(view, model));
    vec4 clip = mat4_mul_vec4(m, vec4_new(obj.x, obj.y, obj.z, 1.0f));
    real_t inv_w = (clip.w != 0.0f) ? 1.0f / clip.w : 1.0f;
    real_t nx = clip.x * inv_w, ny = clip.y * inv_w, nz = clip.z * inv_w;
    return vec3_new(
        viewport.x + (nx * 0.5f + 0.5f) * viewport.z,
        viewport.y + (ny * 0.5f + 0.5f) * viewport.w,
        nz);
}
/* 窗口点反投影到 3D 空间: win.z 须为 NDC 深度∈[0,1], 与 mat4_project 互逆 */
static FORCE_INLINE vec3 mat4_unproject(vec3 win, mat4 model, mat4 view, mat4 proj, vec4 viewport) {
    real_t nx = (win.x - viewport.x) / viewport.z * 2.0f - 1.0f;
    real_t ny = (win.y - viewport.y) / viewport.w * 2.0f - 1.0f;
    mat4 inv = mat4_inverse(mat4_mul(proj, mat4_mul(view, model)));
    vec4 p = mat4_mul_vec4(inv, vec4_new(nx, ny, win.z, 1.0f));
    real_t inv_w = (p.w != 0.0f) ? 1.0f / p.w : 1.0f;
    return vec3_new(p.x * inv_w, p.y * inv_w, p.z * inv_w);
}

/* ---- 转换 ---- */
/* mat2 -> mat4 */
static FORCE_INLINE mat4 mat2_to_mat4(mat2 m) {
    mat4 r = mat4_identity();
    qty_t i, j;
    for (i = 0; i < MAT2_DIM; i++) for (j = 0; j < MAT2_DIM; j++) r.m[i][j] = m.m[i][j];
    return r;
}
/* mat3 -> mat4 实现(前向声明见"核心变换"节前) */
static FORCE_INLINE mat4 mat3_to_mat4(mat3 m) {
    mat4 r = mat4_identity();
    qty_t i, j;
    for (i = 0; i < MAT3_DIM; i++) for (j = 0; j < MAT3_DIM; j++) r.m[i][j] = m.m[i][j];
    return r;
}
/* mat4 -> mat3 */
static FORCE_INLINE mat3 mat4_to_mat3(mat4 m)
{
    mat3 r; qty_t i, j; for (i = 0;i < MAT3_DIM;i++)for (j = 0;j < MAT3_DIM;j++)r.m[i][j] = m.m[i][j]; return r;
}
/* mat4 -> mat2 */
static FORCE_INLINE mat2 mat4_to_mat2(mat4 m)
{
    mat2 r; qty_t i, j; for (i = 0;i < MAT2_DIM;i++)for (j = 0;j < MAT2_DIM;j++)r.m[i][j] = m.m[i][j]; return r;
}

/* ---- 数组互操作 ---- */
/* 矩阵数据填充到数组 */
static FORCE_INLINE void mat4_to_array(mat4 m, real_t out[MAT4_ELEM_COUNT]) {
    qty_t i, j; for (i = 0; i < MAT4_DIM; i++) for (j = 0; j < MAT4_DIM; j++) out[i * MAT4_DIM + j] = m.m[i][j];
}
/* 数组数据构造矩阵 */
static FORCE_INLINE mat4 mat4_from_array(const real_t in[MAT4_ELEM_COUNT]) {
    mat4 r; qty_t i, j; for (i = 0; i < MAT4_DIM; i++) for (j = 0; j < MAT4_DIM; j++) r.m[i][j] = in[i * MAT4_DIM + j];
    return r;
}
/* 列主序写入 out[MAT4_ELEM_COUNT], 供 GPU 上传(Vulkan/D3D 期望列主序) */
static FORCE_INLINE void mat4_to_array_col(mat4 m, real_t out[MAT4_ELEM_COUNT]) {
    qty_t i, j; for (i = 0; i < MAT4_DIM; i++) for (j = 0; j < MAT4_DIM; j++) out[j * MAT4_DIM + i] = m.m[i][j];
}
/* 列主序从 in[MAT4_ELEM_COUNT] 构造 */
static FORCE_INLINE mat4 mat4_from_array_col(const real_t in[MAT4_ELEM_COUNT]) {
    mat4 r; qty_t i, j; for (i = 0; i < MAT4_DIM; i++) for (j = 0; j < MAT4_DIM; j++) r.m[i][j] = in[j * MAT4_DIM + i];
    return r;
}


EXTERN_C_END
#endif