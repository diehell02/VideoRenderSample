#include "D3D11InteropImage.h"

namespace Render {
    namespace Interop {
        D3D11InteropImage::D3D11InteropImage()
        {
        }

        D3D11InteropImage::~D3D11InteropImage()
        {
            this->!D3D11InteropImage();
        }

        D3D11InteropImage::!D3D11InteropImage()
        {
            SAFE_RELEASE(m_pD2D1RenderTarget);
            SAFE_RELEASE(m_pD2D1Bitmap);
        }

        void D3D11InteropImage::DoRender(IntPtr surface, bool isNewSurface)
        {
            if (isNewSurface)
            {
                IDXGISurface* pDXGISurface = (IDXGISurface*)(surface.ToPointer());
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
            Render(this, EventArgs::Empty);
        }
        void D3D11InteropImage::Draw(IntPtr buffer)
        {
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
    }
}
