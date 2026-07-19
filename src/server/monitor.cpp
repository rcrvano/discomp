// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2011 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Authors: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>     |
// |          Madrahimov Bunyod  <bunyodmadraximov@gmail.com>           |
// |          Pavlov Alexandr <teknick@mail.ru>                         |
// +--------------------------------------------------------------------+

#include "monitor.h"
#include "../lib/log.h"
#include <QThread>


/**
 * @brief Class for for system monitoring
 *
 * This class provide methods for wotks with ...
 * ...
 * ...
 */










/**
  * Constructor
  */
DiscompMonitor::DiscompMonitor(QObject *parent) 
	: QThread( parent )
{
	m_discomp_clips = new DiscompClips();
}

/**
  * Destructor
  */
DiscompMonitor::~DiscompMonitor()
{

}




/**
  * Start functuin
  */
void DiscompMonitor::run()
{
	forever {
		m_discomp_clips->doAnalyse();
		sleep ( 15 );
	}

	exec();
}



        
/**
 * Private function for write log messages from this class
 * @param log_level - log level from lib/log.h
 * @param message - log message 
 */
void DiscompMonitor::logMessage ( int log_level, const QString& message ) 
{
	Log::addMessage (log_level, "DiscompMonitor", message);
};


