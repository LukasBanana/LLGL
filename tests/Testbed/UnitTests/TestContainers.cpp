/*
 * TestContainers.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Container/DynamicArray.h>
#include <LLGL/Container/SmallVector.h>
#include <LLGL/Container/Strings.h>
#include <LLGL/Utils/ForRange.h>
#include <locale>
#include <codecvt> //TODO: replace this as it's deprecated in C++17
#include <string>
#include <vector>
#include <initializer_list>
#include <algorithm>


DEF_RITEST( ContainerDynamicArray )
{
    // Test byte array
    const char* cmp8BytesZero   = "\0\0\0\0\0\0\0\0";
    const char* cmp8BytesCharF  = "ffffffff";

    DynamicByteArray ba1{ 8 };
    DynamicByteArray ba2{ 8, cmp8BytesCharF[0] };

    auto TestByteArray = [](const char* name, const DynamicByteArray& arr, const void* cmpData) -> TestResult
    {
        const std::size_t dataSize = arr.size();
        if (::memcmp(arr.data(), cmpData, dataSize) != 0)
        {
            const std::string arrStr = TestbedContext::FormatByteArray(arr.data(), dataSize, sizeof(char));
            const std::string cmpStr = TestbedContext::FormatByteArray(cmpData, dataSize, sizeof(char));
            Log::Errorf(
                "Mismatch between DynamicByteArray '%s{%u}' [%s] and initial data [%s]\n",
                name, static_cast<unsigned>(arr.size()), arrStr.c_str(), cmpStr.c_str()
            );
            return TestResult::FailedMismatch;
        }
        return TestResult::Passed;
    };

    #define TEST_BYTE_ARRAY(ARR, CMP)                                   \
        {                                                               \
            const TestResult result = TestByteArray(#ARR, ARR, CMP);    \
            if (result != TestResult::Passed)                           \
                return result;                                          \
        }

    TEST_BYTE_ARRAY(ba1, cmp8BytesZero);
    TEST_BYTE_ARRAY(ba2, cmp8BytesCharF);

    // Test int array
    const int cmp8Ints[8] = { 42,42,42,42, 16,16,16,16 };

    DynamicArray<int> ia1{ 4u, cmp8Ints[0] };
    DynamicArray<int> ia2{ cmp8Ints, cmp8Ints + 8 };

    auto TestIntArray = [](const char* name, const DynamicArray<int>& arr, const void* cmpData) -> TestResult
    {
        const std::size_t dataSize = arr.size()*sizeof(int);
        if (::memcmp(arr.data(), cmpData, dataSize) != 0)
        {
            const std::string arrStr = TestbedContext::FormatByteArray(arr.data(), dataSize, sizeof(int));
            const std::string cmpStr = TestbedContext::FormatByteArray(cmpData, dataSize, sizeof(int));
            Log::Errorf(
                "Mismatch between DynamicArray<int> '%s{%u}' [%s] and initial data [%s]\n",
                name, static_cast<unsigned>(arr.size()), arrStr.c_str(), cmpStr.c_str()
            );
            return TestResult::FailedMismatch;
        }
        return TestResult::Passed;
    };

    #define TEST_INT_ARRAY(ARR, CMP)                                \
        {                                                           \
            const TestResult result = TestIntArray(#ARR, ARR, CMP); \
            if (result != TestResult::Passed)                       \
                return result;                                      \
        }

    TEST_INT_ARRAY(ia1, cmp8Ints);
    TEST_INT_ARRAY(ia2, cmp8Ints);

    ia1.resize(8, cmp8Ints[4]);

    TEST_INT_ARRAY(ia1, cmp8Ints);

    // Test structured array
    struct TrivialStruct
    {
        int     a;
        float   b;
    };

    const TrivialStruct cmp4StructsZero[4] = {};
    const TrivialStruct cmp4Structs_16_25[2] = { { 16, 2.5f }, { 16, 2.5f } };
    const TrivialStruct cmp4StructsRandom[4] = { { 1, 1.0f }, { 2, 2.0f }, { 3, .14f }, { 42, 66.3f } };

    DynamicArray<TrivialStruct> sa1{ 4 };
    DynamicArray<TrivialStruct> sa2{ 2, cmp4Structs_16_25[0] };
    DynamicArray<TrivialStruct> sa3{ cmp4StructsRandom, cmp4StructsRandom + sizeof(cmp4StructsRandom)/sizeof(cmp4StructsRandom[0]) };

    auto TestStructuredArray = [](const char* name, const DynamicArray<TrivialStruct>& arr, const void* cmpData) -> TestResult
    {
        const std::size_t dataSize = arr.size()*sizeof(int);
        if (::memcmp(arr.data(), cmpData, dataSize) != 0)
        {
            const std::string arrStr = TestbedContext::FormatByteArray(arr.data(), dataSize, sizeof(TrivialStruct));
            const std::string cmpStr = TestbedContext::FormatByteArray(cmpData, dataSize, sizeof(TrivialStruct));
            Log::Errorf(
                "Mismatch between DynamicArray<TrivialStruct> '%s{%u}' [%s] and initial data [%s]\n",
                name, static_cast<unsigned>(arr.size()), arrStr.c_str(), cmpStr.c_str()
            );
            return TestResult::FailedMismatch;
        }
        return TestResult::Passed;
    };

    #define TEST_STRUCT_ARRAY(ARR, CMP)                                     \
        {                                                                   \
            const TestResult result = TestStructuredArray(#ARR, ARR, CMP);  \
            if (result != TestResult::Passed)                               \
                return result;                                              \
        }

    TEST_STRUCT_ARRAY(sa1, cmp4StructsZero);
    TEST_STRUCT_ARRAY(sa2, cmp4Structs_16_25);
    TEST_STRUCT_ARRAY(sa3, cmp4StructsRandom);

    return TestResult::Passed;
}

DEF_RITEST( ContainerSmallVector )
{
    constexpr int cmpInt16[16] = { 1,2,3,4, 42,3476,93,-12, 0xFF,0xCD,0x10,0xDE, 384723,901872,-874673,1234567 };
    constexpr int cmpInt4[4] = { 4,3,2,1 };
    constexpr int cmpInt0[1] = { 0 };

    auto TestSmallVector = [](const char* name, const void* vec, const void* cmp, std::size_t size) -> TestResult
    {
        if (::memcmp(vec, cmp, size) != 0)
        {
            const std::string vecStr = TestbedContext::FormatByteArray(vec, size, 4);
            const std::string cmpStr = TestbedContext::FormatByteArray(cmp, size, 4);
            Log::Errorf(
                "Mismatch between SmallVector '%s' [%s] and initial data [%s]\n",
                name, vecStr.c_str(), cmpStr.c_str()
            );
            return TestResult::FailedMismatch;
        }
        return TestResult::Passed;
    };

    #define TEST_SMALL_VECTOR_EXT(VEC, CMP, SIZE)                                                           \
        {                                                                                                   \
            const std::size_t expectedSize = (SIZE);                                                        \
            if (VEC.size() != expectedSize)                                                                 \
            {                                                                                               \
                Log::Errorf(                                                                                \
                    "Mismatch between SmallVector '%s' size (%zu) and expected size (%zu\n)",               \
                    #VEC, VEC.size(), expectedSize                                                          \
                );                                                                                          \
                return TestResult::FailedMismatch;                                                          \
            }                                                                                               \
            const TestResult result = TestSmallVector(#VEC, VEC.data(), CMP, VEC.size() * sizeof(VEC[0]));  \
            if (result != TestResult::Passed)                                                               \
                return result;                                                                              \
        }

    #define TEST_SMALL_VECTOR(VEC, CMP) \
        TEST_SMALL_VECTOR_EXT(VEC, CMP, VEC.size())

    // Test basic initialization with local capacity
    SmallVector<int, 4> iv4_4 = { 1, 2, 3, 4 };
    SmallVector<int, 4> iv4_8 = { cmpInt16, cmpInt16 + 8 };
    SmallVector<int, 4> iv4_16;

    iv4_16.insert(iv4_16.begin(), cmpInt16, cmpInt16 + 16);
    TEST_SMALL_VECTOR(iv4_4,  cmpInt16);
    TEST_SMALL_VECTOR(iv4_8,  cmpInt16);
    TEST_SMALL_VECTOR(iv4_16, cmpInt16);

    // Test basic initialization with dynamic capacity only
    SmallVector<int, 0> iv0_4 = { 1, 2, 3, 4 };
    SmallVector<int, 0> iv0_8 = { cmpInt16, cmpInt16 + 8 };
    SmallVector<int, 0> iv0_16;

    iv0_16.insert(iv0_16.begin(), cmpInt16, cmpInt16 + 16);
    TEST_SMALL_VECTOR(iv0_4,  cmpInt16);
    TEST_SMALL_VECTOR(iv0_8,  cmpInt16);
    TEST_SMALL_VECTOR(iv0_16, cmpInt16);

    // Test inserting elements beyond static capacity
    SmallVector<int, 2> iv2_n;
    std::vector<int> iv_std;

    for (int i = 0; i < 1024; ++i)
    {
        iv2_n.push_back(i);
        iv_std.push_back(i);
    }

    TEST_SMALL_VECTOR(iv2_n, iv_std.data());

    // Test swapping containers
    iv4_4.swap(iv4_8);
    TEST_SMALL_VECTOR_EXT(iv4_4, cmpInt16, 8);

    iv4_4.swap(iv4_8);
    TEST_SMALL_VECTOR_EXT(iv4_4, cmpInt16, 4);

    // Test move semantics
    iv0_4 = std::move(iv0_8);
    TEST_SMALL_VECTOR_EXT(iv0_4, cmpInt16, 8);
    TEST_SMALL_VECTOR_EXT(iv0_8, cmpInt0, 0);

    iv4_4 = std::move(iv4_16);
    TEST_SMALL_VECTOR_EXT(iv4_4,  cmpInt16, 16);
    TEST_SMALL_VECTOR_EXT(iv4_16, cmpInt0,   0);

    SmallVector<int, 5> iv5_4a = { 4,3,2,1 };
    SmallVector<int, 5> iv5_4b = { 1,2,3,4 };

    TEST_SMALL_VECTOR_EXT(iv5_4a, cmpInt4,  4);
    TEST_SMALL_VECTOR_EXT(iv5_4b, cmpInt16, 4);
    iv5_4a = std::move(iv5_4b);
    TEST_SMALL_VECTOR_EXT(iv5_4a, cmpInt16, 4);
    TEST_SMALL_VECTOR_EXT(iv5_4b, cmpInt0,  0);

    return TestResult::Passed;
}

DEF_RITEST( ContainerUTF8String )
{
    // Test UTF8String concatentation
    UTF8String sa1 = "Hello";
    UTF8String sa2 = " ";
    UTF8String sa3 = "World";
    UTF8String sa4 = sa1 + sa2 + sa3;

    const char* sa4Expected = "Hello World";
    if (sa4 != sa4Expected)
    {
        Log::Errorf(
            "Mismatch between UTF8String concatenation 'sa4' \"%s\" and initial value \"%s\"\n",
            sa4.c_str(), sa4Expected
        );
        return TestResult::FailedMismatch;
    }

    // Test unicode characters
    UTF8String su1 = L"Hello ";
    UTF8String su2 = L"\x4E16\x754C\x3002";
    UTF8String su3 = su1 + su2;

    const char* su3Expected = "Hello \xE4\xB8\x96\xE7\x95\x8C\xE3\x80\x82";
    if (su3 != su3Expected)
    {
        Log::Errorf(
            "Mismatch between UTF8String concatenation 'su3' \"%s\" and initial value \"%s\"\n",
            su3.c_str(), su3Expected
        );
        return TestResult::FailedMismatch;
    }

    SmallVector<wchar_t> su3UTF16 = su3.to_utf16();
    std::wstring su3Wide = su3UTF16.data();
    const wchar_t* su3WideExpected = L"Hello \x4E16\x754C\x3002";
    if (su3Wide != su3WideExpected)
    {
        const std::string su3Ansi           = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.to_bytes(su3Wide);
        const std::string su3AnsiExpected   = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.to_bytes(su3WideExpected);
        Log::Errorf(
            "Mismatch between UTF8String concatenation 'su3Wide' \"%s\" and initial value \"%s\"\n",
            su3Ansi.c_str(), su3AnsiExpected.c_str()
        );
        return TestResult::FailedMismatch;
    }

    // Test resize
    UTF8String sa5 = "Hello";
    sa5 += ' ';
    sa5 += "World";

    const char* sa5Expected = "Hello World";
    if (sa5 != sa4Expected)
    {
        Log::Errorf(
            "Mismatch between UTF8String concatenation 'sa5' \"%s\" and initial value \"%s\"\n",
            sa5.c_str(), sa5Expected
        );
        return TestResult::FailedMismatch;
    }

    const StringView boilerplate = "This is a simple boilerplate text to be used for testing purposes only";

    UTF8String sa6;

    for_range(i, 10)
    {
        const StringView subA = boilerplate.substr(i, 5);
        const StringView subB = boilerplate.substr(20 + i, 10 + i);

        sa6 = subA;
        sa6 += subB;

        const std::string sa6Expected = std::string(subA.begin(), subA.end()) + std::string(subB.begin(), subB.end());

        if (sa6.size() != subA.size() + subB.size() || ::memcmp(sa6.data(), sa6Expected.data(), sa6Expected.size()) != 0)
        {
            Log::Errorf(
                "Mismatch between UTF8String concatenation 'sa6' \"%s\" and initial value \"%s\"\n",
                sa6.c_str(), sa6Expected.c_str()
            );
            return TestResult::FailedMismatch;
        }

        sa6.clear();
    }

    return TestResult::Passed;
}

struct TestLinearAllocator
{
    char* allocate(std::size_t n, const void* hint = nullptr)
    {
        char* p = &(TestLinearAllocator::data[pos]);
        TestLinearAllocator::pos += n;
        TestLinearAllocator::counter += n;
        return p;
    }

    void deallocate(char* p, std::size_t n)
    {
        // dummy
    }

    static std::size_t GetAndFlushCounter()
    {
        std::size_t n = TestLinearAllocator::counter;
        TestLinearAllocator::counter = 0;
        return n;
    }

    static constexpr std::size_t capacity = 1024;
    static std::size_t counter;
    static std::size_t pos;
    static char data[capacity];
};

std::size_t TestLinearAllocator::counter = 0;
std::size_t TestLinearAllocator::pos = 0;
char TestLinearAllocator::data[capacity];

DEF_RITEST( ContainerStringLiteral )
{
    // Test reference and dynamic string literals
    {
        StringLiteral l0 = "This is a string literal";
        StringLiteral l1 = StringLiteral{ l0.c_str(), CopyTag{} };

        if (l0 != l1)
        {
            Log::Errorf("Mismatch between reference and dynamic string literals: l0=\"%s\" and l1=\"%s\"\n", l0.c_str(), l1.c_str());
            return TestResult::FailedMismatch;
        }
    }

    // Test dynamic strings with custom allocator to track allocation size
    {
        using TestStringLiteral = BasicStringLiteral<char, std::char_traits<char>, TestLinearAllocator>;

        TestStringLiteral l2 = "This is a reference string literal";
        const std::size_t l2DynamicLen = TestLinearAllocator::GetAndFlushCounter();

        TestStringLiteral l3 = StringView{ "This is a dynamic string literal" };
        const std::size_t l3DynamicLen = TestLinearAllocator::GetAndFlushCounter();

        const std::size_t commonSubstrLen = TestStringLiteral{ "This is a " }.size();
        if (l2.compare(0, commonSubstrLen, l3, 0, commonSubstrLen) != 0)
        {
            Log::Errorf("Mismatch between reference and dynamic sub-string literals:\n -> l2 = \"%s\" and l3 = \"%s\"\n", l2.c_str(), l3.c_str());
            return TestResult::FailedMismatch;
        }

        if (l2DynamicLen != 0)
        {
            Log::Errorf("Expected l2 string to be reference, but dynamic length is %zu\n", l2DynamicLen);
            return TestResult::FailedMismatch;
        }

        if (l3DynamicLen != l3.size() + 1)
        {
            Log::Errorf("Expected l3 string to be dynamic with length %zu, but length is %zu\n", (l3.size() + 1), l3DynamicLen);
            return TestResult::FailedMismatch;
        }

        // Use after copy
        l2 = l3;
        if (l2 != l3)
        {
            Log::Errorf("Expected l2 and l3 strings to be equal:\n -> l2 = \"%s\" and l3 = \"%s\"\n", l2.c_str(), l3.c_str());
            return TestResult::FailedMismatch;
        }

        // Use after move
        l2 = std::move(l3);
        if (l2 == l3)
        {
            Log::Errorf("Expected l2 and l3 strings to be non-equal:\n -> l2 = \"%s\" and l3 = \"%s\"\n", l2.c_str(), l3.c_str());
            return TestResult::FailedMismatch;
        }

        l2.clear();
        l3.clear();
    }

    // Test string view to literal conversion
    {
        StringLiteral l4 = "This is a slightly longer string to test memory boundaries.";
        StringLiteral l5 = StringView{ l4 };

        if (l4 != l5)
        {
            Log::Errorf("Mismatch between reference and dynamic string literals: l4=\"%s\" and l5=\"%s\"\n", l4.c_str(), l5.c_str());
            return TestResult::FailedMismatch;
        }
    }

    // Test absence of ambiguity (Compile-time only test)
    {
        StringLiteral l6{ std::string("Test") };
        StringLiteral l7{ l6 };
    }

    return TestResult::Passed;
}

// Fills a list of STL strings and LLGL's own strings with the same entries.
// Then sorts them with std::sort() and ensures that both lists are ordered equally.
template <typename T>
TestResult TestStringSort(const std::initializer_list<const char*> inStrings, bool sanityCheck, const char* llglStringTypeName)
{
    // Fill both STL and LLGL string containers
    std::vector<std::string> stdStrings;
    DynamicVector<T> llglStrings;

    stdStrings.reserve(inStrings.size());
    llglStrings.reserve(inStrings.size());

    for (const char* s : inStrings)
    {
        stdStrings.push_back(s);
        llglStrings.push_back(s);
    }

    // Sort both containers
    std::sort(stdStrings.begin(), stdStrings.end());
    std::sort(llglStrings.begin(), llglStrings.end());

    // Ensure both containers are equally sorted
    if (stdStrings.size() != llglStrings.size())
    {
        Log::Errorf(
            "Mismatch between STL string container size (%zu) and LLGL string container size (%zu)\n",
            stdStrings.size(), llglStrings.size()
        );
        return TestResult::FailedMismatch;
    }

    auto PrintStringChart = [&stdStrings, &llglStrings, llglStringTypeName](bool printAsErrors) -> void
    {
        constexpr std::size_t chartColumnDist = 20; // Distance between beginning of "STL strings:" and "LLGL strings:"
        const char* caption =
        (
            "std::string         %s\n"
            "-----------         %s\n"
        );
        const std::string llglStringTypeUnderline(::strlen(llglStringTypeName), '-');

        if (printAsErrors)
            Log::Errorf(caption, llglStringTypeName, llglStringTypeUnderline.c_str());
        else
            Log::Printf(Log::ColorFlags::StdAnnotation, caption, llglStringTypeName, llglStringTypeUnderline.c_str());

        for_range(i, stdStrings.size())
        {
            const std::string lhs{ stdStrings[i].data(), stdStrings[i].size() };
            const std::string rhs{ llglStrings[i].data(), llglStrings[i].size() };
            const std::string spaces(lhs.size() < chartColumnDist ? chartColumnDist - lhs.size() : 1, ' ');

            if (printAsErrors)
                Log::Errorf("%s%s%s\n", lhs.c_str(), spaces.c_str(), rhs.c_str());
            else
                Log::Printf(Log::ColorFlags::StdAnnotation, "%s%s%s\n", lhs.c_str(), spaces.c_str(), rhs.c_str());
        }

        Log::Printf("\n");
    };

    for_range(i, stdStrings.size())
    {
        if (stdStrings[i].size() != llglStrings[i].size() ||
            ::strncmp(stdStrings[i].data(), llglStrings[i].data(), stdStrings[i].size()) != 0)
        {
            // Print list side by side
            Log::Errorf("Mismatch between order of sorted STL string container and LLGL string container:\n");
            PrintStringChart(true);
            return TestResult::FailedMismatch;
        }
    }

    // Print sorted list for sanity check
    if (sanityCheck)
        PrintStringChart(false);

    return TestResult::Passed;
}

DEF_RITEST( ContainerStringOperators )
{
    std::initializer_list<const char*> inStrings
    {
        "Hello", "World", "!", "This", "string", "must", "be", "properly", "sorted", ".", "5", "4", "3", "2", "1", "Go!"
    };

    #define TEST_STRING_OPERATORS(TYPE)                                                     \
        {                                                                                   \
            TestResult result = TestStringSort<TYPE>(inStrings, opt.sanityCheck, #TYPE);    \
            if (result != TestResult::Passed)                                               \
                return result;                                                              \
        }

    TEST_STRING_OPERATORS(LLGL::UTF8String);
    TEST_STRING_OPERATORS(LLGL::StringView);
    TEST_STRING_OPERATORS(LLGL::StringLiteral);

    return TestResult::Passed;
}

