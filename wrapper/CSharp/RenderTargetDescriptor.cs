/*
 * RenderTargetDescriptor.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;
using System.Text;

namespace LLGL
{
    public class RenderTargetDescriptor
    {

        public AnsiString DebugName { get; set; }
        public RenderPass RenderPass { get; set; }
        public Extent2D Resolution { get; set; }
        public int Samples { get; set; } = 1;
        public AttachmentDescriptorArray ColorAttachments { get; private set; } = new AttachmentDescriptorArray();
        public AttachmentDescriptorArray ResolveAttachments { get; private set; } = new AttachmentDescriptorArray();
        public AttachmentDescriptor DepthStencilAttachment { get; set; } = new AttachmentDescriptor();

        internal NativeLLGL.RenderTargetDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.RenderTargetDescriptor();
                unsafe
                {
                    fixed (byte* debugNamePtr = DebugName.Ascii)
                    {
                        native.debugName = debugNamePtr;
                    }
                    if (RenderPass != null)
                    {
                        native.renderPass = RenderPass.Native;
                    }
                    native.resolution = Resolution;
                    native.samples = Samples;
                    NativeLLGL.AttachmentDescriptor* nativeColorAttachments = &native.colorAttachments0;
                    for (int i = 0; i < ColorAttachments.Length && ColorAttachments[i] != null; ++i)
                    {
                        nativeColorAttachments[i] = ColorAttachments[i].Native;
                    }
                    NativeLLGL.AttachmentDescriptor* nativeResolveAttachments = &native.resolveAttachments0;
                    for (int i = 0; i < ResolveAttachments.Length && ResolveAttachments[i] != null; ++i)
                    {
                        nativeColorAttachments[i] = ResolveAttachments[i].Native;
                    }
                    native.depthStencilAttachment = DepthStencilAttachment.Native;
                }
                return native;
            }
        }
    }
}




// ================================================================================
