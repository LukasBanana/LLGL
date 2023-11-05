/*
 * BufferArray.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGL
{
    public sealed class BufferArray : RenderSystemChild
    {
        internal NativeLLGL.BufferArray Native { get; private set; }

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

        internal BufferArray(NativeLLGL.BufferArray native)
        {
            Native = native;
        }

        ~BufferArray()
        {
            NativeLLGL.ReleaseBufferArray(Native);
        }

        public BindFlags BindFlags
        {
            get
            {
                return (BindFlags)NativeLLGL.GetBufferArrayBindFlags(Native);
            }
        }
    }
}




// ================================================================================
