/*
 * D3D9ProgrammablePSO.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_PROGRAMMABLE_PSO_H
#define LLGL_D3D9_PROGRAMMABLE_PSO_H


#include "D3D9PipelineState.h"
#include "../Direct3D9.h"


namespace LLGL
{


class D3D9ProgrammablePSO final : public D3D9PipelineState
{

    public:

        const Report* GetReport() const override;

    public:

        D3D9ProgrammablePSO(const GraphicsPipelineDescriptor& desc);

        inline IDirect3DVertexShader9* GetVertexShader() const
        {
            return d3dVertexShader_.Get();
        }

        inline IDirect3DPixelShader9* GetPixelShader() const
        {
            return d3dPixelShader_.Get();
        }

    private:

        ComPtr<IDirect3DVertexShader9>  d3dVertexShader_;
        ComPtr<IDirect3DPixelShader9>   d3dPixelShader_;

};


} // /namespace LLGL


#endif



// ================================================================================
