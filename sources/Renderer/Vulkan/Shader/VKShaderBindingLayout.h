/*
 * VKShaderBindingLayout.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_SHADER_BINDING_LAYOUT_H
#define LLGL_VK_SHADER_BINDING_LAYOUT_H


#include <LLGL/PipelineLayoutFlags.h>
#include "../../../Core/FieldIterator.h"
#include "../Vulkan.h"
#include <vector>
#include <cstdint>


namespace LLGL
{


// Store shader reflection of binding points per VKShader instance.
class VKShaderBindingLayout
{

    public:

        // Builds the internal binding table from the specified SPIR-V module.
        bool BuildFromSpirvModule(const void* data, std::size_t size);

        // Returns true if the binding layout already matches the layout as is assigned by 'AssignBindingSlots'.
        bool MatchesBindingSlots(
            ConstFieldRangeIterator<BindingSlot>    iter,
            std::uint32_t                           dstSet,
            bool                                    dstBindingInAscendingOrder = false
        ) const;

        /*
        Assigns new binding slots for all resource bindings in the specified range and returns the number of updated bindings.
        Parameter 'dstBindingInAscendingOrder' specifies whether binding indices are to be re-assigned as well, in which case they are assigned from [0, N).
        Otherwise, only the descriptor set is re-assgined.
        */
        std::uint32_t AssignBindingSlots(
            ConstFieldRangeIterator<BindingSlot>    iter,
            std::uint32_t                           dstSet,
            bool                                    dstBindingInAscendingOrder = false
        );

        /*
        Writes the updated resource bindings to the specified SPIR-V module.
        This SPIR-V module must be identical to the one used when the layout was built, except for the binding values.
        */
        void UpdateSpirvModule(void* data, std::size_t size);

        // Returns true if this shader binding layout contains any binding with the specified descriptor type.
        bool HasAnyDescriptorOfType(VkDescriptorType type) const;

        // Returns the descriptor type for the specified binding slot.
        // If this shader binding layout does not have such a binding slot, the return value is VK_DESCRIPTOR_TYPE_MAX_ENUM.
        VkDescriptorType GetDescriptorTypeForBinding(const BindingSlot& slot) const;

    private:

        // Container structure for SPIR-V module resource bindings.
        struct ModuleBinding
        {
            std::uint32_t       srcDescriptorSet;   // Original descriptor set for the binding point.
            std::uint32_t       srcBinding;         // Original binding index for the binding point.
            std::uint32_t       dstDescriptorSet;   // Re-assigned descriptor set.
            std::uint32_t       dstBinding;         // Re-assigned binding index.
            std::uint32_t       spirvDescriptorSet; // SPIR-V word offset to the OpDecorate DescriptorSet instruction operand.
            std::uint32_t       spirvBinding;       // SPIR-V word offset to the OpDecorate Binding instruction operand.
            VkDescriptorType    descriptorType;     // Vulkan descriptor type.
        };

    private:

        bool MatchesBindingSlot(
            const ModuleBinding&    binding,
            const BindingSlot&      slot,
            std::uint32_t           dstSet,
            std::uint32_t*          dstBinding
        ) const;

        bool AssignBindingSlot(
            ModuleBinding&      binding,
            const BindingSlot&  slot,
            std::uint32_t       dstSet,
            std::uint32_t*      dstBinding
        );

    private:

        std::vector<ModuleBinding> bindings_;

};


} // /namespace LLGL


#endif



// ================================================================================
