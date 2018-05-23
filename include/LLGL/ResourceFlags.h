/*
 * ResourceFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RESOURCE_FLAGS_H
#define LLGL_RESOURCE_FLAGS_H


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Hardware resource type enumeration.
\remarks This is primarily used to describe the source type for a layout binding (see BindingDescriptor),
which is why all buffer types are enumerated but not the texture types.
\see BindingDescriptor::type
\see BufferType
*/
enum class ResourceType
{
    //! Undefined resource type.
    Undefined,

    /**
    \brief Vertex buffer resource.
    \see Buffer
    \see BufferType::Vertex
    */
    VertexBuffer,

    /**
    \brief Index buffer resource.
    \see Buffer
    \see BufferType::Index
    */
    IndexBuffer,

    /**
    \brief Constant buffer (or uniform buffer) resource.
    \see Buffer
    \see BufferType::Constant
    */
    ConstantBuffer,

    /**
    \brief Storage buffer resource.
    \see Buffer
    \see BufferType::Storage
    */
    StorageBuffer,

    /**
    \brief Stream-output buffer resource.
    \see Buffer
    \see BufferType::StreamOutput
    */
    StreamOutputBuffer,

    /**
    \brief Texture resource.
    \see Texture
    \see TextureType
    */
    Texture,

    /**
    \brief Sampler state resource.
    \see Sampler
    */
    Sampler,
};


} // /namespace LLGL


#endif



// ================================================================================
