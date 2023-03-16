/*
 * VKShader.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_SHADER_H
#define LLGL_VK_SHADER_H


#include <LLGL/Shader.h>
#include <vector>
#include "../Vulkan.h"
#include "../VKPtr.h"
#include "../../../Core/BasicReport.h"


namespace LLGL
{


struct ShaderReflection;
struct Extent3D;

class VKShader final : public Shader
{

    public:

        const Report* GetReport() const override;
        bool Reflect(ShaderReflection& reflection) const override;

    public:

        VKShader(const VKPtr<VkDevice>& device, const ShaderDescriptor& desc);

        bool ReflectLocalSize(Extent3D& localSize) const;

        void FillShaderStageCreateInfo(VkPipelineShaderStageCreateInfo& createInfo) const;
        void FillVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& createInfo) const;

        // Returns the Vulkan shader module.
        inline const VKPtr<VkShaderModule>& GetShaderModule() const
        {
            return shaderModule_;
        }

    private:

        // Note: "Success" is a reserved macro by X11 lib.
        enum class LoadBinaryResult
        {
            Undefined,
            Successful,
            InvalidCodeSize,
            ReflectFailed,
        };

    private:

        bool BuildShader(const ShaderDescriptor& shaderDesc);
        void BuildInputLayout(std::size_t numVertexAttribs, const VertexAttribute* vertexAttribs);
        void BuildReport();

        bool CompileSource(const ShaderDescriptor& shaderDesc);
        bool LoadBinary(const ShaderDescriptor& shaderDesc);

    private:

        struct VertexInputLayout
        {
            std::vector<VkVertexInputBindingDescription>    bindingDescs;
            std::vector<VkVertexInputAttributeDescription>  attribDescs;
        };

    private:

        VkDevice                device_             = VK_NULL_HANDLE;
        VKPtr<VkShaderModule>   shaderModule_;
        std::vector<char>       shaderModuleData_;
        LoadBinaryResult        loadBinaryResult_   = LoadBinaryResult::Undefined;
        VertexInputLayout       inputLayout_;

        std::string             entryPoint_;
        BasicReport             report_;

};


} // /namespace LLGL


#endif



// ================================================================================
