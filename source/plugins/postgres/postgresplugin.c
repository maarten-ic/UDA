/*---------------------------------------------------------------
* v1 UDA Plugin: Query the UDA POSTGRES Metadata Catalog
*                Designed for use case where each signal class is recorded but not each signal instance
*                Only the first record identified that satisfies the selection criteria is returned - multiple records raises an error
*                Records within the database must be unique 
*
* Input Arguments:  IDAM_PLUGIN_INTERFACE *idam_plugin_interface
*
* Returns:      0 if the plugin functionality was successful
*           otherwise a Error Code is returned 
*
* Public Functions:
*
*       query   return the database metadata record for a data object
*
* Private Functions:
*
*       get return the name mapping between an alias or generic name for a data object and its true name or
*               method of data access.
*      
*       connection return the private database connection object for other plugins to reuse 
* 
* Standard functionality:
*
*   help    a description of what this plugin does together with a list of functions available
*
*   reset   frees all previously allocated heap, closes file handles and resets all static parameters.
*       This has the same functionality as setting the housekeeping directive in the plugin interface
*       data structure to TRUE (1)
*
*   init    Initialise the plugin: read all required data and process. Retain staticly for
*       future reference.   
*
* Change History
*
* 26May2017 D.G.Muir    Original Version
*---------------------------------------------------------------------------------------------------------------*/

#include "postgresplugin.h"

#include <clientserver/stringUtils.h>
#include <structures/struct.h>
#include <structures/accessors.h>
#include <clientserver/initStructs.h>

static int preventSQLInjection(PGconn* DBConnect, char** from)
{

// Replace the passed string with an Escaped String
// Free the Original string from Heap

    int err = 0;
    size_t fromCount = strlen(*from);
    char* to = (char*)malloc((2 * fromCount + 1) * sizeof(char));
    PQescapeStringConn(DBConnect, to, *from, fromCount, &err);
    if (err != 0) {
        if (to != NULL) free((void*)to);
        return 1;
    }
    free((void*)*from);
    *from = to;
    return 0;
}

static int do_help(IDAM_PLUGIN_INTERFACE* idam_plugin_interface);

static int do_version(IDAM_PLUGIN_INTERFACE* idam_plugin_interface);

static int do_builddate(IDAM_PLUGIN_INTERFACE* idam_plugin_interface);

static int do_defaultmethod(IDAM_PLUGIN_INTERFACE* idam_plugin_interface);

static int do_maxinterfaceversion(IDAM_PLUGIN_INTERFACE* idam_plugin_interface);

static int do_query(IDAM_PLUGIN_INTERFACE* idam_plugin_interface, PGconn* conn);

// Open the Connection with the PostgreSQL IDAM Database

static PGconn* postgresConnect()
{

    char* pghost = getenv("UDA_SQLHOST");
    char* pgport = getenv("UDA_SQLPORT");
    char* dbname = getenv("UDA_SQLDBNAME");
    char* user = getenv("UDA_SQLUSER");

    char pswrd[9] = "readonly";    // Default for user 'readonly'

    char* pgoptions = NULL;    //"connect_timeout=5";
    char* pgtty = NULL;

    PGconn* DBConnect = NULL;

//------------------------------------------------------------- 
// Debug Trace Queries

    UDA_LOG(UDA_LOG_DEBUG, "SQL Connection: host %s\n", pghost);
    UDA_LOG(UDA_LOG_DEBUG, "                port %s\n", pgport);
    UDA_LOG(UDA_LOG_DEBUG, "                db   %s\n", dbname);
    UDA_LOG(UDA_LOG_DEBUG, "                user %s\n", user);

//-------------------------------------------------------------
// Connect to the Database Server

    if (strcmp(user, "readonly") != 0) pswrd[0] = '\0';    // No password - set in the server's .pgpass file

    if ((DBConnect = PQsetdbLogin(pghost, pgport, pgoptions, pgtty, dbname, user, pswrd)) == NULL) {
        addIdamError(CODEERRORTYPE, "startSQL", 1, "SQL Server Connect Error");
        PQfinish(DBConnect);
        return NULL;
    }

    if (PQstatus(DBConnect) == CONNECTION_BAD) {
        addIdamError(CODEERRORTYPE, "startSQL", 1, "Bad SQL Server Connect Status");
        PQfinish(DBConnect);
        return NULL;
    }

    return DBConnect;
}

extern int postgres_query(IDAM_PLUGIN_INTERFACE* idam_plugin_interface)
{
    static short init = 0;

    //----------------------------------------------------------------------------------------
    // Database Objects

    static PGconn* conn = NULL;
    static short sqlPrivate = 0;                // If the Database connection was opened here, it remains local and open but is not passed back.
    static unsigned short DBType = PLUGINSQLNOTKNOWN;    // The database type

    unsigned short housekeeping;

    if (idam_plugin_interface->interfaceVersion > THISPLUGIN_MAX_INTERFACE_VERSION) {
        RAISE_PLUGIN_ERROR("Plugin Interface Version Unknown to this plugin: Unable to execute the request!");
    }

    idam_plugin_interface->pluginVersion = THISPLUGIN_VERSION;

    housekeeping = idam_plugin_interface->housekeeping;

    // Database connection passed in from the server (external)

    if (!sqlPrivate) {
        // Use External database connection
        DBType = idam_plugin_interface->sqlConnectionType;
        if (DBType == PLUGINSQLPOSTGRES) {
            conn = idam_plugin_interface->sqlConnection;
        }
    }

    REQUEST_BLOCK* request_block = idam_plugin_interface->request_block;

    if (housekeeping || !strcasecmp(request_block->function, "reset")) {

        if (!init) return 0;        // Not previously initialised: Nothing to do!

        // Free Heap & reset counters

        if (sqlPrivate && conn != NULL && DBType == PLUGINSQLPOSTGRES) {
            PQfinish(conn);
            sqlPrivate = 0;
            conn = NULL;
            DBType = PLUGINSQLNOTKNOWN;
        }

        init = 0;

        return 0;
    }

    //----------------------------------------------------------------------------------------
    // Initialise

    if (!init || !strcasecmp(request_block->function, "init") || !strcasecmp(request_block->function, "initialise")) {

        // Is there an Open Database Connection? If not then open a private (within scope of this plugin only) connection

        if (conn == NULL && (DBType == PLUGINSQLPOSTGRES || DBType == PLUGINSQLNOTKNOWN)) {

            if ((conn = postgresConnect()) == NULL) {
                return 999;
            }

            DBType = PLUGINSQLPOSTGRES;
            sqlPrivate = 1;            // the connection belongs to this plugin
            UDA_LOG(UDA_LOG_DEBUG, "postgresplugin: Private regular database connection made.\n");
        }

        init = 1;
    }

    //----------------------------------------------------------------------------------------
    // Return the private (local) connection Object
    // Not intended for client use ...

    if (!strcasecmp(request_block->function, "connection")) {

        if (sqlPrivate && conn != NULL) {
            idam_plugin_interface->sqlConnectionType = PLUGINSQLPOSTGRES;
            idam_plugin_interface->sqlConnection = (void*)conn;
        } else {
            idam_plugin_interface->sqlConnectionType = PLUGINSQLNOTKNOWN;
            idam_plugin_interface->sqlConnection = NULL;
        }

        return 0;
    }

    //----------------------------------------------------------------------------------------
    // Plugin Functions
    //----------------------------------------------------------------------------------------
    /*
     The query method is controlled by passed name value pairs. This method and plugin is called explicitly for use cases where this is just another regular plugin resource.

     The default method is called when the plugin has the 'special' role of resolving generic signal names using the IDAM database containing name mappings etc.
     Neither plugin name nor method name is included in the user request for this specific use case. Therefore, the default method must be identified via the plugin registration
     process and added to the request.

     query( [signal|objectName]=objectName, [shot|exp_number|pulse|pulno]=exp_number [,source|objectSource]=objectSource [,type=type] [,sourceClass=sourceClass] [,objectClass=objectClass])

     get()  requires the shot number to be passed as the second argument

    */

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
    } else if (request_block->function[0] == '\0' || STR_IEQUALS(request_block->function, "query")
               || STR_IEQUALS(request_block->function, THISPLUGIN_DEFAULT_METHOD)) {
        return do_query(idam_plugin_interface, conn);
    } else {
        RAISE_PLUGIN_ERROR("Unknown function requested!");
    }

    UDA_LOG(UDA_LOG_DEBUG, "Exit\n");

    return 0;
}

/**
 * Help: A Description of library functionality
 * @param idam_plugin_interface
 * @return
 */
int do_help(IDAM_PLUGIN_INTERFACE* idam_plugin_interface)
{
    const char* help = "\npostgresplugin: Function Names, Syntax, and Descriptions\n\n"
            "Query the POSTGRES IDAM database for specific instances of a data object by alias or generic name and shot number\n\n"
            "\tquery( [signal|objectName]=objectName, [shot|exp_number|pulse|pulno]=exp_number [,source|objectSource=objectSource])\n"
            "\t       [objectClass=objectClass] [,sourceClass=sourceClass] [,type=type])\n\n"
            "\tobjectName: The alias or generic name of the data object.\n"
            "\texp_number: The experiment shot number. This may be passed via the UDA client API's second argument.\n"
            "\tobjectSource: the abstract name of the source. This may be passed via the client API's second argument, either alone or with exp_number [exp_number/objectSource]\n"
            "\tobjectClass: the name of the data's measurement class, e.g. magnetics\n"
            "\tsourceClass: the name of the data's source class, e.g. imas\n"
            "\ttype: the data type classsification, e.g. P for Plugin\n\n"
            "\tobjectName is a mandatory argument. One or both of exp_number and ObjectSource is also mandatory unless passed via the client API's second argument.\n\n\n";
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
    return setReturnDataIntScalar(idam_plugin_interface->data_block, THISPLUGIN_MAX_INTERFACE_VERSION,
                                  "Maximum Interface Version");
}

//----------------------------------------------------------------------------------------
// Add functionality here ....
int do_query(IDAM_PLUGIN_INTERFACE* idam_plugin_interface, PGconn* conn)
{
    REQUEST_BLOCK* request_block = idam_plugin_interface->request_block;

    bool isObjectName;
    const char* objectName = NULL;

    bool isObjectSource = false;
    const char* objectSource = NULL;

    bool isExpNumber = false;
    int expNumber = 0;

    bool isType = false;
    const char* type = NULL;

    bool isSourceClass = false;
    const char* sourceClass = NULL;

    bool isObjectClass = false;
    const char* objectClass = NULL;

    if (!strcasecmp(request_block->function, "query")) {
        // Name Value pairs => a regular returned DATA_BLOCK

        isObjectName = findStringValue(&request_block->nameValueList, &objectName, "signal|objectName");
        isObjectSource = findStringValue(&request_block->nameValueList, &objectSource, "source|objectSource");
        isExpNumber = findIntValue(&request_block->nameValueList, &expNumber, "shot|exp_number|pulse|pulno");
        isType = findStringValue(&request_block->nameValueList, &type, "type");
        isSourceClass = findStringValue(&request_block->nameValueList, &sourceClass, "sourceClass");
        isObjectClass = findStringValue(&request_block->nameValueList, &objectClass, "objectClass");

    } else {
        // Default Method: Names and shot or source passed via the standard legacy API arguments => returned SIGNAL_DESC and DATA_SOURCE

        isObjectName = 1;
        objectName = request_block->signal;

        if (request_block->exp_number > 0) {
            isExpNumber = 1;
            expNumber = request_block->exp_number;
        }

        if (request_block->tpass[0] != '\0') {
            isObjectSource = 1;
            objectSource = request_block->tpass;
        }
    }

    // Mandatory arguments

    if (!isObjectName) {
        RAISE_PLUGIN_ERROR("No Data Object Name specified");
    }

    if (!isExpNumber && !isObjectSource) {

        if (request_block->exp_number > 0) {
            // The expNumber has been specified via the client API's second argument
            isExpNumber = 1;
            expNumber = request_block->exp_number;
        }

        if (request_block->tpass[0] != '\0') {
            isObjectSource = 1;
            objectSource = request_block->tpass;    // The object's source has been specified via the client API's second argument
        }

        if (!isExpNumber && !isObjectSource) {
            RAISE_PLUGIN_ERROR("No Experiment Number or data source specified");
        }
    }

    // Query for a specific named data object valid for the shot number range

    // All classification and abstraction data should be recorded in single case
    // Upper case is chosen as the convention. This is mandatory for the object name
    // Lower case matches are also made so do not use Mixed Case for data in the database!

    //-------------------------------------------------------------
    // Escape SIGNAL and TPASS to protect against SQL Injection

    char* signal = strdup(objectName);
    strupr(signal);
    if (preventSQLInjection(conn, &signal)) {
        free((void*)signal);
        RAISE_PLUGIN_ERROR("Unable to Escape the signal name");
    }

    char* tpass = NULL;
    if(isObjectSource && objectSource != NULL){
       tpass = strdup(objectSource);
       strupr(tpass);
       if (preventSQLInjection(conn, &tpass)) {
           free((void*)signal);
           free((void*)tpass);
           RAISE_PLUGIN_ERROR("Unable to Escape the tpass string");
       }
    }

    //-------------------------------------------------------------
    // Initialise Returned Structures

    SIGNAL_DESC* signal_desc = idam_plugin_interface->signal_desc;
    DATA_SOURCE* data_source = idam_plugin_interface->data_source;

    initSignalDesc(signal_desc);
    initDataSource(data_source);

    //-------------------------------------------------------------
    // Build SQL

    // Signal_Desc records have a uniqueness constraint: signal alias + generic name + name mapping shotrange + source alias name
    // Signal_alias or generic names are used by users to identify the correct signal for the given shot number
    // Generic names can be shared with multiple signal_name records, but only with non overlapping shot ranges.
    //

    /*

    // Add shot range to table: mapping_shot_range of type int4range
    // Add default inclusive range values '[0, 10000000]'
    // Add time-stamp range to table: mapping_time_range of type tstzrange (specific time-zone with millisecond time resolution)

    // Create ranges with inclusive end values [] rather than exclusive end values (). [ and ) can be mixed in defining the range.
    // Change uniqueness constraint to UNIQUE(signal_alias, generic_name, mapping_shot_range, source_alias);
    // Change index for range: CREATE INDEX mapping_shot_range_idx ON signal_desc USING gist (mapping_shot_range);

    ToDo:

    The 'tpass' string normally contain the data's sequence number but can pass any set of name-value pairs or other types of directives. These are passed into this plugin via the
    GET API's second argument - the data source argument. This is currently unused in the query.

    Parameters passed to the plugin as name-value pairs (type, source_alias or sourceClass, signal_class or objectClass) are also not used.
    */

    char sql[2048];

    if (isExpNumber) {
        sprintf(sql,
                "SELECT type, source_alias, signal_alias, generic_name, signal_name, signal_class FROM signal_desc WHERE "
                        "signal_alias = '%s' OR generic_name = '%s' AND mapping_shot_range @> %d ",
                signal, signal, expNumber);
    } else {
        sprintf(sql,
                "SELECT type, source_alias, signal_alias, generic_name, signal_name, signal_class FROM signal_desc WHERE "
                        "signal_alias = '%s' OR generic_name = '%s' ",
                signal, signal);
    }

    if (isType) {
        char* work = (char*)malloc((13 + strlen(type)) * sizeof(char));
        char* upper = strupr(strdup(type));
        sprintf(work, "AND type='%s' ", upper);
        strcat(sql, work);
        free(work);
        free(upper);
    }

    if (isSourceClass) {
        char* work = (char*)malloc((21 + strlen(sourceClass)) * sizeof(char));
        char* upper = strupr(strdup(sourceClass));
        sprintf(work, "AND source_alias='%s' ", upper);
        strcat(sql, work);
        free(work);
        free(upper);
    }

    if (isObjectClass) {
        char* work = (char*)malloc((21 + strlen(objectClass)) * sizeof(char));
        char* upper = strupr(strdup(objectClass));
        sprintf(work, "AND signal_class='%s' ", upper);
        strcat(sql, work);
        free(work);
        free(upper);
    }

    if(signal != NULL) free((void*)signal);
    if(tpass != NULL) free((void*)tpass);

    //-------------------------------------------------------------
    // Test Performance

    UDA_LOG(UDA_LOG_DEBUG, "%s\n", sql);

    //-------------------------------------------------------------
    // Execute SQL

    PGresult* query;

    if ((query = PQexec(conn, sql)) == NULL) {
        addIdamError(CODEERRORTYPE, "sqlSignalDescMap", 1, PQresultErrorMessage(query));
        PQclear(query);
        return PQresultStatus(query);
    }

    int nrows = PQntuples(query);        // Number of Rows
    int ncols = PQnfields(query);        // Number of Columns

    UDA_LOG(UDA_LOG_DEBUG, "No. Rows: %d\n", nrows);
    UDA_LOG(UDA_LOG_DEBUG, "No. Cols: %d\n", ncols);
    UDA_LOG(UDA_LOG_DEBUG, "SQL Msg : %s\n", PQresultErrorMessage(query));

    if (nrows == 0) {        // Nothing matched!
        PQclear(query);
        return 0;
    }

    //-------------------------------------------------------------
    // Multiple records: Process Exception and return error
    //
    // nrows == 0 => no match
    // nrows == 1 => unambiguous match to signal_alias or generic_name. This is the target record
    // nrows >= 2 => ambiguous match => Error.

    if (nrows > 1) {
        PQclear(query);
        RAISE_PLUGIN_ERROR("Ambiguous database entries found! Please advise the System Administrator.")
    }

    //-------------------------------------------------------------
    // Extract information

    int rowId = 0;

    // Signal_Desc fields

    int col = 0;

    signal_desc->type = PQgetvalue(query, rowId, col++)[0];
    strcpy(signal_desc->source_alias, PQgetvalue(query, rowId, col++));
    strcpy(signal_desc->signal_alias, PQgetvalue(query, rowId, col++));
    strcpy(signal_desc->generic_name, PQgetvalue(query, rowId, col++));
    strcpy(signal_desc->signal_name, PQgetvalue(query, rowId, col++));
    strcpy(signal_desc->signal_class, PQgetvalue(query, rowId, col++));

    PQclear(query);

    if (isExpNumber) {
        sprintf(data_source->path, "%d", expNumber);
    }

    data_source->type = signal_desc->type;
    strcpy(data_source->source_alias, signal_desc->source_alias);

    if (STR_IEQUALS(request_block->function, "query")) {

        // Create the Returned Structure Definition

        USERDEFINEDTYPE usertype;
        initUserDefinedType(&usertype);            // New structure definition

        strcpy(usertype.name, "POSTGRES_R");
        usertype.size = sizeof(POSTGRES_R);

        strcpy(usertype.source, "postgresplugin");
        usertype.ref_id = 0;
        usertype.imagecount = 0;                // No Structure Image data
        usertype.image = NULL;
        usertype.idamclass = UDA_TYPE_COMPOUND;

        int offset = 0;

        COMPOUNDFIELD field;

        defineField(&field, "objectName", "The target data object's (abstracted) (signal) name", &offset,
                    SCALARSTRING);
        addCompoundField(&usertype, field);
        defineField(&field, "expNumber", "The target experiment number", &offset, SCALARINT);
        addCompoundField(&usertype, field);
        defineField(&field, "signal_name", "the name of the target data object", &offset, SCALARSTRING);
        addCompoundField(&usertype, field);
        defineField(&field, "type", "the type of data object", &offset, SCALARSTRING);
        addCompoundField(&usertype, field);
        defineField(&field, "signal_alias", "the alias name of the target data object", &offset, SCALARSTRING);
        addCompoundField(&usertype, field);
        defineField(&field, "generic_name", "the generic name of the target data object", &offset,
                    SCALARSTRING);
        addCompoundField(&usertype, field);
        defineField(&field, "source_alias", "the source class", &offset, SCALARSTRING);
        addCompoundField(&usertype, field);
        defineField(&field, "signal_class", "the data class", &offset, SCALARSTRING);
        addCompoundField(&usertype, field);
        defineField(&field, "description", "a description of the data object", &offset, SCALARSTRING);
        addCompoundField(&usertype, field);
        defineField(&field, "range_start", "validity range starting value", &offset, SCALARINT);
        addCompoundField(&usertype, field);
        defineField(&field, "range_stop", "validity range ending value", &offset, SCALARINT);
        addCompoundField(&usertype, field);

        USERDEFINEDTYPELIST* userdefinedtypelist = idam_plugin_interface->userdefinedtypelist;
        LOGMALLOCLIST* logmalloclist = idam_plugin_interface->logmalloclist;

        addUserDefinedType(userdefinedtypelist, usertype);

        // Create the data to be returned

        POSTGRES_R* data = (POSTGRES_R*)malloc(sizeof(POSTGRES_R));
        addMalloc(logmalloclist, (void*)data, 1, sizeof(POSTGRES_R), "POSTGRES_R");

        size_t lstr = strlen(objectName) + 1;
        data->objectName = (char*)malloc(lstr * sizeof(char));
        addMalloc(logmalloclist, (void*)data->objectName, 1, lstr * sizeof(char), "char");
        strcpy(data->objectName, objectName);

        lstr = strlen(signal_desc->signal_name) + 1;
        data->signal_name = (char*)malloc(lstr * sizeof(char));
        addMalloc(logmalloclist, (void*)data->signal_name, 1, lstr * sizeof(char), "char");
        strcpy(data->signal_name, signal_desc->signal_name);

        lstr = 2;
        data->type = (char*)malloc(lstr * sizeof(char));
        addMalloc(logmalloclist, (void*)data->type, 1, lstr * sizeof(char), "char");
        data->type[0] = signal_desc->type;
        data->type[1] = '\0';

        lstr = strlen(signal_desc->signal_alias) + 1;
        data->signal_alias = (char*)malloc(lstr * sizeof(char));
        addMalloc(logmalloclist, (void*)data->signal_alias, 1, lstr * sizeof(char), "char");
        strcpy(data->signal_alias, signal_desc->signal_alias);

        lstr = strlen(signal_desc->generic_name) + 1;
        data->generic_name = (char*)malloc(lstr * sizeof(char));
        addMalloc(logmalloclist, (void*)data->generic_name, 1, lstr * sizeof(char), "char");
        strcpy(data->generic_name, signal_desc->generic_name);

        lstr = strlen(signal_desc->source_alias) + 1;
        data->source_alias = (char*)malloc(lstr * sizeof(char));
        addMalloc(logmalloclist, (void*)data->source_alias, 1, lstr * sizeof(char), "char");
        strcpy(data->source_alias, signal_desc->source_alias);

        lstr = strlen(signal_desc->signal_class) + 1;
        data->signal_class = (char*)malloc(lstr * sizeof(char));
        addMalloc(logmalloclist, (void*)data->signal_class, 1, lstr * sizeof(char), "char");
        strcpy(data->signal_class, signal_desc->signal_class);

        lstr = strlen(signal_desc->description) + 1;
        data->description = (char*)malloc(lstr * sizeof(char));
        addMalloc(logmalloclist, (void*)data->description, 1, lstr * sizeof(char), "char");
        strcpy(data->description, signal_desc->description);

        data->expNumber = expNumber;
        data->range_start = signal_desc->range_start;
        data->range_stop = signal_desc->range_stop;

        // Return the Data

        DATA_BLOCK* data_block = idam_plugin_interface->data_block;
        initDataBlock(data_block);

        data_block->data_type = UDA_TYPE_COMPOUND;
        data_block->rank = 0;
        data_block->data_n = 1;
        data_block->data = (char*)data;

        strcpy(data_block->data_desc, "postgresplugin query result");
        strcpy(data_block->data_label, "");
        strcpy(data_block->data_units, "");

        data_block->opaque_type = UDA_OPAQUE_TYPE_STRUCTURES;
        data_block->opaque_count = 1;
        data_block->opaque_block = (void*)findUserDefinedType(userdefinedtypelist, "POSTGRES_R", 0);
    }

    return 0;
}
