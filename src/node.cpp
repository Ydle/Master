/**
 * Ydle - (http://www.ydle.fr)
 * Home automotion project lowcost and DIY, all questions and information on http://www.ydle.fr
 *
 * @package Master
 * @license CC by SA
 * Autor : Yargol
**/
#include <iostream>
#include <stdio.h>
#include "master.h"
#include "logging.h"


using namespace std;

#define NBSEND 1

 static struct timespec WaitEndCmommand={0,500000000L};

int NodeSendOff(int sender , int target, int id,int* result)
{
	YDLE_DEBUG << "enter in NodeSendOff " << sender << target <<std::endl;

  	//send signal NBSEND time 
	for (int i=0; i<NBSEND; i++)
    {
		g_ProtocolRF->dataToFrame(target,sender, TYPE_CMD);
		g_ProtocolRF->addData(CMD_OFF,id);	// CMD_OFF, output 0 (node)
		g_ProtocolRF->transmit();
		nanosleep(&WaitEndCmommand,NULL); // Delay 400 mili. this prevend lost frame if multi transmit
	}

	*result = 1;
	return 1;
}

int NodeSendOn(int sender , int target,int id, int* result)
{
	YDLE_DEBUG << "enter in NodeSendOn " << std::endl;

  	//send signal NBSEND
	for (int i=0; i<NBSEND; i++)
   	{
		g_ProtocolRF->dataToFrame(target,sender, TYPE_CMD);
		g_ProtocolRF->addCmd(CMD_ON,id);	// CMD_ON, output 0 (node)
		g_ProtocolRF->transmit();
		nanosleep(&WaitEndCmommand,NULL); // Delay 400 mili. this prevend lost frame if multi transmit
	}

	*result = 1;
	return 1;
}

int NodeLink(int sender , int target, int* result)
{
	YDLE_DEBUG << "enter in NodeLink ";

  	//send signal NBSEND
	for (int i=0; i<NBSEND; i++)
   	{
		g_ProtocolRF->dataToFrame(target,sender, TYPE_CMD);
		g_ProtocolRF->addCmd(CMD_LINK,target);	// CMD_ON, output 0 (node)
		g_ProtocolRF->transmit();
		nanosleep(&WaitEndCmommand,NULL); // Delay 400 mili. this prevend lost frame if multi transmit
	}

	*result = 1;
	return 1;
}

int NodeReset(int sender , int target, int* result)
{
	YDLE_DEBUG << "enter in NodeReset " << std::endl;
  	//send signal NBSEND
	for (int i=0; i<NBSEND; i++)
   	{
		g_ProtocolRF->dataToFrame(target,sender, TYPE_CMD);
		g_ProtocolRF->addCmd(CMD_RESET,target);	// CMD_ON, output 0 (node)
		g_ProtocolRF->transmit();
		nanosleep(&WaitEndCmommand,NULL); // Delay 400 mili. this prevend lost frame if multi transmit
	}
	return 1;
}
/*
int NodeList(NodeArray &Result_)
{
	ns3__node *noteptr = new ns3__node[100];
///TODO: pour test only
	for (int i=0;i<100;i++)
	{
		noteptr[i].id=i;
		char sztmp[200];
		sprintf(sztmp,"Name_%d",i);
		noteptr[i].name=soap_strdup(&gsoapserver,sztmp);
	}
	Result_.__ptr=noteptr;
	Result_.__size=100;

	return 1;
}



NodeArray::NodeArray()
{
	__size = 0;
	__ptr = NULL;
}

NodeArray::~NodeArray()
{
	std::cout << "NodeArry destructor" << std::endl;
}

void NodeArray::print()
{
	std::cout << "Print node" << std::endl;

}
*/
