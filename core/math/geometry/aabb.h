#ifndef MATH_GEOMETRY_AABB_H
/* 轴对齐包围盒
 * 约定: 任一轴 min > max 即视为空盒, 空盒不参与求交
 */
#define MATH_GEOMETRY_AABB_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/math/constants.h"
#include "core/math/vector/vector.h"
#include "core/math/matrix/mat4.h"
EXTERN_C_BEGIN


/* 轴对齐包围盒 */
typedef struct { vec3 min; vec3 max; } aabb;

/* ---- 构造 ---- */
/* 由最小/最大角点构造 */
static FORCE_INLINE aabb aabb_new(vec3 min, vec3 max) { aabb b; b.min = min; b.max = max; return b; }
/* 由中心与半尺寸构造 */
static FORCE_INLINE aabb aabb_new_center(vec3 center, vec3 half) {
    return aabb_new(vec3_sub(center, half), vec3_add(center, half));
}
/* 空盒(不可见) */
static FORCE_INLINE aabb aabb_empty(void) {
    return aabb_new(vec3_new(MATH_INFINITY, MATH_INFINITY, MATH_INFINITY),
        vec3_new(MATH_NEG_INFINITY, MATH_NEG_INFINITY, MATH_NEG_INFINITY));
}
/* 是否有效(非空且各轴有序); NaN 分量会判为无效 */
static FORCE_INLINE bool_t aabb_is_valid(aabb b) {
    return (b.min.x <= b.max.x && b.min.y <= b.max.y && b.min.z <= b.max.z) ? 1 : 0;
}
/* 取第 i 个角点(0..7, 位 0/1/2 对应 x/y/z 取 min 或 max) */
static FORCE_INLINE vec3 aabb_corner(aabb b, qty_t i) {
    return vec3_new((i & 1) ? b.max.x : b.min.x, (i & 2) ? b.max.y : b.min.y, (i & 4) ? b.max.z : b.min.z);
}
/* 中心点 */
static FORCE_INLINE vec3 aabb_center(aabb b) { return vec3_scale(vec3_add(b.min, b.max), 0.5f); }
/* 半尺寸 */
static FORCE_INLINE vec3 aabb_half(aabb b) { return vec3_scale(vec3_sub(b.max, b.min), 0.5f); }
/* 尺寸(直径) */
static FORCE_INLINE vec3 aabb_size(aabb b) { return vec3_sub(b.max, b.min); }
/* 体积; 空盒为 0 */
static FORCE_INLINE real_t aabb_volume(aabb b) {
    if (!aabb_is_valid(b)) return 0.0f;
    vec3 s = aabb_size(b);
    return s.x * s.y * s.z;
}
/* 表面积; 空盒为 0 */
static FORCE_INLINE real_t aabb_area(aabb b) {
    if (!aabb_is_valid(b)) return 0.0f;
    vec3 s = aabb_size(b);
    return 2.0f * (s.x * s.y + s.y * s.z + s.z * s.x);
}

/* ---- 包含与相交(同类) ---- */
/* 是否包含点(边界视为包含) */
static FORCE_INLINE bool_t aabb_contains_point(aabb b, vec3 p) {
    return (p.x >= b.min.x && p.x <= b.max.x &&
        p.y >= b.min.y && p.y <= b.max.y &&
        p.z >= b.min.z && p.z <= b.max.z) ? 1 : 0;
}
/* 是否完全包含另一个盒 */
static FORCE_INLINE bool_t aabb_contains_aabb(aabb b, aabb o) {
    if (!aabb_is_valid(b) || !aabb_is_valid(o)) return 0;
    return (o.min.x >= b.min.x && o.max.x <= b.max.x &&
        o.min.y >= b.min.y && o.max.y <= b.max.y &&
        o.min.z >= b.min.z && o.max.z <= b.max.z) ? 1 : 0;
}
/* 是否与另一个盒重叠(边界接触视为重叠) */
static FORCE_INLINE bool_t aabb_overlaps(aabb b, aabb o) {
    if (!aabb_is_valid(b) || !aabb_is_valid(o)) return 0;
    return (b.min.x <= o.max.x && b.max.x >= o.min.x &&
        b.min.y <= o.max.y && b.max.y >= o.min.y &&
        b.min.z <= o.max.z && b.max.z >= o.min.z) ? 1 : 0;
}

/* ---- 扩展与合并 ---- */
/* 扩展以包含点 */
static FORCE_INLINE aabb aabb_expand_point(aabb b, vec3 p) {
    aabb r;
    r.min = vec3_min(b.min, p);
    r.max = vec3_max(b.max, p);
    return r;
}
/* 扩展以包含另一个盒 */
static FORCE_INLINE aabb aabb_merge(aabb b, aabb o) {
    if (!aabb_is_valid(b)) return o;
    if (!aabb_is_valid(o)) return b;
    aabb r;
    r.min = vec3_min(b.min, o.min);
    r.max = vec3_max(b.max, o.max);
    return r;
}
/* 各方向外扩 margin; 负值会收缩 */
static FORCE_INLINE aabb aabb_expand_margin(aabb b, real_t margin) {
    vec3 m = vec3_splat(margin);
    return aabb_new(vec3_sub(b.min, m), vec3_add(b.max, m));
}
/* 两个盒的交集; 无交集时为空盒 */
static FORCE_INLINE aabb aabb_intersection(aabb a, aabb b) {
    if (!aabb_overlaps(a, b)) return aabb_empty();
    return aabb_new(vec3_max(a.min, b.min), vec3_min(a.max, b.max));
}

/* ---- 距离查询 ---- */
/* 盒上离 p 最近的点(点在盒内时返回自身) */
static FORCE_INLINE vec3 aabb_closest_point(aabb b, vec3 p) {
    return vec3_min(vec3_max(p, b.min), b.max);
}
/* 盒到点的距离平方 */
static FORCE_INLINE real_t aabb_distance_sq(aabb b, vec3 p) {
    vec3 d = vec3_sub(p, aabb_closest_point(b, p));
    return vec3_dot(d, d);
}

/* ---- 变换 ---- */
/* 用矩阵变换盒(取 8 角点变换后的包围盒, 旋转后会变大) */
static FORCE_INLINE aabb aabb_transform(aabb b, mat4 m) {
    if (!aabb_is_valid(b)) return aabb_empty();
    aabb r = aabb_expand_point(aabb_empty(), mat4_transform_point(m, aabb_corner(b, 0)));
    qty_t i;
    for (i = 1; i < MATH_AABB_CORNER_COUNT; i++) r = aabb_expand_point(r, mat4_transform_point(m, aabb_corner(b, i)));
    return r;
}


EXTERN_C_END
#endif