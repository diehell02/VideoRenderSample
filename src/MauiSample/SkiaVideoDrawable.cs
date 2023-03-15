// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Reflection.Metadata;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Maui.Graphics.Skia;
using Render.Source;
using SkiaSharp;

namespace MauiSample
{
    internal class SkiaVideoDrawable : IDrawable, IDisposable, IVideoDrawable
    {
        private SKBitmap? _sKBitmap;
        private SkiaImage? _skiaImage;
        private Microsoft.Maui.Graphics.IImage? _image;
        private uint _width;
        private uint _height;
        private byte[]? _buffer;

        public event EventHandler? ImageCreated;

        public Microsoft.Maui.Graphics.IImage? Image => _image;

        public void Draw(ICanvas canvas, RectF dirtyRect)
        {
            if (_image is null)
            {
                return;
            }
            canvas?.DrawImage(_image, dirtyRect.X, dirtyRect.Y, dirtyRect.Width, dirtyRect.Height);
        }

        private void EnsureBitmap(uint width, uint height)
        {
            bool needResize = false;
            if (_width != width)
            {
                needResize = true;
                _width = width;
            }
            if (_height != height)
            {
                needResize = true;
                _height = height;
            }
            if (_sKBitmap is null)
            {
                _sKBitmap = SKBitmap.Decode(@"C:\\Users\\cade.huang\\Pictures\\01bdce574f9e1c6ac72525aef3d5be.jpg@3000w_1l_0o_100sh.jpg");
                //_sKBitmap = new SKBitmap((int)3000, (int)2000, SKColorType.Bgra8888, SKAlphaType.Unpremul);
                //var bytes = File.ReadAllBytes(@"C:\\Users\\cade.huang\\Pictures\\01bdce574f9e1c6ac72525aef3d5be.jpg@3000w_1l_0o_100sh.jpg");
                //unsafe
                //{
                //    fixed (byte* p = bytes)
                //    {
                //        _sKBitmap?.SetPixels((nint)p);
                //    }
                //}
            }
            else if (needResize)
            {
                _sKBitmap.Resize(new SKImageInfo((int)width, (int)height), SKFilterQuality.None);
            }
            if (_skiaImage is null)
            {
                _skiaImage = new SkiaImage(_sKBitmap);
                _image = _skiaImage;
                ImageCreated?.Invoke(this, EventArgs.Empty);
            }
            else if (needResize)
            {
                _skiaImage.Resize(width, height);
            }
            if (needResize)
            {
                _buffer = new byte[width * height << 2];
            }
        }

        public void DrawVideoFrame(byte[] yuvFrame, uint width, uint height)
        {
            EnsureBitmap(width, height);
            byte[] pixels = new byte[width * height * 4];
            VideoFrameConverter.YUV2RGBA(yuvFrame, pixels, width, height);

            //unsafe
            //{
            //    fixed (byte* p = pixels)
            //    {
            //        _sKBitmap?.SetPixels((nint)p);
            //    }
            //}
        }

        #region IDisposable

        private bool _disposed = false;

        // Use C# finalizer syntax for finalization code.
        // This finalizer will run only if the Dispose method
        // does not get called.
        // It gives your base class the opportunity to finalize.
        // Do not provide finalizer in types derived from this class.
        ~SkiaVideoDrawable()
        {
            // Do not re-create Dispose clean-up code here.
            // Calling Dispose(disposing: false) is optimal in terms of
            // readability and maintainability.
            Dispose(disposing: false);
        }

        // Implement IDisposable.
        // Do not make this method virtual.
        // A derived class should not be able to override this method.
        public void Dispose()
        {
            Dispose(disposing: true);
            // This object will be cleaned up by the Dispose method.
            // Therefore, you should call GC.SuppressFinalize to
            // take this object off the finalization queue
            // and prevent finalization code for this object
            // from executing a second time.
            GC.SuppressFinalize(this);
        }

        // Dispose(bool disposing) executes in two distinct scenarios.
        // If disposing equals true, the method has been called directly
        // or indirectly by a user's code. Managed and unmanaged resources
        // can be disposed.
        // If disposing equals false, the method has been called by the
        // runtime from inside the finalizer and you should not reference
        // other objects. Only unmanaged resources can be disposed.
        protected virtual void Dispose(bool disposing)
        {
            // Check to see if Dispose has already been called.
            if (!_disposed)
            {
                // If disposing equals true, dispose all managed
                // and unmanaged resources.
                if (disposing)
                {
                    // Dispose managed resources.
                    _skiaImage?.Dispose();
                    _sKBitmap?.Dispose();
                }

                // Note disposing has been done.
                _disposed = true;
            }
        }

        #endregion
    }
}
