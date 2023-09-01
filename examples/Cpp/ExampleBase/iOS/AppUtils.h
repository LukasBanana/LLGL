/*
 * AppUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <string>

// Tries to find the specified filename in the main NSBundle.
std::string FindNSResourcePath(const std::string& filename);
