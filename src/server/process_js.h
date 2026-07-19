// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2006 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+

#ifndef PROCESS_JS_H
#define PROCESS_JS_H

#include <QDataStream>
#include <QPluginLoader>
#include <QLibrary>
#include <QFile>
#include <QDir>
#include <QMutex>

#include "config.h"
#include "node.h"
#include "process_private.h"
#include "module_js.h"
#include "parameter_js.h"
#include "js_interpretator.h"

class DiscompModuleJS;



class DiscompProcessJS : public DiscompProcessPrivate
{
	Q_OBJECT
		
public:
	DiscompProcessJS(QObject *parent = 0);
	~DiscompProcessJS();

	
	int      id;
	QString  name;
	QString  user_name;
	QString  package_name;
	QString  scheme_name;
	QString  log_dir;
	QString  log_file;
	bool     completed;
	int      current_stage;
	int      process_destructing;
	QString  process_type;
	bool     EDP;

	QString  absolute_process_dir;

	int      get_id() { return this->id; }
	void     set_id( int n ) { this->id = n; }
	QString  get_name() { return this->name; }
	void     set_name( const QString & n ) { this->name = n; }
	QString  get_absolute_process_dir() { return this->absolute_process_dir; }
	int      get_priority() { return this->priority; }
	bool     get_completed() { return this->isProcessCompleted(); }
	int      get_current_stage() { return this->current_stage; }
	QString  get_user_name() { return this->user_name; }
	void     set_completed(bool c) { this->completed = c; }
	uint     get_execution_start_time() { return this->execution_start_time; } 



	QString  process_parameters_dir;

	int  loadPlugin   ( );
	int  unloadPlugin ( );
	
	int  Init                        ( const QString &process_name, const QString &user_name );
	
	QString giveJobToNode            ( DiscompNode* node );
	void clearJobOnNode              ( DiscompNode* node );


	void stopModuleByName            ( const QString &module_name );
	void stopListModuleByListNum     ( const QString &module_name, int list_num );

	void finishModuleExecutionOnNode ( DiscompNode* node, const QString &module_name );
	void moduleMaxTimeReached        ( DiscompNode* node, const QString &module_name );

	void logMessage                  ( int log_level, QString message );
	void logMessageFromScript ( const QString &message );

	int priority;
	int setPriority(int);

	int  initParametersForNode       ( DiscompNode *node, DiscompModuleJS *module );//TODO code dublication from process.h
	uint     execution_start_time;  //!< time when stage execution start

	QVector <DiscompModuleJS*> modules;
	//QVector<QString>    ready_modules;

	QVector <DiscompModuleParameter*> parameters;
	bool evaluateScript( const QString & script ); //!< evaluate helper script
signals:
	void processFinished_signal      ( int process_id );
	void processFreeNode_signal      ( int process_id, int node_id, const QString &module_name );
	void processStopModuleExecutionOnNode_signal ( int node_id, const QString &module_name );
	void newModulesAvailable_signal ();

public slots:
	void moduleStarted_slot();

private:
	QScriptEngine* process_script_engine;         //!< ECMA Script Engine
	DiscompJSInterpretator interpretator;
	bool _initParametersFile ( const QString &file_path ); //TODO code dublication from process.h
	bool isProcessCompleted ();
	QMutex *unimutex;
};
 
#endif
// vim: set fenc=utf-8 tabstop=8 :
