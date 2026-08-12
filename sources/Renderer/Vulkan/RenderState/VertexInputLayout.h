/*
 * VKPipelineLayoutPermutation.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VERTEX_INPUT_LAYOUT_H
#define LLGL_VERTEX_INPUT_LAYOUT_H

#include "../Vulkan.h"
#include <vector>

namespace LLGL
{


struct VertexInputLayout
{
    std::vector<VkVertexInputBindingDescription>    bindingDescs;
    std::vector<VkVertexInputAttributeDescription>  attribDescs;
};


} // /namespace LLGL

#endif



// ================================================================================