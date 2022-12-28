/*
 * DbgTexture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_TEXTURE_H
#define LLGL_DBG_TEXTURE_H


#include <LLGL/Texture.h>
#include <string>
#include <set>


namespace LLGL
{


class DbgTexture final : public Texture
{

    public:

        void SetName(const char* name) override;
        Extent3D GetMipExtent(std::uint32_t mipLevel) const override;
        TextureDescriptor GetDesc() const override;
        Format GetFormat() const override;

    public:

        DbgTexture(Texture& instance, const TextureDescriptor& desc);
        //DbgTexture(Texture& instance, DbgTexture* sharedTexture, const TextureViewDescriptor& desc);
        ~DbgTexture();

    public:

        Texture&                instance;
        const TextureDescriptor desc;
        TextureViewDescriptor   viewDesc;
        std::uint32_t           mipLevels           = 1;        // Actual number of MIP-map levels.
        std::string             label;
        const bool              isTextureView       = false;

    private:

        //DbgTexture*             sharedTexture_      = nullptr;  // Reference to the shared texture (only for texture-views)
        //std::set<DbgTexture*>   sharedTextureViews_;             // List of texture views that share the image data with this texture

};


} // /namespace LLGL


#endif



// ================================================================================
