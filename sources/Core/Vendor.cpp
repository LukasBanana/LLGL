/*
 * Vendor.cpp
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Vendor.h"


namespace LLGL
{


// see http://pcidatabase.com/vendors.php?sort=id
LLGL_EXPORT std::string GetVendorByID(unsigned short id)
{
    switch (id)
    {
        case 0x1002: return "Advanced Micro Devices, Inc.";
        case 0x10de: return "NVIDIA Corporation";
        case 0x102b: return "Matrox Electronic Systems Ltd.";
        case 0x1414: return "Microsoft Corporation";
        case 0x5333: return "S3 Graphics Co., Ltd.";
        case 0x8086: return "Intel Corporation";
        case 0x80ee: return "Oracle Corporation";
        case 0x15ad: return "VMware Inc.";
    }
    return "";
}


} // /namespace LLGL



// ================================================================================
