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
 * Logging.h
 * Header file for the logging
 * Copyright (C) 2005-2009 Simon Newton
 */
/**
 * @defgroup logging Logging
 * @brief The YDLE logging system.
 *
 * @examplepara
 * ~~~~~~~~~~~~~~~~~~~~~
 * #include <logging.h>
 *
 * // Call this once
 * ydle::InitLogging(ydle::YDLE_LOG_WARN, ydle::YDLE_LOG_STDERR);
 *
 * YDLE_FATAL << "Null pointer!";
 * YDLE_WARN << "Could not connect to server: " << ip_address;
 * YDLE_INFO << "Reading configs from " << config_dir;
 * YDLE_DEBUG << "Counter was " << counter;
 * ~~~~~~~~~~~~~~~~~~~~~
 *
 * @addtogroup logging
 * @{
 *
 * @file Logging.h
 * @brief Header file for YDLE Logging
 */

#ifndef INCLUDE_YDLE_LOGGING_H_
#define INCLUDE_YDLE_LOGGING_H_

#ifdef WIN32
#include <windows.h>  // for HANDLE
#endif

#include <ostream>
#include <string>
#include <sstream>

/**
 * Provide a stream interface to log a message at the specified log level.
 * Rather than calling this directly use one of the YDLE_FATAL, YDLE_WARN,
 * YDLE_INFO or YDLE_DEBUG macros.
 * @param level the log_level to log at.
 */
#define YDLE_LOG(level) (level <= ydle::LogLevel()) && \
                        ydle::LogLine(__FILE__, __LINE__, level).stream()
/**
 * Provide a stream to log a fatal message. e.g.
 * @code
 *     YDLE_FATAL << "Null pointer!";
 * @endcode
 */
#define YDLE_FATAL YDLE_LOG(ydle::YDLE_LOG_FATAL)

/**
 * Provide a stream to log a warning message.
 * @code
 *     YDLE_WARN << "Could not connect to server: " << ip_address;
 * @endcode
 */
#define YDLE_WARN YDLE_LOG(ydle::YDLE_LOG_WARN)

/**
 * Provide a stream to log an infomational message.
 * @code
 *     YDLE_INFO << "Reading configs from " << config_dir;
 * @endcode
 */
#define YDLE_INFO YDLE_LOG(ydle::YDLE_LOG_INFO)

/**
 * Provide a stream to log a debug message.
 * @code
 *     YDLE_DEBUG << "Counter was " << counter;
 * @endcode
 */
#define YDLE_DEBUG YDLE_LOG(ydle::YDLE_LOG_DEBUG)

namespace ydle {

using std::string;

/**
 * @brief The YDLE log levels.
 * This controls the verbosity of logging. Each level also includes those below
 * it.
 */
enum log_level {
  YDLE_LOG_NONE,   /**< No messages are logged. */
  YDLE_LOG_FATAL,  /**< Fatal messages are logged. */
  YDLE_LOG_WARN,   /**< Warnings messages are logged. */
  YDLE_LOG_INFO,   /**< Informational messages are logged. */
  YDLE_LOG_DEBUG,  /**< Debug messages are logged. */
  YDLE_LOG_MAX,
};

/**
 * @private
 * @brief Application global logging level
 */
extern log_level logging_level;

/**
 * @brief The destination to write log messages to
 */
typedef enum {
  YDLE_LOG_STDERR,  /**< Log to stderr. */
  YDLE_LOG_SYSLOG,  /**< Log to syslog. */
  YDLE_LOG_NULL,
} log_output;

/**
 * @class LogDestination
 * @brief The base class for log destinations.
 */
class LogDestination {
 public:
	LogDestination(log_level l);
    /**
     * @brief Destructor
     */
    virtual ~LogDestination() {}

    /**
     * @brief An abstract function for writing to your log destination
     * @note You must over load this if you want to create a new log
     * destination
     */
    virtual void Write(log_level level, const string &log_line) = 0;
    void setLevel(log_level level);
protected:
    ydle::log_level _level;
};

/**
 * @brief A LogDestination that writes to stderr
 */
class StdErrorLogDestination: public LogDestination {
 public:
	StdErrorLogDestination(log_level level);
    /**
     * @brief Writes a messages out to stderr.
     */
    void Write(log_level level, const string &log_line);
};

/**
 * @brief A LogDestination that writes to syslog
 */
class SyslogDestination: public LogDestination {
 public:
	SyslogDestination(log_level l);
    /**
     * @brief Initialize the SyslogDestination
     */
    bool Init();

    /**
     * @brief Write a line to the system logger.
     * @note This is syslog on *nix or the event log on windows.
     */
    void Write(log_level level, const string &log_line);
};

/**@}*/

/**
 * @cond HIDDEN_SYMBOLS
 * @class LogLine
 * @brief A LogLine, this represents a single log message.
 */
class LogLine {
 public:
    LogLine(const char *file, int line, log_level level);
    ~LogLine();
    void Write();

    std::ostream &stream() { return m_stream; }
 private:
    log_level m_level;
    std::ostringstream m_stream;
    unsigned int m_prefix_length;
};
/**@endcond*/

/**
 * @addtogroup logging
 * @{
 */

/**
 * @brief Set the logging level.
 * @param level the new log_level to use.
 */
void SetLogLevel(log_level level);

/**
 * @brief Fetch the current level of logging.
 * @returns the current log_level.
 */
inline log_level LogLevel() { return logging_level; }

/**
 * @brief Increment the log level by one. The log level wraps to YDLE_LOG_NONE.
 */
void IncrementLogLevel();

/**
 * @brief Initialize the YDLE logging system from flags.
 * @pre ParseFlags() must have been called before calling this.
 * @returns true if logging was initialized sucessfully, false otherwise.
 */
bool InitLoggingFromFlags();

/**
 * @brief Initialize the YDLE logging system
 * @param level the level to log at
 * @param output the destintion for the logs
 * @returns true if logging was initialized sucessfully, false otherwise.
 */
bool InitLogging(log_level level, log_output output);

/**
 * @brief Initialize the YDLE logging system using the specified LogDestination.
 * @param level the level to log at
 * @param destination the LogDestination to use.
 * @returns true if logging was initialized sucessfully, false otherwise.
 */
void InitLogging(log_level level, LogDestination *destination);

/***/
}  // namespace ydle
/**@}*/
#endif  // INCLUDE_YDLE_LOGGING_H_
