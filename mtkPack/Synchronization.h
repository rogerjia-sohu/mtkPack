#pragma once
#include <Windows.h>

namespace mtkPack {

	enum SyncType{
		SYNC_UNKNOWN,
		SYNC_CS,
		SYNC_MUTEX
	};

	class Mutex{
	public:
		Mutex(void);
		Mutex(LPCTSTR lpName);

		HANDLE hMutex;
		LPSECURITY_ATTRIBUTES lpMutexAttributes;
		LPCTSTR lpName;
		DWORD dwCount;
	};

	class LockObj {
	public:
		LockObj(void);
		virtual ~LockObj(void);

		int Created;
		CRITICAL_SECTION* pCS;
		Mutex* pMutex;	
	};

	class Synchronization {
	public:
		Synchronization(void);
		Synchronization(const SyncType stType);
		virtual ~Synchronization(void);

		void SetType(const SyncType stType);

		virtual void Lock(void);
		virtual void Unlock(void);

	protected:
		virtual void Init(void);
		virtual void Term(void);

		SyncType mType;
		LockObj mLockObj;
	};

	class AutoLock {
	public:
		AutoLock(Synchronization& pSync);
		virtual ~AutoLock(void);

	protected:
		Synchronization& mSync;
	};

}