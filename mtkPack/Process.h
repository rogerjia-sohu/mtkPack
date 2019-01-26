#pragma once
#include <Windows.h>

namespace mtkPack {

	class Process {
	public:
		Process(void);
		Process(const CHAR* pAppNameA, const CHAR* pCmdLineA);
		Process(const WCHAR* pAppNameW, const WCHAR* pCmdLineW);
		virtual ~Process(void);

		BOOL Prepare(const CHAR* pAppNameA, const CHAR* pCmdLineA);
		BOOL Prepare(const WCHAR* pAppNameW, const WCHAR* pCmdLineW);

		DWORD Run(void);
		BOOL IsRunning(void) const;

		BOOL IsCreated(void) const;
		DWORD GetExitCode(void) const;
		DWORD GetPID(void) const;
		INT GetPriority(void) const;
		BOOL SetPriority(INT nPriority);

		static INT StartAndWait(const CHAR* pCmdLineA);
		static INT StartAndWait(const WCHAR* pCmdLineW);

	protected:
		BOOL CloseProcessHandles(void);
		BOOL mIsCreated;
		DWORD mExitCode;
		STARTUPINFOA* mStartupInfoA;
		STARTUPINFOW* mStartupInfoW;
		PROCESS_INFORMATION mProcesInfo;
	};

}