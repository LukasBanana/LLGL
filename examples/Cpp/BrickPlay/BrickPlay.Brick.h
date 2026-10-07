/*
 * BrickPlay.Brick.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_EXAMPLE_BRICKPLAY_BRICK_H
#define LLGL_EXAMPLE_BRICKPLAY_BRICK_H


#include <Gauss/Vector3.h>
#include <Gauss/Equals.h> // For Vector3 == operator
#include <GeometryUtils.h>
#include <string>
#include <cstdint>
#include <map>


#define INVALID_BRICK_ID UINT32_MAX

// 3D vector type for the Brick grid.
using GridVec3i = Gs::Vector3i;

// Axis-aligned bounding box for the Brick grid.
using AABB3i = AABB<GridVec3i>;


// How bricks can be rotated in this example. Only multiples of 90 degrees is supported.
enum GridRotation : std::uint8_t
{
    GridRotation_0 = 0,
    GridRotation_90,
    GridRotation_180,
    GridRotation_270,

    GridRotation_Num,
};

using BrickId = std::uint32_t;

enum BrickGroup : std::uint32_t
{
    BrickGroup_Default = 0, // Default opaque brick rendering
    BrickGroup_WallsOnly,   // Bricks rendered with an optimzied version, when blocked from above and below
    BrickGroup_Transparent, // Render bricks with different shaders
    BrickGroup_Invisible,   // Not rendered at all, for bricks that are blocked from all sides

    BrickGroup_Num,
};

struct BrickMesh
{
    // Number of vertices to render.
    std::uint32_t numVertices = 0;

    // Zero-based index to the first vertex into the vertex buffer.
    std::uint32_t firstVertex = 0;
};

struct InstanceGroup
{
    // Debug label constructed from BrickId and BrickGroup. Pre-allocate to avoid string concatenation during rendering (minor optimization).
    std::string     debugName;

    // Number of instances to render for this group
    std::uint32_t   numInstances      = 0;

    // Zero-based index to the first instance into the global `instances` container for this group. Instance 0 is reserved for the preview instance.
    std::uint32_t   firstInstance     = 0;

    // Number of instances allocated for this group in the global `instances` container. This must always be greater than or equal to `numInstances`.
    std::uint32_t   instanceCapacity  = 0;
};

// Meta data for each brick.
struct BrickInfo
{
    std::string             label;
    GridVec3i               extent;
    std::vector<GridVec3i>  excludedCells;
    std::uint32_t           numGroups                       = 0;
    BrickMesh               meshes[BrickGroup_Num];
    InstanceGroup           instanceGroups[BrickGroup_Num];

    // Expands upper bound of how many groups are used for this brick type.
    void ExpandGroups(BrickGroup newGroup)
    {
        numGroups = std::max(numGroups, static_cast<std::uint32_t>(newGroup) + 1);
    }
};

struct BrickDimensions
{
    GridVec3i offset;
    GridVec3i extent;
};

inline std::uint32_t CombineToBrickId(int type, int group)
{
    return static_cast<std::uint32_t>(type * BrickGroup_Num + group);
}

inline BrickId GroupIdToBrickId(std::uint32_t groupId)
{
    return static_cast<BrickId>(groupId / BrickGroup_Num);
}

inline BrickGroup GroupIdToBrickGroup(std::uint32_t groupId)
{
    return static_cast<BrickGroup>(groupId % BrickGroup_Num);
}

// Helper class to convert BrickId to meta data and vice-versa.
class BrickDictionary
{

    std::vector<BrickInfo>          brickInfos;
    std::map<std::string, BrickId>  labelToBrickId;

    BrickDictionary() = default;

public:

    BrickDictionary(const BrickDictionary&) = delete;
    BrickDictionary& operator = (const BrickDictionary&) = delete;

    // Returns this singleton instance.
    static BrickDictionary& Get()
    {
        static BrickDictionary self;
        return self;
    }

    // Returns the BrickInfo entry for the specified label.
    BrickInfo& FindOrInsert(std::string&& label)
    {
        auto it = labelToBrickId.find(label);
        if (it == labelToBrickId.end())
        {
            // Map label to brick ID
            labelToBrickId[label] = static_cast<BrickId>(brickInfos.size());

            // Add new entry
            brickInfos.push_back({});
            brickInfos.back().label = std::move(label);
            return brickInfos.back();
        }
        return brickInfos[it->second];
    }

    std::uint32_t NumBrickTypes() const
    {
        return static_cast<std::uint32_t>(brickInfos.size());
    }

    const BrickInfo& GetInfo(BrickId id) const
    {
        return brickInfos[id];
    }

    BrickInfo& GetInfo(BrickId id)
    {
        return brickInfos[id];
    }

    const BrickMesh& GetMesh(BrickId id, BrickGroup group = BrickGroup_Default) const
    {
        return GetInfo(id).meshes[group];
    }

    BrickMesh& GetMesh(BrickId id, BrickGroup group = BrickGroup_Default)
    {
        return GetInfo(id).meshes[group];
    }

    // Returns the specified BrickId as a string representation. Used by the scene file writer.
    const char* TypeToString(BrickId id) const
    {
        return (id < brickInfos.size() ? GetInfo(id).label.c_str() : "");
    }

    // Returns the BrickId that's mapped from the specified string. Used by the scene file reader.
    BrickId TypeFromString(const char* s) const
    {
        auto it = labelToBrickId.find(s);
        return (it != labelToBrickId.end() ? it->second : INVALID_BRICK_ID);
    }

    BrickId FindClosestTypeFromString(const char* s) const
    {
        for (const auto& it : labelToBrickId)
        {
            if (it.first.find(s) != std::string::npos)
                return it.second;
        }
        return INVALID_BRICK_ID;
    }

    BrickGroup GroupFromString(const char* s) const
    {
        if (::strcmp(s, "Default"    ) == 0) return BrickGroup_Default;
        if (::strcmp(s, "WallsOnly"  ) == 0) return BrickGroup_WallsOnly;
        if (::strcmp(s, "Transparent") == 0) return BrickGroup_Transparent;
        if (::strcmp(s, "Invisible"  ) == 0) return BrickGroup_Invisible;
        return BrickGroup_Num;
    }

    const char* GroupToString(BrickGroup g) const
    {
        switch (g)
        {
            case BrickGroup_Default :       return "Default";
            case BrickGroup_WallsOnly:      return "WallsOnly";
            case BrickGroup_Transparent:    return "Transparent";
            case BrickGroup_Invisible:      return "Invisible";
            default:                        return "";
        }
    }

    GridVec3i GetExtent(BrickId id) const
    {
        return (id < brickInfos.size() ? brickInfos[id].extent : GridVec3i{});
    }

    BrickDimensions GetBrickDimensions(BrickId id, GridRotation rotation) const
    {
        BrickDimensions brickDim;

        brickDim.extent = GetExtent(id);
        if (rotation == GridRotation_90 || rotation == GridRotation_270)
            std::swap(brickDim.extent.x, brickDim.extent.z);

        brickDim.offset.x = ((rotation == GridRotation_90 || rotation == GridRotation_180) ? -(brickDim.extent.x + 1)/2 : -brickDim.extent.x/2);
        brickDim.offset.y = 0;
        brickDim.offset.z = ((rotation == GridRotation_180 || rotation == GridRotation_270) ? -(brickDim.extent.z + 1)/2 : -brickDim.extent.z/2);

        return brickDim;
    }

    BrickId FindBrickIdForDimension(const GridVec3i& extent) const
    {
        // Try to find brick with the specified dimensions
        for (BrickId id = 0; id < brickInfos.size(); ++id)
        {
            if (brickInfos[id].extent == extent)
                return id;
        }
        return INVALID_BRICK_ID; // No such brick found
    }

};

// Returns true if the specified brick ID has a mesh that only renders its walls.
inline bool IsBrickTypeOptimized(BrickId id)
{
    return (BrickDictionary::Get().GetInfo(id).meshes[BrickGroup_WallsOnly].numVertices > 0);
}

// Brick handle to reference an instance in the grid.
// These change as bricks are being transferred between groups for optimized rendering.
// It's important to keep this in mind especially inside 
struct Brick
{
    static constexpr std::uint32_t groupIdBits      = 8;
    static constexpr std::uint32_t groupIdInvalid   = (1u << groupIdBits) - 1;

    /*static_assert(
        (1u << groupIdBits) > BrickType_Num * BrickGroup_Num,
        "Not enough bits to encode `BrickType x BrickGroup` elements; Increase `Brick::groupIdBits` when adding elements to BrickType or BrickGroup!"
    );*/

    Brick() :
        groupId       { groupIdInvalid },
        groupInstance { 0              }
    {
    }

    Brick(std::uint32_t groupId, std::uint32_t groupInstance) :
        groupId       { groupId       },
        groupInstance { groupInstance }
    {
    }

    // Combined brick type and group.
    std::uint32_t   groupId         : groupIdBits;

    // Index into the group's instance. Use `group.firstInstance + groupInstance` to get index into global `instances` container.
    std::uint32_t   groupInstance   : (32 - groupIdBits);

    void SetId(BrickId id)
    {
        groupId = id * BrickGroup_Num + GroupIdToBrickGroup(groupId);
    }

    BrickId GetId() const
    {
        return GroupIdToBrickId(groupId);
    }

    void SetGroup(BrickGroup group)
    {
        groupId = GroupIdToBrickId(groupId) * BrickGroup_Num + group;
    }

    BrickGroup GetGroup() const
    {
        return GroupIdToBrickGroup(groupId);
    }

    operator bool() const
    {
        return (groupId != groupIdInvalid);
    }

    bool operator == (const Brick& rhs) const
    {
        return (groupId == rhs.groupId && groupInstance == rhs.groupInstance);
    }

    bool operator != (const Brick& rhs) const
    {
        return !(*this == rhs);
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


#endif // /LLGL_EXAMPLE_BRICKPLAY_BRICK_H


