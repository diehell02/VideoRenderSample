#pragma once

using namespace System;

namespace Render {
    namespace Interop {
        public ref class FrameConverter
        {
        public:
            FrameConverter();
            ~FrameConverter();

            IntPtr I420ToARGB(IntPtr ptr, UInt32 frameWidth, UInt32 frameHeight);

            IntPtr I420ToARGB(IntPtr yPtr, UInt32 yStride, IntPtr uPtr, UInt32 uStride, IntPtr vPtr, UInt32 vStride, UInt32 frameWidth, UInt32 frameHeight);

            void I420ToARGB(IntPtr ptr, UInt32 frameWidth, UInt32 frameHeight, IntPtr destPtr);

            void I420ToARGB(IntPtr yPtr, UInt32 yStride, IntPtr uPtr, UInt32 uStride, IntPtr vPtr, UInt32 vStride, UInt32 frameWidth, UInt32 frameHeight, IntPtr destPtr);

            void I420ToARGB(IntPtr ptr, UInt32 frameWidth, UInt32 frameHeight, IntPtr destPtr, UInt32 destWidth, UInt32 destHeight, IntPtr tempPtr);

            void I420ToARGB(IntPtr yPtr, UInt32 yStride, IntPtr uPtr, UInt32 uStride, IntPtr vPtr, UInt32 vStride, UInt32 frameWidth, UInt32 frameHeight, IntPtr destPtr, UInt32 destWidth, UInt32 destHeight, IntPtr tempYPtr, UInt32 tempYStride, IntPtr tempUPtr, UInt32 tempUStride, IntPtr tempVPtr, UInt32 tempVStride);

            void I420Scale(IntPtr yPtr, UInt32 yStride, IntPtr uPtr, UInt32 uStride, IntPtr vPtr, UInt32 vStride, UInt32 frameWidth, UInt32 frameHeight, IntPtr destYPtr, UInt32 destYStride, IntPtr destUPtr, UInt32 destUStride, IntPtr destVPtr, UInt32 destVStride, UInt32 destWidth, UInt32 destHeight);

        private:
            void CheckBuffer(int frameWidth, int frameHeight);

        private:
            char* m_buffer = nullptr;
            int m_width = 0;
            int m_height = 0;
        };
    }
}

