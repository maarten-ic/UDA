#ifndef IDAM_PLUGIN_WEST_DYN_DATA_UTILITIES_H
#define IDAM_PLUGIN_WEST_DYN_DATA_UTILITIES_H

#include <clientserver/udaStructs.h>

int SetNormalizedDynamicData(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices,
		char* TOP_collections_parameters, char* attributes, char* normalizationAttributes);

int SetNormalizedDynamicDataTime(int shotNumber, DATA_BLOCK* data_block, int* nodeIndices,
		char* TOP_collections_parameters, char* attributes, char* normalizationAttributes);

int GetNormalizedDynamicData(int shotNumber, float** time, float** data, int* len, int* nodeIndices,
		char* TOP_collections_parameters, char* attributes, char* normalizationAttributes);

int GetDynData(int shotNumber, float** time, float** data, int* len, int* nodeIndices,
		char* TOP_collections_parameters, char* attributes);

void SetDynamicData(DATA_BLOCK* data_block, int len, float *time, float *data);

void SetDynamicDataTime(DATA_BLOCK* data_block, int len, float *time, float *data);

int getArcadeSignal(char* nomsigp, int shotNumber, int extractionIndex, float** time, float** data, int* len, float normalizationFactor);

int flt1D(const char* mappingValue, int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);
int flt1D_contrib(const char* mappingValue, int shotNumber, DATA_BLOCK* data_block, int* nodeIndices);

int setUDABlockSignalFromArcade(char* sigName, int shotNumber, int extractionIndex, DATA_BLOCK* data_block, int* nodeIndices, float normalizationFactor);
int setUDABlockSignalFromArcade2(int shotNumber, char* sigName, int extractionIndex, char* sigName2, int extractionIndex2, DATA_BLOCK* data_block, int* nodeIndices, float treshold);
void merge2Signals_according_to_ip_treshold(float **data, int len, float *data1, float *data2, float *ip, float treshold);
void averageArcadeSignal(char* sigName, int shotNumber, int extractions[], int extractions_length, float** time, float** averaged_data, int* len);

int signalsRatio(float **result_q_by_r, float *q, float *r, int lenq, int lenr);
int signalsSquare(float **square_s, float *s, int len);
int multiplySignals(float **result, float *p, float *q, int len);

void sum(float* sum_data, float* data, int len);
void normalize(float* sum_data, int len, int normalizationFactor);
void multiply(float* data, int len, float factor);


#endif // IDAM_PLUGIN_WEST_DYN_DATA_UTILITIES_H
