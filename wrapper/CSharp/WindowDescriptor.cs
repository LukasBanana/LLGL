/*
 * WindowDescriptor.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;
using System.Text;

namespace LLGL
{
    public class WindowDescriptor
    {
        internal NativeLLGL.WindowDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.WindowDescriptor();
                unsafe
                {
                    fixed (byte* titlePtr = titleAscii)
                    {
                        native.title = titlePtr;
                    }
                    native.position = Position;
                    native.size = Size;
                    native.flags = (int)Flags;
                    native.windowContext = null; //TODO
                    native.windowContextSize = (IntPtr)0; //TODO
                }
                return native;
            }
        }

        private string title;
        private byte[] titleAscii;

        public string Title
        {
            get
            {
                return title;
            }
            set
            {
                title = value;
                titleAscii = Encoding.ASCII.GetBytes(title + "\0");
            }
        }

        public Offset2D Position { get; set; }
        public Extent2D Size { get; set; }
        public WindowFlags Flags { get; set; }
        public Window WindowContext { get; set; }
    }
}




// ================================================================================
