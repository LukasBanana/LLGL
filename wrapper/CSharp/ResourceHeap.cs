/*
 * ResourceHeap.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGL
{
    public sealed class ResourceHeap : RenderSystemChild
    {
        internal NativeLLGL.ResourceHeap Native { get; private set; }

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

        internal ResourceHeap(NativeLLGL.ResourceHeap native, string debugName = null)
        {
            Native = native;
            InitializeDebugName(debugName);
        }

        ~ResourceHeap()
        {
            NativeLLGL.ReleaseResourceHeap(Native);
        }

        public int NumDescriptorSets
        {
            get
            {
                return NativeLLGL.GetResourceHeapNumDescriptorSets(Native);
            }
        }
    }
}




// ================================================================================
