// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2011 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Authors: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>     |
// |          Pavlov Alexandr <teknick@mail.ru>                         |
// +--------------------------------------------------------------------+


#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QMutex>
#include <QHash>
#include "config.h"

#include <QtSql/QSqlQuery>

namespace Database {
        void     init                            ( const QString &file_path );
        bool     connect                         (                          );
        void closeConnection();
        void collectStatsRecord(const QString& name, const QString& value);
        void insertCollected();
        void getRecordById(int id);
        void getRecordByDate(const QString& date);
        void rebuildStatsTable();
        void deleteRecordBy(int id);
        void rebuildStatsTableByType (int type);
        QSqlQuery getRecordsByType(int type);
};

#endif
// vim: set fenc=utf-8 tabstop=8 :
