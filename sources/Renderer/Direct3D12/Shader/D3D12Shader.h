/*
 * D3D12Shader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D12_SHADER_H__
#define __LLGL_D3D12_SHADER_H__


#include <LLGL/Shader.h>
#include <vector>
#include <d3d12.h>


namespace LLGL
{


class D3D12Shader : public Shader
{

    public:

        D3D12Shader(const D3D12Shader&) = delete;
        D3D12Shader& operator = (const D3D12Shader&) = delete;

        D3D12Shader(const ShaderType type);

        bool Compile(const std::string& shaderSource) override;

        std::string QueryInfoLog() override;

        D3D12_SHADER_BYTECODE GetByteCode() const;

    private:

        std::vector<char> byteCode_;

};


} // /namespace LLGL


#endif



// ================================================================================
