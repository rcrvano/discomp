// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2006 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+

#ifndef PROCESS_H
#define PROCESS_H

#include <QDataStream>
#include <QPluginLoader>
#include <QLibrary>
#include <QFile>
#include <QDir>

#include "config.h"
#include "node.h"
#include "process_private.h"
#include "process_stage.h"
#include "process_controlpoint.h"
#include "process_plugin.h"
#include "lib/module.h"
#include "lib/module_parameter.h"
#include "lib/xml.h"
#include "lib/log.h"
#include "lib/common.h"
#include "lib/file.h"


class DiscompProcessStageModule;
class DiscompProcessStage;
class DiscompProcessControlPoint;
class DiscompProcessPlugin;
/*
class DiscompNode;
class DiscompModule;
class DiscompModuleParameter;
*/

class DiscompProcess : public DiscompProcessPrivate
{
	Q_OBJECT
		
public:
	DiscompProcess(QObject *parent = 0);
	~DiscompProcess();
	
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

	int      get_id() { return this->id; };
	void     set_id( int n ) { this->id = n; };
	QString  get_name() { return this->name; };
	void     set_name( const QString & n ) { this->name = n; };
	QString  get_absolute_process_dir() { return this->absolute_process_dir; };
	int      get_priority() { return this->priority; };
	bool     get_completed() { return this->completed; };
	int      get_current_stage() { return this->current_stage; };
	QString  get_user_name() { return this->user_name; };
	void     set_completed(bool c) { this->completed = c; };
	uint     get_execution_start_time() { return this->execution_start_time; } 



	QString  absolute_process_dir;


	QString  process_parameters_dir;

	int  loadPlugin   ( );
	int  unloadPlugin ( );

	int gotoStage (int stage);
	int jumpToTheStage (int stage);

	int  Init                        ( const QString &process_name, const QString &user_name );
	int  InitSingleTask              ( const QString &process_dir, const QString &user_name );
	
	QString giveJobToNode            ( DiscompNode* node );
	void clearJobOnNode              ( DiscompNode* node );


	void stopModuleByName            ( const QString &module_name );
	void stopListModuleByListNum     ( const QString &module_name, int list_num );

	int  initParametersForNode       ( DiscompNode* node, const QString &module_name, DiscompProcessStageModule* process_module );
	void finishModuleExecutionOnNode ( DiscompNode* node, const QString &module_name );
	void moduleMaxTimeReached        ( DiscompNode* node, const QString &module_name );

	void logMessage                  ( int log_level, QString message );


	int priority;
	int setPriority(int);


	uint     execution_start_time;  //<! time when stage execution start

	QVector<DiscompProcessStage*>    stages; //public for controlpoints. TODO. should be a friend
	QVector<DiscompModuleParameter*> parameters;

public slots:
	//for process plugin signals
	//void pluginLogMessage_slot          ( const QString& ); 
	//void pluginStopModuleByListNum_slot ( int );
	/*
	void jumpToTheStage (int stage);

	void processFreeNode_signal      ( int process_id, int node_id, const QString &module_name );
	void processStopModuleExecutionOnNode_signal ( int node_id, const QString &module_name );

	*/
signals:
	void processFinished_signal      ( int process_id );
	void processFreeNode_signal      ( int process_id, int node_id, const QString &module_name );
	void processStopModuleExecutionOnNode_signal ( int node_id, const QString &module_name );

private:
//	bool                           plugin_loaded;
//	QString                        plugin_filename;
//	QPluginLoader*                 plugin_loader;
//	QObject*                       plugin;
	DiscompProcessPlugin *plugin;
	QString commands_finished;


	bool _initProcessFile    ( const QString &file_path );
	bool _initParametersFile ( const QString &file_path );


	DiscompProcessControlPoint*      control_point;

	void collectStatistic();

	int  checkCurrentStageIsDone             ( );
	void rebuildStageModulesForParallelList  ( );
	void rebuildStageOutputParameters        ( );

	
	int  getParameterNumByName               ( const QString &param_name );
};
 
#endif
// vim: set fenc=utf-8 tabstop=8 :
