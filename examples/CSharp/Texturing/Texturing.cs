/*
 * Texturing.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGLExamples
{
    public class Texturing : ExampleBase
    {
        struct Vertex
        {
            public float x, y;
            public float u, v;
        };

        private LLGL.Buffer VertexBuffer { get; set; }
        private LLGL.Texture ColorMap { get; set; }
        private LLGL.Sampler ColorMapSampler { get; set; }
        private LLGL.Shader VS { get; set; }
        private LLGL.Shader FS { get; set; }
        private LLGL.PipelineLayout PSOLayout { get; set; }
        private LLGL.PipelineState PSO { get; set; }
        private LLGL.VertexAttribute[] VertexAttribs { get; set; }

        private void CreateBuffers()
        {
            const float s = 0.5f;
            const float t = 4.0f;
            var vertices = new Vertex[4]
            {
                new Vertex() { x = -s, y = +s, u = 0, v = t },
                new Vertex() { x = +s, y = +s, u = t, v = t },
                new Vertex() { x = -s, y = -s, u = 0, v = 0 },
                new Vertex() { x = +s, y = -s, u = t, v = 0 },
            };

            unsafe
            {
                VertexAttribs = new LLGL.VertexAttribute[2]
                {
                    new LLGL.VertexAttribute("position", format: LLGL.Format.RG32Float, location: 0, offset: 0,               stride: sizeof(Vertex)),
                    new LLGL.VertexAttribute("texCoord", format: LLGL.Format.RG32Float, location: 1, offset: sizeof(float)*2, stride: sizeof(Vertex)),
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

        private void CreateTextures()
        {
            var colors = new LLGL.Color[4]
            {
                new LLGL.Color(1, 0, 0, 1),
                new LLGL.Color(0, 1, 0, 1),
                new LLGL.Color(0, 0, 1, 1),
                new LLGL.Color(1, 0, 1, 1),
            };
            var imageView = new LLGL.ImageView(LLGL.ImageFormat.RGBA, LLGL.DataType.Float32, LLGL.Interop.ToBytes(colors));

            var colorMapDesc = new LLGL.TextureDescriptor();
            colorMapDesc.Extent = new LLGL.Extent3D(2, 2, 1);
            ColorMap = Renderer.CreateTexture(colorMapDesc, imageView);

            // Create sampler state
            var samplerDesc = new LLGL.SamplerDescriptor();
            samplerDesc.MinFilter = LLGL.SamplerFilter.Linear;
            samplerDesc.MagFilter = LLGL.SamplerFilter.Linear;
            ColorMapSampler = Renderer.CreateSampler(samplerDesc);
        }

        private void CreateShaders()
        {
            const string shaderSource =
                @"
                struct VertexIn {
                    float2 position : POSITION;
                    float2 texCoord : TEXCOORD;
                };
                struct VertexOut {
                    float4 position : SV_Position;
                    float2 texCoord : TEXCOORD;
                };
                void VSMain(VertexIn inp, out VertexOut outp) {
                    outp.position = float4(inp.position.xy, 0, 1);
                    outp.texCoord = inp.texCoord;
                }
                Texture2D colorMap : register(t0);
                SamplerState colorMapSampler : register(s1);
                float4 PSMain(VertexOut inp) : SV_Target {
                    float t = 0.1;
                    return smoothstep(0.5 - t, 0.5 + t, colorMap.Sample(colorMapSampler, inp.texCoord));
                }
                "
            ;

            var psoLayoutDesc = new LLGL.PipelineLayoutDescriptor();
            psoLayoutDesc.Bindings = new LLGL.BindingDescriptor[]
            {
                new LLGL.BindingDescriptor("colorMap",        LLGL.ResourceType.Texture, LLGL.BindFlags.Sampled, LLGL.StageFlags.FragmentStage, new LLGL.BindingSlot(0)),
                new LLGL.BindingDescriptor("colorMapSampler", LLGL.ResourceType.Sampler, 0,                      LLGL.StageFlags.FragmentStage, new LLGL.BindingSlot(1)),
            };
            PSOLayout = Renderer.CreatePipelineLayout(psoLayoutDesc);

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
                PipelineLayout = PSOLayout,
                RenderPass = SwapChain.RenderPass,
                VertexShader = VS,
                FragmentShader = FS,
                PrimitiveTopology = LLGL.PrimitiveTopology.TriangleStrip,
            };
            PSO = Renderer.CreatePipelineState(psoDesc);

            var psoReport = PSO.Report;
            if (psoReport != null && psoReport.HasErrors)
            {
                Console.WriteLine(psoReport.ToString());
            }
        }

        protected override void OnInitialize()
        {
            // Create resources
            CreateBuffers();
            CreateTextures();
            CreateShaders();
            CreatePipeline();
        }

        public override void OnDrawFrame()
        {
            CmdBuffer.Begin();
            {
                CmdBuffer.SetVertexBuffer(VertexBuffer);

                CmdBuffer.BeginRenderPass(SwapChain);
                {
                    CmdBuffer.Clear(LLGL.ClearFlags.Color, new LLGL.ClearValue(bgColor));

                    CmdBuffer.SetPipelineState(PSO);
                    CmdBuffer.SetResource(0, ColorMap);
                    CmdBuffer.SetResource(1, ColorMapSampler);

                    CmdBuffer.SetViewport(FullViewport);

                    CmdBuffer.Draw(4, 0);
                }
                CmdBuffer.EndRenderPass();
            }
            CmdBuffer.End();
        }

        public static int Main(string[] args)
        {
            var example = new Texturing();
            return example.Run("Texturing", args);
        }

    }
}




// ================================================================================
