/*
 * GLExtensionViewer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_EXTENSION_VIEWER_H__
#define __LLGL_GL_EXTENSION_VIEWER_H__


#include <string>
#include <set>


namespace LLGL
{


//! OpenGL extension map type.
using GLExtensionList = std::set<std::string>;

class GLExtensionViewer
{

    public:

        GLExtensionViewer(GLExtensionList&& extensions);

        //! Returns true if the specified extensions is supported.
        bool HasExtension(const std::string& name) const;

        //! Returns the hash-map of all extensions.
        inline const GLExtensionList& GetExtensions() const
        {
            return extensions_;
        }

    private:

        GLExtensionList extensions_;

};


} // /namespace LLGL


#endif



// ================================================================================
