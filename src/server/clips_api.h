// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2011 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Authors: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>     |
// |          Madrahimov Bunyod  <bunyodmadraximov@gmail.com>           |
// +--------------------------------------------------------------------+


#ifndef DISCOMPCLIPS_H
#define DISCOMPCLIPS_H


#include <QObject>
#include <QThread>
#include <QDomNode>

class DiscompClips : QObject
{
    Q_OBJECT
public:
     DiscompClips          ( QObject *parent = 0 );
    ~DiscompClips          (                     );

    void doAnalyse  ( );
    void loadMonitoringFacts();
private:
    void logMessage          ( int log_level, const QString& message );
};

#endif // DISCOMPCLIPS_H
