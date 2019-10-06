/*
 * Serialization.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SERIALIZATION_H
#define LLGL_SERIALIZATION_H


#include <LLGL/Blob.h>
#include <cstdint>
#include <vector>
#include <type_traits>


namespace LLGL
{

namespace Serialization
{


// Segment identifier type for a serialization segment.
using IdentType = std::uint16_t;

// Size type for a serialization segment.
using SizeType = std::size_t;


/* ----- Structures ----- */

/*
Example structure of a serialization blob on a 32-bit system:

Offset      Header
0x00000000  |-segments[0].ident         = <renderer-specific>
0x00000002  |-segments[0].size          = 8
0x00000006  |-segments[0].data[0..7]
0x0000000E  |-segments[1].ident         = <renderer-specific>
0x00000010  |-segments[1].size          = 4
0x00000014  |-segments[1].data[0..3]
0x00000018  `-END                       = 0
*/

// Segemnt structure for serialization blobs.
struct Segment
{
    // Segment identifier number (renderer specific). A magic number of zero terminates the list of segments.
    IdentType           ident;

    /*
    Size (in bytes) of the data segment (excluding this size field). Use 'data + size' to jump to the next segment.
    If this is USHORT_MAX, the remaining data is stored in the next segment.
    */
    SizeType            size;

    // Pointer to the segment data.
    const std::int8_t*  data;
};


/* ----- Classes ----- */

/*
Reading example:
\code
Serialization::Serializer   serial;
Serialization::Segment      seg = {};

seg = serial.ReadSegment(initialData, initialSize);
if (seg.magic == magicSeg0)
    initialSize -= seg.size;
else
    return;

// ...

seg = serial.ReadSegment(seg.data + seg.size, initialSize);
if (seg.magic == magicSeg1)
    initialSize -= seg.size;
else
    return;
\endcode
*/
class LLGL_EXPORT Serializer
{

    public:

        // Reserves the specified size (in bytes) for data serialization.
        void Reserve(std::size_t size);

        // Begins a new segment with the specified magic number
        void Begin(IdentType ident, std::size_t preallocatedSize = 0);

        // Writes the next part of the current segment.
        void Write(const void* data, std::size_t size);

        // Writes the next part of the current segment as null terminator string.
        void WriteCString(const char* str);

        // Ends the current segment.
        void End();

        // Writes the next segment at once, i.e. calls Begin, Write, and End.
        void WriteSegment(IdentType ident, const void* data, std::size_t size);

        // Returns the final blob of the serialized data. A new serialization can be created after this call.
        std::unique_ptr<Blob> Finalize();

    public:

        // Writes the next part of the current segment as templated version.
        template <typename T>
        void WriteTyped(const T& data)
        {
            static_assert(!std::is_pointer<T>::value, "LLGL::Serialization::Serializer::WriteTyped<T> does not accept pointer types");
            static_assert(std::is_standard_layout<T>::value, "LLGL::Serialization::Serializer::WriteTyped<T> only accepts standard layout types");
            Write(&data, sizeof(data));
        }

    private:

        std::vector<std::int8_t>    data_;
        std::size_t                 begin_  = 0;
        std::size_t                 pos_    = 0;

};

// Deserializer class for reading serialized data.
class LLGL_EXPORT Deserializer
{

    public:

        Deserializer() = default;
        Deserializer(const Deserializer&) = default;
        Deserializer& operator = (const Deserializer&) = default;

        Deserializer(const Blob& blob);
        Deserializer(const void* data, std::size_t size);

    public:

        // Resets the reading position to the begin.
        void Reset();

        // Reads the next segment header.
        Segment Begin();

        // Reads the next segment header or throws an error if the segment does not match the specified identifier.
        Segment Begin(IdentType ident);

        // Reads the next segment header if the identifiers match. Otherwise, the reading position is not modified.
        Segment BeginOnMatch(IdentType ident);

        // Reads the next data part of the current segment.
        void Read(void* data, std::size_t size);

        // Reads a null terminated string from the current segment.
        const char* ReadCString();

        // Fast forwards to the end of the current segment.
        void End();

        // Reads the header of the next segment and fast forwards to the end of that segment.
        Segment ReadSegment();

        /*
        Reads the header of the next segment and fast forwards to the end of that segment
        or throws an error if the segment does not match the specified identifier.
        */
        Segment ReadSegment(IdentType ident);

        // Reads the header of the next segment and fast forwards to the end of that segment if the identifiers match. Otherwise, the reading position is not modified.
        Segment ReadSegmentOnMatch(IdentType ident);

        // Reads the entire next segment into the output buffer or throws an error if the segment does not match the specified identifier or size.
        void ReadSegment(IdentType ident, void* data, std::size_t size);

    public:

        template <typename T>
        void ReadTyped(T& data)
        {
            static_assert(!std::is_pointer<T>::value, "LLGL::Serialization::Deserializer::ReadTyped<T> does not accept pointer types");
            static_assert(std::is_standard_layout<T>::value, "LLGL::Serialization::Deserializer::ReadTyped<T> only accepts standard layout types");
            Read(&data, sizeof(data));
        }

    private:

        const std::int8_t*  data_       = nullptr;
        std::size_t         size_       = 0;
        std::size_t         pos_        = 0;
        std::size_t         segmentEnd_ = 0;

};


} // /namespace Serialization

} // /namespace LLGL


#endif



// ================================================================================
