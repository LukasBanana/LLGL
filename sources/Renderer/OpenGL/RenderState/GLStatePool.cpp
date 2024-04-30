/*
 * GLStatePool.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLStatePool.h"
#include "GLStateManager.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../../CheckedCast.h"
#include "../../../Core/CoreUtils.h"
#include <functional>

#include "../Shader/GLLegacyShader.h"
#include "../Shader/GLShaderProgram.h"

#ifdef LLGL_OPENGL
#   include "../Shader/GLSeparableShader.h"
#   include "../Shader/GLProgramPipeline.h"
#endif


namespace LLGL
{


/*
 * Internal templates
 */

// Searches a compatible state object with complexity O(log n)
template <typename T, typename TCompare = T, typename TBase = T>
std::shared_ptr<T> FindCompatibleStateObject(
    std::vector<std::shared_ptr<TBase>>&    container,
    const TCompare&                         compareObject,
    std::size_t&                            index)
{
    std::shared_ptr<TBase>* entry = FindInSortedArray<std::shared_ptr<TBase>>(
        container.data(),
        container.size(),
        [&compareObject](const std::shared_ptr<TBase>& entry) -> int
        {
            return T::CompareSWO(*entry.get(), compareObject);
        },
        &index
    );
    return std::static_pointer_cast<T>(entry != nullptr ? *entry : nullptr);
}

template <typename T, typename TCompare, typename TBase, typename... Args>
std::shared_ptr<T> CreateRenderStateObjectExt(std::vector<std::shared_ptr<TBase>>& container, Args&&... args)
{
    /* Try to find render state object with same parameter */
    const TCompare stateToCompare{ std::forward<Args>(args)... };

    std::size_t insertionIndex = 0;
    if (std::shared_ptr<T> sharedState = FindCompatibleStateObject<T, TCompare, TBase>(container, stateToCompare, insertionIndex))
        return sharedState;

    /* Allocate new render state object with insertion sort */
    std::shared_ptr<T> newState = std::make_shared<T>(std::forward<Args>(args)...);
    container.insert(container.begin() + insertionIndex, newState);

    return newState;
}

template <typename T, typename... Args>
std::shared_ptr<T> CreateRenderStateObject(std::vector<std::shared_ptr<T>>& container, Args&&... args)
{
    /* Try to find render state object with same parameter */
    T stateToCompare{ std::forward<Args>(args)... };

    std::size_t insertionIndex = 0;
    if (std::shared_ptr<T> sharedState = FindCompatibleStateObject<T, T, T>(container, stateToCompare, insertionIndex))
        return sharedState;

    /* Allocate new render state object with insertion sort */
    std::shared_ptr<T> newState = std::make_shared<T>(stateToCompare);
    container.insert(container.begin() + insertionIndex, newState);

    return newState;
}

template <typename T>
void ReleaseRenderStateObject(
    std::vector<std::shared_ptr<T>>&    container,
    const std::function<void(T*)>&      callback,
    std::shared_ptr<T>&&                renderState)
{
    if (renderState && renderState.use_count() == 2)
    {
        /* Reset render state */
        T* objectRef = renderState.get();
        renderState.reset();

        /* Retrieve entry index in container to remove entry */
        std::size_t entryIndex = 0;
        if (FindCompatibleStateObject<T, T, T>(container, *objectRef, entryIndex) != nullptr)
        {
            /* Notify via callback and erase from container */
            if (callback)
                callback(objectRef);
            container.erase(container.begin() + entryIndex);
        }
    }
}


/*
 * GLStatePool class
 */

GLStatePool& GLStatePool::Get()
{
    static GLStatePool instance;
    return instance;
}

void GLStatePool::Clear()
{
    depthStencilStates_.clear();
    rasterizerStates_.clear();
    blendStates_.clear();
    shaderBindingLayouts_.clear();
    shaderPipelines_.clear();
}

/* ----- Depth-stencil states ----- */

GLDepthStencilStateSPtr GLStatePool::CreateDepthStencilState(const DepthDescriptor& depthDesc, const StencilDescriptor& stencilDesc)
{
    return CreateRenderStateObject(depthStencilStates_, depthDesc, stencilDesc);
}

void GLStatePool::ReleaseDepthStencilState(GLDepthStencilStateSPtr&& depthStencilState)
{
    ReleaseRenderStateObject<GLDepthStencilState>(
        depthStencilStates_,
        std::bind(&GLStateManager::NotifyDepthStencilStateRelease, &(GLStateManager::Get()), std::placeholders::_1),
        std::forward<GLDepthStencilStateSPtr>(depthStencilState)
    );
}

/* ----- Rasterizer states ----- */

GLRasterizerStateSPtr GLStatePool::CreateRasterizerState(const RasterizerDescriptor& rasterizerDesc)
{
    return CreateRenderStateObject(rasterizerStates_, rasterizerDesc);
}

void GLStatePool::ReleaseRasterizerState(GLRasterizerStateSPtr&& rasterizerState)
{
    ReleaseRenderStateObject<GLRasterizerState>(
        rasterizerStates_,
        std::bind(&GLStateManager::NotifyRasterizerStateRelease, &(GLStateManager::Get()), std::placeholders::_1),
        std::forward<GLRasterizerStateSPtr>(rasterizerState)
    );
}

/* ----- Blend states ----- */

GLBlendStateSPtr GLStatePool::CreateBlendState(const BlendDescriptor& blendDesc, std::uint32_t numColorAttachments)
{
    return CreateRenderStateObject(blendStates_, blendDesc, numColorAttachments);
}

void GLStatePool::ReleaseBlendState(GLBlendStateSPtr&& blendState)
{
    ReleaseRenderStateObject<GLBlendState>(
        blendStates_,
        std::bind(&GLStateManager::NotifyBlendStateRelease, &(GLStateManager::Get()), std::placeholders::_1),
        std::forward<GLBlendStateSPtr>(blendState)
    );
}

/* ----- Shader binding layouts ----- */

GLShaderBindingLayoutSPtr GLStatePool::CreateShaderBindingLayout(const GLPipelineLayout& pipelineLayout)
{
    return CreateRenderStateObject(shaderBindingLayouts_, pipelineLayout);
}

void GLStatePool::ReleaseShaderBindingLayout(GLShaderBindingLayoutSPtr&& shaderBindingLayout)
{
    ReleaseRenderStateObject<GLShaderBindingLayout>(
        shaderBindingLayouts_,
        nullptr,
        std::forward<GLShaderBindingLayoutSPtr>(shaderBindingLayout)
    );
}

/* ----- Shader pipelines ----- */

static bool IsGLSeparableShader(const Shader* shader)
{
    if (shader != nullptr)
    {
        auto shaderGL = LLGL_CAST(const GLShader*, shader);
        return shaderGL->IsSeparable();
    }
    return false;
}

// Returns true if the specified list of shaders has separable shaders.
// Actually all shaders must be of the same type, but full validation is handled in the debug layer.
static bool HasGLSeparableShaders(std::size_t numShaders, Shader* const* shaders)
{
    return (numShaders > 0 && IsGLSeparableShader(shaders[0]));
}

GLShaderPipelineSPtr GLStatePool::CreateShaderPipeline(
    std::size_t             numShaders,
    Shader* const*          shaders,
    GLShader::Permutation   permutation,
    GLPipelineCache*        pipelineCache)
{
    #ifdef LLGL_OPENGL
    if (HasExtension(GLExt::ARB_separate_shader_objects) && HasGLSeparableShaders(numShaders, shaders))
    {
        return std::static_pointer_cast<GLShaderPipeline>(
            CreateRenderStateObjectExt<GLProgramPipeline, GLPipelineSignature>(shaderPipelines_, numShaders, shaders, permutation)
        );
    }
    else
    #endif
    {
        return std::static_pointer_cast<GLShaderPipeline>(
            CreateRenderStateObjectExt<GLShaderProgram, GLPipelineSignature>(shaderPipelines_, numShaders, shaders, permutation, pipelineCache)
        );
    }
}

void GLStatePool::ReleaseShaderPipeline(GLShaderPipelineSPtr&& shaderPipeline)
{
    ReleaseRenderStateObject<GLShaderPipeline>(
        shaderPipelines_,
        nullptr,
        std::forward<GLShaderPipelineSPtr>(shaderPipeline)
    );
}


} // /namespace LLGL



// ================================================================================
