/*
 * BrickPlay.KDTree.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_EXAMPLE_BRICKPLAY_KDTREE_H
#define LLGL_EXAMPLE_BRICKPLAY_KDTREE_H


#include "BrickPlay.Brick.h"
#include <GeometryUtils.h>
#include <memory>
#include <set>
#include <functional>


 // Enable log output to debug kd-Tree construction
#define DEBUG_KDTREE            0

 // Subdivide kd-Tree uniformly. This is only used for performance testing.
#define UNIFORM_TREE_SUBDIVIDE  0


inline Gs::Vector3f ClosestPointOnAABB(const Gs::Vector3f& point, const AABB3i& aabb);

inline bool IsPointInsideAABB(const Gs::Vector3i& point, const AABB3i& aabb);


// Tree hierarchy for the grid of brick cells. A standard 4x2x3 brick for example takes up 24 cells in the grid.
// There are few exceptions like the roof tiles, which have one row of cells that are skipped, to allow users to place other bricks more closely above them.
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

public:

    using LeafIterator = std::function<void(Brick brick, const Gs::Vector3i& gridPos)>;

    static const Gs::Vector3f   gridScale;

    static AABB3i               rootAABB;
    static int                  numAABBTests;

    static bool IsInsideBoundingVolume(const Gs::Vector3i& gridPos)
    {
        return IsPointInsideAABB(gridPos, KDTreeNode::rootAABB);
    }

    static void InitBoundingVolume(const LLGL::ArrayView<int> boundingVolume)
    {
        LLGL_VERIFY(boundingVolume.size() == 3);
        KDTreeNode::rootAABB = AABB3i
        {
            Gs::Vector3i{ -boundingVolume[0]/2,                  0, -boundingVolume[2]/2 },
            Gs::Vector3i{ +boundingVolume[0]/2, +boundingVolume[1], +boundingVolume[2]/2 }
        };
    }

public:

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

    void ForEachLeaf(const Gs::Vector3i& gridPos, const Gs::Vector3i& extent, const LeafIterator& iterator) const
    {
        // Clamp AABB to root AABB
        LLGL_VERIFY(iterator != nullptr);
        if (extent.x > 0 && extent.y > 0 && extent.z > 0)
        {
            AABB3i gridArea;
            {
                // Clamp area into kd-Tree AABB. If coordinate for bbMin is equal to bbMax, its extent will be 0
                gridArea.bbMin.x = Gs::Clamp(gridPos.x, KDTreeNode::rootAABB.bbMin.x, KDTreeNode::rootAABB.bbMax.x);
                gridArea.bbMin.y = Gs::Clamp(gridPos.y, KDTreeNode::rootAABB.bbMin.y, KDTreeNode::rootAABB.bbMax.y);
                gridArea.bbMin.z = Gs::Clamp(gridPos.z, KDTreeNode::rootAABB.bbMin.z, KDTreeNode::rootAABB.bbMax.z);

                gridArea.bbMax.x = std::min(gridArea.bbMin.x + extent.x, KDTreeNode::rootAABB.bbMax.x);
                gridArea.bbMax.y = std::min(gridArea.bbMin.y + extent.y, KDTreeNode::rootAABB.bbMax.y);
                gridArea.bbMax.z = std::min(gridArea.bbMin.z + extent.z, KDTreeNode::rootAABB.bbMax.z);
            }
            ForEachLeafInside(gridArea.bbMin, gridArea.bbMax - gridArea.bbMin, KDTreeNode::rootAABB, iterator);
        }
    }

    Brick IntersectWithLeaf(const Ray3f& ray, AABBIntersection* outIntersection = nullptr) const
    {
        return FindIntersectionWithLeaf(ray, KDTreeNode::rootAABB, outIntersection);
    }

    float GetSqDistanceToAABB(const Gs::Vector3f& point, const AABB3i& aabb) const
    {
        return Gs::DistanceSq(point, ClosestPointOnAABB(point, aabb));
    }

    std::set<Brick> GatherPotentialColliderBricks(const Gs::Vector3f& colliderPoint, float colliderRadius) const
    {
        std::set<Brick> colliderBricks;
        const float colliderRadius2 = colliderRadius*2.0f;
        FindPotentialColliderBricks(colliderPoint, colliderRadius2*colliderRadius2, KDTreeNode::rootAABB, colliderBricks);
        return colliderBricks;
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

private:

    AABB3i GetChild0AABB(AABB3i aabbParent) const
    {
        aabbParent.bbMax[axis] = aabbParent.bbMin[axis] + static_cast<std::int32_t>(distance);
        return aabbParent;
    }

    AABB3i GetChild1AABB(AABB3i aabbParent) const
    {
        aabbParent.bbMin[axis] += static_cast<std::int32_t>(distance);
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
                child[0]->SubdivideAndSetLeaf(gridPos, GetChild0AABB(aabb), brick);
            else
                child[1]->SubdivideAndSetLeaf(gridPos, GetChild1AABB(aabb), brick);
        }
    }

    void FindAndInsertLeaf(const Gs::Vector3i& gridPos, const AABB3i& aabb, Brick brick)
    {
        if (HasChildren())
        {
            // Shrink root AABB until we reach a leaf node
            if (gridPos[axis] < aabb.bbMin[axis] + static_cast<std::int32_t>(distance))
                child[0]->FindAndInsertLeaf(gridPos, GetChild0AABB(aabb), brick);
            else
                child[1]->FindAndInsertLeaf(gridPos, GetChild1AABB(aabb), brick);
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
                child[0]->FindAndUpdateLeaf(gridPos, GetChild0AABB(aabb), newGroupInstance);
            else
                child[1]->FindAndUpdateLeaf(gridPos, GetChild1AABB(aabb), newGroupInstance);
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
                if (child[0]->FindAndRemoveLeaf(gridPos, GetChild0AABB(aabb)))
                {
                    TryDropChildren();
                    return true;
                }
            }
            else
            {
                if (child[1]->FindAndRemoveLeaf(gridPos, GetChild1AABB(aabb)))
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
                return child[0]->FindLeaf(gridPos, GetChild0AABB(aabb));
            else
                return child[1]->FindLeaf(gridPos, GetChild1AABB(aabb));
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

    void ForEachLeafInside(const Gs::Vector3i& gridPos, const Gs::Vector3i& extent, const AABB3i& aabb, LeafIterator iterator) const
    {
        if (HasChildren())
        {
            const std::int32_t split = aabb.bbMin[axis] + static_cast<std::int32_t>(distance);
            if (gridPos[axis] < split)
            {
                // Split grid position plus extent range between child0 and child1
                Gs::Vector3i child0Extent = extent;
                child0Extent[axis] = std::min(extent[axis], split - gridPos[axis]);
                child[0]->ForEachLeafInside(gridPos, child0Extent, GetChild0AABB(aabb), iterator);

                if (gridPos[axis] + extent[axis] > split)
                {
                    Gs::Vector3i child1GridPos = gridPos;
                    child1GridPos[axis] = split;

                    Gs::Vector3i child1Extent = extent;
                    child1Extent[axis] -= child0Extent[axis];

                    child[1]->ForEachLeafInside(child1GridPos, child1Extent, GetChild1AABB(aabb), iterator);
                }
            }
            else
            {
                // Pass parameters to child1. No need to clamp the extent, this already happend in the above branch from a parent node.
                child[1]->ForEachLeafInside(gridPos, extent, GetChild1AABB(aabb), iterator);
            }
        }
        else if (aabb.bbMax == aabb.bbMin + Gs::Vector3i{ 1 })
        {
            // We found the leaf node
            iterator(this->brick, gridPos);
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

                childBrick[0] = child[0]->FindIntersectionWithLeaf(ray, GetChild0AABB(aabb), &(childIntersect[0]));
                childBrick[1] = child[1]->FindIntersectionWithLeaf(ray, GetChild1AABB(aabb), &(childIntersect[1]));

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

    void FindPotentialColliderBricks(const Gs::Vector3f& colliderPoint, float colliderRadiusSq2, const AABB3i& aabb, std::set<Brick>& outColliderBricks) const
    {
        if (GetSqDistanceToAABB(colliderPoint, aabb) <= colliderRadiusSq2)
        {
            if (HasChildren())
            {
                child[0]->FindPotentialColliderBricks(colliderPoint, colliderRadiusSq2, GetChild0AABB(aabb), outColliderBricks);
                child[1]->FindPotentialColliderBricks(colliderPoint, colliderRadiusSq2, GetChild1AABB(aabb), outColliderBricks);
            }
            else if (aabb.bbMax == aabb.bbMin + Gs::Vector3i{ 1 } && this->brick)
            {
                outColliderBricks.insert(this->brick);
            }
        }
    }

    unsigned NumLeafsFromRoot(const AABB3i& aabb) const
    {
        if (HasChildren())
            return child[0]->NumLeafsFromRoot(GetChild0AABB(aabb)) + child[1]->NumLeafsFromRoot(GetChild1AABB(aabb));
        else if (aabb.bbMax == aabb.bbMin + Gs::Vector3i{ 1 })
            return 1;
        else
            return 0;
    }

};


inline Gs::Vector3f ClosestPointOnAABB(const Gs::Vector3f& point, const AABB3i& aabb)
{
    Gs::Vector3f closestPoint;
    for (int axis = 0; axis < 3; ++axis)
    {
        closestPoint[axis] = Gs::Clamp(
            point[axis],
            static_cast<float>(aabb.bbMin[axis]) * KDTreeNode::gridScale[axis],
            static_cast<float>(aabb.bbMax[axis]) * KDTreeNode::gridScale[axis]
        );
    }
    return closestPoint;
}

inline bool IsPointInsideAABB(const Gs::Vector3i& point, const AABB3i& aabb)
{
    return
    (
        point.x >=  aabb.bbMin.x &&
        point.y >=  aabb.bbMin.y &&
        point.z >=  aabb.bbMin.z &&
        point.x <   aabb.bbMax.x &&
        point.y <   aabb.bbMax.y &&
        point.z <   aabb.bbMax.z
    );
}


#endif // /LLGL_EXAMPLE_BRICKPLAY_KDTREE_H


