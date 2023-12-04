/*
 * ExampleBase.h (C99)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_EXAMPLE_BASE_C99_H
#define LLGL_EXAMPLE_BASE_C99_H


#include <LLGL-C/LLGL.h>
#include <stdbool.h>


/*
 * Helper macros
 */

#define MATH_PI         ( 3.141592654f )
#define DEG2RAD(X)      ( (X) * MATH_PI / 180.0f )
#define ARRAY_SIZE(A)   (sizeof(A)/sizeof((A)[0]))


/*
 * Structures
 */

typedef struct TexturedVertex
{
    float position[3];
    float normal[3];
    float texCoord[2];
}
TexturedVertex;

typedef struct TangentSpaceVertex
{
    float position[3];
    float normal[3];
    float tangents[2][3];
    float texCoord[2];
}
TangentSpaceVertex;

typedef struct TriangleMesh
{
    uint32_t    firstVertex;
    uint32_t    numVertices;
    float       transform[4][4];
    float       color[4];
}
TriangleMesh;


/*
 * Global constants
 */

// Clear value with default background color for all examples.
extern const LLGLClearValue         g_defaultClear;

// Default sampler descriptor.
extern const LLGLSamplerDescriptor  g_defaultSamplerDesc;


/*
 * Global variables
 */

// Main swap-chain
extern LLGLSwapChain        g_swapChain;

// Surface connected to the swap-cain
extern LLGLSurface          g_surface;

// Main command buffer
extern LLGLCommandBuffer    g_commandBuffer;

// Command queue
extern LLGLCommandQueue     g_commandQueue;

// Current viewport for the full swap-chain size.
extern LLGLViewport         g_viewport;

// Primary camera projection
extern float                g_projection[4][4];


/*
 * Global functions
 */

// Initializes the example with the specified title and returns a non-zero error code if initialization failed.
int example_init(const wchar_t* title);

// Releases all example resources.
void example_release();

// Processes all surface events, polls the event list (See mouse_movement_x() etc.) and returns false if the window was closed.
bool example_poll_events();

// Builds a perspective projection matrix.
void perspective_projection(float outProjection[4][4], float aspectRatio, float nearPlane, float farPlane, float fieldOfView);

// Returns the pointers the vertex and index data of a textured cube.
void get_textured_cube(const TexturedVertex** outVertices, size_t* outVertexCount, const uint32_t** outIndices, size_t* outIndexCount);

// Loads the identity of the specified 4x4 matrix.
void matrix_load_identity(float outMatrix[4][4]);

// Multiplies the two matrices and stores the result in the output matrix.
void matrix_mul(float outMatrix[4][4], const float inMatrixLhs[4][4], const float inMatrixRhs[4][4]);

// Translates the matrix into the specified 3D direction.
void matrix_translate(float outMatrix[4][4], float x, float y, float z);

// Rotates the matrix around the specified vector with a given angle (in radians).
void matrix_rotate(float outMatrix[4][4], float x, float y, float z, float angle);

// Returns true if the specified key is currently pressed down.
bool key_pressed(LLGLKey keyCode);

// Returns the mouse movement on the X-axis.
float mouse_movement_x();

// Returns the mouse movement on the Y-axis.
float mouse_movement_y();


#endif

