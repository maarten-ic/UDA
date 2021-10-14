#include "clientXDRStream.h"

#include <cstdio>
#include <rpc/rpc.h>

#include <logging/logging.h>
#include <clientserver/udaDefines.h>

#include "connection.h"

#if defined(SSLAUTHENTICATION) && !defined(FATCLIENT)
#  include <authentication/udaClientSSL.h>
#endif

static XDR clientXDRinput;
static XDR clientXDRoutput;

#if defined(__GNUC__)
XDR* clientInput = &clientXDRinput;
XDR* clientOutput = &clientXDRoutput;
#else
extern "C" XDR* clientInput = &clientXDRinput;
extern "C" XDR* clientOutput = &clientXDRoutput;
#endif


void createXDRStream()
{
    clientOutput->x_ops = nullptr;
    clientInput->x_ops = nullptr;

    UDA_LOG(UDA_LOG_DEBUG, "Creating XDR Streams \n");

#if defined(SSLAUTHENTICATION) && !defined(FATCLIENT)

    if (getUdaClientSSLDisabled()) {
    
#if defined (__APPLE__) || defined(__TIRPC__)
       xdrrec_create(clientOutput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                     reinterpret_cast<int (*)(void *, void *, int)>(clientReadin),
                     reinterpret_cast<int (*)(void *, void *, int)>(clientWriteout));

       xdrrec_create(clientInput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                     reinterpret_cast<int (*)(void *, void *, int)>(clientReadin),
                     reinterpret_cast<int (*)(void *, void *, int)>(clientWriteout));
#else
       xdrrec_create(clientOutput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                     reinterpret_cast<int (*)(char *, char *, int)>(clientReadin),
                     reinterpret_cast<int (*)(char *, char *, int)>(clientWriteout));

       xdrrec_create(clientInput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                     reinterpret_cast<int (*)(char *, char *, int)>(clientReadin),
                     reinterpret_cast<int (*)(char *, char *, int)>(clientWriteout));
#endif    
    } else {
#if defined (__APPLE__) || defined(__TIRPC__)
       xdrrec_create(clientOutput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                     reinterpret_cast<int (*)(void *, void *, int)>(readUdaClientSSL),
                     reinterpret_cast<int (*)(void *, void *, int)>(writeUdaClientSSL));

       xdrrec_create(clientInput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                     reinterpret_cast<int (*)(void *, void *, int)>(readUdaClientSSL),
                     reinterpret_cast<int (*)(void *, void *, int)>(writeUdaClientSSL));
#else
       xdrrec_create(clientOutput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                     reinterpret_cast<int (*)(char *, char *, int)>(readUdaClientSSL),
                     reinterpret_cast<int (*)(char *, char *, int)>(writeUdaClientSSL));

       xdrrec_create(clientInput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                     reinterpret_cast<int (*)(char *, char *, int)>(readUdaClientSSL),
                     reinterpret_cast<int (*)(char *, char *, int)>(writeUdaClientSSL));
#endif
    }
#else

#if defined (__APPLE__) || defined(__TIRPC__)
    xdrrec_create(clientOutput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                  reinterpret_cast<int (*)(void *, void *, int)>(clientReadin),
                  reinterpret_cast<int (*)(void *, void *, int)>(clientWriteout));

    xdrrec_create(clientInput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                  reinterpret_cast<int (*)(void *, void *, int)>(clientReadin),
                  reinterpret_cast<int (*)(void *, void *, int)>(clientWriteout));
#else
    xdrrec_create(clientOutput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                  reinterpret_cast<int (*)(char *, char *, int)>(clientReadin),
                  reinterpret_cast<int (*)(char *, char *, int)>(clientWriteout));

    xdrrec_create(clientInput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                  reinterpret_cast<int (*)(char *, char *, int)>(clientReadin),
                  reinterpret_cast<int (*)(char *, char *, int)>(clientWriteout));
#endif

#endif   // SSLAUTHENTICATION

    clientInput->x_op = XDR_DECODE;
    clientOutput->x_op = XDR_ENCODE;
}
