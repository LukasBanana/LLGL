/*
 * Color.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGL
{
    public struct Color
    {
        public float R { get; set; }
        public float G { get; set; }
        public float B { get; set; }
        public float A { get; set; }

        public Color(float grayScale)
        {
            R = grayScale;
            G = grayScale;
            B = grayScale;
            A = 1.0f;
        }

        public Color(float r, float g, float b, float a = 1.0f)
        {
            R = r;
            G = g;
            B = b;
            A = a;
        }

        public static readonly Color White = new Color(1.0f, 1.0f, 1.0f, 1.0f);

        public static readonly Color TransparentWhite = new Color(1.0f, 1.0f, 1.0f, 0.0f);

        public static readonly Color Black = new Color(0.0f, 0.0f, 0.0f, 1.0f);

        public static readonly Color TransparentBlack = new Color(0.0f, 0.0f, 0.0f, 0.0f);
    }
}




// ================================================================================
