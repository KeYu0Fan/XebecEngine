#ifndef MATH_QUAT_QUAT_CONVERT_H
#define MATH_QUAT_QUAT_CONVERT_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/math/vector/vector.h"
#include "core/math/matrix/matrix.h"
#include "core/math/quat/quat.h"
EXTERN_C_BEGIN

/* 本文件集中存放 四元数 <-> 矩阵 / 欧拉角 / TRS 的互转
 * 单独成文件的原因: quat.h 依赖 matrix.h, 矩阵头不能反向依赖 quat.h,
 * 转换集中在此避免循环包含; 由 quat.h 末尾自动包含 */


/* quat -> mat3 (与 quat_to_mat3 等价, 命名对齐 mat3_from_cols) */
static FORCE_INLINE mat3 mat3_from_quat(quat q) { return quat_to_mat3(q); }
/* mat3 -> quat (与 quat_from_mat3 等价) */
static FORCE_INLINE quat mat3_to_quat(mat3 m) { return quat_from_mat3(m); }

/* quat -> mat4 (只含旋转, 无平移缩放) */
static FORCE_INLINE mat4 mat4_from_quat(quat q) { return quat_to_mat4(q); }
/* mat4 -> quat (取左上 3x3 线性部分, 要求为纯旋转) */
static FORCE_INLINE quat mat4_to_quat(mat4 m) { return quat_from_mat4(m); }

/* 平移-旋转-缩放组合为矩阵, 等价 T * R * S
 * 注意: 与 mat4_compose_trs(t, quat_to_mat3(r), s) 完全等价 */
static FORCE_INLINE mat4 mat4_from_trs(vec3 translation, quat rotation, vec3 scale) {
    return mat4_compose_trs(translation, quat_to_mat3(rotation), scale);
}
/* 兼容旧名: mat4_compose 等价 mat4_from_trs */
static FORCE_INLINE mat4 mat4_compose(vec3 translation, quat rotation, vec3 scale) {
    return mat4_from_trs(translation, rotation, scale);
}

/* 分解 TRS(无剪切), 与 mat4_from_trs 对偶
 * 平移 -> *t, 旋转 -> *r, 缩放 -> *s; 指针可为 0
 * 注意: 负缩放归到 s.x; 退化列时该轴取标准基, 不报错 */
static FORCE_INLINE void mat4_decompose_trs(mat4 m, vec3* t, quat* r, vec3* s) {
    vec3 tt, ss; mat3 rr;
    mat4_decompose_rs(m, &tt, &rr, &ss);
    if (t) *t = tt;
    if (s) *s = ss;
    if (r) *r = quat_from_mat3(rr);
}

/* 矩阵 -> 欧拉角(Pitch-X, Yaw-Y, Roll-Z), 与 mat4_from_euler 互逆 */
static FORCE_INLINE vec3 mat4_to_euler(mat4 m) { return quat_to_euler(quat_from_mat4(m)); }
/* 欧拉角 -> 矩阵, 等价 mat4_rotate_euler */
static FORCE_INLINE mat4 mat4_from_euler(vec3 euler) { return mat4_rotate_euler(euler); }


EXTERN_C_END
#endif