using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Text;

namespace WPFSample.Utils.Threading
{
    internal interface IAwaitable<out TAwaiter> where TAwaiter : IAwaiter
    {
        TAwaiter GetAwaiter();
    }

    internal interface IAwaitable<out TAwaiter, out TResult> where TAwaiter : IAwaiter<TResult> where TResult : class
    {
        TAwaiter GetAwaiter();
    }

    internal interface IAwaiter : INotifyCompletion
    {
        bool IsCompleted { get; }

        void GetResult();
    }

    internal interface ICriticalAwaiter : IAwaiter, ICriticalNotifyCompletion
    {
    }

    internal interface IAwaiter<out TResult> : INotifyCompletion where TResult : class
    {
        bool IsCompleted { get; }

        TResult? GetResult();
    }

    internal interface ICriticalAwaiter<out TResult> : IAwaiter<TResult>, ICriticalNotifyCompletion where TResult : class
    {
    }
}
