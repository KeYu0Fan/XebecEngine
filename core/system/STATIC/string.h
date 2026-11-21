#ifndef SYSTEM_STRING_H
#define SYSTEM_STRING_H
#include "core/system/STATIC/base.h"
EXTERN_C_BEGIN


/* 将字符串转换为本地平台接口可识别字符串 */
PORT_HIDE static bool_t system_string_to_native(void* dst, const char* src, qty_t dst_size);
/* 将本地平台接口字符串转换为 UTF-8 */
PORT_HIDE static bool_t system_string_from_native(char* dst, const void* src, qty_t dst_size);
/* 返回字符串长度 */
PORT_HIDE static qty_t system_string_len(const char* i);
/* 复制字符串,返回是否成功 */
PORT_HIDE static bool_t system_string_cpy(char* dst, const char* src, qty_t dst_size);

/* 将字符串转换为本地平台接口可识别字符串 */
PORT_HIDE static bool_t system_string_to_native(void* dst, const char* src, qty_t dst_size)
{
    if (src == 0 || dst == 0 || dst_size == 0) return 0;
#if PLATFORM_WINDOWS
    /* 计算转换所需的宽字符数(含终止符) */
    int wchars_needed = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, src, -1, NULL, 0);
    if (wchars_needed == 0) return 0;
    /* 计算所需字节数(每个 wchar_t 占 sizeof(wchar_t) 字节) */
    if (wchars_needed < 0 || (size_t)wchars_needed >(size_t)dst_size / sizeof(wchar_t))
        return 0;
    /* 执行转换, 将结果写入 dst(将其强制视为 wchar_t*) */
    int result = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, src, -1, (wchar_t*)dst, wchars_needed);
    if (result == 0) return 0;
    return 1;
#elif PLATFORM_POSIX
    const char* start = src;
    while (*src) src++;
    qty_t src_len = (qty_t)(src - start);
    if (src_len + 1 > dst_size) return 0;
    system_string_cpy(dst, start, src_len + 1);
    return 1;
#else   
    return 0;
#endif
}
/* 将本地平台接口字符串转换为 UTF-8 */
PORT_HIDE static bool_t system_string_from_native(char* dst, const void* src, qty_t dst_size)
{
    if (src == 0 || dst == 0 || dst_size == 0) return 0;
#if PLATFORM_WINDOWS
    const wchar_t* wstr = (const wchar_t*)src;
    int needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    if (needed == 0) return 0;
    if ((qty_t)needed > dst_size) return 0;
    int result = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, dst, needed, NULL, NULL);
    if (result == 0) return 0;
    return 1;
#elif PLATFORM_POSIX
    const char* cstr = src;
    qty_t len = 0;
    while (cstr[len]) len++;
    if (len + 1 > dst_size) return 0;
    for (qty_t i = 0; i <= len; i++)
    {
        dst[i] = cstr[i];
    }
    return 1;
#else
    return 0;
#endif
}
/* 返回字符串长度 */
PORT_HIDE static qty_t system_string_len(const char* i)
{
    qty_t tmp = (qty_t)i;
    while (*i)i++;
    return (qty_t)i - tmp;
}
/* 复制字符串,返回是否成功 */
PORT_HIDE static bool_t system_string_cpy(char* dst, const char* src, qty_t dst_size)
{
    if (dst == NULL || src == NULL || dst_size == 0) return 0;
    qty_t len = system_string_len(src);
    if (len >= dst_size) return 0;
    qty_t i = 0;
    for (; i <= len;i++)
        dst[i] = src[i];
    return 1;
}


EXTERN_C_END
#endif