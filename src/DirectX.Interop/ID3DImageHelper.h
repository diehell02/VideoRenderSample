#pragma once

#include "stdafx.h"

using namespace System;
using namespace System::Windows::Interop;

namespace Render {
    namespace Interop {
        interface class ID3DImageHelper {
        public:
            void WritePixels(IntPtr buffer);
            void WritePixels(
                IntPtr yBuffer, UInt32 yStride,
                IntPtr uBuffer, UInt32 uStride,
                IntPtr vBuffer, UInt32 vStride);
            void WritePixels(HANDLE hSharedHandle);
            bool Initialize();
            bool InitD3D();
            bool InitD3D9();
            bool InitSurfaces();
            void CleanupD3D();
            void CleanupSurfaces();

            void SetD3DImage(D3DImage^ d3dImage);
            void SetHwnd(HWND hwnd);
            void SetupSurface(int width, int height);
        };
    }
}
