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


#ifndef DISCOMPMONITOR_H
#define DISCOMPMONITOR_H


#include <QObject>
#include <QThread>
#include "monitor_sensors.h"
#include "../lib/xml_rrd.h"


class DiscompClient;

class DiscompMonitor : QThread
{
    Q_OBJECT
public:
     DiscompMonitor          ( DiscompClient* discomp_client );
    ~DiscompMonitor          (                     );



    void  startMonitoring    ( int interval        );

protected:
    void run           ( );

private:

    QString  getLastInfo  ( );

    int timeout_interval;

    DiscompClient* m_discomp_client;
    DiscompMonitorSensors* m_sensors;
    DiscompXMLRRD* m_rrd;

signals:

public slots:

};

#endif // DISCOMPMONITOR_H
