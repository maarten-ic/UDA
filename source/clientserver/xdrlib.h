#ifndef UDA_CLIENTSERVER_XDRLIB_H
#define UDA_CLIENTSERVER_XDRLIB_H

#include "udaStructs.h"
#include "export.h"

#include <rpc/types.h>
#include <rpc/xdr.h>

//-------------------------------------------------------
// XDR Stream Directions

#define XDR_SEND        0
#define XDR_RECEIVE     1
#define XDR_FREE_HEAP   2

#ifdef __APPLE__
#  define xdr_uint64_t xdr_u_int64_t
#endif

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------
// Test version's type passing capability

LIBRARY_API int protocolVersionTypeTest(int protocolVersion, int type);

LIBRARY_API int wrap_string(XDR* xdrs, char* sp);

LIBRARY_API int WrapXDRString(XDR* xdrs, const char* sp, int maxlen);

LIBRARY_API bool_t xdr_meta(XDR* xdrs, DATA_BLOCK* str);
LIBRARY_API bool_t xdr_securityBlock1(XDR* xdrs, SECURITY_BLOCK* str);
LIBRARY_API bool_t xdr_securityBlock2(XDR* xdrs, SECURITY_BLOCK* str);
LIBRARY_API bool_t xdr_client(XDR* xdrs, CLIENT_BLOCK* str, int protocolVersion);
LIBRARY_API bool_t xdr_server(XDR* xdrs, SERVER_BLOCK* str);
LIBRARY_API bool_t xdr_server1(XDR* xdrs, SERVER_BLOCK* str, int protocolVersion);
LIBRARY_API bool_t xdr_server2(XDR* xdrs, SERVER_BLOCK* str);
LIBRARY_API bool_t xdr_request(XDR* xdrs, REQUEST_BLOCK* str, int protocolVersion);
LIBRARY_API bool_t xdr_request_data(XDR* xdrs, REQUEST_DATA* str, int protocolVersion);
LIBRARY_API bool_t xdr_putdatablocklist_block(XDR* xdrs, PUTDATA_BLOCK_LIST* str);
LIBRARY_API bool_t xdr_putdata_block1(XDR* xdrs, PUTDATA_BLOCK* str);
LIBRARY_API bool_t xdr_putdata_block2(XDR* xdrs, PUTDATA_BLOCK* str);
LIBRARY_API bool_t xdr_data_block_list(XDR* xdrs, DATA_BLOCK_LIST* str, int protocolVersion);
LIBRARY_API bool_t xdr_data_block1(XDR* xdrs, DATA_BLOCK* str, int protocolVersion);
LIBRARY_API bool_t xdr_data_block2(XDR* xdrs, DATA_BLOCK* str);
LIBRARY_API bool_t xdr_data_block3(XDR* xdrs, DATA_BLOCK* str);
LIBRARY_API bool_t xdr_data_block4(XDR* xdrs, DATA_BLOCK* str);
LIBRARY_API bool_t xdr_data_dim1(XDR* xdrs, DATA_BLOCK* str);
LIBRARY_API bool_t xdr_data_dim2(XDR* xdrs, DATA_BLOCK* str);
LIBRARY_API bool_t xdr_data_dim3(XDR* xdrs, DATA_BLOCK* str);
LIBRARY_API bool_t xdr_data_dim4(XDR* xdrs, DATA_BLOCK* str);
LIBRARY_API bool_t xdr_data_object1(XDR* xdrs, DATA_OBJECT* str);
LIBRARY_API bool_t xdr_data_object2(XDR* xdrs, DATA_OBJECT* str);

//-----------------------------------------------------------------------
// From DATA_SYSTEM Table
LIBRARY_API bool_t xdr_data_system(XDR* xdrs, DATA_SYSTEM* str);

//-----------------------------------------------------------------------
// From SYSTEM_CONFIG Table
LIBRARY_API bool_t xdr_system_config(XDR* xdrs, SYSTEM_CONFIG* str);

//-----------------------------------------------------------------------
// From DATA_SOURCE Table
LIBRARY_API bool_t xdr_data_source(XDR* xdrs, DATA_SOURCE* str);

//-----------------------------------------------------------------------
// From SIGNAL Table
LIBRARY_API bool_t xdr_signal(XDR* xdrs, SIGNAL* str);

//-----------------------------------------------------------------------
// From SIGNAL_DESC Table
LIBRARY_API bool_t xdr_signal_desc(XDR* xdrs, SIGNAL_DESC* str);

#ifdef __cplusplus
}
#endif

#endif // UDA_CLIENTSERVER_XDRLIB_H

