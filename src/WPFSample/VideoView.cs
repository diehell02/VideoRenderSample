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

namespace WPFSample
{
    public class VideoView : FrameworkElement
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
                typeof(VideoView),
                new PropertyMetadata(false, (d, e) =>
                {
                    if (d is VideoView videoView && videoView.Child is VideoElement videoElement)
                    {
                        _ = videoElement.Dispatcher.InvokeAsync(() =>
                        {
                            videoElement.AutoResize = (bool)e.NewValue;
                        });
                    }
                }));

        public static readonly DependencyProperty StretchProperty =
            DependencyProperty.Register(
                nameof(Stretch),
                typeof(Stretch),
                typeof(VideoView),
                new PropertyMetadata(Stretch.Uniform, (d, e) =>
                {
                    if (d is VideoView videoView && videoView.Child is VideoElement videoElement)
                    {
                        _ = videoElement.Dispatcher.SafeInvokeAsync(() =>
                        {
                            videoElement.Stretch = (Stretch)e.NewValue;
                        });
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
        private VideoElement? VideoElement => Child as VideoElement;
        private Dispatcher? _dispatcher;
        private double _width = 0;
        private double _height = 0;
        private object _setVideoStateLock = new();
        private bool _videoState = false;
        private IVideoSource? _videoSource = null;
        private LifetimeStage _lifetimeStage = LifetimeStage.Unloaded;


        #endregion

        #region Constructor

        public VideoView()
        {
            _hostVisual = new HostVisual();
            IsHitTestVisible = false;
            Loaded += VideoView_Loaded;
            Unloaded += VideoView_Unloaded;
            SizeChanged += VideoView_SizeChanged;
        }

        #endregion

        #region Child

        private bool _isUpdatingChild;

        public UIElement? Child { get; private set; }

        private async Task SetChildAsync<T>(Dispatcher? dispatcher = null)
            where T : UIElement, new()
        {
            await SetChildAsync(() => new T(), dispatcher);
        }

        private async Task SetChildAsync<T>(Func<T> @new, Dispatcher? dispatcher = null)
            where T : UIElement
        {
            dispatcher ??= await UIDispatcher.RunNewAsync($"{typeof(T).Name}");
            if (dispatcher is null)
            {
                return;
            }
            T child = await dispatcher.InvokeAsync(@new);
            await SetChildAsync(child);
        }

        private async Task SetChildAsync(UIElement? value)
        {
            if (_isUpdatingChild)
            {
                throw new InvalidOperationException("Child property should not be set during Child updating.");
            }

            _isUpdatingChild = true;
            try
            {
                await SetChildAsync();
            }
            finally
            {
                _isUpdatingChild = false;
            }

            async Task SetChildAsync()
            {
                var oldChild = Child;
                var visualTarget = _targetSource;
                var hostVisual = _hostVisual;
                if (Equals(oldChild, value))
                    return;

                _targetSource = null;
                if (visualTarget != null)
                {
                    RemoveVisualChild(hostVisual);
                    await Dispatcher.Yield(DispatcherPriority.Loaded);
                    await visualTarget.Dispatcher.InvokeAsync(visualTarget.Dispose);
                }

                Child = value;
                if (Child is VideoElement view)
                {
                    view.OnVideoRendered -= VideoViewContainer_OnVideoRendered;
                    view.OnVideoRendered += VideoViewContainer_OnVideoRendered;
                }
                if (value == null)
                {
                    _targetSource = null;
                }
                else
                {
                    await value.Dispatcher.InvokeAsync(() =>
                    {
                        _targetSource = new VisualTargetPresentationSource(_hostVisual)
                        {
                            RootVisual = value,
                        };
                    });
                    AddVisualChild(_hostVisual);
                }
                InvalidateMeasure();
            }
        }

        #endregion

        #region Tree & Layout

        protected override Visual GetVisualChild(int index)
        {
            if (index != 0)
                throw new ArgumentOutOfRangeException(nameof(index));
            return _hostVisual;
        }

        protected override int VisualChildrenCount => Child != null ? 1 : 0;

        protected override Size MeasureOverride(Size availableSize)
        {
            var child = Child;
            if (child == null)
                return default(Size);

            child.Dispatcher.InvokeAsync(
                () => child.Measure(availableSize),
                DispatcherPriority.Loaded);

            return default(Size);
        }

        protected override Size ArrangeOverride(Size finalSize)
        {
            var child = Child;
            if (child == null)
                return finalSize;

            child.Dispatcher.InvokeAsync(
                () => child.Arrange(new Rect(finalSize)),
                DispatcherPriority.Loaded);

            return finalSize;
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
                if (Child is VideoElement view)
                {
                    view.SetState(videoState);
                    lock (_setVideoStateLock)
                    {
                        if (!videoState)
                        {
                            view.OnVideoRendered -= VideoViewContainer_OnVideoRendered;
                            view.OnVideoRendered += VideoViewContainer_OnVideoRendered;
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
            if (Child is VideoElement view)
            {
                view.OnVideoRendered -= VideoViewContainer_OnVideoRendered;
            }
            OnVideoRendered?.Invoke(this, EventArgs.Empty);
        }

        private void VideoView_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            _width = e.NewSize.Width;
            _height = e.NewSize.Height;
            if (this.Child is VideoElement frameworkElement)
            {
                frameworkElement.Dispatcher.InvokeAsync(() =>
                {
                    frameworkElement.Width = _width;
                    frameworkElement.Height = _height;
                    frameworkElement.UpdateViewPort((uint)_width, (uint)_height);
                }, DispatcherPriority.Send);
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
            _ = SetChildAsync(() =>
            {
                var videoView = new D3D11ImageElement();
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
            }, _dispatcher);
        }

        #endregion
    }
}
