/*
 * GLExtensionViewer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_EXTENSION_VIEWER_H__
#define __LLGL_GL_EXTENSION_VIEWER_H__


#include "GLExtensionsEnum.h"
#include <string>
#include <array>


namespace LLGL
{


class GLExtensionViewer
{

    public:

        GLExtensionViewer();

        //! Returns true if the specified extension is supported.
        inline bool HasExtension(GLExt extension) const
        {
            return extensions_[static_cast<unsigned int>(extension)];
        }

        void Enable(GLExt extension);

    private:

        static const unsigned int numExtensions = static_cast<int>(GLExt::Count);

        std::array<bool, numExtensions> extensions_;

};


} // /namespace LLGL


#endif



// ================================================================================
