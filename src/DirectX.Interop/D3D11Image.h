#pragma once

#include <d3d9.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <d2d1.h>

using namespace System;
using namespace System::Windows;
using namespace System::Windows::Media;
using namespace System::Windows::Interop;

namespace Render {
    namespace Interop {
#define SAFE_RELEASE(punk)  \
              if ((punk) != nullptr)  \
                { (punk)->Release(); (punk) = nullptr; }
#define IFC(hr) \
if (hr != S_OK) return false;
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE)
        typedef struct
        {
            float x, y, z, h;
            D3DCOLOR color;
        }COLORRECTVERTEX;
        struct CUSTOMVERTEX
        {
            FLOAT x, y, z;
            DWORD color;
        };

        public enum class Direct3DSurfaceType {
            Direct3DSurface9 = 0,
            Direct3DSurface11 = 1,
        };

        public ref class D3D11Image : D3DImage
        {
        public:
            D3D11Image::D3D11Image(IntPtr hwnd, Direct3DSurfaceType direct3DSurfaceType);
            D3D11Image(Direct3DSurfaceType direct3DSurfaceType);
            ~D3D11Image();
            !D3D11Image();

            bool SetupSurface(int videoWidth, int videoHeight);

            void WritePixels(IntPtr buffer, UInt32 videoWidth, UInt32 videoHeight);

        private:
            IntPtr m_backbuffer;
            IDirect3D9Ex* m_pDirect3D9Ex = nullptr;
            HWND m_hwnd;
            IDirect3DDevice9Ex* m_pDevice9Ex;
            IDirect3DTexture9* m_pTexture;
            IDirect3DSurface9* m_pSurface;
            IDirect3DSurface9* m_pSurfaceLevel;
            CUSTOMVERTEX* m_pVertex;
            Int32Rect m_imageSourceRect;
            bool m_cleaning = false;
            char* m_tempPtr = nullptr;
            UInt32 m_tempStride;
            int m_width;
            int m_height;
            LPCWSTR d3dWindowClass = L"D3D11Image";
            Direct3DSurfaceType m_direct3DSurfaceType;
            ID3D11Device* m_pD3D11Device = nullptr;
            ID3D11DeviceContext* m_pD3D11DeviceContext = nullptr;
            ID3D11Texture2D* m_pD3D11Texture2D = nullptr;
            ID2D1Factory* m_pD2D1Factory = nullptr;
            ID2D1RenderTarget* m_pD2D1RenderTarget = nullptr;
            ID2D1Bitmap* m_pD2D1Bitmap = nullptr;
            bool m_initializeD3DSuccess = false;
            bool m_createResourceSuccess = false;
            int m_stride;

            bool InitD3D();
            void OnIsFrontBufferAvailableChanged(System::Object^ sender, System::Windows::DependencyPropertyChangedEventArgs e);
            void SetImageSourceBackBuffer();
            void ReleaseResource();
            bool CreateResource(int width, int height);
            bool FillD3D9Surface(IntPtr buffer, int width, int height);
            bool FillD3D11Surface(IntPtr buffer, int width, int height);
            void StretchSurface();
            void CreateScene();
            void AllocResizeBuffer(int width, int height);
            HRESULT EnsureHWND();
        };
    }
}
