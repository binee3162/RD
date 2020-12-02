#pragma once

#include <Windows.h>
#include<iostream>
#include"SMIPC.h"
#include"../Signal.h"
#define MODE int  //!!!!!!!!!!!!! should be changed depending on your application
#define END 10   
class TransmitterFuncs
{
public:
	TransmitterFuncs();
	~TransmitterFuncs();
	HANDLE StartNewThread(MODE mode);
	void StartNewThread(MODE mode, HANDLE& thread);
	Signal::SM_Transmitter* signalTrans;
	std::shared_ptr<SMIPC::Server> server;
	std::shared_ptr<SMIPC::Client> client;
	wchar_t* charToWchar(const char* str);

private:

	HANDLE transHandle;




};
HANDLE StartNewThread(MODE mode, SMIPC::SMIPC_OBJ* obj);
void ThreadFunc0(LPVOID cBuff);

void ThreadFunc1(LPVOID cBuff);

void ThreadFunc2(LPVOID cBuff);

void ThreadFunc3(LPVOID cBuff);

