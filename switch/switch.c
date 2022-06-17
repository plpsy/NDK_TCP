///////////////////////////////////////////////////////////////////////////////
// switch function
// DSP----(p10)(SW20)(p5)-----(p10)(SW40)
// SW20 nodeid(0,12) SW40 nodeid(100+0, 100+12)
// DSP ID = 10
///////////////////////////////////////////////////////////////////////////////

#include <usertype.h>
#include <serrno.h>
#include <stdlib.h>
#include <stdio.h>

#include <errno.h>
#include <string.h>

#include "mytypes.h"
#include "rfd.h"
#include "fetriod.h"

#define SW20_TO_SW40_PORT 5
#define SW20_TO_DSP_PORT 10
#define SW40_TO_SW20_PORT 10
#define SW_TOTOL_PORTS	12

#define SW20_ID 20
#define SW40_ID 40
#define SW20_HOPCOUT 0
#define SW40_HOPCOUT 1
#define SW40_BASE_ID 100

typedef enum {
	_PowerOff = 1,	
	_PowerOn = 0
} NodeStatus;

unsigned char gNodesStatus[256] = {_PowerOff}

Void switchMain(UArg a0, UArg a1)
{
	Task_sleep(1000);
	cfgSWPaths();

	while(1)
	{
		scanNewNodes();
		Task_sleep(1000);
	}
}



Void switchInit()
{
	/* Task 线程 */
    Task_create(switchMain, NULL, NULL);
}


void cfgSW20Route(unsigned char destid, unsigned char port)
{
	UINT32 data;
	data = destid << 24;
	frdMaintWrite(0, 0, SW20_ID, SW20_HOPCOUT, 0x70, ePRIORITY_M, data, 1, 4, 0);
	data = port << 24;
	frdMaintWrite(0, 0, SW20_ID, SW20_HOPCOUT, 0x74, ePRIORITY_M, data, 1, 4, 0);
}

/*
int frdMaintWrite(				frdHandle hMaintWindow,
								unsigned char bLargeTT,
								UINT32 ulDestID,
								unsigned char uchHopCount,
								UINT32 ulOffset,
								ePRIORITY ePriority,
								void* pData,
								UINT32 ulWordCount,
								UINT32 ulWordSize,
								UINT32 ulStride)

int frdMaintRead(				frdHandle hMaintWindow,
								unsigned char bLargeTT,
								UINT32 ulDestID,
								unsigned char uchHopCount,
								UINT32 ulOffset,
								ePRIORITY ePriority,
								void* pData,
								UINT32 ulWordCount,
								UINT32 ulWordSize,
								UINT32 ulStride)
								
*/

void cfgSW40Route(unsigned char destid, unsigned char port)
{
	UINT32 data;
	data = destid << 24;
	frdMaintWrite(0, 0, SW40_ID, SW40_HOPCOUT, 0x70, ePRIORITY_M, data, 1, 4, 0);

	data = port << 24;
	frdMaintWrite(0, 0, SW40_ID, SW40_HOPCOUT, 0x74, ePRIORITY_M, data, 1, 4, 0);

}

void cfgSWPaths()
{
	unsigned char port = 0;
	unsigned char destid = 0;

	// cfg node route connected to SW20 via SW20
	for(port = 0; port < SW_TOTOL_PORTS; port++)
	{
		destid = port;
		cfgSW20Route(destid, port);
	}

	// cfg DSP route to SW40
	cfgSW20Route(SW40_ID, SW20_TO_SW40_PORT);
	// cfg nodes route connected to SW40 via SW20(SW20_TO_SW40_PORT)
	for(destid = SW40_BASE_ID; destid < SW40_BASE_ID + SW_TOTOL_PORTS; destid++)
	{
		cfgSW20Route(destid, SW20_TO_SW40_PORT);
	}

	// cfg nodes route connected to SW40 via SW40
	for(port = 0; port < SW_TOTOL_PORTS; port++)
	{
		destid = SW40_BASE_ID + port
		cfgSW40Route(destid, port);
	}

	// cfg nodes route connected to SW20 via SW40(SW40_TO_SW20_PORT)
	for(destid = 0; destid < SW_TOTOL_PORTS; destid++)
	{
		cfgSW40Route(destid, SW40_TO_SW20_PORT);
	}
}

void disableSW20Port(unsigned char port)
{

}

void enableSW20Port(unsigned char port)
{

}

void disableSW40Port(unsigned char port)
{

}

void enableSW40Port(unsigned char port)
{

}

void scanNewNodes()
{
	unsigned char port = 0;
	unsigned char destid = 0;
	unsigned char uchHopCount = 0;
	UINT32 data = 0;

	// scan SW20
	for(port = 0; port < SW_TOTOL_PORTS; port++)
	{
		if(port == SW20_TO_SW40_PORT || port == SW20_TO_DSP_PORT)
		{
			continue;
		}
		destid = port;
		frdMaintRead(0, 0, 20, SW20_HOPCOUT, 0x158 + 0x20*port, &data, 1, 4, 0);
		if(data&0x2)
		{
			if(gNodesStatus[destid] == _PowerOff)
			{
				enableSW20Port(port);
				cfgSW20Route(0xff, port);
				data = destid<<16|destid;
				frdMaintWrite(0, 0, 0xff, SW20_HOPCOUT+1, 0x60, ePRIORITY_M, data, 1, 4, 0);
				gNodesStatus[destid] = _PowerOn;
			}
		}
		else {
			if(gNodesStatus[destid] == _PowerOn)
			{
				disableSW20Port(port);
				gNodesStatus[destid] = _PowerOff;
			}			
		}
	}

	cfgSW20Route(0xff, SW20_TO_SW40_PORT);
	// scan SW40
	for(port = 0; port < SW_TOTOL_PORTS; port++)
	{
		if(port == SW40_TO_SW20_PORT)
		{
			continue;
		}
		destid = SW40_BASE_ID + port;
		frdMaintRead(0, 0, SW40_ID, SW40_HOPCOUT, 0x158 + 0x20*port, &data, 1, 4, 0);
		if(data&0x2)
		{
			if(gNodesStatus[destid] == _PowerOff)
			{
				enableSW40Port(port);
				cfgSW40Route(0xff, port);
				data = destid<<16|destid;
				uchHopCount = 1;
				frdMaintWrite(0, 0, 0xff, SW40_HOPCOUT+1, 0x60, ePRIORITY_M, data, 1, 4, 0);
				gNodesStatus[destid] = _PowerOn;
			}
		}
		else {
			if(gNodesStatus[destid] == _PowerOn)
			{
				disableSW40Port(port);
				gNodesStatus[destid] = _PowerOff;
			}			
		}
	}
}
