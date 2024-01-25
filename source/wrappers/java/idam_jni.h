/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class Idam */

#ifndef _Included_Idam
#  define _Included_Idam

#  include "export.h"

#  ifdef __cplusplus
extern "C" {
#  endif
/*
 * Class:     Idam
 * Method:    udaGetAPI
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetAPI(JNIEnv*, jclass, jstring, jstring);

/*
 * Class:     Idam
 * Method:    udaFree
 * Signature: (I)V
 */
LIBRARY_API JNIEXPORT void JNICALL Java_Idam_udaFree(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaFreeAll
 * Signature: ()V
 */
LIBRARY_API JNIEXPORT void JNICALL Java_Idam_udaFreeAll(JNIEnv*, jclass);

/*
 * Class:     Idam
 * Method:    udaSetPrivateFlag
 * Signature: (I)V
 */
LIBRARY_API JNIEXPORT void JNICALL Java_Idam_udaSetPrivateFlag(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaSetClientFlag
 * Signature: (I)V
 */
LIBRARY_API JNIEXPORT void JNICALL Java_Idam_udaSetClientFlag(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaSetProperty
 * Signature: (Ljava/lang/String;)V
 */
LIBRARY_API JNIEXPORT void JNICALL Java_Idam_udaSetProperty(JNIEnv*, jclass, jstring);

/*
 * Class:     Idam
 * Method:    udaGetProperty
 * Signature: (Ljava/lang/String;)I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetProperty(JNIEnv*, jclass, jstring);

/*
 * Class:     Idam
 * Method:    reudaSetPrivateFlag
 * Signature: (I)V
 */
LIBRARY_API JNIEXPORT void JNICALL Java_Idam_reudaSetPrivateFlag(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    reudaSetClientFlag
 * Signature: (I)V
 */
LIBRARY_API JNIEXPORT void JNICALL Java_Idam_reudaSetClientFlag(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    reudaSetProperty
 * Signature: (Ljava/lang/String;)V
 */
LIBRARY_API JNIEXPORT void JNICALL Java_Idam_reudaSetProperty(JNIEnv*, jclass, jstring);

/*
 * Class:     Idam
 * Method:    udaResetProperties
 * Signature: ()V
 */
LIBRARY_API JNIEXPORT void JNICALL Java_Idam_udaResetProperties(JNIEnv*, jclass);

/*
 * Class:     Idam
 * Method:    udaPutServer
 * Signature: (Ljava/lang/String;I)V
 */
LIBRARY_API JNIEXPORT void JNICALL Java_Idam_udaPutServer(JNIEnv*, jclass, jstring, jint);

/*
 * Class:     Idam
 * Method:    udaPutServerHost
 * Signature: (Ljava/lang/String;)V
 */
LIBRARY_API JNIEXPORT void JNICALL Java_Idam_udaPutServerHost(JNIEnv*, jclass, jstring);

/*
 * Class:     Idam
 * Method:    udaPutServerPort
 * Signature: (I)V
 */
LIBRARY_API JNIEXPORT void JNICALL Java_Idam_udaPutServerPort(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaPutServerSocket
 * Signature: (I)V
 */
LIBRARY_API JNIEXPORT void JNICALL Java_Idam_udaPutServerSocket(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaGetServerHost
 * Signature: ()Ljava/lang/String;
 */
LIBRARY_API JNIEXPORT jstring JNICALL Java_Idam_udaGetServerHost(JNIEnv*, jclass);

/*
 * Class:     Idam
 * Method:    udaGetServerPort
 * Signature: ()I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetServerPort(JNIEnv*, jclass);

/*
 * Class:     Idam
 * Method:    udaGetServerSocket
 * Signature: ()I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetServerSocket(JNIEnv*, jclass);

/*
 * Class:     Idam
 * Method:    udaGetClientVersion
 * Signature: ()I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetClientVersion(JNIEnv*, jclass);

/*
 * Class:     Idam
 * Method:    udaGetServerVersion
 * Signature: ()I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetServerVersion(JNIEnv*, jclass);

/*
 * Class:     Idam
 * Method:    udaGetServerErrorCode
 * Signature: ()I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetServerErrorCode(JNIEnv*, jclass);

/*
 * Class:     Idam
 * Method:    udaGetServerErrorMsg
 * Signature: ()Ljava/lang/String;
 */
LIBRARY_API JNIEXPORT jstring JNICALL Java_Idam_udaGetServerErrorMsg(JNIEnv*, jclass);

/*
 * Class:     Idam
 * Method:    udaGetServerErrorStackSize
 * Signature: ()I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetServerErrorStackSize(JNIEnv*, jclass);

/*
 * Class:     Idam
 * Method:    udaGetServerErrorStackRecordType
 * Signature: (I)I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetServerErrorStackRecordType(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaGetServerErrorStackRecordCode
 * Signature: (I)I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetServerErrorStackRecordCode(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaGetServerErrorStackRecordLocation
 * Signature: (I)Ljava/lang/String;
 */
LIBRARY_API JNIEXPORT jstring JNICALL Java_Idam_udaGetServerErrorStackRecordLocation(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaGetServerErrorStackRecordMsg
 * Signature: (I)Ljava/lang/String;
 */
LIBRARY_API JNIEXPORT jstring JNICALL Java_Idam_udaGetServerErrorStackRecordMsg(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaGetErrorCode
 * Signature: (I)I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetErrorCode(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaGetErrorMsg
 * Signature: (I)Ljava/lang/String;
 */
LIBRARY_API JNIEXPORT jstring JNICALL Java_Idam_udaGetErrorMsg(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaGetSignalStatus
 * Signature: (I)I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetSignalStatus(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaGetSourceStatus
 * Signature: (I)I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetSourceStatus(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaGetDataStatus
 * Signature: (I)I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetDataStatus(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaGetLastHandle
 * Signature: (I)I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetLastHandle(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaGetDataNum
 * Signature: (I)I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetDataNum(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaGetRank
 * Signature: (I)I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetRank(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaGetOrder
 * Signature: (I)I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetOrder(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaGetDataType
 * Signature: (I)I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetDataType(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaGetErrorType
 * Signature: (I)I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetErrorType(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaGetDataTypeId
 * Signature: (Ljava/lang/String;)I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetDataTypeId(JNIEnv*, jclass, jstring);

/*
 * Class:     Idam
 * Method:    udaGetDataLabel
 * Signature: (I)Ljava/lang/String;
 */
LIBRARY_API JNIEXPORT jstring JNICALL Java_Idam_udaGetDataLabel(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaGetDataUnits
 * Signature: (I)Ljava/lang/String;
 */
LIBRARY_API JNIEXPORT jstring JNICALL Java_Idam_udaGetDataUnits(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaGetDataDesc
 * Signature: (I)Ljava/lang/String;
 */
LIBRARY_API JNIEXPORT jstring JNICALL Java_Idam_udaGetDataDesc(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaGetFloatData
 * Signature: (I)[F
 */
LIBRARY_API JNIEXPORT jfloatArray JNICALL Java_Idam_udaGetFloatData(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaGetDoubleData
 * Signature: (I)[D
 */
LIBRARY_API JNIEXPORT jdoubleArray JNICALL Java_Idam_udaGetDoubleData(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    castIdamDataToFloat
 * Signature: (I)[F
 */
LIBRARY_API JNIEXPORT jfloatArray JNICALL Java_Idam_castIdamDataToFloat(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    castIdamDataToDouble
 * Signature: (I)[D
 */
LIBRARY_API JNIEXPORT jdoubleArray JNICALL Java_Idam_castIdamDataToDouble(JNIEnv*, jclass, jint);

/*
 * Class:     Idam
 * Method:    udaGetDimNum
 * Signature: (II)I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetDimNum(JNIEnv*, jclass, jint, jint);

/*
 * Class:     Idam
 * Method:    udaGetDimType
 * Signature: (II)I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetDimType(JNIEnv*, jclass, jint, jint);

/*
 * Class:     Idam
 * Method:    udaGetDimErrorType
 * Signature: (II)I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_udaGetDimErrorType(JNIEnv*, jclass, jint, jint);

/*
 * Class:     Idam
 * Method:    udaGetDimLabel
 * Signature: (II)Ljava/lang/String;
 */
LIBRARY_API JNIEXPORT jstring JNICALL Java_Idam_udaGetDimLabel(JNIEnv*, jclass, jint, jint);

/*
 * Class:     Idam
 * Method:    udaGetDimUnits
 * Signature: (II)Ljava/lang/String;
 */
LIBRARY_API JNIEXPORT jstring JNICALL Java_Idam_udaGetDimUnits(JNIEnv*, jclass, jint, jint);

/*
 * Class:     Idam
 * Method:    udaGetFloatDimData
 * Signature: (II)[F
 */
LIBRARY_API JNIEXPORT jfloatArray JNICALL Java_Idam_udaGetFloatDimData(JNIEnv*, jclass, jint, jint);

/*
 * Class:     Idam
 * Method:    udaGetDoubleDimData
 * Signature: (II)[D
 */
LIBRARY_API JNIEXPORT jdoubleArray JNICALL Java_Idam_udaGetDoubleDimData(JNIEnv*, jclass, jint, jint);

/*
 * Class:     Idam
 * Method:    castIdamDimDataToFloat
 * Signature: (II)[F
 */
LIBRARY_API JNIEXPORT jfloatArray JNICALL Java_Idam_castIdamDimDataToFloat(JNIEnv*, jclass, jint, jint);

/*
 * Class:     Idam
 * Method:    castIdamDimDataToDouble
 * Signature: (II)[D
 */
LIBRARY_API JNIEXPORT jdoubleArray JNICALL Java_Idam_castIdamDimDataToDouble(JNIEnv*, jclass, jint, jint);

/*
 * Class:     Idam
 * Method:    getLine
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
LIBRARY_API JNIEXPORT jstring JNICALL Java_Idam_getLine(JNIEnv*, jclass, jstring);

/*
 * Class:     Idam
 * Method:    sumArray1
 * Signature: ([I)I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_sumArray1(JNIEnv*, jclass, jintArray);

/*
 * Class:     Idam
 * Method:    sumArray2
 * Signature: ([I)I
 */
LIBRARY_API JNIEXPORT jint JNICALL Java_Idam_sumArray2(JNIEnv*, jclass, jintArray);

#  ifdef __cplusplus
}
#  endif
#endif
