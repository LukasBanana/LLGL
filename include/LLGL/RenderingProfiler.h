/*
 * RenderingProfiler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_RENDERER_PROFILE_H__
#define __LLGL_RENDERER_PROFILE_H__


#include "Export.h"
#include "RenderContextFlags.h"


namespace LLGL
{


/**
\brief Rendering profiler model class.
\remarks This can be used to profile the renderer draw calls and buffer updates.
*/
class LLGL_EXPORT RenderingProfiler
{

    public:

        class LLGL_EXPORT Counter
        {

            public:

                using ValueType = unsigned int;

                void Inc()
                {
                    ++value_;
                }

                void Inc(ValueType value)
                {
                    value_ += value;
                }

                void Reset()
                {
                    value_ = 0;
                }

                inline ValueType Count() const
                {
                    return value_;
                }

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

        void RecordDrawCall(DrawMode drawMode, Counter::ValueType numVertices);
        void RecordDrawCall(DrawMode drawMode, Counter::ValueType numVertices, Counter::ValueType numInstances);

        Counter writeVertexBuffer;
        Counter writeIndexBuffer;
        Counter writeConstantBuffer;
        Counter writeStorageBuffer;

        Counter writeVertexBufferSub;
        Counter writeIndexBufferSub;
        Counter writeConstantBufferSub;
        Counter writeStorageBufferSub;

        Counter mapConstantBuffer;
        Counter mapStorageBuffer;

        Counter bindVertexBuffer;
        Counter bindIndexBuffer;
        Counter bindConstantBuffer;
        Counter bindStorageBuffer;
        Counter bindGraphicsPipeline;
        Counter bindComputePipeline;
        Counter bindTexture;
        Counter bindSampler;
        Counter bindRenderTarget;

        Counter drawCalls;
        Counter dispatchComputeCalls;

        Counter renderedPoints;
        Counter renderedLines;
        Counter renderedTriangles;

};


} // /namespace LLGL


#endif



// ================================================================================
