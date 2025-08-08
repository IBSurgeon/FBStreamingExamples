#pragma once
#ifndef STRING_CONVERTER_HELPER_H
#define STRING_CONVERTER_HELPER_H

#include <string>

#include "../include/StreamingInterface.h"

namespace Firebird {

class StringConverterHelper final {
    friend class StringEncodeHelper;

public:
    // max char per code point
    static constexpr int8_t MAX_UTF8_CP_CHAR = sizeof(char32_t) / sizeof(char);
    static constexpr int8_t MAX_UTF16_CP_CHAR = sizeof(char32_t) / sizeof(char16_t);
    static constexpr int8_t MAX_UTF32_CP_CHAR = sizeof(char32_t) / sizeof(char32_t);
    static constexpr int8_t MAX_WCS_CP_CHAR = sizeof(char32_t) / sizeof(wchar_t);

    StringConverterHelper() = delete;
    explicit StringConverterHelper(IStringConverter* converter);

    StringConverterHelper(const StringConverterHelper&) = delete;
    void operator=(const StringConverterHelper&) = delete;
    StringConverterHelper(StringConverterHelper&& converter) noexcept;
    StringConverterHelper& operator=(StringConverterHelper&& r) noexcept;
    ~StringConverterHelper();

    IStringConverter* getStringConverter() const;

    int getMaxCharSize() const;
    int getMinCharSize() const;
    unsigned getCharsetId() const;
    std::string getCharsetName() const;

    size_t toUtf8(ThrowStatusWrapper* status, const char* src, size_t srcSize, char* destBuffer, size_t destBufferSize) const;
    size_t fromUtf8(ThrowStatusWrapper* status, const char* src, size_t srcSize, char* destBuffer, size_t destBufferSize) const;
    size_t toUtf16(ThrowStatusWrapper* status, const char* src, size_t srcSize, char16_t* destBuffer, size_t destBufferSize) const;
    size_t fromUtf16(ThrowStatusWrapper* status, const char16_t* src, size_t srcSize, char* destBuffer, size_t destBufferSize) const;
    size_t toUtf32(ThrowStatusWrapper* status, const char* src, size_t srcSize, char32_t* destBuffer, size_t destBufferSize) const;
    size_t fromUtf32(ThrowStatusWrapper* status, const char32_t* src, size_t srcSize, char* destBuffer, size_t destBufferSize) const;
    size_t toWCS(ThrowStatusWrapper* status, const char* src, size_t srcSize, wchar_t* destBuffer, size_t destBufferSize) const;
    size_t fromWCS(ThrowStatusWrapper* status, const wchar_t* src, size_t srcSize, char* destBuffer, size_t destBufferSize) const;

    std::string toUtf8(ThrowStatusWrapper* status, std::string_view src) const;
    std::string fromUtf8(ThrowStatusWrapper* status, std::string_view src) const;
    std::u16string toUtf16(ThrowStatusWrapper* status, std::string_view src) const;
    std::string fromUtf16(ThrowStatusWrapper* status, std::u16string_view src) const;
    std::u32string toUtf32(ThrowStatusWrapper* status, std::string_view src) const;
    std::string fromUtf32(ThrowStatusWrapper* status, std::u32string_view src) const;
    std::wstring toWCS(ThrowStatusWrapper* status, std::string_view src) const;
    std::string fromWCS(ThrowStatusWrapper* status, std::wstring_view src) const;

private:
    IStringConverter* m_converter { nullptr };
};

} // namespace Firebird

#endif // STRING_CONVERTER_HELPER_H
