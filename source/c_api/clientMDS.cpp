#include <uda/client.h>

#include "client/startup.h"
#include "clientserver/initStructs.h"
#include "logging/logging.h"

using namespace uda::client_server;
using namespace uda::client;
using namespace uda::logging;

/**
 * Reads the Requested Data
 *
 * @param server MDS+ Server Name
 * @param tree MDS+ Tree Name or Generic)
 * @param node MDS+ Node Name
 * @param treenum MDS+ Tree Number
 * @return
 */
int udaClientMDS(const char* server, const char* tree, const char* node, int treenum)
{
    //-------------------------------------------------------------------------
    // Open the Logs

    CLIENT_FLAGS* client_flags = udaClientFlags();
    static bool reopen_logs = true;

    if (udaStartup(0, client_flags, &reopen_logs) != 0) {
        return PROBLEM_OPENING_LOGS;
    }

    //-------------------------------------------------------------------------
    // Passed Args

    REQUEST_BLOCK request_block;
    init_request_block(&request_block);

    request_block.num_requests = 1;
    request_block.requests = (REQUEST_DATA*)malloc(sizeof(REQUEST_DATA));
    auto request = &request_block.requests[0];
    init_request_data(request);

    request->request = REQUEST_READ_MDS;
    request->exp_number = treenum;

    strcpy(request->file, tree);
    strcpy(request->signal, node);
    strcpy(request->server, server);

    UDA_LOG(UDA_LOG_DEBUG, "Routine: ClientMDS\n");
    UDA_LOG(UDA_LOG_DEBUG, "Server          %s\n", request->server);
    UDA_LOG(UDA_LOG_DEBUG, "Tree           %s\n", request->file);
    UDA_LOG(UDA_LOG_DEBUG, "Node           %s\n", request->signal);
    UDA_LOG(UDA_LOG_DEBUG, "Tree Number       %d\n", request->exp_number);

    //-------------------------------------------------------------------------
    // Fetch Data

    int handle;
    int err = idamClient(&request_block, &handle);
    if (err < 0) {
        handle = err;
    }

    return handle;
}
