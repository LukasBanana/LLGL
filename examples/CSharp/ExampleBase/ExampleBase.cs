/*
 * Texturing.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGLExamples
{
    public abstract class ExampleBase
    {
        sealed class MyEventListener : LLGL.Window.EventListener
        {
            private ExampleBase App { get; set; }

            public MyEventListener(ExampleBase app)
            {
                App = app;
            }

            public override void OnKeyDown(LLGL.Window sender, LLGL.Key keyCode)
            {
                switch (keyCode)
                {
                    case LLGL.Key.Escape:
                        sender.PostQuit();
                        break;
                    case LLGL.Key.F1:
                        App.TriggerFrameProfiling();
                        break;
                }
            }

            public override void OnResize(LLGL.Window sender, LLGL.Extent2D clientAreaSize)
            {
                App.OnResize(clientAreaSize);
                App.OnDrawFrame();
            }
        };

        protected static readonly LLGL.Color bgColor = new LLGL.Color(0.1f, 0.1f, 0.2f);
        protected static readonly LLGL.Extent2D initialResolution = new LLGL.Extent2D(800, 600);

        protected LLGL.RenderingDebugger Debugger { get; private set; }
        protected LLGL.RenderSystem Renderer { get; private set; }
        protected LLGL.CommandBuffer CmdBuffer { get; private set; }
        protected LLGL.SwapChain SwapChain { get; private set; }
        protected LLGL.Viewport FullViewport
        {
            get
            {
                var resolution = SwapChain.Resolution;
                return new LLGL.Viewport(0, 0, (float)resolution.Width, (float)resolution.Height);
            }
        }

        private static void PrintRendererInfo(LLGL.RendererInfo info)
        {
            Console.WriteLine($"Renderer:         {info.RendererName}");
            Console.WriteLine($"Device:           {info.DeviceName}");
            Console.WriteLine($"Vendor:           {info.VendorName}");
            Console.WriteLine($"Shading Language: {info.ShadingLanguageName}");
        }

        static string GetSelectedRendererModule(string[] args)
        {
            foreach (string arg in args)
            {
                switch (arg)
                {
                    case "d3d11": return "Direct3D11";
                    case "d3d12": return "Direct3D12";
                    case "gl": return "OpenGL";
                    case "vk": return "Vulkan";
                }
            }
            return "Direct3D11";
        }

        public int Run(string title, string[] args)
        {
            // Configure example by input arguments
            Func<string, bool> HasArgument = (string search) =>
            {
                foreach (string arg in args)
                {
                    if (arg == search)
                    {
                        return true;
                    }
                }
                return false;
            };

            string rendererModule = ExampleBase.GetSelectedRendererModule(args);
            if (HasArgument("-d") || HasArgument("--debug"))
            {
                Debugger = new LLGL.RenderingDebugger();
            }

            var report = new LLGL.Report();
            Renderer = LLGL.RenderSystem.Load(new LLGL.RenderSystemDescriptor(rendererModule, debugger: Debugger), report);
            if (Renderer == null)
            {
                Console.Write(report.ToString());
                return 1;
            }

            var swapChainDesc = new LLGL.SwapChainDescriptor(resolution: initialResolution, samples: 8);
            swapChainDesc.DebugName = "MySwapChain";
            SwapChain = Renderer.CreateSwapChain(swapChainDesc);
            SwapChain.SetVsyncInterval(1);

            ExampleBase.PrintRendererInfo(Renderer.RendererInfo);

            // Create command buffer
            CmdBuffer = Renderer.CreateCommandBuffer(new LLGL.CommandBufferDescriptor() { Flags = LLGL.CommandBufferFlags.ImmediateSubmit });
            CmdBuffer.DebugName = "MyCommandBuffer";

            // Create resources
            OnInitialize();

            // Change window title and register event listener
            var window = SwapChain.Surface.AsWindow();
            window.AddEventListener(new MyEventListener(this));

            window.Title = $"LLGL C# Example: {title} ( {Renderer.RendererInfo.RendererName} )";
            window.Show();

            // Main loop
            while (LLGL.Surface.ProcessEvents() && !window.HasQuit)
            {
                if (profileNextFrame)
                {
                    // Record frame timings
                    Debugger.TimeRecording = true;
                    OnDrawFrame();
                    SwapChain.Present();
                    Debugger.TimeRecording = false;

                    // Print frame profile results to console
                    PrintFrameProfile(Debugger.FlushProfile());
                    profileNextFrame = false;
                }
                else
                {
                    OnDrawFrame();
                    SwapChain.Present();
                }
                ++frameNo;
            }

            return 0;
        }

        private bool profileNextFrame = false;
        private int frameNo = 0;
        public void TriggerFrameProfiling()
        {
            if (Debugger != null)
            {
                profileNextFrame = true;
            }
            else
            {
                Console.WriteLine("Cannot start frame profiling without debugger. Start with -d or --debug option.");
            }
        }

        private void PrintFrameProfile(LLGL.FrameProfile frameProfile)
        {
            Console.WriteLine("");
            Console.WriteLine($"FRAME {frameNo}:");
            Console.WriteLine(String.Format("{0,30} {1,-10} {2,-10}", "COMMAND", "TIMESTAMP", "DURATION"));
            Console.WriteLine(String.Format("{0,30} {1,-10} {2,-10}", "-------", "---------", "--------"));
            double ticksToMilliseconds = 1000.0 / (double)LLGL.Timer.Frequency;
            foreach (LLGL.ProfileTimeRecord record in frameProfile.TimeRecords)
            {
                double timestamp = (double)record.CPUTicksStart * ticksToMilliseconds;
                double duration = (double)(record.CPUTicksEnd - record.CPUTicksStart) * ticksToMilliseconds;
                Console.WriteLine(String.Format("{0,30} {1,-10} {2,-10}", $"{(string)record.Annotation}()", $"{timestamp} ms", $"{duration} ms"));
            }
        }

        protected abstract void OnInitialize();

        public abstract void OnDrawFrame();

        public virtual void OnResize(LLGL.Extent2D resolution) { }
    }
}




// ================================================================================
