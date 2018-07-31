/*
 * CsBufferFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include "CsFormat.h"
#include "CsVertexFormat.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;


namespace LHermanns
{

namespace SharpLLGL
{


/* ----- Enumerations ----- */

public enum class BufferType
{
    Vertex,
    Index,
    Constant,
    Storage,
    StreamOutput,
};

public enum class StorageBufferType
{
    Undefined,
    Buffer,
    StructuredBuffer,
    ByteAddressBuffer,
    RWBuffer,
    RWStructuredBuffer,
    RWByteAddressBuffer,
    AppendStructuredBuffer,
    ConsumeStructuredBuffer,
};


/* ----- Flags ----- */

[Flags]
public enum class BufferFlags
{
    None                = 0,
    MapReadAccess       = (1 << 0),
    MapWriteAccess      = (1 << 1),
    MapReadWriteAccess  = (MapReadAccess | MapWriteAccess),
    DynamicUsage        = (1 << 2),
};


/* ----- Structures ----- */

public ref class IndexFormat
{

    public:

        IndexFormat();
        IndexFormat(IndexFormat^ rhs);
        IndexFormat(LHermanns::SharpLLGL::DataType dataType);

        property LHermanns::SharpLLGL::DataType DataType
        {
            LHermanns::SharpLLGL::DataType get();
        };

        property unsigned int FormatSize
        {
            unsigned int get();
        };

    private:

        LHermanns::SharpLLGL::DataType dataType_;

};

public ref class BufferDescriptor
{

    public:

        BufferDescriptor();

        ref class VertexBufferDescriptor
        {

            public:

                VertexBufferDescriptor();

                property VertexFormat^ Format;

        };

        ref class IndexBufferDescriptor
        {

            public:

                IndexBufferDescriptor();

                property IndexFormat^ Format;

        };

        ref class StorageBufferDescriptor
        {

            public:

                StorageBufferDescriptor();

                property StorageBufferType  StorageType;// = StorageBufferType::Buffer;
                property Format             Format;//      = Format::RGBA32Float;
                property System::UInt32     Stride;

        };

        property BufferType                 Type;
        property BufferFlags                Flags;
        property System::UInt64             Size;
        property VertexBufferDescriptor^    VertexBuffer;
        property IndexBufferDescriptor^     IndexBuffer;
        property StorageBufferDescriptor^   StorageBuffer;
};


} // /namespace SharpLLGL

} // /namespace LHermanns



// ================================================================================
