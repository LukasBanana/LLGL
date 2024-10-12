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


#if __cplusplus >= 201703L
#   define LLGL_MAYBE_UNUSED [[maybe_unused]]
#elif defined __GNUC__ || defined __clang__
#   define LLGL_MAYBE_UNUSED __attribute__((unused))
#else
#   define LLGL_MAYBE_UNUSED
#endif

#define DEF_TEST(NAME) \
    TestResult TestbedContext::Test##NAME(unsigned frame)

#define DEF_RITEST(NAME) \
    TestResult TestbedContext::Test##NAME(const Options& opt)

#define CREATE_BUFFER_COND(COND, OBJ, DESC, NAME, INITIAL)              \
    LLGL_MAYBE_UNUSED Buffer* OBJ = nullptr;                            \
    LLGL_MAYBE_UNUSED const char* OBJ##_Name = NAME;                    \
    if (!!(COND))                                                       \
    {                                                                   \
        TestResult result = CreateBuffer(DESC, NAME, &OBJ, INITIAL);    \
        if (result != TestResult::Passed)                               \
            return result;                                              \
    }

#define CREATE_BUFFER(OBJ, DESC, NAME, INITIAL)                         \
    CREATE_BUFFER_COND(true, OBJ, DESC, NAME, INITIAL)

#define CREATE_TEXTURE_COND(COND, OBJ, DESC, NAME, INITIAL)             \
    LLGL_MAYBE_UNUSED Texture* OBJ = nullptr;                           \
    LLGL_MAYBE_UNUSED const char* OBJ##_Name = NAME;                    \
    if (!!(COND))                                                       \
    {                                                                   \
        TestResult result = CreateTexture(DESC, NAME, &OBJ, INITIAL);   \
        if (result != TestResult::Passed)                               \
            return result;                                              \
    }

#define CREATE_TEXTURE(OBJ, DESC, NAME, INITIAL)                        \
    CREATE_TEXTURE_COND(true, OBJ, DESC, NAME, INITIAL)

#define CREATE_RENDER_TARGET(OBJ, DESC, NAME)                       \
    LLGL_MAYBE_UNUSED RenderTarget* OBJ = nullptr;                  \
    LLGL_MAYBE_UNUSED const char* OBJ##_Name = NAME;                \
    {                                                               \
        TestResult result = CreateRenderTarget(DESC, NAME, &OBJ);   \
        if (result != TestResult::Passed)                           \
            return result;                                          \
    }

#define CREATE_GRAPHICS_PSO_EXT(OBJ, DESC, NAME)                    \
    {                                                               \
        TestResult result = CreateGraphicsPSO(DESC, NAME, &OBJ);    \
        if (result != TestResult::Passed)                           \
            return result;                                          \
    }

#define CREATE_GRAPHICS_PSO(OBJ, DESC, NAME)                        \
    LLGL_MAYBE_UNUSED PipelineState* OBJ = nullptr;                 \
    LLGL_MAYBE_UNUSED const char* OBJ##_Name = NAME;                \
    CREATE_GRAPHICS_PSO_EXT(OBJ, DESC, NAME)


#endif



// ================================================================================
