/*
 * SpirvIterator.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_SPIRV_ITERATOR_H
#define LLGL_SPIRV_ITERATOR_H


#include "SpirvInstruction.h"
#include "SpirvInstructionInfo.h"
#include <cstdint>
#include <type_traits>


namespace LLGL
{


// SPIR-V error codes.
enum class SpirvResult
{
    NoError = 0,        // No error occurred.

    InvalidModule,      // Invalid size of module; must be a multiple of 4.
    InvalidHeader,      // Invalid module header.
    OperandOutOfBounds, // Instruction does not have the correct number of operands.
    IdOutOfBounds,      // Operand ID is out of bounds.
    IdTypeMismatch,     // Operand ID does not match with type.
};

// SPIR-V shader module header structure.
struct SpirvHeader
{
    std::uint32_t spirvMagic;
    std::uint32_t spirvVersion;
    std::uint32_t builderMagic;
    std::uint32_t idBound;
    std::uint32_t schema;
};

// SPIR-V instruction iterator base class.
template <typename T>
class BasicSpirvForwardIterator
{

    static_assert(std::is_integral<T>::value, "BasicSpirvForwardIterator<T>: T must be an integral type");
    static_assert(std::is_same<std::uint32_t, typename std::remove_cv<T>::type>::value, "BasicSpirvForwardIterator<T>: T must be 'uint32_t' or 'const uint32_t'");

    public:

        using value_type    = T;
        using pointer       = T*;

    public:

        BasicSpirvForwardIterator() = default;
        BasicSpirvForwardIterator(const BasicSpirvForwardIterator&) = default;
        BasicSpirvForwardIterator& operator = (const BasicSpirvForwardIterator&) = default;

        BasicSpirvForwardIterator(pointer ptr) :
            ptr_ { ptr }
        {
        }

        BasicSpirvForwardIterator(pointer ptr, bool isPointingToHeader) :
            ptr_ { (isPointingToHeader ? ptr + sizeof(SpirvHeader)/sizeof(T) : ptr) }
        {
        }

        // Returns the internal pointer of this iterator.
        pointer Words() const
        {
            return ptr_;
        }

        // Returns the number of words (32-bit values) of the instruction this iterator points to.
        std::uint32_t WordCount() const
        {
            return (ptr_[0] >> spv::WordCountShift);
        }

        // Returns the opcode of the instruction this iterator points to.
        spv::Op Opcode() const
        {
            return static_cast<spv::Op>(ptr_[0] & spv::OpCodeMask);
        }

        // Returns the raw pointer of this iterator.
        pointer Ptr() const
        {
            return ptr_;
        }

        // Returns the instruction this iterator points to.
        SpirvInstruction Get() const
        {
            return SpirvInstruction{ Words() };
        }

    public:

        BasicSpirvForwardIterator& operator ++ ()
        {
            ptr_ += WordCount();
            return *this;
        }

        BasicSpirvForwardIterator operator ++ (int)
        {
            BasicSpirvForwardIterator prev{ *this };
            this->operator++();
            return prev;
        }

        bool operator == (const BasicSpirvForwardIterator& rhs) const
        {
            return (ptr_ == rhs.ptr_);
        }

        bool operator != (const BasicSpirvForwardIterator& rhs) const
        {
            return (ptr_ != rhs.ptr_);
        }

        SpirvInstruction operator * () const
        {
            return Get();
        }

    private:

        pointer ptr_ = nullptr;

};

using SpirvForwardIterator = BasicSpirvForwardIterator<std::uint32_t>;
using SpirvConstForwardIterator = BasicSpirvForwardIterator<const std::uint32_t>;


} // /namespace LLGL


#endif



// ================================================================================
