/*
 * GLShaderProgram.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLShaderProgram.h"
#include "GLLegacyShader.h"
#include "GLShaderBindingLayout.h"
#include "../GLTypes.h"
#include "../GLObjectUtils.h"
#include "../RenderState/GLStateManager.h"
#include "../RenderState/GLPipelineCache.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../../CheckedCast.h"
#include "../../../Core/Exception.h"
#include <LLGL/Report.h>
#include <LLGL/VertexAttribute.h>
#include <LLGL/Constants.h>
#include <LLGL/Utils/ForRange.h>
#include <vector>
#include <stdexcept>


namespace LLGL
{


#if LLGL_USE_NULL_FRAGMENT_SHADER

// Helper class to manage shared GL objects only necessary workaround issues with Mac implementation of OpenGL.
class SharedGLShader
{

    public:

        // Returns the native GL shader object and increments the reference counter.
        GLuint GetOrCreate(GLenum type, const char* source);

        // Decrements the reference counter and destroys the GL object once the counter reaches zero.
        void Release();

    private:

        GLuint      id_         = 0;
        std::size_t refCount_   = 0;

};

GLuint SharedGLShader::GetOrCreate(GLenum type, const char* source)
{
    if (id_ == 0)
    {
        /* Create shader and compile with specified source */
        id_ = glCreateShader(type);
        GLLegacyShader::CompileShaderSource(id_, source);

        /* Check for errors */
        if (!GLLegacyShader::GetCompileStatus(id_))
            LLGL_TRAP("compilation of shared GL shader failed:\n%s", GLLegacyShader::GetGLShaderLog(id_).c_str());
    }
    ++refCount_;
    return id_;
}

void SharedGLShader::Release()
{
    if (id_ != 0)
    {
        --refCount_;
        if (refCount_ == 0)
        {
            glDeleteShader(id_);
            id_ = 0;
        }
    }
}

static SharedGLShader g_nullFragmentShader;

#endif // /LLGL_USE_NULL_FRAGMENT_SHADER

GLShaderProgram::GLShaderProgram(
    std::size_t             numShaders,
    const Shader* const*    shaders,
    GLShader::Permutation   permutation,
    GLPipelineCache*        pipelineCache)
:
    GLShaderPipeline { glCreateProgram() }
{
    /* Try to load cached program binary first */
    if (pipelineCache != nullptr)
    {
        if (!(pipelineCache->HasProgramBinary(permutation) && pipelineCache->ProgramBinary(permutation, GetID())))
        {
            BuildProgramBinary(numShaders, shaders, permutation);
            pipelineCache->GetProgramBinary(permutation, GetID());
        }
    }
    else
        BuildProgramBinary(numShaders, shaders, permutation);

    /* Build pipeline signature */
    BuildSignature(numShaders, shaders, permutation);
}

GLShaderProgram::~GLShaderProgram()
{
    glDeleteProgram(GetID());
    GLStateManager::Get().NotifyShaderProgramRelease(this);
    #if LLGL_USE_NULL_FRAGMENT_SHADER
    if (hasNullFragmentShader_)
        g_nullFragmentShader.Release();
    #endif
}

void GLShaderProgram::Bind(GLStateManager& stateMngr)
{
    stateMngr.BindShaderProgram(GetID());
}

void GLShaderProgram::BindResourceSlots(const GLShaderBindingLayout& bindingLayout, const GLShaderBufferInterfaceMap* bufferInterfaceMap)
{
    if (bindingLayout_ != &bindingLayout)
    {
        bindingLayout.UniformAndBlockBinding(GetID(), bufferInterfaceMap);
        bindingLayout_ = &bindingLayout;
    }
}

void GLShaderProgram::QueryInfoLogs(Report& report)
{
    const bool hasErrors = !GLShaderProgram::GetLinkStatus(GetID());
    std::string log = GLShaderProgram::GetGLProgramLog(GetID());
    report.Reset(std::move(log), hasErrors);
}

void GLShaderProgram::QueryTexBufferNames(std::set<std::string>& outSamplerBufferNames, std::set<std::string>& outImageBufferNames) const
{
    outSamplerBufferNames.clear();
    outImageBufferNames.clear();
    GLShaderProgram::QueryTexBufferNames(GetID(), outSamplerBufferNames, outImageBufferNames);
}

bool GLShaderProgram::GetLinkStatus(GLuint program)
{
    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    return (status != GL_FALSE);
}

std::string GLShaderProgram::GetGLProgramLog(GLuint program)
{
    /* Query info log length */
    GLint infoLogLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

    if (infoLogLength > 0)
    {
        /* Store info log in byte buffer (because GL writes it's own null-terminator character!) */
        std::vector<char> infoLog;
        infoLog.resize(infoLogLength, '\0');

        /* Query info log output */
        GLsizei charsWritten = 0;
        glGetProgramInfoLog(program, infoLogLength, &charsWritten, infoLog.data());

        /* Convert byte buffer to string */
        return std::string(infoLog.data(), static_cast<std::size_t>(charsWritten));
    }

    return "";
}

void GLShaderProgram::BindAttribLocations(GLuint program, std::size_t numVertexAttribs, const GLShaderAttribute* vertexAttribs)
{
    /* Bind all vertex attribute locations */
    for_range(i, numVertexAttribs)
    {
        const auto& attr = vertexAttribs[i];
        glBindAttribLocation(program, attr.index, attr.name);
    }
}

void GLShaderProgram::BindFragDataLocations(GLuint program, std::size_t numFragmentAttribs, const GLShaderAttribute* fragmentAttribs)
{
    #if LLGL_OPENGL && EXT_gpu_shader4
    /* Only bind if extension is supported, otherwise the sahder won't have multiple fragment outpus anyway */
    if (HasExtension(GLExt::EXT_gpu_shader4))
    {
        for_range(i, numFragmentAttribs)
        {
            const auto& attr = fragmentAttribs[i];
            glBindFragDataLocation(program, attr.index, attr.name);
        }
    }
    #endif
}

static void BuildTransformFeedbackVaryingsEXT(GLuint program, std::size_t numVaryings, const char* const* varyings)
{
    #if !LLGL_GL_ENABLE_OPENGL2X

    if (numVaryings == 0 || varyings == nullptr)
        return;

    /* Specify transform-feedback varyings by names */
    glTransformFeedbackVaryings(
        program,
        static_cast<GLsizei>(numVaryings),
        reinterpret_cast<const GLchar* const*>(varyings),
        GL_INTERLEAVED_ATTRIBS
    );

    #endif // /!LLGL_GL_ENABLE_OPENGL2X
}

#if GL_NV_transform_feedback

static void BuildTransformFeedbackVaryingsNV(GLuint program, std::size_t numVaryings, const char* const* varyings)
{
    if (numVaryings == 0 || varyings == nullptr)
        return;

    /* Specify transform-feedback varyings by locations */
    std::vector<GLint> varyingLocations;
    varyingLocations.reserve(numVaryings);

    for_range(i, numVaryings)
    {
        /*
        Get varying location by its name.
        Silently ignore invalid names since the EXT extension doesn't report errors either
        and NV extension fails on gl_Position input.
        */
        GLint location = glGetVaryingLocationNV(program, varyings[i]);
        if (location >= 0)
            varyingLocations.push_back(location);
    }

    glTransformFeedbackVaryingsNV(
        program,
        static_cast<GLsizei>(varyingLocations.size()),
        varyingLocations.data(),
        GL_INTERLEAVED_ATTRIBS_NV
    );
}

#endif

void GLShaderProgram::LinkProgramWithTransformFeedbackVaryings(GLuint program, std::size_t numVaryings, const char* const* varyings)
{
    /* Check if transform-feedback varyings must be specified (before or after shader linking) */
    if (numVaryings > 0 && varyings != nullptr)
    {
        /* For GL_EXT_transform_feedback the varyings must be specified BEFORE linking */
        #ifndef __APPLE__
        if (HasExtension(GLExt::EXT_transform_feedback))
        #endif
        {
            BuildTransformFeedbackVaryingsEXT(program, numVaryings, varyings);
            glLinkProgram(program);
            return;
        }

        #if GL_NV_transform_feedback
        /* For GL_NV_transform_feedback (Vendor specific) the varyings must be specified AFTER linking */
        if (HasExtension(GLExt::NV_transform_feedback))
        {
            glLinkProgram(program);
            BuildTransformFeedbackVaryingsNV(program, numVaryings, varyings);
            return;
        }
        #endif
    }

    /* Just link shader program */
    glLinkProgram(program);
}

void GLShaderProgram::LinkProgram(GLuint program)
{
    glLinkProgram(program);
}

static bool GLQueryActiveAttribs(
    GLuint              program,
    GLenum              attribCountType,
    GLenum              attribNameLengthType,
    GLint&              outNumAttribs,
    GLint&              outMaxNameLength,
    std::vector<char>&  nameBuffer)
{
    /* Query number of active attributes */
    glGetProgramiv(program, attribCountType, &outNumAttribs);
    if (outNumAttribs <= 0)
        return false;

    /* Query maximal name length of all attributes */
    glGetProgramiv(program, attribNameLengthType, &outMaxNameLength);
    if (outMaxNameLength <= 0)
        return false;

    nameBuffer.resize(outMaxNameLength, '\0');

    return true;
}

static bool GLQueryActiveResources(
    GLuint              program,
    GLenum              programInterface,
    GLint&              outNumResources,
    GLint&              outMaxNameLength,
    std::vector<char>&  nameBuffer)
{
    #if LLGL_GLEXT_SHADER_STORAGE_BUFFER_OBJECT

    if (!HasExtension(GLExt::ARB_shader_storage_buffer_object) || !HasExtension(GLExt::ARB_program_interface_query))
        return false;

    /* Query number of shader storage blocks */
    glGetProgramInterfaceiv(program, programInterface, GL_ACTIVE_RESOURCES, &outNumResources);
    if (outNumResources <= 0)
        return false;

    /* Query maximal name length of all shader storage blocks */
    glGetProgramInterfaceiv(program, programInterface, GL_MAX_NAME_LENGTH, &outMaxNameLength);
    if (outMaxNameLength <= 0)
        return false;

    nameBuffer.resize(outMaxNameLength, '\0');

    return true;

    #else // LLGL_GLEXT_SHADER_STORAGE_BUFFER_OBJECT

    return false;

    #endif // /LLGL_GLEXT_SHADER_STORAGE_BUFFER_OBJECT
}

struct GLMatrixTypeFormat
{
    Format          format;
    std::uint32_t   rows;
};

// Vector format and number of vectors, e.g. mat2x3 --> { RGB32Float, 2 }
static GLMatrixTypeFormat UnmapAttribType(GLenum type)
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
        #if !LLGL_GL_ENABLE_OPENGL2X
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
        #endif // /!LLGL_GL_ENABLE_OPENGL2X
    }
    return { Format::R32Float, 0 };
}

struct GLSystemValueNamePair
{
    const char* name;
    SystemValue value;
};

static SystemValue FindSystemValue(const StringView& name)
{
    const GLSystemValueNamePair glslSystemValues[] =
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
        if (name == sysVal.name)
            return sysVal.value;
    }

    return SystemValue::Undefined;
}

// Internal struct for QueryVertexAttributes function
struct GLReflectVertexAttribute
{
    StringLiteral   name;
    Format          format;
    std::uint32_t   semanticIndex;
    std::uint32_t   location;
};

static void GLQueryVertexAttributes(GLuint program, ShaderReflection& reflection)
{
    /* Query active vertex attributes */
    std::vector<char> attribName;
    GLint numAttribs = 0, maxNameLength = 0;
    if (!GLQueryActiveAttribs(program, GL_ACTIVE_ATTRIBUTES, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, numAttribs, maxNameLength, attribName))
        return;

    std::vector<GLReflectVertexAttribute> attributes;
    attributes.reserve(static_cast<std::size_t>(numAttribs));

    /* Iterate over all vertex attributes */
    for_range(i, static_cast<GLuint>(numAttribs))
    {
        /* Query attribute information */
        GLint   size        = 0;
        GLenum  type        = 0;
        GLsizei nameLength  = 0;

        glGetActiveAttrib(program, i, maxNameLength, &nameLength, &size, &type, attribName.data());

        /* Convert attribute information */
        auto name = StringView(attribName.data(), static_cast<std::size_t>(nameLength));
        auto attr = UnmapAttribType(type);

        /* Get attribute location */
        auto location = static_cast<std::uint32_t>(glGetAttribLocation(program, attribName.data()));

        /* Insert vertex attribute into list */
        for_range(semanticIndex, attr.rows)
            attributes.push_back({ name, attr.format, semanticIndex, location });
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

    for_range(i, attributes.size())
    {
        GLReflectVertexAttribute&   src = attributes[i];
        VertexAttribute&            dst = reflection.vertex.inputAttribs[i];

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

static void GLQueryStreamOutputAttributes(GLuint program, ShaderReflection& reflection)
{
    #if !LLGL_GL_ENABLE_OPENGL2X

    VertexAttribute soAttrib;

    #ifndef __APPLE__
    if (HasExtension(GLExt::EXT_transform_feedback))
    #endif
    {
        /* Query active varyings */
        std::vector<char> attribName;
        GLint numVaryings = 0, maxNameLength = 0;
        if (!GLQueryActiveAttribs(program, GL_TRANSFORM_FEEDBACK_VARYINGS, GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH, numVaryings, maxNameLength, attribName))
            return;

        /* Iterate over all vertex attributes */
        for_range(i, static_cast<GLuint>(numVaryings))
        {
            /* Query attribute information */
            GLint   size        = 0;
            GLenum  type        = 0;
            GLsizei nameLength  = 0;

            glGetTransformFeedbackVarying(program, i, maxNameLength, &nameLength, &size, &type, attribName.data());

            /* Convert attribute information */
            soAttrib.name       = StringView(attribName.data(), static_cast<std::size_t>(nameLength));
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
    #if GL_NV_transform_feedback
    else if (HasExtension(GLExt::NV_transform_feedback))
    {
        /* Query active varyings */
        std::vector<char> attribName;
        GLint numVaryings = 0, maxNameLength = 0;
        if (!GLQueryActiveAttribs(program, GL_ACTIVE_VARYINGS_NV, GL_ACTIVE_VARYING_MAX_LENGTH_NV, numVaryings, maxNameLength, attribName))
            return;

        /* Iterate over all vertex attributes */
        for_range(i, static_cast<GLuint>(numVaryings))
        {
            /* Query attribute information */
            GLint   size        = 0;
            GLenum  type        = 0;
            GLsizei nameLength  = 0;

            glGetActiveVaryingNV(program, i, maxNameLength, &nameLength, &size, &type, attribName.data());

            /* Convert attribute information */
            soAttrib.name       = StringView(attribName.data(), static_cast<std::size_t>(nameLength));
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

    #endif // /!LLGL_GL_ENABLE_OPENGL2X
}

#ifdef LLGL_GLEXT_PROGRAM_INTERFACE_QUERY

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
        glGetProgramResourceiv(program, programInterface, resourceIndex, /*propCount:*/ count, props, /*paramCount:*/ count, nullptr, params);
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

    for_range(i, count)
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

static void GLQueryBufferProperties(GLuint program, ShaderResourceReflection& resource, GLenum programInterface, GLuint resourceIndex)
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

    if (GLGetProgramResourceProperties(program, programInterface, resourceIndex, 7, props, params))
    {
        resource.binding.stageFlags = GetStageFlagsFromResourceProperties(6, props, params);
        resource.binding.slot       = static_cast<std::uint32_t>(params[6]);
    }
    else
    {
        /* Set binding slot to invalid index */
        resource.binding.stageFlags = StageFlags::AllStages;
        resource.binding.slot       = LLGL_INVALID_SLOT;
    }
}

#endif // /LLGL_GLEXT_PROGRAM_INTERFACE_QUERY

static void GLQueryConstantBuffers(GLuint program, ShaderReflection& reflection)
{
    #if LLGL_GLEXT_UNIFORM_BUFFER_OBJECT

    if (!HasExtension(GLExt::ARB_uniform_buffer_object))
        return;

    /* Query active uniform blocks */
    std::vector<char> blockName;
    GLint numUniformBlocks = 0, maxNameLength = 0;
    if (!GLQueryActiveAttribs(program, GL_ACTIVE_UNIFORM_BLOCKS, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, numUniformBlocks, maxNameLength, blockName))
        return;

    /* Iterate over all uniform blocks */
    for_range(i, static_cast<GLuint>(numUniformBlocks))
    {
        ShaderResourceReflection resource;
        {
            /* Initialize resource view descriptor */
            resource.binding.type           = ResourceType::Buffer;
            resource.binding.stageFlags     = StageFlags::AllStages;
            resource.binding.bindFlags      = BindFlags::ConstantBuffer;

            /* Query uniform block name */
            GLsizei nameLength = 0;
            glGetActiveUniformBlockName(program, i, maxNameLength, &nameLength, blockName.data());
            resource.binding.name = StringView(blockName.data(), static_cast<std::size_t>(nameLength));

            /* Query uniform block size */
            GLint blockSize = 0;
            glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
            resource.constantBufferSize = static_cast<std::uint32_t>(blockSize);

            #ifdef LLGL_GLEXT_PROGRAM_INTERFACE_QUERY
            /* Query resource view properties */
            GLQueryBufferProperties(program, resource, GL_UNIFORM_BLOCK, i);
            #else
            /* Set binding slot to invalid index */
            resource.binding.stageFlags = StageFlags::AllStages;
            resource.binding.slot       = LLGL_INVALID_SLOT;
            #endif
        }
        reflection.resources.push_back(resource);
    }

    #endif // /LLGL_GLEXT_UNIFORM_BUFFER_OBJECT
}

static void GLQueryStorageBuffers(GLuint program, ShaderReflection& reflection)
{
    #if LLGL_GLEXT_SHADER_STORAGE_BUFFER_OBJECT

    GLint numStorageBlocks = 0;
    GLint maxNameLength = 0;
    std::vector<char> blockName;
    if (!GLQueryActiveResources(program, GL_SHADER_STORAGE_BLOCK, numStorageBlocks, maxNameLength, blockName))
        return;

    /* Iterate over all shader storage blocks */
    for_range(i, static_cast<GLuint>(numStorageBlocks))
    {
        ShaderResourceReflection resource;
        {
            /* Initialize resource view descriptor */
            resource.binding.type       = ResourceType::Buffer;
            resource.binding.bindFlags  = BindFlags::Storage;

            /* Query shader storage block name */
            GLsizei nameLength = 0;
            glGetProgramResourceName(program, GL_SHADER_STORAGE_BLOCK, i, maxNameLength, &nameLength, blockName.data());
            resource.binding.name = StringView(blockName.data(), static_cast<std::size_t>(nameLength));

            #ifdef LLGL_GLEXT_PROGRAM_INTERFACE_QUERY
            /* Query resource view properties */
            GLQueryBufferProperties(program, resource, GL_SHADER_STORAGE_BLOCK, i);
            #else
            /* Set binding slot to invalid index */
            resource.binding.stageFlags = StageFlags::AllStages;
            resource.binding.slot       = LLGL_INVALID_SLOT;
            #endif

            /* Assume SSBOs to have structured read/write access */
            resource.storageBufferType  = StorageBufferType::RWStructuredBuffer;
        }
        reflection.resources.push_back(resource);
    }

    #endif // /LLGL_GLEXT_SHADER_STORAGE_BUFFER_OBJECT
}

static void GLQueryUniforms(GLuint program, ShaderReflection& reflection)
{
    /* Query active uniforms */
    std::vector<char> uniformName;
    GLint numUniforms = 0, maxNameLength = 0;
    if (!GLQueryActiveAttribs(program, GL_ACTIVE_UNIFORMS, GL_ACTIVE_UNIFORM_MAX_LENGTH, numUniforms, maxNameLength, uniformName))
        return;

    /* Iterate over all uniforms */
    for_range(i, static_cast<GLuint>(numUniforms))
    {
        /* Query uniform block name */
        GLsizei nameLength  = 0;
        GLint   size        = 0;
        GLenum  type        = 0;

        glGetActiveUniform(program, i, maxNameLength, &nameLength, &size, &type, uniformName.data());

        /* Integrate uniform into reflection containers */
        UniformType uniformType = GLTypes::UnmapUniformType(type);

        if (uniformType == UniformType::Sampler || uniformType == UniformType::Image)
        {
            /* Append GLSL compiled texture-sampler as both texture and sampler resource */
            ShaderResourceReflection resource;
            {
                /* Initialize name, type, and binding flags for resource view */
                resource.binding.name = StringView(uniformName.data(), static_cast<std::size_t>(nameLength));
                resource.binding.type = ResourceType::Texture;

                if (uniformType == UniformType::Image)
                    resource.binding.bindFlags = BindFlags::Storage;
                else
                    resource.binding.bindFlags = BindFlags::Sampled | BindFlags::CombinedSampler;

                /* Get binding slot from uniform value */
                GLint uniformValue      = 0;
                GLint uniformLocation   = glGetUniformLocation(program, uniformName.data());

                glGetUniformiv(program, uniformLocation, &uniformValue);

                resource.binding.slot = static_cast<std::uint32_t>(uniformValue);

                #ifdef LLGL_GLEXT_PROGRAM_INTERFACE_QUERY
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

                if (GLGetProgramResourceProperties(program, GL_UNIFORM, i, 7, props, params))
                {
                    /* Determine stage flags by program resource properties */
                    resource.binding.stageFlags = GetStageFlagsFromResourceProperties(6, props, params);
                    resource.binding.arraySize  = static_cast<std::uint32_t>(params[6]);
                }
                else
                #endif // /LLGL_GLEXT_PROGRAM_INTERFACE_QUERY
                {
                    /* Set binding slot to invalid index */
                    resource.binding.stageFlags = StageFlags::AllStages;
                    resource.binding.arraySize  = 0;
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
            UniformDescriptor uniform;
            {
                uniform.name        = StringView(uniformName.data(), static_cast<std::size_t>(nameLength));
                uniform.type        = uniformType;
                uniform.arraySize   = static_cast<std::uint32_t>(size);
            }
            reflection.uniforms.push_back(uniform);
        }
    }
}

static void GLQueryWorkGroupSize(GLuint program, ShaderReflection& reflection)
{
    #ifdef GL_ARB_compute_shader
    if (HasExtension(GLExt::ARB_compute_shader))
    {
        GLint params[3] = { 0 };
        glGetProgramiv(program, GL_COMPUTE_WORK_GROUP_SIZE, params);
        if (params[0] > 0 && params[1] > 0 && params[2] > 0)
        {
            reflection.compute.workGroupSize.width  = static_cast<std::uint32_t>(params[0]);
            reflection.compute.workGroupSize.height = static_cast<std::uint32_t>(params[1]);
            reflection.compute.workGroupSize.depth  = static_cast<std::uint32_t>(params[2]);
        }
    }
    #endif // /GL_ARB_compute_shader
}

void GLShaderProgram::QueryReflection(GLuint program, GLenum shaderStage, ShaderReflection& reflection)
{
    GLQueryVertexAttributes(program, reflection);
    GLQueryStreamOutputAttributes(program, reflection);
    GLQueryConstantBuffers(program, reflection);
    GLQueryStorageBuffers(program, reflection);
    GLQueryUniforms(program, reflection);
    #ifdef GL_ARB_compute_shader
    if (shaderStage == GL_COMPUTE_SHADER)
        GLQueryWorkGroupSize(program, reflection);
    #endif
}

void GLShaderProgram::QueryTexBufferNames(GLuint program, std::set<std::string>& samplerBufferNames, std::set<std::string>& imageBufferNames)
{
    /* Query active uniform blocks */
    std::vector<char> blockName;
    GLint numUniforms = 0, maxNameLength = 0;
    if (!GLQueryActiveAttribs(program, GL_ACTIVE_UNIFORMS, GL_ACTIVE_UNIFORM_MAX_LENGTH, numUniforms, maxNameLength, blockName))
        return;

    for_range(i, static_cast<GLuint>(numUniforms))
    {
        /* Get active uniform name */
        GLsizei nameLength = 0;
        GLint size = 0;
        GLenum type = 0;
        glGetActiveUniform(program, i, maxNameLength, &nameLength, &size, &type, blockName.data());
        std::string uniformName(blockName.data(), static_cast<std::size_t>(nameLength));

        /* Map sampler buffer and image buffer names to output sets */
        switch (type)
        {
            #if defined(GL_SAMPLER_BUFFER) && defined(GL_INT_SAMPLER_BUFFER) && defined(GL_UNSIGNED_INT_SAMPLER_BUFFER)
            case GL_SAMPLER_BUFFER:
            case GL_INT_SAMPLER_BUFFER:
            case GL_UNSIGNED_INT_SAMPLER_BUFFER:
                samplerBufferNames.insert(std::move(uniformName));
                break;
            #endif

            #if defined(GL_IMAGE_BUFFER) && defined(GL_INT_IMAGE_BUFFER) && defined(GL_UNSIGNED_INT_IMAGE_BUFFER)
            case GL_IMAGE_BUFFER:
            case GL_INT_IMAGE_BUFFER:
            case GL_UNSIGNED_INT_IMAGE_BUFFER:
                imageBufferNames.insert(std::move(uniformName));
                break;
            #endif

            default:
                break;
        }
    }
}


/*
 * ======= Private: =======
 */

struct GLOrderedShaders
{
    const GLShader* vertexShader                = nullptr;
    const GLShader* tessEvaluationShader        = nullptr;
    const GLShader* geometryShader              = nullptr;
    const GLShader* fragmentShader              = nullptr;
    const GLShader* shaderWithFlippedYPosition  = nullptr; // Last shader that modifies gl_Position (vertex, tessellation-evaluation, or geometry)

    GLuint GetGLShaderID(const GLShader* shader) const
    {
        const GLShader::Permutation permutation =
        (
            shader == shaderWithFlippedYPosition
                ? GLShader::PermutationFlippedYPosition
                : GLShader::PermutationDefault
        );
        return shader->GetID(permutation);
    }
};

static void AttachGLLegacyShaders(
    GLuint                  program,
    std::size_t             numShaders,
    const Shader* const*    shaders,
    GLOrderedShaders&       orderedShaders)
{
    for_range(i, numShaders)
    {
        if (const Shader* shader = shaders[i])
        {
            /* Attach shader to shader program */
            auto shaderGL = LLGL_CAST(const GLLegacyShader*, shader);
            glAttachShader(program, orderedShaders.GetGLShaderID(shaderGL));

            switch (shaderGL->GetType())
            {
                case ShaderType::Vertex:
                    orderedShaders.vertexShader = shaderGL;
                    break;
                case ShaderType::TessEvaluation:
                    orderedShaders.tessEvaluationShader = shaderGL;
                    break;
                case ShaderType::Geometry:
                    orderedShaders.geometryShader = shaderGL;
                    break;
                case ShaderType::Fragment:
                    orderedShaders.fragmentShader = shaderGL;
                    break;
                default:
                    break;
            }
        }
    }
}

void GLShaderProgram::BuildProgramBinary(
    std::size_t             numShaders,
    const Shader* const*    shaders,
    GLShader::Permutation   permutation)
{
    GLOrderedShaders orderedShaders;

    /* Find last shader in pipeline that transforms gl_Position if such permutation is requested */
    if (permutation == GLShader::PermutationFlippedYPosition)
        orderedShaders.shaderWithFlippedYPosition = GLPipelineSignature::FindFinalGLPositionShader(numShaders, shaders);

    /* Attach all specified shaders to this shader program */
    AttachGLLegacyShaders(GetID(), numShaders, shaders, orderedShaders);

    #if LLGL_USE_NULL_FRAGMENT_SHADER
    /*
    Mac implementation of OpenGL violates GL spec and always requires a fragment shader,
    so we create a dummy if not specified by client.
    */
    if (orderedShaders.fragmentShader == nullptr)
    {
        const GLchar* nullFragmentShaderSource =
            #ifdef LLGL_OPENGL
            "#version 330 core\n"
            #else
            "#version 300 es\n"
            #endif
            "void main() {}\n"
        ;
        const GLuint nullFragmentShader = g_nullFragmentShader.GetOrCreate(GL_FRAGMENT_SHADER, nullFragmentShaderSource);
        glAttachShader(GetID(), nullFragmentShader);
        hasNullFragmentShader_ = true;
    }
    #endif // /LLGL_USE_NULL_FRAGMENT_SHADER

    /* Build input layout for vertex shader */
    if (const GLShader* vs = orderedShaders.vertexShader)
        GLShaderProgram::BindAttribLocations(GetID(), vs->GetNumVertexAttribs(), vs->GetVertexAttribs());

    /* Build output layout for fragment shader */
    if (const GLShader* fs = orderedShaders.fragmentShader)
        GLShaderProgram::BindFragDataLocations(GetID(), fs->GetNumFragmentAttribs(), fs->GetFragmentAttribs());

    /* Build transform feedback varyings for vertex or geometry shader and link program */
    const GLShader* shaderWithVaryings = nullptr;

    if (const GLShader* gs = orderedShaders.geometryShader)
    {
        if (!gs->GetTransformFeedbackVaryings().empty())
            shaderWithVaryings = gs;
    }
    else if (const GLShader* ts = orderedShaders.tessEvaluationShader)
    {
        if (!ts->GetTransformFeedbackVaryings().empty())
            shaderWithVaryings = ts;
    }
    else if (const GLShader* vs = orderedShaders.vertexShader)
    {
        if (!vs->GetTransformFeedbackVaryings().empty())
            shaderWithVaryings = vs;
    }

    /* Link shader program */
    if (shaderWithVaryings != nullptr)
    {
        const auto& varyings = shaderWithVaryings->GetTransformFeedbackVaryings();
        GLShaderProgram::LinkProgramWithTransformFeedbackVaryings(GetID(), varyings.size(), varyings.data());
    }
    else
        GLShaderProgram::LinkProgram(GetID());
}


} // /namespace LLGL



// ================================================================================
