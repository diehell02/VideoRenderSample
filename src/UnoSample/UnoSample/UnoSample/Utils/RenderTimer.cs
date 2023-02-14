using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Windows.UI.Core;

namespace UnoSample.Utils
{
    internal class RenderTimer
    {
        #region Internal Class

        private class RenderingHelper
        {
            private const int TEN_SECONDS = 10000;
            private const int ONE_SECONDS = 1000;

            readonly List<Action> _actions = new();
            readonly Stopwatch _stopwatch = new();
            long _lastRenderingMilliseconds;
            decimal _fps;
            readonly decimal _targetFps = 25;

            private long _renderingCallbackCount = 0;
            private long _renderingCount = 0;
            private uint _ratio = (uint)RenderIndexDictionary.Map.Count;

            public bool IsEmpty => _actions.Count == 0;

            public void AddCallback(Action callback)
            {
                if (_actions.Contains(callback))
                {
                    return;
                }
                if (_actions.Count == 0)
                {
                    StartRendering();
                }
                _actions.Add(callback);
            }

            public void RemoveCallback(Action callback)
            {
                if (!_actions.Contains(callback))
                {
                    return;
                }
                _actions.Remove(callback);
                if (_actions.Count == 0)
                {
                    StopRendering();
                }
            }

            private void StartRendering()
            {
                DispatcherManager.AddRenderingListener(Rendering);
                _stopwatch.Start();
            }

            private void StopRendering()
            {
                _stopwatch.Stop();
                DispatcherManager.RemoveRenderingListener(Rendering);
            }

            private void FillFrameToVideo()
            {
                _renderingCount++;
                foreach (var action in _actions)
                {
                    action.Invoke();
                }
            }

            private void Rendering()
            {
                if (_actions == null)
                {
                    return;
                }

                long currentRenderingMilliseconds = _stopwatch.ElapsedMilliseconds;
                long elapsed = currentRenderingMilliseconds - _lastRenderingMilliseconds;
                if (currentRenderingMilliseconds - _lastRenderingMilliseconds > TEN_SECONDS)
                {
                    _fps = (decimal)_renderingCallbackCount / elapsed * ONE_SECONDS;
                    var renderingFPS = (decimal)_renderingCount / elapsed * ONE_SECONDS;
                    Trace.WriteLine($"[{Environment.CurrentManagedThreadId}] render fps:{renderingFPS}, callback fps:{_fps}");
                    _renderingCount = 1;
                    _renderingCallbackCount = 1;
                    _lastRenderingMilliseconds = currentRenderingMilliseconds;
                    _ratio = (uint)RenderIndexDictionary.Map.Count;
                    if (_targetFps < _fps)
                    {
                        _ratio = (uint)Math.Round(_targetFps / _fps * RenderIndexDictionary.Map.Count);
                    }
                }
                else
                {
                    _renderingCallbackCount++;
                }

                int index = ((int)((_renderingCallbackCount - 1) % RenderIndexDictionary.Map.Count));
                if (RenderIndexDictionary.Map.TryGetValue(_ratio - 1, out var value))
                {
                    if (value != null && value.Length > index && value[index] == 1)
                    {
                        FillFrameToVideo();
                    }
                }
            }
        }

        #endregion

        #region Static

        public static RenderTimer Instance { get; private set; }

        static RenderTimer()
        {
            Instance = new RenderTimer();
        }

        #endregion

        #region Private Fields

        private readonly Dictionary<int, RenderingHelper> _renderingDic = new();
        private static readonly object _lock = new();

        #endregion

        #region Public Properies

        #endregion

        #region Constructor

        private RenderTimer()
        {
        }

        #endregion Constructor

        #region Public Functions

        public void AddListener(Action action)
        {
            lock (_lock)
            {
                int threadId = Environment.CurrentManagedThreadId;
                if (!_renderingDic.TryGetValue(threadId, out RenderingHelper? renderingHelper))
                {
                    renderingHelper = new RenderingHelper();
                    _renderingDic.Add(threadId, renderingHelper);
                }
                renderingHelper.AddCallback(action);
            }
        }

        public void RemoveListener(Action action)
        {
            lock (_lock)
            {
                int threadId = Environment.CurrentManagedThreadId;
                if (_renderingDic.TryGetValue(threadId, out RenderingHelper? renderingHelper))
                {
                    renderingHelper.RemoveCallback(action);
                    if (renderingHelper.IsEmpty)
                    {
                        _renderingDic.Remove(threadId);
                    }
                }
            }
        }

        #endregion Public Functions
    }
}
