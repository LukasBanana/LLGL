/*
 * FileUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGLEXAMPLES_FILE_UTILS_H
#define LLGLEXAMPLES_FILE_UTILS_H


#include <string>
#include <vector>
#include <type_traits>
#include <LLGL/RenderingDebuggerFlags.h>


/*
 * Global helper macros
 */

struct ExampleMacroUtils
{
    template <std::size_t N>
    static constexpr char GetLastChar(const char (&str)[N])
    {
        return (N >= 2 ? str[N - 2] : '\0');
    }

    template <std::size_t N>
    static constexpr bool EndsWithNewline(const char (&str)[N])
    {
        return (ExampleMacroUtils::GetLastChar(str) == '\n');
    }
};

#define LLGL_OPTIONAL_VARGS(...) \
    , ## __VA_ARGS__

#define LLGL_LOG(FORMAT, ...)                                                       \
    do                                                                              \
    {                                                                               \
        static_assert(                                                              \
            ExampleMacroUtils::EndsWithNewline(FORMAT),                             \
            "LLGL_LOG string must end with a newline character");                   \
        LLGL::Log::Printf((FORMAT) LLGL_OPTIONAL_VARGS(__VA_ARGS__));               \
    }                                                                               \
    while (false)

#define LLGL_WARN(FORMAT, ...)                                                      \
    do                                                                              \
    {                                                                               \
        static_assert(                                                              \
            ExampleMacroUtils::EndsWithNewline(FORMAT),                             \
            "LLGL_WARN string must end with a newline character");                  \
        LLGL::Log::Errorf(                                                          \
            LLGL::Log::ColorFlags::StdWarning, ("Warning: " FORMAT)                 \
            LLGL_OPTIONAL_VARGS(__VA_ARGS__));                                      \
    }                                                                               \
    while (false)

#define LLGL_ERROR(FORMAT, ...)                                                     \
    do                                                                              \
    {                                                                               \
        static_assert(                                                              \
            ExampleMacroUtils::EndsWithNewline(FORMAT),                             \
            "LLGL_ERROR string must end with a newline character");                 \
        LLGL::Log::Errorf(                                                          \
            LLGL::Log::ColorFlags::StdError, ("Error: " FORMAT)                     \
            LLGL_OPTIONAL_VARGS(__VA_ARGS__));                                      \
    }                                                                               \
    while (false)


/*
 * Global helper functions
 */

// Helper class to abstract reading assets for multiple platforms.
class AssetReader
{

    public:

        AssetReader() = default;

        // Take ownership of the specified content to read an asset.
        AssetReader(std::vector<char>&& content);

        // Returns true if this asset reader has any content.
        bool IsValid() const;

        // Shortcut to IsValid().
        operator bool () const;

        // Main function to read data from the asset.
        std::size_t Read(void* data, std::size_t dataSize);

        // Generic version to read a single value/struct.
        template <typename T>
        T Read()
        {
            static_assert(std::is_trivially_constructible<T>::value, "AssetReader::Read<T>(): T must be trivially constructible");
            T value = {};
            Read(&value, sizeof(T));
            return value;
        }

    public:

        std::vector<char>   content_;
        std::size_t         readPos_ = 0;

};

// Returns true if the specified file can be read.
bool FileExists(const std::string& filename);

// Returns true if the specified asset exists and resolves the output path.
bool FindAsset(const std::string& name, std::string* outFullPath = nullptr);

// Returns the content of the specified asset.
// If the file could not be found, an empty container is returned and an error is reported to the log.
std::vector<char> ReadAsset(const std::string& name, std::string* outFullPath = nullptr);

// Reads the specified asset as text file and returns each line in an array.
std::vector<std::string> ReadTextLines(const std::string& name, std::string* outFullPath = nullptr);

// Writes the specified FrameProfile as a JSON body string.
// This can be loaded up in Google Chrome's Trace Viewer (see https://google.github.io/trace-viewer/).
std::string WriteFrameProfileToJson(const LLGL::FrameProfile& frameProfile);

// Writes the specified FrameProfile to a JSON file.
bool WriteFrameProfileToJsonFile(const LLGL::FrameProfile& frameProfile, const char* filename);


#endif

