#pragma once

#include "stdafx.h"

using namespace System;
using namespace System::Windows;

namespace Render {
    namespace Interop {
        public ref class D3DHiddenWindowHelper
        {
#pragma region Static

        public:
            static property D3DHiddenWindowHelper^ Instance {
                D3DHiddenWindowHelper^ get() {
                    if (s_instance == nullptr)
                    {
                        s_instance = gcnew D3DHiddenWindowHelper();
                    }
                    return s_instance;
                }
            }
        private:
            static D3DHiddenWindowHelper^ s_instance;

#pragma endregion

        public:
            D3DHiddenWindowHelper();

            property IntPtr Handle {
                IntPtr get() {
                    return mHandle;
                }
            }
        private:
            HRESULT EnsureHWND();
            void OnExitEventHandler(Object^ sender, ExitEventArgs^ args);

        private:
            IntPtr mHandle = IntPtr::Zero;
            HWND mHwnd;
            LPCWSTR mD3DWindowClass = L"D3DWindowClass";
        };
    }
}
