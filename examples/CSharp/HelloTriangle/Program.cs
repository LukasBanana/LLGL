using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

using SharpLLGL;

namespace LLGLExamples
{
    struct Vertex
    {
        public float x, y, z, w;
        public byte r, g, b, a;
    }

    class HelloTriangle
    {
        private RenderingDebugger debugger = new RenderingDebugger();
        private RenderSystem renderer;
        private RenderContext context;
        private CommandQueue cmdQueue;
        private CommandBuffer cmdBuffer;
        private GraphicsPipeline pipeline;

        public void Run()
        {

            try
            {
                // Load renderer
                renderer = RenderSystem.Load("OpenGL", debugger);

                // Create render context
                var contextDesc = new RenderContextDescriptor();
                {
                    contextDesc.VideoMode.Resolution.Width  = 800;
                    contextDesc.VideoMode.Resolution.Height = 600;
                    contextDesc.VideoMode.ColorBits         = 32;
                    contextDesc.VideoMode.DepthBits         = 24;
                    contextDesc.VideoMode.StencilBits       = 8;
                }
                context = renderer.CreateRenderContext(contextDesc);

                // Get context window
                var window = context.Surface;
                window.Shown = true;

                window.Title = $"LLGL for C# - HelloTriangle ( {renderer.Name} )";

                // Print renderer information
                Console.WriteLine("Renderer Info:");
                var info = renderer.Info;
                {
                    Console.WriteLine($"  Renderer:         {info.RendererName}");
                    Console.WriteLine($"  Device:           {info.DeviceName}");
                    Console.WriteLine($"  Vendor:           {info.VendorName}");
                    Console.WriteLine($"  Shading Language: {info.ShadingLanguageName}");
                }

                // Create vertex buffer
                var vertexFormat = new VertexFormat();
                vertexFormat.AppendAttribute(new VertexAttribute("coord", Format.RGBA32Float));
                vertexFormat.AppendAttribute(new VertexAttribute("color", Format.RGBA8UNorm));

                var vertices = new Vertex[]
                {
                    new Vertex{ x =  0.0f, y =  0.5f, z = 0.0f, w = 1.0f, r = 255, g =   0, b =   0, a = 255 },
                    new Vertex{ x =  0.5f, y = -0.5f, z = 0.0f, w = 1.0f, r =   0, g = 255, b =   0, a = 255 },
                    new Vertex{ x = -0.5f, y = -0.5f, z = 0.0f, w = 1.0f, r =   0, g =   0, b = 255, a = 255 },
                };

                var vertexBufferDesc = new BufferDescriptor();
                {
                    vertexBufferDesc.BindFlags              = BindFlags.VertexBuffer;
                    vertexBufferDesc.Size                   = vertexFormat.Stride * (ulong)vertices.Length;
                    vertexBufferDesc.VertexBuffer.Format    = vertexFormat;
                }
                var vertexBuffer = renderer.CreateBuffer(vertexBufferDesc, vertices);

                // Create shaders
                var vertShader = renderer.CreateShader(
                    new ShaderDescriptor(
                        type: ShaderType.Vertex,
                        sourceType: ShaderSourceType.CodeString,
                        source: @"
                            #version 330 core
                            in vec4 coord;
                            in vec3 color;
                            out vec4 vColor;
                            void main() {
                                gl_Position = coord;
                                vColor = vec4(color, 1);
                            }
                        "
                    )
                );
                var fragShader = renderer.CreateShader(
                    new ShaderDescriptor
                    (
                        type: ShaderType.Fragment,
                        sourceType: ShaderSourceType.CodeString,
                        source: @"
                            #version 330 core
                            in vec4 vColor;
                            out vec4 fColor;
                            void main() {
                                fColor = vColor;
                            }
                        "
                    )
                );

                var shaderProgramDesc = new ShaderProgramDescriptor();
                {
                    shaderProgramDesc.VertexFormats.Add(vertexFormat);
                    shaderProgramDesc.VertexShader      = vertShader;
                    shaderProgramDesc.FragmentShader    = fragShader;
                }
                var shaderProgram = renderer.CreateShaderProgram(shaderProgramDesc);

                if (shaderProgram.HasErrors)
                    throw new Exception(shaderProgram.Report);

                // Create graphics pipeline
                var pipelineDesc = new GraphicsPipelineDescriptor();
                {
                    pipelineDesc.ShaderProgram = shaderProgram;
                }
                pipeline = renderer.CreateGraphicsPipeline(pipelineDesc);

                // Get command queue
                cmdQueue = renderer.CommandQueue;
                cmdBuffer = renderer.CreateCommandBuffer();

                cmdBuffer.SetClearColor(0.1f, 0.1f, 0.2f, 1.0f);

                // Render loop
                while (window.ProcessEvents())
                {
                    cmdBuffer.Begin();
                    {
                        cmdBuffer.SetVertexBuffer(vertexBuffer);

                        cmdBuffer.BeginRenderPass(context);
                        {
                            cmdBuffer.Clear(ClearFlags.Color);
                            cmdBuffer.SetViewport(new Viewport(context.Resolution));

                            cmdBuffer.SetGraphicsPipeline(pipeline);

                            cmdBuffer.Draw(3, 0);
                        }
                        cmdBuffer.EndRenderPass();
                    }
                    cmdBuffer.End();
                    cmdQueue.Submit(cmdBuffer);

                    context.Present();
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
                Console.WriteLine("press any key to continue ...");
                Console.ReadKey();
            }
            finally
            {
                RenderSystem.Unload(renderer);
            }
        }
    };

    class Program
    {
        static void Main(string[] args)
        {
            var example = new HelloTriangle();
            example.Run();
        }
    }
}
