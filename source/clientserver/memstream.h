#ifndef UDA_CLIENTSERVER_MAC_MEMSTREAM_H
#define UDA_CLIENTSERVER_MAC_MEMSTREAM_H

#include "export.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

LIBRARY_API FILE* open_memstream(char** cp, size_t* lenp);

#ifdef __cplusplus
}
#endif

#endif // UDA_CLIENTSERVER_MAC_MEMSTREAM_H
