#pragma once
#include <list>

#include "SecureDelete.h"

namespace mtkPack {

	template<class T>
	class MRUList {
	public:
		MRUList()
			:mMaxSize(10)
		{
		}
		
		MRUList(const size_t nSize)
			:mMaxSize(nSize)
		{
		}

		virtual ~MRUList() { Clear(); }

		void Add(const T &Val) {
			if (!mDataList.empty()) {
				if (mDataList.front() == Val) {
					return;
				}
				mDataList.remove(Val);
				if (mDataList.size() == mMaxSize) {
					mDataList.pop_back();
				}
			}
			mDataList.push_front(Val);
		}

		void Resize(const size_t &nSize) {
			mMaxSize = nSize;
			if (mDataList.size() > mMaxSize) {
				mDataList.resize(mMaxSize);
			}
		}

		void Remove(const T &Val) {
			if (!mDataList.empty()) {
				mDataList.remove(Val);
			}
		}

		void Clear() { mDataList.clear(); }

		const std::list<T> &DataList() { return mDataList; }
		const size_t MaxSize() { return mMaxSize; }
		const size_t Size() { return mDataList.size(); }
		const T &First() { return mDataList.front(); }
		const T &Last() { return mDataList.back(); }

	protected:
		size_t mMaxSize;
		std::list<T> mDataList;
	};

}