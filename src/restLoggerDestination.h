/*
 * restLogger.h
 *
 *  Created on: Jan 19, 2014
 *      Author: denia
 */
#include <queue>
#include <mutex>
#include "jsoncpp/json/json.h"
#include "logging.h"

#ifndef RESTLOGGER_H_
#define RESTLOGGER_H_


namespace ydle {

class restLoggerDestination: public LogDestination {
struct log_message{
	std::string log_line;
	log_level level;
};

public:
	restLoggerDestination(log_level level, std::string hub_url);
	static size_t responseCallback( char *response, size_t size, size_t nmemb, void *userdata);
   /**
    * @brief Initialize the restLoggerDestination
    */
   bool Init();

   /**
    * @brief Write a line to the system logger.
    */
   void Write(log_level level, const string &log_line);
   void run();

private:

   void Sending(log_message & message);
   void Sending(Json::Value & message);
   std::queue<log_message> messages;
   bool stop;
   std::string hub_url;
   std::mutex queue_mutex;
};

} /* namespace ydle */

#endif /* RESTLOGGER_H_ */
