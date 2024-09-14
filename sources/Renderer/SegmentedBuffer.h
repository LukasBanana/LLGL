/*
 * SegmentedBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_SEGMENTED_BUFFER_H
#define LLGL_SEGMENTED_BUFFER_H


#include "../Core/Assertion.h"
#include "../Core/CoreUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <string.h>
#include <vector>
#include <type_traits>
#include <cstdint>
#include <limits.h>


namespace LLGL
{


/*
Helper function to consolidate consecutive indices into segments. Returns the number of sequences.
For example, the sequence {1,2,3,5,6,8} will be consolidated to the segments {1,2,3}, {5,6}, and {8} with a return value of 3.
*/
template <typename TSizeType, typename TIter, typename SegmentCallback, typename Predicate>
TSizeType ConsolidateConsecutiveSequences(
    TIter           first,
    TIter           last,
    SegmentCallback segmentCallback,
    Predicate       predicate)
{
    static_assert(std::is_integral<TSizeType>::value, "ConsolidateIndexSegments<TSizeType,,,>: TSizeType must be an integral type");

    TSizeType numSequences = 0;

    if (first != last)
    {
        /* Initialize iterators for sub-ranges of input bindings */
        TIter       prev    = first;
        TIter       curr    = first;
        TSizeType   count   = 0;

        for (++curr, ++count; curr != last; ++curr, ++count)
        {
            if (predicate(*curr) > predicate(*prev) + 1)
            {
                /* Build next segment */
                segmentCallback(first, count);
                ++numSequences;
                count = 0;
                first = curr;
            }
            prev = curr;
        }

        if (first != last)
        {
            /* Add last segment */
            segmentCallback(first, count);
            ++numSequences;
        }
    }

    return numSequences;
}

// Helper class to simplify allocations of SegmentedBuffer.
template <typename TSegmentHeader, typename TBaseType = std::uintptr_t>
class SegmentedBufferAllocator
{

    public:

        SegmentedBufferAllocator(const SegmentedBufferAllocator&) = delete;
        SegmentedBufferAllocator& operator = (const SegmentedBufferAllocator&) = delete;

        SegmentedBufferAllocator(SegmentedBufferAllocator&&) = default;
        SegmentedBufferAllocator& operator = (SegmentedBufferAllocator&&) = default;

        // Constructs the allocator with a buffer and segment size.
        inline SegmentedBufferAllocator(std::vector<TBaseType>& buffer, std::size_t payloadSize) :
            buffer_ { buffer                                                                  },
            offset_ { buffer.size() * sizeof(TBaseType)                                       },
            size_   { GetAlignedSize(payloadSize + sizeof(TSegmentHeader), sizeof(TBaseType)) }
        {
            LLGL_ASSERT(size_ < USHRT_MAX);
            buffer_.resize(DivideRoundUp(offset_ + size_, sizeof(TBaseType)));
        }

        // Returns the segment header.
        inline TSegmentHeader* Header()
        {
            return reinterpret_cast<TSegmentHeader*>(Data() + offset_);
        }

        // Returns the segment body at the specified offset.
        template <typename T>
        inline T* Payload(std::size_t offset)
        {
            return reinterpret_cast<T*>(Data() + offset_ + sizeof(TSegmentHeader) + offset);
        }

        // Returns the total size (in bytes) for this segment allocation, i.e. payload + header size.
        inline std::uint32_t Size() const
        {
            return static_cast<std::uint32_t>(size_);
        }

        // Returns the start offset for the payload.
        inline std::uint32_t PayloadOffset() const
        {
            return sizeof(TSegmentHeader);
        }

    private:

        char* Data()
        {
            return reinterpret_cast<char*>(buffer_.data());
        }

        const char* Data() const
        {
            return reinterpret_cast<const char*>(buffer_.data());
        }

    private:

        std::vector<TBaseType>& buffer_;
        std::size_t             offset_;
        std::size_t             size_;

};

// Container class to store a raw buffer with internal (opaque) segmentation.
class SegmentedBuffer
{

    public:

        // Basic type for the internal buffer, used for alignment.
        using value_type = std::uintptr_t;

    public:

        // Allocates a new segment.
        template <typename TSegmentHeader>
        SegmentedBufferAllocator<TSegmentHeader, value_type> AllocSegment(std::size_t payloadSize)
        {
            return SegmentedBufferAllocator<TSegmentHeader, value_type>{ buffer_, payloadSize };
        }

        // Finalizes the segments and duplicates them for the specified number of sets.
        inline void FinalizeSegments(std::size_t numSegmentSets)
        {
            LLGL_ASSERT(stride_ == 0);
            LLGL_ASSERT(numSegmentSets > 0);
            stride_         = Size();
            payloadOffset_  = Size() * numSegmentSets;
            buffer_.resize(DivideRoundUp(payloadOffset_, sizeof(value_type)));
            for_subrange(i, 1, numSegmentSets)
                ::memcpy(Data() + stride_ * i, Data(), stride_);
        }

        // Appends the specified payload data at the end of this buffer.
        inline void AppendPayload(const void* data, std::size_t size)
        {
            LLGL_ASSERT(stride_ != 0);
            const std::size_t startOffset = Size();
            buffer_.resize(DivideRoundUp(startOffset + size, sizeof(value_type)));
            ::memcpy(Data() + startOffset, data, size);
        }

        // Returns the size (in bytes) of the entire buffer.
        inline std::size_t Size() const
        {
            return buffer_.size() * sizeof(value_type);
        }

        // Returns the stride (in bytes).
        inline std::size_t Stride() const
        {
            return stride_;
        }

        // Returns the offset (in bytes) to the start of the payload after all segment data.
        inline std::size_t PayloadOffset() const
        {
            return payloadOffset_;
        }

        // Returns the number of segment sets
        inline std::size_t NumSets() const
        {
            return (stride_ > 0 ? payloadOffset_ / stride_ : 0);
        }

        inline char* Data()
        {
            return reinterpret_cast<char*>(buffer_.data());
        }

        inline const char* Data() const
        {
            return reinterpret_cast<const char*>(buffer_.data());
        }

        inline char* SegmentData(std::size_t index)
        {
            return (Data() + Stride() * index);
        }

        inline const char* SegmentData(std::size_t index) const
        {
            return (Data() + Stride() * index);
        }

        inline char* PayloadData()
        {
            return (Data() + payloadOffset_);
        }

        inline const char* PayloadData() const
        {
            return (Data() + payloadOffset_);
        }

    private:

        std::size_t             stride_         = 0;
        std::size_t             payloadOffset_  = 0;
        std::vector<value_type> buffer_;

};


} // /namespace LLGL


#endif



// ================================================================================
