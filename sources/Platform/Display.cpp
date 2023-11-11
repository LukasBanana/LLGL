/*
 * Display.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Display.h>
#include <algorithm>


namespace LLGL
{


/*
 * ======= Protected: =======
 */

void Display::FinalizeDisplayModes(std::vector<DisplayMode>& displayMode)
{
    /* Sort display mode descriptors in ascending order (with, height, frequency) */
    std::sort(displayMode.begin(), displayMode.end(), CompareSWO);

    /* Remove all duplicates */
    displayMode.erase(
        std::unique(displayMode.begin(), displayMode.end()),
        displayMode.end()
    );
}


} // /namespace LLGL



// ================================================================================
