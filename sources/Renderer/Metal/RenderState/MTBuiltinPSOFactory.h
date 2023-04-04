/*
 * MTBuiltinPSOFactory.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_BUILTIN_PSO_FACTORY_H
#define LLGL_MT_BUILTIN_PSO_FACTORY_H


#import <Metal/Metal.h>


namespace LLGL
{


// Enumeration of all builtin Metal compute PSOs.
enum class MTBuiltinComputePSO
{
    FillBufferByte4 = 0,
    Num
};

// Builtin Metal PSO factory singleton.
class MTBuiltinPSOFactory
{

    public:

        MTBuiltinPSOFactory(const MTBuiltinPSOFactory&) = delete;
        MTBuiltinPSOFactory& operator = (const MTBuiltinPSOFactory&) = delete;

        // Returns the instance of this singleton.
        static MTBuiltinPSOFactory& Get();

        // Loads all builtin shaders and creates the respective pipeline state objects (PSO).
        void CreateBuiltinPSOs(id<MTLDevice> device);

        // Returns the specified builtin compute PSO.
        id<MTLComputePipelineState> GetComputePSO(const MTBuiltinComputePSO builtin) const;

    private:

        MTBuiltinPSOFactory() = default;

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
