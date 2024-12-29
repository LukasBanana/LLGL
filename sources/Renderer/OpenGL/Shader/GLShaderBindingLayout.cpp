/*
 * GLShaderBindingLayout.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLShaderBindingLayout.h"
#include "GLShaderBufferInterfaceMap.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../Ext/GLExtensions.h"
#include "../RenderState/GLPipelineLayout.h"
#include "../RenderState/GLStateManager.h"
#include "../../../Core/MacroUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


namespace LLGL
{


GLShaderBindingLayout::GLShaderBindingLayout(const GLPipelineLayout& pipelineLayout)
{
    BuildUniformBindings(pipelineLayout);
    BuildUniformBlockBindings(pipelineLayout);
    BuildShaderStorageBindings(pipelineLayout);
}

void GLShaderBindingLayout::UniformAndBlockBinding(GLuint program, const GLShaderBufferInterfaceMap* bufferInterfaceMap, GLStateManager* stateMngr) const
{
    bool isShaderProgramStored = false;
    std::size_t resourceIndex = 0;

    /* Set uniform bindings */
    #if LLGL_GLEXT_SEPARATE_SHADER_OBJECTS
    if (HasExtension(GLExt::ARB_separate_shader_objects))
    {
        for_range(i, numUniformBindings_)
        {
            const NamedResourceBinding& resource = bindings_[resourceIndex++];
            GLShaderBindingLayout::GLSetProgramUniformBinding(program, resource);
        }
    }
    else
    #endif
    {
        if (stateMngr != nullptr)
        {
            stateMngr->PushBoundShaderProgram();
            stateMngr->BindShaderProgram(program);
            isShaderProgramStored = true;
        }

        for_range(i, numUniformBindings_)
        {
            const NamedResourceBinding& resource = bindings_[resourceIndex++];
            GLShaderBindingLayout::GLSetUniformBinding(program, resource);
        }
    }

    /* Set uniform-block bindings */
    #if LLGL_GLEXT_UNIFORM_BUFFER_OBJECT
    for_range(i, numUniformBlockBindings_)
    {
        const NamedResourceBinding& resource = bindings_[resourceIndex++];
        GLuint blockIndex = glGetUniformBlockIndex(program, resource.name.c_str());
        if (blockIndex != GL_INVALID_INDEX)
            glUniformBlockBinding(program, blockIndex, resource.slot);
    }
    #endif // /LLGL_GLEXT_UNIFORM_BUFFER_OBJECT

    if (bufferInterfaceMap != nullptr && !bufferInterfaceMap->HasSSBOEntriesOnly())
    {
        /* Set interface bindings for SSBOs, sampler buffers, and image buffers */
        ArrayView<GLBufferInterface> bufferInterfaces = bufferInterfaceMap->GetInterfaces();
        LLGL_ASSERT(bufferInterfaces.size() == numShaderStorageBindings_);
        for (GLBufferInterface bufferInterface : bufferInterfaces)
        {
            const NamedResourceBinding& resource = bindings_[resourceIndex++];
            switch (bufferInterface)
            {
                case GLBufferInterface_SSBO:
                {
                    /* Set shader-storage binding (not supported in GLES) */
                    #if LLGL_GLEXT_SHADER_STORAGE_BUFFER_OBJECT && LLGL_OPENGL
                    GLuint blockIndex = glGetProgramResourceIndex(program, GL_SHADER_STORAGE_BLOCK, resource.name.c_str());
                    if (blockIndex != GL_INVALID_INDEX)
                        glShaderStorageBlockBinding(program, blockIndex, resource.slot);
                    #endif
                }
                break;

                case GLBufferInterface_Sampler:
                case GLBufferInterface_Image:
                {
                    #if LLGL_GLEXT_SEPARATE_SHADER_OBJECTS
                    if (HasExtension(GLExt::ARB_separate_shader_objects))
                    {
                        GLShaderBindingLayout::GLSetProgramUniformBinding(program, resource);
                    }
                    else
                    #endif // /LLGL_GLEXT_SEPARATE_SHADER_OBJECTS
                    {
                        GLShaderBindingLayout::GLSetUniformBinding(program, resource);
                    }
                }
                break;
            }
        }
    }
    else
    {
        /* Set shader-storage bindings (not supported in GLES) */
        #if LLGL_GLEXT_SHADER_STORAGE_BUFFER_OBJECT && LLGL_OPENGL
        for_range(i, numShaderStorageBindings_)
        {
            const NamedResourceBinding& resource = bindings_[resourceIndex++];
            GLuint blockIndex = glGetProgramResourceIndex(program, GL_SHADER_STORAGE_BLOCK, resource.name.c_str());
            if (blockIndex != GL_INVALID_INDEX)
                glShaderStorageBlockBinding(program, blockIndex, resource.slot);
        }
        #endif
    }

    if (isShaderProgramStored)
        stateMngr->PopBoundShaderProgram();
}

bool GLShaderBindingLayout::HasBindings() const
{
    return ((numUniformBindings_ | numUniformBlockBindings_ | numShaderStorageBindings_) != 0);
}

bool GLShaderBindingLayout::HasShaderStorageBindings() const
{
    return (numShaderStorageBindings_ != 0);
}

int GLShaderBindingLayout::CompareSWO(const GLShaderBindingLayout& lhs, const GLShaderBindingLayout& rhs)
{
    /* Compare number of bindings first; if equal we can use one of the arrays only */
    LLGL_COMPARE_MEMBER_SWO( bindings_.size() );

    for_range(i, lhs.bindings_.size())
    {
        LLGL_COMPARE_MEMBER_SWO( bindings_[i].slot );
        LLGL_COMPARE_MEMBER_SWO( bindings_[i].name );
    }

    return 0;
}


/*
 * ======= Private: =======
 */

void GLShaderBindingLayout::BuildUniformBindings(const GLPipelineLayout& pipelineLayout)
{
    /* Gather all uniform bindings from heap resource descriptors */
    ArrayView<GLHeapResourceBinding> heapBindings = pipelineLayout.GetHeapBindings();
    for (std::size_t i = 0; i < heapBindings.size();)
    {
        const GLHeapResourceBinding& binding = heapBindings[i];

        /* Don't append uniform binding if this is already handled as a combined texture-sampler */
        if (!binding.name.empty() && binding.combiners == 0)
        {
            if (binding.type == ResourceType::Texture)
            {
                /* Skip over next binding descriptors depending on array size, since this list has already been expanded */
                const std::uint32_t clampedArraySize = std::max<std::uint32_t>(1u, binding.arraySize);
                AppendUniformBinding(binding.name.c_str(), binding.slot, clampedArraySize);
                i += clampedArraySize;
                continue;
            }
        }
        ++i;
    }

    /* Gather all uniform bindings from dynamic resource descriptors */
    for_range(i, pipelineLayout.GetBindingNames().size())
    {
        const std::string& name = pipelineLayout.GetBindingNames()[i];
        if (!name.empty())
        {
            const GLPipelineResourceBinding& binding = pipelineLayout.GetBindings()[i];

            /* Don't append uniform binding if this is already handled as a combined texture-sampler */
            if (binding.combiners == 0)
            {
                if (binding.type == GLResourceType_Texture || binding.type == GLResourceType_Image)
                    AppendUniformBinding(name, binding.slot);
            }
        }
    }

    /* Append all uniform bindings for combined texture-samplers */
    for_range(i, pipelineLayout.GetCombinedSamplerNames().size())
    {
        const std::string& name = pipelineLayout.GetCombinedSamplerNames()[i];
        if (!name.empty())
            AppendUniformBinding(name, pipelineLayout.GetCombinedSamplerSlots()[i]);
    }
}

void GLShaderBindingLayout::BuildUniformBlockBindings(const GLPipelineLayout& pipelineLayout)
{
    /* Gather all uniform-block bindings from heap resource descriptors */
    for (const GLHeapResourceBinding& binding : pipelineLayout.GetHeapBindings())
    {
        if (!binding.name.empty())
        {
            if (binding.type == ResourceType::Buffer && (binding.bindFlags & BindFlags::ConstantBuffer) != 0)
                AppendUniformBlockBinding(binding.name.c_str(), binding.slot);
        }
    }

    /* Gather all uniform-block bindings from dynamic resource descriptors */
    for_range(i, pipelineLayout.GetBindings().size())
    {
        const std::string& name = pipelineLayout.GetBindingNames()[i];
        if (!name.empty())
        {
            const GLPipelineResourceBinding& binding = pipelineLayout.GetBindings()[i];
            if (binding.type == GLResourceType_UBO)
                AppendUniformBlockBinding(name, binding.slot);
        }
    }
}

void GLShaderBindingLayout::BuildShaderStorageBindings(const GLPipelineLayout& pipelineLayout)
{
    /* Gather all shader-storage bindings from heap resource descriptors */
    for (const GLHeapResourceBinding& binding : pipelineLayout.GetHeapBindings())
    {
        if (!binding.name.empty() && binding.IsSSBO())
            AppendShaderStorageBinding(binding.name.c_str(), binding.slot);
    }

    /* Gather all shader-storage bindings from dynamic resource descriptors */
    for_range(i, pipelineLayout.GetBindings().size())
    {
        const GLPipelineResourceBinding& binding = pipelineLayout.GetBindings()[i];
        const std::string& name = pipelineLayout.GetBindingNames()[i];
        if (!name.empty() && binding.IsSSBO())
            AppendShaderStorageBinding(name, binding.slot);
    }
}

void GLShaderBindingLayout::AppendUniformBinding(const std::string& name, std::uint32_t slot, std::uint32_t size)
{
    bindings_.push_back({ name, slot, size });
    ++numUniformBindings_;
}

void GLShaderBindingLayout::AppendUniformBlockBinding(const std::string& name, std::uint32_t slot)
{
    bindings_.push_back({ name, slot, 1u });
    ++numUniformBlockBindings_;
}

void GLShaderBindingLayout::AppendShaderStorageBinding(const std::string& name, std::uint32_t slot)
{
    bindings_.push_back({ name, slot, 1u });
    ++numShaderStorageBindings_;
}

#if LLGL_GLEXT_SEPARATE_SHADER_OBJECTS

void GLShaderBindingLayout::GLSetProgramUniformBinding(GLuint program, const NamedResourceBinding& resource)
{
    GLint location = glGetUniformLocation(program, resource.name.c_str());
    if (location != GL_INVALID_INDEX)
    {
        for_range(j, resource.size)
            glProgramUniform1i(program, location + j, static_cast<GLint>(resource.slot + j));
    }
}

#endif // /LLGL_GLEXT_SEPARATE_SHADER_OBJECTS

void GLShaderBindingLayout::GLSetUniformBinding(GLuint program, const NamedResourceBinding& resource)
{
    GLint location = glGetUniformLocation(program, resource.name.c_str());
    if (location != GL_INVALID_INDEX)
    {
        for_range(j, resource.size)
            glUniform1i(location + j, static_cast<GLint>(resource.slot + j));
    }
}


} // /namespace LLGL



// ================================================================================
