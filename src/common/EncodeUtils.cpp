/**
 *  String conversion functions.
 *
 *  The original code was created by Simonov Denis
 *  for the open source Lucene UDR full-text search library for Firebird DBMS.
 *
 *  Copyright (c) 2022 Simonov Denis <sim-mail@list.ru>
 *  and all contributors signed below.
 *
 *  All Rights Reserved.
 *  Contributor(s): ______________________________________.
**/

#include "EncodeUtils.h"

#ifndef _WINDOWS
#include "unicode/uchar.h"
#include "unicode/unistr.h"
#include "unicode/ustring.h"
#endif

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>

#ifndef _WINDOWS
using namespace icu;
#endif

using namespace std;



string FBStringEncoder::toUtf8(const string& source_str)
{
    // if the string is already in utf-8, then it makes no sense to re-encode it
    if (sourceCharsetInfo.codePage == 65001) {
        return source_str;
    }
    const auto src_len = static_cast<int32_t>(source_str.size());
    if (src_len == 0) {
        return source_str;
    }
#ifdef _WINDOWS
    return toUtf8(toUnicode(source_str));
#else
    const auto srclen = static_cast<int32_t>(source_str.size());
    vector<UChar> target(srclen);

    UErrorCode status = U_ZERO_ERROR;
    UConverter* conv = ucnv_open(sourceCharsetInfo.charsetName.c_str(), &status);
    if (!U_SUCCESS(status))
        return string();

    int32_t len = ucnv_toUChars(conv, target.data(), srclen, source_str.c_str(), srclen, &status);
    if (!U_SUCCESS(status))
        return string();

    ucnv_close(conv);

    UnicodeString ustr(target.data(), len);

    string retval;
    ustr.toUTF8String(retval);

    return retval;
#endif
}

string FBStringEncoder::toUtf8(const wstring& source_str)
{
#ifdef _WINDOWS	
    int src_len = static_cast<int>(source_str.size());
    int dest_len = WideCharToMultiByte(65001, 0, source_str.c_str(), src_len, nullptr, 0, nullptr, nullptr);

    if (!dest_len)
        return "";

    string result{ "" };
    unique_ptr<char[]> pRes = make_unique<char[]>(dest_len);
    {
        char* buffer = pRes.get();
        if (!WideCharToMultiByte(65001, 0, source_str.c_str(), src_len, buffer, dest_len, nullptr, nullptr))
        {
            throw runtime_error("Cannot convert string to Unicode");
        }
        result.assign(buffer, dest_len);
    }

    return result;
#else
    std::string result = "";
    if (source_str.empty())
        return result;

    std::vector<UChar> buffer;

    result.resize(source_str.size() * 3); // UTF-8 uses max 4 bytes per char
    buffer.resize(source_str.size() * 2); // UTF-16 uses 2 code-points per char

    UErrorCode status = U_ZERO_ERROR;
    int32_t len = 0;

    u_strFromWCS(
        &buffer[0],
        buffer.size(),
        &len,
        &source_str[0],
        source_str.size(),
        &status
    );
    if (!U_SUCCESS(status))
    {
        throw runtime_error("utf8: u_strFromWCS failed");
    }
    buffer.resize(len);

    u_strToUTF8(
        &result[0],
        result.size(),
        &len,
        &buffer[0],
        buffer.size(),
        &status
    );
    if (!U_SUCCESS(status))
    {
        throw runtime_error("utf8: u_strToUTF8 failed");
    }
    result.resize(len);

    return result;
#endif
}

wstring FBStringEncoder::toUnicode(const string& source_str)
{
#ifdef _WINDOWS		
    int src_len = static_cast<int>(source_str.size());
    int dest_len = MultiByteToWideChar(sourceCharsetInfo.codePage, 0, source_str.c_str(), src_len, nullptr, 0);


    if (!dest_len)
        return L"";

    wstring result{ L"" };
    unique_ptr<wchar_t[]> pRes = make_unique<wchar_t[]>(dest_len);
    {
        wchar_t* buffer = pRes.get();
        if (!MultiByteToWideChar(sourceCharsetInfo.codePage, 0, source_str.c_str(), src_len, buffer, dest_len))
        {
            throw runtime_error("Cannot convert string to Unicode");
        }
        result.assign(buffer, dest_len);
    }
    return result;
#else
    std::wstring result = L"";
    if (source_str.empty())
        return result;

    std::vector<UChar> buffer;

    result.resize(source_str.size());
    buffer.resize(source_str.size());

    UErrorCode status = U_ZERO_ERROR;
    int32_t len = 0;

    u_strFromUTF8(
        &buffer[0],
        buffer.size(),
        &len,
        &source_str[0],
        source_str.size(),
        &status
    );
    if (!U_SUCCESS(status))
    {
        throw runtime_error("utf8: u_strFromUTF8 failed");
    }
    buffer.resize(len);

    u_strToWCS(
        &result[0],
        result.size(),
        &len,
        &buffer[0],
        buffer.size(),
        &status
    );
    if (!U_SUCCESS(status))
    {
        throw runtime_error("utf8: u_strToWCS failed");
    }
    result.resize(len);

    return result;
#endif
}
