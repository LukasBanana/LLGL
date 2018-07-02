/*
 * DbgShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_SHADER_PROGRAM_H
#define LLGL_DBG_SHADER_PROGRAM_H


#include <LLGL/ShaderProgram.h>
#include <LLGL/ShaderFlags.h>
#include <LLGL/RenderingDebugger.h>
#include <vector>


namespace LLGL
{


class DbgShader;

class DbgShaderProgram : public ShaderProgram
{

    public:

        struct VertexLayout
        {
            std::vector<VertexAttribute>    attributes;
            bool                            bound       = false;
        };

        DbgShaderProgram(ShaderProgram& instance, RenderingDebugger* debugger, const GraphicsShaderProgramDescriptor& desc);
        DbgShaderProgram(ShaderProgram& instance, RenderingDebugger* debugger, const ComputeShaderProgramDescriptor& desc);

        bool HasErrors() const override;

        std::string QueryInfoLog() override;

        ShaderReflectionDescriptor QueryReflectionDesc() const override;

        void BindConstantBuffer(const std::string& name, std::uint32_t bindingIndex) override;
        void BindStorageBuffer(const std::string& name, std::uint32_t bindingIndex) override;

        ShaderUniform* LockShaderUniform() override;
        void UnlockShaderUniform() override;

        inline const VertexLayout& GetVertexLayout() const
        {
            return vertexLayout_;
        }

        ShaderProgram& instance;

    private:

        void DebugShaderAttachment(Shader* shader);
        void DebugShaderComposition();

        RenderingDebugger*      debugger_               = nullptr;
        int                     shaderAttachmentMask_   = 0;

        std::vector<ShaderType> shaderTypes_;
        VertexLayout            vertexLayout_;

};


} // /namespace LLGL


#endif



// ================================================================================
