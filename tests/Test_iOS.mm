/*
 * Test_iOS.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/LLGL.h>
#include <LLGL/Misc/VertexFormat.h>
#include <iostream>


#if 0 // TESTING

#import <UIKit/UIKit.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

@interface AppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow* window;
@end

@interface AppDelegate ()
@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    return YES;
}
- (void)applicationWillResignActive:(UIApplication *)application {}
- (void)applicationDidEnterBackground:(UIApplication *)application {}
- (void)applicationWillEnterForeground:(UIApplication *)application {}
- (void)applicationDidBecomeActive:(UIApplication *)application {}
- (void)applicationWillTerminate:(UIApplication *)application {}

@end

MTKView* _view;

dispatch_semaphore_t _inFlightSemaphore;
id <MTLDevice> _device;
id <MTLCommandQueue> _commandQueue;
id <MTLRenderPipelineState> _pipelineState;
id <MTLDepthStencilState> _depthState;

int main(int argc, char* argv[])
{
    /*@autoreleasepool
    {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }*/

    _device = MTLCreateSystemDefaultDevice();

    CGRect screenBounds = [UIScreen mainScreen].nativeBounds;
    _view = [[MTKView alloc] initWithFrame:screenBounds device:_device];

    _view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    _view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
    _view.sampleCount = 1;

    UIView* topView = [[[[UIApplication sharedApplication] keyWindow] subviews] lastObject];
    [topView addSubview:_view];

    _commandQueue = [_device newCommandQueue];

    /* Get source */
    const char* shaderSourceStr =
    (
        "#include <metal_stdlib>\n"
        "#include <simd/simd.h>\n"
        "using namespace metal;\n"
        "struct VOut {\n"
        "    float4 position [[position]];\n"
        "    float4 color;\n"
        "};\n"
        "vertex VOut VS(uint vid [[vertex_id]]) {\n"
        "    VOut outp;\n"
        "    switch (vid) {\n"
        "        case 0:\n"
        "            outp.position = float4(0.0, 0.5, 0.0, 1.0);\n"
        "            outp.color    = float4(1.0, 0.0, 0.0, 1.0);\n"
        "            break;\n"
        "        case 1:\n"
        "            outp.position = float4(0.5, -0.5, 0.0, 1.0);\n"
        "            outp.color    = float4(0.0, 1.0, 0.0, 1.0);\n"
        "            break;\n"
        "        case 2:\n"
        "            outp.position = float4(-0.5, -0.5, 0.0, 1.0);\n"
        "            outp.color    = float4(0.0, 0.0, 1.0, 1.0);\n"
        "            break;\n"
        "    }\n"
        "    return outp;\n"
        "}\n"
        "fragment float4 FS(VOut inp [[stage_in]]) {\n"
        "    return inp.color;\n"
        "}\n"
    );

    NSString* sourceString = nil;
    sourceString = [[NSString alloc] initWithUTF8String:shaderSourceStr];

    if (sourceString == nil)
        throw std::runtime_error("cannot compile Metal shader without source");

    /* Convert entry point to NSString, and initialize shader compile options */
    MTLCompileOptions* shaderOpt = [MTLCompileOptions alloc];
    [shaderOpt setLanguageVersion:MTLLanguageVersion2_0];

    /* Load shader library */
    NSError* shaderError = [NSError alloc];

    id<MTLLibrary> shaderLibrary = [_device
        newLibraryWithSource:   sourceString
        options:                shaderOpt
        error:                  &shaderError
    ];

    [sourceString release];
    [shaderOpt release];
    [shaderError release];

    id<MTLFunction> vertexFunction = [shaderLibrary newFunctionWithName:@"VS"];
    id<MTLFunction> fragmentFunction = [shaderLibrary newFunctionWithName:@"FS"];

    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label = @"MyPipeline";
    pipelineStateDescriptor.sampleCount = _view.sampleCount;
    pipelineStateDescriptor.vertexFunction = vertexFunction;
    pipelineStateDescriptor.fragmentFunction = fragmentFunction;
    //pipelineStateDescriptor.vertexDescriptor = _mtlVertexDescriptor;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = _view.colorPixelFormat;
    pipelineStateDescriptor.depthAttachmentPixelFormat = _view.depthStencilPixelFormat;
    pipelineStateDescriptor.stencilAttachmentPixelFormat = _view.depthStencilPixelFormat;

    NSError *error = NULL;
    _pipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
    if (!_pipelineState)
    {
        NSLog(@"Failed to created pipeline state, error %@", error);
    }

    MTLDepthStencilDescriptor *depthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
    depthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;
    depthStateDesc.depthWriteEnabled = NO;
    _depthState = [_device newDepthStencilStateWithDescriptor:depthStateDesc];

    _inFlightSemaphore = dispatch_semaphore_create(3);

    while (true)
    {
        dispatch_semaphore_wait(_inFlightSemaphore, DISPATCH_TIME_FOREVER);

        id <MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];

        __block dispatch_semaphore_t block_sema = _inFlightSemaphore;
        [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer)
        {
            dispatch_semaphore_signal(block_sema);
        }];

        MTLRenderPassDescriptor* renderPassDescriptor = _view.currentRenderPassDescriptor;

        if(renderPassDescriptor != nil)
        {
            id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];

            [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
            [renderEncoder setCullMode:MTLCullModeBack];
            [renderEncoder setRenderPipelineState:_pipelineState];
            [renderEncoder setDepthStencilState:_depthState];

            [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];

            [renderEncoder endEncoding];

            [commandBuffer presentDrawable:_view.currentDrawable];
        }

        [commandBuffer commit];

        [_view draw];
    }

    return 0;
}

#else

int main()
{
    try
    {
        LLGL::Log::SetReportCallbackStd(&std::cout);

        // Load render system module
        auto renderer = LLGL::RenderSystem::Load("Metal");

        // Create render context
        LLGL::SwapChainDescriptor swapChainDesc;
        {
            if (auto display = LLGL::Display::GetPrimary())
                swapChainDesc.resolution = display->GetDisplayMode().resolution;
        }
        auto swapChain = renderer->CreateSwapChain(swapChainDesc);

        auto& canvas = LLGL::CastTo<LLGL::Canvas>(swapChain->GetSurface());

        // Print rendering capabilities
        const auto& info = renderer->GetRendererInfo();

        std::cout << "Renderer: " << info.rendererName << std::endl;
        std::cout << "Vendor: " << info.vendorName << std::endl;
        std::cout << "Device: " << info.deviceName << std::endl;
        std::cout << "Shading Language: " << info.shadingLanguageName << std::endl;

        // Create command buffer
        auto commandQueue = renderer->GetCommandQueue();
        auto commands = renderer->CreateCommandBuffer();

        // Setup input controller
        //auto input = std::make_shared<LLGL::Input>();
        //canvas.AddEventListener(input);

        // Create vertex buffer
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "POSITION", LLGL::Format::RG32Float });
        vertexFormat.AppendAttribute({ "COLOR", LLGL::Format::RGBA8UNorm });

        const float triSize = 0.5f;

        struct Vertex
        {
            float   position[2];
            uint8_t color[4];
        }
        vertices[] =
        {
            { {        0,  triSize }, { 255,   0,   0, 255 } },
            { {  triSize, -triSize }, {   0, 255,   0, 255 } },
            { { -triSize, -triSize }, {   0,   0, 255, 255 } },
        };

        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.size           = sizeof(vertices);
            vertexBufferDesc.bindFlags      = LLGL::BindFlags::VertexBuffer;
            vertexBufferDesc.vertexAttribs  = vertexFormat.attributes;
        }
        auto vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices);

        // Load shader
        const char* shaderSource =
        (
            "#include <metal_stdlib>\n"
            "#include <simd/simd.h>\n"
            "using namespace metal;\n"
            "struct VIn {\n"
            "    float2 position [[attribute(0)]];\n"
            "    float4 color    [[attribute(1)]];\n"
            "};\n"
            "struct VOut {\n"
            "    float4 position [[position]];\n"
            "    float4 color;\n"
            "};\n"
            "vertex VOut VS(VIn inp [[stage_in]]) {\n"
            "    VOut outp;\n"
            "    outp.position = float4(inp.position, 0.0, 1.0);\n"
            "    outp.color    = inp.color;\n"
            "    return outp;\n"
            "}\n"
            "fragment float4 FS(VOut inp [[stage_in]]) {\n"
            "    return inp.color;\n"
            "}\n"
        );

        LLGL::ShaderDescriptor vsDesc;
        vsDesc.type         = LLGL::ShaderType::Vertex;
        vsDesc.sourceType   = LLGL::ShaderSourceType::CodeString;
        vsDesc.entryPoint   = "VS";
        vsDesc.source       = shaderSource;
        vsDesc.profile      = "2.1";

        LLGL::ShaderDescriptor fsDesc;
        fsDesc.type         = LLGL::ShaderType::Fragment;
        fsDesc.sourceType   = LLGL::ShaderSourceType::CodeString;
        fsDesc.entryPoint   = "FS";
        fsDesc.profile      = "2.1";
        fsDesc.source       = shaderSource;

        vsDesc.vertex.inputAttribs = vertexFormat.attributes;

        auto vertShader = renderer->CreateShader(vsDesc);
        auto fragShader = renderer->CreateShader(fsDesc);

        for (auto shader : { vertShader, fragShader })
        {
            if (auto report = shader->GetReport())
            {
                if (report->HasErrors())
                    std::cerr << report->GetText() << std::endl;
            }
        }

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.vertexShader   = vertShader;
            pipelineDesc.fragmentShader = fragShader;
            pipelineDesc.renderPass     = swapChain->GetRenderPass();
        }
        auto pipeline = renderer->CreatePipelineState(pipelineDesc);

        LLGL::ColorRGBAf bgColor{ 0.0, 1.0f, 0.0f };

        // Main loop
        while (canvas.ProcessEvents()/* && !input->KeyDown(LLGL::Key::Escape)*/)
        {
            commands->Begin();
            {
                // Set viewport, graphics pipeline, and vertex buffer
                commands->SetViewport(swapChain->GetResolution());
                commands->SetPipelineState(*pipeline);
                commands->SetVertexBuffer(*vertexBuffer);

                // Render color pass
                commands->BeginRenderPass(*swapChain);
                {
                    commands->Clear(LLGL::ClearFlags::Color, bgColor);
                    commands->Draw(3, 0);
                }
                commands->EndRenderPass();
            }
            commands->End();
            commandQueue->Submit(*commands);

            swapChain->Present();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}

#endif
