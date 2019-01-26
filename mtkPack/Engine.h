#pragma once
#include <vector>

#include "Process.h"

namespace mtkPack {

	enum EngineExecMode {
		EEM_BLOCKING = 0,
		EEM_NONBLOCKING
	};

	class Engine {
	public:
		Engine(void);
		Engine(const char* pAppName, const char* pCmdLine);
		virtual ~Engine(void);

		INT SetAppCmdLine(const std::string& sAppName, const std::string& sCmdLine);

		INT SetAppName(const std::string& sAppName);
		void SetCmdLine(const std::string& sCmdLine);

		INT Exec(LPSTR lpAdditionalParams = NULL, EngineExecMode mode = EEM_BLOCKING);
		BOOL IsRunning(void) const { return mProcess.IsRunning(); }

	protected:
		std::string mAppName;
		std::string mCmdLine;
		Process mProcess;
	};

}