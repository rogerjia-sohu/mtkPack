#include <Windows.h>

#include "ComputerInfo.h"

unsigned long mtkPack::CPUInfo::dwEAX = 0;
unsigned long mtkPack::CPUInfo::dwEBX = 0;
unsigned long mtkPack::CPUInfo::dwECX = 0;
unsigned long mtkPack::CPUInfo::dwEDX = 0;

mtkPack::CPUInfo::CPUInfo(void)
	:ID(GetID()), Name(GetName()), SpecFreq(GetSpecFreq(Name)), CoreNum(GetCoreNum())
{

}


mtkPack::CPUInfo::~CPUInfo(void)
{
}

long mtkPack::CPUInfo::GetFreqMHz(void)
{
	int start1,start2;
	_asm rdtsc
	_asm mov start1,eax
	Sleep(50);
	_asm rdtsc
	_asm mov start2,eax
	return ((start2-start1)/50)/(1000);
}

unsigned int mtkPack::CPUInfo::Utilization(void) {
	unsigned int ratio = 0;

	return ratio;
}

void mtkPack::CPUInfo::CPUID(const unsigned long dwCmd) {
	__asm
	{
		mov eax,dwCmd
		cpuid
		mov dwEAX,eax
		mov dwEBX,ebx
		mov dwECX,ecx
		mov dwEDX,edx
	}
}


std::string mtkPack::CPUInfo::GetSpecFreq(const std::string& sName) {
	int pos = sName.find_last_of("@");
	while (sName[++pos] == ' ') {
		;// nop
	}
	return sName.substr(pos, sName.length()-pos);
}

const unsigned int  mtkPack::CPUInfo::GetCoreNum(void) {
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}

std::string mtkPack::CPUInfo::GetID(void) {
	char ID[13] = {0};
	CPUID(0);
	memcpy(ID+0, &dwEBX, 4);
	memcpy(ID+4, &dwEDX, 4);
	memcpy(ID+8, &dwECX, 4);
	return std::string(ID);
}

std::string mtkPack::CPUInfo::GetName(void)
{
	const DWORD id = 0x80000002;
	char Name[49]= {0};
	for (int i = 0; i < 3; i++) {
		CPUID(id + i);
		memcpy(Name + i*16, &dwEAX, 16); 
	}
	int pos = -1;
	while (Name[++pos] == ' ') {
		;// nop
	}
	return std::string(&Name[pos]);
}


mtkPack::OSInfo::OSInfo(void)
{
}


mtkPack::OSInfo::~OSInfo(void)
{
}

mtkPack::ComputerInfo::ComputerInfo(void){

}

mtkPack::ComputerInfo::~ComputerInfo(void){

}