/*---------------------------------------------------------------
*  Open Client Side Log Files
*
* Argument: reset => if 1 then always reopen Files
*
* Checks for an Environment var (UDA_LOG) for the directory of the log file
* otherwise it assumes the file is to be local to the PWD.
*
* The directory named must not have a trailing forward slash.
*
* It is assumed that the directory specified by UDA_LOG already exits
*
* Returns:
*
*--------------------------------------------------------------*/
#include "startup.h"

#include <cerrno>

#include <logging/logging.h>
#include <clientserver/errorLog.h>
#include <clientserver/udaErrors.h>

#include "udaClient.h"
#include "getEnvironment.h"

int udaStartup(int reset)
{
    static int start_status = 0;

    //---------------------------------------------------------------
    // Are the Files Already Open?

    if (start_status && !reset && !reopen_logs) return 0;

    //----------------------------------------------------------------
    // Read Environment Variable Values (Held in a Global Structure)

    const ENVIRONMENT* environment = nullptr;

    if (!start_status || environment == nullptr) {
        environment = getIdamClientEnvironment();
    }

    printIdamClientEnvironment(environment);

    //----------------------------------------------------------------
    // Client set Property Flags (can be changed via property accessor functions)
    // Coded user properties changes have priority

    if (environment->clientFlags != 0) {
        clientFlags = clientFlags | environment->clientFlags;
    }

    if (environment->altRank != 0 && altRank == 0) {
        altRank = environment->altRank;
    }

    //----------------------------------------------------------------
    // X.509 Security Certification

    //if((rc = readIdamSecurityCert(environment->security_cert)) != 0){
    //   if(verbose) fprintf(stderr, "Idam: Problem Locating the Security Certificate [%d]\n",  rc);
    //   return(-1);
    //}

    //----------------------------------------------------------------
    // Check if Output Requested

    udaSetLogLevel((LOG_LEVEL)environment->loglevel);

    if (environment->loglevel == UDA_LOG_NONE) return 0;

    //---------------------------------------------------------------
    // Open the Log File

    start_status = 1;
    errno = 0;

    FILE* file = nullptr;

    char idamFile[STRING_LENGTH];

    strcpy(idamFile, environment->logdir);
    strcat(idamFile, "Debug.dbg");
    file = fopen(idamFile, environment->logmode);
    udaSetLogFile(UDA_LOG_WARN, file);
    udaSetLogFile(UDA_LOG_DEBUG, file);
    udaSetLogFile(UDA_LOG_INFO, file);

    if (errno != 0) {
        addIdamError(SYSTEMERRORTYPE, __func__, errno, "failed to open debug log");
        udaCloseLogging();
        return -1;
    }

    if (udaGetLogLevel() == UDA_LOG_ERROR) {
        strcpy(idamFile, environment->logdir);
        strcat(idamFile, "Error.err");
        file = fopen(idamFile, environment->logmode);
        udaSetLogFile(UDA_LOG_ERROR, file);
    }

    if (errno != 0) {
        addIdamError(SYSTEMERRORTYPE, __func__, errno, "failed to open error log");
        udaCloseLogging();
        return -1;
    }

    reopen_logs = 0;

    return 0;
}
