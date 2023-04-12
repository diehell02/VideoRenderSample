#pragma once

namespace Render {
    namespace Interop {
        public enum class FrameFormat
        {
            NONE = -1,
            YV12 = 0,
            NV12 = 1,
            YUY2 = 2,
            UYVY = 3,
            YU12 = 4,
            RGB15 = 10,
            RGB16 = 11,
            RGB24 = 12,
            RGB32 = 13,
            ARGB32 = 14,
            SharedHandle = 15,
        };
    }
}
