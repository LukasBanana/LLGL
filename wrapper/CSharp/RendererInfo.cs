/*
 * RendererInfo.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;
using System.Runtime.InteropServices;

namespace LLGL
{
    public class RendererInfo
    {
        internal RendererInfo(NativeLLGL.RendererInfo native)
        {
            unsafe
            {
                RendererName = Marshal.PtrToStringAnsi((IntPtr)native.rendererName);
                DeviceName = Marshal.PtrToStringAnsi((IntPtr)native.deviceName);
                VendorName = Marshal.PtrToStringAnsi((IntPtr)native.vendorName);
                ShadingLanguageName = Marshal.PtrToStringAnsi((IntPtr)native.shadingLanguageName);
                ExtensionNames = new string[(uint)native.numExtensionNames];
                for (uint i = 0; i < (uint)native.numExtensionNames; ++i)
                {
                    ExtensionNames[i] = Marshal.PtrToStringAnsi((IntPtr)native.extensionNames[i]);
                }
                if (native.pipelineCacheID != null)
                {
                    PipelineCacheID = new byte[(uint)native.numPipelineCacheID];
                    Marshal.Copy((IntPtr)native.pipelineCacheID, PipelineCacheID, 0, PipelineCacheID.Length);
                }
            }
        }

        public string RendererName { get; set; }
        public string DeviceName { get; set; }
        public string VendorName { get; set; }
        public string ShadingLanguageName { get; set; }
        public string[] ExtensionNames { get; set; }
        public byte[] PipelineCacheID { get; set; }
    }
}




// ================================================================================
