#ifndef SYSTEM_PIPE_H
/* ----匿名管道----
* system_fifo_new system_fifo_del 在windows为空实现返回1
*
* system_fifo_server system_fifo_client
* windows下需要 L"\\\\.\\pipe\\"前缀 内部已自动处理 请勿传输带有不完全前缀的名字
*
* system_pipe_set_nonblock windows只有命名管道支持
*
* system_pipe_wait mode在windows下不使用
----匿名管道---- */
#define SYSTEM_PIPE_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/config.h"
#include "core/system/STATIC/def.h"
EXTERN_C_BEGIN


typedef system_handle_h system_pipe_t;

/* PIPE 匿名管道 */

/* 创建匿名管道句柄 */
PORT_API bool_t system_pipe_open(system_pipe_t* fpR, system_pipe_t* fpW);
/* 关闭管道句柄 */
PORT_API bool_t system_pipe_close(system_pipe_t fp);
/* 读管道 */
PORT_API bool_t system_pipe_read(system_pipe_t fp, void* buf, qty_t count, qty_t* buf_size);
/* 写管道 */
PORT_API bool_t system_pipe_write(system_pipe_t fp, const void* buf, qty_t count, qty_t* buf_size);
/* 设置非阻塞模式 enable = { 1=开启非阻塞 0=关闭非阻塞 } */
PORT_API bool_t system_pipe_set_nonblock(system_pipe_t fp, qty_t enable);
/* 等待管道可读/可写 支持超时毫秒 mode ={ 0=读 1=写 2=读或写 } 返回 1=事件发生 0=超时 -1=错误 */
PORT_API bool_t system_pipe_wait(system_pipe_t fp,qty_t mode, qty_t ms);

/* FIFO 命名管道 */

/* 创建命名管道句柄 */
PORT_API bool_t system_fifo_new(const char* path);
/* 销毁命名管道句柄 */
PORT_API bool_t system_fifo_del(const char* path);
/* 开启服务端 等待客户端 */
PORT_API system_pipe_t system_fifo_server(const char* path);
/* 开启客户端 连接服务端 */
PORT_API system_pipe_t system_fifo_client(const char* path);


EXTERN_C_END
#endif