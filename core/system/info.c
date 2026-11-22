#include "info.h"
#if COM_USE(CONFIG_SYSTEM_INFO)


#include "core/system/STATIC/base.h"
#include "core/system/STATIC/string.h"
#include "core/system/thread.h"

#if PLATFORM_WINDOWS
#pragma comment(lib, "pdh.lib")
#include "core/system/STATIC/string.h"
#elif PLATFORM_POSIX
/**/
#else
/**/
#endif

#if COM_USE(CONFIG_SYSTEM_THREAD)
PORT_HIDE static system_once_init system_info_var = { 0 };
#endif
PORT_HIDE static char system_info_os_name[SYSTEM_STRING_SAFE_MAX] = { 0 };
PORT_HIDE static char system_info_os_version[SYSTEM_STRING_SAFE_MAX] = { 0 };
PORT_HIDE static qty_t  system_info_os_loadavg[3] = { 0 };
PORT_HIDE static qty_t  system_info_os_init = 0;

#if PLATFORM_WINDOWS
PORT_HIDE static HQUERY      g_load_query = NULL;
PORT_HIDE static HCOUNTER    g_counter_cpu_time = NULL;
PORT_HIDE static HCOUNTER    g_counter_queue_length = NULL;
PORT_HIDE static int         g_load_initialized = 0;
PORT_HIDE static unsigned long long g_last_sample_time = 0;
/* 函数指针类型定义 */
typedef PDH_STATUS(WINAPI* PdhOpenQueryW_t)(LPCWSTR, DWORD_PTR, HQUERY*);
typedef PDH_STATUS(WINAPI* PdhAddCounterW_t)(HQUERY, LPCWSTR, DWORD_PTR, HCOUNTER*);
typedef PDH_STATUS(WINAPI* PdhCloseQuery_t)(HQUERY);
typedef PDH_STATUS(WINAPI* PdhCollectQueryData_t)(HQUERY);
typedef PDH_STATUS(WINAPI* PdhGetFormattedCounterValue_t)(HCOUNTER, DWORD, LPDWORD, PDH_FMT_COUNTERVALUE*);

static HMODULE g_hPdhDll = NULL;
static PdhOpenQueryW_t g_pPdhOpenQueryW = NULL;
static PdhAddCounterW_t g_pPdhAddCounterW = NULL;
static PdhCloseQuery_t g_pPdhCloseQuery = NULL;
static PdhCollectQueryData_t g_pPdhCollectQueryData = NULL;
static PdhGetFormattedCounterValue_t g_pPdhGetFormattedCounterValue = NULL;
PORT_HIDE static void system_info_aux_00(void)
{
    g_hPdhDll = LoadLibraryW(L"pdh.dll");
    if (g_hPdhDll == 0) return;
    g_pPdhOpenQueryW = (PdhOpenQueryW_t)GetProcAddress(g_hPdhDll, "PdhOpenQueryW");
    g_pPdhAddCounterW = (PdhAddCounterW_t)GetProcAddress(g_hPdhDll, "PdhAddCounterW");
    g_pPdhCloseQuery = (PdhCloseQuery_t)GetProcAddress(g_hPdhDll, "PdhCloseQuery");
    g_pPdhCollectQueryData = (PdhCollectQueryData_t)GetProcAddress(g_hPdhDll, "PdhCollectQueryData");
    g_pPdhGetFormattedCounterValue = (PdhGetFormattedCounterValue_t)GetProcAddress(g_hPdhDll, "PdhGetFormattedCounterValue");
    /* 替换所有直接调用为函数指针 */
    PDH_STATUS status = g_pPdhOpenQueryW(NULL, 0, &g_load_query);
    if (status != ERROR_SUCCESS) return;
    status = g_pPdhAddCounterW(g_load_query, L"\\Processor(_Total)\\% Processor Time", 0, &g_counter_cpu_time);
    if (status != ERROR_SUCCESS)
    {
        g_pPdhCloseQuery(g_load_query);
        g_load_query = NULL;
        return;
    }
    status = g_pPdhAddCounterW(g_load_query, L"\\System\\Processor Queue Length", 0, &g_counter_queue_length);
    if (status != ERROR_SUCCESS)
    {
        g_pPdhCloseQuery(g_load_query);
        g_load_query = NULL;
        return;
    }
    g_pPdhCollectQueryData(g_load_query);
    g_last_sample_time = (unsigned long long)system_info_get_uptime();
    g_load_initialized = 2;
}
PORT_HIDE static void system_info_aux_loadavg(void)
{
    if (g_load_initialized == 0)
    {
        g_load_initialized = 1;
#if COM_USE(CONFIG_SYSTEM_THREAD)
        static system_once_init tmp_loadavg = { 0 };
        system_once_run(&tmp_loadavg, system_info_aux_00);
#else
        system_info_aux_00();
#endif
    }
    if (g_load_query == NULL) return;
    /* 采集数据 */
    PDH_FMT_COUNTERVALUE cpu_val = { 0 }, queue_val = { 0 };
    if (g_pPdhCollectQueryData(g_load_query) != ERROR_SUCCESS) return;
    if (g_pPdhGetFormattedCounterValue(g_counter_cpu_time, PDH_FMT_DOUBLE, NULL, &cpu_val) != ERROR_SUCCESS ||
        cpu_val.CStatus != PDH_CSTATUS_VALID_DATA) return;
    if (g_pPdhGetFormattedCounterValue(g_counter_queue_length, PDH_FMT_LONG, NULL, &queue_val) != ERROR_SUCCESS ||
        queue_val.CStatus != PDH_CSTATUS_VALID_DATA) return;
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    /* 当前负载 */
    const qty_t SCALE = 65536;
    qty_t cores = (qty_t)si.dwNumberOfProcessors;
    qty_t cpu_part = (qty_t)(cpu_val.doubleValue * (double)cores * (double)SCALE / 100.0);
    qty_t queue_part = (qty_t)queue_val.longValue * SCALE;
    qty_t current_load = cpu_part + queue_part;
    /* 采样间隔 */
    unsigned long long now = (unsigned long long)system_info_get_uptime();
    qty_t interval = (qty_t)(now - g_last_sample_time);
    if (interval < 5) return;
    g_last_sample_time = now;
    /* 线性近似衰减因子 */
    qty_t alpha1 = (interval * SCALE) / 60;
    qty_t alpha5 = (interval * SCALE) / 300;
    qty_t alpha15 = (interval * SCALE) / 900;
    if (alpha1 > SCALE) alpha1 = SCALE;
    if (alpha5 > SCALE) alpha5 = SCALE;
    if (alpha15 > SCALE) alpha15 = SCALE;
    /* EWMA 更新 */
    system_info_os_loadavg[0] = (system_info_os_loadavg[0] * (SCALE - alpha1) + current_load * alpha1) / SCALE;
    system_info_os_loadavg[1] = (system_info_os_loadavg[1] * (SCALE - alpha5) + current_load * alpha5) / SCALE;
    system_info_os_loadavg[2] = (system_info_os_loadavg[2] * (SCALE - alpha15) + current_load * alpha15) / SCALE;
}
#endif
PORT_HIDE static void system_info_aux_osinit(void)
{
#if PLATFORM_WINDOWS
    /* 使用 RtlGetVersion 动态加载,避免依赖 DDK */
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (hNtdll)
    {
        typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
        RtlGetVersionPtr pRtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hNtdll, "RtlGetVersion");
        if (pRtlGetVersion)
        {
            RTL_OSVERSIONINFOW osvi = { sizeof(osvi) };
            if (pRtlGetVersion(&osvi) == 0)
            {
                /* 映射友好名称 */
                const char* name = "Windows";
                if (osvi.dwMajorVersion == 10 && osvi.dwMinorVersion == 0)
                {
                    if (osvi.dwBuildNumber >= 22000)
                        name = "Windows 11";
                    else
                        name = "Windows 10";
                }
                else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 3)
                    name = "Windows 8.1";
                else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2)
                    name = "Windows 8";
                else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
                    name = "Windows 7";
                else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
                    name = "Windows Vista";
                else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
                    name = "Windows Server 2003";
                else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
                    name = "Windows XP";
                system_string_cpy(system_info_os_name, name, sizeof(system_info_os_name));
                qty_t i = 0, u = 0;
                DWORD v_build = osvi.dwBuildNumber;
                DWORD v_minor = osvi.dwMinorVersion;
                DWORD v_major = osvi.dwMajorVersion;
                while (v_build) { system_info_os_version[i] = (char)('0' + (v_build % 10)); v_build /= 10; i++; }
                system_info_os_version[i] = '.';
                i++;
                while (v_minor) { system_info_os_version[i] = (char)('0' + (v_minor % 10)); v_minor /= 10; i++; }
                system_info_os_version[i] = '.';
                i++;
                while (v_major) { system_info_os_version[i] = (char)('0' + (v_major % 10)); v_major /= 10; i++; }
                i--;
                while (u < i)
                {
                    char tmp = system_info_os_version[i];
                    system_info_os_version[i] = system_info_os_version[u];
                    system_info_os_version[u] = tmp;
                    i--; u++;
                }
            }
        }
    }
    else
    {
        system_string_cpy(system_info_os_name, "Windows", SYSTEM_STRING_SAFE_MAX);
        system_string_cpy(system_info_os_version, "Unknown", SYSTEM_STRING_SAFE_MAX);
    }
#elif PLATFORM_POSIX
    struct utsname uts;
    if (uname(&uts) == 0)
    {
        system_string_cpy(system_info_os_name, uts.sysname, sizeof(system_info_os_name));
        system_string_cpy(system_info_os_version, uts.release, sizeof(system_info_os_version));
    }
    else
    {
        strcpy(system_info_os_name, "Unix");
        strcpy(system_info_os_version, "Unknown");
    }
#else
    system_string_cpy(system_info_os_name, "___", SYSTEM_STRING_SAFE_MAX);
    system_string_cpy(system_info_os_version, "___", SYSTEM_STRING_SAFE_MAX);
#endif
    system_info_os_init = 1;
}

/* 获取操作系统名称 静态存储, 无需释放 */
PORT_API const char* system_info_get_os_name(void)
{

    if (system_info_os_init == 0)
#if COM_USE(CONFIG_SYSTEM_THREAD)
        system_once_run(&system_info_var, system_info_aux_osinit);
#else
        system_info_aux_osinit();
#endif
    return system_info_os_name;
}
/* 获取操作系统版本号,静态存储,无需释放 */
PORT_API const char* system_info_get_os_version(void)
{
    if (system_info_os_init == 0)
#if COM_USE(CONFIG_SYSTEM_THREAD)
        system_once_run(&system_info_var, system_info_aux_osinit);
#else
        system_info_aux_osinit();
#endif
    return system_info_os_version;
}
/* 获取逻辑 CPU 核心数 */
PORT_API qty_t system_info_get_cpu_cores(void)
{
#if PLATFORM_WINDOWS
    SYSTEM_INFO si = { 0 };
    GetSystemInfo(&si);
    return (qty_t)si.dwNumberOfProcessors;
#elif PLATFORM_POSIX
    long cores = sysconf(_SC_NPROCESSORS_ONLN);
    return (cores > 0) ? (qty_t)cores : 0;
#else
    return 0;
#endif
}
/*
 * 获取系统累计 CPU 时间(自开机) 单位 100ms
 * user:   用户态时间
 * kernel: 内核态时间 不含idle
 * idle:   空闲时间
 * 总时间 = user + kernel + idle
 * 忙碌时间 = user + kernel
 * 使用率 = 1 - (idle_new - idle_old) / (总时间_new - 总时间_old)
 */
PORT_API bool_t system_info_get_cpu_times(qty_t* user, qty_t* kernel, qty_t* idle)
{
#if PLATFORM_WINDOWS
    FILETIME ft_idle, ft_kernel, ft_user;
    ULARGE_INTEGER u_idle = { 0 }, u_kernel = { 0 }, u_user = { 0 };
    if (GetSystemTimes(&ft_idle, &ft_kernel, &ft_user) == 0)
        return 0;
    u_idle.LowPart = ft_idle.dwLowDateTime;
    u_idle.HighPart = ft_idle.dwHighDateTime;
    u_kernel.LowPart = ft_kernel.dwLowDateTime;
    u_kernel.HighPart = ft_kernel.dwHighDateTime;
    u_user.LowPart = ft_user.dwLowDateTime;
    u_user.HighPart = ft_user.dwHighDateTime;
    /* 100ns -> 100ms : / 1000000 */
    if (user)   *user = (qty_t)(u_user.QuadPart / 1000000ULL);
    /* Windows kernel 含 idle, 需减掉 */
    if (kernel) *kernel = (qty_t)((u_kernel.QuadPart - u_idle.QuadPart) / 1000000ULL);
    if (idle)   *idle = (qty_t)(u_idle.QuadPart / 1000000ULL);
    return 1;
#elif PLATFORM_POSIX
    /* /proc/stat 第一行: cpu user nice system idle iowait irq softirq steal ... */
    unsigned long long v_user = 0, v_nice = 0, v_system = 0, v_idle = 0;
    unsigned long long v_iowait = 0, v_irq = 0, v_softirq = 0, v_steal = 0;
    long hz;
    FILE* fp = fopen("/proc/stat", "r");
    if (fp == 0)
        return 0;
    if (fscanf(fp, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
        &v_user, &v_nice, &v_system, &v_idle,
        &v_iowait, &v_irq, &v_softirq, &v_steal) < 4)
    {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    hz = sysconf(_SC_CLK_TCK);
    if (hz <= 0)
        hz = 100;
    /* jiffies -> 100ms : * 10 / hz */
    if (user)   *user = (qty_t)((v_user + v_nice) * 10ULL / (unsigned long long)hz);
    if (kernel) *kernel = (qty_t)((v_system + v_irq + v_softirq) * 10ULL / (unsigned long long)hz);
    if (idle)   *idle = (qty_t)((v_idle + v_iowait) * 10ULL / (unsigned long long)hz);
    return 1;
#else
    return 0;
#endif
}
/* 获取物理内存总量 字节 */
PORT_API qty_t system_info_get_total_ram(void)
{
#if PLATFORM_WINDOWS
    MEMORYSTATUSEX mem = { 0 };
    mem.dwLength = sizeof(mem);
    if (GlobalMemoryStatusEx(&mem))
        return (qty_t)mem.ullTotalPhys;
    return 0;
#elif PLATFORM_POSIX
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE);
    if (pages > 0 && page_size > 0)
        return (qty_t)((unsigned long long)pages * (unsigned long long)page_size);
    return 0;
#else
    return 0;
#endif
}
/* 获取空闲物理内存总量 字节 */
PORT_API qty_t system_info_get_free_ram(void)
{
#if PLATFORM_WINDOWS
    MEMORYSTATUSEX mem = { 0 };
    mem.dwLength = sizeof(mem);
    if (GlobalMemoryStatusEx(&mem))
        return (qty_t)mem.ullAvailPhys;
    return 0;
#elif PLATFORM_POSIX
    long a = sysconf(_SC_AVPHYS_PAGES);
    long b = sysconf(_SC_PAGESIZE);
    if (a > 0 && b > 0)
        return (qty_t)((unsigned long long)a * (unsigned long long)b);
    return 0;
#else
    return 0;
#endif
}
/* 获取内存页面大小 */
PORT_API qty_t system_info_get_page_size(void)
{
#if PLATFORM_WINDOWS
    SYSTEM_INFO si = { 0 };
    GetSystemInfo(&si);
    return (qty_t)si.dwPageSize;
#elif PLATFORM_POSIX
    long size = sysconf(_SC_PAGESIZE);
    return (size > 0) ? (qty_t)size : 0;
#else
    return 0;
#endif
}
/* 获取主机名 */
PORT_API bool_t system_info_get_hostname(char* buf, qty_t size)
{
    if ((buf && size) == 0) return 0;
#if PLATFORM_WINDOWS
    wchar_t wbuf[SYSTEM_STRING_SAFE_MAX] = { 0 };
    DWORD len = (DWORD)(sizeof(wbuf) / sizeof(wchar_t));
    if (GetComputerNameExW(ComputerNameDnsHostname, wbuf, &len) == 0)
        return 0;
    /* 转换为 UTF-8 */
    int needed = WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, buf, (int)size, NULL, NULL);
    if (needed == 0 || (size_t)needed > (size_t)size) return 0;
    return 1;
#elif PLATFORM_POSIX
    if (gethostname(buf, (size_t)size) == 0)
    {
        buf[size - 1] = '\0';  /* 保证终止 */
        return 1;
    }
    return 0;
#else
    return 0;
#endif
}
/* 获取系统启动后的运行时间s */
PORT_API qty_t system_info_get_uptime(void)
{
#if PLATFORM_WINDOWS
    return (qty_t)(GetTickCount64() / 1000);
#elif PLATFORM_POSIX
    struct sysinfo si = { 0 };
    if (sysinfo(&si) == 0)
        return (qty_t)si.uptime;
    return 0;
#else
    return 0;
#endif
}
/* 获取系统负载平均值 1|5|15 分钟 */
PORT_API bool_t system_info_get_loadavg(system_float_t* load1, system_float_t* load5, system_float_t* load15)
{
#if PLATFORM_WINDOWS
    system_info_aux_loadavg();
    if (load1) *load1 = (system_float_t)system_info_os_loadavg[0] / 65536.0;
    if (load5) *load5 = (system_float_t)system_info_os_loadavg[1] / 65536.0;
    if (load15) *load15 = (system_float_t)system_info_os_loadavg[2] / 65536.0;
    return 1;
#elif PLATFORM_POSIX
    struct sysinfo info = { 0 };
    if (sysinfo(&info) != 0) return 0;
    /* 内核负载值放大了 65536 倍,转换回浮点数 */
    if (load1) *load1 = (system_float_t)info.loads[0] / 65536.0;
    if (load5) *load5 = (system_float_t)info.loads[1] / 65536.0;
    if (load15) *load15 = (system_float_t)info.loads[2] / 65536.0;
    return 1;
#else
    return 0;
#endif
}
/* 系统主机名设置 需特权 */
PORT_API bool_t system_info_set_hostname(const char* name)
{
    if ((name && name[0] != '\0') == 0) return 0;
#if PLATFORM_WINDOWS
    wchar_t wbuf[SYSTEM_STRING_SAFE_MAX] = { 0 };
    if (system_string_to_native(wbuf, name, sizeof(wbuf)) == 0)return 0;
    if (SetComputerNameExW(ComputerNamePhysicalDnsHostname, wbuf))
        return 1;
    return 0;
#elif PLATFORM_POSIX
    return (sethostname(name, (size_t)system_string_len(name)) == 0) ? 1 : 0;
#else
    return 0;
#endif
}


#else


PORT_API const char* system_info_get_os_name(void) { ; }
PORT_API const char* system_info_get_os_version(void) { ; }
PORT_API qty_t system_info_get_cpu_cores(void) { ; }
PORT_API bool_t system_info_get_cpu_times(qty_t* user, qty_t* kernel, qty_t* idle) { ; }
PORT_API qty_t system_info_get_total_ram(void) { ; }
PORT_API qty_t system_info_get_free_ram(void) { ; }
PORT_API qty_t system_info_get_page_size(void) { ; }
PORT_API bool_t system_info_get_hostname(char* buf, qty_t size) { ; }
PORT_API qty_t system_info_get_uptime(void) { ; }
PORT_API bool_t system_info_get_loadavg(system_float_t* load1, system_float_t* load5, system_float_t* load15) { ; }
PORT_API bool_t system_info_set_hostname(const char* name) { ; }


#endif