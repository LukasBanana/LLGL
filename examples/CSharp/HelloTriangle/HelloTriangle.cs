/*
 * HelloTriangle.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGLExamples
{
    public class HelloTriangle
    {
        sealed class MyEventListener : LLGL.Window.EventListener
        {
            public override void OnKeyDown(LLGL.Window sender, LLGL.Key keyCode)
            {
                if (keyCode == LLGL.Key.Escape)
                {
                    sender.PostQuit();
                }
            }
        };

        struct Vertex
        {
            public float x, y;
            public byte r, g, b, a;
        };

        private static readonly LLGL.Color bgColor = new LLGL.Color(0.1f, 0.1f, 0.2f);
        private static readonly LLGL.Extent2D initialResolution = new LLGL.Extent2D(800, 600);

        private LLGL.RenderingDebugger Debugger { get; set; } = new LLGL.RenderingDebugger();
        private LLGL.RenderSystem Renderer { get; set; }
        private LLGL.CommandBuffer CmdBuffer { get; set; }
        private LLGL.SwapChain SwapChain { get; set; }
        private LLGL.Buffer VertexBuffer { get; set; }
        private LLGL.Shader VS { get; set; }
        private LLGL.Shader FS { get; set; }
        private LLGL.PipelineState PSO { get; set; }
        private LLGL.VertexAttribute[] VertexAttribs { get; set; }

        private void CreateBuffers()
        {
            const float s = 0.5f;
            var vertices = new Vertex[3]
            {
                new Vertex() { x = 0.0f, y = +s, r = 255, g =   0, b =   0, a = 255 },
                new Vertex() { x =   +s, y = -s, r =   0, g = 255, b =   0, a = 255 },
                new Vertex() { x =   -s, y = -s, r =   0, g =   0, b = 255, a = 255 },
            };

            unsafe
            {
                VertexAttribs = new LLGL.VertexAttribute[2]
                {
                    new LLGL.VertexAttribute("position", format: LLGL.Format.RG32Float,  location: 0, offset: 0,               stride: sizeof(Vertex)),
                    new LLGL.VertexAttribute("color",    format: LLGL.Format.RGBA8UNorm, location: 1, offset: sizeof(float)*2, stride: sizeof(Vertex)),
                };

                fixed (Vertex* verticesPtr = vertices)
                {
                    var bufferDesc = new LLGL.BufferDescriptor()
                    {
                        DebugName = "MyVertexBuffer",
                        Size = sizeof(Vertex) * vertices.Length,
                        BindFlags = LLGL.BindFlags.VertexBuffer,
                        VertexAttribs = VertexAttribs
                    };
                    VertexBuffer = Renderer.CreateBufferUnsafe(bufferDesc, verticesPtr);
                }
            }
        }

        private void CreateShaders()
        {
            const string shaderSource =
                @"
                struct VertexIn {
                    float2 position : POSITION;
                    float4 color    : COLOR;
                };
                struct VertexOut {
                    float4 position : SV_Position;
                    float4 color    : COLOR;
                };
                void VSMain(VertexIn inp, out VertexOut outp) {
                    outp.position = float4(inp.position.xy, 0, 1);
                    outp.color = inp.color;
                }
                float4 PSMain(VertexOut inp) : SV_Target {
                    return inp.color;
                }
                "
            ;

            // Create vertex shader
            var vsDesc = new LLGL.ShaderDescriptor(
                type: LLGL.ShaderType.Vertex,
                sourceText: shaderSource,
                sourceType: LLGL.ShaderSourceType.CodeString,
                entryPoint: "VSMain",
                profile: "vs_5_0",
                name: "MyVertexShader",
                vertex: new LLGL.VertexShaderAttributes(inputAttribs: VertexAttribs)
            );

            VS = Renderer.CreateShader(vsDesc);

            var vsReport = VS.Report;
            if (vsReport != null && vsReport.HasErrors)
            {
                Console.WriteLine(vsReport.ToString());
            }

            // Create fragment shader
            var fsDesc = new LLGL.ShaderDescriptor(
                type: LLGL.ShaderType.Fragment,
                sourceText: shaderSource,
                sourceType: LLGL.ShaderSourceType.CodeString,
                entryPoint: "PSMain",
                profile: "ps_5_0",
                name: "MyFragmentShader"
            );

            FS = Renderer.CreateShader(fsDesc);

            var fsReport = FS.Report;
            if (fsReport != null && fsReport.HasErrors)
            {
                Console.WriteLine(fsReport.ToString());
            }
        }

        private void CreatePipeline()
        {
            var psoDesc = new LLGL.GraphicsPipelineDescriptor()
            {
                DebugName = "MyGraphicsPSO",
                RenderPass = SwapChain.RenderPass,
                VertexShader = VS,
                FragmentShader = FS,
            };
            PSO = Renderer.CreatePipelineState(psoDesc);

            var psoReport = PSO.Report;
            if (psoReport != null && psoReport.HasErrors)
            {
                Console.WriteLine(psoReport.ToString());
            }
        }

        private void PrintRendererInfo(LLGL.RendererInfo info)
        {
            Console.WriteLine($"Renderer:         {info.RendererName}");
            Console.WriteLine($"Device:           {info.DeviceName}");
            Console.WriteLine($"Vendor:           {info.VendorName}");
            Console.WriteLine($"Shading Language: {info.ShadingLanguageName}");
        }

        private int Run()
        {
            var report = new LLGL.Report();
            Renderer = LLGL.RenderSystem.Load(new LLGL.RenderSystemDescriptor("Direct3D11", debugger: Debugger), report);
            if (Renderer == null)
            {
                Console.Write(report.ToString());
                return 1;
            }

            var swapChainDesc = new LLGL.SwapChainDescriptor(resolution: initialResolution, samples: 8);
            SwapChain = Renderer.CreateSwapChain(swapChainDesc);
            SwapChain.DebugName = "MySwapChain";

            PrintRendererInfo(Renderer.RendererInfo);

            // Change window title and register event listener
            var window = SwapChain.Surface.AsWindow();
            window.AddEventListener(new MyEventListener());

            window.Title = $"LLGL C# Example: Hello Triangle ( {Renderer.RendererInfo.RendererName} )";
            window.Show();

            // Create command buffer
            CmdBuffer = Renderer.CreateCommandBuffer(
                new LLGL.CommandBufferDescriptor()
                {
                    DebugName = "MyCommandBuffer",
                    Flags = LLGL.CommandBufferFlags.ImmediateSubmit
                }
            );

            // Create resources
            CreateBuffers();
            CreateShaders();
            CreatePipeline();

            // Main loop
            while (LLGL.Surface.ProcessEvents() && !window.HasQuit)
            {
                //Debugger.TimeRecording = true;

                RenderFrame();
                SwapChain.Present();

                var frameProfile = Debugger.FlushProfile();
            }

            return 0;
        }

        private void RenderFrame()
        {
            CmdBuffer.Begin();
            {
                CmdBuffer.SetVertexBuffer(VertexBuffer);

                CmdBuffer.BeginRenderPass(SwapChain);
                {
                    CmdBuffer.Clear(LLGL.ClearFlags.Color, new LLGL.ClearValue(bgColor));

                    CmdBuffer.SetPipelineState(PSO);

                    CmdBuffer.SetViewport(new LLGL.Viewport(width: (float)initialResolution.Width, height: (float)initialResolution.Height));

                    CmdBuffer.Draw(3, 0);
                }
                CmdBuffer.EndRenderPass();
            }
            CmdBuffer.End();
        }

        public static int Main()
        {
            var example = new HelloTriangle();
            return example.Run();
        }

    }
}




// ================================================================================
