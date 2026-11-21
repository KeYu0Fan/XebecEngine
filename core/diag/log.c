#include "log.h"
#if COM_USE(CONFIG_LOG_LOG)


#include "core/struct/string.h"
#include "core/struct/buffer.h"
#include "core/system/thread.h"
#include "core/system/mmap.h"
#include "core/system/time.h"


PORT_HIDE static bool_t init = 0;
PORT_HIDE static struct system_timer_t* timer = 0;
PORT_HIDE static system_mutex_t mutex = { 0 };
PORT_HIDE static void(*call)(void* user, char* data, qty_t size) = 0;
PORT_HIDE static struct buffer buffer = { 0 };
PORT_HIDE static char data[32768] = { 0 };
PORT_HIDE static void PLATFORM_CALL log_call(void* user)
{
	char _buffer_[sizeof(data)];
	system_mutex_lock(&mutex);
	qty_t _size_ = buffer_read(&buffer, _buffer_, sizeof(_buffer_));
	system_mutex_unlock(&mutex);
	if (_size_ == 0) return;
	call(user, _buffer_, _size_);
}

/* 初始化日志 */
PORT_API bool_t log_init(qty_t interval_ms, qty_t busy_threshold_ms, void(*call)(void* user, char* data, qty_t size), void* user)
{
	COM_RETURN(call == 0, 0);
	if (init) return 0;
	
	timer = system_timer_new(interval_ms, busy_threshold_ms, 0, log_call, user);
	if (timer == 0) return 0;
	if (system_mutex_init(&mutex) == 0) { system_timer_del(timer); timer = 0; return 0; }
	buffer_init((void*)&buffer, 1);
	buffer_ext(&buffer, 0, data, sizeof(data));
	system_timer_run(timer);
	init = 1;
	return 1;
}
/* 写入信息 未有长度时计算 */
PORT_API void log_write(char* data, qty_t len)
{
	COM_RETURN(data == 0, );
	if (init == 0) return;
	system_mutex_lock(&mutex);
	buffer_write(&buffer, data, len ? len : str_len(data, (qty_t)-1));
	system_mutex_unlock(&mutex);
}
/* 提前回调 */
PORT_API void log_flush(void)
{
	if (init == 0) return;
	void* user = 0;
	if (timer)
		user = timer->arg;
	log_call(user);
}
/* 销毁日志 */
PORT_API void log_del(void)
{
	if (init == 0) return;
	init = 0;
	void* user = 0;
	if (timer)
		user = timer->arg;
	if (timer) { system_timer_del(timer); timer = 0; }
	log_call(user);
	system_mutex_del(&mutex);
}


#else


PORT_API bool_t log_init(qty_t interval_ms, qty_t busy_threshold_ms, void(*call)(void* user, char* data, qty_t size), void* user) { ; }
PORT_API void log_write(char* data, qty_t len) { ; }
PORT_API void log_flush(void) { ; }
PORT_API void log_del(void) { ; }


#endif