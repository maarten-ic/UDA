#ifndef UDA_SERVER_WRITER_H
#define UDA_SERVER_WRITER_H

#if defined(__GNUC__)
#  include <unistd.h>
#endif
#include <fcntl.h>

#ifdef _WIN32
#  include <winsock2.h> // must be included before connection.h to avoid macro redefinition in rpc/types.h
#else
#  include <sys/select.h>
#endif

#include <clientserver/export.h>

#define MIN_BLOCK_TIME    1000
#define MAX_BLOCK_TIME    10000

struct IoData {
    int* server_tot_block_time;
    int* server_timeout;
};

#ifdef __cplusplus
extern "C" {
#endif

LIBRARY_API void setSelectParms(int fd, fd_set* rfds, struct timeval* tv, int* server_tot_block_time);
LIBRARY_API void updateSelectParms(int fd, fd_set* rfds, struct timeval* tv, int server_tot_block_time);
LIBRARY_API int server_write(void* iohandle, char* buf, int count);

/*
//-----------------------------------------------------------------------------------------
// This routine is only called when the Server expects to Read something from the Client
//
// There are two time constraints:
//
//    The Maximum Blocking period is 1ms when reading
//    A Maximum number (MAXLOOP) of blocking periods is allowed before this time
//    is modified: It is extended to 100ms to minimise server resource consumption.
//
// When the Server is in a Holding state, it is listening to the Socket for either a
// Closedown or a Data request. 
//
// Three Global variables are used to control the Blocking timeout
//
//    min_block_time
//    max_block_time
//    tot_block_time
//
// A Maximum time (MAXBLOCK in seconds) from the last Data Request is permitted before the Server Automatically
// closes down.
//-----------------------------------------------------------------------------------------
*/
LIBRARY_API int server_read(void* iohandle, char* buf, int count);

#ifdef __cplusplus
}
#endif

#endif // UDA_SERVER_WRITER_H
