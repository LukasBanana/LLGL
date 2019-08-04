/*
 * D3D12MipGenerator.cpp
 \*

 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12MipGenerator.h"
#include "../Shader/Builtin/D3D12Builtin.h"
#include "D3D12Texture.h"
#include "../D3DX12/d3dx12.h"
#include "../../DXCommon/DXCore.h"
#include "../../DXCommon/DXTypes.h"


namespace LLGL
{


D3D12MipGenerator::D3D12MipGenerator() :
    shaders_
    {
        { ShaderType::Compute, LLGL_IDR_GENERATEMIPS_CS            },
        { ShaderType::Compute, LLGL_IDR_GENERATEMIPS_CS_ODDX       },
        { ShaderType::Compute, LLGL_IDR_GENERATEMIPS_CS_ODDY       },
        { ShaderType::Compute, LLGL_IDR_GENERATEMIPS_CS_ODDXY      },
        { ShaderType::Compute, LLGL_IDR_GENERATEMIPS_CS_SRGB       },
        { ShaderType::Compute, LLGL_IDR_GENERATEMIPS_CS_SRGB_ODDX  },
        { ShaderType::Compute, LLGL_IDR_GENERATEMIPS_CS_SRGB_ODDY  },
        { ShaderType::Compute, LLGL_IDR_GENERATEMIPS_CS_SRGB_ODDXY },
    }
{
}

void D3D12MipGenerator::CreateRootSignature(ID3D12Device* device)
{
    /* Initialize linear sampler */
    linearSamplerDesc_.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    linearSamplerDesc_.SetTextureAddressModes(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

    /* Initialize root signature */
    D3D12RootSignature rootSignature;
    {
        rootSignature.ResetAndAlloc(3, 1);
        rootSignature[0].InitAsConstants(0, 4);
        rootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
        rootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 4);
        //rootSignature.InitStaticSampler();
    }
    rootSignature_ = rootSignature.Finalize(device);
}

void D3D12MipGenerator::GenerateMips2D(ID3D12GraphicsCommandList* commandList, D3D12Texture& texture) const
{
    commandList->SetComputeRootSignature(rootSignature_.Get());

    //TODO...
}


/*
 * ======= Private: =======
 */




} // /namespace LLGL



// ================================================================================
