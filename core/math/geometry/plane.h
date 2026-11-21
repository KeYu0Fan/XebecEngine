#ifndef MATH_GEOMETRY_PLANE_H
/* 平面
 * 方程: normal·x + distance = 0, normal 为单位向量
 * 存储: (nx, ny, nz, distance) 四个 real_t, 与 vec4 同布局
 *       第 4 个分量同时承担距离, 与 vec3 用 w 填充到 4 个 real_t 同理:
 *       平面恰好 16 字节, 数组按 16 字节步长连续排列, 单次四分量点积即可算距离
 * 约定: normal 指向侧为正面, distance_to_point() > 0 表示点在正面
 */
#define MATH_GEOMETRY_PLANE_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/math/constants.h"
#include "core/math/vector/vector.h"
#include "core/math/matrix/mat4.h"
EXTERN_C_BEGIN


/* 平面: 前三个分量是单位法线, 第四个分量是距离(填充成员, 同 vec3 的 w) */
typedef struct { vec4 n; } plane;

/* ---- 访问器 ---- */
/* 取法线(单位向量) */
static FORCE_INLINE vec3 plane_normal(plane p) { return vec3_new(p.n.x, p.n.y, p.n.z); }
/* 取距离(与法线同尺度, 法线单位化后即为到原点距离) */
static FORCE_INLINE real_t plane_distance(plane p) { return p.n.w; }
/* 由四分量直接构造: xyz 为法线, w 为距离 */
static FORCE_INLINE plane plane_from_vec4(vec4 v) { plane p; p.n = v; return p; }

/* ---- 构造 ---- */
/* 由法线与距离构造(法线需已归一化) */
static FORCE_INLINE plane plane_new(vec3 normal, real_t distance) {
    plane p;
    p.n = vec4_new(normal.x, normal.y, normal.z, distance);
    return p;
}
/* 零平面(不可见) */
static FORCE_INLINE plane plane_empty(void) { return plane_from_vec4(vec4_zero()); }
/* 是否有效(法线非零且各分量有限) */
static FORCE_INLINE bool_t plane_is_valid(plane p) {
    return (vec3_length_sq(plane_normal(p)) > MATH_EPSILON && math_is_finite(p.n.w) && vec3_is_finite(plane_normal(p))) ? 1 : 0;
}
/* 由平面上一点与法线构造, 内部归一化法线; 法线为零时返回零平面 */
static FORCE_INLINE plane plane_from_point_normal(vec3 point, vec3 normal) {
    real_t len = vec3_length(normal);
    if (len < MATH_EPSILON) return plane_empty();
    vec3 n = vec3_scale(normal, 1.0f / len);
    return plane_new(n, -vec3_dot(n, point));
}
/* 由不共线三点构造, 法线方向为 (b-a)×(c-a); 退化时返回零平面 */
static FORCE_INLINE plane plane_from_points(vec3 a, vec3 b, vec3 c) {
    vec3 n = vec3_cross(vec3_sub(b, a), vec3_sub(c, a));
    real_t len = vec3_length(n);
    if (len < MATH_EPSILON) return plane_empty();
    n = vec3_scale(n, 1.0f / len);
    return plane_new(n, -vec3_dot(n, a));
}
/* 归一化; 法线为零时返回零平面 */
static FORCE_INLINE plane plane_normalize(plane p) {
    real_t len = vec3_length(plane_normal(p));
    if (len < MATH_EPSILON) return plane_empty();
    return plane_from_vec4(vec4_scale(p.n, 1.0f / len));
}
/* 翻转朝向(内外互换) */
static FORCE_INLINE plane plane_flip(plane p) { return plane_from_vec4(vec4_negate(p.n)); }

/* ---- 距离与投影 ---- */
/* 点到平面的有符号距离; 正数表示在法线指向侧
 * 用一次四分量点积完成: dot(normal, point) + distance */
static FORCE_INLINE real_t plane_distance_to_point(plane p, vec3 point) {
    return vec4_dot(p.n, vec4_new(point.x, point.y, point.z, 1.0f));
}
/* 点是否在正面(含平面上) */
static FORCE_INLINE bool_t plane_is_point_front(plane p, vec3 point) {
    return (plane_distance_to_point(p, point) >= 0.0f) ? 1 : 0;
}
/* 点投影到平面上 */
static FORCE_INLINE vec3 plane_project_point(plane p, vec3 point) {
    return vec3_sub(point, vec3_scale(plane_normal(p), plane_distance_to_point(p, point)));
}

/* ---- 变换 ---- */
/* 用矩阵变换平面: 法线用逆转置矩阵变换, 距离由变换后的点重算
 * 注意: 要求矩阵可逆; 不可逆时 mat4_inverse 会退化为单位阵, 平面原样返回 */
static FORCE_INLINE plane plane_transform(plane p, mat4 m) {
    if (!plane_is_valid(p)) return plane_empty();
    mat4 nm = mat4_transpose(mat4_inverse(m));
    vec3 n = mat4_transform_dir(nm, plane_normal(p));
    real_t len = vec3_length(n);
    if (len < MATH_EPSILON) return plane_empty();
    n = vec3_scale(n, 1.0f / len);
    vec3 point = vec3_scale(plane_normal(p), -plane_distance(p));
    point = mat4_transform_point(m, point);
    return plane_new(n, -vec3_dot(n, point));
}
/* 平面镜像矩阵(水面/镜面反射用), 对点做关于该平面的对称 */
static FORCE_INLINE mat4 plane_reflect_matrix(plane p) {
    plane n = plane_normalize(p);
    real_t x = n.n.x, y = n.n.y, z = n.n.z, d = n.n.w;
    mat4 r = mat4_identity();
    r.m[0][0] = 1.0f - 2.0f * x * x; r.m[0][1] = -2.0f * x * y;      r.m[0][2] = -2.0f * x * z;      r.m[0][3] = -2.0f * d * x;
    r.m[1][0] = -2.0f * y * x;      r.m[1][1] = 1.0f - 2.0f * y * y; r.m[1][2] = -2.0f * y * z;      r.m[1][3] = -2.0f * d * y;
    r.m[2][0] = -2.0f * z * x;      r.m[2][1] = -2.0f * z * y;      r.m[2][2] = 1.0f - 2.0f * z * z; r.m[2][3] = -2.0f * d * z;
    return r;
}

/* 两平面求交线: point 为线上一点, dir 为单位方向; 平行时返回 0 */
static FORCE_INLINE bool_t plane_intersect_plane(plane a, plane b, vec3* point, vec3* dir) {
    vec3 na = plane_normal(a), nb = plane_normal(b);
    vec3 d = vec3_cross(na, nb);
    real_t len2 = vec3_dot(d, d);
    real_t a11, a12, a22, det, b1, b2, alpha, beta;
    if (len2 < MATH_EPSILON) return 0;
    a11 = vec3_dot(na, na); a12 = vec3_dot(na, nb); a22 = vec3_dot(nb, nb);
    det = a11 * a22 - a12 * a12;
    if (math_abs(det) < MATH_EPSILON) return 0;
    b1 = -plane_distance(a); b2 = -plane_distance(b);
    alpha = (b1 * a22 - b2 * a12) / det;
    beta = (a11 * b2 - a12 * b1) / det;
    if (point) *point = vec3_add(vec3_scale(na, alpha), vec3_scale(nb, beta));
    if (dir) *dir = vec3_scale(d, 1.0f / math_sqrt(len2));
    return 1;
}


EXTERN_C_END
#endif