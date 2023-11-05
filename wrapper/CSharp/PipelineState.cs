/*
 * PipelineState.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGL
{
    public sealed class PipelineState : RenderSystemChild
    {
        internal NativeLLGL.PipelineState Native { get; private set; }

        internal override NativeLLGL.RenderSystemChild NativeChild
        {
            get
            {
                unsafe
                {
                    return new NativeLLGL.RenderSystemChild() { ptr = Native.ptr };
                }
            }
        }

        internal PipelineState(NativeLLGL.PipelineState native)
        {
            Native = native;
        }

        ~PipelineState()
        {
            NativeLLGL.ReleasePipelineState(Native);
        }

        public Report Report
        {
            get
            {
                unsafe
                {
                    NativeLLGL.Report nativeReport = NativeLLGL.GetPipelineStateReport(Native);
                    if (nativeReport.ptr != null)
                    {
                        return new Report(nativeReport);
                    }
                    return null;
                }
            }
        }
    }
}




// ================================================================================
