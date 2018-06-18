/*
 * MTShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_SHADER_PROGRAM_H
#define LLGL_MT_SHADER_PROGRAM_H


#import <Metal/Metal.h>

#include <LLGL/ShaderProgram.h>


namespace LLGL
{


class MTShaderProgram : public ShaderProgram
{

    public:

        MTShaderProgram();
        ~MTShaderProgram();

        void AttachShader(Shader& shader) override;
        void DetachAll() override;

        bool LinkShaders() override;

        std::string QueryInfoLog() override;

        ShaderReflectionDescriptor QueryReflectionDesc() const override;

        void BuildInputLayout(std::uint32_t numVertexFormats, const VertexFormat* vertexFormats) override;
        void BindConstantBuffer(const std::string& name, std::uint32_t bindingIndex) override;
        void BindStorageBuffer(const std::string& name, std::uint32_t bindingIndex) override;

        ShaderUniform* LockShaderUniform() override;
        void UnlockShaderUniform() override;

        // Returns the MTLVertexDescriptor object for this shader program.
        inline MTLVertexDescriptor* GetMTLVertexDesc() const
        {
            return vertexDesc_;
        }

    private:
    
        void ReleaseVertexDesc();

        MTLVertexDescriptor*    vertexDesc_     = nullptr;
        id<MTLFunction>         vertexFunc_     = nil;
        id<MTLFunction>         fragmentFunc_   = nil;
        id<MTLFunction>         kernelFunc_     = nil;

};


} // /namespace LLGL


#endif



// ================================================================================
