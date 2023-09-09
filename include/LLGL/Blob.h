/*
 * Blob.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_BLOB_H
#define LLGL_BLOB_H


#include <LLGL/NonCopyable.h>
#include <LLGL/Container/DynamicArray.h>
#include <vector>
#include <string>
#include <memory>


namespace LLGL
{


/**
\brief CPU read-only buffer of arbitrary size.
\see RenderSystem::CreatePipelineState
*/
class LLGL_EXPORT Blob : public NonCopyable
{

    public:

        struct Pimpl;

        //! Construcst an empty blob.
        Blob();

        //! Move constructor.
        Blob(Blob&& rhs);

        //! Move operator.
        Blob& operator = (Blob&& rhs);

        /**
        \brief Constructs the blob with a copy of the specified data or a weak reference to the data.
        \param[in] data Raw pointer to the data this blob is meant to take care of.
        \param[in] size Specifies the size (in bytes) of the memory \c data pointer to.
        \param[in] isWeakRef If this is true, this blob only references the data but does not copy it.
        In this case, the caller is responsible for managing the lifetime of the data and it must remain valid until the end of this blob. By default false.
        */
        Blob(const void* data, std::size_t size, bool isWeakRef = false);

        //! Deletes the internal memory blob.
        ~Blob();

    public:

        /**
        \brief Creates a new Blob instance with a copy of the specified data.
        \param[in] data Pointer to the data that is to be copied and managed by this blob.
        \param[in] size Specifies the size (in bytes) of the data.
        \return New instance of Blob that manages the memory that is copied.
        */
        static Blob CreateCopy(const void* data, std::size_t size);

        /**
        \brief Creates a new Blob instance with a weak reference to the specified data.
        \param[in] data Pointer to the data that is to be referenced. This pointer must remain valid for the lifetime of this Blob instance.
        \param[in] size Specifies the size (in bytes) of the data.
        \return New instance of Blob that refers to the specified memory.
        */
        static Blob CreateWeakRef(const void* data, std::size_t size);

        /**
        \brief Creates a new Blob instance with a strong reference to the specified byte array container.
        \param[in] cont Specifies the container whose data is to be moved into this Blob instance.
        \return New instance of Blob that manages the specified unique pointer.
        */
        static Blob CreateStrongRef(DynamicByteArray&& cont);

        /**
        \brief Creates a new Blob instance with a strong reference to the specified vector container.
        \param[in] cont Specifies the container whose data is to be moved into this Blob instance.
        \return New instance of Blob that manages the specified container.
        */
        static Blob CreateStrongRef(std::vector<char>&& cont);

        /**
        \brief Creates a new Blob instance with a strong reference to the specified string container.
        \param[in] str Specifies the container whose data is to be moved into this Blob instance.
        \return New instance of Blob that manages the specified container.
        */
        static Blob CreateStrongRef(std::string&& str);

        /**
        \brief Creates a new Blob instance with the data read from the specified binary file.
        \param[in] filename Specifies the file that is to be read.
        \return New instance of Blob that manages the memory of a conent copy from the specified file or null if the file could not be read.
        */
        static Blob CreateFromFile(const char* filename);

        /**
        \brief Creates a new Blob instance with the data read from the specified binary file.
        \param[in] filename Specifies the file that is to be read.
        \return New instance of Blob that manages the memory of a conent copy from the specified file or null if the file could not be read.
        */
        static Blob CreateFromFile(const std::string& filename);

    public:

        //! Returns a constant pointer to the internal buffer or null if this is a default initialized blob.
        const void* GetData() const;

        //! Returns the size (in bytes) of the internal buffer or zero if this is a default initialized blob.
        std::size_t GetSize() const;

    public:

        //! Returns true if this blob is non-empty.
        operator bool () const;

    private:

        Pimpl* pimpl_;

};


} // /namespace LLGL


#endif



// ================================================================================
