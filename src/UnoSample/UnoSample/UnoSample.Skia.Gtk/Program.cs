// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using GLib;
using Uno.UI.Runtime.Skia;

namespace UnoSample.Skia.Gtk
{
    public sealed class Program
    {
        static void Main(string[] args)
        {
            ExceptionManager.UnhandledException += delegate (UnhandledExceptionArgs expArgs)
            {
                Console.WriteLine("GLIB UNHANDLED EXCEPTION" + expArgs.ExceptionObject.ToString());
                expArgs.ExitApplication = true;
            };

            var host = new GtkHost(() => new AppHead());

            host.Run();
        }
    }
}
