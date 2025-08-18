#include "Utils.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <stdio.h>
#include "FBAutoPtr.h"

using namespace Firebird;
using namespace std;

namespace {

constexpr unsigned int BUFFER_LARGE = 2048;

constexpr char WHITESPACE[] = " \n\r\t\f\v";

constexpr wchar_t WHITESPACE_L[] = L" \n\r\t\f\v";

constexpr const char hex_digits[] = "0123456789ABCDEF";

} // namespace

namespace FbUtils
{

    void raiseError(const char* msg, ...)
    {
        char buffer[BUFFER_LARGE];

        va_list ptr;
        va_start(ptr, msg);
        vsnprintf(buffer, BUFFER_LARGE, msg, ptr);
        va_end(ptr);

        throw std::runtime_error(buffer);
    }

    IscRandomStatus IscRandomStatus::createFmtStatus(const char* message, ...)
    {
        char buffer[BUFFER_LARGE];

        va_list ptr;
        va_start(ptr, message);
        vsnprintf(buffer, BUFFER_LARGE, message, ptr);
        va_end(ptr);

        return IscRandomStatus(buffer);
    }

    const std::string vformat(const char* zcFormat, ...)
    {

        // initialize use of the variable argument array
        va_list vaArgs;
        va_start(vaArgs, zcFormat);

        // reliably acquire the size
        // from a copy of the variable argument array
        // and a functionally reliable call to mock the formatting
        va_list vaArgsCopy;
        va_copy(vaArgsCopy, vaArgs);
        const int iLen = std::vsnprintf(nullptr, 0, zcFormat, vaArgsCopy);
        va_end(vaArgsCopy);

        // return a formatted string without risking memory mismanagement
        // and without assuming any compiler or platform specific behavior
        std::vector<char> zc(iLen + 1);
        std::vsnprintf(zc.data(), zc.size(), zcFormat, vaArgs);
        va_end(vaArgs);
        return std::string(zc.data(), iLen);
    }

    std::string rtrim(const std::string& s)
    {
        size_t end = s.find_last_not_of(WHITESPACE);
        return (end == std::string::npos) ? "" : s.substr(0, end + 1);
    }

    std::string rtrim(const std::string& s, const char* ch)
    {
        size_t end = s.find_last_not_of(ch);
        return (end == std::string::npos) ? "" : s.substr(0, end + 1);
    }

    std::wstring rtrim(const std::wstring& s)
    {
        size_t end = s.find_last_not_of(WHITESPACE_L);
        return (end == std::wstring::npos) ? L"" : s.substr(0, end + 1);
    }

    std::string_view sv_rtrim(std::string_view str) noexcept
    {
        if (auto pos = str.find_last_not_of(WHITESPACE); pos != std::string::npos) {
            str.remove_suffix(str.length() - pos - 1);
        }
        return str;
    }

    std::string_view sv_ltrim(std::string_view str) noexcept
    {
        if (auto pos = str.find_first_not_of(WHITESPACE); pos != std::string::npos) {
            str.remove_prefix(pos);
        }
        return str;
    }

    std::string_view sv_trim(std::string_view str) noexcept
    {
        str = sv_ltrim(str);
        str = sv_rtrim(str);
        return str;
    }

    string replace_first(const string& str, string_view what, string_view replacement)
    {
        string s(str);
        std::size_t pos = s.find(what);
        if (pos == std::string::npos)
            return s;
        s.replace(pos, what.length(), replacement);
        return s;
    }

    std::string binary_to_hex(const unsigned char* data, size_t size)
    {
        std::string output;
        output.reserve(size * 2);
        for (const auto end = data + size; data < end; ++data) {
            unsigned char c = *data;
            output.push_back(hex_digits[c >> 4]);
            output.push_back(hex_digits[c & 15]);
        }
        return output;
    }

    std::string getBinaryString(const std::byte* data, size_t length)
    {
        std::stringstream ss;
        if (data) {
            for (unsigned int i = 0; i < length; ++i)
                ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(*data++);
        }
        return ss.str();
    }

    std::wstring getBinaryStringW(const std::byte* data, size_t length)
    {
        std::wstringstream ss;
        if (data) {
            for (unsigned int i = 0; i < length; ++i)
                ss << std::hex << std::setfill(L'0') << std::setw(2) << static_cast<int>(*data++);
        }
        return ss.str();
    }


    bool readBoolFromConfig(ThrowStatusWrapper* status, IConfig* config, const char* name, bool defaultValue)
    {
        AutoRelease<IConfigEntry> ceBool(config->find(status, name));
        return ceBool.hasData() ? ceBool->getBoolValue() : defaultValue;
    }

    int64_t readIntFromConfig(Firebird::ThrowStatusWrapper* status, Firebird::IConfig* config, const char* name, int64_t defaultValue)
    {
        AutoRelease<IConfigEntry> ceInt(config->find(status, name));
        return ceInt.hasData() ? ceInt->getIntValue() : defaultValue;
    }

    std::string readStringFromConfig(Firebird::ThrowStatusWrapper* status, Firebird::IConfig* config, const char* name, const std::string defaultValue)
    {
        AutoRelease<IConfigEntry> ceString(config->find(status, name));
        return ceString.hasData() ? std::string(ceString->getValue()) : defaultValue;
    }

}
