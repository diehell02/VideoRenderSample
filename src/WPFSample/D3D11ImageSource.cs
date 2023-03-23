using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows;
using System.Runtime.InteropServices;
using System.Diagnostics;
using Render.Interop;
using WPFSample.Utils.Threading;
using WPFSample;

namespace Render.Core
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
        private readonly object _lockObj = new object();
        private byte[]? _source;
        private byte[]? _dest;
        private readonly IntPtr _hwnd = IntPtr.Zero;

        public ImageSource? ImageSource => _imageSource;

        public bool IsInitialize
        {
            get;
            private set;
        }

        public D3D11ImageSource()
        {
        }

        public D3D11ImageSource(IntPtr hwnd)
        {
            _hwnd = hwnd;
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

            _width = (ushort)width;
            _height = (ushort)height;
            var imageSource = _imageSource;
            if (_hwnd == IntPtr.Zero)
            {
                _imageSource = new D3D11Image(RenderMode.DXGI, RenderFormat.NV12);
            }
            else
            {
                _imageSource = new D3D11Image(RenderMode.DXGI, RenderFormat.NV12);
            }
            imageSource?.Dispose();
            _imageSource?.SetupSurface(width, height);
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

#if DX_YUV
                _imageSource.WritePixels(yPtr, yStride, uPtr, uStride, vPtr, vStride);
#else
                GCHandle arrayHandle = GCHandle.Alloc(_dest, GCHandleType.Pinned);
                IntPtr buffer = arrayHandle.AddrOfPinnedObject();
                _frameConverter.I420ToARGB(
                    yPtr, yStride, uPtr, uStride, vPtr, vStride,
                    _width, _height, buffer);
                _imageSource.WritePixels(buffer);
                arrayHandle.Free();
#endif
            }
        }

        public unsafe void Fill(void* sharedHandle, uint videoWidth, uint videoHeight)
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

                _imageSource.WritePixels(sharedHandle);
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

            //_imageSource.Lock();
            //_imageSource.AddDirtyRect(_rect);
            //_imageSource.Unlock();
            _isFilled = false;
        }

        public void Clean()
        {
            lock (_lockObj)
            {
                _isCleaned = true;
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
                    _imageSource.WritePixels(IntPtr.Zero);
                }
            }, System.Windows.Threading.DispatcherPriority.Send);
        }

    }
}
