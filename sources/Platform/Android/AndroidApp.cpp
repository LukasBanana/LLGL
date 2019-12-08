/*
 * AndroidApp.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
