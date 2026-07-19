// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#ifndef MODULE_H
#define MODULE_H

#include <QVector>
#include <QProcess>
#include <QTimer>
#include <QString>
#include <QtXml>
#include <QDateTime>
#include <QTime>

#include "config.h"


#define MODULE_STATUS_WAIT 0          //<! module not executed yet
#define MODULE_STATUS_DONE 1          //<! module execution finished
#define MODULE_STATUS_STOPED 2        //<! module execution stoped (max time reached, etc)
#define MODULE_STATUS_FAILED 3        //<! module execution failed
#define MODULE_STATUS_STARTED 4       //<! module is calculating now
#define MODULE_STATUS_STOPPING 5      //<! module is stopping now


class DiscompModuleParameter;

class DiscompModule : public QObject
{
	Q_OBJECT
		
public:
	DiscompModule  ( QObject *parent = 0);
	~DiscompModule ();

	QString     log_file; //!< Log file to which will be saved log data
	QString     error_msg;//!< Error message of module execution (for send it to the server)

	//module info
	QString     name;     //!< Name
	QString     abs_name; //!< Absolute module name (without operation)
	QString     operation;//!< Name of the operation
	QString     fullname; //!< Full name
	QString     comment;  //!< Comment
	QString     language; //!< Programing language
	QString     type;     //!< Module type (binary, shell script, etc.)
	QString     md5;      //!< Module directory md5 sum

	//commands
	QString     cmd_start;              //!< Start module command
	qint64      cmd_start_max_time;     //!< Max execution time
	bool        cmd_start_detached;     //!< Process should be started as detached?
	QString     cmd_start_stdout_file;  //!< Filename to which will be save programm output
	QString     cmd_start_stderr_file;  //!< Filename to which will be save programm output
	QString     cmd_stop;               //!< Stop module command
	bool        cmd_stop_detached;      //!< Stop module command should be started as detached?

	bool        is_failed_module;       //!< Module execution failed once. This module will be skiped next time.
	bool        stoping_flag;           //!< Module is stoping now
	bool        started_flag;           //!< Module is executing now?
	QString     module_dir;             //!< Absolute path to module directory
	QString     temp_module_dir;        //!< Absolute path to temp module directory (in which module will work)
	QString     process_parameters_dir; //!< Where on the server should be saved output parameters
	QVector<DiscompModuleParameter*>   inputParameters;  //!< Module input parameters array
	QVector<DiscompModuleParameter*>   outputParameters; //!< Module output parameters array

	uint        execution_start_time;   //!< Module execution start time (used on the client and server
	                                    //!< sides for calculate real and full time).
	uint        execution_full_time;    //!< Full execution time with transoprt charges
	uint        execution_real_time;    //!< Real execution time (module execution on the client side)

	QTime       execution_start_time_ms;//!< same execution_start_time param (in miliseconds)
	int         execution_full_time_ms; //!< same execution_full_time param (in miliseconds)
	int         execution_real_time_ms; //!< same execution_real_time param (in milisecconds)

	QString     node_resources_usage_info;//!< resouces usage information on the node
	int         output_synctoserver_interval; //!< interval for check output parameters changes (for sync it to the server)

	#ifdef SERVER
	QMap <QString,QString> events;      //!< Process stages interpreter events (onStart, onFinish, etc.)
	#endif



	int  LoadModule     ( QString module_name, QString operation_name = "" );

	int  isReady        ( );

	#ifdef CLIENT
	int  Start          ( );
	int  Stop           ( );

	int     createModuleTempEnvroment ( );
	int     clearModuleTempEnviroment ( );
	QString getModuleTempDir          ( );
	void    setModuleParametersDir    ( const QString &dir );
	
	DiscompModuleParameter*  getParameterByName ( const QString &param_name );
	#endif 

	QString     getModuleMD5        ( const QString &module_name = "" );
	QStringList getModuleOperations ( const QString &module_name );
	QString     getModuleDirectory  ( const QString &module_name = "" );

	void    logMessage   ( int log_level, const QString &message );

private:
	#ifdef CLIENT
	QProcess *system_process;        //!< System process class
	QTimer   *max_execution_timer;   //!< Timer for check max module execution time
	QTimer   *check_resources_timer; //!< Timer for check process resources: memory, cpu, etc. 



	QString                  getProcessErrorMsg      ( QProcess::ProcessError error );

	DiscompModuleParameter*  getInputParameterByName ( const QString &param_name );
	DiscompModuleParameter*  getOutputParameterByName( const QString &param_name );
	#endif

	QDomNode                 getModuleRootNode       ( const QString &module_name );


private slots:
	#ifdef CLIENT	
	void startProcessExited            ( int, QProcess::ExitStatus ); 
	void startProcessExitedError       ( QProcess::ProcessError );
	void startProcessExitedDetached    ( );
	void stopProcessExitedError        ( QProcess::ProcessError );
	void processExecutionTimeout       ( );
	void checkProcessResources         ( );
	#endif
	void logMessage_slot               ( int log_level, const QString &message );

signals :
	#ifdef CLIENT
	void moduleExecutionFinished_signal( const QString &module_name, int status );
	#endif
};

#endif

// vim: set fenc=utf-8 tabstop=8 :
