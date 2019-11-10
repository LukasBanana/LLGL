/*
 * ShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/ShaderProgram.h>
#include <LLGL/Shader.h>
#include <algorithm>
#include "../Core/Helper.h"
#include "../Core/HelperMacros.h"


namespace LLGL
{


/*
 * ======= Protected: =======
 */

bool ShaderProgram::ValidateShaderComposition(Shader* const * shaders, std::size_t numShaders)
{
    enum ShaderTypeBits
    {
        BitVert = (1 << static_cast<int>( ShaderType::Vertex         )),
        BitTesc = (1 << static_cast<int>( ShaderType::TessControl    )),
        BitTese = (1 << static_cast<int>( ShaderType::TessEvaluation )),
        BitGeom = (1 << static_cast<int>( ShaderType::Geometry       )),
        BitFrag = (1 << static_cast<int>( ShaderType::Fragment       )),
        BitComp = (1 << static_cast<int>( ShaderType::Compute        )),
    };

    /* Determine which shader types are attached */
    bool hasPostTessVertexShader = false;
    int bitmask = 0;

    for (std::size_t i = 0; i < numShaders; ++i)
    {
        if (auto shader = shaders[i])
        {
            /* Check if bit was already set */
            auto bit = (1 << static_cast<int>(shader->GetType()));

            if ((bitmask & bit) != 0)
                return false;

            bitmask |= bit;

            /* Check if a post-tessellation vertex shader is used to enable validation with Metal semantics */
            if (shader->IsPostTessellationVertex())
                hasPostTessVertexShader = true;
        }
    }

    /* Validate composition of attached shaders */
    if (hasPostTessVertexShader)
    {
        switch (bitmask)
        {
            case (          BitVert          ):
            case (          BitVert | BitFrag):
            case (BitComp                    ):
            case (BitComp | BitVert          ):
            case (BitComp | BitVert | BitFrag):
                return true;
        }
    }
    else
    {
        switch (bitmask)
        {
            case (BitVert                                        ):
            case (BitVert |                     BitGeom          ):
            case (BitVert | BitTesc | BitTese                    ):
            case (BitVert | BitTesc | BitTese | BitGeom          ):
            case (BitVert |                               BitFrag):
            case (BitVert |                     BitGeom | BitFrag):
            case (BitVert | BitTesc | BitTese |           BitFrag):
            case (BitVert | BitTesc | BitTese | BitGeom | BitFrag):
            case (BitComp):
                return true;
        }
    }

    return false;
}

static int CompareResourceViewSWO(const ShaderResource& lhs, const ShaderResource& rhs)
{
    LLGL_COMPARE_MEMBER_SWO(binding.type     );
    LLGL_COMPARE_MEMBER_SWO(binding.bindFlags);
    LLGL_COMPARE_MEMBER_SWO(binding.slot     );
    return 0;
}

void ShaderProgram::ClearShaderReflection(ShaderReflection& reflection)
{
    reflection.resources.clear();
    reflection.uniforms.clear();
    reflection.vertex.inputAttribs.clear();
    reflection.vertex.outputAttribs.clear();
    reflection.fragment.outputAttribs.clear();
    reflection.compute.workGroupSize = { 0, 0, 0 };
}

void ShaderProgram::FinalizeShaderReflection(ShaderReflection& reflection)
{
    /* Sort vertex input and output attributes by their location */
    auto SortVertexAttributes = [](std::vector<VertexAttribute>& attribs)
    {
        std::sort(
            attribs.begin(),
            attribs.end(),
            [](const VertexAttribute& lhs, const VertexAttribute& rhs) -> bool
            {
                return (lhs.location < rhs.location);
            }
        );
    };

    SortVertexAttributes(reflection.vertex.inputAttribs);
    SortVertexAttributes(reflection.vertex.outputAttribs);

    /* Sort fragment output attributes by their location */
    std::sort(
        reflection.fragment.outputAttribs.begin(),
        reflection.fragment.outputAttribs.end(),
        [](const FragmentAttribute& lhs, const FragmentAttribute& rhs) -> bool
        {
            return (lhs.location < rhs.location);
        }
    );

    /* Sort resources by their strict-weak-order (SWO) */
    std::sort(
        reflection.resources.begin(),
        reflection.resources.end(),
        [](const ShaderResource& lhs, const ShaderResource& rhs)
        {
            return (CompareResourceViewSWO(lhs, rhs) < 0);
        }
    );

    /* Merge stage flags of equal resource bindings */
    auto mergeEnd = UniqueMerge(
        reflection.resources.begin(),
        reflection.resources.end(),
        []( ShaderResource& lhs, const ShaderResource& rhs) -> bool
        {
            /* Compare shader resource on equality (except of binding flags and stage flags) */
            if (lhs.binding.name        == rhs.binding.name         &&
                lhs.binding.type        == rhs.binding.type         &&
                lhs.binding.slot        == rhs.binding.slot         &&
                lhs.binding.arraySize   == rhs.binding.arraySize    &&
                lhs.constantBufferSize  == rhs.constantBufferSize   &&
                lhs.storageBufferType   == rhs.storageBufferType)
            {
                /* Merge binding flags and stage flags */
                lhs.binding.bindFlags   |= rhs.binding.bindFlags;
                lhs.binding.stageFlags  |= rhs.binding.stageFlags;
                return true;
            }
            return false;
        }
    );

    reflection.resources.erase(mergeEnd, reflection.resources.end());
}

const char* ShaderProgram::LinkErrorToString(const LinkError errorCode)
{
    switch (errorCode)
    {
        case LinkError::InvalidComposition:
            return "invalid composition of attached shaders";
        case LinkError::InvalidByteCode:
            return "invalid shader byte code";
        case LinkError::TooManyAttachments:
            return "too many attachments in shader program";
        case LinkError::IncompleteAttachments:
            return "incomplete attachments in shader program";
        default:
            return nullptr;
    }
}


} // /namespace LLGL



// ================================================================================
