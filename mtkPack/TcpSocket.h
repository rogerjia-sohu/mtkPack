#pragma once
#include <ctime>
#include <string>
#include <map>
#include <set>
#include <list>
#include <WinSock2.h>
#include <Ws2tcpip.h>

#include "SecureDelete.h"
#include "Synchronization.h"

namespace mtkPack {
	const int RECV_BUFFER_LEN = 512;
	const int SELECT_SYNC_MSEC = 20; // no less than 20
	const size_t MIN_TCPCLIENTS = 10;
	const size_t MAX_TCPCLIENTS = 10000;

	enum IPDirection {
		CLIENT_IP = 0,
		SERVER_IP
	};

	class TcpServer;
	class TcpClient;

	typedef int(*RecvCallback)(const TcpClient *pClient);

	class TcpSocket
	{
	public:
		TcpSocket(void);
		virtual ~TcpSocket(void);

		virtual int Start(void) = 0;
		virtual int Stop(void) = 0;
		virtual const std::string GetIPv4(void) = 0;

		int IsRunning(void) const;
		HANDLE GetSafeStopSocketEvent(void);
		WSAEVENT GetSafeSelectSockEvent(void);
		time_t GetSafeCreationTime(void);
		int ErrCode(void);

		static DWORD IPv4StrToDWord(const std::string &sIP);
		static std::string IPv4DWordToStr(const DWORD &dwIP);
		static std::string GetIPv4(const SOCKET &socket, IPDirection ipDir);
		static const bool IsValidMask(const std::string& sMask);
		static const bool IsValidMask(const DWORD& dwMask);

	protected:
		virtual int Init(void) = 0;
		int Clear(void);

		std::string mHost;
		int mPort;
		WSADATA mWSAData;
		SOCKET mWinSock;
		int mIsCreated;
		int mRunning;
		int mErrCode;
		time_t mCreationTime;
		HANDLE mStopSocketEvent;
		WSAEVENT mSelectSockEvent;
		RecvCallback mRecvCallback;
	};

	typedef std::list<TcpClient *> TcpClientPtrList;
	typedef std::set<std::string> IPv4List;
	typedef std::map<std::string, std::map<DWORD, DWORD>> IPv4MaskMap;
	typedef std::set<std::string> MesssageRuleList;

	enum IPLIST_PRIORITY {
		IP_WHITE_PRIORITY = 0,
		IP_BLACK_PRIORITY
	};

	enum IP_TYPE {
		WHITE_IP = 0,
		BLACK_IP
	};

	class TcpServerParam {
	public:
		TcpServerParam::TcpServerParam(void);
		TcpServerParam::TcpServerParam(int interval, int maxclnt, unsigned long timeoutsecond, struct addrinfo *pAddrInfo);
		TcpServerParam::~TcpServerParam(void);
		void Clear(void);
		void TryRemoveDeadClients(size_t iCnt);

		int Interval;
		size_t MaxClients;
		unsigned long ClientTimeoutSecond;
		SOCKET AcceptedSocket;
		struct addrinfo *AddrInfo;
		struct sockaddr_in SAddrIn;
		HANDLE ServingThreadHandle;
		DWORD ServingThreadID;
		TcpClientPtrList ClientPtrList;
		Synchronization Sync;
		
		IPLIST_PRIORITY IpListPriority;
		MesssageRuleList MsgRuleList;
		IPv4List WhiteIPList;
		IPv4List BlackIPList;
		IPv4MaskMap WhiteIPMap;
		IPv4MaskMap BlackIpMap;
	};

	typedef int(*AcceptCallback)(const TcpServer &Server, const SOCKET &ClientSock);
	typedef void(*BanCallback)(const std::string &sIPv4);

	typedef std::map<std::string, void *> ClientIpList;

	class TcpServer :public TcpSocket {
	public:
		TcpServer::TcpServer(void);

		TcpServer::TcpServer(const std::string &sHost, const int iPort, AcceptCallback pAcceptCallback, RecvCallback pRecvCallback, BanCallback pBanCallback);
		virtual TcpServer::~TcpServer(void);

		int Setup(const std::string &sHost, const int iPort, AcceptCallback pAcceptCallback, RecvCallback pRecvCallback, BanCallback pBanCallback);

		virtual int Start(void);
		virtual int Stop(void);
		virtual const std::string GetIPv4(void);

		const TcpServerParam & GetParam(void) { return mServerParam; }
		void SetMaxClients(size_t nMaxClients);
		void SetClientTimeoutSecond(unsigned long nSecond);

		int AddToIPList(IP_TYPE type, const std::string& sIPv4);
		int DeleteFromIPList(IP_TYPE type, const std::string& sIPv4);

		int AddToIPMap(IP_TYPE type, const std::string& sIPv4, const std::string& sMask);
		int DeleteFromIPMap(IP_TYPE type, const std::string& sIPv4, const std::string& sMask);
		const bool IsIPBanned(const std::string& sIPv4);

		int AddToMsgRuleList(const std::string& sMsg);
		const bool CheckMsgByRules(const std::string& sMsg);
		
		SOCKET &mServerSocket;

	protected:
		virtual int Init(void);
		static bool IsClientStopped(const TcpClient *pClient);
		static DWORD WINAPI ServingThread(LPVOID pParam);
		static void NTAPI ServingThreadCallback(PTP_CALLBACK_INSTANCE instance, void *context, PTP_WORK work);
		TcpServerParam mServerParam;
		AcceptCallback mAcceptCallback;
		BanCallback mBanCallback;
	};

	class TcpClientParam {
	public:
		TcpClientParam::TcpClientParam(void);
		TcpClientParam::~TcpClientParam(void);
	
		struct addrinfo *AddrInfo;
		HANDLE ClientThreadHandle;
		DWORD ClientThreadID;
	};

	enum ClientChkMask {
		CHK_STOPEVENT = 0x0001,
		CHK_SELECTEVENT = 0x0002,
		CHK_SOCKET = 0x0004,
		CHK_DATA = 0x0008,
		CHK_ALL = CHK_STOPEVENT | CHK_SELECTEVENT | CHK_SOCKET | CHK_DATA
	};
	class TcpClient :public TcpSocket {
	public:
		TcpClient::TcpClient(void);
		TcpClient::TcpClient(const SOCKET &Socket, RecvCallback pRecvCallback);
		TcpClient::TcpClient(const std::string &sRemoteHost, const int iRemotePort, RecvCallback pRecvCallback);
		virtual TcpClient::~TcpClient(void);

		void Setup(const SOCKET &Socket, RecvCallback pRecvCallback);
		void Setup(const std::string &sRemoteHost, const int iRemotePort, RecvCallback pRecvCallback);

		bool HasReceivedData(void) const;
		const char* GetData(void);

		virtual int Start(void);
		virtual int Stop(void);
		virtual const std::string GetIPv4(void);
		virtual int Send(const char *pData);

		const TcpClientParam & GetParam(void) { return mClientParam; }
		void SetClosingKeyword(const std::string &sKeyword);

		SOCKET GetSafeClientSocket(void);
		RecvCallback GetSafeRecvCallback(void);
		bool IsValid(const ClientChkMask& chkmask);

	protected:
		virtual int Init(void);

		static DWORD WINAPI ClientThread(LPVOID pParam);
		SOCKET &mClientSocket;
		std::string &mRemoteHost;
		int &mRemotePort;
		TcpClientParam mClientParam;
		std::string mData;
		std::string mClosingKeyword;
	};
}