/*
 * PipelineLayout.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGL
{
    public sealed class PipelineLayout : RenderSystemChild
    {
        internal NativeLLGL.PipelineLayout Native { get; private set; }

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

        internal PipelineLayout(NativeLLGL.PipelineLayout native, string debugName = null)
        {
            Native = native;
            InitializeDebugName(debugName);
        }

        ~PipelineLayout()
        {
            NativeLLGL.ReleasePipelineLayout(Native);
        }

        public int NumHeapBindings
        {
            get
            {
                return NativeLLGL.GetPipelineLayoutNumHeapBindings(Native);
            }
        }

        public int NumBindings
        {
            get
            {
                return NativeLLGL.GetPipelineLayoutNumBindings(Native);
            }
        }

        public int NumStaticSamplers
        {
            get
            {
                return NativeLLGL.GetPipelineLayoutNumStaticSamplers(Native);
            }
        }

        public int NumUniforms
        {
            get
            {
                return NativeLLGL.GetPipelineLayoutNumUniforms(Native);
            }
        }
    }
}




// ================================================================================
