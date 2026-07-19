// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+

#include <QString>

#include "client.h"
#include "lib/log.h"
#include "client_bind_port.h"

/**
 * @class DiscompClientBindPort
 * @brief Binding port for connect client from server side
 * 
 * Class innherit QTcpClass and bind port specified in the config as 'ClientPort'.
 * Client listeining this port and if detected incoming connection and client
 * isn't connected to server yet, then initializing "connecting to server" procedute.
 */

/**
 * Class constructor 
 */
DiscompClientBindPort::DiscompClientBindPort(QObject *parent)
    : QTcpServer(parent)
{
	this->discomp_clients_list = NULL;

	logMessage (LOG_DEBUG, "Constructed new DiscompClientBindPort Class");
}

/**
 * Class destructor 
 */
DiscompClientBindPort::~DiscompClientBindPort()
{
};


/**
 * Incoming connection. Drop it and connect to Discomp Server
 * */
void DiscompClientBindPort::incomingConnection (int socketDescriptor)
{
	logMessage ( LOG_COMMON, "Detected connection from remote host. Drop it and make connection to discomp server" );
	for ( int i=0; i<discomp_clients_list->size(); i++ ) {
		if ( discomp_clients_list->at(i) ) {
			if (  discomp_clients_list->at(i)->getConnectionStatus() == 2 ) {
				discomp_clients_list->at(i)->doDisconnect();
			};
			logMessage ( LOG_COMMON, QString("Connect client num %1").arg(i) );
			discomp_clients_list->at(i)->doConnect();
		};
	};

	socketDescriptor = 0;//for hide GCC unused warning
}

/**
 * Private function for write log message
 */
void DiscompClientBindPort::logMessage ( int log_level, const QString &message ) 
{
	Log::addMessage (log_level, "DiscompClientBindPort", message);
};

// vim: set fenc=utf-8 tabstop=8 :
