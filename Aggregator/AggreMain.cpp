#include"SMIPC.h"
#include<iostream>
#include<Windows.h>
#include"AggregatorFuncs.h"

int main()
{
	using namespace std;
	/*cout << "Hello from Aggregator" << endl;
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
	}*/
	auto aggreFuncs = AggregatorFuncs();

	int currentmode =aggreFuncs.signalAggre->mode;    
	HANDLE thread = aggreFuncs.StartNewThread(currentmode);   

	while (1) {
		if (currentmode != aggreFuncs.signalAggre->mode) {
			currentmode = aggreFuncs.signalAggre->mode;
			if (currentmode == END)
				break;
			aggreFuncs.StartNewThread(currentmode, thread);
		}
		Sleep(500);  //check mode change every 0.5s
	}
	cout << "Aggregator exiting" << endl;
	return 0;






}