#pragma once

#include <cstdio> // this must be included before rpc.h
#include <rpc/rpc.h>
#include <tuple>

#include "structures/genStructs.h"

#ifdef FATCLIENT
#  define protocolXML protocolXMLFat
#endif

namespace uda::client_server
{

struct IoData;

using CreateXDRStreams = std::pair<XDR*, XDR*> (*)(uda::client_server::IoData*);

int protocol_xml(XDR* xdrs, int protocol_id, int direction, int* token, LOGMALLOCLIST* logmalloclist,
                USERDEFINEDTYPELIST* userdefinedtypelist, void* str, int protocolVersion,
                LOGSTRUCTLIST* log_struct_list, IoData* io_data, unsigned int private_flags, int malloc_source,
                CreateXDRStreams create_xdr_streams);

} // namespace uda::client_server
