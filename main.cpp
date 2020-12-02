#include<iostream>
#include<Windows.h>
#include<chrono>
#include"Signal.h"
#include"SMIPC.h"
using namespace std;


void startChildProcess(bool& errorFlag)
{
	errorFlag = true;
	STARTUPINFO si_aggre;
	PROCESS_INFORMATION pi_aggre;
	char path[100] = ".\\Debug\\Aggregator.exe";
	ZeroMemory(&si_aggre, sizeof(si_aggre));
	si_aggre.cb = sizeof(si_aggre);
	ZeroMemory(&pi_aggre, sizeof(pi_aggre));
	// Start the child process.
	if (!CreateProcess(NULL,   // No module name (use command line)
		path,      // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory
		&si_aggre,            // Pointer to STARTUPINFO structure
		&pi_aggre)           // Pointer to PROCESS_INFORMATION structure
		)
	{
		std::cout << "Create Aggregator Process failed: " << GetLastError() << __LINE__ << std::endl;
		return ;
	}
	//start DSP process
	STARTUPINFO si_dsp;
	PROCESS_INFORMATION pi_dsp;
	char path1[100] = ".\\Debug\\DSP.exe";
	ZeroMemory(&si_dsp, sizeof(si_dsp));
	si_dsp.cb = sizeof(si_dsp);
	ZeroMemory(&pi_dsp, sizeof(pi_dsp));
	// Start the child process.
	if (!CreateProcess(NULL,path1, NULL,NULL,FALSE,0,NULL,NULL,&si_dsp,&pi_dsp))
	{
		std::cout << "Create DSP Process failed: " << GetLastError() << __LINE__ << std::endl;
		return;
	}
	//start Transmitter process
	STARTUPINFO si_trans;
	PROCESS_INFORMATION pi_trans;
	char path2[100] = ".\\Debug\\Transmitter.exe";
	ZeroMemory(&si_trans, sizeof(si_trans));
	si_trans.cb = sizeof(si_trans);
	ZeroMemory(&pi_trans, sizeof(pi_trans));
	// Start the child process.
	if (!CreateProcess(NULL, path2, NULL, NULL, FALSE, 0, NULL, NULL, &si_trans, &pi_trans))
	{
		std::cout << "Create Transmitter Process failed: " << GetLastError() << __LINE__ << std::endl;
		return;
	}
	errorFlag = false;
}



int main()
{
	HANDLE listenerThread;
	auto signal=Signal();
	signal.InitSignal();
	signal.InitSharedMemory();
	//set mode. This step should be down in GUI.
	//example mode setting 

	//signal.getSignalAggre()->mode = 1;
	//signal.getSignalDSP()->mode = 1;
	//signal.getSignalTrans()->mode = 1;
	// startChildProcess
	bool errFlag;
	MODE mode = 0;

	startChildProcess(errFlag);
	if (errFlag) {
		std::cout << "error when start ChildProcess" << endl;
		return 1;
	}
	cout << "successful!" << endl;

	listenerThread = signal.StartNewThread(mode);


	while (1) {
		MODE premode = mode;
		cin >>mode;
		if (premode != mode) {
			signal.StartNewTHread(mode, listenerThread);
			signal.ChangeMode(mode);
		}
		MODE funcComb = 0;
		cin >> funcComb;
		if (funcComb == 1) {
			signal.getSignalDSP()->func[0] = 1;
			signal.getSignalDSP()->func[1] = 2;
		}
		else if (funcComb = 2) {
			signal.getSignalDSP()->func[0] = 3;
			signal.getSignalDSP()->func[1] = 2;
		}
		else {
			signal.getSignalDSP()->func[0] = 3;
			signal.getSignalDSP()->func[1] = 2;
			signal.getSignalDSP()->func[3] = 1;
			signal.getSignalDSP()->func[4] = 5;
		}
		
		Sleep(1000);

	}



	//auto client1 = SMIPC::Client::Client("TEST", 2, 2);
	////std::cout << server.getAddress() << std::endl;
	//cout << "test..." << endl;
	//int n = 0;
	//int i = 0;
	//char buffer1[BLOCK_SIZE];
	//while (n++ != 30000) {
	//	
	//	//SMIPC::Block* b = server.getBlock();
	//	//strcpy_s(buffer, BLOCK_SIZE, (char*)b->Data);
	//	client1.read((BYTE*)buffer1);
	//	if (!strcmp(buffer1, "" + n))
	//		i++;
	//	ZeroMemory(buffer1, BLOCK_SIZE);
	//	//cout << buffer << endl;
	//	//cout << n << endl;
	//}


	//std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
	//	std::chrono::system_clock::now().time_since_epoch()
	//	);
	//cout << ms.count() << endl;
	//cout << "error: " << i << endl;
	return 0;
}
