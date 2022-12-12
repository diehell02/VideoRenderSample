using Render.Interop;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace WPFSample
{
    internal class VideoFrameQueue : BufferQueue<VideoFrame>
    {
        private const int _bufferWidth = 1920;
        private const int _bufferHeight = 1080;
        private readonly int _bufferSize = _bufferWidth * _bufferHeight * 3 / 2;

        protected override VideoFrame CreateCacheItem()
        {
            VideoFrame videoFrame = new VideoFrame
            {
                YPtr = Marshal.AllocCoTaskMem(_bufferSize)
            };
            return videoFrame;
        }

        protected override void CopyIntoBuffer(VideoFrame buffer, VideoFrame item)
        {
            buffer.YStride = item.YStride;
            buffer.UStride = item.UStride;
            buffer.VStride = item.VStride;
            buffer.Width = item.Width;
            buffer.Height = item.Height;
            buffer.Size = item.Size;
            uint ySize = item.YStride * item.Height;
            uint uSize = (item.YStride >> 2) * item.Height;
            uint vSize = uSize;
            buffer.UPtr = buffer.YPtr + (int)ySize;
            buffer.VPtr = buffer.YPtr + (int)ySize + (int)uSize;

            NativeMethods.Memcpy(buffer.YPtr, item.YPtr, ySize);
            NativeMethods.Memcpy(buffer.YPtr + (int)ySize, item.UPtr, uSize);
            NativeMethods.Memcpy(buffer.YPtr + (int)ySize + (int)uSize, item.VPtr, vSize);
        }

        protected override void CleanCacheItem(VideoFrame buffer)
        {
            Marshal.FreeCoTaskMem(buffer.YPtr);
        }
    }
}
