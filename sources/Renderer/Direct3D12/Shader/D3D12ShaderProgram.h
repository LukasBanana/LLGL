/*
 * D3D12ShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_SHADER_PROGRAM_H
#define LLGL_D3D12_SHADER_PROGRAM_H


#include <LLGL/ShaderProgram.h>
#include "../../DXCommon/ComPtr.h"
#include "../../../Core/LinearStringContainer.h"
#include <vector>
#include <d3d12.h>


namespace LLGL
{


class D3D12Shader;

class D3D12ShaderProgram final : public ShaderProgram
{

    public:

        D3D12ShaderProgram(const ShaderProgramDescriptor& desc);

        bool HasErrors() const override;

        std::string QueryInfoLog() override;

        ShaderReflection QueryReflection() const override;
        UniformLocation QueryUniformLocation(const char* name) const override;

        bool SetWorkGroupSize(const Extent3D& workGroupSize) override;
        bool GetWorkGroupSize(Extent3D& workGroupSize) const override;

    public:

        D3D12_INPUT_LAYOUT_DESC GetInputLayoutDesc() const;

        inline D3D12Shader* GetVS() const { return vs_; }
        inline D3D12Shader* GetPS() const { return ps_; }
        inline D3D12Shader* GetHS() const { return hs_; }
        inline D3D12Shader* GetDS() const { return ds_; }
        inline D3D12Shader* GetGS() const { return gs_; }
        inline D3D12Shader* GetCS() const { return cs_; }

    private:

        void BuildInputLayout(std::size_t numVertexFormats, const VertexFormat* vertexFormats);
        void Link();

    private:

        std::vector<D3D12_INPUT_ELEMENT_DESC>   inputElements_;
        LinearStringContainer                   inputElementNames_; // custom string container to hold valid string pointers.

        union
        {
            struct
            {
                D3D12Shader*                    vs_;
                D3D12Shader*                    hs_;
                D3D12Shader*                    ds_;
                D3D12Shader*                    gs_;
                D3D12Shader*                    ps_;
                D3D12Shader*                    cs_;
            };
            D3D12Shader*                        shaders_[6] = {};
        };

        LinkError                               linkError_  = LinkError::NoError;

};


} // /namespace LLGL


#endif



// ================================================================================
