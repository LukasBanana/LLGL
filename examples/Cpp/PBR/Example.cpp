/*
 * Example.cpp (Example_PBR)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>
#include <FileUtils.h>
#include <ImageReader.h>
#include <LLGL/Utils/Image.h>


class Example_PBR : public ExampleBase
{

    LLGL::Buffer*               vertexBuffer            = nullptr;
    LLGL::Buffer*               sceneViewCbuffer        = nullptr;

    ShaderPipeline              shaderPipelineMeshes;
    LLGL::PipelineLayout*       layoutMeshes            = nullptr;
    LLGL::PipelineState*        pipelineMeshes          = nullptr;

    ShaderPipeline              shaderPipelineSky;
    LLGL::PipelineLayout*       layoutSky               = nullptr;
    LLGL::PipelineState*        pipelineSky             = nullptr;

    LLGL::Texture*              skyboxCubemap           = nullptr;
    LLGL::Texture*              defaultWhiteTex         = nullptr; // Default 1x1 texture with (1.0, 1.0, 1.0, 1.0) color value.
    LLGL::Texture*              defaultBlackTex         = nullptr; // Default 1x1 texture with (0.0, 0.0, 0.0, 1.0) color value.
    LLGL::Texture*              defaultNormalTex        = nullptr; // Default 1x1 texture with (0.5, 0.5, 1.0, 1.0) color value for a normal pointing away from the surface.

    LLGL::Sampler*              linearSampler           = nullptr;

    LLGL::ResourceHeap*         materialsResourceHeap   = nullptr;

    struct PBRMaterial
    {
        LLGL::Texture* colorMap     = nullptr;
        LLGL::Texture* normalMap    = nullptr;
        LLGL::Texture* roughnessMap = nullptr;
        LLGL::Texture* metallicMap  = nullptr;
    };

    struct Model
    {
        TriangleMesh    mesh;
        float           scale;
    };

    std::vector<PBRMaterial>    materials;
    std::vector<Model>          models;

    struct alignas(16) SceneView
    {
        Gs::Matrix4f    cMatrix;
        Gs::Matrix4f    vpMatrix;
        Gs::Matrix4f    wMatrix;
        Gs::Vector2f    aspectRatio;
        float           mipCount        = 0.0f;
        float           projZAxis       = 0.0f;
        Gs::Vector4f    lightDir        = { 0, 0, -1, 0 };
    }
    sceneView;

    struct Presentation
    {
        std::uint32_t   currentModel    = 0;
        std::uint32_t   currentMaterial = 0;
        float           viewPitch       = 0.0f;
        float           viewYaw         = 0.0f;
    }
    presentation;

public:

    Example_PBR() :
        ExampleBase { "LLGL Example: PBR" }
    {
        // Validate required rendering capabilities
        LLGL::RenderingCapabilities caps;
        {
            caps.features.hasArrayTextures      = true;
            caps.features.hasCubeArrayTextures  = true;
        }
        LLGL::ValidateRenderingCaps(renderer->GetRenderingCaps(), caps);

        // Create all graphics objects
        CreateBuffers();
        LoadShaders();
        CreatePipelines();
        LoadTextures();
        CreateResourceHeap();

        // Update vectors for projection
        sceneView.projZAxis = GetProjectionZAxis();
        sceneView.lightDir.z *= sceneView.projZAxis;

        // Print some information on the standard output
        LLGL::Log::Printf(
            "=========================================================================\n"
            "Press TAB KEY to switch through various materials\n"
            "Press TAB KEY while holding SPACE KEY to switch through various 3D models\n"
            "=========================================================================\n"
        );
    }

private:

    void CreateBuffers()
    {
        // Load 3D models
        std::vector<TexturedVertex> vertices;
        models.push_back(Model{ Load3DModel(vertices, "UVSphere.obj"), 1.00f });
        models.push_back(Model{ Load3DModel(vertices, "UVCube.obj"),   0.75f });

        // Create vertex and constant buffer
        vertexBuffer = CreateVertexBuffer(GenerateTangentSpaceVertices(vertices), sizeof(TangentSpaceVertex));
        sceneViewCbuffer = CreateConstantBuffer(sceneView);
    }

    void LoadShaders()
    {
        shaderPipelineSky.vs    = LoadVertexShader  ("Example", "VSky");
        shaderPipelineSky.ps    = LoadFragmentShader("Example", "PSky");

        shaderPipelineMeshes.vs = LoadVertexShader  ("Example", "VMesh");
        shaderPipelineMeshes.ps = LoadFragmentShader("Example", "PMesh");
    }

    void CreatePipelines()
    {
        // Create pipeline layout for skybox
        layoutSky = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(SceneView@1):frag:vert,"
                "sampler(smpl@2):frag,"
                "texture(skyBox@3):frag,"

                "sampler<skyBox, smpl>(skyBox@3),"
            )
        );

        // Vertex attributes
        const LLGL::DynamicVector<LLGL::VertexAttribute> vertexAttribs = LLGL::Parse(
            "rgb32f(position),"
            "rgb32f(normal),"
            "rgb32f(tangent),"
            "rgb32f(bitangent),"
            "rg32f(texCoord),"
        );

        // Create graphics pipeline for skybox
        LLGL::GraphicsPipelineDescriptor pipelineDescSky;
        {
            pipelineDescSky.debugName                       = "Sky.PSO";
            pipelineDescSky.inputVertexAttribs              = vertexAttribs;
            pipelineDescSky.vertexShader                    = shaderPipelineSky.vs;
            pipelineDescSky.fragmentShader                  = shaderPipelineSky.ps;
            pipelineDescSky.pipelineLayout                  = layoutSky;
            pipelineDescSky.rasterizer.multiSampleEnabled   = (GetSampleCount() > 1);

            // Use triangle winding order as indicator whether right-handed projection is used for fullscreen triangles
            pipelineDescSky.rasterizer.frontCCW             = HasRightHandedProjection();
        }
        pipelineSky = renderer->CreatePipelineState(pipelineDescSky);
        ReportPSOErrors(pipelineSky);

        // Create pipeline layout for meshes
        layoutMeshes = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(SceneView@1):frag:vert,"
                "sampler(smpl@2):frag,"
                "texture(skyBox@3):frag,"
                "heap{ texture(colorMap@4, normalMap@5, roughnessMap@6, metallicMap@7):frag },"

                "sampler<skyBox, smpl>(s_skyBoxsmpl@3),"
                "sampler<colorMap, smpl>(s_colorMapsmpl@4),"
                "sampler<normalMap, smpl>(s_normalMapsmpl@5),"
                "sampler<roughnessMap, smpl>(s_roughnessMapsmpl@6),"
                "sampler<metallicMap, smpl>(s_metallicMapsmpl@7),"
            )
        );

        // Create graphics pipeline for meshes
        LLGL::GraphicsPipelineDescriptor pipelineDescMeshes;
        {
            pipelineDescMeshes.debugName                        = "Mesh.PSO";
            pipelineDescMeshes.inputVertexAttribs               = vertexAttribs;
            pipelineDescMeshes.vertexShader                     = shaderPipelineMeshes.vs;
            pipelineDescMeshes.fragmentShader                   = shaderPipelineMeshes.ps;
            pipelineDescMeshes.pipelineLayout                   = layoutMeshes;
            pipelineDescMeshes.depth.testEnabled                = true;
            pipelineDescMeshes.depth.writeEnabled               = true;
            pipelineDescMeshes.rasterizer.cullMode              = LLGL::CullMode::Back;
            pipelineDescMeshes.rasterizer.multiSampleEnabled    = (GetSampleCount() > 1);
        }
        pipelineMeshes = renderer->CreatePipelineState(pipelineDescMeshes);
        ReportPSOErrors(pipelineMeshes);
    }

    bool LoadTextureArrayLayer(const std::string& filename, LLGL::Extent3D& texLayerExtent, ImageReader& outImageReader)
    {
        // Print information about current texture
        LLGL::Log::Printf("Load image: \"%s\"\n", filename.c_str());

        // Load image data from file (using STBI library, see http://nothings.org/stb_image.h)
        outImageReader.LoadFromFile(filename);

        // Check if image size is compatible
        const LLGL::Extent3D& imageExtent = outImageReader.GetTextureDesc().extent;
        if (texLayerExtent.width == 0)
        {
            texLayerExtent.width    = imageExtent.width;
            texLayerExtent.height   = imageExtent.height;
            texLayerExtent.depth    = 1;
        }
        else if (imageExtent.width != texLayerExtent.width || imageExtent.height != texLayerExtent.height)
        {
            LLGL::Log::Errorf("size mismatch for texture array while loading image: \"%s\"", filename.c_str());
            return false;
        }

        return true;
    }

    // Loads multiple images into one texture array or cube-map array
    LLGL::Texture* LoadTextureArray(const LLGL::TextureType texType, const std::initializer_list<const char*>& texFilenames)
    {
        // Load image data
        LLGL::Extent3D texLayerExtent;
        std::vector<ImageReader> texLayerImages(texFilenames.size());
        std::uint32_t numImageSlices = 0;

        const std::string texBaseDir = "PBR/";
        for (std::size_t i = 0; i < texFilenames.size(); ++i)
        {
            if (LoadTextureArrayLayer(texBaseDir + *(texFilenames.begin() + i), texLayerExtent, texLayerImages[i]))
                ++numImageSlices;
        }

        // Create texture
        LLGL::TextureDescriptor texDesc;
        {
            texDesc.type            = texType;
            texDesc.format          = LLGL::Format::RGBA8UNorm;
            texDesc.extent          = texLayerExtent;
            texDesc.arrayLayers     = numImageSlices;
            texDesc.miscFlags       = LLGL::MiscFlags::NoInitialData;
        }
        LLGL::Texture* cubeOrArrayTexture = renderer->CreateTexture(texDesc);

        // Write texture layers one by one.
        // This is to support WebGL, which is ristricted in consecutive memory blocks.
        LLGL::TextureRegion texRegion;
        texRegion.extent = texLayerExtent;

        for (std::uint32_t layer = 0; layer < texDesc.arrayLayers; ++layer)
        {
            texRegion.subresource.baseArrayLayer = layer;
            renderer->WriteTexture(*cubeOrArrayTexture, texRegion, texLayerImages[layer].GetImageView());
        }

        // Generate MIP-maps
        commands->Begin();
        commands->GenerateMips(*cubeOrArrayTexture);
        commands->End();
        commandQueue->Submit(*commands);

        return cubeOrArrayTexture;
    }

    LLGL::Texture* LoadTextureOrDefault(const std::string& filename, LLGL::Texture* defaultTexture = nullptr)
    {
        return (FindAsset(filename) ? LoadTexture(filename) : defaultTexture);
    }

    void LoadMaterial(const std::string& basename)
    {
        const std::string texBaseDir = "PBR/";
        PBRMaterial newMaterial;
        newMaterial.colorMap        = LoadTextureOrDefault(texBaseDir + basename + '/' + basename + "_col.jpg", defaultWhiteTex);
        newMaterial.normalMap       = LoadTextureOrDefault(texBaseDir + basename + '/' + basename + "_nrm.jpg", defaultNormalTex);
        newMaterial.roughnessMap    = LoadTextureOrDefault(texBaseDir + basename + '/' + basename + "_rgh.jpg", defaultBlackTex);
        newMaterial.metallicMap     = LoadTextureOrDefault(texBaseDir + basename + '/' + basename + "_met.jpg", defaultBlackTex);
        materials.push_back(newMaterial);
    }

    LLGL::Texture* CreateDefaultTexture(const LLGL::ColorRGBAf& colorValue)
    {
        LLGL::TextureDescriptor texDesc;
        {
            texDesc.debugName           = "DefaultTex2D";
            texDesc.clearValue.color[0] = colorValue.r;
            texDesc.clearValue.color[1] = colorValue.g;
            texDesc.clearValue.color[2] = colorValue.b;
            texDesc.clearValue.color[3] = colorValue.a;
        }
        return renderer->CreateTexture(texDesc);
    }

    void LoadTextures()
    {
        // Load skybox textures
        if (HasRightHandedProjection())
        {
            // We need to create a skybox that is specifically created for right-handed coordinates, otherwise it will be displayed flipped.
            // This can be done by rotating and mirroring the images as well as swapping the cube faces left with right and back with front.
            // For this example, we simply use a different skybox to show a different environment depending on whether left- or right-handed projections are used.
            // This simply hides the fact that we did not provide a right-handed skybox nor did we care to convert it.
            skyboxCubemap = LoadTextureArray(
                LLGL::TextureType::TextureCube,
                {
                    // 1st skybox "mp_hanging"
                    "mp_hanging/hangingstone_ft.tga", // X+ = interpret 'ft' as right
                    "mp_hanging/hangingstone_bk.tga", // X- = interpret 'bk' as left
                    "mp_hanging/hangingstone_up.tga", // Y+ = up
                    "mp_hanging/hangingstone_dn.tga", // Y- = down
                    "mp_hanging/hangingstone_rt.tga", // Z+ = interpret 'rt' as front (right-handed has Z+ at the front)
                    "mp_hanging/hangingstone_lf.tga", // Z- = interpret 'lf' as back (right-handed has Z- at the back)
                }
            );
        }
        else
        {
            skyboxCubemap = LoadTextureArray(
                LLGL::TextureType::TextureCube,
                {
                    // 1st skybox "mp_alpha"
                    "mp_alpha/alpha-island_rt.tga", // X+ = right
                    "mp_alpha/alpha-island_lf.tga", // X- = left
                    "mp_alpha/alpha-island_up.tga", // Y+ = up
                    "mp_alpha/alpha-island_dn.tga", // Y- = down
                    "mp_alpha/alpha-island_bk.tga", // Z+ = back
                    "mp_alpha/alpha-island_ft.tga", // Z- = front
                }
            );
        }

        // Store number of MIP-maps for environment map
        sceneView.mipCount = static_cast<float>(skyboxCubemap->GetDesc().mipLevels);

        // Create default textures
        defaultWhiteTex  = CreateDefaultTexture(LLGL::ColorRGBAf{ 1.0f, 1.0f, 1.0f, 1.0f });
        defaultBlackTex  = CreateDefaultTexture(LLGL::ColorRGBAf{ 0.0f, 0.0f, 0.0f, 1.0f });
        defaultNormalTex = CreateDefaultTexture(LLGL::ColorRGBAf{ 0.5f, 0.5f, 1.0f, 1.0f });

        // Load PBR textures
        for (const char* name : { "Tiles26", "Tiles22", "Wood13", "Metal04" })
            LoadMaterial(name);

        // Create linear sampler
        LLGL::SamplerDescriptor samplerDesc;
        {
            samplerDesc.maxAnisotropy = 8;
        }
        linearSampler = renderer->CreateSampler(samplerDesc);
    }

    void CreateResourceHeap()
    {
        // Create resource heap for meshes
        constexpr std::uint32_t numTexturesPerMaterial = 4;
        LLGL::ResourceHeapDescriptor resHeapDesc;
        {
            resHeapDesc.debugName           = "Materials.ResourceHeap";
            resHeapDesc.pipelineLayout      = layoutMeshes;
            resHeapDesc.numResourceViews    = numTexturesPerMaterial * static_cast<std::uint32_t>(materials.size());
        }
        materialsResourceHeap = renderer->CreateResourceHeap(resHeapDesc);

        // Write all mateiral textures into heap
        for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(materials.size()); ++i)
        {
            const PBRMaterial& material = materials[i];
            renderer->WriteResourceHeap(
                *materialsResourceHeap,
                i * numTexturesPerMaterial,
                { material.colorMap, material.normalMap, material.roughnessMap, material.metallicMap }
            );
        }
    }

private:

    void UpdateScene()
    {
        const float projZAxis = GetProjectionZAxis();

        // Update camera rotation
        const auto motion = input.GetMouseMotion();
        const Gs::Vector2f motionVec
        {
            static_cast<float>(motion.x),
            static_cast<float>(motion.y)
        };

        if (input.KeyPressed(LLGL::Key::LButton))
        {
            if (input.KeyPressed(LLGL::Key::Space))
            {
                // Rotate mesh
                Gs::Matrix4f& m = models[presentation.currentModel].mesh.transform;
                Gs::Matrix4f deltaRotation;
                const float rotateSpeed = 0.01f * projZAxis;
                Gs::RotateFree(deltaRotation, { 1, 0, 0 }, motionVec.y * rotateSpeed);
                Gs::RotateFree(deltaRotation, { 0, 1, 0 }, motionVec.x * rotateSpeed);
                m = deltaRotation * m;
            }
            else
            {
                // Rotate camera
                const float rotateSpeed = 0.25f;
                presentation.viewPitch  += motionVec.y * rotateSpeed;
                presentation.viewYaw    += motionVec.x * rotateSpeed;
                presentation.viewPitch = Gs::Clamp(presentation.viewPitch, -90.0f, 90.0f);
            }
        }

        // Update material and skybox layer switches
        if (input.KeyDown(LLGL::Key::Tab))
        {
            if (input.KeyPressed(LLGL::Key::Space))
            {
                if (!models.empty())
                    presentation.currentModel = (presentation.currentModel + 1) % static_cast<std::uint32_t>(models.size());
            }
            else
            {
                if (!materials.empty())
                    presentation.currentMaterial = (presentation.currentMaterial + 1) % static_cast<std::uint32_t>(materials.size());
            }
        }

        // Set camera, view-projection, and world matrix
        sceneView.cMatrix.LoadIdentity();
        Gs::RotateFree(sceneView.cMatrix, Gs::Vector3f{ 0, 1, 0 }, Gs::Deg2Rad(projZAxis * presentation.viewYaw));
        Gs::RotateFree(sceneView.cMatrix, Gs::Vector3f{ 1, 0, 0 }, Gs::Deg2Rad(projZAxis * presentation.viewPitch));
        Gs::Translate(sceneView.cMatrix, Gs::Vector3f{ 0, 0, -4 * projZAxis });

        sceneView.vpMatrix = projection * sceneView.cMatrix.Inverse();
        sceneView.wMatrix = models[presentation.currentModel].mesh.transform;
        Gs::Scale(sceneView.wMatrix, Gs::Vector3f{ models[presentation.currentModel].scale });

        sceneView.aspectRatio = { GetAspectRatio(), 1.0f };
    }

    void BindViewAndSkyboxResources()
    {
        commands->SetResource(0, *sceneViewCbuffer);
        commands->SetResource(1, *linearSampler);
        commands->SetResource(2, *skyboxCubemap);
    }

    void RenderSkybox()
    {
        commands->SetPipelineState(*pipelineSky);
        BindViewAndSkyboxResources();
        commands->Draw(3, 0);
    }

    void RenderMesh(const TriangleMesh& mesh)
    {
        commands->SetPipelineState(*pipelineMeshes);
        BindViewAndSkyboxResources();
        commands->SetResourceHeap(*materialsResourceHeap, presentation.currentMaterial);
        commands->Draw(mesh.numVertices, mesh.firstVertex);
    }

    void RenderScene()
    {
        commands->UpdateBuffer(*sceneViewCbuffer, 0, &sceneView, sizeof(sceneView));
        commands->BeginRenderPass(*swapChain);
        {
            commands->Clear(LLGL::ClearFlags::ColorDepth);
            commands->SetViewport(swapChain->GetResolution());
            commands->SetVertexBuffer(*vertexBuffer);

            RenderSkybox();
            RenderMesh(models[presentation.currentModel].mesh);
        }
        commands->EndRenderPass();
    }

    void OnDrawFrame(float dt) override
    {
        UpdateScene();

        commands->Begin();
        {
            RenderScene();
        }
        commands->End();

        commandQueue->Submit(*commands);
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_PBR);



