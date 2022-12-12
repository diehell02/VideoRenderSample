using Render.Interop;
using Render.Source;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using WPFSample.Utils.Threading;

namespace WPFSample
{
    internal abstract class VideoElement : Image
    {
        protected IRenderSource? _render;
        private readonly VideoFrameQueue _videoFrameQueue = new VideoFrameQueue();
        private readonly VideoFrame _videoFrame = new();
        private int _lastRenderWidth;
        private int _lastRenderHeight;
        private int _lastFrameWidth;
        private int _lastFrameHeight;
        private int _lastTargetWidth;
        private int _lastTargetHeight;
        protected bool _autoResize;
        private int _frameWidth;
        private int _frameHeight;
        private int _renderWidth;
        private int _renderHeight;
        private readonly IVideoSource _videoSource;
        private bool _isCleaned = false;

        public event EventHandler? OnVideoRendered;

        public VideoElement()
        {
            IsHitTestVisible = false;
            RenderOptions.SetBitmapScalingMode(this, BitmapScalingMode.LowQuality);
            _videoSource = VideoSourceFactory.GetVideoSource();
            _videoSource.VideoFrameEvent += VideoSource_VideoFrameEvent;
            RenderTimer.Instance.AddListener(OnRendering);
            Unloaded += VideoView_Unloaded;
        }

        public void UpdateViewPort(uint width, uint height)
        {
            _videoSource.UpdateViewPort(width, height);
        }

        public void Clean()
        {
            if (_isCleaned)
            {
                return;
            }
            _isCleaned = true;
            Trace.WriteLine("Enter");
            _render?.Clean();
            _videoFrameQueue.Clean();
            _videoSource.Close();
            RenderTimer.Instance.RemoveListener(OnRendering);
            Trace.WriteLine("Leave");
        }

        private void VideoSource_VideoFrameEvent(object? sender, VideoFrameEventArg e)
        {
            _videoFrame.OnFrame(e.YPtr, e.YStride, e.UPtr, e.UStride, e.VPtr, e.VStride, e.Width, e.Height);
            _videoFrameQueue.Write(_videoFrame);
        }

        private void OnRendering()
        {
            DoRender();
        }

        protected void VideoView_Unloaded(object? sender, RoutedEventArgs e)
        {
            Clean();
        }

        private void DoRender()
        {
            VideoFrame? readedFrame = _videoFrameQueue.Read();
            if (readedFrame is null)
            {
                return;
            }
            if (Width > 0 && Height > 0)
            {
                DpiScale dpiScale = VisualTreeHelper.GetDpi(this);
                UpdateRenderTargetSize((int)readedFrame.Width, (int)readedFrame.Height, (int)(Width * dpiScale.DpiScaleX), (int)(Height * dpiScale.DpiScaleY));
                if (_render != null && _render.SetupSurface(_lastTargetWidth, _lastTargetHeight))
                {
                    Source = _render.ImageSource;
                }
            }
            if (_render != null)
            {
                if (_autoResize)
                {
                    _render.Fill(readedFrame.YPtr, readedFrame.YStride, readedFrame.UPtr, readedFrame.UStride, readedFrame.VPtr, readedFrame.VStride, readedFrame.Width, readedFrame.Height);
                    OnVideoRendered?.Invoke(this, EventArgs.Empty);
                }
                else
                {
                    _render.Fill(readedFrame.YPtr, readedFrame.YStride, readedFrame.UPtr, readedFrame.UStride, readedFrame.VPtr, readedFrame.VStride);
                    OnVideoRendered?.Invoke(this, EventArgs.Empty);
                }
            }
            _videoFrameQueue.PutBack(readedFrame);
        }

        private void UpdateRenderTargetSize(int frameWidth, int frameHeight, int renderWidth, int renderHeight)
        {
            if (_frameWidth == frameWidth && _frameHeight == frameHeight && _renderWidth == renderWidth && _renderHeight == renderHeight)
            {
                return;
            }
            _frameWidth = frameWidth;
            _frameHeight = frameHeight;
            _renderWidth = renderWidth;
            _renderHeight = renderHeight;
            if (_autoResize)
            {
                if (renderWidth > 0 && renderHeight > 0 && ((renderWidth * 1.1) < frameWidth || (renderHeight * 1.1) < frameHeight))
                {
                    if (_lastRenderWidth != renderWidth ||
                        _lastRenderHeight != renderHeight ||
                        _lastFrameWidth != (int)frameWidth ||
                        _lastFrameHeight != (int)frameHeight)
                    {
                        _lastRenderWidth = (int)renderWidth;
                        _lastRenderHeight = (int)renderHeight;
                        _lastFrameWidth = (int)frameWidth;
                        _lastFrameHeight = (int)frameHeight;

                        decimal ratioRenderSize = (decimal)_lastRenderWidth / _lastRenderHeight;
                        decimal ratioFrame = (decimal)frameWidth / frameHeight;
                        if (ratioRenderSize > ratioFrame)
                        {
                            _lastTargetHeight = (int)_lastRenderHeight;
                            _lastTargetWidth = (int)(ratioFrame * _lastTargetHeight);
                        }
                        else
                        {
                            _lastTargetWidth = (int)_lastRenderWidth;
                            _lastTargetHeight = (int)(_lastTargetWidth / ratioFrame);
                        }
                    }
                }
                else
                {
                    _lastTargetWidth = frameWidth;
                    _lastTargetHeight = frameHeight;
                }
            }
            else
            {
                _lastTargetWidth = frameWidth;
                _lastTargetHeight = frameHeight;
            }

            if (_lastTargetWidth % 2 == 1)
            {
                _lastTargetWidth--;
            }
            if (_lastTargetHeight % 2 == 1)
            {
                _lastTargetHeight--;
            }
        }
    }
}
