// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+

#include "config.h"
#include "client.h"
#include "sysinfo.h"
#include "lib/xml.h"
#include "lib/common.h"
#include "lib/file.h"
#include "lib/log.h"
#include "lib/module.h"
#include "lib/module_parameter.h"
#include "lib/socket.h"

/**
 * @class DiscompClient
 * @brief Discomp client class
 * 
 * In this class implemented functions for connect discomp client to server, 
 * authorization, functions for prepare module, start and stop it, and other.
 * When client start called Init method for get info about all available 
 * modules. And afrter connect to server client sent info about available 
 * modules. Server compare this list with own and return back list which 
 * modules not used yet (or server don't know about it).
 *
 * \sa DiscompModule
 */      

/***********************************************************************/
/*-----------------  Constructor & Destructor  ------------------------*/
/***********************************************************************/
/**
 * Constructor
 */
DiscompClient::DiscompClient()
{
	current_module_exec = "";
	connect_after_start = true;
	log_file            = "";
	sysinfo             = new DiscompSysInfo();
	
	logMessage( LOG_DEBUG, "Constructed DiscompClient" );
};

/**
 * Destructor
 */
DiscompClient::~DiscompClient()
{
	//stop all modules
	logMessage( LOG_DEBUG, "Destructing modules" );
	for ( int i=0; i < (int)modules.size(); i++ ) {
		DiscompModule *module = modules[i];
		if ( this->current_module_exec == modules[i]->name ) {
			module->Stop();
		};
		delete module;
	};
	delete discomp_socket;
	delete output_synctoserver_timer;
};

/**
 * Initializtion
 */
void DiscompClient::InitModules()
{
	if ( modules.size() > 0 ) {
		logMessage( LOG_COMMON, "Destructing modules" );
		for ( int i=0; i < (int)modules.size(); i++ ) {
			DiscompModule *module = modules[i];
			if ( this->current_module_exec == modules[i]->name ) {
				module->Stop();
			};
			delete module;
		};
		modules.clear();
	};

	logMessage( LOG_COMMON, "Client modules initialization" );



	//add system.single module
	DiscompModule *newModule = new DiscompModule();
	newModule->log_file = this->log_file;
	newModule->name     = "system.single";
	newModule->abs_name = "system.single";
	connect ( newModule, SIGNAL ( moduleExecutionFinished_signal          (const QString &, int) ),
	          this,      SLOT   ( sendModuleOutputParametersToServer_slot (const QString &, int) ) );
	modules.push_back ( newModule );
	
	//load all modules
	this->loadModulesFromDir ( Config::variables["ModulesDirectory"] ) ;


	if ( modules.size() < 1 ) {
		logMessage( LOG_ERROR, "Modules list is empty");
	};
};


void DiscompClient::loadModulesFromDir( const QString &dir_path )
{
	logMessage( LOG_COMMON, "Load modules from directory: " + dir_path );
	QDir dir ( dir_path );

	QStringList::Iterator module_name;
	QStringList dirs = dir.entryList(QDir::Dirs);
	
	module_name = dirs.begin();
	while ( module_name != dirs.end() ) {
		if ( *module_name != "." && *module_name != ".." && *module_name != "CVS" ) {
			if ( QString(*module_name).contains ( "." ) ) {
				this->loadModuleByName ( *module_name );
			} else { //it's a dir with modules for some package. (TODO. it's still used??)
				this->loadModulesFromDir ( dir_path + QDir::separator() + *module_name );
			};
		};
		module_name++;
	};

}

void DiscompClient::loadModuleByName( const QString &module_name )
{
	DiscompModule *tmpModule = new DiscompModule();
	//get module operations list
	QStringList module_operations = tmpModule->getModuleOperations ( module_name );
	for ( int oper_num=0; oper_num<module_operations.size(); oper_num++ ) {
		//initialize all module operations
		DiscompModule *newModule = new DiscompModule();
		newModule->log_file = this->log_file;
		if ( newModule->LoadModule( module_name, module_operations.at(oper_num) ) ) {

			connect ( newModule, SIGNAL ( moduleExecutionFinished_signal          (const QString &, int) ),
			          this,      SLOT   ( sendModuleOutputParametersToServer_slot (const QString &, int) ) );
				
			modules.push_back ( newModule );
		} else {
			delete newModule;
		};
	};
	delete tmpModule;
};

/**
 * Start thread
 */
void DiscompClient::run()
{
	connection_status = 0;
	discomp_socket   = new DiscompSocket(0);

	discomp_socket->log_file = this->log_file;

	connect(discomp_socket, SIGNAL ( connectedToServer_signal() ),
		this,           SLOT   ( connectedToServer_slot  () ) );
	connect(discomp_socket, SIGNAL ( connectionClosed_signal () ),
		this,           SLOT   ( connectionClosed_slot   () ) );
	connect(discomp_socket, SIGNAL ( socketError_signal                  ( int ) ),
		this,           SLOT   ( socketError_slot                    ( int ) ) );
	connect(discomp_socket, SIGNAL ( processRequestFromRemoteHost_signal (const QString &) ),
		this,           SLOT   ( processRequestFromServer_slot       (const QString &) ));
	connect(discomp_socket, SIGNAL ( processAnswerFromRemoteHost_signal  (const QString &) ),
		this,           SLOT   ( processAnswerFromServer_slot        (const QString &) ));
	connect(discomp_socket, SIGNAL ( fileSendingDone_signal              (const QString &, int, const QString &) ),
	        this,           SLOT   ( outputParameterSendFileDone_slot    (const QString &, int, const QString &) ) );
	
	connect(this,           SIGNAL ( connectToServer_signal              (const QString &, const QString &) ),
	        discomp_socket, SLOT   ( connectToServer_slot                (const QString &, const QString &) ) );
	connect(this,           SIGNAL ( doDisconnect_signal                 () ),
	        discomp_socket, SLOT   ( doDisconnect_slot                   () ) );
	connect(this,           SIGNAL ( sendDataToRemoteHost_signal         (const QString &) ),
	        discomp_socket, SLOT   ( sendDataToRemoteHost_slot           (const QString &) ) );
	connect(this,           SIGNAL ( sendFile_signal                     (const QString &) ),
	        discomp_socket, SLOT   ( sendFile_slot                       (const QString &) ) );
	connect(this,           SIGNAL ( sendFile_signal                     (const QString &,const QString &) ),
	        discomp_socket, SLOT   ( sendFile_slot                       (const QString &,const QString &) ) );
	connect(this,           SIGNAL ( sendFile_signal                     (const QString &,const QString &,const QString &) ),
	        discomp_socket, SLOT   ( sendFile_slot                       (const QString &,const QString &,const QString &) ) );


	output_synctoserver_timer = new QTimer();
	connect(output_synctoserver_timer, SIGNAL(timeout()), this, SLOT(syncModuleOutputParametersToServer_slot()));
	
	discomp_socket->setIncomingFilesDir ( Config::variables["TempDirectory"] );


	logMessage (LOG_DEBUG, "Staring client thread");

	//get system information
	sysinfo->Init();

	if ( connect_after_start ) {
		doConnect();
	};

	exec();
};



/***********************************************************************/
/*---------------------------- Protocol slots -------------------------*/
/***********************************************************************/

/** 
 * This slot called when connection with server is closed or lost
 */
void DiscompClient::connectionClosed_slot()
{
	connection_status = 0;
	emit connectionStatusChanged(0);

	logMessage ( LOG_COMMON, "Diconnected from server. Will try to reconnect again after: " + Config::variables["ReconnectInterval"] + "sec" );

	QTimer::singleShot(  Config::variables["ReconnectInterval"].toInt()*1000, this, SLOT(connectToServer_slot()));
}
/**
 * This slot called when socket successfully connected to server
 */
void DiscompClient::connectedToServer_slot()
{
	connection_status = 1;
	emit connectionStatusChanged(1);

	sendAuthorizationRequest();
}

/**
 * Some socket error occured. Host not found, Connection closed or refused.
 * Print log message and exit
 */
void DiscompClient::socketError_slot ( int code ) {
	logMessage ( LOG_COMMON, "Diconnected from server. Will try to reconnect again after: " + Config::variables["ReconnectInterval"] + "sec" );
	QTimer::singleShot(  Config::variables["ReconnectInterval"].toInt()*1000, this, SLOT(connectToServer_slot()));

	code = 0; //for hide gcc unused warning
};



/**
 * This slot called by timer for auomatically reconnect to server
 */
void DiscompClient::connectToServer_slot()
{
	doConnect();
}

void DiscompClient::sendDataToRemoteHost(const QString& xml)
{
	emit sendDataToRemoteHost_signal ( xml );
}

/**
 * Protocol switch where we detect server request
 */
void DiscompClient::processRequestFromServer_slot ( const QString &xml_data_str )
{
	QString errorStr;
	int errorLine;
	int errorColumn;

	QDomDocument doc;
	if ( !doc.setContent( xml_data_str, true, &errorStr, &errorLine, &errorColumn ) ) {
		logMessage (LOG_ERROR, QString("XML processing error ( line %1, column %2: %3 )").arg(errorLine).arg(errorColumn).arg(errorStr) );
		return;
	};

	QDomElement rootNode = doc.documentElement();

	QDomNode requestNode = rootNode.firstChild();
	while ( !requestNode.isNull() && requestNode.nodeType() != QDomNode::ElementNode ) {
		requestNode = requestNode.nextSibling();
	}


	int requestId = requestNode.toElement().attribute("id").toInt();

	switch ( requestId ) {
		case 101 : //ping signal
			sendDataToRemoteHost ( QString("<answer id='101' status='1' ping_time='%1'/>").arg(requestNode.toElement().attribute("ping_time")) );
			break;
		case 106: //update sys info
			updateSysInfo ();
			break;

		case 112 : //send modules list after timeout
			logMessage ( LOG_COMMON, "Wait while general client will update module. After that module list will be send again");
			//try reconnect after 10 seconds
			QTimer::singleShot( 10*1000, this, SLOT(sendModulesListToServer_slot()));
			break;
		case 120 : //server want to send a parameters list for module
			prepareForStartModule( requestNode );
			break;
			
		case 121 : //received input parameters. if all is ok - start module
			receivedInputParameters( requestNode );
			break;

		//case 122 : //start module execution
		//	//receivedRequestOnStartModule( requestNode );
		//	break;

		case 124 : //stop module execution
			receivedRequestOnStopModule( requestNode );
			break;

		case 200 : //monitoring


		default :
			logMessage( LOG_ERROR, QString("Unknown request from server (id:%1)").arg(requestId) );
			break;
	};
};


/**
 * Protocol switch where we detect server answer 
 * on the our request
 */
void DiscompClient::processAnswerFromServer_slot( const QString &xml_data_str )
{
	QString errorStr;
	int errorLine;
	int errorColumn;

	QDomDocument doc;
	if ( !doc.setContent( xml_data_str, true, &errorStr, &errorLine, &errorColumn ) ) {
		logMessage (LOG_ERROR, QString("XML processing error ( line %1, column %2: %3 )").arg(errorLine).arg(errorColumn).arg(errorStr) );
		return;
	};

	QDomElement rootNode = doc.documentElement();

	QDomNode answerNode = rootNode.firstChild();
	while ( !answerNode.isNull() && answerNode.nodeType() != QDomNode::ElementNode ) {
		answerNode = answerNode.nextSibling();
	}

	int answerTypeId = answerNode.toElement().attribute("id").toInt();

	switch ( answerTypeId ) {
		case 100: 
			checkAuthorization ( answerNode );
			break;

		case 110:
			checkSendModulesListToServer( answerNode );
			break;

		case 111:
			updateModuleFromArchive ( answerNode );
			break;

//		case 666: //debug
//			sendDataToRemoteHost( "<request id='666'/>" );

		default :
			logMessage (LOG_ERROR, "Unknown answer id. Skip" );
			break;
				
	};
};







/***********************************************************************/
/*----------------- Protocol processing functions  --------------------*/
/***********************************************************************/


/**
 * Authorization.
 * After connecting to server we should send authorization data.
 * (now it's simple. for authorization we should send client name only)
 */
void DiscompClient::sendAuthorizationRequest() 
{
	logMessage ( LOG_COMMON, "Sending authorization request" ); 

	//debug
	/*
	disconnect(discomp_socket, SIGNAL ( fileSendingDone_signal                   (const QString &, int, const QString &) ),
	           this,           SLOT   ( outputParameterSendFileDone_slot         (const QString &, int, const QString &) ) );
	disconnect(discomp_socket, SIGNAL ( fileSendingDone_signal                   (const QString &, int, const QString &) ),
	           this,           SLOT   ( sendFailedModuleTempDirToServerDone_slot (const QString &, int, const QString &) ) );
	connect(   discomp_socket, SIGNAL ( fileSendingDone_signal                   (const QString &, int, const QString &) ),
	           this,           SLOT   ( sendAuth_slot                            (const QString &, int, const QString &) ) );
	emit sendFile_signal ( "/tmp/discomp/1", "/tmp/discomp", "2" );
	return;
	*/

	//send client name to server
	QString xml;
	xml += "<request id='100' name='authorization'>\n";
	xml += "	<client name='" + clientName + "' cpu_num='" + QString("%1").arg(this->cpu_num) + "' version='"+QString("%1").arg(VERSION)+"'/>\n";
	xml += "	" + sysinfo->getXML();
	xml += "</request>";
	
	sendDataToRemoteHost ( xml ); 
};




/**
 * Check authorization authorization and if ok - send modules list to server
 */
void DiscompClient::checkAuthorization( const QDomNode & xml_node )
{
	if ( xml_node.toElement().attribute("status").toInt() != 1 ) {
		logMessage(LOG_ERROR, "Authorization failed. Maybe you forgot specify client name?");

		emit doDisconnect_signal();
	};

	logMessage( LOG_COMMON, "Authorization success");

	connection_status = 2;
	emit connectionStatusChanged(2);

	sendModulesListToServer();
};


/**
 * Send modules list to server slot
 * This slot called if some modules need to be upgraded by general client.
 */
void DiscompClient::sendModulesListToServer_slot()
{
	sendModulesListToServer();
};
/**
 * Send modules list to server 
 */
void DiscompClient::sendModulesListToServer()
{
	logMessage(LOG_COMMON, "Init and send list of available modules to server");

	InitModules();

	QString xml;	
	xml += "<request id='110' name='modules_list'>\n";
	for ( int i=0; i < (int)modules.size(); i++ ) {
		xml += "	<module name='" + QString("%1").arg(modules[i]->name) + "' name='" + modules[i]->name + "' md5='" + modules[i]->getModuleMD5() + "' allow_update='" + Config::variables["AllowUpdateModules"] + "'/>\n";
	};
	xml += "</request>";

	sendDataToRemoteHost ( xml ); 
};


/**
 * After receiving modules list server trying to find 
 * these modules in his modules dir. if it's not found 
 * server return zero status and module class should be
 * destroyed at the client side
 * */
void DiscompClient::checkSendModulesListToServer ( const QDomNode & root_node )
{
	QDomNode data_node = root_node.firstChild();
	while ( !data_node.isNull() ) {
		if ( data_node.toElement().tagName() == "module" ) {
			
			//get module name
			int     moduleStatus = data_node.toElement().attribute("status").toInt();
			QString moduleName   = data_node.toElement().attribute("name");

			if ( moduleStatus != 1 ) {
				//remove this module from our list of "known modules"
				for ( int i=0; i < (int)modules.size(); i++ ) {
					if ( modules[i]->name == moduleName ) {
						logMessage ( LOG_ERROR, QString("Server retrun null status for module: %1 (unknown)").arg(moduleName) );

						DiscompModule *tmpPtr = modules[i];
						
						modules.erase(&modules[i]);
						delete tmpPtr;
					};
				};
			};
		};
		data_node = data_node.nextSibling();
	};

	logMessage( LOG_COMMON,  QString("Client successfully connected") );
};

/**
 * Update system information state
 * and send new info to the server
 */
void DiscompClient::updateSysInfo() 
{
	logMessage ( LOG_COMMON, "Server ask to update current system information state" ); 

	sysinfo->Update();
	//send client name to server
	QString xml;
	xml += "<answer id='106' name='system information' status='1'>\n";
	xml += "	" + sysinfo->getXML();
	xml += "</answer>";
	
	sendDataToRemoteHost ( xml ); 
}




/***********************************************************************/
/*-------------------------- Module execution  ------------------------*/
/***********************************************************************/
/**
 * Prepare module for start. Copy module to the temp directory
 * Set enviroment for file transfer, execution, etc.
 */
void DiscompClient::prepareForStartModule( const QDomNode & rootNode )
{
	QDomNode moduleNode = XML::getChildNodeByTag( "module", rootNode );
	QString moduleName = moduleNode.toElement().attribute("name");
	this->current_module_exec = moduleName;

	discomp_socket->setIncomingFilesDir ( Config::variables["TempDirectory"] );

	int module_num = getModuleNumByName( moduleName );

	modules[module_num]->stoping_flag = false;

	modules[module_num]->logMessage( LOG_COMMON, QString("Received request for prepare module execution: %1").arg(moduleName));

	//TODO:first check module md5
	QString server_module_md5 = moduleNode.toElement().attribute("md5");
	if ( server_module_md5 != modules[module_num]->md5 ) {
		QString error_msg = "MD5 sums for module: " + modules[module_num]->name + " are different on the client (" + modules[module_num]->md5 + ") and server ("+server_module_md5+") sides.";

		modules[module_num]->logMessage ( LOG_ERROR, error_msg );

		sendDataToRemoteHost ( QString("<answer id='120' status='0' substatus='md5_error' ping_time='"+rootNode.toElement().attribute("ping_time")+"'><error>"+error_msg+"</error><module name='%1'/></answer>").arg(moduleName) );

		return;
	};

	//create module temp enviroment
	if ( modules[module_num]->createModuleTempEnvroment() ) {
		discomp_socket->setIncomingFilesDir ( modules[module_num]->getModuleTempDir() );
	} else {
		modules[module_num]->logMessage( LOG_ERROR, QString("Couldn't create temporary module directory. Module cannot be started" ) );
		sendDataToRemoteHost ( QString("<answer id='120' status='0' ping_time='"+rootNode.toElement().attribute("ping_time")+"'><error>"+modules[module_num]->error_msg+"</error><module name='%1'/></answer>").arg(moduleName) );
		return;
	};

	sendDataToRemoteHost ( QString("<answer id='120' status='1' ping_time='"+rootNode.toElement().attribute("ping_time")+"'><module name='%1'/></answer>").arg(moduleName) ); 
};


/**
 * Receive parameters specification for module.
 * Files and filelist should be already in the module dir.
 * Server send it with file transfer protocol. Here we check 
 * parameters specification and set it for our module parameters
 * If all parameters defined - then start module.
 */
void DiscompClient::receivedInputParameters ( const QDomNode & rootNode )
{
	QDomNode moduleNode = XML::getChildNodeByTag( "module", rootNode );
	QString moduleName = moduleNode.toElement().attribute("name");
	
	int module_num = getModuleNumByName( moduleName );
	if ( moduleName == "system.single" ) {
		modules[module_num]->logMessage( LOG_COMMON, "Reload system.single module specification");
		modules[module_num]->module_dir = modules[module_num]->getModuleTempDir();
		modules[module_num]->logMessage( LOG_DEBUG, "Module temp directory is: " + modules[module_num]->module_dir );
		modules[module_num]->LoadModule(moduleName);
		modules[module_num]->temp_module_dir = modules[module_num]->module_dir;
		modules[module_num]->setModuleParametersDir(modules[module_num]->module_dir);
	};

	modules[module_num]->process_parameters_dir = moduleNode.toElement().attribute("process_parameters_dir");
	modules[module_num]->logMessage(LOG_COMMON,"Received input parameters from server");

	QDomNode parametersNode = XML::getChildNodeByTag( "parameters", moduleNode );
	QDomNode paramNode      = parametersNode.firstChild();
	while ( !paramNode.isNull() ) {
		if ( paramNode.toElement().tagName() == "param" ) {
			QString paramName = paramNode.toElement().attribute("name");

			int module_num = getModuleNumByName( moduleName );
			DiscompModuleParameter *param  = modules[module_num]->getParameterByName(paramName);
			if ( !param ) {
				modules[module_num]->logMessage( LOG_ERROR, QString("Parameter '%2' is not exists.").arg(paramName) );

				clearModuleTempEnviroment( moduleName );

				sendDataToRemoteHost ( 
					QString("<answer id='121' status='0'><module name='%2'/><error>Unknown parameter '%1' in the module specification</error></answer>").arg(paramName).arg(moduleName) ); 
				return;
			};
			param->remote_filename = paramNode.toElement().attribute("remote_filename");
		};
		paramNode = paramNode.nextSibling();
	};
	startModule ( moduleName );
};


/**
 * Start module
 **/
void DiscompClient::startModule ( const QString &moduleName ) 
{

	int module_num = getModuleNumByName( moduleName );

	if ( !modules[module_num]->isReady() ) {
		modules[module_num]->logMessage( LOG_ERROR, QString("Module is not ready for start: %1").arg(moduleName) );
		
		clearModuleTempEnviroment( moduleName );

		sendDataToRemoteHost ( QString("<answer id='121' status='0'><module name='%1'/><error>Module is not ready</error></answer>").arg(moduleName) ); 
		return;
	};
	
	
	modules[module_num]->logMessage( LOG_COMMON, QString("Module ready for start: %1").arg(moduleName) );
	
	//staring module
	if ( modules[module_num]->Start() ) {
		sendDataToRemoteHost ( QString("<answer id='122' status='1'><module name='%1'/></answer>").arg(moduleName) ); 

		// timer for check output parameters changes (for sync it to the server)
		if ( modules[module_num]->output_synctoserver_interval ) {
			syncModuleOutputParametersToServer_slot();
			output_synctoserver_timer->start(modules[module_num]->output_synctoserver_interval*1000);
		};
	} else {
		if ( modules[module_num]->error_msg != "-1" ) { // if it's -1 then error will be send by startProcessExited_signal 
			sendFailedModuleTempDirToServer ( moduleName, QString("<answer id='122' status='0'><module name='%1' file_archive='###FILE_ARCHIVE###'/><error>%2</error></answer>").arg(moduleName).arg(modules[module_num]->error_msg) );
		};
	};

};


/**
 * Received request for stop module
 */
void DiscompClient::receivedRequestOnStopModule( const QDomNode & rootNode )
{
	QDomNode moduleNode = XML::getChildNodeByTag( "module", rootNode );
	QString moduleName = moduleNode.toElement().attribute("name");
	int module_num = getModuleNumByName( moduleName );

	modules[module_num]->logMessage( LOG_COMMON, "Received request for stop module" );
	modules[module_num]->stoping_flag = true;

	output_synctoserver_timer->stop();

	if ( modules[module_num]->Stop() ) {
		clearModuleTempEnviroment( moduleName );
		sendDataToRemoteHost ( QString("<answer id='124' status='1'><module name='%1' time='%2' time_ms='%3'/></answer>").arg(moduleName).arg(modules[module_num]->execution_real_time).arg(modules[module_num]->execution_real_time_ms) ); 
	} else {
		clearModuleTempEnviroment( moduleName );
		sendDataToRemoteHost ( QString("<answer id='124' status='0'><module name='%1' time='%3' time_ms='%4'/><error>%2</error></answer>").arg(moduleName).arg(modules[module_num]->error_msg).arg(modules[module_num]->execution_real_time).arg(modules[module_num]->execution_real_time_ms) ); 
	};

	discomp_socket->stopFileSending();
	discomp_socket->clearFilesQueue();
};


/**
 * Send module temporarary output parameters to server when module is executing 
 * i.e. syncing output parameters data which we already have to the server output parameter dir
 */
void DiscompClient::syncModuleOutputParametersToServer_slot (  ) 
{

	//don't check temporary files sending
	disconnect(discomp_socket, SIGNAL ( fileSendingDone_signal                   (const QString &, int, const QString &) ),
	           this,           SLOT   ( outputParameterSendFileDone_slot         (const QString &, int, const QString &) ) );

	int module_num   = getModuleNumByName( this->current_module_exec );

	//check that module is executing now
	if ( !modules[module_num]->started_flag ) { return; };


	if ( discomp_socket->getFileSendingQueueSize() > 0 ) {
		modules[module_num]->logMessage( LOG_COMMON, "Seems that some files already sending to server. Skip");
		output_synctoserver_timer->start(modules[module_num]->output_synctoserver_interval*1000);
		return;
	};

	for ( int param_num=0; param_num < (int)modules[module_num]->outputParameters.size(); param_num++ ) {
		DiscompModuleParameter* param = modules[module_num]->outputParameters[param_num];

		
		if ( param->type == "file" ) {
			//check exists
			param->dir = this->modules[module_num]->getModuleTempDir();
			QString filepath  = this->modules[module_num]->getModuleTempDir() + param->filename;

			QString tmp_md5 = File::getMD5(filepath);
			if ( param->md5 != tmp_md5  ) {
				param->md5 = tmp_md5;

				modules[module_num]->logMessage(LOG_COMMON, "Sending (syncing) output parameter of executed module to server: " + filepath);

				if ( param->remote_filename != "" ) {
					emit sendFile_signal ( filepath, modules[module_num]->process_parameters_dir, param->remote_filename );
				} else {
					emit sendFile_signal ( filepath, modules[module_num]->process_parameters_dir );
				};
			};
			
		} else if ( param->type == "filelist" ) {
			param->dir = this->modules[module_num]->getModuleTempDir();
			if ( param->listsize() != 0  ) { 
				//check md5;
				QString tmp_md5;
				for ( int list_num=0; list_num < (int)param->listsize(); list_num++ ) {
					QString filepath = this->modules[module_num]->getModuleTempDir() + QString(param->filepattern).arg(list_num);
					tmp_md5 += File::getMD5(filepath);
				};
				tmp_md5 = File::getStrMD5(tmp_md5);

				if ( param->md5 != tmp_md5 ) {
					param->md5 = tmp_md5;

					modules[module_num]->logMessage(LOG_COMMON, QString("Sending (syncing) output parameter of executed module to server %1 (listsize: %2)").arg(param->filepattern).arg(param->listsize()) );
					for ( int list_num=0; list_num < (int)param->listsize(); list_num++ ) {
						QString filepath = this->modules[module_num]->getModuleTempDir() + QString(param->filepattern).arg(list_num);
						emit sendFile_signal( filepath, modules[module_num]->process_parameters_dir );
					};
				};
			};
		};
	};


	output_synctoserver_timer->start(modules[module_num]->output_synctoserver_interval*1000);
};



/**
 * Send module output parameters to server
 */
void DiscompClient::sendModuleOutputParametersToServer_slot ( const QString &module_name, int status ) 
{
	//disconnected at syncModuleOutputParametersToServer_slot
	disconnect(discomp_socket, SIGNAL ( fileSendingDone_signal                   (const QString &, int, const QString &) ),
	           this,           SLOT   ( outputParameterSendFileDone_slot         (const QString &, int, const QString &) ) );
	connect(   discomp_socket, SIGNAL ( fileSendingDone_signal                   (const QString &, int, const QString &) ),
	           this,           SLOT   ( outputParameterSendFileDone_slot         (const QString &, int, const QString &) ) );
	output_synctoserver_timer->stop();

	int module_num = getModuleNumByName( module_name );

	if ( modules[module_num]->stoping_flag ) { return; };

	if ( status == 0 ) {
		logMessage ( LOG_ERROR, "Module execution failed" );
		QString request;
		request += QString("<request id='123' status='%1'>\n").arg(status);
		request += "	<error>Module execution failed. "+modules[module_num]->error_msg+"</error>\n";
		request += QString ("<module name='%1' file_archive='###FILE_ARCHIVE###' time='%2' time_ms='%3' usage_info='%4'/>\n").arg(module_name).arg(modules[module_num]->execution_real_time).arg(modules[module_num]->execution_real_time_ms).arg(modules[module_num]->node_resources_usage_info);
		request += "</request>";
		
		sendFailedModuleTempDirToServer ( module_name, request );

		return;

	} else if ( status == 2 ) { //max time reached
		logMessage ( LOG_COMMON, "Module execution failed: " + modules[module_num]->error_msg );
		QString request;
		request += QString("<request id='123' status='%1'>\n").arg(status);
		request += "	<error>Module execution failed. "+modules[module_num]->error_msg+"</error>\n";
		request += QString ("<module name='%1' time='%2' time_ms='%3' usage_info='%4'/>\n").arg(module_name).arg(modules[module_num]->execution_real_time).arg(modules[module_num]->execution_real_time_ms).arg(modules[module_num]->node_resources_usage_info);
		request += "</request>";
		
		sendDataToRemoteHost ( request );
		return;
	};

	modules[module_num]->logMessage( LOG_COMMON, "Send module output parameters to server");

	this->files_for_send.clear();

	for ( int param_num=0; param_num < (int)modules[module_num]->outputParameters.size(); param_num++ ) {
		DiscompModuleParameter* param = modules[module_num]->outputParameters[param_num];

		if ( param->type == "file" ) {
			//check exists
			param->dir = this->modules[module_num]->getModuleTempDir();

			if ( !param->is_defined() ) {
				outputParameterIsNotExists ( module_name, param->name );
				return;
			};
			
			QString filepath = this->modules[module_num]->getModuleTempDir() + param->filename;

			this->files_for_send << filepath;

			modules[module_num]->logMessage(LOG_COMMON, "Sending file to server: " + filepath);

			if ( param->remote_filename != "" ) {
				emit sendFile_signal ( filepath, modules[module_num]->process_parameters_dir, param->remote_filename );
			} else {
				emit sendFile_signal ( filepath, modules[module_num]->process_parameters_dir );
			};
			
		} else if ( param->type == "filelist" ) {
			param->dir = this->modules[module_num]->getModuleTempDir();
			if ( param->listsize() == 0  ) { //list size is not defined
				outputParameterIsNotExists ( module_name, param->name );
				this->files_for_send.clear();
				return;
			};
			
			modules[module_num]->logMessage(LOG_COMMON, QString("Sending filelist to server %1 (listsize: %2)").arg(param->filepattern).arg(param->listsize()) );

			// here we going from file fith index 0 to latest.
			// in this case in the for loop hard edgeds
			for ( int list_num=0; list_num <= (int)param->listsize(); list_num++ ) {
				QString filepath = this->modules[module_num]->getModuleTempDir() + QString(param->filepattern).arg(list_num);
				if ( QFile::exists (filepath) ) {
					emit sendFile_signal( filepath, modules[module_num]->process_parameters_dir );
					this->files_for_send << filepath;
				};
			};
		};
	};

	if ( this->files_for_send.size() == 0 ) {
		allOutputParametersSent();
	};
};

/**
 * If output parameter isn't exit ( output file not exists )
 * then send to server failed status about module execution
 */
void DiscompClient::outputParameterIsNotExists ( const QString &module_name, const QString & param_name )
{
	this->files_for_send.clear();

	int module_num = getModuleNumByName( module_name );

	if ( modules[module_num]->stoping_flag ) { return; };

	QString error_msg = QString ("Module execution finished but output parameter '%1' is not exists").arg(param_name);

	modules[module_num]->logMessage ( LOG_ERROR, error_msg );
	
	QString request;
	request += "<request id='123' status='0'>\n";
	request += "	<error>" + error_msg +"</error>\n";
	request += "	<module name='" + module_name + "' time='"+QString("%1").arg(modules[module_num]->execution_real_time)+"' time_ms='"+QString("%1").arg(modules[module_num]->execution_real_time_ms)+"' usage_info='" + modules[module_num]->node_resources_usage_info + "' file_archive='###FILE_ARCHIVE###'/>\n";
	request += "</request>";
		
	sendFailedModuleTempDirToServer ( module_name, request );
}

/**
 * Function for check sending file to server
 */
void DiscompClient::outputParameterSendFileDone_slot ( const QString &filename, int status, const QString &reason ) 
{
	int module_num = getModuleNumByName(this->current_module_exec);

	if ( modules[module_num]->stoping_flag ) { return; };

	// search if it is our file
	for ( int i=0; i < this->files_for_send.size(); i++ ) {
		if ( filename == this->files_for_send.at(i) ) {
			if ( !status ) {
				modules[module_num]->logMessage(LOG_ERROR, QString("Couldn't send file to remote host: %1. Reason: %2").arg(filename).arg(reason) );
				this->files_for_send.clear();

				QString request;
				request += "<request id='123' status='0'>\n";
				request += "	<error>Module execution finished not correctly. Could't send file to server: "+filename+". Reason: "+reason+"</error>\n";
				request += "	<module name='" + this->current_module_exec + "' time='"+QString("%1").arg(modules[module_num]->execution_real_time)+"' time_ms='"+QString("%1").arg(modules[module_num]->execution_real_time_ms)+"' usage_info='" + modules[module_num]->node_resources_usage_info + "' file_archive='###FILE_ARCHIVE###'/>\n";
				request += "</request>";
		
				sendFailedModuleTempDirToServer (  this->current_module_exec, request );

				return;
			};
			
			modules[module_num]->logMessage(LOG_COMMON, QString("File successfully sent to server: %1").arg(filename) );
			
			files_for_send.removeAt(i);
			modules[module_num]->logMessage(LOG_COMMON, QString("File successfully sent to server: size  %1").arg(files_for_send.size()) );
			if ( files_for_send.size() == 0  ) { 
				this->allOutputParametersSent();
			};
			
			break;
		};
	};
};


/**
 * All parameters files sucessfully sent to server. 
 * Now send specification about ouput parameters
 */
void DiscompClient::allOutputParametersSent() {
	
	QString request;

	int module_num = getModuleNumByName( this->current_module_exec );

	if ( modules[module_num]->stoping_flag ) { return; };
	
	request += "<request id='123' status='1'>\n";
	request += "	<module name='" + this->current_module_exec + "' time='"+QString("%1").arg(modules[module_num]->execution_real_time)+"' time_ms='"+QString("%1").arg(modules[module_num]->execution_real_time_ms)+"' usage_info='" + modules[module_num]->node_resources_usage_info + "'>\n";
	request += "		<parameters>\n";
	
	
	for ( int param_num=0; param_num < (int)modules[module_num]->outputParameters.size(); param_num++ ) {
		DiscompModuleParameter* param = modules[module_num]->outputParameters[param_num];

		request += "\t\t\t<param name='" + QString("%1").arg(param->name) + "'/>\n";
	};

	request += "		</parameters>\n";
	request += "	</module>\n";
	request += "</request>";

	//clear module temp dir
	clearModuleTempEnviroment ( this->current_module_exec );

	modules[module_num]->logMessage ( LOG_COMMON, "Module successfully finished. All output parameters was sent to server");

	if ( modules[module_num]->stoping_flag ) { return; };
	sendDataToRemoteHost ( request );
};

/**
 * Clear module temp enviroment (remove dir. set socket incoming dir back)
 * @param module_id - module identificator
 */
int DiscompClient::clearModuleTempEnviroment ( const QString &module_name ) 
{
	discomp_socket->setIncomingFilesDir ( Config::variables["TempDirectory"] );
	output_synctoserver_timer->stop();

	int module_num = getModuleNumByName( module_name );
	if ( !modules[module_num]->clearModuleTempEnviroment() ) {
		return false;
	};

	return true;
};

/**
 * Received info for update module from archive
 */
void DiscompClient::updateModuleFromArchive ( const QDomNode & rootNode )
{
	QDomNode moduleNode = XML::getChildNodeByTag( "info", rootNode );
	QString module_name = moduleNode.toElement().attribute("module_name");
	QString file_name   = moduleNode.toElement().attribute("file_name");
	QString archive_path = Config::variables["TempDirectory"] + QDir::separator() + file_name;
	
	logMessage ( LOG_COMMON, "Received request for update module: " + module_name);

	//TODO! can be incorrect if operations used
	QString module_dir;
	module_dir =  QDir::convertSeparators ( Config::variables["ModulesDirectory"] + "/" + module_name + "/" );

	File::recursiveRemoveDirectoryContent ( module_dir );
	if ( !File::extractArchive( archive_path, module_dir ) ) {
		logMessage ( LOG_ERROR, "Couldn't extract archive file" );
		return;
	};
	

	//resend modules list to server
	sendModulesListToServer();
};


void DiscompClient::sendFailedModuleTempDirToServer ( const QString &module_name, const QString &protocol_msg ) 
{
	this->tmp_protocol_msg = protocol_msg;

	//get temp_module_dir
	int module_num = getModuleNumByName( module_name );

	if ( modules[module_num]->stoping_flag ) { return; };

	QString temp_module_dir = modules[module_num]->getModuleTempDir();

	if ( temp_module_dir == "" ) {
		logMessage ( LOG_DEBUG, "Send failed module to server failed. Because module output dir isn't exists" );
		return;
	};

	//creating temp module dir archive
	QString archive_file = File::createTempFile( QString("archive_%1_%2").arg(modules[module_num]->abs_name).arg( File::getStrMD5(this->clientName) ) );
	if ( !File::createArchive( temp_module_dir, archive_file ) ) {
		logMessage ( LOG_ERROR, "Couldn't create archive file: " + archive_file );
		clearModuleTempEnviroment( module_name );
		sendDataToRemoteHost ( protocol_msg ); 
		return;
	};

	//reconnect file sending done signal
	disconnect(discomp_socket, SIGNAL ( fileSendingDone_signal                   (const QString &, int, const QString &) ),
	           this,           SLOT   ( outputParameterSendFileDone_slot         (const QString &, int, const QString &) ) );
	disconnect(discomp_socket, SIGNAL ( fileSendingDone_signal                   (const QString &, int, const QString &) ),
	           this,           SLOT   ( sendFailedModuleTempDirToServerDone_slot (const QString &, int, const QString &) ) );

	connect   (discomp_socket, SIGNAL ( fileSendingDone_signal                   (const QString &, int, const QString &) ),
	           this,           SLOT   ( sendFailedModuleTempDirToServerDone_slot (const QString &, int, const QString &) ) );

	//TODO why to the tmp?
	emit sendFile_signal ( archive_file, "tmp" );
	
	clearModuleTempEnviroment( module_name );
};

void DiscompClient::sendFailedModuleTempDirToServerDone_slot ( const QString &filename, int status, const QString &reason )
{
	logMessage ( LOG_DEBUG, "sendFailedModuleTempDirToServerDone_slot" );

	//connect back file send signal
	disconnect(discomp_socket, SIGNAL ( fileSendingDone_signal                   (const QString &, int, const QString &) ),
	           this,           SLOT   ( sendFailedModuleTempDirToServerDone_slot (const QString &, int, const QString &) ) );
	connect(   discomp_socket, SIGNAL ( fileSendingDone_signal                   (const QString &, int, const QString &) ),
	           this,           SLOT   ( outputParameterSendFileDone_slot         (const QString &, int, const QString &) ) );

	QFileInfo file_info ( filename );
	this->tmp_protocol_msg.replace( "###FILE_ARCHIVE###", file_info.fileName() );
	sendDataToRemoteHost( this->tmp_protocol_msg );

	QFile::remove ( filename );

	if ( !status ) {
		logMessage ( LOG_WARNING, "Couldn't send file to remote host. Reason: " + reason );
	};
};







/**************************************************************/
/*-------------------- Public class methods ------------------*/
/**************************************************************/
/**
 * Set server parameters
 * @param IP - server IP (v4)
 * @param port - server port
 */
void DiscompClient::setServer ( const QString &IP, const QString &port ) 
{
	serverIP   = IP;	
	serverPort = port;
}
/**
 * Set client name
 * @param name - client name
 */
void DiscompClient::setClientName ( const QString &name ) 
{
	clientName = name;
}
/**
 * Connect to the server. Before call this function should be set server parameters by
 * calling method setServer
 * \sa setServer
 */
void DiscompClient::doConnect( )
{
	emit connectToServer_signal ( serverIP, serverPort );

	connection_status = 1;
	emit connectionStatusChanged(1);
}

/**
 * Disconnect from server (drop socket)
 */
void DiscompClient::doDisconnect( )
{
	emit doDisconnect_signal();
	
	connection_status = 0;
	emit connectionStatusChanged(0);
}
/**
 * Get connection status (connected or disconnected
 */
int DiscompClient::getConnectionStatus()
{
	return connection_status;
}

/** 
 * Set CPU number for current thread
 * @param - cpu number (0, 1, ... )
 */
void DiscompClient::setCPUNum ( int number )
{
	this->cpu_num = number;
}

/**
 * Set time interval for checking connection to server.
 * And reconnect if possible.
 * @param interval - interval in seconds
 */
void DiscompClient::setReconnectInterval ( int interval )
{
	this->reconnect_interval = interval;
};

/**
 * Connect to the server after initialization
 * @param flag - true or false
 */
void DiscompClient::setConnectAfterStart ( bool flag )
{
	this->connect_after_start = flag;
};





/***********************************************************************/
/*-------------------------- Private functions ------------------------*/
/***********************************************************************/

/**
 * Private function for return module num in the this->modules array by module id
 * @param mod_id - module identificator
 * @return module position in the this->modules array
 */
int DiscompClient::getModuleNumByName( const QString &module_name ) 
{
	//first search module with this id in modules space
	for ( int module_num=0; module_num<(int)modules.size(); module_num++ ) 
	{
		if ( modules[module_num]->name == module_name ) {
			return module_num;
		};
	};
	return -1;
};

/**
 * Add log message
 * @param log_level - log level from log.h
 * @param message - log message
 */
void DiscompClient::logMessage ( int log_level, const QString &message ) 
{
	Log::addMessage ( this->log_file, log_level, "DiscompClient", message);
};

// vim: set fenc=utf-8 tabstop=8 :
