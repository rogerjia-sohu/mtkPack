#include "Watcher.h"
#include <process.h>
#include <Windows.h>

#include "SecureDelete.h"


mtkPack::WatcherInfo::WatcherInfo(unsigned int nInterval, WatcherCallback cbOnWatch, void* lpParam)
	:Interval(nInterval), OnWatch(cbOnWatch), Param(lpParam)
{
}

mtkPack::Watcher::Watcher()
	:mlpWatcherInfo(nullptr), mRunning(false), mStopWatcherEvent(CreateEvent(NULL, false, false, NULL))
{
	mSync.SetType(SYNC_MUTEX);
}

mtkPack::Watcher::Watcher(const WatcherInfo& Info)
	:mlpWatcherInfo(nullptr), mRunning(false), mStopWatcherEvent(CreateEvent(NULL, false, false, NULL))
{
	mSync.SetType(SYNC_MUTEX);
	Setup(Info);
}

mtkPack::Watcher::~Watcher() {
	SecureDelete<WatcherInfo> sd1(&mlpWatcherInfo, 0);
	if (mStopWatcherEvent && (long)mStopWatcherEvent != ERROR_ALREADY_EXISTS) {
		CloseHandle(mStopWatcherEvent);
		mStopWatcherEvent = NULL;
	}
}

void mtkPack::Watcher::Setup(const WatcherInfo& Info) {
	if (mlpWatcherInfo) {
		SecureDelete<WatcherInfo> sd1(&mlpWatcherInfo, 0);
	}
	mlpWatcherInfo = new WatcherInfo(Info);
}

void mtkPack::Watcher::Start(void) {
	if (mlpWatcherInfo && mRunning == false) {
		_beginthread(WatchingThread, 0, this);
		mRunning = true;
	}
}

void mtkPack::Watcher::Stop(void) {
	if (mRunning) {
		SetEvent(mStopWatcherEvent);
		mRunning = false;
	}
}

void mtkPack::Watcher::WatchingThread(void* pParam) {
	if (!pParam) {
		return;
	}
	Watcher* pWatcher = (Watcher*)pParam;
	DWORD dwWaitRet = 0;
	for (;;) {
		_beginthread(pWatcher->PreOnWatch, 0, pParam);
		dwWaitRet = WaitForSingleObject(pWatcher->mStopWatcherEvent, pWatcher->mlpWatcherInfo->Interval);
		if (dwWaitRet == WAIT_OBJECT_0) {
			break;
		}
	}
	return;
}

void mtkPack::Watcher::PreOnWatch(void* pParam) {
	if (!pParam) {
		return;
	}
	Watcher* pWatcher = (Watcher*)pParam;
	AutoLock AL(pWatcher->mSync);
	pWatcher->mlpWatcherInfo->OnWatch(pWatcher->mlpWatcherInfo->Param);
}