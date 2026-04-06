/*
 * WGShaderModulePool.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGShaderModulePool.h"
#include "../../ShaderUtils.h"
#include "../../../Core/CoreUtils.h"


namespace LLGL
{


/*
 * Internal templates
 */

// Searches a compatible state object with complexity O(log n)
template <typename T, typename TCompare = T, typename TBase = T>
std::shared_ptr<T> FindCompatiblePooledObject(
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
std::shared_ptr<T> CreatePooledObject(std::vector<std::shared_ptr<T>>& container, Args&&... args)
{
    /* Try to find render state object with same parameter */
    T stateToCompare{ std::forward<Args>(args)... };

    std::size_t insertionIndex = 0;
    if (std::shared_ptr<T> sharedState = FindCompatiblePooledObject<T, T, T>(container, stateToCompare, insertionIndex))
        return sharedState;

    /* Allocate new render state object with insertion sort */
    std::shared_ptr<T> newState = std::make_shared<T>(stateToCompare);
    container.insert(container.begin() + insertionIndex, newState);

    return newState;
}

template <typename T>
void ReleasePooledObject(std::vector<std::shared_ptr<T>>& container, std::shared_ptr<T>&& object)
{
    if (object && object.use_count() == 2)
    {
        /* Reset render state */
        T* objectRef = object.get();
        object.reset();

        /* Retrieve entry index in container to remove entry */
        std::size_t entryIndex = 0;
        if (FindCompatiblePooledObject<T, T, T>(container, *objectRef, entryIndex) != nullptr)
            container.erase(container.begin() + entryIndex);
    }
}


/*
 * WGShaderModulePool class
 */

WGShaderModulePool& WGShaderModulePool::Get()
{
    static WGShaderModulePool instance;
    return instance;
}

void WGShaderModulePool::Clear()
{
    shaderModules_.clear();
}

WGShaderModuleSPtr WGShaderModulePool::CreateShaderModule(WGPUInstance instance, WGPUDevice device, const ShaderSourceContext& sourceContext)
{
    /* Try to find existing shader module with given source */
    SourceBlob sourceBlob = ToSourceBlob(sourceContext.sourceText);
    if (ShaderModuleSourcePair* it = FindShaderModuleWithSource(sourceBlob))
        return it->shaderModule;

    /* Create new shader module */
    WGShaderModuleSPtr newShaderModule = std::make_shared<WGShaderModule>(instance, device, sourceContext);
    shaderModules_.emplace_back(ShaderModuleSourcePair{ std::move(sourceBlob), newShaderModule });
    return newShaderModule;
}

void WGShaderModulePool::ReleaseShaderModule(WGShaderModuleSPtr&& shaderModule)
{
    if (shaderModule.use_count() == 2)
    {
        for (auto it = shaderModules_.begin(); it != shaderModules_.end(); ++it)
        {
            if (it->shaderModule.get() == shaderModule.get())
            {
                shaderModules_.erase(it);
                break;
            }
        }
    }
}


/*
 * ======= Private: =======
 */

WGShaderModulePool::ShaderModuleSourcePair* WGShaderModulePool::FindShaderModuleWithSource(const SourceBlob& sourceBlob, std::size_t* outIndex)
{
    return FindInSortedArray<ShaderModuleSourcePair>(
        shaderModules_.data(),
        shaderModules_.size(),
        [&sourceBlob](const ShaderModuleSourcePair& entry) -> int
        {
            if (entry.sourceBlob.size() < sourceBlob.size())
                return -1;
            if (entry.sourceBlob.size() > sourceBlob.size())
                return -1;
            return ::memcmp(entry.sourceBlob.data(), sourceBlob.data(), sourceBlob.size());
        },
        outIndex
    );
}


} // /namespace LLGL



// ================================================================================
