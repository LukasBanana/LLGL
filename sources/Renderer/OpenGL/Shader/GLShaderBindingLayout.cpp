/*
 * GLShaderBindingLayout.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLShaderBindingLayout.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../Ext/GLExtensions.h"
#include "../RenderState/GLStateManager.h"
#include "../../../Core/HelperMacros.h"
#include <LLGL/Misc/ForRange.h>


namespace LLGL
{


GLShaderBindingLayout::GLShaderBindingLayout(const GLPipelineLayout& pipelineLayout)
{
    BuildUniformBindings(pipelineLayout);
    BuildUniformBlockBindings(pipelineLayout);
    BuildShaderStorageBindings(pipelineLayout);
}

void GLShaderBindingLayout::UniformAndBlockBinding(GLuint program, GLStateManager* stateMngr) const
{
    std::size_t resourceIndex = 0;

    /* Set uniform bindings */
    #ifdef GL_ARB_separate_shader_objects
    if (HasExtension(GLExt::ARB_separate_shader_objects))
    {
        for_range(i, numUniformBindings_)
        {
            const auto& resource = bindings_[resourceIndex++];
            auto blockIndex = glGetUniformLocation(program, resource.name.c_str());
            if (blockIndex != GL_INVALID_INDEX)
                glProgramUniform1i(program, blockIndex, static_cast<GLint>(resource.slot));
        }
    }
    else
    #endif
    if (stateMngr != nullptr)
    {
        stateMngr->PushBoundShaderProgram();
        stateMngr->BindShaderProgram(program);
        {
            for_range(i, numUniformBindings_)
            {
                const auto& resource = bindings_[resourceIndex++];
                auto blockIndex = glGetUniformLocation(program, resource.name.c_str());
                if (blockIndex != GL_INVALID_INDEX)
                    glUniform1i(blockIndex, static_cast<GLint>(resource.slot));
            }
        }
        stateMngr->PopBoundShaderProgram();
    }
    else
    {
        for_range(i, numUniformBindings_)
        {
            const auto& resource = bindings_[resourceIndex++];
            auto blockIndex = glGetUniformLocation(program, resource.name.c_str());
            if (blockIndex != GL_INVALID_INDEX)
                glUniform1i(blockIndex, static_cast<GLint>(resource.slot));
        }
    }

    /* Set uniform-block bindings */
    for_range(i, numUniformBlockBindings_)
    {
        const auto& resource = bindings_[resourceIndex++];
        auto blockIndex = glGetUniformBlockIndex(program, resource.name.c_str());
        if (blockIndex != GL_INVALID_INDEX)
            glUniformBlockBinding(program, blockIndex, resource.slot);
    }

    /* Set shader-storage bindings */
    #ifdef LLGL_GLEXT_SHADER_STORAGE_BUFFER_OBJECT
    for_range(i, numShaderStorageBindings_)
    {
        const auto& resource = bindings_[resourceIndex++];
        auto blockIndex = glGetProgramResourceIndex(program, GL_SHADER_STORAGE_BLOCK, resource.name.c_str());
        if (blockIndex != GL_INVALID_INDEX)
            glShaderStorageBlockBinding(program, blockIndex, resource.slot);
    }
    #endif
}

bool GLShaderBindingLayout::HasBindings() const
{
    return ((numUniformBindings_ | numUniformBlockBindings_ | numShaderStorageBindings_) != 0);
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
    for (const auto& binding : pipelineLayout.GetHeapBindings())
    {
        if (!binding.name.empty())
        {
            if (binding.type == ResourceType::Sampler || binding.type == ResourceType::Texture)
                AppendUniformBinding(binding.name, binding.slot);
        }
    }

    /* Gather all uniform bindings from dynamic resource descriptors */
    for_range(i, pipelineLayout.GetBindings().size())
    {
        const auto& name = pipelineLayout.GetBindingNames()[i];
        if (!name.empty())
        {
            const auto& binding = pipelineLayout.GetBindings()[i];
            if (binding.type == GLResourceType_Texture  ||
                binding.type == GLResourceType_Image    ||
                binding.type == GLResourceType_Sampler  ||
                binding.type == GLResourceType_GL2XSampler)
            {
                AppendUniformBinding(name, binding.slot);
            }
        }
    }

    /* Gather all uniform bindings from dynamic resource descriptors */
    for_range(i, pipelineLayout.GetStaticSamplerSlots().size())
    {
        const auto& name = pipelineLayout.GetStaticSamplerNames()[i];
        if (!name.empty())
            AppendUniformBinding(name, pipelineLayout.GetStaticSamplerSlots()[i]);
    }
}

void GLShaderBindingLayout::BuildUniformBlockBindings(const GLPipelineLayout& pipelineLayout)
{
    /* Gather all uniform-block bindings from heap resource descriptors */
    for (const auto& binding : pipelineLayout.GetHeapBindings())
    {
        if (!binding.name.empty())
        {
            if (binding.type == ResourceType::Buffer && (binding.bindFlags & BindFlags::ConstantBuffer) != 0)
                AppendUniformBlockBinding(binding.name, binding.slot);
        }
    }

    /* Gather all uniform-block bindings from dynamic resource descriptors */
    for_range(i, pipelineLayout.GetBindings().size())
    {
        const auto& name = pipelineLayout.GetBindingNames()[i];
        if (!name.empty())
        {
            const auto& binding = pipelineLayout.GetBindings()[i];
            if (binding.type == GLResourceType_UBO)
                AppendUniformBlockBinding(name, binding.slot);
        }
    }
}

void GLShaderBindingLayout::BuildShaderStorageBindings(const GLPipelineLayout& pipelineLayout)
{
    /* Gather all shader-storage bindings from heap resource descriptors */
    for (const auto& binding : pipelineLayout.GetHeapBindings())
    {
        if (!binding.name.empty())
        {
            if (binding.type == ResourceType::Buffer && (binding.bindFlags & (BindFlags::Storage | BindFlags::Sampled)) != 0)
                AppendShaderStorageBinding(binding.name, binding.slot);
        }
    }

    /* Gather all shader-storage bindings from dynamic resource descriptors */
    for_range(i, pipelineLayout.GetBindings().size())
    {
        const auto& name = pipelineLayout.GetBindingNames()[i];
        if (!name.empty())
        {
            const auto& binding = pipelineLayout.GetBindings()[i];
            if (binding.type == GLResourceType_SSBO)
                AppendShaderStorageBinding(name, binding.slot);
        }
    }
}

void GLShaderBindingLayout::AppendUniformBinding(const std::string& name, std::uint32_t slot)
{
    bindings_.push_back({ name, slot });
    ++numUniformBindings_;
}

void GLShaderBindingLayout::AppendUniformBlockBinding(const std::string& name, std::uint32_t slot)
{
    bindings_.push_back({ name, slot });
    ++numUniformBlockBindings_;
}

void GLShaderBindingLayout::AppendShaderStorageBinding(const std::string& name, std::uint32_t slot)
{
    bindings_.push_back({ name, slot });
    ++numShaderStorageBindings_;
}


} // /namespace LLGL



// ================================================================================
