using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace Render.Source
{
    internal class VideoSource : IVideoSource
    {
        #region Static

        static VideoSource()
        {
            ThreadPool.GetMinThreads(out int workerThreads, out int completionPortThreads);
            if (Config.Instance.VideoViewNumber > workerThreads)
            {
                ThreadPool.SetMinThreads(Config.Instance.VideoViewNumber, completionPortThreads);
            }
        }

        public static VideoSource Create() => new VideoSource();

        #endregion

        #region IVideoSource

        public event EventHandler<VideoFrameEventArg>? VideoFrameEvent;

        public void UpdateViewPort(uint width, uint height)
        {
            if (width == 0 || height == 0)
            {
                return;
            }
            YuvFile? file = GetYuvFile(width, height);
            if (file is null)
            {
                return;
            }
            _fileQueue.Enqueue(file);
        }

        public void Close()
        {
            VideoFrameEvent = null;
            _cancellationTokenSource.Cancel();
            _cancellationTokenSource.Dispose();
        }

        #endregion

        #region Field

        private const int DEFAULT_SLEEP_TIME = 1000;
        private readonly ConcurrentQueue<YuvFile> _fileQueue = new ConcurrentQueue<YuvFile>();
        private readonly CancellationTokenSource _cancellationTokenSource = new CancellationTokenSource();
        private readonly int _frameRate;

        #endregion

        #region Constructor

        public VideoSource()
        {
            _frameRate = (int)(1000f / Config.Instance.FramePerSecond);
            _cancellationTokenSource = new CancellationTokenSource();
            ThreadPool.QueueUserWorkItem(new WaitCallback(DoFrame), _cancellationTokenSource);
        }

        #endregion

        #region Private Function

        private YuvFile? GetYuvFile(uint width, uint height)
        {
            var files = Config.Instance.YuvFiles;
            if (files is null)
            {
                return null;
            }
            foreach (var file in files)
            {
                uint area = width * height;
                if (area <= file.FrameWidth * file.FrameHeight)
                {
                    return file;
                }
            }
            return files.Last();
        }

        private void DoFrame(object? obj)
        {
            if (obj is CancellationTokenSource cts)
            {
                YuvFile? yuvFile = null;
                int index = 0;
                int ySize = 0;
                int uvSize = 0;
                VideoFrameEventArg eventArg = new VideoFrameEventArg();
                TimerHelper timerHelper = new TimerHelper();
                timerHelper.Start();
                long lastTime = 0;
                while (!cts.IsCancellationRequested)
                {
                    _fileQueue.TryDequeue(out YuvFile? newFile);
                    if (!(newFile is null))
                    {
                        yuvFile = newFile;
                        ySize = yuvFile.FrameWidth * yuvFile.FrameHeight;
                        uvSize = ySize >> 2;
                        eventArg.YStride = (uint)yuvFile.FrameWidth;
                        eventArg.UStride = eventArg.VStride = eventArg.YStride >> 1;
                        eventArg.Width = (uint)yuvFile.FrameWidth;
                        eventArg.Height = (uint)yuvFile.FrameHeight;
                    }
                    if (yuvFile is null)
                    {
                        Thread.Sleep(DEFAULT_SLEEP_TIME);
                        continue;
                    }
                    if (index >= yuvFile.FrameLength)
                    {
                        index = 0;
                    }
                    eventArg.YPtr = yuvFile.GetFrame(index++);
                    eventArg.UPtr = eventArg.YPtr + ySize;
                    eventArg.VPtr = eventArg.UPtr + uvSize;
                    VideoFrameEvent?.Invoke(this, eventArg);
                    long ElapsedTime = (int)(timerHelper.Milliseconds - lastTime);
                    lastTime = timerHelper.Milliseconds;
                    int millisecondsTimeout = (int)(_frameRate - ElapsedTime) + _frameRate;
                    if (millisecondsTimeout > 0)
                    {
                        Thread.Sleep(millisecondsTimeout);
                    }
                }
                timerHelper.Stop();
            }
        }

        #endregion
    }
}
