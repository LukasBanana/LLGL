/*
 * Test_Vulkan.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/LLGL.h>
#include <LLGL/Utils/Utility.h>
#include <LLGL/Utils/VertexFormat.h>
#include <LLGL/Trap.h>
#include <Gauss/Gauss.h>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>


#define TEST_QUERY              0
#define TEST_CUSTOM_VKDEVICE    0


#if TEST_CUSTOM_VKDEVICE && _WIN32
#   include <vulkan/vulkan.h>
#   include <vulkan/vulkan_win32.h>
#   include <LLGL/Backend/Vulkan/NativeHandle.h>
#   define VALIDATE_VKRESULT(EXPR)                                                          \
        {                                                                                   \
            VkResult result = (EXPR);                                                       \
            if (result != VK_SUCCESS)                                                       \
            {                                                                               \
                LLGL_THROW_RUNTIME_ERROR(                                                   \
                    #EXPR " failed; VkResult = " + std::to_string(static_cast<int>(result)) \
                );                                                                          \
            }                                                                               \
        }
#   pragma comment(lib, "vulkan-1")
#endif


int main()
{
    try
    {
        LLGL::Log::RegisterCallbackStd();

        #if TEST_CUSTOM_VKDEVICE && _WIN32

        const char* vulkanInstanceExt[] = { VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };

        // Create Vulkan instance
        VkInstance vulkanInstance = VK_NULL_HANDLE;
        VkInstanceCreateInfo instanceCreateInfo = {};
        instanceCreateInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.enabledExtensionCount    = sizeof(vulkanInstanceExt)/sizeof(vulkanInstanceExt[0]);
        instanceCreateInfo.ppEnabledExtensionNames  = vulkanInstanceExt;
        VALIDATE_VKRESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &vulkanInstance));

        // Create Vulkan physical device
        std::uint32_t numPhysicalDevices = 0;
        VALIDATE_VKRESULT(vkEnumeratePhysicalDevices(vulkanInstance, &numPhysicalDevices, nullptr));
        if (numPhysicalDevices == 0)
            LLGL_THROW_RUNTIME_ERROR("failed to find physical device with Vulkan support");

        std::vector<VkPhysicalDevice> physicalDeviceList(numPhysicalDevices, VK_NULL_HANDLE);
        VALIDATE_VKRESULT(vkEnumeratePhysicalDevices(vulkanInstance, &numPhysicalDevices, physicalDeviceList.data()));
        if (physicalDeviceList.empty() || physicalDeviceList.front() == VK_NULL_HANDLE)
            LLGL_THROW_RUNTIME_ERROR("failed to find physical device with Vulkan support");

        VkPhysicalDevice physicalDevice = physicalDeviceList.front();

        // Find queue families
        uint32_t numQueueFamilies = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueueFamilies, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(numQueueFamilies);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueueFamilies, queueFamilies.data());

        std::uint32_t queueGraphicsFamily = 0;
        for (std::uint32_t i = 0; i < queueFamilies.size(); ++i)
        {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                queueGraphicsFamily = i;
                break;
            }
        }

        // Create Vulkan logical device
        const float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType               = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex    = queueGraphicsFamily;
        queueCreateInfo.queueCount          = 1;
        queueCreateInfo.pQueuePriorities    = &queuePriority;

        const char* vulkanDeviceExt[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE1_EXTENSION_NAME };
        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType                      = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos          = &queueCreateInfo;
        deviceCreateInfo.queueCreateInfoCount       = 1;
        deviceCreateInfo.enabledExtensionCount      = sizeof(vulkanDeviceExt)/sizeof(vulkanDeviceExt[0]);
        deviceCreateInfo.ppEnabledExtensionNames    = vulkanDeviceExt;

        VkDevice vulkanDevice = VK_NULL_HANDLE;
        VALIDATE_VKRESULT(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &vulkanDevice));

        // Pass native handles to LLGL
        LLGL::Vulkan::RenderSystemNativeHandle nativeVulkanHandle;
        nativeVulkanHandle.instance         = vulkanInstance;
        nativeVulkanHandle.physicalDevice   = physicalDevice;
        nativeVulkanHandle.device           = vulkanDevice;

        #endif // /TEST_CUSTOM_VKDEVICE

        // Load render system module
        LLGL::RenderSystemDescriptor renderSysDesc("Vulkan");

        #if TEST_CUSTOM_VKDEVICE
        renderSysDesc.nativeHandle      = &nativeVulkanHandle;
        renderSysDesc.nativeHandleSize  = sizeof(nativeVulkanHandle);
        #endif

        auto renderer = LLGL::RenderSystem::Load(renderSysDesc);

        // Print renderer information
        const auto& info = renderer->GetRendererInfo();
        const auto& caps = renderer->GetRenderingCaps();

        LLGL::Log::Printf("Renderer:         %s\n", info.rendererName.c_str());
        LLGL::Log::Printf("Device:           %s\n", info.deviceName.c_str());
        LLGL::Log::Printf("Vendor:           %s\n", info.vendorName.c_str());
        LLGL::Log::Printf("Shading Language: %s\n", info.shadingLanguageName.c_str());

        // Create swap-chain
        LLGL::SwapChainDescriptor swapChainDesc;

        swapChainDesc.resolution    = { 800, 600 };
        swapChainDesc.swapBuffers   = 2;
        //swapChainDesc.fullscreen    = true;
        swapChainDesc.samples       = 8;

        const auto resolution = swapChainDesc.resolution;
        const Gs::Vector2f viewportSize { static_cast<float>(resolution.width), static_cast<float>(resolution.height) };

        LLGL::WindowDescriptor windowDesc;
        {
            windowDesc.size     = swapChainDesc.resolution;
            windowDesc.flags    = LLGL::WindowFlags::Visible | LLGL::WindowFlags::Centered;
        }
        auto window = std::shared_ptr<LLGL::Window>(std::move(LLGL::Window::Create(windowDesc)));

        window->SetTitle("LLGL Vulkan Test");

        auto swapChain = renderer->CreateSwapChain(swapChainDesc, window);

        // Add resize event handler
        class ResizeHandler : public LLGL::Window::EventListener
        {

            public:

                explicit ResizeHandler(LLGL::SwapChain& swapChain) :
                    swapChain_ { swapChain }
                {
                }

                void OnResize(LLGL::Window& sender, const LLGL::Extent2D& clientAreaSize) override
                {
                    swapChain_.ResizeBuffers(clientAreaSize);
                }

            private:

                LLGL::SwapChain& swapChain_;

        };

        window->AddEventListener(std::make_shared<ResizeHandler>(*swapChain));

        // Get command queue
        auto queue = renderer->GetCommandQueue();

        // Create command buffer
        auto commands = renderer->CreateCommandBuffer();

        // Create vertex format
        LLGL::VertexFormat vertexFormat;

        vertexFormat.AppendAttribute({ "coord",    LLGL::Format::RG32Float });
        vertexFormat.AppendAttribute({ "texCoord", LLGL::Format::RG32Float });
        vertexFormat.AppendAttribute({ "color",    LLGL::Format::RGB32Float });

        // Create vertex data
        const float uScale = 2.0f, vScale = -2.0f;

        struct Vertex
        {
            Gs::Vector2f    coord;
            Gs::Vector2f    texCoord;
            LLGL::ColorRGBf color;
        }
        vertices[] =
        {
            { { -1.0f,  1.0f }, { 0.0f  , vScale }, { 1.0f, 1.0f, 1.0f } },
            { { -1.0f, -1.0f }, { 0.0f  , 0.0f   }, { 1.0f, 1.0f, 1.0f } },
            { {  1.0f,  1.0f }, { uScale, vScale }, { 1.0f, 1.0f, 1.0f } },
            { {  1.0f, -1.0f }, { uScale, 0.0f   }, { 1.0f, 1.0f, 1.0f } },
        };

        // Create vertex buffer
        auto vertexBuffer = renderer->CreateBuffer(LLGL::VertexBufferDesc(sizeof(vertices), vertexFormat), vertices);

        // Create shader program
        auto vertShaderDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Vertex,   "Shaders/Triangle.vert.spv");
        auto fragShaderDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Fragment, "Shaders/Triangle.frag.spv");

        vertShaderDesc.vertex.inputAttribs = vertexFormat.attributes;

        // Create constant buffers
        struct Matrices
        {
            Gs::Matrix4f projection;
            Gs::Matrix4f modelView;
        }
        matrices;

        const float projectionScale = 0.005f;
        matrices.projection = Gs::ProjectionMatrix4f::Orthogonal(viewportSize.x * projectionScale, viewportSize.y * projectionScale, -100.0f, 100.0f, 0).ToMatrix4();
        //Gs::RotateFree(matrices.modelView, Gs::Vector3f(0, 0, 1), Gs::pi * 0.5f);

        auto constBufferMatrices = renderer->CreateBuffer(LLGL::ConstantBufferDesc(sizeof(matrices), LLGL::CPUAccessFlags::ReadWrite), &matrices);

        struct Colors
        {
            LLGL::ColorRGBAf diffuse;
        }
        colors;

        //colors.diffuse = { 1.0f, 2.0f, 5.0f };
        colors.diffuse = { 1.0f, 1.0f, 1.0f };

        auto constBufferColors = renderer->CreateBuffer(LLGL::ConstantBufferDesc(sizeof(colors)), &colors);

        // Create sampler
        LLGL::SamplerDescriptor samplerDesc;
        {
            //samplerDesc.mipMapEnabled = false;
            #if 0
            samplerDesc.minFilter = LLGL::TextureFilter::Nearest;
            samplerDesc.magFilter = LLGL::TextureFilter::Nearest;
            #endif
        }
        auto sampler = renderer->CreateSampler(samplerDesc);

        // Create texture
        std::string texFilename = "../examples/Media/Textures/Logo_Vulkan.png";
        int texWidth = 0, texHeight = 0, texComponents = 0;

        auto imageBuffer = stbi_load(texFilename.c_str(), &texWidth, &texHeight, &texComponents, 4);
        if (!imageBuffer)
            LLGL_THROW_RUNTIME_ERROR("failed to load texture from file: \"%s\"", texFilename.c_str());

        LLGL::ImageView imageView;
        {
            imageView.data      = imageBuffer;
            imageView.dataSize  = texWidth*texHeight*4;
        };
        auto texture = renderer->CreateTexture(LLGL::Texture2DDesc(LLGL::Format::RGBA8UNorm, texWidth, texHeight), &imageView);

        stbi_image_free(imageBuffer);

        // Create pipeline layout
        LLGL::PipelineLayoutDescriptor layoutDesc;

        layoutDesc.heapBindings =
        {
            LLGL::BindingDescriptor{ LLGL::ResourceType::Buffer,  LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::VertexStage  , 2 },
            LLGL::BindingDescriptor{ LLGL::ResourceType::Buffer,  LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::FragmentStage, 5 },
            LLGL::BindingDescriptor{ LLGL::ResourceType::Sampler, 0,                               LLGL::StageFlags::FragmentStage, 3 },
            LLGL::BindingDescriptor{ LLGL::ResourceType::Texture, 0,                               LLGL::StageFlags::FragmentStage, 4 },
        };

        auto pipelineLayout = renderer->CreatePipelineLayout(layoutDesc);

        // Create resource view heap
        auto resourceViewHeap = renderer->CreateResourceHeap(pipelineLayout, { constBufferMatrices, constBufferColors, sampler, texture });

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.vertexShader       = renderer->CreateShader(vertShaderDesc);
            pipelineDesc.fragmentShader     = renderer->CreateShader(fragShaderDesc);
            pipelineDesc.renderPass         = swapChain->GetRenderPass();
            pipelineDesc.pipelineLayout     = pipelineLayout;
            pipelineDesc.primitiveTopology  = LLGL::PrimitiveTopology::TriangleStrip;

            pipelineDesc.viewports.push_back(LLGL::Viewport{ 0.0f, 0.0f, viewportSize.x, viewportSize.y });

            pipelineDesc.blend.targets[0].blendEnabled = true;
        }
        auto pipeline = renderer->CreatePipelineState(pipelineDesc);

        if (auto report = pipeline->GetReport())
            LLGL::Log::Errorf("%s\n", report->GetText());

        // Create query
        #if TEST_QUERY
        auto query = renderer->CreateQueryHeap(LLGL::QueryType::PipelineStatistics);
        #endif

        // Add input event listener
        LLGL::Input input{ *window };

        int vsyncInterval = 1;
        swapChain->SetVsyncInterval(vsyncInterval);

        // Main loop
        while (LLGL::Surface::ProcessEvents() && !window->HasQuit() && !input.KeyDown(LLGL::Key::Escape))
        {
            // Update user input
            if (input.KeyDown(LLGL::Key::F1))
            {
                vsyncInterval = 1 - vsyncInterval;
                swapChain->SetVsyncInterval(vsyncInterval);
            }

            // Render scene
            commands->Begin();
            {
                commands->SetVertexBuffer(*vertexBuffer);
                commands->SetPipelineState(*pipeline);
                commands->SetResourceHeap(*resourceViewHeap);

                // Update constant buffer
                Gs::RotateFree(matrices.modelView, Gs::Vector3f(0, 0, 1), Gs::pi * 0.002f);
                commands->UpdateBuffer(*constBufferMatrices, 0, &matrices, sizeof(matrices));

                commands->BeginRenderPass(*swapChain);
                {
                    commands->SetViewport(swapChain->GetResolution());
                    commands->Clear(LLGL::ClearFlags::ColorDepth, { 0.2f, 0.2f, 0.4f, 1.0f });

                    // Draw scene
                    #if TEST_QUERY
                    commands->BeginQuery(*query);
                    commands->Draw(4, 0);
                    commands->EndQuery(*query);

                    queue->WaitIdle();
                    LLGL::QueryPipelineStatistics stats;
                    if (commands->QueryPipelineStatisticsResult(*query, stats))
                    {
                        __debugbreak();
                    }
                    #else
                    commands->Draw(4, 0);
                    #endif
                }
                commands->EndRenderPass();

                // Update constant buffer
                Gs::RotateFree(matrices.modelView, Gs::Vector3f(0, 0, 1), Gs::pi * 0.05f);
                commands->UpdateBuffer(*constBufferMatrices, 0, &matrices, sizeof(matrices));
                Gs::RotateFree(matrices.modelView, Gs::Vector3f(0, 0, 1), Gs::pi * -0.05f);

                commands->BeginRenderPass(*swapChain);
                {
                    // Draw scene again
                    commands->Draw(4, 0);
                }
                commands->EndRenderPass();
            }
            commands->End();
            queue->Submit(*commands);

            // Present result on screen
            swapChain->Present();
        }

        #if TEST_CUSTOM_VKDEVICE && _WIN32

        LLGL::RenderSystem::Unload(std::move(renderer));

        vkDestroyDevice(vulkanDevice, nullptr);
        vkDestroyInstance(vulkanInstance, nullptr);

        #endif
    }
    catch (const std::exception& e)
    {
        LLGL::Log::Errorf("%s\n", e.what());
        #ifdef _WIN32
        system("pause");
        #endif
    }

    return 0;
}
