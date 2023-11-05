/*
 * ImageView.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;
using System.Runtime.InteropServices;

namespace LLGL
{
    /// <summary>
    /// Class <c>ImageView</c> manages the memory for a mutable image, i.e. data that is read back from the CPU.
    /// The C# implementation of this class is identical to <seealso cref="MutableImageView"/>.
    /// </summary>
    public class ImageView
    {
        private NativeLLGL.ImageView native = new NativeLLGL.ImageView();
        internal NativeLLGL.ImageView Native
        {
            get
            {
                return native;
            }
        }

        public ImageView() { }

        public ImageView(ImageFormat imageFormat, DataType dataType, byte[] data)
        {
            ImageFormat = imageFormat;
            DataType = dataType;
            Data = data;
        }

        public unsafe ImageView(ImageFormat imageFormat, DataType dataType, void* data, IntPtr dataSize)
        {
            ImageFormat = imageFormat;
            DataType = dataType;
            UnsafeData(data, dataSize);
        }

        public ImageFormat ImageFormat {
            get
            {
                return native.format;
            }
            set
            {
                native.format = value;
            }
        }

        public DataType DataType
        {
            get
            {
                return native.dataType;
            }
            set
            {
                native.dataType = value;
            }
        }

        private byte[] data;
        public byte[] Data
        {
            get
            {
                return data;
            }
            set
            {
                data = value;
                unsafe
                {
                    fixed (void* dataPtr = data)
                    {
                        native.data = dataPtr;
                    }
                }
                native.dataSize = (IntPtr)data.Length;
            }
        }

        public unsafe void UnsafeData(void* data, IntPtr dataSize)
        {
            this.data = null;
            this.native.data = data;
            this.native.dataSize = dataSize;
        }

        public bool IsValid
        {
            get
            {
                unsafe
                {
                    return native.data != null;
                }
            }
        }
    }
}




// ================================================================================
