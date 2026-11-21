#include "process.h"
#if COM_USE(CONFIG_SYSTEM_PROCESS)


#include "core/system/STATIC/base.h"
#include "core/system/STATIC/string.h"
#include "core/system/mmap.h"

#if PLATFORM_WINDOWS
/* 判断句柄是否需要在属性列表中剔除
 * 仅 Windows 7 及更早需要剔除控制台句柄
 * Windows 8+ 控制台句柄是真实内核句柄 可正常进属性列表
 * 返回值 1=需要剔除 0=正常加入属性列表
 */
typedef BOOL(WINAPI* IsWindows8OrGreater_t)(void);
PORT_HIDE static int system_process_is_console(HANDLE h)
{
	if (h == NULL || h == INVALID_HANDLE_VALUE) return 0;
	/* 缓存版本判断 Win8+ 直接返回 0 不剔除任何句柄 */
	static int s_win8plus = -1;
	if (s_win8plus < 0)
	{
		s_win8plus = 0;
		HMODULE hKernel = GetModuleHandleW(L"kernel32.dll");
		if (hKernel)
		{
			IsWindows8OrGreater_t fn =
				(IsWindows8OrGreater_t)GetProcAddress(hKernel, "IsWindows8OrGreater");
			if (fn && fn()) s_win8plus = 1;
		}
	}
	if (s_win8plus) return 0;
	/* Win7 及更早 控制台句柄不能进属性列表 */
	return GetFileType(h) == FILE_TYPE_CHAR;
}
#endif

/* 创建新的进程 */
PORT_API system_process_t system_process_spawn(const char* path, const struct system_process_attr_t* attr)
{
	if (path == 0 || attr == 0) return 0;
#if PLATFORM_WINDOWS
	/* 创建失败标志 默认成功 */
	qty_t error = 0;
	/* 环境字符集指针 */
	wchar_t* arg = 0;
	wchar_t* env = 0;
	wchar_t* workdir = 0;
	qty_t arglen = 0;
	qty_t envlen = 0;
	qty_t worklen = 0;
	qty_t maxlen = 0;
	/* 句柄表 */
	HANDLE* inherit_handles = NULL;
	DWORD inherit_count = 0;   /* 估计上限 用于分配内存 */
	DWORD inherit_actual = 0;  /* 实际加入属性列表的句柄数 */
	DWORD console_count = 0;   /* 控制台句柄数 用于决定是否回退传统继承 */
	/* mmap */
	struct system_mmap_prot_t prot = { 0 };
	prot.read = 1;
	prot.write = 1;
	struct system_mmap_flags_t flags = { 0 };
	/* 初始化 环境字符集指针 句柄表 */
	switch (0)
	{
	case 0:
	{
		/* 计算内存大小 */
		/* arg */
		arglen = 0;
		if (attr->argv != NULL && attr->argv[0] != NULL)
		{
			const char* const* args = attr->argv;
			while (*args != NULL) {
				arglen += system_string_len(*args) + 1;
				args++;
			}
			if (arglen > 0) arglen--;
		}
		else
			arglen = system_string_len(path) + 1;
		arglen *= sizeof(wchar_t);
		arglen += sizeof(wchar_t);
		/* env */
		envlen = 2 * sizeof(wchar_t);
		if (attr->envp != 0)
		{
			const char* const* envs = attr->envp;
			while (*envs != 0)
			{
				envlen += (system_string_len(*envs) + 1) * sizeof(wchar_t);
				envs++;
			}
		}
		/* workdir */
		worklen = SYSTEM_PATH_MAX * sizeof(wchar_t);
		/* inherit_count */
		if (attr->stdin_h) inherit_count++;
		if (attr->stdout_h) inherit_count++;
		if (attr->stderr_h) inherit_count++;
		if (attr->h_arr && attr->h_len > 0) inherit_count += (DWORD)attr->h_len;
		/* 申请内存 分配指针 */
		maxlen = arglen + envlen + worklen + (inherit_count * sizeof(HANDLE));
		char* data = system_mmap_new(0, maxlen, prot, flags);
		if (data == 0) { error++;break; }
		arg = (wchar_t*)((char*)data);
		env = (wchar_t*)((char*)arg + arglen);
		workdir = (wchar_t*)((char*)env + envlen);
		inherit_handles = (HANDLE*)((char*)workdir + worklen);
		/* 填充信息 */
		/* arg */
		wchar_t* ptr = arg;
		int first = 1;
		if (attr->argv != NULL && attr->argv[0] != NULL)
		{
			const char* const* args = attr->argv;
			while (*args != NULL) {
				if (!first) *ptr++ = L' ';
				size_t remaining = (arglen - (ptr - arg) * sizeof(wchar_t)) / sizeof(wchar_t);
				if (system_string_to_native(ptr, *args, remaining * sizeof(wchar_t)) == 0)
				{
					error++; break;
				}
				ptr += wcslen(ptr);
				first = 0;
				args++;
			}
			*ptr = L'\0';
		}
		else
		{
			if (system_string_to_native(arg, path, arglen) == 0)
			{
				error++; break;
			}
		}
		/* env */
		if (attr->envp != 0 && attr->envp[0] != 0)
		{
			wchar_t* eptr = env;
			const char* const* envs = attr->envp;
			while (*envs != 0)
			{
				size_t remaining = (envlen - (eptr - env) * sizeof(wchar_t)) / sizeof(wchar_t);
				if (system_string_to_native(eptr, *envs, remaining * sizeof(wchar_t)) == 0)
				{
					error++; break;
				}
				eptr += wcslen(eptr) + 1;
				envs++;
			}
			*eptr++ = L'\0';
			*eptr = L'\0';
		}
		else
			env = NULL;
		/* workdir */
		if (attr->workdir != 0)
		{
			if (system_string_to_native(workdir, attr->workdir, worklen) == 0)
			{
				error++; break;
			}
		}
		else
			workdir = NULL;
		/* inherit_handles 控制台句柄不能进属性列表 需统计回退 */
		if (attr->stdin_h)
		{
			if (system_process_is_console(attr->stdin_h))
				console_count++;
			else
			{
				SetHandleInformation(attr->stdin_h, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
				inherit_handles[inherit_actual++] = attr->stdin_h;
			}
		}
		if (attr->stdout_h)
		{
			if (system_process_is_console(attr->stdout_h))
				console_count++;
			else
			{
				SetHandleInformation(attr->stdout_h, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
				inherit_handles[inherit_actual++] = attr->stdout_h;
			}
		}
		if (attr->stderr_h)
		{
			if (system_process_is_console(attr->stderr_h))
				console_count++;
			else
			{
				SetHandleInformation(attr->stderr_h, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
				inherit_handles[inherit_actual++] = attr->stderr_h;
			}
		}
		if (attr->h_arr && attr->h_len > 0)
		{
			qty_t i = 0;
			for (; i < attr->h_len; i++)
			{
				if (attr->h_arr[i])
				{
					if (system_process_is_console(attr->h_arr[i]))
						console_count++;
					else
					{
						SetHandleInformation(attr->h_arr[i], HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
						inherit_handles[inherit_actual++] = attr->h_arr[i];
					}
				}
			}
		}
	}
	}
	/* 启动信息结构 */
	STARTUPINFOW si = { 0 };
	STARTUPINFOEXW si_ex = { 0 };
	BOOL use_ex = FALSE;
	/* 进程属性列表 */
	LPPROC_THREAD_ATTRIBUTE_LIST attr_list = NULL;
	SIZE_T attr_list_size = 0;
	/* 进程创建标志和结果 */
	DWORD creation_flags = 0;
	PROCESS_INFORMATION pi = { 0 };
	BOOL success = FALSE;
	/* 进程生命周期 */
	HANDLE hJob = NULL;
	/* 初始化 系统资源 */
	if (error == 0)
	{
		/* si */
		si.cb = sizeof(si);
		si.dwFlags |= STARTF_USESTDHANDLES;
		si.hStdInput = attr->stdin_h ? attr->stdin_h : GetStdHandle(STD_INPUT_HANDLE);
		si.hStdOutput = attr->stdout_h ? attr->stdout_h : GetStdHandle(STD_OUTPUT_HANDLE);
		si.hStdError = attr->stderr_h ? attr->stderr_h : GetStdHandle(STD_ERROR_HANDLE);
		/* si_ex */
		si_ex.StartupInfo = si;
		si_ex.StartupInfo.cb = sizeof(si_ex);
		/* attr_list 属性列表只装非控制台句柄 控制台句柄走 si */
		if (inherit_actual > 0)
		{
			SIZE_T attr_size = 0;
			InitializeProcThreadAttributeList(NULL, 1, 0, &attr_size);
			attr_list = (LPPROC_THREAD_ATTRIBUTE_LIST)system_mmap_new(0, attr_size, prot, flags);
			attr_list_size = (SIZE_T)attr_size;
			if (attr_list != NULL)
			{
				if (InitializeProcThreadAttributeList(attr_list, 1, 0, &attr_size))
				{
					if (UpdateProcThreadAttribute(
						attr_list,
						0,
						PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
						inherit_handles,
						inherit_actual * sizeof(HANDLE),
						NULL,
						NULL))
					{
						si_ex.lpAttributeList = attr_list;
						use_ex = TRUE;
					}
					else
					{
						error++; error++;
					}
				}
				else
				{
					error++; error++;
				}
			}
			else
			{
				error++;
			}
		}
		/* creation_flags */
		/* 如果环境变量块是宽字符 使用了env 需要加此标志 */
		if (env != NULL)
			creation_flags |= CREATE_UNICODE_ENVIRONMENT;
		/* 窗口显示方式 */
		if (attr->window == -1)
			creation_flags |= CREATE_NO_WINDOW;
		else if (attr->window == 1)
			/* window == 0 表示继承, 不添加标志 */
			creation_flags |= CREATE_NEW_CONSOLE;
		/* 进程分离 */
		if (attr->detach == 1)
			creation_flags |= DETACHED_PROCESS;
		/* 生命周期绑定 */
		if (attr->detach == 0 && attr->exit_extends)
		{
			hJob = CreateJobObject(NULL, NULL);
			if (hJob)
			{
				JOBOBJECT_EXTENDED_LIMIT_INFORMATION info = { 0 };
				info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
				if (SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &info, sizeof(info)) == 0)
				{
					CloseHandle(hJob);
					hJob = NULL;
				}
			}
		}
	}
	/* 创建进程初始化失败 清理返回 */
	switch (error)
	{
	default:
	case 3:
		if (attr_list) DeleteProcThreadAttributeList(attr_list);
	case 2:
		system_mmap_del(attr_list, attr_list_size);
	case 1:
		if (hJob) CloseHandle(hJob);
		system_mmap_del(arg, maxlen);
		return 0;
	case 0:
		((void)0);
	}
	/* 创建进程 */
	if (use_ex)
	{
		creation_flags |= EXTENDED_STARTUPINFO_PRESENT;
		success = CreateProcessW(
			NULL,               /* 应用程序名称 使用命令行中的第一个参数 */
			arg,                /* 命令行 */
			NULL,               /* 进程安全属性 */
			NULL,               /* 线程安全属性 */
			TRUE,              /* 禁止句柄继承 通过属性列表继承 */
			creation_flags,     /* 窗口和分离 */
			env,                /* 环境变量块 可为NULL */
			workdir,            /* 工作目录 */
			&si_ex.StartupInfo, /* 使用扩展启动信息 */
			&pi
		);
	}
	else
	{
		/* inherit_actual == 0 只有控制台句柄或一个都没传 */
		if (console_count == 0)
		{
			/* 一个句柄都没传 原语义 拒绝创建 */
			if (hJob) CloseHandle(hJob);
			system_mmap_del(arg, maxlen);
			return 0;
		}
		/* 只有控制台句柄 伪句柄靠 si 传递 不继承任何句柄 */
		success = CreateProcessW(
			NULL, arg, NULL, NULL,
			FALSE,              /* 关键: 不继承任何句柄 */
			creation_flags, env, workdir,
			&si, &pi
		);
	}
	/* 资源托管 */
	if (success && hJob)
		AssignProcessToJobObject(hJob, pi.hProcess);
	/* 创建进程成功 清理资源 */
	{
		if (attr_list) DeleteProcThreadAttributeList(attr_list);
		system_mmap_del(attr_list, attr_list_size);
		system_mmap_del(arg, maxlen);
	}
	if (success)
	{
		CloseHandle(pi.hThread);
		return (system_process_t)pi.hProcess;
	}
	else
		return 0;
#elif PLATFORM_POSIX
	pid_t pid = fork();
	if (pid == -1) return 0;
	if (pid == 0)
	{
		/* detach */
		if (attr->detach)
		{
			if (setsid() < 0) _exit(127);
			pid_t pid2 = fork();
			if (pid2 < 0) _exit(127);
			if (pid2 > 0) _exit(0);
		}
		/* handle std */
		if (attr->stdin_h)  dup2((int)(qty_t)attr->stdin_h, STDIN_FILENO);
		if (attr->stdout_h) dup2((int)(qty_t)attr->stdout_h, STDOUT_FILENO);
		if (attr->stderr_h) dup2((int)(qty_t)attr->stderr_h, STDERR_FILENO);
		/* handle arr */
		int maxfd = (int)sysconf(_SC_OPEN_MAX);
		if (maxfd == -1) maxfd = 1024;
		for (int fd = 3; fd < maxfd; fd++)
		{
			int keep = 0;
			if (attr->h_arr && attr->h_len > 0)
			{
				for (qty_t i = 0; i < attr->h_len; i++)
				{
					if ((int)(qty_t)attr->h_arr[i] == fd) { keep = 1; break; }
				}
			}
			if (keep == 0) close(fd);
		}
		/* workdir */
		if (attr->workdir)
			chdir(attr->workdir);
		/* window */
		((void)0);
		/* exit_extends */
		if (attr->detach == 0 && attr->exit_extends) prctl(PR_SET_PDEATHSIG, SIGTERM);
		/* argv envp */
		if (attr->envp != NULL)
			execve(path, (char**)attr->argv, (char**)attr->envp);
		else
			execvp(path, (char**)attr->argv);
		/* exec 失败 */
		_exit(127);
	}
	return (system_process_t)(qty_t)pid;
#else
	return 0;
#endif
}
/* 获取当前进程ID */
PORT_API qty_t system_process_getpid(void)
{
#if PLATFORM_WINDOWS
	return (qty_t)GetCurrentProcessId();
#elif PLATFORM_POSIX
	return (qty_t)getpid();
#else
	return 0;
#endif
}
/*
* 等待进程退出 支持超时
* proc:进程句柄 info:输出进程信息|退出码和状态 timeout_ms:超时毫秒数|-1表示无限等待|0表示立即返回
* 返回 { 1=进程已退出|info有效 0=超时|pid和exited=0有效|其他字段无效 -1=错误 }
*/
PORT_API bool_t system_process_wait(system_process_t proc, struct system_process_info_t* info, qty_t timeout_ms)
{
	if (proc == 0 || info == 0) return -1;
	*info = (struct system_process_info_t){ 0 };
#if PLATFORM_WINDOWS
	DWORD timeout = (timeout_ms == (qty_t)-1) ? INFINITE : (DWORD)timeout_ms;
	DWORD ret = WaitForSingleObject(proc, timeout);
	if (ret == WAIT_OBJECT_0)
	{
		DWORD exit_code;
		if (GetExitCodeProcess(proc, &exit_code) == 0)
			return -1;
		info->pid = (qty_t)GetProcessId(proc);
		if (info->pid == 0) return -1;
		info->exited = 1;
		info->exit_code = (system_status_t)exit_code;
		info->signal = (system_status_t)0;
		return 1;
	}
	else if (ret == WAIT_TIMEOUT)
	{
		info->pid = (qty_t)GetProcessId(proc);
		if (info->pid == 0) return -1;
		info->exited = 0;
		return 0;
	}
	else
		return -1;
#elif PLATFORM_POSIX
	int status = 0;
	int options = (timeout_ms == (qty_t)-1) ? 0 : WNOHANG;
	struct timespec start_ts, now_ts;
	if (timeout_ms != (qty_t)-1 && timeout_ms != 0)
		clock_gettime(CLOCK_MONOTONIC, &start_ts);
	/* 循环主体 */
	while (1)
	{
		pid_t ret = waitpid((pid_t)(qty_t)proc, &status, options);
		if (ret == (pid_t)(qty_t)proc)
		{
			info->pid = (qty_t)ret;
			info->exited = 1;
			info->exit_code = 0;
			info->signal = 0;
			if (WIFEXITED(status))
				info->exit_code = WEXITSTATUS(status);
			else if (WIFSIGNALED(status))
				info->signal = WTERMSIG(status);
			return 1;
		}
		else if (ret == 0)
		{
			if (timeout_ms == (qty_t)-1)
				continue;
			if (timeout_ms == 0)
			{
				info->pid = (qty_t)proc;
				info->exited = 0;
				return 0;
			}
			/* 计算已经过的时间 (毫秒) */
			clock_gettime(CLOCK_MONOTONIC, &now_ts);
			qty_t elapsed_ms = (qty_t)(
				(now_ts.tv_sec - start_ts.tv_sec) * 1000 +
				(now_ts.tv_nsec - start_ts.tv_nsec) / 1000000
				);
			if (elapsed_ms >= timeout_ms)
			{
				/* 超时 */
				info->pid = (qty_t)proc;
				info->exited = 0;
				return 0;
			}
			/* 计算剩余时间, 并睡眠一段固定间隔 (例如 10ms) */
			qty_t remaining_ms = timeout_ms - elapsed_ms;
			qty_t sleep_ms = (remaining_ms > 10) ? 10 : remaining_ms;

			struct timespec req;
			req.tv_sec = (time_t)(sleep_ms / 1000);
			req.tv_nsec = (long)((sleep_ms % 1000) * 1000000);
			while (nanosleep(&req, &req) == -1 && errno == EINTR) { ; }
		}
		else
		{
			if (errno == EINTR)
				continue;
			else
				return -1;
		}
	}
#else
	return -1;
#endif
}
/* 终止进程 强制结束 */
PORT_API bool_t system_process_kill(system_process_t proc)
{
	if (proc == 0) return 0;
#if PLATFORM_WINDOWS
	return TerminateProcess(proc, 1) ? 1 : 0;
#elif PLATFORM_POSIX
	return (kill((pid_t)(qty_t)proc, SIGKILL) == 0) ? 1 : 0;
#else
	return 0;
#endif
}
/* 终止当前进程 */
PORT_API bool_t system_process_kill_self(void)
{
#if PLATFORM_WINDOWS
	return TerminateProcess(GetCurrentProcess(), 0) ? 1 : 0;
#elif PLATFORM_POSIX
	return kill(getpid(), SIGKILL) == 0 ? 1 : 0;
#else
	return 0;
#endif
}
/* 清理进程 回收资源 */
PORT_API bool_t system_process_close(system_process_t proc)
{
	if (proc == 0) return 0;
#if PLATFORM_WINDOWS
	return CloseHandle(proc) ? 1 : 0;
#elif PLATFORM_POSIX
	int status;
	return (waitpid((pid_t)(qty_t)proc, &status, 0) == (pid_t)(qty_t)proc) ? 1 : 0;
#else
	return 0;
#endif
}


#else


PORT_API system_process_t system_process_spawn(const char* path, const system_process_attr_t* attr) { ; }
PORT_API qty_t system_process_getpid(void) { ; }
PORT_API bool_t system_process_wait(system_process_t proc, system_process_info_t* info, qty_t timeout_ms) { ; }
PORT_API bool_t system_process_kill(system_process_t proc) { ; }
PORT_API bool_t system_process_kill_self(void) { ; }
PORT_API bool_t system_process_close(proc) { ; }


#endif