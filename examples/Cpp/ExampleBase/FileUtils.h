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
 * Global helper functions
 */

// Helper class to abstract reading assets for multiple platforms.
class AssetReader
{

    public:

        AssetReader() = default;

        AssetReader(AssetReader&&) = default;
        AssetReader& operator = (AssetReader&&) = default;

        // Take ownership of the specified content to read an asset.
        AssetReader(std::vector<char>&& content);

        // Returns true if this asset reader has any content.
        bool IsValid() const;

        // Shotcut to IsValid().
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

// Returns the content of the specified asset.
// If the file could not be found, an empty container is returned and an error is reported to the log.
std::vector<char> ReadAsset(const std::string& name, std::string* outFullPath = nullptr);

// Reads the specified asset as text file and returns each line in an array.
std::vector<std::string> ReadTextLines(const std::string& name, std::string* outFullPath = nullptr);

// Writes the specified FrameProfile as a JSON body string.
// This can be loaded up in Goolge Chrome's Trace Viewer (see https://google.github.io/trace-viewer/).
std::string WriteFrameProfileToJson(const LLGL::FrameProfile& frameProfile);

// Writes the specified FrameProfile to a JSON file.
bool WriteFrameProfileToJsonFile(const LLGL::FrameProfile& frameProfile, const char* filename);


#endif

