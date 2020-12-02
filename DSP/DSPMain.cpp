#include"SMIPC.h"
#include<iostream>
#include<Windows.h>
#include"../Signal.h"
#include"DSPFuncs.h"
using namespace std;
wchar_t* charToWchar(const char* str)
{
	if (!str) return NULL;
	size_t length = strlen(str) + 1;
	wchar_t* t = (wchar_t*)malloc(sizeof(wchar_t) * length);
	if (!t) return NULL;
	memset(t, 0, length * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, str, strlen(str), t, length);
	return t;
}
int main()
{
	cout << "Hello from DSP" << endl;
	auto server1 = SMIPC::Server::Server("DSP2Aggre", 2, true);
	auto server2 = SMIPC::Server::Server("DSP2Trans", 2, true);
	if (GetLastError() != 0)	return 1;
	auto client1 = SMIPC::Client::Client("Aggre2DSP", 2, 1);
	if (GetLastError() != 0) {
		cout << "DSP failed " << endl;
		return 1;
	}
	auto client2 = SMIPC::Client::Client("Trans2DSP", 2, 1);
	if (GetLastError() != 0) {
		cout << "DSP failed " << endl;
		return 1;
	}
	cout << "DSP starts successfully " << endl;

	auto temp = charToWchar("Signal_DSP");
	auto transHandle = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, temp);
	free(temp);
	auto signalDSP = (Signal::SM_DSP*)MapViewOfFile(transHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Signal::SM_DSP));
	if (signalDSP == NULL) {
		std::cout << "Signal.c: Mapping File error! " << __LINE__ << std::endl;
		return 1;
	}
	cout << signalDSP->mode << endl;
	int currentmode = signalDSP->mode;    //or some other mode structure. depends on your application 
	SMIPC::SMIPC_OBJ* ThreadParameter[2] = { (SMIPC::SMIPC_OBJ*) & client1,(SMIPC::SMIPC_OBJ*) & server2 };
	HANDLE thread = StartNewThread(currentmode, ThreadParameter);   //master or slave mode should be checked, data here might be server
#define END 10    
	//example...  can be delete afterwards
	while (1) {
		if (currentmode != signalDSP->mode) {
			currentmode = signalDSP->mode;
			cout << "DSP new mode:" << currentmode<<endl;
			if (currentmode == END)
				break;

			//terminate old thread
			if (client1.tryLock()) {
				TerminateThread(thread, 0);
				client1.unlock();
			}
			client1.unlock();
			TerminateThread(thread, 0);
			thread = StartNewThread(currentmode, ThreadParameter);   //master or slave mode should be checked, data here might be server

				//cout << "mode change to "<<signalDSP->mode << endl;
				//break;


			Sleep(500);  //check mode change every 0.5s
		}
		


		
	}
	cout << "DSP exiting" << endl;
	return 0;
}