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

restLoggerDestination::restLoggerDestination(log_level level, std::string hub_url) : LogDestination(level){
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
		while(!this->messages.empty() && queue_mutex.try_lock()){
			int m_size = this->messages.size();
			log_message m;
			// Récupération des messages
			Json::Value doc;
			Json::Value item;
			for(int i = 0; i < m_size; ++i){
				m = this->messages.front();
				//item[i] = new Json::Value();
				item["level"] = (int)m.level;
				item["message"] = m.log_line;
				doc.append(item);
				this->messages.pop();
			}
			queue_mutex.unlock();
			// Envois groupé des messages
			this->Sending(doc);
		}
		// Si le lock à fonctionné mais que la liste était vide
		queue_mutex.unlock();
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
}

void restLoggerDestination::Sending(Json::Value & message){
	CURL *curl;
	CURLcode res;
	char *error_buf;
	std::stringstream data;
	std::string spdata;
	char response;
	error_buf = (char*)malloc(sizeof(char) * CURL_ERROR_SIZE * 2);
	if(error_buf == NULL){
		return;
	}
	curl = curl_easy_init();
	if(curl) {
		// Build POST request
		std::stringstream request;
		std::string req_param;
		request << "http://" << this->hub_url << "/api/logs/add";
		req_param = request.str();
		Json::FastWriter writer;

		curl_easy_setopt(curl, CURLOPT_URL, req_param.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, writer.write(message).c_str());
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buf);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &ydle::restLoggerDestination::responseCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		res = curl_easy_perform(curl);

		if(res != CURLE_OK){
			std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res);
			std::cout << "Error is :" << error_buf;
		}
		curl_easy_cleanup(curl);
		free(error_buf);
	}
}
size_t restLoggerDestination::responseCallback( char *response, size_t size, size_t nmemb, void *userdata){
	size_t realsize = size * nmemb;
	// Rajouter ici un traitement si erreur
	return realsize;
}

void restLoggerDestination::Sending(log_message & message){
	CURL *curl;
	CURLcode res;
	char *error_buf;
	std::stringstream data;
	std::string spdata;
	char response;
	error_buf = (char*)malloc(sizeof(char) * CURL_ERROR_SIZE * 2);
	if(error_buf == NULL){
		return;
	}
	curl = curl_easy_init();
	if(curl) {
		// Build get request
		std::stringstream request;
		std::string req_param;
		request << "http://" << this->hub_url << "/api/logs/add";
		req_param = request.str();

		char *temp = curl_escape(message.log_line.c_str(), message.log_line.length());
		data << "message=" << temp << "&level=" << message.level << "\r\n\r\n";

		char *form = (char*)malloc(data.str().length() * sizeof(char*));
		strcpy(form, data.str().c_str());

		curl_easy_setopt(curl, CURLOPT_URL, req_param.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, form);
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buf);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &ydle::restLoggerDestination::responseCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

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
	if(level <= this->_level){
		log_message m;
		m.level = level;
		m.log_line = log_line;
		this->queue_mutex.lock();
		this->messages.push(m);
		this->queue_mutex.unlock();
	}
}
} /* namespace ydle */
