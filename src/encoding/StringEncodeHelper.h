#pragma once
#ifndef STRING_ENCODE_HELPER_H
#define STRING_ENCODE_HELPER_H

#include <string>

#include "../include/StreamingInterface.h"
#include "StringConverterHelper.h"

namespace Firebird {


/**
 * @brief String encoding helper.
 * 
 * @details Contains functions for encoding strings from one charater set to another.
 */
class StringEncodeHelper final {
public:
    StringEncodeHelper() = delete;

    /**
     * @brief Constructor.
     * 
     * @param[in] encodeUtil Pointer to the string conversion interface.
     */
    explicit StringEncodeHelper(IStringEncodeUtils* const encodeUtil);

    /**
     * @brief Returns a pointer to the string conversion interface.
     * 
     * @return       Pointer to the string conversion interface.
     */
    IStringEncodeUtils* getStringEncodeUtils() const;

    /**
     * @brief Returns the string encoding object.
     * 
     * @param[inout] status         Pointer to the status vector for error handling.
     * @param[in]    charsetId      String charater set identifier.
     * 
     * @return       String encoding object.
     */
    StringConverterHelper getConverterById(ThrowStatusWrapper* status, unsigned charsetId) const;

    /**
     * @brief Returns the string encoding object.
     *
     * @param[inout] status         Pointer to the status vector for error handling.
     * @param[in]    charsetName    String charater set name.
     *
     * @return       String encoding object.
     */
    StringConverterHelper getConverterByName(ThrowStatusWrapper* status, std::string_view charsetName) const;

    /**
     * @brief    Converts a string from one encoding to another.
     *
     * @param[inout] status         Pointer to the status vector for error handling.
     * @param[in]    srcConveter    A reference to the encoding object of the source string.
     * @param[in]    destConveter   A reference to the encoding object of the target string.
     * @param[in]    src            Pointer to the beginning of the source string.
     * @param[in]    srcSize        Size of the source string.
     * @param[inout] destBuffer     Pointer to the target string buffer.
     * @param[in]    destBufferSize The size of the target string buffer.
     * 
     * @return       The size of the target string.
     */
    size_t convertCharset(
        ThrowStatusWrapper* status,
        const StringConverterHelper& srcConveter,
        const StringConverterHelper& destConveter,
        const char* src, size_t srcSize,
        char* destBuffer, size_t destBufferSize) const;

    /**
     * @brief    Converts a string from one encoding to another.
     *
     * @param[inout] status         Pointer to the status vector for error handling.
     * @param[in]    srcConveter    A reference to the encoding object of the source string.
     * @param[in]    destConveter   A reference to the encoding object of the target string.
     * @param[in]    src            Source string.
     *
     * @return       Target string.
     */
    std::string convertCharset(
        ThrowStatusWrapper* status,
        const StringConverterHelper& srcConveter,
        const StringConverterHelper& destConveter,
        std::string_view src) const;

    size_t convertUtf8ToWCS(ThrowStatusWrapper* status, const char* src, size_t srcSize,
        wchar_t* destBuffer, size_t destBufferSize) const;
    size_t convertUtf8ToUtf16(ThrowStatusWrapper* status, const char* src, size_t srcSize,
        char16_t* destBuffer, size_t destBufferSize) const;
    size_t convertUtf8ToUtf32(ThrowStatusWrapper* status, const char* src, size_t srcSize,
        char32_t* destBuffer, size_t destBufferSize) const;
    size_t convertUtf16ToWCS(ThrowStatusWrapper* status, const char16_t* src, size_t srcSize,
        wchar_t* destBuffer, size_t destBufferSize) const;
    size_t convertUtf16ToUtf8(ThrowStatusWrapper* status, const char16_t* src, size_t srcSize,
        char* destBuffer, size_t destBufferSize) const;
    size_t convertUtf16ToUtf32(ThrowStatusWrapper* status, const char16_t* src, size_t srcSize,
        char32_t* destBuffer, size_t destBufferSize) const;
    size_t convertUtf32ToWCS(ThrowStatusWrapper* status, const char32_t* src, size_t srcSize,
        wchar_t* destBuffer, size_t destBufferSize) const;
    size_t convertUtf32ToUtf8(ThrowStatusWrapper* status, const char32_t* src, size_t srcSize,
        char* destBuffer, size_t destBufferSize) const;
    size_t convertUtf32ToUtf16(ThrowStatusWrapper* status, const char32_t* src, size_t srcSize,
        char16_t* destBuffer, size_t destBufferSize) const;
    size_t convertWCSToUtf8(ThrowStatusWrapper* status, const wchar_t* src, size_t srcSize,
        char* destBuffer, size_t destBufferSize) const;
    size_t convertWCSToUtf16(ThrowStatusWrapper* status, const wchar_t* src, size_t srcSize,
        char16_t* destBuffer, size_t destBufferSize) const;
    size_t convertWCSToUtf32(ThrowStatusWrapper* status, const wchar_t* src, size_t srcSize,
        char32_t* destBuffer, size_t destBufferSize) const;

    std::wstring convertUtf8ToWCS(ThrowStatusWrapper* status, std::string_view src) const;
    std::u16string convertUtf8ToUtf16(ThrowStatusWrapper* status, std::string_view src) const;
    std::u32string convertUtf8ToUtf32(ThrowStatusWrapper* status, std::string_view src) const;
    std::wstring convertUtf16ToWCS(ThrowStatusWrapper* status, std::u16string_view src) const;
    std::string convertUtf16ToUtf8(ThrowStatusWrapper* status, std::u16string_view src) const;
    std::u32string convertUtf16ToUtf32(ThrowStatusWrapper* status, std::u16string_view src) const;
    std::wstring convertUtf32ToWCS(ThrowStatusWrapper* status, std::u32string_view src) const;
    std::string convertUtf32ToUtf8(ThrowStatusWrapper* status, std::u32string_view src) const;
    std::u16string convertUtf32ToUtf16(ThrowStatusWrapper* status, std::u32string_view src) const;
    std::string convertWCSToUtf8(ThrowStatusWrapper* status, std::wstring_view src) const;
    std::u16string convertWCSToUtf16(ThrowStatusWrapper* status, std::wstring_view src) const;
    std::u32string convertWCSToUtf32(ThrowStatusWrapper* status, std::wstring_view src) const;

private:
    IStringEncodeUtils* const m_encodeUtil;
};

} // namespace Firebird

#endif // STRING_ENCODE_HELPER_H
