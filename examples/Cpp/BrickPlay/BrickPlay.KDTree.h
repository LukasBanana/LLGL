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


// Tree hierarchy for the grid of brick cells. A standard 4x2x3 brick for example takes up 24 cells in the grid.
// There are few exceptions like the roof tiles, which have one row of cells that are skipped, to allow users to place other bricks more closely above them.
class KDTreeNode
{

    // Function callback to notify the collider about a contact with a brick.
    using OnCollisionContact = std::function<void(Brick, const Gs::Vector3f& contactPoint)>;

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

    void FindPotentialColliderBricks(const Gs::Vector3f& colliderPoint, float colliderRadius2, const AABB3i& aabb, std::set<Brick>& outColliderBricks) const
    {
        if (GetSqDistanceToAABB(colliderPoint, aabb) <= colliderRadius2*colliderRadius2)
        {
            if (HasChildren())
            {
                child[0]->FindPotentialColliderBricks(colliderPoint, colliderRadius2, GetChild0AABB(aabb), outColliderBricks);
                child[1]->FindPotentialColliderBricks(colliderPoint, colliderRadius2, GetChild1AABB(aabb), outColliderBricks);
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

public:

    static const Gs::Vector3f   gridScale;

    static AABB3i               rootAABB;
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

    float GetSqDistanceToAABB(const Gs::Vector3f& point, const AABB3i& aabb) const
    {
        const Gs::Vector3f closestPointOnAABB
        {
            Gs::Clamp(point.x, static_cast<float>(aabb.bbMin.x), static_cast<float>(aabb.bbMax.x)),
            Gs::Clamp(point.y, static_cast<float>(aabb.bbMin.y), static_cast<float>(aabb.bbMax.y)),
            Gs::Clamp(point.z, static_cast<float>(aabb.bbMin.z), static_cast<float>(aabb.bbMax.z)),
        };
        return Gs::DistanceSq(closestPointOnAABB, point);
    }

    void ResolveCollision(Gs::Vector3f& colliderPoint, float colliderRadius, const OnCollisionContact& onCollision = nullptr) const
    {
        // Find all potential bricks to collide with
        std::set<Brick> colliderBricks;
        FindPotentialColliderBricks(colliderPoint, colliderRadius*2.0f, KDTreeNode::rootAABB, colliderBricks);

        // Resolve collisions against AABB faces


        // Now resolve collisions against AABB edges and corners



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


#endif // /LLGL_EXAMPLE_BRICKPLAY_KDTREE_H


