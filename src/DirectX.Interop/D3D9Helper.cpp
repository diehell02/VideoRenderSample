#include "D3D9Helper.h"
#include <libyuv.h>

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
        if (nullptr == m_pSurfaceRGBA)
        {
            goto UnlockD3DImage;
        }
        HRESULT lRet = m_pSurfaceRGBA->LockRect(&d3dRect, nullptr, D3DLOCK_DONOTWAIT);
        if (FAILED(lRet))
        {
            goto UnlockD3DImage;
        }

        FillB8G8R8A8((uint8_t*)buffer.ToPointer(), m_width << 2,
            (uint8_t*)d3dRect.pBits, d3dRect.Pitch);

        lRet = m_pSurfaceRGBA->UnlockRect();
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
        m_pDevice9Ex->StretchRect(m_pSurfaceRGBA, &rtVideo, m_pSurfaceLevel, &rtVideo, D3DTEXF_NONE);
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
        if (nullptr == m_pSurfaceYV12)
        {
            goto UnlockD3DImage;
        }
        HRESULT lRet = m_pSurfaceYV12->LockRect(&d3dRect, nullptr, D3DLOCK_DONOTWAIT);
        if (FAILED(lRet))
        {
            goto UnlockD3DImage;
        }

        FillYV12((uint8_t*)yBuffer.ToPointer(), yStride,
            (uint8_t*)uBuffer.ToPointer(), uStride,
            (uint8_t*)vBuffer.ToPointer(), vStride,
            (uint8_t*)d3dRect.pBits, d3dRect.Pitch);

        lRet = m_pSurfaceYV12->UnlockRect();
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
        m_pDevice9Ex->StretchRect(m_pSurfaceYV12, &rtVideo,
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
        pin_ptr<IDirect3DSurface9*> ppSurfaceYV12 = &m_pSurfaceYV12;
        IFC(m_pDevice9Ex->CreateOffscreenPlainSurfaceEx(m_width, m_height,
            (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2'), D3DPOOL_DEFAULT,
            ppSurfaceYV12, nullptr, 0));
        pin_ptr<IDirect3DSurface9*> ppSurfaceRGBA = &m_pSurfaceRGBA;
        IFC(m_pDevice9Ex->CreateOffscreenPlainSurfaceEx(m_width, m_height,
            D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
            ppSurfaceRGBA, nullptr, 0));

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
    SAFE_RELEASE(m_pSurfaceYV12);
    SAFE_RELEASE(m_pSurfaceRGBA);
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
