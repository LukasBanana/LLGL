/*
 * SpirvModule.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_SPIRV_MODULE_H
#define LLGL_SPIRV_MODULE_H


#include "SpirvIterator.h"
#include <LLGL/Container/ArrayView.h>
#include <vector>


namespace LLGL
{


// SPIR-V shader module container.
class SpirvModule
{

    public:

        using value_type        = std::uint32_t;
        using size_type         = typename std::vector<value_type>::size_type;
        using iterator          = SpirvForwardIterator;
        using const_iterator    = SpirvConstForwardIterator;

    public:

        SpirvModule() = default;
        SpirvModule(SpirvModule&&) = default;
        SpirvModule& operator = (SpirvModule&&) = default;

        SpirvModule(std::vector<value_type>&& data);
        SpirvModule(const void* data, size_type size);
        SpirvModule(const ArrayView<value_type>& words);

    public:

        // Reads the SPIR-V module header.
        SpirvResult ReadHeader(SpirvHeader& outHeader) const;

        // Returns the word offset for the specified iterator.
        std::uint32_t WordOffset(const const_iterator& iter) const;

        // Returns the container of 32-bit words.
        inline std::vector<value_type>& Words()
        {
            return words_;
        }

        // Returns the container of 32-bit words.
        inline const std::vector<value_type>& Words() const
        {
            return words_;
        }

    public:

        inline iterator begin()
        {
            return iterator{ words_.data(), /*isPointingToHeader:*/ true };
        }

        inline const_iterator begin() const
        {
            return const_iterator{ words_.data(), /*isPointingToHeader:*/ true };
        }

        inline const_iterator cbegin() const
        {
            return const_iterator{ words_.data(), /*isPointingToHeader:*/ true };
        }

        inline iterator end()
        {
            return iterator{ words_.data() + words_.size() };
        }

        inline const_iterator end() const
        {
            return const_iterator{ words_.data() + words_.size() };
        }

        inline const_iterator cend() const
        {
            return const_iterator{ words_.data() + words_.size() };
        }

    private:

        std::vector<value_type> words_;

};

// SPIR-V shader module view container.
class SpirvModuleView
{

    public:

        using value_type        = std::uint32_t;
        using size_type         = std::size_t;
        using const_iterator    = SpirvConstForwardIterator;

    public:

        SpirvModuleView() = default;
        SpirvModuleView(const SpirvModuleView&) = default;
        SpirvModuleView& operator = (const SpirvModuleView&) = default;
        SpirvModuleView(SpirvModuleView&&) = default;
        SpirvModuleView& operator = (SpirvModuleView&&) = default;

        SpirvModuleView(const ArrayView<value_type>& words);
        SpirvModuleView(const void* data, std::size_t size);

    public:

        // Reads the SPIR-V module header.
        SpirvResult ReadHeader(SpirvHeader& outHeader) const;

        // Returns the word offset for the specified iterator.
        std::uint32_t WordOffset(const const_iterator& iter) const;

        // Returns the container of 32-bit words.
        inline const ArrayView<value_type>& Words() const
        {
            return words_;
        }

    public:

        inline const_iterator begin() const
        {
            return const_iterator{ words_.data(), /*isPointingToHeader:*/ true };
        }

        inline const_iterator cbegin() const
        {
            return const_iterator{ words_.data(), /*isPointingToHeader:*/ true };
        }

        inline const_iterator end() const
        {
            return const_iterator{ words_.data() + words_.size() };
        }

        inline const_iterator cend() const
        {
            return const_iterator{ words_.data() + words_.size() };
        }

    private:

        ArrayView<value_type> words_;

};


} // /namespace LLGL


#endif



// ================================================================================
