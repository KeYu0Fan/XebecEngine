#ifndef CORE_FORMAT_H
#define CORE_FORMAT_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
EXTERN_C_BEGIN


PORT_API char* format_dtos(qty_t value, void*buff, qty_t size);
PORT_API char* format_ftos(real_t value, void*buff, qty_t size);
PORT_API char* format_mtos(void* value, qty_t len, void* buff, qty_t size);


EXTERN_C_END
#endif