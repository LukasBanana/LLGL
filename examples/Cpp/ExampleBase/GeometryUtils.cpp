/*
 * GeometryUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GeometryUtils.h"
#include "FileUtils.h"
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <sstream>
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Trap.h>


/*
* Global helper functions
*/

std::vector<TexturedVertex> LoadObjModel(const std::string& filename)
{
    std::vector<TexturedVertex> vertices;
    LoadObjModel(vertices, filename);
    return vertices;
}

TriangleMesh LoadObjModel(std::vector<TexturedVertex>& vertices, const std::string& filename, unsigned verticesPerFace)
{
    // Read obj file
    std::vector<char> fileContent = ReadAsset(filename);
    if (fileContent.empty())
        LLGL_THROW_RUNTIME_ERROR("failed to load model from file: \"%s\"", filename.c_str());

    // Initialize triangle mesh
    TriangleMesh mesh;
    mesh.firstVertex = static_cast<std::uint32_t>(vertices.size());

    std::vector<Gs::Vector3f> coords, normals;
    std::vector<Gs::Vector2f> texCoords;

    // Convert file content into a stream (this code used to read the file via std::ifstream)
    std::stringstream stream;
    std::copy(fileContent.begin(), fileContent.end(), std::ostream_iterator<char>{ stream });

    // Read each line
    std::string line;
    while (std::getline(stream, line))
    {
        std::stringstream s;
        s << line;

        // Parse line
        std::string mode;
        s >> mode;

        if (mode == "v")
        {
            Gs::Vector3f v;
            s >> v.x;
            s >> v.y;
            s >> v.z;
            coords.push_back(v);
        }
        else if (mode == "vt")
        {
            Gs::Vector2f t;
            s >> t.x;
            s >> t.y;
            texCoords.push_back(t);
        }
        else if (mode == "vn")
        {
            Gs::Vector3f n;
            s >> n.x;
            s >> n.y;
            s >> n.z;
            normals.push_back(n);
        }
        else if (mode == "f")
        {
            unsigned int v = 0, vt = 0, vn = 0;

            for (unsigned i = 0; i < verticesPerFace; ++i)
            {
                // Read vertex index
                s >> v;

                // Read texture-coordinate index
                if (texCoords.empty())
                    s.ignore(2);
                else
                {
                    s.ignore(1);
                    s >> vt;
                    s.ignore(1);
                }

                // Read normal index
                s >> vn;

                // Add vertex to mesh
                vertices.push_back(
                    {
                        coords[v - 1],
                        (vn - 1 < normals.size() ? normals[vn - 1] : Gs::Vector3f{}),
                        (vt - 1 < texCoords.size() ? texCoords[vt - 1] : Gs::Vector2f{})
                    }
                );
                mesh.numVertices++;
            }
        }
    }

    return mesh;
}

std::vector<Gs::Vector3f> GenerateCubeVertices()
{
    return
    {
        { -1, -1, -1 }, { -1,  1, -1 }, {  1,  1, -1 }, {  1, -1, -1 },
        { -1, -1,  1 }, { -1,  1,  1 }, {  1,  1,  1 }, {  1, -1,  1 },
    };
}

std::vector<std::uint32_t> GenerateCubeTriangleIndices()
{
    return
    {
        0, 1, 2, 0, 2, 3, // front
        3, 2, 6, 3, 6, 7, // right
        4, 5, 1, 4, 1, 0, // left
        1, 5, 6, 1, 6, 2, // top
        4, 0, 3, 4, 3, 7, // bottom
        7, 6, 5, 7, 5, 4, // back
    };
}

std::vector<TexturedVertex> GenerateTexturedCubeVertices()
{
    return
    {
        //   x   y   z      nx  ny  nz      u  v
        // front
        { { -1, -1, -1 }, {  0,  0, -1 }, { 0, 1 } },
        { { -1,  1, -1 }, {  0,  0, -1 }, { 0, 0 } },
        { {  1,  1, -1 }, {  0,  0, -1 }, { 1, 0 } },
        { {  1, -1, -1 }, {  0,  0, -1 }, { 1, 1 } },

        // right
        { {  1, -1, -1 }, { +1,  0,  0 }, { 0, 1 } },
        { {  1,  1, -1 }, { +1,  0,  0 }, { 0, 0 } },
        { {  1,  1,  1 }, { +1,  0,  0 }, { 1, 0 } },
        { {  1, -1,  1 }, { +1,  0,  0 }, { 1, 1 } },

        // left
        { { -1, -1,  1 }, { -1,  0,  0 }, { 0, 1 } },
        { { -1,  1,  1 }, { -1,  0,  0 }, { 0, 0 } },
        { { -1,  1, -1 }, { -1,  0,  0 }, { 1, 0 } },
        { { -1, -1, -1 }, { -1,  0,  0 }, { 1, 1 } },

        // top
        { { -1,  1, -1 }, {  0, +1,  0 }, { 0, 1 } },
        { { -1,  1,  1 }, {  0, +1,  0 }, { 0, 0 } },
        { {  1,  1,  1 }, {  0, +1,  0 }, { 1, 0 } },
        { {  1,  1, -1 }, {  0, +1,  0 }, { 1, 1 } },

        // bottom
        { { -1, -1,  1 }, {  0, -1,  0 }, { 0, 1 } },
        { { -1, -1, -1 }, {  0, -1,  0 }, { 0, 0 } },
        { {  1, -1, -1 }, {  0, -1,  0 }, { 1, 0 } },
        { {  1, -1,  1 }, {  0, -1,  0 }, { 1, 1 } },

        // back
        { {  1, -1,  1 }, {  0,  0, +1 }, { 0, 1 } },
        { {  1,  1,  1 }, {  0,  0, +1 }, { 0, 0 } },
        { { -1,  1,  1 }, {  0,  0, +1 }, { 1, 0 } },
        { { -1, -1,  1 }, {  0,  0, +1 }, { 1, 1 } },
    };
}

std::vector<std::uint32_t> GenerateTexturedCubeTriangleIndices()
{
    return
    {
         0,  1,  2,  0,  2,  3, // front
         4,  5,  6,  4,  6,  7, // right
         8,  9, 10,  8, 10, 11, // left
        12, 13, 14, 12, 14, 15, // top
        16, 17, 18, 16, 18, 19, // bottom
        20, 21, 22, 20, 22, 23, // back
    };
}

std::vector<std::uint32_t> GenerateTexturedCubeQuadIndices(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    // Assume the vertices are already laid out as quads. Generate indices 0/1/3/2 for each quad.
    std::vector<std::uint32_t> indices;
    indices.resize(numVertices);

    for_range(i, numVertices)
    {
        switch (i % 4)
        {
            case 2:
                indices[i] = firstVertex + i + 1;
                break;
            case 3:
                indices[i] = firstVertex + i - 1;
                break;
            default:
                indices[i] = firstVertex + i;
                break;
        }
    }

    return indices;
}

static void CopyVertex(TangentSpaceVertex& dst, const TexturedVertex& src)
{
    dst.position = src.position;
    dst.normal   = src.normal;
    dst.texCoord = src.texCoord;
}

static void NormalizeTangents(TangentSpaceVertex& v, const Gs::Vector3f& tangent0, const Gs::Vector3f& tangent1)
{
    v.tangents[0] = Gs::Cross(v.normal, tangent1).Normalized();
    v.tangents[1] = Gs::Cross(v.normal, tangent0).Normalized();
}

static void GenerateTangentSpace(TangentSpaceVertex& v0, TangentSpaceVertex& v1, TangentSpaceVertex& v2)
{
    const Gs::Vector3f edge1 = v1.position - v0.position;
    const Gs::Vector3f edge2 = v2.position - v0.position;

    const Gs::Vector2f deltaUV1 = v1.texCoord - v0.texCoord;
    const Gs::Vector2f deltaUV2 = v2.texCoord - v0.texCoord;

    Gs::Vector3f tangent0 = edge1 * deltaUV2.y - edge2 * deltaUV1.y;
    Gs::Vector3f tangent1 = edge1 * deltaUV2.x - edge2 * deltaUV1.x;

    tangent0.Normalize();
    tangent1.Normalize();

    NormalizeTangents(v0, tangent0, tangent1);
    NormalizeTangents(v1, tangent0, tangent1);
    NormalizeTangents(v2, tangent0, tangent1);
}

std::vector<TangentSpaceVertex> GenerateTangentSpaceVertices(const LLGL::ArrayView<TexturedVertex>& vertices)
{
    std::vector<TangentSpaceVertex> outp;
    outp.resize(vertices.size());

    for (std::size_t i = 0, n = outp.size(); i + 3 <= n; i += 3)
    {
        // Copy position, normal, and texture-coordinate
        CopyVertex(outp[i    ], vertices[i    ]);
        CopyVertex(outp[i + 1], vertices[i + 1]);
        CopyVertex(outp[i + 2], vertices[i + 2]);

        // Generate tangent-space
        GenerateTangentSpace(outp[i], outp[i + 1], outp[i + 2]);
    }

    return outp;
}

static void GenerateTangentSpaceQuad(TangentSpaceVertex& v0, TangentSpaceVertex& v1, TangentSpaceVertex& v2, TangentSpaceVertex& v3)
{
    const Gs::Vector3f edge1 = v1.position - v0.position;
    const Gs::Vector3f edge2 = v2.position - v0.position;

    const Gs::Vector2f deltaUV1 = v1.texCoord - v0.texCoord;
    const Gs::Vector2f deltaUV2 = v2.texCoord - v0.texCoord;

    Gs::Vector3f tangent0 = edge1 * deltaUV2.y - edge2 * deltaUV1.y;
    Gs::Vector3f tangent1 = edge1 * deltaUV2.x - edge2 * deltaUV1.x;

    tangent0.Normalize();
    tangent1.Normalize();

    NormalizeTangents(v0, tangent0, tangent1);
    NormalizeTangents(v1, tangent0, tangent1);
    NormalizeTangents(v2, tangent0, tangent1);
    NormalizeTangents(v3, tangent0, tangent1);
}

std::vector<TangentSpaceVertex> GenerateTangentSpaceQuadVertices(const LLGL::ArrayView<TexturedVertex>& vertices)
{
    std::vector<TangentSpaceVertex> outp;
    outp.resize(vertices.size());

    for (std::size_t i = 0, n = outp.size(); i + 4 <= n; i += 4)
    {
        // Copy position, normal, and texture-coordinate
        CopyVertex(outp[i    ], vertices[i    ]);
        CopyVertex(outp[i + 1], vertices[i + 1]);
        CopyVertex(outp[i + 2], vertices[i + 2]);
        CopyVertex(outp[i + 3], vertices[i + 3]);

        // Generate tangent-space
        GenerateTangentSpaceQuad(outp[i], outp[i + 1], outp[i + 2], outp[i + 3]);
    }

    return outp;
}

Gs::Vector3f ClosestPointOnLineSegment(const Gs::Vector3f& linePointA, const Gs::Vector3f& linePointB, const Gs::Vector3f& referencePoint)
{
    Gs::Vector3f relativePoint = referencePoint - linePointA;
    Gs::Vector3f lineDirection = linePointB - linePointA;

    float lineLength = Gs::Length(lineDirection);
    lineDirection *= (1.0f / lineLength);

    float interpolation = Gs::Dot(lineDirection, relativePoint);
    if (interpolation <= 0.0f)
        return linePointA;
    if (interpolation >= lineLength)
        return linePointB;

    lineDirection *= interpolation;

    return linePointA + lineDirection;
}

