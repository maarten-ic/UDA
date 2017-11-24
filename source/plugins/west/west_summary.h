
#ifndef IDAM_PLUGIN_WEST_SUMMARY_H
#define IDAM_PLUGIN_WEST_SUMMARY_H

#include <clientserver/udaStructs.h>

void summary_global_quantities_beta_tor_value(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);
void summary_global_quantities_tau_resistance_value(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);
void summary_global_quantities_v_loop_value(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);
void summary_global_quantities_r0_value(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);
void summary_global_quantities_b0_value(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);
void summary_heating_current_drive_ec_power_source(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);
void summary_heating_current_drive_ec_power(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);
void ip_value(float **ip_data, float *ip_time, int *ip_len, int shotNumber, DATA_BLOCK* data_block, const float treshold);

#endif // IDAM_PLUGIN_WEST_SUMMARY_H
