/*
 * LLGLWrapper.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* AUTO GENERATED CODE - DO NOT EDIT */

using System;
using System.Text;
using System.Runtime.InteropServices;

namespace LLGL
{
    /* ----- Constants ----- */

    public enum RendererID : int
    {
        Undefined   = 0x00000000,
        Null        = 0x00000001,
        OpenGL      = 0x00000002,
        OpenGLES    = 0x00000003,
        WebGL       = 0x00000004,
        WebGPU      = 0x00000005,
        Direct3D9   = 0x00000006,
        Direct3D10  = 0x00000007,
        Direct3D11  = 0x00000008,
        Direct3D12  = 0x00000009,
        Vulkan      = 0x0000000A,
        Metal       = 0x0000000B,
        OpenGLES1   = OpenGLES,
        OpenGLES2   = OpenGLES,
        OpenGLES3   = OpenGLES,
        Reserved    = 0x000000FF,
    }

    /* ----- Enumerations ----- */

    public enum EventAction
    {
        Began,
        Changed,
        Ended,
    }

    public enum RenderConditionMode
    {
        Wait,
        NoWait,
        ByRegionWait,
        ByRegionNoWait,
        WaitInverted,
        NoWaitInverted,
        ByRegionWaitInverted,
        ByRegionNoWaitInverted,
    }

    public enum StencilFace
    {
        FrontAndBack,
        Front,
        Back,
    }

    public enum Format
    {
        Undefined,
        A8UNorm,
        R8UNorm,
        R8SNorm,
        R8UInt,
        R8SInt,
        R16UNorm,
        R16SNorm,
        R16UInt,
        R16SInt,
        R16Float,
        R32UInt,
        R32SInt,
        R32Float,
        R64Float,
        RG8UNorm,
        RG8SNorm,
        RG8UInt,
        RG8SInt,
        RG16UNorm,
        RG16SNorm,
        RG16UInt,
        RG16SInt,
        RG16Float,
        RG32UInt,
        RG32SInt,
        RG32Float,
        RG64Float,
        RGB8UNorm,
        RGB8UNorm_sRGB,
        RGB8SNorm,
        RGB8UInt,
        RGB8SInt,
        RGB16UNorm,
        RGB16SNorm,
        RGB16UInt,
        RGB16SInt,
        RGB16Float,
        RGB32UInt,
        RGB32SInt,
        RGB32Float,
        RGB64Float,
        RGBA8UNorm,
        RGBA8UNorm_sRGB,
        RGBA8SNorm,
        RGBA8UInt,
        RGBA8SInt,
        RGBA16UNorm,
        RGBA16SNorm,
        RGBA16UInt,
        RGBA16SInt,
        RGBA16Float,
        RGBA32UInt,
        RGBA32SInt,
        RGBA32Float,
        RGBA64Float,
        BGRA8UNorm,
        BGRA8UNorm_sRGB,
        BGRA8SNorm,
        BGRA8UInt,
        BGRA8SInt,
        RGB10A2UNorm,
        RGB10A2UInt,
        RG11B10Float,
        RGB9E5Float,
        D16UNorm,
        D24UNormS8UInt,
        D32Float,
        D32FloatS8X24UInt,
        BC1UNorm,
        BC1UNorm_sRGB,
        BC2UNorm,
        BC2UNorm_sRGB,
        BC3UNorm,
        BC3UNorm_sRGB,
        BC4UNorm,
        BC4SNorm,
        BC5UNorm,
        BC5SNorm,
        ASTC4x4,
        ASTC4x4_sRGB,
        ASTC5x4,
        ASTC5x4_sRGB,
        ASTC5x5,
        ASTC5x5_sRGB,
        ASTC6x5,
        ASTC6x5_sRGB,
        ASTC6x6,
        ASTC6x6_sRGB,
        ASTC8x5,
        ASTC8x5_sRGB,
        ASTC8x6,
        ASTC8x6_sRGB,
        ASTC8x8,
        ASTC8x8_sRGB,
        ASTC10x5,
        ASTC10x5_sRGB,
        ASTC10x6,
        ASTC10x6_sRGB,
        ASTC10x8,
        ASTC10x8_sRGB,
        ASTC10x10,
        ASTC10x10_sRGB,
        ASTC12x10,
        ASTC12x10_sRGB,
        ASTC12x12,
        ASTC12x12_sRGB,
        ETC1UNorm,
        ETC2UNorm,
        ETC2UNorm_sRGB,
    }

    public enum ImageFormat
    {
        Alpha,
        R,
        RG,
        RGB,
        BGR,
        RGBA,
        BGRA,
        ARGB,
        ABGR,
        Depth,
        DepthStencil,
        Stencil,
        Compressed,
        BC1,
        BC2,
        BC3,
        BC4,
        BC5,
    }

    public enum DataType
    {
        Undefined,
        Int8,
        UInt8,
        Int16,
        UInt16,
        Int32,
        UInt32,
        Float16,
        Float32,
        Float64,
    }

    public enum ReportType
    {
        Default = 0,
        Error,
    }

    public enum Key
    {
        LButton,
        RButton,
        Cancel,
        MButton,
        XButton1,
        XButton2,
        Back,
        Tab,
        Clear,
        Return,
        Shift,
        Control,
        Menu,
        Pause,
        Capital,
        Escape,
        Space,
        PageUp,
        PageDown,
        End,
        Home,
        Left,
        Up,
        Right,
        Down,
        Select,
        Print,
        Exe,
        Snapshot,
        Insert,
        Delete,
        Help,
        D0,
        D1,
        D2,
        D3,
        D4,
        D5,
        D6,
        D7,
        D8,
        D9,
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        LWin,
        RWin,
        Apps,
        Sleep,
        Keypad0,
        Keypad1,
        Keypad2,
        Keypad3,
        Keypad4,
        Keypad5,
        Keypad6,
        Keypad7,
        Keypad8,
        Keypad9,
        KeypadMultiply,
        KeypadPlus,
        KeypadSeparator,
        KeypadMinus,
        KeypadDecimal,
        KeypadDivide,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,
        F16,
        F17,
        F18,
        F19,
        F20,
        F21,
        F22,
        F23,
        F24,
        NumLock,
        ScrollLock,
        LShift,
        RShift,
        LControl,
        RControl,
        LMenu,
        RMenu,
        BrowserBack,
        BrowserForward,
        BrowserRefresh,
        BrowserStop,
        BrowserSearch,
        BrowserFavorits,
        BrowserHome,
        VolumeMute,
        VolumeDown,
        VolumeUp,
        MediaNextTrack,
        MediaPrevTrack,
        MediaStop,
        MediaPlayPause,
        LaunchMail,
        LaunchMediaSelect,
        LaunchApp1,
        LaunchApp2,
        Plus,
        Comma,
        Minus,
        Period,
        Exponent,
        Attn,
        CrSel,
        ExSel,
        ErEOF,
        Play,
        Zoom,
        NoName,
        PA1,
        OEMClear,
        Any,
    }

    public enum UniformType
    {
        Undefined,
        Float1,
        Float2,
        Float3,
        Float4,
        Double1,
        Double2,
        Double3,
        Double4,
        Int1,
        Int2,
        Int3,
        Int4,
        UInt1,
        UInt2,
        UInt3,
        UInt4,
        Bool1,
        Bool2,
        Bool3,
        Bool4,
        Float2x2,
        Float2x3,
        Float2x4,
        Float3x2,
        Float3x3,
        Float3x4,
        Float4x2,
        Float4x3,
        Float4x4,
        Double2x2,
        Double2x3,
        Double2x4,
        Double3x2,
        Double3x3,
        Double3x4,
        Double4x2,
        Double4x3,
        Double4x4,
        Sampler,
        Image,
        AtomicCounter,
    }

    public enum PrimitiveTopology
    {
        PointList,
        LineList,
        LineStrip,
        LineListAdjacency,
        LineStripAdjacency,
        TriangleList,
        TriangleStrip,
        TriangleListAdjacency,
        TriangleStripAdjacency,
        Patches1,
        Patches2,
        Patches3,
        Patches4,
        Patches5,
        Patches6,
        Patches7,
        Patches8,
        Patches9,
        Patches10,
        Patches11,
        Patches12,
        Patches13,
        Patches14,
        Patches15,
        Patches16,
        Patches17,
        Patches18,
        Patches19,
        Patches20,
        Patches21,
        Patches22,
        Patches23,
        Patches24,
        Patches25,
        Patches26,
        Patches27,
        Patches28,
        Patches29,
        Patches30,
        Patches31,
        Patches32,
    }

    public enum CompareOp
    {
        NeverPass,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        AlwaysPass,
    }

    public enum StencilOp
    {
        Keep,
        Zero,
        Replace,
        IncClamp,
        DecClamp,
        Invert,
        IncWrap,
        DecWrap,
    }

    public enum BlendOp
    {
        Zero,
        One,
        SrcColor,
        InvSrcColor,
        SrcAlpha,
        InvSrcAlpha,
        DstColor,
        InvDstColor,
        DstAlpha,
        InvDstAlpha,
        SrcAlphaSaturate,
        BlendFactor,
        InvBlendFactor,
        Src1Color,
        InvSrc1Color,
        Src1Alpha,
        InvSrc1Alpha,
    }

    public enum BlendArithmetic
    {
        Add,
        Subtract,
        RevSubtract,
        Min,
        Max,
    }

    public enum PolygonMode
    {
        Fill,
        Wireframe,
        Points,
    }

    public enum CullMode
    {
        Disabled,
        Front,
        Back,
    }

    public enum LogicOp
    {
        Disabled,
        Clear,
        Set,
        Copy,
        CopyInverted,
        NoOp,
        Invert,
        AND,
        ANDReverse,
        ANDInverted,
        NAND,
        OR,
        ORReverse,
        ORInverted,
        NOR,
        XOR,
        Equiv,
    }

    public enum TessellationPartition
    {
        Undefined,
        Integer,
        Pow2,
        FractionalOdd,
        FractionalEven,
    }

    public enum QueryType
    {
        SamplesPassed,
        AnySamplesPassed,
        AnySamplesPassedConservative,
        TimeElapsed,
        StreamOutPrimitivesWritten,
        StreamOutOverflow,
        PipelineStatistics,
    }

    public enum ErrorType
    {
        InvalidArgument,
        InvalidState,
        UnsupportedFeature,
        UndefinedBehavior,
    }

    public enum WarningType
    {
        ImproperArgument,
        ImproperState,
        PointlessOperation,
        VaryingBehavior,
    }

    public enum AttachmentLoadOp
    {
        Undefined,
        Load,
        Clear,
    }

    public enum AttachmentStoreOp
    {
        Undefined,
        Store,
    }

    public enum ShadingLanguage
    {
        GLSL           = (0x10000),
        GLSL_110       = (0x10000|110),
        GLSL_120       = (0x10000|120),
        GLSL_130       = (0x10000|130),
        GLSL_140       = (0x10000|140),
        GLSL_150       = (0x10000|150),
        GLSL_330       = (0x10000|330),
        GLSL_400       = (0x10000|400),
        GLSL_410       = (0x10000|410),
        GLSL_420       = (0x10000|420),
        GLSL_430       = (0x10000|430),
        GLSL_440       = (0x10000|440),
        GLSL_450       = (0x10000|450),
        GLSL_460       = (0x10000|460),
        ESSL           = (0x20000),
        ESSL_100       = (0x20000|100),
        ESSL_300       = (0x20000|300),
        ESSL_310       = (0x20000|310),
        ESSL_320       = (0x20000|320),
        HLSL           = (0x30000),
        HLSL_2_0       = (0x30000|200),
        HLSL_2_0a      = (0x30000|201),
        HLSL_2_0b      = (0x30000|202),
        HLSL_3_0       = (0x30000|300),
        HLSL_4_0       = (0x30000|400),
        HLSL_4_1       = (0x30000|410),
        HLSL_5_0       = (0x30000|500),
        HLSL_5_1       = (0x30000|510),
        HLSL_6_0       = (0x30000|600),
        HLSL_6_1       = (0x30000|610),
        HLSL_6_2       = (0x30000|620),
        HLSL_6_3       = (0x30000|630),
        HLSL_6_4       = (0x30000|640),
        HLSL_6_5       = (0x30000|650),
        HLSL_6_6       = (0x30000|660),
        HLSL_6_7       = (0x30000|670),
        HLSL_6_8       = (0x30000|680),
        Metal          = (0x40000),
        Metal_1_0      = (0x40000|100),
        Metal_1_1      = (0x40000|110),
        Metal_1_2      = (0x40000|120),
        Metal_2_0      = (0x40000|200),
        Metal_2_1      = (0x40000|210),
        Metal_2_2      = (0x40000|220),
        Metal_2_3      = (0x40000|230),
        Metal_2_4      = (0x40000|240),
        Metal_3_0      = (0x40000|300),
        SPIRV          = (0x50000),
        SPIRV_100      = (0x50000|100),
        VersionBitmask = 0x0000ffff,
    }

    public enum ScreenOrigin
    {
        LowerLeft,
        UpperLeft,
    }

    public enum ClippingRange
    {
        MinusOneToOne,
        ZeroToOne,
    }

    public enum CPUAccess
    {
        ReadOnly,
        WriteOnly,
        WriteDiscard,
        ReadWrite,
    }

    public enum ResourceType
    {
        Undefined,
        Buffer,
        Texture,
        Sampler,
    }

    public enum SamplerAddressMode
    {
        Repeat,
        Mirror,
        Clamp,
        Border,
        MirrorOnce,
    }

    public enum SamplerFilter
    {
        Nearest,
        Linear,
    }

    public enum ShaderType
    {
        Undefined,
        Vertex,
        TessControl,
        TessEvaluation,
        Geometry,
        Fragment,
        Compute,
    }

    public enum ShaderSourceType
    {
        CodeString,
        CodeFile,
        BinaryBuffer,
        BinaryFile,
    }

    public enum StorageBufferType
    {
        Undefined,
        TypedBuffer,
        StructuredBuffer,
        ByteAddressBuffer,
        RWTypedBuffer,
        RWStructuredBuffer,
        RWByteAddressBuffer,
        AppendStructuredBuffer,
        ConsumeStructuredBuffer,
    }

    public enum SystemValue
    {
        Undefined,
        ClipDistance,
        Color,
        CullDistance,
        Depth,
        DepthGreater,
        DepthLess,
        FrontFacing,
        InstanceID,
        Position,
        PrimitiveID,
        RenderTargetIndex,
        SampleMask,
        SampleID,
        Stencil,
        VertexID,
        ViewportIndex,
    }

    public enum TextureType
    {
        Texture1D,
        Texture2D,
        Texture3D,
        TextureCube,
        Texture1DArray,
        Texture2DArray,
        TextureCubeArray,
        Texture2DMS,
        Texture2DMSArray,
    }

    public enum TextureSwizzle
    {
        Zero,
        One,
        Red,
        Green,
        Blue,
        Alpha,
    }

    /* ----- Flags ----- */

    [Flags]
    public enum CanvasFlags : int
    {
        Borderless = (1 << 0),
    }

    [Flags]
    public enum CommandBufferFlags : int
    {
        Secondary       = (1 << 0),
        MultiSubmit     = (1 << 1),
        ImmediateSubmit = (1 << 2),
    }

    [Flags]
    public enum ClearFlags : int
    {
        Color        = (1 << 0),
        Depth        = (1 << 1),
        Stencil      = (1 << 2),
        ColorDepth   = (Color | Depth),
        DepthStencil = (Depth | Stencil),
        All          = (Color | Depth | Stencil),
    }

    [Flags]
    public enum FormatFlags : int
    {
        HasDepth             = (1 << 0),
        HasStencil           = (1 << 1),
        IsColorSpace_sRGB    = (1 << 2),
        IsCompressed         = (1 << 3),
        IsNormalized         = (1 << 4),
        IsInteger            = (1 << 5),
        IsUnsigned           = (1 << 6),
        IsPacked             = (1 << 7),
        SupportsRenderTarget = (1 << 8),
        SupportsMips         = (1 << 9),
        SupportsGenerateMips = (1 << 10),
        SupportsTexture1D    = (1 << 11),
        SupportsTexture2D    = (1 << 12),
        SupportsTexture3D    = (1 << 13),
        SupportsTextureCube  = (1 << 14),
        SupportsVertex       = (1 << 15),
        IsUnsignedInteger    = (IsUnsigned | IsInteger),
        HasDepthStencil      = (HasDepth | HasStencil),
    }

    [Flags]
    public enum StdOutFlags : int
    {
        Colored = (1 << 0),
    }

    [Flags]
    public enum ColorFlags : int
    {
        Default       = (1 << 0),
        Red           = (1 << 1),
        Green         = (1 << 2),
        Blue          = (1 << 3),
        Bright        = (1 << 4),
        Bold          = (1 << 5),
        Underline     = (1 << 6),
        FullRGB       = (1 << 7),
        Yellow        = (Red | Green),
        Pink          = (Red | Blue),
        Cyan          = (Green | Blue),
        Gray          = (Red | Green | Blue),
        BrightRed     = (Bright | Red),
        BrightGreen   = (Bright | Green),
        BrightBlue    = (Bright | Blue),
        BrightYellow  = (Bright | Yellow),
        BrightPink    = (Bright | Pink),
        BrightCyan    = (Bright | Cyan),
        White         = (Bright | Gray),
        StdError      = (Bold | Red),
        StdWarning    = (Bold | BrightYellow),
        StdAnnotation = (Bold | BrightPink),
    }

    [Flags]
    public enum BarrierFlags : int
    {
        StorageBuffer  = (1 << 0),
        StorageTexture = (1 << 1),
        Storage        = (StorageBuffer | StorageTexture),
    }

    [Flags]
    public enum ColorMaskFlags : int
    {
        Zero = 0,
        R    = (1 << 0),
        G    = (1 << 1),
        B    = (1 << 2),
        A    = (1 << 3),
        All  = (R | G | B | A),
    }

    [Flags]
    public enum RenderSystemFlags : int
    {
        DebugDevice  = (1 << 0),
        PreferNVIDIA = (1 << 1),
        PreferAMD    = (1 << 2),
        PreferIntel  = (1 << 3),
    }

    [Flags]
    public enum BindFlags : int
    {
        VertexBuffer           = (1 << 0),
        IndexBuffer            = (1 << 1),
        ConstantBuffer         = (1 << 2),
        StreamOutputBuffer     = (1 << 3),
        IndirectBuffer         = (1 << 4),
        Sampled                = (1 << 5),
        Storage                = (1 << 6),
        ColorAttachment        = (1 << 7),
        DepthStencilAttachment = (1 << 8),
        CombinedSampler        = (1 << 9),
        CopySrc                = (1 << 10),
        CopyDst                = (1 << 11),
    }

    [Flags]
    public enum CPUAccessFlags : int
    {
        Read      = (1 << 0),
        Write     = (1 << 1),
        ReadWrite = (Read | Write),
    }

    [Flags]
    public enum MiscFlags : int
    {
        DynamicUsage  = (1 << 0),
        FixedSamples  = (1 << 1),
        GenerateMips  = (1 << 2),
        NoInitialData = (1 << 3),
        Append        = (1 << 4),
        Counter       = (1 << 5),
    }

    [Flags]
    public enum ShaderCompileFlags : int
    {
        Debug               = (1 << 0),
        NoOptimization      = (1 << 1),
        OptimizationLevel1  = (1 << 2),
        OptimizationLevel2  = (1 << 3),
        OptimizationLevel3  = (1 << 4),
        WarningsAreErrors   = (1 << 5),
        PatchClippingOrigin = (1 << 6),
        SeparateShader      = (1 << 7),
        DefaultLibrary      = (1 << 8),
    }

    [Flags]
    public enum StageFlags : int
    {
        VertexStage         = (1 << 0),
        TessControlStage    = (1 << 1),
        TessEvaluationStage = (1 << 2),
        GeometryStage       = (1 << 3),
        FragmentStage       = (1 << 4),
        ComputeStage        = (1 << 5),
        AllTessStages       = (TessControlStage | TessEvaluationStage),
        AllGraphicsStages   = (VertexStage | AllTessStages | GeometryStage | FragmentStage),
        AllStages           = (AllGraphicsStages | ComputeStage),
    }

    [Flags]
    public enum ResizeBuffersFlags : int
    {
        AdaptSurface   = (1 << 0),
        FullscreenMode = (1 << 1),
        WindowedMode   = (1 << 2),
    }

    [Flags]
    public enum WindowFlags : int
    {
        Visible              = (1 << 0),
        Borderless           = (1 << 1),
        Resizable            = (1 << 2),
        Centered             = (1 << 3),
        AcceptDropFiles      = (1 << 4),
        DisableClearOnResize = (1 << 5),
        DisableSizeScaling   = (1 << 6),
    }

    /* ----- Structures ----- */

    public struct DrawIndirectArguments
    {
        public int NumVertices { get; set; }
        public int NumInstances { get; set; }
        public int FirstVertex { get; set; }
        public int FirstInstance { get; set; }
    }

    public struct DrawIndexedIndirectArguments
    {
        public int NumIndices { get; set; }
        public int NumInstances { get; set; }
        public int FirstIndex { get; set; }
        public int VertexOffset { get; set; }
        public int FirstInstance { get; set; }
    }

    public struct DrawPatchIndirectArguments
    {
        public int NumPatches { get; set; }
        public int NumInstances { get; set; }
        public int FirstPatch { get; set; }
        public int FirstInstance { get; set; }
    }

    public struct BindingSlot
    {
        public BindingSlot(int index = 0, int set = 0)
        {
            Index = index;
            Set   = set;
        }

        public int Index { get; set; } /* = 0 */
        public int Set { get; set; }   /* = 0 */
    }

    public struct Viewport
    {
        public Viewport(float x = 0.0f, float y = 0.0f, float width = 0.0f, float height = 0.0f, float minDepth = 0.0f, float maxDepth = 1.0f)
        {
            X        = x;
            Y        = y;
            Width    = width;
            Height   = height;
            MinDepth = minDepth;
            MaxDepth = maxDepth;
        }

        public float X { get; set; }        /* = 0.0f */
        public float Y { get; set; }        /* = 0.0f */
        public float Width { get; set; }    /* = 0.0f */
        public float Height { get; set; }   /* = 0.0f */
        public float MinDepth { get; set; } /* = 0.0f */
        public float MaxDepth { get; set; } /* = 1.0f */
    }

    public struct Scissor
    {
        public Scissor(int x = 0, int y = 0, int width = 0, int height = 0)
        {
            X      = x;
            Y      = y;
            Width  = width;
            Height = height;
        }

        public int X { get; set; }      /* = 0 */
        public int Y { get; set; }      /* = 0 */
        public int Width { get; set; }  /* = 0 */
        public int Height { get; set; } /* = 0 */
    }

    public struct QueryPipelineStatistics
    {
        public long InputAssemblyVertices { get; set; }           /* = 0 */
        public long InputAssemblyPrimitives { get; set; }         /* = 0 */
        public long VertexShaderInvocations { get; set; }         /* = 0 */
        public long GeometryShaderInvocations { get; set; }       /* = 0 */
        public long GeometryShaderPrimitives { get; set; }        /* = 0 */
        public long ClippingInvocations { get; set; }             /* = 0 */
        public long ClippingPrimitives { get; set; }              /* = 0 */
        public long FragmentShaderInvocations { get; set; }       /* = 0 */
        public long TessControlShaderInvocations { get; set; }    /* = 0 */
        public long TessEvaluationShaderInvocations { get; set; } /* = 0 */
        public long ComputeShaderInvocations { get; set; }        /* = 0 */
    }

    public struct TextureSubresource
    {
        public int BaseArrayLayer { get; set; } /* = 0 */
        public int NumArrayLayers { get; set; } /* = 1 */
        public int BaseMipLevel { get; set; }   /* = 0 */
        public int NumMipLevels { get; set; }   /* = 1 */
    }

    public struct SubresourceFootprint
    {
        public long Size { get; set; }         /* = 0 */
        public int  RowAlignment { get; set; } /* = 0 */
        public int  RowSize { get; set; }      /* = 0 */
        public int  RowStride { get; set; }    /* = 0 */
        public int  LayerSize { get; set; }    /* = 0 */
        public int  LayerStride { get; set; }  /* = 0 */
    }

    public struct Extent2D
    {
        public Extent2D(int width = 0, int height = 0)
        {
            Width  = width;
            Height = height;
        }

        public int Width { get; set; }  /* = 0 */
        public int Height { get; set; } /* = 0 */
    }

    public struct Extent3D
    {
        public Extent3D(int width = 0, int height = 0, int depth = 0)
        {
            Width  = width;
            Height = height;
            Depth  = depth;
        }

        public int Width { get; set; }  /* = 0 */
        public int Height { get; set; } /* = 0 */
        public int Depth { get; set; }  /* = 0 */
    }

    public struct Offset2D
    {
        public Offset2D(int x = 0, int y = 0)
        {
            X = x;
            Y = y;
        }

        public int X { get; set; } /* = 0 */
        public int Y { get; set; } /* = 0 */
    }

    public struct Offset3D
    {
        public Offset3D(int x = 0, int y = 0, int z = 0)
        {
            X = x;
            Y = y;
            Z = z;
        }

        public int X { get; set; } /* = 0 */
        public int Y { get; set; } /* = 0 */
        public int Z { get; set; } /* = 0 */
    }

    public struct FormatAttributes
    {
        public short       BitSize { get; set; }
        public byte        BlockWidth { get; set; }
        public byte        BlockHeight { get; set; }
        public byte        Components { get; set; }
        public ImageFormat Format { get; set; }
        public DataType    DataType { get; set; }
        public int         Flags { get; set; }
    }

    public struct TextureSwizzleRGBA
    {
        public TextureSwizzle R { get; set; } /* = TextureSwizzle.Red */
        public TextureSwizzle G { get; set; } /* = TextureSwizzle.Green */
        public TextureSwizzle B { get; set; } /* = TextureSwizzle.Blue */
        public TextureSwizzle A { get; set; } /* = TextureSwizzle.Alpha */
    }

    public struct TextureLocation
    {
        public Offset3D Offset { get; set; }     /* = new Offset3D() */
        public int      ArrayLayer { get; set; } /* = 0 */
        public int      MipLevel { get; set; }   /* = 0 */
    }

    public struct TextureRegion
    {
        public TextureSubresource Subresource { get; set; } /* = new TextureSubresource() */
        public Offset3D           Offset { get; set; }      /* = new Offset3D() */
        public Extent3D           Extent { get; set; }      /* = new Extent3D() */
    }

    /* ----- Classes ----- */

    public class CommandBufferDescriptor
    {
        public AnsiString         DebugName { get; set; }          = null;
        public CommandBufferFlags Flags { get; set; }              = 0;
        public int                NumNativeBuffers { get; set; }   = 0;
        public long               MinStagingPoolSize { get; set; } = (0xFFFF+1);
        public RenderPass         RenderPass { get; set; }         = null;

        internal NativeLLGL.CommandBufferDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.CommandBufferDescriptor();
                unsafe
                {
                    fixed (byte* debugNamePtr = DebugName.Ascii)
                    {
                        native.debugName = debugNamePtr;
                    }
                    native.flags              = (int)Flags;
                    native.numNativeBuffers   = NumNativeBuffers;
                    native.minStagingPoolSize = MinStagingPoolSize;
                    if (RenderPass != null)
                    {
                        native.renderPass = RenderPass.Native;
                    }
                }
                return native;
            }
        }
    }

    public class ColorCodes
    {
        public ColorFlags TextFlags { get; set; }       = 0;
        public ColorFlags BackgroundFlags { get; set; } = 0;

        public ColorCodes() { }

        internal ColorCodes(NativeLLGL.ColorCodes native)
        {
            Native = native;
        }

        internal NativeLLGL.ColorCodes Native
        {
            get
            {
                var native = new NativeLLGL.ColorCodes();
                native.textFlags       = (int)TextFlags;
                native.backgroundFlags = (int)BackgroundFlags;
                return native;
            }
            set
            {
                TextFlags       = (ColorFlags)value.textFlags;
                BackgroundFlags = (ColorFlags)value.backgroundFlags;
            }
        }
    }

    public class DepthBiasDescriptor
    {
        public float ConstantFactor { get; set; } = 0.0f;
        public float SlopeFactor { get; set; }    = 0.0f;
        public float Clamp { get; set; }          = 0.0f;

        internal NativeLLGL.DepthBiasDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.DepthBiasDescriptor();
                native.constantFactor = ConstantFactor;
                native.slopeFactor    = SlopeFactor;
                native.clamp          = Clamp;
                return native;
            }
        }
    }

    public class ComputePipelineDescriptor
    {
        public AnsiString     DebugName { get; set; }      = null;
        public PipelineLayout PipelineLayout { get; set; } = null;
        public Shader         ComputeShader { get; set; }  = null;

        internal NativeLLGL.ComputePipelineDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.ComputePipelineDescriptor();
                unsafe
                {
                    fixed (byte* debugNamePtr = DebugName.Ascii)
                    {
                        native.debugName = debugNamePtr;
                    }
                    if (PipelineLayout != null)
                    {
                        native.pipelineLayout = PipelineLayout.Native;
                    }
                    if (ComputeShader != null)
                    {
                        native.computeShader = ComputeShader.Native;
                    }
                }
                return native;
            }
        }
    }

    public class ProfileTimeRecord
    {
        public AnsiString Annotation { get; set; }    = "";
        public long       CPUTicksStart { get; set; } = 0;
        public long       CPUTicksEnd { get; set; }   = 0;
        public long       ElapsedTime { get; set; }   = 0;

        public ProfileTimeRecord() { }

        internal ProfileTimeRecord(NativeLLGL.ProfileTimeRecord native)
        {
            Native = native;
        }

        internal NativeLLGL.ProfileTimeRecord Native
        {
            get
            {
                var native = new NativeLLGL.ProfileTimeRecord();
                unsafe
                {
                    fixed (byte* annotationPtr = Annotation.Ascii)
                    {
                        native.annotation = annotationPtr;
                    }
                    native.cpuTicksStart = CPUTicksStart;
                    native.cpuTicksEnd   = CPUTicksEnd;
                    native.elapsedTime   = ElapsedTime;
                }
                return native;
            }
            set
            {
                unsafe
                {
                    Annotation    = Marshal.PtrToStringAnsi((IntPtr)value.annotation);
                    CPUTicksStart = value.cpuTicksStart;
                    CPUTicksEnd   = value.cpuTicksEnd;
                    ElapsedTime   = value.elapsedTime;
                }
            }
        }
    }

    public class ProfileCommandQueueRecord
    {
        public int BufferWrites { get; set; }             = 0;
        public int BufferReads { get; set; }              = 0;
        public int BufferMappings { get; set; }           = 0;
        public int TextureWrites { get; set; }            = 0;
        public int TextureReads { get; set; }             = 0;
        public int CommandBufferSubmittions { get; set; } = 0;
        public int FenceSubmissions { get; set; }         = 0;

        public ProfileCommandQueueRecord() { }

        internal ProfileCommandQueueRecord(NativeLLGL.ProfileCommandQueueRecord native)
        {
            Native = native;
        }

        internal NativeLLGL.ProfileCommandQueueRecord Native
        {
            set
            {
                BufferWrites             = value.bufferWrites;
                BufferReads              = value.bufferReads;
                BufferMappings           = value.bufferMappings;
                TextureWrites            = value.textureWrites;
                TextureReads             = value.textureReads;
                CommandBufferSubmittions = value.commandBufferSubmittions;
                FenceSubmissions         = value.fenceSubmissions;
            }
        }
    }

    public class ProfileCommandBufferRecord
    {
        public int Encodings { get; set; }                = 0;
        public int MipMapsGenerations { get; set; }       = 0;
        public int VertexBufferBindings { get; set; }     = 0;
        public int IndexBufferBindings { get; set; }      = 0;
        public int ConstantBufferBindings { get; set; }   = 0;
        public int SampledBufferBindings { get; set; }    = 0;
        public int StorageBufferBindings { get; set; }    = 0;
        public int SampledTextureBindings { get; set; }   = 0;
        public int StorageTextureBindings { get; set; }   = 0;
        public int SamplerBindings { get; set; }          = 0;
        public int ResourceHeapBindings { get; set; }     = 0;
        public int GraphicsPipelineBindings { get; set; } = 0;
        public int ComputePipelineBindings { get; set; }  = 0;
        public int AttachmentClears { get; set; }         = 0;
        public int BufferUpdates { get; set; }            = 0;
        public int BufferCopies { get; set; }             = 0;
        public int BufferFills { get; set; }              = 0;
        public int TextureCopies { get; set; }            = 0;
        public int RenderPassSections { get; set; }       = 0;
        public int StreamOutputSections { get; set; }     = 0;
        public int QuerySections { get; set; }            = 0;
        public int RenderConditionSections { get; set; }  = 0;
        public int DrawCommands { get; set; }             = 0;
        public int DispatchCommands { get; set; }         = 0;

        public ProfileCommandBufferRecord() { }

        internal ProfileCommandBufferRecord(NativeLLGL.ProfileCommandBufferRecord native)
        {
            Native = native;
        }

        internal NativeLLGL.ProfileCommandBufferRecord Native
        {
            set
            {
                Encodings                = value.encodings;
                MipMapsGenerations       = value.mipMapsGenerations;
                VertexBufferBindings     = value.vertexBufferBindings;
                IndexBufferBindings      = value.indexBufferBindings;
                ConstantBufferBindings   = value.constantBufferBindings;
                SampledBufferBindings    = value.sampledBufferBindings;
                StorageBufferBindings    = value.storageBufferBindings;
                SampledTextureBindings   = value.sampledTextureBindings;
                StorageTextureBindings   = value.storageTextureBindings;
                SamplerBindings          = value.samplerBindings;
                ResourceHeapBindings     = value.resourceHeapBindings;
                GraphicsPipelineBindings = value.graphicsPipelineBindings;
                ComputePipelineBindings  = value.computePipelineBindings;
                AttachmentClears         = value.attachmentClears;
                BufferUpdates            = value.bufferUpdates;
                BufferCopies             = value.bufferCopies;
                BufferFills              = value.bufferFills;
                TextureCopies            = value.textureCopies;
                RenderPassSections       = value.renderPassSections;
                StreamOutputSections     = value.streamOutputSections;
                QuerySections            = value.querySections;
                RenderConditionSections  = value.renderConditionSections;
                DrawCommands             = value.drawCommands;
                DispatchCommands         = value.dispatchCommands;
            }
        }
    }

    public class RenderingFeatures
    {
        public bool HasRenderTargets { get; set; }             = false;
        public bool Has3DTextures { get; set; }                = false;
        public bool HasCubeTextures { get; set; }              = false;
        public bool HasArrayTextures { get; set; }             = false;
        public bool HasCubeArrayTextures { get; set; }         = false;
        public bool HasMultiSampleTextures { get; set; }       = false;
        public bool HasMultiSampleArrayTextures { get; set; }  = false;
        public bool HasTextureViews { get; set; }              = false;
        public bool HasTextureViewSwizzle { get; set; }        = false;
        public bool HasTextureViewFormatSwizzle { get; set; }  = false;
        public bool HasBufferViews { get; set; }               = false;
        [Obsolete("LLGL.RenderingFeatures.hasSamplers is deprecated since 0.04b; All backends must support sampler states either natively or emulated.")]
        public bool HasSamplers { get; set; }                  = true;
        public bool HasConstantBuffers { get; set; }           = false;
        public bool HasStorageBuffers { get; set; }            = false;
        [Obsolete("LLGL.RenderingFeatures.hasUniforms is deprecated since 0.04b; All backends must support uniforms either natively or emulated.")]
        public bool HasUniforms { get; set; }                  = true;
        public bool HasGeometryShaders { get; set; }           = false;
        public bool HasTessellationShaders { get; set; }       = false;
        public bool HasTessellatorStage { get; set; }          = false;
        public bool HasComputeShaders { get; set; }            = false;
        public bool HasInstancing { get; set; }                = false;
        public bool HasOffsetInstancing { get; set; }          = false;
        public bool HasIndirectDrawing { get; set; }           = false;
        public bool HasViewportArrays { get; set; }            = false;
        public bool HasConservativeRasterization { get; set; } = false;
        public bool HasStreamOutputs { get; set; }             = false;
        public bool HasLogicOp { get; set; }                   = false;
        public bool HasPipelineCaching { get; set; }           = false;
        public bool HasPipelineStatistics { get; set; }        = false;
        public bool HasRenderCondition { get; set; }           = false;

        public RenderingFeatures() { }

        internal RenderingFeatures(NativeLLGL.RenderingFeatures native)
        {
            Native = native;
        }

        internal NativeLLGL.RenderingFeatures Native
        {
            set
            {
                HasRenderTargets             = value.hasRenderTargets;
                Has3DTextures                = value.has3DTextures;
                HasCubeTextures              = value.hasCubeTextures;
                HasArrayTextures             = value.hasArrayTextures;
                HasCubeArrayTextures         = value.hasCubeArrayTextures;
                HasMultiSampleTextures       = value.hasMultiSampleTextures;
                HasMultiSampleArrayTextures  = value.hasMultiSampleArrayTextures;
                HasTextureViews              = value.hasTextureViews;
                HasTextureViewSwizzle        = value.hasTextureViewSwizzle;
                HasTextureViewFormatSwizzle  = value.hasTextureViewFormatSwizzle;
                HasBufferViews               = value.hasBufferViews;
                HasConstantBuffers           = value.hasConstantBuffers;
                HasStorageBuffers            = value.hasStorageBuffers;
                HasGeometryShaders           = value.hasGeometryShaders;
                HasTessellationShaders       = value.hasTessellationShaders;
                HasTessellatorStage          = value.hasTessellatorStage;
                HasComputeShaders            = value.hasComputeShaders;
                HasInstancing                = value.hasInstancing;
                HasOffsetInstancing          = value.hasOffsetInstancing;
                HasIndirectDrawing           = value.hasIndirectDrawing;
                HasViewportArrays            = value.hasViewportArrays;
                HasConservativeRasterization = value.hasConservativeRasterization;
                HasStreamOutputs             = value.hasStreamOutputs;
                HasLogicOp                   = value.hasLogicOp;
                HasPipelineCaching           = value.hasPipelineCaching;
                HasPipelineStatistics        = value.hasPipelineStatistics;
                HasRenderCondition           = value.hasRenderCondition;
            }
        }
    }

    public class RenderingLimits
    {
        public float[] LineWidthRange { get; set; }                = new float[]{ 1.0f, 1.0f };
        public int     MaxTextureArrayLayers { get; set; }         = 0;
        public int     MaxColorAttachments { get; set; }           = 0;
        public int     MaxPatchVertices { get; set; }              = 0;
        public int     Max1DTextureSize { get; set; }              = 0;
        public int     Max2DTextureSize { get; set; }              = 0;
        public int     Max3DTextureSize { get; set; }              = 0;
        public int     MaxCubeTextureSize { get; set; }            = 0;
        public int     MaxAnisotropy { get; set; }                 = 0;
        public int[]   MaxComputeShaderWorkGroups { get; set; }    = new int[]{ 0, 0, 0 };
        public int[]   MaxComputeShaderWorkGroupSize { get; set; } = new int[]{ 0, 0, 0 };
        public int     MaxViewports { get; set; }                  = 0;
        public int[]   MaxViewportSize { get; set; }               = new int[]{ 0, 0 };
        public long    MaxBufferSize { get; set; }                 = 0;
        public long    MaxConstantBufferSize { get; set; }         = 0;
        public int     MaxStreamOutputs { get; set; }              = 0;
        public int     MaxTessFactor { get; set; }                 = 0;
        public long    MinConstantBufferAlignment { get; set; }    = 0;
        public long    MinSampledBufferAlignment { get; set; }     = 0;
        public long    MinStorageBufferAlignment { get; set; }     = 0;
        public int     MaxColorBufferSamples { get; set; }         = 0;
        public int     MaxDepthBufferSamples { get; set; }         = 0;
        public int     MaxStencilBufferSamples { get; set; }       = 0;
        public int     MaxNoAttachmentSamples { get; set; }        = 0;

        public RenderingLimits() { }

        internal RenderingLimits(NativeLLGL.RenderingLimits native)
        {
            Native = native;
        }

        internal NativeLLGL.RenderingLimits Native
        {
            set
            {
                unsafe
                {
                    LineWidthRange[0]                = value.lineWidthRange[0];
                    LineWidthRange[1]                = value.lineWidthRange[1];
                    MaxTextureArrayLayers            = value.maxTextureArrayLayers;
                    MaxColorAttachments              = value.maxColorAttachments;
                    MaxPatchVertices                 = value.maxPatchVertices;
                    Max1DTextureSize                 = value.max1DTextureSize;
                    Max2DTextureSize                 = value.max2DTextureSize;
                    Max3DTextureSize                 = value.max3DTextureSize;
                    MaxCubeTextureSize               = value.maxCubeTextureSize;
                    MaxAnisotropy                    = value.maxAnisotropy;
                    MaxComputeShaderWorkGroups[0]    = value.maxComputeShaderWorkGroups[0];
                    MaxComputeShaderWorkGroups[1]    = value.maxComputeShaderWorkGroups[1];
                    MaxComputeShaderWorkGroups[2]    = value.maxComputeShaderWorkGroups[2];
                    MaxComputeShaderWorkGroupSize[0] = value.maxComputeShaderWorkGroupSize[0];
                    MaxComputeShaderWorkGroupSize[1] = value.maxComputeShaderWorkGroupSize[1];
                    MaxComputeShaderWorkGroupSize[2] = value.maxComputeShaderWorkGroupSize[2];
                    MaxViewports                     = value.maxViewports;
                    MaxViewportSize[0]               = value.maxViewportSize[0];
                    MaxViewportSize[1]               = value.maxViewportSize[1];
                    MaxBufferSize                    = value.maxBufferSize;
                    MaxConstantBufferSize            = value.maxConstantBufferSize;
                    MaxStreamOutputs                 = value.maxStreamOutputs;
                    MaxTessFactor                    = value.maxTessFactor;
                    MinConstantBufferAlignment       = value.minConstantBufferAlignment;
                    MinSampledBufferAlignment        = value.minSampledBufferAlignment;
                    MinStorageBufferAlignment        = value.minStorageBufferAlignment;
                    MaxColorBufferSamples            = value.maxColorBufferSamples;
                    MaxDepthBufferSamples            = value.maxDepthBufferSamples;
                    MaxStencilBufferSamples          = value.maxStencilBufferSamples;
                    MaxNoAttachmentSamples           = value.maxNoAttachmentSamples;
                }
            }
        }
    }

    public class ResourceHeapDescriptor
    {
        public AnsiString     DebugName { get; set; }        = null;
        public PipelineLayout PipelineLayout { get; set; }   = null;
        public int            NumResourceViews { get; set; } = 0;
        [Obsolete("ResourceHeapDescriptor.barrierFlags is deprecated since 0.04b; Use PipelineLayoutDescriptor.barrierFlags instead!")]
        public BarrierFlags   BarrierFlags { get; set; }     = 0;

        internal NativeLLGL.ResourceHeapDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.ResourceHeapDescriptor();
                unsafe
                {
                    fixed (byte* debugNamePtr = DebugName.Ascii)
                    {
                        native.debugName = debugNamePtr;
                    }
                    if (PipelineLayout != null)
                    {
                        native.pipelineLayout = PipelineLayout.Native;
                    }
                    native.numResourceViews = NumResourceViews;
                }
                return native;
            }
        }
    }

    public class ShaderMacro
    {
        public ShaderMacro(string name = null, string definition = null)
        {
            Name       = name;
            Definition = definition;
        }

        public AnsiString Name { get; set; }       = null;
        public AnsiString Definition { get; set; } = null;

        internal NativeLLGL.ShaderMacro Native
        {
            get
            {
                var native = new NativeLLGL.ShaderMacro();
                unsafe
                {
                    fixed (byte* namePtr = Name.Ascii)
                    {
                        native.name = namePtr;
                    }
                    fixed (byte* definitionPtr = Definition.Ascii)
                    {
                        native.definition = definitionPtr;
                    }
                }
                return native;
            }
        }
    }

    public class BufferViewDescriptor
    {
        public Format Format { get; set; } = Format.Undefined;
        public long   Offset { get; set; } = 0;
        public long   Size { get; set; }   = -1;

        internal NativeLLGL.BufferViewDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.BufferViewDescriptor();
                native.format = Format;
                native.offset = Offset;
                native.size   = Size;
                return native;
            }
        }
    }

    public class AttachmentClear
    {
        public ClearFlags Flags { get; set; }           = 0;
        public int        ColorAttachment { get; set; } = 0;
        public ClearValue ClearValue { get; set; }      = new ClearValue();

        internal NativeLLGL.AttachmentClear Native
        {
            get
            {
                var native = new NativeLLGL.AttachmentClear();
                native.flags           = (int)Flags;
                native.colorAttachment = ColorAttachment;
                if (ClearValue != null)
                {
                    native.clearValue = ClearValue.Native;
                }
                return native;
            }
        }
    }

    public class DisplayMode
    {
        public Extent2D Resolution { get; set; }  = new Extent2D();
        public int      RefreshRate { get; set; } = 0;

        public DisplayMode() { }

        internal DisplayMode(NativeLLGL.DisplayMode native)
        {
            Native = native;
        }

        internal NativeLLGL.DisplayMode Native
        {
            get
            {
                var native = new NativeLLGL.DisplayMode();
                native.resolution  = Resolution;
                native.refreshRate = RefreshRate;
                return native;
            }
            set
            {
                Resolution  = value.resolution;
                RefreshRate = value.refreshRate;
            }
        }
    }

    public class FragmentAttribute
    {
        public FragmentAttribute(string name = null, Format format = Format.RGBA32Float, int location = 0, SystemValue systemValue = SystemValue.Undefined)
        {
            Name        = name;
            Format      = format;
            Location    = location;
            SystemValue = systemValue;
        }

        public AnsiString  Name { get; set; }
        public Format      Format { get; set; }      = Format.RGBA32Float;
        public int         Location { get; set; }    = 0;
        public SystemValue SystemValue { get; set; } = SystemValue.Undefined;

        public FragmentAttribute() { }

        internal FragmentAttribute(NativeLLGL.FragmentAttribute native)
        {
            Native = native;
        }

        internal NativeLLGL.FragmentAttribute Native
        {
            get
            {
                var native = new NativeLLGL.FragmentAttribute();
                unsafe
                {
                    fixed (byte* namePtr = Name.Ascii)
                    {
                        native.name = namePtr;
                    }
                    native.format      = Format;
                    native.location    = Location;
                    native.systemValue = SystemValue;
                }
                return native;
            }
            set
            {
                unsafe
                {
                    Name        = Marshal.PtrToStringAnsi((IntPtr)value.name);
                    Format      = value.format;
                    Location    = value.location;
                    SystemValue = value.systemValue;
                }
            }
        }
    }

    public class BindingDescriptor
    {
        public BindingDescriptor() { }

        public BindingDescriptor(string name = null, ResourceType type = ResourceType.Undefined, BindFlags bindFlags = 0, StageFlags stageFlags = 0, BindingSlot slot = new BindingSlot(), int arraySize = 0)
        {
            Name       = name;
            Type       = type;
            BindFlags  = bindFlags;
            StageFlags = stageFlags;
            Slot       = slot;
            ArraySize  = arraySize;
        }

        public AnsiString   Name { get; set; }
        public ResourceType Type { get; set; }       = ResourceType.Undefined;
        public BindFlags    BindFlags { get; set; }  = 0;
        public StageFlags   StageFlags { get; set; } = 0;
        public BindingSlot  Slot { get; set; }       = new BindingSlot();
        public int          ArraySize { get; set; }  = 0;

        internal BindingDescriptor(NativeLLGL.BindingDescriptor native)
        {
            Native = native;
        }

        internal NativeLLGL.BindingDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.BindingDescriptor();
                unsafe
                {
                    fixed (byte* namePtr = Name.Ascii)
                    {
                        native.name = namePtr;
                    }
                    native.type       = Type;
                    native.bindFlags  = (int)BindFlags;
                    native.stageFlags = (int)StageFlags;
                    native.slot       = Slot;
                    native.arraySize  = ArraySize;
                }
                return native;
            }
            set
            {
                unsafe
                {
                    Name       = Marshal.PtrToStringAnsi((IntPtr)value.name);
                    Type       = value.type;
                    BindFlags  = (BindFlags)value.bindFlags;
                    StageFlags = (StageFlags)value.stageFlags;
                    Slot       = value.slot;
                    ArraySize  = value.arraySize;
                }
            }
        }
    }

    public class UniformDescriptor
    {
        public AnsiString  Name { get; set; }
        public UniformType Type { get; set; }      = UniformType.Undefined;
        public int         ArraySize { get; set; } = 0;

        public UniformDescriptor() { }

        internal UniformDescriptor(NativeLLGL.UniformDescriptor native)
        {
            Native = native;
        }

        internal NativeLLGL.UniformDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.UniformDescriptor();
                unsafe
                {
                    fixed (byte* namePtr = Name.Ascii)
                    {
                        native.name = namePtr;
                    }
                    native.type      = Type;
                    native.arraySize = ArraySize;
                }
                return native;
            }
            set
            {
                unsafe
                {
                    Name      = Marshal.PtrToStringAnsi((IntPtr)value.name);
                    Type      = value.type;
                    ArraySize = value.arraySize;
                }
            }
        }
    }

    public class CombinedTextureSamplerDescriptor
    {
        public AnsiString  Name { get; set; }
        public AnsiString  TextureName { get; set; }
        public AnsiString  SamplerName { get; set; }
        public BindingSlot Slot { get; set; }        = new BindingSlot();

        public CombinedTextureSamplerDescriptor() { }

        internal CombinedTextureSamplerDescriptor(NativeLLGL.CombinedTextureSamplerDescriptor native)
        {
            Native = native;
        }

        internal NativeLLGL.CombinedTextureSamplerDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.CombinedTextureSamplerDescriptor();
                unsafe
                {
                    fixed (byte* namePtr = Name.Ascii)
                    {
                        native.name = namePtr;
                    }
                    fixed (byte* textureNamePtr = TextureName.Ascii)
                    {
                        native.textureName = textureNamePtr;
                    }
                    fixed (byte* samplerNamePtr = SamplerName.Ascii)
                    {
                        native.samplerName = samplerNamePtr;
                    }
                    native.slot        = Slot;
                }
                return native;
            }
            set
            {
                unsafe
                {
                    Name        = Marshal.PtrToStringAnsi((IntPtr)value.name);
                    TextureName = Marshal.PtrToStringAnsi((IntPtr)value.textureName);
                    SamplerName = Marshal.PtrToStringAnsi((IntPtr)value.samplerName);
                    Slot        = value.slot;
                }
            }
        }
    }

    public class DepthDescriptor
    {
        public bool      TestEnabled { get; set; }  = false;
        public bool      WriteEnabled { get; set; } = false;
        public CompareOp CompareOp { get; set; }    = CompareOp.Less;

        internal NativeLLGL.DepthDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.DepthDescriptor();
                native.testEnabled  = TestEnabled;
                native.writeEnabled = WriteEnabled;
                native.compareOp    = CompareOp;
                return native;
            }
        }
    }

    public class StencilFaceDescriptor
    {
        public StencilOp StencilFailOp { get; set; } = StencilOp.Keep;
        public StencilOp DepthFailOp { get; set; }   = StencilOp.Keep;
        public StencilOp DepthPassOp { get; set; }   = StencilOp.Keep;
        public CompareOp CompareOp { get; set; }     = CompareOp.Less;
        public int       ReadMask { get; set; }      = -1;
        public int       WriteMask { get; set; }     = -1;
        public int       Reference { get; set; }     = 0;

        internal NativeLLGL.StencilFaceDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.StencilFaceDescriptor();
                native.stencilFailOp = StencilFailOp;
                native.depthFailOp   = DepthFailOp;
                native.depthPassOp   = DepthPassOp;
                native.compareOp     = CompareOp;
                native.readMask      = ReadMask;
                native.writeMask     = WriteMask;
                native.reference     = Reference;
                return native;
            }
        }
    }

    public class RasterizerDescriptor
    {
        public PolygonMode         PolygonMode { get; set; }               = PolygonMode.Fill;
        public CullMode            CullMode { get; set; }                  = CullMode.Disabled;
        public DepthBiasDescriptor DepthBias { get; set; }                 = new DepthBiasDescriptor();
        public bool                FrontCCW { get; set; }                  = false;
        public bool                DiscardEnabled { get; set; }            = false;
        public bool                DepthClampEnabled { get; set; }         = false;
        public bool                ScissorTestEnabled { get; set; }        = false;
        public bool                MultiSampleEnabled { get; set; }        = false;
        public bool                AntiAliasedLineEnabled { get; set; }    = false;
        public bool                ConservativeRasterization { get; set; } = false;
        public float               LineWidth { get; set; }                 = 1.0f;

        internal NativeLLGL.RasterizerDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.RasterizerDescriptor();
                native.polygonMode               = PolygonMode;
                native.cullMode                  = CullMode;
                if (DepthBias != null)
                {
                    native.depthBias = DepthBias.Native;
                }
                native.frontCCW                  = FrontCCW;
                native.discardEnabled            = DiscardEnabled;
                native.depthClampEnabled         = DepthClampEnabled;
                native.scissorTestEnabled        = ScissorTestEnabled;
                native.multiSampleEnabled        = MultiSampleEnabled;
                native.antiAliasedLineEnabled    = AntiAliasedLineEnabled;
                native.conservativeRasterization = ConservativeRasterization;
                native.lineWidth                 = LineWidth;
                return native;
            }
        }
    }

    public class BlendTargetDescriptor
    {
        public bool            BlendEnabled { get; set; }    = false;
        public BlendOp         SrcColor { get; set; }        = BlendOp.SrcAlpha;
        public BlendOp         DstColor { get; set; }        = BlendOp.InvSrcAlpha;
        public BlendArithmetic ColorArithmetic { get; set; } = BlendArithmetic.Add;
        public BlendOp         SrcAlpha { get; set; }        = BlendOp.SrcAlpha;
        public BlendOp         DstAlpha { get; set; }        = BlendOp.InvSrcAlpha;
        public BlendArithmetic AlphaArithmetic { get; set; } = BlendArithmetic.Add;
        public ColorMaskFlags  ColorMask { get; set; }       = ColorMaskFlags.All;

        internal NativeLLGL.BlendTargetDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.BlendTargetDescriptor();
                native.blendEnabled    = BlendEnabled;
                native.srcColor        = SrcColor;
                native.dstColor        = DstColor;
                native.colorArithmetic = ColorArithmetic;
                native.srcAlpha        = SrcAlpha;
                native.dstAlpha        = DstAlpha;
                native.alphaArithmetic = AlphaArithmetic;
                native.colorMask       = (byte)ColorMask;
                return native;
            }
        }
    }

    public class TessellationDescriptor
    {
        public TessellationPartition Partition { get; set; }        = TessellationPartition.Undefined;
        public int                   MaxTessFactor { get; set; }    = 64;
        public bool                  OutputWindingCCW { get; set; } = false;

        internal NativeLLGL.TessellationDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.TessellationDescriptor();
                native.partition        = Partition;
                native.maxTessFactor    = MaxTessFactor;
                native.outputWindingCCW = OutputWindingCCW;
                return native;
            }
        }
    }

    public class QueryHeapDescriptor
    {
        public AnsiString DebugName { get; set; }       = null;
        public QueryType  Type { get; set; }            = QueryType.SamplesPassed;
        public int        NumQueries { get; set; }      = 1;
        public bool       RenderCondition { get; set; } = false;

        internal NativeLLGL.QueryHeapDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.QueryHeapDescriptor();
                unsafe
                {
                    fixed (byte* debugNamePtr = DebugName.Ascii)
                    {
                        native.debugName = debugNamePtr;
                    }
                    native.type            = Type;
                    native.numQueries      = NumQueries;
                    native.renderCondition = RenderCondition;
                }
                return native;
            }
        }
    }

    public class FrameProfile
    {
        public ProfileCommandQueueRecord  CommandQueueRecord { get; set; }  = new ProfileCommandQueueRecord();
        public ProfileCommandBufferRecord CommandBufferRecord { get; set; } = new ProfileCommandBufferRecord();
        private ProfileTimeRecord[] timeRecords;
        private NativeLLGL.ProfileTimeRecord[] timeRecordsNative;
        public ProfileTimeRecord[] TimeRecords
        {
            get
            {
                return timeRecords;
            }
            set
            {
                if (value != null)
                {
                    timeRecords = value;
                    timeRecordsNative = new NativeLLGL.ProfileTimeRecord[timeRecords.Length];
                    for (int timeRecordsIndex = 0; timeRecordsIndex < timeRecords.Length; ++timeRecordsIndex)
                    {
                        if (timeRecords[timeRecordsIndex] != null)
                        {
                            timeRecordsNative[timeRecordsIndex] = timeRecords[timeRecordsIndex].Native;
                        }
                    }
                }
                else
                {
                    timeRecords = null;
                    timeRecordsNative = null;
                }
            }
        }

        public FrameProfile() { }

        internal FrameProfile(NativeLLGL.FrameProfile native)
        {
            Native = native;
        }

        internal NativeLLGL.FrameProfile Native
        {
            set
            {
                unsafe
                {
                    CommandQueueRecord.Native= value.commandQueueRecord;
                    CommandBufferRecord.Native= value.commandBufferRecord;
                    TimeRecords         = new ProfileTimeRecord[(int)value.numTimeRecords];
                    for (int i = 0; i < TimeRecords.Length; ++i)
                    {
                        TimeRecords[i] = new ProfileTimeRecord(value.timeRecords[i]);
                    }
                }
            }
        }
    }

    public class AttachmentFormatDescriptor
    {
        public Format            Format { get; set; }  = Format.Undefined;
        public AttachmentLoadOp  LoadOp { get; set; }  = AttachmentLoadOp.Undefined;
        public AttachmentStoreOp StoreOp { get; set; } = AttachmentStoreOp.Undefined;

        internal NativeLLGL.AttachmentFormatDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.AttachmentFormatDescriptor();
                native.format  = Format;
                native.loadOp  = LoadOp;
                native.storeOp = StoreOp;
                return native;
            }
        }
    }

    public class RenderingCapabilities
    {
        public ScreenOrigin      ScreenOrigin { get; set; }     = ScreenOrigin.UpperLeft;
        public ClippingRange     ClippingRange { get; set; }    = ClippingRange.ZeroToOne;
        public ShadingLanguage[] ShadingLanguages { get; set; }
        public Format[]          TextureFormats { get; set; }
        public RenderingFeatures Features { get; set; }         = new RenderingFeatures();
        public RenderingLimits   Limits { get; set; }           = new RenderingLimits();

        public RenderingCapabilities() { }

        internal RenderingCapabilities(NativeLLGL.RenderingCapabilities native)
        {
            Native = native;
        }

        internal NativeLLGL.RenderingCapabilities Native
        {
            set
            {
                unsafe
                {
                    ScreenOrigin     = value.screenOrigin;
                    ClippingRange    = value.clippingRange;
                    ShadingLanguages = new ShadingLanguage[(int)value.numShadingLanguages];
                    for (int i = 0; i < ShadingLanguages.Length; ++i)
                    {
                        ShadingLanguages[i] = value.shadingLanguages[i];
                    }
                    TextureFormats   = new Format[(int)value.numTextureFormats];
                    for (int i = 0; i < TextureFormats.Length; ++i)
                    {
                        TextureFormats[i] = value.textureFormats[i];
                    }
                    Features.Native  = value.features;
                    Limits.Native    = value.limits;
                }
            }
        }
    }

    public class AttachmentDescriptor
    {
        public Format  Format { get; set; }     = Format.Undefined;
        public Texture Texture { get; set; }    = null;
        public int     MipLevel { get; set; }   = 0;
        public int     ArrayLayer { get; set; } = 0;

        internal NativeLLGL.AttachmentDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.AttachmentDescriptor();
                native.format     = Format;
                if (Texture != null)
                {
                    native.texture = Texture.Native;
                }
                native.mipLevel   = MipLevel;
                native.arrayLayer = ArrayLayer;
                return native;
            }
        }
    }

    public class SamplerDescriptor
    {
        public AnsiString         DebugName { get; set; }      = null;
        public SamplerAddressMode AddressModeU { get; set; }   = SamplerAddressMode.Repeat;
        public SamplerAddressMode AddressModeV { get; set; }   = SamplerAddressMode.Repeat;
        public SamplerAddressMode AddressModeW { get; set; }   = SamplerAddressMode.Repeat;
        public SamplerFilter      MinFilter { get; set; }      = SamplerFilter.Linear;
        public SamplerFilter      MagFilter { get; set; }      = SamplerFilter.Linear;
        public SamplerFilter      MipMapFilter { get; set; }   = SamplerFilter.Linear;
        public bool               MipMapEnabled { get; set; }  = true;
        public float              MipMapLODBias { get; set; }  = 0.0f;
        public float              MinLOD { get; set; }         = 0.0f;
        public float              MaxLOD { get; set; }         = 1000.0f;
        public int                MaxAnisotropy { get; set; }  = 1;
        public bool               CompareEnabled { get; set; } = false;
        public CompareOp          CompareOp { get; set; }      = CompareOp.Less;
        public float[]            BorderColor { get; set; }    = new float[]{ 0.0f, 0.0f, 0.0f, 0.0f };

        public SamplerDescriptor() { }

        internal SamplerDescriptor(NativeLLGL.SamplerDescriptor native)
        {
            Native = native;
        }

        internal NativeLLGL.SamplerDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.SamplerDescriptor();
                unsafe
                {
                    fixed (byte* debugNamePtr = DebugName.Ascii)
                    {
                        native.debugName = debugNamePtr;
                    }
                    native.addressModeU   = AddressModeU;
                    native.addressModeV   = AddressModeV;
                    native.addressModeW   = AddressModeW;
                    native.minFilter      = MinFilter;
                    native.magFilter      = MagFilter;
                    native.mipMapFilter   = MipMapFilter;
                    native.mipMapEnabled  = MipMapEnabled;
                    native.mipMapLODBias  = MipMapLODBias;
                    native.minLOD         = MinLOD;
                    native.maxLOD         = MaxLOD;
                    native.maxAnisotropy  = MaxAnisotropy;
                    native.compareEnabled = CompareEnabled;
                    native.compareOp      = CompareOp;
                    native.borderColor[0] = BorderColor[0];
                    native.borderColor[1] = BorderColor[1];
                    native.borderColor[2] = BorderColor[2];
                    native.borderColor[3] = BorderColor[3];
                }
                return native;
            }
            set
            {
                unsafe
                {
                    DebugName      = Marshal.PtrToStringAnsi((IntPtr)value.debugName);
                    AddressModeU   = value.addressModeU;
                    AddressModeV   = value.addressModeV;
                    AddressModeW   = value.addressModeW;
                    MinFilter      = value.minFilter;
                    MagFilter      = value.magFilter;
                    MipMapFilter   = value.mipMapFilter;
                    MipMapEnabled  = value.mipMapEnabled;
                    MipMapLODBias  = value.mipMapLODBias;
                    MinLOD         = value.minLOD;
                    MaxLOD         = value.maxLOD;
                    MaxAnisotropy  = value.maxAnisotropy;
                    CompareEnabled = value.compareEnabled;
                    CompareOp      = value.compareOp;
                    BorderColor[0] = value.borderColor[0];
                    BorderColor[1] = value.borderColor[1];
                    BorderColor[2] = value.borderColor[2];
                    BorderColor[3] = value.borderColor[3];
                }
            }
        }
    }

    public class ComputeShaderAttributes
    {
        public ComputeShaderAttributes() { }

        public ComputeShaderAttributes(Extent3D workGroupSize)
        {
            WorkGroupSize = workGroupSize;
        }

        public Extent3D WorkGroupSize { get; set; } = new Extent3D() { Width =  1, Height =  1, Depth =  1  };

        internal ComputeShaderAttributes(NativeLLGL.ComputeShaderAttributes native)
        {
            Native = native;
        }

        internal NativeLLGL.ComputeShaderAttributes Native
        {
            get
            {
                var native = new NativeLLGL.ComputeShaderAttributes();
                native.workGroupSize = WorkGroupSize;
                return native;
            }
            set
            {
                WorkGroupSize = value.workGroupSize;
            }
        }
    }

    public class SwapChainDescriptor
    {
        public SwapChainDescriptor() { }

        public SwapChainDescriptor(string debugName = null, Extent2D resolution = new Extent2D(), int colorBits = 32, int depthBits = 24, int stencilBits = 8, int samples = 1, int swapBuffers = 2, bool fullscreen = false)
        {
            DebugName   = debugName;
            Resolution  = resolution;
            ColorBits   = colorBits;
            DepthBits   = depthBits;
            StencilBits = stencilBits;
            Samples     = samples;
            SwapBuffers = swapBuffers;
            Fullscreen  = fullscreen;
        }

        public AnsiString DebugName { get; set; }   = null;
        public Extent2D   Resolution { get; set; }  = new Extent2D();
        public int        ColorBits { get; set; }   = 32;
        public int        DepthBits { get; set; }   = 24;
        public int        StencilBits { get; set; } = 8;
        public int        Samples { get; set; }     = 1;
        public int        SwapBuffers { get; set; } = 2;
        public bool       Fullscreen { get; set; }  = false;

        internal NativeLLGL.SwapChainDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.SwapChainDescriptor();
                unsafe
                {
                    fixed (byte* debugNamePtr = DebugName.Ascii)
                    {
                        native.debugName = debugNamePtr;
                    }
                    native.resolution  = Resolution;
                    native.colorBits   = ColorBits;
                    native.depthBits   = DepthBits;
                    native.stencilBits = StencilBits;
                    native.samples     = Samples;
                    native.swapBuffers = SwapBuffers;
                    native.fullscreen  = Fullscreen;
                }
                return native;
            }
        }
    }

    public class TextureDescriptor
    {
        public AnsiString     DebugName { get; set; }      = null;
        public TextureType    Type { get; set; }           = TextureType.Texture2D;
        public BindFlags      BindFlags { get; set; }      = (BindFlags.Sampled | BindFlags.ColorAttachment);
        public CPUAccessFlags CPUAccessFlags { get; set; } = (CPUAccessFlags.Read | CPUAccessFlags.Write);
        public MiscFlags      MiscFlags { get; set; }      = (MiscFlags.FixedSamples | MiscFlags.GenerateMips);
        public Format         Format { get; set; }         = Format.RGBA8UNorm;
        public Extent3D       Extent { get; set; }         = new Extent3D() { Width =  1, Height =  1, Depth =  1  };
        public int            ArrayLayers { get; set; }    = 1;
        public int            MipLevels { get; set; }      = 0;
        public int            Samples { get; set; }        = 1;
        public ClearValue     ClearValue { get; set; }     = new ClearValue();

        public TextureDescriptor() { }

        internal TextureDescriptor(NativeLLGL.TextureDescriptor native)
        {
            Native = native;
        }

        internal NativeLLGL.TextureDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.TextureDescriptor();
                unsafe
                {
                    fixed (byte* debugNamePtr = DebugName.Ascii)
                    {
                        native.debugName = debugNamePtr;
                    }
                    native.type           = Type;
                    native.bindFlags      = (int)BindFlags;
                    native.cpuAccessFlags = (int)CPUAccessFlags;
                    native.miscFlags      = (int)MiscFlags;
                    native.format         = Format;
                    native.extent         = Extent;
                    native.arrayLayers    = ArrayLayers;
                    native.mipLevels      = MipLevels;
                    native.samples        = Samples;
                    if (ClearValue != null)
                    {
                        native.clearValue = ClearValue.Native;
                    }
                }
                return native;
            }
            set
            {
                unsafe
                {
                    DebugName      = Marshal.PtrToStringAnsi((IntPtr)value.debugName);
                    Type           = value.type;
                    BindFlags      = (BindFlags)value.bindFlags;
                    CPUAccessFlags = (CPUAccessFlags)value.cpuAccessFlags;
                    MiscFlags      = (MiscFlags)value.miscFlags;
                    Format         = value.format;
                    Extent         = value.extent;
                    ArrayLayers    = value.arrayLayers;
                    MipLevels      = value.mipLevels;
                    Samples        = value.samples;
                    ClearValue.Native= value.clearValue;
                }
            }
        }
    }

    public class VertexAttribute
    {
        public VertexAttribute(string name = null, Format format = Format.RGBA32Float, int location = 0, int semanticIndex = 0, SystemValue systemValue = SystemValue.Undefined, int slot = 0, int offset = 0, int stride = 0, int instanceDivisor = 0)
        {
            Name            = name;
            Format          = format;
            Location        = location;
            SemanticIndex   = semanticIndex;
            SystemValue     = systemValue;
            Slot            = slot;
            Offset          = offset;
            Stride          = stride;
            InstanceDivisor = instanceDivisor;
        }

        public AnsiString  Name { get; set; }
        public Format      Format { get; set; }          = Format.RGBA32Float;
        public int         Location { get; set; }        = 0;
        public int         SemanticIndex { get; set; }   = 0;
        public SystemValue SystemValue { get; set; }     = SystemValue.Undefined;
        public int         Slot { get; set; }            = 0;
        public int         Offset { get; set; }          = 0;
        public int         Stride { get; set; }          = 0;
        public int         InstanceDivisor { get; set; } = 0;

        public VertexAttribute() { }

        internal VertexAttribute(NativeLLGL.VertexAttribute native)
        {
            Native = native;
        }

        internal NativeLLGL.VertexAttribute Native
        {
            get
            {
                var native = new NativeLLGL.VertexAttribute();
                unsafe
                {
                    fixed (byte* namePtr = Name.Ascii)
                    {
                        native.name = namePtr;
                    }
                    native.format          = Format;
                    native.location        = Location;
                    native.semanticIndex   = SemanticIndex;
                    native.systemValue     = SystemValue;
                    native.slot            = Slot;
                    native.offset          = Offset;
                    native.stride          = Stride;
                    native.instanceDivisor = InstanceDivisor;
                }
                return native;
            }
            set
            {
                unsafe
                {
                    Name            = Marshal.PtrToStringAnsi((IntPtr)value.name);
                    Format          = value.format;
                    Location        = value.location;
                    SemanticIndex   = value.semanticIndex;
                    SystemValue     = value.systemValue;
                    Slot            = value.slot;
                    Offset          = value.offset;
                    Stride          = value.stride;
                    InstanceDivisor = value.instanceDivisor;
                }
            }
        }
    }

    public class BufferDescriptor
    {
        public AnsiString        DebugName { get; set; }      = null;
        public long              Size { get; set; }           = 0;
        public int               Stride { get; set; }         = 0;
        public Format            Format { get; set; }         = Format.Undefined;
        public BindFlags         BindFlags { get; set; }      = 0;
        public CPUAccessFlags    CPUAccessFlags { get; set; } = 0;
        public MiscFlags         MiscFlags { get; set; }      = 0;
        private VertexAttribute[] vertexAttribs;
        private NativeLLGL.VertexAttribute[] vertexAttribsNative;
        public VertexAttribute[] VertexAttribs
        {
            get
            {
                return vertexAttribs;
            }
            set
            {
                if (value != null)
                {
                    vertexAttribs = value;
                    vertexAttribsNative = new NativeLLGL.VertexAttribute[vertexAttribs.Length];
                    for (int vertexAttribsIndex = 0; vertexAttribsIndex < vertexAttribs.Length; ++vertexAttribsIndex)
                    {
                        if (vertexAttribs[vertexAttribsIndex] != null)
                        {
                            vertexAttribsNative[vertexAttribsIndex] = vertexAttribs[vertexAttribsIndex].Native;
                        }
                    }
                }
                else
                {
                    vertexAttribs = null;
                    vertexAttribsNative = null;
                }
            }
        }

        public BufferDescriptor() { }

        internal BufferDescriptor(NativeLLGL.BufferDescriptor native)
        {
            Native = native;
        }

        internal NativeLLGL.BufferDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.BufferDescriptor();
                unsafe
                {
                    fixed (byte* debugNamePtr = DebugName.Ascii)
                    {
                        native.debugName = debugNamePtr;
                    }
                    native.size           = Size;
                    native.stride         = Stride;
                    native.format         = Format;
                    native.bindFlags      = (int)BindFlags;
                    native.cpuAccessFlags = (int)CPUAccessFlags;
                    native.miscFlags      = (int)MiscFlags;
                    if (vertexAttribs != null)
                    {
                        native.numVertexAttribs = (IntPtr)vertexAttribs.Length;
                        fixed (NativeLLGL.VertexAttribute* vertexAttribsPtr = vertexAttribsNative)
                        {
                            native.vertexAttribs = vertexAttribsPtr;
                        }
                    }
                }
                return native;
            }
            set
            {
                unsafe
                {
                    DebugName      = Marshal.PtrToStringAnsi((IntPtr)value.debugName);
                    Size           = value.size;
                    Stride         = value.stride;
                    Format         = value.format;
                    BindFlags      = (BindFlags)value.bindFlags;
                    CPUAccessFlags = (CPUAccessFlags)value.cpuAccessFlags;
                    MiscFlags      = (MiscFlags)value.miscFlags;
                    VertexAttribs  = new VertexAttribute[(int)value.numVertexAttribs];
                    for (int i = 0; i < VertexAttribs.Length; ++i)
                    {
                        VertexAttribs[i] = new VertexAttribute(value.vertexAttribs[i]);
                    }
                }
            }
        }
    }

    public class StaticSamplerDescriptor
    {
        public AnsiString        Name { get; set; }
        public StageFlags        StageFlags { get; set; } = 0;
        public BindingSlot       Slot { get; set; }       = new BindingSlot();
        public SamplerDescriptor Sampler { get; set; }    = new SamplerDescriptor();

        public StaticSamplerDescriptor() { }

        internal StaticSamplerDescriptor(NativeLLGL.StaticSamplerDescriptor native)
        {
            Native = native;
        }

        internal NativeLLGL.StaticSamplerDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.StaticSamplerDescriptor();
                unsafe
                {
                    fixed (byte* namePtr = Name.Ascii)
                    {
                        native.name = namePtr;
                    }
                    native.stageFlags = (int)StageFlags;
                    native.slot       = Slot;
                    if (Sampler != null)
                    {
                        native.sampler = Sampler.Native;
                    }
                }
                return native;
            }
            set
            {
                unsafe
                {
                    Name       = Marshal.PtrToStringAnsi((IntPtr)value.name);
                    StageFlags = (StageFlags)value.stageFlags;
                    Slot       = value.slot;
                    Sampler.Native= value.sampler;
                }
            }
        }
    }

    public class StencilDescriptor
    {
        public bool                  TestEnabled { get; set; }      = false;
        public bool                  ReferenceDynamic { get; set; } = false;
        public StencilFaceDescriptor Front { get; set; }            = new StencilFaceDescriptor();
        public StencilFaceDescriptor Back { get; set; }             = new StencilFaceDescriptor();

        internal NativeLLGL.StencilDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.StencilDescriptor();
                native.testEnabled      = TestEnabled;
                native.referenceDynamic = ReferenceDynamic;
                if (Front != null)
                {
                    native.front = Front.Native;
                }
                if (Back != null)
                {
                    native.back = Back.Native;
                }
                return native;
            }
        }
    }

    public class VertexShaderAttributes
    {
        public VertexShaderAttributes(VertexAttribute[] inputAttribs = null, VertexAttribute[] outputAttribs = null)
        {
            InputAttribs  = inputAttribs;
            OutputAttribs = outputAttribs;
        }

        private VertexAttribute[] inputAttribs;
        private NativeLLGL.VertexAttribute[] inputAttribsNative;
        public VertexAttribute[] InputAttribs
        {
            get
            {
                return inputAttribs;
            }
            set
            {
                if (value != null)
                {
                    inputAttribs = value;
                    inputAttribsNative = new NativeLLGL.VertexAttribute[inputAttribs.Length];
                    for (int inputAttribsIndex = 0; inputAttribsIndex < inputAttribs.Length; ++inputAttribsIndex)
                    {
                        if (inputAttribs[inputAttribsIndex] != null)
                        {
                            inputAttribsNative[inputAttribsIndex] = inputAttribs[inputAttribsIndex].Native;
                        }
                    }
                }
                else
                {
                    inputAttribs = null;
                    inputAttribsNative = null;
                }
            }
        }
        private VertexAttribute[] outputAttribs;
        private NativeLLGL.VertexAttribute[] outputAttribsNative;
        public VertexAttribute[] OutputAttribs
        {
            get
            {
                return outputAttribs;
            }
            set
            {
                if (value != null)
                {
                    outputAttribs = value;
                    outputAttribsNative = new NativeLLGL.VertexAttribute[outputAttribs.Length];
                    for (int outputAttribsIndex = 0; outputAttribsIndex < outputAttribs.Length; ++outputAttribsIndex)
                    {
                        if (outputAttribs[outputAttribsIndex] != null)
                        {
                            outputAttribsNative[outputAttribsIndex] = outputAttribs[outputAttribsIndex].Native;
                        }
                    }
                }
                else
                {
                    outputAttribs = null;
                    outputAttribsNative = null;
                }
            }
        }

        public VertexShaderAttributes() { }

        internal VertexShaderAttributes(NativeLLGL.VertexShaderAttributes native)
        {
            Native = native;
        }

        internal NativeLLGL.VertexShaderAttributes Native
        {
            get
            {
                var native = new NativeLLGL.VertexShaderAttributes();
                unsafe
                {
                    if (inputAttribs != null)
                    {
                        native.numInputAttribs = (IntPtr)inputAttribs.Length;
                        fixed (NativeLLGL.VertexAttribute* inputAttribsPtr = inputAttribsNative)
                        {
                            native.inputAttribs = inputAttribsPtr;
                        }
                    }
                    if (outputAttribs != null)
                    {
                        native.numOutputAttribs = (IntPtr)outputAttribs.Length;
                        fixed (NativeLLGL.VertexAttribute* outputAttribsPtr = outputAttribsNative)
                        {
                            native.outputAttribs = outputAttribsPtr;
                        }
                    }
                }
                return native;
            }
            set
            {
                unsafe
                {
                    InputAttribs  = new VertexAttribute[(int)value.numInputAttribs];
                    for (int i = 0; i < InputAttribs.Length; ++i)
                    {
                        InputAttribs[i] = new VertexAttribute(value.inputAttribs[i]);
                    }
                    OutputAttribs = new VertexAttribute[(int)value.numOutputAttribs];
                    for (int i = 0; i < OutputAttribs.Length; ++i)
                    {
                        OutputAttribs[i] = new VertexAttribute(value.outputAttribs[i]);
                    }
                }
            }
        }
    }

    public class FragmentShaderAttributes
    {
        public FragmentShaderAttributes(FragmentAttribute[] outputAttribs = null)
        {
            OutputAttribs = outputAttribs;
        }

        private FragmentAttribute[] outputAttribs;
        private NativeLLGL.FragmentAttribute[] outputAttribsNative;
        public FragmentAttribute[] OutputAttribs
        {
            get
            {
                return outputAttribs;
            }
            set
            {
                if (value != null)
                {
                    outputAttribs = value;
                    outputAttribsNative = new NativeLLGL.FragmentAttribute[outputAttribs.Length];
                    for (int outputAttribsIndex = 0; outputAttribsIndex < outputAttribs.Length; ++outputAttribsIndex)
                    {
                        if (outputAttribs[outputAttribsIndex] != null)
                        {
                            outputAttribsNative[outputAttribsIndex] = outputAttribs[outputAttribsIndex].Native;
                        }
                    }
                }
                else
                {
                    outputAttribs = null;
                    outputAttribsNative = null;
                }
            }
        }

        public FragmentShaderAttributes() { }

        internal FragmentShaderAttributes(NativeLLGL.FragmentShaderAttributes native)
        {
            Native = native;
        }

        internal NativeLLGL.FragmentShaderAttributes Native
        {
            get
            {
                var native = new NativeLLGL.FragmentShaderAttributes();
                unsafe
                {
                    if (outputAttribs != null)
                    {
                        native.numOutputAttribs = (IntPtr)outputAttribs.Length;
                        fixed (NativeLLGL.FragmentAttribute* outputAttribsPtr = outputAttribsNative)
                        {
                            native.outputAttribs = outputAttribsPtr;
                        }
                    }
                }
                return native;
            }
            set
            {
                unsafe
                {
                    OutputAttribs = new FragmentAttribute[(int)value.numOutputAttribs];
                    for (int i = 0; i < OutputAttribs.Length; ++i)
                    {
                        OutputAttribs[i] = new FragmentAttribute(value.outputAttribs[i]);
                    }
                }
            }
        }
    }

    public class ShaderResourceReflection
    {
        public BindingDescriptor Binding { get; set; }            = new BindingDescriptor();
        public int               ConstantBufferSize { get; set; } = 0;
        public StorageBufferType StorageBufferType { get; set; }  = StorageBufferType.Undefined;

        public ShaderResourceReflection() { }

        internal ShaderResourceReflection(NativeLLGL.ShaderResourceReflection native)
        {
            Native = native;
        }

        internal NativeLLGL.ShaderResourceReflection Native
        {
            get
            {
                var native = new NativeLLGL.ShaderResourceReflection();
                if (Binding != null)
                {
                    native.binding = Binding.Native;
                }
                native.constantBufferSize = ConstantBufferSize;
                native.storageBufferType  = StorageBufferType;
                return native;
            }
            set
            {
                Binding.Native     = value.binding;
                ConstantBufferSize = value.constantBufferSize;
                StorageBufferType  = value.storageBufferType;
            }
        }
    }

    public class TextureViewDescriptor
    {
        public TextureType        Type { get; set; }        = TextureType.Texture2D;
        public Format             Format { get; set; }      = Format.RGBA8UNorm;
        public TextureSubresource Subresource { get; set; } = new TextureSubresource();
        public TextureSwizzleRGBA Swizzle { get; set; }     = new TextureSwizzleRGBA();

        internal NativeLLGL.TextureViewDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.TextureViewDescriptor();
                native.type        = Type;
                native.format      = Format;
                native.subresource = Subresource;
                native.swizzle     = Swizzle;
                return native;
            }
        }
    }

    public class PipelineLayoutDescriptor
    {
        public AnsiString                         DebugName { get; set; }               = null;
        private BindingDescriptor[] heapBindings;
        private NativeLLGL.BindingDescriptor[] heapBindingsNative;
        public BindingDescriptor[] HeapBindings
        {
            get
            {
                return heapBindings;
            }
            set
            {
                if (value != null)
                {
                    heapBindings = value;
                    heapBindingsNative = new NativeLLGL.BindingDescriptor[heapBindings.Length];
                    for (int heapBindingsIndex = 0; heapBindingsIndex < heapBindings.Length; ++heapBindingsIndex)
                    {
                        if (heapBindings[heapBindingsIndex] != null)
                        {
                            heapBindingsNative[heapBindingsIndex] = heapBindings[heapBindingsIndex].Native;
                        }
                    }
                }
                else
                {
                    heapBindings = null;
                    heapBindingsNative = null;
                }
            }
        }
        private BindingDescriptor[] bindings;
        private NativeLLGL.BindingDescriptor[] bindingsNative;
        public BindingDescriptor[] Bindings
        {
            get
            {
                return bindings;
            }
            set
            {
                if (value != null)
                {
                    bindings = value;
                    bindingsNative = new NativeLLGL.BindingDescriptor[bindings.Length];
                    for (int bindingsIndex = 0; bindingsIndex < bindings.Length; ++bindingsIndex)
                    {
                        if (bindings[bindingsIndex] != null)
                        {
                            bindingsNative[bindingsIndex] = bindings[bindingsIndex].Native;
                        }
                    }
                }
                else
                {
                    bindings = null;
                    bindingsNative = null;
                }
            }
        }
        private StaticSamplerDescriptor[] staticSamplers;
        private NativeLLGL.StaticSamplerDescriptor[] staticSamplersNative;
        public StaticSamplerDescriptor[] StaticSamplers
        {
            get
            {
                return staticSamplers;
            }
            set
            {
                if (value != null)
                {
                    staticSamplers = value;
                    staticSamplersNative = new NativeLLGL.StaticSamplerDescriptor[staticSamplers.Length];
                    for (int staticSamplersIndex = 0; staticSamplersIndex < staticSamplers.Length; ++staticSamplersIndex)
                    {
                        if (staticSamplers[staticSamplersIndex] != null)
                        {
                            staticSamplersNative[staticSamplersIndex] = staticSamplers[staticSamplersIndex].Native;
                        }
                    }
                }
                else
                {
                    staticSamplers = null;
                    staticSamplersNative = null;
                }
            }
        }
        private UniformDescriptor[] uniforms;
        private NativeLLGL.UniformDescriptor[] uniformsNative;
        public UniformDescriptor[] Uniforms
        {
            get
            {
                return uniforms;
            }
            set
            {
                if (value != null)
                {
                    uniforms = value;
                    uniformsNative = new NativeLLGL.UniformDescriptor[uniforms.Length];
                    for (int uniformsIndex = 0; uniformsIndex < uniforms.Length; ++uniformsIndex)
                    {
                        if (uniforms[uniformsIndex] != null)
                        {
                            uniformsNative[uniformsIndex] = uniforms[uniformsIndex].Native;
                        }
                    }
                }
                else
                {
                    uniforms = null;
                    uniformsNative = null;
                }
            }
        }
        private CombinedTextureSamplerDescriptor[] combinedTextureSamplers;
        private NativeLLGL.CombinedTextureSamplerDescriptor[] combinedTextureSamplersNative;
        public CombinedTextureSamplerDescriptor[] CombinedTextureSamplers
        {
            get
            {
                return combinedTextureSamplers;
            }
            set
            {
                if (value != null)
                {
                    combinedTextureSamplers = value;
                    combinedTextureSamplersNative = new NativeLLGL.CombinedTextureSamplerDescriptor[combinedTextureSamplers.Length];
                    for (int combinedTextureSamplersIndex = 0; combinedTextureSamplersIndex < combinedTextureSamplers.Length; ++combinedTextureSamplersIndex)
                    {
                        if (combinedTextureSamplers[combinedTextureSamplersIndex] != null)
                        {
                            combinedTextureSamplersNative[combinedTextureSamplersIndex] = combinedTextureSamplers[combinedTextureSamplersIndex].Native;
                        }
                    }
                }
                else
                {
                    combinedTextureSamplers = null;
                    combinedTextureSamplersNative = null;
                }
            }
        }
        public BarrierFlags                       BarrierFlags { get; set; }            = 0;

        internal NativeLLGL.PipelineLayoutDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.PipelineLayoutDescriptor();
                unsafe
                {
                    fixed (byte* debugNamePtr = DebugName.Ascii)
                    {
                        native.debugName = debugNamePtr;
                    }
                    if (heapBindings != null)
                    {
                        native.numHeapBindings = (IntPtr)heapBindings.Length;
                        fixed (NativeLLGL.BindingDescriptor* heapBindingsPtr = heapBindingsNative)
                        {
                            native.heapBindings = heapBindingsPtr;
                        }
                    }
                    if (bindings != null)
                    {
                        native.numBindings = (IntPtr)bindings.Length;
                        fixed (NativeLLGL.BindingDescriptor* bindingsPtr = bindingsNative)
                        {
                            native.bindings = bindingsPtr;
                        }
                    }
                    if (staticSamplers != null)
                    {
                        native.numStaticSamplers = (IntPtr)staticSamplers.Length;
                        fixed (NativeLLGL.StaticSamplerDescriptor* staticSamplersPtr = staticSamplersNative)
                        {
                            native.staticSamplers = staticSamplersPtr;
                        }
                    }
                    if (uniforms != null)
                    {
                        native.numUniforms = (IntPtr)uniforms.Length;
                        fixed (NativeLLGL.UniformDescriptor* uniformsPtr = uniformsNative)
                        {
                            native.uniforms = uniformsPtr;
                        }
                    }
                    if (combinedTextureSamplers != null)
                    {
                        native.numCombinedTextureSamplers = (IntPtr)combinedTextureSamplers.Length;
                        fixed (NativeLLGL.CombinedTextureSamplerDescriptor* combinedTextureSamplersPtr = combinedTextureSamplersNative)
                        {
                            native.combinedTextureSamplers = combinedTextureSamplersPtr;
                        }
                    }
                    native.barrierFlags            = (int)BarrierFlags;
                }
                return native;
            }
        }
    }

    public class GraphicsPipelineDescriptor
    {
        public AnsiString             DebugName { get; set; }            = null;
        public PipelineLayout         PipelineLayout { get; set; }       = null;
        public RenderPass             RenderPass { get; set; }           = null;
        public Shader                 VertexShader { get; set; }         = null;
        public Shader                 TessControlShader { get; set; }    = null;
        public Shader                 TessEvaluationShader { get; set; } = null;
        public Shader                 GeometryShader { get; set; }       = null;
        public Shader                 FragmentShader { get; set; }       = null;
        public Format                 IndexFormat { get; set; }          = Format.Undefined;
        public PrimitiveTopology      PrimitiveTopology { get; set; }    = PrimitiveTopology.TriangleList;
        public Viewport[]             Viewports { get; set; }
        public Scissor[]              Scissors { get; set; }
        public DepthDescriptor        Depth { get; set; }                = new DepthDescriptor();
        public StencilDescriptor      Stencil { get; set; }              = new StencilDescriptor();
        public RasterizerDescriptor   Rasterizer { get; set; }           = new RasterizerDescriptor();
        public BlendDescriptor        Blend { get; set; }                = new BlendDescriptor();
        public TessellationDescriptor Tessellation { get; set; }         = new TessellationDescriptor();

        internal NativeLLGL.GraphicsPipelineDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.GraphicsPipelineDescriptor();
                unsafe
                {
                    fixed (byte* debugNamePtr = DebugName.Ascii)
                    {
                        native.debugName = debugNamePtr;
                    }
                    if (PipelineLayout != null)
                    {
                        native.pipelineLayout = PipelineLayout.Native;
                    }
                    if (RenderPass != null)
                    {
                        native.renderPass = RenderPass.Native;
                    }
                    if (VertexShader != null)
                    {
                        native.vertexShader = VertexShader.Native;
                    }
                    if (TessControlShader != null)
                    {
                        native.tessControlShader = TessControlShader.Native;
                    }
                    if (TessEvaluationShader != null)
                    {
                        native.tessEvaluationShader = TessEvaluationShader.Native;
                    }
                    if (GeometryShader != null)
                    {
                        native.geometryShader = GeometryShader.Native;
                    }
                    if (FragmentShader != null)
                    {
                        native.fragmentShader = FragmentShader.Native;
                    }
                    native.indexFormat          = IndexFormat;
                    native.primitiveTopology    = PrimitiveTopology;
                    if (Viewports != null)
                    {
                        native.numViewports = (IntPtr)Viewports.Length;
                        fixed (Viewport* viewportsPtr = Viewports)
                        {
                            native.viewports = viewportsPtr;
                        }
                    }
                    if (Scissors != null)
                    {
                        native.numScissors = (IntPtr)Scissors.Length;
                        fixed (Scissor* scissorsPtr = Scissors)
                        {
                            native.scissors = scissorsPtr;
                        }
                    }
                    if (Depth != null)
                    {
                        native.depth = Depth.Native;
                    }
                    if (Stencil != null)
                    {
                        native.stencil = Stencil.Native;
                    }
                    if (Rasterizer != null)
                    {
                        native.rasterizer = Rasterizer.Native;
                    }
                    if (Blend != null)
                    {
                        native.blend = Blend.Native;
                    }
                    if (Tessellation != null)
                    {
                        native.tessellation = Tessellation.Native;
                    }
                }
                return native;
            }
        }
    }

    public class ResourceViewDescriptor
    {
        public Resource              Resource { get; set; }     = null;
        public TextureViewDescriptor TextureView { get; set; }  = new TextureViewDescriptor();
        public BufferViewDescriptor  BufferView { get; set; }   = new BufferViewDescriptor();
        public int                   InitialCount { get; set; } = 0;

        internal NativeLLGL.ResourceViewDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.ResourceViewDescriptor();
                if (Resource != null)
                {
                    native.resource = Resource.NativeBase;
                }
                if (TextureView != null)
                {
                    native.textureView = TextureView.Native;
                }
                if (BufferView != null)
                {
                    native.bufferView = BufferView.Native;
                }
                native.initialCount = InitialCount;
                return native;
            }
        }
    }

    public class ShaderReflection
    {
        private ShaderResourceReflection[] resources;
        private NativeLLGL.ShaderResourceReflection[] resourcesNative;
        public ShaderResourceReflection[] Resources
        {
            get
            {
                return resources;
            }
            set
            {
                if (value != null)
                {
                    resources = value;
                    resourcesNative = new NativeLLGL.ShaderResourceReflection[resources.Length];
                    for (int resourcesIndex = 0; resourcesIndex < resources.Length; ++resourcesIndex)
                    {
                        if (resources[resourcesIndex] != null)
                        {
                            resourcesNative[resourcesIndex] = resources[resourcesIndex].Native;
                        }
                    }
                }
                else
                {
                    resources = null;
                    resourcesNative = null;
                }
            }
        }
        private UniformDescriptor[] uniforms;
        private NativeLLGL.UniformDescriptor[] uniformsNative;
        public UniformDescriptor[] Uniforms
        {
            get
            {
                return uniforms;
            }
            set
            {
                if (value != null)
                {
                    uniforms = value;
                    uniformsNative = new NativeLLGL.UniformDescriptor[uniforms.Length];
                    for (int uniformsIndex = 0; uniformsIndex < uniforms.Length; ++uniformsIndex)
                    {
                        if (uniforms[uniformsIndex] != null)
                        {
                            uniformsNative[uniformsIndex] = uniforms[uniformsIndex].Native;
                        }
                    }
                }
                else
                {
                    uniforms = null;
                    uniformsNative = null;
                }
            }
        }
        public VertexShaderAttributes     Vertex { get; set; }    = new VertexShaderAttributes();
        public FragmentShaderAttributes   Fragment { get; set; }  = new FragmentShaderAttributes();
        public ComputeShaderAttributes    Compute { get; set; }   = new ComputeShaderAttributes();

        public ShaderReflection() { }

        internal ShaderReflection(NativeLLGL.ShaderReflection native)
        {
            Native = native;
        }

        internal NativeLLGL.ShaderReflection Native
        {
            set
            {
                unsafe
                {
                    Resources = new ShaderResourceReflection[(int)value.numResources];
                    for (int i = 0; i < Resources.Length; ++i)
                    {
                        Resources[i] = new ShaderResourceReflection(value.resources[i]);
                    }
                    Uniforms  = new UniformDescriptor[(int)value.numUniforms];
                    for (int i = 0; i < Uniforms.Length; ++i)
                    {
                        Uniforms[i] = new UniformDescriptor(value.uniforms[i]);
                    }
                    Vertex.Native= value.vertex;
                    Fragment.Native= value.fragment;
                    Compute.Native= value.compute;
                }
            }
        }
    }

    #region NativeLLGL - native interface to LLGL using P/Invoke

    internal static class NativeLLGL
    {
        #if DEBUG
        const string DllName = "LLGLD";
        #else
        const string DllName = "LLGL";
        #endif

        #pragma warning disable 0649 // Disable warning about unused fields

        /* ----- Handles ----- */

        public unsafe struct Buffer
        {
            internal unsafe void* ptr;
            public Buffer(Resource instance)
            {
                ptr = instance.ptr;
            }
            public Resource AsResource()
            {
                return new Resource(this);
            }
        }

        public unsafe struct BufferArray
        {
            internal unsafe void* ptr;
        }

        public unsafe struct Canvas
        {
            internal unsafe void* ptr;
            public Canvas(Surface instance)
            {
                ptr = instance.ptr;
            }
            public Surface AsSurface()
            {
                return new Surface(this);
            }
        }

        public unsafe struct CommandBuffer
        {
            internal unsafe void* ptr;
        }

        public unsafe struct CommandQueue
        {
            internal unsafe void* ptr;
        }

        public unsafe struct Display
        {
            internal unsafe void* ptr;
        }

        public unsafe struct Fence
        {
            internal unsafe void* ptr;
        }

        public unsafe struct Image
        {
            internal unsafe void* ptr;
        }

        public unsafe struct PipelineCache
        {
            internal unsafe void* ptr;
        }

        public unsafe struct PipelineLayout
        {
            internal unsafe void* ptr;
        }

        public unsafe struct PipelineState
        {
            internal unsafe void* ptr;
        }

        public unsafe struct QueryHeap
        {
            internal unsafe void* ptr;
        }

        public unsafe struct RenderPass
        {
            internal unsafe void* ptr;
        }

        public unsafe struct RenderTarget
        {
            internal unsafe void* ptr;
            public RenderTarget(SwapChain instance)
            {
                ptr = instance.ptr;
            }
            public SwapChain AsSwapChain()
            {
                return new SwapChain(this);
            }
        }

        public unsafe struct RenderSystemChild
        {
            internal unsafe void* ptr;
        }

        public unsafe struct RenderingDebugger
        {
            internal unsafe void* ptr;
        }

        public unsafe struct RenderingProfiler
        {
            internal unsafe void* ptr;
        }

        public unsafe struct Report
        {
            internal unsafe void* ptr;
        }

        public unsafe struct Resource
        {
            internal unsafe void* ptr;
            public Resource(Buffer instance)
            {
                ptr = instance.ptr;
            }
            public Buffer AsBuffer()
            {
                return new Buffer(this);
            }
            public Resource(Texture instance)
            {
                ptr = instance.ptr;
            }
            public Texture AsTexture()
            {
                return new Texture(this);
            }
            public Resource(Sampler instance)
            {
                ptr = instance.ptr;
            }
            public Sampler AsSampler()
            {
                return new Sampler(this);
            }
        }

        public unsafe struct ResourceHeap
        {
            internal unsafe void* ptr;
        }

        public unsafe struct Sampler
        {
            internal unsafe void* ptr;
            public Sampler(Resource instance)
            {
                ptr = instance.ptr;
            }
            public Resource AsResource()
            {
                return new Resource(this);
            }
        }

        public unsafe struct Shader
        {
            internal unsafe void* ptr;
        }

        public unsafe struct Surface
        {
            internal unsafe void* ptr;
            public Surface(Window instance)
            {
                ptr = instance.ptr;
            }
            public Window AsWindow()
            {
                return new Window(this);
            }
            public Surface(Canvas instance)
            {
                ptr = instance.ptr;
            }
            public Canvas AsCanvas()
            {
                return new Canvas(this);
            }
        }

        public unsafe struct SwapChain
        {
            internal unsafe void* ptr;
            public SwapChain(RenderTarget instance)
            {
                ptr = instance.ptr;
            }
            public RenderTarget AsRenderTarget()
            {
                return new RenderTarget(this);
            }
        }

        public unsafe struct Texture
        {
            internal unsafe void* ptr;
            public Texture(Resource instance)
            {
                ptr = instance.ptr;
            }
            public Resource AsResource()
            {
                return new Resource(this);
            }
        }

        public unsafe struct Window
        {
            internal unsafe void* ptr;
            public Window(Surface instance)
            {
                ptr = instance.ptr;
            }
            public Surface AsSurface()
            {
                return new Surface(this);
            }
        }

        /* ----- Native structures ----- */

        public unsafe struct CanvasDescriptor
        {
            public byte* title;
            public int   flags; /* = 0 */
        }

        public unsafe struct ClearValue
        {
            public fixed float color[4]; /* = { 0.0f, 0.0f, 0.0f, 0.0f } */
            public float       depth;    /* = 1.0f */
            public int         stencil;  /* = 0 */
        }

        public unsafe struct CommandBufferDescriptor
        {
            public byte*      debugName;          /* = null */
            public int        flags;              /* = 0 */
            public int        numNativeBuffers;   /* = 0 */
            public long       minStagingPoolSize; /* = (0xFFFF+1) */
            public RenderPass renderPass;         /* = null */
        }

        public unsafe struct DispatchIndirectArguments
        {
            public fixed int numThreadGroups[3];
        }

        public unsafe struct ColorCodes
        {
            public int textFlags;       /* = 0 */
            public int backgroundFlags; /* = 0 */
        }

        public unsafe struct DepthBiasDescriptor
        {
            public float constantFactor; /* = 0.0f */
            public float slopeFactor;    /* = 0.0f */
            public float clamp;          /* = 0.0f */
        }

        public unsafe struct ComputePipelineDescriptor
        {
            public byte*          debugName;      /* = null */
            public PipelineLayout pipelineLayout; /* = null */
            public Shader         computeShader;  /* = null */
        }

        public unsafe struct ProfileTimeRecord
        {
            public byte* annotation;    /* = "" */
            public long  cpuTicksStart; /* = 0 */
            public long  cpuTicksEnd;   /* = 0 */
            public long  elapsedTime;   /* = 0 */
        }

        public unsafe struct ProfileCommandQueueRecord
        {
            public int bufferWrites;             /* = 0 */
            public int bufferReads;              /* = 0 */
            public int bufferMappings;           /* = 0 */
            public int textureWrites;            /* = 0 */
            public int textureReads;             /* = 0 */
            public int commandBufferSubmittions; /* = 0 */
            public int fenceSubmissions;         /* = 0 */
        }

        public unsafe struct ProfileCommandBufferRecord
        {
            public int encodings;                /* = 0 */
            public int mipMapsGenerations;       /* = 0 */
            public int vertexBufferBindings;     /* = 0 */
            public int indexBufferBindings;      /* = 0 */
            public int constantBufferBindings;   /* = 0 */
            public int sampledBufferBindings;    /* = 0 */
            public int storageBufferBindings;    /* = 0 */
            public int sampledTextureBindings;   /* = 0 */
            public int storageTextureBindings;   /* = 0 */
            public int samplerBindings;          /* = 0 */
            public int resourceHeapBindings;     /* = 0 */
            public int graphicsPipelineBindings; /* = 0 */
            public int computePipelineBindings;  /* = 0 */
            public int attachmentClears;         /* = 0 */
            public int bufferUpdates;            /* = 0 */
            public int bufferCopies;             /* = 0 */
            public int bufferFills;              /* = 0 */
            public int textureCopies;            /* = 0 */
            public int renderPassSections;       /* = 0 */
            public int streamOutputSections;     /* = 0 */
            public int querySections;            /* = 0 */
            public int renderConditionSections;  /* = 0 */
            public int drawCommands;             /* = 0 */
            public int dispatchCommands;         /* = 0 */
        }

        public unsafe struct RendererInfo
        {
            public byte*  rendererName;
            public byte*  deviceName;
            public byte*  vendorName;
            public byte*  shadingLanguageName;
            public IntPtr numExtensionNames;
            public byte** extensionNames;
            public IntPtr numPipelineCacheID;
            public byte*  pipelineCacheID;
        }

        public unsafe struct RenderingFeatures
        {
            [MarshalAs(UnmanagedType.I1)]
            public bool hasRenderTargets;             /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool has3DTextures;                /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasCubeTextures;              /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasArrayTextures;             /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasCubeArrayTextures;         /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasMultiSampleTextures;       /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasMultiSampleArrayTextures;  /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasTextureViews;              /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasTextureViewSwizzle;        /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasTextureViewFormatSwizzle;  /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasBufferViews;               /* = false */
            [Obsolete("LLGL.RenderingFeatures.hasSamplers is deprecated since 0.04b; All backends must support sampler states either natively or emulated.")]
            [MarshalAs(UnmanagedType.I1)]
            public bool hasSamplers;
            [MarshalAs(UnmanagedType.I1)]
            public bool hasConstantBuffers;           /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasStorageBuffers;            /* = false */
            [Obsolete("LLGL.RenderingFeatures.hasUniforms is deprecated since 0.04b; All backends must support uniforms either natively or emulated.")]
            [MarshalAs(UnmanagedType.I1)]
            public bool hasUniforms;
            [MarshalAs(UnmanagedType.I1)]
            public bool hasGeometryShaders;           /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasTessellationShaders;       /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasTessellatorStage;          /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasComputeShaders;            /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasInstancing;                /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasOffsetInstancing;          /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasIndirectDrawing;           /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasViewportArrays;            /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasConservativeRasterization; /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasStreamOutputs;             /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasLogicOp;                   /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasPipelineCaching;           /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasPipelineStatistics;        /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool hasRenderCondition;           /* = false */
        }

        public unsafe struct RenderingLimits
        {
            public fixed float lineWidthRange[2];                /* = { 1.0f, 1.0f } */
            public int         maxTextureArrayLayers;            /* = 0 */
            public int         maxColorAttachments;              /* = 0 */
            public int         maxPatchVertices;                 /* = 0 */
            public int         max1DTextureSize;                 /* = 0 */
            public int         max2DTextureSize;                 /* = 0 */
            public int         max3DTextureSize;                 /* = 0 */
            public int         maxCubeTextureSize;               /* = 0 */
            public int         maxAnisotropy;                    /* = 0 */
            public fixed int   maxComputeShaderWorkGroups[3];    /* = { 0, 0, 0 } */
            public fixed int   maxComputeShaderWorkGroupSize[3]; /* = { 0, 0, 0 } */
            public int         maxViewports;                     /* = 0 */
            public fixed int   maxViewportSize[2];               /* = { 0, 0 } */
            public long        maxBufferSize;                    /* = 0 */
            public long        maxConstantBufferSize;            /* = 0 */
            public int         maxStreamOutputs;                 /* = 0 */
            public int         maxTessFactor;                    /* = 0 */
            public long        minConstantBufferAlignment;       /* = 0 */
            public long        minSampledBufferAlignment;        /* = 0 */
            public long        minStorageBufferAlignment;        /* = 0 */
            public int         maxColorBufferSamples;            /* = 0 */
            public int         maxDepthBufferSamples;            /* = 0 */
            public int         maxStencilBufferSamples;          /* = 0 */
            public int         maxNoAttachmentSamples;           /* = 0 */
        }

        public unsafe struct ResourceHeapDescriptor
        {
            public byte*          debugName;        /* = null */
            public PipelineLayout pipelineLayout;   /* = null */
            public int            numResourceViews; /* = 0 */
            [Obsolete("ResourceHeapDescriptor.barrierFlags is deprecated since 0.04b; Use PipelineLayoutDescriptor.barrierFlags instead!")]
            public int            barrierFlags;
        }

        public unsafe struct ShaderMacro
        {
            public byte* name;       /* = null */
            public byte* definition; /* = null */
        }

        public unsafe struct CanvasEventListener
        {
            public IntPtr onProcessEvents;
            public IntPtr onQuit;
            public IntPtr onInit;
            public IntPtr onDestroy;
            public IntPtr onDraw;
            public IntPtr onResize;
            public IntPtr onTapGesture;
            public IntPtr onPanGesture;
            public IntPtr onKeyDown;
            public IntPtr onKeyUp;
        }

        public unsafe struct WindowEventListener
        {
            public IntPtr onQuit;
            public IntPtr onKeyDown;
            public IntPtr onKeyUp;
            public IntPtr onDoubleClick;
            public IntPtr onChar;
            public IntPtr onWheelMotion;
            public IntPtr onLocalMotion;
            public IntPtr onGlobalMotion;
            public IntPtr onResize;
            public IntPtr onUpdate;
            public IntPtr onGetFocus;
            public IntPtr onLostFocus;
        }

        public unsafe struct BufferViewDescriptor
        {
            public Format format; /* = Format.Undefined */
            public long   offset; /* = 0 */
            public long   size;   /* = -1 */
        }

        public unsafe struct AttachmentClear
        {
            public int        flags;           /* = 0 */
            public int        colorAttachment; /* = 0 */
            public ClearValue clearValue;
        }

        public unsafe struct DisplayMode
        {
            public Extent2D resolution;
            public int      refreshRate; /* = 0 */
        }

        public unsafe struct FragmentAttribute
        {
            public byte*       name;
            public Format      format;      /* = Format.RGBA32Float */
            public int         location;    /* = 0 */
            public SystemValue systemValue; /* = SystemValue.Undefined */
        }

        public unsafe struct ImageView
        {
            public ImageFormat format;    /* = ImageFormat.RGBA */
            public DataType    dataType;  /* = DataType.UInt8 */
            public void*       data;      /* = null */
            public IntPtr      dataSize;  /* = 0 */
            public int         rowStride; /* = 0 */
        }

        public unsafe struct MutableImageView
        {
            public ImageFormat format;   /* = ImageFormat.RGBA */
            public DataType    dataType; /* = DataType.UInt8 */
            public void*       data;     /* = null */
            public IntPtr      dataSize; /* = 0 */
        }

        public unsafe struct BindingDescriptor
        {
            public byte*        name;
            public ResourceType type;       /* = ResourceType.Undefined */
            public int          bindFlags;  /* = 0 */
            public int          stageFlags; /* = 0 */
            public BindingSlot  slot;
            public int          arraySize;  /* = 0 */
        }

        public unsafe struct UniformDescriptor
        {
            public byte*       name;
            public UniformType type;      /* = UniformType.Undefined */
            public int         arraySize; /* = 0 */
        }

        public unsafe struct CombinedTextureSamplerDescriptor
        {
            public byte*       name;
            public byte*       textureName;
            public byte*       samplerName;
            public BindingSlot slot;
        }

        public unsafe struct DepthDescriptor
        {
            [MarshalAs(UnmanagedType.I1)]
            public bool      testEnabled;  /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool      writeEnabled; /* = false */
            public CompareOp compareOp;    /* = CompareOp.Less */
        }

        public unsafe struct StencilFaceDescriptor
        {
            public StencilOp stencilFailOp; /* = StencilOp.Keep */
            public StencilOp depthFailOp;   /* = StencilOp.Keep */
            public StencilOp depthPassOp;   /* = StencilOp.Keep */
            public CompareOp compareOp;     /* = CompareOp.Less */
            public int       readMask;      /* = -1 */
            public int       writeMask;     /* = -1 */
            public int       reference;     /* = 0 */
        }

        public unsafe struct RasterizerDescriptor
        {
            public PolygonMode         polygonMode;               /* = PolygonMode.Fill */
            public CullMode            cullMode;                  /* = CullMode.Disabled */
            public DepthBiasDescriptor depthBias;
            [MarshalAs(UnmanagedType.I1)]
            public bool                frontCCW;                  /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool                discardEnabled;            /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool                depthClampEnabled;         /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool                scissorTestEnabled;        /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool                multiSampleEnabled;        /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool                antiAliasedLineEnabled;    /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool                conservativeRasterization; /* = false */
            public float               lineWidth;                 /* = 1.0f */
        }

        public unsafe struct BlendTargetDescriptor
        {
            [MarshalAs(UnmanagedType.I1)]
            public bool            blendEnabled;    /* = false */
            public BlendOp         srcColor;        /* = BlendOp.SrcAlpha */
            public BlendOp         dstColor;        /* = BlendOp.InvSrcAlpha */
            public BlendArithmetic colorArithmetic; /* = BlendArithmetic.Add */
            public BlendOp         srcAlpha;        /* = BlendOp.SrcAlpha */
            public BlendOp         dstAlpha;        /* = BlendOp.InvSrcAlpha */
            public BlendArithmetic alphaArithmetic; /* = BlendArithmetic.Add */
            public byte            colorMask;       /* = ColorMaskFlags.All */
        }

        public unsafe struct TessellationDescriptor
        {
            public TessellationPartition partition;        /* = TessellationPartition.Undefined */
            public int                   maxTessFactor;    /* = 64 */
            [MarshalAs(UnmanagedType.I1)]
            public bool                  outputWindingCCW; /* = false */
        }

        public unsafe struct QueryHeapDescriptor
        {
            public byte*     debugName;       /* = null */
            public QueryType type;            /* = QueryType.SamplesPassed */
            public int       numQueries;      /* = 1 */
            [MarshalAs(UnmanagedType.I1)]
            public bool      renderCondition; /* = false */
        }

        public unsafe struct FrameProfile
        {
            public ProfileCommandQueueRecord  commandQueueRecord;
            public ProfileCommandBufferRecord commandBufferRecord;
            public IntPtr                     numTimeRecords;
            public ProfileTimeRecord*         timeRecords;
        }

        public unsafe struct AttachmentFormatDescriptor
        {
            public Format            format;  /* = Format.Undefined */
            public AttachmentLoadOp  loadOp;  /* = AttachmentLoadOp.Undefined */
            public AttachmentStoreOp storeOp; /* = AttachmentStoreOp.Undefined */
        }

        public unsafe struct RenderSystemDescriptor
        {
            public byte*             moduleName;
            public int               flags;              /* = 0 */
            public void*             profiler;           /* = null */
            public RenderingDebugger debugger;           /* = null */
            public void*             rendererConfig;     /* = null */
            public IntPtr            rendererConfigSize; /* = 0 */
            public void*             nativeHandle;       /* = null */
            public IntPtr            nativeHandleSize;   /* = 0 */
        }

        public unsafe struct RenderingCapabilities
        {
            public ScreenOrigin      screenOrigin;        /* = ScreenOrigin.UpperLeft */
            public ClippingRange     clippingRange;       /* = ClippingRange.ZeroToOne */
            public IntPtr            numShadingLanguages;
            public ShadingLanguage*  shadingLanguages;
            public IntPtr            numTextureFormats;
            public Format*           textureFormats;
            public RenderingFeatures features;
            public RenderingLimits   limits;
        }

        public unsafe struct AttachmentDescriptor
        {
            public Format  format;     /* = Format.Undefined */
            public Texture texture;    /* = null */
            public int     mipLevel;   /* = 0 */
            public int     arrayLayer; /* = 0 */
        }

        public unsafe struct SamplerDescriptor
        {
            public byte*              debugName;      /* = null */
            public SamplerAddressMode addressModeU;   /* = SamplerAddressMode.Repeat */
            public SamplerAddressMode addressModeV;   /* = SamplerAddressMode.Repeat */
            public SamplerAddressMode addressModeW;   /* = SamplerAddressMode.Repeat */
            public SamplerFilter      minFilter;      /* = SamplerFilter.Linear */
            public SamplerFilter      magFilter;      /* = SamplerFilter.Linear */
            public SamplerFilter      mipMapFilter;   /* = SamplerFilter.Linear */
            [MarshalAs(UnmanagedType.I1)]
            public bool               mipMapEnabled;  /* = true */
            public float              mipMapLODBias;  /* = 0.0f */
            public float              minLOD;         /* = 0.0f */
            public float              maxLOD;         /* = 1000.0f */
            public int                maxAnisotropy;  /* = 1 */
            [MarshalAs(UnmanagedType.I1)]
            public bool               compareEnabled; /* = false */
            public CompareOp          compareOp;      /* = CompareOp.Less */
            public fixed float        borderColor[4]; /* = { 0.0f, 0.0f, 0.0f, 0.0f } */
        }

        public unsafe struct ComputeShaderAttributes
        {
            public Extent3D workGroupSize; /* = new Extent3D() { Width =  1, Height =  1, Depth =  1  } */
        }

        public unsafe struct SwapChainDescriptor
        {
            public byte*    debugName;   /* = null */
            public Extent2D resolution;
            public int      colorBits;   /* = 32 */
            public int      depthBits;   /* = 24 */
            public int      stencilBits; /* = 8 */
            public int      samples;     /* = 1 */
            public int      swapBuffers; /* = 2 */
            [MarshalAs(UnmanagedType.I1)]
            public bool     fullscreen;  /* = false */
        }

        public unsafe struct TextureDescriptor
        {
            public byte*       debugName;      /* = null */
            public TextureType type;           /* = TextureType.Texture2D */
            public int         bindFlags;      /* = (BindFlags.Sampled | BindFlags.ColorAttachment) */
            public int         cpuAccessFlags; /* = (CPUAccessFlags.Read | CPUAccessFlags.Write) */
            public int         miscFlags;      /* = (MiscFlags.FixedSamples | MiscFlags.GenerateMips) */
            public Format      format;         /* = Format.RGBA8UNorm */
            public Extent3D    extent;         /* = new Extent3D() { Width =  1, Height =  1, Depth =  1  } */
            public int         arrayLayers;    /* = 1 */
            public int         mipLevels;      /* = 0 */
            public int         samples;        /* = 1 */
            public ClearValue  clearValue;
        }

        public unsafe struct VertexAttribute
        {
            public byte*       name;
            public Format      format;          /* = Format.RGBA32Float */
            public int         location;        /* = 0 */
            public int         semanticIndex;   /* = 0 */
            public SystemValue systemValue;     /* = SystemValue.Undefined */
            public int         slot;            /* = 0 */
            public int         offset;          /* = 0 */
            public int         stride;          /* = 0 */
            public int         instanceDivisor; /* = 0 */
        }

        public unsafe struct WindowDescriptor
        {
            public byte*    title;
            public Offset2D position;
            public Extent2D size;
            public int      flags;             /* = 0 */
            public void*    windowContext;     /* = null */
            public IntPtr   windowContextSize; /* = 0 */
        }

        public unsafe struct BufferDescriptor
        {
            public byte*            debugName;        /* = null */
            public long             size;             /* = 0 */
            public int              stride;           /* = 0 */
            public Format           format;           /* = Format.Undefined */
            public int              bindFlags;        /* = 0 */
            public int              cpuAccessFlags;   /* = 0 */
            public int              miscFlags;        /* = 0 */
            public IntPtr           numVertexAttribs;
            public VertexAttribute* vertexAttribs;
        }

        public unsafe struct StaticSamplerDescriptor
        {
            public byte*             name;
            public int               stageFlags; /* = 0 */
            public BindingSlot       slot;
            public SamplerDescriptor sampler;
        }

        public unsafe struct StencilDescriptor
        {
            [MarshalAs(UnmanagedType.I1)]
            public bool                  testEnabled;      /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool                  referenceDynamic; /* = false */
            public StencilFaceDescriptor front;
            public StencilFaceDescriptor back;
        }

        public unsafe struct BlendDescriptor
        {
            [MarshalAs(UnmanagedType.I1)]
            public bool                  alphaToCoverageEnabled;  /* = false */
            [MarshalAs(UnmanagedType.I1)]
            public bool                  independentBlendEnabled; /* = false */
            public int                   sampleMask;              /* = -1 */
            public LogicOp               logicOp;                 /* = LogicOp.Disabled */
            public fixed float           blendFactor[4];          /* = { 0.0f, 0.0f, 0.0f, 0.0f } */
            [MarshalAs(UnmanagedType.I1)]
            public bool                  blendFactorDynamic;      /* = false */
            public BlendTargetDescriptor targets0;
            public BlendTargetDescriptor targets1;
            public BlendTargetDescriptor targets2;
            public BlendTargetDescriptor targets3;
            public BlendTargetDescriptor targets4;
            public BlendTargetDescriptor targets5;
            public BlendTargetDescriptor targets6;
            public BlendTargetDescriptor targets7;
        }

        public unsafe struct RenderPassDescriptor
        {
            public byte*                      debugName;         /* = null */
            public AttachmentFormatDescriptor colorAttachments0;
            public AttachmentFormatDescriptor colorAttachments1;
            public AttachmentFormatDescriptor colorAttachments2;
            public AttachmentFormatDescriptor colorAttachments3;
            public AttachmentFormatDescriptor colorAttachments4;
            public AttachmentFormatDescriptor colorAttachments5;
            public AttachmentFormatDescriptor colorAttachments6;
            public AttachmentFormatDescriptor colorAttachments7;
            public AttachmentFormatDescriptor depthAttachment;
            public AttachmentFormatDescriptor stencilAttachment;
            public int                        samples;           /* = 1 */
        }

        public unsafe struct RenderTargetDescriptor
        {
            public byte*                debugName;              /* = null */
            public RenderPass           renderPass;             /* = null */
            public Extent2D             resolution;
            public int                  samples;                /* = 1 */
            public AttachmentDescriptor colorAttachments0;
            public AttachmentDescriptor colorAttachments1;
            public AttachmentDescriptor colorAttachments2;
            public AttachmentDescriptor colorAttachments3;
            public AttachmentDescriptor colorAttachments4;
            public AttachmentDescriptor colorAttachments5;
            public AttachmentDescriptor colorAttachments6;
            public AttachmentDescriptor colorAttachments7;
            public AttachmentDescriptor resolveAttachments0;
            public AttachmentDescriptor resolveAttachments1;
            public AttachmentDescriptor resolveAttachments2;
            public AttachmentDescriptor resolveAttachments3;
            public AttachmentDescriptor resolveAttachments4;
            public AttachmentDescriptor resolveAttachments5;
            public AttachmentDescriptor resolveAttachments6;
            public AttachmentDescriptor resolveAttachments7;
            public AttachmentDescriptor depthStencilAttachment;
        }

        public unsafe struct VertexShaderAttributes
        {
            public IntPtr           numInputAttribs;
            public VertexAttribute* inputAttribs;
            public IntPtr           numOutputAttribs;
            public VertexAttribute* outputAttribs;
        }

        public unsafe struct FragmentShaderAttributes
        {
            public IntPtr             numOutputAttribs;
            public FragmentAttribute* outputAttribs;
        }

        public unsafe struct ShaderResourceReflection
        {
            public BindingDescriptor binding;
            public int               constantBufferSize; /* = 0 */
            public StorageBufferType storageBufferType;  /* = StorageBufferType.Undefined */
        }

        public unsafe struct TextureViewDescriptor
        {
            public TextureType        type;        /* = TextureType.Texture2D */
            public Format             format;      /* = Format.RGBA8UNorm */
            public TextureSubresource subresource;
            public TextureSwizzleRGBA swizzle;
        }

        public unsafe struct PipelineLayoutDescriptor
        {
            public byte*                             debugName;                  /* = null */
            public IntPtr                            numHeapBindings;
            public BindingDescriptor*                heapBindings;
            public IntPtr                            numBindings;
            public BindingDescriptor*                bindings;
            public IntPtr                            numStaticSamplers;
            public StaticSamplerDescriptor*          staticSamplers;
            public IntPtr                            numUniforms;
            public UniformDescriptor*                uniforms;
            public IntPtr                            numCombinedTextureSamplers;
            public CombinedTextureSamplerDescriptor* combinedTextureSamplers;
            public int                               barrierFlags;               /* = 0 */
        }

        public unsafe struct GraphicsPipelineDescriptor
        {
            public byte*                  debugName;            /* = null */
            public PipelineLayout         pipelineLayout;       /* = null */
            public RenderPass             renderPass;           /* = null */
            public Shader                 vertexShader;         /* = null */
            public Shader                 tessControlShader;    /* = null */
            public Shader                 tessEvaluationShader; /* = null */
            public Shader                 geometryShader;       /* = null */
            public Shader                 fragmentShader;       /* = null */
            public Format                 indexFormat;          /* = Format.Undefined */
            public PrimitiveTopology      primitiveTopology;    /* = PrimitiveTopology.TriangleList */
            public IntPtr                 numViewports;
            public Viewport*              viewports;
            public IntPtr                 numScissors;
            public Scissor*               scissors;
            public DepthDescriptor        depth;
            public StencilDescriptor      stencil;
            public RasterizerDescriptor   rasterizer;
            public BlendDescriptor        blend;
            public TessellationDescriptor tessellation;
        }

        public unsafe struct ResourceViewDescriptor
        {
            public Resource              resource;     /* = null */
            public TextureViewDescriptor textureView;
            public BufferViewDescriptor  bufferView;
            public int                   initialCount; /* = 0 */
        }

        public unsafe struct ShaderDescriptor
        {
            public byte*                    debugName;  /* = null */
            public ShaderType               type;       /* = ShaderType.Undefined */
            public byte*                    source;     /* = null */
            public IntPtr                   sourceSize; /* = 0 */
            public ShaderSourceType         sourceType; /* = ShaderSourceType.CodeFile */
            public byte*                    entryPoint; /* = null */
            public byte*                    profile;    /* = null */
            public ShaderMacro*             defines;    /* = null */
            public int                      flags;      /* = 0 */
            [Obsolete("ShaderDescriptor.name is deprecated since 0.04b; Use ShaderDescriptor.debugName instead!")]
            public byte*                    name;
            public VertexShaderAttributes   vertex;
            public FragmentShaderAttributes fragment;
            public ComputeShaderAttributes  compute;
        }

        public unsafe struct ShaderReflection
        {
            public IntPtr                    numResources;
            public ShaderResourceReflection* resources;
            public IntPtr                    numUniforms;
            public UniformDescriptor*        uniforms;
            public VertexShaderAttributes    vertex;
            public FragmentShaderAttributes  fragment;
            public ComputeShaderAttributes   compute;
        }

        /* ----- Native delegates ----- */

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnCanvasProcessEventsDelegate(Canvas sender);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnCanvasQuitDelegate(Canvas sender, bool* veto);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnCanvasInitDelegate(Canvas sender);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnCanvasDestroyDelegate(Canvas sender);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnCanvasDrawDelegate(Canvas sender);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnCanvasResizeDelegate(Canvas sender, ref Extent2D clientAreaSize);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnCanvasTapGestureDelegate(Canvas sender, ref Offset2D position, int numTouches);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnCanvasPanGestureDelegate(Canvas sender, ref Offset2D position, int numTouches, float dx, float dy, EventAction action);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnCanvasKeyDownDelegate(Canvas sender, Key keyCode);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnCanvasKeyUpDelegate(Canvas sender, Key keyCode);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void ReportCallbackDelegate(ReportType type, [MarshalAs(UnmanagedType.LPStr)] string text, void* userData);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void ReportCallbackExtDelegate(ReportType type, [MarshalAs(UnmanagedType.LPStr)] string text, void* userData, ref ColorCodes colors);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnWindowQuitDelegate(Window sender, bool* veto);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnWindowKeyDownDelegate(Window sender, Key keyCode);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnWindowKeyUpDelegate(Window sender, Key keyCode);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnWindowDoubleClickDelegate(Window sender, Key keyCode);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnWindowCharDelegate(Window sender, char chr);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnWindowWheelMotionDelegate(Window sender, int motion);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnWindowLocalMotionDelegate(Window sender, ref Offset2D position);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnWindowGlobalMotionDelegate(Window sender, ref Offset2D motion);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnWindowResizeDelegate(Window sender, ref Extent2D clientAreaSize);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnWindowUpdateDelegate(Window sender);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnWindowGetFocusDelegate(Window sender);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void OnWindowLostFocusDelegate(Window sender);


        /* ----- Native functions ----- */

        [DllImport(DllName, EntryPoint="llglGetBufferBindFlags", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe int GetBufferBindFlags(Buffer buffer);

        [DllImport(DllName, EntryPoint="llglGetBufferDesc", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void GetBufferDesc(Buffer buffer, ref BufferDescriptor outDesc);

        [DllImport(DllName, EntryPoint="llglGetBufferArrayBindFlags", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe int GetBufferArrayBindFlags(BufferArray bufferArray);

        [DllImport(DllName, EntryPoint="llglCreateCanvas", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe Canvas CreateCanvas(ref CanvasDescriptor canvasDesc);

        [DllImport(DllName, EntryPoint="llglReleaseCanvas", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ReleaseCanvas(Canvas canvas);

        [DllImport(DllName, EntryPoint="llglSetCanvasTitle", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetCanvasTitle(Canvas canvas, [MarshalAs(UnmanagedType.LPWStr)] string title);

        [DllImport(DllName, EntryPoint="llglSetCanvasTitleUTF8", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetCanvasTitleUTF8(Canvas canvas, [MarshalAs(UnmanagedType.LPStr)] string title);

        [DllImport(DllName, EntryPoint="llglGetCanvasTitle", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe IntPtr GetCanvasTitle(Canvas canvas, IntPtr outTitleLength, char* outTitle);

        [DllImport(DllName, EntryPoint="llglGetCanvasTitleUTF8", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe IntPtr GetCanvasTitleUTF8(Canvas canvas, IntPtr outTitleLength, byte* outTitle);

        [DllImport(DllName, EntryPoint="llglHasCanvasQuit", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool HasCanvasQuit(Canvas canvas);

        [DllImport(DllName, EntryPoint="llglSetCanvasUserData", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetCanvasUserData(Canvas canvas, void* userData);

        [DllImport(DllName, EntryPoint="llglGetCanvasUserData", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void* GetCanvasUserData(Canvas canvas);

        [DllImport(DllName, EntryPoint="llglAddCanvasEventListener", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe int AddCanvasEventListener(Canvas canvas, ref CanvasEventListener eventListener);

        [DllImport(DllName, EntryPoint="llglRemoveCanvasEventListener", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void RemoveCanvasEventListener(Canvas canvas, int eventListenerID);

        [DllImport(DllName, EntryPoint="llglPostCanvasQuit", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PostCanvasQuit(Canvas canvas);

        [DllImport(DllName, EntryPoint="llglPostCanvasInit", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PostCanvasInit(Canvas sender);

        [DllImport(DllName, EntryPoint="llglPostCanvasDestroy", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PostCanvasDestroy(Canvas sender);

        [DllImport(DllName, EntryPoint="llglPostCanvasDraw", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PostCanvasDraw(Canvas sender);

        [DllImport(DllName, EntryPoint="llglPostCanvasResize", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PostCanvasResize(Canvas sender, ref Extent2D clientAreaSize);

        [DllImport(DllName, EntryPoint="llglPostCanvasTapGesture", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PostCanvasTapGesture(Canvas sender, ref Offset2D position, int numTouches);

        [DllImport(DllName, EntryPoint="llglPostCanvasPanGesture", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PostCanvasPanGesture(Canvas sender, ref Offset2D position, int numTouches, float dx, float dy, EventAction action);

        [DllImport(DllName, EntryPoint="llglPostCanvasKeyDown", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PostCanvasKeyDown(Canvas sender, Key keyCode);

        [DllImport(DllName, EntryPoint="llglPostCanvasKeyUp", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PostCanvasKeyUp(Canvas sender, Key keyCode);

        [DllImport(DllName, EntryPoint="llglBegin", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void Begin(CommandBuffer commandBuffer);

        [DllImport(DllName, EntryPoint="llglEnd", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void End();

        [DllImport(DllName, EntryPoint="llglExecute", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void Execute(CommandBuffer secondaryCommandBuffer);

        [DllImport(DllName, EntryPoint="llglUpdateBuffer", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void UpdateBuffer(Buffer dstBuffer, long dstOffset, void* data, short dataSize);

        [DllImport(DllName, EntryPoint="llglCopyBuffer", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void CopyBuffer(Buffer dstBuffer, long dstOffset, Buffer srcBuffer, long srcOffset, long size);

        [DllImport(DllName, EntryPoint="llglCopyBufferFromTexture", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void CopyBufferFromTexture(Buffer dstBuffer, long dstOffset, Texture srcTexture, ref TextureRegion srcRegion, int rowStride, int layerStride);

        [DllImport(DllName, EntryPoint="llglFillBuffer", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void FillBuffer(Buffer dstBuffer, long dstOffset, int value, long fillSize);

        [DllImport(DllName, EntryPoint="llglCopyTexture", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void CopyTexture(Texture dstTexture, ref TextureLocation dstLocation, Texture srcTexture, ref TextureLocation srcLocation, ref Extent3D extent);

        [DllImport(DllName, EntryPoint="llglCopyTextureFromBuffer", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void CopyTextureFromBuffer(Texture dstTexture, ref TextureRegion dstRegion, Buffer srcBuffer, long srcOffset, int rowStride, int layerStride);

        [DllImport(DllName, EntryPoint="llglCopyTextureFromFramebuffer", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void CopyTextureFromFramebuffer(Texture dstTexture, ref TextureRegion dstRegion, ref Offset2D srcOffset);

        [DllImport(DllName, EntryPoint="llglGenerateMips", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void GenerateMips(Texture texture);

        [DllImport(DllName, EntryPoint="llglGenerateMipsRange", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void GenerateMipsRange(Texture texture, ref TextureSubresource subresource);

        [DllImport(DllName, EntryPoint="llglSetViewport", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetViewport(ref Viewport viewport);

        [DllImport(DllName, EntryPoint="llglSetViewports", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetViewports(int numViewports, Viewport* viewports);

        [DllImport(DllName, EntryPoint="llglSetScissor", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetScissor(ref Scissor scissor);

        [DllImport(DllName, EntryPoint="llglSetScissors", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetScissors(int numScissors, Scissor* scissors);

        [DllImport(DllName, EntryPoint="llglSetVertexBuffer", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetVertexBuffer(Buffer buffer);

        [DllImport(DllName, EntryPoint="llglSetVertexBufferArray", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetVertexBufferArray(BufferArray bufferArray);

        [DllImport(DllName, EntryPoint="llglSetIndexBuffer", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetIndexBuffer(Buffer buffer);

        [DllImport(DllName, EntryPoint="llglSetIndexBufferExt", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetIndexBufferExt(Buffer buffer, Format format, long offset);

        [DllImport(DllName, EntryPoint="llglSetResourceHeap", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetResourceHeap(ResourceHeap resourceHeap, int descriptorSet);

        [DllImport(DllName, EntryPoint="llglSetResource", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetResource(int descriptor, Resource resource);

        [DllImport(DllName, EntryPoint="llglResourceBarrier", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ResourceBarrier(int numBuffers, Buffer* buffers, int numTextures, Texture* textures);

        [DllImport(DllName, EntryPoint="llglResetResourceSlots", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ResetResourceSlots(ResourceType resourceType, int firstSlot, int numSlots, int bindFlags, int stageFlags);

        [DllImport(DllName, EntryPoint="llglBeginRenderPass", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void BeginRenderPass(RenderTarget renderTarget);

        [DllImport(DllName, EntryPoint="llglBeginRenderPassWithClear", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void BeginRenderPassWithClear(RenderTarget renderTarget, RenderPass renderPass, int numClearValues, ClearValue* clearValues, int swapBufferIndex);

        [DllImport(DllName, EntryPoint="llglEndRenderPass", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void EndRenderPass();

        [DllImport(DllName, EntryPoint="llglClear", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void Clear(int flags, ref ClearValue clearValue);

        [DllImport(DllName, EntryPoint="llglClearAttachments", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ClearAttachments(int numAttachments, AttachmentClear* attachments);

        [DllImport(DllName, EntryPoint="llglSetPipelineState", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetPipelineState(PipelineState pipelineState);

        [DllImport(DllName, EntryPoint="llglSetBlendFactor", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetBlendFactor(float* color);

        [DllImport(DllName, EntryPoint="llglSetStencilReference", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetStencilReference(int reference, StencilFace stencilFace);

        [DllImport(DllName, EntryPoint="llglSetUniforms", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetUniforms(int first, void* data, short dataSize);

        [DllImport(DllName, EntryPoint="llglBeginQuery", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void BeginQuery(QueryHeap queryHeap, int query);

        [DllImport(DllName, EntryPoint="llglEndQuery", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void EndQuery(QueryHeap queryHeap, int query);

        [DllImport(DllName, EntryPoint="llglBeginRenderCondition", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void BeginRenderCondition(QueryHeap queryHeap, int query, RenderConditionMode mode);

        [DllImport(DllName, EntryPoint="llglEndRenderCondition", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void EndRenderCondition();

        [DllImport(DllName, EntryPoint="llglBeginStreamOutput", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void BeginStreamOutput(int numBuffers, Buffer* buffers);

        [DllImport(DllName, EntryPoint="llglEndStreamOutput", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void EndStreamOutput();

        [DllImport(DllName, EntryPoint="llglDraw", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void Draw(int numVertices, int firstVertex);

        [DllImport(DllName, EntryPoint="llglDrawIndexed", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void DrawIndexed(int numIndices, int firstIndex);

        [DllImport(DllName, EntryPoint="llglDrawIndexedExt", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void DrawIndexedExt(int numIndices, int firstIndex, int vertexOffset);

        [DllImport(DllName, EntryPoint="llglDrawInstanced", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void DrawInstanced(int numVertices, int firstVertex, int numInstances);

        [DllImport(DllName, EntryPoint="llglDrawInstancedExt", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void DrawInstancedExt(int numVertices, int firstVertex, int numInstances, int firstInstance);

        [DllImport(DllName, EntryPoint="llglDrawIndexedInstanced", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void DrawIndexedInstanced(int numIndices, int numInstances, int firstIndex);

        [DllImport(DllName, EntryPoint="llglDrawIndexedInstancedExt", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void DrawIndexedInstancedExt(int numIndices, int numInstances, int firstIndex, int vertexOffset, int firstInstance);

        [DllImport(DllName, EntryPoint="llglDrawIndirect", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void DrawIndirect(Buffer buffer, long offset);

        [DllImport(DllName, EntryPoint="llglDrawIndirectExt", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void DrawIndirectExt(Buffer buffer, long offset, int numCommands, int stride);

        [DllImport(DllName, EntryPoint="llglDrawIndexedIndirect", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void DrawIndexedIndirect(Buffer buffer, long offset);

        [DllImport(DllName, EntryPoint="llglDrawIndexedIndirectExt", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void DrawIndexedIndirectExt(Buffer buffer, long offset, int numCommands, int stride);

        [DllImport(DllName, EntryPoint="llglDrawStreamOutput", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void DrawStreamOutput();

        [DllImport(DllName, EntryPoint="llglDispatch", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void Dispatch(int numWorkGroupsX, int numWorkGroupsY, int numWorkGroupsZ);

        [DllImport(DllName, EntryPoint="llglDispatchIndirect", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void DispatchIndirect(Buffer buffer, long offset);

        [DllImport(DllName, EntryPoint="llglPushDebugGroup", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PushDebugGroup([MarshalAs(UnmanagedType.LPStr)] string name);

        [DllImport(DllName, EntryPoint="llglPopDebugGroup", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PopDebugGroup();

        [DllImport(DllName, EntryPoint="llglDoNativeCommand", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void DoNativeCommand(void* nativeCommand, IntPtr nativeCommandSize);

        [DllImport(DllName, EntryPoint="llglGetNativeHandle", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool GetNativeHandle(void* nativeHandle, IntPtr nativeHandleSize);

        [DllImport(DllName, EntryPoint="llglSubmitCommandBuffer", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SubmitCommandBuffer(CommandBuffer commandBuffer);

        [DllImport(DllName, EntryPoint="llglQueryResult", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool QueryResult(QueryHeap queryHeap, int firstQuery, int numQueries, void* data, IntPtr dataSize);

        [DllImport(DllName, EntryPoint="llglSubmitFence", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SubmitFence(Fence fence);

        [DllImport(DllName, EntryPoint="llglWaitFence", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool WaitFence(Fence fence, long timeout);

        [DllImport(DllName, EntryPoint="llglWaitIdle", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void WaitIdle();

        [DllImport(DllName, EntryPoint="llglDisplayCount", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe IntPtr DisplayCount();

        [DllImport(DllName, EntryPoint="llglGetDisplayList", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe Display* GetDisplayList();

        [DllImport(DllName, EntryPoint="llglGetDisplay", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe Display GetDisplay(IntPtr index);

        [DllImport(DllName, EntryPoint="llglGetPrimaryDisplay", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe Display GetPrimaryDisplay();

        [DllImport(DllName, EntryPoint="llglShowCursor", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool ShowCursor([MarshalAs(UnmanagedType.I1)] bool show);

        [DllImport(DllName, EntryPoint="llglIsCursorShown", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool IsCursorShown();

        [DllImport(DllName, EntryPoint="llglSetCursorPosition", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool SetCursorPosition(ref Offset2D position);

        [DllImport(DllName, EntryPoint="llglGetCursorPosition", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void GetCursorPosition(ref Offset2D outPosition);

        [DllImport(DllName, EntryPoint="llglIsDisplayPrimary", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool IsDisplayPrimary(Display display);

        [DllImport(DllName, EntryPoint="llglGetDisplayDeviceName", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe IntPtr GetDisplayDeviceName(Display display, IntPtr outNameLength, char* outName);

        [DllImport(DllName, EntryPoint="llglGetDisplayOffset", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void GetDisplayOffset(Display display, ref Offset2D outOffset);

        [DllImport(DllName, EntryPoint="llglResetDisplayMode", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool ResetDisplayMode(Display display);

        [DllImport(DllName, EntryPoint="llglSetDisplayMode", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool SetDisplayMode(Display display, ref DisplayMode displayMode);

        [DllImport(DllName, EntryPoint="llglGetDisplayMode", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void GetDisplayMode(Display display, ref DisplayMode outDisplayMode);

        [DllImport(DllName, EntryPoint="llglGetSupportedDisplayModes", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe IntPtr GetSupportedDisplayModes(Display display, IntPtr maxNumDisplayModes, DisplayMode* outDisplayModes);

        [DllImport(DllName, EntryPoint="llglRegisterLogCallback", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe IntPtr RegisterLogCallback(IntPtr callback, void* userData);

        [DllImport(DllName, EntryPoint="llglRegisterLogCallbackExt", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe IntPtr RegisterLogCallbackExt(IntPtr callback, void* userData);

        [DllImport(DllName, EntryPoint="llglRegisterLogCallbackReport", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe IntPtr RegisterLogCallbackReport(Report report);

        [DllImport(DllName, EntryPoint="llglRegisterLogCallbackStd", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe IntPtr RegisterLogCallbackStd(int stdOutFlags);

        [DllImport(DllName, EntryPoint="llglUnregisterLogCallback", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void UnregisterLogCallback(IntPtr handle);

        [DllImport(DllName, EntryPoint="llglGetPipelineCacheBlob", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe IntPtr GetPipelineCacheBlob(PipelineCache pipelineCache, void* data, IntPtr size);

        [DllImport(DllName, EntryPoint="llglGetPipelineLayoutNumHeapBindings", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe int GetPipelineLayoutNumHeapBindings(PipelineLayout pipelineLayout);

        [DllImport(DllName, EntryPoint="llglGetPipelineLayoutNumBindings", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe int GetPipelineLayoutNumBindings(PipelineLayout pipelineLayout);

        [DllImport(DllName, EntryPoint="llglGetPipelineLayoutNumStaticSamplers", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe int GetPipelineLayoutNumStaticSamplers(PipelineLayout pipelineLayout);

        [DllImport(DllName, EntryPoint="llglGetPipelineLayoutNumUniforms", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe int GetPipelineLayoutNumUniforms(PipelineLayout pipelineLayout);

        [DllImport(DllName, EntryPoint="llglGetPipelineStateReport", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe Report GetPipelineStateReport(PipelineState pipelineState);

        [DllImport(DllName, EntryPoint="llglGetQueryHeapType", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe QueryType GetQueryHeapType(QueryHeap queryHeap);

        [DllImport(DllName, EntryPoint="llglAllocRenderingDebugger", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe RenderingDebugger AllocRenderingDebugger();

        [DllImport(DllName, EntryPoint="llglFreeRenderingDebugger", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void FreeRenderingDebugger(RenderingDebugger debugger);

        [DllImport(DllName, EntryPoint="llglSetDebuggerTimeRecording", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetDebuggerTimeRecording(RenderingDebugger debugger, [MarshalAs(UnmanagedType.I1)] bool enabled);

        [DllImport(DllName, EntryPoint="llglGetDebuggerTimeRecording", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool GetDebuggerTimeRecording(RenderingDebugger debugger);

        [DllImport(DllName, EntryPoint="llglFlushDebuggerProfile", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void FlushDebuggerProfile(RenderingDebugger debugger, ref FrameProfile outFrameProfile);

        [DllImport(DllName, EntryPoint="llglLoadRenderSystem", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe int LoadRenderSystem([MarshalAs(UnmanagedType.LPStr)] string moduleName);

        [DllImport(DllName, EntryPoint="llglLoadRenderSystemExt", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe int LoadRenderSystemExt(ref RenderSystemDescriptor renderSystemDesc, Report report);

        [DllImport(DllName, EntryPoint="llglUnloadRenderSystem", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void UnloadRenderSystem();

        [DllImport(DllName, EntryPoint="llglMakeRenderSystemCurrent", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void MakeRenderSystemCurrent(int id);

        [DllImport(DllName, EntryPoint="llglGetRendererID", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe int GetRendererID();

        [DllImport(DllName, EntryPoint="llglGetRendererName", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        public static extern unsafe string GetRendererName();

        [DllImport(DllName, EntryPoint="llglGetRendererInfo", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void GetRendererInfo(ref RendererInfo outInfo);

        [DllImport(DllName, EntryPoint="llglGetRenderingCaps", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void GetRenderingCaps(ref RenderingCapabilities outCaps);

        [DllImport(DllName, EntryPoint="llglGetRendererReport", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe Report GetRendererReport();

        [DllImport(DllName, EntryPoint="llglCreateSwapChain", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe SwapChain CreateSwapChain(ref SwapChainDescriptor swapChainDesc);

        [DllImport(DllName, EntryPoint="llglCreateSwapChainExt", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe SwapChain CreateSwapChainExt(ref SwapChainDescriptor swapChainDesc, Surface surface);

        [DllImport(DllName, EntryPoint="llglReleaseSwapChain", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ReleaseSwapChain(SwapChain swapChain);

        [DllImport(DllName, EntryPoint="llglCreateCommandBuffer", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe CommandBuffer CreateCommandBuffer(ref CommandBufferDescriptor commandBufferDesc);

        [DllImport(DllName, EntryPoint="llglReleaseCommandBuffer", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ReleaseCommandBuffer(CommandBuffer commandBuffer);

        [DllImport(DllName, EntryPoint="llglCreateBuffer", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe Buffer CreateBuffer(ref BufferDescriptor bufferDesc, void* initialData);

        [DllImport(DllName, EntryPoint="llglReleaseBuffer", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ReleaseBuffer(Buffer buffer);

        [DllImport(DllName, EntryPoint="llglWriteBuffer", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void WriteBuffer(Buffer buffer, long offset, void* data, long dataSize);

        [DllImport(DllName, EntryPoint="llglReadBuffer", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ReadBuffer(Buffer buffer, long offset, void* data, long dataSize);

        [DllImport(DllName, EntryPoint="llglMapBuffer", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void* MapBuffer(Buffer buffer, CPUAccess access);

        [DllImport(DllName, EntryPoint="llglMapBufferRange", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void* MapBufferRange(Buffer buffer, CPUAccess access, long offset, long length);

        [DllImport(DllName, EntryPoint="llglUnmapBuffer", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void UnmapBuffer(Buffer buffer);

        [DllImport(DllName, EntryPoint="llglCreateBufferArray", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe BufferArray CreateBufferArray(int numBuffers, Buffer* buffers);

        [DllImport(DllName, EntryPoint="llglReleaseBufferArray", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ReleaseBufferArray(BufferArray bufferArray);

        [DllImport(DllName, EntryPoint="llglCreateTexture", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe Texture CreateTexture(ref TextureDescriptor textureDesc, ImageView* initialImage);

        [DllImport(DllName, EntryPoint="llglReleaseTexture", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ReleaseTexture(Texture texture);

        [DllImport(DllName, EntryPoint="llglWriteTexture", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void WriteTexture(Texture texture, ref TextureRegion textureRegion, ref ImageView srcImageView);

        [DllImport(DllName, EntryPoint="llglReadTexture", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ReadTexture(Texture texture, ref TextureRegion textureRegion, ref MutableImageView dstImageView);

        [DllImport(DllName, EntryPoint="llglCreateSampler", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe Sampler CreateSampler(ref SamplerDescriptor samplerDesc);

        [DllImport(DllName, EntryPoint="llglReleaseSampler", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ReleaseSampler(Sampler sampler);

        [DllImport(DllName, EntryPoint="llglCreateResourceHeap", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe ResourceHeap CreateResourceHeap(ref ResourceHeapDescriptor resourceHeapDesc);

        [DllImport(DllName, EntryPoint="llglCreateResourceHeapExt", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe ResourceHeap CreateResourceHeapExt(ref ResourceHeapDescriptor resourceHeapDesc, IntPtr numInitialResourceViews, ResourceViewDescriptor* initialResourceViews);

        [DllImport(DllName, EntryPoint="llglReleaseResourceHeap", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ReleaseResourceHeap(ResourceHeap resourceHeap);

        [DllImport(DllName, EntryPoint="llglWriteResourceHeap", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe int WriteResourceHeap(ResourceHeap resourceHeap, int firstDescriptor, IntPtr numResourceViews, ResourceViewDescriptor* resourceViews);

        [DllImport(DllName, EntryPoint="llglCreateRenderPass", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe RenderPass CreateRenderPass(ref RenderPassDescriptor renderPassDesc);

        [DllImport(DllName, EntryPoint="llglReleaseRenderPass", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ReleaseRenderPass(RenderPass renderPass);

        [DllImport(DllName, EntryPoint="llglCreateRenderTarget", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe RenderTarget CreateRenderTarget(ref RenderTargetDescriptor renderTargetDesc);

        [DllImport(DllName, EntryPoint="llglReleaseRenderTarget", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ReleaseRenderTarget(RenderTarget renderTarget);

        [DllImport(DllName, EntryPoint="llglCreateShader", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe Shader CreateShader(ref ShaderDescriptor shaderDesc);

        [DllImport(DllName, EntryPoint="llglReleaseShader", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ReleaseShader(Shader shader);

        [DllImport(DllName, EntryPoint="llglCreatePipelineLayout", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe PipelineLayout CreatePipelineLayout(ref PipelineLayoutDescriptor pipelineLayoutDesc);

        [DllImport(DllName, EntryPoint="llglReleasePipelineLayout", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ReleasePipelineLayout(PipelineLayout pipelineLayout);

        [DllImport(DllName, EntryPoint="llglCreatePipelineCache", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe PipelineCache CreatePipelineCache(void* initialBlobData, IntPtr initialBlobsize);

        [DllImport(DllName, EntryPoint="llglReleasePipelineCache", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ReleasePipelineCache(PipelineCache pipelineCache);

        [DllImport(DllName, EntryPoint="llglCreateGraphicsPipelineState", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe PipelineState CreateGraphicsPipelineState(ref GraphicsPipelineDescriptor pipelineStateDesc);

        [DllImport(DllName, EntryPoint="llglCreateGraphicsPipelineStateExt", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe PipelineState CreateGraphicsPipelineStateExt(ref GraphicsPipelineDescriptor pipelineStateDesc, PipelineCache pipelineCache);

        [DllImport(DllName, EntryPoint="llglCreateComputePipelineState", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe PipelineState CreateComputePipelineState(ref ComputePipelineDescriptor pipelineStateDesc);

        [DllImport(DllName, EntryPoint="llglCreateComputePipelineStateExt", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe PipelineState CreateComputePipelineStateExt(ref ComputePipelineDescriptor pipelineStateDesc, PipelineCache pipelineCache);

        [DllImport(DllName, EntryPoint="llglReleasePipelineState", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ReleasePipelineState(PipelineState pipelineState);

        [DllImport(DllName, EntryPoint="llglCreateQueryHeap", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe QueryHeap CreateQueryHeap(ref QueryHeapDescriptor queryHeapDesc);

        [DllImport(DllName, EntryPoint="llglReleaseQueryHeap", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ReleaseQueryHeap(QueryHeap queryHeap);

        [DllImport(DllName, EntryPoint="llglCreateFence", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe Fence CreateFence();

        [DllImport(DllName, EntryPoint="llglReleaseFence", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ReleaseFence(Fence fence);

        [DllImport(DllName, EntryPoint="llglGetRenderSystemNativeHandle", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool GetRenderSystemNativeHandle(void* nativeHandle, IntPtr nativeHandleSize);

        [DllImport(DllName, EntryPoint="llglSetDebugName", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetDebugName(RenderSystemChild renderSystemChild, [MarshalAs(UnmanagedType.LPStr)] string name);

        [DllImport(DllName, EntryPoint="llglSetName", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetName(RenderSystemChild renderSystemChild, [MarshalAs(UnmanagedType.LPStr)] string name);

        [DllImport(DllName, EntryPoint="llglGetRenderTargetResolution", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void GetRenderTargetResolution(RenderTarget renderTarget, ref Extent2D outResolution);

        [DllImport(DllName, EntryPoint="llglGetRenderTargetSamples", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe int GetRenderTargetSamples(RenderTarget renderTarget);

        [DllImport(DllName, EntryPoint="llglGetRenderTargetNumColorAttachments", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe int GetRenderTargetNumColorAttachments(RenderTarget renderTarget);

        [DllImport(DllName, EntryPoint="llglHasRenderTargetDepthAttachment", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool HasRenderTargetDepthAttachment(RenderTarget renderTarget);

        [DllImport(DllName, EntryPoint="llglHasRenderTargetStencilAttachment", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool HasRenderTargetStencilAttachment(RenderTarget renderTarget);

        [DllImport(DllName, EntryPoint="llglGetRenderTargetRenderPass", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe RenderPass GetRenderTargetRenderPass(RenderTarget renderTarget);

        [DllImport(DllName, EntryPoint="llglIsInstanceOfSwapChain", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool IsInstanceOfSwapChain(RenderTarget renderTarget);

        [DllImport(DllName, EntryPoint="llglAllocReport", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe Report AllocReport();

        [DllImport(DllName, EntryPoint="llglFreeReport", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void FreeReport(Report report);

        [DllImport(DllName, EntryPoint="llglGetReportText", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        public static extern unsafe string GetReportText(Report report);

        [DllImport(DllName, EntryPoint="llglHasReportErrors", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool HasReportErrors(Report report);

        [DllImport(DllName, EntryPoint="llglResetReport", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ResetReport(Report report, [MarshalAs(UnmanagedType.LPStr)] string text, [MarshalAs(UnmanagedType.I1)] bool hasErrors);

        [DllImport(DllName, EntryPoint="llglGetResourceType", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe ResourceType GetResourceType(Resource resource);

        [DllImport(DllName, EntryPoint="llglGetResourceNativeHandle", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool GetResourceNativeHandle(Resource resource, void* nativeHandle, IntPtr nativeHandleSize);

        [DllImport(DllName, EntryPoint="llglGetResourceHeapNumDescriptorSets", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe int GetResourceHeapNumDescriptorSets(ResourceHeap resourceHeap);

        [DllImport(DllName, EntryPoint="llglGetShaderReport", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe Report GetShaderReport(Shader shader);

        [DllImport(DllName, EntryPoint="llglReflectShader", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool ReflectShader(Shader shader, ref ShaderReflection reflection);

        [DllImport(DllName, EntryPoint="llglGetShaderType", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe ShaderType GetShaderType(Shader shader);

        [DllImport(DllName, EntryPoint="llglGetSurfaceNativeHandle", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool GetSurfaceNativeHandle(Surface surface, void* nativeHandle, IntPtr nativeHandleSize);

        [DllImport(DllName, EntryPoint="llglGetSurfaceContentSize", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void GetSurfaceContentSize(Surface surface, ref Extent2D outSize);

        [DllImport(DllName, EntryPoint="llglAdaptSurfaceForVideoMode", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool AdaptSurfaceForVideoMode(Surface surface, ref Extent2D outResolution, bool* outFullscreen);

        [DllImport(DllName, EntryPoint="llglProcessSurfaceEvents", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool ProcessSurfaceEvents();

        [DllImport(DllName, EntryPoint="llglFindSurfaceResidentDisplay", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe Display FindSurfaceResidentDisplay(Surface surface);

        [DllImport(DllName, EntryPoint="llglResetSurfacePixelFormat", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ResetSurfacePixelFormat(Surface surface);

        [DllImport(DllName, EntryPoint="llglIsPresentable", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool IsPresentable(SwapChain swapChain);

        [DllImport(DllName, EntryPoint="llglPresent", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void Present(SwapChain swapChain);

        [DllImport(DllName, EntryPoint="llglGetCurrentSwapIndex", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe int GetCurrentSwapIndex(SwapChain swapChain);

        [DllImport(DllName, EntryPoint="llglGetNumSwapBuffers", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe int GetNumSwapBuffers(SwapChain swapChain);

        [DllImport(DllName, EntryPoint="llglGetColorFormat", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe Format GetColorFormat(SwapChain swapChain);

        [DllImport(DllName, EntryPoint="llglGetDepthStencilFormat", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe Format GetDepthStencilFormat(SwapChain swapChain);

        [DllImport(DllName, EntryPoint="llglResizeBuffers", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool ResizeBuffers(SwapChain swapChain, ref Extent2D resolution, int flags);

        [DllImport(DllName, EntryPoint="llglSetVsyncInterval", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool SetVsyncInterval(SwapChain swapChain, int vsyncInterval);

        [DllImport(DllName, EntryPoint="llglSwitchFullscreen", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool SwitchFullscreen(SwapChain swapChain, [MarshalAs(UnmanagedType.I1)] bool enable);

        [DllImport(DllName, EntryPoint="llglGetSurface", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe Surface GetSurface(SwapChain swapChain);

        [DllImport(DllName, EntryPoint="llglGetTextureType", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe TextureType GetTextureType(Texture texture);

        [DllImport(DllName, EntryPoint="llglGetTextureBindFlags", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe int GetTextureBindFlags(Texture texture);

        [DllImport(DllName, EntryPoint="llglGetTextureDesc", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void GetTextureDesc(Texture texture, ref TextureDescriptor outDesc);

        [DllImport(DllName, EntryPoint="llglGetTextureFormat", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe Format GetTextureFormat(Texture texture);

        [DllImport(DllName, EntryPoint="llglGetTextureMipExtent", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void GetTextureMipExtent(Texture texture, int mipLevel, ref Extent3D outExtent);

        [DllImport(DllName, EntryPoint="llglGetTextureSubresourceFootprint", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void GetTextureSubresourceFootprint(Texture texture, int mipLevel, ref SubresourceFootprint outFootprint);

        [DllImport(DllName, EntryPoint="llglTimerFrequency", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe long TimerFrequency();

        [DllImport(DllName, EntryPoint="llglTimerTick", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe long TimerTick();

        [DllImport(DllName, EntryPoint="llglShaderTypeToString", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        public static extern unsafe string ShaderTypeToString(ShaderType val);

        [DllImport(DllName, EntryPoint="llglErrorTypeToString", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        public static extern unsafe string ErrorTypeToString(ErrorType val);

        [DllImport(DllName, EntryPoint="llglWarningTypeToString", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        public static extern unsafe string WarningTypeToString(WarningType val);

        [DllImport(DllName, EntryPoint="llglShadingLanguageToString", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        public static extern unsafe string ShadingLanguageToString(ShadingLanguage val);

        [DllImport(DllName, EntryPoint="llglFormatToString", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        public static extern unsafe string FormatToString(Format val);

        [DllImport(DllName, EntryPoint="llglTextureTypeToString", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        public static extern unsafe string TextureTypeToString(TextureType val);

        [DllImport(DllName, EntryPoint="llglBlendOpToString", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        public static extern unsafe string BlendOpToString(BlendOp val);

        [DllImport(DllName, EntryPoint="llglResourceTypeToString", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        public static extern unsafe string ResourceTypeToString(ResourceType val);

        [DllImport(DllName, EntryPoint="llglCreateWindow", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe Window CreateWindow(ref WindowDescriptor windowDesc);

        [DllImport(DllName, EntryPoint="llglReleaseWindow", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ReleaseWindow(Window window);

        [DllImport(DllName, EntryPoint="llglSetWindowPosition", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetWindowPosition(Window window, ref Offset2D position);

        [DllImport(DllName, EntryPoint="llglGetWindowPosition", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void GetWindowPosition(Window window, ref Offset2D outPosition);

        [DllImport(DllName, EntryPoint="llglSetWindowSize", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetWindowSize(Window window, ref Extent2D size, [MarshalAs(UnmanagedType.I1)] bool useClientArea);

        [DllImport(DllName, EntryPoint="llglGetWindowSize", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void GetWindowSize(Window window, ref Extent2D outSize, [MarshalAs(UnmanagedType.I1)] bool useClientArea);

        [DllImport(DllName, EntryPoint="llglSetWindowTitle", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetWindowTitle(Window window, [MarshalAs(UnmanagedType.LPWStr)] string title);

        [DllImport(DllName, EntryPoint="llglSetWindowTitleUTF8", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetWindowTitleUTF8(Window window, [MarshalAs(UnmanagedType.LPStr)] string title);

        [DllImport(DllName, EntryPoint="llglGetWindowTitle", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe IntPtr GetWindowTitle(Window window, IntPtr outTitleLength, char* outTitle);

        [DllImport(DllName, EntryPoint="llglGetWindowTitleUTF8", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe IntPtr GetWindowTitleUTF8(Window window, IntPtr outTitleLength, byte* outTitle);

        [DllImport(DllName, EntryPoint="llglShowWindow", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void ShowWindow(Window window, [MarshalAs(UnmanagedType.I1)] bool show);

        [DllImport(DllName, EntryPoint="llglIsWindowShown", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool IsWindowShown(Window window);

        [DllImport(DllName, EntryPoint="llglSetWindowDesc", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetWindowDesc(Window window, ref WindowDescriptor windowDesc);

        [DllImport(DllName, EntryPoint="llglGetWindowDesc", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void GetWindowDesc(Window window, ref WindowDescriptor outWindowDesc);

        [DllImport(DllName, EntryPoint="llglHasWindowFocus", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool HasWindowFocus(Window window);

        [DllImport(DllName, EntryPoint="llglHasWindowQuit", CallingConvention=CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern unsafe bool HasWindowQuit(Window window);

        [DllImport(DllName, EntryPoint="llglSetWindowUserData", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void SetWindowUserData(Window window, void* userData);

        [DllImport(DllName, EntryPoint="llglGetWindowUserData", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void* GetWindowUserData(Window window);

        [DllImport(DllName, EntryPoint="llglAddWindowEventListener", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe int AddWindowEventListener(Window window, ref WindowEventListener eventListener);

        [DllImport(DllName, EntryPoint="llglRemoveWindowEventListener", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void RemoveWindowEventListener(Window window, int eventListenerID);

        [DllImport(DllName, EntryPoint="llglPostWindowQuit", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PostWindowQuit(Window window);

        [DllImport(DllName, EntryPoint="llglPostWindowKeyDown", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PostWindowKeyDown(Window window, Key keyCode);

        [DllImport(DllName, EntryPoint="llglPostWindowKeyUp", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PostWindowKeyUp(Window window, Key keyCode);

        [DllImport(DllName, EntryPoint="llglPostWindowDoubleClick", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PostWindowDoubleClick(Window window, Key keyCode);

        [DllImport(DllName, EntryPoint="llglPostWindowChar", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PostWindowChar(Window window, char chr);

        [DllImport(DllName, EntryPoint="llglPostWindowWheelMotion", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PostWindowWheelMotion(Window window, int motion);

        [DllImport(DllName, EntryPoint="llglPostWindowLocalMotion", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PostWindowLocalMotion(Window window, ref Offset2D position);

        [DllImport(DllName, EntryPoint="llglPostWindowGlobalMotion", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PostWindowGlobalMotion(Window window, ref Offset2D motion);

        [DllImport(DllName, EntryPoint="llglPostWindowResize", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PostWindowResize(Window window, ref Extent2D clientAreaSize);

        [DllImport(DllName, EntryPoint="llglPostWindowUpdate", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PostWindowUpdate(Window window);

        [DllImport(DllName, EntryPoint="llglPostWindowGetFocus", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PostWindowGetFocus(Window window);

        [DllImport(DllName, EntryPoint="llglPostWindowLostFocus", CallingConvention=CallingConvention.Cdecl)]
        public static extern unsafe void PostWindowLostFocus(Window window);

        #pragma warning restore 0649 // Restore warning about unused fields
    }

    #endregion
}




// ================================================================================
