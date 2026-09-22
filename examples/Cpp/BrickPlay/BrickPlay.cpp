/*
 * BrickPlay.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "BrickPlay.Brick.h"
#include "BrickPlay.KDTree.h"
#include <ExampleBase.h>
#include <algorithm>
#include <LLGL/Utils/ForRange.h>
#include <set>
#include <map>

// File and string streams for simple scene file writer/reader
#include <fstream>
#include <sstream>


class Example_BrickPlay : public ExampleBase
{

    #define foreach_dimension(X, Y, Z, VEC) \
        for_range(Z, (VEC).z) for_range(Y, (VEC).y) for_range(X, (VEC).x)

    static constexpr std::uint32_t  numAllocInitialInstances    = 1;                    // Number of initially allocated instances. Must be at least 1.
    static constexpr int            boundingVolume[3]           = { 100, 60, 100 };
    static constexpr float          cameraTurnSpeed             = 0.25f;
    static constexpr float          cameraMoveSpeed             = 0.25f;
    static constexpr float          previewBlockOffsetY         = 0.075f;               // Y-axis world position offset for the preview block before putting it into place

    static constexpr float          silhouetteColor[3]          = { 1.0f, 1.0f, 0.0f }; // Color of the silhouette outline for selectin bricks
    static constexpr float          silhouetteWidth             = 5.0f;

    static constexpr std::uint32_t  maxUIVertexBatchSize        = 64 * 3;               // Maxmimum number of UI vertices before batch is flushed and send to the GPU.
    static constexpr int            uiColorPickerSize           = 25;                   // Size (in pixels) of each color picker button
    static constexpr int            uiColorPickerOffset[2]      = { 15, 15 };
    static constexpr int            uiButtonFrameSize           = 3;
    static constexpr int            uiButtonMargin              = 5;
    const LLGL::ColorRGBAub         uiButtonFrameColor          = { 0, 0, 0, 255 };

    static constexpr int            uiBrickThumbnailSize        = 64;
    const LLGL::ColorRGBAub         uiBrickThumbnailBgColor     = { 180, 180, 180, 255 };
    const LLGL::ColorRGBAub         uiMenuBgColor               = { 0, 0, 0, 180 };

    const char*                     sceneFilename               = "Bricks.txt";

    enum BrickPSO
    {
        BrickPSO_Default = 0,
        BrickPSO_Transparent,
        BrickPSO_Wireframe,
        BrickPSO_Preview, // With dithering to indicate blocked position

        BrickPSO_Num,
    };

    // Shaders to render brick instances, the brick preview model, and UI elements
    LLGL::Shader*               vsBrick                     = nullptr;
    LLGL::Shader*               fsBrick                     = nullptr;
    LLGL::Shader*               vsBrickPreview              = nullptr;
    LLGL::Shader*               fsBrickPreview              = nullptr;
    LLGL::Shader*               vsOverlay                   = nullptr;
    LLGL::Shader*               fsOverlay                   = nullptr;
    LLGL::Shader*               vsOutline                   = nullptr;
    LLGL::Shader*               gsOutline                   = nullptr;
    LLGL::Shader*               fsOutline                   = nullptr;

    LLGL::PipelineLayout*       psoLayout                   = nullptr;
    LLGL::PipelineState*        pso[BrickPSO_Num]           = {};

    #if DEBUG_KDTREE
    LLGL::PipelineState*        psoDebug                    = nullptr;
    #endif

    LLGL::PipelineLayout*       psoOutlineWriteStencilLayout    = nullptr;
    LLGL::PipelineState*        psoOutlineWriteStencil          = nullptr;

    LLGL::PipelineLayout*       psoOutlineWriteColorLayout      = nullptr;
    LLGL::PipelineState*        psoOutlineWriteColor            = nullptr;

    LLGL::PipelineLayout*       psoUILayout                 = nullptr;
    LLGL::PipelineState*        psoUI                       = nullptr;

    LLGL::Buffer*               sceneViewCbuffer            = nullptr;
    LLGL::Buffer*               vertexBuffer                = nullptr;
    LLGL::Buffer*               instanceBuffer              = nullptr;
    std::uint64_t               instanceBufferSize          = 0;

    LLGL::Buffer*               uiVertexBuffer              = nullptr;
    LLGL::Buffer*               uiViewCbuffer               = nullptr;
    LLGL::Texture*              uiBlankTexture              = nullptr;
    LLGL::Texture*              uiBrickThumbnailTexture     = nullptr;

    bool                        hasOffsetInstancing         = false;

    // The scene grid stores cells with references to the brick instances.
    // Used for scene picking, mesh optimization, and collision detection.
    KDTreeNode                  grid;

    struct UIState
    {
        // To avoid competing mouse interactions between scene picking and UI elements,
        // declare a dead zone for the scene picker when the menu is active on the left and on the right.
        // This adjusted according to screen resolution when UI elements have to shift.
        int                     mouseDeadZoneLeft           = 0;
        int                     mouseDeadZoneRight          = 0;
    }
    uiState;

    const LLGL::ColorRGBAub     colorPalette[10]            =
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
        Gs::Matrix4f    vpMatrix;                                   // View-projection matrix
        Gs::Vector3f    gridScale       = Gs::Vector3f{ 1.0f };
        float           pad0            = {};
        Gs::Vector4f    viewPos;                                    // Camera view position (in world space)
        Gs::Vector3f    lightDir        = { 0.0f, 0.0f, -1.0f };    // Light vector pointing from a pixel towards the light source
        float           specularPower   = 90.0f;                    // Specular exponent for blinn-phone shading model

        // Preview shaders only
        float           worldOffsetY    = 0.0f;
        std::int32_t    isDithering     = 0;

        // Outline shaders only
        float           viewportSize[2] = {};
        float           outlineColor[3] = { silhouetteColor[0], silhouetteColor[1], silhouetteColor[2] };
        float           outlineWidth    = silhouetteWidth;
    }
    sceneView;

    struct alignas(16) UIView
    {
        Gs::Matrix4f    pMatrix;
    }
    uiView;

    const BrickDictionary brickDictionary;

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

    // Vertex strucutre for brick instances. They are untextured, solid-colored bricks, so they don't need texture coordinates.
    struct Vertex
    {
        float position[3];
        float normal[3];
    };

    // Brick instance structure. Stores the position, rotation, and color.
    struct Instance
    {
        Gs::Vector3i        gridPos;
        GridRotation        gridRotation;
        LLGL::ColorRGBAub   color;
    };

    struct InstanceGroup
    {
        // Number of vertices to render.
        std::uint32_t numVertices       = 0;

        // Zero-based index to the first vertex into the vertex buffer.
        std::uint32_t firstVertex       = 0;

        // Number of instances to render for this group
        std::uint32_t numInstances      = 0;

        // Zero-based index to the first instance into the global `instances` container for this group. Instance 0 is reserved for the preview instance.
        std::uint32_t firstInstance     = 1;

        // Number of instances allocated for this group in the global `instances` container. This must always be greater than or equal to `numInstances`.
        std::uint32_t instanceCapacity  = 0;
    };

    std::vector<Instance>   instances;
    std::size_t             instanceDirtyRange[2]           = { 0, 0 };
    InstanceGroup           instanceGroups[BrickType_Num * BrickGroup_Num];

    struct PickerContact
    {
        Gs::Vector3f    point;
        Gs::Vector3i    normal;
        Brick           brick;
    };

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

        void Turn(float pitch, float yaw)
        {
            this->pitch = Gs::Clamp(this->pitch + pitch, -90.0f, +90.0f);
            this->yaw   = this->yaw + yaw;
        }

        void Move(const Gs::Vector3f& direction)
        {
            Gs::Matrix3f rotation;
            Gs::RotateY(rotation, Gs::Deg2Rad(yaw));
            Gs::RotateX(rotation, Gs::Deg2Rad(pitch));
            position += rotation * direction;
        }

        void UpdateTransform()
        {
            wMatrix.LoadIdentity();
            Gs::Translate(wMatrix, position);
            Gs::RotateY(wMatrix, Gs::Deg2Rad(yaw));
            Gs::RotateX(wMatrix, Gs::Deg2Rad(pitch));

            vMatrix = wMatrix.Inverse();
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
        CreateBuffers();
        CreatePipelines();

        // Render all bricks into thumbnail render-targets for the UI
        RenderAllBrickThumbnails();

        // Load previoulsy saved scene or create default initial scene on failure.
        if (!LoadScene(sceneFilename))
            CreateDefaultScene();

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

    //////////////////// SCENE CONSTRUCTION ////////////////////

    InstanceGroup& GetInstanceGroup(BrickType type, BrickGroup group = BrickGroup_Default)
    {
        return instanceGroups[CombineToBrickId(type, group)];
    }

    void MarkInstanceDirty(std::size_t first, std::size_t count = 1)
    {
        instanceDirtyRange[0] = std::min(instances.size(), std::min(instanceDirtyRange[0], first));
        instanceDirtyRange[1] = std::min(instances.size(), std::max(instanceDirtyRange[1], first + count));
    }

    void ResetInstanceDirtyRange()
    {
        instanceDirtyRange[0] = static_cast<std::size_t>(-1);
        instanceDirtyRange[1] = 0;
    }

    void ReleaseInstanceBuffer()
    {
        if (instanceBuffer != nullptr)
        {
            renderer->Release(*instanceBuffer);
            instanceBuffer = nullptr;
        }
    }

    // Resizes the instance buffer, but only increases in size.
    bool ResizeInstanceBuffer(std::uint64_t newSize)
    {
        if (instanceBufferSize < newSize)
        {
            // Resize buffer if new required size is larger than the current instance buffer
            ReleaseInstanceBuffer();

            LLGL::BufferDescriptor instBufferDesc;
            {
                instBufferDesc.debugName    = "Instances";
                instBufferDesc.size         = newSize;
                instBufferDesc.stride       = sizeof(Instance);
                instBufferDesc.bindFlags    = LLGL::BindFlags::VertexBuffer;
            }
            instanceBuffer = renderer->CreateBuffer(instBufferDesc);

            // Store new instance buffer size
            instanceBufferSize = newSize;

            return true;
        }
        return false;
    }

    // Adds the specified number of instances to the group.
    Instance* AllocateInstancesInGroup(std::uint32_t groupId, std::uint32_t count = 1)
    {
        InstanceGroup& group = instanceGroups[groupId];
        if (group.numInstances + count > group.instanceCapacity)
        {
            const std::uint32_t numMinCount = group.numInstances + count;
            const std::uint32_t numCapacity = numMinCount + numMinCount/2;

            // Insert new instances into global container
            const std::uint32_t instanceGrowth = numCapacity - group.instanceCapacity;
            instances.insert(instances.begin() + group.firstInstance + group.numInstances, instanceGrowth, Instance{});

            // Resize GPU buffer and mark all instances as dirty
            if (ResizeInstanceBuffer(instances.size() * sizeof(Instance)))
                MarkInstanceDirty(0, instances.size());

            // Update instance offsets in subsequent groups
            for (int nextGroupId = groupId + 1; nextGroupId < BrickType_Num * BrickGroup_Num; ++nextGroupId)
                instanceGroups[nextGroupId].firstInstance += instanceGrowth;

            // Store new instance capacity for this group
            group.instanceCapacity = numCapacity;
        }

        // Return pointer to first new instance
        Instance* firstInstancePtr = &(instances[group.firstInstance + group.numInstances]);
        MarkInstanceDirty(group.firstInstance + group.numInstances, count);
        group.numInstances += count;

        return firstInstancePtr;
    }

    // This function must not take `gridPos` as constant reference parameter.
    // When it comes from an instance (like the preview model), it will invalidate that pointer in AllocateInstancesInGroup().
    Brick AddInstanceToGroup(std::uint32_t groupId, Gs::Vector3i gridPos, GridRotation gridRotation = GridRotation_0, LLGL::ColorRGBAub color = {}, bool notifyNeighbors = false)
    {
        Brick newBrick{ groupId, instanceGroups[groupId].numInstances };

        if (Instance* newInstance = AllocateInstancesInGroup(groupId))
        {
            newInstance->gridPos        = gridPos;
            newInstance->gridRotation   = gridRotation;
            newInstance->color          = color;

            if (!InsertBrickIntoGrid(newBrick, notifyNeighbors))
            {
                LLGL::Log::Errorf(
                    "Failed to insert brick (groupId=%u, groupInstance=%u) into grid\n",
                    newBrick.groupId, newBrick.groupInstance
                );
            }
        }

#if 1 //TEST
        ValidateInstances();
#endif

        return newBrick;
    }

    bool RemoveInstanceFromGroup(std::uint32_t groupId, std::uint32_t groupInstance, Instance* outRemovedInstance = nullptr, bool notifyNeighbors = false)
    {
        // Early exit if instance is out of bounds
        InstanceGroup& group = instanceGroups[groupId];
        if (groupInstance >= group.numInstances)
            return false;

        // Make copy of retired instance and override that entry with the last instance below.
        // RemoveBrickFromGrid()/InsertBrickIntoGrid() might modify the instances container, so this has to happen first.
        const Instance retiredInstance = instances[group.firstInstance + groupInstance];

        if (outRemovedInstance != nullptr)
            *outRemovedInstance = retiredInstance;

        const BrickType brickType = IdToBrickType(groupId);

        group.numInstances--;
        if (groupInstance < group.numInstances)
        {
            // Remove instance from group by writing last instance in that group into the position of the old instance and update its grid entry
            const Instance& lastInstance = instances[group.firstInstance + group.numInstances];
            instances[group.firstInstance + groupInstance] = lastInstance;
            UpdateBrickInstanceInGrid(brickType, lastInstance, groupInstance);
        }

        // Remove destroyed instance from grid
        RemoveBrickFromGrid(brickType, retiredInstance, notifyNeighbors);

        // Mark all remaining instances of that group as dirty
        MarkInstanceDirty(group.firstInstance + groupInstance);

        return true;
    }

    // Transfers the specified brick to a new group and returns the updated brick entry
    void TransferBrickGroup(Brick brick, BrickGroup newGroup)
    {
        Instance oldInstance;
        if (RemoveInstanceFromGroup(brick.groupId, brick.groupInstance, &oldInstance))
        {
            const std::uint32_t newGroupId = CombineToBrickId(brick.GetType(), newGroup);
            (void)AddInstanceToGroup(newGroupId, oldInstance.gridPos, oldInstance.gridRotation, oldInstance.color);
        }
    }

    void FlushInstanceDirtyRange()
    {
        // Update instances within the range that is marked as dirty
        if (instanceDirtyRange[0] < instanceDirtyRange[1])
        {
            LLGL_VERIFY(instanceBuffer != nullptr);

            commands->UpdateBuffer(
                *instanceBuffer,
                instanceDirtyRange[0]*sizeof(Instance),
                &(instances[instanceDirtyRange[0]]),
                (instanceDirtyRange[1] - instanceDirtyRange[0])*sizeof(Instance)
            );

            ResetInstanceDirtyRange();
        }
    }

    std::size_t BrickToInstance(Brick brick)
    {
        return (brick ? instanceGroups[brick.groupId].firstInstance + brick.groupInstance : static_cast<std::size_t>(-1));
    }

    const Instance* GetBrickInstance(Brick brick)
    {
        const std::size_t instanceIndex = BrickToInstance(brick);
        return (instanceIndex < instances.size() ? &(instances[instanceIndex]) : nullptr);
    }

    Instance* GetMutableBrickInstance(Brick brick)
    {
        const std::size_t instanceIndex = BrickToInstance(brick);
        if (instanceIndex < instances.size())
        {
            MarkInstanceDirty(instanceIndex);
            return &(instances[instanceIndex]);
        }
        return nullptr;
    }

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

    // Projects the specified screeen coordinate into a ray originating from the camera view to pick elements in the scene.
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

        const Gs::Matrix4f vMatrixNoTranslation
        {
            camera.vMatrix(0, 0), camera.vMatrix(0, 1), camera.vMatrix(0, 2), 0.0f,
            camera.vMatrix(1, 0), camera.vMatrix(1, 1), camera.vMatrix(1, 2), 0.0f,
            camera.vMatrix(2, 0), camera.vMatrix(2, 1), camera.vMatrix(2, 2), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        Gs::Matrix4f vpMatrixInv = projection * vMatrixNoTranslation;
        vpMatrixInv.MakeInverse();

        ndc = vpMatrixInv * ndc;

        return Ray3f
        {
            Gs::TransformVector(camera.wMatrix, Gs::Vector3f{}),
            Gs::Vector3f{ ndc }.Normalized()
        };
    }

    BrickDimensions GetBrickDimensions(BrickType type, GridRotation rotation)
    {
        BrickDimensions brickDim;

        brickDim.extent = brickDictionary.GetExtent(type);
        if (rotation == GridRotation_90 || rotation == GridRotation_270)
            std::swap(brickDim.extent.x, brickDim.extent.z);

        brickDim.offset.x = ((rotation == GridRotation_90 || rotation == GridRotation_180) ? -(brickDim.extent.x + 1)/2 : -brickDim.extent.x/2);
        brickDim.offset.y = 0;
        brickDim.offset.z = ((rotation == GridRotation_180 || rotation == GridRotation_270) ? -(brickDim.extent.z + 1)/2 : -brickDim.extent.z/2);

        return brickDim;
    }

    bool BrickFitsInGrid(const Gs::Vector3i& gridPos, BrickType type, GridRotation rotation)
    {
        // Get brick dimension and apply rotation
        const BrickDimensions brickDim = GetBrickDimensions(type, rotation);

        // Brick must fit inside the root AABB
        const Gs::Vector3i alignedGridPos = gridPos + brickDim.offset;
        if (alignedGridPos.x                     < KDTreeNode::rootAABB.bbMin.x ||
            alignedGridPos.y                     < KDTreeNode::rootAABB.bbMin.y ||
            alignedGridPos.z                     < KDTreeNode::rootAABB.bbMin.z ||
            alignedGridPos.x + brickDim.extent.x > KDTreeNode::rootAABB.bbMax.x ||
            alignedGridPos.y + brickDim.extent.y > KDTreeNode::rootAABB.bbMax.y ||
            alignedGridPos.z + brickDim.extent.z > KDTreeNode::rootAABB.bbMax.z)
        {
            return false;
        }

        // Check if there is any blocking brick
        foreach_dimension(x, y, z, brickDim.extent)
        {
            if (grid.GetLeaf(alignedGridPos + Gs::Vector3i{ x, y, z }))
                return false;
        }

        return true;
    }

    bool IsBrickTypeBlockedTopAndBottom(BrickType type, const Gs::Vector3i& gridPos, GridRotation gridRotation)
    {
        const BrickDimensions brickDim = GetBrickDimensions(type, gridRotation);
        const Gs::Vector3i alignedGridPos = gridPos + brickDim.offset;
        for_range(yi, 2)
        {
            const std::int32_t y = (yi == 0 ? -1 : brickDim.extent.y);

            // If current grid position is under the floor, it's also considered blocked
            if (alignedGridPos.y + y < KDTreeNode::rootAABB.bbMin.y)
                continue;

            for_range(z, brickDim.extent.z)
            {
                for_range(x, brickDim.extent.x)
                {
                    Brick otherBrick = grid.GetLeaf(alignedGridPos + Gs::Vector3i{ x, y, z });
                    if (!otherBrick.IsBlocking())
                        return false;
                }
            }
        }
        return true;
    }

    bool IsBrickBlockedTopAndBottom(Brick brick)
    {
        if (const Instance* instance = GetBrickInstance(brick))
            return IsBrickTypeBlockedTopAndBottom(brick.GetType(), instance->gridPos, instance->gridRotation);
        else
            return false;
    }

    // Notifies the specified brick that its surrounding has changed,
    // allowing it to transfer to a different mesh group for better rendering performance.
    void NotifyBrickChange(Brick brick, bool isBlocking)
    {
        // Only bother to change groups for brick types that support the walls-only group
        if (GetInstanceGroup(brick.GetType(), BrickGroup_WallsOnly).numVertices > 0)
        {
            const BrickGroup group = brick.GetGroup();
            if (group == BrickGroup_Default && isBlocking)
            {
                // If brick is now blocked top and bottom, switch to walls-only mesh
                if (IsBrickBlockedTopAndBottom(brick))
                    TransferBrickGroup(brick, BrickGroup_WallsOnly);
            }
            else if (group == BrickGroup_WallsOnly && !isBlocking)
            {
                // Brick is no longer fully blocked top and bottom, so switch back to default mesh
                TransferBrickGroup(brick, BrickGroup_Default);
            }
        }
    }

    // Notifes all bricks surrounding the specified changed brick that their surrounding has changed.
    void NotifyBrickChangeTopAndBottom(const Gs::Vector3i alignedGridPos, const Gs::Vector3i& brickExtent, Brick changedBrick)
    {
        // First gather all brick cells that need to be notified to avoid running the blocking test multiple times for the same brick.
        // Use their cell coordinates instead of storing the brick references as they might be invalidated in the next phase.
        std::set<Brick> visitedBricks;
        std::vector<Gs::Vector3i> bricksToNotifyCells;

        for_range(yi, 2)
        {
            const std::int32_t y = (yi == 0 ? -1 : brickExtent.y);
            for_range(z, brickExtent.z)
            {
                for_range(x, brickExtent.x)
                {
                    const Gs::Vector3i gridPos = alignedGridPos + Gs::Vector3i{ x, y, z };
                    if (Brick brick = grid.GetLeaf(gridPos))
                    {
                        if (visitedBricks.insert(brick).second)
                            bricksToNotifyCells.push_back(gridPos);
                    }
                }
            }
        }

        // Now notify the bricks that their surrounding has changed
        const bool isBlocking = changedBrick.IsBlocking();
        for (const Gs::Vector3i& gridPos : bricksToNotifyCells)
        {
            if (Brick brick = grid.GetLeaf(gridPos))
                NotifyBrickChange(brick, isBlocking);
        }
    }

    // Returns true if the specified cell (position relative to brick, not to the grid) is excluded from the brick type.
    // This is used for a few select bricks that don't cover a full bounding box, like the roof tile.
    bool IsBrickCellExcluded(BrickType brickType, GridRotation brickRotation, const Gs::Vector3i& cellPos)
    {
        if (brickType == BrickType_2x3x3_Roof)
        {
            // This is hardcoded but if more bricks are added, this should be generalized.
            // The 2x3x3 roof tile has two cells that can be excluded, but the coordinate depends on its rotation.
            if (cellPos.y == 2)
            {
                if ((brickRotation == GridRotation_0   && cellPos.z == 2) ||
                    (brickRotation == GridRotation_90  && cellPos.x == 0) ||
                    (brickRotation == GridRotation_180 && cellPos.z == 0) ||
                    (brickRotation == GridRotation_270 && cellPos.x == 2))
                {
                    return true;
                }
            }
        }
        return false;
    }

    bool InsertBrickIntoGrid(Brick brick, bool notifyNeighbors = false)
    {
        // Get brick dimension and apply rotation
        if (const Instance* instance = GetBrickInstance(brick))
        {
            const BrickType brickType = brick.GetType();
            const BrickDimensions brickDim = GetBrickDimensions(brickType, instance->gridRotation);

            // Insert brick entries into grid
            const Gs::Vector3i alignedGridPos = instance->gridPos + brickDim.offset;
            foreach_dimension(x, y, z, brickDim.extent)
            {
                const Gs::Vector3i cellPos{ x, y, z };
                if (!IsBrickCellExcluded(brickType, instance->gridRotation, cellPos))
                    grid.InsertLeaf(alignedGridPos + cellPos, brick);
            }

            // Notify top and bottom grid entries that they are now blocked, so they can optimize their mesh if they are now fully blocked
            if (notifyNeighbors)
                NotifyBrickChangeTopAndBottom(alignedGridPos, brickDim.extent, brick);

            #if DEBUG_KDTREE
            LLGL::Log::Printf("kd-Tree debug info: Nodes=%u, Leafs=%u\n", grid.NumNodes(), grid.NumLeafs());
            #endif

            return true;
        }
        return false;
    }

    void RemoveBrickFromGrid(BrickType type, const Instance& instance, bool notifyNeighbors = false)
    {
        // Get brick dimension and apply rotation
        const BrickDimensions brickDim = GetBrickDimensions(type, instance.gridRotation);

        // Remove brick entries from grid
        const Gs::Vector3i alignedGridPos = instance.gridPos + brickDim.offset;
        foreach_dimension(x, y, z, brickDim.extent)
            grid.RemoveLeaf(alignedGridPos + Gs::Vector3i{ x, y, z });

        // Notify top and bottom grid entries that they are now unblocked, so they need to switch back to the full mesh
        if (notifyNeighbors)
            NotifyBrickChangeTopAndBottom(alignedGridPos, brickDim.extent, Brick{});

        #if DEBUG_KDTREE
        LLGL::Log::Printf("kd-Tree debug info: Nodes=%u, Leafs=%u\n", grid.NumNodes(), grid.NumLeafs());
        #endif
    }

    void UpdateBrickInstanceInGrid(BrickType type, const Instance& instance, std::uint32_t groupInstance)
    {
        // Get brick dimension and apply rotation
        const BrickDimensions brickDim = GetBrickDimensions(type, instance.gridRotation);

        // Remove brick entries from grid
        const Gs::Vector3i alignedGridPos = instance.gridPos + brickDim.offset;
        foreach_dimension(x, y, z, brickDim.extent)
            grid.UpdateLeaf(alignedGridPos + Gs::Vector3i{ x, y, z }, groupInstance);
    }

    Brick NewBrick(BrickType type, const Gs::Vector3i& gridPos, GridRotation gridRotation = GridRotation_0, LLGL::ColorRGBAub color = {}, bool isTransparent = false)
    {
        if (!BrickFitsInGrid(gridPos, type, gridRotation))
            return Brick{};

        const BrickGroup group =
        (
            isTransparent
                ? BrickGroup_Transparent
                : IsBrickTypeBlockedTopAndBottom(type, gridPos, gridRotation)
                    ? BrickGroup_WallsOnly
                    : BrickGroup_Default
        );
        const std::uint32_t groupId = CombineToBrickId(type, group);

        // Insert brick without notification first, so its instance is fully initialized before neighbors can rearrange the groups
        constexpr bool notifyNeighbors = true;
        return AddInstanceToGroup(groupId, gridPos, gridRotation, color, notifyNeighbors);
    }

    void DestroyBrick(Brick brick)
    {
        if (controller.pickedBrick == brick)
            ResetPreviouslySelectedBrick();
        constexpr bool notifyNeighbors = true;
        RemoveInstanceFromGroup(brick.groupId, brick.groupInstance, nullptr, notifyNeighbors);
    }

    void ResetScene()
    {
        // Allocate preview instance
        instances.resize(1);
        instances[0].color = LLGL::ColorRGBAub{ 255, 255, 255, 128 };

        // Reset instance range of all instance groups (not the vertex range)
        for (std::uint32_t groupId = 0; groupId < BrickType_Num * BrickGroup_Num; ++groupId)
        {
            InstanceGroup& group = instanceGroups[groupId];
            group.numInstances      = 0;
            group.firstInstance     = 1;
            group.instanceCapacity  = 0;
        }

        // Destroy the entire kd-Tree
        grid.Reset();
    }

    void LoadBrickModel(const char* basename, std::vector<Vertex>& vertices, InstanceGroup& outInstanceGroup)
    {
        LLGL::Log::Printf("Load brick model: %s\n", basename);

        const std::vector<TexturedVertex> intermediateVertices = Load3DModel(std::string("Bricks/") + basename + ".obj");

        outInstanceGroup.firstVertex = static_cast<std::uint32_t>(vertices.size());
        outInstanceGroup.numVertices = static_cast<std::uint32_t>(intermediateVertices.size());

        vertices.resize(vertices.size() + intermediateVertices.size());

        auto ConvertVertex = [](Vertex& dst, const TexturedVertex& src) -> void
        {
            auto ConvertVector = [](float (&vout)[3], const Gs::Vector3f& vin) -> void
            {
                vout[0] = vin.x;
                vout[1] = vin.y;
                vout[2] = vin.z;
            };
            ConvertVector(dst.position, src.position);
            ConvertVector(dst.normal, src.normal);
        };

        for (std::uint32_t i = 0; i < outInstanceGroup.numVertices; ++i)
            ConvertVertex(vertices[outInstanceGroup.firstVertex + i], intermediateVertices[i]);
    }

    void LoadAllBrickModels(std::vector<Vertex>& vertices)
    {
        LoadBrickModel("Brick1x1x1"          , vertices, GetInstanceGroup(BrickType_1x1x1));
        LoadBrickModel("Brick1x1x1_WallsOnly", vertices, GetInstanceGroup(BrickType_1x1x1, BrickGroup_WallsOnly));

        LoadBrickModel("Tile1x1"             , vertices, GetInstanceGroup(BrickType_1x1x1_Tile));

        LoadBrickModel("Brick1x1x3"          , vertices, GetInstanceGroup(BrickType_1x1x3));
        LoadBrickModel("Brick1x1x3_WallsOnly", vertices, GetInstanceGroup(BrickType_1x1x3, BrickGroup_WallsOnly));

        LoadBrickModel("Brick1x2x1"          , vertices, GetInstanceGroup(BrickType_1x2x1));
        LoadBrickModel("Brick1x2x1_WallsOnly", vertices, GetInstanceGroup(BrickType_1x2x1, BrickGroup_WallsOnly));

        LoadBrickModel("Brick1x2x3"          , vertices, GetInstanceGroup(BrickType_1x2x3));
        LoadBrickModel("Brick1x2x3_WallsOnly", vertices, GetInstanceGroup(BrickType_1x2x3, BrickGroup_WallsOnly));

        LoadBrickModel("Brick2x2x1"          , vertices, GetInstanceGroup(BrickType_2x2x1));
        LoadBrickModel("Brick2x2x1_WallsOnly", vertices, GetInstanceGroup(BrickType_2x2x1, BrickGroup_WallsOnly));

        LoadBrickModel("Tile2x2"             , vertices, GetInstanceGroup(BrickType_2x2x1_Tile));

        LoadBrickModel("Brick2x2x3"          , vertices, GetInstanceGroup(BrickType_2x2x3));
        LoadBrickModel("Brick2x2x3_WallsOnly", vertices, GetInstanceGroup(BrickType_2x2x3, BrickGroup_WallsOnly  ));
        LoadBrickModel("Brick2x2x3_Glass"    , vertices, GetInstanceGroup(BrickType_2x2x3, BrickGroup_Transparent));

        LoadBrickModel("Roof2x2x3"           , vertices, GetInstanceGroup(BrickType_2x2x3_Roof));
        LoadBrickModel("Roof2x3x3"           , vertices, GetInstanceGroup(BrickType_2x3x3_Roof));

        LoadBrickModel("Brick4x2x1"          , vertices, GetInstanceGroup(BrickType_4x2x1));
        LoadBrickModel("Brick4x2x1_WallsOnly", vertices, GetInstanceGroup(BrickType_4x2x1, BrickGroup_WallsOnly  ));

        LoadBrickModel("Brick4x2x3"          , vertices, GetInstanceGroup(BrickType_4x2x3));
        LoadBrickModel("Brick4x2x3_WallsOnly", vertices, GetInstanceGroup(BrickType_4x2x3, BrickGroup_WallsOnly  ));
        LoadBrickModel("Brick4x2x3_Glass"    , vertices, GetInstanceGroup(BrickType_4x2x3, BrickGroup_Transparent));
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
        projectionParams.zFar = 500.0f;
        UpdateProjection(swapChain->GetResolution());

        // Cache feature support for offset instancing as this will be used every frame
        const LLGL::RenderingCapabilities& caps = renderer->GetRenderingCaps();
        hasOffsetInstancing = caps.features.hasOffsetInstancing;
        ResetInstanceDirtyRange();

        if (caps.limits.lineWidthRange[1] >= 2.0f)
            selectionRenderMode = SelectionRenderMode::NativeLineWidth;
        else if (caps.features.hasGeometryShaders)
            selectionRenderMode = SelectionRenderMode::EmulatedLineWidth;
        else
            selectionRenderMode = SelectionRenderMode::Simplified;
    }

    void CreateBuffers()
    {
        const float projZAxis = GetProjectionZAxis();

        std::vector<Vertex> vertices;
        LoadAllBrickModels(vertices);

        // Create buffer for per-vertex data
        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.debugName = "BrickVertexBuffer";
            vertexBufferDesc.size      = sizeof(Vertex) * vertices.size();
            vertexBufferDesc.stride    = sizeof(Vertex);
            vertexBufferDesc.bindFlags = LLGL::BindFlags::VertexBuffer;
        }
        vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices.data());

        // Create initial instance buffer
        static_assert(numAllocInitialInstances > 0, "numAllocInitialInstances must be greater than or equal to 1");
        ResizeInstanceBuffer(sizeof(Instance) * numAllocInitialInstances);

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

        // Initialize scene by restting it. This initializes the instance for the preview mesh.
        ResetScene();
    }

    void CreatePipelines()
    {
        // Create shaders
        vsBrick         = LoadVertexShader  ("BrickPlay", "VSBrick");
        fsBrick         = LoadFragmentShader("BrickPlay", "PSBrick");
        vsBrickPreview  = LoadVertexShader  ("BrickPlay", "VSBrickPreview");
        fsBrickPreview  = LoadFragmentShader("BrickPlay", "PSBrickPreview");
        vsOverlay       = LoadVertexShader  ("BrickPlay", "VSOverlay");
        fsOverlay       = LoadFragmentShader("BrickPlay", "PSOverlay");

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
            LLGL::VertexAttribute{ "position", LLGL::Format::RGB32Float, /*location:*/ 0, /*offset:*/ offsetof(Vertex, position), /*stride:*/ sizeof(Vertex), /*slot:*/ 0 },
            LLGL::VertexAttribute{ "normal",   LLGL::Format::RGB32Float, /*location:*/ 1, /*offset:*/ offsetof(Vertex, normal  ), /*stride:*/ sizeof(Vertex), /*slot:*/ 0 },

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
        ReportPSOErrors(pso[BrickPSO_Default]);

        // Create PSO for wrieframe view mode
        {
            psoDesc.rasterizer.polygonMode = LLGL::PolygonMode::Wireframe;
        }
        pso[BrickPSO_Wireframe] = renderer->CreatePipelineState(psoDesc);
        ReportPSOErrors(pso[BrickPSO_Wireframe]);

        // Create PSO for transparent bricks
        LLGL::GraphicsPipelineDescriptor psoTransparentDesc;
        {
            SetPSODefaults(psoTransparentDesc);
            psoTransparentDesc.depth.writeEnabled               = false;
            psoTransparentDesc.blend.targets[0].blendEnabled    = true;
        }
        pso[BrickPSO_Transparent] = renderer->CreatePipelineState(psoTransparentDesc);
        ReportPSOErrors(pso[BrickPSO_Transparent]);

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
        ReportPSOErrors(pso[BrickPSO_Preview]);

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
        ReportPSOErrors(psoDebug);
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
        ReportPSOErrors(psoUI);

        if (selectionRenderMode != SelectionRenderMode::Simplified)
        {
            // Create PSO to render silhouette outline when selecting bricks
            psoOutlineWriteStencilLayout = renderer->CreatePipelineLayout(
                LLGL::Parse(
                    "cbuffer(SceneView@2):vert,"
                )
            );

            vsOutline = LoadVertexShader  ("BrickPlay", "VSBrickOutline");
            fsOutline = LoadFragmentShader("BrickPlay", "PSBrickOutline");

            // Create PSO to write stencil to mask out pixels that are inside the silhouette
            LLGL::GraphicsPipelineDescriptor psoOutlineDesc;
            {
                psoOutlineDesc.debugName                        = "EmulatedLineWidthPSO-WriteStencil";
                psoOutlineDesc.pipelineLayout                   = psoOutlineWriteStencilLayout;
                psoOutlineDesc.inputVertexAttribs               = vertexAttribs;
                psoOutlineDesc.renderPass                       = swapChain->GetRenderPass();
                psoOutlineDesc.vertexShader                     = vsOutline;

                // Enable stenicil test and write out a reference value of 1
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
            ReportPSOErrors(psoOutlineWriteStencil);

            // Create PSO to read stencil and write color output for silhouette
            if (selectionRenderMode == SelectionRenderMode::NativeLineWidth)
            {
                // Use native support for line width
                psoOutlineWriteColorLayout = renderer->CreatePipelineLayout(
                    LLGL::Parse(
                        "cbuffer(SceneView@2):vert:frag,"
                    )
                );

                psoOutlineDesc.pipelineLayout                   = psoOutlineWriteColorLayout;
                psoOutlineDesc.rasterizer.lineWidth             = silhouetteWidth;
                psoOutlineDesc.rasterizer.polygonMode           = LLGL::PolygonMode::Wireframe;
            }
            else
            {
                // Emulate line width using a geometry shader
                gsOutline = LoadGeometryShader("BrickPlay", "GSBrickOutline");

                psoOutlineWriteColorLayout = renderer->CreatePipelineLayout(
                    LLGL::Parse(
                        "cbuffer(SceneView@2):vert:geom:frag,"
                    )
                );

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
            ReportPSOErrors(psoOutlineWriteColor);
        }
    }

    void ValidateInstances()
    {
#if _WIN32 && 1 //TEST
        for_range(brickType, BrickType_Num)
        {
            for_range(groupType, BrickGroup_Num)
            {
                const InstanceGroup& group = GetInstanceGroup(static_cast<BrickType>(brickType), static_cast<BrickGroup>(groupType));
                for_range(i, group.numInstances)
                {
                    const Instance& instance = instances[group.firstInstance + i];
                    if (instance.gridPos.x < -10000 || instance.gridPos.y < -10000 || instance.gridPos.z < -10000 ||
                        instance.gridPos.x > +10000 || instance.gridPos.y > +10000 || instance.gridPos.z > +10000)
                    {
                        __debugbreak();
                    }
                }
            }
        }
#endif
    }

    bool SaveScene(const char* filename)
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
                const InstanceGroup& group = GetInstanceGroup(static_cast<BrickType>(brickType), static_cast<BrickGroup>(groupType));
                if (group.numInstances > 0)
                {
                    if (lastWrittenBrickType != brickType)
                    {
                        lastWrittenBrickType = static_cast<BrickType>(brickType);
                        file << "b " << brickDictionary.TypeToString(static_cast<BrickType>(brickType)) << std::endl;
                    }
                }

                // Write all instances
                for_range(i, group.numInstances)
                {
                    const Instance& instance = instances[group.firstInstance + i];
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

    bool LoadScene(const char* filename)
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
                    ResetScene();
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
                NewBrick(currentBrickType, gridPos + currentOffset, static_cast<GridRotation>(gridRotation / 90), ColorFromString(colorStr));
            }
            else if (cmd == 'b')
            {
                // Parse brick type and make it current
                std::string brickTypeStr;
                stream >> brickTypeStr;
                currentBrickType = brickDictionary.TypeFromString(brickTypeStr.c_str());

                if (currentBrickType == BrickType_Num)
                {
                    LLGL::Log::Errorf("Unknown brick type 'b %s' in %s:%d\n", brickTypeStr.c_str(), filename, lineNo);
                    ResetScene();
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
                ResetScene();
                return false;
            }
        }

        LLGL::Log::Printf("Loaded scene from file: %s\n", filename);
        return true;
    }

    void CreateDefaultScene()
    {
        // Place initial camera position
        camera.position = { 0, 10.0f, -20.0f };
        camera.pitch    = 20.0f;

        //TODO

        for (int i = 0; i <= 4; ++i)
        {
            for (int j = 0; j <= 4 - i; ++j)
            {
                NewBrick(BrickType_4x2x3, { 4*(i-2)+j*2, j*3,  0 }, GridRotation_0, colorPalette[i%5]);
            }
        }

        NewBrick(BrickType_4x2x1, { -9, 3, -4 }, GridRotation_90, colorPalette[1]);
        NewBrick(BrickType_4x2x1, { -9, 3, 0 }, GridRotation_90, colorPalette[1]);
        NewBrick(BrickType_4x2x1, { -9, 3, +4 }, GridRotation_90, colorPalette[1]);
        //NewBrick(BrickType_2x2x3, { 0, 0, -2 }, GridRotation_0, colorPalette[1]);

    }

    //////////////////// RENDERING ////////////////////

    void BindInstancePSO(BrickPSO psoType)
    {
        commands->SetPipelineState(*pso[psoType]);
        commands->SetResource(0, *sceneViewCbuffer);
    }

    void RenderBrickInstanceGroup(const InstanceGroup& group, LLGL::VertexBufferView (&vertexBufferViews)[2])
    {
        if (hasOffsetInstancing)
        {
            // Draw all group instances
            commands->DrawInstanced(group.numVertices, group.firstVertex, group.numInstances, group.firstInstance);
        }
        else
        {
            // Bind vertex buffers with instance offset if not natively supported
            vertexBufferViews[1].offset = group.firstInstance * sizeof(Instance);
            commands->SetVertexBuffers(2, vertexBufferViews);
            commands->DrawInstanced(group.numVertices, group.firstVertex, group.numInstances);
        }
    }

    void RenderBrickInstances()
    {
        // Exit early if no instances have been allocated yet
        if (instanceBuffer == nullptr)
            return;

        // Set vertex buffers (vertices and instances)
        LLGL::VertexBufferView vertexBufferViews[] =
        {
            LLGL::VertexBufferView{ vertexBuffer },
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
                const InstanceGroup& group = instanceGroups[CombineToBrickId(brickType, groupType)];
                if (group.numInstances > 0)
                {
                    const std::string debugGroup = "InstanceGroup" + std::to_string(groupId);
                    commands->PushDebugGroup(debugGroup.c_str());
                    {
                        RenderBrickInstanceGroup(group, vertexBufferViews);
                    }
                    commands->PopDebugGroup();
                }
            }
        }
    }

    void RenderPreviewBrick(const PreviewBrick& previewBrick)
    {
        // Not every combination of brick type and group is supported. If number of vertices is 0, exit early.
        const InstanceGroup& group = GetInstanceGroup(previewBrick.type, previewBrick.isTransparent ? BrickGroup_Transparent : BrickGroup_Default);
        if (group.numVertices == 0)
            return;

        commands->SetPipelineState(*pso[BrickPSO_Preview]);
        commands->SetResource(0, *sceneViewCbuffer);

        // Set vertex buffers (vertices and instances).
        // We don't care about offset instancing here, because the preview instance is always the first one (index 0).
        LLGL::VertexBufferView vertexBufferViews[] =
        {
            LLGL::VertexBufferView{ vertexBuffer },
            LLGL::VertexBufferView{ instanceBuffer },
        };
        commands->SetVertexBuffers(2, vertexBufferViews);

        commands->Draw(group.numVertices, group.firstVertex);
    }

    void RenderSilhouette(Brick brick)
    {
        const Instance* brickInstance = GetBrickInstance(brick);
        if (brickInstance == nullptr)
            return;

        const InstanceGroup& visibleGroup = GetInstanceGroup(brick.GetType(), BrickGroup_Default);
        if (visibleGroup.numVertices == 0)
            return;

        const InstanceGroup& pickedGroup = instanceGroups[brick.groupId];
        if (pickedGroup.numVertices == 0)
            return;

        // Bind vertex buffers for single instance. No need to rely on offset instancing support, just always use vertex buffer offset.
        LLGL::VertexBufferView vertexBufferViews[] =
        {
            LLGL::VertexBufferView{ vertexBuffer },
            LLGL::VertexBufferView{ instanceBuffer, 0, (pickedGroup.firstInstance + brick.groupInstance) * sizeof(Instance) },
        };
        commands->SetVertexBuffers(2, vertexBufferViews);

        // First render stencil reference to mask out pixels inside the silhouette. This is the regular brick model.
        commands->PushDebugGroup("RenderSilhouetteStencil");
        {
            commands->SetPipelineState(*psoOutlineWriteStencil);
            commands->SetResource(0, *sceneViewCbuffer);
            commands->Draw(visibleGroup.numVertices, visibleGroup.firstVertex);
        }
        commands->PopDebugGroup();

        // Now render silhouette by rendering with a wireframe model with thick lines
        commands->PushDebugGroup("RenderSilhouetteColor");
        {
            commands->SetPipelineState(*psoOutlineWriteColor);
            commands->SetResource(0, *sceneViewCbuffer);
            commands->Draw(visibleGroup.numVertices, visibleGroup.firstVertex);
        }
        commands->PopDebugGroup();
    }

    void TransformSceneViewForThumbnail(BrickType brickType, SceneView& outView)
    {
        const float dist = 10.0f;

        Gs::Matrix4f vMatrix;
        Gs::RotateY(vMatrix, Gs::Deg2Rad(45.0f));
        Gs::RotateX(vMatrix, Gs::Deg2Rad(45.0f));
        Gs::Translate(vMatrix, Gs::Vector3f{ 0, 0, -dist });

        outView.viewPos     = Gs::TransformVector(vMatrix, Gs::Vector4f{ 0, 0, 0, 1 });
        outView.gridScale   = KDTreeNode::gridScale;

        // Visually, we only want to rotate the brick, but they only support 90 degree angles.
        // So instead, we turn the camera around it but still want the light to look like it shines straight against the brick.
        outView.lightDir    = -Gs::Vector3f{ Gs::TransformVector(vMatrix, Gs::Vector4f{ 0, -0.25f, +1, 0 }) }.Normalized();

        vMatrix.MakeInverse();

        // Use orthogonal projection for brick thumbnails
        const float volumeSize = 7.0f;
        const Gs::Matrix4f thumbnailProjection = OrthogonalProjection(volumeSize, volumeSize, 0.1f, 50.0f);
        outView.vpMatrix = thumbnailProjection * vMatrix;
    }

    void RenderBrickThumnail(BrickType brickType, LLGL::RenderTarget& renderTarget)
    {
        PreviewBrick thumbnailBrick;
        SceneView thumbnailSceneView;

        // Move camera so that the thumbnail for the current brick type is clearly visible
        TransformSceneViewForThumbnail(brickType, thumbnailSceneView);
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
            const std::string rtLabel = std::string("RenderTarget.Thumbnail_") + brickDictionary.TypeToString(static_cast<BrickType>(brickType));
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
        instances[0].gridPos        = {};
        instances[0].gridRotation   = GridRotation_180;
        instances[0].color          = {};

        MarkInstanceDirty(0);

        // Render bricks as thumbnails into all offscreen render-targets
        commands->Begin();
        {
            FlushInstanceDirtyRange();

            commands->PushDebugGroup("RenderBrickThumbnails");
            {
                for (int brickType = 0; brickType < BrickType_Num; ++brickType)
                {
                    const std::string debugLabel = std::string("RenderThumbnail_") + brickDictionary.TypeToString(static_cast<BrickType>(brickType));
                    commands->PushDebugGroup(debugLabel.c_str());
                    {
                        RenderBrickThumnail(static_cast<BrickType>(brickType), *renderTargets[brickType]);
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
        Draw2DRect(0, 0, uiState.mouseDeadZoneLeft, scrH, uiMenuBgColor);
        Draw2DRect(uiState.mouseDeadZoneRight, 0, scrW - uiState.mouseDeadZoneRight, scrH, uiMenuBgColor);

        ResetMouseDeadZone(swapChain->GetResolution());

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
                uiState.mouseDeadZoneLeft += uiButtonAdvance;
            }

            const bool isHighlighted = (controller.pickedColorPaletteIndex == i);
            if (DrawUIColorPicker(uiPos[0], uiPos[1], i, isHighlighted))
            {
                // Set new color from palette to preview brick
                Instance& previewInstance = instances[0];
                previewInstance.color = colorPalette[i];
                controller.pickedColorPaletteIndex = i;
                MarkInstanceDirty(0);
            }

            // Move to next UI element position
            uiPos[1] += uiButtonAdvance;
        }

        // Draw brick thumbnails for the user to choose from on the right of the overlay
        uiButtonAdvance = uiBrickThumbnailSize + uiButtonFrameSize*2 + uiButtonMargin;
        uiPos[0] = scrW - (uiColorPickerOffset[0] + uiButtonAdvance);
        uiPos[1] = uiColorPickerOffset[1];

        for (int brickType = 0; brickType < BrickType_Num; ++brickType)
        {
            if (uiPos[1] > scrH - uiButtonAdvance)
            {
                // Move to next color picker column
                uiPos[0] -= uiButtonAdvance;
                uiPos[1] = uiColorPickerOffset[1];
                uiState.mouseDeadZoneRight -= uiButtonAdvance;
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
            camera.Move({ xAxis, yAxis, 0 });
        }

        const float moveSpeed = (input.KeyPressed(LLGL::Key::Shift) ? cameraMoveSpeed*3.0f : cameraMoveSpeed);
        if (input.KeyPressed(LLGL::Key::A) || input.KeyPressed(LLGL::Key::Left))
            camera.Move({ -moveSpeed, 0, 0 });
        if (input.KeyPressed(LLGL::Key::D) || input.KeyPressed(LLGL::Key::Right))
            camera.Move({ +moveSpeed, 0, 0 });
        if (input.KeyPressed(LLGL::Key::W) || input.KeyPressed(LLGL::Key::Up))
            camera.Move({ 0, 0, +moveSpeed });
        if (input.KeyPressed(LLGL::Key::S) || input.KeyPressed(LLGL::Key::Down))
            camera.Move({ 0, 0, -moveSpeed });
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
        if (Instance* oldBrickInstance = GetMutableBrickInstance(controller.pickedBrick))
        {
            oldBrickInstance->color = controller.pickedInstanceColor;
            controller.pickedBrick = {};
            return oldBrickInstance;
        }
        return nullptr;
    }

    bool PickScene(const Ray3f& ray, PickerContact& outContact)
    {
        AABBIntersection aabbIntersection;
        if (outContact.brick = grid.IntersectWithLeaf(ray, &aabbIntersection))
        {
            outContact.point = ray.Interpolate(aabbIntersection.interpolator);
            outContact.normal = aabbIntersection.normal;
            return true;
        }
        else
        {
            const Gs::Vector3i planeNormal{ 0, 1, 0 };
            if (IntersectRayWithPlane(ray, planeNormal.Cast<float>(), 0.0f, &aabbIntersection.interpolator))
            {
                // Place pending brick at picked location on the floor plane
                outContact.point = ray.Interpolate(aabbIntersection.interpolator);
                outContact.normal = planeNormal;
                return true;
            }
        }
        return false;
    }

    Brick SelectBrickFromScene()
    {
        // Pick scene objects with the mouse pointer
        const Ray3f pickerRay = ProjectScreenPosToViewRay(input.GetMousePosition());

        PickerContact contact;
        if (PickScene(pickerRay, contact))
        {
            // Highlight new picked brick
            if (Instance* newBrickInstance = GetMutableBrickInstance(contact.brick))
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
        const Gs::Vector3i brickExtent = brickDictionary.GetExtent(type);
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
        Instance& previewInstance = instances[0];

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

        PickerContact contact;
        if (PickScene(pickerRay, contact))
        {
            const BrickDimensions brickDim = GetBrickDimensions(newBrickType, newPreviewRotation);

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
            previewBrick.isBlocked = !BrickFitsInGrid(newPreviewGridPos, newBrickType, newPreviewRotation);

            // Update brick position and rotation
            previewBrick.type               = newBrickType;
            previewInstance.gridPos         = newPreviewGridPos;
            previewInstance.gridRotation    = newPreviewRotation;
            MarkInstanceDirty(0);
        }
    }

    void ProcessEditModeAddOrMove()
    {
        PickGridPosForPreviewBrick();

        const PreviewBrick& previewBrick = viewOptions.previewBrick;
        if (!previewBrick.isBlocked && input.KeyDown(LLGL::Key::LButton))
        {
            // Place new brick and update pick position for preview
            NewBrick(previewBrick.type, instances[0].gridPos, instances[0].gridRotation, instances[0].color, previewBrick.isTransparent);
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
                if (const Instance* selectedInstance = GetBrickInstance(selectedBrick))
                {
                    ResetPreviouslySelectedBrick();
                    Instance& previewInstance = instances[0];
                    {
                        viewOptions.previewBrick.type           = selectedBrick.GetType();
                        viewOptions.previewBrick.isTransparent  = (selectedBrick.GetGroup() == BrickGroup_Transparent);
                        previewInstance.color                   = selectedInstance->color;
                        previewInstance.gridRotation            = selectedInstance->gridRotation;

                        // Try to find picked color in color palette. If not found, it was edited externally and we keep the color picker unchanged.
                        int colorPaletteIndex = FindColorPaletteIndex(selectedInstance->color);
                        if (colorPaletteIndex >= 0 && colorPaletteIndex < sizeof(colorPalette)/sizeof(colorPalette[0]))
                            controller.pickedColorPaletteIndex = colorPaletteIndex;
                    }

                    // Change to edit mode that allows the user to move the selected brick around.
                    // Immediately update picking position of the preview brick before it is rendered with a stale position.
                    controller.editMode = EditMode::Move;
                    PickGridPosForPreviewBrick();
                }

                // Remove selected brick
                DestroyBrick(selectedBrick);
            }
        }
    }

    void ProcessUserInput()
    {
        const LLGL::Offset2D mousePos = input.GetMousePosition();
        const bool isMouseInDeadZone = (mousePos.x < uiState.mouseDeadZoneLeft || mousePos.x > uiState.mouseDeadZoneRight);

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
        // Update common scene view states
        sceneView.viewPos           = camera.wMatrix * Gs::Vector4f{ 0, 0, 0, 1 };
        sceneView.vpMatrix          = projection * camera.vMatrix;

        sceneView.gridScale         = KDTreeNode::gridScale;

        sceneView.lightDir          = Gs::Vector3f{ -0.2f, +0.4f, -1.0f }.Normalized();

        // Update preview shader states
      //sceneView.worldOffsetY      = previewBlockOffsetY;
        sceneView.isDithering       = (viewOptions.previewBrick.isBlocked ? 1 : 0);

        // Update outline shader states
        const LLGL::Extent2D& viewportSize = swapChain->GetResolution();
        sceneView.viewportSize[0]   = static_cast<float>(viewportSize.width);
        sceneView.viewportSize[1]   = static_cast<float>(viewportSize.height);

        // Update planar projection for UI elements
        const LLGL::Extent2D resolution = swapChain->GetResolution();
        uiView.pMatrix = Gs::ProjectionMatrix4f::Planar(static_cast<float>(resolution.width), static_cast<float>(resolution.height));
    }

    void ResetMouseDeadZone(const LLGL::Extent2D& resolution)
    {
        // Update mouse dead zone
        uiState.mouseDeadZoneLeft   = uiColorPickerOffset[0]*2 + uiColorPickerSize + uiButtonFrameSize*2;
        uiState.mouseDeadZoneRight  = static_cast<int>(resolution.width) - (uiColorPickerOffset[0]*2 + uiBrickThumbnailSize + uiButtonFrameSize*2);
    }

    //////////////////// MAIN CALLBACKS ////////////////////

    void OnDrawFrame(float /*dt*/) override
    {
        #if DEBUG_KDTREE
        KDTreeNode::numAABBTests = 0;
        #endif

        camera.UpdateTransform();

        ProcessUserInput();
        UpdateSceneAndUIView();

        commands->Begin();
        {
            commands->UpdateBuffer(*sceneViewCbuffer, 0, &sceneView, sizeof(sceneView));
            commands->UpdateBuffer(*uiViewCbuffer, 0, &uiView, sizeof(uiView));

            FlushInstanceDirtyRange();

            commands->BeginRenderPass(*swapChain);
            {
                // Clear all buffers. Stencil might be used for silhouette rendering.
                commands->Clear(LLGL::ClearFlags::All, backgroundColor);
                commands->SetViewport(swapChain->GetResolution());

                commands->PushDebugGroup("RenderBrickInstances");
                {
                    RenderBrickInstances();
                }
                commands->PopDebugGroup();

                if (controller.cameraMode == CameraMode::None && (controller.editMode == EditMode::Add || controller.editMode == EditMode::Move))
                {
                    commands->PushDebugGroup("RenderPreviewBrick");
                    {
                        RenderPreviewBrick(viewOptions.previewBrick);
                    }
                    commands->PopDebugGroup();
                }

                if (controller.editMode == EditMode::Remove)
                {
                    commands->PushDebugGroup("RenderSilhouette");
                    {
                        RenderSilhouette(controller.pickedBrick);
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
        // Ensure previoulsy selected brick retains its original color
        ResetPreviouslySelectedBrick();
        SaveScene(sceneFilename);
    }

};

const Gs::Vector3f  KDTreeNode::gridScale    = { 1.0f, 0.5f, 1.0f };
int                 KDTreeNode::numAABBTests = 0;
AABB3i              KDTreeNode::rootAABB;


LLGL_IMPLEMENT_EXAMPLE(Example_BrickPlay);



