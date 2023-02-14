using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace UnoSample.Utils
{
    internal class VideoFrameQueue : BufferQueue<VideoFrame>
    {
        private const int _bufferWidth = 1920;
        private const int _bufferHeight = 1080;
        private static readonly int s_bufferSize = _bufferWidth * _bufferHeight * 3 / 2;

        protected override VideoFrame CreateCacheItem()
        {
            VideoFrame videoFrame = new()
            {
                YPtr = Marshal.AllocCoTaskMem(s_bufferSize)
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

            unsafe
            {
                Buffer.MemoryCopy((void*)item.YPtr, (void*)buffer.YPtr, ySize, ySize);
                Buffer.MemoryCopy((void*)item.UPtr, (void*)(buffer.YPtr + (int)ySize), uSize, uSize);
                Buffer.MemoryCopy((void*)item.VPtr, (void*)(buffer.YPtr + (int)ySize + (int)uSize), vSize, vSize);
            }

            buffer.HasCopied = false;
        }

        protected override void CleanCacheItem(VideoFrame buffer)
        {
            Marshal.FreeCoTaskMem(buffer.YPtr);
        }
    }
}
