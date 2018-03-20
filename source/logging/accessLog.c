/*---------------------------------------------------------------
* Server Access Log
*
* Log Format: Conforms to the Common Log Format for the first 6 fields
*
*		client address, client userid, date, client request,
*		error code, data bytes returned
* plus:		error message, elapsed time, client version, server version
*		client process id
*
*--------------------------------------------------------------*/

#include "accessLog.h"

#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <clientserver/stringUtils.h>
#include <plugins/serverPlugin.h>
#include <server/udaServer.h>
#include <clientserver/udaTypes.h>

unsigned int countDataBlockSize(DATA_BLOCK* data_block, CLIENT_BLOCK* client_block)
{

    int factor;
    DIMS dim;
    unsigned int count = sizeof(DATA_BLOCK);

    count += (unsigned int)(getSizeOf(data_block->data_type) * data_block->data_n);

    if (data_block->error_type != UDA_TYPE_UNKNOWN) {
        count += (unsigned int)(getSizeOf(data_block->error_type) * data_block->data_n);
    }
    if (data_block->errasymmetry) count += (unsigned int)(getSizeOf(data_block->error_type) * data_block->data_n);

    unsigned int k;
    if (data_block->rank > 0) {
        for (k = 0; k < data_block->rank; k++) {
            count += sizeof(DIMS);
            dim = data_block->dims[k];
            if (!dim.compressed) {
                count += (unsigned int)(getSizeOf(dim.data_type) * dim.dim_n);
                factor = 1;
                if (dim.errasymmetry) factor = 2;
                if (dim.error_type != UDA_TYPE_UNKNOWN) {
                    count += (unsigned int)(factor * getSizeOf(dim.error_type) * dim.dim_n);
                }
            } else {
                unsigned int i;
                switch (dim.method) {
                    case 0:
                        count += +2 * sizeof(double);
                        break;
                    case 1:
                        for (i = 0; i < dim.udoms; i++) {
                            count += (unsigned int)(*((long*)dim.sams + i) * getSizeOf(dim.data_type));
                        }
                        break;
                    case 2:
                        count += dim.udoms * getSizeOf(dim.data_type);
                        break;
                    case 3:
                        count += dim.udoms * getSizeOf(dim.data_type);
                        break;
                }
            }
        }
    }

    if (client_block->get_meta) {
        count += sizeof(DATA_SYSTEM) + sizeof(SYSTEM_CONFIG) + sizeof(DATA_SOURCE) + sizeof(SIGNAL) +
                 sizeof(SIGNAL_DESC);
    }

    return count;
}


void idamAccessLog(int init, CLIENT_BLOCK client_block, REQUEST_BLOCK request, SERVER_BLOCK server_block,
                   const PLUGINLIST* pluginlist)
{

    int err = 0;
    const char* msg;
    const char* const empty = "";
#ifndef FATCLIENT
    int socket;
    unsigned int addrlen;
#endif
#ifndef IPV6PROTOCOL
#  ifndef FATCLIENT
    struct sockaddr_in addr;
#  endif
    static char host[INET6_ADDRSTRLEN + 1];
#else
#  ifndef FATCLIENT
    struct sockaddr_in6 addr;
#  endif
    static char host[INET6_ADDRSTRLEN+1];
#endif
    static struct timeval et_start, et_end;
    double elapsedtime;            // Elapsed Time
    time_t calendar;            // Simple Calendar Date & Time
    struct tm* broken;            // Broken Down calendar Time
    static char accessdate[DATELENGTH];    // The Calendar Time as a formatted String

    errno = 0;

    if (init) {            // Start of Access Log Record - at start of the request

// Remote Host IP Address

#ifndef FATCLIENT
        socket = 0;
        memset(&addr, 0, sizeof(addr));
        addrlen = sizeof(addr);
#ifndef IPV6PROTOCOL
        addr.sin_family = AF_INET;
        if ((getpeername(socket, (struct sockaddr*)&addr, &addrlen)) == -1) {        // Socket Address
            strcpy(host, "-");
        } else {
            if (addrlen <= HOSTNAMELENGTH - 1) {
                strncpy(host, inet_ntoa(addr.sin_addr), addrlen);
                host[addrlen] = '\0';
            } else {
                strncpy(host, inet_ntoa(addr.sin_addr), HOSTNAMELENGTH - 1);
                host[HOSTNAMELENGTH - 1] = '\0';
            }
#endif
            convertNonPrintable2(host);
            TrimString(host);
            if (strlen(host) == 0) strcpy(host, "-");
        }
#else
        strcpy(host,"-");
#endif

// Client's Userid: from the client_block structure

// Request Start Time
        gettimeofday(&et_start, NULL);

// Calendar Time

        time(&calendar);
        broken = gmtime(&calendar);
        asctime_r(broken, accessdate);

        convertNonPrintable2(accessdate);
        TrimString(accessdate);

// Client Request: From the request_block structure

        return;

    }

// Write the Log Record

// Error Code & Message

    if (server_block.idamerrorstack.nerrors > 0) {
        err = server_block.idamerrorstack.idamerror[0].code;
        msg = server_block.idamerrorstack.idamerror[0].msg;
    } else {
        err = 0;
        msg = empty;
    }

// Error Message: from data_block

// Amount of Data Returned (Excluding Meta Data) from global: totalDataBlockSize;

// Request Completed Time: Elasped & CPU

    gettimeofday(&et_end, NULL);
    elapsedtime = (double)((et_end.tv_sec - et_start.tv_sec) * 1000);    // millisecs

    if (et_end.tv_usec < et_start.tv_usec) {
        elapsedtime = elapsedtime - 1.0 + (double)(1000000 + et_end.tv_usec - et_start.tv_usec) / 1000.0;
    } else {
        elapsedtime = elapsedtime + (double)(et_end.tv_usec - et_start.tv_usec) / 1000.0;
    }

// Write the Log Record & Flush the fd

    size_t wlen = strlen(host) + 1 +
                  strlen(client_block.uid) + 1 +
                  strlen(accessdate) + 1 +
                  strlen(request.signal) + 1 +
                  strlen(request.tpass) + 1 +
                  strlen(request.path) + 1 +
                  strlen(request.file) + 1 +
                  strlen(request.format) + 1 +
                  strlen(request.archive) + 1 +
                  strlen(request.device_name) + 1 +
                  strlen(request.server) + 1 +
                  strlen(msg) + 1 +
                  strlen(client_block.DOI) + 1 +
                  1024;

    if (wlen < MAXMETA) {
        char* work = (char*)malloc(MAXMETA * sizeof(char));

        sprintf(work, "%s - %s [%s] [%d %s %d %d %s %s %s %s %s %s %s] %d %d [%s] %f %d %d [%d %d] [%s]",
                host, client_block.uid, accessdate, request.request, request.signal, request.exp_number,
                request.pass, request.tpass, request.path, request.file, request.format, request.archive,
                request.device_name, request.server, err, (int)totalDataBlockSize, msg,
                elapsedtime, client_block.version, server_block.version, client_block.pid, server_block.pid,
                client_block.DOI);

        idamLog(UDA_LOG_ACCESS, "%s\n", work);

// Save Provenance with socket stream protection

        idamServerRedirectStdStreams(0);
        idamProvenancePlugin(&client_block, &request, NULL, NULL, pluginlist, work);
        idamServerRedirectStdStreams(1);

        free((void*)work);

    } else {
        idamLog(UDA_LOG_ACCESS, "%s - %s [%s] [%d %s %d %d %s %s %s %s %s %s %s] %d %d [%s] %f %d %d [%d %d] [%s]\n",
                host, client_block.uid, accessdate, request.request, request.signal, request.exp_number,
                request.pass, request.tpass, request.path, request.file, request.format, request.archive,
                request.device_name, request.server, err, (int)totalDataBlockSize, msg,
                elapsedtime, client_block.version, server_block.version, client_block.pid, server_block.pid,
                client_block.DOI);
    }

}
