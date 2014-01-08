#ifndef MASTER_H_INCLUDED
#define MASTER_H_INCLUDED


//----------------------------------------------------------------------
//  Project: Ydle
//  Function: xml-rpc server functions
//-------------------------------------------------------------------------
//  Author: Yargol
//
//-------------------------------------------------------------------------
//
//  Copyright 2013
//-------------------------------------------------------------------------
#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <sched.h>
#include <sstream>
#include "protocolRF.h"
#include <vector>
#include <string>
#include <list>


#ifndef RASPBERRY
#define delay(x)
#endif

 void sendDataToIHM(int idNode,std::string data);
 std::string LongToString(long value);
 void AddToListCmd(protocolRF::Frame_t cmd);
 extern protocolRF *g_ProtocolRF;
#endif // MASTER_H_INCLUDED
