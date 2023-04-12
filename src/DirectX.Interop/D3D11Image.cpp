#include "D3D11Image.h"
#include <cstdint>
#include "D3D9Helper.h"
#include "DXGIHelper.h"

namespace Render {
    namespace Interop {
        D3D11Image::D3D11Image(IntPtr hwnd, FrameFormat frameFormat)
        {
            if (hwnd == IntPtr::Zero)
            {
                return;
            }
            m_hwnd = static_cast<HWND>(hwnd.ToPointer());
            switch (frameFormat)
            {
            case Render::Interop::FrameFormat::YU12:
                m_d3dImageHelper = gcnew D3D9Helper();
                break;
            case Render::Interop::FrameFormat::SharedHandle:
                m_d3dImageHelper = gcnew DXGIHelper();
                break;
            default:
                break;
            }
            m_d3dImageHelper->SetD3DImage(this);
            m_d3dImageHelper->SetHwnd(m_hwnd);
        }

        D3D11Image::~D3D11Image()
        {
            this->!D3D11Image();
        }

        D3D11Image::!D3D11Image()
        {
            MCH_VERIFY_VOID(m_d3dImageHelper);
            m_d3dImageHelper->CleanupD3D();
        }

        void D3D11Image::SetupSurface(int videoWidth, int videoHeight)
        {
            MCH_VERIFY_VOID(m_d3dImageHelper);
            m_d3dImageHelper->SetupSurface(videoWidth, videoHeight);
        }

        void D3D11Image::WritePixels(IntPtr buffer)
        {
            MCH_VERIFY_VOID(m_d3dImageHelper);
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
            MCH_VERIFY_VOID(m_d3dImageHelper);
            m_d3dImageHelper->WritePixels(yBuffer, yStride,
                uBuffer, uStride,
                vBuffer, vStride);
        }

        void D3D11Image::WritePixels(HANDLE hSharedHandle)
        {
            MCH_VERIFY_VOID(m_d3dImageHelper);
            m_d3dImageHelper->WritePixels(hSharedHandle);
        }
    }
}
