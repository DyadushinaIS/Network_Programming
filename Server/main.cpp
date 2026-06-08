//Server

/*#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include<iostream>
#include<Windows.h>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iphlpapi.h>
using namespace std;

#pragma comment(lib, "WS2_32.lib")

#define MTU	1500

string FormatLastError(DWORD errorCode = WSAGetLastError())
{
	LPSTR messageBuffer = nullptr;

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&messageBuffer, 0, nullptr
	);

	string errorMessage;
	if (messageBuffer == nullptr) {
		char buffer[100];
		sprintf_s(buffer, sizeof(buffer), "Unknown error (code: %lu)", errorCode);
		errorMessage = buffer;
	}
	else {
		errorMessage = messageBuffer;
		LocalFree(messageBuffer);
	}

	return errorMessage;
}

void main()
{
	setlocale(LC_ALL, "");
	cout << "SERVER" << endl;
	//1) Инициализация WinSOCK:
	WSADATA wsaData;
	INT iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		cout << "WSAStartup failed with error: " << FormatLastError (iResult) << endl;
		return;
	}

	//2) Параметры подключения:
	addrinfo hints;
	addrinfo* target;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;	//Соединение будет работать в режиме 'LISTENING';

	iResult = getaddrinfo(NULL, "27015", &hints, &target);
	if (iResult != 0)
	{
		cout << "getaddrinfo() failed with error: " << FormatLastError(iResult) << endl;
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//3) Создание серверного сокета, который он будет постоянно прослушивать:
	SOCKET listen_socket =
		socket(target->ai_family, target->ai_socktype, target->ai_protocol);
	if (listen_socket == INVALID_SOCKET)
	{
		cout << "SOCKET creation failed with error: " << FormatLastError() << endl;
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//4) Привязываем сокет к интерфейсу и порту:
	iResult = bind(listen_socket, target->ai_addr, target->ai_addrlen);
	if (iResult != 0)
	{
		cout << "bind failed with error: " << FormatLastError() << endl;
		freeaddrinfo(target);
		closesocket(listen_socket);
		WSACleanup();
		return;
	}

	//5) Запускаем прослушивание порта:
	if (listen(listen_socket, 1) == SOCKET_ERROR)	//1 - Максимальное количество одновременно подключенных клиентов
	{
		cout << "Listen failed with error: " << FormatLastError() << endl;
		closesocket(listen_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//6) Принимаем подключение от клиента
	SOCKET client_socket = accept(listen_socket, NULL, NULL);
	if (client_socket == INVALID_SOCKET)
	{
		cout << "Accept failed with error: " << FormatLastError() << endl;
		closesocket(listen_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//7) Получаем данные от клиента:
	CHAR recv_buffer[MTU] = {};
	CHAR send_buffer[MTU] = "Hello client";
	INT iReceivedBytes = 0;
	INT iSentBytes = 0;
	do
	{
		iReceivedBytes = recv(client_socket, recv_buffer, MTU, 0);
		//Функция recv() - Receive ожидает получение данных по указанному сокету, и возвращает количество полученных Байт.
		if (iReceivedBytes > 0)
		{
			cout << "Received " << iReceivedBytes << " " << recv_buffer << endl;
			iSentBytes = send(client_socket, send_buffer, strlen(send_buffer), 0);
			if (iSentBytes == SOCKET_ERROR)	cout << "Send failed with error:\t" << FormatLastError() << endl;
			else cout << iSentBytes << " Bytes sent" << endl;
		}
		else if (iReceivedBytes == 0) cout << "Connection closing..." << endl;
		else cout << "Receive failed with error: " << FormatLastError() << endl;
	} while (iReceivedBytes > 0);

	//8) Разрываем TCP-соединение:
	iResult = shutdown(client_socket, SD_BOTH);
	if (iResult == SOCKET_ERROR)
		cout << "Shutdown failed with error:\t" << FormatLastError() << endl;

	//9) Освобождаем ресурсы, занятиые WinSOCK:
	closesocket(listen_socket);
	closesocket(client_socket);
	freeaddrinfo(target);
	WSACleanup();
}
*/

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif 

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<Windows.h>
#include <iostream>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iphlpapi.h>

#include<FormatLastError.h>
using namespace std;

#pragma comment(lib,"WS2_32.lib")
#pragma comment(lib,"FormatLastError.lib")

#define MTU 1500

void main()
{
	setlocale(LC_ALL, "");
	cout << "SERVER" << endl;
	//1)Инициализация WinSOCK
	WSADATA wsaData;
	INT iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		cout << "WSAStartup failed with error: " << iResult << endl;
		return;
	}
	//2) Параметры подключения
	addrinfo hints;
	addrinfo* target;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;	//PASSIVE значит, что соединение будет работать в режиме listening

	iResult = getaddrinfo(NULL, "27015", &hints, &target);
	if (iResult != 0)
	{
		cout << "getaddrinfo failed with error: " << iResult << endl;
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//3) Создание серверного сокета, который он будет постоянно прослушивать
	SOCKET listen_socket =
		socket(target->ai_family, target->ai_socktype, target->ai_protocol);
	if (listen_socket == INVALID_SOCKET)
	{
		cout << "SOCKET creation failed with error: " << WSAGetLastError() << endl;
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//4) Привязываем сокет к интерфейсу и порту:
	iResult = bind(listen_socket, target->ai_addr, target->ai_addrlen);
	if (iResult != 0)
	{
		cout << "bind failed with error: " << WSAGetLastError() << endl;
		freeaddrinfo(target);
		closesocket(listen_socket);
		WSACleanup();
		return;
	}

	//5) Запускаем прослушивание порта
	if (listen(listen_socket, 1) == SOCKET_ERROR)    //1 - максимальное количество одновременно подключенных клиентов
	{
		cout << "Listen failed with error: " << WSAGetLastError() << endl;
		closesocket(listen_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//6) Принимаем подключение от клиента
	SOCKADDR_IN client_address;
	INT client_address_len = sizeof(client_address);
	SOCKET client_socket = accept(listen_socket, (SOCKADDR*) & client_address, &client_address_len);
	if (client_socket == INVALID_SOCKET)
	{
		cout << "Accept failed with error: " << WSAGetLastError() << endl;
		closesocket(listen_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}
	cout << inet_ntoa(client_address.sin_addr) << ":" << ntohs(client_address.sin_port) << endl;

	//7) Получаем данные от клиента
	CHAR recv_buffer[MTU] = {};
	CHAR send_buffer[MTU] = "Hello, client!";
	INT iReceivedBytes = 0;
	INT iSentBytes = 0;
	do
	{
		iReceivedBytes = recv(client_socket, recv_buffer, MTU, 0);
		//функция recv() - Receive ожидает получения данных по указанному сокету и возвращает количество полученных байт
		if (iReceivedBytes > 0)
		{
			cout << "Received " << iReceivedBytes << " " << recv_buffer << endl;
			iSentBytes = send(client_socket, send_buffer, strlen(send_buffer), 0);
			if (iSentBytes == SOCKET_ERROR)	cout << "Send failed with error: " << WSAGetLastError() << endl;
			else cout << iSentBytes << " Bytes sent" << endl;
		}
		else if (iReceivedBytes == 0) cout << "Connection closing..." << endl;
		else cout << "Receive failed with error " << WSAGetLastError() << endl;
	} while (iReceivedBytes > 0);

	//8) Разрываем TCP-соединение
	iResult = shutdown(client_socket, SD_BOTH);
	if (iResult != SOCKET_ERROR) cout << "shutdown failed with error:\t" << WSAGetLastError();	

	//9)Освобождаем ресурсы, занятые WinSOCK
	closesocket(listen_socket);
	freeaddrinfo(target);
	WSACleanup();
}

	//--------------01:54:04-------------------------    ЗА 04 ИЮНЯ!!!!! (WINSOCK DAY 3)