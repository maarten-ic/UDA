#include "cache.h"

#include <clientserver/xdrlib.h>
#include <clientserver/protocol.h>
#include <clientserver/initStructs.h>

void writeCacheData(FILE* fp, LOGMALLOCLIST* logmalloclist, USERDEFINEDTYPELIST* userdefinedtypelist,
                    const DATA_BLOCK* data_block, int protocolVersion, NTREE* full_ntree,
                    LOGSTRUCTLIST* log_struct_list, unsigned int private_flags, int malloc_source)
{
    XDR xdrs;
    xdrstdio_create(&xdrs, fp, XDR_ENCODE);

    int token;
    DATA_BLOCK_LIST data_block_list;
    data_block_list.count = 1;
    data_block_list.data = (DATA_BLOCK*)data_block;
    protocol2(&xdrs, PROTOCOL_DATA_BLOCK_LIST, XDR_SEND, &token, logmalloclist, userdefinedtypelist,
              &data_block_list, protocolVersion, full_ntree, log_struct_list, private_flags, malloc_source);

    xdr_destroy(&xdrs);     // Destroy before the  file otherwise a segmentation error occurs
}

DATA_BLOCK*
readCacheData(FILE* fp, LOGMALLOCLIST* logmalloclist, USERDEFINEDTYPELIST* userdefinedtypelist, int protocolVersion,
              NTREE* full_ntree, LOGSTRUCTLIST* log_struct_list, unsigned int private_flags, int malloc_source)
{
    XDR xdrs;
    xdrstdio_create(&xdrs, fp, XDR_DECODE);

    DATA_BLOCK_LIST data_block_list;
    initDataBlockList(&data_block_list);

    int token;
    protocol2(&xdrs, PROTOCOL_DATA_BLOCK_LIST, XDR_RECEIVE, &token, logmalloclist, userdefinedtypelist,
              &data_block_list, protocolVersion, full_ntree, log_struct_list, private_flags, malloc_source);

    xdr_destroy(&xdrs);     // Destroy before the  file otherwise a segmentation error occurs

    return data_block_list.data;
}