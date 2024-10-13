/*
 * GeometryUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGLEXAMPLES_GEOMETRY_UTILS_H
#define LLGLEXAMPLES_GEOMETRY_UTILS_H


#include <LLGL/LLGL.h>
#include <LLGL/Container/ArrayView.h>
#include <Gauss/Gauss.h>
#include <vector>


/*
 * Global helper structures
 */

struct TexturedVertex
{
    Gs::Vector3f position;
    Gs::Vector3f normal;
    Gs::Vector2f texCoord;
};

struct TangentSpaceVertex
{
    Gs::Vector3f position;
    Gs::Vector3f normal;
    Gs::Vector3f tangents[2];
    Gs::Vector2f texCoord;
};

//TODO: rename to ModelView, since this can also be used for quad primitives and does not contain any vertex data
struct TriangleMesh
{
    std::uint32_t       firstVertex = 0;
    std::uint32_t       numVertices = 0;
    Gs::Matrix4f        transform;
    LLGL::ColorRGBAf    color;
};


/*
 * Global helper functions
 */

// Loads the vertices with position and normal from the specified Wavefront OBJ model file.
std::vector<TexturedVertex> LoadObjModel(const std::string& filename);

// Loads the vertices with position and normal from the specified Wavefront OBJ model file.
TriangleMesh LoadObjModel(std::vector<TexturedVertex>& vertices, const std::string& filename, unsigned verticesPerFace = 3);

// Generates eight vertices for a unit cube.
std::vector<Gs::Vector3f> GenerateCubeVertices();

// Generates 36 indices for a unit cube of 8 vertices
// (36 = 3 indices per triangle * 2 triangles per cube face * 6 faces).
std::vector<std::uint32_t> GenerateCubeTriangleIndices();

// Generates 24 vertices for a unit cube with texture coordinates.
std::vector<TexturedVertex> GenerateTexturedCubeVertices();

// Generates 36 indices for a unit cube of 24 vertices
std::vector<std::uint32_t> GenerateTexturedCubeTriangleIndices();

// Generates 24 indices for a unit cube of 8 vertices.
// (24 = 4 indices per quad * 1 quad per cube face * 6 faces)
std::vector<std::uint32_t> GenerateTexturedCubeQuadIndices(std::uint32_t numVertices, std::uint32_t firstVertex);

// Generates tangent-space vertices from the specified list of textured vertices.
std::vector<TangentSpaceVertex> GenerateTangentSpaceVertices(const LLGL::ArrayView<TexturedVertex>& vertices);

// Generates tangent-space vertices (per quad) from the specified list of textured vertices.
std::vector<TangentSpaceVertex> GenerateTangentSpaceQuadVertices(const LLGL::ArrayView<TexturedVertex>& vertices);

// Returns a point on the specified line segment that is the closest to the reference point.
Gs::Vector3f ClosestPointOnLineSegment(const Gs::Vector3f& linePointA, const Gs::Vector3f& linePointB, const Gs::Vector3f& referencePoint);


#endif

