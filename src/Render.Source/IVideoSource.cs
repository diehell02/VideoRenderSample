using System;
using System.Collections.Generic;
using System.Text;

namespace Render.Source
{
    public sealed class VideoFrameEventArg : EventArgs
    {
        public IntPtr YPtr { get; internal set; }

        public IntPtr UPtr { get; internal set; }

        public IntPtr VPtr { get; internal set; }

        public uint YStride { get; internal set; }

        public uint UStride { get; internal set; }

        public uint VStride { get; internal set; }

        public uint Width { get; internal set; }

        public uint Height { get; internal set; }
    }

    public interface IVideoSource
    {
        event EventHandler<VideoFrameEventArg>? VideoFrameEvent;

        void UpdateViewPort(uint width, uint height);

        void Close();
    }
}
