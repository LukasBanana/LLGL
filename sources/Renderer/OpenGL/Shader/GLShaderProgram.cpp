/*
 * GLShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLShaderProgram.h"
#include "GLShader.h"
#include "GLShaderBindingLayout.h"
#include "../GLTypes.h"
#include "../GLObjectUtils.h"
#include "../RenderState/GLStateManager.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionLoader.h"
#include "../../CheckedCast.h"
#include "../../../Core/Exception.h"
#include <LLGL/VertexAttribute.h>
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

    /* Build input layout for vertex shader */
    if (auto vs = desc.vertexShader)
    {
        auto vsGL = LLGL_CAST(GLShader*, vs);
        BindAttribLocations(vsGL->GetNumVertexAttribs(), vsGL->GetVertexAttribs());
    }

    /* Build input layout for vertex shader */
    if (auto fs = desc.fragmentShader)
    {
        auto fsGL = LLGL_CAST(GLShader*, fs);
        BindFragDataLocations(fsGL->GetNumFragmentAttribs(), fsGL->GetFragmentAttribs());
    }

    /* Build transform feedback varyings for vertex or geometry shader (latter one has higher order) */
    GLShader* shaderWithVaryings = nullptr;

    if (auto gs = desc.geometryShader)
    {
        auto gsGL = LLGL_CAST(GLShader*, gs);
        if (!gsGL->GetTransformFeedbackVaryings().empty())
            shaderWithVaryings = gsGL;
    }
    else if (auto vs = desc.vertexShader)
    {
        auto vsGL = LLGL_CAST(GLShader*, vs);
        if (!vsGL->GetTransformFeedbackVaryings().empty())
            shaderWithVaryings = vsGL;
    }

    if (shaderWithVaryings != nullptr)
    {
        const auto& varyings = shaderWithVaryings->GetTransformFeedbackVaryings();
        LinkProgram(varyings.size(), varyings.data());
    }
    else
        LinkProgram(0, nullptr);
}

GLShaderProgram::~GLShaderProgram()
{
    glDeleteProgram(id_);
    GLStateManager::Get().NotifyShaderProgramRelease(id_);
}

void GLShaderProgram::SetName(const char* name)
{
    GLSetObjectLabel(GL_PROGRAM, GetID(), name);
}

bool GLShaderProgram::HasErrors() const
{
    GLint status = 0;
    glGetProgramiv(id_, GL_LINK_STATUS, &status);
    return (status == GL_FALSE);
}

std::string GLShaderProgram::GetReport() const
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

bool GLShaderProgram::Reflect(ShaderReflection& reflection) const
{
    ShaderProgram::ClearShaderReflection(reflection);
    QueryReflection(reflection);
    ShaderProgram::FinalizeShaderReflection(reflection);
    return true;
}

UniformLocation GLShaderProgram::FindUniformLocation(const char* name) const
{
    if (id_ != 0)
        return static_cast<UniformLocation>(glGetUniformLocation(id_, name));
    else
        return -1;
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

void GLShaderProgram::Attach(Shader* shader)
{
    if (shader != nullptr)
    {
        auto shaderGL = LLGL_CAST(GLShader*, shader);

        /* Attach shader to shader program */
        glAttachShader(id_, shaderGL->GetID());
    }
}

void GLShaderProgram::BindAttribLocations(std::size_t numVertexAttribs, const GLShaderAttribute* vertexAttribs)
{
    /* Bind all vertex attribute locations */
    for (std::size_t i = 0; i < numVertexAttribs; ++i)
    {
        const auto& attr = vertexAttribs[i];
        glBindAttribLocation(id_, attr.index, attr.name);
    }
}

void GLShaderProgram::BindFragDataLocations(std::size_t numFragmentAttribs, const GLShaderAttribute* fragmentAttribs)
{
    #ifdef LLGL_OPENGL
    /* Only bind if extension is supported, otherwise the sahder won't have multiple fragment outpus anyway */
    if (HasExtension(GLExt::EXT_gpu_shader4))
    {
        for (std::size_t i = 0; i < numFragmentAttribs; ++i)
        {
            const auto& attr = fragmentAttribs[i];
            glBindFragDataLocation(id_, attr.index, attr.name);
        }
    }
    #endif
}

void GLShaderProgram::LinkProgram(std::size_t numVaryings, const char* const* varyings)
{
    /* Check if transform-feedback varyings must be specified (before or after shader linking) */
    if (numVaryings > 0 && varyings != nullptr)
    {
        /* For GL_EXT_transform_feedback the varyings must be specified BEFORE linking */
        #ifndef __APPLE__
        if (HasExtension(GLExt::EXT_transform_feedback))
        #endif
        {
            BuildTransformFeedbackVaryingsEXT(numVaryings, varyings);
            glLinkProgram(id_);
            return;
        }

        #ifdef GL_NV_transform_feedback
        /* For GL_NV_transform_feedback (Vendor specific) the varyings must be specified AFTER linking */
        if (HasExtension(GLExt::NV_transform_feedback))
        {
            glLinkProgram(id_);
            BuildTransformFeedbackVaryingsNV(numVaryings, varyings);
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

void GLShaderProgram::BuildTransformFeedbackVaryingsEXT(std::size_t numVaryings, const char* const* varyings)
{
    if (numVaryings == 0 || varyings == nullptr)
        return;

    /* Specify transform-feedback varyings by names */
    glTransformFeedbackVaryings(
        id_,
        static_cast<GLsizei>(numVaryings),
        reinterpret_cast<const GLchar* const*>(varyings),
        GL_INTERLEAVED_ATTRIBS
    );
}

#ifdef GL_NV_transform_feedback

void GLShaderProgram::BuildTransformFeedbackVaryingsNV(std::size_t numVaryings, const char* const* varyings)
{
    if (numVaryings == 0 || varyings == nullptr)
        return;

    /* Specify transform-feedback varyings by locations */
    std::vector<GLint> varyingLocations;
    varyingLocations.reserve(numVaryings);

    for (std::size_t i = 0; i < numVaryings; ++i)
    {
        /* Get varying location by its name */
        auto location = glGetVaryingLocationNV(id_, varyings[i]);
        if (location >= 0)
            varyingLocations.push_back(location);
        else
            throw std::invalid_argument("stream-output attribute \"" + std::string(varyings[i]) + "\" does not specify an active varying in GLSL shader program");
    }

    glTransformFeedbackVaryingsNV(
        id_,
        static_cast<GLsizei>(varyingLocations.size()),
        varyingLocations.data(),
        GL_INTERLEAVED_ATTRIBS_NV
    );
}

#endif

void GLShaderProgram::QueryReflection(ShaderReflection& reflection) const
{
    QueryVertexAttributes(reflection);
    QueryStreamOutputAttributes(reflection);
    QueryConstantBuffers(reflection);
    QueryStorageBuffers(reflection);
    QueryUniforms(reflection);
    QueryWorkGroupSize(reflection);
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
        #ifdef LLGL_OPENGL
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
        #endif // /LLGL_OPENGL
    }
    return { Format::R32Float, 0 };
}

static SystemValue FindSystemValue(const std::string& name)
{
    const std::pair<const char*, SystemValue> glslSystemValues[] =
    {
        { "gl_ClipDistance",    SystemValue::ClipDistance       },
        { "gl_CullDistance",    SystemValue::CullDistance       },
        { "gl_FragDepth",       SystemValue::Depth              },
        { "gl_FrontFacing",     SystemValue::FrontFacing        },
        { "gl_InstanceID",      SystemValue::InstanceID         }, // GLSL
        { "gl_InstanceIndex",   SystemValue::InstanceID         }, // SPIR-V
        { "gl_Position",        SystemValue::Position           },
        { "gl_FragCoord",       SystemValue::Position           },
        { "gl_PrimitiveID",     SystemValue::PrimitiveID        },
        { "gl_Layer",           SystemValue::RenderTargetIndex  },
        { "gl_SampleMask",      SystemValue::SampleMask         },
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
struct GLReflectVertexAttribute
{
    std::string     name;
    Format          format;
    std::uint32_t   semanticIndex;
    std::uint32_t   location;
};

void GLShaderProgram::QueryVertexAttributes(ShaderReflection& reflection) const
{
    /* Query active vertex attributes */
    std::vector<char> attribName;
    GLint numAttribs = 0, maxNameLength = 0;
    if (!QueryActiveAttribs(GL_ACTIVE_ATTRIBUTES, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, numAttribs, maxNameLength, attribName))
        return;

    std::vector<GLReflectVertexAttribute> attributes;
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
        [](const GLReflectVertexAttribute& lhs, const GLReflectVertexAttribute& rhs)
        {
            if (lhs.location < rhs.location)
                return true;
            if (lhs.location > rhs.location)
                return false;
            return (lhs.name < rhs.name);
        }
    );

    /* Copy attribute into final list and determine offsets */
    reflection.vertex.inputAttribs.resize(attributes.size());

    for (std::size_t i = 0; i < attributes.size(); ++i)
    {
        auto& src = attributes[i];
        auto& dst = reflection.vertex.inputAttribs[i];

        dst.name    = std::move(src.name);
        dst.format  = src.format;

        if (src.location == std::uint32_t(-1))
        {
            dst.location        = 0;
            dst.systemValue     = FindSystemValue(dst.name);
        }
        else
        {
            dst.location        = src.location;
            dst.semanticIndex   = src.semanticIndex;
        }
    }
}

void GLShaderProgram::QueryStreamOutputAttributes(ShaderReflection& reflection) const
{
    VertexAttribute soAttrib;

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
            soAttrib.name       = std::string(attribName.data());
            soAttrib.location   = i;
            //auto attr = UnmapAttribType(type);

            /* Insert stream-output attribute into list */
            #if 0
            while (attr.second-- > 0)
                reflection.vertex.outputAttribs.push_back(soAttrib);
            #else
            reflection.vertex.outputAttribs.push_back(soAttrib);
            #endif
        }
    }
    #ifdef GL_NV_transform_feedback
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
            soAttrib.name       = std::string(attribName.data());
            soAttrib.location   = i;
            //auto attr = UnmapAttribType(type);

            /* Insert stream-output attribute into list */
            #if 0
            while (attr.second-- > 0)
                reflection.vertex.outputAttribs.push_back(soAttrib);
            #else
            reflection.vertex.outputAttribs.push_back(soAttrib);
            #endif
        }
    }
    #endif
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

void GLShaderProgram::QueryConstantBuffers(ShaderReflection& reflection) const
{
    /* Query active uniform blocks */
    std::vector<char> blockName;
    GLint numUniformBlocks = 0, maxNameLength = 0;
    if (!QueryActiveAttribs(GL_ACTIVE_UNIFORM_BLOCKS, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, numUniformBlocks, maxNameLength, blockName))
        return;

    /* Iterate over all uniform blocks */
    for (GLuint i = 0; i < static_cast<GLuint>(numUniformBlocks); ++i)
    {
        ShaderResource resource;
        {
            /* Initialize resource view descriptor */
            resource.binding.type           = ResourceType::Buffer;
            resource.binding.stageFlags     = StageFlags::AllStages;
            resource.binding.bindFlags      = BindFlags::ConstantBuffer;

            /* Query uniform block name */
            GLsizei nameLength = 0;
            glGetActiveUniformBlockName(id_, i, maxNameLength, &nameLength, blockName.data());
            resource.binding.name = std::string(blockName.data());

            /* Query uniform block size */
            GLint blockSize = 0;
            glGetActiveUniformBlockiv(id_, i, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
            resource.constantBufferSize = static_cast<std::uint32_t>(blockSize);

            #ifdef GL_ARB_program_interface_query
            /* Query resource view properties */
            QueryBufferProperties(resource, GL_UNIFORM_BLOCK, i);
            #else
            /* Set binding slot to invalid index */
            resource.binding.stageFlags = StageFlags::AllStages;
            resource.binding.slot       = Constants::invalidSlot;
            #endif
        }
        reflection.resources.push_back(resource);
    }
}

void GLShaderProgram::QueryStorageBuffers(ShaderReflection& reflection) const
{
    #ifdef LLGL_GLEXT_SHADER_STORAGE_BUFFER_OBJECT

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
        ShaderResource resource;
        {
            /* Initialize resource view descriptor */
            resource.binding.type       = ResourceType::Buffer;
            resource.binding.bindFlags  = BindFlags::Storage;

            /* Query shader storage block name */
            GLsizei nameLength = 0;
            glGetProgramResourceName(id_, GL_SHADER_STORAGE_BLOCK, i, maxNameLength, &nameLength, blockName.data());
            resource.binding.name = std::string(blockName.data());

            /* Query resource view properties */
            QueryBufferProperties(resource, GL_SHADER_STORAGE_BLOCK, i);
        }
        reflection.resources.push_back(resource);
    }

    #endif
}

void GLShaderProgram::QueryUniforms(ShaderReflection& reflection) const
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
            ShaderResource resource;
            {
                /* Initialize name, type, and binding flags for resource view */
                resource.binding.name = std::string(uniformName.data());
                resource.binding.type = ResourceType::Texture;

                if (uniformType == UniformType::Image)
                    resource.binding.bindFlags = BindFlags::Storage;
                else
                    resource.binding.bindFlags = BindFlags::Sampled | BindFlags::CombinedSampler;

                /* Get binding slot from uniform value */
                GLint uniformValue      = 0;
                GLint uniformLocation   = glGetUniformLocation(id_, uniformName.data());

                glGetUniformiv(id_, uniformLocation, &uniformValue);

                resource.binding.slot = static_cast<std::uint32_t>(uniformValue);

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
                    resource.binding.stageFlags = GetStageFlagsFromResourceProperties(6, props, params);
                    resource.binding.arraySize  = static_cast<std::uint32_t>(params[6]);
                }
                else
                #endif // /GL_ARB_program_interface_query
                {
                    /* Set binding slot to invalid index */
                    resource.binding.stageFlags = StageFlags::AllStages;
                    resource.binding.arraySize  = 1;
                }
            }
            reflection.resources.push_back(resource);
            {
                resource.binding.type       = ResourceType::Sampler;
                resource.binding.bindFlags  = 0;
            }
            reflection.resources.push_back(resource);
        }
        else
        {
            /* Append default uniform */
            ShaderUniform uniform;
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

void GLShaderProgram::QueryWorkGroupSize(ShaderReflection& reflection) const
{
    #ifdef GL_ARB_compute_shader
    if (HasExtension(GLExt::ARB_compute_shader))
    {
        GLint params[3] = { 0 };
        glGetProgramiv(id_, GL_COMPUTE_WORK_GROUP_SIZE, params);
        if (params[0] > 0 && params[1] > 0 && params[2] > 0)
        {
            reflection.compute.workGroupSize.width  = static_cast<std::uint32_t>(params[0]);
            reflection.compute.workGroupSize.height = static_cast<std::uint32_t>(params[1]);
            reflection.compute.workGroupSize.depth  = static_cast<std::uint32_t>(params[2]);
        }
    }
    #endif // /GL_ARB_compute_shader
}

#ifdef GL_ARB_program_interface_query

void GLShaderProgram::QueryBufferProperties(ShaderResource& resource, GLenum programInterface, GLuint resourceIndex) const
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
        resource.binding.stageFlags = GetStageFlagsFromResourceProperties(6, props, params);
        resource.binding.slot       = static_cast<std::uint32_t>(params[6]);
    }
    else
    {
        /* Set binding slot to invalid index */
        resource.binding.stageFlags = StageFlags::AllStages;
        resource.binding.slot       = Constants::invalidSlot;
    }
}

#endif // /GL_ARB_program_interface_query


} // /namespace LLGL



// ================================================================================
