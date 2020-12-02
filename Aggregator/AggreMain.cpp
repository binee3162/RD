#include"SMIPC.h"
#include<iostream>
#include<Windows.h>
#include"../Signal.h"
#include"AggregatorFuncs.h"
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
	using namespace std;
	cout << "Hello from Aggregator" << endl;
	auto server = SMIPC::Server::Server("Aggre2DSP", 2,true);
	auto client = SMIPC::Client::Client("DSP2Aggre", 2, 1);
	if (GetLastError() != 0) {
		cout << "aggregator failed "  << endl;
		return 1;
	}
	cout << "Aggregator starts successfully " << endl;
	auto temp = charToWchar("Signal_Aggregator");
	auto AggreHandle = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, temp);
	free(temp);
	auto signalAggre = (Signal::SM_Transmitter*)MapViewOfFile(AggreHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Signal::SM_Aggregator));
	if (signalAggre == NULL) {
		std::cout << "Signal.c: Mapping File error! " << __LINE__ << std::endl;
		return 1;
	}
	cout << signalAggre->mode << endl;


	int currentmode = signalAggre->mode;    //or some other mode structure. depends on your application 
	HANDLE thread = StartNewThread(currentmode, (SMIPC::SMIPC_OBJ*) & server);   //master or slave mode should be checked, data here might be server
#define END 10    
	//example...  can be delete afterwards
	while (1) {
		if (currentmode != signalAggre->mode) {
			currentmode = signalAggre->mode;
			if (currentmode == END)
				break;
			
		
		//terminate old thread
		if (client.tryLock()) {
			TerminateThread(thread, 0);
			client.unlock();
		}
		client.unlock();
		TerminateThread(thread, 0);
		thread = StartNewThread(currentmode, (SMIPC::SMIPC_OBJ*) & server);


		}
		Sleep(500);  //check mode change every 0.5s
	}
	cout << "Aggregator exiting" << endl;
	return 0;






}