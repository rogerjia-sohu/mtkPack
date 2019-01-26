#include "Codecs.h"
#include <cmath>
#include <ctime>

#include "SecureDelete.h"

mtkPack::CodecBase::CodecBase()
{
}

mtkPack::CodecBase::CodecBase(const std::string &sData)
	:mData(sData)
{
}

mtkPack::CodecBase::~CodecBase()
{
}

void mtkPack::CodecBase::SetData(const std::string &sData) {
	mData = sData;
}

const std::string mtkPack::Base64::mCodes = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

mtkPack::Base64::Base64(const std::string &sData)
	:CodecBase(sData)
{
}

std::string mtkPack::Base64::Encode() {
	std::string ret;
	int b = 0;
	for (size_t i = 0; i < mData.length(); i += 3)  {
		b = (mData[i] & 0xFC) >> 2;
		ret.append(1, mCodes[b]);

		b = (mData[i] & 0x03) << 4;
		if (i + 1 < mData.length())      {
			b |= (mData[i + 1] & 0xF0) >> 4;
			ret.append(1, mCodes[b]);

			b = (mData[i + 1] & 0x0F) << 2;
			if (i + 2 < mData.length())  {
				b |= (mData[i + 2] & 0xC0) >> 6;
				ret.append(1, mCodes[b]);
				b = mData[i + 2] & 0x3F;
				ret.append(1, mCodes[b]);
			} else {
				ret.append(1,mCodes[b]);
				ret.append("=");
			}
		} else {
			ret.append(1, mCodes[b]);
			ret.append("==");
		}
	}

	return ret;
}

std::string mtkPack::Base64::Decode() {
	std::string ret;
	if (mData.length() % 4 == 0) {
		int cbSize = ((mData.length() * 3) / 4) - (mData.find("=") != std::string::npos ? (mData.length() - mData.find("=")) : 0);
		char *pszDecoded = new(std::nothrow) char[cbSize + 1];
		if (pszDecoded) {
			pszDecoded[cbSize] = 0;

			SecureDelete<char> sd1(&pszDecoded, 1);
			int j = 0;
			int b[4] = { 0 };
			for (size_t i = 0; i < mData.length(); i += 4) {
				// This could be made faster (but more complicated) by precomputing these index locations
				b[0] = mCodes.find((mData[i]));
				b[1] = mCodes.find(mData[i + 1]);
				b[2] = mCodes.find(mData[i + 2]);
				b[3] = mCodes.find(mData[i + 3]);
				pszDecoded[j++] = (char)((b[0] << 2) | (b[1] >> 4));
				if (b[2] < 64)      {
					pszDecoded[j++] = (char)((b[1] << 4) | (b[2] >> 2));
					if (b[3] < 64)  {
						pszDecoded[j++] = (char)((b[2] << 6) | b[3]);
					}
				}
			}// for (int i = 0; i < mData.length(); i += 4) {
			ret = pszDecoded;
		} else {
			mErrCode = 2; // memory allocation failed
		}
	} else {
		mErrCode = 1;// invalid base64 input
	}

	return ret;
}

std::string mtkPack::Base64::Encode(const std::string &sData) {
	Base64 base64(sData);
	return base64.Encode();
}

std::string mtkPack::Base64::Decode(const std::string &sData) {
	Base64 base64(sData);
	return base64.Decode();
}