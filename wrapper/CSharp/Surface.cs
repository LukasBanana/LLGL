/*
 * Surface.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGL
{
    public class Surface
    {
        internal NativeLLGL.Surface NativeBase { get; private set; }

        internal Surface(NativeLLGL.Surface native)
        {
            NativeBase = native;
        }

        public static bool ProcessEvents()
        {
            return NativeLLGL.ProcessSurfaceEvents();
        }

        public Extent2D ContentSize
        {
            get
            {
                var size = new Extent2D();
                NativeLLGL.GetSurfaceContentSize(NativeBase, ref size);
                return size;
            }
        }

        public bool AdaptForVideoMode(ref Extent2D resolution, ref bool fullscreen)
        {
            unsafe
            {
                fixed (bool* fullscreenPtr = &fullscreen)
                {
                    return NativeLLGL.AdaptSurfaceForVideoMode(NativeBase, ref resolution, fullscreenPtr);
                }
            }
        }

        [Obsolete("LLGL.Surface.ResetPixelFormat() is deprecated since 0.04b; No need to reset pixel format anymore!")]
        public void ResetPixelFormat()
        {
            // dummy
        }

        public Window AsWindow()
        {
            return new Window(NativeBase.AsWindow());
        }

        public Display FindResidentDisplay()
        {
            return new Display(NativeLLGL.FindSurfaceResidentDisplay(NativeBase));
        }

    }
}




// ================================================================================
