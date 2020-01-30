// Windows SDK needs to be downloaded
// I did this in Visual Studio 2017
// If the socket lib is not found, the "Include", "Src" and "Lib" folders from the SDK need to be included. Info below
// https://docs.microsoft.com/en-us/windows/win32/winsock/creating-a-basic-winsock-application

#include <iostream>
#include <chrono>
#include <ctime>
//from windows SDK
#include <WS2tcpip.h>
#include <string>

#pragma comment(lib, "WS2_32.lib")


#define DEFAULT_PORT "45000"
#define DEFAULT_BUFLEN 128
#define SERVER_ID 255

typedef unsigned long long uint64_t;

class Node {
	uint64_t nodeId;
	uint64_t extraCounter;

public:
	Node() : nodeId(0), extraCounter(0) {}
	Node(uint64_t id) : nodeId(id), extraCounter(0) {}

	uint64_t getNodeId() { return this->nodeId; }
	void setNodeId(uint64_t id) { this->nodeId = id; }

	//millisecondsSinceEpoch since epoch
	uint64_t getTimestamp() {
		uint64_t millisecondsSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>
			(std::chrono::system_clock::now().time_since_epoch()).count();

		return millisecondsSinceEpoch;
	}

	// 10 bits nodeId + 12 bits counter (for 4096 ids) + 42 bits timestamp (milliseconds)
	uint64_t getId() {
		uint64_t GUID = 0;
		GUID |= (nodeId << 54);
		GUID |= (extraCounter << 42);
		GUID |= this->getTimestamp();
		if (extraCounter == 4095)
			extraCounter = 0;
		else
			extraCounter++;
		return GUID;
	}
};

SOCKET initSocket();
void cleanup(SOCKET ClientSocket);

using namespace std;

int main()
{
	Node *node = new Node(SERVER_ID);
	SOCKET ClientSocket = initSocket();

	if (ClientSocket == INVALID_SOCKET) {
		cout << "Init failed!\n";
		return 1;
	}

	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	int iResult = 0;
	std::cout << "Start receiving!\n";

	auto start = std::chrono::system_clock::now();
	// Receive until the peer shuts down the connection
	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			//printf("Bytes received: %d\n", iResult);
			//printf("%s\n", recvbuf);

			if (strstr(recvbuf, "getID") != nullptr) {
				std::string toSend = std::to_string(node->getId());
				uint64_t id = node->getId();
				
				send(ClientSocket, toSend.c_str(), toSend.length(), 0);
			}
			else if (strstr(recvbuf, "STOP") != nullptr) {
				cout << "BREAK!" << endl;
				break;
			}
		}
		else if (iResult == 0) {
			//nothing
		}
		else {
			printf("recv failed: %d\n", WSAGetLastError());
			cleanup(ClientSocket);
			return 1;
		}

	} while (true);
	auto end = std::chrono::system_clock::now();

	std::chrono::duration<double> d = end - start;

	std::time_t startTime = std::chrono::system_clock::to_time_t(start);
	std::time_t endTime = std::chrono::system_clock::to_time_t(end);

	cout << "Started at: " << std::ctime(&startTime);
	cout << start.time_since_epoch().count() << endl;
	cout << "Ended at:   " << std::ctime(&endTime);
	cout << end.time_since_epoch().count() << endl;

	cout << "Duration: " << d.count() << "s" << endl;

	// cleanup
	cleanup(ClientSocket);

	return 0;
}

SOCKET initSocket() {

	struct addrinfo *result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Initialize Winsock
	int iResult;
	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed: " << iResult << std::endl;
		return INVALID_SOCKET;
	}

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return INVALID_SOCKET;
	}

	SOCKET ListenSocket = INVALID_SOCKET;
	// Create a SOCKET for the server to listen for client connections
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return INVALID_SOCKET;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return INVALID_SOCKET;
	}
	freeaddrinfo(result);

	cout << "Start listening!\n";
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return INVALID_SOCKET;
	}

	SOCKET ClientSocket = INVALID_SOCKET;

	cout << "Start accepting!\n";
	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return INVALID_SOCKET;
	}
	cout << "Init done!\n";
	return ClientSocket;
}

void cleanup(SOCKET ClientSocket) {
	if (ClientSocket != INVALID_SOCKET) {
		closesocket(ClientSocket);
		WSACleanup();
	}
}
