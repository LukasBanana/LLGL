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


class Example_InstancingAdvanced : public ExampleBase
{

    #define foreach_dimension(X, Y, Z, VEC) \
        for_range(Z, (VEC).z) for_range(Y, (VEC).y) for_range(X, (VEC).x)

    static constexpr int        boundingVolume[3]   = { 60, 45, 60 };

    LLGL::Shader*               vertexShader        = nullptr;
    LLGL::Shader*               fragmentShader      = nullptr;

    LLGL::PipelineLayout*       psoLayout           = nullptr;
    LLGL::PipelineState*        pso[2]              = {};

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

    struct ViewOptions
    {
        bool showWireframe = false;
    }
    viewOptions;

    enum GridRotation : std::int32_t
    {
        GridRotation_0 = 0,
        GridRotation_90,
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

        // Zero-based index to the first instance into the global `instances` container for this group
        std::uint32_t firstInstance     = 0;

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

        BrickType       groupId         : 3;

        // Index into the group's instance. Use `group.firstInstance + groupInstance` to get index into global `instances` container.
        std::uint32_t   groupInstance   : 29;

        inline operator bool() const
        {
            return (groupId != BrickType_Num);
        }
    };

    // State tracking for picking scene objects with the mouse pointer.
    struct Picker
    {
        Brick               lastPickedBrick;
        LLGL::ColorRGBAub   lastPickedInstanceColor;

        Brick               pendingBrick;
    }
    picker;

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
                distance = static_cast<std::uint32_t>(aabbExtent[axis]/2);
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

        Brick FindIntersectionWithLeaf(const Ray3f& ray, const AABB3i& aabb, Intersection* outIntersection) const
        {
            if (HasChildren())
            {
                // Check intersection against both child nodes' AABBs and continue down the path of the closer intersection
                Intersection childIntersect[2];

                const AABB3i child0AABB = GetChild0AABB(axis, aabb);
                const bool isChild0Intersect = IntersectRayWithAABB(ray, child0AABB, &(childIntersect[0]));

                const AABB3i child1AABB = GetChild1AABB(axis, aabb);
                const bool isChild1Intersect = IntersectRayWithAABB(ray, child1AABB, &(childIntersect[1]));

                if (isChild0Intersect && isChild1Intersect)
                {
                    // Continue intersection down the path of the node with the closest intersection
                    if (childIntersect[0].interpolator < childIntersect[1].interpolator)
                        return child[0]->FindIntersectionWithLeaf(ray, child0AABB, outIntersection);
                    else
                        return child[1]->FindIntersectionWithLeaf(ray, child1AABB, outIntersection);
                }
                else if (isChild0Intersect)
                    return child[0]->FindIntersectionWithLeaf(ray, child0AABB, outIntersection);
                else if (isChild1Intersect)
                    return child[1]->FindIntersectionWithLeaf(ray, child1AABB, outIntersection);
            }
            else if (aabb.bbMax == aabb.bbMin + Gs::Vector3i{ 1 })
            {
                // Make final intersection test against AABB of current node
                if (IntersectRayWithAABB(ray, aabb, outIntersection))
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

        static const AABB3i rootAABB;

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

        Brick IntersectWithLeaf(const Ray3f& ray, Intersection* outIntersection = nullptr) const
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
    Instance* AddInstancesToGroup(BrickType groupId, std::uint32_t count)
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

            // Update instances references in grid
            //TODO
        }

        // Return pointer to first new instance
        Instance* firstInstancePtr = &(instances[group.firstInstance + group.numInstances]);
        group.numInstances += count;
        MarkInstanceDirty(group.firstInstance, count);

        return firstInstancePtr;
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
            (static_cast<float>(screenPos.x) - viewport.x) / viewport.width,
            (static_cast<float>(screenPos.y) - viewport.y) / viewport.height,
            1.0f,
            0.0f,
        };

        ndc = projection.Inverse() * ndc;
        ndc /= ndc.w;

        return Ray3f
        {
            Gs::Vector3f{ sceneView.viewPos.x, sceneView.viewPos.y, sceneView.viewPos.z },
            Gs::Vector3f{ ndc.x, ndc.y, ndc.z }.Normalized()
        };
    }

    Gs::Vector3i GetBrickTypeDimensions(BrickType type)
    {
        switch (type)
        {
            case BrickType_2x2x3:
            case BrickType_2x2x3_WallsOnly:
                return { 2, 3, 2 };
            case BrickType_4x2x3:
            case BrickType_4x2x3_WallsOnly:
                return { 4, 3, 2 };
            case BrickType_4x2x1:
            case BrickType_4x2x1_WallsOnly:
                return { 4, 1, 2 };
            default:
                return { 0, 0, 0 };
        }
    }

    bool BrickFitsInGrid(const Gs::Vector3i& gridPos, BrickType type, GridRotation rotation)
    {
        // Get brick dimension and apply rotation
        Gs::Vector3i brickDim = GetBrickTypeDimensions(type);
        if (rotation == GridRotation_90)
            std::swap(brickDim.x, brickDim.z);

        // Brick must fit inside the root AABB
        if (gridPos.x              < KDTreeNode::rootAABB.bbMin.x ||
            gridPos.y              < KDTreeNode::rootAABB.bbMin.y ||
            gridPos.z              < KDTreeNode::rootAABB.bbMin.z ||
            gridPos.x + brickDim.x > KDTreeNode::rootAABB.bbMax.x ||
            gridPos.y + brickDim.y > KDTreeNode::rootAABB.bbMax.y ||
            gridPos.z + brickDim.z > KDTreeNode::rootAABB.bbMax.z)
        {
            return false;
        }

        // Check if there is any blocking brick
        foreach_dimension(x, y, z, brickDim)
        {
            if (grid.GetLeaf(gridPos + Gs::Vector3i{ x, y, z }))
                return false;
        }

        return true;
    }

    bool SetBrickInGrid(const Gs::Vector3i& gridPos, Brick brick)
    {
        // Get brick dimension and apply rotation
        Gs::Vector3i brickDim = GetBrickTypeDimensions(brick.groupId);
        if (const Instance* instance = GetBrickInstance(brick))
        {
            if (instance->gridRotation == GridRotation_90)
                std::swap(brickDim.x, brickDim.z);
        }

        if (brick)
        {
            // Insert brick entries into grid
            foreach_dimension(x, y, z, brickDim)
                grid.InsertLeaf(gridPos + Gs::Vector3i{ x, y, z }, brick);
        }
        else
        {
            // Remove brick entries from grid
            foreach_dimension(x, y, z, brickDim)
                grid.RemoveLeaf(gridPos + Gs::Vector3i{ x, y, z });
        }

        #if DEBUG_KDTREE
        LLGL::Log::Printf("kd-Tree debug info: Nodes=%u, Leafs=%u\n", grid.NumNodes(), grid.NumLeafs());
        #endif

        return true;
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
        Instance* newInstance = AddInstancesToGroup(type, 1);
        {
            newInstance->gridPos        = gridPos;
            newInstance->gridRotation   = gridRotation;
            newInstance->color          = color;
        }

        SetBrickInGrid(gridPos, newBrick);

        return newBrick;
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
    }

    void CreatePipelines()
    {
        // Create shaders
        vertexShader    = LoadVertexShader("InstancingAdvanced", "VSMain");
        fragmentShader  = LoadFragmentShader("InstancingAdvanced", "PSMain");

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
            psoDesc.inputVertexAttribs             = vertexAttribs;
            psoDesc.vertexShader                   = vertexShader;
            psoDesc.fragmentShader                 = fragmentShader;
            psoDesc.pipelineLayout                 = psoLayout;
            psoDesc.primitiveTopology              = LLGL::PrimitiveTopology::TriangleList;
            psoDesc.depth.testEnabled              = true;
            psoDesc.depth.writeEnabled             = true;
            psoDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
        }
        pso[0] = renderer->CreatePipelineState(psoDesc);
        ReportPSOErrors(pso[0]);
        {
            psoDesc.rasterizer.polygonMode          = LLGL::PolygonMode::Wireframe;
        }
        pso[1] = renderer->CreatePipelineState(psoDesc);
        ReportPSOErrors(pso[1]);
    }

    void CreateInitialScene()
    {
        //TODO

        for (int i = 0; i <= 4; ++i)
        {
            for (int j = 0; j <= 4 - i; ++j)
            {
                NewBrick(BrickType_4x2x3, { 4*(i-2)+j*2, j*3,  0 }, GridRotation_0, colorPalette[i%5]);
            }
        }

        //NewBrick(BrickType_4x2x1, { 0, 0, -4 }, GridRotation_90, colorPalette[1]);
        NewBrick(BrickType_2x2x3, { 0, 0, -2 }, GridRotation_0, colorPalette[1]);

    }

    void MainRenderPass()
    {
        // Exit early if no instances have been allocated yet
        if (instanceBuffer == nullptr)
            return;

        commands->SetPipelineState(*pso[viewOptions.showWireframe ? 1 : 0]);

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

    void ProcessUserInput()
    {
        if (input.KeyDown(LLGL::Key::Tab))
            viewOptions.showWireframe = !viewOptions.showWireframe;

        // Pick scene objects with the mouse pointer
        const Ray3f pickerRay = ProjectScreenPosToViewRay(input.GetMousePosition());
        Intersection pickedIntersection;

        if (Brick pickedBrick = grid.IntersectWithLeaf(pickerRay, &pickedIntersection))
        {
            // Reset previously picked brick
            if (Instance* oldBrickInstance = GetMutableBrickInstance(picker.lastPickedBrick))
            {
                oldBrickInstance->color = picker.lastPickedInstanceColor;
                picker.lastPickedBrick = {};
            }

            // Highlight new picked brick
            if (Instance* newBrickInstance = GetMutableBrickInstance(pickedBrick))
            {
                picker.lastPickedInstanceColor = newBrickInstance->color;
                picker.lastPickedBrick = pickedBrick;

                newBrickInstance->color = LLGL::ColorRGBAub{ 255, 0, 0, 255 };
            }

            // Place pending brick at picked location
            if (picker.pendingBrick)
            {
                const Gs::Vector3f pickedIntersectionPoint = pickerRay.origin + pickerRay.direction * pickedIntersection.interpolator;
                const Gs::Vector3i pickedIntersectionGridPos = pickedIntersectionPoint.Cast<std::int32_t>();

                (void)SetBrickInGrid(pickedIntersectionGridPos, picker.pendingBrick);
            }
        }
    }

    void UpdateSceneView()
    {
        Gs::Matrix4f vMatrix;
        vMatrix.LoadIdentity();
        Gs::Translate(vMatrix, Gs::Vector3f{ 0, 10, -15 });
        Gs::RotateX(vMatrix, Gs::Deg2Rad(25.0f));

        sceneView.viewPos = vMatrix * Gs::Vector4f{ 0, 0, 0, 1 };

        vMatrix.MakeInverse();
        sceneView.vpMatrix = projection * vMatrix;

        sceneView.gridScale = { 1.0f, 0.5f, 1.0f };

        sceneView.lightDir = Gs::Vector3f{ -0.2f, +0.4f, -1.0f }.Normalized();
    }

    void OnDrawFrame(float /*dt*/) override
    {
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

                MainRenderPass();
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

LLGL_IMPLEMENT_EXAMPLE(Example_InstancingAdvanced);



