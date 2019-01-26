#pragma once
#include "Synchronization.h"

namespace mtkPack {

	typedef void(*WatcherCallback)(void* pParam);

	class WatcherInfo {
	public:
		WatcherInfo(unsigned int nInterval, WatcherCallback cbOnWatch, void* lpParam);
		unsigned long Interval;
		WatcherCallback OnWatch;
		void* Param;
	};

	class Watcher {
	public:
		Watcher(void);
		Watcher(const WatcherInfo& Info);
		virtual ~Watcher();
		void Setup(const WatcherInfo& Info);
		void Start(void);
		void Stop(void);

	protected:
		WatcherInfo* mlpWatcherInfo;
		bool mRunning;
		void* mStopWatcherEvent;
		Synchronization mSync;
		static void WatchingThread(void* pParam);
		static void PreOnWatch(void* pParam);
	};

}
