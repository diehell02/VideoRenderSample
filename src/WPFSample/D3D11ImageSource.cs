using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows;
using System.Runtime.InteropServices;
using System.Diagnostics;
using WPFSample.Utils.Threading;
using Render.Source;
using Render.Interop;

namespace WPFSample
{
    internal class D3D11ImageSource : IRenderSource
    {
        private D3D11Image? _imageSource;
        private ushort _width;
        private ushort _height;
        private bool _isFilled = false;

        private FrameConverter _frameConverter = new FrameConverter();
        private bool _isCleaned = false;
        private Int32Rect _rect;
        private int _bufferSize;
        private readonly object _lockObj = new();
        private IntPtr _tempYPtr;
        private uint _tempYStride;
        private uint _tempYLength;
        private IntPtr _tempUPtr;
        private uint _tempUStride;
        private uint _tempULength;
        private IntPtr _tempVPtr;
        private uint _tempVStride;
        private uint _tempVLength;
        private byte[] _source;
        private byte[] _dest;

        private const bool USE_LIBYUV = true;

        public ImageSource? ImageSource => _imageSource;

        public bool IsInitialize
        {
            get;
            private set;
        }

        public D3D11ImageSource()
        {
            AllocResizeBuffer(1920, 1080);
        }

        /// <summary>
        /// Alloc the memory for resize
        /// </summary>
        /// <param name="width"></param>
        /// <param name="height"></param>
        private void AllocResizeBuffer(int width, int height)
        {
            _tempYStride = (uint)width;
            int tempYLength = width * height;
            if (_tempYPtr != IntPtr.Zero)
            {
                Marshal.FreeCoTaskMem(_tempYPtr);
            }
            _tempYPtr = Marshal.AllocCoTaskMem(tempYLength);
            _tempYLength = (uint)tempYLength;

            _tempUStride = (uint)(width >> 1);
            int tempULength = (int)_tempUStride * (height >> 1);
            if (_tempUPtr != IntPtr.Zero)
            {
                Marshal.FreeCoTaskMem(_tempUPtr);
            }
            _tempUPtr = Marshal.AllocCoTaskMem(tempULength);
            _tempULength = (uint)tempULength;

            _tempVStride = _tempUStride;
            int tempVLength = (int)_tempVStride * (height >> 1);
            if (_tempVPtr != IntPtr.Zero)
            {
                Marshal.FreeCoTaskMem(_tempVPtr);
            }
            _tempVPtr = Marshal.AllocCoTaskMem(tempVLength);
            _tempVLength = (uint)tempVLength;
        }

        /// <summary>
        /// Setup the WriteableBitmap
        /// </summary>
        /// <param name="width"></param>
        /// <param name="height"></param>
        /// <param name="format"></param>
        /// <returns></returns>
        public bool SetupSurface(int width, int height)
        {
            if (_width == width && _height == height)
            {
                return false;
            }

            //AllocResizeBuffer(width, height);
            _width = (ushort)width;
            _height = (ushort)height;
            _imageSource = new D3D11Image(Direct3DSurfaceType.Direct3DSurface11);
            _imageSource.SetupSurface(width, height);
            _rect = new Int32Rect(0, 0, _width, _height);
            _bufferSize = _width * _height << 2;
            _source = new byte[_width * _height * 3 / 2];
            _dest = new byte[_bufferSize];
            IsInitialize = true;
            return true;
        }

        /// <summary>
        /// Fill I420 data into WriteabltBitmap
        /// </summary>
        /// <param name="yPtr"></param>
        /// <param name="yStride"></param>
        /// <param name="uPtr"></param>
        /// <param name="uStride"></param>
        /// <param name="vPtr"></param>
        /// <param name="vStride"></param>
        /// <param name="videoWidth"></param>
        /// <param name="videoHeight"></param>
        public void Fill(IntPtr yPtr, uint yStride, IntPtr uPtr, uint uStride, IntPtr vPtr, uint vStride, uint videoWidth, uint videoHeight)
        {
            Fill(yPtr, yStride, uPtr, uStride, vPtr, vStride);
        }

        public void Fill(IntPtr yPtr, uint yStride, IntPtr uPtr, uint uStride, IntPtr vPtr, uint vStride)
        {
            lock (_lockObj)
            {
                if (_isCleaned)
                {
                    Trace.WriteLine($"_isCleaned is true, return");
                    return;
                }

                if (_imageSource is null)
                {
                    Trace.WriteLine($"_imageSource is null, return");
                    return;
                }

                _isFilled = true;

                if (USE_LIBYUV)
                {
                    GCHandle arrayHandle = GCHandle.Alloc(_dest, GCHandleType.Pinned);
                    IntPtr buffer = arrayHandle.AddrOfPinnedObject();
                    _frameConverter.I420ToARGB(
                        yPtr, yStride, uPtr, uStride, vPtr, vStride,
                        _width, _height, buffer);
                    _imageSource.WritePixels(buffer, _width, _height);
                    arrayHandle.Free();
                }
                else
                {
                    Marshal.Copy(yPtr, _source, 0, _source.Length);
                    VideoFrameConverter.YUV2RGBA(_source, _dest, _width, _height);
                    GCHandle arrayHandle = GCHandle.Alloc(_dest, GCHandleType.Pinned);
                    IntPtr buffer = arrayHandle.AddrOfPinnedObject();
                    _imageSource.WritePixels(buffer, _width, _height);
                    arrayHandle.Free();
                }
            }
        }

        /// <summary>
        /// Update Visual
        /// </summary>
        public void Draw()
        {
            if (_isCleaned)
            {
                Trace.WriteLine($"_isCleaned is true, return");
                return;
            }

            if (!_isFilled)
            {
                return;
            }

            if (_imageSource is null)
            {
                Trace.WriteLine($"_imageSource is null, return");
                return;
            }

            _imageSource.Lock();
            _imageSource.AddDirtyRect(_rect);
            _imageSource.Unlock();
            _isFilled = false;
        }

        public void Clean()
        {
            lock (_lockObj)
            {
                _isCleaned = true;
                if (_tempYPtr != IntPtr.Zero)
                {
                    Marshal.FreeCoTaskMem(_tempYPtr);
                }
                if (_tempUPtr != IntPtr.Zero)
                {
                    Marshal.FreeCoTaskMem(_tempUPtr);
                }
                if (_tempVPtr != IntPtr.Zero)
                {
                    Marshal.FreeCoTaskMem(_tempVPtr);
                }
            }
        }

        public void ClearScreen()
        {
            _imageSource?.Dispatcher.SafeInvokeAsync(() =>
            {
                lock (_lockObj)
                {
                    if (_isCleaned)
                    {
                        Trace.WriteLine($"_isCleaned is true, return");
                        return;
                    }

                    if (_imageSource is null)
                    {
                        Trace.WriteLine($"_imageSource is null, return");
                        return;
                    }

                    Trace.WriteLine($"Clear Screen");
                    byte[] bytes = new byte[_bufferSize];
                    GCHandle arrayHandle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
                    IntPtr buffer = arrayHandle.AddrOfPinnedObject();
                    _imageSource.WritePixels(buffer, _width, _height);
                    arrayHandle.Free();
                }
            }, System.Windows.Threading.DispatcherPriority.Send);
        }

    }
}
