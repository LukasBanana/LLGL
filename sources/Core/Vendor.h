/*
 * Vendor.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VENDOR_H
#define LLGL_VENDOR_H


#include <LLGL/Export.h>
#include <string>


namespace LLGL
{


//! Returns the name of the hardware vendor by the specified ID number.
LLGL_EXPORT std::string GetVendorByID(unsigned short id);


} // /namespace LLGL


#endif



// ================================================================================
