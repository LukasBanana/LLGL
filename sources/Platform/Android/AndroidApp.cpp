/*
 * AndroidApp.cpp
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "AndroidApp.h"


namespace LLGL
{


AndroidApp& AndroidApp::Get()
{
    static AndroidApp instance;
    return instance;
}

void AndroidApp::Initialize(android_app* state)
{
    state_ = state;
}


} // /namespace LLGL



// ================================================================================
