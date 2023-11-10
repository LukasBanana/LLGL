/*
 * StringUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "StringUtils.h"
#include <fstream>
#include <codecvt>
#include <locale>
#include <stdio.h>
#include <algorithm>
#include <LLGL/Container/SmallVector.h>
#include <LLGL/Utils/ForRange.h>
#include "../Platform/Path.h"


namespace LLGL
{


// Returns the specified filename either unchanged or as absolute path for mobile platforms.
static UTF8String GetPlatformAppropriateFilename(const char* filename)
{
    #ifdef LLGL_MOBILE_PLATFORM
    return Path::GetAbsolutePath(filename);
    #else
    return filename;
    #endif
}

LLGL_EXPORT std::string ReadFileString(const char* filename)
{
    /* Read file content into string */
    const UTF8String path = GetPlatformAppropriateFilename(filename);
    std::ifstream file{ path.c_str() };
    if (file.good())
    {
        return std::string
        {
            ( std::istreambuf_iterator<char>(file) ),
            ( std::istreambuf_iterator<char>() )
        };
    }
    return "";
}

LLGL_EXPORT std::vector<char> ReadFileBuffer(const char* filename)
{
    /* Read file content into buffer */
    const UTF8String path = GetPlatformAppropriateFilename(filename);
    std::ifstream file{ path.c_str(), (std::ios_base::binary | std::ios_base::ate) };
    if (file.good())
    {
        const std::size_t fileSize = static_cast<std::size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        return buffer;
    }
    return {};
}

LLGL_EXPORT std::string ToUTF8String(const std::wstring& utf16)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.to_bytes(utf16);
}

LLGL_EXPORT std::string ToUTF8String(const wchar_t* utf16)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.to_bytes(utf16);
}

LLGL_EXPORT std::wstring ToUTF16String(const std::string& utf8)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.from_bytes(utf8);
}

LLGL_EXPORT std::wstring ToUTF16String(const char* utf8)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.from_bytes(utf8);
}

void StringPrintf(std::string& str, const char* format, va_list args1, va_list args2)
{
    const int len = ::vsnprintf(nullptr, 0, format, args1);
    if (len > 0)
    {
        /*
        Since C++11 we can override the last character with '\0' ourselves,
        so it's safe to let ::vsnprintf override std::string from [0, size()] inclusive.
        */
        const std::size_t formatLen = static_cast<std::size_t>(len);
        const std::size_t appendOff = str.size();
        str.resize(appendOff + formatLen);
        ::vsnprintf(&str[appendOff], formatLen + 1, format, args2);
    }
}

LLGL_EXPORT UTF8String WriteTableToUTF8String(const ArrayView<FormattedTableColumn>& columns)
{
    UTF8String s;

    if (columns.empty())
        return s;

    /* Determine maximum width for each column */
    SmallVector<std::size_t> columnWidths;
    columnWidths.resize(columns.size() - 1);

    std::size_t maxNumRows = 0;

    for_range(i, columns.size() - 1)
    {
        const FormattedTableColumn& col = columns[i];

        /* Find longest row width for current column */
        for (const UTF8String& cell : col.cells)
            columnWidths[i] = std::max<std::size_t>(columnWidths[i], cell.size());

        /* Apply limit per column and find maximum number of rows */
        columnWidths[i] = std::min<std::size_t>(columnWidths[i], col.maxWidth);
        maxNumRows      = std::max<std::size_t>(maxNumRows, col.cells.size());
    }

    SmallVector<StringView> multiRowQueue;
    multiRowQueue.resize(columns.size());

    bool hasMultiLineCells = false;

    /* Format each row for all columns */
    for_range(row, maxNumRows)
    {
        do
        {
            hasMultiLineCells = false;

            for_range(col, columns.size())
            {
                const auto& entry = columns[col];
                if (row < entry.cells.size())
                {
                    /* Append indentation */
                    const std::size_t indent = (multiRowQueue[col].empty() ? 0 : std::min<std::size_t>(entry.multiLineIndent, columnWidths[col]/2));
                    s.append(indent, ' ');

                    const StringView cell = (!multiRowQueue[col].empty() ? multiRowQueue[col] : entry.cells[row]);
                    if (!(col < columnWidths.size() && cell.size() + indent > columnWidths[col]))
                    {
                        /* Append cell */
                        s += cell;
                        if (col + 1 < columns.size())
                            s.append(columnWidths[col] - cell.size() - indent, ' ');
                        multiRowQueue[col] = StringView{};
                    }
                    else
                    {
                        /* Find position where to split current cell */
                        const std::size_t maxCellWidth  = columnWidths[col] - indent;
                        const std::size_t delimiterPos  = cell.find_last_of(":;., ", maxCellWidth);
                        const std::size_t splitPos      = (delimiterPos == StringView::npos ? maxCellWidth : delimiterPos + 1);

                        /* Split cell into head and tail */
                        const StringView cellHead = cell.substr(0, splitPos);
                        const StringView cellTail = cell.substr(cellHead.size());

                        /* Append partial cell and add tail to multi-row list */
                        s += cellHead;
                        s.append(columnWidths[col] - cellHead.size() - indent, ' ');

                        /* Trim cell via delimiter characters */
                        multiRowQueue[col] = cellTail;
                        if (!cellTail.empty())
                            hasMultiLineCells = true;
                    }
                }
                else if (col + 1 < columns.size())
                {
                    /* Fill with blanks */
                    s.append(columnWidths[col], ' ');
                }

                if (col + 1 < columns.size())
                {
                    /* Append column separator */
                    s += " | ";
                }
            }

            s += '\n';
        }
        while (hasMultiLineCells);
    }

    return s;
}


} // /namespace LLGL



// ================================================================================
