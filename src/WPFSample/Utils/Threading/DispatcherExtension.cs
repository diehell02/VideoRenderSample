// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Threading;

namespace WPFSample.Utils.Threading
{
    internal static class DispatcherExtension
    {
        public static async Task SafeInvokeAsync(this Dispatcher dispatcher, Action action, DispatcherPriority priority, CancellationToken cancellationToken)
        {
            if (dispatcher.CheckAccess())
            {
                action.Invoke();
            }
            else
            {
                await dispatcher.InvokeAsync(action, priority, cancellationToken);
            }
        }

        public static async Task SafeInvokeAsync(this Dispatcher dispatcher, Action action, DispatcherPriority priority)
        {
            if (dispatcher.CheckAccess())
            {
                action.Invoke();
            }
            else
            {
                await dispatcher.InvokeAsync(action, priority);
            }
        }

        public static async Task SafeInvokeAsync(this Dispatcher dispatcher, Action action)
        {
            if (dispatcher.CheckAccess())
            {
                action.Invoke();
            }
            else
            {
                await dispatcher.InvokeAsync(action);
            }
        }

        public static async Task<T> SafeInvokeAsync<T>(this Dispatcher dispatcher, Func<T> callback, DispatcherPriority priority, CancellationToken cancellationToken)
        {
            if (dispatcher.CheckAccess())
            {
                return callback.Invoke();
            }
            else
            {
                return await dispatcher.InvokeAsync(callback, priority, cancellationToken);
            }
        }

        public static async Task<T> SafeInvokeAsync<T>(this Dispatcher dispatcher, Func<T> callback, DispatcherPriority priority)
        {
            if (dispatcher.CheckAccess())
            {
                return callback.Invoke();
            }
            else
            {
                return await dispatcher.InvokeAsync(callback, priority);
            }
        }

        public static async Task<T> SafeInvokeAsync<T>(this Dispatcher dispatcher, Func<T> callback)
        {
            if (dispatcher.CheckAccess())
            {
                return callback.Invoke();
            }
            else
            {
                return await dispatcher.InvokeAsync(callback);
            }
        }
    }
}
