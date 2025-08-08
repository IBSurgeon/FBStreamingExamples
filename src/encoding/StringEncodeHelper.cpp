#include "StringEncodeHelper.h"

#include "../common/Utils.h"

namespace Firebird {

using std::string;
using std::string_view;
using std::u16string;
using std::u16string_view;
using std::u32string;
using std::u32string_view;
using std::wstring;
using std::wstring_view;
using FbUtils::IscRandomStatus;

StringEncodeHelper::StringEncodeHelper(IStringEncodeUtils* const encodeUtil)
    : m_encodeUtil(encodeUtil)
{
}

IStringEncodeUtils* StringEncodeHelper::getStringEncodeUtils() const
{
    return m_encodeUtil;
}

StringConverterHelper StringEncodeHelper::getConverterById(ThrowStatusWrapper* status, unsigned charsetId) const
{
    return StringConverterHelper(m_encodeUtil->getConverterById(status, charsetId));
}

StringConverterHelper StringEncodeHelper::getConverterByName(ThrowStatusWrapper* status, string_view charsetName) const
{
    string sCharsetName(charsetName);
    return StringConverterHelper(m_encodeUtil->getConverterByName(status, sCharsetName.c_str()));
}

size_t StringEncodeHelper::convertCharset(ThrowStatusWrapper* status, const StringConverterHelper& srcConveter,
    const StringConverterHelper& destConveter, const char* src, size_t srcSize, char* destBuffer,
    size_t destBufferSize) const
{
    return static_cast<size_t>(m_encodeUtil->convertCharset(
        status,
        srcConveter.m_converter,
        destConveter.m_converter,
        src,
        static_cast<ISC_UINT64>(srcSize),
        destBuffer,
        static_cast<ISC_UINT64>(destBufferSize)));
}

string StringEncodeHelper::convertCharset(ThrowStatusWrapper* status, const StringConverterHelper& srcConveter,
    const StringConverterHelper& destConveter, string_view src) const
try {
    size_t resultCapacity = src.size() / srcConveter.m_converter->getMinCharSize() * destConveter.m_converter->getMaxCharSize();
    string result(resultCapacity, '\0');
    size_t resultLength = convertCharset(status, srcConveter, destConveter, src.data(), src.size(), result.data(), resultCapacity);
    result.resize(resultLength);
    return result;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

size_t StringEncodeHelper::convertUtf8ToWCS(ThrowStatusWrapper* status, const char* src, size_t srcSize,
    wchar_t* destBuffer, size_t destBufferSize) const
{
    return static_cast<size_t>(m_encodeUtil->convertUtf8ToWCS(
        status,
        src,
        static_cast<ISC_UINT64>(srcSize),
        destBuffer,
        static_cast<ISC_UINT64>(destBufferSize)));
}

size_t StringEncodeHelper::convertUtf8ToUtf16(ThrowStatusWrapper* status, const char* src, size_t srcSize,
    char16_t* destBuffer, size_t destBufferSize) const
{
    return static_cast<size_t>(m_encodeUtil->convertUtf8ToUtf16(
        status,
        src,
        static_cast<ISC_UINT64>(srcSize),
        destBuffer,
        static_cast<ISC_UINT64>(destBufferSize)));
}

size_t StringEncodeHelper::convertUtf8ToUtf32(ThrowStatusWrapper* status, const char* src, size_t srcSize,
    char32_t* destBuffer, size_t destBufferSize) const
{
    return static_cast<size_t>(m_encodeUtil->convertUtf8ToUtf32(
        status,
        src,
        static_cast<ISC_UINT64>(srcSize),
        destBuffer,
        static_cast<ISC_UINT64>(destBufferSize)));
}

size_t StringEncodeHelper::convertUtf16ToWCS(ThrowStatusWrapper* status, const char16_t* src, size_t srcSize,
    wchar_t* destBuffer, size_t destBufferSize) const
{
    return static_cast<size_t>(m_encodeUtil->convertUtf16ToWCS(
        status,
        src,
        static_cast<ISC_UINT64>(srcSize),
        destBuffer,
        static_cast<ISC_UINT64>(destBufferSize)));
}

size_t StringEncodeHelper::convertUtf16ToUtf8(ThrowStatusWrapper* status, const char16_t* src, size_t srcSize,
    char* destBuffer, size_t destBufferSize) const
{
    return static_cast<size_t>(m_encodeUtil->convertUtf16ToUtf8(
        status,
        src,
        static_cast<ISC_UINT64>(srcSize),
        destBuffer,
        static_cast<ISC_UINT64>(destBufferSize)));
}

size_t StringEncodeHelper::convertUtf16ToUtf32(ThrowStatusWrapper* status, const char16_t* src, size_t srcSize,
    char32_t* destBuffer, size_t destBufferSize) const
{
    return static_cast<size_t>(m_encodeUtil->convertUtf16ToUtf32(
        status,
        src,
        static_cast<ISC_UINT64>(srcSize),
        destBuffer,
        static_cast<ISC_UINT64>(destBufferSize)));
}

size_t StringEncodeHelper::convertUtf32ToWCS(ThrowStatusWrapper* status, const char32_t* src, size_t srcSize,
    wchar_t* destBuffer, size_t destBufferSize) const
{
    return static_cast<size_t>(m_encodeUtil->convertUtf32ToWCS(
        status,
        src,
        static_cast<ISC_UINT64>(srcSize),
        destBuffer,
        static_cast<ISC_UINT64>(destBufferSize)));
}

size_t StringEncodeHelper::convertUtf32ToUtf8(ThrowStatusWrapper* status, const char32_t* src, size_t srcSize,
    char* destBuffer, size_t destBufferSize) const
{
    return static_cast<size_t>(m_encodeUtil->convertUtf32ToUtf8(
        status,
        src,
        static_cast<ISC_UINT64>(srcSize),
        destBuffer,
        static_cast<ISC_UINT64>(destBufferSize)));
}

size_t StringEncodeHelper::convertUtf32ToUtf16(ThrowStatusWrapper* status, const char32_t* src, size_t srcSize,
    char16_t* destBuffer, size_t destBufferSize) const
{
    return static_cast<size_t>(m_encodeUtil->convertUtf32ToUtf16(
        status,
        src,
        static_cast<ISC_UINT64>(srcSize),
        destBuffer,
        static_cast<ISC_UINT64>(destBufferSize)));
}

size_t StringEncodeHelper::convertWCSToUtf8(ThrowStatusWrapper* status, const wchar_t* src, size_t srcSize,
    char* destBuffer, size_t destBufferSize) const
{
    return static_cast<size_t>(m_encodeUtil->convertWCSToUtf8(
        status,
        src,
        static_cast<ISC_UINT64>(srcSize),
        destBuffer,
        static_cast<ISC_UINT64>(destBufferSize)));
}

size_t StringEncodeHelper::convertWCSToUtf16(ThrowStatusWrapper* status, const wchar_t* src, size_t srcSize,
    char16_t* destBuffer, size_t destBufferSize) const
{
    return static_cast<size_t>(m_encodeUtil->convertWCSToUtf16(
        status,
        src,
        static_cast<ISC_UINT64>(srcSize),
        destBuffer,
        static_cast<ISC_UINT64>(destBufferSize)));
}

size_t StringEncodeHelper::convertWCSToUtf32(ThrowStatusWrapper* status, const wchar_t* src, size_t srcSize,
    char32_t* destBuffer, size_t destBufferSize) const
{
    return static_cast<size_t>(m_encodeUtil->convertWCSToUtf32(
        status,
        src,
        static_cast<ISC_UINT64>(srcSize),
        destBuffer,
        static_cast<ISC_UINT64>(destBufferSize)));
}

wstring StringEncodeHelper::convertUtf8ToWCS(ThrowStatusWrapper* status, string_view src) const
try {
    size_t resultCapacity = src.size() * StringConverterHelper::MAX_WCS_CP_CHAR;
    wstring result(resultCapacity, L'\0');
    size_t resultLength = convertUtf8ToWCS(status, src.data(), src.size(), result.data(), resultCapacity);
    result.resize(resultLength);
    return result;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

u16string StringEncodeHelper::convertUtf8ToUtf16(ThrowStatusWrapper* status, string_view src) const
try {
    size_t resultCapacity = src.size() * StringConverterHelper::MAX_UTF16_CP_CHAR;
    u16string result(resultCapacity, u'\0');
    size_t resultLength = convertUtf8ToUtf16(status, src.data(), src.size(), result.data(), resultCapacity);
    result.resize(resultLength);
    return result;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

u32string StringEncodeHelper::convertUtf8ToUtf32(ThrowStatusWrapper* status, string_view src) const
try {
    size_t resultCapacity = src.size() * StringConverterHelper::MAX_UTF32_CP_CHAR;
    u32string result(resultCapacity, U'\0');
    size_t resultLength = convertUtf8ToUtf32(status, src.data(), src.size(), result.data(), resultCapacity);
    result.resize(resultLength);
    return result;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

wstring StringEncodeHelper::convertUtf16ToWCS(ThrowStatusWrapper* status, u16string_view src) const
try {
    size_t resultCapacity = src.size() * StringConverterHelper::MAX_WCS_CP_CHAR;
    wstring result(resultCapacity, L'\0');
    size_t resultLength = convertUtf16ToWCS(status, src.data(), src.size(), result.data(), resultCapacity);
    result.resize(resultLength);
    return result;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

string StringEncodeHelper::convertUtf16ToUtf8(ThrowStatusWrapper* status, u16string_view src) const
try {
    size_t resultCapacity = src.size() * StringConverterHelper::MAX_UTF8_CP_CHAR;
    string result(resultCapacity, '\0');
    size_t resultLength = convertUtf16ToUtf8(status, src.data(), src.size(), result.data(), resultCapacity);
    result.resize(resultLength);
    return result;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

u32string StringEncodeHelper::convertUtf16ToUtf32(ThrowStatusWrapper* status, u16string_view src) const
try {
    size_t resultCapacity = src.size() * StringConverterHelper::MAX_UTF32_CP_CHAR;
    u32string result(resultCapacity, U'\0');
    size_t resultLength = convertUtf16ToUtf32(status, src.data(), src.size(), result.data(), resultCapacity);
    result.resize(resultLength);
    return result;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

wstring StringEncodeHelper::convertUtf32ToWCS(ThrowStatusWrapper* status, u32string_view src) const
try {
    size_t resultCapacity = src.size() * StringConverterHelper::MAX_WCS_CP_CHAR;
    wstring result(resultCapacity, L'\0');
    size_t resultLength = convertUtf32ToWCS(status, src.data(), src.size(), result.data(), resultCapacity);
    result.resize(resultLength);
    return result;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

string StringEncodeHelper::convertUtf32ToUtf8(ThrowStatusWrapper* status, u32string_view src) const
try {
    size_t resultCapacity = src.size() * StringConverterHelper::MAX_UTF8_CP_CHAR;
    string result(resultCapacity, '\0');
    size_t resultLength = convertUtf32ToUtf8(status, src.data(), src.size(), result.data(), resultCapacity);
    result.resize(resultLength);
    return result;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

u16string StringEncodeHelper::convertUtf32ToUtf16(ThrowStatusWrapper* status, u32string_view src) const
try {
    size_t resultCapacity = src.size() * StringConverterHelper::MAX_UTF16_CP_CHAR;
    u16string result(resultCapacity, u'\0');
    size_t resultLength = convertUtf32ToUtf16(status, src.data(), src.size(), result.data(), resultCapacity);
    result.resize(resultLength);
    return result;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

string StringEncodeHelper::convertWCSToUtf8(ThrowStatusWrapper* status, wstring_view src) const
try {
    size_t resultCapacity = src.size() * StringConverterHelper::MAX_UTF8_CP_CHAR;
    string result(resultCapacity, '\0');
    size_t resultLength = convertWCSToUtf8(status, src.data(), src.size(), result.data(), resultCapacity);
    result.resize(resultLength);
    return result;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

u16string StringEncodeHelper::convertWCSToUtf16(ThrowStatusWrapper* status, wstring_view src) const
try {
    size_t resultCapacity = src.size() * StringConverterHelper::MAX_UTF16_CP_CHAR;
    u16string result(resultCapacity, u'\0');
    size_t resultLength = convertWCSToUtf16(status, src.data(), src.size(), result.data(), resultCapacity);
    result.resize(resultLength);
    return result;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

u32string StringEncodeHelper::convertWCSToUtf32(ThrowStatusWrapper* status, wstring_view src) const
try {
    size_t resultCapacity = src.size() * StringConverterHelper::MAX_UTF32_CP_CHAR;
    u32string result(resultCapacity, U'\0');
    size_t resultLength = convertWCSToUtf32(status, src.data(), src.size(), result.data(), resultCapacity);
    result.resize(resultLength);
    return result;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

} // namespace Firebird
