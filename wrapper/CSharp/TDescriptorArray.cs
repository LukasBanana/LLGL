/*
 * TDescriptorArray.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;
using System.Text;

namespace LLGL
{
    public abstract class TDescriptorArray<T>
    {
        internal TDescriptorArray(int count)
        {
            Descs = new T[count];
        }

        private T[] Descs { get; set; }

        public int Length
        {
            get
            {
                return Descs.Length;
            }
        }

        public T this[int key]
        {
            get
            {
                return Descs[key];
            }
            set
            {
                Descs[key] = value;
            }
        }
    }
}




// ================================================================================
