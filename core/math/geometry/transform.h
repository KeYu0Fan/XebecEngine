#ifndef MATH_GEOMETRY_TRANSFORM_H
/* 三维变换: 平移 + 四元数旋转 + 缩放 (T * R * S)
 * 与 mat4_from_trs / mat4_decompose_trs 完全对偶, 可直接互相转换
 */
#define MATH_GEOMETRY_TRANSFORM_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/math/constants.h"
#include "core/math/vector/vector.h"
#include "core/math/matrix/mat4.h"
#include "core/math/quat/quat.h"
EXTERN_C_BEGIN


/* 变换 */
typedef struct { vec3 origin; quat rotation; vec3 scale; } transform;

/* ---- 构造 ---- */
/* 单位变换 */
static FORCE_INLINE transform transform_identity(void) {
    transform t; t.origin = vec3_zero(); t.rotation = quat_identity(); t.scale = vec3_one(); return t;
}
/* 由分量构造 */
static FORCE_INLINE transform transform_new(vec3 origin, quat rotation, vec3 scale) {
    transform t; t.origin = origin; t.rotation = rotation; t.scale = scale; return t;
}
/* 由矩阵分解构造 */
static FORCE_INLINE transform transform_from_mat4(mat4 m) {
    transform t;
    mat4_decompose_trs(m, &t.origin, &t.rotation, &t.scale);
    return t;
}
/* 转为矩阵 */
static FORCE_INLINE mat4 transform_to_mat4(transform t) {
    return mat4_from_trs(t.origin, t.rotation, t.scale);
}

/* ---- 组合与求逆 ---- */
/* 组合父子变换: 返回值作用于子空间等价于先 child 再 parent */
static FORCE_INLINE transform transform_mul(transform parent, transform child) {
    transform r;
    r.rotation = quat_normalize(quat_mul(parent.rotation, child.rotation));
    r.scale = vec3_mul(parent.scale, child.scale);
    r.origin = vec3_add(parent.origin, quat_mul_vec3(parent.rotation, vec3_mul(parent.scale, child.origin)));
    return r;
}
/* 逆缩放分量; 0 轴按 0 处理(退化) */
static FORCE_INLINE vec3 transform_inv_scale(transform t) {
    return vec3_new(
        (math_abs(t.scale.x) > MATH_EPSILON) ? 1.0f / t.scale.x : 0.0f,
        (math_abs(t.scale.y) > MATH_EPSILON) ? 1.0f / t.scale.y : 0.0f,
        (math_abs(t.scale.z) > MATH_EPSILON) ? 1.0f / t.scale.z : 0.0f);
}
/* 逆变换
 * 注意: TRS 表示在非均匀缩放下不封闭, 本函数仅在缩放均匀时精确;
 *       需要非均匀缩放的精确逆请用 transform_inverse_matrix 或 transform_inv_point/dir
 * 缩放分量为 0 的轴按 0 处理(退化, 不报错) */
static FORCE_INLINE transform transform_inverse(transform t) {
    transform r;
    r.rotation = quat_conjugate(quat_normalize(t.rotation));
    r.scale = transform_inv_scale(t);
    r.origin = vec3_negate(vec3_mul(quat_mul_vec3(r.rotation, t.origin), r.scale));
    return r;
}
/* 逆变换矩阵(任意缩放都精确) */
static FORCE_INLINE mat4 transform_inverse_matrix(transform t) {
    return mat4_inverse(transform_to_mat4(t));
}

/* ---- 点与方向 ---- */
/* 变换点(含平移) */
static FORCE_INLINE vec3 transform_point(transform t, vec3 p) {
    return vec3_add(t.origin, quat_mul_vec3(t.rotation, vec3_mul(t.scale, p)));
}
/* 变换方向(忽略平移) */
static FORCE_INLINE vec3 transform_dir(transform t, vec3 d) {
    return quat_mul_vec3(t.rotation, vec3_mul(t.scale, d));
}
/* 逆变换点 */
static FORCE_INLINE vec3 transform_inv_point(transform t, vec3 p) {
    vec3 q = quat_mul_vec3(quat_conjugate(quat_normalize(t.rotation)), vec3_sub(p, t.origin));
    return vec3_mul(q, transform_inv_scale(t));
}
/* 逆变换方向 */
static FORCE_INLINE vec3 transform_inv_dir(transform t, vec3 d) {
    return vec3_mul(quat_mul_vec3(quat_conjugate(quat_normalize(t.rotation)), d), transform_inv_scale(t));
}

/* ---- 插值 ---- */
/* 平移/缩放线性插值 + 旋转球面插值 */
static FORCE_INLINE transform transform_lerp(transform a, transform b, real_t t) {
    transform r;
    r.origin = vec3_lerp(a.origin, b.origin, t);
    r.scale = vec3_lerp(a.scale, b.scale, t);
    r.rotation = quat_slerp(a.rotation, b.rotation, t);
    return r;
}
/* 是否近似相等 */
static FORCE_INLINE bool_t transform_is_equal(transform a, transform b) {
    return (vec3_is_equal(a.origin, b.origin) && vec3_is_equal(a.scale, b.scale) && quat_is_equal(a.rotation, b.rotation)) ? 1 : 0;
}


EXTERN_C_END
#endif