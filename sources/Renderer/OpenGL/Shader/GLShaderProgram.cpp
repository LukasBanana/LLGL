/*
 * GLShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLShaderProgram.h"
#include "GLVertexShader.h"
#include "GLFragmentShader.h"
#include "GLGeometryShader.h"
#include "GLTessControlShader.h"
#include "GLTessEvaluationShader.h"
#include "GLComputeShader.h"
#include "../GLExtensions.h"
#include "../../CheckedCast.h"
#include <vector>
#include <stdexcept>


namespace LLGL
{


GLShaderProgram::GLShaderProgram()
{
    id_ = glCreateProgram();
}

GLShaderProgram::~GLShaderProgram()
{
    glDeleteProgram(id_);
}

void GLShaderProgram::AttachShader(VertexShader& vertexShader)
{
    AttachHWShader<GLVertexShader>(vertexShader);
}

void GLShaderProgram::AttachShader(FragmentShader& fragmentShader)
{
    AttachHWShader<GLFragmentShader>(fragmentShader);
}

void GLShaderProgram::AttachShader(GeometryShader& geometryShader)
{
    AttachHWShader<GLGeometryShader>(geometryShader);
}

void GLShaderProgram::AttachShader(TessControlShader& tessControlShader)
{
    AttachHWShader<GLTessControlShader>(tessControlShader);
}

void GLShaderProgram::AttachShader(TessEvaluationShader& tessEvaluationShader)
{
    AttachHWShader<GLTessEvaluationShader>(tessEvaluationShader);
}

void GLShaderProgram::AttachShader(ComputeShader& computeShader)
{
    AttachHWShader<GLComputeShader>(computeShader);
}

bool GLShaderProgram::LinkShaders()
{
    /* Link shader program */
    glLinkProgram(id_);

    /* Query linking status */
    GLint linkStatus = 0;
    glGetProgramiv(id_, GL_LINK_STATUS, &linkStatus);

    linkStatus_ = (linkStatus != GL_FALSE);

    return linkStatus_;
}

std::string GLShaderProgram::QueryInfoLog()
{
    /* Query info log length */
    GLint infoLogLength = 0;
    glGetProgramiv(id_, GL_INFO_LOG_LENGTH, &infoLogLength);

    if (infoLogLength > 0)
    {
        /* Store info log in byte buffer (because GL writes it's own null-terminator character!) */
        std::vector<char> infoLog;
        infoLog.resize(infoLogLength, '\0');

        /* Query info log output */
        GLsizei charsWritten = 0;
        glGetProgramInfoLog(id_, infoLogLength, &charsWritten, infoLog.data());

        /* Convert byte buffer to string */
        return std::string(infoLog.data());
    }

    return "";
}

std::vector<ConstantBufferDescriptor> GLShaderProgram::QueryConstantBuffers() const
{
    std::vector<ConstantBufferDescriptor> descList;

    /* Query number of uniform blocks */
    GLint numUniformBlocks = 0;
    glGetProgramiv(id_, GL_ACTIVE_UNIFORM_BLOCKS, &numUniformBlocks);

    if (numUniformBlocks <= 0)
        return descList;

    /* Query maximal uniform block name length */
    GLint maxNameLength = 0;
    glGetProgramiv(id_, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &maxNameLength);

    if (maxNameLength <= 0)
        return descList;

    std::vector<char> blockName;
    blockName.resize(maxNameLength);

    /* Iterate over all uniform blocks */
    for (GLuint i = 0; i < static_cast<GLuint>(numUniformBlocks); ++i)
    {
        ConstantBufferDescriptor desc;
        desc.index = i;

        /* Query uniform block name */
        GLsizei nameLength = 0;
        glGetActiveUniformBlockName(id_, i, maxNameLength, &nameLength, blockName.data());
        desc.name = std::string(blockName.data());

        /* Query uniform block size */
        GLint blockSize = 0;
        glGetActiveUniformBlockiv(id_, i, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
        desc.size = blockSize;

        /* Insert uniform block into list */
        descList.push_back(desc);
    }

    return descList;
}

void GLShaderProgram::BindVertexAttributes(const std::vector<VertexAttribute>& vertexAttribs, bool ignoreUnusedAttributes)
{
    if (!linkStatus_)
        throw std::runtime_error("failed to bind vertex attributes, because shaders have not been linked successfully yet");
    
    if (vertexAttribs.size() > GL_MAX_VERTEX_ATTRIBS)
    {
        throw std::invalid_argument(
            "failed to bind vertex attributes, because too many attributes are specified (maximum is " +
            std::to_string(GL_MAX_VERTEX_ATTRIBS) + ")"
        );
    }

    /* Bind all vertex attribute locations */
    for (const auto& attrib : vertexAttribs)
    {
        /* Get and bind attribute location */
        GLint index = glGetAttribLocation(id_, attrib.name.c_str());
        if (index != -1)
            glBindAttribLocation(id_, static_cast<GLuint>(index), attrib.name.c_str());
        else if (!ignoreUnusedAttributes)
            throw std::invalid_argument("failed to bind vertex attribute '" + std::string(attrib.name) + "'");
    }
}

void GLShaderProgram::BindConstantBuffer(const std::string& name, unsigned int bindingIndex)
{
    /* Query uniform block index and bind it to the specified binding index */
    GLint blockIndex = glGetUniformBlockIndex(id_, name.c_str());
    if (blockIndex != GL_INVALID_INDEX)
        glUniformBlockBinding(id_, blockIndex, bindingIndex);
    else
        throw std::invalid_argument("failed to bind constant buffer, because uniform block name is invalid");
}


/*
 * ======= Private: ========
 */

template <typename GLShaderType, typename ShaderType>
void GLShaderProgram::AttachHWShader(const ShaderType& shader)
{
    const auto& shaderGL = LLGL_CAST(const GLShaderType&, shader);
    glAttachShader(id_, shaderGL.hwShader.GetID());
}


} // /namespace LLGL



// ================================================================================
