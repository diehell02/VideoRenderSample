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
    internal class DispatcherManager
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
                try
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
                catch (Exception ex)
                {
                    Trace.WriteLine(ex.ToString());
                }
            }

            public void AddListener(Action? action)
            {
                try
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
                catch (Exception ex)
                {
                    Trace.WriteLine(ex.ToString());
                }
            }

            public void RemoveListener(Action? action)
            {
                try
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
                catch (Exception ex)
                {
                    Trace.WriteLine(ex.ToString());
                }
            }
        }

        private const string RENDER_THREAD_NAME = "Render Thread";
        private const int MAX_DISPATCHER_NUMBER = 16;

        private static readonly Dictionary<int, RenderingEventSource> s_threadActionDic = new();
        private static readonly object s_lock = new();
        private static readonly Dictionary<Dispatcher, int> s_dispatcherDic= new();

        public static Dispatcher? CreateRenderDispatcher()
        {
            try
            {
                if (s_dispatcherDic.Count > MAX_DISPATCHER_NUMBER)
                {
                    int minNumber = int.MaxValue;
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
                    if (s_dispatcherDic.TryGetValue(dispatcher, out int number))
                    {
                        number++;
                        s_dispatcherDic[dispatcher] = number;
                    }
                    return dispatcher;
                }
                else
                {
                    var render = UIDispatcher.RunNew(RENDER_THREAD_NAME + $"#{s_dispatcherDic.Count}");
                    if (render != null)
                    {
                        s_dispatcherDic.Add(render, 1);
                        return render;
                    }
                }
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.ToString());
            }
            return null;
        }

        public static void ReleaseRenderDispatcher(Dispatcher? dispatcher)
        {
            try
            {
                if (dispatcher is null)
                {
                    return;
                }
                dispatcher.InvokeShutdown();
                if (s_dispatcherDic.TryGetValue(dispatcher, out int number))
                {
                    if (number > 1)
                    {
                        number--;
                        s_dispatcherDic[dispatcher] = number;
                    }
                    else
                    {
                        s_dispatcherDic.Remove(dispatcher);
                    }
                }
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.ToString());
            }
        }

        public static void AddRenderingListener(Action action)
        {
            try
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
            catch (Exception ex)
            {
                Trace.WriteLine(ex.ToString());
            }
        }

        public static void RemoveRenderingListener(Action action)
        {
            try
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
            catch (Exception ex)
            {
                Trace.WriteLine(ex.ToString());
            }
        }
    }
}
