// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#include "process_stage.h"
#include "lib/log.h"
#include "lib/xml.h"


/**
 * @class DiscompProcessStage
 * @brief Class for works with stages from DiscompProcess
 *
 * This class allow to works with stages for DiscompProcess. 
 * For each stage, which defined in the process.xml, created
 * exemplar of this class. When stage init from XML specification
 * it's check and load all modules which should be executed on 
 * this stage.
 *
 * \sa DiscompProcessStageModule
 */

/**
 * Class constructor
 */
DiscompProcessStage::DiscompProcessStage(QObject *parent)
   : QObject( parent)
{
	id       = -1;
	done     =  false;
	log_file = "";
	execution_start_time = 0;
	execution_time = 0;
	package_name = "";
}

/**
 * Class destructor
 */
DiscompProcessStage::~DiscompProcessStage()
{
	logMessage( LOG_DEBUG, QString("Destruct process stage [id:%1]").arg(this->id) );
	
	logMessage( LOG_DEBUG, QString("Destruct all modules in this stage [id:%1]").arg(this->id));
	for ( int mod_num=0; mod_num < (int)modules.size(); mod_num++ ) {
		delete modules[mod_num];
	};
};


/**
 * Init stage by XML specification. For ex.
 * \code
 *	<stage>
 *		<module name='decompose'/>
 *		<listmodule name='solver'/>
 *	</stage>
 * \endcode
 * */
int DiscompProcessStage::Init( const QDomNode &stageNode )
{
	logMessage  (LOG_COMMON, QString("Initializing stage %1").arg(this->id) );
	
	this->events["onStart"] = XML::getNodeAttribute("onStart", stageNode);
	if ( this->events["onStart"] != "" ) logMessage ( LOG_COMMON, "   Event onStart: " + this->events["onStart"] );

	this->events["onStop"] = XML::getNodeAttribute("onStop", stageNode);
	if ( this->events["onStop"] != "" ) logMessage ( LOG_COMMON,  "   Event onStop: " + this->events["onStop"] );

	this->events["onFinish"] = XML::getNodeAttribute("onFinish", stageNode);
	if ( this->events["onFinish"] != "" ) logMessage ( LOG_COMMON,"   Event onFinish: " + this->events["onFinish"] );

	this->events["onTimer"] = XML::getNodeAttribute("onTimer", stageNode);
	this->events["onTimerInterval"] = XML::getNodeAttribute("onTimerInterval", stageNode);
	if ( this->events["onTimer"] != "" ) {
		logMessage ( LOG_COMMON, "   Event onTimer: " + this->events["onTimer"] );
		if ( this->events["onTimerInterval"] != "" ) {
			logMessage ( LOG_COMMON, "        onTimerInterval: " + this->events["onTimerInterval"] );
		} else {
			logMessage ( LOG_WARNING, "  Event onTimer specified without onTimerInterval attribute. onTimer event disabled");
			this->events["onTimer"] = "";
		};
	};



	QDomNode moduleNode = stageNode.firstChild();
	while ( !moduleNode.isNull() )
	{
		if ( moduleNode.toElement().tagName() == "module" || moduleNode.toElement().tagName() == "listmodule") {
			DiscompProcessStageModule *new_mod = new DiscompProcessStageModule();
			new_mod->log_file       = this->log_file;
			QString module_name     = moduleNode.toElement().attribute("name");
			// module name should contain package name
			if ( module_name.indexOf (".") == -1 ) {
				module_name = this->package_name + "." + module_name;
			};

			QString module_operation= moduleNode.toElement().attribute("operation");
			//new_mod->max_execution_time = moduleNode.toElement().attribute("max_time").toInt();
			if ( this->process_type == "single" ) {
				new_mod->module_dir = this->single_process_dir;
			};
			
			//it's a list module
			if ( moduleNode.toElement().tagName() == "listmodule" )  {
				new_mod->list_flag = true;
			};
			
			if ( !new_mod->LoadModule ( module_name, module_operation ) ) {
				logMessage( LOG_ERROR, QString("Module initialization failed: %1").arg(module_name) );
				delete new_mod;
				return false;
			};

			//new_mod->list_parameter = moduleNode.toElement().attribute("list_parameter").toInt();

			//events
			new_mod->events["onStart"] = XML::getNodeAttribute("onStart", moduleNode);
			if ( new_mod->events["onStart"] != "" ) 
				logMessage ( LOG_COMMON, QString("[Module: %1]   Event onStart: " + new_mod->events["onStart"]).arg(new_mod->name));

			new_mod->events["onFinish"] = XML::getNodeAttribute("onFinish", moduleNode);
			if ( new_mod->events["onFinish"] != "" ) 
				logMessage ( LOG_COMMON, QString("[Module: %1]   Event onFinish: " + new_mod->events["onFinish"]).arg(new_mod->name));

			new_mod->events["onStop"] = XML::getNodeAttribute("onStop", moduleNode);
			if ( new_mod->events["onStop"] != "" ) 
				logMessage ( LOG_COMMON, QString("[Module: %1]   Event onStop: " + new_mod->events["onStop"]).arg(new_mod->name));
			
			modules.push_back( new_mod );
		};
		moduleNode = moduleNode.nextSibling();
	};
	return true;
};

/**
 * Private function for write log messages from this class
 */
void DiscompProcessStage::logMessage ( int log_level, const QString& message ) 
{
	if ( this->log_file != "" ) {
		Log::addMessage (this->log_file, log_level, "DiscompProcessStage", message);
	} else {
		Log::addMessage (log_level, "DiscompProcessStage", message);
	};
};






/**
 * @class DiscompProcessStageModule
 * @brief Class for works with modules from DiscompProcess
 *
 * This class inherit properties and methods of class DiscompModule
 * The additional variables which defined in this class used by DiscompProcess
 * status of module execution and also processing parallel list parameter.
 * When discomp process is creating it's initialized process stages from process.xml
 * specification. For each stage created exemplars of this class which allow discomp 
 * process save state of each module execution. 
 * If stage contained module which should works with parallel list, for ex. 
 * in the process.xml:
 * \code
 * <listmodule name='solver'>
 * \endcode
 * than created multiple exemplars of this class for each element of list.
 * 
 */
DiscompProcessStageModule::DiscompProcessStageModule() 
{
	status           =  MODULE_STATUS_WAIT;
	executed_on_node = -1;
	//max_execution_time = -1;
	list_flag        =  false;
	list_element_num = -1;
	node_name        = "";
	node_ping        = -1;
};

// vim: set fenc=utf-8 tabstop=8 :
