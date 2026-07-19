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
#include <QtXml>
#include <QThread>
#include "clips_api.h"

class DiscompMonitor : public QThread
{
    Q_OBJECT

public:
     DiscompMonitor          ( QObject* parent = 0 );
    ~DiscompMonitor          (                     );

protected:
    void run();

private:
    void logMessage          ( int log_level, const QString& message );

    DiscompClips *m_discomp_clips;

//    QString recursiveParseData ( QDomNode node, const QString& prefix = "" );

public slots:
//    void processMonitoringDataFromNode_slot (int, const QString &, const QString &);

};

#endif // DISCOMPMONITOR_H
