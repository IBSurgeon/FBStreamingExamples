#include "StringConverterHelper.h"

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


StringConverterHelper::StringConverterHelper(IStringConverter* converter)
    : m_converter(converter)
{
}

StringConverterHelper::StringConverterHelper(StringConverterHelper&& converter) noexcept
    : m_converter(converter.m_converter)
{
    converter.m_converter = nullptr;
}

StringConverterHelper& StringConverterHelper::operator=(StringConverterHelper&& r) noexcept
{
    if (this != &r) {
        m_converter = r.m_converter;
        r.m_converter = nullptr;
    }

    return *this;
}

StringConverterHelper::~StringConverterHelper()
{
    if (m_converter)
        m_converter->release();
}

int StringConverterHelper::getMaxCharSize() const
{
    return m_converter->getMaxCharSize();
}

int StringConverterHelper::getMinCharSize() const
{
    return m_converter->getMinCharSize();
}

unsigned StringConverterHelper::getCharsetId() const
{
    return m_converter->getCharsetId();
}

string StringConverterHelper::getCharsetName() const
{
    return string(m_converter->getCharsetName());
}

IStringConverter* StringConverterHelper::getStringConverter() const
{
    return m_converter;
}

size_t StringConverterHelper::toUtf8(ThrowStatusWrapper* status, const char* src, size_t srcSize,
    char* destBuffer, size_t destBufferSize) const
{
    return static_cast<size_t>(m_converter->toUtf8(
        status,
        src,
        static_cast<ISC_UINT64>(srcSize),
        destBuffer,
        static_cast<ISC_UINT64>(destBufferSize)));
}

size_t StringConverterHelper::fromUtf8(ThrowStatusWrapper* status, const char* src, size_t srcSize,
    char* destBuffer, size_t destBufferSize) const
{
    return static_cast<size_t>(m_converter->fromUtf8(
        status,
        src,
        static_cast<ISC_UINT64>(srcSize),
        destBuffer,
        static_cast<ISC_UINT64>(destBufferSize)));
}

size_t StringConverterHelper::toUtf16(ThrowStatusWrapper* status, const char* src, size_t srcSize,
    char16_t* destBuffer, size_t destBufferSize) const
{
    return static_cast<size_t>(m_converter->toUtf16(
        status,
        src,
        static_cast<ISC_UINT64>(srcSize),
        destBuffer,
        static_cast<ISC_UINT64>(destBufferSize)));
}

size_t StringConverterHelper::fromUtf16(ThrowStatusWrapper* status, const char16_t* src, size_t srcSize,
    char* destBuffer, size_t destBufferSize) const
{
    return static_cast<size_t>(m_converter->fromUtf16(
        status,
        src,
        static_cast<ISC_UINT64>(srcSize),
        destBuffer,
        static_cast<ISC_UINT64>(destBufferSize)));
}

size_t StringConverterHelper::toUtf32(ThrowStatusWrapper* status, const char* src, size_t srcSize,
    char32_t* destBuffer, size_t destBufferSize) const
{
    return static_cast<size_t>(m_converter->toUtf32(
        status,
        src,
        static_cast<ISC_UINT64>(srcSize),
        destBuffer,
        static_cast<ISC_UINT64>(destBufferSize)));
}

size_t StringConverterHelper::fromUtf32(ThrowStatusWrapper* status, const char32_t* src, size_t srcSize,
    char* destBuffer, size_t destBufferSize) const
{
    return static_cast<size_t>(m_converter->fromUtf32(
        status,
        src,
        static_cast<ISC_UINT64>(srcSize),
        destBuffer,
        static_cast<ISC_UINT64>(destBufferSize)));
}

size_t StringConverterHelper::toWCS(ThrowStatusWrapper* status, const char* src, size_t srcSize,
    wchar_t* destBuffer, size_t destBufferSize) const
{
    return static_cast<size_t>(m_converter->toWCS(
        status,
        src,
        static_cast<ISC_UINT64>(srcSize),
        destBuffer,
        static_cast<ISC_UINT64>(destBufferSize)));
}

size_t StringConverterHelper::fromWCS(ThrowStatusWrapper* status, const wchar_t* src, size_t srcSize,
    char* destBuffer, size_t destBufferSize) const
{
    return static_cast<size_t>(m_converter->fromWCS(
        status,
        src,
        static_cast<ISC_UINT64>(srcSize),
        destBuffer,
        static_cast<ISC_UINT64>(destBufferSize)));
}

string StringConverterHelper::toUtf8(ThrowStatusWrapper* status, string_view src) const
try {
    size_t resultCapacity = src.size() / m_converter->getMinCharSize() * MAX_UTF8_CP_CHAR;
    string result(resultCapacity, '\0');
    size_t resultLength = toUtf8(status, src.data(), src.size(), result.data(), resultCapacity);
    result.resize(resultLength);
    return result;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

string StringConverterHelper::fromUtf8(ThrowStatusWrapper* status, string_view src) const
try {
    size_t resultCapacity = src.size() * m_converter->getMaxCharSize();
    string result(resultCapacity, '\0');
    size_t resultLength = fromUtf8(status, src.data(), src.size(), result.data(), resultCapacity);
    result.resize(resultLength);
    return result;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

u16string StringConverterHelper::toUtf16(ThrowStatusWrapper* status, string_view src) const
try {
    size_t resultCapacity = src.size() / m_converter->getMinCharSize() * MAX_UTF16_CP_CHAR;
    u16string result(resultCapacity, u'\0');
    size_t resultLength = toUtf16(status, src.data(), src.size(), result.data(), resultCapacity);
    result.resize(resultLength);
    return result;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

string StringConverterHelper::fromUtf16(ThrowStatusWrapper* status, u16string_view src) const
try {
    size_t resultCapacity = src.size() * m_converter->getMaxCharSize();
    string result(resultCapacity, '\0');
    size_t resultLength = fromUtf16(status, src.data(), src.size(), result.data(), resultCapacity);
    result.resize(resultLength);
    return result;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

u32string StringConverterHelper::toUtf32(ThrowStatusWrapper* status, string_view src) const
try {
    size_t resultCapacity = src.size() / m_converter->getMinCharSize() * MAX_UTF32_CP_CHAR;
    u32string result(resultCapacity, U'\0');
    size_t resultLength = toUtf32(status, src.data(), src.size(), result.data(), resultCapacity);
    result.resize(resultLength);
    return result;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

string StringConverterHelper::fromUtf32(ThrowStatusWrapper* status, u32string_view src) const
try {
    size_t resultCapacity = src.size() * m_converter->getMaxCharSize();
    string result(resultCapacity, '\0');
    size_t resultLength = fromUtf32(status, src.data(), src.size(), result.data(), resultCapacity);
    result.resize(resultLength);
    return result;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

wstring StringConverterHelper::toWCS(ThrowStatusWrapper* status, string_view src) const
try {
    size_t resultCapacity = src.size() / m_converter->getMinCharSize() * MAX_WCS_CP_CHAR;
    wstring result(resultCapacity, L'\0');
    size_t resultLength = toWCS(status, src.data(), src.size(), result.data(), resultCapacity);
    result.resize(resultLength);
    return result;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

string StringConverterHelper::fromWCS(ThrowStatusWrapper* status, wstring_view src) const
try {
    size_t resultCapacity = src.size() * m_converter->getMaxCharSize();
    string result(resultCapacity, '\0');
    size_t resultLength = fromWCS(status, src.data(), src.size(), result.data(), resultCapacity);
    result.resize(resultLength);
    return result;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

} // namespace Firebird
