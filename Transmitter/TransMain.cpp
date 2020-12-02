#include"SMIPC.h"
#include<iostream>
#include<Windows.h>
#include"../Signal.h"
#include"TransmitterFuncs.h"
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
	/*cout << "Hello from Transmitter" << endl;	
	auto server = SMIPC::Server::Server("Trans2DSP", 2,true);
	auto client = SMIPC::Client::Client("DSP2Trans", 2, 1);
	if (GetLastError() != 0) {
		cout << "transmitter failed " << endl;
		return 1;
	}
	cout << "Transmitter starts successfully " << endl;

	auto temp = charToWchar("Signal_Transmitter");
	auto transHandle = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, temp);
	free(temp);
	auto signalTrans = (Signal::SM_Transmitter*)MapViewOfFile(transHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Signal::SM_Transmitter));
	if (signalTrans == NULL) {
		std::cout << "Signal.c: Mapping File error! " << __LINE__ << std::endl;
		return 1;
	}*/
	auto transFuncs = TransmitterFuncs();
	int currentmode = transFuncs.signalTrans->mode;    //or some other mode structure. depends on your application 
	HANDLE thread = transFuncs.StartNewThread(currentmode);   //master or slave mode should be checked, data here might be server
#define END 10    
	//example...  can be delete afterwards
	while (1) {
		if (currentmode != transFuncs.signalTrans->mode) {
			currentmode = transFuncs.signalTrans->mode;
			//cout << "mode change to "<<signalTrans->mode << endl;
			//break;
			if (currentmode == END)
				break;
			transFuncs.StartNewThread(currentmode, thread);
		}
		Sleep(500);  //check mode change every 0.5s
	}
	cout << "transmitter exiting" << endl;
	return 0;
}