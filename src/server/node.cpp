// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#include "node.h"
#include "clips_api.h"


/***********************************************************************/
/*-----------------  Constructor & Destructor  ------------------------*/
/***********************************************************************/

/* 
 * Constructor 
 * */
DiscompNode::DiscompNode(int sockDesc)
{
	socketDescriptor = sockDesc;

	id          = -1;
	ip          = "";
	name        = "";
	log_file    = "";
	sysinfo_xml = "";
	addit_log_file = "";
	process_id  = -1;
	busy        = false;
	authorized  = false;
	uptime_start             = 0; 
	current_module_exec = "";
	previous_process_id      = -1;
	previous_module_name       = "";
	class_destructed         = false;
	stoping_module_execution = false;
	process_list_element_num = 0;
	cpu_num = 0;
	ping_msec = -1;
        
//	discomp_clips = new DiscompClips();
	m_rrd = new DiscompXMLRRD();
	m_rrd->Init();    
};

/* 
 * Destructor 
 * */
DiscompNode::~DiscompNode()
{
	logMessage (LOG_DEBUG,"Destroy TCPSocket");
	
	if ( class_destructed ) { //terminate can call it again
		return;
	};
	class_destructed = true;

	emit doDisconnect_signal();
	if ( nodeSocket->state() == QAbstractSocket::ConnectedState ) 
		nodeSocket->waitForDisconnected();

	//stop thread
	//this->quit();
	//this->wait(1000);

	delete nodeSocket;
	
	logMessage (LOG_DEBUG,"Destroy all modules classes for this node");
	for ( int mod_num=0; mod_num < (int)modules.size(); mod_num++ ) {
		delete modules[mod_num];
	};
	
	logMessage ( LOG_DEBUG, "Thread destroyed" );
};


/* 
 * Start function for thread 
 * */
void DiscompNode::run()
{
	logMessage ( LOG_DEBUG, "Start thread" );

	if ( this->id == -1 ) {
		logMessage (LOG_ERROR,"Node id isn't specifeied.");
		return;
	};

	logMessage(LOG_DEBUG,"Trying to create TCP socket");

	
	//create TCP Socket
	nodeSocket = new DiscompSocket(socketDescriptor);
	if ( !nodeSocket->ok ) {
		logMessage(LOG_ERROR,"TCP socket creation failed");
	}

	connect ( nodeSocket, SIGNAL ( connectionClosed_signal()                        ),
	          this,       SLOT   ( connectionClosed_slot()                          ) );
	connect ( nodeSocket, SIGNAL ( processRequestFromRemoteHost_signal (const QString &)    ),
	          this,       SLOT   ( processRequestFromNode_slot         (const QString &)    ) );
	connect ( nodeSocket, SIGNAL ( processAnswerFromRemoteHost_signal  (const QString &)    ),
	          this,       SLOT   ( processAnswerFromNode_slot          (const QString &)    ) );

	//for socket
//	connect ( this,       SIGNAL ( connectToServer_signal              (const QString &, const QString &) ),
//	          nodeSocket, SLOT   ( connectToServer_slot                (const QString &, const QString &) ) );
	connect ( this,       SIGNAL ( doDisconnect_signal                 () ),
	          nodeSocket, SLOT   ( doDisconnect_slot                   () ) );
	connect ( this,       SIGNAL ( sendDataToRemoteHost_signal         (const QString &) ),
	          nodeSocket, SLOT   ( sendDataToRemoteHost_slot           (const QString &) ) );

	connect ( this,       SIGNAL ( clearFilesQueue_signal              () ),
	          nodeSocket, SLOT   ( clearFilesQueue_slot                () ) );
	connect ( this,       SIGNAL ( sendFile_signal                     (const QString &) ),
	          nodeSocket, SLOT   ( sendFile_slot                       (const QString &) ) );
	connect ( this,       SIGNAL ( sendFile_signal                     (const QStringList &, const QStringList &) ),
	          nodeSocket, SLOT   ( sendFile_slot                       (const QStringList &, const QStringList &) ) );
	connect ( this,       SIGNAL ( sendFile_signal                     (const QString &,const QString &) ),
	          nodeSocket, SLOT   ( sendFile_slot                       (const QString &,const QString &) ) );
	connect ( this,       SIGNAL ( sendFile_signal                     (const QString &,const QString &,const QString &) ),
	          nodeSocket, SLOT   ( sendFile_slot                       (const QString &,const QString &,const QString &) ) );




	this->ip         = nodeSocket->peerAddress().toString(); 
	
	this->log_file       = QString("node_%1_%2.%3").arg(this->id).arg(this->ip).arg(LOG_FILE_EXT);
	nodeSocket->log_file = QString("node_%1_%2.%3").arg(this->id).arg(this->ip).arg(LOG_FILE_EXT);

	logMessage (LOG_COMMON,"node IP is " + this->ip );


	QDateTime timestamp (QDateTime::currentDateTime ());
	this->uptime_start = timestamp.toTime_t ();

	logMessage (LOG_DEBUG,"Thread initialized");
	
	exec();

	//wait data from socket
	/*
	while (nodeSocket->state() == QAbstractSocket::ConnectedState)
	{
		if ( class_destructed ) {
			logMessage(LOG_DEBUG, "Seems class is destroyed. Exit from thread loop");
			return;
		};

		if (nodeSocket->bytesAvailable() < 1)
		{
			if (!nodeSocket->waitForReadyRead(-1))
				break;
		}
	
		if ( nodeSocket->bytesAvailable() >= 1 ) {
			emit readDataFromRemoteHost_signal();
		}

	}
	*/
	/*
	logMessage (LOG_DEBUG,"Thread initialized1");
	nodeSocket->readDataFromRemoteHost();
	exec();
	logMessage (LOG_DEBUG,"Thread initialized2");
	*/
}







/***********************************************************************/
/*------------------------  TCP Socket slots  -------------------------*/
/***********************************************************************/

void DiscompNode::connectionClosed_slot()
{
	if ( class_destructed ) {
		return;
	};
	logMessage (LOG_COMMON,"Remote node has been disconnected");

	//this->quit(); // stop thread
	//terminate(); //stop thread
	//wait();
	//

	emit nodeDisconnected_signal( this->id );
};


/* 
 * Protocol switch where we detect request id and execute 
 * process function.
 * */
void DiscompNode::processRequestFromNode_slot( QString xml_data_str  )
{
	QString errorStr;
	int errorLine;
	int errorColumn;

	//module stoping stage
	if ( this->stoping_module_execution ) {//nothing to do. wait while be stoped
		return;
	}

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

	int requestTypeId = requestNode.toElement().attribute("id").toInt();

	switch ( requestTypeId ) {
			
		case 100: //authorization
			receiveAuthorizationRequest( requestNode );
			break;
		
		case 110: //node send modules list
			receiveModulesListRequest ( requestNode );
			break;
			
		case 123: //module execution ended. status:failed
			moduleExecutionEnded ( requestNode );
			break;
			
//		case 666: //debug
//			emit sendDataToRemoteHost_signal( "<answer id='666'/>" );
//			break;
	
		default :
			logMessage (LOG_ERROR, "Unknown request type. Skip" );
			break;
	};
};


/* 
 * Protocol switch where we detect answer id and execute process function
 * */
void DiscompNode::processAnswerFromNode_slot( QString xml_data_str )
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

	//module stoping stage
	if ( this->stoping_module_execution && answerTypeId != 124 ) {//nothing to do. wait while be stoped
		return;
	}


	switch ( answerTypeId ) {
		case 101: //ping
			setNodePing ( answerNode );
			break;
		case 106: //system information
			receiveSysInfo( answerNode );
			break;

	
		case 120: //remote node send status about prepare module execution
			checkPrepareForStartModule ( answerNode );
			break;

		case 121: //remote node send status about receiving input parameters
			checkInputParametersSending ( answerNode );
			break;

		case 122: //remote node send status about module execution
			checkModuleStarting( answerNode );
			break;

		case 124: //remote node send status about module execution
			checkModuleStoping( answerNode );
			break;

		case 200: //monitoring data
			processMonitoringData(answerNode);
			break;

		default :
			logMessage (LOG_ERROR, "Unknown answer type id. Skip" );
			break;
				
	};
};


void DiscompNode::processMonitoringData ( const QDomNode & answerNode )
{
	QString xml_str;
	QTextStream stream(&xml_str);
	answerNode.firstChild().save(stream, -1);

	this->m_monitor_data.push_front( xml_str );
	if ( this->m_monitor_data.size () > 30 ) {
		this->m_monitor_data.pop_back();
	};

	QDomNode sysinfoNode = XML::getChildNodeByTag("sysinfo", answerNode.firstChild());
	this->m_rrd->setSubdirPath ( sysinfoNode.toElement().attribute("hostname") );
	this->m_rrd->setData(xml_str);

	//TODO: save to file

	//process monitoring data by CLIPS
	
//        discomp_clips->processMonitoringData( answerNode );

//	emit processMonitoringData_signal ( this->id, this->name ,  xml_data_str );
}

QString DiscompNode::getMonitoringData ( const QString & interval )
{
	logMessage ( LOG_COMMON, "Get node monitoring data for interval : " + interval );
	if ( this->m_monitor_data.size() ) { 
		QString xml_data = this->m_monitor_data.at(0);
		return xml_data;//.remove(QRegExp("<?xml.*?>"));
	} else {
		return "";
	};
}









/***********************************************************************/
/*----------------- Protocol processing functions ---------------------*/
/***********************************************************************/


/* 
 * node send Authorization request
 * */
void DiscompNode::receiveAuthorizationRequest ( QDomNode root_node )
{
	logMessage (LOG_DEBUG,"Node send authorization request");
	
	//search <client> node
	QDomNode client_node = XML::getChildNodeByTag("client", root_node);

	this->name    = client_node.toElement().attribute("name");
	this->cpu_num = client_node.toElement().attribute("cpu_num").toInt();
	this->sysinfo_xml = XML::getTextNodeByTag("sysinfo", root_node);
	this->client_version = client_node.toElement().attribute("version");
			
	if ( this->name != "" ) {//node name specified
		logMessage (LOG_COMMON, "Authorization success" );
				
		this->authorized = true;
		emit sendDataToRemoteHost_signal("<answer id='100' status='1' msg='success'/>");
		sendPingRequest();
		//now node should send modules list to us
				
		return;
	};
			
	
	logMessage (LOG_COMMON, "Authorization failed" );
	emit sendDataToRemoteHost_signal("<answer id='100' status='0' msg='authorization failed'/>");
};

void DiscompNode::updateSysInfo ( )
{
	logMessage (LOG_COMMON, "Update system information from node" );
	emit sendDataToRemoteHost_signal("<request id='106' status='1' msg='get system information'/>");
}
void DiscompNode::receiveSysInfo( QDomNode root_node ) 
{
	logMessage (LOG_DEBUG,"Node send system information");
	
	//search <client> node
	this->sysinfo_xml = XML::getTextNodeByTag("sysinfo", root_node);
}

/*
 * Node send modules list which he have installed.
 * In this function we initialize module class on the server side
 * */
void DiscompNode::receiveModulesListRequest ( QDomNode root_node ) 
{
	QString answerXML = "";
	bool found_flag = false;

	//clear all created modules classes
	logMessage (LOG_DEBUG,"Destroy all modules classes for this node");
	for ( int mod_num=0; mod_num < (int)modules.size(); mod_num++ ) {
		delete modules[mod_num];
	};
	modules.clear();


	QDomNode data_node = root_node.firstChild();
	while ( !data_node.isNull() ) {
		if ( data_node.toElement().tagName() == "module" ) {
			//get module id, name
			QString moduleName  = data_node.toElement().attribute("name");
			QString moduleMD5   = data_node.toElement().attribute("md5");
			int     allowUpdate = data_node.toElement().attribute("allow_update").toInt();

			//load module specification
			DiscompModule *newModule = new DiscompModule();
			newModule->log_file   = this->log_file;

			//check module MD5 on the client and server sides
			bool module_status = true;
			if ( moduleName != "system.single" ) {
			QString server_module_md5 = newModule->getModuleMD5(moduleName);
			if ( moduleMD5 != server_module_md5 && server_module_md5 != "" ) {
				//check failed upgrade modules for skip forever update
				if ( upgrade_last_module == moduleName ) {
					upgrade_failed_modules.push_back(moduleName);
				};
				for ( int i=0; i<upgrade_failed_modules.size(); i++ ) {
					if ( moduleName == upgrade_failed_modules.at(i) ) {
						module_status = false;
					};
				};

				logMessage ( LOG_ERROR, "MD5 sums for module: " + moduleName + " are different on the client (" + moduleMD5 + ") and server ("+server_module_md5+") sides." );
				
				if ( module_status && allowUpdate == 1 ) {
					logMessage ( LOG_ERROR, "Node initialization stoped. Need to update module: " + moduleName + " on the client side" );

					module_status = false;
					upgrade_last_module = moduleName;
					this->updateModuleOnTheNode( moduleName );
					return;
				} else if ( module_status ) {
					logMessage ( LOG_ERROR, "Auto upgrade is disabled in the client configuration. Need to update manually" );
					module_status = false;
				};
			};
			};


			if ( moduleName == "system.single" ) {
				newModule->name = "system.single";
				modules.push_back ( newModule );
				
				answerXML += QString("<module name='%1' status='1'/>\n").arg(moduleName);


			} else if (module_status && newModule->LoadModule( moduleName ))  {
				
				modules.push_back ( newModule );
				
				answerXML += QString("<module name='%1' status='1'/>\n").arg(moduleName);
				
			} else {
				
				logMessage (LOG_ERROR,"Could't load module " + moduleName );
				delete newModule;
				
				answerXML += QString("<module name='%1' status='0'/>\n").arg(moduleName);
			};
			
			found_flag = true;
		};
		data_node = data_node.nextSibling();
	};
	if ( !found_flag ) { 
		logMessage(LOG_WARNING, "Recevied empty modules list from node.");
	};

	answerXML = "<answer id='110'>\n" + answerXML + "</answer>";
	emit sendDataToRemoteHost_signal ( answerXML );

	logMessage(LOG_DEBUG,"Tell to server that we connected new node");
	emit nodeConnected_signal( this->id );
};

void DiscompNode::sendPingRequest ( )
{
	QTime time = QTime::currentTime();
	qint64 curr_time = time.hour()*60*60*1000 + time.minute()*60*1000 + time.second()*1000 + time.msec();
	emit sendDataToRemoteHost_signal ( QString("<request id='101' ping_time='%1'/>").arg(curr_time) );
}
void DiscompNode::setNodePing ( QDomNode root_node )
{
	qint64 prev_time = root_node.toElement().attribute("ping_time").toLongLong();

	QTime time = QTime::currentTime();
	qint64 curr_time = time.hour()*60*60*1000 + time.minute()*60*1000 + time.second()*1000 + time.msec() ;

	logMessage(LOG_DEBUG, "Time from node :" + QString("%1").arg(prev_time) + "; Current time:" + QString("%1").arg(curr_time) ) ;

	if ( curr_time >= prev_time ) {
		this->ping_msec = (curr_time-prev_time)/2;
		logMessage ( LOG_DEBUG, "Node ping is: " + QString ( "%1" ).arg( this->ping_msec ) );
	} else {
		//somthing wrong. maybe time returned in a new day 0:00 o'clock
	};
}










/***********************************************************************/
/*-----------------------  MODULE EXECUTION  --------------------------*/
/***********************************************************************/
/*
 * Execution module on remote host have 6 stages:
 * 1. we tell to node that we want to execute module N
 * 2. if he ready we send all input files to remote host
 * 3. when all files done we send request where describe all parameters which we send to him
 * 4. module check it and send success status if it's ok
 * 5. prepare done. now we send request for execute module
 * 6. when execution will be finished remote host send all output parameters to us.
 * */

/*
 * Send request for prepare module execution
 * */
void DiscompNode::prepareModule ( const QString &module_name ) 
{
	this->current_module_exec = module_name;

	QDateTime timestamp (QDateTime::currentDateTime ());
	this->modules[getModuleNumByName(module_name)]->execution_start_time = timestamp.toTime_t();
	this->modules[getModuleNumByName(module_name)]->execution_start_time_ms.restart();

	QTime time = QTime::currentTime();
	qint64 curr_time = time.hour()*60*60*1000 + time.minute()*60*1000 + time.second()*1000 + time.msec();
	
	//TODO. getModuleMD5() for each module will use too much rewsources.
	//maybe check here if current process_id and prevoius_process id is differ, than 
	//call getModuleMD%
	if ( this->process_id != this->previous_process_id ) {
		this->modules[getModuleNumByName(module_name)]->md5 = this->modules[getModuleNumByName(module_name)]->getModuleMD5(); 
	};
	
	QString xml = "";
	xml += "<request id='120' name='prepare module for start' ping_time='" + QString("%1").arg(curr_time) + "'>\n";
	xml += "	<module name='" + module_name + "' md5='"+this->modules[getModuleNumByName(module_name)]->md5+"'/>\n";
	xml += "</request>";

	emit sendDataToRemoteHost_signal( xml );
};

/*
 * If remote host ready we start to send all input files
 * */
void DiscompNode::checkPrepareForStartModule ( QDomNode root_node ) 
{
	QDomNode moduleNode = XML::getChildNodeByTag( "module", root_node );
	
	QString module_name  = moduleNode.toElement().attribute("name");
	int module_num = getModuleNumByName(module_name);	

	if ( root_node.toElement().attribute("status").toInt() != 1 ) {
		if ( root_node.toElement().attribute("substatus") == "md5_error" ) { //md5 is differ

			QString error_msg = XML::getTextNodeByTag("error", root_node);
			logMessage (LOG_ERROR, "Couldn't prepare module for start. " + error_msg );

			//this->clearNodeState();
			//it will be called in the prepareModuleFailed_slot

			emit prepareModuleFailed_signal( this->id, module_name, 6, error_msg );

			this->updateModuleOnTheNode( modules[module_num]->name );

			return;
		} else {
			QString error_msg = XML::getTextNodeByTag("error", root_node);
			logMessage (LOG_COMMON, QString("Remote node told that module '%1' don't ready for execution").arg(module_name) );
			logMessage (LOG_COMMON, QString("Client error message: %1").arg(error_msg) );

			emit prepareModuleFailed_signal( this->id, module_name, 5, error_msg );
			return;
		};
	};

	setNodePing ( root_node );


	//send input parameters
	this->files_for_send.clear();


	if ( !modules[module_num]->inputParameters.size() ) {
		allInputParametersSent();
		logMessage (LOG_WARNING, QString("Module '%1' doesn't have aby input parameters in the specification").arg(module_name) );
		logMessage (LOG_COMMON, QString("Module '%1' prepared for execution.").arg(module_name) );
		return;
	};


	
	logMessage (LOG_COMMON, QString("Module '%1' prepared for execution. Send input files").arg(module_name) );

	logMessage(LOG_DEBUG, "Connect fileSendingDone_signal -> inputParameterSendFileDone_slot");

	disconnect ( nodeSocket, SIGNAL ( fileSendingDone_signal          (QString, int, QString) ),
	             this,       SLOT   ( inputParameterSendFileDone_slot (QString, int, QString) ) );
	connect ( nodeSocket, SIGNAL ( fileSendingDone_signal          (QString, int, QString) ),
	          this,       SLOT   ( inputParameterSendFileDone_slot (QString, int, QString) ) );



	//for sending multiple files to remote host create alist of all files and only after that gives it tothe discomp_socket
	//in other case some time on windows discomp_scoket works in many threads and why executing sendFile when discomp_socket 
	//already busy it can break some files sending
	QVector<QStringList> files_list_for_send;

	for ( int param_num=0; param_num < (int)modules[module_num]->inputParameters.size(); param_num++ ) {
		DiscompModuleParameter *currParam = modules[module_num]->inputParameters[param_num];

		logMessage ( LOG_DEBUG, QString("%1").arg(currParam->name));

		if ( currParam->constant && module_name == this->previous_module_name && this->process_id == this->previous_process_id  ) {
			logMessage(LOG_DEBUG, QString("Parameter '%1' is constant and already sent to the module. Skip").arg(currParam->name) );
			continue;
		};

		if ( currParam->type == "file" ) {
			if ( !QFile::exists ( currParam->filepath ) ) {
				QString error_msg = QString("Input parameter '%1' is not exists (path: %2)").arg(currParam->name).arg(currParam->filepath);
				logMessage ( LOG_ERROR, error_msg );
				emit prepareModuleFailed_signal( this->id, module_name, 5, error_msg );
				return;
			};
			this->files_for_send << currParam->filepath;

			emit sendFile_signal ( currParam->filepath, "", currParam->filename );
//			QStringList file_for_send;
			//file_for_send << currParam->filepath << "modules/" + modules[module_num]->name << currParam->filename;
			//send to the current dir. module should set this dir manualy for socket when called prepareModule
//			file_for_send << currParam->filepath << currParam->filename;
//			files_list_for_send.append(file_for_send);
			//nodeSocket->sendFile( currParam->filepath, "modules/" + modules[module_num]->name, currParam->filename );
			
		} else if ( currParam->type == "filelist" ) {
			QStringList files_names;
			int list_size = currParam->listsize(1);

			logMessage (LOG_DEBUG, "Start checking");
			for ( int list_num=0; list_num < (int)list_size ; list_num++ ) {

				QString filepath = QString(currParam->filepath).arg(list_num);

				if ( !QFile::exists ( filepath ) ) {
					QString error_msg = QString("Input parameter '%1' is not exists (path: %2)").arg(currParam->name).arg(filepath);;
					logMessage ( LOG_ERROR, error_msg );
					emit prepareModuleFailed_signal( this->id, module_name, 5, error_msg );
					return;
				};
				this->files_for_send << filepath;
				files_names << QString(currParam->filepattern).arg(list_num);
			};
			logMessage (LOG_DEBUG, "Done checking");

			emit sendFile_signal ( this->files_for_send, files_names );

		};
	};

//	emit sendFiles_signal(files_list_for_send);
//
//	for ( int i=0; i < this->files_for_send.size(); i++ ) {
//		logMessage(LOG_COMMONT,  QString(this->files_for_send.at(i) + ":" + this->files_for_send.at(i)));
//	};


};

/*
 * This slot called when file has been sent to remote host. When all files 
 * has been sent we sent info about parameters
 * */
void DiscompNode::inputParameterSendFileDone_slot ( QString filename, int status, QString reason ) 
{
	logMessage (LOG_DEBUG, "We in the inputParameterSendFileDone_slot");
	logMessage (LOG_DEBUG, QString("files_for_send.size() is '%1'").arg(this->files_for_send.size()) );
	//TODO some log message should be here!!!
	if ( this->files_for_send.size() == 0 ) {
		allInputParametersSent();
		return;
	};

	
	// search: it's our file?
	for ( int i=0; i < this->files_for_send.size(); i++ ) {
		if ( filename == this->files_for_send.at(i) ) {
			if ( !status ) {
				logMessage(LOG_DEBUG, QString("Couldn't send file to remote host:%1. Reason:%2. Module cannot be started").arg(filename).arg(reason) );
				emit prepareModuleFailed_signal( this->id, this->current_module_exec, 5, QString("Could't send file to remote host:%1").arg(filename) );
				this->files_for_send.clear();
				emit clearFilesQueue_signal();
				return;

			};
			logMessage(LOG_DEBUG, QString("File sending done:%1").arg(filename) );
			files_for_send.removeAt(i);
			logMessage(LOG_DEBUG, QString("size:%1").arg(files_for_send.size()) );
			if ( files_for_send.size() == 0 ) {
				this->allInputParametersSent();
		

				logMessage(LOG_DEBUG, "Disconnect fileSendingDone_signal -> inputParameterSendFileDone_slot");
				//qobject disconnect "file sending signal"
				disconnect ( nodeSocket, SIGNAL ( fileSendingDone_signal          (QString, int, QString) ),
				             this,       SLOT   ( inputParameterSendFileDone_slot (QString, int, QString) ) );

			};
			break;
		};
	};
};


/*
 * All input files has been sent. Now we should send info about 
 * all parameters for module
 * */
void DiscompNode::allInputParametersSent() {
	if ( this->stoping_module_execution ) {
		return;
	}
	// sendInputParametersToNode
	logMessage (LOG_COMMON, "All input parameters sucessfully sent to remote host");
	
	const QString &module_name = this->current_module_exec;
	
	int currentModuleNum = getModuleNumByName(module_name);	

	if ( currentModuleNum == -1 ) {
		logMessage ( LOG_ERROR, "Current module num isn't defined " );
		return;
	};
	
	QStringList files_for_send;
	
	QString parameters_list = "";

	//input parameters
	for ( int param_num=0; param_num < (int)modules[currentModuleNum]->inputParameters.size(); param_num++ ) {
		DiscompModuleParameter *currParam = modules[currentModuleNum]->inputParameters[param_num];

		if ( currParam->type == "file" ) {
			parameters_list += "\t\t\t<param name='"+ currParam->name +"'/>\n";
		} else if ( currParam->type == "filelist" ) {
			parameters_list += "\t\t\t<param name='"+ currParam->name +"'/>\n";
		};
	};

	//output parameters
	for ( int param_num=0; param_num < (int)modules[currentModuleNum]->outputParameters.size(); param_num++ ) {
		DiscompModuleParameter *currParam = modules[currentModuleNum]->outputParameters[param_num];

		if ( currParam->filename != "" ) {
			//if module calculated the element of the list, then it should save file on server with this name
			parameters_list += "\t\t\t<param name='"+ currParam->name +"' remote_filename='"+currParam->filename+"'/>\n";
		} else {
			parameters_list += "\t\t\t<param name='"+ currParam->name +"'/>\n";
		};
	};


	QString xml = "";
	xml += "<request id='121' name='input parameters'>\n";
	xml += "	<module name='" + module_name + "' process_parameters_dir='" + modules[currentModuleNum]->process_parameters_dir + "'>\n";
	xml += "		<parameters>\n";
	xml += 				parameters_list;
	xml += "		</parameters>\n";
	xml += "	</module>\n";
	xml += "</request>";

	emit sendDataToRemoteHost_signal( xml );
};


/*
 * Prepare done. ALl input parameters has been transfered to remote side
 * Module ready for execution
 * */
//this function will call only if errors
void DiscompNode::checkInputParametersSending( QDomNode root_node )
{
	QDomNode moduleNode = XML::getChildNodeByTag( "module", root_node );
	QString module_name  = moduleNode.toElement().attribute("name");
	//int module_num = getModuleNumByName(module_name);	

	if ( root_node.toElement().attribute("status").toInt() != 1 ) {
		QString error_msg = XML::getTextNodeByTag("error", root_node);
		logMessage (LOG_COMMON, QString("Remote node told that module '%1') don't ready for execution").arg(module_name) );
		logMessage (LOG_COMMON, QString("Client error message: %1").arg(error_msg) );

		emit prepareModuleFailed_signal( this->id, module_name, 5, error_msg );
		return;
	};

	logMessage (LOG_COMMON, QString("Module '%1' ready for execution").arg(module_name) );
	emit prepareModuleSuccess_signal( this->id, module_name );
};


/*
 * Send command to start module on remote host
 * */
//this function will never call
void DiscompNode::startModule ( const QString &module_name )
{
	QString xml = "";
	xml += "<request id='122' name='start'>\n";
	xml += "	<module name='" + module_name + "'/>\n";
	xml += "</request>";

	emit sendDataToRemoteHost_signal( xml );
	
	logMessage (LOG_COMMON, QString("Tell to remote node that he should start module '%1'").arg(module_name) );
};

/*
 * Remote hosst send status about module starting
 * */
void DiscompNode::checkModuleStarting( QDomNode root_node )
{
	QDomNode moduleNode = XML::getChildNodeByTag( "module", root_node );
	QString module_name = moduleNode.toElement().attribute("name");
	//int module_num = getModuleNumByName(module_name);	


	if ( root_node.toElement().attribute("status").toInt() != 1 ) {
		QString error_msg = XML::getTextNodeByTag("error", root_node);
		logMessage (LOG_ERROR, QString("Could not start module '%1' on the remote host.").arg(module_name));
		logMessage (LOG_ERROR, QString("Client error message: %1").arg(error_msg) );

		//extract archive
		extractFailedModuleArchive ( module_name, moduleNode.toElement().attribute("file_archive") );

		emit startModuleFailed_signal( this->id, module_name, 2, error_msg );
		return;
	};

	logMessage (LOG_COMMON, QString("Module '%1' successfully started on the remote host").arg(module_name) );
	emit startModuleSuccess_signal( this->id, module_name );
};



/*
 * Send command to stop module on remote host
 * */
void DiscompNode::stopModule ( const QString &module_name )
{
	QString xml = "";
	xml += "<request id='124' name='stop'>\n";
	xml += "	<module name='" + module_name + "'/>\n";
	xml += "</request>";

	emit clearFilesQueue_signal();
	emit sendDataToRemoteHost_signal( xml );
	
	logMessage (LOG_COMMON, QString("Tell to remote node to STOP module '%1'").arg(module_name) );
};

/*
 * Remote host send status about module stoping
 * */
void DiscompNode::checkModuleStoping( QDomNode root_node )
{
	QDomNode moduleNode = XML::getChildNodeByTag( "module", root_node );
	QString module_name = moduleNode.toElement().attribute("name");
	int module_num = getModuleNumByName(module_name);	

	modules[module_num]->execution_real_time    = moduleNode.toElement().attribute("time").toInt();
	modules[module_num]->execution_real_time_ms = moduleNode.toElement().attribute("time_ms").toInt();
	modules[module_num]->node_resources_usage_info = moduleNode.toElement().attribute("usage_info");

	if ( root_node.toElement().attribute("status").toInt() != 1 ) {
		QString error_msg = XML::getTextNodeByTag("error", root_node);
		logMessage (LOG_COMMON, QString("Module '%1' NOT STOPED on the remote node.").arg(module_name) );
		logMessage (LOG_COMMON, QString("Client error message: %1").arg(error_msg) );
		emit stopModuleFailed_signal( this->id, module_name, error_msg );
		return;
	};

	logMessage (LOG_COMMON, QString("Module '%1' was STOPED on the remote host").arg(module_name) );
	emit stopModuleSuccess_signal( this->id, module_name );

};



/*
 * Module execution finised. All parameters should be at our side
 * */
void DiscompNode::moduleExecutionEnded ( QDomNode root_node )
{
	QDomNode moduleNode = XML::getChildNodeByTag( "module", root_node );
	QString module_name = moduleNode.toElement().attribute("name");
	int module_num  = getModuleNumByName(module_name);	

	if ( module_name != current_module_exec ) {
		logMessage ( LOG_WARNING, "Received module finished signal from node. Module tell that finished module: " + module_name + ", but we waiting module: " + current_module_exec );
		return;
	};

	modules[module_num]->execution_real_time    = moduleNode.toElement().attribute("time").toInt();
	modules[module_num]->execution_real_time_ms = moduleNode.toElement().attribute("time_ms").toInt();
	modules[module_num]->node_resources_usage_info = moduleNode.toElement().attribute("usage_info");
	
	if ( root_node.toElement().attribute("status").toInt() != 1 ) {
		QString error_msg = XML::getTextNodeByTag("error", root_node);
		logMessage (LOG_COMMON, QString("Module '%1' execution failed").arg(module_name) );
		logMessage (LOG_COMMON, QString("Client error message: %1").arg(error_msg) );

		//extract archive
		if ( root_node.toElement().attribute("status").toInt() != 2 ) { // 2 - max time reached
			logMessage ( LOG_COMMON, "Extract failed module archive" );
			extractFailedModuleArchive ( module_name, moduleNode.toElement().attribute("file_archive") );
		};

		emit endModuleFailed_signal ( this->id, module_name, root_node.toElement().attribute("status").toInt(), error_msg );
		return;
	}

	//for constant parameters
	this->previous_module_name = module_name;
	this->previous_process_id = this->process_id;

	//check if all output parameters has been successfully received.
	for ( int param_num=0; param_num < (int)modules[module_num]->outputParameters.size(); param_num++ ) {
		DiscompModuleParameter *currParam = modules[module_num]->outputParameters[param_num];

		if ( !currParam->is_defined() ) {
			logMessage (LOG_ERROR, QString("Parameter '%1' is not defined").arg(currParam->name) );

			emit endModuleFailed_signal ( this->id, module_name, 1, QString("Parameter '%1' is not defined").arg(currParam->name) );
			return;
		};
	};


	logMessage (LOG_COMMON,"Module successfully finished. All output parameters exists in the global parameters space");
	emit endModuleSuccess_signal ( this->id, module_name );
};



/***********************************************************************/
/*-----------------------  Update modules -----------------------------*/
/***********************************************************************/


void DiscompNode::updateModuleOnTheNode( const QString &module_name )
{
	//remove operation from module_name
	QString abs_module_name = QString(module_name).replace ( QRegExp (":.*$"), "" );

	if ( this->cpu_num != 0 ) {
		logMessage(LOG_NOTICE, "This client node is not general on the client side (client has more than one cpu). Wait while module will be upgraded.");
		emit sendDataToRemoteHost_signal("<request id='112'/>");
		return;
	};

	//define module dir
	QStringList mod_info = abs_module_name.split(".");
	if ( mod_info.size() < 2 || mod_info.at(0) == "" || mod_info.at(1) == "" )
	{
		return;
	};
	//
	QString module_dir = "";
	if ( Config::variables["PackagesDirectory"] != "" ) {
		module_dir = QDir::convertSeparators ( Config::variables["PackagesDirectory"] + "/" + mod_info.at(0) + "/" + QString(MODULES_DIR) + "/" + mod_info.at(1) + "/" );
	} else {
		module_dir = QDir::convertSeparators ( Config::variables["BaseDirectory"] + "/" + QString(PACKAGES_DIR) + "/" + mod_info.at(0) + "/" + QString(MODULES_DIR) + "/" + mod_info.at(1) + "/" );
	};

	
	QString archive_file = File::createTempFile("archive_"+abs_module_name +"_");
	if ( !File::createArchive( module_dir, archive_file ) ) {
		logMessage ( LOG_ERROR, "Couldn't create archive file: " + archive_file );
		return;
	};

	this->module_name_for_upgrade = abs_module_name;

	connect ( nodeSocket, SIGNAL ( fileSendingDone_signal           (QString, int, QString) ),
	          this,       SLOT   ( updateModuleArchiveUploaded_slot (QString, int, QString) ) );

	modules.clear();

	emit sendFile_signal ( archive_file ); 

}

void DiscompNode::updateModuleArchiveUploaded_slot ( QString filename, int status, QString reason )
{
	disconnect ( nodeSocket, SIGNAL ( fileSendingDone_signal           (QString, int, QString) ),
	             this,       SLOT   ( updateModuleArchiveUploaded_slot (QString, int, QString) ) );

	if ( !status ) {
		logMessage ( LOG_ERROR, "Module upgrade failed. Reason: " + reason );
		return;
	};

	QFile::remove ( filename );
	
	//send info to node that archive uploaded
	QFileInfo file_info ( filename );
	emit sendDataToRemoteHost_signal("<answer id='111' status='1' msg='upgrade module'><info module_name='" + this->module_name_for_upgrade + "' file_name='" + file_info.fileName() + "' /></answer>");
}


void DiscompNode::extractFailedModuleArchive ( const QString &module_name, const QString &filename)
{
	int module_num  = getModuleNumByName(module_name);	

	QString file_archive = Config::variables["BaseDirectory"] + QDir::separator() + "tmp" + QDir::separator() + filename;

	if ( QFile::exists(file_archive) ) 
	{
		QString extract_path = File::createTempDirectory ( QString("node_%1_%2_%3_").arg(this->id).arg(this->ip).arg(modules[module_num]->name.replace(":","_")), 
								   this->process_dir + QDir::separator() + "failedmodules" );

		if ( extract_path != "" && File::extractArchive ( file_archive, extract_path ) ) {
			logMessage ( LOG_ERROR, "Module temporary directory (from client side) located in the: " + extract_path );
		} else {
			logMessage ( LOG_ERROR, "Couldn't extract module temporary directory archive: " + file_archive + " to the dir: " + extract_path );
		};

		QFile::remove(file_archive);
	} else {
		logMessage ( LOG_ERROR, "Could not extract failed module archive. File isn't exists: " + file_archive );
	};
};


/***********************************************************************/
/*-------------------------  Public methods  --------------------------*/
/***********************************************************************/
void DiscompNode::clearNodeState () 
{
	logMessage ( LOG_DEBUG, "Clear node state" );
	this->busy = false;
	this->stoping_module_execution = false;
	this->process_id = -1;
	this->process_list_element_num=0;
	this->current_module_exec = "";

	disconnect ( nodeSocket, SIGNAL ( fileSendingDone_signal           (QString, int, QString) ),
	             this,       SLOT   ( updateModuleArchiveUploaded_slot (QString, int, QString) ) );

	disconnect ( nodeSocket, SIGNAL ( fileSendingDone_signal          (QString, int, QString) ),
	             this,       SLOT   ( inputParameterSendFileDone_slot (QString, int, QString) ) );

	//send input parameters
	this->files_for_send.clear();
};
void DiscompNode::addFailedModule (int process_id, const QString &module_name, int reason, QString msg)
{
	logMessage(LOG_WARNING, QString("Add module:%1, process_id:%2 to the list of failed modules. Reason:%3").arg(module_name).arg(process_id).arg(msg));
	logMessage(LOG_WARNING, QString("This module will be disabled for this node") );
	
	//TODO. we don't check failed modules now. just remove this module for node
	for ( int mod_num=0; mod_num < (int)modules.size(); mod_num++ ) {
		if ( modules[mod_num]->name == module_name ) {
			modules[mod_num]->is_failed_module = true;
			break;
		};
	};
	return;
	
	
	
	DiscompNodeFailedModule failed_module;
	failed_module.process_id = process_id;
	failed_module.module_name= module_name;
	failed_module.reason     = reason;
	failed_module.msg        = msg;

//	failedModules << failed_module;
};

void DiscompNode::clearFailedModules ()
{
	logMessage(LOG_WARNING, QString("Clear failed modules list"));
	
	//TODO. we don't check failed modules now. just remove this module for node
	for ( int mod_num=0; mod_num < (int)modules.size(); mod_num++ ) {
		modules[mod_num]->is_failed_module = false;
	};
};



int DiscompNode::isFailedModule (int process_id, const QString &module_name)
{
	//TODO
	return false;


	for (int i = 0; i < failedModules.size(); ++i) {
		if (failedModules.at(i).process_id == process_id && failedModules.at(i).module_name == module_name ) {
			logMessage(LOG_COMMON, QString("The module:%1, process_id:%2 is in the list of failed modules. Reason:%3").arg(module_name).arg(process_id).arg(failedModules.at(i).msg));
			return true;
		};
	};
	return false;
};

/* search module in private modules space */
int DiscompNode::getModuleNumByName( const QString &module_name )
{
	int moduleNum = -1;
	
	for ( int mod_num=0; mod_num < (int)modules.size(); mod_num++ ) {
		if ( modules[mod_num]->name == module_name ) {
			moduleNum = mod_num;
			break;
		};
	};

	return moduleNum;
};

/**
 * Set additional log file
 * for copy data to it.]
 * used from process
 */
void DiscompNode::setAdditionalLogFile ( const QString &file_path ) {
	this->addit_log_file = file_path;
	this->nodeSocket->addit_log_file = file_path;
};


/* diconnect host from server */
/*
void DiscompNode::disconnectFromHost() 
{
	logMessage(LOG_COMMON, "Disconnect From Server");
	terminate();
	wait();
	nodeSocket->disconnectFromHost();
	connectionClosed_slot();
}
*/



/***********************************************************************/
/*-----------------------  Private functions  -------------------------*/
/***********************************************************************/

void DiscompNode::logMessage ( int log_level, const QString &message ) 
{
	Log::addMessage (this->log_file, log_level, "DiscompNode", message);

	if ( this->addit_log_file != "" ) {
		Log::addMessage (this->addit_log_file, log_level, "DiscompNode", message);
	};
};



// vim: set fenc=utf-8 tabstop=8 :
