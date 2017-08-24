#if 0
#!/bin/bash
g++ test_pf_passive.cpp -g -O0 -gdwarf-3 -o test -DHOME=$HOME -I$HOME/iter/uda/source -I$HOME/iter/uda/source/wrappers \
-L$HOME/iter/uda/lib -Wl,-rpath,$HOME/iter/uda/lib  -luda_cpp -lssl -lcrypto -lxml2
exit 0
#endif

#include <c++/UDA.hpp>
#include <typeinfo>
#include <iostream>
#include <sstream>
#include <fstream>

#define QUOTE_(X) #X
#define QUOTE(X) QUOTE_(X)
#define SHOT_NUM_TORE_SUPRA "43979" // WEST
#define SHOT_NUM "51371" // WEST

int main() {
	setenv("UDA_PLUGIN_DIR", QUOTE(HOME) "/iter/uda/etc/plugins", 1);
	setenv("UDA_PLUGIN_CONFIG", QUOTE(HOME) "/iter/uda/test/idamTest.conf", 1);
	setenv("UDA_SARRAY_CONFIG", QUOTE(HOME) "/iter/uda/bin/plugins/idamgenstruct.conf", 1);
	setenv("UDA_WEST_MAPPING_FILE", QUOTE(HOME) "/iter/uda/source/plugins/west/WEST_mappings/IDAM_mapping.xml", 1);
	setenv("UDA_WEST_MAPPING_FILE_DIRECTORY", QUOTE(HOME) "/iter/uda/source/plugins/west/WEST_mappings", 1);
	setenv("UDA_LOG", QUOTE(HOME) "/iter/uda/", 1);
	setenv("UDA_LOG_MODE", "a", 1);
	setenv("UDA_LOG_LEVEL", "DEBUG", 1);
	setenv("UDA_DEBUG_APPEND", "a", 1);

	uda::Client::setProperty(uda::PROP_DEBUG, true);
	uda::Client::setProperty(uda::PROP_VERBOSE, true);

	uda::Client::setServerHostName("localhost");
	uda::Client::setServerPort(56565);

	uda::Client client;

	const uda::Result& passive_name = client.get("imas::get(idx=0, group='pf_passive', variable='loop/1/name', expName='WEST', type=string, rank=1, shot=" SHOT_NUM ", )", "");
	const uda::Data * passive_name_data = passive_name.data();
	const uda::String* s_passive_name_data = dynamic_cast<const uda::String*>(passive_name_data);

	std::cout << "loop/1/name : ";
	std::cout << s_passive_name_data->str();
	std::cout << std::endl;

	const uda::Result& passive_name2 = client.get("imas::get(idx=0, group='pf_passive', variable='loop/2/name', expName='WEST', type=string, rank=1, shot=" SHOT_NUM ", )", "");
	const uda::Data * passive_name_data2 = passive_name2.data();
	const uda::String* s_passive_name_data2 = dynamic_cast<const uda::String*>(passive_name_data2);

	std::cout << "loop/2/name : ";
	std::cout << s_passive_name_data2->str();
	std::cout << std::endl;

	const uda::Result& passive_name11 = client.get("imas::get(idx=0, group='pf_passive', variable='loop/11/name', expName='WEST', type=string, rank=1, shot=" SHOT_NUM ", )", "");
	const uda::Data * passive_name_data11 = passive_name11.data();
	const uda::String* s_passive_name_data11 = dynamic_cast<const uda::String*>(passive_name_data11);

	std::cout << "loop/11/name : ";
	std::cout << s_passive_name_data11->str();
	std::cout << std::endl;

	const uda::Result& passive_name12 = client.get("imas::get(idx=0, group='pf_passive', variable='loop/12/name', expName='WEST', type=string, rank=1, shot=" SHOT_NUM ", )", "");
	const uda::Data * passive_name_data12 = passive_name12.data();
	const uda::String* s_passive_name_data12 = dynamic_cast<const uda::String*>(passive_name_data12);

	std::cout << "loop/12/name : ";
	std::cout << s_passive_name_data12->str();
	std::cout << std::endl;


	const uda::Result& current = client.get("imas::get(idx=0, group='pf_passive', variable='loop/1/current', expName='WEST', type=double, rank=1, shot=" SHOT_NUM ", )", "");
	const uda::Data * current_data = current.data();
	const uda::Array* arr_current_data = dynamic_cast<const uda::Array*>(current_data);

	std::cout << "values for loop/1/current from 200 to 210: ";
	for (int j = 199; j < 209; ++j) {
		std::cout << arr_current_data->as<double>().at(j) << " ";
	}
	std::cout << "..." << std::endl;





	return 0;
}

