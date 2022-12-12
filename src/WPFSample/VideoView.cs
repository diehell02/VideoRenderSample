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

namespace WPFSample
{
    public class VideoView : FrameworkElement
    {
        #region DependencyProperty

        public static readonly DependencyProperty AutoResizeProperty =
            DependencyProperty.Register(
                nameof(AutoResize),
                typeof(bool),
                typeof(VideoView));

        public static readonly DependencyProperty StretchProperty =
            DependencyProperty.Register(
                nameof(Stretch),
                typeof(Stretch),
                typeof(VideoView),
                new PropertyMetadata(Stretch.Uniform, (d, e) =>
                {
                    if (d is VideoView videoView && videoView.Child is VideoElement videoElement)
                    {
                        _ = videoElement.Dispatcher.InvokeAsync(() =>
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
        private bool _isCleaned = false;
        private object _lockObj = new object();
        private Dispatcher? _dispatcher;
        private bool _isCreated = false;
        private bool _hasVideo = false;
        private double _width = 0;
        private double _height = 0;

        #endregion

        #region Constructor

        public VideoView()
        {
            _hostVisual = new HostVisual();
            this.Loaded += (sender, e) =>
            {
                if (_isCreated)
                {
                    return;
                }

                _isCreated = true;

                Window window = Window.GetWindow(this);
                if (window is null)
                {
                    return;
                }
                _dispatcher = DispatcherManager.CreateRenderDispatcher();
                Stretch stretch = this.Stretch;
                if (_dispatcher != null)
                {
                    bool autoResize = AutoResize;
                    _ = SetChildAsync(() =>
                    {
                        var videoView = new WriteableBitmapElement(autoResize);
                        if (videoView is VideoElement videoElement)
                        {
                            videoElement.Stretch = stretch;
                        }
                        videoView.HorizontalAlignment = HorizontalAlignment.Center;
                        videoView.VerticalAlignment = VerticalAlignment.Center;
                        videoView.Width = _width;
                        videoView.Height = _height;
                        videoView.UpdateViewPort((uint)_width, (uint)_height);
                        return videoView;
                    }, _dispatcher);
                }
            };
            this.Unloaded += (sender, e) =>
            {
                _ = Clean();
            };
            SizeChanged += VideoView_SizeChanged;
            IsHitTestVisible = false;
        }

        #endregion

        #region Child

        private bool _isUpdatingChild;
        private UIElement? _child;
        public UIElement? Child => _child;

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
            var child = await dispatcher.InvokeAsync(@new);
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
                var oldChild = _child;
                var visualTarget = _targetSource;

                if (Equals(oldChild, value))
                    return;

                _targetSource = null;
                if (visualTarget != null)
                {
                    RemoveVisualChild(_hostVisual);
                    await visualTarget.Dispatcher.InvokeAsync(visualTarget.Dispose);
                }

                _child = value;
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

        protected override int VisualChildrenCount => _child != null ? 1 : 0;

        protected override Size MeasureOverride(Size availableSize)
        {
            var child = _child;
            if (child == null)
                return default(Size);

            child.Dispatcher.InvokeAsync(
                () => child.Measure(availableSize),
                DispatcherPriority.Loaded);

            return default(Size);
        }

        protected override Size ArrangeOverride(Size finalSize)
        {
            var child = _child;
            if (child == null)
                return finalSize;

            child.Dispatcher.InvokeAsync(
                () => child.Arrange(new Rect(finalSize)),
                DispatcherPriority.Loaded);

            return finalSize;
        }

        #endregion

        #region IVideoView

        public async Task Clean()
        {
            Trace.WriteLine("Enter");
            try
            {
                Visibility = Visibility.Collapsed;
                OnVideoRendered = null;
                await SetChildAsync(null);
                lock (_lockObj)
                {
                    _isCleaned = true;
                    VideoElement?.Clean();
                    DispatcherManager.ReleaseRenderDispatcher(_dispatcher);
                    _dispatcher = null;
                }
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex);
            }
            Trace.WriteLine("Leave");
        }

        #endregion

        #region Function

        public void SetHasVideo(bool hasVideo)
        {
            Trace.WriteLine($"hasVideo: {hasVideo}");
            if (!hasVideo)
            {
                if (Child is VideoElement view)
                {
                    view.OnVideoRendered -= VideoViewContainer_OnVideoRendered;
                    view.OnVideoRendered += VideoViewContainer_OnVideoRendered;
                }
            }
        }

        private void VideoViewContainer_OnVideoRendered(object? sender, EventArgs e)
        {
            try
            {
                Dispatcher.InvokeAsync(HandleVideoRendered, DispatcherPriority.Send);
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.ToString());
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

        #endregion
    }
}
