/*
 * GLShader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLShader.h"
#include "GLShaderSourcePatcher.h"
#include "../GLObjectUtils.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/Exception.h"
#include "../../../Core/ReportUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <vector>
#include <stdexcept>
#include <algorithm>


namespace LLGL
{


GLShader::GLShader(const bool isSeparable, const ShaderDescriptor& desc) :
    Shader       { desc.type   },
    isSeparable_ { isSeparable }
{
    ReserveAttribs(desc);
    BuildVertexInputLayout(desc.vertex.inputAttribs.size(), desc.vertex.inputAttribs.data());
    BuildTransformFeedbackVaryings(desc.vertex.outputAttribs.size(), desc.vertex.outputAttribs.data());
    BuildFragmentOutputLayout(desc.fragment.outputAttribs.size(), desc.fragment.outputAttribs.data());
}

const Report* GLShader::GetReport() const
{
    return (report_ ? &report_ : nullptr);
}

const GLShaderAttribute* GLShader::GetVertexAttribs() const
{
    if (!shaderAttribs_.empty())
        return &shaderAttribs_[0];
    else
        return nullptr;
}

std::size_t GLShader::GetNumVertexAttribs() const
{
    return numVertexAttribs_;
}

const GLShaderAttribute* GLShader::GetFragmentAttribs() const
{
    if (numVertexAttribs_ < shaderAttribs_.size())
        return &shaderAttribs_[numVertexAttribs_];
    else
        return nullptr;
}

std::size_t GLShader::GetNumFragmentAttribs() const
{
    return (shaderAttribs_.size() - numVertexAttribs_);
}

void GLShader::PatchShaderSource(
    const ShaderSourceCallback& sourceCallback,
    const ShaderDescriptor&     shaderDesc)
{
    /* Generate statement to flip vertex Y-coordinate if requested */
    const char* vertexTransformStmt = nullptr;
    if ((shaderDesc.flags & ShaderCompileFlags::PatchClippingOrigin) != 0)
    {
        if (shaderDesc.type == ShaderType::Vertex ||
            shaderDesc.type == ShaderType::TessEvaluation ||
            shaderDesc.type == ShaderType::Geometry)
        {
            vertexTransformStmt = "gl_Position.y = -gl_Position.y;";
        }
    }

    /* Add '#pragma optimize(off)'-directive to source if optimization is disabled */
    const bool pragmaOptimizeOff = ((shaderDesc.flags & ShaderCompileFlags::NoOptimization) != 0);

    /* Get source code */
    if (shaderDesc.sourceType == ShaderSourceType::CodeFile)
    {
        const std::string fileContent = ReadFileString(shaderDesc.source);
        GLShader::PatchShaderSourceWithOptions(sourceCallback, fileContent.c_str(), shaderDesc.defines, pragmaOptimizeOff, vertexTransformStmt);
    }
    else
        GLShader::PatchShaderSourceWithOptions(sourceCallback, shaderDesc.source, shaderDesc.defines, pragmaOptimizeOff, vertexTransformStmt);
}

void GLShader::PatchShaderSourceWithOptions(
    const ShaderSourceCallback& sourceCallback,
    const char*                 source,
    const ShaderMacro*          defines,
    bool                        pragmaOptimizeOff,
    const char*                 vertexTransformStmt)
{
    if (sourceCallback)
    {
        const bool hasDefines = (defines != nullptr && defines->name != nullptr);
        const bool hasVertexStmt = (vertexTransformStmt != nullptr);
        if (hasDefines || pragmaOptimizeOff || hasVertexStmt)
        {
            GLShaderSourcePatcher patcher{ source };
            patcher.AddDefines(defines);
            if (pragmaOptimizeOff)
                patcher.AddPragmaDirective("optimize(off)");
            patcher.AddFinalVertexTransformStatements(vertexTransformStmt);
            sourceCallback(patcher.GetSource());
        }
        else
            sourceCallback(source);
    }
}

void GLShader::ReportStatusAndLog(bool status, const std::string& log)
{
    ResetReportWithNewline(report_, log.c_str(), !status);
}


/*
 * ======= Private: =======
 */

void GLShader::ReserveAttribs(const ShaderDescriptor& desc)
{
    /* Reset attributes and their name storage */
    shaderAttribs_.clear();
    shaderAttribNames_.Clear();

    /* Reserve names for vertex attributs */
    for (const auto& attr : desc.vertex.inputAttribs)
    {
        if (attr.semanticIndex == 0)
        {
            shaderAttribNames_.Reserve(attr.name.size());
            ++numVertexAttribs_;
        }
    }

    /* Reserve names for transform feedback varyings */
    for (const auto& attr : desc.vertex.outputAttribs)
        shaderAttribNames_.Reserve(attr.name.size());

    /* Reserve names for fragment output attributes */
    for (const auto& attr : desc.fragment.outputAttribs)
        shaderAttribNames_.Reserve(attr.name.size());

    /* Reserve memory for vertex input and fragment output attributes */
    shaderAttribs_.reserve(numVertexAttribs_ + desc.fragment.outputAttribs.size());
}

void GLShader::BuildVertexInputLayout(std::size_t numVertexAttribs, const VertexAttribute* vertexAttribs)
{
    if (numVertexAttribs == 0 || vertexAttribs == nullptr)
        return;

    /* Validate maximal number of vertex attributes (OpenGL supports at least 8 vertex attribute) */
    constexpr std::size_t minSupportedVertexAttribs = 8;

    std::size_t highestAttribIndex = 0;
    for_range(i, numVertexAttribs)
        highestAttribIndex = std::max(highestAttribIndex, static_cast<std::size_t>(vertexAttribs[i].location));

    if (highestAttribIndex > minSupportedVertexAttribs)
    {
        GLint maxSupportedVertexAttribs = 0;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxSupportedVertexAttribs);

        if (highestAttribIndex > static_cast<std::size_t>(maxSupportedVertexAttribs))
        {
            throw std::invalid_argument(
                "failed build input layout, because too many vertex attributes are specified (" +
                std::to_string(highestAttribIndex) + " is specified, but maximum is " + std::to_string(maxSupportedVertexAttribs) + ")"
            );
        }
    }

    /* Bind all vertex attribute locations */
    for_range(i, numVertexAttribs)
    {
        /* Store attribute meta data (matrices only use the 1st column) */
        const auto& attr = vertexAttribs[i];
        if (attr.semanticIndex == 0)
            shaderAttribs_.push_back({ attr.location, shaderAttribNames_.CopyString(attr.name) });
    }
}

void GLShader::BuildFragmentOutputLayout(std::size_t numFragmentAttribs, const FragmentAttribute* fragmentAttribs)
{
    if (numFragmentAttribs == 0 || fragmentAttribs == nullptr)
        return;

    /* Bind all fragment attribute locations */
    for_range(i, numFragmentAttribs)
    {
        /* Store attribute meta data (matrices only use the 1st column) */
        const auto& attr = fragmentAttribs[i];
        shaderAttribs_.push_back({ attr.location, shaderAttribNames_.CopyString(attr.name) });
    }
}

void GLShader::BuildTransformFeedbackVaryings(std::size_t numVaryings, const VertexAttribute* varyings)
{
    if (numVaryings > 0 && varyings != nullptr)
    {
        transformFeedbackVaryings_.reserve(numVaryings);
        for_range(i, numVaryings)
            transformFeedbackVaryings_.push_back(shaderAttribNames_.CopyString(varyings[i].name));
    }
}


} // /namespace LLGL



// ================================================================================
