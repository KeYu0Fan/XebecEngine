#include "error.h"
#if COM_USE(CONFIG_CORE_ERROR)


/*
 * 将错误码转译为可读的字符串
 * 静态存储 无需销毁
 */
PORT_API const char* error_to_string(err_t error)
{
	switch (error)
	{
	case ERROR_UNKNOWN:
	default:
		return "Error: Unknown error.\n";
	case ERROR_NULL:
		return "Error: No error.\n";
	case ERROR_PARAMTER:
		return "Error: Exist illegal paramter.\n";
	case ERROR_SYNTAX:
		return "Error: Syntax as error.\n";

	case ERROR_UNFULFILL:
		return "Error: Function no fulfill.\n";
	case ERROR_MEMORY_LACK:
		return "Error: No malloc memory.\n";
	case ERROR_MEMORY_OVER:
		return "Error: Exist memory over.\n";
	}
}


#else


PORT_API const char* error_to_string(err_t error) { ; }


#endif