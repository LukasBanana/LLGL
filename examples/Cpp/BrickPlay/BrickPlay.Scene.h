/*
 * BrickPlay.Scene.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_EXAMPLE_BRICKPLAY_BRICKSCENE_H
#define LLGL_EXAMPLE_BRICKPLAY_BRICKSCENE_H


#include "BrickPlay.Brick.h"
#include "BrickPlay.KDTree.h"
#include "BrickPlay.Utils.h"
#include <ExampleBase.h>
#include <algorithm>
#include <functional>
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Container/SmallVector.h>
#include <set>
#include <list> // Linked list for undo/redo (ring buffer would be better, but simple enough)


#define DEBUG_INSTANCES 0


// Brick instance structure. Stores the position, rotation, and color.
struct Instance
{
    std::int16_t    gridPos[3];
    GridRotation    gridRotation;
    std::uint8_t    colorIndex;

    inline GridVec3i GetPos() const
    {
        return GridVec3i{ gridPos[0], gridPos[1], gridPos[2] };
    }

    inline void SetPos(const GridVec3i& pos)
    {
        gridPos[0] = pos[0];
        gridPos[1] = pos[1];
        gridPos[2] = pos[2];
    }
};

struct RayIntersection
{
    Gs::Vector3f    point;
    GridVec3i       normal;
    Brick           brick;
};

// Owns the scene with its bricks, instance buffer, and the grid (kd-Tree) representation.
class BrickScene
{

public:

    // Function callback to notify the collider about a contact with a brick.
    using OnCollisionContact = std::function<void(Brick brick, const Gs::Vector3f& contactPoint, float impact)>;

    // Function callback to paint the color of a brick for auto-generated chunks.
    using OnBrickPaint = std::function<int(BrickId brickId, const GridVec3i& gridPos)>;

    using InstanceGroupIterator = std::function<void(BrickId brickId, BrickGroup groupType, InstanceGroup& group)>;
    using InstanceGroupConstIterator = std::function<void(BrickId brickId, BrickGroup groupType, const InstanceGroup& group)>;

private:

    #define foreach_dimension(X, Y, Z, VEC) \
        for_range(Z, (VEC).z) for_range(Y, (VEC).y) for_range(X, (VEC).x)

    LLGL::RenderSystem&             renderSystem;

    LLGL::Buffer*                   instanceBuffer              = nullptr;
    std::uint64_t                   instanceBufferSize          = 0;

    std::vector<Instance>           instances;
    std::size_t                     instanceDirtyRange[2]       = { 0, 0 };

    // The scene grid stores cells with references to the brick instances.
    // Used for scene picking, mesh optimization, and collision detection.
    KDTreeNode                      grid;

    bool                            hasUnsavedChanges           = false;

public:

    BrickScene(const BrickScene&) = delete;
    BrickScene& operator = (const BrickScene&) = delete;

    BrickScene(LLGL::RenderSystem& renderSystem) :
        renderSystem { renderSystem }
    {
        // Initialize scene by restting it. This initializes the instance for the preview mesh.
        ResetScene();
        InitInstanceGroupDebugNames();
    }

    ~BrickScene()
    {
        ReleaseInstanceBuffer();
    }

    bool HasUnsavedChanges() const
    {
        return hasUnsavedChanges;
    }

    void MarkSaved()
    {
        hasUnsavedChanges = false;
    }

    LLGL::Buffer* GetInstanceBuffer() const
    {
        return instanceBuffer;
    }

    Brick FindBrickAt(const GridVec3i& gridPos) const
    {
        return grid.GetLeaf(gridPos);
    }

    const InstanceGroup& GetInstanceGroupById(std::uint32_t groupId) const
    {
        return GetInstanceGroup(GroupIdToBrickId(groupId), GroupIdToBrickGroup(groupId));
    }

    InstanceGroup& GetInstanceGroupById(std::uint32_t groupId)
    {
        return GetInstanceGroup(GroupIdToBrickId(groupId), GroupIdToBrickGroup(groupId));
    }

    const InstanceGroup& GetInstanceGroup(BrickId id, BrickGroup group = BrickGroup_Default) const
    {
        LLGL_VERIFY(group < BrickGroup_Num);
        return BrickDictionary::Get().GetInfo(id).instanceGroups[group];
    }

    InstanceGroup& GetInstanceGroup(BrickId id, BrickGroup group = BrickGroup_Default)
    {
        LLGL_VERIFY(group < BrickGroup_Num);
        return BrickDictionary::Get().GetInfo(id).instanceGroups[group];
    }

    void ForEachInstanceGroup(const InstanceGroupIterator& iterator, bool includeAllGroups = false)
    {
        for_range(brickId, BrickDictionary::Get().NumBrickTypes())
        {
            BrickInfo& brickInfo = BrickDictionary::Get().GetInfo(brickId);
            const std::uint32_t numGroups = (includeAllGroups ? BrickGroup_Num : brickInfo.numGroups);
            for_range(groupType, numGroups)
            {
                iterator(brickId, static_cast<BrickGroup>(groupType), brickInfo.instanceGroups[groupType]);
            }
        }
    }

    void ForEachInstanceGroup(const InstanceGroupConstIterator& iterator, bool includeAllGroups = false) const
    {
        for_range(brickId, BrickDictionary::Get().NumBrickTypes())
        {
            const BrickInfo& brickInfo = BrickDictionary::Get().GetInfo(brickId);
            const std::uint32_t numGroups = (includeAllGroups ? BrickGroup_Num : brickInfo.numGroups);
            for_range(groupType, numGroups)
            {
                iterator(brickId, static_cast<BrickGroup>(groupType), brickInfo.instanceGroups[groupType]);
            }
        }
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

    void FlushInstanceDirtyRange(LLGL::CommandBuffer& cmdBuffer)
    {
        // Update instances within the range that is marked as dirty
        if (instanceDirtyRange[0] < instanceDirtyRange[1])
        {
            LLGL_VERIFY(instanceBuffer != nullptr);

            cmdBuffer.UpdateBuffer(
                *instanceBuffer,
                instanceDirtyRange[0]*sizeof(Instance),
                &(instances[instanceDirtyRange[0]]),
                (instanceDirtyRange[1] - instanceDirtyRange[0])*sizeof(Instance)
            );

            ResetInstanceDirtyRange();
        }
    }

    const Instance& GetInstanceByIndex(std::size_t index) const
    {
        return instances[index];
    }

    const Instance* GetBrickInstance(Brick brick) const
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

    bool NewBrick(BrickId brickId, const GridVec3i& gridPos, GridRotation gridRotation = GridRotation_0, int colorIndex = 0)
    {
        if (!BrickFitsInGrid(brickId, gridPos, gridRotation))
            return false;

        hasUnsavedChanges = true;

        const bool isTransparent = ColorPalette::Get().IsTransparent(colorIndex);
        const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(brickId, gridRotation);

        const BrickGroup group = [this, &isTransparent, &brickDim, &gridPos, brickId]() -> BrickGroup
        {
            if (IsBrickVolumeBlockedTopAndBottom(gridPos, brickDim))
            {
                if (IsBrickVolumeBlockedSideways(gridPos, brickDim))
                    return BrickGroup_Invisible;
                else if (!isTransparent && IsBrickTypeOptimized(brickId))
                    return BrickGroup_WallsOnly;
            }
            if (isTransparent)
                return BrickGroup_Transparent;
            return BrickGroup_Default;
        }();

        const std::uint32_t groupId = CombineToBrickId(brickId, group);

        // Insert brick without notification first, so its instance is fully initialized before neighbors can rearrange the groups
        constexpr bool notifyNeighbors = true;
        return AddInstanceToGroup(groupId, gridPos, gridRotation, colorIndex, notifyNeighbors);
    }

    // Adds new bricks to the scene to fill up the specified AABB and returns the number of bricks that were added.
    int NewBricksChunk(const GridVec3i& bbOffset, const GridVec3i& bbExtent, const OnBrickPaint& painter)
    {
        int count = 0;

        auto NewBrickAndCount = [this, &bbOffset, &count, &painter](int x, int y, int z, int w, int h, int d) -> void
        {
            BrickId brickId = BrickDictionary::Get().FindBrickIdForDimension(GridVec3i{ w, h, d });
            if (brickId != INVALID_BRICK_ID)
            {
                // Counter offset brick so that its boundary is (0,0,0) to (W,H,D). The bricks are usually centered for better user placement.
                const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(brickId, GridRotation_0);
                const GridVec3i gridPos = bbOffset + GridVec3i{ x, y, z } - brickDim.offset;

                if (NewBrick(brickId, gridPos, GridRotation_0, painter(brickId, gridPos)))
                    ++count;
            }
        };

        int y = 0;
        int z = 0;

        auto NewBricksZAxis = [this, &bbExtent, &NewBrickAndCount, &y, &z](int yStep, int zStep) -> void
        {
            for (; z + zStep <= bbExtent.z; z += zStep)
            {
                int x = 0;
              //for (; x + 4 <= bbExtent.x; x += 4) { NewBrickAndCount(x,y,z, 4,yStep,zStep); }
                for (; x + 2 <= bbExtent.x; x += 2) { NewBrickAndCount(x,y,z, 2,yStep,zStep); }
                for (; x + 1 <= bbExtent.x; x += 1) { NewBrickAndCount(x,y,z, 1,yStep,zStep); }
            }
        };

        auto NewBricksYAxis = [this, &bbExtent, &NewBricksZAxis, &y, &z](int yStep) -> void
        {
            for (; y + yStep <= bbExtent.y; y += yStep)
            {
                z = 0;
                NewBricksZAxis(yStep, 2);
                NewBricksZAxis(yStep, 1);
            }
        };

        NewBricksYAxis(3);
        NewBricksYAxis(1);

        return count;
    }

    void DestroyBrick(Brick brick)
    {
        constexpr bool notifyNeighbors = true;
        RemoveInstanceFromGroup(brick.groupId, brick.groupInstance, nullptr, notifyNeighbors);
        hasUnsavedChanges = true;
    }

    void ResetScene()
    {
        // Reset all instances
        instances.clear();

        // Reset instance range of all instance groups (not the vertex range)
        ForEachInstanceGroup(
            [](BrickId /*brickId*/, BrickGroup /*groupType*/, InstanceGroup& group)
            {
                group.numInstances      = 0;
                group.firstInstance     = 0;
                group.instanceCapacity  = 0;
            }
        );

        // Destroy the entire kd-Tree
        grid.Reset();

        hasUnsavedChanges = true;
    }

    bool BrickFitsInGrid(BrickId brickId, const GridVec3i& gridPos, GridRotation rotation) const
    {
        // Get brick dimension and apply rotation
        const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(brickId, rotation);

        // Brick must fit inside the root AABB
        const GridVec3i alignedGridPos = gridPos + brickDim.offset;
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
            if (grid.GetLeaf(alignedGridPos + GridVec3i{ x, y, z }))
                return false;
        }

        return true;
    }

    bool PickScene(const Ray3f& ray, RayIntersection& outContact)
    {
        AABBIntersection aabbIntersection;
        outContact.brick = grid.IntersectWithLeaf(ray, &aabbIntersection);
        if (outContact.brick)
        {
            outContact.point = ray.Interpolate(aabbIntersection.interpolator);
            outContact.normal = aabbIntersection.normal;
            return true;
        }
        else
        {
            const GridVec3i planeNormal{ 0, 1, 0 };
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

    void ResolveCollision(Gs::Vector3f& colliderPoint, float colliderRadius, const OnCollisionContact& onCollision = nullptr) const
    {
        KeepColliderInsidePlayArea(colliderPoint, colliderRadius);

        // Find all potential bricks to collide with
        std::set<Brick> colliderBricks = grid.GatherPotentialColliderBricks(colliderPoint, colliderRadius);

        // Resolve collisions against AABB faces
        auto GetBrickAABB = [this](Brick brick, const Instance& instance, AABB3i& aabb)
        {
            const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(brick.GetId(), instance.gridRotation);

            aabb.bbMin = instance.GetPos() + brickDim.offset;
            aabb.bbMax = aabb.bbMin + brickDim.extent;
        };

        auto ClosestPointOnAABBFace = [](const Gs::Vector3f& point, const AABB3i& aabb, Gs::Vector3f& outClosestPoint) -> bool
        {
            int numAxisInside = 0;

            for (int axis = 0; axis < 3; ++axis)
            {
                const float axisMin = aabb.bbMin[axis] * KDTreeNode::gridScale[axis];
                if (point[axis] < axisMin)
                {
                    outClosestPoint[axis] = axisMin;
                }
                else
                {
                    const float axisMax = aabb.bbMax[axis] * KDTreeNode::gridScale[axis];
                    if (point[axis] > axisMax)
                    {
                        outClosestPoint[axis] = axisMax;
                    }
                    else
                    {
                        outClosestPoint[axis] = point[axis];
                        ++numAxisInside;
                    }
                }
            }

            // Exactly two axes have to be inside for the closest point to be perpendicular to one of the AABB faces 
            return (numAxisInside == 2);
        };

        auto ResolveContact = [&colliderPoint, &onCollision, colliderRadius](Brick brick, const Gs::Vector3f& closestPoint) -> void
        {
            const float distSq = Gs::DistanceSq(colliderPoint, closestPoint);
            if (distSq < colliderRadius*colliderRadius)
            {
                // Resolve collision by pushing input collider out of the impacted contact and invoke the contact callback
                Gs::Vector3f contactVector = colliderPoint - closestPoint;
                const float contactVectorLen = contactVector.Length();
                const float impact = colliderRadius - contactVectorLen;

                contactVector.Normalize();
                colliderPoint += contactVector * impact;

                if (onCollision)
                    onCollision(brick, closestPoint, impact);
            }
        };

        AABB3i brickAABB;
        Gs::Vector3f closestPoint;

        for (Brick brick : colliderBricks)
        {
            if (const Instance* instance = GetBrickInstance(brick))
            {
                GetBrickAABB(brick, *instance, brickAABB);
                if (ClosestPointOnAABBFace(colliderPoint, brickAABB, closestPoint))
                    ResolveContact(brick, closestPoint);
            }
        }

        // Now resolve collisions against AABB edges and corners
        for (Brick brick : colliderBricks)
        {
            if (const Instance* instance = GetBrickInstance(brick))
            {
                GetBrickAABB(brick, *instance, brickAABB);
                closestPoint = ClosestPointOnAABB(colliderPoint, brickAABB);
                ResolveContact(brick, closestPoint);
            }
        }
    }

    std::size_t NumTotalInstances() const
    {
        std::size_t n = 0;
        ForEachInstanceGroup(
            [&n](BrickId /*brickId*/, BrickGroup /*groupType*/, const InstanceGroup& group)
            {
                n += group.numInstances;
            }
        );
        return n;
    }

    void PrintInfo() const
    {
        LLGL::Log::Printf(
            "Scene information:\n"
            "  - Bricks: %s\n"
            "  - Instance GPU buffer size: %s\n"
            "  - kd-Tree nodes: %s\n"
            "  - kd-Tree leaves: %s\n"
            ,
            NumToStr_enUS(NumTotalInstances()).c_str(),
            NumToBytes_enUS(instanceBufferSize).c_str(),
            NumToStr_enUS(grid.NumNodes()).c_str(),
            NumToStr_enUS(grid.NumLeaves()).c_str()
        );
    }

private:

    void InitInstanceGroupDebugNames()
    {
        // Initialize debug group names for all instance groups
        ForEachInstanceGroup(
            [](BrickId brickId, BrickGroup groupType, InstanceGroup& group)
            {
                // Write new brick type
                group.debugName =
                (
                    std::string("InstanceGroup_") +
                    BrickDictionary::Get().GroupToString(static_cast<BrickGroup>(groupType)) + '_' +
                    BrickDictionary::Get().TypeToString(brickId)
                );
            },
            true
        );
    }

    void ReleaseInstanceBuffer()
    {
        if (instanceBuffer != nullptr)
        {
            renderSystem.Release(*instanceBuffer);
            instanceBuffer = nullptr;
        }
    }

    std::size_t BrickToInstance(Brick brick) const
    {
        if (brick)
        {
            const InstanceGroup& group = GetInstanceGroup(brick.GetId(), brick.GetGroup());
            return group.firstInstance + brick.groupInstance;
        }
        return static_cast<std::size_t>(-1);
    }

    // Resizes the instance buffer, but only increases in size.
    bool ResizeInstanceBuffer(std::uint64_t newSize)
    {
        if (instanceBufferSize < newSize)
        {
            #if DEBUG_INSTANCES
            LLGL::Log::Printf("DEBUG: ResizeInstanceBuffer(newSize=%zu)\n", static_cast<std::size_t>(newSize));
            #endif

            // Resize buffer if new required size is larger than the current instance buffer
            ReleaseInstanceBuffer();

            LLGL::BufferDescriptor instBufferDesc;
            {
                instBufferDesc.debugName    = "BrickInstances";
                instBufferDesc.size         = newSize;
                instBufferDesc.stride       = sizeof(Instance);
                instBufferDesc.bindFlags    = LLGL::BindFlags::VertexBuffer;
            }
            instanceBuffer = renderSystem.CreateBuffer(instBufferDesc);

            // Store new instance buffer size
            instanceBufferSize = newSize;

            return true;
        }
        return false;
    }

    // Adds the specified number of instances to the group.
    Instance* AllocateInstancesInGroup(std::uint32_t groupId, std::uint32_t count = 1)
    {
        InstanceGroup& group = GetInstanceGroupById(groupId);
        const std::uint32_t instanceOffset = group.firstInstance + group.numInstances;

        if (group.numInstances + count > group.instanceCapacity)
        {
            const std::uint32_t numMinCount = group.numInstances + count;
            const std::uint32_t numCapacity = numMinCount + numMinCount/2;

            // Insert new instances into global container
            const std::uint32_t instanceGrowth = numCapacity - group.instanceCapacity;
            instances.insert(instances.begin() + instanceOffset, instanceGrowth, Instance{});

            // Resize GPU buffer and mark all instances as dirty if the buffer was re-created
            if (ResizeInstanceBuffer(instances.size() * sizeof(Instance)))
                MarkInstanceDirty(0, instances.size());
            else
                MarkInstanceDirty(instanceOffset, instances.size() - instanceOffset);

            // Update instance offsets in subsequent groups
            #if DEBUG_INSTANCES
            LLGL::Log::Printf(
                "DEBUG: Move instance groups [%u, %u): instanceGrowth=%u\n",
                groupId + 1, BrickDictionary::Get().NumBrickTypes() * BrickGroup_Num, instanceGrowth
            );
            #endif

            for (std::uint32_t nextGroupId = groupId + 1; nextGroupId < BrickDictionary::Get().NumBrickTypes() * BrickGroup_Num; ++nextGroupId)
                GetInstanceGroupById(nextGroupId).firstInstance += instanceGrowth;

            // Store new instance capacity for this group
            group.instanceCapacity = numCapacity;
        }

        // Return pointer to first new instance
        Instance* firstInstancePtr = &(instances[instanceOffset]);
        MarkInstanceDirty(instanceOffset, count);
        group.numInstances += count;

        return firstInstancePtr;
    }

    // This function must not take `gridPos` as constant reference parameter.
    // When it comes from an instance (like the preview model), it will invalidate that pointer in AllocateInstancesInGroup().
    bool AddInstanceToGroup(std::uint32_t groupId, const GridVec3i& gridPos, GridRotation gridRotation = GridRotation_0, int colorIndex = 0, bool notifyNeighbors = false)
    {
        Brick newBrick{ groupId, GetInstanceGroupById(groupId).numInstances };

        if (Instance* newInstance = AllocateInstancesInGroup(groupId))
        {
            newInstance->SetPos(gridPos);
            newInstance->gridRotation   = gridRotation;
            newInstance->colorIndex     = static_cast<std::uint8_t>(colorIndex);

            if (InsertBrickIntoGrid(newBrick, notifyNeighbors))
                return true;
        }

        LLGL::Log::Errorf(
            "Failed to insert brick (groupId=%u, groupInstance=%u) into grid\n",
            newBrick.groupId, newBrick.groupInstance
        );
        return false;
    }

    bool RemoveInstanceFromGroup(std::uint32_t groupId, std::uint32_t groupInstance, Instance* outRemovedInstance = nullptr, bool notifyNeighbors = false)
    {
        // Early exit if instance is out of bounds
        InstanceGroup& group = GetInstanceGroupById(groupId);
        if (groupInstance >= group.numInstances)
            return false;

        // Make copy of retired instance and override that entry with the last instance below.
        // RemoveBrickFromGrid()/InsertBrickIntoGrid() might modify the instances container, so this has to happen first.
        const Instance retiredInstance = instances[group.firstInstance + groupInstance];

        if (outRemovedInstance != nullptr)
            *outRemovedInstance = retiredInstance;

        const BrickId brickId = GroupIdToBrickId(groupId);

        group.numInstances--;
        if (groupInstance < group.numInstances)
        {
            // Remove instance from group by writing last instance in that group into the position of the old instance and update its grid entry
            const Instance& lastInstance = instances[group.firstInstance + group.numInstances];
            instances[group.firstInstance + groupInstance] = lastInstance;
            UpdateBrickInstanceInGrid(brickId, lastInstance, groupInstance);
        }

        // Remove destroyed instance from grid
        RemoveBrickFromGrid(brickId, retiredInstance, notifyNeighbors);

        // Mark all remaining instances of that group as dirty
        MarkInstanceDirty(group.firstInstance + groupInstance);

        return true;
    }

    // Returns true if the specified brick is blocking the view of other bricks. False for transparent bricks.
    bool IsBrickBlocking(Brick brick) const
    {
        if (const Instance* brickInstance = GetBrickInstance(brick))
        {
            const bool isTransparent = ColorPalette::Get().IsTransparent(brickInstance->colorIndex);
            return !isTransparent;
        }
        return false;
    }

    bool IsBrickVolumeBlockedTopAndBottom(const GridVec3i& gridPos, const BrickDimensions& brickDim)
    {
        const GridVec3i alignedGridPos = gridPos + brickDim.offset;
        for_range(yi, 2)
        {
            const std::int32_t y = (yi == 0 ? -1 : brickDim.extent.y);

            // If current grid position is under the floor, it's also considered blocked
            if (alignedGridPos.y + y < KDTreeNode::rootAABB.bbMin.y)
                continue;

            // Iterate through all cells within the current Y-axis layer and break iteration once a brick that is non-blocking has been found
            const bool areAllBlocking = grid.ForEachCell(
                alignedGridPos + GridVec3i{ 0, y, 0 },
                GridVec3i{ brickDim.extent.x, 1, brickDim.extent.z },
                [this](Brick brick, const GridVec3i& /*gridPos*/) -> bool
                {
                    return IsBrickBlocking(brick);
                }
            );
            if (!areAllBlocking)
                return false;
        }
        return true;
    }

    bool IsBrickVolumeBlockedSideways(const GridVec3i& gridPos, const BrickDimensions& brickDim)
    {
        const GridVec3i alignedGridPos = gridPos + brickDim.offset;

        // Any X/Z grid position outside its bounds is considered visible since the camera can move outside the grid, but never under the floor
        if (alignedGridPos.x <= KDTreeNode::rootAABB.bbMin.x || alignedGridPos.x + brickDim.extent.x >= KDTreeNode::rootAABB.bbMax.x ||
            alignedGridPos.z <= KDTreeNode::rootAABB.bbMin.z || alignedGridPos.z + brickDim.extent.z >= KDTreeNode::rootAABB.bbMax.z)
        {
            return false;
        }

        // Iterate through all cells within the AABB of the brick and its neighbor cells. Even though we ignore all the cells that are inside the brick itself,
        // this is a lot faster than probing each cell one by one via `GetLeaf()` since it reduces the amount of tree traversal quite drastically.
        return grid.ForEachCell(
            alignedGridPos + GridVec3i{ -1 },
            brickDim.extent + GridVec3i{ +2 },
            [this, &alignedGridPos, &brickDim](Brick brick, const GridVec3i& cellPos) -> bool
            {
                // Ignore cells inside the brick itself for the entire Y-axis; they are already handled by IsBrickVolumeBlockedTopAndBottom()
                if (!(cellPos.x >= alignedGridPos.x && cellPos.x < alignedGridPos.x + brickDim.extent.x &&
                      cellPos.z >= alignedGridPos.z && cellPos.z < alignedGridPos.z + brickDim.extent.z))
                {
                    return IsBrickBlocking(brick);
                }
                return true;
            }
        );
    }

    // Tries to transfer the specified brick to a new group.
    void TryTransferBrickGroup(Brick brick, BrickGroup newGroup)
    {
        // Don't transfer bricks if the group doesn't change
        if (brick.GetGroup() != newGroup)
        {
            // Don't transfer bricks into walls-only optimized group if their mesh doesn't support this group
            if (newGroup != BrickGroup_WallsOnly || IsBrickTypeOptimized(brick.GetId()))
            {
                Instance oldInstance;
                if (RemoveInstanceFromGroup(brick.groupId, brick.groupInstance, &oldInstance))
                {
                    const std::uint32_t newGroupId = CombineToBrickId(brick.GetId(), newGroup);
                    (void)AddInstanceToGroup(newGroupId, oldInstance.GetPos(), oldInstance.gridRotation, oldInstance.colorIndex);
                }
            }
        }
    }

    // Notifies the specified brick that its surrounding has changed,
    // allowing it to transfer to a different mesh group for better rendering performance.
    void OnBrickChanged(Brick brick, bool isChangedBrickBlocking, bool isChangeFromTopAndBottom)
    {
        if (const Instance* instance = GetBrickInstance(brick))
        {
            const BrickGroup group = brick.GetGroup();
            if (isChangedBrickBlocking)
            {
                // If brick is now blocked top and bottom, try to transfer brick to group for optimized rendering
                const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(brick.GetId(), instance->gridRotation);
                if (IsBrickVolumeBlockedTopAndBottom(instance->GetPos(), brickDim))
                {
                    // If brick is also blocked sidesways (left, right, front, back), don't render brick at all
                    if (IsBrickVolumeBlockedSideways(instance->GetPos(), brickDim))
                    {
                        // Transfer brick to group of invisible bricks, these are not rendered at all
                        TryTransferBrickGroup(brick, BrickGroup_Invisible);
                    }
                    else if (group == BrickGroup_Default)
                    {
                        // Transfer brick into walls-only optimized group, ignore transparent bricks or bricks already in that group
                        TryTransferBrickGroup(brick, BrickGroup_WallsOnly);
                    }
                }
            }
            else
            {
                if (group == BrickGroup_Invisible)
                {
                    // Brick is no longer fully blocked and must become visible again. Determine whether it was an opque or transparent brick
                    const bool isBrickTransparent = ColorPalette::Get().IsTransparent(instance->colorIndex);
                    if (!isBrickTransparent && !isChangeFromTopAndBottom && IsBrickTypeOptimized(brick.GetId()))
                    {
                        // Re-evalute if brick is still blocked top and bottom, to try putting it back into the group for optimized rendering
                        const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(brick.GetId(), instance->gridRotation);
                        if (IsBrickVolumeBlockedTopAndBottom(instance->GetPos(), brickDim))
                        {
                            TryTransferBrickGroup(brick, BrickGroup_WallsOnly);
                            return;
                        }
                    }
                    TryTransferBrickGroup(brick, isBrickTransparent ? BrickGroup_Transparent : BrickGroup_Default);
                }
                else if (group == BrickGroup_WallsOnly && isChangeFromTopAndBottom)
                {
                    // Brick is no longer fully blocked top and bottom, so switch back to default mesh
                    TryTransferBrickGroup(brick, BrickGroup_Default);
                }
            }
        }
    }

    // Notifes all bricks surrounding the specified changed brick that their surrounding has changed.
    void NotifyBrickChangeToNeighbors(const GridVec3i alignedGridPos, const GridVec3i& brickExtent, Brick changedBrick)
    {
        // First gather all brick cells that need to be notified to avoid running the blocking test multiple times for the same brick.
        // Use their cell coordinates instead of storing the brick handles as they might be invalidated in the next phase.
        std::set<Brick> visitedBricks[2];
        std::vector<GridVec3i> bricksToNotifyCells[2];

        const AABB3i topAndBottomAABB
        {
            alignedGridPos + GridVec3i{ 0, -1, 0 },
            alignedGridPos + brickExtent + GridVec3i{ 0, 2, 0}
        };

        grid.ForEachCell(
            alignedGridPos + GridVec3i{ -1 },
            brickExtent + GridVec3i{ 2 },
            [&visitedBricks, &bricksToNotifyCells, changedBrick, &topAndBottomAABB](Brick brick, const GridVec3i& gridPos) -> bool
            {
                // Don't notify the brick that just changed itself and also don't notify bricks more than once
                if (brick != changedBrick)
                {
                    // Notify bricks via two separate lists: (1) About changes above and blow a brick and (2) For all other neighbors.
                    const bool isChangeFromTopAndBottom = IsPointInsideAABB(gridPos, topAndBottomAABB);
                    const int listIndex = (isChangeFromTopAndBottom ? 0 : 1);
                    if (visitedBricks[listIndex].insert(brick).second)
                        bricksToNotifyCells[listIndex].push_back(gridPos);
                }
                return true;
            },
            /*iterateThroughLeavesOnly:*/ true
        );

        // Now notify the bricks that their surrounding has changed
        const bool isChangedBrickBlocking = IsBrickBlocking(changedBrick);

        for_range(listIndex, 2)
        {
            const bool isChangeFromTopAndBottom = (listIndex == 0);
            for (const GridVec3i& gridPos : bricksToNotifyCells[listIndex])
            {
                if (Brick brick = grid.GetLeaf(gridPos))
                    OnBrickChanged(brick, isChangedBrickBlocking, isChangeFromTopAndBottom);
            }
        }
    }

    // Returns true if the specified cell (position relative to brick, not to the grid) is excluded from the brick type.
    // This is used for a few select bricks that don't cover a full bounding box, like the roof tile.
    bool IsBrickCellExcluded(BrickId brickId, GridRotation brickRotation, const GridVec3i& cellPos)
    {
        const BrickInfo& brickInfo = BrickDictionary::Get().GetInfo(brickId);
        if (!brickInfo.excludedCells.empty())
        {
            auto IsCellExcluded = [&brickInfo](int x, int y, int z) -> bool
            {
                return (std::find(brickInfo.excludedCells.begin(), brickInfo.excludedCells.end(), GridVec3i{ x, y, z }) != brickInfo.excludedCells.end());
            };

            /* Only rotate input cell position since local cell positions are not offset */
            switch (brickRotation)
            {
                case GridRotation_0:
                    return IsCellExcluded(cellPos.x, cellPos.y, cellPos.z);
                case GridRotation_90:
                    return IsCellExcluded(cellPos.z, cellPos.y, brickInfo.extent.z - cellPos.x - 1);
                case GridRotation_180:
                    return IsCellExcluded(brickInfo.extent.x - cellPos.x - 1, cellPos.y, brickInfo.extent.z - cellPos.z - 1);
                case GridRotation_270:
                    return IsCellExcluded(brickInfo.extent.x - cellPos.z - 1, cellPos.y, cellPos.x);
                default:
                    break;
            }
        }
        return false;
    }

    bool InsertBrickIntoGrid(Brick brick, bool notifyNeighbors = false)
    {
        // Get brick dimension and apply rotation
        if (const Instance* instance = GetBrickInstance(brick))
        {
            const BrickId brickId = brick.GetId();
            const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(brickId, instance->gridRotation);

            // Insert brick entries into grid
            const GridVec3i alignedGridPos = instance->GetPos() + brickDim.offset;
            foreach_dimension(x, y, z, brickDim.extent)
            {
                const GridVec3i cellPos{ x, y, z };
                if (!IsBrickCellExcluded(brickId, instance->gridRotation, cellPos))
                    grid.InsertLeaf(alignedGridPos + cellPos, brick);
            }

            // Notify top and bottom grid entries that they are now blocked, so they can optimize their mesh if they are now fully blocked
            if (notifyNeighbors)
                NotifyBrickChangeToNeighbors(alignedGridPos, brickDim.extent, brick);

            #if KDTREE_DEBUG_LEVEL >= 2
            LLGL::Log::Printf("kd-Tree debug info: Nodes=%u, Leafs=%u\n", grid.NumNodes(), grid.NumLeafs());
            #endif

            return true;
        }
        return false;
    }

    void RemoveBrickFromGrid(BrickId brickId, const Instance& instance, bool notifyNeighbors = false)
    {
        // Get brick dimension and apply rotation
        const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(brickId, instance.gridRotation);

        // Remove brick entries from grid
        const GridVec3i alignedGridPos = instance.GetPos() + brickDim.offset;
        foreach_dimension(x, y, z, brickDim.extent)
        {
            const GridVec3i cellPos{ x, y, z };
            if (!IsBrickCellExcluded(brickId, instance.gridRotation, cellPos))
                grid.RemoveLeaf(alignedGridPos + GridVec3i{ x, y, z });
        }

        // Notify top and bottom grid entries that they are now unblocked, so they need to switch back to the full mesh
        if (notifyNeighbors)
            NotifyBrickChangeToNeighbors(alignedGridPos, brickDim.extent, Brick{});

        #if KDTREE_DEBUG_LEVEL >= 2
        LLGL::Log::Printf("kd-Tree debug info: Nodes=%u, Leafs=%u\n", grid.NumNodes(), grid.NumLeafs());
        #endif
    }

    void UpdateBrickInstanceInGrid(BrickId brickId, const Instance& instance, std::uint32_t groupInstance)
    {
        // Get brick dimension and apply rotation
        const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(brickId, instance.gridRotation);

        // Remove brick entries from grid
        const GridVec3i alignedGridPos = instance.GetPos() + brickDim.offset;
        foreach_dimension(x, y, z, brickDim.extent)
            grid.UpdateLeaf(alignedGridPos + GridVec3i{ x, y, z }, groupInstance);
    }

    void KeepColliderInsidePlayArea(Gs::Vector3f& colliderPoint, float colliderRadius) const
    {
        // First resolve hard-coded collision with floor plane
        if (colliderPoint.y < colliderRadius)
            colliderPoint.y = colliderRadius;

        // Also don't let the collider leave the play area too far, 200% the bounding volume
        const GridVec3i    aabbExtent = (KDTreeNode::rootAABB.bbMax - KDTreeNode::rootAABB.bbMin);
        const Gs::Vector3f aabbCenter = (KDTreeNode::rootAABB.bbMin + KDTreeNode::rootAABB.bbMax).Cast<float>() * 0.5f;
        const Gs::Vector3f colliderToCenterVector = colliderPoint - aabbCenter;

        const float outsideAreaTolerance = 50.0f;
        const float maxDist = Gs::Length(aabbExtent.Cast<float>()) * 0.5f + outsideAreaTolerance;

        const float colliderDistSqFromCenter = Gs::LengthSq(colliderToCenterVector);
        if (colliderDistSqFromCenter > maxDist*maxDist)
        {
            const float impactOutsideVolume = std::sqrt(colliderDistSqFromCenter) - maxDist;
            colliderPoint -= colliderToCenterVector.Normalized() * impactOutsideVolume;
        }
    }

};

// BrickPlay camera object.
struct Camera : RigidBody<Gs::Vector3f, Cfg::useCameraAcceleration>
{
    float           pitch       = 0.0f;
    float           yaw         = 0.0f;

    Gs::Matrix4f    wMatrix;
    Gs::Matrix4f    vMatrix;
    Gs::Matrix4f    vMatrixNoPosition;

    Camera()
    {
        SetMass(Cfg::cameraMass);
    }

    void Turn(float pitch, float yaw)
    {
        this->pitch = Gs::Clamp(this->pitch + pitch, -90.0f, +90.0f);
        this->yaw   = this->yaw + yaw;
    }

    void Move(const Gs::Vector3f& direction)
    {
        // Move camera into the direction, taking camera rotation into account
        Gs::Matrix3f rotation;
        Gs::RotateY(rotation, Gs::Deg2Rad(yaw));
        Gs::RotateX(rotation, Gs::Deg2Rad(pitch));
        AddForces(rotation * direction);
    }

    void ApplyForces(float dt, const BrickScene* collisionScene = nullptr)
    {
        // Apply velocity to position and resolve collisions
        const Gs::Vector3f velocityDelta = IntegrateForces(dt, Cfg::cameraDampingCeofficient);
        const float velocityDeltaLen = velocityDelta.Length();

        if (velocityDeltaLen > GS_EPSILON_F32)
        {
            // Resolve collisions in iterations by maximum velocity length
            if (collisionScene != nullptr)
            {
                const float travelStep = Cfg::cameraCollisionRadius * 0.9f;
                const Gs::Vector3f moveDir = velocityDelta.Normalized();

                float travelDist = 0.0f;
                while (travelDist + travelStep < velocityDeltaLen)
                {
                    position += moveDir * travelStep;
                    collisionScene->ResolveCollision(position, Cfg::cameraCollisionRadius);
                    travelDist += travelStep;
                }
                position += moveDir * (velocityDeltaLen - travelDist);
                collisionScene->ResolveCollision(position, Cfg::cameraCollisionRadius);
            }
            else
            {
                position += velocityDelta;
            }
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
};

// Semi-memento pattern to undo/redo changes.
struct History
{
    struct Event
    {
        BrickId     addedBrickId;
        Instance    addedInstance;
        BrickId     destroyedBrickId;
        Instance    destroyedInstance;
    };

    std::list<Event>            events;
    std::list<Event>::iterator  eventIter;

    History() :
        eventIter { events.end() }
    {
    }

    bool IsAtBegining() const
    {
        return eventIter == events.begin();
    }

    bool IsAtEnd() const
    {
        return eventIter == events.end();
    }

    void Register(Event&& event)
    {
        // If there are too many events in the queue, drop the oldest one
        if (events.size() >= Cfg::maxHistoryLength)
            events.pop_front();

        // When using 'undo' and then registering new events, all events after the current history iterator are dropped.
        events.erase(eventIter, events.end());
        events.push_back(std::move(event));
        eventIter = events.end();
    }

    const Event* Undo()
    {
        if (!IsAtBegining())
            return &(*(--eventIter));
        else
            return nullptr;
    }

    const Event* Redo()
    {
        if (!IsAtEnd())
            return &(*eventIter++);
        else
            return nullptr;
    }
};


#endif // /LLGL_EXAMPLE_BRICKPLAY_BRICKSCENE_H


