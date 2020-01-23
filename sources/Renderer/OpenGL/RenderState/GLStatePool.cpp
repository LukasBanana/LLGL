/*
 * GLStatePool.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLStatePool.h"
#include "GLStateManager.h"
#include "../../../Core/ContainerUtils.h"
#include <functional>


namespace LLGL
{


/*
 * Internal templates
 */

// Searches a compatible state object with complexity O(log n)
template <typename T>
std::shared_ptr<T> FindCompatibleStateObject(
    std::vector<std::shared_ptr<T>>&    container,
    const T&                            compareObject,
    std::size_t&                        index)
{
    auto* entry = Utils::FindInSortedArray<std::shared_ptr<T>>(
        container.data(),
        container.size(),
        [&compareObject](const std::shared_ptr<T>& rhs)
        {
            return compareObject.CompareSWO(*rhs.get());
        },
        &index
    );
    return (entry != nullptr ? *entry : nullptr);
}

template <typename T, typename... Args>
std::shared_ptr<T> CreateRenderStateObject(std::vector<std::shared_ptr<T>>& container, Args&&... args)
{
    /* Try to find render state object with same parameter */
    T stateToCompare{ std::forward<Args>(args)... };

    std::size_t insertionIndex = 0;
    if (auto sharedState = FindCompatibleStateObject(container, stateToCompare, insertionIndex))
        return sharedState;

    /* Allocate new render state object with insertion sort */
    auto newState = std::make_shared<T>(stateToCompare);
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
        auto objectRef = renderState.get();
        renderState.reset();

        /* Retrieve entry index in container to remove entry */
        std::size_t entryIndex = 0;
        if (FindCompatibleStateObject(container, *objectRef, entryIndex) != nullptr)
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
}

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


} // /namespace LLGL



// ================================================================================
