#pragma once
#include <Windows.h>
#include<iostream>
#include"SMIPC.h"
#include"../Signal.h"
#include<Windows.h>
#define MODE int  //!!!!!!!!!!!!! should be changed depending on your application
#define END 10  
class DSPFuncs
{
public:
	DSPFuncs();
	~DSPFuncs();
	HANDLE StartNewThread(MODE mode);
	void StartNewThread(MODE mode, HANDLE& thread);
	Signal::SM_DSP* signalDSP;
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
	//delete when implempentation starts
	void dspExample1(char* buffer);
	void dspExample2(char* buffer);
	void dspExample3(char* buffer);
	void dspExample4(char* buffer);
	void dspExample5(char* buffer);
	void exFunc(int nr, char* buffer);
private:

	HANDLE DSPHandle;

public:
	std::shared_ptr<SMIPC::Server> server1; //DSP2Aggre
	std::shared_ptr<SMIPC::Server> server2;//DSP2Trans
	std::shared_ptr<SMIPC::Client> client1;//Aggre2DSP
	std::shared_ptr<SMIPC::Client> client2;//Trans2DSP






};

HANDLE StartNewThread(MODE mode, SMIPC::SMIPC_OBJ** obj);
void ThreadFunc0(LPVOID cBuff);
void ThreadFunc1(LPVOID cBuff);

void ThreadFunc2(LPVOID cBuff);

void ThreadFunc3(LPVOID cBuff);

void ThreadFunc4(LPVOID cBuff);
