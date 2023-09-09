/*
 * TestbedContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_TESTBED_CONTEXT_H
#define LLGL_TESTBED_CONTEXT_H


#include <LLGL/LLGL.h>
#include <LLGL/Utils/VertexFormat.h>
#include <LLGL/Utils/ColorRGBA.h>
#include <Gauss/Matrix.h>
#include <Gauss/Vector4.h>
#include <vector>
#include <functional>
#include <initializer_list>


constexpr float epsilon = 0.00001f;

enum class TestResult
{
    Continue,       // Continue testing.
    Passed,         // Test passed.
    FailedMismatch, // Test failed due to mismatch between expected and given data.
    FailedErrors,   // Test failed due to interface errors.
};

class TestbedContext
{

    public:

        TestbedContext(const char* moduleName, int version, int argc, char* argv[]);

        // Runs all tests and returns the number of failed ones. If all succeeded, the return value is 0.
        unsigned RunAllTests();

    public:

        static void RunRendererIndependentTests();

    protected:

        TestResult RunTest(const std::function<TestResult(unsigned)>& callback);

        TestResult CreateBuffer(
            const LLGL::BufferDescriptor&   desc,
            const char*                     name,
            LLGL::Buffer**                  output,
            const void*                     initialData = nullptr
        );

        TestResult CreateTexture(
            const LLGL::TextureDescriptor&  desc,
            const char*                     name,
            LLGL::Texture**                 output,
            const LLGL::SrcImageDescriptor* initialImage = nullptr
        );

        TestResult CreateRenderTarget(
            const LLGL::RenderTargetDescriptor& desc,
            const char*                         name,
            LLGL::RenderTarget**                output
        );

    protected:

        enum Models
        {
            ModelCube = 0,

            ModelCount,
        };

        enum Shaders
        {
            VSSolid = 0,
            PSSolid,
            VSTextured,
            PSTextured,

            ShaderCount,
        };

        enum DiffErrors
        {
            DiffErrorLoadRefFailed      = -1,
            DiffErrorLoadResultFailed   = -2,
            DiffErrorExtentMismatch     = -3,
            DiffErrorSaveDiffFailed     = -4,
        };

    protected:

        struct Vertex
        {
            float position[3];
            float normal[3];
            float texCoord[2];
        };

        struct IndexedTriangleMesh
        {
            std::uint64_t indexBufferOffset;
            std::uint32_t numIndices;
        };

        struct IndexedTriangleMeshBuffer
        {
            std::vector<Vertex>         vertices;
            std::vector<std::uint32_t>  indices;
            std::uint32_t               firstVertex = 0;
            std::uint32_t               firstIndex  = 0;

            void NewMesh();
            void AddVertex(float x, float y, float z, float nx, float ny, float nz, float tx, float ty);
            void AddIndices(const std::initializer_list<std::uint32_t>& indices, std::uint32_t offset = 0);
            void FinalizeMesh(IndexedTriangleMesh& outMesh);
        };

        struct SceneConstants
        {
            Gs::Matrix4f wvpMatrix;
            Gs::Matrix4f wMatrix;
            Gs::Vector4f solidColor = { 1, 1, 1, 1 };
            Gs::Vector4f lightVec   = { 0, 0, -1, 0 };
        }
        sceneConstants;

    protected:

        const std::string           moduleName;
        const std::string           outputDir;
        const bool                  verbose;
        const bool                  sanityCheck;    // This is 'very verbose' and dumps out all intermediate data on successful tests
        const bool                  showTiming;
        const bool                  fastTest;       // Skip slow buffer/texture creations to speed up test run

        unsigned                    failures                = 0;

        LLGL::RenderingProfiler     profiler;
        LLGL::RenderingDebugger     debugger;
        LLGL::RenderSystemPtr       renderer;
        LLGL::RenderingCapabilities caps;
        LLGL::SwapChain*            swapChain               = nullptr;
        LLGL::CommandBuffer*        cmdBuffer               = nullptr;
        LLGL::CommandQueue*         cmdQueue                = nullptr;
        LLGL::Surface*              surface                 = nullptr;
        LLGL::Buffer*               meshBuffer              = nullptr;
        LLGL::Buffer*               sceneCbuffer            = nullptr;

        LLGL::VertexFormat          vertexFormat;
        IndexedTriangleMesh         models[ModelCount];
        LLGL::Shader*               shaders[ShaderCount]    = {};
        Gs::Matrix4f                projection;

    private:

        #define DECL_TEST(NAME) \
            TestResult Test##NAME(unsigned frame)

        DECL_TEST( CommandBufferSubmit );
        DECL_TEST( BufferWriteAndRead );
        DECL_TEST( BufferMap );
        DECL_TEST( BufferFill );
        DECL_TEST( BufferUpdate );
        DECL_TEST( BufferCopy );
        DECL_TEST( BufferToTextureCopy );
        DECL_TEST( TextureCopy );
        DECL_TEST( TextureToBufferCopy );
        DECL_TEST( TextureWriteAndRead );
        DECL_TEST( TextureTypes );
        DECL_TEST( DepthBuffer );
        DECL_TEST( StencilBuffer );
        DECL_TEST( RenderTargetNoAttachments );
        DECL_TEST( RenderTarget1Attachment );
        DECL_TEST( RenderTargetNAttachments );

        #undef DECL_TEST

    private:

        static std::string FormatByteArray(const void* data, std::size_t size, std::size_t bytesPerGroup = 1, bool formatAsFloats = false);

        static double ToMillisecs(std::uint64_t t0, std::uint64_t t1);

    private:

        void LogRendererInfo();

        bool LoadShaders();
        void LoadProjectionMatrix(float nearPlane = 0.1f, float farPlane = 100.0f, float fov = 45.0f);

        void CreateTriangleMeshes();

        void CreateModelCube(IndexedTriangleMeshBuffer& scene, IndexedTriangleMesh& outMesh);

        void CreateConstantBuffers();

        void SaveDepthImageTGA(const std::vector<float>& image, const LLGL::Extent2D& extent, const std::string& name);
        void SaveDepthImageTGA(const std::vector<float>& image, const LLGL::Extent2D& extent, const std::string& name, float nearPlane, float farPlane);
        void SaveStencilImageTGA(const std::vector<std::uint8_t>& image, const LLGL::Extent2D& extent, const std::string& name);

        // Creates a heat-map image from the two input filenames and returns the highest difference pixel value. A negative value indicates an error.
        int DiffImagesTGA(const std::string& name, int threshold = 1, int scale = 1);

        void RecordTestResult(TestResult result, const char* name);

};


#endif



// ================================================================================
