/*
 * NodeRequestHandler.h
 *
 *  Created on: Dec 20, 2013
 *      Author: denia
 */

#include "protocolRF.h"
#include "webServer.h"

#ifndef NODEREQUESTHANDLER_H_
#define NODEREQUESTHANDLER_H_

namespace WebServer {

class NodeRequestHandler  : public HTTPServer::BaseHTTPCallback {
private:
	protocolRF *protocol;

public:
	NodeRequestHandler(protocolRF *p);
	virtual ~NodeRequestHandler();
    int Run(const HTTPRequest *request, HTTPResponse *response);
	int NodeSendOff(int sender , int target, int id,int* result);
	int NodeSendOn(int sender , int target,int id, int* result);
	int NodeLink(int sender , int target, int* result);
	int NodeReset(int sender , int target, int* result);
};

} /* namespace WebServer */

#endif /* NODEREQUESTHANDLER_H_ */
