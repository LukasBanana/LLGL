/*
 * Fence.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGL
{
    public class Fence : RenderSystemChild
    {
        internal NativeLLGL.Fence Native { get; private set; }

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

        internal Fence(NativeLLGL.Fence native)
        {
            Native = native;
        }

        ~Fence()
        {
            NativeLLGL.ReleaseFence(Native);
        }
    }
}




// ================================================================================
