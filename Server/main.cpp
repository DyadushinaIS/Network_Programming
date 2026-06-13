//Server
//#define _WINSOCK_DEPRECATED_NO_WARNINGS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include<iostream>
#include<Windows.h>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iphlpapi.h>
#include<FormatLastError.h>
#include<Messages.h>
using namespace std;

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "FormatLastError.lib")

#define MTU				 1500
#define MAX_CONNECTIONS		3

//создаем структуру, чтобы хранить сразу всю информацию про IP, порты и прочее

//struct ABOUT_CLIENTS 
//{
//	SOCKET socket;
//	DWORD threadID;
//	CHAR address[32];
//	USHORT port;
//};

//ABOUT_CLIENTS client[MAX_CONNECTIONS] = {};
INT g_ActiveClients = 0;
CHAR client_addresses[MAX_CONNECTIONS][32];  // IP-адреса клиентов
USHORT client_ports[MAX_CONNECTIONS];         // Порты клиентов

// Объявления функций
VOID ShowActiveClients();
VOID ClientHandle(LPVOID param);  // Изменено с SOCKET на LPVOID
VOID Shift(INT index);
INT GetClientIndex(DWORD dwThreadID);
VOID Broadcast(CHAR sz_message[], INT sender_index);

SOCKET client_sockets[MAX_CONNECTIONS] = {};
DWORD  dwThreadIDs[MAX_CONNECTIONS] = {};		//Идентификаторы потоков
HANDLE hThreads[MAX_CONNECTIONS] = {};			//Дескрипторы потов

void main()
{
	setlocale(LC_ALL, "");
	DWORD dwError = 0;
	CHAR szError[256] = {};
	cout << "SERVER" << endl;
	//1) Инициализация WinSOCK:
	WSADATA wsaData;
	INT iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		cout << "WSAStartup failed with error: " << iResult << endl;
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
		cout << "getaddrinfo() failed with error: " << iResult << endl;
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//3) Создание серверного сокета, который он будет постоянно прослушивать:
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

	//5) Запускаем прослушивание порта:
	if (listen(listen_socket, MAX_CONNECTIONS) == SOCKET_ERROR)	//1 - Максимальное количество одновременно подключенных клиентов
	{
		cout << "Listen failed with error: " << WSAGetLastError() << endl;
		closesocket(listen_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//6) Принимаем подключение от клиента
	do
	{
		ShowActiveClients();
		SOCKADDR_IN client_address;
		INT client_address_len = sizeof(client_address);
		SOCKET client_socket = accept(listen_socket, (SOCKADDR*)&client_address, &client_address_len);
		if (client_socket == INVALID_SOCKET)
		{
			cout << "Accept failed with error: " << WSAGetLastError() << endl;
			closesocket(listen_socket);
			freeaddrinfo(target);
			WSACleanup();
			return;
		}
		CHAR sz_client_address[32];
		cout << inet_ntop(AF_INET, &client_address.sin_addr, sz_client_address, 32) << ":" << ntohs(client_address.sin_port) << endl;

		//7) Получаем данные от клиента:
		//ClientHandle(client_socket);
		if (g_ActiveClients < MAX_CONNECTIONS)
		{
			// Получаем информацию об адресе клиента
			SOCKADDR_IN client_address;
			INT client_address_len = sizeof(client_address);
			getpeername(client_socket, (SOCKADDR*)&client_address, &client_address_len);

			// Сохраняем сокет
			client_sockets[g_ActiveClients] = client_socket;

			// Сохраняем адрес и порт
			inet_ntop(AF_INET, &client_address.sin_addr,
				client_addresses[g_ActiveClients], 32);
			client_ports[g_ActiveClients] = ntohs(client_address.sin_port);

			// Передаем индекс в поток
			INT* client_index = new INT(g_ActiveClients);

			// Создаем поток - явно приводим функцию к нужному типу
			hThreads[g_ActiveClients] = CreateThread
			(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)ClientHandle,
				(LPVOID)client_index,
				0,
				&dwThreadIDs[g_ActiveClients]
			);

			if (hThreads[g_ActiveClients] == NULL)
			{
				cout << "Ошибка создания потока: " << GetLastError() << endl;
				delete client_index;
			}
			else
			{
				cout << "Клиент подключен: [" << client_addresses[g_ActiveClients] << ":"
					<< client_ports[g_ActiveClients] << "]" << endl;

				g_ActiveClients++;
				Sleep(10);
				cout << "Количество клиентов: " << g_ActiveClients << endl;
			}
		}
		else
		{
			iResult = send(client_socket, DECLINE_MESSAGE, strlen(DECLINE_MESSAGE), 0);
			dwError = WSAGetLastError();
			if (iResult != 0)cout << FormatLastError(dwError, szError) << endl;
			iResult = shutdown(client_socket, SD_BOTH); if (iResult != 0)cout << FormatLastError(WSAGetLastError(), szError) << endl;
			iResult = closesocket(client_socket);		if (iResult != 0)cout << FormatLastError(WSAGetLastError(), szError) << endl;
			cout << "DECLINED" << endl;
		}
	} while (true);
	//Синхронизируем все потоки с основным потоком, в котором выполняется main()
	{
		WaitForMultipleObjects(g_ActiveClients, hThreads, TRUE, INFINITE);
	}

	//9) Освобождаем ресурсы, занятиые WinSOCK:
	closesocket(listen_socket);
	freeaddrinfo(target);
	WSACleanup();
}
//INT GetClientIndex(DWORD dwThreadID)
//{
//	for (INT i = 0; i < g_ActiveClients; i++)
//	{
//		if (dwThreadID == dwThreadIDs[i])return i;
//	}
//	return -1;
//}

INT GetClientIndex(DWORD dwThreadID)
{
	for (INT i = 0; i < g_ActiveClients; i++)
	{
		if (dwThreadID == dwThreadIDs[i])
			return i;
	}
	return -1;
}


//VOID Shift(INT index)
//{
//	if (index == -1)return;
//	CloseHandle(hThreads[index]);
//	for (INT i = index; i < g_ActiveClients; i++)
//	{
//		client_sockets[i] = client_sockets[i + 1];
//		dwThreadIDs[i] = dwThreadIDs[i + 1];
//		hThreads[i] = hThreads[i + 1];
//	}
//	client_sockets[MAX_CONNECTIONS - 1] = NULL;
//	dwThreadIDs[MAX_CONNECTIONS - 1] = NULL;
//	hThreads[MAX_CONNECTIONS - 1] = NULL;
//	g_ActiveClients--;
//	ShowActiveClients();
//	cout << "Количество клиентов: " << g_ActiveClients << endl;
//}

VOID Shift(INT index)
{
	if (index == -1) return;

	CloseHandle(hThreads[index]);

	for (INT i = index; i < g_ActiveClients - 1; i++)
	{
		client_sockets[i] = client_sockets[i + 1];
		dwThreadIDs[i] = dwThreadIDs[i + 1];
		hThreads[i] = hThreads[i + 1];
		// Копируем адреса и порты
		strcpy_s(client_addresses[i], client_addresses[i + 1]);
		client_ports[i] = client_ports[i + 1];
	}

	// Очищаем последние элементы
	client_sockets[g_ActiveClients - 1] = INVALID_SOCKET;
	dwThreadIDs[g_ActiveClients - 1] = 0;
	hThreads[g_ActiveClients - 1] = NULL;
	memset(client_addresses[g_ActiveClients - 1], 0, 32);
	client_ports[g_ActiveClients - 1] = 0;

	g_ActiveClients--;
	ShowActiveClients();
	cout << "Количество клиентов: " << g_ActiveClients << endl;
}

VOID ShowActiveClients()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(hConsole, &info);
	SetConsoleCursorPosition(hConsole, { 8,0 });
	cout << "                                                                                ";
	SetConsoleCursorPosition(hConsole, { 25,0 });
	cout << "Количество клиентов: " << g_ActiveClients << endl;
	SetConsoleCursorPosition(hConsole, info.dwCursorPosition);
}
//VOID Broadcast(CHAR sz_message[], INT client_index)
//{
//	INT iResult = 0;
//	for (INT i = 0; i < g_ActiveClients; i++)
//	{
//		if (i != client_index)
//			iResult = send(client_sockets[i], sz_message, strlen(sz_message), 0);
//	}
//}

VOID Broadcast(CHAR sz_message[], INT sender_index)
{
	INT iResult = 0;
	CHAR formatted_message[MTU + 100] = {};

	// Проверяем, что отправитель существует
	if (sender_index >= 0 && sender_index < g_ActiveClients)
	{
		// Форматируем сообщение с информацией об отправителе
		sprintf_s(formatted_message, sizeof(formatted_message), "[%s:%d] %s",
			client_addresses[sender_index], client_ports[sender_index], sz_message);
	}
	else
	{
		strcpy_s(formatted_message, sz_message);
	}

	// Отправляем всем клиентам, кроме отправителя
	for (INT i = 0; i < g_ActiveClients; i++)
	{
		if (i != sender_index && client_sockets[i] != INVALID_SOCKET)
		{
			iResult = send(client_sockets[i], formatted_message, strlen(formatted_message), 0);
			if (iResult == SOCKET_ERROR)
			{
				cout << "Broadcast to client " << i << " failed" << endl;
			}
		}
	}
}
//VOID ClientHandle(SOCKET client_socket)
//{
//	INT iResult = 0;
//	DWORD dwError = 0;
//	CHAR szError[256] = {};
//	CHAR send_buffer[MTU] = "Hello client";
//	CHAR recv_buffer[MTU] = {};
//	INT iReceivedBytes = 0;
//	INT iSentBytes = 0;
//	do
//	{
//		ZeroMemory(recv_buffer, MTU);
//		cout << &recv_buffer << endl;
//		iReceivedBytes = recv(client_socket, recv_buffer, MTU, 0);
//		dwError = WSAGetLastError();
//		//Функция recv() - Receive ожидает получение данных по указанному сокету, и возвращает количество полученных Байт.
//		if (iReceivedBytes > 0)Broadcast(recv_buffer, GetClientIndex(GetCurrentThreadId()));
//		{
//			//sprintf(send_buffer, "\x1b[32m%s\x1b[0m", recv_buffer);
//			/*cout << "Received " << iReceivedBytes << " " << recv_buffer << endl;
//			iSentBytes = send(client_socket, recv_buffer, strlen(recv_buffer), 0);
//			if (iSentBytes == SOCKET_ERROR)	cout << "Send failed with error:\t" << WSAGetLastError() << endl;
//			else cout << iSentBytes << " Bytes sent" << endl;*/
//		}
//		//else if (iReceivedBytes == 0) cout << "Connection closing..." << endl;
//		//else cout << "Receive failed with error: " << FormatLastError(dwError, szError) << endl;
//	} while (iReceivedBytes > 0 && strcmp(recv_buffer, "exit") != 0);
//
//	//8) Разрываем TCP-соединение:
//	iResult = shutdown(client_socket, SD_BOTH);
//	dwError = WSAGetLastError();
//	if (iResult != SOCKET_ERROR)cout << "shutdown failed with error:\t" << FormatLastError(dwError, szError) << endl;
//	closesocket(client_socket);
//	Shift(GetClientIndex(GetCurrentThreadId()));
//	ExitThread(0);
//}

VOID ClientHandle(LPVOID param)
{
	INT client_index = *(INT*)param;
	delete (INT*)param;  // Освобождаем память

	SOCKET client_socket = client_sockets[client_index];

	INT iResult = 0;
	DWORD dwError = 0;
	CHAR szError[256] = {};
	CHAR recv_buffer[MTU] = {};
	INT iReceivedBytes = 0;

	do
	{
		ZeroMemory(recv_buffer, MTU);
		iReceivedBytes = recv(client_socket, recv_buffer, MTU, 0);
		dwError = WSAGetLastError();

		if (iReceivedBytes > 0)
		{
			// Убираем лишние символы перевода строки, если они есть
			recv_buffer[strcspn(recv_buffer, "\r\n")] = 0;

			// Показываем на сервере от кого пришло сообщение
			cout << "\nСообщение от [" << client_addresses[client_index] << ":"
				<< client_ports[client_index] << "]: " << recv_buffer << endl;

			// Отправляем всем клиентам с указанием отправителя
			Broadcast(recv_buffer, client_index);
		}

	} while (iReceivedBytes > 0 && strcmp(recv_buffer, "exit") != 0);

	// Разрываем TCP-соединение
	iResult = shutdown(client_socket, SD_BOTH);
	dwError = WSAGetLastError();
	if (iResult == SOCKET_ERROR)
		cout << "shutdown failed with error:\t" << FormatLastError(dwError, szError) << endl;

	closesocket(client_socket);
	Shift(client_index);
	ExitThread(0);
}