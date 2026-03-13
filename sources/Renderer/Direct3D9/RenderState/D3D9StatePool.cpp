/*
 * D3D9StatePool.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9StatePool.h"
#include "D3D9StateManager.h"
#include "../../../Core/CoreUtils.h"
#include <functional>


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
void ReleaseRenderStateObject(std::vector<std::shared_ptr<T>>& container, std::shared_ptr<T>&& renderState)
{
    if (renderState && renderState.use_count() == 2)
    {
        /* Reset render state */
        T* objectRef = renderState.get();
        renderState.reset();

        /* Retrieve entry index in container to remove entry */
        std::size_t entryIndex = 0;
        if (FindCompatibleStateObject<T, T, T>(container, *objectRef, entryIndex) != nullptr)
            container.erase(container.begin() + entryIndex);
    }
}


/*
 * D3D9StatePool class
 */

D3D9StatePool& D3D9StatePool::Get()
{
    static D3D9StatePool instance;
    return instance;
}

void D3D9StatePool::Clear()
{
    depthStencilStates_.clear();
    rasterizerStates_.clear();
    blendStates_.clear();
}

/* ----- Depth-stencil states ----- */

D3D9DepthStencilStateSPtr D3D9StatePool::CreateDepthStencilState(const DepthDescriptor& depthDesc, const StencilDescriptor& stencilDesc)
{
    return CreateRenderStateObject(depthStencilStates_, depthDesc, stencilDesc);
}

void D3D9StatePool::ReleaseDepthStencilState(D3D9DepthStencilStateSPtr&& depthStencilState)
{
    ReleaseRenderStateObject<D3D9DepthStencilState>(depthStencilStates_, std::forward<D3D9DepthStencilStateSPtr>(depthStencilState));
}

/* ----- Rasterizer states ----- */

D3D9RasterizerStateSPtr D3D9StatePool::CreateRasterizerState(const RasterizerDescriptor& rasterizerDesc)
{
    return CreateRenderStateObject(rasterizerStates_, rasterizerDesc);
}

void D3D9StatePool::ReleaseRasterizerState(D3D9RasterizerStateSPtr&& rasterizerState)
{
    ReleaseRenderStateObject<D3D9RasterizerState>(rasterizerStates_, std::forward<D3D9RasterizerStateSPtr>(rasterizerState));
}

/* ----- Blend states ----- */

D3D9BlendStateSPtr D3D9StatePool::CreateBlendState(const BlendDescriptor& blendDesc)
{
    return CreateRenderStateObject(blendStates_, blendDesc);
}

void D3D9StatePool::ReleaseBlendState(D3D9BlendStateSPtr&& blendState)
{
    ReleaseRenderStateObject<D3D9BlendState>(blendStates_, std::forward<D3D9BlendStateSPtr>(blendState));
}


} // /namespace LLGL



// ================================================================================
