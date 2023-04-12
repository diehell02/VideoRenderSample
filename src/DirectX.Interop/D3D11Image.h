#pragma once

#include "stdafx.h"

#include <d3d9.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <d2d1.h>
#include <msclr/lock.h>
#include "FrameFormat.h"
#include "ID3DImageHelper.h"

using namespace System;
using namespace System::Windows;
using namespace System::Windows::Media;
using namespace System::Windows::Interop;
using namespace msclr;

namespace Render {
    namespace Interop {
        public ref class D3D11Image : D3DImage
        {
        public:
            D3D11Image::D3D11Image(IntPtr hwnd, FrameFormat frameFormat);
            ~D3D11Image();
            !D3D11Image();

            void SetupSurface(int width, int height);
            void WritePixels(IntPtr buffer);
            void WritePixels(
                IntPtr yBuffer, UInt32 yStride,
                IntPtr uBuffer, UInt32 uStride,
                IntPtr vBuffer, UInt32 vStride);
            void WritePixels(HANDLE hSharedHandle);

        private:
            HWND                m_hwnd;
            ID3DImageHelper^    m_d3dImageHelper;
        };
    }
}
