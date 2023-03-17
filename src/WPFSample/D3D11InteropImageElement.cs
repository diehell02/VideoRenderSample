// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Interop;

namespace WPFSample
{
    internal class D3D11InteropImageElement : VideoElement
    {
        public D3D11InteropImageElement()
        {
            IntPtr hwnd = IntPtr.Zero;
            Application.Current.Dispatcher.Invoke(() =>
            {
                WindowInteropHelper windowInteropHelper = new WindowInteropHelper(Application.Current.MainWindow);
                hwnd = windowInteropHelper.Handle;
            });
            _render = new D3D11InteropImageSource(hwnd);
        }

        public D3D11InteropImageElement(IntPtr hwnd)
        {
            _render = new D3D11InteropImageSource(hwnd);
        }
    }
}
