/*
 * GLShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLShaderProgram.h"
#include "GLShader.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionLoader.h"
#include "../../CheckedCast.h"
#include "../../../Core/Exception.h"
#include "../RenderState/GLStateManager.h"
#include "../../GLCommon/GLTypes.h"
#include <LLGL/Log.h>
#include <LLGL/VertexFormat.h>
#include <vector>
#include <stdexcept>


namespace LLGL
{


GLShaderProgram::GLShaderProgram() :
    id_      { glCreateProgram() },
    uniform_ { id_               }
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

void GLShaderProgram::DetachAll()
{
    /* Retrieve all attached shaders from program */
    GLsizei numShaders = 0;
    GLuint shaders[6] = { 0 };
    glGetAttachedShaders(id_, 6, &numShaders, shaders);

    /* Detach all shaders */
    for (GLsizei i = 0; i < numShaders; ++i)
        glDetachShader(id_, shaders[i]);

    /* Reset shader attributes */
    hasFragmentShader_ = false;
    streamOutputFormat_.attributes.clear();
}

bool GLShaderProgram::LinkShaders()
{
    /* Check if transform-feedback varyings must be specified (before or after shader linking) */
    if (!streamOutputFormat_.attributes.empty())
    {
        /* For GL_EXT_transform_feedback the varyings must be specified BEFORE linking */
        #ifndef __APPLE__
        if (HasExtension(GLExt::EXT_transform_feedback))
        #endif
        {
            BuildTransformFeedbackVaryingsEXT(streamOutputFormat_.attributes);
            return LinkShaderProgram();
        }

        #ifndef __APPLE__
        /* For GL_NV_transform_feedback (Vendor specific) the varyings must be specified AFTER linking */
        if (HasExtension(GLExt::NV_transform_feedback))
        {
            auto result = LinkShaderProgram();
            BuildTransformFeedbackVaryingsNV(streamOutputFormat_.attributes);
            return result;
        }
        #endif
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

static std::pair<VectorType, std::uint32_t> UnmapAttribType(GLenum type)
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

    /* Query active vertex attributes */
    std::vector<char> attribName;
    GLint numAttribs = 0, maxNameLength = 0;
    if (!QueryActiveAttribs(GL_ACTIVE_ATTRIBUTES, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, numAttribs, maxNameLength, attribName))
        return {};

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

    #ifndef __APPLE__
    if (HasExtension(GLExt::EXT_transform_feedback))
    #endif
    {
        /* Query active varyings */
        std::vector<char> attribName;
        GLint numVaryings = 0, maxNameLength = 0;
        if (!QueryActiveAttribs(GL_TRANSFORM_FEEDBACK_VARYINGS, GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH, numVaryings, maxNameLength, attribName))
            return {};

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
    #ifndef __APPLE__
    else if (HasExtension(GLExt::NV_transform_feedback))
    {
        /* Query active varyings */
        std::vector<char> attribName;
        GLint numVaryings = 0, maxNameLength = 0;
        if (!QueryActiveAttribs(GL_ACTIVE_VARYINGS_NV, GL_ACTIVE_VARYING_MAX_LENGTH_NV, numVaryings, maxNameLength, attribName))
            return {};

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
    #endif

    return streamOutputFormat.attributes;
}

std::vector<ConstantBufferViewDescriptor> GLShaderProgram::QueryConstantBuffers() const
{
    std::vector<ConstantBufferViewDescriptor> descList;

    /* Query active uniform blocks */
    std::vector<char> blockName;
    GLint numUniformBlocks = 0, maxNameLength = 0;
    if (!QueryActiveAttribs(GL_ACTIVE_UNIFORM_BLOCKS, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, numUniformBlocks, maxNameLength, blockName))
        return {};

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

    /* Query active uniforms */
    std::vector<char> uniformName;
    GLint numUniforms = 0, maxNameLength = 0;
    if (!QueryActiveAttribs(GL_ACTIVE_UNIFORMS, GL_ACTIVE_UNIFORM_MAX_LENGTH, numUniforms, maxNameLength, uniformName))
        return {};

    /* Iterate over all uniforms */
    for (GLuint i = 0; i < static_cast<GLuint>(numUniforms); ++i)
    {
        try
        {
            UniformDescriptor desc;

            /* Query uniform block name */
            GLsizei nameLength  = 0;
            GLint   size        = 0;
            GLenum  type        = 0;

            glGetActiveUniform(id_, i, maxNameLength, &nameLength, &size, &type, uniformName.data());

            desc.name       = std::string(uniformName.data());
            desc.location   = glGetUniformLocation(id_, uniformName.data());
            desc.size       = static_cast<std::uint32_t>(size);

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

void GLShaderProgram::BuildInputLayout(std::uint32_t numVertexFormats, const VertexFormat* vertexFormats)
{
    if (numVertexFormats == 0 || vertexFormats == nullptr)
        return;

    /* Validate maximal number of vertex attributes (OpenGL supports at least 8 vertex attribute) */
    static const std::size_t minSupportedVertexAttribs = 8;

    std::size_t numVertexAttribs = 0;
    for (std::uint32_t i = 0; i < numVertexFormats; ++i)
        numVertexAttribs += vertexFormats[i].attributes.size();

    if (numVertexAttribs > minSupportedVertexAttribs)
    {
        GLint maxSupportedVertexAttribs = 0;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxSupportedVertexAttribs);

        if (numVertexAttribs > static_cast<std::size_t>(maxSupportedVertexAttribs))
        {
            throw std::invalid_argument(
                "failed build input layout, because too many vertex attributes are specified (maximum is " +
                std::to_string(maxSupportedVertexAttribs) + ")"
            );
        }
    }

    /* Bind all vertex attribute locations */
    GLuint index = 0;

    for (std::uint32_t i = 0; i < numVertexFormats; ++i)
    {
        for (const auto& attrib : vertexFormats[i].attributes)
        {
            /* Bind attribute location (matrices only use the column) */
            if (attrib.semanticIndex == 0)
                glBindAttribLocation(id_, index, attrib.name.c_str());
            ++index;
        }
    }

    /* Re-link shader program if the shader has already been linked */
    if (isLinked_)
        LinkShaderProgram();
}

void GLShaderProgram::BindConstantBuffer(const std::string& name, std::uint32_t bindingIndex)
{
    /* Query uniform block index and bind it to the specified binding index */
    auto blockIndex = glGetUniformBlockIndex(id_, name.c_str());
    if (blockIndex != GL_INVALID_INDEX)
        glUniformBlockBinding(id_, blockIndex, bindingIndex);
    else
        throw std::invalid_argument("failed to bind constant buffer, because uniform block name is invalid");
}

void GLShaderProgram::BindStorageBuffer(const std::string& name, std::uint32_t bindingIndex)
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

bool GLShaderProgram::QueryActiveAttribs(
    GLenum attribCountType, GLenum attribNameLengthType,
    GLint& numAttribs, GLint& maxNameLength, std::vector<char>& nameBuffer) const
{
    /* Query number of active attributes */
    glGetProgramiv(id_, attribCountType, &numAttribs);
    if (numAttribs <= 0)
        return false;

    /* Query maximal name length of all attributes */
    glGetProgramiv(id_, attribNameLengthType, &maxNameLength);
    if (maxNameLength <= 0)
        return false;

    nameBuffer.resize(maxNameLength, '\0');

    return true;
}

bool GLShaderProgram::LinkShaderProgram()
{
    /* Link shader program */
    glLinkProgram(id_);

    /* Query linking status */
    GLint linkStatus = 0;
    glGetProgramiv(id_, GL_LINK_STATUS, &linkStatus);

    /* Store if program is linked successful */
    isLinked_ = (linkStatus != GL_FALSE);

    return isLinked_;
}

void GLShaderProgram::BuildTransformFeedbackVaryingsEXT(const std::vector<StreamOutputAttribute>& attributes)
{
    /* Specify transform-feedback varyings by names */
    std::vector<const GLchar*> varyings;
    varyings.reserve(attributes.size());

    for (const auto& attr : attributes)
        varyings.push_back(attr.name.c_str());

    glTransformFeedbackVaryings(id_, static_cast<GLsizei>(varyings.size()), varyings.data(), GL_INTERLEAVED_ATTRIBS);
}

#ifndef __APPLE__

void GLShaderProgram::BuildTransformFeedbackVaryingsNV(const std::vector<StreamOutputAttribute>& attributes)
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

#endif


} // /namespace LLGL



// ================================================================================
