#pragma once
#include <Windows.h>
#include<iostream>
#include"SMIPC.h"
#define MODE int  //!!!!!!!!!!!!! should be changed depending on your application
class AggregatorFuncs
{
};

HANDLE StartNewThread(MODE mode, SMIPC::SMIPC_OBJ* obj);
void ThreadFunc0(LPVOID cBuff);
void ThreadFunc1(LPVOID cBuff);

void ThreadFunc2(LPVOID cBuff);

void ThreadFunc3(LPVOID cBuff);

void ThreadFunc4(LPVOID cBuff);
