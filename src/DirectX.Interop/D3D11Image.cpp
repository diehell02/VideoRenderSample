#include "D3D11Image.h"
#include <cstdint>
#include "D3D9Helper.h"
#include "DXGIHelper.h"

namespace Render {
    namespace Interop {
        D3D11Image::D3D11Image(IntPtr hwnd,
            RenderMode renderMode, RenderFormat renderFormat)
        {
            m_hwnd = static_cast<HWND>(hwnd.ToPointer());
            //this->IsFrontBufferAvailableChanged += gcnew System::Windows::DependencyPropertyChangedEventHandler(this, &D3D11Image::OnIsFrontBufferAvailableChanged);

            m_renderMode = renderMode;
            m_renderFormat = renderFormat;

            switch (renderMode)
            {
            case Render::Interop::RenderMode::DX9:
                m_d3dImageHelper = gcnew D3D9Helper(renderFormat);
                break;
            case Render::Interop::RenderMode::DXGI:
                m_d3dImageHelper = gcnew DXGIHelper(renderFormat);
                break;
            default:
                break;
            }
        }

        D3D11Image::D3D11Image(RenderMode renderMode, RenderFormat renderFormat)
        {
            HRESULT hr = EnsureHWND();
            if (hr != S_OK)
            {
                return;
            }
            //this->IsFrontBufferAvailableChanged += gcnew System::Windows::DependencyPropertyChangedEventHandler(this, &D3D11Image::OnIsFrontBufferAvailableChanged);

            m_renderMode = renderMode;
            m_renderFormat = renderFormat;

            switch (renderMode)
            {
            case Render::Interop::RenderMode::DX9:
                m_d3dImageHelper = gcnew D3D9Helper(renderFormat);
                break;
            case Render::Interop::RenderMode::DXGI:
                m_d3dImageHelper = gcnew DXGIHelper(renderFormat);
                break;
            default:
                break;
            }
        }

        D3D11Image::~D3D11Image()
        {
            this->!D3D11Image();
        }

        D3D11Image::!D3D11Image()
        {
            m_d3dImageHelper->CleanupD3D();
        }

        void D3D11Image::SetupSurface(int videoWidth, int videoHeight)
        {
            m_d3dImageHelper->SetupSurface(videoWidth, videoHeight);
        }

        void D3D11Image::WritePixels(IntPtr buffer)
        {
            m_d3dImageHelper->WritePixels(buffer);
        }

        void D3D11Image::WritePixels(
            IntPtr yBuffer,
            UInt32 yStride,
            IntPtr uBuffer,
            UInt32 uStride,
            IntPtr vBuffer,
            UInt32 vStride)
        {
            m_d3dImageHelper->WritePixels(yBuffer, yStride,
                uBuffer, uStride,
                vBuffer, vStride);
        }

        void D3D11Image::WritePixels(HANDLE hSharedHandle)
        {
            m_d3dImageHelper->WritePixels(hSharedHandle);
        }

        void D3D11Image::OnIsFrontBufferAvailableChanged(System::Object^ sender, System::Windows::DependencyPropertyChangedEventArgs e)
        {
            SetImageSourceBackBuffer();
        }

        void D3D11Image::SetImageSourceBackBuffer()
        {
            this->Lock();
            this->SetBackBuffer(D3DResourceType::IDirect3DSurface9,
                m_backbuffer,
                true);
            this->Unlock();
        }

        HRESULT D3D11Image::EnsureHWND()
        {
            HRESULT hr = S_OK;

            lock l(m_hiddenWindowLock);

            if (!m_hwnd)
            {
                WNDCLASS wndclass;

                wndclass.style = CS_HREDRAW | CS_VREDRAW;
                wndclass.lpfnWndProc = DefWindowProc;
                wndclass.cbClsExtra = 0;
                wndclass.cbWndExtra = 0;
                wndclass.hInstance = NULL;
                wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
                wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
                wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
                wndclass.lpszMenuName = NULL;
                wndclass.lpszClassName = d3dWindowClass;

                if (!RegisterClass(&wndclass))
                {
                    return E_FAIL;
                }

                m_hwnd = CreateWindow(d3dWindowClass,
                    TEXT("D3D11Image"),
                    WS_OVERLAPPEDWINDOW,
                    0,                   // Initial X
                    0,                   // Initial Y
                    0,                   // Width
                    0,                   // Height
                    NULL,
                    NULL,
                    NULL,
                    NULL);
                m_createdHiddenWindow = true;
            }

            l.release();
            return hr;
        }

        void D3D11Image::OnApplicationExit()
        {
            if (m_createdHiddenWindow)
            {
                DestroyWindow(m_hwnd);
                UnregisterClass(d3dWindowClass, NULL);
            }
        }
    }
}
