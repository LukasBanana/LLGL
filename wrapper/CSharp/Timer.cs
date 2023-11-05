/*
 * Timer.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGL
{
    public static class Timer
    {
        public static long Frequency
        {
            get
            {
                return NativeLLGL.TimerFrequency();
            }
        }

        public static long Tick
        {
            get
            {
                return NativeLLGL.TimerTick();
            }
        }
    }
}




// ================================================================================
