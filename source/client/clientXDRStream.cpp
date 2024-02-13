#include "clientXDRStream.h"

#include <cstdio>
#include <rpc/rpc.h>

#include "clientserver/udaDefines.h"
#include "logging/logging.h"

#include "connection.h"

#if defined(SSLAUTHENTICATION) && !defined(FATCLIENT)
#  include <authentication/udaClientSSL.h>

using namespace uda::authentication;
#endif

std::pair<XDR*, XDR*> uda::client::clientCreateXDRStream()
{
    static XDR client_input = {};
    static XDR client_output = {};

    client_output.x_ops = nullptr;
    client_input.x_ops = nullptr;

    UDA_LOG(UDA_LOG_DEBUG, "Creating XDR Streams \n");

#if defined(SSLAUTHENTICATION) && !defined(FATCLIENT)
    if (getUdaClientSSLDisabled()) {

#  if defined(__APPLE__) || defined(__TIRPC__)
        xdrrec_create(&client_output, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                      reinterpret_cast<int (*)(void*, void*, int)>(uda::client::clientReadin),
                      reinterpret_cast<int (*)(void*, void*, int)>(uda::client::clientWriteout));

        xdrrec_create(&client_input, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                      reinterpret_cast<int (*)(void*, void*, int)>(uda::client::clientReadin),
                      reinterpret_cast<int (*)(void*, void*, int)>(uda::client::clientWriteout));
#  else
        xdrrec_create(&client_output, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                      reinterpret_cast<int (*)(char*, char*, int)>(clientReadin),
                      reinterpret_cast<int (*)(char*, char*, int)>(clientWriteout));

        xdrrec_create(&client_input, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                      reinterpret_cast<int (*)(char*, char*, int)>(clientReadin),
                      reinterpret_cast<int (*)(char*, char*, int)>(clientWriteout));
#  endif
    } else {
#  if defined(__APPLE__) || defined(__TIRPC__)
        xdrrec_create(&client_output, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                      reinterpret_cast<int (*)(void*, void*, int)>(readUdaClientSSL),
                      reinterpret_cast<int (*)(void*, void*, int)>(writeUdaClientSSL));

        xdrrec_create(&client_input, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                      reinterpret_cast<int (*)(void*, void*, int)>(readUdaClientSSL),
                      reinterpret_cast<int (*)(void*, void*, int)>(writeUdaClientSSL));
#  else
        xdrrec_create(&client_output, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                      reinterpret_cast<int (*)(char*, char*, int)>(readUdaClientSSL),
                      reinterpret_cast<int (*)(char*, char*, int)>(writeUdaClientSSL));

        xdrrec_create(&client_input, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                      reinterpret_cast<int (*)(char*, char*, int)>(readUdaClientSSL),
                      reinterpret_cast<int (*)(char*, char*, int)>(writeUdaClientSSL));
#  endif
    }
#else

#  if defined(__APPLE__) || defined(__TIRPC__)
    xdrrec_create(&client_output, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                  reinterpret_cast<int (*)(void*, void*, int)>(uda::client::clientReadin),
                  reinterpret_cast<int (*)(void*, void*, int)>(uda::client::clientWriteout));

    xdrrec_create(&client_input, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                  reinterpret_cast<int (*)(void*, void*, int)>(uda::client::clientReadin),
                  reinterpret_cast<int (*)(void*, void*, int)>(uda::client::clientWriteout));
#  else
    xdrrec_create(&client_output, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                  reinterpret_cast<int (*)(char*, char*, int)>(uda::client::clientReadin),
                  reinterpret_cast<int (*)(char*, char*, int)>(uda::client::clientWriteout));

    xdrrec_create(&client_input, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                  reinterpret_cast<int (*)(char*, char*, int)>(uda::client::clientReadin),
                  reinterpret_cast<int (*)(char*, char*, int)>(uda::client::clientWriteout));
#  endif

#endif // SSLAUTHENTICATION

    client_input.x_op = XDR_DECODE;
    client_output.x_op = XDR_ENCODE;

    return std::make_pair(&client_input, &client_output);
}
