// client_test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>
#include <ctime>
//from windows SDK
#include <WS2tcpip.h>
#include <string>

using namespace std;

#pragma comment(lib, "WS2_32.lib")

#define DEFAULT_PORT "45000"
#define DEFAULT_BUFLEN 128
#define TEST_SIZE 100000

SOCKET initSocket();
void displayAdjacentIdsWhoAreEqual(std::string arr[], int arrSize);
void displayArray(std::string arr[], int arrSize);

int main()
{
	SOCKET ConnectSocket = initSocket();
	if (ConnectSocket == INVALID_SOCKET) {
		cout << "Init failed!\n";
		return 1;
	}

	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];

	int iResult = 0;

	std::string getID = "getID";
	std::string *testArray = new string[TEST_SIZE];
	//or you can do this with TEST_SIZE = 1024 but it gives the same result
	//std::string testArray[TEST_SIZE];

	cout << "Sending requests... (should take ~4 seconds for 100k)" << endl;
	for (int i = 0; i < TEST_SIZE; i++) {
		iResult = send(ConnectSocket, getID.c_str(), getID.length(), 0);
		if (iResult > 0) {
			//printf("Bytes recv: %d\n", iResult);

			recv(ConnectSocket, recvbuf, 64, 0);
			testArray[i] = recvbuf;
		}
		else if (iResult == 0) {}
		else
			printf("send failed: %d\n", WSAGetLastError());
	}
	std::string stop = "STOP";
	send(ConnectSocket, stop.c_str(), stop.length(), 0);

	std::cout << "done receiving!\n";

	displayAdjacentIdsWhoAreEqual(testArray, TEST_SIZE);
	//displayArray(testArray, TEST_SIZE);

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	delete[] testArray;
	return 0;
}

SOCKET initSocket() {
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Initialize Winsock
	int iResult;
	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed: " << iResult << std::endl;
		return INVALID_SOCKET;
	}

	// Resolve the server address and port
	iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return INVALID_SOCKET;
	}

	SOCKET ConnectSocket = INVALID_SOCKET;
	// Attempt to connect to the first address returned by the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return INVALID_SOCKET;
	}

	// Connect to server.
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}

	// Should really try the next address returned by getaddrinfo
	// if the connect call failed
	// But for this simple example we just free the resources
	// returned by getaddrinfo and print an error message

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return INVALID_SOCKET;
	}

	cout << "Init done!\n";
	return ConnectSocket;
}

void displayAdjacentIdsWhoAreEqual(std::string arr[], int arrSize) {

	for (int i = 0; i < arrSize - 1; i++) {
		if (arr[i] == arr[i + 1]) {
			cout << "### " << i << "e duplicat!" << endl;
		}
	}
	cout << "done searching duplicates\n";
}

void displayArray(std::string arr[], int arrSize) {
	for (int i = 0; i < TEST_SIZE - 1; i++) {
		char toBeParsed[65];
		strncpy_s(toBeParsed, arr[i].c_str(), 64);
		toBeParsed[64] = '\0';
		char *end;
		int64_t GUID = std::strtoll(toBeParsed, &end, 10);
		printf("GUID: %llu\n", GUID);
	}
	cout << "Done displaying!\n";
}
