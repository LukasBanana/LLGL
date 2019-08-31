/*
 * Display.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
