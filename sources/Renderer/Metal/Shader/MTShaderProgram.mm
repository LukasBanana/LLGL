/*
 * MTShaderProgram.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTShaderProgram.h"
#include "MTShader.h"
#include "../../CheckedCast.h"
#include <LLGL/Platform/Platform.h>


namespace LLGL
{


MTShaderProgram::MTShaderProgram(id<MTLDevice> device, const ShaderProgramDescriptor& desc) :
    device_ { device }
{
    /* Only consider vertex, fragment and compute shaders for Metal */
    Attach(desc.vertexShader);
    Attach(desc.fragmentShader);
    Attach(desc.computeShader);
}

MTShaderProgram::~MTShaderProgram()
{
    if (vertexDesc_)
        [vertexDesc_ release];
}

bool MTShaderProgram::HasErrors() const
{
    return (shaderWithError_ != nullptr);
}

std::string MTShaderProgram::GetReport() const
{
    if (shaderWithError_ != nullptr)
        return shaderWithError_->GetReport();
    else
        return "";
}

bool MTShaderProgram::Reflect(ShaderReflection& reflection) const
{
    ShaderProgram::ClearShaderReflection(reflection);

    if (vertexFunc_ != nil || fragmentFunc_ != nil)
        ReflectRenderPipeline(reflection);
    if (kernelFunc_ != nil)
        ReflectComputePipeline(reflection);

    ShaderProgram::FinalizeShaderReflection(reflection);
    return true;
}

UniformLocation MTShaderProgram::FindUniformLocation(const char* /*name*/) const
{
    return -1; // dummy
}

NSUInteger MTShaderProgram::GetNumPatchControlPoints() const
{
    if (vertexFunc_ != nil && [vertexFunc_ patchType] != MTLPatchTypeNone)
        return [vertexFunc_ patchControlPointCount];
    else
        return 0;
}


/*
 * ======= Private: =======
 */

void MTShaderProgram::Attach(Shader* shader)
{
    if (shader != nullptr)
    {
        /* Get native shader function */
        auto* shaderMT = LLGL_CAST(MTShader*, shader);
        id<MTLFunction> shaderFunc = shaderMT->GetNative();

        /* Check for errors */
        if (shaderMT->HasErrors())
        {
            /* Store reference to shader with error (for later info log queries) */
            shaderWithError_ = shaderMT;
        }
        else
        {
            /* Store reference to shader function */
            switch (shaderMT->GetType())
            {
                case ShaderType::Vertex:
                    /* Grab vertex descriptor and increment reference counter */
                    vertexFunc_ = shaderFunc;
                    if (auto vertexDesc = shaderMT->GetMTLVertexDesc())
                    {
                        [vertexDesc retain];
                        vertexDesc_ = vertexDesc;
                    }
                    break;

                case ShaderType::TessControl:
                case ShaderType::TessEvaluation:
                case ShaderType::Geometry:
                    // Ignore, shader stages not supported by Metal
                    break;

                case ShaderType::Fragment:
                    fragmentFunc_ = shaderFunc;
                    break;

                case ShaderType::Compute:
                    kernelFunc_         = shaderFunc;
                    numThreadsPerGroup_ = shaderMT->GetNumThreadsPerGroup();
                    break;

                default:
                    break;
            }
        }
    }
}

static ResourceType ToResourceType(MTLArgumentType type)
{
    switch (type)
    {
        case MTLArgumentTypeBuffer:     return ResourceType::Buffer;
        case MTLArgumentTypeTexture:    return ResourceType::Texture;
        case MTLArgumentTypeSampler:    return ResourceType::Sampler;
        default:                        return ResourceType::Undefined;
    }
}

static long GetShaderArgumentBindFlags(MTLArgument* arg, const ResourceType resourceType)
{
    long bindFlags = 0;

    bool isRead     = (arg.access != MTLArgumentAccessWriteOnly);
    bool isWritten  = (arg.access != MTLArgumentAccessReadOnly);

    if (resourceType == ResourceType::Buffer || resourceType == ResourceType::Texture)
    {
        if (isRead)
            bindFlags |= BindFlags::Sampled;
        if (isWritten)
            bindFlags |= BindFlags::Storage;
    }

    return bindFlags;
}

static void ReflectShaderArgument(MTLArgument* arg, ShaderReflection& reflection, long stageFlags)
{
    auto resourceType = ToResourceType(arg.type);
    if (resourceType != ResourceType::Undefined)
    {
        ShaderResource resource;
        {
            resource.binding.name       = [arg.name UTF8String];
            resource.binding.type       = resourceType;
            resource.binding.bindFlags  = GetShaderArgumentBindFlags(arg, resourceType);
            resource.binding.stageFlags = stageFlags;
            resource.binding.slot       = static_cast<std::uint32_t>(arg.index);
            resource.binding.arraySize  = static_cast<std::uint32_t>(arg.arrayLength);
            if (resourceType == ResourceType::Buffer)
                resource.constantBufferSize = static_cast<std::uint32_t>(arg.bufferDataSize);
        }
        reflection.resources.push_back(resource);
    }
}

void MTShaderProgram::ReflectRenderPipeline(ShaderReflection& reflection) const
{
    //TODO: get pixel formats from render target or render context
    /* Create temporary render pipeline state to retrieve shader reflection */
    MTLRenderPipelineDescriptor* pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
    {
        pipelineDesc.vertexDescriptor                   = GetMTLVertexDesc();
        pipelineDesc.alphaToCoverageEnabled             = NO;
        pipelineDesc.alphaToOneEnabled                  = NO;
        pipelineDesc.fragmentFunction                   = GetFragmentMTLFunction();
        pipelineDesc.vertexFunction                     = GetVertexMTLFunction();
        #ifndef LLGL_OS_IOS
        pipelineDesc.inputPrimitiveTopology             = MTLPrimitiveTopologyClassTriangle;
        #endif // /LLGL_OS_IOS
        pipelineDesc.colorAttachments[0].pixelFormat    = MTLPixelFormatBGRA8Unorm;
        pipelineDesc.depthAttachmentPixelFormat         = MTLPixelFormatDepth32Float_Stencil8;
        pipelineDesc.stencilAttachmentPixelFormat       = MTLPixelFormatDepth32Float_Stencil8;
        pipelineDesc.rasterizationEnabled               = (GetFragmentMTLFunction() != nil);
        pipelineDesc.sampleCount                        = 1;
    }

    MTLRenderPipelineReflection* psoReflect = nullptr;
    MTLPipelineOption opt = (MTLPipelineOptionArgumentInfo | MTLPipelineOptionBufferTypeInfo);
    NSError* error = nullptr;

    id<MTLRenderPipelineState> pso = [device_
        newRenderPipelineStateWithDescriptor:   pipelineDesc
        options:                                opt
        reflection:                             &psoReflect
        error:                                  &error
    ];

    [pipelineDesc release];

    for (MTLArgument* arg in psoReflect.vertexArguments)
        ReflectShaderArgument(arg, reflection, StageFlags::VertexStage);
    for (MTLArgument* arg in psoReflect.fragmentArguments)
        ReflectShaderArgument(arg, reflection, StageFlags::FragmentStage);

    [pso release];
}

void MTShaderProgram::ReflectComputePipeline(ShaderReflection& reflection) const
{
    /* Create temporary compute pipeline state to retrieve shader reflection */
    MTLComputePipelineReflection* psoReflect = nullptr;
    MTLPipelineOption opt = (MTLPipelineOptionArgumentInfo | MTLPipelineOptionBufferTypeInfo);
    NSError* error = nullptr;

    id<MTLComputePipelineState> pso = [device_
        newComputePipelineStateWithFunction:    GetKernelMTLFunction()
        options:                                opt
        reflection:                             &psoReflect
        error:                                  &error
    ];

    for (MTLArgument* arg in psoReflect.arguments)
        ReflectShaderArgument(arg, reflection, StageFlags::ComputeStage);

    [pso release];
}


} // /namespace LLGL



// ================================================================================
