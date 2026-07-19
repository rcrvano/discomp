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
#include "rpc_server.h"
#include "rpc_thread.h"

/**
 * @class DiscompServerRPC
 * @brief RPC threaded server
 *
 * In this class realized threaded server for rpc
 * connections. For each incoming connection created new
 * thread which works with discomp_server class, collect 
 * info and send it to rpc client
 */

/***********************************************************************/
/*-----------------  Constructor & Destructor  ------------------------*/
/***********************************************************************/

/**
 * Class constructor
 * */
DiscompServerRPC::DiscompServerRPC(QObject *parent)
    : QTcpServer(parent)
{
	logMessage (LOG_DEBUG, "Constructed new DiscompServerRPC Class");
}

/** 
 * Class destructor
 * */
DiscompServerRPC::~DiscompServerRPC()
{
	logMessage (LOG_DEBUG, "DiscompServerRPC Class destructed");
};

/**
 * Set pointer to the DiscompServer class
 */
void DiscompServerRPC::setDiscompServer( DiscompServer* ds ) 
{
	this->discomp_server_ptr = ds;
};


/***********************************************************************/
/*-------------------Connect & disconnect remote host -----------------*/
/***********************************************************************/

/** 
 * New host trying to connect to the server. 
 * Create a new thread and pass pointer on the DiscompServer to 
 * the created thread.
 */
void DiscompServerRPC::incomingConnection (int socketDescriptor)
{
	logMessage ( LOG_DEBUG, "New host trying to connect." );

	//create new thread
	DiscompServerRPCThread *newHostThread = new DiscompServerRPCThread(socketDescriptor);
	newHostThread->setDiscompServer ( this->discomp_server_ptr );
	connect(newHostThread, SIGNAL(finished()), newHostThread, SLOT(deleteLater()));
	
	//run thread
	newHostThread->start();
}


/***********************************************************************/
/*-------------------------- Private functions ------------------------*/
/***********************************************************************/

/**
 * Private function for write log messages from this class
 */
void DiscompServerRPC::logMessage ( int log_level, const QString &message ) 
{
	Log::addMessage ( QString("server_rpc.%1").arg(LOG_FILE_EXT), log_level, "DiscompServerRPC", message);
};

// vim: set fenc=utf-8 tabstop=8 :
