/*
 * SemanticErrors.CSMain.hlsl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

[numthreads(1, 1, 1)]
void CSMain(uint id : SV_DispatchThreadID)
{
    OutBuffer[id] = 0; // <-- Expected error: Undefined identifier "OutBuffer"
}
