#include "TransmitterFuncs.h"


TransmitterFuncs::TransmitterFuncs()
{
	using namespace std;
	cout << "Hello from Transmitter" << endl;
	server = make_shared<SMIPC::Server>("Trans2DSP", 2, true);
	client = make_shared<SMIPC::Client>("DSP2Trans", 2, 1);
	if (GetLastError() != 0) {
		cout << "transmitter failed " << endl;
		return ;
	}
	cout << "Transmitter starts successfully " << endl;

	auto temp = charToWchar("Signal_Transmitter");
	transHandle = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, temp);
	free(temp);
	signalTrans = (Signal::SM_Transmitter*)MapViewOfFile(transHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Signal::SM_Transmitter));
	if (signalTrans == NULL) {
		std::cout << " Mapping File error! " << __LINE__ << std::endl;
		return ;
	}

}
TransmitterFuncs::~TransmitterFuncs()
{
	if (signalTrans)
		UnmapViewOfFile(signalTrans);

	if (transHandle)
		CloseHandle(transHandle);

	//other memory needs to be free

}
HANDLE TransmitterFuncs::StartNewThread(MODE mode)
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
void TransmitterFuncs::StartNewThread(MODE mode, HANDLE& thread)
{
	//terminate old thread
	TerminateThread(thread, 0);
	thread = StartNewThread(mode);


}




wchar_t* TransmitterFuncs::charToWchar(const char* str)
{
	if (!str) return NULL;
	size_t length = strlen(str) + 1;
	wchar_t* t = (wchar_t*)malloc(sizeof(wchar_t) * length);
	if (!t) return NULL;
	memset(t, 0, length * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, str, strlen(str), t, length);
	return t;
}


void ThreadFunc0(LPVOID cBuff)
{
	Sleep(INFINITE);
}
void ThreadFunc1(LPVOID cBuff)
{
	//do someThing...
	char buffer[BLOCK_SIZE];

	auto temp = (TransmitterFuncs*)cBuff;
	auto client = temp->client;
	while (1)
	{
		client->read((BYTE*)buffer);
		std::cout << "Transmitter Mode 1: " << buffer << std::endl;


	}

}
void ThreadFunc2(LPVOID cBuff)
{
	//do someThing...
	char buffer[BLOCK_SIZE];

	auto temp = (TransmitterFuncs*)cBuff;
	auto server = temp->server;

	while (1)
	{
		sprintf_s(buffer, "Data From mode 2 Transmitter,");
		server->write((BYTE*)buffer);
		std::cout << "Transmitter Mode 2: " << buffer << std::endl;
		Sleep(1500);
	}
}
void ThreadFunc3(LPVOID cBuff)
{
	//do someThing...
	char buffer[BLOCK_SIZE];

	auto temp = (TransmitterFuncs*)cBuff;
	auto client = temp->client;
	while (1)
	{
		client->read((BYTE*)buffer);
		std::cout << "Transmitter Mode 3: " << buffer << std::endl;


	}
}
