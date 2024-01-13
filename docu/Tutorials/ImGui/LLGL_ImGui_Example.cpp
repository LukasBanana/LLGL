/*
 * LLGL_ImGui_Example.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/LLGL.h>
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Backend/Direct3D11/NativeHandle.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#ifndef _WIN32
#   error Platform not supported for this example
#endif


LLGL::RenderSystemPtr   renderer;
LLGL::SwapChain*        swapChain;
LLGL::CommandBuffer*    cmdBuffer;
LLGL::Input             input;

ID3D11Device*           d3dDevice;
ID3D11DeviceContext*    d3dDeviceContext;

static void InitLLGL()
{
    renderer = LLGL::RenderSystem::Load("Direct3D11");

    LLGL::SwapChainDescriptor swapChainDesc;
    swapChainDesc.resolution = { 1280, 768 };
    swapChain = renderer->CreateSwapChain(swapChainDesc);

    cmdBuffer = renderer->CreateCommandBuffer(LLGL::CommandBufferFlags::ImmediateSubmit);
}

static void ShutdownLLGL()
{
    // Release D3D handles
    if (d3dDevice != nullptr)
        d3dDevice->Release();
    if (d3dDeviceContext != nullptr)
        d3dDeviceContext->Release();

    // Unload LLGL
    input.Drop(swapChain->GetSurface());
    LLGL::RenderSystem::Unload(std::move(renderer));
}

static void InitImGui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup platform backend
    LLGL::Window& wnd = LLGL::CastTo<LLGL::Window>(swapChain->GetSurface());
    LLGL::NativeHandle nativeHandle;
    wnd.GetNativeHandle(&nativeHandle, sizeof(nativeHandle));

    ImGui_ImplWin32_Init(nativeHandle.window);

    // Setup renderer backend
    LLGL::Direct3D11::RenderSystemNativeHandle nativeDeviceHandle;
    renderer->GetNativeHandle(&nativeDeviceHandle, sizeof(nativeDeviceHandle));
    d3dDevice = nativeDeviceHandle.device;

    LLGL::Direct3D11::CommandBufferNativeHandle nativeContextHandle;
    cmdBuffer->GetNativeHandle(&nativeContextHandle, sizeof(nativeContextHandle));
    d3dDeviceContext = nativeContextHandle.deviceContext;

    ImGui_ImplDX11_Init(d3dDevice, d3dDeviceContext);
}

static void ShutdownImGui()
{
    // Shutdown ImGui
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

int main()
{
    InitLLGL();
    InitImGui();

    LLGL::Window& wnd = LLGL::CastTo<LLGL::Window>(swapChain->GetSurface());
    wnd.SetTitle("LLGL ImGui Example - " + LLGL::UTF8String(renderer->GetRendererInfo().rendererName));
    wnd.Show();

    input.Listen(wnd);

    const float backgroundColor[4] = { 0.2f, 0.2f, 0.4f, 1.0f };

    ImGuiIO& io = ImGui::GetIO();

    while (LLGL::Surface::ProcessEvents() && !input.KeyPressed(LLGL::Key::Escape))
    {
        // Forward user input to ImGui
        if (input.KeyDown(LLGL::Key::LButton))
        {
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
        }
        if (input.KeyUp(LLGL::Key::LButton))
        {
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Show ImGui's demo window
        ImGui::ShowDemoWindow();

        // Rendering
        cmdBuffer->Begin();
        {
            cmdBuffer->BeginRenderPass(*swapChain);
            {
                cmdBuffer->Clear(LLGL::ClearFlags::Color, LLGL::ClearValue{ backgroundColor });

                // GUI Rendering
                ImGui::Render();
                ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            }
            cmdBuffer->EndRenderPass();
        }
        cmdBuffer->End();

        // Present result on screen
        swapChain->Present();

        // Reset input state
        input.Reset();
    }

    ShutdownImGui();
    ShutdownLLGL();

    return 0;
}

