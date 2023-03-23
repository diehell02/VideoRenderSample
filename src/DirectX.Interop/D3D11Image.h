#pragma once

#include "stdafx.h"

#include <d3d9.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <d2d1.h>
#include <msclr/lock.h>
#include "RenderMode.h"
#include "RenderFormat.h"
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
            D3D11Image::D3D11Image(IntPtr hwnd,
                RenderMode renderMode, RenderFormat renderFormat);
            D3D11Image(RenderMode renderMode, RenderFormat renderFormat);
            ~D3D11Image();
            !D3D11Image();

            void SetupSurface(int width, int height);
            void WritePixels(IntPtr buffer);
            void WritePixels(
                IntPtr yBuffer, UInt32 yStride,
                IntPtr uBuffer, UInt32 uStride,
                IntPtr vBuffer, UInt32 vStride);
            void WritePixels(HANDLE hSharedHandle);

            static void OnApplicationExit();

        private:
            IntPtr m_backbuffer;
            static HWND m_hwnd;
            IDirect3D9Ex* m_pDirect3D9Ex = nullptr;
            IDirect3DDevice9Ex* m_pDevice9Ex;
            ID3D11Device* m_pD3D11Device;
            ID3D11DeviceContext* m_pD3D11DeviceContext;
            Int32Rect m_imageSourceRect;
            bool m_cleaning = false;
            char* m_tempPtr = nullptr;
            UInt32 m_tempStride;
            int m_width;
            int m_height;
            static LPCWSTR d3dWindowClass = L"D3D11Image";
            bool m_initializeD3DSuccess = false;
            bool m_createResourceSuccess = false;
            int m_stride;
            static bool m_createdHiddenWindow = false;
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

            IDirect3DTexture9* m_pTexture;
            IDirect3DSurface9* m_pSurface;
            IDirect3DSurface9* m_pSurfaceLevel;

            RenderMode m_renderMode;
            RenderFormat m_renderFormat;

            Object^ m_hiddenWindowLock = gcnew Object();

            ID3DImageHelper^ m_d3dImageHelper;

            
            void OnIsFrontBufferAvailableChanged(
                Object^ sender,
                DependencyPropertyChangedEventArgs e);
            void SetImageSourceBackBuffer();
            HRESULT EnsureHWND();
        };
    }
}
