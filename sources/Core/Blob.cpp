/*
 * Blob.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Blob.h>
#include <fstream>
#include "CoreUtils.h"
#include "Assertion.h"
#include "../Platform/Path.h"


#include <LLGL/Platform/Platform.h>

#if LLGL_OS_ANDROID

#include "../Platform/Android/AndroidApp.h"

#ifndef __USE_BSD
#define __USE_BSD /* Defines funopen() */
#endif

#include <stdio.h>
#include <android/asset_manager.h>

#endif // /LLGL_OS_ANDROID


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
        container { static_cast<const char*>(data), static_cast<const char*>(data) + size }
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
    InternalBufferBlob(DynamicByteArray&& cont) :
        container { std::forward<DynamicByteArray>(cont) }
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

    DynamicByteArray container;
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
        return new InternalUnmanagedBlob{ data, size };
    else
        return new InternalVectorBlob{ data, size };
}

static Blob::Pimpl* MakeInternalBlob(DynamicByteArray&& cont)
{
    return new InternalBufferBlob{ std::forward<DynamicByteArray>(cont) };
}

static Blob::Pimpl* MakeInternalBlob(std::vector<char>&& cont)
{
    return new InternalVectorBlob{ std::forward<std::vector<char>>(cont) };
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

Blob::Blob(Blob&& rhs) noexcept :
    pimpl_ { rhs.pimpl_ }
{
    rhs.pimpl_ = nullptr;
}

Blob& Blob::operator = (Blob&& rhs) noexcept
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

Blob Blob::CreateCopy(const Blob& other)
{
    return (other ? Blob{ other.GetData(), other.GetSize() } : Blob{});
}

Blob Blob::CreateCopy(const void* data, std::size_t size)
{
    return Blob{ data, size };
}

Blob Blob::CreateWeakRef(const void* data, std::size_t size)
{
    return Blob{ data, size, /*isWeakRef:*/ true };
}

Blob Blob::CreateStrongRef(DynamicByteArray&& cont)
{
    Blob blob;
    blob.pimpl_ = MakeInternalBlob(std::forward<DynamicByteArray>(cont));
    return blob;
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

#if LLGL_OS_ANDROID

static bool ReadBufferFromFile(const char* filename, DynamicByteArray& outContent, bool isNULTerminated)
{
    AAssetManager* assetMngr = AndroidApp::Get().GetContext().assetManager;
    LLGL_ASSERT(assetMngr != nullptr, "AndroidContext::assetManager must be set to read files by path");

    if (AAsset* asset = AAssetManager_open(assetMngr, filename, AASSET_MODE_STREAMING))
    {
        /* Get asset size by setting position at end of stream */
        const off_t assetSize = AAsset_seek(asset, 0, SEEK_END);
        AAsset_seek(asset, 0, SEEK_SET);
        if (assetSize > 0)
        {
            /* Allocate internal container and optional NUL-terminator */
            if (isNULTerminated)
            {
                outContent = DynamicByteArray{ static_cast<std::size_t>(assetSize + 1), UninitializeTag{} };
                outContent[assetSize] = '\0';
            }
            else
                outContent = DynamicByteArray{ static_cast<std::size_t>(assetSize), UninitializeTag{} };

            /* Read entire asset content */
            AAsset_read(asset, outContent.data(), static_cast<std::size_t>(assetSize));
        }
        AAsset_close(asset);
        return true;
    }
    return false;
}

#else // LLGL_OS_ANDROID

static bool ReadBufferFromFile(const char* filename, DynamicByteArray& outContent, bool isNULTerminated)
{
    /* Read file as binary */
    std::ifstream file{ filename, (std::ios::in | std::ios::binary) };
    if (!file.good())
        return false;

    /* Determine file size */
    file.seekg(0, std::ios::end);
    const std::size_t fileSize = static_cast<std::size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    /* Allocate internal container and optional NUL-terminator */
    if (isNULTerminated)
    {
        outContent = DynamicByteArray{ fileSize + 1, UninitializeTag{} };
        outContent[fileSize] = '\0';
    }
    else
        outContent = DynamicByteArray{ fileSize, UninitializeTag{} };

    /* Read entire file into output content */
    file.read(outContent.data(), fileSize);

    return true;
}

#endif // /LLGL_OS_ANDROID

// Consolidates combined newline characters consisting of carriage return '\r' and newline '\n' which are common on Windows systems.
static Blob::Pimpl* MakeInternalBlobAndSanitizeNewlineChars(DynamicByteArray&& buffer)
{
    /* If there are no carriage return characters, just return the input buffer as blob container */
    if (buffer.empty() || std::find(buffer.begin(), buffer.end(), '\r') == buffer.end())
        return MakeInternalBlob(std::move(buffer));

    /* Otherwise, pre-allocate string with same size and append all characters, consolidating '\r'+'\n' characters to a single '\n'. */
    std::string converted;
    converted.reserve(buffer.size() - 1);

    for (const char* s = static_cast<const char*>(buffer.data()); *s != '\0'; ++s)
    {
        if (s[0] == '\r' && s[1] == '\n')
            continue;
        else
            converted.append(1, s[0]);
    }

    return MakeInternalBlob(std::move(converted));
}

Blob Blob::CreateFromFile(const char* filename, bool isTextFile)
{
    if (filename != nullptr && *filename != '\0')
    {
        //TODO: this might not be necessary anymore; Re-evaluate on iOS
        #if LLGL_OS_IOS
        /* Substitute filename with its absolute path for mobile platforms. */
        const UTF8String fullFilePath = Path::GetAbsolutePath(filename);
        filename = fullFilePath.c_str();
        #endif

        /* Read binary file into container */
        DynamicByteArray buffer;
        if (ReadBufferFromFile(filename, buffer, isTextFile))
        {
            /* Return blob that manages the read buffer */
            Blob blob;
            if (isTextFile)
                blob.pimpl_ = MakeInternalBlobAndSanitizeNewlineChars(std::move(buffer));
            else
                blob.pimpl_ = MakeInternalBlob(std::move(buffer));
            return blob;
        }
    }
    return Blob{};
}

Blob Blob::CreateFromFile(const std::string& filename, bool isTextFile)
{
    return CreateFromFile(filename.c_str(), isTextFile);
}

const void* Blob::GetData() const
{
    return (pimpl_ != nullptr ? pimpl_->GetData() : nullptr);
}

std::size_t Blob::GetSize() const
{
    return (pimpl_ != nullptr ? pimpl_->GetSize() : 0);
}

const char* Blob::AsCString() const
{
    if (pimpl_ != nullptr)
    {
        if (const void* data = pimpl_->GetData())
        {
            if (const std::size_t size = pimpl_->GetSize())
            {
                const char* str = static_cast<const char*>(data);
                if (str[size - 1] == '\0')
                    return str;
            }
        }
    }
    return nullptr;
}

StringView Blob::AsStringView() const
{
    if (pimpl_ != nullptr)
    {
        if (const void* data = pimpl_->GetData())
        {
            if (const std::size_t size = pimpl_->GetSize())
            {
                const char* str = static_cast<const char*>(data);
                return (str[size - 1] == '\0' ? StringView{ str, size - 1 } : StringView{ str, size });
            }
        }
    }
    return {};
}

Blob::operator bool () const
{
    return (pimpl_ != nullptr && pimpl_->GetData() != nullptr && pimpl_->GetSize() > 0);
}


} // /namespace LLGL



// ================================================================================
