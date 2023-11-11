/*
 * Display.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;
using System.Collections.Generic;

namespace LLGL
{
    public class Display
    {
        internal NativeLLGL.Display Native { get; private set; }

        internal Display(NativeLLGL.Display native)
        {
            Native = native;
        }

        public static int Count
        {
            get
            {
                return (int)NativeLLGL.DisplayCount();
            }
        }

        private static List<Display> displayList = new List<Display>();
        public static List<Display> List
        {
            get
            {
                int count = Count;
                displayList.Clear();
                unsafe
                {
                    NativeLLGL.Display* nativeDisplayList = NativeLLGL.GetDisplayList();
                    for (int i = 0; i < count; ++i)
                    {
                        displayList.Add(new Display(nativeDisplayList[i]));
                    }
                }
                return displayList;
            }
        }

        public static Display Get(int index)
        {
            var nativeDisplay = NativeLLGL.GetDisplay((IntPtr)index);
            unsafe
            {
                if (nativeDisplay.ptr != null)
                {
                    return new Display(nativeDisplay);
                }
            }
            return null;
        }

        public static Display Primary
        {
            get
            {
                return new Display(NativeLLGL.GetPrimaryDisplay());
            }
        }

        public bool ShowCursor(bool show)
        {
            return NativeLLGL.ShowCursor(show);
        }

        public bool IsCursorShown
        {
            get
            {
                return NativeLLGL.IsCursorShown();
            }
        }

        static Offset2D CursorPosition
        {
            get
            {
                var offset = new Offset2D();
                NativeLLGL.GetCursorPosition(ref offset);
                return offset;
            }
            set
            {
                NativeLLGL.SetCursorPosition(ref value);
            }
        }

        public bool IsPrimary
        {
            get
            {
                return NativeLLGL.IsDisplayPrimary(Native);
            }
        }

        public string DeviceName
        {
            get
            {
                unsafe
                {
                    IntPtr nullTerminatedNameLength = NativeLLGL.GetDisplayDeviceName(Native, (IntPtr)0, null);
                    if (nullTerminatedNameLength.ToInt64() > 0)
                    {
                        int nameLength = (int)nullTerminatedNameLength - 1;
                        var nativeDisplayName = new char[nameLength];
                        fixed (char* nativeDisplayNamePtr = nativeDisplayName)
                        {
                            NativeLLGL.GetDisplayDeviceName(Native, (IntPtr)nameLength, nativeDisplayNamePtr);
                        }
                        return new string(nativeDisplayName);
                    }
                    return "";
                }
            }
        }

        public DisplayMode DisplayMode
        {
            get
            {
                var nativeDisplayMode = new NativeLLGL.DisplayMode();
                NativeLLGL.GetDisplayMode(Native, ref nativeDisplayMode);
                return new DisplayMode(nativeDisplayMode);
            }
            set
            {
                var nativeDisplayMode = value.Native;
                NativeLLGL.SetDisplayMode(Native, ref nativeDisplayMode);
            }
        }

        public List<DisplayMode> SupportedDisplayModes
        {
            get
            {
                unsafe
                {
                    IntPtr numDisplayModes = NativeLLGL.GetSupportedDisplayModes(Native, (IntPtr)0, null);
                    var nativeDisplayModes = new NativeLLGL.DisplayMode[(int)numDisplayModes];
                    fixed (NativeLLGL.DisplayMode* nativeDisplayModesPtr = nativeDisplayModes)
                    {
                        NativeLLGL.GetSupportedDisplayModes(Native, numDisplayModes, nativeDisplayModesPtr);
                    }
                    var displayModes = new List<DisplayMode>((int)numDisplayModes);
                    foreach (var mode in nativeDisplayModes)
                    {
                        displayModes.Add(new DisplayMode(mode));
                    }
                    return displayModes;
                }
            }
        }

    }
}




// ================================================================================
