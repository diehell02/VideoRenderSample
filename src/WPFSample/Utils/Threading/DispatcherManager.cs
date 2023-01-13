using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Threading;

namespace WPFSample.Utils.Threading
{
    internal static class DispatcherManager
    {
        private class RenderingEventSource
        {
            private readonly int _threadId;
            private readonly List<Action?> _actions = new();
            private TimeSpan _lastRenderTime;

            public int ListenerCount => _actions.Count;

            public RenderingEventSource(int threadId)
            {
                _threadId = threadId;
            }

            private void CompositionTarget_Rendering(object? sender, EventArgs e)
            {
                RenderingEventArgs args = (RenderingEventArgs)e;
                if (_lastRenderTime == args.RenderingTime)
                {
                    return;
                }
                _lastRenderTime = args.RenderingTime;
                foreach (Action? action in _actions)
                {
                    action?.Invoke();
                }
            }

            public void AddListener(Action? action)
            {
                if (action is null)
                {
                    return;
                }
                if (_actions.Count == 0)
                {
                    CompositionTarget.Rendering += CompositionTarget_Rendering;
                }
                _actions.Add(action);
            }

            public void RemoveListener(Action? action)
            {
                if (action is null)
                {
                    return;
                }
                _actions.Remove(action);
                if (_actions.Count == 0)
                {
                    CompositionTarget.Rendering -= CompositionTarget_Rendering;
                }
            }
        }

        private const string RENDER_THREAD_NAME = "Render Thread";

        private static readonly int s_maxDispatcherNumber = Environment.ProcessorCount;
        private static readonly Dictionary<int, RenderingEventSource> s_threadActionDic = new();
        private static readonly object s_lock = new();
        private static readonly Dictionary<Dispatcher, uint> s_dispatcherDic= new();

        static DispatcherManager()
        {

        }

        public static Dispatcher? CreateRenderDispatcher()
        {
            if (s_dispatcherDic.Count > s_maxDispatcherNumber)
            {
                uint minNumber = int.MaxValue;
                Dispatcher? dispatcher = null;
                foreach (var keyValuePair in s_dispatcherDic)
                {
                    if (minNumber > keyValuePair.Value)
                    {
                        minNumber = keyValuePair.Value;
                        dispatcher = keyValuePair.Key;
                    }
                }
                if (dispatcher is null)
                {
                    return null;
                }
                if (s_dispatcherDic.TryGetValue(dispatcher, out uint number))
                {
                    number++;
                    s_dispatcherDic[dispatcher] = number;
                }
                return dispatcher;
            }
            else
            {
                Dispatcher? render = UIDispatcher.RunNew(RENDER_THREAD_NAME + $"#{s_dispatcherDic.Count}");
                if (render != null)
                {
                    s_dispatcherDic.Add(render, 1);
                    return render;
                }
            }
            return null;
        }

        public static void ReleaseRenderDispatcher(Dispatcher? dispatcher)
        {
            if (dispatcher is null)
            {
                return;
            }
            if (s_dispatcherDic.TryGetValue(dispatcher, out uint number))
            {
                number--;
                s_dispatcherDic[dispatcher] = number;
            }
        }

        public static void AddRenderingListener(Action action)
        {
            lock (s_lock)
            {
                int threadId = Environment.CurrentManagedThreadId;
                if (s_threadActionDic.TryGetValue(threadId, out RenderingEventSource? renderingEventSource))
                {
                    renderingEventSource?.AddListener(action);
                    return;
                }
                renderingEventSource = new RenderingEventSource(threadId);
                s_threadActionDic.Add(threadId, renderingEventSource);
                renderingEventSource.AddListener(action);
            }
        }

        public static void RemoveRenderingListener(Action action)
        {
            lock (s_lock)
            {
                int threadId = Environment.CurrentManagedThreadId;
                if (s_threadActionDic.TryGetValue(threadId, out RenderingEventSource? renderingEventSource))
                {
                    if (renderingEventSource != null)
                    {
                        renderingEventSource.RemoveListener(action);
                        if (renderingEventSource.ListenerCount == 0)
                        {
                            s_threadActionDic.Remove(threadId);
                        }
                        return;
                    }
                }
            }
        }
    }
}
