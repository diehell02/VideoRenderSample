﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows;
using Render.Interop;
using System.Runtime.InteropServices;

namespace WPFSample
{
    internal class WriteableBitmapSource : IRenderSource
    {
        private WriteableBitmap? _imageSource;
        private ushort _width;
        private ushort _height;
        private bool _isFilled = false;
        private FrameConverter _frameConverter = new FrameConverter();
        private bool _isCleaned = false;
        private Int32Rect _rect;
        private int _bufferSize;
        private object _lockObj = new object();
        private IntPtr _tempYPtr;
        private uint _tempYStride;
        private uint _tempYLength;
        private IntPtr _tempUPtr;
        private uint _tempUStride;
        private uint _tempULength;
        private IntPtr _tempVPtr;
        private uint _tempVStride;
        private uint _tempVLength;

        public ImageSource? ImageSource => _imageSource;

        public bool IsInitialize
        {
            get;
            private set;
        }

        public WriteableBitmapSource()
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
            _imageSource = new WriteableBitmap(_width, _height, 96, 96, PixelFormats.Bgr32, null);
            _rect = new Int32Rect(0, 0, _width, _height);
            _bufferSize = _width * _height << 2;
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
            lock (_lockObj)
            {
                if (_isCleaned)
                {
                    return;
                }

                if (_imageSource is null)
                {
                    return;
                }

                _isFilled = true;

                _imageSource.Lock();
                if (_width < videoWidth || _height < videoHeight)
                {
                    // Convert I420 to ARGB with resize
                    _frameConverter.I420ToARGB(yPtr, yStride, uPtr, uStride, vPtr, vStride, videoWidth, videoHeight, _imageSource.BackBuffer,
                        _width, _height,
                        _tempYPtr, _tempYStride, _tempUPtr, _tempUStride, _tempVPtr, _tempVStride);
                }
                else
                {
                    // Convert I420 to ARGB without resize
                    _frameConverter.I420ToARGB(yPtr, yStride, uPtr, uStride, vPtr, vStride, _width, _height, _imageSource.BackBuffer);
                }
                _imageSource.AddDirtyRect(new Int32Rect(0, 0, _width, _height));
                _imageSource.Unlock();
            }
        }

        public void Fill(IntPtr yPtr, uint yStride, IntPtr uPtr, uint uStride, IntPtr vPtr, uint vStride)
        {
            lock (_lockObj)
            {
                if (_isCleaned)
                {
                    return;
                }

                if (_imageSource is null)
                {
                    return;
                }

                _isFilled = true;

                _imageSource.Lock();
                // Convert I420 to ARGB without resize
                _frameConverter.I420ToARGB(yPtr, yStride, uPtr, uStride, vPtr, vStride, _width, _height, _imageSource.BackBuffer);
                _imageSource.AddDirtyRect(new Int32Rect(0, 0, _width, _height));
                _imageSource.Unlock();
            }
        }

        /// <summary>
        /// Update Visual
        /// </summary>
        public void Draw()
        {
            if (_isCleaned)
            {
                return;
            }

            if (!_isFilled)
            {
                return;
            }

            if (_imageSource is null)
            {
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
    }
}
