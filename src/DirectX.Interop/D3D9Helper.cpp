#include "D3D9Helper.h"
#include <libyuv.h>

Render::Interop::D3D9Helper::D3D9Helper(RenderFormat format)
{
    m_format = format;
    switch (format)
    {
    case Render::Interop::RenderFormat::YV12:
        m_d3dFormat = (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2');
        break;
    case Render::Interop::RenderFormat::NV12:
        m_d3dFormat = (D3DFORMAT)MAKEFOURCC('N', 'V', '1', '2');
        break;
    case Render::Interop::RenderFormat::B8G8R8A8:
        m_d3dFormat = D3DFORMAT::D3DFMT_A8B8G8R8;
        break;
    default:
        break;
    }
}

void Render::Interop::D3D9Helper::WritePixels(IntPtr buffer)
{
    if (!Initialize())
    {
        return;
    }
    if (nullptr == m_d3dImage)
    {
        return;
    }
    m_d3dImage->Lock();
    if (IntPtr::Zero != buffer)
    {
        D3DLOCKED_RECT d3dRect;
        if (nullptr == m_pSurface)
        {
            goto UnlockD3DImage;
        }
        HRESULT lRet = m_pSurface->LockRect(&d3dRect, nullptr, D3DLOCK_DONOTWAIT);
        if (FAILED(lRet))
        {
            goto UnlockD3DImage;
        }

        switch (m_format)
        {
        case Render::Interop::RenderFormat::YV12:
            libyuv::ARGBToI420((uint8_t*)buffer.ToPointer(), m_width << 2,
                (uint8_t*)yBuffer, yStride,
                (uint8_t*)uBuffer, uStride,
                (uint8_t*)vBuffer, vStride,
                m_width, m_height);
            FillYV12((uint8_t*)yBuffer, yStride,
                (uint8_t*)uBuffer, uStride,
                (uint8_t*)vBuffer, vStride,
                (uint8_t*)d3dRect.pBits, d3dRect.Pitch);
            break;
        case Render::Interop::RenderFormat::NV12:
            libyuv::ARGBToNV12((uint8_t*)buffer.ToPointer(), m_width << 2,
                (uint8_t*)yBuffer, yStride,
                (uint8_t*)uvBuffer, uvStride,
                m_width, m_height);
            FillNV12((uint8_t*)yBuffer, yStride,
                (uint8_t*)uvBuffer, uvStride,
                (uint8_t*)d3dRect.pBits, d3dRect.Pitch);
            break;
        case Render::Interop::RenderFormat::B8G8R8A8:
            FillB8G8R8A8((uint8_t*)buffer.ToPointer(), m_width << 2,
                (uint8_t*)d3dRect.pBits, d3dRect.Pitch);
            break;
        default:
            break;
        }

        lRet = m_pSurface->UnlockRect();
        if (FAILED(lRet))
        {
            goto UnlockD3DImage;
        }
    }
    if (m_backbuffer != (IntPtr)(void*)m_pSurfaceLevel)
    {
        m_backbuffer = (IntPtr)(void*)m_pSurfaceLevel;
        m_d3dImage->SetBackBuffer(D3DResourceType::IDirect3DSurface9,
            m_backbuffer,
            true);
    }
    RECT rtVideo = { 0, 0, m_width, m_height };
    if (m_pDevice9Ex)
    {
        m_pDevice9Ex->StretchRect(m_pSurface, &rtVideo, m_pSurfaceLevel, &rtVideo, D3DTEXF_NONE);
    }
    m_d3dImage->AddDirtyRect(m_imageSourceRect);
UnlockD3DImage:
    m_d3dImage->Unlock();
}

void Render::Interop::D3D9Helper::WritePixels(IntPtr yBuffer, UInt32 yStride, IntPtr uBuffer, UInt32 uStride, IntPtr vBuffer, UInt32 vStride)
{
    if (!Initialize())
    {
        return;
    }
    if (nullptr == m_d3dImage)
    {
        return;
    }
    m_d3dImage->Lock();
    if (yBuffer != IntPtr::Zero && yStride != 0
        && uBuffer != IntPtr::Zero && uStride != 0
        && vBuffer != IntPtr::Zero && vStride != 0)
    {
        D3DLOCKED_RECT d3dRect;
        HRESULT lRet = m_pSurface->LockRect(&d3dRect, nullptr, D3DLOCK_DONOTWAIT);
        if (FAILED(lRet))
        {
            goto UnlockD3DImage;
        }

        switch (m_format)
        {
        case Render::Interop::RenderFormat::YV12:
            FillYV12((uint8_t*)yBuffer.ToPointer(), yStride,
                (uint8_t*)uBuffer.ToPointer(), uStride,
                (uint8_t*)vBuffer.ToPointer(), vStride,
                (uint8_t*)d3dRect.pBits, d3dRect.Pitch);
            break;
        case Render::Interop::RenderFormat::NV12:
            libyuv::I420ToNV12((uint8_t*)yBuffer.ToPointer(), yStride,
                (uint8_t*)uBuffer.ToPointer(), uStride,
                (uint8_t*)vBuffer.ToPointer(), vStride,
                (uint8_t*)this->yBuffer, this->yStride,
                (uint8_t*)this->uvBuffer, this->uvStride,
                m_width, m_height);
            FillNV12((uint8_t*)this->yBuffer, yStride,
                (uint8_t*)this->uvBuffer, uvStride,
                (uint8_t*)d3dRect.pBits, d3dRect.Pitch);
            break;
        case Render::Interop::RenderFormat::B8G8R8A8:
            libyuv::I420ToARGB((uint8_t*)yBuffer.ToPointer(), yStride,
                (uint8_t*)uBuffer.ToPointer(), uStride,
                (uint8_t*)vBuffer.ToPointer(), vStride,
                (uint8_t*)this->buffer, this->stride,
                m_width, m_height);
            FillB8G8R8A8((uint8_t*)this->buffer, this->stride,
                (uint8_t*)d3dRect.pBits, d3dRect.Pitch);
            break;
        default:
            break;
        }

        lRet = m_pSurface->UnlockRect();
        if (FAILED(lRet))
        {
            goto UnlockD3DImage;
        }
    }
    if (m_backbuffer != (IntPtr)(void*)m_pSurfaceLevel)
    {
        m_backbuffer = (IntPtr)(void*)m_pSurfaceLevel;
        m_d3dImage->SetBackBuffer(D3DResourceType::IDirect3DSurface9,
            m_backbuffer,
            true);
    }
    if (m_pDevice9Ex)
    {
        RECT rtVideo = { 0, 0, m_width, m_height };
        m_pDevice9Ex->StretchRect(m_pSurface, &rtVideo,
            m_pSurfaceLevel, &rtVideo, D3DTEXF_NONE);
    }
    m_d3dImage->AddDirtyRect(m_imageSourceRect);
UnlockD3DImage:
    m_d3dImage->Unlock();
}

void Render::Interop::D3D9Helper::WritePixels(HANDLE hSharedHandle)
{
}

bool Render::Interop::D3D9Helper::Initialize()
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

bool Render::Interop::D3D9Helper::InitD3D()
{
    if (!m_isD3DInitialized)
    {
        if (!InitD3D9())
        {
            return false;
        }

        m_isD3DInitialized = true;
        return true;
    }
    return true;
}

bool Render::Interop::D3D9Helper::InitD3D9()
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

bool Render::Interop::D3D9Helper::InitSurfaces()
{
    HRESULT hr = S_OK;

    if (!m_isD3DInitialized)
    {
        hr = S_FALSE;
        goto Cleanup;
    }

    if (!m_areSurfacesInitialized)
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
        pin_ptr<IDirect3DSurface9*> ppSurface = &m_pSurface;
        IFC(m_pDevice9Ex->CreateOffscreenPlainSurfaceEx(m_width, m_height,
            m_d3dFormat, D3DPOOL_DEFAULT,
            ppSurface, nullptr, 0));

        m_areSurfacesInitialized = true;
    }

Cleanup:

    return SUCCEEDED(hr);
}

void Render::Interop::D3D9Helper::CleanupD3D()
{
    if (m_areSurfacesInitialized)
    {
        CleanupSurfaces();
    }

    m_isD3DInitialized = false;
    SAFE_RELEASE(m_pDevice9Ex);
    SAFE_RELEASE(m_pDirect3D9Ex);
}

void Render::Interop::D3D9Helper::CleanupSurfaces()
{
    m_areSurfacesInitialized = false;

    SAFE_RELEASE(m_pTexture);
    SAFE_RELEASE(m_pSurface);
    SAFE_RELEASE(m_pSurfaceLevel);
}

void Render::Interop::D3D9Helper::SetD3DImage(D3DImage^ d3dImage)
{
    m_d3dImage = d3dImage;
}

void Render::Interop::D3D9Helper::SetHwnd(HWND hwnd)
{
    m_hwnd = hwnd;
}

void Render::Interop::D3D9Helper::SetupSurface(int width, int height)
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

void Render::Interop::D3D9Helper::FillYV12(uint8_t* src_y, int stride_y,
    uint8_t* src_u, int stride_u,
    uint8_t* src_v, int stride_v,
    uint8_t* dst, int pitch)
{
    byte* pDest = dst;

    uint32_t stride = pitch;
    uint32_t w = m_imageSourceRect.Width, h = m_imageSourceRect.Height;
    uint32_t copyLen = min(stride, w);
    uint32_t ySize = stride * h;
    uint32_t uvSize = stride * h >> 2;
    byte* ySrcPtr = src_y;
    byte* uSrcPtr = src_u;
    byte* vSrcPtr = src_v;
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
}

void Render::Interop::D3D9Helper::FillNV12(uint8_t* src_y, int stride_y,
    uint8_t* src_uv, int stride_uv,
    uint8_t* dst, int pitch)
{
    byte* pDest = dst;

    uint32_t stride = pitch;
    uint32_t w = m_width, h = m_height;
    uint32_t copyLen = min(stride, w);
    uint32_t ySize = stride * h;
    uint32_t uvSize = stride * h >> 1;
    byte* ySrcPtr = src_y;
    byte* uvSrcPtr = src_uv;
    uint32_t halfHeight = h / 2;

    byte* yDestPtr = pDest;
    byte* uvDestPtr = pDest + ySize;

    if (stride == w)
    {
        memcpy_s(yDestPtr, ySize, ySrcPtr, ySize);
        memcpy_s(uvDestPtr, uvSize, uvSrcPtr, uvSize);
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
                    memcpy_s(uvDestPtr, copyLen, uvSrcPtr, copyLen);
                    uvDestPtr += stride;
                    uvSrcPtr += w;
                }
            }
        }
    }
}

void Render::Interop::D3D9Helper::FillB8G8R8A8(uint8_t* src, int stride,
    uint8_t* dst, int pitch)
{
    if (stride == pitch)
    {
        memcpy_s(dst, pitch * m_height, src, stride * m_height);
    }
    else
    {
        uint32_t copyLen = min(stride, pitch);
        if (m_width > 0 && m_height > 0 && stride > 0)
        {
            for (uint32_t i = 0; i < m_height; ++i)
            {
                memcpy_s(dst, copyLen, src, copyLen);
                dst += stride;
                src += pitch;
            }
        }
    }
}
