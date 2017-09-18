/*
 * VKShader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKShader.h"
#include "../VKCore.h"


namespace LLGL
{


VKShader::VKShader(const VKPtr<VkDevice>& device, const ShaderType type) :
    Shader        { type                          },
    device_       { device                        },
    shaderModule_ { device, vkDestroyShaderModule }
{
}

bool VKShader::Compile(const std::string& sourceCode, const ShaderDescriptor& shaderDesc)
{
    return false; // dummy
}


bool VKShader::LoadBinary(std::vector<char>&& binaryCode, const ShaderDescriptor& shaderDesc)
{
    /* Validate code size */
    if (binaryCode.empty() || binaryCode.size() % 4 != 0)
    {
        loadBinaryResult_ = LoadBinaryResult::InvalidCodeSize;
        return false;
    }

    /* Create shader module */
    VkShaderModuleCreateInfo createInfo;

    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext    = nullptr;
    createInfo.flags    = 0;
    createInfo.codeSize = binaryCode.size();
    createInfo.pCode    = reinterpret_cast<const uint32_t*>(binaryCode.data());

    VkResult result = vkCreateShaderModule(device_, &createInfo, nullptr, shaderModule_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan shader module");

    loadBinaryResult_ = LoadBinaryResult::Success;
    
    return true;
}

std::string VKShader::Disassemble(int flags)
{
    return ""; // dummy
}

std::string VKShader::QueryInfoLog()
{
    switch (loadBinaryResult_)
    {
        case LoadBinaryResult::InvalidCodeSize:
            return "shader module code size is not a multiple of four bytes";
        default:
            break;
    }
    return "";
}


} // /namespace LLGL



// ================================================================================
