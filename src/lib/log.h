// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2006 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#ifndef LOG_H
#define LOG_H

#include <QString>
#include "config.h"

//Define log levels
#define LOG_CORE                     0 //!< critical error. execution should be stopped. not used yet
#define LOG_ERROR                    1 //!< if somthing is wrong, but not critical. in this case error will be writed to the standart log file and to the error.log
#define LOG_WARNING                  2 //!< warning message. maybe warnings in the user input files.
#define LOG_COMMON                   3 //!< common log level. 
#define LOG_NOTICE                   4 //!< notice message. for show user to do somthing. maybe update configuration file. etc.
#define LOG_DEBUG                    5 //!< debug level=1. verbose mode. show all program operations
#define LOG_COMMUNICATIONS           6 //!< debug level=2. to the logs will be writed some protocol processing messages.
#define LOG_COMMUNICATIONS_VERBOSE   7 //!< debug level=3. to the logs will be writed all protocol data
#define LOG_CONSOLE                  8 //!< print log on the console


//define globals vars if needed.
#ifndef LOG_FILE
#define LOG_FILE "discomp"
#endif

#ifndef LOG_FILE_EXT
#ifdef Q_OS_UNIX
#define LOG_FILE_EXT "log"
#else        
#define LOG_FILE_EXT "txt"
#endif
#endif


namespace Log {

	extern bool addHostnamePrefix;
	extern bool logErrorsOnConsole;
	extern bool logVerboseOnConsole;

	extern QString previous_log_message;

	void addMessage  (                          int log_level, const QString &log_from, const QString &log_message );
	void addMessage  ( const QString &log_file, int log_level, const QString &log_from, const QString &log_message );

	void _addMessage ( const QString &log_file, int log_level, const QString &log_from, const QString &log_message, bool debug = false );

	void clearLogDir ( );

	QString getLogDirPath();
};

#endif
// vim: set fenc=utf-8 tabstop=8 :
