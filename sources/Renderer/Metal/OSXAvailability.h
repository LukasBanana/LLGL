/*
 * OSXAvailability.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OSX_AVAILABILITY_H
#define LLGL_OSX_AVAILABILITY_H


#define LLGL_OSX_AVAILABLE(...) \
    ([]() -> bool { if (@available(__VA_ARGS__)) { return true; } else { return false; }; }) ()


#endif



// ================================================================================
