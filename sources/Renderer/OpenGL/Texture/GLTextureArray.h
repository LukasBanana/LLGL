/*
 * GLTextureArray.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_TEXTURE_ARRAY_H
#define LLGL_GL_TEXTURE_ARRAY_H


#include <LLGL/TextureArray.h>
#include "../RenderState/GLStateManager.h"
#include "../OpenGL.h"
#include <vector>


namespace LLGL
{


class Texture;

class GLTextureArray : public TextureArray
{

    public:

        GLTextureArray(std::uint32_t numTextures, Texture* const * textureArray);

        //! Returns the array of texture IDs.
        inline const std::vector<GLuint>& GetIDArray() const
        {
            return idArray_;
        }

        //! Returns the array of texture targets.
        inline const std::vector<GLTextureTarget>& GetTargetArray() const
        {
            return targetArray_;
        }

    private:

        std::vector<GLuint>             idArray_;
        std::vector<GLTextureTarget>    targetArray_;

};


} // /namespace LLGL


#endif



// ================================================================================
