#ifndef UDA_STRUCTURES_XDRUSERDEFINEDDATA_H
#define UDA_STRUCTURES_XDRUSERDEFINEDDATA_H

#include <rpc/rpc.h>

#include <structures/genStructs.h>
#include <clientserver/export.h>

#ifdef __cplusplus
extern "C" {
#endif

LIBRARY_API int xdrUserDefinedData(XDR* xdrs, LOGMALLOCLIST* logmalloclist, USERDEFINEDTYPELIST* userdefinedtypelist,
                                   USERDEFINEDTYPE* userdefinedtype, void** data, int datacount, int structRank,
                                   int* structShape,
                                   int index, NTREE** NTree, int protocolVersion, int malloc_source);

#ifdef __cplusplus
}
#endif

#endif // UDA_STRUCTURES_XDRUSERDEFINEDDATA_H
