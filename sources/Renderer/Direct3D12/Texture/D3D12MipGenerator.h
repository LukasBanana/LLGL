/*
 * D3D12MipGenerator.h
 \*

 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_MIP_GENERATOR_H
#define LLGL_D3D12_MIP_GENERATOR_H


#include <LLGL/Texture.h>
#include <d3d12.h>
#include "../Shader/D3D12RootSignature.h"
#include "D3D12SamplerDesc.h"
#include "../Shader/D3D12Shader.h"
#include "../../DXCommon/ComPtr.h"


namespace LLGL
{


class D3D12Texture;

class D3D12MipGenerator
{

    public:

        D3D12MipGenerator();

        void CreateRootSignature(ID3D12Device* device);

        void GenerateMips2D(ID3D12GraphicsCommandList* commandList, D3D12Texture& texture) const;

    private:

        D3D12SamplerDesc            linearSamplerDesc_;
        ComPtr<ID3D12RootSignature> rootSignature_;
        D3D12Shader                 shaders_[8];

};


} // /namespace LLGL


#endif



// ================================================================================
