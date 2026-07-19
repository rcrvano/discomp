// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+




#include <QTextStream>
#include <QDateTime>
#include <QHostInfo>
#include <QHash>
#include <QFile>
#include <QDir>
#include <QMutex>
#include <iostream>

#include "log.h"
#include "common.h"
#include "file.h"
#include "config.h"


QMutex log_mutex;
/**
* @brief Logs system
*
*
* This namespace contain functions for make a simple way for write logs messages.
* Evry log message specified by log level mark. Such as LOG_DEBUG, LOG_COMMON, etc.
* For more info about log level see log.h \n
* Logs format is: [date] [level] [from] [message]
*/
namespace Log 
{
	bool addHostnamePrefix  = false;      //!< add to the log filename hostname prefix 
	bool logErrorsOnConsole = false;      //!< display log messages on the console
	bool logVerboseOnConsole= false;      //!< display verbose log messages on the console
	QHash<QString,QString> dublicated_messages; //<! hash for check dublicated messages
	QHash<QString,int> dublicated_messages_count; //<! hash for check dublicated messages

	QString previous_log_message = "";    //!< var for check dublicated messages

	/**
	 * Clear log file directory. This function should be executed when server or client is started
	 */
	void clearLogDir() 
	{
		if ( Config::variables["BaseDirectory"] == "" && Config::variables["LogDirectory"] == "" ) {
			return;
		};
		
		//open log directory
		QString log_dir_path = Log::getLogDirPath();
		
		//don't clear core file
		QString core_file     = QString("%1core.%2").arg(log_dir_path).arg(LOG_FILE_EXT);
		QString tmp_core_file = "";
		if ( QFile::exists ( core_file ) ) {
			tmp_core_file = File::moveToTemporary ( core_file, "core" );
		};

		//clear log dir files
		File::recursiveRemoveDirectoryContent(log_dir_path);

		//move back core log file
		if ( tmp_core_file != "" ) {
			File::move(tmp_core_file, core_file);
		};
	};

	/**
	 * Main function for write log message to the base log file
	 * @param log_level - log level from defined in log.h
	 * @param log_from  - a references from which class has been sent log message. usefull for debug.
	 * @param log_message - it's a log message
	 */
	void addMessage( int log_level, const QString &log_from, const QString &log_message ) 
	{
		Log::addMessage( QString("%1.%2").arg(LOG_FILE).arg(LOG_FILE_EXT), log_level, log_from, log_message);
	};
	
	/**
	 * This function used if log message should be write to the different log file. As example evry node and process have private log file.
	 * @param log_filename - filename to which will be writed log message
	 * @param log_level - log level from defined in log.h
	 * @param log_from  - a references from which class has been sent log message. usefull for debug.
	 * @param log_message - it's a log message
	 */
	void addMessage( const QString &log_filename, int log_level, const QString &log_from, const QString &log_message ) 
	{
		//add message to the error log file
		//if ( log_level == LOG_ERROR ) {
		//	_addMessage( QString("%1.%2").arg(LOG_ERROR_FILE).arg(LOG_FILE_EXT), log_level, log_from, log_message );
		//};

		//add to the specified log file
		if ( log_level < LOG_DEBUG ) {
			_addMessage( log_filename, log_level, log_from, log_message );
		}

		//add message to the debug log file which contain all messages
		QString log_message_tmp = "[" + QString(log_filename).replace(Config::variables["BaseDirectory"],"") + "] " + log_message;
		_addMessage( QString("%1_debug.%2").arg(LOG_FILE).arg(LOG_FILE_EXT), log_level, log_from, log_message_tmp, true );

		if ( log_level == LOG_CORE ) {
			_addMessage( QString("core.%2").arg(LOG_FILE).arg(LOG_FILE_EXT), log_level, log_from, log_message );
			//TODO: here I should get 10 lines from the debug log file.
		};


	};


	/**
	 * Private for write log message to the file. executed from addMessage
	 * @param log_filename - filename to which will be writed log message
	 * @param log_level    - log level from defined in log.h
	 * @param log_from     - a references from which class has been sent log message. usefull for debug.
	 * @param log_message  - it's a log message
	 * @param debug_flag   - add this message to the debug file (log_level higer than LOG_COMMON)
	 */
	void _addMessage( const QString &log_filename, int log_level, const QString &log_from, const QString &log_message, bool debug_flag ) 
	{
		//compose message
		QString message= "";


		log_mutex.lock();

		//check dublicated messages
		if ( log_message == dublicated_messages[log_filename] ) {
			dublicated_messages_count[log_filename]++;
			log_mutex.unlock();
			return;
		};
		if ( dublicated_messages_count[log_filename] > 0 ) {
			message += QString("[Latest message repeated %1 times]\r\n").arg(dublicated_messages_count[log_filename]+1);
		};

			
		dublicated_messages[log_filename] = log_message;
		dublicated_messages_count[log_filename] = 0;;

		//print log on the console. used by discomp communicator
		if ( logErrorsOnConsole || logVerboseOnConsole ) { 
			if ( !log_filename.contains("_debug.") ) {
				if ( log_level == LOG_ERROR ) {
					std::cout << "Error: " << log_message.toStdString() << std::endl;
				} else if ( logVerboseOnConsole ) {
					std::cout << log_message.toStdString() << std::endl;
				};
			};
			//for what? write to the file too
			//return;
		};




		message += QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss:zzz");
		message += " ";
		switch ( log_level ) {
			case LOG_COMMON:
				message += "[common]";
				break;
			case LOG_ERROR:
				message += "[ERROR]";
				break;
			case LOG_NOTICE:
				message += "[notice]";
				break;
			case LOG_WARNING:
				message += "[WARNING]";
				break;
			case LOG_DEBUG:
				message += "[debug]";
				break;
			case LOG_COMMUNICATIONS:
				message += "[protocol]";
				break;
			case LOG_COMMUNICATIONS_VERBOSE:
				message += "[verbose]";
				break;
			case LOG_CORE:
				message += "[CORE]";
				break;
			default:
				message += "[" + QString(log_level) + "]";
				break;
		};


		if ( debug_flag ) {
			message += "[" + log_from + "]: ";
		} else {
			if ( QString(message).length() < 34) {
				QString spaces;
				spaces.fill(' ', (34-message.length()));
				message += spaces;
			};
		};

        	
		message += log_message;
		#ifdef WIN32
		message += "\r\n";
		#else
		message += "\n";
		#endif

		//fixing file paths /home//file -> /home/file
		message.replace( QString(QDir::separator())  + QString(QDir::separator()), QDir::separator() );

	
		
		QString log_file_path = "";

		if ( log_filename != "" && QDir::isAbsolutePath(log_filename) ) {
			log_file_path = log_filename;
		} else {
			QString log_file_dir = "";
			if ( Config::variables["LogDirectory"] != "" ) {
				log_file_dir = Config::variables["LogDirectory"] + QDir::separator();
			} else {
				log_file_dir = Config::variables["BaseDirectory"] + QDir::separator() + "log" + QDir::separator();
			};
		
			if ( addHostnamePrefix ) {
				log_file_dir += QHostInfo::localHostName() + "_";
			};

			if ( log_filename != "" ) {
				log_file_path = log_file_dir + log_filename;
			} else {
				log_file_path = log_file_dir + QString("%1.%2").arg(LOG_FILE).arg(LOG_FILE_EXT);;
			};
		};

		//create directories if needed
		QFileInfo file_info ( log_file_path );
		QString abs_log_dir_path = file_info.absolutePath ();
		QDir abs_log_dir ( abs_log_dir_path );
		if ( !abs_log_dir.exists() ) {
			abs_log_dir.mkpath ( abs_log_dir_path );
		};

		File::append ( log_file_path, message );
		
		log_mutex.unlock();

		return;
	};

	QString getLogDirPath() 
	{
		QString log_dir_path = "";
		if ( Config::variables["LogDirectory"] != "" ) {
			log_dir_path = Config::variables["LogDirectory"] + QDir::separator();
		} else {
			log_dir_path = Config::variables["BaseDirectory"] + QDir::separator() + "log" + QDir::separator();
		}
		return log_dir_path;
	};
};
// vim: set fenc=utf-8 tabstop=8 :
