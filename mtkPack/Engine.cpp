#include <iostream>

#include "Engine.h"
#include "File.h"
#include "SecureDelete.h"

mtkPack::Engine::Engine(void)
{
}

mtkPack::Engine::Engine(const char* pAppName, const char* pCmdLine)
	:mAppName(pAppName)
{
	mCmdLine.append(" ").append(pCmdLine);
}

mtkPack::Engine::~Engine(void)
{
}

INT mtkPack::Engine::SetAppCmdLine(const std::string& sAppName, const std::string& sCmdLine) {
	SetCmdLine(sCmdLine);
	return SetAppName(sAppName);
}

INT mtkPack::Engine::SetAppName(const std::string& sAppName) {
	INT ret = 0;
	//if (File::Exist(sAppName.c_str())) {
		mAppName = sAppName;
		ret = 1;
	//}
	return ret;
}

void mtkPack::Engine::SetCmdLine(const std::string& sCmdLine) {
	if (mAppName.length()) {
		mCmdLine.clear();
		mCmdLine.append(" ").append(sCmdLine);
	} else {
		mCmdLine = sCmdLine;
	}
}

INT mtkPack::Engine::Exec(LPSTR lpAdditionalParams /* = NULL */, EngineExecMode mode /* = EEM_BLOCKING */) {
	INT ret = 0;
	if (mAppName.length() == 0 && mCmdLine.length() == 0) {
		return ret;
	}
	if (mode == EEM_BLOCKING) {
		std::string sFullCmd(mAppName);
		sFullCmd.append(" ").append(mCmdLine);
		if (lpAdditionalParams) sFullCmd.append(lpAdditionalParams);
		ret = Process::StartAndWait(sFullCmd.c_str());
	} else {
		std::string sFullCmd(mCmdLine);
		if (lpAdditionalParams) sFullCmd.append(lpAdditionalParams);
		if (!mProcess.IsRunning()) {
			mProcess.Prepare(mAppName.c_str(), sFullCmd.c_str());
			ret = mProcess.Run();
		}
	}
	return ret;
}