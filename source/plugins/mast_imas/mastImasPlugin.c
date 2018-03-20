/*---------------------------------------------------------------
* v1 IDAM Plugin Template: Standardised plugin design template, just add ... 
*
* Input Arguments:	IDAM_PLUGIN_INTERFACE *idam_plugin_interface
*
* Returns:		0 if the plugin functionality was successful
*			otherwise a Error Code is returned 
*
* Standard functionality:
*
*	help	a description of what this plugin does together with a list of functions available
*
*	reset	frees all previously allocated heap, closes file handles and resets all static parameters.
*		This has the same functionality as setting the housekeeping directive in the plugin interface
*		data structure to TRUE (1)
*
*	init	Initialise the plugin: read all required data and process. Retain staticly for
*		future reference.	
*
*---------------------------------------------------------------------------------------------------------------*/
#include "mastImasPlugin.h"

#include <libpq-fe.h>
#include <stdlib.h>
#include <strings.h>

#include <clientserver/initStructs.h>
#include <server/makeServerRequestBlock.h>
#include <client/accAPI.h>
#include <plugins/serverPlugin.h>
#include <server/sqllib.h>
#include <clientserver/stringUtils.h>
#include <clientserver/udaTypes.h>
#include <client/udaClient.h>

static int do_help(IDAM_PLUGIN_INTERFACE* idam_plugin_interface);

static int do_version(IDAM_PLUGIN_INTERFACE* idam_plugin_interface);

static int do_builddate(IDAM_PLUGIN_INTERFACE* idam_plugin_interface);

static int do_defaultmethod(IDAM_PLUGIN_INTERFACE* idam_plugin_interface);

static int do_maxinterfaceversion(IDAM_PLUGIN_INTERFACE* idam_plugin_interface);

static int do_read(IDAM_PLUGIN_INTERFACE* idam_plugin_interface);

int mastImasPlugin(IDAM_PLUGIN_INTERFACE* idam_plugin_interface)
{
    int err = 0;

    static short init = 0;

    //----------------------------------------------------------------------------------------
    // Standard v1 Plugin Interface

    unsigned short housekeeping;

    if (idam_plugin_interface->interfaceVersion > THISPLUGIN_MAX_INTERFACE_VERSION) {
        RAISE_PLUGIN_ERROR("Plugin Interface Version Unknown to this plugin: Unable to execute the request!");
    }

    idam_plugin_interface->pluginVersion = THISPLUGIN_VERSION;

    REQUEST_BLOCK* request_block = idam_plugin_interface->request_block;

    housekeeping = idam_plugin_interface->housekeeping;

    // Additional interface components (must be defined at the bottom of the standard data structure)
    // Versioning must be consistent with the macro THISPLUGIN_MAX_INTERFACE_VERSION and the plugin registration with the server

    //if(idam_plugin_interface->interfaceVersion >= 2){
    // NEW COMPONENTS
    //}

    //----------------------------------------------------------------------------------------
    // Heap Housekeeping

    // Plugin must maintain a list of open file handles and sockets: loop over and close all files and sockets
    // Plugin must maintain a list of plugin functions called: loop over and reset state and free heap.
    // Plugin must maintain a list of calls to other plugins: loop over and call each plugin with the housekeeping request
    // Plugin must destroy lists at end of housekeeping

    // A plugin only has a single instance on a server. For multiple instances, multiple servers are needed.
    // Plugins can maintain state so recursive calls (on the same server) must respect this.
    // If the housekeeping action is requested, this must be also applied to all plugins called.
    // A list must be maintained to register these plugin calls to manage housekeeping.
    // Calls to plugins must also respect access policy and user authentication policy

    if (housekeeping || STR_IEQUALS(request_block->function, "reset")) {

        if (!init) return 0;        // Not previously initialised: Nothing to do!

        // Free Heap & reset counters

        init = 0;

        return 0;
    }

    //----------------------------------------------------------------------------------------
    // Initialise

    if (!init || STR_IEQUALS(request_block->function, "init")
        || STR_IEQUALS(request_block->function, "initialise")) {

        init = 1;
        if (STR_IEQUALS(request_block->function, "init") || STR_IEQUALS(request_block->function, "initialise"))
            return 0;
    }

    //----------------------------------------------------------------------------------------
    // Plugin Functions
    //----------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------------
    // Standard methods: version, builddate, defaultmethod, maxinterfaceversion

    if (STR_IEQUALS(request_block->function, "help")) {
        err = do_help(idam_plugin_interface);
    } else if (STR_IEQUALS(request_block->function, "version")) {
        err = do_version(idam_plugin_interface);
    } else if (STR_IEQUALS(request_block->function, "builddate")) {
        err = do_builddate(idam_plugin_interface);
    } else if (STR_IEQUALS(request_block->function, "defaultmethod")) {
        err = do_defaultmethod(idam_plugin_interface);
    } else if (STR_IEQUALS(request_block->function, "maxinterfaceversion")) {
        err = do_maxinterfaceversion(idam_plugin_interface);
    } else if (STR_IEQUALS(request_block->function, "read")) {
        err = do_read(idam_plugin_interface);
    } else {
        RAISE_PLUGIN_ERROR("Unknown function requested!");
    }

    return err;
}

// Help: A Description of library functionality
int do_help(IDAM_PLUGIN_INTERFACE* idam_plugin_interface)
{
    DATA_BLOCK* data_block = idam_plugin_interface->data_block;

    char* p = (char*) malloc(sizeof(char) * 2 * 1024);

    strcpy(p, "\ntemplatePlugin: Add Functions Names, Syntax, and Descriptions\n\n");

    initDataBlock(data_block);

    data_block->rank = 1;
    data_block->dims = (DIMS*) malloc(data_block->rank * sizeof(DIMS));

    int i;
    for (i = 0; i < data_block->rank; i++) {
        initDimBlock(&data_block->dims[i]);
    }

    data_block->data_type = UDA_TYPE_STRING;
    strcpy(data_block->data_desc, "templatePlugin: help = description of this plugin");

    data_block->data = p;

    data_block->dims[0].data_type = UDA_TYPE_UNSIGNED_INT;
    data_block->dims[0].dim_n = strlen(p) + 1;
    data_block->dims[0].compressed = 1;
    data_block->dims[0].dim0 = 0.0;
    data_block->dims[0].diff = 1.0;
    data_block->dims[0].method = 0;

    data_block->data_n = data_block->dims[0].dim_n;

    strcpy(data_block->data_label, "");
    strcpy(data_block->data_units, "");

    return 0;
}

int do_version(IDAM_PLUGIN_INTERFACE* idam_plugin_interface)
{
    DATA_BLOCK* data_block = idam_plugin_interface->data_block;

    initDataBlock(data_block);
    data_block->data_type = UDA_TYPE_INT;
    data_block->rank = 0;
    data_block->data_n = 1;
    int* data = (int*) malloc(sizeof(int));
    data[0] = THISPLUGIN_VERSION;
    data_block->data = (char*) data;
    strcpy(data_block->data_desc, "Plugin version number");
    strcpy(data_block->data_label, "version");
    strcpy(data_block->data_units, "");

    return 0;
}

// Plugin Build Date
int do_builddate(IDAM_PLUGIN_INTERFACE* idam_plugin_interface)
{
    DATA_BLOCK* data_block = idam_plugin_interface->data_block;

    initDataBlock(data_block);
    data_block->data_type = UDA_TYPE_STRING;
    data_block->rank = 0;
    data_block->data_n = strlen(__DATE__) + 1;
    char* data = (char*) malloc(data_block->data_n * sizeof(char));
    strcpy(data, __DATE__);
    data_block->data = data;
    strcpy(data_block->data_desc, "Plugin build date");
    strcpy(data_block->data_label, "date");
    strcpy(data_block->data_units, "");

    return 0;
}

// Plugin Default Method
int do_defaultmethod(IDAM_PLUGIN_INTERFACE* idam_plugin_interface)
{
    DATA_BLOCK* data_block = idam_plugin_interface->data_block;

    initDataBlock(data_block);
    data_block->data_type = UDA_TYPE_STRING;
    data_block->rank = 0;
    data_block->data_n = strlen(THISPLUGIN_DEFAULT_METHOD) + 1;
    char* data = (char*) malloc(data_block->data_n * sizeof(char));
    strcpy(data, THISPLUGIN_DEFAULT_METHOD);
    data_block->data = data;
    strcpy(data_block->data_desc, "Plugin default method");
    strcpy(data_block->data_label, "method");
    strcpy(data_block->data_units, "");

    return 0;
}

// Plugin Maximum Interface Version
int do_maxinterfaceversion(IDAM_PLUGIN_INTERFACE* idam_plugin_interface)
{
    DATA_BLOCK* data_block = idam_plugin_interface->data_block;

    initDataBlock(data_block);
    data_block->data_type = UDA_TYPE_INT;
    data_block->rank = 0;
    data_block->data_n = 1;
    int* data = (int*) malloc(sizeof(int));
    data[0] = THISPLUGIN_MAX_INTERFACE_VERSION;
    data_block->data = (char*) data;
    strcpy(data_block->data_desc, "Maximum Interface Version");
    strcpy(data_block->data_label, "version");
    strcpy(data_block->data_units, "");

    return 0;
}

static char** get_names(PGconn* db, const char* name, int shot, int* size)
{
    const char* sql = "SELECT sd.signal_name FROM signal_desc sd, signal s, data_source ds\n" \
            "  WHERE sd.signal_desc_id = s.signal_desc_id\n" \
            "    AND ds.source_id = s.source_id\n" \
            "    AND ds.exp_number = %d\n" \
            "    AND ds.pass = 0\n" \
            "    AND sd.signal_alias ILIKE '%s'\n" \
            "    AND sd.signal_alias NOT ILIKE '%%status';";
    int n = snprintf(NULL, 0, sql, shot, name);
    char str[n];
    snprintf(str, n, sql, shot, name);

    PGresult* result;

    if ((result = PQexec(db, str)) == NULL) {
        UDA_LOG(UDA_LOG_ERROR, "Database Query Failed!\n");
        addIdamError(CODEERRORTYPE, "MAST IMAS plugin", 999, "Database Query Failed!");
        return NULL;
    }

    if (PQresultStatus(result) != PGRES_TUPLES_OK && PQresultStatus(result) != PGRES_COMMAND_OK) {
        UDA_LOG(UDA_LOG_ERROR, "%s\n", PQresultErrorMessage(result));
        addIdamError(CODEERRORTYPE, "MAST IMAS plugin", 999, PQresultErrorMessage(result));
        return NULL;
    }

    *size = PQntuples(result);

    if (*size == 0) {
        UDA_LOG(UDA_LOG_ERROR, "No data available!\n");
        addIdamError(CODEERRORTYPE, "MAST IMAS plugin", 999, "No Meta Data available!");
        return NULL;
    }

    char** names = (char**)malloc(*size * sizeof(char*));

    int i;
    for (i = 0; i < *size; i++) {
        size_t len = strlen(PQgetvalue(result, i, 0)) + 1;
        names[i] = (char*)malloc(len * sizeof(char));
        strcpy(names[i], PQgetvalue(result, i, 0));
    }

    PQclear(result);

    return names;
}

static int get_signal(IDAM_PLUGIN_INTERFACE* idam_plugin_interface, const char* signal, int shot_number)
{
    idam_plugin_interface->client_block->get_datadble = 1;
    idam_plugin_interface->client_block->get_dimdble = 1;

    char work[MAXMETA];

    IDAM_PLUGIN_INTERFACE next_plugin_interface;
    REQUEST_BLOCK* request_block = idam_plugin_interface->request_block;
    REQUEST_BLOCK next_request_block;

    const PLUGINLIST* plugin_list = idam_plugin_interface->pluginList; // List of all data reader plugins (internal and external shared libraries)

    if (plugin_list == NULL) {
        UDA_LOG(UDA_LOG_ERROR, "the specified format is not recognised!\n");
        addIdamError(CODEERRORTYPE,
                     "get_signal", 999, "imas source: No plugins are available for this data request");
        return 999;
    }

    next_plugin_interface = *idam_plugin_interface; // New plugin interface

    next_plugin_interface.request_block = &next_request_block;
    initServerRequestBlock(&next_request_block);
    strcpy(next_request_block.api_delim, request_block->api_delim);

    sprintf(next_request_block.source, "%d", shot_number); // Re-Use the original source argument

    // Create the Request data structure

    char* env = getenv("UDA_IDAM_PLUGIN");

    if (env != NULL) {
        sprintf(work, "%s::get(host=%s, port=%d, signal=\"%s\", source=\"%s\")", env, getIdamServerHost(),
                getIdamServerPort(), signal, next_request_block.source);
    } else {
        sprintf(work, "IDAM::get(host=%s, port=%d, signal=\"%s\", source=\"%s\")", getIdamServerHost(),
                getIdamServerPort(), signal, next_request_block.source);
    }

    next_request_block.source[0] = '\0';
    strcpy(next_request_block.signal, work);

    makeServerRequestBlock(&next_request_block, *plugin_list);

    // Call the IDAM client via the IDAM plugin (ignore the request identified)

    if (env != NULL) {
        next_request_block.request = findPluginRequestByFormat(env, plugin_list);
    } else {
        next_request_block.request = findPluginRequestByFormat("IDAM", plugin_list);
    }

    if (next_request_block.request < 0) {
        UDA_LOG(UDA_LOG_ERROR, "No IDAM server plugin found!\n");
        addIdamError(CODEERRORTYPE, "get_signal", 999, "No IDAM server plugin found!");
        return 999;
    }

// Locate and Execute the IDAM plugin

    int err = 0;
    int id = findPluginIdByRequest(next_request_block.request, plugin_list);
    PLUGIN_DATA* plugin = &plugin_list->plugin[id];
    if (id >= 0 && plugin->idamPlugin != NULL) {
        err = plugin->idamPlugin(&next_plugin_interface);        // Call the data reader
    } else {
        UDA_LOG(UDA_LOG_ERROR, "Data Access is not available for this data request!\n");
        addIdamError(CODEERRORTYPE, "get_signal", 999, "Data Access is not available for this data request!");
        return 999;
    }

    freeNameValueList(&next_request_block.nameValueList);

    // Return data is automatic since both next_request_block and request_block point to the same DATA_BLOCK etc.

    return err;
}

int do_read_magnetics(IDAM_PLUGIN_INTERFACE* idam_plugin_interface)
{
    const char* element;
    FIND_REQUIRED_STRING_VALUE(idam_plugin_interface->request_block->nameValueList, element);

    int shot = -1;
    FIND_REQUIRED_INT_VALUE(idam_plugin_interface->request_block->nameValueList, shot);

    int index = -1;
    findIntValue(&idam_plugin_interface->request_block->nameValueList, &index, "index");

    PGconn* db;
    if (idam_plugin_interface->sqlConnection != NULL && idam_plugin_interface->sqlConnectionType == PLUGINSQLPOSTGRES) {
        db = (PGconn*) idam_plugin_interface->sqlConnection;
    } else {
        db = startSQL();
    }

//    int get_shape = findValue(idam_plugin_interface->request_block->nameValueList, "get_shape");

    DATA_BLOCK* data_block = idam_plugin_interface->data_block;

    int num_flux_loops = -1;
    static char** flux_loops = NULL;

    int num_bpol_probes = -1;
    static char** bpol_probes = NULL;

    int err = 0;

    if (STR_EQUALS(element, "magnetics/method/Size_of")) {
        int* d = (int*)malloc(sizeof(int));
        d[0] = 1;
        data_block->data = (void*)d;
        data_block->data_n = 1;
        data_block->data_type = UDA_TYPE_INT;
    } else if (STR_EQUALS(element, "magnetics/flux_loop/Size_of")) {
        flux_loops = get_names(db, "amb_fl%", shot, &num_flux_loops);
        data_block->data = malloc(sizeof(int));
        *((int*)data_block->data) = num_flux_loops;
        data_block->data_n = 1;
        data_block->data_type = UDA_TYPE_INT;
    } else if (STR_EQUALS(element, "magnetics/bpol_probe/Size_of")) {
        bpol_probes = get_names(db, "amb_cc%", shot, &num_bpol_probes);
        data_block->data = malloc(sizeof(int));
        *((int*)data_block->data) = num_bpol_probes;
        data_block->data_n = 1;
        data_block->data_type = UDA_TYPE_INT;
    } else if (STR_EQUALS(element, "magnetics/flux_loop/#/name")) {
        data_block->data = flux_loops[index];
        data_block->data_n = 1;
        data_block->data_type = UDA_TYPE_STRING;
    } else if (STR_EQUALS(element, "magnetics/flux_loop/#/identifier")) {
    } else if (STR_EQUALS(element, "magnetics/flux_loop/#/position/r")) {
    } else if (STR_EQUALS(element, "magnetics/flux_loop/#/position/z")) {
    } else if (STR_EQUALS(element, "magnetics/flux_loop/#/position/phi")) {
    } else if (STR_EQUALS(element, "magnetics/bpol_probe/#/name")) {
        data_block->data = bpol_probes[index];
        data_block->data_n = 1;
        data_block->data_type = UDA_TYPE_STRING;
    } else if (STR_EQUALS(element, "magnetics/bpol_probe/#/identifier")) {
    } else if (STR_EQUALS(element, "magnetics/bpol_probe/#/position/r")) {
    } else if (STR_EQUALS(element, "magnetics/bpol_probe/#/position/z")) {
    } else if (STR_EQUALS(element, "magnetics/bpol_probe/#/position/phi")) {
    } else if (STR_EQUALS(element, "magnetics/bpol_probe/#/poloidal_angle")) {
    } else if (STR_EQUALS(element, "magnetics/bpol_probe/#/toroidal_angle")) {
    } else if (STR_EQUALS(element, "magnetics/bpol_probe/#/area")) {
    } else if (STR_EQUALS(element, "magnetics/bpol_probe/#/length")) {
    } else if (STR_EQUALS(element, "magnetics/bpol_probe/#/turns")) {
    } else if (STR_EQUALS(element, "magnetics/flux_loop/#/flux")) {
        err = get_signal(idam_plugin_interface, flux_loops[index], shot);
    } else if (STR_EQUALS(element, "magnetics/bpol_probe/#/field")) {
        err = get_signal(idam_plugin_interface, bpol_probes[index], shot);
    } else if (STR_EQUALS(element, "magnetics/method/#/ip")) {
        err = get_signal(idam_plugin_interface, "AMC_PLASMA CURRENT", shot);
    } else if (STR_EQUALS(element, "magnetics/method/#/diamagnetic_flux")) {
        err = get_signal(idam_plugin_interface, "AMD_DIA FLUX", shot);
    }

    return err;
}

//----------------------------------------------------------------------------------------
// Add functionality here ....
int do_read(IDAM_PLUGIN_INTERFACE* idam_plugin_interface)
{
    DATA_BLOCK* data_block = idam_plugin_interface->data_block;
    initDataBlock(data_block);

    const char* element;
    FIND_REQUIRED_STRING_VALUE(idam_plugin_interface->request_block->nameValueList, element);

    if (STR_STARTSWITH(element, "magnetics")) {
        int err = do_read_magnetics(idam_plugin_interface);
        if (err) return err;
    } else {
        addIdamError(CODEERRORTYPE, "do_read", 999, "element type not handled");
        return -1;
    }

    return 0;
}
