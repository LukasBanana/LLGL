/*
 * GLShaderBindingLayout.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLShaderBindingLayout.h"
#include "../../../Core/HelperMacros.h"
#include "../../GLCommon/GLExtensionRegistry.h"
#include "../Ext/GLExtensions.h"


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
            if (binding.type == ResourceType::Buffer && (binding.bindFlags & (BindFlags::RWStorageBuffer | BindFlags::SampleBuffer)) != 0)
            {
                bindings_.push_back({ binding.name, binding.slot });
                ++numShaderStorageBindings_;
            }
        }
    }
}

void GLShaderBindingLayout::BindResourceSlots(GLuint program) const
{
    std::size_t resourceIndex = 0;

    /* Set uniform bindings */
    for (std::uint8_t i = 0; i < numUniformBindings_; ++i)
    {
        const auto& resource = bindings_[resourceIndex++];
        auto blockIndex = glGetUniformLocation(program, resource.name.c_str());
        if (blockIndex != GL_INVALID_INDEX)
            glUniform1i(blockIndex, static_cast<GLint>(resource.slot));
    }

    /* Set uniform-block bindings */
    for (std::uint8_t i = 0; i < numUniformBlockBindings_; ++i)
    {
        const auto& resource = bindings_[resourceIndex++];
        auto blockIndex = glGetUniformBlockIndex(program, resource.name.c_str());
        if (blockIndex != GL_INVALID_INDEX)
            glUniformBlockBinding(program, blockIndex, resource.slot);
    }

    /* Set shader-storage bindings */
    #ifndef __APPLE__
    for (std::uint8_t i = 0; i < numShaderStorageBindings_; ++i)
    {
        const auto& resource = bindings_[resourceIndex++];
        auto blockIndex = glGetProgramResourceIndex(program, GL_SHADER_STORAGE_BLOCK, resource.name.c_str());
        if (blockIndex != GL_INVALID_INDEX)
            glShaderStorageBlockBinding(program, blockIndex, resource.slot);
    }
    #endif
}

int GLShaderBindingLayout::CompareSWO(const GLShaderBindingLayout& rhs) const
{
    const auto& lhs = *this;

    for (std::size_t i = 0, n = bindings_.size(); i < n; ++i)
    {
        LLGL_COMPARE_MEMBER_SWO( bindings_[i].slot );
        LLGL_COMPARE_MEMBER_SWO( bindings_[i].name );
    }

    return 0;
}

bool GLShaderBindingLayout::HasBindings() const
{
    return ((numUniformBindings_ | numUniformBlockBindings_ | numShaderStorageBindings_) != 0);
}


} // /namespace LLGL



// ================================================================================
