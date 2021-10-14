#include "createXDRStream.h"

#include <server/udaServer.h>

#include "writer.h"

#if !defined(FATCLIENT) && defined(SSLAUTHENTICATION)
#include <authentication/udaServerSSL.h>
#endif

void CreateXDRStream() {
    serverOutput->x_ops  = nullptr;
    serverInput->x_ops   = nullptr;

#if !defined(FATCLIENT) && defined(SSLAUTHENTICATION)

    if (getUdaServerSSLDisabled()) {

#if defined (__APPLE__) || defined(__TIRPC__)
       xdrrec_create( serverOutput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                      reinterpret_cast<int (*)(void *, void *, int)>(Readin),
                      reinterpret_cast<int (*)(void *, void *, int)>(Writeout));

       xdrrec_create( serverInput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                      reinterpret_cast<int (*)(void *, void *, int)>(Readin),
                      reinterpret_cast<int (*)(void *, void *, int)>(Writeout));
#else
       xdrrec_create( serverOutput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                      reinterpret_cast<int (*)(char *, char *, int)>(Readin),
                      reinterpret_cast<int (*)(char *, char *, int)>(Writeout));

       xdrrec_create( serverInput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                      reinterpret_cast<int (*)(char *, char *, int)>(Readin),
                      reinterpret_cast<int (*)(char *, char *, int)>(Writeout));
#endif     
    } else { 
#if defined (__APPLE__) || defined(__TIRPC__)
       xdrrec_create( serverOutput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                      reinterpret_cast<int (*)(void *, void *, int)>(readUdaServerSSL),
                      reinterpret_cast<int (*)(void *, void *, int)>(writeUdaServerSSL));

       xdrrec_create( serverInput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                      reinterpret_cast<int (*)(void *, void *, int)>(readUdaServerSSL),
                      reinterpret_cast<int (*)(void *, void *, int)>(writeUdaServerSSL));
#else
       xdrrec_create( serverOutput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                      reinterpret_cast<int (*)(char *, char *, int)>(readUdaServerSSL),
                      reinterpret_cast<int (*)(char *, char *, int)>(writeUdaServerSSL));

       xdrrec_create( serverInput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                      reinterpret_cast<int (*)(char *, char *, int)>(readUdaServerSSL),
                      reinterpret_cast<int (*)(char *, char *, int)>(writeUdaServerSSL));
#endif
    }
    
#else	// SSLAUTHENTICATION

#if defined (__APPLE__) || defined(__TIRPC__)
    xdrrec_create( serverOutput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                   reinterpret_cast<int (*)(void *, void *, int)>(Readin),
                   reinterpret_cast<int (*)(void *, void *, int)>(Writeout));

    xdrrec_create( serverInput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                   reinterpret_cast<int (*)(void *, void *, int)>(Readin),
                   reinterpret_cast<int (*)(void *, void *, int)>(Writeout));
#else
    xdrrec_create( serverOutput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                   reinterpret_cast<int (*)(char *, char *, int)>(Readin),
                   reinterpret_cast<int (*)(char *, char *, int)>(Writeout));

    xdrrec_create( serverInput, DB_READ_BLOCK_SIZE, DB_WRITE_BLOCK_SIZE, nullptr,
                   reinterpret_cast<int (*)(char *, char *, int)>(Readin),
                   reinterpret_cast<int (*)(char *, char *, int)>(Writeout));
#endif

#endif   // SSLAUTHENTICATION

    serverInput->x_op   = XDR_DECODE;
    serverOutput->x_op  = XDR_ENCODE;
}
