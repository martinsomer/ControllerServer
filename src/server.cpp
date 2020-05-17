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
 * This program is based on Microsoft's Winsock server code example.
 * https://docs.microsoft.com/en-us/windows/win32/winsock/complete-server-code
 */

#include "pch.hpp"
#include "emulator.hpp"

void startServer(void) {
	WSADATA wsaData;
	int res;
	SOCKET ServerSocket = INVALID_SOCKET;
	struct addrinfo hints;
	struct addrinfo *result = NULL;

	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);

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
	ServerSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ServerSocket == INVALID_SOCKET) {
		std::cout << "Failed to create socket. Code: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	// Set up the listening socket
	res = bind(ServerSocket, result->ai_addr, (int)result->ai_addrlen);
	if (res == SOCKET_ERROR) {
		std::cout << "Failed to bind socket. Code: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		closesocket(ServerSocket);
		WSACleanup();
		return;
	}

	freeaddrinfo(result);

	// Get server port
	res = getsockname(ServerSocket, (struct sockaddr *)&sin, &len);
	if (res != 0) {
		std::cout << "Failed to retrieve socket name. Code: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		closesocket(ServerSocket);
		WSACleanup();
		return;
	}

	std::cout << "Server running on port " << ntohs(sin.sin_port) << std::endl;

	// Receive until disconnect command is received
	char data[18];
	while (true) {
		res = recvfrom(ServerSocket, data, sizeof(data), 0, NULL, NULL);

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
	res = closesocket(ServerSocket);
	if (res != 0) {
		std::cout << "Failed to close socket. Code: " << WSAGetLastError() << std::endl;
	}

	res = WSACleanup();
	if (res != 0) {
		std::cout << "Failed to terminate Winsock. Code: " << WSAGetLastError() << std::endl;
	}
}

