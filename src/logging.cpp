/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Logging.cpp
 * The logging functions. See include/ydle/Logging.h for details on how to use
 * these.
 * Copyright (C) 2005-2009 Simon Newton
 */

/**
 * @addtogroup logging
 * @{
 *
 * @file Logging.cpp
 *
 * @}
 */
#include <stdio.h>

#include <syslog.h>

#include <iostream>
#include <string>
#include <ctime>
#include <boost/signals2.hpp>
#include <boost/functional.hpp>

#include "logging.h"

//#include "ydle/base/Flags.h"

/**@private*/
//DEFINE_s_int8(log_level, l, ydle::YDLE_LOG_WARN, "Set the logging level 0 .. 4.");
/**@private*/
//DEFINE_bool(syslog, false, "Send to syslog rather than stderr.");

namespace ydle {

using std::string;
using std::ostringstream;

/**
 * @cond HIDDEN_SYMBOLS
 * @brief pointer to a log target
 */
LogDestination *log_target = NULL;
boost::signals2::signal<void (log_level, std::string)> loggers;
log_level logging_level = YDLE_LOG_WARN;
/**@endcond*/

/**
 * @addtogroup logging
 * @{
 */
void SetLogLevel(log_level level) {
  logging_level = level;
}


void IncrementLogLevel() {
  logging_level = (log_level) (logging_level + 1);
  if (logging_level == YDLE_LOG_MAX)
    logging_level = YDLE_LOG_NONE;
}


bool InitLoggingFromFlags() {
  LogDestination *destination;
  if (true) { // FLAGS_syslog
    SyslogDestination *syslog_dest = new SyslogDestination(ydle::YDLE_LOG_NONE);
    if (!syslog_dest->Init()) {
      delete syslog_dest;
      return false;
    }
    destination = syslog_dest;
  } else {
    destination = new StdErrorLogDestination(ydle::YDLE_LOG_WARN);
  }

  log_level log_level = ydle::YDLE_LOG_WARN;
  switch (4) { // FLAGS_log_level
    case 0:
      // nothing is written at this level
      // so this turns logging off
      log_level = ydle::YDLE_LOG_NONE;
      break;
    case 1:
      log_level = ydle::YDLE_LOG_FATAL;
      break;
    case 2:
      log_level = ydle::YDLE_LOG_WARN;
      break;
    case 3:
      log_level = ydle::YDLE_LOG_INFO;
      break;
    case 4:
      log_level = ydle::YDLE_LOG_DEBUG;
      break;
    default :
      break;
  }

  InitLogging(log_level, destination);
  return true;
}


bool InitLogging(log_level level, log_output output) {
  LogDestination *destination;
  if (output == YDLE_LOG_SYSLOG) {
    SyslogDestination *syslog_dest = new SyslogDestination(level);
    if (!syslog_dest->Init()) {
      delete syslog_dest;
      return false;
    }
    destination = syslog_dest;
  } else if (output == YDLE_LOG_STDERR) {
    destination = new StdErrorLogDestination(ydle::YDLE_LOG_WARN);
  } else {
    destination = NULL;
  }
  InitLogging(level, destination);
  return true;
}

void InitLogging(log_level level, LogDestination *destination) {
  SetLogLevel(level);
  loggers.connect(boost::bind(&LogDestination::Write, destination, _1, _2));
}

/**@}*/
/**@cond HIDDEN_SYMBOLS*/
LogLine::LogLine(const char *file,
                 int line,
                 log_level level):
  m_level(level),
  m_stream(ostringstream::out) {
	time_t timer;
	time(&timer);
	struct tm * timeinfo;
	timeinfo = localtime(&timer);

    m_stream << "[" << timeinfo->tm_hour << ":" << timeinfo->tm_min << ":" << timeinfo->tm_sec << "] " << file << ":" << line << ": ";
    m_prefix_length = m_stream.str().length();
}

LogLine::~LogLine() {
  Write();
}

void LogLine::Write() {
  if (m_stream.str().length() == m_prefix_length)
    return;

 /* if (m_level > logging_level)
    return;
*/
  string line = m_stream.str();

  if (line.at(line.length() - 1) != '\n')
    line.append("\n");

  loggers(m_level, line);
}
/**@endcond*/


LogDestination::LogDestination(log_level l){
	this->_level = l;
}

void LogDestination::setLevel(log_level level){
	switch(level){
		case ydle::YDLE_LOG_INFO:
			this->_level = ydle::YDLE_LOG_INFO;
			break;
		case ydle::YDLE_LOG_FATAL:
			this->_level = ydle::YDLE_LOG_FATAL;
			break;
		case ydle::YDLE_LOG_WARN:
			this->_level = ydle::YDLE_LOG_WARN;
			break;
		case ydle::YDLE_LOG_DEBUG:
			this->_level = ydle::YDLE_LOG_DEBUG;
			break;
		default:
			this->_level = ydle::YDLE_LOG_NONE;
			break;
	}
}
/**
 * @addtogroup logging
 * @{
 */
void StdErrorLogDestination::Write(log_level level, const string &log_line) {
	if(level <= this->_level){
		std::cerr << log_line;
		(void) level;
	}
}

StdErrorLogDestination::StdErrorLogDestination(log_level level) : LogDestination(level){
}

SyslogDestination::SyslogDestination(log_level l) : LogDestination(l){};

bool SyslogDestination::Init() {
  return true;
}


void SyslogDestination::Write(log_level level, const string &log_line) {
  int pri;
  switch (level) {
    case YDLE_LOG_FATAL:
      pri = LOG_CRIT;
      break;
    case YDLE_LOG_WARN:
      pri = LOG_WARNING;
      break;
    case YDLE_LOG_INFO:
      pri = LOG_INFO;
      break;
    case YDLE_LOG_DEBUG:
      pri = LOG_DEBUG;
      break;
    default :
      pri = LOG_INFO;
  }
  syslog(pri, "%s", log_line.data());
}
}  // namespace  ydle
/**@}*/
