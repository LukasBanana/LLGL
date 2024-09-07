/*
 * VirtualCommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VIRTUAL_COMMAND_BUFFER_H
#define LLGL_VIRTUAL_COMMAND_BUFFER_H


#include "../Core/Assertion.h"
#include "../Core/CoreUtils.h"
#include <cstddef>
#include <algorithm>
#include <iterator>
#include <limits>
#include <string.h>


namespace LLGL
{


// The default policy to grow a buffer is to double the previous size.
struct DefaultBufferGrowPolicy
{
    static inline std::size_t MinChunkCapacity()
    {
        return 8192u;
    }
    static inline std::size_t NextChunkCapacity(std::size_t currentChunkCapacity)
    {
        return (std::max)(currentChunkCapacity + currentChunkCapacity/2, MinChunkCapacity());
    }
};

// Container class to manage the memory for virtual command buffers.
template <typename TOpcode, typename TGrowPolicy = DefaultBufferGrowPolicy>
class VirtualCommandBuffer
{

    private:

        // POD structure for the memory chunks. These chunks contain a payload at the end of the struct.
        struct Chunk
        {
            std::size_t capacity;
            std::size_t size;
            Chunk*      next;
            // <payload>
        };

    public:

        using AlignOffsetType = std::uint8_t;

        // View structure for a chunk iterator.
        struct ChunkPayloadView
        {
            const char* data;
            std::size_t size;
        };

    public:

        // Constant iterator for the virtual command buffer chunks. A packed virtual command buffer has only one chunk.
        class ChunkIterator
        {

            public:

                using iterator_category = std::forward_iterator_tag;
                using difference_type   = std::size_t;
                using value_type        = const ChunkPayloadView;
                using pointer           = const ChunkPayloadView*;
                using reference         = const ChunkPayloadView&;

            public:

                ChunkIterator() = default;
                ChunkIterator(const ChunkIterator&) = default;
                ChunkIterator& operator = (const ChunkIterator&) = default;

            public:

                bool operator == (const ChunkIterator& rhs) const
                {
                    return (chunk_ == rhs.chunk_);
                }

                bool operator != (const ChunkIterator& rhs) const
                {
                    return (chunk_ != rhs.chunk_);
                }

                ChunkIterator& operator ++ ()
                {
                    chunk_ = chunk_->next;
                    return *this;
                }

                ChunkIterator operator ++ (int)
                {
                    ChunkIterator result{ chunk_ };
                    chunk_ = chunk_->next;
                    return result;
                }

                value_type operator * () const
                {
                    return value_type{ VirtualCommandBuffer::GetChunkData(chunk_), chunk_->size };
                }

            private:

                ChunkIterator(const Chunk* chunk) :
                    chunk_ { chunk }
                {
                }

            private:

                friend class VirtualCommandBuffer;

                const Chunk* chunk_ = nullptr;

        };

    public:

        VirtualCommandBuffer() = default;

        VirtualCommandBuffer(const VirtualCommandBuffer&) = delete;
        VirtualCommandBuffer& operator = (const VirtualCommandBuffer&) = delete;

        // Takes the ownership of the specified virtual command buffer memory.
        VirtualCommandBuffer(VirtualCommandBuffer&& rhs)
        {
            std::swap(first_, rhs.first_);
            std::swap(current_, rhs.current_);
            std::swap(size_, rhs.size_);
        }

        // Takes the ownership of the specified virtual command buffer memory.
        VirtualCommandBuffer& operator = (VirtualCommandBuffer&& rhs)
        {
            std::swap(first_, rhs.first_);
            std::swap(current_, rhs.current_);
            std::swap(size_, rhs.size_);
            return *this;
        }

        // Initializes the virtual command buffer with the specified size (in bytes).
        VirtualCommandBuffer(std::size_t initialCapacity) :
            initialCapacity_ { std::max(TGrowPolicy::MinChunkCapacity(), initialCapacity) }
        {
        }

        // Deletes all memory chunks of this command buffer.
        ~VirtualCommandBuffer()
        {
            Release();
        }

        // Returns the total capacity (in bytes) of this virtual command buffer.
        std::size_t Capacity() const
        {
            return capacity_;
        }

        // Returns the total size (in bytes) of used memory of this virtual command buffer.
        std::size_t Size() const
        {
            return size_;
        }

        // Returns true if this virtual command buffer is empty, i.e. Size() returns zero.
        bool Empty() const
        {
            return (Size() == 0);
        }

        // Clears the container but keeps the allocated capacity.
        void Clear()
        {
            if (!Empty())
            {
                for (Chunk* c = first_; c != nullptr; c = c->next)
                    c->size = 0;
                current_    = first_;
                size_       = 0;
            }
        }

        // Deletes all memory chunks.
        void Release()
        {
            for (Chunk* c = first_, *next = nullptr; c != nullptr; c = next)
            {
                next = c->next;
                VirtualCommandBuffer::FreeChunk(c);
            }
            first_      = nullptr;
            current_    = nullptr;
            biggest_    = nullptr;
            capacity_   = 0;
            size_       = 0;
        }

        // Packs the entire buffer to one consecutive memory block.
        void Pack()
        {
#if 0 //TODO: disabled until preservation of memory alignment can be ensured
            /* Only pack if there is more than one memory chunk */
            if (first_ != nullptr && first_->next != nullptr)
            {
                /* Check if last (and biggest) chunk is big enough */
                Chunk* biggest = FindBiggestChunk();
                if (biggest != nullptr && biggest->capacity >= size_)
                    PackRecycle(biggest);
                else
                    PackNew();
            }
#endif
        }

        // Allocates a new opcode in this virtual command buffer.
        void AllocOpcode(const TOpcode opcode)
        {
            (void)AllocAlignedDataWithOpcode(opcode);
        }

        // Allocates a new command with the specified opcode and optional payload (in bytes).
        template <typename TCommand>
        TCommand* AllocCommand(const TOpcode opcode, std::size_t payloadSize = 0)
        {
            return reinterpret_cast<TCommand*>(AllocAlignedDataWithOpcode(opcode, sizeof(TCommand) + payloadSize, alignof(TCommand)));
        }

        // Runs the input function over every command in this virtual command buffer.
        // The function callback must return the size (in bytes) of the command being processed.
        template <typename Functor, typename... TArgs>
        void Run(Functor func, TArgs&&... args) const
        {
            for (const ChunkPayloadView& chunk : *this)
            {
                const char* pc      = chunk.data;
                const char* pcEnd   = chunk.data + chunk.size;

                while (pc < pcEnd)
                {
                    /* Read alignment offset */
                    const AlignOffsetType offset = *reinterpret_cast<const AlignOffsetType*>(pc);
                    pc += sizeof(AlignOffsetType);
                    pc += offset;

                    /* Read opcode */
                    const TOpcode opcode = reinterpret_cast<const TOpcode*>(pc)[-1];

                    /* Execute command and increment program counter */
                    pc += func(opcode, pc, std::forward<TArgs&&>(args)...);
                }
            }
        }

    private:

        // Allocates a new memory chunk of the specified capacity plus sizeof(Chunk).
        static Chunk* AllocChunk(std::size_t capacity, Chunk* next = nullptr)
        {
            Chunk* chunk = reinterpret_cast<Chunk*>(::new char[sizeof(Chunk) + capacity]);
            {
                chunk->capacity = capacity;
                chunk->size     = 0;
                chunk->next     = next;
            }
            return chunk;
        }

        // Deletes the specified memory chunk.
        static void FreeChunk(Chunk* chunk)
        {
            if (chunk != nullptr)
            {
                char* buf = reinterpret_cast<char*>(chunk);
                delete [] buf;
            }
        }

        // Returns a raw pointer to the beginning of the chunk data.
        static char* GetChunkData(Chunk* chunk)
        {
            return reinterpret_cast<char*>(chunk + 1);
        }

        // Returns a raw pointer to the beginning of the chunk data.
        static const char* GetChunkData(const Chunk* chunk)
        {
            return reinterpret_cast<const char*>(chunk + 1);
        }

    public:

        // STL compatible function to return the constant iterator to the first memory chunk.
        ChunkIterator cbegin() const
        {
            return ChunkIterator{ first_ };
        }

        // STL compatible function to return the iterator to the first memory chunk.
        ChunkIterator begin() const
        {
            return cbegin();
        }

        // STL compatible function to return the constant iterator to the end if the memory chunks.
        ChunkIterator cend() const
        {
            return ChunkIterator{ nullptr };
        }

        // STL compatible function to return the iterator to the end if the memory chunks.
        ChunkIterator end() const
        {
            return cend();
        }

    private:

        // Returns whether the specified byte size fits into the current chunk.
        bool FitsIntoCurrentChunk(std::size_t size) const
        {
            return (current_ != nullptr && current_->size + size <= current_->capacity);
        }

        // Allocates a new chunk and makes it the current one.
        void AllocNextChunkAndMakeCurrent(std::size_t capacity, Chunk* next = nullptr)
        {
            current_->next = VirtualCommandBuffer::AllocChunk(capacity, next);
            current_ = current_->next;
            capacity_ += capacity;
            if (biggest_ == nullptr || capacity > biggest_->capacity)
                biggest_ = current_;
        }

        // Allocates the next chunk.
        void AllocNextChunk(std::size_t capacity)
        {
            if (current_ != nullptr)
            {
                /* Is there already a next chunk? */
                if (current_->next != nullptr)
                {
                    /* Check if old chunk can be re-used */
                    if (current_->next->capacity >= capacity)
                    {
                        /* Re-use old chunk */
                        current_ = current_->next;
                    }
                    else
                    {
                        /* Delete old chunk, allocate a new one, and re-assign reference to biggest chunk even if second-next is NULL */
                        Chunk* secondNext = current_->next->next;
                        if (biggest_ == current_->next)
                            biggest_ = secondNext;
                        VirtualCommandBuffer::FreeChunk(current_->next);
                        AllocNextChunkAndMakeCurrent(capacity, secondNext);
                    }
                }
                else
                {
                    /* Allocate new chunk and move pointer to the new chunk */
                    AllocNextChunkAndMakeCurrent(capacity);
                }
            }
            else
            {
                /* Allocate first chunk */
                first_      = VirtualCommandBuffer::AllocChunk(capacity);
                current_    = first_;
                biggest_    = first_;
                capacity_   = capacity;
            }
        }

        // Returns the default capacity for the next chunk.
        std::size_t NextCapacity() const
        {
            if (size_ == 0)
                return initialCapacity_;
            else
                return TGrowPolicy::NextChunkCapacity(current_->capacity);
        }

        // Allocates a data block of the specified size.
        // @param size: Specifies the size of data block.
        // @param alignment: Specifies the memory alignment.
        // @param headerSize: Specifies extra data to be allocated in front of the returned data block, excluded from the memory alignment.
        char* AllocData(std::size_t size, std::size_t alignment = 0, std::size_t headerSize = 0)
        {
            /* Allocate one extra byte to store offset for alignment */
            size += sizeof(AlignOffsetType) + headerSize;

            /* Check if new data fits into current chunk */
            const std::size_t estimatedPadding = (alignment > 0 ? alignment - 1 : 0);
            const std::size_t sizeWithEstimatedPadding = size + estimatedPadding;

            if (!FitsIntoCurrentChunk(sizeWithEstimatedPadding))
                AllocNextChunk(std::max(NextCapacity(), sizeWithEstimatedPadding));

            /* Allocate new data block within current chunk and apply memory alignment */
            char* data = VirtualCommandBuffer::GetChunkData(current_) + current_->size;
            data += sizeof(AlignOffsetType);

            const std::uintptr_t basePtr = reinterpret_cast<std::uintptr_t>(data);
            const std::uintptr_t offset = GetAlignedSize(basePtr + headerSize, static_cast<std::uintptr_t>(alignment)) - basePtr;

            /* Write offset for alignment at first byte in new data block */
            constexpr std::uintptr_t maxAlignmentOffset = std::numeric_limits<AlignOffsetType>::max();
            LLGL_ASSERT(offset <= maxAlignmentOffset);

            *reinterpret_cast<AlignOffsetType*>(data - sizeof(AlignOffsetType)) = static_cast<AlignOffsetType>(offset);

            /* Now shift return pointer to aligned offset minus the input alignment shift (for opcode) */
            data += offset;
            size += offset - headerSize;

            /* Keep track of chunk size */
            current_->size += size;
            LLGL_ASSERT(current_->size <= current_->capacity);
            size_ += size;
            return data;
        }

        // Allocates a data block with alignment and adds the opcode *before* the aligned payload,
        // i.e. only the payload data will be aligned since the opcode is usually only 1 byte in size.
        char* AllocAlignedDataWithOpcode(const TOpcode opcode, std::size_t payloadSize = 0, std::size_t alignment = 0)
        {
            /* Allocate data with extra space for alignment */
            char* data = AllocData(payloadSize, alignment, sizeof(TOpcode));
            reinterpret_cast<TOpcode*>(data)[-1] = opcode;
            return data;
        }

        // Returns the biggest memory chunk.
        Chunk* FindBiggestChunk() const
        {
            if (biggest_ == nullptr && first_ != nullptr)
            {
                Chunk* result = first_;
                for (Chunk* c = first_->next; c != nullptr; c = c->next)
                {
                    if (result->capacity < c->capacity)
                        result = c;
                }
                return result;
            }
            return biggest_;
        }

        // Packs the entire virtual command buffer into the specified memory chunk.
        void PackRecycle(Chunk* chunk)
        {
            LLGL_ASSERT_PTR(chunk);

            /* Determine offset to move existing memory within recycled chunk */
            std::size_t offset = 0;
            for (Chunk* c = first_; c != nullptr; c = c->next)
            {
                if (c == chunk)
                    break;
                offset += c->size;
            }

            /* Move memory within recycled chunk */
            ::memmove(VirtualCommandBuffer::GetChunkData(chunk) + offset, VirtualCommandBuffer::GetChunkData(chunk), chunk->size);

            /* Copy other chunks into this chunk and free old chunks */
            offset = 0;
            for (Chunk* c = first_, *next = nullptr; c != nullptr; c = next)
            {
                if (c != chunk)
                {
                    ::memcpy(VirtualCommandBuffer::GetChunkData(chunk) + offset, VirtualCommandBuffer::GetChunkData(c), c->size);
                    offset += c->size;
                    next = c->next;
                    VirtualCommandBuffer::FreeChunk(c);
                }
                else
                {
                    offset += c->size;
                    next = c->next;
                }
            }
            chunk->size = offset;

            /* Clean up references */
            first_      = chunk;
            current_    = chunk;
            biggest_    = chunk;
            capacity_   = chunk->size;
        }

        // Packs the entire virtual command buffer into a new single memory chunk.
        void PackNew()
        {
            /* Allocate new chunk */
            Chunk* chunk = VirtualCommandBuffer::AllocChunk(size_);

            /* Copy all chunks into new chunk and free old chunks */
            for (Chunk* c = first_, *next = nullptr; c != nullptr; c = next)
            {
                /* Copy old chunk into new at current offset */
                ::memcpy(VirtualCommandBuffer::GetChunkData(chunk) + chunk->size, VirtualCommandBuffer::GetChunkData(c), c->size);
                chunk->size += c->size;

                /* Delete old chunk and move to next one */
                next = c->next;
                VirtualCommandBuffer::FreeChunk(c);
            }

            /* Clean up references */
            first_      = chunk;
            current_    = chunk;
            biggest_    = chunk;
            capacity_   = chunk->size;
        }

    private:

        Chunk*      first_              = nullptr;
        Chunk*      current_            = nullptr;
        Chunk*      biggest_            = nullptr; // Keep track of biggest chunk for packing
        std::size_t capacity_           = 0;
        std::size_t size_               = 0;
        std::size_t initialCapacity_    = TGrowPolicy::MinChunkCapacity();

};


} // /namespace LLGL


#endif



// ================================================================================
