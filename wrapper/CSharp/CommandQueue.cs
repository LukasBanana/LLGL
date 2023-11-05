/*
 * CommandQueue.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;
using System.Runtime.InteropServices;

namespace LLGL
{
    public class CommandQueue
    {
        internal CommandQueue()
        {
        }

        public void Submit(CommandBuffer commandBuffer)
        {
            NativeLLGL.SubmitCommandBuffer(commandBuffer.Native);
        }

        public bool QueryResult(QueryHeap queryHeap, int firstQuery, int numQueries, ref QueryPipelineStatistics[] data)
        {
            unsafe
            {
                fixed (QueryPipelineStatistics* dataPtr = data)
                {
                    return NativeLLGL.QueryResult(queryHeap.Native, firstQuery, numQueries, dataPtr, (IntPtr)(data.Length * sizeof(QueryPipelineStatistics)));
                }
            }
        }

        public bool QueryResult(QueryHeap queryHeap, int firstQuery, int numQueries, ref int[] data)
        {
            unsafe
            {
                fixed (int* dataPtr = data)
                {
                    return NativeLLGL.QueryResult(queryHeap.Native, firstQuery, numQueries, dataPtr, (IntPtr)(data.Length * sizeof(int)));
                }
            }
        }

        public bool QueryResult(QueryHeap queryHeap, int firstQuery, int numQueries, ref long[] data)
        {
            unsafe
            {
                fixed (long* dataPtr = data)
                {
                    return NativeLLGL.QueryResult(queryHeap.Native, firstQuery, numQueries, dataPtr, (IntPtr)(data.Length * sizeof(ulong)));
                }
            }
        }

        public void Submit(Fence fence)
        {
            NativeLLGL.SubmitFence(fence.Native);
        }

        public bool WaitFence(Fence fence, long timeout)
        {
            return NativeLLGL.WaitFence(fence.Native, timeout);
        }

        public void WaitIdle()
        {
            NativeLLGL.WaitIdle();
        }
    }
}




// ================================================================================
