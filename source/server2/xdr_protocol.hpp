#pragma once

#include <rpc/rpc.h>
#include <vector>

#include "cache/memcache.hpp"
#include "clientserver/protocol.h"
#include "clientserver/udaDefines.h"
#include "clientserver/udaStructs.h"
#include "server_environment.hpp"
#include "structures/genStructs.h"

namespace uda::server
{

struct IoData : client_server::IoData {
    int* server_tot_block_time;
    int* server_timeout;
};

struct MetadataBlock;

class XdrProtocol
{
  public:
    XdrProtocol();
    void create();
    void set_version(int protocol_version);

    int read_client_block(client_server::ClientBlock* client_block,
                          structures::LogMallocList* log_malloc_list,
                          structures::UserDefinedTypeList* user_defined_type_list);
    int recv_request_block(client_server::RequestBlock* request_block,
                           structures::LogMallocList* log_malloc_list,
                           structures::UserDefinedTypeList* user_defined_type_list);
    int send_server_block(client_server::ServerBlock server_block, structures::LogMallocList* log_malloc_list,
                          structures::UserDefinedTypeList* user_defined_type_list);
    int recv_putdata_block_list(client_server::PutDataBlockList* putdata_block_list,
                                structures::LogMallocList* log_malloc_list,
                                structures::UserDefinedTypeList* user_defined_type_list);

    int flush();
    int eof();

    client_server::DataBlock* read_from_cache(config::Config& config, cache::UdaCache* cache,
                                                   client_server::RequestData* request,
                                                   structures::LogMallocList* log_malloc_list,
                                                   structures::UserDefinedTypeList* user_defined_type_list);
    void write_to_cache(config::Config& config, cache::UdaCache* cache, client_server::RequestData* request,
                        client_server::DataBlock* data_block,
                        structures::LogMallocList* log_malloc_list,
                        structures::UserDefinedTypeList* user_defined_type_list);

    int send_meta_data(MetadataBlock& metadata_block, structures::LogMallocList* log_malloc_list,
                       structures::UserDefinedTypeList* user_defined_type_list);
    int send_data_blocks(const std::vector<client_server::DataBlock>& data_blocks,
                         structures::LogMallocList* log_malloc_list,
                         structures::UserDefinedTypeList* user_defined_type_list);
    int send_hierachical_data(const client_server::DataBlock& data_block,
                              structures::LogMallocList* log_malloc_list,
                              structures::UserDefinedTypeList* user_defined_type_list);
    int recv_client_block(client_server::ServerBlock& server_block, client_server::ClientBlock* client_block,
                          bool* fatal, int server_tot_block_time, const int* server_timeout,
                          structures::LogMallocList* log_malloc_list,
                          structures::UserDefinedTypeList* user_defined_type_list);

  private:
    int _protocol_version = 8;
    XDR _server_input;
    XDR _server_output;
    int _server_tot_block_time;
    int _server_timeout;
    server::IoData _io_data;
    structures::LogStructList _log_struct_list;
    int _malloc_source;
    int _private_flags;

    void create_streams();
};

} // namespace uda::server
