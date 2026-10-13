/*
 * Texturing.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using LLGL;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Net.Http;
using System.Numerics;
using System.Runtime.InteropServices;

namespace LLGLExamples
{
    public static class ImageLoader
    {
        // Best practice: reuse a single HttpClient instance across your application
        private static readonly HttpClient HttpClient = new HttpClient();

        /// <summary>
        /// Downloads an image from a URL and extracts its raw pixel byte array, width, and height.
        /// </summary>
        public static byte[] DownloadImage(string url, out int width, out int height)
        {
            // 1. Download the compressed file bytes
            byte[] fileBytes = HttpClient.GetByteArrayAsync(url).Result;

            using (var memoryStream = new MemoryStream(fileBytes))
            {
                using (var bitmap = new Bitmap(memoryStream))
                {
                    width = bitmap.Width;
                    height = bitmap.Height;

                    var rect = new Rectangle(0, 0, width, height);

                    // Lock the bitmap bits into system memory (standard 32-bit ARGB format)
                    var bitmapData = bitmap.LockBits(rect, ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);

                    try
                    {
                        // Calculate total byte size (accounting for potential row stride padding)
                        int byteCount = Math.Abs(bitmapData.Stride) * height;
                        byte[] pixelBytes = new byte[byteCount];

                        // Copy the raw pixel data from unmanaged memory (Scan0) to a managed byte array
                        Marshal.Copy(bitmapData.Scan0, pixelBytes, 0, byteCount);

                        return pixelBytes;
                    }
                    finally
                    {
                        // Always unlock bits to prevent memory leaks/locks
                        bitmap.UnlockBits(bitmapData);
                    }
                }
            }
        }
    }

    public class Texturing : ExampleBase
    {
        static readonly string imageURL = "https://picsum.photos/256/256";

        struct Vertex
        {
            public float x, y, z;
            public float nx, ny, nz;
            public float u, v;
        };

        struct SceneView
        {
            public Matrix4x4 wvpMatrix;
            public Matrix4x4 wMatrix;
            public Vector4 lightVec;
        };

        SceneView sceneView = new SceneView();

        Model Model = new Model();

        LLGL.Buffer VertexBuffer { get; set; }
        LLGL.Buffer IndexBuffer { get; set; }
        LLGL.Buffer SceneViewCbuffer { get; set; }
        LLGL.Texture ColorMap { get; set; }
        LLGL.Sampler ColorMapSampler { get; set; }
        LLGL.Shader VS { get; set; }
        LLGL.Shader FS { get; set; }
        LLGL.PipelineLayout PSOLayout { get; set; }
        LLGL.PipelineState PSO { get; set; }
        LLGL.VertexAttribute[] VertexAttribs { get; set; }

        static IndexedTriangelMesh<Vertex, UInt16> CreateCubeMesh(float r)
        {
            var mesh = new IndexedTriangelMesh<Vertex, UInt16>();

            mesh.Vertices.AddRange(new Vertex[]{
                // front
                new Vertex() { x = -1, y = -1, z = -1,   nx =  0, ny =  0, nz = -1,   u = 0, v = 1 },
                new Vertex() { x = -1, y =  1, z = -1,   nx =  0, ny =  0, nz = -1,   u = 0, v = 0 },
                new Vertex() { x =  1, y =  1, z = -1,   nx =  0, ny =  0, nz = -1,   u = 1, v = 0 },
                new Vertex() { x =  1, y = -1, z = -1,   nx =  0, ny =  0, nz = -1,   u = 1, v = 1 },

                // right
                new Vertex() { x = +1, y = -1, z = -1,   nx = +1, ny =  0, nz =  0,   u = 0, v = 1 },
                new Vertex() { x = +1, y = +1, z = -1,   nx = +1, ny =  0, nz =  0,   u = 0, v = 0 },
                new Vertex() { x = +1, y = +1, z = +1,   nx = +1, ny =  0, nz =  0,   u = 1, v = 0 },
                new Vertex() { x = +1, y = -1, z = +1,   nx = +1, ny =  0, nz =  0,   u = 1, v = 1 },

                // left
                new Vertex() { x = -1, y = -1, z = +1,   nx = -1, ny =  0, nz =  0,   u = 0, v = 1 },
                new Vertex() { x = -1, y = +1, z = +1,   nx = -1, ny =  0, nz =  0,   u = 0, v = 0 },
                new Vertex() { x = -1, y = +1, z = -1,   nx = -1, ny =  0, nz =  0,   u = 1, v = 0 },
                new Vertex() { x = -1, y = -1, z = -1,   nx = -1, ny =  0, nz =  0,   u = 1, v = 1 },

                // top
                new Vertex() { x = -1, y = +1, z = -1,   nx =  0, ny = +1, nz =  0,   u = 0, v = 1 },
                new Vertex() { x = -1, y = +1, z = +1,   nx =  0, ny = +1, nz =  0,   u = 0, v = 0 },
                new Vertex() { x = +1, y = +1, z = +1,   nx =  0, ny = +1, nz =  0,   u = 1, v = 0 },
                new Vertex() { x = +1, y = +1, z = -1,   nx =  0, ny = +1, nz =  0,   u = 1, v = 1 },

                // bottom
                new Vertex() { x = -1, y = -1, z = +1,   nx =  0, ny = -1, nz =  0,   u = 0, v = 1 },
                new Vertex() { x = -1, y = -1, z = -1,   nx =  0, ny = -1, nz =  0,   u = 0, v = 0 },
                new Vertex() { x = +1, y = -1, z = -1,   nx =  0, ny = -1, nz =  0,   u = 1, v = 0 },
                new Vertex() { x = +1, y = -1, z = +1,   nx =  0, ny = -1, nz =  0,   u = 1, v = 1 },

                // back
                new Vertex() { x = +1, y = -1, z = +1,   nx =  0, ny =  0, nz = +1,   u = 0, v = 1 },
                new Vertex() { x = +1, y = +1, z = +1,   nx =  0, ny =  0, nz = +1,   u = 0, v = 0 },
                new Vertex() { x = -1, y = +1, z = +1,   nx =  0, ny =  0, nz = +1,   u = 1, v = 0 },
                new Vertex() { x = -1, y = -1, z = +1,   nx =  0, ny =  0, nz = +1,   u = 1, v = 1 },
            });

            mesh.Indices.AddRange(new UInt16[]{
                 0,  1,  2,  0,  2,  3, // front
                 4,  5,  6,  4,  6,  7, // right
                 8,  9, 10,  8, 10, 11, // left
                12, 13, 14, 12, 14, 15, // top
                16, 17, 18, 16, 18, 19, // bottom
                20, 21, 22, 20, 22, 23, // back
            });

            return mesh;
        }

        void CreateBuffers()
        {
            var cubeMesh = CreateCubeMesh(1.0f);

            unsafe
            {
                // Create vertex buffer and initialize with vertices
                fixed (Vertex* verticesPtr = cubeMesh.Vertices.ToArray())
                {
                    var vbufferDesc = new LLGL.BufferDescriptor()
                    {
                        DebugName = "VertexBuffer",
                        Size = sizeof(Vertex) * cubeMesh.Vertices.Count,
                        BindFlags = LLGL.BindFlags.VertexBuffer,
                        Stride = sizeof(Vertex)
                    };
                    VertexBuffer = Renderer.CreateBufferUnsafe(vbufferDesc, verticesPtr);
                }

                // Create index buffer and initialize with vertices
                fixed (UInt16* indicesPtr = cubeMesh.Indices.ToArray())
                {
                    var ibufferDesc = new LLGL.BufferDescriptor()
                    {
                        DebugName = "IndexBuffer",
                        Size = sizeof(UInt16) * cubeMesh.Indices.Count,
                        BindFlags = LLGL.BindFlags.IndexBuffer,
                        Format = LLGL.Format.R16UInt
                    };
                    IndexBuffer = Renderer.CreateBufferUnsafe(ibufferDesc, indicesPtr);
                }

                // Create scene view constant buffer
                var cbufferDesc = new LLGL.BufferDescriptor()
                {
                    DebugName = "SceneView.Cbuffer",
                    Size = sizeof(SceneView),
                    BindFlags = BindFlags.ConstantBuffer
                };
                SceneViewCbuffer = Renderer.CreateBuffer(cbufferDesc);
            }

            Model.NumVertices = cubeMesh.Vertices.Count;
            Model.NumIndices = cubeMesh.Indices.Count;
        }

        void CreateTextures()
        {
            int width = 0, height = 0;
            byte[] imageData = ImageLoader.DownloadImage(imageURL, out width, out height);

            var imageView = new LLGL.ImageView(LLGL.ImageFormat.BGRA, LLGL.DataType.UInt8, imageData);

            var colorMapDesc = new LLGL.TextureDescriptor();
            colorMapDesc.Extent = new LLGL.Extent3D(width, height, 1);
            ColorMap = Renderer.CreateTexture(colorMapDesc, imageView);

            // Create sampler state
            var samplerDesc = new LLGL.SamplerDescriptor();
            samplerDesc.MinFilter = LLGL.SamplerFilter.Linear;
            samplerDesc.MagFilter = LLGL.SamplerFilter.Linear;
            ColorMapSampler = Renderer.CreateSampler(samplerDesc);
        }

        void CreateShaders()
        {
            const string shaderSource =
                @"
                cbuffer SceneView : register(b0) {
                    float4x4 wvpMatrix;
                    float4x4 wMatrix;
                    float4   lightVec;
                }
                struct VertexIn {
                    float3 position : POSITION;
                    float3 normal   : NORMAL;
                    float2 texCoord : TEXCOORD;
                };
                struct VertexOut {
                    float4 position : SV_Position;
                    float3 normal   : NORMAL;
                    float2 texCoord : TEXCOORD;
                };
                void VSMain(VertexIn inp, out VertexOut outp) {
                    outp.position = mul(wvpMatrix, float4(inp.position.xyz, 1));
                    outp.normal   = mul(wMatrix, float4(inp.normal, 0)).xyz;
                    outp.texCoord = inp.texCoord;
                }
                Texture2D colorMap : register(t0);
                SamplerState colorMapSampler : register(s0);
                float4 PSMain(VertexOut inp) : SV_Target {
                    float NdotL = saturate(dot(normalize(inp.normal), lightVec.xyz));
                    float4 color = colorMap.Sample(colorMapSampler, inp.texCoord);
                    return float4(color.rgb * NdotL, color.a);
                }
                "
            ;

            var psoLayoutDesc = new LLGL.PipelineLayoutDescriptor();
            psoLayoutDesc.Bindings = new LLGL.BindingDescriptor[]
            {
                new LLGL.BindingDescriptor("SceneView",       LLGL.ResourceType.Buffer,  LLGL.BindFlags.ConstantBuffer, LLGL.StageFlags.FragmentStage | LLGL.StageFlags.VertexStage,   new LLGL.BindingSlot(0)),
                new LLGL.BindingDescriptor("colorMap",        LLGL.ResourceType.Texture, LLGL.BindFlags.Sampled,        LLGL.StageFlags.FragmentStage, new LLGL.BindingSlot(0)),
                new LLGL.BindingDescriptor("colorMapSampler", LLGL.ResourceType.Sampler, 0,                             LLGL.StageFlags.FragmentStage, new LLGL.BindingSlot(0)),
            };
            PSOLayout = Renderer.CreatePipelineLayout(psoLayoutDesc);

            // Create vertex shader
            var vsDesc = new LLGL.ShaderDescriptor(
                type: LLGL.ShaderType.Vertex,
                sourceText: shaderSource,
                sourceType: LLGL.ShaderSourceType.CodeString,
                entryPoint: "VSMain",
                profile: "vs_5_0",
                name: "MyVertexShader"
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

        void CreatePipeline()
        {
            unsafe
            {
                VertexAttribs = new LLGL.VertexAttribute[3]
                {
                    new LLGL.VertexAttribute("position", format: LLGL.Format.RGB32Float, location: 0, offset: 0,               stride: sizeof(Vertex)),
                    new LLGL.VertexAttribute("normal",   format: LLGL.Format.RGB32Float, location: 1, offset: sizeof(float)*3, stride: sizeof(Vertex)),
                    new LLGL.VertexAttribute("texCoord", format: LLGL.Format.RG32Float,  location: 2, offset: sizeof(float)*6, stride: sizeof(Vertex)),
                };
            }

            var psoDesc = new LLGL.GraphicsPipelineDescriptor()
            {
                DebugName = "MyGraphicsPSO",
                PipelineLayout = PSOLayout,
                RenderPass = SwapChain.RenderPass,
                InputVertexAttribs = VertexAttribs,
                VertexShader = VS,
                FragmentShader = FS,
                PrimitiveTopology = LLGL.PrimitiveTopology.TriangleList,
                Rasterizer = new LLGL.RasterizerDescriptor() { CullMode = LLGL.CullMode.Back }
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

        void UpdateSceneView(float dt)
        {
            LLGL.Extent2D resolution = SwapChain.Resolution;

            Matrix4x4 pMatrix = CreatePerspectiveFieldOfViewLeftHanded(
                fieldOfView: 45.0f * (float)Math.PI / 180.0f,
                aspectRatio: (float)resolution.Width / (float)resolution.Height,
                nearPlaneDistance: 0.1f,
                farPlaneDistance: 100.0f
            );

            Matrix4x4 vMatrix = Matrix4x4.Identity;
            vMatrix.Translation = new Vector3(0, 0, -5);
            Matrix4x4.Invert(vMatrix, out vMatrix);

            Model.Transform.Turn(new Vector3(0, 1, 0), dt);

            sceneView.wMatrix = Model.Transform.Matrix;
            sceneView.wvpMatrix = sceneView.wMatrix * vMatrix * pMatrix;
            sceneView.lightVec = Vector4.Normalize(new Vector4(0, 0, -1, 0));
        }

        public override void OnDrawFrame(float dt)
        {
            UpdateSceneView(dt);

            CmdBuffer.Begin();
            {
                unsafe
                {
                    fixed (SceneView* ptr = &sceneView)
                    {
                        CmdBuffer.UpdateBufferUnsafe(SceneViewCbuffer, 0, ptr, sizeof(SceneView));
                    }
                }

                CmdBuffer.SetVertexBuffer(VertexBuffer);
                CmdBuffer.SetIndexBuffer(IndexBuffer);

                CmdBuffer.BeginRenderPass(SwapChain);
                {
                    CmdBuffer.Clear(LLGL.ClearFlags.Color, new LLGL.ClearValue(bgColor));

                    CmdBuffer.SetPipelineState(PSO);
                    CmdBuffer.SetResource(0, SceneViewCbuffer);
                    CmdBuffer.SetResource(1, ColorMap);
                    CmdBuffer.SetResource(2, ColorMapSampler);

                    CmdBuffer.SetViewport(FullViewport);

                    CmdBuffer.DrawIndexed(Model.NumIndices, 0);
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
