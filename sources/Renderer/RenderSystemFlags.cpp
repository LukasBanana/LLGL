/*
 * RenderSystemFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/RenderSystemFlags.h>
#include <LLGL/Strings.h>
#include "../Core/Helper.h"


namespace LLGL
{


static bool ReportValidationFailure(const ValidateRenderingCapsFunc& callback, const std::string& info, const std::string& attrib)
{
    if (callback)
        return callback(info, attrib);
    else
        return false;
}

LLGL_EXPORT bool ValidateRenderingCaps(
    const RenderingCapabilities&        presentCaps,
    const RenderingCapabilities&        requiredCaps,
    const ValidateRenderingCapsFunc&    callback)
{
    bool result = true;

    #define LLGL_CONTINUE_VALIDATION_IF(COND)   \
        if (!COND)                              \
            return false;                       \
        else                                    \
            result = false

    /* Validate shading languages */
    for (std::size_t i = 0; i < requiredCaps.shadingLanguages.size(); ++i)
    {
        auto shaderLang = requiredCaps.shadingLanguages[i];
        if (!Contains(presentCaps.shadingLanguages, shaderLang))
        {
            bool continueValidation = ReportValidationFailure(
                callback,
                "shading language not supported: " + std::string(ToString(shaderLang)),
                "shadingLanguages[" + std::to_string(i) + ']'
            );
            LLGL_CONTINUE_VALIDATION_IF(continueValidation);
        }
    }

    /* Validate texture formats */
    for (std::size_t i = 0; i < requiredCaps.textureFormats.size(); ++i)
    {
        auto texFormat = requiredCaps.textureFormats[i];
        if (!Contains(presentCaps.textureFormats, texFormat))
        {
            bool continueValidation = ReportValidationFailure(
                callback,
                "texture format not supported: " + std::string(ToString(texFormat)),
                "textureFormats[" + std::to_string(i) + ']'
            );
            LLGL_CONTINUE_VALIDATION_IF(continueValidation);
        }
    }

    /* Validate features */
    #define LLGL_VALIDATE_FEATURE(ATTRIB, INFO)                             \
        if (requiredCaps.features.ATTRIB && !presentCaps.features.ATTRIB)   \
        {                                                                   \
            bool continueValidation = ReportValidationFailure(              \
                callback, INFO " not supported", #ATTRIB                    \
            );                                                              \
            LLGL_CONTINUE_VALIDATION_IF(continueValidation);                \
        }

    LLGL_VALIDATE_FEATURE( hasCommandBufferExt,          "extended command buffer"    );
    LLGL_VALIDATE_FEATURE( hasRenderTargets,             "render targets"             );
    LLGL_VALIDATE_FEATURE( has3DTextures,                "3D textures"                );
    LLGL_VALIDATE_FEATURE( hasCubeTextures,              "cube textures"              );
    LLGL_VALIDATE_FEATURE( hasArrayTextures,             "array textures"             );
    LLGL_VALIDATE_FEATURE( hasCubeArrayTextures,         "cube array textures"        );
    LLGL_VALIDATE_FEATURE( hasMultiSampleTextures,       "multi-sample textures"      );
    LLGL_VALIDATE_FEATURE( hasSamplers,                  "samplers"                   );
    LLGL_VALIDATE_FEATURE( hasConstantBuffers,           "constant buffers"           );
    LLGL_VALIDATE_FEATURE( hasStorageBuffers,            "storage buffers"            );
    LLGL_VALIDATE_FEATURE( hasUniforms,                  "uniforms"                   );
    LLGL_VALIDATE_FEATURE( hasGeometryShaders,           "geometry shaders"           );
    LLGL_VALIDATE_FEATURE( hasTessellationShaders,       "tessellation shaders"       );
    LLGL_VALIDATE_FEATURE( hasComputeShaders,            "compute shaders"            );
    LLGL_VALIDATE_FEATURE( hasInstancing,                "instancing"                 );
    LLGL_VALIDATE_FEATURE( hasOffsetInstancing,          "offset instancing"          );
    LLGL_VALIDATE_FEATURE( hasViewportArrays,            "viewport arrays"            );
    LLGL_VALIDATE_FEATURE( hasConservativeRasterization, "conservative rasterization" );
    LLGL_VALIDATE_FEATURE( hasStreamOutputs,             "stream outputs"             );
    LLGL_VALIDATE_FEATURE( hasLogicOp,                   "logic fragment operations"  );

    #undef LLGL_VALIDATE_FEATURE

    /* Validate special case of limits */
    if (requiredCaps.limits.lineWidthRange[0] < presentCaps.limits.lineWidthRange[0])
    {
        bool continueValidation = ReportValidationFailure(
            callback,
            (
                "line width range lower bound of " + std::to_string(requiredCaps.limits.lineWidthRange[0]) +
                " exceeded limit of " + std::to_string(presentCaps.limits.lineWidthRange[0])
            ),
            "lineWidthRange[0]"
        );
        LLGL_CONTINUE_VALIDATION_IF(continueValidation);
    }

    if (requiredCaps.limits.lineWidthRange[1] > presentCaps.limits.lineWidthRange[1])
    {
        bool continueValidation = ReportValidationFailure(
            callback,
            (
                "line width range upper bound of " + std::to_string(requiredCaps.limits.lineWidthRange[1]) +
                " exceeded limit of " + std::to_string(presentCaps.limits.lineWidthRange[1])
            ),
            "lineWidthRange[1]"
        );
        LLGL_CONTINUE_VALIDATION_IF(continueValidation);
    }

    /* Validate limits */
    #define LLGL_VALIDATE_LIMIT(ATTRIB, INFO)                                               \
        if (requiredCaps.limits.ATTRIB > presentCaps.limits.ATTRIB)                         \
        {                                                                                   \
            bool continueValidation = ReportValidationFailure(                              \
                callback,                                                                   \
                (                                                                           \
                    "required " INFO " of " + std::to_string(requiredCaps.limits.ATTRIB) +  \
                    " exceeded limit of " + std::to_string(presentCaps.limits.ATTRIB)       \
                ),                                                                          \
                #ATTRIB                                                                     \
            );                                                                              \
            LLGL_CONTINUE_VALIDATION_IF(continueValidation);                                \
        }

    LLGL_VALIDATE_LIMIT( maxNumTextureArrayLayers,          "texture array layers count"                );
    LLGL_VALIDATE_LIMIT( maxNumRenderTargetAttachments,     "render target attachment count"            );
    LLGL_VALIDATE_LIMIT( maxPatchVertices,                  "patch vertices"                            );
    LLGL_VALIDATE_LIMIT( max1DTextureSize,                  "1D texture size"                           );
    LLGL_VALIDATE_LIMIT( max2DTextureSize,                  "2D texture size"                           );
    LLGL_VALIDATE_LIMIT( max3DTextureSize,                  "3D texture size"                           );
    LLGL_VALIDATE_LIMIT( maxCubeTextureSize,                "cube texture size"                         );
    LLGL_VALIDATE_LIMIT( maxAnisotropy,                     "anisotropy"                                );
    LLGL_VALIDATE_LIMIT( maxNumComputeShaderWorkGroups[0],  "compute shader work group count on X-axis" );
    LLGL_VALIDATE_LIMIT( maxNumComputeShaderWorkGroups[1],  "compute shader work group count on Y-axis" );
    LLGL_VALIDATE_LIMIT( maxNumComputeShaderWorkGroups[2],  "compute shader work group count on Z-axis" );
    LLGL_VALIDATE_LIMIT( maxComputeShaderWorkGroupSize[0],  "compute shader work group size on X-axis"  );
    LLGL_VALIDATE_LIMIT( maxComputeShaderWorkGroupSize[1],  "compute shader work group size on Y-axis"  );
    LLGL_VALIDATE_LIMIT( maxComputeShaderWorkGroupSize[2],  "compute shader work group size on Z-axis"  );
    LLGL_VALIDATE_LIMIT( maxNumViewports,                   "viewport count"                            );
    LLGL_VALIDATE_LIMIT( maxViewportSize[0],                "viewport width"                            );
    LLGL_VALIDATE_LIMIT( maxViewportSize[1],                "viewport height"                           );
    LLGL_VALIDATE_LIMIT( maxBufferSize,                     "buffer size"                               );
    LLGL_VALIDATE_LIMIT( maxConstantBufferSize,             "constant buffer size"                      );

    #undef LLGL_VALIDATE_LIMIT
    #undef LLGL_CONTINUE_VALIDATION_IF

    return result;
}


} // /namespace LLGL



// ================================================================================
