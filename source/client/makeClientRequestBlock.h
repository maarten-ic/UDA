#ifndef IDAM_CLIENT_MAKECLIENTREQUESTBLOCK_H
#define IDAM_CLIENT_MAKECLIENTREQUESTBLOCK_H

#include <clientserver/udaStructs.h>

#ifdef __cplusplus
extern "C" {
#endif

int makeClientRequestBlock(const char *data_object, const char *data_source, REQUEST_BLOCK *request_block);

#ifdef __cplusplus
}
#endif

#endif // IDAM_CLIENT_MAKECLIENTREQUESTBLOCK_H
