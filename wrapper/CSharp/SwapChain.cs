/*
 * SwapChain.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGL
{
    public sealed class SwapChain : RenderTarget
    {
        internal NativeLLGL.SwapChain NativeSub { get; private set; }

        internal SwapChain(NativeLLGL.SwapChain native, string debugName = null) : base(native.AsRenderTarget())
        {
            NativeSub = native;
            InitializeDebugName(debugName);
        }

        ~SwapChain()
        {
            NativeLLGL.ReleaseSwapChain(NativeSub);
            DisposeNative();
        }

        public Surface Surface
        {
            get
            {
                return new Surface(NativeLLGL.GetSurface(NativeSub));
            }
        }

        public bool IsPresentable
        {
            get
            {
                return NativeLLGL.IsPresentable(NativeSub);
            }
        }

        public void Present()
        {
            NativeLLGL.Present(NativeSub);
        }

        public int CurrentSwapIndex
        {
            get
            {
                return NativeLLGL.GetCurrentSwapIndex(NativeSub);
            }
        }

        public int NumSwapBuffers
        {
            get
            {
                return NativeLLGL.GetNumSwapBuffers(NativeSub);
            }
        }

        public Format ColorFormat
        {
            get
            {
                return NativeLLGL.GetColorFormat(NativeSub);
            }
        }

        public Format DepthStencilFormat
        {
            get
            {
                return NativeLLGL.GetDepthStencilFormat(NativeSub);
            }
        }

        public bool ResizeBuffers(Extent2D resolution, ResizeBuffersFlags flags = 0)
        {
            return NativeLLGL.ResizeBuffers(NativeSub, ref resolution, (int)flags);
        }

        public bool SetVsyncInterval(int vsyncInterval)
        {
            return NativeLLGL.SetVsyncInterval(NativeSub, vsyncInterval);
        }

        public bool SwitchFullscreen(bool enable)
        {
            return NativeLLGL.SwitchFullscreen(NativeSub, enable);
        }

    }
}




// ================================================================================
