// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+
#include <QProcess>
#include "monitor_jsapi.h"
#include "lib/log.h"

/**
 * @class DiscompMonitorJSAPI
 */

/***********************************************************************/
/*-----------------  Constructor & Destructor  ------------------------*/
/***********************************************************************/
/**
 * Class constructor
 */
DiscompMonitorJSAPI::DiscompMonitorJSAPI(QObject *parent)
{
};

/**
 * Class destructor
 */
DiscompMonitorJSAPI::~DiscompMonitorJSAPI()
{
};


/***********************************************************************/
/*---------------------  DiscompAPI methods ---------------------------*/
/***********************************************************************/


/**
 * Restart module on the current stage
 */
QString DiscompMonitorJSAPI::startProcess ( const QString &cmd )
{
	QProcess process;
	process.start(cmd);
	process.waitForFinished();
	return process.readAll();
};



/**
 * Write log message to the process log file.
 * This method used only from DiscompAPI.logMessage()
 * @param message - log message
 */
void DiscompMonitorJSAPI::logMessage ( const QString& message ) 
{
	Log::addMessage ( LOG_COMMON, "DiscompMonitorJSAPI", "[DiscompAPI] [Log] " + message);
};




/***********************************************************************/
/*-----------------------  Private functions  -------------------------*/
/***********************************************************************/

/**
 * Private function for write log messages from this class
 * @param log_level - log level from lib/log.h
 * @param message - log message 
 */
void DiscompMonitorJSAPI::logMessage ( int log_level, const QString& message ) 
{
	Log::addMessage ( log_level, "DiscompMonitorJSAPI", "[DiscompAPI] " + message);
};



// vim: set fenc=utf-8 tabstop=8 :
