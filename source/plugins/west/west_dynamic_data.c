#include "west_dynamic_data.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <logging/logging.h>
#include <clientserver/initStructs.h>
#include <clientserver/errorLog.h>
#include <clientserver/stringUtils.h>
#include <clientserver/udaTypes.h>

#include "west_utilities.h"
#include "west_dyn_data_utilities.h"

#include "west_ece.h"
#include "west_pf_passive.h"
#include "west_pf_active.h"
#include "west_soft_x_rays.h"
#include "west_summary.h"
#include "west_lh_antennas.h"

int GetDynamicData(int shotNumber, const char* mapfun, DATA_BLOCK* data_block, int* nodeIndices)
{

	UDA_LOG(UDA_LOG_DEBUG, "Entering GetDynamicData() -- WEST plugin\n");

	assert(mapfun); //Mandatory function to get WEST data

	char* fun_name = NULL; //Shape_of, tsmat_collect, tsbase
	char* TOP_collections_parameters = NULL; //example : TOP_collections_parameters = DMAG:GMAG_BNORM:PosR, DMAG:GMAG_BTANG:PosR, ...
	char* attributes = NULL; //example: attributes = 1:float:#1 (rank = 1, type = float, #1 = second IDAM index)
	char* normalizationAttributes = NULL; //example : multiply:cste:3     (multiply value by constant factor equals to 3)

	getFunName(mapfun, &fun_name);

	UDA_LOG(UDA_LOG_DEBUG, "UDA request: %s for shot: %d\n", fun_name, shotNumber);

	if (strcmp(fun_name, "tsbase_collect") == 0) {
		tokenizeFunParameters(mapfun, &TOP_collections_parameters, &attributes, &normalizationAttributes);
		return SetNormalizedDynamicData(shotNumber, data_block, nodeIndices, TOP_collections_parameters, attributes,
				normalizationAttributes);
	} else if (strcmp(fun_name, "tsbase_time") == 0) {
		tokenizeFunParameters(mapfun, &TOP_collections_parameters, &attributes, &normalizationAttributes);
		return SetNormalizedDynamicDataTime(shotNumber, data_block, nodeIndices, TOP_collections_parameters, attributes,
				normalizationAttributes);
	} else if (strcmp(fun_name, "tsbase_collect_with_channels") == 0) {
		char* unvalid_channels = NULL; //used for interfero_polarimeter IDS, example : invalid_channels:1,2
		tokenizeFunParametersWithChannels(mapfun, &unvalid_channels, &TOP_collections_parameters, &attributes,
				&normalizationAttributes);
		return SetNormalizedDynamicData(shotNumber, data_block, nodeIndices, TOP_collections_parameters, attributes,
				normalizationAttributes);
	} else if (strcmp(fun_name, "tsbase_time_with_channels") == 0) {
		char* unvalid_channels = NULL; //used for interfero_polarimeter IDS, example : invalid_channels:1,2
		tokenizeFunParametersWithChannels(mapfun, &unvalid_channels, &TOP_collections_parameters, &attributes,
				&normalizationAttributes);
		return SetNormalizedDynamicDataTime(shotNumber, data_block, nodeIndices, TOP_collections_parameters, attributes,
				normalizationAttributes);
	} else if (strcmp(fun_name, "ece_t_e_data") == 0) {
		char* ece_mapfun = NULL;
		ece_t_e_data(shotNumber, &ece_mapfun);
		tokenizeFunParameters(ece_mapfun, &TOP_collections_parameters, &attributes, &normalizationAttributes);
		UDA_LOG(UDA_LOG_DEBUG, "TOP_collections_parameters : %s\n", TOP_collections_parameters);

		int status = SetNormalizedDynamicData(shotNumber, data_block, nodeIndices, TOP_collections_parameters, attributes,
				normalizationAttributes);
		UDA_LOG(UDA_LOG_DEBUG, "Test1");
		free(ece_mapfun);
		UDA_LOG(UDA_LOG_DEBUG, "Test2");
		return status;

	} else if (strcmp(fun_name, "ece_t_e_time") == 0) {
		char* ece_mapfun = NULL;
		ece_t_e_time(shotNumber, &ece_mapfun);
		tokenizeFunParameters(ece_mapfun, &TOP_collections_parameters, &attributes, &normalizationAttributes);
		UDA_LOG(UDA_LOG_DEBUG, "TOP_collections_parameters : %s\n", TOP_collections_parameters);
		int status = SetNormalizedDynamicDataTime(shotNumber, data_block, nodeIndices, TOP_collections_parameters, attributes,
				normalizationAttributes);
		free(ece_mapfun);
		return status;
	} else if (strcmp(fun_name, "ece_harmonic_data") == 0) {
		return ece_harmonic_data(shotNumber, data_block, nodeIndices);
	} else if (strcmp(fun_name, "ece_harmonic_time") == 0) {
		return ece_harmonic_time(shotNumber, data_block, nodeIndices);
	} else if (strcmp(fun_name, "ece_frequencies") == 0) {
		return ece_frequencies(shotNumber, data_block, nodeIndices);
	} else if (strcmp(fun_name, "ece_frequencies_time") == 0) {
		return ece_harmonic_time(shotNumber, data_block, nodeIndices); //TODO
	} else if (strcmp(fun_name, "pf_passive_current_data") == 0) {
		return pf_passive_current_data(shotNumber, data_block, nodeIndices);
	} else if (strcmp(fun_name, "pf_passive_current_time") == 0) {
		return pf_passive_current_time(shotNumber, data_block, nodeIndices);
	} else if (strcmp(fun_name, "pf_active_current_data") == 0) {
		return pf_active_current_data(shotNumber, data_block, nodeIndices);
	} else if (strcmp(fun_name, "pf_active_current_time") == 0) {
		return pf_active_current_time(shotNumber, data_block, nodeIndices);
	} else if (strcmp(fun_name, "soft_x_rays_channels_power_density_data") == 0) {
		return soft_x_rays_channels_power_density_data(shotNumber, data_block, nodeIndices); //TODO
	} else if (strcmp(fun_name, "soft_x_rays_channels_power_density_time") == 0) {
		return soft_x_rays_channels_power_density_time(shotNumber, data_block, nodeIndices); //TODO
	} else if (strcmp(fun_name, "flt1D") == 0) {
		return flt1D(mapfun, shotNumber, data_block, nodeIndices); //TODO
	} else if (strcmp(fun_name, "flt1D_contrib") == 0) {
		return flt1D_contrib(mapfun, shotNumber, data_block, nodeIndices); //TODO
	} /*else if (strcmp(fun_name, "summary_global_quantities_tau_resistance_value") == 0) {
		return summary_global_quantities_tau_resistance_value(shotNumber, data_block, nodeIndices); //TODO
	} */else if (strcmp(fun_name, "summary_global_quantities_v_loop_value") == 0) {
		return summary_global_quantities_v_loop_value(shotNumber, data_block, nodeIndices); //TODO
	} else if (strcmp(fun_name, "summary_global_quantities_b0_value") == 0) {
		return summary_global_quantities_b0_value(shotNumber, data_block, nodeIndices); //TODO
	} else if (strcmp(fun_name, "summary_global_quantities_beta_tor_value") == 0) {
		return summary_global_quantities_beta_tor_value(shotNumber, data_block, nodeIndices); //TODO
	} else if (strcmp(fun_name, "summary_heating_current_drive_ec_power") == 0) {
		return summary_heating_current_drive_ec_power(shotNumber, data_block, nodeIndices); //TODO
	} else if (strcmp(fun_name, "summary_time") == 0) {
		return summary_time(shotNumber, data_block, nodeIndices); //TODO
	}else if (strcmp(fun_name, "lh_antennas_power") == 0) {
		lh_antennas_power(shotNumber, data_block, nodeIndices);
		return 0;
	}else if (strcmp(fun_name, "lh_antennas_power_forward") == 0) {
		lh_antennas_power_forward(shotNumber, data_block, nodeIndices);
	}else if (strcmp(fun_name, "lh_antennas_power_reflected") == 0) {
		lh_antennas_power_reflected(shotNumber, data_block, nodeIndices);
	}else if (strcmp(fun_name, "lh_antennas_reflection_coefficient") == 0) {
		lh_antennas_reflection_coefficient(shotNumber, data_block, nodeIndices);
	}else if (strcmp(fun_name, "lh_antennas_modules_power") == 0) {
		lh_antennas_modules_power(shotNumber, data_block, nodeIndices);
	}else if (strcmp(fun_name, "lh_antennas_modules_power_forward") == 0) {
		lh_antennas_modules_power_forward(shotNumber, data_block, nodeIndices);
	}else if (strcmp(fun_name, "lh_antennas_modules_power_reflected") == 0) {
		lh_antennas_modules_power_reflected(shotNumber, data_block, nodeIndices);
	}else if (strcmp(fun_name, "lh_antennas_modules_reflection_coefficient") == 0) {
		lh_antennas_modules_reflection_coefficient(shotNumber, data_block, nodeIndices);
	}else if (strcmp(fun_name, "lh_antennas_modules_phase") == 0) {
		lh_antennas_modules_phase(shotNumber, data_block, nodeIndices);
	}else if (strcmp(fun_name, "lh_antennas_phase_average") == 0) {
		lh_antennas_phase_average(shotNumber, data_block, nodeIndices);
		return 0;
	}else if (strcmp(fun_name, "lh_antennas_n_parallel_peak") == 0) {
		lh_antennas_n_parallel_peak(shotNumber, data_block, nodeIndices);
	}else if (strcmp(fun_name, "lh_antennas_position_r") == 0) {
		lh_antennas_position_r(shotNumber, data_block, nodeIndices);
	}else if (strcmp(fun_name, "lh_antennas_position_z") == 0) {
		lh_antennas_position_z(shotNumber, data_block, nodeIndices);
	}else if (strcmp(fun_name, "lh_antennas_pressure_tank") == 0) {
		lh_antennas_pressure_tank(shotNumber, data_block, nodeIndices); //TODO
	}else if (strcmp(fun_name, "test_fun") == 0) {
		return test_fun(shotNumber, data_block, nodeIndices); //TODO
	}
	else {
		UDA_LOG(UDA_LOG_DEBUG, "WEST:ERROR: mapped C function not found in west_dynamic_data.c !\n");
	}


	free(fun_name);
	free(TOP_collections_parameters);
	free(attributes);
	free(normalizationAttributes);

	return 0;

}

int flt1D(const char* mappingValue, int shotNumber, DATA_BLOCK* data_block, int* nodeIndices)
{
	UDA_LOG(UDA_LOG_DEBUG, "Calling flt1D\n");
	char* diagnostic = NULL;
	char* object_name = NULL;
	int extractionIndex;
	tokenize1DArcadeParameters(mappingValue, &diagnostic, &object_name, &extractionIndex);
	int status = setUDABlockSignalFromArcade(object_name, shotNumber, extractionIndex, data_block, nodeIndices, 1);
	if (status != 0) {
		int err = 901;
		char* errorMsg = "WEST:ERROR: unable to get object ";
		strcat(errorMsg, object_name);
		strcat(errorMsg, " in west_dynamic_data_data.c:flt1D method.");
		addIdamError(CODEERRORTYPE, errorMsg, err, "");
	}
	free(diagnostic);
	free(object_name);
	return status;
}

int flt1D_normalize(const char* mappingValue, int shotNumber, DATA_BLOCK* data_block, int* nodeIndices, float normalizationFactor)
{
	UDA_LOG(UDA_LOG_DEBUG, "Calling flt1D\n");
	char* diagnostic = NULL;
	char* object_name = NULL;
	int extractionIndex;
	tokenize1DArcadeParameters(mappingValue, &diagnostic, &object_name, &extractionIndex);
	int status = setUDABlockSignalFromArcade(object_name, shotNumber, extractionIndex, data_block, nodeIndices, normalizationFactor);
	if (status != 0) {
		int err = 901;
		char* errorMsg = "WEST:ERROR: unable to get object ";
		strcat(errorMsg, object_name);
		strcat(errorMsg, " in west_dynamic_data_data.c:flt1D method.");
		addIdamError(CODEERRORTYPE, errorMsg, err, "");
	}
	free(diagnostic);
	free(object_name);
	return status;
}

int flt1D_contrib(const char* mappingValue, int shotNumber, DATA_BLOCK* data_block, int* nodeIndices)
{
	UDA_LOG(UDA_LOG_DEBUG, "Calling flt1D_contrib\n");
	char* diagnostic = NULL;
	char* object_name = NULL;
	int extractionIndex;

	char* diagnostic2 = NULL;
	char* object_name2 = NULL;
	int extractionIndex2;

	tokenize1DArcadeParameters2(mappingValue, &diagnostic, &object_name, &extractionIndex, &diagnostic2, &object_name2, &extractionIndex2);

	UDA_LOG(UDA_LOG_DEBUG, "setting UDA block in flt1D_contrib\n");

	int status = setUDABlockSignalFromArcade2(shotNumber, object_name, extractionIndex, object_name2, extractionIndex2, data_block, nodeIndices, 1);
	UDA_LOG(UDA_LOG_DEBUG, "after setting UDA block in flt1D_contrib\n");
	if (status != 0) {
		int err = 901;
		char* errorMsg = "WEST:ERROR: unable to get object ";
		strcat(errorMsg, object_name);
		strcat(errorMsg, " in west_dynamic_data_data.c:flt1D_contrib method.");
		addIdamError(CODEERRORTYPE, errorMsg, err, "");
	}
	free(diagnostic);
	free(object_name);
	free(diagnostic2);
	free(object_name2);
	return status;
}

