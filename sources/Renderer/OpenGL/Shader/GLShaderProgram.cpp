/*
 * GLShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLShaderProgram.h"
#include "GLShader.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionLoader.h"
#include "../../CheckedCast.h"
#include "../../../Core/Exception.h"
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
    auto& shaderGL = LLGL_CAST(GLShader&, shader);

    /* Attach shader to shader program */
    glAttachShader(id_, shaderGL.GetID());

    /* Store attribute if fragment shader is set */
    if (shader.GetType() == ShaderType::Fragment)
        hasFragmentShader_ = true;

    /* Move stream-output format from shader to shader program (if available) */
    shaderGL.MoveStreamOutputFormat(streamOutputFormat_);
}

bool GLShaderProgram::LinkShaders()
{
    /* Check if transform-feedback varyings must be specified (before or after shader linking) */
    if (!streamOutputFormat_.attributes.empty())
    {
        /* For GL_EXT_transform_feedback the varyings must be specified BEFORE linking */
        if (HasExtension(GLExt::EXT_transform_feedback))
        {
            SpecifyTransformFeedbackVaryingsEXT(streamOutputFormat_.attributes);
            return LinkShaderProgram();
        }

        /* For GL_NV_transform_feedback (Vendor specific) the varyings must be specified AFTER linking */
        if (HasExtension(GLExt::NV_transform_feedback))
        {
            auto result = LinkShaderProgram();
            SpecifyTransformFeedbackVaryingsNV(streamOutputFormat_.attributes);
            return result;
        }
    }

    /* Just link shader program */
    return LinkShaderProgram();
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

static std::pair<VectorType, unsigned int> UnmapAttribType(GLenum type)
{
    switch (type)
    {
        case GL_FLOAT:              return { VectorType::Float,   1 };
        case GL_FLOAT_VEC2:         return { VectorType::Float2,  1 };
        case GL_FLOAT_VEC3:         return { VectorType::Float3,  1 };
        case GL_FLOAT_VEC4:         return { VectorType::Float4,  1 };
        case GL_FLOAT_MAT2:         return { VectorType::Float2,  2 };
        case GL_FLOAT_MAT3:         return { VectorType::Float3,  3 };
        case GL_FLOAT_MAT4:         return { VectorType::Float4,  4 };
        case GL_FLOAT_MAT2x3:       return { VectorType::Float3,  2 };
        case GL_FLOAT_MAT2x4:       return { VectorType::Float4,  2 };
        case GL_FLOAT_MAT3x2:       return { VectorType::Float2,  3 };
        case GL_FLOAT_MAT3x4:       return { VectorType::Float4,  3 };
        case GL_FLOAT_MAT4x2:       return { VectorType::Float2,  4 };
        case GL_FLOAT_MAT4x3:       return { VectorType::Float3,  4 };
        case GL_INT:                return { VectorType::Int,     1 };
        case GL_INT_VEC2:           return { VectorType::Int2,    1 };
        case GL_INT_VEC3:           return { VectorType::Int3,    1 };
        case GL_INT_VEC4:           return { VectorType::Int4,    1 };
        case GL_UNSIGNED_INT:       return { VectorType::UInt,    1 };
        case GL_UNSIGNED_INT_VEC2:  return { VectorType::UInt2,   1 };
        case GL_UNSIGNED_INT_VEC3:  return { VectorType::UInt3,   1 };
        case GL_UNSIGNED_INT_VEC4:  return { VectorType::UInt4,   1 };
        case GL_DOUBLE:             return { VectorType::Double,  1 };
        case GL_DOUBLE_VEC2:        return { VectorType::Double2, 1 };
        case GL_DOUBLE_VEC3:        return { VectorType::Double3, 1 };
        case GL_DOUBLE_VEC4:        return { VectorType::Double4, 4 };
        case GL_DOUBLE_MAT2:        return { VectorType::Double2, 2 };
        case GL_DOUBLE_MAT3:        return { VectorType::Double3, 3 };
        case GL_DOUBLE_MAT4:        return { VectorType::Double4, 4 };
        case GL_DOUBLE_MAT2x3:      return { VectorType::Double3, 2 };
        case GL_DOUBLE_MAT2x4:      return { VectorType::Double4, 2 };
        case GL_DOUBLE_MAT3x2:      return { VectorType::Double2, 3 };
        case GL_DOUBLE_MAT3x4:      return { VectorType::Double4, 3 };
        case GL_DOUBLE_MAT4x2:      return { VectorType::Double2, 4 };
        case GL_DOUBLE_MAT4x3:      return { VectorType::Double3, 4 };
    }
    return { VectorType::Float, 0 };
}

std::vector<VertexAttribute> GLShaderProgram::QueryVertexAttributes() const
{
    VertexFormat vertexFormat;

    /* Query number of vertex attributes */
    GLint numAttribs = 0;
    glGetProgramiv(id_, GL_ACTIVE_ATTRIBUTES, &numAttribs);
    if (numAttribs <= 0)
        return vertexFormat.attributes;

    /* Query maximal name length of all vertex attributes */
    GLint maxNameLength = 0;
    glGetProgramiv(id_, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxNameLength);
    if (maxNameLength <= 0)
        return vertexFormat.attributes;

    std::vector<char> attribName(maxNameLength, 0);

    /* Iterate over all vertex attributes */
    for (GLuint i = 0; i < static_cast<GLuint>(numAttribs); ++i)
    {
        /* Query attribute information */
        GLint   size        = 0;
        GLenum  type        = 0;
        GLsizei nameLength  = 0;

        glGetActiveAttrib(id_, i, maxNameLength, &nameLength, &size, &type, attribName.data());

        /* Convert attribute information */
        auto name = std::string(attribName.data());
        auto attr = UnmapAttribType(type);

        /* Insert vertex attribute into list */
        while (attr.second-- > 0)
            vertexFormat.AppendAttribute({ name, attr.first });
    }

    return vertexFormat.attributes;
}

std::vector<StreamOutputAttribute> GLShaderProgram::QueryStreamOutputAttributes() const
{
    StreamOutputFormat streamOutputFormat;
    StreamOutputAttribute soAttrib;

    if (HasExtension(GLExt::EXT_transform_feedback))
    {
        /* Query number of vertex attributes */
        GLint numVaryings = 0;
        glGetProgramiv(id_, GL_TRANSFORM_FEEDBACK_VARYINGS, &numVaryings);
        if (numVaryings <= 0)
            return streamOutputFormat.attributes;

        /* Query maximal name length of all vertex attributes */
        GLint maxNameLength = 0;
        glGetProgramiv(id_, GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH, &maxNameLength);
        if (maxNameLength <= 0)
            return streamOutputFormat.attributes;

        std::vector<char> attribName(maxNameLength, 0);

        /* Iterate over all vertex attributes */
        for (GLuint i = 0; i < static_cast<GLuint>(numVaryings); ++i)
        {
            /* Query attribute information */
            GLint   size        = 0;
            GLenum  type        = 0;
            GLsizei nameLength  = 0;

            glGetTransformFeedbackVarying(id_, i, maxNameLength, &nameLength, &size, &type, attribName.data());

            /* Convert attribute information */
            soAttrib.name = std::string(attribName.data());
            //auto attr = UnmapAttribType(type);

            /* Insert stream-output attribute into list */
            #if 0
            while (attr.second-- > 0)
                streamOutputFormat.AppendAttribute(soAttrib);
            #else
            streamOutputFormat.AppendAttribute(soAttrib);
            #endif
        }
    }
    else if (HasExtension(GLExt::NV_transform_feedback))
    {
        /* Query number of vertex attributes */
        GLint numVaryings = 0;
        glGetProgramiv(id_, GL_ACTIVE_VARYINGS_NV, &numVaryings);
        if (numVaryings <= 0)
            return streamOutputFormat.attributes;

        /* Query maximal name length of all vertex attributes */
        GLint maxNameLength = 0;
        glGetProgramiv(id_, GL_ACTIVE_VARYING_MAX_LENGTH_NV, &maxNameLength);
        if (maxNameLength <= 0)
            return streamOutputFormat.attributes;

        std::vector<char> attribName(maxNameLength, 0);

        /* Iterate over all vertex attributes */
        for (GLuint i = 0; i < static_cast<GLuint>(numVaryings); ++i)
        {
            /* Query attribute information */
            GLint   size        = 0;
            GLenum  type        = 0;
            GLsizei nameLength  = 0;

            glGetActiveVaryingNV(id_, i, maxNameLength, &nameLength, &size, &type, attribName.data());

            /* Convert attribute information */
            soAttrib.name = std::string(attribName.data());
            //auto attr = UnmapAttribType(type);

            /* Insert stream-output attribute into list */
            #if 0
            while (attr.second-- > 0)
                streamOutputFormat.AppendAttribute(soAttrib);
            #else
            streamOutputFormat.AppendAttribute(soAttrib);
            #endif
        }
    }
    else
        ThrowNotSupported("stream-outputs");

    return streamOutputFormat.attributes;
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

void GLShaderProgram::BuildInputLayout(const VertexFormat& vertexFormat)
{
    if (vertexFormat.attributes.size() > GL_MAX_VERTEX_ATTRIBS)
    {
        throw std::invalid_argument(
            "failed to bind vertex attributes, because too many attributes are specified (maximum is " +
            std::to_string(GL_MAX_VERTEX_ATTRIBS) + ")"
        );
    }

    /* Bind all vertex attribute locations */
    GLuint index = 0;

    for (const auto& attrib : vertexFormat.attributes)
    {
        /* Bind attribute location (matrices only use the column) */
        if (attrib.semanticIndex == 0)
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


/*
 * ======= Private: =======
 */

bool GLShaderProgram::LinkShaderProgram()
{
    /* Link shader program */
    glLinkProgram(id_);

    /* Query linking status */
    GLint linkStatus = 0;
    glGetProgramiv(id_, GL_LINK_STATUS, &linkStatus);

    return (linkStatus != GL_FALSE);
}

void GLShaderProgram::SpecifyTransformFeedbackVaryingsEXT(const std::vector<StreamOutputAttribute>& attributes)
{
    /* Specify transform-feedback varyings by names */
    std::vector<const GLchar*> varyings;
    varyings.reserve(attributes.size());

    for (const auto& attr : attributes)
        varyings.push_back(attr.name.c_str());

    glTransformFeedbackVaryings(id_, static_cast<GLsizei>(varyings.size()), varyings.data(), GL_INTERLEAVED_ATTRIBS);
}

void GLShaderProgram::SpecifyTransformFeedbackVaryingsNV(const std::vector<StreamOutputAttribute>& attributes)
{
    /* Specify transform-feedback varyings by locations */
    std::vector<GLint> varyings;
    varyings.reserve(attributes.size());

    for (const auto& attr : attributes)
    {
        /* Get varying location by its name */
        auto location = glGetVaryingLocationNV(id_, attr.name.c_str());
        if (location >= 0)
            varyings.push_back(location);
        else
            throw std::invalid_argument("stream-output attribute \"" + attr.name + "\" does not specify an active varying in GLSL shader program");
    }

    glTransformFeedbackVaryingsNV(id_, static_cast<GLsizei>(varyings.size()), varyings.data(), GL_INTERLEAVED_ATTRIBS_NV);
}


} // /namespace LLGL



// ================================================================================
