/*
 * LLGLWrapper.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* AUTO GENERATED CODE - DO NOT EDIT */

package llgl

// #cgo LDFLAGS: libLLGL.dll.a
// #cgo CFLAGS: -I ../../include
// #include <LLGL-C/LLGL.h>
import "C"

import "unsafe"


/* ----- Constants ----- */

const (
    RendererIDUndefined   = 0x00000000
    RendererIDNull        = 0x00000001
    RendererIDOpenGL      = 0x00000002
    RendererIDOpenGLES    = 0x00000003
    RendererIDWebGL       = 0x00000004
    RendererIDWebGPU      = 0x00000005
    RendererIDDirect3D9   = 0x00000006
    RendererIDDirect3D10  = 0x00000007
    RendererIDDirect3D11  = 0x00000008
    RendererIDDirect3D12  = 0x00000009
    RendererIDVulkan      = 0x0000000A
    RendererIDMetal       = 0x0000000B
    RendererIDOpenGLES1   = RendererIDOpenGLES
    RendererIDOpenGLES2   = RendererIDOpenGLES
    RendererIDOpenGLES3   = RendererIDOpenGLES
    RendererIDReserved    = 0x000000FF
)

/* ----- Enumerations ----- */

type EventAction int
const (
    EventActionBegan EventAction = iota
    EventActionChanged
    EventActionEnded
)

type RenderConditionMode int
const (
    RenderConditionModeWait RenderConditionMode = iota
    RenderConditionModeNoWait
    RenderConditionModeByRegionWait
    RenderConditionModeByRegionNoWait
    RenderConditionModeWaitInverted
    RenderConditionModeNoWaitInverted
    RenderConditionModeByRegionWaitInverted
    RenderConditionModeByRegionNoWaitInverted
)

type StencilFace int
const (
    StencilFaceFrontAndBack StencilFace = iota
    StencilFaceFront
    StencilFaceBack
)

type Format int
const (
    FormatUndefined Format = iota
    FormatA8UNorm
    FormatR8UNorm
    FormatR8SNorm
    FormatR8UInt
    FormatR8SInt
    FormatR16UNorm
    FormatR16SNorm
    FormatR16UInt
    FormatR16SInt
    FormatR16Float
    FormatR32UInt
    FormatR32SInt
    FormatR32Float
    FormatR64Float
    FormatRG8UNorm
    FormatRG8SNorm
    FormatRG8UInt
    FormatRG8SInt
    FormatRG16UNorm
    FormatRG16SNorm
    FormatRG16UInt
    FormatRG16SInt
    FormatRG16Float
    FormatRG32UInt
    FormatRG32SInt
    FormatRG32Float
    FormatRG64Float
    FormatRGB8UNorm
    FormatRGB8UNorm_sRGB
    FormatRGB8SNorm
    FormatRGB8UInt
    FormatRGB8SInt
    FormatRGB16UNorm
    FormatRGB16SNorm
    FormatRGB16UInt
    FormatRGB16SInt
    FormatRGB16Float
    FormatRGB32UInt
    FormatRGB32SInt
    FormatRGB32Float
    FormatRGB64Float
    FormatRGBA8UNorm
    FormatRGBA8UNorm_sRGB
    FormatRGBA8SNorm
    FormatRGBA8UInt
    FormatRGBA8SInt
    FormatRGBA16UNorm
    FormatRGBA16SNorm
    FormatRGBA16UInt
    FormatRGBA16SInt
    FormatRGBA16Float
    FormatRGBA32UInt
    FormatRGBA32SInt
    FormatRGBA32Float
    FormatRGBA64Float
    FormatBGRA8UNorm
    FormatBGRA8UNorm_sRGB
    FormatBGRA8SNorm
    FormatBGRA8UInt
    FormatBGRA8SInt
    FormatRGB10A2UNorm
    FormatRGB10A2UInt
    FormatRG11B10Float
    FormatRGB9E5Float
    FormatD16UNorm
    FormatD24UNormS8UInt
    FormatD32Float
    FormatD32FloatS8X24UInt
    FormatBC1UNorm
    FormatBC1UNorm_sRGB
    FormatBC2UNorm
    FormatBC2UNorm_sRGB
    FormatBC3UNorm
    FormatBC3UNorm_sRGB
    FormatBC4UNorm
    FormatBC4SNorm
    FormatBC5UNorm
    FormatBC5SNorm
    FormatASTC4x4
    FormatASTC4x4_sRGB
    FormatASTC5x4
    FormatASTC5x4_sRGB
    FormatASTC5x5
    FormatASTC5x5_sRGB
    FormatASTC6x5
    FormatASTC6x5_sRGB
    FormatASTC6x6
    FormatASTC6x6_sRGB
    FormatASTC8x5
    FormatASTC8x5_sRGB
    FormatASTC8x6
    FormatASTC8x6_sRGB
    FormatASTC8x8
    FormatASTC8x8_sRGB
    FormatASTC10x5
    FormatASTC10x5_sRGB
    FormatASTC10x6
    FormatASTC10x6_sRGB
    FormatASTC10x8
    FormatASTC10x8_sRGB
    FormatASTC10x10
    FormatASTC10x10_sRGB
    FormatASTC12x10
    FormatASTC12x10_sRGB
    FormatASTC12x12
    FormatASTC12x12_sRGB
    FormatETC1UNorm
    FormatETC2UNorm
    FormatETC2UNorm_sRGB
)

type ImageFormat int
const (
    ImageFormatAlpha ImageFormat = iota
    ImageFormatR
    ImageFormatRG
    ImageFormatRGB
    ImageFormatBGR
    ImageFormatRGBA
    ImageFormatBGRA
    ImageFormatARGB
    ImageFormatABGR
    ImageFormatDepth
    ImageFormatDepthStencil
    ImageFormatStencil
    ImageFormatCompressed
    ImageFormatBC1
    ImageFormatBC2
    ImageFormatBC3
    ImageFormatBC4
    ImageFormatBC5
)

type DataType int
const (
    DataTypeUndefined DataType = iota
    DataTypeInt8
    DataTypeUInt8
    DataTypeInt16
    DataTypeUInt16
    DataTypeInt32
    DataTypeUInt32
    DataTypeFloat16
    DataTypeFloat32
    DataTypeFloat64
)

type ReportType int
const (
    ReportTypeDefault ReportType = iota
    ReportTypeError
)

type Key int
const (
    KeyLButton Key = iota
    KeyRButton
    KeyCancel
    KeyMButton
    KeyXButton1
    KeyXButton2
    KeyBack
    KeyTab
    KeyClear
    KeyReturn
    KeyShift
    KeyControl
    KeyMenu
    KeyPause
    KeyCapital
    KeyEscape
    KeySpace
    KeyPageUp
    KeyPageDown
    KeyEnd
    KeyHome
    KeyLeft
    KeyUp
    KeyRight
    KeyDown
    KeySelect
    KeyPrint
    KeyExe
    KeySnapshot
    KeyInsert
    KeyDelete
    KeyHelp
    KeyD0
    KeyD1
    KeyD2
    KeyD3
    KeyD4
    KeyD5
    KeyD6
    KeyD7
    KeyD8
    KeyD9
    KeyA
    KeyB
    KeyC
    KeyD
    KeyE
    KeyF
    KeyG
    KeyH
    KeyI
    KeyJ
    KeyK
    KeyL
    KeyM
    KeyN
    KeyO
    KeyP
    KeyQ
    KeyR
    KeyS
    KeyT
    KeyU
    KeyV
    KeyW
    KeyX
    KeyY
    KeyZ
    KeyLWin
    KeyRWin
    KeyApps
    KeySleep
    KeyKeypad0
    KeyKeypad1
    KeyKeypad2
    KeyKeypad3
    KeyKeypad4
    KeyKeypad5
    KeyKeypad6
    KeyKeypad7
    KeyKeypad8
    KeyKeypad9
    KeyKeypadMultiply
    KeyKeypadPlus
    KeyKeypadSeparator
    KeyKeypadMinus
    KeyKeypadDecimal
    KeyKeypadDivide
    KeyF1
    KeyF2
    KeyF3
    KeyF4
    KeyF5
    KeyF6
    KeyF7
    KeyF8
    KeyF9
    KeyF10
    KeyF11
    KeyF12
    KeyF13
    KeyF14
    KeyF15
    KeyF16
    KeyF17
    KeyF18
    KeyF19
    KeyF20
    KeyF21
    KeyF22
    KeyF23
    KeyF24
    KeyNumLock
    KeyScrollLock
    KeyLShift
    KeyRShift
    KeyLControl
    KeyRControl
    KeyLMenu
    KeyRMenu
    KeyBrowserBack
    KeyBrowserForward
    KeyBrowserRefresh
    KeyBrowserStop
    KeyBrowserSearch
    KeyBrowserFavorits
    KeyBrowserHome
    KeyVolumeMute
    KeyVolumeDown
    KeyVolumeUp
    KeyMediaNextTrack
    KeyMediaPrevTrack
    KeyMediaStop
    KeyMediaPlayPause
    KeyLaunchMail
    KeyLaunchMediaSelect
    KeyLaunchApp1
    KeyLaunchApp2
    KeyPlus
    KeyComma
    KeyMinus
    KeyPeriod
    KeyExponent
    KeyAttn
    KeyCrSel
    KeyExSel
    KeyErEOF
    KeyPlay
    KeyZoom
    KeyNoName
    KeyPA1
    KeyOEMClear
    KeyAny
)

type UniformType int
const (
    UniformTypeUndefined UniformType = iota
    UniformTypeFloat1
    UniformTypeFloat2
    UniformTypeFloat3
    UniformTypeFloat4
    UniformTypeDouble1
    UniformTypeDouble2
    UniformTypeDouble3
    UniformTypeDouble4
    UniformTypeInt1
    UniformTypeInt2
    UniformTypeInt3
    UniformTypeInt4
    UniformTypeUInt1
    UniformTypeUInt2
    UniformTypeUInt3
    UniformTypeUInt4
    UniformTypeBool1
    UniformTypeBool2
    UniformTypeBool3
    UniformTypeBool4
    UniformTypeFloat2x2
    UniformTypeFloat2x3
    UniformTypeFloat2x4
    UniformTypeFloat3x2
    UniformTypeFloat3x3
    UniformTypeFloat3x4
    UniformTypeFloat4x2
    UniformTypeFloat4x3
    UniformTypeFloat4x4
    UniformTypeDouble2x2
    UniformTypeDouble2x3
    UniformTypeDouble2x4
    UniformTypeDouble3x2
    UniformTypeDouble3x3
    UniformTypeDouble3x4
    UniformTypeDouble4x2
    UniformTypeDouble4x3
    UniformTypeDouble4x4
    UniformTypeSampler
    UniformTypeImage
    UniformTypeAtomicCounter
)

type PrimitiveTopology int
const (
    PrimitiveTopologyPointList PrimitiveTopology = iota
    PrimitiveTopologyLineList
    PrimitiveTopologyLineStrip
    PrimitiveTopologyLineListAdjacency
    PrimitiveTopologyLineStripAdjacency
    PrimitiveTopologyTriangleList
    PrimitiveTopologyTriangleStrip
    PrimitiveTopologyTriangleListAdjacency
    PrimitiveTopologyTriangleStripAdjacency
    PrimitiveTopologyPatches1
    PrimitiveTopologyPatches2
    PrimitiveTopologyPatches3
    PrimitiveTopologyPatches4
    PrimitiveTopologyPatches5
    PrimitiveTopologyPatches6
    PrimitiveTopologyPatches7
    PrimitiveTopologyPatches8
    PrimitiveTopologyPatches9
    PrimitiveTopologyPatches10
    PrimitiveTopologyPatches11
    PrimitiveTopologyPatches12
    PrimitiveTopologyPatches13
    PrimitiveTopologyPatches14
    PrimitiveTopologyPatches15
    PrimitiveTopologyPatches16
    PrimitiveTopologyPatches17
    PrimitiveTopologyPatches18
    PrimitiveTopologyPatches19
    PrimitiveTopologyPatches20
    PrimitiveTopologyPatches21
    PrimitiveTopologyPatches22
    PrimitiveTopologyPatches23
    PrimitiveTopologyPatches24
    PrimitiveTopologyPatches25
    PrimitiveTopologyPatches26
    PrimitiveTopologyPatches27
    PrimitiveTopologyPatches28
    PrimitiveTopologyPatches29
    PrimitiveTopologyPatches30
    PrimitiveTopologyPatches31
    PrimitiveTopologyPatches32
)

type CompareOp int
const (
    CompareOpNeverPass CompareOp = iota
    CompareOpLess
    CompareOpEqual
    CompareOpLessEqual
    CompareOpGreater
    CompareOpNotEqual
    CompareOpGreaterEqual
    CompareOpAlwaysPass
)

type StencilOp int
const (
    StencilOpKeep StencilOp = iota
    StencilOpZero
    StencilOpReplace
    StencilOpIncClamp
    StencilOpDecClamp
    StencilOpInvert
    StencilOpIncWrap
    StencilOpDecWrap
)

type BlendOp int
const (
    BlendOpZero BlendOp = iota
    BlendOpOne
    BlendOpSrcColor
    BlendOpInvSrcColor
    BlendOpSrcAlpha
    BlendOpInvSrcAlpha
    BlendOpDstColor
    BlendOpInvDstColor
    BlendOpDstAlpha
    BlendOpInvDstAlpha
    BlendOpSrcAlphaSaturate
    BlendOpBlendFactor
    BlendOpInvBlendFactor
    BlendOpSrc1Color
    BlendOpInvSrc1Color
    BlendOpSrc1Alpha
    BlendOpInvSrc1Alpha
)

type BlendArithmetic int
const (
    BlendArithmeticAdd BlendArithmetic = iota
    BlendArithmeticSubtract
    BlendArithmeticRevSubtract
    BlendArithmeticMin
    BlendArithmeticMax
)

type PolygonMode int
const (
    PolygonModeFill PolygonMode = iota
    PolygonModeWireframe
    PolygonModePoints
)

type CullMode int
const (
    CullModeDisabled CullMode = iota
    CullModeFront
    CullModeBack
)

type LogicOp int
const (
    LogicOpDisabled LogicOp = iota
    LogicOpClear
    LogicOpSet
    LogicOpCopy
    LogicOpCopyInverted
    LogicOpNoOp
    LogicOpInvert
    LogicOpAND
    LogicOpANDReverse
    LogicOpANDInverted
    LogicOpNAND
    LogicOpOR
    LogicOpORReverse
    LogicOpORInverted
    LogicOpNOR
    LogicOpXOR
    LogicOpEquiv
)

type TessellationPartition int
const (
    TessellationPartitionUndefined TessellationPartition = iota
    TessellationPartitionInteger
    TessellationPartitionPow2
    TessellationPartitionFractionalOdd
    TessellationPartitionFractionalEven
)

type QueryType int
const (
    QueryTypeSamplesPassed QueryType = iota
    QueryTypeAnySamplesPassed
    QueryTypeAnySamplesPassedConservative
    QueryTypeTimeElapsed
    QueryTypeStreamOutPrimitivesWritten
    QueryTypeStreamOutOverflow
    QueryTypePipelineStatistics
)

type ErrorType int
const (
    ErrorTypeInvalidArgument ErrorType = iota
    ErrorTypeInvalidState
    ErrorTypeUnsupportedFeature
    ErrorTypeUndefinedBehavior
)

type WarningType int
const (
    WarningTypeImproperArgument WarningType = iota
    WarningTypeImproperState
    WarningTypePointlessOperation
    WarningTypeVaryingBehavior
)

type AttachmentLoadOp int
const (
    AttachmentLoadOpUndefined AttachmentLoadOp = iota
    AttachmentLoadOpLoad
    AttachmentLoadOpClear
)

type AttachmentStoreOp int
const (
    AttachmentStoreOpUndefined AttachmentStoreOp = iota
    AttachmentStoreOpStore
)

type ShadingLanguage int
const (
    ShadingLanguageGLSL ShadingLanguage = iota
    ShadingLanguageGLSL_110
    ShadingLanguageGLSL_120
    ShadingLanguageGLSL_130
    ShadingLanguageGLSL_140
    ShadingLanguageGLSL_150
    ShadingLanguageGLSL_330
    ShadingLanguageGLSL_400
    ShadingLanguageGLSL_410
    ShadingLanguageGLSL_420
    ShadingLanguageGLSL_430
    ShadingLanguageGLSL_440
    ShadingLanguageGLSL_450
    ShadingLanguageGLSL_460
    ShadingLanguageESSL
    ShadingLanguageESSL_100
    ShadingLanguageESSL_300
    ShadingLanguageESSL_310
    ShadingLanguageESSL_320
    ShadingLanguageHLSL
    ShadingLanguageHLSL_2_0
    ShadingLanguageHLSL_2_0a
    ShadingLanguageHLSL_2_0b
    ShadingLanguageHLSL_3_0
    ShadingLanguageHLSL_4_0
    ShadingLanguageHLSL_4_1
    ShadingLanguageHLSL_5_0
    ShadingLanguageHLSL_5_1
    ShadingLanguageHLSL_6_0
    ShadingLanguageHLSL_6_1
    ShadingLanguageHLSL_6_2
    ShadingLanguageHLSL_6_3
    ShadingLanguageHLSL_6_4
    ShadingLanguageHLSL_6_5
    ShadingLanguageHLSL_6_6
    ShadingLanguageHLSL_6_7
    ShadingLanguageHLSL_6_8
    ShadingLanguageMetal
    ShadingLanguageMetal_1_0
    ShadingLanguageMetal_1_1
    ShadingLanguageMetal_1_2
    ShadingLanguageMetal_2_0
    ShadingLanguageMetal_2_1
    ShadingLanguageMetal_2_2
    ShadingLanguageMetal_2_3
    ShadingLanguageMetal_2_4
    ShadingLanguageMetal_3_0
    ShadingLanguageSPIRV
    ShadingLanguageSPIRV_100
    ShadingLanguageVersionBitmask
)

type ScreenOrigin int
const (
    ScreenOriginLowerLeft ScreenOrigin = iota
    ScreenOriginUpperLeft
)

type ClippingRange int
const (
    ClippingRangeMinusOneToOne ClippingRange = iota
    ClippingRangeZeroToOne
)

type CPUAccess int
const (
    CPUAccessReadOnly CPUAccess = iota
    CPUAccessWriteOnly
    CPUAccessWriteDiscard
    CPUAccessReadWrite
)

type ResourceType int
const (
    ResourceTypeUndefined ResourceType = iota
    ResourceTypeBuffer
    ResourceTypeTexture
    ResourceTypeSampler
)

type SamplerAddressMode int
const (
    SamplerAddressModeRepeat SamplerAddressMode = iota
    SamplerAddressModeMirror
    SamplerAddressModeClamp
    SamplerAddressModeBorder
    SamplerAddressModeMirrorOnce
)

type SamplerFilter int
const (
    SamplerFilterNearest SamplerFilter = iota
    SamplerFilterLinear
)

type ShaderType int
const (
    ShaderTypeUndefined ShaderType = iota
    ShaderTypeVertex
    ShaderTypeTessControl
    ShaderTypeTessEvaluation
    ShaderTypeGeometry
    ShaderTypeFragment
    ShaderTypeCompute
)

type ShaderSourceType int
const (
    ShaderSourceTypeCodeString ShaderSourceType = iota
    ShaderSourceTypeCodeFile
    ShaderSourceTypeBinaryBuffer
    ShaderSourceTypeBinaryFile
)

type StorageBufferType int
const (
    StorageBufferTypeUndefined StorageBufferType = iota
    StorageBufferTypeTypedBuffer
    StorageBufferTypeStructuredBuffer
    StorageBufferTypeByteAddressBuffer
    StorageBufferTypeRWTypedBuffer
    StorageBufferTypeRWStructuredBuffer
    StorageBufferTypeRWByteAddressBuffer
    StorageBufferTypeAppendStructuredBuffer
    StorageBufferTypeConsumeStructuredBuffer
)

type SystemValue int
const (
    SystemValueUndefined SystemValue = iota
    SystemValueClipDistance
    SystemValueColor
    SystemValueCullDistance
    SystemValueDepth
    SystemValueDepthGreater
    SystemValueDepthLess
    SystemValueFrontFacing
    SystemValueInstanceID
    SystemValuePosition
    SystemValuePrimitiveID
    SystemValueRenderTargetIndex
    SystemValueSampleMask
    SystemValueSampleID
    SystemValueStencil
    SystemValueVertexID
    SystemValueViewportIndex
)

type TextureType int
const (
    TextureTypeTexture1D TextureType = iota
    TextureTypeTexture2D
    TextureTypeTexture3D
    TextureTypeTextureCube
    TextureTypeTexture1DArray
    TextureTypeTexture2DArray
    TextureTypeTextureCubeArray
    TextureTypeTexture2DMS
    TextureTypeTexture2DMSArray
)

type TextureSwizzle int
const (
    TextureSwizzleZero TextureSwizzle = iota
    TextureSwizzleOne
    TextureSwizzleRed
    TextureSwizzleGreen
    TextureSwizzleBlue
    TextureSwizzleAlpha
)


/* ----- Flags ----- */

type CanvasFlags int
const (
    CanvasBorderless = (1 << 0)
)

type CommandBufferFlags int
const (
    CommandBufferSecondary       = (1 << 0)
    CommandBufferMultiSubmit     = (1 << 1)
    CommandBufferImmediateSubmit = (1 << 2)
)

type ClearFlags int
const (
    ClearColor        = (1 << 0)
    ClearDepth        = (1 << 1)
    ClearStencil      = (1 << 2)
    ClearColorDepth   = (ClearColor | ClearDepth)
    ClearDepthStencil = (ClearDepth | ClearStencil)
    ClearAll          = (ClearColor | ClearDepth | ClearStencil)
)

type FormatFlags int
const (
    FormatHasDepth             = (1 << 0)
    FormatHasStencil           = (1 << 1)
    FormatIsColorSpace_sRGB    = (1 << 2)
    FormatIsCompressed         = (1 << 3)
    FormatIsNormalized         = (1 << 4)
    FormatIsInteger            = (1 << 5)
    FormatIsUnsigned           = (1 << 6)
    FormatIsPacked             = (1 << 7)
    FormatSupportsRenderTarget = (1 << 8)
    FormatSupportsMips         = (1 << 9)
    FormatSupportsGenerateMips = (1 << 10)
    FormatSupportsTexture1D    = (1 << 11)
    FormatSupportsTexture2D    = (1 << 12)
    FormatSupportsTexture3D    = (1 << 13)
    FormatSupportsTextureCube  = (1 << 14)
    FormatSupportsVertex       = (1 << 15)
    FormatIsUnsignedInteger    = (FormatIsUnsigned | FormatIsInteger)
    FormatHasDepthStencil      = (FormatHasDepth | FormatHasStencil)
)

type StdOutFlags int
const (
    StdOutColored = (1 << 0)
)

type ColorFlags int
const (
    ColorDefault       = (1 << 0)
    ColorRed           = (1 << 1)
    ColorGreen         = (1 << 2)
    ColorBlue          = (1 << 3)
    ColorBright        = (1 << 4)
    ColorBold          = (1 << 5)
    ColorUnderline     = (1 << 6)
    ColorFullRGB       = (1 << 7)
    ColorYellow        = (ColorRed | ColorGreen)
    ColorPink          = (ColorRed | ColorBlue)
    ColorCyan          = (ColorGreen | ColorBlue)
    ColorGray          = (ColorRed | ColorGreen | ColorBlue)
    ColorBrightRed     = (ColorBright | ColorRed)
    ColorBrightGreen   = (ColorBright | ColorGreen)
    ColorBrightBlue    = (ColorBright | ColorBlue)
    ColorBrightYellow  = (ColorBright | ColorYellow)
    ColorBrightPink    = (ColorBright | ColorPink)
    ColorBrightCyan    = (ColorBright | ColorCyan)
    ColorWhite         = (ColorBright | ColorGray)
    ColorStdError      = (ColorBold | ColorRed)
    ColorStdWarning    = (ColorBold | ColorBrightYellow)
    ColorStdAnnotation = (ColorBold | ColorBrightPink)
)

type BarrierFlags int
const (
    BarrierStorageBuffer  = (1 << 0)
    BarrierStorageTexture = (1 << 1)
    BarrierStorage        = (BarrierStorageBuffer | BarrierStorageTexture)
)

type ColorMaskFlags int
const (
    ColorMaskZero = 0
    ColorMaskR    = (1 << 0)
    ColorMaskG    = (1 << 1)
    ColorMaskB    = (1 << 2)
    ColorMaskA    = (1 << 3)
    ColorMaskAll  = (ColorMaskR | ColorMaskG | ColorMaskB | ColorMaskA)
)

type RenderSystemFlags int
const (
    RenderSystemDebugDevice       = (1 << 0)
    RenderSystemPreferNVIDIA      = (1 << 1)
    RenderSystemPreferAMD         = (1 << 2)
    RenderSystemPreferIntel       = (1 << 3)
    RenderSystemSoftwareDevice    = (1 << 4)
    RenderSystemDebugBreakOnError = (1 << 5)
)

type BindFlags int
const (
    BindVertexBuffer           = (1 << 0)
    BindIndexBuffer            = (1 << 1)
    BindConstantBuffer         = (1 << 2)
    BindStreamOutputBuffer     = (1 << 3)
    BindIndirectBuffer         = (1 << 4)
    BindSampled                = (1 << 5)
    BindStorage                = (1 << 6)
    BindColorAttachment        = (1 << 7)
    BindDepthStencilAttachment = (1 << 8)
    BindCombinedSampler        = (1 << 9)
    BindCopySrc                = (1 << 10)
    BindCopyDst                = (1 << 11)
)

type CPUAccessFlags int
const (
    CPUAccessRead  = (1 << 0)
    CPUAccessWrite = (1 << 1)
)

type MiscFlags int
const (
    MiscDynamicUsage  = (1 << 0)
    MiscFixedSamples  = (1 << 1)
    MiscGenerateMips  = (1 << 2)
    MiscNoInitialData = (1 << 3)
    MiscAppend        = (1 << 4)
    MiscCounter       = (1 << 5)
)

type ShaderCompileFlags int
const (
    ShaderCompileDebug               = (1 << 0)
    ShaderCompileNoOptimization      = (1 << 1)
    ShaderCompileOptimizationLevel1  = (1 << 2)
    ShaderCompileOptimizationLevel2  = (1 << 3)
    ShaderCompileOptimizationLevel3  = (1 << 4)
    ShaderCompileWarningsAreErrors   = (1 << 5)
    ShaderCompilePatchClippingOrigin = (1 << 6)
    ShaderCompileSeparateShader      = (1 << 7)
    ShaderCompileDefaultLibrary      = (1 << 8)
)

type StageFlags int
const (
    StageVertexStage         = (1 << 0)
    StageTessControlStage    = (1 << 1)
    StageTessEvaluationStage = (1 << 2)
    StageGeometryStage       = (1 << 3)
    StageFragmentStage       = (1 << 4)
    StageComputeStage        = (1 << 5)
    StageAllTessStages       = (StageTessControlStage | StageTessEvaluationStage)
    StageAllGraphicsStages   = (StageVertexStage | StageAllTessStages | StageGeometryStage | StageFragmentStage)
    StageAllStages           = (StageAllGraphicsStages | StageComputeStage)
)

type ResizeBuffersFlags int
const (
    ResizeBuffersAdaptSurface   = (1 << 0)
    ResizeBuffersFullscreenMode = (1 << 1)
    ResizeBuffersWindowedMode   = (1 << 2)
)

type WindowFlags int
const (
    WindowVisible              = (1 << 0)
    WindowBorderless           = (1 << 1)
    WindowResizable            = (1 << 2)
    WindowCentered             = (1 << 3)
    WindowAcceptDropFiles      = (1 << 4)
    WindowDisableClearOnResize = (1 << 5)
    WindowDisableSizeScaling   = (1 << 6)
)


/* ----- Structures ----- */

type CanvasDescriptor struct {
    Title string
    Flags uint   /* = 0 */
}

type ClearValue struct {
    Color   [4]float32 /* = {0.0,0.0,0.0,0.0} */
    Depth   float32    /* = 1.0 */
    Stencil uint32     /* = 0 */
}

type CommandBufferDescriptor struct {
    DebugName          string      /* = "" */
    Flags              uint        /* = 0 */
    NumNativeBuffers   uint32      /* = 0 */
    MinStagingPoolSize uint64      /* = (0xFFFF+1) */
    RenderPass         *RenderPass /* = nil */
}

type DrawIndirectArguments struct {
    NumVertices   uint32
    NumInstances  uint32
    FirstVertex   uint32
    FirstInstance uint32
}

type DrawIndexedIndirectArguments struct {
    NumIndices    uint32
    NumInstances  uint32
    FirstIndex    uint32
    VertexOffset  int32
    FirstInstance uint32
}

type DrawPatchIndirectArguments struct {
    NumPatches    uint32
    NumInstances  uint32
    FirstPatch    uint32
    FirstInstance uint32
}

type DispatchIndirectArguments struct {
    NumThreadGroups [3]uint32
}

type ColorCodes struct {
    TextFlags       uint /* = 0 */
    BackgroundFlags uint /* = 0 */
}

type BindingSlot struct {
    Index uint32 /* = 0 */
    Set   uint32 /* = 0 */
}

type Viewport struct {
    X        float32 /* = 0.0 */
    Y        float32 /* = 0.0 */
    Width    float32 /* = 0.0 */
    Height   float32 /* = 0.0 */
    MinDepth float32 /* = 0.0 */
    MaxDepth float32 /* = 1.0 */
}

type Scissor struct {
    X      int32 /* = 0 */
    Y      int32 /* = 0 */
    Width  int32 /* = 0 */
    Height int32 /* = 0 */
}

type DepthBiasDescriptor struct {
    ConstantFactor float32 /* = 0.0 */
    SlopeFactor    float32 /* = 0.0 */
    Clamp          float32 /* = 0.0 */
}

type ComputePipelineDescriptor struct {
    DebugName      string          /* = "" */
    PipelineLayout *PipelineLayout /* = nil */
    ComputeShader  *Shader         /* = nil */
}

type QueryPipelineStatistics struct {
    InputAssemblyVertices           uint64 /* = 0 */
    InputAssemblyPrimitives         uint64 /* = 0 */
    VertexShaderInvocations         uint64 /* = 0 */
    GeometryShaderInvocations       uint64 /* = 0 */
    GeometryShaderPrimitives        uint64 /* = 0 */
    ClippingInvocations             uint64 /* = 0 */
    ClippingPrimitives              uint64 /* = 0 */
    FragmentShaderInvocations       uint64 /* = 0 */
    TessControlShaderInvocations    uint64 /* = 0 */
    TessEvaluationShaderInvocations uint64 /* = 0 */
    ComputeShaderInvocations        uint64 /* = 0 */
}

type ProfileTimeRecord struct {
    Annotation    string
    CPUTicksStart uint64 /* = 0 */
    CPUTicksEnd   uint64 /* = 0 */
    ElapsedTime   uint64 /* = 0 */
}

type ProfileCommandQueueRecord struct {
    BufferWrites             uint32 /* = 0 */
    BufferReads              uint32 /* = 0 */
    BufferMappings           uint32 /* = 0 */
    TextureWrites            uint32 /* = 0 */
    TextureReads             uint32 /* = 0 */
    CommandBufferSubmittions uint32 /* = 0 */
    FenceSubmissions         uint32 /* = 0 */
}

type ProfileCommandBufferRecord struct {
    Encodings                uint32 /* = 0 */
    MipMapsGenerations       uint32 /* = 0 */
    VertexBufferBindings     uint32 /* = 0 */
    IndexBufferBindings      uint32 /* = 0 */
    ConstantBufferBindings   uint32 /* = 0 */
    SampledBufferBindings    uint32 /* = 0 */
    StorageBufferBindings    uint32 /* = 0 */
    SampledTextureBindings   uint32 /* = 0 */
    StorageTextureBindings   uint32 /* = 0 */
    SamplerBindings          uint32 /* = 0 */
    ResourceHeapBindings     uint32 /* = 0 */
    GraphicsPipelineBindings uint32 /* = 0 */
    ComputePipelineBindings  uint32 /* = 0 */
    AttachmentClears         uint32 /* = 0 */
    BufferUpdates            uint32 /* = 0 */
    BufferCopies             uint32 /* = 0 */
    BufferFills              uint32 /* = 0 */
    TextureCopies            uint32 /* = 0 */
    RenderPassSections       uint32 /* = 0 */
    StreamOutputSections     uint32 /* = 0 */
    QuerySections            uint32 /* = 0 */
    RenderConditionSections  uint32 /* = 0 */
    DrawCommands             uint32 /* = 0 */
    DispatchCommands         uint32 /* = 0 */
}

type RendererInfo struct {
    RendererName        string
    DeviceName          string
    VendorName          string
    ShadingLanguageName string
    ExtensionNames      string /* = nil */
    PipelineCacheID     []byte /* = nil */
}

type RenderingFeatures struct {
    HasRenderTargets             bool /* = false */
    Has3DTextures                bool /* = false */
    HasCubeTextures              bool /* = false */
    HasArrayTextures             bool /* = false */
    HasCubeArrayTextures         bool /* = false */
    HasMultiSampleTextures       bool /* = false */
    HasMultiSampleArrayTextures  bool /* = false */
    HasTextureViews              bool /* = false */
    HasTextureViewSwizzle        bool /* = false */
    HasTextureViewFormatSwizzle  bool /* = false */
    HasBufferViews               bool /* = false */
    HasSamplers                  bool /* LLGLRenderingFeatures.hasSamplers is deprecated since 0.04b; All backends must support sampler states either natively or emulated. */
    HasConstantBuffers           bool /* = false */
    HasStorageBuffers            bool /* = false */
    HasUniforms                  bool /* LLGLRenderingFeatures.hasUniforms is deprecated since 0.04b; All backends must support uniforms either natively or emulated. */
    HasGeometryShaders           bool /* = false */
    HasTessellationShaders       bool /* = false */
    HasTessellatorStage          bool /* = false */
    HasComputeShaders            bool /* = false */
    HasInstancing                bool /* = false */
    HasOffsetInstancing          bool /* = false */
    HasIndirectDrawing           bool /* = false */
    HasViewportArrays            bool /* = false */
    HasConservativeRasterization bool /* = false */
    HasStreamOutputs             bool /* = false */
    HasLogicOp                   bool /* = false */
    HasPipelineCaching           bool /* = false */
    HasPipelineStatistics        bool /* = false */
    HasRenderCondition           bool /* = false */
}

type RenderingLimits struct {
    LineWidthRange                [2]float32 /* = {1.0,1.0} */
    MaxTextureArrayLayers         uint32     /* = 0 */
    MaxColorAttachments           uint32     /* = 0 */
    MaxPatchVertices              uint32     /* = 0 */
    Max1DTextureSize              uint32     /* = 0 */
    Max2DTextureSize              uint32     /* = 0 */
    Max3DTextureSize              uint32     /* = 0 */
    MaxCubeTextureSize            uint32     /* = 0 */
    MaxAnisotropy                 uint32     /* = 0 */
    MaxComputeShaderWorkGroups    [3]uint32  /* = {0,0,0} */
    MaxComputeShaderWorkGroupSize [3]uint32  /* = {0,0,0} */
    MaxViewports                  uint32     /* = 0 */
    MaxViewportSize               [2]uint32  /* = {0,0} */
    MaxBufferSize                 uint64     /* = 0 */
    MaxConstantBufferSize         uint64     /* = 0 */
    MaxStreamOutputs              uint32     /* = 0 */
    MaxTessFactor                 uint32     /* = 0 */
    MinConstantBufferAlignment    uint64     /* = 0 */
    MinSampledBufferAlignment     uint64     /* = 0 */
    MinStorageBufferAlignment     uint64     /* = 0 */
    MaxColorBufferSamples         uint32     /* = 0 */
    MaxDepthBufferSamples         uint32     /* = 0 */
    MaxStencilBufferSamples       uint32     /* = 0 */
    MaxNoAttachmentSamples        uint32     /* = 0 */
    StorageResourceStageFlags     uint       /* = 0 */
}

type ResourceHeapDescriptor struct {
    DebugName        string          /* = "" */
    PipelineLayout   *PipelineLayout /* = nil */
    NumResourceViews uint32          /* = 0 */
    BarrierFlags     uint            /* ResourceHeapDescriptor.barrierFlags is deprecated since 0.04b; Use PipelineLayoutDescriptor.barrierFlags instead! */
}

type ShaderMacro struct {
    Name       string /* = "" */
    Definition string /* = "" */
}

type TextureSubresource struct {
    BaseArrayLayer uint32 /* = 0 */
    NumArrayLayers uint32 /* = 1 */
    BaseMipLevel   uint32 /* = 0 */
    NumMipLevels   uint32 /* = 1 */
}

type SubresourceFootprint struct {
    Size         uint64 /* = 0 */
    RowAlignment uint32 /* = 0 */
    RowSize      uint32 /* = 0 */
    RowStride    uint32 /* = 0 */
    LayerSize    uint32 /* = 0 */
    LayerStride  uint32 /* = 0 */
}

type Extent2D struct {
    Width  uint32 /* = 0 */
    Height uint32 /* = 0 */
}

type Extent3D struct {
    Width  uint32 /* = 0 */
    Height uint32 /* = 0 */
    Depth  uint32 /* = 0 */
}

type Offset2D struct {
    X int32 /* = 0 */
    Y int32 /* = 0 */
}

type Offset3D struct {
    X int32 /* = 0 */
    Y int32 /* = 0 */
    Z int32 /* = 0 */
}

type BufferViewDescriptor struct {
    Format Format /* = FormatUndefined */
    Offset uint64 /* = 0 */
    Size   uint64 /* = LLGL_WHOLE_SIZE */
}

type AttachmentClear struct {
    Flags           uint       /* = 0 */
    ColorAttachment uint32     /* = 0 */
    ClearValue      ClearValue
}

type DisplayMode struct {
    Resolution  Extent2D
    RefreshRate uint32   /* = 0 */
}

type FormatAttributes struct {
    BitSize     uint16
    BlockWidth  uint8
    BlockHeight uint8
    Components  uint8
    Format      ImageFormat
    DataType    DataType
    Flags       uint
}

type FragmentAttribute struct {
    Name        string
    Format      Format      /* = FormatRGBA32Float */
    Location    uint32      /* = 0 */
    SystemValue SystemValue /* = SystemValueUndefined */
}

type MutableImageView struct {
    Format   ImageFormat    /* = ImageFormatRGBA */
    DataType DataType       /* = DataTypeUInt8 */
    Data     unsafe.Pointer /* = nil */
    DataSize uintptr        /* = 0 */
}

type ImageView struct {
    Format      ImageFormat    /* = ImageFormatRGBA */
    DataType    DataType       /* = DataTypeUInt8 */
    Data        unsafe.Pointer /* = nil */
    DataSize    uintptr        /* = 0 */
    RowStride   uint32         /* = 0 */
    LayerStride uint32         /* = 0 */
}

type BindingDescriptor struct {
    Name       string
    Type       ResourceType /* = ResourceTypeUndefined */
    BindFlags  uint         /* = 0 */
    StageFlags uint         /* = 0 */
    Slot       BindingSlot
    ArraySize  uint32       /* = 0 */
}

type UniformDescriptor struct {
    Name      string
    Type      UniformType /* = UniformTypeUndefined */
    ArraySize uint32      /* = 0 */
}

type CombinedTextureSamplerDescriptor struct {
    Name        string
    TextureName string
    SamplerName string
    Slot        BindingSlot
}

type DepthDescriptor struct {
    TestEnabled  bool      /* = false */
    WriteEnabled bool      /* = false */
    CompareOp    CompareOp /* = CompareOpLess */
}

type StencilFaceDescriptor struct {
    StencilFailOp StencilOp /* = StencilOpKeep */
    DepthFailOp   StencilOp /* = StencilOpKeep */
    DepthPassOp   StencilOp /* = StencilOpKeep */
    CompareOp     CompareOp /* = CompareOpLess */
    ReadMask      uint32    /* = ~0u */
    WriteMask     uint32    /* = ~0u */
    Reference     uint32    /* = 0u */
}

type RasterizerDescriptor struct {
    PolygonMode               PolygonMode         /* = PolygonModeFill */
    CullMode                  CullMode            /* = CullModeDisabled */
    DepthBias                 DepthBiasDescriptor
    FrontCCW                  bool                /* = false */
    DiscardEnabled            bool                /* = false */
    DepthClampEnabled         bool                /* = false */
    ScissorTestEnabled        bool                /* = false */
    MultiSampleEnabled        bool                /* = false */
    AntiAliasedLineEnabled    bool                /* = false */
    ConservativeRasterization bool                /* = false */
    LineWidth                 float32             /* = 1.0 */
}

type BlendTargetDescriptor struct {
    BlendEnabled    bool            /* = false */
    SrcColor        BlendOp         /* = BlendOpSrcAlpha */
    DstColor        BlendOp         /* = BlendOpInvSrcAlpha */
    ColorArithmetic BlendArithmetic /* = BlendArithmeticAdd */
    SrcAlpha        BlendOp         /* = BlendOpSrcAlpha */
    DstAlpha        BlendOp         /* = BlendOpInvSrcAlpha */
    AlphaArithmetic BlendArithmetic /* = BlendArithmeticAdd */
    ColorMask       uint8           /* = ColorMaskAll */
}

type TessellationDescriptor struct {
    Partition        TessellationPartition /* = TessellationPartitionUndefined */
    MaxTessFactor    uint32                /* = 64 */
    OutputWindingCCW bool                  /* = false */
}

type QueryHeapDescriptor struct {
    DebugName       string    /* = "" */
    Type            QueryType /* = QueryTypeSamplesPassed */
    NumQueries      uint32    /* = 1 */
    RenderCondition bool      /* = false */
}

type FrameProfile struct {
    CommandQueueRecord  ProfileCommandQueueRecord
    CommandBufferRecord ProfileCommandBufferRecord
    TimeRecords         []ProfileTimeRecord        /* = nil */
}

type AttachmentFormatDescriptor struct {
    Format  Format            /* = FormatUndefined */
    LoadOp  AttachmentLoadOp  /* = AttachmentLoadOpUndefined */
    StoreOp AttachmentStoreOp /* = AttachmentStoreOpUndefined */
}

type RenderSystemDescriptor struct {
    ModuleName         string
    Flags              uint               /* = 0 */
    Profiler           unsafe.Pointer     /* = nil */
    Debugger           *RenderingDebugger /* = nil */
    RendererConfig     unsafe.Pointer     /* = nil */
    RendererConfigSize uintptr            /* = 0 */
    NativeHandle       unsafe.Pointer     /* = nil */
    NativeHandleSize   uintptr            /* = 0 */
}

type RenderingCapabilities struct {
    ScreenOrigin     ScreenOrigin      /* = ScreenOriginUpperLeft */
    ClippingRange    ClippingRange     /* = ClippingRangeZeroToOne */
    ShadingLanguages []ShadingLanguage /* = nil */
    TextureFormats   []Format          /* = nil */
    Features         RenderingFeatures
    Limits           RenderingLimits
}

type AttachmentDescriptor struct {
    Format     Format   /* = FormatUndefined */
    Texture    *Texture /* = nil */
    MipLevel   uint32   /* = 0 */
    ArrayLayer uint32   /* = 0 */
}

type SamplerDescriptor struct {
    DebugName      string             /* = "" */
    AddressModeU   SamplerAddressMode /* = SamplerAddressModeRepeat */
    AddressModeV   SamplerAddressMode /* = SamplerAddressModeRepeat */
    AddressModeW   SamplerAddressMode /* = SamplerAddressModeRepeat */
    MinFilter      SamplerFilter      /* = SamplerFilterLinear */
    MagFilter      SamplerFilter      /* = SamplerFilterLinear */
    MipMapFilter   SamplerFilter      /* = SamplerFilterLinear */
    MipMapEnabled  bool               /* = true */
    MipMapLODBias  float32            /* = 0.0 */
    MinLOD         float32            /* = 0.0 */
    MaxLOD         float32            /* = 1000.0 */
    MaxAnisotropy  uint32             /* = 1 */
    CompareEnabled bool               /* = false */
    CompareOp      CompareOp          /* = CompareOpLess */
    BorderColor    [4]float32         /* = {0.0,0.0,0.0,0.0} */
}

type ComputeShaderAttributes struct {
    WorkGroupSize Extent3D /* = {1,1,1} */
}

type SwapChainDescriptor struct {
    DebugName   string   /* = "" */
    Resolution  Extent2D
    ColorBits   int      /* = 32 */
    DepthBits   int      /* = 24 */
    StencilBits int      /* = 8 */
    Samples     uint32   /* = 1 */
    SwapBuffers uint32   /* = 2 */
    Fullscreen  bool     /* = false */
    Resizable   bool     /* = false */
}

type TextureSwizzleRGBA struct {
    R TextureSwizzle /* = TextureSwizzleRed */
    G TextureSwizzle /* = TextureSwizzleGreen */
    B TextureSwizzle /* = TextureSwizzleBlue */
    A TextureSwizzle /* = TextureSwizzleAlpha */
}

type TextureLocation struct {
    Offset     Offset3D
    ArrayLayer uint32   /* = 0 */
    MipLevel   uint32   /* = 0 */
}

type TextureRegion struct {
    Subresource TextureSubresource
    Offset      Offset3D
    Extent      Extent3D
}

type TextureDescriptor struct {
    DebugName      string      /* = "" */
    Type           TextureType /* = TextureTypeTexture2D */
    BindFlags      uint        /* = (BindSampled | BindColorAttachment) */
    CPUAccessFlags uint        /* = (CPUAccessRead | CPUAccessWrite) */
    MiscFlags      uint        /* = (MiscFixedSamples | MiscGenerateMips) */
    Format         Format      /* = FormatRGBA8UNorm */
    Extent         Extent3D    /* = {1,1,1} */
    ArrayLayers    uint32      /* = 1 */
    MipLevels      uint32      /* = 0 */
    Samples        uint32      /* = 1 */
    ClearValue     ClearValue
}

type VertexAttribute struct {
    Name            string
    Format          Format      /* = FormatRGBA32Float */
    Location        uint32      /* = 0 */
    SemanticIndex   uint32      /* = 0 */
    SystemValue     SystemValue /* = SystemValueUndefined */
    Slot            uint32      /* = 0 */
    Offset          uint32      /* = 0 */
    Stride          uint32      /* = 0 */
    InstanceDivisor uint32      /* = 0 */
}

type WindowDescriptor struct {
    Title             string
    Position          Offset2D
    Size              Extent2D
    Flags             uint           /* = 0 */
    WindowContext     unsafe.Pointer /* = nil */
    WindowContextSize uintptr        /* = 0 */
}

type BufferDescriptor struct {
    DebugName      string            /* = "" */
    Size           uint64            /* = 0 */
    Stride         uint32            /* = 0 */
    Format         Format            /* = FormatUndefined */
    BindFlags      uint              /* = 0 */
    CPUAccessFlags uint              /* = 0 */
    MiscFlags      uint              /* = 0 */
    VertexAttribs  []VertexAttribute /* = nil */
}

type StaticSamplerDescriptor struct {
    Name       string
    StageFlags uint              /* = 0 */
    Slot       BindingSlot
    Sampler    SamplerDescriptor
}

type StencilDescriptor struct {
    TestEnabled      bool                  /* = false */
    ReferenceDynamic bool                  /* = false */
    Front            StencilFaceDescriptor
    Back             StencilFaceDescriptor
}

type BlendDescriptor struct {
    AlphaToCoverageEnabled  bool                     /* = false */
    IndependentBlendEnabled bool                     /* = false */
    SampleMask              uint32                   /* = ~0u */
    LogicOp                 LogicOp                  /* = LogicOpDisabled */
    BlendFactor             [4]float32               /* = {0.0,0.0,0.0,0.0} */
    BlendFactorDynamic      bool                     /* = false */
    Targets                 [8]BlendTargetDescriptor
}

type RenderPassDescriptor struct {
    DebugName         string                        /* = "" */
    ColorAttachments  [8]AttachmentFormatDescriptor
    DepthAttachment   AttachmentFormatDescriptor
    StencilAttachment AttachmentFormatDescriptor
    Samples           uint32                        /* = 1 */
}

type RenderTargetDescriptor struct {
    DebugName              string                  /* = "" */
    RenderPass             *RenderPass             /* = nil */
    Resolution             Extent2D
    Samples                uint32                  /* = 1 */
    ColorAttachments       [8]AttachmentDescriptor
    ResolveAttachments     [8]AttachmentDescriptor
    DepthStencilAttachment AttachmentDescriptor
}

type VertexShaderAttributes struct {
    InputAttribs  []VertexAttribute /* = nil */
    OutputAttribs []VertexAttribute /* = nil */
}

type FragmentShaderAttributes struct {
    OutputAttribs []FragmentAttribute /* = nil */
}

type ShaderResourceReflection struct {
    Binding            BindingDescriptor
    ConstantBufferSize uint32            /* = 0 */
    StorageBufferType  StorageBufferType /* = StorageBufferTypeUndefined */
}

type TextureViewDescriptor struct {
    Type        TextureType        /* = TextureTypeTexture2D */
    Format      Format             /* = FormatRGBA8UNorm */
    Subresource TextureSubresource
    Swizzle     TextureSwizzleRGBA
}

type PipelineLayoutDescriptor struct {
    DebugName               string                             /* = "" */
    HeapBindings            []BindingDescriptor                /* = nil */
    Bindings                []BindingDescriptor                /* = nil */
    StaticSamplers          []StaticSamplerDescriptor          /* = nil */
    Uniforms                []UniformDescriptor                /* = nil */
    CombinedTextureSamplers []CombinedTextureSamplerDescriptor /* = nil */
    BarrierFlags            uint                               /* = 0 */
}

type GraphicsPipelineDescriptor struct {
    DebugName            string                 /* = "" */
    PipelineLayout       *PipelineLayout        /* = nil */
    RenderPass           *RenderPass            /* = nil */
    VertexShader         *Shader                /* = nil */
    TessControlShader    *Shader                /* = nil */
    TessEvaluationShader *Shader                /* = nil */
    GeometryShader       *Shader                /* = nil */
    FragmentShader       *Shader                /* = nil */
    IndexFormat          Format                 /* = FormatUndefined */
    PrimitiveTopology    PrimitiveTopology      /* = PrimitiveTopologyTriangleList */
    Viewports            []Viewport             /* = nil */
    Scissors             []Scissor              /* = nil */
    Depth                DepthDescriptor
    Stencil              StencilDescriptor
    Rasterizer           RasterizerDescriptor
    Blend                BlendDescriptor
    Tessellation         TessellationDescriptor
}

type ResourceViewDescriptor struct {
    Resource     *Resource             /* = nil */
    TextureView  TextureViewDescriptor
    BufferView   BufferViewDescriptor
    InitialCount uint32                /* = 0 */
}

type ShaderDescriptor struct {
    DebugName  string                   /* = "" */
    Type       ShaderType               /* = ShaderTypeUndefined */
    Source     string                   /* = "" */
    SourceSize uintptr                  /* = 0 */
    SourceType ShaderSourceType         /* = ShaderSourceTypeCodeFile */
    EntryPoint string                   /* = "" */
    Profile    string                   /* = "" */
    Defines    *ShaderMacro             /* = nil */
    Flags      uint                     /* = 0 */
    Name       string                   /* ShaderDescriptor.name is deprecated since 0.04b; Use ShaderDescriptor.debugName instead! */
    Vertex     VertexShaderAttributes
    Fragment   FragmentShaderAttributes
    Compute    ComputeShaderAttributes
}

type ShaderReflection struct {
    Resources []ShaderResourceReflection /* = nil */
    Uniforms  []UniformDescriptor        /* = nil */
    Vertex    VertexShaderAttributes
    Fragment  FragmentShaderAttributes
    Compute   ComputeShaderAttributes
}





/* ================================================================================ */

