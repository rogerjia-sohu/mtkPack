#pragma once
#include <io.h>
#include <cwchar>
#include <string>
#include <vector>

namespace mtkPack {

	enum SplitMode {
		SM_FILE_SIZE = 0,
		SM_FILE_COUNT
	};

	class SplitParam {
	public:
		SplitParam(void);
		SplitParam(const size_t nOffset, const size_t nReadCnt, const std::string& sInFile, const std::string& sOutFile);
	
		size_t Offset;
		size_t ReadCnt;
		std::string InFile;
		std::string OutFile;
	};

	class File {
	public:
		File(void);
		virtual ~File(void);

		static const int Exist(const char* cFilename);
		static const int Exist(const wchar_t* wcFilename);

		static const int IsDirectory(const char* cFilename);
		static const int IsDirectory(const wchar_t* wcFilename);
		
		static std::string ExtractFileExt(const std::string& sFileName);
		static std::string ExtractFileName(const std::string& sFullName, const bool bWithExt = true, const bool bWithPath = false);
		static std::string ExtractFilePath(const std::string& sFullName);

		static std::string TrName(const std::string& sExpandableName);

		static size_t Size(const char* cFilename);
		static size_t Size(const wchar_t* wcFilename);

		static int Split(const std::string& sFileName, const SplitMode mode, const size_t nSizeOrCount);
		static int Join(const std::string& sFileName, const size_t nCnt, const std::string& sNewFileName);
		static int Join(const std::string& sFirstFileName, const std::string& sLastFileName, const std::string& sNewFileName);

		class Iterator {
		public:
			Iterator(void);
			virtual ~Iterator(void);

			void SetCompatibleMode(const int bOnOff);
			virtual void Find(const std::string& sSearchDir, const std::string& sFilePatterns);
			virtual int Next(void);
			virtual int operator ++();

			virtual int Close(void);
			const std::string FilePath(void);
			const std::string FileName(void);

			const int IsDirectory(void);

		protected:
			void Init(void);
			void BuildDirList(void);
			int BuildFilePatternList(void);

			void Invalidate(void);
			
			int IsValid(void);
			inline void SetDirLevel(const int nCnt) { mDirLevel = nCnt; }
			
			int mCompatibleMode;

			std::string mBaseDir;
			std::string mFilePatterns;
			std::string mSeparators;

			std::vector<std::string> mPatternList;
			std::vector<std::string> mDirList;

			intptr_t mHFind;
			int mPatternIndex;
			int mDirIndex;
			int mDirLevel;
			struct _finddata_t mFindData;
		};

	protected:
		static unsigned int __stdcall SplittingThread(void* pParam);
		static std::string mEnvPrefixChar;
	};

}