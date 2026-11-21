#include "env.h"
#if COM_USE(CONFIG_SYSTEM_ENV)


#include "core/system/STATIC/base.h"
#include "core/system/thread.h"
#include "core/system/STATIC/string.h"
#include "core/system/mmap.h"

#if COM_USE(CONFIG_SYSTEM_THREAD)
PORT_HIDE static system_rwlock_t system_env_lock;
PORT_HIDE static system_once_init system_env_init = { 0 };
PORT_HIDE static bool_t system_env_var = 0;
PORT_HIDE static void system_env_aux_00(void)
{
    if (system_rwlock_init(&system_env_lock))
        system_env_var = 1;
    else
        system_env_var = 0;
}
#endif

/* 获取环境变量 */
PORT_API bool_t system_env_get(char* buf, const char* name, qty_t size)
{
    if ((buf && name && size) == 0) return 0;
#if COM_USE(CONFIG_SYSTEM_THREAD)
    if (system_env_var == 0)system_once_run(&system_env_init, &system_env_aux_00);
    if (system_env_var)
        if (system_rwlock_rdlock(&system_env_lock) == 0)return 0;
#endif
#if PLATFORM_WINDOWS

    wchar_t wname[SYSTEM_STRING_SAFE_MAX] = { 0 };
    DWORD wlen = 0;
    if (system_string_to_native(wname, name, sizeof(wname)))
        wlen = GetEnvironmentVariableW(wname, NULL, 0);
    if (wlen == 0)
    {
#if COM_USE(CONFIG_SYSTEM_THREAD)
        if (system_env_var) system_rwlock_unlock_rd(&system_env_lock);
#endif
        return 0;
    }
    /* 计算实际需要的宽字符数 含终止符 */
    qty_t wvalue_len = ((qty_t)wlen + 1) * sizeof(wchar_t);
#if COM_USE(CONFIG_SYSTEM_MMAP)
    struct system_mmap_prot_t prot = { 0 };
    struct system_mmap_flags_t flags = { 0 };
    prot.read = 1;
    prot.write = 1;
    wchar_t* wvalue = (wchar_t*)system_mmap_new(NULL, wvalue_len, prot, flags);
    if (wvalue == 0)
#else
    wchar_t wvalue[SYSTEM_EVN_MAX] = { 0 };
    if (wvalue_len > sizeof(wvalue))
#endif
    {
#if COM_USE(CONFIG_SYSTEM_THREAD)
        if (system_env_var) system_rwlock_unlock_rd(&system_env_lock);
#endif
        return 0;
    }
    qty_t needed = 0;
    qty_t tmp = 0;
    if (GetEnvironmentVariableW(wname, wvalue, wlen + 1))
        needed = (qty_t)WideCharToMultiByte(CP_UTF8, 0, wvalue, -1, NULL, 0, NULL, NULL);
    if (needed && needed <= size)
        tmp = (qty_t)(WideCharToMultiByte(CP_UTF8, 0, wvalue, -1, buf, (int)size, NULL, NULL) ? 1 : 0);
#if COM_USE(CONFIG_SYSTEM_MMAP)
    system_mmap_del(wvalue, wvalue_len);
#endif
#elif PLATFORM_POSIX
    const char* val = getenv(name);
    qty_t val_len = 0;
    qty_t tmp = 0;
    if (val)
        val_len = (qty_t)system_string_len(val) + 1;
    if (val && val_len <= size)
        tmp = (qty_t)system_string_cpy(buf, val, (size_t)size);
#else
    qty_t tmp = 0;
#endif
#if COM_USE(CONFIG_SYSTEM_THREAD)
    if (system_env_var) system_rwlock_unlock_rd(&system_env_lock);
#endif
    return (bool_t)tmp;
}
/* 设置环境变量 */
PORT_API bool_t system_env_set(const char* name, const char* value)
{
    if ((name && value) == 0) return 0;
#if COM_USE(CONFIG_SYSTEM_THREAD)
    if (system_env_var == 0)system_once_run(&system_env_init, &system_env_aux_00);
    if (system_env_var)
        if (system_rwlock_wrlock(&system_env_lock) == 0)return 0;
#endif
#if PLATFORM_WINDOWS
    wchar_t wname[SYSTEM_STRING_SAFE_MAX] = { 0 };
    qty_t wlen = (qty_t)MultiByteToWideChar(CP_UTF8, 0, value, -1, NULL, 0);
    if (wlen == 0)
    {
#if COM_USE(CONFIG_SYSTEM_THREAD)
        if (system_env_var) system_rwlock_unlock_wr(&system_env_lock);
#endif
        return 0;
    }
    qty_t value_len = wlen * sizeof(wchar_t);
#if COM_USE(CONFIG_SYSTEM_MMAP)
    struct system_mmap_prot_t tmp_prot = { 0 };
    tmp_prot.read = 1;
    tmp_prot.write = 1;
    struct system_mmap_flags_t tmp_flags = { 0 };
    wchar_t* wvalue = system_mmap_new(0, value_len, tmp_prot, tmp_flags);
    if (wvalue == 0)
#else
    wchar_t wvalue[SYSTEM_EVN_MAX] = { 0 };
    if (value_len > sizeof(wvalue))
#endif
    {
#if COM_USE(CONFIG_SYSTEM_THREAD)
        if (system_env_var) system_rwlock_unlock_wr(&system_env_lock);
#endif
        return 0;
    }
    qty_t tmp = 0;
    if (system_string_to_native(wname, name, sizeof(wname)))
        if (MultiByteToWideChar(CP_UTF8, 0, value, -1, wvalue, (int)wlen))
            tmp = (qty_t)(SetEnvironmentVariableW(wname, wvalue) ? 1 : 0);
#if COM_USE(CONFIG_SYSTEM_MMAP)
    system_mmap_del(wvalue, value_len);
#endif
#elif PLATFORM_POSIX
    qty_t tmp = (setenv(name, value, 1) == 0) ? 1 : 0;
#else
    qty_t tmp = 0;
#endif
#if COM_USE(CONFIG_SYSTEM_THREAD)
    if (system_env_var) system_rwlock_unlock_wr(&system_env_lock);
#endif
    return (bool_t)tmp;
}
/* 删除环境变量 */
PORT_API bool_t system_env_unset(const char* name)
{
    if (name == 0) return 0;
#if COM_USE(CONFIG_SYSTEM_THREAD)
    if (system_env_var == 0)system_once_run(&system_env_init, &system_env_aux_00);
    if (system_env_var)
        if (system_rwlock_wrlock(&system_env_lock) == 0)return 0;
#endif
#if PLATFORM_WINDOWS
    wchar_t wname[SYSTEM_STRING_SAFE_MAX] = { 0 };
    bool_t tmp = 0;
    if (system_string_to_native(wname, name, sizeof(wname)))
        tmp = SetEnvironmentVariableW(wname, NULL) ? 1 : 0;
#elif PLATFORM_POSIX
    qty_t tmp = (unsetenv(name) == 0) ? 1 : 0;
#else
    qty_t tmp = 0;
#endif
#if COM_USE(CONFIG_SYSTEM_THREAD)
    if (system_env_var) system_rwlock_unlock_wr(&system_env_lock);
#endif
    return (bool_t)tmp;
}


#else


PORT_API bool_t system_env_get(char* buf, const char* name, qty_t size) { ; }
PORT_API bool_t system_env_set(const char* name, const char* value) { ; }
PORT_API bool_t system_env_unset(const char* name) { ; }


#endif