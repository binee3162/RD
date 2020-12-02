
#include"DSPFuncs.h"


DSPFuncs::DSPFuncs()
{
	using namespace std;
	cout << "Hello from DSP" << endl;
	server1 = make_shared<SMIPC::Server>("DSP2Aggre", 2, true);
	server2 = make_shared<SMIPC::Server>("DSP2Trans", 2, true);
	if (GetLastError() != 0)	return;
	client1 = make_shared<SMIPC::Client>("Aggre2DSP", 2, 1);
	if (GetLastError() != 0) {
		cout << "DSP failed " << endl;
		return ;
	}
	client2 = make_shared<SMIPC::Client>("Trans2DSP", 2, 1);
	if (GetLastError() != 0) {
		cout << "DSP failed " << endl;
		return;
	}
	cout << "DSP starts successfully " << endl;

	auto temp = charToWchar("Signal_DSP");
	DSPHandle = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, temp);
	free(temp);
	signalDSP = (Signal::SM_DSP*)MapViewOfFile(DSPHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Signal::SM_DSP));
	if (signalDSP == NULL) {
		std::cout << "Signal.c: Mapping File error! " << __LINE__ << std::endl;
		return;
	}
}
DSPFuncs::~DSPFuncs()
{
	if (signalDSP)
		UnmapViewOfFile(signalDSP);

	if (DSPHandle)
		CloseHandle(DSPHandle);
}

HANDLE DSPFuncs::StartNewThread(MODE mode)
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

void DSPFuncs::StartNewThread(MODE mode, HANDLE& thread)
{
	//terminate old thread
	TerminateThread(thread, 0);
	thread = StartNewThread(mode);


}

void DSPFuncs::dspExample1(char* buffer)
{
	//do something with data in buffer
	std::cout << "data processed by func 1" << std::endl;

}
void DSPFuncs::dspExample2(char* buffer)
{
	//do something with data in buffer
	std::cout << "data processed by func 2" << std::endl;

}
void DSPFuncs::dspExample3(char* buffer)
{
	//do something with data in buffer
	std::cout << "data processed by func 3" << std::endl;

}
void DSPFuncs::dspExample4(char* buffer)
{
	//do something with data in buffer
	std::cout << "data processed by func 4" << std::endl;

}
void DSPFuncs::dspExample5(char* buffer)
{
	//do something with data in buffer
	std::cout << "data processed by func 5" << std::endl;

}
void DSPFuncs::exFunc(int nr, char* buffer)
{
	switch (nr)
	{
	case 1:
		return dspExample1(buffer);
	case 2:
		return dspExample2(buffer);
	case 3:
		return dspExample3(buffer);
	case 4:
		return dspExample4(buffer);
	case 5:
		return dspExample5(buffer);
	default:
		return;

	}
}





void ThreadFunc1(LPVOID cBuff)
{
	//do someThing...
	char buffer[BLOCK_SIZE];

	//SMIPC::Client* temp1 = *(SMIPC::Client**)cBuff;
	//SMIPC::Server* temp2 = (SMIPC::Server*) *((SMIPC::SMIPC_OBJ**)cBuff+1);
	auto temp = (DSPFuncs*)cBuff;
	auto client =temp->client1;
	auto server = temp->server2;
	auto func = temp->signalDSP->func;

	while (1)
	{
		ZeroMemory(buffer, BLOCK_SIZE);
		client->read((BYTE*)buffer);
		std::cout << "DSP Mode 1 read: " << buffer << std::endl;
		sprintf_s(buffer,"Data processed by DSP mode1,");

		for (int i = 0; i < 5; i++) //assume we have 5 function
		{
			if (*(func + i) == 0)
				break;
			temp->exFunc(*(func + i),buffer);
		}

		server->write((BYTE*)buffer);
		std::cout << "DSP Mode 1 write: " << buffer << std::endl;
	}

}
void ThreadFunc0(LPVOID cBuff)
{
	Sleep(INFINITE);
}
void ThreadFunc2(LPVOID cBuff)
{
	//do someThing...
	char buffer[BLOCK_SIZE];

	//SMIPC::Client* temp1 = *(SMIPC::Client**)cBuff;
	//SMIPC::Server* temp2 = (SMIPC::Server*) *((SMIPC::SMIPC_OBJ**)cBuff+1);
	auto temp = (DSPFuncs*)cBuff;
	auto client = temp->client2;
	auto server = temp->server1;
	auto func = temp->signalDSP->func;


	while (1)
	{
		ZeroMemory(buffer, BLOCK_SIZE);
		client->read((BYTE*)buffer);
		std::cout << "DSP Mode 2 read: " << buffer << std::endl;
		sprintf_s(buffer, "Data processed by DSP mode 2,");
		for (int i = 0; i < 5; i++) //assume we have 5 function
		{
			if (*(func + i) == 0)
				break;
			temp->exFunc(*(func + i), buffer);
		}



		server->write((BYTE*)buffer);
		std::cout << "DSP Mode 2 write: " << buffer << std::endl;
	}

}
void ThreadFunc3(LPVOID cBuff)
{
	//do someThing...
	char buffer[BLOCK_SIZE];

	//SMIPC::Client* temp1 = *(SMIPC::Client**)cBuff;
	//SMIPC::Server* temp2 = (SMIPC::Server*) *((SMIPC::SMIPC_OBJ**)cBuff+1);
	auto temp = (DSPFuncs*)cBuff;
	auto client = temp->client1;
	auto server = temp->server2;
	auto func = temp->signalDSP->func;

	while (1)
	{
		ZeroMemory(buffer, BLOCK_SIZE);
		client->read((BYTE*)buffer);
		std::cout << "DSP Mode 3 read: " << buffer << std::endl;
		sprintf_s(buffer, "Data processed by DSP mode3,");

		for (int i = 0; i < 5; i++) //assume we have 5 function
		{
			if (*(func + i) == 0)
				break;
			temp->exFunc(*(func + i), buffer);
		}


		server->write((BYTE*)buffer);
		std::cout << "DSP Mode 3 write: " << buffer << std::endl;
	}
}
