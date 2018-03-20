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
#include "west_tunnel.h"

#include <stdlib.h>
#include <strings.h>

#include <clientserver/stringUtils.h>
#include <clientserver/initStructs.h>
#include <clientserver/udaTypes.h>
#include <plugins/udaPlugin.h>
#include <client/udaGetAPI.h>

#include "west_tunnel_ssh.h"
#include "west_tunnel_ssh_server.h"

static int do_help(IDAM_PLUGIN_INTERFACE* idam_plugin_interface);

static int do_version(IDAM_PLUGIN_INTERFACE* idam_plugin_interface);

static int do_builddate(IDAM_PLUGIN_INTERFACE* idam_plugin_interface);

static int do_defaultmethod(IDAM_PLUGIN_INTERFACE* idam_plugin_interface);

static int do_maxinterfaceversion(IDAM_PLUGIN_INTERFACE* idam_plugin_interface);

static int do_read(IDAM_PLUGIN_INTERFACE* idam_plugin_interface);

int west_tunnel(IDAM_PLUGIN_INTERFACE* idam_plugin_interface)
{
    static int init = 0;

    //----------------------------------------------------------------------------------------
    // Standard v1 Plugin Interface

    if (idam_plugin_interface->interfaceVersion > THISPLUGIN_MAX_INTERFACE_VERSION) {
        RAISE_PLUGIN_ERROR("Plugin Interface Version Unknown to this plugin: Unable to execute the request!");
    }

    idam_plugin_interface->pluginVersion = THISPLUGIN_VERSION;

    REQUEST_BLOCK* request_block = idam_plugin_interface->request_block;

    if (idam_plugin_interface->housekeeping || STR_IEQUALS(request_block->function, "reset")) {
        if (!init) return 0; // Not previously initialised: Nothing to do!
        // Free Heap & reset counters
        init = 0;
        return 0;
    }

    //----------------------------------------------------------------------------------------
    // Initialise

    if (!init) {
        g_server_port = 0;
        g_initialised = false;

        pthread_cond_init(&g_initialised_cond, NULL);
        pthread_mutex_init(&g_initialised_mutex, NULL);

        pthread_t server_thread;
        SERVER_THREAD_DATA thread_data = {};
        thread_data.experiment = "WEST";
    	thread_data.ssh_host = "andromede1.partenaires.cea.fr";
    	thread_data.uda_host = "andromede1.partenaires.cea.fr";

        pthread_create(&server_thread, NULL, server_task, &thread_data);

        pthread_mutex_lock(&g_initialised_mutex);
        while (!g_initialised) {
            pthread_cond_wait(&g_initialised_cond, &g_initialised_mutex);
        }
        pthread_mutex_unlock(&g_initialised_mutex);

        pthread_mutex_destroy(&g_initialised_mutex);
        pthread_cond_destroy(&g_initialised_cond);

        struct timespec sleep_for;
        sleep_for.tv_sec = 0;
        sleep_for.tv_nsec = 100000000;
        nanosleep(&sleep_for, NULL);

        init = 1;
    }

    if (STR_IEQUALS(request_block->function, "help")) {
        return do_help(idam_plugin_interface);
    } else if (STR_IEQUALS(request_block->function, "version")) {
        return do_version(idam_plugin_interface);
    } else if (STR_IEQUALS(request_block->function, "builddate")) {
        return do_builddate(idam_plugin_interface);
    } else if (STR_IEQUALS(request_block->function, "defaultmethod")) {
        return do_defaultmethod(idam_plugin_interface);
    } else if (STR_IEQUALS(request_block->function, "maxinterfaceversion")) {
        return do_maxinterfaceversion(idam_plugin_interface);
    } else if (STR_IEQUALS(request_block->function, "read")) {
        return do_read(idam_plugin_interface, host);
    } else {
        RAISE_PLUGIN_ERROR("Unknown function requested!");
    }
}

/**
 * Help: A Description of library functionality
 * @param idam_plugin_interface
 * @return
 */
int do_help(IDAM_PLUGIN_INTERFACE* idam_plugin_interface)
{
    const char* help = "\ntemplatePlugin: Add Functions Names, Syntax, and Descriptions\n\n";
    const char* desc = "templatePlugin: help = description of this plugin";

    return setReturnDataString(idam_plugin_interface->data_block, help, desc);
}

/**
 * Plugin version
 * @param idam_plugin_interface
 * @return
 */
int do_version(IDAM_PLUGIN_INTERFACE* idam_plugin_interface)
{
    return setReturnDataIntScalar(idam_plugin_interface->data_block, THISPLUGIN_VERSION, "Plugin version number");
}

/**
 * Plugin Build Date
 * @param idam_plugin_interface
 * @return
 */
int do_builddate(IDAM_PLUGIN_INTERFACE* idam_plugin_interface)
{
    return setReturnDataString(idam_plugin_interface->data_block, __DATE__, "Plugin build date");
}

/**
 * Plugin Default Method
 * @param idam_plugin_interface
 * @return
 */
int do_defaultmethod(IDAM_PLUGIN_INTERFACE* idam_plugin_interface)
{
    return setReturnDataString(idam_plugin_interface->data_block, THISPLUGIN_DEFAULT_METHOD, "Plugin default method");
}

/**
 * Plugin Maximum Interface Version
 * @param idam_plugin_interface
 * @return
 */
int do_maxinterfaceversion(IDAM_PLUGIN_INTERFACE* idam_plugin_interface)
{
    return setReturnDataIntScalar(idam_plugin_interface->data_block, THISPLUGIN_MAX_INTERFACE_VERSION, "Maximum Interface Version");
}

typedef struct ServerThreadData {
    const char* experiment;
    const char* ssh_host;
    const char* uda_host;
} SERVER_THREAD_DATA;

static void* server_task(void* ptr)
{
    SERVER_THREAD_DATA* data = (SERVER_THREAD_DATA*)ptr;
    ssh_run_server(data->experiment, data->ssh_host, data->uda_host);
    return NULL;
}

//----------------------------------------------------------------------------------------
// Add functionality here ....
int do_read(IDAM_PLUGIN_INTERFACE* idam_plugin_interface)
{
    DATA_BLOCK* data_block = idam_plugin_interface->data_block;

    setenv("UDA_HOST", "localhost", 1);

    char port[100];
    sprintf(port, "%d", g_server_port);
    setenv("UDA_PORT", port, 1);

    REQUEST_BLOCK request_block = idam_plugin_interface->request_block;

    const char* group;
    FIND_REQUIRED_STRING_VALUE(request_block->nameValueList, group);

    const char* expName;
    FIND_REQUIRED_STRING_VALUE(request_block->nameValueList, expName);

    const char* type;
    FIND_REQUIRED_STRING_VALUE(request_block->nameValueList, type);

    const char* variable;
    FIND_REQUIRED_STRING_VALUE(request_block->nameValueList, variable);

    int shot = 0;
    FIND_REQUIRED_INT_VALUE(request_block->nameValueList, shot);

    int rank = 0;
    FIND_REQUIRED_INT_VALUE(request_block->nameValueList, rank);

    char request[1024];
    //sprintf(request, "imas::get(idx=0, group='%s', variable='%s', expName='%s', type=%s, rank=%d, shot=%d", ...);
    sprintf(request, "imas::get(idx=0, group='%s', variable='%s', expName='%s', type=%s, rank=%d, shot=%d", group, expName, type, rank, shot);
    idamGetAPI("IMAS::read(...)", "");

    return 0;
}