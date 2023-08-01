/*
 * Vendor.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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
