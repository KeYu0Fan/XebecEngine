#ifndef MATH_GEOMETRY_RAY_H
/* 射线
 * 参数方程: origin + direction * t
 * 约定: t 以 |direction| 为单位; 需要世界距离时先 ray_normalized()
 */
#define MATH_GEOMETRY_RAY_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/math/constants.h"
#include "core/math/vector/vector.h"
#include "core/math/matrix/mat4.h"
EXTERN_C_BEGIN


/* 射线 */
typedef struct { vec3 origin; vec3 direction; } ray;

/* ---- 构造 ---- */
/* 由起点与方向构造(方向不要求归一化) */
static FORCE_INLINE ray ray_new(vec3 origin, vec3 direction) {
    ray r; r.origin = origin; r.direction = direction; return r;
}
/* 是否有效(方向非零且各分量有限) */
static FORCE_INLINE bool_t ray_is_valid(ray r) {
    return (vec3_length_sq(r.direction) > MATH_EPSILON && vec3_is_finite(r.origin) && vec3_is_finite(r.direction)) ? 1 : 0;
}
/* 取参数 t 处的点 */
static FORCE_INLINE vec3 ray_at(ray r, real_t t) { return vec3_add(r.origin, vec3_scale(r.direction, t)); }
/* 方向归一化后的射线(此后 t 即世界距离) */
static FORCE_INLINE ray ray_normalized(ray r) { return ray_new(r.origin, vec3_normalize(r.direction)); }

/* ---- 距离查询 ---- */
/* 射线上离 p 最近的点; p 在起点后方时返回起点 */
static FORCE_INLINE vec3 ray_closest_point(ray r, vec3 p) {
    real_t denom = vec3_dot(r.direction, r.direction);
    if (denom < MATH_EPSILON) return r.origin;
    real_t t = vec3_dot(vec3_sub(p, r.origin), r.direction) / denom;
    return (t <= 0.0f) ? r.origin : ray_at(r, t);
}
/* 射线到点的距离平方 */
static FORCE_INLINE real_t ray_distance_sq(ray r, vec3 p) {
    vec3 d = vec3_sub(p, ray_closest_point(r, p));
    return vec3_dot(d, d);
}

/* ---- 反射与折射 ---- */
/* 反射方向, 入射方向与法线需为单位向量 */
static FORCE_INLINE vec3 ray_reflect(vec3 direction, vec3 normal) {
    return vec3_sub(direction, vec3_scale(normal, 2.0f * vec3_dot(direction, normal)));
}
/* 折射方向(斯涅尔定律), direction 与 normal 需为单位向量且满足 dot(direction, normal) < 0
 * eta 为 入射折射率 / 折射侧折射率; 全反射时返回零向量 */
static FORCE_INLINE vec3 ray_refract(vec3 direction, vec3 normal, real_t eta) {
    real_t cosi = -vec3_dot(direction, normal);
    real_t k = 1.0f - eta * eta * (1.0f - cosi * cosi);
    if (k < 0.0f) return vec3_zero();
    return vec3_add(vec3_scale(direction, eta), vec3_scale(normal, eta * cosi - math_sqrt(k)));
}

/* ---- 变换 ---- */
/* 用矩阵变换射线: 起点按点变换, 方向按方向变换 */
static FORCE_INLINE ray ray_transform(ray r, mat4 m) {
    return ray_new(mat4_transform_point(m, r.origin), mat4_transform_dir(m, r.direction));
}


EXTERN_C_END
#endif