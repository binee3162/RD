//This is the a multi client inter-process-communiciation library based on shared memory
//@author: Song Gao
//github: binee3162@gmail.com
//






#pragma once
#ifdef SMIPC_DLL_EXPORTS
#define SMIPC_API __declspec(dllexport)
#else
#define SMIPC_API __declspec(dllimport)
#endif

#include <windows.h>
#include<stdio.h>


#define BLOCK_SIZE			4096
#define BLOCK_COUNT			512
#define	MAX_ADDR_SIZE		128




class SMIPC
{
public:

	//the smallest data unit
	struct Block
	{
		BYTE				Data[BLOCK_SIZE];		//data contains in this block
		LONG				next;
		LONG				prev;
		int					nrOfRead;
	};



	//Circluar buffer contains all the data blocks that is not yet transferred
	struct CBuffer
	{
		Block				blocks[BLOCK_COUNT];
		//int					size;					//size of buffer ie.number of blocks
		volatile LONG		tail;
		volatile LONG		head;

	};	
public:
	class SMIPC_OBJ
	{
	};
public:
	class Server:SMIPC_OBJ
	{
	private:
		char* sAddr;			// Address of this server
		HANDLE				hMapFile;		// Handle to the mapped memory file
		HANDLE				hSignal;		// Handle used to signal when data exists
		//HANDLE				hSignal2;		//Handle used tosignal when data exists if there is 2 client
		HANDLE				hAvail;			// Handle used to signal when some blocks become available
		HANDLE				hRead;			//handle used to perform atomic increase;Used if multiple client exist
		int					nrOfClient=1;
		HANDLE*				hSignals;		//handles used to signal when data exists.Used when  multiple client exist

		//HANDLE				hAvail;			// Handle used to signal when some blocks become available
		CBuffer* cBuff;			// Buffer that points to the shared memory



	public:
		SMIPC_API Server(const char* addr);
		SMIPC_API ~Server();
		SMIPC_API Server(const char* addr, int nrOfClient);
		SMIPC_API Server(const char* addr, int nrOfClient, bool useExistingHandle);//actually create a client that can write
		SMIPC_API DWORD	read(void* cBuffer, DWORD buffSize);
		SMIPC_API char* getAddress(void) { return sAddr; };
		SMIPC_API void	createWcharAddr(const char* addr, wchar_t** wcSignal,wchar_t** wcAvail, wchar_t** wcMem, bool& retflag);
		SMIPC_API void createWcharAddr(const char* addr, wchar_t*** wcSignal, wchar_t** wcRead, int nrOfClient, bool& retflag);
		SMIPC_API Block* getBlock();
		SMIPC_API void	write(BYTE* b);
		SMIPC_API void	create(const char* addr);
		SMIPC_API void	create(const char* addr,int nrOfClient);
		SMIPC_API void	create(const char* addr, int nrOfClient, bool useExistingHandle);
		SMIPC_API void	close();
	};
	class Client :SMIPC_OBJ
	{
	private:
		char* cAddr;			// Address of this client
		int					clientNumber;
		int					nrOfClient=1;
		int					readCur=0;
		HANDLE				hMapFile;		// Handle to the mapped memory file
		HANDLE				hSignal;		// Handle used to signal when data exists
		HANDLE				hRead;			//handle used to perform atomic increase;Used if multiple client exist
		HANDLE				hAvail;			// Handle used to signal when some blocks become available
		CBuffer* cBuff;			// Buffer that points to the shared memory
	public:
		SMIPC_API Client(const char* addr);
		SMIPC_API Client(const char* addr,int nrOfClient,int clientNumber);
		SMIPC_API ~Client();
		SMIPC_API void	Create(const char* addr);
		SMIPC_API void	Create(const char* addr, int clientNumber);
		SMIPC_API void	close();
		SMIPC_API Block* getBlock();
		SMIPC_API void  read(BYTE* buffer);
		SMIPC_API void createWcharAddr(const char* addr, wchar_t** wcSignal, wchar_t** wcRead, int clientNumber, bool& retflag);
		SMIPC_API void	createWcharAddr(const char* addr, wchar_t** wcSignal, wchar_t** wcAvail, wchar_t** wcMem, bool& retflag);
		SMIPC_API bool	tryLock();
		SMIPC_API void	unlock();
	};


};