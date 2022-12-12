using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.ExceptionServices;
using System.Text;
using System.Windows.Threading;

namespace WPFSample.Utils.Threading
{
    internal class DispatcherAsyncOperation<T> : DispatcherObject,
        IAwaitable<DispatcherAsyncOperation<T>, T>, IAwaiter<T> where T: class
    {
        private DispatcherAsyncOperation()
        {
        }

        public DispatcherAsyncOperation<T> GetAwaiter()
        {
            return this;
        }

        public bool IsCompleted { get; private set; }

        public T? Result { get; private set; }

        public T? GetResult()
        {
            if (_exception != null)
            {
                ExceptionDispatchInfo.Capture(_exception).Throw();
            }
            return Result;
        }

        public DispatcherAsyncOperation<T> ConfigurePriority(DispatcherPriority priority)
        {
            _priority = priority;
            return this;
        }

        public void OnCompleted(Action continuation)
        {
            if (IsCompleted)
            {
                continuation?.Invoke();
            }
            else
            {
                _continuation += continuation;
            }
        }

        private void ReportResult(T result, Exception exception)
        {
            Result = result;
            _exception = exception;
            IsCompleted = true;

            if (_continuation != null)
            {
                Dispatcher.InvokeAsync(_continuation, _priority);
            }
        }

        private Action? _continuation;

        private Exception? _exception;

        private DispatcherPriority _priority = DispatcherPriority.Normal;

        public static DispatcherAsyncOperation<T> Create([NotNull] out Action<T, Exception> reportResult)
        {
            var asyncOperation = new DispatcherAsyncOperation<T>();
            reportResult = asyncOperation.ReportResult;
            return asyncOperation;
        }
    }
}
