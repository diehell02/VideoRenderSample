using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.UI.Core;
using System.Threading;
using System.Diagnostics;
using Windows.UI.Xaml.Media;

namespace UnoUWP.Utils
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

            private void CompositionTarget_Rendering(object? sender, object e)
            {
                try
                {
                    if (e is RenderingEventArgs args)
                    {
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
                }
                catch (Exception ex)
                {
                    Trace.WriteLine(ex);
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
                    Trace.WriteLine(ex);
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
                    Trace.WriteLine(ex);
                }
            }
        }

        private static readonly Dictionary<int, RenderingEventSource> s_dispatcherActionDic = new();
        private static readonly object s_lock = new();

        public static void AddRenderingListener(Action action)
        {
            try
            {
                lock (s_lock)
                {
                    int threadId = Environment.CurrentManagedThreadId;
                    if (s_dispatcherActionDic.TryGetValue(threadId, out RenderingEventSource? renderingEventSource))
                    {
                        renderingEventSource?.AddListener(action);
                        return;
                    }
                    renderingEventSource = new RenderingEventSource(threadId);
                    s_dispatcherActionDic.Add(threadId, renderingEventSource);
                    renderingEventSource.AddListener(action);
                }
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex);
            }
        }

        public static void RemoveRenderingListener(Action action)
        {
            try
            {
                lock (s_lock)
                {
                    int threadId = Environment.CurrentManagedThreadId;
                    if (s_dispatcherActionDic.TryGetValue(threadId, out RenderingEventSource? renderingEventSource))
                    {
                        if (renderingEventSource != null)
                        {
                            renderingEventSource.RemoveListener(action);
                            if (renderingEventSource.ListenerCount == 0)
                            {
                                s_dispatcherActionDic.Remove(threadId);
                            }
                            return;
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex);
            }
        }
    }
}
