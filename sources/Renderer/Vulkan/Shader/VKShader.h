/*
 * VKShader.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_SHADER_H
#define LLGL_VK_SHADER_H


#include <LLGL/Shader.h>
#include <LLGL/Report.h>
#include "../Vulkan.h"
#include "../VKPtr.h"
#include "VKShaderBindingLayout.h"
#include <LLGL/Container/Vector.h>
#include <functional>


namespace LLGL
{


struct ShaderReflection;
struct Extent3D;

// Container type of 32-bit words for Vulkan shader binary code.
using VKShaderCode = vector<std::uint32_t>;

struct VKUniformRange
{
    std::uint32_t offset;
    std::uint32_t size;
};

class VKShader final : public Shader
{

    public:

        #include <LLGL/Backend/Shader.inl>

    public:

        // Function interface which returns a binding slot ierator to re-assign bindings slots for a permuation of the SPIR-V module.
        using PermutationBindingFunc = std::function<bool(unsigned index, ConstFieldRangeIterator<BindingSlot>& iter, std::uint32_t& dstSet)>;

    public:

        VKShader(VkDevice device, const ShaderDescriptor& desc);
        ~VKShader();

        bool ReflectLocalSize(Extent3D& outLocalSize) const;

        /*
        Reflects the push constants of this shader module and returns their byte ranges.
        The output container has the same number of elements as the input container, but inaccessible uniforms have a zero-range.
        */
        bool ReflectPushConstants(
            const ArrayView<UniformDescriptor>& inUniformDescs,
            vector<VKUniformRange>&             outUniformRanges
        ) const;

        void FillShaderStageCreateInfo(VkPipelineShaderStageCreateInfo& createInfo) const;
        void FillVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& createInfo) const;

        /*
        Returns true if a shader permutation is needed for the specified binding functor.
        Call this before 'CreateVkShaderModulePermutation' to determine whether a permutation is necessary.
        */
        bool NeedsShaderModulePermutation(const PermutationBindingFunc& permutationBindingFunc) const;

        /*
        Creates a shader module permutation with re-assigned binding slots using the specified function callback.
        Re-assigned descriptor sets for [0, N) invocations of the callback until 'permutationBindingFunc' returns false.
        Returns VK_NULL_HANDLE if no permutation was created. Should only be used by VKPipelineLayout.
        */
        VKPtr<VkShaderModule> CreateVkShaderModulePermutation(const PermutationBindingFunc& permutationBindingFunc);

        // Returns the Vulkan shader module.
        inline const VKPtr<VkShaderModule>& GetShaderModule() const
        {
            return shaderModule_;
        }

        // Returns true if this shader's binding layout contains any binding point of the specified descriptor type.
        inline bool HasAnyDescriptorOfType(VkDescriptorType type) const
        {
            return bindingLayout_.HasAnyDescriptorOfType(type);
        }

        // Returns the descriptor type for the specified binding slot.
        // If this shader binding layout does not have such a binding slot, the return value is VK_DESCRIPTOR_TYPE_MAX_ENUM.
        inline VkDescriptorType GetDescriptorTypeForBinding(const BindingSlot& slot) const
        {
            return bindingLayout_.GetDescriptorTypeForBinding(slot);
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
        void BuildBindingLayout();
        void BuildReport();

        bool CompileSource(const ShaderDescriptor& shaderDesc);
        bool LoadBinary(const ShaderDescriptor& shaderDesc);

    private:

        struct VertexInputLayout
        {
            vector<VkVertexInputBindingDescription>    bindingDescs;
            vector<VkVertexInputAttributeDescription>  attribDescs;
        };

    private:

        VkDevice                device_             = VK_NULL_HANDLE;

        VKPtr<VkShaderModule>   shaderModule_;
        VKShaderCode            shaderCode_;
        VKShaderBindingLayout   bindingLayout_;

        LoadBinaryResult        loadBinaryResult_   = LoadBinaryResult::Undefined;
        VertexInputLayout       inputLayout_;

        string                  entryPoint_;
        Report                  report_;

};


} // /namespace LLGL


#endif



// ================================================================================
