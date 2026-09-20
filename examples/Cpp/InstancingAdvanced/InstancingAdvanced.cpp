/*
 * InstancingAdvanced.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>
#include <algorithm>
#include <LLGL/Utils/ForRange.h>
#include <set>

// File and string streams for simple scene file writer/reader
#include <fstream>
#include <sstream>


// Enable log output to debug kd-Tree construction
#define DEBUG_KDTREE 0

// Subdivide kd-Tree uniformly. This is only used for performance testing.
#define UNIFORM_TREE_SUBDIVIDE 0


class Example_InstancingAdvanced : public ExampleBase
{

    #define foreach_dimension(X, Y, Z, VEC) \
        for_range(Z, (VEC).z) for_range(Y, (VEC).y) for_range(X, (VEC).x)

    static constexpr int        boundingVolume[3]   = { 60, 45, 60 };
    static constexpr float      cameraTurnSpeed     = 0.25f;
    static constexpr float      cameraMoveSpeed     = 0.25f;
    static constexpr float      previewBlockOffsetY = 0.1f;     // Y-axis world position offset for the preview block before putting it into place

    const char*                 sceneFilename       = "Bricks.txt";

    enum BrickPSO
    {
        BrickPSO_Default = 0,
        BrickPSO_Transparent,
        BrickPSO_Wireframe,
        BrickPSO_Preview, // With dithering to indicate blocked position

        BrickPSO_Num,
    };

    LLGL::Shader*               vsBrick             = nullptr;
    LLGL::Shader*               fsBrick             = nullptr;
    LLGL::Shader*               vsBrickPreview      = nullptr;
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
        float           pad0            = {};
        Gs::Vector4f    viewPos;                                    // Camera view position (in world space)
        Gs::Vector3f    lightDir        = { 0.0f, 0.0f, -1.0f };    // Light vector pointing from a pixel towards the light source
        float           specularPower   = 90.0f;                    // Specular exponent for blinn-phone shading model

        // Preview shaders only
        float           worldOffsetY    = 0.0f;
        std::int32_t    isDithering     = 0;
        float           pad1[2]         = {};
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

        BrickType_Num,
    };

    enum BrickGroup : std::uint32_t
    {
        BrickGroup_Default = 0,
        BrickGroup_WallsOnly,
        BrickGroup_Transparent,

        BrickGroup_Num,
    };

    // Returns the specified BrickType as a string representation. Used by the scene file writer.
    static const char* BrickTypeToString(BrickType t)
    {
        switch (t)
        {
            case BrickType_2x2x3:   return "2x2x3";
            case BrickType_4x2x3:   return "4x2x3";
            case BrickType_4x2x1:   return "4x2x1";
            default:                return "";
        }
    }

    // Returns the BrickType that's mapped from the specified string. Used by the scene file reader.
    static BrickType BrickTypeFromString(const char* s)
    {
        if (::strncmp(s, "2x2x3", 5) == 0) return BrickType_2x2x3;
        if (::strncmp(s, "4x2x3", 5) == 0) return BrickType_4x2x3;
        if (::strncmp(s, "4x2x1", 5) == 0) return BrickType_4x2x1;
        return BrickType_Num;
    }

    static std::uint32_t CombineToBrickId(int type, int group)
    {
        return static_cast<std::uint32_t>(type * BrickGroup_Num + group);
    }

    static BrickType IdToBrickType(std::uint32_t groupId)
    {
        return static_cast<BrickType>(groupId / BrickGroup_Num);
    }

    static BrickGroup IdToBrickGroup(std::uint32_t groupId)
    {
        return static_cast<BrickGroup>(groupId % BrickGroup_Num);
    }

    struct PreviewBrick
    {
        BrickType       type            = BrickType_2x2x3; // Previews brick with instance index 0.
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
    InstanceGroup           instanceGroups[BrickType_Num * BrickGroup_Num];

    InstanceGroup& GetInstanceGroup(BrickType type, BrickGroup group = BrickGroup_Default)
    {
        return instanceGroups[CombineToBrickId(type, group)];
    }

    struct Brick
    {
        Brick() :
            groupId       { 0xF },
            groupInstance { 0   }
        {
        }

        Brick(std::uint32_t groupId, std::uint32_t groupInstance) :
            groupId       { groupId       },
            groupInstance { groupInstance }
        {
        }

        // Combined brick type and group.
        std::uint32_t   groupId         : 4;

        // Index into the group's instance. Use `group.firstInstance + groupInstance` to get index into global `instances` container.
        std::uint32_t   groupInstance   : 28;

        void SetType(BrickType type)
        {
            groupId = type * BrickGroup_Num + IdToBrickGroup(groupId);
        }

        void SetGroup(BrickGroup group)
        {
            groupId = IdToBrickType(groupId) * BrickGroup_Num + group;
        }

        // Returns true if this brick blocks the view from other bricks. False for invalid and transparent bricks.
        bool IsBlocking() const
        {
            return (groupId != 0xF && IdToBrickGroup(groupId) != BrickGroup_Transparent);
        }

        operator bool() const
        {
            return (groupId != 0xF);
        }

        bool operator == (const Brick& rhs) const
        {
            return (groupId == rhs.groupId && groupInstance == rhs.groupInstance);
        }

        // Required for std::set. Without it, the implicit bool conversion would be used and treat all valid bricks as equal.
        bool operator < (const Brick& rhs) const
        {
            if (groupId != rhs.groupId)
                return (groupId < rhs.groupId);
            else
                return (groupInstance < rhs.groupInstance);
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
        Move,
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

        void FindAndUpdateLeaf(const Gs::Vector3i& gridPos, const AABB3i& aabb, std::uint32_t newGroupInstance)
        {
            if (HasChildren())
            {
                // Shrink root AABB until we reach a leaf node
                if (gridPos[axis] < aabb.bbMin[axis] + static_cast<std::int32_t>(distance))
                    child[0]->FindAndUpdateLeaf(gridPos, GetChild0AABB(axis, aabb), newGroupInstance);
                else
                    child[1]->FindAndUpdateLeaf(gridPos, GetChild1AABB(axis, aabb), newGroupInstance);
            }
            else if (aabb.bbMax == aabb.bbMin + Gs::Vector3i{ 1 })
            {
                // Update leaf entry if not empty
                if (this->brick)
                    this->brick.groupInstance = newGroupInstance;
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

        static bool IsInsideBoundingVolume(const Gs::Vector3i& gridPos)
        {
            return
            (
                gridPos.x >=  KDTreeNode::rootAABB.bbMin.x &&
                gridPos.y >=  KDTreeNode::rootAABB.bbMin.y &&
                gridPos.z >=  KDTreeNode::rootAABB.bbMin.z &&
                gridPos.x <   KDTreeNode::rootAABB.bbMax.x &&
                gridPos.y <   KDTreeNode::rootAABB.bbMax.y &&
                gridPos.z <   KDTreeNode::rootAABB.bbMax.z
            );
        }

        KDTreeNode() :
            axis     { Axis_X },
            distance { 0      }
        {
        }

        bool HasChildren() const
        {
            return (child[0].get() != nullptr);
        }

        void InsertLeaf(const Gs::Vector3i& gridPos, Brick brick)
        {
            FindAndInsertLeaf(gridPos, KDTreeNode::rootAABB, brick);
        }

        void RemoveLeaf(const Gs::Vector3i& gridPos)
        {
            FindAndRemoveLeaf(gridPos, KDTreeNode::rootAABB);
        }

        void UpdateLeaf(const Gs::Vector3i& gridPos, std::uint32_t groupInstance)
        {
            FindAndUpdateLeaf(gridPos, KDTreeNode::rootAABB, groupInstance);
        }

        Brick GetLeaf(const Gs::Vector3i& gridPos) const
        {
            // Positions outside the root AABB have no leaf. Without this check, the traversal would return an unrelated leaf.
            if (KDTreeNode::IsInsideBoundingVolume(gridPos))
                return FindLeaf(gridPos, KDTreeNode::rootAABB);
            else
                return Brick{};
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

        // Deletes the entire tree.
        void Reset()
        {
            child[0].reset();
            child[1].reset();
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

        // Load previoulsy saved scene or create default initial scene on failure.
        if (!LoadScene(sceneFilename))
            CreateDefaultScene();

        // Place initial camera position
        camera.position = { 0, 10.0f, -20.0f };
        camera.pitch    = 20.0f;

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

    Brick AddInstanceToGroup(std::uint32_t groupId, const Gs::Vector3i& gridPos, GridRotation gridRotation = GridRotation_0, LLGL::ColorRGBAub color = {}, bool notifyNeighbors = false)
    {
        Brick newBrick{ groupId, instanceGroups[groupId].numInstances };

        if (Instance* newInstance = AllocateInstancesInGroup(groupId))
        {
            newInstance->gridPos        = gridPos;
            newInstance->gridRotation   = gridRotation;
            newInstance->color          = color;

            InsertBrickIntoGrid(gridPos, newBrick, notifyNeighbors);
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
            const std::uint32_t newGroupId = CombineToBrickId(IdToBrickType(brick.groupId), newGroup);
            (void)AddInstanceToGroup(newGroupId, oldInstance.gridPos, oldInstance.gridRotation, oldInstance.color);
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

    static Gs::Vector3i GetBrickExtent(BrickType type)
    {
        switch (type)
        {
            case BrickType_2x2x3:   return { 2, 3, 2 };
            case BrickType_4x2x3:   return { 4, 3, 2 };
            case BrickType_4x2x1:   return { 4, 1, 2 };
            default:                return {};
        }
    }

    BrickDimensions GetBrickDimensions(BrickType type, GridRotation rotation)
    {
        BrickDimensions brickDim;

        brickDim.extent = GetBrickExtent(type);
        if (rotation == GridRotation_90)
            std::swap(brickDim.extent.x, brickDim.extent.z);

        brickDim.offset.x = -brickDim.extent.x/2;
        brickDim.offset.y = 0;
        brickDim.offset.z = -brickDim.extent.z/2;

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
            return IsBrickTypeBlockedTopAndBottom(IdToBrickType(brick.groupId), instance->gridPos, instance->gridRotation);
        else
            return false;
    }

    // Notifies the specified brick that its surrounding has changed,
    // allowing it to transfer to a different mesh group for better rendering performance.
    void NotifyBrickChange(Brick brick, bool isBlocking)
    {
        const BrickGroup group = IdToBrickGroup(brick.groupId);
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

    void InsertBrickIntoGrid(const Gs::Vector3i& gridPos, Brick brick, bool notifyNeighbors = false)
    {
        // Get brick dimension and apply rotation
        const Instance* instance = GetBrickInstance(brick);
        const BrickDimensions brickDim = GetBrickDimensions(IdToBrickType(brick.groupId), instance != nullptr ? instance->gridRotation : GridRotation_0);

        // Insert brick entries into grid
        const Gs::Vector3i alignedGridPos = gridPos + brickDim.offset;
        foreach_dimension(x, y, z, brickDim.extent)
            grid.InsertLeaf(alignedGridPos + Gs::Vector3i{ x, y, z }, brick);

        // Notify top and bottom grid entries that they are now blocked, so they can optimize their mesh if they are now fully blocked
        if (notifyNeighbors)
            NotifyBrickChangeTopAndBottom(alignedGridPos, brickDim.extent, brick);

        #if DEBUG_KDTREE
        LLGL::Log::Printf("kd-Tree debug info: Nodes=%u, Leafs=%u\n", grid.NumNodes(), grid.NumLeafs());
        #endif
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
        LoadBrickModel("Brick2x2x3"          , vertices, GetInstanceGroup(BrickType_2x2x3                        ));
        LoadBrickModel("Brick2x2x3_WallsOnly", vertices, GetInstanceGroup(BrickType_2x2x3, BrickGroup_WallsOnly  ));
        LoadBrickModel("Brick2x2x3_Glass"    , vertices, GetInstanceGroup(BrickType_2x2x3, BrickGroup_Transparent));
        LoadBrickModel("Brick4x2x3"          , vertices, GetInstanceGroup(BrickType_4x2x3                        ));
        LoadBrickModel("Brick4x2x3_WallsOnly", vertices, GetInstanceGroup(BrickType_4x2x3, BrickGroup_WallsOnly  ));
        LoadBrickModel("Brick4x2x3_Glass"    , vertices, GetInstanceGroup(BrickType_4x2x3, BrickGroup_Transparent));
        LoadBrickModel("Brick4x2x1"          , vertices, GetInstanceGroup(BrickType_4x2x1                        ));
        LoadBrickModel("Brick4x2x1_WallsOnly", vertices, GetInstanceGroup(BrickType_4x2x1, BrickGroup_WallsOnly  ));
        LoadBrickModel("Brick4x2x1_Glass"    , vertices, GetInstanceGroup(BrickType_4x2x1, BrickGroup_Transparent));

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

        // Initialize scene by restting it. This initializes the instance for the preview mesh.
        ResetScene();
    }

    void CreatePipelines()
    {
        // Create shaders
        vsBrick         = LoadVertexShader  ("InstancingAdvanced", "VSBrick");
        fsBrick         = LoadFragmentShader("InstancingAdvanced", "PSBrick");
        vsBrickPreview  = LoadVertexShader  ("InstancingAdvanced", "VSBrickPreview");
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

        // Create graphics PSO for default scene rendering
        auto SetPSODefaults = [this, &vertexAttribs](LLGL::GraphicsPipelineDescriptor& psoDesc) -> void
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
            psoDebugDesc.vertexShader       = LoadVertexShader  ("InstancingAdvanced", "VSDebug");
            psoDebugDesc.fragmentShader     = LoadFragmentShader("InstancingAdvanced", "PSDebug");
            psoDebugDesc.primitiveTopology  = LLGL::PrimitiveTopology::LineList;
            psoDebugDesc.depth.testEnabled  = true;
        }
        psoDebug = renderer->CreatePipelineState(psoDebugDesc);
        ReportPSOErrors(psoDebug);
        #endif
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
        file << "# Scene file for LLGL's example 'InstancingAdvanced'" << std::endl;
        file << "# Current brick type: 'b <DIMENSIONS>' e.g. 'b 2x2x3', supported dimensions are 2x2x3, 4x2x3, 4x2x1" << std::endl;
        file << "# New instance: 'i X Y Z ROTATE RGBA' e.g. 'i 10 0 -3 90 FFFF00FF', ROTATE must be 0 or 90" << std::endl;

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
                        file << "b " << BrickTypeToString(static_cast<BrickType>(brickType)) << std::endl;
                    }
                }

                // Write all instances
                for_range(i, group.numInstances)
                {
                    const Instance& instance = instances[group.firstInstance + i];
                    file
                        << "i"
                        << " " << instance.gridPos.x
                        << " " << instance.gridPos.y
                        << " " << instance.gridPos.z
                        << " " << (instance.gridRotation == GridRotation_90 ? "90" : "0")
                        << " " << ColorToString(instance.color)
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

            if (cmd == 'b')
            {
                // Parse brick type and make it current
                std::string brickTypeStr;
                stream >> brickTypeStr;
                currentBrickType = BrickTypeFromString(brickTypeStr.c_str());

                if (currentBrickType == BrickType_Num)
                {
                    LLGL::Log::Errorf("Unknown brick type 'b %s' in %s:%d\n", brickTypeStr.c_str(), filename, lineNo);
                    ResetScene();
                    return false;
                }
            }
            else if (cmd == 'i')
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

                if (gridRotation != 0 && gridRotation != 90)
                    LLGL::Log::Errorf("Ignored unknown brick rotation %d (must be 0 or 90) in %s:%d\n", gridRotation, filename, lineNo);

                // Create new brick instance
                NewBrick(currentBrickType, gridPos, (gridRotation == 90 ? GridRotation_90 : GridRotation_0), ColorFromString(colorStr));
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

        const InstanceGroup& group = GetInstanceGroup(previewBrick.type, previewBrick.isTransparent ? BrickGroup_Transparent : BrickGroup_Default);
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

    void ClampBrickGridOffset(BrickType type, std::int32_t& offset)
    {
        const Gs::Vector3i brickExtent = GetBrickExtent(type);
        const std::int32_t maxWidthOrDepth = std::max(brickExtent.x, brickExtent.z);
        const std::int32_t maxOffset = maxWidthOrDepth/2 - 1;
        offset = Gs::Clamp(offset, -maxOffset, +maxOffset);
    }

    void PickGridPosForPreviewBrick(PreviewBrick& previewBrick, Instance& previewInstance)
    {
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
            // Find a potential grid position for the preview brick
            newPreviewGridPos = WorldPosToGridPos(contact.point);

            auto ApplyGridOffset = [&newPreviewGridPos, &previewBrick, newPreviewRotation]() -> void
            {
                // Offset by user input to allow placing bricks on ledges
                if (newPreviewRotation == GridRotation_90)
                    newPreviewGridPos.z += previewBrick.gridOffset;
                else
                    newPreviewGridPos.x += previewBrick.gridOffset;
            };

            const BrickDimensions brickDim = GetBrickDimensions(newBrickType, newPreviewRotation);
            if (contact.normal.x < 0)
                newPreviewGridPos.x -= brickDim.offset.x + brickDim.extent.x;
            else if (contact.normal.x > 0)
                newPreviewGridPos.x -= brickDim.offset.x;
            else if (contact.normal.y < 0)
            {
                newPreviewGridPos.y -= brickDim.offset.y + brickDim.extent.y;
                ApplyGridOffset();
            }
            else if (contact.normal.y > 0)
            {
                newPreviewGridPos.y -= brickDim.offset.y;
                ApplyGridOffset();
            }
            else if (contact.normal.z < 0)
                newPreviewGridPos.z -= brickDim.offset.z + brickDim.extent.z;
            else if (contact.normal.z > 0)
                newPreviewGridPos.z -= brickDim.offset.z;
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
        PickGridPosForPreviewBrick(viewOptions.previewBrick, instances[0]);

        const PreviewBrick& previewBrick = viewOptions.previewBrick;
        if (!previewBrick.isBlocked && input.KeyDown(LLGL::Key::LButton))
        {
            // Place new brick and update pick position for preview
            NewBrick(previewBrick.type, instances[0].gridPos, instances[0].gridRotation, instances[0].color, previewBrick.isTransparent);
            PickGridPosForPreviewBrick(viewOptions.previewBrick, instances[0]);

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
                        viewOptions.previewBrick.type           = IdToBrickType(selectedBrick.groupId);
                        viewOptions.previewBrick.isTransparent  = (IdToBrickGroup(selectedBrick.groupId) == BrickGroup_Transparent);
                        previewInstance.color           = selectedInstance->color;
                        previewInstance.gridRotation    = selectedInstance->gridRotation; 
                    }
                    controller.editMode = EditMode::Move;
                }

                // Remove selected brick
                DestroyBrick(selectedBrick);
            }
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

        if (controller.cameraMode == CameraMode::None)
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

    void UpdateSceneView()
    {
        // Update common scene view states
        sceneView.viewPos       = camera.wMatrix * Gs::Vector4f{ 0, 0, 0, 1 };
        sceneView.vpMatrix      = projection * camera.vMatrix;

        sceneView.gridScale     = KDTreeNode::gridScale;

        sceneView.lightDir      = Gs::Vector3f{ -0.2f, +0.4f, -1.0f }.Normalized();

        // Update preview shader states
        //sceneView.worldOffsetY  = (previewBlockOffsetY;
        sceneView.isDithering   = (viewOptions.previewBrick.isBlocked ? 1 : 0);
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

                if (controller.cameraMode == CameraMode::None && (controller.editMode == EditMode::Add || controller.editMode == EditMode::Move))
                {
                    commands->PushDebugGroup("RenderPreviewBrick");
                    {
                        RenderPreviewBrick(viewOptions.previewBrick);
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

int                 Example_InstancingAdvanced::KDTreeNode::numAABBTests    = 0;
const Gs::Vector3f  Example_InstancingAdvanced::KDTreeNode::gridScale       = { 1.0f, 0.5f, 1.0f };
const AABB3i        Example_InstancingAdvanced::KDTreeNode::rootAABB
{
    Gs::Vector3i{ -boundingVolume[0]/2,                  0, -boundingVolume[2]/2 },
    Gs::Vector3i{ +boundingVolume[0]/2, +boundingVolume[1], +boundingVolume[2]/2 }
};


LLGL_IMPLEMENT_EXAMPLE(Example_InstancingAdvanced);



