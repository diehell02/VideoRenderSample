#include "FrameConverter.h"
#include <libyuv.h>

#include "libyuv.h"

Interop::FrameConverter::FrameConverter()
{
}

Interop::FrameConverter::~FrameConverter()
{
	if (m_buffer)
	{
		delete[] m_buffer;
		m_buffer = nullptr;
	}
}

void* Interop::FrameConverter::I420ToARGB(void* ptr, unsigned int frameWidth, unsigned int frameHeight)
{
	CheckBuffer(frameWidth, frameHeight);
	int strideY = frameWidth;
	int strideU = frameWidth >> 1;
	int strideV = strideU;
	uint8_t* dataY = (uint8_t*)ptr;
	uint8_t* dataU = (uint8_t*)ptr + (strideY * frameHeight);
	uint8_t* dataV = dataU + (strideU * (frameHeight >> 1));
	libyuv::I420ToARGB(dataY, strideY,
		dataU, strideU,
		dataV, strideV,
		(uint8_t*)m_buffer,
		frameWidth * 4,
		frameWidth, frameHeight);
	return m_buffer;
}

void* Interop::FrameConverter::I420ToARGB(void* yPtr, unsigned int yStride, void* uPtr, unsigned int uStride, void* vPtr, unsigned int vStride, unsigned int frameWidth, unsigned int frameHeight)
{
	CheckBuffer(frameWidth, frameHeight);
	int strideY = yStride;
	int strideU = uStride;
	int strideV = vStride;
	uint8_t* dataY = (uint8_t*)yPtr;
	uint8_t* dataU = (uint8_t*)uPtr;
	uint8_t* dataV = (uint8_t*)vPtr;
	libyuv::I420ToARGB(dataY, strideY,
		dataU, strideU,
		dataV, strideV,
		(uint8_t*)m_buffer,
		frameWidth * 4,
		frameWidth, frameHeight);
	return (uint8_t*)m_buffer;
}

void Interop::FrameConverter::I420ToARGB(void* ptr, unsigned int frameWidth, unsigned int frameHeight, void* destPtr)
{
	int strideY = frameWidth;
	int strideU = frameWidth >> 1;
	int strideV = strideU;
	uint8_t* dataY = (uint8_t*)ptr;
	uint8_t* dataU = (uint8_t*)ptr + (strideY * frameHeight);
	uint8_t* dataV = dataU + (strideU * (frameHeight >> 1));
	libyuv::I420ToARGB(dataY, strideY,
		dataU, strideU,
		dataV, strideV,
        (uint8_t*)destPtr,
		frameWidth * 4,
		frameWidth, frameHeight);
}

void Interop::FrameConverter::I420ToARGB(void* yPtr, unsigned int yStride, void* uPtr, unsigned int uStride, void* vPtr, unsigned int vStride, unsigned int frameWidth, unsigned int frameHeight, void* destPtr)
{
	int strideY = yStride;
	int strideU = uStride;
	int strideV = vStride;
	uint8_t* dataY = (uint8_t*)yPtr;
	uint8_t* dataU = (uint8_t*)uPtr;
	uint8_t* dataV = (uint8_t*)vPtr;
	libyuv::I420ToARGB(dataY, strideY,
		dataU, strideU,
		dataV, strideV,
		(uint8_t*)destPtr,
		frameWidth * 4,
		frameWidth, frameHeight);
}

void Interop::FrameConverter::I420ToARGB(void* ptr, unsigned int frameWidth, unsigned int frameHeight, void* destPtr, unsigned int destWidth, unsigned int destHeight, void* tempPtr)
{
	int strideY = frameWidth;
	int strideU = frameWidth >> 1;
	int strideV = strideU;
	uint8_t* dataY = (uint8_t*)ptr;
	uint8_t* dataU = (uint8_t*)ptr + (strideY * frameHeight);
	uint8_t* dataV = dataU + (strideU * (frameHeight >> 1));
	int tempStrideY = destWidth;
	int tempStrideU = destWidth >> 1;
	int tempStrideV = tempStrideU;
	uint8_t* tempDataY = (uint8_t*)tempPtr;
	uint8_t* tempDataU = (uint8_t*)tempPtr + (tempStrideY * destHeight);
	uint8_t* tempDataV = tempDataU + (tempStrideU * (destHeight >> 1));

	libyuv::I420Scale(dataY, strideY,
		dataU, strideU,
		dataV, strideV,
		frameWidth, frameHeight,
		tempDataY, tempStrideY,
		tempDataU, tempStrideU,
		tempDataV, tempStrideV,
		destWidth, destHeight,
		libyuv::FilterMode::kFilterBox);
	libyuv::I420ToARGB(tempDataY, tempStrideY,
		tempDataU, tempStrideU,
		tempDataV, tempStrideV,
		(uint8_t*)destPtr,
		destWidth * 4,
		destWidth, destHeight);
}

void Interop::FrameConverter::I420ToARGB(void* yPtr, unsigned int yStride, void* uPtr, unsigned int uStride, void* vPtr, unsigned int vStride, unsigned int frameWidth, unsigned int frameHeight,
	void* destPtr, unsigned int destWidth, unsigned int destHeight, void* tempYPtr, unsigned int tempYStride, void* tempUPtr, unsigned int tempUStride, void* tempVPtr, unsigned int tempVStride)
{
	int strideY = yStride;
	int strideU = uStride;
	int strideV = vStride;
	uint8_t* dataY = (uint8_t*)yPtr;
	uint8_t* dataU = (uint8_t*)uPtr;
	uint8_t* dataV = (uint8_t*)vPtr;
	int tempStrideY = tempYStride;
	int tempStrideU = tempUStride;
	int tempStrideV = tempVStride;
	uint8_t* tempDataY = (uint8_t*)tempYPtr;
	uint8_t* tempDataU = (uint8_t*)tempUPtr;
	uint8_t* tempDataV = (uint8_t*)tempVPtr;
	libyuv::I420Scale(dataY, strideY,
		dataU, strideU,
		dataV, strideV,
		frameWidth, frameHeight,
		tempDataY, tempStrideY,
		tempDataU, tempStrideU,
		tempDataV, tempStrideV,
		destWidth, destHeight,
		libyuv::FilterMode::kFilterBox);
	libyuv::I420ToARGB(tempDataY, tempStrideY,
		tempDataU, tempStrideU,
		tempDataV, tempStrideV,
		(uint8_t*)destPtr,
		destWidth * 4,
		destWidth, destHeight);
}

void Interop::FrameConverter::I420Scale(void* yPtr, unsigned int yStride, void* uPtr, unsigned int uStride, void* vPtr, unsigned int vStride, unsigned int frameWidth, unsigned int frameHeight, void* destYPtr, unsigned int destYStride, void* destUPtr, unsigned int destUStride, void* destVPtr, unsigned int destVStride, unsigned int destWidth, unsigned int destHeight)
{
	int strideY = yStride;
	int strideU = uStride;
	int strideV = vStride;
	uint8_t* dataY = (uint8_t*)yPtr;
	uint8_t* dataU = (uint8_t*)uPtr;
	uint8_t* dataV = (uint8_t*)vPtr;
	int destStrideY = destYStride;
	int destStrideU = destUStride;
	int destStrideV = destVStride;
	uint8_t* destDataY = (uint8_t*)destYPtr;
	uint8_t* destDataU = (uint8_t*)destUPtr;
	uint8_t* destDataV = (uint8_t*)destVPtr;
	libyuv::I420Scale(dataY, strideY,
		dataU, strideU,
		dataV, strideV,
		frameWidth, frameHeight,
		destDataY, destStrideY,
		destDataU, destStrideU,
		destDataV, destStrideV,
		destWidth, destHeight,
		libyuv::FilterMode::kFilterBox);
}

void Interop::FrameConverter::CheckBuffer(int frameWidth, int frameHeight)
{
	if (m_buffer == nullptr) {
		m_buffer = new char[frameWidth * frameHeight << 2];
	}
	else if (m_width != frameWidth || m_height != frameHeight) {
		delete m_buffer;

		m_buffer = new char[frameWidth * frameHeight << 2];
	}
	m_width = frameWidth;
	m_height = frameHeight;
}
