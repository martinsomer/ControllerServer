/*
 * This is the server program.
 *
 * It creates a UDP server and waits for packets from the client.
 * When a packet is received, the server determines
 * if it is a disconnect message, or contains
 * valid data to be asynchronously emulated.
 *
 * The port number of the server is selected
 * automatically, and is displayed in the console window.
 *
 * This program is based on Microsoft's examples.
 * https://docs.microsoft.com/en-us/windows/win32/winsock/complete-server-code
 * https://docs.microsoft.com/en-us/windows/win32/api/iphlpapi/nf-iphlpapi-getadaptersinfo
 */

#include "pch.hpp"
#include "emulator.hpp"

int getServerIpAddress(std::string &ipAddress) {
	PIP_ADAPTER_INFO adapterInfo;
	PIP_ADAPTER_INFO adapter;
	ULONG size = 0;
	DWORD res;

	// Retrieve adapter information 
	while (true) {
		adapterInfo = (IP_ADAPTER_INFO *) malloc(size);
		if (adapterInfo == NULL) {
			return 10;
		}

		res = GetAdaptersInfo(adapterInfo, &size);
		if (res == NO_ERROR) {
			break;
		}

		free(adapterInfo);
		if (res != ERROR_BUFFER_OVERFLOW) {
			return 11;
		}
	}

	// Loop over adapters to find suitable IPv4 address
	adapter = adapterInfo;
	while (adapter) {
		ipAddress = adapter->IpAddressList.IpAddress.String;
		if (ipAddress != "0.0.0.0") {
			free(adapterInfo);
			return 0;
		}

		adapter = adapter->Next;
	}

	// No suitable address found
	free(adapterInfo);
	return 12;
}

void startServer(void) {
	WSADATA wsaData;
	int res;
	SOCKET serverSocket = INVALID_SOCKET;
	struct addrinfo hints;
	struct addrinfo *result = NULL;

	std::string ipAddress;
	struct sockaddr_in sockname;
	socklen_t socknameLen = sizeof(sockname);

	// Initialize Winsock
	res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != 0) {
		std::cout << "Failed to initialize Winsock. Code: " << res << std::endl;
		return;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	// Resolve the server address and port
	res = getaddrinfo(NULL, (char *) "0", &hints, &result);
	if (res != 0) {
		std::cout << "Failed to resolve server address. Code: " << res << std::endl;
		WSACleanup();
		return;
	}

	// Create a SOCKET for connecting to server
	serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (serverSocket == INVALID_SOCKET) {
		std::cout << "Failed to create socket. Code: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	// Set up the listening socket
	res = bind(serverSocket, result->ai_addr, (int)result->ai_addrlen);
	if (res == SOCKET_ERROR) {
		std::cout << "Failed to bind socket. Code: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		closesocket(serverSocket);
		WSACleanup();
		return;
	}

	// Get server IP address
	res = getServerIpAddress(ipAddress);
	if (res != 0) {
		std::cout << "Failed to retrieve IP address. Code: " << res << std::endl;
		freeaddrinfo(result);
		closesocket(serverSocket);
		WSACleanup();
		return;
	}

	// Get server port
	res = getsockname(serverSocket, (struct sockaddr *)&sockname, &socknameLen);
	if (res != 0) {
		std::cout << "Failed to retrieve socket name. Code: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		closesocket(serverSocket);
		WSACleanup();
		return;
	}

	std::cout << "Server running on " << ipAddress << ":" << ntohs(sockname.sin_port) << std::endl;

	// Receive until disconnect command is received
	char data[18];
	while (true) {
		res = recvfrom(serverSocket, data, sizeof(data), 0, NULL, NULL);

		if (res == 18) {
			boost::async(boost::launch::async, [&data] {
				emulate(data);
			});
		} else if (res == 1 && data[0] == -1) {
			std::cout << "Device disconnected." << std::endl;
			break;
		} else if (res < 0) {
			std::cout << "Failed to read from the socket. Code: " << WSAGetLastError() << std::endl;
			break;
		}
	}

	// Cleanup
	freeaddrinfo(result);
	
	res = closesocket(serverSocket);
	if (res != 0) {
		std::cout << "Failed to close socket. Code: " << WSAGetLastError() << std::endl;
	}

	res = WSACleanup();
	if (res != 0) {
		std::cout << "Failed to terminate Winsock. Code: " << WSAGetLastError() << std::endl;
	}
}

