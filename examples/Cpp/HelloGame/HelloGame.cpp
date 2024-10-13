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


// Enables cheats by allowing page up/down to select next or previous level
#define ENABLE_CHEATS 0


class Example_HelloGame : public ExampleBase
{

    static constexpr float  levelTransitionSpeed    = 0.5f; // in seconds
    static constexpr float  levelDoneSpeed          = 1.0f; // in seconds
    static constexpr float  playerMoveSpeed         = 0.25f; // in seconds
    static constexpr float  playerFallAcceleration  = 2.0f; // in units per seconds
    static constexpr float  warpEffectDuration      = 1.0f; // in seconds
    static constexpr int    warpEffectBounces       = 3;
    static constexpr float  warpEffectScale         = 2.0f;
    static constexpr float  wallPosY                = 2.0f;
    static constexpr int    inputStackSize          = 4;
    static constexpr float  playerColor[3]          = { 0.6f, 0.7f, 1.0f };

    LLGL::PipelineLayout*   psoLayoutScene          = nullptr;
    LLGL::PipelineState*    psoScene                = nullptr;
    ShaderPipeline          sceneShaders;

    LLGL::Buffer*           cbufferScene            = nullptr;
    LLGL::Buffer*           vertexBuffer            = nullptr;
    LLGL::Buffer*           instanceBuffer          = nullptr;
    std::uint32_t           instanceBufferCapacity  = 0; // Number of instances the instance buffer can hold

    LLGL::VertexFormat      vertexFormat;

    TriangleMesh            mdlPlayer;
    TriangleMesh            mdlBlock;
    TriangleMesh            mdlTree;

    struct alignas(16) Scene
    {
        Gs::Matrix4f    vpMatrix;
        Gs::Vector3f    lightDir        = Gs::Vector3f(-0.25f, -0.7f, 1.25f).Normalized();
        float           shininess       = 90.0f;                                            // Blinn-phong specular power factor
        Gs::Vector3f    viewPos;                                                        // World-space camera position
        float           pad0;
        Gs::Vector3f    warpCenter;
        float           warpIntensity   = 0.0f;
    }
    scene;

    struct alignas(16) Uniforms
    {
        float           worldOffset[3]  = { 0.0f, 0.0f, 0.0f };
        std::uint32_t   firstInstance   = 0;
    }
    uniforms;

    struct Vertex
    {
        float           position[3];
        float           normal[3];
        float           texCoord[2];
    };

    struct Instance
    {
        float           wMatrix[3][4];
        float           color[4];
    };

    // Decor for trees in the background
    struct Decor
    {

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
                *Get(x, y) = *tile;
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

    struct Level
    {
        std::string             name;
        LLGL::ColorRGBub        wallColors[2];
        TileGrid                floor;
        TileGrid                walls;
        std::vector<Instance>   meshInstances;
        std::uint32_t           meshInstanceDirtyRange[2]   = { ~0u, 0u };
        int                     gridSize[2]                 = {}; // Bounding box in grid coordinates
        int                     playerStart[2]              = {};
        float                   viewDistance                = 0.0f;
        int                     activatedTiles              = 0;
        int                     maxTilesToActivate          = 0;

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

        void InvalidateMeshInstances(std::uint32_t begin, std::uint32_t end)
        {
            meshInstanceDirtyRange[0] = std::min<std::uint32_t>(meshInstanceDirtyRange[0], begin);
            meshInstanceDirtyRange[1] = std::max<std::uint32_t>(meshInstanceDirtyRange[1], end);
        }

        void InvalidateMeshInstance(std::uint32_t index)
        {
            InvalidateMeshInstances(index, index + 1);
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
                    InvalidateMeshInstance(tile->instanceIndex);
                    ++activatedTiles;
                    return IsCompleted();
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
            levelCenterPos.x = static_cast<float>(level.gridSize[0]) - 2.0f;
            levelCenterPos.y = static_cast<float>(level.gridSize[1]) - 2.0f;
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

    struct Player
    {
        Instance    instance                    = {};
        int         gridPos[2]                  = {};
        int         moveDirStack                = 0;
        int         moveDir[inputStackSize][2]  = {};
        float       moveTransition              = 0.0f;
        bool        isFalling                   = false;
        float       fallDepth                   = 0.0f;
        float       fallVelocity                = 0.0f;

        void Move(int moveX, int moveZ)
        {
            if (moveDirStack < inputStackSize)
            {
                for_subrange_reverse(i, 1, moveDirStack + 1)
                {
                    moveDir[i][0] = moveDir[i - 1][0];
                    moveDir[i][1] = moveDir[i - 1][1];
                }
                moveDir[0][0] = moveX;
                moveDir[0][1] = moveZ;
                ++moveDirStack;
            }
        }

        void Put(const int (&pos)[2])
        {
            // Reset all player states and place onto grid position
            gridPos[0]      = pos[0];
            gridPos[1]      = pos[1];
            moveDirStack    = 0;
            moveTransition  = 0.0f;
            isFalling       = false;
            fallDepth       = 0.0f;
            fallVelocity    = 0.0f;
        }
    }
    player;

    std::vector<Level>  levels;
    int                 currentLevelIndex   = -1;
    Level*              currentLevel        = nullptr;
    Level*              nextLevel           = nullptr;
    float               levelTransition     = 0.0f; // Transitioning state between two levesl - in the range [0, 1]
    float               levelDistance       = 0.0f; // Distance between two levels (to transition between them)
    std::uint32_t       levelInstanceOffset = 0;
    float               levelDoneTransition = 0.0f; // Transition starting when the level is completed

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
        bool    warpEnabled = false;
        float   warpTime    = 0.0f;

        void StartWarp()
        {
            warpEnabled = true;
            warpTime    = 0.0f;
        }
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

        // Create vertex, index, and constant buffer
        vertexBuffer    = CreateVertexBuffer(vertices, vertexFormat);
        cbufferScene    = CreateConstantBuffer(scene);

        return vertexFormat;
    }

    void CreateInstanceBuffer(std::uint32_t numInstances)
    {
        // Check if the buffer must be resized
        if (numInstances <= instanceBufferCapacity)
            return;

        // Release previous buffers
        if (instanceBuffer != nullptr)
            renderer->Release(*instanceBuffer);

        // Create instance buffer from mesh instance data
        LLGL::BufferDescriptor instanceBufferDesc;
        {
            instanceBufferDesc.debugName    = "InstanceBuffer";
            instanceBufferDesc.size         = sizeof(Instance) * numInstances;
            instanceBufferDesc.stride       = sizeof(Instance);
            instanceBufferDesc.bindFlags    = LLGL::BindFlags::Sampled;
        }
        instanceBuffer = renderer->CreateBuffer(instanceBufferDesc);
        instanceBufferCapacity = numInstances;
    }

    void CreateShaders(const LLGL::VertexFormat& vertexFormat)
    {
        if (Supported(LLGL::ShadingLanguage::GLSL))
        {
            sceneShaders.vs = LoadShader({ LLGL::ShaderType::Vertex,     "HelloGame.vert" }, { vertexFormat });
            sceneShaders.ps = LoadShader({ LLGL::ShaderType::Fragment,   "HelloGame.frag" });
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            sceneShaders.vs = LoadShader({ LLGL::ShaderType::Vertex,     "HelloGame.450core.vert.spv" }, { vertexFormat });
            sceneShaders.ps = LoadShader({ LLGL::ShaderType::Fragment,   "HelloGame.450core.frag.spv" });
        }
        else if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            sceneShaders.vs = LoadShader({ LLGL::ShaderType::Vertex,     "HelloGame.hlsl", "VSMain", "vs_5_0" }, { vertexFormat });
            sceneShaders.ps = LoadShader({ LLGL::ShaderType::Fragment,   "HelloGame.hlsl", "PSMain", "ps_5_0" });
        }
        else if (Supported(LLGL::ShadingLanguage::Metal))
        {
            sceneShaders.vs = LoadShader({ LLGL::ShaderType::Vertex,     "HelloGame.metal", "VSMain", "vs_5_0" }, { vertexFormat });
            sceneShaders.ps = LoadShader({ LLGL::ShaderType::Fragment,   "HelloGame.metal", "PSMain", "ps_5_0" });
        }
    }

    void CreatePipelines()
    {
        // Create pipeline layout
        psoLayoutScene = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(Scene@1):vert:frag,"
                "buffer(instances@2):vert,"
                "float3(worldOffset),"
                "uint(firstInstance),"
            )
        );

        // Create graphics pipeline for scene rendering
        LLGL::GraphicsPipelineDescriptor psoSceneDesc;
        {
            psoSceneDesc.vertexShader                   = sceneShaders.vs;
            psoSceneDesc.fragmentShader                 = sceneShaders.ps;
            psoSceneDesc.renderPass                     = swapChain->GetRenderPass();
            psoSceneDesc.pipelineLayout                 = psoLayoutScene;
            psoSceneDesc.depth.testEnabled              = true;
            psoSceneDesc.depth.writeEnabled             = true;
            psoSceneDesc.rasterizer.cullMode            = LLGL::CullMode::Back;
            psoSceneDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
        }
        psoScene = renderer->CreatePipelineState(psoSceneDesc);
        ReportPSOErrors(psoScene);
    }

    static Gs::Vector3f GridPosToWorldPos(const int (&gridPos)[2], float posY)
    {
        const float posX = static_cast<float>(gridPos[0])*2.0f;
        const float posZ = static_cast<float>(gridPos[1])*2.0f;
        return Gs::Vector3f{ posX, posY, posZ };
    }

    static void RotateAroundPivot(Gs::AffineMatrix4f& outMatrix, const Gs::Vector3f& pivot, const Gs::Vector3f& axis, float angle)
    {
        Gs::Matrix3f rotation;
        Gs::RotateFree(rotation, axis, angle);
        const Gs::Vector3f offset = rotation * pivot;

        Gs::Translate(outMatrix, pivot - offset);
        Gs::RotateFree(outMatrix, axis, angle);
    }

    void SetPlayerTransform(Gs::AffineMatrix4f& outMatrix, const int (&gridPosA)[2], int moveX, int moveZ, float posY, float transition)
    {
        outMatrix.LoadIdentity();

        const Gs::Vector3f posA = GridPosToWorldPos(gridPosA, posY);
        Gs::Translate(outMatrix, posA);

        if (transition > 0.0f)
        {
            const float angle = Gs::SmoothStep(transition) * Gs::pi * 0.5f;
            if (moveX < 0)
            {
                // Move left
                RotateAroundPivot(outMatrix, Gs::Vector3f{ -1,-1, 0 }, Gs::Vector3f{ 0,0,1 }, -angle);
            }
            else if (moveX > 0)
            {
                // Move right
                RotateAroundPivot(outMatrix, Gs::Vector3f{ +1,-1, 0 }, Gs::Vector3f{ 0,0,1 }, +angle);
            }
            else if (moveZ < 0)
            {
                // Move forwards
                RotateAroundPivot(outMatrix, Gs::Vector3f{  0,-1,-1 }, Gs::Vector3f{ 1,0,0 }, +angle);
            }
            else if (moveZ > 0)
            {
                // Move backwards
                RotateAroundPivot(outMatrix, Gs::Vector3f{  0,-1,+1 }, Gs::Vector3f{ 1,0,0 }, -angle);
            }
        }
    }

    void SetTileInstance(Instance& instance, const int (&gridPos)[2], float posY, const Gradient* gradient = nullptr)
    {
        Gs::AffineMatrix4f& wMatrix = *reinterpret_cast<Gs::AffineMatrix4f*>(instance.wMatrix);
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

    void GenerateTileInstances(TileGrid& grid, std::vector<Instance>& meshInstances, std::uint32_t& tileCounter, float posY, const Gradient* gradient)
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
                    tile.instanceIndex = tileCounter++;
                    SetTileInstance(meshInstances[tile.instanceIndex], gridPos, posY, gradient);
                }
            }
        }
    }

    void FinalizeLevel(Level& level)
    {
        // Build instance data from tiles
        level.meshInstances.resize(level.floor.CountTiles() + level.walls.CountTiles());

        // Colorize wall tiles with gradient
        Gradient wallGradient;
        wallGradient.colors[0] = level.wallColors[0].Cast<float>();
        wallGradient.colors[1] = level.wallColors[1].Cast<float>();
        wallGradient.points[0] = Gs::Vector3f{ 0.0f, wallPosY, 0.0f };
        wallGradient.points[1] = Gs::Vector3f{ static_cast<float>(level.gridSize[0])*2.0f, wallPosY, static_cast<float>(level.gridSize[1])*2.0f };

        std::uint32_t tileCounter = 0;
        GenerateTileInstances(level.floor, level.meshInstances, tileCounter, 0.0f, nullptr);
        GenerateTileInstances(level.walls, level.meshInstances, tileCounter, wallPosY, &wallGradient);
    }

    void LoadLevels()
    {
        // Load level files
        const std::vector<std::string> levelsFileLines = ReadTextLines("HelloGame.levels.txt");

        std::string name;
        std::string wallGradient;
        std::vector<std::string> currentGrid;

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
                ((color >> 16) & 0xFF),
                ((color >>  8) & 0xFF),
                ((color      ) & 0xFF)
            };
        };

        auto FlushLevelConstruct = [&]() -> void
        {
            if (currentGrid.empty())
                return;

            // Construct a new level
            Level newLevel;

            newLevel.name = (name.empty() ? "Unnamed" : name);

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
            newLevel.gridSize[0] = 0;
            newLevel.gridSize[1] = static_cast<int>(currentGrid.size());

            for (const std::string& row : currentGrid)
                newLevel.gridSize[0] = std::max<int>(newLevel.gridSize[0], static_cast<int>(row.size()));

            newLevel.viewDistance = static_cast<float>(std::max<int>(newLevel.gridSize[0], newLevel.gridSize[1])) * 2.7f;

            // Build grid of tiles row by row by interpreting characters from the level text file
            Tile initialTile;
            initialTile.instanceIndex = 0;

            int gridPosY = newLevel.gridSize[1];
            for (const std::string& row : currentGrid)
            {
                --gridPosY;
                int gridPosX = 0;

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

        if (currentLevelIndex == index)
        {
            // Level unchanged
            return;
        }

        // Update instance buffer
        const std::uint64_t playerBufferSize = sizeof(Instance);
        if (currentLevel != nullptr)
        {
            // Select next level to transition to
            nextLevel = &levels[index];
            levelDistance = static_cast<float>(currentLevel->gridSize[0] + nextLevel->gridSize[0]) * 1.5f;

            // Position player
            nextLevel->ResetTiles();
            nextLevel->PutPlayer(player);

            // Update instance buffer from current and next level instance data plus one instance for the player model
            CreateInstanceBuffer(1 + static_cast<std::uint32_t>(currentLevel->meshInstances.size() + nextLevel->meshInstances.size()));

            const std::uint64_t instanceBufferSize0 = sizeof(Instance) * currentLevel->meshInstances.size();
            const std::uint64_t instanceBufferSize1 = sizeof(Instance) * nextLevel   ->meshInstances.size();

            renderer->WriteBuffer(*instanceBuffer, playerBufferSize                      , currentLevel->meshInstances.data(), instanceBufferSize0);
            renderer->WriteBuffer(*instanceBuffer, playerBufferSize + instanceBufferSize0, nextLevel   ->meshInstances.data(), instanceBufferSize1);
        }
        else
        {
            // Select first level
            currentLevel = &levels[index];
            levelDistance = 0.0f;

            // Position player
            currentLevel->ResetTiles();
            currentLevel->PutPlayer(player);

            // Update instance buffer from current level instance data plus one instance for the player model
            CreateInstanceBuffer(1 + static_cast<std::uint32_t>(currentLevel->meshInstances.size()));
            renderer->WriteBuffer(*instanceBuffer, playerBufferSize, currentLevel->meshInstances.data(), sizeof(Instance) * currentLevel->meshInstances.size());
        }

        // Store index to current level to conveneintly selecting next and previous levels
        currentLevelIndex = index;
        levelInstanceOffset = 0;
    }

    void UpdateScene(float dt)
    {
        // Update user input, but not while transitioning
        if (nextLevel == nullptr)
        {
            if (player.moveDirStack < inputStackSize)
            {
                if (input.KeyDownRepeated(LLGL::Key::Left))
                    player.Move(-1, 0);
                else if (input.KeyDownRepeated(LLGL::Key::Right))
                    player.Move(+1, 0);
                else if (input.KeyDownRepeated(LLGL::Key::Up))
                    player.Move(0, +1);
                else if (input.KeyDownRepeated(LLGL::Key::Down))
                    player.Move(0, -1);
            }
            #if ENABLE_CHEATS
            if (input.KeyDown(LLGL::Key::PageUp))
                SelectLevel(currentLevelIndex + 1);
            else if (input.KeyDown(LLGL::Key::PageDown))
                SelectLevel(currentLevelIndex - 1);
            #endif
        }

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
                    levelTransition     = 0.0f;
                    levelInstanceOffset = static_cast<std::uint32_t>(currentLevel->meshInstances.size());
                    currentLevel        = nextLevel;
                    nextLevel           = nullptr;

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

        // Update view transformation
        scene.vpMatrix.LoadIdentity();
        Gs::Translate(scene.vpMatrix, { camera.levelCenterPos.x, 0, camera.levelCenterPos.y });
        Gs::RotateFree(scene.vpMatrix, { 1, 0, 0 }, Gs::Deg2Rad(-65.0f));
        Gs::Translate(scene.vpMatrix, { 0, 0, -camera.viewDistance });
        scene.viewPos = Gs::TransformVector(scene.vpMatrix, Gs::Vector3f{ 0, 0, 0 });
        scene.vpMatrix.MakeInverse();
        scene.vpMatrix = projection * scene.vpMatrix;

        // Update player transformation
        Gs::AffineMatrix4f& wMatrixPlayer = *reinterpret_cast<Gs::AffineMatrix4f*>(player.instance.wMatrix);

        bool isMovementBlocked = false;

        if (player.moveDirStack > 0 && !player.isFalling && !(currentLevel != nullptr && currentLevel->IsCompleted()))
        {
            const int moveStackPos = player.moveDirStack - 1;

            int nextPosX = player.gridPos[0] + player.moveDir[moveStackPos][0];
            int nextPosY = player.gridPos[1] + player.moveDir[moveStackPos][1];

            if (currentLevel != nullptr)
            {
                if (currentLevel->IsTileBlocked(nextPosX, nextPosY))
                {
                    // Block player from moving when hitting a wall or already activated tile
                    nextPosX = player.gridPos[0];
                    nextPosY = player.gridPos[1];
                    isMovementBlocked = true;
                }
            }

            player.moveTransition += (dt / playerMoveSpeed) * static_cast<float>(player.moveDirStack);
            if (player.moveTransition >= 1.0f)
            {
                // Perform tile action
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
                            effects.StartWarp();
                        }
                    }
                }

                // Finish player movement transition
                player.moveTransition   = 0.0f;
                player.gridPos[0]       = nextPosX;
                player.gridPos[1]       = nextPosY;
                player.moveDirStack--;

                // Cancel remaining movements if they are also blocked in the same direction
                while (isMovementBlocked &&
                    player.moveDirStack > 0 &&
                    player.moveDir[moveStackPos][0] == player.moveDir[player.moveDirStack - 1][0] &&
                    player.moveDir[moveStackPos][1] == player.moveDir[player.moveDirStack - 1][1])
                {
                    player.moveDirStack--;
                }
            }
        }

        if (player.isFalling)
        {
            // Fall animation
            if (player.fallDepth < 100.0f)
            {
                player.fallVelocity += dt * playerFallAcceleration;
                player.fallDepth += player.fallVelocity;
                SetPlayerTransform(wMatrixPlayer, player.gridPos, 0, 0, wallPosY - player.fallDepth, 0.0f);
            }
        }
        else if (player.moveDirStack > 0)
        {
            const int moveStackPos  = player.moveDirStack - 1;
            const int moveDirX      = player.moveDir[moveStackPos][0];
            const int moveDirY      = player.moveDir[moveStackPos][1];
            if (isMovementBlocked)
            {
                const int oppositePosX = player.gridPos[0] - moveDirX;
                const int oppositePosY = player.gridPos[1] - moveDirY;
                if (currentLevel != nullptr && !currentLevel->IsWall(oppositePosX, oppositePosY))
                {
                    // Animate player to bounce off the wall
                    const float bounceTransition = std::abs(std::sinf(player.moveTransition * Gs::pi * 2.0f)) * Gs::SmoothStep(1.0f - player.moveTransition * 0.5f) * 0.2f;
                    SetPlayerTransform(wMatrixPlayer, player.gridPos, -moveDirX, -moveDirY, wallPosY, bounceTransition);
                }
                else
                {
                    // Player is completely blocked, no animation
                    SetPlayerTransform(wMatrixPlayer, player.gridPos, 0, 0, wallPosY, 0.0f);
                }
            }
            else
            {
                // Animate player turn over
                SetPlayerTransform(wMatrixPlayer, player.gridPos, moveDirX, moveDirY, wallPosY, player.moveTransition);
            }
        }
        else
        {
            // No player animation
            SetPlayerTransform(wMatrixPlayer, player.gridPos, 0, 0, wallPosY, 0.0f);
        }

        // Apply warp effect around player position
        if (effects.warpEnabled)
        {
            effects.warpTime += dt / warpEffectDuration;
            float maxWarpIntensity = (1.0f - effects.warpTime) * warpEffectScale;
            if (maxWarpIntensity > 0.0f)
            {
                scene.warpCenter    = Gs::TransformVector(wMatrixPlayer, Gs::Vector3f{});
                scene.warpIntensity = std::sinf(effects.warpTime * Gs::pi * 2.0f * static_cast<float>(warpEffectBounces)) * maxWarpIntensity;
            }
            else
            {
                effects.warpEnabled = false;
                scene.warpIntensity = 0.0f;
            }
        }

        // Update player color
        player.instance.color[0] = playerColor[0];
        player.instance.color[1] = playerColor[1];
        player.instance.color[2] = playerColor[2];
        player.instance.color[3] = 1.0f;
    }

    void RenderScene()
    {
        commands->SetPipelineState(*psoScene);
        commands->SetResource(0, *cbufferScene);
        commands->SetResource(1, *instanceBuffer);

        // Draw all tile instances
        if (currentLevel != nullptr)
        {
            const std::uint32_t numTilesCurrentLevel = static_cast<std::uint32_t>(currentLevel->meshInstances.size());
            commands->PushDebugGroup("CurrentLevel");
            {
                uniforms.worldOffset[0] = Gs::Lerp(0.0f, -levelDistance, levelTransition);
                uniforms.worldOffset[1] = 0.0f;
                uniforms.worldOffset[2] = 0.0f;
                uniforms.firstInstance  = 1 + levelInstanceOffset;
                commands->SetUniforms(0, &uniforms, sizeof(uniforms));

                commands->DrawInstanced(mdlBlock.numVertices, mdlBlock.firstVertex, numTilesCurrentLevel);
            }
            commands->PopDebugGroup();

            if (nextLevel != nullptr)
            {
                const std::uint32_t numBlocksNextLevel = static_cast<std::uint32_t>(nextLevel->meshInstances.size());
                commands->PushDebugGroup("NextLevel");
                {
                    uniforms.worldOffset[0] = Gs::Lerp(levelDistance, 0.0f, levelTransition);
                    uniforms.worldOffset[1] = 0.0f;
                    uniforms.worldOffset[2] = 0.0f;
                    uniforms.firstInstance  = 1 + numTilesCurrentLevel;
                    commands->SetUniforms(0, &uniforms, sizeof(uniforms));

                    commands->DrawInstanced(mdlBlock.numVertices, mdlBlock.firstVertex, numBlocksNextLevel);
                }
                commands->PopDebugGroup();
            }
        }

        // Draw player mesh, but not while transitioning
        commands->PushDebugGroup("Player");
        {
            if (nextLevel == nullptr)
            {
                uniforms.worldOffset[0] = 0.0f;
                uniforms.worldOffset[1] = 0.0f;
                uniforms.worldOffset[2] = 0.0f;
            }
            uniforms.firstInstance  = 0;

            commands->SetUniforms(0, &uniforms, sizeof(uniforms));

            commands->Draw(mdlPlayer.numVertices, mdlPlayer.firstVertex);
        }
        commands->PopDebugGroup();
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
            commands->UpdateBuffer(*cbufferScene, 0, &scene, sizeof(scene));
            commands->UpdateBuffer(*instanceBuffer, 0, &player.instance, sizeof(player.instance));

            // Update mesh instances in dirty range
            if (currentLevel != nullptr)
            {
                if (currentLevel->meshInstanceDirtyRange[0] < currentLevel->meshInstanceDirtyRange[1])
                {
                    // Update mesh instance buffer
                    const std::uint32_t firstInstanceToUpdate = (1 + levelInstanceOffset + currentLevel->meshInstanceDirtyRange[0]);
                    const std::uint32_t numInstancesToUpdate = (currentLevel->meshInstanceDirtyRange[1] - currentLevel->meshInstanceDirtyRange[0]);

                    commands->UpdateBuffer(
                        *instanceBuffer,
                        sizeof(Instance) * firstInstanceToUpdate,
                        &(currentLevel->meshInstances[currentLevel->meshInstanceDirtyRange[0]]),
                        static_cast<std::uint16_t>(sizeof(Instance) * numInstancesToUpdate)
                    );

                    // Reset dirty range
                    currentLevel->meshInstanceDirtyRange[0] = ~0u;
                    currentLevel->meshInstanceDirtyRange[1] = 0u;
                }
            }

            // Render everything directly into the swap-chain
            commands->BeginRenderPass(*swapChain);
            {
                commands->Clear(LLGL::ClearFlags::ColorDepth, backgroundColor);
                commands->SetViewport(swapChain->GetResolution());
                commands->PushDebugGroup("RenderScene");
                {
                    RenderScene();
                }
                commands->PopDebugGroup();
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_HelloGame);



