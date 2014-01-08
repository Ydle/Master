/*
 * NodeRequestHandler.cpp
 *
 *  Created on: Dec 20, 2013
 *      Author: denia
 */
#include "logging.h"
#include "NodeRequestHandler.h"
#include "protocolRF.h"
#include <cstring>
#include <algorithm>
#include <jsoncpp/json/json.h>

namespace WebServer {

#define NBSEND 1

static struct timespec WaitEndCmommand={0,500000000L};

NodeRequestHandler::NodeRequestHandler(protocolRF* p) {
	this->protocol = p;
}

NodeRequestHandler::~NodeRequestHandler() {
	// TODO Auto-generated destructor stub
}

int NodeRequestHandler::NodeSendOff(int sender , int target, int id,int* result)
{
	YDLE_DEBUG << "enter in NodeSendOff " << sender << target << std::endl;

  	//send signal NBSEND time
	for (int i=0; i<NBSEND; i++)
    {
		this->protocol->dataToFrame(target, sender, TYPE_CMD);
		this->protocol->addData(CMD_OFF,id);	// CMD_OFF, output 0 (node)
		this->protocol->transmit();
		nanosleep(&WaitEndCmommand,NULL); // Delay 400 mili. this prevend lost frame if multi transmit
	}

	*result = 1;
	return 1;
}

int NodeRequestHandler::NodeSendOn(int sender , int target,int id, int* result)
{
	YDLE_DEBUG << "enter in NodeSendOn " << std::endl;

  	//send signal NBSEND
	for (int i=0; i<NBSEND; i++)
   	{
		this->protocol->dataToFrame(target, sender, TYPE_CMD);
		this->protocol->addCmd(CMD_ON,id);	// CMD_ON, output 0 (node)
		this->protocol->transmit();
		nanosleep(&WaitEndCmommand,NULL); // Delay 400 mili. this prevend lost frame if multi transmit
	}

	*result = 1;
	return 1;
}

int NodeRequestHandler::NodeLink(int sender , int target, int* result)
{
	YDLE_DEBUG << "enter in NodeLink ";

  	//send signal NBSEND
	for (int i=0; i<NBSEND; i++)
   	{
		this->protocol->dataToFrame(target, sender, TYPE_CMD);
		this->protocol->addCmd(CMD_LINK, target);	// CMD_LINK, output 0 (node)
		this->protocol->transmit();
		nanosleep(&WaitEndCmommand,NULL); // Delay 400 mili. this prevend lost frame if multi transmit
	}

	*result = 1;
	return 1;
}

int NodeRequestHandler::NodeReset(int sender , int target, int* result)
{
	YDLE_DEBUG << "enter in NodeReset " << std::endl;
  	//send signal NBSEND
	for (int i=0; i<NBSEND; i++)
   	{
		this->protocol->dataToFrame(target, sender, TYPE_CMD);
		this->protocol->addCmd(CMD_RESET, target);	// CMD_RESET, output 0 (node)
		this->protocol->transmit();
		nanosleep(&WaitEndCmommand,NULL); // Delay 400 mili. this prevend lost frame if multi transmit
	}
	return 1;
}

int NodeRequestHandler::Run(const HTTPRequest *request, HTTPResponse *response) {
	response->SetContentType(HTTPServer::CONTENT_TYPE_JSON);
	Json::StyledWriter *writer = new Json::StyledWriter();
	Json::Value root;

	string url = request->Url();

	if(std::count(url.begin(), url.end(), '/') > 1){
		// Split the URL
		const char *pch = std::strtok((char*)url.data(), "/" );
		pch = std::strtok(NULL, "/" );
		url = pch;
	}
	int result;

	if(url.compare("on") == 0){
		string nid = request->GetParameter("nid");
		string target = request->GetParameter("target");
		string sender = request->GetParameter("sender");

		if(nid.length() == 0 || target.length() == 0 || sender.length() == 0){
			root["result"] = "ko";
			root["message"] = "a parameter is missing";
		}else{
			this->NodeSendOn(atoi(sender.c_str()), atoi(target.c_str()),atoi(nid.c_str()),  &result);
			root["result"] = "ok";
		}
		response->Append(writer->write(root));
		return response->Send();
	}else if(url.compare("off") == 0){
		string nid = request->GetParameter("nid");
		string target = request->GetParameter("target");
		string sender = request->GetParameter("sender");

		if(nid.length() == 0 || target.length() == 0 || sender.length() == 0){
			root["result"] = "ko";
			root["message"] = "a parameter is missing";
		}else{
			this->NodeSendOff(atoi(sender.c_str()), atoi(target.c_str()),atoi(nid.c_str()),  &result);
			root["result"] = "ok";
		}
		response->Append(writer->write(root));
		return response->Send();
	}else if(url.compare("link") == 0){
		string target = request->GetParameter("target");
		string sender = request->GetParameter("sender");

		if(target.length() == 0 || sender.length() == 0){
			root["result"] = "ko";
			root["message"] = "a parameter is missing";
		}else{
			this->NodeLink(atoi(sender.c_str()), atoi(target.c_str()), &result);
			root["result"] = "ok";
		}
		response->Append(writer->write(root));
		return response->Send();
	}else if(url.compare("reset") == 0){
		string target = request->GetParameter("target");
		string sender = request->GetParameter("sender");

		if(target.length() == 0 || sender.length() == 0){
			root["result"] = "ko";
			root["message"] = "a parameter is missing";
		}else{
			this->NodeReset(atoi(sender.c_str()), atoi(target.c_str()), &result);
			root["result"] = "ok";
		}

		response->Append(writer->write(root));
		return response->Send();
	}else if(url.compare("") != 0){
		root["result"] = "node information";
		response->Append(writer->write(root));
		return response->Send();
	}else{
		root["result"] = "ko";
		root["message"] = "unknown action";
		response->Append(writer->write(root));
		return response->Send();
	}
}

} /* namespace WebServer */
