﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.WindowsRuntime;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media.Imaging;
using Microsoft.UI.Xaml;
using Render.Source;
using UnoSample.Utils;
using VideoFrame = UnoSample.Utils.VideoFrame;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace UnoSample
{
    public sealed partial class VideoView : UserControl
    {
        private WriteableBitmap? _writeableBitmap;
        private int _width;
        private int _height;
        private readonly IVideoSource _videoSource;
        private readonly VideoFrame _videoFrame = new();
        private readonly VideoFrameQueue _videoFrameQueue = new();
        private readonly FrameConverter _frameConverter = new();
        private byte[]? _frameBuffer;

        public VideoView()
        {
            _videoSource = VideoSourceFactory.GetVideoSource();
            _videoSource.VideoFrameEvent += VideoSource_VideoFrameEvent;
            RenderTimer.Instance.AddListener(OnRendering);
            this.SizeChanged += VideoView_SizeChanged;
            this.Unloaded += VideoView_Unloaded;
            this.InitializeComponent();
        }

        private void VideoView_Unloaded(object sender, RoutedEventArgs e)
        {
            _videoSource.Close();
            RenderTimer.Instance.RemoveListener(OnRendering);
        }

        private void OnRendering()
        {
            VideoFrame? videoFrame = _videoFrameQueue.Read();
            if (videoFrame is null)
            {
                return;
            }
            IntPtr buffer = _frameConverter.I420ToARGB(videoFrame.YPtr, videoFrame.Width, videoFrame.Height);
            DoRender(buffer, (int)videoFrame.Width, (int)videoFrame.Height);
            _videoFrameQueue.PutBack(videoFrame);
        }

        private void VideoView_SizeChanged(object? sender, SizeChangedEventArgs e)
        {
            _videoSource.UpdateViewPort((uint)e.NewSize.Width, (uint)e.NewSize.Height);
        }

        private void VideoSource_VideoFrameEvent(object? sender, VideoFrameEventArg e)
        {
            _videoFrame.OnFrame(e.YPtr, e.YStride, e.UPtr, e.UStride, e.VPtr, e.VStride, e.Width, e.Height);
            _videoFrameQueue.Write(_videoFrame);
        }

        public void DoRender(IntPtr buffer, int width, int height)
        {
            if (width == 0 || height == 0)
            {
                return;
            }
            if (_width != width || _height != height)
            {
                _width = width;
                _height = height;
                _writeableBitmap = new WriteableBitmap(width, height);
                VideoImage.Source = _writeableBitmap;
                _frameBuffer = new byte[_width * height * 4];
            }
            if (_frameBuffer is null)
            {
                return;
            }
            if (_writeableBitmap is null)
            {
                return;
            }
            Marshal.Copy(buffer, _frameBuffer, 0, _frameBuffer.Length);
            using Stream stream = _writeableBitmap.PixelBuffer.AsStream();
            stream.WriteAsync(_frameBuffer, 0, _frameBuffer.Length);
            _writeableBitmap.Invalidate();
        }
    }
}
