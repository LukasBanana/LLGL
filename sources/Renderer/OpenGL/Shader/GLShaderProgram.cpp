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
#include <LLGL/VertexFormat.h>
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

static void UnmapAttribType(GLenum type, DataType& dataType, unsigned int& rows, unsigned int& cols)
{
    auto Set = [&](DataType t, unsigned int r, unsigned int c)
    {
        dataType    = t;
        rows        = r;
        cols        = c;
    };

    switch (type)
    {
        case GL_FLOAT:              Set(DataType::Float,  1, 1); break;
        case GL_FLOAT_VEC2:         Set(DataType::Float,  2, 1); break;
        case GL_FLOAT_VEC3:         Set(DataType::Float,  3, 1); break;
        case GL_FLOAT_VEC4:         Set(DataType::Float,  4, 1); break;
        case GL_FLOAT_MAT2:         Set(DataType::Float,  2, 2); break;
        case GL_FLOAT_MAT3:         Set(DataType::Float,  3, 3); break;
        case GL_FLOAT_MAT4:         Set(DataType::Float,  4, 4); break;
        case GL_FLOAT_MAT2x3:       Set(DataType::Float,  2, 3); break;
        case GL_FLOAT_MAT2x4:       Set(DataType::Float,  2, 4); break;
        case GL_FLOAT_MAT3x2:       Set(DataType::Float,  3, 2); break;
        case GL_FLOAT_MAT3x4:       Set(DataType::Float,  3, 4); break;
        case GL_FLOAT_MAT4x2:       Set(DataType::Float,  4, 2); break;
        case GL_FLOAT_MAT4x3:       Set(DataType::Float,  4, 3); break;
        case GL_INT:                Set(DataType::Int,    1, 1); break;
        case GL_INT_VEC2:           Set(DataType::Int,    2, 1); break;
        case GL_INT_VEC3:           Set(DataType::Int,    3, 1); break;
        case GL_INT_VEC4:           Set(DataType::Int,    4, 1); break;
        case GL_UNSIGNED_INT:       Set(DataType::UInt,   1, 1); break;
        case GL_UNSIGNED_INT_VEC2:  Set(DataType::UInt,   2, 1); break;
        case GL_UNSIGNED_INT_VEC3:  Set(DataType::UInt,   3, 1); break;
        case GL_UNSIGNED_INT_VEC4:  Set(DataType::UInt,   4, 1); break;
        case GL_DOUBLE:             Set(DataType::Double, 1, 1); break;
        case GL_DOUBLE_VEC2:        Set(DataType::Double, 2, 1); break;
        case GL_DOUBLE_VEC3:        Set(DataType::Double, 3, 1); break;
        case GL_DOUBLE_VEC4:        Set(DataType::Double, 4, 1); break;
        case GL_DOUBLE_MAT2:        Set(DataType::Double, 2, 2); break;
        case GL_DOUBLE_MAT3:        Set(DataType::Double, 3, 3); break;
        case GL_DOUBLE_MAT4:        Set(DataType::Double, 4, 4); break;
        case GL_DOUBLE_MAT2x3:      Set(DataType::Double, 2, 3); break;
        case GL_DOUBLE_MAT2x4:      Set(DataType::Double, 2, 4); break;
        case GL_DOUBLE_MAT3x2:      Set(DataType::Double, 3, 2); break;
        case GL_DOUBLE_MAT3x4:      Set(DataType::Double, 3, 4); break;
        case GL_DOUBLE_MAT4x2:      Set(DataType::Double, 4, 2); break;
        case GL_DOUBLE_MAT4x3:      Set(DataType::Double, 4, 3); break;
    }
}

std::vector<VertexAttribute> GLShaderProgram::QueryVertexAttributes() const
{
    VertexFormat vertexFormat;

    /* Query number of vertex attributes */
    GLint numVertexAttribs = 0;
    glGetProgramiv(id_, GL_ACTIVE_ATTRIBUTES, &numVertexAttribs);

    if (numVertexAttribs <= 0)
        return vertexFormat.GetAttributes();

    /* Query maximal name length of all vertex attributes */
    GLint maxNameLength = 0;
    glGetProgramiv(id_, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxNameLength);

    if (maxNameLength <= 0)
        return vertexFormat.GetAttributes();

    std::vector<char> attribName;
    attribName.resize(maxNameLength);

    /* Iterate over all vertex attributes */
    for (GLuint i = 0; i < static_cast<GLuint>(numVertexAttribs); ++i)
    {
        /* Query attribute information */
        GLint   size        = 0;
        GLenum  type        = 0;
        GLsizei nameLength  = 0;

        glGetActiveAttrib(id_, i, maxNameLength, &nameLength, &size, &type, attribName.data());

        /* Convert attribute information */
        auto name       = std::string(attribName.data());
        auto dataType   = DataType::Byte;

        unsigned rows = 0, cols = 0;
        UnmapAttribType(type, dataType, rows, cols);
        auto components = rows*cols;

        /* Insert uniform block into list */
        vertexFormat.AddAttribute(name, dataType, components);
    }

    return vertexFormat.GetAttributes();
}

std::vector<ConstantBufferDescriptor> GLShaderProgram::QueryConstantBuffers() const
{
    std::vector<ConstantBufferDescriptor> descList;

    /* Query number of uniform blocks */
    GLint numUniformBlocks = 0;
    glGetProgramiv(id_, GL_ACTIVE_UNIFORM_BLOCKS, &numUniformBlocks);

    if (numUniformBlocks <= 0)
        return descList;

    /* Query maximal name length of all uniform blocks */
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
