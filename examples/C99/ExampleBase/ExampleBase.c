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

#if defined(ANDROID) || defined(__ANDROID__)
#   include "Android/AppUtils.h"
#endif


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

int                 g_renderer          = 0;
LLGLSwapChain       g_swapChain         = LLGL_NULL_OBJECT;
LLGLSurface         g_surface           = LLGL_NULL_OBJECT;
LLGLCommandBuffer   g_commandBuffer     = LLGL_NULL_OBJECT;
LLGLCommandQueue    g_commandQueue      = LLGL_NULL_OBJECT;
LLGLViewport        g_viewport;
float               g_projection[4][4]  = { { 1.0f, 0.0f, 0.0f, 0.0f },
                                            { 0.0f, 1.0f, 0.0f, 0.0f },
                                            { 0.0f, 0.0f, 1.0f, 0.0f },
                                            { 0.0f, 0.0f, 0.0f, 1.0f } };
ExampleConfig       g_config            = { .rendererDesc.moduleName    = "OpenGL",
                                            .resolution                 = { 800, 600 },
                                            .samples                    = 8,
                                            .vsync                      = true,
                                            .debugger                   = false,
                                            .noDepthStencil             = false };

#if defined(ANDROID) || defined(__ANDROID__)
struct android_app* g_androidApp        = NULL;
#endif


/*
 * Internals
 */

static struct ExampleEvents
{
    float   mouseMotion[2];
    bool    keyDown[256];
    bool    keyPushed[256];
}
g_events =
{
    .mouseMotion = { 0.0f, 0.0f }
};

static void reset_event_status()
{
    g_events.mouseMotion[0] = 0.0f;
    g_events.mouseMotion[1] = 0.0f;
    memset(g_events.keyPushed, 0, sizeof(g_events.keyPushed));
}

static void key_down_event(LLGLWindow sender, LLGLKey keyCode)
{
    if (!g_events.keyDown[keyCode])
        g_events.keyPushed[keyCode] = true;
    g_events.keyDown[keyCode] = true;
}

static void key_up_event(LLGLWindow sender, LLGLKey keyCode)
{
    g_events.keyDown[keyCode] = false;
}

static void mouse_motion_event(LLGLWindow sender, const LLGLOffset2D* motion)
{
    g_events.mouseMotion[0] = (float)motion->x;
    g_events.mouseMotion[1] = (float)motion->y;
}


/*
 * Global functions
 */

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

static void example_config(const ExampleArgs* args)
{
#if defined(ANDROID) || defined(__ANDROID__)
    g_androidApp = args->androidApp;

    g_config.rendererDesc.moduleName    = "OpenGLES3";
    g_config.rendererDesc.androidApp    = g_androidApp;

    // Store pointer to asset manager so we can load assets from the APK bundle
    if (g_androidApp->activity != NULL)
        AndroidSetAssetManager(g_androidApp->activity->assetManager);
#else
    g_config.rendererDesc.moduleName    = "OpenGL";
    g_config.resolution[0]              = 800;
    g_config.resolution[1]              = 600;
    g_config.samples                    = 8;
    g_config.vsync                      = true;
    g_config.debugger                   = false;
    g_config.noDepthStencil             = false;
#endif
}

void log_renderer_info()
{
    LLGLRendererInfo info;
    llglGetRendererInfo(&info);

    LLGLExtent2D swapChainRes;
    LLGLRenderTarget swapChainRT = LLGL_GET_AS(LLGLRenderTarget, g_swapChain);
    llglGetRenderTargetResolution(swapChainRT, &swapChainRes);

    llglLogPrintf(
        "render system:\n"
        "  renderer:           %s\n"
        "  device:             %s\n"
        "  vendor:             %s\n"
        "  shading language:   %s\n"
        "\n"
        "swap-chain:\n"
        "  resolution:         %u x %u\n"
        "  samples:            %u\n"
        "\n",
        info.rendererName,
        info.deviceName,
        info.vendorName,
        info.shadingLanguageName,
        swapChainRes.width,
        swapChainRes.height,
        llglGetRenderTargetSamples(swapChainRT)
    );
}

int example_init(const char* title)
{
    // Register standard output as log callback
    llglRegisterLogCallbackStd();

    // Load render system module
    LLGLReport report = llglAllocReport();
    if (llglLoadRenderSystemExt(&(g_config.rendererDesc), report) == 0)
    {
        llglLogErrorf("Failed to load render system: %s\n", g_config.rendererDesc.moduleName);
        if (llglHasReportErrors(report))
            llglLogErrorf("%s", llglGetReportText(report));
        llglFreeReport(report);
        return 1;
    }
    llglFreeReport(report);

    // Create swap-chain
    LLGLSwapChainDescriptor swapChainDesc =
    {
        .resolution     = { g_config.resolution[0], g_config.resolution[1] },
        .colorBits      = 32,                                   // 32 bits for color information
        .depthBits      = (g_config.noDepthStencil ? 0 : 24),   // 24 bits for depth comparison
        .stencilBits    = (g_config.noDepthStencil ? 0 : 8),    // 8 bits for stencil patterns
        .samples        = g_config.samples,                     // check if LLGL adapts sample count that is too high
    };
    g_swapChain = llglCreateSwapChain(&swapChainDesc);
    g_surface = llglGetSurface(g_swapChain);

    // Enable V-sync
    llglSetVsyncInterval(g_swapChain, 1);

    // Set surface title to example name
    char fullTitle[1024] = { L'\0' };
    snprintf(fullTitle, sizeof(fullTitle), "LLGL C99 Example: %s", title);

#if LLGLEXAMPLE_MOBILE
    // Set canvas title
    LLGLCanvas canvas = LLGL_GET_AS(LLGLCanvas, g_surface);
    llglSetCanvasTitleUTF8(canvas, fullTitle);
#else
    // Set window title and show window
    LLGLWindow window = LLGL_GET_AS(LLGLWindow, g_surface);
    llglSetWindowTitleUTF8(window, fullTitle);

    // Register event listener to respond to move and keyboard events
    memset(&g_events, 0, sizeof(g_events));
    const LLGLWindowEventListener windowCallbacks =
    {
        .onKeyDown      = key_down_event,
        .onKeyUp        = key_up_event,
        .onGlobalMotion = mouse_motion_event,
    };
    llglAddWindowEventListener(window, &windowCallbacks);

    // Show window after its setup is done
    llglShowWindow(window, true);
#endif

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

    // Print information about the render system and swap-chain
    log_renderer_info();

    return 0;
}

static bool is_example_running()
{
#if LLGLEXAMPLE_MOBILE
    return true;
#else
    return !llglHasWindowQuit(LLGL_GET_AS(LLGLWindow, g_surface)) && !g_events.keyDown[LLGLKeyEscape];
#endif
}

static bool example_poll_events()
{
    // Reset event status
    reset_event_status();

    // Process surface and events and check if window was closed
    return llglProcessSurfaceEvents() && is_example_running();
}

static void example_release()
{
    llglUnloadRenderSystem();
}

int example_main(int (*pfnInit)(), void (*pfnLoop)(double dt), const ExampleArgs* args)
{
    // Configure initial setup
    example_config(args);

    // Invoke example initialization callback
    if (pfnInit != NULL)
    {
        int ret = pfnInit();
        if (ret != 0)
            return ret;
    }

    // Run main loop
    if (pfnLoop != NULL)
    {
        uint64_t startTick = llglTimerTick();
        double tickFrequency = 1.0 / (double)llglTimerFrequency();

        while (example_poll_events())
        {
            // Update frame time
            uint64_t endTick = llglTimerTick();
            double dt = (double)(endTick - startTick) * tickFrequency;
            startTick = endTick;

            #if __ANDROID__
            if (key_pressed(LLGLKeyBrowserBack))
                ANativeActivity_finish(g_config.rendererDesc.androidApp->activity);
            #endif

            // Tick main loop callback
            pfnLoop(dt);
        }
    }

    // Clean up
    example_release();

    return 0;
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
    return g_events.keyDown[keyCode];
}

bool key_pushed(LLGLKey keyCode)
{
    return g_events.keyPushed[keyCode];
}

float mouse_movement_x()
{
    return g_events.mouseMotion[0];
}

float mouse_movement_y()
{
    return g_events.mouseMotion[1];
}

