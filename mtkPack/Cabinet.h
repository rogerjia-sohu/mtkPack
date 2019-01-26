#pragma once
#include <iostream>
#include <vector>

#include "Engine.h"

namespace mtkPack {

	class Cabinet
	{
	public:
		Cabinet(void);
		Cabinet(const std::string& sCabFile);
		virtual ~Cabinet(void);

		void SetFileName(const std::string& sCabFile);
		INT GetFileList(std::vector<std::string>& svFileList);
		INT Expand(const std::string& sWildName, const std::string& sDest);
		INT Compress(const std::string& sWildName, const BOOL bPreservePath = TRUE, const BOOL bRecursive = TRUE);

	protected:
		void InitEngine(void);

		enum EngineType {
			ET_CABARC = 0,
			ET_EXPAND
		};
		EngineType mEngineType;
		Engine mCabEngine;
		std::string mCabFile;
	};
}