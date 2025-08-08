#pragma once
#ifndef FB_STREAMING_UTILS_H
#define FB_STREAMING_UTILS_H

#include <cstdarg>
#include <cstring>
#include <stdexcept>
#include <string>

#include "firebird/Interface.h"

/*****************************************************
 * Linux platforms
 *****************************************************/
#ifdef LINUX
#define QUADFORMAT "ll"
#endif

/*****************************************************
 * Windows NT
 *****************************************************/

#ifdef _WINDOWS
#define QUADFORMAT "I64"
#endif

// Shortcuts to make it easier to convert code using SLONGFORMAT/ULONGFORMAT
#define UQUADFORMAT QUADFORMAT "u"
#define SQUADFORMAT QUADFORMAT "d"

namespace FbUtils
{

    inline constexpr int64_t NUMERIC_FACTORS[] = {
        0, // 0
        10, // 1
        100, // 2
        1000, // 3
        10000, // 4
        100000, // 5
        1000000, // 6
        10000000, // 7
        100000000, // 8
        1000000000, // 9
        10000000000, // 10
        100000000000, // 11
        1000000000000, // 12
        10000000000000, // 13
        100000000000000, // 14
        1000000000000000, // 15
        10000000000000000, // 16
        100000000000000000, // 17
        1000000000000000000 // 18
    };

    [[noreturn]] void raiseError(const char* msg, ...);

    struct IscRandomStatus {
    public:
        explicit IscRandomStatus(const std::string& message)
            : statusVector{ isc_arg_gds, isc_random,
                isc_arg_string, (ISC_STATUS)message.c_str(),
                isc_arg_end }
        {
        }

        explicit IscRandomStatus(const char* message)
            : statusVector{ isc_arg_gds, isc_random,
                isc_arg_string, (ISC_STATUS)message,
                isc_arg_end }
        {
        }

        explicit IscRandomStatus(const std::exception& e)
            : statusVector{ isc_arg_gds, isc_random,
                isc_arg_string, (ISC_STATUS)e.what(),
                isc_arg_end }
        {
        }

        operator const ISC_STATUS* () const { return statusVector; }

        static IscRandomStatus createFmtStatus(const char* message, ...);

    private:
        ISC_STATUS statusVector[5] = {};
    };

    const std::string vformat(const char* zcFormat, ...);

    std::string rtrim(const std::string& s);
    std::string rtrim(const std::string& s, const char* ch);
    std::wstring rtrim(const std::wstring& s);
    std::string replace_first(const std::string& str, std::string_view what, std::string_view replacement);

    std::string binary_to_hex(const unsigned char* data, size_t size);

    std::string getBinaryString(const std::byte* data, size_t length);
    std::wstring getBinaryStringW(const std::byte* data, size_t length);

    template <typename T>
    std::string getScaledInteger(const T value, short scale)
    {
        auto factor = static_cast<T>(NUMERIC_FACTORS[-scale]);
        auto int_value = value / factor;
        auto frac_value = value % factor;
        return vformat("%d.%0*d", int_value, -scale, frac_value);
    }

    bool readBoolFromConfig(Firebird::ThrowStatusWrapper* status, Firebird::IConfig* config, const char* name, bool defaultValue = false);
    int64_t readIntFromConfig(Firebird::ThrowStatusWrapper* status, Firebird::IConfig* config, const char* name, int64_t defaultValue = 0);
    std::string readStringFromConfig(Firebird::ThrowStatusWrapper* status, Firebird::IConfig* config, const char* name, const std::string defaultValue = {});

}

#endif // FB_STREAMING_UTILS_H
