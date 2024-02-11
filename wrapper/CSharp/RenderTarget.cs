/*
 * RenderTarget.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGL
{
    public class RenderTarget : RenderSystemChild
    {
        internal NativeLLGL.RenderTarget Native { get; private set; }

        internal protected void DisposeNative()
        {
            Native = default(NativeLLGL.RenderTarget);
        }

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

        internal RenderTarget(NativeLLGL.RenderTarget native, string debugName = null)
        {
            Native = native;
            InitializeDebugName(debugName);
        }

        ~RenderTarget()
        {
            unsafe
            {
                if (Native.ptr != null)
                {
                    NativeLLGL.ReleaseRenderTarget(Native);
                }
            }
        }

        public Extent2D Resolution
        {
            get
            {
                var resolution = new Extent2D();
                NativeLLGL.GetRenderTargetResolution(Native, ref resolution);
                return resolution;
            }
        }

        public int Samples
        {
            get
            {
                return NativeLLGL.GetRenderTargetSamples(Native);
            }
        }

        public int NumColorAttachments
        {
            get
            {
                return NativeLLGL.GetRenderTargetNumColorAttachments(Native);
            }
        }

        public bool HasDepthAttachment
        {
            get
            {
                return NativeLLGL.HasRenderTargetDepthAttachment(Native);
            }
        }

        public bool HasStencilAttachment
        {
            get
            {
                return NativeLLGL.HasRenderTargetStencilAttachment(Native);
            }
        }

        public RenderPass RenderPass
        {
            get
            {
                return new RenderPass(NativeLLGL.GetRenderTargetRenderPass(Native));
            }
        }
    }
}




// ================================================================================
