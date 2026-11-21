#include "priv.h"
#if COM_USE(CONFIG_SYSTEM_PRIV)


#include "core/system/STATIC/base.h"

/* 启用指定的特权 */
PORT_API bool_t system_priv_open(enum system_priv_t priv)
{
#if PLATFORM_WINDOWS
    void* tmp = 0;
    switch (priv)
    {
    case SYSTEM_PRIV_SYMLINK: tmp = (void*)SE_CREATE_SYMBOLIC_LINK_NAME; break;
    case SYSTEM_PRIV_NET_BIND: tmp = (void*)SE_TCB_NAME; break;
    case SYSTEM_PRIV_PRIORITY: tmp = (void*)SE_INC_BASE_PRIORITY_NAME; break;
    case SYSTEM_PRIV_DEBUG: tmp = (void*)SE_DEBUG_NAME; break;
    case SYSTEM_PRIV_LOAD_DRIVER: tmp = (void*)SE_LOAD_DRIVER_NAME; break;
    case SYSTEM_PRIV_BACKUP: tmp = (void*)SE_BACKUP_NAME; break;
    case SYSTEM_PRIV_RESTORE: tmp = (void*)SE_RESTORE_NAME; break;
    case SYSTEM_PRIV_SHUTDOWN: tmp = (void*)SE_SHUTDOWN_NAME; break;
    case SYSTEM_PRIV_MEMLOCK: tmp = (void*)SE_LOCK_MEMORY_NAME; break;
    case SYSTEM_PRIV_VOLUME: tmp = (void*)SE_MANAGE_VOLUME_NAME; break;
    case SYSTEM_PRIV_NET_RAW:
    {
        HANDLE hToken = NULL;
        BOOL isAdmin = FALSE;
        /* 获取当前进程的访问令牌 */
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) == 0)
            return FALSE;
        /* 获取内置管理员组的SID */
        BYTE adminSID[SECURITY_MAX_SID_SIZE] = { 0 };
        DWORD sidSize = sizeof(adminSID);
        if (CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, adminSID, &sidSize) == 0)
        {
            CloseHandle(hToken);
            return FALSE;
        }
        /* 检查令牌是否属于该组 */
        if (CheckTokenMembership(hToken, adminSID, &isAdmin) == 0)
            isAdmin = FALSE;
        /* 清理资源 */
        CloseHandle(hToken);
        return isAdmin;
    }
    case SYSTEM_PRIV_TAKE_OWNERSHIP: tmp = (void*)SE_TAKE_OWNERSHIP_NAME; break;
    case SYSTEM_PRIV_FIRMWARE: tmp = (void*)SE_SYSTEM_ENVIRONMENT_NAME; break;
    default:
    case SYSTEM_PRIV_ALL:
    case SYSTEM_PRIV_KNOWN:
        return 0;
    }
    if (tmp == 0)return 0;
    HANDLE token;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token) == 0)
        return 0;
    LUID luid = { 0 };
    if (LookupPrivilegeValueW(NULL, (LPCWSTR)tmp, &luid) == 0)
    {
        CloseHandle(token);
        return 0;
    }
    TOKEN_PRIVILEGES tp = { 1, {{luid, SE_PRIVILEGE_ENABLED}} };
    if (AdjustTokenPrivileges(token, FALSE, &tp, sizeof(tp), NULL, NULL) == 0)
    {
        CloseHandle(token);
        return 0;
    }
    CloseHandle(token);
    return 1;
#elif PLATFORM_POSIX
    cap_value_t cap[sizeof(void*)] = { 0 };
    qty_t cap_num = 1;
    switch (priv)
    {
    case SYSTEM_PRIV_SYMLINK: return 1;
    case SYSTEM_PRIV_NET_BIND:cap[0] = CAP_NET_BIND_SERVICE; break;
    case SYSTEM_PRIV_PRIORITY: cap[0] = CAP_SYS_NICE; break;
    case SYSTEM_PRIV_DEBUG: cap[0] = CAP_SYS_PTRACE; break;
    case SYSTEM_PRIV_LOAD_DRIVER: cap[0] = CAP_SYS_MODULE; break;
    case SYSTEM_PRIV_BACKUP:
    case SYSTEM_PRIV_RESTORE:
    {
        cap[0] = CAP_DAC_OVERRIDE;
        cap[1] = CAP_DAC_READ_SEARCH;
        cap_num = 2;
        break;
    }
    case SYSTEM_PRIV_SHUTDOWN: cap[0] = CAP_SYS_BOOT; break;
    case SYSTEM_PRIV_MEMLOCK: cap[0] = CAP_IPC_LOCK; break;
    case SYSTEM_PRIV_VOLUME:
    {
        cap[0] = CAP_SYS_ADMIN;
        cap[1] = CAP_SYS_RAWIO;
        cap_num = 2;
        break;
    }
    case SYSTEM_PRIV_NET_RAW: cap[0] = CAP_NET_RAW; break;
    case SYSTEM_PRIV_TAKE_OWNERSHIP:
    {
        cap[0] = CAP_CHOWN;
        cap[1] = CAP_FOWNER;
        cap_num = 2;
        break;
    }
    case SYSTEM_PRIV_FIRMWARE: cap[0] = CAP_SYS_ADMIN; break;
    default:
    case SYSTEM_PRIV_ALL:
    case SYSTEM_PRIV_KNOWN:
        return 0;
    }
    cap_t caps = cap_get_proc();
    if (caps == NULL) return 0;
    cap_flag_value_t flag = CAP_CLEAR;
    qty_t tmp = 0;
    for (;tmp < cap_num;tmp++)
    {
        cap_get_flag(caps, cap[tmp], CAP_PERMITTED, &flag);
        if (flag != CAP_SET)
        {
            cap_free(caps);
            return 0;
        }
        cap_set_flag(caps, CAP_EFFECTIVE, 1, &cap[tmp], CAP_SET);
    }
    bool_t ret = (bool_t)cap_set_proc(caps);
    cap_free(caps);
    return (ret == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 禁用指定的特权 */
PORT_API bool_t system_priv_close(enum system_priv_t priv)
{
#if PLATFORM_WINDOWS
    void* tmp = 0;
    switch (priv)
    {
    case SYSTEM_PRIV_SYMLINK: tmp = (void*)SE_CREATE_SYMBOLIC_LINK_NAME; break;
    case SYSTEM_PRIV_NET_BIND: tmp = (void*)SE_TCB_NAME; break;
    case SYSTEM_PRIV_PRIORITY: tmp = (void*)SE_INC_BASE_PRIORITY_NAME; break;
    case SYSTEM_PRIV_DEBUG: tmp = (void*)SE_DEBUG_NAME; break;
    case SYSTEM_PRIV_LOAD_DRIVER: tmp = (void*)SE_LOAD_DRIVER_NAME; break;
    case SYSTEM_PRIV_BACKUP: tmp = (void*)SE_BACKUP_NAME; break;
    case SYSTEM_PRIV_RESTORE: tmp = (void*)SE_RESTORE_NAME; break;
    case SYSTEM_PRIV_SHUTDOWN: tmp = (void*)SE_SHUTDOWN_NAME; break;
    case SYSTEM_PRIV_MEMLOCK: tmp = (void*)SE_LOCK_MEMORY_NAME; break;
    case SYSTEM_PRIV_VOLUME: tmp = (void*)SE_MANAGE_VOLUME_NAME; break;
    case SYSTEM_PRIV_NET_RAW: return 1;
    case SYSTEM_PRIV_TAKE_OWNERSHIP: tmp = (void*)SE_TAKE_OWNERSHIP_NAME; break;
    case SYSTEM_PRIV_FIRMWARE: tmp = (void*)SE_SYSTEM_ENVIRONMENT_NAME; break;
    case SYSTEM_PRIV_ALL:
    {
        HANDLE token;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token) == 0)
            return 0;
        TOKEN_PRIVILEGES tp = { 0 };
        bool_t ok = (bool_t)AdjustTokenPrivileges(token, TRUE, &tp, 0, NULL, NULL);
        CloseHandle(token);
        return ok;
    }
    case SYSTEM_PRIV_KNOWN:
    {
        enum system_priv_t min = SYSTEM_PRIV_NA_MIN + 1;
        enum system_priv_t max = SYSTEM_PRIV_NA_MAX;
        bool_t ret = 1;
        for (;min < max;min++)
        {
            if (min == SYSTEM_PRIV_ALL || min == SYSTEM_PRIV_KNOWN)
                continue;
            ret = ret && system_priv_close(min);
        }
        return ret;
    }
    default:
        return 0;
    }
    HANDLE token;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token) == 0)
        return 0;
    LUID luid;
    if (LookupPrivilegeValueW(NULL, (LPCWSTR)tmp, &luid) == 0)
    {
        CloseHandle(token);
        return 0;
    }
    TOKEN_PRIVILEGES tp = { 1, {{luid, 0}} };
    if (AdjustTokenPrivileges(token, FALSE, &tp, sizeof(tp), NULL, NULL) == 0)
    {
        CloseHandle(token);
        return 0;
    }
    CloseHandle(token);
    return 1;
#elif PLATFORM_POSIX
    cap_value_t cap[sizeof(void*)] = { 0 };
    qty_t cap_num = 1;
    switch (priv)
    {
    case SYSTEM_PRIV_SYMLINK: return 1;
    case SYSTEM_PRIV_NET_BIND:cap[0] = CAP_NET_BIND_SERVICE; break;
    case SYSTEM_PRIV_PRIORITY: cap[0] = CAP_SYS_NICE; break;
    case SYSTEM_PRIV_DEBUG: cap[0] = CAP_SYS_PTRACE; break;
    case SYSTEM_PRIV_LOAD_DRIVER: cap[0] = CAP_SYS_MODULE; break;
    case SYSTEM_PRIV_BACKUP:
    case SYSTEM_PRIV_RESTORE:
    {
        cap[0] = CAP_DAC_OVERRIDE;
        cap[1] = CAP_DAC_READ_SEARCH;
        cap_num = 2;
        break;
    }
    case SYSTEM_PRIV_SHUTDOWN: cap[0] = CAP_SYS_BOOT; break;
    case SYSTEM_PRIV_MEMLOCK: cap[0] = CAP_IPC_LOCK; break;
    case SYSTEM_PRIV_VOLUME:
    {
        cap[0] = CAP_SYS_ADMIN;
        cap[1] = CAP_SYS_RAWIO;
        cap_num = 2;
        break;
    }
    case SYSTEM_PRIV_NET_RAW: cap[0] = CAP_NET_RAW; break;
    case SYSTEM_PRIV_TAKE_OWNERSHIP:
    {
        cap[0] = CAP_CHOWN;
        cap[1] = CAP_FOWNER;
        cap_num = 2;
        break;
    }
    case SYSTEM_PRIV_FIRMWARE: cap[0] = CAP_SYS_ADMIN; break;
    case SYSTEM_PRIV_ALL:
    {
        cap_t caps = cap_get_proc();
        if (caps == NULL) return 0;
        cap_clear_flag(caps, CAP_EFFECTIVE);
        bool_t tmp = (bool_t)cap_set_proc(caps);
        cap_free(caps);
        return (tmp == 0) ? 1 : 0;
    }
    case SYSTEM_PRIV_KNOWN:
    {
        enum system_priv_t min = SYSTEM_PRIV_NA_MIN + 1;
        enum system_priv_t max = SYSTEM_PRIV_NA_MAX;
        bool_t ret = 1;
        for (;min < max;min++)
        {
            if (min == SYSTEM_PRIV_ALL || min == SYSTEM_PRIV_KNOWN || min == SYSTEM_PRIV_BACKUP)continue;
            ret = ret && system_priv_close(min);
        }
        return ret;
    }
    default:
        return 0;
    }
    cap_t caps = cap_get_proc();
    cap_flag_value_t flag = CAP_CLEAR;
    qty_t tmp = 0;
    for (;tmp < cap_num;tmp++)
        cap_set_flag(caps, CAP_EFFECTIVE, 1, &cap[tmp], CAP_CLEAR);
    tmp = (qty_t)cap_set_proc(caps);
    cap_free(caps);
    return (tmp == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 检查当前进程是否拥有指定的特权 */
PORT_API bool_t system_priv_has(enum system_priv_t priv)
{
#if PLATFORM_WINDOWS
    void* tmp = 0;
    switch (priv)
    {
    case SYSTEM_PRIV_SYMLINK: tmp = (void*)SE_CREATE_SYMBOLIC_LINK_NAME; break;
    case SYSTEM_PRIV_NET_BIND: tmp = (void*)SE_TCB_NAME; break;
    case SYSTEM_PRIV_PRIORITY: tmp = (void*)SE_INC_BASE_PRIORITY_NAME; break;
    case SYSTEM_PRIV_DEBUG: tmp = (void*)SE_DEBUG_NAME; break;
    case SYSTEM_PRIV_LOAD_DRIVER: tmp = (void*)SE_LOAD_DRIVER_NAME; break;
    case SYSTEM_PRIV_BACKUP: tmp = (void*)SE_BACKUP_NAME; break;
    case SYSTEM_PRIV_RESTORE: tmp = (void*)SE_RESTORE_NAME; break;
    case SYSTEM_PRIV_SHUTDOWN: tmp = (void*)SE_SHUTDOWN_NAME; break;
    case SYSTEM_PRIV_MEMLOCK: tmp = (void*)SE_LOCK_MEMORY_NAME; break;
    case SYSTEM_PRIV_VOLUME: tmp = (void*)SE_MANAGE_VOLUME_NAME; break;
    case SYSTEM_PRIV_NET_RAW:
    {
        HANDLE hToken = NULL;
        BOOL isAdmin = FALSE;
        /* 获取当前进程的访问令牌 */
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) == 0)
            return FALSE;
        /* 获取内置管理员组的SID */
        BYTE adminSID[SECURITY_MAX_SID_SIZE] = { 0 };
        DWORD sidSize = sizeof(adminSID);
        if (CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, adminSID, &sidSize) == 0)
        {
            CloseHandle(hToken);
            return FALSE;
        }
        /* 检查令牌是否属于该组 */
        if (CheckTokenMembership(hToken, adminSID, &isAdmin) == 0)
            isAdmin = FALSE;
        /* 清理资源 */
        CloseHandle(hToken);
        return isAdmin;
    }
    case SYSTEM_PRIV_TAKE_OWNERSHIP: tmp = (void*)SE_TAKE_OWNERSHIP_NAME; break;
    case SYSTEM_PRIV_FIRMWARE: tmp = (void*)SE_SYSTEM_ENVIRONMENT_NAME; break;
    default:
    case SYSTEM_PRIV_ALL:
    case SYSTEM_PRIV_KNOWN:
        return 0;
    }
    HANDLE token = 0;
    LUID luid = { 0 };
    PRIVILEGE_SET ps = { 1, PRIVILEGE_SET_ALL_NECESSARY, {{luid}} };
    BOOL result = 0;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
        if (LookupPrivilegeValueW(NULL, (LPCWSTR)tmp, &luid))
            PrivilegeCheck(token, &ps, &result);
    CloseHandle(token);
    return result ? 1 : 0;
#elif PLATFORM_POSIX
    cap_value_t cap[sizeof(void*)] = { 0 };
    qty_t cap_num = 1;
    switch (priv)
    {
    case SYSTEM_PRIV_SYMLINK: return 1;
    case SYSTEM_PRIV_NET_BIND:cap[0] = CAP_NET_BIND_SERVICE; break;
    case SYSTEM_PRIV_PRIORITY: cap[0] = CAP_SYS_NICE; break;
    case SYSTEM_PRIV_DEBUG: cap[0] = CAP_SYS_PTRACE; break;
    case SYSTEM_PRIV_LOAD_DRIVER: cap[0] = CAP_SYS_MODULE; break;
    case SYSTEM_PRIV_BACKUP:
    case SYSTEM_PRIV_RESTORE:
    {
        cap[0] = CAP_DAC_OVERRIDE;
        cap[1] = CAP_DAC_READ_SEARCH;
        cap_num = 2;
        break;
    }
    case SYSTEM_PRIV_SHUTDOWN: cap[0] = CAP_SYS_BOOT; break;
    case SYSTEM_PRIV_MEMLOCK: cap[0] = CAP_IPC_LOCK; break;
    case SYSTEM_PRIV_VOLUME:
    {
        cap[0] = CAP_SYS_ADMIN;
        cap[1] = CAP_SYS_RAWIO;
        cap_num = 2;
        break;
    }
    case SYSTEM_PRIV_NET_RAW: cap[0] = CAP_NET_RAW; break;
    case SYSTEM_PRIV_TAKE_OWNERSHIP:
    {
        cap[0] = CAP_CHOWN;
        cap[1] = CAP_FOWNER;
        cap_num = 2;
        break;
    }
    case SYSTEM_PRIV_FIRMWARE: cap[0] = CAP_SYS_ADMIN; break;
    default:
    case SYSTEM_PRIV_ALL:
    case SYSTEM_PRIV_KNOWN:
        return 0;
    }
    cap_t caps = cap_get_proc();
    if (caps == NULL) return 0;
    cap_flag_value_t flag = CAP_CLEAR;
    qty_t tmp = 0;
    for (;tmp < cap_num;tmp++)
    {
        cap_get_flag(caps, cap[tmp], CAP_EFFECTIVE, &flag);
        if (flag == CAP_CLEAR)break;
    }
    cap_free(caps);
    return (flag == CAP_SET) ? 1 : 0;
#else
    return 0;
#endif
}


#else


PORT_API bool_t system_priv_open(enum system_priv_t priv) { ; }
PORT_API bool_t system_priv_close(enum system_priv_t priv) { ; }
PORT_API bool_t system_priv_has(enum system_priv_t priv) { ; }


#endif