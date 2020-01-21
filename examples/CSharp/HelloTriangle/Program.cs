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
        private readonly RenderingDebugger debugger = new RenderingDebugger();
        private RenderSystem renderer;

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
                    contextDesc.Samples                     = 8;
                }
                var context = renderer.CreateRenderContext(contextDesc);

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
                vertexFormat.AppendAttribute(new VertexAttribute("coord", Format.RGBA32Float, 0));
                vertexFormat.AppendAttribute(new VertexAttribute("color", Format.RGBA8UNorm, 1));

                var vertices = new []
                {
                    new Vertex{ x =  0.0f, y =  0.5f, z = 0.0f, w = 1.0f, r = 255, g =   0, b =   0, a = 255 },
                    new Vertex{ x =  0.5f, y = -0.5f, z = 0.0f, w = 1.0f, r =   0, g = 255, b =   0, a = 255 },
                    new Vertex{ x = -0.5f, y = -0.5f, z = 0.0f, w = 1.0f, r =   0, g =   0, b = 255, a = 255 },
                };

                var vertexBufferDesc = new BufferDescriptor();
                {
                    vertexBufferDesc.BindFlags      = BindFlags.VertexBuffer;
                    vertexBufferDesc.Size           = vertexFormat.Attributes[0].Stride * (ulong)vertices.Length;
                    vertexBufferDesc.VertexAttribs  = vertexFormat.Attributes;
                }
                var vertexBuffer = renderer.CreateBuffer(vertexBufferDesc, vertices);

                // Create shaders
                var vsDesc = new ShaderDescriptor(
                    type: ShaderType.Vertex,
                    sourceType: ShaderSourceType.CodeString,
                    source: @"
                        #version 330 core
                        in vec4 coord;
                        in vec4 color;
                        out vec4 vColor;
                        void main() {
                            gl_Position = coord;
                            vColor = color;
                        }
                    "
                );

                vsDesc.Vertex.InputAttribs = vertexFormat.Attributes;

                var fsDesc = new ShaderDescriptor(
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
                );

                var vertShader = renderer.CreateShader(vsDesc);
                var fragShader = renderer.CreateShader(fsDesc);

                var shaderProgramDesc = new ShaderProgramDescriptor();
                {
                    shaderProgramDesc.VertexShader      = vertShader;
                    shaderProgramDesc.FragmentShader    = fragShader;
                }
                var shaderProgram = renderer.CreateShaderProgram(shaderProgramDesc);

                if (shaderProgram.HasErrors)
                    throw new System.IO.InvalidDataException(shaderProgram.Report);

                // Create graphics pipeline
                var pipelineDesc = new GraphicsPipelineDescriptor();
                {
                    pipelineDesc.ShaderProgram                  = shaderProgram;
                    pipelineDesc.Rasterizer.MultiSampleEnabled  = true;
                }
                var pipeline = renderer.CreatePipelineState(pipelineDesc);

                // Get command queue
                var cmdQueue = renderer.CommandQueue;
                var cmdBuffer = renderer.CreateCommandBuffer();

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
                            cmdBuffer.SetPipelineState(pipeline);
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

    static class Program
    {
        static void Main(string[] args)
        {
            var example = new HelloTriangle();
            example.Run();
        }
    }
}
