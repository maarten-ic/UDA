#pragma once

#include "structures/genStructs.h"

#include <cstdio>
#include <rpc/rpc.h>

#ifdef FATCLIENT
#  define protocolXML2 protocolXML2Fat
#endif

namespace uda::client_server
{

int protocol_xml2(XDR* xdrs, int protocol_id, int direction, int* token, LOGMALLOCLIST* logmalloclist,
                 USERDEFINEDTYPELIST* userdefinedtypelist, void* str, int protocolVersion,
                 LOGSTRUCTLIST* log_struct_list, unsigned int private_flags, int malloc_source);

int xdr_user_defined_type_data(XDR* xdrs, LOGMALLOCLIST* logmalloclist, USERDEFINEDTYPELIST* userdefinedtypelist,
                           USERDEFINEDTYPE* userdefinedtype, void** data, int protocolVersion, bool xdr_stdio_flag,
                           LOGSTRUCTLIST* log_struct_list, int malloc_source);

bool_t xdr_user_defined_type_list(XDR* xdrs, USERDEFINEDTYPELIST* str, bool xdr_stdio_flag);

} // namespace uda::client_server
