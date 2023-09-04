/*
 * LLGLWrapper.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* AUTO GENERATED CODE - DO NOT EDIT */

#ifndef LLGL_C99_LLGLWRAPPER_H
#define LLGL_C99_LLGLWRAPPER_H


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <LLGL-C/Types.h>

#if defined LLGL_OS_ANDROID
#   include <android_native_app_glue.h>
#endif /* defined LLGL_OS_ANDROID */


/* ----- Constants ----- */

#define LLGL_RENDERERID_UNDEFINED   ( 0x00000000 )
#define LLGL_RENDERERID_NULL        ( 0x00000001 )
#define LLGL_RENDERERID_OPENGL      ( 0x00000002 )
#define LLGL_RENDERERID_OPENGLES1   ( 0x00000003 )
#define LLGL_RENDERERID_OPENGLES2   ( 0x00000004 )
#define LLGL_RENDERERID_OPENGLES3   ( 0x00000005 )
#define LLGL_RENDERERID_DIRECT3D9   ( 0x00000006 )
#define LLGL_RENDERERID_DIRECT3D10  ( 0x00000007 )
#define LLGL_RENDERERID_DIRECT3D11  ( 0x00000008 )
#define LLGL_RENDERERID_DIRECT3D12  ( 0x00000009 )
#define LLGL_RENDERERID_VULKAN      ( 0x0000000A )
#define LLGL_RENDERERID_METAL       ( 0x0000000B )
#define LLGL_RENDERERID_RESERVED    ( 0x000000FF )


/* ----- Enumerations ----- */

typedef enum LLGLRenderConditionMode
{
    LLGLRenderConditionModeWait,
    LLGLRenderConditionModeNoWait,
    LLGLRenderConditionModeByRegionWait,
    LLGLRenderConditionModeByRegionNoWait,
    LLGLRenderConditionModeWaitInverted,
    LLGLRenderConditionModeNoWaitInverted,
    LLGLRenderConditionModeByRegionWaitInverted,
    LLGLRenderConditionModeByRegionNoWaitInverted,
}
LLGLRenderConditionMode;

typedef enum LLGLStencilFace
{
    LLGLStencilFaceFrontAndBack,
    LLGLStencilFaceFront,
    LLGLStencilFaceBack,
}
LLGLStencilFace;

typedef enum LLGLFormat
{
    LLGLFormatUndefined,
    LLGLFormatA8UNorm,
    LLGLFormatR8UNorm,
    LLGLFormatR8SNorm,
    LLGLFormatR8UInt,
    LLGLFormatR8SInt,
    LLGLFormatR16UNorm,
    LLGLFormatR16SNorm,
    LLGLFormatR16UInt,
    LLGLFormatR16SInt,
    LLGLFormatR16Float,
    LLGLFormatR32UInt,
    LLGLFormatR32SInt,
    LLGLFormatR32Float,
    LLGLFormatR64Float,
    LLGLFormatRG8UNorm,
    LLGLFormatRG8SNorm,
    LLGLFormatRG8UInt,
    LLGLFormatRG8SInt,
    LLGLFormatRG16UNorm,
    LLGLFormatRG16SNorm,
    LLGLFormatRG16UInt,
    LLGLFormatRG16SInt,
    LLGLFormatRG16Float,
    LLGLFormatRG32UInt,
    LLGLFormatRG32SInt,
    LLGLFormatRG32Float,
    LLGLFormatRG64Float,
    LLGLFormatRGB8UNorm,
    LLGLFormatRGB8UNorm_sRGB,
    LLGLFormatRGB8SNorm,
    LLGLFormatRGB8UInt,
    LLGLFormatRGB8SInt,
    LLGLFormatRGB16UNorm,
    LLGLFormatRGB16SNorm,
    LLGLFormatRGB16UInt,
    LLGLFormatRGB16SInt,
    LLGLFormatRGB16Float,
    LLGLFormatRGB32UInt,
    LLGLFormatRGB32SInt,
    LLGLFormatRGB32Float,
    LLGLFormatRGB64Float,
    LLGLFormatRGBA8UNorm,
    LLGLFormatRGBA8UNorm_sRGB,
    LLGLFormatRGBA8SNorm,
    LLGLFormatRGBA8UInt,
    LLGLFormatRGBA8SInt,
    LLGLFormatRGBA16UNorm,
    LLGLFormatRGBA16SNorm,
    LLGLFormatRGBA16UInt,
    LLGLFormatRGBA16SInt,
    LLGLFormatRGBA16Float,
    LLGLFormatRGBA32UInt,
    LLGLFormatRGBA32SInt,
    LLGLFormatRGBA32Float,
    LLGLFormatRGBA64Float,
    LLGLFormatBGRA8UNorm,
    LLGLFormatBGRA8UNorm_sRGB,
    LLGLFormatBGRA8SNorm,
    LLGLFormatBGRA8UInt,
    LLGLFormatBGRA8SInt,
    LLGLFormatRGB10A2UNorm,
    LLGLFormatRGB10A2UInt,
    LLGLFormatRG11B10Float,
    LLGLFormatRGB9E5Float,
    LLGLFormatD16UNorm,
    LLGLFormatD24UNormS8UInt,
    LLGLFormatD32Float,
    LLGLFormatD32FloatS8X24UInt,
    LLGLFormatBC1UNorm,
    LLGLFormatBC1UNorm_sRGB,
    LLGLFormatBC2UNorm,
    LLGLFormatBC2UNorm_sRGB,
    LLGLFormatBC3UNorm,
    LLGLFormatBC3UNorm_sRGB,
    LLGLFormatBC4UNorm,
    LLGLFormatBC4SNorm,
    LLGLFormatBC5UNorm,
    LLGLFormatBC5SNorm,
}
LLGLFormat;

typedef enum LLGLImageFormat
{
    LLGLImageFormatAlpha,
    LLGLImageFormatR,
    LLGLImageFormatRG,
    LLGLImageFormatRGB,
    LLGLImageFormatBGR,
    LLGLImageFormatRGBA,
    LLGLImageFormatBGRA,
    LLGLImageFormatARGB,
    LLGLImageFormatABGR,
    LLGLImageFormatDepth,
    LLGLImageFormatDepthStencil,
    LLGLImageFormatStencil,
    LLGLImageFormatBC1,
    LLGLImageFormatBC2,
    LLGLImageFormatBC3,
    LLGLImageFormatBC4,
    LLGLImageFormatBC5,
}
LLGLImageFormat;

typedef enum LLGLDataType
{
    LLGLDataTypeUndefined,
    LLGLDataTypeInt8,
    LLGLDataTypeUInt8,
    LLGLDataTypeInt16,
    LLGLDataTypeUInt16,
    LLGLDataTypeInt32,
    LLGLDataTypeUInt32,
    LLGLDataTypeFloat16,
    LLGLDataTypeFloat32,
    LLGLDataTypeFloat64,
}
LLGLDataType;

typedef enum LLGLKey
{
    LLGLKeyLButton,
    LLGLKeyRButton,
    LLGLKeyCancel,
    LLGLKeyMButton,
    LLGLKeyXButton1,
    LLGLKeyXButton2,
    LLGLKeyBack,
    LLGLKeyTab,
    LLGLKeyClear,
    LLGLKeyReturn,
    LLGLKeyShift,
    LLGLKeyControl,
    LLGLKeyMenu,
    LLGLKeyPause,
    LLGLKeyCapital,
    LLGLKeyEscape,
    LLGLKeySpace,
    LLGLKeyPageUp,
    LLGLKeyPageDown,
    LLGLKeyEnd,
    LLGLKeyHome,
    LLGLKeyLeft,
    LLGLKeyUp,
    LLGLKeyRight,
    LLGLKeyDown,
    LLGLKeySelect,
    LLGLKeyPrint,
    LLGLKeyExe,
    LLGLKeySnapshot,
    LLGLKeyInsert,
    LLGLKeyDelete,
    LLGLKeyHelp,
    LLGLKeyD0,
    LLGLKeyD1,
    LLGLKeyD2,
    LLGLKeyD3,
    LLGLKeyD4,
    LLGLKeyD5,
    LLGLKeyD6,
    LLGLKeyD7,
    LLGLKeyD8,
    LLGLKeyD9,
    LLGLKeyA,
    LLGLKeyB,
    LLGLKeyC,
    LLGLKeyD,
    LLGLKeyE,
    LLGLKeyF,
    LLGLKeyG,
    LLGLKeyH,
    LLGLKeyI,
    LLGLKeyJ,
    LLGLKeyK,
    LLGLKeyL,
    LLGLKeyM,
    LLGLKeyN,
    LLGLKeyO,
    LLGLKeyP,
    LLGLKeyQ,
    LLGLKeyR,
    LLGLKeyS,
    LLGLKeyT,
    LLGLKeyU,
    LLGLKeyV,
    LLGLKeyW,
    LLGLKeyX,
    LLGLKeyY,
    LLGLKeyZ,
    LLGLKeyLWin,
    LLGLKeyRWin,
    LLGLKeyApps,
    LLGLKeySleep,
    LLGLKeyKeypad0,
    LLGLKeyKeypad1,
    LLGLKeyKeypad2,
    LLGLKeyKeypad3,
    LLGLKeyKeypad4,
    LLGLKeyKeypad5,
    LLGLKeyKeypad6,
    LLGLKeyKeypad7,
    LLGLKeyKeypad8,
    LLGLKeyKeypad9,
    LLGLKeyKeypadMultiply,
    LLGLKeyKeypadPlus,
    LLGLKeyKeypadSeparator,
    LLGLKeyKeypadMinus,
    LLGLKeyKeypadDecimal,
    LLGLKeyKeypadDivide,
    LLGLKeyF1,
    LLGLKeyF2,
    LLGLKeyF3,
    LLGLKeyF4,
    LLGLKeyF5,
    LLGLKeyF6,
    LLGLKeyF7,
    LLGLKeyF8,
    LLGLKeyF9,
    LLGLKeyF10,
    LLGLKeyF11,
    LLGLKeyF12,
    LLGLKeyF13,
    LLGLKeyF14,
    LLGLKeyF15,
    LLGLKeyF16,
    LLGLKeyF17,
    LLGLKeyF18,
    LLGLKeyF19,
    LLGLKeyF20,
    LLGLKeyF21,
    LLGLKeyF22,
    LLGLKeyF23,
    LLGLKeyF24,
    LLGLKeyNumLock,
    LLGLKeyScrollLock,
    LLGLKeyLShift,
    LLGLKeyRShift,
    LLGLKeyLControl,
    LLGLKeyRControl,
    LLGLKeyLMenu,
    LLGLKeyRMenu,
    LLGLKeyBrowserBack,
    LLGLKeyBrowserForward,
    LLGLKeyBrowserRefresh,
    LLGLKeyBrowserStop,
    LLGLKeyBrowserSearch,
    LLGLKeyBrowserFavorits,
    LLGLKeyBrowserHome,
    LLGLKeyVolumeMute,
    LLGLKeyVolumeDown,
    LLGLKeyVolumeUp,
    LLGLKeyMediaNextTrack,
    LLGLKeyMediaPrevTrack,
    LLGLKeyMediaStop,
    LLGLKeyMediaPlayPause,
    LLGLKeyLaunchMail,
    LLGLKeyLaunchMediaSelect,
    LLGLKeyLaunchApp1,
    LLGLKeyLaunchApp2,
    LLGLKeyPlus,
    LLGLKeyComma,
    LLGLKeyMinus,
    LLGLKeyPeriod,
    LLGLKeyExponent,
    LLGLKeyAttn,
    LLGLKeyCrSel,
    LLGLKeyExSel,
    LLGLKeyErEOF,
    LLGLKeyPlay,
    LLGLKeyZoom,
    LLGLKeyNoName,
    LLGLKeyPA1,
    LLGLKeyOEMClear,
    LLGLKeyAny,
}
LLGLKey;

typedef enum LLGLUniformType
{
    LLGLUniformTypeUndefined,
    LLGLUniformTypeFloat1,
    LLGLUniformTypeFloat2,
    LLGLUniformTypeFloat3,
    LLGLUniformTypeFloat4,
    LLGLUniformTypeDouble1,
    LLGLUniformTypeDouble2,
    LLGLUniformTypeDouble3,
    LLGLUniformTypeDouble4,
    LLGLUniformTypeInt1,
    LLGLUniformTypeInt2,
    LLGLUniformTypeInt3,
    LLGLUniformTypeInt4,
    LLGLUniformTypeUInt1,
    LLGLUniformTypeUInt2,
    LLGLUniformTypeUInt3,
    LLGLUniformTypeUInt4,
    LLGLUniformTypeBool1,
    LLGLUniformTypeBool2,
    LLGLUniformTypeBool3,
    LLGLUniformTypeBool4,
    LLGLUniformTypeFloat2x2,
    LLGLUniformTypeFloat2x3,
    LLGLUniformTypeFloat2x4,
    LLGLUniformTypeFloat3x2,
    LLGLUniformTypeFloat3x3,
    LLGLUniformTypeFloat3x4,
    LLGLUniformTypeFloat4x2,
    LLGLUniformTypeFloat4x3,
    LLGLUniformTypeFloat4x4,
    LLGLUniformTypeDouble2x2,
    LLGLUniformTypeDouble2x3,
    LLGLUniformTypeDouble2x4,
    LLGLUniformTypeDouble3x2,
    LLGLUniformTypeDouble3x3,
    LLGLUniformTypeDouble3x4,
    LLGLUniformTypeDouble4x2,
    LLGLUniformTypeDouble4x3,
    LLGLUniformTypeDouble4x4,
    LLGLUniformTypeSampler,
    LLGLUniformTypeImage,
    LLGLUniformTypeAtomicCounter,
}
LLGLUniformType;

typedef enum LLGLPrimitiveTopology
{
    LLGLPrimitiveTopologyPointList,
    LLGLPrimitiveTopologyLineList,
    LLGLPrimitiveTopologyLineStrip,
    LLGLPrimitiveTopologyLineListAdjacency,
    LLGLPrimitiveTopologyLineStripAdjacency,
    LLGLPrimitiveTopologyTriangleList,
    LLGLPrimitiveTopologyTriangleStrip,
    LLGLPrimitiveTopologyTriangleListAdjacency,
    LLGLPrimitiveTopologyTriangleStripAdjacency,
    LLGLPrimitiveTopologyPatches1,
    LLGLPrimitiveTopologyPatches2,
    LLGLPrimitiveTopologyPatches3,
    LLGLPrimitiveTopologyPatches4,
    LLGLPrimitiveTopologyPatches5,
    LLGLPrimitiveTopologyPatches6,
    LLGLPrimitiveTopologyPatches7,
    LLGLPrimitiveTopologyPatches8,
    LLGLPrimitiveTopologyPatches9,
    LLGLPrimitiveTopologyPatches10,
    LLGLPrimitiveTopologyPatches11,
    LLGLPrimitiveTopologyPatches12,
    LLGLPrimitiveTopologyPatches13,
    LLGLPrimitiveTopologyPatches14,
    LLGLPrimitiveTopologyPatches15,
    LLGLPrimitiveTopologyPatches16,
    LLGLPrimitiveTopologyPatches17,
    LLGLPrimitiveTopologyPatches18,
    LLGLPrimitiveTopologyPatches19,
    LLGLPrimitiveTopologyPatches20,
    LLGLPrimitiveTopologyPatches21,
    LLGLPrimitiveTopologyPatches22,
    LLGLPrimitiveTopologyPatches23,
    LLGLPrimitiveTopologyPatches24,
    LLGLPrimitiveTopologyPatches25,
    LLGLPrimitiveTopologyPatches26,
    LLGLPrimitiveTopologyPatches27,
    LLGLPrimitiveTopologyPatches28,
    LLGLPrimitiveTopologyPatches29,
    LLGLPrimitiveTopologyPatches30,
    LLGLPrimitiveTopologyPatches31,
    LLGLPrimitiveTopologyPatches32,
}
LLGLPrimitiveTopology;

typedef enum LLGLCompareOp
{
    LLGLCompareOpNeverPass,
    LLGLCompareOpLess,
    LLGLCompareOpEqual,
    LLGLCompareOpLessEqual,
    LLGLCompareOpGreater,
    LLGLCompareOpNotEqual,
    LLGLCompareOpGreaterEqual,
    LLGLCompareOpAlwaysPass,
}
LLGLCompareOp;

typedef enum LLGLStencilOp
{
    LLGLStencilOpKeep,
    LLGLStencilOpZero,
    LLGLStencilOpReplace,
    LLGLStencilOpIncClamp,
    LLGLStencilOpDecClamp,
    LLGLStencilOpInvert,
    LLGLStencilOpIncWrap,
    LLGLStencilOpDecWrap,
}
LLGLStencilOp;

typedef enum LLGLBlendOp
{
    LLGLBlendOpZero,
    LLGLBlendOpOne,
    LLGLBlendOpSrcColor,
    LLGLBlendOpInvSrcColor,
    LLGLBlendOpSrcAlpha,
    LLGLBlendOpInvSrcAlpha,
    LLGLBlendOpDstColor,
    LLGLBlendOpInvDstColor,
    LLGLBlendOpDstAlpha,
    LLGLBlendOpInvDstAlpha,
    LLGLBlendOpSrcAlphaSaturate,
    LLGLBlendOpBlendFactor,
    LLGLBlendOpInvBlendFactor,
    LLGLBlendOpSrc1Color,
    LLGLBlendOpInvSrc1Color,
    LLGLBlendOpSrc1Alpha,
    LLGLBlendOpInvSrc1Alpha,
}
LLGLBlendOp;

typedef enum LLGLBlendArithmetic
{
    LLGLBlendArithmeticAdd,
    LLGLBlendArithmeticSubtract,
    LLGLBlendArithmeticRevSubtract,
    LLGLBlendArithmeticMin,
    LLGLBlendArithmeticMax,
}
LLGLBlendArithmetic;

typedef enum LLGLPolygonMode
{
    LLGLPolygonModeFill,
    LLGLPolygonModeWireframe,
    LLGLPolygonModePoints,
}
LLGLPolygonMode;

typedef enum LLGLCullMode
{
    LLGLCullModeDisabled,
    LLGLCullModeFront,
    LLGLCullModeBack,
}
LLGLCullMode;

typedef enum LLGLLogicOp
{
    LLGLLogicOpDisabled,
    LLGLLogicOpClear,
    LLGLLogicOpSet,
    LLGLLogicOpCopy,
    LLGLLogicOpCopyInverted,
    LLGLLogicOpNoOp,
    LLGLLogicOpInvert,
    LLGLLogicOpAND,
    LLGLLogicOpANDReverse,
    LLGLLogicOpANDInverted,
    LLGLLogicOpNAND,
    LLGLLogicOpOR,
    LLGLLogicOpORReverse,
    LLGLLogicOpORInverted,
    LLGLLogicOpNOR,
    LLGLLogicOpXOR,
    LLGLLogicOpEquiv,
}
LLGLLogicOp;

typedef enum LLGLTessellationPartition
{
    LLGLTessellationPartitionUndefined,
    LLGLTessellationPartitionInteger,
    LLGLTessellationPartitionPow2,
    LLGLTessellationPartitionFractionalOdd,
    LLGLTessellationPartitionFractionalEven,
}
LLGLTessellationPartition;

typedef enum LLGLQueryType
{
    LLGLQueryTypeSamplesPassed,
    LLGLQueryTypeAnySamplesPassed,
    LLGLQueryTypeAnySamplesPassedConservative,
    LLGLQueryTypeTimeElapsed,
    LLGLQueryTypeStreamOutPrimitivesWritten,
    LLGLQueryTypeStreamOutOverflow,
    LLGLQueryTypePipelineStatistics,
}
LLGLQueryType;

typedef enum LLGLAttachmentLoadOp
{
    LLGLAttachmentLoadOpUndefined,
    LLGLAttachmentLoadOpLoad,
    LLGLAttachmentLoadOpClear,
}
LLGLAttachmentLoadOp;

typedef enum LLGLAttachmentStoreOp
{
    LLGLAttachmentStoreOpUndefined,
    LLGLAttachmentStoreOpStore,
}
LLGLAttachmentStoreOp;

typedef enum LLGLShadingLanguage
{
    LLGLShadingLanguageGLSL           = (0x10000),
    LLGLShadingLanguageGLSL_110       = (0x10000|110),
    LLGLShadingLanguageGLSL_120       = (0x10000|120),
    LLGLShadingLanguageGLSL_130       = (0x10000|130),
    LLGLShadingLanguageGLSL_140       = (0x10000|140),
    LLGLShadingLanguageGLSL_150       = (0x10000|150),
    LLGLShadingLanguageGLSL_330       = (0x10000|330),
    LLGLShadingLanguageGLSL_400       = (0x10000|400),
    LLGLShadingLanguageGLSL_410       = (0x10000|410),
    LLGLShadingLanguageGLSL_420       = (0x10000|420),
    LLGLShadingLanguageGLSL_430       = (0x10000|430),
    LLGLShadingLanguageGLSL_440       = (0x10000|440),
    LLGLShadingLanguageGLSL_450       = (0x10000|450),
    LLGLShadingLanguageGLSL_460       = (0x10000|460),
    LLGLShadingLanguageESSL           = (0x20000),
    LLGLShadingLanguageESSL_100       = (0x20000|100),
    LLGLShadingLanguageESSL_300       = (0x20000|300),
    LLGLShadingLanguageESSL_310       = (0x20000|310),
    LLGLShadingLanguageESSL_320       = (0x20000|320),
    LLGLShadingLanguageHLSL           = (0x30000),
    LLGLShadingLanguageHLSL_2_0       = (0x30000|200),
    LLGLShadingLanguageHLSL_2_0a      = (0x30000|201),
    LLGLShadingLanguageHLSL_2_0b      = (0x30000|202),
    LLGLShadingLanguageHLSL_3_0       = (0x30000|300),
    LLGLShadingLanguageHLSL_4_0       = (0x30000|400),
    LLGLShadingLanguageHLSL_4_1       = (0x30000|410),
    LLGLShadingLanguageHLSL_5_0       = (0x30000|500),
    LLGLShadingLanguageHLSL_5_1       = (0x30000|510),
    LLGLShadingLanguageHLSL_6_0       = (0x30000|600),
    LLGLShadingLanguageHLSL_6_1       = (0x30000|601),
    LLGLShadingLanguageHLSL_6_2       = (0x30000|602),
    LLGLShadingLanguageHLSL_6_3       = (0x30000|603),
    LLGLShadingLanguageHLSL_6_4       = (0x30000|604),
    LLGLShadingLanguageMetal          = (0x40000),
    LLGLShadingLanguageMetal_1_0      = (0x40000|100),
    LLGLShadingLanguageMetal_1_1      = (0x40000|110),
    LLGLShadingLanguageMetal_1_2      = (0x40000|120),
    LLGLShadingLanguageMetal_2_0      = (0x40000|200),
    LLGLShadingLanguageMetal_2_1      = (0x40000|210),
    LLGLShadingLanguageSPIRV          = (0x50000),
    LLGLShadingLanguageSPIRV_100      = (0x50000|100),
    LLGLShadingLanguageVersionBitmask = 0x0000ffff,
}
LLGLShadingLanguage;

typedef enum LLGLScreenOrigin
{
    LLGLScreenOriginLowerLeft,
    LLGLScreenOriginUpperLeft,
}
LLGLScreenOrigin;

typedef enum LLGLClippingRange
{
    LLGLClippingRangeMinusOneToOne,
    LLGLClippingRangeZeroToOne,
}
LLGLClippingRange;

typedef enum LLGLCPUAccess
{
    LLGLCPUAccessReadOnly,
    LLGLCPUAccessWriteOnly,
    LLGLCPUAccessWriteDiscard,
    LLGLCPUAccessReadWrite,
}
LLGLCPUAccess;

typedef enum LLGLResourceType
{
    LLGLResourceTypeUndefined,
    LLGLResourceTypeBuffer,
    LLGLResourceTypeTexture,
    LLGLResourceTypeSampler,
}
LLGLResourceType;

typedef enum LLGLSamplerAddressMode
{
    LLGLSamplerAddressModeRepeat,
    LLGLSamplerAddressModeMirror,
    LLGLSamplerAddressModeClamp,
    LLGLSamplerAddressModeBorder,
    LLGLSamplerAddressModeMirrorOnce,
}
LLGLSamplerAddressMode;

typedef enum LLGLSamplerFilter
{
    LLGLSamplerFilterNearest,
    LLGLSamplerFilterLinear,
}
LLGLSamplerFilter;

typedef enum LLGLShaderType
{
    LLGLShaderTypeUndefined,
    LLGLShaderTypeVertex,
    LLGLShaderTypeTessControl,
    LLGLShaderTypeTessEvaluation,
    LLGLShaderTypeGeometry,
    LLGLShaderTypeFragment,
    LLGLShaderTypeCompute,
}
LLGLShaderType;

typedef enum LLGLShaderSourceType
{
    LLGLShaderSourceTypeCodeString,
    LLGLShaderSourceTypeCodeFile,
    LLGLShaderSourceTypeBinaryBuffer,
    LLGLShaderSourceTypeBinaryFile,
}
LLGLShaderSourceType;

typedef enum LLGLStorageBufferType
{
    LLGLStorageBufferTypeUndefined,
    LLGLStorageBufferTypeTypedBuffer,
    LLGLStorageBufferTypeStructuredBuffer,
    LLGLStorageBufferTypeByteAddressBuffer,
    LLGLStorageBufferTypeRWTypedBuffer,
    LLGLStorageBufferTypeRWStructuredBuffer,
    LLGLStorageBufferTypeRWByteAddressBuffer,
    LLGLStorageBufferTypeAppendStructuredBuffer,
    LLGLStorageBufferTypeConsumeStructuredBuffer,
}
LLGLStorageBufferType;

typedef enum LLGLSystemValue
{
    LLGLSystemValueUndefined,
    LLGLSystemValueClipDistance,
    LLGLSystemValueColor,
    LLGLSystemValueCullDistance,
    LLGLSystemValueDepth,
    LLGLSystemValueDepthGreater,
    LLGLSystemValueDepthLess,
    LLGLSystemValueFrontFacing,
    LLGLSystemValueInstanceID,
    LLGLSystemValuePosition,
    LLGLSystemValuePrimitiveID,
    LLGLSystemValueRenderTargetIndex,
    LLGLSystemValueSampleMask,
    LLGLSystemValueSampleID,
    LLGLSystemValueStencil,
    LLGLSystemValueVertexID,
    LLGLSystemValueViewportIndex,
}
LLGLSystemValue;

typedef enum LLGLTextureType
{
    LLGLTextureTypeTexture1D,
    LLGLTextureTypeTexture2D,
    LLGLTextureTypeTexture3D,
    LLGLTextureTypeTextureCube,
    LLGLTextureTypeTexture1DArray,
    LLGLTextureTypeTexture2DArray,
    LLGLTextureTypeTextureCubeArray,
    LLGLTextureTypeTexture2DMS,
    LLGLTextureTypeTexture2DMSArray,
}
LLGLTextureType;

typedef enum LLGLTextureSwizzle
{
    LLGLTextureSwizzleZero,
    LLGLTextureSwizzleOne,
    LLGLTextureSwizzleRed,
    LLGLTextureSwizzleGreen,
    LLGLTextureSwizzleBlue,
    LLGLTextureSwizzleAlpha,
}
LLGLTextureSwizzle;


/* ----- Flags ----- */

typedef enum LLGLCanvasFlags
{
    LLGLCanvasBorderless = (1 << 0),
}
LLGLCanvasFlags;

typedef enum LLGLCommandBufferFlags
{
    LLGLCommandBufferSecondary       = (1 << 0),
    LLGLCommandBufferMultiSubmit     = (1 << 1),
    LLGLCommandBufferImmediateSubmit = (1 << 2),
}
LLGLCommandBufferFlags;

typedef enum LLGLClearFlags
{
    LLGLClearColor        = (1 << 0),
    LLGLClearDepth        = (1 << 1),
    LLGLClearStencil      = (1 << 2),
    LLGLClearColorDepth   = (LLGLClearColor | LLGLClearDepth),
    LLGLClearDepthStencil = (LLGLClearDepth | LLGLClearStencil),
    LLGLClearAll          = (LLGLClearColor | LLGLClearDepth | LLGLClearStencil),
}
LLGLClearFlags;

typedef enum LLGLFormatFlags
{
    LLGLFormatHasDepth             = (1 << 0),
    LLGLFormatHasStencil           = (1 << 1),
    LLGLFormatIsColorSpace_sRGB    = (1 << 2),
    LLGLFormatIsCompressed         = (1 << 3),
    LLGLFormatIsNormalized         = (1 << 4),
    LLGLFormatIsInteger            = (1 << 5),
    LLGLFormatIsUnsigned           = (1 << 6),
    LLGLFormatIsPacked             = (1 << 7),
    LLGLFormatSupportsRenderTarget = (1 << 8),
    LLGLFormatSupportsMips         = (1 << 9),
    LLGLFormatSupportsGenerateMips = (1 << 10),
    LLGLFormatSupportsTexture1D    = (1 << 11),
    LLGLFormatSupportsTexture2D    = (1 << 12),
    LLGLFormatSupportsTexture3D    = (1 << 13),
    LLGLFormatSupportsTextureCube  = (1 << 14),
    LLGLFormatSupportsVertex       = (1 << 15),
    LLGLFormatIsUnsignedInteger    = (LLGLFormatIsUnsigned | LLGLFormatIsInteger),
    LLGLFormatHasDepthStencil      = (LLGLFormatHasDepth | LLGLFormatHasStencil),
}
LLGLFormatFlags;

typedef enum LLGLColorMaskFlags
{
    LLGLColorMaskZero = 0,
    LLGLColorMaskR    = (1 << 0),
    LLGLColorMaskG    = (1 << 1),
    LLGLColorMaskB    = (1 << 2),
    LLGLColorMaskA    = (1 << 3),
    LLGLColorMaskAll  = (LLGLColorMaskR | LLGLColorMaskG | LLGLColorMaskB | LLGLColorMaskA),
}
LLGLColorMaskFlags;

typedef enum LLGLRenderSystemFlags
{
    LLGLRenderSystemDebugDevice = (1 << 0),
}
LLGLRenderSystemFlags;

typedef enum LLGLBindFlags
{
    LLGLBindVertexBuffer           = (1 << 0),
    LLGLBindIndexBuffer            = (1 << 1),
    LLGLBindConstantBuffer         = (1 << 2),
    LLGLBindStreamOutputBuffer     = (1 << 3),
    LLGLBindIndirectBuffer         = (1 << 4),
    LLGLBindSampled                = (1 << 5),
    LLGLBindStorage                = (1 << 6),
    LLGLBindColorAttachment        = (1 << 7),
    LLGLBindDepthStencilAttachment = (1 << 8),
    LLGLBindCombinedSampler        = (1 << 9),
    LLGLBindCopySrc                = (1 << 10),
    LLGLBindCopyDst                = (1 << 11),
}
LLGLBindFlags;

typedef enum LLGLCPUAccessFlags
{
    LLGLCPUAccessRead  = (1 << 0),
    LLGLCPUAccessWrite = (1 << 1),
}
LLGLCPUAccessFlags;

typedef enum LLGLMiscFlags
{
    LLGLMiscDynamicUsage  = (1 << 0),
    LLGLMiscFixedSamples  = (1 << 1),
    LLGLMiscGenerateMips  = (1 << 2),
    LLGLMiscNoInitialData = (1 << 3),
    LLGLMiscAppend        = (1 << 4),
    LLGLMiscCounter       = (1 << 5),
}
LLGLMiscFlags;

typedef enum LLGLBarrierFlags
{
    LLGLBarrierStorageBuffer  = (1 << 0),
    LLGLBarrierStorageTexture = (1 << 1),
    LLGLBarrierStorage        = (LLGLBarrierStorageBuffer | LLGLBarrierStorageTexture),
}
LLGLBarrierFlags;

typedef enum LLGLShaderCompileFlags
{
    LLGLShaderCompileDebug               = (1 << 0),
    LLGLShaderCompileNoOptimization      = (1 << 1),
    LLGLShaderCompileOptimizationLevel1  = (1 << 2),
    LLGLShaderCompileOptimizationLevel2  = (1 << 3),
    LLGLShaderCompileOptimizationLevel3  = (1 << 4),
    LLGLShaderCompileWarningsAreErrors   = (1 << 5),
    LLGLShaderCompilePatchClippingOrigin = (1 << 6),
    LLGLShaderCompileSeparateShader      = (1 << 7),
    LLGLShaderCompileDefaultLibrary      = (1 << 8),
}
LLGLShaderCompileFlags;

typedef enum LLGLStageFlags
{
    LLGLStageVertexStage         = (1 << 0),
    LLGLStageTessControlStage    = (1 << 1),
    LLGLStageTessEvaluationStage = (1 << 2),
    LLGLStageGeometryStage       = (1 << 3),
    LLGLStageFragmentStage       = (1 << 4),
    LLGLStageComputeStage        = (1 << 5),
    LLGLStageAllTessStages       = (LLGLStageTessControlStage | LLGLStageTessEvaluationStage),
    LLGLStageAllGraphicsStages   = (LLGLStageVertexStage | LLGLStageAllTessStages | LLGLStageGeometryStage | LLGLStageFragmentStage),
    LLGLStageAllStages           = (LLGLStageAllGraphicsStages | LLGLStageComputeStage),
}
LLGLStageFlags;

typedef enum LLGLResizeBuffersFlags
{
    LLGLResizeBuffersAdaptSurface   = (1 << 0),
    LLGLResizeBuffersFullscreenMode = (1 << 1),
    LLGLResizeBuffersWindowedMode   = (1 << 2),
}
LLGLResizeBuffersFlags;

typedef enum LLGLWindowFlags
{
    LLGLWindowVisible              = (1 << 0),
    LLGLWindowBorderless           = (1 << 1),
    LLGLWindowResizable            = (1 << 2),
    LLGLWindowCentered             = (1 << 3),
    LLGLWindowAcceptDropFiles      = (1 << 4),
    LLGLWindowDisableClearOnResize = (1 << 5),
}
LLGLWindowFlags;


/* ----- Structures ----- */

typedef struct LLGLCanvasDescriptor
{
    const char* title;
    long        flags; /* = 0 */
}
LLGLCanvasDescriptor;

typedef struct LLGLClearValue
{
    float    color[4]; /* = {0.0f,0.0f,0.0f,0.0f} */
    float    depth;    /* = 1.0f */
    uint32_t stencil;  /* = 0 */
}
LLGLClearValue;

typedef struct LLGLCommandBufferDescriptor
{
    long     flags;              /* = 0 */
    uint32_t numNativeBuffers;   /* = 2 */
    uint64_t minStagingPoolSize; /* = (0xFFFF+1) */
}
LLGLCommandBufferDescriptor;

typedef struct LLGLDrawIndirectArguments
{
    uint32_t numVertices;
    uint32_t numInstances;
    uint32_t firstVertex;
    uint32_t firstInstance;
}
LLGLDrawIndirectArguments;

typedef struct LLGLDrawIndexedIndirectArguments
{
    uint32_t numIndices;
    uint32_t numInstances;
    uint32_t firstIndex;
    int32_t  vertexOffset;
    uint32_t firstInstance;
}
LLGLDrawIndexedIndirectArguments;

typedef struct LLGLDrawPatchIndirectArguments
{
    uint32_t numPatches;
    uint32_t numInstances;
    uint32_t firstPatch;
    uint32_t firstInstance;
}
LLGLDrawPatchIndirectArguments;

typedef struct LLGLDispatchIndirectArguments
{
    uint32_t numThreadGroups[3];
}
LLGLDispatchIndirectArguments;

typedef struct LLGLBindingSlot
{
    uint32_t index; /* = 0 */
    uint32_t set;   /* = 0 */
}
LLGLBindingSlot;

typedef struct LLGLViewport
{
    float x;        /* = 0.0f */
    float y;        /* = 0.0f */
    float width;    /* = 0.0f */
    float height;   /* = 0.0f */
    float minDepth; /* = 0.0f */
    float maxDepth; /* = 1.0f */
}
LLGLViewport;

typedef struct LLGLScissor
{
    int32_t x;      /* = 0 */
    int32_t y;      /* = 0 */
    int32_t width;  /* = 0 */
    int32_t height; /* = 0 */
}
LLGLScissor;

typedef struct LLGLDepthBiasDescriptor
{
    float constantFactor; /* = 0.0f */
    float slopeFactor;    /* = 0.0f */
    float clamp;          /* = 0.0f */
}
LLGLDepthBiasDescriptor;

typedef struct LLGLComputePipelineDescriptor
{
    LLGLPipelineLayout pipelineLayout; /* = LLGL_NULL_OBJECT */
    LLGLShader         computeShader;  /* = LLGL_NULL_OBJECT */
}
LLGLComputePipelineDescriptor;

typedef struct LLGLQueryPipelineStatistics
{
    uint64_t inputAssemblyVertices;           /* = 0 */
    uint64_t inputAssemblyPrimitives;         /* = 0 */
    uint64_t vertexShaderInvocations;         /* = 0 */
    uint64_t geometryShaderInvocations;       /* = 0 */
    uint64_t geometryShaderPrimitives;        /* = 0 */
    uint64_t clippingInvocations;             /* = 0 */
    uint64_t clippingPrimitives;              /* = 0 */
    uint64_t fragmentShaderInvocations;       /* = 0 */
    uint64_t tessControlShaderInvocations;    /* = 0 */
    uint64_t tessEvaluationShaderInvocations; /* = 0 */
    uint64_t computeShaderInvocations;        /* = 0 */
}
LLGLQueryPipelineStatistics;

typedef struct LLGLRendererInfo
{
    const char*        rendererName;
    const char*        deviceName;
    const char*        vendorName;
    const char*        shadingLanguageName;
    size_t             numExtensionNames;   /* = 0 */
    const char* const* extensionNames;      /* = NULL */
}
LLGLRendererInfo;

typedef struct LLGLRenderingFeatures
{
    bool hasRenderTargets;             /* = false */
    bool has3DTextures;                /* = false */
    bool hasCubeTextures;              /* = false */
    bool hasArrayTextures;             /* = false */
    bool hasCubeArrayTextures;         /* = false */
    bool hasMultiSampleTextures;       /* = false */
    bool hasMultiSampleArrayTextures;  /* = false */
    bool hasTextureViews;              /* = false */
    bool hasTextureViewSwizzle;        /* = false */
    bool hasBufferViews;               /* = false */
    bool hasSamplers;                  /* = false */
    bool hasConstantBuffers;           /* = false */
    bool hasStorageBuffers;            /* = false */
    bool hasUniforms;                  /* = false */
    bool hasGeometryShaders;           /* = false */
    bool hasTessellationShaders;       /* = false */
    bool hasTessellatorStage;          /* = false */
    bool hasComputeShaders;            /* = false */
    bool hasInstancing;                /* = false */
    bool hasOffsetInstancing;          /* = false */
    bool hasIndirectDrawing;           /* = false */
    bool hasViewportArrays;            /* = false */
    bool hasConservativeRasterization; /* = false */
    bool hasStreamOutputs;             /* = false */
    bool hasLogicOp;                   /* = false */
    bool hasPipelineStatistics;        /* = false */
    bool hasRenderCondition;           /* = false */
}
LLGLRenderingFeatures;

typedef struct LLGLRenderingLimits
{
    float    lineWidthRange[2];                /* = {1.0f,1.0f} */
    uint32_t maxTextureArrayLayers;            /* = 0 */
    uint32_t maxColorAttachments;              /* = 0 */
    uint32_t maxPatchVertices;                 /* = 0 */
    uint32_t max1DTextureSize;                 /* = 0 */
    uint32_t max2DTextureSize;                 /* = 0 */
    uint32_t max3DTextureSize;                 /* = 0 */
    uint32_t maxCubeTextureSize;               /* = 0 */
    uint32_t maxAnisotropy;                    /* = 0 */
    uint32_t maxComputeShaderWorkGroups[3];    /* = {0,0,0} */
    uint32_t maxComputeShaderWorkGroupSize[3]; /* = {0,0,0} */
    uint32_t maxViewports;                     /* = 0 */
    uint32_t maxViewportSize[2];               /* = {0,0} */
    uint64_t maxBufferSize;                    /* = 0 */
    uint64_t maxConstantBufferSize;            /* = 0 */
    uint32_t maxStreamOutputs;                 /* = 0 */
    uint32_t maxTessFactor;                    /* = 0 */
    uint64_t minConstantBufferAlignment;       /* = 0 */
    uint64_t minSampledBufferAlignment;        /* = 0 */
    uint64_t minStorageBufferAlignment;        /* = 0 */
    uint32_t maxColorBufferSamples;            /* = 0 */
    uint32_t maxDepthBufferSamples;            /* = 0 */
    uint32_t maxStencilBufferSamples;          /* = 0 */
    uint32_t maxNoAttachmentSamples;           /* = 0 */
}
LLGLRenderingLimits;

typedef struct LLGLResourceHeapDescriptor
{
    LLGLPipelineLayout pipelineLayout;   /* = LLGL_NULL_OBJECT */
    uint32_t           numResourceViews; /* = 0 */
    long               barrierFlags;     /* = 0 */
}
LLGLResourceHeapDescriptor;

typedef struct LLGLShaderMacro
{
    const char* name;       /* = NULL */
    const char* definition; /* = NULL */
}
LLGLShaderMacro;

typedef struct LLGLTextureSubresource
{
    uint32_t baseArrayLayer; /* = 0 */
    uint32_t numArrayLayers; /* = 1 */
    uint32_t baseMipLevel;   /* = 0 */
    uint32_t numMipLevels;   /* = 1 */
}
LLGLTextureSubresource;

typedef struct LLGLSubresourceFootprint
{
    uint64_t size;         /* = 0 */
    uint32_t rowAlignment; /* = 0 */
    uint32_t rowSize;      /* = 0 */
    uint32_t rowStride;    /* = 0 */
    uint32_t layerSize;    /* = 0 */
    uint32_t layerStride;  /* = 0 */
}
LLGLSubresourceFootprint;

typedef struct LLGLExtent2D
{
    uint32_t width;  /* = 0 */
    uint32_t height; /* = 0 */
}
LLGLExtent2D;

typedef struct LLGLExtent3D
{
    uint32_t width;  /* = 0 */
    uint32_t height; /* = 0 */
    uint32_t depth;  /* = 0 */
}
LLGLExtent3D;

typedef struct LLGLOffset2D
{
    int32_t x; /* = 0 */
    int32_t y; /* = 0 */
}
LLGLOffset2D;

typedef struct LLGLOffset3D
{
    int32_t x; /* = 0 */
    int32_t y; /* = 0 */
    int32_t z; /* = 0 */
}
LLGLOffset3D;

typedef struct LLGLBufferViewDescriptor
{
    LLGLFormat format; /* = LLGLFormatUndefined */
    uint64_t   offset; /* = 0 */
    uint64_t   size;   /* = LLGLConstantswholeSize */
}
LLGLBufferViewDescriptor;

typedef struct LLGLAttachmentClear
{
    long           flags;           /* = 0 */
    uint32_t       colorAttachment; /* = 0 */
    LLGLClearValue clearValue;
}
LLGLAttachmentClear;

typedef struct LLGLDisplayModeDescriptor
{
    LLGLExtent2D resolution;
    uint32_t     refreshRate; /* = 0 */
}
LLGLDisplayModeDescriptor;

typedef struct LLGLFormatAttributes
{
    uint16_t        bitSize;
    uint8_t         blockWidth;
    uint8_t         blockHeight;
    uint8_t         components;
    LLGLImageFormat format;
    LLGLDataType    dataType;
    long            flags;
}
LLGLFormatAttributes;

typedef struct LLGLFragmentAttribute
{
    const char*     name;
    LLGLFormat      format;      /* = LLGLFormatRGBA32Float */
    uint32_t        location;    /* = 0 */
    LLGLSystemValue systemValue; /* = LLGLSystemValueUndefined */
}
LLGLFragmentAttribute;

typedef struct LLGLSrcImageDescriptor
{
    LLGLImageFormat format;   /* = LLGLImageFormatRGBA */
    LLGLDataType    dataType; /* = LLGLDataTypeUInt8 */
    const void*     data;     /* = NULL */
    size_t          dataSize; /* = 0 */
}
LLGLSrcImageDescriptor;

typedef struct LLGLDstImageDescriptor
{
    LLGLImageFormat format;   /* = LLGLImageFormatRGBA */
    LLGLDataType    dataType; /* = LLGLDataTypeUInt8 */
    void*           data;     /* = NULL */
    size_t          dataSize; /* = 0 */
}
LLGLDstImageDescriptor;

typedef struct LLGLBindingDescriptor
{
    const char*      name;
    LLGLResourceType type;       /* = LLGLResourceTypeUndefined */
    long             bindFlags;  /* = 0 */
    long             stageFlags; /* = 0 */
    LLGLBindingSlot  slot;
    uint32_t         arraySize;  /* = 0 */
}
LLGLBindingDescriptor;

typedef struct LLGLUniformDescriptor
{
    const char*     name;
    LLGLUniformType type;      /* = LLGLUniformTypeUndefined */
    uint32_t        arraySize; /* = 0 */
}
LLGLUniformDescriptor;

typedef struct LLGLDepthDescriptor
{
    bool          testEnabled;  /* = false */
    bool          writeEnabled; /* = false */
    LLGLCompareOp compareOp;    /* = LLGLCompareOpLess */
}
LLGLDepthDescriptor;

typedef struct LLGLStencilFaceDescriptor
{
    LLGLStencilOp stencilFailOp; /* = LLGLStencilOpKeep */
    LLGLStencilOp depthFailOp;   /* = LLGLStencilOpKeep */
    LLGLStencilOp depthPassOp;   /* = LLGLStencilOpKeep */
    LLGLCompareOp compareOp;     /* = LLGLCompareOpLess */
    uint32_t      readMask;      /* = 0u */
    uint32_t      writeMask;     /* = 0u */
    uint32_t      reference;     /* = 0u */
}
LLGLStencilFaceDescriptor;

typedef struct LLGLRasterizerDescriptor
{
    LLGLPolygonMode         polygonMode;               /* = LLGLPolygonModeFill */
    LLGLCullMode            cullMode;                  /* = LLGLCullModeDisabled */
    LLGLDepthBiasDescriptor depthBias;
    bool                    frontCCW;                  /* = false */
    bool                    discardEnabled;            /* = false */
    bool                    depthClampEnabled;         /* = false */
    bool                    scissorTestEnabled;        /* = false */
    bool                    multiSampleEnabled;        /* = false */
    bool                    antiAliasedLineEnabled;    /* = false */
    bool                    conservativeRasterization; /* = false */
    float                   lineWidth;                 /* = 1.0f */
}
LLGLRasterizerDescriptor;

typedef struct LLGLBlendTargetDescriptor
{
    bool                blendEnabled;    /* = false */
    LLGLBlendOp         srcColor;        /* = LLGLBlendOpSrcAlpha */
    LLGLBlendOp         dstColor;        /* = LLGLBlendOpInvSrcAlpha */
    LLGLBlendArithmetic colorArithmetic; /* = LLGLBlendArithmeticAdd */
    LLGLBlendOp         srcAlpha;        /* = LLGLBlendOpSrcAlpha */
    LLGLBlendOp         dstAlpha;        /* = LLGLBlendOpInvSrcAlpha */
    LLGLBlendArithmetic alphaArithmetic; /* = LLGLBlendArithmeticAdd */
    uint8_t             colorMask;       /* = LLGLColorMaskAll */
}
LLGLBlendTargetDescriptor;

typedef struct LLGLTessellationDescriptor
{
    LLGLTessellationPartition partition;        /* = LLGLTessellationPartitionUndefined */
    LLGLFormat                indexFormat;      /* = LLGLFormatUndefined */
    uint32_t                  maxTessFactor;    /* = 64 */
    bool                      outputWindingCCW; /* = false */
}
LLGLTessellationDescriptor;

typedef struct LLGLQueryHeapDescriptor
{
    LLGLQueryType type;            /* = LLGLQueryTypeSamplesPassed */
    uint32_t      numQueries;      /* = 1 */
    bool          renderCondition; /* = false */
}
LLGLQueryHeapDescriptor;

typedef struct LLGLAttachmentFormatDescriptor
{
    LLGLFormat            format;  /* = LLGLFormatUndefined */
    LLGLAttachmentLoadOp  loadOp;  /* = LLGLAttachmentLoadOpUndefined */
    LLGLAttachmentStoreOp storeOp; /* = LLGLAttachmentStoreOpUndefined */
}
LLGLAttachmentFormatDescriptor;

typedef struct LLGLRenderSystemDescriptor
{
    const char*           moduleName;
    long                  flags;              /* = 0 */
    LLGLRenderingProfiler profiler;           /* = LLGL_NULL_OBJECT */
    LLGLRenderingDebugger debugger;           /* = LLGL_NULL_OBJECT */
    const void*           rendererConfig;     /* = NULL */
    size_t                rendererConfigSize; /* = 0 */
#if defined LLGL_OS_ANDROID
    android_app*          androidApp;
#endif /* defined LLGL_OS_ANDROID */
}
LLGLRenderSystemDescriptor;

typedef struct LLGLRenderingCapabilities
{
    LLGLScreenOrigin           screenOrigin;        /* = LLGLScreenOriginUpperLeft */
    LLGLClippingRange          clippingRange;       /* = LLGLClippingRangeZeroToOne */
    size_t                     numShadingLanguages; /* = 0 */
    const LLGLShadingLanguage* shadingLanguages;    /* = NULL */
    size_t                     numTextureFormats;   /* = 0 */
    const LLGLFormat*          textureFormats;      /* = NULL */
    LLGLRenderingFeatures      features;
    LLGLRenderingLimits        limits;
}
LLGLRenderingCapabilities;

typedef struct LLGLAttachmentDescriptor
{
    LLGLFormat  format;     /* = LLGLFormatUndefined */
    LLGLTexture texture;    /* = LLGL_NULL_OBJECT */
    uint32_t    mipLevel;   /* = 0 */
    uint32_t    arrayLayer; /* = 0 */
}
LLGLAttachmentDescriptor;

typedef struct LLGLSamplerDescriptor
{
    LLGLSamplerAddressMode addressModeU;   /* = LLGLSamplerAddressModeRepeat */
    LLGLSamplerAddressMode addressModeV;   /* = LLGLSamplerAddressModeRepeat */
    LLGLSamplerAddressMode addressModeW;   /* = LLGLSamplerAddressModeRepeat */
    LLGLSamplerFilter      minFilter;      /* = LLGLSamplerFilterLinear */
    LLGLSamplerFilter      magFilter;      /* = LLGLSamplerFilterLinear */
    LLGLSamplerFilter      mipMapFilter;   /* = LLGLSamplerFilterLinear */
    bool                   mipMapEnabled;  /* = true */
    float                  mipMapLODBias;  /* = 0.0f */
    float                  minLOD;         /* = 0.0f */
    float                  maxLOD;         /* = 1000.0f */
    uint32_t               maxAnisotropy;  /* = 1 */
    bool                   compareEnabled; /* = false */
    LLGLCompareOp          compareOp;      /* = LLGLCompareOpLess */
    float                  borderColor[4]; /* = {0.0f,0.0f,0.0f,0.0f} */
}
LLGLSamplerDescriptor;

typedef struct LLGLComputeShaderAttributes
{
    LLGLExtent3D workGroupSize; /* = {1,1,1} */
}
LLGLComputeShaderAttributes;

typedef struct LLGLSwapChainDescriptor
{
    LLGLExtent2D resolution;
    int          colorBits;   /* = 32 */
    int          depthBits;   /* = 24 */
    int          stencilBits; /* = 8 */
    uint32_t     samples;     /* = 1 */
    uint32_t     swapBuffers; /* = 2 */
    bool         fullscreen;  /* = false */
}
LLGLSwapChainDescriptor;

typedef struct LLGLTextureSwizzleRGBA
{
    LLGLTextureSwizzle r : 8; /* = LLGLTextureSwizzleRed */
    LLGLTextureSwizzle g : 8; /* = LLGLTextureSwizzleGreen */
    LLGLTextureSwizzle b : 8; /* = LLGLTextureSwizzleBlue */
    LLGLTextureSwizzle a : 8; /* = LLGLTextureSwizzleAlpha */
}
LLGLTextureSwizzleRGBA;

typedef struct LLGLTextureLocation
{
    LLGLOffset3D offset;
    uint32_t     arrayLayer; /* = 0 */
    uint32_t     mipLevel;   /* = 0 */
}
LLGLTextureLocation;

typedef struct LLGLTextureRegion
{
    LLGLTextureSubresource subresource;
    LLGLOffset3D           offset;
    LLGLExtent3D           extent;
}
LLGLTextureRegion;

typedef struct LLGLTextureDescriptor
{
    LLGLTextureType type;        /* = LLGLTextureTypeTexture2D */
    long            bindFlags;   /* = (LLGLBindSampled | LLGLBindColorAttachment) */
    long            miscFlags;   /* = (LLGLMiscFixedSamples | LLGLMiscGenerateMips) */
    LLGLFormat      format;      /* = LLGLFormatRGBA8UNorm */
    LLGLExtent3D    extent;      /* = {1,1,1} */
    uint32_t        arrayLayers; /* = 1 */
    uint32_t        mipLevels;   /* = 0 */
    uint32_t        samples;     /* = 1 */
    LLGLClearValue  clearValue;
}
LLGLTextureDescriptor;

typedef struct LLGLVertexAttribute
{
    const char*     name;
    LLGLFormat      format;          /* = LLGLFormatRGBA32Float */
    uint32_t        location;        /* = 0 */
    uint32_t        semanticIndex;   /* = 0 */
    LLGLSystemValue systemValue;     /* = LLGLSystemValueUndefined */
    uint32_t        slot;            /* = 0 */
    uint32_t        offset;          /* = 0 */
    uint32_t        stride;          /* = 0 */
    uint32_t        instanceDivisor; /* = 0 */
}
LLGLVertexAttribute;

typedef struct LLGLWindowDescriptor
{
    const char*  title;
    LLGLOffset2D position;
    LLGLExtent2D size;
    long         flags;             /* = 0 */
    const void*  windowContext;     /* = NULL */
    size_t       windowContextSize; /* = 0 */
}
LLGLWindowDescriptor;

typedef struct LLGLBufferDescriptor
{
    uint64_t                   size;             /* = 0 */
    uint32_t                   stride;           /* = 0 */
    LLGLFormat                 format;           /* = LLGLFormatUndefined */
    long                       bindFlags;        /* = 0 */
    long                       cpuAccessFlags;   /* = 0 */
    long                       miscFlags;        /* = 0 */
    size_t                     numVertexAttribs; /* = 0 */
    const LLGLVertexAttribute* vertexAttribs;    /* = NULL */
}
LLGLBufferDescriptor;

typedef struct LLGLStaticSamplerDescriptor
{
    const char*           name;
    long                  stageFlags; /* = 0 */
    LLGLBindingSlot       slot;
    LLGLSamplerDescriptor sampler;
}
LLGLStaticSamplerDescriptor;

typedef struct LLGLStencilDescriptor
{
    bool                      testEnabled;      /* = false */
    bool                      referenceDynamic; /* = false */
    LLGLStencilFaceDescriptor front;
    LLGLStencilFaceDescriptor back;
}
LLGLStencilDescriptor;

typedef struct LLGLBlendDescriptor
{
    bool                      alphaToCoverageEnabled;  /* = false */
    bool                      independentBlendEnabled; /* = false */
    uint32_t                  sampleMask;              /* = 0u */
    LLGLLogicOp               logicOp;                 /* = LLGLLogicOpDisabled */
    float                     blendFactor[4];          /* = {0.0f,0.0f,0.0f,0.0f} */
    bool                      blendFactorDynamic;      /* = false */
    LLGLBlendTargetDescriptor targets[8];
}
LLGLBlendDescriptor;

typedef struct LLGLRenderPassDescriptor
{
    LLGLAttachmentFormatDescriptor colorAttachments[8];
    LLGLAttachmentFormatDescriptor depthAttachment;
    LLGLAttachmentFormatDescriptor stencilAttachment;
    uint32_t                       samples;             /* = 1 */
}
LLGLRenderPassDescriptor;

typedef struct LLGLRenderTargetDescriptor
{
    LLGLRenderPass           renderPass;             /* = LLGL_NULL_OBJECT */
    LLGLExtent2D             resolution;
    uint32_t                 samples;                /* = 1 */
    LLGLAttachmentDescriptor colorAttachments[8];
    LLGLAttachmentDescriptor resolveAttachments[8];
    LLGLAttachmentDescriptor depthStencilAttachment;
}
LLGLRenderTargetDescriptor;

typedef struct LLGLVertexShaderAttributes
{
    size_t                     numInputAttribs;  /* = 0 */
    const LLGLVertexAttribute* inputAttribs;     /* = NULL */
    size_t                     numOutputAttribs; /* = 0 */
    const LLGLVertexAttribute* outputAttribs;    /* = NULL */
}
LLGLVertexShaderAttributes;

typedef struct LLGLFragmentShaderAttributes
{
    size_t                       numOutputAttribs; /* = 0 */
    const LLGLFragmentAttribute* outputAttribs;    /* = NULL */
}
LLGLFragmentShaderAttributes;

typedef struct LLGLShaderResourceReflection
{
    LLGLBindingDescriptor binding;
    uint32_t              constantBufferSize; /* = 0 */
    LLGLStorageBufferType storageBufferType;  /* = LLGLStorageBufferTypeUndefined */
}
LLGLShaderResourceReflection;

typedef struct LLGLTextureViewDescriptor
{
    LLGLTextureType        type;        /* = LLGLTextureTypeTexture2D */
    LLGLFormat             format;      /* = LLGLFormatRGBA8UNorm */
    LLGLTextureSubresource subresource;
    LLGLTextureSwizzleRGBA swizzle;
}
LLGLTextureViewDescriptor;

typedef struct LLGLPipelineLayoutDescriptor
{
    size_t                             numHeapBindings;   /* = 0 */
    const LLGLBindingDescriptor*       heapBindings;      /* = NULL */
    size_t                             numBindings;       /* = 0 */
    const LLGLBindingDescriptor*       bindings;          /* = NULL */
    size_t                             numStaticSamplers; /* = 0 */
    const LLGLStaticSamplerDescriptor* staticSamplers;    /* = NULL */
    size_t                             numUniforms;       /* = 0 */
    const LLGLUniformDescriptor*       uniforms;          /* = NULL */
}
LLGLPipelineLayoutDescriptor;

typedef struct LLGLGraphicsPipelineDescriptor
{
    LLGLPipelineLayout         pipelineLayout;       /* = LLGL_NULL_OBJECT */
    LLGLRenderPass             renderPass;           /* = LLGL_NULL_OBJECT */
    LLGLShader                 vertexShader;         /* = LLGL_NULL_OBJECT */
    LLGLShader                 tessControlShader;    /* = LLGL_NULL_OBJECT */
    LLGLShader                 tessEvaluationShader; /* = LLGL_NULL_OBJECT */
    LLGLShader                 geometryShader;       /* = LLGL_NULL_OBJECT */
    LLGLShader                 fragmentShader;       /* = LLGL_NULL_OBJECT */
    LLGLPrimitiveTopology      primitiveTopology;    /* = LLGLPrimitiveTopologyTriangleList */
    size_t                     numViewports;         /* = 0 */
    const LLGLViewport*        viewports;            /* = NULL */
    size_t                     numScissors;          /* = 0 */
    const LLGLScissor*         scissors;             /* = NULL */
    LLGLDepthDescriptor        depth;
    LLGLStencilDescriptor      stencil;
    LLGLRasterizerDescriptor   rasterizer;
    LLGLBlendDescriptor        blend;
    LLGLTessellationDescriptor tessellation;
}
LLGLGraphicsPipelineDescriptor;

typedef struct LLGLResourceViewDescriptor
{
    LLGLResource              resource;     /* = LLGL_NULL_OBJECT */
    LLGLTextureViewDescriptor textureView;
    LLGLBufferViewDescriptor  bufferView;
    uint32_t                  initialCount; /* = 0 */
}
LLGLResourceViewDescriptor;

typedef struct LLGLShaderDescriptor
{
    LLGLShaderType               type;       /* = LLGLShaderTypeUndefined */
    const char*                  source;     /* = NULL */
    size_t                       sourceSize; /* = 0 */
    LLGLShaderSourceType         sourceType; /* = LLGLShaderSourceTypeCodeFile */
    const char*                  entryPoint; /* = NULL */
    const char*                  profile;    /* = NULL */
    const LLGLShaderMacro*       defines;    /* = NULL */
    long                         flags;      /* = 0 */
    LLGLVertexShaderAttributes   vertex;
    LLGLFragmentShaderAttributes fragment;
    LLGLComputeShaderAttributes  compute;
}
LLGLShaderDescriptor;

typedef struct LLGLShaderReflection
{
    size_t                              numResources; /* = 0 */
    const LLGLShaderResourceReflection* resources;    /* = NULL */
    size_t                              numUniforms;  /* = 0 */
    const LLGLUniformDescriptor*        uniforms;     /* = NULL */
    LLGLVertexShaderAttributes          vertex;
    LLGLFragmentShaderAttributes        fragment;
    LLGLComputeShaderAttributes         compute;
}
LLGLShaderReflection;


#endif /* LLGL_C99_LLGLWRAPPER_H */



/* ================================================================================ */

