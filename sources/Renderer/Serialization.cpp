/*
 * Serialization.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Serialization.h"
#include "../Core/Assertion.h"
#include "../Core/Helper.h"


namespace LLGL
{

namespace Serialization
{


static const std::size_t g_segmentHeaderSize = (sizeof(IdentType) + sizeof(SizeType));

/*
 * Serializer class
 */

void Serializer::Reserve(std::size_t size)
{
    data_.reserve(size);
}

void Serializer::Begin(IdentType ident, std::size_t preallocatedSize)
{
    /* Resize serialization buffer and store offset to begin of new segment */
    begin_ = data_.size();
    data_.resize(begin_ + g_segmentHeaderSize + preallocatedSize);

    /* Write identifier number */
    *reinterpret_cast<IdentType*>(&(data_[begin_])) = ident;

    /* Set write position to begin of data block */
    pos_ = begin_ + g_segmentHeaderSize;
}

void Serializer::Write(const void* data, std::size_t size)
{
    /* Resize serialization buffer on demand */
    if (pos_ + size > data_.size())
        data_.resize(pos_ + size);

    /* Copy data into serialization buffer */
    ::memcpy(&(data_[pos_]), data, size);

    /* Increment write position */
    pos_ += size;
}

void Serializer::WriteCString(const char* str)
{
    Write(str, ::strlen(str) + 1);
}

void Serializer::End()
{
    /* Store segment size */
    const auto size = (pos_ - begin_ - g_segmentHeaderSize);
    *reinterpret_cast<SizeType*>(&(data_[begin_ + sizeof(IdentType)])) = size;
}

void Serializer::WriteSegment(IdentType ident, const void* data, std::size_t size)
{
    Begin(ident, size);
    Write(data, size);
    End();
}

std::unique_ptr<Blob> Serializer::Finalize()
{
    if (!data_.empty())
    {
        /* Move serialization buffer to blob and reset offsets */
        begin_  = 0;
        pos_    = 0;
        return Blob::CreateStrongRef(std::move(data_));
    }
    return nullptr;
}


/*
 * Deserializer class
 */

Deserializer::Deserializer(const Blob& blob) :
    data_ { reinterpret_cast<const std::int8_t*>(blob.GetData()) },
    size_ { blob.GetSize()                                       }
{
}

Deserializer::Deserializer(const void* data, std::size_t size) :
    data_ { reinterpret_cast<const std::int8_t*>(data) },
    size_ { size                                       }
{
}

void Deserializer::Reset()
{
    pos_ = 0;
}

Segment Deserializer::Begin()
{
    if (pos_ + g_segmentHeaderSize >= size_)
        return {};

    /* Read segment header */
    Segment seg;
    seg.ident   = *reinterpret_cast<const IdentType*>(data_ + pos_);
    seg.size    = *reinterpret_cast<const SizeType*>(data_ + pos_ + sizeof(IdentType));

    /* Set new reading position and end of segment */
    pos_ += g_segmentHeaderSize;
    segmentEnd_ = pos_ + seg.size;

    /* Return pointer to data segment */
    if (seg.size > 0)
        seg.data = (data_ + pos_);
    else
        seg.data = nullptr;

    return seg;
}

Segment Deserializer::Begin(IdentType ident)
{
    auto seg = Begin();
    if (seg.ident != ident)
    {
        throw std::runtime_error(
            "mismatch in serialization segment identifier: read 0x" +
            ToHex(seg.ident) + ", but expected 0x" + ToHex(ident)
        );
    }
    return seg;
}

Segment Deserializer::BeginOnMatch(IdentType ident)
{
    const auto prevPos = pos_;
    auto seg = Begin();
    if (seg.ident != ident)
    {
        pos_ = prevPos;
        return {};
    }
    return seg;
}

void Deserializer::Read(void* data, std::size_t size)
{
    /* Out of bounds check */
    if (pos_ + size > segmentEnd_)
        throw std::out_of_range("reading position out of bounds in serialization segment");

    /* Copy segment data into output buffer */
    ::memcpy(data, (data_ + pos_), size);

    /* Increase reading position */
    pos_ += size;
}

const char* Deserializer::ReadCString()
{
    /* Determine string length and validate segment boundary */
    std::size_t len = 0;
    while (data_[pos_ + len] != '\0')
    {
        ++len;
        if (pos_ + len > segmentEnd_)
            throw std::out_of_range("null terminated string out of bounds in serialization segment");
    }

    /* Return string and increment reading position */
    auto str = reinterpret_cast<const char*>(&(data_[pos_]));
    pos_ += (len + 1);
    return str;
}

void Deserializer::End()
{
    /* Set reading position to end of segment */
    pos_ = segmentEnd_;
}

Segment Deserializer::ReadSegment()
{
    auto seg = Begin();
    End();
    return seg;
}

Segment Deserializer::ReadSegment(IdentType ident)
{
    auto seg = Begin(ident);
    End();
    return seg;
}

Segment Deserializer::ReadSegmentOnMatch(IdentType ident)
{
    auto seg = BeginOnMatch(ident);
    if (seg.ident == ident)
    {
        End();
        return seg;
    }
    return {};
}

void Deserializer::ReadSegment(IdentType ident, void* data, std::size_t size)
{
    Begin(ident);
    Read(data, size);
    End();
}


} // /namespace Serialization

} // /namespace LLGL



// ================================================================================
