#ifndef MATH_VECTOR_VEC_CONVERT_H
#define MATH_VECTOR_VEC_CONVERT_H
#include "core/platform.h"
#include "core/typedef.h"
#include "core/math/scalar.h"
#include "core/math/vector/vec2.h"
#include "core/math/vector/vec3.h"
#include "core/math/vector/vec4.h"
EXTERN_C_BEGIN


/* vec2 -> vec3: 补一个 z 分量 */
static FORCE_INLINE vec3 vec3_from_vec2(vec2 v, real_t z) {
    return vec3_new(v.x, v.y, z);
}
/* vec3 -> vec2: 丢弃 z 分量 */
static FORCE_INLINE vec2 vec2_from_vec3(vec3 v) {
    return vec2_new(v.x, v.y);
}

/* vec3 -> vec4: 齐次“点”, w = 1, 参与平移 */
static FORCE_INLINE vec4 vec4_from_vec3_point(vec3 v) {
    return vec4_new(v.x, v.y, v.z, 1.0f);
}
/* vec3 -> vec4: 齐次“方向”, w = 0, 忽略平移 */
static FORCE_INLINE vec4 vec4_from_vec3_dir(vec3 v) {
    return vec4_new(v.x, v.y, v.z, 0.0f);
}
/* vec3 -> vec4: 指定 w 分量(自定义齐次坐标) */
static FORCE_INLINE vec4 vec4_from_vec3_w(vec3 v, real_t w) {
    return vec4_new(v.x, v.y, v.z, w);
}
/* vec4 -> vec3: 丢弃 w, 不做透视除法
 * 注意: 返回值的 w 为 padding, 由 vec3_new 重置为 0, 原 v.w 不保留 */
static FORCE_INLINE vec3 vec3_from_vec4(vec4 v) {
    return vec3_new(v.x, v.y, v.z);
}

/* vec2 -> vec4: 补 z|w(UI / 顶点打包常用) */
static FORCE_INLINE vec4 vec4_from_vec2(vec2 v, real_t z, real_t w) {
    return vec4_new(v.x, v.y, z, w);
}
/* vec4 -> vec2: 丢弃 z|w(取 xy, 常用于屏幕/纹理坐标)
 * 注意: 返回 vec2 无 padding, z|w 信息全部丢弃
 * 要求 v 已处于目标空间, 或 w 无意义; 齐次坐标请先调用 vec4_perspective_divide
 */
static FORCE_INLINE vec2 vec2_from_vec4(vec4 v) {
    return vec2_new(v.x, v.y);
}

/* 透视除法: 投影后的 vec4 -> 屏幕空间 vec3
 * 注意: |v.w| <= MATH_EPSILON 时返回零向量(此时屏幕坐标无意义), 
 *       该退化结果由调用方决定如何处理 */
static FORCE_INLINE vec3 vec4_perspective_divide(vec4 v) {
    return (math_abs(v.w) <= MATH_EPSILON)
        ? vec3_zero()
        : vec3_scale(vec3_from_vec4(v), (real_t)1.0 / v.w);
}


EXTERN_C_END
#endif