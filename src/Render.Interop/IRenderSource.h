#pragma once

#include "FrameFormat.h"

using namespace System;
using namespace System::Windows;
using namespace System::Windows::Media;

namespace Render {
    namespace Interop {
        public interface class IRenderSource {
            bool CheckFormat(FrameFormat format);

            bool SetupSurface(int videoWidth, int videoHeight, FrameFormat format);

            void Fill(IntPtr yBuffer, UInt32 yStride, IntPtr uBuffer, UInt32 uStride, IntPtr vBuffer, UInt32 vStride);
            
            void Fill(IntPtr yBuffer, UInt32 yStride, IntPtr uBuffer, UInt32 uStride, IntPtr vBuffer, UInt32 vStride, UInt32 videoWidth, UInt32 videoHeight);

            void Draw();

            void Clean();

            property ImageSource^ ImageSource {
                System::Windows::Media::ImageSource^ get();
            }

            property bool IsInitialize {
                bool get();
            }
        };
    }
}