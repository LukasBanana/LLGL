/*
 * ExampleBase.c (C99)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>
#include <wchar.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>


/*
 * Global constants
 */

const LLGLClearValue g_defaultClear =
{
    .color = { 0.1f, 0.1f, 0.2f, 1.0f },
    .depth = 1.0f
};

const LLGLSamplerDescriptor g_defaultSamplerDesc =
{
    .addressModeU   = LLGLSamplerAddressModeRepeat,
    .addressModeV   = LLGLSamplerAddressModeRepeat,
    .addressModeW   = LLGLSamplerAddressModeRepeat,
    .minFilter      = LLGLSamplerFilterLinear,
    .magFilter      = LLGLSamplerFilterLinear,
    .mipMapFilter   = LLGLSamplerFilterLinear,
    .mipMapEnabled  = true,
    .mipMapLODBias  = 0.0f,
    .minLOD         = 0.0f,
    .maxLOD         = 1000.0f,
    .maxAnisotropy  = 1,
    .compareEnabled = false,
    .compareOp      = LLGLCompareOpLess,
    .borderColor    = { 0.0f, 0.0f, 0.0f, 0.0f },
};

/*
 * Global variables
 */

int                     g_renderer          = 0;
LLGLSwapChain           g_swapChain         = LLGL_NULL_OBJECT;
LLGLSurface             g_surface           = LLGL_NULL_OBJECT;
LLGLCommandBuffer       g_commandBuffer     = LLGL_NULL_OBJECT;
LLGLCommandQueue        g_commandQueue      = LLGL_NULL_OBJECT;
LLGLViewport            g_viewport;
float                   g_projection[4][4]  = { { 1.0f, 0.0f, 0.0f, 0.0f },
                                                { 0.0f, 1.0f, 0.0f, 0.0f },
                                                { 0.0f, 0.0f, 1.0f, 0.0f },
                                                { 0.0f, 0.0f, 0.0f, 1.0f } };


/*
 * Internals
 */

static struct ExampleEventStatus
{
    float   mouseMotion[2];
    bool    keyDown[256];
}
g_EventStauts =
{
    .mouseMotion = { 0.0f, 0.0f }
};

static void reset_event_status()
{
    g_EventStauts.mouseMotion[0] = 0.0f;
    g_EventStauts.mouseMotion[1] = 0.0f;
}

static void key_down_event(LLGLWindow sender, LLGLKey keyCode)
{
    g_EventStauts.keyDown[keyCode] = true;
}

static void key_up_event(LLGLWindow sender, LLGLKey keyCode)
{
    g_EventStauts.keyDown[keyCode] = false;
}

static void mouse_motion_event(LLGLWindow sender, const LLGLOffset2D* motion)
{
    g_EventStauts.mouseMotion[0] = (float)motion->x;
    g_EventStauts.mouseMotion[1] = (float)motion->y;
}


/*
 * Global functions
 */

static struct ExampleConfig g_Config =
{
    .rendererModule = "OpenGL",
    .windowSize     = { 800, 600 },
    .samples        = 8,
    .vsync          = true,
    .debugger       = false,
    .noDepthStencil = false
};

static void update_viewport()
{
    LLGLExtent2D swapChainResolution;
    llglGetSurfaceContentSize(g_surface, &swapChainResolution);

    g_viewport.x        = 0.0f;
    g_viewport.y        = 0.0f;
    g_viewport.width    = (float)swapChainResolution.width;
    g_viewport.height   = (float)swapChainResolution.height;
    g_viewport.minDepth = 0.0f;
    g_viewport.maxDepth = 1.0f;
}

static float aspect_ratio()
{
    LLGLExtent2D swapChainResolution;
    llglGetSurfaceContentSize(g_surface, &swapChainResolution);
    return (float)swapChainResolution.width / (float)swapChainResolution.height;
}

void example_config(const ExampleConfig* config)
{
    if (config != NULL)
    {
        if (config->rendererModule != NULL)
            g_Config.rendererModule = config->rendererModule;
        if (config->windowSize[0] != 0)
            g_Config.windowSize[0] = config->windowSize[0];
        if (config->windowSize[1] != 0)
            g_Config.windowSize[1] = config->windowSize[1];
        g_Config.samples        = config->samples;
        g_Config.vsync          = config->vsync;
        g_Config.debugger       = config->debugger;
        g_Config.noDepthStencil = config->noDepthStencil;
    }
    else
    {
        g_Config.rendererModule = "OpenGL";
        g_Config.windowSize[0]  = 800;
        g_Config.windowSize[1]  = 600;
        g_Config.samples        = 8;
        g_Config.vsync          = true;
        g_Config.debugger       = false;
        g_Config.noDepthStencil = false;
    }
}

int example_init(const wchar_t* title)
{
    // Register standard output as log callback
    llglRegisterLogCallbackStd();

    // Load render system module
    const char* rendererModule = g_Config.rendererModule;
    if (llglLoadRenderSystem(rendererModule) == 0)
    {
        fprintf(stderr, "Failed to load render system: %s\n", rendererModule);
        return 1;
    }

    // Create swap-chain
    LLGLSwapChainDescriptor swapChainDesc =
    {
        .resolution     = { g_Config.windowSize[0], g_Config.windowSize[1] },
        .colorBits      = 32,                                   // 32 bits for color information
        .depthBits      = (g_Config.noDepthStencil ? 0 : 24),   // 24 bits for depth comparison
        .stencilBits    = (g_Config.noDepthStencil ? 0 : 8),    // 8 bits for stencil patterns
        .samples        = g_Config.samples,                     // check if LLGL adapts sample count that is too high
    };
    g_swapChain = llglCreateSwapChain(&swapChainDesc);

    // Enable V-sync
    llglSetVsyncInterval(g_swapChain, 1);

    // Set window title and show window
    g_surface = llglGetSurface(g_swapChain);
    LLGLWindow window = LLGL_GET_AS(LLGLWindow, g_surface);

    wchar_t fullTitle[1024] = { L'\0' };
    swprintf(fullTitle, sizeof(fullTitle)/sizeof(fullTitle[0]), L"LLGL C99 Example: %s", title);
    llglSetWindowTitle(window, fullTitle);

    // Register event listener to respond to move and keyboard events
    const LLGLWindowEventListener windowCallbacks =
    {
        .onKeyDown      = key_down_event,
        .onKeyUp        = key_up_event,
        .onGlobalMotion = mouse_motion_event,
    };
    llglAddWindowEventListener(window, &windowCallbacks);

    // Show window after its setup is done
    llglShowWindow(window, true);

    // Create command buffer to submit subsequent graphics commands to the GPU
    LLGLCommandBufferDescriptor cmdBufferDesc =
    {
        // Use immediate context to avoid redundant calls to llglSubmitCommandBuffer() in every example
        .flags              = LLGLCommandBufferImmediateSubmit,

        // Use two native command buffers; This is merely a hint to the backend (OpenGL for instance will ignore this)
        .numNativeBuffers   = 2,
    };
    g_commandBuffer = llglCreateCommandBuffer(&cmdBufferDesc);

    // Initialize viewport
    update_viewport();

    // Initialize default projection matrix
    const float aspectRatio = aspect_ratio();
    perspective_projection(g_projection, aspectRatio, /*nearPlane:*/ 0.1f, /*farPlane;*/ 100.0f, /*fieldOfView:*/ DEG2RAD(45.0f));

    return 0;
}

void example_release()
{
    llglUnloadRenderSystem();
}

static void build_perspective_projection(float m[4][4], float aspect, float nearPlane, float farPlane, float fov, bool isUnitCube)
{
    const float h = 1.0f / tanf(fov * 0.5f);
    const float w = h / aspect;

    m[0][0] = w;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = 0.0f;

    m[1][0] = 0.0f;
    m[1][1] = h;
    m[1][2] = 0.0f;
    m[1][3] = 0.0f;

    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = (isUnitCube ? (farPlane + nearPlane)/(farPlane - nearPlane) : farPlane/(farPlane - nearPlane));
    m[2][3] = 1.0f;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = (isUnitCube ? -(2.0f*farPlane*nearPlane)/(farPlane - nearPlane) : -(farPlane*nearPlane)/(farPlane - nearPlane));
    m[3][3] = 0.0f;
}

bool example_poll_events()
{
    // Reset event status
    reset_event_status();

    // Process surface and events and check if window was closed
    return llglProcessSurfaceEvents() && !llglHasWindowQuit(LLGL_GET_AS(LLGLWindow, g_surface)) && !g_EventStauts.keyDown[LLGLKeyEscape];
}

void perspective_projection(float outProjection[4][4], float aspectRatio, float nearPlane, float farPlane, float fieldOfView)
{
    const int rendererID = llglGetRendererID();
    const bool isUnitCube = (rendererID == LLGL_RENDERERID_OPENGL || rendererID == LLGL_RENDERERID_VULKAN);
    build_perspective_projection(outProjection, aspectRatio, nearPlane, farPlane, fieldOfView, isUnitCube);
}

static void build_orthogonal_projection(float m[4][4], float width, float height, float nearPlane, float farPlane, bool isUnitCube)
{
    m[0][0] = 2.0f / width;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = 0.0f;

    m[1][0] = 0.0f;
    m[1][1] = 2.0f / height;
    m[1][2] = 0.0f;
    m[1][3] = 0.0f;

    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = (isUnitCube ? 2.0f/(farPlane - nearPlane) : 1.0f/(farPlane - nearPlane));
    m[3][2] = 0.0f;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[2][3] = (isUnitCube ? -(farPlane + nearPlane)/(farPlane - nearPlane) : -nearPlane/(farPlane - nearPlane));
    m[3][3] = 1.0f;
}

void orthogonal_projection(float outProjection[4][4], float width, float height, float nearPlane, float farPlane)
{
    const int rendererID = llglGetRendererID();
    const bool isUnitCube = (rendererID == LLGL_RENDERERID_OPENGL || rendererID == LLGL_RENDERERID_VULKAN);
    build_orthogonal_projection(outProjection, width, height, nearPlane, farPlane, isUnitCube);
}

void get_textured_cube(const TexturedVertex** outVertices, size_t* outVertexCount, const uint32_t** outIndices, size_t* outIndexCount)
{
    static const TexturedVertex vertices[] =
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

    static const uint32_t indices[] =
    {
         0,  1,  2,  0,  2,  3, // front
         4,  5,  6,  4,  6,  7, // right
         8,  9, 10,  8, 10, 11, // left
        12, 13, 14, 12, 14, 15, // top
        16, 17, 18, 16, 18, 19, // bottom
        20, 21, 22, 20, 22, 23, // back
    };

    *outVertices    = vertices;
    *outVertexCount = ARRAY_SIZE(vertices);
    *outIndices     = indices;
    *outIndexCount  = ARRAY_SIZE(indices);
}

#define foreach_matrix_element(R, C) \
    for (int R = 0; R < 4; ++R) for (int C = 0; C < 4; ++C)

void matrix_load_identity(float outMatrix[4][4])
{
    foreach_matrix_element(r, c)
        outMatrix[c][r] = (r == c ? 1.0f : 0.0f);
}

void matrix_mul(float outMatrix[4][4], const float inMatrixLhs[4][4], const float inMatrixRhs[4][4])
{
    foreach_matrix_element(r, c)
    {
        outMatrix[c][r] = 0.0f;
        for (int i = 0; i < 4; ++i)
            outMatrix[c][r] += inMatrixLhs[i][r] * inMatrixRhs[c][i];
    }
}

void matrix_translate(float outMatrix[4][4], float x, float y, float z)
{
    outMatrix[3][0] += outMatrix[0][0]*x + outMatrix[1][0]*y + outMatrix[2][0]*z;
    outMatrix[3][1] += outMatrix[0][1]*x + outMatrix[1][1]*y + outMatrix[2][1]*z;
    outMatrix[3][2] += outMatrix[0][2]*x + outMatrix[1][2]*y + outMatrix[2][2]*z;
}

void matrix_rotate(float outMatrix[4][4], float x, float y, float z, float angle)
{
    const float c  = cosf(angle);
    const float s  = sinf(angle);
    const float cc = 1.0f - c;

    const float vecInvLen = 1.0f / sqrtf(x*x + y*y + z*z);
    x *= vecInvLen;
    y *= vecInvLen;
    z *= vecInvLen;

    outMatrix[0][0] = x*x*cc + c;
    outMatrix[0][1] = x*y*cc - z*s;
    outMatrix[0][2] = x*z*cc + y*s;

    outMatrix[1][0] = y*x*cc + z*s;
    outMatrix[1][1] = y*y*cc + c;
    outMatrix[1][2] = y*z*cc - x*s;

    outMatrix[2][0] = x*z*cc - y*s;
    outMatrix[2][1] = y*z*cc + x*s;
    outMatrix[2][2] = z*z*cc + c;
}

bool key_pressed(LLGLKey keyCode)
{
    return g_EventStauts.keyDown[keyCode];
}

float mouse_movement_x()
{
    return g_EventStauts.mouseMotion[0];
}

float mouse_movement_y()
{
    return g_EventStauts.mouseMotion[1];
}

