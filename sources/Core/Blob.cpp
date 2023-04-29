/*
 * Blob.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Blob.h>
#include <LLGL/ImageFlags.h>
#include <fstream>
#include "CoreUtils.h"


namespace LLGL
{


/*
 * Blob::Pimpl struct
 */

struct Blob::Pimpl
{
    virtual ~Pimpl() = default;
    virtual const void* GetData() const = 0;
    virtual std::size_t GetSize() const = 0;
};

struct InternalStringBlob final : Blob::Pimpl
{
    InternalStringBlob(std::string&& str) :
        str { std::forward<std::string>(str) }
    {
    }

    const void* GetData() const override
    {
        return str.data();
    }

    std::size_t GetSize() const override
    {
        return str.size();
    }

    std::string str;
};

struct InternalVectorBlob final : Blob::Pimpl
{
    InternalVectorBlob(const void* data, std::size_t size) :
        container { reinterpret_cast<const char*>(data), reinterpret_cast<const char*>(data) + size }
    {
    }

    InternalVectorBlob(std::vector<char>&& cont) :
        container { std::forward<std::vector<char>>(cont) }
    {
    }

    const void* GetData() const override
    {
        return container.data();
    }

    std::size_t GetSize() const override
    {
        return container.size();
    }

    std::vector<char> container;
};

struct InternalBufferBlob final : Blob::Pimpl
{
    InternalBufferBlob(ByteBuffer&& buffer, std::size_t size) :
        buffer { std::forward<ByteBuffer>(buffer) },
        size   { size                             }
    {
    }

    const void* GetData() const override
    {
        return buffer.get();
    }

    std::size_t GetSize() const override
    {
        return size;
    }

    ByteBuffer  buffer;
    std::size_t size;
};

struct InternalUnmanagedBlob final : Blob::Pimpl
{
    InternalUnmanagedBlob(const void* data, std::size_t size) :
        data { data },
        size { size }
    {
    }

    const void* GetData() const override
    {
        return data;
    }

    std::size_t GetSize() const override
    {
        return size;
    }

    const void* data;
    std::size_t size;
};

static Blob::Pimpl* MakeInternalBlob(const void* data, std::size_t size, bool isWeakRef)
{
    if (isWeakRef)
        return new InternalVectorBlob{ data, size };
    else
        return new InternalUnmanagedBlob{ data, size };
}

static Blob::Pimpl* MakeInternalBlob(std::vector<char>&& cont)
{
    return new InternalVectorBlob{ std::forward<std::vector<char>>(cont) };
}

static Blob::Pimpl* MakeInternalBlob(ByteBuffer&& buf, std::size_t size)
{
    return new InternalBufferBlob{ std::forward<ByteBuffer>(buf), size };
}

static Blob::Pimpl* MakeInternalBlob(std::string&& str)
{
    return new InternalStringBlob{ std::forward<std::string>(str) };
}


/*
 * Blob class
 */

Blob::Blob() :
    pimpl_ { nullptr }
{
}

Blob::Blob(Blob&& rhs) :
    pimpl_ { rhs.pimpl_ }
{
    rhs.pimpl_ = nullptr;
}

Blob& Blob::operator = (Blob&& rhs)
{
    if (this != &rhs)
    {
        delete pimpl_;
        pimpl_ = rhs.pimpl_;
        rhs.pimpl_ = nullptr;
    }
    return *this;
}

Blob::Blob(const void* data, std::size_t size, bool isWeakRef) :
    pimpl_ { MakeInternalBlob(data, size, isWeakRef) }
{
}

Blob::~Blob()
{
    delete pimpl_;
}

Blob Blob::CreateCopy(const void* data, std::size_t size)
{
    return Blob{ data, size };
}

Blob Blob::CreateWeakRef(const void* data, std::size_t size)
{
    return Blob{ data, size, /*isWeakRef:*/ true };
}

Blob Blob::CreateStrongRef(std::vector<char>&& cont)
{
    Blob blob;
    blob.pimpl_ = MakeInternalBlob(std::forward<std::vector<char>>(cont));
    return blob;
}

Blob Blob::CreateStrongRef(std::string&& str)
{
    Blob blob;
    blob.pimpl_ = MakeInternalBlob(std::forward<std::string>(str));
    return blob;
}

Blob Blob::CreateFromFile(const char* filename)
{
    if (filename == nullptr || *filename == '\0')
        return Blob{};

    /* Read file as binary */
    std::ifstream file{ filename, std::ios::in | std::ios::binary };
    if (!file.good())
        return Blob{};

    /* Determine file size */
    file.seekg(0, std::ios::end);
    const auto fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    /* Read binary file into container */
    auto buffer = AllocateByteBuffer(static_cast<std::size_t>(fileSize), UninitializeTag{});
    file.read(buffer.get(), fileSize);

    /* Return blob that manages the read buffer */
    Blob blob;
    blob.pimpl_ = MakeInternalBlob(std::move(buffer), static_cast<std::size_t>(fileSize));
    return blob;
}

Blob Blob::CreateFromFile(const std::string& filename)
{
    return CreateFromFile(filename.c_str());
}

const void* Blob::GetData() const
{
    return (pimpl_ != nullptr ? pimpl_->GetData() : nullptr);
}

std::size_t Blob::GetSize() const
{
    return (pimpl_ != nullptr ? pimpl_->GetSize() : 0);
}

Blob::operator bool () const
{
    return (pimpl_ != nullptr && pimpl_->GetData() != nullptr && pimpl_->GetSize() > 0);
}


} // /namespace LLGL



// ================================================================================
