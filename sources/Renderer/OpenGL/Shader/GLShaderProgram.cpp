/*
 * GLShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLShaderProgram.h"
#include "GLShader.h"
#include "../Ext/GLExtensions.h"
#include "../../CheckedCast.h"
#include "../RenderState/GLStateManager.h"
#include "../GLTypes.h"
#include <LLGL/Log.h>
#include <LLGL/VertexFormat.h>
#include <vector>
#include <stdexcept>


namespace LLGL
{


GLShaderProgram::GLShaderProgram() :
    id_     ( glCreateProgram() ),
    uniform_( id_               )
{
}

GLShaderProgram::~GLShaderProgram()
{
    glDeleteProgram(id_);
}

void GLShaderProgram::AttachShader(Shader& shader)
{
    const auto& shaderGL = LLGL_CAST(GLShader&, shader);
    glAttachShader(id_, shaderGL.GetID());
}

bool GLShaderProgram::LinkShaders()
{
    /* Link shader program */
    glLinkProgram(id_);

    /* Query linking status */
    GLint linkStatus = 0;
    glGetProgramiv(id_, GL_LINK_STATUS, &linkStatus);

    return (linkStatus != GL_FALSE);
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
        case GL_INT:                Set(DataType::Int32,  1, 1); break;
        case GL_INT_VEC2:           Set(DataType::Int32,  2, 1); break;
        case GL_INT_VEC3:           Set(DataType::Int32,  3, 1); break;
        case GL_INT_VEC4:           Set(DataType::Int32,  4, 1); break;
        case GL_UNSIGNED_INT:       Set(DataType::UInt32, 1, 1); break;
        case GL_UNSIGNED_INT_VEC2:  Set(DataType::UInt32, 2, 1); break;
        case GL_UNSIGNED_INT_VEC3:  Set(DataType::UInt32, 3, 1); break;
        case GL_UNSIGNED_INT_VEC4:  Set(DataType::UInt32, 4, 1); break;
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
        return vertexFormat.attributes;

    /* Query maximal name length of all vertex attributes */
    GLint maxNameLength = 0;
    glGetProgramiv(id_, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxNameLength);
    if (maxNameLength <= 0)
        return vertexFormat.attributes;

    std::vector<char> attribName(maxNameLength, 0);

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
        auto dataType   = DataType::Int8;

        unsigned rows = 0, cols = 0;
        UnmapAttribType(type, dataType, rows, cols);
        auto components = rows*cols;

        /* Insert uniform block into list */
        vertexFormat.AppendAttribute({ name, dataType, components });
    }

    return vertexFormat.attributes;
}

std::vector<ConstantBufferViewDescriptor> GLShaderProgram::QueryConstantBuffers() const
{
    std::vector<ConstantBufferViewDescriptor> descList;

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

    std::vector<char> blockName(maxNameLength, 0);

    /* Iterate over all uniform blocks */
    for (GLuint i = 0; i < static_cast<GLuint>(numUniformBlocks); ++i)
    {
        ConstantBufferViewDescriptor desc;
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

std::vector<StorageBufferViewDescriptor> GLShaderProgram::QueryStorageBuffers() const
{
    std::vector<StorageBufferViewDescriptor> descList;

    #ifndef __APPLE__
    
    /* Query number of shader storage blocks */
    GLenum properties[3] = { 0 };
    properties[0] = GL_NUM_ACTIVE_VARIABLES;
    GLint numStorageBlocks = 0;
    glGetProgramResourceiv(id_, GL_SHADER_STORAGE_BLOCK, 0, 1, properties, 1, nullptr, &numStorageBlocks);
    if (numStorageBlocks <= 0)
        return descList;

    /* Query maximal name length of all shader storage blocks */
    GLint maxNameLength = 0;
    glGetProgramInterfaceiv(id_, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &maxNameLength);
    if (maxNameLength <= 0)
        return descList;

    std::vector<char> blockName(maxNameLength, 0);

    /* Iterate over all shader storage blocks */
    for (GLuint i = 0; i < static_cast<GLuint>(numStorageBlocks); ++i)
    {
        StorageBufferViewDescriptor desc;
        desc.index = i;

        /* Query shader storage block name */
        GLsizei nameLength = 0;
        glGetProgramResourceName(id_, GL_SHADER_STORAGE_BLOCK, i, maxNameLength, &nameLength, blockName.data());
        desc.name = std::string(blockName.data());

        /* Insert shader storage block into list */
        descList.push_back(desc);
    }
    
    #endif

    return descList;
}

std::vector<UniformDescriptor> GLShaderProgram::QueryUniforms() const
{
    std::vector<UniformDescriptor> descList;

    /* Query number of uniforms */
    GLint numUniforms = 0;
    glGetProgramiv(id_, GL_ACTIVE_UNIFORMS, &numUniforms);
    if (numUniforms <= 0)
        return descList;

    /* Query maximal name length of all uniforms */
    GLint maxNameLength = 0;
    glGetProgramiv(id_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);
    if (maxNameLength <= 0)
        return descList;

    std::vector<char> blockName;
    blockName.resize(maxNameLength);

    /* Iterate over all uniform blocks */
    for (GLuint i = 0; i < static_cast<GLuint>(numUniforms); ++i)
    {
        try
        {
            UniformDescriptor desc;

            /* Query uniform block name */
            GLsizei nameLength  = 0;
            GLint   size        = 0;
            GLenum  type        = 0;
        
            glGetActiveUniform(id_, i, maxNameLength, &nameLength, &size, &type, blockName.data());

            desc.name       = std::string(blockName.data());
            desc.location   = glGetUniformLocation(id_, blockName.data());
            desc.size       = static_cast<unsigned int>(size);

            GLTypes::Unmap(desc.type, type);

            /* Insert uniform block into list */
            descList.push_back(desc);
        }
        catch (const std::exception& e)
        {
            Log::StdOut() << e.what() << std::endl;
        }
    }

    return descList;
}

void GLShaderProgram::BuildInputLayout(const std::vector<VertexAttribute>& vertexAttribs)
{
    if (vertexAttribs.size() > GL_MAX_VERTEX_ATTRIBS)
    {
        throw std::invalid_argument(
            "failed to bind vertex attributes, because too many attributes are specified (maximum is " +
            std::to_string(GL_MAX_VERTEX_ATTRIBS) + ")"
        );
    }

    /* Bind all vertex attribute locations */
    GLuint index = 0;

    for (const auto& attrib : vertexAttribs)
    {
        /* Bind attribute location */
        glBindAttribLocation(id_, index, attrib.name.c_str());
        ++index;
    }
}

void GLShaderProgram::BindConstantBuffer(const std::string& name, unsigned int bindingIndex)
{
    /* Query uniform block index and bind it to the specified binding index */
    auto blockIndex = glGetUniformBlockIndex(id_, name.c_str());
    if (blockIndex != GL_INVALID_INDEX)
        glUniformBlockBinding(id_, blockIndex, bindingIndex);
    else
        throw std::invalid_argument("failed to bind constant buffer, because uniform block name is invalid");
}

void GLShaderProgram::BindStorageBuffer(const std::string& name, unsigned int bindingIndex)
{
    #ifndef __APPLE__
    /* Query shader storage block index and bind it to the specified binding index */
    auto blockIndex = glGetProgramResourceIndex(id_, GL_SHADER_STORAGE_BLOCK, name.c_str());
    if (blockIndex != GL_INVALID_INDEX)
        glShaderStorageBlockBinding(id_, blockIndex, bindingIndex);
    else
        throw std::invalid_argument("failed to bind storage buffer, because storage block name is invalid");
    #else
    throw std::runtime_error("storage buffers not supported on this platform");
    #endif
}

ShaderUniform* GLShaderProgram::LockShaderUniform()
{
    GLStateManager::active->PushShaderProgram();
    GLStateManager::active->BindShaderProgram(id_);
    return (&uniform_);
}

void GLShaderProgram::UnlockShaderUniform()
{
    GLStateManager::active->PopShaderProgram();
}


} // /namespace LLGL



// ================================================================================
