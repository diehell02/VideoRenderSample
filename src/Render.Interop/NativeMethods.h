#pragma once
#include <stdint.h>

using namespace System;

namespace Render {
    namespace Interop {
        public ref class NativeMethods
        {
        public:
            static void Memcpy(IntPtr dest, IntPtr source, uint64_t length);
        };
    }
}