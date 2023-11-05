/*
 * BlendTargetDescriptorArray.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;
using System.Text;

namespace LLGL
{
    public sealed class BlendTargetDescriptorArray : TDescriptorArray<BlendTargetDescriptor>
    {
        internal BlendTargetDescriptorArray() : base(count: 8)
        {
            for (int i = 0; i < Length; ++i)
            {
                this[i] = new BlendTargetDescriptor();
            }
        }
    }
}




// ================================================================================
