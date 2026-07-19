// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#include "module.h"


#include <QTimer>
#include <QFile>
#include <QDir>
#include <QRegExp>
#include <QtXml>

#include "module_parameter.h"
#include "common.h"
#include "file.h"
#include "xml.h"
#include "log.h"
#include "config.h"

#ifdef CLIENT
#ifdef Q_WS_WIN
#include <windows.h>
#include <psapi.h>
#endif
#endif

/**
 * @class DiscompModule
 * @brief Discomp module class
 *
 * This class allow working with discomp module.
 * This class initialized by module.xml specification
 * where defined commands for start and stop module, 
 * input and output parameters and some info about module.
 *
 */



/***********************************************************************/
/*-----------------  Constructor & Destructor  ------------------------*/
/***********************************************************************/

/** 
 * Constructor. 
 */
DiscompModule::DiscompModule(QObject *parent)
	: QObject(parent)
{
	name           = "";
	log_file       = "";
	error_msg      = "";
	md5            = "";

	module_dir     = "";
	stoping_flag   = false;

	is_failed_module = false;

	#ifdef CLIENT
	system_process = NULL;

	temp_module_dir= "";

	//timer for check max execution time
	max_execution_timer = new QTimer(this);
	connect(max_execution_timer, SIGNAL(timeout()), this, SLOT(processExecutionTimeout()));

	//timer for check process resources
	check_resources_timer = new QTimer(this);
	connect(check_resources_timer, SIGNAL(timeout()), this, SLOT(checkProcessResources()));                                                                     
	#endif

	//execution timers in seconds
	execution_start_time = 0;
	execution_full_time  = 0;
	execution_real_time  = 0;

	output_synctoserver_interval = 0;

	//execution timers in miliseconds
	execution_full_time_ms = 0;
	execution_real_time_ms = 0;

	node_resources_usage_info = "";

	// where on the server should be saved output parameters
	/// it's used for processing filelist.
	process_parameters_dir = "";
};

/**
 * Destructor
 */
DiscompModule::~DiscompModule()
{
	logMessage( LOG_DEBUG, QString("Destructing module") );

	#ifdef CLIENT
	if ( system_process ) {
		logMessage( LOG_WARNING, "Detected active system process (some module calculating now). Kill it" );
		system_process->kill();
		delete system_process;
	};
	#endif

	logMessage( LOG_DEBUG, "Destructing module parameters" );
	for ( int param_num=0; param_num < (int)this->inputParameters.size(); param_num++ ) {
		delete this->inputParameters[param_num];
	};
	for ( int param_num=0; param_num < (int)this->outputParameters.size(); param_num++ ) {
		delete this->outputParameters[param_num];
	};
};






/***********************************************************************/
/*-----------------  Load module specification  -----------------------*/
/***********************************************************************/

/**
 * Get list of the module operations. Each operation have own
 * commands for start and stop module, differ input and output parameters.
 * @param module_name - module identificator
 * @return string list with all supported operations
 */
QStringList DiscompModule::getModuleOperations ( const QString &module_name ) 
{
	QStringList operations_list; 
	if ( module_name == "" ) {
		logMessage(LOG_ERROR, QString("Module name is not specified"));
		return operations_list;
	};

	QDomNode rootNode = this->getModuleRootNode ( module_name );

	if ( rootNode.isNull() ) {
		logMessage( LOG_ERROR, QString("Couldn't load module specification for: %1").arg(module_name));
		return operations_list;
	};

	QDomNode operationsNode = XML::getChildNodeByTag( "operations",     rootNode );
	QDomNode operationNode = rootNode;
	if ( !operationsNode.isNull () ) {
		QDomNode opNode = operationsNode.firstChild();
		while ( !opNode.isNull() )
		{
			if ( opNode.toElement().tagName() == "operation" ) {
				//QString operation = opNode.toElement().attribute("name");
				//if ( operation == "" ) { operation = "main"; };
				operations_list << opNode.toElement().attribute("name");
			};
			
			opNode = opNode.nextSibling();
		};
	};
	if ( !operations_list.size() ) {
		operations_list << "";
	};
	return operations_list;
}


/**
 * Module initialization by module name. 
 * Module directory and and name in the module specification file should be equal.
 * @param module_name - module name. It's can be like (dsat.solver, dsat.decompose:operation1)
 * @param operation_name - specify direct operation in the module
 * @return - status of the module loading (true or false)
 * */
int DiscompModule::LoadModule ( QString module_name, QString operation_name  )
{
	if ( module_name == "" ) {
		logMessage(LOG_ERROR, QString("Module name is not specified"));
		return false;
	};

	// module_name can be like this "module:operation3". 
	int separator_pos = module_name.indexOf (":");
	QString abs_module_name = module_name;
	if ( separator_pos != -1 ) {
		abs_module_name = module_name.left(separator_pos);
		operation_name  = module_name.mid(separator_pos+1);
	} else if ( operation_name != "" ) {
		module_name += ":" + operation_name;
	};

	//module_name should be PACKAGE.MODULE 
	int dot_pos = abs_module_name.indexOf (".");
	if ( dot_pos == -1 ) {
		logMessage ( LOG_ERROR, "Module name ("+module_name+") should contain package name separated by dot (for ex dsat.solver)");
		return false;
	};

	logMessage ( LOG_DEBUG, "Absolute module directory: " + this->module_dir + " !!!! " +  abs_module_name );
	//set paths to the module
	this->module_dir = this->getModuleDirectory ( abs_module_name );
	logMessage ( LOG_DEBUG, "Absolute module directory: " + this->module_dir );

	//load specification
	QDomNode rootNode = this->getModuleRootNode ( abs_module_name );

	if ( rootNode.isNull() ) {
		logMessage( LOG_ERROR, QString("Couldn't load module specification for: %1").arg(module_name));
		return false;
	};



	// ------------------- Common info ----------------------------
	// INFO
	QDomNode infoNode = XML::getChildNodeByTag( "info",     rootNode );
	this->name        = module_name;
	this->abs_name    = abs_module_name;
	this->operation   = operation_name;
	this->fullname    = XML::getTextNodeByTag ( "fullname", infoNode );
	this->comment     = XML::getTextNodeByTag ( "comment",  infoNode );
	this->language    = XML::getTextNodeByTag ( "language", infoNode );
	this->type        = XML::getTextNodeByTag ( "type",     infoNode );
	this->md5         = this->getModuleMD5();
	logMessage( LOG_COMMON, QString("Module initialization:") );


	// ------------------- Init Module Operation ------------------------- 
	// searching operation Node, 
	// if it's not exists - then module contain only 1 operation
	QDomNode operationsNode = XML::getChildNodeByTag( "operations",     rootNode );
	QDomNode operationNode = rootNode;
	if ( !operationsNode.isNull () ) {
		QDomNode opNode = operationsNode.firstChild();
		bool found = false;
		while ( !opNode.isNull() )
		{
			if ( opNode.toElement().tagName() == "operation" && (opNode.toElement().attribute("name") == operation_name || operation_name == "") ) {
				if ( operation_name == "" ) {
					this->operation = operation_name = opNode.toElement().attribute("name");
					this->name += ":" + this->operation;
				};

				operationNode = opNode;
				found = true;
				break;
			};
			
			opNode = opNode.nextSibling();
		};
		if ( !found ) {
			logMessage ( LOG_WARNING, QString("Couldn't find '%1' operation in the module specification").arg(operation_name) );
			return false;
		};
	};

	// ------------------- Init operation specification ------------------------- 
	
	//first trying to find OS-specific commands
	QDomNode commandsSpecificNode = XML::getChildNodeByTag( "commands", operationNode );
	#ifdef Q_OS_UNIX
	commandsSpecificNode = XML::getChildNodeByTag( "commands_unix", operationNode );
	#endif
	#ifdef Q_WS_MAC
	commandsSpecificNode = XML::getChildNodeByTag( "commands_mac", operationNode );
	#endif
	#ifdef Q_WS_WIN
	commandsSpecificNode = XML::getChildNodeByTag( "commands_win", operationNode );
	#endif

	QDomNode commandsNode;
	if ( commandsSpecificNode.isNull() ) {
		// COMMANDS
		commandsNode = XML::getChildNodeByTag( "commands", operationNode );
		if ( commandsNode.isNull() ) {
			logMessage( LOG_ERROR, QString("Couldn't find commands node") );
			return 0;
		};
	} else {
		commandsNode = commandsSpecificNode;
	};

	QDomNode startNode       = XML::getChildNodeByTag ( "start", commandsNode );
	this->cmd_start          = XML::getTextNodeByTag  ( "start", commandsNode );
	this->cmd_start_max_time = startNode.toElement().attribute("max_time").toLongLong();
	this->cmd_start_detached = startNode.toElement().attribute("detached").toInt();
	this->cmd_start_stdout_file = startNode.toElement().attribute("stdout");
	this->cmd_start_stderr_file = startNode.toElement().attribute("stderr");
	QDomNode stopNode        = XML::getChildNodeByTag ( "stop", commandsNode );
	this->cmd_stop           = XML::getTextNodeByTag  ( "stop", commandsNode );
	this->cmd_stop_detached  = stopNode.toElement().attribute("detached").toInt();



	// PARAMETERS

	//clear if exists (for reload module)
	for ( int param_num=0; param_num < (int)this->inputParameters.size(); param_num++ ) {
		delete this->inputParameters[param_num];
	};
	this->inputParameters.clear();
	for ( int param_num=0; param_num < (int)this->outputParameters.size(); param_num++ ) {
		delete this->outputParameters[param_num];
	};
	this->outputParameters.clear();

	
	QDomNode parametersNode    = XML::getChildNodeByTag( "parameters", operationNode );
	
	//input parameters
	logMessage(LOG_COMMON, "  Input parameters");

	//for syste,.single module, module.xml should be transfered too
	if ( module_name == "system.single" ) { 
		DiscompModuleParameter *param = new DiscompModuleParameter();
		param->dir      = this->module_dir;
		param->type	= "file";
		param->name	= "module.xml";
		param->filename	= "module.xml";
		inputParameters.push_back( param );
	};

	
	QDomNode parametersInNode = XML::getChildNodeByTag( "input", parametersNode );
	QDomNode paramNode = parametersInNode.firstChild();
	while ( !paramNode.isNull() )
	{
		if ( paramNode.toElement().tagName() == "param" ) {
			DiscompModuleParameter *param = new DiscompModuleParameter();
			param->dir      = this->module_dir;
			connect ( param, SIGNAL ( logMessage_signal  (int, const QString &) ),
			          this,  SLOT   ( logMessage_slot    (int, const QString &) ) );

			if ( param->Init( paramNode ) ) {
				inputParameters.push_back( param );
			} else {
				logMessage(LOG_ERROR, "Parameters initialization failed");
				delete param;
				return false;
			};
		};
		paramNode = paramNode.nextSibling();
	};

	
	//output parameters
	logMessage(LOG_COMMON, "  Output parameters");
	
	QDomNode parametersOutNode  = XML::getChildNodeByTag( "output", parametersNode );

	// interval for check output parameters changes (for sync it to the server)
	output_synctoserver_interval = parametersOutNode.toElement().attribute("synctoserver_interval").toInt();
	if ( output_synctoserver_interval > 0 && output_synctoserver_interval < 10 ) { 
		output_synctoserver_interval = 10;
	};
	if ( output_synctoserver_interval ) {
		logMessage(LOG_NOTICE, QString("    Output parameters will be send (synchronized) to the server with %1 sec interval").arg(output_synctoserver_interval));
	};
	
	paramNode = parametersOutNode.firstChild();
	while ( !paramNode.isNull() )
	{
		if ( paramNode.toElement().tagName() == "param" ) {
			DiscompModuleParameter *param = new DiscompModuleParameter();
			param->dir      = this->module_dir;
			connect ( param, SIGNAL ( logMessage_signal  (int, const QString &) ),
			          this,  SLOT   ( logMessage_slot    (int, const QString &) ) );

			if ( param->Init( paramNode ) ) {
				outputParameters.push_back( param );
			} else {
				logMessage(LOG_ERROR, "Parameters initialization failed");
				delete param;
				return false;
			};
		};
		paramNode = paramNode.nextSibling();
	};
	
	
	return true;
};


/**
 * Recursive calculate module directory md5 sum.
 * It's used for compare module on the server and client side
 * @param module_name - module name
 * @return MD5 hash string. See openssl.h for hash length
 * */
QString DiscompModule::getModuleMD5( const QString &module_name )
{
	return File::getDirMD5 ( this->getModuleDirectory( module_name ) );
}


/**
 * Get absolute path to the module directory
 * @param module_name - module identificator
 * @return absolute path
 */
QString DiscompModule::getModuleDirectory ( const QString &module_name ) 
{
	QString abs_module_name = (module_name == "") ? this->abs_name : QString(module_name).replace(QRegExp(":.*$"),"");

	if ( module_name == "system.single" ) {
		//for system.single all shouyld be defined before LoadModule
		if ( this->module_dir == "" ) { logMessage ( LOG_WARNING, "SOMTHING WRONG. System.single module dir variable isn't set" ); };
		return this->module_dir;
	};


#ifdef CLIENT
	//on the client all modules stried in the one directory with dir name PACKAGE.MODULE
	if ( Config::variables["ModulesDirectory"] != "" ) {
		return QDir::convertSeparators ( Config::variables["ModulesDirectory"] + "/" + abs_module_name + "/" );
	} else {
		return QDir::convertSeparators ( Config::variables["BaseDirectory"] + "/" + QString(MODULES_DIR) + "/" + abs_module_name + "/" );
	};
#endif
#ifdef SERVER
	//on the server all modules storied in the processes directory structure processes/PACKAGE/modules/MODULE
	QStringList mod_info = abs_module_name.split(".");
	return QDir::convertSeparators ( Config::variables["BaseDirectory"] + "/" + QString(PACKAGES_DIR) + "/" + mod_info.at(0) + "/" + QString(MODULES_DIR) + "/" + mod_info.at(1) + "/" );
#endif
};


/***********************************************************************/
/*----------------------  Module execution  ---------------------------*/
/***********************************************************************/



/**
 * Check if defined all input parameters for module execution
 * */
int DiscompModule::isReady()
{
	for ( int param_num=0; param_num < (int)inputParameters.size(); param_num++ ) {
		if ( !inputParameters[param_num]->is_defined() ) {
			this->error_msg = QString("Parameter (name: %1) is not defined").arg(inputParameters[param_num]->name);
			logMessage( LOG_ERROR, this->error_msg );
			return false;
		};
	};

	return true;

}

#ifdef CLIENT

/**
 * Create module temp enviroment.
 * Module dir copied to the temp folder.
 * @return status of the creation
 */
int DiscompModule::createModuleTempEnvroment( )
{
	//create temp directory
	QString temp_dir_path = File::createTempDirectory ( this->abs_name + "_" + this->operation + "_") ;
	
	logMessage( LOG_COMMON, QString ("Creating module temporary directory: %1").arg(temp_dir_path) );
	if ( temp_dir_path == NULL ) {
		this->error_msg = "Couldn't create module temporary directory." ;
		logMessage( LOG_ERROR, this->error_msg );
		return false;
	};


	this->temp_module_dir = temp_dir_path + QDir::separator();
	if ( this->name != "system.single" ) {
		if ( !File::recursiveCopyDirectory( this->module_dir, this->temp_module_dir ) ) {
			this->error_msg = "  Couldn't create module temp directory. Directory copy from " + this->module_dir + " to the " + this->temp_module_dir;
			logMessage ( LOG_ERROR, this->error_msg ) ;
			return false;
		};

		//here we check md5 sum of old module and of the temporary. if it's differ, than something wrong
		QString copied_module_md5 = File::getDirMD5 ( this->temp_module_dir );
		if ( this->md5 != copied_module_md5 ) {
			this->error_msg = "  Couldn't create module temp directory. Original module directory md5 ("+this->md5+") and temporary module dir md5 ("+copied_module_md5+") are differ. Maybe there are no disk space.";
			logMessage ( LOG_ERROR, this->error_msg ) ;
			return false;
		};
	};
	
	
	setModuleParametersDir ( this->temp_module_dir );

	// now restore all constant parameters for this module
	// if it's already defined - copy it to the temp module dir
	for ( int param_num=0; param_num < (int)inputParameters.size(); param_num++ ) {
		if ( inputParameters[param_num]->constant ) {
			if ( inputParameters[param_num]->constant_tmp_path != "" ) {
				logMessage ( LOG_COMMON, QString("  Restore constant parameter (name: %1, from: %2)").
						arg(inputParameters[param_num]->name).arg(inputParameters[param_num]->constant_tmp_path));
				if ( !File::move ( inputParameters[param_num]->constant_tmp_path,
				                   this->temp_module_dir + QDir::separator() + inputParameters[param_num]->filename ) 
				) {
					this->error_msg = "  Prepare module failed. Reason: couldn't restore constant parameter: " + this->temp_module_dir + QDir::separator() + inputParameters[param_num]->filename + " from file: " + inputParameters[param_num]->constant_tmp_path;
					logMessage ( LOG_ERROR, this->error_msg ) ;
					return false;
				};

				inputParameters[param_num]->constant_tmp_path = "";
			};
		};
	};

	return true;
};

/**
 * Clear module temp enviroment.
 * Delete dir. For socket set incoming dir to the default
 * @return status of the operation
 */
int DiscompModule::clearModuleTempEnviroment ( )
{
	logMessage( LOG_COMMON, QString ("Clear module temporary directory: %1").arg(this->temp_module_dir));
	if ( this->temp_module_dir == "" ) {
		logMessage ( LOG_WARNING, QString ("Module temp directory isn't exist: %1").arg(this->temp_module_dir) );
		return true;
	};

	//first save all constant parametes for this module
	for ( int param_num=0; param_num < (int)inputParameters.size(); param_num++ ) {
		if ( inputParameters[param_num]->constant ) {
			QString param_filepath = this->temp_module_dir + QDir::separator() + inputParameters[param_num]->filename;
			QString param_tmp_path;
			if ( !Config::variables["ClearTempModulesDir"].toInt() ) { //for debug we should save state.
				param_tmp_path = File::copyToTemporary ( param_filepath, this->abs_name + "_" + this->operation + "_" + inputParameters[param_num]->filename + "_" );
			} else {
				param_tmp_path = File::moveToTemporary ( param_filepath, this->abs_name + "_" + this->operation + "_" + inputParameters[param_num]->filename + "_" );
			};

			if ( param_tmp_path != NULL ) {
				logMessage ( LOG_COMMON, QString("  Save constant parameter for following execution (name: %1, path: %2)").
						arg(inputParameters[param_num]->name).arg(param_tmp_path));

				inputParameters[param_num]->constant_tmp_path = param_tmp_path;
			};
		};
	};

	//remove module temp directory
	if ( Config::variables["ClearTempModulesDir"].toInt() && (this->temp_module_dir != this->module_dir) && this->temp_module_dir != "" ) {
		if (  !File::recursiveRemoveDirectory ( this->temp_module_dir ) ) {
			return false;
		};
	};
	this->temp_module_dir = "";
	return true;
};

QString DiscompModule::getModuleTempDir ()
{
	return this->temp_module_dir;
};


/**
 * Start module on the client side.
 * */
int DiscompModule::Start()
{
	logMessage( LOG_COMMON, "Starting module" );
	logMessage( LOG_COMMON, "  Working directory: " + this->temp_module_dir );

	this->error_msg = "";
	
	if ( system_process ) { delete system_process; };
	system_process = new QProcess;
	system_process->setWorkingDirectory(this->temp_module_dir);
	
	//parse command string
	QStringList command_parameters = this->cmd_start.split(" ");
	
	QStringList arguments;
	QString program_path = command_parameters[0];
	
	for (int param_num=1; param_num < (int)command_parameters.size(); param_num++ ) {

		QRegExp rx_param("\\{PARAM_VALUE:(\\w+)\\}");
		int pos = rx_param.indexIn( command_parameters[param_num] );
		QString paramName = ( pos > -1 ) ? rx_param.cap( 1 ) : "";

		if ( paramName != "" ) { //if parameter name was found
			
			DiscompModuleParameter* param = getParameterByName( paramName );

			if ( !param ) {
				this->error_msg = QString("Couldn't find parameter (name: %1) in the module specification").arg(paramName);
				logMessage( LOG_ERROR, this->error_msg );
				return 0;
			};
		
			if ( param->type == "file" ) {
				QString file_value = param->getFileTypeValue();
				arguments << file_value.split(" ");
			};
			//} else if ( param->type == "filelist" ) {
			//	for ( int list_num=1; list_num <= param->listsize(); list_num++ ) {
			//		arguments << QString(param->filepattern).arg(list_num);
			//	};
			//};
		} else { //static value
			//if should be specified absolute path to the module directory
			QRegExp rx_param("\\{MODULE_DIR\\}");
			command_parameters[param_num].replace(rx_param, this->temp_module_dir );
		
			arguments << command_parameters[param_num];
		};
	};


	connect ( system_process, SIGNAL ( finished           (int, QProcess::ExitStatus) ),
	          this,           SLOT   ( startProcessExited (int, QProcess::ExitStatus) ) );
	
	connect ( system_process, SIGNAL ( error                   (QProcess::ProcessError)  ),
	          this,           SLOT   ( startProcessExitedError (QProcess::ProcessError)  ) );
	

#ifdef WIN32
	system_process->setWorkingDirectory(this->temp_module_dir);
	program_path = this->temp_module_dir +QDir::separator() + program_path;
#endif
#ifdef Q_OS_LINUX
	//TODO: wine config files shoudl be located in the temp dir 
	//because if home dir is a shared filesystems it take a long time for init
	
	//system_process->setEnvironment(QStringList() << "WINEPREFIX=" + Config::variables["TempDirectory"] + "/.wine/");
	//NO!! this call rewrite clear all systyem variables!!
	//set it manually in additional script if you need it
#endif

	//remember execution start time
	QDateTime timestamp (QDateTime::currentDateTime ());
	execution_start_time = timestamp.toTime_t ();
	execution_start_time_ms.restart();

	logMessage( LOG_COMMON, "  Starting command: " + program_path + " " + arguments.join(" ") );

	//set stdout file if needed
	if ( this->cmd_start_stdout_file != "" ) {
		system_process->setStandardOutputFile(this->temp_module_dir + QDir::separator() + this->cmd_start_stdout_file );
		logMessage ( LOG_COMMON, "  stdout will be saved to the: "+ this->temp_module_dir + QDir::separator() + this->cmd_start_stdout_file );
	};
	//set stderr file if needed
	if ( this->cmd_start_stderr_file != "" ) {
		system_process->setStandardErrorFile(this->temp_module_dir + QDir::separator() + this->cmd_start_stderr_file );
		logMessage ( LOG_COMMON, "  stderr will be saved to the: "+ this->temp_module_dir + QDir::separator() + this->cmd_start_stderr_file );
	};


	//chmod 755 
	QFile::setPermissions ( this->temp_module_dir + program_path, QFile::ReadUser|QFile::WriteUser|QFile::ExeUser );

	if ( this->cmd_start_detached ) {
		logMessage(LOG_COMMON, "    Process should be started detached");

		system_process->startDetached( program_path, arguments);

		QTimer::singleShot(1000, this, SLOT(startProcessExitedDetached()));
	
	} else {
		system_process->start( program_path, arguments);

		if ( !system_process->waitForStarted() ) {
			//TODO. here can be a bug?
			this->error_msg = "-1";
			//this->error_msg = "  Execution failed. Command: " + program_path + " " + arguments.join(" ");
			//logMessage( LOG_ERROR, this->error_msg );
			return 0;
		};

		//check process resources
		checkProcessResources();
		check_resources_timer->start(1000);
	}


	if ( this->cmd_start_max_time > 0 ) {
		logMessage (LOG_COMMON, QString("  Max module execution time is: %1 (sec)").arg(this->cmd_start_max_time) );
		max_execution_timer->start(this->cmd_start_max_time*1000);
	};
	
	if ( !system_process ) {
		logMessage( LOG_WARNING, "System process isn't defined. Somthing wrong." );
		return 0;
	};

	logMessage( LOG_COMMON, QString("  Module successfully started") );
	this->started_flag = true;
	
	return 1;
};

/**
 * Stop module execution on the client side
 */
int DiscompModule::Stop()
{	
	logMessage( LOG_COMMON, "Stoping module execution");
	stoping_flag = true;
	started_flag = false;

	//stop timer "max execution time"
	max_execution_timer->stop();
	check_resources_timer->stop();

	//calculate real time
	QDateTime timestamp (QDateTime::currentDateTime ());
	execution_real_time    = timestamp.toTime_t () - execution_start_time;
	execution_real_time_ms = execution_start_time_ms.elapsed();

	if ( system_process ) {
		//trying to kill system process
		system_process->kill();
		if ( !system_process->waitForFinished() ) {
			logMessage( LOG_WARNING, "  Couldn't stop system process by executing kill()");

			//try again
			system_process->terminate();
			system_process->kill();

			if ( !system_process->waitForFinished() ) {
				logMessage( LOG_WARNING, "  Couldn't stop system process by executing terminate() too.");
			};
		};
	} else {
		logMessage( LOG_WARNING, "System process object isn't defined." );
	};


	//stop command for module is defined
	delete system_process;
	system_process = NULL;

	if ( this->cmd_stop == "" ) {
		stoping_flag = false;
		return 1;
	};
	

	logMessage ( LOG_COMMON, "Executing command for stoping module execution" );
	logMessage( LOG_COMMON, "  Working directory:" + this->temp_module_dir );

	if ( this->temp_module_dir == "" ) {
		logMessage ( LOG_WARNING, "Module temp directory isn't defined. Stop command cannot be executed.");
		return 1; //tell that all ok
	};

	system_process = new QProcess;
	system_process->setWorkingDirectory(this->temp_module_dir);
	
	//parse command string
	QStringList command_parameters = this->cmd_stop.split(" ");
	QStringList arguments;
	QString program_path = command_parameters[0];
	for (int i = 1; i < command_parameters.size(); ++i) {
		arguments << command_parameters.at(i);
	}

	connect ( system_process, SIGNAL ( error                  (QProcess::ProcessError)  ),
	          this,           SLOT   ( stopProcessExitedError (QProcess::ProcessError)  ) );

#ifdef WIN32
	system_process->setWorkingDirectory(this->temp_module_dir);
	program_path = this->temp_module_dir +QDir::separator() + program_path;
#endif

	
	logMessage( LOG_COMMON, "  Starting command: " + program_path + " " + arguments.join(" ") );


	//chmod 755 
	QFile::setPermissions ( this->temp_module_dir + program_path, QFile::ReadUser|QFile::WriteUser|QFile::ExeUser );

	if ( this->cmd_stop_detached ) {
		logMessage(LOG_COMMON, "    Process should be started detached");

		system_process->startDetached( program_path, arguments);
	} else {
		system_process->start( program_path, arguments);

		if ( !system_process->waitForStarted() ) {
			delete system_process;
			system_process = NULL;
			stoping_flag = false;
			logMessage( LOG_ERROR, "   Execution failed. Command: " + program_path + " " + arguments.join(" ") );
			return 0;
		};
	}

	if ( !system_process ) {
		delete system_process;
		system_process = NULL;
		stoping_flag = false;
		return 0;
	};

	if ( !system_process->waitForFinished() ) {
		logMessage( LOG_WARNING, "  Stoping command isn't finished");
	} else {
		logMessage ( LOG_COMMON, "  Stoping module execution command successfully finished" );
	};

	if ( system_process ) { delete system_process; };
	system_process = NULL;
	stoping_flag = false;
	
	return 1;
}





/***********************************************************************/
/*------------------------ QProcess slots -----------------------------*/
/***********************************************************************/

/**
 * "Start command" process has been finished
 */
void DiscompModule::startProcessExitedDetached()
{
	for ( int param_num=0; param_num < (int)outputParameters.size(); param_num++ ) {
		if ( !outputParameters[param_num]->is_defined() ) {
			QTimer::singleShot(1000, this, SLOT(startProcessExitedDetached()));
			return;
		};
	};

	started_flag = false;
	
	//stop timer "max execution time"
	max_execution_timer->stop();
	check_resources_timer->stop();

	//calculate real time
	QDateTime timestamp (QDateTime::currentDateTime ());
	execution_real_time    = timestamp.toTime_t () - execution_start_time;
	execution_real_time_ms = execution_start_time_ms.elapsed();

	logMessage(LOG_DEBUG, QString("Process exited") );
	startProcessExited(0, QProcess::NormalExit);
};

/**
 * "Start command" process has been exited"
 */
void DiscompModule::startProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
	started_flag = false;

	if ( stoping_flag ) {
		return;
	};

//	if ( exitCode ) {
		logMessage(LOG_WARNING, QString("Process exited. Code: %1. Status: %2").arg(exitCode).arg(exitStatus) );
//	};

	//stop timer "max execution time"
	max_execution_timer->stop();
	check_resources_timer->stop();


	//calculate real time
	QDateTime timestamp (QDateTime::currentDateTime ());
	execution_real_time    = timestamp.toTime_t () - execution_start_time;
	execution_real_time_ms = execution_start_time_ms.elapsed();
	
	emit moduleExecutionFinished_signal ( this->name, 1 );

	logMessage (LOG_COMMON, "Module execution finished");

	//delete system_process;
	if ( system_process ) { delete system_process; };
	system_process = NULL;
};

/**
 * "Start command" process exited with error
 */
void DiscompModule::startProcessExitedError ( QProcess::ProcessError error )
{
	started_flag = false;

	if ( stoping_flag ) {
		return;
	};

	//stop timer "max execution time"
	max_execution_timer->stop();
	check_resources_timer->stop();

	//calculate real time
	QDateTime timestamp (QDateTime::currentDateTime ());
	execution_real_time    = timestamp.toTime_t () - execution_start_time;
	execution_real_time_ms = execution_start_time_ms.elapsed();

	this->error_msg = "Process exited. Reason: " + DiscompModule::getProcessErrorMsg ( error );
	logMessage (LOG_ERROR, this->error_msg);
	emit moduleExecutionFinished_signal ( this->name, 0 );
};

/**
 * "Stop command" process exited with error
 */
void DiscompModule::stopProcessExitedError ( QProcess::ProcessError error )
{
	logMessage (LOG_ERROR, "Process exited. Reason: " + DiscompModule::getProcessErrorMsg ( error ) );
};


/**
 * Max module execution time is reached.
 * Module should be stoped
 */
void DiscompModule::processExecutionTimeout()
{
	this->error_msg = QString("Max module execution time (%1 sec) is reached").arg(this->cmd_start_max_time);
	logMessage ( LOG_WARNING, this->error_msg );

	started_flag = false;

	//calculate real time
	QDateTime timestamp (QDateTime::currentDateTime ());
	execution_real_time    = timestamp.toTime_t () - execution_start_time;
	execution_real_time_ms = execution_start_time_ms.elapsed();

	this->Stop();
	emit moduleExecutionFinished_signal ( this->name, 2 );
};


/**
 * Check process resources (memory, cpu) every sec
 */
void DiscompModule::checkProcessResources()
{
	check_resources_timer->start(5000);
#ifdef Q_OS_WIN
        PROCESS_MEMORY_COUNTERS pmc;

	Q_PID process_info = system_process->pid();
	//node_resources_usage_info = QString("ProcessId: %1; ").arg(process_info->dwProcessId);	
	node_resources_usage_info = "";
	
	if ( GetProcessMemoryInfo( process_info->hProcess, &pmc, sizeof(pmc) ) ) {
		node_resources_usage_info += QString("MemoryPeak: %1; ").arg(pmc.WorkingSetSize);
		node_resources_usage_info += QString("PagefilePeak: %1; ").arg(pmc.PeakPagefileUsage);
	}
	
	//logMessage(LOG_DEBUG, node_resources_usage_info);
#endif
#ifdef Q_OS_LINUX
	QProcess sys_process;
	sys_process.start( QString("cat /proc/%1/status").arg(system_process->pid()));
	sys_process.waitForFinished();
	QString stdout_data ( sys_process.readAllStandardOutput().data() );
	
	//node_resources_usage_info = QString("ProcessId: %1; ").arg(process_info->dwProcessId);	
	node_resources_usage_info = "";
	if ( stdout_data != "" )  {
		QRegExp rx_mem("VmPeak:\\s*(\\d+)");
		int pos = rx_mem.indexIn( stdout_data );
		if ( pos > -1 ) {
			node_resources_usage_info = ( pos > -1 ) ? "MemoryPeak: " + rx_mem.cap( 1 ) + " kB"  : "";
		} else {
			rx_mem.setPattern("VmSize:\\s*(\\d+)");
			pos = rx_mem.indexIn( stdout_data );
			node_resources_usage_info = ( pos > -1 ) ? "MemorySize: " + rx_mem.cap( 1 ) + " kB"  : "";
		};
	};

//	logMessage(LOG_DEBUG, "+3:" + node_resources_usage_info);
#endif

};


/**
 * Process error message
 * @param error - error identificator
 * @return message of the error id
 */
QString DiscompModule::getProcessErrorMsg ( QProcess::ProcessError error )
{
	QString msg;
	switch ( error ) {
		case QProcess::FailedToStart:
			msg = "The process failed to start. Either the invoked program is missing, or you may have insufficient permissions to invoke the program.";
			break;
		case QProcess::Crashed:
			msg = "The process crashed some time after starting successfully.";
			break;
		default :
			msg = QString("Unknown. Error num: %1. See: http://doc.trolltech.com/4/qprocess.html#ProcessError-enum").arg(error);
			break;
	};
	return msg;
};

#endif   //#ifdef CLIENT






/***********************************************************************/
/*-----------------------  Private functions   ------------------------*/
/***********************************************************************/

/**
 * Get module root node from specification XML file
 * @param module_name - module name
 * @return QDomNode pointer to the root node &lt;module&gt;
 */
QDomNode DiscompModule::getModuleRootNode ( const QString &module_name )
{
	QString module_dir = this->getModuleDirectory(module_name);

	//read module specification from file
	QString fileName = module_dir + QString("%1").arg(MODULE_CONFIG_FILE);
	
	QFile file ( fileName );
	if  ( !file.open( QIODevice::ReadOnly ) ) {
		logMessage( LOG_ERROR, QString("Couldn't open file %1 for reading").arg(fileName) );
		return QDomNode();
	};

	QString errorStr;
	int errorLine;
	int errorColumn;
	
	QDomDocument doc;
	if ( !doc.setContent( &file, true, &errorStr, &errorLine, &errorColumn ) ) {
		logMessage ( LOG_ERROR, QString("XML processing error ( line %1, column %2: %3 )").arg(errorLine).arg(errorColumn).arg(errorStr) );
		file.close();
		return QDomNode();
	};
	file.close();

	QDomElement rootNode = doc.documentElement();
	if (rootNode.tagName() != "module") {
		logMessage( LOG_ERROR, "The root node of configuration file should be <module>");
		return QDomNode();
	};

	return rootNode;
}

#ifdef CLIENT
/**
 * Get module parameter by name
 * @param param_name - parameter name
 * @return - \a DiscompModuleParameter* class
 */
DiscompModuleParameter* DiscompModule::getParameterByName( const QString &param_name )
{
	DiscompModuleParameter *param;
	param = getInputParameterByName( param_name );
	if ( param ) {
		return param;
	};

	param = getOutputParameterByName( param_name );
	if ( param ) {
		return param;
	};

	return NULL;
};

/**
 * Get module input parameter by name
 * @param param_name - parameter name
 * @return - \a DiscompModuleParameter* class
 */
DiscompModuleParameter* DiscompModule::getInputParameterByName( const QString &param_name )
{
	for ( int param_num=0; param_num < (int)inputParameters.size(); param_num++ ) {
		if ( inputParameters[param_num]->name == param_name ) {
			return inputParameters[param_num];
		};
	};

	return NULL;
};


/**
 * Get module output parameter by name
 * @param param_name - parameter name
 * @return - \a DiscompModuleParameter* class
 */
DiscompModuleParameter* DiscompModule::getOutputParameterByName( const QString &param_name )
{
	for ( int param_num=0; param_num < (int)outputParameters.size(); param_num++ ) {
		if ( outputParameters[param_num]->name == param_name ) {
			return outputParameters[param_num];
		};
	};

	return NULL;
};

/**
 * Set module parameters (input and output) directory path
 * @param dir - path to the module directory
 */
void DiscompModule::setModuleParametersDir ( const QString &dir )
{
	for ( int param_num=0; param_num < (int)inputParameters.size(); param_num++ ) {
		inputParameters[param_num]->dir = dir;
	};

	for ( int param_num=0; param_num < (int)outputParameters.size(); param_num++ ) {
		outputParameters[param_num]->dir = dir;
	};
};

#endif

/**
 * Log message slot used for get messages from DiscompModuleParameter class
 * @param log_level - log level from log.h
 * @param message - log message
 */
void DiscompModule::logMessage_slot (  int log_level, const QString &message )
{
	this->logMessage ( log_level, message );
};

/**
 * Add log message
 * @param log_level - log level from log.h
 * @param message - log message
 */
void DiscompModule::logMessage( int log_level, const QString &message ) 
{
	QString message2;
	if ( this->name != "" ) {
		message2 = QString("[Module: %1] %2").arg(this->name).arg(message);
	} else {
		message2 = message;
	}
	
	if ( this->log_file != "" ) {
		Log::addMessage ( this->log_file, log_level, "DiscompModule", message2);
	} else {
		Log::addMessage ( log_level, "DiscompModule", message2);
	}
};
// vim: set fenc=utf-8 tabstop=8 :
