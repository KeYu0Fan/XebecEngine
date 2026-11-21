#ifndef MATH_GEOMETRY_INTERSECT_H
/* 几何求交
 * 跨类型的相交测试集中在此, 避免 aabb/sphere/plane/ray 互相包含造成循环依赖
 * 约定: ray 的 t 以 |direction| 为单位; 命中结果中的法线为单位向量
 */
#define MATH_GEOMETRY_INTERSECT_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/math/constants.h"
#include "core/math/vector/vector.h"
#include "core/math/geometry/aabb.h"
#include "core/math/geometry/sphere.h"
#include "core/math/geometry/plane.h"
#include "core/math/geometry/ray.h"
#include "core/math/geometry/segment.h"
#include "core/math/geometry/triangle.h"
#include "core/math/geometry/capsule.h"
#include "core/math/geometry/obb.h"
EXTERN_C_BEGIN


/* 射线命中信息 */
typedef struct { real_t t; vec3 point; vec3 normal; } ray_hit;

/* ---- 射线求交 ---- */
/* 射线与盒求交(slab 法); 命中时给出进入/离开参数
 * 注意: 起点在盒内时 tmin 为 0; 方向分量为 0 的轴按平行处理 */
static FORCE_INLINE bool_t intersect_ray_aabb(ray r, aabb b, real_t* tmin, real_t* tmax) {
    if (!aabb_is_valid(b) || !ray_is_valid(r)) return 0;
    real_t t0 = 0.0f, t1 = MATH_INFINITY;
    qty_t i;
    for (i = 0; i < 3; i++) {
        real_t o, d, mn, mx, tn, tf;
        if (i == 0) { o = r.origin.x; d = r.direction.x; mn = b.min.x; mx = b.max.x; }
        else if (i == 1) { o = r.origin.y; d = r.direction.y; mn = b.min.y; mx = b.max.y; }
        else { o = r.origin.z; d = r.direction.z; mn = b.min.z; mx = b.max.z; }
        if (math_abs(d) < MATH_EPSILON) {
            if (o < mn || o > mx) return 0;
        }
        else {
            real_t inv = 1.0f / d;
            tn = (mn - o) * inv;
            tf = (mx - o) * inv;
            if (tn > tf) { real_t tmp = tn; tn = tf; tf = tmp; }
            if (tn > t0) t0 = tn;
            if (tf < t1) t1 = tf;
            if (t0 > t1) return 0;
        }
    }
    if (tmin) *tmin = t0;
    if (tmax) *tmax = t1;
    return 1;
}
/* 射线与球求交; 返回最近的 t >= 0 命中 */
static FORCE_INLINE bool_t intersect_ray_sphere(ray r, sphere s, ray_hit* hit) {
    if (!sphere_is_valid(s) || !ray_is_valid(r)) return 0;
    vec3 oc = vec3_sub(r.origin, s.center);
    real_t a = vec3_dot(r.direction, r.direction);
    real_t b = 2.0f * vec3_dot(oc, r.direction);
    real_t c = vec3_dot(oc, oc) - s.radius * s.radius;
    real_t disc = b * b - 4.0f * a * c;
    if (disc < 0.0f) return 0;
    real_t sq = math_sqrt(disc);
    real_t t = (-b - sq) / (2.0f * a);
    if (t < 0.0f) t = (-b + sq) / (2.0f * a);
    if (t < 0.0f) return 0;
    if (hit) {
        hit->t = t;
        hit->point = ray_at(r, t);
        hit->normal = vec3_normalize(vec3_sub(hit->point, s.center));
    }
    return 1;
}
/* 射线与平面求交; 平行或命中点在起点后方时返回 0 */
static FORCE_INLINE bool_t intersect_ray_plane(ray r, plane p, ray_hit* hit) {
    vec3 pn;
    if (!plane_is_valid(p) || !ray_is_valid(r)) return 0;
    pn = plane_normal(p);
    real_t denom = vec3_dot(pn, r.direction);
    if (math_abs(denom) < MATH_EPSILON) return 0;
    real_t t = -(vec3_dot(pn, r.origin) + plane_distance(p)) / denom;
    if (t < 0.0f) return 0;
    if (hit) {
        hit->t = t;
        hit->point = ray_at(r, t);
        hit->normal = pn;
    }
    return 1;
}
/* 射线与三角形求交(Möller–Trumbore, 双面); 法线取 (b-a)×(c-a) 方向 */
static FORCE_INLINE bool_t intersect_ray_triangle(ray r, vec3 a, vec3 b, vec3 c, ray_hit* hit) {
    if (!ray_is_valid(r)) return 0;
    vec3 e1 = vec3_sub(b, a), e2 = vec3_sub(c, a);
    vec3 pv = vec3_cross(r.direction, e2);
    real_t det = vec3_dot(e1, pv);
    if (math_abs(det) < MATH_EPSILON) return 0;
    real_t inv = 1.0f / det;
    vec3 tv = vec3_sub(r.origin, a);
    real_t u = vec3_dot(tv, pv) * inv;
    if (u < 0.0f || u > 1.0f) return 0;
    vec3 qv = vec3_cross(tv, e1);
    real_t v = vec3_dot(r.direction, qv) * inv;
    if (v < 0.0f || u + v > 1.0f) return 0;
    real_t t = vec3_dot(e2, qv) * inv;
    if (t < 0.0f) return 0;
    if (hit) {
        hit->t = t;
        hit->point = ray_at(r, t);
        hit->normal = vec3_normalize(vec3_cross(e1, e2));
    }
    return 1;
}

/* ---- 体与体 ---- */
/* 球与盒是否相交 */
static FORCE_INLINE bool_t intersect_sphere_aabb(sphere s, aabb b) {
    if (!sphere_is_valid(s) || !aabb_is_valid(b)) return 0;
    return (aabb_distance_sq(b, s.center) <= s.radius * s.radius) ? 1 : 0;
}
/* 球与球是否相交 */
static FORCE_INLINE bool_t intersect_sphere_sphere(sphere a, sphere b) {
    return sphere_overlaps(a, b);
}
/* 盒与盒是否相交 */
static FORCE_INLINE bool_t intersect_aabb_aabb(aabb a, aabb b) {
    return aabb_overlaps(a, b);
}

/* ---- 最近点 ---- */
/* 线段 ab 上离 p 最近的点; 返回参数 t ∈ [0,1] */
static FORCE_INLINE real_t intersect_closest_point_segment(vec3 a, vec3 b, vec3 p, vec3* out) {
    real_t t;
    vec3 q = segment_closest_point_t(segment_new(a, b), p, &t);
    if (out) *out = q;
    return t;
}
/* 三角形 abc 上离 p 最近的点(Ericson 区域判定) */
static FORCE_INLINE vec3 intersect_closest_point_triangle(vec3 a, vec3 b, vec3 c, vec3 p) {
    return triangle_closest_point(triangle_new(a, b, c), p);
}

/* ---- 线段求交 ---- */
/* 两线段最近点; 返回最近距离平方, 最近点写入 p1/p2(可为 0) */
static FORCE_INLINE real_t intersect_segment_segment(segment s1, segment s2, vec3* p1, vec3* p2) {
    vec3 d1 = segment_direction(s1), d2 = segment_direction(s2);
    vec3 r = vec3_sub(s1.a, s2.a);
    real_t a = vec3_dot(d1, d1), e = vec3_dot(d2, d2), f = vec3_dot(d2, r);
    real_t s, t;
    if (a <= MATH_EPSILON && e <= MATH_EPSILON) {
        s = 0.0f; t = 0.0f;
    }
    else if (a <= MATH_EPSILON) {
        s = 0.0f;
        t = math_clamp(f / e, 0.0f, 1.0f);
    }
    else {
        real_t c = vec3_dot(d1, r);
        if (e <= MATH_EPSILON) {
            t = 0.0f;
            s = math_clamp(-c / a, 0.0f, 1.0f);
        }
        else {
            real_t b = vec3_dot(d1, d2);
            real_t denom = a * e - b * b;
            s = (denom > MATH_EPSILON) ? math_clamp((b * f - c * e) / denom, 0.0f, 1.0f) : 0.0f;
            t = (b * s + f) / e;
            if (t < 0.0f) { t = 0.0f; s = math_clamp(-c / a, 0.0f, 1.0f); }
            else if (t > 1.0f) { t = 1.0f; s = math_clamp((b - c) / a, 0.0f, 1.0f); }
        }
    }
    {
        vec3 q1 = segment_at(s1, s), q2 = segment_at(s2, t);
        if (p1) *p1 = q1;
        if (p2) *p2 = q2;
        return vec3_distance_sq(q1, q2);
    }
}
/* 线段与平面求交; 整条线段在平面上时返回 t=0 的命中 */
static FORCE_INLINE bool_t intersect_segment_plane(segment s, plane p, ray_hit* hit) {
    real_t da, db, denom, t;
    if (!plane_is_valid(p) || !segment_is_valid(s)) return 0;
    da = plane_distance_to_point(p, s.a);
    db = plane_distance_to_point(p, s.b);
    if ((da > 0.0f && db > 0.0f) || (da < 0.0f && db < 0.0f)) return 0;
    denom = da - db;
    if (math_abs(denom) < MATH_EPSILON) {
        if (math_abs(da) < MATH_EPSILON) {
            if (hit) { hit->t = 0.0f; hit->point = s.a; hit->normal = plane_normal(p); }
            return 1;
        }
        return 0;
    }
    t = da / denom;
    if (t < 0.0f || t > 1.0f) return 0;
    if (hit) { hit->t = t; hit->point = segment_at(s, t); hit->normal = plane_normal(p); }
    return 1;
}
/* 线段与盒是否相交 */
static FORCE_INLINE bool_t intersect_segment_aabb(segment s, aabb b) {
    ray r;
    real_t t0, t1;
    if (!segment_is_valid(s) || !aabb_is_valid(b)) return 0;
    r = ray_new(s.a, segment_direction(s));
    if (!intersect_ray_aabb(r, b, &t0, &t1)) return 0;
    return (t0 <= 1.0f && t1 >= 0.0f) ? 1 : 0;
}
/* 线段与三角形求交 */
static FORCE_INLINE bool_t intersect_segment_triangle(segment s, triangle t, ray_hit* hit) {
    ray r;
    ray_hit h;
    if (!triangle_is_valid(t)) return 0;
    r = ray_new(s.a, segment_direction(s));
    if (!intersect_ray_triangle(r, t.a, t.b, t.c, &h)) return 0;
    if (h.t < 0.0f || h.t > 1.0f) return 0;
    if (hit) *hit = h;
    return 1;
}

/* ---- 胶囊体求交 ---- */
/* 胶囊体与球是否相交 */
static FORCE_INLINE bool_t intersect_capsule_sphere(capsule c, sphere s) {
    real_t r;
    if (!capsule_is_valid(c) || !sphere_is_valid(s)) return 0;
    r = c.radius + s.radius;
    return (segment_distance_sq(capsule_segment(c), s.center) <= r * r) ? 1 : 0;
}
/* 胶囊体与平面是否相交(中线跨过平面即相交) */
static FORCE_INLINE bool_t intersect_capsule_plane(capsule c, plane p) {
    real_t da, db;
    if (!capsule_is_valid(c) || !plane_is_valid(p)) return 0;
    da = plane_distance_to_point(p, c.a);
    db = plane_distance_to_point(p, c.b);
    if (da * db <= 0.0f) return 1;
    return (math_min(math_abs(da), math_abs(db)) <= c.radius) ? 1 : 0;
}
/* 胶囊体与胶囊体是否相交 */
static FORCE_INLINE bool_t intersect_capsule_capsule(capsule a, capsule b) {
    real_t r;
    if (!capsule_is_valid(a) || !capsule_is_valid(b)) return 0;
    r = a.radius + b.radius;
    return (intersect_segment_segment(capsule_segment(a), capsule_segment(b), 0, 0) <= r * r) ? 1 : 0;
}

/* ---- 有向包围盒求交 ---- */
/* 射线与 OBB 求交: 变换到盒局部空间后用 slab 法 */
static FORCE_INLINE bool_t intersect_ray_obb(ray r, obb b, real_t* tmin, real_t* tmax) {
    mat3 inv;
    ray local;
    aabb box;
    real_t t0, t1;
    if (!obb_is_valid(b) || !ray_is_valid(r)) return 0;
    inv = mat3_transpose(b.rotation);
    local = ray_new(mat3_mul_vec3(inv, vec3_sub(r.origin, b.center)), mat3_mul_vec3(inv, r.direction));
    box = aabb_new(vec3_negate(b.half), b.half);
    if (!intersect_ray_aabb(local, box, &t0, &t1)) return 0;
    if (t1 < 0.0f) return 0;
    if (tmin) *tmin = t0;
    if (tmax) *tmax = t1;
    return 1;
}
/* OBB 与球是否相交 */
static FORCE_INLINE bool_t intersect_obb_sphere(obb b, sphere s) {
    if (!obb_is_valid(b) || !sphere_is_valid(s)) return 0;
    return (obb_distance_sq(b, s.center) <= s.radius * s.radius) ? 1 : 0;
}
/* OBB 与 OBB 是否相交(15 轴分离轴测试) */
static FORCE_INLINE bool_t intersect_obb_obb(obb a, obb b) {
    vec3 A[3], B[3], t;
    real_t ra, rb, proj;
    qty_t i, j, k;
    if (!obb_is_valid(a) || !obb_is_valid(b)) return 0;
    for (i = 0; i < 3; i++) { A[i] = obb_axis(a, i); B[i] = obb_axis(b, i); }
    t = vec3_sub(b.center, a.center);
    for (i = 0; i < 3; i++) {
        proj = math_abs(vec3_dot(t, A[i]));
        ra = obb_extent(a, i);
        rb = 0.0f;
        for (j = 0; j < 3; j++) rb += math_abs(vec3_dot(B[j], A[i])) * obb_extent(b, j);
        if (proj > ra + rb + MATH_EPSILON) return 0;
    }
    for (i = 0; i < 3; i++) {
        proj = math_abs(vec3_dot(t, B[i]));
        rb = obb_extent(b, i);
        ra = 0.0f;
        for (j = 0; j < 3; j++) ra += math_abs(vec3_dot(A[j], B[i])) * obb_extent(a, j);
        if (proj > ra + rb + MATH_EPSILON) return 0;
    }
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            vec3 axis = vec3_cross(A[i], B[j]);
            real_t len = vec3_length(axis);
            if (len < MATH_EPSILON) continue;
            axis = vec3_scale(axis, 1.0f / len);
            proj = math_abs(vec3_dot(t, axis));
            ra = 0.0f;
            rb = 0.0f;
            for (k = 0; k < 3; k++) {
                ra += math_abs(vec3_dot(A[k], axis)) * obb_extent(a, k);
                rb += math_abs(vec3_dot(B[k], axis)) * obb_extent(b, k);
            }
            if (proj > ra + rb + MATH_EPSILON) return 0;
        }
    }
    return 1;
}
/* OBB 与 AABB 是否相交 */
static FORCE_INLINE bool_t intersect_obb_aabb(obb o, aabb b) {
    return intersect_obb_obb(o, obb_from_aabb(b, mat3_identity()));
}


EXTERN_C_END
#endif