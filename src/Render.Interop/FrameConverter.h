#pragma once

#include <stdint.h>

namespace Interop {
    class FrameConverter
    {
    public:
        FrameConverter();
        ~FrameConverter();

        void* I420ToARGB(void* ptr, unsigned int frameWidth, unsigned int frameHeight);

        void* I420ToARGB(void* yPtr, unsigned int yStride, void* uPtr, unsigned int uStride, void* vPtr, unsigned int vStride, unsigned int frameWidth, unsigned int frameHeight);

        void I420ToARGB(void* ptr, unsigned int frameWidth, unsigned int frameHeight, void* destPtr);

        void I420ToARGB(void* yPtr, unsigned int yStride, void* uPtr, unsigned int uStride, void* vPtr, unsigned int vStride, unsigned int frameWidth, unsigned int frameHeight, void* destPtr);

        void I420ToARGB(void* ptr, unsigned int frameWidth, unsigned int frameHeight, void* destPtr, unsigned int destWidth, unsigned int destHeight, void* tempPtr);

        void I420ToARGB(void* yPtr, unsigned int yStride, void* uPtr, unsigned int uStride, void* vPtr, unsigned int vStride, unsigned int frameWidth, unsigned int frameHeight, void* destPtr, unsigned int destWidth, unsigned int destHeight, void* tempYPtr, unsigned int tempYStride, void* tempUPtr, unsigned int tempUStride, void* tempVPtr, unsigned int tempVStride);

        void I420Scale(void* yPtr, unsigned int yStride, void* uPtr, unsigned int uStride, void* vPtr, unsigned int vStride, unsigned int frameWidth, unsigned int frameHeight, void* destYPtr, unsigned int destYStride, void* destUPtr, unsigned int destUStride, void* destVPtr, unsigned int destVStride, unsigned int destWidth, unsigned int destHeight);

    private:
        void CheckBuffer(int frameWidth, int frameHeight);

    private:
        char* m_buffer = nullptr;
        int m_width = 0;
        int m_height = 0;
    };
}

