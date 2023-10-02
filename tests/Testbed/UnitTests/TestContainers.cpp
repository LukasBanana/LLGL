/*
 * TestContainers.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Container/DynamicArray.h>
#include <LLGL/Container/SmallVector.h>
#include <LLGL/Container/UTF8String.h>
#include <LLGL/Container/Strings.h>
#include <locale>
#include <codecvt>


TestResult TestbedContext::TestContainerDynamicArray()
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

TestResult TestbedContext::TestContainerSmallVector()
{
    const int cmpInt16[16] = { 1,2,3,4, 42,3476,93,-12, 0xFF,0xCD,0x10,0xDE, 384723,901872,-874673,1234567 };

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

    #define TEST_SMALL_VECTOR(VEC, CMP)                                                                     \
        {                                                                                                   \
            const TestResult result = TestSmallVector(#VEC, VEC.data(), CMP, VEC.size() * sizeof(VEC[0]));  \
            if (result != TestResult::Passed)                                                               \
                return result;                                                                              \
        }

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

    return TestResult::Passed;
}

TestResult TestbedContext::TestContainerUTF8String()
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

    return TestResult::Passed;
}

