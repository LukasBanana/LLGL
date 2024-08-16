/*
 * RenderingDebugger.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;
using System.Runtime.InteropServices;

namespace LLGL
{
    public class RenderingDebugger
    {
        internal NativeLLGL.RenderingDebugger Native { get; private set; }

        public RenderingDebugger()
        {
            Native = NativeLLGL.AllocRenderingDebugger();
        }

        ~RenderingDebugger()
        {
            NativeLLGL.FreeRenderingDebugger(Native);
        }

        public bool TimeRecording
        {
            get
            {
                return NativeLLGL.GetDebuggerTimeRecording(Native);
            }
            set
            {
                NativeLLGL.SetDebuggerTimeRecording(Native, value);
            }
        }

        public FrameProfile FlushProfile()
        {
            var nativeFrameProfile = new NativeLLGL.FrameProfile();
            NativeLLGL.FlushDebuggerProfile(Native, ref nativeFrameProfile);
            return new FrameProfile(nativeFrameProfile);
        }
    }
}




// ================================================================================
