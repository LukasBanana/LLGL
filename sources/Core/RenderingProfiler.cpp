/*
 * RenderingProfiler.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/RenderingProfiler.h>


namespace LLGL
{


void RenderingProfiler::ResetCounters()
{
    writeVertexBuffer.Reset();
    writeIndexBuffer.Reset();
    writeConstantBuffer.Reset();
    writeStorageBuffer.Reset();

    mapConstantBuffer.Reset();
    mapStorageBuffer.Reset();

    setVertexBuffer.Reset();
    setIndexBuffer.Reset();
    setConstantBuffer.Reset();
    setStorageBuffer.Reset();
    setGraphicsPipeline.Reset();
    setComputePipeline.Reset();
    setTexture.Reset();
    setSampler.Reset();
    setRenderTarget.Reset();

    drawCalls.Reset();
    dispatchComputeCalls.Reset();

    renderedPoints.Reset();
    renderedLines.Reset();
    renderedTriangles.Reset();
    renderedPatches.Reset();
}

void RenderingProfiler::RecordDrawCall(const PrimitiveTopology topology, Counter::ValueType numVertices)
{
    drawCalls.Inc();

    switch (topology)
    {
        case PrimitiveTopology::PointList:
            renderedPoints.Inc(numVertices);
            break;

        case PrimitiveTopology::LineList:
            renderedPoints.Inc(numVertices / 2);
            break;

        case PrimitiveTopology::LineStrip:
            if (numVertices >= 2)
                renderedPoints.Inc(numVertices - 1);
            break;

        case PrimitiveTopology::LineLoop:
            if (numVertices == 2)
                renderedPoints.Inc(1);
            else if (numVertices >= 3)
                renderedPoints.Inc(numVertices);
            break;

        case PrimitiveTopology::LineListAdjacency:
            renderedPoints.Inc(numVertices / 2);
            break;

        case PrimitiveTopology::LineStripAdjacency:
            if (numVertices >= 2)
                renderedPoints.Inc(numVertices - 1);
            break;

        case PrimitiveTopology::TriangleList:
            renderedTriangles.Inc(numVertices / 3);
            break;

        case PrimitiveTopology::TriangleStrip:
        case PrimitiveTopology::TriangleFan:
            if (numVertices >= 3)
                renderedTriangles.Inc(numVertices - 2);
            break;

        case PrimitiveTopology::TriangleListAdjacency:
            renderedTriangles.Inc(numVertices / 3);
            break;

        case PrimitiveTopology::TriangleStripAdjacency:
            if (numVertices >= 3)
                renderedTriangles.Inc(numVertices - 2);
            break;

        default:
            if (topology >= PrimitiveTopology::Patches1 && topology <= PrimitiveTopology::Patches32)
            {
                auto numPatchVertices = static_cast<unsigned int>(topology) - static_cast<unsigned int>(PrimitiveTopology::Patches1) + 1;
                renderedPatches.Inc(numVertices / numPatchVertices);
            }
            break;
    }
}

void RenderingProfiler::RecordDrawCall(const PrimitiveTopology topology, Counter::ValueType numVertices, Counter::ValueType numInstances)
{
    drawCalls.Inc();

    switch (topology)
    {
        case PrimitiveTopology::PointList:
            renderedPoints.Inc(numVertices * numInstances);
            break;

        case PrimitiveTopology::LineList:
            renderedPoints.Inc((numVertices / 2) * numInstances);
            break;

        case PrimitiveTopology::LineStrip:
            if (numVertices >= 2)
                renderedPoints.Inc((numVertices - 1) * numInstances);
            break;

        case PrimitiveTopology::LineLoop:
            if (numVertices == 2)
                renderedPoints.Inc(numInstances);
            else if (numVertices >= 3)
                renderedPoints.Inc(numVertices * numInstances);
            break;

        case PrimitiveTopology::LineListAdjacency:
            renderedPoints.Inc((numVertices / 2) * numInstances);
            break;

        case PrimitiveTopology::LineStripAdjacency:
            if (numVertices >= 2)
                renderedPoints.Inc((numVertices - 1) * numInstances);
            break;

        case PrimitiveTopology::TriangleList:
            renderedTriangles.Inc((numVertices / 3) * numInstances);
            break;

        case PrimitiveTopology::TriangleStrip:
        case PrimitiveTopology::TriangleFan:
            if (numVertices >= 3)
                renderedTriangles.Inc((numVertices - 2) * numInstances);
            break;

        case PrimitiveTopology::TriangleListAdjacency:
            renderedTriangles.Inc((numVertices / 3) * numInstances);
            break;

        case PrimitiveTopology::TriangleStripAdjacency:
            if (numVertices >= 3)
                renderedTriangles.Inc((numVertices - 2) * numInstances);
            break;

        default:
            if (topology >= PrimitiveTopology::Patches1 && topology <= PrimitiveTopology::Patches32)
            {
                auto numPatchVertices = static_cast<unsigned int>(topology) - static_cast<unsigned int>(PrimitiveTopology::Patches1) + 1;
                renderedPatches.Inc((numVertices / numPatchVertices) * numInstances);
            }
            break;
    }
}


} // /namespace LLGL



// ================================================================================
