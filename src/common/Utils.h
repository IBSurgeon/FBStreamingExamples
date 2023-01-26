#ifndef REPLICATION_UTILS_H
#define REPLICATION_UTILS_H

#include <stdarg.h>
#include <string>
#include <cstring>
#include "firebird/Interface.h"

using namespace Firebird;
using namespace std;


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
#define UQUADFORMAT QUADFORMAT"u"
#define SQUADFORMAT QUADFORMAT"d"

static const int64_t NUMERIC_FACTORS[] = {
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
	1000000000000000000  // 18
};

[[noreturn]]
void raiseError(const char* msg, ...);

[[noreturn]]
void throwException(ThrowStatusWrapper* const status, const char* message, ...);

class BlobUtils final
{
public:
	static const size_t MAX_SEGMENT_SIZE;

	static string getString(ThrowStatusWrapper* status, IBlob* blob);

	static void setString(ThrowStatusWrapper* status, IBlob* blob, const string& str);
};

const string vformat(const char* zcFormat, ...);

string rtrim(const string& s);
wstring rtrim(const wstring& s);

string getBinaryString(const std::byte* data, size_t length);
wstring getBinaryStringW(const std::byte* data, size_t length);

template <typename T>
string getScaledInteger(const T value, short scale)
{
	auto factor = static_cast<T>(NUMERIC_FACTORS[-scale]);
	auto int_value = value / factor;
	auto frac_value = value % factor;
	return vformat("%d.%0*d", int_value, -scale, frac_value);
}


#endif
