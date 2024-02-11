/*
 * AnsiString.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;
using System.Text;

namespace LLGL
{
    public struct AnsiString
    {
        private string Str;
        internal byte[] Ascii { get;  private set; }

        public static implicit operator AnsiString(string s)
        {
            var ansiStr = new AnsiString();
            ansiStr.Str = s;
            ansiStr.Ascii = (ansiStr.Str != null ? Encoding.ASCII.GetBytes(ansiStr.Str + "\0") : null);
            return ansiStr;
        }

        public static implicit operator string(AnsiString s)
        {
            return s.Str;
        }
    }
}




// ================================================================================
