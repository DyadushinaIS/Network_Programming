#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
//если с библиотекой Winsock2.h подключается файл Windows.h или iphlpapi.h,
//то они тоже подключают файл Winsock2.h, что приводит к конфликтам
//для того, чтолбы <Windows.h> и <iphpapi> не подтягивали Winsock2.h, создается макроопределение
#endif // !WIN32_LEAN_AND_MEAN
#define MTU 1500		//Maximum transfer Unit - Максимально возможный размер Ethernet-кадра


#include <iostream>
#include <Windows.h>
#include <Winsock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>

#include<FormatLastError.h>
using namespace std;

#pragma comment(lib,"WS2_32.lib") //встраиваем статическую библиотеку для заголовка <WS2tcpip.h>
#pragma comment(lib,"FormatLastError.lib")

//string FormatLastError(DWORD errorCode = WSAGetLastError())
//{
//	LPSTR messageBuffer = nullptr;
//
//	FormatMessageA(
//		FORMAT_MESSAGE_ALLOCATE_BUFFER |
//		FORMAT_MESSAGE_FROM_SYSTEM |
//		FORMAT_MESSAGE_IGNORE_INSERTS,
//		nullptr, errorCode,
//		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//		(LPSTR)&messageBuffer, 0, nullptr
//	);
//
//	string errorMessage;
//	if (messageBuffer == nullptr) {
//		char buffer[100];
//		sprintf_s(buffer, sizeof(buffer), "Unknown error (code: %lu)", errorCode);
//		errorMessage = buffer;
//	}
//	else {
//		errorMessage = messageBuffer;
//		LocalFree(messageBuffer);
//	}
//
//	return errorMessage;
//}



void main()
{
	setlocale(LC_ALL, "");
	cout << "CLIENT" << endl<<endl;
	DWORD dwError = 0;
	CHAR szError[256] = {};

	//1) Инициализация WinSock:
	WSAData wsaData;
	int iResult = 0;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		cout << "WinSOCK init failed with code: " << iResult << endl;
		return;
	}

	//2) Определяем параметры подключения
	addrinfo hints;
	addrinfo* target;
	ZeroMemory(&hints, sizeof(hints));	//обнуляем экземплярн структуры 
	hints.ai_family = AF_INET;			//Стек протоколов TCP/IPv4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;	//Определяем протокол транспортного уровня
	iResult = getaddrinfo("127.0.0.1", "27015", &hints, &target);
	if (iResult != 0)
	{
		cout << "getaddresinfo() failed with code " << iResult << endl;
		WSACleanup();
		return;
	}

	//3) Создаем сокет:
	SOCKET connect_socket = socket(target->ai_family, target->ai_socktype, target->ai_protocol);
	dwError = WSAGetLastError();
	if (connect_socket == INVALID_SOCKET)
	{
		cout << "SOCKET creation failed with error:\t" <<dwError<< /*FormatLastError() <<*/ endl;
		cout << FormatLastError(dwError, szError) << endl;
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//4)Подключение к узлу
	iResult = connect(connect_socket, target->ai_addr, target->ai_addrlen);
	dwError = WSAGetLastError();
	freeaddrinfo(target);
	if (iResult == SOCKET_ERROR)
	{
		//cout << "Error " << dwError << ":\t";
		cout << FormatLastError(dwError, szError)<<endl;
		cout << "Enable to connect to server" << endl;		
		//cout << lpError << endl;
		//cout << "Connect failed with error:\t" << /*FormatLastError() <<*/ endl;
		closesocket(connect_socket);
		WSACleanup();
		return;
	}

	//5) Отправка
	CHAR send_buffer[MTU] = "Hello Server";
	do
	{
		iResult = send(connect_socket, send_buffer, strlen(send_buffer), 0);
		dwError = WSAGetLastError();
		if (iResult == SOCKET_ERROR)
		{
			cout << "Send failed with error: " << WSAGetLastError() << /*FormatLastError() <<*/ endl;
			cout << FormatLastError(dwError, szError) << endl;
			closesocket(connect_socket);
			WSACleanup();
			return;
		}

		//6) Получение данных
		CHAR recv_buffer[MTU] = {};
		do
		{
			iResult = recv(connect_socket, recv_buffer, MTU, 0);
			dwError = WSAGetLastError();
			if (iResult > 0)
				cout << "Bytes received: " << iResult << " Message: " << recv_buffer << endl;
			else if (iResult == 0) cout << "Connection closed" << endl;
			else cout << "Receive failed with " << FormatLastError(dwError, szError) << /*FormatLastError() <<*/ endl;
		} while (iResult > 0);
		cout << "Введите сообщение: ";
		cin.getline(send_buffer,MTU);
	} while (strcmp(send_buffer,"exit")!=0);

	//Закрываем сокет на получение и отправку данных (разрываем TCP-соединение):
	iResult = shutdown(connect_socket, SD_BOTH);
	if (iResult == SOCKET_ERROR)
		cout << "Shutdown failed with "<<FormatLastError(WSAGetLastError(), szError) << /*FormatLastError() <<*/ endl;

	//7)Освобождаем ресурсы WinSOCK
	closesocket(connect_socket);
	WSACleanup();
}

// FORMAT LAST ERROR 1:01:53

