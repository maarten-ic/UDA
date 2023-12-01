#pragma once

#ifndef UDA_SERVER_SERVERSUBSETDATA_H
#define UDA_SERVER_SERVERSUBSETDATA_H

#include <clientserver/parseXML.h>
#include "udaStructs.h"
#include "genStructs.h"
#include "export.h"
#include "pluginStructs.h"

#ifdef __cplusplus
extern "C" {
#endif

LIBRARY_API int serverSubsetData(DATA_BLOCK *data_block, const ACTION& action, LOGMALLOCLIST* logmalloclist);
LIBRARY_API int serverParseServerSide(REQUEST_DATA *request_block, ACTIONS *actions_serverside, const PLUGINLIST* plugin_list);

#ifdef __cplusplus
}
#endif

#endif // UDA_SERVER_SERVERSUBSETDATA_H
