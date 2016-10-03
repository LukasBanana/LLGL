/*
 * GLExtensionViewer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLExtensionViewer.h"


namespace LLGL
{


GLExtensionViewer::GLExtensionViewer(OpenGLExtensionMap&& extensions) :
    extensions_( std::move(extensions) )
{
}

bool GLExtensionViewer::HasExtension(const std::string& name) const
{
    return (extensions_.find(name) != extensions_.end());
}


} // /namespace LLGL



// ================================================================================
