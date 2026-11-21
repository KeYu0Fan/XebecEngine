#include <stdio.h>
#include <math.h>
#include <xmmintrin.h>
#include "core/math/math.h"

int main(void)
{
    frustum f = frustum_from_matrix(mat4_mul(mat4_perspective(1.0f, 1.0f, 0.1f, 100.0f),
        mat4_look_at(vec3_new(0, 0, -10), vec3_zero(), vec3_new(0, 1, 0))));
    plane p = plane_from_point_normal(vec3_new(0, 0, 0), vec3_new(0, 1, 0));
    int fails = 0;
    int i;

    printf("[size] plane=%d frustum=%d (expect 16 / %d)\n", (int)sizeof(plane), (int)sizeof(frustum), (int)(sizeof(plane) * MATH_FRUSTUM_PLANE_COUNT));
    if (sizeof(plane) != 16 || sizeof(frustum) != sizeof(plane) * MATH_FRUSTUM_PLANE_COUNT) fails++;

    /* 成员填充: 平面间步长必须 16, 与 vec3 用 w 补齐到 4 个 real_t 同理 */
    {
        long stride = (long)((const char*)&f.planes[1] - (const char*)&f.planes[0]);
        long plane_stride = (long)((const char*)&p.n.y - (const char*)&p.n.x);
        printf("[stride] plane[1]-plane[0]=%ld  n.y-n.x=%ld\n", stride, plane_stride);
        if (stride != 16 || plane_stride != 4) fails++;
    }

    /* 4 字节自然对齐即可用非对齐 SIMD 加载; 结构体本身不依赖编译器对齐属性 */
    for (i = 0; i < MATH_FRUSTUM_PLANE_COUNT; i++) {
        __m128 v = _mm_loadu_ps(&f.planes[i].n.x);
        float out[4];
        _mm_storeu_ps(out, v);
        if (fabsf(out[0] * out[0] + out[1] * out[1] + out[2] * out[2] - 1.0f) > 1e-4f) fails++;
    }
    printf("[simd] loadu %d planes ok, last=(%g,%g,%g,%g)\n", (int)MATH_FRUSTUM_PLANE_COUNT,
        f.planes[MATH_FRUSTUM_BOTTOM].n.x, f.planes[MATH_FRUSTUM_BOTTOM].n.y, f.planes[MATH_FRUSTUM_BOTTOM].n.z, f.planes[MATH_FRUSTUM_BOTTOM].n.w);

    /* 四分量点积与语义 API 一致 */
    {
        float d1 = plane_distance_to_point(p, vec3_new(0, 3, 0));
        float d2 = p.n.x * 0.0f + p.n.y * 3.0f + p.n.z * 0.0f + p.n.w;
        printf("[plane] distance=%g dot4=%g\n", d1, d2);
        if (fabsf(d1 - d2) > 1e-6f || fabsf(d1 - 3.0f) > 1e-6f) fails++;
    }

    /* 类型宽度 */
    printf("[type] s8=%d u8=%d s16=%d u16=%d s32=%d u32=%d s64=%d u64=%d f32=%d f64=%d\n",
        (int)sizeof(s8_t), (int)sizeof(u8_t), (int)sizeof(s16_t), (int)sizeof(u16_t),
        (int)sizeof(s32_t), (int)sizeof(u32_t), (int)sizeof(s64_t), (int)sizeof(u64_t),
        (int)sizeof(f32_t), (int)sizeof(f64_t));
    if (sizeof(s8_t) != 1 || sizeof(u16_t) != 2 || sizeof(s32_t) != 4 || sizeof(u64_t) != 8) fails++;
    if (sizeof(half_t) != 2 || sizeof(fixed_t) != 4) fails++;

    printf("\n%s (%d failures)\n", fails ? "FAIL" : "ALL PASS", fails);
    return fails ? 1 : 0;
}