#pragma once
#ifndef SIGNAL_H
#define SIGNAL_H
#include<Windows.h>
#include"SMIPC.h"
#include<memory>
#define MODE int
class Signal
{
public:
	struct SM_Aggregator      //this structure is used to format the information transferred 	//between GUI process and aggregator process							
	{
		MODE			mode;
		int			fs;
		// add whenever you need more....
	};
	struct SM_DSP      //this structure is used to format the information transferred //between GUI process and DSP process				
	{
		MODE			mode;
		int			temp;
		int			func[5]; //assume we only have 5 dsp functions.
		// add whenever you need more....
	};
	struct SM_Transmitter      //this structure is used to format the information transferred//between GUI process and transmitter process				
	{
		MODE			mode;
		int			temp;
		// add whenever you need more....
	};


private:
	HANDLE				aggreHandle;
	HANDLE				dspHandle;
	HANDLE				transHandle;
	SM_Aggregator*  signalAggre;		   //content can be changed using this parameter
	SM_DSP*			signalDSP;			   //content can be changed using this parameter
	SM_Transmitter* signalTrans;		   //content can be changed using this parameter

	std::shared_ptr<SMIPC::Server> SM_Aggre2DSP_Server;
	std::shared_ptr<SMIPC::Server> SM_Trans2DSP_Server;
	std::shared_ptr<SMIPC::Server> SM_DSP2Aggre_Server;
	std::shared_ptr<SMIPC::Server> SM_DSP2Trans_Server;
	SMIPC::Client** ParaMode1and3;
	SMIPC::Client** ParaMode2;
public:	
	std::shared_ptr<SMIPC::Client> SM_Aggre2DSP_Client;
	std::shared_ptr<SMIPC::Client> SM_Trans2DSP_Client;
	std::shared_ptr<SMIPC::Client> SM_DSP2Aggre_Client;
	std::shared_ptr<SMIPC::Client> SM_DSP2Trans_Client;


public:
	Signal();
	~Signal();
	void InitSignal();
	void InitSharedMemory();
	SM_Aggregator*		getSignalAggre();
	SM_DSP*				getSignalDSP();
	SM_Transmitter*		getSignalTrans();

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
	HANDLE StartNewThread(MODE mode);
	void StartNewTHread(MODE mode, HANDLE& thread);
	void ChangeMode(MODE mode);
};
void	ThreadFuncMode0(LPVOID para);
void	ThreadFunc(LPVOID para);  

#endif // !SIGNAL_H