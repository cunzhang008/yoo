/*============================================================================*/
/*                        NI Vision                                           */
/*----------------------------------------------------------------------------*/
/*    Copyright (c) National Instruments 2016.  All Rights Reserved.          */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Title:       NIVisionExtExports.h                                          */
/*                                                                            */
/*============================================================================*/

#ifndef __NIVISION_INCLUDE_EXT_EXPORTS__
#define __NIVISION_INCLUDE_EXT_EXPORTS__

#include "stdint.h"
#include "cstddef"

#define NI_OCV_SUPPORT

typedef uintptr_t NIImageHandle;
typedef uintptr_t NIArrayHandle;
typedef uintptr_t NIErrorHandle;
typedef int NIERROR;

#define EXTERN_C extern "C"

#define NI_ERR_OCV_USER               999999999
#define NI_ERR_SUCCESS                0
#define NI_ERR_INVALID_IMAGE_TYPE    -1074396080
#define NI_ERR_NULL_POINTER          -1074395269

#include "lv_prolog.h"

template<unsigned ndims, typename datatype>
struct LVArray {
	unsigned dims[ndims];
	datatype data[1];
};
typedef uintptr_t LVArrayPtr;

#include "lv_epilog.h"

#if defined(_MSC_VER)
	#define NI_EXPORT __declspec(dllexport)
	#define NI_IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
	#define NI_EXPORT
	#define NI_IMPORT
#endif

EXTERN_C NIERROR NI_IMPORT NILockImage(NIImageHandle imageHandle);
EXTERN_C NIERROR NI_IMPORT NIUnlockImage(NIImageHandle imageHandle);
EXTERN_C NIERROR NI_IMPORT NIGetImageInfo(NIImageHandle imageHandle, unsigned char **_pixelPtr, int *_type, int *_width, int *_height, size_t *_stepInBytes);
EXTERN_C NIERROR NI_IMPORT NISetImage(NIImageHandle imageHandle, unsigned char *pixelPtr, int width, int height, size_t stepInBytes);
EXTERN_C NIERROR NI_IMPORT NIResizeImage(NIImageHandle imageHandle, int width, int height);

EXTERN_C NIERROR NI_IMPORT NIGetArray1D(NIArrayHandle niArrayHandle, LVArrayPtr lvArrayPtr);
EXTERN_C NIERROR NI_IMPORT NISetArray1D(void *src, void *dest, size_t elemSize, unsigned int count);
EXTERN_C NIERROR NI_IMPORT NISetArray1DFrom2DArray(void *src, void *dest, size_t elemSize, int rows, int cols, size_t stepInBytes);
EXTERN_C NIERROR NI_IMPORT NIResizeArray1D(NIArrayHandle niArrayHandle, size_t elemSize, unsigned int size);

EXTERN_C NIERROR NI_IMPORT NIGetArray2D(NIArrayHandle niArrayHandle, LVArrayPtr lvArrayPtr);
EXTERN_C NIERROR NI_IMPORT NISetArray2D(void *src, void *dest, size_t elemSize, int rows, int cols, size_t stepInBytes);
EXTERN_C NIERROR NI_IMPORT NIResizeArray2D(NIArrayHandle niArrayHandle, size_t elemSize, int rows, int cols);

EXTERN_C NIERROR NI_IMPORT NISetErrorHandleValue(NIErrorHandle niErrorHandle, NIERROR nierror);
EXTERN_C NIERROR NI_IMPORT NIGetErrorHandleValue(NIErrorHandle niErrorHandle);

EXTERN_C NIERROR NI_IMPORT NIInitHandle(NIArrayHandle *handle);

#endif