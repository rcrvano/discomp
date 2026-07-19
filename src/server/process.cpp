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


/**
 * @class DiscompProcess
 * @brief Calculating Process
 *
 * This is a base class for works with calculating proces
 */

/***********************************************************************/
/*-----------------  Constructor & Destructor  ------------------------*/
/***********************************************************************/

#include "process_plugin.h"

/*
 * Constructor
 * */
DiscompProcess::DiscompProcess(QObject *parent)
    : DiscompProcessPrivate( parent )
{
	id        = -1;
	name      = "";
	log_dir   = "";
	log_file  = "";
	

	completed = false;
	process_destructing = false;
	process_type = "xml";

	execution_start_time = 0;
//	start_timestamp = 0; 


	commands_finished = "";
	control_point = NULL;
	current_stage = 0;

	priority = 0;
/*
	plugin_filename = "";
	plugin_loaded   = false;
	plugin_loader   = NULL;
	plugin          = NULL;
*/
	//plugin = NULL;
	plugin = new DiscompProcessPlugin( this );

	logMessage( LOG_DEBUG, "Construct new process class");
}


/*
 * Destructor
 * */
DiscompProcess::~DiscompProcess()
{
	logMessage( LOG_DEBUG, "Destruct process class");

	process_destructing = true;

	if ( this->plugin != NULL ) {
		delete this->plugin;
	};


	logMessage( LOG_DEBUG, "Mark free all nodes in this calculating process");
	if ( this->stages.size() > this->current_stage ) {
		for ( int mod_num=0; mod_num < (int)this->stages[this->current_stage]->modules.size(); mod_num++ ) {
			if ( this->stages[this->current_stage]->modules[mod_num]->status == MODULE_STATUS_STARTED ) {
				emit processFreeNode_signal ( this->id, this->stages[this->current_stage]->modules[mod_num]->executed_on_node, this->stages[this->current_stage]->modules[mod_num]->name );
			};
		};
	};

	logMessage( LOG_DEBUG, "Destruct process stages");
	for ( int stage_num=0; stage_num < (int)this->stages.size(); stage_num++ ) {
		delete this->stages[stage_num];
	};

	logMessage( LOG_DEBUG, "Destruct process parameters");
	for ( int param_num=0; param_num < (int)this->parameters.size(); param_num++ ) {
		delete this->parameters[param_num];
	};

	//delete control point
	if ( control_point != NULL ) {
//		this->control_point->Delete();
		delete control_point;
	};
};



/***********************************************************************/
/*---------------  Calculating process functions  ---------------------*/
/***********************************************************************/


/*
 * Initialization of the calculating process
 * */
int DiscompProcess::Init ( const QString &process_name, const QString &user )
{
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

//	QString process_file_path = QDir::convertSeparators ( absolute_process_dir + "/" + QString("%1").arg(SCHEME_CONFIG_FILE) );
//	if ( !QFile::exists(process_file_path ) ) {
//		return -2;
//	};


	this->process_parameters_dir= QDir::convertSeparators ( absolute_process_dir + "/" + PARAMETERS_DIR );

	//log
	log_dir  = absolute_process_dir + QDir::separator() + "log" + QDir::separator();
	File::recursiveRemoveDirectoryContent ( log_dir );
	this->log_file = log_dir + QDir::separator() + QString("process.%1").arg(LOG_FILE_EXT);
	
	logMessage ( LOG_COMMON, "Process initialization: " + process_name + "");
	logMessage ( LOG_DEBUG,  "Process log directory: " + log_file );


	logMessage ( LOG_DEBUG, "Clear process failedmodules directory" );
	File::recursiveRemoveDirectoryContent ( absolute_process_dir + QDir::separator() +  "failedmodules" );

	

	//init process.xml
	QString process_file_path = QDir::convertSeparators ( absolute_process_dir + "/" + QString("%1").arg(SCHEME_CONFIG_FILE) );
	if ( !this->_initProcessFile ( process_file_path ) ) {
		return false;
	};


	//init parameters
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

	

	
	//init control point class
	this->control_point = new DiscompProcessControlPoint(this);
	this->control_point->log_file = this->log_file;
	this->control_point->controlpoint_file = this->absolute_process_dir+"/controlpoints.xml";
	this->control_point->process_plan_file = process_file_path;


	//here we check if we need make a resume from control point. if yes than
	//don't remove output parameters.

	bool resume = ( this->control_point->isExists() ); // && stagesNode.toElement().attribute("controlpoints").toInt() );
	//bool clear_output_parameters = true;
	logMessage(LOG_COMMON, QString("Controlpoints resume flag: %1").arg(resume));
	if ( !resume ) {
		//clear parameters output directory
		logMessage ( LOG_COMMON, "Clear output parameters directory" );
		File::recursiveRemoveDirectoryContent( this->process_parameters_dir + QDir::separator() + "output" );
	};


	if ( resume ) {
		logMessage(LOG_DEBUG, "trying to resume..");
		if ( !this->control_point->Resume() ) {
			logMessage(LOG_DEBUG, "resume impossible..");
		};
	};
	this->control_point->Start();
	this->control_point->Create();





	//if first stage - it's a list, then we should reInit this stage
	this->rebuildStageModulesForParallelList();



	
	logMessage( LOG_COMMON, "Plan Initialized successfull");	
	logMessage( LOG_COMMON, QString("--------------------------------------------------") );


	if ( checkCurrentStageIsDone() == -1 )  { return false; };

	return true;
};




int DiscompProcess::InitSingleTask ( const QString &process_dir, const QString &user )
{
	this->name = process_dir;

	logMessage ( LOG_WARNING, "It's a single process. Dir is: " + this->name );

	//check user 
	this->user_name = user;

	
	//get package name and scheme name
	this->package_name = "system";
	this->scheme_name  = "single";

	this->process_type = "single";
	
	//paths
	this->absolute_process_dir  = process_dir;
	this->process_parameters_dir= process_dir;
	
	//log
	log_dir  = process_dir + QDir::separator() + "log" + QDir::separator();
	this->log_file = log_dir + QDir::separator() + QString("process.%1").arg(LOG_FILE_EXT);
	
	logMessage(LOG_COMMON, "===============================================" );
	logMessage(LOG_COMMON, "Process initialization" );


	//init process.xml
	File::save ( process_dir + QDir::separator() + "process.xml", "<?xml version='1.0' encoding='UTF-8'?><process><stages><stage><module name='single'/></stage></stages></process>" );

	QString file_path = QDir::convertSeparators ( process_dir + QDir::separator() + "process.xml" );
	if ( !this->_initProcessFile ( file_path ) ) {
		return false;
	};

	logMessage(LOG_DEBUG, "Init parameters" );
	//init parameters.xml
	QString param_file_path = process_dir + QDir::separator() + "module.xml";
	if ( !this->_initParametersFile ( param_file_path ) ) {
		return false;
	};
	//we need add 'module.xml' parameter which need to be transfered to the node
	DiscompModuleParameter *param = new DiscompModuleParameter();
	param->input	= true;
	param->dir	= this->process_parameters_dir;
	param->type     = "file";
	param->name     = "module.xml";
	param->filename = "module.xml";
	this->parameters.push_back( param );


	
	//init control point class
	this->control_point = new DiscompProcessControlPoint(this);
	this->control_point->log_file = this->log_file;
	this->control_point->controlpoint_file = this->absolute_process_dir+"/controlpoints.xml";
	this->control_point->Start();
	this->control_point->Create();



	logMessage( LOG_COMMON, "Plan Initialized successfull");	
	logMessage( LOG_COMMON, QString("--------------------------------------------------") );


	if ( checkCurrentStageIsDone() == -1 )  { return false; };

	return true;

}




bool DiscompProcess::_initProcessFile ( const QString& file_path ) 
{
	//read process.xml
	QFile file ( file_path );
	if  ( !file.open( QIODevice::ReadOnly ) ) {
		logMessage( LOG_ERROR, QString("Could't open file %1 for reading").arg(file_path) );
		return false;
	};


	//read speciification
	QString errorStr;
	int errorLine;
	int errorColumn;
	
	QDomDocument doc;
	if ( !doc.setContent( &file, true, &errorStr, &errorLine, &errorColumn ) ) {
		logMessage ( LOG_ERROR, QString("XML processing error for process.xml ( line %1, column %2: %3 )").arg(errorLine).arg(errorColumn).arg(errorStr) );
		return false;
	};
	file.close();


	QDomElement rootNode = doc.documentElement();
	if (rootNode.tagName() != "process") {
		logMessage( LOG_ERROR, "The root node of process.xml should be <process>");
		return false;
	};


	//load  plugin
	QDomNode pluginNode = XML::getChildNodeByTag( "plugin", rootNode );
	if ( !pluginNode.isNull() ) {
		plugin->Init( pluginNode );
	};

	//get info about process commands (before start/stop)
	QDomNode commandsNode   = XML::getChildNodeByTag( "commands", rootNode );
	this->commands_finished = XML::getTextNodeByTag ( "finished", commandsNode );


	//get information about calculation process stages
	logMessage(LOG_DEBUG, "Loading process stages..");
	QDomNode stagesNode = XML::getChildNodeByTag( "stages", rootNode );
	if ( stagesNode.isNull() ) {
		logMessage(LOG_ERROR, "Couldn't find stages node tag in the process.xml");
		return false;
	};
	this->current_stage = stagesNode.toElement().attribute("start_stage").toInt();
	//TODO!!!!!!!!!check current stage. it's received from input file. and server can be damaged.
	QDomNode stageNode  = stagesNode.firstChild();
	while ( !stageNode.isNull() )
	{
		if ( stageNode.toElement().tagName() == "stage" ) {
			
			DiscompProcessStage *stage = new DiscompProcessStage();
			stage->log_file     = this->log_file;
			stage->package_name = this->package_name; 
			stage->process_type = this->process_type;
			stage->single_process_dir  = this->absolute_process_dir;
			stage->id           = this->stages.size();
			if ( !stage->Init(stageNode) ) {
				//TODO!
				delete stage;
				return false;
			};
			this->stages.push_back(stage);
		};
		stageNode = stageNode.nextSibling();
	};



	QDateTime timestamp (QDateTime::currentDateTime ());
	this->stages[this->current_stage]->execution_start_time =  timestamp.toTime_t ();

	this->execution_start_time = timestamp.toTime_t ();
	


	return true;
};



bool DiscompProcess::_initParametersFile ( const QString &file_path )
{
	//read parameters which will be used in the calculating process
	//
	
	logMessage(LOG_DEBUG, "Get information about parameters which will be used in the calculating process");
	logMessage(LOG_COMMON, "Init process global parameters.");


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
					logMessage (LOG_COMMON, QString("Parameter (name:%1; dir:current; type:%2)").arg(param->name).arg(param->type).arg(""));
					logMessage ( LOG_DEBUG, QString("Init parameter: " + param->name + "; %1; %2" ).arg(param->dir).arg(param->is_defined()) );
				} else {
					param->dir = this->process_parameters_dir + QDir::separator() + "input";
					if ( !param->is_defined() ) { //it's output parameter
						param->input = false;
						param->dir = this->process_parameters_dir + QDir::separator() + "output";
						//logMessage (LOG_COMMON, param->dir);
						logMessage (LOG_COMMON, QString("Parameter (name:%1; dir:%3; type:%2;)").arg(param->name).arg(param->type).arg("output"));
					} else {
						//logMessage (LOG_COMMON, param->dir);
						logMessage (LOG_COMMON, QString("Parameter (name:%1; dir:%3; type:%2;)").arg(param->name).arg(param->type).arg("input"));
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



















int DiscompProcess::setPriority ( int new_pr )
{
	this->priority = new_pr;
};



void DiscompProcess::collectStatistic()
{
	//don't write to common, because it's add many times in logs
	logMessage(LOG_DEBUG, "Collect stats"); 

	QString statistics_xml = "<?xml version='1.0' encoding='UTF-8'?>\n";

	//QDateTime timestamp (QDateTime::currentDateTime ()); 
	//logMessage(LOG_COMMON, QString("%1").arg(timestamp.toTime_t ()));

	QDateTime timestamp (QDateTime::currentDateTime ());
	uint execution_time = timestamp.toTime_t () - this->execution_start_time;
	statistics_xml += QString("<statistics time_s='%1'>\n").arg(execution_time);

	for ( int stage_num=0; stage_num < (int)this->stages.size(); stage_num++ ) {
		statistics_xml += QString("\t<stage num='%1' done='%2' time_s='%3'>\n").arg(stage_num).arg(this->stages[stage_num]->done).arg(this->stages[stage_num]->execution_time);

		for ( int mod_num=0; mod_num < (int)this->stages[stage_num]->modules.size(); mod_num++ ) 
		{
			QString  mod_name   = this->stages[stage_num]->modules[mod_num]->name;
			int mod_status      = this->stages[stage_num]->modules[mod_num]->status;
			int mod_list        = this->stages[stage_num]->modules[mod_num]->list_flag;
			int mod_list_el_num = this->stages[stage_num]->modules[mod_num]->list_element_num;
			uint mod_real_time  = this->stages[stage_num]->modules[mod_num]->execution_real_time;
			uint mod_full_time  = this->stages[stage_num]->modules[mod_num]->execution_full_time;
			uint mod_real_time_ms = this->stages[stage_num]->modules[mod_num]->execution_real_time_ms;
			uint mod_full_time_ms = this->stages[stage_num]->modules[mod_num]->execution_full_time_ms;
			QString usage_info    = this->stages[stage_num]->modules[mod_num]->node_resources_usage_info;

			QString node_name   = "";
			QString node_ping   = "";
			if ( mod_status == MODULE_STATUS_DONE  || mod_status == MODULE_STATUS_STARTED || mod_status == MODULE_STATUS_STOPED ) { 
				node_name  = "node='"+this->stages[stage_num]->modules[mod_num]->node_name+"' ";
				node_name += "node_ping='"+QString("%1").arg(this->stages[stage_num]->modules[mod_num]->node_ping)+"' ";
			};

			if ( mod_list ) {
				statistics_xml += QString("\t\t<module name='%1' status='%2' " + node_name + node_ping + "list='%3' list_num='%4' real_time_s='%5' full_time_s='%6' real_time_ms='%7' full_time_ms='%8' usage_info='%9'/>\n").arg(mod_name).arg(mod_status).arg(mod_list).arg(mod_list_el_num).arg(mod_real_time).arg(mod_full_time).arg(mod_real_time_ms).arg(mod_full_time_ms).arg(usage_info);
			} else {
				statistics_xml += QString("\t\t<module name='%1' status='%2' " + node_name + node_ping + "real_time_s='%3' full_time_s='%4' real_time_ms='%5' full_time_ms='%6' usage_info='%7'/>\n").arg(mod_name).arg(mod_status).arg(mod_real_time).arg(mod_full_time).arg(mod_real_time_ms).arg(mod_full_time_ms).arg(usage_info);
			};
		};

		statistics_xml += "\t</stage>\n";
	};

	statistics_xml += "</statistics>\n";

	if ( !File::save(this->absolute_process_dir+"/statistic.xml", statistics_xml) ) {
		Log::addMessage( LOG_WARNING, "File", "Couldn't open file " + this->absolute_process_dir+"/statistic.xml" + " for writing" ) ;
	}
};


/**
 * Used in the controlpoints
 */
int DiscompProcess::jumpToTheStage(int stage)
{
	this->current_stage = stage;
	this->rebuildStageModulesForParallelList();
	return 1;
};

int DiscompProcess::gotoStage(int stage)
{
	if ( stage > this->current_stage ) {
		return  0;
	};
	//first stop all modules on the current stage
	//
	for ( int stage_num = stage; stage_num <= this->current_stage ; stage_num++ ) { 
		for ( int mod_num=0; mod_num < (int)this->stages[stage_num]->modules.size(); mod_num++ ) 
		{

			if ( this->stages[stage_num]->modules[mod_num]->status == MODULE_STATUS_STARTED ) {
				emit processFreeNode_signal ( this->id, this->stages[stage_num]->modules[mod_num]->executed_on_node, this->stages[stage_num]->modules[mod_num]->name );
			};

			DiscompProcessStageModule* module = this->stages[stage_num]->modules[mod_num];
			module->status= MODULE_STATUS_WAIT;
			module->executed_on_node = -1;
		};
	};




	this->current_stage = stage;

	logMessage( LOG_COMMON, QString("--------------------------------------------------") );
	logMessage( LOG_COMMON, QString("Stage: %1").arg(this->current_stage));


	this->rebuildStageModulesForParallelList();
	this->checkCurrentStageIsDone();

	return 1;
};

/*
 * If all modules on the current stage has been calculated, 
 * then we can go to next stage
 * */
int DiscompProcess::checkCurrentStageIsDone()
{
	if ( process_destructing ) { return false; };

	// stage.type - modules
	logMessage(LOG_DEBUG, "Check current stage is done:" );

//	if ( this->stages[this->current_stage]->type == "modules" ) {
	/*	
	this->stages[this->current_stage]->stop_after ...
	check nuber of calculated modules. 
	if true, start stoping all execution(emit signal). all not done mark as done.
	if (
	*/


	
	for ( int mod_num=0; mod_num < (int)this->stages[this->current_stage]->modules.size(); mod_num++ ) 
	{
		if ( this->stages[this->current_stage]->modules[mod_num]->status != MODULE_STATUS_DONE &&
		     this->stages[this->current_stage]->modules[mod_num]->status != MODULE_STATUS_STOPED  ) 
		{
		     
			//logMessage(LOG_DEBUG, QString("NOT DONE: Stage:%1, mod_num:%2").arg(this->current_stage).arg(mod_num)  );
			return false;
		};
	};

	//stage completed. rebuild output parameters. if it'sparallel list then all parameters should be defined correctly: param1, param2, etc.
	rebuildStageOutputParameters();


	this->stages[this->current_stage]->done = true;

	QDateTime timestamp (QDateTime::currentDateTime ());
	this->stages[this->current_stage]->execution_time = timestamp.toTime_t () - this->stages[this->current_stage]->execution_start_time;

	logMessage( LOG_COMMON, QString("Stage %1 finished").arg(this->current_stage) );


	if ( this->stages[this->current_stage]->events["onFinish"] != "" ) {
		plugin->processStageEvent ( current_stage, "onFinish" );
	};

	logMessage( LOG_COMMON, QString("--------------------------------------------------") );

	logMessage( LOG_DEBUG,  QString("Number of stages:%1").arg(this->stages.size()) );

	if ( this->current_stage == (int)this->stages.size() - 1 ) { //it's last stage
		//calculating process completed
		logMessage( LOG_COMMON, "Calculating process completed");
		completed = true;

		this->collectStatistic();

		//executing command after finishing process
		if ( this->commands_finished  != "" ) {
			logMessage ( LOG_COMMON, "Executing: " + this->absolute_process_dir + QDir::separator() + this->commands_finished );
			QProcess sys_process;
			sys_process.setWorkingDirectory(this->absolute_process_dir);
			sys_process.start (  this->absolute_process_dir + QDir::separator() + this->commands_finished ); 
			sys_process.waitForFinished();
		};
		
		//emmit signal that finished
		emit processFinished_signal (this->id);
		return -1;
	};
	


	this->control_point->Create();
	this->collectStatistic();


	this->current_stage++;
	logMessage( LOG_COMMON, QString("Stage: %1").arg(this->current_stage));

	this->stages[this->current_stage]->execution_start_time =  timestamp.toTime_t ();
	
	this->rebuildStageModulesForParallelList();

	if ( this->stages[this->current_stage]->events["onStart"] != "" ) {
		plugin->processStageEvent ( current_stage, "onStart" );
	};





	//TODO. create timer. onTimer event here
	//and stop it when stage finished
	
	return true;
};


//this method executed when all modules on the currecnt stage are completed
void DiscompProcess::rebuildStageOutputParameters ()
{
	//TODO. what to do with modules which used for parallel list?
	//maybe remove them first?
	//and what will be with statistics?
	if ( process_destructing || completed ) { return; };

	logMessage(LOG_DEBUG, "rebuildStageOutputParameters()");
	//logMessage(LOG_DEBUG, QString("Modules szie on the current stage:%1").arg(this->stages[this->current_stage]->modules.size()));

	/*
	for ( int mod_num=0; mod_num < (int)this->stages[this->current_stage]->modules.size(); mod_num++ ) 
	{
		//logMessage(LOG_DEBUG, QString("mod_num:%1").arg(mod_mum) );
		for (int module_param_num=0; module_param_num < (int)this->stages[this->current_stage]->modules[mod_num]->outputParameters.size(); module_param_num++ ) 
		{
			DiscompModuleParameter *param = this->stages[this->current_stage]->modules[mod_num]->outputParameters[module_param_num];

			//TODO!!! NOW here we should find the same parameter in the global parameters space for this process 
			//and check his parameteter type, because param->type for module can be a file, but in the process it's a filelist
			//logMessage( LOG_DEBUG, QString("param id:%1, name:%2, type:%3").arg(param->id).arg(param->name).arg(param->type) );


			//search module parameter in the process parameters space
			for ( int process_param_num=0; process_param_num < (int)this->parameters.size(); process_param_num++ ) {			
				if ( param->id == this->parameters[process_param_num]->id ) {

					//logMessage( LOG_DEBUG, QString("param id:%1, name:%2, type:%3").arg(param->id).arg(param->name).arg(param->type) );
					//logMessage( LOG_DEBUG, QString("global param id:%1, name:%2, type:%3").arg(this->parameters[process_param_num]->id).arg(this->parameters[process_param_num]->name).arg(this->parameters[process_param_num]->type) );
					if ( this->parameters[process_param_num]->type == "filelist" ) {
						logMessage( LOG_DEBUG, "found list parameter" );

						int list_parameter_num = getParameterNumById( this->stages[this->current_stage]->modules[mod_num]->list_parameter );
						if ( !list_parameter_num || list_parameter_num == -1 ) {
							continue;
						};

						//int list_size          = this->parameters[list_parameter_num]->listsize();

						//TODO . why it's not checked all list parameters???
						// tile for dsat

						//TODO. comment here?
						//this->parameters[process_param_num]->checkListParameterSequence( );
					};
				};
			};
		};
	};
	*/

	//check all input parameters. maybe it's redefined in the output dir?
	for ( int process_param_num=0; process_param_num < (int)this->parameters.size(); process_param_num++ ) {
		DiscompModuleParameter *param = this->parameters[process_param_num];
		if ( param->input ) {
			//check defined in the output
			param->dir = process_parameters_dir + QDir::separator() + "output";
			if ( param->is_defined(1) ) {
				param->input = false;
			} else {
				//set dir back to input
				param->dir = process_parameters_dir + QDir::separator() + "input";
			}
		};
	};
};

void DiscompProcess::stopModuleByName (const QString &module_name)
{
	if ( process_destructing || completed ) { return; };

	logMessage ( LOG_DEBUG, QString("All modules with name '%1' should be stoped on the current stage").arg(module_name) );
	
	bool found_flag = false;
	for ( int mod_num=0; mod_num < (int)this->stages[this->current_stage]->modules.size(); mod_num++ ) 
	{
		DiscompProcessStageModule* module = this->stages[this->current_stage]->modules[mod_num];

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



void DiscompProcess::stopListModuleByListNum ( const QString &module_name, int list_num )
{
	if ( process_destructing || completed ) { return; };

	logMessage ( LOG_DEBUG, QString("Stop list module %1, list_num: %2").arg(module_name).arg(list_num) );
	
	bool found_flag = false;
	for ( int mod_num=0; mod_num < (int)this->stages[this->current_stage]->modules.size(); mod_num++ ) 
	{
		DiscompProcessStageModule* module = this->stages[this->current_stage]->modules[mod_num];

		if ( module->name == module_name && module->list_flag && module->list_element_num == list_num ) {
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
		logMessage ( LOG_WARNING, QString("Couldn't find module '%1', list_num: %2 on the current stage").arg(module_name).arg(list_num) );
	};
}


/*
 * here I check if next stage is list, modules in thus stage should be equal the number of elements of list
 * i.e. to the calculating process should be added modules
 * */
void DiscompProcess::rebuildStageModulesForParallelList()
{
	if ( process_destructing || completed ) { return; };

	for ( int mod_num=0; mod_num < (int)this->stages[this->current_stage]->modules.size(); mod_num++ ) {
		if ( this->stages[this->current_stage]->modules[mod_num]->list_flag == true && 
		     this->stages[this->current_stage]->modules[mod_num]->list_element_num == -1 ) //if this module hasn't already generated in this block
		{
			logMessage( LOG_DEBUG, "This stage contained list.");

			
			// we need detect list parameter for listmodule
			int list_size = 0;
			for ( int module_param_num=0; 
			          module_param_num < (int)this->stages[this->current_stage]->modules[mod_num]->inputParameters.size();
				  module_param_num++ ) 
			{
		
				DiscompModuleParameter *param = this->stages[this->current_stage]->modules[mod_num]->inputParameters[module_param_num];
		
				//search module parameter in the process parameters space
				for ( int process_param_num=0; process_param_num < (int)this->parameters.size(); process_param_num++ ) {
			
					if ( param->name == this->parameters[process_param_num]->name ) {
						//we found parameter in the process parameter space. 
				
						if ( this->parameters[process_param_num]->type == "filelist" ) {

							//remote node receive element of the list
							if ( param->type == "file" ) {
								list_size = this->parameters[process_param_num]->listsize();

								logMessage ( LOG_COMMON, QString("List parameter for module '%1' is '%2' (elements:%3)").
									arg(this->stages[this->current_stage]->modules[mod_num]->name).
									arg(param->name).arg(list_size));
							};
						};
					};

				};
			};

			//if list is empty, then skip this stage
			if ( list_size == 0 ) {
				this->stages[this->current_stage]->modules[mod_num]->status = MODULE_STATUS_DONE;
				return;
			};
			
			//this stage module I init by first element of list
			//and create new modules in this stage for all elements of list
			
			//one module already created when stage has been initialized.
			//now I should init other modules
			//
			logMessage( LOG_DEBUG, QString("Add new modules to the stages list. Hoping to the best. list size is: %1").arg(list_size) );
			this->stages[this->current_stage]->modules[mod_num]->list_element_num = 0;
			for ( int list_num=1; list_num < list_size; list_num ++ ) {
				
				DiscompProcessStageModule *new_mod = new DiscompProcessStageModule();
				new_mod->name             = this->stages[this->current_stage]->modules[mod_num]->name;
				new_mod->list_parameter   = this->stages[this->current_stage]->modules[mod_num]->list_parameter;
				new_mod->list_flag        = true;
				new_mod->list_element_num = list_num;
				new_mod->events           = this->stages[this->current_stage]->modules[mod_num]->events;

				this->stages[this->current_stage]->modules.push_back(new_mod);
			};
		};
	};

	this->control_point->Create();
	this->collectStatistic();
};




/***********************************************************************/
/*-------------------------- Module execution  ------------------------*/
/***********************************************************************/

/*
 * Check if in the current stage we have some module which can be 
 * executed on the node specified as param. If it's found will be 
 * started procedure for check input parameters 
 * */
QString DiscompProcess::giveJobToNode ( DiscompNode* node )
{
	if ( process_destructing || completed ) { return 0; };

	checkCurrentStageIsDone();

	if ( process_destructing || completed ) { return 0; };

	
	//TODO. blya. modulei mojet bit' v 100 raz bol'she chem uzlov.
	//i imenno poetomu smotrim na moduli. a ne dlya kajdogo svobodnogo usla.
	//
	for ( int mod_num=0; mod_num < (int)this->stages[this->current_stage]->modules.size(); mod_num++ ) {
		if ( this->stages[this->current_stage]->modules[mod_num]->status == MODULE_STATUS_WAIT ) {
			for ( int node_mod_num=0; node_mod_num < (int)node->modules.size(); node_mod_num++ ) {
				if ( (node->modules[node_mod_num]->name == this->stages[this->current_stage]->modules[mod_num]->name) &&
				     (!node->modules[node_mod_num]->is_failed_module) )
				{
					
					QString mod_name = node->modules[node_mod_num]->name;

					if ( this->process_type == "single" && node->modules[node_mod_num]->name == "system.single" ) {
						logMessage ( LOG_COMMON, "Reloading system.single module specification");
						node->modules[node_mod_num]->module_dir = this->absolute_process_dir;
						node->modules[node_mod_num]->LoadModule ("system.single");
					};


					if ( !initParametersForNode ( node, mod_name, this->stages[this->current_stage]->modules[mod_num] ) ) {
						return false;
					};

					node->setAdditionalLogFile ( this->log_dir + QDir::separator() + node->log_file );
					
					this->stages[this->current_stage]->modules[mod_num]->executed_on_node = node->id;
					this->stages[this->current_stage]->modules[mod_num]->node_name = QString ("%1 (%2)").arg(node->name).arg(node->ip);
					this->stages[this->current_stage]->modules[mod_num]->node_ping = node->ping_msec;

					this->stages[this->current_stage]->modules[mod_num]->status = MODULE_STATUS_STARTED;

					QDateTime timestamp (QDateTime::currentDateTime ());
					this->stages[this->current_stage]->modules[mod_num]->execution_start_time = timestamp.toTime_t ();
					this->stages[this->current_stage]->modules[mod_num]->execution_start_time_ms.restart();

					//EVENT processing
					if ( this->stages[this->current_stage]->modules[mod_num]->events["onStart"] != "" ) {
						plugin->processModuleEvent ( current_stage, mod_num, "onStart" );
					};

						
					return mod_name;
				};
			};
		};
	};

	return 0;
};

/*
 * node has been disconnected or other reasons can be present. if this module has calculating some module, 
 * status of this module in calculation process should be setted to 0
 * */
void DiscompProcess::clearJobOnNode ( DiscompNode* node ) 
{
	if ( process_destructing || completed ) { return; };

	for ( int mod_num=0; mod_num < (int)this->stages[this->current_stage]->modules.size(); mod_num++ ) {
		if ( this->stages[this->current_stage]->modules[mod_num]->executed_on_node == node->id &&
		     this->stages[this->current_stage]->modules[mod_num]->status == MODULE_STATUS_STARTED ) 
		{
			this->stages[this->current_stage]->modules[mod_num]->executed_on_node = -1;
			this->stages[this->current_stage]->modules[mod_num]->status = MODULE_STATUS_WAIT;

			QDateTime timestamp (QDateTime::currentDateTime ());
		};
	};
	node->setAdditionalLogFile("");
}




/*
 * Check if all input parameters for module is defined
 * and set paths to the input files in the node module defenition
 * */
int DiscompProcess::initParametersForNode( DiscompNode* node, const QString &module_name, DiscompProcessStageModule* stage_module )
{
	//TODO. what log level??
	logMessage( LOG_DEBUG, QString("Check input parameters for module '%1'").arg(module_name) );

	
	int module_num = node->getModuleNumByName(module_name);
	
	node->process_list_element_num = 0;

	//first check and init all input parameters
	bool all_parametes_defined_flag = true;
	for (int module_param_num=0; module_param_num < (int)node->modules[module_num]->inputParameters.size(); module_param_num++ ) {
		
		DiscompModuleParameter *param = node->modules[module_num]->inputParameters[module_param_num];
		
		bool parameter_founded_flag = false;
		
		//search module parameter in the process parameters space
		for ( int process_param_num=0; process_param_num < (int)this->parameters.size(); process_param_num++ ) {
			
			if ( param->name == this->parameters[process_param_num]->name ) {
				//we found parameter in the process parameter space. 
				
				//chech exists
				if ( !this->parameters[process_param_num]->is_defined() ) {
					logMessage( LOG_ERROR, QString("Parameter '%1' isn't defined. Module could't be started").arg(this->parameters[process_param_num]->name) );
					all_parametes_defined_flag = false;
				};

				
				if ( this->parameters[process_param_num]->type == "file" ) {
					param->dir      = this->parameters[process_param_num]->dir;
					param->filepath = param->dir + QDir::separator() +param->filename;
					
				} else if ( this->parameters[process_param_num]->type == "filelist" ) {

					//remote node receive element of the list
					if ( param->type == "file" ) {
						
						QString filename = QString(this->parameters[process_param_num]->filepattern).arg(stage_module->list_element_num);
						node->process_list_element_num = stage_module->list_element_num;
						param->dir       = this->parameters[process_param_num]->dir;
						param->filepath  = param->dir + QDir::separator() + filename;

					} 
					//remote node receive this list as input parameter
					else if ( param->type == "filelist" ) {
						param->dir       = this->parameters[process_param_num]->dir;
						param->filepath  = param->dir + QDir::separator() + this->parameters[process_param_num]->filepattern;
					};
				
				};
				
				parameter_founded_flag = true;
				
			};

		};
				
		if ( !parameter_founded_flag ) {
			logMessage( LOG_WARNING, QString("Could't find parameter '%1' in the process parameters space").arg(param->name) );
			all_parametes_defined_flag = false;
		};
				
	};
				



	
	if ( !all_parametes_defined_flag ) {
		logMessage( LOG_WARNING, QString("Prepare failed because some input parameters in not found for executing module %1").arg(module_name)) ;
		
		return false;
	};
	//all input parameters defined
		
	
	//give process dir for module. to which dir module should return output parameters
	if ( this->process_type == "single" ) {
		node->modules[module_num]->process_parameters_dir = this->process_parameters_dir;
	} else {
		node->modules[module_num]->process_parameters_dir = QDir::convertSeparators ( QString(PACKAGES_DIR) + "/" + this->package_name + "/" + QString(PROCESSES_DIR) + "/"+ this->scheme_name + "/" + "parameters" + "/" + "output" ); 
	};


	//prepare output parameters
	//here if module receive element of the list and out parameter it's element of list too, 
	//then I set the filename to which remote host should save this element on the server side
	for (int module_param_num=0; module_param_num < (int)node->modules[module_num]->outputParameters.size(); module_param_num++ ) 
	{
		DiscompModuleParameter *param = node->modules[module_num]->outputParameters[module_param_num];
		//param->dir = process_parameters_dir + QDir::separator() + "output";
		
		//search module parameter in the process parameter space
		for ( int process_param_num=0; process_param_num < (int)this->parameters.size(); process_param_num++ ) {

			if ( param->name == this->parameters[process_param_num]->name ) {

				param->dir      = this->parameters[process_param_num]->dir;
				if ( this->parameters[process_param_num]->type == "filelist" ) {

					//remote node generate element of the list
					if ( param->type == "file" ) {
						//local for check
						param->filename = QString(this->parameters[process_param_num]->filepattern).arg(stage_module->list_element_num);
						//create empty file
						param->touch();

						//and remote
						//param->remote_filename = QString(this->parameters[process_param_num]->filepattern).arg(stage_module->list_element_num);
					};
				};
			};
		};
	};


	
	//TODO. what log level??
	logMessage( LOG_DEBUG, QString("Looks good. All input parameters for module '%1' defined in the process parameters space" ).arg(module_name) );
	return true;
};


/* 
 * Mark module as done in the calculation process
 * */
void DiscompProcess::finishModuleExecutionOnNode( DiscompNode* node, const QString &module_name )
{
	logMessage(LOG_DEBUG, "finishModuleExecutionOnNode");

	//for what this loop??
	//maybe here should be some break?
	for ( int mod_num=0; mod_num < (int)this->stages[this->current_stage]->modules.size(); mod_num++ ) 
	{
		if ( (this->stages[this->current_stage]->modules[mod_num]->status != MODULE_STATUS_DONE) && 
		     (this->stages[this->current_stage]->modules[mod_num]->status != MODULE_STATUS_STOPED) && 
		     (this->stages[this->current_stage]->modules[mod_num]->executed_on_node == node->id) ) 
		{
			if ( module_name == this->stages[this->current_stage]->modules[mod_num]->name ) {
				this->stages[this->current_stage]->modules[mod_num]->status = MODULE_STATUS_DONE;
			
				QDateTime timestamp (QDateTime::currentDateTime ());
				this->stages[this->current_stage]->modules[mod_num]->execution_full_time = timestamp.toTime_t () - this->stages[this->current_stage]->modules[mod_num]->execution_start_time;
				this->stages[this->current_stage]->modules[mod_num]->execution_real_time = node->modules[ node->getModuleNumByName(module_name) ]->execution_real_time;
				this->stages[this->current_stage]->modules[mod_num]->execution_full_time_ms = this->stages[this->current_stage]->modules[mod_num]->execution_start_time_ms.elapsed ();
				this->stages[this->current_stage]->modules[mod_num]->execution_real_time_ms = node->modules[ node->getModuleNumByName(module_name) ]->execution_real_time_ms;
				this->stages[this->current_stage]->modules[mod_num]->node_resources_usage_info = node->modules[ node->getModuleNumByName(module_name) ]->node_resources_usage_info;
				this->stages[this->current_stage]->modules[mod_num]->node_name = QString ("%1 (%2)").arg(node->name).arg(node->ip);
				this->stages[this->current_stage]->modules[mod_num]->node_ping = node->ping_msec;

				if ( node->stoping_module_execution ) {
					if ( this->stages[this->current_stage]->modules[mod_num]->events["onStop"] != "" ) {
						plugin->processModuleEvent ( current_stage, mod_num, "onStop", 1 );
					};
				} else {
					if ( this->stages[this->current_stage]->modules[mod_num]->events["onFinish"] != "" ) {
						plugin->processModuleEvent ( current_stage, mod_num, "onFinish" );
					};
				};

			} else {
				logMessage( LOG_ERROR, QString("Fatall error: we receive wrong module id from node. This node(id:%1) should return output parameters for module '%2', but we received output parameter for module '%3'").arg(node->id).arg(this->stages[this->current_stage]->modules[mod_num]->name).arg(module_name) );
			};
			
		};
	};
	node->setAdditionalLogFile("");
			

	this->control_point->Create();
	this->collectStatistic();

	checkCurrentStageIsDone();
};

/* 
 * Mark module as done in the calculation process
 * */
void DiscompProcess::moduleMaxTimeReached ( DiscompNode* node, const QString &module_name )
{
	logMessage(LOG_DEBUG, "Max module time reached function!");

	for ( int mod_num=0; mod_num < (int)this->stages[this->current_stage]->modules.size(); mod_num++ ) 
	{
		if ( (this->stages[this->current_stage]->modules[mod_num]->status != MODULE_STATUS_DONE) && 
		     (this->stages[this->current_stage]->modules[mod_num]->status != MODULE_STATUS_STOPED) && 
		     (this->stages[this->current_stage]->modules[mod_num]->executed_on_node == node->id) ) 
		{
			if ( module_name == this->stages[this->current_stage]->modules[mod_num]->name ) {
				
				this->stages[this->current_stage]->modules[mod_num]->status = MODULE_STATUS_STOPED;

				//timers
				QDateTime timestamp (QDateTime::currentDateTime ());
				this->stages[this->current_stage]->modules[mod_num]->execution_full_time = timestamp.toTime_t () - this->stages[this->current_stage]->modules[mod_num]->execution_start_time;
				this->stages[this->current_stage]->modules[mod_num]->execution_real_time = node->modules[ node->getModuleNumByName(module_name) ]->execution_real_time;
				this->stages[this->current_stage]->modules[mod_num]->execution_full_time_ms = this->stages[this->current_stage]->modules[mod_num]->execution_start_time_ms.elapsed ();
				this->stages[this->current_stage]->modules[mod_num]->execution_real_time_ms = node->modules[ node->getModuleNumByName(module_name) ]->execution_real_time_ms;
				this->stages[this->current_stage]->modules[mod_num]->node_resources_usage_info = node->modules[ node->getModuleNumByName(module_name) ]->node_resources_usage_info;

				if ( this->stages[this->current_stage]->modules[mod_num]->events["onStop"] != "" ) {
					plugin->processModuleEvent ( current_stage, mod_num, "onStop", 2 );
				};
			} else {
				logMessage( LOG_ERROR, QString("Fatal error: we receive wrong module id from node. This node(id:%1) should return output parameters for module '%2', but we received output parameter for module '%3'").arg(node->id).arg(this->stages[this->current_stage]->modules[mod_num]->name).arg(module_name) );
			};
			
		};
	};
	node->setAdditionalLogFile("");
			

	this->control_point->Create();
	this->collectStatistic();

	checkCurrentStageIsDone();
};






/******************************* Plugin functions ***************************/
/**
 * Loading process plugin
 */
/*
int DiscompProcess::loadPlugin ()
{
	QString plugin_path = this->absolute_process_dir + QDir::separator() + this->plugin_filename;

	logMessage( LOG_COMMON, QString("Loading plugin: %1").arg(plugin_path) );

	plugin_loader = new QPluginLoader(plugin_path);
	logMessage( LOG_COMMON, "1");
	if ( !plugin_loader->load() || !plugin_loader->isLoaded() ) {
	logMessage( LOG_COMMON, "2");
	try {
		logMessage( LOG_ERROR, "Could't load plugin library. Error: " + plugin_loader->errorString () );
	}  catch (int e) {};
	logMessage( LOG_COMMON, "3");
		this->unloadPlugin ();
	logMessage( LOG_COMMON, "4");
		return false;
	};
	logMessage( LOG_COMMON, "5");
	this->plugin = plugin_loader->instance();
	logMessage( LOG_COMMON, "6");

	if ( !plugin ) {
	logMessage( LOG_COMMON, "7");
		logMessage ( LOG_ERROR, "Could't get plugin instance" );
	logMessage( LOG_COMMON, "8");
		this->unloadPlugin ();
	logMessage( LOG_COMMON, "9");
		return false;
	};

	logMessage( LOG_COMMON, "10");
	connect ( plugin, SIGNAL ( pluginLogMessage_signal (const QString&) ),
	          this,   SLOT   ( pluginLogMessage_slot   (const QString&) ) );
	logMessage( LOG_COMMON, "11");
	connect ( plugin, SIGNAL ( pluginStopModuleByListNum_signal ( int ) ),
	          this,   SLOT   ( pluginStopModuleByListNum_slot   ( int ) ) );

	logMessage( LOG_COMMON, "12");
	plugin_interface = qobject_cast<DiscompProcessPluginInterface *>(plugin);
	plugin_interface->constructor(this);
	plugin_interface->setParametersDir(this->process_parameters_dir);
	logMessage( LOG_COMMON, "13");

	logMessage ( LOG_COMMON, "Plugin sucessfully loaded" );

	logMessage( LOG_COMMON, "14");
	plugin_loaded = true;

	return true;
};

int DiscompProcess::unloadPlugin ()
{
	if ( this->plugin_loaded ) {
		plugin_interface->destructor();
	};
	logMessage(LOG_COMMON, "Plugin sucessfully unloaded. But plugin still in memory. If you will change plugin lib you should restart discomp server." );
	return true;
};


void DiscompProcess::pluginLogMessage_slot ( const QString& message )
{
	if ( this->log_file != "" ) {
		Log::addMessage (this->log_file, LOG_COMMON, "DiscompProcessPlugin", "[plugin] "+ message);
	};
}
void DiscompProcess::pluginStopModuleByListNum_slot ( int list_num )  
{
	for ( int mod_num=0; mod_num < (int)this->stages[this->current_stage]->modules.size(); mod_num++ ) {
		if ( this->stages[this->current_stage]->modules[mod_num]->list_flag == true &&
		     this->stages[this->current_stage]->modules[mod_num]->list_element_num == list_num 
		   ) {

			if ( this->stages[this->current_stage]->modules[mod_num]->status == MODULE_STATUS_STARTED ) {
		   		logMessage( LOG_COMMON, QString("Received signal for stop module by list num: %1, node_id: %2").arg(list_num).arg(this->stages[this->current_stage]->modules[mod_num]->executed_on_node) );

				emit processStopModuleExecutionOnNode_signal ( this->stages[this->current_stage]->modules[mod_num]->executed_on_node, this->stages[this->current_stage]->modules[mod_num]->id );
			};
		};
	};

};
*/

/******************************* /Plugin functions ***************************/




/***********************************************************************/
/*-------------------------- Private functions ------------------------*/
/***********************************************************************/

/*
 * search parameter by id in the process parameters space 
 * */
int DiscompProcess::getParameterNumByName( const QString &param_name )
{
	int paramNum = -1;
	
	for ( int param_num=0; param_num < (int)this->parameters.size(); param_num++ ) {
		if ( this->parameters[param_num]->name == param_name ) {
			paramNum = param_num;
			break;
		};
	};

	logMessage(LOG_DEBUG, QString("param_name:%1, paramNum:%2").arg(param_name).arg(paramNum) );

	return paramNum;
};


void DiscompProcess::logMessage ( int log_level, QString message ) 
{
	if ( this->log_file != "" ) {
		Log::addMessage ( this->log_file, log_level, "DiscompProcess", message);
	} else {
		Log::addMessage ( log_level, "DiscompProcess", message);
	};
};

// vim: set fenc=utf-8 tabstop=8 :
