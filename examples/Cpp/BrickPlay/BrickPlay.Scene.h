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
#include <ExampleBase.h>
#include <algorithm>
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Container/SmallVector.h>
#include <set>


// Owns the scene with its bricks, instance buffer, and the grid (kd-Tree) representation.
class BrickScene
{

public:

    // Function callback to notify the collider about a contact with a brick.
    using OnCollisionContact = std::function<void(Brick, const Gs::Vector3f& contactPoint, float impact)>;

    // Brick instance structure. Stores the position, rotation, and color.
    struct Instance
    {
        Gs::Vector3i        gridPos;
        GridRotation        gridRotation;
        LLGL::ColorRGBAub   color;
    };

    struct InstanceGroup
    {
        // Number of instances to render for this group
        std::uint32_t numInstances      = 0;

        // Zero-based index to the first instance into the global `instances` container for this group. Instance 0 is reserved for the preview instance.
        std::uint32_t firstInstance     = 0;

        // Number of instances allocated for this group in the global `instances` container. This must always be greater than or equal to `numInstances`.
        std::uint32_t instanceCapacity  = 0;
    };

    struct RayIntersection
    {
        Gs::Vector3f    point;
        Gs::Vector3i    normal;
        Brick           brick;
    };

private:

    #define foreach_dimension(X, Y, Z, VEC) \
        for_range(Z, (VEC).z) for_range(Y, (VEC).y) for_range(X, (VEC).x)

    LLGL::RenderSystem&             renderSystem;

    LLGL::Buffer*                   instanceBuffer              = nullptr;
    std::uint64_t                   instanceBufferSize          = 0;

    std::vector<Instance>           instances;
    InstanceGroup                   instanceGroups[BrickType_Num * BrickGroup_Num];
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

    const InstanceGroup& GetInstanceGroupById(std::uint32_t groupId) const
    {
        return instanceGroups[groupId];
    }

    const InstanceGroup& GetInstanceGroup(BrickType type, BrickGroup group = BrickGroup_Default) const
    {
        return instanceGroups[CombineToBrickId(type, group)];
    }

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

    bool NewBrick(BrickType type, const Gs::Vector3i& gridPos, GridRotation gridRotation = GridRotation_0, LLGL::ColorRGBAub color = {})
    {
        if (!BrickFitsInGrid(gridPos, type, gridRotation))
            return false;

        hasUnsavedChanges = true;

        const bool isTransparent = (color.a < 255);
        const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(type, gridRotation);

        const BrickGroup group = [this, &isTransparent, &brickDim, &gridPos, type]() -> BrickGroup
        {
            if (IsBrickVolumeBlockedTopAndBottom(gridPos, brickDim))
            {
                if (IsBrickVolumeBlockedSideways(gridPos, brickDim))
                    return BrickGroup_Invisible;
                else if (!isTransparent && IsBrickTypeOptimized(type))
                    return BrickGroup_WallsOnly;
            }
            if (isTransparent)
                return BrickGroup_Transparent;
            return BrickGroup_Default;
        }();

        const std::uint32_t groupId = CombineToBrickId(type, group);

        // Insert brick without notification first, so its instance is fully initialized before neighbors can rearrange the groups
        constexpr bool notifyNeighbors = true;
        return AddInstanceToGroup(groupId, gridPos, gridRotation, color, notifyNeighbors);
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
        for (std::uint32_t groupId = 0; groupId < BrickType_Num * BrickGroup_Num; ++groupId)
        {
            InstanceGroup& group = instanceGroups[groupId];
            group.numInstances      = 0;
            group.firstInstance     = 0;
            group.instanceCapacity  = 0;
        }

        // Destroy the entire kd-Tree
        grid.Reset();

        hasUnsavedChanges = true;
    }

    bool BrickFitsInGrid(const Gs::Vector3i& gridPos, BrickType type, GridRotation rotation) const
    {
        // Get brick dimension and apply rotation
        const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(type, rotation);

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

    bool PickScene(const Ray3f& ray, RayIntersection& outContact)
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

    void ResolveCollision(Gs::Vector3f& colliderPoint, float colliderRadius, const OnCollisionContact& onCollision = nullptr) const
    {
        KeepColliderInsidePlayArea(colliderPoint, colliderRadius);

        // Find all potential bricks to collide with
        std::set<Brick> colliderBricks = grid.GatherPotentialColliderBricks(colliderPoint, colliderRadius);

        // Resolve collisions against AABB faces
        auto GetBrickAABB = [this](Brick brick, const Instance& instance, AABB3i& aabb)
        {
            const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(brick.GetType(), instance.gridRotation);

            aabb.bbMin = instance.gridPos + brickDim.offset;
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

private:

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
        return (brick ? instanceGroups[brick.groupId].firstInstance + brick.groupInstance : static_cast<std::size_t>(-1));
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
    bool AddInstanceToGroup(std::uint32_t groupId, Gs::Vector3i gridPos, GridRotation gridRotation = GridRotation_0, LLGL::ColorRGBAub color = {}, bool notifyNeighbors = false)
    {
        Brick newBrick{ groupId, instanceGroups[groupId].numInstances };

        if (Instance* newInstance = AllocateInstancesInGroup(groupId))
        {
            newInstance->gridPos        = gridPos;
            newInstance->gridRotation   = gridRotation;
            newInstance->color          = color;

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

    // Returns true if the specified brick is blocking the view of other bricks. False for transparent bricks.
    bool IsBrickBlocking(Brick brick) const
    {
        if (const Instance* brickInstance = GetBrickInstance(brick))
        {
            const bool isTransparent = (brickInstance->color.a < 255);
            return !isTransparent;
        }
        return false;
    }

    bool IsGridCellBlocked(const Gs::Vector3i& gridPos) const
    {
        return IsBrickBlocking(grid.GetLeaf(gridPos));
    }

    bool IsBrickVolumeBlockedTopAndBottom(const Gs::Vector3i& gridPos, const BrickDimensions& brickDim)
    {
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
                    if (!IsGridCellBlocked(alignedGridPos + Gs::Vector3i{ x, y, z }))
                        return false;
                }
            }
        }
        return true;
    }

    bool IsBrickVolumeBlockedSideways(const Gs::Vector3i& gridPos, const BrickDimensions& brickDim)
    {
        const Gs::Vector3i alignedGridPos = gridPos + brickDim.offset;
        for_subrange(y, -1, brickDim.extent.y + 1)
        {
            // If current grid position is under the floor, it's also considered blocked
            if (alignedGridPos.y + y < KDTreeNode::rootAABB.bbMin.y)
                continue;

            for_subrange(x, -1, brickDim.extent.x + 1)
            {
                if (!IsGridCellBlocked(alignedGridPos + Gs::Vector3i{ x, y, -1 }))
                    return false;
                if (!IsGridCellBlocked(alignedGridPos + Gs::Vector3i{ x, y, brickDim.extent.z }))
                    return false;
            }
            for_range(z, brickDim.extent.z)
            {
                if (!IsGridCellBlocked(alignedGridPos + Gs::Vector3i{ -1, y, z }))
                    return false;
                if (!IsGridCellBlocked(alignedGridPos + Gs::Vector3i{ brickDim.extent.x, y, z }))
                    return false;
            }
        }
        return true;
    }

    // Tries to transfer the specified brick to a new group.
    void TryTransferBrickGroup(Brick brick, BrickGroup newGroup)
    {
        // Don't transfer bricks if the group doesn't change
        if (brick.GetGroup() != newGroup)
        {
            // Don't transfer bricks into walls-only optimized group if their mesh doesn't support this group
            if (newGroup != BrickGroup_WallsOnly || IsBrickTypeOptimized(brick.GetType()))
            {
                Instance oldInstance;
                if (RemoveInstanceFromGroup(brick.groupId, brick.groupInstance, &oldInstance))
                {
                    const std::uint32_t newGroupId = CombineToBrickId(brick.GetType(), newGroup);
                    (void)AddInstanceToGroup(newGroupId, oldInstance.gridPos, oldInstance.gridRotation, oldInstance.color);
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
                const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(brick.GetType(), instance->gridRotation);
                if (IsBrickVolumeBlockedTopAndBottom(instance->gridPos, brickDim))
                {
                    // If brick is also blocked sidesways (left, right, front, back), don't render brick at all
                    if (IsBrickVolumeBlockedSideways(instance->gridPos, brickDim))
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
                    const bool isBrickTransparent = (instance->color.a < 255);
                    if (!isBrickTransparent && !isChangeFromTopAndBottom)
                    {
                        // Re-evalute if brick is still blocked top and bottom, to try putting it back into the group for optimized rendering
                        const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(brick.GetType(), instance->gridRotation);
                        if (IsBrickVolumeBlockedTopAndBottom(instance->gridPos, brickDim))
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
    void NotifyBrickChangeToNeighbors(const Gs::Vector3i alignedGridPos, const Gs::Vector3i& brickExtent, Brick changedBrick)
    {
        // First gather all brick cells that need to be notified to avoid running the blocking test multiple times for the same brick.
        // Use their cell coordinates instead of storing the brick handles as they might be invalidated in the next phase.
        std::set<Brick> visitedBricks[2];
        std::vector<Gs::Vector3i> bricksToNotifyCells[2];

        const AABB3i topAndBottomAABB
        {
            alignedGridPos + Gs::Vector3i{ 0, -1, 0 },
            alignedGridPos + brickExtent + Gs::Vector3i{ 0, 2, 0}
        };

        grid.ForEachLeaf(
            alignedGridPos + Gs::Vector3i{ -1 },
            brickExtent + Gs::Vector3i{ 2 },
            [&visitedBricks, &bricksToNotifyCells, changedBrick, &topAndBottomAABB](Brick brick, const Gs::Vector3i& gridPos)
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
            }
        );

        // Now notify the bricks that their surrounding has changed
        const bool isChangedBrickBlocking = IsBrickBlocking(changedBrick);

        for_range(listIndex, 2)
        {
            const bool isChangeFromTopAndBottom = (listIndex == 0);
            for (const Gs::Vector3i& gridPos : bricksToNotifyCells[listIndex])
            {
                if (Brick brick = grid.GetLeaf(gridPos))
                    OnBrickChanged(brick, isChangedBrickBlocking, isChangeFromTopAndBottom);
            }
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
            const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(brickType, instance->gridRotation);

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
                NotifyBrickChangeToNeighbors(alignedGridPos, brickDim.extent, brick);

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
        const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(type, instance.gridRotation);

        // Remove brick entries from grid
        const Gs::Vector3i alignedGridPos = instance.gridPos + brickDim.offset;
        foreach_dimension(x, y, z, brickDim.extent)
        {
            const Gs::Vector3i cellPos{ x, y, z };
            if (!IsBrickCellExcluded(type, instance.gridRotation, cellPos))
                grid.RemoveLeaf(alignedGridPos + Gs::Vector3i{ x, y, z });
        }

        // Notify top and bottom grid entries that they are now unblocked, so they need to switch back to the full mesh
        if (notifyNeighbors)
            NotifyBrickChangeToNeighbors(alignedGridPos, brickDim.extent, Brick{});

        #if DEBUG_KDTREE
        LLGL::Log::Printf("kd-Tree debug info: Nodes=%u, Leafs=%u\n", grid.NumNodes(), grid.NumLeafs());
        #endif
    }

    void UpdateBrickInstanceInGrid(BrickType type, const Instance& instance, std::uint32_t groupInstance)
    {
        // Get brick dimension and apply rotation
        const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(type, instance.gridRotation);

        // Remove brick entries from grid
        const Gs::Vector3i alignedGridPos = instance.gridPos + brickDim.offset;
        foreach_dimension(x, y, z, brickDim.extent)
            grid.UpdateLeaf(alignedGridPos + Gs::Vector3i{ x, y, z }, groupInstance);
    }

    void KeepColliderInsidePlayArea(Gs::Vector3f& colliderPoint, float colliderRadius) const
    {
        // First resolve hard-coded collision with floor plane
        if (colliderPoint.y < colliderRadius)
            colliderPoint.y = colliderRadius;

        // Also don't let the collider leave the play area too far, 200% the bounding volume
        const Gs::Vector3i aabbExtent = (KDTreeNode::rootAABB.bbMax - KDTreeNode::rootAABB.bbMin);
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


#endif // /LLGL_EXAMPLE_BRICKPLAY_BRICKSCENE_H


