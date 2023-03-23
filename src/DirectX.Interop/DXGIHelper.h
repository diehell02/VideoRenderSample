#pragma once

#include "ID3DImageHelper.h"

using namespace System;
using namespace System::Windows;

namespace Render {
    namespace Interop {

        ref class DXGIHelper : ID3DImageHelper
        {
        public:
            DXGIHelper(RenderFormat format);
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

            bool InitD3D11();

        private:
            void CreateBitmap(IDXGISurface* pDXGISurface);
            void RenderToDXGI(IntPtr buffer,
                IDXGISurface* pDXGISurface);
            void RenderToDXGI(
                IntPtr yBuffer, UInt32 yStride,
                IntPtr uBuffer, UInt32 uStride,
                IntPtr vBuffer, UInt32 vStride,
                IDXGISurface* pDXGISurface);
            void RenderToDXGI(
                HANDLE hSharedHandle,
                IDXGISurface* pDXGISurface);
            HRESULT CopySurface(
                IUnknown* pDst,
                IUnknown* pSrc,
                UINT width,
                UINT height);


        private:
            D3DImage^ m_d3dImage = nullptr;
            IntPtr m_backbuffer;
            static HWND m_hwnd;
            IDirect3D9Ex* m_pDirect3D9Ex = nullptr;
            IDirect3DDevice9Ex* m_pDevice9Ex;
            ID3D11Device* m_pD3D11Device;
            ID3D11DeviceContext* m_pD3D11DeviceContext;
            Int32Rect m_imageSourceRect;
            int m_width;
            int m_height;
            bool m_areSurfacesInitialized = false;
            bool m_isD3DInitialized = false;

            ISurfaceQueue* m_ABQueue;
            ISurfaceQueue* m_BAQueue;
            ISurfaceConsumer* m_ABConsumer;
            ISurfaceProducer* m_BAProducer;
            ISurfaceConsumer* m_BAConsumer;
            ISurfaceProducer* m_ABProducer;

            ID2D1RenderTarget* m_pD2D1RenderTarget = nullptr;
            ID2D1Bitmap* m_pD2D1Bitmap = nullptr;

            //char* m_buffer = nullptr;
            RenderFormat m_format;
            DXGI_FORMAT m_dxgi_format;

#pragma region YV12
            char* yBuffer = nullptr;
            char* uBuffer = nullptr;
            char* vBuffer = nullptr;
            int yStride = 0;
            int uStride = 0;
            int vStride = 0;

#pragma endregion

#pragma region NV12

            char* uvBuffer = nullptr;
            int uvStride = 0;

#pragma endregion

#pragma region ARGB

            char* buffer = nullptr;
            int stride = 0;

#pragma endregion
        };

    }
}
