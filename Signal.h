#pragma once
class Signal
{
public:
	struct SM_Aggregator      //this structure is used to format the information transferred
								//between GUI process and aggregator process
	{
		int			mode;
		int			fs;
		// add whenever you need more....
	};
	struct SM_DSP      //this structure is used to format the information transferred
							//between GUI process and DSP process
	{
		int			mode;
		int			temp;
		// add whenever you need more....
	};
	struct SM_Transmitter      //this structure is used to format the information transferred
								//between GUI process and transmitter process
	{
		int			mode;
		int			temp;
		// add whenever you need more....
	};

private:
	HANDLE				aggreHandle;
	HANDLE				dspHandle;
	HANDLE				transHandle;
	SM_Aggregator*		signalAggre;		   //content can be changed using this parameter
	SM_DSP*				signalDSP;			   //content can be changed using this parameter
	SM_Transmitter*		signalTrans;		   //content can be changed using this parameter

public:
	Signal();
	void InitSignal();
	SM_Aggregator*		getSignalAggre();
	SM_DSP*				getSignalDSP();
	SM_Transmitter*		getSignalTrans();

};

