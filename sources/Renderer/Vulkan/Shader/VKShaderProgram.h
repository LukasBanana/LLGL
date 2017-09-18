/*
 * VKShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_SHADER_PROGRAM_H
#define LLGL_VK_SHADER_PROGRAM_H


#include <LLGL/ShaderProgram.h>
#include "../Vulkan.h"
#include "../VKPtr.h"


namespace LLGL
{


class VKShaderProgram : public ShaderProgram
{

    public:

        VKShaderProgram();
        ~VKShaderProgram();

        void AttachShader(Shader& shader) override;
        void DetachAll() override;

        bool LinkShaders() override;

        std::string QueryInfoLog() override;

        std::vector<VertexAttribute> QueryVertexAttributes() const override;
        std::vector<StreamOutputAttribute> QueryStreamOutputAttributes() const override;
        std::vector<ConstantBufferViewDescriptor> QueryConstantBuffers() const override;
        std::vector<StorageBufferViewDescriptor> QueryStorageBuffers() const override;
        std::vector<UniformDescriptor> QueryUniforms() const override;

        void BuildInputLayout(const VertexFormat& vertexFormat) override;
        void BindConstantBuffer(const std::string& name, unsigned int bindingIndex) override;
        void BindStorageBuffer(const std::string& name, unsigned int bindingIndex) override;

        ShaderUniform* LockShaderUniform() override;
        void UnlockShaderUniform() override;

    private:


};


} // /namespace LLGL


#endif



// ================================================================================
