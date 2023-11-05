/*
 * RenderSystemDescriptor.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;
using System.Text;

namespace LLGL
{
    public class RenderSystemDescriptor
    {
        public RenderSystemDescriptor()
        {
            ModuleName = "";
        }
        public RenderSystemDescriptor(string moduleName, RenderSystemFlags flags = 0, RenderingDebugger debugger = null)
        {
            ModuleName = moduleName;
            Flags = flags;
            Debugger = debugger;
        }

        private string moduleName;
        private byte[] moduleNameAscii;

        public string ModuleName
        {
            get
            {
                return moduleName;
            }
            set
            {
                moduleName = value;
                moduleNameAscii = Encoding.ASCII.GetBytes(moduleName + "\0");
            }
        }

        public RenderSystemFlags Flags { get; set; }

        public RenderingDebugger Debugger { get; set; }

        internal NativeLLGL.RenderSystemDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.RenderSystemDescriptor();
                unsafe
                {
                    fixed (byte* moduleNameAsciiPtr = moduleNameAscii)
                    {
                        native.moduleName = moduleNameAsciiPtr;
                    }
                    native.flags = (int)Flags;
                    native.debugger = Debugger != null ? Debugger.Native : new NativeLLGL.RenderingDebugger();
                    native.rendererConfig = null;
                    native.rendererConfigSize = (IntPtr)0;
                }
                return native;
            }
        }
    }
}




// ================================================================================
