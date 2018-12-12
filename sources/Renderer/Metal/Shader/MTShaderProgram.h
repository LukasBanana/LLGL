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


class MTShader;

class MTShaderProgram : public ShaderProgram
{

    public:

        MTShaderProgram(id<MTLDevice> device, const ShaderProgramDescriptor& desc);
        ~MTShaderProgram();
    
        bool HasErrors() const override;

        std::string QueryInfoLog() override;

        ShaderReflectionDescriptor QueryReflectionDesc() const override;

        void BindConstantBuffer(const std::string& name, std::uint32_t bindingIndex) override;
        void BindStorageBuffer(const std::string& name, std::uint32_t bindingIndex) override;

        ShaderUniform* LockShaderUniform() override;
        void UnlockShaderUniform() override;

        // Returns the MTLVertexDescriptor object for this shader program.
        inline MTLVertexDescriptor* GetMTLVertexDesc() const
        {
            return vertexDesc_;
        }

        inline id<MTLFunction> GetVertexMTLFunction() const
        {
            return vertexFunc_;
        }
    
        inline id<MTLFunction> GetFragmentMTLFunction() const
        {
            return fragmentFunc_;
        }
        
        inline id<MTLFunction> GetKernelMTLFunction() const
        {
            return kernelFunc_;
        }
    
    private:
    
        void Attach(Shader* shader);
        void BuildInputLayout(std::size_t numVertexFormats, const VertexFormat* vertexFormats);
        void ReleaseVertexDesc();
    
        void ReflectRenderPipeline(ShaderReflectionDescriptor& reflection) const;
        void ReflectComputePipeline(ShaderReflectionDescriptor& reflection) const;

        id<MTLDevice>           device_             = nil;
    
        MTLVertexDescriptor*    vertexDesc_     	= nullptr;
        id<MTLFunction>         vertexFunc_         = nil;
        id<MTLFunction>         fragmentFunc_       = nil;
        id<MTLFunction>         kernelFunc_         = nil;
    
        MTShader*               shaderWithError_    = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
