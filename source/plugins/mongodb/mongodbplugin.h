#ifndef UDA_PLUGINS_MONGODB_MONGODBPLUGIN_H
#define UDA_PLUGINS_MONGODB_MONGODBPLUGIN_H

#include <plugins/pluginStructs.h>

#ifdef __cplusplus
extern "C" {
#endif

#define THISPLUGIN_VERSION                  1
#define THISPLUGIN_MAX_INTERFACE_VERSION    1        // Interface versions higher than this will not be understood!
#define THISPLUGIN_DEFAULT_METHOD           "get"

int query(IDAM_PLUGIN_INTERFACE * idam_plugin_interface);

#ifdef __cplusplus
}
#endif

#endif // UDA_PLUGINS_MONGODB_MONGODBPLUGIN_H
