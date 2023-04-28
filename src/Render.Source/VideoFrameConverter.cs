// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Render.Source
{
    public class VideoFrameConverter
    {
        #region C#

        const byte FF = 0xFF;

        // This snippet is based almost entirely on Neil Townsend's answer at: https://stackoverflow.com/questions/9325861/converting-yuv-rgbimage-processing-yuv-during-onpreviewframe-in-android

        public static void YUV2RGBA(IntPtr source, IntPtr dest, uint imageWidth, uint imageHeight)
        {
            int index = 0;
            uint ySize = imageWidth * imageHeight;
            uint uSize = (imageWidth * imageHeight >> 2);
            uint uStride = imageWidth >> 1;

            // Get each pixel, one at a time
            for (int y = 0; y < imageHeight; y++)
            {
                for (int x = 0; x < imageWidth; x++)
                {
                    // Get the Y value, stored in the first block of data
                    // The logical "AND 0xff" is needed to deal with the signed issue
                    byte Y = Marshal.ReadByte(source, (int)(y * imageWidth + x));

                    // Get U and V values, stored after Y values, one per 2x2 block
                    // of pixels, interleaved. Prepare them as floats with correct range
                    // ready for calculation later.
                    int xby2 = x >> 1;
                    int yby2 = y >> 1;
                    long offset = yby2 * uStride;
                    byte U = Marshal.ReadByte(source, (int)(ySize + offset + xby2));
                    byte V = Marshal.ReadByte(source, (int)(ySize + uSize + offset + xby2));

                    ConvertYUVToRGBA(Y, U, V, out byte r, out byte g, out byte b, out byte a);
                    Marshal.WriteByte(dest, index++, b);
                    Marshal.WriteByte(dest, index++, g);
                    Marshal.WriteByte(dest, index++, r);
                    Marshal.WriteByte(dest, index++, a);
                }
            }
        }

        public static void YUV2RGBA(byte[] source, IntPtr dest, uint imageWidth, uint imageHeight)
        {
            int index = 0;
            uint ySize = imageWidth * imageHeight;
            uint uSize = (imageWidth * imageHeight >> 2);
            uint uStride = imageWidth >> 1;

            // Get each pixel, one at a time
            for (int y = 0; y < imageHeight; y++)
            {
                for (int x = 0; x < imageWidth; x++)
                {
                    // Get the Y value, stored in the first block of data
                    // The logical "AND 0xff" is needed to deal with the signed issue
                    byte Y = source[y * imageWidth + x];

                    // Get U and V values, stored after Y values, one per 2x2 block
                    // of pixels, interleaved. Prepare them as floats with correct range
                    // ready for calculation later.
                    int xby2 = x >> 1;
                    int yby2 = y >> 1;
                    long offset = yby2 * uStride;
                    byte U = source[ySize + offset + xby2];
                    byte V = source[ySize + uSize + offset + xby2];

                    ConvertYUVToRGBA(Y, U, V, out byte r, out byte g, out byte b, out byte a);
                    Marshal.WriteByte(dest, index++, b);
                    Marshal.WriteByte(dest, index++, g);
                    Marshal.WriteByte(dest, index++, r);
                    Marshal.WriteByte(dest, index++, a);
                }
            }
        }

        public static void YUV2RGBA(IntPtr source, byte[] dest, uint imageWidth, uint imageHeight)
        {
            int index = 0;
            uint ySize = imageWidth * imageHeight;
            uint uSize = (imageWidth * imageHeight >> 2);
            uint uStride = imageWidth >> 1;

            // Get each pixel, one at a time
            for (int y = 0; y < imageHeight; y++)
            {
                for (int x = 0; x < imageWidth; x++)
                {
                    // Get the Y value, stored in the first block of data
                    // The logical "AND 0xff" is needed to deal with the signed issue
                    byte Y = Marshal.ReadByte(source, (int)(y * imageWidth + x));

                    // Get U and V values, stored after Y values, one per 2x2 block
                    // of pixels, interleaved. Prepare them as floats with correct range
                    // ready for calculation later.
                    int xby2 = x >> 1;
                    int yby2 = y >> 1;
                    long offset = yby2 * uStride;
                    byte U = Marshal.ReadByte(source, (int)(ySize + offset + xby2));
                    byte V = Marshal.ReadByte(source, (int)(ySize + uSize + offset + xby2));

                    // the buffer we fill up which we then fill the bitmap with
                    //MemoryStream intBuffer = new MemoryStream();

                    ConvertYUVToRGBA(Y, U, V, out byte r, out byte g, out byte b, out byte a);
                    dest[index++] = b;
                    dest[index++] = g;
                    dest[index++] = r;
                    dest[index++] = a;
                }
            }
        }

        public static void YUV2RGBA(byte[] source, byte[] dest, uint imageWidth, uint imageHeight)
        {
            int index = 0;
            uint ySize = imageWidth * imageHeight;
            uint uSize = (imageWidth * imageHeight >> 2);
            uint uStride = imageWidth >> 1;

            // Get each pixel, one at a time
            for (int y = 0; y < imageHeight; y++)
            {
                for (int x = 0; x < imageWidth; x++)
                {
                    // Get the Y value, stored in the first block of data
                    // The logical "AND 0xff" is needed to deal with the signed issue
                    short Y = source[y * imageWidth + x];

                    // Get U and V values, stored after Y values, one per 2x2 block
                    // of pixels, interleaved. Prepare them as floats with correct range
                    // ready for calculation later.
                    int xby2 = x >> 1;
                    int yby2 = y >> 1;
                    long offset = yby2 * uStride;
                    short U = source[ySize + offset + xby2];
                    short V = source[ySize + uSize + offset + xby2];

                    ConvertYUVToRGBA(Y, U, V, out byte r, out byte g, out byte b, out byte a);
                    dest[index++] = b;
                    dest[index++] = g;
                    dest[index++] = r;
                    dest[index++] = a;
                }
            }
        }

        private static void ConvertYUVToRGBA(short Y, short U, short V, out byte R, out byte G, out byte B, out byte A)
        {
            // Correct Y to allow for the fact that it is [16..235] and not [0..255]
            Y -= 16;
            U -= 128;
            V -= 128;
            int r = 1191 * Y + (1634 * V) >> 10;
            int g = 1191 * Y - 401 * U - (832 * V) >> 10;
            int b = 1191 * Y + 2065 * U >> 10;
            B = (byte)(b < 0 ? 0 : b > FF ? FF : b);
            G = (byte)(g < 0 ? 0 : g > FF ? FF : g);
            R = (byte)(r < 0 ? 0 : r > FF ? FF : r);
            A = FF;
        }

        #endregion

        #region libyuv

        [DllImport("libyuv", SetLastError = true)]
        private static extern unsafe int I420ToARGB(
            IntPtr src_y, int src_stride_y,
            IntPtr src_u, int src_stride_u,
            IntPtr src_v, int src_stride_v,
            IntPtr dst_argb,
            int dst_stride_argb,
            int width, int height);


        public static unsafe void I420ToARGB(IntPtr source, IntPtr dest, uint imageWidth, uint imageHeight)
        {
            int width = (int)imageWidth;
            int height = (int)imageHeight;
            int size_y = width * height;
            int size_u = size_y >> 2;
            int size_v = size_u;
            IntPtr src_y = source;
            int src_stride_y = width;
            IntPtr src_u = src_y + size_y;
            int src_stride_u = width >> 1;
            IntPtr src_v = src_u + size_u;
            int src_stride_v = src_stride_u;
            IntPtr dst_argb = dest;
            int dst_stride_argb = width << 2;
            _ = I420ToARGB(src_y, src_stride_y,
                src_u, src_stride_u,
                src_v, src_stride_v,
                dst_argb,
                dst_stride_argb,
                width, height);
        }

        #endregion
    }
}
