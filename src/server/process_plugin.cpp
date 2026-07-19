// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#include "process.h"
#include "process_plugin.h"
//#include "process_plugin_api.h"

#include "lib/log.h"
#include "lib/xml.h"
#include "lib/common.h"
#include "lib/module_parameter.h"
//#include <qsutilfactory.h>

Q_SCRIPT_DECLARE_QMETAOBJECT(QFile, QObject*)
//SCRIPT_DECLARE_QMETAOBJECT(QDir,  QObject*)


/**
 * @class DiscompProcessPlugin
 * @todo write descriptions here
 */



/***********************************************************************/
/*-----------------  Constructor & Destructor  ------------------------*/
/***********************************************************************/
/**
 * Class constructor
 */
DiscompProcessPlugin::DiscompProcessPlugin(DiscompProcess *parent)
   : discomp_process (parent)
{
	// create DiscompAPI object
	discomp_api = new DiscompProcessPluginAPI(discomp_process);
	
	// create Interpeter
	script_engine = new QScriptEngine();

	initialized = false;
}

/**
 * Class destructor
 */
DiscompProcessPlugin::~DiscompProcessPlugin()
{
	delete discomp_api;
	delete script_engine;
};


/**
 * Init interpreter & evalute plugin code
 */
int DiscompProcessPlugin::Init ( const QDomNode &pluginNode ) 
{
	logMessage ( LOG_COMMON, "Loading plugin: " + pluginNode.toElement().attribute("src") );
	QString plugin_file = this->discomp_process->absolute_process_dir + QDir::separator() + pluginNode.toElement().attribute("src");
	QString plugin_code;
	if ( !File::read ( plugin_file, plugin_code ) ) {
		logMessage ( LOG_COMMON, "Plugin load error. Couln't read file: " + plugin_file ); 
		return false;
	};

	// attach DiscompAPI object to the script engine interpreter
	QScriptValue discomp_api_sv = script_engine->newQObject(discomp_api);
	script_engine->globalObject().setProperty("DiscompAPI", discomp_api_sv);

//	QScriptValue dirClass = script_engine->newQObject(new QDir);
//	script_engine->globalObject().setProperty("Dir", dirClass);

	QScriptValue fileClass = script_engine->scriptValueFromQMetaObject<QFile>();
	script_engine->globalObject().setProperty("QFile", fileClass);

	// evaluate script
	if ( !this->evaluateScript ( plugin_code ) ) {
		logMessage ( LOG_COMMON, "Plugin code execution error. Plugin functionality will be disabled" );
		return false;
	};

	initialized =true;
	return true;
};

bool DiscompProcessPlugin::evaluateScript( const QString & script ) 
{
	QScriptValue result = script_engine->evaluate( script );
	if ( script_engine->hasUncaughtException() ) {
		int line = script_engine->uncaughtExceptionLineNumber();
		logMessage(LOG_ERROR, QString("Uncaught exception at line %1 : %2").arg(line).arg(result.toString()));
		return false;
	}
	return true;
}


/**
 * Set global variables for the interpreter
 * like current module num, current_stage, etc.
 */
void DiscompProcessPlugin::setDiscompAPIEnviroment( int stage_num, int module_num, const QString& event, int reason )
{
}

/***********************************************************************/
/*----------------- Plugin calls / process events ---------------------*/
/***********************************************************************/
/**
 * Process module event:
 * - onStart
 * - onFinish
 * - onStop
 */
void DiscompProcessPlugin::processModuleEvent ( int stage_num, int module_num, const QString& event, int reason ) 
{
	if ( !initialized ) { 
		logMessage ( LOG_DEBUG, QString("Couldn't call %1 event, because plugin code not loaded correctly").arg(event) );
		return; 
	};

	
	DiscompProcessStageModule *module = discomp_process->stages[stage_num]->modules[module_num];
	QString event_handler = module->events[event];

	if ( event_handler == "" ) {
		logMessage(LOG_WARNING, QString("DiscompProcessPlugin::processModuleEvent called but event handler isn't defined (module:%1)").arg(module->name) );
		return;
	};

	logMessage (LOG_DEBUG, QString("[Event module %1] id:%2, event handler: %3").arg(event).arg(module->name).arg(event_handler));
	this->setDiscompAPIEnviroment(stage_num, module_num, event, reason);

	this->evaluateScript ( event_handler );
};

/** 
 * Process stage event
 * - onStart
 * - onFinish
 * - onStop
 * - onTimer
 */
void DiscompProcessPlugin::processStageEvent ( int stage_num, const QString& event, int reason ) 
{
	if ( !initialized ) { return; };

	DiscompProcessStage *stage = discomp_process->stages[stage_num];
	QString event_handler = stage->events[event];

	if ( event_handler == "" ) {
		logMessage(LOG_WARNING, QString("DiscompProcessPlugin::processStageEvent called but event handler isn't defined (stage_num:%1)").arg(stage_num) );
		return;
	};

	logMessage (LOG_DEBUG, QString("[Event stage %1] stage_num:%2, event handler: %3").arg(event).arg(stage_num).arg(event_handler));
	this->setDiscompAPIEnviroment(stage_num, -1, event, reason);

	this->evaluateScript ( event_handler );
};





/***********************************************************************/
/*-----------------------  Private functions  -------------------------*/
/***********************************************************************/

/**
 * Private function for write log messages from this class
 */
void DiscompProcessPlugin::logMessage ( int log_level, const QString& message ) 
{
	if ( this->discomp_process->log_file != "" ) {
		Log::addMessage (this->discomp_process->log_file, log_level, "DiscompProcessPlugin", "[DiscompAPI] " + message);
	} else {
		Log::addMessage (log_level, "DiscompProcessPlugin", "[DiscompAPI] " + message);
	};
};

// vim: set fenc=utf-8 tabstop=8 :
