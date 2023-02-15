// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Tizen.Applications;
using Uno.UI.Runtime.Skia;

namespace UnoUWP.Skia.Tizen
{
    public sealed class Program
    {
        static void Main(string[] args)
        {
            var host = new TizenHost(() => new UnoUWP.App());
            host.Run();
        }
    }
}
