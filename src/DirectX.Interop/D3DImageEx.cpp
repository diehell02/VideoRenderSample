#include "D3DImageEx.h"
#include <cstdint>

namespace Render {
    namespace Interop {
        D3DImageEx::D3DImageEx(IntPtr hwnd, D3DFormat format)
        {
            switch (format)
            {
            case D3DFormat::D3DFMT_A8R8G8B8:
                m_format = D3DFORMAT::D3DFMT_A8R8G8B8;
                break;
            case D3DFormat::D3DFMT_YV12:
                m_format = (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2');
                break;
            default:
                break;
            }
            m_hwnd = static_cast<HWND>(hwnd.ToPointer());
            this->IsFrontBufferAvailableChanged +=
                gcnew System::Windows::DependencyPropertyChangedEventHandler(
                    this, &D3DImageEx::OnIsFrontBufferAvailableChanged);

            AllocResizeBuffer(1920, 1080);
            m_initializeD3DSuccess = InitD3D();
        }

        D3DImageEx::D3DImageEx(D3DFormat format)
        {
            switch (format)
            {
            case D3DFormat::D3DFMT_A8R8G8B8:
                m_format = D3DFORMAT::D3DFMT_A8R8G8B8;
                break;
            case D3DFormat::D3DFMT_YV12:
                m_format = (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2');
                break;
            default:
                break;
            }
            HRESULT hr = EnsureHWND();
            if (hr != S_OK)
            {
                return;
            }
            this->IsFrontBufferAvailableChanged +=
                gcnew System::Windows::DependencyPropertyChangedEventHandler(
                    this, &D3DImageEx::OnIsFrontBufferAvailableChanged);

            AllocResizeBuffer(1920, 1080);
            m_initializeD3DSuccess = InitD3D();
        }

        D3DImageEx::~D3DImageEx()
        {
            this->!D3DImageEx();
        }

        D3DImageEx::!D3DImageEx()
        {
            ReleaseResource();
            SAFE_RELEASE(m_pDevice9Ex);
            m_backbuffer = IntPtr::Zero;

            if (m_hwnd)
            {
                DestroyWindow(m_hwnd);
                UnregisterClass(d3dWindowClass, NULL);
            }
            SAFE_RELEASE(m_pDirect3D9Ex);
        }

        bool D3DImageEx::SetupSurface(int videoWidth, int videoHeight)
        {
            if (m_width == videoWidth && m_height == videoHeight)
            {
                return true;
            }

            m_width = videoWidth;
            m_height = videoHeight;

            if (!m_initializeD3DSuccess)
            {
                return false;
            }

            ReleaseResource();

            m_createResourceSuccess = CreateResource(videoWidth, videoHeight);

            if (!m_createResourceSuccess)
            {
                return false;
            }

            return true;
        }

        void D3DImageEx::WritePixels(IntPtr buffer)
        {
            if (!m_initializeD3DSuccess)
            {
                return;
            }
            if (!m_createResourceSuccess)
            {
                return;
            }
            if (!this->IsFrontBufferAvailable)
            {
                return;
            }
            if (IntPtr::Zero == m_backbuffer) return;
            this->Lock();
            FillD3D9Surface(buffer, m_width, m_height);
            //CreateScene();
            this->AddDirtyRect(m_imageSourceRect);
            this->Unlock();
        }

        void D3DImageEx::WritePixels(IntPtr yBuffer, UInt32 yStride,
            IntPtr uBuffer, UInt32 uStride,
            IntPtr vBuffer, UInt32 vStride)
        {
            if (!m_initializeD3DSuccess)
            {
                return;
            }
            if (!m_createResourceSuccess)
            {
                return;
            }
            if (!this->IsFrontBufferAvailable)
            {
                return;
            }
            if (IntPtr::Zero == m_backbuffer) return;
            this->Lock();
            FillD3D9Surface(yBuffer, yStride, uBuffer, uStride, vBuffer, vStride);
            StretchSurface();
            //CreateScene();
            this->AddDirtyRect(m_imageSourceRect);
            this->Unlock();
        }

        bool D3DImageEx::InitD3D()
        {
            // D3D Device
            pin_ptr<IDirect3D9Ex*> ppDirect3D9Ex = &m_pDirect3D9Ex;
            IFF(Direct3DCreate9Ex(D3D_SDK_VERSION, ppDirect3D9Ex));

            // D3D Present parameters
            D3DPRESENT_PARAMETERS d3dpp;
            ZeroMemory(&d3dpp, sizeof(d3dpp));
            d3dpp.Windowed = TRUE;
            d3dpp.hDeviceWindow = NULL;
            d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
            d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
            //d3dpp.BackBufferCount = 10;

            if (m_createdHiddenWindow)
            {
                d3dpp.BackBufferWidth = 1920;
                d3dpp.BackBufferHeight = 1080;
            }

            // Create Direct3D Device
            pin_ptr<IDirect3DDevice9Ex*> ppDevice9Ex = &m_pDevice9Ex;
            if (nullptr == m_pDirect3D9Ex)
            {
                return false;
            }
            IFF(m_pDirect3D9Ex->CreateDeviceEx(
                D3DADAPTER_DEFAULT,
                D3DDEVTYPE_HAL,
                m_hwnd,
                D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
                &d3dpp,
                NULL,
                ppDevice9Ex));
            if (nullptr == m_pDevice9Ex)
            {
                return false;
            }
            //IFF(m_pDevice9Ex->SetRenderState(D3DRS_LIGHTING, FALSE));
            //IFF(m_pDevice9Ex->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE));
            //IFF(m_pDevice9Ex->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
            //IFF(m_pDevice9Ex->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE));
            //IFF(m_pDevice9Ex->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));

            return true;
        }

        void D3DImageEx::OnIsFrontBufferAvailableChanged(System::Object^ sender, System::Windows::DependencyPropertyChangedEventArgs e)
        {
            SetImageSourceBackBuffer();
        }

        void D3DImageEx::SetImageSourceBackBuffer()
        {
            this->Lock();
            this->SetBackBuffer(D3DResourceType::IDirect3DSurface9, m_backbuffer, true);
            this->Unlock();
        }

        void D3DImageEx::ReleaseResource()
        {
            SAFE_RELEASE(m_pTexture);
            SAFE_RELEASE(m_pSurface);
            SAFE_RELEASE(m_pSurfaceLevel);
        }

        bool D3DImageEx::CreateResource(int width, int height)
        {
            // Create Direct3D Texture
            pin_ptr<IDirect3DTexture9*> ppTexture = &m_pTexture;
            IFF(m_pDevice9Ex->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, ppTexture, NULL));
            if (nullptr == m_pTexture) return false;
            //m_pDevice9Ex->SetTexture(0, m_pTexture);

            // Get Direct3D Surface
            pin_ptr<IDirect3DSurface9*> ppSurfaceLevel = &m_pSurfaceLevel;
            IFF(m_pTexture->GetSurfaceLevel(0, ppSurfaceLevel));
            if (nullptr == m_pSurfaceLevel) return false;
            StretchSurface();
            // Set m_backbuffer for D3DImage
            m_backbuffer = (IntPtr)(void*)m_pSurfaceLevel;
            m_imageSourceRect = Int32Rect(0, 0, width, height);
            SetImageSourceBackBuffer();

            // Create Offscreen Plain Surface
            pin_ptr<IDirect3DSurface9*> ppSurface = &m_pSurface;
            IFF(m_pDevice9Ex->CreateOffscreenPlainSurfaceEx(width, height,
                m_format, D3DPOOL_DEFAULT, ppSurface, nullptr, 0));

            if (nullptr == m_pSurface) return false;

            return true;
        }

        bool D3DImageEx::FillD3D9Surface(IntPtr buffer, int width, int height)
        {
            D3DLOCKED_RECT d3dRect;
            if (nullptr == m_pSurface) return false;
            HRESULT lRet = m_pSurface->LockRect(&d3dRect, nullptr, D3DLOCK_DONOTWAIT);
            if (FAILED(lRet))
            {
                return false;
            }

            //byte* pSrc = (BYTE*)yBuffer.ToPointer();
            byte* pDest = (BYTE*)d3dRect.pBits;

            uint32_t stride = d3dRect.Pitch;
            uint32_t bufferStride = m_width << 2;
            uint32_t w = m_width, h = m_height;

            byte* destPtr = pDest;
            byte* srcPtr = (BYTE*)buffer.ToPointer();

            if (stride == bufferStride)
            {
                memcpy(destPtr, srcPtr, bufferStride * h);
            }
            else
            {
                uint32_t copyLen = min(stride, bufferStride);
                if (w > 0 && h > 0 && stride > 0)
                {
                    for (uint32_t i = 0; i < h; ++i)
                    {
                        memcpy(destPtr, srcPtr, copyLen);
                        destPtr += stride;
                        srcPtr += bufferStride;
                    }
                }
            }

            lRet = m_pSurface->UnlockRect();
            if (FAILED(lRet))
            {
                return false;
            }

            return true;
        }

        bool D3DImageEx::FillD3D9Surface(IntPtr yBuffer, UInt32 yStride, IntPtr uBuffer, UInt32 uStride, IntPtr vBuffer, UInt32 vStride)
        {
            D3DLOCKED_RECT d3dRect;
            HRESULT lRet = m_pSurface->LockRect(&d3dRect, nullptr, D3DLOCK_DONOTWAIT);
            if (FAILED(lRet))
            {
                return false;
            }

            byte* pSrc = (BYTE*)yBuffer.ToPointer();
            byte* pDest = (BYTE*)d3dRect.pBits;

            uint32_t stride = d3dRect.Pitch;
            uint32_t w = m_imageSourceRect.Width, h = m_imageSourceRect.Height;
            uint32_t copyLen = min(stride, w);
            uint32_t ySize = stride * h;
            uint32_t uvSize = stride * h >> 2;
            byte* ySrcPtr = (BYTE*)yBuffer.ToPointer();
            byte* uSrcPtr = (BYTE*)uBuffer.ToPointer();
            byte* vSrcPtr = (BYTE*)vBuffer.ToPointer();
            uint32_t halfHeight = h / 2;
            uint32_t halfWidth = w / 2;
            uint32_t halfStride = stride / 2;
            uint32_t halfCopyLen = copyLen / 2;
            uint32_t tmpVal2 = ySize * 5 / 4;

            byte* yDestPtr = pDest;
            byte* uDestPtr = pDest + tmpVal2;
            byte* vDestPtr = pDest + ySize;

            if (stride == w)
            {
                memcpy(yDestPtr, ySrcPtr, ySize);
                memcpy(uDestPtr, uSrcPtr, uvSize);
                memcpy(vDestPtr, vSrcPtr, uvSize);
            }
            else
            {
                if (w > 0 && h > 0 && stride > 0)
                {
                    for (uint32_t i = 0; i < h; ++i)
                    {
                        memcpy(yDestPtr, ySrcPtr, copyLen);
                        yDestPtr += stride;
                        ySrcPtr += w;
                        if (i < halfHeight)
                        {
                            memcpy(uDestPtr, uSrcPtr, halfCopyLen);
                            uDestPtr += halfStride;
                            uSrcPtr += halfWidth;
                            memcpy(vDestPtr, vSrcPtr, halfCopyLen);
                            vDestPtr += halfStride;
                            vSrcPtr += halfWidth;
                        }
                    }
                }
            }

            lRet = m_pSurface->UnlockRect();
            if (FAILED(lRet))
            {
                return false;
            }

            return true;
        }

        void D3DImageEx::StretchSurface()
        {
            RECT rtVideo = { 0, 0, m_width, m_height };
            if (nullptr == m_pDevice9Ex) return;
            m_pDevice9Ex->StretchRect(m_pSurface, &rtVideo, m_pSurfaceLevel, &rtVideo, D3DTEXF_NONE);
        }

        void D3DImageEx::CreateScene()
        {
            if (nullptr == m_pDevice9Ex) return;
            m_pDevice9Ex->BeginScene();
            m_pDevice9Ex->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 1);
            m_pDevice9Ex->EndScene();
        }

        void D3DImageEx::AllocResizeBuffer(int width, int height)
        {
            m_tempStride = width << 2;
            int tempLength = m_tempStride * (height >> 1);
            if (m_tempPtr != nullptr)
            {
                delete m_tempPtr;
            }
            m_tempPtr = new char[tempLength];
        }

        HRESULT D3DImageEx::EnsureHWND()
        {
            HRESULT hr = S_OK;

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
                    TEXT("D3DImage"),
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
            return hr;
        }
    }
}
