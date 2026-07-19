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

class DiscompClips : QObject
{
    Q_OBJECT
public:
     DiscompClips          ( QObject *parent = 0 );
    ~DiscompClips          (                     );

    void processClipsFile  ( );

};

#endif // DISCOMPCLIPS_H
