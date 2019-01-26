#pragma once
#include <string>

namespace mtkPack {
	class CPUInfo {
	public:
		CPUInfo(void);
		~CPUInfo(void);

		long GetFreqMHz(void); // current freq. in MHz
		unsigned int Utilization(void); // CPU utilization in percentage

		const std::string ID;
		const std::string Name;
		const std::string SpecFreq; // frequency on specification
		const unsigned int CoreNum;
	
	protected:
		void CPUID(const unsigned long dwCmd);
		
		std::string GetID(void);
		std::string GetName(void);
		std::string GetSpecFreq(const std::string& sName);
		const unsigned int GetCoreNum(void);

		static unsigned long dwEAX;
		static unsigned long dwEBX;
		static unsigned long dwECX;
		static unsigned long dwEDX;
	};

	class MemoryInfo {
	public:
		MemoryInfo(void);
		~MemoryInfo(void);
	protected:
		MEMORYSTATUSEX statex;
	};

	class OSInfo {
	public:
		OSInfo(void);
		~OSInfo(void);
	};

	class ComputerInfo {
	public:
		ComputerInfo(void);
		~ComputerInfo(void);

		const CPUInfo CPU;
		const OSInfo OS;
	};
}