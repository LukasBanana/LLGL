/*
 * Blob.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Blob.h>
#include <LLGL/ImageFlags.h>
#include <fstream>
#include "Helper.h"


namespace LLGL
{


/*
 * BlobManaged class
 */

// Managed implementation of <Blob> interface.
class BlobManaged final : public Blob
{

    public:

        BlobManaged(const void* data, std::size_t size);
        BlobManaged(ByteBuffer&& data, std::size_t size);

    public:

        const void* GetData() const override;
        std::size_t GetSize() const override;

    private:

        ByteBuffer  data_;
        std::size_t size_ = 0;

};

BlobManaged::BlobManaged(const void* data, std::size_t size) :
    data_ { GenerateEmptyByteBuffer(size, false) },
    size_ { size                                 }
{
    ::memcpy(data_.get(), data, size);
}

BlobManaged::BlobManaged(ByteBuffer&& data, std::size_t size) :
    data_ { std::move(data) },
    size_ { size            }
{
}

const void* BlobManaged::GetData() const
{
    return data_.get();
}

std::size_t BlobManaged::GetSize() const
{
    return size_;
}


/*
 * BlobUnmanaged class
 */

// Unmanaged implementation of <Blob> interface.
class BlobUnmanaged final : public Blob
{

    public:

        BlobUnmanaged(const void* data, std::size_t size);

    public:

        const void* GetData() const override;
        std::size_t GetSize() const override;

    private:

        const void* data_ = nullptr;
        std::size_t size_ = 0;

};

BlobUnmanaged::BlobUnmanaged(const void* data, std::size_t size) :
    data_ { data },
    size_ { size }
{
}

const void* BlobUnmanaged::GetData() const
{
    return data_;
}

std::size_t BlobUnmanaged::GetSize() const
{
    return size_;
}


/*
 * BlobContainer class
 */

// Container wrapper implementation of <Blob> interface.
template <typename T>
class BlobContainer final : public Blob
{

    public:

        explicit BlobContainer(T&& container);

    public:

        const void* GetData() const override;
        std::size_t GetSize() const override;

    private:

        T container_;

};

template <typename T>
BlobContainer<T>::BlobContainer(T&& container) :
    container_ { std::move(container) }
{
}

template <typename T>
const void* BlobContainer<T>::GetData() const
{
    return container_.data();
}

template <typename T>
std::size_t BlobContainer<T>::GetSize() const
{
    return container_.size() * sizeof(typename T::value_type);
}

using BlobStdVectorInt8 = BlobContainer<std::vector<std::int8_t>>;
using BlobStdString     = BlobContainer<std::string>;


/*
 * Blob class
 */

std::unique_ptr<Blob> Blob::CreateCopy(const void* data, std::size_t size)
{
    return MakeUnique<BlobManaged>(data, size);
}

std::unique_ptr<Blob> Blob::CreateWeakRef(const void* data, std::size_t size)
{
    return MakeUnique<BlobUnmanaged>(data, size);
}

std::unique_ptr<Blob> Blob::CreateStrongRef(std::vector<std::int8_t>&& container)
{
    return MakeUnique<BlobStdVectorInt8>(std::forward<std::vector<std::int8_t>>(container));
}

std::unique_ptr<Blob> Blob::CreateStrongRef(std::string&& container)
{
    return MakeUnique<BlobStdString>(std::forward<std::string>(container));
}

std::unique_ptr<Blob> Blob::CreateFromFile(const char* filename)
{
    if (filename == nullptr || *filename == '\0')
        return nullptr;

    /* Read file as binary */
    std::ifstream file{ filename, std::ios::in | std::ios::binary };
    if (!file.good())
        return nullptr;

    /* Determine file size */
    file.seekg(0, std::ios::end);
    const auto fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    /* Read binary file into container */
    auto buffer = GenerateEmptyByteBuffer(static_cast<std::size_t>(fileSize), false);
    file.read(buffer.get(), fileSize);

    /* Return blob that manages the read buffer */
    return MakeUnique<BlobManaged>(std::move(buffer), static_cast<std::size_t>(fileSize));
}

std::unique_ptr<Blob> Blob::CreateFromFile(const std::string& filename)
{
    return CreateFromFile(filename.c_str());
}


} // /namespace LLGL



// ================================================================================
