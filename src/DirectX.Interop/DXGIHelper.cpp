#include "DXGIHelper.h"
#include <libyuv.h>

REFIID                  surfaceIDDXGI = __uuidof(IDXGISurface);
REFIID                  surfaceID9 = __uuidof(IDirect3DTexture9);

Render::Interop::DXGIHelper::DXGIHelper(RenderFormat format)
{
    m_format = format;
    switch (format)
    {
    case Render::Interop::RenderFormat::YV12:
        m_dxgi_format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
        break;
    case Render::Interop::RenderFormat::NV12:
        m_dxgi_format = DXGI_FORMAT::DXGI_FORMAT_NV12;
        break;
    case Render::Interop::RenderFormat::B8G8R8A8:
        m_dxgi_format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
        break;
    default:
        break;
    }
}

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
        CreateBitmap(pDXGISurface);
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

Cleanup:
    if (fNeedUnlock)
    {
        m_d3dImage->AddDirtyRect(m_imageSourceRect);
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
        CreateBitmap(pDXGISurface);
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

Cleanup:
    if (fNeedUnlock)
    {
        m_d3dImage->AddDirtyRect(m_imageSourceRect);
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
        CreateBitmap(pDXGISurface);
    }

    RenderToDXGI(hSharedHandle, pDXGISurface);

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

Cleanup:
    if (fNeedUnlock)
    {
        m_d3dImage->AddDirtyRect(m_imageSourceRect);
        m_d3dImage->Unlock();
    }

    ReleaseInterface(pSurface9);

    ReleaseInterface(pTexture9);
    ReleaseInterface(pUnkTexture9);

    ReleaseInterface(pDXGISurface);
    ReleaseInterface(pUnkDXGISurface);
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
        if (!InitD3D11())
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
    //IFF(m_pDevice9Ex->SetRenderState(D3DRS_LIGHTING, FALSE));
    //IFF(m_pDevice9Ex->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE));
    //IFF(m_pDevice9Ex->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
    //IFF(m_pDevice9Ex->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE));
    //IFF(m_pDevice9Ex->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));

    return true;
}

bool Render::Interop::DXGIHelper::InitSurfaces()
{
    HRESULT hr = S_OK;

    SURFACE_QUEUE_DESC  desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = m_width;
    desc.Height = m_height;
    desc.Format = m_dxgi_format;
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
    SAFE_RELEASE(m_pD3D11Device);
    SAFE_RELEASE(m_pD3D11DeviceContext);
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

    switch (m_format)
    {
    case Render::Interop::RenderFormat::YV12:
        if (yBuffer)
        {
            yStride = width;
            delete yBuffer;
            yBuffer = new char[yStride * height];
        }
        if (uBuffer)
        {
            uStride = (width >> 1);
            delete uBuffer;
            uBuffer = new char[uStride * (height >> 1)];
        }
        if (vBuffer)
        {
            vStride = (width >> 1);
            delete vBuffer;
            vBuffer = new char[vStride * (height >> 1)];
        }
        break;
    case Render::Interop::RenderFormat::NV12:
        if (yBuffer)
        {
            yStride = width;
            delete yBuffer;
            yBuffer = new char[yStride * height];
        }
        if (uvBuffer)
        {
            uvStride = (width);
            delete uvBuffer;
            uvBuffer = new char[uvStride * (height >> 1)];
        }
        delete uvBuffer;
        break;
    case Render::Interop::RenderFormat::B8G8R8A8:
        if (buffer)
        {
            stride = width << 2;
            delete buffer;
            buffer = new char[stride * height];
        }
        break;
    default:
        break;
    }
}

bool Render::Interop::DXGIHelper::InitD3D11()
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

void Render::Interop::DXGIHelper::CreateBitmap(IDXGISurface* pDXGISurface)
{
    SAFE_RELEASE(m_pD2D1RenderTarget);
    SAFE_RELEASE(m_pD2D1Bitmap);

    ID2D1Factory* pD2D1Factory = NULL;
    D2D1CreateFactory(
        D2D1_FACTORY_TYPE::D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &pD2D1Factory);
    D2D1_PIXEL_FORMAT pixelFormat{};
    pixelFormat.format = m_dxgi_format;
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
        m_dxgi_format,
        D2D1_ALPHA_MODE_PREMULTIPLIED
    };
    properties.dpiX = dpiX;
    properties.dpiY = dpiY;
    pin_ptr<ID2D1Bitmap*> ppD2D1Bitmap = &m_pD2D1Bitmap;
    D2D1_SIZE_U size = { (UINT32)m_width, (UINT32)m_height };
    m_pD2D1RenderTarget->CreateBitmap(size, properties, ppD2D1Bitmap);
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
    switch (m_format)
    {
    case Render::Interop::RenderFormat::NV12:
    {
        libyuv::ARGBToNV12((uint8_t*)buffer.ToPointer(), m_width << 2,
            (uint8_t*)yBuffer, yStride,
            (uint8_t*)uvBuffer, uvStride,
            m_width, m_height);
        D2D1_RECT_U yRect = { 0, 0, (UINT32)m_width, (UINT32)m_height };
        m_pD2D1Bitmap->CopyFromMemory(&yRect, yBuffer, yStride);
        D2D1_RECT_U uvRect = { 0, (UINT32)m_height, (UINT32)m_width, (UINT32)m_height + ((UINT32)m_height >> 1) };
        m_pD2D1Bitmap->CopyFromMemory(&uvRect, uvBuffer, uvStride);
    }
        break;
    case Render::Interop::RenderFormat::B8G8R8A8:
    {
        D2D1_RECT_U rect = { 0, 0, (UINT32)m_width, (UINT32)m_height };
        m_pD2D1Bitmap->CopyFromMemory(&rect, buffer.ToPointer(), m_width << 2);
    }
        break;
    default:
        break;
    }
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
    switch (m_format)
    {
    case Render::Interop::RenderFormat::NV12:
    {
        libyuv::I420ToNV12((uint8_t*)yBuffer.ToPointer(), yStride,
            (uint8_t*)uBuffer.ToPointer(), uStride,
            (uint8_t*)vBuffer.ToPointer(), vStride,
            (uint8_t*)this->yBuffer, this->yStride,
            (uint8_t*)this->uvBuffer, this->uvStride,
            m_width, m_height);
        D2D1_RECT_U yRect = { 0, 0, (UINT32)m_width, (UINT32)m_height };
        m_pD2D1Bitmap->CopyFromMemory(&yRect, this->yBuffer, this->yStride);
        D2D1_RECT_U uvRect = { 0, (UINT32)m_height, (UINT32)m_width, (UINT32)m_height + ((UINT32)m_height >> 1) };
        m_pD2D1Bitmap->CopyFromMemory(&uvRect, this->uvBuffer, this->uvStride);
    }
        break;
    case Render::Interop::RenderFormat::B8G8R8A8:
    {
        libyuv::I420ToARGB((uint8_t*)yBuffer.ToPointer(), yStride,
            (uint8_t*)uBuffer.ToPointer(), uStride,
            (uint8_t*)vBuffer.ToPointer(), vStride,
            (uint8_t*)this->buffer, this->stride,
            m_width, m_height);
        D2D1_RECT_U rect = { 0, 0, (UINT32)m_width, (UINT32)m_height };
        m_pD2D1Bitmap->CopyFromMemory(&rect, this->buffer, this->stride);
    }
        break;
    default:
        break;
    }
    m_pD2D1RenderTarget->DrawBitmap(m_pD2D1Bitmap);
    m_pD2D1RenderTarget->EndDraw();
}

void Render::Interop::DXGIHelper::RenderToDXGI(HANDLE hSharedHandle, IDXGISurface* pDXGISurface)
{
    IUnknown* pSurface = NULL;
    m_pD3D11Device->OpenSharedResource(hSharedHandle, __uuidof(ID3D11Texture2D), (void**)&pSurface);
    CopySurface(pDXGISurface, pSurface, m_width, m_height);
}

HRESULT Render::Interop::DXGIHelper::CopySurface(IUnknown* pDst, IUnknown* pSrc, UINT width, UINT height)
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
