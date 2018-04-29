#include "stdafx.h"
#include "windows.h"
#include "conio.h"
#include "stdlib.h"
#include "list"

#define min_time 300
#define max_time 1700
#define step 2
#define phil_count 5

using namespace std;

list<int> Threads_sequence;

//event for gathering into threads numbers of philophers
HANDLE iteratorControl = CreateEvent(NULL, TRUE, TRUE, NULL);//manual reset, init signaled
int philosopher_number = 0;

//cs for unbreakable part of philosopher activity
CRITICAL_SECTION philosopherControl;

//events for forks and threads for philosophers
HANDLE some[phil_count];
HANDLE Philosophers[phil_count];

DWORD WINAPI philosopher(LPVOID arg)
{
	//const because somehow this value changes for all threads in random way :D
	const int left_fork = philosopher_number, right_fork = philosopher_number + 1;
	SetEvent(iteratorControl); //this event return control to other threads after getting self number
	for (int i = 0; i < step; i++)
	{
		Sleep(min_time + rand() % (max_time - min_time)); //some delay
		Threads_sequence.push_back(left_fork);
		//cycle for right sequence of threads critical section entering
		while (true)
		{
			if (Threads_sequence.front() == left_fork) { Threads_sequence.pop_back(); break; }
			else continue;
		}
		EnterCriticalSection(&philosopherControl);     //unbreakable piece of activity (start)
		if (WaitForSingleObject(some[left_fork], 1) == WAIT_TIMEOUT)
		{
			if (WaitForSingleObject(some[right_fork], 1) == WAIT_TIMEOUT)
			{
				printf("Philosopher[%i] want to eat. L[%i] n r[%i] forks - success.\n", left_fork, left_fork, right_fork);
				//occupy forks
				SetEvent(some[left_fork]);
				SetEvent(some[right_fork]);
				LeaveCriticalSection(&philosopherControl); //unbreakable piece of activity (stop)
				printf("begin Eating[%i].\n", left_fork);
				Sleep(min_time + rand() % (max_time - min_time)); //some delay
																  //unoccupy forks
				ResetEvent(some[left_fork]);
				ResetEvent(some[right_fork]);
				printf("end   Eating[%i].\n", left_fork);
			}
			else if (WaitForSingleObject(some[right_fork], 1) == WAIT_OBJECT_0)
			{
				printf("Philosopher[%i] want to eat. R[%i] fork - failed.\n", left_fork, right_fork);
			}
		}
		else if (WaitForSingleObject(some[left_fork], 1) == WAIT_OBJECT_0)
		{
			printf("Philosopher[%i] want to eat. L[%i] fork - failed.\n", left_fork, left_fork);
		}
		else if (WaitForSingleObject(some[right_fork], 1) == WAIT_OBJECT_0)
		{
			printf("Philosopher[%i] want to eat. R[%i] fork - failed.\n", left_fork, right_fork);
		}
		else if ((WaitForSingleObject(some[right_fork], 1) == WAIT_OBJECT_0) & (WaitForSingleObject(some[left_fork], 1) == WAIT_OBJECT_0))
		{
			printf("Philosopher[%i] want to eat. L[%i] n r[%i] forks - failed.\n", left_fork, left_fork, right_fork);
		}
		else
		{
			printf("Philosopher[%i] want to eat. Error on [%i].\n", left_fork, left_fork);
		}
		LeaveCriticalSection(&philosopherControl); //unbreakable piece of activity (stop)
		Sleep(min_time + rand() % (max_time - min_time)); //some delay
		printf("Philosopher[%i] thinking.\n", left_fork);
	}
	printf("Philosopher%i EXITING the program. \n", left_fork);
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	InitializeCriticalSection(&philosopherControl);

	for (int i = 0; i < phil_count; i++)
	{
		some[i] = CreateEvent(NULL, TRUE, FALSE, NULL); //FALSE - auto-reset, TRUE - intial state is singaled
		ResetEvent(some[i]);
	}

	for (int i = 0; i < phil_count; i++)
	{
		WaitForSingleObject(iteratorControl, INFINITE);
		ResetEvent(iteratorControl); //lock other threads to catch the right self number
		philosopher_number = i;
		Philosophers[i] = CreateThread(NULL, 0, philosopher, NULL, 0, NULL);
	}

	system("PAUSE");
	return 0;
}
