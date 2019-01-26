#include "Synchronization.h"
#include <ctime>

#include "SecureDelete.h"
#include "StrUtils.h"

mtkPack::Mutex::Mutex(void)
	:hMutex(NULL), lpMutexAttributes(NULL), lpName(NULL), dwCount(20)
{
}

mtkPack::Mutex::Mutex(LPCTSTR lpName)
	: hMutex(NULL), lpMutexAttributes(NULL), lpName(lpName), dwCount(20)
{
}

mtkPack::LockObj::LockObj(void)
	:Created(0), pCS(NULL), pMutex(NULL)
{
}

mtkPack::LockObj::~LockObj(void) {
	SecureDelete<CRITICAL_SECTION> sd1(&pCS, 0);
	SecureDelete<Mutex> sd2(&pMutex, 0);
}

mtkPack::Synchronization::Synchronization(void)
	:mType(SYNC_UNKNOWN)
{
}

mtkPack::Synchronization::Synchronization(const SyncType stType)
	:mType(stType)
{
	Init();
}

mtkPack::Synchronization::~Synchronization(void)
{
	Term();
}

void mtkPack::Synchronization::SetType(const SyncType stType) {
	if (mLockObj.Created == 0) {
		mType = stType;
	}
}

void mtkPack::Synchronization::Lock(void) {
	if (mLockObj.Created == 0) {
		Init();
	}

	if (mType == SYNC_CS) {
		EnterCriticalSection(mLockObj.pCS);
	} else if (mType == SYNC_MUTEX) {
		DWORD cnt = 0;
		while (cnt < mLockObj.pMutex->dwCount) {
			DWORD dwWaitResult = WaitForSingleObject(mLockObj.pMutex->hMutex, INFINITE);
			if (dwWaitResult == WAIT_OBJECT_0) {
				break;
			} else if (dwWaitResult == WAIT_FAILED) {
				cnt++;
			}
		}
	}
	return;
}

void mtkPack::Synchronization::Unlock(void) {
	if (mType == SYNC_CS) {
		LeaveCriticalSection(mLockObj.pCS);
	} else if (mType == SYNC_MUTEX) {
		ReleaseMutex(mLockObj.pMutex->hMutex);
	}
	return;
}

void mtkPack::Synchronization::Init(void) {
	srand((unsigned int)time(NULL));

	if (mType == SYNC_CS) {
		mLockObj.pCS = new CRITICAL_SECTION;
		InitializeCriticalSection(mLockObj.pCS);
		mLockObj.Created = 1;
	} else if (mType == SYNC_MUTEX) {
		char szVal[16] = { 0 };
		_itoa_s(rand(), szVal, 15, 10);
		mLockObj.pMutex = new Mutex(szVal);
		mLockObj.pMutex->hMutex = CreateMutex(mLockObj.pMutex->lpMutexAttributes, FALSE, mLockObj.pMutex->lpName);
		mLockObj.Created = 1;
	}
}

void mtkPack::Synchronization::Term(void) {
	if (mLockObj.Created == 1) {
		if (mType == SYNC_CS) {
			DeleteCriticalSection(mLockObj.pCS);
			mLockObj.Created = 0;
		} else if (mType == SYNC_MUTEX) {
			CloseHandle(mLockObj.pMutex->hMutex);
			mLockObj.Created = 0;
		}
	}
}

mtkPack::AutoLock::AutoLock(Synchronization& pSync)
	:mSync(pSync)
{
	mSync.Lock();
}

mtkPack::AutoLock::~AutoLock(void) {
	mSync.Unlock();
}