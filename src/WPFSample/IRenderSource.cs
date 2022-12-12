using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;

namespace WPFSample
{
    internal interface IRenderSource
    {
        bool SetupSurface(int videoWidth, int videoHeight);

        void Fill(IntPtr yBuffer, UInt32 yStride, IntPtr uBuffer, UInt32 uStride, IntPtr vBuffer, UInt32 vStride);

        void Fill(IntPtr yBuffer, UInt32 yStride, IntPtr uBuffer, UInt32 uStride, IntPtr vBuffer, UInt32 vStride, UInt32 videoWidth, UInt32 videoHeight);

        void Draw();

        void Clean();

        ImageSource? ImageSource { get; }

        bool IsInitialize { get; }
    }
}
