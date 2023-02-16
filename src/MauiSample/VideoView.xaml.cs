using System.Runtime.InteropServices;
using MauiSample.Utils;
using Render.Source;

namespace MauiSample;

public partial class VideoView : ContentView
{
	private IDispatcherTimer? _dispatcherTimer;
	private readonly IVideoSource _videoSource;
	private readonly VideoFrame _videoFrame = new();
	private readonly VideoFrameQueue _videoFrameQueue = new();
	private uint _width;
	private uint _height;
	private byte[]? _buffer;
	private VideoDrawable? _videoDrawable;

	public VideoView()
	{
		_videoSource = VideoSourceFactory.GetVideoSource();
		_videoSource.VideoFrameEvent += VideoSource_VideoFrameEvent;
		this.SizeChanged += VideoView_SizeChanged;
		this.Unloaded += VideoView_Unloaded;
		InitializeComponent();
		graphicsView.Loaded += GraphicsView_Loaded;
		graphicsView.Unloaded += GraphicsView_Unloaded;
		if (Resources.TryGetValue("drawable", out object drawable))
		{
			_videoDrawable = drawable as VideoDrawable;
		}
	}

	private void VideoView_Unloaded(object? sender, EventArgs e)
	{
		_videoSource.Close();
		_dispatcherTimer?.Stop();
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

	private void GraphicsView_Unloaded(object? sender, EventArgs e)
	{
		_dispatcherTimer?.Stop();
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
			_buffer = new byte[_width * _height * 3 / 2];
		}
		if (_buffer is null)
		{
			return;
		}
		Marshal.Copy(videoFrame.YPtr, _buffer, 0, _buffer.Length);
		_videoFrameQueue.PutBack(videoFrame);
		_videoDrawable?.DrawVideoFrame(_buffer, _width, _height);
		graphicsView.Invalidate();
	}
}
