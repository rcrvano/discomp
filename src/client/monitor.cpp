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

#include <QDateTime>
#include <QDebug>
#include <QProcess>

#include "monitor.h"


#include "lib/common.h"
#include "lib/log.h"
#include "config.h"

#include "client.h"


/**
  * Constructor
  */
DiscompMonitor::DiscompMonitor(DiscompClient *discomp_client) :
    QThread(discomp_client)
{
	m_discomp_client = discomp_client;
	m_sensors = new DiscompMonitorSensors();
	m_rrd = new DiscompXMLRRD();
}

/**
  * Destructor
  */
DiscompMonitor::~DiscompMonitor()
{
	delete m_sensors;
}


/**
 * Set interval for the monitor thread
 * @param interval - integer value for update interval
 */
void DiscompMonitor::startMonitoring ( int interval )
{
    Log::addMessage(LOG_COMMON, "monitor", "Starting system monitor");

    this->timeout_interval = interval;

    this->m_rrd->Init();    

    //init some variables for calculate percents in next time  
    this->getLastInfo();
    this->getLastInfo();
    

    start();
}


/**
 * Start thread
 */
void DiscompMonitor::run()
{
    forever {
        //check system parameters
        Log::addMessage(LOG_COMMON, "monitor", "======================================================");

        QString stat_xml = this->getLastInfo();

	this->m_rrd->setData(stat_xml);
        this->sleep(this->timeout_interval);

	QString xml = "<answer id='200' name='monitoring_data'>\n" + stat_xml + "</answer>";
	m_discomp_client->sendDataToRemoteHost(xml);
    }
}


QString DiscompMonitor::getLastInfo()
{

	QString result_xml  = "<?xml version='1.0' encoding='UTF-8'?>\n"; 

	qint64 start_microtime = QDateTime::currentMSecsSinceEpoch ();
	QString xml = "<stat>\n";

	
	xml += m_sensors->getSysInfo();
	xml += m_sensors->getUptime();
	xml += m_sensors->getLoadAVG();
	xml += m_sensors->getCPUStat();
	xml += m_sensors->getMemory();
	xml += m_sensors->getSwap();
	xml += m_sensors->getFileSystem();
	xml += m_sensors->getNetStat();
//	xml += QString("<g3>%1</g3>\n").arg(QDateTime::currentMSecsSinceEpoch () - start_microtime);
	xml += m_sensors->getExternalSensors();
//	xml += QString("<g4>%1</g4>\n").arg(QDateTime::currentMSecsSinceEpoch () - start_microtime);
/*
	ml += m_sensors->getUsersList();
*/

//	xml += m_sensors->getProcList();
	xml += QString("<microtime>%1</microtime>\n").arg(start_microtime);
	xml += QString("<generation_mtime>%1</generation_mtime>\n").arg(QDateTime::currentMSecsSinceEpoch () - start_microtime);
	xml += "</stat>\n";

	return xml;
	
        //Log::addMessage(LOG_COMMON, "monitor",  QString("------\n%1").arg(xml));
}
