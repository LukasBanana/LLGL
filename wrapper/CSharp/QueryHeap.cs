/*
 * QueryHeap.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGL
{
    public sealed class QueryHeap : RenderSystemChild
    {
        internal NativeLLGL.QueryHeap Native { get; private set; }

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

        internal QueryHeap(NativeLLGL.QueryHeap native, string debugName = null)
        {
            Native = native;
            InitializeDebugName(debugName);
        }

        ~QueryHeap()
        {
            NativeLLGL.ReleaseQueryHeap(Native);
        }

        public QueryType Type
        {
            get
            {
                return NativeLLGL.GetQueryHeapType(Native);
            }
        }

    }
}




// ================================================================================
