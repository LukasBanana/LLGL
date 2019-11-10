/*
 * MTShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_SHADER_PROGRAM_H
#define LLGL_MT_SHADER_PROGRAM_H


#import <Metal/Metal.h>

#include <LLGL/ShaderProgram.h>


namespace LLGL
{


class MTShader;

class MTShaderProgram final : public ShaderProgram
{

    public:

        MTShaderProgram(id<MTLDevice> device, const ShaderProgramDescriptor& desc);
        ~MTShaderProgram();

        bool HasErrors() const override;
        std::string GetReport() const override;

        bool Reflect(ShaderReflection& reflection) const override;
        UniformLocation FindUniformLocation(const char* name) const override;

    public:

        // Returns the number of patch control points or 0 if this shader program does not contain a post-tessellation vertex function.
        NSUInteger GetNumPatchControlPoints() const;

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

        // Returns the number of threads per thread-group for compute kernels.
        inline const MTLSize& GetNumThreadsPerGroup() const
        {
            return numThreadsPerGroup_;
        }

    private:

        void Attach(Shader* shader);
        void ReleaseVertexDesc();

        void ReflectRenderPipeline(ShaderReflection& reflection) const;
        void ReflectComputePipeline(ShaderReflection& reflection) const;

    private:

        id<MTLDevice>           device_             = nil;

        MTLVertexDescriptor*    vertexDesc_     	= nullptr;
        id<MTLFunction>         vertexFunc_         = nil;
        id<MTLFunction>         fragmentFunc_       = nil;
        id<MTLFunction>         kernelFunc_         = nil;

        MTShader*               shaderWithError_    = nullptr;
        MTLSize                 numThreadsPerGroup_ = { 1, 1, 1 };

};


} // /namespace LLGL


#endif



// ================================================================================
