#include "AggregatorFuncs.h"
#include<Windows.h>
AggregatorFuncs::AggregatorFuncs()
{
	std::cout << "Hello from Aggregator" << std::endl;
	server = std::make_shared< SMIPC::Server>("Aggre2DSP", 2, true);
	client = std::make_shared<SMIPC::Client>("DSP2Aggre", 2, 1);
	if (GetLastError() != 0) {
		std::cout << "aggregator failed " << std::endl;
		return ;
	}
	std::cout << "Aggregator starts successfully " << std::endl;
	auto temp = charToWchar("Signal_Aggregator");
	AggreHandle = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, temp);
	free(temp);
	signalAggre = (Signal::SM_Aggregator*)MapViewOfFile(AggreHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Signal::SM_Aggregator));
	if (signalAggre == NULL) {
		std::cout << "Signal.c: Mapping File error! " << __LINE__ << std::endl;
		return ;
	}


}
AggregatorFuncs::~AggregatorFuncs()
{
	if (signalAggre)
		UnmapViewOfFile(signalAggre);

	if (AggreHandle)
		CloseHandle(AggreHandle);

	//other memory needs to be free

}
HANDLE AggregatorFuncs::StartNewThread(MODE mode)
{
	DWORD threadID;
	switch (mode)
	{
	case 0:
		return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFunc0, NULL, 0, &threadID);
	case 1:
		return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFunc1, (LPVOID)this, 0, &threadID);
	case 2:																		  
		return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFunc2, (LPVOID)this, 0, &threadID);
	case 3:																	
		return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFunc3, (LPVOID)this, 0, &threadID);
	}

}
void AggregatorFuncs::StartNewThread(MODE mode, HANDLE& thread)
{
	//terminate old thread
	TerminateThread(thread, 0);
	thread = StartNewThread(mode);


}
void ThreadFunc0(LPVOID cBuff)
{
	Sleep(INFINITE);
}
void ThreadFunc1(LPVOID cBuff)
{
	//do someThing...
	char buffer[BLOCK_SIZE];

	auto temp = (AggregatorFuncs*)cBuff;
	auto server = temp->server;

	while (1)
	{
		sprintf_s(buffer, "Data From mode 1 aggregator,");
		server->write((BYTE*)buffer);
		std::cout << "Aggregator Mode 1: " << buffer << std::endl;
		Sleep(1500);

	}

}
void ThreadFunc2(LPVOID cBuff)
{
	//do someThing...
	char buffer[BLOCK_SIZE];

	auto temp = (AggregatorFuncs*)cBuff;
	auto client = temp->client;

	while (1)
	{
		
		client->read((BYTE*)buffer);
		std::cout << "Aggregator Mode 2: " << buffer << std::endl;


	}

}
void ThreadFunc3(LPVOID cBuff)
{
	//do someThing...
	char buffer[BLOCK_SIZE];

	auto temp = (AggregatorFuncs*)cBuff;
	auto server = temp->server;

	while (1)
	{
		sprintf_s(buffer, "Data From mode 3 aggregator,");
		server->write((BYTE*)buffer);
		std::cout << "Aggregator Mode 3: " << buffer << std::endl;
		Sleep(1500);

	}
}
