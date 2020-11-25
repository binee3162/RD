#include "Signal.h"
#include "SMIPC.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>
#include <memory.h>

Signal::Signal()
{

	aggreHandle = 0;
	dspHandle = 0;
	transHandle = 0;
	InitSignal();
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
	wchar_t* temp = charToWchar("Signal_DSP");
	dspHandle = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
		sizeof(SM_DSP), temp);
	if (dspHandle == NULL || dspHandle == INVALID_HANDLE_VALUE) {
		free(temp);
		std::cout << "Signal.c: CreateFileMapping error! " << __LINE__ << std::endl;
		return;
	}
	free(temp);
	wchar_t* temp = charToWchar("Signal_Transmitter");
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
