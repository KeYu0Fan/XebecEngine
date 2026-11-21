#include "pipe.h"
#if COM_USE(CONFIG_SYSTEM_PIPE)


#include "core/system/STATIC/base.h"
#include "core/system/STATIC/string.h"

/* PIPE 匿名管道 */

/* 创建匿名管道句柄 */
PORT_API bool_t system_pipe_open(system_pipe_t* fpR, system_pipe_t* fpW)
{
	if (fpW == 0 || fpR == 0) return 0;
#if PLATFORM_WINDOWS
	HANDLE hRead = 0, hWrite = 0;
	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
	if (CreatePipe(&hRead, &hWrite, &sa, 0) == 0) return 0;
	*fpR = hRead;
	*fpW = hWrite;
	return 1;
#elif PLATFORM_POSIX
	int fds[2];
	if (pipe(fds) == -1) return 0;
	*fpR = (system_pipe_t)(qty_t)(fds[0] + 1);
	*fpW = (system_pipe_t)(qty_t)(fds[1] + 1);
	return 1;
#else
	return 0;
#endif
}
/* 关闭管道句柄 */
PORT_API bool_t system_pipe_close(system_pipe_t fp)
{
	if (fp == 0) return 0;
#if PLATFORM_WINDOWS
	return CloseHandle(fp) ? 1 : 0;
#elif PLATFORM_POSIX
	return (close((int)(qty_t)fp - 1) == 0) ? 1 : 0;
#else
	return 0;
#endif
}
/* 读管道 */
PORT_API bool_t system_pipe_read(system_pipe_t fp, void* buf, qty_t count, qty_t* buf_size)
{
	if (fp == 0 || buf == 0 || buf_size == 0) return 0;
	*buf_size = 0;
	if (count == 0) return 1;
	unsigned char* buf_tmp = buf;
#if PLATFORM_WINDOWS
	while (count > 0)
	{
		DWORD bytesRead = 0;
		DWORD bytesToRead = (count > ((DWORD)-1)) ? ((DWORD)-1) : (DWORD)count;
		bool_t ret = (bool_t)ReadFile(fp, buf_tmp, bytesToRead, &bytesRead, NULL);
		if (ret == 0)
		{
			if (*buf_size > 0)return 1;
			return 0;
		}
		buf_tmp += bytesRead;
		count -= bytesRead;
		*buf_size += (qty_t)bytesRead;
		if (bytesRead == 0) break;
	}
	return 1;
#elif PLATFORM_POSIX
	while (count > 0)
	{
		ssize_t ret = read((int)(qty_t)fp - 1, buf_tmp, (size_t)count);
		if (ret < 0)
		{
			if (*buf_size) return 1;
			return 0;
		}
		buf_tmp += ret;
		*buf_size += (qty_t)ret;
		count -= (qty_t)ret;
		if (ret == 0) break;
	}
	return 1;
#else
	return 0;
#endif
}
/* 写管道 */
PORT_API bool_t system_pipe_write(system_pipe_t fp, const void* buf, qty_t count, qty_t* buf_size)
{
	if (fp == 0 || buf == 0 || buf_size == 0) return 0;
	*buf_size = 0;
	if (count == 0) return 1;
	const unsigned char* buf_tmp = buf;
#if PLATFORM_WINDOWS
	while (count > 0)
	{
		DWORD bytesWritten = 0;
		DWORD bytesToWrite = (count > ((DWORD)-1)) ? ((DWORD)-1) : (DWORD)count;
		bool_t ret = (bool_t)WriteFile(fp, buf_tmp, bytesToWrite, &bytesWritten, NULL);
		if (ret == 0)
		{
			if (*buf_size > 0)return 1;
			return 0;
		}
		buf_tmp += bytesWritten;
		count -= bytesWritten;
		*buf_size += (qty_t)bytesWritten;
		if (bytesWritten == 0) break;
	}
	return 1;
#elif PLATFORM_POSIX
	while (count > 0)
	{
		ssize_t ret = write((int)(qty_t)fp - 1, buf_tmp, (size_t)count);
		if (ret < 0)
		{
			if (*buf_size > 0) return 1;
			return 0;
		}
		*buf_size += (qty_t)ret;
		buf_tmp += ret;
		count -= (qty_t)ret;
		if (ret == 0) break;
	}
	return 1;
#else
	return 0;
#endif
}
/* 设置非阻塞模式 enable = { 1=开启非阻塞 0=关闭非阻塞 } */
PORT_API bool_t system_pipe_set_nonblock(system_pipe_t fp, qty_t enable)
{
	if (fp == 0) return 0;
#if PLATFORM_WINDOWS
	DWORD mode = (enable ? PIPE_NOWAIT : PIPE_WAIT);
	if (SetNamedPipeHandleState(fp, &mode, NULL, NULL) == 0)
		return 0;
	return 1;
#elif PLATFORM_POSIX
	int flags = fcntl((int)(qty_t)fp - 1, F_GETFL, 0);
	if (flags == -1) return 0;
	if (enable)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	return (fcntl((int)(qty_t)fp - 1, F_SETFL, flags) == 0) ? 1 : 0;
#else
	return 0;
#endif
}
/* 等待管道可读/可写 支持超时毫秒 mode ={ 0=读 1=写 2=读或写 } 返回 1=事件发生 0=超时 -1=错误 */
PORT_API bool_t system_pipe_wait(system_pipe_t fp, qty_t mode, qty_t ms)
{
	if (fp == 0) return 0;
#if PLATFORM_WINDOWS
	(void)mode; /* Windows 只等待可读, 无读写模式区分 */
	DWORD timeout = (ms == (qty_t)-1) ? INFINITE : (DWORD)ms;
	DWORD ret = WaitForSingleObject(fp, timeout);
	if (ret == WAIT_OBJECT_0) return 1;
	if (ret == WAIT_TIMEOUT) return 0;
	return -1;
#elif PLATFORM_POSIX
	struct pollfd pfd;
	pfd.fd = (int)(qty_t)fp - 1;
	pfd.events = 0;
	if (mode == 0 || mode == 2) pfd.events |= POLLIN;
	if (mode == 1 || mode == 2) pfd.events |= POLLOUT;
	int timeout = (ms == (qty_t)-1) ? -1 : (int)ms;
	int ret = poll(&pfd, 1, timeout);
	if (ret > 0) return 1;
	if (ret == 0) return 0;
	return -1;
#else
	return -1;
#endif
}

/* FIFO 命名管道 */

/* 创建命名管道句柄 */
PORT_API bool_t system_fifo_new(const char* path)
{
	if (path == 0) return 0;
#if PLATFORM_WINDOWS
	return 1;
#elif PLATFORM_POSIX
	return (mkfifo(path, 0666) == 0) ? 1 : 0;
#else
	return 0;
#endif
}
/* 销毁命名管道句柄 */
PORT_API bool_t system_fifo_del(const char* path)
{
	if (path == 0) return 0;
#if PLATFORM_WINDOWS
	return 1;
#elif PLATFORM_POSIX
	return (unlink(path) == 0) ? 1 : 0;
#else
	return 0;
#endif
}
/* 开启服务端 等待客户端 */
PORT_API system_pipe_t system_fifo_server(const char* path)
{
	if (path == 0) return 0;
#if PLATFORM_WINDOWS
	wchar_t pipe_L[SYSTEM_PATH_MAX] = L"\\\\.\\pipe\\";
	qty_t pipe_size = wcslen(pipe_L);
	{
		char* tmp = "\\\\.\\pipe\\";
		qty_t a = 0;
		while (path[a] == tmp[a])
		{
			a++;
		}
		if (tmp[a] == 0)path += a;
	}
	if (system_string_to_native(pipe_L + pipe_size, path, sizeof(pipe_L) - pipe_size * sizeof(wchar_t)) == 0)
		return 0;
	HANDLE hPipe = CreateNamedPipeW(
		pipe_L, PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
		1, 4096, 4096, 0, NULL);
	if (hPipe == INVALID_HANDLE_VALUE) return 0;
	/* 等待客户端连接 */
	if (ConnectNamedPipe(hPipe, NULL) == 0)
	{
		if (GetLastError() != ERROR_PIPE_CONNECTED)
		{
			CloseHandle(hPipe);
			return 0;
		}
	}
	return hPipe;
#elif PLATFORM_POSIX
	int fd = open(path, O_RDWR);
	if (fd < 0) return 0;
	return (system_pipe_t)(qty_t)(fd + 1);
#else
	return 0;
#endif
}
/* 开启客户端 连接服务端 */
PORT_API system_pipe_t system_fifo_client(const char* path)
{
	if (path == 0) return 0;
#if PLATFORM_WINDOWS
	wchar_t pipe_L[SYSTEM_PATH_MAX] = L"\\\\.\\pipe\\";
	qty_t pipe_size = wcslen(pipe_L);
	{
		char* tmp = "\\\\.\\pipe\\";
		qty_t a = 0;
		while (path[a] == tmp[a])
		{
			a++;
		}
		if (tmp[a] == 0)path += a;
	}
	if (system_string_to_native(pipe_L + pipe_size, path, sizeof(pipe_L) - pipe_size * sizeof(wchar_t)) == 0)
		return 0;
	HANDLE hPipe = CreateFileW(
		pipe_L, GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, 0, NULL);
	if (hPipe == INVALID_HANDLE_VALUE) return 0;
	return (system_pipe_t)hPipe;
#elif PLATFORM_POSIX
	int fd = open(path, O_RDWR);
	if (fd < 0) return 0;
	return (system_pipe_t)(qty_t)(fd + 1);
#else
	return 0;
#endif
}


#else


PORT_API bool_t system_pipe_open(handle* fpW, handle* fpR) { ; }
PORT_API bool_t system_pipe_close(handle fp) { ; }
PORT_API bool_t system_pipe_read(handle fp, void* buf, qty_t count, qty_t* buf_size) { ; }
PORT_API bool_t system_pipe_write(handle fp, void* buf, qty_t count, qty_t* buf_size) { ; }
PORT_API bool_t system_pipe_set_nonblock(handle fp, qty_t enable) { ; }
PORT_API bool_t system_pipe_wait(handle fp, qty_t mode, qty_t ms) { ; }
PORT_API bool_t system_fifo_new(const char* path) { ; }
PORT_API bool_t system_fifo_del(const char* path) { ; }
PORT_API handle system_fifo_server(const char* path) { ; }
PORT_API handle system_fifo_client(const char* path) { ; }


#endif