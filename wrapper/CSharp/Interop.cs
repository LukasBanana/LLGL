/*
 * Interop.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;
using System.Runtime.InteropServices;

namespace LLGL
{
    public static class Interop
    {
        public static byte[] ToBytes<T>(T src) where T : struct
        {
            int size = Marshal.SizeOf(typeof(T));

            IntPtr ptr = Marshal.AllocHGlobal(size);
            byte[] output = new byte[size];

            Marshal.StructureToPtr(src, ptr, false);
            Marshal.Copy(ptr, output, 0, size);
            Marshal.FreeHGlobal(ptr);

            return output;
        }

        public static byte[] ToBytes<T>(T[] src) where T : struct
        {
            int elementSize = Marshal.SizeOf(typeof(T));
            int size = src.Length * elementSize;

            IntPtr ptr = Marshal.AllocHGlobal(size);
            byte[] output = new byte[size];

            for (int i = 0; i < src.Length; ++i)
            {
                Marshal.StructureToPtr(src[i], ptr + elementSize * i, false);
            }

            Marshal.Copy(ptr, output, 0, size);
            Marshal.FreeHGlobal(ptr);

            return output;
        }

        public static T ToStruct<T>(byte[] src, int startIndex = 0) where T : struct
        {
            int size = Marshal.SizeOf(typeof(T));
            if (startIndex < 0)
            {
                throw new IndexOutOfRangeException("Start index for byte array must not be negative");
            }
            if (startIndex >= src.Length)
            {
                throw new IndexOutOfRangeException("Start index for byte array must not greater than or equal to array length");
            }
            if (src.Length < size + startIndex)
            {
                throw new ArgumentException("Source byte array is not large enough for output structure", "src");
            }

            T output = default(T);

            IntPtr ptr = Marshal.AllocHGlobal(size);
            Marshal.Copy(src, startIndex, ptr, size);
            output = (T)Marshal.PtrToStructure(ptr, typeof(T));
            Marshal.FreeHGlobal(ptr);

            return output;
        }

        public static T[] ToStructArray<T>(byte[] src, int startIndex = 0) where T : struct
        {
            int elementSize = Marshal.SizeOf(typeof(T));
            if (startIndex < 0)
            {
                throw new IndexOutOfRangeException("Start index for byte array must not be negative");
            }
            if (startIndex >= src.Length)
            {
                throw new IndexOutOfRangeException("Start index for byte array must not greater than or equal to array length");
            }
            if ((src.Length % elementSize) != (startIndex % elementSize))
            {
                throw new ArgumentException("Source byte array is not large enough for output structure array", "src");
            }

            int arrayLength = ((src.Length - startIndex) / elementSize);
            int size = arrayLength * elementSize;

            IntPtr ptr = Marshal.AllocHGlobal(size);
            Marshal.Copy(src, 0, ptr, size);

            T[] output = new T[arrayLength];

            for (int i = 0; i < arrayLength; ++i)
            {
                output[i] = (T)Marshal.PtrToStructure(ptr + elementSize*i, typeof(T));
            }

            Marshal.FreeHGlobal(ptr);

            return output;
        }
    }
}




// ================================================================================
