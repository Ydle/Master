/*
 * restLogger.h
 *
 *  Created on: Jan 19, 2014
 *      Author: denia
 */
#include <queue>
#include <mutex>
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
	restLoggerDestination(std::string hub_url);


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
   std::queue<log_message> messages;
   bool stop;
   std::string hub_url;
   std::mutex queue_mutex;
};

} /* namespace ydle */

#endif /* RESTLOGGER_H_ */
