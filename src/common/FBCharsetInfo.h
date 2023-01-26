#ifndef FB_CHARSET_INFO_H
#define FB_CHARSET_INFO_H

#include "charsets.h"
#include <string>
#include <map>

using namespace std;

struct FBCharsetInfo {
	string charsetName = "";
	string icuCharsetName = "";
	unsigned codePage = 0;
	unsigned bytesPerChar = 0;
};



static const std::map<unsigned, FBCharsetInfo> FBCharsetMap {
	{CS_NONE, {"NONE", "", 0, 1 /* CP_ACP */}},
	{CS_BINARY, {"OCTETS", "", 0, 1 /* CP_ACP */}},
	{CS_ASCII, {"ASCII", "", 0, 1 /* CP_ACP */}},
	{CS_UNICODE_FSS, {"UNICODE_FSS", "utf-8", 65001, 3 /* CP_UTF8 */}},
	{CS_UTF8, {"UTF8", "utf-8", 65001, 4 /* CP_UTF8 */}},
	{CS_SJIS, {"SJIS", "cp932", 932, 2}},
	{CS_EUCJ, {"EUCJ", "ibm-1350", 932, 2}},
	{CS_DOS_737, {"DOS737", "cp737", 737, 1}},
	{CS_DOS_437, {"DOS437", "cp437", 437, 1}},
	{CS_DOS_850, {"DOS850", "cp850", 850, 1}},
	{CS_DOS_865, {"DOS865", "cp865", 865, 1}},
	{CS_DOS_860, {"DOS860", "cp860", 860, 1}},
	{CS_DOS_863, {"DOS863", "cp863", 863, 1}},
	{CS_DOS_775,  {"DOS775", "cp775", 775, 1}},
	{CS_DOS_858, {"DOS858", "cp858", 858, 1}},
	{CS_DOS_862,  {"DOS862", "cp862", 862, 1}},
	{CS_DOS_864,  {"DOS864", "cp864",  864, 1}},
	{CS_ISO8859_1, {"ISO8859_1", "iso-8859-1",  28591, 1}},
	{CS_ISO8859_2, {"ISO8859_2", "iso-8859-2", 28592, 1}},
	{CS_ISO8859_3, {"ISO8859_3", "iso-8859-3",  28593, 1}},
	{CS_ISO8859_4, {"ISO8859_4", "iso-8859-4", 28594, 1}},
	{CS_ISO8859_5, {"ISO8859_5", "iso-8859-5", 28595, 1}},
	{CS_ISO8859_6, {"ISO8859_6", "iso-8859-6", 28596, 1}},
	{CS_ISO8859_7, {"ISO8859_7", "iso-8859-7", 28597, 1}},
	{CS_ISO8859_8, {"ISO8859_8", "iso-8859-8", 28598, 1}},
	{CS_ISO8859_9, {"ISO8859_9", "iso-8859-9", 28599, 1}},
	{CS_ISO8859_13, {"ISO8859_13", "iso-8859-13", 28603, 1}},
	{CS_KSC5601, {"KSC_5601", "windows-949", 949, 2}},
	{CS_DOS_852, {"DOS852", "cp852",  852, 1}},
	{CS_DOS_857, {"DOS857", "cp857", 857, 1}},
	{CS_DOS_861, {"DOS861", "cp861", 861, 1}},
	{CS_DOS_866, {"DOS866", "cp866", 866, 1}},
	{CS_DOS_869, {"DOS869", "cp869", 869, 1}},
	{CS_CYRL, {"CYRL", "windows-1251", 1251, 1}},
	{CS_WIN1250, {"WIN1250", "windows-1250", 1250, 1}},
	{CS_WIN1251, {"WIN1251", "windows-1251", 1251, 1}},
	{CS_WIN1252, {"WIN1252", "windows-1252", 1252, 1}},
	{CS_WIN1253, {"WIN1253", "windows-1253", 1253, 1}},
	{CS_WIN1254, {"WIN1254", "windows-1254", 1254, 1}},
	{CS_BIG5, {"BIG_5", "windows-950", 950, 2}},
	{CS_GB2312, {"GB_2312", "ibm-5478", 936, 2}},
	{CS_WIN1255, {"WIN1255", "windows-1255", 1255, 1}},
	{CS_WIN1256, {"WIN1256", "windows-1256", 1256, 1}},
	{CS_WIN1257, {"WIN1257", "windows-1257", 1257, 1}},
	{CS_KOI8R, {"KOI8R", "KOI8-R", 20866, 1}},
	{CS_KOI8U, {"KOI8U", "KOI8-U", 21866, 1}},
	{CS_WIN1258, {"WIN1258", "windows-1258", 1258, 1}},
	{CS_TIS620, {"TIS620", "TIS-620", 874, 1}},
	{CS_GBK, {"GBK", "GBK", 936, 2}},
	{CS_CP943C, {"CP943C", "ibm-943", 943, 2}},
	{CS_GB18030, {"GB18030", "windows-54936", 54936, 4}}
};

static const std::map<string, unsigned> FBChasetIdByName{
	{"NONE", CS_NONE},
	{"OCTETS", CS_BINARY},
	{"ASCII", CS_ASCII},
	{"UNICODE_FSS", CS_UNICODE_FSS},
	{"UTF8", CS_UTF8},
	{"SJIS", CS_SJIS},
	{"EUCJ", CS_EUCJ},
	{"DOS737", CS_DOS_737},
	{"DOS437", CS_DOS_437},
	{"DOS850", CS_DOS_850},
	{"DOS865", CS_DOS_865},
	{"DOS860", CS_DOS_860},
	{"DOS863", CS_DOS_863},
	{"DOS775", CS_DOS_775},
	{"DOS858", CS_DOS_858},
	{"DOS862", CS_DOS_862},
	{"DOS864", CS_DOS_864},
	{"ISO8859_1", CS_ISO8859_1},
	{"ISO8859_2", CS_ISO8859_2},
	{"ISO8859_3", CS_ISO8859_3},
	{"ISO8859_4", CS_ISO8859_4},
	{"ISO8859_5", CS_ISO8859_5},
	{"ISO8859_6", CS_ISO8859_6},
	{"ISO8859_7", CS_ISO8859_7},
	{"ISO8859_8", CS_ISO8859_8},
	{"ISO8859_9", CS_ISO8859_9},
	{"ISO8859_13", CS_ISO8859_13},
	{"KSC_5601", CS_KSC5601},
	{"DOS852", CS_DOS_852},
	{"DOS857", CS_DOS_857},
	{"DOS861", CS_DOS_861},
	{"DOS866", CS_DOS_866},
	{"DOS869", CS_DOS_869},
	{"CYRL", CS_CYRL},
	{"WIN1250", CS_WIN1250},
	{"WIN1251", CS_WIN1251},
	{"WIN1252", CS_WIN1252},
	{"WIN1253", CS_WIN1253},
	{"WIN1254", CS_WIN1254},
	{"BIG_5", CS_BIG5},
	{"GB_2312", CS_GB2312},
	{"WIN1255", CS_WIN1255},
	{"WIN1256", CS_WIN1256},
	{"WIN1257", CS_WIN1257},
	{"KOI8R", CS_KOI8R},
	{"KOI8U", CS_KOI8U},
	{"WIN1258", CS_WIN1258},
	{"TIS620", CS_TIS620},
	{"GBK", CS_GBK},
	{"CP943C", CS_CP943C},
	{"GB18030", CS_GB18030}
};

inline const FBCharsetInfo getCharsetInfo(unsigned charsetId)
{
	return FBCharsetMap.at(charsetId);
}

inline const FBCharsetInfo getCharsetInfo(const string& charsetName)
{
	auto charsetId = FBChasetIdByName.at(charsetName);
	return FBCharsetMap.at(charsetId);
}

#endif // FB_CHARSET_INFO_H
