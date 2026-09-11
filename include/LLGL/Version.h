/*
 * Version.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VERSION_H
#define LLGL_VERSION_H


#include <LLGL/Export.h>
#include <LLGL/Container/UTF8String.h>
#include <LLGL/Deprecated.h>
#include <cstdint>


namespace LLGL
{


/**
\brief Version status enumeration.
\see VersionInfo::status
*/
enum class VersionStatus : std::uint8_t
{
    Undefined,
    Alpha,
    Beta,
    Stable,
};

/**
\brief General purpose version information structure.
\remarks This is used to identify the LLGL build version but can also be used for other versioning purposes.
\see GetLLGLVersion
*/
struct VersionInfo
{
    //! Major version number.
    std::uint16_t   major     = 0;

    //! Minor version number. Should be in the range [0, 99].
    std::uint8_t    minor     = 0;

    //! Version status (Alpha, Beta etc.).
    VersionStatus   status    = VersionStatus::Undefined;

    //! Revision number
    std::uint32_t   revision  = 0;
};

/**
\briefs Compares the two VersionInfo arguments.
\returns 0 if both arguemnts are equal,
< 0 if \c lhs is considered an earlier version than \c rhs,
or > 0 if \c lhs is considered a newer version than \c ths.
*/
LLGL_EXPORT int Compare(VersionInfo lhs, VersionInfo rhs);

//! Returns a string representation of the specified version status or null if undefined (VersionStatus::Undefined).
LLGL_EXPORT const char* ToString(VersionStatus status);

/**
\brief Returns a string representation of the specified version.
\remarks This version of this function uses an internal thread-local buffer and can therefore only be used for one version at a time.
To forward the return value to a \c printf function directly for instance, use the other version of this function and provide it with a custom buffer.
\see ToString(VersionInfo, std::size_t, char*)
*/
LLGL_EXPORT const char* ToString(VersionInfo info);

/**
\brief Returns a string representation of the specified version using a custom buffer.
\param[in] info Specifies the version information to print as a string.
\param[in] bufferSize Specifies the size (in number of characters) of the output buffer. This size includes the NUL-terminator.
\param[out] buffer Pointer to the output buffer to write the string representation to.
This buffer \b must point to a memory block of at least \c bufferSize characters.
This may also be null, in which case only the \c outMinBufferSize is used to write out the requird buffer size.
\param[out] outMinBufferSize Optional pointer the function can write the required buffer size to.
\return Same value as \c buffer parameter for convenient use in \c printf style functions.
\see ToString(VersionInfo)
*/
LLGL_EXPORT const char* ToString(VersionInfo info, std::size_t bufferSize, char* buffer, std::size_t* outMinBufferSize = nullptr);

//! \brief Returns the version information of this library.
LLGL_EXPORT VersionInfo GetLLGLVersion();

/**
\brief Returns true if both VersionInfo arguments are equal.
\see Compare(VersionInfo)
*/
inline bool operator == (VersionInfo lhs, VersionInfo rhs) { return (Compare(lhs, rhs) == 0); }

/**
\brief Returns true if both VersionInfo arguments are \e not equal.
\see Compare(VersionInfo)
*/
inline bool operator != (VersionInfo lhs, VersionInfo rhs) { return (Compare(lhs, rhs) != 0); }

/**
\brief Returns true if \c lhs describes an earlier version than \c rhs.
\see Compare(VersionInfo)
*/
inline bool operator <  (VersionInfo lhs, VersionInfo rhs) { return (Compare(lhs, rhs) <  0); }

/**
\brief Returns true if \c lhs describes an earlier or equal version as \c rhs.
\see Compare(VersionInfo)
*/
inline bool operator <= (VersionInfo lhs, VersionInfo rhs) { return (Compare(lhs, rhs) <= 0); }

/**
\brief Returns true if \c lhs describes a newer version than \c rhs.
\see Compare(VersionInfo)
*/
inline bool operator >  (VersionInfo lhs, VersionInfo rhs) { return (Compare(lhs, rhs) >  0); }

/**
\brief Returns true if \c lhs describes a newer or equal version as \c rhs.
\see Compare(VersionInfo)
*/
inline bool operator >= (VersionInfo lhs, VersionInfo rhs) { return (Compare(lhs, rhs) >= 0); }


//! \deprecated Since 0.05b; Use VersionInfo and GetLLGLVersion instead!
namespace Version
{


//! \deprecated Since 0.05b; Use VersionInfo and GetLLGLVersion instead!
LLGL_DEPRECATED("LLGL::Version::GetMajor() is deprecated since 0.05b; Use GetLLGLVersion() instead!", "GetLLGLVersion().major")
inline unsigned GetMajor() { return GetLLGLVersion().major; }

//! \deprecated Since 0.05b; Use VersionInfo and GetLLGLVersion instead!
LLGL_DEPRECATED("LLGL::Version::GetMinor() is deprecated since 0.05b; Use GetLLGLVersion() instead!", "GetLLGLVersion().minor")
inline unsigned GetMinor() { return GetLLGLVersion().minor; }

//! \deprecated Since 0.05b; Use VersionInfo and GetLLGLVersion instead!
LLGL_DEPRECATED("LLGL::Version::GetRevision() is deprecated since 0.05b; Use GetLLGLVersion() instead!", "GetLLGLVersion().revision")
inline unsigned GetRevision() { return GetLLGLVersion().revision; }

//! \deprecated Since 0.05b; Use VersionInfo and GetLLGLVersion instead!
LLGL_DEPRECATED("LLGL::Version::GetStatus() is deprecated since 0.05b; Use GetLLGLVersion() instead!", "ToString(GetLLGLVersion().status)")
inline const char* GetStatus() { return ToString(GetLLGLVersion().status); }

//! \deprecated Since 0.05b; Use VersionInfo and GetLLGLVersion instead!
LLGL_DEPRECATED("LLGL::Version::GetID() is deprecated since 0.05b; Use GetLLGLVersion() instead!", "GetLLGLVersion()")
inline unsigned GetID()
{
    const VersionInfo info = GetLLGLVersion();
    return static_cast<int>(info.major) * 100000 + static_cast<int>(info.minor) * 100 + static_cast<int>(info.revision);
}

//! \deprecated Since 0.05b; Use VersionInfo and GetLLGLVersion instead!
LLGL_DEPRECATED("LLGL::Version::GetString() is deprecated since 0.05b; Use GetLLGLVersion() instead!", "ToString(GetLLGLVersion())")
inline const char* GetString() { return ToString(GetLLGLVersion()); }


} // /namespace Version


} // /namespace LLGL


#endif



// ================================================================================
