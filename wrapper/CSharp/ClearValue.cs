/*
 * ClearValue.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGL
{
    public class ClearValue
    {
        public ClearValue()
        {
        }

        public ClearValue(Color color, float depth = 1.0f, int stencil = 0)
        {
            Color = color;
            Depth = depth;
            Stencil = stencil;
        }

        public ClearValue(float depth)
        {
            Depth = depth;
        }

        public ClearValue(int stencil)
        {
            Stencil = stencil;
        }

        public Color Color { get; set; } = Color.TransparentBlack;
        public float Depth { get; set; } = 1.0f;
        public int Stencil { get; set; } = 0;

        internal NativeLLGL.ClearValue Native
        {
            get
            {
                var native = new NativeLLGL.ClearValue();
                CopyTo(ref native);
                return native;
            }
            set
            {
                unsafe
                {
                    Color = new Color(value.color[0], value.color[1], value.color[2], value.color[3]);
                    Depth = value.depth;
                    Stencil = value.stencil;
                }
            }
        }

        internal void CopyTo(ref NativeLLGL.ClearValue outNative)
        {
            unsafe
            {
                fixed (float* outColor = outNative.color)
                {
                    outColor[0] = Color.R;
                    outColor[1] = Color.G;
                    outColor[2] = Color.B;
                    outColor[3] = Color.A;
                }
                outNative.depth = Depth;
                outNative.stencil = Stencil;
            }
        }
    }
}




// ================================================================================
