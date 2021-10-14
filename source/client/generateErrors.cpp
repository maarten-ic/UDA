/*---------------------------------------------------------------
* IDAM Model Based Symmetric/Asymmetric Error Data Generation
*
* Input Arguments:
* Returns:
*
*
*
* Notes:
*
*--------------------------------------------------------------*/

#include "generateErrors.h"

#include <math.h>
#include <cstdlib>

#include <clientserver/udaTypes.h>
#include <clientserver/errorLog.h>
#include <clientserver/allocData.h>

#include "accAPI.h"

#ifndef NO_GSL_LIB
#  include <gsl/gsl_randist.h>
#endif

//--------------------------------------------------------------------------------------------------------------
// Generate Error Data

int idamErrorModel(int model, int param_n, float* params, int data_n, float* data, int* asymmetry, float* errhi,
                   float* errlo)
{
    *asymmetry = 0;        // No Error Asymmetry for most models

    switch (model) {

        case ERROR_MODEL_DEFAULT:
            if (param_n != 2) return 1;
            for (int i = 0; i < data_n; i++) {
                errhi[i] = params[0] + params[1] * fabsf(data[i]);
                errlo[i] = errhi[i];
            }
            break;

        case ERROR_MODEL_DEFAULT_ASYMMETRIC:
            if (param_n != 4) return 1;
            *asymmetry = 1;
            for (int i = 0; i < data_n; i++) {
                errhi[i] = params[0] + params[1] * fabsf(data[i]);
                errlo[i] = params[2] + params[3] * fabsf(data[i]);
            }
            break;

        default:
            for (int i = 0; i < data_n; i++) {
                errhi[i] = 0.0;
                errlo[i] = 0.0;
            }
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------------------
// Generate Synthetic Data using Random Number generators * PDFs
//
// NB: to change the default GSL library settings use the following environment variables
//	GSL_RNG_SEED	12345		for the seed value
//	GSL_RNG_TYPE	mrg		for the name of the random number generator

int idamSyntheticModel(int model, int param_n, float* params, int data_n, float* data)
{

#ifdef NO_GSL_LIB
    int err = 999;
    addIdamError(CODEERRORTYPE, "idamSyntheticModel", err,
                 "Random Number Generators from the GSL library required.");
    return 999;
#else
    float shift;
    static gsl_rng *random=nullptr;

    if(random == nullptr) {		// Initialise the Random Number System
        gsl_rng_env_setup();
        random = gsl_rng_alloc (gsl_rng_default);
        gsl_rng_set(random, (unsigned long int) ERROR_MODEL_SEED);	// Seed the Random Number generator with the library default
    }

    switch (model) {

    case ERROR_MODEL_GAUSSIAN:  	// Standard normal Distribution
        if(param_n < 1 || param_n > 2) return 1;
        if(param_n == 2) gsl_rng_set(random, (unsigned long int) params[1]);		// Change the Seed before Sampling
        for(i=0; i<data_n; i++) data[i] = data[i] + (float) gsl_ran_gaussian (random, (double) params[0]); // Random sample from PDF
        break;

    case ERROR_MODEL_RESEED:  							// Change the Seed
        if(param_n == 1) gsl_rng_set(random, (unsigned long int) params[0]);
        break;

    case ERROR_MODEL_GAUSSIAN_SHIFT:
        if(param_n < 1 || param_n > 2) return 1;
        if(param_n == 2) gsl_rng_set(random, (unsigned long int) params[1]);		// Change the Seed before Sampling
        shift = (float) gsl_ran_gaussian (random, (double) params[0]);
        for(i=0; i<data_n; i++) data[i] = data[i] + shift;				// Random linear shift of data array
        break;

    case ERROR_MODEL_POISSON:
        if(param_n < 0 || param_n > 1) return 1;
        if(param_n == 1) gsl_rng_set(random, (unsigned long int) params[0]);				// Change the Seed before Sampling
        for(i=0; i<data_n; i++) data[i] = data[i] + (float) gsl_ran_poisson(random, (double)data[i]);	// Randomly perturb data array
        break;
    }

    return 0;
#endif
}

int generateIdamSyntheticData(int handle)
{
    int err = 0;

    char* synthetic = nullptr;

    //--------------------------------------------------------------------------------------------------------------
    // Check the handle and model are ok

    if (getIdamData(handle) == nullptr) return 0;

    int model;
    int param_n;
    float params[MAXERRPARAMS];

    getIdamErrorModel(handle, &model, &param_n, params);

    if (model <= ERROR_MODEL_UNKNOWN || model >= ERROR_MODEL_UNDEFINED) return 0;    // No valid Model

    if (getIdamDataNum(handle) <= 0) return 0;

    //--------------------------------------------------------------------------------------------------------------
    // Allocate local float work arrays and copy the data array to the work array

    if (getIdamDataType(handle) == UDA_TYPE_DCOMPLEX || getIdamDataType(handle) == UDA_TYPE_COMPLEX) {
        err = 999;
        addIdamError(CODEERRORTYPE, "generateIdamSyntheticData", err,
                     "Not configured to Generate Complex Type Synthetic Data");
        return 999;
    }

    float* data = nullptr;
    if ((data = (float*)malloc(getIdamDataNum(handle) * sizeof(float))) == nullptr) {
        return 1;
    }

    switch (getIdamDataType(handle)) {
        case UDA_TYPE_FLOAT: {
            auto fp = (float*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = fp[i];        // Cast all data to Float
            break;
        }
        case UDA_TYPE_DOUBLE: {
            auto dp = (double*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float)dp[i];
            break;
        }
        case UDA_TYPE_SHORT: {
            auto sp = (short*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float)sp[i];
            break;
        }
        case UDA_TYPE_INT: {
            auto ip = (int*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float)ip[i];
            break;
        }
        case UDA_TYPE_LONG: {
            auto lp = (long*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float)lp[i];
            break;
        }
        case UDA_TYPE_LONG64: {
            auto lp = (long long int*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float)lp[i];
            break;
        }
        case UDA_TYPE_CHAR: {
            auto cp = (char*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float)cp[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_SHORT: {
            auto sp = (unsigned short*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float)sp[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_INT: {
            auto up = (unsigned int*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float)up[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_LONG: {
            auto lp = (unsigned long*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float)lp[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_LONG64: {
            auto lp = (unsigned long long int *) getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float) lp[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_CHAR: {
            auto cp = (unsigned char*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float)cp[i];
            break;
        }

        default:
            free(data);
            return 0;
    }

    //--------------------------------------------------------------------------------------------------------------
    // Generate Synthetic Data

    err = idamSyntheticModel(model, param_n, params, getIdamDataNum(handle), data);

    if (err != 0) {
        addIdamError(CODEERRORTYPE, "generateIdamSyntheticData", err,
                     "Unable to Generate Synthetic Data");
        free(data);
        return err;
    }

    //--------------------------------------------------------------------------------------------------------------
    // Return the Synthetic Data

    if (acc_getSyntheticData(handle) == nullptr) {
        if ((err = allocArray(getIdamDataType(handle), getIdamDataNum(handle), &synthetic))) {
            addIdamError(CODEERRORTYPE, "generateIdamSyntheticData", err,
                         "Problem Allocating Heap Memory for Synthetic Data");
            return err;
        }
        acc_setSyntheticData(handle, synthetic);
    }

    switch (getIdamDataType(handle)) {
        case UDA_TYPE_FLOAT: {
            auto fp = (float*)acc_getSyntheticData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) fp[i] = data[i];        // Overwrite the Data
            break;
        }
        case UDA_TYPE_DOUBLE: {
            auto dp = (double*)acc_getSyntheticData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) dp[i] = (double)data[i];
            break;
        }
        case UDA_TYPE_SHORT: {
            auto sp = (short*)acc_getSyntheticData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) sp[i] = (short)data[i];
            break;
        }
        case UDA_TYPE_INT: {
            auto ip = (int*)acc_getSyntheticData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) ip[i] = (int)data[i];
            break;
        }
        case UDA_TYPE_LONG: {
            auto lp = (long*)acc_getSyntheticData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) lp[i] = (long)data[i];
            break;
        }
        case UDA_TYPE_LONG64: {
            auto lp = (long long int*)acc_getSyntheticData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) lp[i] = (long long int)data[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_SHORT: {
            auto sp = (unsigned short*)acc_getSyntheticData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) sp[i] = (unsigned short)data[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_INT: {
            auto up = (unsigned int*)acc_getSyntheticData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) up[i] = (unsigned int)data[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_LONG: {
            auto lp = (unsigned long*)acc_getSyntheticData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) lp[i] = (unsigned long)data[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_LONG64: {
            auto lp = (unsigned long long int *) acc_getSyntheticData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) lp[i] = (unsigned long long int) data[i];
            break;
        }
        case UDA_TYPE_CHAR: {
            auto cp = (char*)acc_getSyntheticData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) cp[i] = (char)data[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_CHAR: {
            auto cp = (unsigned char*)acc_getSyntheticData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) cp[i] = (unsigned char)data[i];
            break;
        }

    }

    //--------------------------------------------------------------------------------------------------------------
    // Housekeeping

    free(data);

    return 0;
}

int generateIdamSyntheticDimData(int handle, int ndim)
{
    int err = 0;

    char* synthetic = nullptr;

    //--------------------------------------------------------------------------------------------------------------
    // Check the handle and model are ok

    if (getIdamData(handle) == nullptr) return 0;            // No Data Block available

    if (ndim < 0 || ndim > getIdamRank(handle)) return 0;

    int model;
    int param_n;
    float params[MAXERRPARAMS];

    getIdamErrorModel(handle, &model, &param_n, params);

    if (model <= ERROR_MODEL_UNKNOWN || model >= ERROR_MODEL_UNDEFINED) return 0; // No valid Model

    if (getIdamDimNum(handle, ndim) <= 0) return 0;

    //--------------------------------------------------------------------------------------------------------------
    // Allocate local float work arrays and copy the data array to the work array

    if (getIdamDataType(handle) == UDA_TYPE_DCOMPLEX || getIdamDataType(handle) == UDA_TYPE_COMPLEX) {
        err = 999;
        addIdamError(CODEERRORTYPE, "generateIdamSyntheticDimData", err,
                     "Not configured to Generate Complex Type Synthetic Data");
        return 999;
    }

    float* data;
    if ((data = (float*)malloc(getIdamDimNum(handle, ndim) * sizeof(float))) == nullptr) {
        addIdamError(CODEERRORTYPE, "generateIdamSyntheticDimData", 1,
                     "Problem Allocating Heap Memory for Synthetic Dimensional Data");
        return 1;
    }

    switch (getIdamDataType(handle)) {
        case UDA_TYPE_FLOAT: {
            auto fp = (float*)getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = fp[i];        // Cast all data to Float
            break;
        }
        case UDA_TYPE_DOUBLE: {
            auto dp = (double*)getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float)dp[i];
            break;
        }
        case UDA_TYPE_INT: {
            auto ip = (int*)getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float)ip[i];
            break;
        }
        case UDA_TYPE_SHORT: {
            auto sp = (short*)getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float)sp[i];
            break;
        }
        case UDA_TYPE_LONG: {
            auto lp = (long*)getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float)lp[i];
            break;
        }
        case UDA_TYPE_LONG64: {
            auto lp = (long long int*)getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float)lp[i];
            break;
        }
        case UDA_TYPE_CHAR: {
            char* cp = (char*)getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float)cp[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_SHORT: {
            auto sp = (unsigned short*)getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float)sp[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_INT: {
            auto up = (unsigned int*)getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float)up[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_LONG: {
            auto lp = (unsigned long*)getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float)lp[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_LONG64: {
            auto lp = (unsigned long long int *) getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float) lp[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_CHAR: {
            auto cp = (unsigned char*)getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float)cp[i];
            break;
        }
        default:
            free(data);
            return 0;
    }

    //--------------------------------------------------------------------------------------------------------------
    // Generate Model Data

    err = idamSyntheticModel(model, param_n, params, getIdamDimNum(handle, ndim), data);

    if (err != 0) {
        addIdamError(CODEERRORTYPE, "generateIdamSyntheticDimData", err,
                     "Unable to Generate Synthetic Dimensional Data");
        free(data);
        return err;
    }

    //--------------------------------------------------------------------------------------------------------------
    // Return the Synthetic Data

    if (acc_getSyntheticDimData(handle, ndim) == nullptr) {
        if ((err = allocArray(getIdamDimType(handle, ndim), getIdamDimNum(handle, ndim), &synthetic))) {
            addIdamError(CODEERRORTYPE, "generateIdamSyntheticDimData", err,
                         "Problem Allocating Heap Memory for Synthetic Dimensional Data");
            return err;
        }

        acc_setSyntheticDimData(handle, ndim, synthetic);
    }

    switch (getIdamDimType(handle, ndim)) {
        case UDA_TYPE_FLOAT: {
            auto fp = (float*)acc_getSyntheticDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) fp[i] = data[i];
            break;
        }
        case UDA_TYPE_DOUBLE: {
            auto dp = (double*)acc_getSyntheticDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) dp[i] = (double)data[i];
            break;
        }
        case UDA_TYPE_SHORT: {
            auto sp = (short*)acc_getSyntheticDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) sp[i] = (short)data[i];
            break;
        }
        case UDA_TYPE_INT: {
            auto ip = (int*)acc_getSyntheticDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) ip[i] = (int)data[i];
            break;
        }
        case UDA_TYPE_LONG: {
            auto lp = (long*)acc_getSyntheticDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) lp[i] = (long)data[i];
            break;
        }
        case UDA_TYPE_LONG64: {
            auto lp = (long long int*)acc_getSyntheticDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) lp[i] = (long long int)data[i];
            break;
        }
        case UDA_TYPE_CHAR: {
            auto cp = acc_getSyntheticDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) cp[i] = (char)data[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_SHORT: {
            auto sp = (unsigned short*)acc_getSyntheticDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) sp[i] = (unsigned short)data[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_INT: {
            auto up = (unsigned int*)acc_getSyntheticDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) up[i] = (unsigned int)data[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_LONG: {
            auto lp = (unsigned long*)acc_getSyntheticDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) lp[i] = (unsigned long)data[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_LONG64: {
            auto lp = (unsigned long long int *) acc_getSyntheticDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) lp[i] = (unsigned long long int) data[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_CHAR: {
            auto cp = (unsigned char*)acc_getSyntheticDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) cp[i] = (unsigned char)data[i];
            break;
        }
    }

    //--------------------------------------------------------------------------------------------------------------
    // Housekeeping

    free(data);

    return 0;
}

int generateIdamDataError(int handle)
{
    int err = 0, asymmetry = 0;

    char* perrlo = nullptr;

    //--------------------------------------------------------------------------------------------------------------
    // Check the handle and model are ok

    if (getIdamData(handle) == nullptr) return 0;            // No Data Block available

    int model;
    int param_n;
    float params[MAXERRPARAMS];

    getIdamErrorModel(handle, &model, &param_n, params);

    if (model <= ERROR_MODEL_UNKNOWN || model >= ERROR_MODEL_UNDEFINED) return 0;    // No valid Model

    if (getIdamDataNum(handle) <= 0) return 0;

    //--------------------------------------------------------------------------------------------------------------
    // Allocate local float work arrays and copy the data array to the work array

    if (getIdamDataType(handle) == UDA_TYPE_DCOMPLEX || getIdamDataType(handle) == UDA_TYPE_COMPLEX) {
        err = 999;
        addIdamError(CODEERRORTYPE, "generateIdamDataError", err,
                     "Not configured to Generate Complex Type Synthetic Data");
        return 999;
    }

    float* data;
    char* errhi;
    char* errlo;

    if ((data = (float*)malloc(getIdamDataNum(handle) * sizeof(float))) == nullptr) return 1;
    if ((errhi = (char*)malloc(getIdamDataNum(handle) * sizeof(float))) == nullptr) return 1;
    if ((errlo = (char*)malloc(getIdamDataNum(handle) * sizeof(float))) == nullptr) return 1;

    switch (getIdamDataType(handle)) {
        case UDA_TYPE_FLOAT: {
            auto fp = (float*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = fp[i];        // Cast all data to Float
            break;
        }
        case UDA_TYPE_DOUBLE: {
            auto dp = (double*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float)dp[i];
            break;
        }
        case UDA_TYPE_INT: {
            auto ip = (int*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float)ip[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_INT: {
            auto up = (unsigned int*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float)up[i];
            break;
        }
        case UDA_TYPE_LONG: {
            auto lp = (long*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float)lp[i];
            break;
        }
        case UDA_TYPE_LONG64: {
            auto lp = (long long int*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float)lp[i];
            break;
        }
        case UDA_TYPE_SHORT: {
            auto sp = (short*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float)sp[i];
            break;
        }
        case UDA_TYPE_CHAR: {
            char* cp = (char*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float)cp[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_LONG: {
            auto lp = (unsigned long*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float)lp[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_LONG64: {
            auto lp = (unsigned long long int *) getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float) lp[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_SHORT: {
            auto sp = (unsigned short*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float)sp[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_CHAR: {
            auto cp = (unsigned char*)getIdamData(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) data[i] = (float)cp[i];
            break;
        }
        default:
            free(data);
            free(errhi);
            free(errlo);
            return 0;
    }

    //--------------------------------------------------------------------------------------------------------------
    // Generate Error Data

    err = idamErrorModel(model, param_n, params, getIdamDataNum(handle), data, &asymmetry, (float*)errhi,
                         (float*)errlo);

    if (err != 0) {
        free(data);
        free(errhi);
        free(errlo);
        return err;
    }

    //--------------------------------------------------------------------------------------------------------------
    // Return the Error Array

    acc_setIdamDataErrType(handle, getIdamDataType(handle));
    acc_setIdamDataErrAsymmetry(handle, asymmetry);

    if (asymmetry && getIdamDataErrLo(handle) == nullptr) {
        if ((err = allocArray(getIdamDataType(handle), getIdamDataNum(handle), &perrlo))) return err;
        acc_setIdamDataErrLo(handle, perrlo);
    }

    switch (getIdamDataType(handle)) {
        case UDA_TYPE_FLOAT: {
            auto feh = (float*)getIdamDataErrHi(handle);
            auto fel = (float*)getIdamDataErrLo(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) {
                feh[i] = (float)errhi[i];
                if (getIdamDataErrAsymmetry(handle)) fel[i] = (float)errlo[i];
            }
            break;
        }
        case UDA_TYPE_DOUBLE: {
            auto deh = (double*)getIdamDataErrHi(handle);
            auto del = (double*)getIdamDataErrLo(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) {
                deh[i] = (double)errhi[i];
                if (getIdamDataErrAsymmetry(handle)) del[i] = (double)errlo[i];
            }
            break;
        }
        case UDA_TYPE_INT: {
            auto ieh = (int*)getIdamDataErrHi(handle);
            auto iel = (int*)getIdamDataErrLo(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) {
                ieh[i] = (int)errhi[i];
                if (getIdamDataErrAsymmetry(handle)) iel[i] = (int)errlo[i];
            }
            break;
        }
        case UDA_TYPE_LONG: {
            auto leh = (long*)getIdamDataErrHi(handle);
            auto lel = (long*)getIdamDataErrLo(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) {
                leh[i] = (long)errhi[i];
                if (getIdamDataErrAsymmetry(handle)) lel[i] = (long)errlo[i];
            }
            break;
        }
        case UDA_TYPE_LONG64: {
            auto leh = (long long int*)getIdamDataErrHi(handle);
            auto lel = (long long int*)getIdamDataErrLo(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) {
                leh[i] = (long long int)errhi[i];
                if (getIdamDataErrAsymmetry(handle)) lel[i] = (long long int)errlo[i];
            }
            break;
        }
        case UDA_TYPE_SHORT: {
            auto seh = (short*)getIdamDataErrHi(handle);
            auto sel = (short*)getIdamDataErrLo(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) {
                seh[i] = (short)errhi[i];
                if (getIdamDataErrAsymmetry(handle)) sel[i] = (short)errlo[i];
            }
            break;
        }
        case UDA_TYPE_CHAR: {
            char* ceh = getIdamDataErrHi(handle);
            char* cel = getIdamDataErrLo(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) {
                ceh[i] = errhi[i];
                if (getIdamDataErrAsymmetry(handle)) cel[i] = errlo[i];
            }
            break;
        }
        case UDA_TYPE_UNSIGNED_INT: {
            auto ueh = (unsigned int*)getIdamDataErrHi(handle);
            auto uel = (unsigned int*)getIdamDataErrLo(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) {
                ueh[i] = (unsigned int)errhi[i];
                if (getIdamDataErrAsymmetry(handle)) uel[i] = (unsigned int)errlo[i];
            }
            break;
        }
        case UDA_TYPE_UNSIGNED_LONG: {
            auto leh = (unsigned long*)getIdamDataErrHi(handle);
            auto lel = (unsigned long*)getIdamDataErrLo(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) {
                leh[i] = (unsigned long)errhi[i];
                if (getIdamDataErrAsymmetry(handle)) lel[i] = (unsigned long)errlo[i];
            }
            break;
        }
        case UDA_TYPE_UNSIGNED_LONG64: {
            auto leh = (unsigned long long int *) getIdamDataErrHi(handle);
            auto lel = (unsigned long long int *) getIdamDataErrLo(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) {
                leh[i] = (unsigned long long int) errhi[i];
                if (getIdamDataErrAsymmetry(handle)) lel[i] = (unsigned long long int) errlo[i];
            }
            break;
        }
        case UDA_TYPE_UNSIGNED_SHORT: {
            auto seh = (unsigned short*)getIdamDataErrHi(handle);
            auto sel = (unsigned short*)getIdamDataErrLo(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) {
                seh[i] = (unsigned short)errhi[i];
                if (getIdamDataErrAsymmetry(handle)) sel[i] = (unsigned short)errlo[i];
            }
            break;
        }
        case UDA_TYPE_UNSIGNED_CHAR: {
            auto ceh = (unsigned char*)getIdamDataErrHi(handle);
            auto cel = (unsigned char*)getIdamDataErrLo(handle);
            for (int i = 0; i < getIdamDataNum(handle); i++) {
                ceh[i] = (unsigned char)errhi[i];
                if (getIdamDataErrAsymmetry(handle)) cel[i] = (unsigned char)errlo[i];
            }
            break;
        }

    }

    //--------------------------------------------------------------------------------------------------------------
    // Housekeeping

    free(data);
    free(errhi);
    free(errlo);

    return 0;
}

int generateIdamDimDataError(int handle, int ndim)
{

    int err = 0, asymmetry = 0;

    char* perrlo = nullptr;

    //--------------------------------------------------------------------------------------------------------------
    // Check the handle and model are ok

    if (getIdamData(handle) == nullptr) return 0;            // No Data Block available

    if (ndim < 0 || ndim > getIdamRank(handle)) return 0;

    int model;
    int param_n;
    float params[MAXERRPARAMS];

    getIdamErrorModel(handle, &model, &param_n, params);

    if (model <= ERROR_MODEL_UNKNOWN || model >= ERROR_MODEL_UNDEFINED) return 0;    // No valid Model

    if (getIdamDimNum(handle, ndim) <= 0) return 0;

    //--------------------------------------------------------------------------------------------------------------
    // Allocate local float work arrays and copy the data array to the work array

    if (getIdamDataType(handle) == UDA_TYPE_DCOMPLEX || getIdamDataType(handle) == UDA_TYPE_COMPLEX) {
        err = 999;
        addIdamError(CODEERRORTYPE, "generateIdamDimDataError", err,
                     "Not configured to Generate Complex Type Synthetic Data");
        return 999;
    }

    float* data;
    char* errhi;
    char* errlo;

    if ((data = (float*)malloc(getIdamDimNum(handle, ndim) * sizeof(float))) == nullptr) return 1;
    if ((errhi = (char*)malloc(getIdamDimNum(handle, ndim) * sizeof(float))) == nullptr) return 1;
    if ((errlo = (char*)malloc(getIdamDimNum(handle, ndim) * sizeof(float))) == nullptr) return 1;

    switch (getIdamDataType(handle)) {
        case UDA_TYPE_FLOAT: {
            auto fp = (float*)getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = fp[i];        // Cast all data to Float
            break;
        }
        case UDA_TYPE_DOUBLE: {
            auto dp = (double*)getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float)dp[i];
            break;
        }
        case UDA_TYPE_INT: {
            auto ip = (int*)getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float)ip[i];
            break;
        }
        case UDA_TYPE_LONG: {
            auto lp = (long*)getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float)lp[i];
            break;
        }
        case UDA_TYPE_LONG64: {
            auto lp = (long long int*)getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float)lp[i];
            break;
        }
        case UDA_TYPE_SHORT: {
            auto sp = (short*)getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float)sp[i];
            break;
        }
        case UDA_TYPE_CHAR: {
            char* cp = getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float)cp[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_INT: {
            auto up = (unsigned int*)getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float)up[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_LONG: {
            auto lp = (unsigned long*)getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float)lp[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_LONG64: {
            auto lp = (unsigned long long int *) getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float) lp[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_SHORT: {
            auto sp = (unsigned short*)getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float)sp[i];
            break;
        }
        case UDA_TYPE_UNSIGNED_CHAR: {
            auto cp = (unsigned char*)getIdamDimData(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) data[i] = (float)cp[i];
            break;
        }
        default:
            free(data);
            free(errhi);
            free(errlo);
            return 0;
    }

    //--------------------------------------------------------------------------------------------------------------
    // Generate Model Data

    err = idamErrorModel(model, param_n, params, getIdamDimNum(handle, ndim), data, &asymmetry, (float*)errhi,
                         (float*)errlo);

    if (err != 0) {
        free(data);
        free(errhi);
        free(errlo);
        return err;
    }

    //--------------------------------------------------------------------------------------------------------------
    // Return the Synthetic Data and Error Array

    acc_setIdamDimErrType(handle, ndim, getIdamDimType(handle, ndim));
    acc_setIdamDimErrAsymmetry(handle, ndim, asymmetry);

    if (getIdamDimErrAsymmetry(handle, ndim) && getIdamDimErrLo(handle, ndim) == nullptr) {
        if ((err = allocArray(getIdamDimType(handle, ndim), getIdamDimNum(handle, ndim), &perrlo))) return err;
        acc_setIdamDimErrLo(handle, ndim, perrlo);
    }

    switch (getIdamDimType(handle, ndim)) {
        case UDA_TYPE_FLOAT: {
            auto feh = (float*)getIdamDimErrHi(handle, ndim);
            auto fel = (float*)getIdamDimErrLo(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) {
                feh[i] = (float)errhi[i];
                if (getIdamDimErrAsymmetry(handle, ndim))fel[i] = (float)errlo[i];
            }
            break;
        }
        case UDA_TYPE_DOUBLE: {
            auto deh = (double*)getIdamDimErrHi(handle, ndim);
            auto del = (double*)getIdamDimErrLo(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) {
                deh[i] = (double)errhi[i];
                if (getIdamDimErrAsymmetry(handle, ndim))del[i] = (double)errlo[i];
            }
            break;
        }
        case UDA_TYPE_INT: {
            auto ieh = (int*)getIdamDimErrHi(handle, ndim);
            auto iel = (int*)getIdamDimErrLo(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) {
                ieh[i] = (int)errhi[i];
                if (getIdamDimErrAsymmetry(handle, ndim))iel[i] = (int)errlo[i];
            }
            break;
        }
        case UDA_TYPE_UNSIGNED_INT: {
            auto ueh = (unsigned int*)getIdamDimErrHi(handle, ndim);
            auto uel = (unsigned int*)getIdamDimErrLo(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) {
                ueh[i] = (unsigned int)errhi[i];
                if (getIdamDimErrAsymmetry(handle, ndim))uel[i] = (unsigned int)errlo[i];
            }
            break;
        }
        case UDA_TYPE_LONG: {
            auto leh = (long*)getIdamDimErrHi(handle, ndim);
            auto lel = (long*)getIdamDimErrLo(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) {
                leh[i] = (long)errhi[i];
                if (getIdamDimErrAsymmetry(handle, ndim))lel[i] = (long)errlo[i];
            }
            break;
        }
        case UDA_TYPE_LONG64: {
            auto leh = (long long int*)getIdamDimErrHi(handle, ndim);
            auto lel = (long long int*)getIdamDimErrLo(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) {
                leh[i] = (long long int)errhi[i];
                if (getIdamDimErrAsymmetry(handle, ndim))lel[i] = (long long int)errlo[i];
            }
            break;
        }
        case UDA_TYPE_SHORT: {
            auto seh = (short*)getIdamDimErrHi(handle, ndim);
            auto sel = (short*)getIdamDimErrLo(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) {
                seh[i] = (short)errhi[i];
                if (getIdamDimErrAsymmetry(handle, ndim))sel[i] = (short)errlo[i];
            }
            break;
        }
        case UDA_TYPE_CHAR: {
            char* ceh = (char*)getIdamDimErrHi(handle, ndim);
            char* cel = (char*)getIdamDimErrLo(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) {
                ceh[i] = (char)errhi[i];
                if (getIdamDimErrAsymmetry(handle, ndim))cel[i] = errlo[i];
            }
            break;
        }
        case UDA_TYPE_UNSIGNED_LONG: {
            auto leh = (unsigned long*)getIdamDimErrHi(handle, ndim);
            auto lel = (unsigned long*)getIdamDimErrLo(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) {
                leh[i] = (unsigned long)errhi[i];
                if (getIdamDimErrAsymmetry(handle, ndim))lel[i] = (unsigned long)errlo[i];
            }
            break;
        }
        case UDA_TYPE_UNSIGNED_LONG64: {
            auto leh = (unsigned long long int *) getIdamDimErrHi(handle, ndim);
            auto lel = (unsigned long long int *) getIdamDimErrLo(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) {
                leh[i] = (unsigned long long int) errhi[i];
                if (getIdamDimErrAsymmetry(handle, ndim))lel[i] = (unsigned long long int) errlo[i];
            }
            break;
        }
        case UDA_TYPE_UNSIGNED_SHORT: {
            auto seh = (unsigned short*)getIdamDimErrHi(handle, ndim);
            auto sel = (unsigned short*)getIdamDimErrLo(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) {
                seh[i] = (unsigned short)errhi[i];
                if (getIdamDimErrAsymmetry(handle, ndim))sel[i] = (unsigned short)errlo[i];
            }
            break;
        }
        case UDA_TYPE_UNSIGNED_CHAR: {
            auto ceh = (unsigned char*)getIdamDimErrHi(handle, ndim);
            auto cel = (unsigned char*)getIdamDimErrLo(handle, ndim);
            for (int i = 0; i < getIdamDimNum(handle, ndim); i++) {
                ceh[i] = (unsigned char)errhi[i];
                if (getIdamDimErrAsymmetry(handle, ndim))cel[i] = (unsigned char)errlo[i];
            }
            break;
        }
    }

    //--------------------------------------------------------------------------------------------------------------
    // Housekeeping

    free(data);
    free(errhi);
    free(errlo);

    return 0;
}

