using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace Render.Source
{
    public class YuvFile
    {
        private IntPtr _backBuffer;

        public string? YuvPath { get; set; }

        public int FrameWidth { get; set; }

        public int FrameHeight { get; set; }

        public int FrameLength { get; set; }

        internal IntPtr GetFrame(int index)
        {
            var frameSize = FrameWidth * FrameHeight * 3 / 2;
            IntPtr frameBuffer = _backBuffer + index * frameSize;
            return frameBuffer;
        }

        internal void Init()
        {
            if (string.IsNullOrEmpty(YuvPath))
            {
                return;
            }
            using var fileStream = new FileStream(YuvPath, FileMode.Open);
            using var binaryReader = new BinaryReader(fileStream);
            _backBuffer = Marshal.AllocCoTaskMem((int)fileStream.Length);
            Marshal.Copy(binaryReader.ReadBytes((int)fileStream.Length), 0, _backBuffer, (int)fileStream.Length);
        }
    }
}
