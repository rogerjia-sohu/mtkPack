#include <cstdlib>

#include "Process.h"
#include "SecureDelete.h"
#include "StrUtils.h"

mtkPack::Process::Process(void)
	:mExitCode(0), mIsCreated(FALSE), mStartupInfoA(NULL), mStartupInfoW(NULL)
{
}

mtkPack::Process::Process(const CHAR* pAppNameA, const CHAR* pCmdLineA)
	:mExitCode(0), mIsCreated(FALSE), mStartupInfoA(NULL), mStartupInfoW(NULL)
{
	Prepare(pAppNameA, pCmdLineA);
}

mtkPack::Process::Process(const WCHAR* pAppNameW, const WCHAR* pCmdLineW)
	:mExitCode(0), mIsCreated(FALSE), mStartupInfoA(NULL), mStartupInfoW(NULL)
{
	Prepare(pAppNameW, pCmdLineW);
}

mtkPack::Process::~Process(void) {
	CloseProcessHandles();
	SecureDelete<STARTUPINFOA> delInfoA(&mStartupInfoA, 0);
	SecureDelete<STARTUPINFOW> delInfoW(&mStartupInfoW, 0);
}

BOOL mtkPack::Process::Prepare(const CHAR* pAppNameA, const CHAR* pCmdLineA) {
	if (mStartupInfoA) {
		SecureDelete<STARTUPINFOA> sd(&mStartupInfoA, 0);
		CloseProcessHandles();
	}
	mStartupInfoA = new STARTUPINFOA;
	ZeroMemory(mStartupInfoA, sizeof(*mStartupInfoA));
	mStartupInfoA->cb = sizeof(*mStartupInfoA);
	ZeroMemory(&mProcesInfo, sizeof(mProcesInfo));

	// create but suspend the child process. 
	const CHAR* lpApp = (pAppNameA && strlen(pAppNameA))? pAppNameA : NULL;
	mIsCreated = CreateProcessA(lpApp, const_cast<CHAR*>(pCmdLineA), NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, mStartupInfoA, &mProcesInfo);
	return mIsCreated;
}

BOOL mtkPack::Process::Prepare(const WCHAR* pAppNameW, const WCHAR* pCmdLineW) {
	if (mStartupInfoW) {
		SecureDelete<STARTUPINFOW> sd(&mStartupInfoW, 0);
		CloseProcessHandles();
	}
	mStartupInfoW = new STARTUPINFOW;
	ZeroMemory(mStartupInfoW, sizeof(*mStartupInfoW));
	mStartupInfoW->cb = sizeof(*mStartupInfoW);
	ZeroMemory(&mProcesInfo, sizeof(mProcesInfo));

	// create but suspend the child process. 
	const WCHAR* lpApp = (pAppNameW && wcslen(pAppNameW))? pAppNameW : NULL;
	mIsCreated = CreateProcessW(pAppNameW, const_cast<WCHAR*>(pCmdLineW), NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, mStartupInfoW, &mProcesInfo);
	return mIsCreated;
}

DWORD mtkPack::Process::Run(void) {
	DWORD ret = ResumeThread(mProcesInfo.hThread);
	if (ret == (DWORD)(-1)) {
		ret = GetLastError();
	}
	return ret;
}

BOOL mtkPack::Process::IsRunning(void) const {
	if (GetExitCodeProcess(mProcesInfo.hProcess, const_cast<DWORD *>(&mExitCode))
		&& mExitCode == STILL_ACTIVE ) {
			return TRUE;
	}
	return FALSE;
}

BOOL mtkPack::Process::IsCreated(void) const {
	return mIsCreated;
}

DWORD mtkPack::Process::GetExitCode(void) const {
	return mExitCode;
}

DWORD mtkPack::Process::GetPID(void) const {
	return mProcesInfo.dwProcessId;
}

INT mtkPack::Process::GetPriority(void) const {
	return GetThreadPriority(mProcesInfo.hThread);
}

BOOL mtkPack::Process::SetPriority(INT nPriority) {
	return SetThreadPriority(mProcesInfo.hThread, nPriority);
}

INT mtkPack::Process::StartAndWait(const CHAR* pCmdLineA) {
	return system(pCmdLineA);
}

INT mtkPack::Process::StartAndWait(const WCHAR* pCmdLineW) {
	CHAR* cmdline = StrUtils::ToChar(pCmdLineW);
	SecureDelete<char> sd(&cmdline, 1);
	return system(cmdline);
}

BOOL mtkPack::Process::CloseProcessHandles(void) {
	BOOL ret = FALSE;
	if (mIsCreated && !IsRunning()) {
		if (CloseHandle(mProcesInfo.hProcess)) {
			ret = CloseHandle(mProcesInfo.hThread);
		}
	}
	return ret;
}