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
        public ref class D3D11InteropImage
        {
        public:
            D3D11InteropImage::D3D11InteropImage();
            ~D3D11InteropImage();
            !D3D11InteropImage();

            property int Width
            {
                int get()
                {
                    return m_width;
                }
                void set(int width)
                {
                    m_width = width;
                }
            }

            property int Height
            {
                int get()
                {
                    return m_height;
                }
                void set(int height)
                {
                    m_height = height;
                }
            }

            event System::EventHandler^ Render;

        public:
            void DoRender(IntPtr surface, bool isNewSurface);
            void Draw(IntPtr buffer);

        private:
            ID2D1RenderTarget* m_pD2D1RenderTarget = nullptr;
            ID2D1Bitmap* m_pD2D1Bitmap = nullptr;
            int m_stride;
            int m_width;
            int m_height;
            IntPtr m_buffer;
        };
    }
}
