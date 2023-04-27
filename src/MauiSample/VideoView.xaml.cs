using System.Runtime.InteropServices;
using MauiSample.Utils;
using Render.Source;
using SkiaSharp;
using SkiaSharp.Views.Maui.Controls;

namespace MauiSample;

public partial class VideoView : ContentView
{
	private IDispatcherTimer? _dispatcherTimer;
    private readonly IVideoSource _videoSource;
	private readonly VideoFrame _videoFrame = new();
	private readonly VideoFrameQueue _videoFrameQueue = new();
	private uint _width;
	private uint _height;
    private IntPtr _buffer;
    private SKBitmap? _sKBitmap;

    public VideoView()
	{
		_videoSource = VideoSourceFactory.GetVideoSource();
		_videoSource.VideoFrameEvent += VideoSource_VideoFrameEvent;
		this.SizeChanged += VideoView_SizeChanged;
		this.Unloaded += VideoView_Unloaded;
		InitializeComponent();
        skiaView.Loaded += GraphicsView_Loaded;
    }

    private void VideoView_Unloaded(object? sender, EventArgs e)
	{
		_videoSource.Close();
		_dispatcherTimer?.Stop();
        _sKBitmap?.Dispose();
    }

	private void VideoView_SizeChanged(object? sender, EventArgs e)
	{
		_videoSource.UpdateViewPort((uint)Width, (uint)Height);
	}

	private void VideoSource_VideoFrameEvent(object? sender, VideoFrameEventArg e)
	{
		_videoFrame.OnFrame(e.YPtr, e.YStride, e.UPtr, e.UStride, e.VPtr, e.VStride, e.Width, e.Height);
		_videoFrameQueue.Write(_videoFrame);
	}

    private void GraphicsView_Loaded(object? sender, EventArgs e)
    {
        _dispatcherTimer = Dispatcher.CreateTimer();
        _dispatcherTimer.Interval = TimeSpan.FromMilliseconds(40);
        _dispatcherTimer.Tick += _dispatcherTimer_Tick;
        _dispatcherTimer.Start();
    }

    private void _dispatcherTimer_Tick(object? sender, EventArgs e)
    {
        skiaView.InvalidateSurface();
    }

    private void OnRendering(SKCanvas canvas)
	{
        VideoFrame? videoFrame = _videoFrameQueue.Read();
        if (videoFrame is null)
        {
            return;
        }
        if (videoFrame.Width == 0 || videoFrame.Height == 0)
        {
            return;
        }
        if (_width != videoFrame.Width || _height != videoFrame.Height)
        {
            _width = videoFrame.Width;
            _height = videoFrame.Height;
            if (_buffer != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(_buffer);
            }
            _buffer = Marshal.AllocCoTaskMem((int)(_width * _height << 2));
            if (_sKBitmap is not null)
            {
				_sKBitmap.Dispose();
            }
            _sKBitmap = new SKBitmap((int)_width, (int)_height, SKColorType.Rgba8888, SKAlphaType.Premul);
        }
        if (_buffer == IntPtr.Zero)
        {
            return;
        }
        VideoFrameConverter.I420ToARGB(videoFrame.YPtr, _buffer, _width, _height);
        _sKBitmap?.SetPixels(_buffer);
        canvas.DrawBitmap(_sKBitmap, canvas.DeviceClipBounds);
        _videoFrameQueue.PutBack(videoFrame);
    }

    private void skiaView_PaintSurface(System.Object sender, SkiaSharp.Views.Maui.SKPaintSurfaceEventArgs e)
    {
        // the the canvas and properties
        SKCanvas? canvas = e.Surface.Canvas;

        if (canvas is null)
        {
            return;
        }

        // make sure the canvas is blank
        //canvas.Clear(SKColors.White);

        OnRendering(canvas);
    }
}
