#include "DXGIHelper.h"
#include <libyuv.h>

#define WIDTH 640
#define HEIGHT 480

REFIID                  surfaceIDDXGI = __uuidof(IDXGISurface);
REFIID                  surfaceID9 = __uuidof(IDirect3DTexture9);

void Render::Interop::DXGIHelper::WritePixels(IntPtr buffer)
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

    m_d3dImage->Lock();
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

    if (isNewSurface)
    {
        InitRenderTarget(pDXGISurface);
    }

    RenderToDXGI(buffer, pDXGISurface);

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
        m_d3dImage->SetBackBuffer(D3DResourceType::IDirect3DSurface9,
            m_backbuffer,
            true);
    }

    // Produce Surface
    m_ABProducer->Enqueue(pTexture9, &count, sizeof(int), SURFACE_QUEUE_FLAG_DO_NOT_WAIT);

    // Flush the AB queue - use "do not wait" here, we'll block at the top of the *next* call if we need to
    m_ABProducer->Flush(SURFACE_QUEUE_FLAG_DO_NOT_WAIT, NULL);

    m_d3dImage->AddDirtyRect(m_imageSourceRect);

Cleanup:
    if (fNeedUnlock)
    {
        m_d3dImage->Unlock();
    }

    ReleaseInterface(pSurface9);

    ReleaseInterface(pTexture9);
    ReleaseInterface(pUnkTexture9);

    ReleaseInterface(pDXGISurface);
    ReleaseInterface(pUnkDXGISurface);
}

void Render::Interop::DXGIHelper::WritePixels(IntPtr yBuffer, UInt32 yStride, IntPtr uBuffer, UInt32 uStride, IntPtr vBuffer, UInt32 vStride)
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

    m_d3dImage->Lock();
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

    if (isNewSurface)
    {
        InitRenderTarget(pDXGISurface);
    }

    RenderToDXGI(yBuffer, yStride, uBuffer, uStride, vBuffer, vStride, pDXGISurface);

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
        m_d3dImage->SetBackBuffer(D3DResourceType::IDirect3DSurface9,
            m_backbuffer,
            true);
    }

    // Produce Surface
    m_ABProducer->Enqueue(pTexture9, &count, sizeof(int), SURFACE_QUEUE_FLAG_DO_NOT_WAIT);

    // Flush the AB queue - use "do not wait" here, we'll block at the top of the *next* call if we need to
    m_ABProducer->Flush(SURFACE_QUEUE_FLAG_DO_NOT_WAIT, NULL);

    m_d3dImage->AddDirtyRect(m_imageSourceRect);

Cleanup:
    if (fNeedUnlock)
    {
        m_d3dImage->Unlock();
    }

    ReleaseInterface(pSurface9);

    ReleaseInterface(pTexture9);
    ReleaseInterface(pUnkTexture9);

    ReleaseInterface(pDXGISurface);
    ReleaseInterface(pUnkDXGISurface);
}

void Render::Interop::DXGIHelper::WritePixels(HANDLE hSharedHandle)
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

    if (!m_d3dImage->Dispatcher->CheckAccess())
    {
        if (Monitor::TryEnter(m_sharedHandleLockObj) &&
            m_isD3DInitialized)
        {
            WriteToInputView(hSharedHandle);
        }
        goto Cleanup;
    }
    else
    {
        Monitor::Enter(m_sharedHandleLockObj);
        WriteToInputView(hSharedHandle);
    }

    m_d3dImage->Lock();
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

    if (isNewSurface)
    {
        InitRenderTarget(pDXGISurface);
    }

    RenderToDXGI(m_pInputView, pDXGISurface);

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
        m_d3dImage->SetBackBuffer(D3DResourceType::IDirect3DSurface9,
            m_backbuffer,
            true);
    }

    // Produce Surface
    m_ABProducer->Enqueue(pTexture9, &count, sizeof(int), SURFACE_QUEUE_FLAG_DO_NOT_WAIT);

    // Flush the AB queue - use "do not wait" here, we'll block at the top of the *next* call if we need to
    m_ABProducer->Flush(SURFACE_QUEUE_FLAG_DO_NOT_WAIT, NULL);

    m_d3dImage->AddDirtyRect(m_imageSourceRect);

Cleanup:
    if (fNeedUnlock)
    {
        m_d3dImage->Unlock();
    }

    ReleaseInterface(pSurface9);

    ReleaseInterface(pTexture9);
    ReleaseInterface(pUnkTexture9);

    ReleaseInterface(pDXGISurface);
    ReleaseInterface(pUnkDXGISurface);

    Monitor::Exit(m_sharedHandleLockObj);
}

bool Render::Interop::DXGIHelper::Initialize()
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

bool Render::Interop::DXGIHelper::InitD3D()
{
    if (!m_isD3DInitialized)
    {
        if (!InitD3D9())
        {
            return false;
        }
        if (!InitD3D10())
        {
            return false;
        }
        if (!InitD3D11())
        {
            return false;
        }

        if (!SUCCEEDED(InitVideoProcessor()))
        {
            return false;
        }

        m_isD3DInitialized = true;
        return true;
    }
    return true;
}

bool Render::Interop::DXGIHelper::InitD3D9()
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
    d3dpp.BackBufferWidth = 1;
    d3dpp.BackBufferHeight = 1;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

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

    return true;
}

bool Render::Interop::DXGIHelper::InitSurfaces()
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

            IFC(m_BAQueue->OpenProducer(m_pD3D10Device, ppm_BAProducer));
        }

        {
            pin_ptr<ISurfaceConsumer*> pinm_ABConsumer = &m_ABConsumer;
            ISurfaceConsumer** ppm_ABConsumer = pinm_ABConsumer;

            IFC(m_ABQueue->OpenConsumer(m_pD3D10Device, ppm_ABConsumer));
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

        m_areSurfacesInitialized = true;
    }

Cleanup:

    return SUCCEEDED(hr);
}

void Render::Interop::DXGIHelper::CleanupD3D()
{
    if (m_areSurfacesInitialized)
    {
        CleanupSurfaces();
    }

    m_isD3DInitialized = false;
    SAFE_RELEASE(m_pDevice9Ex);
    SAFE_RELEASE(m_pDirect3D9Ex);
    SAFE_RELEASE(m_pD3D10Device);
    SAFE_RELEASE(m_pD3D11Device);
    SAFE_RELEASE(m_pD3D11DeviceContext);
    SAFE_RELEASE(m_pDX11VideoDevice);
    SAFE_RELEASE(m_pVideoContext);
    SAFE_RELEASE(m_VideoProcessorEnum);
    SAFE_RELEASE(m_pVideoProcessor);
    SAFE_RELEASE(m_pDXGIMutex);
}

void Render::Interop::DXGIHelper::CleanupSurfaces()
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
    ReleaseInterface(m_pOutputView);
}

void Render::Interop::DXGIHelper::SetD3DImage(D3DImage^ d3dImage)
{
    m_d3dImage = d3dImage;
}

void Render::Interop::DXGIHelper::SetHwnd(HWND hwnd)
{
    m_hwnd = hwnd;
}

void Render::Interop::DXGIHelper::SetupSurface(int width, int height)
{
    if (m_width == width &&
        m_height == height)
    {
        return;
    }

    m_width = width;
    m_height = height;
    m_imageSourceRect = Int32Rect(0, 0, m_width, m_height);

    CleanupSurfaces();
    WritePixels(IntPtr::Zero);

    if (buffer)
    {
        delete buffer;
    }
    stride = width << 2;
    buffer = new char[stride * height];
}

bool Render::Interop::DXGIHelper::InitD3D11()
{
    HRESULT hr = S_OK;
    // D3D11 Device
    UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);
    pin_ptr<ID3D11Device*> ppD3D11Device = &m_pD3D11Device;
    pin_ptr<ID3D11DeviceContext*> ppD3D11DeviceContext = &m_pD3D11DeviceContext;
    pin_ptr<D3D_FEATURE_LEVEL> pfeatureLevel = &m_featureLevel;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; ++driverTypeIndex)
    {
        hr = D3D11CreateDevice(NULL, driverTypes[driverTypeIndex], NULL, createDeviceFlags, featureLevels, numFeatureLevels,
            D3D11_SDK_VERSION, ppD3D11Device, pfeatureLevel, ppD3D11DeviceContext);

        if (SUCCEEDED(hr))
        {
            m_driverType = driverTypes[driverTypeIndex];
            break;
        }
    }

Cleanup:
    return SUCCEEDED(hr);
}

bool Render::Interop::DXGIHelper::InitD3D10()
{
    HRESULT hr;
    UINT		DeviceFlags = D3D10_CREATE_DEVICE_BGRA_SUPPORT;
    //DWORD		dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;

#ifdef _DEBUG
                    // To debug DirectX, uncomment the following lines:

                    //DeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
                    //dwShaderFlags	|= D3D10_SHADER_DEBUG;
#endif

    {
        pin_ptr<ID3D10Device1*> pinD3D10Device = &m_pD3D10Device;
        ID3D10Device1** ppD3D10Device = pinD3D10Device;

        if (FAILED(hr = D3D10CreateDevice1(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL,
            DeviceFlags, D3D10_FEATURE_LEVEL_10_0, D3D10_1_SDK_VERSION, ppD3D10Device)))
        {
            return hr;
        }
    }

    D3D10_VIEWPORT vp;
    vp.Width = WIDTH;
    vp.Height = HEIGHT;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_pD3D10Device->RSSetViewports(1, &vp);

    return SUCCEEDED(hr);
}

HRESULT Render::Interop::DXGIHelper::InitRenderTarget(IDXGISurface* pDXGISurface)
{
    HRESULT hr = S_OK;

#pragma region D2DRenderTarget

    ID2D1Factory* pD2D1Factory = NULL;
    IFC(D2D1CreateFactory(
        D2D1_FACTORY_TYPE::D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &pD2D1Factory));
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
    IFC(pD2D1Factory->CreateDxgiSurfaceRenderTarget(pDXGISurface,
        &renderTargetProperties, ppD2D1RenderTarget));

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
    IFC(m_pD2D1RenderTarget->CreateBitmap(size, properties, ppD2D1Bitmap));

#pragma endregion

#pragma region OutputView

    IDXGIResource* pDXGIResource = NULL;
    IFC(pDXGISurface->QueryInterface(__uuidof(IDXGIResource), (void**)&pDXGIResource));

    HANDLE sharedHandle;
    IFC(pDXGIResource->GetSharedHandle(&sharedHandle));

    IUnknown* tempResource11 = NULL;
    IFC(m_pD3D11Device->OpenSharedResource(sharedHandle, __uuidof(ID3D11Resource), (void**)(&tempResource11)));

    ID3D11Texture2D* pOutputResource = NULL;
    IFC(tempResource11->QueryInterface(__uuidof(ID3D11Texture2D), (void**)(&pOutputResource)));

    D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC OutputViewDesc{};
    OutputViewDesc.ViewDimension = D3D11_VPOV_DIMENSION_TEXTURE2D;
    OutputViewDesc.Texture2D.MipSlice = 0;
    pin_ptr<ID3D11VideoProcessorOutputView*> ppOutputView = &m_pOutputView;
    IFC(m_pDX11VideoDevice->CreateVideoProcessorOutputView(
        pOutputResource,
        m_VideoProcessorEnum,
        &OutputViewDesc,
        ppOutputView));

#pragma endregion

Cleanup:
    ReleaseInterface(pD2D1Factory);
    ReleaseInterface(pDXGIResource);
    ReleaseInterface(tempResource11);
    ReleaseInterface(pOutputResource);
    return hr;
}

void Render::Interop::DXGIHelper::RenderToDXGI(IntPtr buffer, IDXGISurface* pDXGISurface)
{
    if (m_pD2D1RenderTarget == nullptr)
    {
        return;
    }
    if (m_pD2D1Bitmap == nullptr)
    {
        return;
    }
    if (buffer == IntPtr::Zero)
    {
        return;
    }
    m_pD2D1RenderTarget->BeginDraw();
    D2D1_RECT_U rect = { 0, 0, (UINT32)m_width, (UINT32)m_height };
    m_pD2D1Bitmap->CopyFromMemory(&rect, buffer.ToPointer(), m_width << 2);
    m_pD2D1RenderTarget->DrawBitmap(m_pD2D1Bitmap);
    m_pD2D1RenderTarget->EndDraw();
}

void Render::Interop::DXGIHelper::RenderToDXGI(IntPtr yBuffer, UInt32 yStride, IntPtr uBuffer, UInt32 uStride, IntPtr vBuffer, UInt32 vStride, IDXGISurface* pDXGISurface)
{
    if (m_pD2D1RenderTarget == nullptr)
    {
        return;
    }
    if (m_pD2D1Bitmap == nullptr)
    {
        return;
    }
    if (yBuffer == IntPtr::Zero ||
        uBuffer == IntPtr::Zero ||
        vBuffer == IntPtr::Zero)
    {
        return;
    }
    m_pD2D1RenderTarget->BeginDraw();
    libyuv::I420ToARGB((uint8_t*)yBuffer.ToPointer(), yStride,
        (uint8_t*)uBuffer.ToPointer(), uStride,
        (uint8_t*)vBuffer.ToPointer(), vStride,
        (uint8_t*)this->buffer, this->stride,
        m_width, m_height);
    D2D1_RECT_U rect = { 0, 0, (UINT32)m_width, (UINT32)m_height };
    m_pD2D1Bitmap->CopyFromMemory(&rect, this->buffer, this->stride);
    m_pD2D1RenderTarget->DrawBitmap(m_pD2D1Bitmap);
    m_pD2D1RenderTarget->EndDraw();
}

void Render::Interop::DXGIHelper::RenderToDXGI(ID3D11VideoProcessorInputView* pInputView, IDXGISurface* pDXGISurface)
{
    HRESULT hr = S_OK;

    if (nullptr == pInputView)
    {
        goto Cleanup;
    }

    RECT rect = { 0 };
    rect.right = m_width;
    rect.bottom = m_height;

    D3D11_VIDEO_PROCESSOR_STREAM StreamData{};
    StreamData.Enable = TRUE;
    StreamData.OutputIndex = 0;
    StreamData.InputFrameOrField = 0;
    StreamData.PastFrames = 0;
    StreamData.FutureFrames = 0;
    StreamData.ppPastSurfaces = NULL;
    StreamData.ppFutureSurfaces = NULL;
    StreamData.pInputSurface = pInputView;
    StreamData.ppPastSurfacesRight = NULL;
    StreamData.ppFutureSurfacesRight = NULL;
    StreamData.pInputSurfaceRight = NULL;

    m_pVideoContext->VideoProcessorSetStreamSourceRect(m_pVideoProcessor, 0, true, &rect);
    m_pVideoContext->VideoProcessorSetStreamFrameFormat(m_pVideoProcessor, 0, D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE);
    IFC(m_pVideoContext->VideoProcessorBlt(m_pVideoProcessor, m_pOutputView, 0, 1, &StreamData));

Cleanup:
    return;
}

void Render::Interop::DXGIHelper::WriteToInputView(HANDLE hSharedHandle)
{
    HRESULT hr = S_OK;

    ID3D11Texture2D* pD3D11Texture2D = NULL;

    if (nullptr == hSharedHandle)
    {
        goto Cleanup;
    }

    SAFE_RELEASE(m_pDXGIMutex);

    IFC(m_pD3D11Device->OpenSharedResource(hSharedHandle, __uuidof(ID3D11Texture2D), (void**)&pD3D11Texture2D));
    pin_ptr<IDXGIKeyedMutex*> ppDXGIMutex = &m_pDXGIMutex;
    hr = pD3D11Texture2D->QueryInterface(__uuidof(IDXGIKeyedMutex), (void**)ppDXGIMutex);
    if (SUCCEEDED(hr))
    {
        IFC(m_pDXGIMutex->AcquireSync(kSharedResAcquireKey, kAcquireSyncTimeout));
        mAcquiredSharedTexture = true;
    }

    D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC InputViewDesc;
    InputViewDesc.FourCC = 0;
    InputViewDesc.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
    InputViewDesc.Texture2D.MipSlice = 0;
    InputViewDesc.Texture2D.ArraySlice = 0;

    SAFE_RELEASE(m_pInputView);
    pin_ptr<ID3D11VideoProcessorInputView*> ppInputView = &m_pInputView;
    IFC(m_pDX11VideoDevice->CreateVideoProcessorInputView(
        pD3D11Texture2D,
        m_VideoProcessorEnum,
        &InputViewDesc,
        ppInputView));

    if (m_pDXGIMutex && mAcquiredSharedTexture) {
        m_pDXGIMutex->ReleaseSync(kSharedResReleaseKey);
        mAcquiredSharedTexture = false;
    }

Cleanup:
    
    SAFE_RELEASE(pD3D11Texture2D);
}

HRESULT Render::Interop::DXGIHelper::InitVideoProcessor()
{
    HRESULT hr = S_OK;

    if (m_VideoProcessorEnum)
        return S_FALSE;

    //create video processor
    D3D11_VIDEO_PROCESSOR_CONTENT_DESC ContentDesc;
    ZeroMemory(&ContentDesc, sizeof(ContentDesc));

    ContentDesc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
    ContentDesc.InputFrameRate.Numerator = 30000;
    ContentDesc.InputFrameRate.Denominator = 1000;
    ContentDesc.InputWidth = m_width;
    ContentDesc.InputHeight = m_height;
    ContentDesc.OutputWidth = m_width;
    ContentDesc.OutputHeight = m_height;
    ContentDesc.OutputFrameRate.Numerator = 30000;
    ContentDesc.OutputFrameRate.Denominator = 1000;

    ContentDesc.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;

    pin_ptr<ID3D11VideoDevice*> ppDX11VideoDevice = &m_pDX11VideoDevice;
    IFC(m_pD3D11Device->QueryInterface(__uuidof(ID3D11VideoDevice), (void**)ppDX11VideoDevice));

    pin_ptr<ID3D11VideoContext*> ppVideoContext = &m_pVideoContext;
    IFC(m_pD3D11DeviceContext->QueryInterface(__uuidof(ID3D11VideoContext), (void**)ppVideoContext));

    pin_ptr<ID3D11VideoProcessorEnumerator*> ppVideoProcessorEnum = &m_VideoProcessorEnum;
    IFC(m_pDX11VideoDevice->CreateVideoProcessorEnumerator(&ContentDesc, ppVideoProcessorEnum));

    pin_ptr<ID3D11VideoProcessor*> ppVideoProcessor = &m_pVideoProcessor;
    IFC(m_pDX11VideoDevice->CreateVideoProcessor(m_VideoProcessorEnum, 0, ppVideoProcessor));

Cleanup:
    return hr;
}
