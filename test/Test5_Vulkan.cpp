/*
 * Test5_Vulkan.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Helper.h"


//#define TEST_RENDER_TARGET
//#define TEST_QUERY
//#define TEST_STORAGE_BUFFER


int main()
{
    try
    {
        // Load render system module
        auto renderer = LLGL::RenderSystem::Load("Vulkan");

        // Create render context
        LLGL::RenderContextDescriptor contextDesc;

        contextDesc.videoMode.resolution        = { 800, 600 };
        //contextDesc.videoMode.fullscreen        = true;

        contextDesc.multiSampling.enabled       = true;
        contextDesc.multiSampling.samples       = 8;

        contextDesc.vsync.enabled               = true;



    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        #ifdef _WIN32
        system("pause");
        #endif
    }

    return 0;
}
