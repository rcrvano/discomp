// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2006 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#ifndef PROCESSTAGE_H
#define PROCESSTAGE_H

#include <QVector>
#include <QMap>
#include <QtXml>
#include "lib/module.h"


class DiscompProcessStageModule;

class DiscompProcessStage : public QObject
{
	Q_OBJECT
public:
	DiscompProcessStage  (QObject *parent = 0);
	~DiscompProcessStage ( );

	int      Init        ( const QDomNode &stageNode );
    
	int      id;                    //<! stage identificator
	bool     done;                  //<! stage done or not
	QString  log_file;              //<! absolute path to the process log file 
	QString  package_name;          //<! package name
	QString  process_type;
	QString  single_process_dir;

	uint     execution_start_time;  //<! time when stage execution start
	uint     execution_time;        //<! total time in seconds of stage execution

	QMap <QString,QString> events;  //!< Process stages interpreter events (onStart, onFinish, onTimer, etc.)


	QVector<DiscompProcessStageModule*> modules; //<! stage modules
private:
	void logMessage ( int log_level, const QString &message );
};





class DiscompProcessStageModule : public DiscompModule
{

public:
	DiscompProcessStageModule();
	
//	QString name;             //<! module identificator
	int     status;           //<! status of current module ( -wait; 1 - in progress; 2 - done; 3 - stopping;
	int     executed_on_node; //<! identificator of node on which this module is calculating now
	qint64	max_exection_time;//<! max exection time specified for module in the scheme
	QString node_name;        //<! name of the node on which module was calculated
	qint64  node_ping;        //<! ping to the node on which module was calculated

	bool list_flag;        //<! this module works with element of list (true or false)
	int  list_parameter;   //<! parameter id of list parameter 
	int  list_element_num; //<! element of list which should calculate this module 

};
 
#endif
// vim: set fenc=utf-8 tabstop=8 :
