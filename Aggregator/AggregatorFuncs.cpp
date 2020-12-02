#include "AggregatorFuncs.h"
HANDLE StartNewThread(MODE mode, SMIPC::SMIPC_OBJ* obj)
{
	DWORD threadID;
	switch (mode)
	{
	case 0:
		return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFunc0, (LPVOID)obj, 0, &threadID);
	case 1:
		return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFunc1, (LPVOID)obj, 0, &threadID);
	case 2:
		return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFunc2, (LPVOID)obj, 0, &threadID);
	case 3:
		return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFunc3, (LPVOID)obj, 0, &threadID);
	}

}
void ThreadFunc0(LPVOID cBuff)
{
	Sleep(INFINITE);
}
void ThreadFunc1(LPVOID cBuff)
{
	//do someThing...
	char buffer[BLOCK_SIZE];

	SMIPC::Server* temp = (SMIPC::Server*)cBuff;

	while (1)
	{
		sprintf_s(buffer, "Data From mode 1 aggregator,");
		temp->write((BYTE*)buffer);
		std::cout << "Aggregator Mode 1: " << buffer << std::endl;
		Sleep(1500);

	}

}
void ThreadFunc2(LPVOID cBuff)
{
	//do someThing...
	char buffer[BLOCK_SIZE];

	SMIPC::Server* temp = (SMIPC::Server*)cBuff;

	while (1)
	{
		sprintf_s(buffer, "Data From mode 2 aggregator,");
		temp->write((BYTE*)buffer);
		std::cout << "Aggregator Mode 2: " << buffer << std::endl;
		Sleep(1500);

	}

}
void ThreadFunc3(LPVOID cBuff)
{
	//do someThing...
}
void ThreadFunc4(LPVOID cBuff)
{
	//do someThing...
}