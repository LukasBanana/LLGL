/*
 * Sampler.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGL
{
    public sealed class Sampler : Resource
    {
        internal NativeLLGL.Sampler Native { get; private set; }

        internal override NativeLLGL.Resource NativeBase
        {
            get
            {
                unsafe
                {
                    return new NativeLLGL.Resource() { ptr = Native.ptr };
                }
            }
        }

        internal Sampler(NativeLLGL.Sampler native, string debugName = null)
        {
            Native = native;
            InitializeDebugName(debugName);
        }

        ~Sampler()
        {
            NativeLLGL.ReleaseSampler(Native);
        }

    }
}




// ================================================================================
