#ifndef ENCODE_UTILS_H
#define ENCODE_UTILS_H

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

#ifdef _WINDOWS
#include <windows.h>
#endif

#include "FBCharsetInfo.h"
#include <string>


#ifndef _WINDOWS
#include "unicode/ucnv.h"
#endif

using namespace std;



class FBStringEncoder final {

private:
	FBCharsetInfo sourceCharsetInfo;
#ifndef _WINDOWS
	UConverter* uConverter;
#endif

public:

	FBStringEncoder() = delete;

	explicit FBStringEncoder(const char* sourceCharsetName) 
#ifndef _WINDOWS
		: uConverter(nullptr)
#endif
	{
		sourceCharsetInfo = getCharsetInfo(sourceCharsetName);
	}

	explicit FBStringEncoder(const unsigned charsetID)
#ifndef _WINDOWS
		: uConverter(nullptr)
#endif
	{
		sourceCharsetInfo = getCharsetInfo(charsetID);
	}

#ifndef _WINDOWS
	~FBStringEncoder() {
		if (uConverter) {
			ucnv_close(uConverter);
		}
	}
#endif

	string toUtf8(const string& source_str);
	string toUtf8(const wstring& source_str);
	wstring toUnicode(const string& source_str);
};



#endif	// ENCODE_UTILS_H