﻿using System;
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
    internal class D3D11InteropImageSource : IRenderSource
    {
        private readonly Microsoft.Wpf.Interop.DirectX.D3D11Image _imageSource = new Microsoft.Wpf.Interop.DirectX.D3D11Image();
        private readonly D3D11InteropImage _d3D11InteropImage = new D3D11InteropImage();
        private ushort _width;
        private ushort _height;
        private bool _isFilled = false;

        private FrameConverter _frameConverter = new FrameConverter();
        private bool _isCleaned = false;
        private Int32Rect _rect;
        private int _bufferSize;
        private readonly object _lockObj = new();
        //private byte[]? _source;
        private byte[]? _dest;
        //private readonly IntPtr _hwnd = IntPtr.Zero;

        public ImageSource? ImageSource => _imageSource;

        public bool IsInitialize
        {
            get;
            private set;
        }

        public D3D11InteropImageSource()
        {
        }

        public D3D11InteropImageSource(IntPtr hwnd)
        {
            //_hwnd = hwnd;
            _imageSource.WindowOwner = hwnd;
            _imageSource.OnRender += _d3D11InteropImage.DoRender;
            _d3D11InteropImage.Render += _d3D11InteropImage_Render;
        }

        private void _d3D11InteropImage_Render(object? sender, EventArgs e)
        {
            GCHandle arrayHandle = GCHandle.Alloc(_dest, GCHandleType.Pinned);
            IntPtr buffer = arrayHandle.AddrOfPinnedObject();
            _d3D11InteropImage.Draw(buffer);
            arrayHandle.Free();
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
            _d3D11InteropImage.Width = width;
            _d3D11InteropImage.Height = height;
            //_rect = new Int32Rect(0, 0, _width, _height);
            _bufferSize = _width * _height << 2;
            //_source = new byte[_width * _height * 3 / 2];
            _dest = new byte[_bufferSize];
            _imageSource.SetPixelSize(width, height);
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

#if USE_LIBYUV
                {
                    GCHandle arrayHandle = GCHandle.Alloc(_dest, GCHandleType.Pinned);
                    IntPtr buffer = arrayHandle.AddrOfPinnedObject();
                    _frameConverter.I420ToARGB(
                        yPtr, yStride, uPtr, uStride, vPtr, vStride,
                        _width, _height, buffer);
                    arrayHandle.Free();
                    _imageSource.RequestRender();
                }
#else
                {
                    Marshal.Copy(yPtr, _source, 0, _source.Length);
                    VideoFrameConverter.YUV2RGBA(_source, _dest, _width, _height);
                    _imageSource.RequestRender();
                }
#endif
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
                    _dest = new byte[_bufferSize];
                    _imageSource.RequestRender();
                }
            }, System.Windows.Threading.DispatcherPriority.Send);
        }

    }
}
