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

void Display::FinalizeDisplayModes(std::vector<DisplayModeDescriptor>& displayModeDescs)
{
    /* Sort display mode descriptors in ascending order (with, height, frequency) */
    std::sort(displayModeDescs.begin(), displayModeDescs.end(), CompareSWO);

    /* Remove all duplicates */
    displayModeDescs.erase(
        std::unique(displayModeDescs.begin(), displayModeDescs.end()),
        displayModeDescs.end()
    );
}


} // /namespace LLGL



// ================================================================================
