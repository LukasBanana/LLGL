/*
 * RawBufferIterator.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RAW_BUFFER_ITERATOR_H
#define LLGL_RAW_BUFFER_ITERATOR_H


namespace LLGL
{


// Helper class to iterate over a raw buffer (or byte aligned buffer).
class RawBufferIterator
{

    public:

        RawBufferIterator() = default;
        RawBufferIterator(const RawBufferIterator&) = default;
        RawBufferIterator& operator = (const RawBufferIterator&) = default;

        // Initializes the byte buffer.
        inline RawBufferIterator(char* byteBuffer) :
            byteBuffer_ { byteBuffer }
        {
        }

        // Returns the next <T>-typed entry.
        template <typename T>
        T* Next()
        {
            auto ptr = reinterpret_cast<T*>(byteBuffer_);
            byteBuffer_ += sizeof(T);
            return ptr;
        }

        // Returns the next count <T>-typed entries.
        template <typename T>
        T* Next(std::size_t count)
        {
            auto ptr = reinterpret_cast<T*>(byteBuffer_);
            byteBuffer_ += (sizeof(T) * count);
            return ptr;
        }

        // Resets the byte buffer.
        inline void Reset(char* byteBuffer)
        {
            byteBuffer_ = byteBuffer;
        }

    private:

        char* byteBuffer_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
