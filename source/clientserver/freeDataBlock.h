// Free Heap Memory
//
//-----------------------------------------------------------------------------

#ifndef IDAM_CLIENTSERVER_FREEDATABLOCK_H
#define IDAM_CLIENTSERVER_FREEDATABLOCK_H

#include "idamStructs.h"

// Forward declarations
struct LOGMALLOCLIST;
struct USERDEFINEDTYPELIST;

void freeIdamDataBlock(DATA_BLOCK *data_block);
void freeMallocLogList(struct LOGMALLOCLIST *str);
void freeUserDefinedTypeList(struct USERDEFINEDTYPELIST *userdefinedtypelist);
void freeDataBlock(DATA_BLOCK *data_block);
void freeReducedDataBlock(DATA_BLOCK *data_block);

#endif // IDAM_CLIENTSERVER_FREEDATABLOCK_H

