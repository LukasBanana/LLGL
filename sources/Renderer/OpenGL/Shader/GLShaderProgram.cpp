/*
 * GLShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLShaderProgram.h"
#include "GLShader.h"
#include "GLShaderBindingLayout.h"
#include "../RenderState/GLStateManager.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionLoader.h"
#include "../../CheckedCast.h"
#include "../../GLCommon/GLTypes.h"
#include "../../../Core/Exception.h"
#include <LLGL/VertexFormat.h>
#include <LLGL/Constants.h>
#include <vector>
#include <stdexcept>


namespace LLGL
{


GLShaderProgram::GLShaderProgram(const ShaderProgramDescriptor& desc) :
    id_ { glCreateProgram() }
{
    Attach(desc.vertexShader);
    Attach(desc.tessControlShader);
    Attach(desc.tessEvaluationShader);
    Attach(desc.geometryShader);
    Attach(desc.fragmentShader);
    Attach(desc.computeShader);
    BuildInputLayout(desc.vertexFormats.size(), desc.vertexFormats.data());
    Link();
}

GLShaderProgram::~GLShaderProgram()
{
    glDeleteProgram(id_);
    GLStateManager::active->NotifyShaderProgramRelease(id_);
}

bool GLShaderProgram::HasErrors() const
{
    GLint status = 0;
    glGetProgramiv(id_, GL_LINK_STATUS, &status);
    return (status == GL_FALSE);
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

UniformLocation GLShaderProgram::QueryUniformLocation(const char* name) const
{
    if (id_ != 0)
        return static_cast<UniformLocation>(glGetUniformLocation(id_, name));
    else
        return -1;
}

bool GLShaderProgram::SetWorkGroupSize(const Extent3D& workGroupSize)
{
    return false; // dummy
}

bool GLShaderProgram::GetWorkGroupSize(Extent3D& workGroupSize) const
{
    #ifdef GL_ARB_compute_shader
    if (HasExtension(GLExt::ARB_compute_shader))
    {
        GLint params[3] = { 0 };
        glGetProgramiv(id_, GL_COMPUTE_WORK_GROUP_SIZE, params);
        if (params[0] > 0 && params[1] > 0 && params[2] > 0)
        {
            workGroupSize.width  = static_cast<std::uint32_t>(params[0]);
            workGroupSize.height = static_cast<std::uint32_t>(params[1]);
            workGroupSize.depth  = static_cast<std::uint32_t>(params[2]);
            return true;
        }
    }
    #endif // /GL_ARB_compute_shader
    return false;
}


/*
 * ======= Internal: =======
 */

void GLShaderProgram::BindResourceSlots(const GLShaderBindingLayout& bindingLayout) const
{
    /* Keep track of state change with mutable reference to binding layout */
    if (bindingLayout_ != &bindingLayout)
    {
        bindingLayout.BindResourceSlots(GetID());
        bindingLayout_ = &bindingLayout;
    }
}


/*
 * ======= Private: =======
 */

//TODO: refactor "MoveStreamOutputFormat" (shader might be used multiple times, but internal container gets lost!)
void GLShaderProgram::Attach(Shader* shader)
{
    if (shader != nullptr)
    {
        auto shaderGL = LLGL_CAST(GLShader*, shader);

        /* Attach shader to shader program */
        glAttachShader(id_, shaderGL->GetID());

        /* Move stream-output format from shader to shader program (if available) */
        shaderGL->MoveStreamOutputFormat(streamOutputFormat_);
    }
}

void GLShaderProgram::BuildInputLayout(std::size_t numVertexFormats, const VertexFormat* vertexFormats)
{
    if (numVertexFormats == 0 || vertexFormats == nullptr)
        return;

    /* Validate maximal number of vertex attributes (OpenGL supports at least 8 vertex attribute) */
    static const std::size_t minSupportedVertexAttribs = 8;

    std::size_t numVertexAttribs = 0;
    for (std::size_t i = 0; i < numVertexFormats; ++i)
        numVertexAttribs += vertexFormats[i].attributes.size();

    if (numVertexAttribs > minSupportedVertexAttribs)
    {
        GLint maxSupportedVertexAttribs = 0;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxSupportedVertexAttribs);

        if (numVertexAttribs > static_cast<std::size_t>(maxSupportedVertexAttribs))
        {
            throw std::invalid_argument(
                "failed build input layout, because too many vertex attributes are specified (" +
                std::to_string(numVertexAttribs) + " is specified, but maximum is " + std::to_string(maxSupportedVertexAttribs) + ")"
            );
        }
    }

    /* Bind all vertex attribute locations */
    GLuint index = 0;

    for (std::size_t i = 0; i < numVertexFormats; ++i)
    {
        for (const auto& attrib : vertexFormats[i].attributes)
        {
            /* Bind attribute location (matrices only use the 1st column) */
            if (attrib.semanticIndex == 0)
                glBindAttribLocation(id_, index, attrib.name.c_str());
            ++index;
        }
    }
}

void GLShaderProgram::Link()
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
            glLinkProgram(id_);
            return;
        }

        #ifndef __APPLE__
        /* For GL_NV_transform_feedback (Vendor specific) the varyings must be specified AFTER linking */
        if (HasExtension(GLExt::NV_transform_feedback))
        {
            glLinkProgram(id_);
            BuildTransformFeedbackVaryingsNV(streamOutputFormat_.attributes);
            return;
        }
        #endif
    }

    /* Just link shader program */
    glLinkProgram(id_);
}

bool GLShaderProgram::QueryActiveAttribs(
    GLenum              attribCountType,
    GLenum              attribNameLengthType,
    GLint&              numAttribs,
    GLint&              maxNameLength,
    std::vector<char>&  nameBuffer) const
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

// Vector format and number of vectors, e.g. mat2x3 --> { RGB32Float, 2 }
static std::pair<Format, std::uint32_t> UnmapAttribType(GLenum type)
{
    switch (type)
    {
        case GL_FLOAT:              return { Format::R32Float,      1 };
        case GL_FLOAT_VEC2:         return { Format::RG32Float,     1 };
        case GL_FLOAT_VEC3:         return { Format::RGB32Float,    1 };
        case GL_FLOAT_VEC4:         return { Format::RGBA32Float,   1 };
        case GL_FLOAT_MAT2:         return { Format::RG32Float,     2 };
        case GL_FLOAT_MAT3:         return { Format::RGB32Float,    3 };
        case GL_FLOAT_MAT4:         return { Format::RGBA32Float,   4 };
        case GL_FLOAT_MAT2x3:       return { Format::RGB32Float,    2 };
        case GL_FLOAT_MAT2x4:       return { Format::RGBA32Float,   2 };
        case GL_FLOAT_MAT3x2:       return { Format::RG32Float,     3 };
        case GL_FLOAT_MAT3x4:       return { Format::RGBA32Float,   3 };
        case GL_FLOAT_MAT4x2:       return { Format::RG32Float,     4 };
        case GL_FLOAT_MAT4x3:       return { Format::RGB32Float,    4 };
        case GL_INT:                return { Format::R32SInt,       1 };
        case GL_INT_VEC2:           return { Format::RG32SInt,      1 };
        case GL_INT_VEC3:           return { Format::RGB32SInt,     1 };
        case GL_INT_VEC4:           return { Format::RGBA32SInt,    1 };
        case GL_UNSIGNED_INT:       return { Format::R32UInt,       1 };
        case GL_UNSIGNED_INT_VEC2:  return { Format::RG32UInt,      1 };
        case GL_UNSIGNED_INT_VEC3:  return { Format::RGB32UInt,     1 };
        case GL_UNSIGNED_INT_VEC4:  return { Format::RGBA32UInt,    1 };
        case GL_DOUBLE:             return { Format::R64Float,      1 };
        case GL_DOUBLE_VEC2:        return { Format::RG64Float,     1 };
        case GL_DOUBLE_VEC3:        return { Format::RGB64Float,    1 };
        case GL_DOUBLE_VEC4:        return { Format::RGBA64Float,   1 };
        case GL_DOUBLE_MAT2:        return { Format::RG64Float,     2 };
        case GL_DOUBLE_MAT3:        return { Format::RGB64Float,    3 };
        case GL_DOUBLE_MAT4:        return { Format::RGBA64Float,   4 };
        case GL_DOUBLE_MAT2x3:      return { Format::RGB64Float,    2 };
        case GL_DOUBLE_MAT2x4:      return { Format::RGBA64Float,   2 };
        case GL_DOUBLE_MAT3x2:      return { Format::RG64Float,     3 };
        case GL_DOUBLE_MAT3x4:      return { Format::RGBA64Float,   3 };
        case GL_DOUBLE_MAT4x2:      return { Format::RG64Float,     4 };
        case GL_DOUBLE_MAT4x3:      return { Format::RGB64Float,    4 };
    }
    return { Format::R32Float, 0 };
}

static SystemValue FindSystemValue(const std::string& name)
{
    const std::pair<const char*, SystemValue> glslSystemValues[] =
    {
        { "gl_ClipDistance",    SystemValue::ClipDistance       },
        { "gl_CullDistance",    SystemValue::CullDistance       },
        { "gl_FrontFacing",     SystemValue::FrontFacing        },
        { "gl_InstanceID",      SystemValue::InstanceID         }, // GLSL
        { "gl_InstanceIndex",   SystemValue::InstanceID         }, // SPIR-V
        { "gl_Position",        SystemValue::Position           },
        { "gl_FragCoord",       SystemValue::Position           },
        { "gl_PrimitiveID",     SystemValue::PrimitiveID        },
        { "gl_Layer",           SystemValue::RenderTargetIndex  },
        { "gl_SampleID",        SystemValue::SampleID           },
        { "gl_VertexID",        SystemValue::VertexID           }, // GLSL
        { "gl_VertexIndex",     SystemValue::VertexID           }, // SPIR-V
        { "gl_ViewportIndex",   SystemValue::ViewportIndex      },
    };

    for (const auto& sysVal : glslSystemValues)
    {
        if (name == sysVal.first)
            return sysVal.second;
    }

    return SystemValue::Undefined;
}

// Internal struct for QueryVertexAttributes function
struct GLVertexAttribute
{
    std::string     name;
    Format          format;
    std::uint32_t   semanticIndex;
    std::uint32_t   location;
};

void GLShaderProgram::QueryVertexAttributes(ShaderReflectionDescriptor& reflection) const
{
    /* Query active vertex attributes */
    std::vector<char> attribName;
    GLint numAttribs = 0, maxNameLength = 0;
    if (!QueryActiveAttribs(GL_ACTIVE_ATTRIBUTES, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, numAttribs, maxNameLength, attribName))
        return;

    std::vector<GLVertexAttribute> attributes;
    attributes.reserve(static_cast<std::size_t>(numAttribs));

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

        /* Get attribute location */
        auto location = static_cast<std::uint32_t>(glGetAttribLocation(id_, name.c_str()));

        /* Insert vertex attribute into list */
        for (std::uint32_t semanticIndex = 0; semanticIndex < attr.second; ++semanticIndex)
            attributes.push_back({ name, attr.first, semanticIndex, location });
    }

    /* Sort attributes by location */
    std::sort(
        attributes.begin(),
        attributes.end(),
        [](const GLVertexAttribute& lhs, const GLVertexAttribute& rhs)
        {
            if (lhs.location < rhs.location)
                return true;
            if (lhs.location > rhs.location)
                return false;
            return (lhs.name < rhs.name);
        }
    );

    /* Copy attribute into final list and determine offsets */
    reflection.vertexAttributes.resize(attributes.size());

    for (std::size_t i = 0; i < attributes.size(); ++i)
    {
        auto& src = attributes[i];
        auto& dst = reflection.vertexAttributes[i];

        dst.name    = std::move(src.name);
        dst.format  = src.format;

        if (src.location == std::uint32_t(-1))
            dst.systemValue = FindSystemValue(dst.name);
        else
            dst.semanticIndex = src.semanticIndex;
    }
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

#ifdef GL_ARB_program_interface_query

static bool GLGetProgramResourceProperties(
    GLuint          program,
    GLenum          programInterface,
    GLuint          resourceIndex,
    GLsizei         count,
    const GLenum*   props,
    GLint*          params)
{
    if (HasExtension(GLExt::ARB_program_interface_query))
    {
        glGetProgramResourceiv(program, programInterface, resourceIndex, count, props, count, nullptr, params);
        return true;
    }
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

#endif // /GL_ARB_program_interface_query

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
            resourceView.type           = ResourceType::Buffer;
            resourceView.stageFlags     = StageFlags::AllStages;
            resourceView.bindFlags      = BindFlags::ConstantBuffer;

            /* Query uniform block name */
            GLsizei nameLength = 0;
            glGetActiveUniformBlockName(id_, i, maxNameLength, &nameLength, blockName.data());
            resourceView.name = std::string(blockName.data());

            /* Query uniform block size */
            GLint blockSize = 0;
            glGetActiveUniformBlockiv(id_, i, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
            resourceView.constantBufferSize = static_cast<std::uint32_t>(blockSize);

            #ifdef GL_ARB_program_interface_query
            /* Query resource view properties */
            QueryBufferProperties(resourceView, GL_UNIFORM_BLOCK, i);
            #else
            /* Set binding slot to invalid index */
            resourceView.stageFlags = StageFlags::AllStages;
            resourceView.slot       = Constants::invalidSlot;
            #endif
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
            resourceView.type       = ResourceType::Buffer;
            resourceView.bindFlags  = BindFlags::RWStorageBuffer;

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
        UniformType uniformType = GLTypes::UnmapUniformType(type);

        if (uniformType == UniformType::Sampler || uniformType == UniformType::Image)
        {
            /* Append GLSL compiled texture-sampler as both texture and sampler resource */
            ShaderReflectionDescriptor::ResourceView resourceView;
            {
                /* Initia�ize name, type, and binding flags for resource view */
                resourceView.name       = std::string(uniformName.data());
                resourceView.type       = ResourceType::Texture;
                resourceView.bindFlags  = (uniformType == UniformType::Image ? BindFlags::RWStorageBuffer : BindFlags::SampleBuffer);

                /* Get binding slot from uniform value */
                GLint uniformValue      = 0;
                GLint uniformLocation   = glGetUniformLocation(id_, uniformName.data());

                glGetUniformiv(id_, uniformLocation, &uniformValue);

                resourceView.slot = static_cast<std::uint32_t>(uniformValue);

                #ifdef GL_ARB_program_interface_query
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

                if (GLGetProgramResourceProperties(id_, GL_UNIFORM, i, 7, props, params))
                {
                    /* Determine stage flags by program resource properties */
                    resourceView.stageFlags = GetStageFlagsFromResourceProperties(6, props, params);
                    resourceView.arraySize  = static_cast<std::uint32_t>(params[6]);
                }
                else
                #endif // /GL_ARB_program_interface_query
                {
                    /* Set binding slot to invalid index */
                    resourceView.stageFlags = StageFlags::AllStages;
                    resourceView.arraySize  = 1;
                }
            }
            reflection.resourceViews.push_back(resourceView);
            {
                resourceView.type       = ResourceType::Sampler;
                resourceView.bindFlags  = 0;
            }
            reflection.resourceViews.push_back(resourceView);
        }
        else
        {
            /* Append default uniform */
            ShaderReflectionDescriptor::Uniform uniform;
            {
                uniform.name        = std::string(uniformName.data());
                uniform.type        = uniformType;
                uniform.location    = glGetUniformLocation(id_, uniformName.data());
                uniform.size        = static_cast<std::uint32_t>(size);
            }
            reflection.uniforms.push_back(uniform);
        }
    }
}

#ifdef GL_ARB_program_interface_query

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

    if (GLGetProgramResourceProperties(id_, programInterface, resourceIndex, 7, props, params))
    {
        resourceView.stageFlags = GetStageFlagsFromResourceProperties(6, props, params);
        resourceView.slot       = static_cast<std::uint32_t>(params[6]);
    }
    else
    {
        /* Set binding slot to invalid index */
        resourceView.stageFlags = StageFlags::AllStages;
        resourceView.slot       = Constants::invalidSlot;
    }
}

#endif // /GL_ARB_program_interface_query


} // /namespace LLGL



// ================================================================================
