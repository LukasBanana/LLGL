/*
 * InstancingAdvanced.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>
#include <algorithm>
#include <LLGL/Utils/ForRange.h>


// Enable log output to debug kd-Tree construction
#define DEBUG_KDTREE 1

// Subdivide kd-Tree uniformly. This is only used for performance testing.
#define UNIFORM_TREE_SUBDIVIDE 0


class Example_InstancingAdvanced : public ExampleBase
{

    #define foreach_dimension(X, Y, Z, VEC) \
        for_range(Z, (VEC).z) for_range(Y, (VEC).y) for_range(X, (VEC).x)

    static constexpr int        boundingVolume[3]   = { 60, 45, 60 };
    static constexpr float      cameraTurnSpeed     = 0.25f;
    static constexpr float      cameraMoveSpeed     = 0.25f;

    enum BrickPSO
    {
        BrickPSO_Default = 0,
        BrickPSO_Wireframe,
        BrickPSO_Preview, // With alpha-blending

        BrickPSO_Num,
    };

    LLGL::Shader*               vsBrick             = nullptr;
    LLGL::Shader*               fsBrick             = nullptr;
    LLGL::Shader*               fsBrickPreview      = nullptr;

    LLGL::PipelineLayout*       psoLayout           = nullptr;
    LLGL::PipelineState*        pso[BrickPSO_Num]   = {};

    #if DEBUG_KDTREE
    LLGL::PipelineState*        psoDebug            = nullptr;
    #endif

    LLGL::Buffer*               sceneViewCbuffer    = nullptr;
    LLGL::Buffer*               vertexBuffer        = nullptr;
    LLGL::Buffer*               instanceBuffer      = nullptr;
    std::uint64_t               instanceBufferSize  = 0;

    bool                        hasOffsetInstancing = false;

    const LLGL::ColorRGBAub     colorPalette[5]      =
    {
        LLGL::ColorRGBAub{ 0xFF, 0xF0, 0x3A }, // Bright light yellow
        LLGL::ColorRGBAub{ 0x73, 0xDC, 0xA1 }, // Medium green
        LLGL::ColorRGBAub{ 0xFF, 0x69, 0x8F }, // Coral
        LLGL::ColorRGBAub{ 0xCD, 0x62, 0x98 }, // Light purple
        LLGL::ColorRGBAub{ 0x92, 0x39, 0x78 }, // Magenta
    };

    struct alignas(16) SceneView
    {
        Gs::Matrix4f    vpMatrix;                                   // View-projection matrix
        Gs::Vector3f    gridScale       = Gs::Vector3f{ 1.0f };
        float           pad0            = 0.0f;
        Gs::Vector4f    viewPos;                                    // Camera view position (in world space)
        Gs::Vector3f    lightDir        = { 0.0f, 0.0f, -1.0f };    // Light vector pointing from a pixel towards the light source
        float           specularPower   = 90.0f;                    // Specular exponent for blinn-phone shading model
    }
    sceneView;

    enum GridRotation : std::int32_t
    {
        GridRotation_0 = 0,
        GridRotation_90,

        GridRotation_Num,
    };

    enum BrickType : std::uint32_t
    {
        //        WxDxH
        BrickType_2x2x3 = 0,
        BrickType_4x2x3,
        BrickType_4x2x1,

        BrickType_WallsOnly_Offset,

        BrickType_2x2x3_WallsOnly = BrickType_WallsOnly_Offset,
        BrickType_4x2x3_WallsOnly,
        BrickType_4x2x1_WallsOnly,

        BrickType_Num,
    };

    struct ViewOptions
    {
        bool        showWireframe           = false;
        BrickType   previewBrickType        = BrickType_2x2x3; // Previews brick with instance index 0.
        bool        isPreviewBrickBlocked   = false;
    }
    viewOptions;

    struct Vertex
    {
        float position[3];
        float normal[3];
    };

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
    InstanceGroup           instanceGroups[BrickType_Num];

    struct Brick
    {
        Brick() :
            groupId       { BrickType_Num },
            groupInstance { 0             }
        {
        }

        Brick(BrickType type, std::uint32_t instance) :
            groupId       { type     },
            groupInstance { instance }
        {
        }

        BrickType       groupId         : 3;

        // Index into the group's instance. Use `group.firstInstance + groupInstance` to get index into global `instances` container.
        std::uint32_t   groupInstance   : 29;

        inline operator bool() const
        {
            return (groupId != BrickType_Num);
        }

        inline bool operator == (const Brick& rhs) const
        {
            return (groupId == rhs.groupId && groupInstance == rhs.groupInstance);
        }
    };

    struct PickerContact
    {
        Gs::Vector3f    point;
        Gs::Vector3i    normal;
        Brick           brick;
    };

    struct BrickDimensions
    {
        Gs::Vector3i offset;
        Gs::Vector3i extent;
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
        Remove,
    };

    // State tracking for user controls such as picking scene objects with the mouse pointer.
    struct Controller
    {
        CameraMode          cameraMode              = CameraMode::None;
        EditMode            editMode                = EditMode::None;

        LLGL::Offset2D      lastCursorPos;

        Brick               pickedBrick;
        LLGL::ColorRGBAub   pickedInstanceColor;

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

    // Tree hierarchy for the grid of brick entires. A 4x2x3 brick for instances takes up 24 entries.
    class KDTreeNode
    {

        enum Axis : std::uint32_t
        {
            Axis_X = 0,
            Axis_Y,
            Axis_Z,
        };

        std::unique_ptr<KDTreeNode> child[2];
        Axis                        axis        : 2;
        std::uint32_t               distance    : 30; // Local distance from node's logical bbMin
        Brick                       brick;

        AABB3i GetChild0AABB(Axis parentAxis, AABB3i aabbParent) const
        {
            aabbParent.bbMax[parentAxis] = aabbParent.bbMin[parentAxis] + static_cast<std::int32_t>(distance);
            return aabbParent;
        }

        AABB3i GetChild1AABB(Axis parentAxis, AABB3i aabbParent) const
        {
            aabbParent.bbMin[parentAxis] += static_cast<std::int32_t>(distance);
            return aabbParent;
        }

        void SubdivideAndSetLeaf(const Gs::Vector3i& gridPos, const AABB3i& aabb, Brick brick)
        {
            if (aabb.bbMax == aabb.bbMin + Gs::Vector3i{ 1 })
            {
                // If we found a leaf node with AABB of size 1x1x1, put the brick into the leaf
                this->brick = brick;
            }
            else
            {
                // Make new child nodes and subdivide
                child[0] = std::unique_ptr<KDTreeNode>(new KDTreeNode());
                child[1] = std::unique_ptr<KDTreeNode>(new KDTreeNode());

                const Gs::Vector3i aabbExtent = aabb.bbMax - aabb.bbMin;
                LLGL_VERIFY(aabbExtent.x > 0 && aabbExtent.y > 0 && aabbExtent.z > 0);
                if (aabbExtent.x >= aabbExtent.y && aabbExtent.x >= aabbExtent.z)
                    axis = Axis_X;
                else if (aabbExtent.y >= aabbExtent.x && aabbExtent.y >= aabbExtent.z)
                    axis = Axis_Y;
                else
                    axis = Axis_Z;

                // Set distance in the center of current AABB extent
                #if UNIFORM_TREE_SUBDIVIDE
                distance = static_cast<std::uint32_t>(aabbExtent[axis]/2);
                #else
                distance = std::max(1, gridPos[axis] - aabb.bbMin[axis]);
                #endif
                if (gridPos[axis] < aabb.bbMin[axis] + static_cast<std::int32_t>(distance))
                    child[0]->SubdivideAndSetLeaf(gridPos, GetChild0AABB(axis, aabb), brick);
                else
                    child[1]->SubdivideAndSetLeaf(gridPos, GetChild1AABB(axis, aabb), brick);
            }
        }

        void FindAndInsertLeaf(const Gs::Vector3i& gridPos, const AABB3i& aabb, Brick brick)
        {
            if (HasChildren())
            {
                // Shrink root AABB until we reach a leaf node
                if (gridPos[axis] < aabb.bbMin[axis] + static_cast<std::int32_t>(distance))
                    child[0]->FindAndInsertLeaf(gridPos, GetChild0AABB(axis, aabb), brick);
                else
                    child[1]->FindAndInsertLeaf(gridPos, GetChild1AABB(axis, aabb), brick);
            }
            else
            {
                // Otherwise, keep adding sub nodes until we reach the destination grid position
                SubdivideAndSetLeaf(gridPos, aabb, brick);
            }
        }

        void TryDropChildren()
        {
            if (!child[0]->HasChildren() && !child[0]->brick &&
                !child[1]->HasChildren() && !child[1]->brick)
            {
                child[0].reset();
                child[1].reset();
            }
        }

        bool FindAndRemoveLeaf(const Gs::Vector3i& gridPos, const AABB3i& aabb)
        {
            if (HasChildren())
            {
                if (gridPos[axis] < aabb.bbMin[axis] + static_cast<std::int32_t>(distance))
                {
                    if (child[0]->FindAndRemoveLeaf(gridPos, GetChild0AABB(axis, aabb)))
                    {
                        TryDropChildren();
                        return true;
                    }
                }
                else
                {
                    if (child[1]->FindAndRemoveLeaf(gridPos, GetChild1AABB(axis, aabb)))
                    {
                        TryDropChildren();
                        return true;
                    }
                }
            }
            else if (aabb.bbMax == aabb.bbMin + Gs::Vector3i{ 1 })
            {
                this->brick = {};
                return true;
            }
            return false;
        }

        Brick FindLeaf(const Gs::Vector3i& gridPos, const AABB3i& aabb) const
        {
            if (HasChildren())
            {
                // Shrink root AABB until we reach a leaf node
                if (gridPos[axis] < aabb.bbMin[axis] + static_cast<std::int32_t>(distance))
                    return child[0]->FindLeaf(gridPos, GetChild0AABB(axis, aabb));
                else
                    return child[1]->FindLeaf(gridPos, GetChild1AABB(axis, aabb));
            }
            else if (aabb.bbMax == aabb.bbMin + Gs::Vector3i{ 1 })
            {
                // We found the leaf node
                return this->brick;
            }
            else
            {
                // No leaf node found, return invalid brick data
                return Brick{};
            }
        }

        bool IntersectRayWithScaledAABB(const Ray3f& ray, const AABB3i& aabb, AABBIntersection* outIntersection = nullptr) const
        {
            #if DEBUG_KDTREE
            KDTreeNode::numAABBTests++;
            #endif

            const AABB3f aabbF32
            {
                aabb.bbMin.Cast<float>() * KDTreeNode::gridScale,
                aabb.bbMax.Cast<float>() * KDTreeNode::gridScale
            };
            return IntersectRayWithAABB(ray, aabbF32, outIntersection);
        }

        Brick FindIntersectionWithLeaf(const Ray3f& ray, const AABB3i& aabb, AABBIntersection* outIntersection) const
        {
            if (HasChildren())
            {
                // Check intersection against both child nodes' AABBs and continue down the path of the closer intersection
                if (IntersectRayWithScaledAABB(ray, aabb))
                {
                    AABBIntersection childIntersect[2];
                    Brick childBrick[2];

                    auto ReturnIntersection = [&childIntersect, &childBrick, outIntersection](int childIndex) -> Brick
                    {
                        if (outIntersection != nullptr)
                            *outIntersection = childIntersect[childIndex];
                        return childBrick[childIndex];
                    };

                    childBrick[0] = child[0]->FindIntersectionWithLeaf(ray, GetChild0AABB(axis, aabb), &(childIntersect[0]));
                    childBrick[1] = child[1]->FindIntersectionWithLeaf(ray, GetChild1AABB(axis, aabb), &(childIntersect[1]));

                    if (childBrick[0] && childBrick[1])
                    {
                        // Continue intersection down the path of the node with the closest intersection
                        if (childIntersect[0].interpolator < childIntersect[1].interpolator)
                            return ReturnIntersection(0);
                        else
                            return ReturnIntersection(1);
                    }
                    else if (childBrick[0])
                        return ReturnIntersection(0);
                    else if (childBrick[1])
                        return ReturnIntersection(1);
                }
            }
            else if (aabb.bbMax == aabb.bbMin + Gs::Vector3i{ 1 } && this->brick)
            {
                // Make final intersection test against AABB of current node
                if (IntersectRayWithScaledAABB(ray, aabb, outIntersection))
                    return this->brick;
            }
            return Brick{};
        }

        unsigned NumLeafsFromRoot(const AABB3i& aabb) const
        {
            if (HasChildren())
                return child[0]->NumLeafsFromRoot(GetChild0AABB(axis, aabb)) + child[1]->NumLeafsFromRoot(GetChild1AABB(axis, aabb));
            else if (aabb.bbMax == aabb.bbMin + Gs::Vector3i{ 1 })
                return 1;
            else
                return 0;
        }

    public:

        static const AABB3i         rootAABB;
        static const Gs::Vector3f   gridScale;

        static int                  numAABBTests;

        KDTreeNode() :
            axis     { Axis_X },
            distance { 0      }
        {
        }

        bool HasChildren() const
        {
            return (child[0].get() != nullptr);
        }

        void InsertLeaf(Gs::Vector3i gridPos, Brick brick)
        {
            FindAndInsertLeaf(gridPos, KDTreeNode::rootAABB, brick);
        }

        void RemoveLeaf(Gs::Vector3i gridPos)
        {
            FindAndRemoveLeaf(gridPos, KDTreeNode::rootAABB);
        }

        Brick GetLeaf(const Gs::Vector3i& gridPos) const
        {
            return FindLeaf(gridPos, KDTreeNode::rootAABB);
        }

        Brick IntersectWithLeaf(const Ray3f& ray, AABBIntersection* outIntersection = nullptr) const
        {
            return FindIntersectionWithLeaf(ray, KDTreeNode::rootAABB, outIntersection);
        }

        unsigned NumNodes() const
        {
            if (HasChildren())
                return 1 + child[0]->NumNodes() + child[1]->NumNodes();
            else
                return 1;
        }

        unsigned NumLeafs() const
        {
            return NumLeafsFromRoot(KDTreeNode::rootAABB);
        }

    };

    KDTreeNode grid;

public:

    Example_InstancingAdvanced() :
        ExampleBase { "LLGL Example: Instancing Advanced" }
    {
        hasOffsetInstancing = renderer->GetRenderingCaps().features.hasOffsetInstancing;
        ResetInstanceDirtyRange();

        // Create all graphics objects
        CreateBuffers();
        CreatePipelines();
        CreateInitialScene();

        // Print info
        LLGL::Log::Printf(
            "Keyboard controls:\n"
            "  1 .............. Default mode\n"
            "  2 .............. Preview brick mode\n"
            "  3/Delete ....... Select brick mode\n"
            "  Right mouse .... Turn camera\n"
            "  Middle mouse ... Move camera left/right & up/down\n"
            "  W/A/S/D ........ Move camera left/right & forwards/backwards\n"
            "  PageUp/-Down ... Change brick type\n"
            "  Space .......... Rotate brick\n"
        );
    }

private:

    void MarkInstanceDirty(std::size_t first, std::size_t count = 1)
    {
        instanceDirtyRange[0] = std::min(instanceDirtyRange[0], first);
        instanceDirtyRange[1] = std::max(instanceDirtyRange[1], first + count);
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
    Instance* AddInstancesToGroup(BrickType groupId, std::uint32_t count = 1)
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
            for (int nextGroupId = groupId + 1; nextGroupId < BrickType_Num; ++nextGroupId)
                instanceGroups[nextGroupId].firstInstance += instanceGrowth;
        }

        // Return pointer to first new instance
        Instance* firstInstancePtr = &(instances[group.firstInstance + group.numInstances]);
        group.numInstances += count;
        MarkInstanceDirty(group.firstInstance, count);

        return firstInstancePtr;
    }

    void RemoveInstanceFromGroup(BrickType groupId, std::uint32_t groupInstance)
    {
        InstanceGroup& group = instanceGroups[groupId];
        if (groupInstance < group.numInstances)
        {
            Instance& retiredInstance = instances[group.firstInstance + groupInstance];

            group.numInstances--;
            if (groupInstance < group.numInstances)
            {
                // Remove instance from group by copying last instance in that group into the position of the old instance
                const Instance& lastInstance = instances[group.firstInstance + group.numInstances];
                RemoveBrickFromGrid(groupId, retiredInstance);
                InsertBrickIntoGrid(lastInstance.gridPos, Brick{ groupId, groupInstance });
                retiredInstance = lastInstance;
            }
            else
            {
                // If destroyed brick is the last instance in that group, just reset the entry in the grid
                RemoveBrickFromGrid(groupId, retiredInstance);
            }

            // Mark all remaining instances of that group as dirty
            MarkInstanceDirty(group.firstInstance + groupInstance);
        }
    }

    void FlushInstanceDirtyRange()
    {
        // Update instances within the range that is marked as dirty
        if (instanceDirtyRange[0] < instanceDirtyRange[1])
        {
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

        switch (type)
        {
            case BrickType_2x2x3:
            case BrickType_2x2x3_WallsOnly:
                brickDim.extent = { 2, 3, 2 };
                break;
            case BrickType_4x2x3:
            case BrickType_4x2x3_WallsOnly:
                brickDim.extent = { 4, 3, 2 };
                break;
            case BrickType_4x2x1:
            case BrickType_4x2x1_WallsOnly:
                brickDim.extent = { 4, 1, 2 };
                break;
            default:
                break;
        }

        brickDim.offset.x = -brickDim.extent.x/2;
        brickDim.offset.z = 0;
        brickDim.offset.z = -brickDim.extent.z/2;

        if (rotation == GridRotation_90)
        {
            std::swap(brickDim.extent.x, brickDim.extent.z);
            std::swap(brickDim.offset.x, brickDim.offset.z);
        }

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

    void InsertBrickIntoGrid(const Gs::Vector3i& gridPos, Brick brick)
    {
        // Get brick dimension and apply rotation
        const Instance* instance = GetBrickInstance(brick);
        const BrickDimensions brickDim = GetBrickDimensions(brick.groupId, instance != nullptr ? instance->gridRotation : GridRotation_0);

        // Insert brick entries into grid
        const Gs::Vector3i alignedGridPos = gridPos + brickDim.offset;
        foreach_dimension(x, y, z, brickDim.extent)
            grid.InsertLeaf(alignedGridPos + Gs::Vector3i{ x, y, z }, brick);

        #if DEBUG_KDTREE
        LLGL::Log::Printf("kd-Tree debug info: Nodes=%u, Leafs=%u\n", grid.NumNodes(), grid.NumLeafs());
        #endif
    }

    void RemoveBrickFromGrid(BrickType groupId, const Instance& instance)
    {
        // Get brick dimension and apply rotation
        const BrickDimensions brickDim = GetBrickDimensions(groupId, instance.gridRotation);

        // Remove brick entries from grid
        const Gs::Vector3i alignedGridPos = instance.gridPos + brickDim.offset;
        foreach_dimension(x, y, z, brickDim.extent)
            grid.RemoveLeaf(alignedGridPos + Gs::Vector3i{ x, y, z });

        #if DEBUG_KDTREE
        LLGL::Log::Printf("kd-Tree debug info: Nodes=%u, Leafs=%u\n", grid.NumNodes(), grid.NumLeafs());
        #endif
    }

    Brick NewBrick(BrickType type, const Gs::Vector3i& gridPos, GridRotation gridRotation = GridRotation_0, LLGL::ColorRGBAub color = {})
    {
        LLGL_VERIFY(type < BrickType_WallsOnly_Offset);

        if (!BrickFitsInGrid(gridPos, type, gridRotation))
            return Brick{};

        Brick newBrick;
        {
            newBrick.groupId        = type;
            newBrick.groupInstance  = instanceGroups[type].numInstances;
        }
        Instance* newInstance = AddInstancesToGroup(type);
        {
            newInstance->gridPos        = gridPos;
            newInstance->gridRotation   = gridRotation;
            newInstance->color          = color;
        }

        InsertBrickIntoGrid(gridPos, newBrick);

        return newBrick;
    }

    void DestroyBrick(Brick brick)
    {
        if (controller.pickedBrick == brick)
            ResetPreviouslySelectedBrick();
        RemoveInstanceFromGroup(brick.groupId, brick.groupInstance);
    }

    void LoadBrickModel(const char* basename, std::vector<Vertex>& vertices, InstanceGroup& outInstanceGroup)
    {
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

    void CreateBuffers()
    {
        const float projZAxis = GetProjectionZAxis();

        std::vector<Vertex> vertices;
        LoadBrickModel("Brick2x2x3"          , vertices, instanceGroups[BrickType_2x2x3          ]);
        LoadBrickModel("Brick4x2x3"          , vertices, instanceGroups[BrickType_4x2x3          ]);
        LoadBrickModel("Brick4x2x1"          , vertices, instanceGroups[BrickType_4x2x1          ]);
        LoadBrickModel("Brick2x2x3_WallsOnly", vertices, instanceGroups[BrickType_2x2x3_WallsOnly]);
        LoadBrickModel("Brick4x2x3_WallsOnly", vertices, instanceGroups[BrickType_4x2x3_WallsOnly]);
        LoadBrickModel("Brick4x2x1_WallsOnly", vertices, instanceGroups[BrickType_4x2x1_WallsOnly]);

        // Create buffer for per-vertex data
        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.debugName = "VertexBuffer";
            vertexBufferDesc.size      = sizeof(Vertex) * vertices.size();
            vertexBufferDesc.stride    = sizeof(Vertex);
            vertexBufferDesc.bindFlags = LLGL::BindFlags::VertexBuffer;
        }
        vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices.data());

        // Create scene view constant buffer
        sceneViewCbuffer = CreateConstantBuffer(sceneView);

        // Allocate preview instance
        instances.resize(1);
        instances[0].color = LLGL::ColorRGBAub{ 255, 255, 255, 128 };
    }

    void CreatePipelines()
    {
        // Create shaders
        vsBrick         = LoadVertexShader  ("InstancingAdvanced", "VSBrick");
        fsBrick         = LoadFragmentShader("InstancingAdvanced", "PSBrick");
        fsBrickPreview  = LoadFragmentShader("InstancingAdvanced", "PSBrickPreview");

        // Create pipeline layout
        psoLayout = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(SceneView@2):vert:frag,"
            )
        );

        // Specify vertex formats
        LLGL::VertexAttribute vertexAttribs[] =
        {
            // Per-vertex attributes
            LLGL::VertexAttribute{ "position", LLGL::Format::RGB32Float, /*location:*/ 0, /*offset:*/ offsetof(Vertex, position), /*stride:*/ sizeof(Vertex), /*slot:*/ 0 },
            LLGL::VertexAttribute{ "normal",   LLGL::Format::RGB32Float, /*location:*/ 1, /*offset:*/ offsetof(Vertex, normal  ), /*stride:*/ sizeof(Vertex), /*slot:*/ 0 },

            // Per-instance attributes
            LLGL::VertexAttribute{ "gridPos",  LLGL::Format::RGBA32SInt, /*location:*/ 2, /*offset:*/ offsetof(Instance, gridPos), /*stride:*/ sizeof(Instance), /*slot:*/ 1, /*instanceDivisor:*/ 1 },
            LLGL::VertexAttribute{ "color",    LLGL::Format::RGBA8UNorm, /*location:*/ 3, /*offset:*/ offsetof(Instance, color  ), /*stride:*/ sizeof(Instance), /*slot:*/ 1, /*instanceDivisor:*/ 1 },
        };

        // Create common graphics pipeline for scene rendering
        LLGL::GraphicsPipelineDescriptor psoDesc;
        {
            psoDesc.inputVertexAttribs              = vertexAttribs;
            psoDesc.vertexShader                    = vsBrick;
            psoDesc.fragmentShader                  = fsBrick;
            psoDesc.pipelineLayout                  = psoLayout;
            psoDesc.primitiveTopology               = LLGL::PrimitiveTopology::TriangleList;
            psoDesc.depth.testEnabled               = true;
            psoDesc.depth.writeEnabled              = true;
            psoDesc.rasterizer.multiSampleEnabled   = (GetSampleCount() > 1);
            psoDesc.rasterizer.cullMode             = LLGL::CullMode::Back;
        }
        pso[BrickPSO_Default] = renderer->CreatePipelineState(psoDesc);
        ReportPSOErrors(pso[BrickPSO_Default]);
        {
            psoDesc.rasterizer.polygonMode          = LLGL::PolygonMode::Wireframe;
        }
        pso[BrickPSO_Wireframe] = renderer->CreatePipelineState(psoDesc);
        ReportPSOErrors(pso[BrickPSO_Wireframe]);
        {
            psoDesc.fragmentShader                      = fsBrickPreview;
            psoDesc.rasterizer.polygonMode              = LLGL::PolygonMode::Fill;
            psoDesc.rasterizer.depthBias.constantFactor = -100.0f;
        }
        pso[BrickPSO_Preview] = renderer->CreatePipelineState(psoDesc);
        ReportPSOErrors(pso[BrickPSO_Preview]);

        #if DEBUG_KDTREE
        LLGL::GraphicsPipelineDescriptor psoDebugDesc;
        {
            psoDebugDesc.debugName          = "DebugPSO";
            psoDebugDesc.pipelineLayout     = renderer->CreatePipelineLayout(LLGL::Parse("cbuffer(SceneView@2):vert,float4(vertexA,vertexB)"));
            psoDebugDesc.vertexShader       = LoadVertexShader  ("InstancingAdvanced", "VSDebug");
            psoDebugDesc.fragmentShader     = LoadFragmentShader("InstancingAdvanced", "PSDebug");
            psoDebugDesc.primitiveTopology  = LLGL::PrimitiveTopology::LineList;
            psoDebugDesc.depth.testEnabled  = true;
        }
        psoDebug = renderer->CreatePipelineState(psoDebugDesc);
        ReportPSOErrors(psoDebug);
        #endif
    }

    void CreateInitialScene()
    {
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

    void RenderBrickInstances()
    {
        // Exit early if no instances have been allocated yet
        if (instanceBuffer == nullptr)
            return;

        commands->SetPipelineState(*pso[viewOptions.showWireframe ? BrickPSO_Wireframe : BrickPSO_Default]);

        commands->SetResource(0, *sceneViewCbuffer);

        // Set vertex buffers (vertices and instances)
        LLGL::VertexBufferView vertexBufferViews[] =
        {
            LLGL::VertexBufferView{ vertexBuffer },
            LLGL::VertexBufferView{ instanceBuffer },
        };

        // If offset instancing is not supported, set vertex buffers with an offset for each instance group
        if (hasOffsetInstancing)
            commands->SetVertexBuffers(2, vertexBufferViews);

        // Draw instances of all groups
        for (int groupId = 0; groupId < BrickType_Num; ++groupId)
        {
            const InstanceGroup& group = instanceGroups[groupId];
            if (group.numInstances > 0)
            {
                const std::string debugGroup = "InstanceGroup" + std::to_string(groupId);
                commands->PushDebugGroup(debugGroup.c_str());

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

                commands->PopDebugGroup();
            }
        }
    }

    void RenderPreviewBrick()
    {
        commands->SetPipelineState(*pso[viewOptions.isPreviewBrickBlocked ? BrickPSO_Preview : BrickPSO_Default]);

        commands->SetResource(0, *sceneViewCbuffer);

        // Set vertex buffers (vertices and instances).
        // We don't care about offset instancing here, because the preview instance is always the first one (index 0).
        LLGL::VertexBufferView vertexBufferViews[] =
        {
            LLGL::VertexBufferView{ vertexBuffer },
            LLGL::VertexBufferView{ instanceBuffer },
        };
        commands->SetVertexBuffers(2, vertexBufferViews);

        const InstanceGroup& group = instanceGroups[viewOptions.previewBrickType];
        commands->Draw(group.numVertices, group.firstVertex);
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

    Gs::Vector3i WorldPosToGridPos(const Gs::Vector3f& worldPos)
    {
        return Gs::Vector3i
        {
            static_cast<std::int32_t>(std::floor(worldPos.x / KDTreeNode::gridScale.x + 0.5f)),
            static_cast<std::int32_t>(std::floor(worldPos.y / KDTreeNode::gridScale.y + 0.01f)),
            static_cast<std::int32_t>(std::floor(worldPos.z / KDTreeNode::gridScale.z + 0.5f))
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
                controller.pickedInstanceColor = newBrickInstance->color;
                controller.pickedBrick = contact.brick;

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

    void PickGridPosForPreviewBrick(Instance& previewInstance)
    {
        // Allow user to turn and change type of preview brick while picking the scene
        GridRotation newPreviewRotation = previewInstance.gridRotation;
        if (input.KeyDown(LLGL::Key::Space))
            newPreviewRotation = static_cast<GridRotation>((newPreviewRotation + 1) % GridRotation_Num);

        BrickType newBrickType = viewOptions.previewBrickType;
        if (input.KeyDown(LLGL::Key::PageUp))
            newBrickType = static_cast<BrickType>((newBrickType + 1) % BrickType_Num);

        // Pick scene objects with the mouse pointer
        const Ray3f pickerRay = ProjectScreenPosToViewRay(input.GetMousePosition());
        Gs::Vector3i newPreviewGridPos = previewInstance.gridPos;

        PickerContact contact;
        if (PickScene(pickerRay, contact))
        {
            // Find a potential grid position for the preview brick
            newPreviewGridPos = WorldPosToGridPos(contact.point);

            const BrickDimensions brickDim = GetBrickDimensions(newBrickType, newPreviewRotation);
            if (contact.normal.x < 0)
                newPreviewGridPos.x -= brickDim.offset.x + brickDim.extent.x;
            else if (contact.normal.x > 0)
                newPreviewGridPos.x -= brickDim.offset.x;
            else if (contact.normal.y < 0)
                newPreviewGridPos.y -= brickDim.offset.y + brickDim.extent.y;
            else if (contact.normal.y > 0)
                newPreviewGridPos.y -= brickDim.offset.y;
            else if (contact.normal.z < 0)
                newPreviewGridPos.z -= brickDim.offset.z + brickDim.extent.z;
            else if (contact.normal.z > 0)
                newPreviewGridPos.z -= brickDim.offset.z;
        }

        // If grid position changed, re-evaluate whether that position is blocked for the preview brick
        if (previewInstance.gridPos      != newPreviewGridPos ||
            previewInstance.gridRotation != newPreviewRotation ||
            viewOptions.previewBrickType != newBrickType)
        {
            viewOptions.isPreviewBrickBlocked = !BrickFitsInGrid(newPreviewGridPos, newBrickType, newPreviewRotation);

            // Update brick position and rotation
            viewOptions.previewBrickType    = newBrickType;
            previewInstance.gridPos         = newPreviewGridPos;
            previewInstance.gridRotation    = newPreviewRotation;
            MarkInstanceDirty(0);
        }
    }

    void ProcessUserInput()
    {
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

        if (controller.editMode == EditMode::Add)
        {
            PickGridPosForPreviewBrick(instances[0]);

            if (!viewOptions.isPreviewBrickBlocked && input.KeyDown(LLGL::Key::LButton))
            {
                // Place new brick and update pick position for preview
                NewBrick(viewOptions.previewBrickType, instances[0].gridPos, instances[0].gridRotation, instances[0].color);
                PickGridPosForPreviewBrick(instances[0]);
            }
        }

        if (controller.editMode == EditMode::Remove)
        {
            if (Brick selectedBrick = SelectBrickFromScene())
            {
                if (input.KeyDown(LLGL::Key::LButton))
                {
                    // Remove selected brick
                    DestroyBrick(selectedBrick);
                }
            }
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

    void UpdateSceneView()
    {
        sceneView.viewPos   = camera.wMatrix * Gs::Vector4f{ 0, 0, 0, 1 };
        sceneView.vpMatrix  = projection * camera.vMatrix;

        sceneView.gridScale = KDTreeNode::gridScale;

        sceneView.lightDir  = Gs::Vector3f{ -0.2f, +0.4f, -1.0f }.Normalized();
    }

    void OnDrawFrame(float /*dt*/) override
    {
        #if DEBUG_KDTREE
        KDTreeNode::numAABBTests = 0;
        #endif

        camera.UpdateTransform();

        ProcessUserInput();
        UpdateSceneView();

        commands->Begin();
        {
            commands->UpdateBuffer(*sceneViewCbuffer, 0, &sceneView, sizeof(sceneView));

            FlushInstanceDirtyRange();

            commands->BeginRenderPass(*swapChain);
            {
                commands->Clear(LLGL::ClearFlags::ColorDepth, backgroundColor);
                commands->SetViewport(swapChain->GetResolution());

                commands->PushDebugGroup("RenderBrickInstances");
                {
                    RenderBrickInstances();
                }
                commands->PopDebugGroup();

                if (controller.editMode == EditMode::Add)
                {
                    commands->PushDebugGroup("RenderPreviewBrick");
                    {
                        RenderPreviewBrick();
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

};

const AABB3i Example_InstancingAdvanced::KDTreeNode::rootAABB
{
    Gs::Vector3i{ -boundingVolume[0]/2,                  0, -boundingVolume[2]/2 },
    Gs::Vector3i{ +boundingVolume[0]/2, +boundingVolume[1], +boundingVolume[2]/2 }
};

const Gs::Vector3f Example_InstancingAdvanced::KDTreeNode::gridScale = { 1.0f, 0.5f, 1.0f };

int Example_InstancingAdvanced::KDTreeNode::numAABBTests = 0;


LLGL_IMPLEMENT_EXAMPLE(Example_InstancingAdvanced);



