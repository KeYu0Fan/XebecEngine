#ifndef MATH_UTILS_COLOR_H
/* 颜色
 * 分量范围 [0, 1], 默认为线性空间(渲染用); 与 sRGB 的互转见 color_srgb_to_linear
 * 打包格式: RGBA8 为 0xRRGGBBAA, BGRA8 为 0xBBGGRRAA(Windows/D3D 常用)
 */
#define MATH_UTILS_COLOR_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/math/constants.h"
#include "core/math/vector/vector.h"
EXTERN_C_BEGIN


/* 颜色 */
typedef struct { real_t r, g, b, a; } color;

/* ---- 构造 ---- */
/* 由分量构造 */
static FORCE_INLINE color color_new(real_t r, real_t g, real_t b, real_t a) {
    color c; c.r = r; c.g = g; c.b = b; c.a = a; return c;
}
/* 不透明黑 */
static FORCE_INLINE color color_black(void) { return color_new(0.0f, 0.0f, 0.0f, 1.0f); }
/* 不透明白 */
static FORCE_INLINE color color_white(void) { return color_new(1.0f, 1.0f, 1.0f, 1.0f); }
/* 透明 */
static FORCE_INLINE color color_clear(void) { return color_new(0.0f, 0.0f, 0.0f, 0.0f); }

/* ---- 打包互转 ---- */
/* 0xRRGGBBAA -> color */
static FORCE_INLINE color color_rgba8(qty_t v) {
    return color_new(
        (real_t)((v >> MATH_COLOR_SHIFT_R) & MATH_COLOR_CHANNEL_MASK) / MATH_COLOR_BYTE_MAX,
        (real_t)((v >> MATH_COLOR_SHIFT_G) & MATH_COLOR_CHANNEL_MASK) / MATH_COLOR_BYTE_MAX,
        (real_t)((v >> MATH_COLOR_SHIFT_B) & MATH_COLOR_CHANNEL_MASK) / MATH_COLOR_BYTE_MAX,
        (real_t)((v >> MATH_COLOR_SHIFT_A) & MATH_COLOR_CHANNEL_MASK) / MATH_COLOR_BYTE_MAX);
}
/* color -> 0xRRGGBBAA, 分量先饱和再四舍五入 */
static FORCE_INLINE qty_t color_to_rgba8(color c) {
    qty_t r = (qty_t)math_round_int(math_saturate(c.r) * MATH_COLOR_BYTE_MAX);
    qty_t g = (qty_t)math_round_int(math_saturate(c.g) * MATH_COLOR_BYTE_MAX);
    qty_t b = (qty_t)math_round_int(math_saturate(c.b) * MATH_COLOR_BYTE_MAX);
    qty_t a = (qty_t)math_round_int(math_saturate(c.a) * MATH_COLOR_BYTE_MAX);
    return (r << MATH_COLOR_SHIFT_R) | (g << MATH_COLOR_SHIFT_G) | (b << MATH_COLOR_SHIFT_B) | (a << MATH_COLOR_SHIFT_A);
}
/* 0xBBGGRRAA -> color */
static FORCE_INLINE color color_bgra8(qty_t v) {
    return color_new(
        (real_t)((v >> MATH_COLOR_SHIFT_B) & MATH_COLOR_CHANNEL_MASK) / MATH_COLOR_BYTE_MAX,
        (real_t)((v >> MATH_COLOR_SHIFT_G) & MATH_COLOR_CHANNEL_MASK) / MATH_COLOR_BYTE_MAX,
        (real_t)((v >> MATH_COLOR_SHIFT_R) & MATH_COLOR_CHANNEL_MASK) / MATH_COLOR_BYTE_MAX,
        (real_t)((v >> MATH_COLOR_SHIFT_A) & MATH_COLOR_CHANNEL_MASK) / MATH_COLOR_BYTE_MAX);
}
/* color -> 0xBBGGRRAA */
static FORCE_INLINE qty_t color_to_bgra8(color c) {
    qty_t r = (qty_t)math_round_int(math_saturate(c.r) * MATH_COLOR_BYTE_MAX);
    qty_t g = (qty_t)math_round_int(math_saturate(c.g) * MATH_COLOR_BYTE_MAX);
    qty_t b = (qty_t)math_round_int(math_saturate(c.b) * MATH_COLOR_BYTE_MAX);
    qty_t a = (qty_t)math_round_int(math_saturate(c.a) * MATH_COLOR_BYTE_MAX);
    return (b << MATH_COLOR_SHIFT_R) | (g << MATH_COLOR_SHIFT_G) | (r << MATH_COLOR_SHIFT_B) | (a << MATH_COLOR_SHIFT_A);
}

/* ---- HSV 互转 ---- */
/* HSV -> color, h ∈ [0,1) 表示色环, s/v ∈ [0,1]; a 直接透传 */
static FORCE_INLINE color color_from_hsv(real_t h, real_t s, real_t v, real_t a) {
    real_t hh = math_fmod(h, 1.0f);
    if (hh < 0.0f) hh += 1.0f;
    hh *= MATH_HSV_SECTORS;
    qty_t i = (qty_t)hh;
    real_t f = hh - (real_t)i;
    real_t p = v * (1.0f - s), q = v * (1.0f - s * f), t = v * (1.0f - s * (1.0f - f));
    switch (i % MATH_HSV_SECTOR_COUNT) {
    case 0: return color_new(v, t, p, a);
    case 1: return color_new(q, v, p, a);
    case 2: return color_new(p, v, t, a);
    case 3: return color_new(p, q, v, a);
    case 4: return color_new(t, p, v, a);
    default: return color_new(v, p, q, a);
    }
}
/* color -> HSV; 指针可为 0; 黑/灰时 h = 0 */
static FORCE_INLINE void color_to_hsv(color c, real_t* h, real_t* s, real_t* v) {
    real_t mx = math_max(c.r, math_max(c.g, c.b));
    real_t mn = math_min(c.r, math_min(c.g, c.b));
    real_t delta = mx - mn;
    real_t hh = 0.0f, ss = 0.0f;
    if (delta > MATH_EPSILON) {
        if (mx == c.r) hh = math_fmod((c.g - c.b) / delta, MATH_HSV_SECTORS);
        else if (mx == c.g) hh = (c.b - c.r) / delta + MATH_HSV_SECTOR_OFFSET_G;
        else hh = (c.r - c.g) / delta + MATH_HSV_SECTOR_OFFSET_B;
        hh /= MATH_HSV_SECTORS;
        if (hh < 0.0f) hh += 1.0f;
        ss = delta / mx;
    }
    if (h) *h = hh;
    if (s) *s = ss;
    if (v) *v = mx;
}

/* ---- 运算 ---- */
/* 逐分量乘法(颜色调制) */
static FORCE_INLINE color color_mul(color a, color b) {
    return color_new(a.r * b.r, a.g * b.g, a.b * b.b, a.a * b.a);
}
/* 逐分量加法 */
static FORCE_INLINE color color_add(color a, color b) {
    return color_new(a.r + b.r, a.g + b.g, a.b + b.b, a.a + b.a);
}
/* 标量乘 */
static FORCE_INLINE color color_scale(color c, real_t s) {
    return color_new(c.r * s, c.g * s, c.b * s, c.a * s);
}
/* 线性插值 */
static FORCE_INLINE color color_lerp(color a, color b, real_t t) {
    return color_new(math_lerp(a.r, b.r, t), math_lerp(a.g, b.g, t),
        math_lerp(a.b, b.b, t), math_lerp(a.a, b.a, t));
}
/* 预乘 alpha (rgb *= a) */
static FORCE_INLINE color color_premultiply(color c) {
    return color_new(c.r * c.a, c.g * c.a, c.b * c.a, c.a);
}
/* 亮度 (Rec.709) */
static FORCE_INLINE real_t color_luminance(color c) {
    return MATH_LUMA_R * c.r + MATH_LUMA_G * c.g + MATH_LUMA_B * c.b;
}
/* 安全比较 */
static FORCE_INLINE bool_t color_is_equal(color a, color b) {
    return (math_is_equal(a.r, b.r) && math_is_equal(a.g, b.g) &&
        math_is_equal(a.b, b.b) && math_is_equal(a.a, b.a)) ? 1 : 0;
}

/* ---- 色彩空间 ---- */
/* 单个 sRGB 分量转线性 */
static FORCE_INLINE real_t color_srgb_channel_to_linear(real_t c) {
    return (c <= MATH_SRGB_KNEE) ? c / MATH_SRGB_SLOPE
        : math_pow((c + MATH_SRGB_OFFSET) / MATH_SRGB_SCALE, MATH_SRGB_GAMMA);
}
/* 单个线性分量转 sRGB */
static FORCE_INLINE real_t color_linear_channel_to_srgb(real_t c) {
    return (c <= MATH_SRGB_KNEE_INV) ? c * MATH_SRGB_SLOPE
        : MATH_SRGB_SCALE * math_pow(c, 1.0f / MATH_SRGB_GAMMA) - MATH_SRGB_OFFSET;
}
/* sRGB -> 线性, alpha 不参与 */
static FORCE_INLINE color color_srgb_to_linear(color c) {
    return color_new(color_srgb_channel_to_linear(c.r), color_srgb_channel_to_linear(c.g),
        color_srgb_channel_to_linear(c.b), c.a);
}
/* 线性 -> sRGB, alpha 不参与 */
static FORCE_INLINE color color_linear_to_srgb(color c) {
    return color_new(color_linear_channel_to_srgb(c.r), color_linear_channel_to_srgb(c.g),
        color_linear_channel_to_srgb(c.b), c.a);
}

/* ---- 向量互转 ---- */
/* color -> vec4 */
static FORCE_INLINE vec4 color_to_vec4(color c) { return vec4_new(c.r, c.g, c.b, c.a); }
/* vec4 -> color */
static FORCE_INLINE color color_from_vec4(vec4 v) { return color_new(v.x, v.y, v.z, v.w); }

/* ---- 十六进制 ---- */
/* 十六进制字符 -> 数值; 非法字符返回 -1 */
static FORCE_INLINE sqty_t color_hex_digit(char c) {
    if (c >= '0' && c <= '9') return (sqty_t)(c - '0');
    if (c >= 'a' && c <= 'f') return (sqty_t)(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return (sqty_t)(c - 'A' + 10);
    return -1;
}
/* 解析 #RGB / #RGBA / #RRGGBB / #RRGGBBAA(可省略 #); 成功返回 1 */
static FORCE_INLINE bool_t color_from_hex(const char* text, color* out) {
    qty_t vals[MATH_COLOR_HEX_MAX_DIGITS];
    qty_t count = 0;
    if (text == 0 || out == 0) return 0;
    if (*text == '#') text++;
    while (text[count] != '\0' && count < MATH_COLOR_HEX_MAX_DIGITS) {
        sqty_t d = color_hex_digit(text[count]);
        if (d < 0) return 0;
        vals[count] = (qty_t)d;
        count++;
    }
    if (count != 3 && count != 4 && count != 6 && count != 8) return 0;
    if (count == 3 || count == 4) {
        /* #RGB/#RGBA: 每位重复一次, 即 nibble * 0x11 */
        qty_t scale = (1u << MATH_COLOR_NIBBLE_BITS) + 1u;
        out->r = (real_t)(vals[0] * scale) / MATH_COLOR_BYTE_MAX;
        out->g = (real_t)(vals[1] * scale) / MATH_COLOR_BYTE_MAX;
        out->b = (real_t)(vals[2] * scale) / MATH_COLOR_BYTE_MAX;
        out->a = (count == 4) ? (real_t)(vals[3] * scale) / MATH_COLOR_BYTE_MAX : 1.0f;
    }
    else {
        qty_t scale = (1u << MATH_COLOR_NIBBLE_BITS);
        out->r = (real_t)(vals[0] * scale + vals[1]) / MATH_COLOR_BYTE_MAX;
        out->g = (real_t)(vals[2] * scale + vals[3]) / MATH_COLOR_BYTE_MAX;
        out->b = (real_t)(vals[4] * scale + vals[5]) / MATH_COLOR_BYTE_MAX;
        out->a = (count == 8) ? (real_t)(vals[6] * scale + vals[7]) / MATH_COLOR_BYTE_MAX : 1.0f;
    }
    return 1;
}

/* 颜色格式化为 "#RRGGBBAA"; 缓冲区不足返回 0 */
static FORCE_INLINE bool_t color_to_hex(color c, char* text, qty_t size) {
    static const char digits[] = "0123456789ABCDEF";
    qty_t v, i;
    const qty_t mask = (1u << MATH_COLOR_NIBBLE_BITS) - 1u;
    if (text == 0 || size < MATH_COLOR_HEX_STRING_SIZE) return 0;
    v = color_to_rgba8(c);
    text[0] = '#';
    for (i = 0; i < MATH_COLOR_HEX_MAX_DIGITS; i++) {
        qty_t shift = (MATH_COLOR_HEX_MAX_DIGITS - 1u - i) * MATH_COLOR_NIBBLE_BITS;
        text[1 + i] = digits[(v >> shift) & mask];
    }
    text[MATH_COLOR_HEX_MAX_DIGITS + 1] = '\0';
    return 1;
}


EXTERN_C_END
#endif