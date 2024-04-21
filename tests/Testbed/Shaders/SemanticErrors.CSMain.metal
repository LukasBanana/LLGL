/*
 * SemanticErrors.CSMain.metal
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <metal_stdlib>

using namespace metal;

kernel void CSMain(uint id [[thread_position_in_grid]])
{
    OutBuffer[id] = 0; // <-- Expected error: Undefined identifier "OutBuffer"
}
