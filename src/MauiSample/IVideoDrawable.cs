// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MauiSample
{
    internal interface IVideoDrawable
    {
        void DrawVideoFrame(byte[] yuvFrame, uint width, uint height);
    }
}
