#pragma once
#include <Windows.h>
#include<iostream>
#include"SMIPC.h"
#include"../Signal.h"
#define MODE int  //!!!!!!!!!!!!! should be changed depending on your application
#define END 10   
class AggregatorFuncs
{
public:
	AggregatorFuncs();
	~AggregatorFuncs();
	HANDLE StartNewThread(MODE mode);
	void StartNewThread(MODE mode,HANDLE& thread);
	Signal::SM_Aggregator* signalAggre;
	std::shared_ptr<SMIPC::Server> server;
	std::shared_ptr<SMIPC::Client> client;
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
private:

	HANDLE AggreHandle;
	


};


void ThreadFunc0(LPVOID cBuff);
void ThreadFunc1(LPVOID cBuff);

void ThreadFunc2(LPVOID cBuff);

void ThreadFunc3(LPVOID cBuff);

void ThreadFunc4(LPVOID cBuff);
