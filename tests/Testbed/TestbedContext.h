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
#include <LLGL/Utils/ColorRGB.h>
#include <LLGL/Utils/Image.h>
#include <Gauss/Matrix.h>
#include <Gauss/Vector4.h>
#include <vector>
#include <functional>
#include <initializer_list>


constexpr float epsilon = 0.00001f;

enum class TestResult
{
    Continue,           // Continue testing.
    ContinueSkipFrame,  // Continue testing, skip frame output.
    Passed,             // Test passed.
    Skipped,            // Test was skipped due to unsupported features. Cannot be treated as error.
    FailedMismatch,     // Test failed due to mismatch between expected and given data.
    FailedErrors,       // Test failed due to interface errors.
};

class TestbedContext
{

    public:

        TestbedContext(const char* moduleName, int version, int argc, char* argv[]);

        ~TestbedContext();

        // Runs all tests and returns the number of failed ones. If all succeeded, the return value is 0.
        unsigned RunAllTests();

        // Returns true if this context has a valid renderer.
        inline bool IsValid() const
        {
            return (renderer.get() != nullptr);
        }

    public:

        static unsigned RunRendererIndependentTests(int argc, char* argv[]);

        static void PrintSeparator();

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
            const LLGL::ImageView*          initialImage = nullptr
        );

        TestResult CreateRenderTarget(
            const LLGL::RenderTargetDescriptor& desc,
            const char*                         name,
            LLGL::RenderTarget**                output
        );

        TestResult CreateGraphicsPSO(
            const LLGL::GraphicsPipelineDescriptor& desc,
            const char*                             name,
            LLGL::PipelineState**                   output
        );

        TestResult CreateComputePSO(
            const LLGL::ComputePipelineDescriptor&  desc,
            const char*                             name,
            LLGL::PipelineState**                   output
        );

        // Returns true if the current renderer requires combined texture samplers (OpenGL only).
        bool HasCombinedSamplers() const;

        // Returns true if the current renderer requires unique bindings slots (Vulkan only).
        bool HasUniqueBindingSlots() const;

    protected:

        enum Models
        {
            ModelCube = 0,
            ModelRect,

            ModelCount,
        };

        enum VertFmt
        {
            VertFmtStd = 0,
            VertFmtColored,
            VertFmtColoredSO,
            VertFmtUnprojected,
            VertFmtEmpty,

            VertFmtCount,
        };

        enum Pipelines
        {
            PipelineSolid,
            PipelineTextured,

            PipelineCount,
        };

        enum Shaders
        {
            VSSolid = 0,
            PSSolid,

            VSTextured,
            PSTextured,

            VSDynamic,
            PSDynamic,

            VSUnprojected,
            PSUnprojected,

            VSDualSourceBlend,
            PSDualSourceBlend,

            VSShadowMap,
            VSShadowedScene,
            PSShadowedScene,

            VSResourceArrays,
            PSResourceArrays,

            VSResourceBinding,
            PSResourceBinding,
            CSResourceBinding,

            VSClear,
            PSClear,

            VSStreamOutput,
            VSStreamOutputXfb,
            HSStreamOutput,
            DSStreamOutput,
            DSStreamOutputXfb,
            GSStreamOutputXfb,
            PSStreamOutput,

            VSCombinedSamplers,
            PSCombinedSamplers,

            CSSamplerBuffer,

            CSReadAfterWrite,

            ShaderCount,
        };

        enum Textures
        {
            TextureGrid10x10 = 0,
            TextureGradient,
            TexturePaintingA_NPOT,  // NPOT texture 600x479
            TexturePaintingB,       // 512x512
            TextureDetailMap,       // 256x256

            TextureCount,
        };

        enum Samplers
        {
            SamplerNearest = 0,
            SamplerNearestClamp,
            SamplerNearestNoMips,
            SamplerLinear,
            SamplerLinearClamp,
            SamplerLinearNoMips,

            SamplerCount,
        };

        enum DiffErrors
        {
            DiffErrorLoadRefFailed      = -1,
            DiffErrorLoadResultFailed   = -2,
            DiffErrorExtentMismatch     = -3,
            DiffErrorSaveDiffFailed     = -4,
        };

    protected:

        struct Options
        {
            std::string                 outputDir;
            bool                        verbose     = false;
            bool                        pedantic    = false; // Ignore thresholds, always compare strictly against reference values
            bool                        greedy      = false; // Continue testing on failure
            bool                        sanityCheck = false; // This is 'very verbose' and dumps out all intermediate data on successful tests
            bool                        showTiming  = false;
            bool                        fastTest    = false; // Skip slow buffer/texture creations to speed up test run
            LLGL::Extent2D              resolution;
            std::vector<std::string>    selectedTests;

            bool ContainsTest(const char* name) const;
        };

        struct StandardVertex
        {
            float position[3];
            float normal[3];
            float texCoord[2];
        };

        struct ColoredVertex
        {
            float position[4];
            float normal[3];
            float color[3];
        };

        struct UnprojectedVertex
        {
            float           position[2];
            std::uint8_t    color[4];
        };

        struct IndexedTriangleMesh
        {
            std::uint64_t indexBufferOffset;
            std::uint32_t numIndices;
        };

        struct IndexedTriangleMeshBuffer
        {
            std::vector<StandardVertex> vertices;
            std::vector<std::uint32_t>  indices;
            std::uint32_t               firstVertex = 0;
            std::uint32_t               firstIndex  = 0;

            void NewMesh();
            void AddVertex(float x, float y, float z, float nx, float ny, float nz, float tx, float ty);
            void AddIndices(const std::initializer_list<std::uint32_t>& indices, std::uint32_t offset = 0);
            void FinalizeMesh(IndexedTriangleMesh& outMesh);
        };

        struct Histogram
        {
            static constexpr int rangeSize = 32;

            void Reset();
            void Add(int val);
            void Print(unsigned rows = 10) const;

            unsigned diffRangeCounts[rangeSize] = {};
        };

        struct DiffResult
        {
            DiffResult() = default;
            DiffResult(const DiffResult&) = default;
            DiffResult& operator = (const DiffResult&) = default;

            DiffResult(DiffErrors error);
            explicit DiffResult(int threshold, unsigned tolerance = 0);

            // Returns the difference result as a string.
            const char* Print() const;

            void Add(int val);

            bool Mismatch() const;

            void ResetHistogram(Histogram* histogram);

            // Returns TestResult::Passed or TestResult::FailedMismatch depending on diff result.
            TestResult Evaluate(const char* name, unsigned frame = ~0u) const;

            Histogram*  histogram   = nullptr;
            int         threshold   = 0; // Difference threshold (related to 'value').
            unsigned    tolerance   = 0; // Number of pixels to tolerate over the threshold (related to 'count').
            int         value       = 0; // Maximum difference value
            unsigned    count       = 0; // Number of different pixels;
        };

        struct SceneConstants
        {
            Gs::Matrix4f vpMatrix;
            Gs::Matrix4f wMatrix;
            Gs::Vector4f solidColor = { 1, 1, 1, 1 };
            Gs::Vector4f lightVec   = { 0, 0, -1, 0 };
        }
        sceneConstants;

    protected:

        const std::string               moduleName;
        const Options                   opt;
        const LLGL::ClearValue          bgColorDarkBlue         = { 0.2f, 0.2f, 0.4f, 1.0f };
        const LLGL::ClearValue          bgColorLightBlue        = { 127.0f/255.0f, 127.0f/255.0f, 1.0f, 1.0f };

        unsigned                        failures                = 0;

        LLGL::RenderingDebugger         debugger;
        LLGL::RenderSystemPtr           renderer;
        LLGL::RendererInfo              rendererInfo;
        LLGL::RenderingCapabilities     caps;
        LLGL::SwapChain*                swapChain               = nullptr;
        LLGL::CommandBuffer*            cmdBuffer               = nullptr;
        LLGL::CommandQueue*             cmdQueue                = nullptr;
        LLGL::Surface*                  surface                 = nullptr;
        LLGL::Buffer*                   meshBuffer              = nullptr;
        LLGL::Buffer*                   sceneCbuffer            = nullptr;

        LLGL::VertexFormat              vertexFormats[VertFmtCount];
        IndexedTriangleMesh             models[ModelCount];
        LLGL::Shader*                   shaders[ShaderCount]    = {};
        LLGL::PipelineLayout*           layouts[PipelineCount]  = {};
        LLGL::Texture*                  textures[TextureCount]  = {};
        LLGL::Sampler*                  samplers[SamplerCount]  = {};
        Gs::Matrix4f                    projection;

    private:

        #include "UnitTests/DeclTests.inl"

    private:

        static Options ParseOptions(int argc, char* argv[]);

        static std::string FormatByteArray(const void* data, std::size_t size, std::size_t bytesPerGroup = 1, bool formatAsFloats = false);

        static double ToMillisecs(std::uint64_t t0, std::uint64_t t1);

        static LLGL::Image LoadImageFromFile(const std::string& filename, bool verbose = false);
        static void SaveImageToFile(const LLGL::Image& img, const std::string& filename, bool verbose = false);

        static bool IsRGBA8ubInThreshold(const std::uint8_t lhs[4], const std::uint8_t rhs[4], int threshold = 1);

    private:

        void LogRendererInfo();

        bool LoadShaders();
        void CreatePipelineLayouts();
        bool LoadTextures();
        void CreateSamplerStates();
        void LoadProjectionMatrix(Gs::Matrix4f& outProjection, float aspectRatio = 1.0f, float nearPlane = 0.1f, float farPlane = 100.0f, float fov = 45.0f);
        void LoadDefaultProjectionMatrix();

        void CreateTriangleMeshes();

        void CreateModelCube(IndexedTriangleMeshBuffer& scene, IndexedTriangleMesh& outMesh);
        void CreateModelRect(IndexedTriangleMeshBuffer& scene, IndexedTriangleMesh& outMesh);

        void ConvertToColoredVertexList(const IndexedTriangleMeshBuffer& scene, std::vector<ColoredVertex>& outVertices, const LLGL::ColorRGBAf& color = {});

        void CreateConstantBuffers();

        LLGL::Shader* LoadShaderFromFile(
            const std::string&          filename,
            LLGL::ShaderType            type,
            const char*                 entry       = nullptr,
            const char*                 profile     = nullptr,
            const LLGL::ShaderMacro*    defines     = nullptr,
            VertFmt                     vertFmt     = VertFmtStd,
            VertFmt                     vertOutFmt  = VertFmtCount
        );

        void SaveColorImage(const std::vector<LLGL::ColorRGBub>& image, const LLGL::Extent2D& extent, const std::string& name);
        void SaveDepthImage(const std::vector<float>& image, const LLGL::Extent2D& extent, const std::string& name);
        void SaveDepthImage(const std::vector<float>& image, const LLGL::Extent2D& extent, const std::string& name, float nearPlane, float farPlane);
        void SaveStencilImage(const std::vector<std::uint8_t>& image, const LLGL::Extent2D& extent, const std::string& name);

        LLGL::Texture* CaptureFramebuffer(LLGL::CommandBuffer& cmdBuffer, LLGL::Format format, const LLGL::Extent2D& extent);
        void SaveCapture(LLGL::Texture* capture, const std::string& name, bool writeStencilOnly = false);

        // Creates a heat-map image from the two input filenames and returns the highest difference pixel value. A negative value indicates an error.
        DiffResult DiffImages(const std::string& name, int threshold = 1, unsigned tolerance = 0, int scale = 1);

        void RecordTestResult(TestResult result, const char* name);

        bool QueryResultsWithTimeout(
            LLGL::QueryHeap&    queryHeap,
            std::uint32_t       firstQuery,
            std::uint32_t       numQueries,
            void*               data,
            std::size_t         dataSize
        );

    private:

        bool                    loadingShadersFailed_ = false;
        Histogram               histogram_;
        LLGL::Report            report_;
        LLGL::Log::LogHandle    reportHandle_;

};


#endif



// ================================================================================
