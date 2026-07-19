// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#include "server.h"
#include "user.h"
#include "lib/socket.h"
#include "lib/common.h"
#include "rpc_thread.h"


/**
 * @class DiscompServerRPCThread
 * @brief Server side rpc implementation
 *
 * In this class implemented processing functions from rpc-client.
 * 
 * \sa DiscompRPC
 */

/***********************************************************************/
/*-----------------  Constructor & Destructor  ------------------------*/
/***********************************************************************/

/**
 * Constructor 
 * */
DiscompServerRPCThread::DiscompServerRPCThread(int sock_desc)
{
	socket_descriptor = sock_desc;

	logMessage ( LOG_DEBUG, "Thread constructed" );
};

/**
 * Destructor 
 * */
DiscompServerRPCThread::~DiscompServerRPCThread()
{
	if ( !host_socket ) {
		return;
	};

	emit doDisconnect_signal();

	if ( host_socket->state() == QAbstractSocket::ConnectedState ) 
		host_socket->waitForDisconnected();

	delete host_socket;
	
	logMessage ( LOG_DEBUG, "Thread destroyed" );
};


/** 
 * Start function for thread 
 * */
void DiscompServerRPCThread::run()
{
	logMessage ( LOG_DEBUG, "Start Discomp RPC Thread" );
	
	//create TCP Socket
	host_socket = new DiscompSocket(socket_descriptor);
	host_socket->log_file = QString("server_rpc.%1").arg(LOG_FILE_EXT);
	if ( !host_socket->ok ) {
		logMessage(LOG_ERROR, "TCP socket creation failed");
	}

	//check access hosts allow
	bool flag;
	QStringList hosts_allow = Config::variables["ServerRPCHostsAllow"].split(',');
	for (int i=0; i <hosts_allow.size(); i++ ) {
		if ( host_socket->peerAddress().toString() == hosts_allow[i] ) {
			flag = true;
		};
	};
	if ( !flag ) { 
		logMessage (LOG_WARNING, "Access from this host denied by server configuration. Check ServerRPCHostsAllow in server.conf");
		delete host_socket;
		return; 
	};


	connect ( host_socket, SIGNAL ( connectionClosed_signal ( )                           ),
	          this,        SLOT   ( connectionClosed_slot   ( )                           ) );
	connect ( host_socket, SIGNAL ( processRequestFromRemoteHost_signal (const QString &) ),
	          this,        SLOT   ( processRequestFromHost_slot         (const QString &) ) );
//	connect ( host_socket, SIGNAL ( processAnswerFromRemoteHost_signal  (const QString &) ),
//	          this,        SLOT   ( processAnswerFromHost_slot          (const QString &) ) );
	connect ( this,        SIGNAL ( doDisconnect_signal                 ( )               ),
	          host_socket, SLOT   ( doDisconnect_slot                   ( )               ) );
	connect ( this,        SIGNAL ( sendDataToRemoteHost_signal         (const QString &) ),
	          host_socket, SLOT   ( sendDataToRemoteHost_slot           (const QString &) ) );

	logMessage ( LOG_DEBUG, "Thread started");

	logMessage ( LOG_COMMON, QString ("Connected new rpc host IP %1").arg(host_socket->peerAddress().toString()) );
	
	exec();

	logMessage(LOG_DEBUG, "Thread exited");

//	delete host_socket;
	quit();
}

/**
 * Set pointer to the DiscompServer class
 */
void DiscompServerRPCThread::setDiscompServer ( DiscompServer *ds ) 
{
	this->discomp_server_ptr = ds;
}






/***********************************************************************/
/*------------------------  TCP Socket slots  -------------------------*/
/***********************************************************************/
/**
 * Socket connection is closed. Destroy thread.
 */
void DiscompServerRPCThread::connectionClosed_slot()
{
	logMessage ( LOG_DEBUG, "Connection closed. Thread should be destroyed" );
	// stop thread
	this->quit();
};


/* 
 * Protocol switch where we detect request id and execute 
 * process function.
 * */
void DiscompServerRPCThread::processRequestFromHost_slot( const QString & xml_data_str  )
{
	QString errorStr;
	int errorLine;
	int errorColumn;

	QDomDocument doc;
	if ( !doc.setContent( xml_data_str, true, &errorStr, &errorLine, &errorColumn ) ) {
		logMessage (LOG_ERROR, QString("XML processing error ( line %1, column %2: %3 )").arg(errorLine).arg(errorColumn).arg(errorStr) );
		emit sendDataToRemoteHost_signal( QString("<answer>XML processing error ( line %1, column %2: %3 )</answer>").arg(errorLine).arg(errorColumn).arg(errorStr) );
		emit doDisconnect_signal();
		return;
	};

	QDomElement rootNode = doc.documentElement();

	QDomNode methodNode = rootNode.firstChild();

	while ( !methodNode.isNull() && methodNode.nodeType() != QDomNode::ElementNode ) {
		methodNode = methodNode.nextSibling();
	}

	int methodTypeId = methodNode.toElement().attribute("id").toInt();
	this->login    = methodNode.toElement().attribute("login");
	QString password = methodNode.toElement().attribute("password");
	if ( !DiscompUser::authorize ( login, password, this->user_type ) ) {
		emit sendDataToRemoteHost_signal( QString("<answer id='%1' auth='-1' msg='Authorization failed'/>").arg(methodTypeId) );
		emit doDisconnect_signal();
		return;
	};


	switch ( methodTypeId ) {
		//0-10 - works with users
		case 1:
			emit sendDataToRemoteHost_signal( "<answer id='1' auth='1' user_type='"+user_type+"' status='1' msg='Authorization successfull'/>" );
			break;
		case 2: 
			if ( login == SUPERUSER ) {
				createNewUser ( methodNode );
			} else {
				emit sendDataToRemoteHost_signal( "<answer id='2' status='0' msg='You not allowed to create new users'/>" );
			};
			break;
		case 3: //change password
			changeUserPassword ( methodNode, login );
			break;
		case 4: //change password
			if ( login == SUPERUSER ) {
				getUsersList (  );
			} else {
				emit sendDataToRemoteHost_signal( "<answer id='2' status='4' msg='You not allowed to check users list'/>" );
			}
			break;



		//100-110  - works with nodes
		case 100: //connect remote node
			if ( login == SUPERUSER ) {
				connectNode ( methodNode );
			};
			break;
		case 101: //disconnect remote node
			if ( login == SUPERUSER ) {
				disconnectNode ( methodNode );
			};
			break;
		case 102: //get node info
			getNodeInfo ( methodNode );
			break;
		case 103: //nodes list
			getConnectedNodesList (  );
			break;
		case 105: //install module on the remote node
			if ( login == SUPERUSER ) {
				installModuleOnNode ( methodNode );
			};
			break;

		case 106: //update node sys info
			updateNodeSysInfo ( methodNode );
			break;

		case 107: //delete failed module
			if ( login == SUPERUSER ) {
				deleteFailedModule ( methodNode );
			};
			break;
		
		case 108: //clear modules failed
			clearFailedModulesForNodes ( );
			break;



		//110-120  - works with processes
		case 110: //start process
			startProcess ( methodNode );
			break;
		case 115: //processes list
			startSingleProcess ( methodNode );
			break;

		case 111: //stop process
			stopProcess ( methodNode );
			break;
		case 112: //process info
			getProcessInfo ( methodNode );
			break;
		case 113: //processes list
			getProcessesList ( );
			break;

		case 201: //monitoring data
			getNodeMonitoringData( methodNode );
			break;

		default :
			logMessage (LOG_ERROR, "Unknown method type. Skip" );
			emit sendDataToRemoteHost_signal( "<answer id='-1' status='0' msg='Unknown method type'/>" );
			break;
	};

	//break connection
	emit doDisconnect_signal();
};










/***********************************************************************/
/*---- RPC processing functions for users manipulations-------*/
/***********************************************************************/

void DiscompServerRPCThread::getUsersList (  ) 
{
	QString return_xml = "<answer id='4' status='1'>";
	QStringList users_list = DiscompUser::getUsersList ();
	for ( int user_num =0; user_num < users_list.size(); user_num ++ ) {
		QStringList user_info = users_list.at(user_num).split(":");
		if ( user_info.size() > 2 ) {
			return_xml += QString("<user login='%1' type='%2'/>").arg(user_info.at(0)).arg(user_info.at(2));
		};
	};
	return_xml += "</answer>";
	emit sendDataToRemoteHost_signal(return_xml);

};

void DiscompServerRPCThread::createNewUser ( const QDomNode &xml_node ) 
{
	QDomNode data_node = xml_node.firstChild();
	while ( !data_node.isNull() ) {
		if ( data_node.toElement().tagName() == "user" ) {
			QString new_login     = data_node.toElement().attribute("new_login");
			QString new_password  = data_node.toElement().attribute("new_password");
			QString new_type  = data_node.toElement().attribute("type");
			DiscompUser::create ( new_login, new_password, new_type );
			emit sendDataToRemoteHost_signal("<answer id='2' status='1' msg='User created successfully'/>");
			return;
		};
	};
	
	emit sendDataToRemoteHost_signal("<answer id='2' status='0' msg='xml data error'/>");

};

void DiscompServerRPCThread::changeUserPassword ( const QDomNode &xml_node, const QString &login ) 
{
	QDomNode data_node = xml_node.firstChild();
	while ( !data_node.isNull() ) {
		if ( data_node.toElement().tagName() == "user" ) {
			QString new_password  = data_node.toElement().attribute("new_password");
			QString new_type = data_node.toElement().attribute("new_type");
			DiscompUser::changePassword ( login, new_password, new_type );
			emit sendDataToRemoteHost_signal("<answer id='3' status='1' msg='Password changed successfully'/>");
			return;
		};
	};
	
	emit sendDataToRemoteHost_signal("<answer id='3' status='0' msg='xml data error'/>");

};

/***********************************************************************/
/*---------- RPC processing functions for nodes --------------*/
/***********************************************************************/
/**
 * Command for connect new nodes to DiscompServer
 */
void DiscompServerRPCThread::connectNode ( const QDomNode &xml_node )
{
	logMessage (LOG_COMMON,"Command: connect new nodes");

	QDomNode data_node = xml_node.firstChild();
	while ( !data_node.isNull() ) {
		if ( data_node.toElement().tagName() == "node" ) {
			QString node_ip   = data_node.toElement().attribute("ip");
			int     node_port = data_node.toElement().attribute("port").toInt();
			logMessage ( LOG_COMMON, QString("  Connect node: IP:%1, Port:%2").arg(node_ip).arg(node_port) );
			discomp_server_ptr->connectNode ( node_ip, node_port );
		};
		data_node = data_node.nextSibling();
	};
	
	emit sendDataToRemoteHost_signal("<answer id='100' status='1' msg='Nodes connected'/>");
};

/**
 * Command for disconnect node from DiscompServer
 */
void DiscompServerRPCThread::disconnectNode ( const QDomNode &xml_node )
{
	logMessage (LOG_COMMON,"Command: disconnect nodes");

	if ( discomp_server_ptr->nodes.size() < 1 ) {
		emit sendDataToRemoteHost_signal("<answer id='101' status='0' msg='Nodes disconnected'/>");
		return;
	};

	QDomNode data_node = xml_node.firstChild();
	while ( !data_node.isNull() ) {
		if ( data_node.toElement().tagName() == "node" ) {
			int     node_id = data_node.toElement().attribute("id").toInt();

			if ( discomp_server_ptr->nodes.size() > node_id && discomp_server_ptr->nodes[node_id] != NULL  ) {
				QString node_ip   = discomp_server_ptr->nodes[node_id]->ip;
				QString node_name = discomp_server_ptr->nodes[node_id]->name;

				logMessage(LOG_COMMON, QString("  Disconnect node id:%1, IP:%2, Name:%3").arg(node_id).arg(node_ip).arg(node_name) );
				discomp_server_ptr->disconnectNode ( node_id );
			}
		};
		data_node = data_node.nextSibling();
	};
	
	emit sendDataToRemoteHost_signal("<answer id='101' status='1' msg='Nodes disconnected'/>");
};

/**
 * Command for get info about connected node
 */
void DiscompServerRPCThread::getNodeInfo ( const QDomNode &xml_node )
{
	logMessage (LOG_COMMON,"Command: get node info");

	if ( discomp_server_ptr->nodes.size() < 1 ) {
		emit sendDataToRemoteHost_signal("<answer id='102' status='0'/>");
		return;
	};

	QString  xml_nodes_info = "";
	QDomNode data_node = xml_node.firstChild();
	while ( !data_node.isNull() ) {
		if ( data_node.toElement().tagName() == "node" ) {
			int     node_id = data_node.toElement().attribute("id").toInt();
		
			if ( discomp_server_ptr->nodes.size() > node_id && discomp_server_ptr->nodes[node_id] != NULL  ) {
				int     process_id   = discomp_server_ptr->nodes[node_id]->process_id;
				int     process_num  = discomp_server_ptr->getProcessNumById(process_id);
				QString process_name = "";

				QString module_name = discomp_server_ptr->nodes[node_id]->current_module_exec;
				int module_exec_time = 0;
				int module_list_num  = 0;
				if ( module_name != "" ) {
					module_list_num  = discomp_server_ptr->nodes[node_id]->process_list_element_num;
					QDateTime timestamp (QDateTime::currentDateTime ());
					module_exec_time = timestamp.toTime_t() - discomp_server_ptr->nodes[node_id]->modules[ discomp_server_ptr->nodes[node_id]->getModuleNumByName(module_name) ]->execution_start_time;
				};



				QString modules_list="";
				QString failed_modules_list="";
				for ( int mod_num = 0; mod_num < discomp_server_ptr->nodes[node_id]->modules.size(); mod_num ++ ) {
					if ( discomp_server_ptr->nodes[node_id]->modules[mod_num]->is_failed_module ) {
						failed_modules_list += discomp_server_ptr->nodes[node_id]->modules[mod_num]->name;
						//discomp_server_ptr->nodes[node_id]->failedModules[mod_num].module_name + 
						//		":" +  discomp_server_ptr->nodes[node_id]->failedModules[mod_num].reason + " ,";
					} else {
						modules_list += discomp_server_ptr->nodes[node_id]->modules[mod_num]->name + ",";
					};
				};

				QDateTime timestamp (QDateTime::currentDateTime ());
				quint64 node_uptime = timestamp.toTime_t () - discomp_server_ptr->nodes[node_id]->uptime_start;

				if ( process_num != -1 ) {
					process_name = discomp_server_ptr->processes[process_num]->get_name();
				};
				xml_nodes_info += QString("<node id='%1' ip='%2' name='%3' busy='%4' process_id='%5' process_name='%6' module_name='%7' module_list_num='%8' module_exec_time='%9' modules_list='%10' failed_modules_list='%11' ping='%12' client_version='%13' uptime='%14' cpu_num='%15'><sysinfo>%16</sysinfo></node>\n").
					arg(discomp_server_ptr->nodes[node_id]->id).
					arg(discomp_server_ptr->nodes[node_id]->ip).
					arg(discomp_server_ptr->nodes[node_id]->name).
					arg(discomp_server_ptr->nodes[node_id]->busy).
					arg(process_id).
					arg(process_name).
					arg(module_name).
					arg(module_list_num).
					arg(module_exec_time).
					arg(modules_list).
					arg(failed_modules_list).
					arg(discomp_server_ptr->nodes[node_id]->ping_msec).
					arg(discomp_server_ptr->nodes[node_id]->client_version).
					arg(node_uptime).
					arg(discomp_server_ptr->nodes[node_id]->cpu_num).
					arg(discomp_server_ptr->nodes[node_id]->sysinfo_xml);

				logMessage(LOG_COMMON, QString("  Get info for id:%1, IP:%2, Name:%3").arg(node_id).arg(discomp_server_ptr->nodes[node_id]->ip).arg(discomp_server_ptr->nodes[node_id]->name) );
			}
		};
		data_node = data_node.nextSibling();
	};

	emit sendDataToRemoteHost_signal( "<answer id='102' status='1'>\n" + xml_nodes_info + "</answer>" );
}

/**
 * Update node system information state
 */
void DiscompServerRPCThread::updateNodeSysInfo( const QDomNode &xml_node )
{

	logMessage (LOG_COMMON,"Command: update node system information");

	if ( discomp_server_ptr->nodes.size() < 1 ) {
		emit sendDataToRemoteHost_signal("<answer id='106' status='0' msg='no such nodes'/>");
		return;
	};

	QDomNode node_info = XML::getChildNodeByTag ("node", xml_node);
	int     node_id = node_info.toElement().attribute("id").toInt();
	if ( discomp_server_ptr->nodes[node_id] != NULL ) {
		discomp_server_ptr->nodes[node_id]->updateSysInfo();
		emit sendDataToRemoteHost_signal( "<answer id='106' status='1'></answer>" );
	} else {
		emit sendDataToRemoteHost_signal( "<answer id='106' status='0' msg='could not find node'></answer>" );
	};
}


/**
 * Update node system information state
 */
void DiscompServerRPCThread::getNodeMonitoringData( const QDomNode &xml_node )
{

	logMessage (LOG_COMMON,"Command: get node monitoring data");

	if ( discomp_server_ptr->nodes.size() < 1 ) {
		emit sendDataToRemoteHost_signal("<answer id='201' status='0' msg='no such nodes'/>");
		return;
	};

	QDomNode node_info = XML::getChildNodeByTag ("node", xml_node);
	int     node_id = node_info.toElement().attribute("id").toInt();
	if ( discomp_server_ptr->nodes[node_id] != NULL ) {
		QString xml = discomp_server_ptr->nodes[node_id]->getMonitoringData(node_info.toElement().attribute("interval"));
		emit sendDataToRemoteHost_signal( "<answer id='201' status='1'>"+xml+"</answer>" );
	} else {
		emit sendDataToRemoteHost_signal( "<answer id='201' status='0' msg='could not find node'></answer>" );
	};
}

/**
 * Remove node from list of failed module
 */
void DiscompServerRPCThread::deleteFailedModule ( const QDomNode &xml_node )
{
	logMessage (LOG_COMMON,"Command: delete failed module");

	if ( discomp_server_ptr->nodes.size() < 1 ) {
		emit sendDataToRemoteHost_signal("<answer id='107' status='0' msg='no such nodes'/>");
		return;
	};

	QDomNode node_info = XML::getChildNodeByTag ("node", xml_node);
	int      node_id     = node_info.toElement().attribute("id").toInt();
	QString  module_name = node_info.toElement().attribute("module");
	if ( discomp_server_ptr->nodes[node_id] != NULL ) {
		for ( int mod_num = 0; mod_num < discomp_server_ptr->nodes[node_id]->modules.size(); mod_num ++ ) {
			if ( discomp_server_ptr->nodes[node_id]->modules[mod_num]->name == module_name &&
			     discomp_server_ptr->nodes[node_id]->modules[mod_num]->is_failed_module )
			{
				discomp_server_ptr->nodes[node_id]->modules[mod_num]->is_failed_module = false;
				emit sendDataToRemoteHost_signal( "<answer id='107' status='1'></answer>" );
				break;
			};
		};
		emit sendDataToRemoteHost_signal( "<answer id='107' status='0' msg='could not find this failed module'></answer>" );
	} else {
		emit sendDataToRemoteHost_signal( "<answer id='107' status='0' msg='could not find node'></answer>" );
	};
}



/**
 * Command for get list with info about connected nodes
 */
void DiscompServerRPCThread::getConnectedNodesList (  )
{
	logMessage (LOG_COMMON,"Command: nodes list");

	QString  xml_nodes_list = "";
	for ( int node_num=0; node_num < (int)discomp_server_ptr->nodes.size(); node_num++ ) {
		if ( discomp_server_ptr->nodes[node_num] != NULL ) {
				int     process_id   = discomp_server_ptr->nodes[node_num]->process_id;
				int     process_num  = discomp_server_ptr->getProcessNumById(process_id);

				QString process_name = "";
				if ( process_num != -1 ) {
					process_name = discomp_server_ptr->processes[process_num]->get_name();
				};

				QString module_name = discomp_server_ptr->nodes[node_num]->current_module_exec;
				int module_exec_time = 0;
				int module_list_num  = 0;
				if ( module_name != "" ) {
					module_list_num  = discomp_server_ptr->nodes[node_num]->process_list_element_num;
					QDateTime timestamp (QDateTime::currentDateTime ());
					module_exec_time = timestamp.toTime_t() - discomp_server_ptr->nodes[node_num]->modules[ discomp_server_ptr->nodes[node_num]->getModuleNumByName(module_name) ]->execution_start_time;
				};

				QDateTime timestamp (QDateTime::currentDateTime ());
				quint64 node_uptime = timestamp.toTime_t () - discomp_server_ptr->nodes[node_num]->uptime_start;

		
				QString modules_list="";
				for ( int mod_num = 0; mod_num < discomp_server_ptr->nodes[node_num]->modules.size(); mod_num ++ ) {
					modules_list += discomp_server_ptr->nodes[node_num]->modules[mod_num]->name + ",";
				};

				xml_nodes_list += QString("<node id='%1' ip='%2' name='%3' busy='%4' process_id='%5' process_name='%6' module_name='%7' module_list_num='%8' module_exec_time='%9' ping='%10' modules_list='%11' uptime='%12' cpu_num='%13'/>\n").
					arg(discomp_server_ptr->nodes[node_num]->id).
					arg(discomp_server_ptr->nodes[node_num]->ip).
					arg(discomp_server_ptr->nodes[node_num]->name).
					arg(discomp_server_ptr->nodes[node_num]->busy).
					arg(process_id).
					arg(process_name).
					arg(module_name).
					arg(module_list_num).
					arg(module_exec_time).
					arg(discomp_server_ptr->nodes[node_num]->ping_msec).
					arg(modules_list).
					arg(node_uptime).
					arg(discomp_server_ptr->nodes[node_num]->cpu_num);
		};
	};
	
	emit sendDataToRemoteHost_signal( "<answer id='103'>\n" + xml_nodes_list + "</answer>" );
};

/**
 * Install/upgrade module on the remote host
 */
void DiscompServerRPCThread::installModuleOnNode ( const QDomNode &xml_node )
{
	logMessage (LOG_COMMON,"Command: install module on the node");

	QDomNode data_node = xml_node.firstChild();
	while ( !data_node.isNull() ) {
		if ( data_node.toElement().tagName() == "node" ) {
			int     node_id     = data_node.toElement().attribute("id").toInt();
			QDomNode module_node  = data_node.firstChild();
			while ( !module_node.isNull() ) {
				if ( module_node.toElement().tagName() == "module" ) {
					QString module_name = module_node.toElement().attribute("name"); 
					if ( discomp_server_ptr->nodes.size() > node_id && discomp_server_ptr->nodes[node_id] != NULL  ) {
						//TODO! Can be problems with threads.
						//TODO. check module name!!!!
						if ( discomp_server_ptr->nodes[node_id]->cpu_num == 0 ) {
							discomp_server_ptr->nodes[node_id]->updateModuleOnTheNode(module_name);
							emit sendDataToRemoteHost_signal("<answer id='105' status='1' msg='Installing module procedure started.'/>");
							return;
						} else {
							QString my_node_name = discomp_server_ptr->nodes[node_id]->name.split(":").at(0);
							QString need_node_name = discomp_server_ptr->nodes[node_id]->name.split(":").at(0);
							//search node with cpu num = 0
							for ( int node_num=0; node_num < (int)discomp_server_ptr->nodes.size(); node_num++ ) {
								if ( discomp_server_ptr->nodes[node_id]->name.split(":").at(0) == 
								     discomp_server_ptr->nodes[node_num]->name.split(":").at(0) &&
								     discomp_server_ptr->nodes[node_num]->cpu_num == 0 ) 
								{
									discomp_server_ptr->nodes[node_num]->updateModuleOnTheNode(module_name);
								};
							};
							emit sendDataToRemoteHost_signal("<answer id='105' status='1' msg='Installing module procedure started.'/>");
							return;
						};
					};
				};
				module_node = module_node.nextSibling();
			}
		};
		data_node = data_node.nextSibling();
	};
	//TODO. if you remove return at the top. then need to rewrite this block
	emit sendDataToRemoteHost_signal("<answer id='105' status='0' msg='Could not find node with this id'/>");
};

void DiscompServerRPCThread::clearFailedModulesForNodes( )
{
	logMessage (LOG_COMMON,"Command: clear failed modules list for all nodes");

	discomp_server_ptr->clearFailedModulesForNodes();
	emit sendDataToRemoteHost_signal("<answer id='108' status='1' msg='Failed modules list cleared for all nodes'/>");
}


/***********************************************************************/
/*---------- RPC processing functions for processes ----------*/
/***********************************************************************/
/**
 * Command for start new process
 */
void DiscompServerRPCThread::startProcess ( const QDomNode &xml_node )
{
	logMessage (LOG_COMMON,"Command: start calculating process");

	QDomNode process_info = XML::getChildNodeByTag( "process", xml_node );
	QString  process_name = process_info.toElement().attribute("name");

	if ( process_name != "" ) {
		logMessage (LOG_COMMON, "  Process name: " + process_name );
		int process_id = discomp_server_ptr->startProcess(process_name, this->login);
		if ( process_id >= 0 ) {
			emit sendDataToRemoteHost_signal(QString("<answer id='110' status='1' msg='Process successfully started. Id: %1'/>").arg(process_id));
		} else if ( process_id == -1 ) {
			emit sendDataToRemoteHost_signal("<answer id='110' status='0' msg='Process already started'/>");
		} else if ( process_id == -2 ) {
			emit sendDataToRemoteHost_signal("<answer id='110' status='0' msg='Could not start process. Check log for details'/>");
		};
	} else {
		emit sendDataToRemoteHost_signal("<answer id='110' status='0' msg='Process name is empty'/>");
	};
}

/**
 * Command for start new process
 */
void DiscompServerRPCThread::startSingleProcess ( const QDomNode &xml_node )
{
	logMessage (LOG_COMMON,"Command: start single task (from the dir)");

	QDomNode process_info = XML::getChildNodeByTag( "process", xml_node );
	QString  process_dir  = process_info.toElement().attribute("dir");

	if ( process_dir != "" ) {
		logMessage (LOG_COMMON, "  Task directory: " + process_dir );
		int process_id = discomp_server_ptr->startSingleProcess(process_dir, this->login);
		if ( process_id >= 0 ) {
			emit sendDataToRemoteHost_signal( QString("<answer id='115' status='1' msg='Process successfully started. Id: %1'/>").arg(process_id) );
		} else if ( process_id == -1 ) {
			emit sendDataToRemoteHost_signal("<answer id='115' status='0' msg='Process already started'/>");
		} else if ( process_id == -2 ) {
			emit sendDataToRemoteHost_signal("<answer id='115' status='0' msg='Could not start process. Check log for details'/>");
		};
	} else {
		emit sendDataToRemoteHost_signal("<answer id='115' status='0' msg='Process name is empty'/>");
	};
}


/**
 * Command for stop calculating process
 */
void DiscompServerRPCThread::stopProcess ( const QDomNode &xml_node )
{
	logMessage (LOG_COMMON,"Command: stop calculating process");

	QDomNode process_info = XML::getChildNodeByTag( "process", xml_node );
	QString  process_name = process_info.toElement().attribute("name");

	if ( process_name != "" ) {
		int process_id = discomp_server_ptr->getProcessIdByName( process_name );
		if ( process_id != -1 ) {
			discomp_server_ptr->stopProcess(process_id);
			emit sendDataToRemoteHost_signal("<answer id='111' status='1' msg='Process successfully stopped'/>");
		} else {
			emit sendDataToRemoteHost_signal("<answer id='111' status='0' msg='Could not find process with this name'/>");
		}
		logMessage (LOG_COMMON, "  Process name: " + process_name );
	} else {
		emit sendDataToRemoteHost_signal("<answer id='111' status='0' msg='Process name and id empty'/>");
	};

};

/**
 * Command for get info about calculating process
 */
void DiscompServerRPCThread::getProcessInfo ( const QDomNode &xml_node )
{
	logMessage (LOG_COMMON, "Command: get process information");

	QDomNode process_info = XML::getChildNodeByTag( "process", xml_node );
	QString  process_name = process_info.toElement().attribute("name");

	if ( process_name != "" ) {
		int process_id   = discomp_server_ptr->getProcessIdByName( process_name );
		if ( process_id != -1 ) {
			int process_num  = discomp_server_ptr->getProcessNumById ( process_id );
			if ( process_num != -1 ) {
				QString process_data = QString("<process id='%1' name='%2' completed='%3' current_stage='%4'/>").
						arg(discomp_server_ptr->processes[process_num]->get_id()).
						arg(discomp_server_ptr->processes[process_num]->get_name()).
						arg(discomp_server_ptr->processes[process_num]->get_completed()).
						arg(discomp_server_ptr->processes[process_num]->get_current_stage());
				emit sendDataToRemoteHost_signal("<answer id='112' status='1'>" + process_data + "</answer>");
				
				logMessage ( LOG_COMMON, "  Process name:" + discomp_server_ptr->processes[process_num]->get_name() );
			} else {
				emit sendDataToRemoteHost_signal("<answer id='112' status='0' msg='Could not find process'/>");
			};
		} else {
			emit sendDataToRemoteHost_signal("<answer id='112' status='0' msg='Could not find process'/>");
		};
	} else {
		emit sendDataToRemoteHost_signal("<answer id='112' status='0' msg='Process name is empty'/>");
	};

};

/**
 * Command for get info about all processes
 */
void DiscompServerRPCThread::getProcessesList (  )
{
	logMessage (LOG_COMMON,"Command: get processes list");

	QString processes_list_xml = "";
//	QVector <DiscompProcess*>::iterator process_it = discomp_server_ptr->processes.begin();
//	while ( process_it != discomp_server_ptr->processes.end() ) {

	for ( int process_num=0; process_num < (int)discomp_server_ptr->processes.size(); process_num++ ) {
		DiscompProcessPrivate *discomp_process = discomp_server_ptr->processes[process_num];

		//check user
		if ( discomp_process->get_user_name() != this->login ) {
			continue;
		};

		QDateTime timestamp (QDateTime::currentDateTime ());
		quint64 exec_time = timestamp.toTime_t () - discomp_process->get_execution_start_time();

		processes_list_xml += QString("<process id='%1' name='%2' completed='%3' current_stage='%4' user='%5' priority='%6' exec_time='%7'/>").
						arg(discomp_process->get_id()).
						arg(discomp_process->get_name()).
						arg(discomp_process->get_completed()).
						arg(discomp_process->get_current_stage()).
						arg(discomp_process->get_user_name()).
						arg(discomp_process->get_priority()).
						arg(exec_time);
	};

	emit sendDataToRemoteHost_signal("<answer id='113'>" + processes_list_xml + "</answer>");
};





/***********************************************************************/
/*-----------------------  Private functions  -------------------------*/
/***********************************************************************/
/**
 * Private function for write log messages from this class
 */
void DiscompServerRPCThread::logMessage ( int log_level, const QString & message ) 
{
	Log::addMessage (QString("server_rpc.%1").arg(LOG_FILE_EXT), log_level, "DiscompServerRPCThread", message);
};


// vim: set fenc=utf-8 tabstop=8 :
