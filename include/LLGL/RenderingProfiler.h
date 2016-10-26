/*
 * RenderingProfiler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDERING_PROFILER_H
#define LLGL_RENDERING_PROFILER_H


#include "Export.h"
#include "RenderContextFlags.h"
#include "GraphicsPipelineFlags.h"


namespace LLGL
{


/**
\brief Rendering profiler model class.
\remarks This can be used to profile the renderer draw calls and buffer updates.
*/
class LLGL_EXPORT RenderingProfiler
{

    public:

        //! Profiling counter class.
        class LLGL_EXPORT Counter
        {

            public:

                using ValueType = unsigned int;

                //! Increment internal counter by one.
                void Inc()
                {
                    ++value_;
                }

                //! Increment internal counter by the specified value.
                void Inc(ValueType value)
                {
                    value_ += value;
                }

                //! Reset internal counter to zero.
                void Reset()
                {
                    value_ = 0;
                }

                //! Returns the internal counter value.
                inline ValueType Count() const
                {
                    return value_;
                }

                //! Returns the internal counter value (same as "Count()" function).
                inline operator unsigned int () const
                {
                    return Count();
                }

            private:

                ValueType value_ = 0;

        };

        /**
        \brief Resets all counters.
        \see Counter::Reset
        */
        void ResetCounters();

        void RecordDrawCall(const PrimitiveTopology topology, Counter::ValueType numVertices);
        void RecordDrawCall(const PrimitiveTopology topology, Counter::ValueType numVertices, Counter::ValueType numInstances);

        Counter writeBuffer;            //!< Counter for buffer writings. \see RenderSystem::WriteBuffer
        Counter mapBuffer;              //!< Counter for buffer mappings. \see RenderSystem::MapBuffer

        Counter setVertexBuffer;        //!< Counter for vertex buffer bindings. \see CommandBuffer::SetVertexBuffer
        Counter setIndexBuffer;         //!< Counter for index buffer bindings. \see CommandBuffer::SetIndexBuffer
        Counter setConstantBuffer;      //!< Counter for constant buffer bindings. \see CommandBuffer::SetConstantBuffer
        Counter setStorageBuffer;       //!< Counter for storage buffer bindings. \see CommandBuffer::SetStorageBuffer
        Counter setStreamOutputBuffer;  //!< Counter for stream-output buffer bindings. \see CommandBuffer::SetStreamOutputBuffer
        Counter setGraphicsPipeline;    //!< Counter for graphics pipeline bindings. \see CommandBuffer::SetGraphicsPipeline
        Counter setComputePipeline;     //!< Counter for compute pipeline bindings. \see CommandBuffer::SetComputePipeline
        Counter setTexture;             //!< Counter for texture bindings. \see CommandBuffer::SetTexture
        Counter setSampler;             //!< Counter for sampler bindings. \see CommandBuffer::SetSampler
        Counter setRenderTarget;        //!< Counter for render target bindings. \see CommandBuffer::SetRenderTarget

        /**
        \brief Counter for draw calls.
        \see CommandBuffer.Draw
        \see CommandBuffer.DrawIndexed
        \see CommandBuffer.DrawInstanced
        \see CommandBuffer.DrawIndexedInstanced
        */
        Counter drawCalls;
        Counter dispatchComputeCalls;   //!< Counter for dispatch compute calls. \see CommandBuffer::Dispatch

        Counter renderedPoints;         //!< Counter for rendered point primitives.
        Counter renderedLines;          //!< Counter for rendered line primitives.
        Counter renderedTriangles;      //!< Counter for rendered triangle primitives.
        Counter renderedPatches;        //!< Counter for rendered patch primitives.

};


} // /namespace LLGL


#endif



// ================================================================================
