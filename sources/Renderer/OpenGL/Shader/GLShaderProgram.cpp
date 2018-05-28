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

ShaderReflectionDescriptor GLShaderProgram::QueryReflectionDesc() const
{
    ShaderReflectionDescriptor reflection;

    /* Reflect shader program */
    Reflect(reflection);

    /* Sort output to meet the interface requirements */
    ShaderProgram::FinalizeShaderReflection(reflection);

    return reflection;
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
            /* Bind attribute location (matrices only use the 1st column) */
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

void GLShaderProgram::Reflect(ShaderReflectionDescriptor& reflection) const
{
    QueryVertexAttributes(reflection);
    QueryStreamOutputAttributes(reflection);
    QueryConstantBuffers(reflection);
    QueryStorageBuffers(reflection);
    QueryUniforms(reflection);
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

void GLShaderProgram::QueryVertexAttributes(ShaderReflectionDescriptor& reflection) const
{
    VertexFormat vertexFormat;

    /* Query active vertex attributes */
    std::vector<char> attribName;
    GLint numAttribs = 0, maxNameLength = 0;
    if (!QueryActiveAttribs(GL_ACTIVE_ATTRIBUTES, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, numAttribs, maxNameLength, attribName))
        return;

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

    reflection.vertexAttributes = std::move(vertexFormat.attributes);
}

void GLShaderProgram::QueryStreamOutputAttributes(ShaderReflectionDescriptor& reflection) const
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
            return;

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
            return;

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
        ThrowNotSupportedExcept(__FUNCTION__, "stream-outputs");
    #endif

    reflection.streamOutputAttributes = std::move(streamOutputFormat.attributes);
}

static bool QueryProgramResourceProperties(
    GLuint program, GLenum programInterface, GLuint resourceIndex,
    GLsizei count, const GLenum* props, GLint* params)
{
    #ifdef GL_ARB_program_interface_query
    if (HasExtension(GLExt::ARB_program_interface_query))
    {
        glGetProgramResourceiv(program, programInterface, resourceIndex, count, props, count, nullptr, params);
        return true;
    }
    #endif
    return false;
}

// Returns the shader stage flags for the specified program properties and their parameters 'GL_REFERENCED_BY_...'
static long GetStageFlagsFromResourceProperties(GLsizei count, const GLenum* props, const GLint* params)
{
    long stageFlags = 0;

    auto AppendStageFlagsBit = [&](GLsizei idx, long bitmask)
    {
        if (params[idx] != 0)
            stageFlags |= bitmask;
    };

    for (GLsizei i = 0; i < count; ++i)
    {
        switch (props[i])
        {
            case GL_REFERENCED_BY_VERTEX_SHADER:
                AppendStageFlagsBit(i, StageFlags::VertexStage);
                break;
            case GL_REFERENCED_BY_TESS_CONTROL_SHADER:
                AppendStageFlagsBit(i, StageFlags::TessControlStage);
                break;
            case GL_REFERENCED_BY_TESS_EVALUATION_SHADER:
                AppendStageFlagsBit(i, StageFlags::TessEvaluationStage);
                break;
            case GL_REFERENCED_BY_GEOMETRY_SHADER:
                AppendStageFlagsBit(i, StageFlags::GeometryStage);
                break;
            case GL_REFERENCED_BY_FRAGMENT_SHADER:
                AppendStageFlagsBit(i, StageFlags::FragmentStage);
                break;
            case GL_REFERENCED_BY_COMPUTE_SHADER:
                AppendStageFlagsBit(i, StageFlags::ComputeStage);
                break;
        }
    }

    return stageFlags;
}

void GLShaderProgram::QueryConstantBuffers(ShaderReflectionDescriptor& reflection) const
{
    /* Query active uniform blocks */
    std::vector<char> blockName;
    GLint numUniformBlocks = 0, maxNameLength = 0;
    if (!QueryActiveAttribs(GL_ACTIVE_UNIFORM_BLOCKS, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, numUniformBlocks, maxNameLength, blockName))
        return;

    /* Iterate over all uniform blocks */
    for (GLuint i = 0; i < static_cast<GLuint>(numUniformBlocks); ++i)
    {
        ShaderReflectionDescriptor::ResourceView resourceView;
        {
            /* Initialize resource view descriptor */
            resourceView.type           = ResourceType::ConstantBuffer;
            resourceView.stageFlags     = StageFlags::AllStages;

            /* Query uniform block name */
            GLsizei nameLength = 0;
            glGetActiveUniformBlockName(id_, i, maxNameLength, &nameLength, blockName.data());
            resourceView.name = std::string(blockName.data());

            /* Query uniform block size */
            GLint blockSize = 0;
            glGetActiveUniformBlockiv(id_, i, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
            resourceView.constantBufferSize = static_cast<std::uint32_t>(blockSize);

            /* Query resource view properties */
            QueryBufferProperties(resourceView, GL_UNIFORM_BLOCK, i);
        }
        reflection.resourceViews.push_back(resourceView);
    }
}

void GLShaderProgram::QueryStorageBuffers(ShaderReflectionDescriptor& reflection) const
{
    #ifndef __APPLE__

    /* Query number of shader storage blocks */
    GLenum properties[3] = { 0 };
    properties[0] = GL_NUM_ACTIVE_VARIABLES;
    GLint numStorageBlocks = 0;
    glGetProgramResourceiv(id_, GL_SHADER_STORAGE_BLOCK, 0, 1, properties, 1, nullptr, &numStorageBlocks);
    if (numStorageBlocks <= 0)
        return;

    /* Query maximal name length of all shader storage blocks */
    GLint maxNameLength = 0;
    glGetProgramInterfaceiv(id_, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &maxNameLength);
    if (maxNameLength <= 0)
        return;

    std::vector<char> blockName(maxNameLength, 0);

    /* Iterate over all shader storage blocks */
    for (GLuint i = 0; i < static_cast<GLuint>(numStorageBlocks); ++i)
    {
        ShaderReflectionDescriptor::ResourceView resourceView;
        {
            /* Initialize resource view descriptor */
            resourceView.type       = ResourceType::StorageBuffer;

            /* Query shader storage block name */
            GLsizei nameLength = 0;
            glGetProgramResourceName(id_, GL_SHADER_STORAGE_BLOCK, i, maxNameLength, &nameLength, blockName.data());
            resourceView.name = std::string(blockName.data());

            /* Query resource view properties */
            QueryBufferProperties(resourceView, GL_SHADER_STORAGE_BLOCK, i);
        }
        reflection.resourceViews.push_back(resourceView);
    }

    #endif
}

void GLShaderProgram::QueryUniforms(ShaderReflectionDescriptor& reflection) const
{
    std::vector<UniformDescriptor> descList;

    /* Query active uniforms */
    std::vector<char> uniformName;
    GLint numUniforms = 0, maxNameLength = 0;
    if (!QueryActiveAttribs(GL_ACTIVE_UNIFORMS, GL_ACTIVE_UNIFORM_MAX_LENGTH, numUniforms, maxNameLength, uniformName))
        return;

    /* Iterate over all uniforms */
    for (GLuint i = 0; i < static_cast<GLuint>(numUniforms); ++i)
    {
        /* Query uniform block name */
        GLsizei nameLength  = 0;
        GLint   size        = 0;
        GLenum  type        = 0;

        glGetActiveUniform(id_, i, maxNameLength, &nameLength, &size, &type, uniformName.data());

        /* Integrate uniform into reflection containers */
        UniformType uniformType = UniformType::Undefined;
        GLTypes::Unmap(uniformType, type);

        if (uniformType == UniformType::Sampler)
        {
            /* Append GLSL compiled texture-sampler as both texture and sampler resource */
            ShaderReflectionDescriptor::ResourceView resourceView;
            {
                resourceView.name = std::string(uniformName.data());
                resourceView.type = ResourceType::Texture;

                /* Query resource properties */
                const GLenum props[] =
                {
                    GL_REFERENCED_BY_VERTEX_SHADER,
                    GL_REFERENCED_BY_TESS_CONTROL_SHADER,
                    GL_REFERENCED_BY_TESS_EVALUATION_SHADER,
                    GL_REFERENCED_BY_GEOMETRY_SHADER,
                    GL_REFERENCED_BY_FRAGMENT_SHADER,
                    GL_REFERENCED_BY_COMPUTE_SHADER,
                    GL_ARRAY_SIZE
                };
                GLint params[7] = {};

                if (QueryProgramResourceProperties(id_, GL_UNIFORM, i, 7, props, params))
                {
                    //resourceView.slot       = static_cast<std::uint32_t>(params[0]);
                    resourceView.stageFlags = GetStageFlagsFromResourceProperties(6, props, params);
                    resourceView.slot       = 0xffffffff;
                    resourceView.arraySize  = static_cast<std::uint32_t>(params[6]);
                }
                else
                {
                    /* Set binding slot to invalid index */
                    resourceView.stageFlags = StageFlags::AllStages;
                    resourceView.slot       = 0xffffffff;
                    resourceView.arraySize  = 1;
                }
            }
            reflection.resourceViews.push_back(resourceView);
            {
                resourceView.type = ResourceType::Sampler;
            }
            reflection.resourceViews.push_back(resourceView);
        }
        else
        {
            /* Append default uniform */
            UniformDescriptor uniform;
            {
                uniform.name        = std::string(uniformName.data());
                uniform.location    = glGetUniformLocation(id_, uniformName.data());
                uniform.size        = static_cast<std::uint32_t>(size);
            }
            reflection.uniforms.push_back(uniform);
        }
    }
}

void GLShaderProgram::QueryBufferProperties(ShaderReflectionDescriptor::ResourceView& resourceView, GLenum programInterface, GLuint resourceIndex) const
{
    const GLenum props[] =
    {
        GL_REFERENCED_BY_VERTEX_SHADER,
        GL_REFERENCED_BY_TESS_CONTROL_SHADER,
        GL_REFERENCED_BY_TESS_EVALUATION_SHADER,
        GL_REFERENCED_BY_GEOMETRY_SHADER,
        GL_REFERENCED_BY_FRAGMENT_SHADER,
        GL_REFERENCED_BY_COMPUTE_SHADER,
        GL_BUFFER_BINDING
    };
    GLint params[7] = {};

    if (QueryProgramResourceProperties(id_, programInterface, resourceIndex, 7, props, params))
    {
        resourceView.stageFlags = GetStageFlagsFromResourceProperties(6, props, params);
        resourceView.slot       = static_cast<std::uint32_t>(params[6]);
    }
    else
    {
        /* Set binding slot to invalid index */
        resourceView.stageFlags = StageFlags::AllStages;
        resourceView.slot       = 0xffffffff;
    }
}


} // /namespace LLGL



// ================================================================================
