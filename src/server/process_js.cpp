// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#include "process_js.h"


/**
 * @class DiscompProcess
 * @brief Calculating Process
 *
 * This is a base class for works with calculating proces
 */

/***********************************************************************/
/*-----------------  Constructor & Destructor  ------------------------*/
/***********************************************************************/

//#include "process_plugin.h"

/*
 * Constructor
 * */
DiscompProcessJS::DiscompProcessJS(QObject *parent)
	: DiscompProcessPrivate( parent )
{
	id        = -1;
	name      = "";
	log_dir   = "";
	log_file  = "";
	

	completed = false;
	process_destructing = false;

	process_type = "js";
	execution_start_time = 0;

	current_stage = 0;

	priority = 0;
	this->EDP = false;
	process_script_engine = new QScriptEngine(this);
	logMessage( LOG_DEBUG, "Construct new process class JavaScript!");
}


/*
 * Destructor
 * */
DiscompProcessJS::~DiscompProcessJS()
{
	delete process_script_engine;
	process_script_engine = NULL;

	//Delete modules
	int mod_count = this->modules.size();
	for ( int mod_num = 0; mod_num < mod_count; mod_num++ ) {
		delete this->modules[mod_num];
		this->modules[mod_num] = NULL;
	}

	//Delete parameters
	int param_count = this->parameters.size();
	for ( int param_num = 0; param_num < param_count; param_num++ ) {
		delete this->parameters[param_num];
		this->parameters[param_num] = NULL;
	}
};



/***********************************************************************/
/*---------------  Calculating process functions  ---------------------*/
/***********************************************************************/


/*
 * Initialization of the calculating process
 * */
int DiscompProcessJS::Init ( const QString &process_name, const QString &user )
{
	logMessage ( LOG_COMMON, "Loading JavaScript process file" );

	this->name = process_name;

	//check user 
	this->user_name = user;

	//get package name and scheme name
	if ( process_name.indexOf (".") == -1 ) {
		logMessage ( LOG_WARNING, "Incorrect process name. Should contain a package name and scheme name (for ex PACKAGE.SCHEME)" );
		return false;
	};
	QStringList process_info = process_name.split(".");
	this->package_name = process_info.at(0);
	this->scheme_name  = process_info.at(1);
	if ( this->package_name == "" || this->scheme_name == "" ) {
		logMessage ( LOG_WARNING, QString("Process name is incorrect (package: %1, scheme: %2)").arg(this->package_name).arg(this->scheme_name) );
		return false;
	};
	
	//paths
	if ( Config::variables["PackagesDirectory"] != "" ) {
		this->absolute_process_dir = QDir::convertSeparators ( Config::variables["PackagesDirectory"] + "/" + QString(PACKAGES_DIR) + "/" + this->package_name + "/" + QString(PROCESSES_DIR) + "/"+ this->scheme_name );
	} else {
		this->absolute_process_dir = QDir::convertSeparators ( Config::variables["BaseDirectory"]     + "/"  + QString(PACKAGES_DIR) + "/" + this->package_name + "/" + QString(PROCESSES_DIR) + "/"+ this->scheme_name );
	}
	this->process_parameters_dir= QDir::convertSeparators ( absolute_process_dir + "/" + PARAMETERS_DIR );

	//log
	log_dir  = absolute_process_dir + QDir::separator() + "log" + QDir::separator();
	File::recursiveRemoveDirectoryContent ( log_dir );
	this->log_file = log_dir + QDir::separator() + QString("process.%1").arg(LOG_FILE_EXT);
	
	logMessage ( LOG_COMMON, "Process initialization: " + process_name + "");
	logMessage ( LOG_DEBUG,  "Process log directory: " + log_file );


	logMessage ( LOG_DEBUG, "Clear process failedmodules directory" );
	File::recursiveRemoveDirectoryContent ( absolute_process_dir + QDir::separator() +  "failedmodules" );


	if (QFile::exists(QDir::convertSeparators ( absolute_process_dir + "/" + QString("%1").arg(SCHEME_JS_TYPE) ))) {
		this->EDP = true;
	}
	// init process.js
	QString process_js_file = QDir::convertSeparators ( absolute_process_dir + "/" + QString("%1").arg(SCHEME_JS_CONFIG_FILE) );
	QString process_js_code;
	if ( !File::read ( process_js_file, process_js_code ) ) {
		logMessage ( LOG_COMMON, "process.js load error. Couldn't read file: " + process_js_file );
		return false;
	};
	logMessage( LOG_DEBUG, "process.js loaded" );

	/*
		START upgrading script engine environment
	*/

	// attach DiscompModule object to the script engine interpreter
	void CreateModuleScriptConstructor(QScriptEngine &engine);
	CreateModuleScriptConstructor(*process_script_engine);

	// attach DiscompParameter object to the script engine interpreter
	void CreateParameterScriptConstructor(QScriptEngine &engine);
	CreateParameterScriptConstructor(*process_script_engine);

	// add function logMessage to script environment
	QScriptValue LogMessageScriptValue(QScriptContext *context, QScriptEngine *engine);
	process_script_engine->globalObject().setProperty("logMessage", process_script_engine->newFunction(LogMessageScriptValue));

	// add function getFilesList("/path/to/directory")
	QScriptValue scriptGetFilesList(QScriptContext *context, QScriptEngine *engine);
	process_script_engine->globalObject().setProperty("getFilesList", process_script_engine->newFunction(scriptGetFilesList));

	// add function getFileContent("/path/to/file")
	QScriptValue scriptGetFileContent(QScriptContext *context, QScriptEngine *engine);
	process_script_engine->globalObject().setProperty("getFileContent", process_script_engine->newFunction(scriptGetFileContent));

	// add function setFileContent("/path/to/file", "content")
	QScriptValue scriptSetFileContent(QScriptContext *context, QScriptEngine *engine);
	process_script_engine->globalObject().setProperty("setFileContent", process_script_engine->newFunction(scriptSetFileContent));

	/*
		STOP upgrading script engine environment
	*/

	//init parameters. TODO code dublication from process.cpp
	logMessage(LOG_DEBUG, "Get information about parameters which will be used in the calculating process");

	QString package_parameters = "";
	if ( Config::variables["PackagesDirectory"] != "" ) {
		package_parameters = QDir::convertSeparators ( Config::variables["PackagesDirectory"] + "/" + this->package_name + "/" + QString(PACKAGE_PARAMETERS_FILE) );
	} else {
		package_parameters = QDir::convertSeparators ( Config::variables["BaseDirectory"] + "/" + QString(PACKAGES_DIR) + "/" + this->package_name + "/" + QString(PACKAGE_PARAMETERS_FILE) );
	};
	if ( !this->_initParametersFile ( package_parameters ) ) {
		return false;
	};
	File::recursiveRemoveDirectoryContent( this->process_parameters_dir + QDir::separator() + "output" );

	this->interpretator.setEngine(process_script_engine);
	this->interpretator.setCode(process_js_code);
	this->interpretator.setMutex(this->unimutex);
	this->interpretator.start();
	logMessage(LOG_DEBUG, "script evaluated");

	return true;
};

/*******************************************************************************
  Functions for upgrading script environment
********************************************************************************/

QScriptValue scriptGetFilesList(QScriptContext *context, QScriptEngine *engine)
{
	QString directory = context->argument(0).toString();
	QStringList files_list;
	QVector<QString> files_vector = File::getFilesList(directory);
	for ( int i = 0; i < files_vector.size(); i++ )
		files_list.append(files_vector[i]);
	return engine->toScriptValue(files_list);
}

QScriptValue scriptGetFileContent(QScriptContext *context, QScriptEngine *engine)
{
	QString file_name = context->argument(0).toString();
	QString file_content;
	File::read(file_name, file_content);
	return engine->toScriptValue(file_content);
}

QScriptValue scriptSetFileContent(QScriptContext *context, QScriptEngine *engine)
{
	QString file_name = context->argument(0).toString();
	QString file_content = context->argument(1).toString();
	return File::save(file_name,file_content);
}

// this function wraps logMessage and provides it's functionality to script environment
QScriptValue LogMessageScriptValue(QScriptContext *context, QScriptEngine *engine)
{
	QString message = context->argument(0).toString();

	static_cast<DiscompProcessJS *> (engine->parent())->logMessageFromScript(message);
	return true;
}

/**
 * Write log message to the process log file.
 * This method used only from scripts logMessage()
 * @param message - log message
 */
void DiscompProcessJS::logMessageFromScript ( const QString &message )
{
	if ( this->log_file != "" ) {
		Log::addMessage (this->log_file, LOG_COMMON, "DiscompProcessJS", "[Script][Log] " + message);
	} else {
		Log::addMessage ( LOG_COMMON, "DiscompProcessJS", "[Script][Log] " + message);
	};
};

/**
 * MetaConstructor for constructing objects in script file
 * like: new DiscompModule("moduleName")
 * TODO move this function in module_js.cpp for clearity of code
 */

QScriptValue DiscompModuleConstructor(QScriptContext *context, QScriptEngine *engine)
{
	QString module_name = context->argument(0).toString();
	DiscompProcessJS *process_js = static_cast<DiscompProcessJS *> (engine->parent());
	DiscompModuleJS *module = new DiscompModuleJS(QString(process_js->package_name + "." + module_name), process_js);

	return engine->newQObject(module);
}

/**
  * add costructor for creating DiscompModuleJS objects in script
  * TODO move this function in module_js.cpp for clearity of code
  */
void CreateModuleScriptConstructor(QScriptEngine &engine) {
	//prototype
	QScriptValue DiscompModuleConstructor(QScriptContext *context, QScriptEngine *engine);

	// define constructor for calling from script
	QScriptValue constructor = engine.newFunction(DiscompModuleConstructor);
	QScriptValue metaObject = engine.newQMetaObject(&QObject::staticMetaObject, constructor);
	engine.globalObject().setProperty("DiscompModule", metaObject);
}

/**
 * MetaConstructor for constructing objects in script file
 * like: new DiscompParameter("parameterName")
 * TODO move this function in parameter_js.cpp for clearity of code
 */

QScriptValue DiscompParameterConstructor(QScriptContext *context, QScriptEngine *engine)
{
	QString param_name = context->argument(0).toString();
	DiscompProcessJS *process_js = static_cast<DiscompProcessJS *> (engine->parent());
	DiscompParameterJS *param = new DiscompParameterJS(param_name, process_js);
	return engine->newQObject(param);
}

/**
  * add costructor for creating DiscompParameterJS objects in script
  * TODO move this function in parameter_js.cpp for clearity of code
  */
void CreateParameterScriptConstructor(QScriptEngine &engine) {
	//prototype
	QScriptValue DiscompParameterConstructor(QScriptContext *context, QScriptEngine *engine);

	// define constructor for calling from script
	QScriptValue constructor = engine.newFunction(DiscompParameterConstructor);
	QScriptValue metaObject = engine.newQMetaObject(&QObject::staticMetaObject, constructor);
	engine.globalObject().setProperty("DiscompParameter", metaObject);
}

/*******************************************************************************
  Functions for upgrading script environment END
********************************************************************************/
bool DiscompProcessJS::evaluateScript( const QString & script )
{
	if (this->interpretator.isRunning()) {
		logMessage(LOG_ERROR, "ASSERT processing event while interpretator running");
		return false;
	}
	QScriptValue result = this->process_script_engine->evaluate( script );
	if ( process_script_engine->hasUncaughtException() ) {
		int line = this->process_script_engine->uncaughtExceptionLineNumber();
		logMessage(LOG_ERROR, QString("Uncaught exception at line %1 : %2").arg(line).arg(result.toString()));
		return false;
	}
	return true;
}

//TODO code dublication from process.cpp
bool DiscompProcessJS::_initParametersFile ( const QString &file_path )
{
	//read parameters which will be used in the calculating process
	//

	logMessage(LOG_DEBUG, "Get information about parameters which will be used in the calculating process");


	QFile parameters_file ( file_path );
	if  ( !parameters_file.open( QIODevice::ReadOnly ) ) {
		logMessage( LOG_ERROR, QString("Could't open file %1 for reading").arg(file_path) );
		return false;
	};

	QDomDocument parameters_doc;
	QString errorStr;
	int errorLine;
	int errorColumn;


	if ( !parameters_doc.setContent( &parameters_file, true, &errorStr, &errorLine, &errorColumn ) ) {
		logMessage ( LOG_ERROR, QString("XML processing error for parameters.xml ( line %1, column %2: %3 )").arg(errorLine).arg(errorColumn).arg(errorStr) );
		return false;
	};
	parameters_file.close();

	QDomNodeList paramNodes  = parameters_doc.elementsByTagName ( "param" );
	for ( int i =0; i < paramNodes.size(); i++ ) {
		QDomNode paramNode = paramNodes.at(i);
		if ( paramNode.toElement().tagName() == "param" ) {
			DiscompModuleParameter *param = new DiscompModuleParameter();

			if ( param->Init( paramNode ) )
			{
				param->input = true;

				if ( this->process_type == "single" ) {
					param->dir = this->process_parameters_dir;
					logMessage ( LOG_DEBUG, QString("Init parameter: " + param->name + "; %1; %2" ).arg(param->dir).arg(param->is_defined()) );
				} else {
					param->dir = this->process_parameters_dir + QDir::separator() + "input";
					if ( !param->is_defined() ) { //it's output parameter
						param->input = false;
						param->dir = this->process_parameters_dir + QDir::separator() + "output";
					}
				};

				this->parameters.push_back( param );
			} else {
				logMessage(LOG_ERROR, "Couldn't init all parameters.");
				delete param;

				//TODO delete all
				return false;
			};
		};
	};

	//TODO!!!! HERE WE SHOULD CHECK THAT ALL PARAMETERS PRESENT IN THE MODULES AND IN THE PARAMETRS
	return true;
};

void DiscompProcessJS::stopModuleByName (const QString &module_name)
{
	if ( process_destructing || completed ) { return; };

	logMessage ( LOG_DEBUG, QString("All modules with name '%1' should be stoped on the current stage").arg(module_name) );

	bool found_flag = false;
	for ( int mod_num=0; mod_num < (int)this->modules.size(); mod_num++ )
	{
		DiscompProcessStageModule* module = this->modules[mod_num];

		if ( module->name == module_name ) {
			found_flag = true;
			switch ( module->status ) {
			case MODULE_STATUS_WAIT:// module not executed yet
				module->status = MODULE_STATUS_STOPED;
				break;
			case MODULE_STATUS_STARTED:// we should send termination signal
				module->status = MODULE_STATUS_STOPPING; //stopping

				//this signal will be processed in the discomp_server
				emit processStopModuleExecutionOnNode_signal ( module->executed_on_node, module->name );
				break;
			default:
				break;
			};
		};
	};
	if ( !found_flag ) {
		logMessage ( LOG_WARNING, QString("Couldn't find module '%1' on the current stage").arg(module_name) );
	};
}



void DiscompProcessJS::stopListModuleByListNum ( const QString &module_name, int list_num )
{
}


/*
 * here I check if next stage is list, modules in thus stage should be equal the number of elements of list
 * i.e. to the calculating process should be added modules
 * */



/***********************************************************************/
/*-------------------------- Module execution  ------------------------*/
/***********************************************************************/

/*
 * Check if in the current stage we have some module which can be 
 * executed on the node specified as param. If it's found will be 
 * started procedure for check input parameters 
 * */
QString DiscompProcessJS::giveJobToNode ( DiscompNode* node )
{
	QString mod_name = "";
	for ( int mod_num=0; mod_num < this->modules.size(); mod_num++ ) {
		if ( this->modules[mod_num]->status == MODULE_STATUS_WAIT ) {
			for ( int node_mod_num=0; node_mod_num < node->modules.size(); node_mod_num++ ) {
				if ( (node->modules[node_mod_num]->name == this->modules[mod_num]->name) )
				{
					QString tmp_mod_name = node->modules[node_mod_num]->name;
					if (mod_name == tmp_mod_name) {
						continue;
					} else {
						mod_name = tmp_mod_name;
					}
					DiscompModuleJS *module = this->modules[mod_num];

					if ( ! this->modules[mod_num]->areAllInputParametersExist() ) {
						logMessage ( LOG_COMMON, QString("Skiping module %1, becouse some of input paramaters are not exist yet").arg(mod_name) );
					}
					if ( ! initParametersForNode ( node, module ) ) {
						logMessage ( LOG_WARNING, QString("Failed to initialize parameters for module %1 on node %1").arg(mod_name).arg(node->name) ) ;
					}

					node->setAdditionalLogFile ( this->log_dir + QDir::separator() + node->log_file );

					module->executed_on_node = node->id;
					module->node_name = QString ("%1 (%2)").arg(node->name).arg(node->ip);
					module->node_ping = node->ping_msec;

					module->status = MODULE_STATUS_STARTED;

					QDateTime timestamp (QDateTime::currentDateTime ());
					module->execution_start_time = timestamp.toTime_t ();
					module->execution_start_time_ms.restart();
					//EVENT processing

					if ( this->EDP ) {
						module->processEvent ( "onStart" );
					};

					return mod_name;
				};
			};
		};
	};
	logMessage( LOG_DEBUG, QString("GiveJobToNode returns nothing") );
	return 0;
}

/*
 * Set directories and filenames for input and output parameters of module on the node
 * */
int DiscompProcessJS::initParametersForNode( DiscompNode* node, DiscompModuleJS* module )
{
	QString module_name = module->name;

	//initialize input parameters
	logMessage( LOG_DEBUG, QString("Initializing input parameters for module '%1'").arg(module_name) );

	node->process_list_element_num = 0;
	//for each input parameter of module on node
	int module_num = node->getModuleNumByName(module_name);
	for (int module_param_num = 0; module_param_num < node->modules[module_num]->inputParameters.size(); module_param_num++ ) {
		DiscompModuleParameter *node_module_param = node->modules[module_num]->inputParameters[module_param_num];

		//for each parameter in process space
		bool process_knows_this_parameter = false;
		DiscompModuleParameter *process_param;
		for ( int process_param_num = 0; process_param_num < this->parameters.size(); process_param_num++ ) {
			process_param = this->parameters[process_param_num];
			if ( node_module_param->name == process_param->name ) {
				node_module_param->dir = process_param->dir;
				if ( process_param->type == "file" ) {
					node_module_param->filepath = node_module_param->dir + QDir::separator() + node_module_param->filename;
				} else if ( process_param->type == "filelist" ) {
					if ( node_module_param->type == "file" ) {
						QString filename = QString( process_param->filepattern ).arg( module->list_element_num );
						node->process_list_element_num = module->list_element_num;
						node_module_param->filepath = node_module_param->dir + QDir::separator() + filename;
					}
					else if ( node_module_param->type == "filelist" ) {
						node_module_param->filepath = node_module_param->dir + QDir::separator() + process_param->filepattern;
					}

				}
				process_knows_this_parameter = true;
			}
		}
		if ( ! process_knows_this_parameter ) {
			logMessage( LOG_ERROR, QString("Could't find parameter '%1' in the process parameters space").arg(node_module_param->name) );
			return false;
		}
	}

	//initialize output parameters
	logMessage( LOG_DEBUG, QString("Initializing output parameters for module '%1'").arg(module_name) );

	//give process dir for module. to which dir module should return output parameters
	if ( this->process_type == "single" ) {
		node->modules[module_num]->process_parameters_dir = this->process_parameters_dir;
	} else {
		node->modules[module_num]->process_parameters_dir = QDir::convertSeparators ( QString(PACKAGES_DIR) + "/" + this->package_name + "/" + QString(PROCESSES_DIR) + "/"+ this->scheme_name + "/" + "parameters" + "/" + "output" );
	};

	//for each output parameter on node set dir and filename
	for (int module_param_num = 0; module_param_num < node->modules[module_num]->outputParameters.size(); module_param_num++ )
	{
		DiscompModuleParameter *node_param = node->modules[module_num]->outputParameters[module_param_num];
		DiscompModuleParameter *process_param;
		for ( int process_param_num = 0; process_param_num < this->parameters.size(); process_param_num++ ) {
			process_param = this->parameters[process_param_num];
			if ( node_param->name == process_param->name ) {

				node_param->dir = process_param->dir;
				if ( process_param->type == "filelist" ) {
					if ( node_param->type == "file" ) {
						node_param->filename = QString( process_param->filepattern ).arg( module->list_element_num );
					};
				};
			};
		};
	};
	return true;
};

/*
 * node has been disconnected or other reasons can be present. if this module has calculating some module, 
 * status of this module in calculation process should be setted to 0
 * */
void DiscompProcessJS::clearJobOnNode ( DiscompNode* node ) 
{
}





/* 
 * Mark module as done in the calculation process
 * */
void DiscompProcessJS::finishModuleExecutionOnNode( DiscompNode* node, const QString &module_name )
{
	logMessage(LOG_DEBUG, "finishModuleExecutionOnNode");

	for ( int mod_num=0; mod_num < (int)this->modules.size(); mod_num++ )
	{
		if ( (this->modules[mod_num]->status != MODULE_STATUS_DONE) &&
			 (this->modules[mod_num]->status != MODULE_STATUS_STOPED) &&
			 (this->modules[mod_num]->executed_on_node == node->id) )
		{
			if ( module_name == this->modules[mod_num]->name ) {
				DiscompModuleJS *module = this->modules[mod_num];
				module->status = MODULE_STATUS_DONE;

				QDateTime timestamp (QDateTime::currentDateTime ());
				module->execution_full_time = timestamp.toTime_t () - module->execution_start_time;
				module->execution_real_time = node->modules[ node->getModuleNumByName(module_name) ]->execution_real_time;
				module->execution_real_time_ms = node->modules[ node->getModuleNumByName(module_name) ]->execution_real_time_ms;
				module->execution_full_time_ms = module->execution_start_time_ms.elapsed ();
				module->node_resources_usage_info = node->modules[ node->getModuleNumByName(module_name) ]->node_resources_usage_info;
				module->node_name = QString ("%1 (%2)").arg(node->name).arg(node->ip);
				module->node_ping = node->ping_msec;
				if ( node->stoping_module_execution ) {
					if (  this->EDP ) {
						module->processEvent ( "onStop");
					};
				} else {
					if (  this->EDP ) {
						module->processEvent ( "onFinish" );
					};
				};

			} else {
				logMessage( LOG_ERROR, QString("Fatall error: we receive wrong module id from node. This node(id:%1) should return output parameters for module '%2', but we received output parameter for module '%3'").arg(node->id).arg(this->modules[mod_num]->name).arg(module_name) );
			};

		};
	};

	if ( this->isProcessCompleted () ) {
		emit processFinished_signal ( this->id );
	}

	node->setAdditionalLogFile("");
};

/* 
 * Mark module as done in the calculation process
 * */
void DiscompProcessJS::moduleMaxTimeReached ( DiscompNode* node, const QString &module_name )
{
};


int DiscompProcessJS::setPriority (int priority)
{
	this->priority = priority;
	return 0;
};


/***********************************************************************/
/*-------------------------- Public slots -----------------------------*/
/***********************************************************************/
void DiscompProcessJS::moduleStarted_slot() {
	emit newModulesAvailable_signal();
}
/***********************************************************************/
/*-------------------------- Private functions ------------------------*/
/***********************************************************************/



void DiscompProcessJS::logMessage ( int log_level, QString message ) 
{
	if ( this->log_file != "" ) {
		Log::addMessage ( this->log_file, log_level, "DiscompProcessJS", message);
	} else {
		Log::addMessage ( log_level, "DiscompProcessJS", message);
	};
};

bool DiscompProcessJS::isProcessCompleted () {
	if ( ! this->interpretator.isFinished () ) {
		return false;
	}
	for ( int mod_num = 0; mod_num < this->modules.size(); mod_num++ ) {
		if ( this->modules[mod_num]->status == MODULE_STATUS_WAIT ||
			 this->modules[mod_num]->status == MODULE_STATUS_STARTED) {
			return false;
		}
	}
	return true;
}

// vim: set fenc=utf-8 tabstop=8 :
