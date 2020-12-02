
#include"DSPFuncs.h"
HANDLE StartNewThread(MODE mode, SMIPC::SMIPC_OBJ** obj)
{


}
void ThreadFunc1(LPVOID cBuff)
{
	//do someThing...
	char buffer[BLOCK_SIZE];

	SMIPC::Client* temp1 = *(SMIPC::Client**)cBuff;
	SMIPC::Server* temp2 = (SMIPC::Server*) *((SMIPC::SMIPC_OBJ**)cBuff+1);

	while (1)
	{
		ZeroMemory(buffer, BLOCK_SIZE);
		temp1->read((BYTE*)buffer);
		std::cout << "DSP Mode 1 read: " << buffer << std::endl;
		sprintf_s(buffer,"Data processed by DSP mode1,");
		temp2->write((BYTE*)buffer);
		std::cout << "DSP Mode 1 write: " << buffer << std::endl;
	}

}
void ThreadFunc0(LPVOID cBuff)
{
	Sleep(INFINITE);
}
void ThreadFunc2(LPVOID cBuff)
{
	//do someThing...
	char buffer[BLOCK_SIZE];

	SMIPC::Client* temp1 = *(SMIPC::Client**)cBuff;
	SMIPC::Server* temp2 = (SMIPC::Server*) * ((SMIPC::SMIPC_OBJ**)cBuff + 1);

	while (1)
	{
		ZeroMemory(buffer, BLOCK_SIZE);
		temp1->read((BYTE*)buffer);
		std::cout << "DSP Mode 2 read: " << buffer << std::endl;
		sprintf_s(buffer, "Data processed by DSP mode 2,");
		temp2->write((BYTE*)buffer);
		std::cout << "DSP Mode 2 write: " << buffer << std::endl;
	}

}
void ThreadFunc3(LPVOID cBuff)
{
	//do someThing...
}
void ThreadFunc4(LPVOID cBuff)
{
	//do someThing...
}