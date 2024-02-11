/*
 * RenderPassDescriptor.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;
using System.Text;

namespace LLGL
{
    public class RenderPassDescriptor
    {
        public AnsiString DebugName { get; set; }
        public AttachmentFormatDescriptorArray ColorAttachments { get; private set; } = new AttachmentFormatDescriptorArray();
        public AttachmentFormatDescriptor DepthAttachment { get; set; } = new AttachmentFormatDescriptor();
        public AttachmentFormatDescriptor StencilAttachment { get; set; } = new AttachmentFormatDescriptor();
        public int Samples { get; set; } = 1;

        internal NativeLLGL.RenderPassDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.RenderPassDescriptor();
                unsafe
                {
                    fixed (byte* debugNamePtr = DebugName.Ascii)
                    {
                        native.debugName = debugNamePtr;
                    }
                    NativeLLGL.AttachmentFormatDescriptor* nativeColorAttachments = &native.colorAttachments0;
                    for (int i = 0; i < ColorAttachments.Length && ColorAttachments[i] != null; ++i)
                    {
                        nativeColorAttachments[i] = ColorAttachments[i].Native;
                    }
                    native.depthAttachment = DepthAttachment.Native;
                    native.stencilAttachment = StencilAttachment.Native;
                    native.samples = Samples;
                }
                return native;
            }
        }
    }
}




// ================================================================================
