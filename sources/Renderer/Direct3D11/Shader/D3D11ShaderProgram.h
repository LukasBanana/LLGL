/*
 * D3D11ShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_SHADER_PROGRAM_H
#define LLGL_D3D11_SHADER_PROGRAM_H


#include <LLGL/ShaderProgram.h>
#include "../../DXCommon/ComPtr.h"
#include <vector>
#include <d3d11.h>


namespace LLGL
{


class D3D11Shader;

class D3D11ShaderProgram final : public ShaderProgram
{

    public:

        bool HasErrors() const override;
        std::string GetReport() const override;

        bool Reflect(ShaderReflection& reflection) const override;
        UniformLocation FindUniformLocation(const char* name) const override;

    public:

        D3D11ShaderProgram(ID3D11Device* device, const ShaderProgramDescriptor& desc);

        ComPtr<ID3D11InputLayout> GetInputLayout() const;

        inline D3D11Shader* GetVS() const { return vs_; }
        inline D3D11Shader* GetHS() const { return hs_; }
        inline D3D11Shader* GetDS() const { return ds_; }
        inline D3D11Shader* GetGS() const { return gs_; }
        inline D3D11Shader* GetPS() const { return ps_; }
        inline D3D11Shader* GetCS() const { return cs_; }

    private:

        void LinkProgram();

    private:

        union
        {
            struct
            {
                D3D11Shader*    vs_;
                D3D11Shader*    hs_;
                D3D11Shader*    ds_;
                D3D11Shader*    gs_;
                D3D11Shader*    ps_;
                D3D11Shader*    cs_;
            };
            D3D11Shader*        shaders_[6]     = {};
        };

        LinkError               linkError_      = LinkError::NoError;

};


} // /namespace LLGL


#endif



// ================================================================================
