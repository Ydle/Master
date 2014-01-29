/*
 * IhmRestHandler.cpp
 *
 *  Created on: Jan 6, 2014
 *      Author: denia
 */

#include "curl/curl.h"
#include <iostream>
#include <thread>

#include "logging.h"
#include "IhmCommunicationThread.h"
#include "RestBrowser.h"

namespace ydle {

IhmCommunicationThread::IhmCommunicationThread(std::string web_adress, list<protocolRF::Frame_t> *cmd_list, pthread_mutex_t *mutex) {
	this->ListCmd = cmd_list;
	this->mutex_listcmd = mutex;
	this->running = true;
	this->web_address = web_adress;
}

IhmCommunicationThread::~IhmCommunicationThread() {
	// TODO Auto-generated destructor stub
}

void IhmCommunicationThread::run(){
	YDLE_INFO << "Start Communication thread";
	protocolRF::Frame_t frame;
	int size;

	while(this->running){
		pthread_mutex_lock(this->mutex_listcmd);
		size = this->ListCmd->size();
		pthread_mutex_unlock(this->mutex_listcmd);

		if(size > 0){
			for(int i = 0; i < size; i++){
				pthread_mutex_lock(this->mutex_listcmd);
				frame = this->ListCmd->front();
				this->ListCmd->pop_front();
				pthread_mutex_unlock(this->mutex_listcmd);

				if(this->putFrame(frame) == 0){
					// Failed to send the data, so re-insert the frame in the waiting list
					pthread_mutex_lock(this->mutex_listcmd);
					this->ListCmd->push_back(frame);
					pthread_mutex_unlock(this->mutex_listcmd);
				}
			}
		}
		sleep(1);
	}
}
int IhmCommunicationThread::putFrame(protocolRF::Frame_t & frame){
	std::string  post_data;
	std::stringstream buf;
	int value;int type;int sender;

	sender = (int)frame.sender;
	// TODO: Need a better method to get the data
	this->extractData(frame, 1, type, value);
	YDLE_DEBUG << "Data received : From "<< sender << " Type : "<< type << " Value : " << value << "\n";

	RestBrowser browser(this->web_address);
	std::stringstream request;
	request << "/api/node/data";
	buf << "sender=" <<  sender << "&type=" << type << "&data=" << value << "\r\n" ;

	browser.doPost(request.str(), buf.str());
	YDLE_DEBUG << "Url :" << request.str();
	YDLE_DEBUG << "    buf :" << buf.str();

	return 1;
}
void IhmCommunicationThread::start(){
	thread_t = new std::thread(&IhmCommunicationThread::run, this);
}

void IhmCommunicationThread::stop(){
	this->running = false;
}
/*
 * Extract the value from the frame
 * Yes, I know this function should not be here. Denia.
 * */
int IhmCommunicationThread::extractData(protocolRF::Frame_t & frame, int index,int &itype,int &ivalue)
{
	uint8_t* ptr;
	bool bifValueisNegativ=false;
	int iCurrentValueIndex=0;
	bool bEndOfData=false;
	int  iLenOfBuffer = 0;
	int  iModifType=0;
	int  iNbByteRest=0;

	iLenOfBuffer=(int)frame.taille;
	ptr=frame.data;

	if(iLenOfBuffer <2) // Min 1 byte of data with the 1 bytes CRC always present, else there is no data
		return -1;

	while (!bEndOfData)
	{
		itype=(unsigned char)*ptr>>4;
		bifValueisNegativ=false;

		// This is a very ugly code :-( Must do something better
		if(frame.type==TYPE_CMD)
		{
			// Cmd type if always 12 bits signed value
			iModifType=DATA_DEGREEC;
		}
		else if(frame.type==TYPE_ETAT)
		{
			iModifType=itype;
		}
		else
		{
			iModifType=itype;
		}

		switch(iModifType)
		{
		// 4 bits no signed
		case DATA_ETAT :
			ivalue=*ptr&0x0F;
			ptr++;
			iNbByteRest--;
			break;

			// 12 bits signed
		case DATA_DEGREEC:
		case DATA_DEGREEF :
		case DATA_PERCENT :
		case DATA_HUMIDITY:
			if(*ptr&0x8)
				bifValueisNegativ=true;
			ivalue=(*ptr&0x07)<<8;
			ptr++;
			ivalue+=*ptr;
			ptr++;
			if(bifValueisNegativ)
				ivalue=ivalue *(-1);
			iNbByteRest-=2;
			break;

			// 12 bits no signed
		case DATA_DISTANCE:
		case DATA_PRESSION:
			ivalue=(*ptr&0x0F)<<8;
			ptr++;
			ivalue+=*ptr;
			ptr++;
			iNbByteRest-=2;
			break;

			// 20 bits no signed
		case DATA_WATT  :
			ivalue=(*ptr&0x0F)<<16;
			ptr++;
			ivalue+=(*ptr)<<8;
			ptr++;
			ivalue+=*ptr;
			ptr++;
			iNbByteRest-=3;
			break;
		}

		if (index==iCurrentValueIndex)
			return 1;

		iCurrentValueIndex++;

		if(iNbByteRest<1)
			bEndOfData =true;;
	}

	return 0;
}



} /* namespace ydle */
