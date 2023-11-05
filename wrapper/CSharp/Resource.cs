/*
 * Resource.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGL
{
    public abstract class Resource : RenderSystemChild
    {
        internal abstract NativeLLGL.Resource NativeBase { get; }

        internal override NativeLLGL.RenderSystemChild NativeChild
        {
            get
            {
                unsafe
                {
                    return new NativeLLGL.RenderSystemChild() { ptr = NativeBase.ptr };
                }
            }
        }

        public ResourceType ResourceType
        {
            get
            {
                return NativeLLGL.GetResourceType(NativeBase);
            }
        }

    }
}




// ================================================================================
