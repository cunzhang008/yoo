/*============================================================================*/
/*                        NI Vision                                           */
/*----------------------------------------------------------------------------*/
/*    Copyright (c) National Instruments 2016.  All Rights Reserved.          */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Title:       NIVisionExtLib.h                                              */
/*                                                                            */
/*============================================================================*/

#ifndef __NIVISION_INCLUDE_EXT_LIB__
#define __NIVISION_INCLUDE_EXT_LIB__

#include "NIVisionExtExports.h"
#include <algorithm>
#include <vector>

#ifdef NI_OCV_SUPPORT
#include "opencv2/core/core.hpp"
using namespace cv;
#endif

using namespace std;

#define NIImage_U8          0
#define NIImage_I16         1
#define NIImage_SGL         2
#define NIImage_COMPLEX     3
#define NIImage_RGB32       4
#define NIImage_HSL32       5
#define NIImage_RGB_U64     6
#define NIImage_U16         7

#define ReturnOnError(niError)                                  \
{                                                               \
	if (niError != NI_ERR_SUCCESS)                              \
		return niError;                                         \
}                                                               \

#define ReturnOnPreviousError(niErrorHandle)                    \
{                                                               \
	NIERROR status = NIGetErrorHandleValue(niErrorHandle);      \
	if (status != NI_ERR_SUCCESS)                               \
		return;                                                 \
}                                                               \

inline NIERROR ProcessNIError(NIERROR niError, NIErrorHandle niErrorHandle)
{
	NIERROR error = NISetErrorHandleValue(niErrorHandle, niError);
	return error;
}

inline void ThrowNIError(NIERROR nierror)
{
	if (nierror != NI_ERR_SUCCESS)
		throw nierror;
}

class NIImage
{
	NIImageHandle niImageHandle;
	NIERROR lockErrorStatus;

public:
	unsigned char *pixelPtr;
	int type;
	int width;
	int height;
	size_t stepInBytes;

	NIImage()
	{
		pixelPtr = NULL;
		type = -1;
		width = -1;
		height = -1;
		stepInBytes = 0;
		lockErrorStatus = -1;
		niImageHandle = 0;
	}

	NIImage(NIImageHandle _niImageHandle)
	{
		lockErrorStatus = -1;
		ThrowNIError(SetImageHandle(_niImageHandle));
	}

	NIImage(NIImageHandle _niImageHandle, int _width, int _height)
	{
		lockErrorStatus = -1;
		ThrowNIError(SetImageHandle(_niImageHandle));
		ThrowNIError(ResizeImage(_width, _height));
	}

	NIERROR SetImageHandle(NIImageHandle _niImageHandle)
	{
		NIERROR err = NI_ERR_SUCCESS;

		if (lockErrorStatus == NI_ERR_SUCCESS)
		{
			NIUnlockImage(niImageHandle);
			lockErrorStatus = -1;
		}

		try
		{
			lockErrorStatus = NILockImage(_niImageHandle);
			if (lockErrorStatus == NI_ERR_SUCCESS)
			{
				niImageHandle = _niImageHandle;
				ThrowNIError(UpdateMembers());
			}
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		return err;
	}

	bool IsValid()
	{
		if (lockErrorStatus == NI_ERR_SUCCESS)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	NIERROR UpdateMembers()
	{
		ReturnOnError(lockErrorStatus);
		NIERROR err = NI_ERR_SUCCESS;

		try
		{
			ThrowNIError(NIGetImageInfo(niImageHandle, &pixelPtr, &type, &width, &height, &stepInBytes));
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		return err;
	}

	NIERROR ResizeImage(int _width, int _height)
	{
		ReturnOnError(lockErrorStatus);
		NIERROR err = NI_ERR_SUCCESS;

		try
		{
			ThrowNIError(NIResizeImage(niImageHandle, width, height));
			ThrowNIError(UpdateMembers());
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		return err;
	}

	NIERROR SetImage(unsigned char *_data, int _width, int _height, int _stepInBytes)
	{
		ReturnOnError(lockErrorStatus);
		NIERROR err = NI_ERR_SUCCESS;

		try
		{
			ThrowNIError(NISetImage(niImageHandle, _data, _width, _height, _stepInBytes));
			ThrowNIError(UpdateMembers());
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		return err;
	}

#ifdef NI_OCV_SUPPORT

	//  _____________________________
	// |  VDM Type   |  Opencv Type  |
	// |_____________|_______________|
	// |     U8      |	  CV_8UC1    |
	// |_____________|_______________|
	// |_____________________________|
	// |     I16     |	  CV_16SC1   |
	// |_____________|_______________|
	// |_____________________________|
	// |     SGL     |    CV_32FC1   |
	// |_____________|_______________|
	// |_____________________________|
	// |   COMPLEX   |    CV_32FC2   |
	// |_____________|_______________|
	// |_____________________________|
	// |   RGB32     |    CV_8UC4    |
	// |_____________|_______________|
	// |_____________________________|
	// |   HSL32     |	  CV_8UC4    |
	// |_____________|_______________|
	// |_____________________________|
	// |   RGB_U64	 |	  CV_16UC4	 |
	// |_____________|_______________|
	// |_____________________________|
	// |    U16      |    CV_16UC1   |
	// |_____________|_______________|

	int NIImgToMatType(int niImgType)
	{
		int typeOfImage[] = { CV_8UC1, CV_16SC1, CV_32FC1, CV_32FC2, CV_8UC4, CV_8UC4, CV_16UC4, CV_16UC1 };
		return typeOfImage[niImgType];
	}

	int GetMatImageType()
	{
		int matType = NIImgToMatType(type);
		return matType;
	}

	int GetNIImageType()
	{
		return type;
	}

	NIERROR MatToImage(Mat &imgMat)
	{
		ReturnOnError(lockErrorStatus);
		NIERROR err = NI_ERR_SUCCESS;

		unsigned char *imgMatData = (unsigned char*)(imgMat.data);
		int rows = imgMat.rows;
		int cols = imgMat.cols;
		size_t matStepInBytes = imgMat.step;

		int niImgMatType = NIImgToMatType(type);
		try
		{
			if (imgMat.type() != niImgMatType)
				ThrowNIError(NI_ERR_INVALID_IMAGE_TYPE);

			ThrowNIError(SetImage(imgMatData, cols, rows, static_cast<int>(matStepInBytes)));
			ThrowNIError(UpdateMembers());
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		return err;
	}

	NIERROR ImageToMat(Mat &imgMat)
	{
		ReturnOnError(lockErrorStatus);
		NIERROR err = NI_ERR_SUCCESS;
		int niImgMatType = NIImgToMatType(type);

		try
		{
			Mat localMat(height, width, niImgMatType, pixelPtr, stepInBytes);
			imgMat = localMat;
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		catch (...)
		{
			throw;
		}
		return err;
	}

	NIERROR CloneToMat(Mat &imgMat)
	{
		ReturnOnError(lockErrorStatus);
		NIERROR err = NI_ERR_SUCCESS;
		int niImgMatType = NIImgToMatType(type);

		try
		{
			Mat localMat(height, width, niImgMatType, pixelPtr, stepInBytes);
			imgMat = localMat.clone();
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		catch (...)
		{
			throw;
		}
		return err;
	}
#endif

	~NIImage()
	{
		if (lockErrorStatus == NI_ERR_SUCCESS)
			NIUnlockImage(niImageHandle);
	}
};

template < class ArrayType >
class NIArray
{
protected:
	NIArrayHandle niArrayHandle;

	NIERROR SetHandle(NIArrayHandle& _niArrayHandle, bool initHandle)
	{
		NIERROR err = NI_ERR_SUCCESS;
		try
		{
			if (initHandle == true || _niArrayHandle == 0)
			{
				ThrowNIError(NIInitHandle(&_niArrayHandle));
			}
			niArrayHandle = _niArrayHandle;
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		return err;
	}

public:
	ArrayType *data;
	size_t elemSize;

	NIArray()
	{
		niArrayHandle = 0;
		data = NULL;
		elemSize = sizeof(ArrayType);
	}

	virtual NIERROR UpdateMembers() = 0;
#ifdef NI_OCV_SUPPORT
	virtual NIERROR ToMat(Mat &mat) = 0;
#endif
};

template < class ArrayType >
class NIArray1D : public NIArray<ArrayType>
{
private:

	NIERROR SetDataPtrAndSize(LVArrayPtr lvArrayPtr)
	{
		NIERROR err = NI_ERR_SUCCESS;
		try
		{
			if (!lvArrayPtr)
				ThrowNIError(NI_ERR_NULL_POINTER);

			data = (reinterpret_cast<LVArray<1, ArrayType>*>(lvArrayPtr))->data;
			size = (reinterpret_cast<LVArray<1, ArrayType>*>(lvArrayPtr))->dims[0];
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		return err;
	}

public:
	unsigned int size;

	using NIArray<ArrayType>::niArrayHandle;
	using NIArray<ArrayType>::data;
	using NIArray<ArrayType>::elemSize;
	using NIArray<ArrayType>::SetHandle;

	NIArray1D() :NIArray<ArrayType>()
	{
		size = 0;
	}

	NIArray1D(NIArrayHandle& _niArrayHandle, bool initHandle=false)
	{
		ThrowNIError(SetArrayHandle(_niArrayHandle, initHandle));
	}

	NIArray1D(NIArrayHandle& _niArrayHandle, int _size, bool initHandle=false)
	{
		ThrowNIError(SetArrayHandle(_niArrayHandle, initHandle));
		ThrowNIError(Resize(_size));
	}

	NIArray1D(NIArrayHandle& _niArrayHandle, size_t _size, bool initHandle=false)
	{
		ThrowNIError(SetArrayHandle(_niArrayHandle, initHandle));
		ThrowNIError(Resize(_size));
	}

	NIERROR SetArrayHandle(NIArrayHandle& _niArrayHandle, bool initHandle=false)
	{ 
		NIERROR err = NI_ERR_SUCCESS;
		try
		{
			ThrowNIError(SetHandle(_niArrayHandle, initHandle));
			ThrowNIError(UpdateMembers());
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		return err;
	}

	NIERROR UpdateMembers()
	{
		NIERROR err = NI_ERR_SUCCESS;
		try
		{
			LVArrayPtr lvArrayPtr;
			ThrowNIError(NIGetArray1D(niArrayHandle, (LVArrayPtr) &lvArrayPtr));
			ThrowNIError(SetDataPtrAndSize(lvArrayPtr));
			elemSize = sizeof(ArrayType);
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		return err;
	}

	NIERROR Resize(unsigned int _size)
	{
		NIERROR err = NI_ERR_SUCCESS;
		try
		{
			ThrowNIError(NIResizeArray1D(niArrayHandle, elemSize, _size));
			ThrowNIError(UpdateMembers());
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		return err;
	}

	NIERROR SetArray(ArrayType *_data, unsigned int _size)
	{
		NIERROR err = NI_ERR_SUCCESS;
		try
		{
			ThrowNIError(Resize(_size));
			ThrowNIError(NISetArray1D((void *) _data, (void *) data, elemSize, _size));
			ThrowNIError(UpdateMembers());
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		return err;
	}

	NIERROR SetArray(ArrayType *_data, int rows, int cols, size_t _stepInBytes)
	{
		NIERROR err = NI_ERR_SUCCESS;
		try
		{
			ThrowNIError(Resize(rows*cols));
			ThrowNIError(NISetArray1DFrom2DArray((void *) _data, (void *) data, elemSize, rows, cols, _stepInBytes));
			ThrowNIError(UpdateMembers());
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		return err;
	}

	NIERROR SetArray(vector<ArrayType> &vec)
	{
		NIERROR err = NI_ERR_SUCCESS;
		try
		{
			ArrayType *vectorPtr = vec.data();
			unsigned int vectorSize = static_cast<unsigned int>(vec.size());

			ThrowNIError(SetArray(vectorPtr, vectorSize));
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		catch (...)
		{
			throw;
		}
		return err;
	}

	NIERROR ToVector(vector<ArrayType> &vec)
	{
		NIERROR err = NI_ERR_SUCCESS;
		try
		{
			vec.resize(size);
			vec.assign(data, data + size);
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		catch (...)
		{
			throw;
		}
		return err;
	}

#ifdef NI_OCV_SUPPORT
	NIERROR SetArray(Mat &mat)
	{
		NIERROR err = NI_ERR_SUCCESS;
		try
		{
			ArrayType *dataPtr = (ArrayType*) mat.data;
			int tRows = mat.rows;
			int tCols = mat.cols;
			int tStepInBytes = mat.step;

			ThrowNIError(SetArray(dataPtr, tRows, tCols, tStepInBytes));
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		catch (...)
		{
			throw;
		}
		return err;
	}

	NIERROR ToMat(Mat &mat)
	{
		NIERROR err = NI_ERR_SUCCESS;
		try
		{
			Mat_<ArrayType> localMat(1, size, data);
			mat = localMat.clone();
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		catch (...)
		{
			throw;
		}
		return err;
	}
#endif
};

template < class ArrayType >
class NIArray2D : public NIArray<ArrayType>
{
private:
	NIERROR SetDataPtrAndSize(LVArrayPtr lvArrayPtr)
	{
		NIERROR err = NI_ERR_SUCCESS;
		try
		{
			if (!lvArrayPtr)
				ThrowNIError(NI_ERR_NULL_POINTER);

			data = (reinterpret_cast<LVArray<2, ArrayType>*>(lvArrayPtr))->data;
			rows = (reinterpret_cast<LVArray<2, ArrayType>*>(lvArrayPtr))->dims[0];
			cols = (reinterpret_cast<LVArray<2, ArrayType>*>(lvArrayPtr))->dims[1];
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		return err;
	}

public:
	int rows, cols;

	using NIArray<ArrayType>::niArrayHandle;
	using NIArray<ArrayType>::data;
	using NIArray<ArrayType>::elemSize;
	using NIArray<ArrayType>::SetHandle;

	NIArray2D() :NIArray<ArrayType>()
	{
		rows = 0;
		cols = 0;
	}

	NIArray2D(NIArrayHandle& _niArrayHandle, bool initHandle=false)
	{
		ThrowNIError(SetArrayHandle(_niArrayHandle, initHandle));
	}

	NIArray2D(NIArrayHandle& _niArrayHandle, int _rows, int _cols, bool initHandle=false)
	{
		ThrowNIError(SetArrayHandle(_niArrayHandle, initHandle));
		ThrowNIError(Resize(_rows, _cols));
	}

	NIArray2D(NIArrayHandle& _niArrayHandle, size_t _rows, size_t _cols, bool initHandle=false)
	{
		ThrowNIError(SetArrayHandle(_niArrayHandle, initHandle));
		ThrowNIError(Resize(_rows, _cols));
	}

	NIERROR SetArrayHandle(NIArrayHandle& _niArrayHandle, bool initHandle=false)
	{
		NIERROR err = NI_ERR_SUCCESS;
		try
		{
			ThrowNIError(SetHandle(_niArrayHandle, initHandle));
			ThrowNIError(UpdateMembers());
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		return err;
	}

	NIERROR UpdateMembers()
	{
		NIERROR err = NI_ERR_SUCCESS;
		try
		{
			LVArrayPtr lvArrayPtr;
			ThrowNIError(NIGetArray2D(niArrayHandle, (LVArrayPtr) &lvArrayPtr));
			ThrowNIError(SetDataPtrAndSize(lvArrayPtr));
			elemSize = sizeof(ArrayType);
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		return err;
	}

	NIERROR Resize(int _rows, int _cols)
	{
		NIERROR err = NI_ERR_SUCCESS;
		try
		{
			ThrowNIError(NIResizeArray2D(niArrayHandle, elemSize, _rows, _cols));
			ThrowNIError(UpdateMembers());
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		return err;
	}

	NIERROR SetArray(ArrayType *_data, int _rows, int _cols, size_t _stepInBytes)
	{
		NIERROR err = NI_ERR_SUCCESS;
		try
		{
			ThrowNIError(Resize(_rows, _cols));
			ThrowNIError(NISetArray2D((void*) _data, (void *) data, elemSize, _rows, _cols, _stepInBytes));
			ThrowNIError(UpdateMembers());
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		return err;
	}

#ifdef NI_OCV_SUPPORT
	NIERROR SetArray(Mat &mat)
	{
		NIERROR err = NI_ERR_SUCCESS;

		ArrayType *matPtr = (ArrayType*) mat.data;
		int matRows = mat.rows;
		int matCols = mat.cols;
		size_t stepInBytes = mat.step;

		try
		{
			ThrowNIError(SetArray(matPtr, matRows, matCols, stepInBytes));
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		return err;
	}

	NIERROR ToMat(Mat &mat)
	{
		NIERROR err = NI_ERR_SUCCESS;
		try
		{
			Mat_<ArrayType> localMat(rows, cols, data);
			mat = localMat.clone();
		}
		catch (NIERROR _err)
		{
			err = _err;
		}
		catch (...)
		{
			throw;
		}

		return err;
	}
#endif
};

#endif