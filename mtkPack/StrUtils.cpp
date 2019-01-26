#include <cstdlib>
#include <climits>
#include <cstring>
#include <algorithm>
#include <utility>

#include "StrUtils.h"

char* mtkPack::StrUtils::ToChar(const wchar_t* pWChar) {
	char* pChar = NULL;
	if (pWChar) {
		size_t scSize = wcsnlen_s(pWChar, INT_MAX);
		if (scSize < INT_MAX) {
			scSize++;
		}
		size_t cvtlen = 0;
		pChar = new(std::nothrow) char[scSize];
		if (pChar) {
			memset(pChar, 0, scSize);
			wcstombs_s(&cvtlen, pChar, scSize, pWChar, scSize);
		}
	}

	return pChar;
}

wchar_t* mtkPack::StrUtils::ToWChar(const char* pChar) {
	wchar_t* pWChar = NULL;
	if (pChar) {
		size_t scSize = strnlen_s(pChar, INT_MAX);
		if (scSize < INT_MAX) {
			scSize++;
		}
		size_t cvtlen = 0;
		pWChar = new(std::nothrow) wchar_t[scSize + 1];
		if (pWChar) {
			wmemset(pWChar, 0, scSize);
			mbstowcs_s(&cvtlen, pWChar, scSize, pChar, scSize);
		}
	}

	return pWChar;
}

std::string mtkPack::StrUtils::ToString(const wchar_t* pWChar) {
	char* pChar = ToChar(pWChar);
	SecureDelete<char> sd1(&pChar, 1);
	if (pChar) {
		return std::string(pChar);
	}
	return std::string("");
}

std::string mtkPack::StrUtils::ToString(const std::wstring& wsStr) {
	return ToString(wsStr.c_str());
}

std::wstring mtkPack::StrUtils::ToWString(const char* pChar) {
	wchar_t* pWChar = ToWChar(pChar);
	SecureDelete<wchar_t> sd1(&pWChar, 1);
	if (pWChar) {
		return std::wstring(pWChar);
	}
	return std::wstring(L"");
}

std::wstring mtkPack::StrUtils::ToWString(const std::string& sStr) {
	return ToWString(sStr.c_str());
}

void mtkPack::StrUtils::ToLower(char* pStr, const int nSize /* = 0 */) {
	toUL(pStr, pStr, tolower, nSize);
}

char* mtkPack::StrUtils::ToLower(const char* pStr, const int nSize /* = 0 */) {
	if (pStr == NULL) {
		return NULL;
	}
	int len = (nSize > 0)? nSize : strlen(pStr);
	char* ret = new(std::nothrow) char[len + 1];
	*(ret+len) = 0;
	toUL(pStr, ret, tolower, nSize);
	return ret;
}

void mtkPack::StrUtils::ToLower(std::string& sStr) {
	transform(sStr.begin(), sStr.end(), sStr.begin(), tolower);
}

std::string mtkPack::StrUtils::ToLower(const std::string& sStr) {
	std::string sRet(sStr);
	transform(sStr.begin(), sStr.end(), sRet.begin(), tolower);
	return sRet;
}

void mtkPack::StrUtils::ToUpper(char* pStr, const int nSize /* = 0 */) {
	toUL(pStr, pStr, toupper, nSize);
}

char* mtkPack::StrUtils::ToUpper(const char* pStr, const int nSize /* = 0 */) {
	if (pStr == NULL) {
		return NULL;
	}
	int len = (nSize > 0)? nSize : strlen(pStr);
	char* ret = new(std::nothrow) char[len + 1];
	toUL(pStr, ret, toupper, nSize);
	return ret;
}

void mtkPack::StrUtils::ToUpper(std::string& sStr) {
	transform(sStr.begin(), sStr.end(), sStr.begin(), toupper);
}

std::string mtkPack::StrUtils::ToUpper(const std::string& sStr) {
	std::string sRet(sStr);
	transform(sStr.begin(), sStr.end(), sRet.begin(), toupper);
	return sRet;
}

int mtkPack::StrUtils::SplitToken(const std::string& sStr, const std::string& sTokenSet, std::vector<std::string>& sResult) {
	sResult.clear();
	const int& slen = sStr.length();
	if (slen == 0) return sResult.size();

	int toknum = sTokenSet.length();
	if (toknum > 0) {
		char* lpStr = new char[slen + 1];
		SecureDelete<char> sd1(&lpStr, 1);
		memcpy_s(lpStr, slen + 1, sStr.c_str(), slen + 1);

		const char* lpTok = sTokenSet.c_str();
		char* lpNextTok = NULL;

		char* pch = strtok_s(lpStr, lpTok, &lpNextTok);
		while (pch != NULL) {
			sResult.push_back(pch);
			pch = strtok_s(NULL, lpTok, &lpNextTok);
		}
	}

	return sResult.size();
}

int mtkPack::StrUtils::SplitToken(const std::string& sStr, const std::string& sTokenSet, const char& cDelimiter, std::map<std::string, std::string>& sResult) {
	sResult.clear();
	const int& slen = sStr.length();
	if (slen == 0) return sResult.size();

	int toknum = sTokenSet.length();
	if (toknum > 0) {
		char* lpStr = new char[slen + 1];
		SecureDelete<char> sd1(&lpStr, 1);
		memcpy_s(lpStr, slen + 1, sStr.c_str(), slen + 1);

		const char* lpTok = sTokenSet.c_str();
		char* lpNextTok = NULL;

		char* pch = strtok_s(lpStr, lpTok, &lpNextTok);
		while (pch != NULL) {
			char* lpKey = strchr(pch, cDelimiter);
			if (lpKey == NULL) {
				sResult.insert(std::make_pair(pch, ""));
			} else {
				*lpKey = 0;
				sResult.insert(std::make_pair(pch, (lpKey+1)));
			}
			pch = strtok_s(NULL, lpTok, &lpNextTok);
		}
	}

	return sResult.size();
}

std::string& mtkPack::StrUtils::LTrim(std::string& sStr, const std::string& sTrimSet /* = " " */) {
	if (!sStr.empty()) {
		sStr.erase(0, sStr.find_first_not_of(sTrimSet));
	}
	return sStr;
}

std::string& mtkPack::StrUtils::RTrim(std::string& sStr, const std::string& sTrimSet /* = " " */) {
	if (!sStr.empty()) {
		sStr.erase(sStr.find_last_not_of(sTrimSet) + 1);
	}
	return sStr;
}

std::string& mtkPack::StrUtils::Trim(std::string& sStr, const std::string& sTrimSet /* = " " */) {
	if (!sStr.empty()) {
		sStr.erase(0, sStr.find_first_not_of(sTrimSet));
		sStr.erase(sStr.find_last_not_of(sTrimSet) + 1);
	}
	return sStr;
}

int mtkPack::StrUtils::ToInt(const char* pChar) {
	int ret = atoi(pChar);
	return ret;
}

int mtkPack::StrUtils::ToInt(const wchar_t* pWChar) {
	int ret = _wtoi(pWChar);
	return ret;
}

int mtkPack::StrUtils::ToInt(const std::string& sStr) {
	return StrUtils::ToInt(sStr.c_str());
}

int mtkPack::StrUtils::ToInt(const std::wstring& wsStr) {
	return StrUtils::ToInt(wsStr.c_str());
}

std::string mtkPack::StrUtils::ToStr(const int nVal) {
	char szVal[16] = { 0 };
	_itoa_s(nVal, szVal, 15, 10);
	return szVal;
}

void mtkPack::StrUtils::toUL(const char* pSrc, char* pDst, fnUpperLower pFn, const int nSize /* = 0 */) {
	if (pSrc == NULL || pDst == NULL) {
		return;
	}
	int len = (nSize > 0)? nSize : strlen(pSrc);
	for(int i = 0; i < len; i++) {
		pDst[i] = pFn(pSrc[i]);
	}
}
