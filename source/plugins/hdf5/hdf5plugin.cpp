#include "hdf5plugin.h"

#include "readHDF58.h"

HDF5Plugin::HDF5Plugin() : UDAPluginBase("HDF5", 1, "read", "")
{
    register_method("read", static_cast<UDAPluginBase::plugin_member_type>(&HDF5Plugin::read));
    register_method("hello", static_cast<UDAPluginBase::plugin_member_type>(&HDF5Plugin::hello));
    register_function("foo", &HDF5Plugin::foo);
}

/**
 * Entry function
 */
extern int hdf5Plugin(UDA_PLUGIN_INTERFACE* plugin_interface)
{
    static HDF5Plugin plugin = {};
    return plugin.call(plugin_interface);
}

//----------------------------------------------------------------------------------------
// Read data from a HDF5 File

int HDF5Plugin::read(UDA_PLUGIN_INTERFACE* plugin_interface)
{
    auto file_path = find_required_arg<std::string>(plugin_interface, "file_path");
    auto cdf_path = find_required_arg<std::string>(plugin_interface, "cdf_path");

    // Legacy data reader!
    int err = read_hdf5(plugin_interface, file_path, cdf_path);

    return err;
}
