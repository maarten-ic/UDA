#ifndef IDAM_PLUGINS_EQUIMAP_IMPORTDATA_H
#define IDAM_PLUGINS_EQUIMAP_IMPORTDATA_H

#include <clientserver/idamStructs.h>
#include "equimap.h"

int whichHandle(char *name);
char *whichName(char *name);
int whichIndex(char *name);
int selectTimes(EQUIMAPDATA * equimapdata);
int subsetTimes(REQUEST_BLOCK *request_block);
int imputeData(char *signal);
float lineInt(float *arr, float *x, int narr);
int importData(REQUEST_BLOCK *request_block, EQUIMAPDATA * equimapdata);
int extractData(float tslice, EFITDATA *efitdata, EQUIMAPDATA * equimapdata);

#endif // IDAM_PLUGINS_EQUIMAP_IMPORTDATA_H
