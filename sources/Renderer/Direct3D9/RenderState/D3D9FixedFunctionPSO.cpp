/*
 * D3D9FixedFunctionPSO.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9FixedFunctionPSO.h"
#include "../Shader/D3D9VertexShader.h"
#include "../Shader/D3D9PixelShader.h"
#include "../../CheckedCast.h"


namespace LLGL
{


D3D9FixedFunctionPSO::D3D9FixedFunctionPSO(const GraphicsPipelineDescriptor& desc) :
    D3D9PipelineState { desc, false }
{
}

const Report* D3D9FixedFunctionPSO::GetReport() const
{
    return nullptr; //TODO
}


} // /namespace LLGL



// ================================================================================
