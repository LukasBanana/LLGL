/*
 * Testbed.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_TESTBED_H
#define LLGL_TESTBED_H


#include "TestbedContext.h"
#include <LLGL/Utils/ForRange.h>
#include <thread>
#include <chrono>
#include <inttypes.h>

using namespace LLGL;


#define DEF_TEST(NAME) \
    TestResult TestbedContext::Test##NAME(unsigned frame)

#define CREATE_BUFFER(OBJ, DESC, NAME, INITIAL)                         \
    Buffer* OBJ = nullptr;                                              \
    {                                                                   \
        TestResult result = CreateBuffer(DESC, NAME, &OBJ, INITIAL);    \
        if (result != TestResult::Passed)                               \
            return result;                                              \
    }


#endif



// ================================================================================
