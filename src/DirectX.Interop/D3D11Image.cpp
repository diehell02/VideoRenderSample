#include "D3D11Image.h"
#include <cstdint>

namespace Render {
    namespace Interop {
        D3D11Image::D3D11Image(IntPtr hwnd, Direct3DSurfaceType direct3DSurfaceType,
            D3DFormat format)
        {
            m_direct3DSurfaceType = direct3DSurfaceType;
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
            this->IsFrontBufferAvailableChanged += gcnew System::Windows::DependencyPropertyChangedEventHandler(this, &D3D11Image::OnIsFrontBufferAvailableChanged);

            AllocResizeBuffer(1920, 1080);
            m_initializeD3DSuccess = InitD3D();
        }

        D3D11Image::D3D11Image(Direct3DSurfaceType direct3DSurfaceType,
            D3DFormat format)
        {
            m_direct3DSurfaceType = direct3DSurfaceType;
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
            this->IsFrontBufferAvailableChanged += gcnew System::Windows::DependencyPropertyChangedEventHandler(this, &D3D11Image::OnIsFrontBufferAvailableChanged);

            AllocResizeBuffer(1920, 1080);
            m_initializeD3DSuccess = InitD3D();
        }

        D3D11Image::~D3D11Image()
        {
            this->!D3D11Image();
        }

        D3D11Image::!D3D11Image()
        {
            ReleaseResource();
            SAFE_RELEASE(m_pDevice9Ex);
            SAFE_RELEASE(m_pD3D11Device);
            SAFE_RELEASE(m_pD3D11DeviceContext);
            m_backbuffer = IntPtr::Zero;

            if (m_hwnd)
            {
                DestroyWindow(m_hwnd);
                UnregisterClass(d3dWindowClass, NULL);
            }
            SAFE_RELEASE(m_pDirect3D9Ex);
        }

        bool D3D11Image::SetupSurface(int videoWidth, int videoHeight)
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

        void D3D11Image::WritePixels(IntPtr buffer)
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
            switch (m_direct3DSurfaceType)
            {
            case Direct3DSurfaceType::Direct3DSurface9:
                this->Lock();
                FillD3D9Surface(buffer, m_width, m_height);
                StretchSurface();
                //CreateScene();
                this->AddDirtyRect(m_imageSourceRect);
                this->Unlock();
                break;
            case Direct3DSurfaceType::Direct3DSurface11:
                this->Lock();
                FillD3D11Surface(buffer, m_width, m_height);
                //CreateScene();
                this->AddDirtyRect(m_imageSourceRect);
                this->Unlock();
                break;
            default:
                break;
            }
        }

        void D3D11Image::WritePixels(IntPtr yBuffer, UInt32 yStride, IntPtr uBuffer, UInt32 uStride, IntPtr vBuffer, UInt32 vStride)
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

        bool D3D11Image::InitD3D()
        {
            // D3D Device
            pin_ptr<IDirect3D9Ex*> ppDirect3D9Ex = &m_pDirect3D9Ex;
            IFC(Direct3DCreate9Ex(D3D_SDK_VERSION, ppDirect3D9Ex));

            // D3D Present parameters
            D3DPRESENT_PARAMETERS d3dpp;
            ZeroMemory(&d3dpp, sizeof(d3dpp));
            d3dpp.Windowed = TRUE;
            d3dpp.hDeviceWindow = NULL;
            d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
            d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
            d3dpp.BackBufferCount = 10;

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
            DWORD behaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE;
            auto h = m_pDirect3D9Ex->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                m_hwnd, behaviorFlags, &d3dpp, NULL, ppDevice9Ex);
            if (nullptr == m_pDevice9Ex)
            {
                return false;
            }
            //IFC(m_pDevice9Ex->SetRenderState(D3DRS_LIGHTING, FALSE));
            //IFC(m_pDevice9Ex->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE));
            //IFC(m_pDevice9Ex->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
            //IFC(m_pDevice9Ex->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE));
            //IFC(m_pDevice9Ex->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));

            if (m_direct3DSurfaceType == Direct3DSurfaceType::Direct3DSurface9)
            {
                return true;
            } 

            // D3D11 Device
            D3D_FEATURE_LEVEL featureLevels[] = {
                D3D_FEATURE_LEVEL_11_1
            };
            UINT numFeatureLevels = ARRAYSIZE(featureLevels);
            pin_ptr<ID3D11Device*> ppD3D11Device = &m_pD3D11Device;
            pin_ptr<ID3D11DeviceContext*> ppD3D11DeviceContext = &m_pD3D11DeviceContext;
            IFC(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
                D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                featureLevels, numFeatureLevels, D3D11_SDK_VERSION, ppD3D11Device,
                NULL, ppD3D11DeviceContext));

            return true;
        }

        void D3D11Image::OnIsFrontBufferAvailableChanged(System::Object^ sender, System::Windows::DependencyPropertyChangedEventArgs e)
        {
            SetImageSourceBackBuffer();
        }

        void D3D11Image::SetImageSourceBackBuffer()
        {
            this->Lock();
            this->SetBackBuffer(D3DResourceType::IDirect3DSurface9, m_backbuffer, true);
            this->Unlock();
        }

        void D3D11Image::ReleaseResource()
        {
            SAFE_RELEASE(m_pTexture);
            SAFE_RELEASE(m_pSurface);
            SAFE_RELEASE(m_pSurfaceLevel);
            SAFE_RELEASE(m_pD3D11Texture2D);
            SAFE_RELEASE(m_pD2D1Factory);
            SAFE_RELEASE(m_pD2D1RenderTarget);
            SAFE_RELEASE(m_pD2D1Bitmap);
        }

        bool D3D11Image::CreateResource(int width, int height)
        {
            HANDLE sharedHandle = nullptr;
            HANDLE* pSharedHandle = &sharedHandle;
            if (m_direct3DSurfaceType == Direct3DSurfaceType::Direct3DSurface11)
            {
                D3D11_TEXTURE2D_DESC desc =
                    CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_B8G8R8A8_UNORM, width, height);
                desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
                desc.MipLevels = 1;
                desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
                pin_ptr<ID3D11Texture2D*> ppD3D11Texture2D = &m_pD3D11Texture2D;
                IFC(m_pD3D11Device->CreateTexture2D(&desc, NULL, ppD3D11Texture2D));

                IDXGISurface* pDXGISurface = nullptr;
                pin_ptr<IDXGISurface*> ppDXGISurface = &pDXGISurface;
                IFC(m_pD3D11Texture2D->QueryInterface(
                    __uuidof(IDXGISurface), (void**)ppDXGISurface));
                ID2D1Factory* pD2D1Factory = NULL;
                IFC(D2D1CreateFactory(
                    D2D1_FACTORY_TYPE::D2D1_FACTORY_TYPE_SINGLE_THREADED,
                    &pD2D1Factory));
                m_pD2D1Factory = pD2D1Factory;
                D2D1_PIXEL_FORMAT pixelFormat{};
                pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
                pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
                D2D1_RENDER_TARGET_PROPERTIES renderTargetProperties{};
                renderTargetProperties.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
                renderTargetProperties.pixelFormat = pixelFormat;
                renderTargetProperties.dpiX = 96;
                renderTargetProperties.dpiY = 96;
                renderTargetProperties.usage = D2D1_RENDER_TARGET_USAGE_NONE;
                renderTargetProperties.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
                pin_ptr<ID2D1RenderTarget*> ppD2D1RenderTarget = &m_pD2D1RenderTarget;
                IFC(m_pD2D1Factory->CreateDxgiSurfaceRenderTarget(pDXGISurface,
                    &renderTargetProperties, ppD2D1RenderTarget));

                IDXGIResource1* pDXGIResource1 = nullptr;
                pin_ptr<IDXGIResource1*> ppDXGIResource1 = &pDXGIResource1;
                IFC(m_pD3D11Texture2D->QueryInterface(
                    __uuidof(IDXGIResource1), (void**)ppDXGIResource1));
                *pSharedHandle = NULL;
                IFC(pDXGIResource1->GetSharedHandle(pSharedHandle));

                D3D11_VIEWPORT viewPort{};
                viewPort.TopLeftX = 0;
                viewPort.TopLeftY = 0;
                viewPort.Width = (FLOAT)width;
                viewPort.Height = (FLOAT)height;
                viewPort.MinDepth = 0;
                viewPort.MaxDepth = 1;
                m_pD3D11DeviceContext->RSSetViewports(1, &viewPort);

                FLOAT dpiX, dpiY;
                m_pD2D1RenderTarget->GetDpi(&dpiX, &dpiY);
                D2D1_BITMAP_PROPERTIES properties{};
                properties.pixelFormat = {
                    DXGI_FORMAT_B8G8R8A8_UNORM,
                    D2D1_ALPHA_MODE_PREMULTIPLIED
                };
                properties.dpiX = dpiX;
                properties.dpiY = dpiY;
                pin_ptr<ID2D1Bitmap*> ppD2D1Bitmap = &m_pD2D1Bitmap;
                D2D1_SIZE_U size = { (UINT32)width, (UINT32)height };
                m_pD2D1RenderTarget->CreateBitmap(size, properties, ppD2D1Bitmap);
            }

            // Create Direct3D Texture
            pin_ptr<IDirect3DTexture9*> ppTexture = &m_pTexture;
            IFC(m_pDevice9Ex->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET,
                D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, ppTexture, pSharedHandle));
            if (nullptr == m_pTexture) return false;
            //m_pDevice9Ex->SetTexture(0, m_pTexture);

            // Get Direct3D Surface
            pin_ptr<IDirect3DSurface9*> ppSurfaceLevel = &m_pSurfaceLevel;
            IFC(m_pTexture->GetSurfaceLevel(0, ppSurfaceLevel));
            if (nullptr == m_pSurfaceLevel) return false;
            // Set m_backbuffer for D3DImage
            m_backbuffer = (IntPtr)(void*)m_pSurfaceLevel;
            m_imageSourceRect = Int32Rect(0, 0, width, height);
            SetImageSourceBackBuffer();

            if (m_direct3DSurfaceType == Direct3DSurfaceType::Direct3DSurface11)
            {
                return true;
            }

            // Create Offscreen Plain Surface
            pin_ptr<IDirect3DSurface9*> ppSurface = &m_pSurface;
            IFC(m_pDevice9Ex->CreateOffscreenPlainSurfaceEx(width, height,
                m_format, D3DPOOL_DEFAULT, ppSurface, nullptr, 0));

            if (nullptr == m_pSurface) return false;

            return true;
        }

        bool D3D11Image::FillD3D9Surface(IntPtr buffer, int width, int height)
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

        bool D3D11Image::FillD3D9Surface(IntPtr yBuffer, UInt32 yStride, IntPtr uBuffer, UInt32 uStride, IntPtr vBuffer, UInt32 vStride)
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

        bool D3D11Image::FillD3D11Surface(IntPtr buffer, int width, int height)
        {
            if (m_pD2D1RenderTarget == nullptr)
            {
                return false;
            }
            m_pD2D1RenderTarget->BeginDraw();
            D2D1_RECT_U rect = { 0, 0, (UINT32)width, (UINT32)height };
            m_pD2D1Bitmap->CopyFromMemory(&rect, buffer.ToPointer(), m_width << 2);
            m_pD2D1RenderTarget->DrawBitmap(m_pD2D1Bitmap);
            m_pD2D1RenderTarget->EndDraw();
        }

        void D3D11Image::StretchSurface()
        {
            RECT rtVideo = { 0, 0, m_width, m_height };
            if (nullptr == m_pDevice9Ex) return;
            m_pDevice9Ex->StretchRect(m_pSurface, NULL, m_pSurfaceLevel, &rtVideo, D3DTEXF_NONE);
        }

        void D3D11Image::CreateScene()
        {
            if (nullptr == m_pDevice9Ex) return;
            m_pDevice9Ex->BeginScene();
            m_pDevice9Ex->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 1);
            m_pDevice9Ex->EndScene();
        }

        void D3D11Image::AllocResizeBuffer(int width, int height)
        {
            m_tempStride = width << 2;
            int tempLength = m_tempStride * (height >> 1);
            if (m_tempPtr != nullptr)
            {
                delete m_tempPtr;
            }
            m_tempPtr = new char[tempLength];
        }

        HRESULT D3D11Image::EnsureHWND()
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

        Cleanup:
            return hr;
        }
    }
}
