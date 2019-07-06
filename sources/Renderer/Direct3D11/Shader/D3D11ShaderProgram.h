/*
 * D3D11ShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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

        D3D11ShaderProgram(ID3D11Device* device, const ShaderProgramDescriptor& desc);

        bool HasErrors() const override;

        std::string QueryInfoLog() override;

        ShaderReflectionDescriptor QueryReflectionDesc() const override;
        UniformLocation QueryUniformLocation(const char* name) const override;

        void BindConstantBuffer(const std::string& name, std::uint32_t bindingIndex) override;
        void BindStorageBuffer(const std::string& name, std::uint32_t bindingIndex) override;

        ShaderUniform* LockShaderUniform() override;
        void UnlockShaderUniform() override;

        bool SetWorkGroupSize(const Extent3D& workGroupSize) override;
        bool GetWorkGroupSize(Extent3D& workGroupSize) const override;

    public:

        inline const ComPtr<ID3D11InputLayout>& GetInputLayout() const
        {
            return inputLayout_;
        }

        inline D3D11Shader* GetVS() const { return vs_; }
        inline D3D11Shader* GetHS() const { return hs_; }
        inline D3D11Shader* GetDS() const { return ds_; }
        inline D3D11Shader* GetGS() const { return gs_; }
        inline D3D11Shader* GetPS() const { return ps_; }
        inline D3D11Shader* GetCS() const { return cs_; }

    private:

        void BuildInputLayout(ID3D11Device* device, std::size_t numVertexFormats, const VertexFormat* vertexFormats);
        void Link();

    private:

        ComPtr<ID3D11InputLayout>   inputLayout_;

        union
        {
            struct
            {
                D3D11Shader*        vs_;
                D3D11Shader*        hs_;
                D3D11Shader*        ds_;
                D3D11Shader*        gs_;
                D3D11Shader*        ps_;
                D3D11Shader*        cs_;
            };
            D3D11Shader*            shaders_[6]     = {};
        };

        LinkError                   linkError_      = LinkError::NoError;

};


} // /namespace LLGL


#endif



// ================================================================================
