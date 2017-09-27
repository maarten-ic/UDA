
#ifndef IDAM_PLUGIN_WEST_PF_PASSIVE_H
#define IDAM_PLUGIN_WEST_PF_PASSIVE_H

#include <clientserver/udaStructs.h>

void pf_passive_elements_shapeOf(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);
void pf_passive_coils_shapeOf(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);
void pf_passive_R(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);
void pf_passive_Z(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);
void pf_passive_H(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);
void pf_passive_W(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);
void pf_passive_element_identifier(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);
void pf_passive_element_name(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);
void pf_passive_coil_identifier(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);
void pf_passive_coil_name(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);
void pf_passive_turns(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);
void pf_passive_current_data(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);
void pf_passive_current_time(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);


#endif // IDAM_PLUGIN_WEST_PF_PASSIVE_H
