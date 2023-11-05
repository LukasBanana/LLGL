/*
 * RenderSystemChild.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGL
{
    public abstract class RenderSystemChild
    {
        private string name = "";

        internal abstract NativeLLGL.RenderSystemChild NativeChild { get; }

        public string Name
        {
            get
            {
                return name;
            }
            set
            {
                name = value;
                NativeLLGL.SetName(NativeChild, name);
            }
        }

    }
}




// ================================================================================
