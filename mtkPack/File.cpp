#include <cmath>
#include <fstream>
#include <sys/stat.h>
#include <process.h>
#include <windows.h>

#include "File.h"
#include "SecureDelete.h"
#include "StrUtils.h"
#include "ComputerInfo.h"
#include "Synchronization.h"

std::string mtkPack::File::mEnvPrefixChar = "$";

mtkPack::SplitParam::SplitParam(void)
	:Offset(0), ReadCnt(0), InFile(""), OutFile("")
{
}

mtkPack::SplitParam::SplitParam(const size_t nOffset, const size_t nReadCnt, const std::string& sInFile, const std::string& sOutFile)
	:Offset(nOffset), ReadCnt(nReadCnt), InFile(sInFile), OutFile(sOutFile)
{
}

mtkPack::File::File(void)
{
}

mtkPack::File::~File(void)
{
}

const int mtkPack::File::Exist(const char* cFilename) {
	int ret = 0;
	if (cFilename && _access_s(cFilename, 0) == 0) {
		ret = 1;
	}
	return ret;
}

const int mtkPack::File::Exist(const wchar_t* wcFilename) {
	int ret = 0;
	if (wcFilename && _waccess_s(wcFilename, 0) == 0) {
		ret = 1;
	}
	return ret;
}

const int mtkPack::File::IsDirectory(const char* cFilename) {
	int ret = 0;
	struct _stat buf;
	if (cFilename && _stat(cFilename, &buf) == 0) {
		ret = (buf.st_mode & _S_IFDIR) ? 1 : 0;
	}
	return ret;
}

const int mtkPack::File::IsDirectory(const wchar_t* wcFilename) {
	int ret = 0;
	struct _stat buf;
	if (wcFilename && _wstat(wcFilename, &buf) == 0) {
		ret = (buf.st_mode & _S_IFDIR) ? 1 : 0;
	}
	return ret;
}

std::string mtkPack::File::ExtractFileExt(const std::string& sFileName) {
	int npos = sFileName.rfind(".");
	return ((npos >= 0) ? sFileName.substr( npos + 1, sFileName.length() - 1) : "");
}

std::string mtkPack::File::ExtractFileName(const std::string& sFullName, const bool bWithExt /* = true */, const bool bWithPath /* = false */) {
	if (bWithExt && bWithPath) {
		return sFullName;
	}

	std::string sFileName(sFullName);
	if (!bWithPath) {
		int lastpos = sFullName.rfind("\\");
		sFileName = sFullName.substr(lastpos + 1, sFullName.length() - lastpos);
	}

	if (!bWithExt) {
		sFileName = sFileName.substr(0, sFileName.rfind("."));
	}
	return sFileName;
}

std::string mtkPack::File::ExtractFilePath(const std::string& sFullName) {
	int lastpos = sFullName.rfind("\\");
	std::string sFileName = sFullName.substr(0, lastpos);
	if (sFileName[sFileName.length()-1] == ':') {
		sFileName += '\\';
	}
	return sFileName;
}

std::string mtkPack::File::TrName(const std::string& sExpandableName) {
	std::string result(sExpandableName);
	char* pEnvVal = NULL;

	std::vector<std::string> sEnvList;
	int envcnt = StrUtils::SplitToken(sExpandableName, mEnvPrefixChar, sEnvList);
	if (envcnt > 0) {
		unsigned int cbSize = 0;
		int pos = 0, envlen = 0, i;
		for (i = 0; i < envcnt; i++) {
			const std::string& sOneEnv = sEnvList[i];
			if (i == 0 && sExpandableName.find(sOneEnv, 0) == 0) {
				continue;
			}
			
			envlen = sOneEnv.length() + 1;
			char* lpEnv = new(std::nothrow) char[envlen];
			SecureDelete<char> sdlpEnv(&lpEnv,1);
			memcpy_s(lpEnv, envlen, sOneEnv.c_str(), envlen);
			char ch = 0;
			
			for (int i = 1; i < envlen; i++) {
				ch = lpEnv[i];
				lpEnv[i] = 0;
				getenv_s(&cbSize, NULL, 0, lpEnv);
				if (cbSize > 0) {
					pEnvVal = new char[cbSize];
					SecureDelete<char> sdEnvVal(&pEnvVal, 1);
					getenv_s(&cbSize, pEnvVal, cbSize, lpEnv);
					pos = result.find(mEnvPrefixChar + lpEnv);
					result = result.replace(pos, i + 1, pEnvVal);
					break;
				}
				lpEnv[i] = ch;
			}// for
		}// outer for
	}

	return result;
}

size_t mtkPack::File::Size(const char* cFilename) {
	size_t filesize = 0;
	if (Exist(cFilename)) {
		struct _stat buf;
		_stat(cFilename, &buf);
		filesize = buf.st_size;
	}
	return filesize;
}

size_t mtkPack::File::Size(const wchar_t* wcFilename) {
	size_t filesize = 0;
	if (Exist(wcFilename)) {
		struct _stat buf;
		_wstat(wcFilename, &buf);
		filesize = buf.st_size;
	}
	return filesize;
}

unsigned int __stdcall mtkPack::File::SplittingThread(void* pParam) {
	unsigned int ret = 0;
	if (pParam == NULL) {
		return ret;
	}

	SplitParam* pSplitParam = (SplitParam*)pParam;
	
	std::ifstream fin(pSplitParam->InFile, std::ofstream::binary);
	std::ofstream fout(pSplitParam->OutFile, std::ofstream::binary);
	if (fin && fout) {
		fin.seekg(pSplitParam->Offset);
		char* buffer = new char[pSplitParam->ReadCnt];
		SecureDelete<char> sd1(&buffer, 1);
		fin.read(buffer, pSplitParam->ReadCnt);
		fout.write(buffer, pSplitParam->ReadCnt);
		ret = 1;
	}

	_endthreadex(ret);
	return ret;
}

int mtkPack::File::Split(const std::string& sFileName, const SplitMode mode, const size_t nSizeOrCount) {
	int ret = 0;
	if (!Exist(sFileName.c_str())) {
		return 0;
	}

	size_t filesize = File::Size(sFileName.c_str());
	size_t cnt = (size_t)ceil((double)filesize / nSizeOrCount);

	size_t filecnt = 0;
	size_t onefilesize = 0;
	if (mode == SM_FILE_SIZE) {
		onefilesize = nSizeOrCount;
		filecnt = cnt;
	} else if (mode == SM_FILE_COUNT) {
		onefilesize = cnt;
		filecnt = (size_t)ceil((double)filesize / onefilesize);
	}
	short int nSuffixDigit = 0;
	for (int i = filecnt; i > 0; i /= 10 ) {
		nSuffixDigit++;
	}
	size_t namelen = sFileName.length() + nSuffixDigit + 2;
	char szNameFmt[10] = {0};
	sprintf_s(szNameFmt, 10, "%%s.%%0%dd", nSuffixDigit);

	size_t lastfilesize = (filecnt > 1) ? filesize - onefilesize * (filecnt-1) : filesize;

	CPUInfo cpu;
	size_t threadnum = (filecnt > cpu.CoreNum)? cpu.CoreNum : filecnt;

	char* buffer = NULL;
	SecureDelete<char> sd1(&buffer, 1);
	
	HANDLE* phThread = new HANDLE[threadnum];
	SecureDelete<HANDLE> sdThread(&phThread, 1);

	SplitParam* pParamlist = new SplitParam[threadnum];
	SecureDelete<SplitParam> sdParamList(&pParamlist, 1);
	
	for (size_t i = 0; i < filecnt; i++) {
		char* pFilename = new char[namelen];
		SecureDelete<char> sd1(&pFilename, 1);
		sprintf_s(pFilename, namelen, szNameFmt, sFileName.c_str(), i);		
		SplitParam param(i * onefilesize, ((i == filecnt - 1)? lastfilesize : onefilesize), sFileName, pFilename);

		WaitForSingleObject(phThread[i % threadnum], INFINITE);
		pParamlist[i % threadnum] = param;
		phThread[i % threadnum] = (HANDLE)_beginthreadex(NULL, 0, SplittingThread, (void*)&pParamlist[i % threadnum], 0, NULL);
	}

	DWORD dwWait = WaitForMultipleObjects(threadnum, phThread, true, INFINITE);

	return ret;
}

int mtkPack::File::Join(const std::string& sFileName, const size_t nCnt, const std::string& sNewFileName) {
	int cnt = 0;
#if 1
	if (File::Exist(sFileName.c_str())) {
		std::string sFileNameNoExt = File::ExtractFileName(sFileName, false, true);
		std::string sExt = File::ExtractFileExt(sFileName);
		int extlen = sExt.length();
		char szNameFmt[10] = {0};
		sprintf_s(szNameFmt, 10, "%%s.%%0%dd", extlen);

		std::ofstream fout(sNewFileName, std::ofstream::binary);
		if (fout) {
			for (size_t i = 0; i < nCnt; i++) {
				char* onefilename = new char[sFileName.length() + 1];
				SecureDelete<char> sd1(&onefilename, 1);

				sprintf_s(onefilename, sFileName.length() + 1, szNameFmt, sFileNameNoExt.c_str(), i);				
				size_t filesize = File::Size(onefilename);
				if (filesize == 0) {
					continue;
				}
				char* buffer = new char[filesize];
				SecureDelete<char> sd2(&buffer, 1);

				std::ifstream fin(onefilename, std::ofstream::binary);
				if (fin) {
					fin.read(buffer, filesize);
					fout.write(buffer, filesize);
					cnt++;
				}
			}// for (; cnt < nCnt; cnt++) {
		}// if (fout) {
	}
#else
//
#endif
	return cnt;
}

int mtkPack::File::Join(const std::string& sFirstFileName, const std::string& sLastFileName, const std::string& sNewFileName) {
	int cnt = 0;
	if (File::Exist(sFirstFileName.c_str()) && File::Exist(sLastFileName.c_str())) {
		cnt = 1 + StrUtils::ToInt(File::ExtractFileExt(sLastFileName)) - StrUtils::ToInt(File::ExtractFileExt(sFirstFileName));
		cnt = File::Join(sFirstFileName, cnt, sNewFileName);
	}
	return cnt;
}

mtkPack::File::Iterator::Iterator(void) {
	Init();
}

mtkPack::File::Iterator::~Iterator(void) {
	Close();
}

void mtkPack::File::Iterator::SetCompatibleMode(const int bOnOff) {
	mCompatibleMode = bOnOff;
}

void mtkPack::File::Iterator::Find(const std::string& sSearchDir, const std::string& sFilePatterns) {
	mBaseDir = sSearchDir;
	if (mBaseDir.length() > 0 && mBaseDir[mBaseDir.length()-1] != '\\' ){
		mBaseDir += "\\";
	}

	mFilePatterns = sFilePatterns;
	mDirList.clear();

	BuildFilePatternList();

	if (!mCompatibleMode) {
		// Only execute in enhanced (non-compatible) mode
		if (mDirLevel == 0) {
			BuildDirList();
		}
	}
}

int mtkPack::File::Iterator::Next(void) {
	int ret = 0;
	if (mPatternList.empty()) {
		return ret;
	}

	std::string sSearchName;

	while (!ret) { 
		if (IsValid()) {
			ret = (_findnext(mHFind, &mFindData) == 0)? 1 : 0;
		} else{
			if (mDirIndex >= 0) {
				// Searching sub directories
				sSearchName = mBaseDir + mDirList[mDirIndex] + "\\" + mPatternList[mPatternIndex];
			} else {
				// Searching base directory
				sSearchName = mBaseDir + mPatternList[mPatternIndex];
			}
			mHFind = _findfirst(sSearchName.c_str(), &mFindData);
			ret = IsValid();
		}
		if (mCompatibleMode) {
			// no further enhanced features
			break;
		}

		if (ret && (strcmp(mFindData.name, ".") == 0 || strcmp(mFindData.name, "..") == 0)) {
			// skip 
			ret = 0;
			continue;
		}

		if (!ret) {
			if (mPatternIndex < (int)mPatternList.size() -1) {
				// Searching next pattern
				mPatternIndex++;
				Invalidate();
				continue;
			} else {
				// Searching next sub directory and all patterns
				if (mDirIndex < (int)mDirList.size() -1 ) {
					mPatternIndex = 0;
					mDirIndex++;
					Invalidate();
					continue;;
				}
			}
		}
		break;
	}

	return ret;
}

int mtkPack::File::Iterator::operator++() {
	return Next();
}

int mtkPack::File::Iterator::Close(void) {
	int ret = 0;

	if (IsValid()) {
		_findclose(mHFind);
		mHFind = -1;
	}

	mDirLevel = 0;

	mPatternIndex = -1;
	mPatternList.clear();

	mDirIndex = -1;
	mDirList.clear();

	return ret;
}

const std::string mtkPack::File::Iterator::FilePath(void) {
	if (!IsValid()) return "";
	std::string pathsep = "\\";

	if (mDirIndex >= 0) {
		return mBaseDir + mDirList[mDirIndex] + pathsep + FileName();
	}
	return mBaseDir + FileName();
}

const std::string mtkPack::File::Iterator::FileName(void) {
	return std::string(mFindData.name);
}

const int mtkPack::File::Iterator::IsDirectory(void) {
	return File::IsDirectory(FilePath().c_str());
}

void mtkPack::File::Iterator::Init(void) {
	mCompatibleMode = 0; // defaulting to enhanced mode

	mBaseDir = "";
	mFilePatterns = "";
	mSeparators = ":|";

	mHFind = -1;
	mDirIndex = -1;
	mDirLevel = 0;
	mDirList.clear();
	mPatternIndex = -1;
	mPatternList.clear();
}


void mtkPack::File::Iterator::BuildDirList(void) {
	if (mDirLevel == 0) {
		mDirList.clear();
	}

	int nBaseLen = mBaseDir.length();

	File::Iterator dirItr;
	dirItr.SetDirLevel(++mDirLevel);
	dirItr.Find(mBaseDir, "*");
	while (dirItr.Next()){
		if (dirItr.IsDirectory()) {
			mDirList.push_back(dirItr.FilePath().substr(nBaseLen, dirItr.FilePath().length() - nBaseLen));
		}
	}

	for (int i = 0; i < (int)mDirList.size(); i++) {
		dirItr.Close();
		dirItr.SetDirLevel(++mDirLevel);
		dirItr.Find(mBaseDir + mDirList[i], "*");
		while (dirItr.Next()) {
			if (dirItr.IsDirectory()) {
				mDirList.push_back(dirItr.FilePath().substr(nBaseLen, dirItr.FilePath().length() - nBaseLen));
			}
		} // end while
	} // end for
}

int mtkPack::File::Iterator::BuildFilePatternList(void) {
	int cnt = 0;
	int nSize = mFilePatterns.length() +1;
	if (nSize <= 1) {
		return cnt;
	}

	char* patlist = new char[nSize];
	strcpy_s(patlist, nSize, mFilePatterns.c_str());

	char* aPat = NULL;
	char* nextPat = NULL;
	aPat = strtok_s(patlist, mSeparators.c_str(), &nextPat);
	while (aPat != NULL) {
		mPatternList.push_back(std::string(aPat));
		aPat = strtok_s(NULL, mSeparators.c_str(), &nextPat);
		cnt++;
	}

	delete[] patlist;
	patlist = NULL;

	if (cnt > 0) {
		mPatternIndex++;
	}

	return cnt;
}

void mtkPack::File::Iterator::Invalidate(void) {
	if (IsValid()) {
		_findclose(mHFind);
	}
	mHFind = -1;
}

int mtkPack::File::Iterator::IsValid(void){
	return (mHFind == -1)? 0 : 1;
}