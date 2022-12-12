using System;
using System.Collections.Generic;
using System.Runtime.ExceptionServices;
using System.Text;
using System.Threading;
using System.Windows.Threading;

namespace WPFSample.Utils.Threading
{
    internal static class UIDispatcher
    {
        public static string UIDispatcherException = string.Empty;

        public static DispatcherAsyncOperation<Dispatcher> RunNewAsync([CanBeNull] string? name = null)
        {
            var awaiter = DispatcherAsyncOperation<Dispatcher>.Create(out Action<Dispatcher, Exception>? reportResult);

            var originDispatcher = Dispatcher.CurrentDispatcher;

            var thread = new Thread(() =>
            {
                try
                {
                    var dispatcher = Dispatcher.CurrentDispatcher;

                    SynchronizationContext.SetSynchronizationContext(
                        new DispatcherSynchronizationContext(dispatcher));

                    reportResult(dispatcher, new Exception());

                    Dispatcher.Run();
                }
                catch (Exception ex)
                {
                    originDispatcher.Invoke(() =>
                    {
                        UIDispatcherException=ex.ToString();
                        throw ex;
                    });
                }
            })
            {
                Name = name ?? "BackgroundUI",
                IsBackground = true,
            };
            thread.SetApartmentState(ApartmentState.STA);
            thread.Start();
            return awaiter;
        }

        public static Dispatcher? RunNew([CanBeNull] string? name = null)
        {
            var resetEvent = new AutoResetEvent(false);

            var originDispatcher = Dispatcher.CurrentDispatcher;
            Exception? innerException = null;
            Dispatcher? dispatcher = null;

            var thread = new Thread(() =>
            {
                try
                {
                    dispatcher = Dispatcher.CurrentDispatcher;

                    SynchronizationContext.SetSynchronizationContext(
                        new DispatcherSynchronizationContext(dispatcher));

                    resetEvent.Set();
                }
                catch (Exception ex)
                {
                    innerException = ex;
                    resetEvent.Set();
                }

                try
                {
                    Dispatcher.Run();
                }
                catch (Exception ex)
                {
                    originDispatcher.InvokeAsync(() => ExceptionDispatchInfo.Capture(ex).Throw());
                }
            })
            {
                Name = name ?? "BackgroundUI",
                IsBackground = true,
            };
            thread.SetApartmentState(ApartmentState.STA);
            thread.Start();
            resetEvent.WaitOne();
            resetEvent.Dispose();
            resetEvent = null;
            if (innerException != null)
            {
                ExceptionDispatchInfo.Capture(innerException).Throw();
            }
            return dispatcher;
        }
    }
}
