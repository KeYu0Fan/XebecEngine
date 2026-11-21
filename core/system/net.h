#ifndef SYSTEM_NET_H
/* ----网络----
* 部分参数直接映射 { flags addr system_net_socket{domain type protocol} opt{...} }
* 
* system_sock_addr_t addr 地址空间
* qty_t addrlen 对应地址空间大小
* 
* system_net_init 在linux下行为是signal(SIGPIPE, SIG_IGN)
* system_net_clean 在linux下行为是 return 1
----网络---- */
#define SYSTEM_NET_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/config.h"
#include "core/system/STATIC/def.h"
EXTERN_C_BEGIN


typedef system_handle_h system_net_socket_t;
typedef void* system_sock_addr_t;

/* 初始化net */
PORT_API bool_t system_net_init(void);
/* 清理net */
PORT_API bool_t system_net_clean(void);

/* 创建端点 domain协议族 type通信协议 protocol具体协议 失败返回-1 */
PORT_API system_net_socket_t system_net_socket(qty_t domain, qty_t type, qty_t protocol);
/* 销毁端点 */
PORT_API bool_t system_net_close(system_net_socket_t sock);

/* 连接远端 客户端 */
PORT_API bool_t system_net_connect(system_net_socket_t sock, const system_sock_addr_t addr, qty_t addrlen);
/* 固定端口 服务端 */ 
PORT_API bool_t system_net_bind(system_net_socket_t sock, const system_sock_addr_t addr, qty_t addrlen);
/* 切换服务状态 主动->被动 等待连接 backlog为连接队列最大长度 服务端 */
PORT_API bool_t system_net_listen(system_net_socket_t sock, qty_t backlog);
/* 接受连接 失败返回-1 服务端 */
PORT_API system_net_socket_t system_net_accept(system_net_socket_t sock, system_sock_addr_t addr, qty_t* addrlen);

/* 读端点 返回实际字节数 0表示对端关闭或异常 需调用error区分 */
PORT_API qty_t system_net_recv(system_net_socket_t sock, void* buf, qty_t count, qty_t flags);
/* 写端点 返回实际字节数 0表示对端关闭或异常 需调用error区分 */
PORT_API qty_t system_net_send(system_net_socket_t sock, const void* buf, qty_t count, qty_t flags);

/* 获取本地地址 */
PORT_API bool_t system_net_get_local(system_net_socket_t sock, system_sock_addr_t addr, qty_t* addrlen);
/* 获取远端地址 */
PORT_API bool_t system_net_get_remote(system_net_socket_t sock, system_sock_addr_t addr, qty_t* addrlen);

/* 设置 socket 选项 */
PORT_API bool_t system_net_set_opt(system_net_socket_t sock, qty_t level, qty_t optname, const void* optval, qty_t optlen);
/* 获取 socket 选项 */
PORT_API bool_t system_net_get_opt(system_net_socket_t sock, qty_t level, qty_t optname, void* optval, qty_t* optlen);

/* 设置读写模式 1不阻塞 0阻塞 */
PORT_API bool_t system_net_set_nonblock(system_net_socket_t sock, qty_t enable);
/* 获取最后一个错误 无错误返回空指针 */
PORT_API const char* system_net_error(void);


EXTERN_C_END
#endif