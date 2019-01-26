#include <string>
#include <fstream>

#include "Cabinet.h"
#include "File.h"
#include "StrUtils.h"

mtkPack::Cabinet::Cabinet(void)
{
	InitEngine();
}

mtkPack::Cabinet::Cabinet(const std::string& sCabFile) {
	InitEngine();
	SetFileName(sCabFile);
}

mtkPack::Cabinet::~Cabinet(void)
{
}

void mtkPack::Cabinet::SetFileName(const std::string& sCabFile) {
	mCabFile = sCabFile;
}

INT mtkPack::Cabinet::GetFileList(std::vector<std::string>& svFileList) {
	INT cnt = 0;
	std::string sTempfile = File::TrName("$TEMP\\~cab.tmp");
	
	if (mEngineType == ET_CABARC) {
		mCabEngine.SetCmdLine("L " + mCabFile + " >" + sTempfile);
	} else {
		mCabEngine.SetCmdLine("-D " + mCabFile + " | find /i \".cab\" >" + sTempfile);
	}
	
	mCabEngine.Exec();

	std::ifstream fin(sTempfile);
	if (fin) {
		int pos = 0;
		std::string sline, compressedfile;
		
		if (mEngineType == ET_CABARC) {
			std::vector<std::string> strlist;
			while(getline(fin, sline)) {
				if (StrUtils::SplitToken(sline, " \t", strlist) > 0) {
					if (strlist.size() == 5 && atoi(strlist[1].c_str()) > 0) {
						svFileList.push_back(strlist[0]);
						cnt++;
					}
				}
			}
		} else {
			while(getline(fin, sline)) {
				pos = sline.rfind(":");
				compressedfile = sline.substr(pos+2, sline.length());
				svFileList.push_back(compressedfile);
				cnt++;
			}
		}
		fin.close();
	}

	return cnt;
}

INT mtkPack::Cabinet::Expand(const std::string& sWildName, const std::string& sDest) {
	INT ret = 0;
	mCabEngine.SetCmdLine("-R " + mCabFile + " -F:" + sWildName + " " + sDest);
	mCabEngine.Exec(NULL, EEM_NONBLOCKING);
	return ret;
}

INT mtkPack::Cabinet::Compress(const std::string& sWildName, const BOOL bPreservePath /* = TRUE */, const BOOL bRecursive /* = TRUE */) {
	INT ret = 0;
	return ret;
}

void mtkPack::Cabinet::InitEngine(void) {
//	std::string &exefn = File::TrName("$WINDIR\\system32\\cabarc.exe");
//	if (File::Exist(exefn.c_str())) {
		mCabEngine.SetAppName("cabarc.exe");
		mEngineType = ET_CABARC;
	//} else {
	//	mCabEngine.SetAppName(File::TrName("$WINDIR\\system32\\expand.exe"));
	//	mEngineType = ET_EXPAND;
	//}
}