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
#include "process_plugin_api.h"
#include "lib/log.h"
#include "lib/module_parameter.h"

/**
 * @class DiscompProcessPluginAPI
 */

/***********************************************************************/
/*-----------------  Constructor & Destructor  ------------------------*/
/***********************************************************************/
/**
 * Class constructor
 */
DiscompProcessPluginAPI::DiscompProcessPluginAPI(DiscompProcess *parent)
   : discomp_process( parent )
{
};

/**
 * Class destructor
 */
DiscompProcessPluginAPI::~DiscompProcessPluginAPI()
{
};


/***********************************************************************/
/*---------------------  DiscompAPI methods ---------------------------*/
/***********************************************************************/
/**
 * Get absolute process directory
 * @return process_dir (string)
 */
QString DiscompProcessPluginAPI::getProcessDir () 
{
	return this->discomp_process->absolute_process_dir;
};

/**
 * Get current process stage num
 * @return stage num (int)
 */
int DiscompProcessPluginAPI::getCurrentStage () 
{
	return this->discomp_process->current_stage;
};




/***********************************************************************/
/*---------------------  DiscompAPI methods ---------------------------*/
/***********************************************************************/


/**
 * Stop module by id
 * @param module_name - module identtificator
 */
void DiscompProcessPluginAPI::stopModule ( const QString &module_name ) 
{
	logMessage ( LOG_COMMON, QString("[StopModule] Called stop module by id: %1").arg(module_name) ); 

	discomp_process->stopModuleByName( module_name );
}

/**
 * Stop list module by id & list num
 * @param module_name - module identtificator
 * @param list_num - list number
 */
void DiscompProcessPluginAPI::stopListModule ( const QString &module_name, int list_num ) 
{
	logMessage ( LOG_COMMON, QString("[StopModule] Called stop list module by id: %1, list_num: %2").arg(module_name).arg(list_num) ); 

	discomp_process->stopListModuleByListNum ( module_name, list_num );
}

/**
 * Restart module on the current stage
 */
void DiscompProcessPluginAPI::restartModule ( const QString &module_name )
{
	logMessage ( LOG_COMMON, QString("[reStartModule] Called restart module by id: %1").arg(module_name) ); 


	//TODO
	//discomp_process->stopListModuleByListNum ( module_name, list_num );
}


/**
 * Restart module on the current stage
 */
void DiscompProcessPluginAPI::restartListModule ( const QString &module_name, int list_num )
{
	logMessage ( LOG_COMMON, QString("[reStartModule] Called restart module by id: %1").arg(module_name) ); 

	discomp_process->stopListModuleByListNum ( module_name, list_num );
	//todo.
}

/**
 * Restart module on the current stage
 */
bool DiscompProcessPluginAPI::gotoStage ( int stage_num )
{
	logMessage ( LOG_COMMON, QString("[reStartModule] gotoStage: %1").arg(stage_num) ); 
	discomp_process->gotoStage ( stage_num );

	//TODO
	return true;
};



/**
 * Get file parameter value
 */
QString DiscompProcessPluginAPI::getFileParameterValue ( const QString &parameter_name ) 
{
	for ( int process_param_num=0; process_param_num < (int)discomp_process->parameters.size(); process_param_num++ ) {
		if ( parameter_name == discomp_process->parameters[process_param_num]->name ) {
			if ( discomp_process->parameters[process_param_num]->type == "file" ) { 
				return discomp_process->parameters[process_param_num]->getFileTypeValue();;
			};
		};
	};
	return "";
}

/**
 * Set list parameter value
 */
bool DiscompProcessPluginAPI::setFileParameterValue ( const QString &parameter_name, const QString & value ) 
{
	for ( int process_param_num=0; process_param_num < (int)discomp_process->parameters.size(); process_param_num++ ) {
		if ( parameter_name == discomp_process->parameters[process_param_num]->name ) {
			if ( discomp_process->parameters[process_param_num]->type == "file" ) { 
				return discomp_process->parameters[process_param_num]->setFileTypeValue(value);
			};
		};
	};
	return false;
}



/**
 * Get list parameter size
 */
int DiscompProcessPluginAPI::getListParameterSize ( const QString &parameter_name ) 
{
	for ( int process_param_num=0; process_param_num < (int)discomp_process->parameters.size(); process_param_num++ ) {
		if ( parameter_name == discomp_process->parameters[process_param_num]->name ) {
			if ( discomp_process->parameters[process_param_num]->type == "filelist" ) { 
				return discomp_process->parameters[process_param_num]->listsize(1);
			};
		};
	};
	return -1;
}



/**
 * Get process parameter (type:list)
 */
QString DiscompProcessPluginAPI::getListParameterValue ( const QString &parameter_name, int list_num ) 
{
	for ( int process_param_num=0; process_param_num < (int)discomp_process->parameters.size(); process_param_num++ ) {
		if ( parameter_name == discomp_process->parameters[process_param_num]->name ) {
			if ( discomp_process->parameters[process_param_num]->type == "filelist" ) { 
				return discomp_process->parameters[process_param_num]->getListTypeValue(list_num);
			};
		};
	};
	return "";
}


/**
 * Set list parameter value
 */
bool DiscompProcessPluginAPI::setListParameterValue ( const QString &parameter_name, int list_num, const QString & value ) 
{
	for ( int process_param_num=0; process_param_num < (int)discomp_process->parameters.size(); process_param_num++ ) {
		if ( parameter_name == discomp_process->parameters[process_param_num]->name ) {
			if ( discomp_process->parameters[process_param_num]->type == "filelist" ) { 
				return discomp_process->parameters[process_param_num]->setListTypeValue(list_num, value);
			};
		};
	};
	return false;
}




/**
 * Write log message to the process log file.
 * This method used only from DiscompAPI.logMessage()
 * @param message - log message
 */
void DiscompProcessPluginAPI::logMessage ( const QString& message ) 
{
	if ( this->discomp_process->log_file != "" ) {
		Log::addMessage (this->discomp_process->log_file, LOG_COMMON, "DiscompProcessPluginAPI", "[DiscompAPI] [Log] " + message);
	} else {
		Log::addMessage ( LOG_COMMON, "DiscompProcessPluginAPI", "[DiscompAPI] [Log] " + message);
	};
};




/***********************************************************************/
/*-----------------------  Private functions  -------------------------*/
/***********************************************************************/

/**
 * Private function for write log messages from this class
 * @param log_level - log level from lib/log.h
 * @param message - log message 
 */
void DiscompProcessPluginAPI::logMessage ( int log_level, const QString& message ) 
{
	if ( this->discomp_process->log_file != "" ) {
		Log::addMessage ( this->discomp_process->log_file, log_level, "DiscompProcessPluginAPI", "[DiscompAPI] " + message);
	} else {
		Log::addMessage ( log_level, "DiscompProcessPluginAPI", "[DiscompAPI] " + message);
	};
};



// vim: set fenc=utf-8 tabstop=8 :
