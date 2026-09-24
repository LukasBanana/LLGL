/*
 * BrickPlay.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "BrickPlay.Brick.h"
#include "BrickPlay.Scene.h"
#include <ExampleBase.h>
#include <LLGL/Utils/ForRange.h>
#include <algorithm>
#include <set>
#include <map>

// File and string streams for simple scene file writer/reader
#include <fstream>
#include <sstream>


// Enables parallax occlusion displacement for rendering the plane studs.
#define ENABLE_PARALLAX_OCCLUSION 1


class Example_BrickPlay : public ExampleBase
{

    #define foreach_dimension(X, Y, Z, VEC) \
        for_range(Z, (VEC).z) for_range(Y, (VEC).y) for_range(X, (VEC).x)

    static constexpr int            boundingVolume[3]               = { 100, 60, 100 };
    static constexpr float          cameraTurnSpeed                 = 0.25f;
    static constexpr float          cameraMoveSpeed                 = 0.25f;
    static constexpr float          cameraCollisionRadius           = 1.5f;
    static constexpr float          previewBlockOffsetY             = 0.075f;               // Y-axis world position offset for the preview block before putting it into place

    static constexpr float          silhouetteColor[3]              = { 1.0f, 1.0f, 0.0f }; // Color of the silhouette outline for selectin bricks
    static constexpr float          silhouetteWidth                 = 5.0f;

    static constexpr std::uint32_t  maxUIVertexBatchSize            = 64 * 3;               // Maxmimum number of UI vertices before batch is flushed and send to the GPU.
    static constexpr int            uiColorPickerSize               = 25;                   // Size (in pixels) of each color picker button
    static constexpr int            uiColorPickerOffset[2]          = { 15, 15 };
    static constexpr int            uiButtonFrameSize               = 3;
    static constexpr int            uiButtonMargin                  = 5;
    const LLGL::ColorRGBAub         uiButtonFrameColor              = { 0, 0, 0, 255 };

    static constexpr int            uiBrickThumbnailSize            = 48;
    const LLGL::ColorRGBAub         uiBrickThumbnailBgColor         = { 180, 180, 180, 255 };
    const LLGL::ColorRGBAub         uiMenuBgColor                   = { 0, 0, 0, 180 };

    static constexpr float          uiThumbnailMinVolumeSize        = 3.0f;                 // Minimum size for a bounding volume of brick thumbnails.

    static constexpr int            planeSubdivisions               = 1;

    // Whether to use extended brick models for their optimized versions (*_WallsOnlyExt.obj).
    // If false, covered bricks can show tiny holes due to beveled edges of bricks sitting above and below them.
    // This can be turned off for mobile platforms where this flaw is virtually invisible
    // and the incurred performance cost of more almost invisible geometry can be avoided.
    static constexpr bool           useWaterTightBrickOptimization  = true;

    const char*                     sceneFilename                   = "Bricks.txt";

    enum BrickPSO
    {
        BrickPSO_Default = 0,
        BrickPSO_Transparent,
        BrickPSO_Wireframe,
        BrickPSO_Preview, // With dithering to indicate blocked position

        BrickPSO_Num,
    };

    // Shaders to render brick instances, the brick preview model, UI elements, an infinite plne for the floor, and the skybox
    LLGL::Shader*               vsBrick                         = nullptr;
    LLGL::Shader*               fsBrick                         = nullptr;
    LLGL::Shader*               vsBrickPreview                  = nullptr;
    LLGL::Shader*               fsBrickPreview                  = nullptr;
    LLGL::Shader*               vsOverlay                       = nullptr;
    LLGL::Shader*               fsOverlay                       = nullptr;
    LLGL::Shader*               vsOutline                       = nullptr;
    LLGL::Shader*               gsOutline                       = nullptr;
    LLGL::Shader*               fsOutline                       = nullptr;
    LLGL::Shader*               vsPlane                         = nullptr;
    LLGL::Shader*               fsPlane                         = nullptr;
    LLGL::Shader*               vsSkybox                        = nullptr;
    LLGL::Shader*               fsSkybox                        = nullptr;

    LLGL::PipelineLayout*       psoLayout                       = nullptr;
    LLGL::PipelineState*        pso[BrickPSO_Num]               = {};

    #if DEBUG_KDTREE
    LLGL::PipelineState*        psoDebug                        = nullptr;
    #endif

    LLGL::PipelineLayout*       psoOutlineWriteStencilLayout    = nullptr;
    LLGL::PipelineState*        psoOutlineWriteStencil          = nullptr;

    LLGL::PipelineLayout*       psoOutlineWriteColorLayout      = nullptr;
    LLGL::PipelineState*        psoOutlineWriteColor            = nullptr;

    LLGL::PipelineLayout*       psoUILayout                     = nullptr;
    LLGL::PipelineState*        psoUI                           = nullptr;

    LLGL::PipelineLayout*       psoPlaneLayout                  = nullptr;
    LLGL::PipelineState*        psoPlane[2]                     = {};

    LLGL::PipelineLayout*       psoSkyLayout                    = nullptr;
    LLGL::PipelineState*        psoSky                          = nullptr;

    LLGL::Buffer*               sceneViewCbuffer                = nullptr;
    LLGL::Buffer*               vertexAndPreviewInstanceBuffer  = nullptr; // Encodes vertices of all meshes and the preview instance
    std::uint64_t               previewInstanceBufferOffset     = 0;

    LLGL::Buffer*               uiVertexBuffer                  = nullptr;
    LLGL::Buffer*               uiViewCbuffer                   = nullptr;
    LLGL::Texture*              uiBlankTexture                  = nullptr;
    LLGL::Texture*              uiBrickThumbnailTexture         = nullptr;

    LLGL::Sampler*              uvWrapSampler                   = nullptr; // UV address wrap sampler. Anisotropic for desktop platforms.

    LLGL::Texture*              skyboxCubemap                   = nullptr;

    bool                        hasOffsetInstancing             = false;
    bool                        hasLoadedScene                  = false;

    std::unique_ptr<BrickScene> currentScene;

    using Instance          = BrickScene::Instance;
    using InstanceGroup     = BrickScene::InstanceGroup;
    using RayIntersection   = BrickScene::RayIntersection;

    Instance                    previewInstance                 = {};

    struct UIState
    {
        // To avoid competing mouse interactions between scene picking and UI elements,
        // declare a dead zone for the scene picker when the menu is active on the left and on the right.
        // This adjusted according to screen resolution when UI elements have to shift.
        int                     menuBarLeft                     = 0;
        int                     menuBarRight                    = 0;
    }
    uiState;

    const LLGL::ColorRGBAub     colorPalette[10] =
    {
        LLGL::ColorRGBAub{ 0xFF, 0xFF, 0xFF }, // White
        LLGL::ColorRGBAub{ 0x85, 0x80, 0x85 }, // Mid gray
        LLGL::ColorRGBAub{ 0x20, 0x25, 0x22 }, // Dark gray
        LLGL::ColorRGBAub{ 0xFF, 0xF0, 0x3A }, // Bright light yellow
        LLGL::ColorRGBAub{ 0x73, 0xDC, 0xA1 }, // Medium green
        LLGL::ColorRGBAub{ 0x62, 0xFA, 0x78 }, // Bright green
        LLGL::ColorRGBAub{ 0x20, 0x32, 0xB0 }, // Dark blue
        LLGL::ColorRGBAub{ 0xFF, 0x69, 0x8F }, // Coral
        LLGL::ColorRGBAub{ 0x92, 0x39, 0x78 }, // Magenta
        LLGL::ColorRGBAub{ 0xD5, 0x40, 0x40 }, // Bright red
    };

    struct alignas(16) SceneView
    {
        Gs::Matrix4f        vpMatrix;                                               // View-projection matrix
        Gs::Matrix4f        vpMatrixInv;
        Gs::Matrix4f        vpMatrixInvNoViewPos;
        Gs::Vector3f        gridScale               = Gs::Vector3f{ 1.0f };
        float               specularPower           = 90.0f;                        // Specular exponent for blinn-phone shading model
        Gs::Vector4f        viewPos;                                                // Camera view position (in world space)
        Gs::Vector4f        ambientLight            = { 0.2f, 0.2f, 0.2f, 0.0f };
        Gs::Vector4f        diffuseLight[2]         =
        {
            Gs::Vector4f{ 0.8f, 0.8f, 0.8f, 1.0f },
            Gs::Vector4f{ 0.1f, 0.1f, 0.1f, 1.0f },
        };

        // Vector for two light sources. They point from a pixel towards the light source as it is common in shading formulas.
        Gs::Vector4f        lightDir[2]             =
        {
            Gs::Vector4f{ 0.0f,  0.0f, -1.0f, 0.0f }.Normalized(),
            Gs::Vector4f{ 0.0f, -0.2f, +1.0f, 0.0f }.Normalized()
        };

        // Preview shaders only
        float               worldOffsetY            = 0.0f;
        std::int32_t        isDithering             = 0;

        // Outline shaders only
        float               viewportSize[2]         = {};
        float               outlineColor[3]         = { silhouetteColor[0], silhouetteColor[1], silhouetteColor[2] };
        float               outlineWidth            = silhouetteWidth;

        // Plane shaders only
        LLGL::ColorRGBAf    planeBaseColor;
        float               parallaxFadeRange[2]    = { 75.0f, 100.0f };
        float               studHeight              = 0.225f;
        float               pad0;
    }
    sceneView;

    struct alignas(16) UIView
    {
        Gs::Matrix4f    pMatrix;
    }
    uiView;

    struct PreviewBrick
    {
        BrickType       type            = static_cast<BrickType>(0); // Previews brick with instance index 0.
        bool            isTransparent   = false;
        bool            isBlocked       = false;
        std::int32_t    gridOffset      = 0;
    };

    struct ViewOptions
    {
        bool            showWireframe               = false;
        PreviewBrick    previewBrick;
    }
    viewOptions;

    struct PlaneVertex
    {
        Gs::Vector3f worldPos;
    };

    struct PlaneModel
    {
        static constexpr int            maxNumFaces             = planeSubdivisions * planeSubdivisions;
        static constexpr int            maxNumVertices          = maxNumFaces * 5;
        static constexpr int            maxNumIndices           = maxNumFaces * 3 * 3;
        static constexpr std::uint64_t  meshBufferIndexOffset   = sizeof(PlaneVertex) * PlaneModel::maxNumVertices;

        std::vector<PlaneVertex>    vertices;
        std::vector<std::uint16_t>  indices;

        std::uint32_t               numVertices = 0;
        std::uint32_t               numIndices  = 0;

        LLGL::Buffer*               meshBuffer  = nullptr;
        LLGL::Texture*              colorMap    = nullptr;
        LLGL::Texture*              normalMap   = nullptr;
        LLGL::Texture*              heightMap   = nullptr;
    }
    planeModel;

    // Brick vertex strucutre for brick instances. They are untextured, solid-colored bricks, so they don't need texture coordinates.
    struct BrickVertex
    {
        Gs::Vector3f position;
        Gs::Vector3f normal;
    };

    // UI element vertex structure. Pixel-based coordinates are stored as 16-bit integers since the screen won't be larger than 2^15-1 pixels.
    struct UIVertex
    {
        std::int16_t        position[2];
        std::uint8_t        texCoord[4];
        LLGL::ColorRGBAub   color;

        void Set(int x, int y, LLGL::ColorRGBAub col)
        {
            position[0] = static_cast<std::int16_t>(x);
            position[1] = static_cast<std::int16_t>(y);
            texCoord[0] = 0;
            texCoord[1] = 0;
            texCoord[2] = 0;
            texCoord[3] = 0;
            color       = col;
        }

        void Set(int x, int y, int tx, int ty, int tz, LLGL::ColorRGBAub col)
        {
            position[0] = static_cast<std::int16_t>(x);
            position[1] = static_cast<std::int16_t>(y);
            texCoord[0] = static_cast<std::uint8_t>(tx);
            texCoord[1] = static_cast<std::uint8_t>(ty);
            texCoord[2] = static_cast<std::uint8_t>(tz);
            texCoord[3] = 1;
            color       = col;
        }
    };

    static_assert(std::is_standard_layout<UIVertex>::value, "UIVertex must have standard layout in order to be passed as C-pointer to the GPU buffer");

    // Structure for UI vertex data and its vertex buffer.
    struct UIVertexBatch
    {
        std::unique_ptr<UIVertex[]> vertices;
        std::uint32_t               numBatchedVertices  = 0;

        // Last bound texture. When this changes, Draw2DImage() needs to flush the vertex buffer in order to bind a new texture for all subsequent UI vertices.
        LLGL::Texture*              cachedTexture       = nullptr;
    }
    uiVertexBatch;

    enum class CameraMode
    {
        None,
        TurnCamera,
        MoveCamera,
    };

    enum class EditMode
    {
        None,
        Add,
        Move,
        Remove,
    };

    // Determines what mode to use to render the highlighting of a selected scene object.
    enum class SelectionRenderMode
    {
        NativeLineWidth,    // Native line-width support to render silhouette
        EmulatedLineWidth,  // Emulated line-width support using a geometry shader (for D3D).
        Simplified,         // Simplified highlighting without a silhouette (for WebGL).
    };

    SelectionRenderMode selectionRenderMode = SelectionRenderMode::Simplified;

    // State tracking for user controls such as picking scene objects with the mouse pointer.
    struct Controller
    {
        CameraMode          cameraMode              = CameraMode::None;
        EditMode            editMode                = EditMode::None;

        LLGL::Offset2D      lastCursorPos;

        Brick               pickedBrick;
        LLGL::ColorRGBAub   pickedInstanceColor;

        int                 pickedColorPaletteIndex = 0;

        #if DEBUG_KDTREE
        Gs::Vector3f        pickedPosition;
        Gs::Vector3f        pickedNormal;
        #endif
    }
    controller;

    struct Camera
    {
        Gs::Vector3f    position;
        float           pitch       = 0.0f;
        float           yaw         = 0.0f;

        Gs::Matrix4f    wMatrix;
        Gs::Matrix4f    vMatrix;
        Gs::Matrix4f    vMatrixNoPosition;

        void Turn(float pitch, float yaw)
        {
            this->pitch = Gs::Clamp(this->pitch + pitch, -90.0f, +90.0f);
            this->yaw   = this->yaw + yaw;
        }

        void Move(const Gs::Vector3f& direction, const BrickScene* collisionScene = nullptr)
        {
            // Move camera into the direction, taking camera rotation into account
            Gs::Matrix3f rotation;
            Gs::RotateY(rotation, Gs::Deg2Rad(yaw));
            Gs::RotateX(rotation, Gs::Deg2Rad(pitch));
            position += rotation * direction;

            // Resolve collisions
            if (collisionScene != nullptr)
            {
                collisionScene->ResolveCollision(position, cameraCollisionRadius);
            }
        }

        void UpdateTransform()
        {
            wMatrix.LoadIdentity();
            Gs::Translate(wMatrix, position);
            Gs::RotateY(wMatrix, Gs::Deg2Rad(yaw));
            Gs::RotateX(wMatrix, Gs::Deg2Rad(pitch));

            vMatrix = wMatrix.Inverse();

            vMatrixNoPosition = Gs::Matrix4f
            {
                vMatrix(0, 0), vMatrix(0, 1), vMatrix(0, 2), 0.0f,
                vMatrix(1, 0), vMatrix(1, 1), vMatrix(1, 2), 0.0f,
                vMatrix(2, 0), vMatrix(2, 1), vMatrix(2, 2), 0.0f,
                         0.0f,          0.0f,          0.0f, 1.0f
            };
        }
    }
    camera;

public:

    Example_BrickPlay() :
        ExampleBase { "LLGL Example: Instancing Advanced" }
    {
        // Initialize kd-Tree vounding volume and camera projection parameters
        InitAppConfig();

        // Create all graphics objects
        CreateResources();
        if (!CreatePipelines())
            return;

        // Render all bricks into thumbnail render-targets for the UI
        RenderAllBrickThumbnails();

        // Load previoulsy saved scene or create default initial scene on failure.
        currentScene = std::unique_ptr<BrickScene>(new BrickScene(*renderer));
        if (LoadScene(*currentScene, sceneFilename))
            hasLoadedScene = true;
        else
            CreateDefaultScene(*currentScene);

        // Print info
        LLGL::Log::Printf(
            "Keyboard controls:\n"
            "  1 .............. Default mode\n"
            "  2 .............. Add brick mode\n"
            "  3/Delete ....... Move/Delete brick mode\n"
            "  Right mouse .... Turn camera\n"
            "  Middle mouse ... Move camera left/right & up/down\n"
            "  W/A/S/D ........ Move camera left/right & forwards/backwards\n"
            "  PageUp/-Down ... Change brick type\n"
            "  Space .......... Rotate brick\n"
        );
    }

private:

    // Returns the color palette index for the specified color or -1 if not found.
    int FindColorPaletteIndex(LLGL::ColorRGBAub color)
    {
        for (int i = 0; i < sizeof(colorPalette)/sizeof(colorPalette[0]); ++i)
        {
            if (colorPalette[i] == color)
                return i;
        }
        return -1;
    }

    // Projects the specified screen coordinate into a ray originating from the camera view to pick elements in the scene.
    Ray3f ProjectScreenPosToViewRay(LLGL::Offset2D screenPos)
    {
        const LLGL::Viewport viewport{ swapChain->GetResolution() };

        Gs::Vector4f ndc
        {
              (static_cast<float>(screenPos.x) - viewport.x) / viewport.width  * 2.0f - 1.0f,
            -((static_cast<float>(screenPos.y) - viewport.y) / viewport.height * 2.0f - 1.0f),
            1.0f,
            1.0f,
        };

        Gs::Matrix4f vpMatrixInv = projection * camera.vMatrixNoPosition;
        vpMatrixInv.MakeInverse();

        ndc = vpMatrixInv * ndc;

        return Ray3f
        {
            Gs::TransformVector(camera.wMatrix, Gs::Vector3f{}),
            Gs::Vector3f{ ndc }.Normalized()
        };
    }

    void LoadBrickModel(const char* basename, std::vector<BrickVertex>& vertices, BrickType brickType, BrickGroup brickGroup = BrickGroup_Default)
    {
        LLGL::Log::Printf("Load brick model: %s\n", basename);

        BrickMesh& mesh = BrickDictionary::Get().GetMesh(brickType, brickGroup);

        const std::vector<TexturedVertex> intermediateVertices = Load3DModel(std::string("Bricks/") + basename + ".obj");

        mesh.firstVertex = static_cast<std::uint32_t>(vertices.size());
        mesh.numVertices = static_cast<std::uint32_t>(intermediateVertices.size());

        vertices.resize(vertices.size() + intermediateVertices.size());

        auto ConvertVertex = [](BrickVertex& dst, const TexturedVertex& src) -> void
        {
            dst.position    = src.position;
            dst.normal      = src.normal;
        };

        for (std::uint32_t i = 0; i < mesh.numVertices; ++i)
            ConvertVertex(vertices[mesh.firstVertex + i], intermediateVertices[i]);
    }

    void LoadAllBrickModels(std::vector<BrickVertex>& vertices)
    {
        enum class ModelOptimization
        {
            None,
            WallsOnly,
            WallsOnlyExt,
        };

        auto LoadBrickModelGroups = [this, &vertices](const char* basename, BrickType brickType, ModelOptimization optimization = ModelOptimization::None) -> void
        {
            LoadBrickModel(basename, vertices, brickType);
            if (optimization != ModelOptimization::None)
            {
                std::string basenameWallsOnly = std::string(basename) + "_WallsOnly";
                if (useWaterTightBrickOptimization && optimization == ModelOptimization::WallsOnlyExt)
                    basenameWallsOnly += "Ext";
                LoadBrickModel(basenameWallsOnly.c_str(), vertices, brickType, BrickGroup_WallsOnly);
            }
        };

        LoadBrickModelGroups("Brick1x1x1",  BrickType_1x1x1,        ModelOptimization::WallsOnly   );
        LoadBrickModelGroups("Tile1x1",     BrickType_1x1x1_Tile                                   );
        LoadBrickModelGroups("Brick1x1x3",  BrickType_1x1x3,        ModelOptimization::WallsOnly   );
        LoadBrickModelGroups("Brick1x2x1",  BrickType_1x2x1,        ModelOptimization::WallsOnlyExt);
        LoadBrickModelGroups("Brick1x2x3",  BrickType_1x2x3,        ModelOptimization::WallsOnlyExt);
        LoadBrickModelGroups("Brick2x2x1",  BrickType_2x2x1,        ModelOptimization::WallsOnlyExt);
        LoadBrickModelGroups("Tile2x2",     BrickType_2x2x1_Tile                                   );
        LoadBrickModelGroups("Brick2x2x3",  BrickType_2x2x3,        ModelOptimization::WallsOnlyExt);
        LoadBrickModelGroups("Roof2x2x3",   BrickType_2x2x3_Roof                                   );
        LoadBrickModelGroups("Roof2x3x3",   BrickType_2x3x3_Roof                                   );
        LoadBrickModelGroups("Brick4x2x1",  BrickType_4x2x1,        ModelOptimization::WallsOnlyExt);
        LoadBrickModelGroups("Brick4x2x3",  BrickType_4x2x3,        ModelOptimization::WallsOnlyExt);

        LoadBrickModel("Brick2x2x3_Glass"    , vertices, BrickType_2x2x3, BrickGroup_Transparent);
        LoadBrickModel("Brick4x2x3_Glass"    , vertices, BrickType_4x2x3, BrickGroup_Transparent);
    }

    void InitAppConfig()
    {
        // Store root AABB of kd-Tree
        KDTreeNode::rootAABB = AABB3i
        {
            Gs::Vector3i{ -boundingVolume[0]/2,                  0, -boundingVolume[2]/2 },
            Gs::Vector3i{ +boundingVolume[0]/2, +boundingVolume[1], +boundingVolume[2]/2 }
        };

        // Use a longer far plane for this example, since the user can pan the camera around
        projectionParams.zNear  = 1.0f;
        projectionParams.zFar   = 500.0f;
        UpdateProjection(swapChain->GetResolution());

        // Cache feature support for offset instancing as this will be used every frame
        const LLGL::RenderingCapabilities& caps = renderer->GetRenderingCaps();
        hasOffsetInstancing = caps.features.hasOffsetInstancing;

        if (caps.limits.lineWidthRange[1] >= 2.0f)
            selectionRenderMode = SelectionRenderMode::NativeLineWidth;
        else if (caps.features.hasGeometryShaders)
            selectionRenderMode = SelectionRenderMode::EmulatedLineWidth;
        else
            selectionRenderMode = SelectionRenderMode::Simplified;
    }

    LLGL::Texture* CreateDummyCubeTexture()
    {
        LLGL::TextureDescriptor cubeTexDesc;
        {
            cubeTexDesc.debugName           = "DummyCubeTexture";
            cubeTexDesc.type                = LLGL::TextureType::TextureCube;
            cubeTexDesc.extent.width        = 1;
            cubeTexDesc.extent.height       = 1;
            cubeTexDesc.arrayLayers         = 6;
            cubeTexDesc.clearValue.color[0] = 1.0f;
            cubeTexDesc.clearValue.color[0] = 1.0f;
            cubeTexDesc.clearValue.color[0] = 1.0f;
            cubeTexDesc.clearValue.color[0] = 1.0f;
        }
        return renderer->CreateTexture(cubeTexDesc);
    }

    LLGL::Texture* LoadCubeTexture(const std::string& basename, std::initializer_list<const char*> cubeFaces)
    {
        LLGL_VERIFY(cubeFaces.size() == 6);

        // Load individual cube faces
        LLGL::Texture* cubeFaceTextures[6] = {};
        int cubeFaceIndex = 0;
        LLGL::Extent3D cubeFace0Extent;

        for (const char* cubeFaceName : cubeFaces)
        {
            // Load cube face texture
            cubeFaceTextures[cubeFaceIndex] = LoadTexture(basename + cubeFaceName);

            // Validate all cube faces have the same size and are square in size
            const LLGL::Extent3D cubeFaceExtent = cubeFaceTextures[cubeFaceIndex]->GetMipExtent(0);
            if (cubeFaceIndex > 0)
            {
                if (cubeFaceExtent != cubeFace0Extent)
                {
                    LLGL::Log::Errorf(
                        "Incompatible cube map sizes: face %d (size=%ux%ux%u)\n does not match face 0 (size=%ux%ux%u)\n",
                        cubeFaceIndex,
                        cubeFaceExtent.width, cubeFaceExtent.height, cubeFaceExtent.depth,
                        cubeFace0Extent.width, cubeFace0Extent.height, cubeFace0Extent.depth
                    );
                    return CreateDummyCubeTexture();
                }
            }
            else
            {
                if (cubeFaceExtent.width != cubeFaceExtent.height || cubeFaceExtent.depth != 1)
                {
                    LLGL::Log::Errorf(
                        "Incompatible cube map size: face 0 (size=%ux%ux%u) does not have a square size\n",
                        cubeFaceExtent.width, cubeFaceExtent.height, cubeFaceExtent.depth
                    );
                    return CreateDummyCubeTexture();
                }
                cubeFace0Extent = cubeFaceExtent;
            }
            ++cubeFaceIndex;
        }

        // Create cube texture with no initial data
        const std::string debugName = "CubeTexture[" + basename + "]";
        LLGL::TextureDescriptor cubeTexDesc;
        {
            cubeTexDesc.debugName       = debugName.c_str();
            cubeTexDesc.type            = LLGL::TextureType::TextureCube;
            cubeTexDesc.extent          = cubeFace0Extent;
            cubeTexDesc.arrayLayers     = 6;
            cubeTexDesc.miscFlags       = LLGL::MiscFlags::NoInitialData;
        }
        LLGL::Texture* cubeTex = renderer->CreateTexture(cubeTexDesc);

        // Copy all cube faces into cube texture on the GPU and generate MIP-maps
        commands->Begin();
        {
            const std::string debugLabel = "BakeCubemap[" + basename + "]";
            commands->PushDebugGroup(debugLabel.c_str());
            {
                LLGL::TextureLocation dstLocation;
                const LLGL::TextureLocation srcLocation;

                for (std::uint32_t cubeFaceIndex = 0; cubeFaceIndex < 6u; ++cubeFaceIndex)
                {
                    dstLocation.arrayLayer = cubeFaceIndex;
                    commands->CopyTexture(*cubeTex, dstLocation, *cubeFaceTextures[cubeFaceIndex], srcLocation, cubeFace0Extent);
                }

                commands->GenerateMips(*cubeTex);
            }
            commands->PopDebugGroup();
        }
        commands->End();
        commandQueue->Submit(*commands);

        return cubeTex;
    }

    void CreateResources()
    {
        const float projZAxis = GetProjectionZAxis();

        std::vector<BrickVertex> vertices;
        LoadAllBrickModels(vertices);

        // Create buffer for per-vertex data
        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.debugName = "BrickVertexBuffer";
            vertexBufferDesc.size      = sizeof(BrickVertex) * vertices.size() + sizeof(Instance);
            vertexBufferDesc.stride    = sizeof(BrickVertex);
            vertexBufferDesc.bindFlags = LLGL::BindFlags::VertexBuffer;
        }
        vertexAndPreviewInstanceBuffer  = renderer->CreateBuffer(vertexBufferDesc);
        previewInstanceBufferOffset     = sizeof(BrickVertex) * vertices.size();

        renderer->WriteBuffer(*vertexAndPreviewInstanceBuffer, 0, vertices.data(), previewInstanceBufferOffset);
        renderer->WriteBuffer(*vertexAndPreviewInstanceBuffer, previewInstanceBufferOffset, &previewInstance, sizeof(previewInstance));

        // Create scene view constant buffer
        sceneViewCbuffer = CreateConstantBuffer(sceneView);

        // Create UI buffers
        uiViewCbuffer = CreateConstantBuffer(uiView);
        uiVertexBatch.vertices = std::unique_ptr<UIVertex[]>(new UIVertex[maxUIVertexBatchSize]);

        LLGL::BufferDescriptor uiVertexBufferDesc;
        {
            uiVertexBufferDesc.debugName    = "UIVertexBuffer";
            uiVertexBufferDesc.size         = sizeof(UIVertex) * maxUIVertexBatchSize;
            uiVertexBufferDesc.stride       = sizeof(UIVertex);
            uiVertexBufferDesc.bindFlags    = LLGL::BindFlags::VertexBuffer;
            uiVertexBufferDesc.miscFlags    = LLGL::MiscFlags::DynamicUsage; // Hint to the renderer that this buffer is updated every frame
        }
        uiVertexBuffer = renderer->CreateBuffer(uiVertexBufferDesc);

        // Create dummy texture for UI PSO
        LLGL::TextureDescriptor uiBlankTexDesc;
        {
            uiBlankTexDesc.debugName            = "BlankUITexture";
            uiBlankTexDesc.type                 = LLGL::TextureType::Texture2DArray;
            uiBlankTexDesc.clearValue.color[0]  = 1.0f;
            uiBlankTexDesc.clearValue.color[1]  = 1.0f;
            uiBlankTexDesc.clearValue.color[2]  = 1.0f;
            uiBlankTexDesc.clearValue.color[3]  = 1.0f;
        }
        uiBlankTexture = renderer->CreateTexture(uiBlankTexDesc);

        // Initialize plane model
        LLGL::BufferDescriptor planeBufferDesc;
        {
            planeBufferDesc.debugName   = "PlaneMeshBuffer";
            planeBufferDesc.size        = sizeof(PlaneVertex) * PlaneModel::maxNumVertices + sizeof(std::uint16_t) * PlaneModel::maxNumIndices;
            planeBufferDesc.stride      = sizeof(PlaneVertex);
            planeBufferDesc.format      = LLGL::Format::R16UInt;
            planeBufferDesc.bindFlags   = LLGL::BindFlags::VertexBuffer | LLGL::BindFlags::IndexBuffer;
        }
        planeModel.meshBuffer = renderer->CreateBuffer(planeBufferDesc);

        planeModel.vertices.resize(PlaneModel::maxNumVertices);
        planeModel.indices.resize(PlaneModel::maxNumIndices);

        planeModel.colorMap     = LoadTexture("Bricks/BrickHead_Albedo.png");
        planeModel.normalMap    = LoadTexture("Bricks/BrickHead_Normals.png");
        planeModel.heightMap    = LoadTexture("Bricks/BrickHead_Heights.png", LLGL::BindFlags::Sampled | LLGL::BindFlags::ColorAttachment, LLGL::Format::R8UNorm);

        // Create primary texture sampler
        LLGL::SamplerDescriptor samplerDesc;
        {
            #ifndef LLGL_MOBILE_PLATFORM
            samplerDesc.maxAnisotropy = 8;
            #endif
        }
        uvWrapSampler = renderer->CreateSampler(samplerDesc);

        // Load skybox textures
        skyboxCubemap = LoadCubeTexture("Skybox/sky_74_cubemap_2k/", { "px.png", "nx.png", "py.png", "ny.png", "pz.png", "nz.png" });
    }

    bool CreatePipelines()
    {
        #define REPORT_PSO_ERRORS(PSO)      \
            do                              \
            {                               \
                if (ReportPSOErrors(PSO))   \
                    return false;           \
            }                               \
            while (false)

        const LLGL::ShaderMacro fsPlaneMacros[]
        {
            LLGL::ShaderMacro{ "ENABLE_PARALLAX_OCCLUSION", ENABLE_PARALLAX_OCCLUSION ? "1" : "0" },
            LLGL::ShaderMacro{},
        };

        // Create shaders
        vsBrick         = LoadVertexShader  ("BrickPlay", "VSBrick");
        fsBrick         = LoadFragmentShader("BrickPlay", "PSBrick");
        vsBrickPreview  = LoadVertexShader  ("BrickPlay", "VSBrickPreview");
        fsBrickPreview  = LoadFragmentShader("BrickPlay", "PSBrickPreview");
        vsOverlay       = LoadVertexShader  ("BrickPlay", "VSOverlay");
        fsOverlay       = LoadFragmentShader("BrickPlay", "PSOverlay");
        vsPlane         = LoadVertexShader  ("BrickPlay", "VSPlane");
        fsPlane         = LoadFragmentShader("BrickPlay", "PSPlane", fsPlaneMacros);
        vsSkybox        = LoadVertexShader  ("BrickPlay", "VSSkybox");
        fsSkybox        = LoadFragmentShader("BrickPlay", "PSSkybox");

        // Create pipeline layout for instanced scene rendering
        psoLayout = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(SceneView@2):vert:frag,"
            )
        );

        // Specify vertex formats
        const LLGL::VertexAttribute vertexAttribs[] =
        {
            // Per-vertex attributes
            LLGL::VertexAttribute{ "position", LLGL::Format::RGB32Float, /*location:*/ 0, /*offset:*/ offsetof(BrickVertex, position), /*stride:*/ sizeof(BrickVertex), /*slot:*/ 0 },
            LLGL::VertexAttribute{ "normal",   LLGL::Format::RGB32Float, /*location:*/ 1, /*offset:*/ offsetof(BrickVertex, normal  ), /*stride:*/ sizeof(BrickVertex), /*slot:*/ 0 },

            // Per-instance attributes
            LLGL::VertexAttribute{ "gridPos",  LLGL::Format::RGBA32SInt, /*location:*/ 2, /*offset:*/ offsetof(Instance, gridPos), /*stride:*/ sizeof(Instance), /*slot:*/ 1, /*instanceDivisor:*/ 1 },
            LLGL::VertexAttribute{ "color",    LLGL::Format::RGBA8UNorm, /*location:*/ 3, /*offset:*/ offsetof(Instance, color  ), /*stride:*/ sizeof(Instance), /*slot:*/ 1, /*instanceDivisor:*/ 1 },
        };

        // Create graphics PSO for default scene rendering
        auto SetPSODefaults = [this, &vertexAttribs](LLGL::GraphicsPipelineDescriptor& psoDesc) -> void
        {
            psoDesc.renderPass                      = swapChain->GetRenderPass();
            psoDesc.inputVertexAttribs              = vertexAttribs;
            psoDesc.vertexShader                    = vsBrick;
            psoDesc.fragmentShader                  = fsBrick;
            psoDesc.pipelineLayout                  = psoLayout;
            psoDesc.primitiveTopology               = LLGL::PrimitiveTopology::TriangleList;
            psoDesc.depth.testEnabled               = true;
            psoDesc.depth.writeEnabled              = true;
            psoDesc.rasterizer.multiSampleEnabled   = (GetSampleCount() > 1);
            psoDesc.rasterizer.cullMode             = LLGL::CullMode::Back;
        };

        LLGL::GraphicsPipelineDescriptor psoDesc;
        {
            SetPSODefaults(psoDesc);
        }
        pso[BrickPSO_Default] = renderer->CreatePipelineState(psoDesc);
        REPORT_PSO_ERRORS(pso[BrickPSO_Default]);

        // Create PSO for wrieframe view mode
        {
            psoDesc.rasterizer.polygonMode = LLGL::PolygonMode::Wireframe;
        }
        pso[BrickPSO_Wireframe] = renderer->CreatePipelineState(psoDesc);
        REPORT_PSO_ERRORS(pso[BrickPSO_Wireframe]);

        // Create PSO for transparent bricks
        LLGL::GraphicsPipelineDescriptor psoTransparentDesc;
        {
            SetPSODefaults(psoTransparentDesc);
            psoTransparentDesc.depth.writeEnabled               = false;
            psoTransparentDesc.blend.targets[0].blendEnabled    = true;
        }
        pso[BrickPSO_Transparent] = renderer->CreatePipelineState(psoTransparentDesc);
        REPORT_PSO_ERRORS(pso[BrickPSO_Transparent]);

        // Create PSO for blocked preview object
        LLGL::GraphicsPipelineDescriptor psoPreviewDesc;
        {
            SetPSODefaults(psoPreviewDesc);
            psoPreviewDesc.vertexShader                         = vsBrickPreview;
            psoPreviewDesc.fragmentShader                       = fsBrickPreview;
            psoPreviewDesc.rasterizer.polygonMode               = LLGL::PolygonMode::Fill;
            psoPreviewDesc.rasterizer.depthBias.constantFactor  = -100.0f;
        }
        pso[BrickPSO_Preview] = renderer->CreatePipelineState(psoPreviewDesc);
        REPORT_PSO_ERRORS(pso[BrickPSO_Preview]);

        #if DEBUG_KDTREE
        LLGL::GraphicsPipelineDescriptor psoDebugDesc;
        {
            psoDebugDesc.debugName          = "DebugPSO";
            psoDebugDesc.pipelineLayout     = renderer->CreatePipelineLayout(LLGL::Parse("cbuffer(SceneView@2):vert,float4(vertexA,vertexB)"));
            psoDebugDesc.vertexShader       = LoadVertexShader  ("BrickPlay", "VSDebug");
            psoDebugDesc.fragmentShader     = LoadFragmentShader("BrickPlay", "PSDebug");
            psoDebugDesc.primitiveTopology  = LLGL::PrimitiveTopology::LineList;
            psoDebugDesc.depth.testEnabled  = true;
        }
        psoDebug = renderer->CreatePipelineState(psoDebugDesc);
        REPORT_PSO_ERRORS(psoDebug);
        #endif

        // Create PSO to render an overlay with simple UI elements
        psoUILayout = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(UIView@1):vert,"
                "texture(uiTexture@2):frag,"

                "sampler(uiTextureSampler@3){filter=nearest}:frag,"                     // Static sampler with nearest filtering
                "sampler<uiTexture, uiTextureSampler>(s_uiTextureuiTextureSampler@2),"  // Combined texture-sampler for OpenGL backends
            )
        );

        const LLGL::VertexAttribute uiVertexAttribs[] =
        {
            LLGL::VertexAttribute{ "position", LLGL::Format::RG16SInt,   /*location:*/ 0, /*offset:*/ offsetof(UIVertex, position), /*stride:*/ sizeof(UIVertex) },
            LLGL::VertexAttribute{ "texCoord", LLGL::Format::RGBA8UInt,  /*location:*/ 1, /*offset:*/ offsetof(UIVertex, texCoord), /*stride:*/ sizeof(UIVertex) },
            LLGL::VertexAttribute{ "color",    LLGL::Format::RGBA8UNorm, /*location:*/ 2, /*offset:*/ offsetof(UIVertex, color   ), /*stride:*/ sizeof(UIVertex) },
        };

        LLGL::GraphicsPipelineDescriptor psoUIDesc;
        {
            psoUIDesc.debugName                     = "OverlayPSO";
            psoUIDesc.renderPass                    = swapChain->GetRenderPass();
            psoUIDesc.pipelineLayout                = psoUILayout;
            psoUIDesc.inputVertexAttribs            = uiVertexAttribs;
            psoUIDesc.vertexShader                  = vsOverlay;
            psoUIDesc.fragmentShader                = fsOverlay;
            psoUIDesc.blend.targets[0].blendEnabled = true; // Enable default alpha blending for UI elements
        }
        psoUI = renderer->CreatePipelineState(psoUIDesc);
        REPORT_PSO_ERRORS(psoUI);

        if (selectionRenderMode != SelectionRenderMode::Simplified)
        {
            // Create PSO to render silhouette outline when selecting bricks
            psoOutlineWriteStencilLayout = renderer->CreatePipelineLayout(LLGL::Parse("cbuffer(SceneView@2):vert"));

            vsOutline = LoadVertexShader  ("BrickPlay", "VSBrickOutline");
            fsOutline = LoadFragmentShader("BrickPlay", "PSBrickOutline");

            // Create PSO to write stencil to mask out pixels that are inside the silhouette
            LLGL::GraphicsPipelineDescriptor psoOutlineDesc;
            {
                psoOutlineDesc.debugName                        = "EmulatedLineWidthPSO-WriteStencil";
                psoOutlineDesc.pipelineLayout                   = psoOutlineWriteStencilLayout;
                psoOutlineDesc.inputVertexAttribs               = vertexAttribs;
                psoOutlineDesc.renderPass                       = swapChain->GetRenderPass();

                // This PSO only renders a stencil value, so no fragment shader is provided
                psoOutlineDesc.vertexShader                     = vsOutline;

                // Enable stencil test and write out a reference value of 1
                psoOutlineDesc.stencil.testEnabled              = true;
                psoOutlineDesc.stencil.front.depthPassOp        = LLGL::StencilOp::Replace;
                psoOutlineDesc.stencil.front.compareOp          = LLGL::CompareOp::AlwaysPass;
                psoOutlineDesc.stencil.front.readMask           = 0x0;
                psoOutlineDesc.stencil.front.writeMask          = 0x1;
                psoOutlineDesc.stencil.front.reference          = 0x1;

                psoOutlineDesc.rasterizer.cullMode              = LLGL::CullMode::Back;
                psoOutlineDesc.rasterizer.multiSampleEnabled    = (GetSampleCount() > 1);
                psoOutlineDesc.blend.targets[0].colorMask       = LLGL::ColorMaskFlags::Zero;
            }
            psoOutlineWriteStencil = renderer->CreatePipelineState(psoOutlineDesc);
            REPORT_PSO_ERRORS(psoOutlineWriteStencil);

            // Create PSO to read stencil and write color output for silhouette
            if (selectionRenderMode == SelectionRenderMode::NativeLineWidth)
            {
                // Use native support for line width
                psoOutlineWriteColorLayout = renderer->CreatePipelineLayout(LLGL::Parse("cbuffer(SceneView@2):vert:frag"));

                psoOutlineDesc.pipelineLayout                   = psoOutlineWriteColorLayout;
                psoOutlineDesc.rasterizer.lineWidth             = silhouetteWidth;
                psoOutlineDesc.rasterizer.polygonMode           = LLGL::PolygonMode::Wireframe;
            }
            else
            {
                // Emulate line width using a geometry shader
                gsOutline = LoadGeometryShader("BrickPlay", "GSBrickOutline");

                psoOutlineWriteColorLayout = renderer->CreatePipelineLayout(LLGL::Parse("cbuffer(SceneView@2):vert:geom:frag"));

                psoOutlineDesc.pipelineLayout                   = psoOutlineWriteColorLayout;
                psoOutlineDesc.geometryShader                   = gsOutline;
            }

            {
                psoOutlineDesc.debugName                        = "EmulatedLineWidthPSO-WriteColor";
                psoOutlineDesc.fragmentShader                   = fsOutline;

                // Enable stencil test and read a reference value of 1, in which case no pixels are written to mask out pixels inside the silhouette
                psoOutlineDesc.stencil.testEnabled              = true;
                psoOutlineDesc.stencil.front.depthPassOp        = LLGL::StencilOp::Keep;
                psoOutlineDesc.stencil.front.compareOp          = LLGL::CompareOp::NotEqual;
                psoOutlineDesc.stencil.front.readMask           = 0x1;
                psoOutlineDesc.stencil.front.writeMask          = 0x0;
                psoOutlineDesc.stencil.front.reference          = 0x1;

                psoOutlineDesc.blend.targets[0].colorMask       = LLGL::ColorMaskFlags::All;
            }
            psoOutlineWriteColor = renderer->CreatePipelineState(psoOutlineDesc);
            REPORT_PSO_ERRORS(psoOutlineWriteColor);
        }

        // Create PSO for plane rendering
        psoPlaneLayout = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(SceneView@2):vert:frag,"
                "sampler(planeSampler@3):frag,"
                "texture(planeColorMap@4):frag,"
                "texture(planeNormalMap@5):frag,"

                #if ENABLE_PARALLAX_OCCLUSION
                "texture(planeHeightMap@6):frag,"
                "sampler<planeHeightMap, planeSampler>(s_planeHeightMapplaneSampler@6),"
                #endif

                "sampler<planeColorMap, planeSampler>(s_planeColorMapplaneSampler@4),"
                "sampler<planeNormalMap, planeSampler>(s_planeNormalMapplaneSampler@5),"
            )
        );

        const LLGL::VertexAttribute planeVertexAttribs[] =
        {
            LLGL::VertexAttribute{ "worldPos", LLGL::Format::RGB32Float, /*location:*/ 0, /*offset:*/ offsetof(PlaneVertex, worldPos), /*stride:*/ sizeof(PlaneVertex) },
        };

        LLGL::GraphicsPipelineDescriptor psoPlaneDesc;
        {
            psoPlaneDesc.debugName                      = "PlanePSO";
            psoPlaneDesc.renderPass                     = swapChain->GetRenderPass();
            psoPlaneDesc.inputVertexAttribs             = planeVertexAttribs;
            psoPlaneDesc.vertexShader                   = vsPlane;
            psoPlaneDesc.fragmentShader                 = fsPlane;
            psoPlaneDesc.pipelineLayout                 = psoPlaneLayout;
            psoPlaneDesc.primitiveTopology              = LLGL::PrimitiveTopology::TriangleList;
            psoPlaneDesc.depth.testEnabled              = true;
            psoPlaneDesc.depth.writeEnabled             = true;
            psoPlaneDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
        };
        psoPlane[0] = renderer->CreatePipelineState(psoPlaneDesc);
        REPORT_PSO_ERRORS(psoPlane[0]);
        {
            psoPlaneDesc.rasterizer.polygonMode         = LLGL::PolygonMode::Wireframe;
        }
        psoPlane[1] = renderer->CreatePipelineState(psoPlaneDesc);
        REPORT_PSO_ERRORS(psoPlane[1]);

        // Create PSO for skybox rendering
        psoSkyLayout = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(SceneView@2):vert,"
                "texture(skyboxTexture@3):frag,"
                "sampler(skyboxSampler@4){}:frag," // Default static sampler

                "sampler<skyboxTexture, skyboxSampler>(s_skyboxTextureskyboxSampler@3),"
            )
        );

        LLGL::GraphicsPipelineDescriptor psoSkyDesc;
        {
            psoSkyDesc.debugName                        = "SkyboxPSO";
            psoSkyDesc.renderPass                       = swapChain->GetRenderPass();
            psoSkyDesc.vertexShader                     = vsSkybox;
            psoSkyDesc.fragmentShader                   = fsSkybox;
            psoSkyDesc.pipelineLayout                   = psoSkyLayout;
            psoSkyDesc.primitiveTopology                = LLGL::PrimitiveTopology::TriangleList;
            psoSkyDesc.depth.testEnabled                = true;
            psoSkyDesc.rasterizer.multiSampleEnabled    = (GetSampleCount() > 1);
        };
        psoSky = renderer->CreatePipelineState(psoSkyDesc);
        REPORT_PSO_ERRORS(psoSky);

        return true;
    }

    bool SaveScene(const BrickScene& scene, const char* filename)
    {
        // Open file for writing
        std::ofstream file{ filename };
        if (!file.good())
        {
            LLGL::Log::Errorf("Failed to save scene to file: %s\n", filename);
            return false;
        }

        // Write header
        file << "# Scene file for LLGL's example 'BrickPlay'" << std::endl;
        file << "# Current brick type: 'b <DIMENSIONS>' e.g. 'b 2x2x3', supported dimensions are 2x2x3, 4x2x3, 4x2x1" << std::endl;
        file << "# New brick instance: 'i X Y Z ROTATE RGBA' e.g. 'i 10 0 -3 90 FFFF00FF', ROTATE must be 0 or 90" << std::endl;
        file << "# Camera posiotion: 'c X Y Z PITCH YAW' e.g. 'i 10.2 5.7 -15.02 25.2 -15.1'" << std::endl;
        file << "# Current offset: 'o X Y Z' e.g. 'o 10 0 0' moves all subsequent bricks +10 on the X-axis" << std::endl;

        // Write camera position
        file
            << 'c'
            << ' ' << camera.position.x
            << ' ' << camera.position.y
            << ' ' << camera.position.z
            << ' ' << camera.pitch
            << ' ' << camera.yaw
            << std::endl;

        // Write all instances for each brick type
        BrickType lastWrittenBrickType = BrickType_Num;

        char colorStr[9] = {};

        auto ColorToString = [&colorStr](LLGL::ColorRGBAub color) -> const char*
        {
            auto UbyteToHex = [](char* out, std::uint8_t val) -> void
            {
                const char* alphabet = "0123456789ABCDEF";
                out[0] = alphabet[val / 0x10];
                out[1] = alphabet[val % 0x10];
            };

            if (color.r == color.g && color.g == color.b && color.a == 0xFF)
            {
                // Write color out as gray-scaled value
                UbyteToHex(colorStr, color.r);
                colorStr[2] = '\0';
            }
            else
            {
                // Write out RGB channels
                UbyteToHex(colorStr    , color.r);
                UbyteToHex(colorStr + 2, color.g);
                UbyteToHex(colorStr + 4, color.b);

                // Skip alpha channel if its opaque (0xFF)
                if (color.a == 0xFF)
                    colorStr[6] = '\0';
                else
                    UbyteToHex(colorStr + 6, color.a);
            }

            return colorStr;
        };

        for_range(brickType, BrickType_Num)
        {
            for_range(groupType, BrickGroup_Num)
            {
                // Write new brick type
                const InstanceGroup& group = scene.GetInstanceGroup(static_cast<BrickType>(brickType), static_cast<BrickGroup>(groupType));
                if (group.numInstances > 0)
                {
                    if (lastWrittenBrickType != brickType)
                    {
                        lastWrittenBrickType = static_cast<BrickType>(brickType);
                        file << "b " << BrickDictionary::Get().TypeToString(static_cast<BrickType>(brickType)) << std::endl;
                    }
                }

                // Write all instances
                for_range(i, group.numInstances)
                {
                    const Instance& instance = scene.GetInstanceByIndex(group.firstInstance + i);
                    file
                        << 'i'
                        << ' ' << instance.gridPos.x
                        << ' ' << instance.gridPos.y
                        << ' ' << instance.gridPos.z
                        << ' ' << static_cast<int>(instance.gridRotation * 90)
                        << ' ' << ColorToString(instance.color)
                        << std::endl;
                }
            }
        }

        LLGL::Log::Printf("Saved scene to file: %s\n", filename);
        return true;
    }

    bool LoadScene(BrickScene& scene, const char* filename)
    {
        std::ifstream file{ filename };
        if (!file.good())
        {
            LLGL::Log::Errorf("Failed to load scene from file: %s\n", filename);
            return false;
        }

        BrickType currentBrickType = BrickType_Num;
        Gs::Vector3i currentOffset;

        auto ColorFromString = [](const std::string& s) -> LLGL::ColorRGBAub
        {
            LLGL::ColorRGBAub color;

            auto HexToUbyte = [](const char* s) -> std::uint8_t
            {
                std::uint8_t val = 0;
                for (int i = 0; i < 2; ++i)
                {
                    val *= 0x10;
                         if (s[i] >= '0' && s[i] <= '9') { val += s[i] - '0';       }
                    else if (s[i] >= 'A' && s[i] <= 'F') { val += s[i] - 'A' + 0xA; }
                    else if (s[i] >= 'a' && s[i] <= 'f') { val += s[i] - 'a' + 0xA; }
                }
                return val;
            };

            if (s.size() == 2)
            {
                // Parse string as gray-scaled color value
                color.r = color.g = color.b = HexToUbyte(&s[0]);
            }
            else if (s.size() >= 6)
            {
                // Parse RGB values and optional Alpha channel
                color.r = HexToUbyte(&s[0]);
                color.g = HexToUbyte(&s[2]);
                color.b = HexToUbyte(&s[4]);
                if (s.size() == 8)
                    color.a = HexToUbyte(&s[6]);
            }

            return color;
        };

        // Parse every line and interpret it
        int lineNo = 0;
        for (std::string line; std::getline(file, line);)
        {
            ++lineNo;

            // Ignore empty line
            if (line.empty())
                continue;

            // Ignore line only consisting of whitespace characters
            std::size_t firstNoneWhitespace = line.find_first_not_of(" \t");
            if (firstNoneWhitespace == std::string::npos)
                continue;

            // Ignore comment line
            if (line[firstNoneWhitespace] == '#')
                continue;

            // Parse current line with stringstream
            std::stringstream stream{ line };

            char cmd = '\0';
            stream >> cmd;

            if (cmd == 'i')
            {
                if (currentBrickType == BrickType_Num)
                {
                    LLGL::Log::Errorf("Missing brick type 'b' before new instance 'i' in %s:%d\n", filename, lineNo);
                    scene.ResetScene();
                    return false;
                }

                // Parse instance data
                Gs::Vector3i gridPos;
                int gridRotation = 0;
                std::string colorStr;

                stream >> gridPos.x >> gridPos.y >> gridPos.z >> gridRotation >> colorStr;

                if (gridRotation % 90 != 0)
                    LLGL::Log::Errorf("Ignored unknown brick rotation %d (must be a multiple of 90) in %s:%d\n", gridRotation, filename, lineNo);

                // Create new brick instance
                scene.NewBrick(currentBrickType, gridPos + currentOffset, static_cast<GridRotation>(gridRotation / 90), ColorFromString(colorStr));
            }
            else if (cmd == 'b')
            {
                // Parse brick type and make it current
                std::string brickTypeStr;
                stream >> brickTypeStr;
                currentBrickType = BrickDictionary::Get().TypeFromString(brickTypeStr.c_str());

                if (currentBrickType == BrickType_Num)
                {
                    LLGL::Log::Errorf("Unknown brick type 'b %s' in %s:%d\n", brickTypeStr.c_str(), filename, lineNo);
                    scene.ResetScene();
                    return false;
                }
            }
            else if (cmd == 'o')
            {
                // Parse grid offset
                stream >> currentOffset.x >> currentOffset.y >> currentOffset.z;
            }
            else if (cmd == 'c')
            {
                // Parse camera orientation
                stream >> camera.position.x >> camera.position.y >> camera.position.z >> camera.pitch >> camera.yaw;
            }
            else
            {
                LLGL::Log::Errorf("Unknown command '%c' in %s:%d\n", cmd, filename, lineNo);
                scene.ResetScene();
                return false;
            }
        }

        LLGL::Log::Printf("Loaded scene from file: %s\n", filename);
        return true;
    }

    void CreateDefaultScene(BrickScene& scene)
    {
        // Place initial camera position
        camera.position = { 0, 10.0f, -20.0f };
        camera.pitch    = 20.0f;

        //TODO

        for (int i = 0; i <= 4; ++i)
        {
            for (int j = 0; j <= 4 - i; ++j)
            {
                scene.NewBrick(BrickType_4x2x3, { 4*(i-2)+j*2, j*3,  0 }, GridRotation_0, colorPalette[i%5]);
            }
        }

        scene.NewBrick(BrickType_4x2x1, { -9, 3, -4 }, GridRotation_90, colorPalette[1]);
        scene.NewBrick(BrickType_4x2x1, { -9, 3, 0 }, GridRotation_90, colorPalette[1]);
        scene.NewBrick(BrickType_4x2x1, { -9, 3, +4 }, GridRotation_90, colorPalette[1]);

    }

    //////////////////// RENDERING ////////////////////

    void BindInstancePSO(BrickPSO psoType)
    {
        commands->SetPipelineState(*pso[psoType]);
        commands->SetResource(0, *sceneViewCbuffer);
    }

    void RenderBrickInstanceGroup(const BrickMesh& mesh, const InstanceGroup& group, LLGL::VertexBufferView (&vertexBufferViews)[2])
    {
        if (hasOffsetInstancing)
        {
            // Draw all group instances
            commands->DrawInstanced(mesh.numVertices, mesh.firstVertex, group.numInstances, group.firstInstance);
        }
        else
        {
            // Bind vertex buffers with instance offset if not natively supported
            vertexBufferViews[1].offset = group.firstInstance * sizeof(Instance);
            commands->SetVertexBuffers(2, vertexBufferViews);
            commands->DrawInstanced(mesh.numVertices, mesh.firstVertex, group.numInstances);
        }
    }

    void RenderBrickInstances(const BrickScene& scene)
    {
        // Exit early if no instances have been allocated yet
        LLGL::Buffer* instanceBuffer = scene.GetInstanceBuffer();
        if (instanceBuffer == nullptr)
            return;

        // Set vertex buffers (vertices and instances)
        LLGL::VertexBufferView vertexBufferViews[] =
        {
            LLGL::VertexBufferView{ vertexAndPreviewInstanceBuffer },
            LLGL::VertexBufferView{ instanceBuffer },
        };

        // If offset instancing is not supported, set vertex buffers with an offset for each instance group
        if (hasOffsetInstancing)
            commands->SetVertexBuffers(2, vertexBufferViews);

        BindInstancePSO(viewOptions.showWireframe ? BrickPSO_Wireframe : BrickPSO_Default);

        // Draw instances of all groups
        for (int groupType = BrickGroup_Default; groupType < BrickGroup_Num; ++groupType)
        {
            // Change PSO for transparent objects
            if (groupType == BrickGroup_Transparent)
                BindInstancePSO(viewOptions.showWireframe ? BrickPSO_Wireframe : BrickPSO_Transparent);

            for (int brickType = 0; brickType < BrickType_Num; ++brickType)
            {
                const std::uint32_t groupId = CombineToBrickId(brickType, groupType);
                const BrickMesh& mesh = BrickDictionary::Get().GetMesh(brickType, groupType);
                const InstanceGroup& group = scene.GetInstanceGroup(static_cast<BrickType>(brickType), static_cast<BrickGroup>(groupType));
                if (group.numInstances > 0 && mesh.numVertices > 0)
                {
                    const std::string debugGroup = "InstanceGroup" + std::to_string(groupId);
                    commands->PushDebugGroup(debugGroup.c_str());
                    {
                        RenderBrickInstanceGroup(mesh, group, vertexBufferViews);
                    }
                    commands->PopDebugGroup();
                }
            }
        }
    }

    void UpdatePlaneGeometryForViewFrustum()
    {
        // Reset plane model state
        planeModel.numVertices = 0;
        planeModel.numIndices = 0;

        // Transform vertices into frustum for all subdivision segments
        const Gs::Matrix4f frustumMatrix = (projection * camera.vMatrix).Inverse();

        // NDC-to-world unprojection requires a perspective divide by W; a plain Vector4->Vector3 cast only truncates .xyz
        auto UnprojectNDC = [&frustumMatrix](float x, float y, float z) -> Gs::Vector3f
        {
            const Gs::Vector4f clip = frustumMatrix * Gs::Vector4f{ x, y, z, 1.0f };
            return Gs::Vector3f{ clip.x, clip.y, clip.z } * (1.0f / clip.w);
        };

        const Gs::Vector3f frustumLeftTop     = UnprojectNDC(-1.0f, +1.0f, +1.0f);
        const Gs::Vector3f frustumRightTop    = UnprojectNDC(+1.0f, +1.0f, +1.0f);
        const Gs::Vector3f frustumRightBottom = UnprojectNDC(+1.0f, -1.0f, +1.0f);
        const Gs::Vector3f frustumLeftBottom  = UnprojectNDC(-1.0f, -1.0f, +1.0f);

        // Eye is the camera's world position; no need to unproject the clip-space origin for this
        const Gs::Vector3f eye = camera.position;

        static_assert(planeSubdivisions > 0, "`planeSubdivisions` must be greater than 0");
        static_assert(planeSubdivisions <= 16, "`planeSubdivisions` should not be too large; consider keeping it at or below 16!");

        Gs::Vector3f segmentedVertices[planeSubdivisions + 1][planeSubdivisions + 1];

        const float planeSubdivisionsInv = 1.0f / static_cast<float>(planeSubdivisions);
        for (int x = 0; x <= planeSubdivisions; ++x)
        {
            float u = static_cast<float>(x) * planeSubdivisionsInv;

            const Gs::Vector3f horizontalTop    = Gs::Lerp(frustumLeftTop,    frustumRightTop,    u);
            const Gs::Vector3f horizontalBottom = Gs::Lerp(frustumLeftBottom, frustumRightBottom, u);

            for (int y = 0; y <= planeSubdivisions; ++y)
            {
                float v = static_cast<float>(y) * planeSubdivisionsInv;

                segmentedVertices[x][y] = Gs::Lerp(horizontalTop, horizontalBottom, v);
            }
        }

        auto GetPlaneIntersection = [](const Gs::Vector3f& eye, const Gs::Vector3f& vert) -> Gs::Vector3f
        {
            float interpolator = 0.0f;
            const Ray3f ray{ eye, (vert - eye).Normalized() };

            const Gs::Vector3f planeNormal{ 0, 1, 0 };
            if (IntersectRayWithPlane(ray, planeNormal, 0.0f, &interpolator))
                return ray.Interpolate(interpolator);
            else
                return eye;
        };

        // Now generate plane vertices by projecting them into the scene
        for (int x = 0; x < planeSubdivisions; ++x)
        {
            for (int y = 0; y < planeSubdivisions; ++y)
            {
                Gs::Vector3f inVertices[4], outVertices[5];

                inVertices[0] = segmentedVertices[x    ][y    ];
                inVertices[1] = segmentedVertices[x + 1][y    ];
                inVertices[2] = segmentedVertices[x + 1][y + 1];
                inVertices[3] = segmentedVertices[x    ][y + 1];

                int numOutVertices = 0;
                for (int i = 0; i < 4; ++i)
                {
                    const Gs::Vector3f& vert = inVertices[i];
                    const Gs::Vector3f& prevVert = inVertices[(i - 1) & 3];

                    if (vert.y > 0.0f)
                    {
                        if (prevVert.y <= 0.0f)
                        {
                            float t = prevVert.y / (prevVert.y - vert.y);
                            outVertices[numOutVertices++] = Gs::Lerp(prevVert, vert, t);
                        }
                    }
                    else
                    {
                        if (prevVert.y > 0.0f)
                        {
                            float t = prevVert.y / (prevVert.y - vert.y);
                            outVertices[numOutVertices++] = Gs::Lerp(prevVert, vert, t);
                        }

                        // Add intersection between eye-to-vertex with plane
                        outVertices[numOutVertices++] = GetPlaneIntersection(eye, vert);
                    }
                }

                // Too many or not enough output vertices -> skip segment
                if (numOutVertices < 3 || numOutVertices > 5)
                    continue;

                // Write output vertices
                for (int i = 0; i < numOutVertices; ++i)
                {
                    PlaneVertex& planeVert = planeModel.vertices[planeModel.numVertices + i];
                    planeVert.worldPos = { outVertices[i].x, 0.0f, outVertices[i].z };
                }

                // Write output indices
                for (int i = 0; i + 2 < numOutVertices; ++i)
                {
                    planeModel.indices[planeModel.numIndices + i*3    ] = planeModel.numVertices;
                    planeModel.indices[planeModel.numIndices + i*3 + 1] = planeModel.numVertices + i + 1;
                    planeModel.indices[planeModel.numIndices + i*3 + 2] = planeModel.numVertices + i + 2;
                }

                planeModel.numVertices += numOutVertices;
                planeModel.numIndices += (numOutVertices - 2)*3;
            }
        }

        // Upload both vertex and index data to the GPU
        if (planeModel.numVertices > 0 && planeModel.numIndices > 0)
        {
            commands->UpdateBuffer(*planeModel.meshBuffer, 0, planeModel.vertices.data(), sizeof(PlaneVertex) * planeModel.numVertices);
            commands->UpdateBuffer(*planeModel.meshBuffer, PlaneModel::meshBufferIndexOffset, planeModel.indices.data(), sizeof(std::uint16_t) * planeModel.numIndices);
        }
    }

    void RenderPlane()
    {
        commands->SetPipelineState(*psoPlane[viewOptions.showWireframe ? 1 : 0]);
        commands->SetResource(0, *sceneViewCbuffer);
        commands->SetResource(1, *uvWrapSampler);
        commands->SetResource(2, *planeModel.colorMap);
        commands->SetResource(3, *planeModel.normalMap);

        #if ENABLE_PARALLAX_OCCLUSION
        commands->SetResource(4, *planeModel.heightMap);
        #endif

        commands->SetIndexBuffer(*planeModel.meshBuffer, LLGL::Format::R16UInt, PlaneModel::meshBufferIndexOffset);
        commands->SetVertexBuffer(*planeModel.meshBuffer);

        commands->DrawIndexed(planeModel.numIndices, 0);
    }

    void RenderSkybox()
    {
        // Skybopx is rendered with a fullscreen triangle, i.e. the vertex shader generates the vertices on its own. No vertex buffer needed.
        commands->SetPipelineState(*psoSky);

        commands->SetResource(0, *sceneViewCbuffer);
        commands->SetResource(1, *skyboxCubemap);

        commands->Draw(3, 0);
    }

    void RenderBrickScene(const BrickScene& scene)
    {
        commands->PushDebugGroup("RenderBrickInstances");
        {
            RenderBrickInstances(scene);
        }
        commands->PopDebugGroup();

        if (planeModel.numIndices > 0)
        {
            commands->PushDebugGroup("RenderPlane");
            {
                RenderPlane();
            }
            commands->PopDebugGroup();
        }

        commands->PushDebugGroup("RenderSkybox");
        {
            RenderSkybox();
        }
        commands->PopDebugGroup();
    }

    void BindPreviewBrickPSOStates()
    {
        commands->SetPipelineState(*pso[BrickPSO_Preview]);
        commands->SetResource(0, *sceneViewCbuffer);

        // Set vertex buffers (vertices and instances).
        // We don't care about offset instancing here, because the preview instance is always the first one (index 0).
        LLGL::VertexBufferView vertexBufferViews[] =
        {
            LLGL::VertexBufferView{ vertexAndPreviewInstanceBuffer },
            LLGL::VertexBufferView{ vertexAndPreviewInstanceBuffer, sizeof(Instance), previewInstanceBufferOffset },
        };
        commands->SetVertexBuffers(2, vertexBufferViews);
    }

    void RenderPreviewBrick(const PreviewBrick& previewBrick)
    {
        // Not every combination of brick type and group is supported. If number of vertices is 0, exit early.
        const BrickMesh& mesh = BrickDictionary::Get().GetMesh(previewBrick.type, previewBrick.isTransparent ? BrickGroup_Transparent : BrickGroup_Default);
        if (mesh.numVertices == 0)
            return;

        commands->Draw(mesh.numVertices, mesh.firstVertex);
    }

    void RenderSilhouette(const BrickScene& scene, Brick brick)
    {
        // Exit early if no instances have been allocated yet
        LLGL::Buffer* instanceBuffer = scene.GetInstanceBuffer();
        if (instanceBuffer == nullptr)
            return;

        const Instance* brickInstance = scene.GetBrickInstance(brick);
        if (brickInstance == nullptr)
            return;

        const BrickMesh& visibleMesh = BrickDictionary::Get().GetMesh(brick.GetType(), BrickGroup_Default);
        if (visibleMesh.numVertices == 0)
            return;

        const InstanceGroup& pickedGroup = scene.GetInstanceGroupById(brick.groupId);

        // Bind vertex buffers for single instance. No need to rely on offset instancing support, just always use vertex buffer offset.
        LLGL::VertexBufferView vertexBufferViews[] =
        {
            LLGL::VertexBufferView{ vertexAndPreviewInstanceBuffer },
            LLGL::VertexBufferView{ instanceBuffer, 0, (pickedGroup.firstInstance + brick.groupInstance) * sizeof(Instance) },
        };
        commands->SetVertexBuffers(2, vertexBufferViews);

        // First render stencil reference to mask out pixels inside the silhouette. This is the regular brick model.
        commands->PushDebugGroup("RenderSilhouetteStencil");
        {
            commands->SetPipelineState(*psoOutlineWriteStencil);
            commands->SetResource(0, *sceneViewCbuffer);
            commands->Draw(visibleMesh.numVertices, visibleMesh.firstVertex);
        }
        commands->PopDebugGroup();

        // Now render silhouette by rendering with a wireframe model with thick lines
        commands->PushDebugGroup("RenderSilhouetteColor");
        {
            commands->SetPipelineState(*psoOutlineWriteColor);
            commands->SetResource(0, *sceneViewCbuffer);
            commands->Draw(visibleMesh.numVertices, visibleMesh.firstVertex);
        }
        commands->PopDebugGroup();
    }

    void TransformSceneViewForThumbnail(BrickType brickType, GridRotation gridRotation, SceneView& outView)
    {
        const float dist = 25.0f;

        const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(brickType, gridRotation);
        const Gs::Vector3i brickCenter = (brickDim.offset*2 + brickDim.extent);
        const Gs::Vector3i unitOffset{ brickCenter.x % 2, brickDim.extent.y, brickCenter.z % 2 };

        Gs::Matrix4f vMatrix;
        Gs::Translate(vMatrix, unitOffset.Cast<float>() * KDTreeNode::gridScale * 0.5f);
        Gs::RotateY(vMatrix, Gs::Deg2Rad(45.0f));
        Gs::RotateX(vMatrix, Gs::Deg2Rad(45.0f));
        Gs::Translate(vMatrix, Gs::Vector3f{ 0, 0, -dist });

        outView.viewPos     = Gs::TransformVector(vMatrix, Gs::Vector4f{ 0, 0, 0, 1 });
        outView.gridScale   = KDTreeNode::gridScale;

        // Visually, we only want to rotate the brick, but they only support 90 degree angles.
        // So instead, we turn the camera around it but still want the light to look like it shines straight against the brick.
        outView.lightDir[0] = -Gs::TransformVector(vMatrix, Gs::Vector4f{ 0.0f, -0.25f, +1.0f, 0.0f }).Normalized();

        vMatrix.MakeInverse();

        // Use orthogonal projection for brick thumbnails
        const Gs::Vector3f brickExtentScaled = brickDim.extent.Cast<float>() * KDTreeNode::gridScale;
        const float maxBrickExtent = std::max(brickExtentScaled.x, std::max(brickExtentScaled.y, brickExtentScaled.z));
        const float volumeSize = std::max(uiThumbnailMinVolumeSize, maxBrickExtent + 1.0f);

        const Gs::Matrix4f thumbnailProjection = OrthogonalProjection(volumeSize, volumeSize, 0.1f, dist*2.0f);
        outView.vpMatrix = thumbnailProjection * vMatrix;
    }

    void RenderBrickThumnail(LLGL::RenderTarget& renderTarget, BrickType brickType, GridRotation gridRotation)
    {
        PreviewBrick thumbnailBrick;
        SceneView thumbnailSceneView;

        // Move camera so that the thumbnail for the current brick type is clearly visible
        TransformSceneViewForThumbnail(brickType, gridRotation, thumbnailSceneView);
        commands->UpdateBuffer(*sceneViewCbuffer, 0, &thumbnailSceneView, sizeof(thumbnailSceneView));

        commands->BeginRenderPass(renderTarget);
        {
            commands->SetViewport(LLGL::Extent2D{ uiBrickThumbnailSize, uiBrickThumbnailSize });
            commands->Clear(LLGL::ClearFlags::ColorDepth, LLGL::ClearValue{ 0.0f, 0.0f, 0.0f, 0.0f });

            // Draw current thumbnail with preview brick
            thumbnailBrick.type = static_cast<BrickType>(brickType);
            RenderPreviewBrick(thumbnailBrick);
        }
        commands->EndRenderPass();
    }

    void RenderAllBrickThumbnails()
    {
        // In case thumbnails must be updated, delete previous texture
        if (uiBrickThumbnailTexture != nullptr)
        {
            renderer->Release(*uiBrickThumbnailTexture);
            uiBrickThumbnailTexture = nullptr;
        }

        // Create thumbnail array texture
        LLGL::TextureDescriptor texArrayDesc;
        {
            texArrayDesc.debugName      = "BrickThumbnail.Texture2DArray";
            texArrayDesc.type           = LLGL::TextureType::Texture2DArray;
            texArrayDesc.extent         = { uiBrickThumbnailSize, uiBrickThumbnailSize, 1 };
            texArrayDesc.arrayLayers    = BrickType_Num;                    // Each brick type gets its own slot in the array texture
            texArrayDesc.mipLevels      = 1;                                // No MIP-maps since this is used for UI elements
            texArrayDesc.miscFlags      = LLGL::MiscFlags::NoInitialData;   // Don't initialize the texture since we render into it right below
        }
        uiBrickThumbnailTexture = renderer->CreateTexture(texArrayDesc);

        // Create intermediate render-targets, one for each array layer
        LLGL::RenderTarget* renderTargets[BrickType_Num] = {};

        for (int brickType = 0; brickType < BrickType_Num; ++brickType)
        {
            const std::string rtLabel = std::string("RenderTarget.Thumbnail_") + BrickDictionary::Get().TypeToString(static_cast<BrickType>(brickType));
            LLGL::RenderTargetDescriptor rtDesc;
            {
                rtDesc.debugName                        = rtLabel.c_str();
                rtDesc.resolution                       = { uiBrickThumbnailSize, uiBrickThumbnailSize };
                rtDesc.samples                          = GetSampleCount();
                rtDesc.colorAttachments[0].format       = LLGL::Format::RGBA8UNorm;
                rtDesc.resolveAttachments[0].texture    = uiBrickThumbnailTexture;
                rtDesc.resolveAttachments[0].arrayLayer = brickType;                            // Render into array layer designated for current brick type
                rtDesc.depthStencilAttachment.format    = swapChain->GetDepthStencilFormat();   // Use same depth format for thumbnails as for swap-chain (to make PSOs happy)
            }
            renderTargets[brickType] = renderer->CreateRenderTarget(rtDesc);
        }

        // Reset preview instance to default
        constexpr GridRotation thumbnailRotation = GridRotation_270;

        previewInstance.gridPos         = {};
        previewInstance.gridRotation    = thumbnailRotation;
        previewInstance.color           = {};

        // Render bricks as thumbnails into all offscreen render-targets
        commands->Begin();
        {
            commands->UpdateBuffer(*vertexAndPreviewInstanceBuffer, previewInstanceBufferOffset, &previewInstance, sizeof(previewInstance));

            BindPreviewBrickPSOStates();

            commands->PushDebugGroup("RenderBrickThumbnails");
            {
                for (int brickType = 0; brickType < BrickType_Num; ++brickType)
                {
                    const std::string debugLabel = std::string("RenderThumbnail_") + BrickDictionary::Get().TypeToString(static_cast<BrickType>(brickType));
                    commands->PushDebugGroup(debugLabel.c_str());
                    {
                        RenderBrickThumnail(*renderTargets[brickType], static_cast<BrickType>(brickType), thumbnailRotation);
                    }
                    commands->PopDebugGroup();
                }
            }
            commands->PopDebugGroup();
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Delete all intermediate render-targets, we only need the texture for UI rendering
        for (LLGL::RenderTarget* renderTarget : renderTargets)
            renderer->Release(*renderTarget);
    }

    void FlushUIVertexBuffer()
    {
        if (uiVertexBatch.numBatchedVertices > 0)
        {
            /*
            This can be improved by encoding the buffer update command in a separate command buffer
            and submit that command buffer before the primary render commands.
            Updating the buffer right before we draw with it has performance implications for Vulkan and Metal,
            but we keep this example simple for now since we only do this a few times with the small number of UI elements.
            */
            commands->UpdateBuffer(*uiVertexBuffer, 0, uiVertexBatch.vertices.get(), sizeof(UIVertex) * uiVertexBatch.numBatchedVertices);
            commands->Draw(uiVertexBatch.numBatchedVertices, 0);
            uiVertexBatch.numBatchedVertices = 0;
        }
    }

    // Allocates new UI vertices and flushes its vertex buffer if necessary.
    UIVertex* NewUIVertices(std::uint32_t count)
    {
        if (uiVertexBatch.numBatchedVertices + count > maxUIVertexBatchSize)
            FlushUIVertexBuffer();

        UIVertex* outVertices = &(uiVertexBatch.vertices[uiVertexBatch.numBatchedVertices]);
        uiVertexBatch.numBatchedVertices += count;
        return outVertices;
    }

    void SetUITexture(LLGL::Texture* tex)
    {
        // Flush UI vertex buffer since the texture has to change for all
        if (uiVertexBatch.cachedTexture != tex)
        {
            FlushUIVertexBuffer();

            // Bind new texture
            uiVertexBatch.cachedTexture = tex;
            commands->SetResource(1, *tex);
        }
    }

    void Draw2DRect(int x, int y, int w, int h, LLGL::ColorRGBAub color)
    {
        SetUITexture(uiBlankTexture);

        // Allocate 6 vertices to draw 2 triangles
        UIVertex* verts = NewUIVertices(6);

        verts[0].Set(x    , y    , color);
        verts[1].Set(x + w, y    , color);
        verts[2].Set(x    , y + h, color);

        verts[3].Set(x    , y + h, color);
        verts[4].Set(x + w, y    , color);
        verts[5].Set(x + w, y + h, color);
    }

    void Draw2DImage(int x, int y, LLGL::Texture* tex, int texLayer = 0, LLGL::ColorRGBAub color = {})
    {
        SetUITexture(tex);

        const LLGL::Extent3D texExtent = tex->GetMipExtent(0);
        const int w = static_cast<int>(texExtent.width);
        const int h = static_cast<int>(texExtent.height);

        // Allocate 6 vertices to draw 2 triangles
        UIVertex* verts = NewUIVertices(6);

        verts[0].Set(x    , y    , 0, 0, texLayer, color);
        verts[1].Set(x + w, y    , 1, 0, texLayer, color);
        verts[2].Set(x    , y + h, 0, 1, texLayer, color);

        verts[3].Set(x    , y + h, 0, 1, texLayer, color);
        verts[4].Set(x + w, y    , 1, 0, texLayer, color);
        verts[5].Set(x + w, y + h, 1, 1, texLayer, color);
    }

    bool IsMouseInRect(int x, int y, int w, int h)
    {
        const LLGL::Offset2D mousePos = input.GetMousePosition();
        return (mousePos.x >= x && mousePos.x <= x + w && mousePos.y >= y && mousePos.y <= y + h);
    }

    bool IsSelectedByMouse(int x, int y, int w, int h)
    {
        return (controller.cameraMode == CameraMode::None && IsMouseInRect(x, y, w, h));
    }

    bool DrawUIColorPicker(int x, int y, int paletteIndex, bool isHighlighted = false)
    {
        int w = uiColorPickerSize + uiButtonFrameSize*2;
        int h = uiColorPickerSize + uiButtonFrameSize*2;
        const bool isSelected = IsSelectedByMouse(x, y, w, h);

        if (isSelected || isHighlighted)
        {
            // Only draw color button when selected
            Draw2DRect(x, y, w, h, colorPalette[paletteIndex]);
        }
        else
        {
            // Draw background frame and then the foreground color
            Draw2DRect(x, y, w, h, uiButtonFrameColor);
            Draw2DRect(x + uiButtonFrameSize, y + uiButtonFrameSize, uiColorPickerSize, uiColorPickerSize, colorPalette[paletteIndex]);
        }

        return (isSelected && input.KeyDown(LLGL::Key::LButton));
    }

    bool DrawUIBrickThumbnail(int x, int y, int brickType, bool isHighlighted = false)
    {
        int w = uiBrickThumbnailSize + uiButtonFrameSize*2;
        int h = uiBrickThumbnailSize + uiButtonFrameSize*2;
        const bool isSelected = IsSelectedByMouse(x, y, w, h);

        const LLGL::ColorRGBAub brickColor = colorPalette[controller.pickedColorPaletteIndex];

        if (isSelected || isHighlighted)
        {
            // Draw button background to highlight selection
            Draw2DRect(x, y, w, h, uiBrickThumbnailBgColor);
        }

        // Draw thumbnail texture layer
        Draw2DImage(x + uiButtonFrameSize, y + uiButtonFrameSize, uiBrickThumbnailTexture, brickType, brickColor);

        return (isSelected && input.KeyDown(LLGL::Key::LButton));
    }

    // Draws UI elements and also interacts with user input immediately.
    void DrawUIImmediateMode()
    {
        const LLGL::Extent2D& resolution = swapChain->GetResolution();
        const int scrW = static_cast<int>(resolution.width);
        const int scrH = static_cast<int>(resolution.height);

        // Draw mouse dead zone as transparent background for the UI
        Draw2DRect(0, 0, uiState.menuBarLeft, scrH, uiMenuBgColor);
        Draw2DRect(uiState.menuBarRight, 0, scrW - uiState.menuBarRight, scrH, uiMenuBgColor);

        ResetMenuBar(swapChain->GetResolution());

        int uiPos[2], uiButtonAdvance;

        // Draw entire color palette for the user to choose from on the left of the overlay
        uiButtonAdvance = uiColorPickerSize + uiButtonFrameSize*2 + uiButtonMargin;
        uiPos[0] = uiColorPickerOffset[0];
        uiPos[1] = uiColorPickerOffset[1];

        for (int i = 0; i < sizeof(colorPalette)/sizeof(colorPalette[0]); ++i)
        {
            if (uiPos[1] > scrH - uiButtonAdvance)
            {
                // Move to next color picker column
                uiPos[0] += uiButtonAdvance;
                uiPos[1] = uiColorPickerOffset[1];
                uiState.menuBarLeft += uiButtonAdvance;
            }

            const bool isHighlighted = (controller.pickedColorPaletteIndex == i);
            if (DrawUIColorPicker(uiPos[0], uiPos[1], i, isHighlighted))
            {
                // Set new color from palette to preview brick
                previewInstance.color = colorPalette[i];
                controller.pickedColorPaletteIndex = i;
            }

            // Move to next UI element position
            uiPos[1] += uiButtonAdvance;
        }

        // Draw brick thumbnails for the user to choose from on the right of the overlay
        uiButtonAdvance = uiBrickThumbnailSize + uiButtonFrameSize*2 + uiButtonMargin;
        uiPos[0] = scrW - (uiColorPickerOffset[0] + uiBrickThumbnailSize + uiButtonFrameSize*2);
        uiPos[1] = uiColorPickerOffset[1];

        for (int brickType = 0; brickType < BrickType_Num; ++brickType)
        {
            if (uiPos[1] > scrH - uiButtonAdvance)
            {
                // Move to next color picker column
                uiPos[0] -= uiButtonAdvance;
                uiPos[1] = uiColorPickerOffset[1];
                uiState.menuBarRight -= uiButtonAdvance;
            }

            const bool isHighlighted = (viewOptions.previewBrick.type == brickType);
            if (DrawUIBrickThumbnail(uiPos[0], uiPos[1], brickType, isHighlighted))
            {
                // Set new brick type to preview brick
                viewOptions.previewBrick.type = static_cast<BrickType>(brickType);
            }

            // Move to next UI element position
            uiPos[1] += uiButtonAdvance;
        }
    }

    void RenderUIElements()
    {
        // Reset UI vertex batch states
        uiVertexBatch.cachedTexture         = nullptr;
        uiVertexBatch.numBatchedVertices    = 0;

        commands->SetVertexBuffer(*uiVertexBuffer);

        commands->SetPipelineState(*psoUI);
        commands->SetResource(0, *uiViewCbuffer);

        DrawUIImmediateMode();

        // Final flush if this hasn't happened already
        FlushUIVertexBuffer();
    }

    #if DEBUG_KDTREE
    void RenderDebugInfo()
    {
        commands->SetPipelineState(*psoDebug);
        const Gs::Vector4f vertices[2] =
        {
            Gs::Vector4f{ controller.pickedPosition, 1.0f },
            Gs::Vector4f{ controller.pickedPosition + controller.pickedNormal, 1.0f }
        };
        commands->SetResource(0, *sceneViewCbuffer);
        commands->SetUniforms(0, vertices, sizeof(vertices));
        commands->Draw(2, 0);
    }
    #endif

    //////////////////// USER INPUT HANDLING ////////////////////

    void StartCameraMode(CameraMode newMode)
    {
        // Hide cursor and store its last position before starting camera movement mode
        controller.lastCursorPos = LLGL::Display::GetCursorPosition();
        LLGL::Display::ShowCursor(false);
        controller.cameraMode = newMode;
    }

    void StopCameraMode()
    {
        // Show cursor and restore its last position when ending camera movement mode
        controller.cameraMode = CameraMode::None;
        LLGL::Display::SetCursorPosition(controller.lastCursorPos);
        LLGL::Display::ShowCursor(true);
    }

    void ControlCameraByUserInput()
    {
        if (input.KeyDown(LLGL::Key::RButton) && controller.cameraMode == CameraMode::None)
            StartCameraMode(CameraMode::TurnCamera);
        else if (input.KeyUp(LLGL::Key::RButton) && controller.cameraMode == CameraMode::TurnCamera)
            StopCameraMode();

        if (input.KeyDown(LLGL::Key::MButton) && controller.cameraMode == CameraMode::None)
            StartCameraMode(CameraMode::MoveCamera);
        else if (input.KeyUp(LLGL::Key::MButton) && controller.cameraMode == CameraMode::MoveCamera)
            StopCameraMode();

        if (controller.cameraMode == CameraMode::TurnCamera)
        {
            const LLGL::Offset2D motion = input.GetMouseMotion();
            const float pitch = static_cast<float>(+motion.y) * cameraTurnSpeed;
            const float yaw   = static_cast<float>(-motion.x) * cameraTurnSpeed;
            camera.Turn(pitch, yaw);
        }
        else if (controller.cameraMode == CameraMode::MoveCamera)
        {
            const LLGL::Offset2D motion = input.GetMouseMotion();
            const float xAxis = static_cast<float>(+motion.x) * cameraMoveSpeed * 0.5f;
            const float yAxis = static_cast<float>(-motion.y) * cameraMoveSpeed * 0.5f;
            camera.Move({ xAxis, yAxis, 0 }, currentScene.get());
        }

        const float moveSpeed = (input.KeyPressed(LLGL::Key::Shift) ? cameraMoveSpeed*3.0f : cameraMoveSpeed);
        if (input.KeyPressed(LLGL::Key::A) || input.KeyPressed(LLGL::Key::Left))
            camera.Move({ -moveSpeed, 0, 0 }, currentScene.get());
        if (input.KeyPressed(LLGL::Key::D) || input.KeyPressed(LLGL::Key::Right))
            camera.Move({ +moveSpeed, 0, 0 }, currentScene.get());
        if (input.KeyPressed(LLGL::Key::W) || input.KeyPressed(LLGL::Key::Up))
            camera.Move({ 0, 0, +moveSpeed }, currentScene.get());
        if (input.KeyPressed(LLGL::Key::S) || input.KeyPressed(LLGL::Key::Down))
            camera.Move({ 0, 0, -moveSpeed }, currentScene.get());
    }

    Gs::Vector3i WorldPosToGridPos(const Gs::Vector3f& worldPos, const Gs::Vector3i& contactNormal, const BrickDimensions& brickDim)
    {
        constexpr float minOffset = 0.01f;

        auto AxisToOffset = [minOffset](int brickOffset, int brickExtent, int normal) -> float
        {
            const float centerOffset = static_cast<float>(1 - brickExtent % 2)*0.5f + (-brickOffset > brickExtent/2 ? 1.0f : 0.0f);
            const float baseOffset = minOffset * static_cast<float>(std::abs(normal));
            return (normal == 0 ? centerOffset + baseOffset : baseOffset);
        };

        const float xOffsetReal = AxisToOffset(brickDim.offset.x, brickDim.extent.x, contactNormal.x);
        const float yOffsetReal = minOffset;
        const float zOffsetReal = AxisToOffset(brickDim.offset.z, brickDim.extent.z, contactNormal.z);

        return Gs::Vector3i
        {
            static_cast<std::int32_t>(std::floor(worldPos.x / KDTreeNode::gridScale.x + xOffsetReal)),
            static_cast<std::int32_t>(std::floor(worldPos.y / KDTreeNode::gridScale.y + yOffsetReal)),
            static_cast<std::int32_t>(std::floor(worldPos.z / KDTreeNode::gridScale.z + zOffsetReal))
        };
    }

    // Resets the previously picked brick and returns its instance.
    Instance* ResetPreviouslySelectedBrick()
    {
        // Reset previously picked brick
        if (Instance* oldBrickInstance = currentScene->GetMutableBrickInstance(controller.pickedBrick))
        {
            oldBrickInstance->color = controller.pickedInstanceColor;
            controller.pickedBrick = {};
            return oldBrickInstance;
        }
        return nullptr;
    }

    Brick SelectBrickFromScene()
    {
        // Pick scene objects with the mouse pointer
        const Ray3f pickerRay = ProjectScreenPosToViewRay(input.GetMousePosition());

        RayIntersection contact;
        if (currentScene->PickScene(pickerRay, contact))
        {
            // Highlight new picked brick
            if (Instance* newBrickInstance = currentScene->GetMutableBrickInstance(contact.brick))
            {
                controller.pickedInstanceColor      = newBrickInstance->color;
                controller.pickedBrick              = contact.brick;

                newBrickInstance->color = LLGL::ColorRGBAub{ 255, 0, 0, 255 };
            }

            #if DEBUG_KDTREE
            controller.pickedPosition   = contact.point;
            controller.pickedNormal     = contact.normal.Cast<float>();
            #endif

            return contact.brick;
        }

        return Brick{};
    }

    void ClampBrickGridOffset(BrickType type, std::int32_t& offset)
    {
        const Gs::Vector3i brickExtent = BrickDictionary::Get().GetExtent(type);
        const std::int32_t maxWidthOrDepth = std::max(brickExtent.x, brickExtent.z);
        if (maxWidthOrDepth > 2)
        {
            const std::int32_t maxOffset = (maxWidthOrDepth + 1)/2 - 1;
            offset = Gs::Clamp(offset, -maxOffset, +maxOffset);
        }
        else
            offset = 0;
    }

    void PickGridPosForPreviewBrick()
    {
        PreviewBrick& previewBrick = viewOptions.previewBrick;

        // Allow user to turn and change type of preview brick while picking the scene
        GridRotation newPreviewRotation = previewInstance.gridRotation;
        if (input.KeyDown(LLGL::Key::Space))
            newPreviewRotation = static_cast<GridRotation>((newPreviewRotation + 1) % GridRotation_Num);

        BrickType newBrickType = previewBrick.type;
        if (input.KeyDown(LLGL::Key::PageUp))
            newBrickType = static_cast<BrickType>((newBrickType + 1) % BrickType_Num);
        if (input.KeyDown(LLGL::Key::PageDown))
            newBrickType = static_cast<BrickType>(ModuloSignInt(newBrickType - 1, BrickType_Num));

        previewBrick.gridOffset += input.GetWheelMotion();

        ClampBrickGridOffset(newBrickType, previewBrick.gridOffset);

        // Pick scene objects with the mouse pointer
        const Ray3f pickerRay = ProjectScreenPosToViewRay(input.GetMousePosition());
        Gs::Vector3i newPreviewGridPos = previewInstance.gridPos;

        RayIntersection contact;
        if (currentScene->PickScene(pickerRay, contact))
        {
            const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(newBrickType, newPreviewRotation);

            // Find a potential grid position for the preview brick
            auto ApplyNormalOffset = [](std::int32_t& gridPos, int normal, int brickOffset, int brickExtent) -> void
            {
                if (normal < 0)
                    gridPos -= (brickExtent + brickOffset);
                else if (normal > 0)
                    gridPos -= brickOffset;
            };

            if (contact.normal.x != 0)
            {
                newPreviewGridPos = WorldPosToGridPos(contact.point, contact.normal, brickDim);
                ApplyNormalOffset(newPreviewGridPos.x, contact.normal.x, brickDim.offset.x, brickDim.extent.x);
            }
            else if (contact.normal.y != 0)
            {
                newPreviewGridPos = WorldPosToGridPos(contact.point, contact.normal, brickDim);
                ApplyNormalOffset(newPreviewGridPos.y, contact.normal.y, brickDim.offset.y, brickDim.extent.y);
            }
            else if (contact.normal.z != 0)
            {
                newPreviewGridPos = WorldPosToGridPos(contact.point, contact.normal, brickDim);
                ApplyNormalOffset(newPreviewGridPos.z, contact.normal.z, brickDim.offset.z, brickDim.extent.z);
            }

            // Offset by user input to allow placing bricks on ledges
            if (brickDim.extent.z > brickDim.extent.x)
            {
                if (contact.normal.z == 0)
                    newPreviewGridPos.z += previewBrick.gridOffset;
            }
            else
            {
                if (contact.normal.x == 0)
                    newPreviewGridPos.x += previewBrick.gridOffset;
            }
        }

        // If grid position changed, re-evaluate whether that position is blocked for the preview brick
        if (previewInstance.gridPos         != newPreviewGridPos    ||
            previewInstance.gridRotation    != newPreviewRotation   ||
            previewBrick.type               != newBrickType)
        {
            previewBrick.isBlocked = !currentScene->BrickFitsInGrid(newPreviewGridPos, newBrickType, newPreviewRotation);

            // Update brick position and rotation
            previewBrick.type               = newBrickType;
            previewInstance.gridPos         = newPreviewGridPos;
            previewInstance.gridRotation    = newPreviewRotation;
        }
    }

    void ProcessEditModeAddOrMove()
    {
        PickGridPosForPreviewBrick();

        const PreviewBrick& previewBrick = viewOptions.previewBrick;
        if (!previewBrick.isBlocked && input.KeyDown(LLGL::Key::LButton))
        {
            // Place new brick and update pick position for preview
            currentScene->NewBrick(previewBrick.type, previewInstance.gridPos, previewInstance.gridRotation, previewInstance.color, previewBrick.isTransparent);
            PickGridPosForPreviewBrick();

            // Reset edit mode if it was only temporarily adding a new brick
            if (controller.editMode == EditMode::Move)
                controller.editMode = EditMode::Remove;
        }
    }

    void ProcessEditModeRemove()
    {
        if (Brick selectedBrick = SelectBrickFromScene())
        {
            if (input.KeyDown(LLGL::Key::LButton))
            {
                // Pick up brick for editing so the user can place it somewhere else
                if (const Instance* selectedInstance = currentScene->GetBrickInstance(selectedBrick))
                {
                    ResetPreviouslySelectedBrick();

                    viewOptions.previewBrick.type           = selectedBrick.GetType();
                    viewOptions.previewBrick.isTransparent  = (selectedBrick.GetGroup() == BrickGroup_Transparent);
                    previewInstance.color                   = selectedInstance->color;
                    previewInstance.gridRotation            = selectedInstance->gridRotation;

                    // Try to find picked color in color palette. If not found, it was edited externally and we keep the color picker unchanged.
                    int colorPaletteIndex = FindColorPaletteIndex(selectedInstance->color);
                    if (colorPaletteIndex >= 0 && colorPaletteIndex < sizeof(colorPalette)/sizeof(colorPalette[0]))
                        controller.pickedColorPaletteIndex = colorPaletteIndex;

                    // Change to edit mode that allows the user to move the selected brick around.
                    // Immediately update picking position of the preview brick before it is rendered with a stale position.
                    controller.editMode = EditMode::Move;
                    PickGridPosForPreviewBrick();
                }

                // Remove selected brick
                if (controller.pickedBrick == selectedBrick)
                    ResetPreviouslySelectedBrick();
                currentScene->DestroyBrick(selectedBrick);
            }
        }
    }

    void ProcessUserInput()
    {
        const LLGL::Offset2D mousePos = input.GetMousePosition();
        const bool isMouseInDeadZone = (mousePos.x < uiState.menuBarLeft || mousePos.x > uiState.menuBarRight);

        if (input.KeyDown(LLGL::Key::Tab))
            viewOptions.showWireframe = !viewOptions.showWireframe;

        ControlCameraByUserInput();

        if (input.KeyDown(LLGL::Key::D1))
            controller.editMode = EditMode::None;
        else if (input.KeyDown(LLGL::Key::D2))
            controller.editMode = EditMode::Add;
        else if (input.KeyDown(LLGL::Key::D3) || input.KeyDown(LLGL::Key::Delete))
            controller.editMode = EditMode::Remove;

        Instance* oldBrickInstance = ResetPreviouslySelectedBrick();

        // Only use mouse editing if the mouse isn't in the dead zone, i.e. covered by UI elements
        if (controller.cameraMode == CameraMode::None && !isMouseInDeadZone)
        {
            if (controller.editMode == EditMode::Add || controller.editMode == EditMode::Move)
                ProcessEditModeAddOrMove();
            else if (controller.editMode == EditMode::Remove)
                ProcessEditModeRemove();
        }

        #if DEBUG_KDTREE
        if (GetBrickInstance(controller.pickedBrick) != oldBrickInstance)
        {
            LLGL::Log::Printf(
                "Selected brick (type=%u, instance=%u): %u Ray/AABB tests\n",
                controller.pickedBrick.groupId, controller.pickedBrick.groupInstance, KDTreeNode::numAABBTests
            );
        }
        #endif
    }

    void UpdateSceneAndUIView()
    {
        camera.UpdateTransform();

        // Update common scene view states
        sceneView.viewPos           = camera.wMatrix * Gs::Vector4f{ 0, 0, 0, 1 };
        sceneView.vpMatrix          = projection * camera.vMatrix;

        sceneView.gridScale         = KDTreeNode::gridScale;

        sceneView.lightDir[0]       = Gs::Vector4f{ -0.2f, +0.4f, -1.0f, 0.0f }.Normalized();

        // Update preview shader states
      //sceneView.worldOffsetY      = previewBlockOffsetY;
        sceneView.isDithering       = (viewOptions.previewBrick.isBlocked ? 1 : 0);

        // Update outline shader states
        const LLGL::Extent2D& viewportSize = swapChain->GetResolution();
        sceneView.viewportSize[0]   = static_cast<float>(viewportSize.width);
        sceneView.viewportSize[1]   = static_cast<float>(viewportSize.height);

        // Update inverse view-projection matrix to displace depth values for parallax occlusion displacement
        sceneView.vpMatrixInv           = sceneView.vpMatrix.Inverse();
        sceneView.vpMatrixInvNoViewPos  = (projection * camera.vMatrixNoPosition).Inverse();

        // Update planar projection for UI elements
        const LLGL::Extent2D resolution = swapChain->GetResolution();
        uiView.pMatrix = Gs::ProjectionMatrix4f::Planar(static_cast<float>(resolution.width), static_cast<float>(resolution.height));
    }

    void ResetMenuBar(const LLGL::Extent2D& resolution)
    {
        // Update mouse dead zone
        uiState.menuBarLeft     = uiColorPickerOffset[0]*2 + uiColorPickerSize + uiButtonFrameSize*2;
        uiState.menuBarRight    = static_cast<int>(resolution.width) - (uiColorPickerOffset[0]*2 + uiBrickThumbnailSize + uiButtonFrameSize*2);
    }

    //////////////////// MAIN CALLBACKS ////////////////////

    void OnDrawFrame(float /*dt*/) override
    {
        #if DEBUG_KDTREE
        KDTreeNode::numAABBTests = 0;
        #endif

        ProcessUserInput();
        UpdateSceneAndUIView();

        const bool isPreviewBrickVisible = (controller.cameraMode == CameraMode::None && (controller.editMode == EditMode::Add || controller.editMode == EditMode::Move));

        commands->Begin();
        {
            // Always update instance for preview brick if its visible
            if (isPreviewBrickVisible)
                commands->UpdateBuffer(*vertexAndPreviewInstanceBuffer, previewInstanceBufferOffset, &previewInstance, sizeof(previewInstance));

            commands->UpdateBuffer(*sceneViewCbuffer, 0, &sceneView, sizeof(sceneView));
            commands->UpdateBuffer(*uiViewCbuffer, 0, &uiView, sizeof(uiView));

            UpdatePlaneGeometryForViewFrustum();

            currentScene->FlushInstanceDirtyRange(*commands);

            commands->BeginRenderPass(*swapChain);
            {
                // Clear all buffers. Stencil might be used for silhouette rendering.
                commands->Clear(LLGL::ClearFlags::All, backgroundColor);
                commands->SetViewport(swapChain->GetResolution());

                commands->PushDebugGroup("RenderBrickScene");
                {
                    RenderBrickScene(*currentScene);
                }
                commands->PopDebugGroup();

                if (isPreviewBrickVisible)
                {
                    commands->PushDebugGroup("RenderPreviewBrick");
                    {
                        BindPreviewBrickPSOStates();
                        RenderPreviewBrick(viewOptions.previewBrick);
                    }
                    commands->PopDebugGroup();
                }

                if (controller.editMode == EditMode::Remove)
                {
                    commands->PushDebugGroup("RenderSilhouette");
                    {
                        RenderSilhouette(*currentScene, controller.pickedBrick);
                    }
                    commands->PopDebugGroup();
                }

                if (controller.editMode != EditMode::None)
                {
                    commands->PushDebugGroup("RenderUIElements");
                    {
                        RenderUIElements();
                    }
                    commands->PopDebugGroup();
                }

                #if DEBUG_KDTREE
                if (controller.editMode == EditMode::Remove)
                    RenderDebugInfo();
                #endif
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);
    }

    void OnExit() override
    {
        // Only save file if it was loaded successfully.
        // We don't want to override an existing file with an empty scene because something went wrong at startup.
        if (hasLoadedScene)
        {
            // Ensure previoulsy selected brick retains its original color
            ResetPreviouslySelectedBrick();
            SaveScene(*currentScene, sceneFilename);
        }
        currentScene.reset();
    }

};

const Gs::Vector3f  KDTreeNode::gridScale    = { 1.0f, 0.5f, 1.0f };
int                 KDTreeNode::numAABBTests = 0;
AABB3i              KDTreeNode::rootAABB;


LLGL_IMPLEMENT_EXAMPLE(Example_BrickPlay);



