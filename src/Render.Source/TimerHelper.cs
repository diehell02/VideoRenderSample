using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace Render.Source
{
    internal class TimerHelper
    {
        #region Internal Class
        #endregion

        #region Static
        #endregion

        #region Filed
        private readonly Stopwatch _stopwatch = new Stopwatch();
        private readonly uint _interval;
        private long _nextFrameStartTime;
        private long _nextFrameEndTime;
        #endregion

        #region Property
        public long Ticks => _stopwatch.ElapsedTicks;

        public long Milliseconds => _stopwatch.ElapsedMilliseconds;

        public bool IsRunning => _stopwatch.IsRunning;
        #endregion

        #region Constructor
        public TimerHelper() : this(0)
        {
        }

        public TimerHelper(uint interval)
        {
            _interval = interval * 10000;
        }
        #endregion

        #region Private Function
        #endregion

        #region Protect Function
        #endregion

        #region Override Function
        #endregion

        #region Public Function
        public void Start() => _stopwatch.Start();

        public void Stop() => _stopwatch.Stop();

        public bool VerifyInterval()
        {
            if (_interval == 0)
            {
                return false;
            }
            long renderingTime = Ticks;
            if (renderingTime < _nextFrameStartTime)
            {
                return false;
            }
            else if (renderingTime < _nextFrameEndTime)
            {
                _nextFrameStartTime = _nextFrameEndTime + 1;
                _nextFrameEndTime = _nextFrameStartTime + _interval;
            }
            else
            {
                long skipFrameCount = ((renderingTime - _nextFrameStartTime) / _interval) + 1;
                _nextFrameStartTime = _nextFrameEndTime + (skipFrameCount * _interval) + 1;
                _nextFrameEndTime = _nextFrameStartTime + _interval;
            }
            return true;
        }
        #endregion
    }
}
