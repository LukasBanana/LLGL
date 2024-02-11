/*
 * RenderPass.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGL
{
    public sealed class RenderPass : RenderSystemChild
    {
        internal NativeLLGL.RenderPass Native { get; private set; }

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

        internal RenderPass(NativeLLGL.RenderPass native, string debugName = null)
        {
            Native = native;
            InitializeDebugName(debugName);
        }
    }
}




// ================================================================================
