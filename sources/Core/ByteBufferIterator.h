/*
 * ByteBufferIterator.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_BYTE_BUFFER_ITERATOR_H
#define LLGL_BYTE_BUFFER_ITERATOR_H


namespace LLGL
{


// Helper class to iterate over a byte aligned buffer that is casted to structured types.
class ByteBufferIterator
{

    public:

        ByteBufferIterator() = default;
        ByteBufferIterator(const ByteBufferIterator&) = default;
        ByteBufferIterator& operator = (const ByteBufferIterator&) = default;

        // Initializes the byte buffer.
        inline ByteBufferIterator(char* byteBuffer) :
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
