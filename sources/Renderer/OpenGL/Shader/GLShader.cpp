/*
 * GLShader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLShader.h"
#include "GLShaderSourcePatcher.h"
#include "../GLTypes.h"
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

GLenum GLShader::GetGLType() const
{
    return GLTypes::Map(GetType());
}

bool GLShader::NeedsPermutationFlippedYPosition(const ShaderType shaderType, long shaderFlags)
{
    /* If GL_ARB_clip_control is supported, emulating this feature via shader permutation is not necessary */
    if (!HasExtension(GLExt::ARB_clip_control))
    {
        /* Is this shader permutation enabled for this shader? */
        if ((shaderFlags & ShaderCompileFlags::PatchClippingOrigin) != 0)
        {
            /* Is this a shader type that modifies gl_Position? */
            if (shaderType == ShaderType::Vertex ||
                shaderType == ShaderType::TessEvaluation ||
                shaderType == ShaderType::Geometry)
            {
                return true;
            }
        }
    }
    return false;
}

bool GLShader::HasAnyShaderPermutation(Permutation permutation, const ArrayView<Shader*>& shaders)
{
    if (!shaders.empty())
    {
        if (permutation == PermutationDefault)
        {
            /* Every GLShader must have at least the default permutation */
            return true;
        }
        for (const Shader* shader : shaders)
        {
            auto* shaderGL = LLGL_CAST(const GLShader*, shader);
            if (shaderGL->GetID(permutation) != 0)
            {
                /* Found GLShader with requested permutation */
                return true;
            }
        }
    }
    return false;
}

void GLShader::PatchShaderSource(
    const ShaderSourceCallback& sourceCallback,
    const char*                 shaderSource,
    const ShaderDescriptor&     shaderDesc,
    long                        enabledFlags)
{
    const long shaderFlags = (shaderDesc.flags & enabledFlags);

    /* Generate statement to flip vertex Y-coordinate if requested */
    const char* vertexTransformStmt = nullptr;
    if (GLShader::NeedsPermutationFlippedYPosition(shaderDesc.type, shaderFlags))
        vertexTransformStmt = "gl_Position.y = -gl_Position.y;";

    /* Add '#pragma optimize(off)'-directive to source if optimization is disabled */
    const bool pragmaOptimizeOff = ((shaderFlags & ShaderCompileFlags::NoOptimization) != 0);

    /* Get source code */
    GLShader::PatchShaderSourceWithOptions(
        /*sourceCallback:*/         sourceCallback,
        /*source:*/                 shaderSource,
        /*defines:*/                shaderDesc.defines,
        /*pragmaOptimizeOff:*/      pragmaOptimizeOff,
        /*vertexTransformStmt:*/    vertexTransformStmt,
        /*versionOverride:*/        shaderDesc.profile
    );
}

void GLShader::PatchShaderSourceWithOptions(
    const ShaderSourceCallback& sourceCallback,
    const char*                 source,
    const ShaderMacro*          defines,
    bool                        pragmaOptimizeOff,
    const char*                 vertexTransformStmt,
    const char*                 versionOverride)
{
    if (sourceCallback)
    {
        const bool hasDefines           = (defines != nullptr && defines->name != nullptr);
        const bool hasVertexStmt        = (vertexTransformStmt != nullptr);
        const bool hasVersionOverride   = (versionOverride != nullptr && *versionOverride != '\0');
        if (hasDefines || pragmaOptimizeOff || hasVertexStmt || hasVersionOverride)
        {
            GLShaderSourcePatcher patcher{ source };
            if (hasVersionOverride)
                patcher.OverrideVersion(versionOverride);
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

bool GLShader::BuildVertexInputLayout(std::size_t numVertexAttribs, const VertexAttribute* vertexAttribs)
{
    if (numVertexAttribs == 0 || vertexAttribs == nullptr)
        return true;

    /* Validate maximal number of vertex attributes (OpenGL supports at least 8 vertex attribute) */
    constexpr std::uint32_t minSupportedVertexAttribs = 8;

    std::uint32_t highestAttribIndex = 0;
    for_range(i, numVertexAttribs)
        highestAttribIndex = std::max(highestAttribIndex, vertexAttribs[i].location);

    if (highestAttribIndex > minSupportedVertexAttribs)
    {
        GLint maxSupportedVertexAttribs = 0;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxSupportedVertexAttribs);

        if (highestAttribIndex > static_cast<std::uint32_t>(maxSupportedVertexAttribs))
        {
            report_.Errorf(
                "failed build input layout, because too many vertex attributes are specified (%u is specified, but maximum is %u)",
                highestAttribIndex, maxSupportedVertexAttribs
            );
            return false;
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

    return true;
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
        {
            if (const char* systemValueName = GLTypes::SystemValueToString(varyings[i].systemValue, GetType()))
                transformFeedbackVaryings_.push_back(systemValueName);
            else
                transformFeedbackVaryings_.push_back(shaderAttribNames_.CopyString(varyings[i].name));
        }
    }
}


} // /namespace LLGL



// ================================================================================
