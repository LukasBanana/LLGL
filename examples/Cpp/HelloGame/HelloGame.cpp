/*
 * HelloGame.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>
#include <LLGL/Utils/ForRange.h>
#include "FileUtils.h"
#include <algorithm>
#include <limits.h>
#include <sstream>
#include <stdio.h>
#include <stdint.h>
#include <cmath>
#include <random>

/*
Make PRIX64 macro visible inside <inttypes.h>; Required on some hosts that predate C++11.
See https://www.gnu.org/software/gnulib/manual/html_node/inttypes_002eh.html
*/
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>


// Enables cheats by allowing page up/down to select next or previous level.
#define ENABLE_CHEATS       LLGL_DEBUG

// Enables verbose logs about resource creation such as the instance buffer size.
#define ENABLE_VERBOSE_LOGS LLGL_DEBUG


/*
This is a simple puzzle game to showcase a small example on how to create a game with LLGL.
Use the arrow keys to move the player cube and try to activate all tiles without locking yourself in.
If cheats are enabled, use page up/down to switch to different levels on demand.
All levels are described in the "HelloGame.levels.txt" text file.
*/
class Example_HelloGame : public ExampleBase
{

    static constexpr float  levelTransitionSpeed    = 0.5f; // in seconds
    static constexpr float  levelDoneSpeed          = 1.0f; // in seconds
    static constexpr float  wallPosY                = 2.0f;
    static constexpr int    inputStackSize          = 4;
    static constexpr float  playerColor[3]          = { 0.6f, 0.7f, 1.0f };
    static constexpr float  treeColorGradient[2][3] = { { 0.1f, 0.5f, 0.1f }, { 0.2f, 0.8f, 0.3f } };
    static constexpr float  treeAnimSpeed           = 8.0f; // in seconds
    static constexpr float  treeAnimRadius          = 0.2f;
    static constexpr int    shadowMapSize           = 512;
    static constexpr float  timeOfDayChangeSpeed    = 2.0f; // in seconds
    static constexpr float  pointerMoveScale        = 15.0f; // percentage of largest screen dimension to move the pointer for player movemnt
    static constexpr float  jitterDelay             = 1.0f;
    static constexpr float  jitterDuration          = 1.0f;
    static constexpr float  jitterMaxAngle          = 15.0f;

    static constexpr float  playerMoveSpeed         = 0.25f; // in seconds
    static constexpr float  playerFallAcceleration  = 2.0f; // in units per seconds
    static constexpr float  playerJumpWait          = 3.0f; // in seconds
    static constexpr float  playerJumpDuration      = 1.0f; // in seconds
    static constexpr float  playerJumpHeight        = 0.7f;
    static constexpr int    playerJumpBounces       = 3;
    static constexpr float  playerFadeAwayDuration  = 3.0f;
    static constexpr float  playerDescendRotations  = 4.0f;
    static constexpr float  playerDescendHeight     = 6.0f;
    static constexpr float  playerDescendDuration   = 1.5f;
    static constexpr float  playerLeanBackSpeed     = 0.7f;
    static constexpr float  playerExplodePow        = 0.5f;
    static constexpr float  playerExplodeScale      = 15.0f;

    struct TimeOfDay
    {
        Gs::Vector3f    lightDir;
        LLGL::ColorRGBf lightColor;
        float           ambientIntensity;

        static TimeOfDay Interpolate(const TimeOfDay& a, const TimeOfDay& b, float t)
        {
            TimeOfDay result;
            result.lightDir         = Gs::Lerp(a.lightDir, b.lightDir, t).Normalized();
            result.lightColor       = Gs::Lerp(a.lightColor, b.lightColor, t);
            result.ambientIntensity = Gs::Lerp(a.ambientIntensity, b.ambientIntensity, t);
            return result;
        }
    };

    const TimeOfDay         timeOfDayMorning        = { Gs::Vector3f{ +0.15f, -1.0f, 0.25f }, LLGL::ColorRGBf{ 1.0f, 1.0f, 1.0f }, 0.6f };
    const TimeOfDay         timeOfDayEvening        = { Gs::Vector3f{ -0.75f, -0.7f, 2.25f }, LLGL::ColorRGBf{ 0.6f, 0.5f, 0.2f }, 0.2f };

    LLGL::PipelineLayout*   scenePSOLayout[2]       = {};
    LLGL::PipelineState*    scenePSO[2]             = {};
    ShaderPipeline          sceneShaders;

    LLGL::PipelineLayout*   groundPSOLayout         = nullptr;
    LLGL::PipelineState*    groundPSO               = nullptr;
    ShaderPipeline          groundShaders;

    LLGL::Buffer*           cbufferScene            = nullptr;
    LLGL::Buffer*           vertexBuffer            = nullptr;

    LLGL::Texture*          groundColorMap          = nullptr;
    LLGL::Sampler*          groundColorMapSampler   = nullptr;

    LLGL::Texture*          shadowMap               = nullptr;
    LLGL::Sampler*          shadowMapSampler        = nullptr;
    LLGL::RenderTarget*     shadowMapTarget         = nullptr;

    LLGL::VertexFormat      vertexFormat;

    TriangleMesh            mdlPlayer;
    TriangleMesh            mdlBlock;
    TriangleMesh            mdlTree;
    TriangleMesh            mdlGround;

    struct RandomNumberGenerator
    {
        RandomNumberGenerator() :
            engine{ device() }
        {
        }

        std::random_device                      device;
        std::mt19937                            engine;
        std::uniform_real_distribution<float>   dist;

        float Next()
        {
            return dist(engine);
        }
    }
    rng;

    using InstanceMatrixType = Gs::AffineMatrix4f;

    struct alignas(16) Scene
    {
        Gs::Matrix4f    vpMatrix;
        Gs::Matrix4f    vpShadowMatrix;
        Gs::Vector3f    lightDir        = Gs::Vector3f(-0.25f, -0.7f, 1.25f).Normalized();
        float           shininess       = 90.0f;                                            // Blinn-phong specular power factor
        Gs::Vector3f    viewPos;                                                        // World-space camera position
        float           shadowSizeInv   = 0.0f;
        Gs::Vector3f    warpCenter;
        float           warpIntensity   = 0.0f;
        Gs::Vector3f    bendDir;
        float           ambientItensity = 0.6f;
        float           groundTint[3]   = { 0.8f, 1.0f, 0.6f };
        float           groundScale     = 5.0f;
        LLGL::ColorRGBf lightColor      = {};
        float           warpScaleInv    = 1.0f;
    }
    scene;

    enum Unifom
    {
        Uniform_worldOffset = 0,
        Uniform_bendIntensity,
        Uniform_firstInstance
    };

    struct alignas(4) WorldTransform
    {
        float           worldOffset[3]  = { 0.0f, 0.0f, 0.0f };
        float           bendIntensity   = 0.0f;
    }
    worldTransform;

    struct Vertex
    {
        float           position[3];
        float           normal[3];
        float           texCoord[2];
    };

    struct Instance
    {
        float           wMatrix[4][3];
        float           color[4];
    };

    // Decor for trees in the background
    struct Decor
    {
        int             gridPos[2]      = {};
        std::uint32_t   instanceIndex   = 0;
    };

    struct Tile
    {
        Tile() :
            instanceIndex { ~0u },
            isActive      { 0   }
        {
        }

        std::uint32_t   instanceIndex; // Index into 'meshInstances'
        std::uint32_t   isActive : 1;

        bool IsValid() const
        {
            return (instanceIndex != ~0u);
        }

        bool IsActivated() const
        {
            return (isActive != 0);
        }
    };

    struct TileRow
    {
        std::vector<Tile> tiles;
    };

    struct TileGrid
    {
        std::vector<TileRow>    rows;
        int                     gridSize[2] = {}; // Bounding box in grid coordinates

        void Resize(int width, int height)
        {
            if (width > gridSize[0] || height > gridSize[1])
            {
                gridSize[0] = std::max<int>(gridSize[0], width  + 1);
                gridSize[1] = std::max<int>(gridSize[1], height + 1);

                rows.resize(static_cast<std::size_t>(gridSize[1]));
                for (TileRow& row : rows)
                    row.tiles.resize(static_cast<std::size_t>(gridSize[0]));
            }
        }

        void Put(int x, int y, const Tile* tile)
        {
            Resize(x + 1, y + 1);
            if (tile != nullptr)
            {
                if (Tile* ownTile = Get(x, y))
                    *ownTile = *tile;
            }
        }

        Tile* Get(int x, int y)
        {
            return (x >= 0 && x < gridSize[0] && y >= 0 && y < gridSize[1] ? &(rows[y].tiles[x]) : nullptr);
        }

        const Tile* Get(int x, int y) const
        {
            return (x >= 0 && x < gridSize[0] && y >= 0 && y < gridSize[1] ? &(rows[y].tiles[x]) : nullptr);
        }

        // Counts all valid tile in this grid
        std::uint32_t CountTiles() const
        {
            std::uint32_t n = 0;
            for (const TileRow& row : rows)
            {
                for (const Tile& tile : row.tiles)
                {
                    if (tile.IsValid())
                        ++n;
                }
            }
            return n;
        };
    };

    struct Player;

    struct InstanceRange
    {
        std::uint32_t begin = ~0u;
        std::uint32_t end   = 0u;

        std::uint32_t Count() const
        {
            return (begin < end ? end - begin : 0);
        }

        void Invalidate()
        {
            begin   = ~0u;
            end     = 0u;
        }

        void Insert(std::uint32_t newBegin, std::uint32_t newEnd)
        {
            begin   = std::min<std::uint32_t>(begin, newBegin);
            end     = std::max<std::uint32_t>(end, newEnd);
        }

        void Insert(std::uint32_t index)
        {
            Insert(index, index + 1);
        }
    };

    struct Level
    {
        std::string             name;
        LLGL::ColorRGBub        wallColors[2];
        TileGrid                floor;
        TileGrid                walls;
        std::vector<Decor>      trees;
        std::vector<Instance>   meshInstances;
        InstanceRange           meshInstanceDirtyRange;
        InstanceRange           tileInstanceRange;
        InstanceRange           treeInstanceRange;
        int                     gridSize[2]                 = {}; // Bounding box in grid coordinates
        int                     playerStart[2]              = {};
        float                   viewDistance                = 0.0f;
        int                     activatedTiles              = 0;
        int                     maxTilesToActivate          = 0;
        float                   lightPhase                  = 0.0f;

        bool IsWall(int x, int y) const
        {
            const Tile* tile = walls.Get(x, y);
            return (tile != nullptr && tile->IsValid());
        }

        bool IsFloor(int x, int y) const
        {
            const Tile* tile = floor.Get(x, y);
            return (tile != nullptr && tile->IsValid());
        }

        bool IsTileBlocked(int x, int y) const
        {
            if (IsWall(x, y))
                return true;
            const Tile* tile = floor.Get(x, y);
            return (tile != nullptr && tile->IsActivated());
        }

        bool IsTileHole(int x, int y) const
        {
            const Tile* tile = floor.Get(x, y);
            return (tile == nullptr || !tile->IsValid());
        }

        // Returns true if a player at the specified grid position is completely blocked. This implies the level is lost.
        bool IsPlayerBlocked(int x, int y) const
        {
            return
            (
                IsTileBlocked(x + 1, y) &&
                IsTileBlocked(x - 1, y) &&
                IsTileBlocked(x, y + 1) &&
                IsTileBlocked(x, y - 1)
            );
        }

        // Returns true if all tiles have been activated.
        bool IsCompleted() const
        {
            return (activatedTiles == maxTilesToActivate);
        }

        // Activates the specified tile and returns true if this was the last tile to activate.
        bool ActivateTile(int x, int y, const float (&color)[3])
        {
            Tile* tile = floor.Get(x, y);
            if (tile != nullptr && tile->IsValid())
            {
                if (tile->isActive == 0)
                {
                    tile->isActive = 1;
                    Instance& instance = meshInstances[tile->instanceIndex];
                    {
                        instance.color[0] = color[0];
                        instance.color[1] = color[1];
                        instance.color[2] = color[2];
                    }
                    meshInstanceDirtyRange.Insert(tile->instanceIndex);
                    ++activatedTiles;
                    return true;
                }
            }
            return false;
        }

        void PutPlayer(Player& player)
        {
            player.Put(playerStart);
            ActivateTile(playerStart[0], playerStart[1], playerColor);
        }

        void ResetTiles()
        {
            activatedTiles      = 0;
            maxTilesToActivate  = 0;

            for_range(row, gridSize[1])
            {
                for_range(col, gridSize[0])
                {
                    Tile* floorTile = floor.Get(col, row);
                    if (floorTile != nullptr && floorTile->IsValid())
                    {
                        Instance& floorInstance = meshInstances[floorTile->instanceIndex];

                        const Tile* wallTile = walls.Get(col, row);
                        if (wallTile != nullptr && wallTile->IsValid())
                        {
                            // Copy floor color from wall if it's underneath
                            const Instance& wallInstance = meshInstances[wallTile->instanceIndex];
                            floorInstance.color[0] = wallInstance.color[0];
                            floorInstance.color[1] = wallInstance.color[1];
                            floorInstance.color[2] = wallInstance.color[2];
                        }
                        else
                        {
                            // Reset floor color to default
                            floorInstance.color[0] = 1.0f;
                            floorInstance.color[1] = 1.0f;
                            floorInstance.color[2] = 1.0f;
                            ++maxTilesToActivate;
                        }

                        // Reset tile
                        floorTile->isActive = 0;
                    }
                }
            }
        }
    };

    struct Camera
    {
        float           viewDistance = 0.0f;
        Gs::Vector2f    levelCenterPos;

        void FocusOnLevel(const Level& level)
        {
            viewDistance = level.viewDistance;
            levelCenterPos.x = static_cast<float>(level.gridSize[0]) - 1.0f;
            levelCenterPos.y = static_cast<float>(level.gridSize[1]) - 1.0f;
        }

        void TransitionBetweenLevels(const Level& levelA, const Level& levelB, float transition)
        {
            Camera camA, camB;
            camA.FocusOnLevel(levelA);
            camB.FocusOnLevel(levelB);
            viewDistance    = Gs::Lerp(camA.viewDistance, camB.viewDistance, transition);
            levelCenterPos  = Gs::Lerp(camA.levelCenterPos, camB.levelCenterPos, transition);
        }
    }
    camera;

    enum class Movement
    {
        Free,
        BlockedByWall,
        BlockedByTile,
    };

    struct Player
    {
        Instance    instance                    = {};
        int         gridPos[2]                  = {};
        int         moveDirStack                = 0;        // Number of stack entries
        int         moveDir[inputStackSize][2]  = {};       // Stack of directions the player is about to move
        int         leanDir[2]                  = {};       // Direction player is leaning towards (before moving)
        float       leanFactor                  = 0.0f;     // Interpolation factor [0, x] where x is ~0.2 when the player is leaning
        float       moveTransition              = 0.0f;
        bool        isDescending                = false;
        bool        isFalling                   = false;
        bool        hasFadedAway                = false;
        float       fadeAwayTime                = 0.0f;
        float       descendPhase                = 0.0f;
        float       fallDepth                   = 0.0f;
        float       fallVelocity                = 0.0f;
        bool        isExploding                 = false;

        void Move(int dirX, int dirZ)
        {
            if (moveDirStack < inputStackSize)
            {
                for_subrange_reverse(i, 1, moveDirStack + 1)
                {
                    moveDir[i][0] = moveDir[i - 1][0];
                    moveDir[i][1] = moveDir[i - 1][1];
                }
                moveDir[0][0] = dirX;
                moveDir[0][1] = dirZ;
                ++moveDirStack;
            }
            leanDir[0] = 0;
            leanDir[1] = 0;
        }

        Movement GetMovability(const Level* level, int dirX, int dirZ) const
        {
            if (level != nullptr)
            {
                int nextPosX = gridPos[0] + dirX;
                int nextPosZ = gridPos[1] + dirZ;
                if (level->IsTileBlocked(nextPosX, nextPosZ))
                {
                    if (level->IsWall(nextPosX, nextPosZ))
                        return Movement::BlockedByWall;
                    else
                        return Movement::BlockedByTile;
                }
            }
            return Movement::Free;
        }

        void Lean(int dirX, int dirZ)
        {
            leanDir[0] = dirX;
            leanDir[1] = dirZ;
        }

        bool IsLeaning() const
        {
            return (leanDir[0] != 0 || leanDir[1] != 0);
        }

        void Put(const int (&pos)[2])
        {
            // Reset all player states and place onto grid position
            gridPos[0]          = pos[0];
            gridPos[1]          = pos[1];
            moveDirStack        = 0;
            moveTransition      = 0.0f;
            isDescending        = true;
            isFalling           = false;
            hasFadedAway        = false;
            fadeAwayTime        = 0.0f;
            descendPhase        = 0.0f;
            fallDepth           = 0.0f;
            fallVelocity        = 0.0f;
            leanFactor          = 0.0f;
            leanDir[0]          = 0;
            leanDir[1]          = 0;
            isExploding         = false;

            // Reset player color (initially transparent)
            instance.color[0]   = playerColor[0];
            instance.color[1]   = playerColor[1];
            instance.color[2]   = playerColor[2];
            instance.color[3]   = 0.0f;
        }
    }
    player;

    // Abstracts the segmentation of a large buffer of mesh instances into one or more segments.
    // This is mainly required to split the instance buffer into smaller chunks for GLES and WebGL
    // so they fit into a fixed size uniform buffer blocks.
    class InstanceBuffer
    {
        static constexpr std::uint32_t  cbufferNumInstances = 64u;      // This must match MAX_NUM_INSTANCES in ESSL shaders

        std::vector<LLGL::Buffer*>      segments;                       // Buffer segments
        std::uint64_t                   sizePerSegment      = 0;        // Size (in bytes) per segment
        std::uint32_t                   capacity            = 0;        // Number of instances the instance buffer can hold
        std::uint64_t                   bufferLimit         = 0;
        bool                            isCbuffer           = false;    // Use cbuffer when storage buffers are not available

    public:

        bool IsCbuffer() const
        {
            return isCbuffer;
        }

        void Init(const LLGL::RenderingCapabilities& caps)
        {
            // SSBOs are not supported in ESSL for instance, so when storage buffers are not supported,
            // we use a uniform buffer (aka constant buffer) for instances
            isCbuffer   = !caps.features.hasStorageBuffers;
            bufferLimit = (isCbuffer ? caps.limits.maxConstantBufferSize : caps.limits.maxBufferSize);

            #if ENABLE_VERBOSE_LOGS
            if (isCbuffer)
                LLGL::Log::Printf("Storage buffers not supported: Using constant buffers for instanced drawing\n");
            #endif
        }

        void Resize(LLGL::RenderSystem& renderer, std::uint32_t numInstances)
        {
            // Check if the buffer must be resized
            if (numInstances <= capacity)
                return;

            // When allocating instances as constant buffer, we have to allocate a minimum size.
            // This minimum size is also the maximum number of instances per draw call,
            // so we'll have to iterate through the buffer when more instances have to be drawn.
            if (isCbuffer)
                numInstances = std::max<std::uint32_t>(numInstances, cbufferNumInstances);

            // Check if number of instances will exceed limit
            std::string bufferName;
            const std::uint64_t bufferSize = sizeof(Instance) * numInstances;

            sizePerSegment = std::min<std::uint64_t>(bufferLimit, (isCbuffer ? sizeof(Instance) * cbufferNumInstances : bufferSize));
            const std::uint64_t numSegments = (bufferSize + sizePerSegment - 1) / sizePerSegment;

            segments.resize(static_cast<std::size_t>(numSegments));

            for_range(i, segments.size())
            {
                // Release previous buffers
                if (segments[i] != nullptr)
                    renderer.Release(*segments[i]);

                // Create instance buffer from mesh instance data
                bufferName = "InstanceBuffer[" + std::to_string(i) + "]";
                LLGL::BufferDescriptor instanceBufferDesc;
                {
                    instanceBufferDesc.debugName    = bufferName.c_str();
                    instanceBufferDesc.size         = sizePerSegment;
                    instanceBufferDesc.stride       = (isCbuffer ? 0 : sizeof(Instance));
                    instanceBufferDesc.bindFlags    = (isCbuffer ? LLGL::BindFlags::ConstantBuffer : LLGL::BindFlags::Sampled);
                }
                segments[i] = renderer.CreateBuffer(instanceBufferDesc);
            }

            // Update capacity of how many instances this buffer can hold
            capacity = static_cast<std::uint32_t>((sizePerSegment / sizeof(Instance)) * numSegments);

            #if ENABLE_VERBOSE_LOGS
            // Log new instance buffer size
            const char* segmentsSuffix = (segments.size() == 1 ? "segment" : "segments");
            if (bufferLimit < UINT32_MAX)
            {
                LLGL::Log::Printf(
                    "Resized instance buffer: %s (%zu %s) for %u instances (limit is %s)\n",
                    BytesToString(bufferSize).c_str(), segments.size(), segmentsSuffix, capacity, BytesToString(bufferLimit).c_str()
                );
            }
            else
            {
                LLGL::Log::Printf(
                    "Resized instance buffer: %s (%zu %s) for %u instances\n",
                    BytesToString(bufferSize).c_str(), segments.size(), segmentsSuffix, capacity
                );
            }
            #endif
        }

        void Write(LLGL::RenderSystem& renderer, std::uint64_t offset, const void* data, std::uint64_t dataSize)
        {
            const char* byteAlignedData     = reinterpret_cast<const char*>(data);
            const char* byteAlignedDataEnd  = byteAlignedData + dataSize;

            while (dataSize > 0)
            {
                const std::uint64_t offsetEnd = ((offset + sizePerSegment) / sizePerSegment) * sizePerSegment;
                const std::uint64_t batchDataSize = std::min<std::uint64_t>(dataSize, offsetEnd - offset);

                renderer.WriteBuffer(*segments[offset / sizePerSegment], offset % sizePerSegment, byteAlignedData, batchDataSize);

                byteAlignedData += batchDataSize;
                offset += batchDataSize;
                dataSize -= batchDataSize;
            }
        }

        void Update(LLGL::CommandBuffer& cmdBuffer, std::uint64_t offset, const void* data, std::uint16_t dataSize)
        {
            const char* byteAlignedData     = reinterpret_cast<const char*>(data);
            const char* byteAlignedDataEnd  = byteAlignedData + dataSize;

            while (dataSize > 0)
            {
                const std::uint64_t offsetEnd = ((offset + sizePerSegment) / sizePerSegment) * sizePerSegment;
                const std::uint16_t batchDataSize = static_cast<std::uint16_t>(std::min<std::uint64_t>(dataSize, offsetEnd - offset));

                cmdBuffer.UpdateBuffer(*segments[offset / sizePerSegment], offset % sizePerSegment, byteAlignedData, batchDataSize);

                byteAlignedData += batchDataSize;
                offset += batchDataSize;
                dataSize -= batchDataSize;
            }
        }

        void DrawInstances(LLGL::CommandBuffer& cmdBuffer, std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
        {
            if (segments.empty())
                return;

            // Split draw call into batches of instances that can fit into the shader's cbuffer (MAX_NUM_INSTANCES in ESSL)
            const std::uint32_t numInstancePerSegment = static_cast<std::uint32_t>(sizePerSegment/sizeof(Instance));
            std::uint32_t instanceIndex = firstInstance;
            const std::uint32_t endInstanceIndex = firstInstance + numInstances;
            const std::uint32_t zeroIndex = 0;

            while (instanceIndex < endInstanceIndex)
            {
                // Move to next index in strides of 'numInstancePerSegment',
                // e.g. stride is 64 and we start at 60, then indices are 60, 64, 128, 192, 256, ...
                const std::uint32_t nextInstanceIndex = std::min<std::uint32_t>(((instanceIndex + numInstancePerSegment) / numInstancePerSegment) * numInstancePerSegment, firstInstance + numInstances);
                const std::uint32_t numInstancesPerBatch = nextInstanceIndex - instanceIndex;

                cmdBuffer.SetResource(1, *segments[instanceIndex / numInstancePerSegment]);

                const std::uint32_t instanceOffset = instanceIndex % numInstancePerSegment;
                cmdBuffer.SetUniforms(Uniform_firstInstance, &instanceOffset, sizeof(instanceOffset));

                cmdBuffer.DrawInstanced(numVertices, firstVertex, numInstancesPerBatch);

                instanceIndex = nextInstanceIndex;
            }
        }
    };

    std::vector<Level>  levels;
    int                 currentLevelIndex           = -1;
    Level*              currentLevel                = nullptr;
    Level*              nextLevel                   = nullptr;
    std::uint32_t       currentLevelInstanceOffset  = 0;
    std::uint32_t       nextLevelInstanceOffset     = 0;
    float               levelTransition             = 0.0f; // Transitioning state between two levesl - in the range [0, 1]
    float               levelDistance               = 0.0f; // Distance between two levels (to transition between them)
    float               levelDoneTransition         = 0.0f; // Transition starting when the level is completed
    InstanceBuffer      instanceBuffer;
    LLGL::Offset2D      pointerStartPos;                    // Start position for the pointer (mouse cursor or touch input)
    bool                pointerPlayerMovement       = false;

    struct Gradient
    {
        Gs::Vector3f    points[2];
        LLGL::ColorRGBf colors[2];

        LLGL::ColorRGBf operator() (const Gs::Vector3f& p) const
        {
            const Gs::Vector3f closestPoint = ClosestPointOnLineSegment(points[0], points[1], p);
            const float closestPointDist = Gs::Distance(points[0], closestPoint);
            const float segmentLength = Gs::Distance(points[0], points[1]);
            const float interpolation = closestPointDist / segmentLength;
            return Gs::Lerp(colors[0], colors[1], interpolation);
        }
    };

    struct Effects
    {
        bool    warpEnabled         = false;
        float   warpDuration        = 1.0f; // in seconds
        int     warpBounces         = 3;
        float   warpMaxIntensity    = 2.0f;
        float   warpTime            = 0.0f;

        float   treeBendTime        = 0.0f;

        bool    lightPhaseChanged   = false;
        float   lightPhase          = 0.0f;
        float   lightPhaseTarget    = 0.0f;

        float   playerJumpTime      = 0.0f; // Make the player jump to get the user's attention
        bool    playerJumpEnabled   = false;
        float   playerJumpPhase     = 0.0f; // [0..1]

        bool    jitterEnabled       = false;
        float   jitterTime          = 0.0f;
        float   jitterRotation[3]   = {};
    }
    effects;

public:

    Example_HelloGame() :
        ExampleBase { "LLGL Example: HelloGame" }
    {
        // Create all graphics objects
        auto vertexFormat = CreateResources();
        CreateShaders(vertexFormat);
        CreatePipelines();
        LoadLevels();
        SelectLevel(0);
    }

private:

    LLGL::VertexFormat CreateResources()
    {
        // Initialize instance buffer
        instanceBuffer.Init(renderer->GetRenderingCaps());

        // Specify vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.attributes =
        {
            LLGL::VertexAttribute{ "position", LLGL::Format::RGB32Float, /*location:*/ 0, offsetof(Vertex, position), sizeof(Vertex) },
            LLGL::VertexAttribute{ "normal",   LLGL::Format::RGB32Float, /*location:*/ 1, offsetof(Vertex, normal  ), sizeof(Vertex) },
            LLGL::VertexAttribute{ "texCoord", LLGL::Format::RG32Float,  /*location:*/ 2, offsetof(Vertex, texCoord), sizeof(Vertex) },
        };

        // Load 3D models
        std::vector<TexturedVertex> vertices;
        mdlPlayer   = LoadObjModel(vertices, "HelloGame_Player.obj");
        mdlBlock    = LoadObjModel(vertices, "HelloGame_Block.obj");
        mdlTree     = LoadObjModel(vertices, "HelloGame_Tree.obj");
        mdlGround   = LoadObjModel(vertices, "HelloGame_Ground.obj");

        // Create vertex, index, and constant buffer
        vertexBuffer    = CreateVertexBuffer(vertices, vertexFormat);
        cbufferScene    = CreateConstantBuffer(scene);

        // Load texture and sampler
        groundColorMap = LoadTexture("Grass.jpg");
        groundColorMapSampler = renderer->CreateSampler({});

        // Create shadow map
        LLGL::TextureDescriptor shadowMapDesc;
        {
            shadowMapDesc.debugName     = "ShadowMap";
            shadowMapDesc.type          = LLGL::TextureType::Texture2D;
            shadowMapDesc.bindFlags     = LLGL::BindFlags::DepthStencilAttachment | LLGL::BindFlags::Sampled;
            shadowMapDesc.format        = LLGL::Format::D32Float;
            shadowMapDesc.extent.width  = static_cast<std::uint32_t>(shadowMapSize);
            shadowMapDesc.extent.height = static_cast<std::uint32_t>(shadowMapSize);
            shadowMapDesc.extent.depth  = 1;
            shadowMapDesc.mipLevels     = 1;
        }
        shadowMap = renderer->CreateTexture(shadowMapDesc);

        LLGL::RenderTargetDescriptor shadowTargetDesc;
        {
            shadowTargetDesc.debugName              = "ShadowMapTarget";
            shadowTargetDesc.resolution.width       = shadowMapDesc.extent.width;
            shadowTargetDesc.resolution.height      = shadowMapDesc.extent.height;
            shadowTargetDesc.depthStencilAttachment = shadowMap;
        }
        shadowMapTarget = renderer->CreateRenderTarget(shadowTargetDesc);

        LLGL::SamplerDescriptor shadowSamplerDesc;
        {
            // Clamp-to-border sampler address mode requires GLES 3.2, so use standard clamp mode in case hardware only supports GLES 3.0
            if (renderer->GetRendererID() == LLGL::RendererID::OpenGLES ||
                renderer->GetRendererID() == LLGL::RendererID::WebGL)
            {
                shadowSamplerDesc.addressModeU      = LLGL::SamplerAddressMode::Clamp;
                shadowSamplerDesc.addressModeV      = LLGL::SamplerAddressMode::Clamp;
                shadowSamplerDesc.addressModeW      = LLGL::SamplerAddressMode::Clamp;
            }
            else
            {
                shadowSamplerDesc.addressModeU      = LLGL::SamplerAddressMode::Border;
                shadowSamplerDesc.addressModeV      = LLGL::SamplerAddressMode::Border;
                shadowSamplerDesc.addressModeW      = LLGL::SamplerAddressMode::Border;
                shadowSamplerDesc.borderColor[0]    = 1.0f;
                shadowSamplerDesc.borderColor[1]    = 1.0f;
                shadowSamplerDesc.borderColor[2]    = 1.0f;
                shadowSamplerDesc.borderColor[3]    = 1.0f;
            }
            shadowSamplerDesc.compareEnabled    = true;
            shadowSamplerDesc.mipMapEnabled     = false;
        }
        shadowMapSampler = renderer->CreateSampler(shadowSamplerDesc);

        // Pass inverse size of shadow map to shader for PCF shadow mapping
        scene.shadowSizeInv = 1.0f / static_cast<float>(shadowMapSize);

        return vertexFormat;
    }

    static std::string BytesToString(std::uint64_t val)
    {
        char buf[128] = { '\0' };
        if (val / 1024 / 1024 / 1024 > 0)
            ::sprintf(buf, "%.2f GiB", static_cast<double>(val) / 1024.0 / 1024.0 / 1024.0);
        else if (val / 1024 / 1024 > 0)
            ::sprintf(buf, "%.2f MiB", static_cast<double>(val) / 1024.0 / 1024.0);
        else if (val / 1024 > 0)
            ::sprintf(buf, "%.2f KiB", static_cast<double>(val) / 1024.0);
        else
            ::sprintf(buf, "%" PRIu64 " Bytes", val);
        return std::string(buf);
    }

    void CreateShaders(const LLGL::VertexFormat& vertexFormat)
    {
        if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            sceneShaders.vs  = LoadShader({ LLGL::ShaderType::Vertex,   "HelloGame.hlsl", "VSInstance", "vs_5_0" }, { vertexFormat });
            sceneShaders.ps  = LoadShader({ LLGL::ShaderType::Fragment, "HelloGame.hlsl", "PSInstance", "ps_5_0" });

            groundShaders.vs = LoadShader({ LLGL::ShaderType::Vertex,   "HelloGame.hlsl", "VSGround",   "vs_5_0" }, { vertexFormat });
            groundShaders.ps = LoadShader({ LLGL::ShaderType::Fragment, "HelloGame.hlsl", "PSGround",   "ps_5_0" });
        }
        else if (Supported(LLGL::ShadingLanguage::GLSL))
        {
            sceneShaders.vs  = LoadShaderAndPatchClippingOrigin({ LLGL::ShaderType::Vertex,   "HelloGame.VSInstance.450core.vert" }, { vertexFormat });
            sceneShaders.ps  = LoadShader                      ({ LLGL::ShaderType::Fragment, "HelloGame.PSInstance.450core.frag" });

            groundShaders.vs = LoadShader({ LLGL::ShaderType::Vertex,   "HelloGame.VSGround.450core.vert" }, { vertexFormat });
            groundShaders.ps = LoadShader({ LLGL::ShaderType::Fragment, "HelloGame.PSGround.450core.frag" });
        }
        else if (Supported(LLGL::ShadingLanguage::ESSL))
        {
            sceneShaders.vs  = LoadShaderAndPatchClippingOrigin({ LLGL::ShaderType::Vertex,   "HelloGame.VSInstance.300es.vert" }, { vertexFormat });
            sceneShaders.ps  = LoadShader                      ({ LLGL::ShaderType::Fragment, "HelloGame.PSInstance.300es.frag" });

            groundShaders.vs = LoadShader({ LLGL::ShaderType::Vertex,   "HelloGame.VSGround.300es.vert" }, { vertexFormat });
            groundShaders.ps = LoadShader({ LLGL::ShaderType::Fragment, "HelloGame.PSGround.300es.frag" });
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            sceneShaders.vs  = LoadShader({ LLGL::ShaderType::Vertex,   "HelloGame.VSInstance.450core.vert.spv" }, { vertexFormat });
            sceneShaders.ps  = LoadShader({ LLGL::ShaderType::Fragment, "HelloGame.PSInstance.450core.frag.spv" });

            groundShaders.vs = LoadShader({ LLGL::ShaderType::Vertex,   "HelloGame.VSGround.450core.vert.spv" }, { vertexFormat });
            groundShaders.ps = LoadShader({ LLGL::ShaderType::Fragment, "HelloGame.PSGround.450core.frag.spv" });
        }
        else if (Supported(LLGL::ShadingLanguage::Metal))
        {
            sceneShaders.vs  = LoadShader({ LLGL::ShaderType::Vertex,   "HelloGame.hlsl", "VSInstance", "1.1" }, { vertexFormat });
            sceneShaders.ps  = LoadShader({ LLGL::ShaderType::Fragment, "HelloGame.hlsl", "PSInstance", "1.1" });

            groundShaders.vs = LoadShader({ LLGL::ShaderType::Vertex,   "HelloGame.hlsl", "VSGround",   "1.1" }, { vertexFormat });
            groundShaders.ps = LoadShader({ LLGL::ShaderType::Fragment, "HelloGame.hlsl", "PSGround",   "1.1" });
        }
        else
        {
            throw std::runtime_error("No shaders provided for this backend");
        }
    }

    void CreatePipelines()
    {
        const bool needsExplicitMultiSample = (GetSampleCount() > 1 && !IsDirect3D());
        const bool needsUniqueBindingSlots  = (IsVulkan());

        // Create PSO for instanced meshes
        scenePSOLayout[0] = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(Scene@1):vert:frag,"
                "%s(instances@2):vert,"
                "texture(shadowMap@4):frag,"
                "sampler(shadowMapSampler@%d):frag,"
                "float3(worldOffset),"  // Uniform_worldOffset   (0)
                "float(bendIntensity)," // Uniform_bendIntensity (1)
                "uint(firstInstance),", // Uniform_firstInstance (2)
                (instanceBuffer.IsCbuffer() ? "cbuffer" : "buffer"),
                (needsUniqueBindingSlots ? 5 : 4)
            )
        );

        LLGL::GraphicsPipelineDescriptor scenePSODesc;
        {
            scenePSODesc.debugName                      = "InstancedMesh.PSO";
            scenePSODesc.pipelineLayout                 = scenePSOLayout[0];
            scenePSODesc.renderPass                     = swapChain->GetRenderPass();
            scenePSODesc.vertexShader                   = sceneShaders.vs;
            scenePSODesc.fragmentShader                 = sceneShaders.ps;
            scenePSODesc.depth.testEnabled              = true;
            scenePSODesc.depth.writeEnabled             = true;
            scenePSODesc.rasterizer.cullMode            = LLGL::CullMode::Back;
            scenePSODesc.rasterizer.multiSampleEnabled  = needsExplicitMultiSample;
            scenePSODesc.blend.targets[0].blendEnabled  = true;
        }
        scenePSO[0] = renderer->CreatePipelineState(scenePSODesc);
        ReportPSOErrors(scenePSO[0]);

        // Create PSO for shadow-mapping but without a fragment shader
        scenePSOLayout[1] = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(Scene@1):vert,"
                "%s(instances@2):vert,"
                "float3(worldOffset),"
                "float(bendIntensity),"
                "uint(firstInstance),",
                (instanceBuffer.IsCbuffer() ? "cbuffer" : "buffer")
            )
        );

        {
            scenePSODesc.debugName                              = "InstancedMesh.Shadow.PSO";
            scenePSODesc.pipelineLayout                         = scenePSOLayout[1];
            scenePSODesc.renderPass                             = shadowMapTarget->GetRenderPass();
            scenePSODesc.fragmentShader                         = nullptr;
            scenePSODesc.rasterizer.depthBias.constantFactor    = 1.0f;
            scenePSODesc.rasterizer.depthBias.slopeFactor       = 1.0f;
            scenePSODesc.rasterizer.cullMode                    = LLGL::CullMode::Front;
            scenePSODesc.rasterizer.multiSampleEnabled          = false;
            scenePSODesc.blend.targets[0].blendEnabled          = false;
            scenePSODesc.blend.targets[0].colorMask             = 0x0;
        }
        scenePSO[1] = renderer->CreatePipelineState(scenePSODesc);
        ReportPSOErrors(scenePSO[1]);

        // Create PSO for background
        groundPSOLayout = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(Scene@1):vert:frag,"
                "texture(colorMap@2):frag,"
                "sampler(colorMapSampler@%d):frag,"
                "texture(shadowMap@4):frag,"
                "sampler(shadowMapSampler@%d):frag,",
                (needsUniqueBindingSlots ? 3 : 2),
                (needsUniqueBindingSlots ? 5 : 4)
            )
        );

        LLGL::GraphicsPipelineDescriptor groundPSODesc;
        {
            groundPSODesc.debugName                     = "Ground.PSO";
            groundPSODesc.pipelineLayout                = groundPSOLayout;
            groundPSODesc.vertexShader                  = groundShaders.vs;
            groundPSODesc.fragmentShader                = groundShaders.ps;
            groundPSODesc.renderPass                    = swapChain->GetRenderPass();
            groundPSODesc.depth.testEnabled             = true;
            groundPSODesc.depth.writeEnabled            = true;
            groundPSODesc.rasterizer.cullMode           = LLGL::CullMode::Back;
            groundPSODesc.rasterizer.multiSampleEnabled = needsExplicitMultiSample;
        }
        groundPSO = renderer->CreatePipelineState(groundPSODesc);
        ReportPSOErrors(groundPSO);
    }

    static Gs::Vector3f GridPosToWorldPos(const int (&gridPos)[2], float posY)
    {
        const float posX = static_cast<float>(gridPos[0])*2.0f;
        const float posZ = static_cast<float>(gridPos[1])*2.0f;
        return Gs::Vector3f{ posX, posY, posZ };
    }

    void SetPlayerTransform(InstanceMatrixType& outMatrix, const int (&gridPos)[2], int moveX, int moveZ, float posY, float transition)
    {
        outMatrix.LoadIdentity();

        // Position player onto current grid
        const Gs::Vector3f pos = GridPosToWorldPos(gridPos, posY);
        Gs::Translate(outMatrix, pos);

        // Animate jumping as a scaling factor
        if (effects.playerJumpEnabled)
        {
            const float jumpDamping = Gs::SmoothStep(1.0f - effects.playerJumpPhase);
            const float jumpPhase   = Gs::pi + effects.playerJumpPhase * Gs::pi * 2.0f * static_cast<float>(playerJumpBounces);
            const float jumpHeight  = std::sin(jumpPhase) * playerJumpHeight * jumpDamping;
            Gs::Translate(outMatrix, Gs::Vector3f{ 0, jumpHeight*0.5f, 0 });
            Gs::Scale(outMatrix, Gs::Vector3f{ 1, 1.0f + jumpHeight, 1 });
        }

        // Rotate around the edge to move the player
        if (transition > 0.0f)
        {
            const float angle = Gs::SmoothStep(transition) * Gs::pi * 0.5f;
            if (moveX < 0)
            {
                // Move left
                Gs::RotateFree(outMatrix, Gs::Vector3f{ 0,0,1 }, -angle, Gs::Vector3f{ -1,-1, 0 });
            }
            else if (moveX > 0)
            {
                // Move right
                Gs::RotateFree(outMatrix, Gs::Vector3f{ 0,0,1 }, +angle, Gs::Vector3f{ +1,-1, 0 });
            }
            else if (moveZ < 0)
            {
                // Move forwards
                Gs::RotateFree(outMatrix, Gs::Vector3f{ 1,0,0 }, +angle, Gs::Vector3f{  0,-1,-1 });
            }
            else if (moveZ > 0)
            {
                // Move backwards
                Gs::RotateFree(outMatrix, Gs::Vector3f{ 1,0,0 }, -angle, Gs::Vector3f{  0,-1,+1 });
            }
        }
    }

    void SetPlayerTransformBounce(InstanceMatrixType& outMatrix, const int (&gridPos)[2], int moveX, int moveZ, float posY, float transition)
    {
        const float bounceTransition = std::abs(std::sin(transition * Gs::pi * 2.0f)) * Gs::SmoothStep(1.0f - transition * 0.5f) * 0.2f;
        SetPlayerTransform(outMatrix, gridPos, moveX, moveZ, posY, bounceTransition);
    }

    void SetPlayerTransformSpin(InstanceMatrixType& outMatrix, float& outTransparency, const int (&gridPos)[2], float transition)
    {
        outMatrix.LoadIdentity();

        // Interpolate between start and end positions
        const float phase       = std::pow(transition, 0.3f);
        const float posYStart   = wallPosY + playerDescendHeight;
        const float posYEnd     = wallPosY;
        const float posY        = Gs::Lerp(posYStart, posYEnd, phase);

        // Position player onto current grid
        const Gs::Vector3f pos = GridPosToWorldPos(gridPos, posY);
        Gs::Translate(outMatrix, pos);

        // Add full rotations on Y-axid
        const float angle = playerDescendRotations * phase * Gs::pi * 2.0f;
        Gs::RotateFree(outMatrix, Gs::Vector3f{ 0, 1, 0 }, angle);

        // Transition transparency
        outTransparency = phase;
    }

    void SetInstanceAttribs(Instance& instance, const int (&gridPos)[2], float posY, const Gradient* gradient = nullptr)
    {
        InstanceMatrixType& wMatrix = *reinterpret_cast<InstanceMatrixType*>(instance.wMatrix);
        wMatrix.LoadIdentity();

        const Gs::Vector3f pos = GridPosToWorldPos(gridPos, posY);
        Gs::Translate(wMatrix, pos);

        LLGL::ColorRGBf color;
        if (gradient != nullptr)
            color = (*gradient)(pos);

        instance.color[0] = color.r;
        instance.color[1] = color.g;
        instance.color[2] = color.b;
        instance.color[3] = 1.0f;
    }

    void GenerateTileInstances(TileGrid& grid, std::vector<Instance>& meshInstances, std::uint32_t& instanceCounter, float posY, const Gradient* gradient)
    {
        int gridPos[2];

        for (gridPos[1] = 0; gridPos[1] < grid.gridSize[1]; ++gridPos[1])
        {
            TileRow& row = grid.rows[gridPos[1]];
            for (gridPos[0] = 0; gridPos[0] < grid.gridSize[0]; ++gridPos[0])
            {
                Tile& tile = row.tiles[gridPos[0]];
                if (tile.IsValid())
                {
                    tile.instanceIndex = instanceCounter++;
                    SetInstanceAttribs(meshInstances[tile.instanceIndex], gridPos, posY, gradient);
                }
            }
        }
    }

    void GenerateDecorInstances(std::vector<Decor>& decors, std::vector<Instance>& meshInstances, std::uint32_t& instanceCounter, const Gradient* gradient)
    {
        for (Decor& decor : decors)
        {
            decor.instanceIndex = instanceCounter++;
            SetInstanceAttribs(meshInstances[decor.instanceIndex], decor.gridPos, 0.0f, gradient);
        }
    }

    void FinalizeLevel(Level& level)
    {
        // Build instance data from tiles
        level.meshInstances.resize(level.floor.CountTiles() + level.walls.CountTiles() + level.trees.size());

        // Colorize wall tiles with gradient
        Gradient gradient;
        gradient.colors[0] = level.wallColors[0].Cast<float>();
        gradient.colors[1] = level.wallColors[1].Cast<float>();
        gradient.points[0] = Gs::Vector3f{ 0.0f, wallPosY, 0.0f };
        gradient.points[1] = Gs::Vector3f{ static_cast<float>(level.gridSize[0])*2.0f, wallPosY, static_cast<float>(level.gridSize[1])*2.0f };

        std::uint32_t instanceCounter = 0;
        GenerateTileInstances(level.floor, level.meshInstances, instanceCounter, 0.0f, nullptr);
        GenerateTileInstances(level.walls, level.meshInstances, instanceCounter, wallPosY, &gradient);
        level.tileInstanceRange.Insert(0, instanceCounter);

        // Colorize tree decors with gradient
        gradient.colors[0] = LLGL::ColorRGBf{ treeColorGradient[0][0], treeColorGradient[0][1], treeColorGradient[0][2] };
        gradient.colors[1] = LLGL::ColorRGBf{ treeColorGradient[1][0], treeColorGradient[1][1], treeColorGradient[1][2] };
        gradient.points[0] = Gs::Vector3f{ 0.0f, wallPosY, 0.0f };
        gradient.points[1] = Gs::Vector3f{ static_cast<float>(level.gridSize[0])*2.0f, wallPosY, static_cast<float>(level.gridSize[1])*2.0f };

        std::uint32_t treeInstanceStart = instanceCounter;
        GenerateDecorInstances(level.trees, level.meshInstances, instanceCounter, &gradient);
        level.treeInstanceRange.Insert(treeInstanceStart, instanceCounter);
    }

    void LoadLevels()
    {
        // Load level files
        const std::vector<std::string> levelsFileLines = ReadTextLines("HelloGame.levels.txt");

        std::string name;
        std::string wallGradient;
        std::vector<std::string> currentGrid;
        float timeOfDay = 0.0f;

        auto HexToInt = [](char c) -> int
        {
            if (c >= '0' && c <= '9')
                return (c - '0');
            else if (c >= 'a' && c <= 'f')
                return 0xA + (c - 'a');
            else if (c >= 'A' && c <= 'F')
                return 0xA + (c - 'A');
            else
                return -1;
        };

        auto FloatFromString = [](const std::string& s) -> float
        {
            std::stringstream sstr(s);
            float val = 0.0f;
            sstr >> val;
            return val;
        };

        auto ParseColorRGB = [&HexToInt](const char*& s) -> LLGL::ColorRGBub
        {
            std::uint32_t color = 0;
            char c;

            // Skip characters until first hex digit is found
            while ((c = *s) != '\0' && HexToInt(c) == -1)
                ++s;

            while ((c = *s) != '\0' && HexToInt(c) != -1)
            {
                color <<= 4;
                color |= HexToInt(c);
                ++s;
            }

            return LLGL::ColorRGBub
            {
                static_cast<std::uint8_t>((color >> 16) & 0xFF),
                static_cast<std::uint8_t>((color >>  8) & 0xFF),
                static_cast<std::uint8_t>((color      ) & 0xFF)
            };
        };

        auto FlushLevelConstruct = [&]() -> void
        {
            if (currentGrid.empty())
                return;

            // Construct a new level
            Level newLevel;

            newLevel.name       = (name.empty() ? "Unnamed" : name);
            newLevel.lightPhase = timeOfDay;

            if (!wallGradient.empty())
            {
                const char* wallGradientStr = wallGradient.c_str();
                newLevel.wallColors[0] = ParseColorRGB(wallGradientStr);
                newLevel.wallColors[1] = ParseColorRGB(wallGradientStr);
            }
            else
            {
                newLevel.wallColors[0] = {};
                newLevel.wallColors[1] = {};
            }

            // Determine longest row
            int gridOffset[2] = { INT_MAX, INT_MAX };
            newLevel.gridSize[0] = 0;
            newLevel.gridSize[1] = 0;

            for_range(i, currentGrid.size())
            {
                const std::string& row = currentGrid[i];
                const std::size_t rowStart = row.find_first_of(".#@");
                if (rowStart != std::string::npos)
                {
                    gridOffset[0] = std::min<int>(gridOffset[0], static_cast<int>(rowStart));
                    gridOffset[1] = std::min<int>(gridOffset[1], static_cast<int>(i));
                    const std::size_t rowEnd = row.find_last_of(".#@");
                    newLevel.gridSize[0] = std::max<int>(newLevel.gridSize[0], static_cast<int>(rowEnd - rowStart) + 1);
                    newLevel.gridSize[1]++;
                }
            }

            newLevel.viewDistance = static_cast<float>(std::max<int>(newLevel.gridSize[0], newLevel.gridSize[1])) * 2.7f;

            // Build grid of tiles row by row by interpreting characters from the level text file
            Tile initialTile;
            initialTile.instanceIndex = 0;

            int gridPosY = newLevel.gridSize[1] + gridOffset[1];
            for (const std::string& row : currentGrid)
            {
                --gridPosY;
                int gridPosX = -gridOffset[0];

                for (char c : row)
                {
                    if (c == '#')
                    {
                        // Add floor and wall tile
                        newLevel.floor.Put(gridPosX, gridPosY, &initialTile);
                        newLevel.walls.Put(gridPosX, gridPosY, &initialTile);
                    }
                    else if (c == '.')
                    {
                        // Add floor tile only
                        newLevel.floor.Put(gridPosX, gridPosY, &initialTile);
                    }
                    else if (c == '@')
                    {
                        // Add floor tile and position player
                        newLevel.floor.Put(gridPosX, gridPosY, &initialTile);
                        newLevel.playerStart[0] = gridPosX;
                        newLevel.playerStart[1] = gridPosY;
                    }
                    else if (c == '$')
                    {
                        // Add floor tile only
                        Decor newDecor;
                        newDecor.gridPos[0] = gridPosX;
                        newDecor.gridPos[1] = gridPosY;
                        newLevel.trees.push_back(newDecor);
                    }
                    ++gridPosX;
                }
            }

            // Finalize level by generating mesh instances for all tiles
            FinalizeLevel(newLevel);

            // Flush new level
            levels.push_back(newLevel);
            currentGrid.clear();
            name.clear();
        };

        for (const std::string& line : levelsFileLines)
        {
            if (line.empty())
                FlushLevelConstruct();
            else if (line.compare(0, 6, "LEVEL:") == 0)
                name = line.substr(line.find_first_not_of(" \t", 6));
            else if (line.compare(0, 6, "WALLS:") == 0)
                wallGradient = line.substr(line.find_first_not_of(" \t", 6));
            else if (line.compare(0, 5, "TIME:") == 0)
                timeOfDay = FloatFromString(line.substr(5));
            else
                currentGrid.push_back(line);
        }

        // Flush even ramining constructs
        FlushLevelConstruct();
    }

    void SelectLevel(int index)
    {
        const int numLevels = static_cast<int>(levels.size());
        if (numLevels == 0)
        {
            // No levels loaded yet
            return;
        }
        if (nextLevel != nullptr)
        {
            // Still transitioning into another level
            return;
        }

        // Wrap level index around the range from both ends
        if (index < 0)
            index = (numLevels + (index % numLevels)) % numLevels;
        if (index >= numLevels)
            index = index % numLevels;

        const float moveDirection = (index > currentLevelIndex ? 1.0f : -1.0f);

        // Update instance buffer
        const std::uint64_t playerBufferSize = sizeof(Instance);
        if (currentLevel != nullptr)
        {
            // Select next level to transition to
            nextLevel = &levels[index];
            nextLevel->ResetTiles();
            levelDistance           = static_cast<float>(currentLevel->gridSize[0] + nextLevel->gridSize[0]) * 1.5f * moveDirection;
            nextLevelInstanceOffset = static_cast<std::uint32_t>(currentLevel->meshInstances.size());

            // Update instance buffer from current and next level instance data plus one instance for the player model
            instanceBuffer.Resize(*renderer, 1 + static_cast<std::uint32_t>(currentLevel->meshInstances.size() + nextLevel->meshInstances.size()));

            const std::uint64_t instanceBufferSize0 = sizeof(Instance) * currentLevel->meshInstances.size();
            const std::uint64_t instanceBufferSize1 = sizeof(Instance) * nextLevel   ->meshInstances.size();

            instanceBuffer.Write(*renderer, playerBufferSize                      , currentLevel->meshInstances.data(), instanceBufferSize0);
            instanceBuffer.Write(*renderer, playerBufferSize + instanceBufferSize0, nextLevel   ->meshInstances.data(), instanceBufferSize1);
        }
        else
        {
            // Select first level
            currentLevel = &levels[index];
            currentLevel->ResetTiles();
            levelDistance           = 0.0f;
            nextLevelInstanceOffset = 0;

            // Update instance buffer from current level instance data plus one instance for the player model
            instanceBuffer.Resize(*renderer, 1 + static_cast<std::uint32_t>(currentLevel->meshInstances.size()));
            instanceBuffer.Write(*renderer, playerBufferSize, currentLevel->meshInstances.data(), sizeof(Instance) * currentLevel->meshInstances.size());
        }

        // Store index to current level to conveniently selecting next and previous levels
        currentLevelIndex           = index;
        currentLevelInstanceOffset  = 0;

        // Position player to start location and set new target light phase
        levels[index].PutPlayer(player);
        SetTargetLightPhase(levels[index].lightPhase);

        // Cancel ongoing effects
        effects.warpEnabled = false;
        scene.warpIntensity = 0.0f;
    }

    void SetShadowMapView()
    {
        // Generate shadow map projection for light direction
        Gs::Vector3f lightDir = scene.lightDir;
        Gs::Vector3f upVector = (lightDir.y < 0.999f ? Gs::Vector3f{ 0, 1, 0 } : Gs::Vector3f{ 0, 0, -1 });
        Gs::Vector3f tangentU = Gs::Cross(upVector, lightDir).Normalized();
        Gs::Vector3f tangentV = Gs::Cross(lightDir, tangentU);

        Gs::Matrix4f lightOrientation;

        lightOrientation(0, 0) = tangentU.x;
        lightOrientation(0, 1) = tangentU.y;
        lightOrientation(0, 2) = tangentU.z;

        lightOrientation(1, 0) = tangentV.x;
        lightOrientation(1, 1) = tangentV.y;
        lightOrientation(1, 2) = tangentV.z;

        lightOrientation(2, 0) = lightDir.x;
        lightOrientation(2, 1) = lightDir.y;
        lightOrientation(2, 2) = lightDir.z;

        lightOrientation.MakeInverse();

        // Update view transformation from light perspective
        scene.vpMatrix.LoadIdentity();
        Gs::Translate(scene.vpMatrix, { camera.levelCenterPos.x, 0.0f, camera.levelCenterPos.y });
        scene.vpMatrix *= lightOrientation;
        Gs::Translate(scene.vpMatrix, { 0, 0, -50.0f });

        const float shadowMapScale = camera.viewDistance*1.5f;
        Gs::Matrix4f lightProjection = OrthogonalProjection(shadowMapScale, shadowMapScale, 0.1f, 100.0f);

        scene.vpMatrix.MakeInverse();
        scene.vpMatrix = lightProjection * scene.vpMatrix;

        // Update shadow map matrix for texture space
        scene.vpShadowMatrix = scene.vpMatrix;
    }

    void SetCameraView()
    {
        // Update view transformation from camera perspective
        scene.vpMatrix.LoadIdentity();
        Gs::Translate(scene.vpMatrix, { camera.levelCenterPos.x, 0, camera.levelCenterPos.y });
        Gs::RotateFree(scene.vpMatrix, { 1, 0, 0 }, Gs::Deg2Rad(-65.0f));
        Gs::Translate(scene.vpMatrix, { 0, 0, -camera.viewDistance });
        scene.viewPos = Gs::TransformVector(scene.vpMatrix, Gs::Vector3f{ 0, 0, 0 });
        scene.vpMatrix.MakeInverse();
        scene.vpMatrix = projection * scene.vpMatrix;
    }

    void StartWarpEffect(float duration = 1.0f, int bounces = 3, float scale = 1.0f, float maxIntensity = 2.0f)
    {
        effects.warpEnabled         = true;
        effects.warpDuration        = duration;
        effects.warpBounces         = bounces;
        effects.warpMaxIntensity    = maxIntensity;
        effects.warpTime            = 0.0f;
        scene.warpCenter.x          = player.instance.wMatrix[3][0];
        scene.warpCenter.y          = player.instance.wMatrix[3][1];
        scene.warpCenter.z          = player.instance.wMatrix[3][2];
        scene.warpScaleInv          = 1.0f / scale;
    }

    void StartJitterEffect()
    {
        effects.jitterEnabled   = true;
        effects.jitterTime      = 0.0f;
    }

    void SetTargetLightPhase(float phase)
    {
        effects.lightPhaseChanged = true;
        effects.lightPhaseTarget = Gs::Saturate(phase);
    }

    void SetLightPhase(float phase)
    {
        phase = Gs::Saturate(phase);
        TimeOfDay timeOfDay = TimeOfDay::Interpolate(timeOfDayMorning, timeOfDayEvening, phase);
        scene.lightDir          = timeOfDay.lightDir;
        scene.ambientItensity   = timeOfDay.ambientIntensity;
        scene.lightColor        = timeOfDay.lightColor;
        effects.lightPhase      = phase;
    }

    void MovePlayer(int moveX, int moveZ)
    {
        // Move player and reset jump timer (only grab user's attention when they are not interacting)
        player.Move(moveX, moveZ);
        effects.playerJumpTime = 0.0f;
    }

    void ControlPlayer()
    {
        // Move player via keyboard input
        if (input.KeyDownRepeated(LLGL::Key::Left))
            MovePlayer(-1, 0);
        else if (input.KeyDownRepeated(LLGL::Key::Right))
            MovePlayer(+1, 0);
        else if (input.KeyDownRepeated(LLGL::Key::Up))
            MovePlayer(0, +1);
        else if (input.KeyDownRepeated(LLGL::Key::Down))
            MovePlayer(0, -1);

        // Move player alternatively via mouse or touch input
        if (input.KeyDown(LLGL::Key::LButton))
        {
            pointerStartPos         = input.GetMousePosition();
            pointerPlayerMovement   = true;
        }
        else if (input.KeyUp(LLGL::Key::LButton))
        {
            pointerPlayerMovement   = false;
        }

        if (pointerPlayerMovement)
        {
            const LLGL::Extent2D resolution = swapChain->GetResolution();
            const float pointerVecScale = pointerMoveScale / std::max<std::uint32_t>(resolution.width, resolution.height);

            const LLGL::Offset2D currentPointerPos  = input.GetMousePosition();
            const LLGL::Offset2D pointerDiff        = currentPointerPos - pointerStartPos;

            const Gs::Vector2f pointerVec
            {
                static_cast<float>(pointerDiff.x) * pointerVecScale,
                static_cast<float>(pointerDiff.y) * pointerVecScale
            };
            const Gs::Vector2f leanVec
            {
                static_cast<float>(player.leanDir[0]),
                static_cast<float>(-player.leanDir[1])
            };

            constexpr float leanMaxFactor   = 1.5f;
            constexpr float leanMinFactor   = leanMaxFactor * 0.1f;
            constexpr float leanAnimFactor  = 0.3f;

            const float pointerLen = (player.IsLeaning() ? std::abs(Gs::Dot(pointerVec, leanVec)) : pointerVec.Length());

            if (pointerLen >= leanMaxFactor)
            {
                // Move player into direction the player was leaning towards.
                // For pointer movement, don't stack up movements to avoid unintended movements.
                if (player.moveDirStack == 0)
                    MovePlayer(player.leanDir[0], player.leanDir[1]);
                pointerStartPos = currentPointerPos;
            }
            else if (pointerLen >= leanMinFactor)
            {
                // Lock leaning direction into place
                if (!player.IsLeaning())
                {
                    if (std::abs(pointerVec.x) > std::abs(pointerVec.y))
                    {
                        // Move horizontally
                        if (pointerVec.x > 0.0f)
                            player.Lean(+1, 0);
                        else
                            player.Lean(-1, 0);
                    }
                    else
                    {
                        // Move vertically
                        if (pointerVec.y > 0.0f)
                            player.Lean(0, -1);
                        else
                            player.Lean(0, +1);
                    }
                }

                // Compute leaning factor for animation
                player.leanFactor = leanAnimFactor * ((pointerLen - leanMinFactor) / (leanMaxFactor - leanMinFactor));
            }
            else if (player.IsLeaning())
            {
                // Reset leaning direction
                player.leanDir[0] = 0;
                player.leanDir[1] = 0;
                pointerStartPos = currentPointerPos;
            }
        }
    }

    void PollUserInput()
    {
        if (player.moveDirStack < inputStackSize && !player.isDescending && !player.isFalling && !player.isExploding)
            ControlPlayer();

        #if ENABLE_CHEATS

        if (input.KeyDown(LLGL::Key::PageUp))
            SelectLevel(currentLevelIndex + 1);
        else if (input.KeyDown(LLGL::Key::PageDown))
            SelectLevel(currentLevelIndex - 1);

        if (input.KeyPressed(LLGL::Key::RButton))
        {
            const float delta = static_cast<float>(input.GetMouseMotion().x) * 0.01f;
            SetLightPhase(effects.lightPhase + delta);
            effects.lightPhaseChanged = false;
        }
            
        #endif // /ENABLE_CHEATS
    }

    void UpdateScene(float dt)
    {
        // Update user input, but not while transitioning
        if (nextLevel == nullptr)
            PollUserInput();

        // Get information from current level
        if (currentLevel != nullptr)
        {
            camera.FocusOnLevel(*currentLevel);

            // Transition to next level if one is selected
            if (nextLevel != nullptr)
            {
                // Transition from current to next level
                levelTransition += dt / levelTransitionSpeed;
                camera.TransitionBetweenLevels(*currentLevel, *nextLevel, levelTransition);

                // Finish transition interpolation reached the end of [0..1] interval
                if (levelTransition >= 1.0f)
                {
                    levelTransition             = 0.0f;
                    currentLevelInstanceOffset  = nextLevelInstanceOffset;
                    currentLevel                = nextLevel;
                    nextLevel                   = nullptr;

                    camera.FocusOnLevel(*currentLevel);
                }
            }
            else if (currentLevel->IsCompleted())
            {
                // Wait until next level is selected
                levelDoneTransition += dt / levelDoneSpeed;
                if (levelDoneTransition >= 1.0f)
                {
                    SelectLevel(currentLevelIndex + 1);
                    levelDoneTransition = 0.0f;
                }
            }
        }

        // Update player transformation
        InstanceMatrixType& wMatrixPlayer = *reinterpret_cast<InstanceMatrixType*>(player.instance.wMatrix);

        Movement movement = Movement::Free;

        if (player.moveDirStack > 0 && !player.isFalling && !(currentLevel != nullptr && currentLevel->IsCompleted()))
        {
            const int moveStackPos = player.moveDirStack - 1;

            int nextPosX = player.gridPos[0] + player.moveDir[moveStackPos][0];
            int nextPosY = player.gridPos[1] + player.moveDir[moveStackPos][1];

            // Block player from moving when hitting a wall or already activated tile
            movement = player.GetMovability(currentLevel, player.moveDir[moveStackPos][0], player.moveDir[moveStackPos][1]);
            if (movement != Movement::Free)
            {
                // Block player from moving when hitting a wall or already activated tile
                nextPosX = player.gridPos[0];
                nextPosY = player.gridPos[1];
            }

            player.moveTransition += (dt / playerMoveSpeed) * static_cast<float>(player.moveDirStack);
            if (player.moveTransition >= 1.0f)
            {
                // Perform tile action (activate tile/ player fall/ win)
                if (currentLevel != nullptr)
                {
                    if (player.gridPos[0] != nextPosX ||
                        player.gridPos[1] != nextPosY)
                    {
                        // Activate tile and start warp effect when level has been completed
                        if (currentLevel->IsTileHole(nextPosX, nextPosY))
                        {
                            player.isFalling = true;
                        }
                        else if (currentLevel->ActivateTile(nextPosX, nextPosY, playerColor))
                        {
                            // Finished level -> start warp effect
                            if (currentLevel->IsCompleted())
                                StartWarpEffect();
                            else if (currentLevel->IsPlayerBlocked(nextPosX, nextPosY))
                                StartJitterEffect();
                        }
                    }
                }

                // Finish player movement transition
                player.moveTransition   = 0.0f;
                player.gridPos[0]       = nextPosX;
                player.gridPos[1]       = nextPosY;
                player.moveDirStack--;

                // Cancel remaining movements if they are also blocked in the same direction
                while (movement != Movement::Free &&
                    player.moveDirStack > 0 &&
                    player.moveDir[moveStackPos][0] == player.moveDir[player.moveDirStack - 1][0] &&
                    player.moveDir[moveStackPos][1] == player.moveDir[player.moveDirStack - 1][1])
                {
                    player.moveDirStack--;
                }
            }
        }

        if (!pointerPlayerMovement && player.IsLeaning())
        {
            // Cancel leaning player
            if (player.leanFactor > 0.0f)
            {
                player.leanFactor -= dt / playerLeanBackSpeed;
            }
            else
            {
                player.leanDir[0] = 0;
                player.leanDir[1] = 0;
            }
        }

        if (player.isDescending)
        {
            // Animate player descending downards into place
            player.descendPhase += dt / playerDescendDuration;
            if (player.descendPhase >= 1.0f)
            {
                // Stop descending and reset player transparency
                player.descendPhase         = 0.0f;
                player.isDescending         = false;
                player.instance.color[3]    = 1.0f;
                SetPlayerTransform(wMatrixPlayer, player.gridPos, 0, 0, wallPosY, 0.0f);
            }
            else
            {
                SetPlayerTransformSpin(wMatrixPlayer, player.instance.color[3], player.gridPos, player.descendPhase);
            }
        }
        else if (player.isFalling)
        {
            // Fall animation
            player.fallVelocity += dt * playerFallAcceleration;
            player.fallDepth += player.fallVelocity;
            const float playerPosY = wallPosY - player.fallDepth;
            SetPlayerTransform(wMatrixPlayer, player.gridPos, 0, 0, playerPosY, 0.0f);

            if (player.hasFadedAway)
            {
                player.fadeAwayTime += dt;
                if (player.fadeAwayTime > playerFadeAwayDuration)
                {
                    // Restart current level
                    SelectLevel(currentLevelIndex);
                }
            }
            else
            {
                if (playerPosY < -2.0f)
                {
                    StartWarpEffect(2.0f, 4, 5.0f, 5.0f);
                    player.hasFadedAway = true;
                }
            }
        }
        else if (player.moveDirStack > 0)
        {
            const int moveStackPos  = player.moveDirStack - 1;
            const int moveDirX = player.moveDir[moveStackPos][0];
            const int moveDirY = player.moveDir[moveStackPos][1];
            if (movement != Movement::Free)
            {
                if (movement == Movement::BlockedByWall)
                {
                    // If movement is blocked by a wall, make the player bounce back.
                    const int oppositePosX = player.gridPos[0] - moveDirX;
                    const int oppositePosY = player.gridPos[1] - moveDirY;
                    if (currentLevel != nullptr && !currentLevel->IsWall(oppositePosX, oppositePosY))
                    {
                        // Animate player to bounce off the wall
                        SetPlayerTransformBounce(wMatrixPlayer, player.gridPos, -moveDirX, -moveDirY, wallPosY, player.moveTransition);
                    }
                    else
                    {
                        // Player is completely blocked, no animation
                        SetPlayerTransform(wMatrixPlayer, player.gridPos, 0, 0, wallPosY, 0.0f);
                    }
                }
                else if (movement == Movement::BlockedByTile)
                {
                    // Animate player to bounce forward
                    SetPlayerTransformBounce(wMatrixPlayer, player.gridPos, moveDirX, moveDirY, wallPosY, player.moveTransition);
                }
            }
            else
            {
                // Animate player turn over
                SetPlayerTransform(wMatrixPlayer, player.gridPos, moveDirX, moveDirY, wallPosY, player.moveTransition);
            }
        }
        else if (player.IsLeaning())
        {
            movement = player.GetMovability(currentLevel, player.leanDir[0], player.leanDir[1]);
            if (movement != Movement::BlockedByWall)
            {
                // Animate player leaning into a direction before the player is moving
                SetPlayerTransform(wMatrixPlayer, player.gridPos, player.leanDir[0], player.leanDir[1], wallPosY, player.leanFactor);

                // When player movement is animated again, continue with transition where the leaning left of to have a smooth transition between leaning and moving
                player.moveTransition = player.leanFactor;
            }
            else
            {
                // No player animation
                SetPlayerTransform(wMatrixPlayer, player.gridPos, 0, 0, wallPosY, 0.0f);
            }
        }
        else
        {
            // No player animation
            SetPlayerTransform(wMatrixPlayer, player.gridPos, 0, 0, wallPosY, 0.0f);
        }

        // Apply jitter effect when player lost the level
        if (effects.jitterEnabled)
        {
            effects.jitterTime += dt;
            if (effects.jitterTime > jitterDelay)
            {
                const float effectPhase = (effects.jitterTime - jitterDelay) / jitterDuration;
                float explodePhase = 0.0f;

                if (!player.isExploding)
                {
                    // Update jitter rotation with random numbers
                    const float maxAngle = Gs::Deg2Rad(effectPhase * jitterMaxAngle);
                    effects.jitterRotation[0] = rng.Next() * maxAngle;
                    effects.jitterRotation[1] = rng.Next() * maxAngle;
                    effects.jitterRotation[2] = rng.Next() * maxAngle;

                    if (effectPhase >= 1.0f)
                        player.isExploding = true;
                }
                else
                {
                    explodePhase = std::pow(effectPhase - 1.0f, playerExplodePow);
                    if (effectPhase >= 2.0f)
                    {
                        // Restart current level and stop jitter effect
                        effects.jitterEnabled = false;
                        SelectLevel(currentLevelIndex);
                    }
                }

                // Update player rotation with jitter effect
                Gs::RotateFree(wMatrixPlayer, Gs::Vector3f{ 1, 0, 0 }, effects.jitterRotation[0]);
                Gs::RotateFree(wMatrixPlayer, Gs::Vector3f{ 0, 1, 0 }, effects.jitterRotation[1]);
                Gs::RotateFree(wMatrixPlayer, Gs::Vector3f{ 0, 0, 1 }, effects.jitterRotation[2]);

                const float playerScale = 1.0f + explodePhase * playerExplodeScale;
                Gs::Scale(wMatrixPlayer, Gs::Vector3f{ playerScale });

                // Fade player away with explosion
                player.instance.color[3] = std::max<float>(0.0f, 1.0f - explodePhase * 1.5f);
            }
        }

        // Apply warp effect around player position
        if (effects.warpEnabled)
        {
            effects.warpTime += dt / effects.warpDuration;
            float maxWarpIntensity = (1.0f - effects.warpTime) * effects.warpMaxIntensity;
            if (maxWarpIntensity > 0.0f)
            {
                scene.warpIntensity = std::sin(effects.warpTime * Gs::pi * 2.0f * static_cast<float>(effects.warpBounces)) * maxWarpIntensity;
            }
            else
            {
                effects.warpEnabled = false;
                scene.warpIntensity = 0.0f;
            }
        }

        // Animate tree rotation
        effects.treeBendTime = std::fmod(effects.treeBendTime + dt / treeAnimSpeed, 1.0f);
        const float treeBendAngle = effects.treeBendTime * Gs::pi * 2.0f;
        scene.bendDir.x = std::sin(treeBendAngle) * treeAnimRadius;
        scene.bendDir.z = std::cos(treeBendAngle) * treeAnimRadius * 0.5f;

        // Animate player to get user's attention, so they know what block represents the player
        if (effects.playerJumpEnabled)
        {
            effects.playerJumpPhase += dt / playerJumpDuration;
            if (effects.playerJumpPhase >= 1.0f)
                effects.playerJumpEnabled = false;
        }
        else
        {
            effects.playerJumpTime += dt;
            if (effects.playerJumpTime > playerJumpWait)
            {
                effects.playerJumpEnabled   = true;
                effects.playerJumpTime      = 0.0f;
                effects.playerJumpPhase     = 0.0f;
            }
        }

        // Progress light direction as time of day simulation
        if (effects.lightPhaseChanged)
        {
            if (effects.lightPhase < effects.lightPhaseTarget)
            {
                effects.lightPhase += dt / timeOfDayChangeSpeed;
                if (!(effects.lightPhase < effects.lightPhaseTarget))
                    effects.lightPhaseChanged = false;
            }
            else if (effects.lightPhase > effects.lightPhaseTarget)
            {
                effects.lightPhase -= dt / timeOfDayChangeSpeed;
                if (!(effects.lightPhase > effects.lightPhaseTarget))
                    effects.lightPhaseChanged = false;
            }
            SetLightPhase(effects.lightPhase);
        }
    }

    void RenderLevel(const Level& level, float worldOffsetX, std::uint32_t instanceOffset)
    {
        worldTransform.worldOffset[0] = worldOffsetX;
        worldTransform.worldOffset[1] = 0.0f;
        worldTransform.worldOffset[2] = 0.0f;

        // Draw all tiles (floor and walls)
        worldTransform.bendIntensity = 0.0f;
        commands->SetUniforms(Uniform_worldOffset, &worldTransform, sizeof(worldTransform));

        const std::uint32_t numTiles = level.tileInstanceRange.Count();
        instanceBuffer.DrawInstances(*commands, mdlBlock.numVertices, mdlBlock.firstVertex, numTiles, 1 + instanceOffset);

        // Draw decor (trees)
        const std::uint32_t numTrees = level.treeInstanceRange.Count();
        if (numTrees > 0)
        {
            worldTransform.worldOffset[1] = -1.0f;
            worldTransform.bendIntensity = 0.5f;
            commands->SetUniforms(Uniform_worldOffset, &worldTransform, sizeof(worldTransform));

            instanceBuffer.DrawInstances(*commands, mdlTree.numVertices, mdlTree.firstVertex, numTrees, 1 + instanceOffset + numTiles);
        }
    }

    void RenderScene(bool renderShadowMap)
    {
        // Draw ground floor last, to cover the rest of the framebuffer
        if (!renderShadowMap)
        {
            commands->PushDebugGroup("Ground");
            {
                commands->SetPipelineState(*groundPSO);
                commands->SetResource(0, *cbufferScene);
                commands->SetResource(1, *groundColorMap);
                commands->SetResource(2, *groundColorMapSampler);
                commands->SetResource(3, *shadowMap);
                commands->SetResource(4, *shadowMapSampler);
                commands->Draw(mdlGround.numVertices, mdlGround.firstVertex);
            }
            commands->PopDebugGroup();
        }

        // Draw instanced meshes
        const int psoIndex = (renderShadowMap ? 1 : 0);

        commands->SetPipelineState(*scenePSO[psoIndex]);
        commands->SetResource(0, *cbufferScene);

        if (!renderShadowMap)
        {
            commands->SetResource(2, *shadowMap);
            commands->SetResource(3, *shadowMapSampler);
        }

        if (currentLevel != nullptr)
        {
            // Render current level
            commands->PushDebugGroup("CurrentLevel");
            {
                RenderLevel(*currentLevel, Gs::Lerp(0.0f, -levelDistance, levelTransition), currentLevelInstanceOffset);
            }
            commands->PopDebugGroup();

            // Render next level if there is one
            if (nextLevel != nullptr)
            {
                const std::uint32_t numInstancesCurrentLevel = static_cast<std::uint32_t>(currentLevel->meshInstances.size());
                commands->PushDebugGroup("NextLevel");
                {
                    RenderLevel(*nextLevel, Gs::Lerp(levelDistance, 0.0f, levelTransition), numInstancesCurrentLevel);
                }
                commands->PopDebugGroup();
            }
        }

        // Don't render shadow of player when the player is exploding
        if (!(renderShadowMap && player.isExploding))
        {
            // Draw player mesh
            commands->PushDebugGroup("Player");
            {
                // Always position player at the next level if there is one
                if (nextLevel == nullptr)
                {
                    worldTransform.worldOffset[0] = 0.0f;
                    worldTransform.worldOffset[2] = 0.0f;
                }
                worldTransform.worldOffset[1] = 0.0f;
                worldTransform.bendIntensity = 0.0f;
                commands->SetUniforms(Uniform_worldOffset, &worldTransform, sizeof(worldTransform));

                instanceBuffer.DrawInstances(*commands, mdlPlayer.numVertices, mdlPlayer.firstVertex, 1, 0);
            }
            commands->PopDebugGroup();
        }
    }

    void OnDrawFrame() override
    {
        // Update scene by user input
        timer.MeasureTime();
        UpdateScene(static_cast<float>(timer.GetDeltaTime()));

        commands->Begin();
        {
            // Bind common input assembly
            commands->SetVertexBuffer(*vertexBuffer);

            instanceBuffer.Update(*commands, 0, &player.instance, sizeof(player.instance));

            // Update mesh instances in dirty range
            if (currentLevel != nullptr)
            {
                const std::uint32_t numInstancesToUpdate = currentLevel->meshInstanceDirtyRange.Count();
                if (numInstancesToUpdate > 0)
                {
                    // Update mesh instance buffer
                    const std::uint32_t firstInstanceToUpdate = (1 + nextLevelInstanceOffset + currentLevel->meshInstanceDirtyRange.begin);

                    instanceBuffer.Update(
                        *commands,
                        sizeof(Instance) * firstInstanceToUpdate,
                        &(currentLevel->meshInstances[currentLevel->meshInstanceDirtyRange.begin]),
                        static_cast<std::uint16_t>(sizeof(Instance) * numInstancesToUpdate)
                    );

                    // Reset dirty range
                    currentLevel->meshInstanceDirtyRange.Invalidate();
                }
            }

            // Render shadow-map
            SetShadowMapView();
            commands->UpdateBuffer(*cbufferScene, 0, &scene, sizeof(scene));

            commands->BeginRenderPass(*shadowMapTarget);
            {
                // Clear depth buffer only, we will render the entire framebuffer
                commands->Clear(LLGL::ClearFlags::Depth);
                commands->SetViewport(shadowMapTarget->GetResolution());
                commands->PushDebugGroup("RenderShadowMap");
                {
                    RenderScene(true);
                }
                commands->PopDebugGroup();
            }
            commands->EndRenderPass();

            // Render everything directly into the swap-chain
            SetCameraView();
            commands->UpdateBuffer(*cbufferScene, 0, &scene, sizeof(scene));

            commands->BeginRenderPass(*swapChain);
            {
                // Clear depth buffer only, we will render the entire framebuffer
                commands->Clear(LLGL::ClearFlags::Depth);
                commands->SetViewport(swapChain->GetResolution());
                commands->PushDebugGroup("RenderScene");
                {
                    RenderScene(false);
                }
                commands->PopDebugGroup();
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);
    }

};

// Clang/GCC need these declared here as well prior to C++17
constexpr float  Example_HelloGame::levelTransitionSpeed   ;
constexpr float  Example_HelloGame::levelDoneSpeed         ;
constexpr float  Example_HelloGame::wallPosY               ;
constexpr int    Example_HelloGame::inputStackSize         ;
constexpr float  Example_HelloGame::playerColor[3]         ;
constexpr float  Example_HelloGame::treeColorGradient[2][3];
constexpr float  Example_HelloGame::treeAnimSpeed          ;
constexpr float  Example_HelloGame::treeAnimRadius         ;
constexpr int    Example_HelloGame::shadowMapSize          ;
constexpr float  Example_HelloGame::timeOfDayChangeSpeed   ;
constexpr float  Example_HelloGame::jitterDelay            ;
constexpr float  Example_HelloGame::jitterDuration         ;
constexpr float  Example_HelloGame::jitterMaxAngle         ;

constexpr float  Example_HelloGame::playerMoveSpeed        ;
constexpr float  Example_HelloGame::playerFallAcceleration ;
constexpr float  Example_HelloGame::playerJumpWait         ;
constexpr float  Example_HelloGame::playerJumpDuration     ;
constexpr float  Example_HelloGame::playerJumpHeight       ;
constexpr int    Example_HelloGame::playerJumpBounces      ;
constexpr float  Example_HelloGame::playerFadeAwayDuration ;
constexpr float  Example_HelloGame::playerDescendRotations ;
constexpr float  Example_HelloGame::playerDescendHeight    ;
constexpr float  Example_HelloGame::playerDescendDuration  ;
constexpr float  Example_HelloGame::playerLeanBackSpeed    ;
constexpr float  Example_HelloGame::playerExplodePow       ;
constexpr float  Example_HelloGame::playerExplodeScale     ;

constexpr std::uint32_t  Example_HelloGame::InstanceBuffer::cbufferNumInstances;

LLGL_IMPLEMENT_EXAMPLE(Example_HelloGame);



