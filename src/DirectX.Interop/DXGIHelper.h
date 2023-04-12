#pragma once

#include "ID3DImageHelper.h"

using namespace System;
using namespace System::Windows;
using namespace System::Threading;

namespace Render {
    namespace Interop {

        ref class DXGIHelper : ID3DImageHelper
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

            bool InitD3D11();
            bool InitD3D10();

        private:
            HRESULT InitRenderTarget(IDXGISurface* pDXGISurface);
            void RenderToDXGI(IntPtr buffer,
                IDXGISurface* pDXGISurface);
            void RenderToDXGI(
                IntPtr yBuffer, UInt32 yStride,
                IntPtr uBuffer, UInt32 uStride,
                IntPtr vBuffer, UInt32 vStride,
                IDXGISurface* pDXGISurface);
            void RenderToDXGI(ID3D11VideoProcessorInputView* pInputView, IDXGISurface* pDXGISurface);
            void WriteToInputView(HANDLE hSharedHandle);
            HRESULT InitVideoProcessor();

        private:
            D3DImage^               m_d3dImage = nullptr;
            IntPtr                  m_backbuffer;
            HWND                    m_hwnd;
            IDirect3D9Ex*           m_pDirect3D9Ex = nullptr;
            IDirect3DDevice9Ex*     m_pDevice9Ex;
            ID3D10Device1*          m_pD3D10Device;
            ID3D11Device*           m_pD3D11Device;
            ID3D11DeviceContext*    m_pD3D11DeviceContext;
            Int32Rect               m_imageSourceRect;
            int                     m_width;
            int                     m_height;
            bool                    m_areSurfacesInitialized = false;
            bool                    m_isD3DInitialized = false;

            ISurfaceQueue*      m_ABQueue;
            ISurfaceQueue*      m_BAQueue;
            ISurfaceConsumer*   m_ABConsumer;
            ISurfaceProducer*   m_BAProducer;
            ISurfaceConsumer*   m_BAConsumer;
            ISurfaceProducer*   m_ABProducer;

            ID2D1RenderTarget*  m_pD2D1RenderTarget = nullptr;
            ID2D1Bitmap*        m_pD2D1Bitmap = nullptr;

            ID3D11VideoDevice*                  m_pDX11VideoDevice = NULL;
            ID3D11VideoContext*                 m_pVideoContext = NULL;
            ID3D11VideoProcessorEnumerator*     m_VideoProcessorEnum = NULL;
            ID3D11VideoProcessor*               m_pVideoProcessor = NULL;
            ID3D11VideoProcessorOutputView*     m_pOutputView = NULL;
            ID3D11VideoProcessorInputView*      m_pInputView = NULL;

            IDXGIKeyedMutex* m_pDXGIMutex = NULL;
            const UINT kSharedResAcquireKey = 0;
            const UINT kSharedResReleaseKey = 0;
            const DWORD kAcquireSyncTimeout = 90;
            bool mAcquiredSharedTexture{ false };

            D3D_DRIVER_TYPE     m_driverType;
            D3D_FEATURE_LEVEL   m_featureLevel;

            Object^ m_sharedHandleLockObj = gcnew Object();

#pragma region ARGB

            char* buffer = nullptr;
            int stride = 0;

#pragma endregion
        };

    }
}
