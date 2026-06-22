#include<iostream>
#include<Windows.h>
using namespace std;

bool finish = false;


VOID Function()
{
	while (!finish)
	{
		cout << "Hello Threads from " << GetCurrentThreadId() << endl;
		//system("PAUSE");
	}
}

struct Point
{
	Point(int x,int y):x(x),y(y){}
	int x;
	int y;
};
VOID Collision(Point* point)
{
	while (point->x != point->y)
	{
		cout << "X= " << point->x++ << "\tY= " << point->y-- << endl;
		Sleep(10);
	}
}

//#define WINDOWS_THREADS_1
#define WINDOWS_THREADS_2

void main()
{
	setlocale(LC_ALL, "");

#ifdef WINDOWS_THREADS_1
	
	DWORD dwID = 0;
	HANDLE hThread = CreateThread
	(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)Function,
		NULL,
		NULL,
		&dwID
	);
	cin.get();		//ожидает нажатия Enter
	finish = true;
	cout << "Thread ID from main(): " << dwID << endl;
	WaitForSingleObject(hThread, INFINITE);
#endif // WINDOWS_THREADS_1

	Point A(0, 1000);
	DWORD dwThreadID = 0;
	HANDLE hThread = CreateThread
	(
		NULL,
		NULL,
		(LPTHREAD_START_ROUTINE)Collision,
		(LPVOID)&A,
		NULL,
		&dwThreadID
	);
	WaitForSingleObject(hThread,INFINITE);
}