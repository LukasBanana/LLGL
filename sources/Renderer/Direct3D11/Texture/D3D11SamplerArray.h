/*
 * D3D11SamplerArray.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D11_SAMPLER_ARRAY_H__
#define __LLGL_D3D11_SAMPLER_ARRAY_H__


#include <LLGL/SamplerArray.h>
#include <d3d11.h>
#include <vector>


namespace LLGL
{


class Sampler;

class D3D11SamplerArray : public SamplerArray
{

    public:

        D3D11SamplerArray(unsigned int numSamplers, Sampler* const * samplerArray);

        // Returns the array of sampler state objects.
        inline const std::vector<ID3D11SamplerState*>& GetSamplerStates() const
        {
            return samplerStates_;
        }

    private:

        std::vector<ID3D11SamplerState*> samplerStates_;

};


} // /namespace LLGL


#endif



// ================================================================================
