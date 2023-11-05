/*
 * PipelineCache.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGL
{
    public sealed class PipelineCache : RenderSystemChild
    {
        internal NativeLLGL.PipelineCache Native { get; private set; }

        internal override NativeLLGL.RenderSystemChild NativeChild
        {
            get
            {
                unsafe
                {
                    return new NativeLLGL.RenderSystemChild() { ptr = Native.ptr };
                }
            }
        }

        internal PipelineCache(NativeLLGL.PipelineCache native)
        {
            Native = native;
        }

        ~PipelineCache()
        {
            NativeLLGL.ReleasePipelineCache(Native);
        }

        public byte[] Blob
        {
            get
            {
                unsafe
                {
                    int dataSize = (int)NativeLLGL.GetPipelineCacheBlob(Native, null, (IntPtr)0);
                    if (dataSize > 0)
                    {
                        var data = new byte[dataSize];
                        fixed (byte* dataPtr = data)
                        {
                            NativeLLGL.GetPipelineCacheBlob(Native, dataPtr, (IntPtr)dataSize);
                        }
                        return data;
                    }
                }
                return null;
            }
        }
    }
}




// ================================================================================
