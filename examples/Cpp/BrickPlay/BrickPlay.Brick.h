/*
 * BrickPlay.Brick.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_EXAMPLE_BRICKPLAY_BRICK_H
#define LLGL_EXAMPLE_BRICKPLAY_BRICK_H


#include <Gauss/Vector3.h>
#include <string>
#include <cstdint>
#include <map>


// How bricks can be rotated in this example. Only multiples of 90 degrees is supported.
enum GridRotation : std::int32_t
{
    GridRotation_0 = 0,
    GridRotation_90,
    GridRotation_180,
    GridRotation_270,

    GridRotation_Num,
};

// Types of brick shapes.
enum BrickType : std::uint32_t
{
    //        WxDxH
    BrickType_1x1x3 = 0,
    BrickType_1x2x3,
    BrickType_2x2x3,
    BrickType_4x2x3,
    BrickType_1x1x1,
    BrickType_1x2x1,
    BrickType_2x2x1,
    BrickType_4x2x1,
    BrickType_1x1x1_Tile,
    BrickType_2x2x1_Tile,
    BrickType_2x2x3_Roof,
    BrickType_2x3x3_Roof,

    BrickType_Num,
};

// Meta data for each brick.
struct BrickInfo
{
    std::string     label;
    Gs::Vector3i    extent;
};

struct BrickMesh
{
    // Number of vertices to render.
    std::uint32_t numVertices = 0;

    // Zero-based index to the first vertex into the vertex buffer.
    std::uint32_t firstVertex = 0;
};

struct BrickDimensions
{
    Gs::Vector3i offset;
    Gs::Vector3i extent;
};

enum BrickGroup : std::uint32_t
{
    BrickGroup_Default = 0, // Default opaque brick rendering
    BrickGroup_WallsOnly,   // Bricks rendered with an optimzied version, when blocked from above and below
    BrickGroup_Transparent, // Render bricks with different shaders
    BrickGroup_Invisible,   // Not rendered at all, for bricks that are blocked from all sides

    BrickGroup_Num,
};

inline std::uint32_t CombineToBrickId(int type, int group)
{
    return static_cast<std::uint32_t>(type * BrickGroup_Num + group);
}

inline BrickType IdToBrickType(std::uint32_t groupId)
{
    return static_cast<BrickType>(groupId / BrickGroup_Num);
}

inline BrickGroup IdToBrickGroup(std::uint32_t groupId)
{
    return static_cast<BrickGroup>(groupId % BrickGroup_Num);
}

// Helper class to convert BrickType to meta data and vice-versa.
class BrickDictionary
{

    const std::map<BrickType, BrickInfo> typeToInfoMap
    {
        { BrickType_1x1x1,      BrickInfo{ "1x1x1",    { 1, 1, 1 } } },
        { BrickType_1x1x1_Tile, BrickInfo{ "1x1_tile", { 1, 1, 1 } } },
        { BrickType_1x1x3,      BrickInfo{ "1x1x3",    { 1, 3, 1 } } },
        { BrickType_1x2x1,      BrickInfo{ "1x2x1",    { 1, 1, 2 } } },
        { BrickType_1x2x3,      BrickInfo{ "1x2x3",    { 1, 3, 2 } } },
        { BrickType_2x2x1,      BrickInfo{ "2x2x1",    { 2, 1, 2 } } },
        { BrickType_2x2x1_Tile, BrickInfo{ "2x2_tile", { 2, 1, 2 } } },
        { BrickType_2x2x3,      BrickInfo{ "2x2x3",    { 2, 3, 2 } } },
        { BrickType_2x2x3_Roof, BrickInfo{ "2x2_roof", { 2, 3, 2 } } },
        { BrickType_2x3x3_Roof, BrickInfo{ "2x3_roof", { 2, 3, 3 } } },
        { BrickType_4x2x1,      BrickInfo{ "4x2x1",    { 4, 1, 2 } } },
        { BrickType_4x2x3,      BrickInfo{ "4x2x3",    { 4, 3, 2 } } },
    };

    BrickMesh brickMeshes[BrickType_Num * BrickGroup_Num];

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

    // Returns the specified BrickType as a string representation. Used by the scene file writer.
    const char* TypeToString(BrickType t) const
    {
        auto it = typeToInfoMap.find(t);
        return (it != typeToInfoMap.end() ? it->second.label.c_str() : "");
    }

    // Returns the BrickType that's mapped from the specified string. Used by the scene file reader.
    BrickType TypeFromString(const char* s) const
    {
        for (const std::pair<BrickType, BrickInfo>& it : typeToInfoMap)
        {
            if (it.second.label == s)
                return it.first;
        }
        return BrickType_Num;
    }

    Gs::Vector3i GetExtent(BrickType type) const
    {
        auto it = typeToInfoMap.find(type);
        return (it != typeToInfoMap.end() ? it->second.extent : Gs::Vector3i{});
    }

    BrickDimensions GetBrickDimensions(BrickType type, GridRotation rotation) const
    {
        BrickDimensions brickDim;

        brickDim.extent = GetExtent(type);
        if (rotation == GridRotation_90 || rotation == GridRotation_270)
            std::swap(brickDim.extent.x, brickDim.extent.z);

        brickDim.offset.x = ((rotation == GridRotation_90 || rotation == GridRotation_180) ? -(brickDim.extent.x + 1)/2 : -brickDim.extent.x/2);
        brickDim.offset.y = 0;
        brickDim.offset.z = ((rotation == GridRotation_180 || rotation == GridRotation_270) ? -(brickDim.extent.z + 1)/2 : -brickDim.extent.z/2);

        return brickDim;
    }

    BrickMesh& GetMesh(int brickType, int brickGroup = BrickGroup_Default)
    {
        return brickMeshes[CombineToBrickId(brickType, brickGroup)];
    }

    const BrickMesh& GetMesh(int brickType, int brickGroup = BrickGroup_Default) const
    {
        return brickMeshes[CombineToBrickId(brickType, brickGroup)];
    }

};

inline bool IsBrickTypeOptimized(BrickType type)
{
    return (BrickDictionary::Get().GetMesh(type, BrickGroup_WallsOnly).numVertices > 0);
}

struct Brick
{
    static constexpr std::uint32_t groupIdBits      = 8;
    static constexpr std::uint32_t groupIdInvalid   = (1u << groupIdBits) - 1;

    static_assert(
        (1u << groupIdBits) > BrickType_Num * BrickGroup_Num,
        "Not enough bits to encode `BrickType x BrickGroup` elements; Increase `Brick::groupIdBits` when adding elements to BrickType or BrickGroup!"
    );

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

    void SetType(BrickType type)
    {
        groupId = type * BrickGroup_Num + IdToBrickGroup(groupId);
    }

    BrickType GetType() const
    {
        return IdToBrickType(groupId);
    }

    void SetGroup(BrickGroup group)
    {
        groupId = IdToBrickType(groupId) * BrickGroup_Num + group;
    }

    BrickGroup GetGroup() const
    {
        return IdToBrickGroup(groupId);
    }

    // Returns true if this brick blocks the view from other bricks. False for invalid and transparent bricks.
    bool IsBlocking() const
    {
        return (groupId != groupIdInvalid && IdToBrickGroup(groupId) != BrickGroup_Transparent);
    }

    operator bool() const
    {
        return (groupId != groupIdInvalid);
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


#endif // /LLGL_EXAMPLE_BRICKPLAY_BRICK_H


