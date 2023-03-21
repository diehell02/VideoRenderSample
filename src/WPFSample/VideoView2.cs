using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using System.Windows.Threading;
using System.Windows;
using WPFSample.Utils.Threading;
using System.Diagnostics;
using System.Windows.Media.Media3D;
using Render.Source;
using System.Windows.Interop;
using System.Windows.Markup;
using System.Windows.Controls;

namespace WPFSample
{
    public class VideoView2 : ContentControl
    {
        #region Internal

        private enum LifetimeStage
        {
            Unloaded,
            Loaded,
        }

        #endregion

        #region DependencyProperty

        public static readonly DependencyProperty AutoResizeProperty =
            DependencyProperty.Register(
                nameof(AutoResize),
                typeof(bool),
                typeof(VideoView2),
                new PropertyMetadata(false, (d, e) =>
                {
                    if (d is VideoView2 videoView && videoView.VideoElement is not null)
                    {
                        videoView.VideoElement.AutoResize = (bool)e.NewValue;
                    }
                }));

        public static readonly DependencyProperty StretchProperty =
            DependencyProperty.Register(
                nameof(Stretch),
                typeof(Stretch),
                typeof(VideoView2),
                new PropertyMetadata(Stretch.Uniform, (d, e) =>
                {
                    if (d is VideoView2 videoView && videoView.VideoElement is not null)
                    {
                        videoView.VideoElement.Stretch = (Stretch)e.NewValue;
                    }
                }));

        public bool AutoResize
        {
            get => (bool)GetValue(AutoResizeProperty);
            set => SetValue(AutoResizeProperty, value);
        }

        public Stretch Stretch
        {
            get => (Stretch)GetValue(StretchProperty);
            set => SetValue(StretchProperty, value);
        }

        #endregion DependencyProperty

        #region Event

        public event EventHandler? OnVideoRendered;

        #endregion

        #region Field

        private readonly HostVisual _hostVisual;
        private VisualTargetPresentationSource? _targetSource;
        private VideoElement? VideoElement;
        private Dispatcher? _dispatcher;
        private double _width = 0;
        private double _height = 0;
        private object _setVideoStateLock = new();
        private bool _videoState = false;
        private IVideoSource? _videoSource = null;
        private LifetimeStage _lifetimeStage = LifetimeStage.Unloaded;


        #endregion

        #region Constructor

        public VideoView2()
        {
            _hostVisual = new HostVisual();
            IsHitTestVisible = false;
            Loaded += VideoView_Loaded;
            Unloaded += VideoView_Unloaded;
            SizeChanged += VideoView_SizeChanged;
        }

        #endregion

        #region Function

        public void VideoView_Unloaded(object sender, RoutedEventArgs e)
        {
            Trace.WriteLine("Enter");
            if (_lifetimeStage == LifetimeStage.Unloaded)
            {
                Trace.WriteLine("Leave");
                return;
            }
            _lifetimeStage = LifetimeStage.Unloaded;
            Visibility = Visibility.Collapsed;
            OnVideoRendered = null;
            VideoElement?.Clean();
            DispatcherManager.ReleaseRenderDispatcher(_dispatcher);
            _dispatcher = null;
            Trace.WriteLine("Leave");
        }

        public void SetVideoState(bool videoState)
        {
            Trace.WriteLine($"hasVideo: {videoState}");
            lock (_setVideoStateLock)
            {
                if (_videoState == videoState)
                {
                    return;
                }
                _videoState = videoState;
                if (VideoElement is not null)
                {
                    VideoElement.SetState(videoState);
                    lock (_setVideoStateLock)
                    {
                        if (!videoState)
                        {
                            VideoElement.OnVideoRendered -= VideoViewContainer_OnVideoRendered;
                            VideoElement.OnVideoRendered += VideoViewContainer_OnVideoRendered;
                        }
                    }
                }
            }
        }

        public void SetVideoSource(IVideoSource videoSource)
        {
            _videoSource = videoSource;
            if (VideoElement is not null)
            {
                VideoElement.VideoSource = videoSource;
            }
        }

        private void VideoViewContainer_OnVideoRendered(object? sender, EventArgs e)
        {
            lock (_setVideoStateLock)
            {
                if (!_videoState)
                {
                    Trace.WriteLine($"_videoState is false, return");
                    return;
                }
                HandleVideoRendered();
            }
        }

        private void HandleVideoRendered()
        {
            if (VideoElement is not null)
            {
                VideoElement.OnVideoRendered -= VideoViewContainer_OnVideoRendered;
            }
            OnVideoRendered?.Invoke(this, EventArgs.Empty);
        }

        private void VideoView_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            _width = e.NewSize.Width;
            _height = e.NewSize.Height;
            if (VideoElement is not null)
            {
                VideoElement.Width = _width;
                VideoElement.Height = _height;
                VideoElement.UpdateViewPort((uint)_width, (uint)_height);
            }
        }

        private void VideoView_Loaded(object sender, RoutedEventArgs e)
        {
            if (_lifetimeStage == LifetimeStage.Loaded)
            {
                return;
            }
            _lifetimeStage = LifetimeStage.Loaded;
            Visibility = Visibility.Visible;
            _dispatcher = DispatcherManager.CreateRenderDispatcher();
            if (_dispatcher is null)
            {
                return;
            }
            Stretch stretch = Stretch;
            bool autoResize = AutoResize;
            Content = VideoElement = CreateVideoView(stretch, autoResize);
        }

        private VideoElement CreateVideoView(Stretch stretch, bool autoResize)
        {
#if D3DImage
                var videoView = new D3DImageElement(hwnd);
#elif D3D11Image
            var videoView = new D3D11ImageElement();
#else
                var videoView = new WriteableBitmapElement();
#endif
            if (videoView is VideoElement videoElement)
            {
                videoElement.AutoResize = autoResize;
                videoElement.Stretch = stretch;
                videoElement.VideoSource = _videoSource;
                videoElement.SetState(_videoState);
            }
            videoView.HorizontalAlignment = HorizontalAlignment.Center;
            videoView.VerticalAlignment = VerticalAlignment.Center;
            videoView.Width = _width;
            videoView.Height = _height;
            videoView.UpdateViewPort((uint)_width, (uint)_height);
            return videoView;
        }

        #endregion
    }
}
