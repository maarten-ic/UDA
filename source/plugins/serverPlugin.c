/*---------------------------------------------------------------
* Identify the correct IDAM Data Server Plugin
*---------------------------------------------------------------------------------------------------------------------*/
#include "serverPlugin.h"

#include <stdlib.h>
#include <errno.h>
#include <dlfcn.h>
#include <strings.h>

#include <cache/cache.h>
#include <client/udaClient.h>
#include <clientserver/expand_path.h>
#include <clientserver/freeDataBlock.h>
#include <clientserver/initStructs.h>
#include <clientserver/printStructs.h>
#include <clientserver/protocol.h>
#include <clientserver/stringUtils.h>
#include <clientserver/udaErrors.h>
#include <server/modules/ida/parseIdaPath.h>
#include <structures/struct.h>
#include <server/getPluginAddress.h>
#include <server/makeServerRequestBlock.h>
#include <server/getServerEnvironment.h>

#define REQUEST_READ_START      1000
#define REQUEST_PLUGIN_MCOUNT   100    // Maximum initial number of plugins that can be registered
#define REQUEST_PLUGIN_MSTEP    10    // Increase heap by 10 records once the maximum is exceeded

int initPlugin(const IDAM_PLUGIN_INTERFACE* plugin_interface)
{
    idamSetLogLevel((LOG_LEVEL)plugin_interface->environment->loglevel);

    return 0;
}

int setReturnDataFloatArray(DATA_BLOCK* data_block, float* values, size_t rank, const size_t* shape,
                            const char* description)
{
    initDataBlock(data_block);

    if (description != NULL) {
        strncpy(data_block->data_desc, description, STRING_LENGTH);
        data_block->data_desc[STRING_LENGTH - 1] = '\0';
    }

    data_block->rank = (int)rank;
    data_block->dims = malloc(rank * sizeof(DIMS));

    size_t len = 1;

    size_t i;
    for (i = 0; i < rank; ++i) {
        initDimBlock(&data_block->dims[i]);

        data_block->dims[0].data_type = UDA_TYPE_UNSIGNED_INT;
        data_block->dims[0].dim_n = (int)shape[i];
        data_block->dims[0].compressed = 1;
        data_block->dims[0].dim0 = 0.0;
        data_block->dims[0].diff = 1.0;
        data_block->dims[0].method = 0;

        len *= shape[i];
    }

    double* data = malloc(len * sizeof(float));
    memcpy(data, values, len * sizeof(float));

    data_block->data_type = UDA_TYPE_FLOAT;
    data_block->data = (char*)data;
    data_block->data_n = (int)len;

    return 0;
}

int setReturnDataDoubleArray(DATA_BLOCK* data_block, double* values, size_t rank, const size_t* shape,
                             const char* description)
{
    initDataBlock(data_block);

    if (description != NULL) {
        strncpy(data_block->data_desc, description, STRING_LENGTH);
        data_block->data_desc[STRING_LENGTH - 1] = '\0';
    }

    data_block->rank = (int)rank;
    data_block->dims = malloc(rank * sizeof(DIMS));

    size_t len = 1;

    size_t i;
    for (i = 0; i < rank; ++i) {
        initDimBlock(&data_block->dims[i]);

        data_block->dims[0].data_type = UDA_TYPE_UNSIGNED_INT;
        data_block->dims[0].dim_n = (int)shape[i];
        data_block->dims[0].compressed = 1;
        data_block->dims[0].dim0 = 0.0;
        data_block->dims[0].diff = 1.0;
        data_block->dims[0].method = 0;

        len *= shape[i];
    }

    double* data = malloc(len * sizeof(float));
    memcpy(data, values, len * sizeof(float));

    data_block->data_type = UDA_TYPE_DOUBLE;
    data_block->data = (char*)data;
    data_block->data_n = (int)len;

    return 0;
}

int
setReturnDataIntArray(DATA_BLOCK* data_block, int* values, size_t rank, const size_t* shape, const char* description)
{
    initDataBlock(data_block);

    if (description != NULL) {
        strncpy(data_block->data_desc, description, STRING_LENGTH);
        data_block->data_desc[STRING_LENGTH - 1] = '\0';
    }

    data_block->rank = (int)rank;
    data_block->dims = malloc(rank * sizeof(DIMS));

    size_t len = 1;

    size_t i;
    for (i = 0; i < rank; ++i) {
        initDimBlock(&data_block->dims[i]);

        data_block->dims[0].data_type = UDA_TYPE_UNSIGNED_INT;
        data_block->dims[0].dim_n = (int)shape[i];
        data_block->dims[0].compressed = 1;
        data_block->dims[0].dim0 = 0.0;
        data_block->dims[0].diff = 1.0;
        data_block->dims[0].method = 0;

        len *= shape[i];
    }

    double* data = malloc(len * sizeof(float));
    memcpy(data, values, len * sizeof(float));

    data_block->data_type = UDA_TYPE_INT;
    data_block->data = (char*)data;
    data_block->data_n = (int)len;

    return 0;
}

int setReturnDataDoubleScalar(DATA_BLOCK* data_block, double value, const char* description)
{
    initDataBlock(data_block);

    double* data = (double*)malloc(sizeof(double));
    data[0] = value;

    if (description != NULL) {
        strncpy(data_block->data_desc, description, STRING_LENGTH);
        data_block->data_desc[STRING_LENGTH - 1] = '\0';
    }

    data_block->rank = 0;
    data_block->data_type = UDA_TYPE_DOUBLE;
    data_block->data = (char*)data;
    data_block->data_n = 1;

    return 0;
}

int setReturnDataFloatScalar(DATA_BLOCK* data_block, float value, const char* description)
{
    initDataBlock(data_block);

    float* data = (float*)malloc(sizeof(float));
    data[0] = value;

    if (description != NULL) {
        strncpy(data_block->data_desc, description, STRING_LENGTH);
        data_block->data_desc[STRING_LENGTH - 1] = '\0';
    }

    data_block->rank = 0;
    data_block->data_type = UDA_TYPE_FLOAT;
    data_block->data = (char*)data;
    data_block->data_n = 1;

    return 0;
}

int setReturnDataIntScalar(DATA_BLOCK* data_block, int value, const char* description)
{
    initDataBlock(data_block);

    int* data = (int*)malloc(sizeof(int));
    data[0] = value;

    if (description != NULL) {
        strncpy(data_block->data_desc, description, STRING_LENGTH);
        data_block->data_desc[STRING_LENGTH - 1] = '\0';
    }

    data_block->rank = 0;
    data_block->data_type = UDA_TYPE_INT;
    data_block->data = (char*)data;
    data_block->data_n = 1;

    return 0;
}

int setReturnDataLongScalar(DATA_BLOCK* data_block, long value, const char* description)
{
    initDataBlock(data_block);

    long* data = (long*)malloc(sizeof(long));
    data[0] = value;

    if (description != NULL) {
        strncpy(data_block->data_desc, description, STRING_LENGTH);
        data_block->data_desc[STRING_LENGTH - 1] = '\0';
    }

    data_block->rank = 0;
    data_block->data_type = UDA_TYPE_LONG;
    data_block->data = (char*)data;
    data_block->data_n = 1;

    return 0;
}

int setReturnDataShortScalar(DATA_BLOCK* data_block, short value, const char* description)
{
    initDataBlock(data_block);

    short* data = (short*)malloc(sizeof(short));
    data[0] = value;

    if (description != NULL) {
        strncpy(data_block->data_desc, description, STRING_LENGTH);
        data_block->data_desc[STRING_LENGTH - 1] = '\0';
    }

    data_block->rank = 0;
    data_block->data_type = UDA_TYPE_SHORT;
    data_block->data = (char*)data;
    data_block->data_n = 1;

    return 0;
}

int setReturnDataString(DATA_BLOCK* data_block, const char* value, const char* description)
{
    initDataBlock(data_block);

    data_block->data_type = UDA_TYPE_STRING;
    data_block->data = strdup(value);

    data_block->rank = 1;
    data_block->dims = (DIMS*)malloc(data_block->rank * sizeof(DIMS));

    int i;
    for (i = 0; i < data_block->rank; i++) {
        initDimBlock(&data_block->dims[i]);
    }

    if (description != NULL) {
        strncpy(data_block->data_desc, description, STRING_LENGTH);
        data_block->data_desc[STRING_LENGTH - 1] = '\0';
    }

    data_block->dims[0].data_type = UDA_TYPE_UNSIGNED_INT;
    data_block->dims[0].dim_n = (int)strlen(value) + 1;
    data_block->dims[0].compressed = 1;
    data_block->dims[0].dim0 = 0.0;
    data_block->dims[0].diff = 1.0;
    data_block->dims[0].method = 0;

    data_block->data_n = data_block->dims[0].dim_n;

    return 0;
}

void allocPluginList(int count, PLUGINLIST* plugin_list)
{
    if (count >= plugin_list->mcount) {
        plugin_list->mcount = plugin_list->mcount + REQUEST_PLUGIN_MSTEP;
        plugin_list->plugin = (PLUGIN_DATA*)realloc((void*)plugin_list->plugin,
                                                    plugin_list->mcount * sizeof(PLUGIN_DATA));
    }
}

void closePluginList(const PLUGINLIST* plugin_list)
{
    int i;
    REQUEST_BLOCK request_block;
    IDAM_PLUGIN_INTERFACE idam_plugin_interface;
    initRequestBlock(&request_block);
    strcpy(request_block.function, "reset");

    idam_plugin_interface.interfaceVersion = 1;
    idam_plugin_interface.housekeeping = 1;            // Force a full reset
    idam_plugin_interface.request_block = &request_block;
    for (i = 0; i < plugin_list->count; i++) {
        if (plugin_list->plugin[i].pluginHandle != NULL) {
            plugin_list->plugin[i].idamPlugin(&idam_plugin_interface);        // Call the housekeeping method
        }
    }
}

void freePluginList(PLUGINLIST* plugin_list)
{
    int i;
    closePluginList(plugin_list);
    for (i = 0; i < plugin_list->count; i++) {
        if (plugin_list->plugin[i].pluginHandle != NULL) {
            dlclose(plugin_list->plugin[i].pluginHandle);
        }
    }
    free((void*)plugin_list->plugin);
    plugin_list->plugin = NULL;
    plugin_list->count = 0;
    plugin_list->mcount = 0;
}

void initPluginData(PLUGIN_DATA* plugin)
{
    plugin->format[0] = '\0';
    plugin->library[0] = '\0';
    plugin->symbol[0] = '\0';
    plugin->extension[0] = '\0';
    plugin->desc[0] = '\0';
    plugin->example[0] = '\0';
    plugin->method[0] = '\0';
    plugin->deviceProtocol[0] = '\0';
    plugin->deviceHost[0] = '\0';
    plugin->devicePort[0] = '\0';
    plugin->request = REQUEST_READ_UNKNOWN;
    plugin->plugin_class = PLUGINUNKNOWN;
    plugin->external = PLUGINNOTEXTERNAL;
    plugin->status = PLUGINNOTOPERATIONAL;
    plugin->is_private = PLUGINPRIVATE;                    // All services are private: Not accessible to external users
    plugin->cachePermission = PLUGINCACHEDEFAULT;       // Data are OK or Not for the Client to Cache
    plugin->interfaceVersion = 1;                       // Maximum Interface Version
    plugin->pluginHandle = NULL;
    plugin->idamPlugin = NULL;
}

void printPluginList(FILE* fd, const PLUGINLIST* plugin_list)
{
    int i;
    for (i = 0; i < plugin_list->count; i++) {
        fprintf(fd, "Request    : %d\n", plugin_list->plugin[i].request);
        fprintf(fd, "Format     : %s\n", plugin_list->plugin[i].format);
        fprintf(fd, "Library    : %s\n", plugin_list->plugin[i].library);
        fprintf(fd, "Symbol     : %s\n", plugin_list->plugin[i].symbol);
        fprintf(fd, "Extension  : %s\n", plugin_list->plugin[i].extension);
        fprintf(fd, "Method     : %s\n", plugin_list->plugin[i].method);
        fprintf(fd, "Description: %s\n", plugin_list->plugin[i].desc);
        fprintf(fd, "Example    : %s\n", plugin_list->plugin[i].example);
        fprintf(fd, "Protocol   : %s\n", plugin_list->plugin[i].deviceProtocol);
        fprintf(fd, "Host       : %s\n", plugin_list->plugin[i].deviceHost);
        fprintf(fd, "Port       : %s\n", plugin_list->plugin[i].devicePort);
        fprintf(fd, "Class      : %d\n", plugin_list->plugin[i].plugin_class);
        fprintf(fd, "External   : %d\n", plugin_list->plugin[i].external);
        fprintf(fd, "Status     : %d\n", plugin_list->plugin[i].status);
        fprintf(fd, "Private    : %d\n", plugin_list->plugin[i].is_private);
        fprintf(fd, "cachePermission : %d\n", plugin_list->plugin[i].cachePermission);
        fprintf(fd, "interfaceVersion: %d\n\n", plugin_list->plugin[i].interfaceVersion);
    }
}

/**
 * Find the Plugin identity: return the reference id or -1 if not found.
 * @param request
 * @param plugin_list
 * @return
 */
int findPluginIdByRequest(int request, const PLUGINLIST* plugin_list)
{
    int i;
    for (i = 0; i < plugin_list->count; i++) {
        if (plugin_list->plugin[i].request == request) return i;
    }
    return -1;
}

/**
 * Find the Plugin identity: return the reference id or -1 if not found.
 * @param format
 * @param plugin_list
 * @return
 */
int findPluginIdByFormat(const char* format, const PLUGINLIST* plugin_list)
{
    int i;
    for (i = 0; i < plugin_list->count; i++) {
        if (STR_IEQUALS(plugin_list->plugin[i].format, format)) return i;
    }
    return -1;
}

/**
 * Find the Plugin identity: return the reference id or -1 if not found.
 * @param device
 * @param plugin_list
 * @return
 */
int findPluginIdByDevice(const char* device, const PLUGINLIST* plugin_list)
{
    int i;
    for (i = 0; i < plugin_list->count; i++) {
        if (plugin_list->plugin[i].plugin_class == PLUGINDEVICE && STR_IEQUALS(plugin_list->plugin[i].format, device)) {
            return i;
        }
    }
    return -1;
}

/**
 * Find the Plugin Request: return the request or REQUEST_READ_UNKNOWN if not found.
 * @param format
 * @param plugin_list
 * @return
 */
int findPluginRequestByFormat(const char* format, const PLUGINLIST* plugin_list)
{
    int i;
    for (i = 0; i < plugin_list->count; i++) {
        if (STR_IEQUALS(plugin_list->plugin[i].format, format)) return plugin_list->plugin[i].request;
    }
    return REQUEST_READ_UNKNOWN;
}

/**
 * Find the Plugin Request: return the request or REQUEST_READ_UNKNOWN if not found.
 * @param extension
 * @param plugin_list
 * @return
 */
int findPluginRequestByExtension(const char* extension, const PLUGINLIST* plugin_list)
{
    int i;
    for (i = 0; i < plugin_list->count; i++) {
        if (STR_IEQUALS(plugin_list->plugin[i].extension, extension)) return plugin_list->plugin[i].request;
    }
    return REQUEST_READ_UNKNOWN;
}

void initPluginList(PLUGINLIST* plugin_list)
{

    int i;

    // initialise the Plugin List and Allocate heap for the list

    plugin_list->count = 0;
    plugin_list->plugin = (PLUGIN_DATA*)malloc(REQUEST_PLUGIN_MCOUNT * sizeof(PLUGIN_DATA));
    plugin_list->mcount = REQUEST_PLUGIN_MCOUNT;

    for (i = 0; i < plugin_list->mcount; i++) {
        initPluginData(&plugin_list->plugin[i]);
    }

    //----------------------------------------------------------------------------------------------------------------------
    // Data Access Server Protocols

    // Generic

    strcpy(plugin_list->plugin[plugin_list->count].format, "GENERIC");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_GENERIC;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINOTHER;
    plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;
    strcpy(plugin_list->plugin[plugin_list->count].desc,
           "Generic Data Access request - no file format or server name specified, only the shot number");
    strcpy(plugin_list->plugin[plugin_list->count].example, "idamGetAPI(\"signal name\", \"12345\")");
    allocPluginList(plugin_list->count++, plugin_list);

#ifndef NOIDAMPLUGIN

    /*!
    Data via an IDAM client plugin can be accessed using either of two protocol names: IDAM or SERVER.
    These access services are identical.
    */
    initPluginData(&plugin_list->plugin[plugin_list->count]);
    strcpy(plugin_list->plugin[plugin_list->count].format, "IDAM");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_IDAM;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINSERVER;

    ENVIRONMENT* environment = getIdamServerEnvironment();

    if (environment->server_proxy[0] != '\0') {
        plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;
    }        // Public service if running as a PROXY

    if (!environment->external_user) {
        plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;
    }        // Public service for internal requests only

    strcpy(plugin_list->plugin[plugin_list->count].desc,
           "Data is accessed from an internal or external IDAM server. The server the client is connected to "
                   "acts as a proxy and passes the access request forward. Multiple servers can be chained "
                   "together.");
    strcpy(plugin_list->plugin[plugin_list->count].example, "idamGetAPI(\"ip\",\"IDAM::server:port/12345\")");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "SERVER");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_IDAM;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINSERVER;
    if (environment->server_proxy[0] != '\0')
        plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;        // Public service if running as a PROXY
    if (!environment->external_user)
        plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;        // Public service for internal requests only
    strcpy(plugin_list->plugin[plugin_list->count].desc,
           "Data is accessed from an internal or external IDAM server. The server the client is connected to "
                   "acts as a proxy and passes the access request forward. Multiple servers can be chained "
                   "together.");
    strcpy(plugin_list->plugin[plugin_list->count].example, "idamGetAPI(\"ip\",\"SERVER::server:port/12345\")");
    allocPluginList(plugin_list->count++, plugin_list);
#endif

#ifndef NOWEBPLUGIN

    /*!
    Data via a WEB browser plugin can be accessed using either of two protocol names: WEB or HTTP.
    These access services are identical.
    */
    strcpy(plugin_list->plugin[plugin_list->count].format, "WEB");
    strcpy(plugin_list->plugin[plugin_list->count].extension, "html");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_WEB;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINSERVER;
    strcpy(plugin_list->plugin[plugin_list->count].desc,
           "Data accessed via an internal or external http Web server. Access to external "
                   "html web pages is subject to authentication with the proxy web server.");
    strcpy(plugin_list->plugin[plugin_list->count].example,
           "idamGetAPI(\"trweb/idam_menu.html\",\"WEB::fuslwn.culham.ukaea.org.uk\")");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "HTTP");
    strcpy(plugin_list->plugin[plugin_list->count].extension, "html");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_WEB;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINSERVER;
    strcpy(plugin_list->plugin[plugin_list->count].desc,
           "Data accessed via an internal or external http Web server. Access to external "
                   "html web pages is subject to authentication with the proxy web server.");
    strcpy(plugin_list->plugin[plugin_list->count].example,
           "idamGetAPI(\"trweb/idam_menu.html\",\"HTTP::fuslwn.culham.ukaea.org.uk\")");
    allocPluginList(plugin_list->count++, plugin_list);
#endif

#ifndef NOMDSPLUSPLUGIN
    /*!
    Data via a MDSPlus Server can be accessed using either of three protocol names: MDS or MDS+ or MDSPLUS.
    These access services are identical.
    */
    strcpy(plugin_list->plugin[plugin_list->count].format, "MDS");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_MDS;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINSERVER;
    strcpy(plugin_list->plugin[plugin_list->count].desc,
           "Data accessed via an internal or external MDSPlus server. The latter may be subject to user authentication.");
    strcpy(plugin_list->plugin[plugin_list->count].example,
           "idamGetAPI(\"\\top.inputs:cur\",\"MDS::/trmast/159052601\")");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "MDS+");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_MDS;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINSERVER;
    strcpy(plugin_list->plugin[plugin_list->count].desc,
           "Data accessed via an internal or external MDSPlus server. The latter may be subject to user authentication.");
    strcpy(plugin_list->plugin[plugin_list->count].example,
           "idamGetAPI(\"\\top.inputs:cur\",\"MDS+::/trmast/159052601\")");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "MDSPLUS");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_MDS;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINSERVER;
    strcpy(plugin_list->plugin[plugin_list->count].desc,
           "Data accessed via an internal or external MDSPlus server. The latter may be subject to user authentication.");
    strcpy(plugin_list->plugin[plugin_list->count].example,
           "idamGetAPI(\"\\top.inputs:cur\",\"MDSPLUS::/trmast/159052601\")");
    allocPluginList(plugin_list->count++, plugin_list);
#endif

#ifndef NOSQLPLUGIN
    strcpy(plugin_list->plugin[plugin_list->count].format, "SQL");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_SQL;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINSERVER;
    strcpy(plugin_list->plugin[plugin_list->count].desc,
           "Data accessed from the IDAM SQL server. Now deprecated. Use the META library.");
    allocPluginList(plugin_list->count++, plugin_list);
#endif

#ifndef NOPPFPLUGIN
    strcpy(plugin_list->plugin[plugin_list->count].format,  "PPF");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_PPF;
    plugin_list->plugin[plugin_list->count].plugin_class   = PLUGINSERVER;	// Treat pathname as a URL
    strcpy(plugin_list->plugin[plugin_list->count].desc,  "Data accessed from the JET PPF server. This is the default data archive on JET so does not need to be explictly "
           "stated in the signal argument.");
    strcpy(plugin_list->plugin[plugin_list->count].example, "idamGetAPI(\"ipla\",\"magn/12345\")\n"
           "idamGetAPI(\"PPF::ipla\",\"magn/12345/\") - PPF is the default data archive on JET so does not need to be explictly stated\n"
           "idamGetAPI(\"ipla\",\"magn/12345\")\n"
           "idamGetAPI(\"ipla\",\"magn/12345/120\")  - use a specific sequence number\n"
           "idamGetAPI(\"ipla\",\"magn/12345/JETABC\")  - use a specific userid for private PPF files\n"
           "idamGetAPI(\"ipla\",\"magn/12345/JETABC\")  - combined use of specific sequence number and userid for private PPF files");
    allocPluginList(plugin_list->count++, plugin_list);
#endif

#ifndef NOJPFPLUGIN
    strcpy(plugin_list->plugin[plugin_list->count].format,  "JPF");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_JPF;
    plugin_list->plugin[plugin_list->count].plugin_class   = PLUGINSERVER;	// Treat pathname as a URL
    strcpy(plugin_list->plugin[plugin_list->count].desc,  "Data accessed from a JET JPF server.");
    strcpy(plugin_list->plugin[plugin_list->count].example, "idamGetAPI(\"DA/C1-IPLA\", \"JPF::/56000\")");
    allocPluginList(plugin_list->count++, plugin_list);
#endif

    //----------------------------------------------------------------------------------------------------------------------
    // File Formats

#ifndef NOIDAPLUGIN
    strcpy(plugin_list->plugin[plugin_list->count].format, "IDA");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_IDA;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Data accessed from a Legacy IDA3 file.");
    strcpy(plugin_list->plugin[plugin_list->count].example,
           "idamGetAPI(\"abc_my data\", \"IDA::/path/to/my/file123.45\")");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "IDA3");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_IDA;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Data accessed from a Legacy IDA3 file.");
    strcpy(plugin_list->plugin[plugin_list->count].example,
           "idamGetAPI(\"abc_my data\", \"IDA3::/path/to/my/file123.45\")");
    allocPluginList(plugin_list->count++, plugin_list);
#endif

#ifndef NONETCDFPLUGIN
    strcpy(plugin_list->plugin[plugin_list->count].format, "NETCDF");
    strcpy(plugin_list->plugin[plugin_list->count].extension, "nc");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_CDF;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Data accessed from a netCDF-3 or netCDF-4 file.");
    strcpy(plugin_list->plugin[plugin_list->count].example,
           "idamGetAPI(\"/group/group/variable\", \"/path/to/my/file.nc\")\n"
                   "idamGetAPI(\"/group/group/variable\", \"NETCDF::/path/to/my/file.nc\")\n"
                   "idamGetAPI(\"/group/group/attribute\", \"/path/to/my/file.nc\")\treturns the value of a group attribute\n"
                   "idamGetAPI(\"/group/group/variable.attribute\", \"/path/to/my/file.nc\")\treturns the value of a variable attribute\n"
                   "idamGetAPI(\"/group/group\", \"/path/to/my/file.nc\")\treturns a sub tree data structure with the group contents");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "CDF");
    strcpy(plugin_list->plugin[plugin_list->count].extension, "nc");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_CDF;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Data accessed from a netCDF-3 or netCDF-4 file.");
    strcpy(plugin_list->plugin[plugin_list->count].example,
           "idamGetAPI(\"/group/group/variable\", \"/path/to/my/file.nc\")\n"
                   "idamGetAPI(\"/group/group/variable\", \"CDF::/path/to/my/file.nc\")\n"
                   "idamGetAPI(\"/group/group/attribute\", \"/path/to/my/file.nc\")\treturns the value of a group attribute\n"
                   "idamGetAPI(\"/group/group/variable.attribute\", \"/path/to/my/file.nc\")\treturns the value of a variable attribute\n"
                   "idamGetAPI(\"/group/group\", \"/path/to/my/file.nc\")\treturns a sub tree data structure with the group contents");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "NETCDF");
    strcpy(plugin_list->plugin[plugin_list->count].extension, "cdf");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_CDF;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Data accessed from a netCDF-3 or netCDF-4 file.");
    strcpy(plugin_list->plugin[plugin_list->count].example,
           "idamGetAPI(\"/group/group/variable\", \"/path/to/my/file.cdf\")\n"
                   "idamGetAPI(\"/group/group/variable\", \"NETCDF::/path/to/my/file.cdf\")\n"
                   "idamGetAPI(\"/group/group/attribute\", \"/path/to/my/file.cdf\")\treturns the value of a group attribute\n"
                   "idamGetAPI(\"/group/group/variable.attribute\", \"/path/to/my/file.cdf\")\treturns the value of a variable attribute\n"
                   "idamGetAPI(\"/group/group\", \"/path/to/my/file.cdf\")\treturns a sub tree data structure with the group contents");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "CDF");
    strcpy(plugin_list->plugin[plugin_list->count].extension, "cdf");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_CDF;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Data accessed from a netCDF-3 or netCDF-4 file.");
    strcpy(plugin_list->plugin[plugin_list->count].example,
           "idamGetAPI(\"/group/group/variable\", \"/path/to/my/file.cdf\")\n"
                   "idamGetAPI(\"/group/group/variable\", \"CDF::/path/to/my/file.cdf\")\n"
                   "idamGetAPI(\"/group/group/attribute\", \"/path/to/my/file.cdf\")\treturns the value of a group attribute\n"
                   "idamGetAPI(\"/group/group/variable.attribute\", \"/path/to/my/file.cdf\")\treturns the value of a variable attribute\n"
                   "idamGetAPI(\"/group/group\", \"/path/to/my/file.cdf\")\treturns a sub tree data structure with the group contents");
    allocPluginList(plugin_list->count++, plugin_list);
#endif

#ifndef NOHDF5PLUGIN
    strcpy(plugin_list->plugin[plugin_list->count].format, "HDF5");
    strcpy(plugin_list->plugin[plugin_list->count].extension, "hf");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_HDF5;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Data accessed from a HDF5 file.");
    strcpy(plugin_list->plugin[plugin_list->count].example,
           "idamGetAPI(\"/group/group/variable\", \"/path/to/my/file.hf\")\n"
                   "idamGetAPI(\"/group/group/variable\", \"HDF5::/path/to/my/file.hf\")");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "HDF");
    strcpy(plugin_list->plugin[plugin_list->count].extension, "hf");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_HDF5;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Data accessed from a HDF5 file.");
    strcpy(plugin_list->plugin[plugin_list->count].example,
           "idamGetAPI(\"/group/group/variable\", \"/path/to/my/file.hf\")\n"
                   "idamGetAPI(\"/group/group/variable\", \"HDF::/path/to/my/file.hf\")");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "HDF5");
    strcpy(plugin_list->plugin[plugin_list->count].extension, "h5");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_HDF5;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Data accessed from a HDF5 file.");
    strcpy(plugin_list->plugin[plugin_list->count].example,
           "idamGetAPI(\"/group/group/variable\", \"/path/to/my/file.h5\")\n"
                   "idamGetAPI(\"/group/group/variable\", \"HDF5::/path/to/my/file.h5\")");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "HDF");
    strcpy(plugin_list->plugin[plugin_list->count].extension, "h5");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_HDF5;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Data accessed from a HDF5 file.");
    strcpy(plugin_list->plugin[plugin_list->count].example,
           "idamGetAPI(\"/group/group/variable\", \"/path/to/my/file.h5\")\n"
                   "idamGetAPI(\"/group/group/variable\", \"HDF::/path/to/my/file.h5\")");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "HDF5");
    strcpy(plugin_list->plugin[plugin_list->count].extension, "hdf5");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_HDF5;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Data accessed from a HDF5 file.");
    strcpy(plugin_list->plugin[plugin_list->count].example,
           "idamGetAPI(\"/group/group/variable\", \"/path/to/my/file.hdf5\")\n"
                   "idamGetAPI(\"/group/group/variable\", \"HDF5::/path/to/my/file.hdf5\")");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "HDF");
    strcpy(plugin_list->plugin[plugin_list->count].extension, "hdf5");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_HDF5;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Data accessed from a HDF5 file.");
    strcpy(plugin_list->plugin[plugin_list->count].example,
           "idamGetAPI(\"/group/group/variable\", \"/path/to/my/file.hdf5\")\n"
                   "idamGetAPI(\"/group/group/variable\", \"HDF::/path/to/my/file.hdf5\")");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "HDF5");
    strcpy(plugin_list->plugin[plugin_list->count].extension, "hd5");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_HDF5;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Data accessed from a HDF5 file.");
    strcpy(plugin_list->plugin[plugin_list->count].example,
           "idamGetAPI(\"/group/group/variable\", \"/path/to/my/file.hd5\")\n"
                   "idamGetAPI(\"/group/group/variable\", \"HDF5::/path/to/my/file.hd5\")");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "HDF");
    strcpy(plugin_list->plugin[plugin_list->count].extension, "hd5");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_HDF5;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Data accessed from a HDF5 file.");
    strcpy(plugin_list->plugin[plugin_list->count].example,
           "idamGetAPI(\"/group/group/variable\", \"/path/to/my/file.hd5\")\n"
                   "idamGetAPI(\"/group/group/variable\", \"HDF::/path/to/my/file.hd5\")");
    allocPluginList(plugin_list->count++, plugin_list);
#endif

#ifndef NOXMLPLUGIN
    strcpy(plugin_list->plugin[plugin_list->count].format, "XML");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_XML;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Data accessed from a XML file.");
    allocPluginList(plugin_list->count++, plugin_list);
#endif

#ifndef NOUFILEPLUGIN
    strcpy(plugin_list->plugin[plugin_list->count].format, "UFILE");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_UFILE;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Data accessed from a UFILE file.");
    strcpy(plugin_list->plugin[plugin_list->count].example, "idamGetAPI(\"\", \"UFILE::/path/to/my/u/file\")");
    allocPluginList(plugin_list->count++, plugin_list);
#endif

#ifndef NOBINARYPLUGIN
    strcpy(plugin_list->plugin[plugin_list->count].format, "BIN");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_FILE;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Return a Binary file.");
    strcpy(plugin_list->plugin[plugin_list->count].example, "idamGetAPI(\"\", \"BIN::/path/to/my/binary/file\")");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "BINARY");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_FILE;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Return a Binary file.");
    strcpy(plugin_list->plugin[plugin_list->count].example, "idamGetAPI(\"\", \"BINARY::/path/to/my/binary/file\")");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "JPG");
    strcpy(plugin_list->plugin[plugin_list->count].extension, "jpg");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_FILE;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Return a JPG file.");
    strcpy(plugin_list->plugin[plugin_list->count].example, "idamGetAPI(\"\", \"/path/to/my/file.jpg\")\n"
            "idamGetAPI(\"\", \"JPG::/path/to/my/file.jpg\")");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "NIDA");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_FILE;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Return a NIDA (Not an IDA) file.");
    strcpy(plugin_list->plugin[plugin_list->count].example, "idamGetAPI(\"\", \"NIDA::/path/to/my/file\")");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "CSV");
    strcpy(plugin_list->plugin[plugin_list->count].extension, "csv");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_FILE;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Return a CSV ASCII file.");
    strcpy(plugin_list->plugin[plugin_list->count].example, "idamGetAPI(\"\", \"/path/to/my/file.csv\")\n"
            "idamGetAPI(\"\", \"CSV::/path/to/my/file.csv\")");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "TIF");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_FILE;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Return a TIF file.");
    strcpy(plugin_list->plugin[plugin_list->count].example, "idamGetAPI(\"\", \"TIF::/path/to/my/file\")");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "IPX");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_FILE;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Return an IPX file.");
    strcpy(plugin_list->plugin[plugin_list->count].example, "idamGetAPI(\"\", \"IPX::/path/to/my/file\")");
    allocPluginList(plugin_list->count++, plugin_list);
#endif

//----------------------------------------------------------------------------------------------------------------------
// Legacy, superceded, deprecated

#ifndef HIERARCHICAL_DATA
    strcpy(plugin_list->plugin[plugin_list->count].format, "HDATA");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_HDATA;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Data accessed from the EFIT++ XML Meta data file.");
    allocPluginList(plugin_list->count++, plugin_list);
#endif

// Testing

#ifndef NOTESTPLUGIN
    strcpy(plugin_list->plugin[plugin_list->count].format, "TEST");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_NEW_PLUGIN;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINOTHER;
    plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Test a New Plugin");
    allocPluginList(plugin_list->count++, plugin_list);
#endif

#ifndef NONOTHINGPLUGIN
    strcpy(plugin_list->plugin[plugin_list->count].format, "NOTHING");
    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_NOTHING;
    plugin_list->plugin[plugin_list->count].plugin_class = PLUGINOTHER;
    plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;
    strcpy(plugin_list->plugin[plugin_list->count].desc, "Generate Test Data");
    allocPluginList(plugin_list->count++, plugin_list);
#endif

    //----------------------------------------------------------------------------------------------------------------------
    // Complete Common Registration

    for (i = 0; i < plugin_list->count; i++) {
        plugin_list->plugin[i].external = PLUGINNOTEXTERNAL;        // These are all linked as internal functions
        plugin_list->plugin[i].status = PLUGINOPERATIONAL;        // By default all these are available
        plugin_list->plugin[i].cachePermission = PLUGINCACHEDEFAULT;    // OK or not for Client and Server to Cache
    }

    //----------------------------------------------------------------------------------------------------------------------
    // Server-Side Functions

    int pluginCount = plugin_list->count;        // Number of internal plugins before adding server-side

    strcpy(plugin_list->plugin[plugin_list->count].format, "SERVERSIDE");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "SSIDE");
    allocPluginList(plugin_list->count++, plugin_list);

    strcpy(plugin_list->plugin[plugin_list->count].format, "SS");
    allocPluginList(plugin_list->count++, plugin_list);

    for (i = pluginCount; i < plugin_list->count; i++) {
        plugin_list->plugin[i].request = REQUEST_READ_GENERIC;
        plugin_list->plugin[i].plugin_class = PLUGINFUNCTION;
        strcpy(plugin_list->plugin[i].symbol, "SERVERSIDE");
        strcpy(plugin_list->plugin[i].desc, "Inbuilt Serverside functions");
        plugin_list->plugin[i].is_private = PLUGINPUBLIC;
        plugin_list->plugin[i].library[0] = '\0';
        plugin_list->plugin[i].pluginHandle = NULL;
        plugin_list->plugin[i].external = PLUGINNOTEXTERNAL;        // These are all linked as internal functions
        plugin_list->plugin[i].status = PLUGINOPERATIONAL;        // By default all these are available
        plugin_list->plugin[i].cachePermission = PLUGINCACHEDEFAULT;    // OK or not for Client and Server to Cache
    }

    //----------------------------------------------------------------------------------------------------------------------
    // Read all other plugins registered via the server configuration file.

    {
        //PLUGINFUNP idamPlugin;	// Plugin Function Pointer - the external data reader function located within a shared library
        int i, j, lstr, err, pluginID;
        int rc = 0;
        int pluginCount = plugin_list->count;                // Number of internal plugins before adding external sources
        static int offset = 0;
        char csvChar = ',';
        char buffer[STRING_LENGTH];
        char* root;
        char* config = getenv("UDA_PLUGIN_CONFIG");            // Server plugin configuration file
        FILE* conf = NULL;
        char* filename = "udaPlugins.conf";                // Default name
        char* work = NULL, * csv, * next, * p;

        // Locate the plugin registration file

        if (config == NULL) {
            root = getenv("UDA_SERVERROOT");                // Where udaPlugins.conf is located by default
            if (root == NULL) {
                lstr = (int)strlen(filename) + 3;
                work = (char*)malloc(lstr * sizeof(char));
                sprintf(work, "./%s", filename);            // Default ROOT is the server's Working Directory
            } else {
                lstr = (int)strlen(filename) + (int)strlen(root) + 2;
                work = (char*)malloc(lstr * sizeof(char));
                sprintf(work, "%s/%s", root, filename);
            }
        } else {
            lstr = (int)strlen(config) + 1;
            work = (char*)malloc(lstr * sizeof(char));            // Alternative File Name and Path
            strcpy(work, config);
        }

        // Read the registration file

        errno = 0;
        if ((conf = fopen(work, "r")) == NULL || errno != 0) {
            err = 999;
            addIdamError(SYSTEMERRORTYPE, "idamServerPlugin", errno, strerror(errno));
            addIdamError(SYSTEMERRORTYPE, "idamServerPlugin", err,
                         "No Server Plugin Configuration File found!");
            if (conf != NULL) {
                fclose(conf);
            }
            free((void*)work);
            return;
        }

        if (work != NULL) free((void*)work);

        /*
        record format: csv, empty records ignored, comment begins #, max record size 1023;
        Organisation - context dependent - 10 fields
        Description field must not contain the csvChar character- ','
        A * (methodName) in field 5 is an ignorable placeholder

        1> Server plugins
        targetFormat,formatClass="server",librarySymbol,libraryName,methodName,interface,cachePermission,publicUse=,description,example
               2> Function library plugins
        targetFormat,formatClass="function",librarySymbol,libraryName,methodName,interface,cachePermission,publicUse,description,example
               3> File format
        targetFormat,formatClass="file",librarySymbol[.methodName],libraryName,fileExtension,interface,cachePermission,publicUse,description,example

               4> Internal Serverside function
               targetFormat,formatClass="function",librarySymbol="serverside",methodName,interface,cachePermission,publicUse,description,example
               5> External Device server re-direction
               targetFormat,formatClass="device",deviceProtocol,deviceHost,devicePort,interface,cachePermission,publicUse,description,example

        cachePermission and publicUse may use one of the following values: "Y|N,1|0,T|F,True|False"
        */

        while (fgets(buffer, STRING_LENGTH, conf) != NULL) {
            convertNonPrintable2(buffer);
            LeftTrimString(TrimString(buffer));
            do {
                if (buffer[0] == '#') break;
                if (strlen(buffer) == 0) break;
                next = buffer;
                initPluginData(&plugin_list->plugin[plugin_list->count]);
                for (i = 0; i < 10; i++) {
                    csv = strchr(next, csvChar);                // Split the string
                    if (csv != NULL && i <= 8)
                        csv[0] = '\0';            // Extract the sub-string ignoring the example - has a comma within text
                    LeftTrimString(TrimString(next));
                    switch (i) {

                        case 0:
                            // File Format or Server Protocol or Library name or Device name etc.
                            strcpy(plugin_list->plugin[plugin_list->count].format, LeftTrimString(next));
                            // If the Format or Protocol is Not unique, the plugin that is selected will be the first one registered: others will be ignored.
                            break;

                        case 1:    // Plugin class: File, Server, Function or Device
                            plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
                            if (STR_IEQUALS(LeftTrimString(next), "server")) {
                                plugin_list->plugin[plugin_list->count].plugin_class = PLUGINSERVER;
                            } else if (STR_IEQUALS(LeftTrimString(next), "function")) {
                                plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFUNCTION;
                            } else if (STR_IEQUALS(LeftTrimString(next), "file")) {
                                plugin_list->plugin[plugin_list->count].plugin_class = PLUGINFILE;
                            } else if (STR_IEQUALS(LeftTrimString(next), "device")) {
                                plugin_list->plugin[plugin_list->count].plugin_class = PLUGINDEVICE;
                            }
                            break;

                        case 2:
                            // Allow the same symbol (name of data access reader function or plugin entrypoint symbol) but from different libraries!
                            if (plugin_list->plugin[plugin_list->count].plugin_class != PLUGINDEVICE) {
                                strcpy(plugin_list->plugin[plugin_list->count].symbol, LeftTrimString(next));
                                plugin_list->plugin[plugin_list->count].external = PLUGINEXTERNAL;        // External (not linked) shared library

                                if (plugin_list->plugin[plugin_list->count].plugin_class ==
                                    PLUGINFILE) {            // Plugin method name using a dot syntax
                                    if ((p = strchr(plugin_list->plugin[plugin_list->count].symbol, '.')) != NULL) {
                                        p[0] = '\0';                                // Remove the method name from the symbol text
                                        strcpy(plugin_list->plugin[plugin_list->count].method,
                                               &p[1]);        // Save the method name
                                    }
                                }

                            } else {
                                // Device name Substitution protocol
                                strcpy(plugin_list->plugin[plugin_list->count].deviceProtocol, LeftTrimString(next));
                            }
                            break;

                        case 3:    // Server Host or Name of the shared library - can contain multiple plugin symbols so may not be unique
                            if (plugin_list->plugin[plugin_list->count].plugin_class != PLUGINDEVICE) {
                                strcpy(plugin_list->plugin[plugin_list->count].library, LeftTrimString(next));
                            } else {
                                strcpy(plugin_list->plugin[plugin_list->count].deviceHost, LeftTrimString(next));
                            }
                            break;

                        case 4:    // File extension or Method Name or Port number
// TO DO: make extensions a list of valid extensions to minimise plugin duplication
                            if (plugin_list->plugin[plugin_list->count].plugin_class != PLUGINDEVICE) {
                                if (plugin_list->plugin[plugin_list->count].plugin_class == PLUGINFILE)
                                    strcpy(plugin_list->plugin[plugin_list->count].extension, next);
                                else if (next[0] != '*')
                                    strcpy(plugin_list->plugin[plugin_list->count].method,
                                           next);    // Ignore the placeholder character *
                            } else {
                                strcpy(plugin_list->plugin[plugin_list->count].devicePort, LeftTrimString(next));
                            }
                            break;

                        case 5:    // Minimum Plugin Interface Version
                            if (strlen(next) > 0) {
                                plugin_list->plugin[plugin_list->count].interfaceVersion = (unsigned short)atoi(next);
                            }
                            break;

                        case 6:    // Permission to Cache returned values

                            strcpy(plugin_list->plugin[plugin_list->count].desc, LeftTrimString(next));
                            if (plugin_list->plugin[plugin_list->count].desc[0] != '\0' && (
                                    plugin_list->plugin[plugin_list->count].desc[0] == 'Y' ||
                                    plugin_list->plugin[plugin_list->count].desc[0] == 'y' ||
                                    plugin_list->plugin[plugin_list->count].desc[0] == 'T' ||
                                    plugin_list->plugin[plugin_list->count].desc[0] == 't' ||
                                    plugin_list->plugin[plugin_list->count].desc[0] == '1')) {
                                plugin_list->plugin[plugin_list->count].cachePermission = PLUGINOKTOCACHE;        // True
                                plugin_list->plugin[plugin_list->count].desc[0] = '\0';
                            } else
                                plugin_list->plugin[plugin_list->count].cachePermission = PLUGINNOTOKTOCACHE;        // False

                            break;

                        case 7:    // Private or Public plugin - i.e. available to external users

                            strcpy(plugin_list->plugin[plugin_list->count].desc, LeftTrimString(next));
                            if (plugin_list->plugin[plugin_list->count].desc[0] != '\0' && (
                                    plugin_list->plugin[plugin_list->count].desc[0] == 'Y' ||
                                    plugin_list->plugin[plugin_list->count].desc[0] == 'y' ||
                                    plugin_list->plugin[plugin_list->count].desc[0] == 'T' ||
                                    plugin_list->plugin[plugin_list->count].desc[0] == 't' ||
                                    plugin_list->plugin[plugin_list->count].desc[0] == '1')) {
                                plugin_list->plugin[plugin_list->count].is_private = PLUGINPUBLIC;
                                plugin_list->plugin[plugin_list->count].desc[0] = '\0';
                            }

                            break;

                        case 8:    // Description

                            strcpy(plugin_list->plugin[plugin_list->count].desc, LeftTrimString(next));
                            break;

                        case 9:    // Example

                            LeftTrimString(next);
                            p = strchr(next, '\n');
                            if (p != NULL) p[0] = '\0';
                            strcpy(plugin_list->plugin[plugin_list->count].example, LeftTrimString(next));
                            break;

                        default:
                            break;

                    }
                    if (csv != NULL) next = &csv[1];    // Next element starting point
                }

                plugin_list->plugin[plugin_list->count].request =
                        REQUEST_READ_START + offset++;    // Issue Unique request ID

                plugin_list->plugin[plugin_list->count].pluginHandle = (void*)NULL;        // Library handle: Not opened
                plugin_list->plugin[plugin_list->count].status = PLUGINNOTOPERATIONAL;    // Not yet available

                // Internal Serverside function ?

                if (plugin_list->plugin[plugin_list->count].plugin_class == PLUGINFUNCTION &&
                    STR_IEQUALS(plugin_list->plugin[plugin_list->count].symbol, "serverside") &&
                    plugin_list->plugin[plugin_list->count].library[0] == '\0') {
                    strcpy(plugin_list->plugin[plugin_list->count].symbol, "SERVERSIDE");
                    plugin_list->plugin[plugin_list->count].request = REQUEST_READ_GENERIC;
                    plugin_list->plugin[plugin_list->count].external = PLUGINNOTEXTERNAL;
                    plugin_list->plugin[plugin_list->count].status = PLUGINOPERATIONAL;
                }

                // Check this library has not already been opened: Preserve the library handle for use if already opened.

                pluginID = -1;

                // States:
                // 1. library not opened: open library and locate symbol (Only if the Class is SERVER or FUNCTION or File)
                // 2. library opened, symbol not located: locate symbol
                // 3. library opened, symbol located: re-use

                for (j = pluginCount; j < plugin_list->count - 1; j++) {            // External sources only
                    if (plugin_list->plugin[j].external == PLUGINEXTERNAL &&
                        plugin_list->plugin[j].status == PLUGINOPERATIONAL &&
                        plugin_list->plugin[j].pluginHandle != NULL &&
                        STR_IEQUALS(plugin_list->plugin[j].library, plugin_list->plugin[plugin_list->count].library)) {

                        // Library may contain different symbols

                        if (STR_IEQUALS(plugin_list->plugin[j].symbol,
                                        plugin_list->plugin[plugin_list->count].symbol) &&
                            plugin_list->plugin[j].idamPlugin != NULL) {
                            rc = 0;
                            plugin_list->plugin[plugin_list->count].idamPlugin = plugin_list->plugin[j].idamPlugin;    // re-use
                        } else {

                            // New symbol in opened library

                            if (plugin_list->plugin[plugin_list->count].plugin_class != PLUGINDEVICE) {
                                rc = getPluginAddress(
                                        &plugin_list->plugin[j].pluginHandle,                // locate symbol
                                        plugin_list->plugin[j].library,
                                        plugin_list->plugin[plugin_list->count].symbol,
                                        &plugin_list->plugin[plugin_list->count].idamPlugin);
                            }
                        }

                        plugin_list->plugin[plugin_list->count].pluginHandle = plugin_list->plugin[j].pluginHandle;
                        pluginID = j;
                        break;
                    }
                }

                if (pluginID == -1) {                                    // open library and locate symbol
                    if (plugin_list->plugin[plugin_list->count].plugin_class != PLUGINDEVICE) {
                        rc = getPluginAddress(&plugin_list->plugin[plugin_list->count].pluginHandle,
                                              plugin_list->plugin[plugin_list->count].library,
                                              plugin_list->plugin[plugin_list->count].symbol,
                                              &plugin_list->plugin[plugin_list->count].idamPlugin);
                    }
                }

                if (rc == 0) {
                    plugin_list->plugin[plugin_list->count].status = PLUGINOPERATIONAL;
                }

                allocPluginList(plugin_list->count++, plugin_list);

            } while (0);
        }

        fclose(conf);
    }
}

int idamServerRedirectStdStreams(int reset)
{
    // Any OS messages will corrupt xdr streams so re-divert IO from plugin libraries to a temporary file

    static FILE* originalStdFH = NULL;
    static FILE* originalErrFH = NULL;
    static FILE* mdsmsgFH = NULL;

    char* env;
    static char tempFile[MAXPATH];

    static int singleFile = 0;

    if (!reset) {
        if (!singleFile) {
            env = getenv("UDA_PLUGIN_DEBUG_SINGLEFILE");        // Use a single file for all plugin data requests
            if (env != NULL) singleFile = 1;                    // Define IDAM_PLUGIN_DEBUG to retain the file
        }

        if (mdsmsgFH != NULL && singleFile) {
            stdout = mdsmsgFH;                                  // Redirect all IO to a temporary file
            stderr = mdsmsgFH;
            return 0;
        }

        originalStdFH = stdout;                                 // Retain current values
        originalErrFH = stderr;
        mdsmsgFH = NULL;

        UDA_LOG(UDA_LOG_DEBUG, "Redirect standard output to temporary file\n");

        env = getenv("UDA_PLUGIN_REDIVERT");

        if (env == NULL) {
            if ((env = getenv("UDA_WORK_DIR")) != NULL) {
                sprintf(tempFile, "%s/idamPLUGINXXXXXX", env);
            } else {
                strcpy(tempFile, "/tmp/idamPLUGINXXXXXX");
            }
        } else {
            strcpy(tempFile, env);
        }

        // Open the message Trap

        errno = 0;
        int fd = mkstemp(tempFile);
        if (fd < 0 || errno != 0) {
            int err = (errno != 0) ? errno : 994;
            THROW_ERROR(err, "Unable to Obtain a Temporary File Name");
        }

        mdsmsgFH = fdopen(fd, "a");

        if (mdsmsgFH == NULL || errno != 0) {
            THROW_ERROR(999, "Unable to Trap Plugin Error Messages.");
        }

        stdout = mdsmsgFH; // Redirect to a temporary file
        stderr = mdsmsgFH;
    } else {
        if (mdsmsgFH != NULL) {
            UDA_LOG(UDA_LOG_DEBUG, "Resetting original file handles and removing temporary file\n");

            if (!singleFile) {
                if (mdsmsgFH != NULL) {
                    errno = 0;
                    int rc = fclose(mdsmsgFH);
                    if (rc) {
                        int err = errno;
                        THROW_ERROR(err, strerror(err));
                    }
                }
                mdsmsgFH = NULL;
                if (getenv("UDA_PLUGIN_DEBUG") == NULL) {
                    errno = 0;
                    int rc = remove(tempFile);    // Delete the temporary file
                    if (rc) {
                        int err = errno;
                        THROW_ERROR(err, strerror(err));
                    }
                    tempFile[0] = '\0';
                }
            }

            stdout = originalStdFH;
            stderr = originalErrFH;

        } else {

            UDA_LOG(UDA_LOG_DEBUG, "Resetting original file handles\n");

            stdout = originalStdFH;
            stderr = originalErrFH;
        }
    }

    return 0;
}

// 1. open configuration file
// 2. read plugin details
//   2.1 format
//   2.2 file or server
//   2.3 library name
//   2.4 symbol name
// 3. check format is unique
// 4. issue a request ID
// 5. open the library
// 6. get plugin function address
// 7. close the file
int idamServerPlugin(REQUEST_BLOCK* request_block, DATA_SOURCE* data_source, SIGNAL_DESC* signal_desc,
                     const PLUGINLIST* plugin_list)
{
    int err = 0;
    char* token = NULL;
    char work[STRING_LENGTH];

    UDA_LOG(UDA_LOG_DEBUG, "Start\n");

    //----------------------------------------------------------------------------
    // Start of Error Trap

    do {

        //----------------------------------------------------------------------------------------------
        // Decode the API Arguments: determine appropriate data reader plug-in

        if ((err = makeServerRequestBlock(request_block, *plugin_list)) != 0) break;

        UDA_LOG(UDA_LOG_DEBUG, "request_block\n");
        printRequestBlock(*request_block);

        //----------------------------------------------------------------------------------------------
        // Does the Path to Private Files contain hierarchical components not seen by the server?
        // If so make a substitution to resolve path problems.

        if (strlen(request_block->server) == 0 && request_block->request != REQUEST_READ_SERVERSIDE) {
            // Must be a File plugin
            if ((err = pathReplacement(request_block->path, getIdamServerEnvironment())) != 0) break;
        }

        //----------------------------------------------------------------------
        // Some legacy stuff ....

        if (request_block->request == REQUEST_READ_IDA) {
            parseIDAPath(request_block);
        } else {
            if (request_block->request == REQUEST_READ_XML) {
                parseXMLPath(request_block);
            }
        }

        //----------------------------------------------------------------------
        // Copy request details into the data_source structure mimicking a SQL query

        strcpy(data_source->source_alias, TrimString(request_block->file));
        strcpy(data_source->filename, TrimString(request_block->file));
        strcpy(data_source->path, TrimString(request_block->path));

        copyString(TrimString(request_block->signal), signal_desc->signal_name, MAXNAME);

        strcpy(data_source->server, TrimString(request_block->server));

        strcpy(data_source->format, TrimString(request_block->format));
        strcpy(data_source->archive, TrimString(request_block->archive));
        strcpy(data_source->device_name, TrimString(request_block->device_name));

        data_source->exp_number = request_block->exp_number;
        data_source->pass = request_block->pass;
        data_source->type = ' ';

        // Legacy Exceptions ...

        switch (request_block->request) {

            case REQUEST_READ_MDS:

                if (strlen(signal_desc->signal_name) == MAXNAME - 1) {
                    copyString(TrimString(request_block->signal), signal_desc->xml, MAXMETA);    // Pass via XML member
                    signal_desc->signal_name[0] = '\0';
                }
                break;

            case REQUEST_READ_NOTHING:

                if (data_source->exp_number == 0 && data_source->pass == -1) {    // May be passed in Path String
                    strcpy(work, request_block->path);
                    if (work[0] == '/' && (token = strtok(work, "/")) != NULL) {    // Tokenise the remaining string
                        if (IsNumber(token)) {                    // Is the First token an integer number?
                            request_block->exp_number = atoi(token);
                            if ((token = strtok(NULL, "/")) != NULL) {        // Next Token
                                if (IsNumber(token)) {
                                    request_block->pass = atoi(token);        // Must be the Pass number
                                } else {
                                    strcpy(request_block->tpass, token);        // anything else
                                }
                            }
                        }
                        data_source->exp_number = request_block->exp_number;        // Size of Data Block
                        data_source->pass = request_block->pass;        // Compressible or Not
                    }
                }
                break;
        }

        if (err != 0) break;

        //------------------------------------------------------------------------------------------------
        // Trap any unexpected output to stdout or stderr

        //------------------------------------------------------------------------------------------------
        // Locate and Execute the Required Plugin

        //------------------------------------------------------------------------------------------------
        // End of Error Trap

    } while (0);

    UDA_LOG(UDA_LOG_DEBUG, "End\n");

    return err;
}

//------------------------------------------------------------------------------------------------
// Provenance gathering plugin with a separate database.
// Functionality exposed to both server (special plugin with standard methods)
// and client application (behaves as a normal plugin)
//
// Server needs are (private to the server):
//	record (put) the original and the actual signal and source terms with the source file DOI
//	record (put) the server log record
// Client needs are (the plugin exposes these to the client in the regular manner):
//	list all provenance records for a specific client DOI - must be given
//	change provenance records status to closed
//	delete all closed records for a specific client DOI
//
// changePlugin option disabled in this context
// private malloc log and userdefinedtypelist

int idamProvenancePlugin(CLIENT_BLOCK* client_block, REQUEST_BLOCK* original_request_block,
                         DATA_SOURCE* data_source, SIGNAL_DESC* signal_desc, const PLUGINLIST* plugin_list,
                         char* logRecord)
{

    if (strcmp(client_block->DOI, "") || strlen(client_block->DOI) == 0) return 0;    // No Provenance to Capture

    // Identify the Provenance Gathering plugin (must be a function library type plugin)

    static short plugin_id = -2;
    static int execMethod = 1;        // The default method used to write efficiently to the backend SQL server
    char* env = NULL;

    struct timeval tv_start, tv_stop;

    gettimeofday(&tv_start, NULL);

    ENVIRONMENT* environment = getIdamServerEnvironment();

    if (plugin_id == -2) {        // On initialisation
        plugin_id = -1;
        if ((env = getenv("UDA_PROVENANCE_PLUGIN")) !=
            NULL) {                // Must be set in the server startup script
            UDA_LOG(UDA_LOG_DEBUG, "Plugin name: %s\n", env);
            int id = findPluginIdByFormat(env, plugin_list); // Must be defined in the server plugin configuration file
            UDA_LOG(UDA_LOG_DEBUG, "Plugin id: %d\n", id);
            if (id >= 0) {
                UDA_LOG(UDA_LOG_DEBUG, "plugin_list->plugin[id].plugin_class == PLUGINFUNCTION = %d\n",
                        plugin_list->plugin[id].plugin_class == PLUGINFUNCTION);
                UDA_LOG(UDA_LOG_DEBUG, "!environment->external_user = %d\n", !environment->external_user);
                UDA_LOG(UDA_LOG_DEBUG, "plugin_list->plugin[id].status == PLUGINOPERATIONAL = %d\n",
                        plugin_list->plugin[id].status == PLUGINOPERATIONAL);
                UDA_LOG(UDA_LOG_DEBUG, "plugin_list->plugin[id].pluginHandle != NULL = %d\n",
                        plugin_list->plugin[id].pluginHandle != NULL);
                UDA_LOG(UDA_LOG_DEBUG, "plugin_list->plugin[id].idamPlugin   != NULL = %d\n",
                        plugin_list->plugin[id].idamPlugin != NULL);
            }
            if (id >= 0 &&
                plugin_list->plugin[id].plugin_class == PLUGINFUNCTION &&
                !environment->external_user &&
                plugin_list->plugin[id].status == PLUGINOPERATIONAL &&
                plugin_list->plugin[id].pluginHandle != NULL &&
                plugin_list->plugin[id].idamPlugin != NULL) {
                plugin_id = id;
            }
        }
        if ((env = getenv("UDA_PROVENANCE_EXEC_METHOD")) != NULL) execMethod = atoi(env);
    }

    UDA_LOG(UDA_LOG_DEBUG, "Plugin id: %d\n", plugin_id);

    if (plugin_id <= 0) return 0;    // Not possible to record anything - no provenance plugin!

    REQUEST_BLOCK request_block;
    initRequestBlock(&request_block);
    strcpy(request_block.api_delim, "::");
    strcpy(request_block.source, "");

    // need 1> record the original and the actual signal and source terms with the source file DOI
    // mimic a client request

    if (logRecord == NULL || strlen(logRecord) == 0) {
        sprintf(request_block.signal, "%s::putSignal(uuid='%s',requestedSignal='%s',requestedSource='%s', "
                "trueSignal='%s', trueSource='%s', trueSourceDOI='%s', execMethod=%d, status=new)",
                plugin_list->plugin[plugin_id].format, client_block->DOI,
                original_request_block->signal, original_request_block->source,
                signal_desc->signal_name, data_source->path, "", execMethod);
    } else {

        // need 2> record the server log record

        sprintf(request_block.signal, "%s::putSignal(uuid='%s',logRecord='%s', execMethod=%d, status=update)",
                plugin_list->plugin[plugin_id].format, client_block->DOI, logRecord, execMethod);
    }

    // Activate the plugin

    UDA_LOG(UDA_LOG_DEBUG, "Provenance Plugin signal: %s\n", request_block.signal);

    makeServerRequestBlock(&request_block, *plugin_list);

    int err, rc, reset;
    DATA_BLOCK data_block;
    IDAM_PLUGIN_INTERFACE idam_plugin_interface;

    // Initialise the Data Block

    initDataBlock(&data_block);

    UDA_LOG(UDA_LOG_DEBUG, "Creating plugin interface\n");

    // Check the Interface Compliance

    if (plugin_list->plugin[plugin_id].interfaceVersion > 1) {
        err = 999;
        addIdamError(CODEERRORTYPE, "idamProvenancePlugin", err,
                     "The Provenance Plugin's Interface Version is not Implemented.");
        return err;
    }

    USERDEFINEDTYPELIST* userdefinedtypelist = NULL;
    copyUserDefinedTypeList(&userdefinedtypelist);                // Allocate and Copy the Master User Defined Type List

    LOGMALLOCLIST* logmalloclist = (LOGMALLOCLIST*)malloc(sizeof(LOGMALLOCLIST));
    initLogMallocList(logmalloclist);

    idam_plugin_interface.interfaceVersion = 1;
    idam_plugin_interface.pluginVersion = 0;
    idam_plugin_interface.sqlConnectionType = 0;
    idam_plugin_interface.data_block = &data_block;
    idam_plugin_interface.client_block = client_block;
    idam_plugin_interface.request_block = &request_block;
    idam_plugin_interface.data_source = data_source;
    idam_plugin_interface.signal_desc = signal_desc;
    idam_plugin_interface.environment = environment;
    idam_plugin_interface.sqlConnection = NULL;        // Private to the plugin
    idam_plugin_interface.housekeeping = 0;
    idam_plugin_interface.changePlugin = 0;
    idam_plugin_interface.pluginList = plugin_list;
    idam_plugin_interface.userdefinedtypelist = userdefinedtypelist;
    idam_plugin_interface.logmalloclist = logmalloclist;

    // Redirect Output to temporary file if no file handles passed

    reset = 0;
    if ((err = idamServerRedirectStdStreams(reset)) != 0) {
        addIdamError(CODEERRORTYPE, "idamProvenancePlugin", err,
                     "Error Redirecting Plugin Message Output");
        return err;
    }

    // Call the plugin

    UDA_LOG(UDA_LOG_DEBUG, "entering the provenance plugin\n");

    err = plugin_list->plugin[plugin_id].idamPlugin(&idam_plugin_interface);

    UDA_LOG(UDA_LOG_DEBUG, "returned from the provenance plugin\n");

    // No data are returned in this context so free everything

    UDA_LOG(UDA_LOG_DEBUG, "housekeeping\n");

    freeMallocLogList(logmalloclist);
    free((void*)logmalloclist);

    freeUserDefinedTypeList(userdefinedtypelist);
    free((void*)userdefinedtypelist);
    userdefinedtypelist = NULL;

    freeNameValueList(&request_block.nameValueList);

    UDA_LOG(UDA_LOG_DEBUG, "testing for bug!!!\n");
    if (data_block.opaque_type != UDA_OPAQUE_TYPE_UNKNOWN ||
        data_block.opaque_count != 0 ||
        data_block.opaque_block != NULL) {
        UDA_LOG(UDA_LOG_DEBUG, "bug detected: mitigation!!!\n");
        data_block.opaque_block = NULL;
    }

    freeDataBlock(&data_block);

    // Reset Redirected Output

    reset = 1;
    if ((rc = idamServerRedirectStdStreams(reset)) != 0 || err != 0) {
        if (rc != 0) {
            addIdamError(CODEERRORTYPE, "idamProvenancePlugin", rc,
                         "Error Resetting Redirected Plugin Message Output");
        }
        if (err != 0) return err;
        return rc;
    }

    gettimeofday(&tv_stop, NULL);
    int msecs = (int)(tv_stop.tv_sec - tv_start.tv_sec) * 1000 + (int)(tv_stop.tv_usec - tv_start.tv_usec) / 1000;

    UDA_LOG(UDA_LOG_DEBUG, "end of housekeeping\n");
    UDA_LOG(UDA_LOG_DEBUG, "Timing (ms) = %d\n", msecs);

    return 0;
}

//------------------------------------------------------------------------------------------------
// Identify the Plugin to use to resolve Generic Name mappings and return its ID 

int idamServerMetaDataPluginId(const PLUGINLIST* plugin_list)
{
    static unsigned short noPluginRegistered = 0;
    static int plugin_id = -1;

    UDA_LOG(UDA_LOG_DEBUG, "Entered: noPluginRegistered state = %d\n", noPluginRegistered);
    UDA_LOG(UDA_LOG_DEBUG, "Entered: plugin_id state = %d\n", plugin_id);

    if (plugin_id >= 0) return plugin_id;     // Plugin previously identified
    if (noPluginRegistered) return -1;        // No Plugin for the MetaData Catalog to resolve Generic Name mappings

    // Identify the MetaData Catalog plugin (must be a function library type plugin)

    char* env = NULL;
    if ((env = getenv("UDA_METADATA_PLUGIN")) != NULL) {        // Must be set in the server startup script
        int id = findPluginIdByFormat(env, plugin_list);        // Must be defined in the server plugin configuration file
        if (id >= 0 &&
            plugin_list->plugin[id].plugin_class == PLUGINFUNCTION &&
            plugin_list->plugin[id].status == PLUGINOPERATIONAL &&
            plugin_list->plugin[id].pluginHandle != NULL &&
            plugin_list->plugin[id].idamPlugin != NULL) {
            plugin_id = (short)id;
        }

        if (id >= 0 && plugin_list->plugin[id].is_private == PLUGINPRIVATE &&
            getIdamServerEnvironment()->external_user) {
                plugin_id = -1;
        }        // Not available to external users


        UDA_LOG(UDA_LOG_DEBUG, "Generic Name Mapping Plugin Name: %s\n", env);
        UDA_LOG(UDA_LOG_DEBUG, "PLUGINFUNCTION?: %d\n", plugin_list->plugin[id].plugin_class == PLUGINFUNCTION);
        UDA_LOG(UDA_LOG_DEBUG, "PLUGINPRIVATE?: %d\n", plugin_list->plugin[id].is_private == PLUGINPRIVATE);
        UDA_LOG(UDA_LOG_DEBUG, "External User?: %d\n", getIdamServerEnvironment()->external_user);
        UDA_LOG(UDA_LOG_DEBUG, "Private?: %d\n",
                plugin_list->plugin[id].is_private == PLUGINPRIVATE && getIdamServerEnvironment()->external_user);
        UDA_LOG(UDA_LOG_DEBUG, "PLUGINOPERATIONAL?: %d\n", plugin_list->plugin[id].status == PLUGINOPERATIONAL);
        UDA_LOG(UDA_LOG_DEBUG, "Plugin OK?: %d\n",
                plugin_list->plugin[id].pluginHandle != NULL && plugin_list->plugin[id].idamPlugin != NULL);
        UDA_LOG(UDA_LOG_DEBUG, "id: %d\n", id);
        UDA_LOG(UDA_LOG_DEBUG, "id: %d\n", plugin_id);
    } else {
        UDA_LOG(UDA_LOG_DEBUG, "NO Generic Name Mapping Plugin identified\n");
    }

    if (plugin_id < 0) noPluginRegistered = 1;        // No Plugin found (registered)

    return plugin_id;
}

//------------------------------------------------------------------------------------------------
// Execute the Generic Name mapping Plugin

int idamServerMetaDataPlugin(const PLUGINLIST* plugin_list, int plugin_id, REQUEST_BLOCK* request_block,
                             SIGNAL_DESC* signal_desc, DATA_SOURCE* data_source, LOGMALLOCLIST* logmalloclist)
{
    int err, reset, rc;
    IDAM_PLUGIN_INTERFACE idam_plugin_interface;

    // Check the Interface Compliance

    if (plugin_list->plugin[plugin_id].interfaceVersion > 1) {
        err = 999;
        addIdamError(CODEERRORTYPE, "idamServerMetaDataPlugin", err,
                     "The Plugin's Interface Version is not Implemented.");
        return err;
    }

    USERDEFINEDTYPELIST* userdefinedtypelist = NULL;
    copyUserDefinedTypeList(&userdefinedtypelist);

    idam_plugin_interface.interfaceVersion = 1;
    idam_plugin_interface.pluginVersion = 0;
    idam_plugin_interface.sqlConnectionType = 0;
    idam_plugin_interface.data_block = NULL;
    idam_plugin_interface.client_block = NULL;
    idam_plugin_interface.request_block = request_block;
    idam_plugin_interface.data_source = data_source;
    idam_plugin_interface.signal_desc = signal_desc;
    idam_plugin_interface.environment = getIdamServerEnvironment();    // Legacy Global variable
    idam_plugin_interface.sqlConnection = NULL;        // Private to the plugin
    idam_plugin_interface.housekeeping = 0;
    idam_plugin_interface.changePlugin = 0;
    idam_plugin_interface.pluginList = plugin_list;
    idam_plugin_interface.userdefinedtypelist = userdefinedtypelist;
    idam_plugin_interface.logmalloclist = logmalloclist;

    // Redirect Output to temporary file if no file handles passed

    reset = 0;
    if ((err = idamServerRedirectStdStreams(reset)) != 0) {
        addIdamError(CODEERRORTYPE, "idamServerMetaDataPlugin", err,
                     "Error Redirecting Plugin Message Output");
        return err;
    }

// Call the plugin (Error handling is managed within)

    err = plugin_list->plugin[plugin_id].idamPlugin(&idam_plugin_interface);

// Reset Redirected Output 

    reset = 1;
    if ((rc = idamServerRedirectStdStreams(reset)) != 0 || err != 0) {
        if (rc != 0) {
            addIdamError(CODEERRORTYPE, "idamServerMetaDataPlugin", rc,
                         "Error Resetting Redirected Plugin Message Output");
        }
        if (err != 0) return err;
        return rc;
    }

    return err;
}

/**
 * Look for an argument with the given name in the provided NAMEVALUELIST and return it's associated value.
 *
 * If the argument is found the value associated with the argument is provided via the value parameter and the function
 * returns 1. Otherwise value is set to NULL and the function returns 0.
 * @param namevaluelist
 * @param value
 * @param name
 * @return
 */
bool findStringValue(const NAMEVALUELIST* namevaluelist, const char** value, const char* name)
{
    char** names = SplitString(name, "|");
    *value = NULL;

    bool found = 0;
    int i;
    for (i = 0; i < namevaluelist->pairCount; i++) {
        size_t n;
        for (n = 0; names[n] != NULL; ++n) {
            if (STR_IEQUALS(namevaluelist->nameValue[i].name, names[n])) {
                *value = namevaluelist->nameValue[i].value;
                found = 1;
                break;
            }
        }
    }

    FreeSplitStringTokens(&names);
    return found;
}

/**
 * Look for an argument with the given name in the provided NAMEVALUELIST and return it's associate value as an integer.
 *
 * If the argument is found the value associated with the argument is provided via the value parameter and the function
 * returns 1. Otherwise value is not set and the function returns 0.
 * @param namevaluelist
 * @param value
 * @param name
 * @return
 */
bool findIntValue(const NAMEVALUELIST* namevaluelist, int* value, const char* name)
{
    const char* str;
    bool found = findStringValue(namevaluelist, &str, name);
    if (found) {
        *value = atoi(str);
    }
    return found;
}

/**
 * Look for an argument with the given name in the provided NAMEVALUELIST and return it's associate value as a short.
 *
 * If the argument is found the value associated with the argument is provided via the value parameter and the function
 * returns 1. Otherwise value is not set and the function returns 0.
 * @param namevaluelist
 * @param value
 * @param name
 * @return
 */
bool findShortValue(const NAMEVALUELIST* namevaluelist, short* value, const char* name)
{
    const char* str;
    bool found = findStringValue(namevaluelist, &str, name);
    if (found) {
        *value = (short)atoi(str);
    }
    return found;
}

/**
 * Look for an argument with the given name in the provided NAMEVALUELIST and return it's associate value as a short.
 *
 * If the argument is found the value associated with the argument is provided via the value parameter and the function
 * returns 1. Otherwise value is not set and the function returns 0.
 * @param namevaluelist
 * @param value
 * @param name
 * @return
 */
bool findCharValue(const NAMEVALUELIST* namevaluelist, char* value, const char* name)
{
    const char* str;
    bool found = findStringValue(namevaluelist, &str, name);
    if (found) {
        *value = (char)atoi(str);
    }
    return found;
}

/**
 * Look for an argument with the given name in the provided NAMEVALUELIST and return it's associate value as a float.
 *
 * If the argument is found the value associated with the argument is provided via the value parameter and the function
 * returns 1. Otherwise value is not set and the function returns 0.
 * @param namevaluelist
 * @param value
 * @param name
 * @return
 */
bool findFloatValue(const NAMEVALUELIST* namevaluelist, float* value, const char* name)
{
    const char* str;
    bool found = findStringValue(namevaluelist, &str, name);
    if (found) {
        *value = strtof(str, NULL);
    }
    return found;
}

bool findIntArray(const NAMEVALUELIST* namevaluelist, int** values, size_t* nvalues, const char* name)
{
    const char* str;
    bool found = findStringValue(namevaluelist, &str, name);
    if (found) {
        char** tokens = SplitString(str, ";");
        size_t n;
        size_t num_tokens = 0;
        for (n = 0; tokens[n] != NULL; ++n) {
            ++num_tokens;
        }
        *values = calloc(num_tokens, sizeof(int));
        for (n = 0; tokens[n] != NULL; ++n) {
            (*values)[n] = (int)strtol(tokens[n], NULL, 10);
        }
        FreeSplitStringTokens(&tokens);
        *nvalues = num_tokens;
    }
    return found;
}

bool findFloatArray(const NAMEVALUELIST* namevaluelist, float** values, size_t* nvalues, const char* name)
{
    const char* str;
    bool found = findStringValue(namevaluelist, &str, name);
    if (found) {
        char** tokens = SplitString(str, ";");
        size_t n;
        size_t num_tokens = 0;
        for (n = 0; tokens[n] != NULL; ++n) {
            ++num_tokens;
        }
        *values = calloc(num_tokens, sizeof(float));
        for (n = 0; tokens[n] != NULL; ++n) {
            (*values)[n] = strtof(tokens[n], NULL);
        }
        FreeSplitStringTokens(&tokens);
        *nvalues = num_tokens;
    }
    return found;
}

bool findDoubleArray(const NAMEVALUELIST* namevaluelist, double** values, size_t* nvalues, const char* name)
{
    const char* str;
    bool found = findStringValue(namevaluelist, &str, name);
    if (found) {
        char** tokens = SplitString(str, ";");
        size_t n;
        size_t num_tokens = 0;
        for (n = 0; tokens[n] != NULL; ++n) {
            ++num_tokens;
        }
        *values = calloc(num_tokens, sizeof(double));
        for (n = 0; tokens[n] != NULL; ++n) {
            (*values)[n] = strtod(tokens[n], NULL);
        }
        FreeSplitStringTokens(&tokens);
        *nvalues = num_tokens;
    }
    return found;
}

bool findValue(const NAMEVALUELIST* namevaluelist, const char* name)
{
    char** names = SplitString(name, "|");

    bool found = false;
    int i;
    for (i = 0; i < namevaluelist->pairCount; i++) {
        size_t n = 0;
        while (names[n] != NULL) {
            if (STR_IEQUALS(namevaluelist->nameValue[i].name, names[n])) {
                found = 1;
                break;
            }
            ++n;
        }
    }

    FreeSplitStringTokens(&names);
    return found;
}

int callPlugin(const PLUGINLIST* pluginlist, const char* request, const IDAM_PLUGIN_INTERFACE* old_plugin_interface)
{
    IDAM_PLUGIN_INTERFACE idam_plugin_interface = *old_plugin_interface;
    REQUEST_BLOCK request_block = *old_plugin_interface->request_block;
    idam_plugin_interface.request_block = &request_block;

    request_block.source[0] = '\0';
    strcpy(request_block.signal, request);
    makeServerRequestBlock(&request_block, *pluginlist);

    request_block.request = findPluginRequestByFormat(request_block.format, pluginlist);

    if (request_block.request < 0) {
        RAISE_PLUGIN_ERROR("Plugin not found!");
    }

    int err = 0;
    int id = findPluginIdByRequest(request_block.request, pluginlist);
    PLUGIN_DATA* plugin = &(pluginlist->plugin[id]);
    if (id >= 0 && plugin->idamPlugin != NULL) {
        err = plugin->idamPlugin(&idam_plugin_interface);    // Call the data reader
    } else {
        RAISE_PLUGIN_ERROR("Data Access is not available for this data request!");
    }

    return err;
}
