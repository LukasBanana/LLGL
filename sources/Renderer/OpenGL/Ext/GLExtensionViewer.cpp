/*
 * GLExtensionViewer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLExtensionViewer.h"
#include <algorithm>


namespace LLGL
{


GLExtensionViewer::GLExtensionViewer()
{
    std::fill(extensions_.begin(), extensions_.end(), false);
}

void GLExtensionViewer::Enable(GLExt extension)
{
    extensions_[static_cast<unsigned int>(extension)] = true;
}


} // /namespace LLGL



// ================================================================================
