#pragma once
#include <string>
#include <map>

namespace mtkPack {

	class IniFile {
	public:
		typedef std::map<int, std::string> GlobalCommentMap;
		typedef std::map<std::string, std::string> KeyValStrMap;
		typedef std::map<int, KeyValStrMap> GlobalDataStrMap;
		typedef std::multimap<std::string, KeyValStrMap> DataStrMap;
		typedef std::map<std::string, int> SectionInfo;

		enum IniFileStoreMode {
			IFSM_CLEAR = 0,	// Store by keeping one space in each side of mDelimiterChar like "key = value"
			IFSM_SLIM		// Store by removing the above two spaces around mDelimiterChar like "key=value"
		};

		IniFileStoreMode StoreMode;

		IniFile(IniFileStoreMode smMode = IFSM_SLIM);
		IniFile(const std::string& sFileName, IniFileStoreMode smMode = IFSM_SLIM);
		virtual ~IniFile(void);

		int Load(void);
		int Load(const std::string& sFileName);
		int Store(void);
		void Clear(void);
		std::string & GetFileName(void);

		int GetInt(const std::string& sSection, const std::string& sKey, const int iDefault = 0);
		std::string GetString(const std::string& sSection, const std::string& sKey, const std::string& sDefault = "");
		void SetInt(const std::string& sSection, const std::string& sKey, const int iVal);
		void SetString(const std::string& sSection, const std::string& sKey, const std::string& sVal);

		int GetGlobalInt(const std::string& sKey, const int iDefault = 0);
		std::string GetGlobalString(const std::string& sKey, const std::string& sDefault = "");
		void SetGlobalInt(const std::string& sKey, const int iVal);
		void SetGlobalString(const std::string& sKey, const std::string& sVal);

		int SectionExist(const std::string& sSection) const;
		int IsSectionEmpty(const std::string& sSection) const;

		int RemoveKey(const std::string& sSection, const std::string& sKey);
		int RemoveSection(const std::string& sSection);

		int CommentKey(const std::string& sSection, const std::string& sKey);
		int UncommentKey(const std::string& sSection, const std::string& sKey);

		int CommentGlobalKey(const std::string& sKey);
		int UncommentGlobalKey(const std::string& sKey);

		int GetSectionInfo(SectionInfo& siSectionInfo);
		int GetSectionCount(const std::string& sSection);

		const GlobalCommentMap & GetGlobalComment(void);
		const GlobalDataStrMap & GetGlobalData(void);
		const DataStrMap & GetData(void);

	protected:
		int FindKey(const std::string& sSection, const std::string& sKey, KeyValStrMap::iterator& KeyItr);
		int FindGlobalKey(const std::string &sKey, KeyValStrMap::iterator& KeyItr);
		int FindSection(const std::string& sSection, DataStrMap::iterator& SecItr);

		const char mCommentChar; // ';'
		const char mDelimiterChar; // '='
		const char mSectionBeginChar; // '['
		const char mSectionEndChar;  // ']'
		int mGlobalCommentCnt;
		int mSectionCommentCnt;

		std::string mFileName;
		GlobalCommentMap mGlobalCommentMap;
		GlobalDataStrMap mGlobalDataMap;
		DataStrMap mDataMap;

		static const int mDigitLen = 16;
	};

	class IniFileCtrl{
	public:
		typedef std::map<std::string, IniFile> IniFileList;
		IniFileCtrl(void);
		IniFileCtrl(const std::string& sDir, const std::string& sFilePatterns = "*.ini");
		virtual ~IniFileCtrl(void);

		int Load(void);
		int Load(const std::string& sDir, const std::string& sFilePatterns = "*.ini");

	protected:
		std::string mSearchDir;
		std::string mFilePatterns;
		IniFileList mIniFileList;
	};
}