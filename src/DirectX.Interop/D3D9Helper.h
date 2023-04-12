#pragma once

#include "ID3DImageHelper.h"
#include <cstdint>

using namespace System;
using namespace System::Windows;

namespace Render {
    namespace Interop {

        ref class D3D9Helper : ID3DImageHelper
        {
        public:
            void WritePixels(IntPtr buffer) override;
            void WritePixels(
                IntPtr yBuffer, UInt32 yStride,
                IntPtr uBuffer, UInt32 uStride,
                IntPtr vBuffer, UInt32 vStride) override;
            void WritePixels(HANDLE hSharedHandle) override;
            bool Initialize() override;
            bool InitD3D() override;
            bool InitD3D9() override;
            bool InitSurfaces() override;
            void CleanupD3D() override;
            void CleanupSurfaces() override;

            void SetD3DImage(D3DImage^ d3dImage) override;
            void SetHwnd(HWND hwnd) override;
            void SetupSurface(int width, int height) override;

        private:
            void FillYV12(uint8_t* src_y, int stride_y,
                uint8_t* src_u, int stride_u,
                uint8_t* src_v, int stride_v,
                uint8_t* dst, int pitch);
            void FillNV12(uint8_t* src_y, int stride_y,
                uint8_t* src_uv, int stride_uv,
                uint8_t* dst, int pitch);
            void FillB8G8R8A8(uint8_t* src, int stride,
                uint8_t* dst, int pitch);

        private:
            D3DImage^               m_d3dImage = nullptr;
            IntPtr                  m_backbuffer;
            HWND                    m_hwnd;
            IDirect3D9Ex*           m_pDirect3D9Ex = nullptr;
            IDirect3DDevice9Ex*     m_pDevice9Ex;
            Int32Rect               m_imageSourceRect;
            int                     m_width;
            int                     m_height;
            bool                    m_areSurfacesInitialized = false;
            bool                    m_isD3DInitialized = false;

            IDirect3DTexture9*      m_pTexture;
            IDirect3DSurface9*      m_pSurfaceYV12;
            IDirect3DSurface9*      m_pSurfaceRGBA;
            IDirect3DSurface9*      m_pSurfaceLevel;
        };

    }
}
