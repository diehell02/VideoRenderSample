#include "D3DHiddenWindowHelper.h"

Render::Interop::D3DHiddenWindowHelper::D3DHiddenWindowHelper()
{
    HRESULT hr = EnsureHWND();
    if (hr != S_OK)
    {
        return;
    }
    mHandle = (IntPtr)mHwnd;
    Application::Current->Exit += gcnew ExitEventHandler(this, &D3DHiddenWindowHelper::OnExitEventHandler);
}

HRESULT Render::Interop::D3DHiddenWindowHelper::EnsureHWND()
{
    HRESULT hr = S_OK;

    if (!mHwnd)
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
        wndclass.lpszClassName = this->mD3DWindowClass;

        if (!RegisterClass(&wndclass))
        {
            return E_FAIL;
        }

        mHwnd = CreateWindow(this->mD3DWindowClass,
            TEXT("D3DWindowClass"),
            WS_OVERLAPPEDWINDOW,
            0,                   // Initial X
            0,                   // Initial Y
            0,                   // Width
            0,                   // Height
            NULL,
            NULL,
            NULL,
            NULL);
    }

    return hr;
}

void Render::Interop::D3DHiddenWindowHelper::OnExitEventHandler(Object^ sender, ExitEventArgs^ args)
{
    DestroyWindow(mHwnd);
    UnregisterClass(this->mD3DWindowClass, NULL);
}
