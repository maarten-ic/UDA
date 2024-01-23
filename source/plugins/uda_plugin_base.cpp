#include "uda_plugin_base.hpp"
#include "clientserver/stringUtils.h"
#include "udaPlugin.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>

int UDAPluginBase::call(IDAM_PLUGIN_INTERFACE* plugin_interface)
{
    try {
        if (plugin_interface->interfaceVersion > interface_version_) {
            RAISE_PLUGIN_ERROR("Plugin Interface Version Unknown to this plugin: Unable to execute the request!");
        }

        plugin_interface->pluginVersion = version_;

        do_reset();
        do_init(plugin_interface);

        //----------------------------------------------------------------------------------------
        // Plugin Functions
        //----------------------------------------------------------------------------------------

        std::string function = get_function(plugin_interface);

        int rc;

        if (function_map_.find(function) != function_map_.end()) {
            rc = function_map_.at(function)(plugin_interface);
        } else if (method_map_.find(function) != method_map_.end()) {
            auto fn = method_map_.at(function);
            rc = (this->*fn)(plugin_interface);
        } else {
            UDA_LOG(UDA_LOG_ERROR, "Unknown function requested %s\n", function.c_str());
            udaAddError(UDA_CODE_ERROR_TYPE, "UDAPluginBase::call", 999, "Unknown function requested");
            udaConcatError(&plugin_interface->error_stack);
            return 999;
        }

        udaConcatError(&plugin_interface->error_stack);
        return rc;
    } catch (std::exception& ex) {
        UDA_LOG(UDA_LOG_ERROR, "Exception: %s\n", ex.what());
        udaAddError(UDA_CODE_ERROR_TYPE, "UDAPluginBase::call", 999, ex.what());
        udaConcatError(&plugin_interface->error_stack);
        return 999;
    }
}

std::string UDAPluginBase::get_function(IDAM_PLUGIN_INTERFACE* plugin_interface)
{
    REQUEST_DATA* request = plugin_interface->request_data;
    return boost::to_lower_copy<std::string>(request->function);
}

void UDAPluginBase::do_init(IDAM_PLUGIN_INTERFACE* plugin_interface)
{
    std::string function = get_function(plugin_interface);
    if (!init_ || (function == "init" || function == "initialise")) {
        init(plugin_interface);
        init_ = true;
    }
}

void UDAPluginBase::do_reset()
{
    if (!init_) {
        // Not previously initialised: Nothing to do!
        return;
    }

    reset();
    init_ = false;
}

int UDAPluginBase::help(IDAM_PLUGIN_INTERFACE* plugin_interface)
{
    std::string desc = name_ + ": help = description of this plugin";

    if (help_file_.empty()) {
        const char* help = "No help available";
        return setReturnDataString(plugin_interface->data_block, help, desc.c_str());
    }

    auto path = boost::filesystem::path(help_file_);
    if (!boost::filesystem::exists(path)) {
        auto help = fmt::format("help file {} does not exist", path.string());
        return setReturnDataString(plugin_interface->data_block, help.c_str(), desc.c_str());
    }

    std::ifstream help_file{path.string()};
    std::stringstream buffer;
    buffer << help_file.rdbuf();
    auto help = buffer.str();

    return setReturnDataString(plugin_interface->data_block, help.c_str(), desc.c_str());
}

int UDAPluginBase::version(IDAM_PLUGIN_INTERFACE* plugin_interface)
{
    return setReturnDataIntScalar(plugin_interface->data_block, version_, "Plugin version number");
}

int UDAPluginBase::build_date(IDAM_PLUGIN_INTERFACE* plugin_interface)
{
    return setReturnDataString(plugin_interface->data_block, __DATE__, "Plugin build date");
}

int UDAPluginBase::default_method(IDAM_PLUGIN_INTERFACE* plugin_interface)
{
    return setReturnDataString(plugin_interface->data_block, default_method_.c_str(), "Plugin default method");
}

int UDAPluginBase::max_interface_version(IDAM_PLUGIN_INTERFACE* plugin_interface)
{
    return setReturnDataIntScalar(plugin_interface->data_block, interface_version_, "Maximum Interface Version");
}

void UDAPluginBase::register_method(const std::string& name, plugin_member_type plugin_method)
{
    method_map_[name] = plugin_method;
}

void UDAPluginBase::register_function(const std::string& name, plugin_function_type plugin_function)
{
    function_map_[name] = plugin_function;
}

bool UDAPluginBase::has_arg(IDAM_PLUGIN_INTERFACE* plugin_interface, const std::string& name)
{
    const char* str;
    return findStringValue(&plugin_interface->request_data->nameValueList, &str, name.c_str());
}
