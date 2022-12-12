#include "FrameConverter.h"
#include <libyuv.h>

#include "libyuv.h"

Render::Interop::FrameConverter::FrameConverter()
{
}

Render::Interop::FrameConverter::~FrameConverter()
{
	if (m_buffer)
	{
		delete[] m_buffer;
		m_buffer = nullptr;
	}
}

IntPtr Render::Interop::FrameConverter::I420ToARGB(IntPtr ptr, UInt32 frameWidth, UInt32 frameHeight)
{
	CheckBuffer(frameWidth, frameHeight);
	int strideY = frameWidth;
	int strideU = frameWidth >> 1;
	int strideV = strideU;
	uint8_t* dataY = (uint8_t*)ptr.ToPointer();
	uint8_t* dataU = (uint8_t*)ptr.ToPointer() + (strideY * frameHeight);
	uint8_t* dataV = dataU + (strideU * (frameHeight >> 1));
	libyuv::I420ToARGB(dataY, strideY,
		dataU, strideU,
		dataV, strideV,
		(uint8_t*)m_buffer,
		frameWidth * 4,
		frameWidth, frameHeight);
	return (IntPtr)m_buffer;
}

IntPtr Render::Interop::FrameConverter::I420ToARGB(IntPtr yPtr, UInt32 yStride, IntPtr uPtr, UInt32 uStride, IntPtr vPtr, UInt32 vStride, UInt32 frameWidth, UInt32 frameHeight)
{
	CheckBuffer(frameWidth, frameHeight);
	int strideY = yStride;
	int strideU = uStride;
	int strideV = vStride;
	uint8_t* dataY = (uint8_t*)yPtr.ToPointer();
	uint8_t* dataU = (uint8_t*)uPtr.ToPointer();
	uint8_t* dataV = (uint8_t*)vPtr.ToPointer();
	libyuv::I420ToARGB(dataY, strideY,
		dataU, strideU,
		dataV, strideV,
		(uint8_t*)m_buffer,
		frameWidth * 4,
		frameWidth, frameHeight);
	return (IntPtr)m_buffer;
}

void Render::Interop::FrameConverter::I420ToARGB(IntPtr ptr, UInt32 frameWidth, UInt32 frameHeight, IntPtr destPtr)
{
	int strideY = frameWidth;
	int strideU = frameWidth >> 1;
	int strideV = strideU;
	uint8_t* dataY = (uint8_t*)ptr.ToPointer();
	uint8_t* dataU = (uint8_t*)ptr.ToPointer() + (strideY * frameHeight);
	uint8_t* dataV = dataU + (strideU * (frameHeight >> 1));
	libyuv::I420ToARGB(dataY, strideY,
		dataU, strideU,
		dataV, strideV,
		(uint8_t*)destPtr.ToPointer(),
		frameWidth * 4,
		frameWidth, frameHeight);
}

void Render::Interop::FrameConverter::I420ToARGB(IntPtr yPtr, UInt32 yStride, IntPtr uPtr, UInt32 uStride, IntPtr vPtr, UInt32 vStride, UInt32 frameWidth, UInt32 frameHeight, IntPtr destPtr)
{
	int strideY = yStride;
	int strideU = uStride;
	int strideV = vStride;
	uint8_t* dataY = (uint8_t*)yPtr.ToPointer();
	uint8_t* dataU = (uint8_t*)uPtr.ToPointer();
	uint8_t* dataV = (uint8_t*)vPtr.ToPointer();
	libyuv::I420ToARGB(dataY, strideY,
		dataU, strideU,
		dataV, strideV,
		(uint8_t*)destPtr.ToPointer(),
		frameWidth * 4,
		frameWidth, frameHeight);
}

void Render::Interop::FrameConverter::I420ToARGB(IntPtr ptr, UInt32 frameWidth, UInt32 frameHeight, IntPtr destPtr, UInt32 destWidth, UInt32 destHeight, IntPtr tempPtr)
{
	int strideY = frameWidth;
	int strideU = frameWidth >> 1;
	int strideV = strideU;
	uint8_t* dataY = (uint8_t*)ptr.ToPointer();
	uint8_t* dataU = (uint8_t*)ptr.ToPointer() + (strideY * frameHeight);
	uint8_t* dataV = dataU + (strideU * (frameHeight >> 1));
	int tempStrideY = destWidth;
	int tempStrideU = destWidth >> 1;
	int tempStrideV = tempStrideU;
	uint8_t* tempDataY = (uint8_t*)tempPtr.ToPointer();
	uint8_t* tempDataU = (uint8_t*)tempPtr.ToPointer() + (tempStrideY * destHeight);
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
		(uint8_t*)destPtr.ToPointer(),
		destWidth * 4,
		destWidth, destHeight);
}

void Render::Interop::FrameConverter::I420ToARGB(IntPtr yPtr, UInt32 yStride, IntPtr uPtr, UInt32 uStride, IntPtr vPtr, UInt32 vStride, UInt32 frameWidth, UInt32 frameHeight,
	IntPtr destPtr, UInt32 destWidth, UInt32 destHeight, IntPtr tempYPtr, UInt32 tempYStride, IntPtr tempUPtr, UInt32 tempUStride, IntPtr tempVPtr, UInt32 tempVStride)
{
	int strideY = yStride;
	int strideU = uStride;
	int strideV = vStride;
	uint8_t* dataY = (uint8_t*)yPtr.ToPointer();
	uint8_t* dataU = (uint8_t*)uPtr.ToPointer();
	uint8_t* dataV = (uint8_t*)vPtr.ToPointer();
	int tempStrideY = tempYStride;
	int tempStrideU = tempUStride;
	int tempStrideV = tempVStride;
	uint8_t* tempDataY = (uint8_t*)tempYPtr.ToPointer();
	uint8_t* tempDataU = (uint8_t*)tempUPtr.ToPointer();
	uint8_t* tempDataV = (uint8_t*)tempVPtr.ToPointer();
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
		(uint8_t*)destPtr.ToPointer(),
		destWidth * 4,
		destWidth, destHeight);
}

void Render::Interop::FrameConverter::I420Scale(IntPtr yPtr, UInt32 yStride, IntPtr uPtr, UInt32 uStride, IntPtr vPtr, UInt32 vStride, UInt32 frameWidth, UInt32 frameHeight, IntPtr destYPtr, UInt32 destYStride, IntPtr destUPtr, UInt32 destUStride, IntPtr destVPtr, UInt32 destVStride, UInt32 destWidth, UInt32 destHeight)
{
	int strideY = yStride;
	int strideU = uStride;
	int strideV = vStride;
	uint8_t* dataY = (uint8_t*)yPtr.ToPointer();
	uint8_t* dataU = (uint8_t*)uPtr.ToPointer();
	uint8_t* dataV = (uint8_t*)vPtr.ToPointer();
	int destStrideY = destYStride;
	int destStrideU = destUStride;
	int destStrideV = destVStride;
	uint8_t* destDataY = (uint8_t*)destYPtr.ToPointer();
	uint8_t* destDataU = (uint8_t*)destUPtr.ToPointer();
	uint8_t* destDataV = (uint8_t*)destVPtr.ToPointer();
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

void Render::Interop::FrameConverter::CheckBuffer(int frameWidth, int frameHeight)
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
