#include "TcpSocket.h"
#include <Process.h>
#include <sstream>

#include "StrUtils.h"

#include <iostream>

#pragma comment(lib, "ws2_32.lib")

mtkPack::TcpSocket::TcpSocket(void)
	:mHost(""), mPort(0), mIsCreated(0), mRunning(0), mErrCode(0), mCreationTime(time(NULL))
	,mStopSocketEvent(CreateEvent(NULL, false, false, NULL))
	,mSelectSockEvent(WSACreateEvent())
	,mRecvCallback(NULL)
{
	WORD ver = MAKEWORD(2,2);
	mErrCode = WSAStartup(ver, &mWSAData);
}

mtkPack::TcpSocket::~TcpSocket(void)
{
	mRecvCallback = NULL;
	if (mIsCreated) {
		try {
			closesocket(mWinSock);
		} catch (...) {

		}
		mWinSock = INVALID_SOCKET;
		mIsCreated = 0;
	}
	if (mStopSocketEvent && (long)mStopSocketEvent != ERROR_ALREADY_EXISTS) {
		try {
			CloseHandle(mStopSocketEvent);
		} catch (...) {

		}
		mStopSocketEvent = NULL;
	}
	if (mSelectSockEvent != WSA_INVALID_EVENT) {
		try {
			WSACloseEvent(mSelectSockEvent);
		} catch (...) {

		}
		mSelectSockEvent = WSA_INVALID_EVENT;
	}
	WSACleanup();
}

int mtkPack::TcpSocket::Clear(void) {
	int ret = 0;
	if (IsRunning()) {
		try {
			if (mWinSock != INVALID_SOCKET) {
				mErrCode = closesocket(mWinSock);
			} else {
				mErrCode = 0;
			}
		} catch (...) {

		}
		try {
			mRunning = 0;
			mIsCreated = 0;
			mWinSock = INVALID_SOCKET;
		} catch (...) {

		}
		if (GetSafeStopSocketEvent() != INVALID_HANDLE_VALUE) {
			ret = SetEvent(mStopSocketEvent);
		}
	}
	return ret;
}

int mtkPack::TcpSocket::IsRunning(void) const {
	try {
		return (mRunning == 1) ? 1 : 0;
	}
	catch (...) {
		return 0;
	}
}

HANDLE mtkPack::TcpSocket::GetSafeStopSocketEvent(void) {
	try {
		return mStopSocketEvent;
	}
	catch (...) {
		return INVALID_HANDLE_VALUE;
	}
}

WSAEVENT mtkPack::TcpSocket::GetSafeSelectSockEvent(void) {
	try {
		return mSelectSockEvent;
	}
	catch (...) {
		return INVALID_HANDLE_VALUE;
	}
}

time_t mtkPack::TcpSocket::GetSafeCreationTime(void) {
	try {
		return mCreationTime;
	}
	catch (...) {
		return 0;
	}
}

int mtkPack::TcpSocket::ErrCode(void) {
	return mErrCode;
}

DWORD mtkPack::TcpSocket::IPv4StrToDWord(const std::string &sIP) {
	DWORD dwIP = 0;
	std::vector<std::string> ipv4;
	if (StrUtils::SplitToken(sIP, ".", ipv4) == 4) {
		BYTE d[4];
		d[0] = atoi(ipv4[0].c_str());
		d[1] = atoi(ipv4[1].c_str());
		d[2] = atoi(ipv4[2].c_str());
		d[3] = atoi(ipv4[3].c_str());
		dwIP = d[0] << 24 | d[1] << 16 | d[2] << 8 | d[3];
	}
	return dwIP;
}

std::string mtkPack::TcpSocket::IPv4DWordToStr(const DWORD &dwIP) {
	std::string ret;

	DWORD d[4];
	d[0] = (dwIP & 0xFF000000) >> 24;
	d[1] = (dwIP & 0x00FF0000) >> 16;
	d[2] = (dwIP & 0x0000FF00) >> 8;
	d[3] = dwIP & 0x000000FF;

	char buffer[9] = {0};
	_itoa_s(d[0], buffer, 8, 10);
	ret.append(buffer).append(".");

	_itoa_s(d[1], buffer, 8, 10);
	ret.append(buffer).append(".");

	_itoa_s(d[2], buffer, 8, 10);
	ret.append(buffer).append(".");

	_itoa_s(d[3], buffer, 8, 10);
	ret.append(buffer);

	return ret;
}

std::string mtkPack::TcpSocket::GetIPv4(const SOCKET &socket, IPDirection ipDir) {
	struct sockaddr name;
	name.sa_family = AF_INET;
	int namelen = sizeof(name);
	if (ipDir == CLIENT_IP) {
		getpeername(socket, &name, &namelen);
	} else {
		getsockname(socket, &name, &namelen);
	}
	char szIPv4[16] = { 0 };
	inet_ntop(name.sa_family, &(((struct sockaddr_in *)(&name))->sin_addr), szIPv4, sizeof(szIPv4));

	return szIPv4;
}

const bool mtkPack::TcpSocket::IsValidMask(const std::string& sMask) {
	bool ret = false;
	std::vector<std::string> mask;
	if (StrUtils::SplitToken(sMask, ".", mask) == 4) {
		BYTE m[4];
		m[0] = atoi(mask[0].c_str());
		m[1] = atoi(mask[1].c_str());
		m[2] = atoi(mask[2].c_str());
		m[3] = atoi(mask[3].c_str());

		DWORD dwMask = m[0] << 24 | m[1] << 16 | m[2] << 8 | m[3];
		ret = TcpSocket::IsValidMask(dwMask);
	}
	
	return ret;
}

const bool mtkPack::TcpSocket::IsValidMask(const DWORD& dwMask) {
	bool ret = false;
	DWORD nmask = ~dwMask;

	//double n = log2(nmask + 1);
	double n = log(nmask + 1.0) / log(2.0);
	ret = ((n - (int)n) < 0.00001 && n <= 31);
	return ret;
}
//////////////////////////////////////////////////////////////////////
mtkPack::TcpServerParam::TcpServerParam(void)
	:Interval(10)
	,MaxClients(MIN_TCPCLIENTS)
	,ClientTimeoutSecond(0)
	,AcceptedSocket(INVALID_SOCKET)
	,AddrInfo(NULL)
	,ServingThreadHandle(NULL)
	,ServingThreadID(0)
	,IpListPriority(IP_BLACK_PRIORITY)
{
	Sync.SetType(SYNC_MUTEX);
}

mtkPack::TcpServerParam::TcpServerParam(int interval, int maxclnt, unsigned long timeoutsecond, struct addrinfo *pAddrInfo)
	:Interval(interval)
	,MaxClients((maxclnt > MAX_TCPCLIENTS)? MAX_TCPCLIENTS : (maxclnt < MIN_TCPCLIENTS)? MIN_TCPCLIENTS: maxclnt)
	,ClientTimeoutSecond(timeoutsecond)
	,AcceptedSocket(INVALID_SOCKET)
	,AddrInfo(pAddrInfo)
	,ServingThreadHandle(NULL)
	,ServingThreadID(0)
	,IpListPriority(IP_BLACK_PRIORITY)
{
	Sync.SetType(SYNC_MUTEX);
}

mtkPack::TcpServerParam::~TcpServerParam(void) {
	Clear();
}

void mtkPack::TcpServerParam::Clear(void) {
	/*
	if (AcceptedSocket != INVALID_SOCKET) {
		closesocket(AcceptedSocket);
		AcceptedSocket = INVALID_SOCKET;
	}*/
	/*
	if (AddrInfo) {
		freeaddrinfo(AddrInfo);
		AddrInfo = NULL;
	}*/
	/*
	for (size_t i = 0; i < ClientPtrList.size(); i++) {
		TcpClient *pClient = ClientPtrList[i];
		pClient->Stop();
		SecureDelete<TcpClient> sdClient(&pClient, 0);
	}*/
}

void mtkPack::TcpServerParam::TryRemoveDeadClients(size_t iCnt) {
	AutoLock al(Sync);
	if (ClientPtrList.size() < iCnt) {
		return;
	}
	size_t cnt = 0;
	while (cnt < iCnt) {
		for (TcpClientPtrList::iterator citr = ClientPtrList.begin(); citr != ClientPtrList.end(); citr++) {
			if (cnt++ < iCnt) {
				TcpClient *pClient = *citr;
				if (!pClient->IsRunning() || !pClient->IsValid(CHK_ALL)) {
					ClientPtrList.remove(pClient);
					SecureDelete<TcpClient> sd1(&pClient, 0);
					break;
				}
				if (ClientTimeoutSecond > 0) {
					time_t now = time(NULL);
					if ( now - pClient->GetSafeCreationTime() >= ClientTimeoutSecond ) {
						ClientPtrList.remove(pClient);
						SecureDelete<TcpClient> sd1(&pClient, 0);
						break;
					}
				}
			} else {
				break;
			}
		}// for
	}// while (cnt < iCnt)
}
//////////////////////////////////////////////////////////////////////

mtkPack::TcpServer::TcpServer(void)
	:mServerSocket(mWinSock), mAcceptCallback(NULL), mBanCallback(NULL)
{
}

mtkPack::TcpServer::TcpServer(const std::string &sHost, const int iPort, AcceptCallback pAcceptCallback, RecvCallback pRecvCallback, BanCallback pBanCallback)
	:mServerSocket(mWinSock)
{
	mHost = sHost;
	mPort = iPort;
	mRecvCallback = pRecvCallback;
	mAcceptCallback = pAcceptCallback;
	mBanCallback = pBanCallback;
	Init();
}

mtkPack::TcpServer::~TcpServer(void) {

}

int mtkPack::TcpServer::Setup(const std::string &sHost, const int iPort, AcceptCallback pAcceptCallback, RecvCallback pRecvCallback, BanCallback pBanCallback)
{
	mHost = sHost;
	mPort = iPort;
	mRecvCallback = pRecvCallback;
	mAcceptCallback = pAcceptCallback;
	mBanCallback = pBanCallback;
	return Init();
}

int mtkPack::TcpServer::Start(void) {
	int ret = 1;
	if (!mIsCreated) {
		Init();
	}

	if (listen(mServerSocket, SOMAXCONN ) == SOCKET_ERROR){
		mErrCode = WSAGetLastError();
		mIsCreated = 0;
		ret = 0;
	} else {
		ret = ResumeThread(mServerParam.ServingThreadHandle);
		if (ret == (DWORD) -1) {

		} else {
			mRunning = 1;
		}
	}

	return ret;
}

int mtkPack::TcpServer::Stop(void) {
	return Clear();
}

const std::string mtkPack::TcpServer::GetIPv4(void) {
	return TcpSocket::GetIPv4(mServerSocket, SERVER_IP);
}

void mtkPack::TcpServer::SetMaxClients(size_t nMaxClients) {
	if (nMaxClients < MIN_TCPCLIENTS) {
		mServerParam.MaxClients = MIN_TCPCLIENTS;
	} else if (nMaxClients > MAX_TCPCLIENTS) {
		mServerParam.MaxClients = MAX_TCPCLIENTS;
	} else {
		mServerParam.MaxClients = nMaxClients;
	}
}

void mtkPack::TcpServer::SetClientTimeoutSecond(unsigned long nSecond) {
	mServerParam.ClientTimeoutSecond = nSecond;
}

int mtkPack::TcpServer::AddToIPList(IP_TYPE type, const std::string& sIPv4) {
	int ret = 0;
	IPv4List* pList = &mServerParam.WhiteIPList;
	if (pList[type].find(sIPv4) == pList[type].end()) {
		pList[type].insert(sIPv4);
		ret = 1;
	}
	return ret;
}

int mtkPack::TcpServer::DeleteFromIPList(IP_TYPE type, const std::string& sIPv4) {
	int ret = 0;
	IPv4List* pList = &mServerParam.WhiteIPList;
	if (pList[type].find(sIPv4) != pList[type].end()) {
		pList[type].erase(sIPv4);
		ret = 1;
	}
	return ret;
}


int mtkPack::TcpServer::AddToIPMap(IP_TYPE type, const std::string& sIPv4, const std::string& sMask) {
	int ret = 0;
	IPv4MaskMap* pMap = &mServerParam.WhiteIPMap;
	//typedef std::map<std::string, std::map<DWORD, DWORD>> IPv4MaskMap;

	std::vector<std::string> ipv4, mask;
	DWORD dwIP = 0, dwMask = 0;

	if (StrUtils::SplitToken(sIPv4, ".", ipv4) == 4 && StrUtils::SplitToken(sMask, ".", mask) == 4) {
		BYTE m[4];
		m[0] = atoi(mask[0].c_str());
		m[1] = atoi(mask[1].c_str());
		m[2] = atoi(mask[2].c_str());
		m[3] = atoi(mask[3].c_str());
		dwMask = m[0] << 24 | m[1] << 16 | m[2] << 8 | m[3];

		if (TcpSocket::IsValidMask(dwMask)) {
			BYTE ip[4];
			ip[0] = atoi(ipv4[0].c_str());
			ip[1] = atoi(ipv4[1].c_str());
			ip[2] = atoi(ipv4[2].c_str());
			ip[3] = atoi(ipv4[3].c_str());
			dwIP = ip[0] << 24 | ip[1] << 16 | ip[2] << 8 | ip[3];

			DWORD dwNetIP = dwIP & dwMask;
			DWORD dwFirstHostIP1 = dwNetIP + 1;
			DWORD dwLastHostIP1 = dwNetIP | ~dwMask - 1;

			std::string sNetIP(TcpSocket::IPv4DWordToStr(dwNetIP));
			std::string sFirstHostIP1(TcpSocket::IPv4DWordToStr(dwFirstHostIP1));
			std::string sLastHostIP1(TcpSocket::IPv4DWordToStr(dwLastHostIP1));
		}
	}

	return ret;
}

int mtkPack::TcpServer::DeleteFromIPMap(IP_TYPE type, const std::string& sIPv4, const std::string& sMask){
	int ret = 0;
	IPv4MaskMap* pMap = &mServerParam.WhiteIPMap;
	return ret;
}

const bool mtkPack::TcpServer::IsIPBanned(const std::string& sIPv4) {
	bool ret = false;
	if (mServerParam.BlackIPList.find(sIPv4) != mServerParam.BlackIPList.end()) {
		ret = true;
	} else {
		mServerParam.BlackIpMap;
	}

	return ret;
}

int mtkPack::TcpServer::AddToMsgRuleList(const std::string& sMsg) {
	int ret = 0;
	if (!sMsg.empty()) {
		mServerParam.MsgRuleList.insert(sMsg);
		ret = 1;
	}
	return ret;
}

const bool mtkPack::TcpServer::CheckMsgByRules(const std::string& sMsg) {
	bool ret = false;
	MesssageRuleList::const_iterator ruleitr = mServerParam.MsgRuleList.begin();
	for (; ruleitr != mServerParam.MsgRuleList.end(); ruleitr++) {
		if (sMsg.find(*ruleitr) != std::string::npos) {
			ret = true;
			break;
		}
	}
	return ret;
}

int mtkPack::TcpServer::Init(void) {
	int ret = 0;
#if 0
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	char szPort[6] = {0}; // 65535
	_itoa_s(mPort, szPort, 6, 10);

	mErrCode = getaddrinfo(mHost.c_str(), szPort, &hints, &mServerParam.AddrInfo);
	if (mErrCode != 0) {
		return ret;
	}

	mWinSock = socket(mServerParam.AddrInfo->ai_family, mServerParam.AddrInfo->ai_socktype, mServerParam.AddrInfo->ai_protocol);
	if (mWinSock == INVALID_SOCKET) {
		mErrCode = WSAGetLastError();
		return ret;
	}

	mErrCode = bind(mWinSock, mServerParam.AddrInfo->ai_addr, (int)mServerParam.AddrInfo->ai_addrlen);
#else
	mServerParam.SAddrIn.sin_family = AF_INET;
	mServerParam.SAddrIn.sin_addr.S_un.S_addr = INADDR_ANY;
	mServerParam.SAddrIn.sin_port = htons(mPort);

	mWinSock = socket(mServerParam.SAddrIn.sin_family, SOCK_STREAM, IPPROTO_TCP);
	if (mWinSock == INVALID_SOCKET) {
		mErrCode = WSAGetLastError();
		return ret;
	}

	mErrCode = bind(mWinSock, (sockaddr *)(&mServerParam.SAddrIn), sizeof(mServerParam.SAddrIn));
#endif
	if (mErrCode == SOCKET_ERROR) {
		mErrCode = WSAGetLastError();
		return ret;
	}

	mIsCreated = 1;

	mServerParam.ServingThreadHandle = (HANDLE)CreateThread(NULL, 0, ServingThread, this, CREATE_SUSPENDED, &mServerParam.ServingThreadID);
	if (mServerParam.ServingThreadHandle != NULL) {
		ret = 1;
	}
	
	return ret;
}

bool mtkPack::TcpServer::IsClientStopped(const TcpClient *pClient) {
	bool ret = true;
	if (pClient) {
		ret = (pClient->IsRunning() == 1) ? false : true;
	}
	return ret;
}

void mtkPack::TcpServer::ServingThreadCallback(PTP_CALLBACK_INSTANCE instance, void * context, PTP_WORK work)
{
	if (context == NULL) {
		return;
	}

	TcpServer *pServer = (TcpServer *)context;
	{
		AutoLock al(pServer->mServerParam.Sync);
		TcpClient *pClient = new TcpClient(pServer->mServerParam.AcceptedSocket, pServer->mRecvCallback);
		pClient->Start();
		pServer->mServerParam.ClientPtrList.push_back(pClient);
	}
	pServer->mServerParam.TryRemoveDeadClients(pServer->mServerParam.ClientPtrList.size() - 1);
}

DWORD WINAPI mtkPack::TcpServer::ServingThread(LPVOID pParam) {
	DWORD ret = 0;
	if (pParam == NULL) {
		return ret;
	}
	TcpServer *server = (TcpServer *)pParam;
	TP_WORK * work = CreateThreadpoolWork(ServingThreadCallback, server, NULL);
	if (work == NULL) {
		//printf("Error %d in CreateThreadpoolWork", GetLastError());
		return ret;
	}

	for (;;) {
		ret = WaitForSingleObject(server->mStopSocketEvent, server->mServerParam.Interval);
		if (ret == WAIT_OBJECT_0 ) {
			// exit thread when mStopSocketEvent signaled
			break;
		}
		if (ret == WAIT_TIMEOUT) {
			if (server->mServerParam.ClientPtrList.size() >= server->mServerParam.MaxClients) {
				server->mServerParam.TryRemoveDeadClients(server->mServerParam.MaxClients - 1);
				Sleep(server->mServerParam.Interval);
				continue;
			}

			SOCKET clientsock = accept(server->mServerSocket, NULL, NULL);
			if (clientsock != INVALID_SOCKET) {
				std::string clientip(TcpSocket::GetIPv4(clientsock, CLIENT_IP));
				bool ipbanned = server->IsIPBanned(clientip);

				if (ipbanned) {
					closesocket(clientsock);
					if (server->mBanCallback != NULL) {
						server->mBanCallback(clientip);
					}
				} else {
					int acceptclient = 1;
					AutoLock al(server->mServerParam.Sync);
					if (server->mAcceptCallback != NULL) {
						acceptclient = server->mAcceptCallback(*server, clientsock);
					}
					if (acceptclient) {
						server->mServerParam.AcceptedSocket = clientsock;
						SubmitThreadpoolWork(work);
					}
				}
			} else {
				server->mServerParam.TryRemoveDeadClients(server->mServerParam.MaxClients - 1);
			}
		} else if (ret == WAIT_FAILED) {
			server->mErrCode = GetLastError();
		} 
	}

	// shutdown and cancel all pending missions
	WaitForThreadpoolWorkCallbacks(work, TRUE);
	CloseThreadpoolWork(work);

	// release all clients from server's ptrlist
	TcpClientPtrList::iterator clientitr = server->mServerParam.ClientPtrList.begin();
	for (; clientitr != server->mServerParam.ClientPtrList.end(); clientitr++) {
		SecureDelete<TcpClient> sd1(&*clientitr, 0);
	}
	server->mServerParam.ClientPtrList.clear();

	return ret;
}

//////////////////////////////////////////////////////////////////////
mtkPack::TcpClientParam::TcpClientParam(void)
	:AddrInfo(NULL), ClientThreadHandle(NULL), ClientThreadID(0)
{
}

mtkPack::TcpClientParam::~TcpClientParam(void) {
	if (AddrInfo) {
		freeaddrinfo(AddrInfo);
		AddrInfo = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
mtkPack::TcpClient::TcpClient(void)
	:mRemoteHost(mHost), mRemotePort(mPort), mClientSocket(mWinSock)
{
}

mtkPack::TcpClient::TcpClient(const SOCKET &Socket, RecvCallback pRecvCallback)
	:mRemoteHost(mHost), mRemotePort(mPort), mClientSocket(mWinSock)
{
	Setup(Socket, pRecvCallback);
}

mtkPack::TcpClient::TcpClient(const std::string &sRemoteHost, const int iRemotePort, RecvCallback pRecvCallback)
	:mRemoteHost(mHost), mRemotePort(mPort), mClientSocket(mWinSock)
{
	Setup(sRemoteHost, iRemotePort, pRecvCallback);
}

mtkPack::TcpClient::~TcpClient(void) {

}

void mtkPack::TcpClient::Setup(const SOCKET &Socket, RecvCallback pRecvCallback) {
	mClientSocket = Socket;
	mRecvCallback = pRecvCallback;
	Init();
	if (mIsCreated) {
		mRunning = 1;
	}
}

void mtkPack::TcpClient::Setup(const std::string &sRemoteHost, const int iRemotePort, RecvCallback pRecvCallback) {
	mRemoteHost = sRemoteHost;
	mRemotePort = iRemotePort;
	mClientSocket = INVALID_SOCKET;
	mRecvCallback = pRecvCallback;
	Init();
}

bool mtkPack::TcpClient::HasReceivedData(void) const {
	bool ret = const_cast<TcpClient *>(this)->IsValid(CHK_DATA);
	if (ret) {
		ret = (mData.length() > 0) ? true : false;
	}
	return ret;
}

const char* mtkPack::TcpClient::GetData(void) {
	try {
		return mData.c_str();
	} catch (...) {
	}
	return NULL;
}

int mtkPack::TcpClient::Start(void) {
	int ret = 0;
	if (!mIsCreated) {
		return ret;
	}

	if (!mRunning) {
		fd_set fdread;
		struct timeval tv = { 0, SELECT_SYNC_MSEC };
		FD_ZERO(&fdread);
		FD_SET(mClientSocket, &fdread);

		connect(mClientSocket, mClientParam.AddrInfo->ai_addr, mClientParam.AddrInfo->ai_addrlen);		
		ret = select(0, &fdread, NULL, NULL, &tv);
		mRunning = 1;
		ret = 1;
	}
	if (mRunning) {
		mClientParam.ClientThreadHandle = (HANDLE)CreateThread(NULL, 0, ClientThread, this, 0, &mClientParam.ClientThreadID);
		if (mClientParam.ClientThreadHandle = NULL) {
			mIsCreated = 0;
		}
	}

	return ret;
}

int mtkPack::TcpClient::Stop(void) {
	return Clear();
}

const std::string mtkPack::TcpClient::GetIPv4(void) {
	return TcpSocket::GetIPv4(mClientSocket, CLIENT_IP);
}

int mtkPack::TcpClient::Send(const char *pData) {
	int ret = 0;
	if (pData == NULL) {
		return ret;
	}
	
	for(;;) {
		if (IsValid(CHK_ALL)) {
			int datalen = send(mClientSocket, pData, (int)strlen(pData), 0);
			if (datalen == SOCKET_ERROR) {
				mErrCode = WSAGetLastError();
				Sleep(SELECT_SYNC_MSEC);
			} else {
				ret = 1;
				break;
			}
		} else {
			break;
		}
	}
	return ret;
}


void mtkPack::TcpClient::SetClosingKeyword(const std::string &sKeyword) {
	mClosingKeyword = sKeyword;
}

SOCKET mtkPack::TcpClient::GetSafeClientSocket(void) {
	try {
		return mClientSocket;
	}
	catch (...) {
		return INVALID_SOCKET;
	}
}

mtkPack::RecvCallback mtkPack::TcpClient::GetSafeRecvCallback(void) {
	RecvCallback pRecvCallBack = NULL;
	if (this->IsRunning() && this->IsValid(CHK_ALL)) {
		pRecvCallBack = this->mRecvCallback;
	}
	return pRecvCallBack;
}

bool mtkPack::TcpClient::IsValid(const ClientChkMask& chkmask) {
	bool ret = true;

	if (chkmask & CHK_STOPEVENT) {
		ret &= (GetSafeStopSocketEvent() != INVALID_HANDLE_VALUE);
	}

	if  (chkmask & CHK_SELECTEVENT) {
		ret &= (GetSafeSelectSockEvent() != INVALID_HANDLE_VALUE);
	}

	if  (chkmask & CHK_SOCKET) {
		ret &= (GetSafeClientSocket() != INVALID_SOCKET);
	}

	if  (chkmask & CHK_DATA) {
		try {
			std::string tmp(mData);
			ret &= true;
		}
		catch (...) {
			ret &= false;
		}
	}

	return ret;
}

int mtkPack::TcpClient::Init(void) {
	mClosingKeyword = "CloseClientSocketNow";

	if (mClientSocket == INVALID_SOCKET) {
		struct addrinfo *result = NULL,	*ptr = NULL, hints;
		ZeroMemory( &hints, sizeof(hints) );
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		char szPort[6] = {0}; // 65535
		_itoa_s(mRemotePort, szPort, 6, 10);
		// Resolve the server address and port
		int iResult = getaddrinfo(mRemoteHost.c_str(), szPort, &hints, &(mClientParam.AddrInfo));
		if ( iResult != 0 ) {
			return 0;
		}
		mClientSocket = socket(mClientParam.AddrInfo->ai_family, mClientParam.AddrInfo->ai_socktype, mClientParam.AddrInfo->ai_protocol);		
	}

	unsigned long nonblockmode = 1;
	ioctlsocket(mClientSocket, FIONBIO, &nonblockmode);

	mErrCode = WSAEventSelect(mClientSocket, mSelectSockEvent, FD_READ|FD_CLOSE);
	mIsCreated = (mClientSocket != INVALID_SOCKET)? 1 : 0;

	return mIsCreated;
}

DWORD WINAPI mtkPack::TcpClient::ClientThread(LPVOID pParam) {
	DWORD ret = 0;
	if (pParam == NULL) {
		return ret;
	}

	TcpClient *client = (TcpClient *)pParam;
	if (!client->IsValid(CHK_ALL) || !client->IsRunning()) {
		return ret;
	}
	char *buf = new char[RECV_BUFFER_LEN + 1]; // handles NULL
	SecureDelete<char> sd1(&buf, 1);
	while (client && WaitForSingleObject(client->GetSafeStopSocketEvent(), SELECT_SYNC_MSEC / 2) != WAIT_OBJECT_0) {
		if (!client->IsRunning() || !client->IsValid(CHK_ALL)) {
			break;
		}
		HANDLE hSelect = client->GetSafeSelectSockEvent();
		DWORD r = WSAWaitForMultipleEvents(1, &hSelect, FALSE, SELECT_SYNC_MSEC / 2, FALSE);
		if (r != WSA_WAIT_TIMEOUT) {
			WSANETWORKEVENTS netevent;
			WSAEnumNetworkEvents(client->GetSafeClientSocket(), client->GetSafeSelectSockEvent(), &netevent);
			if (netevent.lNetworkEvents & FD_READ) {
				int datalen = 0;
				do {
					if (datalen == 0) {
						if (!client->IsValid(CHK_ALL) || !client->IsRunning()) {
							return ret;
						}
						client->mData.clear();
					}
					memset(buf, 0, RECV_BUFFER_LEN + 1);
					datalen = recv(client->GetSafeClientSocket(), buf, RECV_BUFFER_LEN, 0);
					if (datalen > 0) {
						if (!client->IsValid(CHK_ALL) || !client->IsRunning()) {
							return ret;
						}
						client->mData.append(buf);
					}

				} while (datalen > 0);
				
				if (!client->IsValid(CHK_ALL) || !client->IsRunning()) {
					break;
				}

				if (client->mData == client->mClosingKeyword) {
					return client->Clear();
				} else {
					if (client->IsValid(CHK_ALL) && client->mRecvCallback != NULL) {
						ret = client->mRecvCallback(client);
					}
				}
			} else if (netevent.lNetworkEvents & FD_CLOSE) {
				client->Clear();
			}
		}// if (r != WSA_WAIT_TIMEOUT)
	}

	return ret;
}