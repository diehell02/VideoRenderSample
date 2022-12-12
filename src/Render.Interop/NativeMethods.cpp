#include "NativeMethods.h"
#include <memory>

void Render::Interop::NativeMethods::Memcpy(IntPtr dest, IntPtr source, uint64_t length)
{
    memcpy_s(dest.ToPointer(), length, source.ToPointer(), length);
}
