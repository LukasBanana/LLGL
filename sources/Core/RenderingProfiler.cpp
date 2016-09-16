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

void RenderingProfiler::RecordDrawCall(DrawMode drawMode, Counter::ValueType numVertices)
{
    drawCalls.Inc();

    switch (drawMode)
    {
        case DrawMode::Points:
            renderedPoints.Inc(numVertices);
            break;

        case DrawMode::Lines:
            renderedPoints.Inc(numVertices / 2);
            break;

        case DrawMode::LineStrip:
            if (numVertices >= 2)
                renderedPoints.Inc(numVertices - 1);
            break;

        case DrawMode::LineLoop:
            if (numVertices == 2)
                renderedPoints.Inc(1);
            else if (numVertices >= 3)
                renderedPoints.Inc(numVertices);
            break;

        case DrawMode::LinesAdjacency:
            renderedPoints.Inc(numVertices / 2);
            break;

        case DrawMode::LineStripAdjacency:
            if (numVertices >= 2)
                renderedPoints.Inc(numVertices - 1);
            break;

        case DrawMode::Triangles:
            renderedTriangles.Inc(numVertices / 3);
            break;

        case DrawMode::TriangleStrip:
        case DrawMode::TriangleFan:
            if (numVertices >= 3)
                renderedTriangles.Inc(numVertices - 2);
            break;

        case DrawMode::TrianglesAdjacency:
            renderedTriangles.Inc(numVertices / 3);
            break;

        case DrawMode::TriangleStripAdjacency:
            if (numVertices >= 3)
                renderedTriangles.Inc(numVertices - 2);
            break;

        default:
            if (drawMode >= DrawMode::Patches1 && drawMode <= DrawMode::Patches32)
            {
                auto numPatchVertices = static_cast<unsigned int>(drawMode) - static_cast<unsigned int>(DrawMode::Patches1) + 1;
                renderedPatches.Inc(numVertices / numPatchVertices);
            }
            break;
    }
}

void RenderingProfiler::RecordDrawCall(DrawMode drawMode, Counter::ValueType numVertices, Counter::ValueType numInstances)
{
    drawCalls.Inc();

    switch (drawMode)
    {
        case DrawMode::Points:
            renderedPoints.Inc(numVertices * numInstances);
            break;

        case DrawMode::Lines:
            renderedPoints.Inc((numVertices / 2) * numInstances);
            break;

        case DrawMode::LineStrip:
            if (numVertices >= 2)
                renderedPoints.Inc((numVertices - 1) * numInstances);
            break;

        case DrawMode::LineLoop:
            if (numVertices == 2)
                renderedPoints.Inc(numInstances);
            else if (numVertices >= 3)
                renderedPoints.Inc(numVertices * numInstances);
            break;

        case DrawMode::LinesAdjacency:
            renderedPoints.Inc((numVertices / 2) * numInstances);
            break;

        case DrawMode::LineStripAdjacency:
            if (numVertices >= 2)
                renderedPoints.Inc((numVertices - 1) * numInstances);
            break;

        case DrawMode::Triangles:
            renderedTriangles.Inc((numVertices / 3) * numInstances);
            break;

        case DrawMode::TriangleStrip:
        case DrawMode::TriangleFan:
            if (numVertices >= 3)
                renderedTriangles.Inc((numVertices - 2) * numInstances);
            break;

        case DrawMode::TrianglesAdjacency:
            renderedTriangles.Inc((numVertices / 3) * numInstances);
            break;

        case DrawMode::TriangleStripAdjacency:
            if (numVertices >= 3)
                renderedTriangles.Inc((numVertices - 2) * numInstances);
            break;

        default:
            if (drawMode >= DrawMode::Patches1 && drawMode <= DrawMode::Patches32)
            {
                auto numPatchVertices = static_cast<unsigned int>(drawMode) - static_cast<unsigned int>(DrawMode::Patches1) + 1;
                renderedPatches.Inc((numVertices / numPatchVertices) * numInstances);
            }
            break;
    }
}


} // /namespace LLGL



// ================================================================================
