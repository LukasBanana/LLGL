/*
 * VKShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_SHADER_PROGRAM_H
#define LLGL_VK_SHADER_PROGRAM_H


#include <LLGL/ShaderProgram.h>
#include "../Vulkan.h"
#include "../VKPtr.h"
#include <vector>


namespace LLGL
{


class VKShader;

class VKShaderProgram final : public ShaderProgram
{

    public:

        VKShaderProgram(const ShaderProgramDescriptor& desc);

        bool HasErrors() const override;

        std::string QueryInfoLog() override;

        ShaderReflection QueryReflection() const override;
        UniformLocation QueryUniformLocation(const char* name) const override;

        bool SetWorkGroupSize(const Extent3D& workGroupSize) override;
        bool GetWorkGroupSize(Extent3D& workGroupSize) const override;

    public:

        // Fills the specified array of create-info structures with the shader stages and returns the number of stages that have been written.
        void FillShaderStageCreateInfos(VkPipelineShaderStageCreateInfo* createInfos, std::uint32_t& stageCount) const;

        // Fills the specified create-info structure with the vertex input layout.
        void FillVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& createInfo) const;

    private:

        void Attach(Shader* shader);
        void BuildInputLayout(std::size_t numVertexFormats, const VertexFormat* vertexFormats);
        void Link();

    private:

        std::vector<VKShader*>                          shaders_;

        std::vector<VkVertexInputBindingDescription>    vertexBindingDescs_;
        std::vector<VkVertexInputAttributeDescription>  vertexAttribDescs_;

        LinkError                                       linkError_          = LinkError::NoError;

};


} // /namespace LLGL


#endif



// ================================================================================
