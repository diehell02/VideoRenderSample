// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using AppKit;

namespace UnoSample
{
    // This is the main entry point of the application.
    public class EntryPoint
    {
        static void Main(string[] args)
        {
            NSApplication.Init();
            NSApplication.SharedApplication.Delegate = new AppHead();
            NSApplication.Main(args);
        }
    }
}

