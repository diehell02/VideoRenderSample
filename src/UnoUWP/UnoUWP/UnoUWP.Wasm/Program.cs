// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using Windows.UI.Xaml;

namespace UnoUWP.Wasm
{
    public sealed class Program
    {
        private static App _app;

        static int Main(string[] args)
        {
            Windows.UI.Xaml.Application.Start(_ => _app = new App());

            return 0;
        }
    }
}
