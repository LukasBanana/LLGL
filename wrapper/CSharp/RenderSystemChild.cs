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
        private string debugName = "";

        internal abstract NativeLLGL.RenderSystemChild NativeChild { get; }

        protected internal void InitializeDebugName(string name)
        {
            if (name != null)
            {
                debugName = name;
            }
        }

        public string DebugName
        {
            get
            {
                return debugName;
            }
            set
            {
                debugName = value;
                NativeLLGL.SetDebugName(NativeChild, debugName);
            }
        }
    }
}




// ================================================================================
