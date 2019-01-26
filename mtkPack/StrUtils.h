#pragma once
#include <cwchar>
#include <string>
#include <vector>
#include <map>

#include "SecureDelete.h"

namespace mtkPack {

	class StrUtils {
	public:
		static char* ToChar(const wchar_t* pWChar);
		static wchar_t* ToWChar(const char* pChar);

		static std::string ToString(const wchar_t* pWChar);
		static std::string ToString(const std::wstring& wsStr);

		static std::wstring ToWString(const char* pChar);
		static std::wstring ToWString(const std::string& sStr);

		static void ToLower(char* pStr, const int nSize = 0);
		static char* ToLower(const char* pStr, const int nSize = 0);
		static void ToLower(std::string& sStr);
		static std::string ToLower(const std::string& sStr);

		static void ToUpper(char* pStr, const int nSize = 0);
		static char* ToUpper(const char* pStr, const int nSize = 0);
		static void ToUpper(std::string& sStr);
		static std::string ToUpper(const std::string& sStr);;

		static int SplitToken(const std::string& sStr, const std::string& sTokenSet, std::vector<std::string>& sResult);
		static int SplitToken(const std::string& sStr, const std::string& sTokenSet, const char& cDelimiter, std::map<std::string, std::string>& sResult);

		static std::string& LTrim(std::string& sStr, const std::string& sTrimSet = " ");
		static std::string& RTrim(std::string& sStr, const std::string& sTrimSet = " ");
		static std::string& Trim(std::string& sStr, const std::string& sTrimSet = " ");

		static int ToInt(const char* pChar);
		static int ToInt(const wchar_t* pWChar);
		static int ToInt(const std::string& sStr);
		static int ToInt(const std::wstring& wsStr);

		static std::string ToStr(const int nVal);

	protected:
		typedef int (*fnUpperLower)(int c);//int __cdecl tolower(_In_ int _C);
		static void toUL(const char* pSrc, char* pDst, fnUpperLower pFn, const int nSize = 0);
	};

}