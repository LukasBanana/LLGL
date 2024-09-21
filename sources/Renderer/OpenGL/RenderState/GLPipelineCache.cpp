/*
 * GLPipelineCache.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLPipelineCache.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include <LLGL/Utils/ForRange.h>
#include <string.h>


namespace LLGL
{


#include "../../../Core/PackStructPush.inl"

struct GLPipelineCacheHeader
{
    std::uint32_t permutationOffsets[1];
}
LLGL_PACK_STRUCT;

struct GLPipelineCacheEntry
{
    GLenum  binaryFormat;
    GLsizei binaryLength;
}
LLGL_PACK_STRUCT;

#include "../../../Core/PackStructPop.inl"

GLPipelineCache::GLPipelineCache(const Blob& initialBlob)
{
    if (const GLPipelineCacheHeader* header = reinterpret_cast<const GLPipelineCacheHeader*>(initialBlob.GetData()))
    {
        InitializeEntry(GLShader::PermutationDefault, header + 1);
        if (header->permutationOffsets[0] > 0)
            InitializeEntry(GLShader::PermutationFlippedYPosition, reinterpret_cast<const char*>(header) + header->permutationOffsets[0]);
    }
}

Blob GLPipelineCache::GetBlob() const
{
    /* Determine size of all cache entries */
    std::size_t cacheSize = 0;
    GLPipelineCacheHeader header = {};

    for_range(permutation, GLShader::PermutationCount)
    {
        const CacheEntry& entry = entries_[permutation];
        if (!entry.data.empty())
        {
            if (permutation > 0)
                header.permutationOffsets[permutation - 1] = static_cast<std::uint32_t>(cacheSize);
            cacheSize += sizeof(GLPipelineCacheEntry);
            cacheSize += entry.length;
        }
    }

    if (cacheSize > 0)
    {
        /* Allocate cache blob including header */
        cacheSize += sizeof(GLPipelineCacheHeader);
        DynamicByteArray cache{ cacheSize, UninitializeTag{} };

        char* bytes = cache.get();

        auto WriteBytes = [&bytes](const void* src, std::size_t len) -> void
        {
            ::memcpy(bytes, src, len);
            bytes += len;
        };

        WriteBytes(&header, sizeof(header));

        for (const CacheEntry& entry : entries_)
        {
            if (!entry.data.empty())
            {
                GLPipelineCacheEntry headerEntry;
                headerEntry.binaryFormat = entry.format;
                headerEntry.binaryLength = entry.length;
                WriteBytes(&headerEntry, sizeof(headerEntry));
                WriteBytes(entry.data.get(), entry.length);
            }
        }

        return Blob::CreateStrongRef(std::move(cache));
    }

    return Blob{};
}

bool GLPipelineCache::ProgramBinary(GLShader::Permutation permutation, GLuint program)
{
    /* No need to check for extension support at runtime here, this is checked before GLPipelineCache is created */
    #if LLGL_GLEXT_GET_PROGRAM_BINARY

    if (!HasProgramBinary(permutation))
        return false;

    /* Load program binary into GL object */
    const CacheEntry& entry = entries_[permutation];
    glProgramBinary(program, entry.format, entry.data.get(), entry.length);

    /* Check link status */
    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    return (status != GL_FALSE);

    #else // LLGL_GLEXT_GET_PROGRAM_BINARY

    return false;

    #endif // /LLGL_GLEXT_GET_PROGRAM_BINARY
}

bool GLPipelineCache::GetProgramBinary(GLShader::Permutation permutation, GLuint program)
{
    /* No need to check for extension support at runtime here, this is checked before GLPipelineCache is created */
    #if LLGL_GLEXT_GET_PROGRAM_BINARY

    CacheEntry& entry = entries_[permutation];

    /* Get program binary length */
    GLint binaryLength = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &binaryLength);
    if (binaryLength > 0)
        entry.length = static_cast<GLsizei>(binaryLength);
    else
        return false;

    /* Get program binary format and data */
    entry.data.resize(static_cast<std::size_t>(binaryLength), UninitializeTag{});

    GLsizei writtenLegnth = 0;
    glGetProgramBinary(
        program,
        entry.length,
        &writtenLegnth,
        &(entry.format),
        entry.data.get()
    );
    if (writtenLegnth != entry.length)
        return false;

    return true;

    #else // LLGL_GLEXT_GET_PROGRAM_BINARY

    return false;

    #endif // /LLGL_GLEXT_GET_PROGRAM_BINARY
}


/*
 * ======= Private: =======
 */

void GLPipelineCache::InitializeEntry(GLShader::Permutation permutation, const void* data)
{
    const GLPipelineCacheEntry* srcEntry    = reinterpret_cast<const GLPipelineCacheEntry*>(data);
    const char*                 bytes       = reinterpret_cast<const char*>(srcEntry + 1);

    CacheEntry& dstEntry = entries_[permutation];
    dstEntry.format = srcEntry->binaryFormat;
    dstEntry.length = srcEntry->binaryLength;
    dstEntry.data   = DynamicByteArray{ bytes, bytes + srcEntry->binaryLength };
}


} // /namespace LLGL



// ================================================================================
