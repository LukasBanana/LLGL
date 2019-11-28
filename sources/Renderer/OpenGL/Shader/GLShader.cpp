/*
 * GLShader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLShader.h"
#include "../GLObjectUtils.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../GLTypes.h"
#include "../../../Core/Helper.h"
#include "../../../Core/Exception.h"
#include <vector>
#include <sstream>
#include <stdexcept>
#include <algorithm>


namespace LLGL
{


GLShader::GLShader(const ShaderDescriptor& desc) :
    Shader { desc.type }
{
    /* Create shader and  */
    id_ = glCreateShader(GLTypes::Map(desc.type));
    BuildShader(desc);
    ReserveAttribs(desc);
    BuildVertexInputLayout(desc.vertex.inputAttribs.size(), desc.vertex.inputAttribs.data());
    BuildTransformFeedbackVaryings(desc.vertex.outputAttribs.size(), desc.vertex.outputAttribs.data());
    BuildFragmentOutputLayout(desc.fragment.outputAttribs.size(), desc.fragment.outputAttribs.data());
}

GLShader::~GLShader()
{
    glDeleteShader(id_);
}

void GLShader::SetName(const char* name)
{
    GLSetObjectLabel(GL_SHADER, GetID(), name);
}

bool GLShader::HasErrors() const
{
    GLint status = 0;
    glGetShaderiv(id_, GL_COMPILE_STATUS, &status);
    return (status == GL_FALSE);
}

std::string GLShader::GetReport() const
{
    /* Query info log length */
    GLint infoLogLength = 0;
    glGetShaderiv(id_, GL_INFO_LOG_LENGTH, &infoLogLength);

    if (infoLogLength > 0)
    {
        /* Store info log in byte buffer (because GL writes it's own null-terminator character!) */
        std::vector<char> infoLog;
        infoLog.resize(infoLogLength, '\0');

        /* Query info log output */
        GLsizei charsWritten = 0;
        glGetShaderInfoLog(id_, infoLogLength, &charsWritten, infoLog.data());

        /* Convert byte buffer to string */
        return std::string(infoLog.data());
    }

    return "";
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


/*
 * ======= Private: =======
 */

void GLShader::BuildShader(const ShaderDescriptor& shaderDesc)
{
    if (IsShaderSourceCode(shaderDesc.sourceType))
        CompileSource(shaderDesc);
    else
        LoadBinary(shaderDesc);
}

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
    static const std::size_t minSupportedVertexAttribs = 8;

    std::size_t highestAttribIndex = 0;
    for (std::size_t i = 0; i < numVertexAttribs; ++i)
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
    for (std::size_t i = 0; i < numVertexAttribs; ++i)
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
    for (std::size_t i = 0; i < numFragmentAttribs; ++i)
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
        for (std::size_t i = 0; i < numVaryings; ++i)
            transformFeedbackVaryings_.push_back(shaderAttribNames_.CopyString(varyings[i].name));
    }
}

void GLShader::CompileSource(const ShaderDescriptor& shaderDesc)
{
    /* Get source code */
    std::string fileContent;
    const GLchar* strings[1];

    if (shaderDesc.sourceType == ShaderSourceType::CodeFile)
    {
        fileContent = ReadFileString(shaderDesc.source);
        strings[0]  = fileContent.c_str();
    }
    else
    {
        strings[0] = shaderDesc.source;
    }

    /* Load shader source code, then compile shader */
    glShaderSource(id_, 1, strings, nullptr);
    glCompileShader(id_);
}

void GLShader::LoadBinary(const ShaderDescriptor& shaderDesc)
{
    #if defined GL_ARB_gl_spirv && defined GL_ARB_ES2_compatibility
    if (HasExtension(GLExt::ARB_gl_spirv) && HasExtension(GLExt::ARB_ES2_compatibility))
    {
        /* Get shader binary */
        std::vector<char>   fileContent;
        const void*         binaryBuffer    = nullptr;
        GLsizei             binaryLength    = 0;

        if (shaderDesc.sourceType == ShaderSourceType::BinaryFile)
        {
            /* Load binary from file */
            fileContent = ReadFileBuffer(shaderDesc.source);
            binaryBuffer = fileContent.data();
            binaryLength = static_cast<GLsizei>(fileContent.size());
        }
        else
        {
            /* Load binary from buffer */
            binaryBuffer = shaderDesc.source;
            binaryLength = static_cast<GLsizei>(shaderDesc.sourceSize);
        }

        /* Load shader binary */
        glShaderBinary(1, &id_, GL_SHADER_BINARY_FORMAT_SPIR_V, binaryBuffer, binaryLength);

        /* Specialize for the default "main" function in a SPIR-V module  */
        const char* entryPoint = (shaderDesc.entryPoint == nullptr || *shaderDesc.entryPoint == '\0' ? "main" : shaderDesc.entryPoint);
        glSpecializeShader(id_, entryPoint, 0, nullptr, nullptr);
    }
    else
    #endif
    {
        ThrowNotSupportedExcept(__FUNCTION__, "loading binary shader");
    }
}


} // /namespace LLGL



// ================================================================================
