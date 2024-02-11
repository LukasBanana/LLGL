/*
 * Buffer.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGL
{
    public sealed class Buffer : Resource
    {
        internal NativeLLGL.Buffer Native { get; private set; }

        internal override NativeLLGL.Resource NativeBase
        {
            get
            {
                unsafe
                {
                    return new NativeLLGL.Resource() { ptr = Native.ptr };
                }
            }
        }

        internal Buffer(NativeLLGL.Buffer native, string debugName = null)
        {
            Native = native;
            InitializeDebugName(debugName);
        }

        ~Buffer()
        {
            NativeLLGL.ReleaseBuffer(Native);
        }

        public BindFlags BindFlags
        {
            get
            {
                return (BindFlags)NativeLLGL.GetBufferBindFlags(Native);
            }
        }

        public BufferDescriptor Desc
        {
            get
            {
                var nativeDesc = new NativeLLGL.BufferDescriptor();
                NativeLLGL.GetBufferDesc(Native, ref nativeDesc);
                return new BufferDescriptor(nativeDesc);
            }
        }
    }
}




// ================================================================================
