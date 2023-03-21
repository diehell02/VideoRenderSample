#include "D3D11Image.h"
#include <cstdint>

REFIID                  surfaceIDDXGI = __uuidof(IDXGISurface);
REFIID                  surfaceID9 = __uuidof(IDirect3DTexture9);

namespace Render {
    namespace Interop {
        D3D11Image::D3D11Image(IntPtr hwnd)
        {
            m_hwnd = static_cast<HWND>(hwnd.ToPointer());
            this->IsFrontBufferAvailableChanged += gcnew System::Windows::DependencyPropertyChangedEventHandler(this, &D3D11Image::OnIsFrontBufferAvailableChanged);
        }

        D3D11Image::D3D11Image()
        {
            HRESULT hr = EnsureHWND();
            if (hr != S_OK)
            {
                return;
            }
            this->IsFrontBufferAvailableChanged += gcnew System::Windows::DependencyPropertyChangedEventHandler(this, &D3D11Image::OnIsFrontBufferAvailableChanged);
        }

        D3D11Image::~D3D11Image()
        {
            this->!D3D11Image();
        }

        D3D11Image::!D3D11Image()
        {
            CleanupD3D();
        }

        bool D3D11Image::SetupSurface(int videoWidth, int videoHeight)
        {
            if (m_width == videoWidth && m_height == videoHeight)
            {
                return true;
            }

            m_width = videoWidth;
            m_height = videoHeight;
            m_imageSourceRect = Int32Rect(0, 0, m_width, m_height);

            /*CleanupSurfaces();
            if (!Initialize())
            {
                return false;
            }*/

            return true;
        }

        void D3D11Image::WritePixels(IntPtr buffer, RenderMode renderMode)
        {
            if (!this->IsFrontBufferAvailable)
            {
                return;
            }
            EnsureBackbuffer(renderMode);
            switch (renderMode)
            {
            case Render::Interop::RenderMode::DX9_YV12:
                break;
            case Render::Interop::RenderMode::DX9_RGBA:
                WritePixelsToDX9(buffer);
                break;
            case Render::Interop::RenderMode::DXGI_RGBA:
                WritePixelsToDXGI(buffer);
                break;
            case Render::Interop::RenderMode::DXGI_Surface:
                break;
            default:
                break;
            }
        }

        void D3D11Image::WritePixels(IntPtr yBuffer, UInt32 yStride, IntPtr uBuffer, UInt32 uStride, IntPtr vBuffer, UInt32 vStride)
        {
            if (!this->IsFrontBufferAvailable)
            {
                return;
            }
            EnsureBackbuffer(RenderMode::DX9_YV12);
            if (!Initialize())
            {
                return;
            }
            this->Lock();
            D3DLOCKED_RECT d3dRect;
            HRESULT lRet = m_pSurface_YV12->LockRect(&d3dRect, nullptr, D3DLOCK_DONOTWAIT);
            if (FAILED(lRet))
            {
                return;
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
                memcpy_s(yDestPtr, ySize, ySrcPtr, ySize);
                memcpy_s(uDestPtr, uvSize, uSrcPtr, uvSize);
                memcpy_s(vDestPtr, uvSize, vSrcPtr, uvSize);
            }
            else
            {
                if (w > 0 && h > 0 && stride > 0)
                {
                    for (uint32_t i = 0; i < h; ++i)
                    {
                        memcpy_s(yDestPtr, copyLen, ySrcPtr, copyLen);
                        yDestPtr += stride;
                        ySrcPtr += w;
                        if (i < halfHeight)
                        {
                            memcpy_s(uDestPtr, halfCopyLen, uSrcPtr, halfCopyLen);
                            uDestPtr += halfStride;
                            uSrcPtr += halfWidth;
                            memcpy_s(vDestPtr, halfCopyLen, vSrcPtr, halfCopyLen);
                            vDestPtr += halfStride;
                            vSrcPtr += halfWidth;
                        }
                    }
                }
            }

            lRet = m_pSurface_YV12->UnlockRect();
            if (FAILED(lRet))
            {
                return;
            }
            if (m_backbuffer == IntPtr::Zero)
            {
                m_backbuffer = (IntPtr)(void*)m_pSurfaceLevel;
                this->SetBackBuffer(D3DResourceType::IDirect3DSurface9, m_backbuffer, true);
            }
            RECT rtVideo = { 0, 0, m_width, m_height };
            if (nullptr == m_pDevice9Ex) return;
            m_pDevice9Ex->StretchRect(m_pSurface_YV12, &rtVideo, m_pSurfaceLevel, &rtVideo, D3DTEXF_NONE);
            this->AddDirtyRect(m_imageSourceRect);
            this->Unlock();
        }

        void D3D11Image::WritePixels(HANDLE hSharedHandle)
        {
            if (!this->IsFrontBufferAvailable)
            {
                return;
            }
            EnsureBackbuffer(RenderMode::DXGI_Surface);
            HRESULT hr = S_OK;

            IDXGISurface* pDXGISurface = NULL;
            IUnknown* pUnkDXGISurface = NULL;

            IDirect3DTexture9* pTexture9 = NULL;
            IUnknown* pUnkTexture9 = NULL;

            IDirect3DSurface9* pSurface9 = NULL;

            DXGI_SURFACE_DESC desc;

            int count = 0;
            UINT size = sizeof(int);

            bool fNeedUnlock = false;

            this->Lock();
            fNeedUnlock = true;

            bool isNewSurface = !m_areSurfacesInitialized;

            if (!Initialize())
            {
                goto Cleanup;
            }

            // Flush the AB queue
            m_ABProducer->Flush(0 /* wait */, NULL);

            // Dequeue from AB queue
            IFC(m_ABConsumer->Dequeue(surfaceIDDXGI, &pUnkDXGISurface, &count, &size, INFINITE));

            IFC(pUnkDXGISurface->QueryInterface(surfaceIDDXGI, (void**)&pDXGISurface));

            IFC(pDXGISurface->GetDesc(&desc));

            IUnknown* pSurface = NULL;
            m_pD3D11Device->OpenSharedResource(hSharedHandle, __uuidof(ID3D11Texture2D), (void**)&pSurface);
            IFC(CopySurface(pDXGISurface, pSurface, m_width, m_height));

            // Produce the surface
            m_BAProducer->Enqueue(pDXGISurface, NULL, NULL, SURFACE_QUEUE_FLAG_DO_NOT_WAIT);

            // Flush the BA queue
            m_BAProducer->Flush(0 /* wait, *not* SURFACE_QUEUE_FLAG_DO_NOT_WAIT*/, NULL);

            // Dequeue from BA queue
            IFC(m_BAConsumer->Dequeue(surfaceID9, &pUnkTexture9, NULL, NULL, INFINITE));
            IFC(pUnkTexture9->QueryInterface(surfaceID9, (void**)&pTexture9));

            // Get the top level surface from the texture
            IFC(pTexture9->GetSurfaceLevel(0, &pSurface9));

            if (m_backbuffer != (IntPtr)(void*)pSurface9)
            {
                m_backbuffer = (IntPtr)(void*)pSurface9;
                this->SetBackBuffer(D3DResourceType::IDirect3DSurface9,
                    m_backbuffer, true);
            }

            // Produce Surface
            m_ABProducer->Enqueue(pTexture9, &count, sizeof(int), SURFACE_QUEUE_FLAG_DO_NOT_WAIT);

            // Flush the AB queue - use "do not wait" here, we'll block at the top of the *next* call if we need to
            m_ABProducer->Flush(SURFACE_QUEUE_FLAG_DO_NOT_WAIT, NULL);

        Cleanup:
            if (fNeedUnlock)
            {
                this->AddDirtyRect(m_imageSourceRect);
                this->Unlock();
            }

            ReleaseInterface(pSurface9);

            ReleaseInterface(pTexture9);
            ReleaseInterface(pUnkTexture9);

            ReleaseInterface(pDXGISurface);
            ReleaseInterface(pUnkDXGISurface);
        }

        void D3D11Image::CleanupD3D()
        {
            if (m_areSurfacesInitialized)
            {
                CleanupSurfaces();
            }

            m_isD3DInitialized = false;
            SAFE_RELEASE(m_pDevice9Ex);
            SAFE_RELEASE(m_pDirect3D9Ex);
        }

        void D3D11Image::CleanupSurfaces()
        {
            m_areSurfacesInitialized = false;
            ReleaseInterface(m_BAProducer);
            ReleaseInterface(m_ABProducer);
            ReleaseInterface(m_BAConsumer);
            ReleaseInterface(m_ABConsumer);

            ReleaseInterface(m_ABQueue);
            ReleaseInterface(m_BAQueue);

            ReleaseInterface(m_pD2D1RenderTarget);
            ReleaseInterface(m_pD2D1Bitmap);

            SAFE_RELEASE(m_pTexture);
            SAFE_RELEASE(m_pSurface_YV12);
            SAFE_RELEASE(m_pSurface_RGBA);
            SAFE_RELEASE(m_pSurfaceLevel);
        }

        bool D3D11Image::InitD3D()
        {
            if (!m_isD3DInitialized)
            {
                if (!InitD3D9())
                {
                    return false;
                }
                if (!InitD3D11())
                {
                    return false;
                }

                m_isD3DInitialized = true;
                return true;
            }
            return true;
        }

        bool D3D11Image::InitD3D9()
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
                d3dpp.BackBufferWidth = 1;
                d3dpp.BackBufferHeight = 1;
                d3dpp.BackBufferCount = 10;
                d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
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

        bool D3D11Image::InitD3D11()
        {
            // D3D11 Device
            D3D_FEATURE_LEVEL featureLevels[] = {
                D3D_FEATURE_LEVEL_11_1
            };
            UINT numFeatureLevels = ARRAYSIZE(featureLevels);
            pin_ptr<ID3D11Device*> ppD3D11Device = &m_pD3D11Device;
            pin_ptr<ID3D11DeviceContext*> ppD3D11DeviceContext = &m_pD3D11DeviceContext;
            IFF(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
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
                    640,                   // Width
                    480,                   // Height
                    NULL,
                    NULL,
                    NULL,
                    NULL);
                m_createdHiddenWindow = true;
            }

            l.release();
            return hr;
        }

        bool D3D11Image::Initialize()
        {
            HRESULT hr = S_OK;

            if (m_isD3DInitialized)
            {
                hr = m_pDevice9Ex->CheckDeviceState(NULL);

                if (D3D_OK != hr)
                {
                    goto Cleanup;
                }
            }

            if (!m_isD3DInitialized)
            {
                if (!InitD3D())
                {
                    goto Cleanup;
                }
            }

            if (!m_areSurfacesInitialized)
            {
                if (!InitSurfaces())
                {
                    goto Cleanup;
                }
            }

            return true;

        Cleanup:

            CleanupD3D();
            return false;
        }

        bool D3D11Image::InitSurfaces()
        {
            HRESULT hr = S_OK;

            SURFACE_QUEUE_DESC  desc;
            ZeroMemory(&desc, sizeof(desc));
            desc.Width = m_width;
            desc.Height = m_height;
            desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            desc.NumSurfaces = 1;
            desc.MetaDataSize = sizeof(int);
            desc.Flags = SURFACE_QUEUE_FLAG_SINGLE_THREADED;

            SURFACE_QUEUE_CLONE_DESC CloneDesc = { 0 };
            CloneDesc.MetaDataSize = 0;
            CloneDesc.Flags = SURFACE_QUEUE_FLAG_SINGLE_THREADED;

            if (!m_isD3DInitialized || (desc.Width <= 0) || (desc.Height <= 0))
            {
                hr = S_FALSE;
                goto Cleanup;
            }

            if (!m_areSurfacesInitialized)
            {
                // 
                // Initialize the surface queues
                //

                {
                    pin_ptr<ISurfaceQueue*> pinABQueue = &m_ABQueue;
                    ISurfaceQueue** ppABQueue = pinABQueue;

                    IFC(CreateSurfaceQueue(&desc, m_pDevice9Ex, ppABQueue));
                }

                // Clone the queue           
                {
                    pin_ptr<ISurfaceQueue*> pinBAQueue = &m_BAQueue;
                    ISurfaceQueue** ppBAQueue = pinBAQueue;

                    IFC(m_ABQueue->Clone(&CloneDesc, ppBAQueue));
                }

                // Setup queue management
                {
                    pin_ptr<ISurfaceProducer*> pinm_BAProducer = &m_BAProducer;
                    ISurfaceProducer** ppm_BAProducer = pinm_BAProducer;

                    IFC(m_BAQueue->OpenProducer(m_pD3D11Device, ppm_BAProducer));
                }

                {
                    pin_ptr<ISurfaceConsumer*> pinm_ABConsumer = &m_ABConsumer;
                    ISurfaceConsumer** ppm_ABConsumer = pinm_ABConsumer;

                    IFC(m_ABQueue->OpenConsumer(m_pD3D11Device, ppm_ABConsumer));
                }

                {
                    pin_ptr<ISurfaceProducer*> pinm_ABProducer = &m_ABProducer;
                    ISurfaceProducer** ppm_ABProducer = pinm_ABProducer;

                    IFC(m_ABQueue->OpenProducer(m_pDevice9Ex, ppm_ABProducer));
                }

                {
                    pin_ptr<ISurfaceConsumer*> pinm_BAConsumer = &m_BAConsumer;
                    ISurfaceConsumer** ppm_BAConsumer = pinm_BAConsumer;

                    IFC(m_BAQueue->OpenConsumer(m_pDevice9Ex, ppm_BAConsumer));
                }

                {
                    // Create Direct3D Texture
                    pin_ptr<IDirect3DTexture9*> ppTexture = &m_pTexture;
                    IFC(m_pDevice9Ex->CreateTexture(m_width, m_height, 1,
                        D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT,
                        ppTexture, NULL));
                    if (nullptr == m_pTexture) return false;

                    // Get Direct3D Surface
                    pin_ptr<IDirect3DSurface9*> ppSurfaceLevel = &m_pSurfaceLevel;
                    IFC(m_pTexture->GetSurfaceLevel(0, ppSurfaceLevel));
                    if (nullptr == m_pSurfaceLevel) return false;

                    // Create Offscreen Plain Surface
                    pin_ptr<IDirect3DSurface9*> ppSurface_YV12 = &m_pSurface_YV12;
                    IFC(m_pDevice9Ex->CreateOffscreenPlainSurfaceEx(m_width, m_height,
                        (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2'), D3DPOOL_DEFAULT,
                        ppSurface_YV12, nullptr, 0));
                    pin_ptr<IDirect3DSurface9*> ppSurface_RGBA = &m_pSurface_RGBA;
                    IFC(m_pDevice9Ex->CreateOffscreenPlainSurfaceEx(m_width, m_height,
                        D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
                        ppSurface_RGBA, nullptr, 0));
                }

                m_areSurfacesInitialized = true;
            }

        Cleanup:

            return SUCCEEDED(hr);
        }

        void D3D11Image::RenderToDXGI(IntPtr buffer, IDXGISurface* pDXGISurface, bool isNewSurface)
        {
            if (isNewSurface)
            {
                SAFE_RELEASE(m_pD2D1RenderTarget);
                SAFE_RELEASE(m_pD2D1Bitmap);

                ID2D1Factory* pD2D1Factory = NULL;
                D2D1CreateFactory(
                    D2D1_FACTORY_TYPE::D2D1_FACTORY_TYPE_SINGLE_THREADED,
                    &pD2D1Factory);
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
                pD2D1Factory->CreateDxgiSurfaceRenderTarget(pDXGISurface,
                    &renderTargetProperties, ppD2D1RenderTarget);
                SAFE_RELEASE(pD2D1Factory);

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
                D2D1_SIZE_U size = { (UINT32)m_width, (UINT32)m_height };
                m_pD2D1RenderTarget->CreateBitmap(size, properties, ppD2D1Bitmap);
            }
            if (m_pD2D1RenderTarget == nullptr)
            {
                return;
            }
            if (m_pD2D1Bitmap == nullptr)
            {
                return;
            }
            m_pD2D1RenderTarget->BeginDraw();
            D2D1_RECT_U rect = { 0, 0, (UINT32)m_width, (UINT32)m_height };
            m_pD2D1Bitmap->CopyFromMemory(&rect, buffer.ToPointer(), m_width << 2);
            m_pD2D1RenderTarget->DrawBitmap(m_pD2D1Bitmap);
            m_pD2D1RenderTarget->EndDraw();
        }

        void D3D11Image::WritePixelsToDX9(IntPtr buffer)
        {
            if (!Initialize())
            {
                return;
            }
            this->Lock();
            D3DLOCKED_RECT d3dRect;
            if (nullptr == m_pSurface_RGBA) return;
            HRESULT lRet = m_pSurface_RGBA->LockRect(&d3dRect, nullptr, D3DLOCK_DONOTWAIT);
            if (FAILED(lRet))
            {
                return;
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
                memcpy_s(destPtr, bufferStride * h, srcPtr, bufferStride * h);
            }
            else
            {
                uint32_t copyLen = min(stride, bufferStride);
                if (w > 0 && h > 0 && stride > 0)
                {
                    for (uint32_t i = 0; i < h; ++i)
                    {
                        memcpy_s(destPtr, copyLen, srcPtr, copyLen);
                        destPtr += stride;
                        srcPtr += bufferStride;
                    }
                }
            }

            lRet = m_pSurface_RGBA->UnlockRect();
            if (FAILED(lRet))
            {
                return;
            }
            if (m_backbuffer == IntPtr::Zero)
            {
                m_backbuffer = (IntPtr)(void*)m_pSurfaceLevel;
                this->SetBackBuffer(D3DResourceType::IDirect3DSurface9, m_backbuffer, true);
            }
            RECT rtVideo = { 0, 0, m_width, m_height };
            if (nullptr == m_pDevice9Ex) return;
            m_pDevice9Ex->StretchRect(m_pSurface_RGBA, &rtVideo, m_pSurfaceLevel, &rtVideo, D3DTEXF_NONE);
            this->AddDirtyRect(m_imageSourceRect);
            this->Unlock();
        }

        void D3D11Image::WritePixelsToDXGI(IntPtr buffer)
        {
            HRESULT hr = S_OK;

            IDXGISurface* pDXGISurface = NULL;
            IUnknown* pUnkDXGISurface = NULL;

            IDirect3DTexture9* pTexture9 = NULL;
            IUnknown* pUnkTexture9 = NULL;

            IDirect3DSurface9* pSurface9 = NULL;

            DXGI_SURFACE_DESC desc;

            int count = 0;
            UINT size = sizeof(int);

            bool fNeedUnlock = false;

            this->Lock();
            fNeedUnlock = true;

            bool isNewSurface = !m_areSurfacesInitialized;

            if (!Initialize())
            {
                goto Cleanup;
            }

            // Flush the AB queue
            m_ABProducer->Flush(0 /* wait */, NULL);

            // Dequeue from AB queue
            IFC(m_ABConsumer->Dequeue(surfaceIDDXGI, &pUnkDXGISurface, &count, &size, INFINITE));

            IFC(pUnkDXGISurface->QueryInterface(surfaceIDDXGI, (void**)&pDXGISurface));

            IFC(pDXGISurface->GetDesc(&desc));

            /*DXGI_MAPPED_RECT mapRect{};
            IFC(pDXGISurface->Map(&mapRect, DXGI_MAP_WRITE | DXGI_MAP_DISCARD));
            auto* src = (BYTE*)buffer.ToPointer();
            auto* dest = mapRect.pBits;
            auto stride = m_width << 2;
            for (uint32_t row = 0; row < m_width; ++row) {
                memcpy_s(dest, stride, src, stride);
                dest += mapRect.Pitch;
                src += stride;
            }
            IFC(pDXGISurface->Unmap());*/

            RenderToDXGI(buffer, pDXGISurface, isNewSurface);

            // Produce the surface
            m_BAProducer->Enqueue(pDXGISurface, NULL, NULL, SURFACE_QUEUE_FLAG_DO_NOT_WAIT);

            // Flush the BA queue
            m_BAProducer->Flush(0 /* wait, *not* SURFACE_QUEUE_FLAG_DO_NOT_WAIT*/, NULL);

            // Dequeue from BA queue
            IFC(m_BAConsumer->Dequeue(surfaceID9, &pUnkTexture9, NULL, NULL, INFINITE));
            IFC(pUnkTexture9->QueryInterface(surfaceID9, (void**)&pTexture9));

            // Get the top level surface from the texture
            IFC(pTexture9->GetSurfaceLevel(0, &pSurface9));

            if (m_backbuffer != (IntPtr)(void*)pSurface9)
            {
                m_backbuffer = (IntPtr)(void*)pSurface9;
                this->SetBackBuffer(D3DResourceType::IDirect3DSurface9,
                    m_backbuffer, true);
            }

            // Produce Surface
            m_ABProducer->Enqueue(pTexture9, &count, sizeof(int), SURFACE_QUEUE_FLAG_DO_NOT_WAIT);

            // Flush the AB queue - use "do not wait" here, we'll block at the top of the *next* call if we need to
            m_ABProducer->Flush(SURFACE_QUEUE_FLAG_DO_NOT_WAIT, NULL);

        Cleanup:
            if (fNeedUnlock)
            {
                this->AddDirtyRect(m_imageSourceRect);
                this->Unlock();
            }

            ReleaseInterface(pSurface9);

            ReleaseInterface(pTexture9);
            ReleaseInterface(pUnkTexture9);

            ReleaseInterface(pDXGISurface);
            ReleaseInterface(pUnkDXGISurface);
        }

        void D3D11Image::EnsureBackbuffer(RenderMode renderMode)
        {
            if (m_renderMode == renderMode)
            {
                return;
            }
            m_renderMode = renderMode;
            m_backbuffer = IntPtr::Zero;
            CleanupSurfaces();
        }

        HRESULT D3D11Image::CopySurface(IUnknown* pDst, IUnknown* pSrc, UINT width, UINT height)
        {
            HRESULT hr;

            D3D11_BOX UnitBox = { 0, 0, 0, width, height, 1 };

            ID3D11DeviceContext* pContext = m_pD3D11DeviceContext;
            ID3D11Resource* pSrcRes = NULL;
            ID3D11Resource* pDstRes = NULL;

            if (FAILED(hr = pDst->QueryInterface(__uuidof(ID3D11Resource), (void**)&pDstRes)))
            {
                goto end;
            }

            if (FAILED(hr = pSrc->QueryInterface(__uuidof(ID3D11Resource), (void**)&pSrcRes)))
            {
                goto end;
            }

            pContext->CopySubresourceRegion(
                pDstRes,
                0,
                0, 0, 0, //(x, y, z)
                pSrcRes,
                0,
                &UnitBox);
        end:
            if (pSrcRes)
            {
                pSrcRes->Release();
            }
            if (pDstRes)
            {
                pDstRes->Release();
            }
            if (pContext)
            {
                pContext->Release();
            }

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
