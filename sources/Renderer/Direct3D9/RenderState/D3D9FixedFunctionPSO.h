/*
 * D3D9FixedFunctionPSO.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_FIXED_FUNCTION_PSO_H
#define LLGL_D3D9_FIXED_FUNCTION_PSO_H


#include "D3D9PipelineState.h"
#include "../Direct3D9.h"
#include "../../DXCommon/ComPtr.h"


namespace LLGL
{


class D3D9FixedFunctionPSO final : public D3D9PipelineState
{

    public:

        const Report* GetReport() const override;

    public:

        D3D9FixedFunctionPSO(const GraphicsPipelineDescriptor& desc);

};


} // /namespace LLGL


#endif



// ================================================================================
