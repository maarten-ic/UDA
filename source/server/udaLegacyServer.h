#pragma once

#include "clientserver/socketStructs.h"
#include "clientserver/udaStructs.h"
#include "serverPlugin.h"
#include "structures/genStructs.h"
#include "uda/types.h"

namespace uda::config
{
class Config;
}

namespace uda::server
{

/**
 * UDA Legacy Data Server (protocol versions <= 6)
 */
int legacyServer(const config::Config& config, client_server::ClientBlock client_block, const plugins::PluginList* pluginlist,
                 structures::LogMallocList* logmalloclist,
                 structures::UserDefinedTypeList* userdefinedtypelist, client_server::SOCKETLIST* socket_list,
                 int protocolVersion, XDR* server_input, XDR* server_output, unsigned int private_flags,
                 int malloc_source);

} // namespace uda::server
