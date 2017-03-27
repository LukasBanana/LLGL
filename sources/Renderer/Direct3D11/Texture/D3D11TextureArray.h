/*
 * D3D11TextureArray.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_TEXTURE_ARRAY_H
#define LLGL_D3D11_TEXTURE_ARRAY_H


#include <LLGL/TextureArray.h>
#include <d3d11.h>
#include <vector>


namespace LLGL
{


class Texture;

class D3D11TextureArray : public TextureArray
{

    public:

        D3D11TextureArray(unsigned int numTextures, Texture* const * textureArray);

        // Returns the array of SRV objects.
        inline const std::vector<ID3D11ShaderResourceView*>& GetResourceViews() const
        {
            return resourceViews_;
        }

    private:

        std::vector<ID3D11ShaderResourceView*> resourceViews_;

};


} // /namespace LLGL


#endif



// ================================================================================
