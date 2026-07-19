// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Slastnoy Konstantin <qeadzc777@yandex.ru>                  |
// +--------------------------------------------------------------------+


#include "module_js.h"

/**
 * @class DiscompModuleJS
 * @brief Discomp module JS object class
 * 
 * An interface between javascript's process scheme and DiscompModule.
 *
 */



/***********************************************************************/
/*-----------------  Constructor & Destructor  ------------------------*/
/***********************************************************************/

/** 
 * Constructor. 
 */
DiscompModuleJS::DiscompModuleJS(QString name, DiscompProcessJS *parentProcess) :
	DiscompProcessStageModule()
{
	log_file = "";
	this->LoadModule( name );
	this->parentProcess = parentProcess;

	//set list_flag if this module has "filelist" type input parameter in scheme, but "file" type parameter in module description
	for (int proc_param_num = 0; proc_param_num < parentProcess->parameters.size(); proc_param_num++) {
		for (int mod_param_num = 0; mod_param_num < this->inputParameters.size(); mod_param_num++) {
			if (parentProcess->parameters[proc_param_num]->name == this->inputParameters[mod_param_num]->name) {
				if (parentProcess->parameters[proc_param_num]->type == "filelist" &&
					this->inputParameters[mod_param_num]->type == "file") {
					this->list_flag = true;
				}
			}
		}
	}

	this->status = -1; // not ready for start yet.
	parentProcess->modules.push_back(this);
	connect ( this,          SIGNAL (moduleStarted_signal ()),
			  parentProcess, SLOT   (moduleStarted_slot ()) );
	parentModule = NULL;
};

/*used to construct subModules for list flaged ModuleJS
*/
DiscompModuleJS::DiscompModuleJS(DiscompProcessJS *parentProcess, DiscompModuleJS *parentModule, QString name, int list_num) :
	DiscompProcessStageModule()
{
	this->parentProcess = parentProcess;
	this->parentModule = parentModule;
	this->status = -1; // not ready for start yet
	this->name = name;
	this->list_flag = true;
	this->list_element_num = list_num;
}

/**
 * Destructor
 */
DiscompModuleJS::~DiscompModuleJS()
{
};



/***********************************************************************/
/*----------------------  Module interface to script-------------------*/
/***********************************************************************/



/**
 * Check if defined all input parameters for module execution
 * */
int DiscompModuleJS::isReady()
{
    bool params_defined = true;
    for ( int param_num; param_num < this->inputParameters.size(); param_num++ ) {
	if ( !this->inputParameters[param_num]->is_defined()) {
	    params_defined = false;
	    break;
	}
    }
    return params_defined;
}

/**
 * Start module on the client side.
 * */
int DiscompModuleJS::start()
{
    if (this->list_flag) {
	logMessage(LOG_ERROR, QString("You could not start module %1 by function Start(). Use StartList(int listNum) instead").arg(this->name));
	return false;
    }
    this->status = MODULE_STATUS_WAIT;
	emit moduleStarted_signal();
    return true;
}

/**
 * Start module with list_num on the client side.
 * */
int DiscompModuleJS::startElement(int list_num)
{
    if (this->list_flag == false) {
	logMessage(LOG_ERROR, QString("You could not start module %1 by function StartList(int). Use Start() instead").arg(this->name));
	return false;
    }

    //check may be we already construct sub module in addEventListener
	if (!this->sub_modules.contains(list_num)) {
		this->sub_modules[list_num] = new DiscompModuleJS(parentProcess, this, this->name, list_num);
    }

    this->sub_modules[list_num]->status = MODULE_STATUS_WAIT;
    parentProcess->modules.append(this->sub_modules[list_num]);
	emit moduleStarted_signal();
    return true;
}

/**
 * Stop module execution on the client side
 */
int DiscompModuleJS::stop()
{	
    logMessage ( LOG_COMMON, QString("This module - %1 wants to stop.").arg(this->name) );

    parentProcess->stopModuleByName( this->name );
    return true;
}

void DiscompModuleJS::logMessage ( int log_level, const QString &message )
{
	if ( this->log_file != "" ) {
		Log::addMessage ( this->log_file, log_level, "DiscompModuleJS", message);
	} else {
		Log::addMessage ( log_level, "DiscompModuleJS", message);
	};
};

/**
 * wait for module finishing
 * */
int DiscompModuleJS::waitForFinished(int interval)
{
	if (parentProcess->EDP) {
		logMessage(LOG_COMMON, "waitForFinished(): you should not use this function in EDP mode");
	}
	if (this->list_flag) {//Если модуль параллельного типа то проверяем все подмодули
		bool finished = false;
		while (!finished) {
			finished = true;
			QMapIterator<int, DiscompModuleJS*> sub_module(sub_modules);
			while (sub_module.hasNext()) {
				sub_module.next();
				if (sub_module.value()->status != MODULE_STATUS_DONE) {
					finished = false;
					logMessage(LOG_COMMON, QString("waitForFinished: module #%1 has not finished").arg(sub_module.value()->list_element_num));
					break;
				}
			}
			if (!finished) {
				logMessage(LOG_COMMON, QString("waitForFinished: %1 sleeping").arg(this->name));
								usleep(interval);
			}
		}
	} else {
		while (this->status != MODULE_STATUS_DONE) {
						usleep(interval);
		}
	}
	return 0;
}

int DiscompModuleJS::waitForElementFinished(int element, int interval)
{
	if (parentProcess->EDP) {
		logMessage(LOG_COMMON, "waitForElementFinished(): you should not use this function in EDP mode");
	}
		if (!this->list_flag) {
				logMessage(LOG_ERROR, "waitForElementFinished: wrong type of module. I need list type.");
				return 1;
		}
		while (true) {
				if (this->sub_modules[element]->status == MODULE_STATUS_DONE) {
						break;
				} else {
						logMessage(LOG_COMMON, QString("waitForFinished: %1[%2] sleeping").arg(this->name).arg(element));
#ifdef Q_WS_WIN
						Sleep(interval);
#else
						usleep(interval);
#endif
				}
		}

		return 0;
}

int DiscompModuleJS::getStatus()
{
	if (!this->list_flag) {
		logMessage(LOG_ERROR, "getStatus: wrong type of module. I need simple type.");
		return -1;
	}
	return this->status;
}

int DiscompModuleJS::getElementStatus(int list_num)
{
	if (!this->list_flag) {
		logMessage(LOG_ERROR, "getStatus: wrong type of module. I need list type.");
		return -1;
	}
	return this->sub_modules[list_num]->status;
}

bool DiscompModuleJS::setEventListener(QString event, QString script, int list_num) {
	if (event != "onFinish" &&
	   event != "onStart" &&
	   event != "onStop") {
	   logMessage(LOG_ERROR, "addEventListener: you are trying to use unsupported event. Supported are onFinish, onStart, onStop");
	   return false;
	}

	if (list_num == -1 && this->list_flag) {
	   //logMessage(LOG_COMMON, "addEventListener: setting for this module you should set list_num parameter greater than 0");
	   this->events[event] = script;
	}
	if (list_num > -1 && this->list_flag) {
	   //check may be we already construct sub module in StartList
	   if (!this->sub_modules.contains(list_num)) {
		   this->sub_modules[list_num] = new DiscompModuleJS(parentProcess, this, this->name, list_num);
	   }
	   this->sub_modules[list_num]->events[event] = script;
	} else {
	   this->events[event] = script;
	}
	return true;
}


/**************************************
	Module public functions
***************************************/
/**
 * Process module event:
 * - onFinish
 * - onStop
 */
void DiscompModuleJS::processEvent ( const QString& event )
{
	QString event_handler = this->events[event];

	if (this->parentModule != NULL ) {
		//Экземпляр модуля опрашивает родителя
		this->parentModule->processEvent(event);
	} else {
		//родительский модуль
		if (event == "onFinish" && event_handler != "") {
			int status = MODULE_STATUS_DONE;
			int i = this->sub_modules.size() - 1;
			//если не все модули выполнились выходим
			while (i >= 0) {
				if (this->sub_modules[i]->status != status) {
					return;
				}
				i--;
			}
		}
	}


	if ( event_handler == "" ) {
		logMessage(LOG_WARNING, QString("DiscompModuleJS::processEvent called but event handler isn't defined (module:%1)").arg(this->name) );
		return;
	};

	logMessage (LOG_DEBUG, QString("[Event module %1] id:%2, event handler: %3").arg(event).arg(this->name).arg(event_handler));
	this->parentProcess->evaluateScript ( event_handler );
};

bool DiscompModuleJS::areAllInputParametersExist() {
	logMessage( LOG_DEBUG, QString("Checking input parameters for module '%1'").arg(this->name) );
	bool result = true;
	for (int module_param_num = 0; module_param_num < this->inputParameters.size(); module_param_num++ ) {
		DiscompModuleParameter *param = this->inputParameters[module_param_num];
		QString directory = param->dir;
		//Ищем параметры в input/ и в output/
		param->dir = parentProcess->process_parameters_dir + QDir::separator() + "input";
		if ( !param->is_defined() ) {
			param->dir = parentProcess->process_parameters_dir + QDir::separator() + "output";
			if ( !param->is_defined() ) {
				result = false;
			}
		}
		//устанавливаем директорию назад
		param->dir = directory;
	}
	return result;
}
