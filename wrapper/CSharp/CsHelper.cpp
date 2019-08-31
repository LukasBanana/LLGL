/*
 * CsHelper.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsHelper.h"
#include <string>

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;


namespace SharpLLGL
{


std::string ToStdString(String^ s)
{
    std::string out;

    if (s != nullptr)
    {
        auto inputSourceCodePtr = Marshal::StringToHGlobalAnsi(s);
        {
            out = static_cast<char*>(static_cast<void*>(inputSourceCodePtr));
        }
        Marshal::FreeHGlobal(inputSourceCodePtr);
    }

    return out;
}

std::wstring ToStdWString(String^ s)
{
    std::wstring out;

    if (s != nullptr)
    {
        auto inputSourceCodePtr = Marshal::StringToHGlobalUni(s);
        {
            out = static_cast<wchar_t*>(static_cast<void*>(inputSourceCodePtr));
        }
        Marshal::FreeHGlobal(inputSourceCodePtr);
    }

    return out;
}

String^ ToManagedString(const std::string& s)
{
    return gcnew String(s.c_str());
}

System::String^ ToManagedString(const std::wstring& s)
{
    return gcnew String(s.c_str());
}

String^ ToManagedString(const char* s)
{
    return gcnew String(s);
}

System::String^ ToManagedString(const wchar_t* s)
{
    return gcnew String(s);
}


} // /namespace SharpLLGL



// ================================================================================
