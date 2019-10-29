/*
 * MTBuiltinShaderPool.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_BUILTIN_SHADER_POOL_H
#define LLGL_MT_BUILTIN_SHADER_POOL_H


#import <Metal/Metal.h>


namespace LLGL
{


// Enumeration of all builtin Metal compute PSOs.
enum class MTBuiltinComputePSO
{
    FillBufferByte4 = 0,
    Num
};

// Builtin Metal shader pool singleton.
class MTBuiltinShaderPool
{

    public:

        MTBuiltinShaderPool(const MTBuiltinShaderPool&) = delete;
        MTBuiltinShaderPool& operator = (const MTBuiltinShaderPool&) = delete;

        // Returns the instance of this singleton.
        static MTBuiltinShaderPool& Get();

        // Loads all builtin shaders and creates the respective pipeline state objects (PSO).
        void LoadBuiltinShaders(id<MTLDevice> device);

        // Returns the specified builtin compute PSO.
        id<MTLComputePipelineState> GetComputePSO(const MTBuiltinComputePSO builtin) const;

    private:

        MTBuiltinShaderPool() = default;

        void LoadBuiltinComputePSO(
            id<MTLDevice>               device,
            const MTBuiltinComputePSO   builtin,
            const char*                 kernelFunc,
            std::size_t                 kernelFuncSize
        );

    private:

        static const std::size_t g_numComputePSOs = static_cast<std::size_t>(MTBuiltinComputePSO::Num);

        id<MTLComputePipelineState> builtinComputePSOs_[g_numComputePSOs];

};


} // /namespace LLGL


#endif



// ================================================================================
