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


#include "../lib/common.h"
#include "../lib/log.h"
#include "config.h"
#include "database.h"

#include <QtSql/QSqlError>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QVariant>
#include <QVector>


/**
 * @brief Namespace for works with SQLite database
 *
 * This namespace provide methods for wotks with ...
 * ...
 * ...
 */


namespace Database
{
     //QDatabase dbconnection; //!< database connection resouce
    QSqlDatabase db;

    int rec_counter;
    QString collect_sql;

    /**
     * Init database namespace local variables. Should be used like
     * @param file_path     - path to the sqlite database
     * @return The reading status (bool)
     * @sa connect
     */
    void init (const QString &file_path)
    {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(file_path);
        Log::addMessage(LOG_COMMON, "database", QString ("Init database") );

    }

    /**
     * @return status - boolean value of the connect operation
     * @sa init
     */
    bool connect (  )
    {
        bool connected = db.open();
        if (!connected) {
            Log::addMessage(LOG_ERROR, "database", QString ("Database Error: %1").arg(db.lastError().text()) );
            return false;
        }
        Log::addMessage(LOG_COMMON, "database", QString ("Successfully connected") );
        return true;
    }

    void closeConnection()
    {
        db.close();
    }

    void collectStatsRecord(const QString& name, const QString& value)
    {
        if ( name != "mem_free" ) { return; };

        QString sql;
        sql = QString("INSERT INTO stats (DATE, TYPE, NAME, VALUE) VALUES (DATETIME('now'), '%1', '%2', '%3');")
                .arg(1).arg(name).arg(value);

        collect_sql += sql;
    }

    void insertCollected() {
        QSqlQuery query;

        query.prepare(collect_sql);

        query.exec();
        if ( ! query.isActive() ){
            Log::addMessage(LOG_ERROR, "database", QString ("Database Insert Error: %1").arg(db.lastError().text()) );
        }

        rec_counter++;
        rebuildStatsTable ();
    }

    void rebuildStatsTable()
    {
        Log::addMessage(LOG_COMMON, "debug","0");
        rebuildStatsTableByType (1);
        rebuildStatsTableByType (2);
        rebuildStatsTableByType (3);
        /*
        if ( rec_counter % 4 == 0 )  {
            QSqlQuery query = Database::getRecordsByType(1);
            int rows_counter = 0;
            QHash <QString,int> hash_arr;

            while (query.next()) {
                if ( rows_counter == 0 ){
                    hash_arr[name] = 0;
                }
                QString name = query.value('NAME').toString();
                QString value = query.value('VALUE').toString();

                hash_arr[name] += value.toLong();

                rows_counter++;

                if ( rows_counter > 4 ) {
                    Database::deleteRecordBy( query.value('ID').toInt() );
                }
            }

            quint64 srednee = (hash_arr[name] / rows_counter);
            QString sql = QString("INSERT INTO stats (DATE, TYPE, NAME, VALUE) VALUES (DATETIME('now'), '%1', '%2', '%3');")
                    .arg(2).arg('mem_free').arg(srednee);
        }

        */

    }

    void rebuildStatsTableByType (int type)
    {
        Log::addMessage(LOG_COMMON, "debug","1");
        QVector <int> type_calls(8);
        type_calls[1] = 4;
        type_calls[2] = 20;
        type_calls[3] = 60;


        Log::addMessage(LOG_COMMON, "debug","1");
        QVector <int> type_max_rec(8);
        type_max_rec[1] = 4;
        type_max_rec[2] = 15;
        type_max_rec[3] = 60;

        Log::addMessage(LOG_COMMON, "debug","2");
        if ( rec_counter % type_calls[type] == 0 )  {
            Log::addMessage(LOG_COMMON, "debug","4");
            //QSqlQuery query = Database::getRecordsByType(type);


            int rows_counter = 0;
            QHash <QString,int> hash_arr;

            Log::addMessage(LOG_COMMON, "debug","type:" + QString("%1").arg(type));
            Log::addMessage(LOG_COMMON, "debug","6");

            QSqlQuery query = QSqlQuery();
            query.prepare("SELECT * FROM stats WHERE TYPE=:type ORDER BY DATE DESC, NAME");
            query.bindValue(":type", type);
            query.exec();
            while (query.next()) {
                Log::addMessage(LOG_COMMON, "debug","7");
                QString name = query.value('NAME').toString();
                QString value = query.value('VALUE').toString();

                if ( rows_counter == 0 ){
                    hash_arr[name] = 0;
                }


                hash_arr[name] += value.toLong();

                rows_counter++;

                if ( rows_counter > type_max_rec[type] ) {
                    Database::deleteRecordBy( query.value('ID').toInt() );
                }
            }

            Log::addMessage(LOG_COMMON, "debug","8");
            if ( rows_counter > 0 )  {
                double srednee = (hash_arr["mem_free"] / rows_counter);
                QString sql = QString("INSERT INTO stats (DATE, TYPE, NAME, VALUE) VALUES (DATETIME('now'), '%1', '%2', '%3');")
                    .arg(type+1).arg('mem_free').arg(srednee);
                Log::addMessage(LOG_COMMON, "debug","9");
            };
        }
    }

    void getRecordById(int id)
    {
        QSqlQuery query = QSqlQuery();
        query.prepare("SELECT * FROM stats WHERE ID=:id");
        query.bindValue(":id", id);

        //TODO: ..

    }
    void getRecordByDate(const QString& date)
    {
        QSqlQuery query = QSqlQuery();
        query.prepare("SELECT * FROM stats WHERE DATE=:date");
        query.bindValue(":date", date);

        //...
    }

    QSqlQuery getRecordsByType(int type)
    {
        QSqlQuery query = QSqlQuery();
        query.prepare("SELECT * FROM stats WHERE TYPE=:type ORDER BY DATE DESC, NAME");
        query.bindValue(":type", type);
        return query;
    }



    void deleteRecordBy(int id)
    {    
        QSqlQuery query = QSqlQuery();
        query.prepare("DELETE FROM stats WHERE ID='"+QString("1").arg(id)+"'");
        Log::addMessage(LOG_COMMON, "debug", "DELETE FROM stats WHERE ID='"+QString("1").arg(id)+"'");
        query.exec();
        if ( ! query.isActive() ){
            Log::addMessage(LOG_ERROR, "database", QString ("Database Delete Error: %1").arg(db.lastError().text()) );
        }
    };

}
