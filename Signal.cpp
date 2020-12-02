#include "Signal.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>
#include <memory.h>
#include"SMIPC.h"

Signal::Signal()
{
	signalAggre=0;
	signalDSP = 0;
	signalTrans = 0;
	aggreHandle = 0;
	dspHandle = 0;
	transHandle = 0;
	//InitSignal();
	//InitSharedMemory();
}

Signal::~Signal()
{			
	if (signalAggre) 
		UnmapViewOfFile(signalAggre);
	if (aggreHandle)
		CloseHandle(aggreHandle);
	if (signalDSP)
		UnmapViewOfFile(signalDSP);
	if (dspHandle)
		CloseHandle(dspHandle);
	if (signalTrans)
		UnmapViewOfFile(signalTrans);
	if (transHandle)
		CloseHandle(transHandle);
	free(ParaMode1and3);
	free(ParaMode2);

		//SM_Aggre2DSP_Server->close();
		//SM_Trans2DSP_Server->close();
		//SM_DSP2Aggre_Server->close();
		//SM_DSP2Trans_Server->close();
		//SM_Aggre2DSP_Client->close();
		//SM_Trans2DSP_Client->close();
		//SM_DSP2Aggre_Client->close();
		//SM_DSP2Trans_Client->close();
}

void Signal::InitSignal()
{

	//initiate shared memory used for interprocess communication 
	wchar_t* temp = charToWchar("Signal_Aggregator");
	aggreHandle = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
		sizeof(SM_Aggregator),temp );
	if (aggreHandle == NULL || aggreHandle == INVALID_HANDLE_VALUE) {
		free(temp);
		std::cout << "Signal.c: CreateFileMapping error! " << __LINE__ << std::endl;
		return;
	}
	free(temp);
	temp = charToWchar("Signal_DSP");
	dspHandle = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
		sizeof(SM_DSP), temp);
	if (dspHandle == NULL || dspHandle == INVALID_HANDLE_VALUE) {
		free(temp);
		std::cout << "Signal.c: CreateFileMapping error! " << __LINE__ << std::endl;
		return;
	}
	free(temp);
	temp = charToWchar("Signal_Transmitter");
	transHandle = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
		sizeof(SM_DSP), temp);
	if (transHandle == NULL || transHandle == INVALID_HANDLE_VALUE) {
		free(temp);
		std::cout << "Signal.c: CreateFileMapping error! " << __LINE__ << std::endl;
		return;
	}
	free(temp);
	//Mapping memory to instance
	signalAggre = (SM_Aggregator*)MapViewOfFile(aggreHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SM_Aggregator));
	if (signalAggre == NULL) {
		std::cout << "Signal.c: Mapping File error! " << __LINE__ << std::endl;
		return;
	}
	ZeroMemory(signalAggre, sizeof(SM_Aggregator)); //clear with all zeros

	signalDSP = (SM_DSP*)MapViewOfFile(dspHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SM_DSP));
	if (signalDSP == NULL) {
		std::cout << "Signal.c: Mapping File error! " << __LINE__ << std::endl;
		return;
	}
	ZeroMemory(signalDSP, sizeof(SM_DSP)); //clear with all zeros

	signalTrans = (SM_Transmitter*)MapViewOfFile(transHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SM_Transmitter));
	if (signalTrans == NULL) {
		std::cout << "Signal.c: Mapping File error! " << __LINE__ << std::endl;
		return;
	}
	ZeroMemory(signalTrans, sizeof(SM_Transmitter)); //clear with all zeros


}

void Signal::InitSharedMemory()
{
	SM_Aggre2DSP_Server = std::make_shared<SMIPC::Server>("Aggre2DSP", 2);
	SM_Trans2DSP_Server = std::make_shared<SMIPC::Server>("Trans2DSP", 2);
	SM_DSP2Aggre_Server = std::make_shared<SMIPC::Server>("DSP2Aggre", 2);
	SM_DSP2Trans_Server = std::make_shared<SMIPC::Server>("DSP2Trans", 2);
									
	SM_Aggre2DSP_Client = std::make_shared<SMIPC::Client>("Aggre2DSP", 2, 2);
	SM_Trans2DSP_Client = std::make_shared<SMIPC::Client>("Trans2DSP", 2, 2);
	SM_DSP2Aggre_Client = std::make_shared<SMIPC::Client>("DSP2Aggre", 2, 2);
	SM_DSP2Trans_Client = std::make_shared<SMIPC::Client>("DSP2Trans", 2, 2);

	ParaMode1and3 =(SMIPC::Client**) malloc(sizeof(SMIPC::Client*) * 2);
	ParaMode2	 = (SMIPC::Client**) malloc(sizeof(SMIPC::Client*) * 2);

	*ParaMode1and3 = SM_Aggre2DSP_Client.get();
	*(ParaMode1and3 + 1 )= SM_DSP2Trans_Client.get();
	*ParaMode2 = SM_Trans2DSP_Client.get();
	* (ParaMode2 + 1) = SM_DSP2Aggre_Client.get();
};
	
 Signal::SM_Aggregator* Signal::getSignalAggre()
{
	return signalAggre;
}

 Signal::SM_DSP* Signal::getSignalDSP()
 {
	 return signalDSP;
 }

 Signal::SM_Transmitter* Signal::getSignalTrans()
 {
	 return signalTrans;
 }

 void ThreadFuncMode0(LPVOID para)
 {
	 Sleep(INFINITE);
 }
 void ThreadFunc(LPVOID para)
 {
	 //do someThing...
	 char buffer1[BLOCK_SIZE];
	 char buffer2[BLOCK_SIZE];
	 SMIPC::Client* temp1 = *(SMIPC::Client**)para;
	 SMIPC::Client* temp2 = (SMIPC::Client*) * ((SMIPC::Client**)para + 1);   //looks stupid but this is the way...

	 while (1)
	 {
		 ZeroMemory(buffer1, BLOCK_SIZE);
		 ZeroMemory(buffer2, BLOCK_SIZE);
		 temp1->read((BYTE*)buffer1);
		 std::cout << "main thread read 1 : " << buffer1 << std::endl;    //do something like feed data to GUI
		 temp2->read((BYTE*)buffer2);
		 std::cout << "main thread read 2: " << buffer2 << std::endl;
	 }
 }

 HANDLE Signal::StartNewThread(MODE mode)
 {
	 DWORD threadID;
	
	 

	 switch (mode)
	 {
	 case 0:
		 return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFuncMode0, NULL, 0, &threadID);
	 case 1:
		 return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFunc, (LPVOID)ParaMode1and3, 0, &threadID);
	 case 2:
		 return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFunc, (LPVOID)ParaMode2, 0, &threadID);
	 case 3:
		 return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFunc, (LPVOID)ParaMode1and3, 0, &threadID);
	 return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFuncMode0, NULL, 0, &threadID);
	 }
 }

 void Signal::StartNewTHread(MODE mode, HANDLE& thread)
 {
	 TerminateThread(thread, 0);
	 thread = StartNewThread(mode);
 }

 void Signal::ChangeMode(MODE mode)
 {
	 this->getSignalAggre()->mode = mode;
	 this->getSignalDSP()->mode = mode;
	 this->getSignalTrans()->mode = mode;
 }







