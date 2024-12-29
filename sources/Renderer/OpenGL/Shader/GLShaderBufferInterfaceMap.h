/*
 * GLShaderBufferInterfaceMap.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_SHADER_BUFFER_INTERFACE_MAP_H
#define LLGL_GL_SHADER_BUFFER_INTERFACE_MAP_H


#include <vector>
#include <LLGL/Container/ArrayView.h>
#include "../RenderState/GLPipelineLayout.h"


namespace LLGL
{


class GLShaderPipeline;

enum GLBufferInterface : char
{
    GLBufferInterface_SSBO = 0, // buffer
    GLBufferInterface_Sampler,  // samplerBuffer, isamplerBuffer, usamplerBuffer
    GLBufferInterface_Image,    // imageBuffer, iimageBuffer, uimageBuffer
};

// Helper class to determine what sampled or storage buffers must be bound as SSBO, sampler buffer, and image buffer.
class GLShaderBufferInterfaceMap
{

    public:

        GLShaderBufferInterfaceMap();

        // Builds the internal bitmap to map resources to SSBOs and sampler buffers.
        void BuildMap(const GLPipelineLayout& pipelineLayout, const GLShaderPipeline& shaderPipeline);

        // Returns the entries that map heap binding descriptors to either SSBO, sampler buffer, or image buffer.
        inline ArrayView<GLBufferInterface> GetHeapInterfaces() const
        {
            return ArrayView<GLBufferInterface>{ bufferMap_.data(), GetNumHeapEntries() };
        }

        // Returns the entries that map heap binding descriptors to either SSBO, sampler buffer, or image buffer.
        inline ArrayView<GLBufferInterface> GetDynamicInterfaces() const
        {
            return ArrayView<GLBufferInterface>{ bufferMap_.data() + GetNumHeapEntries(), GetNumDynamicEntries() };
        }

        // Returns all entries that map binding descriptors to either SSBO, sampler buffer, or image buffer. 
        inline ArrayView<GLBufferInterface> GetInterfaces() const
        {
            return bufferMap_;
        }

        // Returns the number of shader storage buffer objects (SSBO) in this map.
        inline std::size_t GetNumSSBOs() const
        {
            return numSSBOs_;
        }

        // Returns the number of sampler buffers in this map.
        inline std::size_t GetNumSamplerBuffers() const
        {
            return (bufferMap_.size() - numSSBOs_);
        }

        // Returns the number of entries for heap bindings. Those entries come first in the bitmap.
        inline std::size_t GetNumHeapEntries() const
        {
            return numHeapEntries_;
        }

        // Returns the number of entries for dynamic bindings. Those entries come second in the bitmap.
        inline std::size_t GetNumDynamicEntries() const
        {
            return (bufferMap_.size() - numHeapEntries_);
        }

        // Returns whether this shader interface map has only SSBO entries for heap bindings.
        // If this is true, the respective resource heap can make use of bundled buffer bindings.
        // Otherwise, each SSBO must be bound individually since some entries must be bound as sampler or image buffers.
        inline bool HasHeapSSBOEntriesOnly() const
        {
            return (hasHeapSSBOEntriesOnly_ != 0);
        }

        // Returns whether this shader interface map has only SSBO entries for dynamic bindings.
        inline bool HasDynamicSSBOEntriesOnly() const
        {
            return (hasDynamicSSBOEntriesOnly_ != 0);
        }

        // Returns whether this shader interface map has only SSBO entries for both heap and dynamic bindings.
        inline bool HasSSBOEntriesOnly() const
        {
            return (HasHeapSSBOEntriesOnly() && HasDynamicSSBOEntriesOnly());
        }

    private:

        void AppendHeapEntry(GLBufferInterface entry);
        void AppendDynamicEntry(GLBufferInterface entry);

    private:

        std::vector<GLBufferInterface>  bufferMap_;                         // Maps a buffer binding to a shader interface type
        std::size_t                     numSSBOs_                   : 15;   // Number of actual SSBOs in this bitmap
        std::size_t                     numHeapEntries_             : 15;
        std::size_t                     hasHeapSSBOEntriesOnly_     : 1;
        std::size_t                     hasDynamicSSBOEntriesOnly_  : 1;

};


} // /namespace LLGL


#endif



// ================================================================================
