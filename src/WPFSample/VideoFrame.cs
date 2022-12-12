using Render.Interop;
using System;
using System.Collections.Generic;
using System.Text;

namespace WPFSample
{
    public class VideoFrame
    {
        #region Event

        public event EventHandler<VideoFrame>? FrameChanged;

        #endregion Event

        #region public Property

        public IntPtr YPtr { get; set; }

        public uint YStride { get; set; }

        public IntPtr UPtr { get; set; }

        public uint UStride { get; set; }

        public IntPtr VPtr { get; set; }

        public uint VStride { get; set; }

        public uint Width { get; set; }

        public uint Height { get; set; }

        public uint Size { get; set; }

        #endregion Property

        #region Public Function

        public void OnFrame(IntPtr yptr, uint yStride, IntPtr uptr, uint uStride, IntPtr vptr, uint vStride, uint width, uint height)
        {
            YPtr = yptr;
            YStride = yStride;
            UPtr = uptr;
            UStride = uStride;
            VPtr = vptr;
            VStride = vStride;
            Width = width - width % 2;
            if (YStride < Width)
            {
                Width = YStride;
            }
            Height = height - height % 2;
            Size = yStride * height + uStride * (height >> 1) + vStride * (height >> 1);
            FrameChanged?.Invoke(this, this);
        }

        public void Clean() => FrameChanged = null;

        #endregion Public Function
    }
}
