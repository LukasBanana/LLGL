/*
 * ShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/ShaderProgram.h>
#include <LLGL/Shader.h>
#include <algorithm>
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
    int bitmask = 0;

    for (std::size_t i = 0; i < numShaders; ++i)
    {
        if (auto shader = shaders[i])
        {
            auto bit = (1 << static_cast<int>(shader->GetType()));

            /* Check if bit was already set */
            if ((bitmask & bit) != 0)
                return false;

            bitmask |= bit;
        }
    }

    /* Validate composition of attached shaders */
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

    return false;
}

static int CompareResourceViewSWO(const ShaderResource& lhs, const ShaderResource& rhs)
{
    LLGL_COMPARE_MEMBER_SWO(binding.type     );
    LLGL_COMPARE_MEMBER_SWO(binding.bindFlags);
    LLGL_COMPARE_MEMBER_SWO(binding.slot     );
    return 0;
}

void ShaderProgram::FinalizeShaderReflection(ShaderReflection& reflection)
{
    std::sort(
        reflection.resources.begin(),
        reflection.resources.end(),
        [](const ShaderResource& lhs, const ShaderResource& rhs)
        {
            return (CompareResourceViewSWO(lhs, rhs) < 0);
        }
    );
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
