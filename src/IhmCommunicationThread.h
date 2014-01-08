/*
 * IhmRestHandler.h
 *
 *  Created on: Jan 6, 2014
 *      Author: denia
 */

#ifndef IHMRESTHANDLER_H_
#define IHMRESTHANDLER_H_

#include <map>
#include <list>
#include <vector>
#include <thread>

#include "protocolRF.h"

namespace ydle {

class IhmCommunicationThread {
public:
	IhmCommunicationThread(std::string web_address, list<protocolRF::Frame_t> *cmd_list, pthread_mutex_t *mutex);
	virtual ~IhmCommunicationThread();
	void run();
	void start();
	void stop();
	int putFrame(protocolRF::Frame_t frame);
	int extractData(protocolRF::Frame_t frame, int index,int &itype,int &ivalue);
private:
	list<protocolRF::Frame_t> *ListCmd;
	pthread_mutex_t *mutex_listcmd;
	bool running;
	std::thread *thread_t;
	std::string web_address;
};

} /* namespace ydle */

#endif /* IHMRESTHANDLER_H_ */
