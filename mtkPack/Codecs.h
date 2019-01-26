#pragma once
#include <string>

namespace mtkPack {
	class CodecBase
	{
	public:
		CodecBase();
		CodecBase(const std::string &sData);
		virtual ~CodecBase();

		virtual std::string Encode() = 0;
		virtual std::string Decode() = 0;

		virtual void SetData(const std::string &sData);
	protected:
		int mErrCode;
		std::string mData;
	};

	class Base64 :public CodecBase
	{
	public:
		Base64(const std::string &sData);
		virtual std::string Encode();
		virtual std::string Decode();
		
		static std::string Encode(const std::string &sData);
		static std::string Decode(const std::string &sData);

	protected:
		static const std::string mCodes;
	};
}