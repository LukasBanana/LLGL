/*
 * D3D12PSOUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_PSO_UTILS_H
#define LLGL_D3D12_PSO_UTILS_H

#include <d3d12.h>

namespace LLGL
{


template <D3D12_PIPELINE_STATE_SUBOBJECT_TYPE TType, typename TObject>
struct alignas(void *) D3DPipelineStreamSubobject
{
    D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = {TType};
    TObject object = {};

    D3DPipelineStreamSubobject() = default;

    inline D3DPipelineStreamSubobject(TObject object) : object{std::move(object)}
    {
    }

    inline D3DPipelineStreamSubobject &operator=(TObject object)
    {
        this->object = std::move(object);
        return *this;
    }

    inline operator TObject &()
    {
        return object;
    }

    inline TObject &operator*()
    {
        return object;
    }

    inline TObject *operator->()
    {
        return &object;
    }
};

} // /namespace LLGL

#endif

// ================================================================================