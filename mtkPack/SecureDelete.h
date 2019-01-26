#pragma once

namespace mtkPack {

	template<class T>
	class SecureDelete {
	public:
		SecureDelete(void)
			:mPtrObjPtr(NULL), mArrayFlg(0)
		{	
		}

		SecureDelete(T** pObjPtr, const int bArrayFlg)
			:mPtrObjPtr(pObjPtr), mArrayFlg(bArrayFlg)
		{
		}

		virtual ~SecureDelete(void) {
			// auto-deleting when destructing
			FreeAndNil();
		}

		void Bind(T** pObjPtr, const int bArrayFlg) {
			mPtrObjPtr = pObjPtr;
			mArrayFlg = bArrayFlg;
		}

	protected:

		T** mPtrObjPtr;		// Pointer to an object's pointer
		int mArrayFlg;	// Using "delete []" or "delete"


		// Delete an object securely
		void FreeAndNil(void) {
			if (mPtrObjPtr == NULL) {
				// empty pointer
				return;
			}

			T* pObj = (T*)(*mPtrObjPtr);
			if (pObj == NULL) {
				// object already deleted by a previous instance
				return;
			}

			if (mArrayFlg == 0) {
				delete pObj;
			} else {
				delete[] pObj;
			}

			*mPtrObjPtr = NULL;
		}
	};

}