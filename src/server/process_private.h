// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2006 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+

#ifndef PROCESS_PRIVATE_H
#define PROCESS_PRIVATE_H

#include <QDataStream>
#include <QPluginLoader>
#include <QLibrary>
#include <QFile>
#include <QDir>

#include "config.h"
#include "node.h"



class DiscompProcessPrivate : public QObject
{
	Q_OBJECT
		
public:

	static QString getProcessType           ( const QString &process_name ); 

	DiscompProcessPrivate(QObject *parent = 0);
	~DiscompProcessPrivate();

/*
	int      id;
	QString  name;
	QString  package_name;
	QString  scheme_name;
	QString  log_dir;
	QString  log_file;
	int      process_destructing;
	QString  process_type;
	uint     execution_start_time;  //<! time when stage execution start
	QString  absolute_process_dir;
	QString  process_parameters_dir;
*/
	
	virtual int      get_id() { return  0; };
	virtual void     set_id(int id ) { };
	virtual QString  get_name() { return ""; };
	virtual void     set_name( const QString & name ) { };
	virtual QString  get_absolute_process_dir() { return ""; };
	virtual int      get_priority() { return 0; };
	virtual bool     get_completed() { return 0; };
	virtual int      get_current_stage() { return 0; };
	virtual QString  get_user_name() { return ""; };
	virtual void     set_completed(bool c) { };
	virtual uint     get_execution_start_time() { return 0; };



	virtual int  Init                        ( const QString &process_name, const QString &user_name )  {} ;

	
	virtual QString giveJobToNode            ( DiscompNode* node ) {}
	virtual void clearJobOnNode              ( DiscompNode* node ) {}


	virtual void stopModuleByName            ( const QString &module_name ) {}
	virtual void stopListModuleByListNum     ( const QString &module_name, int list_num ) {}

//	int  initParametersForNode       ( DiscompNode* node, const QString &module_name, DiscompProcessStageModule* process_module );
	virtual void finishModuleExecutionOnNode ( DiscompNode* node, const QString &module_name ) {}
	virtual void moduleMaxTimeReached        ( DiscompNode* node, const QString &module_name ) {}

	virtual void logMessage                  ( int log_level, QString message ) {}


	virtual int setPriority(int) {}



signals:
	virtual void processFinished_signal      ( int process_id );
	virtual void processFreeNode_signal      ( int process_id, int node_id, const QString &module_name );
	virtual void processStopModuleExecutionOnNode_signal ( int node_id, const QString &module_name );

private:


};
 
#endif
// vim: set fenc=utf-8 tabstop=8 :
