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
        public Vertex(float x, float y, float u, float v)
        {
            this.x = x;
            this.y = y;
            this.u = u;
            this.v = v;
        }
        public float x, y;
        public float u, v;
    }

    struct RGBA
    {
        public RGBA(byte r, byte g, byte b, byte a)
        {
            this.r = r;
            this.g = g;
            this.b = b;
            this.a = a;
        }
        public byte r, g, b, a;
    };

    class Texturing
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

                window.Title = $"LLGL for C# - Texturing ( {renderer.Name} )";

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
                vertexFormat.AppendAttribute(new VertexAttribute("coord",    Format.RG32Float, 0));
                vertexFormat.AppendAttribute(new VertexAttribute("texCoord", Format.RG32Float, 1));

                const float uvScale = 10.0f;

                var vertices = new []
                {
                    new Vertex(-0.5f, -0.5f, 0.0f, uvScale),
                    new Vertex(-0.5f, +0.5f, 0.0f, 0.0f),
                    new Vertex(+0.5f, -0.5f, uvScale, uvScale),
                    new Vertex(+0.5f, +0.5f, uvScale, 0.0f),
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
                        in vec2 coord;
                        in vec2 texCoord;
                        out vec2 vTexCoord;
                        void main() {
                            gl_Position = vec4(coord, 0, 1);
                            vTexCoord = texCoord;
                        }
                    "
                );

                vsDesc.Vertex.InputAttribs = vertexFormat.Attributes;

                var fsDesc = new ShaderDescriptor
                (
                    type: ShaderType.Fragment,
                    sourceType: ShaderSourceType.CodeString,
                    source: @"
                        #version 330 core
                        in vec2 vTexCoord;
                        out vec4 fColor;
                        uniform sampler2D tex;
                        void main() {
                            fColor = texture(tex, vTexCoord);
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

                // Create pipeline layout
                var pipelineLayout = renderer.CreatePipelineLayout("texture(0):frag, sampler(0):frag");

                // Create graphics pipeline
                var pipelineDesc = new GraphicsPipelineDescriptor();
                {
                    pipelineDesc.ShaderProgram                  = shaderProgram;
                    pipelineDesc.PipelineLayout                 = pipelineLayout;
                    pipelineDesc.PrimitiveTopology              = PrimitiveTopology.TriangleStrip;
                    pipelineDesc.Blend.Targets[0].BlendEnabled  = true;
                    pipelineDesc.Rasterizer.MultiSampleEnabled  = true;
                }
                var pipeline = renderer.CreatePipelineState(pipelineDesc);

                // Create texture
                var imageDesc = new SrcImageDescriptor<RGBA>();
                {
                    imageDesc.Format    = ImageFormat.RGBA;
                    imageDesc.DataType  = DataType.UInt8;
                    imageDesc.Data      = new RGBA[4];
                    imageDesc.Data[0]   = new RGBA(255,   0,   0, 255);
                    imageDesc.Data[1]   = new RGBA(  0, 255,   0, 255);
                    imageDesc.Data[2]   = new RGBA(  0,   0,   0,   0);
                    imageDesc.Data[3]   = new RGBA(  0,   0,   0,   0);
                }
                var textureDesc = new TextureDescriptor();
                {
                    textureDesc.Type    = TextureType.Texture2D;
                    textureDesc.Extent  = new Extent3D(2, 2, 1);
                }
                var texture = renderer.CreateTexture(textureDesc, imageDesc);

                // Create sampler
                var samplerDesc = new SamplerDescriptor();
                {
                    samplerDesc.MagFilter = SamplerFilter.Nearest;
                }
                var sampler = renderer.CreateSampler(samplerDesc);

                // Create resource heap
                var resourceHeapDesc = new ResourceHeapDescriptor();
                {
                    resourceHeapDesc.PipelineLayout = pipelineLayout;
                    resourceHeapDesc.ResourceViews.Add(new ResourceViewDescriptor(texture));
                    resourceHeapDesc.ResourceViews.Add(new ResourceViewDescriptor(sampler));
                }
                var resourceHeap = renderer.CreateResourceHeap(resourceHeapDesc);

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
                            cmdBuffer.SetViewport(new Viewport(0, 0, context.Resolution.Width, context.Resolution.Height));

                            cmdBuffer.SetPipelineState(pipeline);
                            cmdBuffer.SetResourceHeap(resourceHeap);

                            cmdBuffer.Draw(4, 0);
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
            var example = new Texturing();
            example.Run();
        }
    }
}
