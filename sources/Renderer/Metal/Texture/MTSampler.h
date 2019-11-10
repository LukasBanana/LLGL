/*
 * MTSampler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_SAMPLER_H
#define LLGL_MT_SAMPLER_H


#import <Metal/Metal.h>

#include <LLGL/Sampler.h>


namespace LLGL
{


class MTSampler final : public Sampler
{

    public:

        MTSampler(id<MTLDevice> device, const SamplerDescriptor& desc);
        ~MTSampler();
    
        // Returns the native MTLSamplerState object.
        inline id<MTLSamplerState> GetNative() const
        {
            return native_;
        }

    private:

        id<MTLSamplerState> native_;

};


} // /namespace LLGL


#endif



// ================================================================================
