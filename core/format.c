#include "format.h"
#if COM_USE(CONFIG_CORE_FORMAT)


#include "core/struct/string.h"

PORT_API char* format_dtos(qty_t value, void* buff, qty_t size)
{
	COM_RETURN(value == 0 || buff == 0 || size == 0, 0);

}
PORT_API char* format_ftos(real_t value, void* buff, qty_t size)
{
	COM_RETURN(value == 0 || buff == 0 || size == 0, 0);

}
PORT_API char* format_mtos(void* value, qty_t len, void* buff, qty_t size)
{
	COM_RETURN(value == 0 || len == 0 || buff == 0 || size == 0, 0);
}


#else


PORT_API char* format_dtos(qty_t value, void* buff, qty_t size) { ; }
PORT_API char* format_ftos(real_t value, void* buff, qty_t size) { ; }
PORT_API char* format_mtos(void* value, qty_t len, void* buff, qty_t size) { ; }


#endif