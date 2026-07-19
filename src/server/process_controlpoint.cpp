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
#include "process_controlpoint.h"
#include "lib/log.h"

/**
 * @class DiscompProcessControlPoint
 * @brief Control points subsystem for calculating process.
 *
 * All parameters for calculating process storied in the parameters dir
 * Output parameters (which was calculated by some modules) storied in
 * the parameters/output directory. This is allow to restore calculating
 * process state from stage on which it was created. And all modules which
 * was done on this stage will be marked as done.
 * I.e. will be restoried process state on the moment when controlpoints
 * was created
 */      

/***********************************************************************/
/*-----------------  Constructor & Destructor  ------------------------*/
/***********************************************************************/

/**
 * Constructor
 */
DiscompProcessControlPoint::DiscompProcessControlPoint(QObject *parent)
    : QObject( parent )
{
	this->discomp_process_ptr  = (DiscompProcess*)parent;
	this->process_plan_md5_sum = "";
	this->controlpoint_file    = "/dev/null";
	this->process_plan_file    = "/dev/null";
	
	logMessage( LOG_DEBUG, "Constructed ProcessControlPoint class");
}


/**
 * Destructor
 */
DiscompProcessControlPoint::~DiscompProcessControlPoint()
{
	logMessage( LOG_DEBUG, "Destruct ProcessControlPoint class");
};



/***********************************************************************/
/*-------------------------- Public functions -------------------------*/
/***********************************************************************/

/**
 * Check if controlpoints file exists
 */
int DiscompProcessControlPoint::isExists()
{
	QFile file(this->controlpoint_file);
	return file.exists();
};

/**
 * Start control points system
 */
int DiscompProcessControlPoint::Start()
{
	this->process_plan_md5_sum = File::getMD5(this->process_plan_file);

	this->Delete();
	
	logMessage(LOG_COMMON, "Start contol points subsystem");

	return true;
};

/**
 * Create controlpoint
 */
int DiscompProcessControlPoint::Create()
{
	if ( this->process_plan_md5_sum == "" ) { 
		return false;
	};
	//don't write to common, because it's add many times in logs
	logMessage(LOG_DEBUG, "Create control point"); 

	QString controlpoint_xml = "<?xml version='1.0' encoding='UTF-8'?>\n";

	controlpoint_xml += QString("<controlpoint process_plan_md5='%1'>\n").arg(this->process_plan_md5_sum);
	controlpoint_xml += QString("\t<stage num='%1'>\n").arg(this->discomp_process_ptr->current_stage);

	for ( int mod_num=0; mod_num < (int)this->discomp_process_ptr->stages[this->discomp_process_ptr->current_stage]->modules.size(); mod_num++ ) 
	{
		QString mod_name    = this->discomp_process_ptr->stages[this->discomp_process_ptr->current_stage]->modules[mod_num]->name;
		int mod_status      = this->discomp_process_ptr->stages[this->discomp_process_ptr->current_stage]->modules[mod_num]->status;
		int mod_list        = this->discomp_process_ptr->stages[this->discomp_process_ptr->current_stage]->modules[mod_num]->list_flag;
		int mod_list_el_num = this->discomp_process_ptr->stages[this->discomp_process_ptr->current_stage]->modules[mod_num]->list_element_num;

		if ( mod_status != 2 ) { mod_status = 0; };

		if ( mod_list ) {
			controlpoint_xml += QString("\t\t<module name='%1' status='%2' list='%3' list_num='%4'/>\n").arg(mod_name).arg(mod_status).arg(mod_list).arg(mod_list_el_num);
		} else {
			controlpoint_xml += QString("\t\t<module name='%1' status='%2'/>\n").arg(mod_name).arg(mod_status);
		};
	};

	controlpoint_xml += "\t</stage>\n";
	controlpoint_xml += "</controlpoint>\n";

	if ( !File::save(this->controlpoint_file, controlpoint_xml) ) {
		Log::addMessage( LOG_WARNING, "File", "Couldn't open file " + this->controlpoint_file + " for writing" ) ;
	}

	return true;
};

/**
 * Delete controlpoints file
 */
int DiscompProcessControlPoint::Delete()
{
	logMessage(LOG_COMMON, "Delete controlpoints file");
	
	QFile file(this->controlpoint_file);
	file.remove();

	return true;
};



/**
 * Resume calculation process from last control pont 
 **/
int DiscompProcessControlPoint::Resume()
{
	logMessage(LOG_COMMON, "Resume calculating process from last control point");

	if ( !this->discomp_process_ptr ) {
		logMessage(LOG_WARNING, "discomp_process_ptr is NULL");
		return false;
	};





	//first read processes/controlpoints.xml
	QString file_path = this->controlpoint_file;
	QFile file( file_path );
	if  ( !file.open( QIODevice::ReadOnly ) ) {
		logMessage( LOG_WARNING, QString("Could't open file %1 for reading").arg(file_path) );
		return false;
	};

	QString errorStr;
	int errorLine;
	int errorColumn;
	
	QDomDocument doc;
	if ( !doc.setContent( &file, true, &errorStr, &errorLine, &errorColumn ) ) {
		logMessage ( LOG_ERROR, QString("XML processing error ( line %1, column %2: %3 )").arg(errorLine).arg(errorColumn).arg(errorStr) );
		return false;
	};
	file.close();


	QDomElement rootNode = doc.documentElement();
	if (rootNode.tagName() != "controlpoint") {
		logMessage( LOG_ERROR, "The root node of controlpoints file should be <controlpoint>");
		return false;
	};


	//check md5 sum of process.xml from control point file and current md5
	QString md5_new = File::getMD5(this->process_plan_file);
	QString md5_old = rootNode.toElement().attribute("process_plan_md5");
//TODO
//	if ( md5_new != md5_old ) {
//		logMessage(LOG_WARNING, "The process.xml file was modified after last controlpoint. Resume from controlpoint is impossible");
//		logMessage(LOG_DEBUG, QString(" Current process.xml MD5:%1 should be MD5:%2").arg(md5_new).arg(md5_old));
//		return false;
//	};




	//get information about calculation process stages
	QDomNode stageNode = XML::getChildNodeByTag( "stage", rootNode );
	this->discomp_process_ptr->jumpToTheStage(stageNode.toElement().attribute("num").toInt());
	logMessage(LOG_COMMON, QString("Last stage in control points file is %1. Process will be started from this stage").arg(this->discomp_process_ptr->current_stage) );


	logMessage (LOG_COMMON, QString("Restore modules state for stage: %1").arg(this->discomp_process_ptr->current_stage) );

	QDomNode moduleNode  = stageNode.firstChild();
	int mod_num = 0;
	while ( !moduleNode.isNull() )
	{
		if ( moduleNode.toElement().tagName() == "module" ) 
		{
			if ( mod_num >= this->discomp_process_ptr->stages[this->discomp_process_ptr->current_stage]->modules.size() ) {
				break;
			};

			QString mod_name    = this->discomp_process_ptr->stages[this->discomp_process_ptr->current_stage]->modules[mod_num]->name;
			int mod_list        = this->discomp_process_ptr->stages[this->discomp_process_ptr->current_stage]->modules[mod_num]->list_flag;
			int mod_list_el_num = this->discomp_process_ptr->stages[this->discomp_process_ptr->current_stage]->modules[mod_num]->list_element_num;

			if ( moduleNode.toElement().attribute("name") != mod_name ) {
 				logMessage(LOG_WARNING, "Modules order in the controlpoints file doesn't match with order of current process.");
				logMessage(LOG_WARNING, QString("Should be mod_name:%1. got mod_name:%2. Resume impossible").arg(mod_name).arg(moduleNode.toElement().attribute("name")));
				this->resetModulesState ();
				this->discomp_process_ptr->jumpToTheStage(0);
				return false;
			};

			if ( mod_list && (mod_list_el_num != moduleNode.toElement().attribute("list_num").toInt()) ) {
				logMessage(LOG_WARNING, "List parameter num is differ. Resume impossible");
				this->resetModulesState ();
				this->discomp_process_ptr->jumpToTheStage(0);
				return false;
			};



			if ( mod_list ) {
				logMessage(LOG_DEBUG, QString("Set status for module %1[list:%2] = %3").arg(mod_name).arg(mod_list_el_num).arg(moduleNode.toElement().attribute("status")) );
			} else {
				logMessage(LOG_DEBUG, QString("Set status for module %1 = %2").arg(mod_name).arg(moduleNode.toElement().attribute("status")) );
			};

			this->discomp_process_ptr->stages[this->discomp_process_ptr->current_stage]->modules[mod_num]->status = moduleNode.toElement().attribute("status").toInt();
			
			mod_num++;
		};
		moduleNode = moduleNode.nextSibling();
	};

	logMessage ( LOG_COMMON, "Resume from controlpoints completed");

	return true;
};



/***********************************************************************/
/*-------------------------- Private functions ------------------------*/
/***********************************************************************/

/**
 * Set all modules status as "not done"
 * if resume waw aborted
 */
void DiscompProcessControlPoint::resetModulesState ()
{
	for ( int mod_num=0; mod_num < (int)this->discomp_process_ptr->stages[this->discomp_process_ptr->current_stage]->modules.size(); mod_num++ ) {
		this->discomp_process_ptr->stages[this->discomp_process_ptr->current_stage]->modules[mod_num]->status = 0;
	};

}

/**
 * Private function for write log messages from this class
 */
void DiscompProcessControlPoint::logMessage ( int log_level, const QString &message ) 
{
	if ( this->log_file != "" ) {
		Log::addMessage (this->log_file, log_level, "DiscompProcessControlPoint", message);
	} else {
		Log::addMessage ( log_level, "DiscompProcessControlPoint", message);
	};
};

// vim: set fenc=utf-8 tabstop=8 :
