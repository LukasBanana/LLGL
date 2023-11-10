/*
 * Win32Debug.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../Debug.h"
#include "../../Core/StringUtils.h"
#include "Win32Module.h"
#include <stdio.h>
#include <limits.h>
#include <vector>
#include <algorithm>
#include <LLGL/Utils/ForRange.h>

#include <Windows.h>
#include <DbgHelp.h>


namespace LLGL
{


class DbgHelpModule final
{

    public:

        DbgHelpModule(const DbgHelpModule&) = delete;
        DbgHelpModule& operator = (const DbgHelpModule&) = delete;

        static DbgHelpModule& Get();

        UTF8String AddressToSymbolName(const void* addr) const;
        UTF8String AddressToSourceInfo(const void* addr) const;

    private:

        DbgHelpModule();

    private:

        typedef DWORD (WINAPI *PFN_SYMSETOPTIONS_PROC)(DWORD SymOptions);
        typedef BOOL (WINAPI *PFN_SYMINITIALIZE_PROC)(HANDLE hProcess, PCSTR UserSearchPath, BOOL fInvadeProcess);

        typedef BOOL (WINAPI *PFN_SYMFROMADDR_PROC)(HANDLE hProcess, DWORD64 Address, PDWORD64 Displacement, PSYMBOL_INFO Symbol);
        typedef BOOL (WINAPI *PFN_SYMFROMADDRW_PROC)(HANDLE hProcess, DWORD64 Address, PDWORD64 Displacement, PSYMBOL_INFOW Symbol);

        typedef BOOL (WINAPI *PFN_SYMGETLINEFROMADDR_PROC)(HANDLE hProcess, DWORD qwAddr, PDWORD pdwDisplacement, PIMAGEHLP_LINE Line);
        typedef BOOL (WINAPI *PFN_SYMGETLINEFROMADDR64_PROC)(HANDLE hProcess, DWORD64 qwAddr, PDWORD pdwDisplacement, PIMAGEHLP_LINE64 Line);
        typedef BOOL (WINAPI *PFN_SYMGETLINEFROMADDRW64_PROC)(HANDLE hProcess, DWORD64 dwAddr, PDWORD pdwDisplacement, PIMAGEHLP_LINEW64 Line);

    private:

        HANDLE      process_ = nullptr;
        Win32Module module_;

        struct Vtable
        {
            PFN_SYMSETOPTIONS_PROC          symSetOptions           = nullptr;
            PFN_SYMINITIALIZE_PROC          symInitialize           = nullptr;
            PFN_SYMFROMADDR_PROC            symFromAddr             = nullptr;
            PFN_SYMFROMADDRW_PROC           symFromAddrW            = nullptr;
            PFN_SYMGETLINEFROMADDR_PROC     symGetLineFromAddr      = nullptr;
            PFN_SYMGETLINEFROMADDR64_PROC   symGetLineFromAddr64    = nullptr;
            PFN_SYMGETLINEFROMADDRW64_PROC  symGetLineFromAddrW64   = nullptr;
        }
        vtable_;

};

DbgHelpModule::DbgHelpModule() :
    process_ { GetCurrentProcess() },
    module_  { "Dbghelp.dll"       }
{
    if (module_.IsValid())
    {
        vtable_.symSetOptions = reinterpret_cast<PFN_SYMSETOPTIONS_PROC>(module_.LoadProcedure("SymSetOptions"));
        if (vtable_.symSetOptions != nullptr)
        {
            vtable_.symInitialize = reinterpret_cast<PFN_SYMINITIALIZE_PROC>(module_.LoadProcedure("SymInitialize"));
            if (vtable_.symInitialize != nullptr)
            {
                vtable_.symSetOptions(SYMOPT_UNDNAME | SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS);
                if (vtable_.symInitialize(process_, nullptr, TRUE) != FALSE)
                {
                    vtable_.symFromAddr             = reinterpret_cast<PFN_SYMFROMADDR_PROC>(module_.LoadProcedure("SymFromAddr"));
                    vtable_.symFromAddrW            = reinterpret_cast<PFN_SYMFROMADDRW_PROC>(module_.LoadProcedure("SymFromAddrW"));
                    vtable_.symGetLineFromAddr      = reinterpret_cast<PFN_SYMGETLINEFROMADDR_PROC>(module_.LoadProcedure("SymGetLineFromAddr"));
                    vtable_.symGetLineFromAddr64    = reinterpret_cast<PFN_SYMGETLINEFROMADDR64_PROC>(module_.LoadProcedure("SymGetLineFromAddr64"));
                    vtable_.symGetLineFromAddrW64   = reinterpret_cast<PFN_SYMGETLINEFROMADDRW64_PROC>(module_.LoadProcedure("SymGetLineFromAddrW64"));
                }
            }
        }
    }
}

DbgHelpModule& DbgHelpModule::Get()
{
    static DbgHelpModule instance;
    return instance;
}

UTF8String DbgHelpModule::AddressToSymbolName(const void* addr) const
{
    constexpr ULONG maxNameLength = 1024;

    DWORD64 addr64 = static_cast<DWORD64>(reinterpret_cast<std::size_t>(addr));
    DWORD64 displacement = 0;

    if (vtable_.symFromAddrW != nullptr)
    {
        constexpr ULONG infoBufferSize = sizeof(SYMBOL_INFOW) + (maxNameLength - 1)*sizeof(WCHAR);
        char infoBuffer[infoBufferSize] = {};
        SYMBOL_INFOW* info = reinterpret_cast<SYMBOL_INFOW*>(infoBuffer);
        {
            info->SizeOfStruct  = sizeof(SYMBOL_INFOW);
            info->MaxNameLen    = maxNameLength;
        }
        if (vtable_.symFromAddrW(process_, addr64, &displacement, info))
            return info->Name;
    }
    else if (vtable_.symFromAddr != nullptr)
    {
        constexpr ULONG infoBufferSize = sizeof(SYMBOL_INFO) + (maxNameLength - 1)*sizeof(CHAR);
        char infoBuffer[infoBufferSize] = {};
        SYMBOL_INFO* info = reinterpret_cast<SYMBOL_INFO*>(infoBuffer);
        {
            info->SizeOfStruct  = sizeof(SYMBOL_INFO);
            info->MaxNameLen    = maxNameLength;
        }
        if (vtable_.symFromAddr(process_, addr64, &displacement, info))
            return info->Name;
    }

    return "";
}

UTF8String DbgHelpModule::AddressToSourceInfo(const void* addr) const
{
    DWORD64 addr64 = static_cast<DWORD64>(reinterpret_cast<std::size_t>(addr));
    DWORD displacement = 0;

    UTF8String s;

    if (vtable_.symGetLineFromAddrW64 != nullptr)
    {
        IMAGEHLP_LINEW64 info = {};
        info.SizeOfStruct = sizeof(IMAGEHLP_LINEW64);

        if (vtable_.symGetLineFromAddrW64(process_, addr64, &displacement, &info))
        {
            s = info.FileName;
            s += ':';
            s += IntToStr(info.LineNumber);
        }
    }
    else if (vtable_.symGetLineFromAddr64 != nullptr)
    {
        IMAGEHLP_LINE64 info = {};
        info.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

        if (vtable_.symGetLineFromAddr64(process_, addr64, &displacement, &info))
        {
            s = info.FileName;
            s += ':';
            s += IntToStr(info.LineNumber);
        }
    }
    else if (vtable_.symGetLineFromAddr != nullptr)
    {
        IMAGEHLP_LINE info = {};
        info.SizeOfStruct = sizeof(IMAGEHLP_LINE);

        const DWORD addr32 = static_cast<DWORD>(addr64);
        if (vtable_.symGetLineFromAddr(process_, addr32, &displacement, &info))
        {
            s = info.FileName;
            s += ':';
            s += IntToStr(info.LineNumber);
        }
    }

    return s;
}


LLGL_EXPORT void DebugPuts(const char* text)
{
    #ifdef LLGL_DEBUG
    if (IsDebuggerPresent())
    {
        /* Print to Visual Debugger */
        OutputDebugStringA(text);
        OutputDebugStringA("\n");
    }
    else
    #endif
    {
        /* Print to standard error stream */
        ::fprintf(stderr, "%s\n", text);
    }
}

static std::vector<void*> CaptureStackTraceAddresses(DWORD framesToSkip, DWORD framesToCapture)
{
    std::vector<void*> backTrace;
    backTrace.resize(framesToCapture);

    const WORD capturedFrames = CaptureStackBackTrace(framesToSkip, framesToCapture, backTrace.data(), nullptr);
    if (capturedFrames > 0)
        backTrace.resize(static_cast<std::size_t>(capturedFrames));
    else
        backTrace.clear();

    return backTrace;
}

LLGL_EXPORT UTF8String DebugStackTrace(unsigned firstStackFrame, unsigned maxNumStackFrames)
{
    constexpr   DWORD framesToAlwaysSkip    = 1; // Always skip stack frame of 'CaptureStackTraceAddresses'
    const       DWORD framesToCapture       = (std::max<DWORD>)(maxNumStackFrames, USHRT_MAX);
    const       DWORD framesToSkip          = (std::max<DWORD>)(0, firstStackFrame) + framesToAlwaysSkip;

    std::vector<void*> framePointers = CaptureStackTraceAddresses(framesToSkip, framesToCapture);

    /* Build chart with columns for stack frames and source information */
    std::vector<UTF8String> columns[2];
    UTF8String cells[2];
    std::size_t columnWidths[2] = { 0, 0 };

    for (auto it = framePointers.rbegin(); it != framePointers.rend(); ++it)
    {
        void* ptr = *it;

        /* Append stack frame address */
        cells[0] += '[';
        cells[0] += IntToHex(reinterpret_cast<std::size_t>(ptr));
        cells[0] += ']';

        /* Append symbol name */
        const UTF8String symbolName = DbgHelpModule::Get().AddressToSymbolName(ptr);
        if (!symbolName.empty())
        {
            cells[0] += ' ';
            cells[0] += symbolName;
        }

        /* Append source information */
        const UTF8String sourceInfo = DbgHelpModule::Get().AddressToSourceInfo(ptr);
        if (!sourceInfo.empty())
            cells[1] += sourceInfo;

        /* Append current lines to rows */
        for_range(i, 2)
        {
            columnWidths[i] = (std::max)(columnWidths[i], cells[i].size());
            columns[i].push_back(std::move(cells[i]));
        }
    }

    /* Build output string from chart */
    FormattedTableColumn tableColumns[2];
    {
        tableColumns[0].maxWidth        = 80;
        tableColumns[0].multiLineIndent = 2;
        tableColumns[0].cells           = columns[0];
        tableColumns[1].cells           = columns[1];
    }
    return WriteTableToUTF8String(tableColumns);
}


} // /namespace LLGL



// ================================================================================
