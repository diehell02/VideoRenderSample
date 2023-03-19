#pragma once

#include "stdafx.h"

#include <d3d9.h>

using namespace System;
using namespace System::Windows;
using namespace System::Windows::Media;
using namespace System::Windows::Interop;

namespace Render {
    namespace Interop {
        public enum class D3DFormat {
            D3DFMT_A8R8G8B8 = 21,
            D3DFMT_YV12 = 842094169,
        };

        public ref class D3DImageEx : D3DImage
        {
        public:
            D3DImageEx::D3DImageEx(IntPtr hwnd, D3DFormat format);
            D3DImageEx(D3DFormat format);
            ~D3DImageEx();
            !D3DImageEx();

            bool SetupSurface(int videoWidth, int videoHeight);
            void WritePixels(IntPtr buffer);
            void WritePixels(IntPtr yBuffer, UInt32 yStride, IntPtr uBuffer, UInt32 uStride, IntPtr vBuffer, UInt32 vStride);

        private:
            IntPtr m_backbuffer;
            IDirect3D9Ex* m_pDirect3D9Ex = nullptr;
            HWND m_hwnd;
            IDirect3DDevice9Ex* m_pDevice9Ex;
            IDirect3DTexture9* m_pTexture;
            IDirect3DSurface9* m_pSurface;
            IDirect3DSurface9* m_pSurfaceLevel;
            Int32Rect m_imageSourceRect;
            bool m_cleaning = false;
            char* m_tempPtr = nullptr;
            UInt32 m_tempStride;
            int m_width;
            int m_height;
            LPCWSTR d3dWindowClass = L"D3DImage";
            bool m_initializeD3DSuccess = false;
            bool m_createResourceSuccess = false;
            int m_stride;
            bool m_createdHiddenWindow = false;
            D3DFORMAT m_format;

            bool InitD3D();
            void OnIsFrontBufferAvailableChanged(System::Object^ sender, System::Windows::DependencyPropertyChangedEventArgs e);
            void SetImageSourceBackBuffer();
            void ReleaseResource();
            bool CreateResource(int width, int height);
            bool FillD3D9Surface(IntPtr buffer, int width, int height);
            bool FillD3D9Surface(IntPtr yBuffer, UInt32 yStride, IntPtr uBuffer, UInt32 uStride, IntPtr vBuffer, UInt32 vStride);
            void StretchSurface();
            void CreateScene();
            void AllocResizeBuffer(int width, int height);
            HRESULT EnsureHWND();
        };
    }
}
