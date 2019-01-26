#include "IniFile.h"

#include <fstream>
#include <utility>
#include "File.h"
#include "StrUtils.h"

mtkPack::IniFile::IniFile(IniFileStoreMode smMode /* = IFSM_SLIM */)
	:StoreMode(smMode), mCommentChar(';'), mDelimiterChar('='), mSectionBeginChar('['), mSectionEndChar(']')
	,mGlobalCommentCnt(0), mSectionCommentCnt(0)
{
}

mtkPack::IniFile::IniFile(const std::string& sFileName, IniFileStoreMode smMode /* = IFSM_SLIM */)
	:StoreMode(smMode), mCommentChar(';'), mDelimiterChar('='), mSectionBeginChar('['), mSectionEndChar(']')
	,mGlobalCommentCnt(0), mSectionCommentCnt(0)
	,mFileName(sFileName)
{
	Load();
}

mtkPack::IniFile::~IniFile(void)
{
	Store();
}

int mtkPack::IniFile::Load(void) {
	int keycnt = 0;
	if (File::Exist(mFileName.c_str())) {
		std::ifstream fin(mFileName);
		if (fin) {
			mGlobalCommentMap.clear();
			mGlobalDataMap.clear();
			mDataMap.clear();
			int pos = 0, slen = 0;
			
			char szSectionCommentCnt[mDigitLen] = {0};
			std::string sline, sSec, sPrevSec, sKey, sVal;
			KeyValStrMap kvmap;
			while (getline(fin, sline)) {
				slen = sline.length();
				if (slen == 0 ) {
					continue; // skip empty lines
				}

				if (sline[0] == mCommentChar) {
					if (sSec.empty()) {
						pos = sline.find(mDelimiterChar);
						if (pos < 0) {
							// store all global comment (NOT including commented global key) lines separately
							mGlobalCommentMap.insert(make_pair(mGlobalCommentCnt++, sline));
						} else {
							// store global comment line like: {";key1", "value1"}
							sKey = sline.substr(0, pos);
							sVal = sline.substr(pos + 1, slen - pos - 1);
							StrUtils::RTrim(sKey);
							StrUtils::LTrim(sVal);
							kvmap.insert(make_pair(sKey, sVal));
							keycnt++;
						}
						continue;
					} else {
						pos = sline.find(mDelimiterChar);
						if (pos < 0) {
							// store in-section comment lines in their sections formatting like:
							//		{";num", ";commen line1"}
							sprintf_s(szSectionCommentCnt, mDigitLen, ";%d", mSectionCommentCnt++);
							kvmap.insert(make_pair(szSectionCommentCnt, sline));
							continue;
						}// if (pos < 0)
					}
					//continue;
				}
				if (sline[0] == mSectionBeginChar && sline[slen-1] == mSectionEndChar) {
					if (!sSec.empty()) {
						mDataMap.insert(make_pair(sSec, kvmap));
						kvmap.clear();
						sPrevSec = sSec;
					} else {
						if (kvmap.size() > 0) {
							// global keys and values within no section
							mGlobalDataMap.insert(make_pair(0, kvmap));
							kvmap.clear();
						}
					}
					sSec = sline.substr(1, slen - 2);
				} else {
					pos = sline.find(mDelimiterChar);
					if (pos > 0) {
						sKey = sline.substr(0, pos);
						sVal = sline.substr(pos + 1, slen - pos - 1);
						StrUtils::RTrim(sKey);
						StrUtils::LTrim(sVal);
						kvmap.insert(make_pair(sKey, sVal));
						keycnt++;
					}// if (pos> 0)
				}
			}// while (getline(fin, sline))
			fin.close();
			if (sPrevSec != sSec) {
				mDataMap.insert(make_pair(sSec, kvmap));
			} else if (sSec.empty()) {
				mGlobalDataMap.insert(make_pair(0, kvmap));
			} else {
				//
				int a = 0;
			}
			kvmap.clear();
		}// if (fin)
	}
	return keycnt;
}

int mtkPack::IniFile::Load(const std::string& sFileName) {
	mFileName = sFileName;
	return Load();
}

int mtkPack::IniFile::Store(void) {
	int ret = 0;
	if (mFileName.length() > 0 && (mDataMap.size() > 0 || mGlobalDataMap.size() > 0 || mGlobalCommentMap.size() > 0)) {
		std::ofstream fout(mFileName);
		if (fout) {
			// global comments
			int gcmtcnt = mGlobalCommentMap.size();
			for (int i = 0; i < gcmtcnt; i++) {
				fout<<mGlobalCommentMap.at(i)<<std::endl;
			}

			// global keys and values
			KeyValStrMap::const_iterator keyitr;
			GlobalDataStrMap::const_iterator gitr = mGlobalDataMap.begin();
			if (gitr != mGlobalDataMap.end()) {
				for (keyitr = gitr->second.begin(); keyitr != gitr->second.end(); keyitr++) {
					// data line: key=value including comment line like ;key=val
					if (StoreMode == IFSM_CLEAR) {
						fout<<keyitr->first<<" "<<mDelimiterChar<<" "<<keyitr->second<<std::endl;
					} else {
						fout<<keyitr->first<<mDelimiterChar<<keyitr->second<<std::endl;
					}
				}
				fout<<std::endl;
			}

			// sectional keys and values
			for (DataStrMap::const_iterator secitr = mDataMap.begin(); secitr != mDataMap.end(); secitr++) {
				// [section name]
				fout<<mSectionBeginChar<<secitr->first<<mSectionEndChar<<std::endl;
					
				// all keys and values in each section
				for (keyitr = secitr->second.begin(); keyitr != secitr->second.end(); keyitr++) {
					if (keyitr->first[0] == mCommentChar && keyitr->second[0] == mCommentChar) {
						// in-section general(;description) comment line
						fout<<keyitr->second<<std::endl;							
					} else {
						// data line: key=value including comment line like ;key=val
						if (StoreMode == IFSM_CLEAR) {
							fout<<keyitr->first<<" "<<mDelimiterChar<<" "<<keyitr->second<<std::endl;
						} else {
							fout<<keyitr->first<<mDelimiterChar<<keyitr->second<<std::endl;
						}
					}
				}
				fout<<std::endl;
			}
			fout.close();
			ret = 1;
		}// if (fout)
	}
	return ret;
}

void mtkPack::IniFile::Clear(void) {
	mGlobalCommentMap.clear();
	mGlobalDataMap.clear();
	mDataMap.clear();
}

std::string & mtkPack::IniFile::GetFileName(void) {
	return mFileName;
}

int mtkPack::IniFile::GetInt(const std::string& sSection, const std::string& sKey, const int iDefault /* = 0 */) {
	int ret = iDefault;
	std::string sRet(GetString(sSection, sKey));
	if (sRet.length() > 0) {
		ret = atoi(sRet.c_str());
	}
	return ret;
}

std::string mtkPack::IniFile::GetString(const std::string& sSection, const std::string& sKey, const std::string& sDefault /* = "" */) {
	std::string ret(sDefault);
	DataStrMap::const_iterator secitr = mDataMap.find(sSection);
	if (secitr != mDataMap.end()) {
		KeyValStrMap::const_iterator KeyItr = secitr->second.find(sKey);
		if (KeyItr != secitr->second.end()) {
			ret = KeyItr->second;
		}
	}
	return ret;
}

void mtkPack::IniFile::SetInt(const std::string& sSection, const std::string& sKey, const int iVal) {
	char szVal[65] = {0};
	if (_itoa_s(iVal, szVal, 65, 10) != 0) {
		szVal[0] = '0';
		szVal[1] = '\0';
	}
	SetString(sSection, sKey, szVal);
}

void mtkPack::IniFile::SetString(const std::string& sSection, const std::string& sKey, const std::string& sVal) {
	DataStrMap::iterator secitr = mDataMap.find(sSection);
	if (secitr != mDataMap.end()) {
		KeyValStrMap::iterator keyitr = secitr->second.find(sKey);
		if (keyitr != secitr->second.end()) {
			// update existing value
			keyitr->second = sVal;
		} else {
			std::string cmtkey(mCommentChar + sKey);
			keyitr = secitr->second.find(cmtkey);
			if (keyitr != secitr->second.end()) {
				// update value of a commented key
				UncommentKey(sSection, sKey);
				keyitr->second = sVal;
			} else {
				// insert key and value into existing section
				secitr->second.insert(make_pair(sKey, sVal));
			}
		}
	} else {
		// insert key and value into new section
		KeyValStrMap kvmap;
		kvmap.insert(make_pair(sKey, sVal));
		mDataMap.insert(make_pair(sSection, kvmap));
	}
}

int mtkPack::IniFile::GetGlobalInt(const std::string& sKey, const int iDefault /* = 0 */) {
	int ret = iDefault;
	std::string sRet(GetGlobalString(sKey));
	if (sRet.length() > 0) {
		ret = atoi(sRet.c_str());
	}
	return ret;
}

std::string mtkPack::IniFile::GetGlobalString(const std::string& sKey, const std::string& sDefault /* = "" */) {
	std::string ret(sDefault);
	GlobalDataStrMap::const_iterator gitr = mGlobalDataMap.begin();
	if (gitr != mGlobalDataMap.end()) {
		KeyValStrMap::const_iterator KeyItr = gitr->second.find(sKey);
		if (KeyItr != gitr->second.end()) {
			ret = KeyItr->second;
		}
	}
	return ret;
}

void mtkPack::IniFile::SetGlobalInt(const std::string& sKey, const int iVal) {
	char szVal[65] = {0};
	if (_itoa_s(iVal, szVal, 65, 10) != 0) {
		szVal[0] = '0';
		szVal[1] = '\0';
	}
	SetGlobalString(sKey, szVal);
}

void mtkPack::IniFile::SetGlobalString(const std::string& sKey, const std::string& sVal) {
	GlobalDataStrMap::iterator gitr = mGlobalDataMap.begin();
	if (gitr != mGlobalDataMap.end()) {
		KeyValStrMap::iterator keyitr = gitr->second.find(sKey);
		if (keyitr != gitr->second.end()) {
			// update existing value
			keyitr->second = sVal;
		} else {
			std::string cmtkey(mCommentChar + sKey);
			keyitr = gitr->second.find(cmtkey);
			if (keyitr != gitr->second.end()) {
				// update value of a commented key
				UncommentGlobalKey(sKey);
				keyitr->second = sVal;
			} else {
				// insert key and value into existing section
				gitr->second.insert(make_pair(sKey, sVal));
			}
		}
	} else {
		// create global keys and values
		KeyValStrMap kvmap;
		kvmap.insert(make_pair(sKey, sVal));
		mGlobalDataMap.insert(make_pair(0, kvmap));
	}
}

int mtkPack::IniFile::SectionExist(const std::string& sSection) const {
	DataStrMap::iterator secitr;
	int ret = const_cast<IniFile*>(this)->FindSection(sSection, secitr);
	return ret;
}

int mtkPack::IniFile::IsSectionEmpty(const std::string& sSection) const {
	DataStrMap::iterator secitr;
	int ret = const_cast<IniFile*>(this)->FindSection(sSection, secitr);
	if (ret) {
		ret = secitr->second.empty();
	} else {
		ret = 0;// no section exists
	}
	return ret;
}

int mtkPack::IniFile::RemoveKey(const std::string& sSection, const std::string& sKey) {
	DataStrMap::iterator secitr;
	int ret = FindSection(sSection, secitr);
	if (ret) {
		ret = secitr->second.erase(sKey);
	}
	return ret;
}

int mtkPack::IniFile::RemoveSection(const std::string& sSection) {
	int ret = mDataMap.erase(sSection);
	return ret;
}

int mtkPack::IniFile::CommentKey(const std::string& sSection, const std::string& sKey) {
	int keycnt = 0;
	KeyValStrMap::iterator keyitr;
	if (FindKey(sSection, sKey, keyitr)) {
		const_cast<std::string &>(keyitr->first).insert(0, 1, mCommentChar);
		keycnt++;
	}
	return keycnt;
}

int mtkPack::IniFile::CommentGlobalKey(const std::string& sKey) {
	int keycnt = 0;
	KeyValStrMap::iterator keyitr;
	if (FindGlobalKey(sKey, keyitr)) {
		const_cast<std::string &>(keyitr->first).insert(0, 1, mCommentChar);
		keycnt++;
	}
	return keycnt;
}

int mtkPack::IniFile::UncommentKey(const std::string& sSection, const std::string& sKey) {
	int keycnt = 0;
	KeyValStrMap::iterator keyitr;
	std::string cmtkey(mCommentChar + sKey);
	if (FindKey(sSection, cmtkey, keyitr)) {
		const_cast<std::string &>(keyitr->first).erase(0, 1);
		keycnt++;
	}
	return keycnt;
}

int mtkPack::IniFile::UncommentGlobalKey(const std::string& sKey) {
	int keycnt = 0;
	KeyValStrMap::iterator keyitr;
	std::string cmtkey(mCommentChar + sKey);
	if (FindGlobalKey(cmtkey, keyitr)) {
		const_cast<std::string &>(keyitr->first).erase(0, 1);
		keycnt++;
	}
	return keycnt;
}

int mtkPack::IniFile::GetSectionInfo(std::map<std::string, int>& siSectionInfo) {
	siSectionInfo.clear();

	SectionInfo::iterator secinfoitr;
	for (DataStrMap::const_iterator secitr = mDataMap.begin(); secitr != mDataMap.end(); secitr++) {
		secinfoitr = siSectionInfo.find(secitr->first);
		if (secinfoitr == siSectionInfo.end()) {
			siSectionInfo.insert(make_pair(secitr->first, 1));
		} else {
			secinfoitr->second++;
		}
	}

	return siSectionInfo.size();
}

int mtkPack::IniFile::GetSectionCount(const std::string& sSection) {
	return mDataMap.count(sSection);
}

const mtkPack::IniFile::GlobalCommentMap & mtkPack::IniFile::GetGlobalComment(void) {
	return mGlobalCommentMap;
}

const mtkPack::IniFile::GlobalDataStrMap & mtkPack::IniFile::GetGlobalData(void) {
	return mGlobalDataMap;
}

const mtkPack::IniFile::DataStrMap & mtkPack::IniFile::GetData(void) {
	return mDataMap;
}

int mtkPack::IniFile::FindKey(const std::string& sSection, const std::string& sKey, KeyValStrMap::iterator& KeyItr) {
	int ret = 0;
	
	DataStrMap::iterator secitr;
	ret = FindSection(sSection, secitr);
	if (ret) {
		KeyItr = secitr->second.find(sKey);
		ret = (KeyItr != secitr->second.end());
	}
	return ret;
}

int mtkPack::IniFile::FindGlobalKey(const std::string& sKey, KeyValStrMap::iterator& KeyItr) {
	int ret = 0;

	GlobalDataStrMap::iterator gitr = mGlobalDataMap.begin();
	if (gitr != mGlobalDataMap.end()) {
		KeyItr = gitr->second.find(sKey);
		ret = (KeyItr != gitr->second.end());
	}
	return ret;
}

int mtkPack::IniFile::FindSection(const std::string& sSection, DataStrMap::iterator& SecItr) {
	SecItr = mDataMap.find(sSection);
	int ret = (SecItr != mDataMap.end());
	return ret;
}

/////////////////////////////////////////////////////////////////////////

mtkPack::IniFileCtrl::IniFileCtrl(void)
{

}

mtkPack::IniFileCtrl::IniFileCtrl(const std::string& sDir, const std::string& sFilePatterns /* = "*.ini" */)
	:mSearchDir(sDir), mFilePatterns(sFilePatterns)
{
	Load();
}

mtkPack::IniFileCtrl::~IniFileCtrl(void)
{

}

int mtkPack::IniFileCtrl::Load(void) {
	int cnt = 0;
	File::Iterator fitr;
	fitr.Find(mSearchDir, mFilePatterns);
	while(fitr.Next()) {
		IniFile inif(fitr.FilePath());
		mIniFileList.insert(make_pair(fitr.FilePath(), inif));
		cnt++;
	}
	return cnt;
}

int mtkPack::IniFileCtrl::Load(const std::string& sDir, const std::string& sFilePatterns /* = "*.ini" */) {
	mSearchDir = sDir;
	mFilePatterns = sFilePatterns;
	return Load();
}