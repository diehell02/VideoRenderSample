// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
using Avalonia.Media.Imaging;
using Avalonia.Platform;
using Avalonia.Rendering;
using Avalonia.Threading;
using AvaloniaSample.Utils;
using Render.Source;

namespace AvaloniaSample.Views
{
    internal class VideoView2 : Control
    {
        private WriteableBitmap? _writeableBitmap;
        private int _width;
        private int _height;
        private readonly IVideoSource _videoSource;
        private readonly VideoFrame _videoFrame = new();
        private readonly VideoFrameQueue _videoFrameQueue = new();
        private byte[]? _frameBuffer;
        private bool _stopRendering = false;
        private DateTime _lastRenderTime;

        public VideoView2()
        {
            _videoSource = VideoSourceFactory.GetVideoSource();
            _videoSource.VideoFrameEvent += VideoSource_VideoFrameEvent;
            PropertyChanged += VideoView_PropertyChanged;
            DetachedFromVisualTree += VideoView_DetachedFromVisualTree;
        }
        private void VideoView_DetachedFromVisualTree(object? sender, Avalonia.VisualTreeAttachmentEventArgs e)
        {
            _stopRendering = true;
            _videoSource.Close();
        }

        private void VideoView_PropertyChanged(object? sender, Avalonia.AvaloniaPropertyChangedEventArgs e)
        {
            if (e.Property == BoundsProperty)
            {
                OnBoundsChanged();
            }
        }

        private void VideoSource_VideoFrameEvent(object? sender, VideoFrameEventArg e)
        {
            _videoFrame.OnFrame(e.YPtr, e.YStride, e.UPtr, e.UStride, e.VPtr, e.VStride, e.Width, e.Height);
            _videoFrameQueue.Write(_videoFrame);
        }

        private void OnRendering()
        {
            VideoFrame? videoFrame = _videoFrameQueue.Read();
            if (videoFrame is null)
            {
                return;
            }
            DoRender(videoFrame.YPtr, (int)videoFrame.Width, (int)videoFrame.Height);
            _videoFrameQueue.PutBack(videoFrame);
        }

        private void OnBoundsChanged()
        {
            _videoSource.UpdateViewPort((uint)Bounds.Width, (uint)Bounds.Height);
        }

        private void DoRender(IntPtr buffer, int width, int height)
        {
            if (width == 0 || height == 0)
            {
                return;
            }
            if (_width != width || _height != height)
            {
                _width = width;
                _height = height;
                _writeableBitmap = new WriteableBitmap(new Avalonia.PixelSize(width, height),
                    new Avalonia.Vector(96, 96),
                    Avalonia.Platform.PixelFormat.Bgra8888,
                    Avalonia.Platform.AlphaFormat.Premul);
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
            using ILockedFramebuffer lockedFramebuffer = _writeableBitmap.Lock();
            VideoFrameConverter.YUV2RGBA(buffer, lockedFramebuffer.Address, (uint)_width, (uint)_height);
        }

        public override void Render(DrawingContext context)
        {
            base.Render(context);
            if (_stopRendering)
            {
                return;
            }
            DateTime now = DateTime.Now;
            //if (now - _lastRenderTime > TimeSpan.FromMilliseconds(40))
            {
                _lastRenderTime = now;
                OnRendering();
                if (_writeableBitmap is not null)
                {
                    context.DrawImage(_writeableBitmap,
                        new Rect(0, 0, _width, _height),
                        this.Bounds);
                }
            }
            Dispatcher.UIThread.Post(InvalidateVisual, DispatcherPriority.Background);
        }
    }
}
