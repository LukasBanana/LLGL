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
    /* Gather all uniform bindings */
    for (const auto& binding : pipelineLayout.GetBindings())
    {
        if (!binding.name.empty())
        {
            if (binding.type == ResourceType::Sampler || binding.type == ResourceType::Texture)
            {
                bindings_.push_back({ binding.name, binding.slot });
                ++numUniformBindings_;
            }
        }
    }

    /* Gather all uniform-block bindings */
    for (const auto& binding : pipelineLayout.GetBindings())
    {
        if (!binding.name.empty())
        {
            if (binding.type == ResourceType::Buffer && (binding.bindFlags & BindFlags::ConstantBuffer) != 0)
            {
                bindings_.push_back({ binding.name, binding.slot });
                ++numUniformBlockBindings_;
            }
        }
    }

    /* Gather all shader-storage bindings */
    for (const auto& binding : pipelineLayout.GetBindings())
    {
        if (!binding.name.empty())
        {
            if (binding.type == ResourceType::Buffer && (binding.bindFlags & (BindFlags::Storage | BindFlags::Sampled)) != 0)
            {
                bindings_.push_back({ binding.name, binding.slot });
                ++numShaderStorageBindings_;
            }
        }
    }
}

void GLShaderBindingLayout::UniformAndBlockBinding(GLuint program, GLStateManager* stateMngr) const
{
    std::size_t resourceIndex = 0;

    /* Set uniform bindings */
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
    else if (stateMngr != nullptr)
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


} // /namespace LLGL



// ================================================================================
