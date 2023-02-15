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

                    // Correct Y to allow for the fact that it is [16..235] and not [0..255]
                    Y -= 16;
                    U -= 128;
                    V -= 128;
                    int R = 1191 * Y + (1634 * V) >> 10;
                    int G = 1191 * Y - 401 * U - (832 * V) >> 10;
                    int B = 1191 * Y + 2065 * U >> 10;

                    Marshal.WriteByte(dest, index++, (byte)(B < 0 ? 0 : B > FF ? FF : B));
                    Marshal.WriteByte(dest, index++, (byte)(G < 0 ? 0 : G > FF ? FF : G));
                    Marshal.WriteByte(dest, index++, (byte)(R < 0 ? 0 : R > FF ? FF : R));
                    index++;
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

                    // Correct Y to allow for the fact that it is [16..235] and not [0..255]
                    Y -= 16;
                    U -= 128;
                    V -= 128;
                    int R = 1191 * Y + (1634 * V) >> 10;
                    int G = 1191 * Y - 401 * U - (832 * V) >> 10;
                    int B = 1191 * Y + 2065 * U >> 10;

                    Marshal.WriteByte(dest, index++, (byte)(B < 0 ? 0 : B > FF ? FF : B));
                    Marshal.WriteByte(dest, index++, (byte)(G < 0 ? 0 : G > FF ? FF : G));
                    Marshal.WriteByte(dest, index++, (byte)(R < 0 ? 0 : R > FF ? FF : R));
                    index++;
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

                    // Correct Y to allow for the fact that it is [16..235] and not [0..255]
                    Y -= 16;
                    U -= 128;
                    V -= 128;
                    int R = 1191 * Y + (1634 * V) >> 10;
                    int G = 1191 * Y - 401 * U - (832 * V) >> 10;
                    int B = 1191 * Y + 2065 * U >> 10;

                    dest[index++] = (byte)(B < 0 ? 0 : B > FF ? FF : B);
                    dest[index++] = (byte)(G < 0 ? 0 : G > FF ? FF : G);
                    dest[index++] = (byte)(R < 0 ? 0 : R > FF ? FF : R);
                    index++;
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

                    // Correct Y to allow for the fact that it is [16..235] and not [0..255]
                    Y -= 16;
                    U -= 128;
                    V -= 128;
                    int R = 1191 * Y + (1634 * V) >> 10;
                    int G = 1191 * Y - 401 * U - (832 * V) >> 10;
                    int B = 1191 * Y + 2065 * U >> 10;

                    dest[index++] = (byte)(B < 0 ? 0 : B > FF ? FF : B);
                    dest[index++] = (byte)(G < 0 ? 0 : G > FF ? FF : G);
                    dest[index++] = (byte)(R < 0 ? 0 : R > FF ? FF : R);
                    index++;
                }
            }
        }
    }
}
