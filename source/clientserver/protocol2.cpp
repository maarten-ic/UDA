/*---------------------------------------------------------------
 * Client - Server Conversation Protocol
 *
 * Args:    xdrs        XDR Stream
 *
 *    protocol_id    Client/Server Conversation item: Data Exchange context
 *    direction    Send (0) or Receive (1) or Free (2)
 *    token        current error condition or next protocol or .... exchange token
 *
 *    str        Information Structure depending on the protocol id ....
 *
 *    2    data_block    Data read from the external Source or Data to be written
 *                to an external source
 *    4    data_system    Database Data_Dystem table record
 *    5    system_config    Database System_Config table record
 *    6    data_source    Database Data_Source table record
 *    7    signal        Database Signal table record
 *    8    signal_desc    Database Signal_Desc table record
 *
 * Returns: error code if failure, otherwise 0
 *
 *--------------------------------------------------------------*/

#include "protocol.h"

#include <cstdlib>

#include "logging/logging.h"
#include <uda/types.h>

#include "allocData.h"
#include "compressDim.h"
#include "initStructs.h"
#include "printStructs.h"
#include "protocolXML2.h"
#include "xdrlib.h"

#include "errorLog.h"
#include "protocolXML2Put.h"
#include "udaErrors.h"

using namespace uda::client_server;
using namespace uda::logging;
using namespace uda::structures;

static int handle_request_block(XDR* xdrs, XDRStreamDirection direction, const void* str, int protocolVersion);
static int handle_data_block(XDR* xdrs, XDRStreamDirection direction, const void* str, int protocolVersion);
static int handle_data_block_list(XDR* xdrs, XDRStreamDirection direction, const void* str, int protocolVersion);
static int handle_putdata_block_list(XDR* xdrs, XDRStreamDirection direction, ProtocolId* token, LogMallocList* logmalloclist,
                                     UserDefinedTypeList* userdefinedtypelist, const void* str, int protocolVersion,
                                     LogStructList* log_struct_list, unsigned int private_flags, int malloc_source);
static int handle_next_protocol(XDR* xdrs, XDRStreamDirection direction, ProtocolId* token);
static int handle_data_system(XDR* xdrs, XDRStreamDirection direction, const void* str);
static int handle_system_config(XDR* xdrs, XDRStreamDirection direction, const void* str);
static int handle_data_source(XDR* xdrs, XDRStreamDirection direction, const void* str);
static int handle_signal(XDR* xdrs, XDRStreamDirection direction, const void* str);
static int handle_signal_desc(XDR* xdrs, XDRStreamDirection direction, const void* str);
static int handle_client_block(XDR* xdrs, XDRStreamDirection direction, const void* str, int protocolVersion);
static int handle_server_block(XDR* xdrs, XDRStreamDirection direction, const void* str, int protocolVersion);
static int handle_dataobject(XDR* xdrs, XDRStreamDirection direction, const void* str);
static int handle_dataobject_file(XDRStreamDirection direction, const void* str);

#ifdef SECURITYENABLED
static int handle_security_block(XDR* xdrs, XDRStreamDirection direction, const void* str);
#endif

int uda::client_server::protocol2(XDR* xdrs, ProtocolId protocol_id, XDRStreamDirection direction, ProtocolId* token, LogMallocList* logmalloclist,
                                  UserDefinedTypeList* userdefinedtypelist, void* str, int protocolVersion,
                                  LogStructList* log_struct_list, unsigned int private_flags, int malloc_source)
{
    int err = 0;

    switch (protocol_id) {
        case ProtocolId::RequestBlock:
            err = handle_request_block(xdrs, direction, str, protocolVersion);
            break;
        case ProtocolId::DataBlockList:
            err = handle_data_block_list(xdrs, direction, str, protocolVersion);
            break;
        case ProtocolId::PutdataBlockList:
            err = handle_putdata_block_list(xdrs, direction, token, logmalloclist, userdefinedtypelist, str,
                                            protocolVersion, log_struct_list, private_flags, malloc_source);
            break;
        case ProtocolId::NextProtocol:
            err = handle_next_protocol(xdrs, direction, token);
            break;
        case ProtocolId::DataSystem:
            err = handle_data_system(xdrs, direction, str);
            break;
        case ProtocolId::SystemConfig:
            err = handle_system_config(xdrs, direction, str);
            break;
        case ProtocolId::DataSource:
            err = handle_data_source(xdrs, direction, str);
            break;
        case ProtocolId::Signal:
            err = handle_signal(xdrs, direction, str);
            break;
        case ProtocolId::SignalDesc:
            err = handle_signal_desc(xdrs, direction, str);
            break;
#ifdef SECURITYENABLED
        case ProtocolId::SecurityBlock:
            err = handle_security_block(xdrs, direction, str);
            break;
#endif
        case ProtocolId::ClientBlock:
            err = handle_client_block(xdrs, direction, str, protocolVersion);
            break;
        case ProtocolId::ServerBlock:
            err = handle_server_block(xdrs, direction, str, protocolVersion);
            break;
        case ProtocolId::DataObject:
            err = handle_dataobject(xdrs, direction, str);
            break;
        case ProtocolId::DataObjectFile:
            err = handle_dataobject_file(direction, str);

            break;
        default:
            if (protocol_id > ProtocolId::OpaqueStart && protocol_id < ProtocolId::OpaqueStop) {
                err = protocol_xml2(xdrs, protocol_id, direction, token, logmalloclist, userdefinedtypelist, str,
                                    protocolVersion, log_struct_list, private_flags, malloc_source);
            }
    }

    return err;
}

#ifdef SECURITYENABLED
static int handle_security_block(XDR* xdrs, XDRStreamDirection direction, const void* str)
{
    int err = 0;
    ClientBlock* client_block = (ClientBlock*)str;
    SecurityBlock* security_block = &(client_block->securityBlock);

    switch (direction) {
        case XDRStreamDirection::Receive:
            if (!xdr_security_block1(xdrs, security_block)) {
                err = UDA_PROTOCOL_ERROR_23;
                break;
            }
            break;

        case XDRStreamDirection::Send:
            if (!xdr_security_block1(xdrs, security_block)) {
                err = UDA_PROTOCOL_ERROR_23;
                break;
            }
            break;

        case XDRStreamDirection::FreeHeap:
            break;

        default:
            err = UDA_PROTOCOL_ERROR_4;
            break;
    }

    // Allocate heap

    if (security_block->client_ciphertextLength > 0) {
        security_block->client_ciphertext =
            (unsigned char*)malloc(security_block->client_ciphertextLength * sizeof(unsigned char));
    }
    if (security_block->client2_ciphertextLength > 0) {
        security_block->client2_ciphertext =
            (unsigned char*)malloc(security_block->client2_ciphertextLength * sizeof(unsigned char));
    }
    if (security_block->server_ciphertextLength > 0) {
        security_block->server_ciphertext =
            (unsigned char*)malloc(security_block->server_ciphertextLength * sizeof(unsigned char));
    }
    if (security_block->client_X509Length > 0) {
        security_block->client_X509 = (unsigned char*)malloc(security_block->client_X509Length * sizeof(unsigned char));
    }
    if (security_block->client2_X509Length > 0) {
        security_block->client2_X509 =
            (unsigned char*)malloc(security_block->client2_X509Length * sizeof(unsigned char));
    }

    switch (direction) {
        case XDRStreamDirection::Receive:
            if (!xdr_security_block2(xdrs, security_block)) {
                err = UDA_PROTOCOL_ERROR_24;
                break;
            }
            break;

        case XDRStreamDirection::Send:
            if (!xdr_security_block2(xdrs, security_block)) {
                err = UDA_PROTOCOL_ERROR_24;
                break;
            }
            break;

        case XDRStreamDirection::FreeHeap:
            break;

        default:
            err = UDA_PROTOCOL_ERROR_4;
            break;
    }
    return err;
}
#endif // SECURITYENABLED

static int handle_dataobject_file(XDRStreamDirection direction, const void* str)
{
    int err = 0;

    switch (direction) {
        case XDRStreamDirection::Receive:
            break;

        case XDRStreamDirection::Send:
            break;

        default:
            err = UDA_PROTOCOL_ERROR_4;
            break;
    }

    return err;
}

static int handle_dataobject(XDR* xdrs, XDRStreamDirection direction, const void* str)
{
    int err = 0;
    auto data_object = (DataObject*)str;

    switch (direction) {
        case XDRStreamDirection::Receive:
            if (!xdr_data_object1(xdrs, data_object)) { // Storage requirements
                err = UDA_PROTOCOL_ERROR_22;
                break;
            }
            if (data_object->objectSize > 0) {
                data_object->object = (char*)malloc(data_object->objectSize * sizeof(char));
            }
            if (data_object->hashLength > 0) {
                data_object->md = (char*)malloc(data_object->hashLength * sizeof(char));
            }
            if (!xdr_data_object2(xdrs, data_object)) {
                err = UDA_PROTOCOL_ERROR_22;
                break;
            }
            break;

        case XDRStreamDirection::Send:

            if (!xdr_data_object1(xdrs, data_object)) {
                err = UDA_PROTOCOL_ERROR_22;
                break;
            }
            if (!xdr_data_object2(xdrs, data_object)) {
                err = UDA_PROTOCOL_ERROR_22;
                break;
            }
            break;

        default:
            err = UDA_PROTOCOL_ERROR_4;
            break;
    }
    return err;
}

static int handle_server_block(XDR* xdrs, XDRStreamDirection direction, const void* str, int protocolVersion)
{
    int err = 0;
    auto server_block = (ServerBlock*)str;

    switch (direction) {
        case XDRStreamDirection::Receive:
            close_error(); // Free Heap associated with Previous Data Access

            if (!xdr_server1(xdrs, server_block, protocolVersion)) {
                err = UDA_PROTOCOL_ERROR_22;
                break;
            }

            if (server_block->idamerrorstack.nerrors > 0) { // No Data to Receive?

                server_block->idamerrorstack.idamerror =
                    (UdaError*)malloc(server_block->idamerrorstack.nerrors * sizeof(UdaError));
                init_error_records(&server_block->idamerrorstack);

                if (!xdr_server2(xdrs, server_block)) {
                    err = UDA_PROTOCOL_ERROR_22;
                    break;
                }
            }

            break;

        case XDRStreamDirection::Send:
            if (!xdr_server1(xdrs, server_block, protocolVersion)) {
                err = UDA_PROTOCOL_ERROR_22;
                break;
            }

            if (server_block->idamerrorstack.nerrors > 0) { // No Data to Send?
                if (!xdr_server2(xdrs, server_block)) {
                    err = UDA_PROTOCOL_ERROR_22;
                    break;
                }
            }

            break;

        case XDRStreamDirection::FreeHeap:
            break;

        default:
            err = UDA_PROTOCOL_ERROR_4;
            break;
    }
    return err;
}

static int handle_client_block(XDR* xdrs, XDRStreamDirection direction, const void* str, int protocolVersion)
{
    int err = 0;
    auto client_block = (ClientBlock*)str;

    switch (direction) {
        case XDRStreamDirection::Receive:
            if (!xdr_client(xdrs, client_block, protocolVersion)) {
                err = UDA_PROTOCOL_ERROR_20;
                break;
            }
            break;

        case XDRStreamDirection::Send:
            if (!xdr_client(xdrs, client_block, protocolVersion)) {
                err = UDA_PROTOCOL_ERROR_20;
                break;
            }
            break;

        case XDRStreamDirection::FreeHeap:
            break;

        default:
            err = UDA_PROTOCOL_ERROR_4;
            break;
    }
    return err;
}

static int handle_signal_desc(XDR* xdrs, XDRStreamDirection direction, const void* str)
{
    int err = 0;
    auto signal_desc = (SignalDesc*)str;

    switch (direction) {
        case XDRStreamDirection::Receive:
            if (!xdr_signal_desc(xdrs, signal_desc)) {
                err = UDA_PROTOCOL_ERROR_18;
                break;
            }
            break;

        case XDRStreamDirection::Send:
            if (!xdr_signal_desc(xdrs, signal_desc)) {
                err = UDA_PROTOCOL_ERROR_18;
                break;
            }
            break;

        case XDRStreamDirection::FreeHeap:
            break;

        default:
            err = UDA_PROTOCOL_ERROR_4;
            break;
    }
    return err;
}

static int handle_signal(XDR* xdrs, XDRStreamDirection direction, const void* str)
{
    int err = 0;
    auto signal = (Signal*)str;

    switch (direction) {
        case XDRStreamDirection::Receive:
            if (!xdr_signal(xdrs, signal)) {
                err = UDA_PROTOCOL_ERROR_16;
                break;
            }
            break;

        case XDRStreamDirection::Send:
            if (!xdr_signal(xdrs, signal)) {
                err = UDA_PROTOCOL_ERROR_16;
                break;
            }
            break;

        case XDRStreamDirection::FreeHeap:
            break;

        default:
            err = UDA_PROTOCOL_ERROR_4;
            break;
    }
    return err;
}

static int handle_data_source(XDR* xdrs, XDRStreamDirection direction, const void* str)
{
    int err = 0;
    auto data_source = (DataSource*)str;

    switch (direction) {

        case XDRStreamDirection::Receive:
            if (!xdr_data_source(xdrs, data_source)) {
                err = UDA_PROTOCOL_ERROR_14;
                break;
            }
            break;

        case XDRStreamDirection::Send:
            if (!xdr_data_source(xdrs, data_source)) {
                err = UDA_PROTOCOL_ERROR_14;
                break;
            }
            break;

        case XDRStreamDirection::FreeHeap:
            break;

        default:
            err = UDA_PROTOCOL_ERROR_4;
            break;
    }
    return err;
}

static int handle_system_config(XDR* xdrs, XDRStreamDirection direction, const void* str)
{
    int err = 0;
    auto system_config = (SystemConfig*)str;

    switch (direction) {
        case XDRStreamDirection::Receive:
            if (!xdr_system_config(xdrs, system_config)) {
                err = UDA_PROTOCOL_ERROR_12;
                break;
            }
            break;

        case XDRStreamDirection::Send:
            if (!xdr_system_config(xdrs, system_config)) {
                err = UDA_PROTOCOL_ERROR_12;
                break;
            }
            break;

        case XDRStreamDirection::FreeHeap:
            break;

        default:
            err = UDA_PROTOCOL_ERROR_4;
            break;
    }
    return err;
}

static int handle_data_system(XDR* xdrs, XDRStreamDirection direction, const void* str)
{
    int err = 0;
    auto data_system = (DataSystem*)str;

    switch (direction) {
        case XDRStreamDirection::Receive:
            if (!xdr_data_system(xdrs, data_system)) {
                err = UDA_PROTOCOL_ERROR_10;
                break;
            }
            break;

        case XDRStreamDirection::Send:
            if (!xdr_data_system(xdrs, data_system)) {
                err = UDA_PROTOCOL_ERROR_10;
                break;
            }
            break;

        case XDRStreamDirection::FreeHeap:
            break;

        default:
            err = UDA_PROTOCOL_ERROR_4;
            break;
    }
    return err;
}

static int handle_next_protocol(XDR* xdrs, XDRStreamDirection direction, ProtocolId* token)
{
    int err = 0;
    switch (direction) {
        case XDRStreamDirection::Receive: // From Client to Server
            if (!xdrrec_skiprecord(xdrs)) {
                err = UDA_PROTOCOL_ERROR_5;
                break;
            }
            if (!xdr_int(xdrs, (int*)token)) {
                err = UDA_PROTOCOL_ERROR_9;
                break;
            }
            break;

        case XDRStreamDirection::Send:
            if (!xdr_int(xdrs, (int*)token)) {
                err = UDA_PROTOCOL_ERROR_9;
                break;
            }
            if (!xdrrec_endofrecord(xdrs, 1)) {
                err = UDA_PROTOCOL_ERROR_7;
                break;
            }
            break;

        case XDRStreamDirection::FreeHeap:
            err = UDA_PROTOCOL_ERROR_3;
            break;

        default:
            err = UDA_PROTOCOL_ERROR_4;
            break;
    }
    return err;
}

static int handle_putdata_block_list(XDR* xdrs, XDRStreamDirection direction, ProtocolId* token, LogMallocList* logmalloclist,
                                     UserDefinedTypeList* userdefinedtypelist, const void* str, int protocolVersion,
                                     LogStructList* log_struct_list, unsigned int private_flags, int malloc_source)
{
    int err = 0;
    auto putDataBlockList = (PutDataBlockList*)str;

    switch (direction) {

        case XDRStreamDirection::Receive: {
            unsigned int blockCount = 0;

            if (!xdr_u_int(xdrs, &blockCount)) {
                err = UDA_PROTOCOL_ERROR_61;
                break;
            }

            UDA_LOG(UDA_LOG_DEBUG, "receive: putDataBlockList Count: {}", blockCount);

            for (unsigned int i = 0; i < blockCount; i++) {
                // Fetch multiple put blocks

                PutDataBlock put_data;
                init_put_data_block(&put_data);

                if (!xdr_putdata_block1(xdrs, &put_data)) {
                    err = UDA_PROTOCOL_ERROR_61;
                    UDA_LOG(UDA_LOG_DEBUG, "xdr_putdata_block1 Error (61)");
                    break;
                }

                if (protocol_version_type_test(protocolVersion, put_data.data_type)) {
                    err = UDA_PROTOCOL_ERROR_9999;
                    break;
                }

                if (put_data.count > 0 || put_data.blockNameLength > 0) { // Some data to receive?

                    if ((err = alloc_put_data(&put_data)) != 0) {
                        break; // Allocate Heap Memory
                    }

                    if (!xdr_putdata_block2(xdrs, &put_data)) { // Fetch data
                        err = UDA_PROTOCOL_ERROR_62;
                        break;
                    }
                }

                if (put_data.data_type == UDA_TYPE_COMPOUND && put_data.opaque_type == UDA_OPAQUE_TYPE_STRUCTURES) {
                    // Structured Data

                    // Create a temporary DataBlock as the function's argument with structured data

                    // logmalloc list is automatically generated
                    // userdefinedtypelist is passed from the client
                    // NTree is automatically generated

                    auto data_block = (DataBlock*)malloc(sizeof(DataBlock));

                    // *** Add to malloclog and test to ensure it is freed after use ***

                    init_data_block(data_block);
                    data_block->opaque_type = UDA_OPAQUE_TYPE_STRUCTURES;
                    data_block->data_n = (int)put_data.count;         // This number (also rank and shape)
                    data_block->opaque_block = put_data.opaque_block; // User Defined Type

                    ProtocolId protocol_id = ProtocolId::Structures;
                    if ((err = protocol_xml2_put(xdrs, protocol_id, direction, token, logmalloclist,
                                                 userdefinedtypelist, data_block, protocolVersion, log_struct_list,
                                                 private_flags, malloc_source)) != 0) {
                        // Fetch Structured data
                        break;
                    }

                    put_data.data = reinterpret_cast<char*>(data_block); // Compact memory block with structures
                    auto general_block = (GeneralBlock*)data_block->opaque_block;
                    put_data.opaque_block = general_block->userdefinedtype;
                }

                add_put_data_block_list(&put_data, putDataBlockList); // Add to the growing list
            }
            break;
        }

        case XDRStreamDirection::Send:

            UDA_LOG(UDA_LOG_DEBUG, "send: putDataBlockList Count: {}", putDataBlockList->blockCount);

            if (!xdr_u_int(xdrs, &(putDataBlockList->blockCount))) {
                err = UDA_PROTOCOL_ERROR_61;
                break;
            }

            for (unsigned int i = 0; i < putDataBlockList->blockCount; i++) { // Send multiple put blocks

                if (!xdr_putdata_block1(xdrs, &(putDataBlockList->putDataBlock[i]))) {
                    err = UDA_PROTOCOL_ERROR_61;
                    break;
                }

                if (protocol_version_type_test(protocolVersion, putDataBlockList->putDataBlock[i].data_type)) {
                    err = UDA_PROTOCOL_ERROR_9999;
                    break;
                }

                if (putDataBlockList->putDataBlock[i].count > 0 ||
                    putDataBlockList->putDataBlock[i].blockNameLength > 0) { // Data to Send?

                    if (!xdr_putdata_block2(xdrs, &(putDataBlockList->putDataBlock[i]))) {
                        err = UDA_PROTOCOL_ERROR_62;
                        break;
                    }
                }

                if (putDataBlockList->putDataBlock[i].data_type == UDA_TYPE_COMPOUND &&
                    putDataBlockList->putDataBlock[i].opaque_type == UDA_OPAQUE_TYPE_STRUCTURES) {
                    // Structured Data

                    // Create a temporary DataBlock as the function's argument with structured data

                    //   *** putdata.opaque_count is not used or needed - count is sufficient

                    DataBlock data_block;
                    init_data_block(&data_block);
                    data_block.opaque_type = UDA_OPAQUE_TYPE_STRUCTURES;
                    data_block.data_n =
                        (int)putDataBlockList->putDataBlock[i].count; // This number (also rank and shape)
                    data_block.opaque_block = putDataBlockList->putDataBlock[i].opaque_block; // User Defined Type
                    data_block.data =
                        (char*)putDataBlockList->putDataBlock[i].data; // Compact memory block with structures

                    ProtocolId protocol_id = ProtocolId::Structures;
                    if ((err = protocol_xml2_put(xdrs, protocol_id, direction, token, logmalloclist,
                                                 userdefinedtypelist, &data_block, protocolVersion, log_struct_list,
                                                 private_flags, malloc_source)) != 0) {
                        // Send Structured data
                        break;
                    }
                }
            }
            break;

        case XDRStreamDirection::FreeHeap:
            break;

        default:
            err = UDA_PROTOCOL_ERROR_4;
            break;
    }
    return err;
}

static int handle_data_block(XDR* xdrs, XDRStreamDirection direction, const void* str, int protocolVersion)
{
    int err = 0;
    auto data_block = (DataBlock*)str;

    switch (direction) {
        case XDRStreamDirection::Receive: {
            if (!xdr_data_block1(xdrs, data_block, protocolVersion)) {
                err = UDA_PROTOCOL_ERROR_61;
                break;
            }

            // Check client/server understands new data types
            // direction == XDRStreamDirection::Receive && protocolVersion == 3 Means Client receiving data from a
            // Version >= 3 Server (Type has to be passed first)

            if (protocol_version_type_test(protocolVersion, data_block->data_type) ||
                protocol_version_type_test(protocolVersion, data_block->error_type)) {
                err = UDA_PROTOCOL_ERROR_9999;
                break;
            }

            if (data_block->data_n == 0) {
                break; // No Data to Receive!
            }

            if ((err = alloc_data(data_block)) != 0) {
                break; // Allocate Heap Memory
            }

            if (!xdr_data_block2(xdrs, data_block)) {
                err = UDA_PROTOCOL_ERROR_62;
                break;
            }

            if (data_block->error_type != UDA_TYPE_UNKNOWN ||
                data_block->error_param_n > 0) { // Receive Only if Error Data are available
                if (!xdr_data_block3(xdrs, data_block)) {
                    err = UDA_PROTOCOL_ERROR_62;
                    break;
                }

                if (!xdr_data_block4(xdrs, data_block)) { // Asymmetric Errors
                    err = UDA_PROTOCOL_ERROR_62;
                    break;
                }
            }

            if (data_block->rank > 0) { // Check if there are Dimensional Data to Receive

                for (unsigned int i = 0; i < data_block->rank; i++) {
                    init_dim_block(&data_block->dims[i]);
                }

                if (!xdr_data_dim1(xdrs, data_block)) {
                    err = UDA_PROTOCOL_ERROR_63;
                    break;
                }

                if (protocolVersion < 3) {
                    for (unsigned int i = 0; i < data_block->rank; i++) {
                        Dims* dim = &data_block->dims[i];
                        if (protocol_version_type_test(protocolVersion, dim->data_type) ||
                            protocol_version_type_test(protocolVersion, dim->error_type)) {
                            err = UDA_PROTOCOL_ERROR_9999;
                            break;
                        }
                    }
                    if (err != 0) {
                        break;
                    }
                }

                if ((err = alloc_dim(data_block)) != 0) {
                    break; // Allocate Heap Memory
                }

                if (!xdr_data_dim2(xdrs, data_block)) { // Collect Only Uncompressed data
                    err = UDA_PROTOCOL_ERROR_64;
                    break;
                }

                for (unsigned int i = 0; i < data_block->rank; i++) { // Expand Compressed Regular Vector
                    err = uncompress_dim(&(data_block->dims[i]));     // Allocate Heap as required
                    err = 0;                                          // Need to Test for Error Condition!
                }

                if (!xdr_data_dim3(xdrs, data_block)) {
                    err = UDA_PROTOCOL_ERROR_65;
                    break;
                }

                if (!xdr_data_dim4(xdrs, data_block)) { // Asymmetric Errors
                    err = UDA_PROTOCOL_ERROR_65;
                    break;
                }
            }
            break;
        }

        case XDRStreamDirection::Send: {

            // Check client/server understands new data types

            // direction == XDRStreamDirection::Send && protocolVersion == 3 Means Server sending data to a Version 3 Client (Type is
            // known)

            UDA_LOG(UDA_LOG_DEBUG, "#1 PROTOCOL: Send/Receive Data Block");
            print_data_block(*data_block);

            if (protocol_version_type_test(protocolVersion, data_block->data_type) ||
                protocol_version_type_test(protocolVersion, data_block->error_type)) {
                err = UDA_PROTOCOL_ERROR_9999;
                UDA_LOG(UDA_LOG_DEBUG, "PROTOCOL: protocolVersionTypeTest Failed");

                break;
            }
            UDA_LOG(UDA_LOG_DEBUG, "#2 PROTOCOL: Send/Receive Data Block");
            if (!xdr_data_block1(xdrs, data_block, protocolVersion)) {
                err = UDA_PROTOCOL_ERROR_61;
                break;
            }

            if (data_block->data_n == 0) { // No Data or Dimensions to Send!
                break;
            }

            if (!xdr_data_block2(xdrs, data_block)) {
                err = UDA_PROTOCOL_ERROR_62;
                break;
            }

            if (data_block->error_type != UDA_TYPE_UNKNOWN || data_block->error_param_n > 0) {
                // Only Send if Error Data are available
                if (!xdr_data_block3(xdrs, data_block)) {
                    err = UDA_PROTOCOL_ERROR_62;
                    break;
                }
                if (!xdr_data_block4(xdrs, data_block)) {
                    err = UDA_PROTOCOL_ERROR_62;
                    break;
                }
            }

            if (data_block->rank > 0) {
                // Dimensional Data to Send

                // Check client/server understands new data types

                if (protocolVersion < 3) {
                    for (unsigned int i = 0; i < data_block->rank; i++) {
                        Dims* dim = &data_block->dims[i];
                        if (protocol_version_type_test(protocolVersion, dim->data_type) ||
                            protocol_version_type_test(protocolVersion, dim->error_type)) {
                            err = UDA_PROTOCOL_ERROR_9999;
                            break;
                        }
                    }
                    if (err != 0) {
                        break;
                    }
                }

                for (unsigned int i = 0; i < data_block->rank; i++) {
                    compress_dim(&(data_block->dims[i])); // Minimise Data Transfer if Regular
                }

                if (!xdr_data_dim1(xdrs, data_block)) {
                    err = UDA_PROTOCOL_ERROR_63;
                    break;
                }

                if (!xdr_data_dim2(xdrs, data_block)) {
                    err = UDA_PROTOCOL_ERROR_64;
                    break;
                }

                if (!xdr_data_dim3(xdrs, data_block)) {
                    err = UDA_PROTOCOL_ERROR_65;
                }

                if (!xdr_data_dim4(xdrs, data_block)) {
                    err = UDA_PROTOCOL_ERROR_65;
                }
            }
            break;
        }

        case XDRStreamDirection::FreeHeap:
            break;

        default:
            err = UDA_PROTOCOL_ERROR_4;
            break;
    }

    return err;
}

static int handle_data_block_list(XDR* xdrs, XDRStreamDirection direction, const void* str, int protocolVersion)
{
    int err = 0;
    auto data_block_list = (DataBlockList*)str;

    switch (direction) {
        case XDRStreamDirection::Receive:
            if (!xdr_data_block_list(xdrs, data_block_list, protocolVersion)) {
                err = UDA_PROTOCOL_ERROR_1;
                break;
            }
            data_block_list->data = (DataBlock*)malloc(data_block_list->count * sizeof(DataBlock));
            for (int i = 0; i < data_block_list->count; ++i) {
                DataBlock* data_block = &data_block_list->data[i];
                init_data_block(data_block);
                err = handle_data_block(xdrs, XDRStreamDirection::Receive, data_block, protocolVersion);
                if (err != 0) {
                    err = UDA_PROTOCOL_ERROR_2;
                    break;
                }
            }
            if (err) {
                break;
            }
            break;

        case XDRStreamDirection::Send: {
            if (!xdr_data_block_list(xdrs, data_block_list, protocolVersion)) {
                err = UDA_PROTOCOL_ERROR_2;
                break;
            }
            for (int i = 0; i < data_block_list->count; ++i) {
                DataBlock* data_block = &data_block_list->data[i];
                int rc = handle_data_block(xdrs, XDRStreamDirection::Send, data_block, protocolVersion);
                if (rc != 0) {
                    err = UDA_PROTOCOL_ERROR_2;
                    break;
                }
            }
            if (err) {
                break;
            }
            break;
        }

        case XDRStreamDirection::FreeHeap:
            break;

        default:
            err = UDA_PROTOCOL_ERROR_4;
            break;
    }
    return err;
}

static int handle_request_block(XDR* xdrs, XDRStreamDirection direction, const void* str, int protocolVersion)
{
    int err = 0;
    auto request_block = (RequestBlock*)str;

    switch (direction) {
        case XDRStreamDirection::Receive:
            if (!xdr_request(xdrs, request_block, protocolVersion)) {
                err = UDA_PROTOCOL_ERROR_1;
                break;
            }
            request_block->requests = (RequestData*)malloc(request_block->num_requests * sizeof(RequestData));
            for (int i = 0; i < request_block->num_requests; ++i) {
                init_request_data(&request_block->requests[i]);
                if (!xdr_request_data(xdrs, &request_block->requests[i], protocolVersion)) {
                    err = UDA_PROTOCOL_ERROR_2;
                    break;
                }
            }
            if (err) {
                break;
            }
            break;

        case XDRStreamDirection::Send:
            if (!xdr_request(xdrs, request_block, protocolVersion)) {
                err = UDA_PROTOCOL_ERROR_2;
                break;
            }
            for (int i = 0; i < request_block->num_requests; ++i) {
                if (!xdr_request_data(xdrs, &request_block->requests[i], protocolVersion)) {
                    err = UDA_PROTOCOL_ERROR_2;
                    break;
                }
            }
            if (err) {
                break;
            }
            break;

        case XDRStreamDirection::FreeHeap:
            break;

        default:
            err = UDA_PROTOCOL_ERROR_4;
            break;
    }
    return err;
}
