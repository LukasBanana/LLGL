/*
 * D3D11ShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11ShaderProgram.h"
#include "D3D11Shader.h"
#include "../D3D11ObjectUtils.h"
#include "../../CheckedCast.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/Helper.h"
#include <LLGL/Log.h>
#include <LLGL/VertexAttribute.h>
#include <algorithm>
#include <stdexcept>


namespace LLGL
{


static void Attach(D3D11Shader*& shaderD3D, Shader* shader)
{
    if (shader != nullptr)
        shaderD3D = LLGL_CAST(D3D11Shader*, shader);
}

D3D11ShaderProgram::D3D11ShaderProgram(ID3D11Device* device, const ShaderProgramDescriptor& desc)
{
    Attach(vs_, desc.vertexShader);
    Attach(hs_, desc.tessControlShader);
    Attach(ds_, desc.tessEvaluationShader);
    Attach(gs_, desc.geometryShader);
    Attach(ps_, desc.fragmentShader);
    Attach(cs_, desc.computeShader);
    LinkProgram();
}

bool D3D11ShaderProgram::HasErrors() const
{
    return (linkError_ != LinkError::NoError);
}

std::string D3D11ShaderProgram::GetReport() const
{
    if (auto s = ShaderProgram::LinkErrorToString(linkError_))
        return s;
    else
        return "";
}

bool D3D11ShaderProgram::Reflect(ShaderReflection& reflection) const
{
    ShaderProgram::ClearShaderReflection(reflection);

    for (auto shader : shaders_)
    {
        if (shader != nullptr)
        {
            if (!shader->Reflect(reflection))
                return false;
        }
    }

    ShaderProgram::FinalizeShaderReflection(reflection);
    return true;
}

UniformLocation D3D11ShaderProgram::FindUniformLocation(const char* name) const
{
    return -1; // dummy
}

ComPtr<ID3D11InputLayout> D3D11ShaderProgram::GetInputLayout() const
{
    if (vs_ != nullptr)
        return vs_->GetInputLayout();
    else
        return nullptr;
}


/*
 * ======= Private: =======
 */

void D3D11ShaderProgram::LinkProgram()
{
    /* Validate shader composition */
    linkError_ = LinkError::NoError;

    /* Validate native shader objects */
    for (std::size_t i = 0; i < 6; ++i)
    {
        if (shaders_[i] != nullptr && shaders_[i]->GetNative().vs == nullptr)
            linkError_ = LinkError::InvalidByteCode;
    }

    /*
    Validate composition of attached shaders
    Note: reinterpret_cast allowed here, because no multiple inheritance is used, just plain pointers!
    */
    if (!ShaderProgram::ValidateShaderComposition(reinterpret_cast<Shader* const*>(shaders_), 6))
        linkError_ = LinkError::InvalidComposition;
}


} // /namespace LLGL



// ================================================================================
