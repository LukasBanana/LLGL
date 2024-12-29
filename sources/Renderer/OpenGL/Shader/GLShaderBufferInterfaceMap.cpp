/*
 * GLShaderBufferInterfaceMap.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLShaderBufferInterfaceMap.h"
#include "../RenderState/GLPipelineLayout.h"
#include "GLShaderPipeline.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


GLShaderBufferInterfaceMap::GLShaderBufferInterfaceMap() :
    numSSBOs_                  { 0 },
    numHeapEntries_            { 0 },
    hasHeapSSBOEntriesOnly_    { 1 },
    hasDynamicSSBOEntriesOnly_ { 1 }
{
}

void GLShaderBufferInterfaceMap::BuildMap(const GLPipelineLayout& pipelineLayout, const GLShaderPipeline& shaderPipeline)
{
    LLGL_ASSERT(bufferMap_.empty(), "shader buffer interface map should only be built once");

    /* Query all active texture buffer names in shader pipeline */
    std::set<std::string> samplerBufferNames, imageBufferNames;
    shaderPipeline.QueryTexBufferNames(samplerBufferNames, imageBufferNames);

    auto MapResourceNameToBufferInterface = [&samplerBufferNames, &imageBufferNames](const std::string& name) -> GLBufferInterface
    {
        /* If binding name matches a sampler buffer uniform, interpret binding descriptor as sampler buffer */
        if (samplerBufferNames.find(name) != samplerBufferNames.end())
            return GLBufferInterface_Sampler;

        /* If binding name matches an image buffer uniform, interpret binding descriptor as image buffer */
        if (imageBufferNames.find(name) != imageBufferNames.end())
            return GLBufferInterface_Image;

        /* Otherwise, interpret binding descriptor as SSBO */
        return GLBufferInterface_SSBO;
    };

    /*
    Iterate through all SSBO resources in the PSO layout and match it with the name set
    to determine which resources should be considered sampler buffers instead of actual SSBOs.
    */
    for (const GLHeapResourceBinding& binding : pipelineLayout.GetHeapBindings())
    {
        if (!binding.name.empty() && binding.IsSSBO())
            AppendHeapEntry(MapResourceNameToBufferInterface(binding.name));
    }

    for_range(i, pipelineLayout.GetBindings().size())
    {
        const GLPipelineResourceBinding& binding = pipelineLayout.GetBindings()[i];
        const std::string& name = pipelineLayout.GetBindingNames()[i];
        if (!name.empty() && binding.IsSSBO())
            AppendDynamicEntry(MapResourceNameToBufferInterface(name));
    }
}


/*
 * ======= Private: =======
 */

void GLShaderBufferInterfaceMap::AppendHeapEntry(GLBufferInterface entry)
{
    bufferMap_.push_back(entry);
    if (entry == GLBufferInterface_SSBO)
    {
        /* Count total number of SSBO entries */
        ++numSSBOs_;
    }
    else
    {
        /* Mark that this interface has a mix of SSBO and non-SSBO entries for heap bindings */
        hasHeapSSBOEntriesOnly_ = 0;
    }
    ++numHeapEntries_;
}

void GLShaderBufferInterfaceMap::AppendDynamicEntry(GLBufferInterface entry)
{
    bufferMap_.push_back(entry);
    if (entry == GLBufferInterface_SSBO)
    {
        /* Count total number of SSBO entries */
        ++numSSBOs_;
    }
    else
    {
        /* Mark that this interface has a mix of SSBO and non-SSBO entries for dynamic bindings */
        hasDynamicSSBOEntriesOnly_ = 0;
    }
}


} // /namespace LLGL



// ================================================================================
