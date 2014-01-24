/*
 * restLogger.cpp
 *
 *  Created on: Jan 19, 2014
 *      Author: denia
 */

#include <string.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <curl/curl.h>

#include "restLoggerDestination.h"

namespace ydle {

restLoggerDestination::restLoggerDestination(std::string hub_url){
	this->hub_url = hub_url;
	this->stop = false;
}

/**
 * @brief: Init function for restLogger
 *
 */
bool restLoggerDestination::Init(){
	this->stop = false;
	std::thread *t = new std::thread(&ydle::restLoggerDestination::run, this);
	return true;
}

void restLoggerDestination::run(){
	while(!this->stop){
		// Si le mutex est libre et qu'il y a des messages
		while(queue_mutex.try_lock() && !this->messages.empty()){
			log_message m;
			m = this->messages.front();
			this->Sending(m);
			this->messages.pop();
			queue_mutex.unlock();
		}
		// Si le lock à fonctionné mais que la liste était vide
		queue_mutex.unlock();
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
}

void restLoggerDestination::Sending(log_message & message){
	CURL *curl;
	CURLcode res;
	char *error_buf;
	std::stringstream data;
	std::string spdata;

	error_buf = (char*)malloc(sizeof(char) * CURL_ERROR_SIZE * 2);
	if(error_buf == NULL){
		return;
	}
	curl = curl_easy_init();
	if(curl) {
		// Build get request
		std::stringstream request;
		std::string req_param;
		request << "http://" << this->hub_url << "/api/log/add";
		req_param = request.str();
		char *temp = curl_escape(message.log_line.c_str(), message.log_line.length());
		data << "message=" << temp << "&level=" << message.level << "\r\n";
		char *form = malloc(data.str().length() * sizeof(char*));
		strcpy(form, data.str().c_str());

		curl_easy_setopt(curl, CURLOPT_URL, req_param.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, form);
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buf);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

		res = curl_easy_perform(curl);

		if(res != CURLE_OK){
			std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res);
			std::cout << "Error is :" << error_buf;
		}
		curl_easy_cleanup(curl);
		free(error_buf);
		free(temp);
		free(form);
	}
}
void restLoggerDestination::Write(log_level level, const string &log_line){
	log_message m;
	m.level = level;
	m.log_line = log_line;
	this->queue_mutex.lock();
	this->messages.push(m);
	this->queue_mutex.unlock();
}
} /* namespace ydle */
