/*
 * LinearStringContainer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_LINEAR_STRING_CONTAINER_H
#define LLGL_LINEAR_STRING_CONTAINER_H


#include "Helper.h"
#include <vector>
#include <string>


namespace LLGL
{


/*
Helper class to manage linear memory of multiple null-terminated strings.
1. Reserve all required memory for all strings
2. Copy strings into linear memory
Example: Buffer = "FirstString\0SecondString\0etc.\0"
*/
template <typename T>
class LinearStringContainerBase
{

    public:

        // Clears the container.
        void Clear()
        {
            data_.clear();
            reserved_   = 0;
            offset_     = 0;
        }

        // Reserve space for a string with the specified length (excluding the null terminator).
        void Reserve(std::size_t len)
        {
            reserved_ += (len + 1);
        }

        // Copy the specified string into this container and return its pointer.
        const T* CopyString(const T* str)
        {
            return CopyStringPrimary(str,  StrLength(str));
        }

        // Copy the specified string into this container and return its pointer.
        const T* CopyString(const std::string& str)
        {
            return CopyStringPrimary(str.c_str(), str.size());
        }

        // Returns the pointer to the first character of the next string, where <ptr> specifies the pointer from the previous string in the container.
        const T* GetNextString(const T* ptr = nullptr) const
        {
            if (ptr != nullptr)
            {
                /* Return pointer after the next null terminator */
                for (; IsInsideRange(ptr); ++ptr)
                {
                    if (*ptr == 0)
                    {
                        /* Get pointer after null terminator, if no longer inside range, we reached the end of the container */
                        ptr++;
                        return (IsInsideRange(ptr) ? ptr : nullptr);
                    }
                }
            }
            else
            {
                /* Return pointer to first string */
                return data_.data();
            }
            return nullptr;
        }

    private:

        // Primary implementation of the "CopyString" functions.
        char* CopyStringPrimary(const T* str, std::size_t len)
        {
            const auto grow = len + 1;

            /* Resize container if necessary */
            if (offset_ + grow > data_.size())
            {
                if (reserved_ > grow)
                    data_.resize(data_.size() + reserved_);
                else
                    data_.resize(data_.size() + grow);
                reserved_ = 0;
            }

            /* Copy string into container */
            auto dst = &data_[offset_];
            ::memcpy(dst, str, sizeof(T) * grow);

            /* Increase offset and return pointer to destination string */
            offset_ += grow;
            return dst;
        }

        // Returns true if the specified pointer is inside the range of the container.
        bool IsInsideRange(const T* ptr) const
        {
            return (ptr >= data_.data() && ptr < data_.data() + data_.size());
        }

    private:

        std::vector<T>  data_;
        std::size_t     reserved_   = 0;
        std::size_t     offset_     = 0;

};

// Linear string container type for ANSI strings.
using LinearStringContainer = LinearStringContainerBase<char>;

// Linear string container type for Unicode strings.
using LinearWStringContainer = LinearStringContainerBase<wchar_t>;


} // /namespace LLGL


#endif



// ================================================================================
