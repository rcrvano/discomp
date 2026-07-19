// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+

#include <QStringList>

#include <iostream>

#include "lib/xml.h"
#include "lib/common.h"
#include "lib/file.h"
#include "lib/socket.h"
#include "lib/log.h"
#include "config.h"
#include "communicator.h"

/**
 * @class DiscompCommunicator
 * @brief Communicator with Discomp server
 *
 * In this class implemented communications with discomp server.
 * Via command line user talk to server to do some action.
 * After processing command line arguments communicator trying to connect 
 * to server. If all is ok and connection established formed 
 * XML-request sending to server. Server make some actions and send answer
 * which processed on the communicator side and on the 
 * console displayed status of operation
 */


/***********************************************************************/
/*-----------------  Constructor & Destructor  ------------------------*/
/***********************************************************************/

/**
 * Constructor
 */
DiscompCommunicator::DiscompCommunicator(QObject *parent)
	: QObject(parent)
{
	discomp_socket = NULL;
};

/**
 * Destructor
 */
DiscompCommunicator::~DiscompCommunicator()
{
	this->doDisconnect();
	if ( discomp_socket != NULL ) {
		delete discomp_socket;
	};
};






/**************************************************************/
/*-------------------- Public class methods ------------------*/
/**************************************************************/

/**
 * Connect to server
 */
void DiscompCommunicator::doConnect( const QString &serverIP, const QString &serverPort )
{
	discomp_socket = new DiscompSocket(0);


	connect(discomp_socket, SIGNAL ( connectedToServer_signal            () ),
		this,           SLOT   ( connectedToServer_slot              () ) );

	connect(discomp_socket, SIGNAL ( socketError_signal                  ( int ) ),
		this,           SLOT   ( socketError_slot                    ( int ) ) );

	connect(discomp_socket, SIGNAL ( connectionClosed_signal             () ),
		this,           SLOT   ( connectionClosed_slot               () ) );

	connect(discomp_socket, SIGNAL ( processAnswerFromRemoteHost_signal  (const QString &) ),
		this,           SLOT   ( processAnswerFromServer_slot        (const QString &) ));

	connect(this,           SIGNAL ( connectToServer_signal              (const QString &, const QString &) ),
	        discomp_socket, SLOT   ( connectToServer_slot                (const QString &, const QString &) ) );

	connect(this,           SIGNAL ( doDisconnect_signal                 () ),
	        discomp_socket, SLOT   ( doDisconnect_slot                   () ) );

	connect(this,           SIGNAL ( sendDataToRemoteHost_signal         (const QString &) ),
	        discomp_socket, SLOT   ( sendDataToRemoteHost_slot           (const QString &) ) );


	emit connectToServer_signal ( serverIP, serverPort );
};

/**
 * Disconnect from server
 */
void DiscompCommunicator::doDisconnect( )
{
	emit doDisconnect_signal ();
	exit (0);
};

/**
 * Display usge help and exit.
 */
void DiscompCommunicator::showHelpUsage() 
{
	std::cout << std::endl;
	std::cout << " Communicator with Discomp Server." << std::endl;
	std::cout << std::endl;

	std::cout << " Examples: " <<std::endl;
	std::cout << "     discomp_communicator --config /etc/discomp/communicator.conf --nodeslist --login vano --password 123" << std::endl;
	std::cout << "     discomp_communicator --login vano --password 123 --verbose --connect 192.168.10.192" << std::endl;
	std::cout << "     discomp_communicator --login vano --password 123 --startprocess dsat " << std::endl;
	std::cout << "     discomp_communicator --login vano --password 123 --host 192.168.1.10 --port 5733 --processeslist " << std::endl;
	std::cout << std::endl;

	std::cout << " Usage manual:" << std::endl;

	std::cout << "     --login          userLogin           - specify user login" << std::endl;
	std::cout << "     --password       userPassword        - specify user password" << std::endl;
	std::cout << std::endl;

	std::cout << "     --newlogin      newUserLogin        - specify new user login" << std::endl;
	std::cout << "     --newpassword   newUserPassword     - specify new user password" << std::endl;
	std::cout << std::endl;

	std::cout << "     --host           serverHost          - specify server host" << std::endl;
	std::cout << "     --port           serverPort          - specify server port" << std::endl;
	std::cout << "     --verbose                            - print verbose info" << std::endl;
	std::cout << std::endl;

	std::cout << "     --nodeslist                          - show list of connected nodes" << std::endl;
	std::cout << "     --connect        IP                  - connect node to server" << std::endl;
	std::cout << "     --connect        IP:Port             - connect node to server (with port spec.)" << std::endl;
	std::cout << "     --connect        IP_1:Port,IP_2:Port - connect multiple nodes to server" << std::endl;
	std::cout << "     --connect        IP_1,IP_2,IP_3      - connect multiple nodes to server (with port spec.)" << std::endl;
	std::cout << "     --disconnect     node_id             - disconnect remote node from server by id" << std::endl;
	std::cout << "     --disconnect     node_id1,node_id2   - disconnect multiple remote nodes from server by id" << std::endl;
	std::cout << "     --nodeinfo       node_id             - show node info" << std::endl;
	std::cout << std::endl;

	std::cout << "     --processeslist                      - show processes list" << std::endl;
	std::cout << "     --startprocess   process_name        - start process by name" << std::endl;
	std::cout << "     --stopprocess    process_name        - stop process by name" << std::endl;
	std::cout << "     --processinfo    process_name        - show process info by name" << std::endl;
	std::cout << std::endl;

	std::cout << "     --xml filePath                       - send command to server from XML file" << std::endl;



};

/**
 * Parse command line parameters and construct 
 * xml-requst to server. It will be send to server 
 * immidiately after connect after connect to server.
 * \sa showHelpUsage
 */
void DiscompCommunicator::parseCommandLine(int argc, char *argv[])
{
	//make best functions for displaying whats wrong
	int method_id = 0;
	QString method_name;
	QString login;
	QString password;
	QString new_login;
	QString new_password;

	QString base_xml_content;
	for ( int arg_num=1; arg_num<argc; arg_num++ ) {
		QString argument_str (argv[arg_num]);
		if ( argument_str.startsWith("--") ) {        //it's a option name like --node
			if ( argument_str == "--xml" ) {
				method_id = -1;

				QString filepath (argv[++arg_num]); //get next argument
				if ( filepath.startsWith("-") ) { this->showHelpUsage(); exit(0); };

				if ( !File::read( filepath, base_xml_content ) ) {
					std::cout << "Could't open file:" << filepath.toStdString() << std::endl;
					exit(0);
				};

			//user auth data
			} else if ( argument_str == "--login" ) {
				login = QString(argv[++arg_num]);
			} else if ( argument_str == "--password" ) {
				password = QString(argv[++arg_num]);

			//create new user
			} else if ( argument_str == "--newlogin" ) {
				method_id = 2;
				method_name = "createNewUser";
				new_login = QString(argv[++arg_num]);
			} else if ( argument_str == "--newpassword" ) {
				method_id = 2;
				method_name = "changeUserPassword";
				new_password = QString(argv[++arg_num]);
			

			// works with nodes
			} else if ( argument_str == "--connect" ) {
				method_id = 100;
				method_name = "connectNode";

				QString argument_value_str (argv[++arg_num]); //get next argument
				if ( argument_value_str.startsWith("-") ) { this->showHelpUsage(); exit(0); };

				QStringList hosts_list = argument_value_str.split(",");
				for ( int i = 0; i < hosts_list.size(); ++i) {
					QStringList host =  hosts_list.at(i).split(":");
					if ( host.size()==2 && host.at(0) != "" && host.at(1) != "" ) {
						base_xml_content += "<node ip='"+host.at(0)+"' port='"+host.at(1)+"'/>\n";
					} else if ( host.size()==1 && host.at(0) != "" ) { //connect on standart port
						base_xml_content += "<node ip='"+host.at(0)+"'/>\n";
					};
				};

			} else if ( argument_str == "--disconnect" ) {
				method_id = 101;
				methodName = "disconnectNode";

				QString argument_value_str (argv[++arg_num]); //get next argument
				if ( argument_value_str.startsWith("-") ) { this->showHelpUsage(); exit(0); };

				QStringList hosts_list = argument_value_str.split(",");
				for ( int i = 0; i < hosts_list.size(); ++i) {
					if ( hosts_list.at(i) != "" ) {
						base_xml_content += "<node id='"+hosts_list.at(i)+"'/>\n";
					};
				};

			} else if ( argument_str == "--nodeinfo" ) {
				method_id = 102;
				method_name = "getNodeInfo";

				QString argument_value_str (argv[++arg_num]); //get next argument
				if ( argument_value_str.startsWith("-") ) { this->showHelpUsage(); exit(0); };

				QStringList hosts_list = argument_value_str.split(",");
				for ( int i = 0; i < hosts_list.size(); ++i) {
					if ( hosts_list.at(i) != "" ) {
						base_xml_content += "<node id='"+hosts_list.at(i)+"'/>\n";
					};
				};

			} else if ( argument_str == "--nodeslist" ) {
				method_id = 103;
				method_name = "getNodesList";

			//works with processes
			} else if ( argument_str == "--startprocess" ) {
				method_id = 110;
				method_name = "startProcess";

				QString argument_value_str (argv[++arg_num]); //get next argument
				if ( argument_value_str.startsWith("-") ) { this->showHelpUsage(); exit(0); };

				base_xml_content += "<process name='"+argument_value_str+"'/>\n";

			} else if ( argument_str == "--stopprocess" ) {
				method_id = 111;
				method_name = "stopProcess";

				QString argument_value_str (argv[++arg_num]); //get next argument
				if ( argument_value_str.startsWith("-") ) { this->showHelpUsage(); exit(0); };

				base_xml_content += "<process name='"+argument_value_str+"'/>\n";
			} else if ( argument_str == "--processinfo" ) {
				method_id = 112;
				method_name = "getProcessInfo";

				QString argument_value_str (argv[++arg_num]); //get next argument
				if ( argument_value_str.startsWith("-") ) { this->showHelpUsage(); exit(0); };

				base_xml_content += "<process name='"+argument_value_str+"'/>\n";
			} else if ( argument_str == "--processeslist" ) {
				method_id = 113;
				method_name = "getProcessesList";

			} else if ( argument_str == "--verbose" ) {
				Log::logVerboseOnConsole = true;
			} else if ( argument_str == "--config" ) {
				++arg_num;
			} else if ( argument_str == "--host" ) {
				++arg_num;
			} else if ( argument_str == "--port" ) {
				++arg_num;
			}

		} else if ( argument_str.startsWith("-") ) {  //it's a small option name like -n or -npv (multiple options)
			//not supported yet
			logMessage(LOG_DEBUG, "start with -:" + argument_str);
			this->showHelpUsage();
			exit(0);
		} else {      //it's a option value like node IP
			logMessage(LOG_ERROR, "unknown command: " + argument_str);
			this->showHelpUsage();
			exit(0);
		};
	};
	if ( method_id == 2 && new_login != "" && new_password != ""  ) {
		base_xml_content += "<user new_login='"+new_login+"' new_password='"+File::getStrMD5(new_password)+"'/>";
	};

	if ( login == "" || password == "" ) {
		std::cout << "Error: You should specify login and password." << std::endl;
		this->showHelpUsage();
		exit(0);
	};

	if ( method_id == -1 ) { 
		xml_command_for_send = base_xml_content;
	} else if ( method_id == 0 ) {
		this->showHelpUsage();
		exit(0);
	} else {
		xml_command_for_send += "<methodCall id='"+QString("%1").arg(method_id)+"' login='"+login+"' password='"+File::getStrMD5(password)+"'>\n"+
		xml_command_for_send += "	<methodName>" + method_name + "</methodCall>";
		xml_command_for_send += "	<params>" + base_xml_content + "</params>";
		xml_command_for_send += "</methodCall>";
	};
	
	logMessage( LOG_DEBUG, "Prepared command for send to server:\n" + xml_command_for_send );
};



/***********************************************************************/
/*---------------------------- Protocol slots -------------------------*/
/***********************************************************************/

/**
 * After connect to server send preparied XML request
 */
void DiscompCommunicator::connectedToServer_slot()
{
	logMessage( LOG_COMMON, "Send command to server:\n" + xml_command_for_send );

	emit sendDataToRemoteHost_signal ( xml_command_for_send ); 

}

/**
 * Connection closed. Call exit
 */
void DiscompCommunicator::connectionClosed_slot()
{
	logMessage(LOG_DEBUG, "Connection closed");

	exit(0);
};

/**
 * Some socket error occured. Host not found, Connection closed or refused.
 * Print log message and exit
 */
void DiscompCommunicator::socketError_slot ( int code ) {
	//logMessage(LOG_DEBUG, message);
	code = 0; //for hide gcc unused warning
	exit(0);
};

/* 
 * Protocol switch where we detect server answer, and call 
 * processing functions this answer id.
 * */
void DiscompCommunicator::processAnswerFromServer_slot( const QString &xml_data_str )
{
	QString errorStr;
	int errorLine;
	int errorColumn;

	logMessage (LOG_COMMON, "Received answer from server:\n " + xml_data_str );

	QDomDocument doc;
	if ( !doc.setContent( xml_data_str, true, &errorStr, &errorLine, &errorColumn ) ) {
		logMessage (LOG_ERROR, QString("XML processing error ( line %1, column %2: %3 )").arg(errorLine).arg(errorColumn).arg(errorStr) );
		return;
	};

	QDomElement rootNode = doc.documentElement();

	QDomNode answerNode = rootNode.firstChild();
	while ( answerNode.nodeType() != QDomNode::ElementNode ) {
		answerNode = answerNode.nextSibling();
	}

	
	int authStatus = answerNode.toElement().attribute("auth").toInt();
	if ( authStatus == -1 ) {
		logMessage(LOG_ERROR, "Invalid username or password" );
		exit(0);
	};
	
	int answerTypeId = answerNode.toElement().attribute("id").toInt();

	switch ( answerTypeId ) {
		case 2:
			if ( answerNode.toElement().attribute("status").toInt() ) {
				std::cout << "User created sucessfully" << std::endl;
			} else {
				std::cout << "Couldn't create new user. Reason: " << 
					answerNode.toElement().attribute("msg").toStdString() << std::endl;
			};
			break;

		case 100:
			if ( answerNode.toElement().attribute("status").toInt() ) {
				std::cout << "Nodes was push in the queue for connect" << std::endl;
			} else {
				std::cout << "Couldn't connect nodes. Reason: " << 
					answerNode.toElement().attribute("msg").toStdString() << std::endl;
			};
			break;

		case 101:
			if ( answerNode.toElement().attribute("status").toInt() ) {
				std::cout << "Nodes sucesssfully disconnected" << std::endl;
			} else {
				std::cout << "Couldn't disconnect nodes. Reason: " << 
					answerNode.toElement().attribute("msg").toStdString() << std::endl;
			};
			break;
		case 102:
			showNodeInfo ( answerNode );
			break;

		case 103:
			showNodesList( answerNode );
			break;

		case 110:
			if ( answerNode.toElement().attribute("status").toInt() ) {
				std::cout << "Process successfully started" << std::endl;
			} else {
				std::cout << "Couldn't start process. Reason: " << 
					answerNode.toElement().attribute("msg").toStdString() << std::endl;
			};
			break;

		case 111:
			if ( answerNode.toElement().attribute("status").toInt() ) {
				std::cout << "Process successfully stoped" << std::endl;
			} else {
				std::cout << "Couldn't stop process. Reason: " << 
					answerNode.toElement().attribute("msg").toStdString() << std::endl;
			};
			break;

		case 112:
			showProcessInfo ( answerNode );
			break;
		case 113:
			showProcessesList( answerNode );
			break;

		default:
			break;
	};
	logMessage( LOG_DEBUG, QString("Received answer from server id: %1").arg(answerTypeId) );

	this->doDisconnect();
};




/***********************************************************************/
/*----------------- Protocol processing functions  --------------------*/
/***********************************************************************/
/**
 * Print information about connected nodes list
 */
void DiscompCommunicator::showNodesList ( const QDomNode &root_node )
{
	logMessage (LOG_DEBUG,"Processing connected nodes list");
	std::cout << "Connected nodes list:" << std::endl;

	QVector<QString> columns;
	columns << "id" << "ip" << "name" << "ping" << "busy" << "process_id" << "process_name" << "module_id" << "module_name" << "module_list_num" << "module_exec_time" << "ping";
	QVector<QString> columns_titles;
	columns_titles << "Id" << "IP" << "Name" << "Ping (ms)" << "Busy" << "PID" << "PName" << "MID" << "MName" << "MListNum" << "MExecTime";


	QVector <QStringList> rows;
	QDomNode data_node = root_node.firstChild();
	while ( !data_node.isNull() ) {
		if ( data_node.toElement().tagName() == "node" ) {
			QStringList node_info;
			for (int i=0; i<columns.size(); i++ ) {
				node_info << data_node.toElement().attribute(columns[i]);
			};
			rows.push_back(node_info);
		};
		data_node = data_node.nextSibling();
	};

	this->printFormatTable ( columns_titles, rows );
};

/**
 * Print information about connected node
 */
void DiscompCommunicator::showNodeInfo ( const QDomNode &root_node )
{
	logMessage (LOG_DEBUG,"Processing node info");

	if ( !root_node.toElement().attribute("status").toInt() ) {
		std::cout << "Node not found" << std::endl;
		return;
	};

	QDomNode node_info = XML::getChildNodeByTag( "node", root_node );
	
	std::cout << "Node information" << std::endl;
	std::cout << "Id:\t"	<< node_info.toElement().attribute("id").toStdString() << std::endl;
	std::cout << "IP:\t"	<< node_info.toElement().attribute("ip").toStdString() << std::endl;
	std::cout << "Name:\t"	<< node_info.toElement().attribute("name").toStdString() << std::endl;
	std::cout << "Busy:\t"	<< node_info.toElement().attribute("busy").toStdString() << std::endl;
	if ( node_info.toElement().attribute("busy").toInt() ) {
		std::cout << "Process id:\t"	<< node_info.toElement().attribute("process_id").toStdString() << std::endl;
		std::cout << "Process name:\t"	<< node_info.toElement().attribute("process_name").toStdString() << std::endl;
	};
};


/**
 * Print information about connected processes
 */
void DiscompCommunicator::showProcessesList ( const QDomNode &root_node )
{
	logMessage (LOG_DEBUG,"Processing connected processs list");
	std::cout << "Processs list:" << std::endl;

	QVector<QString> columns;
	columns << "id" << "name" << "current_stage" << "completed";
	QVector<QString> columns_titles;
	columns_titles << "Id" << "Name" << "Stage" << "Completed";

	QVector <QStringList> rows;
	QDomNode data_node = root_node.firstChild();
	while ( !data_node.isNull() ) {
		if ( data_node.toElement().tagName() == "process" ) {
			QStringList node_info;
			for (int i=0; i<columns.size(); i++ ) {
				node_info << data_node.toElement().attribute(columns[i]);
			};
			rows.push_back(node_info);
		};
		data_node = data_node.nextSibling();
	};

	this->printFormatTable ( columns_titles, rows );
};

/**
 * Print process information
 */
void DiscompCommunicator::showProcessInfo ( const QDomNode &root_node )
{
	logMessage (LOG_DEBUG,"Processing node info");

	if ( !root_node.toElement().attribute("status").toInt() ) {
		std::cout << "Could't get information about process. Reason: " << root_node.toElement().attribute("msg").toStdString() <<  std::endl;
		return;
	};

	QDomNode node_info = XML::getChildNodeByTag( "process", root_node );
	
	std::cout << "Process information" << std::endl;
	std::cout << "Id: " << node_info.toElement().attribute("id").toStdString() << std::endl;
	std::cout << "Name: " << node_info.toElement().attribute("name").toStdString() << std::endl;
	std::cout << "Completed: " << node_info.toElement().attribute("completed").toStdString() << std::endl;
	std::cout << "Current stage: " << node_info.toElement().attribute("current_stage").toStdString() << std::endl;
};




/***********************************************************************/
/*-------------------------- Private functions ------------------------*/
/***********************************************************************/

/**
 * Print formated table with titles and rows
 */
void DiscompCommunicator::printFormatTable ( QVector<QString> &columns, QVector <QStringList> &rows )
{

	//set max column size to column name + space
	QVector<int> columns_size;
	columns_size.resize(columns.size());
	for (int i=0; i<columns.size(); i++ ) {
		columns_size[i] = columns[i].size();
	}

	//collect info about max columns size
	for ( int i=0; i<rows.size(); i++ ) {
		for (int j=0; j<columns.size(); j++ ) {
			if ( rows[i][j].size() > columns_size[j] ) {
				columns_size[j] = rows[i][j].size();
			};
		};
	};

	//print horizontal line before header
	std::cout << "+";
	for (int i=0; i<columns.size(); i++ ) {
		for ( int j=0; j<columns_size[i]; j++ ) {
			std::cout << "-";
		};
		std::cout << "--+";
	};
	std::cout << std::endl;


	std::cout << "| ";
	//Print headers
	for (int i=0; i<columns.size(); i++ ) {
		//header title
		std::cout << columns[i].toStdString();

		//and spaces after that
		for ( int j=0; j<columns_size[i]-columns[i].size(); j++ ) {
			std::cout << " ";
		};
		std::cout << " | ";
	}
	std::cout << std::endl;

	//print horizontal line after header
	std::cout << "+";
	for (int i=0; i<columns.size(); i++ ) {
		for ( int j=0; j<columns_size[i]; j++ ) {
			std::cout << "-";
		};
		std::cout << "--+";
	};
	std::cout << std::endl;



	//And now print info about rows
	for ( int i=0; i<rows.size(); i++ ) {
		std::cout << "| ";
		for (int j=0; j<columns.size(); j++ ) {
			std::cout << rows[i][j].toStdString();

			//and spaces after that
			for ( int k=0; k<columns_size[j]-rows[i][j].size(); k++ ) {
				std::cout << " ";
			};
			std::cout << " | ";
		};
		std::cout << std::endl;
	};


	//print end horizontal line line after header
	std::cout << "+";
	for (int i=0; i<columns.size(); i++ ) {
		for ( int j=0; j<columns_size[i]; j++ ) {
			std::cout << "-";
		};
		std::cout << "--+";
	};
	std::cout << std::endl;
};


/**
 * Wrapper for Log::addMessage
 */
void DiscompCommunicator::logMessage ( int log_level, const QString &message ) 
{
	Log::addMessage ( log_level, "DiscompCommunicator", message);
};
// vim: set fenc=utf-8 tabstop=8 :
