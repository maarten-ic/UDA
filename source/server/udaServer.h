#pragma once

#ifndef UDA_SERVER_UDASERVER_H
#  define UDA_SERVER_UDASERVER_H

#  include "export.h"
#  include "genStructs.h"
#  include "pluginStructs.h"
#  include <clientserver/socketStructs.h>

LIBRARY_API int udaServer(CLIENT_BLOCK client_block);

LIBRARY_API int fatServer(CLIENT_BLOCK client_block, SERVER_BLOCK* server_block, REQUEST_BLOCK* request_block0,
                          DATA_BLOCK_LIST* data_blocks0);

#endif // UDA_SERVER_UDASERVER_H
