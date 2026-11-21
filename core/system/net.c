#include "net.h"
#if COM_USE(CONFIG_SYSTEM_NET)


#include "core/system/STATIC/base.h"
#include "core/system/thread.h"

#if PLATFORM_WINDOWS
#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif
#elif PLATFORM_POSIX

#else

#endif

#define SYSTEM_NET_CLOSED  ((qty_t)0x7FFFFFFF)
#define SYSTEM_NET_EINVAL  ((qty_t)0x7FFFFFFE)

#if COM_USE(CONFIG_SYSTEM_THREAD)
PORT_HIDE static system_tls_key system_net_key = 0;
PORT_HIDE static bool_t system_net_var = 0;
PORT_HIDE static void system_init(void)
{
	system_net_var = system_tls_key_init(&system_net_key, 0);
}
PORT_HIDE static void system_set(void* value)
{
	if (system_net_var)
		system_tls_key_set(system_net_key, value);
}
PORT_HIDE static void* system_get(void)
{
	if (system_net_var)
		return system_tls_key_get(system_net_key);
	return 0;
}
#else
PORT_HIDE static void* system_net_key = 0;
PORT_HIDE static void system_set(void* value)
{
	system_net_key = value;
}
PORT_HIDE static void* system_get(void)
{
	return system_net_key;
}
#endif

/* 初始化net */
PORT_API bool_t system_net_init(void)
{
#if COM_USE(CONFIG_SYSTEM_THREAD)
	PORT_HIDE static system_once_init init = { 0 };
	system_once_run(&init, &system_init);
#endif
#if PLATFORM_WINDOWS
	WSADATA wsaData = { 0 };
	WORD wVersionRequested = MAKEWORD(2, 2);
	bool_t e = WSAStartup(wVersionRequested, &wsaData) == 0 ? 1 : 0;
	if (e == 0)
		system_set((void*)(qty_t)WSAGetLastError());
	return e;
#elif PLATFORM_POSIX
	bool_t e = signal(SIGPIPE, SIG_IGN) != SIG_ERR ? 1 : 0;
	if (e == 0)
		system_set((void*)(qty_t)errno);
	return e;
#else
	return 0;
#endif
}
/* 清理net */
PORT_API bool_t system_net_clean(void)
{
#if PLATFORM_WINDOWS
	bool_t e = WSACleanup() == 0 ? 1 : 0;
	if (e == 0)
		system_set((void*)(qty_t)WSAGetLastError());
	return e;
#elif PLATFORM_POSIX
	return 1;
#else
	return 0;
#endif
}

/* 创建端点 domain协议族 type通信协议 protocol具体协议 失败返回-1 */
PORT_API system_net_socket_t system_net_socket(qty_t domain, qty_t type, qty_t protocol)
{
#if PLATFORM_WINDOWS
	SOCKET s = socket((int)domain, (int)type, (int)protocol);
	s = (s == INVALID_SOCKET ? -1 : s);
	if (s == -1)
		system_set((void*)(qty_t)WSAGetLastError());
	return (system_net_socket_t)s;
#elif PLATFORM_POSIX
	int s = socket((int)domain, (int)type, (int)protocol);
	if (s == -1)
		system_set((void*)(qty_t)errno);
	return (system_net_socket_t)(qty_t)s;
#else
	return (system_net_socket_t)-1;
#endif
}
/* 销毁端点 */
PORT_API bool_t system_net_close(system_net_socket_t sock)
{
#if PLATFORM_WINDOWS
	bool_t e = closesocket((SOCKET)sock) == 0 ? 1 : 0;
	if (e == 0)
		system_set((void*)(qty_t)WSAGetLastError());
	return e;
#elif PLATFORM_POSIX
	bool_t e = close((int)(qty_t)sock) == 0 ? 1 : 0;
	if (e == 0)
		system_set((void*)(qty_t)errno);
	return e;
#else
	return 0;
#endif
}

/* 连接远端 客户端 */
PORT_API bool_t system_net_connect(system_net_socket_t sock, const system_sock_addr_t addr, qty_t addrlen)
{
#if PLATFORM_WINDOWS
	bool_t e = connect((SOCKET)sock, addr, (int)addrlen) == 0 ? 1 : 0;
	if (e == 0)
		system_set((void*)(qty_t)WSAGetLastError());
	return e;
#elif PLATFORM_POSIX
	bool_t e = connect((int)(qty_t)sock, addr, (socklen_t)addrlen) == 0 ? 1 : 0;
	if (e == 0)
		system_set((void*)(qty_t)errno);
	return e;
#else
	return 0;
#endif
}
/* 固定端口 服务端 */
PORT_API bool_t system_net_bind(system_net_socket_t sock, const system_sock_addr_t addr, qty_t addrlen)
{
#if PLATFORM_WINDOWS
	bool_t e = bind((SOCKET)sock, addr, (int)addrlen) == 0 ? 1 : 0;
	if (e == 0)
		system_set((void*)(qty_t)WSAGetLastError());
	return e;
#elif PLATFORM_POSIX
	bool_t e = bind((int)(qty_t)sock, addr, (socklen_t)addrlen) == 0 ? 1 : 0;
	if (e == 0)
		system_set((void*)(qty_t)errno);
	return e;
#else
	return 0;
#endif
}
/* 切换服务状态 主动->被动 等待连接 backlog为连接队列最大长度 服务端 */
PORT_API bool_t system_net_listen(system_net_socket_t sock, qty_t backlog)
{
#if PLATFORM_WINDOWS
	bool_t e = listen((SOCKET)sock, (int)backlog) == 0 ? 1 : 0;
	if (e == 0)
		system_set((void*)(qty_t)WSAGetLastError());
	return e;
#elif PLATFORM_POSIX
	bool_t e = listen((int)(qty_t)sock, (int)backlog) == 0 ? 1 : 0;
	if (e == 0)
		system_set((void*)(qty_t)errno);
	return e;
#else
	return 0;
#endif
}
/* 接受连接 失败返回-1 服务端 */
PORT_API system_net_socket_t system_net_accept(system_net_socket_t sock, system_sock_addr_t addr, qty_t* addrlen)
{
	if (addrlen == 0) { system_set((void*)SYSTEM_NET_EINVAL); return (system_net_socket_t)-1; }
#if PLATFORM_WINDOWS
	int len = (int)*addrlen;
	SOCKET s = accept((SOCKET)sock, addr, &len);
	*addrlen = (qty_t)len;
	if (s == INVALID_SOCKET)
		system_set((void*)(qty_t)WSAGetLastError());
	return (system_net_socket_t)s;
#elif PLATFORM_POSIX
	socklen_t len = (socklen_t)*addrlen;
	int s = accept((int)(qty_t)sock, addr, &len);
	*addrlen = (qty_t)len;
	if (s == -1)
		system_set((void*)(qty_t)errno);
	return (system_net_socket_t)(qty_t)s;
#else
	return (system_net_socket_t)-1;
#endif
}

/* 读端点 返回实际字节数 0表示对端关闭或异常 需调用error区分 */
PORT_API qty_t system_net_recv(system_net_socket_t sock, void* buf, qty_t count, qty_t flags)
{
#if PLATFORM_WINDOWS
	int n = recv((SOCKET)sock, buf, (int)count, (int)flags);
	if (n < 0)
	{
		system_set((void*)(qty_t)WSAGetLastError());
		return 0;
	}
	if (n == 0) system_set((void*)SYSTEM_NET_CLOSED);
	return (qty_t)n;
#elif PLATFORM_POSIX
	ssize_t n = recv((int)(qty_t)sock, buf, (size_t)count, (int)flags);
	if (n < 0)
	{
		system_set((void*)(qty_t)errno);
		return 0;
	}
	if (n == 0) system_set((void*)SYSTEM_NET_CLOSED);
	return (qty_t)n;
#else
	return 0;
#endif
}
/* 写端点 返回实际字节数 0表示对端关闭或异常 需调用error区分 */
PORT_API qty_t system_net_send(system_net_socket_t sock, const void* buf, qty_t count, qty_t flags)
{
#if PLATFORM_WINDOWS
	int n = send((SOCKET)sock, buf, (int)count, (int)flags);
	if (n < 0)
	{
		system_set((void*)(qty_t)WSAGetLastError());
		return 0;
	}
	if (n == 0) system_set((void*)SYSTEM_NET_CLOSED);
	return (qty_t)n;
#elif PLATFORM_POSIX
	ssize_t n = send((int)(qty_t)sock, buf, (size_t)count, (int)flags);
	if (n < 0)
	{
		system_set((void*)(qty_t)errno);
		return 0;
	}
	if (n == 0) system_set((void*)SYSTEM_NET_CLOSED);
	return (qty_t)n;
#else
	return 0;
#endif
}

/* 获取本地地址 */
PORT_API bool_t system_net_get_local(system_net_socket_t sock, system_sock_addr_t addr, qty_t* addrlen)
{
	if (addrlen == 0) { system_set((void*)SYSTEM_NET_EINVAL); return -1; }
#if PLATFORM_WINDOWS
	int len = (int)*addrlen;
	int e = getsockname((SOCKET)sock, addr, &len) == 0 ? 1 : 0;
	*addrlen = (qty_t)len;
	if (e == 0)
		system_set((void*)(qty_t)WSAGetLastError());
	return e;
#elif PLATFORM_POSIX
	socklen_t len = (socklen_t)*addrlen;
	bool_t e = getsockname((int)(qty_t)sock, addr, &len) == 0 ? 1 : 0;
	*addrlen = (qty_t)len;
	if (e == 0)
		system_set((void*)(qty_t)errno);
	return e;
#else
	return 0;
#endif
}
/* 获取远端地址 */
PORT_API bool_t system_net_get_remote(system_net_socket_t sock, system_sock_addr_t addr, qty_t* addrlen)
{
	if (addrlen == 0) { system_set((void*)SYSTEM_NET_EINVAL); return -1; }
#if PLATFORM_WINDOWS
	int len = (int)*addrlen;
	bool_t e = getpeername((SOCKET)sock, addr, &len) == 0 ? 1 : 0;
	*addrlen = (qty_t)len;
	if (e == 0)
		system_set((void*)(qty_t)WSAGetLastError());
	return e;
#elif PLATFORM_POSIX
	socklen_t len = (socklen_t)*addrlen;
	bool_t e = getpeername((int)(qty_t)sock, addr, &len) == 0 ? 1 : 0;
	*addrlen = (qty_t)len;
	if (e == 0)
		system_set((void*)(qty_t)errno);
	return e;
#else
	return 0;
#endif
}

/* 设置 socket 选项 */
PORT_API bool_t system_net_set_opt(system_net_socket_t sock, qty_t level, qty_t optname, const void* optval, qty_t optlen)
{
#if PLATFORM_WINDOWS
	bool_t e = setsockopt((SOCKET)sock, (int)level, (int)optname, optval, (int)optlen) == 0 ? 1 : 0;
	if (e == 0)
		system_set((void*)(qty_t)WSAGetLastError());
	return e;
#elif PLATFORM_POSIX
	bool_t e = setsockopt((int)(qty_t)sock, (int)level, (int)optname, optval, (socklen_t)optlen) == 0 ? 1 : 0;
	if (e == 0)
		system_set((void*)(qty_t)errno);
	return e;
#else
	return 0;
#endif
}
/* 获取 socket 选项 */
PORT_API bool_t system_net_get_opt(system_net_socket_t sock, qty_t level, qty_t optname, void* optval, qty_t* optlen)
{
	if (optlen == 0) { system_set((void*)SYSTEM_NET_EINVAL); return -1; }
#if PLATFORM_WINDOWS
	int len = (int)*optlen;
	bool_t e = getsockopt((SOCKET)sock, (int)level, (int)optname, optval, &len) == 0 ? 1 : 0;
	*optlen = (qty_t)len;
	if (e == 0)
		system_set((void*)(qty_t)WSAGetLastError());
	return e;
#elif PLATFORM_POSIX
	socklen_t len = (socklen_t)*optlen;
	bool_t e = getsockopt((int)(qty_t)sock, (int)level, (int)optname, optval, &len) == 0 ? 1 : 0;
	*optlen = (qty_t)len;
	if (e == 0)
		system_set((void*)(qty_t)errno);
	return e;
#else
	return 0;
#endif
}

/* 设置读写模式 1不阻塞 0阻塞 */
PORT_API bool_t system_net_set_nonblock(system_net_socket_t sock, qty_t enable)
{
#if PLATFORM_WINDOWS
	u_long iMode = enable ? 1 : 0;
	bool_t e = ioctlsocket((SOCKET)sock, FIONBIO, &iMode) == NO_ERROR ? 1 : 0;
	if (e == 0)
		system_set((void*)(qty_t)WSAGetLastError());
	return e;
#elif PLATFORM_POSIX
	int flags = fcntl((int)(qty_t)sock, F_GETFL, 0);
	if (flags == -1)
	{
		system_set((void*)(qty_t)errno);
		return 0;
	}
	if (enable) flags |= O_NONBLOCK;
	else flags &= ~O_NONBLOCK;
	bool_t e = fcntl((int)(qty_t)sock, F_SETFL, flags) != -1 ? 1 : 0;
	if (e == 0)
		system_set((void*)(qty_t)errno);
	return e;
#else
	return 0;
#endif
}
/* 获取最后一个错误 无错误返回空指针 */
PORT_API const char* system_net_error(void)
{
	qty_t err = (qty_t)system_get();
#if PLATFORM_WINDOWS
	switch (err)
	{
	case 0:                  return 0;

		/* 连接状态 */
	case SYSTEM_NET_CLOSED:  return "connection closed";
	case WSAECONNRESET:      return "connection reset";
	case WSAECONNREFUSED:    return "connection refused";
	case WSAECONNABORTED:    return "connection aborted";
	case WSAENOTCONN:        return "not connected";
	case WSAEISCONN:         return "already connected";
	case WSAESHUTDOWN:       return "cannot send after shutdown";
	case WSAEDISCON:         return "graceful shutdown in progress";

		/* 超时/重试 */
	case WSAETIMEDOUT:       return "timed out";
	case WSAEWOULDBLOCK:     return "would block";
	case WSAEINPROGRESS:     return "operation in progress";
	case WSAEALREADY:        return "operation already in progress";

		/* 网络可达性 */
	case WSAENETDOWN:        return "network is down";
	case WSAENETUNREACH:     return "network unreachable";
	case WSAENETRESET:       return "network dropped connection";
	case WSAEHOSTDOWN:       return "host is down";
	case WSAEHOSTUNREACH:    return "host unreachable";

		/* 地址相关 */
	case WSAEADDRINUSE:      return "address already in use";
	case WSAEADDRNOTAVAIL:   return "address not available";
	case WSAEDESTADDRREQ:    return "destination address required";

		/* 参数/操作错误 */
	case SYSTEM_NET_EINVAL:  return "invalid argument";
	case WSAEINVAL:          return "invalid argument";
	case WSAEFAULT:          return "bad address";
	case WSAEINTR:           return "interrupted";
	case WSAEACCES:          return "permission denied";
	case WSAEBADF:           return "bad file descriptor";
	case WSAEMSGSIZE:        return "message too long";
	case WSAEOPNOTSUPP:      return "operation not supported";
	case WSAENOPROTOOPT:     return "bad protocol option";
	case WSAEPROTOTYPE:      return "protocol wrong type for socket";

		/* Socket 对象错误 */
	case WSAENOTSOCK:        return "not a socket";
	case WSAEAFNOSUPPORT:    return "address family not supported";
	case WSAEPFNOSUPPORT:    return "protocol family not supported";
	case WSAEPROTONOSUPPORT: return "protocol not supported";
	case WSAESOCKTNOSUPPORT: return "socket type not supported";

		/* 资源不足 */
	case WSAEMFILE:          return "too many open files";
	case WSAENOBUFS:         return "no buffer space";

		/* 系统/初始化 */
	case WSANOTINITIALISED:  return "WSAStartup not called";
	case WSASYSNOTREADY:     return "network subsystem not ready";
	case WSAVERNOTSUPPORTED: return "Winsock version not supported";

		/* DNS 相关 */
	case WSAHOST_NOT_FOUND:  return "host not found";
	case WSATRY_AGAIN:       return "DNS temporary failure";
	case WSANO_RECOVERY:     return "DNS unrecoverable error";
	case WSANO_DATA:         return "DNS no data";

	default:                 return "unknown error";
	}
#elif PLATFORM_POSIX
	switch (err)
	{
	case 0:                  return 0;

		/* 连接状态 */
	case SYSTEM_NET_CLOSED:  return "connection closed";
	case ECONNRESET:         return "connection reset";
	case ECONNREFUSED:       return "connection refused";
	case ECONNABORTED:       return "connection aborted";
	case ENOTCONN:           return "not connected";
	case EISCONN:            return "already connected";
	case EPIPE:              return "broken pipe";

		/* 超时/重试 */
	case ETIMEDOUT:          return "timed out";
	case EAGAIN:             return "would block";
	case EINPROGRESS:        return "operation in progress";
	case EALREADY:           return "operation already in progress";

		/* 网络可达性 */
	case ENETDOWN:           return "network is down";
	case ENETUNREACH:        return "network unreachable";
	case ENETRESET:          return "network dropped connection";
	case EHOSTDOWN:          return "host is down";
	case EHOSTUNREACH:       return "host unreachable";
	case ENONET:             return "machine is not on the network";
	case ENOLINK:            return "link has been severed";

		/* 地址相关 */
	case EADDRINUSE:         return "address already in use";
	case EADDRNOTAVAIL:      return "address not available";
	case EDESTADDRREQ:       return "destination address required";

		/* 参数/操作错误 */
	case SYSTEM_NET_EINVAL:  return "invalid argument";
	case EINVAL:             return "invalid argument";
	case EFAULT:             return "bad address";
	case EINTR:              return "interrupted";
	case EACCES:             return "permission denied";
	case EBADF:              return "bad file descriptor";
	case EMSGSIZE:           return "message too long";
	case EOPNOTSUPP:         return "operation not supported";
	case ENOPROTOOPT:        return "bad protocol option";
	case EPROTOTYPE:         return "protocol wrong type for socket";

		/* Socket 对象错误 */
	case ENOTSOCK:           return "not a socket";
	case EAFNOSUPPORT:       return "address family not supported";
	case EPROTONOSUPPORT:    return "protocol not supported";

		/* 资源不足 */
	case EMFILE:             return "too many open files";
	case ENOBUFS:            return "no buffer space";

	default:                 return "unknown error";
	}
#else
	return "Undef system net error";
#endif
}


#else


PORT_API bool_t system_net_init(void) { ; }
PORT_API bool_t system_net_clean(void) { ; }
PORT_API system_net_socket_t system_net_socket(qty_t domain, qty_t type, qty_t protocol) { ; }
PORT_API bool_t system_net_close(system_net_socket_t sock) { ; }
PORT_API bool_t system_net_connect(system_net_socket_t sock, const system_sock_addr_t addr, qty_t addrlen) { ; }
PORT_API bool_t system_net_bind(system_net_socket_t sock, const system_sock_addr_t addr, qty_t addrlen) { ; }
PORT_API bool_t system_net_listen(system_net_socket_t sock, qty_t backlog) { ; }
PORT_API system_net_socket_t system_net_accept(system_net_socket_t sock, system_sock_addr_t addr, qty_t* addrlen) { ; }
PORT_API qty_t system_net_recv(system_net_socket_t sock, void* buf, qty_t count, qty_t flags) { ; }
PORT_API qty_t system_net_send(system_net_socket_t sock, const void* buf, qty_t count, qty_t flags) { ; }
PORT_API bool_t system_net_get_local(system_net_socket_t sock, system_sock_addr_t addr, qty_t* addrlen) { ; }
PORT_API bool_t system_net_get_remote(system_net_socket_t sock, system_sock_addr_t addr, qty_t* addrlen) { ; }
PORT_API bool_t system_net_set_opt(system_net_socket_t sock, qty_t level, qty_t optname, const void* optval, qty_t optlen) { ; }
PORT_API bool_t system_net_get_opt(system_net_socket_t sock, qty_t level, qty_t optname, void* optval, qty_t* optlen) { ; }
PORT_API bool_t system_net_set_nonblock(system_net_socket_t sock, qty_t enable) { ; }
PORT_API const char* system_net_error(void) { ; }


#endif