#include "Utils.h"
#include <stdio.h>
#include <exception>
#include <vector>
#include <sstream> 
#include <iomanip>

const unsigned int BUFFER_LARGE = 2048;

using namespace Firebird;
using namespace std;

void raiseError(const char* msg, ...)
{
	char buffer[BUFFER_LARGE];

	va_list ptr;
	va_start(ptr, msg);
	vsnprintf(buffer, BUFFER_LARGE, msg, ptr);
	va_end(ptr);

	throw std::runtime_error(buffer);
}

void throwException(Firebird::ThrowStatusWrapper* const status, const char* message, ...)
{
	char buffer[BUFFER_LARGE];

	va_list ptr;
	va_start(ptr, message);
	vsnprintf(buffer, BUFFER_LARGE, message, ptr);
	va_end(ptr);

	ISC_STATUS statusVector[] = {
		isc_arg_gds, isc_random,
		isc_arg_string, (ISC_STATUS)buffer,
		isc_arg_end
	};
	throw Firebird::FbException(status, statusVector);
}

const size_t BlobUtils::MAX_SEGMENT_SIZE = 65535;

string BlobUtils::getString(ThrowStatusWrapper* status, IBlob* blob)
{
	std::stringstream ss("");
	auto b = make_unique<char[]>(MAX_SEGMENT_SIZE + 1);
	{
		char* buffer = b.get();
		bool eof = false;
		unsigned int l;
		while (!eof) {
			switch (blob->getSegment(status, MAX_SEGMENT_SIZE, buffer, &l))
			{
			case IStatus::RESULT_OK:
			case IStatus::RESULT_SEGMENT:
				ss.write(buffer, l);
				continue;
			default:
				eof = true;
				break;
			}
		}
	}
	return ss.str();
}

void BlobUtils::setString(ThrowStatusWrapper* status, IBlob* blob, const string& str)
{
	size_t str_len = str.length();
	size_t offset = 0;
	auto b = make_unique<char[]>(MAX_SEGMENT_SIZE + 1);
	{
		char* buffer = b.get();
		while (str_len > 0) {
			const auto len = static_cast<unsigned int>(min(str_len, MAX_SEGMENT_SIZE));
			memset(buffer, 0, MAX_SEGMENT_SIZE + 1);
			memcpy(buffer, str.data() + offset, len);
			blob->putSegment(status, len, buffer);
			offset += len;
			str_len -= len;
		}
	}
}

const std::string vformat(const char* zcFormat, ...) {

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


const std::string WHITESPACE = " \n\r\t\f\v";

std::string rtrim(const std::string& s)
{
	size_t end = s.find_last_not_of(WHITESPACE);
	return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

const std::wstring WHITESPACE_L = L" \n\r\t\f\v";

std::wstring rtrim(const std::wstring& s)
{
	size_t end = s.find_last_not_of(WHITESPACE_L);
	return (end == std::wstring::npos) ? L"" : s.substr(0, end + 1);
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


