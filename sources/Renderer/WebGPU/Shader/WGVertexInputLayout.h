/*
 * WGVertexInput.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_VERTEX_INPUT_H
#define LLGL_WG_VERTEX_INPUT_H


#include <LLGL/VertexAttribute.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Report.h>
#include <webgpu/webgpu.h>
#include <vector>
#include <memory>


namespace LLGL
{


// Container class for WebGPU vertex attributes and vertex buffer layouts.
class WGVertexInputLayout
{

    public:

        WGVertexInputLayout() = default;

        WGVertexInputLayout(std::vector<VertexAttribute> attribs, Report* outReport = nullptr);

        bool BuildVertexBufferLayouts(std::vector<VertexAttribute> attribs, Report* outReport = nullptr);

        inline const std::vector<WGPUVertexBufferLayout>& GetVertexBufferLayouts() const
        {
            return vertexBufferLayouts_;
        }

    private:

        std::vector<WGPUVertexAttribute>    vertexAttribs_;
        std::vector<WGPUVertexBufferLayout> vertexBufferLayouts_;

};

using WGVertexInputLayoutPtr = std::unique_ptr<WGVertexInputLayout>;


} // /namespace LLGL


#endif



// ================================================================================
