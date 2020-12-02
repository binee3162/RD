#include "pch.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>
#include <memory.h>
#include "SMIPC.h"
using namespace std;

// ===================================================================
//								 server
// ===================================================================

SMIPC::Server::Server(const char* addr)
{
	sAddr = NULL;
	hMapFile = 0;
	hSignal = 0;
	hAvail = 0;
	cBuff = 0;
	hRead = 0;
	nrOfClient = 1;
	create(addr);
}

SMIPC::Server::~Server()
{
	free(sAddr);
	close();
}

SMIPC::Server::Server(const char* addr, int nrOfClient)
{
	this->nrOfClient = nrOfClient;
	sAddr = NULL;
	hMapFile = 0;
	hAvail = 0;
	cBuff = 0;
	hSignal = 0;
	hRead = 0;
	hSignals = (HANDLE*)malloc(nrOfClient * sizeof(HANDLE));
	if (hSignals == NULL) {
		cout << "server create: memory error! " << __LINE__ << endl;
		return;
	}
	create(addr, nrOfClient);

}
SMIPC::Server::Server(const char* addr, int nrOfClient, bool useExistingHandle)
{
	if (!useExistingHandle) {
		Server(addr, nrOfClient);
		return;
	}
	this->nrOfClient = nrOfClient;
	sAddr = NULL;
	hMapFile = 0;
	hAvail = 0;
	cBuff = 0;
	hSignal = 0;
	hRead = 0;
	hSignals = (HANDLE*)malloc(nrOfClient * sizeof(HANDLE));
	if (hSignals == NULL) {
		cout << "server create: memory error! " << __LINE__ << endl;
		return;
	}
	create(addr, nrOfClient,true);
}
void SMIPC::Server::create(const char* addr, int nrOfClient)
{
	create(addr);
	bool retflag;
	wchar_t** wcSignals=(wchar_t**)malloc(nrOfClient*sizeof(wchar_t*));
	wchar_t* wcRead;
	createWcharAddr(addr, &wcSignals, &wcRead, nrOfClient, retflag);
	if (retflag) {
		cout << "server create: memory error! " << __LINE__ << endl;
		return;
	}
	//create semphore for all clients
	for (int i = 0; i < nrOfClient; i++) {
		hSignals[i] = CreateSemaphoreW(NULL, 0, BLOCK_COUNT - 1, wcSignals[i]);
		if (hSignals[i] == NULL || hSignals[i] == INVALID_HANDLE_VALUE) {
			free(wcSignals);
			free(wcRead);
			cout << "server create: CreateSemaphore error! " << __LINE__ << endl;
			return;
		}
		free(wcSignals[i]);
	}
	free(wcSignals);
	//Create muteX for client
	hRead = CreateMutexW(nullptr, false, wcRead);
	if (hRead == NULL || hRead == INVALID_HANDLE_VALUE) {
		free(wcRead);
		cout << "server create: CreateSemaphore error! " << __LINE__ << endl;
		return;
	}
	free(wcRead);


}

SMIPC_API void SMIPC::Server::create(const char* addr, int nrOfClient, bool useExistingHandle)
{
	if (!useExistingHandle) {
		create(addr, nrOfClient);
		return;
	}
	bool retflag;
	wchar_t* wcSignal;
	wchar_t* wcMem;
	wchar_t* wcAvail;

	createWcharAddr(addr, &wcSignal, &wcAvail, &wcMem, retflag);
	if (retflag) {
		cout << "server create: memory error! " << __LINE__ << endl;
		return;
	}
	//create semaphore
	
	hAvail = OpenSemaphoreW(SEMAPHORE_ALL_ACCESS, false, wcAvail);
	if (hAvail == NULL || hAvail == INVALID_HANDLE_VALUE) {
		free(wcSignal);
		free(wcMem);
		free(wcAvail);
		cout << "server create: CreateSemaphore error! " << __LINE__ << endl;
		return;
	}
	free(wcAvail);
	//hSignal = OpenSemaphoreW(SEMAPHORE_ALL_ACCESS, false, wcSignal); 
	//if (hSignal == NULL || hSignal == INVALID_HANDLE_VALUE) {
	//	free(wcSignal);
	//	free(wcMem);
	//	cout << "server create: CreateSemaphore error! " << __LINE__ << endl;
	//	return;
	//}
	//free(wcSignal);

	//create file mapping
	hMapFile = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, wcMem);
	if (hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE) {
		free(wcMem);
		cout << "server create: CreateFileMapping error! " << __LINE__ << endl;
		return;
	}
	free(wcMem);

	cBuff = (CBuffer*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(CBuffer));
	if (cBuff == NULL) {

		cout << "server create: Mapping File error! " << __LINE__ << endl;
		return;
	}

	//clear the buffer with all zero
	ZeroMemory(cBuff, sizeof(CBuffer));

	//initialize circular buffer
	//cBuff->size = BLOCK_COUNT;

	cBuff->blocks[0].next = 1;
	cBuff->blocks[0].prev = BLOCK_COUNT - 1;
	cBuff->blocks[BLOCK_COUNT - 1].next = 0;
	cBuff->blocks[BLOCK_COUNT - 1].prev = BLOCK_COUNT - 2;
	for (int i = 1; i < BLOCK_COUNT - 1; i++) {
		cBuff->blocks[i].next = i + 1;
		cBuff->blocks[i].prev = i - 1;
		ZeroMemory(cBuff->blocks[i].Data, BLOCK_SIZE * sizeof(BYTE));
	}

	wchar_t** wcSignals = (wchar_t**)malloc(nrOfClient * sizeof(wchar_t*));
	wchar_t* wcRead;
	createWcharAddr(addr, &wcSignals, &wcRead, nrOfClient, retflag);
	if (retflag) {
		cout << "server create: memory error! " << __LINE__ << endl;
		return;
	}
	//create semphore for all clients
	for (int i = 0; i < nrOfClient; i++) {
		hSignals[i] = OpenSemaphoreW(SEMAPHORE_ALL_ACCESS, false, wcSignals[i]);
		if (hSignals[i] == NULL || hSignals[i] == INVALID_HANDLE_VALUE) {
			free(wcSignals);
			free(wcRead);
			cout << "server create: CreateSemaphore error! " << __LINE__ << endl;
			return;
		}
		free(wcSignals[i]);
	}
	free(wcSignals);
	//Create muteX for client
	hRead = OpenMutexW(MUTEX_ALL_ACCESS, false, wcRead);
	if (hRead == NULL || hRead == INVALID_HANDLE_VALUE) {
		free(wcRead);
		cout << "server create: CreateSemaphore error! " << __LINE__ << endl;
		return;
	}
	free(wcRead);

}

void SMIPC::Server::create(const char* addr)
{
	bool retflag;
	wchar_t* wcSignal;
	wchar_t* wcMem;
	wchar_t* wcAvail;

	createWcharAddr(addr, &wcSignal,&wcAvail, &wcMem, retflag);
	if (retflag) {
		cout << "server create: memory error! " << __LINE__ << endl;
		return;
	}
	//create semaphore
	hAvail	= CreateSemaphoreW(NULL, BLOCK_COUNT - 1, BLOCK_COUNT - 1, wcAvail);
	if (hAvail == NULL || hAvail == INVALID_HANDLE_VALUE) {
		free(wcSignal);
		free(wcMem);
		free(wcAvail);
		cout << "server create: CreateSemaphore error! " << __LINE__ << endl;
		return;
	}
	free(wcAvail);
	hSignal = CreateSemaphoreW(NULL, 0, BLOCK_COUNT - 1, wcSignal);
	if (hSignal == NULL || hSignal == INVALID_HANDLE_VALUE) {
		free(wcSignal);
		free(wcMem);
		cout << "server create: CreateSemaphore error! " << __LINE__ << endl;
		return;
	}
	free(wcSignal);

	//create file mapping
	hMapFile = CreateFileMappingW(INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		sizeof(CBuffer),
		wcMem);
	if (hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE) {
		free(wcMem);
		cout << "server create: CreateFileMapping error! " << __LINE__ << endl;
		return;
	}
	free(wcMem);

	cBuff = (CBuffer*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(CBuffer));
	if (cBuff == NULL) {

		cout << "server create: Mapping File error! " << __LINE__ << endl;
		return;
	}

	//clear the buffer with all zero
	ZeroMemory(cBuff, sizeof(CBuffer));

	//initialize circular buffer
	//cBuff->size = BLOCK_COUNT;

	cBuff->blocks[0].next = 1;
	cBuff->blocks[0].prev = BLOCK_COUNT - 1;
	cBuff->blocks[BLOCK_COUNT - 1].next = 0;
	cBuff->blocks[BLOCK_COUNT - 1].prev = BLOCK_COUNT - 2;
	for (int i = 1; i < BLOCK_COUNT - 1; i++) {
		cBuff->blocks[i].next = i + 1;
		cBuff->blocks[i].prev = i - 1;
		ZeroMemory(cBuff->blocks[i].Data, BLOCK_SIZE * sizeof(BYTE));
	}
}

void SMIPC::Server::close()
{
	// Close the event
	if (hSignal) {
		CloseHandle(hSignal);
	}

	// unmap the memory
	if (cBuff) {
		UnmapViewOfFile(cBuff);
	}

	// close the file handle
	if (hMapFile) {
		CloseHandle(hMapFile);
	}
	if (nrOfClient != 1) {
		for (int i = 0; i < nrOfClient; i++) {
			if (hSignals[i]) {
				CloseHandle(hSignals[i]);
			}
		}
		free(hSignals);
		if (hRead) {
			CloseHandle(hRead);
		}
	}
		
}

SMIPC::Block* SMIPC::Server::getBlock()
{
	if (WaitForSingleObject(hAvail, INFINITE) == WAIT_OBJECT_0)
	{
		Block* b = cBuff->blocks + cBuff->tail;
		cBuff->tail = b->next;
		return b;
	}
	return NULL;
}

void SMIPC::Server::write(BYTE* data)
{
	
		Block* b = getBlock();
		memcpy(b->Data, data, BLOCK_SIZE);
		b->nrOfRead = 0;
	if (nrOfClient == 1) {
		ReleaseSemaphore(hSignal, 1, NULL);
		return;
	}
	else {
		for (int i = 0; i < nrOfClient; i++) {
			ReleaseSemaphore(hSignals[i], 1, NULL);
		}
		return;
	}
	
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

//Create address for signal and mem in wchar format
void SMIPC::Server::createWcharAddr(const char* addr, wchar_t** wcSignal, wchar_t** wcAvail, wchar_t** wcMem, bool& retflag)
{
	if (!addr) {
		cout << "parameter of Server constructer cannot be NULL" << endl;
		return;
	}
	sAddr = (char*)malloc(MAX_ADDR_SIZE);
	if (sAddr == NULL) {
		return;
	}
	ZeroMemory(sAddr, MAX_ADDR_SIZE);
	strcpy_s(sAddr, MAX_ADDR_SIZE, addr);
	retflag = true;


	char* cSignal = (char*)malloc(MAX_ADDR_SIZE);
	if (!cSignal) {
		cout << "server create: memory error! " << __LINE__ << endl;
		free(sAddr);
		return;
	}
	ZeroMemory(cSignal, MAX_ADDR_SIZE);
	sprintf_s(cSignal, MAX_ADDR_SIZE, "%s_signal", sAddr);

	char* cMem = (char*)malloc(MAX_ADDR_SIZE);
	if (!cMem) {
		cout << "server create: memory error! " << __LINE__ << endl;
		free(sAddr);
		free(cSignal);
		return;
	}
	ZeroMemory(cMem, MAX_ADDR_SIZE);
	sprintf_s(cMem, MAX_ADDR_SIZE, "%s_Mem", sAddr);

	char* cAvail = (char*)malloc(MAX_ADDR_SIZE);
	if (!cAvail) {
		cout << "server create: memory error! " << __LINE__ << endl;
		free(sAddr);
		free(cSignal);
		free(cMem);
		return;
	}
	ZeroMemory(cAvail, MAX_ADDR_SIZE);
	sprintf_s(cAvail, MAX_ADDR_SIZE, "%s_Avail", sAddr);

	*wcSignal = charToWchar(cSignal);
	if (!wcSignal) {
		cout << "server create: memory error! " << __LINE__ << endl;
		free(sAddr);
		free(cSignal);
		free(cMem);
		free(cAvail);
		return;
	}
	*wcMem = charToWchar(cMem);
	if (!wcSignal) {
		cout << "server create: memory error! " << __LINE__ << endl;
		free(sAddr);
		free(cSignal);
		free(cMem);
		free(wcSignal);
		free(cAvail);
		return;
	}
	*wcAvail = charToWchar(cAvail);
	if (!wcSignal) {
		cout << "server create: memory error! " << __LINE__ << endl;
		free(sAddr);
		free(cSignal);
		free(cMem);
		free(wcSignal);
		free(wcMem);
		free(cAvail);
		return;
	}
	free(cSignal);
	free(cMem);
	free(cAvail);

	retflag = false;
}
void SMIPC::Server::createWcharAddr(const char* addr, wchar_t*** wcSignals, wchar_t** wcRead, int nrOfClient, bool& retflag)
{
	if (!addr) {
		cout << "parameter of Server constructer cannot be NULL" << endl;
		return;
	}
	sAddr = (char*)malloc(MAX_ADDR_SIZE);
	if (sAddr == NULL) {
		return;
	}
	ZeroMemory(sAddr, MAX_ADDR_SIZE);
	strcpy_s(sAddr, MAX_ADDR_SIZE, addr);
	retflag = true;
	char** cSignals = (char**)malloc(nrOfClient * sizeof(char*));
	if (cSignals == NULL)
		return;
	for (int i = 1; i <= nrOfClient; i++) {
		char* cSignal = (char*)malloc(MAX_ADDR_SIZE);
		if (!cSignal) {
			cout << "server create: memory error! " << __LINE__ << endl;
			free(sAddr);
			return;
		}
		ZeroMemory(cSignal, MAX_ADDR_SIZE);
		sprintf_s(cSignal, MAX_ADDR_SIZE, "%s_signal_%d", sAddr,i);
		cSignals[i - 1] = cSignal;
	}
	char* cRead = (char*)malloc(MAX_ADDR_SIZE);
	if (!cRead) {
		cout << "server create: memory error! " << __LINE__ << endl;
		free(sAddr);
		free(cSignals);
		return;
	}
	ZeroMemory(cRead, MAX_ADDR_SIZE);
	sprintf_s(cRead, MAX_ADDR_SIZE, "%s_Read", sAddr);
	for (int i = 0; i < nrOfClient; i++) {
		(*wcSignals)[i] = charToWchar(cSignals[i]);
		free(cSignals[i]);
	}
	*wcRead = charToWchar(cRead);
	free(cRead);
	free(cSignals);
	retflag = false;

}







// ===================================================================
//								 client 
// ===================================================================

SMIPC::Client::Client(const char* addr)
{
	cAddr = NULL;
	hMapFile = 0;
	hSignal = 0;
	cBuff = 0;
	Create(addr);
}
SMIPC::Client::Client(const char* addr, int nrOfClient, int clientNumber)
{
	cAddr = NULL;
	hMapFile = 0;
	hSignal = 0;
	cBuff = 0;
	this->nrOfClient = nrOfClient;
	Create(addr,clientNumber);
	if(cBuff!=NULL)
		readCur = cBuff->head;
}
SMIPC::Client::~Client()
{
	free(cAddr);
	close();
}
void SMIPC::Client::Create(const char* addr, int clientNumber)
{
	Create(addr);
	bool retflag;
	wchar_t* wcSignal;
	wchar_t* wcRead;
	createWcharAddr(addr, &wcSignal, &wcRead, clientNumber, retflag);
	if (retflag) {
		cout << "server create: memory error! " << __LINE__ << endl;
		return;
	}
	//create semphore for all clients

		hSignal = OpenSemaphoreW(SEMAPHORE_ALL_ACCESS, false, wcSignal);
		if (hSignal == NULL || hSignal == INVALID_HANDLE_VALUE) {
			free(wcSignal);
			free(wcRead);
			cout << "client create: CreateSemaphore error! " << __LINE__ << endl;
			return;
		}
		free(wcSignal);
	
	//Create muteX for client
	hRead = OpenMutexW(MUTEX_ALL_ACCESS, false, wcRead);
	if (hRead == NULL || hRead == INVALID_HANDLE_VALUE) {
		int errCode = GetLastError();
		std::cout << "error code:" << errCode << std::endl;
		free(wcRead);
		cout << "server create: CreateMuteX error! " << __LINE__ << endl;
		return;
	}
	free(wcRead);

}
void SMIPC::Client::Create(const char* addr)
{
	bool retflag;
	wchar_t* wcSignal;
	wchar_t* wcMem;
	wchar_t* wcAvail;
	createWcharAddr(addr, &wcSignal,&wcAvail, &wcMem, retflag);
	if (retflag) {
		cout << "client create: memory error! " << __LINE__ << endl;
		return;
	}
	//create semaphore
	//hSignal = CreateSemaphoreW(NULL, 0, BLOCK_COUNT - 1, wcSignal);
	hAvail = OpenSemaphoreW(SEMAPHORE_ALL_ACCESS, false, wcAvail);
	if (hAvail == NULL || hAvail == INVALID_HANDLE_VALUE) {
		free(wcSignal);
		free(wcAvail);
		free(wcMem);
		cout << "client create: CreateSemaphore error! " << __LINE__ << endl;
		return;
	}
	free(wcAvail);
	hSignal = OpenSemaphoreW(SEMAPHORE_ALL_ACCESS, false, wcSignal);
	if (hSignal == NULL || hSignal == INVALID_HANDLE_VALUE) {
		free(wcSignal);
		free(wcMem);
		cout << "client create: CreateSemaphore error! " << __LINE__ << endl;
		return;
	}
	free(wcSignal);
	//open the shared memory
	hMapFile = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, wcMem);
	if (hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE) {
		free(wcMem);
		cout << "client create: OpenFileMapping error! " << __LINE__ << endl;
		return;
	}
	free(wcMem);
	cBuff = (CBuffer*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(CBuffer));
	if (cBuff == NULL) {

		cout << "client create: Mapping File error! " << __LINE__ << endl;
		return;
	}
}

void SMIPC::Client::close()
{
	// Close the event
	if (hSignal) {
		CloseHandle(hSignal);
	}

	// unmap the memory
	if (cBuff) {
		UnmapViewOfFile(cBuff);
	}

	// close the file handle
	if (hMapFile) {
		CloseHandle(hMapFile);
	}
}


SMIPC::Block* SMIPC::Client::getBlock()
{
	if(WaitForSingleObject(hSignal, INFINITE)==WAIT_OBJECT_0)
	{
		
		if (nrOfClient == 1) {
			Block* b = cBuff->blocks + cBuff->head;
			cBuff->head = b->next;
			return b;
		}
		if (WaitForSingleObject(hRead, INFINITE) == WAIT_OBJECT_0)
		{
			Block* b = cBuff->blocks + readCur;
			readCur = readCur == BLOCK_COUNT - 1 ? 0 : readCur+1;
			if(++(b->nrOfRead)==nrOfClient)
				cBuff->head = b->next;
			ReleaseMutex(hRead);
			return b;
		}
		

	}
	return NULL;
}
void SMIPC::Client::read(BYTE* buffer)
{
	Block* b = getBlock();
	if (b == NULL) return;
	memcpy(buffer, b->Data, BLOCK_SIZE);
	if (nrOfClient == 1) {
		ReleaseSemaphore(hAvail, 1, NULL);
	}
	else {
		if(b->nrOfRead == nrOfClient)
			ReleaseSemaphore(hAvail, 1, NULL);
	}


}
void SMIPC::Client::createWcharAddr(const char* addr, wchar_t** wcSignal, wchar_t** wcAvail, wchar_t** wcMem, bool& retflag)
{
	if (!addr) {
		cout << "parameter of Server constructer cannot be NULL" << endl;
		return;
	}
	cAddr = (char*)malloc(MAX_ADDR_SIZE);
	if (cAddr == NULL) {
		return;
	}
	ZeroMemory(cAddr, MAX_ADDR_SIZE);
	strcpy_s(cAddr, MAX_ADDR_SIZE, addr);
	retflag = true;


	char* cSignal = (char*)malloc(MAX_ADDR_SIZE);
	if (!cSignal) {
		cout << "server create: memory error! " << __LINE__ << endl;
		free(cAddr);
		return;
	}
	ZeroMemory(cSignal, MAX_ADDR_SIZE);
	sprintf_s(cSignal, MAX_ADDR_SIZE, "%s_signal", cAddr);

	char* cMem = (char*)malloc(MAX_ADDR_SIZE);
	if (!cMem) {
		cout << "server create: memory error! " << __LINE__ << endl;
		free(cAddr);
		free(cSignal);
		return;
	}
	ZeroMemory(cMem, MAX_ADDR_SIZE);
	sprintf_s(cMem, MAX_ADDR_SIZE, "%s_Mem", cAddr);

	char* cAvail = (char*)malloc(MAX_ADDR_SIZE);
	if (!cAvail) {
		cout << "server create: memory error! " << __LINE__ << endl;
		free(cAddr);
		free(cSignal);
		free(cMem);
		return;
	}
	ZeroMemory(cAvail, MAX_ADDR_SIZE);
	sprintf_s(cAvail, MAX_ADDR_SIZE, "%s_Avail", cAddr);

	*wcSignal = charToWchar(cSignal);
	if (!wcSignal) {
		cout << "server create: memory error! " << __LINE__ << endl;
		free(cAddr);
		free(cSignal);
		free(cMem);
		free(cAvail);
		return;
	}
	*wcMem = charToWchar(cMem);
	if (!wcSignal) {
		cout << "server create: memory error! " << __LINE__ << endl;
		free(cAddr);
		free(cSignal);
		free(cMem);
		free(wcSignal);
		free(cAvail);
		return;
	}
	*wcAvail = charToWchar(cAvail);
	if (!wcSignal) {
		cout << "server create: memory error! " << __LINE__ << endl;
		free(cAddr);
		free(cSignal);
		free(cMem);
		free(wcSignal);
		free(wcMem);
		free(cAvail);
		return;
	}
	free(cSignal);
	free(cMem);
	free(cAvail);

	retflag = false;
}
SMIPC_API bool SMIPC::Client::tryLock()
{
	if (WaitForSingleObject(hRead, 0) == WAIT_TIMEOUT)
		return false;
	else return true;
}
SMIPC_API void SMIPC::Client::unlock()
{
	ReleaseMutex(hRead);
}
void SMIPC::Client::createWcharAddr(const char* addr, wchar_t** wcSignal, wchar_t** wcRead, int clientNumber, bool& retflag)
{
	if (!addr) {
		cout << "parameter of Server constructer cannot be NULL" << endl;
		return;
	}
	cAddr = (char*)malloc(MAX_ADDR_SIZE);
	if (cAddr == NULL) {
		return;
	}
	ZeroMemory(cAddr, MAX_ADDR_SIZE);
	strcpy_s(cAddr, MAX_ADDR_SIZE, addr);
	retflag = true;
	char* cSignal = (char*)malloc(MAX_ADDR_SIZE);
	if (!cSignal) {
		cout << "server create: memory error! " << __LINE__ << endl;
		free(cAddr);
		return;
	}
	ZeroMemory(cSignal, MAX_ADDR_SIZE);
	sprintf_s(cSignal, MAX_ADDR_SIZE, "%s_signal_%d", cAddr, clientNumber);
	
	char* cRead = (char*)malloc(MAX_ADDR_SIZE);
	if (!cRead) {
		cout << "server create: memory error! " << __LINE__ << endl;
		free(cAddr);
		free(cSignal);
		return;
	}
	ZeroMemory(cRead, MAX_ADDR_SIZE);
	sprintf_s(cRead, MAX_ADDR_SIZE, "%s_Read", cAddr);
	*wcSignal = charToWchar(cSignal);
	free(cSignal);
	*wcRead = charToWchar(cRead);
	free(cRead);
	retflag = false;
}